/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */

#ifndef _ATHTYPES_LINUX_H_
#define _ATHTYPES_LINUX_H_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <sys/types.h>
#endif

typedef int8_t      A_INT8;
typedef int16_t     A_INT16;
typedef int32_t     A_INT32;
typedef int64_t     A_INT64;

typedef u_int8_t     A_UINT8;
typedef u_int16_t    A_UINT16;
typedef u_int32_t    A_UINT32;
typedef u_int64_t    A_UINT64;

typedef int             A_BOOL;
typedef char            A_CHAR;
typedef unsigned char   A_UCHAR;
typedef unsigned long   A_ATH_TIMER;


#endif /* _ATHTYPES_LINUX_H_ */
