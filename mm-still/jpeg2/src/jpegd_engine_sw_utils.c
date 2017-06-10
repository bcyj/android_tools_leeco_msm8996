/*========================================================================


*//** @file jpegd_engine_sw_utils.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-09 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
02/22/10   leim    Remove static variable in function 
                   jpegd_engine_sw_derive_huff, instead a scratch memory is
                   used from heap to reduce the stack usage.
10/15/09   mingy   Modified iDCT function signature.
09/21/09   mingy   Added consume MCU and consume Block functions for
                   region based decoding to skip areas outside the region.
07/20/09   zhiminl Added copying quant table from frame info into engine
                   in jpegd_engine_sw_check_qtable().
07/07/09   zhiminl Decoupled bit buffer from jpegd_engine_input_fetcher_t.
                   Added jpegd_engine_sw_init_bit_buffer().
05/21/09   mingy   Ported WinMobile optimizations.
                   2-step Huffman approach, left aligned bit buffer,
                   AC decoding method, etc.
03/05/09   leim    Fixed bug in outputing one mcu for h1v2 format.
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_buffer_private.h"
#include "jpegd_engine_sw.h"
#include "jpeg_writer.h"
#include "jpegd.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include <stdlib.h>
#include "writer_utility.h"


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
extern int32_t jpegd_engine_sw_huff_extend(int32_t nResidue, 
                                           int32_t nResidueSize);
extern void    jpegd_engine_sw_derive_huff(uint8_t * bits, 
                                           uint8_t * val,
                                           jpegd_derived_htable_t * derivedTable,
                                           int32_t *scratch);
extern void    jpegd_engine_sw_ac_huff_decode(jpegd_bitbuffer_t *p_engine,
                                              jpegd_derived_htable_t * hTable,
                                              int32_t * symbol,
                                              int32_t * codeSize);
extern int32_t jpegd_engine_sw_get_residue_huff_extend (jpegd_bitbuffer_t *p_engine,
                                                        int32_t nCodeSize,
                                                        int32_t nResidueSize);
void           jpegd_engine_sw_put_one_byte(jpegd_engine_sw_t *p_engine, 
                                            uint8_t byte);
void           jpegd_engine_sw_ac_code_residue_consumed (jpegd_engine_sw_t *p_engine, 
                                                         int32_t numOfBits);
void           jpegd_engine_sw_ac_bit_buffer_update (jpegd_engine_sw_t *p_engine, 
                                                     int32_t numOfBits);

/* =======================================================================
**                       Macro/Constant Definitions
** ======================================================================= */
/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/**************************************************************************
FUNCTION        jpegd_engine_sw_get_one_byte
---------------------------------------------------------------------------
DESCRIPTION     get one byte from the input jpeg bitstream buffer
---------------------------------------------------------------------------
INPUT  VALUE    none
---------------------------------------------------------------------------
RETURN VALUE    the obtained byte
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
uint8_t jpegd_engine_sw_get_one_byte(jpegd_engine_sw_t *p_engine)
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
    if (p_engine->input_fetcher.num_bytes_left == 0)
    {
        rc = jpegd_engine_input_fetcher_fetch(&p_engine->input_fetcher);
        /******************************************************************
         * Now test if bitstream buffer is empty again, if still empty,
         * meaning the input file is eof, so pad it with EOI (0xFFD9), and
         * set the isPadded flag to indicate that the big read is padded
         *****************************************************************/
        if (p_engine->input_fetcher.num_bytes_left == 0)
        {
            p_engine->fIsPadded = 1;
            p_engine->fPaddingFlag ^= 1;

            if (JPEG_FAILED(rc))
            {
                p_engine->fInputError = 1;
            }

            if (p_engine->fPaddingFlag == 0)
            {
                return 0xD9;
            }
            else
            {
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
FUNCTION        jpegd_engine_sw_get_one_padded_byte
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
uint8_t jpegd_engine_sw_get_one_padded_byte(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     * Local variables
     *********************************************************************/
    uint8_t byteRead;

    // first read the byte.
    byteRead = jpegd_engine_sw_get_one_byte(p_engine);

    // check if this is a 0xFF, could be maker
    if (byteRead == 0xFF)
    {
        if (p_engine->fIsPadded != 0)
        {
            return 0xFF;
        }

        // read in next byte
        byteRead = jpegd_engine_sw_get_one_byte(p_engine);

        if (p_engine->fIsPadded != 0)
        {
            jpegd_engine_sw_put_one_byte(p_engine, 0xFF);
            return 0xFF;
        }

        // if it is 00, then we have 0xff00, return ff, discard the 00
        // in bitstream, 00 is padded bits.
        if (byteRead == 0x0)
        {
            return 0xFF;
        }
        else
        {
            // if not, we have a marker, and put it back and tell the
            // caller that we have a marker.
            jpegd_engine_sw_put_one_byte(p_engine, byteRead);
            jpegd_engine_sw_put_one_byte(p_engine, 0xFF);
            return 0xFF;
        }
    }

    return byteRead;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_get_one_valid_byte
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
uint8_t jpegd_engine_sw_get_one_valid_byte (jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     * Local variables
     *********************************************************************/
    uint8_t byteRead;

    // first read the byte.
    byteRead = jpegd_engine_sw_get_one_byte(p_engine);

    // check if this is a 0xFF, could be maker
    if (byteRead == 0xFF)
    {
        if (p_engine->fIsPadded != 0)
        {
            return 0xFF;
        }

        // read in next byte
        byteRead = jpegd_engine_sw_get_one_byte(p_engine);

        if (p_engine->fIsPadded != 0)
        {
            jpegd_engine_sw_put_one_byte(p_engine, 0xFF);
            return 0xFF;
        }

        // if it is 00, then we have 0xff00, return ff, discard the 00
        // in bitstream, 00 is padded bits.
        if (byteRead == 0x0)
        {
            return 0xFF;
        }
        else
        {
            jpegd_engine_sw_put_one_byte(p_engine, byteRead);
            return 0xFF;
        }
    }

    /**********************************************************************
     * end of GetOnePaddedByte, and return the byte read.
     *********************************************************************/
    return byteRead;
}


/**************************************************************************
FUNCTION        jpegd_engine_sw_put_one_byte
---------------------------------------------------------------------------
DESCRIPTION     put one byte to the input jpeg bitstream buffer
---------------------------------------------------------------------------
INPUT  VALUE    byte: byte to put
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           None
**************************************************************************/
void jpegd_engine_sw_put_one_byte(jpegd_engine_sw_t *p_engine, uint8_t byte)
{
    /**********************************************************************
     * put one byte back to bitsteam and increase bytes left count
     *********************************************************************/
    *(--(p_engine->input_fetcher.p_fetched_bytes)) = byte;
    p_engine->input_fetcher.num_bytes_left++;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_round_shift_left
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
uint32_t jpegd_engine_sw_round_shift_left(uint32_t bitbuf, int32_t shift)
{
    /**********************************************************************
     * perform circular shift left and return value. assumming 32 bit buffer
     *********************************************************************/
    return ( (bitbuf << shift) | (bitbuf >> (JPEGD_BIT_BUFFER_LENGTH - shift)));
}


/**************************************************************************
FUNCTION        jpegd_engine_sw_get_bits
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
uint32_t jpegd_engine_sw_get_bits(jpegd_engine_sw_t *p_engine, int32_t numOfBits)
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
    if ( (p_engine->bitbuffer.bits_left -= numOfBits) <= 16)
    {
        // read two bytes from input buffer
        byte1 = jpegd_engine_sw_get_one_valid_byte(p_engine);
        byte2 = jpegd_engine_sw_get_one_valid_byte(p_engine);

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
FUNCTION        jpegd_engine_sw_get_padded_bits
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
uint32_t jpegd_engine_sw_get_padded_bits(jpegd_engine_sw_t *p_engine,
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
    if ( (p_engine->bitbuffer.bits_left -= numOfBits) <= 16) //use q6 conv.
    {
        // read two bytes from input buffer
        byte1 = jpegd_engine_sw_get_one_padded_byte(p_engine);
        byte2 = jpegd_engine_sw_get_one_padded_byte(p_engine);

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
FUNCTION        jpegd_engine_sw_init_bit_buffer
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
int jpegd_engine_sw_init_bit_buffer(jpegd_engine_sw_t *p_engine)
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

    if (JPEG_SUCCEEDED(rc))
    {
        /**********************************************************************
         * Set inital bitsLeft to 32
         * Then do two dummy read from input buffer to fill the bit buffer to 32
         * The first two read bits are garbage memory
         *********************************************************************/
        p_engine->bitbuffer.bits_left = 32;
        bits = jpegd_engine_sw_get_padded_bits(p_engine, 16);
        bits = jpegd_engine_sw_get_padded_bits(p_engine, 16);
    }

    return rc;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_reset_bit_buffer
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
void jpegd_engine_sw_reset_bit_buffer(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     * if any 0xFF is swallowed, put it back
     *********************************************************************/
    if (p_engine->bitbuffer.bits_left == 32)
    {
        p_engine->bitbuffer.bits_left = 32;
    }
    else if (p_engine->bitbuffer.bits_left >= 24)
    {
        p_engine->bitbuffer.bits_left = 24;
    }
    else
    {
        p_engine->bitbuffer.bits_left = 16;
    }
}


/**************************************************************************
FUNCTION        jpegd_engine_sw_get_next_marker
---------------------------------------------------------------------------
DESCRIPTION     locate and get next marker from bitstream, before SOS only
---------------------------------------------------------------------------
INPUT VALUE     (* marker): pointer to marker got
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           None
**************************************************************************/
int jpegd_engine_sw_get_next_marker(jpegd_engine_sw_t *p_engine,
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
    bitsRead = jpegd_engine_sw_get_bits(p_engine, 8);

    *marker = (uint8_t)(bitsRead);

    while (*marker != 0xFF)
    {
        bytesSkipped++;
        bitsRead = jpegd_engine_sw_get_bits(p_engine, 8);
        *marker = (uint8_t)(bitsRead);
    }
    // now we found the 0xFF, see what the marker code is, remember there
    // might more more than one pair of 0xFF before the marker code
    do
    {
        bitsRead = jpegd_engine_sw_get_bits(p_engine, 8);
        *marker = (uint8_t)(bitsRead);
    } while (*marker == 0xFF);

    // Todo: if we skipped any garbage data, report it.
    // not sure what to do with this now, so leave it NULL
    if (bytesSkipped != 0)
    {
        JPEG_DBG_LOW("jpegd_engine_sw_get_next_marker: "
                     "Garbage data skipped when search for marker\n");
    }

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_skip_marker
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
int jpegd_engine_sw_skip_marker(jpegd_engine_sw_t *p_engine)
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
    skipLength = jpegd_engine_sw_get_bits(p_engine, 16);

    if (skipLength < 2)
    {
        return JPEGERR_EFAILED;
    }

    // since we already read the skipLength in.
    skipLength -= 2;

    while (skipLength > 0)
    {
        // now read pass skipLength of bytes
        skipByte = jpegd_engine_sw_get_bits(p_engine, 8);
        skipLength--;
    }

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_check_qtable
---------------------------------------------------------------------------
DESCRIPTION     initialize the correct quant table
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_sw_check_qtable(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     * Local variable
     *********************************************************************/
    uint32_t i;
    jpeg_frame_info_t *p_frame_info = p_engine->source.p_frame_info;

    if (p_engine->nNumberOfComponentsInScan > JPEGD_MAXCOMPSINSCAN) {
      JPEG_DBG_ERROR("jpegd_engine_sw_check_qtable: Invalid number of"
                     "components %d \n", p_engine->nNumberOfComponentsInScan);
      return JPEGERR_EFAILED;
    }

    // copy quant table from frame info
    for (i = 0; i < JPEGD_MAXQUANTTABLES; i++)
    {
        if ((p_frame_info->qtable_present_flag & (1 << i)))
        {
            if (!p_engine->pDeQuantTable[i])
            {
                p_engine->pDeQuantTable[i] = (jpeg_quant_table_t)JPEG_MALLOC(64 * sizeof(uint16_t));
                if (!p_engine->pDeQuantTable[i]) return JPEGERR_EMALLOC;
            }
            (void)STD_MEMMOVE(p_engine->pDeQuantTable[i],
                              p_engine->source.p_frame_info->p_qtables[i],
                              64 * sizeof(uint16_t));
        }
    }

    // check if quant table is defined for each components
    // no quant table, no decoding.
    for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++)
    {
        if (!(p_frame_info->qtable_present_flag &
              (1 << p_engine->quantId[p_engine->compListInScan[i]])))
        {
            JPEG_DBG_ERROR("jpegd_engine_sw_check_qtable: quant table %d missing\n",
                           p_engine->quantId[p_engine->compListInScan[i]]);
            return JPEGERR_EFAILED;
        }
    }

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_check_htable
---------------------------------------------------------------------------
DESCRIPTION     initialize the correct huff table
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_sw_check_htable(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     * Local variable
     *********************************************************************/
    uint32_t i;
    jpeg_frame_info_t *p_frame_info = p_engine->source.p_frame_info;
    int32_t *htable_scratch_buffer = NULL;

    if (p_engine->nNumberOfComponentsInScan > JPEGD_MAXCOMPSINSCAN) {
      JPEG_DBG_ERROR("jpegd_engine_sw_check_htable: Invalid number of"
                     "components %d \n", p_engine->nNumberOfComponentsInScan);
      return JPEGERR_EFAILED;
    }

    /**********************************************************************
     * first check if any huff table is missing.
     *********************************************************************/
    for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++)
    {
        if (p_engine->currentScanProgressiveInfo.ss == 0)
        {
            // The scan has DC
            if (!(p_frame_info->htable_present_flag &
                  (1 << p_engine->dcHuffTableId[p_engine->compListInScan[i]])))
            {
                JPEG_DBG_ERROR("jpegd_engine_sw_check_htable: dc huff table %d missing\n",
                               p_engine->dcHuffTableId[p_engine->compListInScan[i]]);
                return JPEGERR_EFAILED;
            }
        }
        if (p_engine->currentScanProgressiveInfo.se != 0)
        {
            // The scan has AC
            if (!(p_frame_info->htable_present_flag &
                  (1 << p_engine->acHuffTableId[p_engine->compListInScan[i]])))
            {
                JPEG_DBG_ERROR("jpegd_engine_sw_check_htable: ac huff table %d missing\n",
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
    if (!htable_scratch_buffer)
    {
        return JPEGERR_EMALLOC;
    } 

    // now extend huffman table
    for (i = 0; i < JPEGD_MAXHUFFTABLES; i++)
    {
        if (p_frame_info->htable_present_flag & (1 << i))
        {
            // if the huffman code exist, but table is missing,
            // then make the table
            // first callocate memory for it.
            if (!p_engine->pDerivedHuffTable[i])
            {
                p_engine->pDerivedHuffTable[i] = (jpegd_derived_htable_t *)JPEG_MALLOC(
                    sizeof(jpegd_derived_htable_t));
                if (!p_engine->pDerivedHuffTable)
                {
                    JPEG_FREE(htable_scratch_buffer);
                    return JPEGERR_EMALLOC;
                }
            }

            // make corresponding derived huffman table
            jpegd_engine_sw_derive_huff(p_frame_info->p_htables[i].bits,
                                        p_frame_info->p_htables[i].values,
                                        p_engine->pDerivedHuffTable[i],
                                        htable_scratch_buffer);
        }
    } // end for (i=0 ...

    JPEG_FREE(htable_scratch_buffer);
    /**********************************************************************
     * end CheckHuffTable
     *********************************************************************/
    return JPEGERR_SUCCESS;
}


/**************************************************************************
FUNCTION        jpegd_engine_sw_dehuff_one_block
---------------------------------------------------------------------------
DESCRIPTION     decode block of coefficients; this routine also de-zigzag
                the coefficients order
---------------------------------------------------------------------------
INPUT VALUE     dctable: dc huffman table
                actable: ac huffman table
                qTable : quantization table
                (* lastVale): pointer to last dc value, this value will
                              be changed if dc coefficients are
                              successfully decoded
                coeffBuf: pointer to output coeffients buffer.
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_sw_dehuff_one_block(jpegd_engine_sw_t *p_engine,
                                     jpegd_derived_htable_t *dcTable,
                                     jpegd_derived_htable_t *acTable,
                                     jpeg_quant_table_t qTable,
                                     int32_t * lastDcVal,
                                     jpegd_coeff_t *coeffBuf,
                                     int32_t numOfValidCoeff)
{
    /**********************************************************************
     * Local variable
     *********************************************************************/
    int32_t  i;
    int32_t  r, s;

    uint32_t categoryCodeSize; //symbolCodesize
    int32_t  codeSize;
    int32_t  coefficient;

    jpegd_derived_htable_entry_t tableEntry;
    int32_t length;
    int16_t bitMask;

    uint32_t bitsRead;
    uint32_t nextBits;

    jpegd_bitbuffer_t *p_bitbuffer = &(p_engine->bitbuffer);

    // first clear the coeff buffer.
    STD_MEMSET(coeffBuf, 0, JPEGD_DCTSIZE2 * sizeof(jpegd_coeff_t));

    /**********************************************************************
     * get the dc coefficient.
     *********************************************************************/

    /**********************************************************************
     * m_jpegdBitBuffer.buffer is 32 bits buffer with left Alignment
     * it right shifts 24 bits to get the most left 8 bits
     * then lookup in the dcTable to find the catorgory and code length
     *********************************************************************/
    categoryCodeSize = (dcTable->nCategoryCodeSize[(p_bitbuffer->bit_accumulator>>24) & 0xFF]);

    /**********************************************************************
     * categoryCodeSize has 16 bits of catorgory and 16 bits of code length.
     * if it is greater than 0, the code length is less than or equal to 8 bits
     * otherwise, the code length is greater than 8 bits
     *********************************************************************/
    if (((int32_t)(categoryCodeSize)) > 0)
    {
        /******************************************************************
         * If the code length is less than or equal to 8 bits, reading out
         * the number of bits which is the lower 16 bits of categoryCodeSize
         *****************************************************************/
        nextBits = jpegd_engine_sw_get_padded_bits(p_engine, (int32_t)(categoryCodeSize & 0xFFFF));

        /******************************************************************
         * the category(residue size) is the most significant 16 bits
         * of categoryCodeSize
         ******************************************************************/
        s = (int32_t) (categoryCodeSize >> 16 );
    }
    else
    {
        /******************************************************************
         * If the code length is longer than 8 bits, the category is not in
         * the first nCategoryCodeSize table and a second search in the
         * symbol table is needed
         *****************************************************************/

        /******************************************************************
         * get the lower 16 bits, ang length is code length minus 8 bits
         *****************************************************************/
        length = categoryCodeSize & 0xFFFF;

        bitMask = (int16_t)((1 << length) - 1);

        /******************************************************************
         * code length is length plus 8
         * the category in the symbol table is equal to the inverse of
         * most significant 16 bits categoryCodeSize plus value of bits
         * that m_jpegdBitBuffer.buffer right shifts (24-length) bits
         *****************************************************************/
        tableEntry = dcTable->symbol[(uint16_t)(~( categoryCodeSize >> 16))
                    + ((p_bitbuffer->bit_accumulator >> (24-length)) & bitMask)];

        /******************************************************************
         * read out the number of bits (tableEntry.len ) in bit buffer
         *****************************************************************/
        nextBits = jpegd_engine_sw_get_padded_bits(p_engine, (int32_t)(tableEntry.len));

        /******************************************************************
         * read out catergory to s
        ******************************************************************/
        s = (int32_t)(tableEntry.cat);
    }

    if (s != 0)
    {
        /******************************************************************
         * if s is non-zero
         * 1. update buffer with number of bits (s)
         * 2. read out s bits to bitsRead which is residue
         * 3. make huffman extersion
         *****************************************************************/
        bitsRead = jpegd_engine_sw_get_padded_bits(p_engine, s);
        s        = jpegd_engine_sw_huff_extend(bitsRead, s);
    }

    *lastDcVal = (s += *lastDcVal);

    // the dc coeff is always coeff[0]
    coeffBuf[0] = (int16_t)( s * qTable[0]);

    /**********************************************************************
     * then decode the AC compoents with AC table.
     *********************************************************************/
    for (i = 1; i < numOfValidCoeff; i++)  //numOfValidCoeff
    {
        /******************************************************************
         * m_jpegdBitBuffer.buffer is 32 bits buffer with left Alignment
         * it right shifts 24 bits to get the most left 8 bits
         * then lookup in the dcTable to find the catorgory and code length
         *****************************************************************/
        categoryCodeSize = (acTable->nCategoryCodeSize[(p_bitbuffer->bit_accumulator>>24) & 0xFF]);

        /******************************************************************
        * categoryCodeSize: the catorgory and the length
        * if greater than 0, meaning the codeword length is less or equal to 8 bits
        * otherwise, the codeword length is greater than 8 bits
        ******************************************************************/
        if (((int32_t)(categoryCodeSize)) > 0)
        {
            /**************************************************************
             * If the code length is less than or equal to 8 bits, reading out
             * the number of bits which is the first 16 bits of categoryCodeSize
             *************************************************************/
            codeSize = (int32_t)(categoryCodeSize & 0xFFFF);

            /**************************************************************
             * the symbol(run size) is the most significant 16 bits
             * of categoryCodeSize
             *************************************************************/
            s = (int32_t)(categoryCodeSize >> 16);
        }
        else
        {
            /**************************************************************
             * If the code length is longer than 8 bits, the category is not in
             * the first nCategoryCodeSize table and a second search in the
             * symbol table is needed
             *************************************************************/

            /**************************************************************
             * get the lower 16 bits, ang length is code length minus 8 bits
             *************************************************************/
            length = categoryCodeSize & 0xFFFF;

            bitMask = (int16_t)((1 << length) - 1);

            /**************************************************************
             * code length is length plus 8
             * the category in the symbol table is equal to the inverse of
             * most significant 16 bits categoryCodeSize plus value of bits
             * that m_jpegdBitBuffer.buffer right shifts (24-length) bits
             *************************************************************/
            tableEntry = acTable->symbol[(uint16_t)(~(categoryCodeSize >> 16))
                        + ((p_bitbuffer->bit_accumulator >> (24-length)) & bitMask)];

            /**************************************************************
            * read out the number of bits (tableEntry.len ) in bit buffer
            **************************************************************/
            codeSize= (int32_t) (tableEntry.len);

            /**************************************************************
            * read out catergory to s
            **************************************************************/
            s = (int32_t) (tableEntry.cat);
        }

        /******************************************************************
         * r : run which is the high 4 bits
         * s:  size which is the size of residue
         *****************************************************************/
        r = s >> 4;
        s &= 0xF;

        if (s != 0)
        {
            if (r != 0)
            {
                // the run
                if ( (i+=r) > 63)
                {
                    JPEG_DBG_LOW("Huffman error, i=%d, r=%d, s=%d", i, r, s);
                    return JPEGERR_EFAILED;
                } // end if ((i+r)>63)
            } //end if (r!=0)

            /**************************************************************
             * the intermediate buffer guarantee to have at least 16 bits
             * if (codeSize + s(residue size) ) <= 16, can go ahead to
             * read out the residue,  after that if necessary, update the
             * bit buffer from bitstream
             *************************************************************/
            if ((codeSize + s) <= 16)
            {
                /**********************************************************
                 * this function is
                 * 1. updating the bit buffer m_jpegdBitBuffer
                 * 2. get the residue to s from bit m_jpegdBitBuffer
                 * 3. huffman extension
                 *********************************************************/
                coefficient = jpegd_engine_sw_get_residue_huff_extend (p_bitbuffer, codeSize, s);

                /**********************************************************
                 * update the m_jpegdBitBuffer
                 *********************************************************/
                jpegd_engine_sw_ac_code_residue_consumed (p_engine, s + codeSize);
            }
            else
            {
                /**********************************************************
                 * if (codeSize + s ) >16
                 * 1. update buffer with bits of codeSize
                 * 2. get s bits from bit buffer to bitsRead which is residue
                 * 3. make huffman extersion
                 *********************************************************/
                jpegd_engine_sw_ac_bit_buffer_update (p_engine, codeSize);

                bitsRead    = jpegd_engine_sw_get_padded_bits(p_engine, s);
                coefficient = jpegd_engine_sw_huff_extend(bitsRead, s);
            }

            /**************************************************************
             * doing unzigzag and dequant
             *************************************************************/
            coeffBuf[zag[i]] = (int16_t)(coefficient * qTable[i]);
        } // end if(s!=0)
        else
        {
            /**************************************************************
             * need to get codeSize bits for the bit buffer from bit stream
             *************************************************************/
            jpegd_engine_sw_ac_bit_buffer_update (p_engine, codeSize);

            // is EOB
            if (r == 15)
            {
                if ( (i+=r) >63)
                {
                    JPEG_DBG_LOW("Huffman error, i=%d, r=%d, s=%d", i, r, s);
                    return JPEGERR_EFAILED;
                } // end if ((i+r)>63)
            } // end if ( r == 15 )
            else
            {
                i = 64;  //eob = TRUE;
                break;
            }
        }
    } // end for ( i=1; i<numOfValidCoeff; i++ )

    /**********************************************************************
     * scalable jpeg decoding for 4x4, 2x2, 1x1 cases
     * only consuming the run & size and residue without
     * 1. decoding the residue
     * 2. huffman extend
     * 3. unzigzag + deQuant
     *********************************************************************/
    for (; i< 64; i++)
    {
        /******************************************************************
         * huffman decode the AC coefficients to s
         * and codeSize is codeword size
         *****************************************************************/
        jpegd_engine_sw_ac_huff_decode (p_bitbuffer, acTable, &s, &codeSize);

        /******************************************************************
         * update the intermediate buffer with codeSize
         *****************************************************************/
        jpegd_engine_sw_ac_bit_buffer_update (p_engine, codeSize);

        /******************************************************************
         * r : run which is the high 4 bits
         * s:  size which is the size of residue
         *****************************************************************/
        r = s >> 4;
        s &= 0xF;

        if (s != 0)
        {
            if (r != 0)
            {
                // the run
                if ( (i+=r) > 63)
                {
                    JPEG_DBG_LOW("Huffman error, i=%d, r=%d, s=%d", i, r, s);
                    return JPEGERR_EFAILED;
                } // end if ((i+r)>63)
            } //end if (r!=0)

            /**************************************************************
             * update the intermediate buffer with number of bits (s) WITHOUT
             * 1. decoding the residue
             * 2. huffman extend
             * 3. unzigzag + deQuant
             *************************************************************/
            jpegd_engine_sw_ac_bit_buffer_update (p_engine, s);
        } // end if(s!=0)
        else
        {
            // is EOB
            if (r == 15)
            {
                if ( (i+=r) >63)
                {
                    JPEG_DBG_LOW("Huffman error, i=%d, r=%d, s=%d", i, r, s);
                    return JPEGERR_EFAILED;
                } // end if ((i+r)>63)
            } // end if ( r == 15 )
            else
            {
                break;
            }
        }
    }

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_consume_one_block
---------------------------------------------------------------------------
DESCRIPTION     decode block of coefficients; this routine also de-zigzag
                the coefficients order
---------------------------------------------------------------------------
INPUT VALUE     dctable: dc huffman table
                actable: ac huffman table
                (* lastVale): pointer to last dc value, this value will
                              be changed if dc coefficients are
                              successfully decoded.
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_sw_consume_one_block(jpegd_engine_sw_t *p_engine,
                                      jpegd_derived_htable_t *dcTable,
                                      jpegd_derived_htable_t *acTable,
                                      int32_t * lastDcVal)
{
    /**********************************************************************
    * Local variable
    *********************************************************************/
    int32_t  i;
    int32_t  r, s;

    uint32_t categoryCodeSize; //symbolCodesize
    int32_t  codeSize;

    jpegd_derived_htable_entry_t tableEntry;
    int32_t length;
    int16_t bitMask;

    uint32_t bitsRead;
    uint32_t nextBits;

    jpegd_bitbuffer_t *p_bitbuffer = &(p_engine->bitbuffer);

    /**********************************************************************
     * get the dc coefficient.
     *********************************************************************/

    /**********************************************************************
     * m_jpegdBitBuffer.buffer is 32 bits buffer with left Alignment
     * it right shifts 24 bits to get the most left 8 bits
     * then lookup in the dcTable to find the catorgory and code length
     *********************************************************************/
    categoryCodeSize = (dcTable->nCategoryCodeSize[(p_bitbuffer->bit_accumulator>>24) & 0xFF]);

    /**********************************************************************
     * categoryCodeSize has 16 bits of catorgory and 16 bits of code length.
     * if it is greater than 0, the code length is less than or equal to 8 bits
     * otherwise, the code length is greater than 8 bits
     *********************************************************************/
    if (((int32_t)(categoryCodeSize)) > 0)
    {
        /******************************************************************
         * If the code length is less than or equal to 8 bits, reading out
         * the number of bits which is the lower 16 bits of categoryCodeSize
         *****************************************************************/
        nextBits = jpegd_engine_sw_get_padded_bits(p_engine, (int32_t)(categoryCodeSize & 0xFFFF));

        /******************************************************************
         * the category(residue size) is the most significant 16 bits
         * of categoryCodeSize
         ******************************************************************/
        s = (int32_t) (categoryCodeSize >> 16 );
    }
    else
    {
        /******************************************************************
         * If the code length is longer than 8 bits, the category is not in
         * the first nCategoryCodeSize table and a second search in the
         * symbol table is needed
         *****************************************************************/

        /******************************************************************
         * get the lower 16 bits, ang length is code length minus 8 bits
         *****************************************************************/
        length = categoryCodeSize & 0xFFFF;

        bitMask = (int16_t)((1 << length) - 1);

        /******************************************************************
         * code length is length plus 8
         * the category in the symbol table is equal to the inverse of
         * most significant 16 bits categoryCodeSize plus value of bits
         * that m_jpegdBitBuffer.buffer right shifts (24-length) bits
         *****************************************************************/
        tableEntry = dcTable->symbol[(uint16_t)(~( categoryCodeSize >> 16))
                    + ((p_bitbuffer->bit_accumulator >> (24-length)) & bitMask)];

        /******************************************************************
         * read out the number of bits (tableEntry.len ) in bit buffer
         *****************************************************************/
        nextBits = jpegd_engine_sw_get_padded_bits(p_engine, (int32_t)(tableEntry.len));

        /******************************************************************
         * read out catergory to s
        ******************************************************************/
        s = (int32_t)(tableEntry.cat);
    }

    if (s != 0)
    {
        /******************************************************************
         * if s is non-zero, read out the residue value, huff extend it
         * to get its actual value, and then compute the lastDcVal.
         *
         * We need lastDcVal even we are only consuming the bitstream.
         * The next block need the lastDcVal to compute the current
         * DC value.
         *****************************************************************/
        bitsRead = jpegd_engine_sw_get_padded_bits(p_engine, s);
        s        = jpegd_engine_sw_huff_extend(bitsRead, s);
    }

    *lastDcVal = (s += *lastDcVal);

    /**********************************************************************
    * Consume AC bitstream
    *********************************************************************/
    for (i = 1; i < 64; i++)
    {
        /******************************************************************
         * huffman decode the AC coefficients to s
         * and codeSize is codeword size
         *****************************************************************/
        jpegd_engine_sw_ac_huff_decode (p_bitbuffer, acTable, &s, &codeSize);

        /******************************************************************
         * update the intermediate buffer with codeSize
         *****************************************************************/
        jpegd_engine_sw_ac_bit_buffer_update (p_engine, codeSize);

        /******************************************************************
         * r : run which is the high 4 bits
         * s:  size which is the size of residue
         *****************************************************************/
        r = s >> 4;
        s &= 0xF;

        if (s != 0)
        {
            if (r != 0)
            {
                // the run
                if ( (i+=r) > 63)
                {
                    JPEG_DBG_LOW("Huffman error, i=%d, r=%d, s=%d", i, r, s);
                    return JPEGERR_EFAILED;
                } // end if ((i+r)>63)
            } //end if (r!=0)

            /**************************************************************
             * update the intermediate buffer with number of bits (s) WITHOUT
             * 1. decoding the residue
             * 2. huffman extend
             * 3. unzigzag + deQuant
             *************************************************************/
            jpegd_engine_sw_ac_bit_buffer_update (p_engine, s);
        } // end if(s!=0)
        else
        {
            // is EOB
            if (r == 15)
            {
                if ( (i+=r) >63)
                {
                    JPEG_DBG_LOW("Huffman error, i=%d, r=%d, s=%d", i, r, s);
                    return JPEGERR_EFAILED;
                } // end if ((i+r)>63)
            } // end if ( r == 15 )
            else
            {
                break;
            }
        }
    }

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_ac_code_residue_consumed
---------------------------------------------------------------------------
DESCRIPTION     get bits from bitstream to bit buffer if less than 16 bits
                left, and pad it if a marker is read;
---------------------------------------------------------------------------
INPUT  VALUE    numOfBits: number of bits already reading out
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_sw_ac_code_residue_consumed (jpegd_engine_sw_t *p_engine, int32_t numOfBits)
{
    /**********************************************************************
     * Local variables
     *********************************************************************/
    uint8_t byte1, byte2;

    /**********************************************************************
     * now check if remaining bits is less than half, if so, read two more
     * bytes from the input buffer and reload, and update bits left;
     *********************************************************************/
    if ( (p_engine->bitbuffer.bits_left -= numOfBits) <= 16)
    {
         // read two bytes from input buffer
        byte1 = jpegd_engine_sw_get_one_padded_byte(p_engine);
        byte2 = jpegd_engine_sw_get_one_padded_byte(p_engine);

        // put two read bytes into bit buffer
        p_engine->bitbuffer.bit_accumulator = (p_engine->bitbuffer.bit_accumulator |
                                  (((uint32_t) byte1) << (24 - p_engine->bitbuffer.bits_left)) |
                                  (((uint32_t) byte2) << (16 - p_engine->bitbuffer.bits_left)));

        // update bitsLeft.
        p_engine->bitbuffer.bits_left += JPEGD_BIT_BUFFER_LENGTH_HALF;
    }

    /**********************************************************************
     * end of AcCodeResidueConsumed, and return
     *********************************************************************/
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_ac_bit_buffer_update
---------------------------------------------------------------------------
DESCRIPTION     update the bit buffer first, then get bits from bitstream
                to bit buffer if less than 16 bits left
---------------------------------------------------------------------------
INPUT  VALUE    numOfBits: number of bits decoded
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_sw_ac_bit_buffer_update (jpegd_engine_sw_t *p_engine, int32_t numOfBits)
{
    /**********************************************************************
     * Local variables
     *********************************************************************/
    uint8_t byte1, byte2;

    /**********************************************************************
     * Always Left shift for left Alignment
     *********************************************************************/
    p_engine->bitbuffer.bit_accumulator <<= numOfBits;

    /**********************************************************************
     * now check if remaining bits is less than half, if so, read two more
     * bytes from the input buffer and reload, and update bits left;
     *********************************************************************/
    if ((p_engine->bitbuffer.bits_left -= numOfBits) <= 16)
    {
          // read two bytes from input buffer
        byte1 = jpegd_engine_sw_get_one_padded_byte(p_engine);
        byte2 = jpegd_engine_sw_get_one_padded_byte(p_engine);

        // put two read bytes into bit buffer.
        p_engine->bitbuffer.bit_accumulator = (p_engine->bitbuffer.bit_accumulator |
                                  (((uint32_t) byte1) << (24 - p_engine->bitbuffer.bits_left)) |
                                  (((uint32_t) byte2) << (16 - p_engine->bitbuffer.bits_left)));

        // update bitsLeft.
        p_engine->bitbuffer.bits_left += JPEGD_BIT_BUFFER_LENGTH_HALF;
    }

    /**********************************************************************
     * end of AcBitBufferUpdate, and return result
     *********************************************************************/
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_decode_one_mcu
---------------------------------------------------------------------------
DESCRIPTION     decode one mcu of samples;
---------------------------------------------------------------------------
INPUT VALUE     none
---------------------------------------------------------------------------
RETURN VALUE    none
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_sw_decode_one_mcu(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     * local variable
     *********************************************************************/
    uint32_t blockCount;
    uint8_t  compIdx;

    /**********************************************************************
     * decode one mcu need to hook up the correct huffman table and quant
     * table, and dehuff->dezigzag->dequant->idct
     * In this implementation, de-zigzag is absorbed in dehuff, dequant is
     * absorbed in idct
     *********************************************************************/
    for (blockCount = 0; blockCount < p_engine->nBlocksPerMCU; blockCount++)
    {
        // fist find the components id.
        compIdx = p_engine->MCUBlockCompList[blockCount];

        // dehuff man and de-zigzag one block
        if (JPEG_FAILED(jpegd_engine_sw_dehuff_one_block(p_engine,
            p_engine->pDerivedHuffTable[p_engine->dcHuffTableId[compIdx]],
            p_engine->pDerivedHuffTable[p_engine->acHuffTableId[compIdx]],
            p_engine->pDeQuantTable[p_engine->quantId[compIdx]],
            & (p_engine->lastDcVal[compIdx]),
            p_engine->pCoeffBlockBuf,
            p_engine->numOfValidCoeff)))
        {
            return JPEGERR_EFAILED;
        }

        // dequant and idct one block
        p_engine->idct_func(p_engine->pCoeffBlockBuf,
                            p_engine->pSampleMCUBuf[blockCount],
                            JPEGD_DCTSIZE * sizeof(jpegd_coeff_t));
    }
    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_consume_one_mcu
---------------------------------------------------------------------------
DESCRIPTION     consume one mcu of samples;
---------------------------------------------------------------------------
INPUT VALUE     none
---------------------------------------------------------------------------
RETURN VALUE    none
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_sw_consume_one_mcu(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     * local variable
     *********************************************************************/
    uint32_t blockCount;
    uint8_t  compIdx;

    /**********************************************************************
     * decode one mcu need to hook up the correct huffman table and quant
     * table, and dehuff->dezigzag->dequant->idct
     * In this implementation, de-zigzag is absorbed in dehuff, dequant is
     * absorbed in idct
     *********************************************************************/
    for (blockCount = 0; blockCount < p_engine->nBlocksPerMCU; blockCount++)
    {
        // fist find the components id.
        compIdx = p_engine->MCUBlockCompList[blockCount];

        // dehuff man and de-zigzag one block
        if (JPEG_FAILED(jpegd_engine_sw_consume_one_block(p_engine,
            p_engine->pDerivedHuffTable[p_engine->dcHuffTableId[compIdx]],
            p_engine->pDerivedHuffTable[p_engine->acHuffTableId[compIdx]],
            & (p_engine->lastDcVal[compIdx]))))
        {
            return JPEGERR_EFAILED;
        }
    }

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_skip_one_mcu
---------------------------------------------------------------------------
DESCRIPTION     skip one mcu, this is equavalent of write one MCU with all
                0 sample values.
---------------------------------------------------------------------------
INPUT VALUE     none
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_sw_skip_one_mcu(jpegd_engine_sw_t *p_engine)
{
    uint32_t blockCount;

    // reset the sample block in mcu to zero.
    for (blockCount = 0; blockCount < p_engine->nBlocksPerMCU; blockCount++)
    {
        STD_MEMSET(p_engine->pSampleMCUBuf[blockCount], 0,
                   JPEGD_DCTSIZE2 * sizeof (jpegd_sample_t));
    }
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_output_gray_mcu
---------------------------------------------------------------------------
DESCRIPTION     write one mcu of Gray samples to mcc line buffer
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_sw_output_gray_mcu(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     *              LOCAL VARIABLES
     *********************************************************************/
    int32_t i, j;

    const int32_t DCTSIZE = JPEGD_DCTSIZE >> p_engine->nResizeFactor;

    // input pointers
    jpegd_sample_t * inputY1Ptr;

    // output pointers
    jpegd_pixel_t *  outputY1Ptr;

    /******************************************************************
    *  Y
    *****************************************************************/
    inputY1Ptr    = p_engine->pSampleMCUBuf[0];
    outputY1Ptr   = p_engine->pMCULineBufPtrY;

    for (i = 0; i < DCTSIZE; i++)
    {
        for (j = 0; j < DCTSIZE; j++)
        {
            *outputY1Ptr++   = (jpegd_pixel_t) *inputY1Ptr++;
        }

        inputY1Ptr += (JPEGD_DCTSIZE - DCTSIZE);

        outputY1Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
    }
    p_engine->pMCULineBufPtrY    += DCTSIZE;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_output_h1v1_mcu
---------------------------------------------------------------------------
DESCRIPTION     write one mcu of H1V1 samples to mcu line buffer
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_sw_output_h1v1_mcu(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     *              LOCAL VARIABLES
     *********************************************************************/
    int32_t i, j;

    const int32_t DCTSIZE = JPEGD_DCTSIZE >> p_engine->nResizeFactor;

    // input pointers
    jpegd_sample_t * inputY1Ptr;
    jpegd_sample_t * inputCbPtr;
    jpegd_sample_t * inputCrPtr;

    // output pointers
    jpegd_pixel_t *  outputY1Ptr;
    jpegd_pixel_t *  outputCbCrPtr;

    /******************************************************************
    *  Y
    * ----
    *  Cb
    * ----
    *  Cr
    *****************************************************************/
    inputY1Ptr    = p_engine->pSampleMCUBuf[0];
    inputCbPtr    = p_engine->pSampleMCUBuf[1];
    inputCrPtr    = p_engine->pSampleMCUBuf[2];
    outputY1Ptr   = p_engine->pMCULineBufPtrY;
    outputCbCrPtr = p_engine->pMCULineBufPtrCbCr;

    for (i = 0; i < DCTSIZE; i++)
    {
        for (j = 0; j < DCTSIZE; j++)
        {
            *outputY1Ptr++   = (jpegd_pixel_t) *inputY1Ptr++;
            *outputCbCrPtr++ = (jpegd_pixel_t) *inputCrPtr++;
            *outputCbCrPtr++ = (jpegd_pixel_t) *inputCbPtr++;
        }

        inputY1Ptr += (JPEGD_DCTSIZE - DCTSIZE);
        inputCbPtr += (JPEGD_DCTSIZE - DCTSIZE);
        inputCrPtr += (JPEGD_DCTSIZE - DCTSIZE);

        outputY1Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
        outputCbCrPtr += (p_engine->nOutputRowIncrCbCr - DCTSIZE*2);
    }
    p_engine->pMCULineBufPtrY    += DCTSIZE;
    p_engine->pMCULineBufPtrCbCr += (DCTSIZE*2);
}


/**************************************************************************
FUNCTION        jpegd_engine_sw_output_h1v2_mcu
---------------------------------------------------------------------------
DESCRIPTION     write one mcu of H1V2 samples to mcu line buffer
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_sw_output_h1v2_mcu(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     *              LOCAL VARIABLES
     *********************************************************************/
    int32_t i, j;

    const int32_t DCTSIZE = JPEGD_DCTSIZE >> p_engine->nResizeFactor;

    // input pointers
    jpegd_sample_t * inputY1Ptr;
    jpegd_sample_t * inputY2Ptr;
    jpegd_sample_t * inputCbPtr;
    jpegd_sample_t * inputCrPtr;

    // output pointers
    jpegd_pixel_t *  outputY1Ptr;
    jpegd_pixel_t *  outputY2Ptr;
    jpegd_pixel_t *  outputCbCrPtr;

    /******************************************************************
    *  Y1
    * ----
    *  Y2
    * ----
    *  Cb
    * ----
    *  Cr
    *****************************************************************/
    inputY1Ptr    = p_engine->pSampleMCUBuf[0];
    inputY2Ptr    = p_engine->pSampleMCUBuf[1];
    inputCbPtr    = p_engine->pSampleMCUBuf[2];
    inputCrPtr    = p_engine->pSampleMCUBuf[3];
    outputY1Ptr   = p_engine->pMCULineBufPtrY;
    outputY2Ptr   = p_engine->pMCULineBufPtrY+DCTSIZE*p_engine->nOutputRowIncrY;
    outputCbCrPtr = p_engine->pMCULineBufPtrCbCr;

    for (i=0; i<DCTSIZE; i++)
    {
        for (j=0; j<DCTSIZE; j++)
        {
            *outputY1Ptr++   = (jpegd_pixel_t) *inputY1Ptr++;
            *outputY2Ptr++   = (jpegd_pixel_t) *inputY2Ptr++;
            *outputCbCrPtr++ = (jpegd_pixel_t) *inputCrPtr++;
            *outputCbCrPtr++ = (jpegd_pixel_t) *inputCbPtr++;
        }

        inputY1Ptr += (JPEGD_DCTSIZE - DCTSIZE);
        inputY2Ptr += (JPEGD_DCTSIZE - DCTSIZE);
        inputCbPtr += (JPEGD_DCTSIZE - DCTSIZE);
        inputCrPtr += (JPEGD_DCTSIZE - DCTSIZE);

        outputY1Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
        outputY2Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
        outputCbCrPtr += (p_engine->nOutputRowIncrCbCr - DCTSIZE*2);
    }
    p_engine->pMCULineBufPtrY    += (DCTSIZE*1);
    p_engine->pMCULineBufPtrCbCr += (DCTSIZE*2);
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_output_h2v1_mcu
---------------------------------------------------------------------------
DESCRIPTION     write one mcu of H2V1 samples to mcu line buffer
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_sw_output_h2v1_mcu(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     *              LOCAL VARIABLES
     *********************************************************************/
    int32_t i, j;

    const int32_t DCTSIZE = JPEGD_DCTSIZE >> p_engine->nResizeFactor;

    // input pointers
    jpegd_sample_t * inputY1Ptr;
    jpegd_sample_t * inputY2Ptr;
    jpegd_sample_t * inputCbPtr;
    jpegd_sample_t * inputCrPtr;

    // output pointers
    jpegd_pixel_t *  outputY1Ptr;
    jpegd_pixel_t *  outputY2Ptr;
    jpegd_pixel_t *  outputCbCrPtr;

    /******************************************************************
    *  Y1 | Y2
    * ---------
    *  Cb
    * ----
    *  Cr
    *****************************************************************/
    inputY1Ptr    = p_engine->pSampleMCUBuf[0];
    inputY2Ptr    = p_engine->pSampleMCUBuf[1];
    inputCbPtr    = p_engine->pSampleMCUBuf[2];
    inputCrPtr    = p_engine->pSampleMCUBuf[3];
    outputY1Ptr   = p_engine->pMCULineBufPtrY;
    outputY2Ptr   = p_engine->pMCULineBufPtrY + DCTSIZE;
    outputCbCrPtr = p_engine->pMCULineBufPtrCbCr;

    for (i = 0; i < DCTSIZE; i++)
    {
        for (j = 0; j < DCTSIZE; j++)
        {
            *outputY1Ptr++   = (jpegd_pixel_t) *inputY1Ptr++;
            *outputY2Ptr++   = (jpegd_pixel_t) *inputY2Ptr++;
            *outputCbCrPtr++ = (jpegd_pixel_t) *inputCrPtr++;
            *outputCbCrPtr++ = (jpegd_pixel_t) *inputCbPtr++;
        }

        inputY1Ptr += (JPEGD_DCTSIZE - DCTSIZE);
        inputY2Ptr += (JPEGD_DCTSIZE - DCTSIZE);
        inputCbPtr += (JPEGD_DCTSIZE - DCTSIZE);
        inputCrPtr += (JPEGD_DCTSIZE - DCTSIZE);

        outputY1Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
        outputY2Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
        outputCbCrPtr += (p_engine->nOutputRowIncrCbCr - DCTSIZE*2);
    }
    p_engine->pMCULineBufPtrY    += (DCTSIZE*2);
    p_engine->pMCULineBufPtrCbCr += (DCTSIZE*2);
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_output_h2v2_mcu
---------------------------------------------------------------------------
DESCRIPTION     write one mcu of H2V2 samples to muc line buffer
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_sw_output_h2v2_mcu(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     *              LOCAL VARIABLES
     *********************************************************************/
    int32_t i, j;

    const int32_t DCTSIZE = JPEGD_DCTSIZE >> p_engine->nResizeFactor;

    // input pointers
    jpegd_sample_t * inputY1Ptr;
    jpegd_sample_t * inputY2Ptr;
    jpegd_sample_t * inputY3Ptr;
    jpegd_sample_t * inputY4Ptr;
    jpegd_sample_t * inputCbPtr;
    jpegd_sample_t * inputCrPtr;

    // output pointers
    jpegd_pixel_t *  outputY1Ptr;
    jpegd_pixel_t *  outputY2Ptr;
    jpegd_pixel_t *  outputY3Ptr;
    jpegd_pixel_t *  outputY4Ptr;
    jpegd_pixel_t *  outputCbCrPtr;

    /******************************************************************
    *  Y1 | Y2
    * -------
    *  Y3 | Y4
    * -----
    *  Cb
    * ----
    *  Cr
    *****************************************************************/
    inputY1Ptr    = p_engine->pSampleMCUBuf[0];
    inputY2Ptr    = p_engine->pSampleMCUBuf[1];
    inputY3Ptr    = p_engine->pSampleMCUBuf[2];
    inputY4Ptr    = p_engine->pSampleMCUBuf[3];
    inputCbPtr    = p_engine->pSampleMCUBuf[4];
    inputCrPtr    = p_engine->pSampleMCUBuf[5];
    outputY1Ptr   = p_engine->pMCULineBufPtrY;
    outputY2Ptr   = p_engine->pMCULineBufPtrY + DCTSIZE;
    outputY3Ptr   = p_engine->pMCULineBufPtrY + DCTSIZE * p_engine->nOutputRowIncrY;
    outputY4Ptr   = p_engine->pMCULineBufPtrY + DCTSIZE * p_engine->nOutputRowIncrY
        + DCTSIZE;
    outputCbCrPtr = p_engine->pMCULineBufPtrCbCr;

    // once the pointer is all set, copy is easy, in this case
    // output is crcb interleaved.
    for (i=0; i<DCTSIZE; i++)
    {
        for (j=0; j<DCTSIZE; j++)
        {
            *outputY1Ptr++   = (jpegd_pixel_t) *inputY1Ptr++;
            *outputY2Ptr++   = (jpegd_pixel_t) *inputY2Ptr++;
            *outputY3Ptr++   = (jpegd_pixel_t) *inputY3Ptr++;
            *outputY4Ptr++   = (jpegd_pixel_t) *inputY4Ptr++;
            *outputCbCrPtr++ = (jpegd_pixel_t) *inputCrPtr++;
            *outputCbCrPtr++ = (jpegd_pixel_t) *inputCbPtr++;
        }
        // point to next line.
        inputY1Ptr += (JPEGD_DCTSIZE - DCTSIZE);
        inputY2Ptr += (JPEGD_DCTSIZE - DCTSIZE);
        inputY3Ptr += (JPEGD_DCTSIZE - DCTSIZE);
        inputY4Ptr += (JPEGD_DCTSIZE - DCTSIZE);
        inputCbPtr += (JPEGD_DCTSIZE - DCTSIZE);
        inputCrPtr += (JPEGD_DCTSIZE - DCTSIZE);

        outputY1Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
        outputY2Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
        outputY3Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
        outputY4Ptr   += (p_engine->nOutputRowIncrY - DCTSIZE);
        outputCbCrPtr += (p_engine->nOutputRowIncrCbCr - (DCTSIZE*2));
    }

    // after we are done, reset the buffer pointer.
    p_engine->pMCULineBufPtrY    += (DCTSIZE*2);
    p_engine->pMCULineBufPtrCbCr += (DCTSIZE*2);
}


/**************************************************************************
FUNCTION        jpegd_engine_sw_get_restart_marker
---------------------------------------------------------------------------
DESCRIPTION     get restart marker
---------------------------------------------------------------------------
INPUT  VALUE    (* marker): pointer to marker found
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_sw_get_restart_marker(jpegd_engine_sw_t *p_engine,
                                       uint8_t * marker)
{
    /**********************************************************************
     * local variable
     *********************************************************************/
    for (;;)
    {
        // first get the marker
        if (JPEG_FAILED(jpegd_engine_sw_get_next_marker(p_engine, marker)))
        {
            return JPEGERR_EFAILED;
        }

        /******************************************************************
         * markers need to be filtered throu, in bitstream, should only be
         * STR markers, and they have no parameters.
         *****************************************************************/
        switch (*marker)
        {
            case M_RST0:    /* no parameters */
            case M_RST1:
            case M_RST2:
            case M_RST3:
            case M_RST4:
            case M_RST5:
            case M_RST6:
            case M_RST7:
            case M_TEM:
            {
                // found!
                return JPEGERR_SUCCESS;
            }
            case M_EOI:
            {
                JPEG_DBG_HIGH("jpegd_engine_sw_get_restart_marker: Restart Marker not found before EOI\n");
                return JPEGERR_EFAILED;
            }

            default:
            {
                continue;
            } // default
        } // end of switch
    } // end for (;;)
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_process_restart_marker
---------------------------------------------------------------------------
DESCRIPTION     process restart marker and return number of mcus to skip
---------------------------------------------------------------------------
INPUT  VALUE    (* skipCount): pointer to number of mcus to skip
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_sw_process_restart_marker(jpegd_engine_sw_t *p_engine,
                                           int32_t * skipCount)
{
    /**********************************************************************
     * local variables
     *********************************************************************/
    uint8_t  markerRead;

    // first reset buffer to make it byte alianed
    jpegd_engine_sw_reset_bit_buffer(p_engine);

    // then get the rst marker
    if (JPEG_FAILED(jpegd_engine_sw_get_restart_marker(p_engine,
                                                       &markerRead)))
    {
        return JPEGERR_EFAILED;
    }
    /**********************************************************************
     * check if this is the restart marker we want
     * if it is, return 0
     * else, return the number of mcus need to be skipped
     *********************************************************************/
    if (markerRead == p_engine->nNextRestartNum)
    {
        *skipCount = 0;
    }
    else
    {
        *skipCount = ( (int8_t) markerRead - (int8_t) p_engine->nNextRestartNum) > 0 ?
                     (markerRead - p_engine->nNextRestartNum) :
                     8 - (p_engine->nNextRestartNum - markerRead);
        *skipCount = (*skipCount) * p_engine->nRestartInterval;
    }

    // now reset the restart left and next restart marker
    p_engine->nRestartLeft    = p_engine->nRestartInterval;
    p_engine->nNextRestartNum = (p_engine->nNextRestartNum + 1) & 0xD7;

    /**********************************************************************
     * Reload the bif buffer to get ready for next mcu decoding
     *********************************************************************/
    jpegd_engine_sw_reset_bit_buffer(p_engine);

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_get_eoi
---------------------------------------------------------------------------
DESCRIPTION     get end of image marker
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           some jpeg files may not have EOI
**************************************************************************/
int jpegd_engine_sw_get_eoi(jpegd_engine_sw_t *p_engine)
{
    /**********************************************************************
     * Local variable
     *********************************************************************/
    uint8_t  markerRead;
    uint32_t bitsRead;

    p_engine->bitbuffer.bits_left = 32;

    bitsRead = jpegd_engine_sw_get_bits(p_engine, 16);
    bitsRead = jpegd_engine_sw_get_bits(p_engine, 16);

    /**********************************************************************
     * search for marker first, if it turns out to be EOI, perfect,
     * if not, bail out (so close to the end!)
     *********************************************************************/
    if (JPEG_FAILED(jpegd_engine_sw_get_next_marker(p_engine, &markerRead)))
    {
        return JPEGERR_EFAILED;
    }

    if (markerRead != M_EOI)
    {
        JPEG_DBG_LOW("jpegd_engine_sw_get_eoi: Marker EOI not found.\n");
        return JPEGERR_EFAILED;
    }

    return JPEGERR_SUCCESS;
}
