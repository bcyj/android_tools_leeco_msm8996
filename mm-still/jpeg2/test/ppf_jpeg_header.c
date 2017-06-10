/*============================================================================

   Copyright  2011 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/


/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
02/05/10   zhiminl Created.

========================================================================== */

/*------------------------------------------------------------------------
 *                          Include Files
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include "jpeg_common.h"

/*------------------------------------------------------------------------
 *                          Type Declarations
 * ----------------------------------------------------------------------- */

typedef struct
{
    uint8_t  *ptr;
    uint32_t  offset;
    uint32_t  size;

} buffer_t;

/*------------------------------------------------------------------------
 *                          Definitions and Constants
 * ----------------------------------------------------------------------- */

#define M_SOI     0xd8
#define M_EOI     0xd9
#define M_SOF0    0xc0
#define M_DQT     0xdb
#define M_DHT     0xc4
#define M_DRI     0xdd
#define M_SOS     0xda

/*------------------------------------------------------------------------
 *                          Static Variable Definitions
 * ----------------------------------------------------------------------- */

/* Standard Zigzag table */
static const uint16_t standard_zigzag_tbl[64] =
{
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

/* Standard Quantization Tables */
static const uint16_t standard_luma_q_tbl[] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99
};

static const uint16_t standard_chroma_q_tbl[] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};

