/**
 *Copyright (C) 2011 Qualcomm Technologies, Inc.
 *All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
**/
/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
03/10/11   mingy   Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "exif_private.h"
#include "mpo_private.h"
#include "jpeglog.h"

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
//int allocate_and_copy(void **dest, void *src, uint32_t size);
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
                             uint32_t             *p_len_required)
{
    uint32_t i, j = 0;
    uint32_t len_required = 0;

    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)attribute_obj;

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
                          exif_tag_entry_t    *p_entry)
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
    pp_tag_entry = (exif_tag_entry_ex_t **)p_attribute + tag_offset;

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

        for (i = 0; i < ATTRIBUTE_TAG_MAX; i++)
        {
            exif_destroy_tag_entry(pp_entries[i]);
        }
        JPEG_FREE(*p_mpo_attribute_obj);
        *p_mpo_attribute_obj = NULL;
    }
}
