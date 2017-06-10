/* Copyright (c) 2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* =======================================================================

                INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_buffer_private.h"
#include "jpeg_writer.h"
#include "jpegd.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include <stdlib.h>
#include "writer_utility.h"
#ifdef USE_MERCURY
  #include "jpegd_engine_hw.h"
#else
  #include "jpegd_engine_hw_10.h"
#endif


/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
extern const int32_t zag[64];
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
//extern int32_t jpegd_engine_hw_huff_extend(int32_t nResidue,
//                                           int32_t nResidueSize);
//extern void    jpegd_engine_hw_derive_huff(uint8_t * bits,
//                                           uint8_t * val,
//                                           jpegd_hw_derived_htable_t * derivedTable,
//                                           int32_t *scratch);
//extern void    jpegd_engine_hw_ac_huff_decode(jpegd_hw_bitbuffer_t *p_engine,
//                                              jpegd_hw_derived_htable_t * hTable,
//                                              int32_t * symbol,
//                                              int32_t * codeSize);
//extern int32_t jpegd_engine_hw_get_residue_huff_extend (jpegd_hw_bitbuffer_t *p_engine,
//                                                        int32_t nCodeSize,
//                                                        int32_t nResidueSize);

/* =======================================================================
**                       Macro/Constant Definitions
** ======================================================================= */
/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/**************************************************************************
FUNCTION        jpegd_engine_hw_get_one_byte
---------------------------------------------------------------------------
DESCRIPTION     get one byte from the input jpeg bitstream buffer
---------------------------------------------------------------------------
INPUT  VALUE    none
---------------------------------------------------------------------------
RETURN VALUE    the obtained byte
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
uint8_t jpegd_engine_hw_get_one_byte(jpegd_engine_hw_t *p_engine)
{
    /**********************************************************************
        * Local variable
        *********************************************************************/
    int rc = JPEGERR_SUCCESS;
    uint8_t oneByte;

    /**********************************************************************
        * First, check if bitstream buffer is empty, if so, fill the bitsteam
        * buffer first
        *********************************************************************/
    if (p_engine->input_fetcher.num_bytes_left == 0) {
        rc = jpegd_engine_input_fetcher_fetch(&p_engine->input_fetcher);
        /******************************************************************
            * Now test if bitstream buffer is empty again, if still empty,
            * meaning the input file is eof, so pad it with EOI (0xFFD9), and
            * set the isPadded flag to indicate that the big read is padded
            *****************************************************************/
        if (p_engine->input_fetcher.num_bytes_left == 0) {
            p_engine->fIsPadded = 1;
            p_engine->fPaddingFlag ^= 1;

            if (JPEG_FAILED(rc)) {
                p_engine->fInputError = 1;
            }

            if (p_engine->fPaddingFlag == 0) {
                return 0xD9;
            } else {
                return 0xFF;
            }
        }
    }

    /**********************************************************************
        * Otherwise, read out one byte as unsigned int32_t, and update the
        * bitstream buffer and number of bytes left in the buffer, set
        * isPadded flag to 0, and return
        *********************************************************************/
    p_engine->fIsPadded = 0;
    oneByte = *(p_engine->input_fetcher.p_fetched_bytes++);
    p_engine->input_fetcher.num_bytes_left--;
    p_engine->input_fetcher.num_bytes_fetched++;

    return oneByte;
}

/**************************************************************************
FUNCTION        jpegd_engine_hw_get_one_padded_byte
---------------------------------------------------------------------------
DESCRIPTION     get one padded byte from bit buffer, pad it if a marker is
                                                                read;
---------------------------------------------------------------------------
INPUT  VALUE    none
---------------------------------------------------------------------------
RETURN VALUE    the obtained padded byte
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
uint8_t jpegd_engine_hw_get_one_padded_byte(jpegd_engine_hw_t *p_engine)
{
    /**********************************************************************
        * Local variables
        *********************************************************************/
    uint8_t byteRead;

    // first read the byte.
    byteRead = jpegd_engine_hw_get_one_byte(p_engine);

    // check if this is a 0xFF, could be maker
    if (byteRead == 0xFF) {
        if (p_engine->fIsPadded != 0) {
            return 0xFF;
        }

        // read in next byte
        byteRead = jpegd_engine_hw_get_one_byte(p_engine);

        if (p_engine->fIsPadded != 0) {
            jpegd_engine_hw_put_one_byte(p_engine, 0xFF);
            return 0xFF;
        }

        // if it is 00, then we have 0xff00, return ff, discard the 00
        // in bitstream, 00 is padded bits.
        if (byteRead == 0x0) {
            return 0xFF;
        } else {
            // if not, we have a marker, and put it back and tell the
            // caller that we have a marker.
            jpegd_engine_hw_put_one_byte(p_engine, byteRead);
            jpegd_engine_hw_put_one_byte(p_engine, 0xFF);
            return 0xFF;
        }
    }

    return byteRead;
}

