/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
 
    This file defines the media/module/master controller's focus logic in
   the mm-camera server. The functionalities of this modules are:

   1. config, process/parse awb/aec stats buffers and events
   2. control the statsproc interface
 
============================================================================*/
#ifndef __MCTL_STATS_H__
#define __MCTL_STATS_H__

int mctl_stats_proc_MSG_ID_STATS_AWB(void *parm1, void *parm2);
int mctl_stats_proc_MSG_ID_STATS_AE(void *parm1, void *parm2);
int mctl_stats_proc_MSG_ID_STATS_RS(void *parm1, void *parm2);
int mctl_stats_proc_MSG_ID_STATS_CS(void *parm1, void *parm2);
int mctl_stats_proc_MSG_ID_STATS_WB_EXP(void *parm1, void *parm2);
int mctl_stats_proc_MSG_ID_STATS_BG(void *parm1, void *parm2);
int mctl_stats_proc_MSG_ID_STATS_BE(void *parm1, void *parm2);
int mctl_stats_proc_MSG_ID_STATS_BF(void *parm1, void *parm2);
int mctl_stats_proc_MSG_ID_STATS_BHIST(void *parm1, void *parm2);

#endif /* __MCTL_STATS_H__ */