/*lint -save -e785 Intentional insufficient initializers */
/* Standard Huffman Tables */
static const jpeg_huff_table_t standard_luma_dc_huff_tbl =
{
    /* Bits (0-based) */
    {0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    /* huffValues */
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
};

static const jpeg_huff_table_t standard_chroma_dc_huff_tbl =
{
    /* Bits (0-based) */
    {0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    /* huffValues */
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
};

static const jpeg_huff_table_t standard_luma_ac_huff_tbl =
{
    /* Bits (0-based) */
    {0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d},
    /* huffValues */
    {0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
     0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
     0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
     0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
     0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
     0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
     0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
     0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
     0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
     0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
     0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
     0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
     0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
     0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
     0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
     0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
     0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
     0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
     0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
     0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
     0xf9, 0xfa},
};

static const jpeg_huff_table_t standard_chroma_ac_huff_tbl =
{
    /* Bits (0-based) */
    {0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77},
    /* huffValues */
    {0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
     0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
     0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
     0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
     0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
     0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
     0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
     0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
     0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
     0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
     0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
     0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
     0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
     0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
     0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
     0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
     0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
     0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
     0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
     0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
     0xf9, 0xfa},
};
/*lint -restore */

static buffer_t jpeg_header;
static uint16_t luma_q_tbl[64];
static uint16_t chroma_q_tbl[64];

/*------------------------------------------------------------------------
 *                          Static Function Definitions
 * ----------------------------------------------------------------------- */

static int emit_byte
(
    buffer_t       *p_header,
    uint8_t         byte
)
{
    if (p_header->offset >= p_header->size) return JPEGERR_EFAILED;
    p_header->ptr[p_header->offset++] = byte;
    return JPEGERR_SUCCESS;
} /* emit_byte */

static int emit_2bytes
(
    buffer_t       *p_header,
    uint16_t        bytes
)
{
    int rc = emit_byte(p_header, (uint8_t)((bytes >> 8) & (uint16_t)0xFF));
    if (JPEG_FAILED(rc)) return rc;
    return emit_byte(p_header, (uint8_t)(bytes & (uint16_t)0xFF));
} /* emit_2bytes */

static int emit_marker
(
    buffer_t       *p_header,
    uint8_t         marker
)
{
    int rc = emit_byte(p_header, 0xFF);
    if (JPEG_FAILED(rc)) return rc;
    return emit_byte(p_header, marker);
} /* emit_marker */

static int emit_DQT
(
    buffer_t       *p_header,
    const uint16_t  luma_qtable[],
    const uint16_t  chroma_qtable[]
)
{
    uint32_t  i;
    int       rc;

    rc = emit_marker(p_header, M_DQT);
    if (JPEG_FAILED(rc)) return rc;

    // Lq
    rc = emit_2bytes(p_header, (64 * 2 + 2 + 2));
    if (JPEG_FAILED(rc)) return rc;

    // Pq, Tq
    rc = emit_byte(p_header, 0);
    if (JPEG_FAILED(rc)) return rc;

    // Qk
    for (i = 0; i < 64; i++)
    {
        rc = emit_byte(p_header, (uint8_t)(luma_qtable[standard_zigzag_tbl[i]] & 0xFF));
        if (JPEG_FAILED(rc)) return rc;
    }

    // Pq, Tq
    rc = emit_byte(p_header, 1);
    if (JPEG_FAILED(rc)) return rc;

    // Qk
    for (i = 0; i < 64; i++)
    {
        rc = emit_byte(p_header, (uint8_t)(chroma_qtable[standard_zigzag_tbl[i]] & 0xFF));
        if (JPEG_FAILED(rc)) return rc;
    }

    return JPEGERR_SUCCESS;
} /* emit_DQT */

static int emit_DHT
(
    buffer_t                 *p_header,
    const jpeg_huff_table_t  *p_htable,
    uint8_t                   index
)
{
    uint16_t length, i;
    int      rc;

    length = 0;
    for (i = 1; i <= 16; i++)
        length = length + (uint16_t)(p_htable->bits[i]);

    // Tc, Th
    rc = emit_byte(p_header, (uint8_t)(index & 0xFF));
    if (JPEG_FAILED(rc)) return rc;

    // Li
    for (i = 1; i <= 16; i++)
    {
        rc = emit_byte(p_header, (uint8_t)(p_htable->bits[i]));
        if (JPEG_FAILED(rc)) return rc;
    }

    // Vi,j
    for (i = 0; i < length; i++)
    {
        rc = emit_byte(p_header, (uint8_t)(p_htable->values[i]));
        if (JPEG_FAILED(rc)) return rc;
    }

    return JPEGERR_SUCCESS;
} /* emit_DHT */

static int emit_DRI
(
    buffer_t *p_header,
    uint8_t   restart
)
{
    int rc = emit_marker(p_header, M_DRI);
    if (JPEG_FAILED(rc)) return rc;

    // Lr
    rc = emit_2bytes(p_header, 4);
    if (JPEG_FAILED(rc)) return rc;

    // Ri
    rc = emit_2bytes(p_header, restart);
    if (JPEG_FAILED(rc)) return rc;

    return JPEGERR_SUCCESS;
} /* emit_DRI */

static int emit_SOF
(
    buffer_t           *p_header,
    uint32_t            width,
    uint32_t            height,
    jpeg_subsampling_t  subsample,
    uint32_t            rotation
)
{
    uint32_t  hsample, vsample, temp;
    int       rc;

    if ((rotation != 0) && (rotation != 90) && (rotation != 180) && (rotation != 270))
        return JPEGERR_EBADPARM;

    switch (subsample)
    {
    case JPEG_H2V2:
    case JPEG_BS_H2V2:
        hsample = 2;
        vsample = 2;
        break;

    case JPEG_H2V1:
    case JPEG_BS_H2V1:
        hsample = 2;
        vsample = 1;
        break;

    case JPEG_H1V2:
    case JPEG_BS_H1V2:
        hsample = 1;
        vsample = 2;
        break;

    case JPEG_H1V1:
    case JPEG_BS_H1V1:
    case JPEG_GRAYSCALE:
    default:
        hsample = 1;
        vsample = 1;
        break;
    }

    if ((rotation == 90) || (rotation == 270))
    {
        // Swap image dimension for odd rotations
        temp = width;
        width = height;
        height = temp;
        // Swap subsamples for odd rotations
        temp = hsample;
        hsample = vsample;
        vsample = temp;
    }

    rc = emit_marker(p_header, M_SOF0);
    if (JPEG_FAILED(rc)) return rc;

    // Lf
    rc = emit_2bytes(p_header, (uint16_t)(3*3+2+5+1));
    if (JPEG_FAILED(rc)) return rc;

    // P
    rc = emit_byte(p_header, 8);
    if (JPEG_FAILED(rc)) return rc;

    // Y
    rc = emit_2bytes(p_header, (uint16_t)height);
    if (JPEG_FAILED(rc)) return rc;

    // X
    rc = emit_2bytes(p_header, (uint16_t)width);
    if (JPEG_FAILED(rc)) return rc;

    // Nf
    if (subsample == JPEG_GRAYSCALE)
    {
        rc = emit_byte(p_header, 1);
    }
    else
    {
        rc = emit_byte(p_header, 3);
    }
    if (JPEG_FAILED(rc)) return rc;

    // Ci (Y)
    rc = emit_byte(p_header, 1);
    if (JPEG_FAILED(rc)) return rc;
    // Hi, Vi (Y)
    rc = emit_byte(p_header, (uint8_t)((hsample << 4) + vsample));
    if (JPEG_FAILED(rc)) return rc;
    // Tqi (Y)
    rc = emit_byte(p_header, 0);
    if (JPEG_FAILED(rc)) return rc;

    // Ci (Cb)
    rc = emit_byte(p_header, 2);
    if (JPEG_FAILED(rc)) return rc;
    // Hi, Vi (Cb)
    rc = emit_byte(p_header, (uint8_t)((1 << 4) + 1));
    if (JPEG_FAILED(rc)) return rc;
    // Tqi (Cb)
    rc = emit_byte(p_header, 1);
    if (JPEG_FAILED(rc)) return rc;

    // Ci (Cr)
    rc = emit_byte(p_header, 3);
    if (JPEG_FAILED(rc)) return rc;
    // Hi, Vi (Cr)
    rc = emit_byte(p_header, (uint8_t)((1 << 4) + 1));
    if (JPEG_FAILED(rc)) return rc;
    // Tqi (Cr)
    rc = emit_byte(p_header, 1);
    if (JPEG_FAILED(rc)) return rc;

    return JPEGERR_SUCCESS;
} /* emit_SOF */

static int emit_SOS
(
    buffer_t *p_header
)
{
    int rc = emit_marker(p_header, M_SOS);
    if (JPEG_FAILED(rc)) return rc;

    // Ls
    rc = emit_2bytes(p_header, (uint16_t)(2*3+2+1+3));
    if (JPEG_FAILED(rc)) return rc;

    // Ns
    rc = emit_byte(p_header, 3);
    if (JPEG_FAILED(rc)) return rc;

    // Csj (Y)
    rc = emit_byte(p_header, 1);
    if (JPEG_FAILED(rc)) return rc;
    // Tdj, Taj (Y)
    rc = emit_byte(p_header, (uint8_t)((0 << 4) + 0));
    if (JPEG_FAILED(rc)) return rc;

    // Csj (Cb)
    rc = emit_byte(p_header, 2);
    if (JPEG_FAILED(rc)) return rc;
    // Tdj, Taj (Cb)
    rc = emit_byte(p_header, (uint8_t)((1 << 4) + 1));
    if (JPEG_FAILED(rc)) return rc;

    // Csj (Cr)
    rc = emit_byte(p_header, 3);
    if (JPEG_FAILED(rc)) return rc;
    // Tdj, Taj (Cr)
    rc = emit_byte(p_header, (uint8_t)((1 << 4) + 1));
    if (JPEG_FAILED(rc)) return rc;

    // Ss
    rc = emit_byte(p_header, 0);
    if (JPEG_FAILED(rc)) return rc;
    // Se
    rc = emit_byte(p_header, 63);
    if (JPEG_FAILED(rc)) return rc;

    // Ah, Al
    rc = emit_byte(p_header, (uint8_t)(0 + 0));
    if (JPEG_FAILED(rc)) return rc;

    return JPEGERR_SUCCESS;
} /* emit_SOS */

static int fetch_byte
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint8_t            *p_byte
)
{
    if ((*p_offset) >= size) return JPEGERR_EFAILED;
    *p_byte = *(ptr + (int32_t)(*p_offset));
    (*p_offset)++;

    return JPEGERR_SUCCESS;
} /* fetch_byte */

static int fetch_2bytes
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint16_t           *p_2bytes
)
{
    uint8_t byte1, byte2;
    int     rc;

    rc = fetch_byte(ptr, size, p_offset, &byte1);
    if (JPEG_FAILED (rc)) return rc;
    rc = fetch_byte(ptr, size, p_offset, &byte2);
    if (JPEG_FAILED (rc)) return rc;

    *p_2bytes = (uint16_t)((byte1 << 8) + byte2);
    return JPEGERR_SUCCESS;
} /* fetch_2bytes */

