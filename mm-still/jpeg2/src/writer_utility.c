/*============================================================================

   Copyright  2011 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/



/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
01/20/11   staceyw Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "writer_utility.h"
#include "jpeg_common_private.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
void jpegw_overwrite_short(uint16_t value, uint8_t* buffer,
                           uint32_t offset, uint32_t size, uint8_t *overflow)
{
    if (offset + 1 >= size)
        *overflow = true;

    if (!(*overflow))
    {
        buffer[offset] = (uint8_t)((value >> 8) & 0xFF);
        buffer[offset+1] = (uint8_t)(value & 0xFF);
    }
}

void jpegw_overwrite_long(int value, uint8_t* buffer,
                          uint32_t offset, uint32_t size, uint8_t *overflow)
{
    if (offset + 3 >= size)
        *overflow = true;

    if (!(*overflow))
    {
        buffer[offset] = (uint8_t)((value >> 24) & 0xFF);
        buffer[offset+1] = (uint8_t)((value >> 16) & 0xFF);
        buffer[offset+2] = (uint8_t)((value >> 8) & 0xFF);
        buffer[offset+3] = (uint8_t)(value & 0xFF);
    }
}

void jpegw_emit_byte(uint8_t value, uint8_t* buffer,
                     uint32_t *offset, uint32_t size, uint8_t *overflow)
{
    if (*offset >= size)
        *overflow = true;

    if (!(*overflow))
    {
        buffer[*offset] = value;
        (*offset)++;
    }
}

void jpegw_emit_short(uint16_t value, uint8_t* buffer,
                      uint32_t *offset, uint32_t size, uint8_t *overflow)
{
    jpegw_overwrite_short(value, buffer, *offset, size, overflow);
    if (!(*overflow))
        (*offset) += 2;
}

void jpegw_emit_long(int value, uint8_t* buffer,
                     uint32_t *offset, uint32_t size, uint8_t *overflow)
{
    jpegw_overwrite_long(value, buffer, *offset, size, overflow);
    if (!(*overflow))
        (*offset) += 4;
}

void jpegw_emit_nbytes(const uint8_t* data, uint32_t count, uint8_t* buffer,
                       uint32_t *offset, uint32_t size, uint8_t *overflow)
{
    if (*offset + count >= size)
        *overflow = true;

    if (!(*overflow))
    {
        (void)STD_MEMMOVE(buffer + *offset, data, count);
        (*offset) += count;
    }
}

uint16_t jpegw_read_short(const uint8_t* buffer, int bufOffset)
{
    return (uint16_t) ( ( (uint16_t) buffer[bufOffset] << 8) + (uint16_t) buffer[bufOffset + 1]);
}

uint32_t jpegw_read_long(const uint8_t *buffer, int bufOffset)
{
    return (uint32_t) ( ( (uint32_t) buffer[bufOffset] << 24) +
                        ( (uint32_t) buffer[bufOffset + 1] << 16) +
                        ( (uint32_t) buffer[bufOffset + 2] << 8)  +
                        (uint32_t) buffer[bufOffset + 3]);
}

/* little Endian functions */
void writer_overwrite_short_little(uint16_t value, uint8_t* buffer,
                           uint32_t offset, uint32_t size, uint8_t *overflow)
{
    if (offset + 1 >= size)
        *overflow = true;

    if (!(*overflow))
    {
        buffer[offset+1] = (uint8_t)((value >> 8) & 0xFF);
        buffer[offset] = (uint8_t)(value & 0xFF);
    }
}

void writer_overwrite_long_little(int value, uint8_t* buffer,
                          uint32_t offset, uint32_t size, uint8_t *overflow)
{
    if (offset + 3 >= size)
        *overflow = true;

    if (!(*overflow))
    {
        buffer[offset+3] = (uint8_t)((value >> 24) & 0xFF);
        buffer[offset+2] = (uint8_t)((value >> 16) & 0xFF);
        buffer[offset+1] = (uint8_t)((value >> 8) & 0xFF);
        buffer[offset] = (uint8_t)(value & 0xFF);
    }
}

void writer_emit_short_little(uint16_t value, uint8_t* buffer,
                      uint32_t *offset, uint32_t size, uint8_t *overflow)
{
    writer_overwrite_short_little(value, buffer, *offset, size, overflow);
    if (!(*overflow))
        (*offset) += 2;
}

void writer_emit_long_little(int value, uint8_t* buffer,
                     uint32_t *offset, uint32_t size, uint8_t *overflow)
{
    writer_overwrite_long_little(value, buffer, *offset, size, overflow);
    if (!(*overflow))
        (*offset) += 4;
}

void writer_emit_nbytes_little(const uint8_t* data, uint32_t count,
  uint8_t* buffer, uint32_t *offset, uint32_t size, uint8_t *overflow)
{
    if (*offset + count >= size)
        *overflow = true;

    if (!(*overflow))
    {
        uint32_t count_no = count;
        while (count_no--)
            buffer[*offset+count_no] = *data++;

        (*offset) += count;
    }
}

uint16_t writer_read_short_little(const uint8_t* buffer, int bufOffset)
{
  return (uint16_t) ( ( (uint16_t) buffer[bufOffset+1] << 8) +
    (uint16_t) buffer[bufOffset]);
}

uint32_t writer_read_long_little(const uint8_t *buffer, int bufOffset)
{
  return (uint32_t) ( ( (uint32_t) buffer[bufOffset + 3] << 24) +
                      ( (uint32_t) buffer[bufOffset + 2] << 16) +
                      ( (uint32_t) buffer[bufOffset + 1] << 8)  +
                      (uint32_t) buffer[bufOffset]);
}
