/*========================================================================

*//** @file jpeg_buffer.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011,2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/29/10   gbc     Fixed lint errors (removed pmem_fd check < 0, since uint)
02/16/10   staceyw Fixed condition check on actual_size > p_buf->size
                   instead of actual_size < p_buf->size
                   inside function jpeg_buffer_set_actual_size.
08/07/09   vma     Removed 2^n buffer size requirement for pmem.
08/03/09   vma     Switched to use the os abstraction layer (os_*) instead
                   of pthread API for thread/mutex/cv creations/destruction
                   and also the pmem usage
07/14/09   vma     Reverted the change which saves the aligned (and clp2-ed)
                   size as the buffer size instead of the requested size.
                   Some clients rely on the assumption that the requested
                   size will be the same as the actual allocated size.
06/23/09   zhiminl Fixed pmem_fd < 0 when invalid, not <= 0.
06/23/09   mingy   Added comment to clp2.
01/22/09   vma     Fixed incorrect alignment macro and added clp2
                   size adjustment.
10/15/08   vma     Fixed improper resetting.
*//*====================================================================== */

#include "jpeg_buffer_private.h"
#include "jpeg_common_private.h"
#include "jpegerr.h"
#include "jpeglog.h"

#include "os_thread.h"
#include "os_pmem.h"

