/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __MODULE_DENOISE_H__
#define __MODULE_DENOISE_H__

/** module_denoise_init:
 *    @name: name of this denoise interface module ("denoise")
 *
 * Initializes new instance of denoise module
 *
 * This function executes in Imaging Server context
 *
 * Returns new instance on success or NULL on fail
 **/
mct_module_t *module_denoise_init(const char *name);

/** module_denoise_deinit:
 *    @module: the instance to be deinitialized
 *
 * Deinitializes instance of denoise module
 *
 * This function executes in Imaging Server context
 *
 * Returns nothing
 **/
void module_denoise_deinit(mct_module_t *module);

#endif //__MODULE_DENOISE_H__