int find_SOI
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset
)
{
    uint8_t   m1, m2;
    uint32_t  cnt = 256;
    int       rc;

    rc = fetch_byte(ptr, size, p_offset, &m1);
    if (JPEG_FAILED (rc)) return rc;
    rc = fetch_byte(ptr, size, p_offset, &m2);
    if (JPEG_FAILED (rc)) return rc;

    if ((m1 == 0xFF) && (m2 == M_SOI)) return JPEGERR_SUCCESS;

    // not found immediately, continuing searching for another 256 bytes
    while (cnt)
    {
        m1 = m2;
        rc = fetch_byte(ptr, size, p_offset, &m2);
        if (JPEG_FAILED (rc)) return rc;

        if ((m1 == 0xFF) && (m2 == M_SOI)) return JPEGERR_SUCCESS;
        cnt--;
    }
    return JPEGERR_EFAILED;
} /* find_SOI */

static int find_next_header
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint8_t            *p_byte
)
{
    uint8_t byte1;
    int     rc;

    do
    {
        do
        {
            rc = fetch_byte(ptr, size, p_offset, &byte1);
            if (JPEG_FAILED (rc)) return rc;

        } while (byte1 != 0xFF);

        do
        {
            rc = fetch_byte(ptr, size, p_offset, &byte1);
            if (JPEG_FAILED (rc)) return M_EOI;

        } while (byte1 == 0xFF);

    } while (byte1 == 0);

    *p_byte = byte1;
    return JPEGERR_SUCCESS;
} /* find_next_header */

