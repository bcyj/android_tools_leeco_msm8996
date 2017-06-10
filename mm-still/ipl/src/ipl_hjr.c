/*===========================================================================

        I M A G E  P R O C E S S I N G   L I B R A R Y    F I L E

DESCRIPTION
  This file contains the implementation of the IPL's hand jitter reduction

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
===========================================================================*/


/*===========================================================================
                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_hjr.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
 3/15/06   babakf  Created

===========================================================================*/


/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipl_types.h"
#include "ipl_helper.h"
#include "ipl_compose.h"
#include "ipl_qvp.h"

// Maxium number of images HJR currently supports
#define HJR_MAX_IMAGES  10
#define HJR_DEBUG       0



/*lint -save -e713, until Systems approves ths code, dont lint yet */
/*lint -save -e530, */
/*lint -save -e534, */
/*lint -save -e737, */
/*lint -save -e702, */
/*lint -save -e704, */
/*lint -save -e732, */
/*lint -save -e734, */
/*lint -save -e818, */
/*lint -save -e725, */
/*lint -save -e771, */
/*lint -save -e774, */

/*===========================================================================
                        FUNCTION DECLARATIONS
===========================================================================*/


/* <EJECT> */
/*===========================================================================
===========================================================================*/
static ipl_status_type hjr_compute_rcs(ipl_image_type * in, 
    int32 *row_sum, int32 *col_sum)
{
  uint8 *ptr;
  uint8 scale_row, scale_col,tmp;
  int32 i,j;
  int32 sum_col; 
  int32 *sum_row;
  int32 *sum_row_orig;
  int32 width, height;


  width = in->dx;
  height = in->dy;

  sum_row = ipl_malloc(width*sizeof(int32));
  if (sum_row == NULL)
    return IPL_NO_MEMORY;
  sum_row_orig = sum_row;

  if (height < 256)
    scale_row = 0;
  else if (height < 512)
    scale_row = 1;
  else if (height < 1024)
    scale_row = 2;
  else if (height < 2048)
    scale_row = 3;
  else 
    scale_row = 4;

  if (width < 256)
    scale_col = 0;
  else if (width < 512)
    scale_col = 1;
  else if (width < 1024)
    scale_col = 2;
  else if (width < 2048)
    scale_col = 3;
  else 
    scale_col = 4;
    
  for (i = 0; i < width; i++)
    sum_row[i] = 0;

  ptr = in->imgPtr;
  for (i = 0; i < height; i++)
  {
    sum_col = 0;
    for (j = 0; j < width; j++)       
    {
      tmp = *ptr++;
      sum_col = sum_col + tmp;
      *sum_row = *sum_row + tmp;
      sum_row++;
    }
    *col_sum++ = (sum_col>>scale_col);
    sum_row = sum_row - width;
  }

  for (i = 0; i < width; i++)
    *row_sum++ = ((*sum_row++)>>scale_row);

  ipl_sys_free(sum_row_orig);

  return IPL_SUCCESS;
}


/* <EJECT> */
/*===========================================================================
===========================================================================*/
static int hjr_corr1D(int32 *sum1, int32 *sum2, int32 len, int32 max_lag)
{
  uint8 scale;
  int32 i,j; 
  int32 N, ind1, ind2;
  int32 max_ind, tmp16;
  int32 *ptr1;
  int32 *ptr2;
  int32 *corr;
  int32 tmp;

  corr = ipl_malloc((2*max_lag+1)*sizeof(int32));
  if (corr == NULL)
    return 0;

  if (len < 256)
    scale = 4;
  else if (len < 512)
    scale = 5;
  else if (len < 1024)
    scale = 6;
  else if (len < 2048)
    scale = 7;
  else 
    scale = 8;

  N = len - max_lag;
  ind1 = 0; 
  ind2 = max_lag;

  for (i = 0; i < (int32) (2*(int32)max_lag+1); i++)
  {
    ptr1 = sum1 + ind1;
    ptr2 = sum2 + ind2;

    tmp = 0;
    for (j = 0; j < N; j++)
    {
      tmp16 = (*ptr1++) - (*ptr2++);
      tmp = tmp + (IPL_ABS(tmp16) >> scale); //lint !e704
    }
    corr[i] = tmp;

    if (ind1 < ind2)
      ind2--;
    else
      ind1++;
  }

  tmp = corr[0];
  max_ind =0;

  for (i = 1; i < (int32) (2*(int32)max_lag+1); i++)
  {
    if (tmp > corr[i])
    {
      tmp = corr[i];
      max_ind = i;
    }
  }
  ipl_sys_free (corr);
  return (max_ind - max_lag);

}



