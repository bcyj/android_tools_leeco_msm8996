/*******************************************************************************
 * -----------------------------------------------------------------------------
 * Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * -----------------------------------------------------------------------------
 ******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/rtc.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "header.h"
#include <dirent.h>


int suspend_test(void)
{
	char wakelock_dmask[16] = "";
	char earlysuspend_dmask[16] = "";
	time_t now;
	time_t expiration;
	int success_count = 0;
	int rv;
	int i = 0;
	int k;
	int num_success = 0;
	int ret;
	unsigned int core1_status;
	unsigned int core2_status;
	unsigned int core3_status;
	unsigned int dual_core = 0;
	signed long usb_active;
	int num_cores = 0;
	char *ss_pid;


	fprintf(stdout, "Determine if wakelock node is there \n");
	g_wakelock_exists = file_exists(WAKELOCK_NODE);

	if (g_wakelock_exists) {

		fprintf(stdout, "reset the sensors data setting \n");
		rv = write_string_to_file(NULL, SENSOR_SETTINGS, "0\n");
		if (rv < 0) {
			fprintf(stdout, "unable to set sensor settings: %s\n", strerror(-rv));
			goto suspend_test_early_bailout;
		}

		fprintf(stdout, "sleep for sensor daemon to pick up the setting \n");
		sleep(2);

		fprintf(stdout, "Save current wakelock debug mask\n");
		rv = read_from_file(
			NULL,
			WAKELOCK_DEBUG_MASK_NODE,
			wakelock_dmask,
			sizeof(wakelock_dmask) - 1);

		if (rv < 0) {
			fprintf(stdout,
				"cannot read %s: %s\n",
				WAKELOCK_DEBUG_MASK_NODE,
				strerror(-rv));
			goto suspend_test_bailout;
		}

		wakelock_dmask[rv] = '\0';

		fprintf(stdout,
			"Turn on additional wakelock debug info\n");
		rv = write_string_to_file(
			NULL, WAKELOCK_DEBUG_MASK_NODE, "22\n");
		if (rv < 0) {
			fprintf(stdout,
				"cannot write to %s: %s\n",
				WAKELOCK_DEBUG_MASK_NODE,
				strerror(-rv));
			goto suspend_test_bailout;
		}

		fprintf(stdout, "Add wakelock to hold off suspend till we suspend ourselves\n");
		rv = write_string_to_file(
			NULL, WAKELOCK_LOCK_NODE, SUSPENDPC_WAKELOCK);

		if (rv < 0) {
			fprintf(stdout, "cannot write to %s: %s\n",
				WAKELOCK_LOCK_NODE, strerror(-rv));
			goto suspend_test_bailout;
		}
	}

	fprintf(stdout, "Save current earlysuspend debug mask\n");
	rv = read_from_file(
		NULL,
		EARLYSUSPEND_DEBUG_MASK_NODE,
		earlysuspend_dmask,
		sizeof(earlysuspend_dmask) - 1);

	if (rv < 0) {
		fprintf(stdout,
			"cannot read %s: %s\n",
			EARLYSUSPEND_DEBUG_MASK_NODE,
			strerror(-rv));
		goto suspend_test_bailout;
	}

	earlysuspend_dmask[rv] = '\0';

	fprintf(stdout, "turn on early suspend logs\n");
	rv = write_string_to_file(
		NULL, EARLYSUSPEND_DEBUG_MASK_NODE, "5\n");

	if (rv < 0) {
		fprintf(stdout,
			"cannot write to %s: %s\n",
			EARLYSUSPEND_DEBUG_MASK_NODE,
			strerror(-rv));
		goto suspend_test_bailout;
	}


	fprintf(stdout, "Check whether power management is up\n");
	if (!file_exists(PM_STATS_NODE)) {
		fprintf(stdout, "power management is not available\n");
		goto suspend_test_bailout;
	}

	fprintf(stdout, "Determine the power management module\n");

	if (directory_exists(SYS_PM_8x60)) {
		g_sys_pm = SYS_PM_8x60;
		if (file_exists_with_prefix(SYS_PM_8x60, SLEEP_MODE_NODE_CORE_3))
			num_cores = 4;
		else if (file_exists_with_prefix(SYS_PM_8x60, SLEEP_MODE_NODE_CORE_1))
			num_cores = 2;
		else
			num_cores = 1;
	}
	else {
		fprintf(stdout, "power management is not available\n");
		goto suspend_test_bailout;
	}

	fprintf(stdout, "Delay tests for some time for usb cable to be plugged out\n");
	if (g_delay_test_sec) {
		unsigned int seconds = g_delay_test_sec;
		while (seconds)
			seconds = sleep(seconds);
	}

	/*
	 *  check if the usb wakelock has been released before
	 *  proceeding with the test
	 */
	if (g_wakelock_exists) {
		/*
		 *  capture the usb wakelock stats
		 */
		rv = read_from_file(NULL, WAKELOCK_NODE, g_wakelock_stats,
				    sizeof(g_wakelock_stats) - 1);

		usb_active = parse_wakelock_stats_for_active_wl(
			g_wakelock_stats, "\"msm_otg\"");

		if (usb_active != 0) {
			fprintf(stdout, " the usb wakelock is still held by the system\n");
			goto suspend_test_bailout;
		}
	}

	/*
	 *  determine resume command
	 */
	do {
		rv = write_string_to_file(NULL, POWER_NODE, POWER_STANDBY);
		if (rv > 0) {
			g_resume_command = POWER_STANDBY;
			break;
		}

		rv = write_string_to_file(
			NULL, POWER_NODE, POWER_ON);

		if (rv > 0) {
			g_resume_command = POWER_ON;
			break;
		}

		fprintf(stdout, "cannot write to %s: %s\n",
			POWER_NODE, strerror(-rv));

		goto suspend_test_bailout;
	} while (0);

	fprintf(stdout, "Set suspend configuration\n");
	if (num_cores == 4) {
		write_int_to_file(g_sys_pm, SLEEP_MODE_NODE_CORE_0, 1);
		write_int_to_file(g_sys_pm, SLEEP_MODE_NODE_CORE_1, 1);
		write_int_to_file(g_sys_pm, SLEEP_MODE_NODE_CORE_2, 1);
		write_int_to_file(g_sys_pm, SLEEP_MODE_NODE_CORE_3, 1);
	}
	else if (num_cores == 2) {
		write_int_to_file(g_sys_pm, SLEEP_MODE_NODE_CORE_0, 1);
		write_int_to_file(g_sys_pm, SLEEP_MODE_NODE_CORE_1, 1);
	}
	else if (num_cores == 1) {
		write_int_to_file(g_sys_pm, SLEEP_MODE_NODE_CORE_0, 1);
	}

	fprintf(stdout, "Turn off idle power collapse and mp-decision\n");
	if ((num_cores == 2) || (num_cores == 4)) {
		if (num_cores == 4) {
			write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_2, 0);
			write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_3, 0);
			write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_2, 0);
			write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_3, 0);
		}

		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_0, 0);
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_1, 0);
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_0, 0);
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_1, 0);

		fprintf(stdout, "Turn off mp-decision before testing suspend\n");
		rv = fork_exec("stop", "mpdecision", NULL, NULL, 0);
		if (rv < 0) {
			fprintf(stdout, "cannot turn off mp-decision: %s\n", strerror(-rv));
			goto suspend_test_bailout;
		}

		fprintf(stdout, "Now explicitly turn ON core 1 if not ON already\n");
		core1_status = read_unsigned_int_from_file(NULL, HOTPLUG_NODE_CORE_1);
		fprintf(stdout, "core1_status = %d\n", core1_status);
		if (!core1_status) {
			fprintf(stdout, "turning on core 1\n");
			rv = write_int_to_file(NULL, HOTPLUG_NODE_CORE_1, 1);
		}

		if (num_cores == 4) {
			fprintf(stdout, "Now explicitly turn ON core 2 if not ON already\n");
			core2_status = read_unsigned_int_from_file(NULL, HOTPLUG_NODE_CORE_2);
			fprintf(stdout, "core2_status = %d\n", core2_status);
			if (!core2_status) {
				fprintf(stdout, "turning on core 2\n");
				rv = write_int_to_file(NULL, HOTPLUG_NODE_CORE_2, 1);
			}

			fprintf(stdout, "Now explicitly turn ON core 3 if not ON already\n");
			core3_status = read_unsigned_int_from_file(NULL, HOTPLUG_NODE_CORE_3);
			fprintf(stdout, "core3_status = %d\n", core3_status);
			if (!core3_status) {
				fprintf(stdout, "turning on core 3\n");
				rv = write_int_to_file(NULL, HOTPLUG_NODE_CORE_3, 1);
			}
		}
	}
	else if (num_cores == 1) {
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_0, 0);
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_0, 0);
	}

	fprintf(stdout, "Clear kernel log\n");
	rv = fork_exec("dmesg", "-c", NULL, NULL, 1);
	if (rv < 0) {
		fprintf(stdout,
			"dmesg -c failed: %s\n", strerror(-rv));
		goto suspend_test_bailout;
	}

	fprintf(stdout, "Clear power management stats\n");
	rv = write_string_to_file(NULL, PM_STATS_NODE, "reset\n");
	if (rv < 0) {
		fprintf(stdout,	"cannot write to %s: %s\n",
			PM_STATS_NODE, strerror(-rv));
		goto suspend_test_bailout;
	}

	fprintf(stdout, "Set wakeup time\n");
	rv = write_int_to_file(
		g_sys_pm, SLEEP_TIME_OVERRIDE_NODE, g_wakeup_sec);
	if (rv < 0) {
		fprintf(stdout,	"cannot write to %s%s: %s\n", g_sys_pm,
			SLEEP_TIME_OVERRIDE_NODE, strerror(-rv));
		goto suspend_test_bailout;
	}

	if (g_wakelock_exists) {
		fprintf(stdout, "Remove the suspend wakelock before issuing the suspend command\n");
		rv = write_string_to_file(
			NULL, WAKELOCK_UNLOCK_NODE, SUSPENDPC_WAKELOCK);

		if (rv < 0) {
			fprintf(stdout, "cannot write to %s: %s\n",
				WAKELOCK_UNLOCK_NODE, strerror(-rv));
			goto suspend_test_bailout;
		}
	}

	for (i = 0; i < g_num_iter; i++) {

		msg("beginning iteration %d for suspend\n", i);
		fprintf(stdout, "Determine expiration time\n");
		now = time(NULL);
		if (now == (time_t)-1) {
			fprintf(stdout,
				"time() failed: %s\n", strerror(errno));
			continue;
		}

		expiration = now  + g_timeout_sec;

		fprintf(stdout, "Issue suspend command\n");
		rv = write_string_to_file(NULL, POWER_NODE, "mem\n");
		if (rv < 0) {
			fprintf(stdout,	"cannot write to %s: %s\n",
				POWER_NODE, strerror(-rv));
			i++;
			goto suspend_test_late_bailout;
		}

		k = 0;
		do {
			usleep(300 * 1000);

			if (k % 10 == 0) {
				fprintf(stdout,
					"load power management stats\n");
			}

			rv = read_from_file(NULL, PM_STATS_NODE, g_pm_stats,
					    sizeof(g_pm_stats) - 1);

			if (rv < 0) {
				fprintf(stdout,	"cannot read %s: %s\n",
					PM_STATS_NODE, strerror(-rv));
				goto suspend_test_bailout_cur_iter;
			}

			if (rv == sizeof(g_pm_stats) - 1) {
				fprintf(stdout,	"buffer too small for %s\n",
					PM_STATS_NODE);
				i++;
				goto suspend_test_late_bailout;
			}

			g_pm_stats[rv] = '\0';

			if (k % 10 == 0) {
				fprintf(stdout, "Look for suspend event\n");
			}

			success_count = parse_pm_stats_count(g_pm_stats,
							     "\n[cpu 0] suspend:\n  count: ");

			if (success_count < 0 ) {
				fprintf(stdout,	"bad count(s) from "
					"power management stats\n");
				i++;
				goto suspend_test_late_bailout;
			}

			if (success_count > num_success)
				break;

			if (k % 10 == 0) {
				fprintf(stdout, "Check timeout\n");
			}

			now = time(NULL);
			if (now == (time_t)-1) {
				fprintf(stdout,	"time() failed: %s\n",
					strerror(errno));
				goto suspend_test_bailout_cur_iter;
			}

			k++;
		} while (now < expiration);

suspend_test_bailout_cur_iter:

		if (g_resume_command != NULL) {
			fprintf(stdout, "Issue resume command\n");
			rv = write_string_to_file(
				NULL, POWER_NODE, g_resume_command);

			if (rv < 0) {
				fprintf(stdout,	"cannot write to %s: %s\n",
					POWER_NODE, strerror(-rv));
				i++;
				goto suspend_test_late_bailout;
			}
		}

		if (success_count > num_success) {
			fprintf(stdout, "Suspend/resume succeeded on iteration %d\n", i + 1);
			msg("Suspend/resume succeeded on iteration %d\n", i + 1);
			num_success++;
		}

		if (now >= expiration) {
			fprintf(stdout, "Suspend/resume timed out on iteration %d\n", i + 1);
			msg("Suspend/resume timed out on iteration %\n", i + 1);
			i++;
			break;
		}
	}


	fprintf(stdout, "====BEGIN power management stats====\n");
	fflush(stdout);
	rv = write_to_fd(STDOUT_FILENO, g_pm_stats, strlen(g_pm_stats));
	if (rv < 0)
		fprintf(stdout,	"cannot write out power management stats\n");
	fprintf(stdout, "====END power management stats====\n");

	if (g_wakelock_exists) {
		fprintf(stdout, "====BEGIN wakelock stats====\n");
		fflush(stdout);
		rv = fork_exec("cat", WAKELOCK_NODE, NULL, NULL, 0);
		if (rv < 0)
			fprintf(stdout, "cannot dump %s\n", WAKELOCK_NODE);
		fprintf(stdout, "====END wakelock stats====\n");
	}


