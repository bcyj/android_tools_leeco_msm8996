/*========================================================================


*//** @file jpeg_reader.c

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
02/03/11   bapu    Changing the implementation of logic for overflow and
                   maintaining consistency during error return.
01/22/11   mingy   Updated APP3 processing.
01/05/11   bapu    Resolving Klockwork errors
01/05/11   bapu    Checking the limits for cnt for memory allocation during
                   tag retrieval
12/06/10   sigu    Nullify the ptr to avoid double freeing it.
12/01/10   bapu    Fixed compiler warnings
07/22/10   sigu    Fixed compiler warnings
07/01/10   mingy   Added p_htables array bound checking.
02/12/10   mingy   Removed lint errors.
08/03/09   vma     Fixed the bug in jpegr_fetch_tag regarding fetching
                   of EXIF_SHORT data
06/24/09   mingy   Fixed the bug in process SOS function when performing
                   validation check on p_scan_info->succ_approx_l.
05/21/09   mingy   Modified the component starting index from 1 to 0.
11/20/08   vma     Fixed logic bug in jpegr_fetch_nbytes().
10/08/08   vma     Reset endianess during parsing of thumbnail in 1st IFD
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_reader.h"
#include "jpeg_buffer_private.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "exif_private.h"
#include "jpeg_header.h"
#include <stdlib.h>

//TBD
#include "writer_utility.h"

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
int jpegr_process_sof(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info,
                      jpeg_marker_t marker);
int jpegr_process_sos(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info);
int jpegr_find_soi(jpeg_reader_t *p_reader);
int jpegr_process_dht(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info);
int jpegr_process_dqt(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info);
int jpegr_process_dri(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info);
int jpegr_process_app0(jpeg_reader_t *p_reader);
int jpegr_process_app1(jpeg_reader_t *p_reader);
int jpegr_process_app3(jpeg_reader_t *p_reader);
int jpegr_parse_sof(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info);
jpeg_marker_t jpegr_find_next_header(jpeg_reader_t *p_reader);
int jpegr_process_zero_ifd (jpeg_reader_t *p_reader);
int jpegr_process_exif_ifd (jpeg_reader_t *p_reader);
int jpegr_process_gps_ifd  (jpeg_reader_t *p_reader);
int jpegr_process_first_ifd(jpeg_reader_t *p_reader);

int jpegr_fetch_tag(jpeg_reader_t        *p_reader,
                    exif_tag_entry_ex_t **pp_tag_entry,
                    uint16_t              expected_type,
                    exif_tag_id_t         tag_id);
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define EXIF_BYTE      1
#define EXIF_ASCII     2
#define EXIF_SHORT     3
#define EXIF_LONG      4
#define EXIF_RATIONAL  5
#define EXIF_UNDEFINED 7
#define EXIF_SLONG     9
#define EXIF_SRATIONAL 10

// The number of bytes the parser attempts to fetch each time
#define MAX_BYTES_TO_FETCH   0x2000

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
int jpegr_init(jpeg_reader_t             *p_reader,
               jpegd_obj_t                decoder,
               jpegd_src_t               *p_source,
               jpegr_input_req_handler_t  p_input_req_handler)
{
    // Validate inputs
    if (!p_reader)
        return JPEGERR_ENULLPTR;

    // Zero all fields in the structure
    STD_MEMSET(p_reader, 0, sizeof(jpeg_reader_t));
    p_reader->decoder  = decoder;
    p_reader->p_source = p_source;
    p_reader->p_input_buf = p_source->buffers[0];
    p_reader->p_input_req_handler = p_input_req_handler;

    return JPEGERR_SUCCESS;
}

void jpegr_destroy(jpeg_reader_t *p_reader)
{
    JPEG_DBG_LOW("jpegr_destroy\n");
    jpeg_header_destroy(&p_reader->header);
}

int jpegr_request_input(jpeg_reader_t *p_reader)
{
    if (p_reader->error_flag)
    {
        return JPEGERR_EFAILED;
    }

    // See if is necessary to make a request
    while (p_reader->next_byte_offset >= p_reader->input_start_offset + p_reader->p_input_buf->offset ||
           p_reader->next_byte_offset < p_reader->input_start_offset)
    {
        // Update input start offset
        p_reader->input_start_offset = p_reader->next_byte_offset;

        jpeg_buffer_mark_empty((jpeg_buf_t *)p_reader->p_source->buffers[0]);
        // Request input
        p_reader->p_input_req_handler(p_reader->decoder,
                                      p_reader->p_input_buf,
                                      p_reader->next_byte_offset,
                                      MAX_BYTES_TO_FETCH);

        // Wait for buffer to be delivered asynchronously by the user
        jpeg_buffer_wait_until_filled((jpeg_buf_t *)p_reader->p_source->buffers[0]);

        // If no bytes were read, flag it
        if (p_reader->p_input_buf->offset == 0)
        {
            JPEG_DBG_LOW("jpegr_request_input: 0 bytes fetched, flaged as error\n");
            return JPEGERR_EFAILED;
        }
    }
    return JPEGERR_SUCCESS;
}

uint8_t jpegr_fetch_byte(jpeg_reader_t *p_reader)
{
    uint8_t byte_fetched;

    // Request buffer if necessary
    if (JPEG_FAILED(jpegr_request_input(p_reader)))
    {
        p_reader->error_flag = true;
        return 0;
    }

    byte_fetched = *(p_reader->p_input_buf->ptr +
                     (p_reader->next_byte_offset - p_reader->input_start_offset));
    p_reader->next_byte_offset++;
    return byte_fetched;
}

uint16_t jpegr_fetch_2bytes(jpeg_reader_t *p_reader)
{
    uint8_t byte1 = jpegr_fetch_byte(p_reader);
    uint8_t byte2 = jpegr_fetch_byte(p_reader);

    return(p_reader->endianness == EXIF_BIG_ENDIAN) ?
    ((byte1 << 8) + byte2) :
    ((byte2 << 8) + byte1);
}

uint32_t jpegr_fetch_4bytes(jpeg_reader_t *p_reader)
{
    uint8_t byte1 = jpegr_fetch_byte(p_reader);
    uint8_t byte2 = jpegr_fetch_byte(p_reader);
    uint8_t byte3 = jpegr_fetch_byte(p_reader);
    uint8_t byte4 = jpegr_fetch_byte(p_reader);

    return(p_reader->endianness == EXIF_BIG_ENDIAN) ?
    ((byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4) :
    ((byte4 << 24) + (byte3 << 16) + (byte2 << 8) + byte1);
}

void jpegr_fetch_nbytes(jpeg_reader_t *p_reader, uint8_t *p_dest,
                        uint32_t bytes_to_fetch, uint32_t *p_bytes_fetched)
{
    uint32_t bytes_fetched = 0;

    while (bytes_fetched < bytes_to_fetch)
    {
        uint32_t bytes_to_copy;
        // Request buffer if necessary
        if (JPEG_FAILED(jpegr_request_input(p_reader)))
        {
            return;
        }

        // Compute how many bytes should be copied
        bytes_to_copy = STD_MIN ((bytes_to_fetch - bytes_fetched),
                                 ((p_reader->p_input_buf->offset +
                                   p_reader->input_start_offset) - p_reader->next_byte_offset));

        // Copy
        STD_MEMMOVE(p_dest, p_reader->p_input_buf->ptr +
                    (p_reader->next_byte_offset - p_reader->input_start_offset),
                    bytes_to_copy);
        p_dest += bytes_to_copy;
        p_reader->next_byte_offset += bytes_to_copy;
        bytes_fetched += bytes_to_copy;
    }
    if (p_bytes_fetched)
        *p_bytes_fetched = bytes_fetched;
}

jpeg_marker_t jpegr_find_next_header(jpeg_reader_t *p_reader)
{
    uint32_t byte1;

    do
    {
        do
        {
            byte1 = jpegr_fetch_byte(p_reader);
            if (p_reader->error_flag)
            {
                return M_EOI;
            }

        } while (byte1 != 0xFF);

        do
        {
            byte1 = jpegr_fetch_byte(p_reader);
            if (p_reader->error_flag)
            {
                return M_EOI;
            }

        } while (byte1 == 0xFF);

    } while (byte1 == 0);

    // save to cast to uint8_t
    return (jpeg_marker_t)byte1;
}

jpeg_header_t *jpegr_read_header(jpeg_reader_t *p_reader)
{
    int rc;

    // Make sure the previously stored header is destroyed first
    jpeg_header_destroy(&p_reader->header);

    // Find SOI marker
    rc = jpegr_find_soi(p_reader);

    if (JPEG_SUCCEEDED(rc))
    {
        p_reader->header.p_main_frame_info = (jpeg_frame_info_t *)JPEG_MALLOC(sizeof(jpeg_frame_info_t));
        if (!p_reader->header.p_main_frame_info)
        {
            rc = JPEGERR_EMALLOC;
        }
        else
        {
            STD_MEMSET(p_reader->header.p_main_frame_info, 0, sizeof(jpeg_frame_info_t));
        }
    }
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpegr_parse_sof(p_reader, p_reader->header.p_main_frame_info);
    }
    if (JPEG_FAILED(rc))
    {
        jpeg_header_destroy(&p_reader->header);
        return NULL;
    }
    jpeg_dump_header(&p_reader->header);
    return &p_reader->header;
}

void jpegr_reset(jpeg_reader_t *p_reader)
{
    if (p_reader->p_input_buf)
    {
        jpeg_buffer_mark_empty(p_reader->p_input_buf);
    }
    p_reader->endianness = EXIF_BIG_ENDIAN;
    p_reader->tiff_hdr_offset = 0;
    p_reader->exif_ifd_offset = 0;
    p_reader->gps_ifd_offset = 0;
    p_reader->first_ifd_offset = 0;
    p_reader->interop_ifd_offset = 0;
}

/**
 * This method parses the file until the first SOS encountered is processed.
 */
