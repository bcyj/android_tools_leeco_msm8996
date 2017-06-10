/**********************************************************************
* Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __EZTUNE_VFE_UTIL_H__
#define __EZTUNE_VFE_UTIL_H__
#include "eztune_vfe_diagnostics.h"

vfe_status_t ez_vfe_set(void *ctrl_obj, void *param1, void *param2);
void ez_vfe_diagnostics(void *ctrl_obj);
void ez_vfe_diagnostics_update(void *ctrl_obj);

#endif /* __EZTUNE_VFE_UTIL_H__ */