/* <EJECT> */
/*===========================================================================
===========================================================================*/
static 
ipl_status_type hjr_add_frames(ipl_image_type * in1,
    ipl_image_type * in2, 
    ipl_image_type * out, 
    int * jitter, 
    int cropBorder,
    int32 scaleQ1)
{
  uint8 *ptr1;
  uint8 *ptr2;
  uint8 *ptr3;
  int32 tmp;
  int32 i,j, ch_width, ch_height;
  int x_corr;
  int y_corr;
  int32 width, height;
  int delta;


  // make sure all images are same type and dimensions
  if ((in1->cFormat != IPL_YCrCb420_LINE_PK) &&
      (in2->cFormat != IPL_YCrCb420_LINE_PK) &&
      (out->cFormat != IPL_YCrCb420_LINE_PK))
    return IPL_FAILURE;

  if ((in1->dx != in2->dx) ||
      (in1->dx != out->dx) ||
      (in1->dy != in2->dy) ||
      (in1->dy != out->dy))
    return IPL_FAILURE;

  //
  // Do Luma First
  //
  x_corr = (*jitter/2)*2;
  y_corr = (*(jitter+1)/2)*2;

  // do some sanity checks
  width = in1->dx;
  height = in1->dy;

  if (abs(x_corr) > width || abs(y_corr) > height)
  {
    memcpy(out->imgPtr, in1->imgPtr, width*height);
    memcpy(out->clrPtr, in1->clrPtr, width*height/2);
    return IPL_FAILURE;
  }


  // make it even
  cropBorder = ((cropBorder+1)/2)*2;

  if (HJR_DEBUG)  //lint !e774 !e506
  {
    printf("hjr_add_frames w/ jitter of %d, %d\n",x_corr,y_corr);//lint !e534
    printf("cropBorder of %d\n", cropBorder); //lint !e534
  }

  if (y_corr >= 0)
  {
    if (x_corr >= 0)
    {
      ptr1 = in1->imgPtr;
      ptr2 = in2->imgPtr + x_corr + width * y_corr;

      ptr3 = out->imgPtr;
    }
    else
    {
      ptr1 = in1->imgPtr + -x_corr; // remember, x_corr is negative
      ptr2 = in2->imgPtr + width * y_corr;

      ptr3 = out->imgPtr + -x_corr;
    }
  }
  else
  {
    if (x_corr >= 0)
    {
      ptr1 = in1->imgPtr + width * -y_corr;
      ptr2 = in2->imgPtr + x_corr;

      ptr3 = out->imgPtr + width * -y_corr;
    }
    else
    {
      ptr1 = in1->imgPtr + (-x_corr + width * -y_corr);
      ptr2 = in2->imgPtr;

      ptr3 = out->imgPtr + (-x_corr + width * -y_corr);
    }
  }

  // add ofset for clipping
  delta = (cropBorder + (cropBorder * width));
  ptr1 += delta;
  ptr2 += delta;

  // do the addition
  for (i = 0; i < (height - IPL_ABS(y_corr)) - (2*cropBorder); i++)
  {
    for (j = 0; j < (width - IPL_ABS(x_corr)) - (2*cropBorder); j++)
    {
      tmp = (*ptr1++) * scaleQ1 + (*ptr2++) * (256-scaleQ1);
      tmp = (tmp+128) >> 8; //lint !e704
      tmp = CLIPIT(tmp); 
      *ptr3++ = (uint8) tmp;
    }
    ptr1 += (IPL_ABS(x_corr) + 2*cropBorder);
    ptr2 += (IPL_ABS(x_corr) + 2*cropBorder);
    ptr3 += (IPL_ABS(x_corr) + 2*cropBorder);
  }



  //
  //
  // Now do Chroma
  //
  //
  ch_width = width;
  ch_height = height;

  if ((in1->cFormat == IPL_YCbCr420_LINE_PK) ||
      (in1->cFormat == IPL_YCrCb420_LINE_PK))
  {
    x_corr /= 2;
    ch_width /= 2;

    y_corr /= 2;
    ch_height /= 2;

    delta = (cropBorder + (cropBorder * ch_width));
  }
  else if ((in1->cFormat == IPL_YCrCb422_LINE_PK) ||
           (in1->cFormat == IPL_YCbCr422_LINE_PK))
  {
    x_corr /= 2;
    ch_width /= 2;

    delta = (cropBorder + (cropBorder * ch_width));
  }
  else if ((in1->cFormat == IPL_YCbCr444_LINE_PK) ||
           (in1->cFormat == IPL_YCrCb444_LINE_PK))
  {
    delta = (cropBorder + (cropBorder * ch_width));
  }


  if (y_corr >= 0)
  {
    if (x_corr >= 0)
    {
      ptr1 = in1->clrPtr;
      ptr2 = in2->clrPtr + 2 * (x_corr + ch_width * y_corr);

      ptr3 = out->clrPtr;
    }
    else
    {
      ptr1 = in1->clrPtr + -2 * x_corr;
      ptr2 = in2->clrPtr + 2 * (ch_width * y_corr);

      ptr3 = out->clrPtr + -2 * x_corr;
    }
  }
  else
  {
    if (x_corr >= 0)
    {
      ptr1 = in1->clrPtr + 2 * ch_width * -y_corr; 
      ptr2 = in2->clrPtr + 2 * x_corr;

      ptr3 = out->clrPtr + 2 * ch_width * -y_corr; 
    }
    else
    {
      ptr1 = in1->clrPtr + (2 * (-x_corr + ch_width * -y_corr));
      ptr2 = in2->clrPtr;

      ptr3 = out->clrPtr + (2 * (-x_corr + ch_width * -y_corr));
    }
  }

  // add ofset for clipping
  ptr1 += delta;
  ptr2 += delta;

  // do the addition
  for (i = 0; i < (ch_height - (IPL_ABS(y_corr))) - 2*cropBorder; i++)
  {
    for (j = 0; j < (ch_width - (IPL_ABS(x_corr))) - 2*cropBorder; j++)
    {
      tmp = (*ptr1++ -128)*scaleQ1 + (*ptr2++ -128)*(256-scaleQ1);
      tmp = ((tmp+128)>>8)+128; //lint !e704
      tmp = CLIPIT(tmp); 
      *ptr3++ = (uint8) tmp;

      tmp = (*ptr1++ -128)*scaleQ1 + (*ptr2++ -128)*(256-scaleQ1);
      tmp = ((tmp+128)>>8)+128; //lint !e704
      tmp = CLIPIT(tmp);
      *ptr3++ = (uint8) tmp;
    }
         
    ptr1 += (2 * (IPL_ABS(x_corr) + 2*cropBorder));
    ptr2 += (2 * (IPL_ABS(x_corr) + 2*cropBorder));
    ptr3 += (2 * (IPL_ABS(x_corr) + 2*cropBorder));
  }

  return IPL_SUCCESS;
}





