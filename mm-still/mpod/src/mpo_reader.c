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
#include "jpeglog.h"
#include "mpo_reader.h"

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
int mpor_parse_first_sof(mpo_reader_t *p_mpo_reader, jpeg_frame_info_t *p_frame_info);
int mpor_parse_sof(mpo_reader_t *p_mpo_reader, jpeg_frame_info_t *p_frame_info);
int mpor_process_first_app2(mpo_reader_t *p_mpo_reader);
int mpor_process_app2(mpo_reader_t *p_mpo_reader);
int mpor_process_index_ifd(mpo_reader_t *p_mpo_reader);
int mpor_process_attribute_ifd(mpo_reader_t *p_mpo_reader);
int mpor_fetch_tag(mpo_reader_t         *p_mpo_reader,
                   exif_tag_entry_ex_t **pp_tag_entry,
                   uint16_t              expected_type,
                   exif_tag_id_t         tag_id);


/* ============================================================================
**                          Function Definitions
** ========================================================================= */
mpo_header_t *mpor_read_first_header(mpo_reader_t *p_mpo_reader)
{
    int rc;

   // Make sure the previously stored header is destroyed first
    jpeg_header_destroy(p_mpo_reader->mpo_header.p_jpeg_header);
    mpo_index_destroy((mpo_index_obj_t*)&p_mpo_reader->mpo_header.p_index_ifd);
    mpo_attribute_destroy((mpo_attribute_obj_t*)&p_mpo_reader->mpo_header.p_attribute_ifd);

    // Find SOI marker
    rc = jpegr_find_soi(p_mpo_reader->p_jpeg_reader);

    if (JPEG_SUCCEEDED(rc))
    {
        p_mpo_reader->p_jpeg_reader->header.p_main_frame_info = (jpeg_frame_info_t *)JPEG_MALLOC(sizeof(jpeg_frame_info_t));
        if (!p_mpo_reader->p_jpeg_reader->header.p_main_frame_info)
        {
            rc = JPEGERR_EMALLOC;
        }
        else
        {
            STD_MEMSET(p_mpo_reader->p_jpeg_reader->header.p_main_frame_info, 0, sizeof(jpeg_frame_info_t));
        }
    }
    if (JPEG_SUCCEEDED(rc))
    {
        rc = mpor_parse_first_sof(p_mpo_reader, p_mpo_reader->p_jpeg_reader->header.p_main_frame_info);
    }
    if (JPEG_FAILED(rc))
    {
        jpeg_header_destroy(p_mpo_reader->mpo_header.p_jpeg_header);
        mpo_index_destroy((mpo_index_obj_t*)&p_mpo_reader->mpo_header.p_index_ifd);
        mpo_attribute_destroy((mpo_attribute_obj_t*)&p_mpo_reader->mpo_header.p_attribute_ifd);

        return NULL;
    }

    jpeg_dump_header(&p_mpo_reader->p_jpeg_reader->header);

    return &p_mpo_reader->mpo_header;
}


