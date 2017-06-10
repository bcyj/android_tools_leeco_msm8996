/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    F I L E

DESCRIPTION
  This file contains the implementation of the IPL APIs.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/


/*===========================================================================
                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_util.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/05/05   babakf  Added support to get red, green, and blue histogram
10/25/05   babakf  wrote ipl_calc_luma_histogram (moved old one to attic.c)
10/25/05   babakf  added ipl_calc_avg, ipl_calc_median.
9/05/05    babakf  added 5x5 bad pixel correction. 
8/29/05    babakf  futher optimized bad pixel correct
5/15/05    babakf  added peak-signal-to-noise ratio function
4/19/05    babakf  Fixed 2 little bugs in fix_boundary
12/07/04   bf      modified ipl_make_gamma_table to use Sigmoid function
10/25/04   bf      optimized bad_pixel_correct. Old code is now in attic
10/05/04   mz      Updated functions APIs. Corrected line widths and tab 
                   spacings.
08/28/04   sj      Added ipl_convert_ycbcr420lp_to_ycbcr422 and
                   ipl_convert_ycbcr422_to_ycbcr420lp
08/24/04   sj      Added Bayer support to ipl_downsize_med
08/14/04   sj      Added ipl_downsize_med for YCbCr Downsize with 9 pt Ave
06/24/04   dle     Created ipl_crop_ycrcb420_to_rgb565.
06/20/04   sj      Added ipl_crop_ycbcr420_to_rgb666
06/11/04   sj      Modified ipl_convert_ycbcr420_to_rgb666 to
                   ipl_crop_ycbcr420_to_rgb565
05/28/04   sj      Modified ipl_convert_ycbcr420_to_rgb666 to
                   ipl_crop_ycbcr420_to_rgb666
05/27/04   sj      Added ipl_convert_ycbcr420_to_rgb666
04/15/04   mvd     Added uppack_rgb444 and 666.  Added pack_rgb 444
                   Added converts between YCbCr & RGB444.
03/13/04   sj      Added Bad Pixel Correction in BGGR and RGGB Bayer format.
01/23/04   sj      Added ipl_upsize
01/22/04   sj      Added Solarize support for RGB565 Input with RGB565 output.
12/23/03   sj      Updated bip_ptr_type to include r,g,b shifts for
                   color correction.
11/27/03   sj      Added Bayer Rotation Support and Bayer Color Conversion
                   to rgb888.
11/05/03   sj      Added IPL_BAYER_BGGR support in ipl_col_for_type
09/18/03   sj      Added pitch to ipl_icon_type
09/11/03   sj      Completed support for optimized icon_display
08/16/03   sj      Added rgb565 ipl_reflect support.  Added ipl_icon_type.
07/09/03   sj      Added IPL_RGB666 and IPL_RGB444
06/26/03   sj      Added ipl_convert_to_rgb666
06/05/03   sj      Added ipl_set_brightness and ipl_set_contrast
                   for megapixel sensor.
06/02/03   sj      Added ipl_downsize_fast.  (Use faster downsizing for 
                   thumbnail in all color formats (not just mega pixel)
05/23/03   sj      Updated bip_ptr_type to have ptrs rather than arrays.
                   Removed ipl_resize and other old code.
05/16/03   dle     Changed from BAYER_BGGR to BAYER_GBRG.
05/15/03   sj      Added ipl_image_sepia, ipl_posterize, ipl_solarize
                   and ipl_rotate90_frame
05/02/03   sj      Added ipl_conv_frame for Rel 4.0.
04/30/03   sj      Added ipl_rot_add_crop for Rel4.0.
                   Support RGB565 input.
01/16/03   sj      Added ipl_rot_add for Rel 3.0
01/03/02   sj      Added ipl_rect_type
11/05/02   sj      Modified after ISOD code review
10/28/02   sj      Added ipl_hue, ipl_sat and ipl_int to not use old buffer.
10/22/02   sj      Modfied func names to match the isod.
08/27/02   sj      Created file.
===========================================================================*/



/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipl_types.h"
#include "ipl_util.h"
#include "ipl_helper.h"


#define IPL_UTIL_DEBUG   0
#define IPL_UTIL_PROFILE 0




/*lint -save -e506, constant value boolean is totally okay */
/*lint -save -e508, extra extern is okay */
/*lint -save -e524, errors in create gamma curve are okay */
/*lint -save -e534, let me call printf in piece, god */
/*lint -save -e574, dont worry about this */
/*lint -save -e702, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e713, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e737, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e734, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e747, errors in create gamma curve are okay */
/*lint -save -e774, constant value boolean is totally okay */
/*lint -save -e818, lets not worry about const optimization right now */
/*lint -save -e834, this warning is just dumb */
/*lint -save -e826, we should look into this someday */


extern const int16 ipl2_Cr2RTable[];
extern const int16 ipl2_Cr2GTable[];
extern const int16 ipl2_Cb2GTable[];
extern const int16 ipl2_Cb2BTable[];


/*===========================================================================
                        FUNCTION DECLARATIONS
===========================================================================*/


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_fix_boundary

DESCRIPTION
    This function boundaries between transparent and non-transparent 
    regions in frames in RGB565.  After the fix, pixels along the boundary 
    that were close in color to the transparent pixels are set to be 
    transparent.  This prevents pseudo-color bleeding from the 
    transparent region.
  
    Both input and output frames must be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr            pointer to the input frame    
  o_img_ptr            pointer to the output frame
  transparentValue     16-bit transparent pixel value

RETURN VALUE
  IPL_SUCCESS          indicates operation was successful
  IPL_FAILURE          otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_fix_boundary