/**************************************************************************
FUNCTION        jpegd_engine_hw_get_one_valid_byte
---------------------------------------------------------------------------
DESCRIPTION     get one valid byte from bit buffer, removing 0xFF00 from
                                  bitstream that was read in
---------------------------------------------------------------------------
INPUT  VALUE    none
---------------------------------------------------------------------------
RETURN VALUE    the obtained valid byte
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
uint8_t jpegd_engine_hw_get_one_valid_byte (jpegd_engine_hw_t *p_engine)
{
    /**********************************************************************
        * Local variables
        *********************************************************************/
    uint8_t byteRead;

    // first read the byte.
    byteRead = jpegd_engine_hw_get_one_byte(p_engine);

    // check if this is a 0xFF, could be maker
    if (byteRead == 0xFF) {
        if (p_engine->fIsPadded != 0) {
            return 0xFF;
        }

        // read in next byte
        byteRead = jpegd_engine_hw_get_one_byte(p_engine);

        if (p_engine->fIsPadded != 0) {
            jpegd_engine_hw_put_one_byte(p_engine, 0xFF);
            return 0xFF;
        }

        // if it is 00, then we have 0xff00, return ff, discard the 00
        // in bitstream, 00 is padded bits.
        if (byteRead == 0x0) {
            return 0xFF;
        } else {
            jpegd_engine_hw_put_one_byte(p_engine, byteRead);
            return 0xFF;
        }
    }

    /**********************************************************************
        * end of GetOnePaddedByte, and return the byte read.
        *********************************************************************/
    return byteRead;
}


/**************************************************************************
FUNCTION        jpegd_engine_hw_put_one_byte
---------------------------------------------------------------------------
DESCRIPTION     put one byte to the input jpeg bitstream buffer
---------------------------------------------------------------------------
INPUT  VALUE    byte: byte to put
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           None
**************************************************************************/
void jpegd_engine_hw_put_one_byte(jpegd_engine_hw_t *p_engine, uint8_t byte)
{
    /**********************************************************************
        * put one byte back to bitsteam and increase bytes left count
        *********************************************************************/
    *(--(p_engine->input_fetcher.p_fetched_bytes)) = byte;
    p_engine->input_fetcher.num_bytes_left++;
}

/**************************************************************************
FUNCTION        jpegd_engine_hw_round_shift_left
---------------------------------------------------------------------------
DESCRIPTION     round shift buffer to the left for number of bits
---------------------------------------------------------------------------
INPUT  VALUE    bitBuf: 32 bit buffer to be rotated
                        shift : length of the shift
---------------------------------------------------------------------------
RETURN VALUE    bitBuf
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
uint32_t jpegd_engine_hw_round_shift_left(uint32_t bitbuf, int32_t shift)
{
    /**********************************************************************
        * perform circular shift left and return value. assumming 32 bit buffer
        *********************************************************************/
    return( (bitbuf << shift) | (bitbuf >> (JPEGD_BIT_BUFFER_LENGTH - shift)));
}


/**************************************************************************
FUNCTION        jpegd_engine_hw_get_bits
---------------------------------------------------------------------------
DESCRIPTION     get number of bits from bit buffer
---------------------------------------------------------------------------
INPUT  VALUE    numOfBits: number of bits to get
                                         (*bits)  : pointer to bits got
---------------------------------------------------------------------------
RETURN VALUE    the bit buffer
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
uint32_t jpegd_engine_hw_get_bits(jpegd_engine_hw_t *p_engine, int32_t numOfBits)
{
    /**********************************************************************
        * Local variable
        *********************************************************************/
    uint8_t byte1, byte2;
    uint32_t bits;

    /**********************************************************************
        * get bit [15, 15-numOfBits] from the bit buffer.
        *********************************************************************/
    bits = (p_engine->bitbuffer.bit_accumulator >> (32 - numOfBits))
    & ( (1 << numOfBits) - 1);

    /**********************************************************************
        * Always Left shift for left Alignment
        *********************************************************************/
    p_engine->bitbuffer.bit_accumulator <<= numOfBits;

    /**********************************************************************
        * now check if remaining bits is less than half, if so, read two more
        * bytes from the input buffer and reload, and update bits left.
        *********************************************************************/
    if ((p_engine->bitbuffer.bits_left -= numOfBits) <= 16) {
        // read two bytes from input buffer
        byte1 = jpegd_engine_hw_get_one_valid_byte(p_engine);
        byte2 = jpegd_engine_hw_get_one_valid_byte(p_engine);

        // put two read bytes to the buffer.
        p_engine->bitbuffer.bit_accumulator =
            (p_engine->bitbuffer.bit_accumulator |
            ( ( (uint32_t) byte1) << (24 - p_engine->bitbuffer.bits_left)) |
            ( ( (uint32_t) byte2) << (16 - p_engine->bitbuffer.bits_left)));

        // update bitsLeft.
        p_engine->bitbuffer.bits_left += JPEGD_BIT_BUFFER_LENGTH_HALF;
    }

    return bits;
}

