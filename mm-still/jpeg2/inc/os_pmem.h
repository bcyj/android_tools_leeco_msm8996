/*========================================================================

*//** @file os_pmem.h

This abstract away details about underlying PMEM implementation and provide
a generic API to deal with PMEM. It assumes the underlying implementation
is based on a driver with a driver handle (defined as pmem_fd_t in the
os_pmem_sp.h header file which can be different for different platforms.

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
05/12/09   vma     Created file.

========================================================================== */

#ifndef _OS_PMEM_H
#define _OS_PMEM_H
// Include the os specific portion of the header file
#include "os_pmem_sp.h"

#include <linux/msm_ion.h>

// The value where the unopened pmem driver handle should be initialized with
#define OS_PMEM_FD_INIT_VALUE    OS_PMEM_FD_INIT_VALUE_SP

/*****************************************************************************
 * OS abstraction layer for PMEM
 *****************************************************************************/

/*****************************************************************************
 * Function:    os_pmem_fd_open
 * Description: opens the underlying platform-specific PMEM driver
 *****************************************************************************/
int os_pmem_fd_open(pmem_fd_t *p_pmem_fd, int *ion_fd, struct ion_allocation_data *alloc,
		           struct ion_fd_data *ion_info_fd);

/*****************************************************************************
 * Function:    os_pmem_fd_close
 * Description: closes the previously opened driver
 *****************************************************************************/
int os_pmem_fd_close(pmem_fd_t *p_pmem_fd, int ion_fd,
					 struct ion_fd_data *ion_info_fd);

/*****************************************************************************
 * Function:    os_pmem_allocate
 * Description: uses the opened driver handle to allocate a chunk of
 *              physically contiguous memory given the size and returns the
 *              virtual address of the buffer
 *****************************************************************************/
int os_pmem_allocate(pmem_fd_t pmem_fd, uint32_t size, uint8_t **p_vaddr);

/*****************************************************************************
 * Function:    os_pmem_free
 * Description: releases the previously allocated PMEM buffer
 *****************************************************************************/
int os_pmem_free(pmem_fd_t pmem_fd, uint32_t size, uint8_t *vaddr);

/*****************************************************************************
 * Function:    os_pmem_get_phy_addr
 * Description: obtains the physical address of the buffer
 *****************************************************************************/
int os_pmem_get_phy_addr(pmem_fd_t pmem_fd, uint8_t **p_paddr);

#endif // #ifndef _OS_PMEM_H