(
  ipl_image_type* i_img_ptr,        /* Points to the input frame  */
  ipl_image_type* o_img_ptr,        /* Points to the output frame */
  uint32 transparentValue           /* Value of transparent pixel */
)
{
  uint16 *inImgPtr, *outImgPtr;
  uint32 outWidth, outHeight;
  uint32 row, col;
  uint16 transparentR, transparentB;
  ipl_image_type * to_img_ptr;
  int thresh = 13;

  // we can do this inplace
  if (o_img_ptr == NULL)
    to_img_ptr = i_img_ptr; 
  else
    to_img_ptr = o_img_ptr; 

  //MSG_LOW("ipl_fix_boundary marker_0\n");
  
  /* Check parameters */
  if (i_img_ptr == NULL)
  {
    MSG_LOW("ipl_fix_boundary marker_200\n");
    return IPL_FAILURE;
  }

  if (i_img_ptr->cFormat != IPL_RGB565 ||
      i_img_ptr->cFormat != to_img_ptr->cFormat)
  {
    MSG_LOW("ipl_fix_boundary marker_201\n");
    return IPL_FAILURE;
  }

  if (i_img_ptr->dx != to_img_ptr->dx ||
      i_img_ptr->dy != to_img_ptr->dy)
  {
    MSG_LOW("ipl_fix_boundary marker_202\n");
    return IPL_FAILURE;
  }
    
  inImgPtr = (uint16*) i_img_ptr->imgPtr;
  outImgPtr = (uint16*) to_img_ptr->imgPtr;
  outWidth = to_img_ptr->dx;
  outHeight = to_img_ptr->dy;

  transparentR = (uint16)((transparentValue & 0xF800) >> 8);
  transparentB = (uint16)((transparentValue & 0x001F) << 3);


  /* Loop through input image */
  // this is way too strict
#if 0
  for(row = 0; row < outHeight; row++) 
  {
    for(col = 0; col < outWidth; col++) 
    {
      if ((((col < (outWidth-1)) && (*(inImgPtr + 1) == transparentValue)) || 
           ((col > 0) && ( *(inImgPtr - 1) == transparentValue)))
           && ((transparentR - ((*inImgPtr & 0xF800) >> 11)) <= 8)
           && ((transparentB - (*inImgPtr & 0x001F)) <= 8))
      {
        *outImgPtr++ = (uint16)transparentValue;
        inImgPtr++;
      }
      else
      {
        *outImgPtr++ = *inImgPtr++;
      }
    } /* end of col loop */
  } /* end of row loop */
#endif

  //MSG_LOW("ipl_fix_boundary marker_1\n");

  // see if not doing inplace
  if (to_img_ptr == o_img_ptr)
  {
    for(row = 0; row < outHeight; row++) 
    {
      for(col = 0; col < outWidth; col++) 
      {
        if ((abs(transparentR - ((*inImgPtr & 0xF800) >> 8)) < thresh) && 
            (                  (((*inImgPtr & 0x07E0) >> 3)) < thresh) &&
            (abs(transparentB - ((*inImgPtr & 0x001F) << 3)) < thresh))
        {
          *outImgPtr++ = (uint16) transparentValue;
          inImgPtr++;
        }
        else
        {
          *outImgPtr++ = *inImgPtr++;
        }
      }  
    }  
  }
  else
  {
    for(row = 0; row < outHeight * outWidth; row++) 
    {
      if ((abs(transparentR - ((*inImgPtr & 0xF800) >> 8)) < thresh) && 
          (                  (((*inImgPtr & 0x07E0) >> 3)) < thresh) &&
          (abs(transparentB - ((*inImgPtr & 0x001F) << 3)) < thresh))
      {
        *inImgPtr = (uint16) transparentValue;
      }
      inImgPtr++;
    }  
  }

  //MSG_LOW("ipl_fix_boundary marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_fix_boundary */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_align_frame

DESCRIPTION
    This function aligns frames in RGB565. 
    After alignment, no transitions between transparent and non-
    transparent pixel values will occur on odd-numbered pixels.  
    This prevents color bleeding when a YCbCr4:2:2 image is converted 
    to other formats, such as RGB565, and has a frame added.  
    Bleeding occurs because IPL functions that add frames look at 
    pairs of pixels when the input is YCbCr4:2:2 for speed purposes.  
    This causes transparent pixels to be only looked at on even 
    boundaries.  If a particular frame has transparent pixels on an 
    odd boundary, it may cause color bleeding.  To overcome this, 
    we recommend that OEMs call this function once to make sure 
    there are no odd pixel transitions.  Whenever an odd pixel
    transition is found, the transparent region is contracted.  
    Frames which don't have odd pixel transitions are left 
    untouched.
  
    Input and output frames must be in RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr          pointer to the input frame    
  o_img_ptr          pointer to the output frame
  transparentValue   16-bit transparent pixel value

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_align_frame
(
  ipl_image_type* i_img_ptr,        /* Points to the input frame  */
  ipl_image_type* o_img_ptr,        /* Points to the output frame */
  uint32 transparentValue            /* Value of transparent pixel */
)
{
  uint16 *inImgPtr, *outImgPtr;
  uint32 outWidth, outHeight;
  uint32 row, col;
  ipl_image_type * to_img_ptr;

  MSG_LOW("ipl_align_frame marker_0\n");

  // we can do this inplace
  if (o_img_ptr == NULL)
    to_img_ptr = i_img_ptr; 
  else
    to_img_ptr = o_img_ptr; 

  /* Check parameters */
  if (i_img_ptr == NULL)
  {
    MSG_LOW("ipl_align_frame marker_200\n");
    return IPL_FAILURE;
  }
  if (i_img_ptr->cFormat != IPL_RGB565 ||
      i_img_ptr->cFormat != to_img_ptr->cFormat)
  {
    MSG_LOW("ipl_align_frame marker_201\n");
    return IPL_FAILURE;
  }
  if (i_img_ptr->dx % 2 || i_img_ptr->dx != to_img_ptr->dx ||
      i_img_ptr->dy != to_img_ptr->dy)
  {
    MSG_LOW("ipl_align_frame marker_202\n");
    return IPL_FAILURE;
  }

  inImgPtr = (uint16*) i_img_ptr->imgPtr;
  outImgPtr = (uint16*) to_img_ptr->imgPtr;
  outWidth = to_img_ptr->dx;
  outHeight = to_img_ptr->dy;

  MSG_LOW("ipl_align_frame marker_1\n");
  
  if (to_img_ptr == o_img_ptr)
  {
    for(row = 0; row < outHeight; row++) 
    {
      for(col = 0; col < outWidth; col+=2) 
      {
        if (*inImgPtr == transparentValue)
        {
          if (*(inImgPtr+1) == transparentValue)
          {
            // leave both transparent 
            *outImgPtr++ = *inImgPtr++;
            *outImgPtr++ = *inImgPtr++;
          }
          else
          {
            // leave first one transparent, force second one transparent
            *outImgPtr++ = *inImgPtr;
            *outImgPtr++ = *inImgPtr++;
            inImgPtr++;
          }
        }
        else
        {
          if (*(inImgPtr+1) == transparentValue)
          {
            // leave first one not t, make second one same as first
            *outImgPtr++ = *inImgPtr;
            *outImgPtr++ = *inImgPtr++;
            inImgPtr++;
          }
          else
          {
            // leave both not transparent
            *outImgPtr++ = *inImgPtr++;
            *outImgPtr++ = *inImgPtr++;
          }
        }
      } /* end of col loop */
    } /* end of row loop */
  }
  else
  {
    for(row = 0; row < outHeight; row++) 
    {
      for(col = 0; col < outWidth; col+=2) 
      {
        if (*inImgPtr == transparentValue)
        {
          if (*(inImgPtr+1) == transparentValue)
          {
            // leave both transparent 
            inImgPtr++;
            inImgPtr++;
          }
          else
          {
            // leave first one transparent, force second one transparent
            inImgPtr++;
            *inImgPtr++ = (uint16) transparentValue;
          }
        }
        else
        {
          if (*(inImgPtr+1) == transparentValue)
          {
            // leave first one not t, make second one same as first
            inImgPtr++;
            *inImgPtr = *(inImgPtr-1);
            inImgPtr++;
          }
          else
          {
            inImgPtr++;
            inImgPtr++;
          }
        }
      } /* end of col loop */
    } 
  }

  MSG_LOW("ipl_align_frame marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_align_frame */





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_make_gamma_table

DESCRIPTION
  This function will make a gamma table for a particular gamma factor
  It will print it out to stdout

DEPENDENCIES
  None

ARGUMENTS IN
  first   power of first gamma curve (0.5 is a good start)
  last    power of first gamma curve (2.5 is a good end)
  step    increment

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_make_gamma_table(float first, float last, float step)
{
#if 0
  float x;
  float g;
  int out;
  int i;

  MSG_LOW("ipl_make_gamma_table marker_0\n");
  // are we generating 8 bit or 10 bit data?
  if (0)
  {
    // starting and stoping sigmoid curve; step
    for (g = first; g <= last; g += step) 
    {
      printf ("uint16 ipl_gammaSigmoid%.1f[256] = {\n", g);
      for (i = 0; i <= 128; i++)
      {
        x = (((float) i)/255.0);
        out = 0.5 + (pow(0.5, 1.0-g) * pow(x, g))*255.0;
        printf ("%3d", out);

        if (i != 255)
          printf (", ");
        else
          printf ("\n");

        if (i % 10 == 9)
          printf ("\n");
      }

      for (i = 129; i <= 255;i++)
      {
        x = ((float) i)/255.0;
        out = 0.5 + (1- pow(0.5, 1.0-g)*pow(1-x, g))*255.0;
        printf ("%d", out);

        if (i != 255)
          printf (", ");
        else
          printf ("\n");

        if (i % 10 == 9)
          printf ("\n");
      }
      printf ("};\n\n");
    }
  }
  else if (0)
  {
    for (g = first; g <= last; g += step) 
    {
      printf ("uint16 ipl_gammaSigmoid%.1f[1024] = {\n", g);
      for (i = 0; i <= 512; i++)
      {
        x = ((float) i)/1023.0;
        out = 0.5 + (pow(0.5, 1.0-g) * pow(x, g))*1023.0;
        printf ("%3d", out);

        if (i != 1023)
          printf (", ");
        else
          printf ("\n");

        if (i % 10 == 9)
          printf ("\n");
      }

      for (i = 513; i < 1024;i++)
      {
        x = ((float) i)/1023.0;
        out = 0.5 + (1- pow(0.5, 1.0-g)*pow(1-x, g))*1023.0;
        printf ("%d", out);

        if (i != 1023)
          printf (", ");
        else
          printf ("\n");

        if (i % 10 == 9)
          printf ("\n");
      }
      printf ("};\n\n");
    }
  }
  else
  {
    for (g = first; g <= last; g += step) 
    {
      printf ("uint8 ipl_gammaSigmoid%.1f[1024] = {\n", g);
      for (i = 0; i <= 512; i++)
      {
        x = ((float) i)/1023.0;
        out = 0.5 + (pow(0.5, 1.0-g) * pow(x, g))*255.0;
        printf ("%3d", out);

        if (i != 255)
          printf (", ");
        else
          printf ("\n");

        if (i % 10 == 9)
          printf ("\n");
      }

      for (i = 513; i < 1024;i++)
      {
        x = (float) i/1023.0;
        out = 0.5 + (1- pow(0.5, 1.0-g)*pow(1-x, g))*255.0;
        printf ("%d", out);

        if (i != 255)
          printf (", ");
        else
          printf ("\n");

        if (i % 10 == 9)
          printf ("\n");
      }
      printf ("};\n\n");
    }
  }
  MSG_LOW("ipl_make_gamma_table marker_100\n");
#else

  first = 0;
  last = 0;
  step = 0;

  first = last = step;;
  step = last++;
  step = first;
#endif

  return IPL_SUCCESS;
} /* End ipl_make_gamma_table */ //lint !e550





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_bad_pixel_correct

DESCRIPTION
  This function is used to do bad pixel correction.
  The Input Image is overwritten with the output image.
  The input should be in either IPL_BAYER_BGGR or in
  IPL_BAYER_RGGB format.
  The output will be in the same color format as input.

DEPENDENCIES
  None

ARGUMENTS IN
  img_ptr points to the input and also the output image.

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_bad_pixel_correct
(
  ipl_image_type* img_ptr          /* Points to the input image           */
)
{
  uint16 i,j;
  uint32 w,w2,h;
  uint32 cols, rows;
  uint8* p;

  int16 maxX, maxP;
  int16 minX, minP;
  int16 temph, tempv;
  int16 diff = 0;

  /* for profiling */
  int fixMax = 0;
  int fixMin = 0;
  int looked = 0;
  int qlooked = 0;
  int falseAlarm = 0;

  MSG_LOW("ipl_bad_pixel_correct marker_0\n");
  
  if (!img_ptr || !img_ptr->imgPtr)
  {
    MSG_LOW("ipl_bad_pixel_correct marker_200\n");
    return IPL_FAILURE;
  }
   /* the largest absolute difference between two pixels which would trigger
   * suspicion and further checks. Each channel has different thereshold
   * based on theory that eye is less suseptible to changes in blue, then 
   * red then green.
   */

/* These values clean up all noticeable bad pixel on our test input. They
 * get rid of WARM pixels. See test image macbethBW_1280x960_BGGR.raw 
 * 16,12, 30 seem to clean all visible bad pixels*/

#define MAX_PIX_DIFF_INC_R  16 
#define MAX_PIX_DIFF_INC_G  12 
#define MAX_PIX_DIFF_INC_B  30 
#define MAX_PIX_DIFF_DEC_R  -16
#define MAX_PIX_DIFF_DEC_G  -12 
#define MAX_PIX_DIFF_DEC_B  -30

/* These values clean up hot and cold pixels, but will allow warm pixel
 * to go through. See test image hotCold{1|2|3|4}.raw
 * Although more lenient delta on detecing cold pixels vs. values
 * above, code only run moderatly 7% faster...*/
  /*
#define MAX_PIX_DIFF_INC_R  15 
#define MAX_PIX_DIFF_INC_G  15 
#define MAX_PIX_DIFF_INC_B  25 
#define MAX_PIX_DIFF_DEC_R  -45
#define MAX_PIX_DIFF_DEC_G  -35 
#define MAX_PIX_DIFF_DEC_B  -65
  */

   /* One might think that the values defined above will work well for mid 
    * to bright pixels, but not for dark pixels, i.e. if pixel value is 40 
    * and cold pixel is 10 in blue channel, algorithm does not fix this 400% 
    * discrepency. This is okay since eye is less sensitive to changes in
    * darks than mid tones */
  
   /* Below are our inline functions that do the acutal check and fix.
    * Testing on ISO focus chart showed you must check all 8 neighboring 
    * pixels to avoid destroying thin 1-pixel wide lines from respecitve
    * angle */
#define FIX_R_BAD_PIX() \
  temph = max2(*(p+2), *(p-2)); \
  tempv = max2(*(p+w2), *(p-w2));\
  maxP = max2(temph, tempv);\
  temph = max2(*(p-2-w2), *(p+2-w2)); \
  tempv = max2(*(p-2+w2), *(p+2+w2)); \
  maxX= max2(temph, tempv);\
  maxP = max2(maxP, maxX); \
  if (*p > maxP+MAX_PIX_DIFF_INC_R)\
  {\
    *p = (uint8) maxP;\
  }\
  else\
  {\
    temph = min2(*(p+2), *(p-2));\
    tempv = min2(*(p+w2), *(p-w2));\
    minP  = min2(temph, tempv);\
    temph = min2(*(p-2-w2), *(p+2-w2)); \
    tempv = min2(*(p-2+w2), *(p+2+w2)); \
    minX = min2(temph, tempv);\
    minP = min2(minP, minX); \
    if ((minP+MAX_PIX_DIFF_DEC_R>0) &&\
        (*p < minP+MAX_PIX_DIFF_DEC_R))\
    {\
      *p = (uint8) minP;\
    }\
  }


#define FIX_B_BAD_PIX() \
  temph = max2(*(p+2), *(p-2)); \
  tempv = max2(*(p+w2), *(p-w2));\
  maxP = max2(temph, tempv);\
  temph = max2(*(p-2-w2), *(p+2-w2)); \
  tempv = max2(*(p-2+w2), *(p+2+w2)); \
  maxX= max2(temph, tempv);\
  maxP = max2(maxP, maxX); \
  if (*p > maxP+MAX_PIX_DIFF_INC_B)\
  {\
    *p = (uint8) maxP;\
  }\
  else\
  {\
    temph = min2(*(p+2), *(p-2));\
    tempv = min2(*(p+w2), *(p-w2));\
    minP  = min2(temph, tempv);\
    temph = min2(*(p-2-w2), *(p+2-w2)); \
    tempv = min2(*(p-2+w2), *(p+2+w2)); \
    minX = min2(temph, tempv);\
    minP = min2(minP, minX); \
    if ((minP+MAX_PIX_DIFF_DEC_B>0) &&\
        (*p < minP+MAX_PIX_DIFF_DEC_B)) \
    {\
      *p = (uint8) minP;\
    }\
  }


#define FIX_G_BAD_PIX() \
  temph = max2(*(p+1-w), *(p-1-w)); \
  tempv = max2(*(p+1+w), *(p-1+w)); \
  maxX = max2(temph, tempv); \
  temph = max2(*(p-2), *(p+2)); \
  tempv = max2(*(p-w2), *(p+w2)); \
  maxP = max2(temph, tempv); \
  maxP = max2(maxP, maxX); \
  if (*p > maxP+MAX_PIX_DIFF_INC_G)\
  { \
    *p = (uint8) maxP; \
  } \
  else \
  {\
    temph = min2(*(p+1-w), *(p-1-w)); \
    tempv = min2(*(p+1+w), *(p-1+w)); \
    minX = min2(temph, tempv); \
    temph = min2(*(p-2), *(p+2)); \
    tempv = min2(*(p-w2), *(p+w2)); \
    minP = min2(temph, tempv); \
    minP = min2(minP, minX); \
    if ((minP+MAX_PIX_DIFF_DEC_G>0) &&\
       (*p < minP+MAX_PIX_DIFF_DEC_G))  \
    { \
      *p = (uint8) minP; \
    }\
  }




  /*
  ** Input should be BGGR or RGGB format
  */
  if ((img_ptr->cFormat != IPL_BAYER_BGGR) &&
      (img_ptr->cFormat != IPL_BAYER_RGGB))
  {
    MSG_LOW("ipl_bad_pixel_correct marker_201\n");
    return IPL_FAILURE;
  }

  w = img_ptr->dx;
  h = img_ptr->dy;

  w2 = w << 1;

  /*
  ** Correct Green Pixels
  ** See if each pixel is more than MAX_PIX_DIFF different than previous
  ** pixel we corrected. If so, then check to see if its value is higher
  ** than his 8 neighbors, or lower than them. If that is also true, then
  ** set to the low or high value depending on which test if failed.
  */
  p = img_ptr->imgPtr;
  p += img_ptr->dx + 2;
  cols = (w-2) >> 1; 
  rows = (h-2) >> 1; 

  MSG_LOW("ipl_bad_pixel_correct marker_1\n");

  for (j=0; j < rows; j++)
  {
    /* lets do some loop unrooling, lets to two lines as a time */

    /* do odd line */
    for (i=0; i < cols; i++)
    {
      diff = *p - *(p-1-w);
      if (IPL_UTIL_DEBUG && 0) //lint !e506 !e774
      {
       printf("G (%d,%d) comparing %d to n%d d%d(%d,%d,%d,%d) (%d,%d,%d,%d)\n",
          i,j,*p,*(p-2),diff,
          *(p-1-w),*(p+1-w),*(p-1+w),*(p+1+w),
          *(p-2*w),*(p-2),*(p+2),*(p+2*w));
      }
      if (IPL_UTIL_PROFILE) /*lint !e774 !e506 */ qlooked++;
      if (diff > MAX_PIX_DIFF_INC_G || diff < MAX_PIX_DIFF_DEC_G)
      {
        FIX_G_BAD_PIX(); 
      }
      p += 2;
    }
    p += 1;


    /* do even line line */
    for (i=0; i < cols; i++)
    {
      diff = *p - *(p-2);
      if (IPL_UTIL_DEBUG && 0)  //lint !e506 !e774
      {
       printf("G(%d,%d) comparing %d to n%d d%d (%d,%d,%d,%d) (%d,%d,%d,%d)\n",
          i,j,*p,*(p-2),diff,
          *(p-1-w),*(p+1-w),*(p-1+w),*(p+1+w),
          *(p-2*w),*(p-2),*(p+2),*(p+2*w));
      }
      if (IPL_UTIL_PROFILE) /*lint !e774 !e506 */ qlooked++;
      if (diff > MAX_PIX_DIFF_INC_G || diff < MAX_PIX_DIFF_DEC_G)
      {
        FIX_G_BAD_PIX(); 
      }
      p += 2;
    }
    p += 3;

    if (IPL_UTIL_DEBUG && 0) //lint !e506 !e774 
      printf("\n\n");
  }

  MSG_LOW("ipl_bad_pixel_correct marker_2\n");

  /*
  ** Correct Blue Pixels
  ** See if each pixel is more than MAX_PIX_DIFF different than previous
  ** pixel we corrected. If so, then check to see if its value is higher
  ** than his 8 neighbors, or lower than them. If that is also true, then
  ** set to the low or high value depending on which test if failed.
  */
  if (img_ptr->cFormat == IPL_BAYER_BGGR) 
  {
    i = 2;
    j = 2;
  }
  else
  {
    i = 3;
    j = 3;
  }
  p = img_ptr->imgPtr;
  p += (img_ptr->dx*j) + i;
  cols = (w-4)>>1; 
  rows = (h-4)>>1; 
  for (j=0; j < rows; j++)
  {
    for (i=0; i < cols; i++)
    {
      diff = *p - *(p-2);
      if (IPL_UTIL_DEBUG && 0) //lint !e506 !e774 
      {
       printf("B(%d,%d) comparing %d to n%d d%d (%d,%d,%d,%d) (%d,%d,%d,%d)\n",
          i,j,*p,*(p-2),diff,
          *(p-1-w),*(p+1-w),*(p-1+w),*(p+1+w),
          *(p-2*w),*(p-2),*(p+2),*(p+2*w));
      }
      if (IPL_UTIL_PROFILE) /*lint !e774 !e506 */ qlooked++;

      if (diff > MAX_PIX_DIFF_INC_B || diff < MAX_PIX_DIFF_DEC_B)
      {
        FIX_B_BAD_PIX();
      }
      p += 2;
    }
    p += (4 + img_ptr->dx);
  }

  MSG_LOW("ipl_bad_pixel_correct marker_3\n");

  /*
  ** Correct Red Pixels
  ** See if each pixel is more than MAX_PIX_DIFF different than previous
  ** pixel we corrected. If so, then check to see if its value is higher
  ** than his 8 neighbors, or lower than them. If that is also true, then
  ** set to the low or high value depending on which test if failed.
  */
  if (img_ptr->cFormat == IPL_BAYER_BGGR) 
  {
    i = 3;
    j = 3;
  }
  else
  {
    i = 2;
    j = 2;
  }
  p = img_ptr->imgPtr;
  p += img_ptr->dx*j + i;
  cols = (w-4)>>1; 
  rows = (h-4)>>1; 
  for (j=0; j < rows; j++)
  {
    for (i=0; i < cols; i++)
    {
      if (IPL_UTIL_DEBUG && 0) //lint !e506 !e774 
      {
       printf("R(%d,%d) comparing %d to n%d d%d (%d,%d,%d,%d) (%d,%d,%d,%d)\n",
              i,j,*p,*(p-2),diff, 
              *(p-1-w),*(p+1-w),*(p-1+w),*(p+1+w),
              *(p-2*w),*(p-2),*(p+2),*(p+2*w));
      }
      if (IPL_UTIL_PROFILE) /*lint !e774 !e506 */ qlooked++;

      diff = *p - *(p-2);
      if (diff > MAX_PIX_DIFF_INC_R || diff < MAX_PIX_DIFF_DEC_R)
      {
        FIX_R_BAD_PIX();
      }
      p += 2;
    }
    p += (4 + img_ptr->dx);
  }

  if (IPL_UTIL_PROFILE)/*lint !e774 !e506 */  
  {
    printf ("new version  qlooked     %d pixels\n", qlooked);
    printf ("new version   looked     %d pixels\n", looked);
    printf ("new version   falseA     %d pixels\n", falseAlarm);
    printf ("new version fixed Mx     %d bad pixels\n", fixMax);
    printf ("new version fixed Mn     %d bad pixels\n", fixMin);
    printf ("new version fixed Total  %d bad pixels\n", fixMin+fixMax);
  }


  MSG_LOW("ipl_bad_pixel_correct marker_100\n");
  return IPL_SUCCESS;
}


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_bad_pixel_correct_5x5

DESCRIPTION
  This function is used to do bad pixel correction.
  The Input Image is overwritten with the output image.
  The input should be in either IPL_BAYER_BGGR or in
  IPL_BAYER_RGGB format.
  The output will be in the same color format as input.

DEPENDENCIES
  None

ARGUMENTS IN
  img_ptr points to the input and also the output image.

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_bad_pixel_correct_5x5
(
  ipl_image_type* img_ptr          /* Points to the input image           */
)
{
  uint32 i,j;
  uint32 w,w2,h;
  uint32 cols, rows;
  uint8* p;

  int16 maxP;
  int16 minP;
  int16 diff = 0;

  /* for profiling */
  int fixMax = 0;
  int fixMin = 0;
  int looked = 0;
  int qlooked = 0;
  int falseAlarm = 0;

  MSG_LOW("ipl_bad_pixel_correct_5x5 marker_0\n");

  if (!img_ptr || !img_ptr->imgPtr)
  {
    MSG_LOW("ipl_bad_pixel_correct_5x5 marker_200\n");
    return IPL_FAILURE;
  }
   /* the largest absolute difference between two pixels which would trigger
   * suspicion and further checks. Each channel has different thereshold
   * based on theory that eye is less suseptible to changes in blue, then 
   * red then green.
   */

   /* One might think that the values defined above will work well for mid 
    * to bright pixels, but not for dark pixels, i.e. if pixel value is 40 
    * and cold pixel is 10 in blue channel, algorithm does not fix this 400% 
    * discrepency. This is okay since eye is less sensitive to changes in
    * darks than mid tones */
  
   /* Below are our inline functions that do the acutal check and fix.
    * Testing on ISO focus chart showed you must check all 8 neighboring 
    * pixels to avoid destroying thin 1-pixel wide lines from respecitve
    * angle */
#define FIX_R_BAD_PIX5x5() \
  maxP = max2(*(p-4-w2-w2), *(p-2-w2-w2)); \
  maxP = max2(*(p-4-w2), maxP); \
  maxP = max2(*(p-2-w2), maxP); \
  maxP = max2(*(p-4), maxP); \
  maxP = max2(*(p-2), maxP); \
  maxP = max2(*(p-4+w2), maxP); \
  maxP = max2(*(p-2+w2), maxP); \
  maxP = max2(*(p-4+w2+w2), maxP); \
  maxP = max2(*(p-2+w2+w2), maxP); \
  maxP = max2(*(p-w2-w2), maxP); \
  maxP = max2(*(p-w2), maxP); \
  maxP = max2(*(p+w2+w2), maxP); \
  maxP = max2(*(p+w2), maxP); \
  maxP = max2(*(p+4-w2-w2), maxP); \
  maxP = max2(*(p+2-w2-w2), maxP); \
  maxP = max2(*(p+4-w2), maxP); \
  maxP = max2(*(p+2-w2), maxP); \
  maxP = max2(*(p+4), maxP); \
  maxP = max2(*(p+2), maxP); \
  maxP = max2(*(p+4+w2), maxP); \
  maxP = max2(*(p+2+w2), maxP); \
  maxP = max2(*(p+4+w2+w2), maxP); \
  maxP = max2(*(p+2+w2+w2), maxP); \
  if (*p > maxP+MAX_PIX_DIFF_INC_R)\
  {\
    *p = (uint8) maxP;\
  }\
  else\
  {\
    minP = min2(*(p-4-w2-w2), *(p-2-w2-w2)); \
    minP = min2(*(p-4-w2), minP); \
    minP = min2(*(p-2-w2), minP); \
    minP = min2(*(p-4), minP); \
    minP = min2(*(p-2), minP); \
    minP = min2(*(p-4+w2), minP); \
    minP = min2(*(p-2+w2), minP); \
    minP = min2(*(p-4+w2+w2), minP); \
    minP = min2(*(p-2+w2+w2), minP); \
    minP = min2(*(p-w2-w2), minP); \
    minP = min2(*(p-w2), minP); \
    minP = min2(*(p+w2+w2), minP); \
    minP = min2(*(p+w2), minP); \
    minP = min2(*(p+4-w2-w2), minP); \
    minP = min2(*(p+2-w2-w2), minP); \
    minP = min2(*(p+4-w2), minP); \
    minP = min2(*(p+2-w2), minP); \
    minP = min2(*(p+4), minP); \
    minP = min2(*(p+2), minP); \
    minP = min2(*(p+4+w2), minP); \
    minP = min2(*(p+2+w2), minP); \
    minP = min2(*(p+4+w2+w2), minP); \
    if ((minP+MAX_PIX_DIFF_DEC_R>0) &&\
        (*p < minP+MAX_PIX_DIFF_DEC_R))\
    {\
      *p = (uint8) minP;\
    }\
  }


#define FIX_B_BAD_PIX5x5() \
  maxP = max2(*(p-4-w2-w2), *(p-2-w2-w2)); \
  maxP = max2(*(p-4-w2), maxP); \
  maxP = max2(*(p-2-w2), maxP); \
  maxP = max2(*(p-4), maxP); \
  maxP = max2(*(p-2), maxP); \
  maxP = max2(*(p-4+w2), maxP); \
  maxP = max2(*(p-2+w2), maxP); \
  maxP = max2(*(p-4+w2+w2), maxP); \
  maxP = max2(*(p-2+w2+w2), maxP); \
  maxP = max2(*(p-w2-w2), maxP); \
  maxP = max2(*(p-w2), maxP); \
  maxP = max2(*(p+w2+w2), maxP); \
  maxP = max2(*(p+w2), maxP); \
  maxP = max2(*(p+4-w2-w2), maxP); \
  maxP = max2(*(p+2-w2-w2), maxP); \
  maxP = max2(*(p+4-w2), maxP); \
  maxP = max2(*(p+2-w2), maxP); \
  maxP = max2(*(p+4), maxP); \
  maxP = max2(*(p+2), maxP); \
  maxP = max2(*(p+4+w2), maxP); \
  maxP = max2(*(p+2+w2), maxP); \
  maxP = max2(*(p+4+w2+w2), maxP); \
  if (*p > maxP+MAX_PIX_DIFF_INC_B)\
  {\
    *p = (uint8) maxP;\
  }\
  else\
  {\
    minP = min2(*(p-4-w2-w2), *(p-2-w2-w2)); \
    minP = min2(*(p-4-w2), minP); \
    minP = min2(*(p-2-w2), minP); \
    minP = min2(*(p-4), minP); \
    minP = min2(*(p-2), minP); \
    minP = min2(*(p-4+w2), minP); \
    minP = min2(*(p-2+w2), minP); \
    minP = min2(*(p-4+w2+w2), minP); \
    minP = min2(*(p-2+w2+w2), minP); \
    minP = min2(*(p-w2-w2), minP); \
    minP = min2(*(p-w2), minP); \
    minP = min2(*(p+w2+w2), minP); \
    minP = min2(*(p+w2), minP); \
    minP = min2(*(p+4-w2-w2), minP); \
    minP = min2(*(p+2-w2-w2), minP); \
    minP = min2(*(p+4-w2), minP); \
    minP = min2(*(p+2-w2), minP); \
    minP = min2(*(p+4), minP); \
    minP = min2(*(p+2), minP); \
    minP = min2(*(p+4+w2), minP); \
    minP = min2(*(p+2+w2), minP); \
    minP = min2(*(p+4+w2+w2), minP); \
    if ((minP+MAX_PIX_DIFF_DEC_B>0) &&\
        (*p < minP+MAX_PIX_DIFF_DEC_B)) \
    {\
      *p = (uint8) minP;\
    }\
  }


#define FIX_G_BAD_PIX5x5() \
  maxP = max2(*(p-w2), *(p+w2)); \
  maxP = max2(*(p-2-w2), maxP); \
  maxP = max2(*(p-3-w), maxP); \
  maxP = max2(*(p-1-w), maxP); \
  maxP = max2(*(p-4), maxP); \
  maxP = max2(*(p-2), maxP); \
  maxP = max2(*(p-3+w), maxP); \
  maxP = max2(*(p-1+w), maxP); \
  maxP = max2(*(p-2+w2), maxP); \
  maxP = max2(*(p+2-w2), maxP); \
  maxP = max2(*(p+3-w), maxP); \
  maxP = max2(*(p+1-w), maxP); \
  maxP = max2(*(p+4), maxP); \
  maxP = max2(*(p+2), maxP); \
  maxP = max2(*(p+3+w), maxP); \
  maxP = max2(*(p+1+w), maxP); \
  maxP = max2(*(p+2+w2), maxP); \
  if (*p > maxP+MAX_PIX_DIFF_INC_G)\
  { \
    *p = (uint8) maxP; \
  } \
  else \
  {\
    minP = min2(*(p-w2), *(p+w2)); \
    minP = min2(*(p-2-w2), minP); \
    minP = min2(*(p-3-w), minP); \
    minP = min2(*(p-1-w), minP); \
    minP = min2(*(p-4), minP); \
    minP = min2(*(p-2), minP); \
    minP = min2(*(p-3+w), minP); \
    minP = min2(*(p-1+w), minP); \
    minP = min2(*(p-2+w2), minP); \
    minP = min2(*(p+2-w2), minP); \
    minP = min2(*(p+3-w), minP); \
    minP = min2(*(p+1-w), minP); \
    minP = min2(*(p+4), minP); \
    minP = min2(*(p+2), minP); \
    minP = min2(*(p+3+w), minP); \
    minP = min2(*(p+1+w), minP); \
    minP = min2(*(p+2+w2), minP); \
    if ((minP+MAX_PIX_DIFF_DEC_G>0) &&\
       (*p < minP+MAX_PIX_DIFF_DEC_G))  \
    { \
      *p = (uint8) minP; \
    }\
  }




  /*
  ** Input should be BGGR or RGGB format
  */
  if ((img_ptr->cFormat != IPL_BAYER_BGGR) &&
      (img_ptr->cFormat != IPL_BAYER_RGGB))
  {
    MSG_LOW("ipl_bad_pixel_correct_5x5 marker_201\n");
    return IPL_FAILURE;
  }

  w = img_ptr->dx;
  h = img_ptr->dy;

  w2 = w << 1;

  MSG_LOW("ipl_bad_pixel_correct_5x5 marker_1\n");

  /*
  ** Correct Green Pixels
  ** See if each pixel is more than MAX_PIX_DIFF different than previous
  ** pixel we corrected. If so, then check to see if its value is higher
  ** than his 8 neighbors, or lower than them. If that is also true, then
  ** set to the low or high value depending on which test if failed.
  */
  p = img_ptr->imgPtr;
  p += img_ptr->dx + 2;
  cols = (w-2) >> 1; 
  rows = (h-2) >> 1; 

  for (j=0; j < rows; j++)
  {
    /* lets do some loop unrooling, lets to two lines as a time */

    /* do odd line */
    for (i=0; i < cols; i++)
    {
      diff = *p - *(p-1-w);
      if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
      {
       printf("G (%lu,%lu)comparing %d to n%d d%d(%d,%d,%d,%d) (%d,%d,%d,%d)\n",
          i,j,*p,*(p-2),diff,
          *(p-1-w),*(p+1-w),*(p-1+w),*(p+1+w),
          *(p-2*w),*(p-2),*(p+2),*(p+2*w));
      }
      if (IPL_UTIL_PROFILE) /*lint !e774 !e506 */ qlooked++;
      if (diff > MAX_PIX_DIFF_INC_G || diff < MAX_PIX_DIFF_DEC_G)
      {
        FIX_G_BAD_PIX5x5(); 
      }
      p += 2;
    }
    p += 1;


    /* do even line line */
    for (i=0; i < cols; i++)
    {
      diff = *p - *(p-2);
      if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
      {
       printf("G(%lu,%lu)comparing %d to n%d d%d (%d,%d,%d,%d) (%d,%d,%d,%d)\n",
          i,j,*p,*(p-2),diff,
          *(p-1-w),*(p+1-w),*(p-1+w),*(p+1+w),
          *(p-2*w),*(p-2),*(p+2),*(p+2*w));
      }
      if (IPL_UTIL_PROFILE) /*lint !e774 !e506 */ qlooked++;
      if (diff > MAX_PIX_DIFF_INC_G || diff < MAX_PIX_DIFF_DEC_G)
      {
        FIX_G_BAD_PIX5x5(); 
      }
      p += 2;
    }
    p += 3;

    if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
      printf("\n\n");
  }

  MSG_LOW("ipl_bad_pixel_correct_5x5 marker_2\n");
  /*
  ** Correct Blue Pixels
  ** See if each pixel is more than MAX_PIX_DIFF different than previous
  ** pixel we corrected. If so, then check to see if its value is higher
  ** than his 8 neighbors, or lower than them. If that is also true, then
  ** set to the low or high value depending on which test if failed.
  */
  if (img_ptr->cFormat == IPL_BAYER_BGGR) 
  {
    i = 2;
    j = 2;
  }
  else
  {
    i = 3;
    j = 3;
  }
  p = img_ptr->imgPtr;
  p += (img_ptr->dx*j) + i;
  cols = (w-4)>>1; 
  rows = (h-4)>>1; 
  for (j=0; j < rows; j++)
  {
    for (i=0; i < cols; i++)
    {
      diff = *p - *(p-2);
      if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
      {
       printf("B(%lu,%lu)comparing %d to n%d d%d (%d,%d,%d,%d) (%d,%d,%d,%d)\n",
          i,j,*p,*(p-2),diff,
          *(p-1-w),*(p+1-w),*(p-1+w),*(p+1+w),
          *(p-2*w),*(p-2),*(p+2),*(p+2*w));
      }
      if (IPL_UTIL_PROFILE) /*lint !e774 !e506 */ qlooked++;

      if (diff > MAX_PIX_DIFF_INC_B || diff < MAX_PIX_DIFF_DEC_B)
      {
        FIX_B_BAD_PIX5x5();
      }
      p += 2;
    }
    p += (4 + img_ptr->dx);
  }

  MSG_LOW("ipl_bad_pixel_correct_5x5 marker_3\n");
  /*
  ** Correct Red Pixels
  ** See if each pixel is more than MAX_PIX_DIFF different than previous
  ** pixel we corrected. If so, then check to see if its value is higher
  ** than his 8 neighbors, or lower than them. If that is also true, then
  ** set to the low or high value depending on which test if failed.
  */
  if (img_ptr->cFormat == IPL_BAYER_BGGR) 
  {
    i = 3;
    j = 3;
  }
  else
  {
    i = 2;
    j = 2;
  }
  p = img_ptr->imgPtr;
  p += img_ptr->dx*j + i;
  cols = (w-4)>>1; 
  rows = (h-4)>>1; 
  for (j=0; j < rows; j++)
  {
    for (i=0; i < cols; i++)
    {
      if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
      {
       printf("R(%lu,%lu)comparing %d to n%d d%d (%d,%d,%d,%d) (%d,%d,%d,%d)\n",
              i,j,*p,*(p-2),diff, 
              *(p-1-w),*(p+1-w),*(p-1+w),*(p+1+w),
              *(p-2*w),*(p-2),*(p+2),*(p+2*w));
      }
      if (IPL_UTIL_PROFILE) /*lint !e774 !e506 */ qlooked++;

      diff = *p - *(p-2);
      if (diff > MAX_PIX_DIFF_INC_R || diff < MAX_PIX_DIFF_DEC_R)
      {
        FIX_R_BAD_PIX5x5();
      }
      p += 2;
    }
    p += (4 + img_ptr->dx);
  }

  if (IPL_UTIL_PROFILE) /*lint !e774 !e506 */ 
  {
    printf ("new version  qlooked     %d pixels\n", qlooked);
    printf ("new version   looked     %d pixels\n", looked);
    printf ("new version   falseA     %d pixels\n", falseAlarm);
    printf ("new version fixed Mx     %d bad pixels\n", fixMax);
    printf ("new version fixed Mn     %d bad pixels\n", fixMin);
    printf ("new version fixed Total  %d bad pixels\n", fixMin+fixMax);
  }

  MSG_LOW("ipl_bad_pixel_correct_5x5 marker_100\n");
  return IPL_SUCCESS;
}




#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_calc_psnr

DESCRIPTION
    This function returns the psnr of two images.

    Both input and output frames must be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr            pointer to the input frame    
  o_img_ptr            pointer to the output frame
  in1                  region to process in first image
  in2                  region to process in second image

RETURN VALUE
  IPL_SUCCESS          indicates operation was successful
  IPL_FAILURE          otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN float ipl_calc_psnr
(
  ipl_image_type* i1_img_ptr,        /* Points to the input frame  */
  ipl_image_type* i2_img_ptr,        /* Points to the output frame */
  ipl_rect_type *in1,
  ipl_rect_type *in2
)
{
  unsigned short *in1ImgPtr, *in2ImgPtr;
  uint32 row, col;
  uint32 accum = 0;
	float mse = 0;
	float rmse;
	float psnr;
	int len = 0;
	unsigned char r1,r2,g1,g2,b1,b2;
  uint32 offx;

  /* Check parameters */
  if (i1_img_ptr == NULL || i2_img_ptr == NULL)
  {
    return IPL_FAILURE;
  }

  if (i1_img_ptr->cFormat != IPL_RGB565 ||
      i1_img_ptr->cFormat != i2_img_ptr->cFormat)
  {
    return IPL_FAILURE;
  }

  if (in1->dx != in2->dx || in1->dy != in2->dy)
  {
    return IPL_FAILURE;
  }
    
  in1ImgPtr = (uint16*) (i1_img_ptr->imgPtr + 2*(in1->x+in1->y*i1_img_ptr->dy));
  in2ImgPtr = (uint16*) (i2_img_ptr->imgPtr + 2*(in2->x+in2->y*i2_img_ptr->dy));

  offx = 2*(i1_img_ptr->dx - in1->dx);

  /* Loop through images */
  accum = 0;
  for(row = 0; row < in1->dy; row++) 
  {
    for(col = 0; col < in1->dx; col++) 
    {
      /*
      if (row == 0)
        printf ("(%d,%d) comparing %d to %d\n", col, row,
                *in1ImgPtr, *in2ImgPtr);
                */

      unpack_rgb565(*in1ImgPtr++,&r1,&g1,&b1);
      unpack_rgb565(*in2ImgPtr++,&r2,&g2,&b2);

        //printf ("(%d,%d) comparing %d %d %d to %d %d %d; accum:%u ",
                //col,row,r1,g1,b1,r2,g2,b2,accum);
                //
//      if (row == 153)
 //       printf ("%d %u\n", col, accum); 

      accum += ((r1-r2)*(r1-r2) + (g1-g2)*(g1-g2) + (b1-b2)*(b1-b2));

      //if (row == 0)
      //  printf ("accum after:%u\n", accum);
    }

    in1ImgPtr += offx;
    in2ImgPtr += offx;

    //printf ("%d %u\n", row, accum/1000);
  }

  printf ("accum %u\n", accum);
  mse = (accum / (i1_img_ptr->dx * i1_img_ptr->dy));
  rmse = sqrt(mse);
  printf ("rmse %f\n", rmse);
  psnr= 20*log10f(255.0/rmse);

	return psnr;
}


/*===========================================================================

FUNCTION    ipl_calc_max

DESCRIPTION

  This function computes the max value for crop area of pixels.

DEPENDENCIES
  None

ARGUMENTS IN
  in          input image
  crop        area in question

ARGUMENTS IN/OUT

  Returns average Y,Cb,Cr packed as 24 bits into uint32

RETURN VALUE

  Returns average Y,Cb,Cr packed as 24 bits into uint32

SIDE EFFECTS
  None

===========================================================================*/
static uint32 ipl_calc_max(ipl_image_type *in, ipl_rect_type *crop)
{
  uint32 x, y;
  unsigned char * p;
  unsigned int index;
  uint32 Y[256], Cb[256], Cr[256];
  uint32 avg;

  uint32 mostYCnt, mostCbCnt, mostCrCnt;
  uint8 mostY = 0, mostCb = 0, mostCr = 0;

  if (IPL_UTIL_DEBUG) //lint !e774 !e506
    printf("Going to find median value for (%lux%lu)@(%lu,%lu)\n", 
          crop->dx, crop->dy, crop->x, crop->y); 

  if (!in || !in->imgPtr)
    return 0;

  if (!crop)
    return 0;


  // initialize arrays
  for (x = 0; x < 256; x++)
    Y[x] = Cb[x] = Cr[x] = 0;

  p = in->imgPtr;
  if (in->cFormat == IPL_YCbCr)
  {
    for (y = 0; y < crop->dy; y++)
    {
      for (x = 0; x < 2*crop->dx; x += 4)
      {
        index = ((2*crop->x + crop->y*2*in->dx) + (x + y*2*in->dx));

        if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          printf("(%d,%d) CbYCrY is %d,%d,%d,%d\n", 
                 x, y, p[index], p[index+ 1], p[index+2], p[index+3]);

        Cb[p[index]]++;
         Y[p[index+1]]++;
        Cr[p[index+2]]++;
         Y[p[index+3]]++;
      }
    }

    if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
    {
      printf("Y histogram:\n");
      for (x = 0; x < 256; x++)
      {
        printf("%d: %lu\n", x, Y[x]);
      }

      printf("Cb histogram:\n");
      for (x = 0; x < 256; x++)
      {
        printf("%d: %lu\n", x, Cb[x]);
      }

      printf("Cr histogram:\n");
      for (x = 0; x < 256; x++)
      {
        printf("%d: %lu\n", x, Cr[x]);
      }
    }

    // find max
    mostYCnt = 0; 
    mostCbCnt = 0; 
    mostCrCnt = 0; 
    for (x = 0; x < 256; x++)
    {
      if (Y[x] > mostYCnt)
      {
        mostY = x; 
        mostYCnt = Y[x];
      }

      if (Cb[x] > mostCbCnt)
      {
        mostCb = x; 
        mostCbCnt = Cb[x];
      }

      if (Cr[x] > mostCrCnt)
      {
        mostCr = x; 
        mostCrCnt = Cr[x];
      }
    }
     
    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Median YCbCr is %d,%d,%d\n", 
             (int)mostY, (int)mostCb, (int)mostCr); //lint !e644

    avg = pack_ycbcr(mostY,mostCb,mostCr);

    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Packed, it is %d\n", (int)avg);

  }
  else
    return 0;

  return avg;

}




#endif


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_calc_mchan_histograms

DESCRIPTION
  This function will calculate the image histogram.
  It currently computes luma only, but the infrastructre is here to do all 
  3 channels.



DEPENDENCIES
  None

ARGUMENTS IN/OUT
  in is the input image whose histogram needs to be calculated
  crop is the area to look at
  img_hist is where we write values

  Currently only support RGB565 and YCbCr images
  The historgram returned is only the luma channel right now


RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_calc_mchan_histograms
(
  ipl_image_type      *in,
  ipl_rect_type       *loc,
  ipl_mchan_histogram_type  *img_hist,
  ipl_quality_type    qual
)
{
  uint32 x,y;
  unsigned char * p;
  unsigned char * c;
  uint16 * p16;
  ipl_rect_type crop;
  int val, cnt;
  uint8 l, cb, cr;
  uint32 step;
  uint8 red,green,blue;

  MSG_LOW("ipl_calc_histogram marker_0\n");

  if (in == NULL)
  {
    MSG_LOW("ipl_calc_histogram marker_200\n");
    return IPL_FAILURE;
  }
  if (in->imgPtr == NULL)
  {
    MSG_LOW("ipl_calc_histogram marker_201\n");
    return IPL_FAILURE;
  }
  if (img_hist == NULL)
  {
    MSG_LOW("ipl_calc_histogram marker_202\n");
    return IPL_FAILURE;
  }

  // if no loc, then add the whole frame
  if (loc == NULL)
  {
    crop.x  = 0;
    crop.y  = 0;
    crop.dx = in->dx;
    crop.dy = in->dy;
  }
  else
  {
    crop.x  = loc->x;
    crop.y  = loc->y;
    crop.dx = loc->dx;
    crop.dy = loc->dy;
  }

  if ((crop.dx + crop.x > in->dx) || 
      (crop.dy + crop.y > in->dy))
  {
    MSG_LOW("ipl_calc_histogram marker_203\n");
    return IPL_FAILURE;
  }

  // cleanup histograms 
  memset(&img_hist->mv[0][0],0,IPL_MAX_HIST_CHAN*IPL_MAX_HIST*sizeof(uint32));

  // user may have forgotten to tell us size of hist 
  if (img_hist->size == 0)
    img_hist->size = 256;


  MSG_LOW("ipl_calc_histogram marker_1\n");

  if (img_hist->channel == IPL_CHANNEL_YRGB) 
  {
    if ((in->cFormat == IPL_YCrCb420_LINE_PK) ||
        (in->cFormat == IPL_YCbCr420_LINE_PK))
    {
      p = in->imgPtr;
      c = in->clrPtr;

      // make crop area even width and height and on even boundary
      crop.x &= 0XFFFE;
      crop.y &= 0XFFFE;
      crop.dx &= 0XFFFE;
      crop.dy &= 0XFFFE;


      // go to top of crop
      p += (crop.x + crop.y   * in->dx);
      c += (crop.x + crop.y/2 * in->dx);

      if (qual == IPL_QUALITY_LOW)
        step = 16;
      else if (qual == IPL_QUALITY_MEDIUM)
        step = 8;
      else if (qual == IPL_QUALITY_HIGH)
        step = 4;
      else 
        step = 2;
        //step = 1;


      cnt = step*step;

      for (y = 0; y < crop.dy/step; y++)
      {
        for (x = 0; x < crop.dx/step; x++)
        {
          l = *p;     // get y, skip 2nd y
          // for debug purposes, make the pixel we red so we can see where
          // it was
          //if (IPL_UTIL_DEBUG) //lint !e774 !e506
          //  *p = 0; 

          p += step;
          cr = *c++;  // get cb 
          cb = *c++;  // get cr 

          if (step == 1)
          {
            if (!(x%2))
              c -= 2;
          }
          else
          {
            c += (step-2);
          }
          

          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) y cb cr is %d %d %d ", x, y, l, cb, cr);

          if (img_hist->active[0])
          {
            // bump luma histogram associated value
            if (IPL_UTIL_DEBUG) //lint !e774 !e506
              printf(" luma %d ", l);
            img_hist->mv[l][0] += cnt;
          }

          // bump red histogram associated value
          if (img_hist->active[1])
          {
            val = l + ipl2_Cr2RTable[cr];
            val = CLIPIT(val);
            if (IPL_UTIL_DEBUG) //lint !e774 !e506
              printf(" red %d ", val);
            img_hist->mv[val][1] += cnt;
          }

          // bump green histogram associated value
          if (img_hist->active[2])
          {
            val = l - ipl2_Cb2GTable[cb] - ipl2_Cr2GTable[cr];
            val = CLIPIT(val);
            if (IPL_UTIL_DEBUG) //lint !e774 !e506
              printf(" green %d ", val);
            img_hist->mv[val][2] += cnt;
          }

          if (img_hist->active[3])
          {
            // bump blue histogram associated value
            val = l + ipl2_Cb2BTable[cb];
            val = CLIPIT(val);
            if (IPL_UTIL_DEBUG) //lint !e774 !e506
              printf(" blue %d\n", val);
            img_hist->mv[val][3] += cnt;
          }

        }

        // consider pitch
        p += (in->dx - crop.dx); 
        c += (in->dx - crop.dx); 

        // skip even lines since 420
        //p += in->dx; 

        // now skip based on steps size
        p += (in->dx * (step-1)); 

        if (step == 1)
          c -= (in->dx);
        else
          c += (in->dx * (step-2/2)); 
      }
    }
    else if (in->cFormat == IPL_RGB565)
    {
      p16 = (uint16 *) in->imgPtr;

      // go to top of crop
      p16 += (crop.x + crop.y * in->dx);

      if (qual == IPL_QUALITY_LOW)
        step = 8;
      else if (qual == IPL_QUALITY_MEDIUM)
        step = 4;
      else if (qual == IPL_QUALITY_HIGH)
        step = 2;
      else 
        step = 1;

      cnt = step*step;

      for (y = 0; y < crop.dy/step; y++)
      {
        for (x = 0; x < crop.dx/step; x++)
        {
          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            *p16 = 0; 

          unpack_rgb565(*p16, &red,&green,&blue);
          l = (red +  green + blue)/3;
          p16 += step;

          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) y red green blue %d %d %d %d\n", x, y, l, red, green, blue);

          if (img_hist->active[0])
            img_hist->mv[l][0] += cnt;

          // bump red histogram associated value
          if (img_hist->active[1])
            img_hist->mv[red][1] += cnt;

          // bump green histogram associated value
          if (img_hist->active[2])
            img_hist->mv[green][2] += cnt;

          // bump blue histogram associated value
          if (img_hist->active[3])
            img_hist->mv[blue][3] += cnt;
        }

        // consider pitch
        p16 += (in->dx - crop.dx); 

        // now skip based on steps size
        p16 += (in->dx * (step - 1)); 
      }
    }
    else
    {
      if (img_hist->size != 255 &&
          img_hist->size != 256 &&
          img_hist->size != 1024)
      {
        MSG_LOW("ipl_calc_histogram marker_205.1\n");
        return IPL_FAILURE;
      }

      for (x = 0; x < img_hist->size; x++)
      {
        img_hist->mv[x][0] = 0;
        img_hist->mv[x][1] = 0;
        img_hist->mv[x][2] = 0;
        img_hist->mv[x][3] = 0;
      }

      MSG_LOW("ipl_calc_histogram marker_205\n");
      return IPL_FAILURE;
    }
  }
  else
  {
    if (img_hist->size != 255 &&
        img_hist->size != 256 &&
        img_hist->size != 1024)
    {
      MSG_LOW("ipl_calc_histogram marker_206.1\n");
      return IPL_FAILURE;
    }

    for (x = 0; x < img_hist->size; x++)
    {
      img_hist->mv[x][0] = 0;
      img_hist->mv[x][1] = 0;
      img_hist->mv[x][2] = 0;
      img_hist->mv[x][3] = 0;
    }

    MSG_LOW("ipl_calc_histogram marker_206\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_calc_histogram marker_100\n");
  return IPL_SUCCESS;
}

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_calc_histogram

DESCRIPTION
  This function will calculate the image histogram.
  It currently computes luma only, but the infrastructre is here to do all 
  3 channels.



DEPENDENCIES
  None

ARGUMENTS IN/OUT
  in is the input image whose histogram needs to be calculated
  crop is the area to look at
  img_hist is where we write values

  Currently only support RGB565 and YCbCr images
  The historgram returned is only the luma channel right now


RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_calc_histograms
(
  ipl_image_type      *in,
  ipl_rect_type       *loc,
  ipl_histogram_type  *img_hist,
  ipl_quality_type    qual
)
{
  uint32 x,y;
  unsigned int index;
  unsigned char * p;
  unsigned char * c;
  ipl_rect_type crop;
  int i1;
  int cnt;
  uint16 * p16;
  uint8 r,g,b;
  uint8 l, cb, cr;
  int rc;
  uint32 step;
  int16 ycbcr2rgb[6] = {8, 25803, -3071, -7672, 30399, 12};

  MSG_LOW("ipl_calc_histogram marker_0\n");

  if (in == NULL)
  {
    MSG_LOW("ipl_calc_histogram marker_200\n");
    return IPL_FAILURE;
  }
  if (in->imgPtr == NULL)
  {
    MSG_LOW("ipl_calc_histogram marker_201\n");
    return IPL_FAILURE;
  }
  if (img_hist == NULL)
  {
    MSG_LOW("ipl_calc_histogram marker_202\n");
    return IPL_FAILURE;
  }

  // if no loc, then add the whole frame
  if (loc == NULL)
  {
    crop.x  = 0;
    crop.y  = 0;
    crop.dx = in->dx;
    crop.dy = in->dy;
  }
  else
  {
    crop.x  = loc->x;
    crop.y  = loc->y;
    crop.dx = loc->dx;
    crop.dy = loc->dy;
  }

  if ((crop.dx + crop.x > in->dx) || 
      (crop.dy + crop.y > in->dy))
  {
    MSG_LOW("ipl_calc_histogram marker_203\n");
    return IPL_FAILURE;
  }

  // cleanup histograms 
  memset(&img_hist->v[0],0,img_hist->size*sizeof(uint32));


  // user may have forgotten to tell us size of hist 
  if (img_hist->size == 0)
    img_hist->size = 256;


  MSG_LOW("ipl_calc_histogram marker_1\n");

  // right now we can only compute luma
  if (img_hist->channel == IPL_CHANNEL_LUMA)
  {
    if (in->cFormat == IPL_YCbCr)
    {
      p = in->imgPtr;

      for (y = 0; y < crop.dy; y++)
      {
        for (x = 0; x < crop.dx; x++)
        {
          index = (1 + (2*crop.x + crop.y*2*in->dx) + (2*x + y*2*in->dx));

          img_hist->v[p[index]]++;

          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) Y is %d\n", x, y, p[index]);
        }

      }
    }
    else if ((in->cFormat == IPL_YCrCb420_LINE_PK) ||
             (in->cFormat == IPL_YCbCr420_LINE_PK))
    {
      p = in->imgPtr;

      // make crop area even width and height and on even boundary
      crop.x &= 0XFFFE;
      crop.y &= 0XFFFE;
      crop.dx &= 0XFFFE;
      crop.dy &= 0XFFFE;


      // go to top of crop
      p += (crop.x + crop.y   * in->dx);

      if (qual == IPL_QUALITY_LOW)
        step = 16;
      else if (qual == IPL_QUALITY_MEDIUM)
        step = 8;
      else if (qual == IPL_QUALITY_HIGH)
        step = 4;
      else 
        step = 2;

      cnt = step*step;

      for (y = crop.dy/step; y; y--)
      {
        for (x = crop.dx/step; x; x--)
        {
          l = *p;     // get y, skip 2nd y
          // for debug purposes, make the pixel we red so we can see where
          // it was
          if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
            *p = 0; 


          p += step;

          // bump luma histogram associated value
          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf(" luma %d ", l);
          img_hist->v[l] += cnt;
        }

        // consider pitch
        p += (in->dx - crop.dx); 

        // skip even lines since 420
        p += in->dx; 

        // now skip based on steps size
        p += (in->dx * (step-2)); 
      }
    }
    else if (in->cFormat == IPL_RGB565)
    {
      p16 = (uint16 *) in->imgPtr;

      for (y = 0; y < crop.dy; y++)
      {
        for (x = 0; x < crop.dx; x++)
        {
          index = ((crop.x + crop.y*in->dx) + (x + y*in->dx));

          unpack_rgb565(p16[index], &r,&g,&b);
          img_hist->v[(r+g+b)/3]++;

          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) R G B is %d,%d,%d, luma is %d\n", 
                     x,y,r,g,b,(r+g+b)/3);
        }
      }
    }
    else
    {
      MSG_LOW("ipl_calc_histogram marker_204\n");
      return IPL_FAILURE;
    }
  }
  else if ((img_hist->channel == IPL_CHANNEL_RED) ||
           (img_hist->channel == IPL_CHANNEL_GREEN) ||
           (img_hist->channel == IPL_CHANNEL_BLUE))
  {
    if (img_hist->channel == IPL_CHANNEL_RED)
      i1 = 0;
    else if (img_hist->channel == IPL_CHANNEL_GREEN)
      i1 = 2;
    else 
      i1 = 4;


    if (in->cFormat == IPL_YCbCr)
    {
      p = in->imgPtr;

      for (y = 0; y < crop.dy; y++)
      {
        for (x = 0; x < crop.dx; x += 2)
        {
          index = ((2*crop.x + crop.y*2*in->dx) + (2*x + y*2*in->dx));
         
          /* Process 4 pixels at a time */
          cb = p[index];
          l = p[index+1];
          cr = p[index+2];

          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) y cb cr is %d %d %d  ", x, y, l, cb, cr);

          rc = (ycbcr2rgb[i1]*(cb-128) + ycbcr2rgb[i1+1]*(cr-128))*4 + 0x8000;
          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("rc is sum of %d %d", l, rc>>16);
          rc = l + (rc>>16);
          rc = CLIPIT(rc);
          img_hist->v[rc]++;

          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) channel is %d\n", x, y, rc);
        }

      }
    }
    else if ((in->cFormat == IPL_YCrCb420_LINE_PK) ||
             (in->cFormat == IPL_YCbCr420_LINE_PK))
    {
      p = in->imgPtr;
      c = in->clrPtr;

      crop.x &= 0XFFFE;
      crop.y &= 0XFFFE;
      crop.dx &= 0XFFFE;
      crop.dy &= 0XFFFE;

      for (y = 0; y < crop.dy; y++)
      {
        for (x = 0; x < crop.dx; x++)
        {
          index = ((crop.x + crop.y*in->dx) + (x + y*in->dx));
          l = p[index];

          index = (crop.x + crop.y*in->dx) + (2*(x/2) + (y/2)*in->dx);
          cr = c[index];
          cb = c[index+1];

          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) y cb cr is %d %d %d  ", x, y, l, cb, cr);

          rc = (ycbcr2rgb[i1]*(cb-128) + ycbcr2rgb[i1+1]*(cr-128))*4 + 0x8000;
          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("rc is sum of %d %d", l, rc>>16);
          rc = l + (rc>>16);
          rc = CLIPIT(rc);
          img_hist->v[rc]++;

          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) channel is %d\n", x, y, rc);
        }
      }
    }
    else if(in->cFormat == IPL_RGB565 && img_hist->channel == IPL_CHANNEL_RED)
    {
      p16 = (uint16 *) in->imgPtr;
      for (y = 0; y < crop.dy; y++)
      {
        for (x = 0; x < crop.dx; x++)
        {
          index = ((crop.x + crop.y*in->dx) + (x + y*in->dx));
          unpack_rgb565(p16[index], &r,&g,&b);
          img_hist->v[r]++;
          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) R G B is %d,%d,%d\n", x,y,r,g,b);
        }
      }
    }
    else if(in->cFormat == IPL_RGB565 && img_hist->channel == IPL_CHANNEL_GREEN)
    {
      p16 = (uint16 *) in->imgPtr;
      for (y = 0; y < crop.dy; y++)
      {
        for (x = 0; x < crop.dx; x++)
        {
          index = ((crop.x + crop.y*in->dx) + (x + y*in->dx));
          unpack_rgb565(p16[index], &r,&g,&b);
          img_hist->v[g]++;
          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) R G B is %d,%d,%d\n", x,y,r,g,b);
        }
      }
    }
    else if(in->cFormat == IPL_RGB565 && img_hist->channel == IPL_CHANNEL_BLUE)
    {
      p16 = (uint16 *) in->imgPtr;
      for (y = 0; y < crop.dy; y++)
      {
        for (x = 0; x < crop.dx; x++)
        {
          index = ((crop.x + crop.y*in->dx) + (x + y*in->dx));
          unpack_rgb565(p16[index], &r,&g,&b);
          img_hist->v[b]++;
          if (IPL_UTIL_DEBUG) //lint !e774 !e506
            printf("(%lu,%lu) R G B is %d,%d,%d\n", x,y,r,g,b);
        }
      }
    }
    else
    {
      MSG_LOW("ipl_calc_histogram marker_205\n");
      return IPL_FAILURE;
    }
  }
  else
  {
    MSG_LOW("ipl_calc_histogram marker_206\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_calc_histogram marker_100\n");
  return IPL_SUCCESS;

}








