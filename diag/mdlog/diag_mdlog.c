// Copyright (c) 2007-2012 by Qualcomm Technologies, Inc.  All Rights Reserved.
//Qualcomm Technologies Proprietary and Confidential.

/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

			  Test Application for Diag Interface

GENERAL DESCRIPTION
  Contains main implementation of Diagnostic Services Test Application.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS
  

Copyright (c) 2007-2012, 2014 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

						EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: 

when       who    what, where, why
--------   ---     ----------------------------------------------------------
10/01/08   SJ     Created
===========================================================================*/

#include "event.h"
#include "msg.h"
#include "log.h"
#include "diag_lsm.h"
#include "diag_lsmi.h"
#include "diag_shared_i.h"
#include "stdio.h"
#include "string.h"
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <errno.h>
#ifndef FEATURE_LE_DIAG
#include <private/android_filesystem_config.h>
#endif

/*
 * strlcpy is from OpenBSD and not supported by Meego.
 * GNU has an equivalent g_strlcpy implementation into glib.
 * Featurized with compile time USE_GLIB flag for Meego builds.
 */
#ifdef USE_GLIB
#define strlcpy g_strlcpy
#define strlcat g_strlcat
#endif

#ifdef FEATURE_LE_DIAG
#define LE_UID_DIAG 53
#define LE_GID_DIAG 53
#define LE_GID_SDCARD 1015
#endif

#define DIAG_MDLOG_WAKELOCK_NAME		"diag_mdlog_wakelock"
#define DIAG_MDLOG_PROCESS_NAME			"diag_mdlog"

/* static data */
static int dir_set = 0;
static int kill_mdlog = 0;
static int dir_access = 0;
static char mask_file_list[FILE_NAME_LEN] = "/sdcard/diag_logs/Diag_list.txt";
static char default_mask_file[NUM_PROC][36] = {
	"/sdcard/diag_logs/Diag.cfg",
	"/sdcard/diag_logs/mdm/Diag.cfg",
	"/sdcard/diag_logs/mdm2/Diag.cfg",
	"/sdcard/diag_logs/mdm3/Diag.cfg",
	"/sdcard/diag_logs/mdm4/Diag.cfg",
	"/sdcard/diag_logs/qsc/Diag.cfg",
	"/sdcard/diag_logs/Diag.cfg",
	"/sdcard/diag_logs/Diag.cfg",
	"/sdcard/diag_logs/Diag.cfg",
	"/sdcard/diag_logs/Diag.cfg" };
static int mask_file_entered = 0;
static int mask_file_mdm_entered = 0;
static int mask_file_list_entered = 0;
static int enable_wakelock = 0;
static int enable_nonrealtime = 0;

extern unsigned long max_file_size;
extern unsigned long min_file_size;
extern int cleanup_mask;
extern int proc_type;
extern unsigned int max_file_num;
extern int rename_file_names;
extern int rename_dir_name;


/*=============================================================================*/
/* Function declarations */
/*=============================================================================*/

extern void flush_buffer(int);

/*=============================================================================*/

static void usage(char *progname)
{
	printf("\n Usage for %s:\n", progname);
	printf("\n-f, --filemsm:\t mask file name for MSM\n");
	printf("\n-m, --filemdm:\t mask file name for MDM\n");
	printf("\n-l, --filelist:\t name of file containing list of mask files\n");
	printf("\n-o, --output:\t output directory name\n");
	printf("\n-s, --size:\t maximum file size in MB\n");
	printf("\n-w, --wait:\t waiting for directory\n");
	printf("\n-n, --number:\t maximum file number\n");
	printf("\n-k, --kill:\t kill existing instance of diag_mdlog\n");
	printf("\n-c  --cleanmask:\t Send mask cleanup to modem at exit\n");
	printf("\n-d  --disablecon:\t Disable console messages\n");
	printf("\n-e  --enablelock:\t Run using wake lock to keep APPS processor on\n");
	printf("\n-b  --nonrealtime:\t Have peripherals buffer data and send data in non-real-time\n");
	printf("\n-r  --renamefiles:\t Rename dir/file names to time when closed\n");
	printf("\n-q  --qshrink4dir:\t Directory containing the APSS qshrink4 database files\n");
	printf("\n-h, --help:\t usage help\n");
	printf("\ne.g. diag_mdlog -f <mask_file name> -o <output dir>"
		       " -s <size in bytes> -c\n");
	exit(0);
}

