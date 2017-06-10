/*========================================================================


*//** @file jpeg_postprocess_dm.c

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
10/16/09   mingy   Fixed the issue for data moving from H2V1 to H2V2 when
                   1/8 scalable decoding is performed. In this case there
                   is only 1 Luma line and 1 Chroma line in the input
                   MCU Row buffer.
09/21/09   mingy   Changed the name of the post process function.
                   parameter structure.
04/22/09   mingy   Created file.
                   Branch out data move from jpeg_postprocessor.c.

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
*                  Data Move Function  0 Degree Rotation
*
* DATA MOVE will copy input (the MCU Row buffer filled by the engine)
* to the final output buffer and remove padding and color subsampling
**************************************************************************/
void data_move(jpegd_postprocess_func_param_t *postprocess)
{
    /**************************************************************************
    *  The source and destination addresses need to be updated and pass back
    *  to the caller function, so double pointer is used here.
    *  postprocess->pSrcLuma is the address of the source Luma (the pointer to the uint8
    *  Luma pixel data).
    **************************************************************************/
    uint32_t x, y; // Loop Counters

    postprocess->pDstRGB = NULL;

    switch (postprocess->p_processor->data_move)
    {
    case H2V2_TO_H2V2:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstChroma, postprocess->pSrcChroma, postprocess->nColsToProcess);
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;


    case H2V2_TO_H2V1:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstChroma, postprocess->pSrcChroma, postprocess->nColsToProcess);
            postprocess->pDstChroma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstChroma, postprocess->pSrcChroma, postprocess->nColsToProcess);
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;


    case H2V1_TO_H2V2:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                STD_MEMMOVE(postprocess->pDstChroma, postprocess->pSrcChroma, postprocess->nColsToProcess);
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
                postprocess->pDstChroma += postprocess->p_processor->nStride;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma += postprocess->p_processor->nStride;

                STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma += postprocess->p_processor->nStride;

                STD_MEMMOVE(postprocess->pDstChroma, postprocess->pSrcChroma, postprocess->nColsToProcess);
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
                postprocess->pDstChroma += postprocess->p_processor->nStride;
            }
        }

        break;


    case H2V1_TO_H2V1:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstChroma, postprocess->pSrcChroma, postprocess->nColsToProcess);
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;


    case H1V2_TO_H2V2:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            // Subsample horizontally
            for (x = 0; x < postprocess->nColsToProcess; x+=2)
            {
                postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x+1];
            }

            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;


    case H1V2_TO_H2V1:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            // Subsample horizontally
            for (x = 0; x < postprocess->nColsToProcess; x+=2)
            {
                postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x+1];
            }
            // Resample vertically
            STD_MEMMOVE (postprocess->pDstChroma + postprocess->p_processor->nStride,
                postprocess->pDstChroma, postprocess->nColsToProcess);

            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += postprocess->p_processor->nStride << 1;
        }

        break;


    case H1V1_TO_H2V2:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                // Subsample horizontally
                for (x = 0; x < postprocess->nColsToProcess; x += 2)
                {
                    postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x];
                    postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x+1];
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma += postprocess->p_processor->nStride;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma += postprocess->p_processor->nStride;

                STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma += postprocess->p_processor->nStride;

                // Subsample horizontally
                for (x = 0; x < postprocess->nColsToProcess; x += 2)
                {
                    postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x];
                    postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x+1];
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma += postprocess->p_processor->nStride;
            }
        }

        break;


    case H1V1_TO_H2V1:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            // subsample horizontally
            for (x = 0; x < postprocess->nColsToProcess; x+=2)
            {
                postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x+1];
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;


    case H2V2_TO_H2V2_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            for (x = 0; x < postprocess->nColsToProcess; x += 2)
            {
                postprocess->pDstChroma[x] = postprocess->pSrcChroma[x+1];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[x];
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;


    case H2V2_TO_H2V1_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            for (x = 0; x < postprocess->nColsToProcess; x += 2)
            {
                postprocess->pDstChroma[x] = postprocess->pSrcChroma[x+1];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[x];
            }
            postprocess->pDstChroma += postprocess->p_processor->nStride;

            for (x = 0; x < postprocess->nColsToProcess; x += 2)
            {
                postprocess->pDstChroma[x] = postprocess->pSrcChroma[x+1];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[x];
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;


    case H2V1_TO_H2V2_CbCr:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (x = 0; x < postprocess->nColsToProcess; x += 2)
                {
                    postprocess->pDstChroma[x] = postprocess->pSrcChroma[x+1];
                    postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[x];
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
                postprocess->pDstChroma += postprocess->p_processor->nStride;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma += postprocess->p_processor->nStride;

                STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma += postprocess->p_processor->nStride;

                for (x = 0; x < postprocess->nColsToProcess; x += 2)
                {
                    postprocess->pDstChroma[x] = postprocess->pSrcChroma[x+1];
                    postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[x];
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
                postprocess->pDstChroma += postprocess->p_processor->nStride;
            }
        }

        break;


    case H2V1_TO_H2V1_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            for (x = 0; x < postprocess->nColsToProcess; x += 2)
            {
                postprocess->pDstChroma[x] = postprocess->pSrcChroma[x+1];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[x];
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;


    case H1V2_TO_H2V2_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            // Subsample horizontally
            for (x = 0; x < postprocess->nColsToProcess; x += 2)
            {
                postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x+1];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x];
            }

            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;


    case H1V2_TO_H2V1_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            // Subsample horizontally
            for (x = 0; x < postprocess->nColsToProcess; x += 2)
            {
                postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x+1];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x];
            }
            // Resample vertically
            STD_MEMMOVE (postprocess->pDstChroma + postprocess->p_processor->nStride,
                postprocess->pDstChroma, postprocess->nColsToProcess);

            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += postprocess->p_processor->nStride << 1;
        }

        break;


    case H1V1_TO_H2V2_CbCr:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                // Subsample horizontally
                for (x = 0; x < postprocess->nColsToProcess; x += 2)
                {
                    postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x+1];
                    postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x];
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma += postprocess->p_processor->nStride;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma += postprocess->p_processor->nStride;

                STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma += postprocess->p_processor->nStride;

                // Subsample horizontally
                for (x = 0; x < postprocess->nColsToProcess; x += 2)
                {
                    postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x+1];
                    postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x];
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma += postprocess->p_processor->nStride;
            }
        }

        break;


    case H1V1_TO_H2V1_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            STD_MEMMOVE(postprocess->pDstLuma, postprocess->pSrcLuma, postprocess->nColsToProcess);
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma += postprocess->p_processor->nStride;

            // subsample horizontally
            for (x = 0; x < postprocess->nColsToProcess; x += 2)
            {
                postprocess->pDstChroma[x]   = postprocess->pSrcChroma[2*x+1];
                postprocess->pDstChroma[x+1] = postprocess->pSrcChroma[2*x];
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += postprocess->p_processor->nStride;
        }

        break;

    default:
        break;

    }
}



