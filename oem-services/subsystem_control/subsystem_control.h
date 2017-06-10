/******************************************************************************

  @file	 subsystem_control.h

  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

#ifndef __SUBSYSTEM_CONTROL_H__
#define __SUBSYSTEM_CONTROL_H__

enum procs {
    PROC_START,
    PROC_MSM = PROC_START,
    PROC_MDM,
    PROC_END,
};

extern int subsystem_control_shutdown(unsigned proc_num, char *name);
#endif