static void parse_args(int argc, char **argv)
{
	int command, z;
	int file_num = 0;
	struct option longopts[] =
	{
		{ "filemsm",	1,	NULL,	'f'},
		{ "filemdm",	1,	NULL,	'm'},
		{ "filelist",	1,	NULL,	'l'},
		{ "output",	1,	NULL,	'o'},
		{ "size",	1,	NULL,	's'},
		{ "wait",       1,      NULL,   'w'},
		{ "number",     1,      NULL,   'n'},
		{ "kill",	0,	NULL,	'k'},
		{ "cleanmask",	0,	NULL,	'c'},
		{ "disablecon",	0,	NULL,	'd'},
		{ "enablelock",	0,	NULL,	'e'},
		{ "nonrealtime",0,	NULL,	'b'},
		{ "renamefiles",0,	NULL,	'r'},
		{ "qshrink4dir",0,	NULL,	'q'},
		{ "help",	0,	NULL,	'h'},
	};

	while ((command = getopt_long(argc, argv, "f:m:l:o:s:w:n:q:cdkebrh", longopts, NULL))
			!= -1) {
		switch (command) {
			case 'f':
				strlcpy(mask_file, optarg, FILE_NAME_LEN);
				mask_file_entered = 1;
				break;
			case 'm':
				strlcpy(mask_file_mdm, optarg, FILE_NAME_LEN);
				mask_file_mdm_entered = 1;
				break;
			case 'l':
				strlcpy(mask_file_list, optarg, FILE_NAME_LEN);
				mask_file_list_entered = 1;
				break;
			case 'o':
				for (z = 0; z < NUM_PROC; z++)
					strlcpy(output_dir[z], optarg,
							FILE_NAME_LEN - 6);
				dir_set = 1;
				break;
			case 's':
				max_file_size = atol(optarg);
				if ((long)max_file_size <= 0)
					max_file_size = 100000000;
				else
					max_file_size *= 1024 * 1024;
				min_file_size = ((max_file_size / 100) * 80);
				break;
			case 'w':
				strlcpy(dir_name, optarg, FILE_NAME_LEN);
				dir_access = 1;
				break;
			case 'n':
				file_num = atoi(optarg);
				if(file_num <= 1) {
					DIAG_LOGE("\n diag_mdlog: Invalid file number, must be greater than 1\n");
					exit(0);
				}
				max_file_num = file_num;
				break;
			case 'k':
				kill_mdlog = 1;
				break;
			case 'c':
				cleanup_mask = 1;
				break;
			case 'd':
				diag_disable_console = 1;
				break;
			case 'e':
				enable_wakelock = 1;
				break;
			case 'b':
				enable_nonrealtime = 1;
				break;
			case 'r':
				rename_file_names = 1;
				rename_dir_name = 1;
				break;
			case 'q':
				set_qshrink4_dir(optarg);
				break;
			case 'h':
			default:
				usage(argv[0]);
		};
	}
}

/* stop_mdlog is called when another instance of diag_mdlog is to be killed off */
static void stop_mdlog(char *pid_file)
{
	int fd;
	int ret;
	pid_t pid;
	char pid_buff[10];

	/* Determine the process id of the instance of diag_mdlog */
	fd = open(pid_file, O_RDONLY);
	if (fd < 0) {
		DIAG_LOGE("\n diag_mdlog: Unable to open pid file, errno: %d\n", errno);
		return;
	}

	ret = read(fd, pid_buff, 10);
	if (ret < 0) {
		DIAG_LOGE("\n diag_mdlog: Unable to read pid file, errno: %d\n", errno);
		close(fd);
		return;
	}

	close(fd);

	/* Make sure the buffer is properly terminated */
	if (ret == sizeof(pid_buff))
		ret--;
	pid_buff[ret] = '\0';

	pid = atoi(pid_buff);

	if (pid == 0 || (kill(pid, SIGTERM)) < 0) {
		DIAG_LOGE("\ndiag_mdlog: Unable to kill diag_mdlog instance pid: %d, errno: %d\n", pid, errno);
	} else {
		DIAG_LOGE("\ndiag_mdlog: stopping diag_mdlog instance pid: %d\n", pid);
	}

	return;
}

