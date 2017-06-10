/*******************************************************************************
 * -----------------------------------------------------------------------------
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
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

int suspend_test(void)
{
	char wakelock_dmask[16] = "";
	time_t now;
	time_t expiration;
	int success_count = 0;
	int failure_count = 0;
	int rv;
	int i;
	int k;
	int num_success, num_failure;
	int ret;
	signed long usb_active;

	g_wakelock_exists = file_exists(WAKELOCK_NODE);

	if (g_wakelock_exists) {
		fprintf(stdout, "Add wakelock to hold off suspend till we suspend ourselves\n");
		rv = write_string_to_file(
			NULL, WAKELOCK_LOCK_NODE, SUSPENDPC_WAKELOCK);

		if (rv < 0) {
			fprintf(stdout, "cannot write to %s: %s\n",
			      WAKELOCK_LOCK_NODE, strerror(-rv));
			goto suspend_test_first_bail_out;
		}
	}

	fprintf(stdout, "Check whether power management is up\n");
	if (!file_exists(PM_STATS_NODE)) {
		fprintf(stdout, "power management is not available\n");
		goto suspend_test_first_bail_out;
	}

	fprintf(stdout, "Determine the power management module\n");

	if (directory_exists(SYS_PM))
		g_sys_pm = SYS_PM;
	else if (directory_exists(SYS_PM2))
		g_sys_pm = SYS_PM2;
	else {
		fprintf(stdout, "power management is not available\n");
		goto suspend_test_first_bail_out;
	}


	fprintf(stdout, "Delay tests for some time for usb cable to be plugged out\n");
	if (g_delay_test_sec) {
		unsigned int seconds = g_delay_test_sec;
		while (seconds)
			seconds = sleep(seconds);
	}


	fprintf(stdout, "Determine if wakelock node is there \n");
	g_wakelock_exists = file_exists(WAKELOCK_NODE);

	if (g_wakelock_exists) {

		/*
		 *  capture the wakelock stats
		 */
		rv = read_from_file(NULL, WAKELOCK_NODE, g_wakelock_stats,
				    sizeof(g_wakelock_stats) - 1);
		/*
		 *  now check if the usb wakelock has been released before
		 *  proceeding with the test
		 */

		usb_active = parse_wakelock_stats_for_active_wl(
			g_wakelock_stats, "\"msm_otg\"");

		if (usb_active != 0) {
			fprintf(stdout, " the usb wakelock is still held by the system\n");
			goto suspend_test_first_bail_out;
		}

		fprintf(stdout, "Determine resume command\n");
		do {
		  rv = write_string_to_file(
			  NULL, POWER_NODE, POWER_STANDBY);
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
		  goto suspend_test_first_bail_out;
	  } while (0);
	}

	fprintf(stdout, "Check suspend configuration\n");

	rv = read_unsigned_int_from_file(g_sys_pm, SLEEP_MODE_NODE);
	if (rv < 0) {
		fprintf(stdout,	"cannot read %s%s: %s\n",
			g_sys_pm, SLEEP_MODE_NODE, strerror(-rv));
		goto suspend_test_bail;
	}

	if (rv != 0 && rv != 1) {
		fprintf(stdout, "incorrect configuration for suspend:\n"
			"%s%s = %d\n", g_sys_pm, SLEEP_MODE_NODE, rv);
		goto suspend_test_bail;
	}

	fprintf(stdout, "Turn off idle power collapse\n");

	rv = write_string_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE, "4\n");
	if (rv < 0) {
		fprintf(stdout,	"cannot write to %s%s: %s\n",
			g_sys_pm, IDLE_SLEEP_MODE_NODE, strerror(-rv));
		goto suspend_test_bail;
	}

	fprintf(stdout, "Clear kernel log\n");
	rv = fork_exec("dmesg", "-c", NULL, NULL, 1);
	if (rv < 0) {
		fprintf(stdout,
			"dmesg -c failed: %s\n", strerror(-rv));
		goto suspend_test_bail;
	}

	fprintf(stdout, "Clear power management stats\n");
	rv = write_string_to_file(NULL, PM_STATS_NODE, "reset\n");
	if (rv < 0) {
		fprintf(stdout,	"cannot write to %s: %s\n",
			PM_STATS_NODE, strerror(-rv));
		goto suspend_test_bail;
	}

	if (g_wakelock_exists) {
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
			goto suspend_test_bail;
		}

		wakelock_dmask[rv] = '\0';

		fprintf(stdout,
			"Turn on additional wakelock debug info\n");
		rv = write_string_to_file(
			NULL, WAKELOCK_DEBUG_MASK_NODE, "31\n");
		if (rv < 0) {
			fprintf(stdout,
				"cannot write to %s: %s\n",
				WAKELOCK_DEBUG_MASK_NODE,
				strerror(-rv));
			goto suspend_test_bail;
		}
	}

	num_success = 0;
	num_failure = 0;

	for (i = 0; i < g_num_iter; i++) {

	        fprintf(stdout, "Set wakeup time\n");
		rv = write_int_to_file(
			g_sys_pm, SLEEP_TIME_OVERRIDE_NODE, g_wakeup_sec);
		if (rv < 0) {
			fprintf(stdout,	"cannot write to %s%s: %s\n", g_sys_pm,
				SLEEP_TIME_OVERRIDE_NODE, strerror(-rv));
			goto suspend_test_bail;
		}

		fprintf(stdout, "Determine expiration time\n");
		now = time(NULL);
		if (now == (time_t)-1) {
			fprintf(stdout,
				"time() failed: %s\n", strerror(errno));
			goto suspend_test_bail;
		}
		expiration = now + g_timeout_sec;

		if (g_wakelock_exists) {
			fprintf(stdout, "Remove the suspend wakeock before issuing the suspend command\n");
			rv = write_string_to_file(
				NULL, WAKELOCK_UNLOCK_NODE, SUSPENDPC_WAKELOCK);

			if (rv < 0) {
				fprintf(stdout, "cannot write to %s: %s\n",
					WAKELOCK_UNLOCK_NODE, strerror(-rv));
				goto suspend_test_bail;
			}
		}

		fprintf(stdout, "Issue suspend command\n");
		rv = write_string_to_file(NULL, POWER_NODE, "mem\n");
		if (rv < 0) {
			fprintf(stdout,	"cannot write to %s: %s\n",
				POWER_NODE, strerror(-rv));
			goto suspend_test_bail;
		}

		k = 0;
		do {
			usleep(300 * 1000);

			if (k % 10 == 0) {
				fprintf(stdout,
					"Load power management stats\n");
			}

			rv = read_from_file(NULL, PM_STATS_NODE, g_pm_stats,
					    sizeof(g_pm_stats) - 1);
			if (rv < 0) {
				fprintf(stdout,	"cannot read %s: %s\n",
					PM_STATS_NODE, strerror(-rv));
				goto suspend_test_bail;
			}

			if (rv == sizeof(g_pm_stats) - 1) {
				fprintf(stdout,	"buffer too small for %s\n",
					PM_STATS_NODE);
				goto suspend_test_bail;
			}
			g_pm_stats[rv] = '\0';

			if (k % 10 == 0) {
				fprintf(stdout, "Look for suspend event\n");
			}

			success_count = parse_pm_stats_count(g_pm_stats,
							     "\nsuspend:\n  count: ");
			failure_count = parse_pm_stats_count(g_pm_stats,
							     "\nfailed-suspend:\n  count: ");

			if (success_count < 0 || failure_count < 0) {
				fprintf(stdout,	"bad count(s) from "
					"power management stats\n");
				goto suspend_test_bail;
			}

			if (success_count > num_success || failure_count > num_failure)
				break;

			if (k % 10 == 0) {
				fprintf(stdout, "Check timeout\n");
			}

			now = time(NULL);
			if (now == (time_t)-1) {
				fprintf(stdout,	"time() failed: %s\n",
					strerror(errno));
				goto suspend_test_bail;
			}

			k++;
		} while (now < expiration);

		if (g_resume_command != NULL) {
			fprintf(stdout, "Issue resume command\n");
			rv = write_string_to_file(
				NULL, POWER_NODE, g_resume_command);
			if (rv < 0) {
				fprintf(stdout,	"cannot write to %s: %s\n",
					POWER_NODE, strerror(-rv));
				goto suspend_test_bail;
			}
		}

		if (success_count > num_success) {
			fprintf(stdout, "Suspend/resume succeeded on iteration %d\n", i+1);
			num_success++;
		}

		if (failure_count > num_failure) {
			fprintf(stdout, "Suspend/resume failed on iteration %d\n", i+1);
			num_failure++;
		}

		if (now >= expiration) {
			fprintf(stdout, "Suspend/resume timed out on iteration %d\n", i+1);
			break;
		}
	}

	if (g_wakelock_exists) {
		fprintf(stdout, "Restore wakelock debug mask\n");
		rv = write_string_to_file(NULL,
					  WAKELOCK_DEBUG_MASK_NODE, wakelock_dmask);
		if (rv < 0) {
			fprintf(stdout,
				"cannot write to %s: %s\n",
				WAKELOCK_DEBUG_MASK_NODE,
				strerror(-rv));
			goto suspend_test_bail;
		}
	}

	fprintf(stdout, "====BEGIN power management stats====\n");
	fflush(stdout);
	rv = write_to_fd(STDOUT_FILENO, g_pm_stats, strlen(g_pm_stats));
	if (rv < 0)
		fprintf(stdout,	"cannot write out power management stats\n");
	fprintf(stdout, "====END power management stats====\n");

	if (g_wakelock_exists) {
		fprintf(stdout, "====BEGIN %s====\n", WAKELOCK_NODE);
		fflush(stdout);
		rv = fork_exec("cat", WAKELOCK_NODE, NULL, NULL, 0);
		if (rv < 0)
			fprintf(stdout, "cannot dump %s\n", WAKELOCK_NODE);
		fprintf(stdout, "====END %s====\n", WAKELOCK_NODE);
	}

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

	fprintf(stdout, "\n Suspend/resume test succeeded %d out of %d times\n", num_success, i+1);

	if (num_success > 0) {
		int line_no_start;
		int line_no_end;

		fprintf(stdout, "Checking clocks for TCXO shutdown\n");

		line_no_start = get_pm_stats_line_no(
			g_pm_stats,
			"Clocks against last TCXO shutdown:");
		line_no_end = get_pm_stats_line_no(
			g_pm_stats,
			"Last power collapse voted");

		if (line_no_start < 0 || line_no_end < 0) {
			fprintf(stdout, "cannot find clocks info from %s\n",
				PM_STATS_NODE);
			goto suspend_test_bail;
		}

		if (line_no_start + 1 != line_no_end) {
			fprintf(stdout, "Some clock(s) prevented TCXO shutdown\n");
			if (!g_ignore_tcxo)
				goto suspend_test_bail;
		}
	}

	if (num_success == g_num_iter)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;

suspend_test_bail:
	if (g_resume_command != NULL)
		write_string_to_file(NULL, POWER_NODE, g_resume_command);

	if (strlen(wakelock_dmask) > 0)
		write_string_to_file(NULL,
				     WAKELOCK_DEBUG_MASK_NODE, wakelock_dmask);

	fprintf(stdout, "====BEGIN kernel log====\n");
	fflush(stdout);
	rv = fork_exec("dmesg", NULL, NULL, NULL, 0);
	if (rv < 0)
		fprintf(stdout,	"cannot dump kernel log\n");
	fprintf(stdout, "====END kernel log====\n");

suspend_test_first_bail_out:

	return EXIT_FAILURE;
}