static int process_SOF
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint32_t           *p_width,
    uint32_t           *p_height,
    jpeg_subsampling_t *p_subsample,
    uint8_t             qtable_selectors[]
)
{
    uint8_t   byte1, num_comps, i;
    uint8_t   hsample = 0, vsample = 0;
    uint16_t  length, two_bytes;
    int       rc;

    // Get Lf
    rc = fetch_2bytes(ptr, size, p_offset, &length);
    if (JPEG_FAILED (rc)) return rc;

    // Get P - ignored
    rc = fetch_byte(ptr, size, p_offset, &byte1);
    if (JPEG_FAILED (rc)) return rc;

    // Get Y
    rc = fetch_2bytes(ptr, size, p_offset, &two_bytes);
    if (JPEG_FAILED (rc)) return rc;
    *p_height = (uint32_t)two_bytes;

    // Get X
    rc = fetch_2bytes(ptr, size, p_offset, &two_bytes);
    if (JPEG_FAILED (rc)) return rc;
    *p_width = (uint32_t)two_bytes;

    // Get Nf
    rc = fetch_byte(ptr, size, p_offset, &num_comps);
    if (JPEG_FAILED (rc)) return rc;

    // Validation on number of components
    if ((num_comps == 0) || (num_comps > 4) || (length != (8 + num_comps * 3)))
        return JPEGERR_EFAILED;

    // Get component infos
    for (i = 0; i < num_comps; i++)
    {
        uint8_t comp_id;

        // Get Ci
        rc = fetch_byte(ptr, size, p_offset, &comp_id);
        if (JPEG_FAILED (rc)) return rc;

        // Get Hi, Vi
        rc = fetch_byte(ptr, size, p_offset, &byte1);
        if (JPEG_FAILED (rc)) return rc;

        if (i == 0)
        {
            hsample = (byte1 >> 4);
            vsample = (byte1 & 0x0F);
        }

        // Get Tqi
        rc = fetch_byte(ptr, size, p_offset, &(qtable_selectors[comp_id]));
        if (JPEG_FAILED (rc)) return rc;

        if (qtable_selectors[comp_id] > 3) return JPEGERR_EFAILED;
    }

    // Derive subsampling information
    if (num_comps == 1)
    {
        *p_subsample = JPEG_GRAYSCALE;
    }
    else if (num_comps == 3)
    {
        if ((hsample == 2) && (vsample == 2))
        {
            *p_subsample = JPEG_H2V2;
        }
        else if ((hsample == 2) && (vsample == 1))
        {
            *p_subsample = JPEG_H2V1;
        }
        else if ((hsample == 1) && (vsample == 2))
        {
            *p_subsample = JPEG_H1V2;
        }
        else if ((hsample == 1) && (vsample == 1))
        {
            *p_subsample = JPEG_H1V1;
        }
        else
            return JPEGERR_EUNSUPPORTED;
    }
    else
        return JPEGERR_EUNSUPPORTED;

    return JPEGERR_SUCCESS;
} /* process_SOF */

