/*========================================================================


*//** @file jpeg_postprocess_yuv2rgb.c

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
10/15/09   mingy   Added ARM_ARCH_7A definition so that ArmV6 won't
                   pick it up.
09/21/09   mingy   Added odd sized image support.
04/22/09   mingy   Created file.
                   Branch out YUV2RGB from jpeg_postprocessor.c.

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


#ifndef ARM_ARCH_7A
/**************************************************************************
 * Convert YVU line to RGB 565 line. (Chroma H2 to RGB565)
 *************************************************************************/
void y2vu2rgb565line(uint8_t * pLumaLine,
                     uint8_t * pChromaLine,
                     uint8_t * pRGB565Line,
                     uint32_t  nLineWidth)
{
    uint32_t i;

    int32_t C, D, E;
    int32_t subChromaR, subChromaG, subChromaB, subLuma;
    uint8_t  R, G, B;

    /***********************************************************************
    * Notice that if nLineWidth is odd, the for loop will execute
    * one more time than when nLineWidth is even.
    ************************************************************************/
    for (i = 0; i < nLineWidth - 2; i += 2)
    {
        C = *pLumaLine      - 16;
        D = *(pChromaLine+1)- 128;
        E = *pChromaLine    - 128;

        subChromaR =            409*E + 128;
        subChromaG = (-100)*D - 208*E + 128;
        subChromaB =            516*D + 128;

        subLuma = 298*C;

        B = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
        G = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        R = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));

        *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
        *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);

        // second pixel, re-use subChroma
        C            = *(pLumaLine+1) - 16;
        subLuma      = 298*C;
        pRGB565Line += 2;

        B = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
        G = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        R = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));


        *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
        *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);

        pLumaLine   += 2;
        pChromaLine += 2;
        pRGB565Line += 2;
}

/**************************************************************************
    * Since the above for loop executes one more time in case of
    * odd nLineWidth, thre is only one pixel, the very last Y (odd case) left.
    * So process only this single pixel for the odd width case.
    **************************************************************************/
        C = *pLumaLine      - 16;
        D = *(pChromaLine+1)- 128;
        E = *pChromaLine    - 128;

        subChromaR =            409*E + 128;
        subChromaG = (-100)*D - 208*E + 128;
        subChromaB =            516*D + 128;

        subLuma = 298*C;

        R = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));
        G = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        B = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));

        *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
        *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);

    /*************************************************************************
    * Execute the following statements only when nLineWidth is even.
    *************************************************************************/
    if (!(nLineWidth & 0x01))
    {
        C = *(pLumaLine+1) - 16;
        subLuma = 298*C;
        pRGB565Line += 2;

        R = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));
        G = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        B = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));

        *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
        *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);
    }
}
#endif /* !ARM_ARCH_7A */


#ifndef ARM_ARCH_7A
/**************************************************************************
 * Convert YVU line to RGB 888 line.
 *************************************************************************/
void y2vu2rgb888line(uint8_t * pLumaLine,
                   uint8_t * pChromaLine,
                   uint8_t * pRGB888Line,
                   uint32_t nLineWidth)
{
    uint32_t i;

    int32_t  C, D, E;
    int32_t  subChromaR, subChromaG, subChromaB, subLuma;

    /***********************************************************************
    * Notice that if nLineWidth is odd, the for loop will execute
    * one more time than when nLineWidth is even.
    ************************************************************************/
    for (i = 0; i < nLineWidth - 2; i += 2)
    {
        // first pixl
        C = *pLumaLine      - 16;   // Y
        D = *(pChromaLine+1)- 128;  // Cb
        E = *pChromaLine    - 128;  // Cr

        subChromaB =            516*D + 128;
        subChromaG = (-100)*D - 208*E + 128;
        subChromaR =            409*E + 128;

        subLuma = 298*C;

        *pRGB888Line     = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGB888Line+1) = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGB888Line+2) = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));

        // second pixel, re-use subChroma
        C            = *(pLumaLine+1) -16;
        subLuma      = 298*C;
        pRGB888Line += 3;

        *pRGB888Line     = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGB888Line+1) = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGB888Line+2) = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));

        // update the pointers
        pLumaLine   += 2;
        pChromaLine += 2;
        pRGB888Line += 3;
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

    *pRGB888Line     = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
    *(pRGB888Line+1) = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
    *(pRGB888Line+2) = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));

    /*************************************************************************
    * Execute the following statements only when nLineWidth is even.
    *************************************************************************/
    if (!(nLineWidth & 0x01))
    {
        C            = *(pLumaLine+1) -16;
        subLuma      = 298*C;
        pRGB888Line += 3;

        *pRGB888Line     = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGB888Line+1) = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGB888Line+2) = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));
    }
}
#endif /* !ARM_ARCH_7A */


#ifndef ARM_ARCH_7A
/**************************************************************************
 * Convert YVU line to RGBa line.
 *************************************************************************/
