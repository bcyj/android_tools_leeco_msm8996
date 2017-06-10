/*========================================================================


*//** @file jpeg_postprocess_ds.c

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
11/03/09   mingy   Added CbCr output support.
09/21/09   mingy   Added odd sized image decoding support.
                   Changed the name of the post process function
                   parameter structure.
04/22/09   mingy   Created file.
                   Branch out down scale from jpeg_postprocessor.c.

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

void down_scale(jpegd_postprocess_func_param_t *postprocess)
{

    /** =====================================================================
     * i is a general counter for the "for" loop,
     * x is the counter when processing the image horizontally,
     * y is the counter when processing the image vertically
     * ====================================================================*/
    uint32_t i, x, y;

    uint32_t nHCount;
    uint32_t nHStep;
    uint32_t nHIndex;

    uint32_t nHLumaAccum;
    uint32_t nHChromaAccum1;
    uint32_t nHChromaAccum2;

    uint8_t fHorizontalChroma;

    // Need a signed variable for the negative increment in the 270 cases
    int32_t nIncrement270;

    for (y = 0; y < postprocess->nRowsToProcess; y++)
    {
        //Downscaling the lines first, in horizontal direction
        nHCount = 0;
        nHLumaAccum = 0;
        nHChromaAccum1 = 0;
        nHChromaAccum2 = 0;
        nHStep = 1;
        nHIndex = 0;

        fHorizontalChroma = true;

        for ( x = 0; x < postprocess->nColsToProcess; x++ )
        {
            nHCount += postprocess->p_processor->nOutputWidth;

            if ( nHCount < postprocess->p_processor->nInputWidth)
            {
                //Luma
                nHLumaAccum += (uint32_t)(postprocess->pSrcLuma[x]);

                //Chroma

                /** ======================================================
                * After down scaling, the Chroma portion needs to be
                * down sampled.
                *
                * The "fHorizontalChroma" variable will be true for every other
                * Chroma pair in horizontal direction, this results a
                * factor 2 down sampling for the output.
                *
                * Note this only works for H2V1 and H2V2 formats,
                * i.e. horizontally factor 2 down sampled Chroma
                * components.
                * =======================================================*/

                if (fHorizontalChroma)
                {
                    /** ====================================================================
                    * This is the technique to process input H2V1 or H2V2 Chroma.
                    * Two horizontal adjacent Luma pixels share a Chroma pair:
                    *
                    *                  Y0   Y1   |   Y2  Y3  |  Y4  Y5
                    *                  U0   V0   |   U1  V1  |  U2  V2
                    *
                    *  Luma index:      x         <-- when index "x" is at even locations
                    *  Chroma index:    x   x+1
                    *
                    *  Luma index:           x    <-- when index "x" is at odd locations
                    *  Chroma index:   x-1   x
                    *
                    * This ensures that each Chroma pair be used twice for adacent Luma
                    * pixels, it is equivalent to horizontally up sampling the Chroma by 2:
                    *
                    *     Y0       Y1    |    Y2       Y3    |    Y4       Y5
                    *   U0  V0   U0  V0  |  U1  V1   U1  V1  |  U2  V2   U2  V2
                    *
                    * This only works for H2V1 and H2V2 formats.
                    * =====================================================================*/

                    if (!(x & 0x01)) // x is at even location
                    {
                        nHChromaAccum1 += (uint32_t)(postprocess->pSrcChroma[x    ]);
                        nHChromaAccum2 += (uint32_t)(postprocess->pSrcChroma[x + 1]);
                    }
                    else             // x is at odd location
                    {
                        nHChromaAccum1 += (uint32_t)(postprocess->pSrcChroma[x - 1]);
                        nHChromaAccum2 += (uint32_t)(postprocess->pSrcChroma[x    ]);
                    }
                }
                nHStep++;
            }
            else
            {
                //Average the line Horizontally by dividing nHStep,
                //and temporarily save the processed value in pLnBufLuma

                //Luma
                postprocess->p_processor->pLumaAccum[nHIndex] +=
                    (nHLumaAccum + (uint32_t)(postprocess->pSrcLuma[x]))/nHStep;

                //Chroma
                if (fHorizontalChroma)
                {
                    if (!(x & 0x01))  // x is at even location
                    {
                        postprocess->p_processor->pChromaAccum[nHIndex] +=
                            (nHChromaAccum1 + (uint32_t)(postprocess->pSrcChroma[x]))/nHStep;
                        postprocess->p_processor->pChromaAccum[nHIndex + 1] +=
                            (nHChromaAccum2 + (uint32_t)(postprocess->pSrcChroma[x + 1]))/nHStep;
                    }
                    else              // x is at odd location
                    {
                        postprocess->p_processor->pChromaAccum[nHIndex] +=
                            (nHChromaAccum1 + (uint32_t)(postprocess->pSrcChroma[x - 1]))/nHStep;
                        postprocess->p_processor->pChromaAccum[nHIndex + 1] +=
                            (nHChromaAccum2 + (uint32_t)(postprocess->pSrcChroma[x]))/nHStep;
                    }
                }

                //Flip "fHorizontalChroma" after outputing a CbCr pair so that
                //every other output pair is saved. The reason is that
                //the Chroma is Horizontally subsampled by 2 for H2V1
                //and H2V2 formats.
                fHorizontalChroma = (~fHorizontalChroma) & 1; //Flipping

                nHIndex++;

                nHLumaAccum = 0;
                nHChromaAccum1 = 0;
                nHChromaAccum2 = 0;
                nHCount -= postprocess->p_processor->nInputWidth;
                nHStep = 1;
            }
        }

        /** ================================================================
        * Now it is the time to process the data vertically.
        *
        * An input line has been processed, so move the source Luma line
        * pointer to the next buffer line.
        *
        * If the input format is H2V1, meaning there is no Vertical Chroma
        * down sampling, then move the source Chroma line pointer to the
        * next buffer line as well.
        *
        * If the input format is H2V2, meaning there is factor 2 Vertical
        * Chroma down sampling, then move the Chroma line pointer every
        * other time, i.e. move it when the index "y" is odd.
        * This is equivalent to vertically upsampling the Chroma by 2.
        *
        * =================================================================*/

        postprocess->pSrcLuma += postprocess->p_processor->chunk_width;

        if (postprocess->p_processor->inputSubsampling == JPEG_H2V1)
        {
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
        }

        else if ((postprocess->p_processor->inputSubsampling == JPEG_H2V2) && (y & 0x01))
        {
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
        }

        postprocess->p_processor->nVCount += postprocess->p_processor->nOutputHeight;

        /** ==================================================================
        * Check if the input height accumulation is less than the output
        * height. If so, increment the vertical step size by 1; otherwise
        * output a line.
        * ===================================================================*/
        if ( postprocess->p_processor->nVCount < postprocess->p_processor->nInputHeight)
        {
            postprocess->p_processor->nVStep++;
        }
        else
        {
            switch (postprocess->p_processor->down_scale)
            {
            case DS_H2V2:

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                    {
                        /*****************************************************
                        * When it reaches the point to output a line:
                        *
                        * For the Luma component, output the processed line,
                        * and move the output Luma pointer to the next line.
                        *
                        * For the Chroma components, if the output format is
                        * H2V2, we need to vertically down sample the Chroma
                        * by 2. This is done by flipping the *newChromaV
                        * variable each time a Chroma line is outputed, then
                        * output a Chroma line and move the output Chroma
                        * pointer to the next line when *newChromaV is ture.
                        *****************************************************/

                        //Average the values vertically and save the results
                        //in postProcess->pDstLuma, the output.

                        /*lint -e{414}: Not possible to have division by zero
                        * because m_outputFormat.nHeight will always be
                        * smaller than m_inputFormat.nHeight so m_nVStep
                        * would already be at least incremented once */
                        postprocess->pDstLuma[i] =
                            (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[i] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma += postprocess->p_processor->nStride;
                }
                else
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                    {
                        //Average the values vertically and save the results
                        //in postProcess->pDstLuma, the output.
                        postprocess->pDstLuma[i] =
                           (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                    }
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma += postprocess->p_processor->nStride;


                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.
                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V1:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    postprocess->pDstLuma[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->pDstChroma[i] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma   += postprocess->p_processor->nStride;
                postprocess->pDstChroma += postprocess->p_processor->nStride;

                break;


            case DS_H2V2_CbCr:

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        /*****************************************************
                        * When it reaches the point to output a line:
                        *
                        * For the Luma component, output the processed line,
                        * and move the output Luma pointer to the next line.
                        *
                        * For the Chroma components, if the output format is
                        * H2V2, we need to vertically down sample the Chroma
                        * by 2. This is done by flipping the *newChromaV
                        * variable each time a Chroma line is outputed, then
                        * output a Chroma line and move the output Chroma
                        * pointer to the next line when *newChromaV is ture.
                        *****************************************************/

                        //Average the values vertically and save the results
                        //in postProcess->pDstLuma, the output.

                        /*lint -e{414}: Not possible to have division by zero
                        * because m_outputFormat.nHeight will always be
                        * smaller than m_inputFormat.nHeight so m_nVStep
                        * would already be at least incremented once */
                        postprocess->pDstLuma[i] =
                            (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                        postprocess->pDstLuma[i+1] =
                            (uint8_t)(postprocess->p_processor->pLumaAccum[i+1]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[i] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[i+1] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma += postprocess->p_processor->nStride;
                }
                else
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                    {
                        //Average the values vertically and save the results
                        //in postProcess->pDstLuma, the output.
                        postprocess->pDstLuma[i] =
                           (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                    }
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma += postprocess->p_processor->nStride;


                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.
                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V1_CbCr:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    postprocess->pDstLuma[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                    postprocess->pDstLuma[i+1] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i+1]/postprocess->p_processor->nVStep);

                    postprocess->pDstChroma[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);
                    postprocess->pDstChroma[i+1] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma   += postprocess->p_processor->nStride;
                postprocess->pDstChroma += postprocess->p_processor->nStride;

                break;


            case DS_RGB565:

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgb565line(postprocess->p_processor->pTempLumaLine,
                                postprocess->p_processor->pTempChromaLine,
                                (postprocess->pDstRGB),
                                postprocess->p_processor->nOutputWidth);

                (postprocess->pDstRGB) += postprocess->p_processor->nStride;

                break;


            case DS_RGB888:

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgb888line(postprocess->p_processor->pTempLumaLine,
                                postprocess->p_processor->pTempChromaLine,
                                (postprocess->pDstRGB),
                                postprocess->p_processor->nOutputWidth);

                (postprocess->pDstRGB) += postprocess->p_processor->nStride;

                break;


            case DS_RGBa:

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                    (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                    (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgbaline(postprocess->p_processor->pTempLumaLine,
                              postprocess->p_processor->pTempChromaLine,
                              (postprocess->pDstRGB),
                              postprocess->p_processor->nOutputWidth);

                (postprocess->pDstRGB) += postprocess->p_processor->nStride;

                break;


            /*************************************************************
             *  90 degree rotation cases
             *************************************************************/

            case DS_H2V2_R90:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    postprocess->pDstLuma[i*postprocess->p_processor->nStride] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma--;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        postprocess->pDstChroma[(i/2)*postprocess->p_processor->nStride] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[(i/2)*postprocess->p_processor->nStride+1] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma -= 2;

                }


                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V1_R90:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    postprocess->pDstLuma[i*postprocess->p_processor->nStride] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma--;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        postprocess->pDstChroma[i*postprocess->p_processor->nStride] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[i*postprocess->p_processor->nStride+1] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[(i+1)*postprocess->p_processor->nStride] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[(i+1)*postprocess->p_processor->nStride+1] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma -= 2;

                }

                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V2_R90_CbCr:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    postprocess->pDstLuma[i*postprocess->p_processor->nStride] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma--;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        postprocess->pDstChroma[(i/2)*postprocess->p_processor->nStride] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[(i/2)*postprocess->p_processor->nStride+1] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma -= 2;

                }


                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V1_R90_CbCr:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    postprocess->pDstLuma[i*postprocess->p_processor->nStride] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma--;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        postprocess->pDstChroma[i*postprocess->p_processor->nStride] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[i*postprocess->p_processor->nStride+1] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[(i+1)*postprocess->p_processor->nStride] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma[(i+1)*postprocess->p_processor->nStride+1] =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma -= 2;

                }

                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_RGB565_R90:

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgb565line_rot(postprocess->p_processor->pTempLumaLine,
                                    postprocess->p_processor->pTempChromaLine,
                                    (postprocess->pDstRGB),
                                    postprocess->p_processor->nOutputWidth,
                                    postprocess->p_processor->nStride);

                (postprocess->pDstRGB) -= 2;

                break;


            case DS_RGB888_R90:

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgb888line_rot(postprocess->p_processor->pTempLumaLine,
                                    postprocess->p_processor->pTempChromaLine,
                                    (postprocess->pDstRGB),
                                    postprocess->p_processor->nOutputWidth,
                                    postprocess->p_processor->nStride);

                (postprocess->pDstRGB) -= 3;

                break;


            case DS_RGBa_R90:

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgbaline_rot(postprocess->p_processor->pTempLumaLine,
                                  postprocess->p_processor->pTempChromaLine,
                                  (postprocess->pDstRGB),
                                  postprocess->p_processor->nOutputWidth,
                                  postprocess->p_processor->nStride);

                (postprocess->pDstRGB) -= 4;

                break;


            /*************************************************************
             *  180 degree rotation cases
             *************************************************************/

            case DS_H2V2_R180:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                     *(postprocess->pDstLuma--) =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma -= postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        *postprocess->pDstChroma =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma+1) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma -= 2;
                    }

                    postprocess->pDstChroma -= postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth;
                }


                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V1_R180:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    *(postprocess->pDstLuma--) =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma -= postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth;

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                {
                    *postprocess->pDstChroma =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                    *(postprocess->pDstChroma+1) =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                    postprocess->pDstChroma -= 2;
                }

                postprocess->pDstChroma -= postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth;

                break;


            case DS_RGB565_R180:

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgb565line_rot(postprocess->p_processor->pTempLumaLine,
                                    postprocess->p_processor->pTempChromaLine,
                                    (postprocess->pDstRGB),
                                    postprocess->p_processor->nOutputWidth,
                                    -2);

                (postprocess->pDstRGB) -= postprocess->p_processor->nStride;

                break;


            case DS_RGB888_R180:

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgb888line_rot(postprocess->p_processor->pTempLumaLine,
                                    postprocess->p_processor->pTempChromaLine,
                                    (postprocess->pDstRGB),
                                    postprocess->p_processor->nOutputWidth,
                                    -3);

                (postprocess->pDstRGB) -= postprocess->p_processor->nStride;

                break;


            case DS_RGBa_R180:

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                    (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                    (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgbaline_rot(postprocess->p_processor->pTempLumaLine,
                                  postprocess->p_processor->pTempChromaLine,
                                  (postprocess->pDstRGB),
                                  postprocess->p_processor->nOutputWidth,
                                  -4);

                (postprocess->pDstRGB) -= postprocess->p_processor->nStride;

                break;


            case DS_H2V2_R180_CbCr:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                     *(postprocess->pDstLuma--) =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma -= postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        *postprocess->pDstChroma =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma+1) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        postprocess->pDstChroma -= 2;
                    }

                    postprocess->pDstChroma -= postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth;
                }


                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V1_R180_CbCr:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    *(postprocess->pDstLuma--) =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma -= postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth;

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                {
                    *postprocess->pDstChroma =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                    *(postprocess->pDstChroma+1) =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->pDstChroma -= 2;
                }

                postprocess->pDstChroma -= postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth;

                break;



            /*************************************************************
            *  270 degree rotation cases
            *************************************************************/

            case DS_H2V2_R270:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    *(postprocess->pDstLuma - i*postprocess->p_processor->nStride) =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma++;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        *(postprocess->pDstChroma - (i/2)*postprocess->p_processor->nStride) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma - (i/2)*postprocess->p_processor->nStride+1) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma += 2;

                }

                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V1_R270:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    *(postprocess->pDstLuma - i*postprocess->p_processor->nStride) =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma++;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        *(postprocess->pDstChroma - i*postprocess->p_processor->nStride) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma - i*postprocess->p_processor->nStride+1) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma - (i+1)*postprocess->p_processor->nStride) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma - (i+1)*postprocess->p_processor->nStride+1) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma += 2;

                }

                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V2_R270_CbCr:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    *(postprocess->pDstLuma - i*postprocess->p_processor->nStride) =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma++;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        *(postprocess->pDstChroma - (i/2)*postprocess->p_processor->nStride) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma - (i/2)*postprocess->p_processor->nStride+1) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma += 2;
                }

                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_H2V1_R270_CbCr:

                for (i = 0; i < postprocess->p_processor->nOutputWidth; i++)
                {
                    //Average the values vertically and save the results
                    //in postprocess->pDstLuma, the output.
                    *(postprocess->pDstLuma - i*postprocess->p_processor->nStride) =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);
                }

                //move the output buffer pointer to the next line.
                postprocess->pDstLuma++;

                if (postprocess->p_processor->fVerticalChroma)
                {
                    for (i = 0; i < postprocess->p_processor->nOutputWidth; i += 2)
                    {
                        *(postprocess->pDstChroma - i*postprocess->p_processor->nStride) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma - i*postprocess->p_processor->nStride+1) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma - (i+1)*postprocess->p_processor->nStride) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i+1]/postprocess->p_processor->nVStep);

                        *(postprocess->pDstChroma - (i+1)*postprocess->p_processor->nStride+1) =
                            (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                    }

                    postprocess->pDstChroma += 2;

                }

                //Only flip it when the output Chroma is Vertically
                //subsampled by 2. For no Vertically subsampled images,
                //do not flip it so that each output line is saved.

                postprocess->p_processor->fVerticalChroma = (~postprocess->p_processor->fVerticalChroma) & 1;  //Flipping

                break;


            case DS_RGB565_R270:

                nIncrement270 = (int32_t)(0 - postprocess->p_processor->nStride);

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgb565line_rot(postprocess->p_processor->pTempLumaLine,
                                    postprocess->p_processor->pTempChromaLine,
                                    (postprocess->pDstRGB),
                                    postprocess->p_processor->nOutputWidth,
                                    nIncrement270);

                (postprocess->pDstRGB) += 2;

                break;


            case DS_RGB888_R270:

                nIncrement270 = (int32_t)(0 - postprocess->p_processor->nStride);

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgb888line_rot(postprocess->p_processor->pTempLumaLine,
                                    postprocess->p_processor->pTempChromaLine,
                                    (postprocess->pDstRGB),
                                    postprocess->p_processor->nOutputWidth,
                                    nIncrement270);

                (postprocess->pDstRGB) += 3;

                break;


            case DS_RGBa_R270:

                nIncrement270 = (int32_t)(0 - postprocess->p_processor->nStride);

                for (i=0; i<postprocess->p_processor->nOutputWidth; i++)
                {
                    postprocess->p_processor->pTempLumaLine[i] =
                        (uint8_t)(postprocess->p_processor->pLumaAccum[i]/postprocess->p_processor->nVStep);

                    postprocess->p_processor->pTempChromaLine[i] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[i]/postprocess->p_processor->nVStep);
                }

                /*******************************************************
                * If the output width is odd, the last Chroma(Cr or Cb)
                * is missing. The following statement recovers it.
                * Notice that the output width is actually the array
                * index + 1.
                *******************************************************/
                if (postprocess->p_processor->nOutputWidth & 0x01)
                {
                    postprocess->p_processor->pTempChromaLine[postprocess->p_processor->nOutputWidth] =
                        (uint8_t)(postprocess->p_processor->pChromaAccum[postprocess->p_processor->nOutputWidth]/postprocess->p_processor->nVStep);
                }

                y2vu2rgbaline_rot(postprocess->p_processor->pTempLumaLine,
                                  postprocess->p_processor->pTempChromaLine,
                                  (postprocess->pDstRGB),
                                  postprocess->p_processor->nOutputWidth,
                                  nIncrement270);

                (postprocess->pDstRGB) += 4;

                break;

            default:
                break;
            }

            //reset the line buffer, make it contains all 0s.
            STD_MEMSET(postprocess->p_processor->pLumaAccum, 0,
                       postprocess->p_processor->nOutputWidth * sizeof(uint32_t));

            STD_MEMSET(postprocess->p_processor->pChromaAccum, 0,
                       2 * postprocess->p_processor->nOutputWidth * sizeof(uint32_t));

            postprocess->p_processor->nVCount -= postprocess->p_processor->nInputHeight;
            postprocess->p_processor->nVStep = 1;

        } //end else

    }//end  for (y=0; y < postprocess->nRowsToProcess; y++)

}

