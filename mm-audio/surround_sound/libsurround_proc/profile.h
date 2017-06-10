/*============================================================================
  @file profile.h

  Profiling support for measuring processing time.

        Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
        All Rights Reserved.
        Qualcomm Technologies Confidential and Proprietary
============================================================================*/
#ifndef _PROFILE_H_
#define _PROFILE_H_

typedef int fn_init(const char *);
typedef void fn_start(int);
typedef void fn_end(int);

typedef struct {
    fn_init *init;
    fn_start *start;
    fn_end *end;
} Profiler;

#endif /* #ifndef _PROFILE_H_ */
