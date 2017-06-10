/*
**
** Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved
** Qualcomm Technologies Confidential and Proprietary.
**
*/
#ifndef DOLBY_DAP_HAL_UTIL
#define DOLBY_DAP_HAL_UTIL

#include <stdbool.h>
#include <system/thread_defs.h>
#include <sys/prctl.h>
#include <pthread.h>

typedef enum {
    SND_CARD            = 0,
    HW_ENDPOINT         = 1,
    DMID                = 2,
    DEVICE_BE_ID_MAP    = 3,
    DAP_BYPASS          = 4,
} dap_hal_hw_info_t;

typedef struct {
     int (*device_id_to_be_id)[2];
     int len;
} dap_hal_device_be_id_map_t;

typedef void* dap_handle_t;

static pthread_mutex_t lock;
static int init_status = -1;

dap_handle_t dap_util_open();
void dap_util_close(dap_handle_t handle);
bool dap_util_get_fd_by_handle(dap_handle_t handle, int* fd);
bool dap_util_get_be_id_by_device_id(int device_id, int* be_id);
bool dap_util_is_hw_unsupported_device(int device_id);
int dap_util_initialize();
dap_handle_t dap_util_get_handle();
bool dap_util_get_endpoint(int endpoint, int *num_endpoint, int *dev_arr);
void dap_util_set_be_id_map(void *data);
int dap_util_set_bypass_type(int fd);

#endif