#ifndef FEATURE_LE_DIAG
static void adjust_permissions()
{
	int status;
	int size;

	gid_t diag_groups[] = {AID_QCOM_DIAG, AID_SDCARD_R, AID_MEDIA_RW, AID_SDCARD_RW};
	size = sizeof(diag_groups)/sizeof(gid_t);

	uid_t e_uid;

	/* Determine the effective user id */
	e_uid = geteuid();

	/*
	 * If this app is running as root, we need to drop the permissions.
	 * We can only drop permissions if this app is running as root. If
	 * the app is not running as root, then the app will need to be
	 * running with the appropriate permissions to support logging to
	 * the SD card.
	 */
	if (e_uid != 0)
		return;

	/* SD card access needs sdcard_rw group membership  */
	status = setgid(AID_SHELL);
	if (status < 0) {
		DIAG_LOGE("diag_mdlog: Error setting group id, errno: %d, Exiting ...\n", errno);
		goto fail_permissions;
	}

	/* Add qcom_diag and sdcard_r as supplemental groups so we can access /dev/diag */
	/* and /storage. Add AID_MEDIA_RW so we can write to the external SD card */
	status = setgroups(size, diag_groups);
	if (status == -1) {
		DIAG_LOGE("diag_mdlog: setgroups error, errno: %d, Exiting ...\n", errno);
		goto fail_permissions;
	}

	/* Ideally we would like a dedicated UID for diag_mdlog, for now use sdcard_rw */
	/* Drop privileges to sdcard_rw since system user does not have access */
	status = setuid(AID_SHELL);
	if (status < 0) {
		DIAG_LOGE("diag_mdlog: Error setting user id, errno: %d, Exiting ...\n", errno);
		goto fail_permissions;
	}
	return;

fail_permissions:
	if (diag_is_wakelock_init()) {
		diag_wakelock_release();
		diag_wakelock_destroy();
	}
	exit(0);
}
#else
static void adjust_permissions()
{
	int status;
	gid_t supplemental_gid = LE_GID_SDCARD;
	uid_t uid;

	/* Determine the real user id */
	uid = getuid();

	/*
	 * If this app is running as root, we need to drop the permissions
	 * We can only drop permissions if this app is running as root,
	 * since this app is not a setuid app on LE.
	 */
	if (uid == 0) {
		/* Set the real group ID to the diag group */
		status = setgid(LE_GID_DIAG);
		if (status < 0) {
			printf("diag_mdlog: Error setting group id, errno: %d, Exiting ...\n", errno);
			goto fail_permissions;
		}

		/* Set 1 supplemental group, the sdcard group so it can access the SD card */
		status = setgroups(1, &supplemental_gid);
		if (status == -1) {
			printf("diag_mdlog: setgroups error, errno: %d, Exiting ...\n", errno);
			goto fail_permissions;
		}

		/* Set the real user id to the diag user */
		status = setuid(LE_UID_DIAG);
		if (status < 0) {
			printf("diag_mdlog: Error setting user id, errno: %d, Exiting ...\n", errno);
			goto fail_permissions;
		}
	}
	return;

fail_permissions:
	if (diag_is_wakelock_init()) {
		diag_wakelock_release();
		diag_wakelock_destroy();
	}
	exit(0);
}
#endif