int jpegr_parse_sof(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info)
{
    int rc;
    for (;;)
    {
        jpeg_marker_t marker = jpegr_find_next_header(p_reader);

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
            rc = jpegr_process_sof(p_reader, p_frame_info, marker);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_SOI:
            // not expected to see SOI again, return failure
            return JPEGERR_EFAILED;
        case M_EOI:
            // not expected to see EOI, return failure
            return JPEGERR_EFAILED;
        case M_SOS:
            rc = jpegr_process_sos(p_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            return JPEGERR_SUCCESS;
        case M_DHT:
            rc = jpegr_process_dht(p_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_DQT:
            rc = jpegr_process_dqt(p_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_DRI:
            rc = jpegr_process_dri(p_reader, p_frame_info);
            if (JPEG_FAILED (rc)) return rc;
            break;
        case M_APP0:
            // Even failed to process APP0, still continue since
            // it is not mandatory
            (void)jpegr_process_app0(p_reader);
            break;
        case M_APP1:
            // Even failed to process APP1, still continue since
            // it is not mandatory
            (void)jpegr_process_app1(p_reader);
            p_reader->endianness = EXIF_BIG_ENDIAN;
            break;
        case M_APP3:
            // Even failed to process APP3, still continue since
            // it is not mandatory
            (void)jpegr_process_app3(p_reader);
            break;

        default:
            {
                // Skip unsupported marker
                uint16_t bytesToSkip = jpegr_fetch_2bytes(p_reader);

                if (bytesToSkip >= 2)
                    bytesToSkip -= 2;

                JPEG_DBG_LOW("Skipping unsupported marker (FF%X)...", marker);
                p_reader->next_byte_offset += bytesToSkip;
                break;
            }
        }
    }
}

int jpegr_find_soi(jpeg_reader_t *p_reader)
{
    jpeg_marker_t m1, m2;
    uint32_t cnt = 256;

    m1 = (jpeg_marker_t)jpegr_fetch_byte(p_reader);
    m2 = (jpeg_marker_t)jpegr_fetch_byte(p_reader);

    if ((m1 == (jpeg_marker_t) 0xFF) && (m2 == M_SOI))
    {
        return JPEGERR_SUCCESS;
    }
    // not found immediately, continuing searching for another 256 bytes
    while (cnt)
    {
        m1 = m2;
        m2 = (jpeg_marker_t)jpegr_fetch_byte(p_reader);
        if ( (m1 == (jpeg_marker_t) 0xFF) && (m2 == M_SOI))
        {
            return JPEGERR_SUCCESS;
        }
        cnt--;
    }
    return JPEGERR_EFAILED;
}

int jpegr_process_sof(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info, jpeg_marker_t marker)
{
    uint32_t i, len, comp_infos_len;

    switch (marker)
    {
    case M_SOF0:
        p_frame_info->process = JPEG_PROCESS_BASELINE;
        break;
    case M_SOF1:
        p_frame_info->process = JPEG_PROCESS_EXTENDED_HUFFMAN;
        break;
    case M_SOF2:
        p_frame_info->process = JPEG_PROCESS_PROGRESSIVE_HUFFMAN;
        break;
    case M_SOF3:
        p_frame_info->process = JPEG_PROCESS_LOSSLESS_HUFFMAN;
        break;
    case M_SOF5:
        p_frame_info->process = JPEG_PROCESS_DIFF_SEQ_HUFFMAN;
        break;
    case M_SOF6:
        p_frame_info->process = JPEG_PROCESS_DIFF_PROG_HUFFMAN;
        break;
    case M_SOF7:
        p_frame_info->process = JPEG_PROCESS_DIFF_LOSSLESS_HUFFMAN;
        break;
    case M_SOF9:
        p_frame_info->process = JPEG_PROCESS_SEQ_ARITHMETIC;
        break;
    case M_SOF10:
        p_frame_info->process = JPEG_PROCESS_PROGRESSIVE_ARITHMETIC;
        break;
    case M_SOF11:
        p_frame_info->process = JPEG_PROCESS_LOSSLESS_ARITHMETIC;
        break;
    case M_SOF13:
        p_frame_info->process = JPEG_PROCESS_DIFF_SEQ_ARITHMETIC;
        break;
    case M_SOF14:
        p_frame_info->process = JPEG_PROCESS_DIFF_PROG_ARITHMETIC;
        break;
    case M_SOF15:
        p_frame_info->process = JPEG_PROCESS_DIFF_LOSSLESS_ARITHMETIC;
        break;

    default:
        return JPEGERR_EUNSUPPORTED;
    }

    // Get length of the compoent infos
    len = jpegr_fetch_2bytes(p_reader);

    // Get precision
    p_frame_info->precision = jpegr_fetch_byte(p_reader);

    // Get height
    p_frame_info->height = jpegr_fetch_2bytes(p_reader);

    // Get width
    p_frame_info->width = jpegr_fetch_2bytes(p_reader);

    // Get number of components in frame
    comp_infos_len = jpegr_fetch_byte(p_reader);

    // Validation on number of components
    if (comp_infos_len == 0 || comp_infos_len > 4 ||
        len != (8 + comp_infos_len * 3))
        return JPEGERR_EFAILED;

    // Get component infos
    p_frame_info->p_comp_infos = (jpeg_comp_info_t *)JPEG_MALLOC(comp_infos_len *
                                                                 sizeof(jpeg_comp_info_t));
    if (!p_frame_info->p_comp_infos)
    {
        return JPEGERR_EMALLOC;
    }
    STD_MEMSET(p_frame_info->p_comp_infos, 0, comp_infos_len * sizeof(jpeg_comp_info_t));

    for (i = 0; i < comp_infos_len; i++)
    {
        uint8_t temp_byte;
        jpeg_comp_info_t *p_info = p_frame_info->p_comp_infos + i;

        p_info->comp_id = jpegr_fetch_byte(p_reader);
        temp_byte = jpegr_fetch_byte(p_reader);
        p_info->sampling_h = temp_byte >> 4;
        p_info->sampling_v = temp_byte & 0x0F;
        p_info->qtable_sel = jpegr_fetch_byte(p_reader);

        if (p_info->qtable_sel > 3) return JPEGERR_EFAILED;
    }
    p_frame_info->num_comps = comp_infos_len;

    // Derive subsampling information
    if (comp_infos_len == 1)
    {
        p_frame_info->subsampling = JPEG_GRAYSCALE;
    }
    else if (comp_infos_len == 3)
    {
        uint8_t subsampling = 3; // JPEG_H1V1
        if (p_frame_info->p_comp_infos[0].sampling_h == 2)
        {
            subsampling -= 2;
        }
        if (p_frame_info->p_comp_infos[0].sampling_v == 2)
        {
            subsampling -= 1;
        }
        p_frame_info->subsampling = (jpeg_subsampling_t)subsampling;
    }
    else
    {
        // Unsupprted number of components
        JPEG_DBG_LOW("jpegr_process_sof: unexpected number of components\n");
        return JPEGERR_EUNSUPPORTED;
    }

    return JPEGERR_SUCCESS;
}

int jpegr_process_sos(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info)
{
    int rc = JPEGERR_SUCCESS;
    uint8_t temp_byte, j;
    uint32_t len, comp_sels_len, i;
    jpeg_scan_info_t *p_scan_info = jpeg_add_scan_info(p_frame_info);

    if (!p_scan_info) return JPEGERR_EMALLOC;

    len = jpegr_fetch_2bytes(p_reader);

    p_scan_info->offset = p_reader->next_byte_offset + (len - 2);

    comp_sels_len = jpegr_fetch_byte(p_reader);

    if (comp_sels_len != 1 && comp_sels_len != 3)
    {
        JPEG_DBG_LOW("jpegr_process_sos: unexpected number of component selectors: %d\n", comp_sels_len);
        return JPEGERR_EFAILED;
    }

    if (len != 6 + comp_sels_len * 2)
    {
        JPEG_DBG_LOW("jpegr_process_sos: length not match with number of component selectors\n");
        return JPEGERR_EFAILED;
    }

    p_scan_info->p_selectors = (jpeg_comp_entropy_sel_t *)JPEG_MALLOC(comp_sels_len *
                                                                      sizeof(jpeg_comp_entropy_sel_t));
    if (!p_scan_info->p_selectors)
    {
        return JPEGERR_EMALLOC;
    }

    for (i = 0; i < comp_sels_len; i++)
    {
        p_scan_info->p_selectors[i].comp_id = jpegr_fetch_byte(p_reader);
        temp_byte = jpegr_fetch_byte(p_reader);
        p_scan_info->p_selectors[i].dc_selector = temp_byte >> 4;
        p_scan_info->p_selectors[i].ac_selector = temp_byte & 0x0F;

        // Map the component Id to the index to the component Info
        for (j = 0; j < p_frame_info->num_comps; j++)
        {
            if (p_scan_info->p_selectors[i].comp_id == p_frame_info->p_comp_infos[j].comp_id)
            {
                p_scan_info->p_selectors[i].comp_id = j;
                break;
            }
        }

        // No corresponding Id found
        if (j == p_frame_info->num_comps ||
            p_scan_info->p_selectors[i].dc_selector >= 4 ||
            p_scan_info->p_selectors[i].ac_selector >= 4)
        {
            rc = JPEGERR_EFAILED;
        }

        // Make sure the tables are present
        if (!(p_frame_info->htable_present_flag &
              (1 << p_scan_info->p_selectors[i].dc_selector)) ||
            !(p_frame_info->htable_present_flag &
              (1 << p_scan_info->p_selectors[i].ac_selector)))
        {
            JPEG_DBG_MED("jpegr_process_sos: invalid entropy table selector\n");
            return JPEGERR_EFAILED;
        }
    }

    if (JPEG_SUCCEEDED(rc))
    {
        p_scan_info->spec_start = jpegr_fetch_byte(p_reader);
        p_scan_info->spec_end   = jpegr_fetch_byte(p_reader);
        temp_byte = jpegr_fetch_byte(p_reader);
        p_scan_info->succ_approx_h = temp_byte >> 4;
        p_scan_info->succ_approx_l = temp_byte & 0x0F;
        p_scan_info->num_selectors = comp_sels_len;

        // Validation
        if (!p_frame_info->qtable_present_flag || // make sure DQT is previously processed
            !p_frame_info->htable_present_flag || // make sure DHT is previously processed
            p_scan_info->spec_start >= 64 || p_scan_info->spec_end >= 64 ||
            p_scan_info->succ_approx_h >= 14 || p_scan_info->succ_approx_l >= 14)
        {
            rc = JPEGERR_EFAILED;
        }
    }

    return rc;
}

int jpegr_process_dqt(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info)
{
    uint32_t i, j, length, dest;

    length = jpegr_fetch_2bytes(p_reader);

    for (i = 0; i < length / 64; i++)
    {
        uint8_t temp_byte = jpegr_fetch_byte(p_reader);
        p_frame_info->quant_precision = temp_byte >> 4;
        dest = temp_byte & 0x0F;

        if (p_frame_info->quant_precision > 1 || dest >= 4)
            return JPEGERR_EFAILED;

        // Set the bit flag to indicate which table are read
        p_frame_info->qtable_present_flag |= (1 << dest);

        // Allocate qtable
        if (!p_frame_info->p_qtables[dest])
        {
            p_frame_info->p_qtables[dest] = (jpeg_quant_table_t)JPEG_MALLOC(64 * sizeof(uint16_t));
            if (!p_frame_info->p_qtables[dest])
            {
                return JPEGERR_EMALLOC;
            }
        }
        for (j = 0; j < 64; j++)
        {
            p_frame_info->p_qtables[dest][j] = (p_frame_info->quant_precision) ?
                jpegr_fetch_2bytes(p_reader) : jpegr_fetch_byte(p_reader);
        }

        // validation
        if (2 + 65 + 64 * (uint32_t) p_frame_info->quant_precision > length)
        {
            return JPEGERR_EFAILED;
        }
    }
    return JPEGERR_SUCCESS;
}

int jpegr_process_dht(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info)
{
    uint32_t i, sum, index, size;

    // Base on Table B.5 of spec. to read in the Huffman table
    // Fetch the size of this chunk
    size = jpegr_fetch_2bytes(p_reader);
    size -= 2;            /* not include the size */

    while (size > 16)
    {
        index = jpegr_fetch_byte(p_reader);
        if (index & 0xEC)
        {
            // Bad DHT table class and destination
            return JPEGERR_EFAILED;
        }
        if (index & 0x10)
            index = (index & 0x3) + 4;

        // Set the bit flag to indicate which table are read
        p_frame_info->htable_present_flag |= (1 << index);

        if (index >= JPEGD_MAXHUFFTABLES)
        {
            // Make sure the buffer index is within range
            return JPEGERR_EFAILED;
        }

        p_frame_info->p_htables[index].bits[0] = 0;
        sum = 0;

        for (i = 1; i < 17; i++)
        {
            p_frame_info->p_htables[index].bits[i] = jpegr_fetch_byte(p_reader);
            sum += p_frame_info->p_htables[index].bits[i];
        }

        if (sum > 256)
        {
            // Bad DHT cnt
            return JPEGERR_EFAILED;
        }

        for (i = 0; i < sum; i++)
            p_frame_info->p_htables[index].values[i] = jpegr_fetch_byte(p_reader);

        /* subtract the number of BITS and the BITS total count */
        size -= (17 + sum);
    }

    return JPEGERR_SUCCESS;
}

int jpegr_process_dri(jpeg_reader_t *p_reader, jpeg_frame_info_t *p_frame_info)
{
    uint32_t len = jpegr_fetch_2bytes(p_reader);

    if (len != 4)
        return JPEGERR_EFAILED;

    p_frame_info->restart_interval = jpegr_fetch_2bytes(p_reader);
    return JPEGERR_SUCCESS;
}

int jpegr_process_app0(jpeg_reader_t *p_reader)
{
    int rc = JPEGERR_SUCCESS;
    uint32_t app0_start_offset = p_reader->next_byte_offset;
    uint32_t len = jpegr_fetch_2bytes(p_reader);

    // Create the JFIF info if not already exists
    if (!p_reader->header.p_exif_info)
    {
        p_reader->header.p_exif_info = (exif_info_t *)JPEG_MALLOC(sizeof(exif_info_t));
        if (!p_reader->header.p_exif_info)
            rc = JPEGERR_EMALLOC;
        else
            STD_MEMSET(p_reader->header.p_exif_info, 0, sizeof(exif_info_t));
    }

    // Match with "JFIF"
    if (JPEG_SUCCEEDED(rc))
        rc = (jpegr_fetch_4bytes(p_reader) == 0x4A464946) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;

    if (JPEG_SUCCEEDED(rc))
    {
        // Skip the padded bits 0x00
        (void) jpegr_fetch_byte(p_reader);
    }

    if (JPEG_SUCCEEDED (rc))
    {
        // Match the JFIF Version
        rc = (jpegr_fetch_2bytes(p_reader) <= 0x0102) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;
    }

    if (JPEG_SUCCEEDED (rc))
    {
        // resolution unit
        //rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_resolution_unit, EXIF_SHORT, EXIFTAGID_RESOLUTION_UNIT);
        exif_tag_entry_ex_t *p_new_tag;

        exif_destroy_tag_entry(p_reader->header.p_exif_info->tiff_ifd.p_resolution_unit);

        p_new_tag = exif_create_tag_entry();

        if (!p_new_tag)
            return JPEGERR_EMALLOC;

        p_new_tag->entry.data._short = jpegr_fetch_byte(p_reader) + 1;

        if (JPEG_SUCCEEDED(rc))
        {
            p_new_tag->entry.copy = true;
            p_new_tag->entry.count = 1;
            p_new_tag->entry.type = (exif_tag_type_t)EXIF_SHORT;
            p_new_tag->tag_id = EXIFTAGID_RESOLUTION_UNIT;
            p_reader->header.p_exif_info->tiff_ifd.p_resolution_unit = p_new_tag;
        }
        else
        {
            exif_destroy_tag_entry(p_new_tag);
        }

    }

    if (JPEG_SUCCEEDED (rc))
    {
        exif_tag_entry_ex_t *p_new_tag;

        exif_destroy_tag_entry(p_reader->header.p_exif_info->tiff_ifd.p_x_resolution);

        p_new_tag = exif_create_tag_entry();

        if (!p_new_tag)
            return JPEGERR_EMALLOC;

        /*p_new_tag->entry.data._rats = (rat_t *)JPEG_MALLOC(sizeof(rat_t));
        if (!p_new_tag->entry.data._rats)
        {
            rc = JPEGERR_EMALLOC;
        }
        else */
        {
            p_new_tag->entry.data._rat.num   = (uint32_t)jpegr_fetch_2bytes(p_reader);
            p_new_tag->entry.data._rat.denom = 1;
        }

        if (JPEG_SUCCEEDED(rc))
        {
            p_new_tag->entry.copy = true;
            p_new_tag->entry.count = 1;
            p_new_tag->entry.type = (exif_tag_type_t)EXIF_RATIONAL;
            p_new_tag->tag_id = EXIFTAGID_X_RESOLUTION;
            p_reader->header.p_exif_info->tiff_ifd.p_x_resolution = p_new_tag;
        }
        else
        {
            exif_destroy_tag_entry(p_new_tag);
        }
    }

    if (JPEG_SUCCEEDED (rc))
    {
        exif_tag_entry_ex_t *p_new_tag;

        exif_destroy_tag_entry(p_reader->header.p_exif_info->tiff_ifd.p_y_resolution);

        p_new_tag = exif_create_tag_entry();

        if (!p_new_tag)
            return JPEGERR_EMALLOC;

        /*p_new_tag->entry.data._rats = (rat_t *)JPEG_MALLOC(sizeof(rat_t));
        if (!p_new_tag->entry.data._rats)
        {
            rc = JPEGERR_EMALLOC;
        }
        else */
        {
            p_new_tag->entry.data._rat.num   = (uint32_t)jpegr_fetch_2bytes(p_reader);
            p_new_tag->entry.data._rat.denom = 1;
        }

        if (JPEG_SUCCEEDED(rc))
        {
            p_new_tag->entry.copy = true;
            p_new_tag->entry.count = 1;
            p_new_tag->entry.type = (exif_tag_type_t)EXIF_RATIONAL;
            p_new_tag->tag_id = EXIFTAGID_Y_RESOLUTION;
            p_reader->header.p_exif_info->tiff_ifd.p_y_resolution = p_new_tag;
        }
        else
        {
            exif_destroy_tag_entry(p_new_tag);
        }
    }

    if (JPEG_FAILED(rc))
    {
        exif_info_obj_t exif_info_obj = (exif_info_obj_t)p_reader->header.p_exif_info;
        exif_destroy(&exif_info_obj);
        p_reader->header.p_exif_info = NULL;
    }

    // skip to end of APP0
    p_reader->next_byte_offset = app0_start_offset + len;
    return rc;

}



int jpegr_process_app1(jpeg_reader_t *p_reader)
{
    int rc = JPEGERR_SUCCESS;
    uint32_t app1_start_offset = p_reader->next_byte_offset;
    uint32_t len = jpegr_fetch_2bytes(p_reader);
    uint32_t nEndianness = 0;

    // Create the exif info if not already exists
    if (!p_reader->header.p_exif_info)
    {
        p_reader->header.p_exif_info = (exif_info_t *)JPEG_MALLOC(sizeof(exif_info_t));
        if (!p_reader->header.p_exif_info)
            rc = JPEGERR_EMALLOC;
        else
            STD_MEMSET(p_reader->header.p_exif_info, 0, sizeof(exif_info_t));
    }

    // Match with "Exif"
    if (JPEG_SUCCEEDED(rc))
        rc = (jpegr_fetch_4bytes(p_reader) == 0x45786966) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;

    if (JPEG_SUCCEEDED(rc))
    {
        // Skip the padded bits
        (void)jpegr_fetch_2bytes(p_reader);

        // Store TIFF header offset
        p_reader->tiff_hdr_offset = p_reader->next_byte_offset;

        // nEndianness (Big-Endian = 0x4D4D; Little-Endian = 0x4949)
        nEndianness = jpegr_fetch_2bytes(p_reader);
        rc = (nEndianness == 0x4d4d || nEndianness == 0x4949) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;
    }

    if (JPEG_SUCCEEDED(rc))
    {
        p_reader->endianness = (nEndianness == 0x4d4d) ? EXIF_BIG_ENDIAN : EXIF_LITTLE_ENDIAN;

        // Match the TIFF Version
        rc = (jpegr_fetch_2bytes(p_reader) == 0x2A) ? JPEGERR_SUCCESS : JPEGERR_EFAILED;
    }

    if (JPEG_SUCCEEDED(rc))
        rc = jpegr_process_zero_ifd(p_reader);

    if (JPEG_SUCCEEDED(rc))
        rc = jpegr_process_exif_ifd(p_reader);

    if (JPEG_SUCCEEDED(rc))
        rc = jpegr_process_gps_ifd(p_reader);

    if (JPEG_SUCCEEDED(rc))
        rc = jpegr_process_first_ifd(p_reader);

    if (JPEG_FAILED(rc))
    {
        exif_info_obj_t exif_info_obj = (exif_info_obj_t)p_reader->header.p_exif_info;
        exif_destroy(&exif_info_obj);
        p_reader->header.p_exif_info = NULL;
    }

    p_reader->next_byte_offset = app1_start_offset + len;
    return rc;
}


int jpegr_process_app3(jpeg_reader_t *p_reader)
{
    int rc = JPEGERR_SUCCESS;
    uint32_t app3_start_offset = p_reader->next_byte_offset;
    uint16_t app3_len = jpegr_fetch_2bytes(p_reader);
    uint8_t  misc_byte, layout_byte, type_byte;

    // Zero out all fields
    STD_MEMSET(&(p_reader->header.jps_info), 0, sizeof(jps_cfg_3d_t));

    // Match with "_JPSJPS_"
    if (JPEG_SUCCEEDED(rc) &&
        jpegr_fetch_4bytes(p_reader) == 0x5F4A5053 &&
        jpegr_fetch_4bytes(p_reader) == 0x4A50535F)
    {
        rc = JPEGERR_SUCCESS;
    }
    else
    {
        rc = JPEGERR_EFAILED;
    }

    if (JPEG_SUCCEEDED(rc))
    {
        // consume descriptor length (uint16_t)
        (void)jpegr_fetch_2bytes(p_reader);

        // extract seperation (uint8_t)
        p_reader->header.jps_info.separation = jpegr_fetch_byte(p_reader);

        // extract Misc. Flags (uint8_t)
        misc_byte = jpegr_fetch_byte(p_reader);

        p_reader->header.jps_info.height_flag = (jps_height_t)(IS_HALF_HEIGHT(misc_byte));
        p_reader->header.jps_info.width_flag  = (jps_width_t)(IS_HALF_WIDTH(misc_byte));
        p_reader->header.jps_info.field_order = (jps_field_order_t)(IS_LEFT_FIRST(misc_byte));

        // extract Layout
        layout_byte = (jps_layout_t)jpegr_fetch_byte(p_reader);

        // only support side_by_side and over_under format
        if (layout_byte != 0x02 && layout_byte != 0x03)
        {
            rc = JPEGERR_EUNSUPPORTED;
        }

        p_reader->header.jps_info.layout = layout_byte;

        // extract Type
        type_byte = jpegr_fetch_byte(p_reader);

        // only supports stereoscopic image
        if (type_byte != 0x01)
        {
            rc = JPEGERR_EUNSUPPORTED;
        }
    }

    p_reader->next_byte_offset = app3_start_offset + app3_len;

    return rc;
}


int jpegr_process_zero_ifd(jpeg_reader_t *p_reader)
{
    int rc = JPEGERR_SUCCESS;
    uint32_t nIfdOffset = jpegr_fetch_4bytes(p_reader);
    int nNumTags, i;

    p_reader->next_byte_offset = p_reader->tiff_hdr_offset + nIfdOffset;

    nNumTags = jpegr_fetch_2bytes(p_reader);
    for (i = 0; i < nNumTags; i++)
    {
        uint16_t tag_id = jpegr_fetch_2bytes(p_reader);
        switch (tag_id)
        {
        case _ID_IMAGE_DESCRIPTION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_image_description,
                                 EXIF_ASCII, EXIFTAGID_IMAGE_DESCRIPTION);
            break;
        case _ID_MAKE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_make,
                                 EXIF_ASCII, EXIFTAGID_MAKE);
            break;
        case _ID_MODEL:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_model,
                                 EXIF_ASCII, EXIFTAGID_MODEL);
            break;
        case _ID_ORIENTATION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_orientation,
                                 EXIF_SHORT, EXIFTAGID_ORIENTATION);
            break;
        case _ID_X_RESOLUTION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_x_resolution,
                                 EXIF_RATIONAL, EXIFTAGID_X_RESOLUTION);
            break;
        case _ID_Y_RESOLUTION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_y_resolution,
                                 EXIF_RATIONAL, EXIFTAGID_Y_RESOLUTION);
            break;
        case _ID_RESOLUTION_UNIT:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_resolution_unit,
                                 EXIF_SHORT, EXIFTAGID_RESOLUTION_UNIT);
            break;
        case _ID_YCBCR_POSITIONING:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_ycbcr_positioning,
                                 EXIF_SHORT, EXIFTAGID_YCBCR_POSITIONING);
            break;
        case _ID_COMPRESSION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_compression,
                                 EXIF_SHORT, EXIFTAGID_COMPRESSION);
            break;
        case _ID_SOFTWARE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->tiff_ifd.p_software,
                                 EXIF_ASCII, EXIFTAGID_SOFTWARE);
            break;
        case _ID_DATE_TIME:
            rc = jpegr_fetch_tag(p_reader,
                                 &p_reader->header.p_exif_info->tiff_ifd.p_date_time,
                                 EXIF_ASCII, EXIFTAGID_DATE_TIME);
            break;
        case _ID_EXIF_IFD_PTR:
        case _ID_GPS_IFD_PTR:
            {
                const uint16_t exif_type = jpegr_fetch_2bytes(p_reader);
                const uint32_t cnt = jpegr_fetch_4bytes(p_reader);
                if (exif_type != EXIF_LONG || cnt != 1)
                    return JPEGERR_EFAILED;
                if (tag_id == _ID_EXIF_IFD_PTR)
                    p_reader->exif_ifd_offset = jpegr_fetch_4bytes(p_reader);
                else
                    p_reader->gps_ifd_offset = jpegr_fetch_4bytes(p_reader);
            }
            break;
        default:
            p_reader->next_byte_offset += 10;
            break;
        }
        if (rc != JPEGERR_TAGTYPE_UNEXPECTED && JPEG_FAILED(rc))
            return rc;
    }
    p_reader->first_ifd_offset = jpegr_fetch_4bytes(p_reader);

    return JPEGERR_SUCCESS;
}

