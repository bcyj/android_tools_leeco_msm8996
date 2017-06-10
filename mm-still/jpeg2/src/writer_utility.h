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
#include <stdlib.h>
#include <os_int.h>

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define M_SOF0    0xc0
#define M_SOF1    0xc1
#define M_SOF2    0xc2
#define M_SOF3    0xc3

#define M_SOF5    0xc5
#define M_SOF6    0xc6
#define M_SOF7    0xc7

#define M_JPG     0xc8
#define M_SOF9    0xc9
#define M_SOF10   0xca
#define M_SOF11   0xcb

#define M_SOF13   0xcd
#define M_SOF14   0xce
#define M_SOF15   0xcf

#define M_DHT     0xc4

#define M_DAC     0xcc

#define M_RST0    0xd0
#define M_RST1    0xd1
#define M_RST2    0xd2
#define M_RST3    0xd3
#define M_RST4    0xd4
#define M_RST5    0xd5
#define M_RST6    0xd6
#define M_RST7    0xd7

#define M_SOI     0xd8
#define M_EOI     0xd9
#define M_SOS     0xda
#define M_DQT     0xdb
#define M_DNL     0xdc
#define M_DRI     0xdd
#define M_DHP     0xde
#define M_EXP     0xdf

#define M_APP0    0xe0
#define M_APP1    0xe1
#define M_APP2    0xe2
#define M_APP3    0xe3
#define M_APP4    0xe4
#define M_APP5    0xe5
#define M_APP6    0xe6
#define M_APP7    0xe7
#define M_APP8    0xe8
#define M_APP9    0xe9
#define M_APP10   0xea
#define M_APP11   0xeb
#define M_APP12   0xec
#define M_APP13   0xed
#define M_APP14   0xee
#define M_APP15   0xef

#define M_JPG0    0xf0
#define M_JPG13   0xfd
#define M_COM     0xfe

#define M_TEM     0x01


/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
static const int tag_type_sizes[11] = {
    0, // not applicable
    1, // EXIF_BYTE
    1, // EXIF_ASCII
    2, // EXIF_SHORT
    4, // EXIF_LONG
    8, // EXIF_RATIONAL
    0, // not applicable
    1, // EXIF_UNDEFINED
    0, // not applicable
    4, // EXIF_SLONG
    8, // EXIF_SRATIONAL
};

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
                           uint32_t offset, uint32_t size, uint8_t *overflow);

void jpegw_overwrite_long(int value, uint8_t* buffer,
                          uint32_t offset, uint32_t size, uint8_t *overflow);

void jpegw_emit_byte(uint8_t value, uint8_t* buffer,
                     uint32_t *offset, uint32_t size, uint8_t *overflow);

void jpegw_emit_short(uint16_t value, uint8_t* buffer,
                      uint32_t *offset, uint32_t size, uint8_t *overflow);

void jpegw_emit_long(int value, uint8_t* buffer,
                     uint32_t *offset, uint32_t size, uint8_t *overflow);

void jpegw_emit_nbytes(const uint8_t* data, uint32_t count, uint8_t* buffer,
                       uint32_t *offset, uint32_t size, uint8_t *overflow);

uint16_t jpegw_read_short(const uint8_t* buffer, int bufOffset);

uint32_t jpegw_read_long(const uint8_t *buffer, int bufOffset);

void writer_overwrite_short_little(uint16_t value, uint8_t* buffer,
                           uint32_t offset, uint32_t size, uint8_t *overflow);

void writer_overwrite_long_little(int value, uint8_t* buffer,
                          uint32_t offset, uint32_t size, uint8_t *overflow);

void writer_emit_short_little(uint16_t value, uint8_t* buffer,
                      uint32_t *offset, uint32_t size, uint8_t *overflow);

void writer_emit_long_little(int value, uint8_t* buffer,
                     uint32_t *offset, uint32_t size, uint8_t *overflow);

void writer_emit_nbytes_little(const uint8_t* data, uint32_t count,
  uint8_t* buffer, uint32_t *offset, uint32_t size, uint8_t *overflow);

uint16_t writer_read_short_little(const uint8_t* buffer, int bufOffset);