/******************************************************************************
 * Function: jpeg_buffer_init
 * Description: Initializes the Jpeg buffer object. jpeg_buffer_destroy should
 *              be called clean up internal variables properly.
 * Input parameters:
 *   p_buffer  - The pointer to the Jpeg buffer object to be initialized.
 * Return values:
 *   JPEGERR_SUCCESS
 *   JPEGERR_ENULLPTR
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_init(jpeg_buffer_t *p_buffer)
{
    jpeg_buf_t *p_buf;

    if (!p_buffer)
    {
        return JPEGERR_ENULLPTR;
    }

    // Allocate the internal jpeg_buf_t structure
    p_buf = (jpeg_buf_t *)JPEG_MALLOC(sizeof(jpeg_buf_t));

    *p_buffer = (jpeg_buffer_t)p_buf;
    if (!p_buf)
    {
        return JPEGERR_EMALLOC;
    }

    // Zero out all fields
    STD_MEMSET(p_buf, 0, sizeof(jpeg_buf_t));

    p_buf->pmem_fd = OS_PMEM_FD_INIT_VALUE;
    p_buf->is_inited = true;
    p_buf->is_empty = true;
    os_mutex_init(&(p_buf->mutex));
    os_cond_init(&(p_buf->cond));

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_allocate
 * Description: There are two ways to set up the Jpeg Buffer object to oversee
 *              a valid buffer. One is to instruct it to internally allocate
 *              a buffer. Another is to supply an externally allocated buffer
 *              to the Jpeg Buffer object. One should call jpeg_buffer_allocate
 *              to instruct the Jpeg Buffer to internally allocate for a valid
 *              buffer.
 * Input parameters:
 *   buffer    - The Jpeg buffer object.
 *   size      - The size of the buffer to be allocated
 *   use_pmem  - A flag to indicate whether the allocation should be made on
 *               a mmaped physical memory (for shared access by accelerating
 *               hardware.)
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAIED
 *    JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_allocate(jpeg_buffer_t buffer, uint32_t size, uint8_t use_pmem)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;

    if (!p_buf || !p_buf->is_inited || !size || p_buf->ptr)
    {
        return JPEGERR_EFAILED;
    }

    if (use_pmem)
    {
        p_buf->alloc_data.len = size;
        p_buf->alloc_data.align = 4096;
        p_buf->alloc_data.flags = ION_FLAG_CACHED;
        p_buf->alloc_data.heap_id_mask = 0x1 << ION_IOMMU_HEAP_ID;
        int rc = os_pmem_fd_open(&p_buf->pmem_fd, &p_buf->ion_fd,
                                 &p_buf->alloc_data, &p_buf->info_fd);
        if (JPEG_SUCCEEDED(rc))
        {
            p_buf->size = size;
            rc = os_pmem_allocate(p_buf->pmem_fd, size, &(p_buf->ptr));
        }
        if (JPEG_SUCCEEDED(rc))
        {
            rc = os_pmem_get_phy_addr(p_buf->pmem_fd, &(p_buf->phy_addr));
        }
        if (JPEG_FAILED(rc))
        {
            os_pmem_fd_close(&(p_buf->pmem_fd), p_buf->ion_fd, &p_buf->info_fd);
            p_buf->ptr = NULL;
            p_buf->size = 0;
            return rc;
        }
    }
    // use standard malloc to obtain allocation
    else
    {
        p_buf->ptr = (uint8_t *)JPEG_MALLOC(size);
        if (!p_buf->ptr)
        {
            return JPEGERR_EMALLOC;
        }
        p_buf->size = size;
    }
    p_buf->offset      = 0;
    p_buf->is_internal = true;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_get_pmem_fd
 * Description: This returns the file descriptor used to do mmap.
 * Input parameters:
 *   buffer    - The Jpeg buffer object.
 * Return values:
 *   The requested descriptor
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
pmem_fd_t jpeg_buffer_get_pmem_fd(jpeg_buffer_t buffer)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    if (!p_buf || !p_buf->is_inited || !p_buf->ptr)
    {
        return OS_PMEM_FD_INIT_VALUE;
    }
    return p_buf->pmem_fd;
}

/******************************************************************************
 * Function: jpeg_buffer_get_addr
 * Description: This returns the address of the managed buffer.
 * Input parameters:
 *   buffer    - The Jpeg buffer object.
 *   pp_addr   - The pointer to the buffer address to be returned.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_get_addr(jpeg_buffer_t buffer, uint8_t **pp_addr)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    if (!p_buf || !p_buf->is_inited || !p_buf->ptr || !pp_addr)
    {
        return JPEGERR_EFAILED;
    }
    *pp_addr = p_buf->ptr;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_get_max_size
 * Description: This returns the maximum size of the managed buffer (the size
 *              that was used during allocation).
 * Input parameters:
 *   buffer     - The Jpeg buffer object.
 *   p_max_size - The pointer to the max size to be returned.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_get_max_size(jpeg_buffer_t buffer, uint32_t *p_max_size)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    if (!p_buf || !p_buf->is_inited || !p_buf->ptr || !p_max_size)
    {
        return JPEGERR_EFAILED;
    }
    *p_max_size = p_buf->size;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_get_actual_size
 * Description: This returns the actual size of the valid data in the buffer.
 * Input parameters:
 *   buffer        - The Jpeg buffer object.
 *   p_actual_size - The pointer to the actual size to be returned.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_get_actual_size(jpeg_buffer_t buffer, uint32_t *p_actual_size)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    if (!p_buf || !p_buf->is_inited || !p_buf->ptr || !p_actual_size)
    {
        return JPEGERR_EFAILED;
    }
    *p_actual_size = p_buf->offset;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_set_actual_size
 * Description: This sets the actual size of the valid data in the buffer.
 * Input parameters:
 *   buffer        - The Jpeg buffer object.
 *   actual_size   - The actual size.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_set_actual_size(jpeg_buffer_t buffer, uint32_t actual_size)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    if (!p_buf || !p_buf->is_inited || (actual_size > p_buf->size))
    {
        return JPEGERR_EFAILED;
    }
    p_buf->offset = actual_size;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_set_start_offset
 * Description: This sets the start offset of the valid data in the buffer.
 * Input parameters:
 *   buffer        - The Jpeg buffer object.
 *   offset   - The start offset.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_set_start_offset(jpeg_buffer_t buffer, uint32_t offset)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    if (!p_buf || !p_buf->is_inited || (offset > p_buf->size))
    {
        return JPEGERR_EFAILED;
    }
    p_buf->start_offset = offset;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_set_phy_offset
 * Description: This sets the physical addr offset of the valid data in the buffer.
 * Input parameters:
 *   buffer        - The Jpeg buffer object.
 *   offset   - The phy addr offset.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_set_phy_offset(jpeg_buffer_t buffer, uint32_t offset)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    if (!p_buf || !p_buf->is_inited)
    {
        return JPEGERR_EFAILED;
    }
    p_buf->phy_offset = offset;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_use_external_buffer
 * Description: The second way to set up the Jpeg Buffer object to oversee
 *              a valid buffer is to supply it with an externally allocated buffer
 *              through this method. There will no attempt to free this buffer
 *              during the destruction of the object. This call will fail if
 *              jpeg_buffer_allocate has been called successfully prior to this.
 * Input parameters:
 *   buffer    - The Jpeg buffer object.
 *   ptr       - The pointer to the externally allocated buffer.
 *   size      - The size of the externally allocated buffer.
 *   pmem_fd   - The pmem driver file descriptor if the buffer is physically mmap-ed.
 *               Set to zero if not applicable.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_use_external_buffer(jpeg_buffer_t buffer,
                                    uint8_t *ptr,
                                    uint32_t size,
                                    pmem_fd_t pmem_fd)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    if (!p_buf || !p_buf->is_inited || p_buf->is_internal || !size || !ptr)
    {
        return JPEGERR_EFAILED;
    }
    // Find the physical address if pmem_fd is valid
    if (JPEG_SUCCEEDED(os_pmem_get_phy_addr(pmem_fd, &(p_buf->phy_addr))))
    {
        p_buf->pmem_fd = pmem_fd;
    }

    p_buf->ptr  = ptr;
    p_buf->size = size;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_attach_existing
 * Description: The third way to set up the Jpeg Buffer object to oversee
 *              a valid buffer is to attach it to an existing Jpeg buffer with
 *              an optional offset to it. This call will fail if
 *              jpeg_buffer_allocate has been called successfully prior to this.
 * Input parameters:
 *   buffer    - The Jpeg buffer object.
 *   e_buffer  - The existing buffer.
 *   offset    - The desired pointer offset to the existing buffer.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_attach_existing(jpeg_buffer_t buffer,
                                jpeg_buffer_t e_buffer,
                                uint32_t      offset)
{
    jpeg_buf_t *p_buf   = (jpeg_buf_t *)buffer;
    jpeg_buf_t *p_e_buf = (jpeg_buf_t *)e_buffer;

    if (!p_buf || !p_buf->is_inited || p_buf->is_internal ||
        !p_e_buf || !p_e_buf->is_inited || !p_e_buf->ptr ||
        p_e_buf->size <= offset)
    {
        return JPEGERR_EFAILED;
    }
    p_buf->is_internal = false;
    p_buf->phy_addr    = p_e_buf->phy_addr + offset;
    p_buf->ptr         = p_e_buf->ptr + offset;
    p_buf->size        = p_e_buf->size - offset;
    p_buf->pmem_fd     = p_e_buf->pmem_fd;
    p_buf->offset      = 0;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpeg_buffer_reset
 * Description: This routine undoes any effect done by jpeg_buffer_allocate or
 *              jpeg_buffer_use_external_buffer. In other words, it deallocates
 *              internally allocated buffer or deassociate itself for the
 *              externally allocated buffer.
 * Input parameters:
 *   buffer    - The Jpeg buffer object.
 * Return values:
 *   JPEGERR_SUCCESS
 *   JPEGERR_EFAILED
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_reset(jpeg_buffer_t buffer)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    if (!p_buf || !p_buf->is_inited)
    {
        return JPEGERR_EFAILED;
    }
    if (p_buf->is_internal && p_buf->ptr)
    {
        if (p_buf->pmem_fd != OS_PMEM_FD_INIT_VALUE)
        {
            (void)os_pmem_free(p_buf->pmem_fd, p_buf->size, p_buf->ptr);
            (void)os_pmem_fd_close(&p_buf->pmem_fd, 
                                   p_buf->ion_fd, &p_buf->info_fd);
            p_buf->pmem_fd = OS_PMEM_FD_INIT_VALUE;
        }
        else
        {
            JPEG_FREE(p_buf->ptr);
        }
    }
    p_buf->is_internal = false;
    p_buf->ptr         = NULL;
    p_buf->phy_addr    = NULL;
    p_buf->size        = 0;
    p_buf->offset      = 0;
    return JPEGERR_SUCCESS;
}

/**
 * Private routines for handling synchronization for buffer to
 * be shared by multiple working threads */

