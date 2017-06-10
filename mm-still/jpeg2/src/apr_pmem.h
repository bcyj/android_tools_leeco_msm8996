/*****************************************************************************
* Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

#ifndef APR_PMEM_H
#define APR_PMEM_H

#include <stdint.h>
#include <inttypes.h>
#include <linux/msm_ion.h>


typedef struct mmap_info_ion
{
    int32_t    ion_fd;
    uint8_t*   *pVirtualAddr;
    uint8_t*   *pPhyAddr;
    size_t     bufsize;
    struct ion_fd_data ion_info_fd;
} mmap_info_ion;


/*===========================================================================

Function            : apr_pmem_alloc_ion

Description         : Allocates a physically contiguous memory in ion CP_HEAP

Input parameter(s)  : size Size of the block in bytes,ionheapid

Output parameter(s) : Ion structure to the allocated memory block; otherwise,
 NULL if the allocation failed.

=========================================================================== */

void apr_pmem_alloc_ion_uncached(struct mmap_info_ion *pmapion,uint32_t ionheapid);

/*===========================================================================

Function            : apr_pmem_alloc_ion

Description         : Allocates a physically contiguous memory in ion CP_HEAP

Input parameter(s)  : size Size of the block in bytes,ionheapid

Output parameter(s) : Ion structure to the allocated memory block; otherwise,
 NULL if the allocation failed.

=========================================================================== */

void apr_pmem_alloc_ion_cached(struct mmap_info_ion *pmapion,uint32_t ionheapid);

/*===========================================================================

Function            : apr_pmem_free_ion

Description         : Free ion memory

Input parameter(s)  : Ion structure to the allocated memory block

Output parameter(s) : None

=========================================================================== */

void apr_pmem_free_ion(struct mmap_info_ion mapion);



#endif /*APR_PMEM_H*/