uint32_t writer_read_long_little(const uint8_t *buffer, int bufOffset);

/*  big Endian */
#define WRITE_BYTE_SCRATCH(b) \
  jpegw_emit_byte((uint8_t)(b), p_writer->scratchBuf->ptr, \
  &(p_writer->scratchBuf->offset), \
  p_writer->scratchBuf->size, &(p_writer->overflow_flag))

#define WRITE_SHORT_SCRATCH(s) \
  jpegw_emit_short((uint16_t)(s), p_writer->scratchBuf->ptr, \
  &(p_writer->scratchBuf->offset), \
  p_writer->scratchBuf->size, &(p_writer->overflow_flag))

#define WRITE_LONG_SCRATCH(l) \
  jpegw_emit_long((int)(l), p_writer->scratchBuf->ptr, \
  &(p_writer->scratchBuf->offset), \
  p_writer->scratchBuf->size, &(p_writer->overflow_flag))

#define WRITE_NBYTES_SCRATCH(b, c) \
  jpegw_emit_nbytes((uint8_t *)(b), c, p_writer->scratchBuf->ptr, \
  &(p_writer->scratchBuf->offset), \
  p_writer->scratchBuf->size, &(p_writer->overflow_flag))

#define WRITE_BYTE_AHEAD(b) \
  jpegw_emit_byte((uint8_t)(b), p_writer->aheadBuf->ptr, \
  &(p_writer->aheadBuf->offset), \
  p_writer->aheadBuf->size, &(p_writer->overflow_flag))

#define WRITE_SHORT_AHEAD(s) \
  jpegw_emit_short((uint16_t)(s), p_writer->aheadBuf->ptr, \
  &(p_writer->aheadBuf->offset), \
  p_writer->aheadBuf->size, &(p_writer->overflow_flag))

#define WRITE_LONG_AHEAD(l) \
  jpegw_emit_long((int)(l), p_writer->aheadBuf->ptr, \
  &(p_writer->aheadBuf->offset), \
  p_writer->aheadBuf->size, &(p_writer->overflow_flag))

#define WRITE_NBYTES_AHEAD(b, c) \
  jpegw_emit_nbytes((uint8_t *)(b), c, p_writer->aheadBuf->ptr, \
  &(p_writer->aheadBuf->offset), \
  p_writer->aheadBuf->size, &(p_writer->overflow_flag))


/* little Endian */
#define WRITE_SHORT_SCRATCH_LITTLE(s) \
  writer_emit_short_little((uint16_t)(s), p_writer->scratchBuf->ptr, \
  &(p_writer->scratchBuf->offset), \
  p_writer->scratchBuf->size, &(p_writer->overflow_flag))

#define WRITE_LONG_SCRATCH_LITTLE(l) \
  writer_emit_long_little((int)(l), p_writer->scratchBuf->ptr, \
  &(p_writer->scratchBuf->offset), \
  p_writer->scratchBuf->size, &(p_writer->overflow_flag))

#define WRITE_NBYTES_SCRATCH_LITTLE(b, c) \
  writer_emit_nbytes_little((uint8_t *)(b), c, p_writer->scratchBuf->ptr, \
  &(p_writer->scratchBuf->offset), \
  p_writer->scratchBuf->size, &(p_writer->overflow_flag))


#define WRITE_SHORT_AHEAD_LITTLE(s) \
  writer_emit_short_little((uint16_t)(s), p_writer->aheadBuf->ptr, \
  &(p_writer->aheadBuf->offset), \
  p_writer->aheadBuf->size, &(p_writer->overflow_flag))

#define WRITE_LONG_AHEAD_LITTLE(l) \
  writer_emit_long_little((int)(l), p_writer->aheadBuf->ptr, \
  &(p_writer->aheadBuf->offset), \
  p_writer->aheadBuf->size, &(p_writer->overflow_flag))

#define WRITE_NBYTES_AHEAD_LITTLE(b, c) \
  writer_emit_nbytes_little((uint8_t *)(b), c, p_writer->aheadBuf->ptr, \
  &(p_writer->aheadBuf->offset), \
  p_writer->aheadBuf->size, &(p_writer->overflow_flag))
