/* =======================================================================
                              WFDMMIonMemory.cpp
DESCRIPTION
This module is for WFD source implementation
INteracts with the ION driver for allocating requested memory

Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History
$Header:
$DateTime:
$Change:
========================================================================== */
#include "errno.h"
#include "WFDMMIonMemory.h"

/*!*************************************************************************
  * @brief        Allocate memory on ION pool
 *
 * @param[in]    Buffer size
 * @param[out]   ION memory handle
 *
 * @return       Error Code
 ***************************************************************************/
int allocate_ion_mem(unsigned int size, ion_user_handle_t *handle, int ionfd, int heap_id, OMX_BOOL secure_memory)
{
    struct ion_allocation_data alloc_data;
    struct ion_fd_data fd_data;
    int rc = -1;
    alloc_data.len = size;
    //TODO, alignment may be required
    alloc_data.align = 4096;
    alloc_data.heap_id_mask= ION_HEAP(heap_id);
    alloc_data.flags = 0;
    //alloc_data.align = 8092;
    //alloc_data.flags = 0x1 << ION_CP_MM_HEAP_ID;
    alloc_data.handle = 0;
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMIonMemory::allocate_ion_mem allocating size %d   heap-id  %d \n",size,heap_id);
    if(secure_memory == OMX_TRUE)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMIonMemory::allocate_ion_mem allocating mem in secure mode\n");
       alloc_data.flags |= ION_SECURE;
       alloc_data.align = 1024 * 1024;
    }
    else
    {
     if(heap_id != ION_QSECOM_HEAP_ID)
     {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMIonMemory::allocate_ion_mem allocating mem in NON secure mode\n");
      alloc_data.heap_id_mask |=  ION_HEAP(ION_IOMMU_HEAP_ID);
     }
    }

    rc = ioctl(ionfd,ION_IOC_ALLOC,&alloc_data);
    if ( rc  || (alloc_data.handle == 0) )
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMIonMemory::Failed to allocate ion memory %s",strerror(errno));
        return -ENOMEM;
    }
    *handle = fd_data.handle = alloc_data.handle;
    rc = ioctl(ionfd,ION_IOC_MAP,&fd_data);
    if (rc)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMIonMemory::Failed to MAP ion memory\n");
        /*TODO: Handle error*/
        return -ENOMEM;
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFDMMIonMemory::allocated ion mem fd = %d",fd_data.fd);
    return fd_data.fd;
}

