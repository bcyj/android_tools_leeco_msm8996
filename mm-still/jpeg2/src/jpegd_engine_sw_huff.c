/*========================================================================


*//** @file jpegd_engine_sw_huff.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008 Qualcomm Technologies, Inc.
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
02/22/10   leim    Remove LongCodeCount to save stack. 
07/20/09   zhiminl Removed unused p_engine from jpegd_engine_sw_derive_huff().
07/07/09   zhiminl Decoupled bit buffer from jpegd_engine_input_fetcher_t.
05/05/09   mingy   Replaced the huffman functions with the 2-step approach.
                   Branch out huffExtend function.
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_buffer_private.h"
#include "jpegd_engine_sw.h"
#include "jpegd.h"
#include "jpegerr.h"
#include "jpeglog.h"
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

/* =======================================================================
**                       Macro/Constant Definitions
** ======================================================================= */
/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/**************************************************************************
FUNCTION        jpegd_engine_sw_derive_huff
---------------------------------------------------------------------------
DESCRIPTION     Make derived huffman table from huff bits and val
---------------------------------------------------------------------------
INPUT  VALUE    bits: huff bits
                val : huff bal
                derivedHuffTable: point to generated derived huff table
                scratch: scratch buffer for intermediate use
---------------------------------------------------------------------------
RETURN VALUE    None
---------------------------------------------------------------------------
NOTES           None
**************************************************************************/
void jpegd_engine_sw_derive_huff(
    uint8_t * bits, 
    uint8_t * val,
    jpegd_derived_htable_t * derivedTable,
    int32_t  *scratch)
{
    /**********************************************************************
     * Local variable
     *********************************************************************/
    int32_t   i, j, k, lastK, si;
    int32_t  *huffSize;
    int32_t  *huffCode;
    uint32_t  code;
    uint32_t  subCode;
    int32_t   codeSize;

    int32_t   resLen;
    int32_t  *residualLength;

    int32_t   symbolTableOffset;

    int32_t   resCode;
    int32_t   resLength;

    uint16_t *category;

    // clear the local buffers
    (void) STD_MEMSET (scratch, 0, 900 * sizeof(int32_t));
    huffSize = scratch;
    huffCode = scratch + 257;
    residualLength = scratch + 514;
    category = (uint16_t *)(scratch + 771);

    /**********************************************************************
     * first make the size table
     *********************************************************************/
    k = 0;
    for (i = 1; i < 17; i++)
    {
        for (j = 1; j <= bits[i]; j++)
        {
            huffSize[k++] = i;
        }
    }
    huffSize[k] = 0;
    lastK       = k;

    /**********************************************************************
     * then generate code table
     *********************************************************************/
    code = 0;
    si   = huffSize[0];
    k    = 0;
    while (huffSize[k] != 0)
    {
        while (huffSize[k] == si)
        {
            huffCode[k++] = code;
            code++;
        }
        code <<= 1;
        si++;
    }

    /**********************************************************************
     * clear up the derived huffman table that is to be made, as the
     * following routine depend on the fact that untouched entry of these
     * tables are zero
     *********************************************************************/
    STD_MEMSET(derivedTable->nCategoryCodeSize,  0, sizeof (derivedTable->nCategoryCodeSize));
    STD_MEMSET(derivedTable->symbol,    0, sizeof (derivedTable->symbol));

    /**********************************************************************
     * now make look-ahead table (LAH), and symbol (S) table.
     * the look ahead table will be used for codewords shorter or equal to
     * 8 bits; symbol table will be used for codewords longer than 8.
     *********************************************************************/
    k = 0;
    while (k < lastK)
    {
        i        = val[k];
        code     = huffCode[k];
        codeSize = huffSize[k];

        // prcess codewords by length
        if (codeSize <= 8)
        {
            /**************************************************************
             * for code shorter or equals to 8, put them in LAH.
             * todo : add more comments as of how LAH table is built
             *************************************************************/
            // left allign it on 8 bits boundary
            code <<= (8 - codeSize);
            for (j = 1 << (8 - codeSize); j>0; j--)
            {
                // fill all entrys starting with that code the same symbol
                derivedTable->nCategoryCodeSize[code] = ((uint32_t)(i) << 16 )|((uint16_t)(codeSize));
                code++;
            }
        }
        else
        {
            /**************************************************************
             * for code longer than 8, build the symbol table.
             * todo : add more comments as of how S table is built
             *************************************************************/
            // first get the part of the code that is longer than 8
            subCode = (code >> (codeSize - 8)) & 0xFF;

            // if code size is over 8, first construct the result length
            // table, i.e. what is the maximum residual length (part that
            // longer than 8) with the same top 8 bits.
            resLen = codeSize - 8;
            residualLength[subCode] = residualLength[subCode] < resLen ? resLen : residualLength[subCode];
        } // end else {}
        k++;
    } // end while(k<lastK)

    // step 2, initialze symbol table, if the residualLength is none zero,
    // create it correspondent symbol table entry, and set the offset in the
    // lookAhead table.
    symbolTableOffset =0;

    for ( i = 0; i < 256; i++)
    {
        if ( residualLength[i] != 0 )
        {
            category[i]= ~((uint16_t)(symbolTableOffset));
            derivedTable->nCategoryCodeSize[i] = ( ~( (uint32_t) (symbolTableOffset) ) << 16 )|((uint16_t)(residualLength[i]));
            symbolTableOffset += ( 1<< residualLength[i] );
        }
    }

    // step 3, populate the symbol table, set offset in lookAhead table
    k=0;

    while (k < lastK)
    {
        i = val[k];
        code = huffCode[k];
        codeSize = huffSize[k];

        if ( codeSize>8 )
        {
            // get the first 8 bits
            subCode = ( code >> ( codeSize - 8 ) ) & 0xFF;
            // get the reset of bits
            resCode = ( code & ( (1<<(codeSize-8)) - 1 ));
            //
            resLength = 8 + residualLength[subCode] - codeSize;

            for (j=0;j<(1<<resLength);j++)
            {
                derivedTable->symbol[(uint16_t)(~(category[subCode]))+(uint16_t)(resCode<<resLength)+j].len = (uint16_t)(codeSize);
                derivedTable->symbol[(uint16_t)(~(category[subCode]))+(uint16_t)(resCode<<resLength)+j].cat = (uint16_t)(i);
            }
        }
        k++;
    }
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_ac_huff_decode
---------------------------------------------------------------------------
DESCRIPTION     decode one huffman symbol.
---------------------------------------------------------------------------
INPUT  VALUE    hTable: huffman table used to decode
                (* symbol): pointer to symbol decoded.
                (* codeSize): is codeword size
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
void jpegd_engine_sw_ac_huff_decode(jpegd_bitbuffer_t *p_bitbuffer,
                                    jpegd_derived_htable_t * hTable,
                                    int32_t * symbol,
                                    int32_t * codeSize)
{
    /**********************************************************************
     * Local variable
     *********************************************************************/
    uint32_t categoryCodeSize;
    jpegd_derived_htable_entry_t tableEntry;
    int32_t length;
    int16_t bitMask;

    /**********************************************************************
     * m_jpegdBitBuffer.buffer is 32 bits buffer with left Alignment
     * it right shifts 24 bits to get the most left 8 bits
     * then lookup in the dcTable to find the catorgory and code length
     *********************************************************************/
    categoryCodeSize = (hTable->nCategoryCodeSize[(p_bitbuffer->bit_accumulator>>24) & 0xFF]);

    /**********************************************************************
     * categoryCodeSize has 16 bits of catorgory and 16 bits of code length
     * if it is greater than 0, the code length is less than or equal to 8 bits
     * otherwise, the code length is greater than 8 bits
     *********************************************************************/
    if ( ( (int32_t)(categoryCodeSize) ) > 0 )
    {
        /******************************************************************
         * If the code length is less than or equal to 8 bits, code length
         * is the lower 16 bits of categoryCodeSize
         *****************************************************************/
        *codeSize= (int32_t) (categoryCodeSize  & 0xFFFF);

        /******************************************************************
         * the category(residue size) is the most significant 16 bits
         * of categoryCodeSize
         ******************************************************************/
        *symbol = (int32_t) (categoryCodeSize >> 16);
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
        length =  categoryCodeSize  & 0xFFFF;

        bitMask = (int16_t)((1 << length )-1);

        /******************************************************************
         * code length is length plus 8
         * the category in the symbol table is equal to the inverse of
         * most significant 16 bits categoryCodeSize plus value of bits
         * that m_jpegdBitBuffer.buffer right shifts (24-length) bits
         *****************************************************************/
        tableEntry = hTable->symbol[(uint16_t)(~( categoryCodeSize >> 16))
                     + (( p_bitbuffer->bit_accumulator >> (24-length)) & bitMask)];

        /******************************************************************
         * the code size is the number of bits (tableEntry.len )
         *****************************************************************/
        *codeSize= (int32_t)(tableEntry.len );

        /******************************************************************
         * get the catergory to symbol
         *****************************************************************/
        *symbol = (int32_t)(tableEntry.cat);
    }
}



