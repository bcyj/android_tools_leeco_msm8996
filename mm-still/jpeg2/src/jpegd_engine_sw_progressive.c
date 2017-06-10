/*========================================================================

*//** @file jpegd_engine_sw_progressive.c
  This file contains the implementations related to progressive decoding.

@par EXTERNALIZED FUNCTIONS
  jpegd_engine_sw_configure_progressive
  jpegd_engine_sw_decode_progressive

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/linux/main/latest/src/jpegd_engine_sw_progressive.c#1 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
12/02/09   mingy   Added checking engine output handler return value.
10/15/09   mingy   Modified iDCT function signature.
09/22/09   mingy   Changed the way to compute the resize factor.
                   Set output dimension in the same fashion regardless
                   the input format.
07/20/09   zhiminl Created.

========================================================================== */

/*------------------------------------------------------------------------
 *                          Include Files
 * ----------------------------------------------------------------------- */

#include <stdlib.h>
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_buffer_private.h"
#include "jpeg_writer.h"
#include "jpegd_engine_sw.h"
#include "jpegd.h"
#include "writer_utility.h"


/*------------------------------------------------------------------------
 *                          Global Data Definitions
 * ----------------------------------------------------------------------- */

extern const int32_t zag[64];

/* -----------------------------------------------------------------------
 *                          Forward Declarations
 * ----------------------------------------------------------------------- */

/* Extern functions */
extern int      jpegd_engine_sw_check_qtable   (jpegd_engine_sw_t *);
extern int      jpegd_engine_sw_check_htable   (jpegd_engine_sw_t *);
extern void     jpegd_engine_sw_output_gray_mcu(jpegd_engine_sw_t *);
extern void     jpegd_engine_sw_output_h1v1_mcu(jpegd_engine_sw_t *);
extern void     jpegd_engine_sw_output_h1v2_mcu(jpegd_engine_sw_t *);
extern void     jpegd_engine_sw_output_h2v1_mcu(jpegd_engine_sw_t *);
extern void     jpegd_engine_sw_output_h2v2_mcu(jpegd_engine_sw_t *);
extern void     jpegd_engine_sw_idct_1x1(jpegd_coeff_t *,
                                         jpegd_sample_t *,
                                         int32_t);
extern void     jpegd_engine_sw_idct_2x2(jpegd_coeff_t *,
                                         jpegd_sample_t *,
                                         int32_t);
extern void     jpegd_engine_sw_idct_4x4(jpegd_coeff_t *,
                                         jpegd_sample_t *,
                                         int32_t);
extern void     jpegd_engine_sw_idct_8x8(jpegd_coeff_t *,
                                         jpegd_sample_t *,
                                         int32_t);
extern int      jpegd_engine_sw_process_restart_marker(jpegd_engine_sw_t *,
                                                       int32_t *);
extern int      jpegd_engine_sw_init_bit_buffer(jpegd_engine_sw_t *);
extern void     jpegd_engine_sw_reset_bit_buffer(jpegd_engine_sw_t *);
extern uint32_t jpegd_engine_sw_get_bits(jpegd_engine_sw_t *, int32_t);
extern uint32_t jpegd_engine_sw_get_padded_bits(jpegd_engine_sw_t *, int32_t);
extern int32_t  jpegd_engine_sw_huff_extend(int32_t, int32_t);
extern int32_t  jpegd_engine_sw_get_residue_huff_extend(jpegd_bitbuffer_t *,
                                                        int32_t, int32_t);
extern void     jpegd_engine_sw_ac_code_residue_consumed(jpegd_engine_sw_t *,
                                                         int32_t);
extern void     jpegd_engine_sw_ac_bit_buffer_update(jpegd_engine_sw_t *,
                                                     int32_t);
extern void     jpegd_engine_sw_derive_huff(uint8_t *, uint8_t *, jpegd_derived_htable_t *);
extern int      jpegd_engine_sw_get_next_marker(jpegd_engine_sw_t *, uint8_t *);

/*------------------------------------------------------------------------
 *                          Static Function Declarations and Definitions
 * ----------------------------------------------------------------------- */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_dehuff_one_block_dc_first