/* <EJECT> */
/*===========================================================================
===========================================================================*/
static
ipl_status_type hjr_add_frames_strips(ipl_image_type * in1,
    ipl_image_type * in2, 
    ipl_image_type * out, 
    int * jitter, 
    int cropBorder,
    int32 scaleQ1)
{
  uint8 * origImgPtr[HJR_MAX_IMAGES];
  uint8 * origClrPtr[HJR_MAX_IMAGES];
  ipl_image_type buffer[HJR_MAX_IMAGES];
  int32 tmp;
  int32 i,j;
  int x_corr;
  int y_corr;
  int dx, dy, bdy;
  int sSize, maxStrips, strip;
  //char fname[40];
  uint8 * ptr0;
  uint8 * ptr1;
  uint8 * ptr2;
  int32 hjr_strip_size;

  hjr_strip_size = 176*144*2;

  // make sure we can even get the extra memory we need
  ptr0 = ipl_malloc(hjr_strip_size);
  if (ptr0 == NULL)
  {
    IPL2_MSG_MED("### IPL HJR: cannot get memory we need 1 ###\n");
    return IPL_NO_MEMORY;
  }
  ipl_sys_free(ptr0);


  // make sure all images are same type and dimensions
  if ((in1->cFormat != IPL_YCrCb420_LINE_PK) &&
      (in2->cFormat != IPL_YCrCb420_LINE_PK) &&
      (out->cFormat != IPL_YCrCb420_LINE_PK))
    return IPL_FAILURE;

  if ((in1->dx != in2->dx) ||
      (in1->dx != out->dx) ||
      (in1->dy != in2->dy) ||
      (in1->dy != out->dy))
    return IPL_FAILURE;

  //
  // Do Luma First
  //
  x_corr = (*jitter/2)*2;
  y_corr = (*(jitter+1)/2)*2;

  // make it even
  cropBorder = ((cropBorder+1)/2)*2;

  if (HJR_DEBUG)  //lint !e774 !e506
  {
    printf("hjr_add_frames w/ jitter of %d, %d\n",x_corr,y_corr);//lint !e534
    printf("cropBorder of %d\n", cropBorder); //lint !e534
  }

  dx = in1->dx;
  dy = in1->dy;

  // do some sanity checks
  if (abs(x_corr) > dx || abs(y_corr) > dy)
  {
    memcpy(out->imgPtr, in1->imgPtr, dx*dy);
    memcpy(out->clrPtr, in1->clrPtr, dx*dy/2);
    return IPL_FAILURE;
  }

  // use strips 320x480 big
  bdy = (hjr_strip_size/2)/dx;
  if (bdy == 0)
  {
    // too little memory to do stripping, use convensional method
    //return(hjr_add_frames(in1, in2, out, jitter, 2, scaleQ1));
    IPL2_MSG_MED("### IPL HJR: cannot get memory we need 2 ###\n");
    return IPL_NO_MEMORY;
  }

  sSize = dx*bdy;
  maxStrips = (dy - abs(y_corr))/ bdy; 

  // if we are tryin to use strips on an area smaller than our scratch
  // buffer, then just forward this call the function that handles
  // adding frmes without buffer
  if (maxStrips == 0)
  {
    return(hjr_add_frames(in1, in2, out, jitter, 2, scaleQ1));
  }


  IPL2_MSG_MED("### IPL HJR: Setting up buffers ###\n");

  // setup
  buffer[0].dx = dx;
  buffer[0].dy = bdy;
  buffer[0].cFormat = in1->cFormat;
  if (ipl_malloc_img(&buffer[0]) != IPL_SUCCESS)
  {
    IPL2_MSG_MED("### IPL HJR: out of memory! ###\n");
    return IPL_FAILURE;
  }

  buffer[1].dx = dx;
  buffer[1].dy = bdy;
  buffer[1].cFormat = in2->cFormat;
  if (ipl_malloc_img(&buffer[1]) != IPL_SUCCESS)
  {
    ipl_free_img(&buffer[0]);
    IPL2_MSG_MED("### IPL HJR: out of memory! ###\n");
    return IPL_FAILURE;
  }


  if (y_corr >= 0)
  {
    if (x_corr >= 0)
    {
      origImgPtr[0] = in1->imgPtr;
      origImgPtr[1] = in2->imgPtr + dx * y_corr;
      origImgPtr[2] = out->imgPtr;


      origClrPtr[0] = in1->clrPtr;
      origClrPtr[1] = in2->clrPtr + dx * y_corr /2;
      origClrPtr[2] = out->clrPtr;

    }
    else
    {
      origImgPtr[0] = in1->imgPtr ; // remember, x_corr is negative
      origImgPtr[1] = in2->imgPtr + dx * y_corr;
      origImgPtr[2] = out->imgPtr ;

      origClrPtr[0] = in1->clrPtr ; // remember, x_corr is negative
      origClrPtr[1] = in2->clrPtr + dx * y_corr /2;
      origClrPtr[2] = out->clrPtr ;
    }
  }
  else
  {
    if (x_corr >= 0)
    {
      // y_corr is -, so - - is positive
      origImgPtr[0] = in1->imgPtr + dx * -y_corr;
      origImgPtr[1] = in2->imgPtr ;
      origImgPtr[2] = out->imgPtr + dx * -y_corr;

      origClrPtr[0] = in1->clrPtr + dx * -y_corr /2;
      origClrPtr[1] = in2->clrPtr ;
      origClrPtr[2] = out->clrPtr + dx * -y_corr /2;
    }
    else
    {
      // y_corr is -, so - - is positive
      origImgPtr[0] = in1->imgPtr + (dx * -y_corr);
      origImgPtr[1] = in2->imgPtr;
      origImgPtr[2] = out->imgPtr + (dx * -y_corr);

      origClrPtr[0] = in1->clrPtr + (dx * -y_corr) /2;
      origClrPtr[1] = in2->clrPtr;
      origClrPtr[2] = out->clrPtr + (dx * -y_corr) /2;
    }
  }

  IPL2_MSG_MED("### IPL HJR: adding images luma ###\n");
  for (strip = 0; strip < maxStrips; strip++)
  {
    //IPL2_MSG_MED("### IPL HJR: %d/%d copying %d lines luma ###\n", strip, maxStrips,buffer[0].dy);

    memcpy(buffer[0].imgPtr, origImgPtr[0] + sSize*strip,sSize);
    memcpy(buffer[1].imgPtr, origImgPtr[1] + sSize*strip,sSize);

    if (HJR_DEBUG) //lint !e506 !e774 
    {
      memcpy(buffer[0].clrPtr, origClrPtr[0] + sSize*strip/2,sSize/2);
      memcpy(buffer[1].clrPtr, origClrPtr[1] + sSize*strip/2,sSize/2);

      //sprintf(fname, "buffer_f%02ds%02d_%dx_%d.ycrcb420lp",0,strip,dx,bdy);
      //ipl_debug_write_tile(&buffer[0], NULL, fname);

      //sprintf(fname, "buffer_f%02ds%02d_%dx_%d.ycrcb420lp",1,strip,dx,bdy);
      //ipl_debug_write_tile(&buffer[1], NULL, fname);
    }

    ptr0 = buffer[0].imgPtr;
    ptr1 = buffer[1].imgPtr;

    if (x_corr >= 0)
      ptr1 += x_corr;
    else
      ptr0 += (-x_corr);

    ptr2 = ptr0;

    // do the addition
    cropBorder = 0;
    //for (i = 0; i < (bdy - IPL_ABS(y_corr)) - (2*cropBorder); i++)
    for (i = 0; i < bdy; i++)
    {
      for (j = 0; j < (dx - IPL_ABS(x_corr)) - (2*cropBorder); j++)
      {
        tmp = (*ptr0++) * scaleQ1 + (*ptr1++) * (256-scaleQ1);
        tmp = (tmp+128) >> 8; //lint !e704
        tmp = CLIPIT(tmp); 
        *ptr2++ = (uint8) tmp;
      }

      ptr0 += (IPL_ABS(x_corr) + 2*cropBorder);
      ptr1 += (IPL_ABS(x_corr) + 2*cropBorder);
      ptr2 += (IPL_ABS(x_corr) + 2*cropBorder);
    }


    // now, copy this buffered result strip to where it belongs
    memcpy(origImgPtr[2] + sSize*strip,   buffer[0].imgPtr, sSize);
  }

  // now do chroma
  IPL2_MSG_MED("### IPL HJR: adding images chroma ###\n");
  for (strip = 0; strip < maxStrips; strip++)
  {
    //IPL2_MSG_MED("### IPL HJR: %d/%d copying %d lines chroma ###\n", strip, maxStrips,buffer[0].dy);

    memcpy(buffer[0].clrPtr, origClrPtr[0] + sSize*strip/2,sSize/2);
    memcpy(buffer[1].clrPtr, origClrPtr[1] + sSize*strip/2,sSize/2);

    ptr0 = buffer[0].clrPtr;
    ptr1 = buffer[1].clrPtr;

    if (x_corr >= 0)
      ptr1 += x_corr;
    else
      ptr0 += (-x_corr);

    ptr2 = ptr0;

    // do the addition
    cropBorder = 0;
    //for (i = 0; i < (bdy - IPL_ABS(y_corr)) - (2*cropBorder); i++)
    for (i = 0; i < bdy/2; i++)
    {
      for (j = 0; j < (dx - IPL_ABS(x_corr)) - (2*cropBorder); j++)
      {
        tmp = (*ptr0++ -128)*scaleQ1 + (*ptr1++ -128)*(256-scaleQ1);
        tmp = ((tmp+128)>>8)+128; //lint !e704
        tmp = CLIPIT(tmp); 
        *ptr2++ = (uint8) tmp;
      }
         
      ptr0 += (1 * (IPL_ABS(x_corr) + 2*cropBorder));
      ptr1 += (1 * (IPL_ABS(x_corr) + 2*cropBorder));
      ptr2 += (1 * (IPL_ABS(x_corr) + 2*cropBorder));
    }


    // now, copy this buffered result strip to where it belongs
    memcpy(origClrPtr[2] + sSize*strip/2, buffer[0].clrPtr, sSize/2);
  }


  // sys_free up our strips
  ipl_free_img(&buffer[0]);
  ipl_free_img(&buffer[1]);
 

  return IPL_SUCCESS;
}




