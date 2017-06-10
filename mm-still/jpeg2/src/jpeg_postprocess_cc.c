/*========================================================================


*//** @file jpeg_postprocess_cc.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
09/21/09   mingy   Added region based decoding and odd sized image decoding
                   support. Ported Chroma upsampling feature.
                   Changed the name of the postt process function
                   parameter structure.
04/22/09   mingy   Created file.
                   Branch out color convert from jpeg_postprocessor.c.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_postprocessor.h"

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
** Chroma upsampling functions
*************************************************************************/

/**************************************************************************
 * Interstitial(off-site) Upsampling H1V2 line to H1V1 line (Vertically)
 *************************************************************************/
void y1vu2upy1vu1line(uint8_t * pChromaLineNear,
                      uint8_t * pChromaLineFar,
                      uint8_t * pChromaUpOutput,
                      uint32_t nLineWidth)
{
    uint32_t i;

    for (i = 0; i < nLineWidth; i++)
    {
        /******************************************************************
         * output_cr=( 3* input_cr_near + 1* input_cr_far +2 ) /4
         * output_cb=( 3* input_cb_near + 1* input_cb_far +2 ) /4
         *****************************************************************/
        // Cb
        *pChromaUpOutput     = (uint8_t)((3*(*pChromaLineNear)     + *pChromaLineFar     + 2) >> 2);
        // Cr
        *(pChromaUpOutput+1) = (uint8_t)((3*(*(pChromaLineNear+1)) + *(pChromaLineFar+1) + 2) >> 2);

        // update pointers
        pChromaLineNear += 2;
        pChromaLineFar  += 2;
        pChromaUpOutput += 2;
    }
}


/**************************************************************************
 * Interstitial(off-site) Upsample Chroma line horizontally(H2 -> H1).
 *************************************************************************/
void y2vu1upy1vu1line(uint8_t * pChromaLine,
                      uint8_t * pChromaUpOutput,
                      uint32_t nLineWidth)
{
    uint32_t i;

    // The first CrCb pair is the same to the original values
    *pChromaUpOutput       = *pChromaLine;
    *(pChromaUpOutput + 1) = *(pChromaLine +1);

    /**********************************************************************
     * output_cr(i, 2j)=( 3* input_cr(i,j) + 1* input_cr(i,j+1) +2 ) /4
     * output_cb(i, 2j)=( 3* input_cb(i,j) + 1* input_cb(i,j+1) +2 ) /4
     *********************************************************************/
    // The second CrCb pair is computed based on the values of
    // both the first and second original CrCb pair
    *(pChromaUpOutput+2) = (uint8_t)((3*(*pChromaLine)     + *(pChromaLine+2) + 2) >> 2);
    *(pChromaUpOutput+3) = (uint8_t)((3*(*(pChromaLine+1)) + *(pChromaLine+3) + 2) >> 2);

    pChromaLine     += 2;
    pChromaUpOutput += 4;

    for (i = 2; i < nLineWidth-2; i += 2)
    {
        /******************************************************************
         * output_cr(i, 2j)=( 3* input_cr(i,j) + 1* input_cr(i,j-1) +2 ) /4
         * output_cb(i, 2j)=( 3* input_cb(i,j) + 1* input_cb(i,j-1) +2 ) /4
         *****************************************************************/
        *pChromaUpOutput     = (uint8_t)((3*(*pChromaLine)     + *(pChromaLine-2) + 2) >> 2);
        *(pChromaUpOutput+1) = (uint8_t)((3*(*(pChromaLine+1)) + *(pChromaLine-1) + 2) >> 2);

        /******************************************************************
         * output_cb(i, 2j)=( 3* input_cb(i,j) + 1* input_cb(i,j+1) +2 ) /4
         * output_cr(i, 2j)=( 3* input_cr(i,j) + 1* input_cr(i,j+1) +2 ) /4
         *****************************************************************/
        *(pChromaUpOutput+2) = (uint8_t)((3*(*pChromaLine)     + *(pChromaLine+2)+ 2) >> 2);
        *(pChromaUpOutput+3) = (uint8_t)((3*(*(pChromaLine+1)) + *(pChromaLine+3)+ 2) >> 2);

        // Update pointers
        pChromaLine     += 2;
        pChromaUpOutput += 4;
    }

    /**********************************************************************
     * output_cr(i, 2j)=( 3* input_cr(i,j) + 1* input_cr(i,j-1) +2 ) /4
     * output_cb(i, 2j)=( 3* input_cb(i,j) + 1* input_cb(i,j-1) +2 ) /4
     *********************************************************************/
    *pChromaUpOutput     = (uint8_t)((3*(*pChromaLine)     + *(pChromaLine-2) + 2) >> 2);
    *(pChromaUpOutput+1) = (uint8_t)((3*(*(pChromaLine+1)) + *(pChromaLine-1) + 2) >> 2);

    /***********************************************************************
    * Since the above for loop executes one more time in case of
    * odd nLineWidth, the CrCb for the very last Y (odd) is already obtained
    * by the above two statements.
    * Execute the following statements only when nLineWidth is even.
    ************************************************************************/
    if (!(nLineWidth & 0x01))
    {
        *(pChromaUpOutput +2) = *pChromaLine;
        *(pChromaUpOutput +3) = *(pChromaLine +1);
    }
}

/**************************************************************************
 * Interstitial(off-site) Upsample Chroma line both horizontally and
 * vertically with an additional neighboring Chroma lines.
 *************************************************************************/
