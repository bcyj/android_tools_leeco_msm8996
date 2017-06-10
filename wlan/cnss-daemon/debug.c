/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "debug.h"

#ifdef CONFIG_DEBUG_SYSLOG
#include <syslog.h>
#endif

#ifdef CONFIG_DEBUG_LOGCAT
#include <android/log.h>

#define LOG_TAG "cnss-daemon"
#endif

char *months_abbr[] = {
	"Jan", "Feb", "Mar",
	"Apr", "May", "Jun",
	"Jul", "Aug", "Sep",
	"Oct", "Nov", "Dec"
};

char *msg_abbr[] = {
	"error", "info", "debug", "dump"
};

#ifdef CONFIG_DEBUG_FILE
static FILE *log_file = NULL;
#endif

#ifdef CONFIG_DEBUG_SYSLOG
static int wsvc_debug_syslog;
void wsvc_debug_open_syslog(void)
{
	openlog("cnss-daemon", LOG_PID | LOG_NDELAY, LOG_DAEMON);
	wsvc_debug_syslog++;
}

void wsvc_debug_close_syslog(void)
{
	if(wsvc_debug_syslog)
		closelog();
	wsvc_debug_syslog = 0;
}

static int syslog_priority(int level)
{
	switch (level) {
	case MSG_ERROR:
		return LOG_ERR;
		break;
	case MSG_INFO:
		return LOG_INFO;
	case MSG_DEBUG:
	case MSG_DUMP:
		return LOG_DEBUG;
    }

	return LOG_INFO;
}
#endif /* CONFIG_DEBUG_SYSLOG */

#ifdef CONFIG_DEBUG_LOGCAT
static int logcat_priority(int level)
{
	switch (level) {
	case MSG_ERROR:
		return ANDROID_LOG_ERROR;
		break;
	case MSG_INFO:
		return ANDROID_LOG_INFO;
		break;
	case MSG_DEBUG:
	case MSG_DUMP:
		return ANDROID_LOG_DEBUG;
	}
	return ANDROID_LOG_INFO;
}
#endif /* CONFIG_DEBUG_LOGCAT */

#ifdef CONFIG_DEBUG_FILE
int wsvc_debug_open_file(const char *path)
{
	if (path == NULL)
		return -1;

	log_file = fopen(path, "a");

	if (log_file == NULL) {
		wsvc_printf_err("%s: failed to open the log file: %s",
				__func__, path);
		return -1;
	}

	setlinebuf(log_file);

	return 0;
}

void wsvc_debug_close_file(void)
{
	if (!log_file)
		return;

	fclose(log_file);
	log_file = NULL;
}
#endif

#ifdef CONFIG_DEBUG
static void wsvc_debug_sigsegv_handler(int signum)
{
	wsvc_printf_err("SIGSEGV Crashed! %d\n", signum);
	abort();

	return;
}
#endif

int wsvc_debug_init(void)
{
#ifdef CONFIG_DEBUG
	struct sigaction sig_act;

	memset(&sig_act, 0, sizeof(sig_act));
	sig_act.sa_handler = wsvc_debug_sigsegv_handler;
	sigemptyset(&sig_act.sa_mask);

	sigaction(SIGSEGV, &sig_act, NULL);
#endif

	return 0;
}

static void _wsvc_vprintf(int level, const char *fmt, va_list argp)
{
	FILE *file = stderr;
	time_t now;
	struct tm tm;
	char buf[1024];

	vsnprintf(buf, sizeof(buf), fmt, argp);

#ifdef CONFIG_DEBUG_LOGCAT
	__android_log_print(logcat_priority(level), LOG_TAG, "%s", buf);
	return;
#endif

#ifdef CONFIG_DEBUG_SYSLOG
	if (wsvc_debug_syslog) {
		syslog(syslog_priority(level), "%s", buf);
		return;
	}
#endif

#ifdef CONFIG_DEBUG_FILE
	if (log_file)
		file = log_file;
#endif

	now = time(NULL);
	localtime_r(&now, &tm);

	fprintf(file, "%s %2d %02d:%02d:%02d %5s: %s\n",
		months_abbr[tm.tm_mon], tm.tm_mday, tm.tm_hour,
		tm.tm_min, tm.tm_sec, msg_abbr[level], buf);

	return;
}

void wsvc_printf(int level, const char *fmt, ...)
{
	va_list argp;

	if (level > wsvc_debug_level)
		return;

	va_start(argp, fmt);

	_wsvc_vprintf(level, fmt, argp);

	va_end(argp);

	return;
}

#ifdef CONFIG_DEBUG
#define WSVC_HEXDUMP_COLUMN      16
#define WSVC_ASCII_CHAR(_ch) ((isascii(_ch) && isprint(_ch)) ? _ch : '.')
void _wsvc_hexdump(int level, const char *title, const void *p, int len)
{
	char dump[WSVC_HEXDUMP_COLUMN * 3 + 20];
	int size, count = 0, n = 0;
	int i;
	char ascii[WSVC_HEXDUMP_COLUMN + 2];
	const uint8_t *buf = p;

	if (level > wsvc_debug_level)
		return;

	wsvc_printf(level, "%s - hexdump(len=%d):", title, len);

	while (len > 0) {
		n += snprintf(&dump[n], sizeof(dump) - n, "%04x:", count);
		size = len > WSVC_HEXDUMP_COLUMN ? WSVC_HEXDUMP_COLUMN : len;

		for (i = 0; i < size && n < sizeof(dump); i++) {
			n += snprintf(&dump[n], sizeof(dump) - n,
				      " %02x", buf[i]);
			ascii[i] = WSVC_ASCII_CHAR(buf[i]);
		}

		ascii[i] = '\0';

		for (i = size;
		     i < WSVC_HEXDUMP_COLUMN && n < sizeof(dump); i++)
			n += snprintf(&dump[n], sizeof(dump) - n, "   ");

		wsvc_printf(level, "%s\t%s", dump, ascii);
		buf += size;
		len -= size;
		count += size;
		n = 0;
	}

	return;
}

#endif /* CONFIG_DEBUG */