---------------------------------------------------------------------------
DESCRIPTION     dehuff one block of DC first scan
---------------------------------------------------------------------------
INPUT VALUE     dcTable:       dc huffman table
                (* lastValue): pointer to last dc value, this value will
                               be changed if dc coefficients are
                               successfully decoded
                coeffBlockBuf: pointer to output coeffients block buffer.
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_dehuff_one_block_dc_first
(
    jpegd_engine_sw_t            *p_engine,
    const jpegd_derived_htable_t *p_dc_table,
    int32_t                      *last_dc_val,
    jpegd_coeff_t                *p_coeff_block
)
{
    jpegd_bitbuffer_t  *p_bitbuffer = &(p_engine->bitbuffer);
    const uint32_t      al = p_engine->currentScanProgressiveInfo.al;
    uint32_t            category_codesize;
    uint32_t            category;
    int32_t             residue = 0;

    /**********************************************************************
     * get the dc coefficient
     *********************************************************************/

    /**********************************************************************
     * jpegd_bitbuffer.buffer is a 32-bit buffer with left alignment
     * it right shifts 24 bits to get the most left 8 bits
     * then lookup in the dcTable to find the catorgory and code size
     *********************************************************************/
    category_codesize = (p_dc_table->nCategoryCodeSize[(p_bitbuffer->bit_accumulator >> 24) & 0xFF]);
    if (category_codesize == 0)
    {
        JPEG_DBG_ERROR("jpegd_engine_sw_progressive_dehuff_one_block_dc_first: "
                       "0 code size\n");
        return JPEGERR_EFAILED;
    }

    /**********************************************************************
     * category_codesize has 16 bits of catorgory and 16 bits of code size
     * if it is greater than 0, the code size is less than or equal to 8 bits
     * otherwise, the code size is greater than 8 bits
     *********************************************************************/
    if (((int32_t)category_codesize) > 0)
    {
        /******************************************************************
         * the code size is less than or equal to 8 bits, consume the
         * codeword (codesize bits, which is the lower 16 bits of
         * category_codesize) from bit buffer
         *****************************************************************/
        (void)jpegd_engine_sw_get_padded_bits(p_engine, (int32_t)(category_codesize & 0xFFFF));

        /******************************************************************
         * the category (residue size) is the most significant 16 bits
         * of category_codesize
         ******************************************************************/
        category = (category_codesize >> 16);
    }
    else
    {
        /******************************************************************
         * the code size is greater than 8 bits, the category is not in
         * the first nCategoryCodeSize table and a second search in the
         * symbol table is needed
         *****************************************************************/

        /******************************************************************
         * get the lower 16 bits, max_size is the size of longest codeword
         * which share the same first 8-bit codeword minus 8
         *****************************************************************/
        const uint32_t max_size = (category_codesize & 0xFFFF);
        const uint32_t bit_mask = (1 << max_size) - 1;

        /******************************************************************
         * the category is located at the entry where the inverse of most
         * significant 16 bits category_codesize (starting index in the
         * symbol table) plus (codeword >> 8), which is
         * ((jpegd_bitbuffer.buffer >> (24-max_size) & bit_mask)
         *****************************************************************/
        jpegd_derived_htable_entry_t entry;
        entry = p_dc_table->symbol[(uint16_t)(~(category_codesize >> 16))
                + (uint16_t)((p_bitbuffer->bit_accumulator >> (24 - max_size)) & bit_mask)];

        /******************************************************************
         * consume the codeword (entry.len bits) from bit buffer
         *****************************************************************/
        (void)jpegd_engine_sw_get_padded_bits(p_engine, (int32_t)(entry.len));

        /******************************************************************
         * get catergory (residue size) from lookup table
         ******************************************************************/
        category = (uint32_t)(entry.cat);
    }

    if (category != 0)
    {
        /******************************************************************
         * if category (residue size) is non-zero
         * 1. read out residue
         * 2. make huffman extension
         *****************************************************************/
        residue = jpegd_engine_sw_huff_extend(
            (int32_t)jpegd_engine_sw_get_padded_bits(p_engine, (int32_t)category),
            (int32_t)category);
    }

    /* Convert DC difference to actual value, update lastDcVal */
    *last_dc_val = (residue += *last_dc_val);

    /* Scale and output DC coefficient which is always coeff[0] */
    p_coeff_block[0] = (jpegd_coeff_t)(residue << al);

    return JPEGERR_SUCCESS;
} /* end jpegd_engine_sw_progressive_dehuff_one_block_dc_first() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_dehuff_one_mcu_dc_first
---------------------------------------------------------------------------
DESCRIPTION     dehuff one mcu of DC first scan
---------------------------------------------------------------------------
INPUT VALUE     none
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_dehuff_one_mcu_dc_first
(
    jpegd_engine_sw_t *p_engine
)
{
    uint32_t i;
    int      rc = JPEGERR_SUCCESS;

    for (i = 0; i < p_engine->nBlocksPerMCU; i++)
    {
        // find the components id.
        const uint8_t comp_id = p_engine->MCUBlockCompList[i];

        // dehuff and de-zigzag DC in one block
        rc = jpegd_engine_sw_progressive_dehuff_one_block_dc_first(
            p_engine,
            p_engine->pDerivedHuffTable[p_engine->dcHuffTableId[comp_id]],
            &p_engine->lastDcVal[comp_id],
            p_engine->pMCUCoeffBlockBuf[i]);

        if (JPEG_FAILED(rc)) return rc;
    }
    return rc;
} /* end jpegd_engine_sw_progressive_dehuff_one_mcu_dc_first() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_dehuff_one_mcu_dc_refine
---------------------------------------------------------------------------
DESCRIPTION     dehuff one mcu of DC subsequent scans
---------------------------------------------------------------------------
INPUT VALUE     none
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_dehuff_one_mcu_dc_refine
(
    jpegd_engine_sw_t *p_engine
)
{
    const uint32_t al = p_engine->currentScanProgressiveInfo.al;
    const int      rc = JPEGERR_SUCCESS;

    uint32_t i;
    for (i = 0; i < p_engine->nBlocksPerMCU; i++)
    {
        jpegd_coeff_t *p_coeff_block = p_engine->pMCUCoeffBlockBuf[i];

        // dehuff and de-zigzag DC in one block
        if (jpegd_engine_sw_get_padded_bits(p_engine, 1) != 0)
        {
            // append bit 1 at position al of DC coefficient
            p_coeff_block[0] |= (int16_t)(1 << al);
        }
    }
    return rc;
} /* end jpegd_engine_sw_progressive_dehuff_one_mcu_dc_refine() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_dehuff_one_mcu_ac_first
---------------------------------------------------------------------------
DESCRIPTION     dehuff one mcu of AC first scan; also de-zigzag the
                coefficients order
---------------------------------------------------------------------------
INPUT VALUE     acTable:       ac huffman table
                coeffBlockBuf: pointer to output coeffients block buffer.
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_dehuff_one_mcu_ac_first
(
    jpegd_engine_sw_t            *p_engine,
    const jpegd_derived_htable_t *p_ac_table,
    jpegd_coeff_t                *p_coeff_block
)
{
    int rc = JPEGERR_SUCCESS;

    if (p_engine->EOBRUN != 0)
    {
        // This block is all-zeros for the selected spectral
        p_engine->EOBRUN--;
    }
    else
    {
        jpegd_bitbuffer_t *p_bitbuffer = &(p_engine->bitbuffer);
        uint32_t           symbol_codesize;
        uint32_t           symbol;
        int32_t            codesize;
        uint32_t           run;
        int32_t            size;
        int32_t            coefficient;
        uint32_t           k;

        const uint32_t ss = p_engine->currentScanProgressiveInfo.ss;
        const uint32_t se = p_engine->currentScanProgressiveInfo.se;
        const uint32_t al = p_engine->currentScanProgressiveInfo.al;

        /**********************************************************************
         * decode the ac coefficients with ac table.
         *********************************************************************/
        for (k = ss; k <= se; k++)
        {
            /**********************************************************************
             * jpegd_bitbuffer.buffer is a 32-bit buffer with left alignment
             * it right shifts 24 bits to get the most left 8 bits
             * then lookup in the acTable to find the symbol and code size
             *********************************************************************/
            symbol_codesize = (p_ac_table->nCategoryCodeSize[(p_bitbuffer->bit_accumulator >> 24) & 0xFF]);
            if (symbol_codesize == 0)
            {
                JPEG_DBG_ERROR("jpegd_engine_sw_progressive_dehuff_one_block_ac_first: "
                               "0 code size, k=%d\n", k);
                return JPEGERR_EFAILED;
            }

            /**********************************************************************
             * symbol_codesize has 16 bits of symbol and 16 bits of code size
             * if it is greater than 0, the code size is less than or equal to 8 bits
             * otherwise, the code size is greater than 8 bits
             *********************************************************************/
            if ((int32_t)symbol_codesize > 0)
            {
                /******************************************************************
                 * the code size is less than or equal to 8 bits, consume the
                 * codeword (codesize bits, which is the lower 16 bits of
                 * symbol_codesize) from bit buffer
                 *****************************************************************/
                codesize = (int32_t)(symbol_codesize & 0xFFFF);

                /******************************************************************
                 * the symbol (run/size) is the most significant 16 bits
                 * of symbol_codesize
                 ******************************************************************/
                symbol = (symbol_codesize >> 16);
            }
            else
            {
                /******************************************************************
                 * the code size is greater than 8 bits, the symbol is not in
                 * the first nCategoryCodeSize table and a second search in the
                 * symbol table is needed
                 *****************************************************************/

                /******************************************************************
                 * get the lower 16 bits, max_size is the size of longest codeword
                 * which share the same first 8-bit codeword minus 8
                 *****************************************************************/
                const uint32_t max_size = (symbol_codesize & 0xFFFF);
                const uint32_t bit_mask = (1 << max_size) - 1;

                /******************************************************************
                 * the symbol is located at the entry where the inverse of most
                 * significant 16 bits symbol_codesize (starting index in the
                 * symbol table) plus (codeword >> 8), which is
                 * ((jpegd_bitbuffer.buffer >> (24-max_size) & bit_mask)
                 *****************************************************************/
                jpegd_derived_htable_entry_t entry;
                entry = p_ac_table->symbol[(uint16_t)(~(symbol_codesize >> 16))
                        + (uint16_t)((p_bitbuffer->bit_accumulator >> (24 - max_size)) & bit_mask)];

                /**************************************************************
                 * get the codesize (entry.len ) from table
                 **************************************************************/
                codesize = (int32_t)(entry.len);

                /**************************************************************
                 * get symbol (run/size) from table
                 **************************************************************/
                symbol = (uint32_t)(entry.cat);
            }

            /******************************************************************
             * run  : run which is the higher 4 bits
             * size : size which is the size of residue
             *****************************************************************/
            run  = symbol >> 4;
            size = symbol & 0xF;

            if (size != 0)
            {
                if (run != 0)
                {
                    // the run
                    if ((k += run) > se)
                    {
                        JPEG_DBG_ERROR("jpegd_engine_sw_progressive_dehuff_one_block_ac_first: "
                                       "k=%d, run=%d, size=%d\n", k, run, size);
                        rc = JPEGERR_EFAILED;
                    } // end if ((k + run) > Se)
                } // end if (run != 0)

                /**************************************************************
                 * the jpegd_bitbuffer guarantee to have at least 16 bits
                 * if (codesize + size(residue size)) <= 16, can go ahead to
                 * read out the residue, after that if necessary, update the
                 * bit buffer from bitstream
                 *************************************************************/
                if ((codesize + size) <= 16)
                {
                    /**********************************************************
                     * this function is
                     * 1. consume the codeword (codesize bits) from bit buffer
                     * 2. read out size bits (residue) from bit buffer
                     * 3. make huffman extension
                     *********************************************************/
                    coefficient = jpegd_engine_sw_get_residue_huff_extend(p_bitbuffer, codesize, size);

                    /**********************************************************
                     * update the jpegd_bitbuffer
                     *********************************************************/
                    jpegd_engine_sw_ac_code_residue_consumed(p_engine, (codesize + size));
                }
                else
                {
                    /**********************************************************
                     * if (codesize + size) > 16
                     * 1. consume the codeword (codesize bits) from bit buffer
                     * 2. read out size bits (residue) from bit buffer
                     * 3. make huffman extersion
                     *********************************************************/
                    jpegd_engine_sw_ac_bit_buffer_update(p_engine, codesize);
                    coefficient = jpegd_engine_sw_huff_extend(
                        (jpegd_engine_sw_get_padded_bits(p_engine, size)), size);
                }

                /**************************************************************
                 * unzigzag
                 *************************************************************/
                p_coeff_block[zag[k]] = (int16_t)(coefficient << al);
            } // end if (size != 0)
            else
            {
                /**************************************************************
                 * just consume the codeword (codesize bits) from bit buffer
                 *************************************************************/
                jpegd_engine_sw_ac_bit_buffer_update(p_engine, codesize);

                // is ZRL ?
                if (run == 15)
                {
                    // This is ZRL
                    if ((k += run) > se)
                    {
                        JPEG_DBG_ERROR("jpegd_engine_sw_progressive_dehuff_one_block_ac_first: "
                                       "k=%d, r=%d, s=%d\n", k, run, size);
                        rc = JPEGERR_EFAILED;
                    } // end if ((k + run) > se)
                } // end if (run == 15)
                else
                {
                    // This is EOBn
                    p_engine->EOBRUN = (1 << run);
                    if (run != 0)
                    {
                        p_engine->EOBRUN += jpegd_engine_sw_get_padded_bits(p_engine, run);
                    }
                    p_engine->EOBRUN--;
                    break;
                } // end (run != 15)
            } // end (size == 0)
        } // end for (k = ss; k <= se; k++)
    } // end (p_engine->EOBRUN == 0)

    return rc;
} /* end jpegd_engine_sw_progressive_dehuff_one_mcu_ac_first() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_dehuff_one_mcu_ac_refine
---------------------------------------------------------------------------
DESCRIPTION     dehuff one mcu of AC subsequent scans; also de-zigzag the
                coefficients order
---------------------------------------------------------------------------
INPUT VALUE     acTable:       ac huffman table
                coeffBlockBuf: pointer to output coeffients block buffer.
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_dehuff_one_mcu_ac_refine
(
    jpegd_engine_sw_t            *p_engine,
    const jpegd_derived_htable_t *p_ac_table,
    jpegd_coeff_t                *p_coeff_block
)
{
    jpegd_bitbuffer_t *p_bitbuffer = &(p_engine->bitbuffer);
    const uint32_t     ss = p_engine->currentScanProgressiveInfo.ss;
    const uint32_t     se = p_engine->currentScanProgressiveInfo.se;
    const uint32_t     al = p_engine->currentScanProgressiveInfo.al;
    const int16_t      p1 = (int16_t)(1 << al);
    const int16_t      n1 = (int16_t)((-1) << al);
    uint32_t           k;
    int                rc = JPEGERR_SUCCESS;

    k = ss;
    if (p_engine->EOBRUN == 0)
    {
        uint32_t symbol_codesize;
        uint32_t symbol;
        int32_t  codesize;
        int32_t  run;
        int32_t  size;
        int32_t  sign_bit;
        int32_t  coefficient = 0;

        /**********************************************************************
         * decode the ac coefficients with ac table.
         *********************************************************************/
        for (; k <= se; k++)
        {
            /**********************************************************************
             * jpegd_bitbuffer.buffer is a 32-bit buffer with left alignment
             * it right shifts 24 bits to get the most left 8 bits
             * then lookup in the acTable to find the symbol and code size
             *********************************************************************/
            symbol_codesize = (p_ac_table->nCategoryCodeSize[(p_bitbuffer->bit_accumulator >> 24) & 0xFF]);
            if (symbol_codesize == 0)
            {
                JPEG_DBG_ERROR("jpegd_engine_sw_progressive_dehuff_one_block_ac_refine: "
                               "0 code size, k=%d\n", k);
                return JPEGERR_EFAILED;
            }

            /**********************************************************************
             * symbol_codesize has 16 bits of symbol and 16 bits of code size
             * if it is greater than 0, the code size is less than or equal to 8 bits
             * otherwise, the code size is greater than 8 bits
             *********************************************************************/
            if ((int32_t)symbol_codesize > 0)
            {
                /******************************************************************
                 * the code size is less than or equal to 8 bits, consume the
                 * codeword (codesize bits, which is the lower 16 bits of
                 * symbol_codesize) from bit buffer
                 *****************************************************************/
                codesize = (int32_t)(symbol_codesize & 0xFFFF);

                /******************************************************************
                 * the symbol (run/size) is the most significant 16 bits
                 * of symbol_codesize
                 ******************************************************************/
                symbol = (symbol_codesize >> 16);
            }
            else
            {
                /******************************************************************
                 * the code size is greater than 8 bits, the symbol is not in
                 * the first nCategoryCodeSize table and a second search in the
                 * symbol table is needed
                 *****************************************************************/

                /******************************************************************
                 * get the lower 16 bits, max_size is the size of longest codeword
                 * which share the same first 8-bit codeword minus 8
                 *****************************************************************/
                const uint32_t max_size = (symbol_codesize & 0xFFFF);
                const uint32_t bit_mask = (1 << max_size) - 1;

                /******************************************************************
                 * the symbol is located at the entry where the inverse of most
                 * significant 16 bits symbol_codesize (starting index in the
                 * symbol table) plus (codeword >> 8), which is
                 * ((jpegd_bitbuffer.buffer >> (24-max_size) & bit_mask)
                 *****************************************************************/
                jpegd_derived_htable_entry_t entry;
                entry = p_ac_table->symbol[(uint16_t)(~(symbol_codesize >> 16))
                        + (uint16_t)((p_bitbuffer->bit_accumulator >> (24 - max_size)) & bit_mask)];

                /**************************************************************
                 * get the codesize (entry.len ) from table
                 **************************************************************/
                codesize = (int32_t)(entry.len);

                /**************************************************************
                * get symbol (run/size) from table
                **************************************************************/
                symbol = (uint32_t)(entry.cat);
            }

            /******************************************************************
             * run  : run which is the higher 4 bits
             * size : size which is the size of residue
             *****************************************************************/
            run  = (int32_t)(symbol >> 4);
            size = (int32_t)(symbol & 0xF);

            /**************************************************************
             * for AC refine scan, the symbol is either:
             * 1. 0xX1 (newly non-zero coefficient) or
             * 2. 0xX0 (ZRL or EOBn)
             *************************************************************/
            if (size != 0)
            {
                // The symbol must be 0xX1, otherwise something is wrong
                if (size != 1)
                {
                    JPEG_DBG_ERROR("jpegd_engine_sw_progressive_dehuff_one_block_ac_refine: "
                                   "k=%d, run=%d, size=%d\n", k, run, size);
                    // force size to be 1
                    size = 1;
                    rc = JPEGERR_EFAILED;
                }
                /**************************************************************
                 * the jpegd_bitbuffer guarantee to have at least 16 bits
                 * if (codesize + 1 (sign bit)) <= 16, can go ahead to
                 * read out the residue, after that if necessary, update the
                 * bit buffer from bitstream
                 *************************************************************/
                if ((codesize + size) <= 16)
                {
                    /**********************************************************
                     * this function is
                     * 1. consume the codeword (codesize bits) from bit buffer
                     * 2. read out 1 bit from bit buffer
                     * 3. make huffman extension
                     *********************************************************/
                    sign_bit = jpegd_engine_sw_get_residue_huff_extend(p_bitbuffer, codesize, size);

                    /**********************************************************
                     * update the jpegd_bitbuffer
                     *********************************************************/
                    jpegd_engine_sw_ac_code_residue_consumed(p_engine, (codesize + size));
                }
                else
                {
                    /**********************************************************
                     * if (codesize + size) > 16
                     * 1. consume the codeword (codesize bits) from bit buffer
                     * 2. read out 1 bit from bit buffer
                     * 3. make huffman extersion
                     *********************************************************/
                    jpegd_engine_sw_ac_bit_buffer_update(p_engine, codesize);
                    sign_bit = jpegd_engine_sw_huff_extend(
                        (jpegd_engine_sw_get_padded_bits(p_engine, size)), size);
                }
                // Calculate the newly non-zero coefficient - G.1.2.3 rule a
                if (sign_bit > 0)
                {
                    coefficient = p1; // the newly non-zero is positive: 1 << al
                }
                else
                {
                    coefficient = n1; // the newly non-zero is negative: -1 << al
                }
            } // end if (size != 0)
            else
            {
                /**************************************************************
                 * just consume the codeword (codesize bits) from bit buffer
                 *************************************************************/
                jpegd_engine_sw_ac_bit_buffer_update(p_engine, codesize);

                // is EOBn ?
                if (run != 15)
                {
                    // This is EOBn
                    p_engine->EOBRUN = (1 << run);
                    if (run != 0)
                    {
                        p_engine->EOBRUN += jpegd_engine_sw_get_padded_bits(p_engine, run);
                    }
                    break; // EOBn will be handled later
                }
            } // end (size == 0)

            /**************************************************************
             * the loop handle 0xX1 and 0xF0 (ZRL)
             * 1. go through already-nonzero coefficients, appending correction
             *    bits to each already-nonzero coefficient if necessary
             * 2. skip run of still-zero coefficients
             *************************************************************/
            do
            {
                int16_t nonzero;
                if ((nonzero = p_coeff_block[zag[k]]) != 0)
                {
                    if (jpegd_engine_sw_get_padded_bits(p_engine, 1) != 0)
                    {
                        if ((nonzero & p1) == 0)
                        {
                            // Append the correction bit - G.1.2.3 rule b
                            p_coeff_block[zag[k]] = nonzero +
                                                    ((nonzero > 0) ? p1 : n1);
                        }
                    } // end if (jpegd_engine_sw_get_padded_bits(p_engine, 1) != 0)
                } // end (p_coeff_block[zag[k]] != 0)
                else
                {
                    if ((--run) < 0)
                    {
                        break; // skip run of still-zeros
                    }
                } // end (p_coeff_block[zag[k]] == 0)
                k++;
            } while (k <= se);
            // Save the newly non-zero coefficient
            if ((size != 0) && (k <= se))
            {
                p_coeff_block[zag[k]] = (int16_t)coefficient;
            }
        } // end for (; k <= se; k++)
    } // end if (p_engine->EOBRUN == 0)

     /**************************************************************
      * Go through the remaining coefficients after the EOB
      * (the last newly non-zero coefficient, if any),  appending
      * a correction bit to each already-nonzero coefficient if needed
      *************************************************************/
    if (p_engine->EOBRUN > 0)
    {
        for (; k <= se; k++)
        {
            int16_t nonzero;
            if ((nonzero = p_coeff_block[zag[k]]) != 0)
            {
                if (jpegd_engine_sw_get_padded_bits(p_engine, 1) != 0)
                {
                    if ((nonzero & p1) == 0)
                    {
                        // Append the correction bit - G.1.2.3 rule b
                        p_coeff_block[zag[k]] = nonzero +
                                                ((nonzero > 0) ? p1 : n1);
                    }
                } // end if (jpegd_engine_sw_get_padded_bits(p_engine, 1) != 0)
            } // end (p_coeff_block[zag[k]] != 0)
        } // end for (; k <= se; k++)
        p_engine->EOBRUN--;
    } // end if (p_engine->EOBRUN > 0)

    return rc;
} /* end jpegd_engine_sw_progressive_dehuff_one_mcu_ac_refine() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_dehuff_one_mcu
---------------------------------------------------------------------------
DESCRIPTION     dehuff one mcu of samples
---------------------------------------------------------------------------
INPUT VALUE     none
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_dehuff_one_mcu(jpegd_engine_sw_t *p_engine)
{
    const jpegd_progressive_info_t *p_progressive_info = &(p_engine->currentScanProgressiveInfo);
    const int rc = JPEGERR_EFAILED;

    /**********************************************************************
     * dehuff one mcu according to spectral selection/successive approximation
     * settings.
     *********************************************************************/
    if (p_progressive_info->ss == 0)
    {
        // DC scans
        if (p_progressive_info->ah == 0)
        {
            // This is a DC first scan
            return jpegd_engine_sw_progressive_dehuff_one_mcu_dc_first(p_engine);
        }
        else
        {
            // This is a DC refine scan
            return jpegd_engine_sw_progressive_dehuff_one_mcu_dc_refine(p_engine);
        }
    }
    else
    {
        // AC scans shall have only one component
        if (p_engine->nBlocksPerMCU == 1)
        {
            // find the component id
            const uint8_t comp_id = p_engine->MCUBlockCompList[0];

            if (p_progressive_info->ah == 0)
            {
                // This is a AC first scan
                // dehuff and de-zigzag AC in one MCU (one block per MCU)
                return jpegd_engine_sw_progressive_dehuff_one_mcu_ac_first(
                    p_engine,
                    p_engine->pDerivedHuffTable[p_engine->acHuffTableId[comp_id]],
                    p_engine->pMCUCoeffBlockBuf[0]);
            }
            else
            {
                // This is a AC refine scan
                // dehuff and de-zigzag AC in one MCU (one block per MCU)
                return jpegd_engine_sw_progressive_dehuff_one_mcu_ac_refine (
                    p_engine,
                    p_engine->pDerivedHuffTable[p_engine->acHuffTableId[comp_id]],
                    p_engine->pMCUCoeffBlockBuf[0]);
            } // end if (p_progressive_info->ah == 0)
        } // end if (p_engine->nBlocksPerMCU == 1)
    }
    return rc;
} /* end jpegd_engine_sw_progressive_dehuff_one_mcu() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_dequant
---------------------------------------------------------------------------
DESCRIPTION     dequant one block of coefficients in place
---------------------------------------------------------------------------
INPUT VALUE     qtable:        quant table
                coeffBlockBuf: pointer to input and output coeffients
                               block buffer.
---------------------------------------------------------------------------
RETURN VALUE    none
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static void jpegd_engine_sw_progressive_dequant
(
    const jpeg_quant_table_t  p_qtable,
          jpegd_coeff_t      *p_coeff_block
)
{
    uint32_t i;

    // quant table is in zigzag order
    for (i = 0; i < JPEGD_DCTSIZE2; i++)
    {
        jpegd_coeff_t coefficient;
        if ((coefficient = p_coeff_block[zag[i]]) != 0)
        {
            p_coeff_block[zag[i]] = coefficient * ((int16_t)(p_qtable[i]));
        }
    }
} /* end jpegd_engine_sw_progressive_dequant() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_process_dqt_marker
---------------------------------------------------------------------------
DESCRIPTION     process DQT marker before SOS
---------------------------------------------------------------------------
INPUT  VALUE    none
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_process_dqt_marker
(
    jpegd_engine_sw_t *p_engine
)
{
    uint32_t  length, i, j;
    uint8_t   table_id;

    // get marker length
    length = jpegd_engine_sw_get_bits(p_engine, 16);

    for (i = 0; i < (length >> 6); i++)
    {
        uint16_t qtable[64];

        // get table precision and table id
        const uint8_t byte = (uint8_t)jpegd_engine_sw_get_bits(p_engine, 8);
        const uint8_t precision = (byte >> 4);
        table_id = (byte & 0xF);

        if (precision > 1 || table_id >= JPEGD_MAXQUANTTABLES) return JPEGERR_EBADPARM;

        // validate the table
        if ((uint32_t)(precision * 64 + 67) > length) return JPEGERR_EBADPARM;

        // fill in the quant table values
        (void)STD_MEMSET(&qtable, 0, sizeof(qtable));
        for (j = 0; j < 64; j++)
        {
            qtable[j] = (uint16_t)((precision) ?
                jpegd_engine_sw_get_bits(p_engine, 16) :
                jpegd_engine_sw_get_bits(p_engine, 8));
        }
        if (!p_engine->pDeQuantTable[table_id])
        {
            p_engine->pDeQuantTable[table_id] =
                (jpeg_quant_table_t)JPEG_MALLOC(64 * sizeof(uint16_t));
            if (!p_engine->pDeQuantTable[table_id]) return JPEGERR_EMALLOC;
        }
        // copy the new quant table
        (void)STD_MEMMOVE(p_engine->pDeQuantTable[table_id], qtable, sizeof(qtable));
    }

    return JPEGERR_SUCCESS;
} /* end jpegd_engine_sw_progressive_process_dqt_marker() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_process_dht_marker
---------------------------------------------------------------------------
DESCRIPTION     process DHT marker before SOS
---------------------------------------------------------------------------
INPUT  VALUE    none
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_process_dht_marker
(
    jpegd_engine_sw_t *p_engine
)
{
    uint32_t  sum, size, i;
    uint32_t  table_id;

    // get marker length
    size = jpegd_engine_sw_get_bits(p_engine, 16);
    size -= 2;

    while (size > 16)
    {
        jpeg_huff_table_t huff_table;
        (void)STD_MEMSET(&huff_table, 0, sizeof(huff_table));

        table_id = jpegd_engine_sw_get_bits(p_engine, 8);
        if (table_id & 0xEC)
        {
            // wrong huff table
            return JPEGERR_EBADPARM;
        }
        if (table_id & 0x10)
        {
            table_id = (table_id & 0x3) + 4;
        }

        huff_table.bits[0] = 0;
        sum = 0;

        for (i = 1; i < 17; i++)
        {
            huff_table.bits[i] = (uint8_t)(jpegd_engine_sw_get_bits(p_engine, 8));
            sum += (uint32_t)(huff_table.bits[i]);
        }

        if (sum > 256)
        {
            // invalid huff bit count;
            return JPEGERR_EBADPARM;
        }

        for (i = 0; i < sum; i++)
        {
            huff_table.values[i] = (uint8_t)(jpegd_engine_sw_get_bits(p_engine, 8));
        }

        // make the derived huffman table
        if (!p_engine->pDerivedHuffTable[table_id])
        {
            // allocate the memory for derived huffman table
            p_engine->pDerivedHuffTable[table_id] =
                (jpegd_derived_htable_t *)JPEG_MALLOC(sizeof(jpegd_derived_htable_t));
            if (!p_engine->pDerivedHuffTable[table_id]) return JPEGERR_EMALLOC;
        }

        // make corresponding derived huffman table
        jpegd_engine_sw_derive_huff(huff_table.bits,
                                    huff_table.values,
                                    p_engine->pDerivedHuffTable[table_id]);

        size -= (sum + 17);
    } // end while (size > 16)

    return JPEGERR_SUCCESS;
} /* end jpegd_engine_sw_progressive_process_dht_marker() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_process_dri_marker
---------------------------------------------------------------------------
DESCRIPTION     process DRI marker before SOS
---------------------------------------------------------------------------
INPUT  VALUE    none
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_process_dri_marker
(
    jpegd_engine_sw_t *p_engine
)
{
    const uint32_t length = jpegd_engine_sw_get_bits(p_engine, 16);
    if (length != 4) return JPEGERR_EBADPARM;

    p_engine->nRestartInterval = (int32_t)jpegd_engine_sw_get_bits(p_engine, 16);
    return JPEGERR_SUCCESS;
} /* jpegd_engine_sw_progressive_process_dri_marker */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_get_sos_marker
---------------------------------------------------------------------------
DESCRIPTION     get SOS marker
---------------------------------------------------------------------------
INPUT  VALUE    (* noMoreScans): flag indicating if there is no more scans
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_get_sos_marker
(
    jpegd_engine_sw_t *p_engine,
    uint8_t           *noMoreScans
)
{
    uint8_t marker;
    int     rc;

    for (; ;)
    {
        // first get the marker
        rc = jpegd_engine_sw_get_next_marker(p_engine, &marker);
        if (JPEG_FAILED(rc)) return rc;

        /******************************************************************
         * markers need to be filtered throu, in bitstream, should only be
         * STR markers, and they have no parameters.
         *****************************************************************/
        switch (marker)
        {
        case M_SOS:
        {
            // found!
            *noMoreScans = 0;
            return rc;
        }
        case M_DHT:
        {
            rc = jpegd_engine_sw_progressive_process_dht_marker(p_engine);
            if (JPEG_FAILED(rc)) return rc;
            break;
        }
        case M_DQT:
        {
            rc = jpegd_engine_sw_progressive_process_dqt_marker(p_engine);
            if (JPEG_FAILED(rc)) return rc;
            break;
        }
        case M_DRI:
        {
            rc = jpegd_engine_sw_progressive_process_dri_marker(p_engine);
            if (JPEG_FAILED(rc)) return rc;
            break;
        }
        case M_EOI:
        {
            // if EOI, that means we dont have any more scans in this frame
            // return and let the decode engine know.
            *noMoreScans = 1;
            return rc;
        }

        default:
        {
            continue;
        } // default
        } // end of switch
    } // end for (; ;)
} /* end jpegd_engine_sw_progressive_get_sos_marker() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_process_sos_marker
---------------------------------------------------------------------------
DESCRIPTION     process SOS marker
---------------------------------------------------------------------------
INPUT  VALUE    (* noMoreScans): flag indicating if there is no more scans
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_process_sos_marker
(
    jpegd_engine_sw_t *p_engine,
    uint8_t           *noMoreScans
)
{
    jpeg_frame_info_t *p_frame_info = p_engine->source.p_frame_info;
    uint32_t           length, num_comps, i;
    uint8_t            byte;
    int                rc;

    // first reset buffer to make it byte alianed
    jpegd_engine_sw_reset_bit_buffer(p_engine);

    // then get the sos marker
    rc = jpegd_engine_sw_progressive_get_sos_marker(p_engine, noMoreScans);
    if (JPEG_FAILED(rc)) return rc;

    // if there is not more scans, return and let decoder know.
    if (*noMoreScans == 1) return rc;

    // clear the scan info from previous scan
    p_engine->nNumberOfComponentsInScan = 0;
    (void)STD_MEMSET(p_engine->compEntropySelectors, 0, sizeof(p_engine->compEntropySelectors));

    // get marker length
    length = jpegd_engine_sw_get_bits(p_engine, 16);

    // find number of components in this sos
    num_comps = jpegd_engine_sw_get_bits(p_engine, 8);

    if ((num_comps < 1) || (num_comps > JPEGD_MAXCOMPONENTS))
        return JPEGERR_EBADPARM;

    if (length != (num_comps * 2 + 6))
        return JPEGERR_EBADPARM;

    for (i = 0; i < num_comps; i++)
    {
        uint8_t comp_id = (uint8_t)jpegd_engine_sw_get_bits(p_engine, 8);
        uint8_t dc_selector, ac_selector;
        uint8_t j;

        byte = (uint8_t)jpegd_engine_sw_get_bits(p_engine, 8);
        dc_selector = (uint8_t)(byte >> 4);
        ac_selector = (uint8_t)(byte & 0x0F);

        for (j = 0; j < p_engine->nNumberOfComponentsInFrame; j++)
        {
            if (comp_id == p_frame_info->p_comp_infos[j].comp_id)
            {
                // found the component
                comp_id = j;
                break;
            }
        }

        if ((j >= p_engine->nNumberOfComponentsInFrame) ||
            (dc_selector >= 4) || (ac_selector >= 4))
        {
            return JPEGERR_EBADPARM;
        }

        p_engine->compEntropySelectors[i].comp_id = comp_id;
        p_engine->compEntropySelectors[i].dc_selector = dc_selector;
        p_engine->compEntropySelectors[i].ac_selector = ac_selector;
    } // end for (i = 0; i < num_comps; i++)

    p_engine->nNumberOfComponentsInScan = (uint8_t)num_comps;

    // find spectral selection and successive approximation
    p_engine->currentScanProgressiveInfo.ss = jpegd_engine_sw_get_bits(p_engine, 8);
    p_engine->currentScanProgressiveInfo.se = jpegd_engine_sw_get_bits(p_engine, 8);
    byte = (uint8_t)jpegd_engine_sw_get_bits(p_engine, 8);
    p_engine->currentScanProgressiveInfo.ah = (byte >> 4);
    p_engine->currentScanProgressiveInfo.al = (byte & 0x0F);

    if ((p_engine->currentScanProgressiveInfo.ss >= 64) ||
        (p_engine->currentScanProgressiveInfo.se >= 64) ||
        (p_engine->currentScanProgressiveInfo.ah >= 14) ||
        (p_engine->currentScanProgressiveInfo.al >= 14))
    {
        return JPEGERR_EBADPARM;
    }

    // Valication if DC scan, Ss = Se = 0 (Ns >= 1)
    //            if AC scan, Ss != 0, Se >= Ss, Ns = 1
    //            if refine scan, Ah = Al + 1
    if (((p_engine->currentScanProgressiveInfo.ss == 0) &&
         (p_engine->currentScanProgressiveInfo.se != 0)) ||
        ((p_engine->currentScanProgressiveInfo.ss != 0) &&
         ((p_engine->currentScanProgressiveInfo.se < p_engine->currentScanProgressiveInfo.ss) ||
          (num_comps != 1))) ||
        ((p_engine->currentScanProgressiveInfo.ah != 0) &&
         (p_engine->currentScanProgressiveInfo.ah != (p_engine->currentScanProgressiveInfo.al + 1))))
    {
        return JPEGERR_EBADPARM;
    }

    // update the scan id
    p_engine->currentScanId++;

    /**********************************************************************
     * Reload the bit buffer to get ready for next mcu decoding
     *********************************************************************/
    jpegd_engine_sw_reset_bit_buffer(p_engine);

    return JPEGERR_SUCCESS;
} /* end jpegd_engine_sw_progressive_process_sos_marker() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_check_qtable
---------------------------------------------------------------------------
DESCRIPTION     check if the correct quant tables are initialized
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    JPEGERR_SUCCESS if successful, JPEGERR_EFAILED otherwise
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_check_qtable(jpegd_engine_sw_t *p_engine)
{
    uint32_t i;

    // check quant table - no quant table, no decoding
    for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++)
    {
        uint8_t table_id = p_engine->quantId[p_engine->compListInScan[i]];
        if (!p_engine->pDeQuantTable[table_id])
        {
            JPEG_DBG_ERROR("jpegd_engine_sw_progressive_check_qtable: "
                           "undefined quant table %d\n", table_id);
            return JPEGERR_EFAILED;
        }
    }

    return JPEGERR_SUCCESS;
}

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_check_htable
---------------------------------------------------------------------------
DESCRIPTION     check if the correct huff tables are initialized
---------------------------------------------------------------------------
INPUT  VALUE    None
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_check_htable(jpegd_engine_sw_t *p_engine)
{
    uint8_t i;

    // check huffman table - no huffman table, no decoding
    for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++)
    {
        uint8_t table_id;
        if (p_engine->currentScanProgressiveInfo.ss == 0)
        {
            // The scan has DC
            table_id = p_engine->dcHuffTableId[p_engine->compListInScan[i]];
            if (!p_engine->pDerivedHuffTable[table_id])
            {
                JPEG_DBG_ERROR("jpegd_engine_sw_progressive_check_htable: "
                               "undefined dc huff table %d\n", table_id);
                return JPEGERR_EFAILED;
            }
        }
        if (p_engine->currentScanProgressiveInfo.se != 0)
        {
            // The scan has AC
            table_id = p_engine->acHuffTableId[p_engine->compListInScan[i]];
            if (!p_engine->pDerivedHuffTable[table_id])
            {
                JPEG_DBG_ERROR("jpegd_engine_sw_progressive_check_htable: "
                               "undefined ac huff table %d\n", table_id);
                return JPEGERR_EFAILED;
            }
        }
    }

    return JPEGERR_SUCCESS;
} /* end jpegd_engine_sw_progressive_check_htable */

