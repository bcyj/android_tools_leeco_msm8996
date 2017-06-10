/* Copyright (c) 2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef _FAKE_LOGD_WRITE_H_
#define _FAKE_LOGD_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

int __android_log_print(int prio, const char *tag, const char *fmt, ...);

void __android_log_assert(const char *cond, const char *tag,const char *fmt, ...);

#ifdef __cplusplus
}
#endif


#endif