void y2vu2upy1vu1line(uint8_t * pChromaLineNear,
                      uint8_t * pChromaLineFar,
                      uint8_t * pChromaUpOutput,
                      uint32_t nLineWidth)
{
    uint32_t i;

    /**********************************************************************
     * output_cr=( 3* input_cr_near + 1* input_cr_far +2 ) /4
     * output_cb=( 3* input_cb_near + 1* input_cb_far +2 ) /4
     *********************************************************************/
    *pChromaUpOutput     = (uint8_t)((3*(*pChromaLineNear)     + *pChromaLineFar     + 2) >> 2);
    *(pChromaUpOutput+1) = (uint8_t)((3*(*(pChromaLineNear+1)) + *(pChromaLineFar+1) + 2) >> 2);

    *(pChromaUpOutput+2) =(uint8_t)((9*(*pChromaLineNear)     + 3*(*(pChromaLineFar))   + 3*(*(pChromaLineNear+2)) + *(pChromaLineFar+2) + 8) >> 4);
    *(pChromaUpOutput+3) =(uint8_t)((9*(*(pChromaLineNear+1)) + 3*(*(pChromaLineFar+1)) + 3*(*(pChromaLineNear+3)) + *(pChromaLineFar+3) + 8) >> 4);

    pChromaLineNear += 2;
    pChromaLineFar  += 2;
    pChromaUpOutput += 4;

    /**************************************************************************
    * Notice that if nLineWidth is odd, the for loop will execute one more time
    * than when nLineWidth is even.
    ***************************************************************************/
    for (i=2; i< nLineWidth-2; i+=2)
    {
        *pChromaUpOutput     = (uint8_t)((9*(*pChromaLineNear)     + 3*(*(pChromaLineFar))   + 3*(*(pChromaLineNear-2)) + *(pChromaLineFar-2) + 8) >> 4);
        *(pChromaUpOutput+1) = (uint8_t)((9*(*(pChromaLineNear+1)) + 3*(*(pChromaLineFar+1)) + 3*(*(pChromaLineNear-1)) + *(pChromaLineFar-1) + 8) >> 4);

        *(pChromaUpOutput+2) = (uint8_t)((9*(*pChromaLineNear)     + 3*(*(pChromaLineFar))   + 3*(*(pChromaLineNear+2)) + *(pChromaLineFar+2) + 8) >> 4);
        *(pChromaUpOutput+3) = (uint8_t)((9*(*(pChromaLineNear+1)) + 3*(*(pChromaLineFar+1)) + 3*(*(pChromaLineNear+3)) + *(pChromaLineFar+3) + 8) >> 4);

        // update pointers
        pChromaLineNear += 2;
        pChromaLineFar  += 2;
        pChromaUpOutput += 4;
    }

    *pChromaUpOutput     = (uint8_t)((9*(*pChromaLineNear)     + 3*(*pChromaLineFar)     + 3*(*(pChromaLineNear-2)) + *(pChromaLineFar-2) + 8) >> 4);
    *(pChromaUpOutput+1) = (uint8_t)((9*(*(pChromaLineNear+1)) + 3*(*(pChromaLineFar+1)) + 3*(*(pChromaLineNear-1)) + *(pChromaLineFar-1) + 8) >> 4);

    /***********************************************************************
    * Since the above for loop executes one more time in case of
    * odd nLineWidth, the CrCb for the very last Y (odd) is already obtained
    * by the above two statements.
    * Execute the following statements only when nLineWidth is even.
    ************************************************************************/
    if (!(nLineWidth & 0x01))
    {
        *(pChromaUpOutput+2) = (uint8_t)((3*(*pChromaLineNear)     + *pChromaLineFar     + 2) >> 2);
        *(pChromaUpOutput+3) = (uint8_t)((3*(*(pChromaLineNear+1)) + *(pChromaLineFar+1) + 2) >> 2);
    }
}