/*===========================================================================

FUNCTION    ipl_calc_avg

DESCRIPTION

  This function computes the average for crop area of pixels.

DEPENDENCIES
  None

ARGUMENTS IN
  in          input image
  crop        area in question

ARGUMENTS IN/OUT

  Returns average Y,Cb,Cr packed as 24 bits into uint32

RETURN VALUE

  Returns average Y,Cb,Cr packed as 24 bits into uint32

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN uint32 ipl_calc_avg(ipl_image_type *in, ipl_rect_type *crop)
{
  uint32 x, y;
  unsigned char * p;
  unsigned char * c;
  unsigned int index;
  uint32 Y, Cb, Cr;
  uint32 avg;
  uint32 dx;
  ipl_rect_type tcrop;

  //MSG_LOW("ipl_calc_avg marker_0\n");

  if (IPL_UTIL_DEBUG) //lint !e774 !e506
    printf("Going to find avg value for (%lu%lu)@(%lu,%lu)\n", 
           crop->dx, crop->dy, crop->x, crop->y); 

  if (!in || !in->imgPtr)
  {
    MSG_LOW("ipl_calc_avg marker_200\n");
    return 0;
  }

  if (!crop)
  {
    tcrop.x = 0;
    tcrop.y = 0;
    tcrop.dx = in->dx;
    tcrop.dy = in->dy;
  }
  else
  {
    tcrop.x = crop->x;
    tcrop.y = crop->y;
    tcrop.dx = crop->dx;
    tcrop.dy = crop->dy;
  }

  p = in->imgPtr;
  c = in->clrPtr;
  dx = in->dx;

  Y = Cb = Cr = 0;
  //MSG_LOW("ipl_calc_avg marker_1\n");

  if (in->cFormat == IPL_YCbCr)
  {
    for (y = 0; y < tcrop.dy; y++)
    {
      for (x = 0; x < 2*tcrop.dx; x += 4)
      {
        index = ((2*tcrop.x + tcrop.y*2*in->dx) + (x + y*2*in->dx));


        // put in Cb
        Cb += p[index];

        // put in Y
        Y += p[index+1];

        // put in Cr
        Cr += p[index+2];

        // put in Y
        Y += p[index+3];

        if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          printf("(%lu,%lu) CbYCrY for %d,%d,%d,%d hence %lu %lu %lu %lu\n", 
                 x, y, p[index], p[index+ 1], p[index+2], p[index+3],
                 Cb, Y, Cr, Y);
      }
    }

    // compute average
    Y  /= (tcrop.dx * tcrop.dy);
    Cb /= (tcrop.dx * tcrop.dy / 2);
    Cr /= (tcrop.dx * tcrop.dy / 2);


    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Avg YCbCr is %d,%d,%d\n", (int)Y, (int)Cb, (int)Cr);

    avg = pack_ycbcr(Y,Cb,Cr);

    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Packed, it is %d\n", (int)avg);
  }
  else if (in->cFormat == IPL_YCrCb420_LINE_PK)
  {
    for (y = 0; y < tcrop.dy/2; y++)
    {
      for (x = 0; x < tcrop.dx/2; x++)
      {
        Y  += *(p);
        Y  += *(p+dx);
        p++;
        Y  += *(p);
        Y  += *(p+dx);
        p++;

        Cr += *c++;
        Cb += *c++;
      }
      p += dx;
    }

    // compute average
    Y  /= (tcrop.dx * tcrop.dy);
    Cb /= (tcrop.dx * tcrop.dy/4);
    Cr /= (tcrop.dx * tcrop.dy/4);

    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Avg YCbCr is %d,%d,%d\n", (int)Y, (int)Cb, (int)Cr);

    avg = pack_ycbcr(Y,Cb,Cr);

    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Packed, it is %d\n", (int)avg);
  }
  else if (in->cFormat == IPL_YCbCr444)
  {

    for (y = 0; y < tcrop.dy; y++)
    {
      for (x = 0; x < tcrop.dx; x++)
      {
        index = ((3*tcrop.x + tcrop.y*3*in->dx) + (3*x + y*3*in->dx));
        p = in->imgPtr;
        p += index;

        Cb += *p++;
        Cr += *p++;
        Y  += *p++;
      }
    }

    // compute average
    Y  /= (tcrop.dx * tcrop.dy);
    Cb /= (tcrop.dx * tcrop.dy);
    Cr /= (tcrop.dx * tcrop.dy);

    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Avg YCbCr is %d,%d,%d\n", (int)Y, (int)Cb, (int)Cr);

    avg = pack_ycbcr(Y,Cb,Cr);

    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Packed, it is %d\n", (int)avg);
  }
  else if ((in->cFormat == IPL_YCbCr444) ||
           (in->cFormat == IPL_RGB888) ||
           (in->cFormat == IPL_HSV))
  {
    Y = 0;
    for (y = 0; y < tcrop.dy; y++)
    {
      for (x = 0; x < tcrop.dx; x++)
      {
        //index = ((3*tcrop.x + tcrop.y*3*in->dx) + (3*x + y*3*in->dx));
        //Cb = (p[index] + p[index+1] + p[index+2])/3; 
        Cb = (*p + *(p+1) + *(p+2)) / 3;
        Y += Cb;

        p += 3;
      }
    }

    // compute average
    Y /= (tcrop.dx * tcrop.dy);


    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Avg RGB is %d\n", (int) Y);

    avg = Y;

    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Packed, it is %d\n", (int)avg);
  }
  else
  {
    MSG_LOW("ipl_calc_avg marker_202\n");
    return 0;
  }
  //MSG_LOW("ipl_calc_avg marker_100\n");
  return avg;
}


/*===========================================================================

FUNCTION    ipl_calc_minmax

DESCRIPTION

  This function computes the min and max value for crop area of pixels.

DEPENDENCIES
  None

ARGUMENTS IN
  in          input image
  crop        area in question

ARGUMENTS IN/OUT

  Returns min and max Y,Cb,Cr packed as 24 bits into uint32

RETURN VALUE

  IPL_SUCCESS if all goes well
  IPL_FAILURE if not all goes well

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_calc_minmax(ipl_image_type *in, ipl_rect_type *crop, 
    uint32 * min, uint32 * max)
{
  uint32 x, y;
  unsigned char * p;
  unsigned int index;
  uint32 Y[256], Cb[256], Cr[256];

  uint8 mY = 0, mCb = 0, mCr = 0;

  MSG_LOW("ipl_calc_minmax marker_0\n");

  if (IPL_UTIL_DEBUG) //lint !e774 !e506
    printf("Going to find min/max value for (%lux%lu)@(%lu,%lu)\n", 
          crop->dx, crop->dy, crop->x, crop->y); 

  if (!in || !in->imgPtr)
  {
    MSG_LOW("ipl_calc_minmax marker_200\n");
    return IPL_FAILURE;
  }
  if (!crop)
  {
    MSG_LOW("ipl_calc_minmax marker_201\n");
    return IPL_FAILURE;
  }

  // initialize arrays
  for (x = 0; x < 256; x++)
    Y[x] = Cb[x] = Cr[x] = 0;

  p = in->imgPtr;
  MSG_LOW("ipl_calc_minmax marker_1\n");

  if (in->cFormat == IPL_YCbCr)
  {
    for (y = 0; y < crop->dy; y++)
    {
      for (x = 0; x < 2*crop->dx; x += 4)
      {
        index = ((2*crop->x + crop->y*2*in->dx) + (x + y*2*in->dx));

        if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          printf("(%lu,%lu) CbYCrY is %d,%d,%d,%d\n", 
                 x, y, p[index], p[index+ 1], p[index+2], p[index+3]);

        Cb[p[index]]++;
         Y[p[index+1]]++;
        Cr[p[index+2]]++;
         Y[p[index+3]]++;
      }
    }

    if (IPL_UTIL_DEBUG) //lint !e774 !e506
    {
      printf("Y histogram\n");
      for (x = 0; x < 256; x++)
      {
        printf("%lu: %lu\n", x, Y[x]);
      }

      printf("Cb histogram:\n");
      for (x = 0; x < 256; x++)
      {
        printf("%lu: %lu\n", x, Cb[x]);
      }

      printf("Cr histogram:\n");
      for (x = 0; x < 256; x++)
      {
        printf("%lu: %lu\n", x, Cr[x]);
      }
    }

    // find min 
    for (x = 0; x < 256; x++)
    {
      if (Y[x] > 0)
        break;
    }
    mY = (uint8) x;

    for (x = 0; x < 256; x++)
    {
      if (Cb[x] > 0)
        break;
    }
    mCb = (uint8) x;

    for (x = 0; x < 256; x++)
    {
      if (Cr[x] > 0)
        break;
    }
    mCr = (uint8) x;

    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Min is %d,%d,%d\n", 
             (int)mY, (int)mCb, (int)mCr); //lint !e644

    *min = pack_ycbcr(mY,mCb,mCr);
    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Packed, it is %d\n", (int)*min);




    // find max 
    for (x = 255; x; x--)
    {
      if (Y[x] > 0)
        break;
    }
    mY = (uint8) x;

    for (x = 255; x; x--)
    {
      if (Cb[x] > 0)
        break;
    }
    mCb = (uint8) x;

    for (x = 255; x; x--)
    {
      if (Cr[x] > 0)
        break;
    }
    mCr = (uint8) x;


    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Max is %d,%d,%d\n", 
             (int)mY, (int)mCb, (int)mCr); //lint !e644

    *max = pack_ycbcr(mY,mCb,mCr);
    if (IPL_UTIL_DEBUG) //lint !e774 !e506
      printf("Packed, it is %d\n", (int)*max);
  }
  else
  {
    MSG_LOW("ipl_calc_minmax marker_202\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_calc_minmax marker_100\n");

  return IPL_SUCCESS;
}




/*===========================================================================

FUNCTION    ipl_calc_median

DESCRIPTION

  This function computes the median for crop area of pixels.

DEPENDENCIES
  None

ARGUMENTS IN
  in          input image
  crop        area in question

ARGUMENTS IN/OUT

  Returns average Y,Cb,Cr packed as 24 bits into uint32

RETURN VALUE

  Returns average Y,Cb,Cr packed as 24 bits into uint32

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN uint32 ipl_calc_median(ipl_image_type *in, ipl_rect_type *crop)
{
  uint32 x, y;
  unsigned char * p;
  unsigned int index;
  uint32 Y[256], Cb[256], Cr[256];
  uint32 med;
  uint32 half;

  uint32 mostYCnt, mostCbCnt, mostCrCnt;
  uint32 mostY, mostCb, mostCr;

  //MSG_LOW("ipl_calc_median marker_0\n");

  if (IPL_UTIL_DEBUG) //lint !e774 !e506
    printf("Going to find median value for (%lux%lu)@(%lu,%lu)\n", 
          crop->dx, crop->dy, crop->x, crop->y); 

  if (!in || !in->imgPtr)
  {
    MSG_LOW("ipl_calc_median marker_200\n");
    return 0;
  }

  if (!crop)
  {
    MSG_LOW("ipl_calc_median marker_201\n");
    return 0;
  }


  // initialize arrays
  for (x = 0; x < 256; x++)
    Y[x] = Cb[x] = Cr[x] = 0;

  //MSG_LOW("ipl_calc_median marker_1\n");
  p = in->imgPtr;
  if (in->cFormat == IPL_YCbCr)
  {
    for (y = 0; y < crop->dy; y++)
    {
      for (x = 0; x < 2*crop->dx; x += 4)
      {
        index = ((2*crop->x + crop->y*2*in->dx) + (x + y*2*in->dx));

        if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          printf("(%lu,%lu) CbYCrY is %d,%d,%d,%d\n", 
                 x, y, p[index], p[index+ 1], p[index+2], p[index+3]);

        Cb[p[index]]++;
         Y[p[index+1]]++;
        Cr[p[index+2]]++;
         Y[p[index+3]]++;
      }
    }
  } 
  else if ((in->cFormat == IPL_YCbCr444) ||
           (in->cFormat == IPL_RGB888) ||
           (in->cFormat == IPL_HSV))
  {
    for (y = 0; y < crop->dy; y++)
    {
      for (x = 0; x < crop->dx; x++)
      {
        index = ((3*crop->x + crop->y*3*in->dx) + (x*3 + y*3*in->dx));

        if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          printf("(%lu,%lu) CbCrY is %d,%d,%d\n", 
                 x, y, p[index], p[index+ 1], p[index+2]);

        Cb[p[index]]++;
        Cr[p[index+1]]++;
         Y[p[index+2]]++;
      }
    }
  } 
  else
  {
    MSG_LOW("ipl_calc_median marker_202\n");
    return 0;
  }



  if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
  {
    printf("Y histogram:\n");
    for (x = 0; x < 256; x++)
    {
      printf("%lu: %lu\n", x, Y[x]);
    }

    printf("Cb histogram:\n");
    for (x = 0; x < 256; x++)
    {
      printf("%lu: %lu\n", x, Cb[x]);
    }

    printf("Cr histogram:\n");
    for (x = 0; x < 256; x++)
    {
      printf("%lu: %lu\n", x, Cr[x]);
    }
  }

  // given histogram, now find mean
  // find median
  half = (crop->dx * crop->dy)>>1;

  if (IPL_UTIL_DEBUG) //lint !e774 !e506
    printf("%lu x %lu median Y is %lu\n", crop->dx, crop->dy, half);
   
  mostYCnt = 0; 
  if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
    printf("Y histogram:\n");
  for (x = 0; x < 256; x++)
  {
    mostYCnt += Y[x];

    if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
      printf("%lu %lu %lu\n", x, Y[x], mostYCnt);

    if (mostYCnt > half)
    {
      if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
        printf("found it!\n");
      break;
    }
  }
  mostY = x;

  if (IPL_UTIL_DEBUG) //lint !e774 !e506
    printf("%lu x %lu median Cb and Cr is %lu\n", crop->dx, crop->dy, half);
  mostCbCnt = 0; 

  if (in->cFormat == IPL_YCbCr)
    half = half >>1;

  if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
    printf("Cb histogram:\n");
  for (x = 0; x < 256; x++)
  {
    mostCbCnt += Cb[x];

    if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
      printf("%lu %lu %lu\n", x, Cb[x], mostCbCnt);

    if (mostCbCnt > half)
    {
      if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
        printf("found it!\n");
      break;
    }
  }
  mostCb = x;

  mostCrCnt = 0; 
  if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
    printf("Cr histogram:\n");
  for (x = 0; x < 256; x++)
  {
    mostCrCnt += Cr[x];

    if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
      printf("%lu %lu %lu\n", x, Cr[x], mostCrCnt);

    if (mostCrCnt > half)
    {
      if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
        printf("found it!\n");
      break;
    }
  }
  mostCr = x;


  if (IPL_UTIL_DEBUG) //lint !e774 !e506
    printf("Median YCbCr is %d,%d,%d\n", 
        (int)mostY, (int)mostCb, (int)mostCr); //lint !e644 

  med = pack_ycbcr(mostY,mostCb,mostCr);

  if (IPL_UTIL_DEBUG) //lint !e774 !e506
     printf("Packed, it is %d\n", (int)med);

  //MSG_LOW("ipl_calc_median marker_100\n");
  return med;

}




/*===========================================================================

FUNCTION    ipl_count_membership

DESCRIPTION

  This function computes how many pixels are of a given color.
  
DEPENDENCIES

  None

ARGUMENTS IN

  in          input image
  crop        area in question
  action      what to count

ARGUMENTS IN/OUT

  Returns value in either a1, a2, or a3

RETURN VALUE

  IPL_SUCCESS if all goes well
  IPL_FAILURE if not all goes well

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_count_members(ipl_image_type *in, 
    ipl_rect_type *crop, int action, uint32 * a1, uint32 * a2, uint32 * a3)
{
  uint32 x, y;
  unsigned char * p;
  //int32 diff;
  //uint32 cnt;
  uint32 cnt2;
  int32 diff2;

  //MSG_LOW("ipl_count_members marker_0\n");
  if (!crop)
  {
    MSG_LOW("ipl_count_members marker_200\n");
    return IPL_FAILURE;
  }
  if (IPL_UTIL_DEBUG) //lint !e774 !e506
    printf("Going to find # colors value for (%lux%lu)@(%lu,%lu)\n", 
          crop->dx, crop->dy, crop->x, crop->y); 

  if (!in || !in->imgPtr)
  {
    MSG_LOW("ipl_count_members marker_201\n");
    return IPL_FAILURE;
  }
  p = in->imgPtr;
  p += (2*crop->x + crop->y*2*in->dx);

  //MSG_LOW("ipl_count_members marker_1\n");
  //cnt = 0;
  cnt2 = 0;
  if (action == 0)
  {
    for (y = 0; y < crop->dy; y++)
    {
      for (x = 0; x < crop->dx; x += 2)
      {
#if 0
        if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          printf("(%d,%d) CbYCrY is %d,%d,%d,%d\n", x, y, *p, *(p+1), *(p+2), *(p+3));
        // normally, if an image has pixels with Cb and CR that are not close
        // to 128, then those pixels can be considered non gray. So we do
        // that check.
        diff = IPL_ABS(((*p)-128)) + IPL_ABS(((*p+2)-128));
        if (diff < 30)
        {
          if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          {
            printf("Cb and Cr are NOT far from 128\n");
          }
        }
        else
        {

          if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          {
            printf("Cb and Cr are far from 128\n");
          }
          cnt++;
        }
#endif

        // however, if the entire image for the most part has Cb and Cr
        // that are not close to 128, it can still be benefitial to
        // call them gray, and only marke pixels who have big differences
        // in Cb and Cr as truely gray.
        diff2 = *p - *(p+2); 
        diff2 = IPL_ABS(diff2);

        if (diff2 > (int32) *a1)
        {
          if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          {
            printf("Cb and Cr are far apart here\n");
          }
          cnt2++;
        }
        else
        {
          if (IPL_UTIL_DEBUG && 0) //lint !e774 !e506
          {
            printf("Cb and Cr are NOT far apart here\n");
          }
        }
        p += 4;
      }
      p += ((in->dx - crop->dx) * 2);

    }

    //*a2 = IPL_MIN(cnt, cnt2);
    *a2 = cnt2;
  }
  else
  {
    MSG_LOW("ipl_count_members marker_202\n");
    return IPL_FAILURE;
  }
  //MSG_LOW("ipl_count_members marker_100\n");
  return IPL_SUCCESS;
} //lint !e715





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_calc_sad

DESCRIPTION
    This function returns the sad of two images.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr            pointer to the input frame    
  o_img_ptr            pointer to the output frame
  in1                  region to process in first image
  in2                  region to process in second image

RETURN VALUE
  IPL_SUCCESS          indicates operation was successful
  IPL_FAILURE          otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_calc_sad
(
  ipl_image_type* i1_img_ptr,        /* Points to the input frame  */
  ipl_image_type* i2_img_ptr,        /* Points to the output frame */
  ipl_rect_type *in1,
  ipl_rect_type *in2,
  int operation,
  uint32 *a1,
  uint32 *a2,
  uint32 *a3
)
{
  unsigned char *in1ImgPtr, *in2ImgPtr;
  uint32 row, col;
  uint32 accum = 0;
  uint32 pitch1x;
  uint32 pitch2x;
  int32 diff;

  /* Check parameters */
  if (i1_img_ptr == NULL || 
      i2_img_ptr == NULL ||
      i1_img_ptr->imgPtr == NULL || 
      i2_img_ptr->imgPtr == NULL)
  {
    return IPL_FAILURE;
  }

  if (!in1 || !in2)
    return IPL_FAILURE;


  if (in1->dx != in2->dx || in1->dy != in2->dy)
    return IPL_FAILURE;
    
  in1ImgPtr = (i1_img_ptr->imgPtr + 2*(in1->x + in1->y*i1_img_ptr->dy));
  in2ImgPtr = (i2_img_ptr->imgPtr + 2*(in2->x + in2->y*i2_img_ptr->dy));

  pitch1x = 2*(i1_img_ptr->dx - in1->dx);
  pitch2x = 2*(i2_img_ptr->dx - in2->dx);

  /* Loop through images */
  accum = 0;
  for(row = 0; row < in1->dy; row++) 
  {
    for(col = 0; col < in1->dx * 2; col++) 
    {
      if (operation)
        printf("(%lu,%lu) diff of %d %d\n",col, row, *in1ImgPtr, *in2ImgPtr); 

      diff = *in1ImgPtr++ - *in2ImgPtr++;
      diff = IPL_ABS(diff);
      in1ImgPtr++;
      in2ImgPtr++;
      accum += (uint32) (diff * diff);
    }

    in1ImgPtr += pitch1x;
    in2ImgPtr += pitch2x;
  }

  *a1 = accum;
  *a2 = 0;
  *a3 = 0;


	return IPL_SUCCESS;
}



/*===========================================================================
 * ===========================================================================*/
void ipl_noop(char * str, void * a, void * b, void * c)
{
}//lint !e715




/*lint -restore */