int read_mask_file_default(int proc_index)
{
	int status = 0;
	if (proc_index < 0 || proc_index >=NUM_PROC) {
		status = -ENODEV;
		return status;
	}

	proc_type = proc_index;
	DIAG_LOGE("diag_mdlog: Default mask file being read for proc_type: %d\n",
									proc_type);
	if (proc_type == 0)
		strlcpy(mask_file, default_mask_file[proc_type], FILE_NAME_LEN);
	else
		strlcpy(mask_file_mdm, default_mask_file[proc_type], FILE_NAME_LEN);

	status = diag_read_mask_file();

	return status;
}

/*
 * This function checks if another instance of diag_mdlog already exists in the
 * system. This is done by checking the pid file. If there are any errors in
 * opening the pid file, the new instance of diag_mdlog exits.
 *
 * It retuns the errno on error, 0 if everything is successful;
 */
static int diag_mdlog_pid_init(char *pid_file)
{
	int fd;
	int pid;
	int ret;
	int proc_fd;
	const int pid_len = 10;
	const int proc_len = 30;
	char pid_buff[pid_len];
	char process_name[proc_len];
	struct stat pid_stat;

	/* Determine the process id of the instance of diag_mdlog */
	fd = open(pid_file, O_RDONLY);
	if (fd < 0) {
		if (errno == ENOENT) {
			/* The pid file doesn't exist. Create a new file. */
			goto create;
		}
		DIAG_LOGE("diag_mdlog: Unable to open pid file, err: %d\n", errno);
		return errno;
	}

	ret = read(fd, pid_buff, pid_len);
	if (ret < 0) {
		DIAG_LOGE("diag_mdlog: Unable to read pid file, err: %d\n", errno);
		close(fd);
		return errno;
	}
	close(fd);

	/* Make sure the buffer is properly terminated */
	if (ret == pid_len)
		ret--;
	pid_buff[ret] = '\0';

	pid = atoi(pid_buff);

	snprintf(process_name, sizeof(process_name), "/proc/%d/cmdline", pid);
	proc_fd = open(process_name, O_RDONLY);
	if (proc_fd < 0) {
		/*
		 * The process is no longer active in the system. This is
		 * actually a no error case and diag_mdlog should continue.
		 * Replace the contents of the pid file with the new pid.
		 */
		goto create;
	}

	ret = read(proc_fd, process_name, proc_len);
	if (ret < 0) {
		DIAG_LOGE("diag_mdlog: Unable to read process file, err: %d\n", errno);
		close(proc_fd);
		return errno;
	}
	close(proc_fd);

	/* Make sure the buffer is properly terminated */
	if (ret == proc_len)
		ret--;
	process_name[ret] = '\0';

	/*
	 * Check if the process is actually a mdlog process. If not, this is not
	 * the same process that we started and is not an error. Go ahead and
	 * store the new pid in the file.
	 */
	if (!strstr(process_name, DIAG_MDLOG_PROCESS_NAME))
		goto create;

	if (pid > 0) {
		DIAG_LOGE("diag_mdlog: another instance of diag_mdlog already exitsts, pid: %d\n", pid);
		return pid;
	}

create:
	/*
	 * Make sure the default directory exists so the diag_pid file
	 * can be created.
	 */
	if (mkdir(DIAG_MDLOG_DIR, 0770)) {
		if (errno != EEXIST) {
			DIAG_LOGE("diag_mdlog: Failed to create directory for diag pid file, err: %d\n", errno);
			return errno;
		}
	}

	/* Check if the PID file is present. Delete the file if present */
	if (stat(pid_file, &pid_stat) == 0)
		unlink(pid_file);

	pid = getpid();
	fd = open(pid_file, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0770);
	if (fd < 0) {
		DIAG_LOGE("diag_mdlog: Unable to create pid file, err: %d\n", errno);
		return errno;
	}

	snprintf(pid_buff, pid_len, "%d", pid);
	write(fd, pid_buff, pid_len);
	close(fd);
	DIAG_LOGE("diag_mdlog: successfully created pid file, pid: %d\n", pid);
	return 0;
}