suspend_test_late_bailout:

	if (g_resume_command != NULL) {
		fprintf(stdout, "Issue resume command\n");
		rv = write_string_to_file(
			NULL, POWER_NODE, g_resume_command);

		if (rv < 0) {
			fprintf(stdout,	"cannot write to %s: %s\n",
				POWER_NODE, strerror(-rv));
			goto suspend_test_bailout;
		}
	}

suspend_test_bailout:

	fprintf(stdout, "Unset wakeup time\n");
	rv = write_string_to_file(
		g_sys_pm, SLEEP_TIME_OVERRIDE_NODE, "0\n");

	if ((num_cores == 2) || (num_cores == 4)) {
		/* restore idle power collapse */

		if (num_cores == 4) {

			write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_2, 1);
			write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_3, 1);
			write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_2, 1);
			write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_3, 1);
		}

		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_0, 1);
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_1, 1);
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_0, 1);
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_1, 1);

		fprintf(stdout, "Turn mp-decision back on\n");
		rv = fork_exec("start", "mpdecision", NULL, NULL, 0);
	}
	else if (num_cores == 1) {

		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_0, 1);
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_0, 1);
	}


	fprintf(stdout, "Release suspend wakelock\n");
	rv = write_string_to_file(
		NULL, WAKELOCK_UNLOCK_NODE, SUSPENDPC_WAKELOCK);

	fprintf(stdout, "Restore wakelock debug mask\n");
	if (g_wakelock_exists) {
		rv = write_string_to_file(NULL,
					  WAKELOCK_DEBUG_MASK_NODE, wakelock_dmask);
		if (rv < 0) {
			fprintf(stdout,
				"cannot write to %s: %s\n",
				WAKELOCK_DEBUG_MASK_NODE,
				strerror(-rv));
		}
	}

	fprintf(stdout, "Restore earlysuspend debug mask\n");
	rv = write_string_to_file(NULL,
				  EARLYSUSPEND_DEBUG_MASK_NODE, earlysuspend_dmask);
	if (rv < 0) {
		fprintf(stdout,
			"cannot write to %s: %s\n",
			EARLYSUSPEND_DEBUG_MASK_NODE,
			strerror(-rv));
	}

	fprintf(stdout, "Restore the sensor settings\n");

	fprintf(stdout, "set the sensors data setting\n");
	rv = write_string_to_file(NULL, SENSOR_SETTINGS, "1\n");

	fprintf(stdout, "sleep for sometime for the sensors daemon to pick up the setting\n");
	sleep(2);

suspend_test_early_bailout:


	fprintf(stdout, "====BEGIN kernel log====\n");
	fflush(stdout);
	rv = fork_exec("dmesg", NULL, NULL, NULL, 0);
	if (rv < 0)
		fprintf(stdout,	"cannot dump kernel log\n");
	fprintf(stdout, "====END kernel log====\n");

	fprintf(stdout, "====BEGIN userspace log====\n");
	fflush(stdout);
	rv = fork_exec("logcat", "-d", "-v", "time", 0);
	if (rv < 0)
		fprintf(stdout,	"cannot dump userspace log\n");
	fprintf(stdout, "====END userspace log====\n");

	fprintf(stdout, "\n Suspend/resume test succeeded %d out of %d times\n", num_success, i);


	if (num_success == g_num_iter)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;

}