/**************************************************************************
*                 Data Move Function  90 Degree Rotation
*************************************************************************/
void data_move_rot90(jpegd_postprocess_func_param_t *postprocess)
{
    uint32_t i, y; // Loop Counters

    postprocess->pDstRGB = NULL;

    switch (postprocess->p_processor->data_move)
    {
    case H2V2_TO_H2V2_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma -= 2;
        }

        break;

     case H2V2_TO_H2V1_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
            }

            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma -= 2;
        }

        break;

    case H2V1_TO_H2V2_R90:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstChroma -= 2;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma--;

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma--;

                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstChroma -= 2;
            }
        }

        break;

    case H2V1_TO_H2V1_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma -= 2;
        }

        break;

    case H1V2_TO_H2V2_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma --;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma -= 2;
        }

        break;

    case H1V2_TO_H2V1_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma --;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma -= 2;
        }

        break;

    case H1V1_TO_H2V2_R90:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma -= 2;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma--;

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma--;

                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma -= 2;
            }
        }

        break;


    case H1V1_TO_H2V1_R90:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma --;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 2);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 3);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
            postprocess->pDstChroma -= 2;
        }

        break;


    case H2V2_TO_H2V2_R90_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma -= 2;
        }

        break;

     case H2V2_TO_H2V1_R90_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
            }

            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma -= 2;
        }

        break;

    case H2V1_TO_H2V2_R90_CbCr:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstChroma -= 2;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma--;

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma--;

                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstChroma -= 2;
            }
        }

        break;

    case H2V1_TO_H2V1_R90_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma -= 2;
        }

        break;

    case H1V2_TO_H2V2_R90_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma --;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma -= 2;
        }

        break;

    case H1V2_TO_H2V1_R90_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma --;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma -= 2;
        }

        break;

    case H1V1_TO_H2V2_R90_CbCr:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma -= 2;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma--;

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma--;

                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                    *(postprocess->pDstChroma + (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma -= 2;
            }
        }

        break;


    case H1V1_TO_H2V1_R90_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma--;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma --;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma + i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 3);
                *(postprocess->pDstChroma + (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 2);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
            postprocess->pDstChroma -= 2;
        }

        break;


    default:
        break;

    }
}