/* <EJECT> */
/*===========================================================================
===========================================================================*/
ipl_status_type hjr_correlate_frames(ipl_image_type * in1, 
    ipl_image_type * in2, int searchX, int searchY, int * jitter)
{
  int32 *row_sum1;
  int32 *row_sum2;
  int32 *col_sum1;
  int32 *col_sum2;
  ipl_status_type retval = IPL_SUCCESS;

  row_sum1 = ipl_malloc(in1->dx * sizeof(int32));
  if (!row_sum1)
    return IPL_NO_MEMORY;
 
  row_sum2 = ipl_malloc(in2->dx * sizeof(int32));
  if (!row_sum2)
  { 
    ipl_sys_free(row_sum1);
    return IPL_NO_MEMORY;
  }

  col_sum1 = ipl_malloc(in1->dy * sizeof(int32));
  if (!col_sum1)
  { 
    ipl_sys_free(row_sum1);
    ipl_sys_free(row_sum2);
    return IPL_NO_MEMORY;
  }

  col_sum2 = ipl_malloc(in2->dy * sizeof(int32));
  if (!col_sum2)
  { 
    ipl_sys_free(row_sum1);
    ipl_sys_free(row_sum2);
    ipl_sys_free(col_sum1);
    return IPL_NO_MEMORY;
  }


  retval = hjr_compute_rcs(in1, row_sum1, col_sum1);
  retval = hjr_compute_rcs(in2, row_sum2, col_sum2);

  *jitter     = -hjr_corr1D(row_sum1, row_sum2, in1->dx, (int32) searchX);
  *(jitter+1) = -hjr_corr1D(col_sum1, col_sum2, in1->dy, (int32) searchY);

  ipl_sys_free(row_sum1);
  ipl_sys_free(row_sum2);
  ipl_sys_free(col_sum1);
  ipl_sys_free(col_sum2);

  return retval;
}





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_shjr

