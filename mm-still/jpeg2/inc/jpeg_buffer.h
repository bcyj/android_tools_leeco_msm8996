/*========================================================================

*//** @file jpeg_buffer.h  

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/03/09   vma     Switched to use the os abstraction layer (os_pmem*)
10/15/08   vma     Initial version.
*//*====================================================================== */

#ifndef __JPEG_BUFFER_H
#define __JPEG_BUFFER_H

#include "jpeg_common.h"
#include "os_pmem.h"

/**
 * jpeg_buffer_t encapsulates all attributes that is related to
 * a buffer to ensure consistent use. */
struct jpeg_buf_t;
typedef struct jpeg_buf_t * jpeg_buffer_t;

/******************************************************************************
 * Function: jpeg_buffer_init                                                      
 * Description: Initializes the Jpeg buffer object. jpeg_buffer_destroy should
 *              be called clean up internal variables properly.
 * Input parameters:
 *   p_buffer  - The pointer to the Jpeg buffer object to be initialized. 
 * Return values: 
 *   JPEGERR_SUCCESS
 *   JPEGERR_ENULLPTR
 *   JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_init(jpeg_buffer_t *p_buffer);

/******************************************************************************
 * Function: jpeg_buffer_allocate
 * Description: There are three ways to set up the Jpeg Buffer object to oversee
 *              a valid buffer. One is to instruct it to internally allocate
 *              a buffer. Another is to supply an externally allocated buffer
 *              to the Jpeg Buffer object. Last is to attach it to an existing
 *              Jpeg buffer with an offset to it. One should call 
 *              jpeg_buffer_allocate to instruct the Jpeg Buffer to internally 
 *              allocate for a valid buffer.
 * Input parameters:
 *   buffer    - The Jpeg buffer object.
 *   size      - The size of the buffer to be allocated
 *   use_pmem  - A flag to indicate whether the allocation should be made on
 *               a mmaped physical memory (for shared access by accelerating 
 *               hardware.)
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 *    JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_allocate(jpeg_buffer_t buffer, uint32_t size, uint8_t use_pmem);

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
                                    pmem_fd_t pmem_fd);

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
                                uint32_t      offset);

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
int jpeg_buffer_reset(jpeg_buffer_t buffer);

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
int jpeg_buffer_get_addr(jpeg_buffer_t buffer, uint8_t **pp_addr);

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
pmem_fd_t jpeg_buffer_get_pmem_fd(jpeg_buffer_t buffer);

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
int jpeg_buffer_get_max_size(jpeg_buffer_t buffer, uint32_t *p_max_size);

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
int jpeg_buffer_get_actual_size(jpeg_buffer_t buffer, uint32_t *p_actual_size);

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
int jpeg_buffer_set_actual_size(jpeg_buffer_t buffer, uint32_t actual_size);

/******************************************************************************
 * Function: jpeg_buffer_set_start_offset
 * Description: This sets the start offset of the valid data in the buffer.
 * Input parameters:
 *   buffer        - The Jpeg buffer object.
 *   offset        - The start offset.
 * Return values:
 *    JPEGERR_SUCCESS
 *    JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpeg_buffer_set_start_offset(jpeg_buffer_t buffer, uint32_t offset);

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
int jpeg_buffer_set_phy_offset(jpeg_buffer_t buffer, uint32_t offset);

/******************************************************************************
 * Function: jpeg_buffer_destroy
 * Description: This cleans up internal variables.
 * Input parameters:
 *   p_buffer  - The pointer to the Jpeg buffer object to be destroyed.
 * Return values: None
 * Notes: none
 *****************************************************************************/
void jpeg_buffer_destroy(jpeg_buffer_t *p_buffer);

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
int jpeg_buffer_get_phys_addr(jpeg_buffer_t buffer, uint8_t **pp_phy_addr);

#endif // #ifndef __JPEG_BUFFER_H

