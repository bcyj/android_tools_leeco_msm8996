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

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_compose.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
  3/2/06   bf      added ipl_alpha_blend_area
 2/14/06   bf      Modified HxVx to RGB color conversion to match JPEG's method
12/06/05   bf      Fixed bug ipl_frame_compose when adding icons as overlay
11/17/05   bf      Added per pixel alpha blending function, IPL_BLEND_PER_PIXEL
11/15/05   bf      fixed bug 422 to 420 copy and paste. Also fixed bug
                   from H2v2 to rgb565 and rgb888 if odd output size.
10/29/05   bf      Removed the dumb idea to resize when copy and paste 
10/29/05   bf      Wrote ipl_draw_histogram
08/29/05   bf      Added cropin support to HxVx to RGB and YCbCr copy and paste
08/21/05   bf      Added rgb888 output HxVx copy and paste function
08/21/05   bf      Added crop on input support for HxVx function to RGB565
08/09/05   kwa     Coverity - fixed RESOURCE_LEAK in function ipl_center
                   Coverity - fixed RESOURCE_LEAK in function
                   ipl_copy_and_paste_rgb565
06/17/05   bf      Added ycrcb420 line pack support to ipl_center, movefilm.
04/12/05   bf      Added 420 line pack verson of horizontal, vertical. Added
                   inplace support for all ipl ipl_comp_AddImage func
03/24/05   bf      added RGB565 output support from HxVx input for copyPaste
02/21/05   bf      fixed bug in ipl_image_add_inplace. YCrCb420lp and YCbCr420
                   line pack now (as they should be) are treated separatly.
02/18/05   bf      wrote ipl_copy_and_paste_h1v1 which convert to 420 line pack
                   wrote ipl_copy_and_paste_h2v2 which convert to 420 line pack
                   wrote ipl_copy_and_paste_h1v2 which convert to 420 line pack
                   wrote ipl_copy_and_paste_h2v1 which convert to 420 line pack
02/14/05   bf      wrote ipl_image_add_inplace to add frame to image in place.
                   Currently support 420 line pack only and makes sure no 
                   color bleeding occurs at borders if frame is on odd row/col.
02/14/05   bf      fixed bug when added 565 frames to 565 image.
02/14/05   bf      Added 420lp support to blend, blend_black, blend_white
02/04/05   bf      Added YCxCx422lp <-> YCxCx420lp support to copy_and_paste
02/04/05   bf      Added 420lp support to copy_and_paste
01/13/05   mz      Added ipl_add_frame_ycrcb420lp and ipl_add_frame_ycbcr444.
12/11/04   bf      for ipl_image_add, added support so frame can be smaller
                   than input image. Input and output must be same though.
12/21/04   bf      Added ipl_cut_and_paste, and ipl_fill_rect
11/04/04   mz      Added ipl_alpha_blend_black and ipl_alpha_blend_white. 
                   Optimized ipl_alpha_blend_rgb565.
10/26/04   mz      Added ipl_copy_and_paste.
10/05/04   mz      Updated function APIs. Updated comments to indicate new 
                   support for YCbCr 4:2:0 line packed format. Corrected tab 
                   spacings and line widths.
09/01/04   bf      Created. See ipl_util.c for previous history.
===========================================================================*/



/* <EJECT> */
/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include <stdio.h>
#include <string.h>
#include "ipl_types.h"
#include "ipl_helper.h"
#include "ipl_xform.h"
#include "ipl_convert.h"
#include "ipl_compose.h"


// Turn off some lint warnings
/*lint -save -e504, all shifts are okay */
/*lint -save -e506, constant value boolean is totally okay */
/*lint -save -e508, extra extern is okay */
/*lint -save -e534, let me call printf in piece, god */
/*lint -save -e573, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e574, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e701, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e702, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e704, all shifts are okay */
/*lint -save -e713, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e715, okay that bip_ptr not used */
/*lint -save -e732, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e737, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e734, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e774, constant value boolean is totally okay */
/*lint -save -e818, lets not worry about const optimization right now */
/*lint -save -e834, this warning is just dumb */
/*lint -save -e826, we should look into this someday */

#define IPL_COMPOSE_DEBUG 0




extern int16 ipl2_rgb565ToYR[];/* R to Y conversion normalized RGB565 */
extern int16 ipl2_rgb565ToYG[];/* G to Y conversion normalized RGB565 */
extern int16 ipl2_rgb565ToYB[];/* B to Y conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCbR[];/* R to Cb conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCbG[];/* G to Cb conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCbB[];/* B to Cb conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCrR[];/* R to Cr conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCrG[];/* G to Cr conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCrB[];/* B to Cr conversion normalized RGB565 */





/* <EJECT> */
/*==========================================================================

FUNCTION    ipl_add_frame_ycrcb420lp

DESCRIPTION
  This function adds a frame. The input parameter transparentY must 
  specify the luma value for a transparent pixel. The frame must have 
  no odd-pixel transitions between transparent and non-transparent 
  regions in either the horizontal or the vertical direction.

  Input, frame, and output images must be in YCrCb 4:2:0 line 
  packed format.

  Input, frame, and output images must have the same size.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         points to the input image
  i_frame_ptr       points to the frame image
  transparentY      transparent luma value

ARGUMENTS IN/OUT
  o_img_ptr         points to the output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

==========================================================================*/
extern ipl_status_type ipl_add_frame_ycrcb420lp
(
  ipl_image_type* i_img_ptr,        /* Points to the input image  */
  ipl_image_type* i_frame_ptr,      /* Points to the frame        */
  ipl_image_type* o_img_ptr,        /* Points to the output image */
  uint8 transparentY                /* Transparent pixel value    */
)
{
  uint8 *inImgPtr, *frameImgPtr, *outImgPtr, *inClrPtr, *outClrPtr, 
        *frameClrPtr, *in2ImgPtr, *frame2ImgPtr, *out2ImgPtr;
  uint32 x, y, w;

  MSG_LOW("ipl_add_frame_ycrcb420lp marker_0\n");

  if (!i_img_ptr    || !i_img_ptr->imgPtr ||
      !o_img_ptr    || !o_img_ptr->imgPtr ||
      !i_frame_ptr  || !i_frame_ptr->imgPtr)
  {
    MSG_LOW("ipl_add_frame_ycrcb420lp marker_200\n");
    return IPL_FAILURE;
  }

  if ((i_img_ptr->cFormat != IPL_YCrCb420_LINE_PK) || 
      (i_frame_ptr->cFormat != IPL_YCrCb420_LINE_PK) || 
      (o_img_ptr->cFormat != IPL_YCrCb420_LINE_PK))
  {
    MSG_LOW("ipl_add_frame_ycrcb420lp marker_201\n");
    return IPL_FAILURE;
  }
  /* Initialize image pointers and local variables */
  inImgPtr = i_img_ptr->imgPtr;
  frameImgPtr = i_frame_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;
  inClrPtr = i_img_ptr->clrPtr;
  frameClrPtr = i_frame_ptr->clrPtr;
  outClrPtr = o_img_ptr->clrPtr;

  w = o_img_ptr->dx;
  in2ImgPtr = inImgPtr + w;
  frame2ImgPtr = frameImgPtr + w;
  out2ImgPtr = outImgPtr + w;

  MSG_LOW("ipl_add_frame_ycrcb420lp marker_1\n");

  /* Loop through frame */
  for (y = o_img_ptr->dy>>1; y; y--) {
    for (x = w>>1; x; x--) {
      /* 
      ** Process 4 pixels at a time. Assume no odd pixel transitions in frame 
      ** in x or y direction.
      */
      if (*frameImgPtr != transparentY) {
        /* 
        ** Copy frame pixels to output
        */
        *outImgPtr++ = *frameImgPtr++;
        *outImgPtr++ = *frameImgPtr++;
        *out2ImgPtr++ = *frame2ImgPtr++;
        *out2ImgPtr++ = *frame2ImgPtr++;
        *outClrPtr++ = *frameClrPtr++;
        *outClrPtr++ = *frameClrPtr++;
        inImgPtr += 2;
        in2ImgPtr += 2;
        inClrPtr += 2;
      } else {
        /*
        ** Copy input pixels to output
        */
        *outImgPtr++ = *inImgPtr++;
        *outImgPtr++ = *inImgPtr++;
        *out2ImgPtr++ = *in2ImgPtr++;
        *out2ImgPtr++ = *in2ImgPtr++;
        *outClrPtr++ = *inClrPtr++;
        *outClrPtr++ = *inClrPtr++;
        frameImgPtr += 2;
        frame2ImgPtr += 2;
        frameClrPtr += 2;
      }
    } /* end x loop */
    inImgPtr += w;
    in2ImgPtr += w;
    frameImgPtr += w;
    frame2ImgPtr += w;
    outImgPtr += w;
    out2ImgPtr += w;
  } /* end y loop */

  MSG_LOW("ipl_add_frame_ycrcb420lp marker_100\n");
  return IPL_SUCCESS;
} /* end of function ipl_add_frame_ycrcb420lp */


/* <EJECT> */
/*==========================================================================

FUNCTION    ipl_add_frame_ycbcr444

DESCRIPTION
  This function adds a frame. The input parameter transparentY must 
  specify the luma value for a transparent pixel.

  Input, frame, and output images must all be in YCrCb 4:4:4 format.
  Input, frame, and output images must have the same size.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         points to the input image
  i_frame_ptr       points to the frame image
  transparentY      transparent luma value

ARGUMENTS IN/OUT
  o_img_ptr         points to the output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

==========================================================================*/
extern ipl_status_type ipl_add_frame_ycbcr444
(
  ipl_image_type* i_img_ptr,        /* Points to the input image  */
  ipl_image_type* i_frame_ptr,      /* Points to the frame        */
  ipl_image_type* o_img_ptr,        /* Points to the output image */
  uint8 transparentY                /* Transparent pixel value    */
)
{
  uint8 *inImgPtr, *frameImgPtr, *outImgPtr;
  uint32 x;

  MSG_LOW("ipl_add_frame_ycbcr444 marker_0\n");

  if (!i_img_ptr    || !i_img_ptr->imgPtr ||
      !o_img_ptr    || !o_img_ptr->imgPtr ||
      !i_frame_ptr  || !i_frame_ptr->imgPtr)
  {
    MSG_LOW("ipl_add_frame_ycbcr444 marker_200\n");
    return IPL_FAILURE;
  }

  if ((i_img_ptr->cFormat != IPL_YCbCr444) || 
      (i_frame_ptr->cFormat != IPL_YCbCr444) || 
      (o_img_ptr->cFormat != IPL_YCbCr444)) 
  {
    MSG_LOW("ipl_add_frame_ycbcr444 marker_201\n");
    return IPL_FAILURE;
  }
  /* Initialize image pointers and local variables */
  inImgPtr = i_img_ptr->imgPtr;
  frameImgPtr = i_frame_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;

  MSG_LOW("ipl_add_frame_ycbcr444 marker_1\n");

  /* Loop through frame */
  for (x = o_img_ptr->dx*o_img_ptr->dy; x; x--) {
    if (*((uint8*)(frameImgPtr+2)) != transparentY) {
      /* 
      ** Copy frame pixels to output
      */
      *outImgPtr++ = *frameImgPtr++;
      *outImgPtr++ = *frameImgPtr++;
      *outImgPtr++ = *frameImgPtr++;
      inImgPtr += 3;
    } else {
      /*
      ** Copy input pixels to output
      */
      *outImgPtr++ = *inImgPtr++;
      *outImgPtr++ = *inImgPtr++;
      *outImgPtr++ = *inImgPtr++;
      frameImgPtr += 3;
    }
  } /* end x loop */

  MSG_LOW("ipl_add_frame_ycbcr444 marker_100\n");
  return IPL_SUCCESS;
} /* end of function ipl_add_frame_ycbcr444 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_cut_and_paste

DESCRIPTION
  This function performs cut-and-paste. A region is cut from the input 
  image and pasted onto a region in the output image. Input and output 
  image sizes can be different. If the input "cut" region is strictly 
  larger than the output "paste" region, the input region will be downsized 
  to fit the output region. If the images are RGB565, an input "cut" region 
  can also be upsized if it is strictly smaller than the output "paste" 
  region.

  Input and output images must be have the same color format, which can be 
  RGB565 or YCbCr 4:2:2 or YCbCr 4:2:0lp.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  cut_area is the input area to be cut out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image
  in_img_ptr  points to the input image which will be left with a cut region
              of color fillerPixel after the function 

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_cut_and_paste
(
  ipl_image_type* in_img_ptr,   /* Points to the input image           */
  ipl_image_type* out_img_ptr,  /* Points to the output image          */
  ipl_rect_type* cut_area,      /* Input region to be copy out          */
  ipl_rect_type* paste_area,    /* Output region to be pasted onto     */
  uint32 fillerPixel            /* pixel color of cut region after cut */
)
{
  MSG_LOW("ipl_cut_and_paste marker_0\n");

  /* copy the region over */
  if (ipl_copy_and_paste(in_img_ptr, out_img_ptr, cut_area, paste_area) !=
      IPL_SUCCESS)
  {
     MSG_LOW("ipl_cut_and_paste marker_200\n");
    return IPL_FAILURE;
  }
  /* fill in region with fillerPixel color */
  if (ipl_draw_box(in_img_ptr, NULL,cut_area,1,fillerPixel) != IPL_SUCCESS)
  {
    MSG_LOW("ipl_cut_and_paste marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_cut_and_paste marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_copy_and_paste */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_rgb888

DESCRIPTION
  This function performs copy-and-paste. 
 
  Input and output images must be in RGB888 format.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_rgb888
(
  ipl_image_type* in_img_ptr,   /* Points to the input image           */
  ipl_image_type* out_img_ptr,  /* Points to the output image          */
  ipl_rect_type* copy_area,      /* Input region to be copy out          */
  ipl_rect_type* paste_area     /* Output region to be pasted onto     */
)
{
  uint8 *inImgPtr, *outImgPtr;
  uint32 x, y, inInc, outInc;

  MSG_LOW("ipl_copy_and_paste_rgb888 marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_rgb888 marker_200\n");
    return IPL_FAILURE;
  }

  // this functions if or RGB888 data
  if (in_img_ptr->cFormat != IPL_RGB888) 
  {
    MSG_LOW("ipl_copy_and_paste_rgb888 marker_201\n");
    return IPL_FAILURE;
  }
  // copy and paste area must be same size
  if ((copy_area->dx != paste_area->dx) ||
      (copy_area->dy != paste_area->dy))
  {
    MSG_LOW("ipl_copy_and_paste_rgb888 marker_202\n");
    return IPL_FAILURE;
  }
  // make sure copy and paste areas do not send us out of bounds
  if ((copy_area->x + copy_area->dx > in_img_ptr->dx)    ||
      (paste_area->x + paste_area->dx > out_img_ptr->dx) ||
      (copy_area->y + copy_area->dy > in_img_ptr->dy)    ||
      (paste_area->y + paste_area->dy > out_img_ptr->dy))
  {
    MSG_LOW("ipl_copy_and_paste_rgb888 marker_203\n");
    return IPL_FAILURE;
  }
  /* Initialize image pointers and local variables */
  inImgPtr = in_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;

  /* Cut and paste regions have the same size */
  inImgPtr += copy_area->y * in_img_ptr->dx * 3 + copy_area->x * 3;
  outImgPtr += paste_area->y * out_img_ptr->dx * 3 + paste_area->x * 3;

  inInc = 3*(in_img_ptr->dx - copy_area->dx);
  outInc = 3*(out_img_ptr->dx - copy_area->dx);

  MSG_LOW("ipl_copy_and_paste_rgb888 marker_1\n");

  for (y = 0; y < copy_area->dy; y++) 
  {
    for (x = 0; x < copy_area->dx; x++) 
    {
      *outImgPtr++ = *inImgPtr++;
      *outImgPtr++ = *inImgPtr++;
      *outImgPtr++ = *inImgPtr++;
    } 

    inImgPtr += inInc;
    outImgPtr += outInc;
  } 

  MSG_LOW("ipl_copy_and_paste_rgb888 marker_100\n");

  return IPL_SUCCESS;
} 




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_rgb565

DESCRIPTION
  This function performs copy-and-paste. 
 
  Input and output images must be in RGB565 format.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_rgb565
(
  ipl_image_type* in_img_ptr,   /* Points to the input image           */
  ipl_image_type* out_img_ptr,  /* Points to the output image          */
  ipl_rect_type* copy_area,      /* Input region to be copy out          */
  ipl_rect_type* paste_area     /* Output region to be pasted onto     */
)
{
  uint16 *inImgPtr, *outImgPtr;
  uint32 x, y, inInc, outInc;

  MSG_LOW("ipl_copy_and_paste_rgb565 marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  // this functions if or RGB565 data
  if (in_img_ptr->cFormat != IPL_RGB565) 
  {
    MSG_LOW("ipl_copy_and_paste_rgb565 marker_201\n");
    return IPL_FAILURE;
  }
  // copy and paste area must be same size
  if ((copy_area->dx != paste_area->dx) ||
      (copy_area->dy != paste_area->dy))
  {
    MSG_LOW("ipl_copy_and_paste_rgb565 marker_202\n");
    return IPL_FAILURE;
  }
  // make sure copy and paste areas do not send us out of bounds
  if ((copy_area->x + copy_area->dx > in_img_ptr->dx)    ||
      (paste_area->x + paste_area->dx > out_img_ptr->dx) ||
      (copy_area->y + copy_area->dy > in_img_ptr->dy)    ||
      (paste_area->y + paste_area->dy > out_img_ptr->dy))
  {
    MSG_LOW("ipl_copy_and_paste_rgb565 marker_203\n");
    return IPL_FAILURE;
  }
  /* Initialize image pointers and local variables */
  inImgPtr = (uint16*) in_img_ptr->imgPtr;
  outImgPtr = (uint16*) out_img_ptr->imgPtr;

  /* Cut and paste regions have the same size */
  inImgPtr += copy_area->y * in_img_ptr->dx + copy_area->x;
  outImgPtr += paste_area->y * out_img_ptr->dx + paste_area->x;

  inInc = in_img_ptr->dx - copy_area->dx;
  outInc = out_img_ptr->dx - copy_area->dx;

  MSG_LOW("ipl_copy_and_paste_rgb565 marker_1\n");

  for (y = 0; y < copy_area->dy; y++) 
  {
    for (x = 0; x < copy_area->dx; x++) 
    {
      *outImgPtr++ = *inImgPtr++;
    } 

    inImgPtr += inInc;
    outImgPtr += outInc;
  } 

  MSG_LOW("ipl_copy_and_paste_rgb565 marker_100\n");

  return IPL_SUCCESS;
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_ycbcr

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input and output images must be in YCbCr 4:2:2 format 

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_ycbcr
(
  ipl_image_type *in_img_ptr,   /* Points to the input image           */
  ipl_image_type *out_img_ptr,  /* Points to the output image          */
  ipl_rect_type  *copy_area,      /* Input region to be copy out          */
  ipl_rect_type  *paste_area     /* Output region to be pasted onto     */
)
{
  unsigned char *inImgPtr;
  unsigned char *outImgPtr;
  uint32 x, y, wcopy, wpaste, hpaste, inInc, outInc;
  uint32 win, wout, xscopy, xspaste;

  MSG_LOW("ipl_copy_and_paste_ycbcr marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  if (in_img_ptr->cFormat != IPL_YCbCr) 
  {
    MSG_LOW("ipl_copy_and_paste_ycbcr marker_201\n");
    return IPL_FAILURE;
  }
  /* setup our input sizes and such */
  win = in_img_ptr->dx;
  wout = out_img_ptr->dx;

  xscopy = copy_area->x;
  wcopy = copy_area->dx;

  xspaste = paste_area->x;
  wpaste = paste_area->dx;
  hpaste = paste_area->dy;

  /* if user want us to start copying on odd pixel, make it even */
  /* i.e. we only copy CbYCrY chunks of data, not CrY            */
  if (xscopy%2) xscopy--;
  if (xspaste%2) xspaste--;

  /* make sure we copy even width area */
  if (wcopy%2) wcopy--;
  if (wpaste%2) wpaste--;

  /* make sure we dont go over our input bounds */
  while ((wcopy+xscopy)>win) 
  {
    wcopy -=2;
    wpaste -=2;
  }

  /* make sure we dont go over our output bounds */
  while ((wpaste+xspaste) > wout) 
  {
    wcopy -=2;
    wpaste -=2;
  }

  MSG_LOW("ipl_copy_and_paste_ycbcr marker_1\n");

  /* Initialize image pointers and local variables */
  if (in_img_ptr->cFormat == IPL_YCbCr) 
  {
    inImgPtr = in_img_ptr->imgPtr;
    outImgPtr = out_img_ptr->imgPtr;

    /* go to upper left of where we cut and where we are going to paste */
    inImgPtr += (copy_area->y * win + xscopy)*2;
    outImgPtr += (paste_area->y * wout + xspaste)*2;

    inInc = (win - wcopy)*2;
    outInc = (wout - wpaste)*2;

    /* do two pixels at a time */
    for (y = 0; y < hpaste; y++) 
    {
      for (x = 0; x < wpaste/2; x++) 
      {
        *outImgPtr++ = *inImgPtr++; /* copy cb */
        *outImgPtr++ = *inImgPtr++; /* copy y  */
        *outImgPtr++ = *inImgPtr++; /* copy cr */
        *outImgPtr++ = *inImgPtr++; /* copy y  */
      } /* end x loop */

      inImgPtr += inInc;
      outImgPtr += outInc;
    } /* end y loop */
  }
  else 
  {
    MSG_LOW("ipl_copy_and_paste_ycbcr marker_202\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_copy_and_paste_ycbcr marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_copy_and_paste_ycbcr */




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_ycc420

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input image must be 

  YCxCx420 line pack <---> YCxCx42y line pack (where x is r|b and y is 0|2) 

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_ycc420
(
  ipl_image_type *in_img_ptr,   /* Points to the input image           */
  ipl_image_type *out_img_ptr,  /* Points to the output image          */
  ipl_rect_type  *copy_area,      /* Input region to be copy out          */
  ipl_rect_type  *paste_area     /* Output region to be pasted onto     */
)
{
  unsigned char *inImgPtr, *inImgPtr2, *inClrPtr;
  unsigned char *outImgPtr, *outImgPtr2, *outClrPtr, *outClrPtr2;
  uint32 x, y, wcopy, hcopy, wpaste, hpaste, inInc, outInc;
  uint32 win, wout, xscopy, xspaste;
  boolean swap = FALSE;

  MSG_LOW("ipl_copy_and_paste_ycc420 marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr  ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_ycc420 marker_200\n");
    return IPL_FAILURE;
  }

  if ((in_img_ptr->cFormat != IPL_YCrCb420_LINE_PK) &&
      (in_img_ptr->cFormat != IPL_YCbCr420_LINE_PK))
  {
    MSG_LOW("ipl_copy_and_paste_ycc420 marker_201\n");
    return IPL_FAILURE;
  }

  /* setup our input sizes and such */
  win  = in_img_ptr->dx;
  wout = out_img_ptr->dx;

  xscopy = copy_area->x;
  wcopy = copy_area->dx;
  hcopy = copy_area->dy;

  xspaste = paste_area->x;
  wpaste = paste_area->dx;
  hpaste = paste_area->dy;

  /* if user want us to start copying on odd pixel, make it even */
  /* i.e. we only copy CbYCrY chunks of data, not CrY            */
  if (xscopy%2) xscopy--;
  if (xspaste%2) xspaste--;

  /* make sure we copy even width area */
  if (wcopy%2) wcopy--;
  if (wpaste%2) wpaste--;

  /* make sure we dont go over our input bounds */
  while ((wcopy+xscopy)>win) 
  {
    wcopy -=2;
    wpaste -=2;
  }

  /* make sure we dont go over our output bounds */
  while ((wpaste+xspaste) > wout) 
  {
    wcopy -=2;
    wpaste -=2;
  }

  if (wcopy != wpaste || hcopy != hpaste)
  {
    MSG_LOW("ipl_copy_and_paste_ycc420 marker_202\n");
    return IPL_FAILURE;
  }
  /* see if we need to swap Cr and Cb */
  if (in_img_ptr->cFormat == IPL_YCbCr420_LINE_PK && 
      out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
    swap = TRUE;

  if (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK && 
      out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)
    swap = TRUE;

  if (in_img_ptr->cFormat == IPL_YCbCr420_LINE_PK && 
      out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
    swap = TRUE;

  if (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK && 
      out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK)
    swap = TRUE;


  /* setup our input pointers */
  inImgPtr  = in_img_ptr->imgPtr;            /* first y ptr */
  inImgPtr2 = in_img_ptr->imgPtr + win;     /* next  y ptr, one line below */
  inClrPtr  = in_img_ptr->clrPtr;            /* cbcr ptr    */

  outImgPtr  = out_img_ptr->imgPtr;
  outImgPtr2 = out_img_ptr->imgPtr + wout;
  outClrPtr  = out_img_ptr->clrPtr;
  outClrPtr2 = out_img_ptr->clrPtr + wout;

  MSG_LOW("ipl_copy_and_paste_ycc420 marker_1\n");

  if ((out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK))
  {
    /* go to upper left of where we cut and where we are going to paste */
    inImgPtr   += (copy_area->y * win + xscopy);
    inImgPtr2  += (copy_area->y * win + xscopy);
    inClrPtr   += ((copy_area->y/2)*win + 2*(xscopy/2));
               
    outImgPtr  += (paste_area->y * wout + xspaste);
    outImgPtr2 += (paste_area->y * wout + xspaste);
    outClrPtr  += ((paste_area->y*wout) + 2*(xspaste/2));
    outClrPtr2 += ((paste_area->y*wout) + 2*(xspaste/2));

    inInc  = (win - wcopy);
    outInc = (wout - wpaste);

    /* do two rows at a time */
    for (y = hpaste/2; y; y--)
    {
      /* do two pixels at a time */
      for (x = wpaste/2; x; x--) 
      {
        /* copy 4 lumas, in the following order
         *
         * Y1 Y3
         * Y2 Y4
         *
         * */
        /* 1st column */
        *outImgPtr++ = *inImgPtr++;
        *outImgPtr2++ = *inImgPtr2++;

        /* 2nd column */
        *outImgPtr++ = *inImgPtr++;
        *outImgPtr2++ = *inImgPtr2++;

        /* copy Cb and Cr 
         *
         * Cb Cr
         *
         */
        if (swap)
        {
          *outClrPtr++ = *(inClrPtr+1);
          *outClrPtr2++ = *(inClrPtr+1);

          *outClrPtr++ = *(inClrPtr);
          *outClrPtr2++ = *(inClrPtr);

          inClrPtr +=2;
        }
        else
        {
          *outClrPtr++ = *inClrPtr;
          *outClrPtr2++ = *inClrPtr++;

          *outClrPtr++ = *inClrPtr;
          *outClrPtr2++ = *inClrPtr++;
        }
      } 

      /* each pointer goes 2 lines down */
      inImgPtr   += (inInc + win);
      inImgPtr2  += (inInc + win);
      outImgPtr  += (outInc + wout);
      outImgPtr2 += (outInc + wout);

      /* goes to correct position in next line */
      inClrPtr   += (inInc);
      outClrPtr  += (outInc + wout);
      outClrPtr2 += (outInc + wout);
    } 
  }
  else if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
           (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    /* go to upper left of where we cut and where we are going to paste */
    inImgPtr += (copy_area->y * win + xscopy);
    inImgPtr2 += (copy_area->y * win + xscopy);
    inClrPtr += ((copy_area->y/2)*win + 2*(xscopy/2));

    outImgPtr += (paste_area->y * wout + xspaste);
    outImgPtr2 += (paste_area->y * wout + xspaste);
    outClrPtr += ((paste_area->y/2)*wout + 2*(xspaste/2));

    inInc = (win - wcopy);
    outInc = (wout - wpaste);

    /* do two rows at a time */
    for (y = 0; y < hpaste/2; y++) 
    {
      /* do two pixels at a time */
      for (x = 0; x < wpaste/2; x++) 
      {
        /* copy 4 lumas, in the following order
         *
         * Y1 Y3
         * Y2 Y4
         *
         * */
        /* 1st column */
        *outImgPtr++ = *inImgPtr++;
        *outImgPtr2++ = *inImgPtr2++;

        /* 2nd column */
        *outImgPtr++ = *inImgPtr++;
        *outImgPtr2++ = *inImgPtr2++;

        /* copy Cb and Cr 
         *
         * Cb Cr
         *
         */
        if (swap)
        {
          *outClrPtr++ = *(inClrPtr+1);
          *outClrPtr++ = *(inClrPtr);
          inClrPtr +=2;
        }
        else
        {
          *outClrPtr++ = *inClrPtr++;
          *outClrPtr++ = *inClrPtr++;
        }
      } 
      /* each pointer goes 2 lines down */
      inImgPtr += (inInc + win);
      inImgPtr2 += (inInc + win);
      outImgPtr += (outInc + wout);
      outImgPtr2 += (outInc + wout);

      /* goes to correct position in next line */
      inClrPtr += (inInc);
      outClrPtr += (outInc);
    } 
  }
  else
  {
    MSG_LOW("ipl_copy_and_paste_ycc420 marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_copy_and_paste_ycc420 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_copy_and_paste_ycc420 */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_ycc422

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input image must be 

  YCxCx422 line pack <---> YCxCx42y line pack (where x is r|b and y is 0|2) 

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_ycc422
(
  ipl_image_type* in_img_ptr,   /* Points to the input image           */
  ipl_image_type* out_img_ptr,  /* Points to the output image          */
  ipl_rect_type* copy_area,      /* Input region to be copy out          */
  ipl_rect_type* paste_area     /* Output region to be pasted onto     */
)
{
  unsigned char *inImgPtr, *inImgPtr2, *inClrPtr;
  unsigned char *outImgPtr, *outImgPtr2, *outClrPtr;
  uint32 x, y, wcopy, hcopy, wpaste, hpaste, inInc, outInc;
  uint32 win, wout, xscopy, xspaste;
  boolean swap = FALSE;

  MSG_LOW("ipl_copy_and_paste_ycc422 marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_ycc422 marker_200\n");
    return IPL_FAILURE;
  }

  if ((in_img_ptr->cFormat != IPL_YCrCb422_LINE_PK) &&
      (in_img_ptr->cFormat != IPL_YCbCr422_LINE_PK))
  {
    MSG_LOW("ipl_copy_and_paste_ycc422 marker_201\n");
    return IPL_FAILURE;
  }
  /* setup our input sizes and such */
  win = in_img_ptr->dx;
  wout = out_img_ptr->dx;

  xscopy = copy_area->x;
  wcopy = copy_area->dx;
  hcopy = copy_area->dy;

  xspaste = paste_area->x;
  wpaste = paste_area->dx;
  hpaste = paste_area->dy;

  /* if user want us to start copying on odd pixel, make it even */
  /* i.e. we only copy CbYCrY chunks of data, not CrY            */
  if (xscopy%2) xscopy--;
  if (xspaste%2) xspaste--;

  /* make sure we copy even width area */
  if (wcopy%2) wcopy--;
  if (wpaste%2) wpaste--;

  /* make sure we dont go over our input bounds */
  while ((wcopy+xscopy)>win) 
  {
    wcopy -=2;
    wpaste -=2;
  }

  /* make sure we dont go over our output bounds */
  while ((wpaste+xspaste) > wout) 
  {
    wcopy -=2;
    wpaste -=2;
  }

  if (wcopy != wpaste || hcopy != hpaste) 
  {
    MSG_LOW("ipl_copy_and_paste_ycc422 marker_202\n");
    return IPL_FAILURE;
  }
  /* setup our input pointers */
  inImgPtr = in_img_ptr->imgPtr;            /* first y ptr */
  inImgPtr2 = in_img_ptr->imgPtr + win;     /* next  y ptr, one line below */
  inClrPtr = in_img_ptr->clrPtr;            /* cbcr ptr    */

  outImgPtr = out_img_ptr->imgPtr;
  outImgPtr2 = out_img_ptr->imgPtr + wout;
  outClrPtr = out_img_ptr->clrPtr;

  /* see if we need to swap Cr and Cb */
  if (in_img_ptr->cFormat == IPL_YCbCr422_LINE_PK && 
      out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
    swap = TRUE;

  if (in_img_ptr->cFormat == IPL_YCrCb422_LINE_PK && 
      out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK)
    swap = TRUE;

  if (in_img_ptr->cFormat == IPL_YCbCr422_LINE_PK && 
      out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
    swap = TRUE;

  if (in_img_ptr->cFormat == IPL_YCrCb422_LINE_PK && 
      out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)
    swap = TRUE;

  MSG_LOW("ipl_copy_and_paste_ycc422 marker_1\n");

  if ((out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK))
  {
    /* go to upper left of where we cut and where we are going to paste */
    inImgPtr += (copy_area->y * win + xscopy);
    inClrPtr += (copy_area->y*win + 2*(xscopy/2));

    outImgPtr += (paste_area->y * wout + xspaste);
    outClrPtr += ((paste_area->y*wout) + 2*(xspaste/2));

    inInc = (win - wcopy);
    outInc = (wout - wpaste);

    /* do one rows at a time */
    for (y = hpaste; y; y--)
    {
      /* do two pixels at a time */
      for (x = wpaste/2; x; x--) 
      {
        /* copy 4 lumas, in the following order
         *
         * Y1 Y2
         *
         * */
        /* 1st column */
        *outImgPtr++ = *inImgPtr++;

        /* 2nd column */
        *outImgPtr++ = *inImgPtr++;

        /* copy Cb and Cr 
         *
         * Cb Cr
         *
         */
        if (swap)
        {
          *outClrPtr++ = *(inClrPtr+1);
          *outClrPtr++ = *(inClrPtr);
          inClrPtr +=2;
        }
        else
        {
          *outClrPtr++ = *inClrPtr++;
          *outClrPtr++ = *inClrPtr++;
        }
      } 
      /* each pointer goes 2 lines down */
      inImgPtr += (inInc);
      outImgPtr += (outInc);

      /* goes to correct position in next line */
      inClrPtr += (inInc);
      outClrPtr += (outInc);
    } 
  }
  else if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
           (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    /* go to upper left of where we cut and where we are going to paste */
    inImgPtr += (copy_area->y * win + xscopy);
    inImgPtr2 += (copy_area->y * win + xscopy);
    inClrPtr += (copy_area->y*win + 2*(xscopy/2));

    outImgPtr += (paste_area->y * wout + xspaste);
    outImgPtr2 += (paste_area->y * wout + xspaste);
    outClrPtr += ((paste_area->y/2)*wout + 2*(xspaste/2));

    inInc = (win - wcopy);
    outInc = (wout - wpaste);

    /* do two rows at a time */
    for (y = 0; y < hpaste/2; y++) 
    {
      /* do two pixels at a time */
      for (x = 0; x < wpaste/2; x++) 
      {
        /* copy 4 lumas, in the following order
         *
         * Y1 Y3
         * Y2 Y4
         *
         * */
        /* 1st column */
        *outImgPtr++ = *inImgPtr++;
        *outImgPtr2++ = *inImgPtr2++;

        /* 2nd column */
        *outImgPtr++ = *inImgPtr++;
        *outImgPtr2++ = *inImgPtr2++;

        /* copy Cb and Cr 
         *
         * Cb Cr
         *
         */
        if (swap)
        {
          *outClrPtr++ = *(inClrPtr+1);
          *outClrPtr++ = *(inClrPtr);
          inClrPtr +=2;
        }
        else
        {
          *outClrPtr++ = *inClrPtr++;
          *outClrPtr++ = *inClrPtr++;
        }
      } 

      /* each pointer goes 2 lines down */
      inImgPtr += (inInc + win);
      inImgPtr2 += (inInc + win);
      outImgPtr += (outInc + wout);
      outImgPtr2 += (outInc + wout);

      /* goes to correct position in next line */
      inClrPtr += (inInc + win);
      outClrPtr += outInc;
    } 
  }
  else
  {
    MSG_LOW("ipl_copy_and_paste_ycc422 marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_copy_and_paste_ycc422 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_copy_and_paste_ycc422 */



#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_mb420

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input image must be 

  YCxCx422 line pack <---> YCxCx42y line pack (where x is r|b and y is 0|2) 

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_mb420
(
  ipl_image_type* in_img_ptr,       /* Pointer to the input image */
  ipl_image_type* out_img_ptr,      /* Pointer to the output image */
  ipl_rect_type* copy_area,         /* Input region to be copy out     */
  ipl_rect_type* paste_area         /* Output region to be pasted onto */
)
{
  uint32 nmbx, nmby,mbx,mby,x,y,cjump,yjump,dx;

  unsigned char *inImgPtr, *inImgPtr2, *inClrPtr;
  unsigned char *outImgPtr, *outImgPtr2, *outClrPtr, *outClrPtr2;
  uint32 wcopy, hcopy, wpaste, hpaste, inInc, outInc;
  uint32 win, wout, xscopy, xspaste;


  if ((in_img_ptr->cFormat != IPL_YCrCb420_MB_PK) && 
      (in_img_ptr->cFormat != IPL_YCbCr420_MB_PK))
    return IPL_FAILURE;

  /* setup our input sizes and such */
  win = in_img_ptr->dx;
  wout = out_img_ptr->dx;

  xscopy = copy_area->x;
  wcopy = copy_area->dx;
  hcopy = copy_area->dy;

  xspaste = paste_area->x;
  wpaste = paste_area->dx;
  hpaste = paste_area->dy;

  /* if user want us to start copying on odd pixel, make it even */
  /* i.e. we only copy CbYCrY chunks of data, not CrY            */
  if (xscopy%2) xscopy--;
  if (xspaste%2) xspaste--;

  /* make sure we copy even width area */
  if (wcopy%2) wcopy--;
  if (wpaste%2) wpaste--;

  /* make sure we dont go over our input bounds */
  while ((wcopy+xscopy)>win) 
  {
    wcopy -=2;
    wpaste -=2;
  }

  /* make sure we dont go over our output bounds */
  while ((wpaste+xspaste) > wout) 
  {
    wcopy -=2;
    wpaste -=2;
  }

  if (wcopy != wpaste || hcopy != hpaste) 
    return IPL_FAILURE;

  /* setup our input pointers */
#if 0
  inImgPtr = in_img_ptr->imgPtr;            /* first y ptr */
  inImgPtr2 = in_img_ptr->imgPtr + win;     /* next  y ptr, one line below */
  inClrPtr = in_img_ptr->clrPtr;            /* cbcr ptr    */

  outImgPtr = out_img_ptr->imgPtr;
  outImgPtr2 = out_img_ptr->imgPtr + wout;
  outClrPtr = out_img_ptr->clrPtr;
  outClrPtr2 = out_img_ptr->clrPtr + wout;
#endif


  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    /* go to upper left of where we cut and where we are going to paste */
    /*
    inImgPtr += (copy_area->y * win + xscopy);
    inImgPtr2 += (copy_area->y * win + xscopy);
    inClrPtr += ((copy_area->y/2)*win + 2*(xscopy/2));

    outImgPtr += (paste_area->y * wout + xspaste);
    outImgPtr2 += (paste_area->y * wout + xspaste);
    outClrPtr += ((paste_area->y*wout) + 2*(xspaste/2));
    outClrPtr2 += ((paste_area->y*wout) + 2*(xspaste/2));

    inInc = (win - wcopy);
    outInc = (wout - wpaste);
    */

    dx = out_img_ptr->dx;

    nmbx = dx/16;
    nmby = in_img_ptr->dy/16;
    yjump = 2*dx-16;
    cjump = dx-16;

    inImgPtr = in_img_ptr->imgPtr;
    inImgPtr2 = in_img_ptr->imgPtr+16;
    inClrPtr = in_img_ptr->clrPtr;

    outImgPtr = out_img_ptr->imgPtr;
    outImgPtr2 = out_img_ptr->imgPtr+dx;
    outClrPtr = out_img_ptr->clrPtr;

    for(mby = 0; mby < nmby; mby++)
    {
      for(mbx = 0; mbx < nmbx; mbx++)
      {
        outImgPtr = out_img_ptr->imgPtr + mbx*16 + mby*(256*nmbx);
        outImgPtr2 = out_img_ptr->imgPtr + mbx*16 + mby*(256*nmbx) + dx;
        outClrPtr = out_img_ptr->clrPtr + mbx*16 + mby*(128*nmbx);

        /* two two rows at a time */
        for(y = 8; y; y--)
        {
          /* copy first row of ys */
          /* copy next row of ys */
          /* copy chroma over */
          for(x = 16; x ; x--)
          {
            *outImgPtr++ = *inImgPtr++;
            *outImgPtr2++ = *inImgPtr2++;
            *outClrPtr++ = *inClrPtr++;
          }

          inImgPtr += 16;
          inImgPtr2 += 16; 
          outImgPtr += yjump;
          outImgPtr2 += yjump;
          outClrPtr += cjump;
        }
      }
    }
  }
  else if ((out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) ||
           (out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK))
  {
    /* go to upper left of where we cut and where we are going to paste */
    /*
    inImgPtr += (copy_area->y * win + xscopy);
    inImgPtr2 += (copy_area->y * win + xscopy);
    inClrPtr += ((copy_area->y/2)*win + 2*(xscopy/2));

    outImgPtr += (paste_area->y * wout + xspaste);
    outImgPtr2 += (paste_area->y * wout + xspaste);
    outClrPtr += ((paste_area->y*wout) + 2*(xspaste/2));
    outClrPtr2 += ((paste_area->y*wout) + 2*(xspaste/2));

    inInc = (win - wcopy);
    outInc = (wout - wpaste);
    */

    dx = out_img_ptr->dx;

    nmbx = dx/16;
    nmby = in_img_ptr->dy/16;
    yjump = 2*dx-16;
    cjump = 2*dx-16;

    inImgPtr = in_img_ptr->imgPtr;
    inImgPtr2 = in_img_ptr->imgPtr+16;
    inClrPtr = in_img_ptr->clrPtr;

    outImgPtr = out_img_ptr->imgPtr;
    outImgPtr2 = out_img_ptr->imgPtr+dx;
    outClrPtr = out_img_ptr->clrPtr;
    outClrPtr2 = out_img_ptr->clrPtr+dx;

    for(mby = 0; mby < nmby; mby++)
    {
      for(mbx = 0; mbx < nmbx; mbx++)
      {
        outImgPtr = out_img_ptr->imgPtr + mbx*16 + mby*(256*nmbx);
        outImgPtr2 = out_img_ptr->imgPtr + mbx*16 + mby*(256*nmbx) + dx;
        outClrPtr = out_img_ptr->clrPtr + mbx*16 + mby*(256*nmbx);
        outClrPtr2 = out_img_ptr->clrPtr + mbx*16 + mby*(256*nmbx) + dx;

        /* two two rows at a time */
        for(y = 8; y; y--)
        {
          /* copy first row of ys */
          /* copy next row of ys */
          /* copy chroma over */
          for(x = 16; x ; x--)
          {
            *outImgPtr++ = *inImgPtr++;
            *outImgPtr2++ = *inImgPtr2++;

            *outClrPtr++ = *inClrPtr;
            *outClrPtr2++ = *inClrPtr++;
          }

          inImgPtr += 16;
          inImgPtr2 += 16; 

          outImgPtr += yjump;
          outImgPtr2 += yjump;
          outClrPtr += cjump;
          outClrPtr2 += cjump;
        }
      }
    }
  }
  else
  {
    return IPL_FAILURE;
  }

  /* see if we need to swap Cr and Cb */
  if (in_img_ptr->cFormat == IPL_YCrCb420_MB_PK && 
      out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)
    ipl_convert_swap_chroma_order(out_img_ptr);

  if (in_img_ptr->cFormat == IPL_YCrCb420_MB_PK && 
      out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK)
    ipl_convert_swap_chroma_order(out_img_ptr);

  return IPL_SUCCESS;
} 

#endif


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_mcu_gray

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input image must be  IPL_MCU_GRAY, output can be 420 and 422 line pack

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_mcu_gray
(
  ipl_image_type* in_img_ptr,       /* Pointer to the input image */
  ipl_image_type* out_img_ptr,      /* Pointer to the output image */
  ipl_rect_type* copy_area,         /* Input region to be copy out     */
  ipl_rect_type* paste_area         /* Output region to be pasted onto */
)
{
  unsigned char *inImgPtr;
  unsigned char *outImgPtr, *outClrPtr;
  uint32 hin, hout, vin;
  uint32 cjump,yjump;
  uint32 i, j;
  uint32 pgh, pgv;
  uint32 nmpgh, nmpgv;
  uint32 px, py;

  uint32 num, inx, iny;
  uint32 cdx, cdy;

  MSG_LOW("ipl_copy_and_paste_mcu_gray marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_mcu_gray marker_200\n");
    return IPL_FAILURE;
  }

  cdx = copy_area->dx; 
  cdy = copy_area->dy; 

  /* right now, we only support paste anywehere, not copy from anywhere */
  if ((copy_area->x != 0) || (copy_area->y != 0) || 
      (copy_area->dx != paste_area->dx) || (copy_area->dy != paste_area->dy))
  {
    MSG_LOW("ipl_copy_and_paste_mcu_gray marker_201\n");
    return IPL_FAILURE;
  }
  px = paste_area->x; 
  py = paste_area->y; 

  /* setup our input sizes and such */
  hin = in_img_ptr->dx;
  vin = in_img_ptr->dy;
  hout = out_img_ptr->dx;

  // see how many PG, pixel groups we have in horizontal and vertical
  // dimensions. 
  nmpgh = hin / 8; 
  nmpgv = vin / 8; 

  yjump = hout-8;
  cjump = hout-8;
  inImgPtr = in_img_ptr->imgPtr;

  MSG_LOW("ipl_copy_and_paste_mcu_gray marker_1\n");

  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    if (py%2) py--; 
    py = py*hout;

    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        outImgPtr = out_img_ptr->imgPtr + px + py + (pgv*8*hout)+(pgh*8);

        // copy over 8x8 y block 
        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx < cdx) && (iny < cdy))
            {
              *outImgPtr++ = *inImgPtr++;
            }
            else
            {
              inImgPtr++;
              outImgPtr++;
            }
          }

          outImgPtr += yjump;
        }

        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + py/2 + px + (pgv*4*hout)+(pgh*8);

        for(j = 0; j < 8; j += 2)
        {
          for(i = 0; i < 8; i += 2)
          {
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx < cdx) && (iny < cdy))
            {
              *outClrPtr++ = 128;
              *outClrPtr++ = 128;
            }
            else
            {
              outClrPtr += 2;
            }
          }
          outClrPtr += cjump;
        }
      }
    }
  }
  else if ((out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) ||
           (out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    py = py*hout;

    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */
        outImgPtr = out_img_ptr->imgPtr+px+py+(pgv*8*hout)+(pgh*8);

        // copy over 8x8 y block 
        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx < cdx) && (iny < cdy))
            {
              *outImgPtr++ = *inImgPtr++;
            }
            else
            {
              inImgPtr++;
              outImgPtr++;
            }
          }

          outImgPtr += yjump;
        }

        /* take where inImgPtr was, and now its the start of chroma */
        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + px+py+(pgv*8*hout) + (pgh*8);

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i += 2)
          {
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx < cdx) && (iny < cdy))
            {
              *outClrPtr++ = 128;
              *outClrPtr++ = 128;
            }
            else
            {
              outClrPtr += 2;
            }
          }
          outClrPtr += cjump;
        }
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB565) 
  {
    int32 luma1;
    uint32 rc,gc,bc,r,g,b;
    unsigned short *outImgPtr16;


    py = py*hout;
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */
        outImgPtr16 = (unsigned short *) out_img_ptr->imgPtr + 
          (px+py+(pgv*8*hout)+(pgh*8))*1;

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;

            if ((inx < cdx) && (iny < cdy))
            {
              luma1 =  *inImgPtr++;
              rc = gc = bc = 0x8000;
              r = luma1 + (rc>>16);
              g = luma1 + (gc>>16);
              b = luma1 + (bc>>16);
              r=CLIPIT255(r);
              g=CLIPIT255(g);
              b=CLIPIT255(b);

              *(outImgPtr16++) = pack_rgb565(r,g,b);
            }
            else
            {
              inImgPtr++;
              outImgPtr16++;
            }
          }
          outImgPtr16 += (yjump);
        }
        // skip over the Cb and Cr blocks we just used
        // inImgPtr += 128;
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB888) 
  {
    int32 luma1;
    uint32 rc,gc,bc,r,g,b;

    py = py*hout;
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */
        outImgPtr = (unsigned char *) out_img_ptr->imgPtr; 
        outImgPtr += 3*(px+py+(pgv*8*hout)+(pgh*8));

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;

            if ((inx < cdx) && (iny < cdy))
            {
              luma1 =  *inImgPtr++;
              rc = gc = bc = 0x8000;
              r = luma1 + (rc>>16);
              g = luma1 + (gc>>16);
              b = luma1 + (bc>>16);
              r=CLIPIT255(r);
              g=CLIPIT255(g);
              b=CLIPIT255(b);

              *(outImgPtr++) = (unsigned char) r;
              *(outImgPtr++) = (unsigned char) g;
              *(outImgPtr++) = (unsigned char) b;
            }
            else
            {
              inImgPtr++;
              outImgPtr += 3;
            }
          }
          outImgPtr += (3*yjump);
        }
        // skip over the Cb and Cr blocks we just used
        // inImgPtr += 128;
      }
    }
  }
  else
  {
    MSG_LOW("ipl_copy_and_paste_mcu_gray marker_202\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_copy_and_paste_mcu_gray marker_100\n");
  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_mcu_h1v1

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input image must be 

  IPL_H1V1MCU_CbCr   <->  YCxCx42y line pack (where x is r|b, y is 0|2)

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_h1v1
(
  ipl_image_type* in_img_ptr,       /* Pointer to the input image */
  ipl_image_type* out_img_ptr,      /* Pointer to the output image */
  ipl_rect_type* copy_area,         /* Input region to be copy out     */
  ipl_rect_type* paste_area         /* Output region to be pasted onto */
)
{
  unsigned char *inImgPtr;
  unsigned char *outImgPtr, *outClrPtr;
  unsigned char *inCbPtr, *inCrPtr;
  uint32 hin, hout, vin;
  uint32 cjump,yjump;
  uint32 i, j;
  uint32 pgh, pgv;
  uint32 nmpgh, nmpgv;
  uint32 val;
  boolean swap = FALSE;
  uint32 px, py;


  uint32 num, inx, iny;
  uint32 cdx, cdy;

  MSG_LOW("ipl_copy_and_paste_h1v1 marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_h1v1 marker_200\n");
    return IPL_FAILURE;
  }

  cdx = copy_area->dx; 
  cdy = copy_area->dy; 

  /* right now, we only support paste anywehere, not copy from anywhere */
  //if ((copy_area->x != 0) || (copy_area->y != 0) || 
  //    (copy_area->dx != paste_area->dx) || (copy_area->dy != paste_area->dy))
  //  return IPL_FAILURE;

  px = paste_area->x; 
  py = paste_area->y; 

  /* setup our input sizes and such */
  hin = in_img_ptr->dx;
  vin = in_img_ptr->dy;
  hout = out_img_ptr->dx;

  // see if we need to swap
  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK))
    swap = TRUE;

  // only copy over even width and heigth
  if (out_img_ptr->cFormat != IPL_RGB565 && 
      out_img_ptr->cFormat != IPL_RGB888)
  {
    if (hout%2) hout--;
  }

  // see how many PG, pixel groups we have in horizontal and vertical
  // dimensions. 
  nmpgh = hin / 8; 
  nmpgv = vin / 8; 

  yjump = hout-8;
  cjump = hout-8;
  inImgPtr = in_img_ptr->imgPtr;

  MSG_LOW("ipl_copy_and_paste_h1v1 marker_1\n");
  
  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    if (py%2) py--; 
    py = py*hout;

    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        // copy over 8x8 y block 
        outImgPtr = out_img_ptr->imgPtr + px + py + (pgv*8*hout)+(pgh*8);
        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx >= cdx) || (iny >= cdy))
            {
              inImgPtr++;
              outImgPtr++;
            }
            else
              *outImgPtr++ = *inImgPtr++;
          }

          outImgPtr += yjump;
        }

        /* take where inImgPtr was, and now its the start of chroma */
        inCbPtr = inImgPtr;
        inCrPtr = inImgPtr + 64;  

        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + py/2 + px + (pgv*4*hout)+(pgh*8);

        for(j = 0; j < 8; j += 2)
        {
          for(i = 0; i < 8; i += 2)
          {
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx >= cdx) || (iny >= cdy))
            {
              outClrPtr += 2;
            }
            else
            {
              if (swap)
              {
                // four Crs, make this guy's cr
                val = (*inCrPtr + *(inCrPtr+8) + *inCrPtr+1 + *(inCrPtr+9))/4;
                *outClrPtr++ = (unsigned char) val;

                // four Cbs, make this guy's cr
                val = (*inCbPtr + *(inCbPtr+8) + *inCbPtr+1 + *(inCbPtr+9))/4;
                *outClrPtr++ = (unsigned char) val;
              }
              else
              {
                // four Cbs, make this guy's cb
                val = (*inCbPtr + *(inCbPtr+8) + *inCbPtr+1 + *(inCbPtr+9))/4;
                *outClrPtr++ = (unsigned char) val;

                // four Crs, make this guy's cr
                val = (*inCrPtr + *(inCrPtr+8) + *inCrPtr+1 + *(inCrPtr+9))/4;
                *outClrPtr++ = (unsigned char) val;
              }
            }

            // skip odd Cb and Cr, and grab from previous even
            inCbPtr += 2;
            inCrPtr += 2;
          }

          // skip odd Cb and Cr rows 
          inCbPtr += 8;
          inCrPtr += 8;

          outClrPtr += cjump;
        }

        /* Move inImgPtr to end of Cr/Cb and to start of next Y block */
        inImgPtr += 128;
      }
    }
  }
  else if ((out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) ||
           (out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    py = py*hout;

    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */
        outImgPtr = out_img_ptr->imgPtr+px+py+(pgv*8*hout)+(pgh*8);


        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx >= cdx) || (iny >= cdy))
            {
              inImgPtr++;
              outImgPtr++;
            }
            else
              *outImgPtr++ = *inImgPtr++;
          }

          outImgPtr += yjump;
        }

        /* take where inImgPtr was, and now its the start of chroma */
        inCbPtr = inImgPtr;
        inCrPtr = inImgPtr + 64;  

        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + px+py+(pgv*8*hout) + (pgh*8);

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i += 2)
          {

            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx >= cdx) || (iny >= cdy))
            {
              outClrPtr += 2;
              inCrPtr += 2;
              inCbPtr += 2;
            }
            else
            {
              if (swap)
              {
                // every two horizontal Crs, make this guy's cr
                val = (*inCrPtr + *(inCrPtr+1))/2;
                *outClrPtr++ = (unsigned char) val;
                inCrPtr += 2;

                // every two horizontal Cbs, make this guy's cr
                val = (*inCbPtr + *(inCbPtr+1))/2;
                *outClrPtr++ = (unsigned char) val;
                inCbPtr += 2;
              }
              else
              {
                // every two horizontal Cbs, make this guy's cr
                val = (*inCbPtr + *(inCbPtr+1))/2;
                *outClrPtr++ = (unsigned char) val;
                inCbPtr += 2;

                // every two horizontal Crs, make this guy's cr
                val = (*inCrPtr + *(inCrPtr+1))/2;
                *outClrPtr++ = (unsigned char) val;
                inCrPtr += 2;
              }
            }
          }
          outClrPtr += cjump;
        }
        /* Move inImgPtr to end of Cr/Cb and to start of next Y block */
        inImgPtr += 128;
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB565) 
  {
    int32 luma1;
    int32 cb,cr;
    unsigned short *outImgPtr16;
    int32 r,g,b;
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
    int32 rc,gc,bc;
    /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
    /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
    int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
#endif

    py = py*hout;
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */
        outImgPtr16 = (unsigned short *) out_img_ptr->imgPtr + 
          (px+py+(pgv*8*hout)+(pgh*8))*1;

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            // for cropping purposes, see which input pixel, in x,y space,
            // not MCU space, we are reading
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx >= cdx) || (iny >= cdy))
            {
              inImgPtr++;
              outImgPtr16++;
            }
            else
            {
              luma1 =  *inImgPtr;
              cb = *(inImgPtr + 64);
              cr = *(inImgPtr + 128);  
              inImgPtr++;
            
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
              /* Convert input to RGB and write output */
              rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                 + 0x8000;
              gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))* 4
                 + 0x8000;
              bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                 + 0x8000;
              r = luma1 + (rc>>16);
              g = luma1 + (gc>>16);
              b = luma1 + (bc>>16);
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);
#else
              r = ipl_clamp(luma1 + ipl_crr[cr]);
              g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
              b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

              *(outImgPtr16++) = pack_rgb565(r,g,b);
            } 
          }
          outImgPtr16 += (yjump);
        }

        // skip over the Cb and Cr blocks we just used
        inImgPtr += 128;
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB888) 
  {
    int32 luma1;
    int32 cb,cr;
    int32 r,g,b;
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
    int32 rc,gc,bc;
    /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
    /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
    int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
#endif

    py = py*hout;
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */
        outImgPtr = (unsigned char *) out_img_ptr->imgPtr + 
          (px+py+(pgv*8*hout)+(pgh*8))*3;

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            // for cropping purposes, see which input pixel, in x,y space,
            // not MCU space, we are reading
            num = ((pgv*8*hin)+(pgh*8)+j*hin+i);
            inx = num%hin;
            iny = num/hin;
            if ((inx >= cdx) || (iny >= cdy))
            {
              inImgPtr++;
              outImgPtr += 3;
            }
            else
            {
              luma1 =  *inImgPtr;
              cb = *(inImgPtr + 64);
              cr = *(inImgPtr + 128);  
              inImgPtr++;
            
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
              /* Convert input to RGB and write output */
              rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                 + 0x8000;
              gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))* 4
                 + 0x8000;
              bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                 + 0x8000;
              r = luma1 + (rc>>16);
              g = luma1 + (gc>>16);
              b = luma1 + (bc>>16);
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);
#else
              r = ipl_clamp(luma1 + ipl_crr[cr]);
              g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
              b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

              *outImgPtr++ = (unsigned char) r;
              *outImgPtr++ = (unsigned char) g;
              *outImgPtr++ = (unsigned char) b;
            } 
          }
          outImgPtr += (3*yjump);
        }

        // skip over the Cb and Cr blocks we just used
        inImgPtr += 128;
      }
    }
  }
  else
  {
    MSG_LOW("ipl_copy_and_paste_h1v1 marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_copy_and_paste_h1v1 marker_100\n");
  return IPL_SUCCESS;
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_h2v2

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input image must be 

  IPL_H2V2MCU_CbCr   <->  YCxCx42y line pack (where x is r|b, y is 0|2)

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_h2v2
(
  ipl_image_type* in_img_ptr,       /* Pointer to the input image */
  ipl_image_type* out_img_ptr,      /* Pointer to the output image */
  ipl_rect_type* copy_area,         /* Input region to be copy out     */
  ipl_rect_type* paste_area         /* Output region to be pasted onto */
)
{
  unsigned char *inImgPtr;
  unsigned char *outImgPtr, *outClrPtr;
  unsigned char *inCbPtr, *inCrPtr;

  uint32 hin, hout, vin;
  uint32 cjump,yjump;
  uint32 Yv, Yh, i, j, k, l;
  uint32 pgh, pgv;
  uint32 nmpgh, nmpgv;
  boolean swap = FALSE;
  uint32 px, py;

  uint32 num, inx, iny;
  uint32 cdx, cdy;

  MSG_LOW("ipl_copy_and_paste_h2v2 marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_h2v2 marker_200\n");
    return IPL_FAILURE;
  }


  cdx = copy_area->dx; 
  cdy = copy_area->dy; 


  /* right now, we only support paste anywehere, not copy from anywhere */
  //if ((copy_area->x != 0) || (copy_area->y != 0) || 
  //    (copy_area->dx != paste_area->dx) || (copy_area->dy != paste_area->dy))
  //  return IPL_FAILURE;

  px = paste_area->x; 
  py = paste_area->y; 


  /* setup our input sizes and such */
  hin = in_img_ptr->dx;
  vin = in_img_ptr->dy;
  hout = out_img_ptr->dx;

  // only copy over even width and heigth
  if (out_img_ptr->cFormat != IPL_RGB565 && 
      out_img_ptr->cFormat != IPL_RGB888)
  {
    if (hout%2) hout--;
  }

  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK))
    swap = TRUE;

  nmpgh = hin / 16;
  nmpgv = vin / 16;

  yjump = hout-8;
  cjump = hout-16;
  inImgPtr = in_img_ptr->imgPtr;

  MSG_LOW("ipl_copy_and_paste_h2v2 marker_1\n");

  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    if (py%2) py--; 
    py = py*hout;

    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */

        /* copy over the number of Y blocks in vertical dimension */
        for(Yv = 0; Yv < 2; Yv++)
        {
          /* copy over the number of Y blocks in horizontal dimension */
          for(Yh = 0; Yh < 2; Yh++)
          {
            /* to the correct place in output */
            outImgPtr = out_img_ptr->imgPtr + (pgv*16*hout) + (pgh*16) 
              + hout*8*Yv + 8*Yh + px + py;

            /* two two rows at a time */
            for(j = 0; j < 8; j++)
            {
              for(i = 0; i  < 8; i++)
              {
                // for cropping purposes, see which input pixel, in x,y space,
                // not MCU space, we are reading
                num = ((pgv*16*hin)+(pgh*16)+(Yv*8*hin)+(Yh*8)+j*hin+i);
                inx = num%hin;
                iny = num/hin;
                if ((inx < cdx) && (iny < cdy))
                {
                  *outImgPtr++ = *inImgPtr++;
                }
                else
                {
                  outImgPtr++;
                  inImgPtr++;
                } 
              }
              outImgPtr += yjump;
            }
          }
        }

        /* take where inImgPtr was, and now its the start of chroma */
        inCbPtr = inImgPtr;
        inCrPtr = inImgPtr + 64;  

        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + (pgv*8*hout) + (pgh*16) + px + py/2;

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            // for cropping purposes, see which input pixel, in x,y space,
            // not MCU space, we are reading
            num = ((pgv*16*hin)+(pgh*16)+2*j*hin+2*i);
            inx = num%hin;
            iny = num/hin;

            if ((inx < cdx) && (iny < cdy))
            {
              if (swap)
              {
                *outClrPtr++ = *inCrPtr++;
                *outClrPtr++ = *inCbPtr++;
              }
              else
              {
                *outClrPtr++ = *inCbPtr++;
                *outClrPtr++ = *inCrPtr++;
              }
            }
            else
            {
              outClrPtr += 2;
              inCbPtr++;
              inCrPtr++;
            }
          }
          outClrPtr += cjump;
        }

        /* Move inImgPtr to end of Cr/Cb and to start of next Y block */
        inImgPtr += 128;
      }
    }
  }
  else if ((out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) ||
           (out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    py = py*hout;

    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */


        /* copy over the number of Y blocks in vertical dimension */
        for(Yv = 0; Yv < 2; Yv++)
        {
          /* copy over the number of Y blocks in horizontal dimension */
          for(Yh = 0; Yh < 2; Yh++)
          {
            /* to the correct place in output */
            outImgPtr = out_img_ptr->imgPtr + (pgv*16*hout) + 
              (pgh*16) + hout*8*Yv + 8*Yh + px + py;

            /* two two rows at a time */
            for(j = 0; j < 8; j++)
            {
              for(i = 0; i < 8; i++)
              {
                // for cropping purposes, see which input pixel, in x,y space,
                // not MCU space, we are reading
                num = ((pgv*16*hin)+(pgh*16)+(Yv*8*hin)+(Yh*8)+j*hin+i);
                inx = num%hin;
                iny = num/hin;
                if ((inx < cdx) && (iny < cdy))
                {
                  *outImgPtr++ = *inImgPtr++;
                }
                else
                {
                  outImgPtr++;
                  inImgPtr++;
                } 
              }
              outImgPtr += yjump;
            }
          }
        }


        /* copy over Cb from one, Cr from other to put in LP format */
        /* take where inImgPtr was, and now its the start of chroma */
        inCbPtr = inImgPtr;
        inCrPtr = inImgPtr + 64;  

        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + (pgv*16*hout) + (pgh*16) +px+py;

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
#if 1
            // for cropping purposes, see which input pixel, in x,y space,
            // not MCU space, we are reading
            num = ((pgv*16*hin)+(pgh*16)+2*j*hin+2*i);
            inx = num%hin;
            iny = num/hin;

            if ((inx < cdx) && (iny < cdy))
            {
              if (swap)
              {
                // each cb is for guy below too
                *outClrPtr = *inCrPtr;
                *(outClrPtr + hout) = *inCrPtr++;
                outClrPtr++;

                // each cb is for guy below too
                *outClrPtr = *inCbPtr;
                *(outClrPtr + hout) = *inCbPtr++;
                outClrPtr++;
              }
              else
              {
                // each cb is for guy below too
                *outClrPtr = *inCbPtr;
                *(outClrPtr + hout) = *inCbPtr++;
                outClrPtr++;

                // each cb is for guy below too
                *outClrPtr = *inCrPtr;
                *(outClrPtr + hout) = *inCrPtr++;
                outClrPtr++;
              }
            }
            else
            {
              inCbPtr++;
              inCrPtr++;
              outClrPtr += 2;
            }
#endif

#if 0
            // expand Cr and Cb in the y dimensions. We are going from 
            // two 8x8 blocks to one 16x16 block.
            if (swap)
            {
              // even row output Cr gets associated w/ even row input Cr value
              *outClrPtr = *inCrPtr;

              // odd row output Cr is average of even row above and below him
              val = (*inCrPtr + *(inCrPtr+8))/2;
              *(outClrPtr + hout) = val;
              outClrPtr++;
              inCrPtr++;

              // even row output Cb gets associated w/ even row input Cb value
              *outClrPtr = *inCbPtr;

              // even row output Cr gets associated w/ even row input Cr value
              val = (*inCbPtr + *(inCbPtr+8))/2;
              *(outClrPtr + hout) = val;
              outClrPtr++;
              inCbPtr++;
            }
            else
            {
              // even row output Cb gets associated w/ even row input Cb value
              *outClrPtr = *inCbPtr;

              // even row output Cr gets associated w/ even row input Cr value
              val = (*inCbPtr + *(inCbPtr+8))/2;
              *(outClrPtr + hout) = val;
              outClrPtr++;
              inCbPtr++;

              // even row output Cb gets associated w/ even row input Cb value
              *outClrPtr = *inCrPtr;

              // even row output Cr gets associated w/ even row input Cr value
              val = (*inCrPtr + *(inCrPtr+8))/2;
              *(outClrPtr + hout) = val;
              outClrPtr++;
              inCrPtr++;
            }
#endif
          }
          outClrPtr += (hout + cjump);
        }

        /* Move inImgPtr to end of Cr/Cb and to start of next Y block */
        inImgPtr += 128;
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB565) 
  {
    unsigned short *outImgPtr16;
    unsigned char *cb_ptr;
    int32 luma1;
    int32 cb,cr;
    int32 r,g,b;

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
    int32 rc,gc,bc;
    /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
    /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
    int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
#endif

    py = py*hout;

    // For every group of 4 (8x8) Y blocks an 1 Cb and 1 Cr 
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {

        // there are 2 (8x8) Y blocks vertically
        for(l = 0; l < 2; l++)
        {
          // there are 2 (8x8) Y blocks horizontally
          for(k = 0; k < 2; k++)
          {
            /* for every 8x8 block, do the following Y and Cb/Cr */
            outImgPtr16 = (unsigned short *) out_img_ptr->imgPtr +
              (px+py+(pgv*16*hout)+(pgh*16)+(l*8*hout)+(k*8));


            //the code below is:
            //cb_ptr = in_img_ptr->imgPtr +
            //  (pgv*6*64*nmpgh)+(pgh*6*64)+(l*4*8)+(k*4) + 4*64;
            cb_ptr = in_img_ptr->imgPtr +
              (pgv*384*nmpgh)+(pgh*384)+(l*32)+(k*4) + 256;

            // do 4 pixels at a time
            for(j = 0; j < 8; j += 2)
            {
              for(i = 0; i  < 8; i += 2)
              {
                // for cropping purposes, see which input pixel, in x,y space,
                // not MCU space, we are reading
                num = ((pgv*16*hin)+(pgh*16)+(l*8*hin)+(k*8)+j*hin+i);
                inx = num%hin;
                iny = num/hin;

                cb = *cb_ptr;
                cr = *(cb_ptr+64);  
                cb_ptr++;


#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                /* Convert input to RGB and write output */
                rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                  + 0x8000;
                gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4
                  + 0x8000;
                bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                  + 0x8000;
#endif

                //printf ("this pixel %d going (%d,%d)\n", num, inx, iny);
                if ((inx < cdx) && (iny < cdy))
                {
                  // output pixel, the first one
                  luma1 =  *inImgPtr;
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                  r = luma1 + (rc>>16);
                  g = luma1 + (gc>>16);
                  b = luma1 + (bc>>16);
                  r=CLIPIT(r);
                  g=CLIPIT(g);
                  b=CLIPIT(b);
#else
                  r = ipl_clamp(luma1 + ipl_crr[cr]);
                  g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                  b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif
                  *(outImgPtr16) = pack_rgb565(r,g,b);
                }

                //printf ("this pixel %d going (%d,%d)\n", num, inx, iny);
                num++;
                inx = num%hin;
                iny = num/hin;
                if ((inx < cdx) && (iny < cdy))
                {
                  // output pixel, the one to the right of our first one
                  luma1 =  *(inImgPtr+1);
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                  r = luma1 + (rc>>16);
                  g = luma1 + (gc>>16);
                  b = luma1 + (bc>>16);
                  r=CLIPIT(r);
                  g=CLIPIT(g);
                  b=CLIPIT(b);
#else
                  r = ipl_clamp(luma1 + ipl_crr[cr]);
                  g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                  b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

                  *(outImgPtr16+1) = pack_rgb565(r,g,b);
                }

                num += hin-1;
                inx = num%hin;
                iny = num/hin;
                //printf ("this pixel %d going (%d,%d)\n", num, inx, iny);
                if ((inx < cdx) && (iny < cdy))
                {
                  // output pixel, the one below our first one
                  luma1 =  *(inImgPtr+8);
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                  r = luma1 + (rc>>16);
                  g = luma1 + (gc>>16);
                  b = luma1 + (bc>>16);
                  r=CLIPIT(r);
                  g=CLIPIT(g);
                  b=CLIPIT(b);
#else
                  r = ipl_clamp(luma1 + ipl_crr[cr]);
                  g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                  b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

                  *(outImgPtr16+hout) = pack_rgb565(r,g,b);
                }

                num++; 
                inx = num%hin;
                iny = num/hin;
                //printf ("this pixel %d going (%d,%d)\n", num, inx, iny);
                if ((inx < cdx) && (iny < cdy))
                {
                  // output pixel, the one below and to the righ of first pixel 
                  luma1 =  *(inImgPtr+9);
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                  r = luma1 + (rc>>16);
                  g = luma1 + (gc>>16);
                  b = luma1 + (bc>>16);
                  r=CLIPIT(r);
                  g=CLIPIT(g);
                  b=CLIPIT(b);
#else
                  r = ipl_clamp(luma1 + ipl_crr[cr]);
                  g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                  b = ipl_clamp(luma1 + ipl_cbb[cb]);

#endif

                  *(outImgPtr16+hout+1) = pack_rgb565(r,g,b); 
                }

                // move inImgPtr over two
                inImgPtr += 2;

                // move outImgPtr16 over two
                outImgPtr16 += 2;
              }

              // skip every other row since we already got those values above
              inImgPtr += 8;

              outImgPtr16 += (yjump + hout);

              cb_ptr += 4;
            }
          }
        }

        // skip over the Cb and Cr blocks we just used
        inImgPtr += 128;
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB888) 
  {
    unsigned char *cb_ptr;
    int32 luma1;
    int32 cb,cr;
    int32 r,g,b;

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
    int32 rc,gc,bc;
    /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
    /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
    int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
#endif

    py = py*hout;

    // For every group of 4 (8x8) Y blocks an 1 Cb and 1 Cr 
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {

        // there are 2 (8x8) Y blocks vertically
        for(l = 0; l < 2; l++)
        {
          // there are 2 (8x8) Y blocks horizontally
          for(k = 0; k < 2; k++)
          {
            /* for every 8x8 block, do the following Y and Cb/Cr */
            outImgPtr = (unsigned char *) out_img_ptr->imgPtr +
              3*(px+py+(pgv*16*hout)+(pgh*16)+(l*8*hout)+(k*8));


            //the code below is:
            //cb_ptr = in_img_ptr->imgPtr +
            //  (pgv*6*64*nmpgh)+(pgh*6*64)+(l*4*8)+(k*4) + 4*64;
            cb_ptr = in_img_ptr->imgPtr +
              (pgv*384*nmpgh)+(pgh*384)+(l*32)+(k*4) + 256;

            // do 4 pixels at a time
            for(j = 0; j < 8; j += 2)
            {
              for(i = 0; i  < 8; i += 2)
              {
                // for cropping purposes, see which input pixel, in x,y space,
                // not MCU space, we are reading
                num = ((pgv*16*hin)+(pgh*16)+(l*8*hin)+(k*8)+j*hin+i);
                inx = num%hin;
                iny = num/hin;

                cb = *cb_ptr;
                cr = *(cb_ptr+64);  
                cb_ptr++;


#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                /* Convert input to RGB and write output */
                rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                  + 0x8000;
                gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4
                  + 0x8000;
                bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                  + 0x8000;
#endif

                //printf ("this pixel %d going (%d,%d)\n", num, inx, iny);
                if ((inx < cdx) && (iny < cdy))
                {
                  // output pixel, the first one
                  luma1 =  *inImgPtr;
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                  r = luma1 + (rc>>16);
                  g = luma1 + (gc>>16);
                  b = luma1 + (bc>>16);
                  r=CLIPIT(r);
                  g=CLIPIT(g);
                  b=CLIPIT(b);
#else
                  r = ipl_clamp(luma1 + ipl_crr[cr]);
                  g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                  b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif


                  *(outImgPtr)   = (unsigned char) r;
                  *(outImgPtr+1) = (unsigned char) g;
                  *(outImgPtr+2) = (unsigned char) b;
                }

                //printf ("this pixel %d going (%d,%d)\n", num, inx, iny);
                num++;
                inx = num%hin;
                iny = num/hin;
                if ((inx < cdx) && (iny < cdy))
                {
                  // output pixel, the one to the right of our first one
                  luma1 =  *(inImgPtr+1);
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                  r = luma1 + (rc>>16);
                  g = luma1 + (gc>>16);
                  b = luma1 + (bc>>16);
                  r=CLIPIT(r);
                  g=CLIPIT(g);
                  b=CLIPIT(b);
#else
                  r = ipl_clamp(luma1 + ipl_crr[cr]);
                  g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                  b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

                  *(outImgPtr+3)   = (unsigned char) r;
                  *(outImgPtr+4) = (unsigned char) g;
                  *(outImgPtr+5) = (unsigned char) b;
                }

                num += hin-1;
                inx = num%hin;
                iny = num/hin;
                //printf ("this pixel %d going (%d,%d)\n", num, inx, iny);
                if ((inx < cdx) && (iny < cdy))
                {
                  // output pixel, the one below our first one
                  luma1 =  *(inImgPtr+8);

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                  r = luma1 + (rc>>16);
                  g = luma1 + (gc>>16);
                  b = luma1 + (bc>>16);
                  r=CLIPIT(r);
                  g=CLIPIT(g);
                  b=CLIPIT(b);
#else
                  r = ipl_clamp(luma1 + ipl_crr[cr]);
                  g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                  b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

                  *(outImgPtr+3*hout)   = (unsigned char) r;
                  *(outImgPtr+3*hout+1) = (unsigned char) g;
                  *(outImgPtr+3*hout+2) = (unsigned char) b;
                }

                num++; 
                inx = num%hin;
                iny = num/hin;
                //printf ("this pixel %d going (%d,%d)\n", num, inx, iny);
                if ((inx < cdx) && (iny < cdy))
                {
                  // output pixel, the one below and to the righ of first pixel 
                  luma1 =  *(inImgPtr+9);

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                  r = luma1 + (rc>>16);
                  g = luma1 + (gc>>16);
                  b = luma1 + (bc>>16);
                  r=CLIPIT(r);
                  g=CLIPIT(g);
                  b=CLIPIT(b);
#else
                  r = ipl_clamp(luma1 + ipl_crr[cr]);
                  g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                  b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

                  *(outImgPtr+3*hout+3)   = (unsigned char) r;
                  *(outImgPtr+3*hout+4) = (unsigned char) g;
                  *(outImgPtr+3*hout+5) = (unsigned char) b;
                }

                // move inImgPtr over two
                inImgPtr += 2;

                // move outImgPtr16 over two
                outImgPtr += 6;
              }

              // skip every other row since we already got those values above
              inImgPtr += 8;

              outImgPtr += 3*(yjump + hout);

              cb_ptr += 4;
            }
          }
        }

        // skip over the Cb and Cr blocks we just used
        inImgPtr += 128;
      }
    }
  }
  else
  {
    MSG_LOW("ipl_copy_and_paste_h2v2 marker_201\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_copy_and_paste_h2v2 marker_100\n");
  return IPL_SUCCESS;
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_h2v1

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input image must be 

  IPL_H2V1MCU_CbCr   <->  YCxCx42y line pack (where x is r|b, y is 0|2)

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_h2v1
(
  ipl_image_type* in_img_ptr,       /* Pointer to the input image */
  ipl_image_type* out_img_ptr,      /* Pointer to the output image */
  ipl_rect_type* copy_area,         /* Input region to be copy out     */
  ipl_rect_type* paste_area         /* Output region to be pasted onto */
)
{
  unsigned char *inImgPtr;
  unsigned char *outImgPtr, *outClrPtr;
  unsigned char *inCbPtr, *inCrPtr;
  uint32 hin, hout, vin;
  uint32 cjump,yjump;
  uint32 Yv, Yh, i, j, k;
  uint32 pgh, pgv;
  uint32 nmpgh, nmpgv;
  boolean swap = FALSE;
  uint32 px, py;
  uint32 val;

  uint32 num, inx, iny;
  uint32 cdx, cdy;

  MSG_LOW("ipl_copy_and_paste_h2v1 marker_0\n");
  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_h2v1 marker_200\n");
    return IPL_FAILURE;
  }


  cdx = copy_area->dx; 
  cdy = copy_area->dy; 

  /* right now, we only support paste anywehere, not copy from anywhere */
  //if ((copy_area->x != 0) || (copy_area->y != 0) || 
  //    (copy_area->dx != paste_area->dx) || (copy_area->dy != paste_area->dy))
  //  return IPL_FAILURE;

  px = paste_area->x; 
  py = paste_area->y; 

  /* setup our input sizes and such */
  hin = in_img_ptr->dx;
  vin = in_img_ptr->dy;
  hout = out_img_ptr->dx;

  if (out_img_ptr->cFormat != IPL_RGB565 && 
      out_img_ptr->cFormat != IPL_RGB888)
  {
    if (hout%2) hout--;
  }

  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK))
    swap = TRUE;

  nmpgh = hin / 16;
  nmpgv = vin / 8;

  yjump = hout-8;
  cjump = hout-16;
  inImgPtr = in_img_ptr->imgPtr;

  MSG_LOW("ipl_copy_and_paste_h2v1 marker_1\n");

  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    if (py%2) py--; 
    py = py*hout;


    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* copy over the number of Y blocks in vertical dimension */
        for(Yv = 0; Yv < 1; Yv++)
        {
          /* copy over the number of Y blocks in horizontal dimension */
          for(Yh = 0; Yh < 2; Yh++)
          {
            /* to the correct place in output */
            outImgPtr = out_img_ptr->imgPtr + (pgv*8*hout) + (pgh*16) 
              + hout*8*Yv + 8*Yh + px + py;

            /* two two rows at a time */
            for(j = 0; j < 8; j++)
            {
              for(i = 0; i < 8; i++)
              {
                num = (pgv*8*hin) + (pgh*16) + hin*8*Yv + 8*Yh +j*hin+i;
                inx = num%hin;
                iny = num/hin;
                if ((inx < cdx) && (iny < cdy))
                {
                  *outImgPtr++ = *inImgPtr++;
                }
                else
                {
                  outImgPtr++;
                  inImgPtr++;
                }
              }
              outImgPtr += yjump;
            }
          }
        }

        /* take where inImgPtr was, and now its the start of chroma */
        inCbPtr = inImgPtr;

        /* take where inImgPtr was, and now its the start of chroma */
        inCbPtr = inImgPtr;
        inCrPtr = inImgPtr + 64;  

        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + (pgv*4*hout) + (pgh*16) + px + py/2;

        for(j = 0; j < 8; j += 2)
        {
          for(i = 0; i < 8; i++)
          {
            num = (pgv*4*hin)+(pgh*16)+2*j*hin+2*i;
            inx = num%hin;
            iny = num/hin;
            if ((inx < cdx) && (iny < cdy))
            {
              if (swap)
              {
                // every two vertical Crs, make this guy's cr
                val = (*inCrPtr + *(inCrPtr+8))/2;
                *outClrPtr++ = (unsigned char) val;
                inCrPtr++;

                // every two vertical Cbs, make this guy's cb
                val = (*inCbPtr + *(inCbPtr+8))/2;
                *outClrPtr++ = (unsigned char) val;
                inCbPtr++;
              }
              else
              {
                // every two vertical Cbs, make this guy's cb
                val = (*inCbPtr + *(inCbPtr+8))/2;
                *outClrPtr++ = (unsigned char) val;
                inCbPtr++;

                // every two vertical Crs, make this guy's cr
                val = (*inCrPtr + *(inCrPtr+8))/2;
                *outClrPtr++ = (unsigned char) val;
                inCrPtr++;
              }
            }
            else
            {
              outClrPtr += 2;
              inCrPtr++;
              inCbPtr++;
            }
          }

          // use every other cr and cb row
          inCrPtr += 8;
          inCbPtr += 8;
          outClrPtr += cjump;
        }

        /* Move inImgPtr to end of Cr/Cb and to start of next Y block */
        inImgPtr += 128;
      }
    }
  }
  else if ((out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) ||
           (out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    py = py*hout;

    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */


        /* copy over the number of Y blocks in vertical dimension */
        for(Yv = 0; Yv < 1; Yv++)
        {
          /* copy over the number of Y blocks in horizontal dimension */
          for(Yh = 0; Yh < 2; Yh++)
          {
            /* to the correct place in output */
            outImgPtr = out_img_ptr->imgPtr + (pgv*8*hout) + (pgh*16) + 
              hout*8*Yv + 8*Yh + px + py;

            /* two two rows at a time */
            for(j = 0; j < 8; j++)
            {
              for(i = 0; i < 8; i++)
              {
                num = (pgv*8*hin) + (pgh*16) + hin*8*Yv + 8*Yh +j*hin+i;
                inx = num%hin;
                iny = num/hin;
                if ((inx < cdx) && (iny < cdy))
                //if (1)
                {
                  *outImgPtr++ = *inImgPtr++;
                }
                else
                {
                  outImgPtr++;
                  inImgPtr++;
                }
              }
              outImgPtr += yjump;
            }
          }
        }


        /* copy over Cb from one, Cr from other to put in LP format */
        /* take where inImgPtr was, and now its the start of chroma */
        inCbPtr = inImgPtr;
        inCrPtr = inImgPtr + 64;  

        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + (pgv*8*hout) + (pgh*16) + px + py;

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i++)
          {
            num = (pgv*8*hin)+(pgh*16)+j*hin+2*i;
            inx = num%hin;
            iny = num/hin;
            if ((inx < cdx) && (iny < cdy))
            //if (1)
            {
              if (swap)
              {
                *outClrPtr++ = *inCrPtr++;
                *outClrPtr++ = *inCbPtr++;
              }
              else
              {
                *outClrPtr++ = *inCbPtr++;
                *outClrPtr++ = *inCrPtr++;
              }
            }
            else
            {
              outClrPtr += 2;
              inCbPtr++;
              inCrPtr++;
            }
          }
          outClrPtr += cjump;
        }

        /* Move inImgPtr to end of Cr/Cb and to start of next Y block */
        inImgPtr += 128;
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB565) 
  {

    unsigned short *outImgPtr16;
    unsigned char *cb_ptr;
    int32 luma1;
    int32 cb,cr;
    int32 r,g,b;

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
    int32 rc,gc,bc;
    /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
    /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
    int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
#endif


    py = py*hout;

    // For every group of 2 (8x8) horizontal Y blocks, we have an 1 Cb and 1 Cr 
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        // there are 2 (8x8) Y blocks horizontally
        for(k = 0; k < 2; k++)
        {
          /* for every 8x8 block, do the following Y and Cb/Cr */
          outImgPtr16 = (unsigned short *) out_img_ptr->imgPtr + 
            (px+py+(pgv*8*hout)+(pgh*16)+(k*8));

          //simplified version of:
          //cb_ptr = in_img_ptr->imgPtr +
          //  (pgv*4*64*nmpgh)+(pgh*4*64)+(l*4*8)+(k*4) + 2*64;
          cb_ptr = in_img_ptr->imgPtr +
            (pgv*256*nmpgh)+(pgh*256)+(k*4) + 128;

          // do 2 pixels at a time per row
          for(j = 0; j < 8; j++)
          {
            for(i = 0; i < 8; i += 2)
            {
              cb = *cb_ptr;
              cr = *(cb_ptr+64);  
              cb_ptr++;

              // for cropping purposes, see which input pixel, in x,y space,
              // not MCU space, we are reading
              num = ((pgv*8*hin)+(pgh*16)+(k*8)+j*hin+i);


#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
              /* Convert input to RGB and write output */
              rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                 + 0x8000;
              gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4
                 + 0x8000;
              bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                 + 0x8000;
#endif

              // output pixel, the first one
              inx = num%hin;
              iny = num/hin;
              if ((inx < cdx) && (iny < cdy))
              {
                luma1 =  *inImgPtr;
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                r = luma1 + (rc>>16);
                g = luma1 + (gc>>16);
                b = luma1 + (bc>>16);
                r=CLIPIT(r);
                g=CLIPIT(g);
                b=CLIPIT(b);
#else
                r = ipl_clamp(luma1 + ipl_crr[cr]);
                g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif
                *(outImgPtr16) = pack_rgb565(r,g,b);
              }

              // output pixel, the one to the right of our first one
              inx = (++num)%hin;
              iny = (num)/hin;
              if ((inx < cdx) && (iny < cdy))
              {
                luma1 =  *(inImgPtr+1);
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                r = luma1 + (rc>>16);
                g = luma1 + (gc>>16);
                b = luma1 + (bc>>16);
                r=CLIPIT(r);
                g=CLIPIT(g);
                b=CLIPIT(b);
#else
                r = ipl_clamp(luma1 + ipl_crr[cr]);
                g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif
                b=CLIPIT(b);
                *(outImgPtr16+1) = pack_rgb565(r,g,b); 
              }

              // move inImgPtr over two
              inImgPtr += 2;

              // move outImgPtr16 over two
              outImgPtr16 += 2;
            }

            outImgPtr16 += (yjump);
            cb_ptr += 4;
          }
        }

        // skip over the Cb and Cr blocks we just used
        inImgPtr += 128;
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB888) 
  {
    unsigned char *cb_ptr;
    int32 luma1;
    int32 cb,cr;
    int32 r,g,b;

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
    int32 rc,gc,bc;
    /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
    /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
    int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
#endif

    py = py*hout;

    // For every group of 2 (8x8) horizontal Y blocks, we have an 1 Cb and 1 Cr 
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        // there are 2 (8x8) Y blocks horizontally
        for(k = 0; k < 2; k++)
        {
          /* for every 8x8 block, do the following Y and Cb/Cr */
          outImgPtr = (unsigned char *) out_img_ptr->imgPtr + 
            3*(px+py+(pgv*8*hout)+(pgh*16)+(k*8));

          //simplified version of:
          //cb_ptr = in_img_ptr->imgPtr +
          //  (pgv*4*64*nmpgh)+(pgh*4*64)+(l*4*8)+(k*4) + 2*64;
          cb_ptr = in_img_ptr->imgPtr +
            (pgv*256*nmpgh)+(pgh*256)+(k*4) + 128;

          // do 2 pixels at a time per row
          for(j = 0; j < 8; j++)
          {
            for(i = 0; i < 8; i += 2)
            {
              cb = *cb_ptr;
              cr = *(cb_ptr+64);  
              cb_ptr++;

              // for cropping purposes, see which input pixel, in x,y space,
              // not MCU space, we are reading
              num = ((pgv*8*hin)+(pgh*16)+(k*8)+j*hin+i);


#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
              /* Convert input to RGB and write output */
              rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                 + 0x8000;
              gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4
                 + 0x8000;
              bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                 + 0x8000;
#endif

              // output pixel, the first one
              inx = num%hin;
              iny = num/hin;
              if ((inx < cdx) && (iny < cdy))
              {
                luma1 =  *inImgPtr;
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                r = luma1 + (rc>>16);
                g = luma1 + (gc>>16);
                b = luma1 + (bc>>16);
                r=CLIPIT(r);
                g=CLIPIT(g);
                b=CLIPIT(b);
#else
                r = ipl_clamp(luma1 + ipl_crr[cr]);
                g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

                *(outImgPtr) = (unsigned char) r;
                *(outImgPtr+1) = (unsigned char) g;
                *(outImgPtr+2) = (unsigned char) b;
              }

              // output pixel, the one to the right of our first one
              inx = (++num)%hin;
              iny = (num)/hin;
              if ((inx < cdx) && (iny < cdy))
              {
                luma1 =  *(inImgPtr+1);
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                r = luma1 + (rc>>16);
                g = luma1 + (gc>>16);
                b = luma1 + (bc>>16);
                r=CLIPIT(r);
                g=CLIPIT(g);
                b=CLIPIT(b);
#else
                r = ipl_clamp(luma1 + ipl_crr[cr]);
                g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

                *(outImgPtr+3) = (unsigned char) r;
                *(outImgPtr+4) = (unsigned char) g;
                *(outImgPtr+5) = (unsigned char) b;
              }

              // move inImgPtr over two
              inImgPtr += 2;

              // move outImgPtr16 over two
              outImgPtr += 6;
            }

            outImgPtr += (3*yjump);
            cb_ptr += 4;
          }
        }

        // skip over the Cb and Cr blocks we just used
        inImgPtr += 128;
      }
    }
  }
  else
  {
    MSG_LOW("ipl_copy_and_paste_h2v1 marker_201\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_copy_and_paste_h2v1 marker_100\n");
  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_h1v2

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input image must be 

  IPL_H1V2MCU_CbCr   <->  YCxCx42y line pack (where x is r|b, y is 0|2)

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_h1v2
(
  ipl_image_type* in_img_ptr,       /* Pointer to the input image */
  ipl_image_type* out_img_ptr,      /* Pointer to the output image */
  ipl_rect_type* copy_area,         /* Input region to be copy out     */
  ipl_rect_type* paste_area         /* Output region to be pasted onto */
)
{
  unsigned char *inImgPtr;
  unsigned char *outImgPtr, *outClrPtr;
  unsigned char *inCbPtr, *inCrPtr;
  uint32 hin, hout, vin;
  uint32 cjump,yjump;
  uint32 Yv, Yh, i, j, k;
  uint32 pgh, pgv;
  uint32 nmpgh, nmpgv;
  boolean swap = FALSE;
  uint32 px, py;
  uint32 val;

  uint32 num, inx, iny;
  uint32 cdx, cdy;

  MSG_LOW("ipl_copy_and_paste_h1v2 marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !copy_area   || !paste_area)
  {
    MSG_LOW("ipl_copy_and_paste_h1v2 marker_200\n");
    return IPL_FAILURE;
  }


  cdx = copy_area->dx; 
  cdy = copy_area->dy; 

  /* right now, we only support paste anywehere, not copy from anywhere */
  //if ((copy_area->x != 0) || (copy_area->y != 0) || 
  //    (copy_area->dx != paste_area->dx) || (copy_area->dy != paste_area->dy))
  //  return IPL_FAILURE;

  px = paste_area->x; 
  py = paste_area->y; 

  /* setup our input sizes and such */
  hin = in_img_ptr->dx;
  vin = in_img_ptr->dy;
  hout = out_img_ptr->dx;

  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK))
    swap = TRUE;

  nmpgh = hin / 8;
  nmpgv = vin / 16;

  yjump = hout-8;
  cjump = hout-8;
  inImgPtr = in_img_ptr->imgPtr;

  MSG_LOW("ipl_copy_and_paste_h1v2 marker_1\n");
  
  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    if (py%2) py--; 
    py = py*hout;

    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */

        /* copy over the number of Y blocks in vertical dimension */
        for(Yv = 0; Yv < 2; Yv++)
        {
          /* copy over the number of Y blocks in horizontal dimension */
          for(Yh = 0; Yh < 1; Yh++)
          {
            /* to the correct place in output */
            outImgPtr = out_img_ptr->imgPtr + (pgv*16*hout) + (pgh*8) 
              + hout*8*Yv + 8*Yh + px + py;

            /* two two rows at a time */
            for(j = 0; j < 8; j++)
            {
              for(i = 0; i < 8; i++)
              {
                num = (pgv*16*hin) + (pgh*8) + hin*8*Yv + 8*Yh +j*hin+i;
                inx = num%hin;
                iny = num/hin;
                if ((inx < cdx) && (iny < cdy))
                {
                  *outImgPtr++ = *inImgPtr++;
                }
                else
                {
                  outImgPtr++;
                  inImgPtr++;
                }
              }
              outImgPtr += yjump;
            }
          }
        }

        /* take where inImgPtr was, and now its the start of chroma */
        inCbPtr = inImgPtr;
        inCrPtr = inImgPtr + 64;  

        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + (pgv*8*hout) + (pgh*8) + px + py/2;

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i+=2)
          {
            num = (pgv*8*hin)+(pgh*8)+j*hin+i;
            inx = num%hin;
            iny = num/hin;
            if ((inx < cdx) && (iny < cdy))
            {
              if (swap)
              {
                // every two horizontal Crs, make this guy's cr
                val = (*inCrPtr + *(inCrPtr+1))/2;
                *outClrPtr++ = (unsigned char) val;
                inCrPtr += 2;

                // every two horizontal Cbs, make this guy's cr
                val = (*inCbPtr + *(inCbPtr+1))/2;
                *outClrPtr++ = (unsigned char) val;
                inCbPtr += 2;
              }
              else
              {
                // every two horizontal Cbs, make this guy's cr
                val = (*inCbPtr + *(inCbPtr+1))/2;
                *outClrPtr++ = (unsigned char) val;
                inCbPtr += 2;

                // every two horizontal Crs, make this guy's cr
                val = (*inCrPtr + *(inCrPtr+1))/2;
                *outClrPtr++ = (unsigned char) val;
                inCrPtr += 2;
              }
            }
            else
            {
              outClrPtr += 2;
              inCbPtr += 2;
              inCrPtr += 2;
            }
          }
          outClrPtr += cjump;
        }

        /* Move inImgPtr to end of Cr/Cb and to start of next Y block */
        inImgPtr += 128;
      }
    }
  }
  else if ((out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) ||
           (out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK))
  {
    /* we only want to paste to even colum and width */
    if (px%2) px--; 
    py = py*hout;

    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        /* for every 8x8 block, do the following Y and Cb/Cr */


        /* copy over the number of Y blocks in vertical dimension */
        for(Yv = 0; Yv < 2; Yv++)
        {
          /* copy over the number of Y blocks in horizontal dimension */
          for(Yh = 0; Yh < 1; Yh++)
          {
            /* to the correct place in output */
            outImgPtr = out_img_ptr->imgPtr + (pgv*16*hout) + (pgh*8) + 
              hout*8*Yv + 8*Yh + px + py;

            /* two two rows at a time */
            for(j = 0; j < 8; j++)
            {
              for(i = 0; i < 8; i++)
              {
                num = (pgv*16*hin) + (pgh*8) + hin*8*Yv + 8*Yh +j*hin+i;
                inx = num%hin;
                iny = num/hin;
                if ((inx < cdx) && (iny < cdy))
                {
                  *outImgPtr++ = *inImgPtr++;
                }
                else
                {
                  outImgPtr++;
                  inImgPtr++;
                }
              }
              outImgPtr += yjump;
            }
          }
        }


        /* copy over Cb from one, Cr from other to put in LP format */
        /* take where inImgPtr was, and now its the start of chroma */
        inCbPtr = inImgPtr;
        inCrPtr = inImgPtr + 64;  

        /* deicde where we are writing our output too */
        outClrPtr = out_img_ptr->clrPtr + (pgv*16*hout) + (pgh*8) + px + py;

        for(j = 0; j < 8; j++)
        {
          for(i = 0; i < 8; i+=2)
          {
            num = (pgv*16*hin)+(pgh*8)+j*hin+i;
            inx = num%hin;
            iny = num/hin;
            if ((inx < cdx) && (iny < cdy))
            {
              if (swap)
              {
                // every two horizontal Crs, make this guy's cr
                // but we also need to pad a row below us too to go from
                // 8 rows of C to 16.
                val = (*inCrPtr + *(inCrPtr+1))/2;
                *outClrPtr = (unsigned char) val;
                *(outClrPtr + hout) = (unsigned char) val;
                outClrPtr++;
                inCrPtr += 2;

                // every two horizontal Cbs, make this guy's cr
                val = (*inCbPtr + *(inCbPtr+1))/2;
                *outClrPtr = (unsigned char) val;
                *(outClrPtr + hout) = (unsigned char) val;
                outClrPtr++;
                inCbPtr += 2;
              }
              else
              {
                // every two horizontal Crs, make this guy's cr
                // but we also need to pad a row below us too to go from
                // 8 rows of C to 16.
                val = (*inCbPtr + *(inCbPtr+1))/2;
                *outClrPtr = (unsigned char) val;
                *(outClrPtr + hout) = (unsigned char) val;
                outClrPtr++;
                inCbPtr += 2;

                // every two horizontal Cbs, make this guy's cr
                val = (*inCrPtr + *(inCrPtr+1))/2;
                *outClrPtr = (unsigned char) val;
                *(outClrPtr + hout) = (unsigned char) val;
                outClrPtr++;
                inCrPtr += 2;
              }
            }
            else
            {
              outClrPtr += 2;
              inCbPtr += 2;
              inCrPtr += 2;
            }
          }
          outClrPtr += (hout + cjump);
        }
        /* Move inImgPtr to end of Cr/Cb and to start of next Y block */
        inImgPtr += 128;
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB565) 
  {
    unsigned short *outImgPtr16;
    unsigned char *cb_ptr;
    int32 luma1;
    int32 cb,cr;
    int32 r,g,b;

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
    int32 rc,gc,bc;
    /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
    /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
    int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
#endif

    py = py*hout;
    cdx = copy_area->dx; 
    cdy = copy_area->dy; 

    // For every group of 2 (8x8) Y vertical blocks an 1 Cb and 1 Cr 
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        // there are 2 (8x8) Y blocks verciall
        for(k = 0; k < 2; k++)
        {
          /* for every 8x8 block, do the following Y and Cb/Cr */
          outImgPtr16 = (unsigned short *) out_img_ptr->imgPtr + 
            (px+py+(pgv*16*hout)+(pgh*8)+(k*8*hout));

          //simplified version of:
          //cb_ptr = in_img_ptr->imgPtr +
          //  (pgv*4*64*nmpgh)+(pgh*4*64)+(k*4*8)+ 2*64;
          cb_ptr = in_img_ptr->imgPtr +
            (pgv*256*nmpgh)+(pgh*256)+(k*32) + 128;

          // do 2 pixels at a time per column 
          for(j = 0; j < 8; j+=2)
          {
            for(i = 0; i < 8; i++)
            {
              // for cropping purposes, see which input pixel, in x,y space,
              // not MCU space, we are reading
              num = ((pgv*16*hin)+(pgh*8)+(k*8*hin)+j*hin+i);

              cb = *cb_ptr;
              cr = *(cb_ptr+64);  
              cb_ptr++;

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
              // output pixel, the first one
              /* Convert input to RGB and write output */
              rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                + 0x8000;
              gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4
                + 0x8000;
              bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                + 0x8000;
#endif

              inx = num%hin;
              iny = num/hin;
              if ((inx < cdx) && (iny < cdy))
              {
                luma1 =  *inImgPtr;
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                r = luma1 + (rc>>16);
                g = luma1 + (gc>>16);
                b = luma1 + (bc>>16);
                r=CLIPIT(r);
                g=CLIPIT(g);
                b=CLIPIT(b);
#else
                r = ipl_clamp(luma1 + ipl_crr[cr]);
                g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif
                *(outImgPtr16) = pack_rgb565(r,g,b);
              }

              num += hin;
              inx = (num)%hin;
              iny = (num)/hin;
              // output pixel, the one to the right of our first one
              if ((inx < cdx) && (iny < cdy))
              {
                luma1 =  *(inImgPtr+8);
#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                r = luma1 + (rc>>16);
                g = luma1 + (gc>>16);
                b = luma1 + (bc>>16);
                r=CLIPIT(r);
                g=CLIPIT(g);
                b=CLIPIT(b);
#else
                r = ipl_clamp(luma1 + ipl_crr[cr]);
                g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif
                *(outImgPtr16+hout) = pack_rgb565(r,g,b); 
              }

              // go to next column
              inImgPtr++;

              // move outImgPtr16 over as well
              outImgPtr16++;
            }

            inImgPtr += 8; // skip an input line (we just did it)
            outImgPtr16 += (yjump + hout); // skip an output line
          }
        }

        // skip over the Cb and Cr blocks we just used
        inImgPtr += 128;
      }
    }
  }
  else if (out_img_ptr->cFormat == IPL_RGB888) 
  {
    unsigned char *cb_ptr;
    int32 luma1;
    int32 cb,cr;
    int32 r,g,b;

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
    int32 rc,gc,bc;
    /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
    /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
    int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
#endif

    py = py*hout;

    // For every group of 2 (8x8) Y vertical blocks an 1 Cb and 1 Cr 
    for(pgv = 0; pgv < nmpgv; pgv++)
    {
      for(pgh = 0; pgh < nmpgh; pgh++)
      {
        // there are 2 (8x8) Y blocks verciall
        for(k = 0; k < 2; k++)
        {
          /* for every 8x8 block, do the following Y and Cb/Cr */
          outImgPtr = (unsigned char *) out_img_ptr->imgPtr + 
            3*(px+py+(pgv*16*hout)+(pgh*8)+(k*8*hout));

          //simplified version of:
          //cb_ptr = in_img_ptr->imgPtr +
          //  (pgv*4*64*nmpgh)+(pgh*4*64)+(k*4*8)+ 2*64;
          cb_ptr = in_img_ptr->imgPtr +
            (pgv*256*nmpgh)+(pgh*256)+(k*32) + 128;

          // do 2 pixels at a time per column 
          for(j = 0; j < 8; j+=2)
          {
            for(i = 0; i < 8; i++)
            {
              // for cropping purposes, see which input pixel, in x,y space,
              // not MCU space, we are reading
              num = ((pgv*16*hin)+(pgh*8)+(k*8*hin)+j*hin+i);

              cb = *cb_ptr;
              cr = *(cb_ptr+64);  
              cb_ptr++;

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
              // output pixel, the first one
              /* Convert input to RGB and write output */
              rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                + 0x8000;
              gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4
                + 0x8000;
              bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                + 0x8000;
#endif

              inx = num%hin;
              iny = num/hin;
              if ((inx < cdx) && (iny < cdy))
              {
                luma1 =  *inImgPtr;

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                r = luma1 + (rc>>16);
                g = luma1 + (gc>>16);
                b = luma1 + (bc>>16);
                r=CLIPIT(r);
                g=CLIPIT(g);
                b=CLIPIT(b);
#else
                r = ipl_clamp(luma1 + ipl_crr[cr]);
                g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

                *(outImgPtr) = (unsigned char) r;
                *(outImgPtr+1) = (unsigned char) g;
                *(outImgPtr+2) = (unsigned char) b;
              }

              num += hin;
              inx = (num)%hin;
              iny = (num)/hin;
              // output pixel, the one to the right of our first one
              if ((inx < cdx) && (iny < cdy))
              {
                luma1 =  *(inImgPtr+8);

#ifdef FEATURE_IPL_HXVX_16_235_TO_RGB_0_255 
                r = luma1 + (rc>>16);
                g = luma1 + (gc>>16);
                b = luma1 + (bc>>16);
                r=CLIPIT(r);
                g=CLIPIT(g);
                b=CLIPIT(b);
#else
                r = ipl_clamp(luma1 + ipl_crr[cr]);
                g = ipl_clamp(luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16));
                b = ipl_clamp(luma1 + ipl_cbb[cb]);
#endif

                *(outImgPtr+3*hout) = (unsigned char) r;
                *(outImgPtr+3*hout+1) = (unsigned char) g;
                *(outImgPtr+3*hout+2) = (unsigned char) b;
              }

              // go to next column
              inImgPtr++;

              // move outImgPtr16 over as well
              outImgPtr += 3;
            }

            inImgPtr += 8; // skip an input line (we just did it)
            outImgPtr += 3*(yjump + hout); // skip an output line
          }
        }

        // skip over the Cb and Cr blocks we just used
        inImgPtr += 128;
      }
    }
  }
  else
  {
    MSG_LOW("ipl_copy_and_paste_h1v2 marker_201\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_copy_and_paste_h1v2 marker_100\n");
  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_mcu

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. 

  Input image must be 

  IPL_H1V1MCU_CbCr   <->  YCxCx42y line pack (where x is r|b, y is 0|2)
  IPL_H2V2MCU_CbCr   <->  YCxCx42y line pack (where x is r|b, y is 0|2)
  IPL_H2V1MCU_CbCr   <->  YCxCx42y line pack (where x is r|b, y is 0|2)
  IPL_H1V2MCU_CbCr   <->  YCxCx42y line pack (where x is r|b, y is 0|2)

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto
  qual good the quality should be

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_copy_and_paste_mcu
(
  ipl_image_type* in_img_ptr,       /* Pointer to the input image */
  ipl_image_type* out_img_ptr,      /* Pointer to the output image */
  ipl_rect_type* copy_area,         /* Input region to be copy out     */
  ipl_rect_type* paste_area         /* Output region to be pasted onto */
)
{
  MSG_LOW("ipl_copy_and_paste_mcu marker_0\n");
  if (!in_img_ptr)
  {
    MSG_LOW("ipl_copy_and_paste_mcu marker_200\n");
    return IPL_FAILURE;
  }
  if (in_img_ptr->cFormat == IPL_H1V1MCU_CbCr)
  {
    MSG_LOW("ipl_copy_and_paste_mcu marker_100\n");
    return(ipl_copy_and_paste_h1v1(in_img_ptr, out_img_ptr, copy_area,
                                         paste_area));
  }
  else if (in_img_ptr->cFormat == IPL_H2V2MCU_CbCr)
  {
    MSG_LOW("ipl_copy_and_paste_mcu marker_101\n");
    return(ipl_copy_and_paste_h2v2(in_img_ptr, out_img_ptr, copy_area,
                                         paste_area));
  }
  else if (in_img_ptr->cFormat == IPL_H1V2MCU_CbCr)
  {
    MSG_LOW("ipl_copy_and_paste_mcu marker_102\n");
    return(ipl_copy_and_paste_h1v2(in_img_ptr, out_img_ptr, copy_area,
                                         paste_area));
  }
  else if (in_img_ptr->cFormat == IPL_H2V1MCU_CbCr)
  {
    MSG_LOW("ipl_copy_and_paste_mcu marker_103\n");
    return(ipl_copy_and_paste_h2v1(in_img_ptr, out_img_ptr, copy_area,
                                         paste_area));
  }
  else if (in_img_ptr->cFormat == IPL_MCU_GRAY)
  {
    MSG_LOW("ipl_copy_and_paste_mcu marker_104\n");
    return(ipl_copy_and_paste_mcu_gray(in_img_ptr, out_img_ptr, copy_area,
                                         paste_area));
  }

  MSG_LOW("ipl_copy_and_paste_mcu marker_201\n");
  return IPL_FAILURE;
}



#ifndef FEATURE_IPL_LIGHT



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_blend

DESCRIPTION
  This function performs copy-and-paste but blends the edges with the
  underlying image. A region is copy from the input 
  image and pasted onto a region in the output image. Input and output 
  image sizes can be different. 

  Input and output images must be:
  
  both YCbCr or, 


DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto
  blend      is the width of the blur area where seams would normally be
  blendWhere which edges to blend,


      1,5
  ---------
  |       |
  |       |
  |3,5    |4,5
  |       |
  |       |
  ---------
      2,5


  5 means blend all sides.


ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_copy_and_paste_blend
(
  ipl_image_type* in_img_ptr,   /* Points to the input image           */
  ipl_image_type* out_img_ptr,  /* Points to the output image          */
  ipl_rect_type* copy_area,     /* Input region to be copy out          */
  ipl_rect_type* paste_area,    /* Output region to be pasted onto     */
  uint32 blend,
  uint32 whereBlend
)
{
  ipl_status_type retval;
  ipl_rect_type temp1;
  ipl_rect_type temp2;

  unsigned char *inImgPtr;
  unsigned char *inClrPtr;
  unsigned char *outImgPtr;
  unsigned char *outClrPtr;
  uint32 x, y, wcopy, wpaste, hpaste, inInc, outInc;
  uint32 win, wout, xscopy, xspaste;
  uint32 a;
  int r;

  MSG_LOW("ipl_copy_and_paste_blend marker_0\n");

  /* Check input parameters */
  if (in_img_ptr == NULL || out_img_ptr == NULL)
  {
    MSG_LOW("ipl_copy_and_paste_blend marker_200\n");
    return IPL_FAILURE;
  }


  if (copy_area == NULL)
  {
    temp1.x = 0;
    temp1.y = 0;
    temp1.dx = in_img_ptr->dx;
    temp1.dy = in_img_ptr->dy;
    copy_area = &temp1;
  }

  if (paste_area == NULL)
  {
    temp2.x = 0;
    temp2.y = 0;
    temp2.dx = in_img_ptr->dx;
    temp2.dy = in_img_ptr->dy;
    paste_area = &temp2;
  }

  if (blend < 1)
    blend = 1;

  // turn on blending for all sides if bad blend area input
  if (whereBlend > 5)
   whereBlend = 5;


  /* Input "copy" region must have appropriate dimensions */
  if (copy_area->dx == 0 || copy_area->dy == 0 ||
      (copy_area->x + copy_area->dx) > in_img_ptr->dx ||
      (copy_area->y + copy_area->dy) > in_img_ptr->dy) 
  {
    MSG_LOW("ipl_copy_and_paste_blend marker_202\n");
    return IPL_FAILURE;
  }

  /* Output "paste" region must have appropriate dimensions */
  if (paste_area->dx == 0 || paste_area->dy == 0 ||
      (paste_area->x + paste_area->dx) > out_img_ptr->dx ||
      (paste_area->y + paste_area->dy) > out_img_ptr->dy) 
  {
    MSG_LOW("ipl_copy_and_paste_blend marker_203\n");
    return IPL_FAILURE;
  }

  /* setup our input sizes and such */
  win = in_img_ptr->dx;
  wout = out_img_ptr->dx;

  xscopy = copy_area->x;
  wcopy = copy_area->dx;

  xspaste = paste_area->x;
  wpaste = paste_area->dx;
  hpaste = paste_area->dy;

  MSG_LOW("ipl_copy_and_paste_blend marker_1\n");

  if (in_img_ptr->cFormat == IPL_YCbCr) 
  {
    /* if user want us to start copying on odd pixel, make it even */
    /* i.e. we only copy CbYCrY chunks of data, not CrY            */
    if (xscopy%2) xscopy--;
    if (xspaste%2) xspaste--;

    /* make sure we copy even width area */
    if (wcopy%2) wcopy--;
    if (wpaste%2) wpaste--;

    /* make sure we dont go over our input bounds */
    while ((wcopy+xscopy)>win) 
    {
      wcopy -=2;
      wpaste -=2;
    }

    /* make sure we dont go over our output bounds */
    while ((wpaste+xspaste) > wout) 
    {
      wcopy -=2;
      wpaste -=2;
    }

    inImgPtr = in_img_ptr->imgPtr;
    outImgPtr = out_img_ptr->imgPtr;

    /* go to upper left of where we cut and where we are going to paste */
    inImgPtr += (copy_area->y * win + xscopy)*2;
    outImgPtr += (paste_area->y * wout + xspaste)*2;

    inInc = (win - wcopy)*2;
    outInc = (wout - wpaste)*2;


    /* do two pixels at a time */
    for (y = 0; y < hpaste; y++)
    {
      for (x = 0; x < wpaste; x += 2)
      {
        // check TOP side (and two top corners)
        if (whereBlend == 0)
        {
          *outImgPtr++ = *inImgPtr++;
          *outImgPtr++ = *inImgPtr++;
          *outImgPtr++ = *inImgPtr++;
          *outImgPtr++ = *inImgPtr++;
        } 
        // dont do blending if copy image over empty image
        else if ((*outImgPtr   == 0) &&
                 (*(outImgPtr+1) == 0) &&
                 (*(outImgPtr+2) == 0))
        {
          *outImgPtr++ = *inImgPtr++;
          *outImgPtr++ = *inImgPtr++;
          *outImgPtr++ = *inImgPtr++;
          *outImgPtr++ = *inImgPtr++;
        }
        else 
        {
          // be default, input is visible
          a = 128;

          if (y <= blend)
          {
            if (whereBlend == 1 || whereBlend == 5)
            {
              // check to see if on either of two corners first
              if (x <= blend)
              {
                r = (blend - x)*(blend-x) + (blend-y)*(blend-y);
  
  
                if (r < (int) (blend*blend))
                {
                  a = ((r<<7) / (blend*blend)); //lint !e414
                  a = 128 - a;
                }
                else
                  a = 0;
              }
              else if (x >= (wpaste - blend))
              {
                r = (wpaste - x - blend)*(wpaste - x - blend) + 
                  (blend-y)*(blend-y);
                if (r < (int) (blend*blend))
                {
                  a = ((r<<7) / (blend*blend)); //lint !e414
                  a = 128 - a;
                }
                else
                  a = 0;
              }
              else
              {
                r = (blend-y)*(blend-y);
                a = ((r<<7) / (blend*blend)); //lint !e414
                a = 128 - a;
              }
            }
            // check LEFT side 
            else if (whereBlend == 3)
            {
              if (x <= blend)
              {
                r = (blend - x)*(blend-x);
                a = ((r<<7) / (blend*blend)); //lint !e414
                a = 128 - a;
              }
            }
            // check RIGHT side 
            else if (whereBlend == 4)
            {
              if (x >= (wpaste - blend))
              {
                r = (wpaste - x - blend)*(wpaste - x - blend);
                a = ((r<<7) / (blend*blend)); //lint !e4209
                a = 128 - a;
              }
            }
          }
          // check BOTTOM side (and two bottom corners)
          else if (y >= (hpaste - blend))
          {
            if (whereBlend == 2 || whereBlend == 5)
            {
              // check to see if on either of two corners first
              if (x <= blend)
              {
                r = (blend-x)*(blend-x) + 
                  (blend+y-hpaste)*(blend+y-hpaste);
  
                if (r < (int) (blend*blend))
                {
                  a = ((r<<7) / (blend*blend)); //lint !e414
                  a = 128 - a;
                }
                else
                  a = 0;
              }
              else if (x >= (wpaste - blend))
              {
                r = (wpaste-x-blend)*(wpaste-x-blend) + 
                  (blend+y-hpaste)*(blend+y-hpaste);
  
                if (r < (int) (blend*blend))
                {
                  a = ((r<<7) / (blend*blend)); //lint !e414
                  a = 128 - a;
                }
                else
                  a = 0;
              }
              else
              {
                r = (blend+y-hpaste)*(blend+y-hpaste);
                a = ((r<<7) / (blend*blend)); //lint !e414
                a = 128 - a;
              }
            }
            // check LEFT side 
            else if (whereBlend == 3)
            {
              if (x <= blend)
              {
                r = (blend - x)*(blend-x);
                a = ((r<<7) / (blend*blend)); //lint !e414
                a = 128 - a;
              }
            }
            // check RIGHT side 
            else if (whereBlend == 4)
            {
              if (x >= (wpaste - blend))
              {
                r = (wpaste - x - blend)*(wpaste - x - blend);
                a = ((r<<7) / (blend*blend)); //lint !e4209
                a = 128 - a;
              }
            }
          }
          // check LEFT side 
          else if (x <= blend && (whereBlend == 3 || whereBlend == 5))
          {
            r = (blend - x)*(blend-x);
            a = ((r<<7) / (blend*blend)); //lint !e414
            a = 128 - a;
          }
          // check RIGHT side 
          else if (x >= (wpaste - blend) &&(whereBlend == 4 || whereBlend == 5))
          {
            r = (wpaste - x - blend)*(wpaste - x - blend);
            a = ((r<<7) / (blend*blend)); //lint !e4209
            a = 128 - a;
          }

          *outImgPtr = (unsigned char) ((*inImgPtr*a + *outImgPtr*(128-a))>>7);
          outImgPtr++;
          inImgPtr++;

          *outImgPtr = (unsigned char) ((*inImgPtr*a + *outImgPtr*(128-a))>>7);
          outImgPtr++;
          inImgPtr++;

          *outImgPtr = (unsigned char) ((*inImgPtr*a + *outImgPtr*(128-a))>>7);
          outImgPtr++;
          inImgPtr++;

          *outImgPtr = (unsigned char) ((*inImgPtr*a + *outImgPtr*(128-a))>>7);
          outImgPtr++;
          inImgPtr++;
        }
      } 

      inImgPtr += inInc;
      outImgPtr += outInc;
    } 

    retval = IPL_SUCCESS;
  }
  else if (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
  {
    /* if user want us to start copying on odd pixel, make it even */
    /* i.e. we only copy CbYCrY chunks of data, not CrY            */
    if (xscopy%2) xscopy--;
    if (xspaste%2) xspaste--;

    /* make sure we copy even width area */
    if (wcopy%2) wcopy--;
    if (wpaste%2) wpaste--;

    /* make sure we dont go over our input bounds */
    while ((wcopy+xscopy)>win) 
    {
      wcopy -=2;
      wpaste -=2;
    }

    /* make sure we dont go over our output bounds */
    while ((wpaste+xspaste) > wout) 
    {
      wcopy -=2;
      wpaste -=2;
    }

    inImgPtr = in_img_ptr->imgPtr;
    inClrPtr = in_img_ptr->clrPtr;

    outImgPtr = out_img_ptr->imgPtr;
    outClrPtr = out_img_ptr->clrPtr;

    /* go to upper left of where we cut and where we are going to paste */
    inImgPtr  += (copy_area->y  * win + xscopy);
    outImgPtr += (paste_area->y * wout + xspaste);

    inClrPtr += (copy_area->y/2   * win  + 2*(xscopy/2));
    outClrPtr += (paste_area->y/2 * wout + 2*(xspaste/2));

    inInc = (win - wcopy);
    outInc = (wout - wpaste);


    /* do two pixels at a time */
    for (y = 0; y < hpaste; y += 2)
    {
      for (x = 0; x < wpaste; x += 2)
      {
        // check TOP side (and two top corners)
        if (whereBlend == 0)
        {
          *outImgPtr = *inImgPtr;
          *(outImgPtr+1) = *(inImgPtr+1);

          *(outImgPtr+wout) = *(inImgPtr+win);
          *(outImgPtr+wout+1) = *(inImgPtr+win+1);

          inImgPtr  += 2;
          outImgPtr += 2;

          *outClrPtr++ = *inClrPtr++;
          *outClrPtr++ = *inClrPtr++;
        } 
        // dont do blending if copy image over empty image
        else if ((*outImgPtr   == 0) &&
                 (*(outImgPtr+1) == 0) &&
                 (*(outImgPtr+2) == 0))
        {
          *outImgPtr = *inImgPtr;
          *(outImgPtr+1) = *(inImgPtr+1);

          *(outImgPtr+wout) = *(inImgPtr+win);
          *(outImgPtr+wout+1) = *(inImgPtr+win+1);

          inImgPtr  += 2;
          outImgPtr += 2;

          *outClrPtr++ = *inClrPtr++;
          *outClrPtr++ = *inClrPtr++;
        }
        else 
        {
          // be default, input is visible
          a = 128;

          if (y <= blend)
          {
            if (whereBlend == 1 || whereBlend == 5)
            {
              // check to see if on either of two corners first
              if (x <= blend)
              {
                r = (blend - x)*(blend-x) + (blend-y)*(blend-y);
  
  
                if (r < (int) (blend*blend))
                {
                  a = ((r<<7) / (blend*blend)); //lint !e414
                  a = 128 - a;
                }
                else
                  a = 0;
              }
              else if (x >= (wpaste - blend))
              {
                r = (wpaste - x - blend)*(wpaste - x - blend) + 
                  (blend-y)*(blend-y);
                if (r < (int) (blend*blend))
                {
                  a = ((r<<7) / (blend*blend)); //lint !e414
                  a = 128 - a;
                }
                else
                  a = 0;
              }
              else
              {
                r = (blend-y)*(blend-y);
                a = ((r<<7) / (blend*blend)); //lint !e414
                a = 128 - a;
              }
            }
            // check LEFT side 
            else if (whereBlend == 3)
            {
              if (x <= blend)
              {
                r = (blend - x)*(blend-x);
                a = ((r<<7) / (blend*blend)); //lint !e414
                a = 128 - a;
              }
            }
            // check RIGHT side 
            else if (whereBlend == 4)
            {
              if (x >= (wpaste - blend))
              {
                r = (wpaste - x - blend)*(wpaste - x - blend);
                a = ((r<<7) / (blend*blend)); //lint !e4209
                a = 128 - a;
              }
            }
          }
          // check BOTTOM side (and two bottom corners)
          else if (y >= (hpaste - blend))
          {
            if (whereBlend == 2 || whereBlend == 5)
            {
              // check to see if on either of two corners first
              if (x <= blend)
              {
                r = (blend-x)*(blend-x) + 
                  (blend+y-hpaste)*(blend+y-hpaste);
  
                if (r < (int) (blend*blend))
                {
                  a = ((r<<7) / (blend*blend)); //lint !e414
                  a = 128 - a;
                }
                else
                  a = 0;
              }
              else if (x >= (wpaste - blend))
              {
                r = (wpaste-x-blend)*(wpaste-x-blend) + 
                  (blend+y-hpaste)*(blend+y-hpaste);
  
                if (r < (int) (blend*blend))
                {
                  a = ((r<<7) / (blend*blend)); //lint !e414
                  a = 128 - a;
                }
                else
                  a = 0;
              }
              else
              {
                r = (blend+y-hpaste)*(blend+y-hpaste);
                a = ((r<<7) / (blend*blend)); //lint !e414
                a = 128 - a;
              }
            }
            // check LEFT side 
            else if (whereBlend == 3)
            {
              if (x <= blend)
              {
                r = (blend - x)*(blend-x);
                a = ((r<<7) / (blend*blend)); //lint !e414
                a = 128 - a;
              }
            }
            // check RIGHT side 
            else if (whereBlend == 4)
            {
              if (x >= (wpaste - blend))
              {
                r = (wpaste - x - blend)*(wpaste - x - blend);
                a = ((r<<7) / (blend*blend)); //lint !e4209
                a = 128 - a;
              }
            }
          }
          // check LEFT side 
          else if (x <= blend && (whereBlend == 3 || whereBlend == 5))
          {
            r = (blend - x)*(blend-x);
            a = ((r<<7) / (blend*blend)); //lint !e414
            a = 128 - a;
          }
          // check RIGHT side 
          else if (x >= (wpaste - blend) &&(whereBlend == 4 || whereBlend == 5))
          {
            r = (wpaste - x - blend)*(wpaste - x - blend);
            a = ((r<<7) / (blend*blend)); //lint !e4209
            a = 128 - a;
          }

          //a = 128;
           

          *outImgPtr =    (unsigned char)(((*inImgPtr)*a + *(outImgPtr)*(128-a))>>7);
          *(outImgPtr+1)= (unsigned char)((*(inImgPtr+1)*a + *(outImgPtr+1)*(128-a))>>7);

          *(outImgPtr+wout)= (unsigned char)((*(inImgPtr+win)*a + *(outImgPtr+wout)*(128-a))>>7);
          *(outImgPtr+wout+1) = (unsigned char)((*(inImgPtr+win+1)*a + *(outImgPtr+wout+1)*(128-a))>>7);

          inImgPtr  += 2;
          outImgPtr += 2;




          *outClrPtr =    (unsigned char)(((*inClrPtr)*a + *(outClrPtr)*(128-a))>>7);
          *(outClrPtr+1)= (unsigned char)((*(inClrPtr+1)*a + *(outClrPtr+1)*(128-a))>>7);
          inClrPtr  += 2;
          outClrPtr += 2;
        }
      } 

      inImgPtr  += (inInc + win);
      outImgPtr += (outInc + wout);

      inClrPtr  += inInc;
      outClrPtr += outInc;
    } 

    retval = IPL_SUCCESS;
  }
  else 
  {
    retval = IPL_FAILURE;
  }

  if (retval == IPL_FAILURE)
    MSG_LOW("ipl_copy_and_paste_blend marker_204\n");
  else
    MSG_LOW("ipl_copy_and_paste_blend marker_100\n");

  return retval;
} 




#endif // FEATURE_IPL_LIGHT


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste

DESCRIPTION
  This function performs copy-and-paste. A region is copy from the input 
  image and pasted onto a region in the output image. Input and output 
  image sizes can be different. If the input "copy" region is strictly 
  larger than the output "paste" region, the input region will be downsized 
  to fit the output region. If the images are RGB565, an input "copy" region 
  can also be upsized if it is strictly smaller than the output "paste" 
  region.

  Input and output images must be:
  
  both rgb565 or, 
  both YCbCr or, 
  YCxCx42y line pack <---> YCxCx42y line pack (where x is r|b and y is 2|0), or


DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copy out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

  babak
===========================================================================*/
API_EXTERN ipl_status_type ipl_copy_and_paste
(
  ipl_image_type *in_img_ptr,   /* Points to the input image           */
  ipl_image_type *out_img_ptr,  /* Points to the output image          */
  ipl_rect_type  *copy_area,     /* Input region to be copy out          */
  ipl_rect_type  *paste_area     /* Output region to be pasted onto     */
)
{
  ipl_rect_type temp1;
  ipl_rect_type temp2;

  MSG_LOW("ipl_copy_and_paste marker_0\n");

  /* Check input parameters */
  if (in_img_ptr == NULL || out_img_ptr == NULL) 
  {
    MSG_LOW("ipl_copy_and_paste marker_200\n");
    return IPL_FAILURE;
  }

  if (copy_area == NULL)
  {
    temp1.x = 0;
    temp1.y = 0;
    temp1.dx = IPL_MIN(in_img_ptr->dx, out_img_ptr->dx);
    temp1.dy = IPL_MIN(in_img_ptr->dy, out_img_ptr->dy);
    copy_area = &temp1;
  }

  if (paste_area == NULL)
  {
    temp2.x = 0;
    temp2.y = 0;
    temp2.dx = IPL_MIN(in_img_ptr->dx, out_img_ptr->dx);
    temp2.dy = IPL_MIN(in_img_ptr->dy, out_img_ptr->dy);
    paste_area = &temp2;
  }

  /* Input "copy" region must have appropriate dimensions */
  if (copy_area->dx == 0 || copy_area->dy == 0        ||
      (copy_area->x + copy_area->dx) > in_img_ptr->dx ||
      (copy_area->y + copy_area->dy) > in_img_ptr->dy) {

    MSG_LOW("ipl_copy_and_paste marker_201\n");
    return IPL_FAILURE;
  }

  /* Output "paste" region must have appropriate dimensions */
  if (paste_area->dx == 0 || paste_area->dy == 0         ||
      (paste_area->x + paste_area->dx > out_img_ptr->dx) ||
      (paste_area->y + paste_area->dy > out_img_ptr->dy)) 
  {
    MSG_LOW("ipl_copy_and_paste marker_202\n");
    return IPL_FAILURE;
  }

  /* Call the appropriate function */
  if (in_img_ptr->cFormat == IPL_RGB565) 
  {
    MSG_LOW("ipl_copy_and_paste marker_100\n");

    return (ipl_copy_and_paste_rgb565(in_img_ptr,
                                      out_img_ptr,
                                      copy_area,
                                      paste_area));
  }
  else if (in_img_ptr->cFormat == IPL_YCbCr) 
  {
    MSG_LOW("ipl_copy_and_paste marker_101\n");

    return (ipl_copy_and_paste_ycbcr(in_img_ptr,
                                     out_img_ptr,
                                     copy_area,
                                     paste_area));
  } 
  else if ((in_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) ||
           (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK))
  {
    MSG_LOW("ipl_copy_and_paste marker_102\n");

    return (ipl_copy_and_paste_ycc420(in_img_ptr,
                                      out_img_ptr,
                                      copy_area,
                                      paste_area));
  } 
  else if ((in_img_ptr->cFormat == IPL_YCbCr422_LINE_PK) ||
           (in_img_ptr->cFormat == IPL_YCrCb422_LINE_PK))
  {
    MSG_LOW("ipl_copy_and_paste marker_103\n");

    return (ipl_copy_and_paste_ycc422(in_img_ptr,
                                      out_img_ptr,
                                      copy_area,
                                      paste_area));
  } 
  else if ((in_img_ptr->cFormat == IPL_H1V1MCU_CbCr) ||
           (in_img_ptr->cFormat == IPL_H2V2MCU_CbCr) ||
           (in_img_ptr->cFormat == IPL_H1V2MCU_CbCr) ||
           (in_img_ptr->cFormat == IPL_H2V1MCU_CbCr) ||
           (in_img_ptr->cFormat == IPL_MCU_GRAY))
  {
    MSG_LOW("ipl_copy_and_paste marker_104\n");

    return(ipl_copy_and_paste_mcu(in_img_ptr,
                                  out_img_ptr,
                                  copy_area,
                                  paste_area));
  } 
  else if(in_img_ptr->cFormat == IPL_RGB888) 
  {
    MSG_LOW("ipl_copy_and_paste marker_105\n");

    return(ipl_copy_and_paste_rgb888(in_img_ptr,
                                     out_img_ptr,
                                     copy_area,
                                     paste_area));
  }
  else if (in_img_ptr->cFormat == IPL_YCbCr444)
  {
    MSG_LOW("ipl_copy_and_paste marker_106\n");

    if (copy_area->dx != paste_area->dx ||
        copy_area->dy != paste_area->dy)
      return IPL_FAILURE;

    memcpy(out_img_ptr->imgPtr, 
           in_img_ptr->imgPtr, 
           copy_area->dx * paste_area->dy * 3);
  }
  else if ((in_img_ptr->cFormat == IPL_YCbCr444_LINE_PK) ||
           (in_img_ptr->cFormat == IPL_YCrCb444_LINE_PK)) 
  {
    MSG_LOW("ipl_copy_and_paste marker_107\n");

    if (copy_area->dx != paste_area->dx ||
        copy_area->dy != paste_area->dy)
      return IPL_FAILURE;

    memcpy(out_img_ptr->imgPtr, 
           in_img_ptr->imgPtr, 
           copy_area->dx * paste_area->dy * 1);

    memcpy(out_img_ptr->clrPtr, 
           in_img_ptr->clrPtr, 
           copy_area->dx * paste_area->dy * 2);
  }
  else if ((in_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK) ||
           (in_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK)) 
  {
    if (copy_area->dx != paste_area->dx ||
        copy_area->dy != paste_area->dy)
      return IPL_FAILURE;

    MSG_LOW("ipl_copy_and_paste marker_108\n");

    memcpy(out_img_ptr->imgPtr, 
           in_img_ptr->imgPtr, 
           copy_area->dx * copy_area->dy);

    memcpy(out_img_ptr->clrPtr, 
           in_img_ptr->clrPtr, 
           (copy_area->dx * copy_area->dy)>>1);
  }
  else if(in_img_ptr->cFormat == IPL_LUMA_ONLY) 
  {
    MSG_LOW("ipl_copy_and_paste marker_109\n");

    if (copy_area->dx != paste_area->dx ||
        copy_area->dy != paste_area->dy)
      return IPL_FAILURE;

    memcpy(out_img_ptr->imgPtr, 
           in_img_ptr->imgPtr, 
           copy_area->dx * paste_area->dy);
  }
  else if(in_img_ptr->cFormat == IPL_HSV) 
  {
    MSG_LOW("ipl_copy_and_paste marker_110\n");

    if (copy_area->dx != paste_area->dx ||
        copy_area->dy != paste_area->dy)
      return IPL_FAILURE;

    memcpy(out_img_ptr->imgPtr, 
           in_img_ptr->imgPtr, 
           copy_area->dx * paste_area->dy * 3);
  }
#if 0
  else if ((in_img_ptr->cFormat == IPL_YCbCr420_MB_PK) ||
           (in_img_ptr->cFormat == IPL_YCrCb420_MB_PK))
  {
    return(ipl_copy_and_paste_mb420(in_img_ptr,out_img_ptr,copy_area,
                                   paste_area));
  }
#endif
  else 
  {
    MSG_LOW("ipl_copy_and_paste marker_203\n");
    /* No other color formats supported */
    return IPL_FAILURE;
  }

  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_alpha_blend_rgb565

DESCRIPTION
  This function performs alpha blending. The user specifies the alpha 
  blending factor for the first input image. The blending factor for 
  the second input is (100 - alpha)%. Input and output images must have 
  the same dimensions.

  Alpha must be between 0 and 100, inclusive.
  
  Input and output images must be in RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
  alpha         alpha blending factor for the first input image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_alpha_blend_rgb565
(
  ipl_image_type* in1_img_ptr,  /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,  /* Points to the second input image */
  ipl_image_type* out_img_ptr,  /* Points to the output image       */
  uint32 alpha                  /* Alpha blending factor            */
)
{
  uint16 *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 w, h, x, y, out;
  uint8 r1, g1, b1, r2, g2, b2, rout, gout, bout;

  MSG_LOW("ipl_alpha_blend_rgb565 marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_alpha_blend_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize the image pointers and local variables */
  in1ImgPtr = (uint16*)in1_img_ptr->imgPtr;
  in2ImgPtr = (uint16*)in2_img_ptr->imgPtr;
  outImgPtr = (uint16*)out_img_ptr->imgPtr;
  w = out_img_ptr->dx;
  h = out_img_ptr->dy;

  MSG_LOW("ipl_alpha_blend_rgb565 marker_1\n");

  /* Loop through the input images once */
  for(y = 0; y < h; y++) {
    for(x = 0; x < w; x++) {
      /* unpack input rgb values */
      out = *in1ImgPtr++;
      r1 = (uint8)((out & 0xF800)>>8);
      g1 = (uint8)((out & 0x07E0)>>3);
      b1 = (uint8)((out & 0x001F)<<3);
      out = *in2ImgPtr++;
      r2 = (uint8)((out & 0xF800)>>8);
      g2 = (uint8)((out & 0x07E0)>>3);
      b2 = (uint8)((out & 0x001F)<<3);
      /* find output rgb values */
      rout = (uint8)((r1*alpha+r2*(128-alpha))>>7);
      gout = (uint8)((g1*alpha+g2*(128-alpha))>>7);
      bout = (uint8)((b1*alpha+b2*(128-alpha))>>7);
      *outImgPtr++ = pack_rgb565(rout, gout, bout);
    } /* end x loop */
  } /* end y loop */

  MSG_LOW("ipl_alpha_blend_rgb565 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_alpha_blend_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_alpha_blend_ycbcr

DESCRIPTION
  This function performs alpha blending. The user specifies the alpha 
  blending factor for the first input image. The blending factor for 
  the second input is (100 - alpha)%. Input and output images must have 
  the same dimensions.

  Alpha must be between 0 and 100, inclusive.
  
  Input and output images must be in YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
  alpha         alpha blending factor for the first input image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_alpha_blend_ycbcr
(
  ipl_image_type* in1_img_ptr,  /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,  /* Points to the second input image */
  ipl_image_type* out_img_ptr,  /* Points to the output image       */
  uint32 alpha                  /* Alpha blending factor            */
)
{
  unsigned char *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 w, h;
  uint32 y;

  MSG_LOW("ipl_alpha_blend_ycbcr marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_alpha_blend_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = in1_img_ptr->imgPtr;
  in2ImgPtr = in2_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;
  w = out_img_ptr->dx;
  h = out_img_ptr->dy;

  MSG_LOW("ipl_alpha_blend_ycbcr marker_1\n");

  /* Loop through input images once */
   /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
  {
  /* Loop through input images once */
  for(y = ((w * h) << 1) ; y; y--) {
    *outImgPtr++ = (unsigned char)(((*in1ImgPtr++*alpha)>>7) + 
                                   ((*in2ImgPtr++*(128-alpha))>>7));
  } /* end y loop */
  } /* end of not inplace */
  else /* inplace */
  {
    for (y = ((w * h) << 1); y;  y--)
    {
      *outImgPtr = (unsigned char)(((*outImgPtr*alpha)>>7) + 
                                   ((*in2ImgPtr++*(128-alpha))>>7));
      outImgPtr++;
    }
  } /* end of inplace */
  MSG_LOW("ipl_alpha_blend_ycbcr marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_alpha_blend_ycbcr */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_alpha_blend_ycbcr_lp

DESCRIPTION
  This function performs alpha blending. The user specifies the alpha 
  blending factor for the first input image. The blending factor for 
  the second input is (100 - alpha)%. Input and output images must have 
  the same dimensions.

  Alpha must be between 0 and 100, inclusive.
  
  Input and output images must be in YCbCr 4:2:0 line pack.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
  alpha         alpha blending factor for the first input image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_alpha_blend_ycbcr_lp
(
  ipl_image_type* in1_img_ptr,  /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,  /* Points to the second input image */
  ipl_image_type* out_img_ptr,  /* Points to the output image       */
  uint32 alpha                  /* Alpha blending factor            */
)
{
  unsigned char *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  unsigned char *in1ClrPtr, *in2ClrPtr, *outClrPtr;
  uint32 w, h;
  uint32 y;

  MSG_LOW("ipl_alpha_blend_ycbcr_lp marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_alpha_blend_ycbcr_lp marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = in1_img_ptr->imgPtr;
  in2ImgPtr = in2_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;

  in1ClrPtr = in1_img_ptr->clrPtr;
  in2ClrPtr = in2_img_ptr->clrPtr;
  outClrPtr = out_img_ptr->clrPtr;

  w = out_img_ptr->dx;
  h = out_img_ptr->dy;

  MSG_LOW("ipl_alpha_blend_ycbcr_lp marker_1\n");

  if (((in1_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) &&
       (in2_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) &&
       (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)) ||
      ((in1_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) &&
       (in2_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) &&
       (out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)))
  {
    /* if not inplace */
    if (out_img_ptr != in1_img_ptr)
    {
    for(y = ((w * h) >> 1); y ; y--) 
    {
      *outImgPtr++ = (unsigned char) (((*in1ImgPtr++ * alpha)>>7) + 
                      ((*in2ImgPtr++ * (128-alpha))>>7));

      *outImgPtr++ = (unsigned char) (((*in1ImgPtr++ * alpha)>>7) + 
                      ((*in2ImgPtr++ * (128-alpha))>>7));

      *outClrPtr++ = (unsigned char) (((*in1ClrPtr++ * alpha)>>7) + 
                      ((*in2ClrPtr++ * (128-alpha))>>7));
    } 
    } /* end of not inplace */
    else /* inplace */
    {
      for (y = ((w * h) >> 1); y; y--)
      {
        *outImgPtr = (unsigned char) (((*outImgPtr*alpha)>>7) + 
                        ((*in2ImgPtr++ * (128-alpha))>>7));
        outImgPtr++;
        *outImgPtr = (unsigned char) (((*outImgPtr*alpha)>>7)+ 
                        ((*in2ImgPtr++ * (128-alpha))>>7));
        outImgPtr++;
        *outClrPtr = (unsigned char) (((*outClrPtr * alpha)>>7) +
                        ((*in2ClrPtr++ * (128-alpha))>>7));
        outClrPtr++;
      }
    } /* end of inplace */

    MSG_LOW("ipl_alpha_blend_ycbcr_lp marker_100\n");
    return IPL_SUCCESS;
  }
  else
  {
    MSG_LOW("ipl_alpha_blend_ycbcr_lp marker_201\n");
    return IPL_FAILURE;
  }
} /* End ipl_alpha_blend_ycbcr_lp */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_alpha_blend

DESCRIPTION
  This function performs alpha blending. The user specifies the alpha 
  blending factor for the first input image. The blending factor for 
  the second input is (100 - alpha)%. Input and output images must have 
  the same dimensions.

  Alpha must be between 0 and 100, inclusive.
  
  Input and output images must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
  alpha         alpha blending factor for the first input image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_alpha_blend
(
  ipl_image_type* in1_img_ptr,  /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,  /* Points to the second input image */
  ipl_image_type* out_img_ptr,  /* Points to the output image       */
  uint32 alpha                   /* Alpha blending factor            */
)
{

  MSG_LOW("ipl_alpha_blend marker_0\n");

  /* Check input parameters */
  if (in1_img_ptr == NULL || in2_img_ptr == NULL || out_img_ptr == NULL)
  {
    MSG_LOW("ipl_alpha_blend marker_200\n");
    return IPL_FAILURE;
  }

  /* Input and output color formats must be the same */
  if (in1_img_ptr->cFormat != in2_img_ptr->cFormat ||
      in1_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_alpha_blend marker_201\n");
    return IPL_FAILURE;
  }
  /* Input and output image sizes must be equal */
  if (in1_img_ptr->dx != in2_img_ptr->dx || 
      in1_img_ptr->dy != in2_img_ptr->dy ||
      in1_img_ptr->dx != out_img_ptr->dx || 
      in1_img_ptr->dy != out_img_ptr->dy)
  {
    MSG_LOW("ipl_alpha_blend marker_202\n");
    return IPL_FAILURE;
  }
  /* Alpha must be in the range 0 <= alpha <= 100 */
  if (alpha > 100)
  {
    MSG_LOW("ipl_alpha_blend marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_alpha_blend marker_1\n");

  /* Call the appropriate function */
  if (in1_img_ptr->cFormat == IPL_RGB565)
  {
    if (ipl_alpha_blend_rgb565(in1_img_ptr, in2_img_ptr, out_img_ptr, 
                               ((alpha<<7)/100)) != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_alpha_blend marker_204\n");
      return IPL_FAILURE;
    }
  } 
  else if (in1_img_ptr->cFormat == IPL_YCbCr)
  {
    if (ipl_alpha_blend_ycbcr(in1_img_ptr, in2_img_ptr, out_img_ptr, 
          ((alpha<<7)/100)) != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_alpha_blend marker_205\n");
      return IPL_FAILURE;
    }
  } 
  else if ((in1_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) || 
           (in1_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)) 
  {
    if (ipl_alpha_blend_ycbcr_lp(in1_img_ptr, in2_img_ptr, out_img_ptr, 
          ((alpha<<7)/100)) != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_alpha_blend marker_206\n");
      return IPL_FAILURE;
    }
  } 
  else 
  {
    MSG_LOW("ipl_alpha_blend marker_207\n");
    /* Other formats not supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_alpha_blend marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_alpha_blend */






/* <EJECT> */
/*===========================================================================

FUNCTION ipl_alpha_blend_color

DESCRIPTION
  This function blends an input image with an all-black image using an 
  alpha blending factor that is an integer in the range 0 <= alpha <= 100.
  
  Input and output images must have the same size.

  Input and output images must be YCbCr 4:2:2 or YCbCr 4:2:0 line pack.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    pointer to the first input image
  out_img_ptr   pointer to the output image
  alpha         alpha blending factor

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_alpha_blend_color
(
  ipl_image_type* in_img_ptr,   /* Points to the first input image  */
  ipl_image_type* out_img_ptr,  /* Points to the output image       */
  uint32 alpha,                 /* Alpha blending factor            */
  uint32 red,                   /* red value of color to fade to    */
  uint32 green,                 /* green value of color to fade to  */
  uint32 blue                   /* blue value of color to fade to   */
)
{
  unsigned char *inImgPtr, *outImgPtr;
  unsigned char *inClrPtr, *outClrPtr;
  uint32 w, h;
  uint32 y;
  uint16 cr, cb, luma;
  uint16 inCr, inCb, inY;

  MSG_LOW("ipl_alpha_blend_color marker_0\n");
  
  if (!in_img_ptr || !in_img_ptr->imgPtr ||!out_img_ptr ||!out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_alpha_blend_color marker_200\n");
    return IPL_FAILURE;
  }

  // get our chroma
  inCr = ipl2_rgb565ToCrR[red] + 
         ipl2_rgb565ToCrG[green] +
         ipl2_rgb565ToCrB[blue];

  inCb = ipl2_rgb565ToCbR[red] + 
         ipl2_rgb565ToCbG[green] +
         ipl2_rgb565ToCbB[blue];

  inY = ipl2_rgb565ToYR[red] + 
        ipl2_rgb565ToYG[green] +
        ipl2_rgb565ToYB[blue];


  /* Initialize image pointers and local variables */
  inImgPtr = in_img_ptr->imgPtr;
  inClrPtr = in_img_ptr->clrPtr;

  outImgPtr = out_img_ptr->imgPtr;
  outClrPtr = out_img_ptr->clrPtr;

  h = out_img_ptr->dx;
  w = out_img_ptr->dy;

  alpha = (alpha<<7)/100;

  //cr = (uint16) (16384 - (alpha<<7));
  //cb = (uint16) (16384 - (alpha<<7));
  //luma = (uint16) (2048 - (alpha<<4));

  cr =   (uint16) (inCr * (128 - alpha));
  cb =   (uint16) (inCb * (128 - alpha));
  luma = (uint16) (inY  * (128 - alpha));

  MSG_LOW("ipl_alpha_blend_color marker_1\n");

  /* Loop through input images once */
  if (in_img_ptr->cFormat == IPL_YCbCr)
  {
    /* if not inplace */
    if (out_img_ptr != in_img_ptr)
    {
      for(y = (w * h)>>1; y; y--) 
      {
        *outImgPtr++ = (unsigned char)(((*inImgPtr++ * alpha) + cb)   >> 7);
        *outImgPtr++ = (unsigned char)(((*inImgPtr++ * alpha) + luma) >> 7);
        *outImgPtr++ = (unsigned char)(((*inImgPtr++ * alpha) + cr)   >> 7);
        *outImgPtr++ = (unsigned char)(((*inImgPtr++ * alpha) + luma) >> 7);
      } 
    } 
    else /* inplace */
    {
      for(y = (w * h)>>1; y; y--) 
      {
        *outImgPtr = (unsigned char)(((*outImgPtr*alpha)+cb)>>7);
        outImgPtr++;

        *outImgPtr = (unsigned char)(((*outImgPtr*alpha)+luma)>>7);
        outImgPtr++;

        *outImgPtr = (unsigned char)(((*outImgPtr*alpha)+cr)>>7);
        outImgPtr++;

        *outImgPtr = (unsigned char)(((*outImgPtr*alpha)+luma)>>7);
        outImgPtr++;
      }
    } /* end of inplace */
      
  }
  else if ((in_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) ||
           (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK))
  {
    /* if not inplace */
    if (out_img_ptr != in_img_ptr)
    {
      for(y = ((w * h) >> 2); y ; y--) 
      {
        *outImgPtr++ = (unsigned char) (((*inImgPtr++ * alpha)+luma)>>7);
        *outImgPtr++ = (unsigned char) (((*inImgPtr++ * alpha)+luma)>>7);
        *outImgPtr++ = (unsigned char) (((*inImgPtr++ * alpha)+luma)>>7);
        *outImgPtr++ = (unsigned char) (((*inImgPtr++ * alpha)+luma)>>7);

        *outClrPtr++ = (unsigned char) (((*inClrPtr++ * alpha)+cr)>>7);
        *outClrPtr++ = (unsigned char) (((*inClrPtr++ * alpha)+cb)>>7);
      }
    } 
    else /* inplace */
    {
      for (y = ((w * h) >> 2); y; y--)
      {
        *outImgPtr = (unsigned char) (((*outImgPtr*alpha)+luma)>>7);
        outImgPtr++;

        *outImgPtr = (unsigned char) (((*outImgPtr*alpha)+luma)>>7);
        outImgPtr++;

        *outImgPtr = (unsigned char) (((*outImgPtr*alpha)+luma)>>7);
        outImgPtr++;

        *outImgPtr = (unsigned char) (((*outImgPtr*alpha)+luma)>>7);
        outImgPtr++;




        *outClrPtr = (unsigned char) (((*outClrPtr*alpha)+cr)>>7);
        outClrPtr++;

        *outClrPtr = (unsigned char) (((*outClrPtr*alpha)+cb)>>7);
        outClrPtr++;
      }
    } 
  }
  else
  {
    MSG_LOW("ipl_alpha_blend_color marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_alpha_blend_color marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_alpha_blend_color */






/* <EJECT> */
/*===========================================================================

FUNCTION ipl_alpha_blend_black

DESCRIPTION
  This function blends an input image with an all-black image using an 
  alpha blending factor that is an integer in the range 0 <= alpha <= 100.
  
  Input and output images must have the same size.

  Input and output images must be YCbCr 4:2:2 or YCbCr 4:2:0 line pack.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    pointer to the first input image
  out_img_ptr   pointer to the output image
  alpha         alpha blending factor

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_alpha_blend_black
(
  ipl_image_type* in_img_ptr,   /* Points to the first input image  */
  ipl_image_type* out_img_ptr,  /* Points to the output image       */
  uint32 alpha                  /* Alpha blending factor            */
)
{
  unsigned char *inImgPtr, *outImgPtr;
  unsigned char *inClrPtr, *outClrPtr;
  uint32 w, h;
  uint16 chroma, luma;
  uint32 y;

  MSG_LOW("ipl_alpha_blend_black marker_0\n");
  
  if (!in_img_ptr || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_alpha_blend_black marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = in_img_ptr->imgPtr;
  inClrPtr = in_img_ptr->clrPtr;

  outImgPtr = out_img_ptr->imgPtr;
  outClrPtr = out_img_ptr->clrPtr;

  h = out_img_ptr->dx;
  w = out_img_ptr->dy;
  alpha = (alpha<<7)/100;
  chroma = (uint16) (16384 - (alpha<<7));
  luma = (uint16) (2048 - (alpha<<4));

  MSG_LOW("ipl_alpha_blend_black marker_1\n");

  /* Loop through input images once */
  if (in_img_ptr->cFormat == IPL_YCbCr)
  {
    /* if not inplace */
    if (out_img_ptr != in_img_ptr)
    {
    for(y = ((w * h) << 1); y; y-=2) 
    {
      *outImgPtr++ = (unsigned char)(((*inImgPtr++*alpha)+chroma)>>7);
      *outImgPtr++ = (unsigned char)(((*inImgPtr++*alpha)+luma)>>7);
    } 
    } /* end of not inplace */
    else /* inplace */
    {
    for(y = ((w * h) << 1); y; y-=2)
    {
      *outImgPtr = (unsigned char)(((*outImgPtr*alpha)+chroma)>>7);
      outImgPtr++;
      *outImgPtr = (unsigned char)(((*outImgPtr*alpha)+luma)>>7);
      outImgPtr++;
    }
    } /* end of inplace */
      
  }
  else if ((in_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) ||
           (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK))
  {
    /* if not inplace */
    if (out_img_ptr != in_img_ptr)
    {
    for(y = ((w * h) >> 1); y ; y--) 
    {
      *outImgPtr++ = (unsigned char) (((*inImgPtr++ * alpha)+luma)>>7);
      *outImgPtr++ = (unsigned char) (((*inImgPtr++ * alpha)+luma)>>7);
      *outClrPtr++ = (unsigned char) (((*inClrPtr++ * alpha)+chroma)>>7);
    }
    } /* end of not inplace */
    else /* inplace */
    {
    for (y = ((w * h) >> 1); y; y--)
    {
      *outImgPtr = (unsigned char) (((*outImgPtr*alpha)+luma)>>7);
      outImgPtr++;
      *outImgPtr = (unsigned char) (((*outImgPtr*alpha)+luma)>>7);
      outImgPtr++;
      *outClrPtr = (unsigned char) (((*outClrPtr*alpha)+chroma)>>7);
      outClrPtr++;
    }
    } /* end of inplace */
  }
  else
  {
    MSG_LOW("ipl_alpha_blend_black marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_alpha_blend_black marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_alpha_blend_black */









/* <EJECT> */
/*===========================================================================

FUNCTION ipl_alpha_blend_white

DESCRIPTION
  This function blends an input image with an all-white image using an 
  alpha blending factor that is an integer in the range 0 <= alpha <= 100.
  
  Input and output images must have the same size.
  Input and output images must be YCbCr 4:2:2 or YCbCr 4:2:0 line pack.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    pointer to the input image
  out_img_ptr   pointer to the output image
  alpha         alpha blending factor

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_alpha_blend_white
(
  ipl_image_type* in_img_ptr,   /* Points to the input image  */
  ipl_image_type* out_img_ptr,  /* Points to the output image */
  uint32 alpha                  /* Alpha blending factor      */
)
{
  unsigned char *inImgPtr, *outImgPtr;
  unsigned char *inClrPtr, *outClrPtr;
  uint32 w, h;
  uint16 chroma, luma;
  uint32 y;

  MSG_LOW("ipl_alpha_blend_white marker_0\n");

  if (!in_img_ptr || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_alpha_blend_white marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = in_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;

  inClrPtr = in_img_ptr->clrPtr;
  outClrPtr = out_img_ptr->clrPtr;


  h = out_img_ptr->dx;
  w = out_img_ptr->dy;
  alpha = (alpha<<7)/100;
  chroma = (uint16) (16384 - (alpha<<7));
  luma = (uint16) (30080 - (alpha*235));

  MSG_LOW("ipl_alpha_blend_white marker_1\n");

  /* Loop through input images once */
  if (in_img_ptr->cFormat == IPL_YCbCr)
  {
    /* if not inplace */
    if (out_img_ptr != in_img_ptr)
    {
    for(y = ((w * h) << 1); y; y-=2) 
    {
      *outImgPtr++ = (unsigned char)(((*inImgPtr++*alpha)+chroma)>>7);
      *outImgPtr++ = (unsigned char)(((*inImgPtr++*alpha)+luma)>>7);
    }
    } /* end of not inplace */
    else /* inplace */
    {
    for(y = ((w * h) << 1); y; y-=2)
    {
      *outImgPtr = (unsigned char)(((*outImgPtr*alpha)+chroma)>>7);
      outImgPtr++;
      *outImgPtr = (unsigned char)(((*outImgPtr*alpha)+luma)>>7);
      outImgPtr++;
    }
    } /* end of inplace */
      
  }
  else if ((in_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) ||
           (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK))
  {
    /* if not inplace */
    if (out_img_ptr != in_img_ptr)
    {
    for(y = ((w * h) >> 1); y ; y--) 
    {
      *outImgPtr++ = (unsigned char) (((*inImgPtr++ * alpha)+luma)>>7);
      *outImgPtr++ = (unsigned char) (((*inImgPtr++ * alpha)+luma)>>7);
      *outClrPtr++ = (unsigned char) (((*inClrPtr++ * alpha)+chroma)>>7);
    }
    } /* end of not inplace */
    else /* inplace */
    {
    for (y = ((w * h) >> 1); y; y--)
    {
      *outImgPtr = (unsigned char) (((*outImgPtr*alpha)+luma)>>7);
      outImgPtr++;
      *outImgPtr = (unsigned char) (((*outImgPtr*alpha)+luma)>>7);
      outImgPtr++;
      *outClrPtr = (unsigned char) (((*outClrPtr*alpha)+chroma)>>7);
      outClrPtr++;
    }
    } /* end of inplace */
  }
  else
  {
    MSG_LOW("ipl_alpha_blend_white marker_201\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_alpha_blend_white marker_100\n");
  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_center_rgb565

DESCRIPTION
  This function combines two images by taking the center part of the first 
  image and the left and right parts of the second image.  
  Both inputs and output must have the same width and height.  
  The width indicates the width of the center image.
  Examples:
  (1) if width = 0, output = second image only;
  (2) if width = output width, output = first image only;
  (3) if width = 1/2 of output width, output = center half of first image.
  
  Both inputs must be RGB565.
  The output must be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  width indicates width of the center portion

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_center_rgb565
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 width                    /* Width of center image            */
)
{
  uint16 *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, row, col, endCol1, endCol2;
  uint32 outIndex;

  MSG_LOW("ipl_center_rgb565 marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_center_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  /* Input and output color formats must be the same */
  if (in1_img_ptr->cFormat != in2_img_ptr->cFormat || 
      in1_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_center_rgb565 marker_201\n");
    return IPL_FAILURE;
  }
  /* Input and output image sizes must be equal */
  if (in1_img_ptr->dx != in2_img_ptr->dx ||
      in1_img_ptr->dx != out_img_ptr->dx ||
      in1_img_ptr->dy != in2_img_ptr->dy ||
      in1_img_ptr->dy != out_img_ptr->dy) 
  {
    MSG_LOW("ipl_center_rgb565 marker_202\n");
    return IPL_FAILURE;
  }
  /* Width of center region must not exceed output image width */
  if (width > out_img_ptr->dx)
  {
    MSG_LOW("ipl_center_rgb565 marker_203\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = (uint16*)in1_img_ptr->imgPtr;
  in2ImgPtr = (uint16*)in2_img_ptr->imgPtr;
  outImgPtr = (uint16*)out_img_ptr->imgPtr;
  outWidth = (uint16)out_img_ptr->dx;
  outHeight = (uint16)out_img_ptr->dy;

  /* Find endpoints */
  endCol1 = (out_img_ptr->dx - (uint16)width) >> 1;
  endCol2 = endCol1 + (uint16)width;
  
  /* Initialize the index to starting position */
  outIndex = 0;
  MSG_LOW("ipl_center_rgb565 marker_1\n");
  /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
  {
  /* Loop through input images */
  for(row = outHeight; row; row--) {
    for(col = endCol1; col; col--) {
      *outImgPtr++ = *((uint16*)(in2ImgPtr+outIndex++));
    }
    
    for (col = endCol1; col < endCol2; col++) {
      *outImgPtr++ = *((uint16*)(in1ImgPtr+outIndex++));
    }
    for (col = endCol2; col < outWidth; col++) {
      *outImgPtr++ = *((uint16*)(in2ImgPtr+outIndex++));
    } /* end of col loop */
  } /* end of row loop */
  } /* end of not inplace */
  else /* inplace */
  {
    ipl_rect_type copy_area, paste_area; 

    /* left part of second input */
    copy_area.x  = 0;
    copy_area.y  = 0;
    copy_area.dx = endCol1;
    copy_area.dy = outHeight;
 
    paste_area.x  = 0;
    paste_area.y  = 0;
    paste_area.dx = endCol1;
    paste_area.dy = outHeight;

    if (ipl_copy_and_paste(in2_img_ptr, out_img_ptr, &copy_area,
                           &paste_area) != IPL_SUCCESS)
    {
      MSG_LOW("ipl_center_rgb565 marker_204\n");
      return IPL_FAILURE;
    }
    /* right part of second input */
    copy_area.x  = endCol2;
    copy_area.y  = 0;
    copy_area.dx = outWidth-endCol2;
    copy_area.dy = outHeight;
 
    paste_area.x  = endCol2;
    paste_area.y  = 0;
    paste_area.dx = outWidth-endCol2;
    paste_area.dy = outHeight;

    if (ipl_copy_and_paste(in2_img_ptr, out_img_ptr, &copy_area,
                           &paste_area) != IPL_SUCCESS)
    {
      MSG_LOW("ipl_center_rgb565 marker_205\n");
      return IPL_FAILURE;
    }  
  } /* end of inplace */
  MSG_LOW("ipl_center_rgb565 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_center_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_center_ycbcr

DESCRIPTION
  This function combines two images by taking the center part of the first 
  image and the left and right parts of the second image.  
  Both inputs and output must have the same width and height.  
  The width indicates the width of the center image and must be even for 
  YCbCr images.
  Examples:
  (1) if width = 0, output = second image only;
  (2) if width = output width, output = first image only;
  (3) if width = 1/2 of output width, output = center half of first image.
  
  Both inputs must be YCbCr.
  The output must be YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  width indicates width of the center portion

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_center_ycbcr
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 width                    /* Width of center image            */
)
{
  unsigned char *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, row, col, endCol1, endCol2;
  uint32 outIndex;

  MSG_LOW("ipl_center_ycbcr marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_center_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  /* Input and output color formats must be the same */
  if (in1_img_ptr->cFormat != in2_img_ptr->cFormat || 
      in1_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_center_ycbcr marker_201\n");
    return IPL_FAILURE;
  }
  /* Input and output image sizes must be equal */
  if (in1_img_ptr->dx != in2_img_ptr->dx ||
      in1_img_ptr->dx != out_img_ptr->dx ||
      in1_img_ptr->dy != in2_img_ptr->dy ||
      in1_img_ptr->dy != out_img_ptr->dy) 
  {
    MSG_LOW("ipl_center_ycbcr marker_202\n");
    return IPL_FAILURE;
  }
  /* Width of center region must not exceed output image width */
  if (width > out_img_ptr->dx)
  {
    MSG_LOW("ipl_center_ycbcr marker_203\n");
    return IPL_FAILURE;
  }

  // if odd, make it even
  width &= 0xFFFE;

  /* Initialize image pointers and local variables */
  in1ImgPtr = in1_img_ptr->imgPtr;
  in2ImgPtr = in2_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;

  /* Find endpoints */
  endCol1 = (out_img_ptr->dx - width) >> 1;
  endCol2 = endCol1 + width;
  if (endCol1%2) 
  {
    endCol1++;
    endCol2--;
  }
  
  /* Initialize index to starting position */
  outIndex = 0;  /* byte addressed */

  MSG_LOW("ipl_center_ycbcr marker_1\n");

   /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
  {
  /* Loop through input images */
  for(row = outHeight; row; row--) 
  {
    /* left part of 2nd input */
    for(col = endCol1; col; col-=2) 
    {
      *outImgPtr++ = *((uint8*)(in2ImgPtr+outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr+outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr+outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr+outIndex++));
    }

    /* center part of 1st input */
    for(col = endCol1; col < endCol2; col+=2) 
    {
      *outImgPtr++ = *((uint8*)(in1ImgPtr+outIndex++));
      *outImgPtr++ = *((uint8*)(in1ImgPtr+outIndex++));
      *outImgPtr++ = *((uint8*)(in1ImgPtr+outIndex++));
      *outImgPtr++ = *((uint8*)(in1ImgPtr+outIndex++));
    }

    /* right part of 2nd input */
    for(col = endCol2; col < outWidth; col+=2) 
    {
      *outImgPtr++ = *((uint8*)(in2ImgPtr+outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr+outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr+outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr+outIndex++));
    }
  } /* end of row loop */
  } /* if not inplace */
  else /* inplace */
  {
    ipl_rect_type copy_area, paste_area; 

    /* left part of second input */
    copy_area.x  = 0;
    copy_area.y  = 0;
    copy_area.dx = endCol1;
    copy_area.dy = outHeight;
 
    paste_area.x  = 0;
    paste_area.y  = 0;
    paste_area.dx = endCol1;
    paste_area.dy = outHeight;

    if (ipl_copy_and_paste(in2_img_ptr, out_img_ptr, &copy_area,
                           &paste_area) != IPL_SUCCESS)
    {
      MSG_LOW("ipl_center_ycbcr marker_204\n");
      return IPL_FAILURE;
    }
    /* right part of second input */
    copy_area.x  = endCol2;
    copy_area.y  = 0;
    copy_area.dx = outWidth-endCol2;
    copy_area.dy = outHeight;
 
    paste_area.x  = endCol2;
    paste_area.y  = 0;
    paste_area.dx = outWidth-endCol2;
    paste_area.dy = outHeight;

    if (ipl_copy_and_paste(in2_img_ptr, out_img_ptr, &copy_area,
                           &paste_area) != IPL_SUCCESS)
    {
      MSG_LOW("ipl_center_ycbcr marker_205\n");
      return IPL_FAILURE;
    }  
  } /* end of inplace */

  MSG_LOW("ipl_center_ycbcr marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_center_ycbcr */

/*===========================================================================

FUNCTION ipl_center_ycrcb420lp

DESCRIPTION
  This function combines two images by taking the center part of the first 
  image and the left and right parts of the second image.  
  Both inputs and output must have the same width and height.  
  The width indicates the width of the center image and must be even for 
  YCbCr images.
  Examples:
  (1) if width = 0, output = second image only;
  (2) if width = output width, output = first image only;
  (3) if width = 1/2 of output width, output = center half of first image.
  
  Both inputs must be YCrCb 4:2:0 line packed.
  The output must be YCrCb 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  width indicates width of the center portion

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_center_ycrcb420lp
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 width                    /* Width of center image            */
)
{
  unsigned char *in1ImgPtr, *in2ImgPtr, *outImgPtr, *out2ImgPtr,  
                *in1ClrPtr, *in2ClrPtr, *outClrPtr;
  uint32 outWidth, outHeight, row, col, endCol1, endCol2;
  uint32 outIndex1, outIndex2, outIndex3;
 
  MSG_LOW("ipl_center_ycrcb420lp marker_0\n");
  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_center_ycrcb420lp marker_200\n");
    return IPL_FAILURE;
  }

  /* Input and output color formats must be the same */
  if (in1_img_ptr->cFormat != in2_img_ptr->cFormat || 
      in1_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_center_ycrcb420lp marker_201\n");
    return IPL_FAILURE;
  }

  /* Input and output image sizes must be equal */
  if (in1_img_ptr->dx != in2_img_ptr->dx ||
      in1_img_ptr->dx != out_img_ptr->dx ||
      in1_img_ptr->dy != in2_img_ptr->dy ||
      in1_img_ptr->dy != out_img_ptr->dy) 
  {
    MSG_LOW("ipl_center_ycrcb420lp marker_202\n");
    return IPL_FAILURE;
  }

  /* Width of center region must not exceed output image width */
  if (width > out_img_ptr->dx)
  {
    MSG_LOW("ipl_center_ycrcb420lp marker_203\n");
    return IPL_FAILURE;
  }

  // if odd, make it even
  width &= 0xFFFE;

  /* Initialize image pointers and local variables */
  in1ImgPtr  = in1_img_ptr->imgPtr;
  in2ImgPtr  = in2_img_ptr->imgPtr;

  in1ClrPtr  = in1_img_ptr->clrPtr;
  in2ClrPtr  = in2_img_ptr->clrPtr;

  outImgPtr  = out_img_ptr->imgPtr;
  outClrPtr  = out_img_ptr->clrPtr;

  outWidth   = out_img_ptr->dx;
  outHeight  = out_img_ptr->dy;

  out2ImgPtr = outImgPtr + outWidth;

  /* Find endpoints */
  endCol1 = (out_img_ptr->dx - width) >> 1;
  endCol2 = endCol1 + width;

  if (endCol1%2) 
  {
    endCol1++;
    endCol2--;
  }
 
  if (outWidth % 2)
    outWidth --;
  if (outHeight % 2)
    outHeight --;

  outIndex1 = 0;
  outIndex2 = 0;
  outIndex3 = 0;

  MSG_LOW("ipl_center_ycrcb420lp marker_1\n");

  if (out_img_ptr != in1_img_ptr)
  {
  /* Loop through input images */
  for(row = outHeight; row; row-=2) {
    /* left part of 2nd input */
    for(col = endCol1; col; col-=2) {
      *outImgPtr++  = *((uint8*)(in2ImgPtr+ outIndex1++));
      *outImgPtr++  = *((uint8*)(in2ImgPtr+ outIndex1++));
      *out2ImgPtr++ = *((uint8*)(in2ImgPtr+ outWidth+ outIndex2++));
      *out2ImgPtr++ = *((uint8*)(in2ImgPtr+ outWidth+ outIndex2++));
      *outClrPtr++  = *((uint8*)(in2ClrPtr+ outIndex3++));
      *outClrPtr++  = *((uint8*)(in2ClrPtr+ outIndex3++));
    }
    /* center part of 1st input */
    for(col = endCol1; col < endCol2; col+=2) {
      *outImgPtr++  = *((uint8*)(in1ImgPtr+ outIndex1++));
      *outImgPtr++  = *((uint8*)(in1ImgPtr+ outIndex1++));
      *out2ImgPtr++ = *((uint8*)(in1ImgPtr+ outWidth+ outIndex2++));
      *out2ImgPtr++ = *((uint8*)(in1ImgPtr+ outWidth+ outIndex2++));
      *outClrPtr++  = *((uint8*)(in1ClrPtr+ outIndex3++));
      *outClrPtr++  = *((uint8*)(in1ClrPtr+ outIndex3++));
    }
    /* right part of 2nd input */
    for(col = endCol2; col < outWidth; col+=2) {
      *outImgPtr++  = *((uint8*)(in2ImgPtr+ outIndex1++));
      *outImgPtr++  = *((uint8*)(in2ImgPtr+ outIndex1++));
      *out2ImgPtr++ = *((uint8*)(in2ImgPtr+ outWidth+ outIndex2++));
      *out2ImgPtr++ = *((uint8*)(in2ImgPtr+ outWidth+ outIndex2++));
      *outClrPtr++  = *((uint8*)(in2ClrPtr+ outIndex3++));
      *outClrPtr++ = *((uint8*)(in2ClrPtr+ outIndex3++));
    }
    outIndex1  += outWidth;
    outIndex2  += outWidth;
    outImgPtr  += outWidth;
    out2ImgPtr += outWidth;
  } /* end of row loop */
  }
  else
  {
    ipl_rect_type copy_area, paste_area; 

    /* left part of second input */
    copy_area.x  = 0;
    copy_area.y  = 0;
    copy_area.dx = endCol1;
    copy_area.dy = outHeight;
 
    paste_area.x  = 0;
    paste_area.y  = 0;
    paste_area.dx = endCol1;
    paste_area.dy = outHeight;

    if (ipl_copy_and_paste(in2_img_ptr, out_img_ptr, &copy_area,
                           &paste_area) != IPL_SUCCESS)
    {
      MSG_LOW("ipl_center_ycrcb420lp marker_204\n");
      return IPL_FAILURE;
    }
    /* right part of second input */
    copy_area.x  = endCol2;
    copy_area.y  = 0;
    copy_area.dx = outWidth-endCol2;
    copy_area.dy = outHeight;
 
    paste_area.x  = endCol2;
    paste_area.y  = 0;
    paste_area.dx = outWidth-endCol2;
    paste_area.dy = outHeight;

    if (ipl_copy_and_paste(in2_img_ptr, out_img_ptr, &copy_area,
                           &paste_area) != IPL_SUCCESS)
    {
      MSG_LOW("ipl_center_ycrcb420lp marker_205\n");
      return IPL_FAILURE;
    }  
  }
  MSG_LOW("ipl_center_ycrcb420lp marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_center_ycrcb420lp */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_center

DESCRIPTION
  This function combines two images by taking the center part of the first 
  image and the left and right parts of the second image. The width 
  indicates the width of the center region taken from the first image.
  Examples:
    (1) if width = 0, output = second image only;
    (2) if width = output width, output = first image only;
    (3) if width = 1/2 of output width, output contains center half of 
        first image.
  
  Input and output images must have the same dimensions.
  Input and output images must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
  width         width of the center portion

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_center
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 width                    /* Width of center image            */
)
{

  MSG_LOW("ipl_center marker_0\n");
  /* Check input parameters */
  if (in1_img_ptr == NULL || in2_img_ptr == NULL || 
      out_img_ptr == NULL)
  {
    MSG_LOW("ipl_center marker_200\n");
    return IPL_FAILURE;
  }
  /* Input and output color formats must be the same */
  if (in1_img_ptr->cFormat != in2_img_ptr->cFormat || 
      in1_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_center marker_201\n");
    return IPL_FAILURE;
  }
  /* Input and output image sizes must be equal */
  if (in1_img_ptr->dx != in2_img_ptr->dx ||
      in1_img_ptr->dx != out_img_ptr->dx ||
      in1_img_ptr->dy != in2_img_ptr->dy ||
      in1_img_ptr->dy != out_img_ptr->dy) 
  {
    MSG_LOW("ipl_center marker_202\n");
    return IPL_FAILURE;
  }
  /* Width of center region must not exceed output image width */
  if (width > out_img_ptr->dx)
  {
    MSG_LOW("ipl_center marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_center marker_1\n");

  /* Call the appropriate function */
  if (in1_img_ptr->cFormat == IPL_RGB565) {
    if (ipl_center_rgb565( \
          in1_img_ptr, in2_img_ptr, 
          out_img_ptr, width)
      != IPL_SUCCESS) {
      MSG_LOW("ipl_center marker_204\n");
      return IPL_FAILURE;
    }
  } 
  else if (in1_img_ptr->cFormat == IPL_YCbCr) 
  {
    /* Width must be even for YCbCr 4:2:2 images */
    if (width%2) width++;
    if (ipl_center_ycbcr( \
          in1_img_ptr, in2_img_ptr, 
          out_img_ptr, width)
      != IPL_SUCCESS) {
      MSG_LOW("ipl_center marker_205\n");
      return IPL_FAILURE;
    }
  } 
  else if ((in1_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)  ||
           (in1_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)) 
  {
    if (ipl_center_ycrcb420lp(in1_img_ptr, in2_img_ptr, out_img_ptr, width)
      != IPL_SUCCESS)
    {
      MSG_LOW("ipl_center marker_206\n");
      return IPL_FAILURE;
    }
  } 
  else 
  {
    MSG_LOW("ipl_center marker_207\n");
    /* No other formats supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_center marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_center */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose_horizontal_rgb565

DESCRIPTION
  This function combines two images by taking the left part of the first 
  image and the right part of the second image.  Both inputs and output 
  must have the same width and height.  The boundary indicates where
  the second image starts.  
  Examples:
  (1) if boundary = 0, output = second image only;
  (2) if boundary = output width, output = first image only.
  
  Both inputs and output must be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  boundary indicates location of the boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_compose_horizontal_rgb565
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{
  uint16 *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, row, col;
  uint32 outIndex;

  MSG_LOW("ipl_compose_horizontal_rgb565 marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_compose_horizontal_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = (uint16*) in1_img_ptr->imgPtr;
  in2ImgPtr = (uint16*) in2_img_ptr->imgPtr;
  outImgPtr = (uint16*) out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;

  /* Initialize the index to starting position */
  outIndex = 0;
  MSG_LOW("ipl_compose_horizontal_rgb565 marker_1\n");

  /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
  {
  /* Loop through input images */
  for(row = outHeight; row; row--) {
    /* left part of 1st */
    for(col = boundary; col; col--) {
      *outImgPtr++ = *((uint16*)(in1ImgPtr+outIndex++));
    }
    /* right part of 2nd */
    for(col = boundary; col < outWidth; col++) {
      *outImgPtr++ = *((uint16*)(in2ImgPtr+outIndex++));
    } /* end of col loop */
  } /* end of row loop */
  } /* end of not inplace */
  else /* inplace */
  {
    ipl_rect_type copy_area;
    ipl_rect_type paste_area;

    // put bottom portion
    copy_area.x = boundary;
    copy_area.y = 0;
    copy_area.dx = in1_img_ptr->dx-boundary;
    copy_area.dy = in1_img_ptr->dy;

    paste_area.x = boundary;
    paste_area.y = 0;
    paste_area.dx = in1_img_ptr->dx-boundary;
    paste_area.dy = in1_img_ptr->dy;

    // just put part of the 2nd image over 1st and get out of dodge
    if (ipl_copy_and_paste(in2_img_ptr, in1_img_ptr, &copy_area, &paste_area) 
        != IPL_SUCCESS)
    {
      MSG_LOW("ipl_compose_horizontal_rgb565 marker_201\n");
      return IPL_FAILURE;
    }
  } /* end of inplace */
  MSG_LOW("ipl_compose_horizontal_rgb565 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_compose_horizontal_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose_horizontal_ycbcr

DESCRIPTION
  This function combines two images by taking the left part of the first 
  image and the right part of the second image.  Both inputs and output 
  must have the same width and height.  The boundary indicates where
  the second image starts.  
  Examples:
  (1) if boundary = 0, output = second image only;
  (2) if boundary = output width, output = first image only.
  
  Both inputs and output must be YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  boundary indicates location of the boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_compose_horizontal_ycbcr
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{
  unsigned char *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, row, col, endCol;
  uint32 outIndex;

  MSG_LOW("ipl_compose_horizontal_ycbcr marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_compose_horizontal_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = in1_img_ptr->imgPtr;
  in2ImgPtr = in2_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;

  /* Find endpoint */
  endCol = boundary;
  if (endCol%2) 
  {
    endCol++;
  }

  /* Initialize the index to starting position */
  outIndex = 0;  /* byte addressed */

  MSG_LOW("ipl_compose_horizontal_ycbcr marker_1\n");

  /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
  {
  /* Loop through input images */
  for(row = outHeight; row; row--) {
    /* left part of 1st input */
    for(col = 0; col < endCol; col+=2) {
      *outImgPtr++ = *((uint8*)(in1ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in1ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in1ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in1ImgPtr + outIndex++));
    } /* end of col loop */
    
    /* right part of 2nd input */
    for(col = endCol; col < outWidth; col+=2) {
      *outImgPtr++ = *((uint8*)(in2ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr + outIndex++));
    } /* end of col loop */
  } /* end of row loop */
  } /* end of not inplace */
  else /* inplace */
  {
    ipl_rect_type copy_area;
    ipl_rect_type paste_area;

    // put bottom portion
    copy_area.x = endCol;
    copy_area.y = 0;
    copy_area.dx = out_img_ptr->dx-endCol;
    copy_area.dy = out_img_ptr->dy;

    paste_area.x = endCol;
    paste_area.y = 0;
    paste_area.dx = out_img_ptr->dx-endCol;
    paste_area.dy = out_img_ptr->dy;

    // just put part of the 2nd image over 1st and get out of dodge
    if (ipl_copy_and_paste(in2_img_ptr, out_img_ptr, &copy_area, &paste_area) 
        != IPL_SUCCESS)
    {
      MSG_LOW("ipl_compose_horizontal_ycbcr marker_201\n");
      return IPL_FAILURE;
    }
  } /* end of inplace */

  MSG_LOW("ipl_compose_horizontal_ycbcr marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_compose_horizontal_ycbcr */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose_horizontal_ycbcr_lp

DESCRIPTION
  This function combines two images by taking the left part of the first 
  image and the right part of the second image.  Both inputs and output 
  must have the same width and height.  The boundary indicates where
  the second image starts.  
  Examples:
  (1) if boundary = 0, output = second image only;
  (2) if boundary = output width, output = first image only.
  
  Both inputs and output must be YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  boundary indicates location of the boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None


===========================================================================*/
static ipl_status_type ipl_compose_horizontal_ycbcr_lp
(
  ipl_image_type* in1_img_ptr,  /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,  /* Points to the second input image */
  ipl_image_type* out_img_ptr,  /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{
  unsigned char *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  unsigned char *in1ClrPtr, *in2ClrPtr, *outClrPtr;
  uint32 endCol;
  uint32 row, col, w;

  MSG_LOW("ipl_compose_horizontal_ycbcr_lp marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_compose_horizontal_ycbcr_lp marker_200\n");
    return IPL_FAILURE;
  }

  w = in1_img_ptr->dx;

  /* Initialize image pointers and local variables */
  in1ImgPtr = in1_img_ptr->imgPtr;
  in1ClrPtr = in1_img_ptr->clrPtr;

  in2ImgPtr = in2_img_ptr->imgPtr;
  in2ClrPtr = in2_img_ptr->clrPtr;

  outImgPtr = out_img_ptr->imgPtr;
  outClrPtr = out_img_ptr->clrPtr;

  /* Find endpoint */
  endCol = boundary;
  if (endCol%2) 
    endCol++;

  MSG_LOW("ipl_compose_horizontal_ycbcr_lp marker_1\n");

  // if output and input are different and we are not doing this in place...
  if (out_img_ptr != in1_img_ptr)
  {
    /* Loop through input images */
    for(row = 0; row < in1_img_ptr->dy/2; row++) 
    {
      // this keeps tracks of bytes of image 1 we read that we will use to
      // offset 2nd image

      /* left part of 1st input */
      for(col = endCol/2; col; col--) 
      {
        *outImgPtr = *in1ImgPtr;
        *(outImgPtr+1) = *(in1ImgPtr+1);
        *(outImgPtr+w) = *(in1ImgPtr+w);
        *(outImgPtr+w+1) = *(in1ImgPtr+w+1);
        *outClrPtr++ = *in1ClrPtr++;
        *outClrPtr++ = *in1ClrPtr++;

        outImgPtr  += 2;
        in1ImgPtr  += 2;
      } 

      // advance 2nd image pointer 
      in2ImgPtr += endCol;
      in2ClrPtr += endCol;

      for(col = (w-endCol)/2; col; col--) 
      {
        *outImgPtr = *in2ImgPtr;
        *(outImgPtr+1) = *(in2ImgPtr+1);
        *(outImgPtr+w) = *(in2ImgPtr+w);
        *(outImgPtr+w+1) = *(in2ImgPtr+w+1);
        *outClrPtr++ = *in2ClrPtr++;
        *outClrPtr++ = *in2ClrPtr++;

        outImgPtr  += 2;
        in2ImgPtr  += 2;
      } 

      // advance 1st image pointer 
      in1ImgPtr += (w-endCol);
      in1ClrPtr += (w-endCol);

      // since we were doing two rows at once, advance to next line
      outImgPtr += w;
      in1ImgPtr += w;
      in2ImgPtr += w;
    } 
  }
  else
  {
    ipl_rect_type copy_area;
    ipl_rect_type paste_area;

    // put bottom portion
    copy_area.x = endCol;
    copy_area.y = 0;
    copy_area.dx = in1_img_ptr->dx-endCol;
    copy_area.dy = in1_img_ptr->dy;

    paste_area.x = endCol;
    paste_area.y = 0;
    paste_area.dx = in1_img_ptr->dx-endCol;
    paste_area.dy = in1_img_ptr->dy;

    // just put part of the 2nd image over 1st and get out of dodge
    if (ipl_copy_and_paste(in2_img_ptr, in1_img_ptr, &copy_area, &paste_area) 
        != IPL_SUCCESS)
    {
      MSG_LOW("ipl_compose_horizontal_ycbcr_lp marker_201\n");
      return IPL_FAILURE;
    }
  }

  MSG_LOW("ipl_compose_horizontal_ycbcr_lp marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_compose_horizontal_ycbcr */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose_horizontal

DESCRIPTION
  This function combines two images by taking the left part of the first 
  image and the right part of the second image. The boundary indicates where
  the second image starts.  
  Examples:
    (1) if boundary = 0, output = second image only;
    (2) if boundary = output width, output = first image only.

  Input and output images must have the same dimensions.
  Input and output images must have the same color format, which 
  can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
  boundary      location of boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_compose_horizontal
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{

   MSG_LOW("ipl_compose_horizontal marker_0\n");

  /* Check input parameters */
  if (in1_img_ptr == NULL || in2_img_ptr == NULL || 
      out_img_ptr == NULL)
  {
    MSG_LOW("ipl_compose_horizontal marker_200\n");
    return IPL_FAILURE;
  }
  /* Input and output color formats must be the same */
  if (in1_img_ptr->cFormat != in2_img_ptr->cFormat || 
      in1_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_compose_horizontal marker_201\n");
    return IPL_FAILURE;
  }
  /* Input and output image sizes must be the same */
  if (in1_img_ptr->dx != in2_img_ptr->dx ||
      in1_img_ptr->dx != out_img_ptr->dx ||
      in1_img_ptr->dy != in2_img_ptr->dy ||
      in1_img_ptr->dy != out_img_ptr->dy) 
  {
    MSG_LOW("ipl_compose_horizontal marker_202\n");
    return IPL_FAILURE;
  }
  /* Boundary must not exceed output image boundaries */
  if (boundary >= out_img_ptr->dx)
  {
    MSG_LOW("ipl_compose_horizontal marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_compose_horizontal marker_1\n");

  /* Call the appropriate function */
  if (in1_img_ptr->cFormat == IPL_RGB565) 
  {
    if (ipl_compose_horizontal_rgb565( \
          in1_img_ptr, in2_img_ptr, 
          out_img_ptr, boundary) != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_compose_horizontal marker_204\n");
      return IPL_FAILURE;
    }
  } 
  else if (in1_img_ptr->cFormat == IPL_YCbCr) 
  {
    if (ipl_compose_horizontal_ycbcr( \
          in1_img_ptr, in2_img_ptr, 
          out_img_ptr, boundary) != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_compose_horizontal marker_205\n");
      return IPL_FAILURE;
    }
  } 
  else if ((in1_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) || 
           (in1_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)) 
  {
    if (ipl_compose_horizontal_ycbcr_lp( \
          in1_img_ptr, in2_img_ptr, 
          out_img_ptr, boundary) != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_compose_horizontal marker_206\n");
      return IPL_FAILURE;
    }
  } 
  else 
  {
    MSG_LOW("ipl_compose_horizontal marker_207\n");
    /* No other formats supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_compose_horizontal marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_compose_horizontal */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_move_film_rgb565

DESCRIPTION
  This function combines the end part of the first image with the 
  beginning part of the second image to simulate the effect of a 
  moving film strip.  Both inputs and output must have the same width 
  and height.  The boundary indicates where the second image 
  starts.
  Examples:
  (1) if boundary = 0, output = second image only;
  (2) if boundary = output width, output = first image only.
  
  Both inputs and output must be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  boundary indicates location of the boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_move_film_rgb565
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{
  uint16 *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, row, col, rowInc;

  MSG_LOW("ipl_move_film_rgb565 marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_move_film_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = (uint16*) in1_img_ptr->imgPtr;
  in2ImgPtr = (uint16*) in2_img_ptr->imgPtr;
  outImgPtr = (uint16*) out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;

  /* Initialize the index to starting position */
  rowInc = outWidth - boundary;
  in1ImgPtr += rowInc;

  MSG_LOW("ipl_move_film_rgb565 marker_1\n");

  /* Loop through end part of 1st input image */
  for(row = outHeight; row; row--) 
  {
    for(col = 0; col < boundary; col++) 
    {
      *outImgPtr++ = *in1ImgPtr++;
    } 
    for(col = boundary; col < outWidth; col++) 
    {
      *outImgPtr++ = *in2ImgPtr++;
    } 
    in1ImgPtr += rowInc;
    in2ImgPtr += boundary;
  } /* end of row loop */

  MSG_LOW("ipl_move_film_rgb565 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_move_film_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_move_film_ycbcr

DESCRIPTION
  This function combines the end part of the first image with the 
  beginning part of the second image to simulate the effect of a 
  moving film strip.  Both inputs and output must have the same width 
  and height.  The boundary indicates where the second image 
  starts.
  Examples:
  (1) if boundary = 0, output = second image only;
  (2) if boundary = output width, output = first image only.
  
  Both inputs and output must be YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  boundary indicates location of the boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_move_film_ycbcr
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{
  uint8 *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, row, col, rowInc, row2Inc, endCol;

  MSG_LOW("ipl_move_film_ycbcr marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_move_film_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = in1_img_ptr->imgPtr;
  in2ImgPtr = in2_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;

  /* For now, assume both inputs and output have same width and height */
  endCol = boundary;
  if (endCol%2) {
    endCol++;
  }

  /* Initialize the index to starting position */
  rowInc = (outWidth - endCol) << 1;
  in1ImgPtr += rowInc;  /* byte addressed */
  row2Inc = endCol << 1;

  MSG_LOW("ipl_move_film_ycbcr marker_1\n");
 
  /* Loop through end part of 1st input image */
  for(row = outHeight; row; row--) 
  {
    for(col = 0; col < endCol; col+=2) 
    {
      *outImgPtr++ = *in1ImgPtr++;
      *outImgPtr++ = *in1ImgPtr++;
      *outImgPtr++ = *in1ImgPtr++;
      *outImgPtr++ = *in1ImgPtr++;
    } /* end of col loop */
    for(col = endCol; col < outWidth; col+=2) 
    {
      *outImgPtr++ = *in2ImgPtr++;
      *outImgPtr++ = *in2ImgPtr++;
      *outImgPtr++ = *in2ImgPtr++;
      *outImgPtr++ = *in2ImgPtr++;
    } /* end of col loop */
    in1ImgPtr += rowInc;
    in2ImgPtr += row2Inc;
  } /* end of row loop */

  MSG_LOW("ipl_move_film_ycbcr marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_move_film_ycbcr */

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_move_film_ycrcb420lp

DESCRIPTION
  This function combines the end part of the first image with the 
  beginning part of the second image to simulate the effect of a 
  moving film strip.  Both inputs and output must have the same width 
  and height.  The boundary indicates where the second image 
  starts.
  Examples:
  (1) if boundary = 0, output = second image only;
  (2) if boundary = output width, output = first image only.
  
  Both inputs and output must be YCrCb 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  boundary indicates location of the boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_move_film_ycrcb420lp
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{
  uint8 *in1ImgPtr, *in12ImgPtr, *in2ImgPtr, *in22ImgPtr,
        *in1ClrPtr, *in2ClrPtr,  *outClrPtr, *outImgPtr, *out2ImgPtr;
  uint32 outWidth, outHeight, row, col, rowInc, endCol;

  MSG_LOW("ipl_move_film_ycrcb420lp marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_move_film_ycrcb420lp marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = in1_img_ptr->imgPtr;
  in1ClrPtr = in1_img_ptr->clrPtr;

  in2ImgPtr = in2_img_ptr->imgPtr;
  in2ClrPtr = in2_img_ptr->clrPtr;

  outImgPtr = out_img_ptr->imgPtr;
  outClrPtr = out_img_ptr->clrPtr;

  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;

  in12ImgPtr = in1ImgPtr + outWidth;
  in22ImgPtr = in2ImgPtr + outWidth;
  out2ImgPtr = outImgPtr + outWidth;

  /* For now, assume both inputs and output have same width and height */
  endCol = boundary;
  if (endCol%2) {
    endCol++;
  }

  /* Initialize the index to starting position */
  rowInc = (outWidth - endCol) ;
  in1ImgPtr  += rowInc;  /* byte addressed */
  in12ImgPtr += rowInc;
  in1ClrPtr  += rowInc;

  MSG_LOW("ipl_move_film_ycrcb420lp marker_1\n");

   /* Loop through end part of 1st input image */
  for(row = outHeight; row; row-=2) 
  {
    for(col = 0; col < endCol; col+=2) 
    {
      *outImgPtr++ = *in1ImgPtr++;
      *outImgPtr++ = *in1ImgPtr++;

      *out2ImgPtr++ = *in12ImgPtr++;
      *out2ImgPtr++ = *in12ImgPtr++;
      
      *outClrPtr++ = *in1ClrPtr++;
      *outClrPtr++ = *in1ClrPtr++;
    } /* end of col loop */
    for(col = endCol; col < outWidth; col+=2) 
    {
      *outImgPtr++ = *in2ImgPtr++;
      *outImgPtr++ = *in2ImgPtr++;
      
      *out2ImgPtr++ = *in22ImgPtr++;
      *out2ImgPtr++ = *in22ImgPtr++;
      
      *outClrPtr++ = *in2ClrPtr++;
      *outClrPtr++ = *in2ClrPtr++;
    } /* end of col loop */
    in1ImgPtr  += rowInc + outWidth;
    in12ImgPtr += rowInc + outWidth;
    
    in2ImgPtr  += endCol + outWidth;
    in22ImgPtr += endCol + outWidth;
    
    outImgPtr  += outWidth;
    out2ImgPtr += outWidth;
    
    in1ClrPtr  += rowInc;
    in2ClrPtr  += endCol;
  } /* end of row loop */

  MSG_LOW("ipl_move_film_ycrcb420lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_move_film_ycrcb420lp */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_move_film

DESCRIPTION
  This function combines the end part of the first image with the 
  beginning part of the second image to simulate the effect of a 
  moving film strip. The boundary indicates where the second image 
  starts.
  Examples:
    (1) if boundary = 0, output = second image only;
    (2) if boundary = output width, output = first image only.

  Input and output images must have the same dimensions. 
  Input and output images must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
  boundary      location of the boundary

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_move_film
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{

  MSG_LOW("ipl_move_film marker_0\n");

  /* Check input parameters */
  if (in1_img_ptr == NULL || in2_img_ptr == NULL || 
      out_img_ptr == NULL)
  {
    MSG_LOW("ipl_move_film marker_200\n");
    return IPL_FAILURE;
  }
  /* Input and output color formats must be the same */
  if (in1_img_ptr->cFormat != in2_img_ptr->cFormat || 
      in1_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_move_film marker_201\n");
    return IPL_FAILURE;
  }
  /* Input and output image sizes must be the same */
  if (in1_img_ptr->dx != in2_img_ptr->dx ||
      in1_img_ptr->dx != out_img_ptr->dx ||
      in1_img_ptr->dy != in2_img_ptr->dy ||
      in1_img_ptr->dy != out_img_ptr->dy) 
  {
    MSG_LOW("ipl_move_film marker_202\n");
    return IPL_FAILURE;
  }
  /* Boundary must not exceed output image boundaries */
  if (boundary >= out_img_ptr->dx)
  {
    MSG_LOW("ipl_move_film marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_move_film marker_1\n");

  /* Call the appropriate function */
  if (in1_img_ptr->cFormat == IPL_RGB565) 
  {
    if (ipl_move_film_rgb565(in1_img_ptr, in2_img_ptr, out_img_ptr, boundary) 
        != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_move_film marker_204\n");
      return IPL_FAILURE;
    }
  } 
  else if (in1_img_ptr->cFormat == IPL_YCbCr) 
  {
    if (ipl_move_film_ycbcr(in1_img_ptr, in2_img_ptr, out_img_ptr, boundary) 
        != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_move_film marker_205\n");
      return IPL_FAILURE;
    }
  } 
  else if ((in1_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)  ||
           (in1_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)) 
  {
    if (ipl_move_film_ycrcb420lp(in1_img_ptr, in2_img_ptr, out_img_ptr, boundary) != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_move_film marker_206\n");
      return IPL_FAILURE;
    }
  } 
  else 
  {
    MSG_LOW("ipl_move_film marker_207\n");
    /* No other formats supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_move_film marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_move_film */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_overlap_rgb565

DESCRIPTION
  This function overlaps one image over another.  The overlap region may be 
  placed anywhere as long as it fits within the boundaries of the first input  
  image.  If the overlap region has dimensions different from the second input 
  image, the second image will be resized.  The output image must have the 
  same dimensions as the first input image.
  
  The inputs must be RGB565.
  The output is RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  overlap points to a structure indicating the overlap region

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_overlap_rgb565
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  ipl_rect_type* overlap          /* Overlap region                   */
)
{
  uint16 *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, overlapWidth, overlapHeight,
         row, col, inc;
  ipl_image_type in2new_img;

  MSG_LOW("ipl_overlap_rgb565 marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !overlap)
  {
    MSG_LOW("ipl_overlap_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  /* Output image must have the same size as 1st input image */
  if (out_img_ptr->dx != in1_img_ptr->dx ||
      out_img_ptr->dy != in1_img_ptr->dy) {
    MSG_LOW("ipl_overlap_rgb565 marker_201\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = (uint16*)in1_img_ptr->imgPtr;
  in2ImgPtr = (uint16*)in2_img_ptr->imgPtr;
  outImgPtr = (uint16*)out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;
  overlapWidth = overlap->dx;
  overlapHeight = overlap->dy;

  MSG_LOW("ipl_overlap_rgb565 marker_1\n");

  /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
    memcpy(outImgPtr, in1ImgPtr, outWidth*outHeight*2);

  if (overlapWidth < in2_img_ptr->dx || overlapHeight < in2_img_ptr->dy)
  {
    /* Downsize 2nd Input Image */
    in2new_img.dx = overlapWidth;
    in2new_img.dy = overlapHeight;
    in2new_img.cFormat = out_img_ptr->cFormat;
    if (ipl_malloc_img(&in2new_img))
    {
      MSG_LOW("ipl_overlap_rgb565 marker_202\n");
      return IPL_NO_MEMORY;
    }
    if (ipl_downsize(in2_img_ptr, &in2new_img, NULL) != IPL_SUCCESS) 
    {
      ipl_free_img(&in2new_img);
      MSG_LOW("ipl_overlap_rgb565 marker_203\n");
      return IPL_FAILURE;
    }
    in2ImgPtr = (uint16*) in2new_img.imgPtr;
  }

  /* Initialize index to starting position */
  outImgPtr += overlap->x + outWidth * overlap->y;  /* byte addressed */
  inc = outWidth - overlapWidth;

  /* Loop through downsized 2nd input image */
  for(row = overlapHeight; row; row--)
  {
    for(col = overlapWidth; col; col--)
    {
      *outImgPtr++ = *in2ImgPtr++;
    } 
    outImgPtr += inc;
  } 

  if (overlapWidth < in2_img_ptr->dx || overlapHeight < in2_img_ptr->dy)
  {
    ipl_free_img(&in2new_img); //lint !e644
  }

  MSG_LOW("ipl_overlap_rgb565 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_overlap_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_overlap_ycbcr

DESCRIPTION
  This function overlaps one image over another.  The overlap region may be 
  placed anywhere as long as it fits within the boundaries of the first input  
  image.  If the overlap region has dimensions different from the second input 
  image, the second image will be resized.  The output image must have the 
  same dimensions as the first input image.
  
  The inputs must be YCbCr.
  The output is YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  overlap points to a structure indicating the overlap region

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_overlap_ycbcr
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  ipl_rect_type* overlap          /* Overlap region                   */
)
{
  uint8 *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, overlapWidth, overlapHeight;
  uint32 row, col, inc;
  ipl_image_type in2new_img;

  MSG_LOW("ipl_overlap_ycbcr marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !overlap ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_overlap_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  /* Overlap region must fit within 1st input (background) image */
  if ((overlap->x + overlap->dx) >= in1_img_ptr->dx ||
      (overlap->y + overlap->dy) >= in1_img_ptr->dy)
  {
    MSG_LOW("ipl_overlap_ycbcr marker_201\n");
    return IPL_FAILURE;
  }
  /* Output image must have the same size as 1st input image */
  if (out_img_ptr->dx != in1_img_ptr->dx ||
      out_img_ptr->dy != in1_img_ptr->dy) {
    MSG_LOW("ipl_overlap_ycbcr marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = in1_img_ptr->imgPtr;
  in2ImgPtr = in2_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;
  overlapWidth = overlap->dx;
  overlapHeight = overlap->dy;

  // make sure even width
  if (overlapWidth & 0x1)
  {
    overlapWidth--;
  }

  MSG_LOW("ipl_overlap_ycbcr marker_1\n");

  /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
  {
    memcpy(outImgPtr, in1ImgPtr, outWidth*outHeight*2);
  }

  if (overlapWidth < in2_img_ptr->dx || overlapHeight < in2_img_ptr->dy)
  {
    /* Downsize 2nd Input Image */
    in2new_img.dx = overlapWidth;
    in2new_img.dy = overlapHeight;
    in2new_img.cFormat = out_img_ptr->cFormat;

    if (ipl_malloc_img(&in2new_img))
    {
      MSG_LOW("ipl_overlap_ycbcr marker_203\n");
      return IPL_NO_MEMORY;
    }
    if (ipl_downsize(in2_img_ptr, &in2new_img, NULL) != IPL_SUCCESS) 
    {
      ipl_free_img(&in2new_img);
      MSG_LOW("ipl_overlap_ycbcr marker_204\n");
      return IPL_FAILURE;
    }
    in2ImgPtr = in2new_img.imgPtr;
  } 

 

  /* Initialize index to starting position */
  // make sure we copy and paste to even location
  if (overlap->x & 0x1)
    outImgPtr += (overlap->x + 1 + outWidth * overlap->y) << 1;
  else
    outImgPtr += (overlap->x + outWidth * overlap->y) << 1;

  inc = (outWidth - overlapWidth) << 1;

  /* Loop through downsized 2nd input image */
  for(row = overlapHeight; row; row--)
  {
    for(col = overlapWidth; col; col-=2)
    {
      *outImgPtr++ = *in2ImgPtr++;
      *outImgPtr++ = *in2ImgPtr++;
      *outImgPtr++ = *in2ImgPtr++;
      *outImgPtr++ = *in2ImgPtr++;
    } /* end of col loop */
    outImgPtr += inc;
  } /* end of row loop */

  if (overlapWidth < in2_img_ptr->dx || overlapHeight < in2_img_ptr->dy)
    ipl_free_img(&in2new_img); 

  MSG_LOW("ipl_overlap_ycbcr marker_100\n");

  return IPL_SUCCESS;
} 

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_overlap_ycrcb420lp

DESCRIPTION
  This function overlaps one image over another.  The overlap region may be 
  placed anywhere as long as it fits within the boundaries of the first input  
  image.  If the overlap region has dimensions different from the second input 
  image, the second image will be resized.  The output image must have the 
  same dimensions as the first input image.
  
  The inputs must be YCrCb 4:2:0 line packed..
  The output is YCrCb 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  overlap points to a structure indicating the overlap region

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_overlap_ycrcb420lp
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  ipl_rect_type* overlap          /* Overlap region                   */
)
{
  ipl_image_type in2new_img;

  MSG_LOW("ipl_overlap_ycrcb420lp marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !overlap ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_overlap_ycrcb420lp marker_200\n");
    return IPL_FAILURE;
  }


  if (in1_img_ptr->dx != out_img_ptr->dx &&
      in1_img_ptr->dy != out_img_ptr->dy)
  {
    MSG_LOW("ipl_overlap_ycrcb420lp marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_overlap_ycrcb420lp marker_1\n");

  /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
  {
    /* Initialize image pointers and local variables */
    memcpy(out_img_ptr->imgPtr, in1_img_ptr->imgPtr, in1_img_ptr->dx*in1_img_ptr->dy);
    memcpy(out_img_ptr->clrPtr, in1_img_ptr->clrPtr, (in1_img_ptr->dx*in1_img_ptr->dy>>1));
  }

  // if we are up or downsizing
  if (overlap->dx != in2_img_ptr->dx || overlap->dy != in2_img_ptr->dy)
  {
    /* Downsize 2nd Input Image */
    in2new_img.dx = 2*(overlap->dx/2);
    in2new_img.dy = 2*(overlap->dy/2);
    in2new_img.cFormat = out_img_ptr->cFormat;
    if (ipl_malloc_img(&in2new_img))
    {
      MSG_LOW("ipl_overlap_ycrcb420lp marker_202\n");
      return IPL_NO_MEMORY;
    }
    if (ipl_crop_resize_rot(in2_img_ptr, &in2new_img, NULL, NULL, IPL_NOROT, IPL_QUALITY_MEDIUM) != IPL_SUCCESS) 
    {
      ipl_free_img(&in2new_img);
      MSG_LOW("ipl_overlap_ycrcb420lp marker_203\n");
      return IPL_FAILURE;
    }

    if (ipl_copy_and_paste(&in2new_img, out_img_ptr, NULL, overlap)) 
    {
      ipl_free_img(&in2new_img);
      MSG_LOW("ipl_overlap_ycrcb420lp marker_204\n");
      return IPL_FAILURE;
    }

    ipl_free_img(&in2new_img);
  } 
  else
  {
    if (ipl_copy_and_paste(in2_img_ptr, out_img_ptr, NULL, overlap)) 
    {
      MSG_LOW("ipl_overlap_ycrcb420lp marker_205\n");
      return IPL_FAILURE;
    }
  }
  MSG_LOW("ipl_overlap_ycrcb420lp marker_100\n");
  return IPL_SUCCESS;
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_overlap

DESCRIPTION
  This function overlaps one image on top of another. The overlap region may 
  be placed anywhere as long as it fits within the boundaries of the first 
  input image. If the overlap region is smaller than the second input image, 
  the second image will be downsized. The output image must have the same 
  dimensions as the first input image. Overlap regions that are larger than 
  the second input image are not supported currently.
  
  Input and output images must have the same color format, which can be 
  RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
  overlap       pointer to a structure indicating the overlap region

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_overlap
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  ipl_rect_type* overlap          /* Overlap region                   */
)
{
  ipl_status_type retval;

  MSG_LOW("ipl_overlap marker_0\n");

  /* Check input parameters */
  if (in1_img_ptr == NULL || in2_img_ptr == NULL || 
      out_img_ptr == NULL || overlap == NULL)
  {
    MSG_LOW("ipl_overlap marker_200\n");
    return IPL_FAILURE;
  }
  /* Input and output color formats must be equal */
  if (in1_img_ptr->cFormat != in2_img_ptr->cFormat || 
      in1_img_ptr->cFormat != out_img_ptr->cFormat ||
      in2_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_overlap marker_201\n");
    return IPL_FAILURE;
  }
  /* 
  ** Upper left corner of overlap region must be within input image 
  ** boundaries
  */
  if (overlap->x > in1_img_ptr->dx || 
      overlap->y > in1_img_ptr->dy)
  {
    MSG_LOW("ipl_overlap marker_202\n");
    return IPL_FAILURE;
  }
  /* Overlap region must fit within 1st input (background) image */
  if ((overlap->x + overlap->dx) >= in1_img_ptr->dx ||
      (overlap->y + overlap->dy) >= in1_img_ptr->dy)
  {
    MSG_LOW("ipl_overlap marker_203\n");
    return IPL_FAILURE;
  }
  /* Output image must have the same size as 1st input image */
  if (out_img_ptr->dx != in1_img_ptr->dx ||
      out_img_ptr->dy != in1_img_ptr->dy) {
    MSG_LOW("ipl_overlap marker_204\n");
    return IPL_FAILURE;
  }
  
  /* Only downsizing supported for 2nd input (foreground) image */
  /*
  if (overlap->dx > in2_img_ptr->dx ||
      overlap->dy > in2_img_ptr->dy)
  {
    return IPL_FAILURE;
  }
  */

  MSG_LOW("ipl_overlap marker_1\n");

  /* Call the appropriate function */
  if (in1_img_ptr->cFormat == IPL_RGB565) 
  {
    retval = ipl_overlap_rgb565(in1_img_ptr, in2_img_ptr, out_img_ptr, overlap);
    if (retval != IPL_SUCCESS)
    {
      MSG_LOW("ipl_overlap marker_205\n");
      return retval;
    }
  } 
  else if (in1_img_ptr->cFormat == IPL_YCbCr) 
  {
    retval = ipl_overlap_ycbcr(in1_img_ptr, in2_img_ptr, out_img_ptr, overlap);
    if (retval != IPL_SUCCESS)
    {
      MSG_LOW("ipl_overlap marker_206\n");
      return retval;
    }
  } 
  else if ((in1_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)  ||
           (in1_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)) 
  {
     retval = ipl_overlap_ycrcb420lp(in1_img_ptr, in2_img_ptr, out_img_ptr, overlap);
     if (retval != IPL_SUCCESS)
     {
       MSG_LOW("ipl_overlap marker_207\n");
       return retval;
     }
  } 
  else
  {
    MSG_LOW("ipl_overlap marker_208\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_overlap marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_overlap */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose_vertical_rgb565

DESCRIPTION
  This function combines two images by taking the top part of the first 
  image and the bottom part of the second image.  Both inputs and output 
  must have the same width and height.  The boundary indicates where
  the second image starts.  
  Examples:
  (1) if boundary = 0, output = second image only;
  (2) if boundary = output height, output = first image only.
  
  Both inputs and output must by RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  boundary indicates location of the boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_compose_vertical_rgb565
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{
  uint16 *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, row, col;
  uint32 outIndex;

  MSG_LOW("ipl_compose_vertical_rgb565 marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_compose_vertical_rgb565 marker_200\n");
    return IPL_FAILURE;
  }
      

  /* Initialize image pointers and local variables */
  in1ImgPtr = (uint16*) in1_img_ptr->imgPtr;
  in2ImgPtr = (uint16*) in2_img_ptr->imgPtr;
  outImgPtr = (uint16*) out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;

  /* Initialize the index to starting position */
  outIndex = 0;

  MSG_LOW("ipl_compose_vertical_rgb565 marker_1\n");

  /* Loop through top part of 1st input */
  /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
  {
  for(row = boundary; row; row--) {
    for(col = outWidth; col; col--) {
      *outImgPtr++ = *((uint16*)(in1ImgPtr+outIndex++));
    } /* end of col loop */
  } /* end of row loop */
  } /* end of not inplace */
  else
  {
    outImgPtr += boundary * outWidth;
    outIndex  += boundary * outWidth;
  }

  /* Loop through bottom part of 2nd input */
  for(row = boundary; row < outHeight; row++) {
    for(col = outWidth; col; col--) {
      *outImgPtr++ = *((uint16*)(in2ImgPtr+outIndex++));
    } /* end of col loop */
  } /* end of row loop */

  MSG_LOW("ipl_compose_vertical_rgb565 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_compose_vertical_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose_vertical_ycbcr

DESCRIPTION
  This function combines two images by taking the top part of the first 
  image and the bottom part of the second image.  Both inputs and output 
  must have the same width and height.  The boundary indicates where
  the second image starts.  
  Examples:
  (1) if boundary = 0, output = second image only;
  (2) if boundary = output height, output = first image only.
  
  Both inputs and output must by YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  boundary indicates location of the boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_compose_vertical_ycbcr
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,      /* Points to the output image       */
  uint32 boundary          /* Boundary location        */
)
{
  unsigned char *in1ImgPtr, *in2ImgPtr, *outImgPtr;
  uint32 outWidth, outHeight, row, col;
  uint32 outIndex;

  MSG_LOW("ipl_compose_vertical_ycbcr marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_compose_vertical_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  in1ImgPtr = in1_img_ptr->imgPtr;
  in2ImgPtr = in2_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;

  /* Initialize the index to starting position */
  outIndex = 0;  /* byte addressed */

  MSG_LOW("ipl_compose_vertical_ycbcr marker_1\n");

  /* Loop through top part of 1st input */
  /* if not inplace */
  if (out_img_ptr != in1_img_ptr)
  {  
  for(row = boundary; row ; row--) {
    for(col = outWidth; col ; col-=2) {
      *outImgPtr++ = *((uint8*)(in1ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in1ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in1ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in1ImgPtr + outIndex++));
     } /* end of col loop */
    } /* end of row loop */
  } /* end of not inplace */
  else
  {
    outImgPtr += ((boundary * outWidth) << 2) >> 1;
    outIndex  += ((boundary * outWidth) << 2) >> 1;
  }

  /* Loop through bottom part of 2nd input */
  for(row = boundary; row < outHeight; row++) {
    for(col = outWidth; col; col-=2) {
      *outImgPtr++ = *((uint8*)(in2ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr + outIndex++));
      *outImgPtr++ = *((uint8*)(in2ImgPtr + outIndex++));
    } /* end of col loop */
  } /* end of row loop */

  MSG_LOW("ipl_compose_vertical_ycbcr marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_compose_vertical_ycbcr */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose_vertical_ycbcr_lp

DESCRIPTION
  This function combines two images by taking the top part of the first 
  image and the bottom part of the second image.  Both inputs and output 
  must have the same width and height.  The boundary indicates where
  the second image starts.  
  Examples:
  (1) if boundary = 0, output = second image only;
  (2) if boundary = output height, output = first image only.
  
  Both inputs and output must by YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr points to the first input image
  in2_img_ptr points to the second input image
  out_img_ptr points to the output image
  boundary indicates location of the boundary between input images

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_compose_vertical_ycbcr_lp
(
  ipl_image_type* in1_img_ptr,  /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,  /* Points to the second input image */
  ipl_image_type* out_img_ptr,  /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{
  uint32 endRow;

  ipl_rect_type copy_area;
  ipl_rect_type paste_area;

  MSG_LOW("ipl_compose_vertical_ycbcr_lp marker_0\n");

  if (!in1_img_ptr || !in1_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_compose_vertical_ycbcr_lp marker_200\n");
    return IPL_FAILURE;
  }

  /* Find endpoint */
  endRow = boundary;
  if (endRow%2) 
    endRow++;

  MSG_LOW("ipl_compose_vertical_ycbcr_lp marker_1\n");

  // if we are not doing this inplace
  if (out_img_ptr != in1_img_ptr)
  {
    // put top portion
    copy_area.x = 0;
    copy_area.y = 0;
    copy_area.dx = in1_img_ptr->dx;
    copy_area.dy = endRow;

    paste_area.x = 0;
    paste_area.y = 0;
    paste_area.dx = in1_img_ptr->dx;
    paste_area.dy = endRow;

    if (ipl_copy_and_paste(in1_img_ptr, out_img_ptr, &copy_area, &paste_area) 
        != IPL_SUCCESS)
    {
      MSG_LOW("ipl_compose_vertical_ycbcr_lp marker_201\n");
      return IPL_FAILURE;
    }
    // put bottom portion
    copy_area.x = 0;
    copy_area.y = endRow;
    copy_area.dx = in1_img_ptr->dx;
    copy_area.dy = in1_img_ptr->dy-endRow;

    paste_area.x = 0;
    paste_area.y = endRow;
    paste_area.dx = in1_img_ptr->dx;
    paste_area.dy = in1_img_ptr->dy-endRow;

    if (ipl_copy_and_paste(in2_img_ptr, out_img_ptr, &copy_area, &paste_area) 
        != IPL_SUCCESS)
    {
      MSG_LOW("ipl_compose_vertical_ycbcr_lp marker_202\n");
      return IPL_FAILURE;
    }
  }
  else
  {
    // put bottom portion
    copy_area.x = 0;
    copy_area.y = endRow;
    copy_area.dx = in1_img_ptr->dx;
    copy_area.dy = in1_img_ptr->dy-endRow;

    paste_area.x = 0;
    paste_area.y = endRow;
    paste_area.dx = in1_img_ptr->dx;
    paste_area.dy = in1_img_ptr->dy-endRow;

    // just put part of the 2nd image over 1st and get out of dodge
    if (ipl_copy_and_paste(in2_img_ptr, in1_img_ptr, &copy_area, &paste_area) 
        != IPL_SUCCESS)
    {
      MSG_LOW("ipl_compose_vertical_ycbcr_lp marker_203\n");
      return IPL_FAILURE;
    }
  }

  MSG_LOW("ipl_compose_vertical_ycbcr_lp marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_compose_vertical_ycbcr */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose_vertical

DESCRIPTION
  This function combines two images by taking the top part of the first image 
  and the bottom part of the second image. The boundary indicates where the 
  second image starts.  
  Examples:
    (1) if boundary = 0, output = second image only;
    (2) if boundary = output height, output = first image only.
  
  Input and output images must have the same dimensions.
  Input and output images must have the same color format, which can be 
  RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
  boundary      location of boundary

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_compose_vertical
(
  ipl_image_type* in1_img_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,    /* Points to the second input image */
  ipl_image_type* out_img_ptr,    /* Points to the output image       */
  uint32 boundary                 /* Boundary location                */
)
{
  MSG_LOW("ipl_compose_vertical marker_0\n");

  /* Check input parameters */
  if (in1_img_ptr == NULL || in2_img_ptr == NULL || 
      out_img_ptr == NULL)
  {
    MSG_LOW("ipl_compose_vertical marker_200\n");
    return IPL_FAILURE;
  }
  /* Input and output color formats must be the same */
  if (in1_img_ptr->cFormat != in2_img_ptr->cFormat || 
      in1_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_compose_vertical marker_201\n");
    return IPL_FAILURE;
  }
  /* Input and output image sizes must be the same */
  if (in1_img_ptr->dx != in2_img_ptr->dx ||
      in1_img_ptr->dx != out_img_ptr->dx ||
      in1_img_ptr->dy != in2_img_ptr->dy ||
      in1_img_ptr->dy != out_img_ptr->dy) 
  {
    MSG_LOW("ipl_compose_vertical marker_202\n");
    return IPL_FAILURE;
  }
  /* Boundary must not exceed output image boundaries */
  if (boundary >= out_img_ptr->dy)
  {
    MSG_LOW("ipl_compose_vertical marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_compose_vertical marker_1\n");

  /* Call the appropriate function */
  if (in1_img_ptr->cFormat == IPL_RGB565) {
    if (ipl_compose_vertical_rgb565( \
          in1_img_ptr, in2_img_ptr, 
          out_img_ptr, boundary)
      != IPL_SUCCESS) {
      MSG_LOW("ipl_compose_vertical marker_204\n");
      return IPL_FAILURE;
    }
  } else if (in1_img_ptr->cFormat == IPL_YCbCr) {
    if (ipl_compose_vertical_ycbcr( \
          in1_img_ptr, in2_img_ptr, 
          out_img_ptr, boundary)
      != IPL_SUCCESS) {
      MSG_LOW("ipl_compose_vertical marker_205\n");
      return IPL_FAILURE;
    }
  } 
  else if ((in1_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) || 
           (in1_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)) 
  {
    if (ipl_compose_vertical_ycbcr_lp( \
          in1_img_ptr, in2_img_ptr, 
          out_img_ptr, boundary) != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_compose_vertical marker_206\n");
      return IPL_FAILURE;
    }
  } 
  else 
  {
    MSG_LOW("ipl_compose_vertical marker_207\n");
    /* No other formats supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_compose_vertical marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_compose_vertical */




/*==========================================================================

FUNCTION    ipl_overlay_inplace

DESCRIPTION
  This function takes three images. One is the image on which overlay is to be 
  done and the another is the overlay image. A third image is the alpha channel.
  There is also a crop which tells where to put the overlay, and how much of 
  it to reveal.

  Input, frame, and output images must be in YCrCb 4:2:0 line packed format.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         points to the input image
  i_frame_ptr       points to the frame image
  i_alpha_ptr       points to alpha values of each pixel of the frame_ptr
  crop              points to struct saying where and how much of frame to 
                    reveal

ARGUMENTS IN/OUT
  

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None
===========================================================================*/
extern ipl_status_type ipl_overlay_inplace
(
  ipl_image_type* i_img_ptr,
  ipl_image_type* i_frame_ptr,
  ipl_image_type* i_alpha_ptr,
  ipl_rect_type*  icrop
)
{
  unsigned char *inImgPtr, *inClrPtr;   // points to input/output
  unsigned char *fImgPtr, *fClrPtr;     // points to frame
  unsigned char *aPtr;                  // points to alpha channel
  ipl_rect_type crop;

  uint32 x, y;
  uint32 fPitch, iPitch;
  uint8 alpha;

  uint32 idx, idy, fdx, fdy;

  MSG_LOW("ipl_overlay_inplace marker_0\n");

  // make sure we have data
  if (!i_img_ptr   || !i_img_ptr->imgPtr   ||
      !i_frame_ptr || !i_frame_ptr->imgPtr || 
      !i_alpha_ptr || !i_alpha_ptr->imgPtr)
  {
    MSG_LOW("ipl_overlay_inplace marker_200\n");
    return IPL_FAILURE;
  }

  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;
  fdx = i_frame_ptr->dx;
  fdy = i_frame_ptr->dy;

  // as a convenience, lets allow the user to skip passing the crop
  // field if he wants to crop the entire frame at the origin of input
  if (icrop)
  {
    crop.x = icrop->x;
    crop.y = icrop->y;
    crop.dx = icrop->dx;
    crop.dy = icrop->dy;
  }
  else
  {
    crop.x = 0;
    crop.y = 0;
    crop.dx = fdx;
    crop.dy = fdy;
  }


  // make sure frame and and alpha channel have samewe have data
  if (fdx != i_alpha_ptr->dx || fdy != i_alpha_ptr->dy)
  {
    MSG_LOW("ipl_overlay_inplace marker_201\n");
    return IPL_FAILURE;
  }
  if (i_alpha_ptr->cFormat != IPL_ALPHA && 
      i_alpha_ptr->cFormat != IPL_LUMA_ONLY)
  {
    MSG_LOW("ipl_overlay_inplace marker_202\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_overlay_inplace marker_1\n");
  if ((i_img_ptr->cFormat   == IPL_YCrCb420_LINE_PK &&
       i_frame_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (i_img_ptr->cFormat   == IPL_YCbCr420_LINE_PK &&
       i_frame_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    // since we are working with line pack data, make sure crop 
    // is multiple of two
    crop.x  &= 0xFFFE;
    crop.y  &= 0xFFFE;
    crop.dx &= 0xFFFE;
    crop.dy &= 0xFFFE;

    // make sure the crop width is not bigger than the frame, also make
    // sure that the start and length of crop does not go past image 
    if (crop.dx > fdx || crop.x + crop.dx > idx || 
        crop.dy > fdy || crop.y + crop.dy > idy)
    {
      MSG_LOW("ipl_overlay_inplace marker_203\n");
      return IPL_FAILURE;
    }
    /* where should we write our data */
    inImgPtr = i_img_ptr->imgPtr + crop.x + (crop.y * idx);
    inClrPtr = i_img_ptr->clrPtr + crop.y + (crop.y * idy/2);
  
    /* we always start at the frame from the begining */
    fImgPtr = i_frame_ptr->imgPtr;
    fClrPtr = i_frame_ptr->clrPtr;

    /*get cbcr location for overlay image */
    aPtr = i_alpha_ptr->imgPtr;

    fPitch = fdx - crop.dx;
    iPitch = idx - crop.dx; 

    // go through the crop area and do the blending 
    for(y = crop.dy/2; y; y--)
    {
      for(x = crop.dx/2; x; x--)
      {
        alpha = *aPtr;
        *(inImgPtr) = (((*inImgPtr * alpha)>>7) +
                       ((*fImgPtr  * (128-alpha))>>7));

        alpha = *(aPtr+1);
        *(inImgPtr+1) = (((*(inImgPtr+1) * alpha)>>7) +
                         ((*(fImgPtr+1)  * (128-alpha))>>7));

        alpha = *(aPtr+fdx);
        *(inImgPtr+idx) = (((*(inImgPtr+idx) * alpha)>>7) +
                           ((*(fImgPtr+fdx)  * (128-alpha))>>7));

        alpha = *(aPtr+fdx+1);
        *(inImgPtr+idx+1) = (((*(inImgPtr+idx+1) * alpha)>>7) +
                             ((*(fImgPtr+fdx+1)  * (128-alpha))>>7));

        *inClrPtr = (((*inClrPtr * alpha)>>7) +
                       ((*fClrPtr++  * (128-alpha))>>7)); 
        inClrPtr++;

        *inClrPtr = (((*inClrPtr * alpha)>>7) +
                       ((*fClrPtr++  * (128-alpha))>>7)); 
        inClrPtr++;
       

        inImgPtr += 2;
        fImgPtr  += 2;
        aPtr     += 2;
      }

      inImgPtr += (idx + iPitch);
      fImgPtr  += (fdx + fPitch);
      aPtr     += (fdx + fPitch);
    }
  }
  else
  {
    MSG_LOW("ipl_overlay_inplace marker_204\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_overlay_inplace marker_100\n");
  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_subtract_ys

DESCRIPTION

  This function subtracts the Y channel of two YCbCr images. The Cb of
  the first input image is copied to output image.
  

  Input and output images must have the same color format, which can 
  be YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr     pointer to the input image
  o_img_ptr     pointer to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type 
ipl_subtract_ys(ipl_image_type * in_img_ptr, 
                ipl_image_type * in2_img_ptr, 
                ipl_image_type * out_img_ptr) 
{
  unsigned int x;
  int diff;
  unsigned char * in; 
  unsigned char * in2; 
  unsigned char * out; 

  MSG_LOW("ipl_subtract_ys marker_0\n");

  if (!in_img_ptr || !in_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_subtract_ys marker_200\n");
    return IPL_FAILURE;
  }

  in = in_img_ptr->imgPtr;
  in2 = in2_img_ptr->imgPtr;
  out = out_img_ptr->imgPtr;

  MSG_LOW("ipl_subtract_ys marker_1\n");

  if ((in_img_ptr->cFormat == IPL_YCbCr) && 
      (in2_img_ptr->cFormat == IPL_YCbCr) && 
      (out_img_ptr->cFormat == IPL_YCbCr))
  {
    for (x = 0; x < (in_img_ptr->dx * in_img_ptr->dy * 2); x++)
    {
      if (x%2)
      {
        // subtract Ys
        diff = *in - *in2;

        if (diff < 0)
          *out = 0;
        else
          *out = diff;
      }
      else
      {
        // copy Cb or Cr of original image over to output
        *out = *in;
      }

      in++;
      in2++;
      out++;
    }
  }
  else if ((in_img_ptr->cFormat == IPL_YCbCr) && 
           (in2_img_ptr->cFormat == IPL_YCbCr) && 
           (out_img_ptr->cFormat == IPL_LUMA_ONLY))
  {
    // get to Y
    in++;
    in2++;

    for (x = 0; x < in_img_ptr->dx * in_img_ptr->dy; x++)
    {
      // subtract first Ys
      diff = *in - *in2;

      if (diff < 0)
        *out = 0;
      else
        *out = diff;

      in += 2;
      in2 += 2;
      out++;
    }
  }
  else if ((in_img_ptr->cFormat == IPL_YCbCr) && 
           (in2_img_ptr->cFormat == IPL_LUMA_ONLY) && 
           (out_img_ptr->cFormat == IPL_LUMA_ONLY))
  {
    // get to Y
    in++;

    for (x = 0; x < in_img_ptr->dx * in_img_ptr->dy; x++)
    {
      // subtract first Ys
      diff = *in - *in2;

      if (diff < 0)
        *out = 0;
      else
        *out = diff;

      in += 2;
      in2++;
      out++;
    }
  }
  else if((in_img_ptr->cFormat == IPL_LUMA_ONLY) && 
          (in2_img_ptr->cFormat == IPL_LUMA_ONLY) &&
          (out_img_ptr->cFormat == IPL_LUMA_ONLY))
  {
    for (x = 0; x < in_img_ptr->dx * in_img_ptr->dy; x++)
    {
      // subtract first Ys
      diff = *in - *in2;

      if (diff < 0)
        *out = 0;
      else
        *out = diff;

      in++;
      in2++;
      out++;
    }
  }
  else if((in_img_ptr->cFormat == IPL_LUMA_ONLY) && 
          (in2_img_ptr->cFormat == IPL_YCbCr) &&
          (out_img_ptr->cFormat == IPL_LUMA_ONLY))
  {
    // get to Y
    in2++;

    for (x = 0; x < in_img_ptr->dx * in_img_ptr->dy; x++)
    {
      // subtract first Ys
      diff = *in - *in2;

      if (diff < 0)
        *out = 0;
      else
        *out = diff;

      in++;
      in2 += 2;
      out++;
    }
  }
  else
  {
    MSG_LOW("ipl_subtract_ys marker_201\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_subtract_ys marker_100\n");
  return IPL_SUCCESS;
}


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_subtract_ycbcr

DESCRIPTION

  This function subtracts the Y,Cb,Cr of two images and puts the value in Luma.
  It put 128 for Cb and Cr.

  Input and output images must have the same color format, which can 
  be YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr     pointer to the input image
  o_img_ptr     pointer to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type 
ipl_subtract_ycbcr(ipl_image_type * in_img_ptr, 
                   ipl_image_type * in2_img_ptr, 
                   ipl_image_type * out_img_ptr) 
{
  unsigned int x;
  int max, diffY1, diffY2, diffCb, diffCr;
  unsigned char * bg; 
  unsigned char * orig; 
  unsigned char * out; 

  MSG_LOW("ipl_subtract_ycbcr marker_0\n");

  if (!in_img_ptr || !in_img_ptr->imgPtr ||
      !in2_img_ptr || !in2_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_subtract_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  bg = in_img_ptr->imgPtr;
  orig = in2_img_ptr->imgPtr;
  out = out_img_ptr->imgPtr;

  if (in_img_ptr->cFormat != IPL_YCbCr)
  {
    MSG_LOW("ipl_subtract_ycbcr marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_subtract_ycbcr marker_1\n");
#if 1
  for (x = 0; x < (in_img_ptr->dx * in_img_ptr->dy * 2); x+= 4)
  {
    diffCb = *bg++ - *(orig);
    diffY1 = *bg++ - *(orig + 1);
    diffCr = *bg++ - *(orig + 2);
    diffY2 = *bg++ - *(orig + 3);

    if (diffCb < 0) diffCb = 0;
    if (diffCr < 0) diffCr = 0;
    if (diffY1 < 0) diffY1 = 0;
    if (diffY2 < 0) diffY2 = 0;

    // since whiteboard call this function mostly, he wants to know what
    // values are draker than bg, not brighter. I.e. respect the order
    // of the image inputs and subtract one from the other, 
    //diffCb = CLIPIT(diffCb);
    //diffY1 = CLIPIT(diffY1);
    //diffCr = CLIPIT(diffCr);
    //diffY2 = CLIPIT(diffY2);

    *out++ = 128;
    //*out++ = diffCb;
    //*out++ = *(orig);

    //max = diffY1;
    //max = (diffCb > diffCr)? diffCb : diffCr; 
    //max = (max > diffY1)? max : diffY1; 
    max = diffY1 + diffCb + diffCr;
    max <<= 2;
    *out++ = CLIPIT(max);

    *out++ = 128;
    //*out++ = diffCr;
    //*out++ = *(orig + 2);

    //max = diffY2;
    //max = (diffCb > diffCr)? diffCb : diffCr; 
    //max = (max > diffY2)? max : diffY2; 
    max = diffY2 + diffCb + diffCr;
    max <<= 2;
    *out++ = CLIPIT(max);

    orig += 4;
  }
#else

  for (x = 0; x < (in_img_ptr->dx * 2 * in_img_ptr->dy); x++)
  {
    diff = *bg++ - *orig++;

    if (diff < 0)
      diff = 0;

    *out++ = diff;
  }
#endif

  MSG_LOW("ipl_subtract_ycbcr marker_100\n");
  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose

DESCRIPTION
  This function combines two images in various ways.
  
  Input and output images must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image, or some third image if doing 
                operation in place. 
  method        method of compositing the two inputs
  boundary      boundary location

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_compose_image
(
  ipl_image_type* in1_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_ptr,    /* Points to the second input image */
  ipl_image_type* out_ptr,    /* Points to the output image or some 3rd image */
  ipl_compose_type method,    /* Method of compositing            */
  void * misc1,               /* misc input (null or commonly alpha) */
  int misc2,               /* misc input (null or commonly red color) */
  int misc3,               /* misc input (null or commonly green color) */
  int misc4                /* misc input (null or commonly blue color) */
)
{
  if (method == IPL_BLEND_COLOR)
  {
    if (out_ptr == NULL)
      out_ptr = in1_ptr;

    return (ipl_alpha_blend_color(in1_ptr, out_ptr, *((uint32*)misc1), 
          misc2, misc3, misc4));
  }
  else
  {
    return(ipl_compose(in1_ptr, in2_ptr, out_ptr, method, misc1));
  }
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose

DESCRIPTION
  This function combines two images in various ways.
  
  Input and output images must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image, or some third image if doing 
                operation in place. 
  method        method of compositing the two inputs
  boundary      boundary location

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_compose
(
  ipl_image_type* in1_ptr,    /* Points to the first input image  */
  ipl_image_type* in2_ptr,    /* Points to the second input image */
  ipl_image_type* out_ptr,    /* Points to the output image or some 3rd image */
  ipl_compose_type method,    /* Method of compositing            */
  void * misc1                /* Boundary location                */
)
{
  ipl_status_type retval;

  MSG_LOW("ipl_compose marker_0\n");

  // for all of these functions, it is okay to do them inplace
  if (out_ptr == NULL)
    out_ptr = in1_ptr;

  MSG_LOW("ipl_compose marker_1\n");

  switch (method)
  {
    case IPL_BLEND_PER_PIXEL:
      retval=ipl_overlay_inplace(in1_ptr,in2_ptr,out_ptr,(ipl_rect_type*)misc1);
    break;

    case IPL_BLEND:
      if (!misc1)
      {
        MSG_LOW("ipl_compose marker_200\n");
        return IPL_FAILURE;
      }
      retval = ipl_alpha_blend(in1_ptr,in2_ptr,out_ptr,*((uint32*)misc1));
    break;

    case IPL_BLEND_BLACK:
      if (!misc1)
      {
        MSG_LOW("ipl_compose marker_201\n");
        return IPL_FAILURE;
      }
      retval = ipl_alpha_blend_black(in1_ptr,out_ptr,*((uint32*)misc1));
    break;

    case IPL_BLEND_WHITE:
      if (!misc1)
      {
        MSG_LOW("ipl_compose marker_202\n");
        return IPL_FAILURE;
      }
      retval = ipl_alpha_blend_white(in1_ptr,out_ptr,*((uint32*)misc1));
    break;

    case IPL_CENTER:
      if (!misc1) 
      {
        MSG_LOW("ipl_compose marker_203\n");
        return IPL_FAILURE;
      }
      retval = ipl_center(in1_ptr,in2_ptr,out_ptr,*((uint32*)misc1));
    break;

    case IPL_FILMSTRIP:
      if (!misc1)
      {
        MSG_LOW("ipl_compose marker_204\n");
        return IPL_FAILURE;
      }
      retval = ipl_move_film(in1_ptr,in2_ptr,out_ptr,*((uint32*)misc1));
    break;

    case IPL_HORIZONTAL:
      if (!misc1)
      {
        MSG_LOW("ipl_compose marker_205\n");
        return IPL_FAILURE;
      }
      retval =ipl_compose_horizontal(in1_ptr,in2_ptr,out_ptr,*((uint32*)misc1));
    break;

    case IPL_VERTICAL:
      if (!misc1)
      {
        MSG_LOW("ipl_compose marker_206\n");
        return IPL_FAILURE;
      }
      retval = ipl_compose_vertical(in1_ptr,in2_ptr,out_ptr,*((uint32*)misc1));
    break;

    case IPL_OVERLAP:
      if (!misc1) 
      {
        MSG_LOW("ipl_compose marker_207\n");
        return IPL_FAILURE;
      }
      retval = ipl_overlap(in1_ptr,in2_ptr,out_ptr,(ipl_rect_type *)misc1);
    break;

    case IPL_SUBTRACT_LUMA:
      retval = ipl_subtract_ys(in1_ptr, in2_ptr, out_ptr);
    break;

    case IPL_SUBTRACT_YCBCR:
      retval = ipl_subtract_ycbcr(in1_ptr, in2_ptr, out_ptr);
    break;

    default:
      retval = IPL_FAILURE;
    break;
  }

  if (retval == IPL_FAILURE)
    MSG_LOW("ipl_compose marker_208\n");
  else
    MSG_LOW("ipl_compose marker_100\n");

  return retval;
} /* End ipl_compose */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_add

DESCRIPTION
  This function adds a frame around an image and performs color conversion 
  if needed. Frame can be smaller than input image.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr      pointer to the input image
  input_frame_ptr    pointer to the frame image
  transparentPixel   16-bit transparent pixel value
  output_img_ptr     pointer to the output image

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_image_add
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* i_frame_ptr,       /* Points to the frame image      */
  uint16 transparentValue,           /* Transparent pixel value        */
  ipl_image_type* o_img_ptr          /* Points to the output image     */
)
{
  int32 cindex, index,luma1=0,luma2=0, luma3, luma4;
  uint32 row,col;
  uint16 out,out2;
  int32 cb=0,cr=0;
  int32 rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
  int16 rgb2ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,
                                7192,7192,-6029,-1163};

  ipl_image_type* input_img_ptr;
  ipl_image_type* input_frame_ptr;
  ipl_image_type* output_img_ptr;
  uint8* inputImgPtr;
  uint8* inputClrPtr;
  uint8* outputImgPtr;

  uint32 idx, fdx;
  uint8* frameImgPtr;
  boolean convert = TRUE;
  uint8 r1=0,r2=0,g1=0,g2=0,b1=0,b2=0;

  MSG_LOW("ipl_image_add marker_0\n");

  // make sure we have too input
  if (!i_img_ptr   || !i_img_ptr->imgPtr ||
      !o_img_ptr   || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_image_add marker_200\n");
    return IPL_FAILURE;
  }

  // see the user called us just to color convert
  else if (i_frame_ptr == NULL || i_frame_ptr->imgPtr == NULL)
  {
    if (i_img_ptr->cFormat != o_img_ptr->cFormat)
    {
      return(ipl_convert_image(i_img_ptr, o_img_ptr));
    }
    else
    {
      MSG_LOW("ipl_image_add marker_200.5\n");
      return IPL_SUCCESS;
    }
  }
  
  input_img_ptr = i_img_ptr;
  input_frame_ptr = i_frame_ptr;
  output_img_ptr = o_img_ptr;
  inputImgPtr = input_img_ptr->imgPtr;
  inputClrPtr = input_img_ptr->clrPtr;
  outputImgPtr = output_img_ptr->imgPtr;

  //if ((input_img_ptr == NULL) || (output_img_ptr == NULL))
  //{
  //  MSG_LOW("ipl_image_add marker_201\n");
  //  return IPL_FAILURE;
  //}

  //if (input_frame_ptr == NULL) 
  //{
  //  MSG_LOW("ipl_image_add marker_202\n");
  //  return IPL_FAILURE;
  //}

  frameImgPtr = input_frame_ptr->imgPtr;

  if (frameImgPtr == NULL) 
  {
    MSG_LOW("ipl_image_add marker_203\n");
    return IPL_FAILURE;
  }
  
  if ((i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) && 
      (i_frame_ptr->cFormat == IPL_YCrCb420_LINE_PK) &&
      (o_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)) 
  {
    MSG_LOW("ipl_image_add marker_100\n");
    return (ipl_add_frame_ycrcb420lp(i_img_ptr, i_frame_ptr, 
                                     o_img_ptr, (uint8) transparentValue));                
  }

  if ((i_img_ptr->cFormat == IPL_YCbCr444) &&
      (i_frame_ptr->cFormat == IPL_YCbCr444) &&
      (o_img_ptr->cFormat == IPL_YCbCr444)) 
  {
    MSG_LOW("ipl_image_add marker_101\n");
    return (ipl_add_frame_ycbcr444(i_img_ptr, i_frame_ptr, 
                                   o_img_ptr, (uint8) transparentValue));
  }

  idx = input_img_ptr->dx;
  fdx = input_frame_ptr->dx;

  if ((output_img_ptr->dx != input_img_ptr->dx))
  {
    MSG_LOW("ipl_image_add marker_204\n");
    return IPL_FAILURE;
  }
  if ((input_frame_ptr->cFormat != IPL_RGB565))
  {
    MSG_LOW("ipl_image_add marker_205\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_image_add marker_1\n");

  if (input_img_ptr->cFormat == IPL_YCbCr)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr)
    {
      convert = FALSE;
    } 
    else if (output_img_ptr->cFormat == IPL_RGB565)
    {
      convert = TRUE;
    } 
    else 
    {
      MSG_LOW("ipl_image_add marker_206\n");
      return IPL_FAILURE;
    }

    /* Now loop through the image once */
    for(row = 0; row < output_img_ptr->dy; row++)
    {
      for(col = 0; col < (output_img_ptr->dx); col=col+2)
      {
        index = (col + row*output_img_ptr->dx)<<1;  /* byte addressed */

        if (col < fdx) 
        {
          if (!(*(uint16*)(frameImgPtr) == (uint16)transparentValue))
          {
            /* first pixel */
            out = *(uint16*) (frameImgPtr);
            unpack_rgb565(out,&r1,&g1,&b1);
            luma1 = (rgb2ycbcr_convert[0]*r1 + rgb2ycbcr_convert[1]*g1 + 
                     rgb2ycbcr_convert[2]*b1)*4+0x8000;
            luma1 = (luma1>>16) + 16;
            luma1= CLIPIT(luma1);
            cb = (rgb2ycbcr_convert[3]*r1 + rgb2ycbcr_convert[4]*g1 + 
                  rgb2ycbcr_convert[5]*b1)*4+0x8000;
            cb = (cb>>16) + 128;
            cb = CLIPIT(cb);

            /* 2nd pixel */
            out = *((uint16*)(frameImgPtr + 2));
            unpack_rgb565(out,&r2,&g2,&b2);
            luma2 = (rgb2ycbcr_convert[0]*r2 + rgb2ycbcr_convert[1]*g2 + 
                     rgb2ycbcr_convert[2]*b2)*4+0x8000;
            luma2 = (luma2>>16) + 16;
            luma2 = CLIPIT(luma2);
            cr = (rgb2ycbcr_convert[6]*r2 + rgb2ycbcr_convert[7]*g2 + 
                  rgb2ycbcr_convert[8]*b2)*4+0x8000;
            cr = (cr>>16) + 128;
            cr = CLIPIT(cr);
          }
          else
          {
            cb=inputImgPtr[index];
            luma1 = inputImgPtr[index+1]; /* Byte addressed */
            cr = inputImgPtr[index+2];
            luma2 = inputImgPtr[index+3];
          }
          frameImgPtr += 4;
        }
        else
        {
          cb=inputImgPtr[index];
          luma1 =  inputImgPtr[index+1]; /* Byte addressed */
          cr = inputImgPtr[index+2];
          luma2 = inputImgPtr[index+3];
        }

        if (convert)
        {
          /* Convert input to RGB and write output */
          rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                 + 0x8000;
          gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4
                 + 0x8000;
          bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                 + 0x8000;
          r = luma1 + (rc>>16);
          g = luma1 + (gc>>16);
          b = luma1 + (bc>>16);
          r=CLIPIT(r);
          g=CLIPIT(g);
          b=CLIPIT(b);
          /*
          ** Masking bits for 5 ==> 0xF8 and 6 ==> 0xFC
          ** Order of the 2 bytes is R5G3 G3B5
          */
          out = pack_rgb565(r,g,b);
          *((unsigned short*)(outputImgPtr+index)) = (unsigned short)out;

          r = luma2 + (rc>>16);
          g = luma2 + (gc>>16);
          b = luma2 + (bc>>16);
          r=CLIPIT(r);
          g=CLIPIT(g);
          b=CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((unsigned short*)(outputImgPtr+index+2)) = (unsigned short)out;
        } 
        else 
        {
          outputImgPtr[index] =   (unsigned char)cb;
          outputImgPtr[index+1] = (unsigned char)luma1;
          outputImgPtr[index+2] = (unsigned char)cr;
          outputImgPtr[index+3] = (unsigned char)luma2;
        }
      } /* End of col loop */
    } /* End of row loop */

    MSG_LOW("ipl_image_add marker_102\n");
    return IPL_SUCCESS;
  } 
  else if (input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
      convert = FALSE;
    else if (output_img_ptr->cFormat == IPL_RGB565)
      convert = TRUE;
    else 
    {
      MSG_LOW("ipl_image_add marker_207\n");
      return IPL_FAILURE;
    }
    /* Now loop through the image once */
    // two lines at a time
    for(row = 0; row < output_img_ptr->dy; row += 2)
    {
      // two pixels at a time
      for(col = 0; col < (output_img_ptr->dx); col += 2)
      {
        index = (col + row*output_img_ptr->dx);
        cindex = (col + (row/2)*output_img_ptr->dx);

        if (col < fdx) 
        {
          if (!(*(uint16*)(frameImgPtr) == (uint16)transparentValue))
          {
            /* first pixel */
            out = *(uint16*) (frameImgPtr);
            unpack_rgb565(out,&r1,&g1,&b1);
            luma1 = (rgb2ycbcr_convert[0]*r1 + rgb2ycbcr_convert[1]*g1 + 
                     rgb2ycbcr_convert[2]*b1)*4+0x8000;
            luma1 = (luma1>>16) + 16;
            luma1= CLIPIT(luma1);
            cb = (rgb2ycbcr_convert[3]*r1 + rgb2ycbcr_convert[4]*g1 + 
                  rgb2ycbcr_convert[5]*b1)*4+0x8000;
            cb = (cb>>16) + 128;
            cb = CLIPIT(cb);

            /* 2nd pixel */
            out = *((uint16*)(frameImgPtr + 2));
            unpack_rgb565(out,&r2,&g2,&b2);
            luma2 = (rgb2ycbcr_convert[0]*r2 + rgb2ycbcr_convert[1]*g2 + 
                     rgb2ycbcr_convert[2]*b2)*4+0x8000;
            luma2 = (luma2>>16) + 16;
            luma2 = CLIPIT(luma2);
            cr = (rgb2ycbcr_convert[6]*r2 + rgb2ycbcr_convert[7]*g2 + 
                  rgb2ycbcr_convert[8]*b2)*4+0x8000;
            cr = (cr>>16) + 128;
            cr = CLIPIT(cr);

            /* third pixel */
            out = *((uint16*)(frameImgPtr + fdx * 2));
            unpack_rgb565(out,&r2,&g2,&b2);
            luma3 = (rgb2ycbcr_convert[0]*r2 + rgb2ycbcr_convert[1]*g2 + 
                     rgb2ycbcr_convert[2]*b2)*4+0x8000;
            luma3 = (luma3>>16) + 16;
            luma3 = CLIPIT(luma3);
            // ignore this guy's chroma

            /* fourth pixel */
            out = *((uint16*)(frameImgPtr + fdx * 2 + 2));
            unpack_rgb565(out,&r2,&g2,&b2);
            luma4 = (rgb2ycbcr_convert[0]*r2 + rgb2ycbcr_convert[1]*g2 + 
                     rgb2ycbcr_convert[2]*b2)*4+0x8000;
            luma4 = (luma4>>16) + 16;
            luma4 = CLIPIT(luma4);
            // ignore this guy's chroma
          }
          else
          {
            luma1 = inputImgPtr[index]; /* Byte addressed */
            luma2 = inputImgPtr[index+1];
            luma3 = inputImgPtr[index+idx];
            luma4 = inputImgPtr[index+idx+1];
            cb = inputClrPtr[cindex];
            cr = inputClrPtr[cindex+1];
          }
          frameImgPtr += 4;
        }
        else
        {
          luma1 = inputImgPtr[index]; /* Byte addressed */
          luma2 = inputImgPtr[index+1];
          luma3 = inputImgPtr[index+idx];
          luma4 = inputImgPtr[index+idx+1];
          cb = inputClrPtr[cindex];
          cr = inputClrPtr[cindex+1];
        }

        if (convert)
        {
          /* Convert input to RGB and write output */
          rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4
                 + 0x8000;
          gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4
                 + 0x8000;
          bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4
                 + 0x8000;
          r = luma1 + (rc>>16);
          g = luma1 + (gc>>16);
          b = luma1 + (bc>>16);
          r=CLIPIT(r);
          g=CLIPIT(g);
          b=CLIPIT(b);
          /*
          ** Masking bits for 5 ==> 0xF8 and 6 ==> 0xFC
          ** Order of the 2 bytes is R5G3 G3B5
          */
          out = pack_rgb565(r,g,b);
          *((unsigned short*)(outputImgPtr+index*2)) = (unsigned short)out;

          r = luma2 + (rc>>16);
          g = luma2 + (gc>>16);
          b = luma2 + (bc>>16);
          r=CLIPIT(r);
          g=CLIPIT(g);
          b=CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((unsigned short*)(outputImgPtr+index*2+2)) = (unsigned short)out;

          r = luma3 + (rc>>16);
          g = luma3 + (gc>>16);
          b = luma3 + (bc>>16);
          r=CLIPIT(r);
          g=CLIPIT(g);
          b=CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((unsigned short*)(outputImgPtr+index*2+idx*2)) = (unsigned short)out;

          r = luma4 + (rc>>16);
          g = luma4 + (gc>>16);
          b = luma4 + (bc>>16);
          r=CLIPIT(r);
          g=CLIPIT(g);
          b=CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((unsigned short*)(outputImgPtr+index*2+idx*2+2)) = (unsigned short)out;
        } 
        else 
        {
          output_img_ptr->imgPtr[index] =       (unsigned char) luma1;
          output_img_ptr->imgPtr[index+1] =     (unsigned char) luma2;
          output_img_ptr->imgPtr[index+idx] =   (unsigned char) luma3;
          output_img_ptr->imgPtr[index+idx+1] = (unsigned char) luma4;

          output_img_ptr->imgPtr[index] = (unsigned char)cb;
          output_img_ptr->imgPtr[index+2] = (unsigned char)cr;
        }
      } /* End of col loop */
      frameImgPtr += (fdx * 2);
    } /* End of row loop */
  } 
  else if ((input_img_ptr->cFormat == IPL_RGB565) &&
           (output_img_ptr->cFormat == IPL_RGB565))
  {
    /* Now loop through the image once */
    for(row = 0; row < output_img_ptr->dy; row++)
    {
      for(col = 0; col < (output_img_ptr->dx); col++)
      {
        index = (col + row*output_img_ptr->dx)<<1;  /* byte addressed */
        if (col < fdx) 
        {
          if (!(*(uint16*)(frameImgPtr) == (uint16)transparentValue))
            out = *(uint16*)(frameImgPtr);
          else 
            out = *(uint16*)(inputImgPtr + index);
          frameImgPtr += 2;
        }
        else
          out = *(uint16*)(inputImgPtr + index);

        *(uint16*)(outputImgPtr + index)= out;
      } 
    } 
  }
  else if ((input_img_ptr->cFormat == IPL_RGB565) &&
           (output_img_ptr->cFormat == IPL_YCbCr))
  {
    /* Input and Frame are in rgb 565 */
    if (output_img_ptr->cFormat == IPL_RGB565)
    {
      /* No need for color conversion */
      convert = FALSE;
    } 
    else if (output_img_ptr->cFormat == IPL_YCbCr)
    {
      convert = TRUE;
    } 
    else 
    {
      MSG_LOW("ipl_image_add marker_208\n");
      return IPL_FAILURE;
    }

    /* Now loop through the image once */
    for(row = 0; row < output_img_ptr->dy; row++)
    {
      for(col = 0; col < (output_img_ptr->dx); col=col+2)
      {
        index = (col + row*output_img_ptr->dx)<<1;  /* byte addressed */

        /* make sure we have frame data */
        if (col < fdx) 
        {
          if (!(*(uint16*)(frameImgPtr) == (uint16)transparentValue))
          {
            out = *(uint16*)(frameImgPtr);
            out2 = *(uint16*)(frameImgPtr+2);
          }  
          else 
          {
            out = *(uint16*)(inputImgPtr + index);
            out2 = *(uint16*)(inputImgPtr+ index + 2);
          }
          frameImgPtr += 4;
        }
        else
        {
          out = *(uint16*)(inputImgPtr + index);
          out2 = *(uint16*)(inputImgPtr + index+ 2);
        }

        if (convert)
        {
          /* Now convert out and out2 to YCbCr */
          unpack_rgb565(out,&r1,&g1,&b1);
          luma1 = (rgb2ycbcr_convert[0]*r1 + rgb2ycbcr_convert[1]*g1 + 
                   rgb2ycbcr_convert[2]*b1)*4+0x8000;
          luma1 = (luma1>>16) + 16;
          luma1= CLIPIT(luma1);
          cb = (rgb2ycbcr_convert[3]*r1 + rgb2ycbcr_convert[4]*g1 + 
                rgb2ycbcr_convert[5]*b1)*4+0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);

          /* 2nd pixel */
          unpack_rgb565(out2,&r2,&g2,&b2);
          luma2 = (rgb2ycbcr_convert[0]*r2 + rgb2ycbcr_convert[1]*g2 + 
                   rgb2ycbcr_convert[2]*b2)*4+0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr = (rgb2ycbcr_convert[6]*r2 + rgb2ycbcr_convert[7]*g2 + 
                rgb2ycbcr_convert[8]*b2)*4+0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);

          /* Write output */
          *((uint8*)(outputImgPtr + index)) = (uint8) cb;
          *((uint8*)(outputImgPtr + index + 1)) = (uint8) luma1;
          *((uint8*)(outputImgPtr + index + 2)) = (uint8) cr;
          *((uint8*)(outputImgPtr + index + 3)) = (uint8) luma2;
        } 
        else
        {
          *(uint16*)(outputImgPtr + index)= out;
          *(uint16*)(outputImgPtr + index+2)= out2;
        }
      } /* End of col loop */
    } /* End of row loop */

    MSG_LOW("ipl_image_add marker_103\n");
    return IPL_SUCCESS;
  } 
  else 
  {
    MSG_LOW("ipl_image_add marker_209\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_image_add marker_104\n");
  return IPL_SUCCESS;
} /* End ipl_image_add */




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_alpha_blend_area

DESCRIPTION
  This function adds a frame (i_frame_ptr) over an input image inplace
  (i_img_ptr). It will blend
  the image at location defined by loc by an alpha value defined by alpha
  argument  (0-128). If loc is zero, it will add the entire frame at
  location 0,0

  Currently, the input image and frame must be 420 YCrCb line pack.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr     pointer to the input image
  i_frame_ptr   pointer to the frame image
  loc           where and how much of the frame to add
  alpha         alpha value, 0 - 128. 0 = all frame, 128 = no frame

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_alpha_blend_area
(
  ipl_image_type* i_img_ptr,      /* Points to the input image */
  ipl_image_type* i_frame_ptr,    /* Points to the frame image (fun picture)  */
  ipl_rect_type*  loc,   /* where to put, and how much of the frame to add */
  uint32 alpha,          /* how much to blend two images (0-128) */
  uint16 transparentValue           /* RGB565 packed transparent color */
)
{
  register unsigned char *inImgPtr, *frameImgPtr;
  register unsigned char *inClrPtr, *frameClrPtr;
  uint32 w, x, y;
  uint32 wF;
  uint32 px, py, pdx, pdy;
  uint32 pitch;
  uint32 pitchFrame;
  uint16 beta;
  int32 tY, tCb, tCr; 
  uint8 r,g,b;

  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_alpha_blend_area marker_0\n");
  
  // make sure we have input
  if (!i_img_ptr  || !i_frame_ptr)
  {
    MSG_LOW("ipl_alpha_blend_area marker_200\n");
    return IPL_FAILURE;
  }
  /* Initialize image pointers and local variables */
  w = (uint16)i_img_ptr->dx;
  wF = (uint16)i_frame_ptr->dx;

  inImgPtr = i_img_ptr->imgPtr;
  inClrPtr = i_img_ptr->clrPtr;

  frameImgPtr = i_frame_ptr->imgPtr;
  frameClrPtr = i_frame_ptr->clrPtr;

  // if no loc, then add the whole frame
  if (loc == NULL)
  {
    px = 0;
    py = 0;
    pdx = wF;
    pdy = i_frame_ptr->dy;
  }
  else
  {
    px = loc->x;
    py = loc->y;
    pdx = loc->dx;
    pdy = loc->dy;
  }

  beta = (uint16) (128 - alpha);

  // compute the transparent Y, Cb, Cr
  unpack_rgb565(transparentValue, &r, &g, &b);

  if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
    printf("Transprent color in RGB space is %d %d %d\n", r, g, b);

  tY = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + ycbcr_convert[2]*b)*4+0x8000;
  tY = (tY >> 16) + 16;
  tY = CLIPIT(tY);

  tCb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + ycbcr_convert[5]*b)*4+0x8000;
  tCb = (tCb>>16) + 128;
  tCb = CLIPIT(tCb);

  tCr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g + ycbcr_convert[8]*b)*4+0x8000;
  tCr = (tCr>>16) + 128;
  tCr = CLIPIT(tCr);

  if (IPL_COMPOSE_DEBUG)  //lint !e506 !e774
    printf("Transprent color in YCbCr space is %ld %ld %ld\n", tY, tCb, tCr);

  MSG_LOW("ipl_alpha_blend_area marker_1\n");
  
  if ((i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (i_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    // make sure we have input
    if (!i_img_ptr->imgPtr  || !i_frame_ptr->imgPtr ||
        !i_img_ptr->clrPtr  || !i_frame_ptr->clrPtr)
    {
      MSG_LOW("ipl_alpha_blend_area marker_201\n");
      return IPL_FAILURE;
    }
    // if user want to blend an image onto top of YCrCb 420 line pack,
    // make sure everything that needs to be even is.
    if (px%2) px--;
    if (py%2) py--;
    if (pdx%2) pdx--;
    if (pdy%2) pdy--;

    // draw as much as you can if we go over
    if (px + pdx > w) 
      pdx = w - px;

    if (py + pdy > i_img_ptr->dy) 
      pdy = i_img_ptr->dy - py;

    // make sure we dont draw more than the frame can allow
    if (pdx > wF) 
      pdx = wF;

    if (pdy > i_frame_ptr->dy) 
      pdy = i_frame_ptr->dy; 



    if ((i_frame_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
        (i_frame_ptr->cFormat == IPL_YCbCr420_LINE_PK))
    {
      pitch = (w - pdx);
      pitchFrame = (i_frame_ptr->dx - pdx);

      inImgPtr += (px + py*w);
      //inClrPtr += (px + (py*w)>>1); //THIS DOESNT WORK! priority of >> is LOW
        inClrPtr += (px + (py*w)/ 2); //THIS WORKS!

      // we do 4 pixels at a time
      for(y = pdy/2; y; y--) 
      {
        for(x = pdx/2; x; x--) 
        {
          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774 
            printf("Comparing ycbcr %d %d %d to trans %ld %ld %ld\n", 
                   *frameImgPtr, *(frameClrPtr), *(frameClrPtr+1),tY,tCr,tCb);

          if ((*frameImgPtr == tY) && (*(frameClrPtr) == tCr) && 
              (*(frameClrPtr+1) == tCb))
          {
            // do not copy frame over;
            inImgPtr += 2;
            inClrPtr += 2;
            frameImgPtr += 2;
            frameClrPtr += 2;
          }
          else
          {
            // Blend frame over image
            // Do Luma
            *(inImgPtr) = (unsigned char) (((*inImgPtr * alpha)>>7) + 
                          ((*frameImgPtr * beta)>>7));
            *(inImgPtr+1) = (unsigned char) (((*(inImgPtr+1) * alpha)>>7) + 
                            ((*(frameImgPtr+1) * beta)>>7));
            *(inImgPtr+w) = (unsigned char) (((*(inImgPtr+w) * alpha)>>7) + 
                            ((*(frameImgPtr+wF) * beta)>>7));
            *(inImgPtr+w+1) = (unsigned char) (((*(inImgPtr+w+1) * alpha)>>7) + 
                              ((*(frameImgPtr+wF+1) * beta)>>7));
            inImgPtr += 2;
            frameImgPtr += 2;


            // Do Chroma 
            *inClrPtr = (unsigned char) (((*inClrPtr * alpha)>>7) + 
                       ((*frameClrPtr++ * beta)>>7));
            inClrPtr++;
            *inClrPtr = (unsigned char) (((*inClrPtr * alpha)>>7) + 
                       ((*frameClrPtr++ * beta)>>7));
            inClrPtr++;
          }
        } 

        // skip a line
        inImgPtr += (w + pitch);
        inClrPtr += (pitch);

        frameImgPtr += (wF + pitchFrame);
        frameClrPtr += (pitchFrame);
      }

    } 
    MSG_LOW("ipl_alpha_blend_area marker_100\n");
    return IPL_SUCCESS;
  }
  else
  {
    MSG_LOW("ipl_alpha_blend_area marker_202\n");
    return IPL_FAILURE;
  }
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_add_overaly_inplace 

DESCRIPTION

  This function adds a 420 line pack overlay over a 420 line pack image.
  This function should only be used when real time constraints are present,
  i.e. encoding. Otherwise, please use the other functions which require
  rgb565 overlays which produce better results. We say this because the 
  overlay most likely being passed to this function was stored as 
  rgb565 and color convert to 420lp. 

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr      pointer to the input image
  input_frame_ptr    pointer to the frame image
  loc                where and how much of the frame 2 add

                     The default color for transparent pixels is magenta,
                     which is 255, 0, 255. However, in rgb565, it is 
                     248, 0, 248. When this rgb combo is converted to y,cb,cr
                     using your ycbcr_convert table, that translates to
                     104, 200 and 219 for y,cb,cr.

  tY                 luma value of transparent color, default 104
  tCb                cb value of transparent color, default 200
  tCr                cr value of transparent color, default 219

  tTolerance         not used

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS

  PLEASE READ! It is imperative that when calling this function to overlay
  ycrcb420 line pack fun frame over ycbc420 line pack image, that the fun
  frame be converted from the native rgb565 format using ipl_conv_frame 
  function.

===========================================================================*/
API_EXTERN ipl_status_type ipl_add_overlay_inplace
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  const ipl_image_type* i_frame_ptr, /* Points to the frame image      */
  ipl_rect_type* loc,                /* where and how much of the frame 2 add*/
  int tY,                            
  int tCb,
  int tCr,
  int tTolerance
)
{
  uint32 row,col;
  uint32 idx, fdx;
  uint32 px, py, pdx, pdy;
  uint32 ipitchx, fpitchx;

  uint8* fYptr;
  uint8* fCptr;
  uint8* iYptr;
  uint8* iCptr;


  // make sure we have too input
  if (!i_frame_ptr || !i_frame_ptr->imgPtr || !i_frame_ptr->clrPtr || 
      !i_img_ptr   || !i_img_ptr->imgPtr || !i_img_ptr->clrPtr)
  {
    return IPL_FAILURE;
  }

  // this function is only for 420 line pack input, if the user calls use
  // with rgb565, call the good ol fashion one
  if (i_frame_ptr->cFormat == IPL_RGB565)
  {
    return(ipl_image_add_inplace(i_img_ptr, i_frame_ptr, loc, 0xF81F));
  }

  if (i_frame_ptr->cFormat != IPL_YCrCb420_LINE_PK)
  {
    return IPL_FAILURE;
  }

  // if user passes in bad value, then chose the YCbCr equivalent of magenta
  // as our transparent value.
  if (tY < 0 || tCb < 0 || tCr < 0)
  {
    tY = 104;
    tCb = 200;
    tCr = 219;
  }

  fYptr = i_frame_ptr->imgPtr;
  fCptr = i_frame_ptr->clrPtr;

  iYptr = i_img_ptr->imgPtr;
  iCptr = i_img_ptr->clrPtr;

  // if no loc, then add the whole frame
  if (loc == NULL)
  {
    px = 0;
    py = 0;
    pdx = i_frame_ptr->dx;
    pdy = i_frame_ptr->dy;
  }
  else
  {
    px = loc->x;
    py = loc->y;
    pdx = loc->dx;
    pdy = loc->dy;
  }
  
  idx = i_img_ptr->dx;
  fdx = i_frame_ptr->dx;

  if (px%2) px--;
  if (py%2) py--;
  if (pdx%2) pdx--;
  if (pdy%2) pdy--;

  if (px + pdx > i_img_ptr->dx)
  {
    return IPL_FAILURE;
  }

  if (py + pdy > i_img_ptr->dy)
  {
    return IPL_FAILURE;
  }


  if ((i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK))
  {
    iYptr = iYptr + px + py*idx; 
    iCptr = iCptr + px + py*idx/2; 

    ipitchx = idx - pdx;
    fpitchx = fdx - pdx;

    // two lines at a time
    for(row = 0; row < pdy; row += 2)
    {
      // two pixels at a time
      for(col = 0; col < pdx; col += 2)
      {
        if (*fCptr != tCr && *(fCptr+1) != tCb && *fYptr != tY)
        {
          *(iYptr      ) = *(fYptr);
          *(iYptr+1    ) = *(fYptr+1);
          *(iYptr+idx  ) = *(fYptr+fdx);
          *(iYptr+idx+1) = *(fYptr+fdx+1);

          *(iCptr)   = *fCptr;
          *(iCptr+1) = *(fCptr+1);
        }

        fYptr += 2;
        fCptr += 2;

        iYptr += 2;
        iCptr += 2;
      } 

      fYptr += (fdx + fpitchx); 
      fCptr += (fpitchx); 

      iYptr += (idx + ipitchx); 
      iCptr += (ipitchx); 
    }
  }
  else 
  {
    return IPL_FAILURE;
  }
  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_add_inplace

DESCRIPTION
  This function adds a frame around an image and performs color conversion 
  of the frame if needed. Frame can be smaller than input image.
  The image must be 420 YCrCb line pack.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr      pointer to the input image
  input_frame_ptr    pointer to the frame image
  loc                where and how much of the frame 2 add
  transparentPixel   16-bit transparent pixel value

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_image_add_inplace
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  const ipl_image_type* i_frame_ptr,       /* Points to the frame image      */
  ipl_rect_type*  loc,               /* where and how much of the frame 2 add*/
  uint16 transparentValue            /* Transparent pixel value        */
)
{
  int32 cindex, index,luma1;

  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  uint32 row,col;
  uint16 out;
  int32 cb,cr;
  int32 tcb,tcr; // temp variables
  uint8 r,g,b;
  uint32 odx, fdx;
  uint32 px, py, pdx, pdy;
  uint32 pitchx;
  uint8* frameImgPtr;


  uint8* outputImgPtr;
  uint8* outputClrPtr;
  ipl_image_type* input_img_ptr;
  ipl_image_type* output_img_ptr;

  MSG_LOW("ipl_image_add_inplace marker_0\n");

  // make sure we have too input
  if (!i_frame_ptr || !i_frame_ptr->imgPtr || 
      !i_img_ptr   ||  !i_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_image_add_inplace marker_200\n");
    return IPL_FAILURE;
  }

  //if (i_frame_ptr == NULL)
  //{
  //  MSG_LOW("ipl_image_add_inplace marker_201\n");
  //  return IPL_FAILURE;
  //}
  frameImgPtr = i_frame_ptr->imgPtr;

  if (frameImgPtr == NULL)
  {
    MSG_LOW("ipl_image_add_inplace marker_202\n");
    return IPL_FAILURE;
  }
    
  input_img_ptr = i_img_ptr;
  output_img_ptr = i_img_ptr;
  outputImgPtr = output_img_ptr->imgPtr;
  outputClrPtr = output_img_ptr->clrPtr;

  // if no loc, then add the whole frame
  if (loc == NULL)
  {
    px = 0;
    py = 0;
    pdx = i_frame_ptr->dx;
    pdy = i_frame_ptr->dy;
  }
  else
  {
    px = loc->x;
    py = loc->y;
    pdx = loc->dx;
    pdy = loc->dy;
  }
  
  odx = output_img_ptr->dx;
  fdx = 2*i_frame_ptr->dx;

  if (px%2) px--;
  if (py%2) py--;
  if (pdx%2) pdx--;
  if (pdy%2) pdy--;

  if (px + pdx > output_img_ptr->dx)
  {
    MSG_LOW("ipl_image_add_inplace marker_203\n");
    return IPL_FAILURE;
  }

  if (py + pdy > output_img_ptr->dy)
  {
    MSG_LOW("ipl_image_add_inplace marker_204\n");
    return IPL_FAILURE;
  }
  if (i_frame_ptr->cFormat != IPL_RGB565)
  {
    MSG_LOW("ipl_image_add_inplace marker_205\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_image_add_inplace marker_1\n");


  // THIS SWITCH DECIDES WHETHER CUSTOMER WANTS SMOOTH EDGES IN
  // COMPOSITING AT THE COST OF SOME COLOR BLEEDING AT EDGES, OR CANNOT
  // HAVE ANY COLOR BLEEDING AND IS WILLING TO HAVE SOME JAGGY EDGES
  //
  // 1 = JAGGY, NO BLEEING
  // 0 = EVERY RGB565 PIXEL TRANSPARENCY IS CONSIDERED BUT COLOR BLEEDING 
#if 0
  
  if (input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    int32 luma2, luma3, luma4;

    pitchx = 2*(i_frame_ptr->dx - pdx);
    /* Now loop through the image once */
    // two lines at a time
    for(row = 0; row < pdy; row += 2)
    {
      // two pixels at a time 
      for(col = 0; col < pdx; col += 2)
      {
        index = (col + row*output_img_ptr->dx + px + py*odx) ;
        cindex = (col + (row/2)*output_img_ptr->dx + px + py*odx/2);

        // upper left pixel in 2x2 breakup of RGB image dictates transprency
        // of frame for that region. This prevents color bleeding
       
        if (*(uint16*)(frameImgPtr) != (uint16)transparentValue)
        {
          // since the upper left corner of a 2x2 grid of pixels is
          // not transparent, i.e. frame, then for each of the 3 
          // remaintng pixels, either make it the frame color, or 
          // the same luma as the upper left corner pix, i.e. do not
          // chose luma fron underlying image with frame's CbCr which
          // would cause bleeding.
            
          /* Pixel 1 */
          out = *(uint16*) (frameImgPtr);
          unpack_rgb565(out,&r,&g,&b);

          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
          {
            printf("(%lu,%lu)\n", col, row); 
            printf("   rgb %d %d %d\n",r,g,b); 
          }

          luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
          luma1 = (luma1 >> 16) + 16;
          luma1 = CLIPIT(luma1);
          cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
                  ycbcr_convert[8]*b)*4+0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);

          cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
                  ycbcr_convert[5]*b)*4+0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);

          /* Pixel 2 */
          out = *(uint16*) (frameImgPtr+2);
          if (out != transparentValue)
          {
            unpack_rgb565(out,&r,&g,&b);
            luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                 ycbcr_convert[2]*b)*4+0x8000;
            luma2 = (luma2>>16) + 16;
            luma2 = CLIPIT(luma2);
          }
          else
            luma2 = luma1;

          /* Pixel 3 */
          out = *(uint16*) (frameImgPtr+fdx);
          if (out != transparentValue)
          {
            unpack_rgb565(out,&r,&g,&b);
            luma3 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                 ycbcr_convert[2]*b)*4+0x8000;
            luma3 = (luma3 >> 16) + 16;
            luma3 = CLIPIT(luma3);
          }
          else
            luma3 = luma1;

          /* Pixel 4 */
          out = *(uint16*) (frameImgPtr+fdx+2);
          if (out != transparentValue)
          {
            unpack_rgb565(out,&r,&g,&b);
            luma4 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                ycbcr_convert[2]*b)*4+0x8000;
            luma4 = (luma4>>16) + 16;
            luma4 = CLIPIT(luma4);
          }
          else
            luma4 = luma1;

          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
          {
            printf("   luma %lu %lu crcb %lu %lu\n",luma1, luma2, cr, cb); 
            printf("   luma %lu %lu\n", luma3, luma4); 
          }


          outputImgPtr[index] =       (unsigned char) luma1;
          outputImgPtr[index+1] =     (unsigned char) luma2;
          outputImgPtr[index+odx] =   (unsigned char) luma3;
          outputImgPtr[index+odx+1] = (unsigned char) luma4;

          outputClrPtr[cindex] = (unsigned char)cr;
          outputClrPtr[cindex+1] = (unsigned char)cb;
        }

        frameImgPtr += 4; // we do 2x2 at a time
      } 
      frameImgPtr += (fdx + pitchx); 
    }

#else


  if (input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    pitchx = 2*(i_frame_ptr->dx - pdx);

    /* Now loop through the image once */
    // two lines at a time
    for(row = 0; row < pdy; row += 2)
    {
      // two pixels at a time 
      for(col = 0; col < pdx; col += 2)
      {
        index = (col + row*output_img_ptr->dx + px + py*odx) ;
        cindex = (col + (row/2)*output_img_ptr->dx + px + py*odx/2);

        out = *(uint16*) (frameImgPtr);
        if (out != (uint16)transparentValue)
        {
          /* Pixel 1 */
          unpack_rgb565(out,&r,&g,&b);
          luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
          luma1 = (luma1 >> 16) + 16;
          luma1 = CLIPIT(luma1);


          cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
                  ycbcr_convert[8]*b)*4+0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);

          cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
                  ycbcr_convert[5]*b)*4+0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);

          // output
          outputImgPtr[index] =       (unsigned char) luma1;
        }
        else
        {
          cr = outputClrPtr[cindex];
          cb = outputClrPtr[cindex+1];
        }

        out = *(uint16*) (frameImgPtr+2);
        if (out != transparentValue)
        {
          /* Pixel 2 */
          unpack_rgb565(out,&r,&g,&b);
          luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                 ycbcr_convert[2]*b)*4+0x8000;
          luma1 = (luma1>>16) + 16;
          luma1 = CLIPIT(luma1);

          tcr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
                 ycbcr_convert[8]*b)*4+0x8000;
          tcr = (tcr>>16) + 128;
          cr += CLIPIT(tcr);

          tcb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
                 ycbcr_convert[5]*b)*4+0x8000;
          tcb = (tcb>>16) + 128;
          cb += CLIPIT(tcb);

          // output
          outputImgPtr[index+1] =     (unsigned char) luma1;
        }
        else
        {
          cr += outputClrPtr[cindex];
          cb += outputClrPtr[cindex+1];
        }


        out = *(uint16*) (frameImgPtr+fdx);
        if (out != transparentValue)
        {
          unpack_rgb565(out,&r,&g,&b);
          luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                 ycbcr_convert[2]*b)*4+0x8000;
          luma1 = (luma1 >> 16) + 16;
          luma1 = CLIPIT(luma1);

          tcr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
                 ycbcr_convert[8]*b)*4+0x8000;
          tcr = (tcr>>16) + 128;
          cr += CLIPIT(tcr);

          tcb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
                 ycbcr_convert[5]*b)*4+0x8000;
          tcb = (tcb>>16) + 128;
          cb += CLIPIT(tcb);

          // output
          outputImgPtr[index+odx] =   (unsigned char) luma1;
        }
        else
        {
          cr += outputClrPtr[cindex];
          cb += outputClrPtr[cindex+1];
        }

        out = *(uint16*) (frameImgPtr+fdx+2);
        if (out != transparentValue)
        {
          unpack_rgb565(out,&r,&g,&b);
          luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                ycbcr_convert[2]*b)*4+0x8000;
          luma1 = (luma1>>16) + 16;
          luma1 = CLIPIT(luma1);

          tcr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
                 ycbcr_convert[8]*b)*4+0x8000;
          tcr = (tcr>>16) + 128;
          cr += CLIPIT(tcr);

          tcb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
                 ycbcr_convert[5]*b)*4+0x8000;
          tcb = (tcb>>16) + 128;
          cb += CLIPIT(tcb);

          outputImgPtr[index+odx+1] = (unsigned char) luma1;
        }
        else
        {
          cr += outputClrPtr[cindex];
          cb += outputClrPtr[cindex+1];
        }

        outputClrPtr[cindex] = (unsigned char) (cr/4);
        outputClrPtr[cindex+1] = (unsigned char) (cb/4);


        frameImgPtr += 4; // we do 2x2 at a time
      } 
      frameImgPtr += (fdx + pitchx); 
    }
  }
#endif
  else 
  {
    MSG_LOW("ipl_image_add_inplace marker_206\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_image_add_inplace marker_100\n");
  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_frame_compose

DESCRIPTION
  This function iterates through a list of icons and composes a frame
  from the list. The list should be terminated by a NULL.

DEPENDENCIES
  None

ARGUMENTS IN
  input_canvas_ptr    pointer to the canvas on which the icons are composed
  icon_list_ptr       pointer to a list of icons ptrs terminated by a NULL
  transparentValue    16-bit transparent pixel value

ARGUMENT OUT
  output_canvas_ptr   pointer to the output image

RETURN VALUE
  IPL_SUCCESS         indicates operation was successful
  IPL_FAILURE         otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_frame_compose
(
  ipl_image_type* input_canvas_ptr,  /* Points to input canvas image        */
  ipl_icon_type** icon_list_ptr,     /* Points to NULL-terminated icon list */
  uint16 transparentValue,           /* Transparent pixel value             */
  ipl_image_type* output_canvas_ptr  /* Points to output canvas image       */
)
{
  register int32 iter = 0;
  uint32 row,col;
  register uint16* f_ptr;
  register int32 pitch = 0;
  register uint16* o_ptr;

  MSG_LOW("ipl_frame_compose marker_0\n");

  if (!input_canvas_ptr  || !input_canvas_ptr->imgPtr ||
      !output_canvas_ptr || !output_canvas_ptr->imgPtr || 
      !icon_list_ptr)
  {
    MSG_LOW("ipl_frame_compose marker_200\n");
    return IPL_FAILURE;
  }

  o_ptr = (uint16*)output_canvas_ptr->imgPtr;

  if ((input_canvas_ptr->cFormat != IPL_RGB565) ||
      (output_canvas_ptr->cFormat != IPL_RGB565))
  {
    MSG_LOW("ipl_frame_compose marker_201\n");
    /* Only RGB565 currently supported */
    return IPL_FAILURE;
  }
  if ((input_canvas_ptr->dx != output_canvas_ptr->dx) ||
      (input_canvas_ptr->dy != output_canvas_ptr->dy))
  {
    MSG_LOW("ipl_frame_compose marker_202\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_frame_compose marker_1\n");

  /*
  ** First copy over the input canvas to output canvas
  */
  if (input_canvas_ptr->imgPtr != NULL)
  {
    memcpy(output_canvas_ptr->imgPtr,input_canvas_ptr->imgPtr,
           input_canvas_ptr->dx*input_canvas_ptr->dy*2);
  }

  /*
  ** Now iterate over the various icons
  */
  while ((iter<100) && icon_list_ptr[iter] != NULL)
  {
    if (icon_list_ptr[iter]->cFormat != IPL_RGB565)
    {
      MSG_LOW("ipl_frame_compose marker_203\n");
      return IPL_FAILURE;
    }
    o_ptr = (uint16*)output_canvas_ptr->imgPtr;
    f_ptr = (uint16*)icon_list_ptr[iter]->imgPtr;

    o_ptr += (icon_list_ptr[iter]->x + 
              icon_list_ptr[iter]->y*output_canvas_ptr->dx);

    pitch = output_canvas_ptr->dx - icon_list_ptr[iter]->dx;
    for (row=0;row < icon_list_ptr[iter]->dy;row++)
    {
      for (col=0;col<icon_list_ptr[iter]->dx;col++)
      {
        if (*f_ptr != transparentValue)
        {
          *o_ptr = *f_ptr;
        }
        o_ptr++;
        f_ptr++;
      }
      o_ptr += pitch;
    }
    iter++;
  }

  MSG_LOW("ipl_frame_compose marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_frame_compose */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_fill_rect_rgb565

DESCRIPTION
  This function fills a rectangular region of an image with a fillerPixel.
  IF OUTPUT IS NULL, OPERATION HAPPENS ON INPUT IMAGE.
  
  Input and output images must be in RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr   pointer to the image to fill with fillerPixel
  out_img_ptr  pointer to the output. If NULL, function operates on in_img_ptr
  fill_rect    pointer to a structure indicating the fill_rect region
  fillerPixel  fill color

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_fill_rect_rgb565
(
  ipl_image_type* in_img_ptr,    /* Points to input image  */
  ipl_image_type* out_img_ptr,   /* Points to output image */
  ipl_rect_type* fill_rect,      /* Overlap region         */
  uint32 fillerPixel             /* color to fill region with */
)
{
  uint16 *inImgPtr, *outImgPtr;
  uint32 row, col, inc;


  if (!in_img_ptr || !in_img_ptr->imgPtr || !fill_rect)
  {
    return IPL_FAILURE;
  }


  /* Initialize image pointers and local variables */
  if (out_img_ptr == NULL)
  {
    outImgPtr = (uint16*) in_img_ptr->imgPtr;
  }
  else
  {
    outImgPtr = (uint16*) out_img_ptr->imgPtr;

    /* If we have output image, then copy src to destination */
    /* memset may be faster, test it out */
    inImgPtr = (uint16*) in_img_ptr->imgPtr;

    for(row = 0; row < in_img_ptr->dy; row++)
    {
      for(col = 0; col < in_img_ptr->dx; col++)
      {
        *outImgPtr++ = *inImgPtr++;
      }
    } 

    /* set the pointer back to the beginning */
    outImgPtr = (uint16*) out_img_ptr->imgPtr;
  }

  /* Initialize the index to starting position */
  outImgPtr += fill_rect->x + in_img_ptr->dx * fill_rect->y; 
  inc = in_img_ptr->dx - fill_rect->dx;

  /* Loop through the downsized 2nd input image once */
  for(row = 0; row < fill_rect->dy; row++)
  {
    for(col = 0; col < fill_rect->dx; col++)
      *outImgPtr++ = (uint16) fillerPixel;
    outImgPtr += inc;
  } 

  return IPL_SUCCESS;
} /* End ipl_fill_rect_rgb565 */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_fill_rect_hsv

DESCRIPTION
  This function fills a rectangular region of an image with a fillerPixel.
  IF OUTPUT IS NULL, OPERATION HAPPENS ON INPUT IMAGE.
  
  Input and output images must be in HSV format

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr   pointer to image to fill region of 
  out_img_ptr  pointer to the output. If NULL, function operates on in_img_ptr
  fill_rect    pointer to a structure indicating the fill_rect region
  fillerPixel  color

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_fill_rect_hsv
(
  ipl_image_type* in_img_ptr,    /* Points to the first input image  */
  ipl_image_type* out_img_ptr,   /* Points to the output image       */
  ipl_rect_type* fill_rect,      /* Overlap region                   */
  uint32 fillerPixel             /* color to fill region with        */
)
{
  unsigned char *outImgPtr; 

  uint32 row, col, inc;
  uint32 fx, fy, fdx, fdy, idx;

  unsigned char h,s,v;


  if (!in_img_ptr || !in_img_ptr->imgPtr || !fill_rect)
  {
    return IPL_FAILURE;
  }
  /* make sure that the cut region's upper left is on an even (Cb) pixel */
  fx = fill_rect->x;
  fy = fill_rect->y;
  fdx = fill_rect->dx;
  fdy = fill_rect->dy;
  
  unpack_ycbcr(fillerPixel, &h, &s, &v);
  idx = in_img_ptr->dx;


  if (in_img_ptr->cFormat == IPL_HSV) 
  {
    /* setup output pointer again */
    if (out_img_ptr == NULL)
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
    else
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;

    /* Reinitialize the index to starting position */
    outImgPtr += (fx + idx * fy) * 3;
    inc = (idx - fdx) * 3;

    for(row = fdy; row; row--)
    {
      for(col = fdx; col; col--)
      {
        /* Set output bytes */
        *outImgPtr++ = h;
        *outImgPtr++ = s;
        *outImgPtr++ = v;
      } 
      outImgPtr += inc;
    } 
  }
  else
  {
    return IPL_FAILURE;
  }

  return IPL_SUCCESS;
} /* End ipl_fill_rect_ycbcr */




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_fill_rect_ycbcr

DESCRIPTION
  This function fills a rectangular region of an image with a fillerPixel.
  IF OUTPUT IS NULL, OPERATION HAPPENS ON INPUT IMAGE.
  
  Input and output images must be in YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr   pointer to image to fill region of 
  out_img_ptr  pointer to the output. If NULL, function operates on in_img_ptr
  fill_rect    pointer to a structure indicating the fill_rect region
  fillerPixel  color

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_fill_rect_ycbcr
(
  ipl_image_type* in_img_ptr,    /* Points to the first input image  */
  ipl_image_type* out_img_ptr,   /* Points to the output image       */
  ipl_rect_type* fill_rect,      /* Overlap region                   */
  uint32 fillerPixel             /* color to fill region with        */
)
{
  unsigned char *outImgPtr2, *outImgPtr, *outClrPtr; 

  uint32 row, col, inc;
  uint32 fx, fy, fdx, fdy, idx;

  unsigned char y,cb,cr;


  if (!in_img_ptr || !in_img_ptr->imgPtr || !fill_rect)
  {
    return IPL_FAILURE;
  }
  /* make sure that the cut region's upper left is on an even (Cb) pixel */
  fx = fill_rect->x;
  fy = fill_rect->y;
  fdx = fill_rect->dx;
  fdy = fill_rect->dy;
  
  unpack_ycbcr(fillerPixel, &y, &cb, &cr);

  idx = in_img_ptr->dx;

  if (fx%2) fx--;
  if (fdx%2) fdx--;
  

  if (in_img_ptr->cFormat == IPL_YCbCr) 
  {
    /* setup output pointer again */
    if (out_img_ptr == NULL)
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
    else
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;

    /* Reinitialize the index to starting position */
    outImgPtr += (fx + idx * fy) << 1;
    inc = (idx - fdx) << 1;

    for(row = fill_rect->dy; row; row--)
    {
      for(col = fdx; col; col -= 2)
      {
        /* Set output bytes */
        *outImgPtr++ = cb;
        *outImgPtr++ = y;
        *outImgPtr++ = cr;
        *outImgPtr++ = y;
      } 
      outImgPtr += inc;
    } 
  }
  else if ((in_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) ||
           (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK))
  {
    /* setup output pointer again */
    if (out_img_ptr == NULL)
    {
      outImgPtr = in_img_ptr->imgPtr;
      outImgPtr2 = in_img_ptr->imgPtr + idx;
      outClrPtr = in_img_ptr->clrPtr;
    }
    else
    {
      outImgPtr = out_img_ptr->imgPtr;
      outImgPtr2 = out_img_ptr->imgPtr + idx;
      outClrPtr = out_img_ptr->clrPtr;
    }

    /* go to upper left of where we cut and where we are going to paste */
    outImgPtr += (fy * idx + fx);
    outImgPtr2 += (fy * idx + fx);
    outClrPtr += ((fy/2)*(idx) + 2*(fx/2));

    inc = (idx - fdx);

    /* do two rows at a time */
    for (row = fdy/2; row; row--) 
    {
      /* do two pixels at a time */
      for (col = fdx/2; col; col--) 
      {
        /* copy 4 lumas, in the following order
        *
        * Y1 Y3
        * Y2 Y4
        *
        * */
        /* 1st column */
        *outImgPtr++ = y;
        *outImgPtr2++ = y;

        /* 2nd column */
        *outImgPtr++ = y;
        *outImgPtr2++ = y;

        /* copy Cb and Cr 
        *
        * Cb Cr
        *
        */
        *outClrPtr++ = cb;
        *outClrPtr++ = cr;
      } 

      /* each pointer goes 2 lines down */
      outImgPtr += (inc + idx);
      outImgPtr2 += (inc + idx);

      /* goes to correct position in next line */
      outClrPtr += (inc);
    } 
  }
  else
  {
    return IPL_FAILURE;
  }

  return IPL_SUCCESS;
} /* End ipl_fill_rect_ycbcr */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_draw_box_rgb888

DESCRIPTION
  This function draws a rectangular region of an image with a fillerPixel.
  IF OUTPUT IS NULL, OPERATION HAPPENS ON INPUT IMAGE.  
  Input and output images must be in HSV format

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr   pointer to image to fill region of 
  out_img_ptr  pointer to the output. If NULL, function operates on in_img_ptr
  fill_rect    pointer to a structure indicating the fill_rect region
  fillerPixel  color

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_draw_box_rgb888
(
  ipl_image_type* in_img_ptr,    /* Points to the first input image  */
  ipl_image_type* out_img_ptr,   /* Points to the output image       */
  ipl_rect_type* fill_rect,      /* Overlap region                   */
  uint32 fillerPixel             /* color to fill region with        */
)
{
  unsigned char *outImgPtr2, *outImgPtr; 

  uint32 i;
  uint32 fx, fy, fdx, fdy, idx;

  uint8 r,g,b;

 
  if (!in_img_ptr || !in_img_ptr->imgPtr || !fill_rect)
  {
    return IPL_FAILURE;
  }
  /* make sure that the cut region's upper left is on an even (Cb) pixel */
  fx = fill_rect->x;
  fy = fill_rect->y;
  fdx = fill_rect->dx;
  fdy = fill_rect->dy;
  
  unpack_rgb565(fillerPixel, &r, &g, &b);

  idx = in_img_ptr->dx;


  if (in_img_ptr->cFormat == IPL_HSV || in_img_ptr->cFormat == IPL_RGB888) 
  {
    // do horizontal lines
    /* setup output pointer again */
    if (out_img_ptr == NULL)
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
    else
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;

    /* Reinitialize the index to starting position */
    outImgPtr  += (3*(fx + idx * fy));
    outImgPtr2 = outImgPtr + (3*(fdy * idx));

    for(i = fdx; i; i--)
    {
      /* Set output bytes */
      *outImgPtr++ = r;
      *outImgPtr++ = g;
      *outImgPtr++ = b;

      *outImgPtr2++ = r;
      *outImgPtr2++ = g;
      *outImgPtr2++ = b;
    } 


    // do vertical lines 
    /* setup output pointer again */
    if (out_img_ptr == NULL)
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
    else
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;

    outImgPtr  += (3*(fx + idx * fy));
    outImgPtr2 = outImgPtr + (3* fdx);
    for(i = fdy; i; i--)
    {
      /* Set output bytes */
      *(outImgPtr)   = r;
      *(outImgPtr+1) = g;
      *(outImgPtr+2) = b;

      /* Set output bytes */
      *(outImgPtr2)   = r;
      *(outImgPtr2+1) = g;
      *(outImgPtr2+2) = b;

      outImgPtr += (3*in_img_ptr->dx);
      outImgPtr2 += (3*in_img_ptr->dx);
    }
  }
  else
  {
    return IPL_FAILURE;
  }

  return IPL_SUCCESS;
} /* End ipl_fill_rect_ycbcr */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_draw_box_hsv

DESCRIPTION
  This function draws a rectangular region of an image with a fillerPixel.
  IF OUTPUT IS NULL, OPERATION HAPPENS ON INPUT IMAGE.  
  Input and output images must be in HSV format

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr   pointer to image to fill region of 
  out_img_ptr  pointer to the output. If NULL, function operates on in_img_ptr
  fill_rect    pointer to a structure indicating the fill_rect region
  fillerPixel  color

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_draw_box_hsv
(
  ipl_image_type* in_img_ptr,    /* Points to the first input image  */
  ipl_image_type* out_img_ptr,   /* Points to the output image       */
  ipl_rect_type* fill_rect,      /* Overlap region                   */
  uint32 fillerPixel             /* color to fill region with        */
)
{
  unsigned char *outImgPtr2, *outImgPtr; 

  uint32 i;
  uint32 fx, fy, fdx, fdy, idx;

  unsigned char h,s,v;

 
  if (!in_img_ptr || !in_img_ptr->imgPtr || !fill_rect)
  {
    return IPL_FAILURE;
  }
  /* make sure that the cut region's upper left is on an even (Cb) pixel */
  fx = fill_rect->x;
  fy = fill_rect->y;
  fdx = fill_rect->dx;
  fdy = fill_rect->dy;
  
  unpack_ycbcr(fillerPixel, &h, &s, &v);

  idx = in_img_ptr->dx;


  if (in_img_ptr->cFormat == IPL_HSV || in_img_ptr->cFormat == IPL_RGB888) 
  {
    // do horizontal lines
    /* setup output pointer again */
    if (out_img_ptr == NULL)
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
    else
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;

    /* Reinitialize the index to starting position */
    outImgPtr  += (3*(fx + idx * fy));
    outImgPtr2 = outImgPtr + (3*(fdy * idx));

    for(i = fdx; i; i--)
    {
      /* Set output bytes */
      *outImgPtr++ = h;
      *outImgPtr++ = s;
      *outImgPtr++ = v;

      *outImgPtr2++ = h;
      *outImgPtr2++ = s;
      *outImgPtr2++ = v;
    } 


    // do vertical lines 
    /* setup output pointer again */
    if (out_img_ptr == NULL)
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
    else
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;

    outImgPtr  += (3*(fx + idx * fy));
    outImgPtr2 = outImgPtr + (3* fdx);
    for(i = fdy; i; i--)
    {
      /* Set output bytes */
      *(outImgPtr)   = h;
      *(outImgPtr+1) = s;
      *(outImgPtr+2) = v;

      /* Set output bytes */
      *(outImgPtr2)   = h;
      *(outImgPtr2+1) = s;
      *(outImgPtr2+2) = v;

      outImgPtr += (3*in_img_ptr->dx);
      outImgPtr2 += (3*in_img_ptr->dx);
    }
  }
  else
  {
    return IPL_FAILURE;
  }

  return IPL_SUCCESS;
} /* End ipl_fill_rect_ycbcr */




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_draw_box_ycbcr

DESCRIPTION
  This function draws a rectangular region of an image with a fillerPixel.
  IF OUTPUT IS NULL, OPERATION HAPPENS ON INPUT IMAGE.
  
  Input and output images must be in YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr   pointer to image to fill region of 
  out_img_ptr  pointer to the output. If NULL, function operates on in_img_ptr
  fill_rect    pointer to a structure indicating the fill_rect region
  fillerPixel  color

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_draw_box_ycbcr
(
  ipl_image_type* in_img_ptr,    /* Points to the first input image  */
  ipl_image_type* out_img_ptr,   /* Points to the output image       */
  ipl_rect_type* fill_rect,      /* Overlap region                   */
  uint32 fillerPixel             /* color to fill region with        */
)
{
  unsigned char *outImgPtr2, *outImgPtr; 
  unsigned char *outClrPtr2, *outClrPtr; 

  uint32 i;
  uint32 fx, fy, fdx, fdy, idx;
  int off;

  unsigned char y,cb,cr;


  if (!in_img_ptr || !in_img_ptr->imgPtr || !fill_rect)
  {
    return IPL_FAILURE;
  }
  /* make sure that the cut region's upper left is on an even (Cb) pixel */
  fx = fill_rect->x;
  fy = fill_rect->y;
  fdx = fill_rect->dx;
  fdy = fill_rect->dy;
  
  unpack_ycbcr(fillerPixel, &y, &cb, &cr);

  idx = in_img_ptr->dx;

  if (fx%2) fx--;
  if (fdx%2) fdx--;
  
  if ((fx + fdx >= in_img_ptr->dx) ||
      (fy + fdy >= in_img_ptr->dy))
  {
    return IPL_FAILURE;
  }
  

  if (in_img_ptr->cFormat == IPL_YCbCr) 
  {
    // do horizontal lines
    /* setup output pointer again */
    if (out_img_ptr == NULL)
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
    else
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;

    /* Reinitialize the index to starting position */
    outImgPtr  += (2*(fx + idx * fy));
    outImgPtr2 = outImgPtr + (2*(fdy * idx));

    for(i = fdx; i; i -= 2)
    {
      /* Set output bytes */
      *outImgPtr++ = cb;
      *outImgPtr++ = y;
      *outImgPtr++ = cr;
      *outImgPtr++ = y;

      *outImgPtr2++ = cb;
      *outImgPtr2++ = y;
      *outImgPtr2++ = cr;
      *outImgPtr2++ = y;
    } 


    // do vertical lines 
    /* setup output pointer again */
    if (out_img_ptr == NULL)
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
    else
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;

    outImgPtr  += (2*(fx + idx * fy));
    outImgPtr2 = outImgPtr + (2* fdx);
    for(i = fill_rect->dy; i; i--)
    {
      /* Set output bytes */
      *(outImgPtr)   = cb;
      *(outImgPtr+1) = y;
      *(outImgPtr+2) = cr;
      outImgPtr += (2*idx);

      /* Set output bytes */
      *(outImgPtr2)   = cb;
      *(outImgPtr2+1) = y;
      *(outImgPtr2+2) = cr;
      outImgPtr2 += (2*idx);
    }
  }
  else if (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
  {
    // do horizontal lines
    
    /* setup output pointer again */
    if (out_img_ptr == NULL)
    {
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
      outClrPtr = (unsigned char *) in_img_ptr->clrPtr;
    }
    else
    {
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;
      outClrPtr = (unsigned char *) out_img_ptr->clrPtr;
    }

    if (fy%2) fy--;
    if (fdy%2) fdy--;

    /* Reinitialize the index to starting position */
    outImgPtr  += (fx + idx * fy);
    outImgPtr2 = outImgPtr + (fdy * idx);

    outClrPtr  += (fx + idx * fy/2);
    outClrPtr2 = outClrPtr + (fdy * idx / 2);

    for(i = fdx; i; i -= 2)
    {
      /* Set output bytes */
      *(outImgPtr     ) = y;
      *(outImgPtr+1   ) = y;
      *(outImgPtr+idx  ) = y;
      *(outImgPtr+idx+1) = y;
      outImgPtr += 2;

      *(outImgPtr2     ) = y;
      *(outImgPtr2+1   ) = y;
      *(outImgPtr2+idx  ) = y;
      *(outImgPtr2+idx+1) = y;
      outImgPtr2 += 2;


      *(outClrPtr++) = cr;
      *(outClrPtr++) = cb;

      *(outClrPtr2++) = cr;
      *(outClrPtr2++) = cb;
    } 


    // do vertical lines 
    /* setup output pointer again */
    if (out_img_ptr == NULL)
    {
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
      outClrPtr = (unsigned char *) in_img_ptr->clrPtr;
    }
    else
    {
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;
      outClrPtr = (unsigned char *) out_img_ptr->clrPtr;
    }

    /* Reinitialize the index to starting position */
    outImgPtr  += (fx + idx * fy);
    outImgPtr2 = outImgPtr + fdx;

    outClrPtr  += (fx + idx * fy/2);
    outClrPtr2 = outClrPtr + fdx;

    for(i = fdy; i; i -= 2)
    {
      /* Set output bytes */
      *(outImgPtr     ) = y;
      *(outImgPtr+1   ) = y;
      *(outImgPtr+idx  ) = y;
      *(outImgPtr+idx+1) = y;
      outImgPtr += (2*idx);

      *(outImgPtr2     ) = y;
      *(outImgPtr2+1   ) = y;
      *(outImgPtr2+idx  ) = y;
      *(outImgPtr2+idx+1) = y;
      outImgPtr2 += (2*idx);


      *(outClrPtr) = cr;
      *(outClrPtr+1) = cb;
      outClrPtr += idx;

      *(outClrPtr2) = cr;
      *(outClrPtr2+1) = cb;
      outClrPtr2 += idx;
    }
  }
  else if (in_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK) 
  {
    // do horizontal lines
    
    /* setup output pointer again */
    if (out_img_ptr == NULL)
    {
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
      outClrPtr = (unsigned char *) in_img_ptr->clrPtr;
    }
    else
    {
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;
      outClrPtr = (unsigned char *) out_img_ptr->clrPtr;
    }

    if (fy%2) fy--;
    if (fdy%2) fdy--;

    /* Reinitialize the index to starting position */
    outImgPtr  += (fx + idx * fy);
    outImgPtr2 = outImgPtr + (fdy * idx);

    outClrPtr  += (fx/2 + idx * fy/4);
    outClrPtr2 = outClrPtr + (fdy * idx / 4);

    off = (in_img_ptr->dx * in_img_ptr->dy/4);

    for(i = 0; i <= fdx; i += 2)
    {
      /* Set output bytes */
      *(outImgPtr     ) = y;
      *(outImgPtr+1   ) = y;
      *(outImgPtr+idx  ) = y;
      *(outImgPtr+idx+1) = y;
      outImgPtr += 2;

      *(outImgPtr2     ) = y;
      *(outImgPtr2+1   ) = y;
      *(outImgPtr2+idx  ) = y;
      *(outImgPtr2+idx+1) = y;
      outImgPtr2 += 2;


      *(outClrPtr) = cb;
      *(outClrPtr + off) = cr;
      outClrPtr++;

      *(outClrPtr2) = cb;
      *(outClrPtr2 + off) = cr;
      outClrPtr2++;
    } 


    // do vertical lines 
    /* setup output pointer again */
    if (out_img_ptr == NULL)
    {
      outImgPtr = (unsigned char *) in_img_ptr->imgPtr;
      outClrPtr = (unsigned char *) in_img_ptr->clrPtr;
    }
    else
    {
      outImgPtr = (unsigned char *) out_img_ptr->imgPtr;
      outClrPtr = (unsigned char *) out_img_ptr->clrPtr;
    }

    /* Reinitialize the index to starting position */
    outImgPtr  += (fx + idx * fy);
    outImgPtr2 = outImgPtr + fdx;

    outClrPtr  += (fx/2 + idx * fy/4);
    outClrPtr2 = outClrPtr + fdx/2;

    for(i = fdy; i; i -= 2)
    {
      /* Set output bytes */
      *(outImgPtr     ) = y;
      *(outImgPtr+1   ) = y;
      *(outImgPtr+idx  ) = y;
      *(outImgPtr+idx+1) = y;
      outImgPtr += (2*idx);

      *(outImgPtr2     ) = y;
      *(outImgPtr2+1   ) = y;
      *(outImgPtr2+idx  ) = y;
      *(outImgPtr2+idx+1) = y;
      outImgPtr2 += (2*idx);


      *(outClrPtr) = cb;
      *(outClrPtr+off) = cr;
      outClrPtr += (idx/2);

      *(outClrPtr2) = cb;
      *(outClrPtr2+off) = cr;
      outClrPtr2 += (idx/2);
    }
  }
  else
  {
    return IPL_FAILURE;
  }

  return IPL_SUCCESS;
} /* End ipl_fill_rect_ycbcr */







/* <EJECT> */
/*===========================================================================

FUNCTION ipl_draw_box

DESCRIPTION
  This function draws/fills a rectangular region of the input image with filler
  Pixel and returns the result in output. If output is NULL, the operation
  happens on the input.
  
  Input and output images must have the same color format, which can be 
  RGB565, YCbCr 4:2:2, or YCbCr 420 line pack.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr*    pointer to input image
  out_img_ptr*   pointer to output image, if NULL, operation happens on input.
  fill_rect      pointer to a structure indicating the fill_rect region
  fill_too       should we just draw box, or fill it too?
  fillerPixel    color to fill region with      

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_draw_box
(
  ipl_image_type* in_img_ptr,    /* Points to the first input image  */
  ipl_image_type* out_img_ptr,   /* Points to the output image       */
  ipl_rect_type* fill_rect,      /* Overlap region                   */
  int fill_too,                  /* where we should just draw box or fill too*/
  uint32 fillerPixel            /* color to fill region with        */
)
{
  ipl_rect_type temp;


  /* Check input parameters */
  if (!in_img_ptr || !in_img_ptr->imgPtr)
  {
    return IPL_FAILURE;
  }

  if (fill_rect == NULL)
  {
    temp.x = 0;
    temp.y = 0;
    temp.dx = in_img_ptr->dx;
    temp.dy = in_img_ptr->dy;
  }
  else
  {
    temp.x = fill_rect->x;
    temp.y = fill_rect->y;
    temp.dx = fill_rect->dx;
    temp.dy = fill_rect->dy;
  }


  if (out_img_ptr != NULL)
  {
    /* Input and output color formats must be equal */
    if (in_img_ptr->cFormat != out_img_ptr->cFormat)
    {
      return IPL_FAILURE;
    }

    /* Output image must have the same size as 1st input image */
    if (out_img_ptr->dx != in_img_ptr->dx ||
        out_img_ptr->dy != in_img_ptr->dy) 
    {
      return IPL_FAILURE;
    }
  }

  /* Overlap region must fit within 1st input (background) image */
  if ((temp.x + temp.dx) >= in_img_ptr->dx ||
      (temp.y + temp.dy) >= in_img_ptr->dy ||
      temp.y  >= in_img_ptr->dy ||
      temp.dy >= in_img_ptr->dy ||
      temp.x  >= in_img_ptr->dx ||
      temp.dx >= in_img_ptr->dx)
  {
    return IPL_FAILURE;
  }


  if (fill_too)
  {
    /* Call the appropriate function */
    if (in_img_ptr->cFormat == IPL_RGB565) 
    {
      if (ipl_fill_rect_rgb565(in_img_ptr, out_img_ptr, &temp, fillerPixel) 
          != IPL_SUCCESS) 
      {
        return IPL_FAILURE;
      }
    } 
    else if ((in_img_ptr->cFormat == IPL_YCbCr) || 
             (in_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) || 
             (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK))
    {
      if (ipl_fill_rect_ycbcr(in_img_ptr, out_img_ptr, &temp, fillerPixel)
          != IPL_SUCCESS) 
      {
        return IPL_FAILURE;
      }
    }  
    else if (in_img_ptr->cFormat == IPL_HSV) 
    {
      if (ipl_fill_rect_hsv(in_img_ptr, out_img_ptr, &temp, fillerPixel)
          != IPL_SUCCESS) 
      {
        return IPL_FAILURE;
      }
    }
    else
    {
      return IPL_FAILURE;
    }
  }
  else
  {
    /* Call the appropriate function */
    if ((in_img_ptr->cFormat == IPL_YCbCr) ||
        (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) || 
        (in_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK))
    {
      if (ipl_draw_box_ycbcr(in_img_ptr, out_img_ptr, &temp, fillerPixel) 
          != IPL_SUCCESS) 
      {
        return IPL_FAILURE;
      }
    }
    else if(in_img_ptr->cFormat == IPL_HSV)
    {
      if (ipl_draw_box_hsv(in_img_ptr, out_img_ptr, &temp, fillerPixel) 
          != IPL_SUCCESS) 
      {
        return IPL_FAILURE;
      }
    }
    else if(in_img_ptr->cFormat == IPL_RGB888)
    {
      if (ipl_draw_box_rgb888(in_img_ptr, out_img_ptr, &temp, fillerPixel) 
          != IPL_SUCCESS) 
      {
        return IPL_FAILURE;
      }
    }
    else
    {
      return IPL_FAILURE;
    }
  }

  return IPL_SUCCESS;
} /* End ipl_fill_rect */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_draw_histogram

DESCRIPTION
  This function adds a 2D bar chart over an image.


DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr      pointer to the input image
  hist               the histogram to print  (computed with ipl_calc_hist)
  loc                upper left and hand corner and size of graph. 
                     LEAVE NULL for a nice default location/size in b/r.
  r,g,b              color of the chart
 

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_draw_histogram
(
  ipl_image_type*     i_img_ptr,         /* Points to the input image      */
  ipl_image_type*     o_img_ptr,         /* Points to the output image     */
  ipl_histogram_type* hist,       
  ipl_rect_type*      loc, 
  uint8               r, 
  uint8               g,
  uint8               b
)
{
  int16 rgb2ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,
    7192,7192,-6029,-1163};

  uint32 row,col;
  uint32 xpos;
  uint32 xscale;
  uint32 yscale;
  uint32 max;
  uint32 i;
  int32 cb,cr, luma;
  uint32 odx;
  uint32 px, py, pdx, pdy;
  uint8*  lDraw;
  uint8*  cDraw;
  uint16* rgbDraw;
  uint16 gColor;
  uint8* outPtr;
  uint8* outCPtr;
  int chan, maxChan;


  MSG_LOW("ipl_draw_histogram marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || !hist)
  {
    MSG_LOW("ipl_draw_histogram marker_200\n");
    return IPL_FAILURE;
  }

  // we are doing this in place
  if (o_img_ptr == NULL)
  {
    outPtr = i_img_ptr->imgPtr;
    outCPtr = i_img_ptr->clrPtr;
  }
  else
  {
    if (!o_img_ptr->imgPtr)
    {
      MSG_LOW("ipl_draw_histogram marker_201\n");
      return IPL_FAILURE;
    }
    // user wants output to be same as input but with histogram info
    if (o_img_ptr->dx != i_img_ptr->dx ||
        o_img_ptr->dy != i_img_ptr->dy ||
        o_img_ptr->cFormat != i_img_ptr->cFormat)
    {
      MSG_LOW("ipl_draw_histogram marker_202\n");
      return IPL_FAILURE;
    }

    // copy input image straight to outuput before we draw chart on top
    if (ipl_copy_and_paste(i_img_ptr, o_img_ptr, NULL, NULL))
    {
      MSG_LOW("ipl_draw_histogram marker_203\n");
      return IPL_FAILURE;
    }
    //memset(i_img_ptr->imgPtr, 128, i_img_ptr->dx*i_img_ptr->dy);
    //memset(i_img_ptr->clrPtr,   0, i_img_ptr->dx*i_img_ptr->dy/2);


    //ipl_copy_and_paste(i_img_ptr, o_img_ptr, NULL, NULL);
    outPtr =  o_img_ptr->imgPtr;
    outCPtr = o_img_ptr->clrPtr;
  }
  odx = i_img_ptr->dx;

  // user may have forgotten to tell use size of histogram
  if (hist->size == 0)
    hist->size = 256;

  maxChan = 1;

  MSG_LOW("ipl_draw_histogram marker_1\n");

  // if no loc, then add the whole frame
  if (loc == NULL)
  {
    pdx = i_img_ptr->dx >> 2;
    pdy = i_img_ptr->dy >> 2;

    if ((pdx < 50) || (pdy < 50)) 
    {
      pdx = i_img_ptr->dx >> 1;
      pdy = i_img_ptr->dy >> 2;

      px = pdx - (i_img_ptr->dx >> 5);
      py = 3*pdy - (i_img_ptr->dx >> 5);
    }
    else
    {
      px = 3*pdx - (i_img_ptr->dx >> 5);
      py = 3*pdy - (i_img_ptr->dx >> 5);
    }
  }
  else
  {
    px = loc->x;
    py = loc->y;
    pdx = loc->dx;
    pdy = loc->dy;
  }


  // currently cannot draw histogram wider than 236 pixles
  if (pdy > 236)
    pdy = 236;

  // make sure values make sense
  if (px + pdx > i_img_ptr->dx)
  {
    MSG_LOW("ipl_draw_histogram marker_204\n");
    return IPL_FAILURE;
  }

  if (py + pdy > i_img_ptr->dy)
  {
    MSG_LOW("ipl_draw_histogram marker_205\n");
    return IPL_FAILURE;
  }

  //make py the bottom of the chart
  py += pdy;

  if (pdx >= hist->size)
    pdx = hist->size-1;


  if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
    printf("Going to draw histograt at %lu, %lu, %lu x %lu\n",px,py,pdx,pdy); 


  max = hist->v[0];
  for (chan = 0; chan < maxChan; chan++)
  {
    for (col = 1; col < hist->size; col++)
    {
      if ((col < IPL_MAX_HIST) && (hist->v[col] > max))
        max = hist->v[col];
    }
  }

  if (max == 0)
    max = 1;

  if (pdx == 0)
    pdx = 1;

  xscale = ((hist->size) << 8)/ (pdx);

  if (pdy == 0)
    pdy = 1;

  yscale = ((max) << 8) / pdy;

  if (xscale < 1<<8)
    xscale = 1<<8;

  if (yscale < 1<<8)
    yscale = 1<<8;


  if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
  {
    MSG_LOW("Max value is %lu and pdy is %lu, so skip is %lu\n", 
             max, pdy, yscale>>8);
    MSG_LOW("Max entires is %d, and pdx is %lu, so skip is %lu\n", 
             hist->size, pdx, xscale>>8);
    MSG_LOW("Start at %lu, %lu\n", px, py);
  }

  MSG_LOW("ipl_draw_histogram marker_2\n");


  if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    if (px%2) px--;
    if (py%2) py--;
    if (pdx%2) pdx--;
    if (pdy%2) pdy--;


    for (chan = 0; chan < maxChan; chan++)
    {
      switch (chan)
      {
        case 0:
          // noop, lear r,g,b alone
        break;

        case 1:
          r = 255;
          g = b = 0;
        break;

        case 2:
          g = 255;
          r = b = 0;
        break;

        case 3:
          b = 255;
          r = g = 0;
        break;

        default:
          // noop
        break;
      }

      // figure out the color we need to draw
      luma = (rgb2ycbcr_convert[0]*r + rgb2ycbcr_convert[1]*g + 
              rgb2ycbcr_convert[2]*b)*4+0x8000;
      luma = (luma>>16) + 16;
      luma= CLIPIT(luma);

      cb = (rgb2ycbcr_convert[3]*r + rgb2ycbcr_convert[4]*g + 
            rgb2ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      cr = (rgb2ycbcr_convert[6]*r + rgb2ycbcr_convert[7]*g + 
            rgb2ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);


      MSG_LOW("ipl_draw_histogram marker_3\n");


      // we are doing this in place
      if (o_img_ptr == NULL)
      {
        outPtr = i_img_ptr->imgPtr;
        outCPtr = i_img_ptr->clrPtr;
      }
      else
      {
        outPtr =  o_img_ptr->imgPtr;
        outCPtr = o_img_ptr->clrPtr;
      }
      
    
      // go through and draw the histogram values
      for (xpos = 0; xpos < pdx; xpos++)
      {
        // find and draw the max of group 
        col = (xpos * xscale)>>8;  
     
        if (col+(xscale>>8) < hist->size)
        {
          max = hist->v[col];
          for (i = col+1; i <= col+(xscale>>8); i++)
          {
            if (hist->v[i] > max)
              max = hist->v[i];
          }
        }
        else
        {
          if ((hist->size-1) < IPL_MAX_HIST)
            max = hist->v[hist->size-1];
        }

        if (max%2) max++;
        if (max < 2) max = 2;

        // draw the max
        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
          MSG_LOW("%lu: Going to draw %lu tall for hist %lu\n", xpos, max,col);

        lDraw = outPtr + (px + py*odx + 2*(xpos/2));
        cDraw = outCPtr + (px + py*odx/2 + 2*(xpos/2)); 

        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
          MSG_LOW("%lu: Going to draw crycb %lu for hist %lu\n",xpos,max,col);

        // shift 8 for Q number, -1 for doing half as many since we do two
        // rows at a time
        for(row = 0; row < max; row += (yscale>>7))
        {
          *(lDraw) = (uint8) luma; 
          *(lDraw+1) = (uint8) luma; // to prevent color bleeding
          *(lDraw+odx) = (uint8) luma; 
          *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding

          *(cDraw) = (uint8) cr; 
          *(cDraw+1) = (uint8) cb; 

          lDraw -= (2*odx);
          cDraw -= (odx);
        }
      }
    }
  }
  else if (i_img_ptr->cFormat == IPL_RGB565)
  {
    gColor = pack_rgb565(r,g,b);
    for (xpos = 0; xpos < pdx; xpos++)
    {
      col = (xpos * xscale)>>8;  

      // since are only going to draw every xscale histogram value, do some
      // averageing
      if (col+(xscale>>8) < hist->size)
      {
        max = hist->v[col];
        for (i = col+1; i <= col+(xscale>>8); i++)
        {
          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
            MSG_LOW("seeing if %lu is greater than %lu\n", hist->v[i], max);

          if (hist->v[i] > max)
          {
            max = hist->v[i];
          }
        }
      }
      else
      {
        if((hist->size-1) < IPL_MAX_HIST)
          max = hist->v[hist->size-1];
      }

      if (max%2) max++;
      if (max < 2) max = 2;

      // draw the max
      if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
        MSG_LOW("%lu: Going to draw line %lu for hist %lu\n",xpos,max,col);

      rgbDraw = (uint16*) outPtr + (px + py*odx + xpos);
      for(row = 0; row < max; row += (yscale>>8))
      {
        *rgbDraw = gColor;
        rgbDraw -= odx;
      }
    }
  }
  else if (i_img_ptr->cFormat == IPL_YCbCr)
  {
    if (px%2) px--;
    if (pdx%2) pdx--;

    luma = (rgb2ycbcr_convert[0]*r + rgb2ycbcr_convert[1]*g + 
           rgb2ycbcr_convert[2]*b)*4+0x8000;
    luma = (luma>>16) + 16;
    luma= CLIPIT(luma);
    cb = (rgb2ycbcr_convert[3]*r + rgb2ycbcr_convert[4]*g + 
        rgb2ycbcr_convert[5]*b)*4+0x8000;
    cb = (cb>>16) + 128;
    cb = CLIPIT(cb);

    cr = (rgb2ycbcr_convert[6]*r + rgb2ycbcr_convert[7]*g + 
        rgb2ycbcr_convert[8]*b)*4+0x8000;
    cr = (cr>>16) + 128;
    cr = CLIPIT(cr);
    
    for (xpos = 0; xpos < pdx; xpos++)
    {

      // find and draw the max of group 
      col = (xpos * xscale)>>8;  
     
      if (col+(xscale>>8) < hist->size)
      {
        max = hist->v[col];
        for (i = col+1; i <= col+(xscale>>8); i++)
        {
          if (hist->v[i] > max)
            max = hist->v[i];
        }
      }
      else
      {
        if((hist->size-1) < IPL_MAX_HIST)
          max = hist->v[hist->size-1];
      }

      if (max%2) max++;
      if (max < 2) max = 2;

      // draw the max

      lDraw = outPtr + (2*px + py*odx*2 + 2*xpos);
      if (xpos % 2)
      {
        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
         MSG_LOW("%lu: Going to draw crycb line %lu for hist %lu\n",xpos,max,col);

        for(row = 0; row < max; row += (yscale>>8))
        {
          *(lDraw-2) = (uint8) cb;
          *(lDraw-1) = (uint8) luma; // to prevent color bleeding
          *(lDraw)   = (uint8) cr;
          *(lDraw+1) = (uint8) luma;

          lDraw -= (2*odx);
        }
      }
      else
      {
        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
         MSG_LOW("%lu: Going to draw cbycr line %lu for hist %lu\n",xpos,max,col);

        for(row = 0; row < max; row += (yscale>>8))
        {
          *(lDraw)   = (uint8) cb;
          *(lDraw+1) = (uint8) luma;
          *(lDraw+2) = (uint8) cr;
          *(lDraw+3) = (uint8) luma; // to prevent color bleeding
          lDraw -= (2*odx);
        }
      }
    }
  }
  else 
  {
    MSG_LOW("ipl_draw_histogram marker_206\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_draw_histogram marker_100\n");
  return IPL_SUCCESS;
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_draw_mchan_histogram

DESCRIPTION
  This function adds a 2D bar chart over an image.


DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr      pointer to the input image
  hist               the histogram to print  (computed with ipl_calc_hist)
  loc                upper left and hand corner and size of graph. 
                     LEAVE NULL for a nice default location/size in b/r.
  r,g,b              color of the chart
 

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_draw_mchan_histogram
(
  ipl_image_type*     i_img_ptr,         /* Points to the input image      */
  ipl_image_type*     o_img_ptr,         /* Points to the output image     */
  ipl_mchan_histogram_type* hist,       
  ipl_rect_type*      loc, 
  ipl_rotate90_type   rot,
  int                 lineType,
  uint8               r, 
  uint8               g,
  uint8               b
)
{
  int16 rgb2ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,
    7192,7192,-6029,-1163};

  uint32 row,col;
  uint32 xpos, ypos;
  uint32 xscale = 0;
  uint32 yscale = 0;
  int lastlastMax, min;
  unsigned int lastMax, max;
  int compMax[4];
  uint32 i;
  int32 cb,cr, luma;
  uint32 odx;
  uint32 px, py, pdx, pdy;
  uint8*  lDraw;
  uint8*  cDraw;
  uint16* rgbDraw;
  uint16 gColor;
  uint8* outPtr;
  uint8* outCPtr;
  int chan, maxChan;


  MSG_LOW("ipl_draw_histogram marker_0\n");


  if (!i_img_ptr || !i_img_ptr->imgPtr || !hist)
  {
    MSG_LOW("ipl_draw_histogram marker_200\n");
    return IPL_FAILURE;
  }

  if (hist->channel != IPL_CHANNEL_YRGB)
  {
    MSG_LOW("ipl_draw_histogram marker_200.0\n");
    return IPL_FAILURE;
  }


  // we are doing this in place
  if (o_img_ptr == NULL)
  {
    outPtr = i_img_ptr->imgPtr;
    outCPtr = i_img_ptr->clrPtr;
  }
  else
  {
    if (!o_img_ptr->imgPtr)
    {
      MSG_LOW("ipl_draw_histogram marker_201\n");
      return IPL_FAILURE;
    }
    // user wants output to be same as input but with histogram info
    if (o_img_ptr->dx != i_img_ptr->dx ||
        o_img_ptr->dy != i_img_ptr->dy ||
        o_img_ptr->cFormat != i_img_ptr->cFormat)
    {
      MSG_LOW("ipl_draw_histogram marker_202\n");
      return IPL_FAILURE;
    }

    // copy input image straight to outuput before we draw chart on top
    if (ipl_copy_and_paste(i_img_ptr, o_img_ptr, NULL, NULL))
    {
      MSG_LOW("ipl_draw_histogram marker_203\n");
      return IPL_FAILURE;
    }
    //memset(i_img_ptr->imgPtr, 128, i_img_ptr->dx*i_img_ptr->dy);
    //memset(i_img_ptr->clrPtr,   0, i_img_ptr->dx*i_img_ptr->dy/2);


    //ipl_copy_and_paste(i_img_ptr, o_img_ptr, NULL, NULL);
    outPtr =  o_img_ptr->imgPtr;
    outCPtr = o_img_ptr->clrPtr;
  }
  odx = i_img_ptr->dx;

  // user may have forgotten to tell use size of histogram
  if (hist->size == 0)
    hist->size = 256;

  maxChan = 4;

  MSG_LOW("ipl_draw_histogram marker_1\n");

  // if no loc, then add the whole frame
  if (loc == NULL)
  {
    pdx = i_img_ptr->dx >> 2;
    pdy = i_img_ptr->dy >> 2;

    // set default to lower left quadron
    if (rot == IPL_ROT270)
    {
      if ((pdx < 50) || (pdy < 50)) 
      {
        pdx = i_img_ptr->dx >> 1;
        pdy = i_img_ptr->dy >> 2;
      }

      px = i_img_ptr->dx - 10;
      py = pdy + 10;
    }
    else if (rot == IPL_ROT90)
    {
      if ((pdx < 50) || (pdy < 50)) 
      {
        pdx = i_img_ptr->dx >> 1;
        pdy = i_img_ptr->dy >> 2;
      }

      px = 2;
      py = i_img_ptr->dy - pdy - 6;
    }
    else
    {
      // set default to lower right quadron
      if ((pdx < 50) || (pdy < 50)) 
      {
        pdx = i_img_ptr->dx >> 1;
        pdy = i_img_ptr->dy >> 2;

        px = pdx - (i_img_ptr->dx >> 5);
        py = 3*pdy - (i_img_ptr->dx >> 5);
      }
      else
      {
        px = 3*pdx - (i_img_ptr->dx >> 5);
        py = 3*pdy - (i_img_ptr->dx >> 5);
      }
    }
  }
  else
  {
    px = loc->x;
    py = loc->y;
    pdx = loc->dx;
    pdy = loc->dy;
  }


  // currently cannot draw histogram wider than 236 pixles
  if (rot == IPL_NOROT || rot == IPL_ROT180)
  {
    if (pdy > 236)
      pdy = 236;

    // we currently do not draw histogram wider than 255 values
    if (pdx >= hist->size)
      pdx = hist->size-1;
  }
  else
  {
    if (pdx > 236)
      pdx = 236;

    // we currently do not draw histogram wider than 255 values
    if (pdy >= hist->size)
      pdy = hist->size-1;
  }


  if (rot == IPL_ROT270)
  {
    // make sure values make sense
    if (px < pdx)
    {
      MSG_LOW("ipl_draw_histogram marker_204\n");
      return IPL_FAILURE;
    }

    if (py < pdy)
    {
      MSG_LOW("ipl_draw_histogram marker_205\n");
      return IPL_FAILURE;
    }
  }
  else
  {
    // make sure values make sense
    if (px + pdx > i_img_ptr->dx)
    {
      MSG_LOW("ipl_draw_histogram marker_204\n");
      return IPL_FAILURE;
    }

    if (py + pdy > i_img_ptr->dy)
    {
      MSG_LOW("ipl_draw_histogram marker_205\n");
      return IPL_FAILURE;
    }
  }

  // find max value in all channels. 
  // This will help us normalize when drawing
  for (chan = 0; chan < maxChan; chan++)
  {
    //compMax[chan] = hist->mv[0][chan];
    compMax[chan] = 1;
    if (hist->active[chan] == 0)
    {
      if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
          MSG_LOW("skipping channel %d\n", chan); 
      compMax[chan] = 1;
    }
    else
    {
      for (col = 0; col < hist->size; col++)
      {
        if (hist->mv[col][chan] > (uint32_t)compMax[chan])
          compMax[chan] = hist->mv[col][chan];
      }

      if (compMax[chan] == 0)
        compMax[chan] = 1;
    }
  }

  MSG_LOW("ipl_draw_histogram marker_2\n");

  max = 0;
  lastMax = 0;
  lastlastMax = 0;

  if (rot == IPL_NOROT || rot == IPL_ROT180)
    py += pdy;

  if (hist->channel == IPL_CHANNEL_YRGB)
  {
  if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    if (px%2) px--;
    if (py%2) py--;
    if (pdx%2) pdx--;
    if (pdy%2) pdy--;


    for (chan = 0; chan < maxChan; chan++)
    {
      max = 0;
      lastMax = 0;
      lastlastMax = 0;

      // if we are drawing luma, we use different x and y scale, one that
      // normalized indepdent of r,g,b values
      max = compMax[chan];

      //make py the bottom of the chart
      if (rot == IPL_NOROT || rot == IPL_ROT180)
      {
        // make luma histogram bigger than rgb, i.e. reduce its max
        // when deciding how much to scale
        if (chan == 0)
          max = (max*3)>>2;

        
        if (pdx == 0)
          pdx = 1;

        xscale = ((hist->size) << 8)/ (pdx);

        if (pdy == 0)
          pdy = 1;

        yscale = ((max) << 8) / pdy;

        if (xscale < 1<<8)
          xscale = 1<<8;

        if (yscale < 1<<8)
          yscale = 1<<8;

        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
        {
          MSG_LOW("Max entry is %3d and pdx is %lu, so skip is %lu\n", 
             hist->size, pdx, xscale>>8);
          MSG_LOW("Max value is %3u and pdy is %lu, so skip is %lu\n", 
             max, pdy, yscale>>8);
          MSG_LOW("Start at %lu, %lu\n", px, py);
        }
      }
      else
      {
        // make luma histogram bigger than rgb, i.e. reduce its max
        // when deciding how much to scale
        if (chan == 0)
          max = (max*3)>>2;

        xscale = ((max) << 8) / pdx;
        yscale = ((hist->size) << 8)/ (pdy);
      

        if (xscale < 1<<8)
          xscale = 1<<8;

        if (yscale < 1<<8)
          yscale = 1<<8;

        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
        {
          MSG_LOW("Max value is %3u and pdx is %lu, so skip is %lu\n", 
             max, pdx, xscale>>8);
          MSG_LOW("Max entry is %3d and pdy is %lu, so skip is %lu\n", 
             hist->size, pdy, yscale>>8);
          MSG_LOW("Start at %lu, %lu\n", px, py);
        }
      }



      if (hist->active[chan] == 0)
      {
        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
          MSG_LOW("skipping channel %d\n", chan); 
        continue;
      }

      switch (chan)
      {
        case 0:
          // noop, lear r,g,b alone
        break;

        case 1:
          r = 255;
          g = b = 0;
        break;

        case 2:
          g = 255;
          r = b = 0;
        break;

        case 3:
          b = 255;
          r = g = 0;
        break;

        default:
          // noop
        break;
      }

      // figure out the color we need to draw
      luma = (rgb2ycbcr_convert[0]*r + rgb2ycbcr_convert[1]*g + 
              rgb2ycbcr_convert[2]*b)*4+0x8000;
      luma = (luma>>16) + 16;
      luma= CLIPIT(luma);

      cb = (rgb2ycbcr_convert[3]*r + rgb2ycbcr_convert[4]*g + 
            rgb2ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      cr = (rgb2ycbcr_convert[6]*r + rgb2ycbcr_convert[7]*g + 
            rgb2ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);


      MSG_LOW("ipl_draw_histogram marker_3\n");


      // we are doing this in place
      if (o_img_ptr == NULL)
      {
        outPtr = i_img_ptr->imgPtr;
        outCPtr = i_img_ptr->clrPtr;
      }
      else
      {
        outPtr =  o_img_ptr->imgPtr;
        outCPtr = o_img_ptr->clrPtr;
      }
      
    
      if (rot == IPL_NOROT || rot == IPL_ROT180)
      {
        // go through and draw the histogram values
        for (xpos = 0; xpos < pdx; xpos++)
        {
          // find and draw the max of group 
          col = (xpos * xscale)>>8;  
     
          if (col+(xscale>>8) < hist->size)
          {
            max = hist->mv[col][chan];
            for (i = col+1; i <= col+(xscale>>8); i++)
            {
              if (hist->mv[i][chan] > max)
                max = hist->mv[i][chan];
            }
          }
          else
          {
            max = hist->mv[hist->size-1][chan];
          }

          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
            printf("\n%lu: max: %u, lastMax:%u lastlastMax:%d\n", xpos, max, lastMax, lastlastMax);



          if (max%2) max++;
          if (max < 2) max = 2;

          // draw the max
          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
            MSG_LOW("%lu:Going to draw %u tall for hist %lu\n", xpos, max,col);
  
          lDraw = outPtr + (px + py*odx + 2*(xpos/2));
          cDraw = outCPtr + (px + py*odx/2 + 2*(xpos/2)); 
  
          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
            MSG_LOW("%lu: Going to draw crycb %u for hist %lu\n",xpos,max,col);
  
          // shift 8 for Q number, -1 for doing half as many since we do two
          // rows at a time

          // draw bar chart
          if ((chan == 0 && lineType == 0) || lineType == 1)
          {
            for(row = 0; row < max; row += (yscale>>7))
            {
              if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                printf("drawing (up) from row %lu to max %u\n", row, max);

              *(lDraw) = (uint8) luma; 
              *(lDraw+1) = (uint8) luma; // to prevent color bleeding
              *(lDraw+odx) = (uint8) luma; 
              *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
  
              *(cDraw) = (uint8) cr; 
              *(cDraw+1) = (uint8) cb; 

              lDraw -= (2*odx);
              cDraw -= (odx);
            }
          }
          else
          {
            // do line chart, draw delta between last guy and me 
            // (if I am higher)
            for(row = 0; row < max; row += (yscale>>7))
            {
              // only draw portion of line greater than last value
              if (row >= lastMax)
              {
                if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                  printf("drawing (up) from row %lu to max %u\n", row, max);

                *(lDraw) = (uint8) luma; 
                *(lDraw+1) = (uint8) luma; // to prevent color bleeding
                *(lDraw+odx) = (uint8) luma; 
                *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
  
                *(cDraw) = (uint8) cr; 
                *(cDraw+1) = (uint8) cb; 
              }

              lDraw -= (2*odx);
              cDraw -= (odx);
            }


            // help make line a bit thicker
            // neeed if pixel is peak of curve
            *(lDraw) = (uint8) luma; 
            *(lDraw+1) = (uint8) luma; // to prevent color bleeding
            *(lDraw+odx) = (uint8) luma; 
            *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
            *(cDraw) = (uint8) cr; 
            *(cDraw+1) = (uint8) cb; 


            // now draw the segment from low value to where the previous
            // value was starting to be drawn
            min = IPL_MIN(lastMax, (uint32_t)lastlastMax);
            for(row = max; row < (uint32_t)min; row += (yscale>>7))
            {
              if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                printf("drawing (down) from row %lu to min %d\n", row, min);

              *(lDraw) = (uint8) luma; 
              *(lDraw+1) = (uint8) luma; // to prevent color bleeding
              *(lDraw+odx) = (uint8) luma; 
              *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
  
              *(cDraw) = (uint8) cr; 
              *(cDraw+1) = (uint8) cb; 

              lDraw -= (2*odx);
              cDraw -= (odx);
            }

            // help make line a bit thicker
            *(lDraw) = (uint8) luma; 
            *(lDraw+1) = (uint8) luma; // to prevent color bleeding
            *(lDraw+odx) = (uint8) luma; 
            *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
            *(cDraw) = (uint8) cr; 
            *(cDraw+1) = (uint8) cb; 
          }

          lastlastMax = lastMax;
          lastMax = max;
        }
      }
      // 90 or 270 rotation
      else if (rot == IPL_ROT90)
      {
        // go through and draw the histogram values
        for (ypos = 0; ypos < pdy; ypos++)
        {
          // find and draw the max of group 
          row = (ypos * yscale)>>8;  
     
          if (row+(yscale>>8) < hist->size)
          {
            max = hist->mv[row][chan];
            for (i = row+1; i <= row+(yscale>>8); i++)
            {
              if (hist->mv[i][chan] > max)
                max = hist->mv[i][chan];
            }
          }
          else
          {
            max = hist->mv[hist->size-1][chan];
          }

          if (max%2) max++;
          if (max < 2) max = 2;

          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
            printf("\n%lu: max: %d, lastMax:%u lastlastMax:%d\n", ypos, max, lastMax, lastlastMax);

          // draw the max
          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
            MSG_LOW("%lu: Going to draw %u tall for hist %lu\n", ypos, max,row);
  
          lDraw = outPtr + (px + py*odx + 2*(ypos/2)*odx);
          cDraw = outCPtr + (px + py*odx/2 + (ypos/2)*odx); 
  
          // draw bar chart
          if ((chan == 0 && lineType == 0) || lineType == 1)
          {
            // shift 8 for Q number, -1 for doing half as many since we do two
            // rows at a time
            for(col = 0; col < max; col += (xscale>>7))
            {
              if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                printf("drawing (up) from col %lu to max %u\n", col, max);

              *(lDraw) = (uint8) luma; 
              *(lDraw+1) = (uint8) luma; // to prevent color bleeding
              *(lDraw+odx) = (uint8) luma; 
              *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
  
              *(cDraw) = (uint8) cr; 
              *(cDraw+1) = (uint8) cb; 

              lDraw += 2;
              cDraw += 2;
            }
          }
          else
          {
            // shift 8 for Q number, -1 for doing half as many since we do two
            // rows at a time
            for(col = 0; col < max; col += (xscale>>7))
            {
              // only draw portion of line greater than last value
              if (col >= lastMax)
              {
                if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                  printf("drawing (up) from col %lu to max %u\n", col, max);

                *(lDraw) = (uint8) luma; 
                *(lDraw+1) = (uint8) luma; // to prevent color bleeding
                *(lDraw+odx) = (uint8) luma; 
                *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
  
                *(cDraw) = (uint8) cr; 
                *(cDraw+1) = (uint8) cb; 
              }

              lDraw += 2;
              cDraw += 2;
            }


            // draw one pixel
            // helps make the line a bit thicker
            // also needed if at the peak.
            *(lDraw) = (uint8) luma; 
            *(lDraw+1) = (uint8) luma; // to prevent color bleeding
            *(lDraw+odx) = (uint8) luma; 
            *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
            *(cDraw) = (uint8) cr; 
            *(cDraw+1) = (uint8) cb; 

            // now draw the segment from low value to where the previous
            // value was starting to be drawn
            min = IPL_MIN(lastMax, (uint32_t)lastlastMax);
            for(col = max; col < (uint32_t)min; col += (xscale>>7))
            {
              if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                printf("drawing (down) from col %lu to max %d\n", col, min);

              *(lDraw) = (uint8) luma; 
              *(lDraw+1) = (uint8) luma; // to prevent color bleeding
              *(lDraw+odx) = (uint8) luma; 
              *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
  
              *(cDraw) = (uint8) cr; 
              *(cDraw+1) = (uint8) cb; 

              lDraw += 2;
              cDraw += 2;
            }

            // draw one pixel
            // helps make the line a bit thicker
            *(lDraw) = (uint8) luma; 
            *(lDraw+1) = (uint8) luma; // to prevent color bleeding
            *(lDraw+odx) = (uint8) luma; 
            *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
            *(cDraw) = (uint8) cr; 
            *(cDraw+1) = (uint8) cb; 

          }

          lastlastMax = lastMax;
          lastMax = max;
        }
      }
      // 270 rotation
      else
      {
        // go through and draw the histogram values
        for (ypos = 0; ypos < pdy; ypos++)
        {
          // find and draw the max of group 
          row = (ypos * yscale)>>8;  
     
          if (row+(yscale>>8) < hist->size)
          {
            max = hist->mv[row][chan];
            for (i = row+1; i <= row+(yscale>>8); i++)
            {
              if (hist->mv[i][chan] > max)
                max = hist->mv[i][chan];
            }
          }
          else
          {
            max = hist->mv[hist->size-1][chan];
          }

          if (max%2) max++;
          if (max < 2) max = 2;

          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
            printf("\n%lu: max: %u, lastMax:%u lastlastMax:%d\n", ypos, max, lastMax, lastlastMax);

          // draw the max
          if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
            MSG_LOW("%lu: Going to draw %u tall for hist %lu\n", ypos, max,row);
  
          lDraw = outPtr  + (px + py*odx   - 2*(ypos/2)*odx);
          cDraw = outCPtr + (px + py*odx/2 -   (ypos/2)*odx); 
  
          // draw bar chart
          if ((chan == 0 && lineType == 0) || lineType == 1)
          {
            // shift 8 for Q number, -1 for doing half as many since we do two
            // rows at a time
            for(col = 0; col < max; col += (xscale>>7))
            {
              if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                printf("drawing (up) from col %lu to max %u\n", col, max);

              *(lDraw) = (uint8) luma; 
              *(lDraw+1) = (uint8) luma; // to prevent color bleeding
              *(lDraw+odx) = (uint8) luma; 
              *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
  
              *(cDraw) = (uint8) cr; 
              *(cDraw+1) = (uint8) cb; 

              lDraw -= 2;
              cDraw -= 2;
            }
          }
          else
          {
            // shift 8 for Q number, -1 for doing half as many since we do two
            // rows at a time
            for(col = 0; col < max; col += (xscale>>7))
            {
              // only draw portion of line greater than last value
              if (col >= lastMax)
              {
                if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                  printf("drawing (up) from col %lu to max %u\n", col, max);

                *(lDraw) = (uint8) luma; 
                *(lDraw+1) = (uint8) luma; // to prevent color bleeding
                *(lDraw+odx) = (uint8) luma; 
                *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
  
                *(cDraw) = (uint8) cr; 
                *(cDraw+1) = (uint8) cb; 
              }

              lDraw -= 2;
              cDraw -= 2;
            }


            // draw one pixel
            // helps make the line a bit thicker
            // also needed if at the peak.
            *(lDraw) = (uint8) luma; 
            *(lDraw+1) = (uint8) luma; // to prevent color bleeding
            *(lDraw+odx) = (uint8) luma; 
            *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
            *(cDraw) = (uint8) cr; 
            *(cDraw+1) = (uint8) cb; 

            // now draw the segment from low value to where the previous
            // value was starting to be drawn
            min = IPL_MIN(lastMax, (uint32_t)lastlastMax);
            for(col = max; col < (uint32_t)min; col += (xscale>>7))
            {
              if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                printf("drawing (down) from col %lu to max %d\n", col, min);

              *(lDraw) = (uint8) luma; 
              *(lDraw+1) = (uint8) luma; // to prevent color bleeding
              *(lDraw+odx) = (uint8) luma; 
              *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
  
              *(cDraw) = (uint8) cr; 
              *(cDraw+1) = (uint8) cb; 

              lDraw -= 2;
              cDraw -= 2;
            }

            // draw one pixel
            // helps make the line a bit thicker
            *(lDraw) = (uint8) luma; 
            *(lDraw+1) = (uint8) luma; // to prevent color bleeding
            *(lDraw+odx) = (uint8) luma; 
            *(lDraw+1+odx) = (uint8) luma; // to prevent color bleeding
            *(cDraw) = (uint8) cr; 
            *(cDraw+1) = (uint8) cb; 

          }

          lastlastMax = lastMax;
          lastMax = max;
        }
      }
    }
  }
  else if (i_img_ptr->cFormat == IPL_RGB565)
  {
    //int pixDrawn;

    for (chan = 0; chan < maxChan; chan++)
    {
      max = 0;
      lastMax = 0;
      lastlastMax = 0;

      if (hist->active[chan] == 0)
      {
        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
          MSG_LOW("skipping channel %d\n", chan); 
        continue;
      }

      switch (chan)
      {
        case 0:
          // noop, lear r,g,b alone
        break;

        case 1:
          r = 255;
          g = b = 0;
        break;

        case 2:
          g = 255;
          r = b = 0;
        break;

        case 3:
          b = 255;
          r = g = 0;
        break;

        default:
          // noop
        break;
      }

      // draw color
      gColor = pack_rgb565(r,g,b);


      // if we are drawing luma, we use different x and y scale, one that
      // normalized indepdent of r,g,b values
      max = compMax[chan];

      //make py the bottom of the chart
      if (rot != IPL_NOROT)
        return IPL_FAILURE;

      if (chan != 0 && hist->active[0] == 1)
      {
        max *= 2;
      }

      
      if (pdx == 0)
        pdx = 1;

      xscale = ((hist->size) << 8)/ (pdx);

      if (pdy == 0)
        pdy = 1;

      yscale = ((max) << 8) / pdy;

      if (xscale < 1<<8)
        xscale = 1<<8;

      if (yscale < 1<<8)
        yscale = 1<<8;


      if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
      {
        MSG_LOW("Max entry is %3d and pdx is %lu, so skip is %lu\n", 
             hist->size, pdx, xscale>>8);
        MSG_LOW("Max value is %3u and pdy is %lu, so skip is %lu\n", 
             max, pdy, yscale>>8);
        MSG_LOW("Start at %lu, %lu\n", px, py);
      }

      for (xpos = 0; xpos < pdx; xpos++)
      {
        //pixDrawn = 0;

        col = (xpos * xscale)>>8;  

        // since are only going to draw every xscale histogram value, do some
        // averageing
        if (col+(xscale>>8) < hist->size)
        {
          max = hist->mv[col][chan];
          for (i = col+1; i <= col+(xscale>>8); i++)
          {
            if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
              MSG_LOW("seeing if %lu is greater than %u\n", hist->mv[i][0], max);

            if (hist->mv[i][chan] > max)
            {
              max = hist->mv[i][chan];
            }
          }
        }
        else
        {
          max = hist->mv[hist->size-1][chan];
        }

        if (max < 2) max = 2;

        // draw the max
        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
          MSG_LOW("%lu: Going to draw line %u for hist %lu\n",xpos,max,col);

        rgbDraw = (uint16*) outPtr + (px + py*odx + xpos);

        // draw bar chart
        if ((chan == 0 && lineType == 0) || lineType == 1)
        {
          *rgbDraw = gColor;

//          for(row = 0; row < max; row += (yscale>>8))
          for(row = 0; row < (max<<8); row += yscale)
          {
            *rgbDraw = gColor;
            rgbDraw -= odx;

            //pixDrawn++;
          }
        }
        else
        {
          // do line chart, draw delta between last guy and me 
          // (if I am higher)
         
          *rgbDraw = gColor;
//          for(row = 0; row < max; row += (yscale>>8))
          for(row = 0; row < (max<<8); row += yscale)
          {
            // only draw portion of line greater than last value
//            if (row >= lastMax)
            if (row >= (lastMax<<8))
            {
              if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
                printf("drawing (up) from row %lu to max %u\n", row, max);

              *rgbDraw = gColor;
            }

            rgbDraw -= odx;
            //pixDrawn++;
          }


          // help make line a bit thicker
          // neeed if pixel is peak of curve
          *rgbDraw = gColor;

          // now draw the segment from low value to where the previous
          // value was starting to be drawn
          min = IPL_MIN(lastMax, (uint32_t)lastlastMax);
//          for(row = max; row < min; row += (yscale>>8))
          for(row = (max<<8); row < (uint32_t)(min<<8); row += yscale)
          {
            if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
              printf("drawing (down) from row %lu to min %d\n", row, min);

            *rgbDraw = gColor;
            rgbDraw -= odx;
            //pixDrawn++;
          }

          // help make line a bit thicker
          *rgbDraw = gColor;
        }

        lastlastMax = lastMax;
        lastMax = max;

        //printf("Drew %d pixels for x %d\n", pixDrawn, xpos);
      }
    }
  }
  else if (i_img_ptr->cFormat == IPL_YCbCr)
  {
    if (px%2) px--;
    if (pdx%2) pdx--;

    luma = (rgb2ycbcr_convert[0]*r + rgb2ycbcr_convert[1]*g + 
           rgb2ycbcr_convert[2]*b)*4+0x8000;
    luma = (luma>>16) + 16;
    luma= CLIPIT(luma);
    cb = (rgb2ycbcr_convert[3]*r + rgb2ycbcr_convert[4]*g + 
        rgb2ycbcr_convert[5]*b)*4+0x8000;
    cb = (cb>>16) + 128;
    cb = CLIPIT(cb);

    cr = (rgb2ycbcr_convert[6]*r + rgb2ycbcr_convert[7]*g + 
        rgb2ycbcr_convert[8]*b)*4+0x8000;
    cr = (cr>>16) + 128;
    cr = CLIPIT(cr);


    if (pdx == 0)
      pdx = 1;

    xscale = ((hist->size) << 8)/ (pdx);

    if (pdy == 0)
      pdy = 1;

    yscale = ((compMax[0]) << 8) / pdy;

    if (xscale < 1<<8)
      xscale = 1<<8;

    if (yscale < 1<<8)
      yscale = 1<<8;

    if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
    {
      MSG_LOW("Max entry is %3d and pdx is %lu, so skip is %lu\n", 
             hist->size, pdx, xscale>>8);
      MSG_LOW("Max value is %3u and pdy is %lu, so skip is %lu\n", 
             max, pdy, yscale>>8);
      MSG_LOW("Start at %lu, %lu\n", px, py);
    }

    for (xpos = 0; xpos < pdx; xpos++)
    {
      // find and draw the max of group 
      col = (xpos * xscale)>>8;  
     
      if (col+(xscale>>8) < hist->size)
      {
        max = hist->mv[col][0];
        for (i = col+1; i <= col+(xscale>>8); i++)
        {
          if (hist->mv[i][0] > max)
            max = hist->mv[i][0];
        }
      }
      else
      {
        max = hist->mv[hist->size-1][0];
      }

      if (max%2) max++;
      if (max < 2) max = 2;

      // draw the max

      lDraw = outPtr + (2*px + py*odx*2 + 2*xpos);
      if (xpos % 2)
      {
        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
         MSG_LOW("%lu: Going to draw crycb line %u for hist %lu\n",xpos,max,col);

        for(row = 0; row < max; row += (yscale>>8))
        {
          *(lDraw-2) = (uint8) cb;
          *(lDraw-1) = (uint8) luma; // to prevent color bleeding
          *(lDraw)   = (uint8) cr;
          *(lDraw+1) = (uint8) luma;

          lDraw -= (2*odx);
        }
      }
      else
      {
        if (IPL_COMPOSE_DEBUG) //lint !e506 !e774
         MSG_LOW("%lu: Going to draw cbycr line %u for hist %lu\n",xpos,max,col);

        for(row = 0; row < max; row += (yscale>>8))
        {
          *(lDraw)   = (uint8) cb;
          *(lDraw+1) = (uint8) luma;
          *(lDraw+2) = (uint8) cr;
          *(lDraw+3) = (uint8) luma; // to prevent color bleeding
          lDraw -= (2*odx);
        }
      }


    }
  }
  else 
  {
    MSG_LOW("ipl_draw_histogram marker_206\n");
    return IPL_FAILURE;
  }
  }
  else 
  {
    MSG_LOW("ipl_draw_histogram marker_206\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_draw_histogram marker_100\n");
  return IPL_SUCCESS;
} 






/*===========================================================================

FUNCTION    ipl_stitchFrames 

DESCRIPTION

  This function will take a pointer to an array of input images and x,y 
  offset and keep pasting the next image over the previous until there
  aren no more images in input array. The output image needs to be big enough
  to hold all the input imges minus the overlapy widths which will be a 
  blended combination of images 1 and 2 or images 2 and 3. We currenly 
  support only 3 images stitching.

  Input frames, and output images must both be YCbCr 422.

DEPENDENCIES
  None

ARGUMENTS IN
  hdiff     horizontal offset, i.e. where should 2nd image be placed over 
            over the 1st.

  vdiff     vertical offset (not currently supported)

  blendWidth How wide should the blend width be once an image is placed
             over the other one.

  smartSeam should the seam be placed intelligently? (not supported)

ARGUMENTS IN/OUT
  out     output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type
ipl_stitchFrames
(
    ipl_image_type **in_img_array, 
    ipl_image_type * out_img_ptr, 
    int hdiff, 
    int vdiff, 
    int blendWidth,
    int smartSeam
) 
{
  ipl_rect_type copy, paste;
  int numFrames;
  int i, dx, dy;
  int hdiffFromEnd;


  // lets see how many input images we were provided
  numFrames = 0;
  while (in_img_array[numFrames] != NULL && numFrames <= 3)
    numFrames++;

  if (IPL_COMPOSE_DEBUG)  //lint !e774 !e506
    printf("%d frames to stitch\n", numFrames); //lint !e534

  if ((numFrames == 4) || (numFrames < 1))
    return IPL_FAILURE;

  // make sure all images are same type and dimensions
  for (i = 0; i < numFrames; i++)
  {
    if (!(in_img_array[i]->cFormat == IPL_YCbCr ||
          in_img_array[i]->cFormat == IPL_YCrCb420_LINE_PK))
      return IPL_FAILURE;

    if ((in_img_array[0]->cFormat != in_img_array[i]->cFormat)  ||
        (in_img_array[0]->dx      != in_img_array[i]->dx)       ||
        (in_img_array[0]->dy      != in_img_array[i]->dy)) 
    return IPL_FAILURE;
  }

  // make sure output matches input
  if ((in_img_array[0]->cFormat != out_img_ptr->cFormat)  ||
      (in_img_array[0]->dy      != out_img_ptr->dy))       
    return IPL_FAILURE;

  dx = in_img_array[0]->dx;
  dy = in_img_array[0]->dy;

  // strategically place right image
  copy.x = 0;
  copy.y = 0;
  copy.dx = dx;
  copy.dy = dy;

  paste.x = 0;
  paste.y = 0;
  paste.dx = dx;
  paste.dy = dy;


  // copy the first image to the output
  if (IPL_COMPOSE_DEBUG) 
    printf("%d image going (%d,%d) %dx%d\n", 0, (int)paste.x, (int)paste.y,
       (int)paste.dx, (int)paste.dy);

  if (ipl_copy_and_paste(in_img_array[0], out_img_ptr, &copy, &paste) != IPL_SUCCESS)
  {
    if (IPL_COMPOSE_DEBUG) printf("Could not copy first image to output\n"); 
    return IPL_FAILURE;
  }

  hdiffFromEnd = dx - hdiff;
 

  // now copy all subsequnt images
  for (i = 1; i < numFrames; i++)
  {
    paste.x = (i-1)*hdiff + dx - hdiffFromEnd;
    paste.y = 0;

    if (IPL_COMPOSE_DEBUG) 
      printf("%d image going (%d,%d) %dx%d\n", i, (int)paste.x, (int)paste.y,
         (int)paste.dx, (int)paste.dy);

    if (ipl_copy_and_paste_blend(in_img_array[i], out_img_ptr, &copy, &paste, blendWidth, 3))
    {
      if (IPL_COMPOSE_DEBUG) printf("Could not copy image %d to final\n", i); 
        return IPL_FAILURE;
    }
  }

  return IPL_SUCCESS;
} 


/*lint -restore */

