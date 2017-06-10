/*========================================================================


*//** @file jpegd_engine_sw_huff_extend.c

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
07/07/09   zhiminl Decoupled bit buffer from jpegd_engine_input_fetcher_t.
05/06/09   mingy   Branch out jpegd_engine_sw_huff_extend from
                   jpegd_engine_sw_huff.c
========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpegd_engine_sw.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
// const array for huffman residual sign test
static const int32_t extendTest[16] = {
    0,      0x0001, 0x0002, 0x0004,
    0x0008, 0x0010, 0x0020, 0x0040,
    0x0080, 0x0100, 0x0200, 0x0400,
    0x0800, 0x1000, 0x2000, 0x4000 };

// const array for huffman residual sign extention
static const int32_t extendOffset[16] = {
    0, ((-1)<<1)+1, ((-1)<<2)+1, ((-1)<<3)+1,
    ((-1)<<4)+1,  ((-1)<<5)+1,  ((-1)<<6)+1,
    ((-1)<<7)+1,  ((-1)<<8)+1,  ((-1)<<9)+1,
    ((-1)<<10)+1, ((-1)<<11)+1, ((-1)<<12)+1,
    ((-1)<<13)+1, ((-1)<<14)+1, ((-1)<<15)+1 };

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
FUNCTION        jpegd_engine_sw_huff_extend
---------------------------------------------------------------------------
DESCRIPTION     extend run and size to get the decimal value
---------------------------------------------------------------------------
INPUT  VALUE    nResidue: run
                nResidueSize: size
---------------------------------------------------------------------------
RETURN VALUE    decimal value
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int32_t jpegd_engine_sw_huff_extend(int32_t nResidue, int32_t nResidueSize)
{
    /**********************************************************************
     * Making Sign Extension based on value of nResidue
     * if nResidue < extendTest[nResidueSize], nResidue is negative
     * otherwise,  nResidue is non-negative
     *
     * if nResidue is negative, return two's complement of nResidue + 1
     *                   which is to plus extendOffset[nResidueSize]
     * otherwise, return the value of nResidue
     *********************************************************************/

    return ( nResidue < extendTest[nResidueSize] ) ?
           ( nResidue + extendOffset[nResidueSize] ) : nResidue;

    /**********************************************************************
     *  end of HuffExtend
     *********************************************************************/
}


/**************************************************************************
FUNCTION     GetResidueHuffmanExtend
---------------------------------------------------------------------------
DESCRIPTION  getting residue, updating 32bits buffer update
             then making huffman sign extension
---------------------------------------------------------------------------
INPUT  VALUE pBufferPtr: intermediate buffer of 32 bits
             nCodeSize : size of codeword
             nResidueSize: size of residue
---------------------------------------------------------------------------
RETURN VALUE none
---------------------------------------------------------------------------
NOTES        none
**************************************************************************/
int32_t jpegd_engine_sw_get_residue_huff_extend (jpegd_bitbuffer_t *p_bitbuffer,
                                                 int32_t nCodeSize,
                                                 int32_t nResidueSize)
{
    int32_t nResidue;

    /**********************************************************************
     * First need to get the nResidue and update the bit buffer
     *********************************************************************/

    /**********************************************************************
     * First left shift buffer with bits of nCodeSize
     * indicating already consuming the number of bits of nCodeSize
     *********************************************************************/
    p_bitbuffer->bit_accumulator <<= nCodeSize;

    /**********************************************************************
     * Second right shift buffer with bits of nResidueSize to get
     * the resude
     *********************************************************************/
    nResidue = p_bitbuffer->bit_accumulator >> (32 - nResidueSize);

    /**********************************************************************
    * Then left shift buffer with bits of nResidueSize
    * indicating already consuming the number of bits of nResidueSize
    *********************************************************************/
    p_bitbuffer->bit_accumulator <<= nResidueSize;

    /**********************************************************************
     * Making Sign Extension based on value of nResidue
     * if nResidue < extendTest[nResidueSize], nResidue is negative
     * otherwise,  nResidue is non-negative
     *
     * if nResidue is negative, return two's complement of nResidue + 1
     *                   which is to plus extendOffset[nResidueSize]
     * otherwise, return the value of nResidue
     *********************************************************************/
    return ( nResidue < extendTest[nResidueSize] ) ?
           ( nResidue + extendOffset[nResidueSize] ) : nResidue;

    /**********************************************************************
     *  end of GetResidueHuffExtend
     *********************************************************************/
}
