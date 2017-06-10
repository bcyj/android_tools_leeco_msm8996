/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include "debug.h"
#include "cnss_qmi_client.h"
#include "nl_loop.h"
#include "cutils/misc.h"

#include <sys/capability.h>
#include <linux/prctl.h>
#include <private/android_filesystem_config.h>

#define DAEMONIZE	0x1
#define LOG_FILE	0x2
#define SYS_LOG		0x4
#define LOGCAT		0x8
#define VERSION_PATH    "/sys/devices/soc.0/6300000.qcom,cnss/wlan_setup"
#define CLD_2_0_MODULE  "/system/lib/modules/qcacld20.ko"
#define CLD_3_0_MODULE  "/system/lib/modules/qcacld30.ko"
#define FW_SETUP_PATH   "/sys/devices/soc.0/6300000.qcom,cnss/fw_image_setup"
#define QCA_CLD_ROME_2_0 0x20
#define QCA_CLD_ROME_3_0 0x30
#define FW_DES_MISSION_MODE 0x02

const unsigned int capabilities = (1 << CAP_NET_RAW) | (1 << CAP_NET_ADMIN);
const gid_t groups[] = {AID_INET, AID_NET_ADMIN, AID_NET_RAW};

int wsvc_debug_level = MSG_DEFAULT;
static char *progname = NULL;
/*
 * The glibc doesn't provide a header file declaration for init_module API.
 * Hence adding it here
 */
extern int init_module(void *, unsigned long, const char *);

static void usage(void)
{
	fprintf(stderr, "\nusage: %s [options] \n"
		"   -n, --nodaemon  do not run as a daemon\n"
		"   -d              show more debug messages (-dd for more)\n"
#ifdef CONFIG_DEBUG_FILE
		"   -f <path/file>  Log output to file \n"
#endif
#ifdef CONFIG_DEBUG_SYSLOG
		"   -s              Log output to syslog \n"
#endif
#ifdef CONFIG_DEBUG_LOGCAT
		"   -l              Log output to logcat \n"
#endif
		"       --help      display this help and exit\n"
		, progname);
	exit(EXIT_FAILURE);
}

static void wlan_service_sighandler(int signum)
{
	wsvc_printf_info("Caught Signal: %d", signum);
	nl_loop_terminate();

	return;
}

static int wlan_service_setup_sighandler(void)
{
	struct sigaction sig_act;

	memset(&sig_act, 0, sizeof(sig_act));
	sig_act.sa_handler = wlan_service_sighandler;
	sigemptyset(&sig_act.sa_mask);

	sigaction(SIGQUIT, &sig_act, NULL);
	sigaction(SIGTERM, &sig_act, NULL);
	sigaction(SIGINT, &sig_act, NULL);
	sigaction(SIGHUP, &sig_act, NULL);

	return 0;
}

int lower_app_cap_handle(void)
{
	struct __user_cap_header_struct cap_header_data;
	cap_user_header_t cap_header = &cap_header_data;
	struct __user_cap_data_struct cap_data_data;
	cap_user_data_t cap_data = &cap_data_data;

	memset(cap_header, 0, sizeof(cap_header_data));
	cap_header->pid = 0;
	cap_header->version = _LINUX_CAPABILITY_VERSION;
	memset(cap_data, 0, sizeof(cap_data_data));

	if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0) {
		wsvc_perror("PR_SET_KEEPCAPS error");
		return -1;
	}

	if (setgroups(ARRAY_SIZE(groups), groups) != 0) {
		wsvc_perror("setgroups error");
		return -1;
	}

	if (setgid(AID_SYSTEM)) {
		wsvc_perror("set gid error");
		return -1;
	}

	if (setuid(AID_SYSTEM)) {
		wsvc_perror("set uid error");
		return -1;
	}

	cap_data->effective = capabilities;
	cap_data->permitted = capabilities;
	cap_data->inheritable = capabilities;

	if (capset(cap_header, cap_data) < 0) {
		wsvc_perror("capset error");
		return -1;
	}
	return 0;
}

#ifdef CONFIG_DUAL_LOAD
static int wlan_insmod(const char *filename, const char *args)
{
	void *module;
	unsigned int size;
	int ret;

	module = load_file(filename, &size);

	if (!module) {
		wsvc_perror("Failed to load driver");
		return -1;
	}

	ret = init_module(module, size, args);
	free(module);

	return ret;
}