/**************************************************************************
*                Data Move Function  180 Degree Rotation
*************************************************************************/
void data_move_rot180(jpegd_postprocess_func_param_t *postprocess)
{
    uint32_t i, y; // Loop Counters

    postprocess->pDstRGB = NULL;

    switch (postprocess->p_processor->data_move)
    {
    case H2V2_TO_H2V2_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            /*********************************************************
            *  Adjust the input pointer to handle partial MCU case.
            *  postprocess->p_processor->chunk_width is the MCU Row buffer width, which
            *  was padded in case of partial MCU, while
            *  postprocess->p_processor->nOutputWidth is the actual output width.
            *  Need to increment postprocess->pSrcLuma by the difference between
            *  the MCU Row width and the actual output width to
            *  skip the paddid bits.
            *********************************************************/
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;


     case H2V2_TO_H2V1_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma -= postprocess->nColsToProcess;
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;

    case H2V1_TO_H2V2_R180:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i+=2)
                {
                    *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                    *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                    postprocess->pDstChroma     -= 2;
                }
                postprocess->pSrcChroma += postprocess->nColsToProcess;
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
                }
                postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
                postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma--)  = *(postprocess->pSrcLuma++);
                }
                postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
                postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

                for (i = 0; i < postprocess->nColsToProcess; i+=2)
                {
                    *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                    *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                    postprocess->pDstChroma     -= 2;
                }
                postprocess->pSrcChroma += postprocess->nColsToProcess;
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
            }
        }

        break;

    case H2V1_TO_H2V1_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;

    case H1V2_TO_H2V2_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                *(postprocess->pDstChroma+1) = *postprocess->pSrcChroma;
                postprocess->pSrcChroma     += 3;
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;

    case H1V2_TO_H2V1_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                *(postprocess->pDstChroma+1) = *postprocess->pSrcChroma;
                postprocess->pSrcChroma     += 3;
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma -= postprocess->nColsToProcess * 2;
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                *(postprocess->pDstChroma+1) = *postprocess->pSrcChroma;
                postprocess->pSrcChroma     += 3;
                postprocess->pDstChroma     -= 2 ;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;

    case H1V1_TO_H2V2_R180:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            // Skip the extra right part of the image in stride cases
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);


            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                 // Subsample Chroma horizontally
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                    *(postprocess->pDstChroma+1) = *postprocess->pSrcChroma;
                    postprocess->pSrcChroma     += 3;
                    postprocess->pDstChroma     -= 2;
                }
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pSrcChroma += postprocess->nColsToProcess * 2;
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pDstChroma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
                }
                // Skip the extra right part of the image in stride cases
                postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
                postprocess->pDstLuma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
                }
                postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
                postprocess->pDstLuma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);

                // Subsample Chroma horizontally
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                    *(postprocess->pDstChroma+1) = *postprocess->pSrcChroma;
                    postprocess->pSrcChroma     += 3;
                    postprocess->pDstChroma     -= 2;
                }
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pSrcChroma += postprocess->nColsToProcess * 2;
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pDstChroma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);
            }
        }

        break;


    case H1V1_TO_H2V1_R180:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                *(postprocess->pDstChroma+1) = *postprocess->pSrcChroma;
                postprocess->pSrcChroma     += 3;
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;


    case H2V2_TO_H2V2_R180_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            /*********************************************************
            *  Adjust the input pointer to handle partial MCU case.
            *  postprocess->p_processor->chunk_width is the MCU Row buffer width, which
            *  was padded in case of partial MCU, while
            *  postprocess->p_processor->nOutputWidth is the actual output width.
            *  Need to increment postprocess->pSrcLuma by the difference between
            *  the MCU Row width and the actual output width to
            *  skip the paddid bits.
            *********************************************************/
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;


     case H2V2_TO_H2V1_R180_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma -= postprocess->nColsToProcess;
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;

    case H2V1_TO_H2V2_R180_CbCr:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                    *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                    postprocess->pDstChroma     -= 2;
                }
                postprocess->pSrcChroma += postprocess->nColsToProcess;
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
                }
                postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
                postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
                }
                postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
                postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                    *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                    postprocess->pDstChroma     -= 2;
                }
                postprocess->pSrcChroma += postprocess->nColsToProcess;
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
            }
        }

        break;

    case H2V1_TO_H2V1_R180_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                *postprocess->pDstChroma     = *(postprocess->pSrcChroma++);
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;

    case H1V2_TO_H2V2_R180_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                *postprocess->pDstChroma     = *postprocess->pSrcChroma;
                postprocess->pSrcChroma     += 3;
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;

    case H1V2_TO_H2V1_R180_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                *postprocess->pDstChroma     = *postprocess->pSrcChroma;
                postprocess->pSrcChroma     += 3;
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma -= postprocess->nColsToProcess * 2;
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                *postprocess->pDstChroma     = *postprocess->pSrcChroma;
                postprocess->pSrcChroma     += 3;
                postprocess->pDstChroma     -= 2 ;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;

    case H1V1_TO_H2V2_R180_CbCr:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            // Skip the extra right part of the image in stride cases
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);


            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                 // Subsample Chroma horizontally
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                    *postprocess->pDstChroma     = *postprocess->pSrcChroma;
                    postprocess->pSrcChroma     += 3;
                    postprocess->pDstChroma     -= 2;
                }
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pSrcChroma += postprocess->nColsToProcess * 2;
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pDstChroma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
                }
                // Skip the extra right part of the image in stride cases
                postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
                postprocess->pDstLuma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
                }
                postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
                postprocess->pDstLuma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);

                // Subsample Chroma horizontally
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                    *postprocess->pDstChroma     = *postprocess->pSrcChroma;
                    postprocess->pSrcChroma     += 3;
                    postprocess->pDstChroma     -= 2;
                }
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pSrcChroma += postprocess->nColsToProcess * 2;
                postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth) * 2;
                postprocess->pDstChroma -= (postprocess->p_processor->nStride     - postprocess->p_processor->nOutputWidth);
            }
        }

        break;


    case H1V1_TO_H2V1_R180_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y++)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma--) = *(postprocess->pSrcLuma++);
            }
            postprocess->pSrcLuma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstLuma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma+1) = *(postprocess->pSrcChroma++);
                *postprocess->pDstChroma     = *postprocess->pSrcChroma;
                postprocess->pSrcChroma     += 3;
                postprocess->pDstChroma     -= 2;
            }
            postprocess->pSrcChroma += (postprocess->p_processor->chunk_width - postprocess->p_processor->nOutputWidth);
            postprocess->pDstChroma -= (postprocess->p_processor->nStride - postprocess->p_processor->nOutputWidth);
        }

        break;


    default:
        break;

    }
}