static int process_SOS
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint8_t             htable_dc_selectors[],
    uint8_t             htable_ac_selectors[]
)
{
    uint16_t  length;
    uint8_t   num_comps, byte1, i;
    int       rc;

    // Get Ls
    rc = fetch_2bytes(ptr, size, p_offset, &length);
    if (JPEG_FAILED (rc)) return rc;

    // Get Ns
    rc = fetch_byte(ptr, size, p_offset, &num_comps);
    if (JPEG_FAILED (rc)) return rc;

    // Validation
    if (num_comps != 1 && num_comps != 3)
    {
        fprintf(stderr, "process_SOS: unexpected number of components: %d\n", num_comps);
        return JPEGERR_EFAILED;
    }
    if (length != 6 + num_comps * 2)
    {
        fprintf(stderr, "process_SOS: length not match with number of components\n");
        return JPEGERR_EFAILED;
    }

    for (i = 0; i < num_comps; i++)
    {
        uint8_t comp_id;

        // Get Csj
        rc = fetch_byte(ptr, size, p_offset, &comp_id);
        if (JPEG_FAILED (rc)) return rc;

        // Get Tdj, Taj
        rc = fetch_byte(ptr, size, p_offset, &byte1);
        if (JPEG_FAILED (rc)) return rc;

        htable_dc_selectors[comp_id] = (byte1 >> 4);
        htable_ac_selectors[comp_id] = (byte1 & 0x0F);
    }

    // Get Ss
    rc = fetch_byte(ptr, size, p_offset, &byte1);
    if (JPEG_FAILED (rc)) return rc;

    // Get Se
    rc = fetch_byte(ptr, size, p_offset, &byte1);
    if (JPEG_FAILED (rc)) return rc;

    // Get Ah, Al
    rc = fetch_byte(ptr, size, p_offset, &byte1);
    return rc;
} /* process_SOS */

static int process_DQT
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint16_t            qtables[][64],
    uint32_t           *p_qtable_present_flag
)
{
    uint16_t  length, i, j;
    uint8_t   byte1, index;
    int       rc;

    // Get Lq
    rc = fetch_2bytes(ptr, size, p_offset, &length);
    if (JPEG_FAILED (rc)) return rc;

    for (i = 0; i < (length / 64); i++)
    {
        uint8_t precision;

        // Get Pq, Tq
        rc = fetch_byte(ptr, size, p_offset, &byte1);
        if (JPEG_FAILED (rc)) return rc;

        precision = (byte1 >> 4);
        index     = (byte1 & 0x0F);

        if (precision > 1 || index >= 4)
            return JPEGERR_EFAILED;

        // Set the bit flag to indicate which table are read
        *p_qtable_present_flag |= (1 << index);

        // Get Qk
        for (j = 0; j < 64; j++)
        {
            if (precision)
            {
                // Unzigzag Qk
                rc = fetch_2bytes(ptr, size, p_offset, &(qtables[index][standard_zigzag_tbl[j]]));
            }
            else
            {
                rc = fetch_byte(ptr, size, p_offset, &byte1);
                // Unzigzag Qk
                qtables[index][standard_zigzag_tbl[j]] = (uint16_t)byte1;
            }
            if (JPEG_FAILED (rc)) return rc;
        }

        // Validation
        if (2 + 65 + 64 * (uint16_t)precision > length)
        {
            return JPEGERR_EFAILED;
        }
    }
    return JPEGERR_SUCCESS;
} /* process_DQT */