static int wlan_load_driver(int val)
{
	const char *WLAN_MODULE_NAME = CLD_2_0_MODULE;

	switch (val) {
	case QCA_CLD_ROME_2_0:
		WLAN_MODULE_NAME = CLD_2_0_MODULE;
		break;
	case QCA_CLD_ROME_3_0:
		WLAN_MODULE_NAME = CLD_3_0_MODULE;
		break;
	default:
		break;
	}

	if (wlan_insmod(WLAN_MODULE_NAME, "") < 0) {
		wsvc_perror("Failed to load WLAN Module");
		return -1;
	}

	return 0;
}

static int wlan_setup_fw_desc()
{
	int fd, val, ret = 0;

	fd = open(VERSION_PATH, O_RDWR);

	if (fd < 0) {
		wsvc_printf_err("Failed to Open:%s", FW_SETUP_PATH);
		return -1;
	}

	val = FW_DES_MISSION_MODE;

	if (write(fd, &val, sizeof(int)) < 0) {
		wsvc_printf_err("Failed to write:%s", FW_SETUP_PATH);
		ret = -1;
	}

	close(fd);
	return ret;
}

static int wlan_version_setup()
{
	int fd, val, len, ret = 0;

	fd = open(VERSION_PATH, O_RDONLY);

	if (fd < 0) {
		wsvc_printf_err("Failed to open:%s", VERSION_PATH);
		return -1;
	}

	len = read(fd, &val, sizeof(int));

	if (len <= 0) {
		wsvc_printf_err("Failed to read:%s\n", VERSION_PATH);
		ret = -1;
		goto end;
	}

	if (wlan_setup_fw_desc()) {
		wsvc_perror("Failed to setup Firmware Descriptor\n");
		ret = -1;
		goto end;
	}

	val = atoi((char *)&val);

	if (wlan_load_driver(val)) {
		wsvc_perror("Failed to load driver");
		ret = -1;
	}
end:
	close(fd);
	return ret;
}
#endif

int main(int argc, char *argv[])
{
	int c;
	int i;
	int flag = DAEMONIZE;
	static struct option options[] =
	{
		{"help", no_argument, NULL, 'h'},
		{"nodaemon", no_argument, NULL, 'n'},
		{0, 0, 0, 0}
	};
	char *log_file = NULL;

	progname = argv[0];

	while (1) {
		c = getopt_long(argc, argv, "hdnf:sl", options, NULL);

		if (c < 0)
			break;

		switch (c) {
		case 'n':
			flag &= ~DAEMONIZE;
			break;
		case 'd':
#ifdef CONFIG_DEBUG
			wsvc_debug_level++;
#else
			wsvc_printf_err("Debugging disabled, "
					"please build with -DCONFIG_DEBUG");
			exit(EXIT_FAILURE);
#endif
			break;
#ifdef CONFIG_DEBUG_FILE
		case 'f':
			log_file = optarg;
			flag |= LOG_FILE;
			break;
#endif /* CONFIG_DEBUG_FILE */
#ifdef CONFIG_DEBUG_SYSLOG
		case 's':
			flag |= SYS_LOG;
			break;
#endif /* CONFIG_DEBUG_SYSLOG */
#ifdef CONFIG_DEBUG_LOGCAT
		case 'l':
			flag |= LOGCAT;
			break;
#endif /* CONFIG_DEBUG_LOGCAT */
		case 'h':
		default:
			usage();
			break;
		}
	}

	if (optind < argc)
		usage();

	wsvc_debug_init();

	if (flag & SYS_LOG)
		wsvc_debug_open_syslog();
	else if (flag & LOG_FILE)
		wsvc_debug_open_file(log_file);

	wlan_service_setup_sighandler();

#ifdef CONFIG_DUAL_LOAD
	if (wlan_version_setup()) {
		wsvc_printf_err("Failed to setup driver");
		exit(EXIT_FAILURE);
	}
#endif

	if (lower_app_cap_handle()) {
		wsvc_perror("Cap bouncing fail EXIT!!");
		exit(EXIT_FAILURE);
	}

	if (flag & DAEMONIZE && daemon(0, 0)) {
		wsvc_perror("daemon");
		exit(EXIT_FAILURE);
	}

	if (nl_loop_init()) {
		wsvc_printf_err("Failed to init nl_loop");
		goto out;
	}

	if (0 != wlan_service_start()) {
		wsvc_printf_err("Failed to start wlan service");
		goto register_fail;
	}

	nl_loop_run();

	wlan_service_stop();

register_fail:

	nl_loop_deinit();

out:

	wsvc_debug_close_syslog();
	wsvc_debug_close_file();
	exit(EXIT_SUCCESS);
}
