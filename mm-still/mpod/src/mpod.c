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
#include "mpod_private.h"
#include "jpegd_private.h"
#include "jpeglog.h"


uint16_t mpod_get_2bytes(uint8_t *p_input, exif_endianness_t endianness);
uint32_t mpod_get_4bytes(uint8_t *p_input, exif_endianness_t endianness);

/******************************************************************************
 * Function: mpod_init
 * Description: Initializes the MPO Decoder object. Dynamic allocations take
 *              place during the call. One should always call mpod_destroy to
 *              clean up the MPO Decoder object after use.
 * Input parameters:
 *   p_mpod_obj  - The pointer to the MPO Decoder object to be initialized.
 *   p_event_handler   - The function pointer to the handler function handling
 *                       Jpeg Decoder events.
 *   p_output_handler  - The function pointer to the output handler function
 *                       handling the filled output buffer.
 *   p_user_data       - The pointer to user data that the decoder stores
 *                       untouched and will be passed back to client in any
 *                       callbacks (input request handler or event handler).
 *                       It is not mandatory and can be set to NULL.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_init(mpod_obj_t             *p_mpod_obj,
              mpod_event_handler_t    p_event_handler,
              mpod_output_handler_t   p_output_handler,
              void                   *p_user_data)
{
    jpeg_decoder_t  *p_jpeg_decoder;
    mpo_decoder_t   *p_mpo_decoder;
    int              rc;

    // Allocate for the MPO Decoder structure
    p_mpo_decoder = (mpo_decoder_t *)JPEG_MALLOC(sizeof(mpo_decoder_t));
    if (!p_mpo_decoder)
        return JPEGERR_EMALLOC;
    // Initialize all fields in the decoder structure
    STD_MEMSET(p_mpo_decoder, 0, sizeof(mpo_decoder_t));

    // Initialize decoder
    rc = jpegd_init((jpegd_obj_t)&p_jpeg_decoder,
                    (jpegd_event_handler_t)p_event_handler,
                    (jpegd_output_handler_t)p_output_handler,
                    p_user_data);
    if (JPEG_FAILED(rc))
    {
        JPEG_FREE(p_mpo_decoder);
        return rc;
    }

    // Pass the JPEG header handle to MPO header
    p_mpo_decoder->mpo_reader.mpo_header.p_jpeg_header = &p_jpeg_decoder->reader.header;

    // Pass the JPEG reader handle to MPO header
    p_mpo_decoder->mpo_reader.p_jpeg_reader = &p_jpeg_decoder->reader;

    // Pass the JPEG decoder handler to MPO decoder
    p_mpo_decoder->p_jpeg_decoder = p_jpeg_decoder;

    // Cast the created MPO decoder into the opaque object pointer
    *p_mpod_obj = (mpod_obj_t)p_mpo_decoder;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpod_set_source
 * Description: Assigns the image source to the MPO Decoder object.
 * Input parameters:
 *   obj       - The MPO Decoder object
 *   p_source  - The pointer to the source object to be assigned to the MPO
 *               Decoder.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_set_source(
    mpod_obj_t  mpod_obj,
    mpod_src_t  *p_source)
{
    jpegd_obj_t    jpegd_obj;
    mpo_decoder_t *p_mpo_decoder;
    int            rc;

    p_mpo_decoder = (mpo_decoder_t *)mpod_obj;
    if (!p_mpo_decoder)
    {
        return JPEGERR_EUNINITIALIZED;
    }
    jpegd_obj = (jpegd_obj_t)p_mpo_decoder->p_jpeg_decoder;

    rc = jpegd_set_source(jpegd_obj,(jpegd_src_t *)p_source);
    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpod_read_first_header
 * Description: Obtains the header information from the first individual image.
 *              This is a synchronous call. When it returns, the header
 *              structure is filled.
 *              The MPO header structure (mpo_hdr_t) contains the mpo_index_obj_t
 *              field and mpo_attribute_obj_t in which their data will be
 *              destroyed when the MPO decoder gets destroyed. The caller
 *              therefore should ensure that the MPO decoder is kept alive
 *              as long as the MPO index data and MPO attribute data need to be
 *              retrieved.
 * Input parameters:
 *   mpod_obj_t    - The MPO Decoder object.
 *   p_mpo_header  - The pointer to the MPO header structure to be filled.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EUNINITIALIZED
 *     JPEGERR_EBADSTATE
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_read_first_header(mpod_obj_t mpo_obj, mpo_hdr_t *p_mpo_header)
{
    mpo_decoder_t *p_mpo_decoder = (mpo_decoder_t *)mpo_obj;
    if (!p_mpo_decoder)
    {
        return JPEGERR_EUNINITIALIZED;
    }

    // Reject request if decoder is actively decoding or is cleaning up
    if (p_mpo_decoder->p_jpeg_decoder->state != JPEGD_IDLE)
    {
        return JPEGERR_EBADSTATE;
    }

    // Parse the Jpeg header
    p_mpo_decoder->p_mpo_full_hdr = mpor_read_first_header(&p_mpo_decoder->mpo_reader);

    // If full header is present, derive short header from it
    if (p_mpo_decoder->p_mpo_full_hdr)
    {
        jpegd_convert_frame_info(p_mpo_decoder->p_mpo_full_hdr->p_jpeg_header->p_main_frame_info,
                                 &(p_mpo_header->main));
        jpegd_convert_frame_info(p_mpo_decoder->p_mpo_full_hdr->p_jpeg_header->p_tn_frame_info,
                                 &(p_mpo_header->thumbnail));

        p_mpo_header->exif_info         = (exif_info_obj_t)p_mpo_decoder->p_mpo_full_hdr->p_jpeg_header->p_exif_info;
        p_mpo_header->mpo_index_obj     = (mpo_index_obj_t)p_mpo_decoder->p_mpo_full_hdr->p_index_ifd;
        p_mpo_header->mpo_attribute_obj = (mpo_attribute_obj_t)p_mpo_decoder->p_mpo_full_hdr->p_attribute_ifd;

        // Assign the JPEG header to decoder object member
        p_mpo_decoder->p_jpeg_decoder->p_full_hdr = p_mpo_decoder->p_mpo_full_hdr->p_jpeg_header;
    }
    else
    {
        return JPEGERR_EFAILED;
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpod_read_header
 * Description: Obtains the header information from the individual images other
 *              than the first individual image. This is a synchronous call.
 *              When it returns, the header structure is filled.
 *              The MPO header structure (mpo_hdr_t) contains the mpo_index_obj_t
 *              field and mpo_attribute_obj_t in which their data will be
 *              destroyed when the MPO decoder gets destroyed. The caller
 *              therefore should ensure that the MPO decoder is kept alive
 *              as long as the MPO index data and MPO attribute data need to be
 *              retrieved.
 * Input parameters:
 *   mpod_obj_t    - The MPO Decoder object.
 *   p_mpo_header  - The pointer to the MPO header structure to be filled.
 *   image_index   - The id of the image being processed.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EUNINITIALIZED
 *     JPEGERR_EBADSTATE
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_read_header(mpod_obj_t mpo_obj, mpo_hdr_t *p_mpo_header)
{
    mpo_decoder_t  *p_mpo_decoder = (mpo_decoder_t *)mpo_obj;
    if (!p_mpo_decoder)
    {
        return JPEGERR_EUNINITIALIZED;
    }

    // Reject request if decoder is actively decoding or is cleaning up
    if (p_mpo_decoder->p_jpeg_decoder->state != JPEGD_IDLE)
    {
        return JPEGERR_EBADSTATE;
    }

    // Assign the address of the JPEG header structure to the JPEG header pointer
    // in the MPO header
    p_mpo_decoder->mpo_reader.mpo_header.p_jpeg_header = &p_mpo_decoder->p_jpeg_decoder->reader.header;

    // Parse the Jpeg header
    p_mpo_decoder->p_mpo_full_hdr = mpor_read_header(&p_mpo_decoder->mpo_reader);

    // If full header is present, derive short header from it
    if (p_mpo_decoder->p_mpo_full_hdr)
    {
        jpegd_convert_frame_info(p_mpo_decoder->p_mpo_full_hdr->p_jpeg_header->p_main_frame_info,
                                 &(p_mpo_header->main));
        jpegd_convert_frame_info(p_mpo_decoder->p_mpo_full_hdr->p_jpeg_header->p_tn_frame_info,
                                 &(p_mpo_header->thumbnail));

        p_mpo_header->exif_info         = (exif_info_obj_t)p_mpo_decoder->p_mpo_full_hdr->p_jpeg_header->p_exif_info;
        p_mpo_header->mpo_attribute_obj = (mpo_attribute_obj_t)p_mpo_decoder->p_mpo_full_hdr->p_attribute_ifd;

        // Assign the JPEG header to decoder object member
        p_mpo_decoder->p_jpeg_decoder->p_full_hdr = p_mpo_decoder->p_mpo_full_hdr->p_jpeg_header;
    }
    else
    {
        return JPEGERR_EFAILED;
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpod_enqueue_output_buf
 * Description: Enqueue a sequence of buffers. It accepts the pointer to the
 *              buffer array to be enqueued and the number of buffers in the
 *              array, appends the buffers sequentially to the queue, and
 *              updates the is_in_q flag accordingly. The number of buffers
 *              to be enqueued is checked against the size of the queue, and
 *              return fail if it is larger than the queue size.
 * Input parameters:
 *   mpo_obj            - The MPO Decoder object.
 *   p_output_buf_array - The pointer to the buffer array.
 *   list_count         - The number of buffers being passed.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_enqueue_output_buf(
    mpod_obj_t          mpo_obj,
    mpod_output_buf_t  *p_output_buf_array,
    uint32_t            list_count)
{
    int             rc;
    mpo_decoder_t *p_mpo_decoder = (mpo_decoder_t *)mpo_obj;
    if (!p_mpo_decoder)
        return JPEGERR_EUNINITIALIZED;

    // Input validation
    if (!p_output_buf_array)
       return JPEGERR_ENULLPTR;

    // Enqueue the output buffer
    rc = jpeg_queue_enqueue(p_mpo_decoder->p_jpeg_decoder->processor.output_queue,
                            (void **)(&p_output_buf_array),
                            list_count);
    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpod_start
 * Description:
 * Input parameters:
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_start(
    mpod_obj_t          mpod_obj,
    mpod_cfg_t         *p_cfg,
    mpod_dst_t         *p_dest,
    mpod_output_buf_t  *p_output_buf_array,
    uint32_t            list_count)
{
    jpegd_obj_t    jpegd_obj;
    mpo_decoder_t *p_mpo_decoder;
    int            rc;

    p_mpo_decoder = (mpo_decoder_t *)mpod_obj;
    if (!p_mpo_decoder)
    {
        return JPEGERR_EUNINITIALIZED;
    }
    jpegd_obj = (jpegd_obj_t)p_mpo_decoder->p_jpeg_decoder;

    rc = jpegd_start(jpegd_obj,
                     (jpegd_cfg_t *)p_cfg,
                     (jpegd_dst_t *)p_dest,
                     (jpegd_output_buf_t *)p_output_buf_array,
                     list_count);

    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpod_abort
 * Description: Aborts the decoding session in progress. If there is no decoding
 *              session in progress, a JPEGERR_SUCCESS will be returned
 *              immediately. Otherwise, the call will block until the decoding
 *              is completely aborted, then return a JPEGERR_SUCCESS.
 * Input parameters:
 *   mpo_obj    - The MPO Decoder object
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_abort(mpod_obj_t mpod_obj)
{
    jpegd_obj_t    jpegd_obj;
    mpo_decoder_t *p_mpo_decoder;
    int            rc;

    p_mpo_decoder = (mpo_decoder_t *)mpod_obj;
    if (!p_mpo_decoder)
    {
        return JPEGERR_EUNINITIALIZED;
    }
    jpegd_obj = (jpegd_obj_t)p_mpo_decoder->p_jpeg_decoder;

    rc = mpod_abort(mpod_obj);

    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpod_destroy
 * Description: Destroys the MPO Decoder object, releasing memory previously
 *              allocated by MPO Decoder.
 * Input parameters:
 *   p_mpod_obj  - The pointer to the MPO Decoder object
 * Return value: None
 * Notes: none
 *****************************************************************************/