/* Main Function. This initializes Diag_LSM, sets up On Device Logging, then exits. */
int main(int argc, char *argv[])
{
	boolean bInit_Success = FALSE;
	int choice, input_scan = 1, i, result, pid, z;
	char buffer[30];
	struct timeval tv;
	time_t curtime;
	struct tm *tm_ptr = NULL;
	struct itimerval value, ovalue, pvalue;
	struct sigaction sact;
	uint16 remote_mask = 0;
	struct stat dir_stat;
	int stat_count = 0;
	int status;
	int num_mask_reads_succeeded = 0;

	parse_args(argc, argv);
	/* If another instance of diag_mdlog is to be killed off */
	if (kill_mdlog) {
		stop_mdlog(DIAG_MDLOG_PID_FILE);
		exit(0);
	}

	/* Acquire wakelock if the client is requesting for wakelock*/
	if (enable_wakelock) {
		diag_wakelock_init(DIAG_MDLOG_WAKELOCK_NAME);
		diag_wakelock_acquire();
	}

	adjust_permissions();

	if (diag_mdlog_pid_init(DIAG_MDLOG_PID_FILE))
		goto exit;

	/* Waiting for directory access */
	if (dir_access) {
		errno = 0;
		stat(dir_name, &dir_stat);
		while(errno != 0) {
			DIAG_LOGE("diag_mdlog: Directory %s is not accessible with errno %d\n",
				dir_name, errno);
			sleep(5);
			stat(dir_name, &dir_stat);
			stat_count++;
			if(stat_count == 24) {
				DIAG_LOGE("diag_mdlog: Directory %s is not accessible for %d seconds so exiting...\n",
				dir_name, stat_count*5);
				dir_access = 0;
				goto fail;
			}
		}
		if(errno == 0) {
			DIAG_LOGE("diag_mdlog: Directory %s is accessible\n",
					dir_name);
		}
		dir_access = 0;
	}

	/* Setup the directory that we will be logging to */
	if (dir_set) {
		if (rename_dir_name) {
			/* Two somewhat conflicting command line parameters
			 * have been entered.
			 * 1. The name of the directory where the logging
			 *    files should be placed.
			 * 2. The renaming of the logging file when closed
			 *    and renaming of the logging directory when
			 *    on-device logging is halted.
			 * In this case, do not rename the logging directory.
			 * But still allow for renaming of logging files on close.
			 */
			rename_dir_name = 0;
		}
		status = mkdir(output_dir[MSM], 0770);
		if (status == -1) {
			if (errno == EEXIST) {
				DIAG_LOGE("diag_mdlog: Warning output directory already exists: %s\n",
					output_dir[MSM]);
				DIAG_LOGE("diag_mdlog: Proceeding...\n");
			} else {
				DIAG_LOGE("diag_mdlog: Error creating: %s, errno: %d\n",
					output_dir[MSM], errno);
				DIAG_LOGE("diag_mdlog: Attempting to use default directory\n");
				dir_set = 0;
			}
		} else {
			DIAG_LOGE("diag_mdlog: Created logging directory %s\n",
						output_dir[MSM]);
		}
	}

	if (!dir_set) {
		gettimeofday(&tv, NULL);
		curtime=tv.tv_sec;
		tm_ptr = localtime(&curtime);
		if (tm_ptr)
			strftime(buffer, 30, "%Y%m%d_%H%M%S", tm_ptr);
		else
			strlcpy(buffer, "00000000_000000", 30);
		mkdir(DIAG_MDLOG_DIR, 0770);
		for (z = 0; z < NUM_PROC; z++) {
			strlcpy(output_dir[z], DIAG_MDLOG_DIR, FILE_NAME_LEN);
			(void)strlcat(output_dir[z], buffer, FILE_NAME_LEN);
		}

		DIAG_LOGE("\ndiag_mdlog: Continuing with default directory path %s\n",
								output_dir[MSM]);
		if (mkdir(output_dir[MSM], 0770)) {
			DIAG_LOGE("diag_mdlog: Unable to create directory, errno: %d Exiting....\n", errno);
			goto fail;
		}
	}


	/*
	 * Since On Device Logging optimizations have implemented
	 * a buffering scheme, set things up so that on receipt of
	 * specified signals the flush_buffer signal handler function
	 * will be call so that the data in the buffers can be flushed
	 * to the SD card before the app exits.
	 */
	sigemptyset( &sact.sa_mask );
	sact.sa_flags = 0;
	sact.sa_handler = flush_buffer;
	sigaction(SIGTERM, &sact, NULL);
	sigaction(SIGHUP, &sact, NULL);
	sigaction(SIGUSR1, &sact, NULL);
	sigaction(SIGINT, &sact, NULL);

	/* Initialize the Diag LSM userspace library */
	bInit_Success = Diag_LSM_Init(NULL);
	if (!bInit_Success) {
		DIAG_LOGE("\ndiag_mdlog: Diag_LSM_Init() failed. Exiting...\n");
		goto fail;
	}

	DIAG_LOGE("\ndiag_mdlog: Diag_LSM_Init succeeded.\n");

	/* Get the mask for remote processor */
	diag_has_remote_device(&remote_mask);
	DIAG_LOGE("\n REMOTE PROCESSOR MASK %x \n", remote_mask);

	if (enable_nonrealtime) {
		status = diag_vote_md_real_time(MODE_NONREALTIME);
		if (status == -1) {
			DIAG_LOGE("diag_mdlog: unable to set mode to non real time mode\n");
		}
	}
	/* Switch logging modes to turn on On Device Logging  */
	diag_switch_logging(MEMORY_DEVICE_MODE, output_dir[MSM]);

	/* Read mask file to tell On Device Logging what you are interested in */
	if (mask_file_list_entered) {
		status = diag_read_mask_file_list(mask_file_list);
		if (!status) {
			DIAG_LOGE("diag_mdlog: Error reading mask file list. Exiting ...\n");
		}
	} else {
		DIAG_LOGE("\ndiag_mdlog: Reading mask for MSM, proc_type: %d\n", proc_type);
		proc_type = MSM;
		if (mask_file_entered)
			status = diag_read_mask_file();
		else
			status = read_mask_file_default(proc_type);

		if (status) {
			DIAG_LOGE("diag_mdlog: Error reading mask file, proc_type: %d, file: %s\n",
					proc_type, mask_file);
		} else {
			num_mask_reads_succeeded++;
		}

		z = 1;
		while(remote_mask) {
			if(remote_mask & 1 ) {
				proc_type = z;
				DIAG_LOGE("\ndiag_mdlog: Reading mask for proc_type: %d\n",
										proc_type);
				if (proc_type == 1 && mask_file_mdm_entered)
					status = diag_read_mask_file();
				else
					status = read_mask_file_default(proc_type);

				if (status) {
					DIAG_LOGE("diag_mdlog: Error reading mask file, proc_type: %d, file: %s\n",
						proc_type, mask_file_mdm);
				} else {
					num_mask_reads_succeeded++;
				}
			}
			z++;
			remote_mask = remote_mask >> 1;
		}

		/*
		 * If no mask files have been successfully read,
		 * try reading from a mask list file
		 */
		if (num_mask_reads_succeeded == 0) {
			DIAG_LOGE("\ndiag_mdlog: No successful mask file reads. Trying default mask list file.\n");
			status = diag_read_mask_file_list(mask_file_list);
			if (!status) {
				DIAG_LOGE("diag_mdlog: No mask files have been successfully read.\n");
				DIAG_LOGE("diag_mdlog: Running with masks that were set prior to diag_mdlog start-up.\n");
			}
		}
		status = 1;
	}

	if (status) {
		/* Reset proc type */
		proc_type = MSM;
		while(1) {
			/* Allow the main thread to sleep while logging is going on. */
			sleep(3600);
		}
	}

	/* De-Initialize the Diag LSM userspace library */
	Diag_LSM_DeInit();

fail:
	unlink(DIAG_MDLOG_PID_FILE);
exit:
	/* Release and destroy wakelock if enabled */
	if (enable_wakelock) {
		diag_wakelock_release();
		diag_wakelock_destroy();
	}
	return 0;
}