/**************************************************************************
*  COLOR CONVERT will convert YUV to RGB and remove padding
*************************************************************************/
void color_convert(jpegd_postprocess_func_param_t *postprocess)
{

    uint32_t y; //Loop counter

    switch (postprocess->p_processor->color_convert)
    {
    case H2V2_TO_RGB565:

        /*****************************************************************
         * Upsampling H2V1 to H1V1 Line
         ****************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
         * Color Conversion YUV to RGB Line
         ****************************************************************/
        y1vu2rgb565line(postprocess->pSrcLuma,
                        postprocess->p_processor->pChromaSmpOutput,
                        postprocess->pDstRGB,
                        postprocess->nColsToProcess);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += postprocess->p_processor->nStride;

        /*****************************************************************
         * Upsampling H2V2 to H1V1 Line
         *
         * Notice that for this H2V2 case, if there are only 2 Y lines
         * to process, there will be only 1 CbCr line, and averaging
         * 2 chroma lines is meaningless.
         * So only when Y lines are greater than 2 shall it averages
         * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        * i.e. only color converting the second Chroma line if there are
        * at least 2 Luma lines.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

        postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
        postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
        for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
        {
            /*************************************************************
             * Upsampling H2V2 to H1V1
             ************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
             * Color Conversion YUV to RGB Line
             ************************************************************/
            y1vu2rgb565line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB  += postprocess->p_processor->nStride;

            /*************************************************************
             * Upsampling H2V2 to H1V1
             ************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
             * Color Conversion YUV to RGB Line
             ************************************************************/
            y1vu2rgb565line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        /*****************************************************************
         * Upsampling H2V2 to H1V1
         ****************************************************************/
        y2vu2upy1vu1line(postprocess->pSrcChroma,
                         postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
         * Color Conversion YUV to RGB Line
         ****************************************************************/
        y1vu2rgb565line(postprocess->pSrcLuma,
                        postprocess->p_processor->pChromaSmpOutput,
                        postprocess->pDstRGB,
                        postprocess->nColsToProcess);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB += postprocess->p_processor->nStride;

                /*************************************************************
                 * Upsampling H2V1 to H1V1 Line
                 ************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb565line(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess);

                postprocess->pDstRGB += postprocess->p_processor->nStride;
            }
        }

        break;


    case H2V2_TO_RGB888:

        /*****************************************************************
        * Upsample Chroma line horizontally(H2 -> H1).
         ****************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
        * Color Convert Non-Chroma sub-sampled YUV Line to RGB Line
         ****************************************************************/
        y1vu2rgb888line(postprocess->pSrcLuma,
                        postprocess->p_processor->pChromaSmpOutput,
                        postprocess->pDstRGB,
                        postprocess->nColsToProcess);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += postprocess->p_processor->nStride;

        /*****************************************************************
        * Upsampling H2V2 to H1V1
        *
        * Notice that for this H2V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 Chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            y1vu2rgb888line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                 * Upsampling H2V2 to H1V1
                 ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgb888line(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += postprocess->p_processor->nStride;

                /*************************************************************
                 * Upsampling H2V2 to H1V1
                 ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgb888line(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    += postprocess->p_processor->nStride;
            }

            /*****************************************************************
             * Upsampling H2V2 to H1V1
             ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgb888line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB += postprocess->p_processor->nStride;

                /*************************************************************
                 * Upsampling H2V1 to H1V1 Line
                 ************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                 * Color Conversion YUV to RGB Line
                 ****************************************************************/
                y1vu2rgb888line(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                        postprocess->nColsToProcess);

                postprocess->pDstRGB += postprocess->p_processor->nStride;
            }
        }

        break;


    case H2V2_TO_RGBa:

        /*****************************************************************
         * Upsampling H2V1 to H1V1 Line
         ****************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
         * Color Conversion YUV to RGB Line
         ****************************************************************/
        y1vu2rgbaline(postprocess->pSrcLuma,
                      postprocess->p_processor->pChromaSmpOutput,
                      postprocess->pDstRGB,
                      postprocess->nColsToProcess);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += postprocess->p_processor->nStride;

        /*****************************************************************
         * Upsampling H2V2 to H1V1 Line
        *
        * Notice that for this H2V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgbaline(postprocess->pSrcLuma,
                          postprocess->p_processor->pChromaSmpOutput,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/

            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                 * Upsampling H2V2 to H1V1
                 ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgbaline(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += postprocess->p_processor->nStride;

                /*************************************************************
                 * Upsampling H2V2 to H1V1
                 ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgbaline(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    += postprocess->p_processor->nStride;
            }

            /*****************************************************************
             * Upsampling H2V2 to H1V1
             ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgbaline(postprocess->pSrcLuma,
                          postprocess->p_processor->pChromaSmpOutput,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += postprocess->p_processor->nStride;

                /*************************************************************
                 * Upsampling H2V1 to H1V1 Line
                 ************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                 * Color Conversion YUV to RGB Line
                 ****************************************************************/
                y1vu2rgbaline(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess);

                postprocess->pDstRGB += postprocess->p_processor->nStride;
            }
        }

        break;


    case H2V1_TO_RGB565:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*****************************************************************
             * Upsampling H2V1 to H1V1 Line
             ****************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
             * Color Conversion YUV to RGB Line
             ************************************************************/
            y1vu2rgb565line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        break;


    case H2V1_TO_RGB888:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*****************************************************************
             * Upsampling H2V1 to H1V1 Line
             ****************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
             * Color Conversion YUV to RGB Line
             ************************************************************/
            y1vu2rgb888line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        break;


    case H2V1_TO_RGBa:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*****************************************************************
             * Upsampling H2V1 to H1V1 Line
             ****************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
             * Color Conversion YUV to RGB Line
             ************************************************************/
            y1vu2rgbaline(postprocess->pSrcLuma,
                          postprocess->p_processor->pChromaSmpOutput,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        break;


    case H1V2_TO_RGB565:

        /*****************************************************************
         * Color Conversion YUV to RGB Line
         ****************************************************************/
        y1vu2rgb565line(postprocess->pSrcLuma,
                        postprocess->pSrcChroma,
                        postprocess->pDstRGB,
                        postprocess->nColsToProcess);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += postprocess->p_processor->nStride;

        /*****************************************************************
         * Upsampling H1V2 to H1V1 Line
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgb565line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                 * Upsampling H1V2 to H1V1
                 ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgb565line(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += postprocess->p_processor->nStride;

                /*************************************************************
                 * Upsampling H1V2 to H1V1
                 ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgb565line(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
                postprocess->pDstRGB    += postprocess->p_processor->nStride;
            }

            /*****************************************************************
             * Upsampling H1V2 to H1V1
             ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgb565line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB += postprocess->p_processor->nStride;

                /*****************************************************************
                 * Color Conversion YUV to RGB Line
                 ****************************************************************/
                y1vu2rgb565line(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                        postprocess->nColsToProcess);

                postprocess->pDstRGB += postprocess->p_processor->nStride;
            }
        }

        break;


    case H1V2_TO_RGB888:

        /*****************************************************************
         * Color Conversion YUV to RGB Line
         ****************************************************************/
        y1vu2rgb888line(postprocess->pSrcLuma,
                        postprocess->pSrcChroma,
                        postprocess->pDstRGB,
                        postprocess->nColsToProcess);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += postprocess->p_processor->nStride;

        /*****************************************************************
         * Upsampling H1V2 to H1V1 Line
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgb888line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                 * Upsampling H1V2 to H1V1
                 ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgb888line(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += postprocess->p_processor->nStride;

                /*************************************************************
                 * Upsampling H1V2 to H1V1
                 ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgb888line(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
                postprocess->pDstRGB    += postprocess->p_processor->nStride;
            }

            /*****************************************************************
             * Upsampling H1V2 to H1V1
             ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgb888line(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB += postprocess->p_processor->nStride;

                /*****************************************************************
                 * Color Conversion YUV to RGB Line
                 ****************************************************************/
                y1vu2rgb888line(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess);

                postprocess->pDstRGB += postprocess->p_processor->nStride;
            }
        }

        break;


    case H1V2_TO_RGBa:

        /*****************************************************************
         * Color Conversion YUV to RGB Line
         ****************************************************************/
        y1vu2rgbaline(postprocess->pSrcLuma,
                      postprocess->pSrcChroma,
                      postprocess->pDstRGB,
                      postprocess->nColsToProcess);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += postprocess->p_processor->nStride;

        /*****************************************************************
         * Upsampling H1V2 to H1V1 Line
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgbaline(postprocess->pSrcLuma,
                          postprocess->p_processor->pChromaSmpOutput,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline(postprocess->pSrcLuma,
                          postprocess->pSrcChroma,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                 * Upsampling H1V2 to H1V1
                 ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgbaline(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += postprocess->p_processor->nStride;

                /*************************************************************
                 * Upsampling H1V2 to H1V1
                 ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                 * Color Conversion YUV to RGB Line
                 ************************************************************/
                y1vu2rgbaline(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
                postprocess->pDstRGB    += postprocess->p_processor->nStride;
            }

            /*****************************************************************
             * Upsampling H1V2 to H1V1
             ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgbaline(postprocess->pSrcLuma,
                          postprocess->p_processor->pChromaSmpOutput,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += postprocess->p_processor->nStride;

                /*****************************************************************
                 * Color Conversion YUV to RGB Line
                 ****************************************************************/
                y1vu2rgbaline(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess);

               postprocess->pDstRGB += postprocess->p_processor->nStride;
            }
        }

        break;


    case H1V1_TO_RGB565:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgb565line(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;

            postprocess->pDstRGB += postprocess->p_processor->nStride;
        }

        break;


    case H1V1_TO_RGB888:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgb888line(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess);

            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;

            postprocess->pDstRGB += postprocess->p_processor->nStride;
        }

        break;


    case H1V1_TO_RGBa:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgbaline(postprocess->pSrcLuma,
                          postprocess->pSrcChroma,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess);

            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;

            postprocess->pDstRGB += postprocess->p_processor->nStride;
        }

        break;

    default:
        break;

    }
}