/**************************************************************************
*               Data Move Function  270 Degree Rotation
*************************************************************************/
void data_move_rot270(jpegd_postprocess_func_param_t *postprocess)
{
    uint32_t i, y; // Loop Counters

    postprocess->pDstRGB = NULL;

    switch (postprocess->p_processor->data_move)
    {
    case H2V2_TO_H2V2_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += 2;
        }

        break;


    case H2V2_TO_H2V1_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
            }

            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += 2;
        }

        break;


    case H2V1_TO_H2V2_R270:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i+=2)
                {
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstChroma += 2;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma++;

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma++;

                for (i = 0; i < postprocess->nColsToProcess; i+=2)
                {
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstChroma += 2;
            }
        }

        break;

    case H2V1_TO_H2V1_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride+ 1) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i + 1);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma+=2;
        }

        break;

    case H1V2_TO_H2V2_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += 2;
        }

        break;

    case H1V2_TO_H2V1_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += 2;
        }

        break;

    case H1V1_TO_H2V2_R270:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i+=2)
                {
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma += 2;
                }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma++;

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma++;

                for (i = 0; i < postprocess->nColsToProcess; i+=2)
                {
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma += 2;
            }
        }

        break;


    case H1V1_TO_H2V1_R270:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i+=2)
            {
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 2);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 3);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
            postprocess->pDstChroma += 2;
        }

        break;


    case H2V2_TO_H2V2_R270_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += 2;
        }

        break;


    case H2V2_TO_H2V1_R270_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
            }

            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma += 2;
        }

        break;


    case H2V1_TO_H2V2_R270_CbCr:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstChroma += 2;
            }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma++;

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma++;

                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
                postprocess->pDstChroma += 2;
            }
        }

        break;

    case H2V1_TO_H2V1_R270_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + i + 1);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + i);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width;
            postprocess->pDstChroma+=2;
        }

        break;

    case H1V2_TO_H2V2_R270_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += 2;
        }

        break;

    case H1V2_TO_H2V1_R270_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 1;
            postprocess->pDstChroma += 2;
        }

        break;

    case H1V1_TO_H2V2_R270_CbCr:

        // If the MCU Row contains only one line
        if (postprocess->nRowsToProcess < 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            //only copy Chroma every other time
            if (postprocess->p_processor->nRowsProcessed % 2 == 0)
            {
                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma += 2;
                }
        }
        else
        {
            for (y = 0; y < postprocess->nRowsToProcess; y += 2)
            {
                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma++;

                for (i = 0; i < postprocess->nColsToProcess; i++)
                {
                    *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
                }
                postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
                postprocess->pDstLuma++;

                for (i = 0; i < postprocess->nColsToProcess; i += 2)
                {
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                    *(postprocess->pDstChroma - (i/2) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
                }
                postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
                postprocess->pDstChroma += 2;
            }
        }

        break;


    case H1V1_TO_H2V1_R270_CbCr:

        for (y = 0; y < postprocess->nRowsToProcess; y += 2)
        {
            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i++)
            {
                *(postprocess->pDstLuma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcLuma + i);
            }
            postprocess->pSrcLuma += postprocess->p_processor->chunk_width;
            postprocess->pDstLuma++;

            for (i = 0; i < postprocess->nColsToProcess; i += 2)
            {
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 1);
                *(postprocess->pDstChroma - i * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride) = *(postprocess->pSrcChroma + 2*i + 3);
                *(postprocess->pDstChroma - (i+1) * postprocess->p_processor->nStride + 1) = *(postprocess->pSrcChroma + 2*i + 2);
            }
            postprocess->pSrcChroma += postprocess->p_processor->chunk_width << 2;
            postprocess->pDstChroma += 2;
        }

        break;


    default:
        break;
    }
}