/**************************************************************************
FUNCTION        jpegd_engine_sw_progressive_get_next_scan
---------------------------------------------------------------------------
DESCRIPTION     initialize progressive scan specific information for
                second and so on. The FIRST scan specific information is
                initialized in InitProgressive().
---------------------------------------------------------------------------
INPUT  VALUE    none, but depend on decodeSpec
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
static int jpegd_engine_sw_progressive_get_next_scan
(
    jpegd_engine_sw_t *p_engine
)
{
    uint8_t comp_id, i;
    int     rc;

    if (p_engine->nNumberOfComponentsInScan == 0)
    {
        // scan info is not initialized properly
        return JPEGERR_EFAILED;
    }

    // component huffman info for the scan
    for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++)
    {
        const jpeg_comp_entropy_sel_t *p_selector = &p_engine->compEntropySelectors[i];

        // get component id
        comp_id = p_selector->comp_id;
        // dc table index
        p_engine->dcHuffTableId[comp_id] = p_selector->dc_selector;
        // since only one set of huff table index is used,
        // ac table is appened to dc table with index starting from
        // JPEGD_MAXHUFFTABLES/2.
        p_engine->acHuffTableId[comp_id] = p_selector->ac_selector
                                           + (uint8_t)(JPEGD_MAXHUFFTABLES >> 1);
        // get components list in a MCU in one scan
        p_engine->compListInScan[i] = comp_id;
    }

    /**********************************************************************
     * need to calculate the number of MCUs per row and col for the scan.
     * This is determind by the color format and number of blocks in image
     * For each mcu, the type of block need to be labeled also
     *********************************************************************/
    if (p_engine->nNumberOfComponentsInScan == 1)
    {
        comp_id = p_engine->compListInScan[0];
        p_engine->nMCUPerRow = ((7 + ((p_engine->nImageWidth * (uint32_t)(p_engine->hSample[comp_id]))
                                      + (uint32_t)(p_engine->maxHSample - 1))
                                     / (uint32_t)(p_engine->maxHSample)) / 8);
        p_engine->nMCUPerCol = ((7 + ((p_engine->nImageHeight * (uint32_t)(p_engine->vSample[comp_id]))
                                      + (uint32_t)(p_engine->maxVSample - 1))
                                     / (uint32_t)(p_engine->maxVSample)) / 8);
        p_engine->nBlocksPerMCU = 1;
        p_engine->MCUBlockCompList[0] = comp_id;
    }
    else
    {
        p_engine->nMCUPerRow = ((p_engine->nImageWidth  + 7) / 8 + (uint32_t)(p_engine->maxHSample - 1))
                                / (uint32_t)(p_engine->maxHSample);
        p_engine->nMCUPerCol = ((p_engine->nImageHeight + 7) / 8 + (uint32_t)(p_engine->maxVSample - 1))
                                / (uint32_t)(p_engine->maxVSample);

        p_engine->nBlocksPerMCU = 0;
        for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++)
        {
            uint32_t num_blocks;

            // using color format information, we can get number of blocks
            // per MCU. we can assume that blocks in MCU is Y first
            // followed by cb and cr.
            comp_id = p_engine->compListInScan[i];
            num_blocks = (p_engine->hSample[comp_id] * p_engine->vSample[comp_id]);
            while (num_blocks--)
            {
                p_engine->MCUBlockCompList[p_engine->nBlocksPerMCU++] = comp_id;
            }
        }
    }
    p_engine->EOBRUN = 0;

    /**********************************************************************
     * check/make quatization table, huffman table and restart parameters
     * if any is missing (specially the derived huffman table), allocate
     * memory for it and make the table.
     *********************************************************************/
    rc = jpegd_engine_sw_progressive_check_qtable(p_engine);
    if (JPEG_FAILED(rc)) return rc;
    rc = jpegd_engine_sw_progressive_check_htable(p_engine);
    if (JPEG_FAILED(rc)) return rc;
    if (p_engine->nRestartInterval != 0)
    {
        p_engine->nRestartLeft    = p_engine->nRestartInterval;
        p_engine->nNextRestartNum = 0xD0;
    }

    JPEG_DBG_LOW("jpegd_engine_sw_progressive_get_next_scan: scan=%d, Ns=%d, Restart=%d\n",
                 p_engine->currentScanId, p_engine->nNumberOfComponentsInScan,
                 p_engine->nRestartInterval);
    JPEG_DBG_LOW("jpegd_engine_sw_progressive_get_next_scan: Ss=%d, Se=%d, Ah=%d, Al=%d\n",
                 p_engine->currentScanProgressiveInfo.ss, p_engine->currentScanProgressiveInfo.se,
                 p_engine->currentScanProgressiveInfo.ah, p_engine->currentScanProgressiveInfo.al);
    JPEG_DBG_LOW("jpegd_engine_sw_progressive_get_next_scan: componentId=%d\n",
                 p_engine->MCUBlockCompList[0]);

    return rc;
} /* end jpegd_engine_sw_progressive_get_next_scan() */

