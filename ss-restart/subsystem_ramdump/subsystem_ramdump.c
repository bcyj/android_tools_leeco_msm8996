/*
 * Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Collect peripheral subsystem ramdumps and logs and output to SD card or eMMC
 *
 * Support internal subsystem ramdumps.
 * Support external subsystem ramdumps including MDM and QSC.
 * Support RPM logging.
 * Support QDSS ramdumps
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/netlink.h>

#include <dirent.h>

#define LOG_NUM 1

#define UEVENT_BUF_SIZE 1024
#define RPMLOG_BUF_SIZE 0x00500000

/* External modem ramdump */
#define UEVENT_MDM_SHUTDOWN "remove@/devices/platform/diag_bridge"
#define UEVENT_MDM_POWERUP "add@/devices/platform/diag_bridge"
#define EXT_MDM2_DIR "/tombstones/mdm2"
#define EXT_MDM_DIR "/tombstones/mdm"
#define RAMDUMP_ROOT_PATH "tombstones"
#define STR_MDM "/mdm"
#define STR_QSC "/qsc"
#define MDM_CHECKER "/dev/mdm"
#define ESOC_ROOT_DIR "/sys/bus/esoc/devices"
#define QSC_CHECKER "/data/misc/mdmhelperdata/qschelpsocket"

/* Generic subsystem ramdump */
#define DUMP_SDCARD_DIR "/sdcard/ramdump"
#define DUMP_SMEM_STR "ramdump_smem"
#define DUMP_MODEM_STR "ramdump_modem"
#define DUMP_MODEM_SW_STR "ramdump_modem_sw"
#define DUMP_AUDIO_OCMEM_STR "ramdump_audio-ocmem"

/* QDSS ramdump */
#define QDSS_ETF_BUFSIZE 0x00010000
#define QDSS_ETF_CUR_SINK "/sys/bus/coresight/devices/coresight-tmc-etf/curr_sink"
#define QDSS_ETF_DUMP "coresight-tmc-etf"
#define QDSS_ETR_BUFSIZE 0x00100000
#define QDSS_ETR_CUR_SINK "/sys/bus/coresight/devices/coresight-tmc-etr/curr_sink"
#define QDSS_ETR_OUT_MODE "/sys/bus/coresight/devices/coresight-tmc-etr/out_mode"
#define QDSS_ETR_DUMP "coresight-tmc-etr"

/* SSR events */
#define SSR_EVENT_MDM_RAMDUMP 0x00000001
#define SSR_EVENT_QSC_RAMDUMP 0x00000002
#define SSR_EVENT_GET_RPM_LOG 0x00000004
#define SSR_EVENT_MDM_RESTART 0x00000008
#define SSR_EVENT_QSC_RESTART 0x00000010

/* List of log dev */
char *LOG_LIST[LOG_NUM] = {
	"/sys/kernel/debug/rpm_log"
};

/* Log output files */
char *LOG_NODES[LOG_NUM] = {
	"/rpm_log"
};

static struct pollfd plogfd[LOG_NUM];

static sem_t ramdump_sem;
static sem_t qdss_sem;
static unsigned int rpm_log;
static unsigned int qdss_dump;

static int parse_args(int argc, char **argv);
static int ssr_copy_dump_based_on_type(char *ramdump_path);
#ifdef ANDROID_BUILD
#define BUILD_FAVOR 1
#include <mdm_detect.h>
#define LOG_TAG "subsystem_ramdump"
#define LOG_NIDEBUG 0
#include <cutils/log.h>
#define DUMP_NUM 10
#else
#define BUILD_FAVOR 0
#define RET_SUCCESS 0
#define RET_FAILED 1
#define MAX_PATH_LEN 255
#define MAX_SUPPORTED_MDM 4
#include <syslog.h>
  #ifdef USE_GLIB
  #include <glib/gprintf.h>
  #define strlcat g_strlcat
  #define strlcpy g_strlcpy
  #endif
#define DUMP_NUM 1
#define ALOGE(format, ...) syslog(LOG_ERR, format, ## __VA_ARGS__)
#define ALOGI(format, ...) syslog(LOG_INFO, format, ## __VA_ARGS__)
#define ALOGW(format, ...) syslog(LOG_WARNING, format, ## __VA_ARGS__)
#define get_system_info(devinfo) 0
typedef enum MdmType {
	MDM_TYPE_EXTERNAL = 0,
	MDM_TYPE_INTERNAL,
} MdmType;

struct mdm_info {
	MdmType type;
		char ram_dump_path[MAX_PATH_LEN];
};

struct dev_info {
	int num_modems;
	struct mdm_info mdm_list[MAX_SUPPORTED_MDM];
};
#endif

#define BUFFER_SIZE 0x00008000
#define WLAN_RAMDUMP_DIR "/dev/ramdump_AR6320"
#define WLAN_RAMDUMP_FILE "ramdump_AR6320"

#define FILENAME_SIZE 60
#define PIL_NAME_SIZE 30
#define TIMEBUF_SIZE 21
#define MAX_STR_LEN 36
#define CHK_ELF_LEN 5

#define PIL_RAMDUMP_DIR "/dev"

#define SSR_CONF_FILE "/etc/ssr.conf"
#define SUBSYS_DIR "/sys/bus/msm_subsys/devices"
#define DUMP_SETTING "/sys/module/subsystem_restart/parameters/enable_ramdumps"
#define DUMP_EMMC_DIR "/data/ramdump"
#define DUMP_HEAD_STR "ramdump_"
#define DUMP_TAIL_BIN ".bin"
#define DUMP_TAIL_ELF ".elf"
#define STR_ELF "ELF"

typedef struct {
	int fd[DUMP_NUM];
	int dev[DUMP_NUM];
	char *dir;
} ramdump_s;

char ramdump_list[DUMP_NUM][PIL_NAME_SIZE];

static ramdump_s ramdump;

/* Poll struct */
static struct pollfd pfd[DUMP_NUM];

static unsigned int ssr_flag;
static int pfd_num;