/**************************************************************************
*  COLOR CONVERT will convert YUV to RGB and remove padding
*************************************************************************/
void color_convert_rot90(jpegd_postprocess_func_param_t *postprocess)
{
    uint32_t y; //Loop counter

    switch (postprocess->p_processor->color_convert)
    {
    case H2V2_TO_RGB565_R90:

        /*************************************************************
        * Upsampling H2V1 to H1V1 Line
        ************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
         * Color Conversion YUV to RGB Line
         ****************************************************************/
        y1vu2rgb565line_rot(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            (int32_t)(postprocess->p_processor->nStride));

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= 2;

        /*************************************************************
         * Upsampling H2V2 to H1V1
         *
         * Notice that for this H2V2 case, if there are only 2 Y lines
         * to process, there will be only 1 CbCr line, and averaging
         * 2 chroma lines is meaningless.
         * So only when Y lines are greater than 2 shall it averages
         * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= 2;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 2;

                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    -= 2;
            }

            /*****************************************************************
            * Upsampling H2V2 to H1V1
            ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 2;

                /*****************************************************************
                * Upsampling H2V2 to H1V1
                ****************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pDstRGB -= 2;
            }
        }

        break;


    case H2V2_TO_RGB888_R90:

        /*****************************************************************
        * Upsample Chroma line horizontally(H2 -> H1).
        ****************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
        * Color Convert Non-Chroma sub-sampled YUV Line to RGB Line
        ****************************************************************/
        y1vu2rgb888line_rot(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            (int32_t)(postprocess->p_processor->nStride));

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= 3;

        /*************************************************************
        * Upsampling H2V2 to H1V1
        *
        * Notice that for this H2V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 Chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= 3;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 3;

                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    -= 3;
            }

            /*****************************************************************
            * Upsampling H2V2 to H1V1
            ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 3;

                /*************************************************************
                * Upsampling H2V1 to H1V1 Line
                ************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pDstRGB -= 3;
            }
        }

        break;


    case H2V2_TO_RGBa_R90:

        /*************************************************************
        * Upsampling H2V2 to H1V1
        ************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgbaline_rot(postprocess->pSrcLuma,
                          postprocess->p_processor->pChromaSmpOutput,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess,
                          (int32_t)(postprocess->p_processor->nStride));

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= 4;

        /*************************************************************
         * Upsampling H2V2 to H1V1
         *
         * Notice that for this H2V2 case, if there are only 2 Y lines
         * to process, there will be only 1 CbCr line, and averaging
         * 2 chroma lines is meaningless.
         * So only when Y lines are greater than 2 shall it averages
         * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              (int32_t)(postprocess->p_processor->nStride));

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= 4;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->p_processor->pChromaSmpOutput,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 4;

                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->p_processor->pChromaSmpOutput,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    -= 4;
            }

            /*****************************************************************
            * Upsampling H2V2 to H1V1
            ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              (int32_t)(postprocess->p_processor->nStride));

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 4;

                /*****************************************************************
                 * Upsampling H2V2 to H1V1
                 ****************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                 * Color Conversion YUV to RGB Line
                 ****************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->p_processor->pChromaSmpOutput,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  (int32_t)(postprocess->p_processor->nStride));

                postprocess->pDstRGB -= 4;
            }
        }

        break;


    case H2V1_TO_RGB565_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*************************************************************
            * Upsampling H2V1 to H1V1
            ************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
            * Color Conversion YUV to RGB Line
            ************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                postprocess->p_processor->nStride);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= 2;
        }

        break;


    case H2V1_TO_RGB888_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*************************************************************
            * Upsampling H2V1 to H1V1
            ************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
            * Color Conversion YUV to RGB Line
            ************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                postprocess->p_processor->nStride);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= 3;
        }

        break;


    case H2V1_TO_RGBa_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*************************************************************
            * Upsampling H2V1 to H1V1
            ************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
            * Color Conversion YUV to RGB Line
            ************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              postprocess->p_processor->nStride);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= 4;
        }

        break;


    case H1V2_TO_RGB565_R90:

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgb565line_rot(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            (int32_t)(postprocess->p_processor->nStride));

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= 2;

        /*************************************************************
        * Upsampling H1V2 to H1V1
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);


            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= 2;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= 2;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 2;

                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2 ;
                postprocess->pDstRGB    -= 2;
            }

            /*****************************************************************
            * Upsampling H1V2 to H1V1
            ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 2;

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pDstRGB -= 2;
            }
        }

        break;


    case H1V2_TO_RGB888_R90:

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgb888line_rot(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            (int32_t)(postprocess->p_processor->nStride));

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= 3;

        /*************************************************************
        * Upsampling H1V2 to H1V1
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= 3;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= 3;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 3;

                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2 ;
                postprocess->pDstRGB    -= 3;
            }

            /*****************************************************************
            * Upsampling H1V2 to H1V1
            ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                (int32_t)(postprocess->p_processor->nStride));

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 3;

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    (int32_t)(postprocess->p_processor->nStride));

                postprocess->pDstRGB -= 3;
            }
        }

        break;


    case H1V2_TO_RGBa_R90:

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgbaline_rot(postprocess->pSrcLuma,
                          postprocess->pSrcChroma,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess,
                          (int32_t)(postprocess->p_processor->nStride));

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= 4;

        /*************************************************************
        * Upsampling H1V2 to H1V1
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              (int32_t)(postprocess->p_processor->nStride));

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= 4;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              (int32_t)(postprocess->p_processor->nStride));

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= 4;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->pSrcChroma,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 4;

                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->pSrcChroma,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  (int32_t)(postprocess->p_processor->nStride));

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2 ;
                postprocess->pDstRGB    -= 4;
            }

            /*****************************************************************
            * Upsampling H1V2 to H1V1
            ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              (int32_t)(postprocess->p_processor->nStride));

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= 4;

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->pSrcChroma,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  (int32_t)(postprocess->p_processor->nStride));

                postprocess->pDstRGB -= 4;
            }
        }

        break;


    case H1V1_TO_RGB565_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                postprocess->p_processor->nStride);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= 2;
        }

        break;


    case H1V1_TO_RGB888_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                postprocess->p_processor->nStride);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= 3;
        }

        break;


    case H1V1_TO_RGBa_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              postprocess->p_processor->nStride);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= 4;
        }

        break;

    default:
        break;

    }
}


/**************************************************************************
*  COLOR CONVERT will convert YUV to RGB and remove padding
*************************************************************************/
void color_convert_rot180(jpegd_postprocess_func_param_t *postprocess)
{
    uint32_t y; //Loop counter

    switch (postprocess->p_processor->color_convert)
    {
    case H2V2_TO_RGB565_R180:

        /*************************************************************
         * Upsampling H2V1 to H1V1 Line
         ************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
         * Color Conversion YUV to RGB Line
         ****************************************************************/
        y1vu2rgb565line_rot(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            -2);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= postprocess->p_processor->nStride;

        /*************************************************************
         * Upsampling H2V2 to H1V1
         *
         * Notice that for this H2V2 case, if there are only 2 Y lines
         * to process, there will be only 1 CbCr line, and averaging
         * 2 chroma lines is meaningless.
         * So only when Y lines are greater than 2 shall it averages
         * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -2);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    -2);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    -2);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    -= postprocess->p_processor->nStride;
            }

            /*****************************************************************
             * Upsampling H2V2 to H1V1
             ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
             * Color Conversion YUV to RGB Line
             ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -2);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*****************************************************************
                 * Upsampling H2V2 to H1V1
                 ****************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                 * Color Conversion YUV to RGB Line
                 ****************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    -2);

                postprocess->pDstRGB -= postprocess->p_processor->nStride;
            }
        }

        break;


    case H2V2_TO_RGB888_R180:

        /*****************************************************************
        * Upsample Chroma line horizontally(H2 -> H1).
        ****************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
        * Color Convert Non-Chroma sub-sampled YUV Line to RGB Line
        ****************************************************************/
        y1vu2rgb888line_rot(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            -3);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= postprocess->p_processor->nStride;

        /*************************************************************
        * Upsampling H2V2 to H1V1
        *
        * Notice that for this H2V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 Chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -3);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    -3);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    -3);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    -= postprocess->p_processor->nStride;
            }

            /*****************************************************************
            * Upsampling H2V2 to H1V1
            ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -3);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*************************************************************
                * Upsampling H2V1 to H1V1 Line
                ************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    -3);

                postprocess->pDstRGB -= postprocess->p_processor->nStride;
            }
        }

        break;


    case H2V2_TO_RGBa_R180:

        /*************************************************************
        * Upsampling H2V2 to H1V1
        ************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgbaline_rot(postprocess->pSrcLuma,
                          postprocess->p_processor->pChromaSmpOutput,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess,
                          -4);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= postprocess->p_processor->nStride;

        /*************************************************************
         * Upsampling H2V2 to H1V1
         *
         * Notice that for this H2V2 case, if there are only 2 Y lines
         * to process, there will be only 1 CbCr line, and averaging
         * 2 chroma lines is meaningless.
         * So only when Y lines are greater than 2 shall it averages
         * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              -4);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->p_processor->pChromaSmpOutput,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  -4);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->p_processor->pChromaSmpOutput,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  -4);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;
            }

            /*****************************************************************
            * Upsampling H2V2 to H1V1
            ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              -4);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*****************************************************************
                 * Upsampling H2V2 to H1V1
                 ****************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                 * Color Conversion YUV to RGB Line
                 ****************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->p_processor->pChromaSmpOutput,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  -4);

                postprocess->pDstRGB -= postprocess->p_processor->nStride;
            }
        }

        break;


    case H2V1_TO_RGB565_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*************************************************************
            * Upsampling H2V1 to H1V1
            ************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
            * Color Conversion YUV to RGB Line
            ************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -2);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        break;


    case H2V1_TO_RGB888_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*************************************************************
            * Upsampling H2V1 to H1V1
            ************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
            * Color Conversion YUV to RGB Line
            ************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -3);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        break;


    case H2V1_TO_RGBa_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*************************************************************
            * Upsampling H2V1 to H1V1
            ************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
            * Color Conversion YUV to RGB Line
            ************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              -4);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        break;


    case H1V2_TO_RGB565_R180:

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgb565line_rot(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            -2);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= postprocess->p_processor->nStride;

        /*************************************************************
        * Upsampling H1V2 to H1V1
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);


            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -2);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -2);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                   postprocess->pSrcChroma,
                                   postprocess->pDstRGB,
                                   postprocess->nColsToProcess,
                                   -2);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                   postprocess->pSrcChroma,
                                   postprocess->pDstRGB,
                                   postprocess->nColsToProcess,
                                   -2);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2 ;
                postprocess->pDstRGB    -= postprocess->p_processor->nStride;
            }

            /*****************************************************************
            * Upsampling H1V2 to H1V1
            ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -2);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    -2);

                postprocess->pDstRGB -= postprocess->p_processor->nStride;
            }
        }

        break;


    case H1V2_TO_RGB888_R180:

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgb888line_rot(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            -3);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= postprocess->p_processor->nStride;

        /*************************************************************
        * Upsampling H1V2 to H1V1
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -3);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -3);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                   postprocess->pSrcChroma,
                                   postprocess->pDstRGB,
                                   postprocess->nColsToProcess,
                                   -3);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                   postprocess->pSrcChroma,
                                   postprocess->pDstRGB,
                                   postprocess->nColsToProcess,
                                   -3);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2 ;
                postprocess->pDstRGB    -= postprocess->p_processor->nStride;
            }

            /*****************************************************************
            * Upsampling H1V2 to H1V1
            ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -3);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    -3);

                postprocess->pDstRGB -= postprocess->p_processor->nStride;
            }
        }

        break;


    case H1V2_TO_RGBa_R180:

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgbaline_rot(postprocess->pSrcLuma,
                          postprocess->pSrcChroma,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess,
                          -4);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  -= postprocess->p_processor->nStride;

        /*************************************************************
        * Upsampling H1V2 to H1V1
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              -4);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              -4);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->pSrcChroma,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  -4);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->pSrcChroma,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  -4);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2 ;
                postprocess->pDstRGB    -= postprocess->p_processor->nStride;
            }

            /*****************************************************************
            * Upsampling H1V2 to H1V1
            ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              -4);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  -= postprocess->p_processor->nStride;

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->pSrcChroma,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  -4);

                postprocess->pDstRGB -= postprocess->p_processor->nStride;
            }
        }

        break;


    case H1V1_TO_RGB565_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -2);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        break;


    case H1V1_TO_RGB888_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                -3);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        break;


    case H1V1_TO_RGBa_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              -4);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    -= postprocess->p_processor->nStride;
        }

        break;

    default:
        break;

    }
}


/**************************************************************************
*  COLOR CONVERT will convert YUV to RGB and remove padding
*************************************************************************/
void color_convert_rot270(jpegd_postprocess_func_param_t *postprocess)
{
    uint32_t y; //Loop counter

    /***********************************************************************
    *  For 270 cases, the increment is the negative of the stride.
    *  Declare a signed variable to hold the casted unsigned Stride
    *  instead of using -Stride directly as the function parameter to
    *  make Lint happy.
    ***********************************************************************/
    int32_t nIncrement270 = (int32_t)(0 - postprocess->p_processor->nStride);

    switch (postprocess->p_processor->color_convert)
    {
    case H2V2_TO_RGB565_R270:

        /*************************************************************
        * Upsampling H2V1 to H1V1 Line
        ************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgb565line_rot(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            nIncrement270);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += 2;

        /*************************************************************
        * Upsampling H2V2 to H1V1
        *
        * Notice that for this H2V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += 2;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 2;

                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    += 2;
            }

            /*****************************************************************
            * Upsampling H2V2 to H1V1
            ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 2;

                /*****************************************************************
                * Upsampling H2V2 to H1V1
                ****************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pDstRGB += 2;
            }
        }

        break;


    case H2V2_TO_RGB888_R270:

        /*****************************************************************
        * Upsample Chroma line horizontally(H2 -> H1).
        ****************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
        * Color Convert Non-Chroma sub-sampled YUV Line to RGB Line
        ****************************************************************/
        y1vu2rgb888line_rot(postprocess->pSrcLuma,
                            postprocess->p_processor->pChromaSmpOutput,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            nIncrement270);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += 3;

        /*************************************************************
        * Upsampling H2V2 to H1V1
        *
        * Notice that for this H2V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 Chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += 3;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 3;

                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    += 3;
            }

            /*****************************************************************
            * Upsampling H2V2 to H1V1
            ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 3;

                /*************************************************************
                * Upsampling H2V1 to H1V1 Line
                ************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->p_processor->pChromaSmpOutput,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pDstRGB += 3;
            }
        }

        break;


    case H2V2_TO_RGBa_R270:

        /*************************************************************
        * Upsampling H2V2 to H1V1
        ************************************************************/
        y2vu1upy1vu1line(postprocess->pSrcChroma,
                         postprocess->p_processor->pChromaSmpOutput,
                         postprocess->nColsToProcess);

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgbaline_rot(postprocess->pSrcLuma,
                          postprocess->p_processor->pChromaSmpOutput,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess,
                          nIncrement270);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += 4;

        /*************************************************************
         * Upsampling H2V2 to H1V1
         *
         * Notice that for this H2V2 case, if there are only 2 Y lines
         * to process, there will be only 1 CbCr line, and averaging
         * 2 chroma lines is meaningless.
         * So only when Y lines are greater than 2 shall it averages
         * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);
        }

        /**********************************************************************
        * If there is only one line in the MCU Row, then process only one line.
        * The following statements under "if" should not run.
        **********************************************************************/
        if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += 4;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->p_processor->pChromaSmpOutput,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  nIncrement270);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 4;

                /*************************************************************
                * Upsampling H2V2 to H1V1
                ************************************************************/
                y2vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->p_processor->pChromaSmpOutput,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  nIncrement270);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB    += 4;
            }

            /*****************************************************************
            * Upsampling H2V2 to H1V1
            ****************************************************************/
            y2vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              nIncrement270);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 4;

                /*****************************************************************
                * Upsampling H2V2 to H1V1
                ****************************************************************/
                y2vu1upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->p_processor->pChromaSmpOutput,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  nIncrement270);

                postprocess->pDstRGB += 4;
            }
        }

        break;


    case H2V1_TO_RGB565_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*************************************************************
            * Upsampling H2V1 to H1V1
            ************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
            * Color Conversion YUV to RGB Line
            ************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += 2;
        }

        break;


    case H2V1_TO_RGB888_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*************************************************************
            * Upsampling H2V1 to H1V1
            ************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
            * Color Conversion YUV to RGB Line
            ************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->p_processor->pChromaSmpOutput,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += 3;
        }

        break;


    case H2V1_TO_RGBa_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            /*************************************************************
            * Upsampling H2V1 to H1V1
            ************************************************************/
            y2vu1upy1vu1line(postprocess->pSrcChroma,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*************************************************************
            * Color Conversion YUV to RGB Line
            ************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->p_processor->pChromaSmpOutput,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstRGB    += 4;
        }

        break;


    case H1V2_TO_RGB565_R270:

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgb565line_rot(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            nIncrement270);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += 2;

        /*************************************************************
        * Upsampling H1V2 to H1V1
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);


            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += 2;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += 2;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 2;

                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);


                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2 ;
                postprocess->pDstRGB    += 2;
            }

            /*****************************************************************
            * Upsampling H1V2 to H1V1
            ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 2;

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pDstRGB += 2;
            }
        }

        break;


    case H1V2_TO_RGB888_R270:

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgb888line_rot(postprocess->pSrcLuma,
                            postprocess->pSrcChroma,
                            postprocess->pDstRGB,
                            postprocess->nColsToProcess,
                            nIncrement270);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += 3;

        /*************************************************************
        * Upsampling H1V2 to H1V1
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
        ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += 3;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += 3;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 3;

                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2 ;
                postprocess->pDstRGB    += 3;
        }

            /*****************************************************************
            * Upsampling H1V2 to H1V1
            ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 3;

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                    postprocess->pSrcChroma,
                                    postprocess->pDstRGB,
                                    postprocess->nColsToProcess,
                                    nIncrement270);

                postprocess->pDstRGB += 3;
            }
        }

        break;


    case H1V2_TO_RGBa_R270:

        /*****************************************************************
        * Color Conversion YUV to RGB Line
        ****************************************************************/
        y1vu2rgbaline_rot(postprocess->pSrcLuma,
                          postprocess->pSrcChroma,
                          postprocess->pDstRGB,
                          postprocess->nColsToProcess,
                          nIncrement270);

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
        postprocess->pDstRGB  += 4;

        /*************************************************************
        * Upsampling H1V2 to H1V1
        *
        * Notice that for this H1V2 case, if there are only 2 Y lines
        * to process, there will be only 1 CbCr line, and averaging
        * 2 chroma lines is meaningless.
        * So only when Y lines are greater than 2 shall it averages
        * 2 chroma lines.
         ************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += 4;
        }
        else if (postprocess->nRowsToProcess > 1)
        {
            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += 4;
        }

        /*****************************************************************
        * The following statements will be executed only when the number
        * of lines to process is greater than 2.
        *****************************************************************/
        if (postprocess->nRowsToProcess > 2)
        {
            /**********************************************************************
            * Notice that if nRowsToProcess is odd, the for loop will execute
            * one more time than when nRowsToProcess is even.
            **********************************************************************/
            for (y = 2; y < postprocess->nRowsToProcess - 2; y += 2)
            {
                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->pSrcChroma,
                                  postprocess->pDstRGB,
                                      postprocess->nColsToProcess,
                                  nIncrement270);

                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 4;

                /*************************************************************
                * Upsampling H1V2 to H1V1
                ************************************************************/
                y1vu2upy1vu1line(postprocess->pSrcChroma,
                                 postprocess->pSrcChroma + postprocess->p_processor->chunk_width * 2,
                                 postprocess->p_processor->pChromaSmpOutput,
                                 postprocess->nColsToProcess);

                /*************************************************************
                * Color Conversion YUV to RGB Line
                ************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->pSrcChroma,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  nIncrement270);

                postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2 ;
                postprocess->pDstRGB    += 4;
            }

            /*****************************************************************
            * Upsampling H1V2 to H1V1
            ****************************************************************/
            y1vu2upy1vu1line(postprocess->pSrcChroma,
                             postprocess->pSrcChroma - postprocess->p_processor->chunk_width * 2,
                             postprocess->p_processor->pChromaSmpOutput,
                             postprocess->nColsToProcess);

            /*****************************************************************
            * Color Conversion YUV to RGB Line
            ****************************************************************/
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              nIncrement270);

            /****************************************************************
            * Since the previous for loop executes one more time in case of
            * odd nRowsToProcess, the Chroma Line for the very last
            * Luma Line (odd) is already obtained and converted to RGB Line
            * throught the above statements.
            *
            * Execute the following statements only when nRowsToProcess
            * is even.
            *****************************************************************/
            if (!(postprocess->nRowsToProcess & 0x01))
            {
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstRGB  += 4;

                /*****************************************************************
                * Color Conversion YUV to RGB Line
                ****************************************************************/
                y1vu2rgbaline_rot(postprocess->pSrcLuma,
                                  postprocess->pSrcChroma,
                                  postprocess->pDstRGB,
                                  postprocess->nColsToProcess,
                                  nIncrement270);

                postprocess->pDstRGB += 4;
            }
        }

        break;


    case H1V1_TO_RGB565_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgb565line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += 2;
        }

        break;


    case H1V1_TO_RGB888_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgb888line_rot(postprocess->pSrcLuma,
                                postprocess->pSrcChroma,
                                postprocess->pDstRGB,
                                postprocess->nColsToProcess,
                                nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += 3;
        }

        break;


    case H1V1_TO_RGBa_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            y1vu2rgbaline_rot(postprocess->pSrcLuma,
                              postprocess->pSrcChroma,
                              postprocess->pDstRGB,
                              postprocess->nColsToProcess,
                              nIncrement270);

            postprocess->pSrcLuma   += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width * 2;
            postprocess->pDstRGB    += 4;
        }

        break;

    default:
        break;

    }
}