/**************************************************************************
FUNCTION        jpegd_engine_hw_get_padded_bits
---------------------------------------------------------------------------
DESCRIPTION     get bits from bit buffer, pad it if a marker is read;
---------------------------------------------------------------------------
INPUT  VALUE    numOfBits: number of bits to get
                 (*bits)  : pointer to bits got
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
uint32_t jpegd_engine_hw_get_padded_bits(jpegd_engine_hw_t *p_engine,
    int32_t numOfBits)
{
    /**********************************************************************
        * Local variables
        *********************************************************************/
    uint8_t  byte1, byte2;
    uint32_t bits;

    /**********************************************************************
        * get bit [31, 31-numOfBits] from the bit buffer.
        *********************************************************************/
    bits = (p_engine->bitbuffer.bit_accumulator >> (32 - numOfBits))
    & ((1 << numOfBits) - 1);

    /**********************************************************************
        * Always Left shift for left Alignment
        *********************************************************************/
    p_engine->bitbuffer.bit_accumulator <<= numOfBits;

    /**********************************************************************
        * now check if remaining bits is less than half, if so, read two more
        * bytes from the input buffer and reload, and update bits left;
        * if not, simply do rotate with out updating bits left.
        *********************************************************************/
    if ((p_engine->bitbuffer.bits_left -= numOfBits) <= 16) {           //use q6 conv.
        // read two bytes from input buffer
        byte1 = jpegd_engine_hw_get_one_padded_byte(p_engine);
        byte2 = jpegd_engine_hw_get_one_padded_byte(p_engine);

        // put two read bytes in buffer.
        p_engine->bitbuffer.bit_accumulator =
            (p_engine->bitbuffer.bit_accumulator|
            ( ( (uint32_t) byte1) << (24 - p_engine->bitbuffer.bits_left)) |
            ( ( (uint32_t) byte2) << (16 - p_engine->bitbuffer.bits_left)));

        //update bitsLeft.
        p_engine->bitbuffer.bits_left += JPEGD_BIT_BUFFER_LENGTH_HALF;
    }

    return bits;
}

/**************************************************************************
FUNCTION        jpegd_engine_hw_init_bit_buffer
---------------------------------------------------------------------------
DESCRIPTION     initialize bit buffer, fill in with jpeg data. This is done
                 at the begining of header parsing.
---------------------------------------------------------------------------
INPUT  VALUE    none
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           should be called only once for each decode session
**************************************************************************/
int jpegd_engine_hw_init_bit_buffer(jpegd_engine_hw_t *p_engine)
{
    uint32_t bits;
    int      rc;

    /**********************************************************************
        * Clear the EndOfInput flag, in the case of back to back decoding,
        * this flag is not cleared from the previous decode session.
        *********************************************************************/
    p_engine->input_fetcher.last_byte_fetched = false;

    /**********************************************************************
        * Then load up the input buffer and bit buffer
        *********************************************************************/
    // first fill the input buffer and set all input buffer parameter correct.
    rc = jpegd_engine_input_fetcher_fetch(&p_engine->input_fetcher);

    if (JPEG_SUCCEEDED(rc)) {
        /**********************************************************************
            * Set inital bitsLeft to 32
            * Then do two dummy read from input buffer to fill the bit buffer to 32
            * The first two read bits are garbage memory
            *********************************************************************/
        p_engine->bitbuffer.bits_left = 32;
        bits = jpegd_engine_hw_get_padded_bits(p_engine, 16);
        bits = jpegd_engine_hw_get_padded_bits(p_engine, 16);
    }

    return rc;
}