static int process_DHT
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    jpeg_huff_table_t   htables[],
    uint32_t           *p_htable_present_flag
)
{
    uint16_t  length, sum, j;
    uint8_t   index;
    int       rc;

    // Get Lh
    rc = fetch_2bytes(ptr, size, p_offset, &length);
    if (JPEG_FAILED (rc)) return rc;

    // Not include length
    length -= 2;

    while (length > 16)
    {
        // Get Tc, Th
        rc = fetch_byte(ptr, size, p_offset, &index);
        if (index & 0xEC)
        {
            // Bad DHT table class and index
            return JPEGERR_EFAILED;
        }
        if (index & 0x10)
            index = (index & 0x3) + 4;

        // Set the bit flag to indicate which table are read
        *p_htable_present_flag |= (1 << index);

        htables[index].bits[0] = 0;

        sum = 0;
        for (j = 1; j < 17; j++)
        {
            // Get Li
            rc = fetch_byte(ptr, size, p_offset, &(htables[index].bits[j]));
            if (JPEG_FAILED (rc)) return rc;

            sum = sum + (uint16_t)(htables[index].bits[j]);
        }

        if (sum > 256)
        {
            // Bad DHT cnt
            return JPEGERR_EFAILED;
        }

        for (j = 0; j < sum; j++)
        {
            // Get Vi,j
            rc = fetch_byte(ptr, size, p_offset, &(htables[index].values[j]));
            if (JPEG_FAILED (rc)) return rc;
        }

        // Subtract number of BITS and BITS total count
        length -= (17 + sum);
    }

    return JPEGERR_SUCCESS;
} /* process_DHT */

static int process_DRI
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint32_t           *p_restart
)
{
    uint16_t  length, restart;
    int       rc;

    rc = fetch_2bytes(ptr, size, p_offset, &length);

    if (length != 4)
        return JPEGERR_EFAILED;

    rc = fetch_2bytes(ptr, size, p_offset, &restart);
    *p_restart = (uint32_t)restart;
    return rc;
} /* process_DRI */

static int parse_SOF
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint32_t           *p_width,                // SOF
    uint32_t           *p_height,               // SOF
    jpeg_subsampling_t *p_subsample,            // SOF
    uint8_t             qtable_selectors[],     // SOF
    uint16_t            qtables[][64],          // DQT
    uint32_t           *p_qtable_present_flag,  // DQT
    uint8_t             htable_dc_selectors[],  // SOS
    uint8_t             htable_ac_selectors[],  // SOS
    jpeg_huff_table_t   htables[],              // DHT
    uint32_t           *p_htable_present_flag,  // DHT
    uint32_t           *p_restart               // DRI
)
{
    int rc;

    for (;;)
    {
        uint8_t marker;

        rc = find_next_header(ptr, size, p_offset, &marker);
        if (JPEG_FAILED (rc)) return rc;

        switch (marker)
        {
        case M_SOF0:
            rc = process_SOF(ptr, size, p_offset, p_width, p_height, p_subsample, qtable_selectors);
            if (JPEG_FAILED (rc)) return rc;
            break;

        case M_SOI:
            // not expected to see SOI again, return failure
            return JPEGERR_EFAILED;

        case M_EOI:
            // not expected to see EOI, return failure
            return JPEGERR_EFAILED;

        case M_SOS:
            rc = process_SOS(ptr, size, p_offset, htable_dc_selectors, htable_ac_selectors);
            if (JPEG_FAILED (rc)) return rc;
            return JPEGERR_SUCCESS;

        case M_DQT:
            rc = process_DQT(ptr, size, p_offset, qtables, p_qtable_present_flag);
            if (JPEG_FAILED (rc)) return rc;
            break;

        case M_DHT:
            rc = process_DHT(ptr, size, p_offset, htables, p_htable_present_flag);
            if (JPEG_FAILED (rc)) return rc;
            break;

        case M_DRI:
            rc = process_DRI(ptr, size, p_offset, p_restart);
            if (JPEG_FAILED (rc)) return rc;
            break;

        default:
            {
                // Skip unsupported marker
                uint16_t bytes_to_skip;

                rc = fetch_2bytes(ptr, size, p_offset, &bytes_to_skip);

                if (bytes_to_skip >= 2)
                    bytes_to_skip -= 2;

                fprintf(stderr, "Skipping unsupported marker (FF%X)...\n", marker);
                *p_offset += bytes_to_skip;
                break;
            }
        } // end switch (marker)
    } // end for (;;)
} /* parse_SOF */