/******************************************************************************
 * The following functions are for rotation cases.
 * For non-rotation cases, VaNum assembly code will be used and the
 * corresponding C code is in a seperate file jpeg_postprocess_yuv2rgb.c.
 ******************************************************************************/

/******************************************************************************
 * Convert YVU line to RGB 565 line for rotation cases with increment parameter.
 ******************************************************************************/
void y2vu2rgb565line_rot(uint8_t * pLumaLine,
                         uint8_t * pChromaLine,
                         uint8_t * pRGB565Line,
                         uint32_t nLineWidth,
                         int32_t nIncrement)
{
    uint32_t i;

    int32_t C, D, E;
    int32_t subChromaR, subChromaG, subChromaB, subLuma;
    uint8_t R, G, B;

    /***********************************************************************
    * Notice that if nLineWidth is odd, the for loop will execute
    * one more time than when nLineWidth is even.
    ************************************************************************/
    for (i = 0; i < nLineWidth - 2; i += 2)
    {
        // first pixel
        C = *pLumaLine      - 16;   // Y
        D = *(pChromaLine+1)- 128;  // Cb
        E = *pChromaLine    - 128;  // Cr

        subChromaR =            409*E + 128;
        subChromaG = (-100)*D - 208*E + 128;
        subChromaB =    516*D         + 128;

        subLuma = 298*C;

        R = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));
        G = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
        B = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));

        *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
        *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);

        // second pixel, re-use subChroma
        C            = *(pLumaLine+1) -16;
        subLuma      = 298*C;
        pRGB565Line += nIncrement;

        R = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));
        G = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
        B = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));

        *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
        *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);

        pLumaLine   += 2;
        pChromaLine += 2;
        pRGB565Line += nIncrement;
    }

    /**************************************************************************
    * Since the above for loop executes one more time in case of
    * odd nLineWidth, thre is only one pixel, the very last Y (odd case) left.
    * So process only this single pixel for the odd width case.
    **************************************************************************/
    C = *pLumaLine      - 16;  // Y
    D = *(pChromaLine+1)- 128; // Cb
    E = *pChromaLine    - 128; // Cr

    subChromaR =            409*E + 128;
    subChromaG = (-100)*D - 208*E + 128;
    subChromaB =    516*D         + 128;

    subLuma = 298*C;

    R = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));
    G = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
    B = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));

    *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
    *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);

    /*************************************************************************
    * Execute the following statements only when nLineWidth is even.
    *************************************************************************/
    if (!(nLineWidth & 0x01))
    {
        C            = *(pLumaLine+1) -16;
        subLuma      = 298*C;
        pRGB565Line += nIncrement;

        R = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));
        G = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
        B = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));

        *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
        *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);
    }
}


