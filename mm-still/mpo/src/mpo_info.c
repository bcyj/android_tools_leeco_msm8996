/*========================================================================


*//** @file mpo_info.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
01/26/11   staceyw Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "exif_private.h"
#include "mpo.h"
#include "mpo_private.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_common.h"
#include <stdlib.h>

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
int mpo_allocate_and_copy(void **dest, void *src, uint32_t size);
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/******************************************************************************
 * Function: mpo_index_init
 * Description: Initializes the MPO Index object. Dynamic allocations take
 *              place during the call. One should always call mpo_index_destroy
 *              to clean up the MPO Index object after use.
 * Input parameters:
 *   p_mpo_index_obj - The pointer to the MPO Index object to be initialized.
 *
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_index_init(mpo_index_obj_t *p_mpo_index_obj)
{
    mpo_index_ifd_t *p_index;

    if (!p_mpo_index_obj)
        return JPEGERR_ENULLPTR;

    // Allocate heap memory to hold the exif_info_t structure
    p_index = (mpo_index_ifd_t *)JPEG_MALLOC(sizeof(mpo_index_ifd_t));
    if (!p_index)
        return JPEGERR_EMALLOC;

    // Zero the structure
    STD_MEMSET(p_index, 0, sizeof(mpo_index_ifd_t));

    // Output it to the output container
    *p_mpo_index_obj = (mpo_index_obj_t)p_index;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpo_attribute_init
 * Description: Initializes the MPO Attribute object. Dynamic allocations take
 *              place during the call. One should always call mpo_attribute_destroy
 *              to clean up the MPO Attribute object after use.
 * Input parameters:
 *   p_mpo_attribute_obj - The pointer to the MPO Attribute object to be initialized.
 *
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_attribute_init(mpo_attribute_obj_t *p_mpo_attribute_obj)
{
    mpo_attribute_ifd_t *p_attribute;

    if (!p_mpo_attribute_obj)
        return JPEGERR_ENULLPTR;

    // Allocate heap memory to hold the exif_info_t structure
    p_attribute = (mpo_attribute_ifd_t *)JPEG_MALLOC(sizeof(mpo_attribute_ifd_t));
    if (!p_attribute)
        return JPEGERR_EMALLOC;

    // Zero the structure
    STD_MEMSET(p_attribute, 0, sizeof(mpo_attribute_ifd_t));

    // Output it to the output container
    *p_mpo_attribute_obj = (mpo_attribute_obj_t)p_attribute;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpo_list_index_tagid
 * Description: List all the tags that are present in the MPO index object.
 * Input parameters:
 *   index_obj      - The MPO Info object where the MPO tags should be obtained.
 *   p_tag_ids      - An array to hold the MPO Index Tag IDs to be retrieved.
 *                    It should be allocated by the caller.
 *   len            - The length of the array (the number of elements)
 *   p_len_required - Pointer to the holder to get back the required number of
 *                    elements enough to hold all tag ids present. It can be set
 *                    to NULL if it is not needed.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_list_index_tagid(mpo_index_obj_t   index_obj,
                         exif_tag_id_t    *p_tag_ids,
                         uint32_t          len,
                         uint32_t         *p_len_required)
{
    uint32_t i, j = 0;
    uint32_t len_required = 0;
    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)index_obj;

    if (len && !p_tag_ids)
        return JPEGERR_EBADPARM;

    // Make sure obj is initialized properly
    if (pp_entries)
    {
        // Visit each tag entry and retrive the ones that are present
        for (i = 0; i < INDEX_TAG_MAX; i++)
        {
            if (pp_entries[i])
            {
                len_required++;
                if (j < len)
                {
                    p_tag_ids[j] = pp_entries[i]->tag_id;
                    j++;
                }
            }
        }
    }
    // Output the total number of tags present
    if (p_len_required)
    {
        *p_len_required = len_required;
    }
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpo_list_attribute_tagid
 * Description: List all the tags that are present in the MPO attribute object.
 * Input parameters:
 *   attribute_obj  - The MPO Info object where the MPO tags should be obtained.
 *   p_tag_ids      - An array to hold the MPO Attribute Tag IDs to be retrieved.
 *                    It should be allocated by the caller.
 *   len            - The length of the array (the number of elements)
 *   p_len_required - Pointer to the holder to get back the required number of
 *                    elements enough to hold all tag ids present. It can be set
 *                    to NULL if it is not needed.
 *   image_id       - The id of the image for which the tag ids are being listed.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_list_attribute_tagid(mpo_attribute_obj_t   attribute_obj,
                             exif_tag_id_t        *p_tag_ids,
                             uint32_t              len,
                             uint32_t             *p_len_required,
                             uint32_t              image_id)
{
    uint32_t i, j = 0;
    uint32_t len_required = 0;

    mpo_attribute_ifd_t *p_mpo_attribute = (mpo_attribute_ifd_t *)attribute_obj;

    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)&p_mpo_attribute[0][image_id];

    if (len && !p_tag_ids)
        return JPEGERR_EBADPARM;

    // Make sure obj is initialized properly
    if (pp_entries)
    {
        // Visit each tag entry and retrive the ones that are present
        for (i = 0; i < ATTRIBUTE_TAG_MAX; i++)
        {
            if (pp_entries[i])
            {
                len_required++;
                if (j < len)
                {
                    p_tag_ids[j] = pp_entries[i]->tag_id;
                    j++;
                }
            }
        }
    }
    // Output the total number of tags present
    if (p_len_required)
    {
        *p_len_required = len_required;
    }
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpo_get_index_tag
 * Description: Queries the stored MPO tags in the MPO Index object. Typical
 *              use is to obtain an MPO Index object from the MPO Decoder
 *              object which parsed an MPO header then make multiple calls
 *              to this function to individually obtain each tag present in
 *              the header. Note that all memory allocations made to hold the
 *              exif tags are owned by the MPO Index object and they will be
 *              released when mpo_info_destroy is called on the MPO Index object.
 *              Any app needing to preserve those data for later use should
 *              make copies of the tag contents.
 *              If the requested TAG is not present from the file, an
 *              JPEGERR_TAGABSENT will be returned.
 * Input parameters:
 *   index_obj - The MPO Index object where the MPO tags should be obtained.
 *   tag_id    - The MPO Tag ID of the tag to be queried for.
 *   p_entry   - The pointer to the tag entry, filled by the function upon
 *               successful operation. Caller should not pre-allocate memory
 *               to hold the tag contents. Contents in the structure pointed
 *               by p_entry will be overwritten.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 *     JPEGERR_TAGABSENT
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_get_index_tag(mpo_index_obj_t    index_obj,
                      exif_tag_id_t      tag_id,
                      exif_tag_entry_t  *p_entry)
{
    uint16_t tag_offset;
    exif_tag_entry_ex_t **pp_tag_entry;
    mpo_index_ifd_t *p_index = (mpo_index_ifd_t *)index_obj;

    // Make sure obj is initialized properly
    if (!p_index)
        return JPEGERR_EUNINITIALIZED;

    // Validate input arguments
    // Validate p_entry
    if (!p_entry)
        return JPEGERR_EBADPARM;

    tag_offset = (tag_id >> 16) & 0xFFFF;

    // Validate tag_id
    if (tag_offset >= INDEX_TAG_MAX)
    {
        JPEG_DBG_MED("mpo_get_index_tag: invalid index tag id: 0x%x\n", tag_id);
        return JPEGERR_EBADPARM;
    }

    // Determine if the entry is present or not
    pp_tag_entry = (exif_tag_entry_ex_t **)p_index + tag_offset;

    if (!(*pp_tag_entry))
    {
        JPEG_DBG_MED("exif_get_tag: requested tag (0x%x) is not present\n", tag_id);
        return JPEGERR_TAGABSENT;
    }

    // Make a copy of the entry to the output container
    *p_entry = (*pp_tag_entry)->entry;
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpo_get_attribute_tag
 * Description: Queries the stored MPO tags in the MPO Attribute object. Typical
 *              use is to obtain an MPO Attribute object from the MPO Decoder
 *              object which parsed an MPO header then make multiple calls
 *              to this function to individually obtain each tag present in
 *              the header. Note that all memory allocations made to hold the
 *              exif tags are owned by the MPO Attribute object and they will be
 *              released when mpo_info_destroy is called on the MPO Attribute object.
 *              Any app needing to preserve those data for later use should
 *              make copies of the tag contents.
 *              If the requested TAG is not present from the file, an
 *              JPEGERR_TAGABSENT will be returned.
 * Input parameters:
 *   index_obj - The MPO Index object where the MPO tags should be obtained.
 *   tag_id    - The MPO Tag ID of the tag to be queried for.
 *   p_entry   - The pointer to the tag entry, filled by the function upon
 *               successful operation. Caller should not pre-allocate memory
 *               to hold the tag contents. Contents in the structure pointed
 *               by p_entry will be overwritten.
 *   image_id  - The id of the frame from which the tag_id will be get.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 *     JPEGERR_TAGABSENT
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_get_attribute_tag(mpo_attribute_obj_t  attribute_obj,
                          exif_tag_id_t        tag_id,
                          exif_tag_entry_t    *p_entry,
                          uint32_t             image_id)
{
    uint16_t tag_offset;
    exif_tag_entry_ex_t **pp_tag_entry;
    mpo_attribute_ifd_t *p_attribute = (mpo_attribute_ifd_t *)attribute_obj;

    // Make sure obj is initialized properly
    if (!p_attribute)
        return JPEGERR_EUNINITIALIZED;

    // Validate input arguments
    // Validate p_entry
    if (!p_entry)
        return JPEGERR_EBADPARM;

    tag_offset = (tag_id >> 16) & 0xFFFF;

    // Validate tag_id
    if (tag_offset >= ATTRIBUTE_TAG_MAX)
    {
        JPEG_DBG_MED("mpo_get_attribute_tag: invalid attribute tag id: 0x%x\n", tag_id);
        return JPEGERR_EBADPARM;
    }

    // Determine if the entry is present or not
    pp_tag_entry = (exif_tag_entry_ex_t **)&p_attribute[0][image_id] + tag_offset;

    if (!(*pp_tag_entry))
    {
        JPEG_DBG_MED("exif_get_tag: requested tag (0x%x) is not present\n", tag_id);
        return JPEGERR_TAGABSENT;
    }

    // Make a copy of the entry to the output container
    *p_entry = (*pp_tag_entry)->entry;
    return JPEGERR_SUCCESS;
}


/******************************************************************************
 * Function: mpo_index_destroy
 * Description: Releases all allocated memory made over the lifetime of the
 *              MPO Index object. One should always call this function to clean
 *              up an 'mpo_index_init'-ed MPO Index object.
 * Input parameters:
 *   p_obj - The pointer to the Exif Info object to be destroyed.
 * Return values: None
 * Notes: none
 *****************************************************************************/