mpo_header_t *mpor_read_header(mpo_reader_t *p_mpo_reader)
{
    int rc;

    // Make sure the previously stored header is destroyed first
    jpeg_header_destroy(p_mpo_reader->mpo_header.p_jpeg_header);
    mpo_index_destroy((mpo_index_obj_t*)&p_mpo_reader->mpo_header.p_index_ifd);
    mpo_attribute_destroy((mpo_attribute_obj_t*)&p_mpo_reader->mpo_header.p_attribute_ifd);

    // Find SOI marker
    rc = jpegr_find_soi(p_mpo_reader->p_jpeg_reader);

    if (JPEG_SUCCEEDED(rc))
    {
        p_mpo_reader->p_jpeg_reader->header.p_main_frame_info = (jpeg_frame_info_t *)JPEG_MALLOC(sizeof(jpeg_frame_info_t));
        if (!p_mpo_reader->p_jpeg_reader->header.p_main_frame_info)
        {
            rc = JPEGERR_EMALLOC;
        }
        else
        {
            STD_MEMSET(p_mpo_reader->p_jpeg_reader->header.p_main_frame_info, 0, sizeof(jpeg_frame_info_t));
        }
    }
    if (JPEG_SUCCEEDED(rc))
    {
        rc = mpor_parse_sof(p_mpo_reader, p_mpo_reader->p_jpeg_reader->header.p_main_frame_info);
    }
    if (JPEG_FAILED(rc))
    {
        jpeg_header_destroy(p_mpo_reader->mpo_header.p_jpeg_header);
        mpo_index_destroy((mpo_index_obj_t*)&p_mpo_reader->mpo_header.p_index_ifd);
        mpo_attribute_destroy((mpo_attribute_obj_t*)&p_mpo_reader->mpo_header.p_attribute_ifd);

        return NULL;
    }

    jpeg_dump_header(&p_mpo_reader->p_jpeg_reader->header);

    return &p_mpo_reader->mpo_header;
}


/**
 * This method parses the first individual image until the first SOS encountered is processed.
 */
