/*===========================================================================

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/


#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#include "thermal.h"
#include "sensors.h"
#include "virtual_sensors.h"
#include "devices.h"
#include "sensors_manager.h"
#include "devices_manager.h"
#include "pid_algorithm.h"
#include "ss_algorithm.h"
#include "thermal_server.h"
#include "speaker_cal.h"

struct thermal_setting_t thermal_settings;
int num_cpus = 0;
int num_gpus = 0;
int exit_daemon = 0;
int debug_output = LOG_LVL_INFO;
int soc_id = -1;
int enable_restore_brightness = 1;
int minimum_mode = 0;
int dump_bcl = 0;
int output_conf = 0;
int trace_fd = -1;
char *dump_bcl_file = NULL;

static char *config_file = NULL;

void print_help(void)
{
	printf("\nTemperature sensor daemon\n");
	printf("Optional arguments:\n");
	printf("  --config-file/-c <file>        config file\n");
	printf("  --debug/-d                     debug output\n");
	printf("  --soc_id/-s <target>           target soc_id\n");
	printf("  --norestorebrightness/-r       disable restore brightness functionality\n");
	printf("  --output-conf/-o               dump config file of active settings\n");
	printf("  --trace/-t                     enable ftrace tracing\n");
	printf("  --dump-bcl/-i                  BCL ibat/imax file\n");
	printf("  --help/-h                      this help screen\n");
}

int parse_commandline(int argc, char *const argv[])
{
	int c;

	struct option long_options[] = {
		{"config-file",         1, 0, 'c'},
		{"debug",               0, 0, 'd'},
		{"soc_id",              1, 0, 's'},
		{"norestorebrightness", 0, 0, 'r'},
		{"output-conf",         0, 0, 'o'},
		{"dump-bcl",            2, 0, 'i'},
		{"help",                0, 0, 'h'},
		{"trace",		0, 0, 't'},
		{0, 0, 0, 0}
	};

	while ((c = getopt_long(argc, argv, "c:i::s:droht", long_options, NULL)) != EOF) {
		switch (c) {
			case 'c':
				info("Using config file '%s'\n", optarg);
				config_file = optarg;
				break;
			case 'i':
				info("dump BCL ibat/imax to a file\n");
				dump_bcl = 1;
				dump_bcl_file = optarg;
				break;
			case 'd':
				info("Debug output enabled\n");
				debug_output = LOG_LVL_DBG;
				break;
			case 's':
				info("Target SOC_ID specified as '%s'\n", optarg);
				soc_id = atoi(optarg);
				break;
			case 'r':
				info("Restore brightness feature disabled\n");
				enable_restore_brightness = 0;
				break;
			case 'o':
				info("Output conf file to stdout and quit\n");
				output_conf = 1;
				break;
			case 't':
				info("tracing enabled\n");
				trace_fd = open(TRACE_MARKER, O_WRONLY);
				break;
			case 'h':
			default:
				return 0;
		}
	}

	/* Too many/unrecognized argument(s) */
	if (optind < argc) {
		msg("Too many arguments\n");
		return 0;
	}

	return 1;
}

int main(int argc, char **argv)
{
	device_clnt_handle kernel_dev;
	union device_request req;

	info("Thermal daemon started\n");

	setpriority(PRIO_PROCESS, getpid(), -20);


	if (!parse_commandline(argc, argv)) {
		print_help();
		return 0;
	}

	/* Get target default config file if no config is requested
	   through command line */
	if (!config_file) {
		if ((config_file = get_target_default_thermal_config_file()))
			info("Using target config file '%s'\n", config_file);
		else
			info("No target config file, falling back to '%s'\n",
			     CONFIG_FILE_DEFAULT);
	}

	if (output_conf) {
		devices_manager_init();
		devices_init(minimum_mode);
		sensors_manager_init();
		sensors_init(minimum_mode);
		init_settings(&thermal_settings);
		pid_init_data(&thermal_settings);
		thermal_monitor_init_data(&thermal_settings);
		speaker_cal_init_data(&thermal_settings);
		ss_init_data(&thermal_settings);
		virtual_sensors_init(&thermal_settings, config_file);
#               ifdef ENABLE_OLD_PARSER
		load_config(&thermal_settings, config_file);
#               else
		load_config(&thermal_settings, config_file, LOAD_ALL_FLAG);
#               endif
		print_settings(&thermal_settings);
		return 0;
	}

#       ifdef ANDROID
	{
		char buf[PROPERTY_VALUE_MAX];
		/* Early stage of encrypted data partition;
		 * run without modem mitigation and framework socket
		 */
		if ((property_get("vold.decrypt", buf, "0") > 0)
		    && (buf[0] == '1')) {
			minimum_mode = 1;
			info("Running in minimum mode\n");
		}
	}
#       endif

	devices_manager_init();
	devices_init(minimum_mode);
	target_algo_init();
	/* Vote to keep kernel mitigation enabled until init is done */
	kernel_dev = devices_manager_reg_clnt("kernel");
	if (kernel_dev == NULL) {
		msg("%s Failed to create kernel device handle\n", __func__);
	}
	req.value = 1;
	device_clnt_request(kernel_dev, &req);

	sensors_manager_init();
	sensors_init(minimum_mode);

	init_settings(&thermal_settings);

	pid_init_data(&thermal_settings);
	thermal_monitor_init_data(&thermal_settings);
	speaker_cal_init_data(&thermal_settings);
	ss_init_data(&thermal_settings);

	virtual_sensors_init(&thermal_settings, config_file);

#       ifdef ENABLE_OLD_PARSER
	load_config(&thermal_settings, config_file);
#       else
	load_config(&thermal_settings, config_file, LOAD_ALL_FLAG);
#       endif

	thermal_server_init();
	pid_algo_init(&thermal_settings);
	thermal_monitor(&thermal_settings);
	ss_algo_init(&thermal_settings);
	speaker_cal_init(&thermal_settings);

	if (kernel_dev)
		device_clnt_cancel_request(kernel_dev);
	while (1)
		pause();

	devices_release();
	devices_manager_release();
	sensors_release();
	sensors_manager_release();
	thermal_server_release();

	info("Thermal daemon exited\n");
	return 0;
}