void mpod_destroy(mpod_obj_t *p_mpod_obj)
{
    if (p_mpod_obj)
    {
        mpo_decoder_t *p_mpod = (mpo_decoder_t *)(*p_mpod_obj);
        if (p_mpod)
        {
            // Destroy MPO decoder
            mpo_header_destroy(&p_mpod->mpo_reader.mpo_header);

            // Destroy Jpeg decoder
            jpegd_destroy((jpegd_obj_t)&(p_mpod->p_jpeg_decoder));

            STD_MEMSET(p_mpod, 0, sizeof(mpo_decoder_t));

            JPEG_FREE(p_mpod);
            *p_mpod_obj = NULL;
        }

        jpeg_show_leak();
    }
}

/******************************************************************************
 * Function: mpod_get_mp_entry_value
 * Description: Obtains the MP entry values.
 * Input parameters:
 *   mpod_obj      - The MPO decoder object.
 *   p_mp_entry    - The pointer to the MP entry structure.
 *   num_image     - The number of images in the MPO file.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EUNINITIALIZED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_get_mp_entry_value(mpod_obj_t mpod_obj, mp_entry_t *p_mp_entry, uint32_t num_image)
{
    mpo_index_ifd_t *p_index; // pointer to the index ifd
    uint32_t attribute;       // MP entry attribute
    uint32_t i;               // loop counter

    // Cast the input mpo decoder object
    mpo_decoder_t  *p_mpo_decoder = (mpo_decoder_t *)mpod_obj;
    if (!p_mpo_decoder)
    {
        return JPEGERR_EUNINITIALIZED;
    }

    p_index = p_mpo_decoder->p_mpo_full_hdr->p_index_ifd;

    for (i = 0; i < num_image; i++)
    {
        attribute = mpod_get_4bytes(p_index->p_mp_entry->entry.data._ascii + i * 16,
                                    p_mpo_decoder->mpo_reader.mp_endian);

        // 4-byte individual image attribute
        p_mp_entry[i].attribute.dependent      = DEPENDENT(attribute);      // Parent or child image flag
        p_mp_entry[i].attribute.representative = REPRESENTATIVE(attribute); // Representative flag
        p_mp_entry[i].attribute.format         = JPEG(attribute);           // MPO image format
        p_mp_entry[i].attribute.type           = TYPE(attribute);           // MPO type

        // 4-byte individual image size
        p_mp_entry[i].image_size               = mpod_get_4bytes(p_index->p_mp_entry->entry.data._ascii + i * 16 + 4,
                                                                 p_mpo_decoder->mpo_reader.mp_endian);

        // 4-byte individual image data offset
        p_mp_entry[i].data_offset              = mpod_get_4bytes(p_index->p_mp_entry->entry.data._ascii + i * 16 + 8,
                                                                 p_mpo_decoder->mpo_reader.mp_endian);

        // 2-byte dependent image 1 entry number
        p_mp_entry[i].dep_image1_entry_num     = mpod_get_2bytes(p_index->p_mp_entry->entry.data._ascii + i * 16 + 12,
                                                                 p_mpo_decoder->mpo_reader.mp_endian);

        // 2-byte dependent image 2 entry number
        p_mp_entry[i].dep_image2_entry_num     = mpod_get_2bytes(p_index->p_mp_entry->entry.data._ascii + i * 16 + 14,
                                                                 p_mpo_decoder->mpo_reader.mp_endian);

    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpod_get_num_image
 * Description: Retrives the number of images in the MPO file.
 * Input parameters:
 *   mpod_obj     - The MPO decoder object.
 *   p_num_image  - The pointer to the number of images in the MPO file.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_get_num_image(mpod_obj_t mpod_obj, uint32_t *p_num_image)
{
    // Cast the input mpo decoder object
    mpo_decoder_t  *p_mpo_decoder = (mpo_decoder_t *)mpod_obj;
    if (!p_mpo_decoder)
    {
        return JPEGERR_EUNINITIALIZED;
    }

    // Obtain the number of images saved in the MPO index object
    *p_num_image = p_mpo_decoder->p_mpo_full_hdr->p_index_ifd->p_number_of_images->entry.data._long;

    // MPO files contains at least 2 images
    if (*p_num_image < 2)
    {
        return JPEGERR_EFAILED;
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpod_get_mp_entry_value
 * Description: Reset the starting address in the input buffer so that it
 *              points to the beginning of the next individual image.
 * Input parameters:
 *   mpod_obj     - The MPO decoder object.
 *   data_offset - The offset to the beginning of the next individual image.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EUNINITIALIZED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_set_input_offset(mpod_obj_t mpod_obj, uint32_t data_offset)
{
    // Cast the input mpo decoder object
    mpo_decoder_t  *p_mpo_decoder = (mpo_decoder_t *)mpod_obj;
    if (!p_mpo_decoder)
    {
        return JPEGERR_EUNINITIALIZED;
    }

    // mpo_start_offset is the global reference for all the offsets in the MPO file
    p_mpo_decoder->p_jpeg_decoder->reader.next_byte_offset = p_mpo_decoder->mpo_reader.start_offset + data_offset;

    return JPEGERR_SUCCESS;
}

// Helper functions
uint16_t mpod_get_2bytes(uint8_t *p_input, exif_endianness_t endianness)
{
    uint8_t byte1 = *p_input;
    uint8_t byte2 = *(p_input + 1);

    return (endianness == EXIF_BIG_ENDIAN) ?
    ((byte1 << 8) + byte2) :
    ((byte2 << 8) + byte1);
}

uint32_t mpod_get_4bytes(uint8_t *p_input, exif_endianness_t endianness)
{
    uint8_t byte1 = *p_input;
    uint8_t byte2 = *(p_input + 1);
    uint8_t byte3 = *(p_input + 2);
    uint8_t byte4 = *(p_input + 3);

    return (endianness == EXIF_BIG_ENDIAN) ?
    ((byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4) :
    ((byte4 << 24) + (byte3 << 16) + (byte2 << 8) + byte1);
}