/******************************************************************************
 * Convert YVU line to RGB 888 line for rotation cases with increment parameter.
 ******************************************************************************/
void y2vu2rgb888line_rot(uint8_t * pLumaLine,
                         uint8_t * pChromaLine,
                         uint8_t * pRGB888Line,
                         uint32_t nLineWidth,
                         int32_t nIncrement)
{
    uint32_t i;

    int32_t C, D, E;
    int32_t subChromaR, subChromaG, subChromaB, subLuma;

    /***********************************************************************
    * Notice that if nLineWidth is odd, the for loop will execute
    * one more time than when nLineWidth is even.
    ************************************************************************/
    for ( i = 0; i < nLineWidth - 2; i += 2 )
    {
        // first pixel
        C = *pLumaLine      - 16;   // Y
        D = *(pChromaLine+1)- 128;  // Cb
        E = *pChromaLine    - 128;  // Cr

        subChromaR =            409*E + 128;
        subChromaG = (-100)*D - 208*E + 128;
        subChromaB =    516*D         + 128;

        subLuma = 298*C;

        *pRGB888Line     = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGB888Line+1) = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGB888Line+2) = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));

        // second pixel, re-use subChroma
        C            = *(pLumaLine+1) -16;
        subLuma      = 298*C;
        pRGB888Line += nIncrement;

        *pRGB888Line     = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGB888Line+1) = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGB888Line+2) = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));

        // update the pointers
        pLumaLine   += 2;
        pChromaLine += 2;
        pRGB888Line += nIncrement;
    }

    /**************************************************************************
    * Since the above for loop executes one more time in case of
    * odd nLineWidth, thre is only one pixel, the very last Y (odd case) left.
    * So process only this single pixel for the odd width case.
    **************************************************************************/
    C = *pLumaLine      - 16;  // Y
    D = *(pChromaLine+1)- 128; // Cb
    E = *pChromaLine    - 128; // Cr

    subChromaR =            409*E + 128;
    subChromaG = (-100)*D - 208*E + 128;
    subChromaB =    516*D         + 128;

    subLuma = 298*C;

    *pRGB888Line     = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));
    *(pRGB888Line+1) = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
    *(pRGB888Line+2) = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));

    /*************************************************************************
    * Execute the following statements only when nLineWidth is even.
    *************************************************************************/
    if (!(nLineWidth & 0x01))
    {
        C            = *(pLumaLine+1) -16;
        subLuma      = 298*C;
        pRGB888Line += nIncrement;

        *pRGB888Line     = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGB888Line+1) = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGB888Line+2) = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));
    }
}