void mpo_index_destroy(mpo_index_obj_t *p_mpo_index_obj)
{
    if (p_mpo_index_obj)
    {
        int i;
        exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(*p_mpo_index_obj);

        if (!pp_entries)
            return;

        for (i = 0; i < INDEX_TAG_MAX; i++)
        {
            exif_destroy_tag_entry(pp_entries[i]);
        }
        JPEG_FREE(*p_mpo_index_obj);
        *p_mpo_index_obj = NULL;
    }
}

/******************************************************************************
 * Function: mpo_attribute_destroy
 * Description: Releases all allocated memory made over the lifetime of the
 *              MPO Attribute object. One should always call this function to clean
 *              up an 'mpo_attribute_init'-ed MPO Attribute object.
 * Input parameters:
 *   p_obj - The pointer to the Exif Info object to be destroyed.
 * Return values: None
 * Notes: none
 *****************************************************************************/
void mpo_attribute_destroy(mpo_attribute_obj_t *p_mpo_attribute_obj)
{
    if (p_mpo_attribute_obj)
    {
        int i;
        exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(*p_mpo_attribute_obj);

        if (!pp_entries)
            return;

        for (i = 0; i < MPO_MAX_FRAME * ATTRIBUTE_TAG_MAX; i++)
        {
            exif_destroy_tag_entry(pp_entries[i]);
        }
        JPEG_FREE(*p_mpo_attribute_obj);
        *p_mpo_attribute_obj = NULL;
    }
}