int mpor_parse_first_sof(mpo_reader_t *p_mpo_reader, jpeg_frame_info_t *p_frame_info)
{
    int rc;
    for (;;)
    {
        jpeg_marker_t marker = jpegr_find_next_header(p_mpo_reader->p_jpeg_reader);

        switch (marker)
        {
        case M_SOF0:
        case M_SOF1:
        case M_SOF2:
        case M_SOF3:
        case M_SOF5:
        case M_SOF6:
        case M_SOF7:
        case M_SOF9:
        case M_SOF10:
        case M_SOF11:
        case M_SOF13:
        case M_SOF14:
        case M_SOF15:
            rc = jpegr_process_sof(p_mpo_reader->p_jpeg_reader, p_frame_info, marker);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_SOI:
            // not expected to see SOI again, return failure
            return JPEGERR_EFAILED;
        case M_EOI:
            // not expected to see EOI, return failure
            return JPEGERR_EFAILED;
        case M_SOS:
            rc = jpegr_process_sos(p_mpo_reader->p_jpeg_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            return JPEGERR_SUCCESS;
        case M_DHT:
            rc = jpegr_process_dht(p_mpo_reader->p_jpeg_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_DQT:
            rc = jpegr_process_dqt(p_mpo_reader->p_jpeg_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_DRI:
            rc = jpegr_process_dri(p_mpo_reader->p_jpeg_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_APP0:
            // Even failed to process APP0, still continue since
            // it is not mandatory
            (void)jpegr_process_app0(p_mpo_reader->p_jpeg_reader);
            break;
        case M_APP1:
            // Even failed to process APP1, still continue since
            // it is not mandatory
            (void)jpegr_process_app1(p_mpo_reader->p_jpeg_reader);
            p_mpo_reader->p_jpeg_reader->endianness = EXIF_BIG_ENDIAN;
            break;
        case M_APP2:
            // Even failed to process APP2, still continue since
            // it is not mandatory
            (void)mpor_process_first_app2(p_mpo_reader);
            break;
        case M_APP3:
            // Even failed to process APP3, still continue since
            // it is not mandatory
            (void)jpegr_process_app3(p_mpo_reader->p_jpeg_reader);
            break;

        default:
            {
                // Skip unsupported marker
                uint16_t bytesToSkip = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);

                if (bytesToSkip >= 2)
                    bytesToSkip -= 2;

                JPEG_DBG_LOW("Skipping unsupported marker (FF%X)...", marker);
                p_mpo_reader->p_jpeg_reader->next_byte_offset += bytesToSkip;
                break;
            }
        }
    }
}

/**
 * This method parses the images other than the first individual image until the first SOS encountered is processed.
 */
int mpor_parse_sof(mpo_reader_t *p_mpo_reader, jpeg_frame_info_t *p_frame_info)
{
    int rc;
    for (;;)
    {
        jpeg_marker_t marker = jpegr_find_next_header(p_mpo_reader->p_jpeg_reader);

        switch (marker)
        {
        case M_SOF0:
        case M_SOF1:
        case M_SOF2:
        case M_SOF3:
        case M_SOF5:
        case M_SOF6:
        case M_SOF7:
        case M_SOF9:
        case M_SOF10:
        case M_SOF11:
        case M_SOF13:
        case M_SOF14:
        case M_SOF15:
            rc = jpegr_process_sof(p_mpo_reader->p_jpeg_reader, p_frame_info, marker);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_SOI:
            // not expected to see SOI again, return failure
            return JPEGERR_EFAILED;
        case M_EOI:
            // not expected to see EOI, return failure
            return JPEGERR_EFAILED;
        case M_SOS:
            rc = jpegr_process_sos(p_mpo_reader->p_jpeg_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            return JPEGERR_SUCCESS;
        case M_DHT:
            rc = jpegr_process_dht(p_mpo_reader->p_jpeg_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_DQT:
            rc = jpegr_process_dqt(p_mpo_reader->p_jpeg_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_DRI:
            rc = jpegr_process_dri(p_mpo_reader->p_jpeg_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_APP0:
            // Even failed to process APP0, still continue since
            // it is not mandatory
            (void)jpegr_process_app0(p_mpo_reader->p_jpeg_reader);
            break;
        case M_APP1:
            // Even failed to process APP1, still continue since
            // it is not mandatory
            (void)jpegr_process_app1(p_mpo_reader->p_jpeg_reader);
            p_mpo_reader->p_jpeg_reader->endianness = EXIF_BIG_ENDIAN;
            break;
        case M_APP2:
            // Even failed to process APP2, still continue since
            // it is not mandatory
            (void)mpor_process_app2(p_mpo_reader);
            break;
        case M_APP3:
            // Even failed to process APP3, still continue since
            // it is not mandatory
            (void)jpegr_process_app3(p_mpo_reader->p_jpeg_reader);
            break;

        default:
            {
                // Skip unsupported marker
                uint16_t bytesToSkip = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);

                if (bytesToSkip >= 2)
                    bytesToSkip -= 2;

                JPEG_DBG_LOW("Skipping unsupported marker (FF%X)...", marker);
                p_mpo_reader->p_jpeg_reader->next_byte_offset += bytesToSkip;
                break;
            }
        }
    }
}

/**
 * This method processes app2 marker segment for the first individual image.
 */
int mpor_process_first_app2(mpo_reader_t *p_mpo_reader)
{
    int rc = JPEGERR_SUCCESS;
    uint32_t app2_start_offset = p_mpo_reader->p_jpeg_reader->next_byte_offset;
    uint32_t app2_len = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
    uint32_t nEndianness = 0;
    uint32_t original_endianness = p_mpo_reader->p_jpeg_reader->endianness;

    // Create the mpo index ifd object if not already exists
    if (!p_mpo_reader->mpo_header.p_index_ifd)
    {
        p_mpo_reader->mpo_header.p_index_ifd = (mpo_index_ifd_t *)JPEG_MALLOC(sizeof(mpo_index_ifd_t));
        if (!p_mpo_reader->mpo_header.p_index_ifd)
            rc = JPEGERR_EMALLOC;
        else
            STD_MEMSET(p_mpo_reader->mpo_header.p_index_ifd, 0, sizeof(mpo_index_ifd_t));
    }

    // Create the mpo attribute ifd object if not already exists
    if (!p_mpo_reader->mpo_header.p_attribute_ifd)
    {
        p_mpo_reader->mpo_header.p_attribute_ifd = (mpo_attribute_ifd_t *)JPEG_MALLOC(sizeof(mpo_attribute_ifd_t));
        if (!p_mpo_reader->mpo_header.p_attribute_ifd)
            rc = JPEGERR_EMALLOC;
        else
            STD_MEMSET(p_mpo_reader->mpo_header.p_attribute_ifd, 0, sizeof(mpo_attribute_ifd_t));
    }

    // Match with "MPO "
    if (JPEG_SUCCEEDED(rc))
        rc = (jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader) == 0x4D504600) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;

    if (JPEG_SUCCEEDED(rc))
    {
        // Store MPO Start of Offset address
        p_mpo_reader->start_offset = p_mpo_reader->p_jpeg_reader->next_byte_offset;

        // nEndianness (Big-Endian = 0x4D4D; Little-Endian = 0x4949)
        nEndianness = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
        rc = (nEndianness == 0x4d4d || nEndianness == 0x4949) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;
    }

    if (JPEG_SUCCEEDED(rc))
    {
        // Keep the MPO IFD endianness
        // MPO IFD endianness could be different from the JPEG endianness
        p_mpo_reader->mp_endian = (nEndianness == 0x4d4d) ? EXIF_BIG_ENDIAN : EXIF_LITTLE_ENDIAN;

        // Temporarily set the JPEG endianness to be the same as MPO IFD endianness,
        // for jpegr_fetch function needs it when fetching data from input buffer
        p_mpo_reader->p_jpeg_reader->endianness = p_mpo_reader->mp_endian;

        // Match the TIFF Version
        rc = (jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader) == 0x2A) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;
    }

    if (JPEG_SUCCEEDED(rc))
    {
        // MP Indix IFD start address = start_offset + first IFD offset
        p_mpo_reader->p_jpeg_reader->next_byte_offset = p_mpo_reader->start_offset + jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);

        rc = mpor_process_index_ifd(p_mpo_reader);
    }

    if (JPEG_FAILED(rc))
    {
        mpo_index_obj_t mpo_index_obj = (mpo_index_obj_t)p_mpo_reader->mpo_header.p_index_ifd;
        mpo_index_destroy(&mpo_index_obj);
        p_mpo_reader->mpo_header.p_index_ifd = NULL;
    }

    if (JPEG_SUCCEEDED(rc))
    {
        if (!p_mpo_reader->next_ifd_offset)
            return JPEGERR_SUCCESS;

        p_mpo_reader->p_jpeg_reader->next_byte_offset = p_mpo_reader->start_offset + p_mpo_reader->next_ifd_offset;

        rc = mpor_process_attribute_ifd(p_mpo_reader);
    }

    if (JPEG_FAILED(rc))
    {
        mpo_attribute_obj_t mpo_attribute_obj = (mpo_attribute_obj_t)p_mpo_reader->mpo_header.p_attribute_ifd;
        mpo_attribute_destroy(&mpo_attribute_obj);
        p_mpo_reader->mpo_header.p_attribute_ifd = NULL;
    }

    p_mpo_reader->p_jpeg_reader->next_byte_offset = app2_start_offset + app2_len;
    p_mpo_reader->p_jpeg_reader->endianness = original_endianness;

    return rc;
}

/**
 * This method processes app2 marker segment for the images other than the first individual image.
 */
int mpor_process_app2(mpo_reader_t *p_mpo_reader)
{
    int rc = JPEGERR_SUCCESS;
    uint32_t app2_start_offset = p_mpo_reader->p_jpeg_reader->next_byte_offset;
    uint32_t app2_len = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
    uint32_t nEndianness = 0;
    uint32_t original_endianness = p_mpo_reader->p_jpeg_reader->endianness;

    // Create the mpo attribute ifd object if not already exists
    if (!p_mpo_reader->mpo_header.p_attribute_ifd)
    {
        p_mpo_reader->mpo_header.p_attribute_ifd = (mpo_attribute_ifd_t *)JPEG_MALLOC(sizeof(mpo_attribute_ifd_t));
        if (!p_mpo_reader->mpo_header.p_attribute_ifd)
            rc = JPEGERR_EMALLOC;
        else
            STD_MEMSET(p_mpo_reader->mpo_header.p_attribute_ifd, 0, sizeof(mpo_attribute_ifd_t));
    }

    // Match with "MPO "
    if (JPEG_SUCCEEDED(rc))
        rc = (jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader) == 0x4D504600) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;

    if (JPEG_SUCCEEDED(rc))
    {
        // Store MPO Start of Offset address
        p_mpo_reader->start_offset = p_mpo_reader->p_jpeg_reader->next_byte_offset;

        // nEndianness (Big-Endian = 0x4D4D; Little-Endian = 0x4949)
        nEndianness = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
        rc = (nEndianness == 0x4d4d || nEndianness == 0x4949) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;
    }

    if (JPEG_SUCCEEDED(rc))
    {
        // Keep the MPO IFD endianness
        // MPO IFD endianness could be different from the JPEG endianness
        p_mpo_reader->mp_endian = (nEndianness == 0x4d4d) ? EXIF_BIG_ENDIAN : EXIF_LITTLE_ENDIAN;

        // Temporarily set the JPEG endianness to be the same as MPO IFD endianness,
        // for jpegr_fetch function needs it when fetching data from input buffer
        p_mpo_reader->p_jpeg_reader->endianness = p_mpo_reader->mp_endian;

        // Match the TIFF Version
        rc = (jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader) == 0x2A) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;
    }

    if (JPEG_SUCCEEDED(rc))
    {
        // MP Attribute IFD start address = start_offset + first IFD offset
        p_mpo_reader->p_jpeg_reader->next_byte_offset = p_mpo_reader->start_offset + jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);

        rc = mpor_process_attribute_ifd(p_mpo_reader);
    }

    if (JPEG_FAILED(rc))
    {

        mpo_attribute_obj_t mpo_attribute_obj = (mpo_attribute_obj_t)p_mpo_reader->mpo_header.p_attribute_ifd;
        mpo_attribute_destroy(&mpo_attribute_obj);
        p_mpo_reader->mpo_header.p_attribute_ifd = NULL;
    }

    p_mpo_reader->p_jpeg_reader->next_byte_offset = app2_start_offset + app2_len;
    p_mpo_reader->p_jpeg_reader->endianness = original_endianness;

    return rc;
}


