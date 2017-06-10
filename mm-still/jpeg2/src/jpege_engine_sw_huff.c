/*========================================================================

*//** @file jpege_engien_sw_huff.c

@par EXTERNALIZED FUNCTIONS
  jpege_engine_sw_huff_init_tables
  jpege_engine_sw_huff_encode

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008, 2009, 2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
04/10/12  csriniva Fixed Static Analysis Warnings on WP
05/07/09   zhiminl Split jpege_engine_sw_huff_encode_ac() into
                   jpege_engine_sw_huff_bs.c.
05/04/09   zhiminl Split jpege_engine_sw_pack_bs() into jpege_engine_sw_huff_bs.c.
04/14/09   zhiminl Added jpege_engine_sw_huff_init_tables() and
                   internal bitstream pack buffer.
04/06/09   zhiminl Split dc and runlength encode into jpege_engine_sw_huff_bs.c.
04/02/09   zhiminl Optimizations ported.
10/14/08   vma     Optimizations.
07/05/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpege_engine_sw.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

extern void jpege_engine_sw_huff_encode_dc
(
    int32_t              dcDiff,
    huff_lookup_table_t *p_dcHuffLookupTable,
    bitstream_t         *p_bitstream
);
extern void jpege_engine_sw_huff_encode_ac
(
    register int16_t             *zigzagOutput,
    register huff_lookup_table_t *p_acHuffLookupTable,
    bitstream_t                  *p_bitstream
);

/* -----------------------------------------------------------------------
** Local Constants
** ----------------------------------------------------------------------- */

#define NUM_HUFFBITS          16
#define NUM_DC_HUFFVALS       12
#define NUM_AC_HUFFVALS       162

#define DC_LOOKUP_TABLE_SIZE  12
/* Although # of AC Run/Size is 162, the value range is from 0 to 255 */
#define AC_LOOKUP_TABLE_SIZE  256

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

static void jpege_engine_sw_huff_init_lookup_table
(
    const uint8_t       *huffBits,
    const uint8_t       *huffVal,
    huff_lookup_table_t *huffLookUpTable,
    uint16_t             component
)
{
    huff_lookup_table_t tempHuffLookUpTable[162];
    uint32_t            maxNumValues, lookUpTableSize;
    uint16_t            codeLength, codeWord;
    uint32_t            i, k;

    STD_MEMSET(tempHuffLookUpTable, 0, 162 * sizeof(huff_lookup_table_t));

    /* Set the maximum number of values for this component                 */
    if (component == DC)
    {
        maxNumValues    = NUM_DC_HUFFVALS;
        lookUpTableSize = DC_LOOKUP_TABLE_SIZE;
    }
    else // component == AC
    {
        maxNumValues    = NUM_AC_HUFFVALS;
        lookUpTableSize = AC_LOOKUP_TABLE_SIZE;
    }

    /* Next populate the code lengths and code words in the local table    */
    codeLength = 1;
    codeWord = 0;
    k = 0;
    for (i = 0; i < NUM_HUFFBITS; i++)
    {
        uint8_t bits = huffBits[i];
        while (bits--)
        {
            tempHuffLookUpTable[k].codeLength = codeLength;
            tempHuffLookUpTable[k].codeWord = codeWord;

            codeWord = codeWord + 1;
            k = k + 1;
        }

        codeLength = codeLength + 1;
        codeWord = codeWord << 1;
    }

    /* Clear the entries in the given look-up table                        */
    for (i = 0; i < lookUpTableSize; i++)
    {
        (huffLookUpTable+i)->codeLength = 0;
        (huffLookUpTable+i)->codeWord = 0;
    }

    /*-----------------------------------------------------------------------
      Now copy the entries from the local look-up table to the given look-up
      table using symbol values as index
    -----------------------------------------------------------------------*/
    for (i = 0; i < maxNumValues; i++)
    {
        uint8_t symbolValue = huffVal[i];
        (huffLookUpTable+symbolValue)->codeLength = tempHuffLookUpTable[i].codeLength;
        (huffLookUpTable+symbolValue)->codeWord = tempHuffLookUpTable[i].codeWord;
    }
} /* jpege_engine_sw_huff_init_lookup_tables() */

