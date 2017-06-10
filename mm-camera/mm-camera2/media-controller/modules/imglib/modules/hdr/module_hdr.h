/**********************************************************************
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
 * Qualcomm Technologies Proprietary and Confidential.                 *
 **********************************************************************/

#ifndef __MODULE_HDR_H__
#define __MODULE_HDR_H__

/** module_hdr_init:
 *    @name: name of this hdr interface module ("hdr")
 *
 * Initializes new instance of hdr module
 *
 * This function executes in Imaging Server context
 *
 * Returns new instance on success or NULL on fail
 **/
mct_module_t *module_hdr_init(const char *name);

/** module_hdr_deinit:
 *    @module: the instance to be deinitialized
 *
 * Deinitializes instance of hdr module
 *
 * This function executes in Imaging Server context
 *
 * Returns nothing
 **/
void module_hdr_deinit(mct_module_t *module);

#endif //__MODULE_HDR_H__