/**
 * This method processes the index ifd.
 */
int mpor_process_index_ifd(mpo_reader_t *p_mpo_reader)
{
    int rc = JPEGERR_SUCCESS;
    int16_t nNumTags, i;

    nNumTags = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
    for (i = 0; i < nNumTags; i++)
    {
        uint16_t tag_id = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
        switch (tag_id)
        {
        case _ID_MP_F_VERSION_FIRST:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_index_ifd->p_mp_f_version_first,
                                 EXIF_UNDEFINED, MPOTAGID_MP_F_VERSION_FIRST);
            break;
        case _ID_NUMBER_OF_IMAGES:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_index_ifd->p_number_of_images,
                                 EXIF_LONG, MPOTAGID_NUMBER_OF_IMAGES);
            break;
        case _ID_MP_ENTRY:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_index_ifd->p_mp_entry,
                                 EXIF_UNDEFINED, MPOTAGID_MP_ENTRY);
            break;
        case _ID_IMAGE_UID_LIST:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_index_ifd->p_image_uid_list,
                                 EXIF_UNDEFINED, MPOTAGID_IMAGE_UID_LIST);
            break;
        case _ID_TOTAL_FRAMES:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_index_ifd->p_total_frames,
                                 EXIF_LONG, MPOTAGID_TOTAL_FRAMES);
            break;

        default:
            p_mpo_reader->p_jpeg_reader->next_byte_offset += 10;
            break;
        }
        if (rc != JPEGERR_TAGTYPE_UNEXPECTED && JPEG_FAILED(rc))
            return rc;
    }
    p_mpo_reader->next_ifd_offset = jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);

    return JPEGERR_SUCCESS;
}