int jpegr_process_exif_ifd(jpeg_reader_t *p_reader)
{
    int rc = JPEGERR_SUCCESS;
    uint16_t nNumTags, i;
    uint32_t nSaveOffset;

    if (!p_reader->exif_ifd_offset)
        return JPEGERR_SUCCESS;

    p_reader->next_byte_offset = p_reader->tiff_hdr_offset + p_reader->exif_ifd_offset;

    nNumTags = jpegr_fetch_2bytes(p_reader);
    for (i = 0; i < nNumTags; i++)
    {
        uint16_t tag_id = jpegr_fetch_2bytes(p_reader);
        switch (tag_id)
        {
        case _ID_EXIF_VERSION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_version,
                                 EXIF_UNDEFINED, EXIFTAGID_EXIF_VERSION);
            break;
        case _ID_EXIF_DATE_TIME_ORIGINAL:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_date_time_original,
                                 EXIF_ASCII, EXIFTAGID_EXIF_DATE_TIME_ORIGINAL);
            break;
        case _ID_EXIF_COMPONENTS_CONFIG:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_components_config,
                                 EXIF_UNDEFINED, EXIFTAGID_EXIF_COMPONENTS_CONFIG);
            break;
        case _ID_EXIF_USER_COMMENT:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_user_comment,
                                 EXIF_UNDEFINED, EXIFTAGID_EXIF_USER_COMMENT);
            break;
        case _ID_EXIF_MAKER_NOTE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_maker_note,
                                 EXIF_UNDEFINED, EXIFTAGID_EXIF_MAKER_NOTE);
            break;
        case _ID_EXIF_FLASHPIX_VERSION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_flashpix_version,
                                 EXIF_UNDEFINED, EXIFTAGID_EXIF_FLASHPIX_VERSION);
            break;
        case _ID_EXIF_COLOR_SPACE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_color_space,
                                 EXIF_SHORT, EXIFTAGID_EXIF_COLOR_SPACE);
            break;
        case _ID_EXIF_PIXEL_X_DIMENSION:
            nSaveOffset = p_reader->next_byte_offset;
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_pixel_x_dimension,
                                 EXIF_LONG, EXIFTAGID_EXIF_PIXEL_X_DIMENSION);
            if (rc == JPEGERR_TAGTYPE_UNEXPECTED)
            {
                p_reader->next_byte_offset = nSaveOffset;
                rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_pixel_x_dimension,
                                     EXIF_SHORT, EXIFTAGID_EXIF_PIXEL_X_DIMENSION);
            }
            break;
        case _ID_EXIF_PIXEL_Y_DIMENSION:
            nSaveOffset = p_reader->next_byte_offset;
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_pixel_y_dimension,
                                 EXIF_LONG, EXIFTAGID_EXIF_PIXEL_Y_DIMENSION);
            if (rc == JPEGERR_TAGTYPE_UNEXPECTED)
            {
                p_reader->next_byte_offset = nSaveOffset;
                rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_pixel_y_dimension,
                                     EXIF_SHORT, EXIFTAGID_EXIF_PIXEL_Y_DIMENSION);
            }
            break;
        case _ID_EXPOSURE_TIME:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exposure_time,
                                 EXIF_RATIONAL, EXIFTAGID_EXPOSURE_TIME);
            break;
        case _ID_EXIF_DATE_TIME_DIGITIZED:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exif_date_time_digitized,
                                 EXIF_ASCII, EXIFTAGID_EXIF_DATE_TIME_DIGITIZED);
            break;
        case _ID_FLASH:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_flash,
                                 EXIF_SHORT, EXIFTAGID_FLASH);
            break;
        case _ID_CUSTOM_RENDERED:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_custom_rendered,
                                 EXIF_SHORT, EXIFTAGID_CUSTOM_RENDERED);
            break;
        case _ID_EXPOSURE_MODE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exposure_mode,
                                 EXIF_SHORT, EXIFTAGID_EXPOSURE_MODE);
            break;
        case _ID_WHITE_BALANCE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_white_balance,
                                 EXIF_SHORT, EXIFTAGID_WHITE_BALANCE);
            break;
        case _ID_DIGITAL_ZOOM_RATIO:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_digital_zoom_ratio,
                                 EXIF_RATIONAL, EXIFTAGID_DIGITAL_ZOOM_RATIO);
            break;
        case _ID_SCENE_CAPTURE_TYPE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_scene_capture_type,
                                 EXIF_SHORT, EXIFTAGID_SCENE_CAPTURE_TYPE);
            break;
        case _ID_SUBJECT_DISTANCE_RANGE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_subject_distance_range,
                                 EXIF_SHORT, EXIFTAGID_SUBJECT_DISTANCE_RANGE);
            break;
        case _ID_APERTURE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_aperture,
                                 EXIF_RATIONAL, EXIFTAGID_APERTURE);
            break;
        case _ID_MAX_APERTURE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_max_aperture,
                                 EXIF_RATIONAL, EXIFTAGID_MAX_APERTURE);
            break;
        case _ID_LIGHT_SOURCE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_light_source,
                                 EXIF_SHORT, EXIFTAGID_LIGHT_SOURCE);
            break;
        case _ID_SENSING_METHOD:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_sensing_method,
                                 EXIF_SHORT, EXIFTAGID_SENSING_METHOD);
            break;
        case _ID_FILE_SOURCE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_file_source,
                                 EXIF_UNDEFINED, EXIFTAGID_FILE_SOURCE);
            break;
        case _ID_CFA_PATTERN:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_cfa_pattern,
                                 EXIF_UNDEFINED, EXIFTAGID_CFA_PATTERN);
            break;
        case _ID_FOCAL_LENGTH_35MM:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_focal_length_35mm,
                                 EXIF_SHORT, EXIFTAGID_FOCAL_LENGTH_35MM);
            break;
        case _ID_CONTRAST:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_contrast,
                                 EXIF_SHORT, EXIFTAGID_CONTRAST);
            break;
        case _ID_SATURATION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_saturation,
                                 EXIF_SHORT, EXIFTAGID_SATURATION);
            break;
        case _ID_METERING_MODE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_metering_mode,
                                 EXIF_SHORT, EXIFTAGID_METERING_MODE);
            break;
        case _ID_SHARPNESS:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_sharpness,
                                 EXIF_SHORT, EXIFTAGID_SHARPNESS);
            break;
        case _ID_GAIN_CONTROL:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_gain_control,
                                 EXIF_SHORT, EXIFTAGID_GAIN_CONTROL);
            break;
        case _ID_F_NUMBER:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_f_number,
                                 EXIF_RATIONAL, EXIFTAGID_F_NUMBER);
            break;
        case _ID_OECF:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_oecf,
                                 EXIF_UNDEFINED, EXIFTAGID_OECF);
            break;
        case _ID_SHUTTER_SPEED:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_shutter_speed,
                                 EXIF_SRATIONAL, EXIFTAGID_SHUTTER_SPEED);
            break;
        case _ID_EXPOSURE_PROGRAM:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exposure_program,
                                 EXIF_SHORT, EXIFTAGID_EXPOSURE_PROGRAM);
            break;
        case _ID_BRIGHTNESS:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_brightness,
                                 EXIF_SRATIONAL, EXIFTAGID_BRIGHTNESS);
            break;
        case _ID_FOCAL_LENGTH:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_focal_length,
                                 EXIF_RATIONAL, EXIFTAGID_FOCAL_LENGTH);
            break;
        case _ID_EXPOSURE_INDEX:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exposure_index,
                                 EXIF_RATIONAL, EXIFTAGID_EXPOSURE_INDEX);
            break;
        case _ID_SUBJECT_DISTANCE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_subject_distance,
                                 EXIF_RATIONAL, EXIFTAGID_SUBJECT_DISTANCE);
            break;
        case _ID_SUBJECT_LOCATION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_subject_location,
                                 EXIF_SHORT, EXIFTAGID_SUBJECT_LOCATION);
            break;
        case _ID_SCENE_TYPE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_scene_type,
                                 EXIF_UNDEFINED, EXIFTAGID_SCENE_TYPE);
            break;
        case _ID_ISO_SPEED_RATING:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_iso_speed_rating,
                                 EXIF_SHORT, EXIFTAGID_ISO_SPEED_RATING);
            break;

        case _ID_EXPOSURE_BIAS_VALUE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->exif_ifd.p_exposure_bias_value,
                                 EXIF_SRATIONAL, EXIFTAGID_EXPOSURE_BIAS_VALUE);
            break;

        case _ID_INTEROP_IFD_PTR:
            {
                const uint16_t exif_type = jpegr_fetch_2bytes(p_reader);
                const uint32_t cnt = jpegr_fetch_4bytes(p_reader);
                if (exif_type != EXIF_LONG || cnt != 1)
                    return JPEGERR_EFAILED;
                p_reader->interop_ifd_offset = jpegr_fetch_4bytes(p_reader);
            }
            break;
        default:
            p_reader->next_byte_offset += 10;
            break;
        }
        if (rc != JPEGERR_TAGTYPE_UNEXPECTED && JPEG_FAILED(rc))
            return rc;
    }

    return JPEGERR_SUCCESS;
}

