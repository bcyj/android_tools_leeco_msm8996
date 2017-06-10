/* mtype.h
 *  														 .
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __M_TYPE_H__
#define __M_TYPE_H__

#include <stdint.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/socket.h>
#include <linux/fb.h>
#include <linux/videodev2.h>
#include <linux/media.h>
#include <linux/msm_ion.h>
#include <media/msmb_camera.h>

/* =====================================================================
 * ***** Important: Provide type definitions for common used types *****
 * =====================================================================
 **  
*/

typedef int   boolean;
#undef  FALSE
#define FALSE (0)

#undef  TRUE
#define TRUE  (1)

#define PAD_TO_WORD(a)               (((a)+3)&~3)
#define PAD_TO_2K(a)                 (((a)+2047)&~2047)
#define PAD_TO_4K(a)                 (((a)+4095)&~4095)
#define PAD_TO_8K(a)                 (((a)+8191)&~8191)

#define CEILING32(X) (((X) + 0x0001F) & 0xFFFFFFE0)
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING4(X)  (((X) + 0x0003) & 0xFFFC)
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)

#undef __FD_SET
#define __FD_SET(fd, fdsetp) \
  (((fd_set *)(fdsetp))->fds_bits[(fd) >> 5] |= (1<<((fd) & 31)))

#undef __FD_CLR
#define __FD_CLR(fd, fdsetp) \
  (((fd_set *)(fdsetp))->fds_bits[(fd) >> 5] &= ~(1<<((fd) & 31)))

#undef  __FD_ISSET
#define __FD_ISSET(fd, fdsetp) \
  ((((fd_set *)(fdsetp))->fds_bits[(fd) >> 5] & (1<<((fd) & 31))) != 0)

#undef  __FD_ZERO
#define __FD_ZERO(fdsetp) \
  (memset (fdsetp, 0, sizeof (*(fd_set *)(fdsetp))))


typedef enum {
  M_BAYER_8_BITS,
  M_BAYER_10_BITS,
  M_BAYER_12_BITS,
  M_YCbCr420,
  M_YCrCb420,
  M_YCbCr422,
  M_YCrCb422,
  M_OPAQUE
} MTypePixelFormat;

#define MTYPE_MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MTYPE_MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MTYPE_ABS(a)	   (((a) < 0) ? -(a) : (a))
#define MTYPE_CLAMP(x, low, high) \
	(((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/*
 * _M_TYPE_WAIT_TIMEOUT: 
 *  cond:    POSIX conditional variable;
 *  mutex:   POSIX mutex;
 *  timeout: type of signed long long,  specified
 *           timeout measured in nanoseconds;
 *           timeout = -1 means no timeout, it becomes
 *           to regular conditional timewait.
 * 
 *  Commonly used for timeout waiting.
 * */ 
#define _M_TYPE_WAIT_TIMEOUT(cond, mutex, timeout)   \
	do {                                               \
    signed long long end_time;                       \
    struct timeval r;                                \
    struct timespec ts;                              \
		if (timeout != 0xffffffffffffffff) {             \
        gettimeofday(&r, NULL);                      \
        end_time = (((((signed long long)r.tv_sec) * 1000000) + r.tv_usec) + \
	 		            (timeout / 1000));                  \
	 		 ts.tv_sec  = (end_time / 1000000);             \
	 		 ts.tv_nsec = ((end_time % 1000000) * 1000);    \
	 		 ret = pthread_cond_timedwait(cond, mutex, &ts);\
		} else {                                          \
		   ret = pthread_cond_wait(cond, mutex);          \
		}                                                 \
	} while(0) 

#define _M_TYPE_OFFSET(s_type, field) ((long)&(((s_type *)0)->field))
#endif /* __M_TYPE_H__ */
