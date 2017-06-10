/*-------------------------------------------------------------------
Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------------*/

#ifndef _VT_ION_ALLOCATOR_H
#define _VT_ION_ALLOCATOR_H

#include <stdlib.h>
#include "linux/msm_ion.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ion_buffer_data {
    struct ion_fd_data fd_data;
    struct ion_allocation_data alloc_data;
    void *data;
};

int vt_ion_allocate(int secure, void** client_ion_buffer, size_t size);

int vt_ion_free(void* client_ion_buffer);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VT_ION_ALLOCATOR_H