static int scale_quant_table
(
    uint16_t       dst_quant_tbl[],
    const uint16_t src_quant_tbl[],
    uint32_t       quality_factor
)
{
    double    scale_factor;
    int       i;

    // Validate input pointers
    if (!dst_quant_tbl || !src_quant_tbl)
        return JPEGERR_ENULLPTR;

    // 50% is equalivalent to no scaling
    if (quality_factor == 50)
    {
        memcpy(dst_quant_tbl, src_quant_tbl, (64 * sizeof(uint16_t)));
        return JPEGERR_SUCCESS;
    }

    // CLAMP(quality_factor, 1, 98);
    if (quality_factor < 1)
        quality_factor = 1;
    if (quality_factor > 98)
        quality_factor = 98;

    // Turn quality percent into scale factor
    if (quality_factor > 50)
    {
        scale_factor = 50.0 / (double) (100 - quality_factor);
    }
    else
    {
        scale_factor = (double) quality_factor / 50.0;
    }

    // Scale quant entries
    for (i = 0; i < 64; i++)
    {
        // Compute new value based on input percent
        // and on the 50% table (low)
        // Add 0.5 after the divide to round up fractional divides to be
        // more conservative.
        dst_quant_tbl[i] = (uint16_t) (((double)src_quant_tbl[i] / scale_factor) + 0.5);

        // CLAMP(dst_quant_tbl[i], 1, 255);
        if (dst_quant_tbl[i] < 1)
            dst_quant_tbl[i] = 1;
        if (dst_quant_tbl[i] > 255)
            dst_quant_tbl[i] = 255;
    }
    return JPEGERR_SUCCESS;
} /* scale_quant_table */

/*------------------------------------------------------------------------
 *                          Externalized Function Definitions
 * ----------------------------------------------------------------------- */