/*****************************************************************************
 * Convert YVU line to RGBa line for rotation cases with increment parameter.
 ****************************************************************************/
void y2vu2rgbaline_rot(uint8_t * pLumaLine,
                       uint8_t * pChromaLine,
                       uint8_t * pRGBaLine,
                       uint32_t nLineWidth,
                       int32_t nIncrement)
{
    uint32_t i;

    int32_t C, D, E;
    int32_t subChromaR, subChromaG, subChromaB, subLuma;

    /***********************************************************************
    * Notice that if nLineWidth is odd, the for loop will execute
    * one more time than when nLineWidth is even.
    ************************************************************************/
    for (i = 0; i < nLineWidth - 2; i += 2 )
    {
        // first pixel
        C = *pLumaLine      - 16;   // Y
        D = *(pChromaLine+1)- 128;  // Cb
        E = *pChromaLine    - 128;  // Cr

        subChromaR =            409*E + 128;
        subChromaG = (-100)*D - 208*E + 128;
        subChromaB =    516*D         + 128;

        subLuma = 298*C;

        *pRGBaLine     = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGBaLine+1) = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGBaLine+2) = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));
        *(pRGBaLine+3) = 0xFF;

        // second pixel, re-use subChroma
        C          = *(pLumaLine+1) -16;
        subLuma    = 298*C;
        pRGBaLine += nIncrement;

        *pRGBaLine     = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGBaLine+1) = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGBaLine+2) = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));
        *(pRGBaLine+3) = 0xFF;

        // update the pointers
        pLumaLine   += 2;
        pChromaLine += 2;
        pRGBaLine   += nIncrement;
    }

    /**************************************************************************
    * Since the above for loop executes one more time in case of
    * odd nLineWidth, thre is only one pixel, the very last Y (odd case) left.
    * So process only this single pixel for the odd width case.
    **************************************************************************/
    C = *pLumaLine      - 16;  // Y
    D = *(pChromaLine+1)- 128; // Cb
    E = *pChromaLine    - 128; // Cr

    subChromaR =            409*E + 128;
    subChromaG = (-100)*D - 208*E + 128;
    subChromaB =    516*D         + 128;

    subLuma = 298*C;

    *pRGBaLine     = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));
    *(pRGBaLine+1) = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
    *(pRGBaLine+2) = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));
    *(pRGBaLine+3) = 0xFF;

    /*************************************************************************
    * Execute the following statements only when nLineWidth is even.
    *************************************************************************/
    if (!(nLineWidth & 0x01))
    {
        C          = *(pLumaLine+1) -16;
        subLuma    = 298*C;
        pRGBaLine += nIncrement;

        *pRGBaLine     = (uint8_t)(_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGBaLine+1) = (uint8_t)(_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGBaLine+2) = (uint8_t)(_CLAMP ((subLuma + subChromaR) >> 8));
        *(pRGBaLine+3) = 0xFF;
    }
}