/******************************************************************************
 * Function: jpeg_buffer_wait_until_empty
 * Description: This routine blocks until the buffer is marked as empty by
 *              using jpeg_buffer_mark_empty by another thread.
 *****************************************************************************/
void jpeg_buffer_wait_until_empty(jpeg_buf_t *p_buf)
{
    os_mutex_lock(&(p_buf->mutex));
    while (!p_buf->is_empty)
    {
        os_cond_wait(&(p_buf->cond), &(p_buf->mutex));
    }
    os_mutex_unlock(&(p_buf->mutex));
}

/******************************************************************************
 * Function: jpeg_buffer_wait_until_filled
 * Description: This routine blocks until the buffer is marked as filled by
 *              using jpeg_buffer_mark_filled by another thread.
 *****************************************************************************/
void jpeg_buffer_wait_until_filled(jpeg_buf_t *p_buf)
{
    os_mutex_lock(&(p_buf->mutex));
    while (p_buf->is_empty)
    {
        os_cond_wait(&(p_buf->cond), &(p_buf->mutex));
    }
    os_mutex_unlock(&(p_buf->mutex));
}
/******************************************************************************
 * Function: jpeg_buffer_mark_empty
 * Description: This routine unblocks any threads that is blocked on the buffer
 *              waiting for it to be marked as empty.
 *****************************************************************************/