int jpegr_process_gps_ifd(jpeg_reader_t *p_reader)
{
    int rc = JPEGERR_SUCCESS;
    int nNumTags, i;

    if (!p_reader->gps_ifd_offset)
        return JPEGERR_SUCCESS;

    p_reader->next_byte_offset = p_reader->tiff_hdr_offset + p_reader->gps_ifd_offset;

    nNumTags = jpegr_fetch_2bytes(p_reader);
    for (i = 0; i < nNumTags; i++)
    {
        uint16_t tag_id = jpegr_fetch_2bytes(p_reader);
        switch (tag_id)
        {
        case _ID_GPS_VERSION_ID:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_version_id,
                                 EXIF_BYTE, EXIFTAGID_GPS_VERSION_ID);
            break;
        case _ID_GPS_LATITUDE_REF:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_latitude_ref,
                                 EXIF_ASCII, EXIFTAGID_GPS_LATITUDE_REF);
            break;
        case _ID_GPS_LATITUDE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_latitude,
                                 EXIF_RATIONAL, EXIFTAGID_GPS_LATITUDE);
            break;
        case _ID_GPS_LONGITUDE_REF:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_longitude_ref,
                                 EXIF_ASCII, EXIFTAGID_GPS_LONGITUDE_REF);
            break;
        case _ID_GPS_LONGITUDE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_longitude,
                                 EXIF_RATIONAL, EXIFTAGID_GPS_LONGITUDE);
            break;
        case _ID_GPS_ALTITUDE_REF:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_altitude_ref,
                                 EXIF_BYTE, EXIFTAGID_GPS_ALTITUDE_REF);
            break;
        case _ID_GPS_TIMESTAMP:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_timestamp,
                                 EXIF_RATIONAL, EXIFTAGID_GPS_TIMESTAMP);
            break;
        case _ID_GPS_MAPDATUM:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_map_datum,
                                 EXIF_ASCII, EXIFTAGID_GPS_MAPDATUM);
            break;
        case _ID_GPS_AREAINFORMATION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_area_information,
                                 EXIF_UNDEFINED, EXIFTAGID_GPS_AREAINFORMATION);
            break;
        case _ID_GPS_DATESTAMP:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_datestamp,
                                 EXIF_ASCII, EXIFTAGID_GPS_DATESTAMP);
            break;
        case _ID_GPS_PROCESSINGMETHOD:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->gps_ifd.p_processing_method,
                                 EXIF_UNDEFINED, EXIFTAGID_GPS_PROCESSINGMETHOD);
            break;
        default:
            p_reader->next_byte_offset += 10;
            break;
        }
        if (rc != JPEGERR_TAGTYPE_UNEXPECTED && JPEG_FAILED(rc))
            return rc;
    }
    return JPEGERR_SUCCESS;
}

