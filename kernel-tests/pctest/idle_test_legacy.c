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

int idle_power_collapse_test(void)
{
	char irq_dmask[16] = "";
	time_t now;
	time_t expiration;
	int success_count;
	int success_cnt_cpu0, success_cnt_cpu0_std, success_cnt_cpu1;
	int rv;

	fprintf(stdout, "Check whether power management is up\n");
	if (!file_exists(PM_STATS_NODE)) {
		fprintf(stdout, "power management is not available\n");
		goto idle_power_collapse_test_bail;
	}

	if (directory_exists(SYS_PM))
		g_sys_pm = SYS_PM;
	else if (directory_exists(SYS_PM2))
		g_sys_pm = SYS_PM2;
	else {
		fprintf(stdout, "power management is not available\n");
		goto idle_power_collapse_test_bail;
	}

	fprintf(stdout, "Check idle power collapse configuration\n");
	rv = read_unsigned_int_from_file(g_sys_pm, IDLE_SLEEP_MODE_NODE);
	if (rv < 0) {
		fprintf(stdout,	"cannot read %s%s: %s\n",
				g_sys_pm, IDLE_SLEEP_MODE_NODE, strerror(-rv));
		goto idle_power_collapse_test_bail;
	}

	if (rv != 0 && rv != 1) {
		fprintf(stdout, "incorrect configuration for idle power "
			"collapse:\n%s%s = %d\n", g_sys_pm,
			IDLE_SLEEP_MODE_NODE, rv);
		goto idle_power_collapse_test_bail;
	}

	fprintf(stdout, "Determine wakelock\n");
	g_wakelock_exists = file_exists(WAKELOCK_NODE);

	if (g_wakelock_exists) {
		fprintf(stdout, "Add wakelock to hold off possible suspend\n");
		rv = write_string_to_file(
					  NULL, WAKELOCK_LOCK_NODE, IDLEPC_WAKELOCK);
		if (rv < 0) {
		  fprintf(stdout,	"cannot write to %s: %s\n",
			  WAKELOCK_LOCK_NODE, strerror(-rv));
		  goto idle_power_collapse_test_bail;
		}
	}

	fprintf(stdout, "Delay tests for some time\n");
	if (g_delay_test_sec) {
		unsigned int seconds = g_delay_test_sec;
		while (seconds)
			seconds = sleep(seconds);
	}

	fprintf(stdout, "Save current IRQ debug mask\n");

	rv = read_from_file(NULL, IRQ_DEBUG_MASK_NODE, irq_dmask,
			    sizeof(irq_dmask) - 1);
	if (rv < 0) {
		fprintf(stdout,	"cannot read %s: %s\n",	IRQ_DEBUG_MASK_NODE,
			strerror(-rv));
		goto idle_power_collapse_test_bail;
	}

	irq_dmask[rv] = '\0';

	fprintf(stdout, "Turn on additional IRQ debug info\n");
	rv = write_string_to_file(NULL, IRQ_DEBUG_MASK_NODE, "16\n");
	if (rv < 0) {
		fprintf(stdout,	"cannot write to %s: %s\n",
			IRQ_DEBUG_MASK_NODE, strerror(-rv));
		goto idle_power_collapse_test_bail;
	}

	fprintf(stdout, "Clear kernel log\n");
	rv = fork_exec("dmesg", "-c", NULL, NULL, 1);
	if (rv < 0) {
		fprintf(stdout,
			"dmesg -c failed: %s\n", strerror(-rv));
		goto idle_power_collapse_test_bail;
	}

	fprintf(stdout, "Clear power management stats\n");
	rv = write_string_to_file(NULL, PM_STATS_NODE, "reset\n");
	if (rv < 0) {
		fprintf(stdout,	"cannot write to %s: %s\n",
			PM_STATS_NODE, strerror(-rv));
		goto idle_power_collapse_test_bail;
	}

	fprintf(stdout, "Determine expiration time\n");
	now = time(NULL);
	if (now == (time_t)-1) {
		fprintf(stdout,
			"time() failed: %s\n", strerror(errno));
		goto idle_power_collapse_test_bail;
	}
	expiration = now + g_timeout_sec;

	fprintf(stdout, "Wait till expiration\n");
	while (expiration > now) {
		sleep(expiration - now);
		now = time(NULL);
		if (now == (time_t)-1) {
			fprintf(stdout,	"time() failed: %s\n",
				strerror(errno));
			goto idle_power_collapse_test_bail;
		}
	}

	fprintf(stdout, "Load power management stats\n");
	rv = read_from_file(NULL,
		PM_STATS_NODE, g_pm_stats, sizeof(g_pm_stats) - 1);
	if (rv < 0) {
		fprintf(stdout,
			"cannot read %s: %s\n",	PM_STATS_NODE, strerror(-rv));
		goto idle_power_collapse_test_bail;
	}
	if (rv == sizeof(g_pm_stats) - 1) {
		fprintf(stdout,	"buffer too small for %s\n", PM_STATS_NODE);
		goto idle_power_collapse_test_bail;
	}
	g_pm_stats[rv] = '\0';

	fprintf(stdout, "Restore IRQ debug mask\n");

	rv = write_string_to_file(NULL, IRQ_DEBUG_MASK_NODE, irq_dmask);
	if (rv < 0) {
		fprintf(stdout,	"cannot write to %s: %s\n",
			IRQ_DEBUG_MASK_NODE, strerror(-rv));
		goto idle_power_collapse_test_bail;
	}

	fprintf(stdout, "====BEGIN power management stats====\n");
	fflush(stdout);
	rv = write_to_fd(STDOUT_FILENO, g_pm_stats, strlen(g_pm_stats));
	if (rv < 0)
		fprintf(stdout,	"cannot write out power management stats\n");
	fprintf(stdout, "====END power management stats====\n");

	fprintf(stdout, "Look for power collapse events\n");

	success_count = parse_pm_stats_count(g_pm_stats,
					     "\nidle-power-collapse:\n  count: ");
	if (success_count < 0) {
			fprintf(stdout,	"bad count(s) from power management stats\n");
			goto idle_power_collapse_test_bail;
	}

	if (success_count > 0) {
		fprintf(stdout, "\n\nIdle power collapse succeeded\n");
		if (g_wakelock_exists) {
			write_string_to_file(NULL, WAKELOCK_UNLOCK_NODE,
					     IDLEPC_WAKELOCK);
		}
		return EXIT_SUCCESS;
	}


	fprintf(stdout, "====BEGIN KERNEL LOG====\n");
	fflush(stdout);
	rv = fork_exec("dmesg", NULL, NULL, NULL, 0);
	if (rv < 0)
		fprintf(stdout,	"cannot dump kernel log\n");
	fprintf(stdout, "====END KERNEL LOG====\n");
	fprintf(stdout, "\n Idle power collapse failed\n");

idle_power_collapse_test_bail:
	if (strlen(irq_dmask) > 0) {
		write_string_to_file(NULL, IRQ_DEBUG_MASK_NODE, irq_dmask);
	}

	if (g_wakelock_exists)
		write_string_to_file(
			NULL, WAKELOCK_UNLOCK_NODE, IDLEPC_WAKELOCK);

	return EXIT_FAILURE;
}