/**
 * This method processes the attribute ifd.
 */
int mpor_process_attribute_ifd(mpo_reader_t *p_mpo_reader)
{
    int rc = JPEGERR_SUCCESS;
    uint16_t nNumTags, i;

    nNumTags = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
    for (i = 0; i < nNumTags; i++)
    {
        uint16_t tag_id = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
        switch (tag_id)
        {
        case _ID_MP_F_VERSION:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_mp_f_version,
                                 EXIF_UNDEFINED, MPOTAGID_MP_F_VERSION);
            break;
        case _ID_MP_INDIVIDUAL_NUM:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_mp_individual_num,
                                 EXIF_LONG, MPOTAGID_MP_INDIVIDUAL_NUM);
            break;
        case _ID_PAN_ORIENTATION:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_pan_orientation,
                                 EXIF_LONG, MPOTAGID_PAN_ORIENTATION);
            break;
        case _ID_PAN_OVERLAP_H:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_pan_overlap_h,
                                 EXIF_RATIONAL, MPOTAGID_PAN_OVERLAP_H);
            break;
        case _ID_PAN_OVERLAP_V:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_pan_overlap_v,
                                 EXIF_RATIONAL, MPOTAGID_PAN_OVERLAP_V);
            break;
        case _ID_BASE_VIEWPOINT_NUM:
            rc = mpor_fetch_tag( p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_base_viewpoint_num,
                                 EXIF_LONG, MPOTAGID_BASE_VIEWPOINT_NUM);
            break;
        case _ID_CONVERGENCE_ANGLE:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_convergence_angle,
                                 EXIF_SRATIONAL, MPOTAGID_CONVERGENCE_ANGLE);
            break;
        case _ID_BASELINE_LENGTH:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_baseline_length,
                                 EXIF_RATIONAL, MPOTAGID_BASELINE_LENGTH);
            break;
        case _ID_VERTICAL_DIVERGENCE:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_vertical_divergence,
                                 EXIF_SRATIONAL, MPOTAGID_VERTICAL_DIVERGENCE);
            break;
        case _ID_AXIS_DISTANCE_X:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_axis_distance_x,
                                 EXIF_SRATIONAL, MPOTAGID_AXIS_DISTANCE_X);
            break;
        case _ID_AXIS_DISTANCE_Y:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_axis_distance_y,
                                 EXIF_SRATIONAL, MPOTAGID_AXIS_DISTANCE_Y);
            break;
        case _ID_AXIS_DISTANCE_Z:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_axis_distance_z,
                                 EXIF_SRATIONAL, MPOTAGID_AXIS_DISTANCE_Z);
            break;
        case _ID_YAW_ANGLE:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_yaw_angle,
                                 EXIF_SRATIONAL, MPOTAGID_YAW_ANGLE);
            break;
        case _ID_PITCH_ANGLE:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_pitch_angle,
                                 EXIF_SRATIONAL, MPOTAGID_PITCH_ANGLE);
            break;
        case _ID_ROLL_ANGLE:
            rc = mpor_fetch_tag(p_mpo_reader, &p_mpo_reader->mpo_header.p_attribute_ifd->p_roll_angle,
                                 EXIF_SRATIONAL, MPOTAGID_ROLL_ANGLE);
            break;

        default:
            p_mpo_reader->p_jpeg_reader->next_byte_offset += 10;
            break;
        }
        if (rc != JPEGERR_TAGTYPE_UNEXPECTED && JPEG_FAILED(rc))
            return rc;
    }

    p_mpo_reader->next_ifd_offset = jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);

    return JPEGERR_SUCCESS;
}