DESCRIPTION

  This function will perform hand jitter reduction on a series of images.
  Input and output image sizes must be equal and of the color format.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_array   points to an array of input images the first input image
  oout_img_ptr points to the output image
  searchX      how much to look in X dim before quitting registration
  searchY      how much to look in Y dim before quitting registration

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_shjr
(
  ipl_image_type** in_img_array,    /* Points to the input imageS */
  ipl_image_type* out_img,          /* Points to the output image */
  int searchX,
  int searchY,
  int strips
)
{//lint !e508
  int i, j;
  int dx, dy, bdy;
  int numFrames;
  int jitter[HJR_MAX_IMAGES][2];
  int blendWeights[HJR_MAX_IMAGES] = {128,128,170,170,170,170,170,170,170,170};
  ipl_image_type * out_img_ptr;
  int sSize;
  int strip;
  int maxStrips;

  ipl_image_type buffer[HJR_MAX_IMAGES];
  uint8 * origImgPtr[HJR_MAX_IMAGES];
  uint8 * origClrPtr[HJR_MAX_IMAGES];

  //int32 *row_sum[HJR_MAX_IMAGES];
  //int32 *col_sum[HJR_MAX_IMAGES];
  //int32 *row_sumSTR;
  //int32 *col_sumSTR;
  int32 *tmp;
  //char fname[30];
  int32 hjr_strip_size;
  ipl_status_type retval;

#define cleanUP() \
    for (i = 0; i < numFrames; i++) \
    { \
      ipl_free_img(&buffer[i]); \
      ipl_sys_free(row_sum[i]); \
      ipl_sys_free(col_sum[i]); \
    } 


#define cleanUP_2() \
    for (i = 0; i < numFrames; i++) \
    { \
      ipl_free_img(&buffer[i]); \
      ipl_sys_free(row_sum[i]); \
      ipl_sys_free(col_sum[i]); \
    } \
    ipl_sys_free(row_sumSTR); \
    ipl_sys_free(col_sumSTR); \





  IPL2_MSG_MED("### IPL HJR MARKER 0 ###\n");

  hjr_strip_size = 176*144*2;

  // lets see how many input images we were provided
  numFrames = 0;
  while (in_img_array[numFrames] != NULL && numFrames <= HJR_MAX_IMAGES)
    numFrames++;

  if (HJR_DEBUG)  //lint !e774 !e506
    printf("%d frames to hjr\n", numFrames); //lint !e534

  if ((numFrames >= HJR_MAX_IMAGES+1) || (numFrames < 2))
    return IPL_FAILURE;

  // if the output image is null, then use the first input image as 
  // the location to write final result
  if (out_img == NULL)
  {
    out_img_ptr = in_img_array[0];
  }
  else
  {
    out_img_ptr = out_img;
    if (ipl_copy_and_paste(in_img_array[0], out_img, NULL, NULL) == IPL_FAILURE)
      return IPL_FAILURE;
  }

  // optimzied version only support 420 line pack
  if (in_img_array[0]->cFormat != IPL_YCrCb420_LINE_PK)
    return IPL_FAILURE;

  // make sure all images are same type and dimensions
  for (i = 0; i < numFrames; i++)
  {
    if ((in_img_array[i]->cFormat != IPL_YCrCb420_LINE_PK) &&
        (in_img_array[i]->cFormat != IPL_YCrCb422_LINE_PK) &&
        (in_img_array[i]->cFormat != IPL_YCrCb444_LINE_PK))
    return IPL_FAILURE;

    if ((in_img_array[0]->cFormat != in_img_array[i]->cFormat)  ||
        (in_img_array[0]->dx      != in_img_array[i]->dx)       ||
        (in_img_array[0]->dy      != in_img_array[i]->dy)) 
    return IPL_FAILURE;

    // initialize jitter
    jitter[i][0] = 0;
    jitter[i][1] = 0;
  }

  // make sure output matches input
  if ((in_img_array[0]->cFormat != out_img_ptr->cFormat)  ||
      (in_img_array[0]->dx      != out_img_ptr->dx)       ||
      (in_img_array[0]->dy      != out_img_ptr->dy)) 
    return IPL_FAILURE;

  if ((out_img_ptr->cFormat != IPL_YCrCb420_LINE_PK) &&
      (out_img_ptr->cFormat != IPL_YCrCb422_LINE_PK) &&
      (out_img_ptr->cFormat != IPL_YCrCb444_LINE_PK))
  return IPL_FAILURE;

  dx = in_img_array[0]->dx;
  dy = in_img_array[0]->dy;


  IPL2_MSG_MED("### IPL HJR MARKER 1 ###\n");

  // use strips 640x480 big
  bdy = (hjr_strip_size/numFrames)/dx;

  // too little memory to do strpping.
  if (bdy == 0)
  {
    strips = 0;
    IPL2_MSG_MED("### IPL HJR small image, using conv means 0###\n");
    sSize = 0;
    maxStrips = 0; 
  }
  else
  {
    sSize = dx*bdy;
    maxStrips = dy / bdy; 
  }

  if (strips == 1)
  {
    if (hjr_strip_size >= dx*dy || out_img_ptr->cFormat != IPL_YCrCb420_LINE_PK)
    {
      // no point in processing in strips for such a small image, just
      // do it the old fashioned way. Working on strips only wors for 420lp
      strips = 0;
      IPL2_MSG_MED("### IPL HJR small image, using conv means 1###\n");
    }
    else
    {
      // lets make sure we can even get the amount of memory we want
      i = hjr_strip_size + (numFrames+2)*dx*sizeof(int32) + (numFrames+1)*dy*sizeof(int32);
      tmp = ipl_malloc(i);
      if (tmp == NULL)
      {
        IPL2_MSG_MED("### IPL HJR, not enough sys_free memory to use strips.###\n");
        strips = 0;
      }
      ipl_sys_free(tmp);
    }
  }

  // as on optimization in 7K, we need to copy data from non-cacheable memory
  // to cacheable
  if (strips)
  {
    int32 *row_sum[HJR_MAX_IMAGES];
    int32 *col_sum[HJR_MAX_IMAGES];
    int32 *row_sumSTR;
    int32 *col_sumSTR;

    // one time setup!
    for (i = 0; i < numFrames; i++)
    {
      IPL2_MSG_MED("### IPL HJR:%d/%d Setting up buffers ###\n", i,numFrames);

      buffer[i].dx = dx;
      buffer[i].dy = bdy;
      buffer[i].cFormat = in_img_array[0]->cFormat;
      if (ipl_malloc_img(&buffer[i]) != IPL_SUCCESS)
      {
        IPL2_MSG_MED("### IPL HJR: out of memory 0! ###\n");
        cleanUP();
        return IPL_NO_MEMORY;
      }

      // make in_img_array[0] data ptrs now point to cacheable area
      origImgPtr[i] = in_img_array[i]->imgPtr;
      origClrPtr[i] = in_img_array[i]->clrPtr;

      // master 1D projections
      row_sum[i] = ipl_malloc(dx * sizeof(int32));
      if (!row_sum[i])
      {
        IPL2_MSG_MED("### IPL HJR: out of memory 1! ###\n");
        cleanUP();
        return IPL_NO_MEMORY;
      }
      memset(row_sum[i], 0, dx * sizeof(int32));
 
      col_sum[i] = ipl_malloc(dy * sizeof(int32));
      if (!col_sum[i])
      { 
        IPL2_MSG_MED("### IPL HJR: out of memory 2! ###\n");
        cleanUP();
        return IPL_NO_MEMORY;
      }
      memset(col_sum[i], 0, dy * sizeof(int32));
    }

    // stripped 1D projections
    row_sumSTR = ipl_malloc(dx * sizeof(int32));
    if (!row_sumSTR)
    {
      IPL2_MSG_MED("### IPL HJR: out of memory 3! ###\n");
      cleanUP();
      return IPL_NO_MEMORY;
    }
 
    col_sumSTR = ipl_malloc(bdy * sizeof(int32));
    if (!col_sumSTR)
    { 
      IPL2_MSG_MED("### IPL HJR: out of memory 4! ###\n");
      cleanUP_2();
      return IPL_NO_MEMORY;
    }

    for (i = 0; i < numFrames; i++)
    {
      IPL2_MSG_MED("\n### IPL HJR: computing 1D proejctions ###\n");
      for (strip = 0; strip < maxStrips; strip++)
      {
        //IPL2_MSG_MED("### IPL HJR: %d/%d copying %d luma lines ###\n", strip, maxStrips,buffer[i].dy);

        memcpy(buffer[i].imgPtr, origImgPtr[i] + sSize*strip,sSize);
        memcpy(buffer[i].clrPtr, origClrPtr[i] + sSize*strip/2,sSize/2);

        if (HJR_DEBUG) //lint !e506 !e774 
        {
          //sprintf(fname,"buffer_f%02ds%02d_%dx_%d.ycrcb420lp",i,strip,dx,bdy);
          //ipl_debug_write_tile(&buffer[i], NULL, fname);
        }


        memset(row_sumSTR, 0,  dx * sizeof(int32));
        memset(col_sumSTR, 0, bdy * sizeof(int32));

        if (hjr_compute_rcs(&buffer[i], row_sumSTR, col_sumSTR) != IPL_SUCCESS)
        {
          IPL2_MSG_MED("### IPL HJR: crap, what do we do? ###\n");
          cleanUP_2();
          return IPL_FAILURE;
        }

        // add this strip to total 1D projection
        for (j = 0; j < dx; j++)
          row_sum[i][j] += row_sumSTR[j];

        for (j = 0; j < bdy; j++)
          col_sum[i][strip*bdy+j] = col_sumSTR[j];

        //if (HJR_DEBUG)
        //{
        //  printf("row f%d s%d\n", i, strip);
        //  for (j = 1; j < 40; j++)
        //    printf("%d ", row_sumSTR[j]);
        //  printf("\n");

        //  printf("col f%d s%d\n", i, strip);
        //  for (j = 1; j < 40; j++)
        //    printf("%d ", col_sumSTR[j]);
        //  printf("\n\n");
        //}

        //IPL2_MSG_MED("### IPL HJR: done 1D for strip ###\n");
      }
      IPL2_MSG_MED("### IPL HJR: done 1D for strip ###\n");

      // for frames 1 and greater, register with frame 0
      if (i > 0)
      {
        IPL2_MSG_MED("\n### IPL HJR: figuring out regitration ###\n");

        jitter[i][0]= -hjr_corr1D(row_sum[0],row_sum[i],dx, (int32) searchX);
        IPL2_MSG_MED("### IPL HJR: done X reg ###\n");

        jitter[i][1]= -hjr_corr1D(col_sum[0],col_sum[i],dy, (int32) searchY);
        IPL2_MSG_MED("### IPL HJR: done Y reg ###\n");
      }
      else
      {
        jitter[i][0]= 0;
        jitter[i][1]= 0;
      }
    }

  
    IPL2_MSG_MED("### IPL HJR MARKER 2 ###\n");

    if (HJR_DEBUG)  //lint !e774 !e506
    {
      for (i = 0; i < numFrames; i++)
      {
        printf("Jitter %d with first image is (%d,%d)\n",
          i,jitter[i][0],jitter[i][1]); //lint !e534

        //if (0)  //lint !e774 !e506
        //{
        //  printf("row %d\n", i);
        //  for (j = 1; j < 40; j++)
        //    printf("%d ", row_sum[i][j]/1000);
        //  printf("\n");

        //  printf("col %d\n", i);
        //  for (j = 1; j < 40; j++)
        //    printf("%d ", col_sum[i][j]/1000);
        //  printf("\n\n");
        //}
      }
    }


    // sys_free up the memory we dont need
    IPL2_MSG_MED("### IPL HJR MARKER 3 ###\n");
    cleanUP_2();

    

    //jitter[1][0] = -200;
    //jitter[1][1] = 100;
    //jitter[2][0] = 2;
    //jitter[2][1] = -1000;

    //jitter[1][0] = 200;
    //jitter[1][1] = 100;
    //jitter[2][0] = -2;
    //jitter[2][1] = -221000;


    // add all images on top of the first image
    IPL2_MSG_MED("### IPL HJR MARKER 4 ###\n");
    for (i = 1; i < numFrames; i++)
    {
      // we do not support cropping borders here
      retval = hjr_add_frames_strips(out_img_ptr, in_img_array[i], out_img_ptr, 
          jitter[i], 0, (int32) blendWeights[i]);
      
      if (retval == IPL_NO_MEMORY)
      {
        IPL2_MSG_MED("### IPL HJR:cannot add usin strips, do old way\n");

        // since the version with strips did not work, lets try the one
        // that does not require any additionaly memory
        if (hjr_add_frames(out_img_ptr, in_img_array[i], out_img_ptr, 
            jitter[i], 2, (int32) blendWeights[i]) != IPL_SUCCESS)
          return IPL_FAILURE;
      }
      else if (retval != IPL_SUCCESS)
      {
        return IPL_FAILURE;
      }
    }


    // cleanup
    IPL2_MSG_MED("### IPL HJR MARKER 5 ###\n");
  }
  else
  {
    // compute the registration offset for each frame relative to first frame
    for (i = 1; i < numFrames; i++)
    {
      IPL2_MSG_MED("### IPL HJR MARKER a ###\n");
      if (hjr_correlate_frames(in_img_array[0], in_img_array[i], 
          searchX, searchY, &jitter[i][0]) != IPL_SUCCESS)
      return IPL_FAILURE;
    }

    IPL2_MSG_MED("### IPL HJR MARKER 2 ###\n");

    if (HJR_DEBUG)  //lint !e774 !e506
    {
      for (i = 1; i < numFrames; i++)
      {
        printf("Jitter %d with first image is (%d,%d)\n",
           i,jitter[i][0],jitter[i][1]); //lint !e534
      }
    }

    IPL2_MSG_MED("### IPL HJR MARKER 3 ###\n");

    // add all images on top of the first image
    for (i = 1; i < numFrames; i++)
    {
      IPL2_MSG_MED("### IPL HJR MARKER n ###\n");
      if (hjr_add_frames(out_img_ptr, in_img_array[i], out_img_ptr, 
          jitter[i], 2, (int32) blendWeights[i]) != IPL_SUCCESS)
      return IPL_FAILURE;
    }

    IPL2_MSG_MED("### IPL HJR MARKER 4 ###\n");
  }


  IPL2_MSG_MED("### IPL HJR MARKER 6 ###\n");

  return IPL_SUCCESS;
}