int jpegr_process_first_ifd(jpeg_reader_t *p_reader)
{
    int rc = JPEGERR_SUCCESS;
    uint16_t nNumTags, i;

    if (!p_reader->first_ifd_offset)
        return JPEGERR_SUCCESS;

    p_reader->next_byte_offset = p_reader->tiff_hdr_offset + p_reader->first_ifd_offset;

    nNumTags = jpegr_fetch_2bytes(p_reader);
    for (i = 0; i < nNumTags; i++)
    {
        uint16_t tag_id = jpegr_fetch_2bytes(p_reader);
        switch (tag_id)
        {
        case _ID_IMAGE_DESCRIPTION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_image_description,
                                 EXIF_ASCII, EXIFTAGID_IMAGE_DESCRIPTION);
            break;
        case _ID_MAKE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_make,
                                 EXIF_ASCII, EXIFTAGID_MAKE);
            break;
        case _ID_MODEL:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_model,
                                 EXIF_ASCII, EXIFTAGID_MODEL);
            break;
        case _ID_ORIENTATION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_orientation,
                                 EXIF_SHORT, EXIFTAGID_ORIENTATION);
            break;
        case _ID_X_RESOLUTION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_x_resolution,
                                 EXIF_RATIONAL, EXIFTAGID_X_RESOLUTION);
            break;
        case _ID_Y_RESOLUTION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_y_resolution,
                                 EXIF_RATIONAL, EXIFTAGID_Y_RESOLUTION);
            break;
        case _ID_RESOLUTION_UNIT:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_resolution_unit,
                                 EXIF_SHORT, EXIFTAGID_RESOLUTION_UNIT);
            break;
        case _ID_YCBCR_POSITIONING:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_ycbcr_positioning,
                                 EXIF_SHORT, EXIFTAGID_YCBCR_POSITIONING);
            break;
        case _ID_COMPRESSION:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_compression,
                                 EXIF_SHORT, EXIFTAGID_COMPRESSION);
            break;
        case _ID_SOFTWARE:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_software,
                                 EXIF_ASCII, EXIFTAGID_SOFTWARE);
            break;
        case _ID_TN_JPEGINTERCHANGE_FORMAT:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_interchange_format,
                                 EXIF_LONG, EXIFTAGID_TN_JPEGINTERCHANGE_FORMAT);
            break;
        case _ID_TN_JPEGINTERCHANGE_FORMAT_L:
            rc = jpegr_fetch_tag(p_reader, &p_reader->header.p_exif_info->thumbnail_ifd.p_interchange_format_l,
                                 EXIF_LONG, EXIFTAGID_TN_JPEGINTERCHANGE_FORMAT_L);
            break;
        default:
            break;
        }
        if (rc != JPEGERR_TAGTYPE_UNEXPECTED && JPEG_FAILED(rc))
            return rc;
    }
    if (JPEG_SUCCEEDED(rc) &&
        p_reader->header.p_exif_info->thumbnail_ifd.p_interchange_format &&
        p_reader->header.p_exif_info->thumbnail_ifd.p_interchange_format_l)
    {
        // Process thumbnail
        p_reader->next_byte_offset = p_reader->tiff_hdr_offset +
            p_reader->header.p_exif_info->thumbnail_ifd.p_interchange_format->entry.data._long;

        rc = jpegr_find_soi(p_reader);
        if (JPEG_SUCCEEDED(rc))
        {
            jpeg_frame_info_destroy(p_reader->header.p_tn_frame_info);
            p_reader->header.p_tn_frame_info = (jpeg_frame_info_t *)JPEG_MALLOC(sizeof(jpeg_frame_info_t));
            if (!p_reader->header.p_tn_frame_info)
            {
                rc = JPEGERR_EMALLOC;
            }
            else
            {
                STD_MEMSET(p_reader->header.p_tn_frame_info, 0, sizeof(jpeg_frame_info_t));
            }
        }

        if (JPEG_SUCCEEDED(rc))
        {
            exif_endianness_t save = p_reader->endianness;
            p_reader->endianness = EXIF_BIG_ENDIAN;
            rc = jpegr_parse_sof(p_reader, p_reader->header.p_tn_frame_info);
            p_reader->endianness = save;
        }
    }
    return rc;
}