// Create a tag and fill in the information based on the input
// stream.
// Side effect: If JPEGERR_SUCCESS is returned, it
// means the tag is fetched successfully and the next fetch
// offset will be set to the next tag ID. Otherwise, the next
// fetch offset will be resetted back to the current tag ID.
int mpor_fetch_tag(mpo_reader_t         *p_mpo_reader,
                   exif_tag_entry_ex_t **pp_tag_entry,
                   uint16_t              expected_type,
                   exif_tag_id_t         tag_id)
{
    uint32_t tag_start_offset, cnt, i;
    int rc = JPEGERR_SUCCESS;
    exif_tag_entry_ex_t *p_new_tag;

    // Destroy any previously allocated entry
    exif_destroy_tag_entry(*pp_tag_entry);

    p_new_tag = exif_create_tag_entry();

    if (!p_new_tag)
        return JPEGERR_EMALLOC;

    // Save the current offset for possible later seeking
    tag_start_offset      = p_mpo_reader->p_jpeg_reader->next_byte_offset;
    p_new_tag->entry.type = (exif_tag_type_t)jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
    cnt                   = jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);

    // Check if the fetched type is expected
    if (p_new_tag->entry.type != expected_type)
    {
        // Reset fetch offset
        p_mpo_reader->p_jpeg_reader->next_byte_offset = tag_start_offset;
        exif_destroy_tag_entry(p_new_tag);
        //nullify the pp_tag_entry, since it is already destroyed,
        //so that next time when jpegr_fetch_tag is been called in,
        //it will not double free the tag entry.
        *pp_tag_entry = NULL;
        return JPEGERR_TAGTYPE_UNEXPECTED;
    }

    switch (expected_type)
    {
    case EXIF_BYTE:
        if (cnt > 1)
        {
            p_new_tag->entry.data._bytes = (uint8_t *)JPEG_MALLOC(cnt);
            if (!p_new_tag->entry.data._bytes)
            {
                rc = JPEGERR_EMALLOC;
                break;
            }

            // data doesn't fit into 4 bytes, read from an offset
            if (cnt > 4)
                p_mpo_reader->p_jpeg_reader->next_byte_offset = p_mpo_reader->start_offset + jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);

            jpegr_fetch_nbytes(p_mpo_reader->p_jpeg_reader, p_new_tag->entry.data._bytes, cnt, NULL);
        }
        else
        {
            p_new_tag->entry.data._byte = jpegr_fetch_byte(p_mpo_reader->p_jpeg_reader);
        }
        break;
    case EXIF_SHORT:
        if (cnt > 1)
        {
            //Check if cnt * sizeof(uint16_t) is overflowing before allocating memory
            // cnt * sizeof(uint16_t) should be less than the limit of uint32_t
            if(cnt > ((1<<(sizeof(uint32_t)*8 - sizeof(uint16_t)))-1))
            {
                //This means it is been overflown,
                //and not something good to proceed
                JPEG_DBG_ERROR(" %s Too big cnt = 0x%x\n",__func__,cnt);
                rc = JPEGERR_EFAILED;
                break;
            }

            p_new_tag->entry.data._shorts = (uint16_t*)JPEG_MALLOC(cnt * sizeof(uint16_t));
            if (!p_new_tag->entry.data._shorts)
            {
                rc = JPEGERR_EMALLOC;
                break;
            }

            // Data doesn't fit into 4 bytes, read from an offset
            if (cnt > 4)
                p_mpo_reader->p_jpeg_reader->next_byte_offset = p_mpo_reader->start_offset + jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);

            for (i = 0; i < cnt; i++)
            {
                p_new_tag->entry.data._shorts[i] = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
            }
        }
        else
        {
            p_new_tag->entry.data._short = jpegr_fetch_2bytes(p_mpo_reader->p_jpeg_reader);
        }
        break;
    case EXIF_LONG:
    case EXIF_SLONG:
        if (cnt > 1)
        {
            //Check if cnt * sizeof(uint32_t) is overflowing before allocating memory
            // cnt * sizeof(uint32_t) should be less than the limit of uint32_t
            if(cnt > ((1<<(sizeof(uint32_t)*8 - sizeof(uint32_t)))-1))
            {
                //This means it is been overflown,
                //and not something good to proceed
                JPEG_DBG_ERROR(" %s Too big cnt = 0x%x\n",__func__,cnt);
                rc = JPEGERR_EFAILED;
                break;
            }
            p_new_tag->entry.data._longs = (uint32_t *)JPEG_MALLOC(cnt * sizeof(uint32_t));
            if (!p_new_tag->entry.data._longs)
            {
                rc = JPEGERR_EMALLOC;
                break;
            }

            // Data doesn't fit into 4 bytes, read from an offset
            p_mpo_reader->p_jpeg_reader->next_byte_offset = p_mpo_reader->start_offset + jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);
            for (i = 0; i < cnt; i++)
            {
                p_new_tag->entry.data._longs[i] = (uint32_t)jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);
            }
        }
        else
        {
            p_new_tag->entry.data._long = (uint32_t)jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);
        }
        break;
    case EXIF_RATIONAL:
    case EXIF_SRATIONAL:
        // Data doesn't fit into 4 bytes, read from an offset
        p_mpo_reader->p_jpeg_reader->next_byte_offset = p_mpo_reader->start_offset + jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);
        if (cnt > 1)
        {
           //Check if cnt * sizeof(rat_t) is overflowing before allocating memory
           // cnt * sizeof(rat_t) should be less than the limit of uint32_t
            if(cnt > ((1<<(sizeof(uint32_t)*8 - sizeof(rat_t)))-1))
            {
                //This means it is been overflown,
                //and not something good to proceed
                JPEG_DBG_ERROR(" %s Too big cnt = 0x%x\n",__func__,cnt);
                rc = JPEGERR_EFAILED;
                break;
            }

            p_new_tag->entry.data._rats = (rat_t *)JPEG_MALLOC(cnt * sizeof(rat_t));
            if (!p_new_tag->entry.data._rats)
            {
                rc = JPEGERR_EMALLOC;
                break;
            }

            for (i = 0; i < cnt; i++)
            {
                p_new_tag->entry.data._rats[i].num   = jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);
                p_new_tag->entry.data._rats[i].denom = jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);
            }
        }
        else
        {
            p_new_tag->entry.data._rat.num   = jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);
            p_new_tag->entry.data._rat.denom = jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);
        }
        break;
    case EXIF_ASCII:
    case EXIF_UNDEFINED:
        {
            //Check if cnt * sizeof(char) is overflowing before allocating memory
            // cnt * sizeof(char) should be less than the limit of uint32_t
            if(cnt > ((1<<((sizeof(uint32_t)*8) - sizeof(char)))-1))
            {
                //This means it is been overflown,
                //and not something good to proceed
                JPEG_DBG_ERROR(" %s Too big cnt = 0x%x\n",__func__,cnt);
                rc = JPEGERR_EFAILED;
                break;
            }
            p_new_tag->entry.data._ascii = (char *)JPEG_MALLOC(cnt * sizeof(char));
            if (!p_new_tag->entry.data._ascii)
            {
                rc = JPEGERR_EMALLOC;
                break;
            }

            if (cnt > 4)
                p_mpo_reader->p_jpeg_reader->next_byte_offset = p_mpo_reader->start_offset + jpegr_fetch_4bytes(p_mpo_reader->p_jpeg_reader);

            jpegr_fetch_nbytes(p_mpo_reader->p_jpeg_reader, (uint8_t *)p_new_tag->entry.data._ascii, cnt, NULL);
        }
        break;
    default:
        break;
    }
    if (JPEG_SUCCEEDED(rc))
    {
        p_new_tag->entry.copy = true;
        p_new_tag->entry.count = cnt;
        p_new_tag->tag_id = tag_id;
        *pp_tag_entry = p_new_tag;
        p_mpo_reader->p_jpeg_reader->next_byte_offset = tag_start_offset + 10;
    }
    else
    {
        p_mpo_reader->p_jpeg_reader->next_byte_offset = tag_start_offset;
        exif_destroy_tag_entry(p_new_tag);
        //nullify the pp_tag_entry, since it is already destroyed,
        //so that next time when jpegr_fetch_tag is been called in,
        //it will not double free the tag entry.
        *pp_tag_entry = NULL;
    }
    return rc;
}


