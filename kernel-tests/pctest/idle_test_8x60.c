/*******************************************************************************
 * -----------------------------------------------------------------------------
 * Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * -----------------------------------------------------------------------------
 ******************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef ANDROID
#include <sys/syscall.h>
#define gettid() ((pid_t) syscall(SYS_gettid))
#endif

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

static void *idle_interference_timer_function(void *arg) {
	pthread_mutex_lock(&thread_mutex);
	struct thread_args *cpu_timer_args = (struct thread_args *)arg;
	int tid = gettid();
	int ms = cpu_timer_args->ms;
	int cpu = cpu_timer_args->cpu;
	int return_val = 1;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	if (sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset) == -1)
		fprintf(stdout, "Could not set affinity of thread %d " \
			"to cpu %d: %s\n", tid, cpu, strerror(errno));
	pthread_mutex_unlock(&thread_mutex);

	while(1)
		usleep(1000 * ms);

	return &return_val;
}

int idle_power_collapse_test(void)
{
	char irq_dmask[16] = "";
	time_t now;
	time_t expiration;
	int success_count;
	int success_cnt_cpu0, success_cnt_cpu0_std;
	int success_cnt_cpu1, success_cnt_cpu2, success_cnt_cpu3;
	int rv;
	unsigned int core1_status;
	unsigned int core2_status;
	unsigned int core3_status;
	unsigned int num_cores = 0;
	pthread_t idle_interference_timer_thread[2];
	int cpu_count = 0;
	struct thread_args timer_thread_args;

	int exit_val = EXIT_FAILURE;

	fprintf(stdout, "Check whether power management is up\n");
	if (!file_exists(PM_STATS_NODE)) {
		fprintf(stdout, "power management is not available\n");
				goto idle_pc_test_early_bailout;
	}

	if (directory_exists(SYS_PM_8x60)) {
		g_sys_pm = SYS_PM_8x60;
		if (file_exists_with_prefix(SYS_PM_8x60, IDLE_SLEEP_MODE_NODE_CORE_3))
			num_cores = 3;
		else if (file_exists_with_prefix(SYS_PM_8x60, IDLE_SLEEP_MODE_NODE_CORE_1))
			num_cores = 2;
		else
			num_cores = 1;
	}
	else {
		fprintf(stdout, "power management is not available\n");
		goto idle_pc_test_early_bailout;
	}

	fprintf(stdout, "Check idle power collapse configuration\n");
	if ((num_cores == 2) || (num_cores == 4)) {
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

		fprintf(stdout, "Turn off mp-decision before testing idle PC\n");
		rv = fork_exec("stop", "mpdecision", NULL, NULL, 0);
		if (rv < 0) {
			fprintf(stdout, "cannot turn off mp-decision: %s\n", strerror(-rv));
			goto idle_pc_test_early_bailout;
		}

		fprintf(stdout, "Now explicitly turn ON core 1 if not ON already\n");
		core1_status = read_unsigned_int_from_file(NULL, HOTPLUG_NODE_CORE_1);
		if (!core1_status) {
			fprintf(stdout, "turning on core 1\n");
			rv = write_int_to_file(NULL, HOTPLUG_NODE_CORE_1, 1);
		}

		if (num_cores == 4) {

			fprintf(stdout, "Now explicitly turn ON core 2 if not ON already\n");
			core2_status = read_unsigned_int_from_file(NULL, HOTPLUG_NODE_CORE_2);
			if (!core2_status) {
				fprintf(stdout, "turning on core 2\n");
				rv = write_int_to_file(NULL, HOTPLUG_NODE_CORE_2, 1);
			}

			fprintf(stdout, "Now explicitly turn ON core 3 if not ON already\n");
			core3_status = read_unsigned_int_from_file(NULL, HOTPLUG_NODE_CORE_3);
			if (!core3_status) {
				fprintf(stdout, "turning on core 3\n");
				rv = write_int_to_file(NULL, HOTPLUG_NODE_CORE_3, 1);
			}
		}
	}
	else if (num_cores == 1) {

		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_CORE_0, 1);
		write_int_to_file(g_sys_pm, IDLE_SLEEP_MODE_NODE_STD_CORE_0, 1);
	}

	/*Start interference timers.*/
	if (g_idle_interference_timer_ms > 0) {
		for (cpu_count = 0; cpu_count < num_cores; cpu_count++) {
			fprintf(stdout, "start idle_interference_timer on core %d\n", \
					cpu_count);
			pthread_mutex_lock(&thread_mutex);
			timer_thread_args.cpu = cpu_count;
			timer_thread_args.ms = g_idle_interference_timer_ms;
			pthread_mutex_unlock(&thread_mutex);
			pthread_create(&idle_interference_timer_thread[cpu_count], \
				NULL, \
				idle_interference_timer_function, \
				&timer_thread_args);
		}
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
			goto idle_pc_test_bailout;
		}
	}

	fprintf(stdout, "Delay tests for some time\n");
	if (g_delay_test_sec) {
		unsigned int seconds = g_delay_test_sec;
		while (seconds)
			seconds = sleep(seconds);
	}

	fprintf(stdout, "Save current IRQ debug mask\n");
	rv = read_from_file(NULL, IRQ_DEBUG_MASK_NODE_8x60, irq_dmask,
			    sizeof(irq_dmask) - 1);
	if (rv < 0) {
		fprintf(stdout,	"cannot read %s: %s\n",
			IRQ_DEBUG_MASK_NODE_8x60, strerror(-rv));
		goto idle_pc_test_bailout;
	}
	irq_dmask[rv] = '\0';

	fprintf(stdout,	"Turn on additional IRQ debug info\n");
	rv = write_string_to_file(NULL, IRQ_DEBUG_MASK_NODE_8x60, "1\n");
	if (rv < 0) {
		fprintf(stdout,	"cannot write to %s: %s\n",
			IRQ_DEBUG_MASK_NODE_8x60, strerror(-rv));
		goto idle_pc_test_bailout;
	}

	fprintf(stdout, "Clear kernel log\n");
	rv = fork_exec("dmesg", "-c", NULL, NULL, 1);
	if (rv < 0) {
		fprintf(stdout,
			"dmesg -c failed: %s\n", strerror(-rv));
		goto idle_pc_test_bailout;
	}

	fprintf(stdout, "Clear power management stats\n");
	rv = write_string_to_file(NULL, PM_STATS_NODE, "reset\n");
	if (rv < 0) {
		fprintf(stdout,	"cannot write to %s: %s\n",
			PM_STATS_NODE, strerror(-rv));
		goto idle_pc_test_bailout;
	}

	fprintf(stdout, "Determine expiration time\n");
	now = time(NULL);
	if (now == (time_t)-1) {
		fprintf(stdout,
			"time() failed: %s\n", strerror(errno));
		goto idle_pc_test_bailout;
	}
	expiration = now  + g_timeout_sec;

	fprintf(stdout, "Wait till expiration\n");
	while (expiration > now) {
		sleep(expiration - now);
		now = time(NULL);
		if (now == (time_t)-1) {
			fprintf(stdout,	"time() failed: %s\n",
				strerror(errno));
			goto idle_pc_test_bailout;
		}
	}

	fprintf(stdout, "Load power management stats\n");
	rv = read_from_file(NULL,
			    PM_STATS_NODE, g_pm_stats, sizeof(g_pm_stats) - 1);
	if (rv < 0) {
		fprintf(stdout,
			"cannot read %s: %s\n",	PM_STATS_NODE, strerror(-rv));
		goto idle_pc_test_bailout;
	}

	if (rv == sizeof(g_pm_stats) - 1) {
		fprintf(stdout,	"buffer too small for %s\n", PM_STATS_NODE);
		goto idle_pc_test_bailout;
	}
	g_pm_stats[rv] = '\0';


	fprintf(stdout, "Look for power collapse events\n");

	success_cnt_cpu0 = parse_pm_stats_count(g_pm_stats,
						"\n[cpu 0] idle-power-collapse:\n  count: ");
	if (success_cnt_cpu0 < 0) {
		fprintf(stdout,	"bad count(s) from power management stats\n");
		goto idle_pc_test_bailout;
	}
	if (success_cnt_cpu0 > 0) {
		fprintf(stdout, "\n\nIdle power collapse for cpu 0 succeeded\n");
	}

	success_cnt_cpu0_std = parse_pm_stats_count(g_pm_stats,
			"\n[cpu 0] idle-standalone-power-collapse:\n  count: ");
	if (success_cnt_cpu0_std < 0) {
		fprintf(stdout,	"bad count(s) from power management stats\n");
		goto idle_pc_test_bailout;
	}
	if (success_cnt_cpu0 > 0) {
		fprintf(stdout, "\n\nIdle standalone power collapse for cpu 0 succeeded\n");

	}

	if (num_cores == 2) {

		success_cnt_cpu1 = parse_pm_stats_count(g_pm_stats,
							"\n[cpu 1] idle-standalone-power-collapse:\n  count: ");
		if (success_cnt_cpu1 < 0) {
			fprintf(stdout,	"bad count(s) from power management stats\n");
			goto idle_pc_test_bailout;
		}
		if (success_cnt_cpu1 > 0) {
			fprintf(stdout, "\n\nIdle standalone power collapse for "
				"cpu 1 succeeded\n");
		}
	}

	if (num_cores == 4) {

		success_cnt_cpu1 = parse_pm_stats_count(g_pm_stats,
							"\n[cpu 1] idle-standalone-power-collapse:\n  count: ");
		if (success_cnt_cpu1 < 0) {
			fprintf(stdout,	"bad count(s) from power management stats\n");
			goto idle_pc_test_bailout;
		}
		if (success_cnt_cpu1 > 0) {
			fprintf(stdout, "\n\nIdle standalone power collapse for "
				"cpu 1 succeeded\n");
		}

		success_cnt_cpu2 = parse_pm_stats_count(g_pm_stats,
							"\n[cpu 2] idle-standalone-power-collapse:\n  count: ");
		if (success_cnt_cpu2 < 0) {
			fprintf(stdout,	"bad count(s) from power management stats\n");
			goto idle_pc_test_bailout;
		}
		if (success_cnt_cpu2 > 0) {
			fprintf(stdout, "\n\nIdle standalone power collapse for "
				"cpu 2 succeeded\n");
		}

		success_cnt_cpu3 = parse_pm_stats_count(g_pm_stats,
							"\n[cpu 3] idle-standalone-power-collapse:\n  count: ");
		if (success_cnt_cpu3 < 0) {
			fprintf(stdout,	"bad count(s) from power management stats\n");
			goto idle_pc_test_bailout;
		}
		if (success_cnt_cpu3 > 0) {
			fprintf(stdout, "\n\nIdle standalone power collapse for "
				"cpu 3 succeeded\n");
		}
	}

	if (num_cores == 1) {
		if (success_cnt_cpu0 > 0)
			exit_val = EXIT_SUCCESS;
		else
			fprintf(stdout, "\n Idle power collapse failed\n");
	}
	else if (num_cores == 2) {
		if ((success_cnt_cpu0 > 0) &&
		    (success_cnt_cpu1 > 0))
			exit_val = EXIT_SUCCESS;
		else
			fprintf(stdout, "\n Idle power collapse failed\n");
	}
	else if (num_cores == 4) {
		if ((success_cnt_cpu0 > 0) &&
		    (success_cnt_cpu1 > 0) &&
		    (success_cnt_cpu2 > 0) &&
		    (success_cnt_cpu3 > 0))
			exit_val = EXIT_SUCCESS;
		else
			fprintf(stdout, "\n Idle power collapse failed\n");
	}


	fprintf(stdout, "====BEGIN power management stats====\n");
	fflush(stdout);
	rv = write_to_fd(STDOUT_FILENO, g_pm_stats, strlen(g_pm_stats));
	if (rv < 0)
		fprintf(stdout,	"cannot write out power management stats\n");
	fprintf(stdout, "====END power management stats====\n");

	fprintf(stdout, "====BEGIN KERNEL LOG====\n");
	fflush(stdout);
	rv = fork_exec("dmesg", NULL, NULL, NULL, 0);
	if (rv < 0)
		fprintf(stdout,	"cannot dump kernel log\n");
	fprintf(stdout, "====END KERNEL LOG====\n");


idle_pc_test_bailout:

	/* restore IRQ debug mask */
	write_string_to_file(NULL, IRQ_DEBUG_MASK_NODE_8x60, irq_dmask);

	if (g_wakelock_exists)
		write_string_to_file(
			NULL, WAKELOCK_UNLOCK_NODE, IDLEPC_WAKELOCK);

	if (( num_cores == 2) || (num_cores == 4)) {

		  fprintf(stdout, "Turn mp-decision back on \n");
		  rv = fork_exec("start", "mpdecision", NULL, NULL, 0);
	}

idle_pc_test_early_bailout:

	return exit_val;
}