// Create a tag and fill in the information based on the input
// stream.
// Side effect: If JPEGERR_SUCCESS is returned, it
// means the tag is fetched successfully and the next fetch
// offset will be set to the next tag ID. Otherwise, the next
// fetch offset will be resetted back to the current tag ID.
int jpegr_fetch_tag(jpeg_reader_t        *p_reader,
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
    tag_start_offset      = p_reader->next_byte_offset;
    p_new_tag->entry.type = (exif_tag_type_t)jpegr_fetch_2bytes(p_reader);
    cnt                   = jpegr_fetch_4bytes(p_reader);

    // Check if the fetched type is expected
    if (p_new_tag->entry.type != expected_type)
    {
        // Reset fetch offset
        p_reader->next_byte_offset = tag_start_offset;
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
                p_reader->next_byte_offset = p_reader->tiff_hdr_offset + jpegr_fetch_4bytes(p_reader);

            jpegr_fetch_nbytes(p_reader, p_new_tag->entry.data._bytes, cnt, NULL);
        }
        else
        {
            p_new_tag->entry.data._byte = jpegr_fetch_byte(p_reader);
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
                p_reader->next_byte_offset = p_reader->tiff_hdr_offset + jpegr_fetch_4bytes(p_reader);

            for (i = 0; i < cnt; i++)
            {
                p_new_tag->entry.data._shorts[i] = jpegr_fetch_2bytes(p_reader);
            }
        }
        else
        {
            p_new_tag->entry.data._short = jpegr_fetch_2bytes(p_reader);
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
            p_reader->next_byte_offset = p_reader->tiff_hdr_offset + jpegr_fetch_4bytes(p_reader);
            for (i = 0; i < cnt; i++)
            {
                p_new_tag->entry.data._longs[i] = (uint32_t)jpegr_fetch_4bytes(p_reader);
            }
        }
        else
        {
            p_new_tag->entry.data._long = (uint32_t)jpegr_fetch_4bytes(p_reader);
        }
        break;
    case EXIF_RATIONAL:
    case EXIF_SRATIONAL:
        // Data doesn't fit into 4 bytes, read from an offset
        p_reader->next_byte_offset = p_reader->tiff_hdr_offset + jpegr_fetch_4bytes(p_reader);
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
                p_new_tag->entry.data._rats[i].num   = jpegr_fetch_4bytes(p_reader);
                p_new_tag->entry.data._rats[i].denom = jpegr_fetch_4bytes(p_reader);
            }
        }
        else
        {
            p_new_tag->entry.data._rat.num   = jpegr_fetch_4bytes(p_reader);
            p_new_tag->entry.data._rat.denom = jpegr_fetch_4bytes(p_reader);
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
                p_reader->next_byte_offset = p_reader->tiff_hdr_offset + jpegr_fetch_4bytes(p_reader);

            jpegr_fetch_nbytes(p_reader, (uint8_t *)p_new_tag->entry.data._ascii, cnt, NULL);
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
        p_reader->next_byte_offset = tag_start_offset + 10;
    }
    else
    {
        p_reader->next_byte_offset = tag_start_offset;
        exif_destroy_tag_entry(p_new_tag);
        //nullify the pp_tag_entry, since it is already destroyed,
        //so that next time when jpegr_fetch_tag is been called in,
        //it will not double free the tag entry.
        *pp_tag_entry = NULL;
    }
    return rc;
}