/*------------------------------------------------------------------------
 *                          Externalized Function Definitions
 * ----------------------------------------------------------------------- */

/**************************************************************************
FUNCTION        jpegd_engine_sw_configure_progressive
---------------------------------------------------------------------------
DESCRIPTION     initialize jpegd object, including set color space and
                format and progressive FIRST scan specific information.
---------------------------------------------------------------------------
INPUT  VALUE    none, but depend on decodeSpec
---------------------------------------------------------------------------
RETURN VALUE    result
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
int jpegd_engine_sw_configure_progressive
(
    jpegd_engine_sw_t *p_engine,
    uint32_t          *p_output_chunk_width,
    uint32_t          *p_output_chunk_height
)
{
    jpeg_frame_info_t *p_frame_info = p_engine->source.p_frame_info;
    jpeg_scan_info_t  *p_scan_info  = p_frame_info->pp_scan_infos[0];
    uint32_t           mcusPerRowInFrame, mcusPerColInFrame;
    uint32_t           rowsPerMCU, colsPerMCU;
    uint8_t            comp_id;
    int                rc = JPEGERR_SUCCESS;
    uint32_t           i;
    uint8_t            j;

    // Validation
    if (p_frame_info->num_scans > 1)
    {
        // This must be the FIRST scan. We do not know if there is any more
        // scans until we hit another SOS.
        return JPEGERR_EFAILED;
    }

    /**********************************************************************
     * First convert jpeg header related info in the p_frame_info into
     * local variables
     * 1. image size
     * 2. restart interval
     * 3. components in frame and FIRST scan
     * 4. set components info (id, sampling, quant table) in sof marker
     * 5. set components info (huff table) in sos (FIRST scan only)
     *    as well as spectrum selection.
     *********************************************************************/
    p_engine->nImageWidth                = p_frame_info->width;
    p_engine->nImageHeight               = p_frame_info->height;

    p_engine->nOutputWidth               = p_engine->dest.width;
    p_engine->nOutputHeight              = p_engine->dest.height;

    p_engine->nRestartInterval           = (int32_t)p_frame_info->restart_interval;
    p_engine->nNumberOfComponentsInFrame = p_frame_info->num_comps;

    // component id, sampling, quant info
    for (j = 0; j < p_engine->nNumberOfComponentsInFrame; j++)
    {
        p_engine->compId[j]  = p_frame_info->p_comp_infos[j].comp_id;
        p_engine->hSample[j] = p_frame_info->p_comp_infos[j].sampling_h;
        p_engine->vSample[j] = p_frame_info->p_comp_infos[j].sampling_v;
        p_engine->quantId[j] = p_frame_info->p_comp_infos[j].qtable_sel;
    }

    // progressive info for FIRST scan
    p_engine->currentScanId = 0;
    p_engine->nNumberOfComponentsInScan = p_scan_info->num_selectors;
    p_engine->currentScanProgressiveInfo.ss = (uint32_t)(p_scan_info->spec_start);
    p_engine->currentScanProgressiveInfo.se = (uint32_t)(p_scan_info->spec_end);
    p_engine->currentScanProgressiveInfo.al = (uint32_t)(p_scan_info->succ_approx_l);
    p_engine->currentScanProgressiveInfo.ah = (uint32_t)(p_scan_info->succ_approx_h);

    // component huffman info for FIRST scan
    for (j = 0; j < p_engine->nNumberOfComponentsInScan; j++)
    {
        // get component id
        comp_id = p_scan_info->p_selectors[j].comp_id;
        // dc table index
        p_engine->dcHuffTableId[comp_id] = p_scan_info->p_selectors[j].dc_selector;

        // since only one set of huff table index is used,
        // ac table is appened to dc table with index starting from
        // JPEGD_MAXHUFFTABLES/2.
        p_engine->acHuffTableId[comp_id] = p_scan_info->p_selectors[j].ac_selector
                                           + (uint8_t)(JPEGD_MAXHUFFTABLES >> 1);
        // get components list in a MCU in one scan
        p_engine->compListInScan[j] = comp_id;
    }

    /**********************************************************************
     * Here we check the color format. Also set up MCU related settings
     * 1. color format
     * 2. numbers of blocks in a MCU
     * 3. numbers of pixels in a row in one MCU
     * 4. numbers of pixels in a column in one MCU
     **********************************************************************/
    if (p_engine->nNumberOfComponentsInFrame == 1)
    {
        /******************************************************************
        * grey scale, only 1 (Y) 8x8 block per MCU
        ******************************************************************/
        p_engine->jpegdFormat = JPEG_GRAYSCALE;
        rowsPerMCU = JPEGD_DCTSIZE;
        colsPerMCU = JPEGD_DCTSIZE;
    }
    else if (p_engine->nNumberOfComponentsInFrame == 3)
    {
        if ((p_engine->hSample[1] != 1) || (p_engine->vSample[1] != 1) ||
            (p_engine->hSample[2] != 1) || (p_engine->vSample[2] != 1))
        {
            /**************************************************************
             * if none of these format, we can not decode this
             *************************************************************/
            JPEG_DBG_ERROR("jpegd_engine_sw_configure_progressive: "
                           "unsupported sampling format\n");
            rc = JPEGERR_EUNSUPPORTED;
        }
        if ((p_engine->hSample[0] == 1) && (p_engine->vSample[0] == 1))
        {
            /**************************************************************
             * H1V1, 3 (Y, Cb, Cr) blocks per MCU
             *************************************************************/
            p_engine->jpegdFormat = JPEG_H1V1;
            rowsPerMCU = JPEGD_DCTSIZE;
            colsPerMCU = JPEGD_DCTSIZE;
        }
        else if ((p_engine->hSample[0] == 2) && (p_engine->vSample[0] == 1))
        {
            /**************************************************************
             * H2V1, 4 (Yx2, Cb, Cr) blocks per MCU
             *************************************************************/
            p_engine->jpegdFormat = JPEG_H2V1;
            rowsPerMCU = JPEGD_DCTSIZE;
            colsPerMCU = JPEGD_DCTSIZE << 1;
        }
        else if ((p_engine->hSample[0] == 1) && (p_engine->vSample[0] == 2))
        {
            /**************************************************************
             * H1V2, 4 (Yx2, Cb, Cr) blocks per MCU
             *************************************************************/
            p_engine->jpegdFormat = JPEG_H1V2;
            rowsPerMCU = JPEGD_DCTSIZE << 1;
            colsPerMCU = JPEGD_DCTSIZE;
        }
        else if ((p_engine->hSample[0] == 2) && (p_engine->vSample[0] == 2))
        {
            /**************************************************************
             * H2V2, 6 (Yx4, Cb, Cr) blocks per MCU
             *************************************************************/
            p_engine->jpegdFormat = JPEG_H2V2;
            rowsPerMCU = JPEGD_DCTSIZE << 1;
            colsPerMCU = JPEGD_DCTSIZE << 1;
        }
        else
        {
            /**************************************************************
             * if none of these format, we can not decode this, never
             * should go here.
             *************************************************************/
            JPEG_DBG_ERROR("jpegd_engine_sw_configure_progressive: "
                           "unsupported sampling format, h: %d, v: %d\n",
                           p_engine->hSample[0], p_engine->vSample[0]);
            rc = JPEGERR_EUNSUPPORTED;
        } // end if (hSample[0] == 1 && vSample[0] == 1)
    }
    else
    {
        /******************************************************************
         * if none of these format, we can not decode this.
         *****************************************************************/
        JPEG_DBG_ERROR("jpegd_engine_sw_configure_progressive: "
                       "unsupported sampling formt\n");
        rc = JPEGERR_EUNSUPPORTED;
    } // end if (p_engine->nNumberOfComponentsInFrame == 1)

    // if format is not supported, exit.
    if (JPEG_FAILED(rc)) return rc;

    /**********************************************************************
     * here we find out how many lines and coloumns of MCUs in a component
     * 1. find out the maximum sampling factor for all components
     * 2. calculate number of blocks in verticle and horizontal direction
     *    for each component.
     **********************************************************************/
    // first figure out the maximum sample for rols and cols
    p_engine->maxHSample = p_engine->maxVSample = 1;
    for (j = 0; j < p_engine->nNumberOfComponentsInFrame; j++)
    {
        if (p_engine->hSample[j] > p_engine->maxHSample)
        {
            p_engine->maxHSample = p_engine->hSample[j];
        }
        if (p_engine->vSample[j] > p_engine->maxVSample)
        {
            p_engine->maxVSample = p_engine->vSample[j];
        }
    }
    /**********************************************************************
     * need to calculate the number of MCUs per row and col
     * This is determind by the color format and number of blocks in image
     * For each mcu, the type of block need to be labeled also
     *********************************************************************/
    if (p_engine->nNumberOfComponentsInFrame == 1)
    {
        // grey image
        comp_id = p_engine->compListInScan[0];
        mcusPerRowInFrame = ((7 + ((p_engine->nImageWidth * (uint32_t)(p_engine->hSample[comp_id]))
                                   + (uint32_t)(p_engine->maxHSample - 1))
                                  / ((uint32_t)p_engine->maxHSample)) / 8);
        mcusPerColInFrame = ((7 + ((p_engine->nImageHeight * (uint32_t)(p_engine->vSample[comp_id]))
                                   + (uint32_t)(p_engine->maxVSample - 1))
                                  / (uint32_t)(p_engine->maxVSample)) / 8);
        p_engine->compCoeffHBlocks[comp_id] = mcusPerRowInFrame;
        p_engine->compCoeffVBlocks[comp_id] = mcusPerColInFrame;
    }
    else
    {
        // color image
        mcusPerRowInFrame = ((p_engine->nImageWidth  + 7) / 8 + (uint32_t)(p_engine->maxHSample - 1))
                            / (uint32_t)(p_engine->maxHSample);
        mcusPerColInFrame = ((p_engine->nImageHeight + 7) / 8 + (uint32_t)(p_engine->maxVSample - 1))
                            / (uint32_t)(p_engine->maxVSample);
        // then use the component sample to determine the maximum number of blocks per row
        // and maximum number of blocks per col.
        //
        // for example, 322 x 226 in H2V2, if just calculate the blocks of Y is:
        // hBlocks = (322 + 7) / 8 = 41,
        // vBlocks = (226 + 7) / 8 = 29,
        // but if the scan has more than one component, like DC scan can have Y with
        // either Cb or Cr or all together, the data is encoded in MCU not in block,
        // in this case, Y would be encoded with:
        // (mcusPerRowInFrame * 2 = 42) blocks
        // (mcusPerColInFrame * 2 = 30) blocks
        for (j = 0; j < p_engine->nNumberOfComponentsInFrame; j++)
        {
            p_engine->compCoeffHBlocks[j] = (mcusPerRowInFrame * (uint32_t)(p_engine->hSample[j]));
            p_engine->compCoeffVBlocks[j] = (mcusPerColInFrame * (uint32_t)(p_engine->vSample[j]));
        }
    }

    /**********************************************************************
     * Calculate the Resize factor for IDCT according to input/output size
     * ratio
     *********************************************************************/
    p_engine->nResizeFactor = 3;
    while ((p_engine->nResizeFactor >= 0) &&
          ((p_engine->nOutputWidth  > (p_engine->nImageWidth >> p_engine->nResizeFactor)) ||
           (p_engine->nOutputHeight > (p_engine->nImageHeight >> p_engine->nResizeFactor))))

    {
        p_engine->nResizeFactor--;
    }
    if (p_engine->nResizeFactor < 0)
    {
        return JPEGERR_EFAILED;
    }
    rowsPerMCU >>= p_engine->nResizeFactor;
    colsPerMCU >>= p_engine->nResizeFactor;
    // set idct method
    switch (p_engine->nResizeFactor)
    {
        case 0:
        {
            p_engine->idct_func = &jpegd_engine_sw_idct_8x8;
            p_engine->numOfValidCoeff = 64;
            break;
        }
        case 1:
        {
            p_engine->idct_func = &jpegd_engine_sw_idct_4x4;
            p_engine->numOfValidCoeff = 25;
            break;
        }
        case 2:
        {
            p_engine->idct_func = &jpegd_engine_sw_idct_2x2;
            p_engine->numOfValidCoeff = 5;
            break;
        }
        case 3:
        {
            p_engine->idct_func = &jpegd_engine_sw_idct_1x1;
            p_engine->numOfValidCoeff = 1;
            break;
        }
        default:
        {
            return JPEGERR_EFAILED;
        }
    }

    /**********************************************************************
     * check/make quatization table, huffman table and restart parameters
     * if any is missing (specially the derived huffman table), allocate
     * memory for it and make the table.
     *********************************************************************/
    rc = jpegd_engine_sw_check_qtable(p_engine);
    if (JPEG_FAILED (rc)) return rc;
    rc = jpegd_engine_sw_check_htable(p_engine);
    if (JPEG_FAILED (rc)) return rc;
    if (p_engine->nRestartInterval != 0)
    {
        p_engine->nRestartLeft    = p_engine->nRestartInterval;
        p_engine->nNextRestartNum = 0xD0;
    }

    /**********************************************************************
     * Image is rounded up to multiple of blocks, but actual image size may
     * not, so calculate the row increment to ouput the actual size image.
     * get the read output dimension and y and cbcr row increament.
     * also determine the dimention of output mcu line buffer.
     *
     * also set which outputonemcu method to use according to format.
     *
     * The output size is changed proportionally to the input size, unlike
     * the output buffer size, which is rounded to multiple of cols per mcu
     *
     * this will be used for postprocess to unpad the image.
     *********************************************************************/
    switch (p_engine->jpegdFormat)
    {
        case JPEG_GRAYSCALE:
            p_engine->nYLinesPerMCU      = rowsPerMCU;
            p_engine->nCbCrLinesPerMCU   = 0;
            p_engine->nOutputRowIncrY    = colsPerMCU * mcusPerRowInFrame;
            p_engine->nOutputRowIncrCbCr = 0;
            p_engine->mcu_output_func    = &jpegd_engine_sw_output_gray_mcu;
            break;

        case JPEG_H1V1:
            p_engine->nYLinesPerMCU      = rowsPerMCU;
            p_engine->nCbCrLinesPerMCU   = rowsPerMCU;
            p_engine->nOutputRowIncrY    = colsPerMCU * mcusPerRowInFrame;
            p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY << 1;
            p_engine->mcu_output_func    = &jpegd_engine_sw_output_h1v1_mcu;
            break;

        case JPEG_H2V1:
            p_engine->nYLinesPerMCU      = rowsPerMCU;
            p_engine->nCbCrLinesPerMCU   = rowsPerMCU;
            p_engine->nOutputRowIncrY    = colsPerMCU * mcusPerRowInFrame;
            p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY;
            p_engine->mcu_output_func    = &jpegd_engine_sw_output_h2v1_mcu;
            break;

        case JPEG_H1V2:
            p_engine->nYLinesPerMCU      = rowsPerMCU;
            p_engine->nCbCrLinesPerMCU   = rowsPerMCU >> 1;
            p_engine->nOutputRowIncrY    = colsPerMCU * mcusPerRowInFrame;
            p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY << 1;
            p_engine->mcu_output_func    = &jpegd_engine_sw_output_h1v2_mcu;
            break;

        case JPEG_H2V2:
            p_engine->nYLinesPerMCU      = rowsPerMCU;
            p_engine->nCbCrLinesPerMCU   = rowsPerMCU >> 1;
            p_engine->nOutputRowIncrY    = colsPerMCU * mcusPerRowInFrame;
            p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY;
            p_engine->mcu_output_func    = &jpegd_engine_sw_output_h2v2_mcu;
            break;

        default:
            return JPEGERR_EFAILED;
    }

    // set output dimension
    p_engine->nOutputWidth       = (uint32_t)(p_engine->nImageWidth >> p_engine->nResizeFactor);
    p_engine->nOutputHeight      = (uint32_t)(p_engine->nImageHeight >> p_engine->nResizeFactor);

    p_engine->nOutputBufferSizeY    = p_engine->nOutputRowIncrY * p_engine->nYLinesPerMCU;
    p_engine->nOutputBufferSizeCbCr = p_engine->nOutputRowIncrCbCr * p_engine->nCbCrLinesPerMCU;

    /**********************************************************************
     * allocate component coefficient buffers before idct.
     *********************************************************************/
    for (j = 0; j < p_engine->nNumberOfComponentsInFrame; j++)
    {
        const int32_t size = (int32_t)((p_engine->compCoeffHBlocks[j] * p_engine->compCoeffVBlocks[j])
                                       * JPEGD_DCTSIZE2 * sizeof(jpegd_coeff_t));
        JPEG_FREE(p_engine->pCompCoeffBuf[j]);

        p_engine->pCompCoeffBuf[j] = (jpegd_coeff_t *)JPEG_MALLOC(size);
        if (!p_engine->pCompCoeffBuf[j]) return JPEGERR_EMALLOC;

        // Clear the coefficient buffer
        (void)STD_MEMSET(p_engine->pCompCoeffBuf[j], 0, size);
    }
    /**********************************************************************
     * allocate sample buffers after idct.
     *********************************************************************/
    for (i = 0; i < JPEGD_MAXBLOCKSPERMCU; i++)
    {
        if (!p_engine->pSampleMCUBuf[i])
        {
            p_engine->pSampleMCUBuf[i] =
                (jpegd_sample_t *)JPEG_MALLOC((JPEGD_DCTSIZE2) * sizeof(jpegd_sample_t));
            if (!p_engine->pSampleMCUBuf[i]) return JPEGERR_EMALLOC;
        }
    }

    /**********************************************************************
     * initialize the MCU info for FIRST scan
     *********************************************************************/
    if (p_engine->nNumberOfComponentsInScan == 1)
    {
        comp_id = p_engine->compListInScan[0];
        p_engine->nMCUPerRow = ((7 + ((p_engine->nImageWidth * (uint32_t)(p_engine->hSample[comp_id]))
                                      + (uint32_t)(p_engine->maxHSample - 1))
                                     / (uint32_t)(p_engine->maxHSample)) / 8);
        p_engine->nMCUPerCol = ((7 + ((p_engine->nImageHeight * (uint32_t)(p_engine->vSample[comp_id]))
                                      + (uint32_t)(p_engine->maxVSample - 1))
                                     / (uint32_t)(p_engine->maxVSample)) / 8);
        p_engine->nBlocksPerMCU = 1;
        p_engine->MCUBlockCompList[0] = comp_id;
    }
    else
    {
        p_engine->nMCUPerRow = ((p_engine->nImageWidth  + 7) / 8 + (uint32_t)(p_engine->maxHSample - 1))
                               / (uint32_t)(p_engine->maxHSample);
        p_engine->nMCUPerCol = ((p_engine->nImageHeight + 7) / 8 + (uint32_t)(p_engine->maxVSample - 1))
                               / (uint32_t)(p_engine->maxVSample);

        p_engine->nBlocksPerMCU = 0;
        for (j = 0; j < p_engine->nNumberOfComponentsInScan; j++)
        {
            uint32_t num_blocks;

            // using color format information, we can get number of blocks
            // per MCU. we can assume that blocks in MCU is Y first
            // followed by cb and cr.
            comp_id = p_engine->compListInScan[j];
            num_blocks = (p_engine->hSample[comp_id] * p_engine->vSample[comp_id]);
            while (num_blocks--)
            {
                p_engine->MCUBlockCompList[p_engine->nBlocksPerMCU++] = comp_id;
            }
        }
    }
    p_engine->EOBRUN = 0;

    JPEG_DBG_LOW("jpegd_engine_sw_configure_progressive: scan=%d, Ns=%d, Restart=%d\n",
                 p_engine->currentScanId, p_engine->nNumberOfComponentsInScan, p_engine->nRestartInterval);
    JPEG_DBG_LOW("jpegd_engine_sw_configure_progressive: Ss=%d, Se=%d, Ah=%d, Al=%d\n",
                 p_engine->currentScanProgressiveInfo.ss, p_engine->currentScanProgressiveInfo.se,
                 p_engine->currentScanProgressiveInfo.ah, p_engine->currentScanProgressiveInfo.al);

    return rc;
} /* end jpegd_engine_sw_configure_progressive() */

