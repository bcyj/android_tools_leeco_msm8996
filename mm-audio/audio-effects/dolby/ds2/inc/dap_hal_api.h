/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2014 Dolby Laboratories, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DOLBY_DAP_HAL_API_
#define DOLBY_DAP_HAL_API_

#define DAP_HAL_LIB "libhwdaphal.so"
#include <system/audio.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
    Pointer which refer to internal DAP structure.
*/
typedef void* dap_handle_t;

/**
    Enumeration for commands supported by DAP kernel driver.
*/
typedef enum
{
    DAP_CMD_COMMIT_ALL         = 0,
    DAP_CMD_COMMIT_CHANGED     = 1,
    DAP_CMD_USE_CACHE_FOR_INIT = 2,
    DAP_CMD_SET_BYPASS         = 3,
    DAP_CMD_SET_ACTIVE_DEVICE  = 4,
    DAP_CMD_SET_BYPASS_TYPE    = 5,

} dap_cmd_t;

/**
    dap_open() Initializes a connection with DAP driver.
    Returns pointer to an internal structure corresponding to DAP instance.
    Returns NULL if DAP cannot be initialized.
*/
dap_handle_t dap_open();

/**
    dap_close() Closes a previously opened DAP instance.
*/
void dap_close(dap_handle_t handle);

/**
    dap_command() Send a command to driver. Returns negative value on error.
    Only one integer is sent as data with a command.
*/
int dap_command(dap_handle_t handle, audio_devices_t device_id,
                dap_cmd_t command, int32_t data);

/**
    dap_set_param() Set single parameter in the cache.
    Returns negative value on error.
*/
int dap_set_param(dap_handle_t handle, audio_devices_t device_id,
                  int32_t param_id, const int32_t* data, int32_t length);

/**
    dap_get_param() Get single parameter from DAP.
    Returns number of values read on success and negative value on error.
*/
int dap_get_param(dap_handle_t handle, audio_devices_t device_id,
                  int32_t param_id, int32_t* data, int32_t* length);

/**
    dap_get_visualizer() Get visualizer data from an active DAP instance.
    Returns number of values read on success and negative value on error.
*/
int dap_get_visualizer(dap_handle_t handle, int32_t* data, int32_t* length);

int dap_hal_set_hw_info(int32_t hw_info, void* data);

#ifdef __cplusplus
}
#endif
#endif//DOLBY_DAP_HAL_API_