void y2vu2rgbaline(uint8_t * pLumaLine,
                     uint8_t * pChromaLine,
                   uint8_t * pRGBaLine,
                     uint32_t nLineWidth)
{
    uint32_t i;

    int32_t  C, D, E;
    int32_t  subChromaR, subChromaG, subChromaB, subLuma;

    /***********************************************************************
    * Notice that if nLineWidth is odd, the for loop will execute
    * one more time than when nLineWidth is even.
    ************************************************************************/
    for (i = 0; i < nLineWidth - 2; i += 2)
    {

        // first pixl
        C = *pLumaLine      - 16;
        D = *(pChromaLine+1)- 128;
        E = *pChromaLine    - 128;

        subChromaB =            516*D + 128;
        subChromaG = (-100)*D - 208*E + 128;
        subChromaR =            409*E + 128;


        subLuma = 298*C;

        *pRGBaLine     = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGBaLine+1) = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGBaLine+2) = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));
        *(pRGBaLine+3) = 0xFF;

        // second pixel, re-use subChroma
        C            = *(pLumaLine+1) -16;
        subLuma      = 298*C;
        pRGBaLine   += 4;

        *pRGBaLine     = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGBaLine+1) = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGBaLine+2) = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));
        *(pRGBaLine+3) = 0xFF;

        // update the pointers
        pLumaLine   += 2;
        pChromaLine += 2;
        pRGBaLine   += 4;
    }

    /**************************************************************************
    * Since the above for loop executes one more time in case of
    * odd nLineWidth, thre is only one pixel, the very last Y (odd case) left.
    * So process only this single pixel for the odd width case.
    **************************************************************************/
    C = *pLumaLine      - 16;
    D = *(pChromaLine+1)- 128;
    E = *pChromaLine    - 128;

    subChromaR =            409*E + 128;
    subChromaG = (-100)*D - 208*E + 128;
    subChromaB =    516*D         + 128;

    subLuma = 298*C;

    *pRGBaLine     = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
    *(pRGBaLine+1) = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
    *(pRGBaLine+2) = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));
    *(pRGBaLine+3) = 0xFF;

    /*************************************************************************
    * Execute the following statements only when nLineWidth is even.
    *************************************************************************/
    if (!(nLineWidth & 0x01))
    {
        C          = *(pLumaLine+1) -16;
        subLuma    = 298*C;
        pRGBaLine += 4;

        *pRGBaLine     = (uint8_t) (_CLAMP ((subLuma + subChromaB) >> 8));
        *(pRGBaLine+1) = (uint8_t) (_CLAMP ((subLuma + subChromaG) >> 8));
        *(pRGBaLine+2) = (uint8_t) (_CLAMP ((subLuma + subChromaR) >> 8));
        *(pRGBaLine+3) = 0xFF;
    }
}
#endif /* !ARM_ARCH_7A */


#ifndef ARM_ARCH_7A
/**************************************************************************
 * Convert YVU line (No horizontal chroma subsampling) to RGB 565 line.
 *************************************************************************/
void y1vu2rgb565line(uint8_t * pLumaLine,
                     uint8_t * pChromaLine,
                     uint8_t * pRGB565Line,
                     uint32_t nLineWidth)
{
    uint32_t i;
    int32_t  C, D, E;
    uint8_t  R, G, B;

    for (i = 0; i < nLineWidth; i++)
    {
        C = *pLumaLine      - 16;
        D = *(pChromaLine+1)- 128;
        E = *pChromaLine    - 128;

        B = (uint8_t) (_CLAMP(( 298 * C + 516 * D           + 128) >> 8));
        G = (uint8_t) (_CLAMP(( 298 * C - 100 * D - 208 * E + 128) >> 8));
        R = (uint8_t) (_CLAMP(( 298 * C           + 409 * E + 128) >> 8));

        *pRGB565Line     = ((G << 3) & 0xE0) | (B >> 3);
        *(pRGB565Line+1) = ( R       & 0xF8) | (G >> 5);

        // update pointers
        pLumaLine   += 1;
        pChromaLine += 2;
        pRGB565Line += 2;
    }
}
#endif /* !ARM_ARCH_7A */


#ifndef ARM_ARCH_7A
/**************************************************************************
 * Convert YVU line (No horizontal chroma subsampling) to RGB 888 line.
 *************************************************************************/
void y1vu2rgb888line(uint8_t * pLumaLine,
                   uint8_t * pChromaLine,
                   uint8_t * pRGB888Line,
                   uint32_t nLineWidth)
{
    uint32_t i;
    int32_t  C, D, E;

    for ( i = 0; i < nLineWidth; i++)
    {
        C = *pLumaLine      - 16;
        D = *(pChromaLine+1)- 128;
        E = *pChromaLine    - 128;

        *pRGB888Line     = (uint8_t) (_CLAMP(( 298 * C + 516 * D           + 128) >> 8));
        *(pRGB888Line+1) = (uint8_t) (_CLAMP(( 298 * C - 100 * D - 208 * E + 128) >> 8));
        *(pRGB888Line+2) = (uint8_t) (_CLAMP(( 298 * C           + 409 * E + 128) >> 8));

        // update the pointers
        pLumaLine   += 1;
        pChromaLine += 2;
        pRGB888Line += 3;
    }
}
#endif /* !ARM_ARCH_7A */


#ifndef ARM_ARCH_7A
/**************************************************************************
 * Convert YVU line (No horizontal chroma subsampling) to RGBa line.
 *************************************************************************/
void y1vu2rgbaline(uint8_t * pLumaLine,
                   uint8_t * pChromaLine,
                   uint8_t * pRGBaLine,
                   uint32_t nLineWidth)
{
    uint32_t i;
    int32_t  C, D, E;

    for ( i = 0; i < nLineWidth; i++)
    {
        C = *pLumaLine      - 16;
        D = *(pChromaLine+1)- 128;
        E = *pChromaLine    - 128;

        *pRGBaLine     = (uint8_t) (_CLAMP(( 298 * C + 516 * D           + 128) >> 8));
        *(pRGBaLine+1) = (uint8_t) (_CLAMP(( 298 * C - 100 * D - 208 * E + 128) >> 8));
        *(pRGBaLine+2) = (uint8_t) (_CLAMP(( 298 * C           + 409 * E + 128) >> 8));
        *(pRGBaLine+3) = 0xFF;

        // update the pointers
        pLumaLine   += 1;
        pChromaLine += 2;
        pRGBaLine   += 4;
    }
}
#endif /* !ARM_ARCH_7A */