int jpeg_header_write
(
    uint8_t            *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint32_t            width,          // SOF
    uint32_t            height,         // SOF
    jpeg_subsampling_t  subsample,      // SOF
    uint32_t            quality,        // DQT
    uint16_t            qtables[][64],  // DQT
    jpeg_huff_table_t  *p_htables[],    // DHT
    uint32_t            restart,        // DRI
    uint32_t            rotation
)
{
    uint16_t  length, i;
    int       rc;

    if (!ptr || !p_offset)
        return JPEGERR_ENULLPTR;

    jpeg_header.ptr = ptr;
    jpeg_header.size = size;
    jpeg_header.offset = 0;
    *p_offset = 0;

    rc = emit_marker(&jpeg_header, M_SOI);
    if (JPEG_FAILED(rc)) return rc;

    rc = emit_SOF(&jpeg_header, width, height, subsample, rotation);
    if (JPEG_FAILED(rc)) return rc;

    if (qtables)
    {
        memcpy(luma_q_tbl,   qtables[0], sizeof(standard_luma_q_tbl));
        memcpy(chroma_q_tbl, qtables[1], sizeof(standard_chroma_q_tbl));
    }
    else
    {
        rc = scale_quant_table(luma_q_tbl, standard_luma_q_tbl, quality);
        if (JPEG_FAILED(rc)) return rc;
        rc = scale_quant_table(chroma_q_tbl, standard_chroma_q_tbl, quality);
        if (JPEG_FAILED(rc)) return rc;
    }
    rc = emit_DQT(&jpeg_header, luma_q_tbl, chroma_q_tbl);
    if (JPEG_FAILED(rc)) return rc;

    rc = emit_marker(&jpeg_header, M_DHT);
    if (JPEG_FAILED(rc)) return rc;

    length = 0;
    if (p_htables)
    {
        for (i = 1; i <= 16; i++)
        {
            length = length + (uint16_t)(p_htables[0]->bits[i]);  //luma_dc_huff_tbl.bits[i];
            length = length + (uint16_t)(p_htables[4]->bits[i]);  //luma_ac_huff_tbl.bits[i];
            length = length + (uint16_t)(p_htables[1]->bits[i]);  //chroma_dc_huff_tbl.bits[i];
            length = length + (uint16_t)(p_htables[5]->bits[i]);  //chroma_ac_huff_tbl.bits[i];
        }
    }
    else
    {
        for (i = 1; i <= 16; i++)
        {
            length = length + (uint16_t)(standard_luma_dc_huff_tbl.bits[i]);
            length = length + (uint16_t)(standard_luma_ac_huff_tbl.bits[i]);
            length = length + (uint16_t)(standard_chroma_dc_huff_tbl.bits[i]);
            length = length + (uint16_t)(standard_chroma_ac_huff_tbl.bits[i]);
        }
    }

    // Lh
    emit_2bytes(&jpeg_header, (uint16_t)(2 + length + 17*4));

    if (p_htables)
    {
        rc = emit_DHT(&jpeg_header, p_htables[0], 0);           // luma_dc_huff_tbl
        if (JPEG_FAILED(rc)) return rc;
        rc = emit_DHT(&jpeg_header, p_htables[4], (0 | 0x10));  // luma_ac_huff_tbl
        if (JPEG_FAILED(rc)) return rc;
        rc = emit_DHT(&jpeg_header, p_htables[1], 1);           // chroma_dc_huff_tbl
        if (JPEG_FAILED(rc)) return rc;
        rc = emit_DHT(&jpeg_header, p_htables[5], (1 | 0x10));  // chroma_ac_huff_tbl
        if (JPEG_FAILED(rc)) return rc;
    }
    else
    {
        rc = emit_DHT(&jpeg_header, (jpeg_huff_table_t *)&(standard_luma_dc_huff_tbl), 0);
        if (JPEG_FAILED(rc)) return rc;
        rc = emit_DHT(&jpeg_header, (jpeg_huff_table_t *)&(standard_luma_ac_huff_tbl), (0 | 0x10));
        if (JPEG_FAILED(rc)) return rc;
        rc = emit_DHT(&jpeg_header, (jpeg_huff_table_t *)&(standard_chroma_dc_huff_tbl), 1);
        if (JPEG_FAILED(rc)) return rc;
        rc = emit_DHT(&jpeg_header, (jpeg_huff_table_t *)&(standard_chroma_ac_huff_tbl), (1 | 0x10));
        if (JPEG_FAILED(rc)) return rc;
    }

    rc = emit_DRI(&jpeg_header, (uint8_t)restart);
    if (JPEG_FAILED(rc)) return rc;

    rc =  emit_SOS(&jpeg_header);
    if (JPEG_FAILED(rc)) return rc;

    *p_offset = jpeg_header.offset;
    return JPEGERR_SUCCESS;
} /* jpeg_header_write */

int jpeg_header_read
(
    const uint8_t      *ptr,
    uint32_t            size,
    uint32_t           *p_offset,
    uint32_t           *p_width,                // SOF
    uint32_t           *p_height,               // SOF
    jpeg_subsampling_t *p_subsample,            // SOF
    uint8_t             qtable_selectors[],     // SOF
    uint16_t            qtables[][64],          // DQT
    uint32_t           *p_qtable_present_flag,  // DQT
    uint8_t             htable_dc_selectors[],  // SOS
    uint8_t             htable_ac_selectors[],  // SOS
    jpeg_huff_table_t   htables[],              // DHT
    uint32_t           *p_htable_present_flag,  // DHT
    uint32_t           *p_restart               // DRI
)
{
    int                 rc;

    if (!ptr || !p_offset)
        return JPEGERR_ENULLPTR;

    *p_offset = 0;
    rc = find_SOI(ptr, size, p_offset);
    if (JPEG_FAILED(rc)) return rc;

    *p_restart = 0;
    *p_qtable_present_flag = 0;
    *p_htable_present_flag = 0;
    rc = parse_SOF(ptr, size, p_offset,
                   p_width, p_height, p_subsample, qtable_selectors,  // SOF
                   qtables, p_qtable_present_flag,                    // DQT
                   htable_dc_selectors, htable_ac_selectors,          // SOS
                   htables, p_htable_present_flag,                    // DHT
                   p_restart);                                        // DRI

    return rc;
} /* jpeg_header_read */