/**************************************************************************
FUNCTION        jpegd_engine_sw_decode_progressive
---------------------------------------------------------------------------
DESCRIPTION     Decode Scans of Progressive encoded image
---------------------------------------------------------------------------
INPUT  VALUE    none, but depend on decodeSpec
---------------------------------------------------------------------------
RETURN VALUE    none
---------------------------------------------------------------------------
NOTES           none
**************************************************************************/
jpeg_event_t jpegd_engine_sw_decode_progressive(jpegd_engine_sw_t *p_engine)
{
    uint32_t  nNumberOfMCUsInRow;
    uint32_t  nMCUPerRowInFrame;
    uint32_t  nMCUPerColInFrame;
    uint32_t  nNumberOfMCUsInFrame;
    uint32_t  nNumberOfMCULinesInFrame;
    uint8_t   comp_id;
    uint32_t  block, ii, jj;
    int       rc;

    if (p_engine->abort_flag)
    {
        JPEG_DBG_HIGH("jpegd_engine_sw_decode_progressive: aborted\n");
        return JPEG_EVENT_ABORTED;
    }

    /**********************************************************************
     * first is to make sure input and output pointer are valid and ready
     *********************************************************************/
    p_engine->input_fetcher.next_byte_to_fetch_offset =
        p_engine->source.p_frame_info->pp_scan_infos[0]->offset;

    /**********************************************************************
     * then load up the input buffer and bit buffer
     *********************************************************************/
    rc = jpegd_engine_sw_init_bit_buffer(p_engine);
    if (JPEG_FAILED(rc)) return JPEG_EVENT_ERROR;

    /**********************************************************************
     * Now start decode bitstream.
     *********************************************************************/
    // Loop until all scans
    for (; ;)
    {
        uint8_t   noMoreScans             = 0;
        int32_t   nSkippedMCUs            = 0;
        uint32_t  nNumberOfMCULinesInScan = 0;
        uint32_t  nNumberOfMCUsInScan     = p_engine->nMCUPerRow * p_engine->nMCUPerCol;

        /**********************************************************************
         * Set up coefficient buffer pointers for the scan
         *********************************************************************/
        (void)STD_MEMSET(p_engine->pMCUCoeffBlockBuf, 0, sizeof(p_engine->pMCUCoeffBlockBuf));
        if (p_engine->nNumberOfComponentsInScan == 1)
        {
            // One block per MCU
            p_engine->pMCUCoeffBlockBuf[0] = p_engine->pCompCoeffBuf[p_engine->MCUBlockCompList[0]];
        }
        else
        {
            int32_t  i, j, k;
            uint32_t mcu_blocks = 0;
            for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++)
            {
                // using color format information, we can get number of blocks
                // per MCU. we can assume that blocks in MCU is Y first
                // followed by cb and cr.
                comp_id = p_engine->compListInScan[i];

                for (j = 0; j < p_engine->vSample[comp_id]; j++)
                    for (k = 0; k < p_engine->hSample[comp_id]; k++)
                    {
                        p_engine->pMCUCoeffBlockBuf[mcu_blocks++] =
                            p_engine->pCompCoeffBuf[comp_id]
                            + ((j * ((int32_t)p_engine->compCoeffHBlocks[comp_id]) + k)
                               * JPEGD_DCTSIZE2);
                    }
            }
        }

        // first make sure dc value is cleared
        (void)STD_MEMSET(p_engine->lastDcVal, 0, sizeof(p_engine->lastDcVal));
        p_engine->EOBRUN = 0;

        /**********************************************************************
         * Now start decode bitstream for the scan.
         *********************************************************************/
        nNumberOfMCUsInRow = 0;

        // Loop until all MCUs in the scan
        while (nNumberOfMCUsInScan > 0)
        {
            /******************************************************************
             * Check abort flag. If abort flag set, do:
             * 1. print debug mesage
             * 2. raise engine aborted event to engine user
             * 3. clean up input buffer
             * 4. reset abort flag to false
             * 5. exit decode thead.
             *****************************************************************/
            if (p_engine->abort_flag)
            {
                JPEG_DBG_HIGH("jpegd_engine_sw_decode_progressive: aborted\n");
                return JPEG_EVENT_ABORTED;
            }

            if (nSkippedMCUs == 0) /* if no MCUs are to be skipped */
            {
                // for each MCU, check if restart marker is expected.
                if ((p_engine->nRestartInterval != 0) && (p_engine->nRestartLeft == 0))
                {
                    /**********************************************************
                     * If restart marker is expected,
                     * process restart marker first
                     * 1. call process_restart_marker: this function will
                     *    search for restart marker
                     * 1.a if correct restart marker is found, then do nothing
                     *     and skipped MCU count will be 0.
                     * 1.b if correct restart marker is not found, calculate
                     *     how many MCUs need to be skipped.
                     * 2. clear dc residual
                     * 3. reset restart left to restart interval, start a fresh
                     *    restart interval.
                     * 4. reset EOBRUN to 0
                     *********************************************************/
                    rc = jpegd_engine_sw_process_restart_marker(p_engine, &nSkippedMCUs);
                    if (JPEG_FAILED(rc)) return JPEG_EVENT_ERROR;

                    (void)STD_MEMSET(p_engine->lastDcVal, 0, sizeof(p_engine->lastDcVal));
                    p_engine->nRestartLeft = p_engine->nRestartInterval;
                    p_engine->EOBRUN       = 0;

                    // continue to the next MCU, the MCU coefficient block pointers
                    // are already adjusted.
                    continue;
                }
                else
                {
                    /**********************************************************
                     * if this is not a restarted marker location
                     * we do normal decode of 1 MCU
                     * 1. call dehuff_one_mcu: this will huff decode 1 MCU
                     * 2. reduce restart counter by 1.
                     * 3. reduce total MCU to decode count by 1.
                     * 4. increase current row MCU count by 1.
                     *********************************************************/
                    // Regardless if there is something wrong or not in dehuff one MCU,
                    // continue until all MCUs are done before the next restart marker.
                    (void)jpegd_engine_sw_progressive_dehuff_one_mcu(p_engine);

                    p_engine->nRestartLeft--;
                    nNumberOfMCUsInScan--;
                    nNumberOfMCUsInRow++;
                }
            }
            else // (nSkippedMCUs != 0)
            {
                uint32_t i;
                /**************************************************************
                 * if here, some MCUs need to be skipped
                 * each time we skip one MCU until it is all skipped
                 * when MCU is skipped we do:
                 * 1. writes 0 to MCU coefficient blocks
                 * 2. reduce skipped MCU count
                 * 3. reduce number of MCUs to be decoded count
                 * 4. increase number of MCUs in current line count
                 *************************************************************/
                // zero out the corresponding coefficient blcoks in MCU
                for (i = 0; i < p_engine->nBlocksPerMCU; i++)
                {
                    (void)STD_MEMSET(p_engine->pMCUCoeffBlockBuf[i], 0,
                                     (sizeof(jpegd_coeff_t) * JPEGD_DCTSIZE2));
                }

                nSkippedMCUs--;
                nNumberOfMCUsInScan--;
                nNumberOfMCUsInRow++;
            }

            // now check if one MCU line is been decoded
            if (nNumberOfMCUsInRow == p_engine->nMCUPerRow)
            {
                // increment the # of MCU lines decoded so far
                nNumberOfMCULinesInScan++;

                // adjust MCU coefficient block pointers to the next MCU line
                if (p_engine->nNumberOfComponentsInScan == 1)
                {
                    comp_id = p_engine->compListInScan[0];
                    p_engine->pMCUCoeffBlockBuf[0] = p_engine->pCompCoeffBuf[comp_id] +
                        (nNumberOfMCULinesInScan * p_engine->compCoeffHBlocks[comp_id] * JPEGD_DCTSIZE2);
                }
                else
                {
                    int32_t  i, j, k;
                    uint32_t mcu_blocks = 0;
                    for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++)
                    {
                        comp_id = p_engine->compListInScan[i];

                        for (j = 0; j < p_engine->vSample[comp_id]; j++)
                            for (k = 0; k < p_engine->hSample[comp_id]; k++)
                            {
                                p_engine->pMCUCoeffBlockBuf[mcu_blocks++] =
                                    p_engine->pCompCoeffBuf[comp_id]
                                    + (((((int32_t)(nNumberOfMCULinesInScan)
                                          * (int32_t)(p_engine->vSample[comp_id])) + j)
                                        * (int32_t)(p_engine->compCoeffHBlocks[comp_id])) + k)
                                    * JPEGD_DCTSIZE2;
                            }
                    }
                }

                // reset mcu row count
                nNumberOfMCUsInRow = 0;
            }
            else // (nNumberOfMCUsInRow != p_engine->nMCUPerRow)
            {
                // move to the next MCU coefficient block pointers in the same MCU line
                if (p_engine->nNumberOfComponentsInScan == 1)
                {
                    // One block per MCU - move to the next block in the same MCU line
                    p_engine->pMCUCoeffBlockBuf[0] += JPEGD_DCTSIZE2;
                }
                else
                {
                    uint8_t  i, j, k;
                    uint32_t mcu_blocks = 0;
                    for (i = 0; i < p_engine->nNumberOfComponentsInScan; i++)
                    {
                        comp_id = p_engine->compListInScan[i];

                        for (j = 0; j < p_engine->vSample[comp_id]; j++)
                            for (k = 0; k < p_engine->hSample[comp_id]; k++)
                            {
                                p_engine->pMCUCoeffBlockBuf[mcu_blocks++] +=
                                  ((int32_t)(p_engine->hSample[comp_id]) * JPEGD_DCTSIZE2);
                            }
                    }
                }
            }

            // now check if an input error has happened, if so exit out
            // otherwise continue with decode.
            if (p_engine->fInputError == 1) return JPEG_EVENT_ERROR;
        } // end while (nNumberOfMCUsInScan > 0)

        // move to the next scan
        rc = jpegd_engine_sw_progressive_process_sos_marker(p_engine, &noMoreScans);
        if (JPEG_FAILED (rc)) return JPEG_EVENT_ERROR;
        if (noMoreScans == 0)
        {
            rc = jpegd_engine_sw_progressive_get_next_scan(p_engine);
            if (JPEG_FAILED (rc)) return JPEG_EVENT_ERROR;
        }
        else
        {
            break;
        }
    } // end for (; ;) scan loop

    // dequant for all blocks
    for (comp_id = 0; comp_id < p_engine->nNumberOfComponentsInFrame; comp_id++)
    {
        jpegd_coeff_t *p_comp_coeff_block = p_engine->pCompCoeffBuf[comp_id];

        for (ii = 0; ii < p_engine->compCoeffVBlocks[comp_id]; ii++)
        {
            for (jj = 0; jj < p_engine->compCoeffHBlocks[comp_id]; jj++)
            {
                jpegd_engine_sw_progressive_dequant(
                    p_engine->pDeQuantTable[p_engine->quantId[comp_id]], p_comp_coeff_block);
                p_comp_coeff_block += JPEGD_DCTSIZE2;
            }
        }
    }

    // set output - MCU line buffer pointers
    p_engine->pOutputBufferY = p_engine->dest.p_luma_buffers[0]->ptr;
    p_engine->pOutputBufferCbCr = p_engine->dest.p_chroma_buffers[0]->ptr;
    p_engine->curr_dest_buf_index = 0;

    p_engine->pMCULineBufPtrY    = p_engine->pOutputBufferY;
    p_engine->pMCULineBufPtrCbCr = p_engine->pOutputBufferCbCr;

    // set up coefficient block pointers in the way that m_pSampleMCUBuf wants
    (void)STD_MEMSET(p_engine->pMCUCoeffBlockBuf, 0, sizeof(p_engine->pMCUCoeffBlockBuf));
    if (p_engine->nNumberOfComponentsInFrame > 1)
    {
        block = 0;
        for (comp_id = 0; comp_id < p_engine->nNumberOfComponentsInFrame; comp_id++)
        {
            int32_t j, k;
            for (j = 0; j < p_engine->vSample[comp_id]; j++)
                for (k = 0; k < p_engine->hSample[comp_id]; k++)
                {
                    p_engine->pMCUCoeffBlockBuf[block++] =
                        (p_engine->pCompCoeffBuf[comp_id] +
                         (j * (int32_t)(p_engine->compCoeffHBlocks[comp_id]) + k) * JPEGD_DCTSIZE2);
                }
        }
    }

    // calculate number of MCUs in the frame
    if (p_engine->nNumberOfComponentsInFrame == 1)
    {
        comp_id = p_engine->compListInScan[0];
        nMCUPerRowInFrame = ((7 + ((p_engine->nImageWidth * (uint32_t)(p_engine->hSample[comp_id]))
                                   + (uint32_t)(p_engine->maxHSample - 1))
                                  / (uint32_t)(p_engine->maxHSample)) / 8);
        nMCUPerColInFrame = ((7 + ((p_engine->nImageHeight * (uint32_t)(p_engine->vSample[comp_id]))
                                   + (uint32_t)(p_engine->maxVSample - 1))
                                  / (uint32_t)(p_engine->maxVSample)) / 8);
    }
    else
    {
        nMCUPerRowInFrame = (((p_engine->nImageWidth  + 7) / 8 + (uint32_t)(p_engine->maxHSample - 1))
                             / (uint32_t)(p_engine->maxHSample));
        nMCUPerColInFrame = (((p_engine->nImageHeight + 7) / 8 + (uint32_t)(p_engine->maxVSample - 1))
                             / (uint32_t)(p_engine->maxVSample));
    }

    nNumberOfMCUsInRow = 0;
    nNumberOfMCUsInFrame = nMCUPerRowInFrame * nMCUPerColInFrame;
    nNumberOfMCULinesInFrame = 0;

    // Loop until all MCUs in frame
    while (nNumberOfMCUsInFrame > 0)
    {
        uint32_t mcu_blocks = 0;
        for (comp_id = 0; comp_id < p_engine->nNumberOfComponentsInFrame; comp_id++)
        {
            uint8_t j, k;

            // idct then output
            for (j = 0; j < p_engine->vSample[comp_id]; j++)
                for (k = 0; k < p_engine->hSample[comp_id]; k++)
                {
                    p_engine->idct_func(p_engine->pMCUCoeffBlockBuf[mcu_blocks],
                                        p_engine->pSampleMCUBuf[mcu_blocks],
                                        JPEGD_DCTSIZE * sizeof(jpegd_coeff_t));

                    // move to the next MCU coefficient block pointers
                    p_engine->pMCUCoeffBlockBuf[mcu_blocks++] +=
                        ((int32_t)(p_engine->hSample[comp_id]) * JPEGD_DCTSIZE2);
                }
        }
        p_engine->mcu_output_func(p_engine);

        nNumberOfMCUsInFrame--;
        nNumberOfMCUsInRow++;

        // now check if one MCU line is been decoded
        // if yes, write out that one line of MCUs to external buffer.
        if (nNumberOfMCUsInRow == nMCUPerRowInFrame)
        {
            /**************************************************************
             * If we are at the end of MCU line, Do write the current
             * decoded MCU line to external storage
             * 1. send MCU line buffer to engine user
             * 2. release buffer
             * 3. request new buffer for next MCU lines
             * 4. get output buffer and buffer pointer
             * 5. reset internal MCU row buffer pointer to the beginning of
             *    the buffer
             * 6. reset the MCU row count
             *************************************************************/
            // send output to engine user
            rc = p_engine->dest.p_output_handler(p_engine->p_wrapper,
                                            p_engine->dest.p_luma_buffers[p_engine->curr_dest_buf_index],
                                            p_engine->dest.p_chroma_buffers[p_engine->curr_dest_buf_index]);

            // if something goes wrong when sending the output, bail out from the loop and return event error.
            if (JPEG_FAILED(rc))
            {
                return JPEG_EVENT_ERROR;
            }

            // switch to use the next set of buffer
            p_engine->curr_dest_buf_index = (p_engine->curr_dest_buf_index + 1) & 0x1;

            // wait for the now current set of buffer to be ready
            jpeg_buffer_wait_until_empty(p_engine->dest.p_luma_buffers[p_engine->curr_dest_buf_index]);
            jpeg_buffer_wait_until_empty(p_engine->dest.p_chroma_buffers[p_engine->curr_dest_buf_index]);

            // reload the new buffer addresses
            p_engine->pOutputBufferY    = p_engine->dest.p_luma_buffers[p_engine->curr_dest_buf_index]->ptr;
            p_engine->pOutputBufferCbCr = p_engine->dest.p_chroma_buffers[p_engine->curr_dest_buf_index]->ptr;

            // reset MCU row buffer to beginning of output buffer
            p_engine->pMCULineBufPtrY    = p_engine->pOutputBufferY;
            p_engine->pMCULineBufPtrCbCr = p_engine->pOutputBufferCbCr;

            // reset mcu row count
            nNumberOfMCUsInRow = 0;

            // increment the # of MCU lines output so far
            nNumberOfMCULinesInFrame++;

            // adjust MCU coefficient block pointers to the next MCU line
            if (p_engine->nNumberOfComponentsInFrame == 1)
            {
                // One block per MCU - move to the next block
                p_engine->pMCUCoeffBlockBuf[0] += JPEGD_DCTSIZE2;
            }
            else
            {
                int32_t  j, k;
                uint32_t mcu_blocks = 0;
                for (comp_id = 0; comp_id < p_engine->nNumberOfComponentsInFrame; comp_id++)
                {
                    for (j = 0; j < p_engine->vSample[comp_id]; j++)
                        for (k = 0; k < p_engine->hSample[comp_id]; k++)
                        {
                            p_engine->pMCUCoeffBlockBuf[mcu_blocks++] =
                                p_engine->pCompCoeffBuf[comp_id]
                                + (((((int32_t)nNumberOfMCULinesInFrame
                                      * (int32_t)p_engine->vSample[comp_id]) + j)
                                    * (int32_t)p_engine->compCoeffHBlocks[comp_id]) + k)
                                * JPEGD_DCTSIZE2;
                        }
                }
            }
        } // end if (nNumberOfMCUsInRow == nMCUPerRowInFrame)
    } // end while (nNumberOfMCUsInFrame > 0)

    // If this point is reached, decoding is done successfully
    return JPEG_EVENT_DONE;
} /* end jpegd_engine_sw_decode_progressive() */