void jpeg_buffer_mark_empty(jpeg_buf_t *p_buf)
{
    os_mutex_lock(&(p_buf->mutex));
    p_buf->is_empty = true;
    p_buf->is_busy = false;
    os_cond_signal(&(p_buf->cond));
    os_mutex_unlock(&(p_buf->mutex));
}

/******************************************************************************
 * Function: jpeg_buffer_mark_filled
 * Description: This routine marks the buffer as filled.
 *****************************************************************************/
void jpeg_buffer_mark_filled(jpeg_buf_t *p_buf)
{
    os_mutex_lock(&(p_buf->mutex));
    p_buf->is_empty = false;
    p_buf->is_busy = false;
    os_cond_signal(&(p_buf->cond));
    os_mutex_unlock(&(p_buf->mutex));
}

/******************************************************************************
 * Function: jpeg_buffer_mark_busy
 * Description: This routine marks the buffer as busy being filled or emptied.
 *****************************************************************************/
void jpeg_buffer_mark_busy(jpeg_buf_t *p_buf)
{
    os_mutex_lock(&(p_buf->mutex));
    p_buf->is_busy = true;
    os_mutex_unlock(&(p_buf->mutex));
}

/******************************************************************************
 * Function: jpeg_buffer_destroy
 * Description: This cleans up internal variables.
 * Input parameters:
 *   p_buffer  - The pointer to the Jpeg buffer object to be destroyed.
 * Return values: None
 * Notes: none
 *****************************************************************************/
void jpeg_buffer_destroy(jpeg_buffer_t *p_buffer)
{

    if (p_buffer)
    {

        jpeg_buf_t *p_buf = (jpeg_buf_t *)(*p_buffer);
        if (p_buf)
        {

            (void)jpeg_buffer_reset((jpeg_buffer_t)p_buf);
            os_mutex_destroy(&(p_buf->mutex));
            os_cond_destroy(&(p_buf->cond));
            JPEG_FREE(p_buf);
        }
        *p_buffer = NULL;
    }
}

/******************************************************************************
 * Function: jpeg_buffer_get_phys_addr
 * Description: This returns the physical address of the PMEM buffer.
 * Input parameters:
 *   buffer        - The Jpeg buffer object.
 *   pp_phy_addr   - The pointer to the buffer physical address to be returned.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_get_phys_addr(jpeg_buffer_t buffer, uint8_t **pp_phy_addr)
{
    jpeg_buf_t *p_buf = (jpeg_buf_t *)buffer;
    int rc;
    if (!p_buf || !p_buf->ptr || !pp_phy_addr || !p_buf->is_inited)
    {
        return JPEGERR_EFAILED;
    }


    rc = os_pmem_get_phy_addr(p_buf->pmem_fd, pp_phy_addr);
    return rc;
}
