/*========================================================================


*//** @file exif.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/30/09   vma     Added a function to convert 16-bit tag ID's to
                   the 32-bit proprietary ones.
05/06/09   vma     Fixed bug in allocate_and_copy().
07/14/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "exif.h"
#include "exif_private.h"
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
int allocate_and_copy(void **dest, void *src, uint32_t size);
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/* =======================================================================
**                          Function Definitions
** ======================================================================= */
/******************************************************************************
 * Function: exif_init
 * Description: Initializes the Exif Info object. Dynamic allocations take
 *              place during the call. One should always call exif_destroy to
 *              clean up the Exif Info object after use.
 * Input parameters:
 *   p_exif_info - The pointer to the Exif Info object to be initialized.
 *
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int exif_init(exif_info_obj_t *p_obj)
{
    exif_info_t *p_info;

    if (!p_obj)
        return JPEGERR_ENULLPTR;

    // Allocate heap memory to hold the exif_info_t structure
    p_info = (exif_info_t *)JPEG_MALLOC(sizeof(exif_info_t));
    if (!p_info)
        return JPEGERR_EMALLOC;

    // Zero the structure
    STD_MEMSET(p_info, 0, sizeof(exif_info_t));

    // Output it to the output container
    *p_obj = (exif_info_obj_t)p_info;

    // Add default tags
    exif_add_defaults(*p_obj);

    return JPEGERR_SUCCESS;
}
/******************************************************************************
 * Function: exif_set_tag
 * Description: Inserts or modifies an Exif tag to the Exif Info object. Typical
 *              use is to call this function multiple times - to insert all the
 *              desired Exif Tags individually to the Exif Info object and
 *              then pass the info object to the Jpeg Encoder object so
 *              the inserted tags would be emitted as tags in the Exif header.
 * Input parameters:
 *   obj       - The Exif Info object where the tag would be inserted to or
 *               modified from.
 *   tag_id    - The Exif Tag ID of the tag to be inserted/modified.
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
int exif_set_tag(exif_info_obj_t    obj,
                 exif_tag_id_t      tag_id,
                 exif_tag_entry_t  *p_entry)
{
    int rc = JPEGERR_SUCCESS;
    exif_tag_entry_ex_t *p_new_entry;
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

    pp_tag_entry = (exif_tag_entry_ex_t **)p_info + tag_offset;


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
            rc = allocate_and_copy((void **)&(p_new_entry->entry.data._ascii),
                                   (void *)p_entry->data._ascii,
                                   p_entry->count * sizeof(char));
        }
        else if (p_entry->type == EXIF_UNDEFINED)
        {
            rc = allocate_and_copy((void **)&(p_new_entry->entry.data._undefined),
                                   (void *)p_entry->data._undefined,
                                   p_entry->count * sizeof(uint8_t));
        }
        else if (p_entry->count > 1)
        {
            switch (p_entry->type)
            {
            case EXIF_BYTE:
                rc = allocate_and_copy((void **)&(p_new_entry->entry.data._bytes),
                                       (void *)p_entry->data._bytes,
                                       p_entry->count * sizeof(uint8_t));
                break;
            case EXIF_SHORT:
                rc = allocate_and_copy((void **)&(p_new_entry->entry.data._shorts),
                                       (void *)p_entry->data._shorts,
                                       p_entry->count * sizeof(uint16_t));
                break;
            case EXIF_LONG:
                rc = allocate_and_copy((void **)&(p_new_entry->entry.data._longs),
                                       (void *)p_entry->data._longs,
                                       p_entry->count * sizeof(uint32_t));
                break;
            case EXIF_RATIONAL:
                rc = allocate_and_copy((void **)&(p_new_entry->entry.data._rats),
                                       (void *)p_entry->data._rats,
                                       p_entry->count * sizeof(rat_t));
                break;
            case EXIF_SLONG:
                rc = allocate_and_copy((void **)&(p_new_entry->entry.data._longs),
                                       (void *)p_entry->data._longs,
                                       p_entry->count * sizeof(uint32_t));
                break;
            case EXIF_SRATIONAL:
                rc = allocate_and_copy((void **)&(p_new_entry->entry.data._srats),
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
 * Function: exif_get_tag
 * Description: Queries the stored Exif tags in the Exif Info object. Typical
 *              use is to obtain an Exif Info object from the Jpeg Decoder
 *              object which parsed an Exif header then make multiple calls
 *              to this function to individually obtain each tag present in
 *              the header. Note that all memory allocations made to hold the
 *              exif tags are owned by the Exif Info object and they will be
 *              released when exif_destroy is called on the Exif Info object.
 *              Any app needing to preserve those data for later use should
 *              make copies of the tag contents.
 *              If the requested TAG is not present from the file, an
 *              JPEGERR_TAGABSENT will be returned.
 * Input parameters:
 *   obj       - The Exif Info object where the Exif tags should be obtained.
 *   tag_id    - The Exif Tag ID of the tag to be queried for.
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
int exif_get_tag(exif_info_obj_t    obj,
                 exif_tag_id_t      tag_id,
                 exif_tag_entry_t  *p_entry)
{
    uint16_t tag_offset;
    exif_tag_entry_ex_t **pp_tag_entry;
    exif_info_t *p_info = (exif_info_t *)obj;

    // Make sure obj is initialized properly
    if (!p_info)
        return JPEGERR_EUNINITIALIZED;

    // Validate input arguments
    // Validate p_entry
    if (!p_entry)
        return JPEGERR_EBADPARM;

    tag_offset = (tag_id >> 16) & 0xFFFF;

    // Validate tag_id
    if (tag_offset >= EXIF_TAG_MAX_OFFSET)
    {
        JPEG_DBG_MED("exif_get_tag: invalid tag id: 0x%x\n", tag_id);
        return JPEGERR_EBADPARM;
    }

    // Determine if the entry is present or not
    pp_tag_entry = (exif_tag_entry_ex_t **)p_info + tag_offset;

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
 * Function: exif_delete_tag
 * Description: Delete the stored Exif tags in the Exif Info object. Typical
 *              use is to obtain an Exif Info object from the Jpeg Decoder
 *              object which parsed an Exif header then make multiple calls
 *              to this function to individually obtain each tag present in
 *              the header. Note that all memory allocations made to hold the
 *              exif tags are owned by the Exif Info object and they will be
 *              released when exif_destroy is called on the Exif Info object.
 *              Any app needing to preserve those data for later use should
 *              make copies of the tag contents.
 *              If the requested TAG is not present from the file, an
 *              JPEGERR_TAGABSENT will be returned.
 * Input parameters:
 *   obj       - The Exif Info object where the Exif tags should be deleted.
 *   tag_id    - The Exif Tag ID of the tag to be deleted.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 *     JPEGERR_TAGABSENT
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int exif_delete_tag(exif_info_obj_t    obj,
                    exif_tag_id_t      tag_id)
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

    pp_tag_entry = (exif_tag_entry_ex_t **)p_info + tag_offset;

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
 * Function: exif_convert_tag_id_short_to_full
 * Description: The tag ID used in exif_set_tag and exif_get_tag are defined
 *              in exif.h as 32-bit ID's. The upper 16 bits are specially
 *              constructed to allow fast table lookup. The lower 16 bits are
 *              the same 16-bit tag ID defined by the EXIF specification.
 *              While one can use the defined constants directly in typical
 *              use where a known tag is to be retrieved or written, it is
 *              not possible to for example enumerate a list of tags in a
 *              for-loop given only the 16-bit tag IDs are known.
 *              For this reason, this function does the reverse lookup
 *              from the 16-bit IDs back to the 32-bit IDs.
 * Input parameters:
 *   short_tag - The 16-bit EXIF compliant ID.
 *   tag_id    - The 32-bit proprietary tag ID.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int exif_convert_tag_id_short_to_full(uint16_t short_tag, exif_tag_id_t *tag_id)
{
    if (!tag_id)
        return JPEGERR_ENULLPTR;

    switch (short_tag)
    {
    case _ID_GPS_VERSION_ID:
        *tag_id = EXIFTAGID_GPS_VERSION_ID;
        break;
    case _ID_GPS_LATITUDE_REF:
        *tag_id = EXIFTAGID_GPS_LATITUDE_REF;
        break;
    case _ID_GPS_LATITUDE:
        *tag_id = EXIFTAGID_GPS_LATITUDE;
        break;
    case _ID_GPS_LONGITUDE_REF:
        *tag_id = EXIFTAGID_GPS_LONGITUDE_REF;
        break;
    case _ID_GPS_LONGITUDE:
        *tag_id = EXIFTAGID_GPS_LONGITUDE;
        break;
    case _ID_GPS_ALTITUDE_REF:
        *tag_id = EXIFTAGID_GPS_ALTITUDE_REF;
        break;
    case _ID_GPS_ALTITUDE:
        *tag_id = EXIFTAGID_GPS_ALTITUDE;
        break;
    case _ID_GPS_TIMESTAMP:
        *tag_id = EXIFTAGID_GPS_TIMESTAMP;
        break;
    case _ID_GPS_SATELLITES:
        *tag_id = EXIFTAGID_GPS_SATELLITES;
        break;
    case _ID_GPS_STATUS:
        *tag_id = EXIFTAGID_GPS_STATUS;
        break;
    case _ID_GPS_MEASUREMODE:
        *tag_id = EXIFTAGID_GPS_MEASUREMODE;
        break;
    case _ID_GPS_DOP:
        *tag_id = EXIFTAGID_GPS_DOP;
        break;
    case _ID_GPS_SPEED_REF:
        *tag_id = EXIFTAGID_GPS_SPEED_REF;
        break;
    case _ID_GPS_SPEED:
        *tag_id = EXIFTAGID_GPS_SPEED;
        break;
    case _ID_GPS_TRACK_REF:
        *tag_id = EXIFTAGID_GPS_TRACK_REF;
        break;
    case _ID_GPS_TRACK:
        *tag_id = EXIFTAGID_GPS_TRACK;
        break;
    case _ID_GPS_IMGDIRECTION_REF:
        *tag_id = EXIFTAGID_GPS_IMGDIRECTION_REF;
        break;
    case _ID_GPS_IMGDIRECTION:
        *tag_id = EXIFTAGID_GPS_IMGDIRECTION;
        break;
    case _ID_GPS_MAPDATUM:
        *tag_id = EXIFTAGID_GPS_MAPDATUM;
        break;
    case _ID_GPS_DESTLATITUDE_REF:
        *tag_id = EXIFTAGID_GPS_DESTLATITUDE_REF;
        break;
    case _ID_GPS_DESTLATITUDE:
        *tag_id = EXIFTAGID_GPS_DESTLATITUDE;
        break;
    case _ID_GPS_DESTLONGITUDE_REF:
        *tag_id = EXIFTAGID_GPS_DESTLONGITUDE_REF;
        break;
    case _ID_GPS_DESTLONGITUDE:
        *tag_id = EXIFTAGID_GPS_DESTLONGITUDE;
        break;
    case _ID_GPS_DESTBEARING_REF:
        *tag_id = EXIFTAGID_GPS_DESTBEARING_REF;
        break;
    case _ID_GPS_DESTBEARING:
        *tag_id = EXIFTAGID_GPS_DESTBEARING;
        break;
    case _ID_GPS_DESTDISTANCE_REF:
        *tag_id = EXIFTAGID_GPS_DESTDISTANCE_REF;
        break;
    case _ID_GPS_DESTDISTANCE:
        *tag_id = EXIFTAGID_GPS_DESTDISTANCE;
        break;
    case _ID_GPS_PROCESSINGMETHOD:
        *tag_id = EXIFTAGID_GPS_PROCESSINGMETHOD;
        break;
    case _ID_GPS_AREAINFORMATION:
        *tag_id = EXIFTAGID_GPS_AREAINFORMATION;
        break;
    case _ID_GPS_DATESTAMP:
        *tag_id = EXIFTAGID_GPS_DATESTAMP;
        break;
    case _ID_GPS_DIFFERENTIAL:
        *tag_id = EXIFTAGID_GPS_DIFFERENTIAL;
        break;
    case _ID_IMAGE_WIDTH:
        *tag_id = EXIFTAGID_IMAGE_WIDTH;
        break;
    case _ID_IMAGE_LENGTH:
        *tag_id = EXIFTAGID_IMAGE_LENGTH;
        break;
    case _ID_BITS_PER_SAMPLE:
        *tag_id = EXIFTAGID_BITS_PER_SAMPLE;
        break;
    case _ID_COMPRESSION:
        *tag_id = EXIFTAGID_COMPRESSION;
        break;
    case _ID_PHOTOMETRIC_INTERPRETATION:
        *tag_id = EXIFTAGID_PHOTOMETRIC_INTERPRETATION;
        break;
    case _ID_THRESH_HOLDING:
        *tag_id = EXIFTAGID_THRESH_HOLDING;
        break;
    case _ID_CELL_WIDTH:
        *tag_id = EXIFTAGID_CELL_WIDTH;
        break;
    case _ID_CELL_HEIGHT:
        *tag_id = EXIFTAGID_CELL_HEIGHT;
        break;
    case _ID_FILL_ORDER:
         *tag_id = EXIFTAGID_FILL_ORDER;
         break;
    case _ID_DOCUMENT_NAME:
        *tag_id = EXIFTAGID_DOCUMENT_NAME;
        break;
    case _ID_IMAGE_DESCRIPTION:
        *tag_id = EXIFTAGID_IMAGE_DESCRIPTION;
        break;
    case _ID_MAKE:
        *tag_id = EXIFTAGID_MAKE;
        break;
    case _ID_MODEL:
        *tag_id = EXIFTAGID_MODEL;
        break;
    case _ID_STRIP_OFFSETS:
        *tag_id = EXIFTAGID_STRIP_OFFSETS;
        break;
    case _ID_ORIENTATION:
        *tag_id = EXIFTAGID_ORIENTATION;
        break;
    case _ID_SAMPLES_PER_PIXEL:
        *tag_id = EXIFTAGID_SAMPLES_PER_PIXEL;
        break;
    case _ID_ROWS_PER_STRIP:
        *tag_id = EXIFTAGID_ROWS_PER_STRIP;
        break;
    case _ID_STRIP_BYTE_COUNTS:
        *tag_id = EXIFTAGID_STRIP_BYTE_COUNTS;
        break;
    case _ID_MIN_SAMPLE_VALUE:
        *tag_id = EXIFTAGID_MIN_SAMPLE_VALUE;
        break;
    case _ID_MAX_SAMPLE_VALUE:
        *tag_id = EXIFTAGID_MAX_SAMPLE_VALUE;
        break;
    case _ID_X_RESOLUTION:
        *tag_id = EXIFTAGID_X_RESOLUTION;
        break;
    case _ID_Y_RESOLUTION:
        *tag_id = EXIFTAGID_Y_RESOLUTION;
        break;
    case _ID_PLANAR_CONFIGURATION:
        *tag_id = EXIFTAGID_PLANAR_CONFIGURATION;
        break;
    case _ID_PAGE_NAME:
        *tag_id = EXIFTAGID_PAGE_NAME;
        break;
    case _ID_X_POSITION:
        *tag_id = EXIFTAGID_X_POSITION;
        break;
    case _ID_Y_POSITION:
        *tag_id = EXIFTAGID_Y_POSITION;
        break;
    case _ID_FREE_OFFSET:
        *tag_id = EXIFTAGID_FREE_OFFSET;
        break;
    case _ID_FREE_BYTE_COUNTS:
        *tag_id = EXIFTAGID_FREE_BYTE_COUNTS;
        break;
    case _ID_GRAY_RESPONSE_UNIT:
        *tag_id = EXIFTAGID_GRAY_RESPONSE_UNIT;
        break;
    case _ID_GRAY_RESPONSE_CURVE:
        *tag_id = EXIFTAGID_GRAY_RESPONSE_CURVE;
        break;
    case _ID_T4_OPTION:
        *tag_id = EXIFTAGID_T4_OPTION;
        break;
    case _ID_T6_OPTION:
        *tag_id = EXIFTAGID_T6_OPTION;
        break;
    case _ID_RESOLUTION_UNIT:
        *tag_id = EXIFTAGID_RESOLUTION_UNIT;
        break;
    case _ID_PAGE_NUMBER:
        *tag_id = EXIFTAGID_PAGE_NUMBER;
        break;
    case _ID_TRANSFER_FUNCTION:
        *tag_id = EXIFTAGID_TRANSFER_FUNCTION;
        break;
    case _ID_SOFTWARE:
        *tag_id = EXIFTAGID_SOFTWARE;
        break;
    case _ID_DATE_TIME:
        *tag_id = EXIFTAGID_DATE_TIME;
        break;
    case _ID_ARTIST:
        *tag_id = EXIFTAGID_ARTIST;
        break;
    case _ID_HOST_COMPUTER:
        *tag_id = EXIFTAGID_HOST_COMPUTER;
        break;
    case _ID_PREDICTOR:
        *tag_id = EXIFTAGID_PREDICTOR;
        break;
    case _ID_WHITE_POINT:
        *tag_id = EXIFTAGID_WHITE_POINT;
        break;
    case _ID_PRIMARY_CHROMATICITIES:
        *tag_id = EXIFTAGID_PRIMARY_CHROMATICITIES;
        break;
    case _ID_COLOR_MAP:
        *tag_id = EXIFTAGID_COLOR_MAP;
        break;
    case _ID_HALFTONE_HINTS:
        *tag_id = EXIFTAGID_HALFTONE_HINTS;
        break;
    case _ID_TILE_WIDTH:
        *tag_id = EXIFTAGID_TILE_WIDTH;
        break;
    case _ID_TILE_LENGTH:
        *tag_id = EXIFTAGID_TILE_LENGTH;
        break;
    case _ID_TILE_OFFSET:
        *tag_id = EXIFTAGID_TILE_OFFSET;
        break;
    case _ID_TILE_BYTE_COUNTS:
        *tag_id = EXIFTAGID_TILE_BYTE_COUNTS;
        break;
    case _ID_INK_SET:
        *tag_id = EXIFTAGID_INK_SET;
        break;
    case _ID_INK_NAMES:
        *tag_id = EXIFTAGID_INK_NAMES;
        break;
    case _ID_NUMBER_OF_INKS:
        *tag_id = EXIFTAGID_NUMBER_OF_INKS;
        break;

    case _ID_DOT_RANGE:
        *tag_id = EXIFTAGID_DOT_RANGE;
        break;
    case _ID_TARGET_PRINTER:
        *tag_id = EXIFTAGID_TARGET_PRINTER;
        break;
    case _ID_EXTRA_SAMPLES:
        *tag_id = EXIFTAGID_EXTRA_SAMPLES;
        break;
    case _ID_SAMPLE_FORMAT:
        *tag_id = EXIFTAGID_SAMPLE_FORMAT;
        break;
    case _ID_TRANSFER_RANGE :
        *tag_id = EXIFTAGID_TRANSFER_RANGE;
        break;
    case _ID_JPEG_PROC:
        *tag_id = EXIFTAGID_JPEG_PROC;
        break;
    case _ID_JPEG_INTERCHANGE_FORMAT:
        *tag_id = EXIFTAGID_JPEG_INTERCHANGE_FORMAT;
        break;
    case _ID_JPEG_INTERCHANGE_FORMAT_LENGTH:
        *tag_id = EXIFTAGID_JPEG_INTERCHANGE_FORMAT_LENGTH;
        break;
    case _ID_JPEG_RESTART_INTERVAL:
        *tag_id = EXIFTAGID_JPEG_RESTART_INTERVAL;
        break;
    case _ID_JPEG_LOSSLESS_PREDICTORS:
        *tag_id = EXIFTAGID_JPEG_LOSSLESS_PREDICTORS;
        break;
    case _ID_JPEG_POINT_TRANSFORMS:
        *tag_id = EXIFTAGID_JPEG_POINT_TRANSFORMS;
        break;
    case _ID_JPEG_Q_TABLES:
        *tag_id = EXIFTAGID_JPEG_Q_TABLES;
        break;
    case _ID_JPEG_DC_TABLES:
        *tag_id = EXIFTAGID_JPEG_DC_TABLES;
        break;
    case _ID_JPEG_AC_TABLES:
        *tag_id = EXIFTAGID_JPEG_AC_TABLES;
        break;
    case _ID_YCBCR_COEFFICIENTS:
        *tag_id = EXIFTAGID_YCBCR_COEFFICIENTS;
        break;
    case _ID_YCBCR_SUB_SAMPLING:
        *tag_id = EXIFTAGID_YCBCR_SUB_SAMPLING;
        break;
    case _ID_YCBCR_POSITIONING:
        *tag_id = EXIFTAGID_YCBCR_POSITIONING;
        break;
    case _ID_REFERENCE_BLACK_WHITE:
        *tag_id = EXIFTAGID_REFERENCE_BLACK_WHITE;
        break;
    case _ID_GAMMA:
        *tag_id = EXIFTAGID_GAMMA;
        break;
    case _ID_ICC_PROFILE_DESCRIPTOR:
        *tag_id = EXIFTAGID_ICC_PROFILE_DESCRIPTOR;
        break;
    case _ID_SRGB_RENDERING_INTENT:
        *tag_id = EXIFTAGID_SRGB_RENDERING_INTENT;
        break;
    case _ID_IMAGE_TITLE:
        *tag_id = EXIFTAGID_IMAGE_TITLE;
        break;
    case _ID_COPYRIGHT:
        *tag_id = EXIFTAGID_COPYRIGHT;
        break;
    case _ID_NEW_SUBFILE_TYPE:
        *tag_id = EXIFTAGID_NEW_SUBFILE_TYPE;
        break;
    case _ID_SUBFILE_TYPE:
        *tag_id = EXIFTAGID_SUBFILE_TYPE;
        break;
/*
    case _ID_TN_COMPRESSION:
        *tag_id = EXIFTAGID_TN_COMPRESSION;
        break;
    case _ID_TN_IMAGE_DESCRIPTION:
        *tag_id = EXIFTAGID_TN_IMAGE_DESCRIPTION;
        break;
    case _ID_TN_MAKE:
        *tag_id = EXIFTAGID_TN_MAKE;
        break;
    case _ID_TN_MODEL:
        *tag_id = EXIFTAGID_TN_MODEL;
        break;
    case _ID_TN_ORIENTATION:
        *tag_id = EXIFTAGID_TN_ORIENTATION;
        break;
    case _ID_TN_X_RESOLUTION:
        *tag_id = EXIFTAGID_TN_X_RESOLUTION;
        break;
    case _ID_TN_Y_RESOLUTION:
        *tag_id = EXIFTAGID_TN_Y_RESOLUTION;
        break;
    case _ID_TN_RESOLUTION_UNIT:
        *tag_id = EXIFTAGID_TN_RESOLUTION_UNIT;
        break;
    case _ID_TN_SOFTWARE:
        *tag_id = EXIFTAGID_TN_SOFTWARE;
        break;
    case _ID_TN_JPEGINTERCHANGE_FORMAT:
        *tag_id = EXIFTAGID_TN_JPEGINTERCHANGE_FORMAT;
        break;
    case _ID_TN_JPEGINTERCHANGE_FORMAT_L:
        *tag_id = EXIFTAGID_TN_JPEGINTERCHANGE_FORMAT_L;
        break;
    case _ID_TN_YCBCR_POSITIONING:
        *tag_id = EXIFTAGID_TN_YCBCR_POSITIONING;
        break;
*/

    case _ID_EXPOSURE_TIME:
        *tag_id = EXIFTAGID_EXPOSURE_TIME;
        break;
    case _ID_F_NUMBER:
        *tag_id = EXIFTAGID_F_NUMBER;
        break;
    case _ID_EXIF_IFD_PTR:
        *tag_id = EXIFTAGID_EXIF_IFD_PTR;
        break;
    case _ID_ICC_PROFILE:
        *tag_id = EXIFTAGID_ICC_PROFILE;
        break;
    case _ID_EXPOSURE_PROGRAM:
        *tag_id = EXIFTAGID_EXPOSURE_PROGRAM;
        break;
    case _ID_SPECTRAL_SENSITIVITY:
        *tag_id = EXIFTAGID_SPECTRAL_SENSITIVITY;
        break;
    case _ID_GPS_IFD_PTR:
        *tag_id = EXIFTAGID_GPS_IFD_PTR;
        break;
    case _ID_ISO_SPEED_RATING:
        *tag_id = EXIFTAGID_ISO_SPEED_RATING;
        break;
    case _ID_OECF:
        *tag_id = EXIFTAGID_OECF;
        break;
    case _ID_EXIF_VERSION:
        *tag_id = EXIFTAGID_EXIF_VERSION;
        break;
    case _ID_EXIF_DATE_TIME_ORIGINAL:
        *tag_id = EXIFTAGID_EXIF_DATE_TIME_ORIGINAL;
        break;
    case _ID_EXIF_DATE_TIME_DIGITIZED:
        *tag_id = EXIFTAGID_EXIF_DATE_TIME_DIGITIZED;
        break;
    case _ID_EXIF_COMPONENTS_CONFIG:
        *tag_id = EXIFTAGID_EXIF_COMPONENTS_CONFIG;
        break;
    case _ID_EXIF_COMPRESSED_BITS_PER_PIXEL:
        *tag_id = EXIFTAGID_EXIF_COMPRESSED_BITS_PER_PIXEL;
        break;
    case _ID_SHUTTER_SPEED:
        *tag_id = EXIFTAGID_SHUTTER_SPEED;
        break;
    case _ID_APERTURE:
        *tag_id = EXIFTAGID_APERTURE;
        break;
    case _ID_BRIGHTNESS:
        *tag_id = EXIFTAGID_BRIGHTNESS;
        break;
    case _ID_EXPOSURE_BIAS_VALUE:
        *tag_id = EXIFTAGID_EXPOSURE_BIAS_VALUE;
        break;
    case _ID_MAX_APERTURE:
        *tag_id = EXIFTAGID_MAX_APERTURE;
        break;
    case _ID_SUBJECT_DISTANCE:
        *tag_id = EXIFTAGID_SUBJECT_DISTANCE;
        break;
    case _ID_METERING_MODE:
        *tag_id = EXIFTAGID_METERING_MODE;
        break;
    case _ID_LIGHT_SOURCE:
        *tag_id = EXIFTAGID_LIGHT_SOURCE;
        break;
    case _ID_FLASH:
        *tag_id = EXIFTAGID_FLASH;
        break;
    case _ID_FOCAL_LENGTH:
        *tag_id = EXIFTAGID_FOCAL_LENGTH;
        break;
    case _ID_SUBJECT_AREA:
        *tag_id = EXIFTAGID_SUBJECT_AREA;
        break;
    case _ID_EXIF_MAKER_NOTE:
        *tag_id = EXIFTAGID_EXIF_MAKER_NOTE;
        break;
    case _ID_EXIF_USER_COMMENT:
        *tag_id = EXIFTAGID_EXIF_USER_COMMENT;
        break;
    case _ID_SUBSEC_TIME:
        *tag_id = EXIFTAGID_SUBSEC_TIME;
        break;
    case _ID_SUBSEC_TIME_ORIGINAL:
        *tag_id = EXIFTAGID_SUBSEC_TIME_ORIGINAL;
        break;
    case _ID_SUBSEC_TIME_DIGITIZED:
        *tag_id = EXIFTAGID_SUBSEC_TIME_DIGITIZED;
        break;
    case _ID_EXIF_FLASHPIX_VERSION:
        *tag_id = EXIFTAGID_EXIF_FLASHPIX_VERSION;
        break;
    case _ID_EXIF_COLOR_SPACE:
        *tag_id = EXIFTAGID_EXIF_COLOR_SPACE;
        break;
    case _ID_EXIF_PIXEL_X_DIMENSION:
        *tag_id = EXIFTAGID_EXIF_PIXEL_X_DIMENSION;
        break;
    case _ID_EXIF_PIXEL_Y_DIMENSION:
        *tag_id = EXIFTAGID_EXIF_PIXEL_Y_DIMENSION;
        break;
    case _ID_RELATED_SOUND_FILE:
        *tag_id = EXIFTAGID_RELATED_SOUND_FILE;
        break;
    case _ID_INTEROP_IFD_PTR:
        *tag_id = EXIFTAGID_INTEROP_IFD_PTR;
        break;
    case _ID_FLASH_ENERGY:
        *tag_id = EXIFTAGID_FLASH_ENERGY;
        break;
    case _ID_SPATIAL_FREQ_RESPONSE:
        *tag_id = EXIFTAGID_SPATIAL_FREQ_RESPONSE;
        break;
    case _ID_FOCAL_PLANE_X_RESOLUTION:
        *tag_id = EXIFTAGID_FOCAL_PLANE_X_RESOLUTION;
        break;
    case _ID_FOCAL_PLANE_Y_RESOLUTION:
        *tag_id = EXIFTAGID_FOCAL_PLANE_Y_RESOLUTION;
        break;
    case _ID_FOCAL_PLANE_RESOLUTION_UNIT:
        *tag_id = EXIFTAGID_FOCAL_PLANE_RESOLUTION_UNIT;
        break;
    case _ID_SUBJECT_LOCATION:
        *tag_id = EXIFTAGID_SUBJECT_LOCATION;
        break;
    case _ID_EXPOSURE_INDEX:
        *tag_id = EXIFTAGID_EXPOSURE_INDEX;
        break;
    case _ID_SENSING_METHOD:
        *tag_id = EXIFTAGID_SENSING_METHOD;
        break;
    case _ID_FILE_SOURCE:
        *tag_id = EXIFTAGID_FILE_SOURCE;
        break;
    case _ID_SCENE_TYPE:
        *tag_id = EXIFTAGID_SCENE_TYPE;
        break;
    case _ID_CFA_PATTERN:
        *tag_id = EXIFTAGID_CFA_PATTERN;
        break;
    case _ID_CUSTOM_RENDERED:
        *tag_id = EXIFTAGID_CUSTOM_RENDERED;
        break;
    case _ID_EXPOSURE_MODE:
        *tag_id = EXIFTAGID_EXPOSURE_MODE;
        break;
    case _ID_WHITE_BALANCE:
        *tag_id = EXIFTAGID_WHITE_BALANCE;
        break;
    case _ID_DIGITAL_ZOOM_RATIO:
        *tag_id = EXIFTAGID_DIGITAL_ZOOM_RATIO;
        break;
    case _ID_FOCAL_LENGTH_35MM:
        *tag_id = EXIFTAGID_FOCAL_LENGTH_35MM;
        break;
    case _ID_GAIN_CONTROL:
        *tag_id = EXIFTAGID_GAIN_CONTROL;
        break;
    case _ID_CONTRAST:
        *tag_id = EXIFTAGID_CONTRAST;
        break;
    case _ID_SATURATION:
        *tag_id = EXIFTAGID_SATURATION;
        break;
    case _ID_SHARPNESS:
        *tag_id = EXIFTAGID_SHARPNESS;
        break;
    case _ID_DEVICE_SETTINGS_DESCRIPTION:
        *tag_id = EXIFTAGID_DEVICE_SETTINGS_DESCRIPTION;
        break;
    case _ID_SUBJECT_DISTANCE_RANGE:
        *tag_id = EXIFTAGID_SUBJECT_DISTANCE_RANGE;
        break;
    case _ID_IMAGE_UID:
        *tag_id = EXIFTAGID_IMAGE_UID;
        break;
    case _ID_PIM:
        *tag_id = EXIFTAGID_PIM_TAG;
        break;
    default:
        return JPEGERR_EFAILED;
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: exif_list_tag_id
 * Description: List all the tags that are present in the exif info object.
 * Input parameters:
 *   obj            - The Exif Info object where the Exif tags should be obtained.
 *   p_tag_ids      - An array to hold the Exif Tag IDs to be retrieved. It should
 *                    be allocated by the caller.
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
int exif_list_tagid(exif_info_obj_t    obj,
                    exif_tag_id_t     *p_tag_ids,
                    uint32_t           len,
                    uint32_t          *p_len_required)
{
    uint32_t i, j = 0;
    uint32_t len_required = 0;
    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)obj;

    if (len && !p_tag_ids)
        return JPEGERR_EBADPARM;

    // Make sure obj is initialized properly
    if (pp_entries)
    {
        // Visit each tag entry and retrive the ones that are present
        for (i = 0; i < EXIF_TAG_MAX_OFFSET; i++)
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
 * Function: exif_destroy
 * Description: Releases all allocated memory made over the lifetime of the
 *              Exif Info object. One should always call this function to clean
 *              up an 'exif_init'-ed Exif Info object.
 * Input parameters:
 *   p_obj - The pointer to the Exif Info object to be destroyed.
 * Return values: None
 * Notes: none
 *****************************************************************************/
void exif_destroy(exif_info_obj_t *p_obj)
{
    if (p_obj)
    {
        int i;
        exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(*p_obj);

        if (!pp_entries)
            return;

        for (i = 0; i < EXIF_TAG_MAX_OFFSET; i++)
        {
            exif_destroy_tag_entry(pp_entries[i]);
        }
        JPEG_FREE(*p_obj);
        *p_obj = NULL;
    }
}

/******************************************************************************
 * Function: exif_create_tag_entry
 * Description:
 * Input parameters:
 * Return values:
 * Notes: none
 *****************************************************************************/
exif_tag_entry_ex_t *exif_create_tag_entry(void)
{
    exif_tag_entry_ex_t *p_entry = (exif_tag_entry_ex_t *)JPEG_MALLOC(sizeof(exif_tag_entry_ex_t));
    if (p_entry)
    {
        STD_MEMSET(p_entry, 0, sizeof(exif_tag_entry_ex_t));
    }
    return p_entry;
}
/******************************************************************************
 * Function: exif_destroy_tag_entry
 * Description:
 * Input parameters:
 * Return values: None
 * Notes: none
 *****************************************************************************/
void exif_destroy_tag_entry(exif_tag_entry_ex_t *p_entry)
{
    if (p_entry)
    {
        // Release memory allocated by the library to hold a copy of the content
        if (p_entry->entry.copy)
        {
            if (p_entry->entry.type == EXIF_ASCII ||
                p_entry->entry.type == EXIF_UNDEFINED ||
                p_entry->entry.count > 1)
            {
                // all pointers are actually starting the same location
                if (p_entry->entry.data._ascii)
                    JPEG_FREE(p_entry->entry.data._ascii);
            }
        }
        // Release the entry's memory
        JPEG_FREE(p_entry);
    }
}

int allocate_and_copy(void **dest, void *src, uint32_t size)
{
    if (!src || !dest)
        return JPEGERR_EBADPARM;

    *dest = (void *)JPEG_MALLOC(size);
    if (!(*dest))
        return JPEGERR_EMALLOC;

    (void)STD_MEMMOVE(*dest, src, size);
    return JPEGERR_SUCCESS;
}
