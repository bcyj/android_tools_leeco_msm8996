/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>
#include <errno.h>
#include <string.h>

enum {
	MSG_ERROR,
	MSG_INFO,
	MSG_DEBUG,
	MSG_DUMP
};

extern int wsvc_debug_level;

#define MSG_DEFAULT      MSG_INFO

#ifdef __GNUC__
#define PRINTF_FORMAT(a,b) __attribute__ ((format (printf, (a), (b))))
#else
#define PRINTF_FORMAT(a,b)
#endif

#define wsvc_printf_err(_fmt...) wsvc_printf(MSG_ERROR, _fmt)
#define wsvc_perror(_str) wsvc_printf(MSG_ERROR, "%s: %s(%d)",\
				      _str, strerror(errno), errno)

#ifdef CONFIG_DEBUG

void _wsvc_hexdump(int level, const char *title, const void *p,
		   int len);

#define wsvc_hexdump(title, p, len)  _wsvc_hexdump(MSG_DUMP, title, p, len)
#define wsvc_printf_info(_fmt...) wsvc_printf(MSG_INFO, _fmt)
#define wsvc_printf_dbg(_fmt...) wsvc_printf(MSG_DEBUG, _fmt)

#else

#define wsvc_hexdump(tittle, p, len) do { } while (0)
#define wsvc_printf_info(_fmt...) do { } while (0)
#define wsvc_printf_dbg(_fmt...) do { } while (0)

#endif

int wsvc_debug_init(void);
void wsvc_printf(int level, const char *fmt, ...) PRINTF_FORMAT(2, 3);

#ifdef CONFIG_DEBUG_FILE
int wsvc_debug_open_file(const char *path);
void wsvc_debug_close_file(void);
#else
static inline int wsvc_debug_open_file(const char *path) { return 0; }
static inline void wsvc_debug_close_file(void) { return; }
#endif

#ifdef CONFIG_DEBUG_SYSLOG
void wsvc_debug_open_syslog(void);
void wsvc_debug_close_syslog(void);
#else
static inline void wsvc_debug_open_syslog(void) { return; }
static inline void wsvc_debug_close_syslog(void) { return; }
#endif

#endif /* __DEBUG_H */