int jpegd_engine_hw_get_buf_in_use(jpegd_engine_hw_t *p_engine, jpeg_buf_t *p_buf)
{
    int rc;

    // first fill the input buffer and set all input buffer parameter correct.
    rc = jpegd_engine_fetcher_get_buf_in_use(&p_engine->input_fetcher, p_buf);

    return rc;
}


/**************************************************************************
FUNCTION        jpegd_engine_hw_reset_bit_buffer
---------------------------------------------------------------------------
DESCRIPTION     reset bit buffer, fill in with jpeg bitstreamdata. This is
                          done at the begining of bitstream decoding.
---------------------------------------------------------------------------
INPUT VALUE     None
---------------------------------------------------------------------------
RETURN VALUE    void
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_hw_reset_bit_buffer(jpegd_engine_hw_t *p_engine)
{
    /**********************************************************************
        * if any 0xFF is swallowed, put it back
        *********************************************************************/
    if (p_engine->bitbuffer.bits_left == 32) {
        p_engine->bitbuffer.bits_left = 32;
    } else if (p_engine->bitbuffer.bits_left >= 24) {
        p_engine->bitbuffer.bits_left = 24;
    } else {
        p_engine->bitbuffer.bits_left = 16;
    }
}


/**************************************************************************
FUNCTION        jpegd_engine_hw_get_next_marker
---------------------------------------------------------------------------
DESCRIPTION     locate and get next marker from bitstream, before SOS only
---------------------------------------------------------------------------
INPUT VALUE     (* marker): pointer to marker got
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           None
**************************************************************************/
int jpegd_engine_hw_get_next_marker(jpegd_engine_hw_t *p_engine,
    uint8_t * marker)
{
    /**********************************************************************
        * Local variable
        *********************************************************************/
    uint32_t bytesSkipped = 0;
    uint32_t bitsRead;

    /**********************************************************************
        * Markers are one or more 0xFF bytes followed by a marker bytes.
        * Sometimes markers are not where we expected it to be, i.e. there are
        * garbage data in between marker, we will skip it and issue an
        * warning.
        *
        * This applys to headers before SOS marker, in data partion of the
        * JPEG bitstream, 0xFF00 has to be tak care of, since it could be
        * data.
        *********************************************************************/
    // first located the 0xFF bytes, if it is not there, count the skipped
    // bytes
    bitsRead = jpegd_engine_hw_get_bits(p_engine, 8);

    *marker = (uint8_t)(bitsRead);

    while (*marker != 0xFF) {
        bytesSkipped++;
        bitsRead = jpegd_engine_hw_get_bits(p_engine, 8);
        *marker = (uint8_t)(bitsRead);
    }
    // now we found the 0xFF, see what the marker code is, remember there
    // might more more than one pair of 0xFF before the marker code
    do {
        bitsRead = jpegd_engine_hw_get_bits(p_engine, 8);
        *marker = (uint8_t)(bitsRead);
    } while (*marker == 0xFF);

    // Todo: if we skipped any garbage data, report it.
    // not sure what to do with this now, so leave it NULL
    if (bytesSkipped != 0) {
        JPEG_DBG_LOW("jpegd_engine_hw_get_next_marker: "
            "Garbage data skipped when search for marker\n");
    }

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_hw_skip_marker
---------------------------------------------------------------------------
DESCRIPTION     skip markers that are unknow or can not process, this refers
                  to markers with parameter and in the header portion only.
                  it uses marker length parameter to skip.
---------------------------------------------------------------------------
INPUT VALUE     None
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           None
**************************************************************************/
int jpegd_engine_hw_skip_marker(jpegd_engine_hw_t *p_engine)
{
    /**********************************************************************
        * Local variable
        *********************************************************************/
    uint32_t skipLength;
    uint32_t skipByte;

    /**********************************************************************
        * There are some markers that we do not care or do not want to process
        * so we skip it by read its length and read pass it.
        *********************************************************************/
    skipLength = jpegd_engine_hw_get_bits(p_engine, 16);

    if (skipLength < 2) {
        return JPEGERR_EFAILED;
    }

    // since we already read the skipLength in.
    skipLength -= 2;

    while (skipLength > 0) {
        // now read pass skipLength of bytes
        skipByte = jpegd_engine_hw_get_bits(p_engine, 8);
        skipLength--;
    }

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_hw_check_qtable
---------------------------------------------------------------------------
DESCRIPTION     initialize the correct quant table
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_hw_check_qtable(jpegd_engine_hw_t *p_engine)
{
    /**********************************************************************
        * Local variable
        *********************************************************************/
    uint32_t i;
    jpeg_frame_info_t *p_frame_info = p_engine->source.p_frame_info;

    // copy quant table from frame info
    for (i = 0; i < JPEGD_MAXQUANTTABLES; i++) {
        if ((p_frame_info->qtable_present_flag & (1 << i))) {
            if (!p_engine->pDeQuantTable[i]) {
                p_engine->pDeQuantTable[i] = (jpeg_quant_table_t)JPEG_MALLOC(64 * sizeof(uint16_t));
                if (!p_engine->pDeQuantTable[i])    return JPEGERR_EMALLOC;
            }
            (void)STD_MEMMOVE(p_engine->pDeQuantTable[i],
                p_engine->source.p_frame_info->p_qtables[i],
                64 * sizeof(uint16_t));
        }
    }

    // check if quant table is defined for each components
    // no quant table, no decoding.
    for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++) {
        if (!(p_frame_info->qtable_present_flag &
            (1 << p_engine->quantId[p_engine->compListInScan[i]]))) {
            JPEG_DBG_ERROR("jpegd_engine_hw_check_qtable: quant table %d missing\n",
                p_engine->quantId[p_engine->compListInScan[i]]);
            return JPEGERR_EFAILED;
        }
    }
    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_hw_check_htable
---------------------------------------------------------------------------
DESCRIPTION     initialize the correct huff table
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_hw_check_htable(jpegd_engine_hw_t *p_engine)
{
    /**********************************************************************
        * Local variable
        *********************************************************************/
    uint32_t i;
    jpeg_frame_info_t *p_frame_info = p_engine->source.p_frame_info;
    int32_t *htable_scratch_buffer = NULL;

    /**********************************************************************
        * first check if any huff table is missing.
        *********************************************************************/
    for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++) {
        if (p_engine->currentScanProgressiveInfo.ss == 0) {
            // The scan has DC
            if (!(p_frame_info->htable_present_flag &
                (1 << p_engine->dcHuffTableId[p_engine->compListInScan[i]]))) {
                JPEG_DBG_ERROR("jpegd_engine_hw_check_htable: dc huff table %d missing\n",
                    p_engine->dcHuffTableId[p_engine->compListInScan[i]]);
                return JPEGERR_EFAILED;
            }
        }
        if (p_engine->currentScanProgressiveInfo.se != 0) {
            // The scan has AC
            if (!(p_frame_info->htable_present_flag &
                (1 << p_engine->acHuffTableId[p_engine->compListInScan[i]]))) {
                JPEG_DBG_ERROR("jpegd_engine_hw_check_htable: ac huff table %d missing\n",
                    p_engine->acHuffTableId[p_engine->compListInScan[i]]);
                return JPEGERR_EFAILED;
            }
        }
    }

    /**********************************************************************
        *  then check if all required huff table are created, if not make them
        *********************************************************************/
    // allocated scratch buffer for the huff table extension. The intermediate
    // buffers can be pretty larget. and easy to overflow stacks.
    htable_scratch_buffer = (int32_t *)JPEG_MALLOC(900 * sizeof(int32_t));
    if (!htable_scratch_buffer) {
        return JPEGERR_EMALLOC;
    }

    // now extend huffman table
    for (i = 0; i < JPEGD_MAXHUFFTABLES; i++) {
        if (p_frame_info->htable_present_flag & (1 << i)) {
            // if the huffman code exist, but table is missing,
            // then make the table
            // first callocate memory for it.
            if (!p_engine->pDerivedHuffTable[i]) {
                p_engine->pDerivedHuffTable[i] = (jpegd_hw_derived_htable_t *)JPEG_MALLOC(
                    sizeof(jpegd_hw_derived_htable_t));
                if (!p_engine->pDerivedHuffTable) {
                    JPEG_FREE(htable_scratch_buffer);
                    return JPEGERR_EMALLOC;
                }
            }

            /* make corresponding derived huffman table
         jpegd_engine_hw_derive_huff(p_frame_info->p_htables[i].bits,
                                     p_frame_info->p_htables[i].values,
                                     p_engine->pDerivedHuffTable[i],
                                     htable_scratch_buffer);*/
        }
    }               // end for (i=0 ...

    JPEG_FREE(htable_scratch_buffer);
    /**********************************************************************
        * end CheckHuffTable
        *********************************************************************/
    return JPEGERR_SUCCESS;
}



