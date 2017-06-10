/*========================================================================


*//** @file jpeg_buffer_private.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/03/09   vma     Switched to use the os abstraction layer (os_*)
*//*====================================================================== */

#ifndef __JPEG_BUFFER_PRIV_H
#define __JPEG_BUFFER_PRIV_H
#include "os_thread.h"
#include "jpeg_buffer.h"

#include <linux/msm_ion.h>

typedef struct jpeg_buf_t
{
    uint8_t         *ptr;         // The pointer to the start of the buffer
    uint8_t         *phy_addr;    // The physical address of the buffer (valid only when it
                                  // is internally allocated with use_pmem enabled)
    uint32_t         reserved;    // Reserved
    uint32_t         size;        // The size of the buffer
    uint32_t         offset;      // The reader of the buffer shall consider this as the
                                  // length of valid data in the buffer
                                  // The writer of the buffer shall consider this as the
                                  // position where next byte shall be written
    pmem_fd_t        pmem_fd;     // It holds the pmem_fd specific to the OS regarding
                                  // to pmem allocation (e.g. driver file descriptor)
    uint8_t          is_inited;   // The flag indicating whether it has been intialized
    uint8_t          is_internal; // The flag indicating whether the buffer is allocated
                                  // internally
    uint8_t          is_empty;    // The flag indicating whether the buffer is empty
    uint8_t          is_busy;     // the flag indicating whether the buffer is being filled
                                  // or emptied
    os_mutex_t       mutex;       // The mutex for locking use
    os_cond_t        cond;        // The condition variable signaling the end of consumption
    uint32_t         start_offset;// The start offset from which data needs to be read
    uint32_t         phy_offset;  // The phy address offset from which data needs to be read

    int ion_fd;
    struct ion_allocation_data alloc_data;
	struct ion_fd_data info_fd;
} jpeg_buf_t;

/**
 * Private routines for handling synchronization for buffer to
 * be shared by multiple working threads */

/******************************************************************************
 * Function: jpeg_buffer_wait_until_empty
 * Description: This routine blocks until the buffer is marked as empty by
 *              using jpeg_buffer_mark_empty by another thread.
 *****************************************************************************/
void jpeg_buffer_wait_until_empty(jpeg_buf_t *p_buf);

/******************************************************************************
 * Function: jpeg_buffer_wait_until_filled
 * Description: This routine blocks until the buffer is marked as filled by
 *              using jpeg_buffer_mark_filled by another thread.
 *****************************************************************************/
void jpeg_buffer_wait_until_filled(jpeg_buf_t *p_buf);

/******************************************************************************
 * Function: jpeg_buffer_mark_consumed
 * Description: This routine unblocks any threads that is blocked on the buffer
 *              waiting for it to be marked as empty.
 *****************************************************************************/
void jpeg_buffer_mark_empty(jpeg_buf_t *p_buf);

/******************************************************************************
 * Function: jpeg_buffer_mark_filled
 * Description: This routine marks the buffer as filled.
 *****************************************************************************/
void jpeg_buffer_mark_filled(jpeg_buf_t *p_buf);

/******************************************************************************
 * Function: jpeg_buffer_mark_busy
 * Description: This routine marks the buffer busy being filled or emptied.
 *****************************************************************************/
void jpeg_buffer_mark_busy(jpeg_buf_t *p_buf);

#endif // #ifndef __JPEG_BUFFER_PRIV_H