void jpege_engine_sw_huff_init_tables(jpege_engine_sw_t *p_engine)
{
    huff_tables_t *p_huffLookUpTables = &(p_engine->jpegeHuffTables);
    uint8_t       *p_huffbits = NULL;
    uint8_t       *p_huffval  = NULL;

    /*------------------------------------------------------------
     Generate Luma AC/DC and Chroma AC/DC Huffman Lookup Tables
    ------------------------------------------------------------*/

    /* Luma DC lookup table */
    p_huffbits =  p_engine->jpegeLumaBitsValTables.dcBits;
    p_huffval  =  p_engine->jpegeLumaBitsValTables.dcHuffVal;
    jpege_engine_sw_huff_init_lookup_table (
        p_huffbits, p_huffval, p_huffLookUpTables->lumaDCHuffLookupTable, DC);

    /* Chroma DC lookup table */
    p_huffbits =  p_engine->jpegeChromaBitsValTables.dcBits;
    p_huffval  =  p_engine->jpegeChromaBitsValTables.dcHuffVal;
    jpege_engine_sw_huff_init_lookup_table (
        p_huffbits, p_huffval, p_huffLookUpTables->chromaDCHuffLookupTable, DC);

    /* Luma AC lookup table */
    p_huffbits =  p_engine->jpegeLumaBitsValTables.acBits;
    p_huffval  =  p_engine->jpegeLumaBitsValTables.acHuffVal;
    jpege_engine_sw_huff_init_lookup_table (
        p_huffbits, p_huffval, p_huffLookUpTables->lumaACHuffLookupTable, AC);

    /* Chroma AC lookup table */
    p_huffbits =  p_engine->jpegeChromaBitsValTables.acBits;
    p_huffval  =  p_engine->jpegeChromaBitsValTables.acHuffVal;
    jpege_engine_sw_huff_init_lookup_table (
        p_huffbits, p_huffval, p_huffLookUpTables->chromaACHuffLookupTable, AC);
} /* jpege_engine_sw_huff_init_tables() */

void jpege_engine_sw_huff_encode
(
    jpege_engine_sw_t *p_engine,
    int16_t           *zigzagOutput,
    yCbCrType_t        yORcbcr
)
{
    huff_lookup_table_t *dcHuffLookupTablePtr, *acHuffLookupTablePtr;
    int32_t dcDiff;

    /*-----------------------------------------------------------------------
     Initialize DC/AC Huff lookup table pointers and previous DC pointer based
     on input component type.
    -----------------------------------------------------------------------*/
    if (yORcbcr == LUMA)   //Luma
    {
        /*-----------------------------------------------------------------------
         Calculate DC difference between current DC and prevDC
        -----------------------------------------------------------------------*/
        // DIFF = ZZ(0) - PRED
        dcDiff = zigzagOutput[0] - p_engine->prevLumaDC;
        // Update current DC
        p_engine->prevLumaDC = zigzagOutput[0];

        dcHuffLookupTablePtr = p_engine->jpegeHuffTables.lumaDCHuffLookupTable;
        acHuffLookupTablePtr = p_engine->jpegeHuffTables.lumaACHuffLookupTable;
    }
    else if (yORcbcr == CB)   //Cb
    {
        /*-----------------------------------------------------------------------
         Calculate DC difference between current DC and prevDC
        -----------------------------------------------------------------------*/
        // DIFF = ZZ(0) - PRED
        dcDiff = zigzagOutput[0] - p_engine->prevCbDC;
        // Update current DC
        p_engine->prevCbDC = zigzagOutput[0];

        dcHuffLookupTablePtr = p_engine->jpegeHuffTables.chromaDCHuffLookupTable;
        acHuffLookupTablePtr = p_engine->jpegeHuffTables.chromaACHuffLookupTable;
    }
    else   //Cr
    {
        /*-----------------------------------------------------------------------
         Calculate DC difference between current DC and prevDC
        -----------------------------------------------------------------------*/
        // DIFF = ZZ(0) - PRED
        dcDiff = zigzagOutput[0] - p_engine->prevCrDC;
        // Update current DC
        p_engine->prevCrDC = zigzagOutput[0];

        dcHuffLookupTablePtr = p_engine->jpegeHuffTables.chromaDCHuffLookupTable;
        acHuffLookupTablePtr = p_engine->jpegeHuffTables.chromaACHuffLookupTable;
    }

    jpege_engine_sw_huff_encode_dc(dcDiff, dcHuffLookupTablePtr,
                                   &(p_engine->jpegeBitStreamState));
    jpege_engine_sw_huff_encode_ac(zigzagOutput, acHuffLookupTablePtr,
                                   &(p_engine->jpegeBitStreamState));

} /* jpege_engine_sw_huff_encode() */