/**************************************************************************
 * Convert YVU line (No horizontal chroma subsampling) to RGB 565 line.
 *************************************************************************/
void y1vu2rgb565line_rot(uint8_t * pLumaLine,
                         uint8_t * pChromaLine,
                         uint8_t * pRGB565Line,
                         uint32_t nLineWidth,
                         int32_t nIncrement)
{
    uint32_t i;
    int32_t C, D, E;
    uint8_t R, G, B;

    for (i = 0; i < nLineWidth; i++)
    {
        C = *pLumaLine      - 16;
        D = *(pChromaLine+1)- 128;
        E = *pChromaLine    - 128;

        R = (uint8_t)(_CLAMP(( 298 * C           + 409 * E + 128) >> 8));
        G = (uint8_t)(_CLAMP(( 298 * C - 100 * D - 208 * E + 128) >> 8));
        B = (uint8_t)(_CLAMP(( 298 * C + 516 * D           + 128) >> 8));

        *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
        *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);

        // update pointers
        pLumaLine   += 1;
        pChromaLine += 2;
        pRGB565Line += nIncrement;
    }
}

/**************************************************************************
 * Convert YVU line (No horizontal chroma subsampling) to RGB 888 line.
 *************************************************************************/
void y1vu2rgb888line_rot(uint8_t * pLumaLine,
                         uint8_t * pChromaLine,
                         uint8_t * pRGB888Line,
                         uint32_t nLineWidth,
                         int32_t nIncrement)
{
    uint32_t i;
    int32_t C, D, E;

    for (i = 0; i < nLineWidth; i++)
    {
        C = *pLumaLine      - 16;
        D = *(pChromaLine+1)- 128;
        E = *pChromaLine    - 128;

        *(pRGB888Line+2) = (uint8_t)(_CLAMP(( 298 * C           + 409 * E + 128) >> 8));
        *(pRGB888Line+1) = (uint8_t)(_CLAMP(( 298 * C - 100 * D - 208 * E + 128) >> 8));
        *pRGB888Line     = (uint8_t)(_CLAMP(( 298 * C + 516 * D           + 128) >> 8));

        // update the pointers
        pLumaLine   += 1;
        pChromaLine += 2;
        pRGB888Line += nIncrement;
    }
}

/**************************************************************************
 * Convert YVU line (No horizontal chroma subsampling) to RGBa line.
 *************************************************************************/
void y1vu2rgbaline_rot(uint8_t * pLumaLine,
                       uint8_t * pChromaLine,
                       uint8_t * pRGBaLine,
                       uint32_t nLineWidth,
                       int32_t nIncrement)
{
    uint32_t i;
    int32_t C, D, E;

    for ( i = 0; i < nLineWidth; i++)
    {
        C = *pLumaLine      - 16;
        D = *(pChromaLine+1)- 128;
        E = *pChromaLine    - 128;


        *pRGBaLine     = (uint8_t)(_CLAMP(( 298 * C + 516 * D           + 128) >> 8));
        *(pRGBaLine+1) = (uint8_t)(_CLAMP(( 298 * C - 100 * D - 208 * E + 128) >> 8));
        *(pRGBaLine+2) = (uint8_t)(_CLAMP(( 298 * C           + 409 * E + 128) >> 8));
        *(pRGBaLine+3) = 0xFF;

        // update the pointers
        pLumaLine   += 1;
        pChromaLine += 2;
        pRGBaLine   += nIncrement;
    }
}


