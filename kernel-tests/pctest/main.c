/*******************************************************************************
 * -----------------------------------------------------------------------------
 * Copyright (c) 2009-12 Qualcomm Technologies, Inc.  All Rights Reserved.
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

int debug_output = 0;

int g_delay_test_sec;
int g_wakeup_sec;
int g_timeout_sec;
int g_num_iter;
int g_ignore_tcxo;

int g_wakelock_exists;
char *g_sys_pm = NULL;
char *g_resume_command = NULL;
char g_pm_stats[1024 * 16];
char g_wakelock_stats[1024 * 200];
int g_idle_interference_timer_ms;
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

static test_func_t parse_arguments(int argc, char *argv[])
{
	if (argc < 2)
		goto parse_argument_usage_bail;

	if (!strcmp(argv[1], "suspend")) {
		if (argc != 6 && argc != 7)
			goto parse_argument_usage_bail;

		g_delay_test_sec = (int)strtoul(argv[2], NULL, 10);
		g_wakeup_sec = (int)strtoul(argv[3], NULL, 10);
		g_timeout_sec = (int)strtoul(argv[4], NULL, 10);
		g_num_iter = (int)strtoul(argv[5], NULL, 10);
		g_ignore_tcxo = (argc == 7 ) && !strcmp(argv[6], "-tcxo");

		fprintf(stdout, "version: %s\n", TEST_REVISION);
		fprintf(stdout, "test: suspend\n");
		fprintf(stdout, "delay test seconds: %d\n", g_delay_test_sec);
		fprintf(stdout, "wakeup seconds: %d\n", g_wakeup_sec);
		fprintf(stdout, "timeout seconds: %d\n", g_timeout_sec);
		fprintf(stdout, "number of iterations: %d\n", g_num_iter);
		fprintf(stdout, "ignore TCXO: %d\n", g_ignore_tcxo);

		return suspend_test;
	}

	if (!strcmp(argv[1], "idle-pc")) {
		if (argc < 4 || argc > 5)
			goto parse_argument_usage_bail;

		g_delay_test_sec = (int)strtoul(argv[2], NULL, 10);
		g_timeout_sec = (int)strtoul(argv[3], NULL, 10);
		if(argc == 5) {
			g_idle_interference_timer_ms = (int)strtoul(argv[4], NULL, 10);
			if(g_idle_interference_timer_ms < 1 || g_idle_interference_timer_ms > 5000)
				goto parse_argument_usage_bail;
		}
		else
			g_idle_interference_timer_ms = 0;

		fprintf(stdout, "version: %s\n", TEST_REVISION);
		fprintf(stdout, "test: idle-pc\n");
		fprintf(stdout, "delay test seconds: %d\n", g_delay_test_sec);
		fprintf(stdout, "timeout seconds: %d\n", g_timeout_sec);
		fprintf(stdout, "idle_interference_timer milliseconds: %d\n", \
				g_idle_interference_timer_ms);
		return idle_power_collapse_test;
	}

	if (!strcmp(argv[1], "version")) {
		fprintf(stdout, "version: %s\n", TEST_REVISION);
		return NULL;
	}

parse_argument_usage_bail:
	fprintf(stdout, "usage: %s suspend delay_test_seconds "
		"wakeup_seconds timeout_seconds num_iter [-tcxo]\n", argv[0]);
/*
 *  idle_interference_timer: is an optional argument.
 *  Based on the time value in milli-seconds entered here,
 *  A periodic timer is started to interrupt the processor.
 *  The periodic timer can have a value between 1ms to 5000ms.
 */
	fprintf(stdout, "       %s idle-pc delay_test_seconds "
		"timeout_seconds [idle_interference_timer_milliseconds]\n", argv[0]);
	fprintf(stdout, "       %s version\n", argv[0]);

	return NULL;
}

int main(int argc, char *argv[])
{
	test_func_t test = parse_arguments(argc, argv);

	if (test == NULL)
		return EXIT_FAILURE;

	return test();
}