/*==========================================================================*/
/* Local Function declarations */
/*==========================================================================*/
int generate_ramdump(int index, ramdump_s *dump, char *tm);

int check_folder(char *f_name)
{
	int ret = 0;
	struct stat st;

	if ((ret = stat(f_name, &st)) != 0)
		ALOGE( "Directory %s does not exist", f_name);

	return ret;
}

int create_folder(char *f_name)
{
	int ret = 0;

	if ((ret = mkdir(f_name, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		ALOGE("Unable to create %s", f_name);

	return ret;
}

char *get_current_timestamp(char *buf, int len)
{
	time_t local_time;
	struct tm *tm;

	if (buf == NULL || len < TIMEBUF_SIZE) {
		ALOGE("Invalid timestamp buffer");
		goto get_timestamp_error;
	}

	/* Get current time */
	local_time = time(NULL);
	if (!local_time) {
		ALOGE("Unable to get timestamp");
		goto get_timestamp_error;
	}

	tm = localtime(&local_time);
	if (!tm) {
		ALOGE("Unable to get local time");
		goto get_timestamp_error;
	}

	snprintf(buf, TIMEBUF_SIZE,
		"_%04d-%02d-%02d_%02d-%02d-%02d", tm->tm_year+1900,
		tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min,
		tm->tm_sec);

	return buf;

get_timestamp_error:
	return NULL;
}

void ssr_ramdump_signal_handler(int sig)
{
	switch (sig) {
	case SIGUSR1:
	case SIGUSR2:
	case SIGTERM:
		/* call thread exits */
		pthread_exit(NULL);
		break;

	default:
		break;
	}
}

int ssr_stop_thread(pthread_t thread_id, int sig)
{
	int ret = 0;

	/* Signal the thread to exit */
	ret = pthread_kill(thread_id, sig);
	if (ret != 0) {
		ALOGE("Unable to terminate thread %lu", thread_id);
	} else {
		if ((ret = pthread_join(thread_id, NULL)) != 0) {
			ALOGE("pthread_join failed for thread %lu",
								thread_id);
		}
	}

	return ret;
}

int copy_file(char *out_f, char *in_f)
{
	int ret = 0;
	int in, out;
	int rbytes;
	char buf[BUFFER_SIZE];

	if (out_f == NULL || in_f == NULL) {
		ret = -EINVAL;
		goto skip_copy;
	}

	/* check if source file exist */
	if (access(in_f, F_OK) != 0) {
		ret = -EINVAL;
		goto skip_copy;
	}

	/* make sure output file doesn't exist before creation */
	remove(out_f);

	in = open(in_f, O_RDONLY);
	if (in < 0) {
		ALOGE("Unable to open %s", in_f);
		ret = -EIO;
		goto skip_copy;
	}

	out = open(out_f, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (out < 0) {
		ALOGE("Unable to open %s", out_f);
		ret = -EIO;
		goto open_error;
	}

	while ((rbytes = read(in, buf, BUFFER_SIZE)) > 0) {
		ret = write(out, buf, rbytes);
		if (ret < 0) {
			ALOGE("Unable to write to file %s", out_f);
			break;
		}
	}
	close(out);

open_error:
	close(in);

skip_copy:
	return ret;
}

static int ssr_copy_dump(char *dst, char *src, char *tm)
{
	int ret = 0;
	char dir[FILENAME_SIZE];
	char out_file[FILENAME_SIZE];
	char in_file[FILENAME_SIZE];
	struct dirent *dir_p;
	DIR *dir_s;

        if((dst == NULL) || (src == NULL) || (tm == NULL)){
		ret = -EINVAL;
		goto skip_copy;
        }
        strlcpy(dir, ramdump.dir, FILENAME_SIZE);
        strlcat(dir, dst, FILENAME_SIZE);
	strlcat(dir, tm, FILENAME_SIZE);
	ALOGI("creating directory %s", dir);
	if (create_folder(dir) < 0) {
		ALOGE("Unable to create directory %s", dir);
		ret = -EINVAL;
		goto skip_copy;
	}

	dir_s = opendir(src);
	if (!dir_s) {
		ALOGE("Unable to open %s", src);
		ret = -EINVAL;
		goto skip_copy;
	}
	while ((dir_p = readdir(dir_s)) != NULL) {
		if ((strncmp(dir_p->d_name, ".", 1) != 0) &&
				(strncmp(dir_p->d_name, "..", 2) != 0)) {
			strlcpy(out_file, dir, FILENAME_SIZE);
			strlcat(out_file, "/", FILENAME_SIZE);
			strlcat(out_file, dir_p->d_name, FILENAME_SIZE);

			strlcpy(in_file, src, FILENAME_SIZE);
			strlcat(in_file, "/", FILENAME_SIZE);
			strlcat(in_file, dir_p->d_name, FILENAME_SIZE);

			if (copy_file(out_file, in_file) < 0) {
				ret = -EINVAL;
				break;
			}
		}
	}
	closedir(dir_s);

	if (ret < 0)
		remove(dir);

skip_copy:
	return ret;
}

static int ssr_copy_dump_based_on_type(char *ramdump_path)
{
	int ret = 0;
	char timestamp[TIMEBUF_SIZE];
	struct stat st;
	int dumpFlag = 0;

	/* get current time */
	if (get_current_timestamp(timestamp, TIMEBUF_SIZE) == NULL) {
		ALOGE("Unable to get timestamp");
		return RET_FAILED;
	}
	if (stat(ramdump_path, &st) != 0) {
		ALOGE("Ramdump directory %s does not exist",
			ramdump_path);
		return RET_FAILED;
	}

	if (ssr_flag & SSR_EVENT_MDM_RAMDUMP){
		dumpFlag |= SSR_EVENT_MDM_RAMDUMP;
	}

	if (ssr_flag & SSR_EVENT_QSC_RAMDUMP){
		dumpFlag |= SSR_EVENT_QSC_RAMDUMP;
	}

	if (dumpFlag == SSR_EVENT_MDM_RAMDUMP) {
		/* MDM ramdump only */
		ret = ssr_copy_dump(STR_MDM, ramdump_path, timestamp);
		if (ret != 0){
			ALOGE("MDM ramdump failed");
			return RET_FAILED;
		}
		else
			ALOGI("MDM ramdump completed");
	} else if (dumpFlag == SSR_EVENT_QSC_RAMDUMP) {
		/* QSC ramdump only */
		ret = ssr_copy_dump(STR_QSC, ramdump_path, timestamp);
		if (ret != 0){
			ALOGE("QSC ramdump failed");
			return RET_FAILED;
		}
		else
			ALOGI("QSC ramdump completed");
		}
	else if (dumpFlag == ((SSR_EVENT_MDM_RAMDUMP)|(SSR_EVENT_QSC_RAMDUMP))){
		/* MDM and QSC ramdump */
		if (stat(EXT_MDM2_DIR, &st) != 0) {
			ALOGE("Directory %s does not exist",
							EXT_MDM2_DIR);
			return RET_FAILED;
		}

		ret = ssr_copy_dump(STR_MDM, ramdump_path, timestamp);
		if (ret != 0){
			ALOGE("MDM ramdump failed");
			return RET_FAILED;
		}
		else
			ALOGI("MDM ramdump completed");
			ret = ssr_copy_dump(STR_QSC, EXT_MDM2_DIR, timestamp);
			if (ret != 0){
				ALOGE("QSC ramdump failed");
				return RET_FAILED;
			}
			else
				ALOGI("QSC ramdump completed");
	}
	else{
		ALOGE("Unknown ramdump type %d", dumpFlag);
		return RET_FAILED;
	}
	return RET_SUCCESS;
}

static void *ext_modem_mon(void* param)
{
    char* ramdump_path = NULL;
    ramdump_path = (char*)malloc(MAX_PATH_LEN * sizeof(char));
	if (!ramdump_path) {
		ALOGE("Fail to allocate memory for ramdump path");
		goto cleanup_ramdump_path;
	}

	while (1) {
		/* wait external modem ramdump ready */
		sem_wait(&ramdump_sem);
                ALOGI("modem ramdump semaphore ready");

		/*
		* External modem ramdump should be collected after a completed
		* restart. Don't collect legacy ramdumps
		*/
		if (!(ssr_flag & SSR_EVENT_MDM_RESTART) &&
					!(ssr_flag & SSR_EVENT_QSC_RESTART))
			continue;
		ssr_flag &= ~(SSR_EVENT_MDM_RESTART | SSR_EVENT_QSC_RESTART);

		/* check ramdump source location */
        struct dev_info devinfo;
		devinfo.num_modems = 0;
        int i = 0;
        int num_esoc_ramdump = 0;
        strlcpy(ramdump_path, EXT_MDM_DIR, (MAX_PATH_LEN*sizeof(char)));

        if(get_system_info(&devinfo) != 0){
			ALOGW("unable to get esoc system info");
        }
		// if esoc ramdumps are available; copy the ramdumps over
		if ( devinfo.num_modems > 0 ){
			for( i = 0; i < devinfo.num_modems; i++){
				if (devinfo.mdm_list[i].type == MDM_TYPE_EXTERNAL){
					num_esoc_ramdump++;
					strlcpy(ramdump_path,
						devinfo.mdm_list[i].ram_dump_path,
						(MAX_PATH_LEN*sizeof(char)));
                    ALOGI("ramdump path is %s", ramdump_path);
					if((ssr_copy_dump_based_on_type(ramdump_path))!= RET_SUCCESS){
						ALOGE("Fail to copy ramdump to specified location");
						break;
					}
				}
			}
        }
		else
		{
			if((ssr_copy_dump_based_on_type(ramdump_path))!=RET_SUCCESS){
				ALOGI("ramdump path is %s", ramdump_path);
				ALOGE("Fail to copy ramdump to specified location");
				break;
			}
		}
	}
cleanup_ramdump_path:
	if(ramdump_path)
		free(ramdump_path);
	return NULL;
}

int open_uevent(void)
{
	struct sockaddr_nl addr;
	int sz = UEVENT_BUF_SIZE;
	int s;

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = 0xffffffff;

	/*
	*	netlink(7) on nl_pid:
	*	If the application sets it to 0, the kernel takes care of
	*	assigning it.
	*	The kernel assigns the process ID to the first netlink socket
	*	the process opens and assigns a unique nl_pid to every netlink
	*	socket that the process subsequently creates.
	*/
	addr.nl_pid = getpid();

	s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if(s < 0) {
		ALOGE("Creating netlink socket failed");
		return -1;
	}

	setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

	if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		ALOGE("Binding to netlink socket failed");
		close(s);
		return -1;
	}

	return s;
}

int uevent_next_event(int fd, char* buffer, int buffer_length)
{
	struct pollfd fds;
	int nr;
	int count;

	while (1) {
		fds.fd = fd;
		fds.events = POLLIN;
		fds.revents = 0;
		nr = poll(&fds, 1, -1);

		if (nr > 0 && fds.revents == POLLIN) {
			count = recv(fd, buffer, buffer_length, 0);
			if (count > 0)
				return count;
			ALOGE("Receiving uevent failed");
		}
	}

	return 0;
}

static void *uevent_mon(void* param)
{
	int i;
	int ufd;
	int count;
	char uevent_buf[UEVENT_BUF_SIZE];

	/* open uevent fd */
	ufd = open_uevent();
	if (ufd < 0) {
		ALOGE("Failed to initialize uevent");
		return NULL;
	}

	while (1) {
		/* Listen for user space event */
		count = uevent_next_event(ufd, uevent_buf, UEVENT_BUF_SIZE);
		if (!count)
			break;

		/*
		* Look for a completed MDM restart
		* MDM power down event: set restart flag
		* MDM power up event: post to semaphore
		*/
		for (i = 0; i < MAX_STR_LEN; i++) {
			if (*(uevent_buf + i) == '\0')
				break;
		}
		if (i == MAX_STR_LEN)
			*(uevent_buf + i) = '\0';

		if (strstr(uevent_buf, UEVENT_MDM_SHUTDOWN)) {
			ssr_flag |= SSR_EVENT_MDM_RESTART;
			if (rpm_log)
				ssr_flag |= SSR_EVENT_GET_RPM_LOG;
		} else if (strstr(uevent_buf, UEVENT_MDM_POWERUP)) {
			sem_post(&ramdump_sem);
		}
	}

        ALOGI("ssr_flag is %u", ssr_flag );
	close(ufd);

	return NULL;
}

int generate_log(char *buf, int len)
{
	int ret = 0;
	int fd;
	char name_buf[FILENAME_SIZE];
	char timestamp[TIMEBUF_SIZE];

	/* get current time */
	if (get_current_timestamp(timestamp, TIMEBUF_SIZE) == NULL) {
		ALOGE("Unable to get timestamp for log");
		ret = -EINVAL;
		goto timestamp_error;
	}

	/* Assemble output log file */
	strlcpy(name_buf, ramdump.dir, FILENAME_SIZE);
	strlcat(name_buf, LOG_NODES[0], FILENAME_SIZE);
	strlcat(name_buf, timestamp, FILENAME_SIZE);
	strlcat(name_buf, DUMP_TAIL_BIN, FILENAME_SIZE);
	ALOGI("The output file is %s", name_buf);

	/* Open the output file */
	fd = open(name_buf, O_WRONLY | O_CREAT, S_IRUSR);
	if (fd < 0) {
		ALOGE("Unable to open log file %s", name_buf);
		ret = -EIO;
		goto open_error;
	}

	/* write to file */
	ret = write(fd, buf, len);
	if (ret < 0) {
		ALOGE("Unable to write to log file %s", name_buf);
		goto write_error;
	}

	/* Make sure things are written */
	fsync(fd);

write_error:
	close(fd);

open_error:
timestamp_error:
	return ret;
}

static void *rpm_log_thread(void* param)
{
	int ret;
	int count = 0;
	char *read_buf;
	char *rpm_log_buf;
	int rpm_log_len = 0;

	rpm_log_buf = (char *)malloc(RPMLOG_BUF_SIZE);
	if (rpm_log_buf == NULL) {
		ALOGE("Failed to allocate memory to collect rpm log");
		return NULL;
	}

	while (1) {
		/* Poll RPM log */
		if ((ret = poll(plogfd, LOG_NUM, -1)) < 0) {
			ALOGE("Polling rpm log fd failed: %d", ret);
			break;
		}

		read_buf = malloc(BUFFER_SIZE);
		if (read_buf == NULL) {
			ALOGE("Failed to allocate memory to read rpm log");
			break;
		}

		/* Collect RPM log */
		count = read(plogfd[0].fd, read_buf, BUFFER_SIZE);

		if ((rpm_log_len + count) >= RPMLOG_BUF_SIZE)
			count = RPMLOG_BUF_SIZE - rpm_log_len;
		if (count) {
			memcpy(rpm_log_buf + rpm_log_len, read_buf, count);
			rpm_log_len += count;
		}

		/* Store rpm log while subystem crash */
		if (ssr_flag & SSR_EVENT_GET_RPM_LOG) {
			ssr_flag &= ~SSR_EVENT_GET_RPM_LOG;
			if (generate_log(rpm_log_buf, rpm_log_len) < 0) {
				ALOGE("Failed to generate rpm log");
				free(read_buf);
				break;
			}
			rpm_log_len = 0;
		}

		if (rpm_log_len >= RPMLOG_BUF_SIZE)
			rpm_log_len = 0;

		free(read_buf);
	}
	free(rpm_log_buf);

	return NULL;
}

int log_init(void)
{
	int ret = 0;

	/* check if log device exist */
	if (access(LOG_LIST[0], F_OK) != 0) {
		ret = -EIO;
		goto open_error;
	}

	plogfd[0].fd = open(LOG_LIST[0], O_RDONLY);
	if (plogfd[0].fd < 0) {
		ALOGE("Unable to open %s", LOG_LIST[0]);
		ret = -EIO;
		goto open_error;
	}
	plogfd[0].events = POLLIN;
	plogfd[0].revents = 0;

open_error:
	return ret;
}

static void ssr_pil_ramdump_notification(int id)
{
	/* RPM log collection only needs to do once for each SSR. Skip
	 * duplicating SSR notification
	 */
	if ((strncmp(ramdump_list[id], DUMP_SMEM_STR, 12) == 0) ||
		(strncmp(ramdump_list[id], DUMP_AUDIO_OCMEM_STR, 19) == 0) ||
		(strncmp(ramdump_list[id], DUMP_MODEM_SW_STR, 16) == 0))
		return;

	/* Collect QSC ramdump for modem ssr */
	if (strncmp(ramdump_list[id], DUMP_MODEM_STR, 13) == 0) {
		if (ssr_flag & SSR_EVENT_QSC_RAMDUMP) {
			ssr_flag |= SSR_EVENT_QSC_RESTART;
			sem_post(&ramdump_sem);
		}
	}

	/* Collect RPM log */
	if (rpm_log)
		ssr_flag |= SSR_EVENT_GET_RPM_LOG;

	/* Collect qdss dump */
	if (qdss_dump)
		sem_post(&qdss_sem);
}

static int ssr_check_ext_modem(char *chk_str)
{
	int ret = 0;

	if (access(chk_str, F_OK) != 0)
		ret = -EINVAL;

	return ret;
}

char *ssr_ramdump_filename(int index, ramdump_s *dump, char *name, char *tm, int type)
{
	strlcpy(name, dump->dir, FILENAME_SIZE);
	strlcat(name, "/", FILENAME_SIZE);
	strlcat(name, ramdump_list[dump->dev[index]], FILENAME_SIZE);
	strlcat(name, tm, FILENAME_SIZE);
	if (type)
		strlcat(name, DUMP_TAIL_ELF, FILENAME_SIZE);
	else
		strlcat(name, DUMP_TAIL_BIN, FILENAME_SIZE);

	return name;
}

int open_ramdump_fd(ramdump_s *dump)
{
	int ret = 0;
	int i;
	int fd;
	char buf[FILENAME_SIZE];

	for (i = 0; i < pfd_num; i++) {
		strlcpy(buf, PIL_RAMDUMP_DIR, FILENAME_SIZE);
		strlcat(buf, "/", FILENAME_SIZE);
		strlcat(buf, ramdump_list[i], FILENAME_SIZE);

		if ((fd = open(buf, O_RDONLY)) < 0) {
			ALOGE("Unable to open %s", buf);
			ret = -EINVAL;
			break;
		}

		/* store open fd */
		dump->fd[i] = fd;
		dump->dev[i] = i;
	}

	if (ret < 0) {
		for (i = 0; i < pfd_num; i++) {
			if (dump->fd[i])
				close(dump->fd[i]);
		}
	}

	return ret;
}

static int ssr_check_ramdump_setting(void)
{
	int ret = 0;
	int fd;
	char buf[2];

	if ((fd = open(DUMP_SETTING, O_RDONLY)) < 0) {
		ALOGE("Unable to open %s", DUMP_SETTING);
		ret = fd;
		goto open_error;
	}

	if (read(fd, buf, 2) <= 0) {
		ALOGE("Unable to read %s", DUMP_SETTING);
		ret = -EIO;
		goto read_error;
	}

	if (buf[0] == '1') {
		ret = 0;
	} else if (buf[0] == '0') {
		ret = -EINVAL;
	} else {
		ALOGE("Invalid ramdump setting %s", buf);
		ret = -EINVAL;
	}

read_error:
	close(fd);

open_error:
	return ret;
}

static int ssr_ramdump_init(ramdump_s *dump, int androidBuild )
{
	int ret = 0;
	int i;

	/* check if ramdump folder can be created */
	if (check_folder(dump->dir) != 0) {
		ALOGI("Attemping to create %s", dump->dir);
		if ((ret = create_folder(ramdump.dir)) < 0) {
			ALOGE("Unable to create %s", dump->dir);
			goto open_error;
		}
	}

	if ( androidBuild ) {
		/* init ramdump semaphore */
		if (sem_init(&ramdump_sem, 0, 0) != 0) {
			ret = -EINVAL;
			goto ramdump_semaphore_error;
		}

		/* init qdss semaphore */
		if (sem_init(&qdss_sem, 0, 0) != 0) {
			ret = -EINVAL;
			goto qdss_semaphore_error;
		}
	}

	/* init ramdump parameters */
	for (i = 0; i < DUMP_NUM; i++) {
		dump->fd[i] = -1;
		dump->dev[i] = -1;
	}

	/* open fd in ramdump list */
	if ((ret = open_ramdump_fd(dump)) < 0) {
		ALOGE("Failed to open ramdump devices");
		goto open_ramdump_error;
	}

	/* Store fd to polling struct */
	for (i = 0; i < pfd_num; i++) {
		pfd[i].fd = dump->fd[i];
		pfd[i].events = POLLIN;
		pfd[i].revents = 0;
	}
	ssr_flag = 0;

	goto exit;

open_ramdump_error:
	if ( androidBuild ){
	 sem_destroy(&qdss_sem);}

qdss_semaphore_error:
	if ( androidBuild ){
	sem_destroy(&ramdump_sem);}
ramdump_semaphore_error:
open_error:
	dump->dir = NULL;

exit:
	return ret;
}

static int ssr_search_pil_ramdump(void)
{
	int ret = 0;
	int i = 0;
	struct dirent *dir_p;
	DIR *dir_s;

	dir_s = opendir(PIL_RAMDUMP_DIR);
	if (!dir_s) {
		ALOGE("Unable to open %s", PIL_RAMDUMP_DIR);
		ret = -EINVAL;
		goto open_error;
	}

	while ((dir_p = readdir(dir_s)) != NULL) {
		if (strncmp(dir_p->d_name, DUMP_HEAD_STR, 8) == 0) {
			if (i >= DUMP_NUM) {
				ALOGE("Exceed max pil number");
				pfd_num = 0;
				ret = -EINVAL;
				goto invalid_num;
			}
			strlcpy(ramdump_list[i], dir_p->d_name, PIL_NAME_SIZE);
			i++;
		}
	}

	/* Store pil ramdump number */
	pfd_num = i;

invalid_num:
	closedir(dir_s);

open_error:
	return ret;
}

static int ssr_search_pil_ramdump_le(void)
{
	if (access(WLAN_RAMDUMP_DIR, F_OK) < 0) {
		return -EINVAL;
	}
	strlcpy(ramdump_list[0], WLAN_RAMDUMP_FILE, PIL_NAME_SIZE);
	pfd_num = 1;

	return 0;
}

static int ssr_ramdump_cleanup(int num, int androidBuild )
{
	int i;

	for (i = 0; i < num; i++)
		close(pfd[i].fd);

	if (androidBuild){
		sem_destroy(&ramdump_sem);
		sem_destroy(&qdss_sem);
	}
	ramdump.dir = NULL;

	return 0;
}

static int ssr_qdss_dump(char *dev, int sz)
{
	int ret = 0;
	int qdss_fd, file_fd;
	int rd_bytes = 0;
	int sum_bytes = 0;
	char *rd_buf;
	char qdss_name[FILENAME_SIZE];
	char file_name[FILENAME_SIZE];
	char timestamp[TIMEBUF_SIZE];

	if ((dev == NULL) || (sz <= 0)) {
		ALOGE("Invalid qdss device or size");
		ret = -EINVAL;
		goto skip_copy;
	}

	if ((rd_buf = malloc(sz)) == NULL) {
		ALOGE("Failed to allocate memory to read qdss");
		ret = -ENOMEM;
		goto skip_copy;
	}

	/* Open qdss etf/etr dev */
	strlcpy(qdss_name, PIL_RAMDUMP_DIR, FILENAME_SIZE);
	strlcat(qdss_name, "/", FILENAME_SIZE);
	strlcat(qdss_name, dev, FILENAME_SIZE);
	if ((qdss_fd = open(qdss_name, O_RDONLY)) < 0) {
		ALOGE("Unable to open %s", qdss_name);
		ret = qdss_fd;
		goto open_error;
	}

	/* Get current time */
	if (get_current_timestamp(timestamp, TIMEBUF_SIZE) == NULL) {
		ALOGE("Unable to get timestamp");
		ret = -EINVAL;
		goto timestamp_error;
	}

	/* Open output file name */
	strlcpy(file_name, ramdump.dir, FILENAME_SIZE);
	strlcat(file_name, "/", FILENAME_SIZE);
	strlcat(file_name, dev, FILENAME_SIZE);
	strlcat(file_name, timestamp, FILENAME_SIZE);
	strlcat(file_name, DUMP_TAIL_BIN, FILENAME_SIZE);
	if ((file_fd = open(file_name, O_WRONLY | O_CREAT, S_IRUSR)) < 0) {
		ALOGE("Unable to open %s", file_name);
		ret = file_fd;
		goto file_error;
	}

	/* Read QDSS ramdump and write to stroage location */
	while ((rd_bytes = read(qdss_fd, rd_buf, BUFFER_SIZE)) > 0) {
		if (write(file_fd, rd_buf, rd_bytes) < 0) {
			ALOGE("Writing qdss ramdump error");
			ret = -EIO;
			goto write_error;
		}
		sum_bytes += rd_bytes;
	}

	/* Make sure things are written */
	fsync(file_fd);

	if (sum_bytes == sz) {
		ALOGI("qdss %s ramdump saved successfully", dev);
		ret = 0;
	} else {
		ALOGE("qdss %s ramdump size mismatch", dev);
		ret = -EINVAL;
	}

write_error:
	close(file_fd);

file_error:
timestamp_error:
	close(qdss_fd);

open_error:
	free(rd_buf);

skip_copy:
	return ret;
}

static int ssr_do_qdss_etf_dump(void)
{
	int ret = 0;
	int fd;
	char buf[2];

	if ((fd = open(QDSS_ETF_CUR_SINK, O_RDONLY)) < 0) {
		ALOGE("Unable to open %s", QDSS_ETF_CUR_SINK);
		ret = fd;
		goto open_error;
	}

	if (!read(fd, buf, 2)) {
		ALOGE("Unable to read qdss etf data");
		ret = -EIO;
		goto read_error;
	}

	if (buf[0] == '1')
		ret = ssr_qdss_dump(QDSS_ETF_DUMP, QDSS_ETF_BUFSIZE);

read_error:
	close(fd);

open_error:
	return ret;
}

static int ssr_do_qdss_etr_dump(void)
{
	int ret = 0;
	int sink_fd, mode_fd;
	int flag = 0;
	char sink_buf[2], mode_buf[4];

	if ((sink_fd = open(QDSS_ETR_CUR_SINK, O_RDONLY)) < 0) {
		ALOGE("Unable to open %s", QDSS_ETR_CUR_SINK);
		ret = sink_fd;
		goto skip_copy;
	}

	if (!read(sink_fd, sink_buf, 2)) {
		ALOGE("Unable to read qdss etr data");
		ret = -EIO;
		goto skip_read;
	}

	if (sink_buf[0] == '1')
		flag = 1;
	if (!flag)
		goto skip_read;

	if ((mode_fd = open(QDSS_ETR_OUT_MODE, O_RDONLY)) < 0) {
		ALOGE("Unable to open %s", QDSS_ETR_OUT_MODE);
		ret = mode_fd;
		goto open_error;
	}

	if (!read(mode_fd, mode_buf, 4)) {
		ALOGE("Unable to read qdss etr mode data");
		ret = -EIO;
		goto read_error;
	}
	if (strncmp(mode_buf, "mem", 3) != 0) {
		ALOGE("Invalid qdss etr out_mode %s", mode_buf);
		ret = -EINVAL;
		goto read_error;
	}

	ret = ssr_qdss_dump(QDSS_ETR_DUMP, QDSS_ETR_BUFSIZE);

read_error:
	close(mode_fd);

open_error:
skip_read:
	close(sink_fd);

skip_copy:
	return ret;
}

static void *qdss_mon(void* param)
{
	int ret;

	while (1) {
		/* wait ssr happened */
		sem_wait(&qdss_sem);

		if ((ret = ssr_do_qdss_etf_dump()) < 0) {
			ALOGE("Failed to save qdss etf dump");
			break;
		}

		if ((ret = ssr_do_qdss_etr_dump()) < 0) {
			ALOGE("Failed to save qdss etr dump");
			break;
		}
	}

	return NULL;
}

static void ssr_tool_helper(void)
{
	ALOGI("Usage:./system/bin/subsystem_ramdump [arg1] [arg2] [arg3]");
	ALOGI("[arg1]: (1/2) Ramdump location: 1: eMMC: /data/ramdump or 2: SD card: /sdcard/ramdump");
	ALOGI("[arg2]: (1/0) 1: Enable RPM log / 0: Disable RPM log");
	ALOGI("[arg3]: (1/0) 1: Enable qdss ramdump / 0: Disable qdss ramdump");
}

static int parse_args(int argc, char **argv)
{
	int ret = 0;
	int location, rpmlog, qdss;

	ssr_tool_helper();
	if (argc == 1) {
		// Use default ramdump location; disable rpm log ; disable qdss ramdump;
		ALOGI(" Using default ramdump location 1: eMMC: /data/ramdump");
		ALOGI(" Using default 0: Disable RPM log");
		ALOGI(" Using default 0: Disable qdss ramdump");

                location = 1;
		rpmlog = 0;
		qdss = 0;
	} else if (argc == 2) {
		/* Disable RPM log and qdss ramdump by default */
		sscanf(argv[1], "%d", &location);
		rpmlog = 0;
		qdss = 0;
	} else if (argc == 3) {
		/* Disable qdss ramdump by default */
		sscanf(argv[1], "%d", &location);
		sscanf(argv[2], "%d", &rpmlog);
		qdss = 0;
	} else if (argc == 4) {
		sscanf(argv[1], "%d", &location);
		sscanf(argv[2], "%d", &rpmlog);
		sscanf(argv[3], "%d", &qdss);
	} else {
		ALOGE("Too many input arguments");
		ret = -EINVAL;
		goto err_exit;
	}

	if (location == 1) {
		ramdump.dir = DUMP_EMMC_DIR;
	} else if (location == 2) {
		ramdump.dir = DUMP_SDCARD_DIR;
	} else {
		ALOGE("Invalid ramdump storage setting");
		ret = -EINVAL;
		goto err_exit;
	}

	if (rpmlog == 1) {
		rpm_log = 1;
	} else if (rpmlog == 0) {
		rpm_log = 0;
	} else {
		ALOGE("Invalid RPM log setting");
		ret = -EINVAL;
	}

	if (qdss == 1) {
		qdss_dump = 1;
	} else if (qdss == 0) {
		qdss_dump = 0;
	} else {
		ALOGE("Invalid qdss ramdump setting");
		ret = -EINVAL;
	}

err_exit:
	return ret;
}

static void enable_wlan_ssr_for_le(void)
{
	int fd;
	struct stat s;
	DIR *ssr_bus;
	struct dirent *de;
	char subsys_name[FILENAME_SIZE];
	char name_dir[MAX_PATH_LEN];
	char rlevel_dir[MAX_PATH_LEN];

	if (stat(SSR_CONF_FILE, &s))
		return;

	if ((ssr_bus = opendir(SUBSYS_DIR)) == 0) {
		ALOGE("Unable to open %s directory", SUBSYS_DIR);
		return;
	}

	while((de = readdir(ssr_bus))){
		if (de->d_name[0] == '.')
			continue;

		snprintf(name_dir, sizeof(name_dir), "%s/%s/name", SUBSYS_DIR,
				de->d_name);

		if ((fd = open(name_dir, O_RDONLY)) < 0) {
			ALOGE("Fail to open name directory %s:%s", name_dir, strerror(errno));
			continue;
		}

		memset(subsys_name, 0, FILENAME_SIZE);
		if (read(fd, subsys_name, FILENAME_SIZE) < 0) {
			ALOGE("Fail to read subsys name %s:%s", subsys_name, strerror(errno));
			close(fd);
			continue;
		}
		close(fd);

		if(!strncmp( subsys_name, "AR6320\n", 7)) {
			snprintf(rlevel_dir, sizeof(rlevel_dir),
				"%s/%s/restart_level", SUBSYS_DIR, de->d_name);

			if ((fd = open(rlevel_dir, O_WRONLY)) < 0) {
				ALOGE("Fail to open restart level directory");
				break;
			}
			if (write(fd, "RELATED", 7 ) < 0) {
				ALOGE("cannot enable wlan ssr");
			}
			ALOGI("wlan ssr enabled");
			close(fd);
			break;
		}
	}
	closedir(ssr_bus);

	if ((fd = open(DUMP_SETTING, O_WRONLY)) < 0) {
		ALOGE("Cannot open %s", DUMP_SETTING);
		return;
	}
	if (write(fd, "1", 1) < 0) {
		ALOGE("Cannot enable subsystem_ramdump");
	} else {
		ALOGI("subsystem ramdump enabled");
	}
	close(fd);
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int i;
	char timestamp[TIMEBUF_SIZE];
	static int androidBuild = BUILD_FAVOR;

	// define event threads
	pthread_t uevent_thread_hdl = 0;
	pthread_t ext_modem_thread_hdl = 0;
	pthread_t rpm_log_thread_hdl = 0;
	pthread_t qdss_thread_hdl = 0;
	struct sigaction action;

	if (!androidBuild) {
		enable_wlan_ssr_for_le();
	}

	if ((ret = parse_args(argc, argv)) < 0) {
		ALOGE("Invalid arguments");
		return ret;
	}

	/* Exit if ramdump is not enable */
	if ((ret = ssr_check_ramdump_setting()) < 0) {
		fprintf(stderr,"Ramdump is not enable\n");
		goto invalid_setting;
	}

	if (androidBuild){
		/* Search PIL ramdumps */
		if ((ret = ssr_search_pil_ramdump()) < 0) {
			ALOGE("Failed to find pil ramdump");
			goto ramdump_search_error;
		}

		if ((ret = ssr_ramdump_init(&ramdump, androidBuild)) < 0) {
			ALOGE("Failed to initialize ramdump");
			goto init_ramdump_error;
		}
		/* Register signal handlers */
		memset(&action, 0, sizeof(action));
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		action.sa_handler = ssr_ramdump_signal_handler;

		/* Check if esoc is supported and if external modem exists in esoc */
        struct dev_info devinfo;
        devinfo.num_modems = 0;
		int external_modem_found_in_esoc = 0;
		if (get_system_info(&devinfo)==RET_SUCCESS){
			for (i=0; i<devinfo.num_modems; i++){
				if( devinfo.mdm_list[i].type == MDM_TYPE_EXTERNAL ){
					external_modem_found_in_esoc = 1;
					ALOGI("External modem found in esoc");
					break;
				}
			}
        }

		if (((ret = ssr_check_ext_modem(MDM_CHECKER)) == 0) ||
            ((external_modem_found_in_esoc )) ) {

			/* create thread to listen uevent */
			ret = pthread_create(&uevent_thread_hdl, NULL, uevent_mon,
									NULL);

			if (ret != 0) {
				ALOGE("Creating uevent thread failed");
				goto mdm_listener_error;
			}
			ALOGI("Enable thread for uevent");
			ssr_flag |= SSR_EVENT_MDM_RAMDUMP;
		}

		if ((ret = ssr_check_ext_modem(QSC_CHECKER)) == 0)
			ssr_flag |= SSR_EVENT_QSC_RAMDUMP;

		/* create thread to copy ext modem dump */
		if ((ssr_flag & SSR_EVENT_MDM_RAMDUMP) ||
						(ssr_flag & SSR_EVENT_QSC_RAMDUMP)) {
			ret = pthread_create(&ext_modem_thread_hdl, NULL,
								ext_modem_mon, NULL);
			if (ret != 0) {
				ALOGE("Creating MDM monitor thread failed");
				goto modem_dumper_error;
			}
		}

		/* create thread to collect rpm log */
		if (rpm_log) {
			if ((ret = log_init()) != 0)
				goto rpm_log_init_error;

			ret = pthread_create(&rpm_log_thread_hdl, NULL, rpm_log_thread,
									NULL);
			if (ret != 0) {
				ALOGE("Creating rpm log thread failed");
				goto rpm_listener_error;
			}
		}

		/* create thread to collect qdss dump */
		if (qdss_dump) {
			ret = pthread_create(&qdss_thread_hdl, NULL, qdss_mon, NULL);
			if (ret != 0) {
				ALOGE("Creating qdss monitor thread failed");
				goto qdss_listener_error;
			}
		}
	}
    else { // LE build
		ramdump.dir = DUMP_EMMC_DIR;
		if ((ret = ssr_search_pil_ramdump_le()) < 0) {
			ALOGE("Failed to find pil ramdump");
			goto ramdump_search_error;
		}

		if ((ret = ssr_ramdump_init(&ramdump, androidBuild)) < 0) {
			ALOGE("Failed to initialize ramdump");
			goto init_ramdump_error;
		}
	}

	/* Loop forever and poll */
	while (1) {
		/* Poll all ramdump files */
		if ((ret = poll(pfd, pfd_num, -1)) < 0) {
			ALOGE("Polling ramdump failed: %d", ret);
			break;
		}

		/* Get current time */
		if (get_current_timestamp(timestamp, TIMEBUF_SIZE) == NULL) {
			ALOGE("Unable to get timestamp");
			break;
		}

		/* Collect ramdumps */
		for (i = 0; i < pfd_num; i++) {
			ret = generate_ramdump(i, &ramdump, timestamp);
			/* If ramdump generation is failed, exit the program */
			if (ret < 0) {
				ALOGE("Generating %s failed",
					ramdump_list[i]);
				if ( androidBuild ){
					goto ramdump_generation_error;
				} else {
					goto cleanup_le_ramdump;
				}
			}
		}
	}

ramdump_generation_error:
	if (qdss_dump)
		ret = ssr_stop_thread(qdss_thread_hdl, SIGUSR2);

qdss_listener_error:
	if (rpm_log)
		ret = ssr_stop_thread(rpm_log_thread_hdl, SIGUSR2);

rpm_listener_error:
	if (rpm_log)
		close(plogfd[0].fd);

rpm_log_init_error:
	if ((ssr_flag & SSR_EVENT_MDM_RAMDUMP) ||
					(ssr_flag & SSR_EVENT_QSC_RAMDUMP)) {
		ret = ssr_stop_thread(ext_modem_thread_hdl, SIGUSR2);
	}

modem_dumper_error:
	if (ssr_flag & SSR_EVENT_MDM_RAMDUMP)
		ret = ssr_stop_thread(uevent_thread_hdl, SIGUSR2);

mdm_listener_error:
cleanup_le_ramdump:
	ssr_ramdump_cleanup(pfd_num, androidBuild);

init_ramdump_error:
	ssr_flag = 0;

ramdump_search_error:
invalid_setting:
	return ret;
}

int generate_ramdump(int index, ramdump_s *dump, char *tm)
{
	int ret = 0;
	int numBytes = 0;
	int totalBytes = 0;
	int fd = -1;
	char *rd_buf;
	char f_name[FILENAME_SIZE];

	if (index < 0 || index >= DUMP_NUM) {
		ret = -EINVAL;
		goto skip_dump_generation;
	}

	/* Check to see if we have anything to do */
	if ((pfd[index].revents & POLLIN) == 0)
		goto skip_dump_generation;

	/* Notify other thread current ramdump */
	ssr_pil_ramdump_notification(dump->dev[index]);

	/* Allocate a buffer for us to use */
	if ((rd_buf = malloc(BUFFER_SIZE)) == NULL) {
		ret = -ENOMEM;
		goto skip_dump_generation;
	}

	/* Read first section of ramdump to determine type */
	while ((numBytes = read(pfd[index].fd, rd_buf, CHK_ELF_LEN)) > 0) {
		*(rd_buf + numBytes) = '\0';
		if (strstr(rd_buf, STR_ELF))
			ssr_ramdump_filename(index, dump, f_name, tm, 1);
		else
			ssr_ramdump_filename(index, dump, f_name, tm, 0);

		/* Open the output file */
		fd = open(f_name, O_WRONLY | O_CREAT, S_IRUSR);
		if (fd < 0) {
			ALOGE("Failed to open %s", f_name);
			ret = -EIO;
			goto open_error;
		}

		/* Write first section ramdump into file and exit loop */
		ret = write(fd, rd_buf, numBytes);
		if (ret < 0) {
			ALOGE("Failed to write to dump file %s",
								f_name);
			goto dump_write_error;
		}
		totalBytes += numBytes;
		break;
	}

	/* Read data from the ramdump, and write it into the output file */
	while ((numBytes = read(pfd[index].fd, rd_buf, BUFFER_SIZE)) > 0) {
		ret = write(fd, rd_buf, numBytes);
		if (ret < 0) {
			ALOGE("Failed to write to dump file %s",
								f_name);
			goto dump_write_error;
		}
		totalBytes += numBytes;
	}

	/* Make sure things are written */
	fsync(fd);

	/* Return the number of bytes written */
	ret = totalBytes;

dump_write_error:
	close(fd);

open_error:
	free(rd_buf);

skip_dump_generation:
	return ret;
}