/******************************************************************************
 * Function: mpo_info_init
 * Description: Initializes the Mpo Info object. Dynamic allocations take
 *              place during the call. One should always call mpo_info_destroy to
 *              clean up the Mpo Info object after use.
 * Input parameters:
 *   p_mpo_info - The pointer to the Mpo Info object to be initialized.
 *
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_info_init(mpo_info_obj_t *p_obj)
{
    mpo_info_t *p_info;

    if (!p_obj)
        return JPEGERR_ENULLPTR;

    // Allocate heap memory to hold the mpo_info_t structure
    p_info = (mpo_info_t *)JPEG_MALLOC(sizeof(mpo_info_t));
    if (!p_info)
        return JPEGERR_EMALLOC;

    // Zero the structure
    STD_MEMSET(p_info, 0, sizeof(mpo_info_t));

    // Output it to the output container
    *p_obj = (mpo_info_obj_t)p_info;

    return JPEGERR_SUCCESS;
}
/******************************************************************************
 * Function: mpo_set_tag
 * Description: Inserts or modifies an mpo tag to the mpo Info object. Typical
 *              use is to call this function multiple times - to insert all the
 *              desired mpo Tags individually to the mpo Info object and
 *              then pass the info object to the Jpeg Encoder object so
 *              the inserted tags would be emitted as tags in the mpo header.
 * Input parameters:
 *   obj       - The mpo Info object where the tag would be inserted to or
 *               modified from.
 *   tag_id    - The mpo Tag ID of the tag to be inserted/modified.
 *   p_entry   - The pointer to the tag entry structure which contains the
 *               details of tag. The pointer can be set to NULL to un-do
 *               previous insertion for a certain tag.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_set_tag(mpo_info_obj_t     obj,
                exif_tag_id_t      tag_id,
                exif_tag_entry_t  *p_entry,
				uint32_t           src_id)
{
    int rc = JPEGERR_SUCCESS;
    exif_tag_entry_ex_t *p_new_entry;
    exif_tag_entry_ex_t **pp_tag_entry;
    uint16_t tag_offset;
    mpo_info_t *p_info = (mpo_info_t *)obj;

    // Make sure obj is initialized properly
    if (!p_info)
        return JPEGERR_EUNINITIALIZED;

    tag_offset = (tag_id >> 16) & 0xFFFF;

    // Validate tag_id
    if (tag_offset >= EXIF_TAG_MAX_OFFSET)
    {
        JPEG_DBG_MED("mpo_set_tag: invalid tag id: 0x%x\n", tag_id);
        return JPEGERR_EBADPARM;
    }

    pp_tag_entry = (exif_tag_entry_ex_t **)p_info + tag_offset + src_id * MPO_ATTRIBUTE_IFD_SIZE;

    // Destroy entry previously being pointed at
    if (*pp_tag_entry)
        exif_destroy_tag_entry(*pp_tag_entry);

    // if p_entry is NULL, return
    if (!p_entry)
        return JPEGERR_SUCCESS;

    // Validate input arguments if p_entry is non NULL
    if (!(p_entry->count) || (uint8_t)p_entry->type > (uint8_t)EXIF_SRATIONAL)
        return JPEGERR_EBADPARM;

    // Create a new entry holder
    p_new_entry = exif_create_tag_entry();
    if (!p_new_entry)
        return JPEGERR_EMALLOC;

    // Make a copy of the entry
    p_new_entry->entry = *p_entry;
    p_new_entry->tag_id = tag_id;

    // Make a copy to the content pointed by the entry if necessary
    if (p_entry->copy)
    {
        if (p_entry->type == EXIF_ASCII)
        {
            rc = mpo_allocate_and_copy((void **)&(p_new_entry->entry.data._ascii),
                                   (void *)p_entry->data._ascii,
                                   p_entry->count * sizeof(char));
        }
        else if (p_entry->type == EXIF_UNDEFINED)
        {
            rc = mpo_allocate_and_copy((void **)&(p_new_entry->entry.data._undefined),
                                   (void *)p_entry->data._undefined,
                                   p_entry->count * sizeof(uint8_t));
        }
        else if (p_entry->count > 1)
        {
            switch (p_entry->type)
            {
            case EXIF_BYTE:
                rc = mpo_allocate_and_copy((void **)&(p_new_entry->entry.data._bytes),
                                       (void *)p_entry->data._bytes,
                                       p_entry->count * sizeof(uint8_t));
                break;
            case EXIF_SHORT:
                rc = mpo_allocate_and_copy((void **)&(p_new_entry->entry.data._shorts),
                                       (void *)p_entry->data._shorts,
                                       p_entry->count * sizeof(uint16_t));
                break;
            case EXIF_LONG:
                rc = mpo_allocate_and_copy((void **)&(p_new_entry->entry.data._longs),
                                       (void *)p_entry->data._longs,
                                       p_entry->count * sizeof(uint32_t));
                break;
            case EXIF_RATIONAL:
                rc = mpo_allocate_and_copy((void **)&(p_new_entry->entry.data._rats),
                                       (void *)p_entry->data._rats,
                                       p_entry->count * sizeof(rat_t));
                break;
            case EXIF_SLONG:
                rc = mpo_allocate_and_copy((void **)&(p_new_entry->entry.data._longs),
                                       (void *)p_entry->data._longs,
                                       p_entry->count * sizeof(uint32_t));
                break;
            case EXIF_SRATIONAL:
                rc = mpo_allocate_and_copy((void **)&(p_new_entry->entry.data._srats),
                                       (void *)p_entry->data._srats,
                                       p_entry->count * sizeof(srat_t));
                break;
            default:
                break;
            }
        }
    }
    if (JPEG_FAILED(rc))
    {
        exif_destroy_tag_entry(p_new_entry);
        return rc;
    }

    // Update the entry pointer
    *pp_tag_entry = p_new_entry;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpo_delete_tag
 * Description: Delete the stored Mpo tags in the Mpo Info object. Typical
 *              use is to obtain an Mpo Info object from the Jpeg Decoder
 *              object which parsed an Mpo header then make multiple calls
 *              to this function to individually obtain each tag present in
 *              the header. Note that all memory allocations made to hold the
 *              exif tags are owned by the Mpo Info object and they will be
 *              released when exif_destroy is called on the Mpo Info object.
 *              Any app needing to preserve those data for later use should
 *              make copies of the tag contents.
 *              If the requested TAG is not present from the file, an
 *              JPEGERR_TAGABSENT will be returned.
 * Input parameters:
 *   obj       - The Mpo Info object where the Mpo tags should be deleted.
 *   tag_id    - The Mpo Tag ID of the tag to be deleted.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 *     JPEGERR_TAGABSENT
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_delete_attribute_tag(
  mpo_info_obj_t    obj,
                   exif_tag_id_t     tag_id,
				   uint32_t          src_id)
{
    exif_tag_entry_ex_t **pp_tag_entry;
    uint16_t tag_offset;
    exif_info_t *p_info = (exif_info_t *)obj;

    // Make sure obj is initialized properly
    if (!p_info)
        return JPEGERR_EUNINITIALIZED;

    tag_offset = (tag_id >> 16) & 0xFFFF;

    // Validate tag_id
    if (tag_offset >= EXIF_TAG_MAX_OFFSET)
    {
        JPEG_DBG_MED("exif_set_tag: invalid tag id: 0x%x\n", tag_id);
        return JPEGERR_EBADPARM;
    }

    pp_tag_entry = (exif_tag_entry_ex_t **)p_info + tag_offset + src_id * MPO_ATTRIBUTE_IFD_SIZE;

    // Destroy entry previously being pointed at
    if (!(*pp_tag_entry))
        return JPEGERR_TAGABSENT;

    // Destroy entry
    exif_destroy_tag_entry(*pp_tag_entry);
    // Set tag entry to NULL
    *pp_tag_entry = NULL;

    return JPEGERR_SUCCESS;
}


/******************************************************************************
 * Function: mpo_info_destroy
 * Description: Releases all allocated memory made over the lifetime of the
 *              Mpo Info object. One should always call this function to clean
 *              up an 'exif_init'-ed Mpo Info object.
 * Input parameters:
 *   p_obj - The pointer to the Mpo Info object to be destroyed.
 * Return values: None
 * Notes: none
 *****************************************************************************/
void mpo_info_destroy(mpo_info_obj_t *p_obj)
{
    if (p_obj)
    {
        exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(*p_obj);

        if (!pp_entries)
            return;

        JPEG_FREE(*p_obj);
        *p_obj = NULL;
    }
}

int mpo_allocate_and_copy(void **dest, void *src, uint32_t size)
{
    if (!src || !dest)
        return JPEGERR_EBADPARM;

    *dest = (void *)JPEG_MALLOC(size);
    if (!(*dest))
        return JPEGERR_EMALLOC;

    (void)STD_MEMMOVE(*dest, src, size);
    return JPEGERR_SUCCESS;
}
