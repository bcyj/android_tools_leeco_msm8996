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

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_xform.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
 2/16/05   bf      Added more input and output type options to ipl_downsize
12/16/05   bf      Added reflection to ipl_rot_add_crop
12/12/05   bf      Made sure resizeFrame draws last colum and last row
10/22/05   bf      Added RGB888 support to ipl_rotate90, reflect, arbitrary
                   rotate.
10/12/05   bf      Added RGB888 support to ipl_upsize and ipl_downsize
09/01/05   bf      Added RGB565 to rotate90
08/12/05   bf      Added ipl_crop_resize_rot function
08/08/05   bf      Update ipl_crop_ycxx_42xlp_to_rgb to use table lookup for
                   color conversion.
07/21/05   bf      Added ipl_resizeFrame to resize image while preserving
                   transparency.
06/16/05   bf      Fixed boundary problem from Bayer to YCbCr downsize_mn
07/16/05   bf      Wrote ipl_resizeFrame
05/26/04   toygar  Wrote bicubic resize
05/25/04   bf      made reflect faster, and in place (if NULL is passed
                   as output image)
04/18/04   bf      are more lenient when deciding to call fast upsize 1.33 
                   or 2x code
04/18/04   bf      fixed bug with downsize. We call downsize_med if resize
                   ratio is a less than 4x.
03/02/04   bf      wrote ipl_crop_downsize_rot to crop, downsize and then
                   rotate in a single pass.
02/28/04   bf      added 422 and 420 line pack support to downsize_mn
02/22/04   bf      added ipl_crop_ycbcr422lp_to_rgb565
02/07/05   bf      fixed bug in ipl_downsize_med which cause last line of
                   9 point sampling method to be one line past image.
12/14/04   bf      added ipl_crop_ycrcb422lp_to_rgb565
12/11/04   bf      Made upsize_qHigh support downsize too, rename to
                   resize_qHigh. Added feature to paste area cropped are 
                   anywere in output.
12/11/04   bf      Add crop on input, and paste to output for
                   ipl_crop_ycrcb422lp_to_rgb565,ipl_crop_ycrcb420lp_to_rgb565
                   ipl_crop_ycbcr420lp_to_rgb666
10/25/04   bf      Added crop parametr to upsize. Made higher quality version.
                   called upsize_qHigh which averages 4 neighbors.
10/05/04   mz      Updated functions APIs. Updated comments to indicate new 
                   support for YCbCr 4:2:0 line packed format. Corrected line 
                   widths and tab spacings.
09/01/04   bf      Created file. See ipl.h for previous history.
===========================================================================*/

/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipl_types.h"
#include "ipl_helper.h"
#include "ipl_xform.h"
#include "ipl_convert.h"
#include "ipl_compose.h"
#include "ipl_qvp.h"
#include "ipl_upSize.h"
#include "ipl_downSize.h"
#include "ipl_rotAddCrop.h"

#define IPL_XFORM_DEBUG 0

/*lint -save -e501, dont worry about casting here */
/*lint -save -e504, all shifts are okay */
/*lint -save -e508, extra extern is okay */
/*lint -save -e525, cleaned up as many indentation probs as I could, these OK*/
/*lint -save -e534, let me call printf in piece, god */
/*lint -save -e539, let me call printf in piece, god */
/*lint -save -e573, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e574, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e613, the few cases of this we have are ok */
/*lint -save -e701, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e702, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e703, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e704, all shifts are okay */
/*lint -save -e713, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e715, okay that bip_ptr not used */
/*lint -save -e725, dont worry about indenting so much */
/*lint -save -e732, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e737, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e734, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e818, lets not worry about const optimization right now */
/*lint -save -e506, constant value boolean is totally okay */
/*lint -save -e774, constant value boolean is totally okay */
/*lint -save -e818, let us not worry about this optimization yet */
/*lint -save -e834, this warning is just dumb */
/*lint -save -e831, this comment repeats earlier warnings */


/*lint -save -e826, we should look into this someday */
/*lint -save -e661, we should look into this someday */
/*lint -save -e662, we should look into this someday */
/*lint -save -e571, these casts are fine */


/*===========================================================================
                      XFORM SPECIFIC TABLES
===========================================================================*/

static uint16 r5xx[256] = {
0, 0, 0, 0, 2048, 2048, 2048, 2048,
2048, 2048, 2048, 2048, 4096, 4096, 4096, 4096,
4096, 4096, 4096, 4096, 6144, 6144, 6144, 6144,
6144, 6144, 6144, 6144, 8192, 8192, 8192, 8192,
8192, 8192, 8192, 8192, 10240, 10240, 10240, 10240,
10240, 10240, 10240, 10240, 12288, 12288, 12288, 12288,
12288, 12288, 12288, 12288, 14336, 14336, 14336, 14336,
14336, 14336, 14336, 14336, 16384, 16384, 16384, 16384,
16384, 16384, 16384, 16384, 18432, 18432, 18432, 18432,
18432, 18432, 18432, 18432, 20480, 20480, 20480, 20480,
20480, 20480, 20480, 20480, 22528, 22528, 22528, 22528,
22528, 22528, 22528, 22528, 24576, 24576, 24576, 24576,
24576, 24576, 24576, 24576, 26624, 26624, 26624, 26624,
26624, 26624, 26624, 26624, 28672, 28672, 28672, 28672,
28672, 28672, 28672, 28672, 30720, 30720, 30720, 30720,
30720, 30720, 30720, 30720, 32768, 32768, 32768, 32768,
32768, 32768, 32768, 32768, 34816, 34816, 34816, 34816,
34816, 34816, 34816, 34816, 36864, 36864, 36864, 36864,
36864, 36864, 36864, 36864, 38912, 38912, 38912, 38912,
38912, 38912, 38912, 38912, 40960, 40960, 40960, 40960,
40960, 40960, 40960, 40960, 43008, 43008, 43008, 43008,
43008, 43008, 43008, 43008, 45056, 45056, 45056, 45056,
45056, 45056, 45056, 45056, 47104, 47104, 47104, 47104,
47104, 47104, 47104, 47104, 49152, 49152, 49152, 49152,
49152, 49152, 49152, 49152, 51200, 51200, 51200, 51200,
51200, 51200, 51200, 51200, 53248, 53248, 53248, 53248,
53248, 53248, 53248, 53248, 55296, 55296, 55296, 55296,
55296, 55296, 55296, 55296, 57344, 57344, 57344, 57344,
57344, 57344, 57344, 57344, 59392, 59392, 59392, 59392,
59392, 59392, 59392, 59392, 61440, 61440, 61440, 61440,
61440, 61440, 61440, 61440, 63488, 63488, 63488, 63488,
63488, 63488, 63488, 63488, 63488, 63488, 63488, 63488
};

static uint16 gx6x[256] = {
0, 0, 32, 32, 32, 32, 64, 64,
64, 64, 96, 96, 96, 96, 128, 128,
128, 128, 160, 160, 160, 160, 192, 192,
192, 192, 224, 224, 224, 224, 256, 256,
256, 256, 288, 288, 288, 288, 320, 320,
320, 320, 352, 352, 352, 352, 384, 384,
384, 384, 416, 416, 416, 416, 448, 448,
448, 448, 480, 480, 480, 480, 512, 512,
512, 512, 544, 544, 544, 544, 576, 576,
576, 576, 608, 608, 608, 608, 640, 640,
640, 640, 672, 672, 672, 672, 704, 704,
704, 704, 736, 736, 736, 736, 768, 768,
768, 768, 800, 800, 800, 800, 832, 832,
832, 832, 864, 864, 864, 864, 896, 896,
896, 896, 928, 928, 928, 928, 960, 960,
960, 960, 992, 992, 992, 992, 1024, 1024,
1024, 1024, 1056, 1056, 1056, 1056, 1088, 1088,
1088, 1088, 1120, 1120, 1120, 1120, 1152, 1152,
1152, 1152, 1184, 1184, 1184, 1184, 1216, 1216,
1216, 1216, 1248, 1248, 1248, 1248, 1280, 1280,
1280, 1280, 1312, 1312, 1312, 1312, 1344, 1344,
1344, 1344, 1376, 1376, 1376, 1376, 1408, 1408,
1408, 1408, 1440, 1440, 1440, 1440, 1472, 1472,
1472, 1472, 1504, 1504, 1504, 1504, 1536, 1536,
1536, 1536, 1568, 1568, 1568, 1568, 1600, 1600,
1600, 1600, 1632, 1632, 1632, 1632, 1664, 1664,
1664, 1664, 1696, 1696, 1696, 1696, 1728, 1728,
1728, 1728, 1760, 1760, 1760, 1760, 1792, 1792,
1792, 1792, 1824, 1824, 1824, 1824, 1856, 1856,
1856, 1856, 1888, 1888, 1888, 1888, 1920, 1920,
1920, 1920, 1952, 1952, 1952, 1952, 1984, 1984,
1984, 1984, 2016, 2016, 2016, 2016, 2016, 2016
};

static uint16 bxx5[256] = {
0, 0, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 2, 2, 2, 2,
2, 2, 2, 2, 3, 3, 3, 3,
3, 3, 3, 3, 4, 4, 4, 4,
4, 4, 4, 4, 5, 5, 5, 5,
5, 5, 5, 5, 6, 6, 6, 6,
6, 6, 6, 6, 7, 7, 7, 7,
7, 7, 7, 7, 8, 8, 8, 8,
8, 8, 8, 8, 9, 9, 9, 9,
9, 9, 9, 9, 10, 10, 10, 10,
10, 10, 10, 10, 11, 11, 11, 11,
11, 11, 11, 11, 12, 12, 12, 12,
12, 12, 12, 12, 13, 13, 13, 13,
13, 13, 13, 13, 14, 14, 14, 14,
14, 14, 14, 14, 15, 15, 15, 15,
15, 15, 15, 15, 16, 16, 16, 16,
16, 16, 16, 16, 17, 17, 17, 17,
17, 17, 17, 17, 18, 18, 18, 18,
18, 18, 18, 18, 19, 19, 19, 19,
19, 19, 19, 19, 20, 20, 20, 20,
20, 20, 20, 20, 21, 21, 21, 21,
21, 21, 21, 21, 22, 22, 22, 22,
22, 22, 22, 22, 23, 23, 23, 23,
23, 23, 23, 23, 24, 24, 24, 24,
24, 24, 24, 24, 25, 25, 25, 25,
25, 25, 25, 25, 26, 26, 26, 26,
26, 26, 26, 26, 27, 27, 27, 27,
27, 27, 27, 27, 28, 28, 28, 28,
28, 28, 28, 28, 29, 29, 29, 29,
29, 29, 29, 29, 30, 30, 30, 30,
30, 30, 30, 30, 31, 31, 31, 31,
31, 31, 31, 31, 31, 31, 31, 31
};


static const uint8 biWeights[8][8][4] = {
  {{64, 0, 0, 0}, {56, 0, 0, 8}, {48, 0, 0,16}, {40, 0, 0,24}, 
   {32, 0, 0,32}, {24, 0, 0,40}, {16, 0, 0,48}, { 8, 0, 0,56}},

  {{56, 8, 0, 0}, {49, 7, 1, 7}, {42, 6, 2,14}, {35, 5, 3,21}, 
   {28, 4, 4,28}, {21, 3, 5,35}, {14, 2, 6,42}, { 7, 1, 7,49}}, 

  {{48,16, 0, 0}, {42,14, 2, 6}, {36,12,4 ,12}, {30,10,6 ,18},
   {24, 8, 8,24}, {18, 6,10,30}, {12,4 ,12,36}, { 6, 2,14,42}}, 

  {{40,24,0 ,0 }, {35,21, 3, 5}, {30,18, 6,10}, {25,15, 9,15},
   {20,12,12,20}, {15, 9,15,25}, {10, 6,18,30}, { 5, 3,21,35}}, 

  {{32,32, 0,0 }, {28,28, 4, 4}, {24,24, 8, 8}, {20,20,12,12},
   {16,16,16,16}, {12,12,20,20}, { 8, 8,24,24}, { 4, 4,28,28}}, 

  {{24,40,0 ,0 }, {21,35, 5, 3}, {18,30,10, 6}, {15,25,15, 9}, 
   {12,20,20,12}, { 9,15,25,15}, { 6,10,30,18}, { 3, 5,35,21}}, 

  {{16,48, 0,0 }, {14,42, 6, 2}, {12,36,12, 4}, {10,30,18, 6}, 
   {8 ,24,24,8 }, { 6,18,30,10}, { 4,12,36,12}, { 2, 6,42,14}},

  {{ 8,56, 0,0 }, { 7,49, 7, 1}, { 6,42,14, 2}, { 5,35,21, 3}, 
   { 4,28,28,4 }, { 3,21,35, 5}, { 2,14,42, 6}, { 1,7 ,49, 7}} 
};


/*--------------------------------------------------------------------------
 * lookup table which simplifies the conversion from YCBR to RGB565
 * these tables store computationally intensive intermediate
 * functions. Look at ipl_init_lookup_tables for more .
 * Selection of these smaller tables and co-locating them
 * is to aid better cache performance.
 * --------------------------------------------------------------------------*/
extern int16 ipl2_CrToRTable[];
extern int16 ipl2_CrToGTable[];
extern int16 ipl2_CbToGTable[];
extern int16 ipl2_CbToBTable[];

/*--------------------------------------------------------------------------
 * shifted and quantized values for RGB565/444/666
 * encoded images
 * ------------------------------------------------------------------------*/
extern const uint16 ipl2_r5xx[];
extern const uint16 ipl2_gx6x[];
extern const uint16 ipl2_bxx5[];
extern const uint16 ipl2_r444[];
extern const uint16 ipl2_g444[];
extern const uint16 ipl2_b444[];




/*===========================================================================

                        FUNCTION DECLARATIONS

===========================================================================*/



/*==========================================================================

FUNCTION IPL_ROT90LP

DESCRIPTION
  This function rotates 90 crops YUV420 line pack date


DEPENDENCIES
  None



ARGUMENTS IN
  i_img_ptr points to the input image
  crop is a structure informing ipl how to crop
  rotate is the rotation to do


ARGUMENTS OUT
  o_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  Modifies the output image buffer

==========================================================================*/
static ipl_status_type ipl_rot90lp
(
  ipl_image_type* i_img_ptr,    /* Input Image Pointer      */
  ipl_image_type* o_img_ptr,    /* Output Image Pointer      */
  ipl_rect_type* icrop          /* cropping params */
)
{
  register unsigned char* inputImgPtr  = i_img_ptr->imgPtr;
  register unsigned char* outputImgPtr = o_img_ptr->imgPtr;
  register uint8 cb,cr;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 rowSize = o_img_ptr->dx;
  uint8 *cr_ptr;
  uint8 *out_cr_ptr;
  uint32 rowInc = 0, destInc, src_index;
  uint32 row, col;
  uint32 render_col_size;
  uint32 render_row_size;
  ipl_rect_type crop;

  
  MSG_LOW("inside ipl_rot90lp\n");

  if (icrop == NULL)
  {
    crop.x  = 0;
    crop.y  = 0;
    crop.dx = o_img_ptr->dx;
    crop.dy = o_img_ptr->dy;
  }
  else
  {
    crop.x  = icrop->x;
    crop.y  = icrop->y;
    crop.dx = icrop->dx;
    crop.dy = icrop->dy;
  }

  render_col_size = (i_img_ptr->dy < crop.dx) ? i_img_ptr->dy : crop.dx;
  render_col_size &= (~0x1);

  render_row_size = (i_img_ptr->dx < crop.dy) ? i_img_ptr->dx : crop.dy;
  render_row_size &= (~0x1);
  
  
  /*------------------------------------------------------------------------
    90 degree rotation needs starting the image from the same side  corner
    where the image starts and wroking upwards. Same with a 2 col offset
    and so on.

     inputImgPtr
     |
     |
     ^__________________________
     |                         |
   ^ |                         |
   | |                         |
   | |                         |
   | |_________________________|
     ^
     | ----> Go this way for rows
     |
   Offset it into the array to start
   picking Y values from here backwards on the same col
   as shown in the arrow leftside

   When offsetted by src_index ( = end of frame - row size )
      the inputImgPtr takes us to this location.
  ------------------------------------------------------------------------*/

  src_index =  i_img_ptr->dx * ( i_img_ptr->dy - 1);
  inputImgPtr  += src_index;

  destInc = (o_img_ptr->dx - crop.dx);
  rowInc = (i_img_ptr->dy - crop.dx);

  out_cr_ptr = o_img_ptr->clrPtr;
  outputImgPtr += crop.x + o_img_ptr->dx*crop.y;
  
  
  if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    cr_ptr = i_img_ptr->clrPtr + ((src_index - i_img_ptr->dx) >> 1);

    for (row = render_row_size/2 ; row; row--)
    {
      for (col = render_col_size/2; col; col--)
      {
        lumaa1 = *( inputImgPtr );
        lumaa3 = *( inputImgPtr + 1 );
  
        inputImgPtr -= input_row_size;
        lumaa2 = *( inputImgPtr );
        lumaa4 = *( inputImgPtr + 1 );
  
        inputImgPtr -= input_row_size;
  
  
        cr = *(cr_ptr);
        cb = *(cr_ptr+1);
        cr_ptr -= input_row_size;
  
        *(outputImgPtr)     = (unsigned char) lumaa1;
        *(outputImgPtr + 1) = (unsigned char) lumaa2;
        *(outputImgPtr++ + rowSize) = (unsigned char) lumaa3;
        *(outputImgPtr++ + rowSize) = (unsigned char) lumaa4;
        *out_cr_ptr++ = (unsigned char) cr;
        *out_cr_ptr++ = (unsigned char) cb;
      }
  
      outputImgPtr =  outputImgPtr + (rowSize) + destInc;
  
  
      inputImgPtr += src_index + 2 - (((rowInc - 1) * input_row_size));
      cr_ptr += ((src_index + input_row_size) >> 1) + 2 -
                ((rowInc * input_row_size ) >> 1 );
    }
  }
  else if (i_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
  {
    cr_ptr = i_img_ptr->clrPtr + src_index;

    // input
    //
    // Y1  Y2                 Y3 Y1
    // Y3  Y4                 Y4 Y2
    //
    // Cr1 Cb1                Cr1 Cb1
    // Cr2 Cb2                Cr2 Cb2
    //
    // do you know of another way? Could average Chromas...
    //
    for (row = render_row_size /2; row; row--)
    {
      for (col = render_col_size /2; col; col--)
      {
        lumaa1 = *(inputImgPtr);
        lumaa3 = *(inputImgPtr + 1);
        inputImgPtr -= input_row_size;

        lumaa2 = *( inputImgPtr );
        lumaa4 = *( inputImgPtr + 1 );
        inputImgPtr -= input_row_size;
  
        *(outputImgPtr)     = (unsigned char) lumaa1;
        *(outputImgPtr + 1) = (unsigned char) lumaa2;
        *(outputImgPtr++ + rowSize) = (unsigned char) lumaa3;
        *(outputImgPtr++ + rowSize) = (unsigned char) lumaa4;

        cr = *(cr_ptr);
        cb = *(cr_ptr+1);
        cr_ptr -= input_row_size;
        *out_cr_ptr = cr;
        *(out_cr_ptr+1) = cb;

        cr = *(cr_ptr);
        cb = *(cr_ptr+1);
        cr_ptr -= input_row_size;
        *(out_cr_ptr+rowSize)   = (unsigned char) cr;
        *(out_cr_ptr+rowSize+1) = (unsigned char) cb;

        out_cr_ptr += 2;

      }
  
      outputImgPtr += (rowSize + destInc);
      out_cr_ptr   += (rowSize + destInc);
      inputImgPtr  += (src_index + 2 - ((rowInc - 1) * input_row_size));
      cr_ptr += (src_index+2 - ((rowInc-1)* input_row_size));
    }
  }
  else
    return( IPL_FAILURE);


  return( IPL_SUCCESS );

} 




/*==========================================================================

FUNCTION IPL_ROT180LP 

DESCRIPTION

  This function rotates 180 and crops YUV420 line pack date.

DEPENDENCIES
  None



ARGUMENTS IN
  i_img_ptr points to the input image
  crop is a structure informing ipl how to crop


ARGUMENTS OUT
  o_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  Modifies the output image buffer

==========================================================================*/
static ipl_status_type ipl_rot180lp
(
  ipl_image_type* i_img_ptr,    /* Input Image Pointer      */
  ipl_image_type* o_img_ptr,    /* Output Image Pointer      */
  ipl_rect_type* icrop          /* cropping params */
)
{
  register unsigned char* inputImgPtr  = i_img_ptr->imgPtr;
  register unsigned char* outputImgPtr = o_img_ptr->imgPtr;
  register uint8 cb,cr;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register uint32 input_row_size = i_img_ptr->dx;
  uint32 rowSize = o_img_ptr->dx;
  uint8 *cr_ptr;
  uint8 *out_cr_ptr;
  uint32 rowInc=0, dest_index, destInc;
  uint32 row, col;
  ipl_rect_type crop;

  MSG_LOW("inside ipl2_Rot180Frame_YCrCb420lpToRGB\n");

  if (icrop == NULL)
  {
    crop.x  = 0;
    crop.y  = 0;
    crop.dx = o_img_ptr->dx;
    crop.dy = o_img_ptr->dy;
  }
  else
  {
    crop.x  = icrop->x;
    crop.y  = icrop->y;
    crop.dx = icrop->dx;
    crop.dy = icrop->dy;
  }


  /*------------------------------------------------------------------------
      180 degree rotation needs starting the image from the extreme corner
      so lets add the whole frame size into the input image ptr and then
      starts decrementing rows

      inputImgPtr
      |
      |
      ^__________________________
      |                         |
      |                         |
      |                         |
      |                         |
      |_______________________|_|
                              |
                              |
                               Offset it into the array to start
                               picking Y values from here backwards
  ------------------------------------------------------------------------*/
  inputImgPtr += i_img_ptr->dx * i_img_ptr->dy  - 1;
  dest_index = (crop.x + o_img_ptr->dx * crop.y);
  outputImgPtr += dest_index;
  destInc = ( o_img_ptr->dx - crop.dx);
  rowInc = (i_img_ptr->dx - crop.dx );
  out_cr_ptr = o_img_ptr->clrPtr;


  if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    cr_ptr = i_img_ptr->clrPtr + ((i_img_ptr->dx * i_img_ptr->dy/2) - 2); 

    for ( row = crop.dy/2; row; row-- )
    {
      for ( col = crop.dx/2; col; col-- )
      {
  
        lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
        lumaa2 = *( inputImgPtr - 1);  /* corresponds to Y2 in the pic */
        lumaa3 = *(inputImgPtr-- - input_row_size);
        lumaa4 = *(inputImgPtr-- - input_row_size);
  
        cr = *(cr_ptr);
        cb = *(cr_ptr+1);
        cr_ptr -= 2;
  
        *(outputImgPtr)     = (unsigned char) lumaa1;
        *(outputImgPtr + 1) = (unsigned char) lumaa2;
        *(outputImgPtr++ + rowSize) = (unsigned char) lumaa3;
        *(outputImgPtr++ + rowSize) = (unsigned char) lumaa4;
        *out_cr_ptr++ = (unsigned char) cr;
        *out_cr_ptr++ = (unsigned char) cb;
      }
  
      outputImgPtr =  (outputImgPtr + (rowSize)+destInc);
      inputImgPtr -= (uint32) (rowInc + input_row_size);
      cr_ptr -= (rowInc);
    }
  }
  else if (i_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
  {
    cr_ptr = i_img_ptr->clrPtr + ((i_img_ptr->dx * i_img_ptr->dy) - 2); 

    for ( row = crop.dy; row; row-- )
    {
      for ( col = crop.dx/2; col; col-- )
      {
        lumaa1 = *inputImgPtr--;  /* corresponds to Y1 in the pic */
        lumaa2 = *inputImgPtr--;  /* corresponds to Y2 in the pic */
  
        cr = *cr_ptr;
        cb = *(cr_ptr+1);
        cr_ptr -= 2;
  
        *outputImgPtr++ = (unsigned char) lumaa1;
        *outputImgPtr++ = (unsigned char) lumaa2;

        *out_cr_ptr++ = (unsigned char) cr;
        *out_cr_ptr++ = (unsigned char) cb;
      }
  
      outputImgPtr += (destInc);
      inputImgPtr  -= (rowInc);
      cr_ptr -= (rowInc);
    }
  }
  else
  {
    return( IPL_FAILURE);
  }


  return( IPL_SUCCESS );

} 



/*==========================================================================

FUNCTION IPL_ROT270LP

DESCRIPTION
  This function rotates 270 and crops and YUV420 lp image

DEPENDENCIES
  None



ARGUMENTS IN
  i_img_ptr points to the input image
  crop is a structure informing ipl how to crop


ARGUMENTS OUT
  o_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  Modifies the output image buffer

==========================================================================*/
static ipl_status_type ipl_rot270lp
(
  ipl_image_type* i_img_ptr,    /* Input Image Pointer      */
  ipl_image_type* o_img_ptr,    /* Output Image Pointer      */
  ipl_rect_type* icrop          /* cropping params */
)
{
  register unsigned char* inputImgPtr  = i_img_ptr->imgPtr;
  register unsigned char* outputImgPtr = o_img_ptr->imgPtr;
  register uint8 cb,cr;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register uint32 input_row_size = i_img_ptr->dx;
  uint32 rowSize = o_img_ptr->dx;
  uint8 *cr_ptr;
  uint8 *out_cr_ptr;
  uint32 destInc, src_index;
  uint32 row, col;
  uint32 render_col_size;
  uint32 render_row_size;
  ipl_rect_type crop;

  MSG_LOW("inside ipl_rot270lp\n");

  if (icrop == NULL)
  {
    crop.x  = 0;
    crop.y  = 0;
    crop.dx = o_img_ptr->dx;
    crop.dy = o_img_ptr->dy;
  }
  else
  {
    crop.x  = icrop->x;
    crop.y  = icrop->y;
    crop.dx = icrop->dx;
    crop.dy = icrop->dy;
  }

  render_col_size = (i_img_ptr->dy < crop.dx) ? i_img_ptr->dy : crop.dx;
  render_col_size &= (~0x1);

  render_row_size = (i_img_ptr->dx < crop.dy) ? i_img_ptr->dx : crop.dy;
  render_row_size &= (~0x1);

  if ( i_img_ptr->dy < crop.dx )
    return(IPL_FAILURE);


  /*------------------------------------------------------------------------
    270 degree rotation needs starting the image from the same side right
    corner from where the image starts and wroking downwards.
    - with a 2 col offset and so on.

      inputImgPtr
     |
     |
     ^__________________________
     |                         |  |
     |                         |  |
     |                         |  |
     |                         |  v  col increment
     |_________________________|
        <-----------------------
         row increment
  ------------------------------------------------------------------------*/
  src_index =  i_img_ptr->dx;
  inputImgPtr  += src_index - 1;
  destInc = (o_img_ptr->dx - crop.dx);
  out_cr_ptr = o_img_ptr->clrPtr;
  outputImgPtr += crop.x + o_img_ptr->dx*crop.y;

  if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    cr_ptr = i_img_ptr->clrPtr + (src_index) - 2; /* end of prev row */;

    for ( row = render_row_size /2 ; row; row-- )
    {
      /*----------------------------------------------------------------------
          We process 2 rows at a time we need half the number of coloumns
          iteratios.
      ----------------------------------------------------------------------*/
      for ( col = render_col_size / 2; col; col-- )
      {
  
        lumaa1 = *( inputImgPtr );
        lumaa3 = *( inputImgPtr - 1 );
        inputImgPtr += input_row_size;
        lumaa2 = *( inputImgPtr );
        lumaa4 = *( inputImgPtr - 1 );
        inputImgPtr += input_row_size;
  
  
        cr = *(cr_ptr);
        cb = *(cr_ptr + 1);
        cr_ptr += input_row_size;
  
  
        *outputImgPtr = (unsigned char) lumaa1;
        *(outputImgPtr + 1) = (unsigned char) lumaa2;
        *(outputImgPtr++ + rowSize) = (unsigned char) lumaa3;
        *(outputImgPtr++ + rowSize) = (unsigned char) lumaa4;

        *out_cr_ptr++ = (unsigned char) cr;
        *out_cr_ptr++ = (unsigned char) cb;
      } 
      outputImgPtr =  (outputImgPtr + (rowSize)+destInc);
  
      src_index -= 2;
      inputImgPtr = i_img_ptr->imgPtr + src_index  - 1;
      cr_ptr = i_img_ptr->clrPtr + (src_index) - 2; 
  
    } /* end of row loop */
  }
  else if (i_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
  {
    cr_ptr = i_img_ptr->clrPtr + (src_index) - 2; /* end of prev row */;

    for (row = render_row_size / 2; row; row--)
    {
      for (col = render_col_size/2; col; col--)
      {
        // input
        //
        // Y1  Y2                 Y2 Y4
        // Y3  Y4                 Y1 Y3
        //
        // Cr1 Cb1                Cr1 Cb1
        // Cr2 Cb2                Cr2 Cb2
        //
        // do you know of another way? Could average Chromas...
        //
        lumaa1 = *(inputImgPtr);
        lumaa2 = *(inputImgPtr - 1);
        inputImgPtr += input_row_size;

        cr = *(cr_ptr);
        cb = *(cr_ptr + 1);
        cr_ptr += input_row_size;
  
  
        *(outputImgPtr) = (unsigned char) lumaa1;
        *(outputImgPtr + o_img_ptr->dx) = (unsigned char) lumaa2;
        outputImgPtr++;

        *out_cr_ptr = (unsigned char) cr;
        *(out_cr_ptr+1) = (unsigned char) cb;



        lumaa1 = *(inputImgPtr);
        lumaa2 = *(inputImgPtr - 1);
        inputImgPtr += input_row_size;

        cr = *(cr_ptr);
        cb = *(cr_ptr + 1);
        cr_ptr += input_row_size;
  
  
        *(outputImgPtr) = (unsigned char) lumaa1;
        *(outputImgPtr + o_img_ptr->dx) = (unsigned char) lumaa2;
        outputImgPtr++;

        *(out_cr_ptr + rowSize)   = (unsigned char) cr;
        *(out_cr_ptr + rowSize+1) = (unsigned char) cb;
        out_cr_ptr += 2;
      } 
      outputImgPtr += (rowSize + destInc);
      out_cr_ptr   += (rowSize + destInc);

      src_index -= 2;

      inputImgPtr = i_img_ptr->imgPtr + src_index  - 1;
      cr_ptr = i_img_ptr->clrPtr + (src_index) - 2; 
  
    } /* end of row loop */
  }
  else
    return( IPL_FAILURE);


  return( IPL_SUCCESS );

} 








/* <EJECT> */
/*===========================================================================

FUNCTION image_swap_row_cr_cb

DESCRIPTION
  This function is an optimized version to do reflection, rotation, 
  adding of a frame, and color conversion from YCbCr 4:2:2 to RGB565.

  It is assumed that the caller will set the input, frame, and output
  image structures to have the correct dimensions after reflection
  and rotation.

  The frame is assumed to be established in 2 pixel units, i.e. the 
  minimum width of any frame edge is 2 pixels.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr      pointer to the input image
  input_frame_ptr    pointer to the frame image
  transparentValue   16-bit transparent pixel value
  output_img_ptr     pointer to the output image
  startPos           starting position
  colInc             column increment
  rowInc             row increment

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type image_swap_row_cr_cb
( 
  ipl_image_type* i_img_ptr,      /* Points to the input image          */
  ipl_image_type* i_frame_ptr,    /* Points to the output image         */
  uint16 transparentValue,        /* Transparent pixel value            */
  ipl_image_type* o_img_ptr,      /* Points to the output image         */
  int32 startPos,                 /* Starting position                  */
  int32 colInc,                   /* Column increment                   */
  int32 rowInc,                   /* Row increment                      */
  ipl_rect_type* crop             /* Crop region                        */
)
{
  int32 src_index,dest_index;
  int32 frame_index;
  uint32 row,col;
  ipl_image_type* output_img_ptr;
  unsigned char* inputImgPtr;
  unsigned char* outputImgPtr;
  unsigned char* frameImgPtr = NULL;
  unsigned char r1,g1,b1,r2,g2,b2;
  unsigned short out;
  int32 luma1,luma2,cb,cr;
  unsigned char cbb=0,crr=0,lumaa1=0,lumaa2=0,cbb2=0,crr2=0;
  long rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  short ycbcr2rgb_convert[6] = {8, 25803, -3071, -7672, 30399, 12};

  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};
  int32 frameIncr = 0;
  uint32* data_out;
  int32 out32;
  uint16 out2;

  MSG_LOW("image_swap_row_cr_cb marker_0\n");

  if (!i_img_ptr ||   !i_img_ptr->imgPtr  || 
      !o_img_ptr ||   !o_img_ptr->imgPtr  ||
      !crop)
  {
    MSG_LOW("image_swap_row_cr_cb marker_200\n");
    return IPL_FAILURE;
  }

  output_img_ptr = o_img_ptr;
  inputImgPtr=i_img_ptr->imgPtr;
  outputImgPtr = output_img_ptr->imgPtr;
  data_out = (uint32*)o_img_ptr->imgPtr;

  /* Initialize the index to starting position */
  src_index = startPos<<1;  /* byte addressed */
  dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
  frame_index = 0;
  if (i_frame_ptr)
  {
    frameImgPtr = i_frame_ptr->imgPtr;
    frameIncr = (i_frame_ptr->dx - crop->dx)<<1;
  }

  MSG_LOW("image_swap_row_cr_cb marker_1\n");

  if ( output_img_ptr->cFormat == IPL_RGB565 )
  {
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2)
      {
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /* Use frame */
          *((uint16*)(outputImgPtr + dest_index)) = 
              *((uint16*)(frameImgPtr + frame_index));
          *((uint16*)(outputImgPtr + dest_index+2)) = 
              *((uint16*)(frameImgPtr + frame_index+2));
        } 
        else 
        {
          /*
          ** Convert input to rgb
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            crr = crr2;
          } else {
            cbb = cbb2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index+2)) = out;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      frame_index += frameIncr;
    } /* End of row loop */
    }
    else
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to rgb
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            crr = crr2;
          } else {
            cbb = cbb2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index+2)) = out;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        }
        src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
        dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      } /* End of row loop */
    }

    MSG_LOW("image_swap_row_cr_cb marker_100\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_RGB666)
  {
    dest_index = dest_index << 1;
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /*
          ** Convert frame to RGB444
          */
          out = *(uint16*)((uint32)frameImgPtr + frame_index);
          r1 = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r1];
          *((uint32*)((uint32)data_out + dest_index))=out32;
          out = *(uint16*)((uint32)frameImgPtr + frame_index + 2);
          r1 = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r1];
          *((uint32*)((uint32)data_out + dest_index+4))=out32;
        } else {
          /*
          ** Convert input to rgb
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            crr = crr2;
          } else {
            cbb = cbb2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index)) = out32;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index + 4)) = out32;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 8;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 2);
      frame_index += frameIncr;
    } /* End of row loop */
    }
    else
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to rgb
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            crr = crr2;
          } else {
            cbb = cbb2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index)) = out32;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index + 4)) = out32;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 8;
        } /* End of col loop */
        src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
        dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 2);
      }
    }

    MSG_LOW("image_swap_row_cr_cb marker_101\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_RGB444)
  {
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++)
    {
      for(col = 0; col < (crop->dx); col=col+2)
      {
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /*
          ** Convert frame to RGB444
          */
          out = *(uint16*)((uint32)frameImgPtr + frame_index);
          r1 = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r1];
          *((uint16*)((uint32)outputImgPtr + dest_index))=out2;
          out = *(uint16*)((uint32)frameImgPtr + frame_index + 2);
          r1 = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r1];
          *((uint16*)(outputImgPtr + dest_index+2))=out2;
        } 
        else 
        {
          /*
          ** Convert input to rgb
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            crr = crr2;
          } else {
            cbb = cbb2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)((uint32)outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)((uint32)outputImgPtr + dest_index + 2)) = out;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      frame_index += frameIncr;
    } /* End of row loop */
    }
    else
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to rgb
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            crr = crr2;
          } else {
            cbb = cbb2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)((uint32)outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)((uint32)outputImgPtr + dest_index + 2)) = out;
          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        } /* End of col loop */
        src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
        dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      } /* End of row loop */
    }

    MSG_LOW("image_swap_row_cr_cb marker_102\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_YCbCr)
  {
    if (frameImgPtr)
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++)
      {
      for(col = 0; col < (crop->dx); col=col+2)
      {
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /* Use frame */
          /* Convert frame to YCbCr 4:2:2 */
          out = *((uint16*)(frameImgPtr + frame_index));
          unpack_rgb565(out,&r1,&g1,&b1);
          luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + 
                   ycbcr_convert[2]*b1)*4 + 0x8000;
          luma1 = (luma1>>16) + 16;
          luma1= CLIPIT(luma1);
          cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + 
                ycbcr_convert[5]*b1)*4 + 0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);
          /* 2nd pixel */
          out = *((uint16*)(frameImgPtr + frame_index + 2));
          unpack_rgb565(out,&r2,&g2,&b2);
          luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                   ycbcr_convert[2]*b2)*4 + 0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                ycbcr_convert[8]*b2)*4 + 0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);
          *((uint8*)(outputImgPtr + dest_index)) = (uint8) cb;
          *((uint8*)(outputImgPtr + dest_index + 1)) = (uint8) luma1;
          *((uint8*)(outputImgPtr + dest_index + 2)) = (uint8) cr;
          *((uint8*)(outputImgPtr + dest_index + 3)) = (uint8) luma2;
        } else {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            crr = crr2;
          } else {
            cbb = cbb2;
          }
          *((uint8*)(outputImgPtr + dest_index)) = cbb;
          *((uint8*)(outputImgPtr + dest_index + 1)) = lumaa1;
          *((uint8*)(outputImgPtr + dest_index + 2)) = crr;
          *((uint8*)(outputImgPtr + dest_index + 3)) = lumaa2;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      frame_index += frameIncr;
    } /* End of row loop */
    }
    else
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            crr = crr2;
          } else {
            cbb = cbb2;
          }
          *((uint8*)(outputImgPtr + dest_index)) = cbb;
          *((uint8*)(outputImgPtr + dest_index + 1)) = lumaa1;
          *((uint8*)(outputImgPtr + dest_index + 2)) = crr;
          *((uint8*)(outputImgPtr + dest_index + 3)) = lumaa2;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        } /* End of col loop */

        src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
        dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      }
    }

    MSG_LOW("image_swap_row_cr_cb marker_103\n");
    return IPL_SUCCESS;
  } 
  else 
  {
    MSG_LOW("image_swap_row_cr_cb marker_201\n");
    /* Only YCbCr 4:2:2 or RGB565 output supported  */
    return IPL_FAILURE;
  }
} /* End image_swap_row_cr_cb */


/* <EJECT> */
/*===========================================================================

FUNCTION image_swap_cb_cr

DESCRIPTION
  This function is an optimized version to do reflection, rotation, 
  adding of a frame, and color conversion from YCbCr 4:2:2 to RGB565.

  It is assumed that the caller will set the input, frame, and output
  image structures to have the correct dimensions after reflection
  and rotation.

  The frame is assumed to be established in 2 pixel units, i.e. the 
  minimum width of any frame edge is 2 pixels.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr      pointer to the input image
  input_frame_ptr    pointer to the frame image
  transparentValue   16-bit transparent pixel value
  output_img_ptr     pointer to the output image
  startPos           starting position
  colInc             column increment
  rowInc             row increment
  crop               crop region

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type image_swap_cb_cr
( 
  ipl_image_type* i_img_ptr,    /* Points to input image           */
  ipl_image_type* i_frame_ptr,  /* Points to input frame           */
  uint16 transparentValue,      /* Transparent pixel value         */
  ipl_image_type* o_img_ptr,    /* Points to output image          */
  int32 startPos,               /* Starting position               */
  int32 colInc,                 /* Column increment                */
  int32 rowInc,                 /* Row increment                   */
  ipl_rect_type* crop           /* Crop region                     */
)
{
  int32 src_index,dest_index;
  int32 frame_index;
  uint32 row,col;
  ipl_image_type* output_img_ptr;
  unsigned char* inputImgPtr;
  unsigned char* outputImgPtr;
  unsigned char* frameImgPtr = NULL;
  unsigned char r1,g1,b1,r2,g2,b2;
  unsigned short out;
  uint16 out2;
  uint32 out32;
  uint32* data_out;
  int32 luma1,luma2,cb,cr;
  unsigned char cbb=0,crr=0,lumaa1=0,lumaa2=0;
  long rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  short ycbcr2rgb_convert[6] = {8, 25803, -3071, -7672, 30399, 12};
  register int32 frameIncr = 0;

  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("image_swap_cb_cr marker_0\n");

  if (!i_img_ptr ||   !i_img_ptr->imgPtr  || 
      !o_img_ptr ||   !o_img_ptr->imgPtr  ||
      !crop)
  {
    MSG_LOW("image_swap_cb_cr marker_200\n");
    return IPL_FAILURE;
  }

  output_img_ptr = o_img_ptr;
  inputImgPtr=i_img_ptr->imgPtr;
  outputImgPtr = output_img_ptr->imgPtr;
  data_out = (uint32*)output_img_ptr->imgPtr;


  /* initialize the index to starting position */
  src_index = startPos<<1;  /* byte addressed */
  dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
  frame_index = 0;
  if (i_frame_ptr)
  {
    frameImgPtr = i_frame_ptr->imgPtr;
    frameIncr = (i_frame_ptr->dx - crop->dx)<<1;
  }

  MSG_LOW("image_swap_cb_cr marker_1\n");

  if ( output_img_ptr->cFormat == IPL_RGB565 )
  {
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /* Use frame */
          *((uint16*)(outputImgPtr + dest_index)) = 
              *((uint16*)(frameImgPtr + frame_index));
          *((uint16*)(outputImgPtr + dest_index+2)) = 
              *((uint16*)(frameImgPtr + frame_index+2));
        } else {
          /*
          ** Convert input to rgb
          */
          /* This is Cr */
          crr = *((uint8*)(inputImgPtr + src_index));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cb */
          cbb = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index+2)) = out;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
      dest_index = dest_index + ((output_img_ptr->dx - crop->dx) << 1);
      frame_index += frameIncr;
    } /* End of row loop */
    }
    else
    {
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to rgb
          */
          /* This is Cr */
          crr = *((uint8*)(inputImgPtr + src_index));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cb */
          cbb = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index+2)) = out;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        }
        src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
        dest_index = dest_index + ((output_img_ptr->dx - crop->dx) << 1);
      } 
    } /* End of row loop */

    MSG_LOW("image_swap_cb_cr marker_100\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_RGB666)
  {
    dest_index = dest_index << 1;
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /*
          ** Convert frame to rgb 666
          */
          /*
          ** Copy Over the Frame and convert from 565 to 444
          */
          out = *(uint16*)((uint32)frameImgPtr + frame_index);
          r1 = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r1];
          *((uint32*)((uint32)data_out + dest_index))=out32;
          out = *(uint16*)((uint32)frameImgPtr + frame_index + 2);
          r1 = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r1];
          *((uint32*)((uint32)data_out + dest_index+4)) = out32;
        } else {
          /*
          ** Convert input to rgb666
          */
          /* This is Cr */
          crr = *((uint8*)(inputImgPtr + src_index));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cb */
          cbb = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index)) = out32;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index+4)) = out32;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 8;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
      dest_index = dest_index + ((output_img_ptr->dx - crop->dx) << 2);
    } /* End of row loop */
    }
    else
    {
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to rgb666
          */
          /* This is Cr */
          crr = *((uint8*)(inputImgPtr + src_index));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cb */
          cbb = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index)) = out32;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index+4)) = out32;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 8;
        }
        src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
        dest_index = dest_index + ((output_img_ptr->dx - crop->dx) << 2);
      } /* End of row loop */
    }
    MSG_LOW("image_swap_cb_cr marker_101\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_RGB444)
  {
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /*
          ** Convert frame to rgb 444
          */
          /*
          ** Copy Over the Frame and convert from 565 to 444
          */
          out = *(uint16*)((uint32)frameImgPtr + frame_index);
          r1 = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r1];
          *((uint16*)((uint32)outputImgPtr + dest_index)) = out2;
          out = *(uint16*)((uint32)frameImgPtr + frame_index + 2);
          r1 = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r1];
          *((uint16*)(outputImgPtr + dest_index+2)) = out2;
        } else {
          /*
          ** Convert input to rgb444
          */
          /* This is Cr */
          crr = *((uint8*)(inputImgPtr + src_index));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cb */
          cbb = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)((uint32)outputImgPtr + dest_index))=out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)(outputImgPtr + dest_index+2)) = out;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      frame_index += frameIncr;
    } /* End of row loop */
    }
    else
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to rgb444
          */
          /* This is Cr */
          crr = *((uint8*)(inputImgPtr + src_index));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cb */
          cbb = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)((uint32)outputImgPtr + dest_index))=out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)(outputImgPtr + dest_index+2)) = out;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        } /* End of col loop */
        src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
        dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      } /* End of row loop */
    }

    MSG_LOW("image_swap_cb_cr marker_102\n");
    return IPL_SUCCESS;
  } else if (output_img_ptr->cFormat == IPL_YCbCr)
  {
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /* Use frame */
          /* Convert frame to YCbCr 4:2:2 */
          out = *((uint16*)(frameImgPtr + frame_index));
          unpack_rgb565(out,&r1,&g1,&b1);
          luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + 
                   ycbcr_convert[2]*b1)*4+0x8000;
          luma1 = (luma1>>16) + 16;
          luma1= CLIPIT(luma1);
          cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + 
                ycbcr_convert[5]*b1)*4+0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);
          /* 2nd pixel */
          out = *((uint16*)(frameImgPtr + frame_index + 2));
          unpack_rgb565(out,&r2,&g2,&b2);
          luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                   ycbcr_convert[2]*b2)*4+0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                ycbcr_convert[8]*b2)*4+0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);
          *((uint8*)(outputImgPtr + dest_index)) = (uint8) cb;
          *((uint8*)(outputImgPtr + dest_index + 1)) = (uint8) luma1;
          *((uint8*)(outputImgPtr + dest_index + 2)) = (uint8) cr;
          *((uint8*)(outputImgPtr + dest_index + 3)) = (uint8) luma2;
        } 
        else 
        {
          /* This is Cb */
          crr = *((uint8*)(inputImgPtr + src_index));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is Cr */
          cbb = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          *((uint8*)(outputImgPtr + dest_index)) = cbb;
          *((uint8*)(outputImgPtr + dest_index + 1)) = lumaa1;
          *((uint8*)(outputImgPtr + dest_index + 2)) = crr;
          *((uint8*)(outputImgPtr + dest_index + 3)) = lumaa2;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
      dest_index = dest_index + ((output_img_ptr->dx - crop->dx) << 1);
      frame_index+=frameIncr;
    } /* End of row loop */
    }
    else
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /* This is Cb */
          crr = *((uint8*)(inputImgPtr + src_index));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is Cr */
          cbb = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          *((uint8*)(outputImgPtr + dest_index)) = cbb;
          *((uint8*)(outputImgPtr + dest_index + 1)) = lumaa1;
          *((uint8*)(outputImgPtr + dest_index + 2)) = crr;
          *((uint8*)(outputImgPtr + dest_index + 3)) = lumaa2;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        } /* End of col loop */

        src_index = src_index + (rowInc<<1) - (colInc<<1); /* byte addressed */
        dest_index = dest_index + ((output_img_ptr->dx - crop->dx) << 1);
      }
    }

    MSG_LOW("image_swap_cb_cr marker_103\n");
    return IPL_SUCCESS;
  } 
  else 
  {
    MSG_LOW("image_swap_cb_cr marker_201\n");
    /* Only YCbCr 4:2:2 or RGB565 output supported  */
    return IPL_FAILURE;
  }
} /* End image_swap_cb_cr */


/* <EJECT> */
/*===========================================================================

FUNCTION image_swap_row_cb_cr

DESCRIPTION
  This function is an optimized version to do reflection, rotation, 
  adding of a frame, and color conversion from YCbCr 4:2:2 to RGB565.

  It is assumed that the caller will set the input, frame, and output
  image structures to have the correct dimensions after reflection
  and rotation.

  The frame is assumed to be established in 2 pixel units, i.e. the 
  minimum length of any frame edge is 2 pixels.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr       pointer to the input image
  input_frame_ptr     pointer to the frame image
  transparentValue    16-bit transparent pixel value
  output_img_ptr      pointer to the output image
  startPos            starting position
  colInc              column increment
  rowInc              row increment
  crop                crop region

RETURN VALUE
  IPL_SUCCESS         indicates operation was successful
  IPL_FAILURE         otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type image_swap_row_cb_cr
( 
  ipl_image_type* i_img_ptr,      /* Points to the input image     */
  ipl_image_type* i_frame_ptr,    /* Points to the input frame     */
  uint16 transparentValue,        /* Transparent pixel value       */
  ipl_image_type* o_img_ptr,      /* Points to the output image    */
  int32 startPos,                 /* Starting position             */
  int32 colInc,                   /* Column increment              */
  int32 rowInc,                   /* Row increment                 */
  ipl_rect_type* crop             /* Crop region                   */
)
{
  int32 src_index,dest_index;
  int32 frame_index;
  uint32 row,col;
  ipl_image_type* input_frame_ptr;
  ipl_image_type* output_img_ptr;
  unsigned char* inputImgPtr;
  unsigned char* outputImgPtr;
  unsigned char* frameImgPtr = NULL;
  unsigned char r1,g1,b1,r2,g2,b2;
  unsigned short out;
  int32 luma1,luma2,cb,cr;
  unsigned char cbb=0,crr=0,lumaa1=0,lumaa2=0,cbb2=0,crr2=0;
  long rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  short ycbcr2rgb_convert[6] = {8, 25803, -3071, -7672, 30399, 12};
  int32 frameIncr = 0;
  uint32* data_out;
  uint32 out32;
  uint16 out2;

  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("image_swap_row_cb_cr marker_0\n");

  if (!i_img_ptr ||   !i_img_ptr->imgPtr  || 
      !o_img_ptr ||   !o_img_ptr->imgPtr  ||
      !crop)
  {
    MSG_LOW("image_swap_row_cb_cr marker_200\n");
    return IPL_FAILURE;
  }
  input_frame_ptr = i_frame_ptr;
  output_img_ptr = o_img_ptr;
  inputImgPtr=i_img_ptr->imgPtr;
  outputImgPtr = output_img_ptr->imgPtr;
  data_out = (uint32*)outputImgPtr;


  /* Initialize the index to starting position */
  src_index = startPos<<1;  /* byte addressed */
  dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
  frame_index = 0;
  if (i_frame_ptr && input_frame_ptr)
  {
    frameImgPtr = i_frame_ptr->imgPtr;
    frameIncr = (i_frame_ptr->dx - crop->dx)<<1;
  }

  MSG_LOW("image_swap_row_cb_cr marker_1\n");

  if ( output_img_ptr->cFormat == IPL_RGB565 )
  {
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /* Use frame */
          *((uint16*)(outputImgPtr + dest_index)) = 
              *((uint16*)(frameImgPtr + frame_index));
          *((uint16*)(outputImgPtr + dest_index+2)) = 
              *((uint16*)(frameImgPtr + frame_index+2));
        } else {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            cbb = cbb2;
          } else {
            crr = crr2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index+2)) = out;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      frame_index+=frameIncr;
    } /* End of row loop */
    }
    else
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++){
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            cbb = cbb2;
          } else {
            crr = crr2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(outputImgPtr + dest_index+2)) = out;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        } /* End of col loop */
        src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
        dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      } /* End of row loop */
    }

    MSG_LOW("image_swap_row_cb_cr marker_100\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_RGB666)
  {
    dest_index = dest_index << 1;
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /*
          ** Use frame but convert it to RGB666
          */
          out = *(uint16*)((uint32)frameImgPtr + frame_index);
          r1 = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r1];
          *((uint32*)((uint32)data_out + dest_index))=out32;
          out = *(uint16*)((uint32)frameImgPtr + frame_index + 2);
          r1 = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r1];
          *((uint32*)((uint32)data_out + dest_index+4))=out32;
        } else {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is Cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            cbb = cbb2;
          } else {
            crr = crr2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index)) = out32;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index + 4)) = out32;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 8;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 2);
      frame_index+=frameIncr;
    } /* End of row loop */
    }
    else
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is Cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            cbb = cbb2;
          } else {
            crr = crr2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index)) = out32;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out32 = r666[r];
          out32 += g666[g];
          out32 += b666[b];
          *((uint32*)((uint32)data_out + dest_index + 4)) = out32;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 8;
        } /* End of col loop */
        src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
        dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 2);
      } /* End of row loop */
    }

    MSG_LOW("image_swap_row_cb_cr marker_101\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_RGB444)
  {
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /*
          ** Use frame but convert it to RGB444
          */
          out = *(uint16*)((uint32)frameImgPtr + frame_index);
          r1 = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r1];
          *((uint16*)((uint32)outputImgPtr + dest_index))=out2;
          out = *(uint16*)((uint32)frameImgPtr + frame_index + 2);
          r1 = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r1];
          r1 = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r1];
          r1 = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r1];
          *((uint16*)(outputImgPtr + dest_index+2))=out2;
        } else {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is Cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            cbb = cbb2;
          } else {
            crr = crr2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) +
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)(outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)(outputImgPtr + dest_index+2)) = out;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
      dest_index = dest_index + ((output_img_ptr->dx - crop->dx) << 1);
      frame_index+=frameIncr;
    } /* End of row loop */
    }
    else
    {
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is Cr */
          crr = *((uint8*)(inputImgPtr + src_index+(colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+(colInc<<1)+1));
          if (row%2)
          {
            cbb = cbb2;
          } else {
            crr = crr2;
          }
          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4 + 0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) +
                ycbcr2rgb_convert[3]*(crr-128))*4 + 0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4 + 0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)(outputImgPtr + dest_index)) = out;
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = r444[r];
          out += g444[g];
          out += b444[b];
          *((uint16*)(outputImgPtr + dest_index+2)) = out;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        }

        src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
        dest_index = dest_index + ((output_img_ptr->dx - crop->dx) << 1);
      } /* End of row loop */
    }

    MSG_LOW("image_swap_row_cb_cr marker_102\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_YCbCr)
  {
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++)
    {
      for(col = 0; col < (crop->dx); col=col+2)
      {
        if ((*(uint16*)(frameImgPtr + frame_index)) != transparentValue)
        {
          /* Use frame */
          /* Convert frame to YCbCr 4:2:2 */
          out = *((uint16*)(frameImgPtr + frame_index));
          unpack_rgb565(out,&r1,&g1,&b1);
          luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + 
                   ycbcr_convert[2]*b1)*4 + 0x8000;
          luma1 = (luma1>>16) + 16;
          luma1= CLIPIT(luma1);
          cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + 
                ycbcr_convert[5]*b1)*4 + 0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);
          /* 2nd pixel */
          out = *((uint16*)(frameImgPtr + frame_index + 2));
          unpack_rgb565(out,&r2,&g2,&b2);
          luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                   ycbcr_convert[2]*b2)*4 + 0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                ycbcr_convert[8]*b2)*4+0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);
          *((uint8*)(outputImgPtr + dest_index)) = (uint8) cb;
          *((uint8*)(outputImgPtr + dest_index + 1)) = (uint8) luma1;
          *((uint8*)(outputImgPtr + dest_index + 2)) = (uint8) cr;
          *((uint8*)(outputImgPtr + dest_index + 3)) = (uint8) luma2;
        } else {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is Cr */
          crr = *((uint8*)(inputImgPtr + src_index + (colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index + (colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index + (colInc<<1)+1));
          if (row%2)
          {
            cbb = cbb2;
          } else {
            crr = crr2;
          }
          *((uint8*)(outputImgPtr + dest_index)) = cbb;
          *((uint8*)(outputImgPtr + dest_index + 1)) = lumaa1;
          *((uint8*)(outputImgPtr + dest_index + 2)) = crr;
          *((uint8*)(outputImgPtr + dest_index + 3)) = lumaa2;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;
        frame_index += 4;
      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      frame_index += frameIncr;
    } /* End of row loop */
    }
    else
    {
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          /*
          ** Convert input to RGB
          */
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          cbb2 = *((uint8*)(inputImgPtr + src_index + 2));
          /* Next byte is luma of 1st pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is Cr */
          crr = *((uint8*)(inputImgPtr + src_index + (colInc<<1)));
          crr2 = *((uint8*)(inputImgPtr + src_index + (colInc<<1)+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index + (colInc<<1)+1));
          if (row%2)
          {
            cbb = cbb2;
          } else {
            crr = crr2;
          }
          *((uint8*)(outputImgPtr + dest_index)) = cbb;
          *((uint8*)(outputImgPtr + dest_index + 1)) = lumaa1;
          *((uint8*)(outputImgPtr + dest_index + 2)) = crr;
          *((uint8*)(outputImgPtr + dest_index + 3)) = lumaa2;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        } /* End of col loop */
        src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
        dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      } /* End of row loop */
    }

    MSG_LOW("image_swap_row_cb_cr marker_103\n");
    return IPL_SUCCESS;
  } 
  else 
  {
    MSG_LOW("image_swap_row_cb_cr marker_201\n");
    /* Only YCbCr 4:2:2 or RGB565 output supported  */
    return IPL_FAILURE;
  }
} /* End image_swap_row_cb_cr */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_xform_Resize_qHigh

DESCRIPTION
  This function is used to do upsize and downsize usin bilinear interpolation
  The input should be RGB565 and the output should be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_xform_Resize_qHigh
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  cropin,
  ipl_rect_type*  cropout
)
{
  uint32 row,col;
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint32 pitchx;

  uint8 xf, yf;
  register uint16 x, y;
  register uint8 w;
  uint16* ptr16; 
  uint16* iptr16; 
  uint8* ptr8; 

  uint8 * rgb_buf;
  //uint8 * out;
  uint16 val;

  //uint32 cix, ciy, cidx, cidy, cidx3;
  uint32 cix, ciy, cidx, cidy;
  uint32 cox, coy, codx, cody;
  uint32 idx,idy;
  uint32 odx,ody;
  uint16 accum_1, accum_2, accum_3;

  unsigned char r,g,b;

  MSG_LOW("ipl_xform_Resize_qHigh marker_0\n");
  //MSG_LOW("inside ipl_xform_Resize_qHigh\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_xform_Resize_qHigh marker_200\n");
    return IPL_FAILURE;
  }

  /* how we lookup a point in crop space */
#define P(x,y,c)  (*(rgb_buf + (y)*cidx3 + (x)*3+c))




  /* if user doesnt provide output crop, assume it is the same as the output
   * image */
  if (cropout == NULL)
  {
    cox = 0;
    coy = 0;
    codx = o_img_ptr->dx;
    cody = o_img_ptr->dy;
  }
  else
  {
    cox = cropout->x;
    coy = cropout->y;
    codx = cropout->dx;
    cody = cropout->dy;
  }

  /* if user doesnt provide input crop, assume it is the same as input 
   * image */
  if (cropin == NULL)
  {
    cix = 0;
    ciy = 0;
    cidx = i_img_ptr->dx;
    cidy = i_img_ptr->dy;
  }
  else
  { 
    cix = cropin->x;
    ciy = cropin->y;
    cidx = cropin->dx;
    cidy = cropin->dy;
  }

  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;
  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;
  //cidx3 = (cidx+1)*3;     

  /* make sure valid input size */
  if (idx < 1 || idy < 1)
  {
    MSG_LOW("ipl_xform_Resize_qHigh marker_201\n");
    return IPL_FAILURE;
  }

  /* dont allow cropping of area bigger than image */
  if ((cidx+cix > idx) || (cidy+ciy > idy))
  {
    MSG_LOW("ipl_xform_Resize_qHigh marker_202\n");
    return IPL_FAILURE;
  }
  /*
   * Choose Q9 so you can upsize area from 10x10 to at 5120x5120, or
   * i.e. at least a factor of 512, 2^9
   */

  resizeFactorX = (cidx<<9) / codx;
  resizeFactorY = (cidy<<9) / cody;

  if (IPL_XFORM_DEBUG) //lint !e774 !e506
  {
    printf("Input          (%lu,%lu)\n", idx, idy);
    printf("CropI  (%lu,%lu) (%lu,%lu)\n", cix,ciy,cidx,cidy);
    printf("CropO  (%lu,%lu) (%lu,%lu)\n", cox,coy,codx,cody);
    printf("Output         (%lu,%lu)\n", odx, ody);
    printf("ResizeX:  %ld\n", resizeFactorX);
    printf("ResizeY:  %ld\n", resizeFactorY);
  }

  MSG_LOW("ipl_xform_Resize_qHigh marker_1\n");

  if(i_img_ptr->cFormat == IPL_RGB565 && 
     o_img_ptr->cFormat == IPL_RGB565)
  {
    /* Go through each output pix and fill with weighted sum from temp buffer*/
    // ptr16 = (uint16*)o_img_ptr->imgPtr;
  
    iptr16 = (uint16*)i_img_ptr->imgPtr + cix + ciy*i_img_ptr->dx;
    ptr16  = (uint16*)o_img_ptr->imgPtr + cox + coy*o_img_ptr->dx;

    pitchx = o_img_ptr->dx - codx;

    for (row=0; row < cody; row++)
    {
      /* shift back by 9 place for integer part */
      y  = (uint16) ((uint32) (row*resizeFactorY) >> 9);
  
      /* shift 6 places and AND with 0111 to get 3 bits in remainder */
      yf = (uint8)  ((uint32)((row*resizeFactorY) >> 6) & 0x7);
  
      /* advance to next row in temp buffer */
      for (col=0; col < codx; col++)
      {
        x  = (uint16) ((uint32)  (col*resizeFactorX) >> 9);
        xf = (uint8)  ((uint32) ((col*resizeFactorX) >> 6) & 0x7);
  
        if (IPL_XFORM_DEBUG && 1) //lint !e774 !e506
        {
          printf("\n(col,row): %lu,%lu\n", col,row);
          printf("x, xf: %d, %d\n", x, xf);
          printf("y, yf: %d, %d\n", y, yf);
        }
  
        /*
        **    +--------+--------+
        **    |        |        |
        **    |        |        |
        **    |   A    |   B    |
        **    |        |        |
        **    +--------+--------+
        **    |      X |        |       X value is A*(1-fx)*(1-fy)
        **    |        |        |                  B*(fx)*(1-fy) 
        **    |   C    |   D    |                  C*(1-fx)*(fy)
        **    |        |        |                  D*(fx)*(fy)
        **    +--------+--------+
        */
  
        if (IPL_XFORM_DEBUG && 1) //lint !e774 !e506
        {
          printf("wA%d wB%d wC%d wD%d\n", (8-xf)*(8-yf),(xf)*(8-yf),
                 (8-xf)*(yf),(xf)*(yf));
          printf("tA%d tB%d tC%d tD%d\n", biWeights[xf][yf][0],
          biWeights[xf][yf][1], biWeights[xf][yf][3], biWeights[xf][yf][2]);
        }
  
        /* A pixel */
        w = biWeights[xf][yf][0];
        val = *(iptr16 + (y)*idx + (x));
        if (IPL_XFORM_DEBUG) //lint !e774 !e506
          printf("val %d\n", val);
        unpack_rgb565(val,&r,&g,&b);
        if (IPL_XFORM_DEBUG) //lint !e774 !e506
          printf("rgb %d %d %d\n", r,g,b);
        accum_1 = w * r;
        accum_2 = w * g;
        accum_3 = w * b;
  
        /* B pixel */
        w = biWeights[xf][yf][1];
        val = *(iptr16 + (y)*idx + (x+1));
        unpack_rgb565(val,&r,&g,&b);
        accum_1 += w * r;
        accum_2 += w * g;
        accum_3 += w * b;
  
        /* C pixel */
        w = biWeights[xf][yf][3];
        val = *(iptr16 + (y+1)*idx + (x));
        unpack_rgb565(val,&r,&g,&b);
        accum_1 += w * r;
        accum_2 += w * g;
        accum_3 += w * b;
  
        /* D pixel */
        w = biWeights[xf][yf][2];
        val = *(iptr16 + (y+1)*idx + (x+1));
        unpack_rgb565(val,&r,&g,&b);
        accum_1 += w * r;
        accum_2 += w * g;
        accum_3 += w * b;
  
        if (IPL_XFORM_DEBUG && 1) //lint !e774 !e506
          printf("Crgb %d,%d,%d\n", accum_1, accum_2, accum_3);
  
        /* divide by 64 */
        accum_1 = accum_1 >> 6;
        accum_2 = accum_2 >> 6;
        accum_3 = accum_3 >> 6;
  
        if (IPL_XFORM_DEBUG && 1) //lint !e774 !e506
          printf("Final RGB %d,%d,%d\n", accum_1, accum_2, accum_3);
  
        *ptr16 = pack_rgb565(accum_1, accum_2, accum_3);

        ptr16++;
      } 
      ptr16 += pitchx;
    } 
  }
  else if(i_img_ptr->cFormat == IPL_RGB888 && 
          o_img_ptr->cFormat == IPL_RGB888)
  {
    #define P8(x,y,c) (*(rgb_buf + (y*cidx+x)*3+c))

  	rgb_buf = i_img_ptr->imgPtr + ((cix + ciy*i_img_ptr->dx)*3);

    /* Go through each output pix and fill with weighted sum from temp buffer*/
  	ptr8 = o_img_ptr->imgPtr + ((cox + coy*o_img_ptr->dx)*3);
    pitchx = (o_img_ptr->dx - codx)*3;

    for (row=0; row < cody; row++)
    {
      /* shift back by 9 place for integer part */
      y  = (uint16) ((uint32) (row*resizeFactorY) >> 9);
  
      /* shift 6 places and AND with 0111 to get 3 bits in remainder */
      yf = (uint8)  ((uint32)((row*resizeFactorY) >> 6) & 0x7);
  
      /* advance to next row in temp buffer */
      for (col=0; col < codx; col++)
      {
        x  = (uint16) ((uint32)  (col*resizeFactorX) >> 9);
        xf = (uint8)  ((uint32) ((col*resizeFactorX) >> 6) & 0x7);
  
        if (IPL_XFORM_DEBUG && 0) //lint !e774 !e506
        {
          printf("\n(col,row): %lu,%lu\n", col,row);
          printf("x, xf: %d, %d\n", x, xf);
          printf("y, yf: %d, %d\n", y, yf);
        }
  
        /*
        **    +--------+--------+
        **    |        |        |
        **    |        |        |
        **    |   A    |   B    |
        **    |        |        |
        **    +--------+--------+
        **    |      X |        |       X value is A*(1-fx)*(1-fy)
        **    |        |        |                  B*(fx)*(1-fy) 
        **    |   C    |   D    |                  C*(1-fx)*(fy)
        **    |        |        |                  D*(fx)*(fy)
        **    +--------+--------+
        */
  
        if (IPL_XFORM_DEBUG && 0) //lint !e774 !e506
        {
          printf("wA%d wB%d wC%d wD%d\n", (8-xf)*(8-yf),(xf)*(8-yf),
                 (8-xf)*(yf),(xf)*(yf));
          printf("tA%d tB%d tC%d tD%d\n\n", biWeights[xf][yf][0],
          biWeights[xf][yf][1], biWeights[xf][yf][3], biWeights[xf][yf][2]);
        }
  
        /* A pixel */
        w = biWeights[xf][yf][0];
        accum_1 = w * P8(x,y,0);
        accum_2 = w * P8(x,y,1);
        accum_3 = w * P8(x,y,2);
  
        /* B pixel */
        w = biWeights[xf][yf][1];
        accum_1 += (w * P8((x+1),y,0));
        accum_2 += (w * P8((x+1),y,1));
        accum_3 += (w * P8((x+1),y,2));
  
        /* C pixel */
        w = biWeights[xf][yf][3];
        accum_1 += (w * P8(x,(y+1),0));
        accum_2 += (w * P8(x,(y+1),1));
        accum_3 += (w * P8(x,(y+1),2));
  
        /* D pixel */
        w = biWeights[xf][yf][2];
        accum_1 += (w * P8((x+1),(y+1),0));
        accum_2 += (w * P8((x+1),(y+1),1));
        accum_3 += (w * P8((x+1),(y+1),2));
  
        if (IPL_XFORM_DEBUG && 0) //lint !e774 !e506
          printf("Crgb %d,%d,%d\n", accum_1, accum_2, accum_3);
  
        /* divide by 64 */
        accum_1 = accum_1 >> 6;
        accum_2 = accum_2 >> 6;
        accum_3 = accum_3 >> 6;
  
        if (IPL_XFORM_DEBUG && 0) //lint !e774 !e506
          printf("Final RGB %d,%d,%d\n", accum_1, accum_2, accum_3);
  
        *ptr8++ = (uint8) accum_1;
        *ptr8++ = (uint8) accum_2;
        *ptr8++ = (uint8) accum_3;
      } 
      ptr8 += pitchx;
    } 
  }
  else
  {
    MSG_LOW("ipl_xform_Resize_qHigh marker_205\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_xform_Resize_qHigh marker_100\n");
  return IPL_SUCCESS;
}

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_downsize_rot_ycbcr

DESCRIPTION
  This function performs cropping on input, downsizing, and then clockwise 
  rotation of an image.

  The input must be YCbCr422 line pack;

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         pointer to the input image
  o_img_ptr         pointer to the output image
  crop              region on input to be cropped
  rotate            type of rotation

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_crop_downsize_rot_ycbcr
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type* crop,              /* Crop config                      */
  ipl_rotate90_type rotate          /* Rotatation                       */
)
{
  register int32 cdb,cdr,lumad1;

  int32 k,m,mv;
  int32 c, cv;
  int32 count,step;
  int32* row_map;
  int32* col_map;
  int32 row,col;

  register int32 i,j,n;
  register unsigned char * iy;
  register uint8 * ic;

  MSG_LOW("ipl_crop_downsize_rot_ycbcr marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr ||
      !crop)
  {
    MSG_LOW("ipl_crop_downsize_rot_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  // if we are rotating 90 or 270, then undo this when building resize table
  if (rotate == IPL_ROT90 || rotate == IPL_ROT270)
  {
    m = o_img_ptr->dy;
    mv = o_img_ptr->dx;
  }
  else
  {
    m = o_img_ptr->dx;
    mv = o_img_ptr->dy;
  }

  // some commonly used vars 
  n = i_img_ptr->dx;
  c = crop->dx;
  cv = crop->dy;

  /*
  ** Malloc the 2 line buffers
  */
  row_map = (int32*)ipl_malloc((mv+1)*sizeof(int32));
  if (!row_map) 
  {
    MSG_LOW("ipl_crop_downsize_rot_ycbcr marker_201\n");
    return IPL_NO_MEMORY;
  }
  col_map = (int32*)ipl_malloc((m+1)*sizeof(int32));
  if (!col_map) 
  {
    ipl_sys_free(row_map);
    MSG_LOW("ipl_crop_downsize_rot_ycbcr marker_202\n");
    return IPL_NO_MEMORY;
  }

  MSG_LOW("ipl_crop_downsize_rot_ycbcr marker_1\n");

  // map NV inputs to MV outputs
  step = 0;
  count = 0;
  // map column input 0 to N to output 0 to N
  for (k=0; k < c; k++)
  {
    count = count + m;
    if (count >= (int32) c)
    {
      count = count - c;
      col_map[step++] = (int32)k+0;
    }
  }
  col_map[m] = (uint32) c;


  // map N inputs to M outputs
  step = 0;
  count = 0;
  // map row input 0 to N to output 0 to N
  for (k=0; k < cv; k++)
  {
    count = count + mv;
    if (count >= (int32) cv)
    {
      count = count - cv;
      row_map[step++] = (int32) k;
    }
  }
  row_map[mv] = (uint32) cv;


  if (i_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
  {
    iy = i_img_ptr->imgPtr + crop->y*n + crop->x;
    ic = i_img_ptr->clrPtr + crop->y*n + crop->x;

    if (rotate == IPL_NOROT) 
    {
      for (row=0; row < mv; row++)
      {
        for (col=0; col < m; col++)
        {
          count = (col_map[col+1]-col_map[col])*(row_map[row+1]-row_map[row]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;

          for (j = row_map[row]; j < row_map[row+1]; j++)
          {
            for (i = col_map[col]; i < col_map[col+1]; i++)
            {
              lumad1 += *(iy + (j*n+i));
              cdb += *(ic + (j*n+2*(i/2)));
              cdr += *(ic + (j*n+2*(i/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          *(o_img_ptr->imgPtr + ((row*m)+col)) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((row*m)+2*(col/2))) = (unsigned char) cdb;
          *(o_img_ptr->clrPtr + ((row*m)+2*(col/2))+1) = (unsigned char) cdr;
        } 
      } 
    }
    else if (rotate == IPL_ROT180) 
    {
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[col+1]-col_map[col])*(row_map[row+1]-row_map[row]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;

          for (j = row_map[row]; j < row_map[row+1]; j++)
          {
            for (i = col_map[col]; i < col_map[col+1]; i++)
            {
              lumad1 += *(iy + (j*n+i));
              cdb += *(ic + (j*n+2*(i/2)));
              cdr += *(ic + (j*n+2*(i/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          *(o_img_ptr->imgPtr + (((mv-row-1)*m)+(m-col-1))) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + (((mv-row-1)*m)+2*((m-col-1)/2))) = (unsigned char) cdb;
          *(o_img_ptr->clrPtr + (((mv-row-1)*m)+2*((m-col-1)/2))+1) = (unsigned char) cdr;
        } 
      } 
    }
    else if (rotate == IPL_ROT90)
    {
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[col+1]-col_map[col])*(row_map[row+1]-row_map[row]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;

          for (j = row_map[row]; j < row_map[row+1]; j++)
          {
            for (i = col_map[col]; i < col_map[col+1]; i++)
            {
              lumad1 += *(iy + (j*n+i));
              cdb += *(ic + (j*n+2*(i/2)));
              cdr += *(ic + (j*n+2*(i/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;


          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          *(o_img_ptr->imgPtr + ((col*mv)+(mv-row-1))) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((col*mv)+2*((mv-row-1)/2))) = (unsigned char) cdb;
          *(o_img_ptr->clrPtr + ((col*mv)+2*((mv-row-1)/2))+1) = (unsigned char) cdr;
        }
      }
    }
    else if (rotate == IPL_ROT270)
    {
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[col+1]-col_map[col])*(row_map[row+1]-row_map[row]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;

          for (j = row_map[row]; j < row_map[row+1]; j++)
          {
            for (i = col_map[col]; i < col_map[col+1]; i++)
            {
              lumad1 += *(iy + (j*n+i));
              cdb += *(ic + (j*n+2*(i/2)));
              cdr += *(ic + (j*n+2*(i/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          *(o_img_ptr->imgPtr + (((m-col-1)*mv)+(row))) = (unsigned char)lumad1;
          *(o_img_ptr->clrPtr + (((m-col-1)*mv)+2*((row)/2))) = (uint8) cdb;
          *(o_img_ptr->clrPtr + (((m-col-1)*mv)+2*((row)/2))+1) = (uint8) cdr;
        }
      }
    }
  } 
  else if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    iy = i_img_ptr->imgPtr + crop->y*n + crop->x;
    ic = i_img_ptr->clrPtr + crop->y*n/2 + crop->x;

    if (rotate == IPL_NOROT) 
    {
      for (row=0; row < mv; row++)
      {
        for (col=0; col < m; col++)
        {
          count = (col_map[col+1]-col_map[col])*(row_map[row+1]-row_map[row]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;

          for (j = row_map[row]; j < row_map[row+1]; j++)
          {
            for (i = col_map[col]; i < col_map[col+1]; i++)
            {
              lumad1 += *(iy + (j*n+i));
              cdb += *(ic + ((j/2)*n+2*(i/2)));
              cdr += *(ic + ((j/2)*n+2*(i/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          *(o_img_ptr->imgPtr + ((row*m)+col)) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((row/2)*m+2*(col/2))) = (uint8) cdb;
          *(o_img_ptr->clrPtr + ((row/2)*m+2*(col/2))+1) = (uint8) cdr;
        } 
      } 
    }
    else if (rotate == IPL_ROT180) 
    {
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[col+1]-col_map[col])*(row_map[row+1]-row_map[row]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;

          for (j = row_map[row]; j < row_map[row+1]; j++)
          {
            for (i = col_map[col]; i < col_map[col+1]; i++)
            {
              lumad1 += *(iy + (j*n+i));
              cdb += *(ic + ((j/2)*n+2*(i/2)));
              cdr += *(ic + ((j/2)*n+2*(i/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          *(o_img_ptr->imgPtr + (((mv-row-1)*m)+(m-col-1))) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + (((mv-row-1)/2)*m)+2*((m-col-1)/2)) = (uint8) cdb;
          *(o_img_ptr->clrPtr + (((mv-row-1)/2)*m)+2*((m-col-1)/2)+1) = (uint8) cdr;
        } 
      } 
    }
    else if (rotate == IPL_ROT90)
    {
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[col+1]-col_map[col])*(row_map[row+1]-row_map[row]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;

          for (j = row_map[row]; j < row_map[row+1]; j++)
          {
            for (i = col_map[col]; i < col_map[col+1]; i++)
            {
              lumad1 += *(iy + (j*n+i));
              cdb += *(ic + ((j/2)*n+2*(i/2)));
              cdr += *(ic + ((j/2)*n+2*(i/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;


          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          *(o_img_ptr->imgPtr + ((col*mv)+(mv-row-1))) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((col/2)*mv+2*((mv-row-1)/2))) = (uint8) cdb;
          *(o_img_ptr->clrPtr + ((col/2)*mv+2*((mv-row-1)/2)+1)) = (uint8) cdr;
        }
      }
    }
    else if (rotate == IPL_ROT270)
    {
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[col+1]-col_map[col])*(row_map[row+1]-row_map[row]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;

          for (j = row_map[row]; j < row_map[row+1]; j++)
          {
            for (i = col_map[col]; i < col_map[col+1]; i++)
            {
              lumad1 += *(iy + (j*n+i));
              cdb += *(ic + ((j/2)*n+2*(i/2)));
              cdr += *(ic + ((j/2)*n+2*(i/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          *(o_img_ptr->imgPtr + (((m-col-1)*mv)+row)) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + (((m-col-1)/2)*mv+2*(row/2))) = (uint8) cdb;
          *(o_img_ptr->clrPtr + (((m-col-1)/2)*mv+2*(row/2))+1) = (uint8) cdr;
        }
      }
    }
  } 
  else 
  {
    ipl_sys_free(row_map);
    ipl_sys_free(col_map);
    MSG_LOW("ipl_crop_downsize_rot_ycbcr marker_203\n");
    return IPL_FAILURE;
  }

  ipl_sys_free(row_map);
  ipl_sys_free(col_map);

  MSG_LOW("ipl_crop_downsize_rot_ycbcr marker_100\n");

  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_resizeFrame

DESCRIPTION
  This function is used to do upsize and downsize using nearest neighbor
  The input should be RGB565 and the output should be RGB565. This 
  function preserves transparency.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   points to the input image
  output_img_ptr  points to the output image
  cropout         how much to resize to in final image, if NULL, upsizes
                  to match size of output image
  tv              Transparent pixel value             


RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_resizeFrame
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  cropout,          /* how much to resize to in final image */
  uint16 tv                         /* Transparent pixel value              */
)
{
  uint32 row,col;
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint32 pitchx;
  //ipl_image_type ycbcrBuf;

  register uint16 accum_1, accum_2, accum_3;
  register uint16 x, y;
  register uint8 w;
  register uint32 in;
  register uint16* ptr16; 
  // register uint8* yptr; 
  int accum_W;

  uint8 xf, yf;
  uint16* inImgPtr;
  uint32 cox, coy, codx, cody;
  uint32 idx,idy;
  // boolean noEdge;
  int ewA, ewB, ewC, ewD;
  // int ep, dir, maxEp;

  MSG_LOW("ipl_resizeFrame marker_0\n");
  //MSG_LOW("inside ipl_resizeFrame\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_resizeFrame marker_200\n");
    return IPL_FAILURE;
  }
  inImgPtr = (uint16 *) i_img_ptr->imgPtr;

  /* if user doesnt provide output crop, assume it is the same as the output
   * image */
  if (cropout == NULL)
  {
    cox = 0;
    coy = 0;
    codx = o_img_ptr->dx;
    cody = o_img_ptr->dy;
  }
  else
  {
    cox = cropout->x;
    coy = cropout->y;
    codx = cropout->dx;
    cody = cropout->dy;
  }

  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;

  /* make sure valid input size */
  if (idx < 1 || idy < 1)
  {
    MSG_LOW("ipl_resizeFrame marker_201\n");
    return IPL_FAILURE;
  }

  /*
   * Choose Q9 so you can upsize area from 10x10 to at 5120x5120, or
   * i.e. at least a factor of 512, 2^9
   */

  if (codx == 0 || cody == 0)
  {
    MSG_LOW("ipl_resizeFrame marker_202\n");
    return IPL_FAILURE;
  }
  // resize factors
  // we cutoff last row since using 2x2 bilinear.
  resizeFactorX = ((idx-1)<<9) / codx;
  resizeFactorY = ((idy-1)<<9) / cody;

  MSG_LOW("ipl_resizeFrame marker_1\n");

  if(i_img_ptr->cFormat == IPL_RGB565 && 
     o_img_ptr->cFormat == IPL_RGB565)
  {
    ptr16 = (uint16*)o_img_ptr->imgPtr + cox + coy*o_img_ptr->dx;
    pitchx = o_img_ptr->dx - codx + 0;

    for (row=0; row < cody-1; row++)
    {
      /* shift back by 9 place for integer part */
      y  = (uint16) ((uint32) (row*resizeFactorY) >> 9);
  
      /* shift 6 places and AND with 0111 to get 3 bits in remainder */
      yf = (uint8)  ((uint32)((row*resizeFactorY) >> 6) & 0x7);
  
      for (col=0; col < codx-1; col++)
      {
        x  = (uint16) ((uint32)  (col*resizeFactorX) >> 9);
        xf = (uint8)  ((uint32) ((col*resizeFactorX) >> 6) & 0x7);
  
        /*
        **    +--------+--------+
        **    |        |        |
        **    |        |        |
        **    |   A    |   B    |
        **    |        |        |
        **    +--------+--------+
        **    |      X |        |       X value is A*(1-fx)*(1-fy)
        **    |        |        |                  B*(fx)*(1-fy) 
        **    |   C    |   D    |                  C*(1-fx)*(fy)
        **    |        |        |                  D*(fx)*(fy)
        **    +--------+--------+
        */
  
        accum_W = accum_1 = accum_2 = accum_3 = 0;

        ewA = ewB = ewC = ewD = 1;
#if 0

        if (x < 2 || x > idx-2 || y < 2 || y > idy-2)
        {
          ewA = ewB = ewC = ewD = 1;
        }
        else
        {
          maxEp = 1000;
          dir = 0;

          // horizontal    (the period "." denotes where x,y take us in image
          //
          //  1  1  1  1
          //  1  1. 1  1
          // -1 -1 -1 -1
          // -1 -1 -1 -1
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x-1));
          ep = *yptr;
          ep += *(yptr + 1);
          ep += *(yptr + 2);
          ep += *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x));
          ep += *(yptr);
          ep += *(yptr + 1);
          ep += *(yptr + 2);
          ep += *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x+1));
          ep -= *yptr;
          ep -= *(yptr + 1);
          ep -= *(yptr + 2);
          ep -= *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x+2));
          ep -= *(yptr);
          ep -= *(yptr + 1);
          ep -= *(yptr + 2);
          ep -= *(yptr + 3);
          ep = IPL_ABS(ep);

          if (ep > maxEp)
          {
            maxEp = ep;
            dir = 1;
          }
          if (IPL_XFORM_DEBUG) 
            printf("\n(%d,%d)\nHoriz ep: %d\n", col, row, ep);


         
          // vertical 
          //
          // -1 -1  1  1
          // -1 -1  1  1
          // -1 -1  1  1
          // -1 -1  1  1
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x-1));
          ep = -1*(*yptr);
          ep -= *(yptr + 1);
          ep += *(yptr + 2);
          ep += *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x));
          ep -= *(yptr);
          ep -= *(yptr + 1);
          ep += *(yptr + 2);
          ep += *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x+1));
          ep -= *yptr;
          ep -= *(yptr + 1);
          ep += *(yptr + 2);
          ep += *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x+2));
          ep -= *(yptr);
          ep -= *(yptr + 1);
          ep += *(yptr + 2);
          ep += *(yptr + 3);
          ep = IPL_ABS(ep);

          if (ep > maxEp)
          {
            maxEp = ep;
            dir = 2;
          }
          if (IPL_XFORM_DEBUG) printf("Vert ep: %d\n", ep);

         
          // NE to SW
          //
          //  1  1  1  0
          //  1  1  0 -1
          //  1  0 -1 -1
          //  0 -1 -1 -1
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x-1));
          ep = *yptr;
          ep += *(yptr + 1);
          ep += *(yptr + 2);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x));
          ep += *(yptr);
          ep += *(yptr + 1);
          ep -= *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x+1));
          ep += *yptr;
          ep -= *(yptr + 1);
          ep -= *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x+2));
          ep -= *(yptr + 1);
          ep -= *(yptr + 2);
          ep -= *(yptr + 3);
          ep = IPL_ABS(ep);

          if (ep > maxEp)
          {
            maxEp = ep;
            dir = 3;
          }
          if (IPL_XFORM_DEBUG) printf("NESW ep: %d\n", ep);

          
          // NE to SW
          //
          //  0  1  1  1
          // -1  0  1  1
          // -1 -1  0  1
          // -1 -1 -1  0
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x-1));
          ep = *yptr;
          ep += *(yptr + 1);
          ep += *(yptr + 2);
          ep += *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x));
          ep += *(yptr);
          ep += *(yptr + 1);
          ep += *(yptr + 2);
          ep += *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x+1));
          ep = *yptr;
          ep -= *(yptr + 1);
          ep -= *(yptr + 2);
          ep -= *(yptr + 3);
          yptr = (ycbcrBuf.imgPtr + (y-1)*idx + (x+2));
          ep -= *(yptr);
          ep -= *(yptr + 1);
          ep -= *(yptr + 2);
          ep -= *(yptr + 3);
          ep = IPL_ABS(ep);
          if (ep > maxEp)
          {
            maxEp = ep;
            dir = 4;
          }
          if (IPL_XFORM_DEBUG) printf("NWSE ep: %d\n", ep);

          if (IPL_XFORM_DEBUG) 
          {
            printf("maxEp: %d\n", maxEp);
            printf("dir:   %d\n", dir);
          }

          switch(dir)
          {
            case 0:
              ewA = ewB = ewC = ewD = 1;
            break;
            case 1:
              ewA = ewB = 1;
              ewC = ewD = 0;
            break;
            case 2:
              ewA = ewC = 1;
              ewB = ewD = 0;
            break;
            case 3:
              ewA = 4;
              ewB = ewC = 1;
              ewD = 0;
            break;
            case 4:
              ewB = 4;
              ewA = ewD = 1;
              ewC = 0;
            break;
            default:
              ipl_sys_free(ycbcrBuf.imgPtr);
              return IPL_FAILURE;
            break;

            if (IPL_XFORM_DEBUG) 
              printf("Edge detected in direction %d\n", dir);
          }
        }
#endif

        /* A pixel */
        in = *(inImgPtr + (y)*idx + (x));
        if (in != tv)
        {
          w = biWeights[xf][yf][0];
          accum_1 = (uint16) (ewA * w * ((in & 0xF800) >>8));
          accum_2 = (uint16) (ewA * w * ((in & 0x07E0) >>3));
          accum_3 = (uint16) (ewA * w * ((in & 0x001F) <<3));
          accum_W += (uint16) (ewA * w);

          if (IPL_XFORM_DEBUG && 0)  //lint !e774 !e506
            printf("A: bi w was %d, but ewA of %d, makes total of %d\n",
                        w,ewA, w*ewA);
        }
  
        /* B pixel */
        in = *(inImgPtr + (y)*idx + (x+1));
        if (in != tv)
        {
          w = biWeights[xf][yf][1];
          accum_1 += (uint16) (ewB * w * ((in & 0xF800) >>8));
          accum_2 += (uint16) (ewB * w * ((in & 0x07E0) >>3));
          accum_3 += (uint16) (ewB * w * ((in & 0x001F) <<3));
          accum_W += (uint16) (ewB * w);
          if (IPL_XFORM_DEBUG && 0)  //lint !e774 !e506
            printf("B: bi w was %d, but ewB of %d, makes total of %d\n",
                        w,ewB, w*ewB);
        }
  
        /* C pixel */
        in = *(inImgPtr + (y+1)*idx + (x));
        if (in != tv)
        {
          w = biWeights[xf][yf][3];
          accum_1 += (uint16) (ewC * w * ((in & 0xF800) >>8));
          accum_2 += (uint16) (ewC * w * ((in & 0x07E0) >>3));
          accum_3 += (uint16) (ewC * w * ((in & 0x001F) <<3));
          accum_W += (uint16) (ewC * w);
          if (IPL_XFORM_DEBUG && 0)  //lint !e774 !e506
            printf("C: bi w was %d, but ewC of %d, makes total of %d\n",
                        w,ewC, w*ewC);
        }
  
        /* D pixel */
        in = *(inImgPtr + (y+1)*idx + (x+1));
        if (in != tv)
        {
          w = biWeights[xf][yf][2];
          accum_1 += (uint16) (ewD * w * ((in & 0xF800) >>8));
          accum_2 += (uint16) (ewD * w * ((in & 0x07E0) >>3));
          accum_3 += (uint16) (ewD * w * ((in & 0x001F) <<3));
          accum_W += (uint16) (ewD * w);
          if (IPL_XFORM_DEBUG && 0)  //lint !e774 !e506
            printf("D: bi w was %d, but ewD of %d, makes total of %d\n",
                        w,ewD, w*ewD);
        }
  
        /* divide by sum of the weights */
        if (accum_W != 0)
        {
          accum_1 /= (accum_W);
          accum_2 /= (accum_W);
          accum_3 /= (accum_W);
          *ptr16 = pack_rgb565(accum_1, accum_2, accum_3);
        }
        else
        {
          *ptr16 = tv;
        }

        ptr16++;
      } 
  
      // copy the last colum
      //y  = (uint16) ((uint32) (row*resizeFactorY) >> 9);
      x  = (uint16) ((uint32) (codx*resizeFactorX) >> 9);
      *ptr16 = *(inImgPtr + y*idx + x + 1);
      ptr16++;


      ptr16 += pitchx;
    } 


    inImgPtr = (uint16*)i_img_ptr->imgPtr + idx * (idy-1);
    for (col=0; col < codx; col++)
    {
      x  = (uint16) ((uint32) (col*resizeFactorX) >> 9);
      *ptr16 = *(inImgPtr + x);
      ptr16++;
    }
  }
  else
  {
    MSG_LOW("ipl_resizeFrame marker_203\n");
    return IPL_FAILURE;
  }

  // sys_free our tmep buffer
  //ipl_sys_free(ycbcrBuf.imgPtr);

  MSG_LOW("ipl_resizeFrame marker_100\n");
  
  return IPL_SUCCESS;
}





#if 0

/*===========================================================================
 *
 * FUNCTION ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB
 *
 * DESCRIPTION
 *   This function uses an optimized bilinear avreaging scheme to upsize an
 *   image any ratio.  This function will also rotate the image and color 
 *   convert.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   input_img_ptr     - input image frame
 *   output_img_ptr    - Points to the output image
 *   rdx               - how much to upsize image to in x dim
 *   rdy               - how much to upsize image to in y dim
 *   i_param           -  Parameters relating to upsize.
 *   rot               - Parameter relating to rotation
 *
 *
 * RETURN VALUE
 *   Status - success or failure
 *
 * SIDE EFFECTS
 *   Changes the output image buffer
 *
 *===========================================================================*/
static ipl_status_type
ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB
(
   ipl_image_type*        i_img_ptr, /* Points to the input image   */
   ipl_image_type*        o_img_ptr, /* Points to the output image  */
   int                    rdx,
   int                    rdy,
   ipl_rect_type*         cropout,
   ipl_rotate90_type      rot,
   ipl_quality_type       qual
)
{
  register uint16* outputImgPtr;
  uint16* outputImgPtrStart;
  register uint16 out, out2, out3, out4;
  register int32 cb, cr, r_factor;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register uint16 *rTable;
  register uint16 *gTable;
  register uint16 *bTable;
  uint32 row, col;
  uint8 xf, yf;
  uint8 xf2, yf2; 
  register uint16 x, y;
  register uint16 x2, y2;
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint32 idx, idxHalf, idy, odx;
  uint8 * inImgPtr1;
  uint8 * cbPtr;
  uint8 * crPtr;
  uint8 * temp;

  // these were written for 90 rotation
  // we swap x and y, and go up for every row

  MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr ||
      !cropout)
  {
    MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB marker_200\n");
    return IPL_FAILURE;
  }


#define getAvgLuma420_rot0(h,v,hf,vf,c) \
                      temp = (inImgPtr1 + (v*idx) + h);\
                      c =  *(temp)       * biWeights[hf][vf][0];\
                      c += *(temp+1)     * biWeights[hf][vf][1];\
                      c += *(temp+idx)   * biWeights[hf][vf][3];\
                      c += *(temp+idx+1) * biWeights[hf][vf][2];\
                      c = c >> 6;
#define PCb420_rot0(x,y)  (*(cbPtr + ((y)*idxHalf+x)))
#define PCr420_rot0(x,y)  (*(crPtr + ((y)*idxHalf+x)))





#define getAvgLuma420_rot90(h,v,hf,vf,c) \
                      temp = (inImgPtr1 - (h*idx) + v);\
                      c =  *(temp)       * biWeights[hf][vf][0];\
                      c += *(temp+1)     * biWeights[hf][vf][3];\
                      c += *(temp-idx)   * biWeights[hf][vf][1];\
                      c += *(temp-idx+1) * biWeights[hf][vf][2];\
                      c = c >> 6;
#define PCr420_rot90(x,y) (*(crPtr - ((x)*idxHalf)+y))
#define PCb420_rot90(x,y) (*(cbPtr - ((x)*idxHalf)+y))






#define getAvgLuma420_rot180(h,v,hf,vf,c) \
                      temp = (inImgPtr1 - (v*idx) - h);\
                      c =  *(temp)       * biWeights[hf][vf][0];\
                      c += *(temp-idx)   * biWeights[hf][vf][3];\
                      c += *(temp-1)     * biWeights[hf][vf][1];\
                      c += *(temp-idx-1) * biWeights[hf][vf][2];\
                      c = c >> 6;
#define PCr420_rot270(x,y) (*(crPtr + ((x)*idxHalf)-y))
#define PCb420_rot270(x,y) (*(cbPtr + ((x)*idxHalf)-y))




#define getAvgLuma420_rot270(h,v,hf,vf,c) \
                      temp = (inImgPtr1 + (h*idx) - v);\
                      c =  *(temp)       * biWeights[hf][vf][0];\
                      c += *(temp+idx)   * biWeights[hf][vf][1];\
                      c += *(temp-1)     * biWeights[hf][vf][3];\
                      c += *(temp+idx-1) * biWeights[hf][vf][2];\
                      c = c >> 6;
#define PCr420_rot180(x,y) (*(crPtr - ((y)*idxHalf)-x))
#define PCb420_rot180(x,y) (*(cbPtr - ((y)*idxHalf)-x))

  // initialze color tables
  if ( ipl2_init() != IPL_SUCCESS )
  {
    MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB marker_201\n");
    return IPL_FAILURE;
  }
  idx = i_img_ptr->dx;
  idxHalf = i_img_ptr->dx/2;
  idy = i_img_ptr->dy;

  odx = o_img_ptr->dx;

  if (rot == IPL_NOROT)
  {
    /*-----------------------------------------------------------------------*/
    // General algorithm:
    //
    // When generating the output, we work forward (to the right) in the 
    // x-dimension, and we go down in the y-direction. 
    //
    // For every pixel, we figure out which 4 pixels in the input are 
    // closest, and then use appropirate weights to combine them. 
    //
    /*-----------------------------------------------------------------------*/
    inImgPtr1 = i_img_ptr->imgPtr;
    cbPtr = i_img_ptr->imgPtr + idx * idy;
    crPtr = cbPtr + (idx*idy)/4;

    // Choose Q9 so you can upsize area from 10x10 to at 5120x5120, or
    // i.e. at least a factor of 512, 2^9
    resizeFactorX = (idx<<9) / rdx;
    resizeFactorY = (idy<<9) / rdy;
  }
  else if (rot == IPL_ROT90)
  {
    /*-----------------------------------------------------------------------*/
    // General algorithm:
    //
    // When generating the output, we work forward (to the right) in the 
    // x-dimension, but we go up in the y-direction. We swap x and y when 
    // read from the input image. 
    //
    // For every pixel, we figure out which 4 pixels in the input are 
    // closest, and then use appropirate weights to combine them. 
    //
    /*-----------------------------------------------------------------------*/
    inImgPtr1 = i_img_ptr->imgPtr + (idx * (idy - 1));
    cbPtr = i_img_ptr->imgPtr + idx * idy + ((idx/2)*((idy/2-1)));
    crPtr = cbPtr + (idx*idy)/4;

    // since rotating 90, x resize factor is now, y resize
    resizeFactorX = (idy<<9) / rdy;
    resizeFactorY = (idx<<9) / rdx;
  }
  else if (rot == IPL_ROT180)
  {
    /*-----------------------------------------------------------------------*/
    // General algorithm:
    //
    // When generating the output, we work to backward (to the left) in the 
    // x-dimension, and go up in the y-direction. 
    //
    // For every pixel, we figure out which 4 pixels in the input are 
    // closest, and then use appropirate weights to combine them. 
    //
    /*-----------------------------------------------------------------------*/
    inImgPtr1 = i_img_ptr->imgPtr + (idx * idy - 1);
    cbPtr = i_img_ptr->imgPtr + idx * idy + (idx*idy/4) - 1;
    crPtr = cbPtr + (idx*idy)/4;

    resizeFactorX = (idx<<9) / rdx;
    resizeFactorY = (idy<<9) / rdy;
  }
  else if (rot == IPL_ROT270)
  {
    /*-----------------------------------------------------------------------*/
    // General algorithm:
    //
    // When generating the output, we work in reverse (to the left ) in the 
    // x-dimension, and we go down in the y-direction. We swap x and y when
    // reading from the input.
    //
    // For every pixel, we figure out which 4 pixels in the input are 
    // closest, and then use appropirate weights to combine them. 
    /*-----------------------------------------------------------------------*/
    inImgPtr1 = i_img_ptr->imgPtr + idx  - 1;
    cbPtr = i_img_ptr->imgPtr + idx * idy + (idx/2) - 1;
    crPtr = cbPtr + (idx*idy)/4;

    // since rotating 270, x resize factor is now, y resize
    resizeFactorX = (idy<<9) / rdy;
    resizeFactorY = (idx<<9) / rdx;
  }
  else 
  {
    MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB marker_202\n");
    return IPL_FAILURE;
  }

  // where are putting the output
  outputImgPtr = (uint16 *) o_img_ptr->imgPtr;
  outputImgPtrStart = outputImgPtr + (cropout->x + o_img_ptr->dx*cropout->y);

  MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB marker_1\n");


  // setup table for doing color conversion
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB marker_203\n");

      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );

    default:
      MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB marker_204\n");
      return( IPL_FAILURE );
  }


  // do one output row at time 
  for (row = 0; row < cropout->dy; row += 2)
  {
    /* shift back by 9 place for integer part */
    y  = (uint16) ((uint32) (row*resizeFactorY) >> 9);

    /* shift 6 places and AND with 0111 to get 3 bits in remainder */
    yf = (uint8)  ((uint32)((row*resizeFactorY) >> 6) & 0x7);
    
    /* do second row */
    y2  = (uint16) ((uint32) ((row+1)*resizeFactorY) >> 9);
    yf2 = (uint8)  ((uint32)(((row+1)*resizeFactorY) >> 6) & 0x7);
                  

    // where putting output
    outputImgPtr = (outputImgPtrStart + (row * odx));
 
    // do do this row
    for (col = 0; col < cropout->dx; col += 2)
    {
      // do first column */
      x  = (uint16) ((uint32)  (col*resizeFactorX) >> 9);
      xf = (uint8)  ((uint32) ((col*resizeFactorX) >> 6) & 0x7);

      // do second column */
      x2  = (uint16) ((uint32)  ((col+1)*resizeFactorX) >> 9);
      xf2 = (uint8)  ((uint32) (((col+1)*resizeFactorX) >> 6) & 0x7);

      if (IPL_XFORM_DEBUG) //lint !e774 !e506
      {
        printf("wA%d wB%d wC%d wD%d\n", (8-xf)*(8-yf),(xf)*(8-yf),
                 (8-xf)*(yf),(xf)*(yf));
        printf("tA%d tB%d tC%d tD%d\n\n", biWeights[xf][yf][0],
        biWeights[xf][yf][1], biWeights[xf][yf][3], biWeights[xf][yf][2]);
      }

      // do bilinear interpolation for each pixel, do 4 pixles at a time 
      // so we can color convert with single Cb and singel Cr
      switch (rot)
      {
        case IPL_NOROT:
          getAvgLuma420_rot0(x, y, xf, yf, lumaa1);
          getAvgLuma420_rot0(x2,y, xf2,yf, lumaa2);  
          getAvgLuma420_rot0(x, y2,xf, yf2,lumaa3);
          getAvgLuma420_rot0(x2,y2,xf2,yf2,lumaa4);
          cb = PCb420_rot0((x/2),(y/2));
          cr = PCr420_rot0((x/2),(y/2));
        break;

        case IPL_ROT90:
          getAvgLuma420_rot90(x, y, xf, yf, lumaa1);
          getAvgLuma420_rot90(x2,y, xf2,yf, lumaa2);  
          getAvgLuma420_rot90(x, y2,xf, yf2,lumaa3);
          getAvgLuma420_rot90(x2,y2,xf2,yf2,lumaa4);
          cb = PCb420_rot90((x/2),(y/2));
          cr = PCr420_rot90((x/2),(y/2));
        break;

        case IPL_ROT180:
          getAvgLuma420_rot180(x, y, xf, yf, lumaa1);
          getAvgLuma420_rot180(x2,y, xf2,yf, lumaa2);  
          getAvgLuma420_rot180(x, y2,xf, yf2,lumaa3);
          getAvgLuma420_rot180(x2,y2,xf2,yf2,lumaa4);
          cb = PCb420_rot180((x/2),(y/2));
          cr = PCr420_rot180((x/2),(y/2));
        break;

        case IPL_ROT270:
          getAvgLuma420_rot270(x, y, xf, yf, lumaa1);
          getAvgLuma420_rot270(x2,y, xf2,yf, lumaa2);  
          getAvgLuma420_rot270(x, y2,xf, yf2,lumaa3);
          getAvgLuma420_rot270(x2,y2,xf2,yf2,lumaa4);
          cb = PCb420_rot270((x/2),(y/2));
          cr = PCr420_rot270((x/2),(y/2));
        break;

        default:
          MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB marker_205\n");
          return IPL_FAILURE;
      }


      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
       lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
       rTable, gTable, bTable );

      // write output
      *outputImgPtr = out;
      *(outputImgPtr++ + odx) = out3;
      *(outputImgPtr) = out2;
      *(outputImgPtr++ + odx) = out4;
    } 
  } 

  MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB marker_100\n");

  return(IPL_SUCCESS);
} 

#endif




/*===========================================================================
 *
 * FUNCTION ipl_bilinUpsize_Rot_Crop_YCbCr
 *
 * DESCRIPTION
 *   This function uses an optimized bilinear avreaging scheme to upsize an
 *   image any ratio.  This function will also rotate the image and color 
 *   convert.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   input_img_ptr     - input image frame
 *   output_img_ptr    - Points to the output image
 *   rdx               - how much to upsize image to in x dim
 *   rdy               - how much to upsize image to in y dim
 *   i_param           - Parameters relating to upsize.
 *   rot               - Parameter relating to rotation
 *
 *
 * RETURN VALUE
 *   Status - success or failure
 *
 * SIDE EFFECTS
 *   Changes the output image buffer
 *
 *
 *===========================================================================*/
static ipl_status_type
ipl_bilinUpsize_Rot_Crop_YCbCr
(
   ipl_image_type*        i_img_ptr, /* Points to the input image   */
   ipl_image_type*        o_img_ptr, /* Points to the output image  */
   int                    rdx,
   int                    rdy,
   ipl_rect_type*         cropout,
   ipl_rotate90_type      rot,
   ipl_quality_type       qual
)
{
  uint8* outputImgPtrStart8;
  register uint8* outputImgPtr8;
  register int32 cb, cr;
  register int32 lumaa1, lumaa2;
  uint32 row, col;
  uint8 xf, yf;
  uint8 xf2; 
  register uint16 x, y;
  register uint16 x2;
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint32 idx, idx2, idy, odx, ody;
  uint8 * inImgPtr1;
  uint8 * cbPtr;
  uint8 * temp;



  // this array tells that what we should add to our image pointers
  // if we expted Cb Y Cr Y (x axis of table) and got Cb Y Cr Y (y axis
  // of table)
  //  Anchor in Cb Y Cr Y is the first Y, i.e. 0 index is first Y 
  int CbMap[4] = {-1, -2,  -3,  -4};
  int CrMap[4] = {1,  0,   -1,  -2};

  // if we are going from right to left, we need separate table
  //  Anchor in Cb Y Cr Y is the second Y, i.e. 0 index is second Y 
  int CbMapRev[4] = {-3, -2,  -1,  0};
  int CrMapRev[4] = {-1,  0,  1,   2};

  //
  // h  is x pos of the pixel in the input
  // v  is y pos of the pixel in the input
  // hf is fractional part of x pos of the pixel in the input
  // vf is fractional part of y pos of the pixel in the input
  //
  //
  MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr ||
      !cropout)
  {
    MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_200\n");
    return IPL_FAILURE;
  }
  if ((i_img_ptr->cFormat == IPL_YCbCr && o_img_ptr->cFormat != IPL_YCbCr) && 
      (i_img_ptr->cFormat == IPL_YCbCr && o_img_ptr->cFormat != IPL_LUMA_ONLY))
  {
    MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_201\n");
    return IPL_FAILURE;
  }  

  
#define getAvgLuma_rot0(h,v,hf,vf,c) \
                      temp = (inImgPtr1 + (v*idx2) + (2*h));\
                      c =  *(temp)       * biWeights[hf][vf][0];\
                      c += *(temp+2)     * biWeights[hf][vf][1];\
                      c += *(temp+idx2)   * biWeights[hf][vf][3];\
                      c += *(temp+idx2+2) * biWeights[hf][vf][2];\
                      c = c >> 6;

#define getCb_rot0(h,v,hf,vf,c) \
                      temp = (inImgPtr1 + (v*idx2) + 2*h + CbMap[(2*h)%4]);\
                      c =  *(temp)      * biWeights[hf][vf][0];\
                      c += *(temp+idx2) * biWeights[hf][vf][3];\
                      if (h%2) \
                        temp += 4;\
                      c += *(temp)      * biWeights[hf][vf][1];\
                      c += *(temp+idx2) * biWeights[hf][vf][2];\
                      c = c >> 6;

#define getCr_rot0(h,v,hf,vf,c) \
                      temp = (inImgPtr1 + (v*idx2) + 2*h + CrMap[(2*h)%4]);\
                      c =  *(temp)      * biWeights[hf][vf][0];\
                      c += *(temp+idx2) * biWeights[hf][vf][3];\
                      if (h%2) \
                        temp += 4;\
                      c += *(temp)      * biWeights[hf][vf][1];\
                      c += *(temp+idx2) * biWeights[hf][vf][2];\
                      c = c >> 6;




#define getAvgLuma_rot90(h,v,hf,vf,c) \
                      temp = (inImgPtr1 - ((h)*idx2) + v*2);\
                      c =  *(temp)       * biWeights[hf][vf][0];\
                      c += *(temp-idx2)   * biWeights[hf][vf][1];\
                      c += *(temp+2)     * biWeights[hf][vf][3];\
                      c += *(temp-idx2+2) * biWeights[hf][vf][2];\
                      c = c >> 6;

#define getCb_rot90(h,v,hf,vf,c) \
                      temp = (inImgPtr1 - ((h)*idx2) + ((v*2)+CbMap[(2*v)%4]));\
                      c =  *(temp)       * biWeights[hf][vf][0];\
                      c += *(temp-idx2)   * biWeights[hf][vf][1];\
                      if (v%2) \
                        temp += 4;\
                      c += *(temp)     * biWeights[hf][vf][3];\
                      c += *(temp-idx2) * biWeights[hf][vf][2];\
                      c = c >> 6;

#define getCr_rot90(h,v,hf,vf,c) \
                      temp = (inImgPtr1 - ((h)*idx2) + ((v*2)+CrMap[(2*v)%4]));\
                      c =  *(temp)       * biWeights[hf][vf][0];\
                      c += *(temp-idx2)  * biWeights[hf][vf][1];\
                      if (v%2) \
                        temp += 4;\
                      c += *(temp)      * biWeights[hf][vf][3];\
                      c += *(temp-idx2) * biWeights[hf][vf][2];\
                      c = c >> 6;



#define getAvgLuma_rot180(h,v,hf,vf,c) \
                      temp = (inImgPtr1 - (v*idx2) - (2*h));\
                      c =  *(temp)       * biWeights[hf][vf][0];\
                      c += *(temp-idx2)   * biWeights[hf][vf][3];\
                      c += *(temp-2)     * biWeights[hf][vf][1];\
                      c += *(temp-idx2-2) * biWeights[hf][vf][2];\
                      c = c >> 6;

#define getCb_rot180(h,v,hf,vf,c) \
                      temp = (inImgPtr1 - (v*idx2) - (2*h) +CbMapRev[(2*h)%4]);\
                      c =  *(temp)        * biWeights[hf][vf][0];\
                      c += *(temp-idx2)   * biWeights[hf][vf][3];\
                      if (h%2) \
                        temp -= 4;\
                      c += *(temp)      * biWeights[hf][vf][1];\
                      c += *(temp-idx2) * biWeights[hf][vf][2];\
                      c = c >> 6;

#define getCr_rot180(h,v,hf,vf,c) \
                      temp = (inImgPtr1 - (v*idx2) - (2*h) +CrMapRev[(2*h)%4]);\
                      c =  *(temp)        * biWeights[hf][vf][0];\
                      c += *(temp-idx2)   * biWeights[hf][vf][3];\
                      if (h%2) \
                        temp -= 4;\
                      c += *(temp)      * biWeights[hf][vf][1];\
                      c += *(temp-idx2) * biWeights[hf][vf][2];\
                      c = c >> 6;




#define getAvgLuma_rot270(h,v,hf,vf,c) \
                      temp = (inImgPtr1 + (h*idx2) - (v*2));\
                      c =  *(temp)        * biWeights[hf][vf][0];\
                      c += *(temp+idx2)   * biWeights[hf][vf][1];\
                      c += *(temp-2)      * biWeights[hf][vf][3];\
                      c += *(temp+idx2-2) * biWeights[hf][vf][2];\
                      c = c >> 6;

#define getCb_rot270(h,v,hf,vf,c) \
                      temp = (inImgPtr1 + (h*idx2) - (v*2)+CbMapRev[(2*v)%4]);\
                      c =  *(temp)        * biWeights[hf][vf][0];\
                      c += *(temp+idx2)   * biWeights[hf][vf][1];\
                      if ((v%2)) \
                        temp -= 4;\
                      c += *(temp)      * biWeights[hf][vf][3];\
                      c += *(temp+idx2) * biWeights[hf][vf][2];\
                      c = c >> 6;

#define getCr_rot270(h,v,hf,vf,c) \
                      temp = (inImgPtr1 + (h*idx2) - (v*2)+CrMapRev[(2*v)%4]);\
                      c =  *(temp)        * biWeights[hf][vf][0];\
                      c += *(temp+idx2)   * biWeights[hf][vf][1];\
                      if ((v%2)) \
                        temp -= 4;\
                      c += *(temp)      * biWeights[hf][vf][3];\
                      c += *(temp+idx2)  * biWeights[hf][vf][2];\
                      c = c >> 6;



  // initialze colro tables
  if ( ipl2_init() != IPL_SUCCESS )
  {
    MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_202\n");
    return IPL_FAILURE;
  }
  idx = i_img_ptr->dx;
  idx2 = 2*i_img_ptr->dx;
  idy = i_img_ptr->dy;

  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;

  if (rot == IPL_NOROT)
  {
    /*-----------------------------------------------------------------------*/
    // General algorithm:
    //
    // When generating the output, we work forward (to the right) in the 
    // x-dimension, and we go down in the y-direction. 
    //
    // For every pixel, we figure out which 4 pixels in the input are 
    // closest, and then use appropirate weights to combine them. 
    //
    /*-----------------------------------------------------------------------*/
    cbPtr = i_img_ptr->imgPtr;
    inImgPtr1 = cbPtr + 1;

    // Choose Q9 so you can upsize area from 10x10 to at 5120x5120, or
    // i.e. at least a factor of 512, 2^9
    resizeFactorX = ((rdx-1)<<9) / odx;
    resizeFactorY = ((rdy-1)<<9) / ody;
  }
  else if (rot == IPL_ROT90)
  {
    /*-----------------------------------------------------------------------*/
    // General algorithm:
    //
    // When generating the output, we work forward (to the right) in the 
    // x-dimension, but we go up in the y-direction. We swap x and y when 
    // read from the input image. 
    //
    // For every pixel, we figure out which 4 pixels in the input are 
    // closest, and then use appropirate weights to combine them. 
    //
    /*-----------------------------------------------------------------------*/
    cbPtr = i_img_ptr->imgPtr + (2*idx * (idy - 1));
    inImgPtr1 = cbPtr + 1;

    // since rotating 90, x resize factor is now, y resize
    resizeFactorX = ((rdy-1)<<9) / odx;   // or is it /ody
    resizeFactorY = ((rdx-1)<<9) / ody;   // or is it /odx
  }
  else if (rot == IPL_ROT180)
  {
    /*-----------------------------------------------------------------------*/
    // General algorithm:
    //
    // When generating the output, we work to backward (to the left) in the 
    // x-dimension, and go up in the y-direction. 
    //
    // For every pixel, we figure out which 4 pixels in the input are 
    // closest, and then use appropirate weights to combine them. 
    //
    /*-----------------------------------------------------------------------*/
    cbPtr = i_img_ptr->imgPtr + (2*idx * idy - 4);
    inImgPtr1 = cbPtr + 3;

    resizeFactorX = ((rdx-1)<<9) / odx;
    resizeFactorY = ((rdy-1)<<9) / ody;
  }
  else if (rot == IPL_ROT270)
  {
    /*-----------------------------------------------------------------------*/
    // General algorithm:
    //
    // When generating the output, we work in reverse (to the left ) in the 
    // x-dimension, and we go down in the y-direction. We swap x and y when
    // reading from the input.
    //
    // For every pixel, we figure out which 4 pixels in the input are 
    // closest, and then use appropirate weights to combine them. 
    /*-----------------------------------------------------------------------*/
    cbPtr = i_img_ptr->imgPtr + 2*idx  - 4;
    inImgPtr1 = cbPtr + 3;

    // since rotating 270, x resize factor is now, y resize
    resizeFactorX = ((rdy-1)<<9) / odx; // or is it /ody
    resizeFactorY = ((rdx-1)<<9) / ody; // or is it /odx
  }
  else 
  {
    MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_1\n");

  if (o_img_ptr->cFormat == IPL_YCbCr)
  {
  // where are putting the output
  outputImgPtr8 = o_img_ptr->imgPtr;
  outputImgPtrStart8 = outputImgPtr8+2*(cropout->x + o_img_ptr->dx*cropout->y);

  // do one output row at time 
  for (row = 0; row < cropout->dy; row++)
  {
    /* shift back by 9 place for integer part */
    y  = (uint16) ((uint32) (row*resizeFactorY) >> 9);

    /* shift 6 places and AND with 0111 to get 3 bits in remainder */
    yf = (uint8)  ((uint32)((row*resizeFactorY) >> 6) & 0x07);
    
    outputImgPtr8 = (outputImgPtrStart8 + (row * 2*odx));

    // do this row
    for (col = 0; col < cropout->dx; col += 2)
    {
      // do first column */
      x  = (uint16) ((uint32)  (col*resizeFactorX) >> 9);
      xf = (uint8)  ((uint32) ((col*resizeFactorX) >> 6) & 0x07);

      // do second column */
      x2  = (uint16) ((uint32)  ((col+1)*resizeFactorX) >> 9);
      xf2 = (uint8)  ((uint32) (((col+1)*resizeFactorX) >> 6) & 0x07);

      if (IPL_XFORM_DEBUG && 0) //lint !e774 !e506
      {
        if (row == 0)
        {
          printf("col %lu, x  is %d, xf  is %d\n", col, x, xf);
          printf("col %lu, x2 is %d, xf2 is %d\n", col, x2, xf2);
        }
      }

      if (IPL_XFORM_DEBUG && 0) //lint !e774 !e506
      {
        printf("wA%d wB%d wC%d wD%d\n", (8-xf)*(8-yf),(xf)*(8-yf),
                 (8-xf)*(yf),(xf)*(yf));
        printf("tA%d tB%d tC%d tD%d\n\n", biWeights[xf][yf][0],
        biWeights[xf][yf][1], biWeights[xf][yf][3], biWeights[xf][yf][2]);
      }

      // do bilinear interpolation for each pixel, do 2 y pixles 
      // and one Cr and one Cb at a time 
      switch (rot)
      {
        case IPL_NOROT:
          getAvgLuma_rot0(x, y, xf,yf,lumaa1);
          getAvgLuma_rot0(x2,y,xf2,yf,lumaa2);  
          getCb_rot0(x,y,xf,yf,cb);  
          getCr_rot0(x,y,xf,yf,cr);  
        break;

        case IPL_ROT90:
          getAvgLuma_rot90(x, y, xf, yf, lumaa1);
          getAvgLuma_rot90(x2,y,xf2, yf, lumaa2);
          getCb_rot90(x,y,xf,yf,cb);  
          getCr_rot90(x,y,xf,yf,cr);  
        break;

        case IPL_ROT180:
          getAvgLuma_rot180(x, y, xf, yf, lumaa1);
          getAvgLuma_rot180(x2,y,xf2, yf, lumaa2);
          getCb_rot180(x,y,xf,yf,cb);  
          getCr_rot180(x,y,xf,yf,cr);  
        break;

        case IPL_ROT270:
          getAvgLuma_rot270(x, y, xf, yf, lumaa1);
          getAvgLuma_rot270(x2,y,xf2, yf, lumaa2);
          getCb_rot270(x,y,xf,yf,cb);  
          getCr_rot270(x,y,xf,yf,cr);  
        break;

        default:
          MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_204\n");
          return IPL_FAILURE;
      }

      // both chroma and luma
#if 1
      *outputImgPtr8++ = (uint8) cb;
      *outputImgPtr8++ = (uint8) lumaa1;
      *outputImgPtr8++ = (uint8) cr;
      *outputImgPtr8++ = (uint8) lumaa2;
#endif

      // luma only
#if 0
      *outputImgPtr8++ = (uint8) 128;
      *outputImgPtr8++ = (uint8) lumaa1;
      *outputImgPtr8++ = (uint8) 128;
      *outputImgPtr8++ = (uint8) lumaa2;
#endif

      // chroma only 
#if 0
      *outputImgPtr8++ = (uint8) 128;
      *outputImgPtr8++ = (uint8) 128;
      *outputImgPtr8++ = (uint8) cr;
      *outputImgPtr8++ = (uint8) 128;
#endif

    } 
  } 
  }
  else if (o_img_ptr->cFormat == IPL_LUMA_ONLY)
  {
  // where are putting the output
  outputImgPtr8 = o_img_ptr->imgPtr;
  outputImgPtrStart8 = outputImgPtr8+(cropout->x + o_img_ptr->dx*cropout->y);

  // do one output row at time 
  for (row = 0; row < cropout->dy; row++)
  {
    /* shift back by 9 place for integer part */
    y  = (uint16) ((uint32) (row*resizeFactorY) >> 9);

    /* shift 6 places and AND with 0111 to get 3 bits in remainder */
    yf = (uint8)  ((uint32)((row*resizeFactorY) >> 6) & 0x07);
    
    outputImgPtr8 = (outputImgPtrStart8 + (row * odx));

    // do this row
    for (col = 0; col < cropout->dx; col += 2)
    {
      // do first column */
      x  = (uint16) ((uint32)  (col*resizeFactorX) >> 9);
      xf = (uint8)  ((uint32) ((col*resizeFactorX) >> 6) & 0x07);

      // do second column */
      x2  = (uint16) ((uint32)  ((col+1)*resizeFactorX) >> 9);
      xf2 = (uint8)  ((uint32) (((col+1)*resizeFactorX) >> 6) & 0x07);

      if (IPL_XFORM_DEBUG && 0) //lint !e774 !e506
      {
        if (row == 0)
        {
          printf("col %lu, x  is %d, xf  is %d\n", col, x, xf);
          printf("col %lu, x2 is %d, xf2 is %d\n", col, x2, xf2);
        }
      }

      if (IPL_XFORM_DEBUG && 0) //lint !e774 !e506
      {
        printf("wA%d wB%d wC%d wD%d\n", (8-xf)*(8-yf),(xf)*(8-yf),
                 (8-xf)*(yf),(xf)*(yf));
        printf("tA%d tB%d tC%d tD%d\n\n", biWeights[xf][yf][0],
        biWeights[xf][yf][1], biWeights[xf][yf][3], biWeights[xf][yf][2]);
      }

      // do bilinear interpolation for each pixel, do 2 y pixles 
      // and one Cr and one Cb at a time 
      switch (rot)
      {
        case IPL_NOROT:
          getAvgLuma_rot0(x, y, xf,yf,lumaa1);
          getAvgLuma_rot0(x2,y,xf2,yf,lumaa2);  
        break;

        case IPL_ROT90:
          getAvgLuma_rot90(x, y, xf, yf, lumaa1);
          getAvgLuma_rot90(x2,y,xf2, yf, lumaa2);
        break;

        case IPL_ROT180:
          getAvgLuma_rot180(x, y, xf, yf, lumaa1);
          getAvgLuma_rot180(x2,y,xf2, yf, lumaa2);
        break;

        case IPL_ROT270:
          getAvgLuma_rot270(x, y, xf, yf, lumaa1);
          getAvgLuma_rot270(x2,y,xf2, yf, lumaa2);
        break;

        default:
          MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_205\n");
          return IPL_FAILURE;
      }

      // both chroma and luma
      *outputImgPtr8++ = (uint8) lumaa1;
      *outputImgPtr8++ = (uint8) lumaa2;
    }
  }
  }
  else
  {
    MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_100\n");
    return(IPL_SUCCESS);
  }
  MSG_LOW("ipl_bilinUpsize_Rot_Crop_YCbCr marker_101\n");
  return(IPL_SUCCESS);
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_xform_Upsize_qLow

DESCRIPTION
  This function is used to do upsize
  The input should be RGB565 and the output should be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_xform_Upsize_qLow
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  crop
)
{
  uint16 row,col;
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  register uint16 x, y;
  register uint16* ptr16;
  register uint8* ptr8;

  uint32 cx, cy, cdx, cdy;
  uint32 idx,idy;
  uint32 odx,ody;

  MSG_LOW("ipl_xform_Upsize_qLow marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_xform_Upsize_qLow marker_200\n");
    return IPL_FAILURE;
  }
  ptr16 = (uint16*)o_img_ptr->imgPtr;
  ptr8 = o_img_ptr->imgPtr;

#define Pi(x,y)  (*(uint16*) (i_img_ptr->imgPtr+((cy+y)*idx+cx+x)*2));
#define P8ptr(x,y)  (*(uint16*) (i_img_ptr->imgPtr+((cy+y)*idx+cx+x)));

  if (crop == NULL)
  {
    cx = 0;
    cy = 0;
    cdx = i_img_ptr->dx;
    cdy = i_img_ptr->dy;
  }
  else
  { 
    cx = crop->x;
    cy = crop->y;
    cdx = crop->dx;
    cdy = crop->dy;
  }

  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;
  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;


  // make sure crop parameter is okay
  if ((cdx+cx > idx) || (cdy+cy > idy))
  {
    MSG_LOW("ipl_xform_Upsize_qLow marker_202\n");
    return IPL_FAILURE;
  }


  /*
  ** Only RGB565 Input and RGB565 Output is supported
  */
  if (i_img_ptr->cFormat != o_img_ptr->cFormat)
  {
    MSG_LOW("ipl_xform_Upsize_qLow marker_203\n");
    return IPL_FAILURE;
  }

  /*
  ** Q7 resize Factors (no rounding!)
  */
  resizeFactorX = (cdx<<9) / odx;
  resizeFactorY = (cdy<<9) / ody;

  MSG_LOW("ipl_xform_Upsize_qLow marker_1\n");

  if (i_img_ptr->cFormat == IPL_RGB565)
  {
    /* go to start position */
    for (row=0; row < ody; row++)
    {
      y  = (uint16) ((uint32)(row*resizeFactorY) >> 9);
      for (col=0; col < odx; col++)
      {
        x  = (uint16) ((uint32)(col*resizeFactorX) >> 9);
        *ptr16 = Pi(x,y);  
        ptr16++;
      }  
    }
  } 
  else if (i_img_ptr->cFormat == IPL_RGB888)
  {
    /* go to start position */
    for (row=0; row < ody; row++)
    {
      y  = (uint16) ((uint32)(row*resizeFactorY) >> 9);
      for (col=0; col < odx; col++)
      {
        x  = (uint16) ((uint32)(col*resizeFactorX) >> 9);

        *ptr8++ = (*(i_img_ptr->imgPtr+((cy+y)*idx+cx+x)*3));
        *ptr8++ = (*(i_img_ptr->imgPtr+((cy+y)*idx+cx+x)*3+1));
        *ptr8++ = (*(i_img_ptr->imgPtr+((cy+y)*idx+cx+x)*3+2));
      }  
    }
  }
  else if (i_img_ptr->cFormat == IPL_LUMA_ONLY)
  {
    /* go to start position */
    for (row=0; row < ody; row++)
    {
      y  = (uint16) ((uint32)(row*resizeFactorY) >> 9);
      for (col=0; col < odx; col++)
      {
        x  = (uint16) ((uint32)(col*resizeFactorX) >> 9);
        *ptr8 = (uint8) P8ptr(x,y);  
        ptr8++;
      }  
    }
  } 
  else
  {
    MSG_LOW("ipl_xform_Upsize_qLow marker_204\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_xform_Upsize_qLow marker_100\n");
  return IPL_SUCCESS;
}






/* <EJECT> */
/*===========================================================================

FUNCTION ipl_xform_Upsize_Crop_qLow

DESCRIPTION
  This function is used to do upsize
  The input should be RGB565 and the output should be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image
  cropi input crop
  cropo where to put in output

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_xform_Upsize_Crop_qLow
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  cropi,
  ipl_rect_type*  cropo
)
{
  uint32 row,col;
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  register uint16 x, y;
  register uint16* ptr16;
  register uint8* ptr8;

  uint32 cix, ciy, cidx, cidy;
  uint32 cox, coy, codx, cody;

  uint32 idx,idy;
  uint32 odx,ody;

  MSG_LOW("ipl_xform_Upsize_Crop_qLow marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_xform_Upsize_Crop_qLow marker_200\n");
    return IPL_FAILURE;
  }
  ptr16 = (uint16*)o_img_ptr->imgPtr;
  ptr8 = o_img_ptr->imgPtr;

#define Pic(x,y)  (*(uint16*) (i_img_ptr->imgPtr+((ciy+y)*idx+cix+x)*2));

  if (cropi == NULL)
  {
    cix = 0;
    ciy = 0;
    cidx = i_img_ptr->dx;
    cidy = i_img_ptr->dy;
  }
  else
  { 
    cix = cropi->x;
    ciy = cropi->y;
    cidx = cropi->dx;
    cidy = cropi->dy;
  }

  if (cropo == NULL)
  {
    cox = 0;
    coy = 0;
    codx = o_img_ptr->dx;
    cody = o_img_ptr->dy;
  }
  else
  { 
    cox = cropo->x;
    coy = cropo->y;
    codx = cropo->dx;
    cody = cropo->dy;
  }


  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;
  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;

  // make sure crop parameter is okay
  if (((codx+cox > odx) || (cody+coy > ody)) ||
      ((cidx+cix > idx) || (cidy+ciy > idy)))
  {
    MSG_LOW("ipl_xform_Upsize_Crop_qLow marker_201\n");
    return IPL_FAILURE;
  }


  /*
  ** Only RGB565 Input and RGB565 Output is supported
  */
  if (i_img_ptr->cFormat != o_img_ptr->cFormat)
  {
    MSG_LOW("ipl_xform_Upsize_Crop_qLow marker_202\n");
    return IPL_FAILURE;
  }

  /*
  ** Q7 resize Factors (no rounding!)
  */
  resizeFactorX = (cidx<<9) / codx;
  resizeFactorY = (cidy<<9) / cody;

  MSG_LOW("ipl_xform_Upsize_Crop_qLow marker_1\n");

  if (i_img_ptr->cFormat == IPL_RGB565)
  {
    /* go to start position */
    for (row=0; row < cody; row++)
    {
      y  = (uint16) ((uint32)(row*resizeFactorY) >> 9);
      for (col=0; col < codx; col++)
      {
        x  = (uint16) ((uint32)(col*resizeFactorX) >> 9);
        *ptr16 = Pic(x,y);  
        ptr16++;
      }  

      // add in pitch
      ptr16 += (o_img_ptr->dx - codx);
    }
  } 
  else if (i_img_ptr->cFormat == IPL_RGB888)
  {
    /* go to start position */
    for (row=0; row < cody; row++)
    {
      y  = (uint16) ((uint32)(row*resizeFactorY) >> 9);
      for (col=0; col < codx; col++)
      {
        x  = (uint16) ((uint32)(col*resizeFactorX) >> 9);

        *ptr8++ = (*(i_img_ptr->imgPtr+((ciy+y)*idx+cix+x)*3));
        *ptr8++ = (*(i_img_ptr->imgPtr+((ciy+y)*idx+cix+x)*3+1));
        *ptr8++ = (*(i_img_ptr->imgPtr+((ciy+y)*idx+cix+x)*3+2));
      }  

      ptr8 += 3*(o_img_ptr->dx - codx);
    }
  }
  else
  {
    MSG_LOW("ipl_xform_Upsize_Crop_qLow marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_xform_Upsize_Crop_qLow marker_100\n");

  return IPL_SUCCESS;
}


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_resize_ycrcblp

DESCRIPTION
  This function performs cropping on input, downsizing/upsizing, and then 
  convestion to rgb

  The input must be YCxCx420 or YCxCx422 line pack;

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         pointer to the input image
  o_img_ptr         pointer to the output image
  cropin            region on input to be cropped
  cropout           region on output to be cropped

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_crop_resize_ycrcblp
(
  ipl_image_type* i_img_ptr,        /* Points to the input image        */
  ipl_image_type* o_img_ptr,        /* Points to the output image       */
  ipl_rect_type* cropin,            /* Crop config                      */
  ipl_rect_type* cropout,           /* Crop config                      */
  ipl_quality_type qual 
)
{
  uint32 odx;     /* size of output row in no of pixels */
  uint32 idx;     /* size of input row in no pixels */
  uint32 ody;     /* size of output coloumn in no of pixels */
  uint32 idy;     /* size of output coloumn in no of pixels */

  uint32 cix, ciy, cidx, cidy;
  uint32 cox, coy, codx, cody;
  uint32 resizeFactorX;  /* sampling facotr in in X dirction */
  uint32 resizeFactorY;  /* sampling factor in Y direction */

  uint32 cicoffset;
  uint32 row, col, i, i2, j, j2;  /* index variables */

  uint8 *oyptr;
  uint8 *ocptr;
  uint8 *iyptr;

  MSG_LOW("ipl_crop_resize_ycrcblp marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_resize_ycrcblp marker_200\n");
    return IPL_FAILURE;
  }
  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;
  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;

  if (cropin == NULL)
  {
    cix = 0;
    ciy = 0;
    cidx = idx;
    cidy = idy;
  }
  else
  {
    cix = cropin->x;
    ciy = cropin->y;
    cidx = cropin->dx;
    cidy = cropin->dy;
  }

  if (cropout == NULL)
  {
    cox = 0;
    coy = 0;
    codx = odx;
    cody = ody;
  }
  else
  {
    cox = cropout->x;
    coy = cropout->y;
    codx = cropout->dx;
    cody = cropout->dy;
  }

  // in 420, we must have even width and height when we crop
  if ((cidx & 0x1) || (cidy & 0x1) || (codx & 0x1) || (cody & 0x1))
  {
    MSG_LOW("ipl_crop_resize_ycrcblp marker_201\n");
    return IPL_FAILURE;
  }

  if ((cox + codx > odx) || (cix + cidx > idx) ||
      (coy + cody > ody) || (ciy + cidy > idy))
  {
    MSG_LOW("ipl_crop_resize_ycrcblp marker_202\n");
    return IPL_FAILURE;
  }
  cicoffset = (2*(cix/2)) + ciy/2 * idx;


  /*------------------------------------------------------------------------
      Calculate X and Y resize factors
  ------------------------------------------------------------------------*/
  resizeFactorX = ((cidx << 7) + (codx >> 1)) / codx;
  resizeFactorY = ((cidy << 7) + (cody >> 1)) / cody;

  oyptr = o_img_ptr->imgPtr + (2*(cox/2)) + coy * odx;
  ocptr = o_img_ptr->clrPtr + (2*(cox/2)) + coy/2 * odx;

  iyptr = i_img_ptr->imgPtr + (2*(cix/2)) + ciy * idx;

  MSG_LOW("ipl_crop_resize_ycrcblp marker_1\n");

  if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
  {
    if (o_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
    {
      for (row=0; row < cody; row += 2)
      {
        j = (uint16)(( (uint32)( (row   *resizeFactorY) << 9) + 0x8000L) >> 16);
        j2 =(uint16)(( (uint32)(((row+1)*resizeFactorY) << 9) + 0x8000L) >> 16);

        for (col=0; col < codx; col += 2)
        {
          i = (uint16) (((uint32)   ((col *  resizeFactorX) << 9)+0x8000L)>>16);
          i2 = (uint16)(((uint32)  (((col+1)*resizeFactorX) << 9)+0x8000L)>>16);

          // write 4 new luma value
          *oyptr        = *(iyptr + (j *idx+i));
          *(oyptr + odx)= *(iyptr + (j2*idx+i));
          oyptr++;

          *(oyptr)      = *(iyptr + (j *idx+i2));
          *(oyptr + odx)= *(iyptr + (j2*idx+i2));
          oyptr++;

          // write 2 new chroma value
          if((i & 0x1))
            *ocptr++ = *(uint8*)(i_img_ptr->clrPtr + cicoffset + (j/2*idx+i+1));
          else
            *ocptr++ = *(uint8*)(i_img_ptr->clrPtr + cicoffset + (j/2*idx+i));

          if((i & 0x1))
            *ocptr++ = *(uint8*)(i_img_ptr->clrPtr + cicoffset + (j/2*idx+i));
          else
            *ocptr++ = *(uint8*)(i_img_ptr->clrPtr + cicoffset + (j/2*idx+i+1));
        }
  
        oyptr += (odx - codx + odx);
        ocptr += (odx - codx);
      }
    } 
  }
  else
  {
    MSG_LOW("ipl_crop_resize_ycrcblp marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_crop_resize_ycrcblp marker_100\n");
  return IPL_SUCCESS;
}



#if 0


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_resize_ycbcr2rgb

DESCRIPTION
  This function performs cropping on input, downsizing/upsizing, and then 
  convestion to rgb

  The input must be YCxCx420 or YCxCx422 line pack;

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         pointer to the input image
  o_img_ptr         pointer to the output image
  cropin            region on input to be cropped
  cropout           region on output to be cropped

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_crop_resize_ycbcr2rgb
(
  ipl_image_type* i_img_ptr,        /* Points to the input image        */
  ipl_image_type* o_img_ptr,        /* Points to the output image       */
  ipl_rect_type* cropin,            /* Crop config                      */
  ipl_rect_type* cropout,           /* Crop config                      */
  ipl_quality_type qual 
)
{
  uint32 odx;     /* size of output row in no of pixels */
  uint32 idx;     /* size of input row in no pixels */
  uint32 ody;     /* size of output coloumn in no of pixels */
  uint32 idy;     /* size of output coloumn in no of pixels */

  uint32 cix, ciy, cidx, cidy;
  uint32 cox, coy, codx, cody;
  uint32 resizeFactorX;  /* sampling facotr in in X dirction */
  uint32 resizeFactorY;  /* sampling factor in Y direction */
  uint8 cb, cr, luma;    /* YCbCr variables */

  uint16 *out_img_buf;
  uint32 cooffset;
  uint32 cicoffset;
  uint32 ciyoffset;
  uint32 row, col, i, j; /* index variables */
  uint16 out;            /* the o/p RGB variable */
  uint16 *rTable;
  uint16 *gTable;
  uint16 *bTable;
  int32 r;

  MSG_LOW("ipl_crop_resize_ycbcr2rgb marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_resize_ycbcr2rgb marker_200\n");
    return IPL_FAILURE;
  }

  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;
  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;


  if (cropin == NULL)
  {
    cix = 0;
    ciy = 0;
    cidx = idx;
    cidy = idy;
  }
  else
  {
    cix = cropin->x;
    ciy = cropin->y;
    cidx = cropin->dx;
    cidy = cropin->dy;
  }

  if (cropout == NULL)
  {
    cox = 0;
    coy = 0;
    codx = odx;
    cody = ody;
  }
  else
  {
    cox = cropout->x;
    coy = cropout->y;
    codx = cropout->dx;
    cody = cropout->dy;
  }

  // init some tables
  if (ipl2_init() != IPL_SUCCESS)
  {
    MSG_LOW("ipl_crop_resize_ycbcr2rgb marker_201\n");
    return IPL_FAILURE;
  }
  /*------------------------------------------------------------------------
      Initialize the conversion table to RGB 565 or 444
  ------------------------------------------------------------------------*/
  MSG_LOW("ipl_crop_resize_ycbcr2rgb marker_1\n");

  switch (o_img_ptr->cFormat)
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
      --------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
        initialize the conversion with RGB444 table
      --------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    default:
      MSG_LOW("ipl_crop_resize_ycbcr2rgb marker_202\n");
      return( IPL_FAILURE );
      /*--------------------------------------------------------------------
        Lint complains -- ignoring
       -------------------------------------------------------------------*/
      /*NOTREACHED*/
      break;
  }



  // in 420, we must have even width and height when we crop
  //if ((cidx % 2) || (cidy %2) || (codx % 2) || (cody %2))
  if ((cidx % 2) || (cidy %2))
  {
    MSG_LOW("ipl_crop_resize_ycbcr2rgb marker_203\n");
    return IPL_FAILURE;
  }
  if ((cox + codx > odx) || (cix + cidx > idx) ||
      (coy + cody > ody) || (ciy + cidy > idy))
  {
    MSG_LOW("ipl_crop_resize_ycbcr2rgb marker_204\n");
    return IPL_FAILURE;
  }
  ciyoffset = (2*(cix/2)) + ciy * idx;
  cicoffset = (2*(cix/2)) + ciy/2 * idx;

  cooffset = cox + coy * odx;

  // grab even Y and always start with Cb
  /*
  if (cicoffset % 2)
    cicoffset--;
  if (ciyoffset % 2)
    ciyoffset--;

  if (cooffset % 2)
    cooffset--;
   */


  out_img_buf = (uint16 *) o_img_ptr->imgPtr;

  /*------------------------------------------------------------------------
      Calculate X and Y resize factors
  ------------------------------------------------------------------------*/
  resizeFactorX = (cidx * 128 + (codx >> 1)) / codx;
  resizeFactorY = (cidy * 128 + (cody >> 1)) / cody;


  if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
  {
    for (row=0; row < cody; row++ )
    {
      i = (uint16)(( (uint32)( (row * resizeFactorY) << 9) + 0x8000L) >> 16);
      if (i >= cidy) 
        i = (cidy-1);

      for ( col=0; col < codx; col++ )
      {
        j = (uint16) (((uint32) ( (col * resizeFactorX) <<9 ) + 0x8000L)>>16);
        if (j >= cidx) 
          j = (cidx-1);

        /*
        if (col == codx-1)
        {
          printf("grab luma input from   %d,%d\n", j, i);
          printf("grab chroma input from %d,%d\n", j, i/2);
        }
        */

        if( ! (j & 0x1) )
        {
          cr = *(uint8*)(i_img_ptr->clrPtr +    cicoffset + (i/2*idx+j));
          cb = *(uint8*)(i_img_ptr->clrPtr +    cicoffset + (i/2*idx+j+1));
          luma = *((uint8*)(i_img_ptr->imgPtr + ciyoffset + (i*idx+j)));
          IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);
        }
        else
        {
          cb = *(uint8*)(i_img_ptr->clrPtr + cicoffset + (i/2*idx+j));
          cr = *(uint8*)(i_img_ptr->clrPtr + cicoffset + (i/2*idx+j+1));
          luma = *((uint8*)(i_img_ptr->imgPtr + ciyoffset + (i*idx+j)));
          IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);
        }
        *(uint16 *) (out_img_buf + cooffset + ((row * odx) + col) ) = out;
      }
    } 
  }

  MSG_LOW("ipl_crop_resize_ycbcr2rgb marker_100\n");

  return IPL_SUCCESS;
}


#endif



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_downsize_mn

DESCRIPTION
  This function is used to do resize and color convert.

  The input can be YCbCr, rgb565 or Bayer or rgb888
  The output can be YCbCr or rgb565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image
  bip_ptr points to a bip structure (for Mega Pixel Color Processing Support)
          The bip_ptr can be set to NULL if no Color Processing is desired.
          Demosaicing will still be done when the bip_ptr is set to NULL.

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_crop_downsize_mn
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  cropin,
  ipl_rect_type*  cropout,
  ipl_bip_type*   bip_ptr
)
{
  int32 k,n,nv;
  uint32 m,mv;
  int32 count=0,step=0,accum_r=0,accum_g=0,accum_b=0;
  int32* row_map;
  int32* col_map;
  uint16 out;
  uint32 row,col;
  int32 i,j;
  int32 r,g,b;

  uint8 r2,g2,b2;
  uint8 cb,cr,luma1,luma2;
  int32 cdb,cdr,lumad1,lumad2;
  int32 rc,gc,bc;

  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are = 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int16 ycbcr2rgb_convert[6] = {8, 25803, -3071, -7672,30399,12};
  uint8 failed = 0;

  const uint16 *rTable;
  const uint16 *gTable;
  const uint16 *bTable;
  int offset, offsetC;

  uint16 *ptr16;
  uint8 *outputImgPtr;
  int dx, odx;

  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  int16 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_crop_downsize_mn marker_0\n");

  IPLMSG("Inside ipl_crop_downsize_mn"); //lint !e774 !e506

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_downsize_mn marker_200\n");
    return IPL_FAILURE;
  }  

  if ((i_img_ptr->dx < o_img_ptr->dx) ||
      (i_img_ptr->dy < o_img_ptr->dy))
  {
    MSG_LOW("ipl_crop_downsize_mn marker_201\n");
    /*
    ** Strictly a downsize
    */
    return IPL_FAILURE;
  }

  // make sure input size is reasonable
  if (!i_img_ptr->dx || !i_img_ptr->dy || !o_img_ptr->dx || !o_img_ptr->dy)
  {
    MSG_LOW("ipl_crop_downsize_mn marker_201.0\n");
    return IPL_FAILURE;
  }

  /*
  ** Malloc the 2 line buffers
  */
  row_map = (int32*)ipl_malloc((o_img_ptr->dx+1)*sizeof(int32));
  if (!row_map) 
  {
    MSG_LOW("ipl_crop_downsize_mn marker_202\n");
    return IPL_NO_MEMORY;
  }

  col_map = (int32*)ipl_malloc((o_img_ptr->dy+1)*sizeof(int32));
  if (!col_map) 
  {
    ipl_sys_free(row_map);
    MSG_LOW("ipl_crop_downsize_mn marker_203\n");
    return IPL_NO_MEMORY;
  }

  MSG_LOW("ipl_crop_downsize_mn marker_1\n");

  if (cropout == NULL || cropin == NULL)
  {
    ipl_sys_free(col_map);
    ipl_sys_free(row_map);
    MSG_LOW("ipl_crop_downsize_mn marker_203.1\n");
    return IPL_FAILURE;
  }

  /*
  ** Set starting bin to 0
  */
  row_map[0] = -1;
  col_map[0] = -1;
  m = cropout->dx;
  n = cropin->dx;
  mv = cropout->dy;
  nv = cropin->dy;

  dx = i_img_ptr->dx;
  odx = o_img_ptr->dx;

  /*
  ** Q7 resize Factors
  */

  step = 1;
  count = 0;

  for (k=0;k<n;k++)
  {
    count = count + m;
    if (count >= n)
    {
      count = count - n;
      row_map[step++] = (int32)k;
    }
  }

  step = 1;
  count = 0;

  for (k=0;k<nv;k++)
  {
    count = count + mv;
    if (count >= nv)
    {
      count = count - nv;
      col_map[step++] = (int32)k;
    }
  }

  if (IPL_XFORM_DEBUG) //lint !e774 !e506
  {
    printf("Col Map\n");
    for (col=0;col<o_img_ptr->dx;col++)
      printf("%lu ", col_map[col]);
    printf("\n");

    printf("Row Map\n");
    for (row=0;row<o_img_ptr->dy;row++)
      printf("%lu ", row_map[row]);
    printf("\n");
  }



  if (i_img_ptr->cFormat == IPL_RGB565)
  {
    offset = (cropin->x + i_img_ptr->dx*cropin->y)*2;

    if (o_img_ptr->cFormat == IPL_RGB565)
    {
      /*
      ** Resize from RGB565 to RGB565
      */
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          accum_r=0;
          accum_g=0;
          accum_b=0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              out = *((uint16*)(i_img_ptr->imgPtr + offset + (i*dx+j)*2));
              unpack_rgb565(out,&r2,&g2,&b2);
              accum_r += r2;
              accum_g += g2;
              accum_b += b2;
            }
          }
          accum_r = (accum_r<<3)/count;
          accum_r += 0x4;
          accum_r = accum_r >> 3;
          accum_g = (accum_g<<3)/count;
          accum_g += 0x4;
          accum_g = accum_g >> 3;
          accum_b = (accum_b<<3)/count;
          accum_b += 0x4;
          accum_b = accum_b >> 3;
          /*
          ** Write out the resize value
          */
          k = k+1;
          out = pack_rgb565(accum_r,accum_g,accum_b);
          *(uint16*)(o_img_ptr->imgPtr + (((row*odx)+col)*2))= out;
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } 
    else 
    {
      /*
      ** Resize from RGB565 to YCbCr
      */
      failed = 1;
    }
  }
  else if (i_img_ptr->cFormat == IPL_RGB888)
  {
    offset = (cropin->x + i_img_ptr->dx*cropin->y)*3;

    if (o_img_ptr->cFormat == IPL_RGB888)
    {
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          accum_r=0;
          accum_g=0;
          accum_b=0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              accum_r += *(i_img_ptr->imgPtr + offset+(i*dx+j)*3);
              accum_g += *(i_img_ptr->imgPtr + offset+(i*dx+j)*3+1);
              accum_b += *(i_img_ptr->imgPtr + offset+(i*dx+j)*3+2);
            }
          }
          accum_r = (accum_r<<3)/count;
          accum_r += 0x4;
          accum_r = accum_r >> 3;

          accum_g = (accum_g<<3)/count;
          accum_g += 0x4;
          accum_g = accum_g >> 3;

          accum_b = (accum_b<<3)/count;
          accum_b += 0x4;
          accum_b = accum_b >> 3;

          /*
          ** Write out the resize value
          */
          k = k+1;

          *(o_img_ptr->imgPtr + (((row*odx)+col)*3))= (unsigned char) accum_r;
          *(o_img_ptr->imgPtr + (((row*odx)+col)*3)+1)= (unsigned char) accum_g;
          *(o_img_ptr->imgPtr + (((row*odx)+col)*3)+2)= (unsigned char) accum_b;
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } 
    else 
    {
      /*
      ** Resize from RGB565 to YCbCr
      */
      failed = 1;
    }
  } 
  else if (i_img_ptr->cFormat == IPL_YCbCr)
  {
    // make sure input size is reasonable
    if (i_img_ptr->dx < 2)
    {
      ipl_sys_free(col_map);
      ipl_sys_free(row_map);

      MSG_LOW("ipl_crop_downsize_mn marker_201.00\n");
      return IPL_FAILURE;
    }

    offset = (2*(cropin->x/2) + i_img_ptr->dx*cropin->y)*2;

    /* Input is YCbCr */
    if (o_img_ptr->cFormat == IPL_RGB565)
    {
      /*
      ** Resize from YCbCr to RGB565
      */
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          accum_r=0;
          accum_g=0;
          accum_b=0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              if (!(j%2))
              {
                /* Cb */
                cb = *(uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j)*2);
                luma1 = *((uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j)*2+1));
                cr = *(uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j+1)*2);
                luma2 = *((uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j+1)*2+1));
                rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                      ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
                gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                      ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
                bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                      ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
                cdb = luma1 + (rc>>16);
                cdr = luma1 + (gc>>16);
                lumad1 = luma1 + (bc>>16);
                r= (uint8)CLIPIT(cdb);
                g= (uint8)CLIPIT(cdr);
                b= (uint8)CLIPIT(lumad1);
              } else {
                /* cr */
                cr = *((uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j)*2));
                luma2 = *((uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j)*2+1));
                cb = *(uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j-1)*2);
                luma1 = *((uint8*)(i_img_ptr->imgPtr + offset +(i*dx+j-1)*2+1));

                rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                      ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
                gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                      ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
                bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                      ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
                cdb = luma2 + (rc>>16);
                cdr = luma2 + (gc>>16);
                lumad1 = luma2 + (bc>>16);
                r= (uint8)CLIPIT(cdb);
                g= (uint8)CLIPIT(cdr);
                b= (uint8)CLIPIT(lumad1);
              }
              accum_r += r;
              accum_g += g;
              accum_b += b;
            }
          }
          accum_r = (accum_r<<3)/count;
          accum_r += 0x4;
          accum_r = accum_r >> 3;
          accum_g = (accum_g<<3)/count;
          accum_g += 0x4;
          accum_g = accum_g >> 3;
          accum_b = (accum_b<<3)/count;
          accum_b += 0x4;
          accum_b = accum_b >> 3;
          /*
          ** Write out the resize value
          */
          out = pack_rgb565(accum_r,accum_g,accum_b);
          *(uint16*)(o_img_ptr->imgPtr + (((row*odx)+col)*2))= out;
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } 
    else if (o_img_ptr->cFormat == IPL_YCbCr)
    {
      // make sure input size is reasonable
      if (o_img_ptr->dx < 2)
      {
        ipl_sys_free(col_map);
        ipl_sys_free(row_map);

        MSG_LOW("ipl_crop_downsize_mn marker_201.000\n");
        return IPL_FAILURE;
      }
      /*
      ** Resize from YCbCr to YCbCr
      */
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          accum_r=0;
          accum_g=0;
          accum_b=0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              if (!(j%2))
              {
                /* Cb */
                cb = *(uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j)*2);
                luma1 = *((uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j)*2+1));
                cr = *(uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j+1)*2);
                luma2 = *((uint8*)(i_img_ptr->imgPtr + offset +(i*dx+j+1)*2+1));
                rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                      ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
                gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                      ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
                bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                      ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
                cdb = luma1 + (rc>>16);
                cdr = luma1 + (gc>>16);
                lumad1 = luma1 + (bc>>16);
                r= (uint8)CLIPIT(cdb);
                g= (uint8)CLIPIT(cdr);
                b= (uint8)CLIPIT(lumad1);
              } else {
                /* Cr */
                cr = *((uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j)*2));
                luma2 = *((uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j)*2+1));
                cb = *(uint8*)(i_img_ptr->imgPtr + offset + (i*dx+j-1)*2);
                luma1 = *((uint8*)(i_img_ptr->imgPtr + offset +(i*dx+j-1)*2+1));
                rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                      ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
                gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                      ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
                bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                      ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
                cdb = luma2 + (rc>>16);
                cdr = luma2 + (gc>>16);
                lumad1 = luma2 + (bc>>16);
                r= (uint8)CLIPIT(cdb);
                g= (uint8)CLIPIT(cdr);
                b= (uint8)CLIPIT(lumad1);
              }
              accum_r += r;
              accum_g += g;
              accum_b += b;
            }
          }
          accum_r = (accum_r<<3)/count;
          accum_r += 0x4;
          accum_r = accum_r >> 3;
          accum_g = (accum_g<<3)/count;
          accum_g += 0x4;
          accum_g = accum_g >> 3;
          accum_b = (accum_b<<3)/count;
          accum_b += 0x4;
          accum_b = accum_b >> 3;
          /*
          ** Write out the resize value
          */
          if (!(col%2))
          {
            /* Cb Path */
            lumad1 = (ycbcr_convert[0]*accum_r + 
                      ycbcr_convert[1]*accum_g + 
                      ycbcr_convert[2]*accum_b)*4+0x8000;
            lumad1 = (lumad1>>16) + 16;
            lumad1 = CLIPIT(lumad1);
            cdb = (ycbcr_convert[3]*accum_r + 
                   ycbcr_convert[4]*accum_g + 
                   ycbcr_convert[5]*accum_b)*4+0x8000;
            cdb = (cdb>>16) + 128;
            cdb = CLIPIT(cdb);
            *(uint8*)(o_img_ptr->imgPtr + (((row*odx)+col)*2))= (uint8)cdb;
            *(uint8*)(o_img_ptr->imgPtr + (((row*odx)+col)*2+1))= (uint8)lumad1;
          } else {
            /* Cr Path */
            lumad2 = (ycbcr_convert[0]*accum_r + 
                      ycbcr_convert[1]*accum_g + 
                      ycbcr_convert[2]*accum_b)*4+0x8000;
            lumad2 = (lumad2>>16) + 16;
            lumad2 = CLIPIT(lumad2);
            cdr = (ycbcr_convert[6]*accum_r + 
                   ycbcr_convert[7]*accum_g + 
                   ycbcr_convert[8]*accum_b)*4+0x8000;
            cdr = (cdr>>16) + 128;
            cdr = CLIPIT(cdr);
            *(uint8*)(o_img_ptr->imgPtr + (((row*odx)+col)*2))= (uint8)cdr;
            *(uint8*)(o_img_ptr->imgPtr + (((row*odx)+col)*2+1))= (uint8)lumad2;
          }
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } 
    else if (o_img_ptr->cFormat == IPL_LUMA_ONLY)
    {
      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + offset + (i*dx+j)*2+1);
            }
          }
          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          *(o_img_ptr->imgPtr + ((row*odx)+col)) = (unsigned char) lumad1;
        } 
      }
    } 
    else 
    {
      failed = 1;
    }
  } 
  else if (i_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
  {
    offset  = (2*(cropin->x/2) + i_img_ptr->dx*cropin->y);
    offsetC = (2*(cropin->x/2) + i_img_ptr->dx*cropin->y);

    // make sure input size is reasonable
    if (i_img_ptr->dx < 2)
    {
      ipl_sys_free(col_map);
      ipl_sys_free(row_map);

      MSG_LOW("ipl_crop_downsize_mn marker_201.01\n");
      return IPL_FAILURE;
    }

    if (o_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
    {
      // make sure input size is reasonable
      if (o_img_ptr->dx < 2)
      {
        ipl_sys_free(col_map);
        ipl_sys_free(row_map);

        MSG_LOW("ipl_crop_downsize_mn marker_201.02\n");
        return IPL_FAILURE;
      }

      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + offset + (i*dx+j));
              cdb += *(i_img_ptr->clrPtr + offsetC + (i*dx+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + offsetC + (i*dx+2*(j/2))+1);
            }
          }
          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;
          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;
          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          /*
          ** Write out the resize value
          */
          *(o_img_ptr->imgPtr + ((row*odx)+col)) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((row*odx)+2*(col/2))) = (uint8) cdb;
          *(o_img_ptr->clrPtr + ((row*odx)+2*(col/2))+1) = (uint8) cdr;
        } 
      } 
    } 
    else if (o_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
    {
      // make sure input size is reasonable
      if (o_img_ptr->dx < 2 || o_img_ptr->dy < 2)
      {
        ipl_sys_free(col_map);
        ipl_sys_free(row_map);

        MSG_LOW("ipl_crop_downsize_mn marker_201.03\n");
        return IPL_FAILURE;
      }

      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + offset + (i*dx+j));
              cdb += *(i_img_ptr->clrPtr + offsetC + (i*dx+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + offsetC + (i*dx+2*(j/2))+1);
            }
          }
          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;
          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;
          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          /*
          ** Write out the resize value
          */
          *(o_img_ptr->imgPtr + ((row*m)+col)) = (unsigned char) lumad1;

          if (!(row%2))
          { 
            *(o_img_ptr->clrPtr + ((row/2)*odx+2*(col/2))) = (uint8) cdb;
            *(o_img_ptr->clrPtr + ((row/2)*odx+2*(col/2))+1) = (uint8) cdr;
          }
        } 
      } 
    } 
    else if (o_img_ptr->cFormat == IPL_RGB565) 
    {
      ptr16 = (uint16 *) o_img_ptr->imgPtr;

      // initialze color tables
      if (ipl2_init() != IPL_SUCCESS)
      {
        ipl_sys_free(col_map);
        ipl_sys_free(row_map);

        MSG_LOW("ipl_crop_downsize_mn marker_203\n");
        return IPL_FAILURE;
      }
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );

      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + offset + (i*dx+j));
              cdb += *(i_img_ptr->clrPtr + offsetC + (i*dx+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + offsetC + (i*dx+2*(j/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;
          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          // Write out the resize value
          IPL2_CONVERT_YCBCR_RGB_SINGLE(lumad1, cdb, cdr, r, out,
                                         rTable, gTable, bTable);

          *(ptr16 + ((row*odx)+col)) = out;
        } 
      } 
    }
    else if (o_img_ptr->cFormat == IPL_RGB888) 
    {
      outputImgPtr = o_img_ptr->imgPtr;

      // initialze color tables
      if (ipl2_init() != IPL_SUCCESS)
      {
        ipl_sys_free(col_map);
        ipl_sys_free(row_map);

        MSG_LOW("ipl_crop_downsize_mn marker_203\n");
        return IPL_FAILURE;
      }
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );

      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + offset + (i*dx+j));
              cdb += *(i_img_ptr->clrPtr + offsetC + (i*dx+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + offsetC + (i*dx+2*(j/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;
          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          // Write out the resize value
          *outputImgPtr++ = ipl_clamp(lumad1 + ipl_crr[cdb]);
          *outputImgPtr++ = ipl_clamp(lumad1 + ((ipl_crg[cdb] + ipl_cbg[cdr]) >> 16));
          *outputImgPtr++ = ipl_clamp(lumad1 + ipl_cbb[cdr]);
         
        } 

        outputImgPtr += (odx - m);
      }
    }
    else
    {
      failed = 1;
    }
  } 
  else if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    MSG_LOW("ipl_crop_downsize_mn marker_201.03\n");

    offset  = (2*(cropin->x/2) + i_img_ptr->dx*cropin->y);
    offsetC = (2*(cropin->x/2) + i_img_ptr->dx*cropin->y/2);

    // make sure input size is reasonable
    if (i_img_ptr->dx < 2 || i_img_ptr->dy < 2)
    {
      ipl_sys_free(col_map);
      ipl_sys_free(row_map);

      MSG_LOW("ipl_crop_downsize_mn marker_201.04\n");
      return IPL_FAILURE;
    }
    if (o_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
    {

      // make sure input size is reasonable
      if (o_img_ptr->dx < 2 || o_img_ptr->dy < 2)
      {
        ipl_sys_free(col_map);
        ipl_sys_free(row_map);

        MSG_LOW("ipl_crop_downsize_mn marker_201.05\n");
        return IPL_FAILURE;
      }

      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
#if 0
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + (i*n+j));
              cdb += *(i_img_ptr->clrPtr + ((i/2)*dx+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + ((i/2)*dx+2*(j/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;
          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;
          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          /*
          ** Write out the resize value
          */
          *(o_img_ptr->imgPtr + ((row*odx)+col)) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((row/2)*odx+2*(col/2))) = (uint8) cdb;
          *(o_img_ptr->clrPtr + ((row/2)*odx+2*(col/2))+1) = (uint8) cdr;
#else
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);

          if (count == 0) 
            count = 1;

          cdb = cdr = lumad1 = 0;

          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + offset + (i*n+j));
            }
          }

          i = col_map[row]+1;
          j = row_map[col]+1;
          cdb = *(i_img_ptr->clrPtr + offsetC+ ((i/2)*dx+2*(j/2)));
          cdr = *(i_img_ptr->clrPtr + offsetC+ ((i/2)*dx+2*(j/2))+1);

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          /*
          ** Write out the resize value
          */
          *(o_img_ptr->imgPtr + ((row*odx)+col)) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((row/2)*odx+2*(col/2))) = (uint8) cdb;
          *(o_img_ptr->clrPtr + ((row/2)*odx+2*(col/2))+1) = (uint8) cdr;
#endif
        } 
      } 
    } 
    else if (o_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
    {
      // make sure input size is reasonable
      if (o_img_ptr->dx < 2)
      {
        ipl_sys_free(col_map);
        ipl_sys_free(row_map);

        MSG_LOW("ipl_crop_downsize_mn marker_201.06\n");
        return IPL_FAILURE;
      }

      for (row=0;row<mv;row++)
      {
        for (col=0;col<m;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + offset + (i*dx+j));
              cdb += *(i_img_ptr->clrPtr + offsetC + ((i/2)*dx+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + offsetC + ((i/2)*dx+2*(j/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;
          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;
          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          /*
          ** Write out the resize value
          */
          *(o_img_ptr->imgPtr + ((row*odx)+col)) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((row)*odx+2*(col/2))) = (uint8) cdb;
          *(o_img_ptr->clrPtr + ((row)*odx+2*(col/2))+1) = (uint8) cdr;
        } 
      } 
    } 
    else if (o_img_ptr->cFormat == IPL_RGB888) 
    {
      outputImgPtr = o_img_ptr->imgPtr;

      // initialze color tables
      if (ipl2_init() != IPL_SUCCESS)
      {
        ipl_sys_free(col_map);
        ipl_sys_free(row_map);

        MSG_LOW("ipl_crop_downsize_mn marker_203\n");
        return IPL_FAILURE;
      }
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );

      for (row=0;row<o_img_ptr->dy;row++)
      {
        for (col=0;col<o_img_ptr->dx;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + offset + (i*dx+j));
              cdb += *(i_img_ptr->clrPtr + offsetC + ((i/2)*dx+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + offsetC + ((i/2)*dx+2*(j/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;


          // Write out the resize value
          *outputImgPtr++ = ipl_clamp(lumad1 + ipl_crr[cdb]);
          *outputImgPtr++ = ipl_clamp(lumad1 + ((ipl_crg[cdb] + ipl_cbg[cdr]) >> 16));
          *outputImgPtr++ = ipl_clamp(lumad1 + ipl_cbb[cdr]);

        } 

        outputImgPtr += (odx - cropout->dx);
      }
    }
    else if (o_img_ptr->cFormat == IPL_RGB565) 
    {
      ptr16 = (uint16 *) o_img_ptr->imgPtr;

      // initialze color tables
      if (ipl2_init() != IPL_SUCCESS)
      {
        ipl_sys_free(col_map);
        ipl_sys_free(row_map);

        MSG_LOW("ipl_crop_downsize_mn marker_203\n");
        return IPL_FAILURE;
      }
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );

      for (row=0;row<cropout->dy;row++)
      {
        for (col=0;col<cropout->dx;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + offset + (i*dx+j));
              cdb += *(i_img_ptr->clrPtr + offsetC + ((i/2)*dx+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + offsetC + ((i/2)*dx+2*(j/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;

          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;

          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;


          // Write out the resize value
          IPL2_CONVERT_YCBCR_RGB_SINGLE(lumad1, cdb, cdr, r, out, rTable, gTable, bTable);
          *(ptr16 + ((row*odx)+col)) = out;

        } 
      }
    }
    else
    {
      failed = 1;
    }
  } 
  else
  {
    failed = 1;
  }


  ipl_sys_free(row_map);
  ipl_sys_free(col_map);
  if (failed)
  {
    MSG_LOW("ipl_crop_downsize_mn marker_204\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_crop_downsize_mn marker_100\n");

  return IPL_SUCCESS;
}


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_resize_rot

DESCRIPTION
  This function is used to do crop, resize and rotate

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   points to the input image
  output_img_ptr  points to the output image
  cropIn          point to where in input to crop
  cropOut         point to where in output to crop
  quality         refers to quality of output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_crop_resize_rot
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  cropIn,
  ipl_rect_type*  cropOut,
  ipl_rotate90_type rot,
  ipl_quality_type  quality 
)
{
  ipl2_image_upsize_param_type i_param;
  //int i, j;  // generic variables, user for several things
  //int upsize;
  ipl_rect_type cropi;
  ipl_status_type retval = IPL_FAILURE;
  ipl_image_type *tmpBuf;
  ipl_image_type tmpBufData;
  int temp;
  int sys_freeTempBuf = 0;

  MSG_LOW("ipl_crop_resize_rot marker_0\n");

  /*------------------------------------------------------------------------
      Switch on the input image type
  ------------------------------------------------------------------------*/

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_resize_rot marker_200\n");
    return IPL_FAILURE;
  }



  if (cropIn != NULL)
  {
    cropi.x = cropIn->x;
    cropi.y = cropIn->y;
    cropi.dx = cropIn->dx;
    cropi.dy = cropIn->dy;
  }
  else
  {
    cropi.x = 0;
    cropi.y = 0;
    cropi.dx = i_img_ptr->dx;
    cropi.dy = i_img_ptr->dy;
  }


  if (cropOut != NULL)
  {
    i_param.crop.x = cropOut->x;
    i_param.crop.y = cropOut->y;
    i_param.crop.dx = cropOut->dx;
    i_param.crop.dy = cropOut->dy;
    i_param.qual = quality;
  }
  else
  {
    i_param.crop.x = 0;
    i_param.crop.y = 0;
    i_param.crop.dx = o_img_ptr->dx;
    i_param.crop.dy = o_img_ptr->dy;
    i_param.qual = quality;
  }


#if 0
  // only support rotation for fp and ycbcr
  if (quality >= IPL_QUALITY_HIGH && 
      i_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK)
  {
    return(ipl_bilinUpsize_Rot_Crop_YCbCr420fpToRGB(i_img_ptr,
            o_img_ptr,i_param.crop.dx,
            i_param.crop.dy,&i_param.crop,rot,quality));
  }
  else if (quality >= IPL_QUALITY_HIGH && i_img_ptr->cFormat == IPL_YCbCr)
  {
    return(ipl_bilinUpsize_Rot_Crop_YCbCr(i_img_ptr,o_img_ptr,
                i_param.crop.dx,
                i_param.crop.dy,&i_param.crop,rot,quality));
  }
#endif


  MSG_LOW("ipl_crop_resize_rot marker_1\n");

  // see if we are only color converting
  if ((i_param.crop.dx == cropi.dx && i_param.crop.dy == cropi.dy) &&
      (i_img_ptr->dx == o_img_ptr->dx && i_img_ptr->dy == o_img_ptr->dy) &&
      (i_img_ptr->dx == cropi.dx && i_img_ptr->dy == cropi.dy) &&
      (rot == IPL_NOROT))
  {
    MSG_LOW("ipl_crop_resize_rot marker_100.1\n");
    return(ipl_convert_image(i_img_ptr, o_img_ptr));
  }

  // see if we are only croping
  if (rot == IPL_NOROT && i_param.crop.dx == cropi.dx && 
      i_param.crop.dy == cropi.dy)
  {
    if (o_img_ptr->cFormat == i_img_ptr->cFormat)
    {
      MSG_LOW("ipl_crop_resize_rot marker_100.2\n");
      return(ipl_copy_and_paste(i_img_ptr, o_img_ptr, &cropi, &i_param.crop));
    }
    else
    {
      if (o_img_ptr->cFormat == IPL_RGB565)
      {
        MSG_LOW("ipl_crop_resize_rot marker_100.3\n");
        return(ipl_crop_YCbCrToRGB(i_img_ptr, o_img_ptr, &cropi,&i_param.crop));
      }
      else
        return IPL_FAILURE;
    }
  }

  // see if we only rotating
  if (rot == IPL_ROT90)
  {
    if (i_param.crop.dx == cropi.dy && i_param.crop.dy == cropi.dx)
    {
      MSG_LOW("ipl_crop_resize_rot marker_100.4\n");
      return(ipl_rotate(i_img_ptr, o_img_ptr, 90, 0));
    }
  }
  else if (rot == IPL_ROT180)
  {
    if (i_param.crop.dx == cropi.dx && i_param.crop.dy == cropi.dy)
    {
      MSG_LOW("ipl_crop_resize_rot marker_100.5\n");
      return(ipl_rotate(i_img_ptr, o_img_ptr, 180, 0));
    }
  }
  else if (rot == IPL_ROT270)
  {
    if (i_param.crop.dx == cropi.dy && i_param.crop.dy == cropi.dx)
    {
      MSG_LOW("ipl_crop_resize_rot marker_100.6\n");
      return(ipl_rotate(i_img_ptr, o_img_ptr, 270, 0));
    }
  }

  // see if we are downsizing
  if((i_param.crop.dx <= cropi.dx && i_param.crop.dy < cropi.dy) ||
     (i_param.crop.dx <  cropi.dx && i_param.crop.dy <= cropi.dy))
  {
    MSG_LOW("ipl_crop_resize_rot marker_100.7\n");
    if (o_img_ptr->cFormat != i_img_ptr->cFormat)
    {
      switch(i_img_ptr->cFormat)
      {
        case IPL_YCbCr:
          retval = (ipl_crop_downsize_mn(i_img_ptr,o_img_ptr,&cropi,&i_param.crop,NULL));
        break;

        case IPL_RGB565:
          retval = ipl_downsize(i_img_ptr, o_img_ptr, NULL);
        break;

        case IPL_YCbCr420_FRAME_PK:
        case IPL_YCrCb420_FRAME_PK:
          retval = ipl_downsize(i_img_ptr, o_img_ptr, NULL);
        break;
  
        case IPL_YCrCb420_LINE_PK:
        //case IPL_YCbCr420_LINE_PK:
          retval = (ipl_crop_downsize_mn(i_img_ptr,o_img_ptr,&cropi,&i_param.crop,NULL));
        break;

        case IPL_YCrCb422_LINE_PK:
        //case IPL_YCbCr422_LINE_PK:
          retval = (ipl_crop_downsize_mn(i_img_ptr,o_img_ptr,&cropi,&i_param.crop,NULL));
        break;

        case IPL_H1V1MCU_CbCr:
        case IPL_H1V2MCU_CbCr:
        case IPL_H2V1MCU_CbCr:
        case IPL_H2V2MCU_CbCr:

          // convert to common place then downsize
          tmpBufData.dx = i_img_ptr->dx;
          tmpBufData.dy = i_img_ptr->dy;
          tmpBufData.cFormat = IPL_YCrCb422_LINE_PK;

          if (ipl_malloc_img(&tmpBufData) != IPL_SUCCESS)
          {
            MSG_LOW("ipl_crop_resize_rot marker_100.8\n");
            return IPL_NO_MEMORY;
          }
          // convert
          if (ipl_convert_image(i_img_ptr, &tmpBufData) != IPL_SUCCESS)
          {
            MSG_LOW("ipl_crop_resize_rot marker_100.9\n");
            ipl_free_img(&tmpBufData);
            return IPL_NO_MEMORY;
          }

          // call ourselves 
          retval = (ipl_crop_resize_rot(&tmpBufData, o_img_ptr, &cropi, &i_param.crop, IPL_NOROT, quality));

          ipl_free_img(&tmpBufData);
        break;

        default: //lint !e616
          retval = IPL_FAILURE;
        break;
      }
    }
    else
    {
      switch(i_img_ptr->cFormat)
      {
        case IPL_YCbCr:
          if (quality == IPL_QUALITY_LOW || quality == IPL_QUALITY_MEDIUM)
          {
            if (rot != IPL_NOROT)
            {
              retval = (ipl_bilinUpsize_Rot_Crop_YCbCr(i_img_ptr,o_img_ptr,
                                                cropi.dx,
                                                cropi.dy,
                                                &i_param.crop,rot,quality));
            }
            else
            {
              retval = (ipl_crop_downsize_mn(i_img_ptr,o_img_ptr,&cropi,&i_param.crop,NULL));
            }
          }
          else 
          {
            retval = (ipl_bilinUpsize_Rot_Crop_YCbCr(i_img_ptr,o_img_ptr,
                                                cropi.dx,
                                                cropi.dy,
                                                &i_param.crop,rot,quality));
          }
        break;

        case IPL_RGB565:
          if (quality == IPL_QUALITY_HIGH)
            retval = (ipl_xform_Resize_qHigh(i_img_ptr, o_img_ptr, &cropi, &i_param.crop));
          else
          {
            retval = (ipl_crop_downsize_mn(i_img_ptr,o_img_ptr,&cropi,&i_param.crop,NULL));
          }
        break;

        case IPL_RGB888:
          if (quality == IPL_QUALITY_HIGH)
          {
            retval = (ipl_xform_Resize_qHigh(i_img_ptr, o_img_ptr, &cropi, &i_param.crop));
          }
          else
          {
            retval = (ipl_crop_downsize_mn(i_img_ptr,o_img_ptr,&cropi,&i_param.crop,NULL));
          }
        break;
  
  
        case IPL_YCbCr420_FRAME_PK:
          retval = IPL_FAILURE;
        break;
  
        case IPL_YCrCb420_LINE_PK:
        //case IPL_YCbCr420_LINE_PK:
          MSG_LOW("ipl_crop_resize_rot marker_102\n");
            retval = ipl_crop_downsize_rot_ycbcr(i_img_ptr, o_img_ptr, 
                                         &cropi, rot);
        break;

        case IPL_YCrCb422_LINE_PK:
        //case IPL_YCbCr422_LINE_PK:
          MSG_LOW("ipl_crop_resize_rot marker_103\n");
            retval = ipl_crop_downsize_rot_ycbcr(i_img_ptr, o_img_ptr, 
                                         &cropi, rot);
        break;

        case IPL_LUMA_ONLY:
            retval = (ipl_bilinUpsize_Rot_Crop_YCbCr(i_img_ptr,o_img_ptr,
                                                cropi.dx,
                                                cropi.dy,
                                                &i_param.crop,rot,quality));
        break;

        default: //lint !e616
          retval = IPL_FAILURE;
        break;
      }
    }
  }
  else
  {
    MSG_LOW("ipl_crop_resize_rot marker_100.8\n");


    // this if fast 14ms upsize, but can get better quality at around 20ms
    // if we drop through
#if 0
    // if we are doing the ever so popular qcif to qvga upsize at medium 
    // or lower quality from linepack to rgb565, then call this special 
    // routine
    if (rot == IPL_NOROT && 
        i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK &&
        o_img_ptr->cFormat == IPL_RGB565 &&
        cropi.x == 0 && 
        cropi.y == 0 &&
        cropi.dx == 176 &&
        cropi.dy == 144 &&
        i_param.crop.dx == 320 &&
        i_param.crop.dy == 240 && 
        quality <  IPL_QUALITY_HIGH)
    {
      return (ipl_upSize_qcif2qvga_YCrCb420lpToRGB(i_img_ptr, o_img_ptr, &i_param));
    }
#endif


    //IPL2_MSG_FATAL("  ipl_crop_resize_rot time1\n");
    // if we are color converting, then we need to do an optimization whereby
    // we color convert first, then upsize... this way, we do not have to 
    // color convert the input pixel multiple times.
    if (o_img_ptr->cFormat != i_img_ptr->cFormat && 
        o_img_ptr->cFormat == IPL_RGB565) // babak, should we do this?
    {
      ipl_rect_type tcrop;

      if (rot == IPL_ROT90 || rot == IPL_ROT270)
      {
        tmpBufData.dx = i_img_ptr->dy;
        tmpBufData.dy = i_img_ptr->dx;

        
        // swap input crop paramters
        temp = cropi.dx;
        cropi.dx = cropi.dy;
        cropi.dy = temp;

        temp = cropi.x;
        cropi.x = cropi.y;
        cropi.y = temp;
      }
      else
      {
        tmpBufData.dx = i_img_ptr->dx;
        tmpBufData.dy = i_img_ptr->dy;
      }
      tmpBufData.cFormat = o_img_ptr->cFormat;

      if (ipl_malloc_img(&tmpBufData) != IPL_SUCCESS)
      {
        MSG_LOW("ipl_crop_resize_rot marker_201\n");
        return IPL_NO_MEMORY;
      }
      sys_freeTempBuf = 1;

      tcrop.x = 0;
      tcrop.y = 0;
      tcrop.dx = tmpBufData.dx;
      tcrop.dy = tmpBufData.dy;

      // color covnert and rot if need be
      if (rot == IPL_NOROT)
      {
        if (ipl_convert_image(i_img_ptr, &tmpBufData)) 
        {
          ipl_free_img(&tmpBufData);
          MSG_LOW("ipl_crop_resize_rot marker_201.5\n");
          return IPL_NO_MEMORY;
        }
      }
      else
      {
        if (ipl2_rot_add_crop(i_img_ptr, NULL, &tmpBufData, &tcrop, rot, NULL, 0) != IPL_SUCCESS) 
        {
          ipl_free_img(&tmpBufData);
          MSG_LOW("ipl_crop_resize_rot marker_202\n");
          return IPL_NO_MEMORY;
        }
      }

    }
    else
    {
      tmpBufData.dx = i_img_ptr->dx;
      tmpBufData.dy = i_img_ptr->dy;
      tmpBufData.cFormat = i_img_ptr->cFormat;
      tmpBufData.imgPtr = i_img_ptr->imgPtr;
      tmpBufData.clrPtr = i_img_ptr->clrPtr;
    }
    tmpBuf = &tmpBufData;

    //IPL2_MSG_FATAL("  ipl_crop_resize_rot time2\n");


    // turn off for debut to see output of colro conversion and rotation
#if 1
    switch(tmpBuf->cFormat)
    {
      case IPL_YCbCr:
        retval = (ipl_bilinUpsize_Rot_Crop_YCbCr(tmpBuf,o_img_ptr,
                                                  cropi.dx,
                                                  cropi.dy,
                                                  &i_param.crop,rot,quality));
      break;

  
      case IPL_YCbCr420_FRAME_PK:
        // no frame pack to anything other than rgb supported
        retval = IPL_FAILURE;
      break;
  
  
      case IPL_YCrCb420_LINE_PK:
      //case IPL_YCbCr420_LINE_PK:


        if (rot == IPL_ROT90 || rot == IPL_ROT270)
        {
          // rotation and upsize is not supported, so do in two passes.
          // First upsize into temp buffer
          tmpBufData.dx = o_img_ptr->dy;
          tmpBufData.dy = o_img_ptr->dx;
          tmpBufData.cFormat = o_img_ptr->cFormat;
          if (ipl_malloc_img(&tmpBufData) != IPL_SUCCESS)
          {
            MSG_LOW("ipl_crop_resize_rot marker_201.1\n");
            return IPL_NO_MEMORY;
          }
          sys_freeTempBuf = 1;

          // swap output crop paramters so we upsize correctly
          temp = i_param.crop.dx;
          i_param.crop.dx = i_param.crop.dy;
          i_param.crop.dy = temp;

          temp = i_param.crop.x;
          i_param.crop.x = i_param.crop.y;
          i_param.crop.y = temp;

          if (IPL_XFORM_DEBUG) //lint !e774 !e506
          {
            printf("going to first resize from %ldx%ld w/ crop %ldx%ld\n", i_img_ptr->dx, i_img_ptr->dy, cropi.dx, cropi.dy);
            printf("to                         %ldx%ld w/ crop %ldx%ld\n", tmpBufData.dx, tmpBufData.dy, i_param.crop.dx, i_param.crop.dy);
          }

          if (ipl_crop_resize_ycrcblp(i_img_ptr, &tmpBufData, &cropi, &i_param.crop, quality) != IPL_SUCCESS)
          {
            MSG_LOW("ipl_crop_resize_rot marker_201.2\n");
            ipl_free_img(&tmpBufData);
            return IPL_NO_MEMORY;
          }

          if (IPL_XFORM_DEBUG) //lint !e774 !e506
          {
            printf("now going to rotate from %ldx%ld to %ldx%ld\n", tmpBufData.dx, tmpBufData.dy, o_img_ptr->dx, o_img_ptr->dy);
          }

          // now rotate into final buffer
          retval = ipl_rotate90(&tmpBufData, o_img_ptr, rot);
        }
        else
        {
          retval = ipl_crop_resize_ycrcblp(tmpBuf, o_img_ptr, &cropi, &i_param.crop, quality);
        }
      break;


      case IPL_RGB565:

        //IPL2_MSG_FATAL("  ipl_crop_resize_rot time3\n");
        if (quality == IPL_QUALITY_LOW)
        {
          retval =(ipl_upSize_arb_low_RGBToRGB(tmpBuf, o_img_ptr,&cropi,
                                           &i_param.crop));
          // for debug purposes
          //retval = ipl_copy_and_paste(tmpBuf, o_img_ptr, NULL, NULL);
        }
        else if (quality == IPL_QUALITY_MEDIUM)
        {
          retval =(ipl_upSize_arb_med_RGBToRGB(tmpBuf, o_img_ptr,&cropi,
                                           &i_param.crop));
          // for debug purposes
          //retval = ipl_copy_and_paste(tmpBuf, o_img_ptr, NULL, NULL);
        }
        else 
        {
          retval = (ipl_xform_Resize_qHigh(tmpBuf, o_img_ptr, &cropi, 
                                        &i_param.crop));
          // for debug purposes
          //retval = ipl_copy_and_paste(tmpBuf, o_img_ptr, NULL, NULL);
        }
        //IPL2_MSG_FATAL("  ipl_crop_resize_rot time4\n");
      break;

      case IPL_LUMA_ONLY:
        MSG_LOW("ipl_crop_resize_rot marker_104\n");
        retval = ipl_xform_Upsize_qLow(i_img_ptr, o_img_ptr, &cropi);
      break;

      default: //lint !e616
        IPL2_MSG_FATAL( "ipl2_upsize_QCIF failed  : Unsupported i/p color /\
                 format  " );
        retval = IPL_FAILURE;
      break;
    }

#else
    retval = ipl_copy_and_paste(&tmpBufData, o_img_ptr, NULL, NULL);
#endif

    // sys_free the temporary buffer we used to create the color converted image
    if (sys_freeTempBuf)
    {
      ipl_free_img(&tmpBufData);
    }
  }

  if (retval == IPL_SUCCESS)
     MSG_LOW("ipl_crop_resize_rot marker_105\n");
  else
     MSG_LOW("ipl_crop_resize_rot marker_203\n");
    
  return retval;
}




/* <EJECT> */
/*===========================================================================
 *
 ===========================================================================*/
static ipl_status_type ipl_bayer_to_rgb_downsize(
    uint8 *p_in, 
    int w_in, 
    int h_in, 
    ipl_col_for_type bayer_pattern,
	  uint8 *p_out, 
    int w_out, 
    int h_out)
{
	int v_half_step, h_half_step;
	int i,j,k, ii,jj, row_cnt, col_cnt;
	int row_ptr, ptr;
	int rgb_w, rgb_h, sum;
	uint8 *rgb_buf, *p;
	int row_bytes;
	int fv_half_step, fh_half_step;

  MSG_LOW("ipl_bayer_to_rgb_downsize marker_0\n");

  if (!p_in || !p_out)
  {
    MSG_LOW("ipl_bayer_to_rgb_downsize marker_200\n");
    return IPL_FAILURE;
  }

	// check the bayer pattern to make sure the data is BAYER
	if (bayer_pattern!=IPL_BAYER_RGGB && 
      bayer_pattern!=IPL_BAYER_GBRG && 
      bayer_pattern!=IPL_BAYER_GRBG && 
      bayer_pattern!=IPL_BAYER_BGGR)
	{	
    MSG_LOW("ipl_bayer_to_rgb_downsize marker_201\n");
		return IPL_FAILURE;
	}
	
	// validate the input & output dimension
	rgb_w=2*w_out+1;
	rgb_h=2*h_out+1;
	h_half_step=(w_in-4)/rgb_w;
	fh_half_step=((w_in-4)<<4)/rgb_w;
	v_half_step=(h_in-4)/rgb_h;
	fv_half_step=((h_in-4)<<4)/rgb_h;

	if (h_half_step<2 || v_half_step<2)
	{	
    MSG_LOW("ipl_bayer_to_rgb_downsize marker_202\n");
		return IPL_FAILURE;
	}
	
	// allocate intermediate working buffer
	rgb_buf=(uint8 *) ipl_malloc(rgb_w*rgb_h*3);
	if (rgb_buf==0)
  {
    MSG_LOW("ipl_bayer_to_rgb_downsize marker_203\n");
    return IPL_NO_MEMORY;
  }	
	// define the sampling boundary
  //	w1=4;	// allow some rooms for interpolation near edge
  //	w2=w1+(rgb_w-1)*h_half_step;
	
  //	h1=4;
  //	h2=h1+(rgb_h-1)*v_half_step;
	p=rgb_buf;

  MSG_LOW("ipl_bayer_to_rgb_downsize marker_1\n");

	// perform demosaic on the half sample positions
	switch (bayer_pattern)
	{
	  case IPL_BAYER_GBRG:
		  //for (i=h1;i<=h2; i+=v_half_step)
		  row_cnt=32;
		  for (ii=0;ii<rgb_h;ii++)
		  {
			  i=row_cnt>>4;
			  // we want to use the odd row
			  if ((i & 0x01)==0)
				  row_ptr=(i-1)*w_in;
			  else
				  row_ptr=i*w_in;
			  // now row_ptr points to the first pixel in an odd row (GR row)
			
			  //for (j=w1; j<=w2; j+=h_half_step)
			  col_cnt=32;
			  for (jj=0;jj<rgb_w;jj++)
			  {
				  j=col_cnt>>4;
				  // we want to use the odd column (G pixel on the GR row)
				  if ((j & 0x01)==0)
					  ptr=row_ptr + j-1;
				  else
				  	ptr=row_ptr + j;
				
				  // G pixel is used as point of demosaic
				  // use simple bilinear interpolation for demosaic
				
				  // interpolate for Red color
				  *p=(p_in[ptr-1] + p_in[ptr+1]) >> 1;
				  p++;
				
				  // interpolcate for Green color
				  *p=(p_in[ptr] >> 1) + ((p_in[ptr-w_in-1]+p_in[ptr-w_in+1]+
                                p_in[ptr+w_in-1]+p_in[ptr+w_in+1])>>3);
				  p++;
				
				  // interpolate for Blue color
				  *p=(p_in[ptr-w_in]+p_in[ptr+w_in])>>1;
				  p++;
				
				  col_cnt+=fh_half_step;
			  }	// end of loop j
			  row_cnt+=fv_half_step;
		  }	// end of loop i
		break;
		
	  case IPL_BAYER_BGGR:
		  //for (i=h1;i<=h2; i+=v_half_step)
		  row_cnt=32;
		  for (ii=0;ii<rgb_h;ii++)
		  {
			  i=row_cnt>>4;
			  // we want to use the odd row
			  if ((i & 0x01)==0)
				  row_ptr=(i-1)*w_in;
			  else
				  row_ptr=i*w_in;
			  // now row_ptr points to the first pixel in an odd row (GR row)
			
			  //for (j=w1; j<=w2; j+=h_half_step)
			  col_cnt=32;
			  for (jj=0;jj<rgb_w;jj++)
			  {
				  j=col_cnt>>4;
				  // we want to use the even column (G pixel on the GR row)
				  if ((j & 0x01)!=0)
					  ptr=row_ptr + j-1;
				  else
				  	ptr=row_ptr + j;
				
				  // G pixel is used as point of demosaic
				  // use simple bilinear interpolation for demosaic
				
				  // interpolate for Red color
				  *p=(p_in[ptr-1] + p_in[ptr+1]) >> 1;
				  p++;
				
				  // interpolcate for Green color
				  *p=(p_in[ptr] >> 1) + ((p_in[ptr-w_in-1]+p_in[ptr-w_in+1]+
                                p_in[ptr+w_in-1]+p_in[ptr+w_in+1])>>3);
				  p++;
				
				  // interpolate for Blue color
				  *p=(p_in[ptr-w_in]+p_in[ptr+w_in])>>1;
				  p++;
				
				  col_cnt+=fh_half_step;
			  }	// end of loop j
			  row_cnt+=fv_half_step;
		  }	// end of loop i
	  break;
		
		
	  case IPL_BAYER_GRBG:
		  //for (i=h1;i<=h2; i+=v_half_step)
		  row_cnt=32;
		  for (ii=0;ii<rgb_h;ii++)
		  {
			  i=row_cnt>>4;
			  // we want to use the even row
			  if (i & 0x01)
				  row_ptr=(i-1)*w_in;
			  else
				  row_ptr=i*w_in;

			  // now row_ptr points to the first pixel in an even row (RG row)
			  //for (j=w1; j<=w2; j+=h_half_step)
			  col_cnt=32;
			  for (jj=0;jj<rgb_w;jj++)
			  {
				  j=col_cnt>>4;
				  // we want to use the even column (G pixel on the GR row)
				  if ((j & 0x01)!=0)
					  ptr=row_ptr + j-1;
				  else
					  ptr=row_ptr + j;
				
				  // G pixel is used as point of demosaic
				  // use simple bilinear interpolation for demosaic
				
				  // interpolate for Red color
				  *p=(p_in[ptr-1] + p_in[ptr+1]) >> 1;
				  p++;
				
				  // interpolcate for Green color
				  *p=(p_in[ptr] >> 1) + ((p_in[ptr-w_in-1]+p_in[ptr-w_in+1]+
                                p_in[ptr+w_in-1]+p_in[ptr+w_in+1])>>3);
				  p++;
				
				  // interpolate for Blue color
				  *p=(p_in[ptr-w_in]+p_in[ptr+w_in])>>1;
				  p++;
				
				  col_cnt+=fh_half_step;
			  }	// end of loop j
			  row_cnt+=fv_half_step;
		  }	// end of loop i
		break;
		
		
	  case IPL_BAYER_RGGB:
		  //for (i=h1;i<=h2; i+=v_half_step)
		  row_cnt=32;
		  for (ii=0;ii<rgb_h;ii++)
		  {
			  i=row_cnt>>4;
			  // we want to use the even row
			  if (i & 0x01)
				  row_ptr=(i-1)*w_in;
			  else
				  row_ptr=i*w_in;
			    // now row_ptr points to the first pixel in an even row (RG row)
			
			  //for (j=w1; j<=w2; j+=h_half_step)
			  col_cnt=32;
			  for (jj=0;jj<rgb_w;jj++)
			  {
				  j=col_cnt>>4;
				  // we want to use the odd column (G pixel)
				  if ((j & 0x01)==0)
					  ptr=row_ptr + j-1;
				  else
					  ptr=row_ptr + j;
				
				  // G pixel is used as point of demosaic
				  // use simple bilinear interpolation for demosaic
				
				  // interpolate for Red color
				  *p=(p_in[ptr-1] + p_in[ptr+1]) >> 1;
				  p++;
				
				  // interpolcate for Green color
				  *p=(p_in[ptr] >> 1) + ((p_in[ptr-w_in-1]+p_in[ptr-w_in+1]+
                                p_in[ptr+w_in-1]+p_in[ptr+w_in+1])>>3);
				  p++;
				
				  // interpolate for Blue color
				  *p=(p_in[ptr-w_in]+p_in[ptr+w_in])>>1;
				  p++;
				
			    col_cnt+=fh_half_step;
			  }	// end of loop j
			  row_cnt+=fv_half_step;
		  }	// end of loop i
	  break;

    default:
	    ipl_sys_free(rgb_buf);
      MSG_LOW("ipl_bayer_to_rgb_downsize marker_204\n");
      return IPL_FAILURE;
  }	// end of switch case
	
	
	// now perform 9-point averaging on the RGB data
	// and scale to the desired output dimension
	p=p_out;
	row_bytes=3*rgb_w;
	for (i=1;i<rgb_h;i+=2)
	{	// for each row
		ptr=i*row_bytes+3;

		for (j=1; j<rgb_w; j+=2)
		{	// for each pixel

			for (k=0;k<3;k++)
			{	// for each color
				sum=rgb_buf[ptr-row_bytes-3]+
          rgb_buf[ptr-row_bytes]+
          rgb_buf[ptr-row_bytes+3];
				sum+=rgb_buf[ptr-3]+rgb_buf[ptr]+rgb_buf[ptr+3];
				sum+=rgb_buf[ptr+row_bytes-3]+
          rgb_buf[ptr+row_bytes]+
          rgb_buf[ptr+row_bytes+3];
				*p = (sum *455) >> 12;	// divided by 9
				p++;
				ptr++;
			}
			ptr+=3;	// skip the next pixel in the row
		}
	}

	
	// clean up memory space
	ipl_sys_free(rgb_buf);
	
  MSG_LOW("ipl_bayer_to_rgb_downsize marker_100\n");

	return IPL_SUCCESS;
}


// Downscale on Bayer domain
// The calling function should allocate the memory space for the output buffer
// scale factor must be at least 4 for each direction (X and Y)
static ipl_status_type ipl_bayer_to_ycbcr_downsize(
    uint8 *p_in, 
    int w_in, 
    int h_in, 
    ipl_col_for_type bayer_pattern,
	  uint8 *p_out, 
    int w_out, 
    int h_out,
    ipl_bip_type* bip_ptr)
{
	int v_half_step, h_half_step;
	int i,j,k, ii,jj, row_cnt, col_cnt;
	/* int w1,w2,h1,h2; */
	int row_ptr, ptr;
	int rgb_w, rgb_h, sum;
	uint8 *rgb_buf, *p;
	int row_bytes;
	int fv_half_step, fh_half_step;

  int32 rc,gc,bc;
  register int32 r,g,b;

  uint8 c[2][3];
  int32 luma1,luma2,cb,cr;
  int everySecondPixel = 0;
  int16 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_bayer_to_ycbcr_downsize marker_0\n");

  if (!p_in || !p_out)
  {
    MSG_LOW("ipl_bayer_to_ycbcr_downsize marker_200\n");
    return IPL_FAILURE;
  }
	// check the bayer pattern to make sure the data is BAYER
	if (bayer_pattern!=IPL_BAYER_RGGB && 
      bayer_pattern!=IPL_BAYER_GBRG && 
      bayer_pattern!=IPL_BAYER_GRBG && 
      bayer_pattern!=IPL_BAYER_BGGR)
	{	// wrong data type passed into this function
    MSG_LOW("ipl_bayer_to_ycbcr_downsize marker_201\n");
		return IPL_FAILURE;
	}
	
	// validate the input & output dimension
	rgb_w=2*w_out+1;
	rgb_h=2*h_out+1;
	h_half_step=(w_in-4)/rgb_w;
	fh_half_step=((w_in-4)<<4)/rgb_w;
	v_half_step=(h_in-4)/rgb_h;
	fv_half_step=((h_in-4)<<4)/rgb_h;

	if (h_half_step<2 || v_half_step<2)
	{	// scale factor restriction is not satisfied
    MSG_LOW("ipl_bayer_to_ycbcr_downsize marker_202\n");
		return IPL_FAILURE;
	}
	
	// allocate intermediate working buffer
	rgb_buf=(uint8 *) ipl_malloc(rgb_w*rgb_h*3);
	if (rgb_buf==0)
  {
    MSG_LOW("ipl_bayer_to_ycbcr_downsize marker_203\n");
    return IPL_NO_MEMORY;
  }	
	// define the sampling boundary
  //	w1=4;	// allow some rooms for interpolation near edge
  //	w2=w1+(rgb_w-1)*h_half_step;
	
  //	h1=4;
  //	h2=h1+(rgb_h-1)*v_half_step;
	p=rgb_buf;

  MSG_LOW("ipl_bayer_to_ycbcr_downsize marker_1\n");

	// perform demosaic on the half sample positions
	switch (bayer_pattern)
	{
	  case IPL_BAYER_GBRG:
		  //for (i=h1;i<=h2; i+=v_half_step)
		  row_cnt=32;
		  for (ii=0;ii<rgb_h;ii++)
		  {
			  i=row_cnt>>4;
			  // we want to use the odd row
			  if ((i & 0x01)==0)
				  row_ptr=(i-1)*w_in;
			  else
				  row_ptr=i*w_in;

			  // now row_ptr points to the first pixel in an odd row (GR row)
			
			  //for (j=w1; j<=w2; j+=h_half_step)
			  col_cnt=32;
			  for (jj=0;jj<rgb_w;jj++)
			  {
				  j=col_cnt>>4;
				  // we want to use the odd column (G pixel on the GR row)
				  if ((j & 0x01)==0)
					  ptr=row_ptr + j-1;
				  else
					  ptr=row_ptr + j;
				
				  // G pixel is used as point of demosaic
				  // use simple bilinear interpolation for demosaic
				
				  // interpolate for Red color
				  *p=(p_in[ptr-1] + p_in[ptr+1]) >> 1;
				  p++;
				
				  // interpolcate for Green color
				  *p=(p_in[ptr] >> 1) + ((p_in[ptr-w_in-1]+p_in[ptr-w_in+1]+
                                p_in[ptr+w_in-1]+p_in[ptr+w_in+1])>>3);
				  p++;
				
				  // interpolate for Blue color
				  *p=(p_in[ptr-w_in]+p_in[ptr+w_in])>>1;
				  p++;
				
				  col_cnt+=fh_half_step;
			  }	// end of loop j
			  row_cnt+=fv_half_step;
		  }	  // end of loop i
		  break;
		
		
	  case IPL_BAYER_BGGR:
		  //for (i=h1;i<=h2; i+=v_half_step)
		  row_cnt=32;
		  for (ii=0;ii<rgb_h;ii++)
		  {
			  i=row_cnt>>4;
			  // we want to use the odd row
			  if ((i & 0x01)==0)
				  row_ptr=(i-1)*w_in;
			  else
				  row_ptr=i*w_in;
			  // now row_ptr points to the first pixel in an odd row (GR row)
			
			  //for (j=w1; j<=w2; j+=h_half_step)
			  col_cnt=32;
			  for (jj=0;jj<rgb_w;jj++)
			  {
				  j=col_cnt>>4;
				  // we want to use the even column (G pixel on the GR row)
				  if ((j & 0x01)!=0)
					  ptr=row_ptr + j-1;
				  else
					  ptr=row_ptr + j;
				
				  // G pixel is used as point of demosaic
				  // use simple bilinear interpolation for demosaic
				
				  // interpolate for Red color
				  *p=(p_in[ptr-1] + p_in[ptr+1]) >> 1;
				  p++;
				
				  // interpolcate for Green color
				  *p=(p_in[ptr] >> 1) + ((p_in[ptr-w_in-1]+p_in[ptr-w_in+1]+
                                p_in[ptr+w_in-1]+p_in[ptr+w_in+1])>>3);
				  p++;
				
				  // interpolate for Blue color
				  *p=(p_in[ptr-w_in]+p_in[ptr+w_in])>>1;
				  p++;
				
				  col_cnt+=fh_half_step;
			  }	// end of loop j
			  row_cnt+=fv_half_step;
		  }	// end of loop i
		  break;
		
		
	  case IPL_BAYER_GRBG:
		  //for (i=h1;i<=h2; i+=v_half_step)
		  row_cnt=32;
		  for (ii=0;ii<rgb_h;ii++)
		  {
			  i=row_cnt>>4;
			  // we want to use the even row
			  if (i & 0x01)
				  row_ptr=(i-1)*w_in;
			  else
				  row_ptr=i*w_in;
			  // now row_ptr points to the first pixel in an even row (RG row)
			
			  //for (j=w1; j<=w2; j+=h_half_step)
			  col_cnt=32;
			  for (jj=0;jj<rgb_w;jj++)
			  {
				  j=col_cnt>>4;
				  // we want to use the even column (G pixel on the GR row)
				  if ((j & 0x01)!=0)
					  ptr=row_ptr + j-1;
				  else
					  ptr=row_ptr + j;
				
				  // G pixel is used as point of demosaic
				  // use simple bilinear interpolation for demosaic
				
				  // interpolate for Red color
				  *p=(p_in[ptr-1] + p_in[ptr+1]) >> 1;
				  p++;
				
				  // interpolcate for Green color
				  *p=(p_in[ptr] >> 1) + ((p_in[ptr-w_in-1]+p_in[ptr-w_in+1]+
                                p_in[ptr+w_in-1]+p_in[ptr+w_in+1])>>3);
				  p++;
				
				  // interpolate for Blue color
				  *p=(p_in[ptr-w_in]+p_in[ptr+w_in])>>1;
				  p++;
				
				  col_cnt+=fh_half_step;
			  }	// end of loop j
			  row_cnt+=fv_half_step;
		  }	// end of loop i
		break;
		
		
	  case IPL_BAYER_RGGB:
		  //for (i=h1;i<=h2; i+=v_half_step)
		  row_cnt=32;
		  for (ii=0;ii<rgb_h;ii++)
		  {
			  i=row_cnt>>4;
			  // we want to use the even row
			  if (i & 0x01)
				  row_ptr=(i-1)*w_in;
			  else
				  row_ptr=i*w_in;
			  // now row_ptr points to the first pixel in an even row (RG row)
			
			  //for (j=w1; j<=w2; j+=h_half_step)
			  col_cnt=32;
			  for (jj=0;jj<rgb_w;jj++)
			  {
				  j=col_cnt>>4;
				  // we want to use the odd column (G pixel)
				  if ((j & 0x01)==0)
					  ptr=row_ptr + j-1;
				  else
					  ptr=row_ptr + j;
				
				  // G pixel is used as point of demosaic
				  // use simple bilinear interpolation for demosaic
				
				  // interpolate for Red color
				  *p=(p_in[ptr-1] + p_in[ptr+1]) >> 1;
				  p++;
				
				  // interpolcate for Green color
				  *p=(p_in[ptr] >> 1) + ((p_in[ptr-w_in-1]+p_in[ptr-w_in+1]+
                                 p_in[ptr+w_in-1]+p_in[ptr+w_in+1])>>3);
				  p++;
				
				  // interpolate for Blue color
				  *p=(p_in[ptr-w_in]+p_in[ptr+w_in])>>1;
				  p++;
				
			    col_cnt+=fh_half_step;
			  }	// end of loop j
			  row_cnt+=fv_half_step;
		  }	// end of loop i
		  break;

    default: //lint !e527
	    ipl_sys_free(rgb_buf);
      MSG_LOW("ipl_bayer_to_ycbcr_downsize marker_204\n");
      return IPL_FAILURE;
	}	// end of switch case
	
	MSG_LOW("ipl_bayer_to_ycbcr_downsize marker_2\n");

  // now perform 9-point averaging on the RGB888 data
  // and scale to the desired output dimension
  p=p_out;
  row_bytes=3*rgb_w;
  everySecondPixel = 0;
  for (i=1;i<rgb_h;i+=2)
  { 
    // for each row
    ptr=i*row_bytes+3;
    for (j=1; j<rgb_w; j+=2)
    {    
      // for each pixel
      for (k=0;k<3;k++)
      {    
        // for each color
        sum=rgb_buf[ptr - row_bytes-3]+
              rgb_buf[ptr - row_bytes]+
              rgb_buf[ptr - row_bytes+3];

        sum+=rgb_buf[ptr - 3]+
               rgb_buf[ptr]+
               rgb_buf[ptr + 3];

        sum+=rgb_buf[ptr+row_bytes - 3]+
               rgb_buf[ptr+row_bytes]+
               rgb_buf[ptr+row_bytes + 3];

        c[everySecondPixel][k] = (sum *455) >> 12;    // divided by 9

        // do next color
        ptr++;
      }

      //now that you your R, G, B averages, do bip processing
      if (bip_ptr != NULL)
      {
        /*
        **  Color Correction 
        */
        /* is putting in register faster? Use armulator/logic analyzer */
        r = c[everySecondPixel][0]; 
        g = c[everySecondPixel][1]; 
        b = c[everySecondPixel][2]; 

        /*
        ** Now do the gains
        */
        r = r*bip_ptr->rGain*8 + 0x8000;
        g = g*bip_ptr->gGain*8 + 0x8000 ;
        b = b*bip_ptr->bGain*8 + 0x8000;
        r = r>>16;
        g = g>>16;
        b = b>>16;
        r=CLIPIT(r);
        g=CLIPIT(g);
        b=CLIPIT(b);


        /*
        **  Color Correction 
        */
        rc = (bip_ptr->colorCorrection[0]*r + 
                bip_ptr->colorCorrection[1]*g + 
                bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
        gc = (bip_ptr->colorCorrection[3]*r +
                bip_ptr->colorCorrection[4]*g +
                bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
        bc = (bip_ptr->colorCorrection[6]*r +
                bip_ptr->colorCorrection[7]*g +
                bip_ptr->colorCorrection[8]*b)*8 + 0x8000;

        rc = (rc>>16) + bip_ptr->colorCorrection[9];
        gc = (gc>>16) + bip_ptr->colorCorrection[10];
        bc = (bc>>16) + bip_ptr->colorCorrection[11];
        rc=CLIPIT(rc);
        gc=CLIPIT(gc);
        bc=CLIPIT(bc);

        /*
        ** Write out the resize value
        */
        /*
        **  Table Lookup
        */
        c[everySecondPixel][0] = (uint8) bip_ptr->gammaTable[rc];
        c[everySecondPixel][1] = (uint8) bip_ptr->gammaTable[gc];
        c[everySecondPixel][2] = (uint8) bip_ptr->gammaTable[bc];
      }

      // once we have 2 pixel pairs, conver to YCbCr
      if (everySecondPixel)
      {
        luma1 = (ycbcr_convert[0] * c[0][0] + 
                   ycbcr_convert[1] * c[0][1] + 
                   ycbcr_convert[2] * c[0][2])*4+0x8000;
        luma1 = (luma1>>16) + 16;
        if (luma1<0) luma1=0;
        luma1=CLIPIT(luma1);

        cb = (ycbcr_convert[3] * c[0][0] + 
                ycbcr_convert[4] * c[0][1] + 
                ycbcr_convert[5] * c[0][2])*4+0x8000;
        cb = (cb>>16) + 128;
        if (cb<0) cb=0;
        cb=CLIPIT(cb);

        /* 2nd pixel */
        luma2 = (ycbcr_convert[0] * c[1][0] + 
                   ycbcr_convert[1] * c[1][1] + 
                   ycbcr_convert[2] * c[1][2])*4+0x8000;
        luma2 = (luma2>>16) + 16;
        if (luma2<0) luma2=0;
        luma2=CLIPIT(luma2);

        cr = (ycbcr_convert[6] * c[1][0] + 
                ycbcr_convert[7] * c[1][1] + 
                ycbcr_convert[8] * c[1][2])*4+0x8000;
        cr = (cr>>16) + 128;
        if (cr<0) cr=0;
        cr=CLIPIT(cr);

        *p = (uint8) cb;
        p++;
        *p = (uint8) luma1;
        p++;
        *p = (uint8) cr;
        p++;
        *p = (uint8) luma2;
        p++;
      }

      ptr+=3;    // skip the next pixel in the row

      everySecondPixel++;
      everySecondPixel = everySecondPixel & 0x1;
    }
  }

  // clean up memory space
  ipl_sys_free(rgb_buf);

  MSG_LOW("ipl_bayer_to_ycbcr_downsize marker_100\n");
  return IPL_SUCCESS;
}






/* <EJECT> */
/*===========================================================================

FUNCTION ipl_bip

DESCRIPTION
  This function takes rgb888 input and does Bayer Image Processing on it
  The Output is RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  bip_ptr points to the bip_ptr

ARGUMENTS OUT
  output_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_bip(
   ipl_image_type* input_img_ptr, 
   ipl_image_type* output_img_ptr, 
   ipl_bip_type* bip_ptr)
{
  int32 rc,gc,bc,row,col;
  register int32 r,g,b;
  register uint16 out;
  uint8* in_ptr;
  uint16* out_ptr;

  MSG_LOW("ipl_bip marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr  || 
      !output_img_ptr || !output_img_ptr->imgPtr ||
      !bip_ptr)
  {
    MSG_LOW("ipl_bip marker_200\n");
    return IPL_FAILURE;
  }
  in_ptr = input_img_ptr->imgPtr;
  out_ptr = (uint16*)output_img_ptr->imgPtr;
 
  MSG_LOW("ipl_bip marker_1\n");

  for (row=output_img_ptr->dy;row!=0;row--)
  {
    for (col=output_img_ptr->dx;col!=0;col--)
    {
      r = *in_ptr++;
      g = *in_ptr++;
      b = *in_ptr++;

      /*
      ** Now do the gains
      */
      r = r*bip_ptr->rGain*8 + 0x8000;
      g = g*bip_ptr->gGain*8 + 0x8000 ;
      b = b*bip_ptr->bGain*8 + 0x8000;
      r = r>>16;
      g = g>>16;
      b = b>>16;
      r=CLIPIT(r);
      g=CLIPIT(g);
      b=CLIPIT(b);


      /*
      **  Color Correction 
      */
      rc = (bip_ptr->colorCorrection[0]*r + 
            bip_ptr->colorCorrection[1]*g + 
            bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
      gc = (bip_ptr->colorCorrection[3]*r +
            bip_ptr->colorCorrection[4]*g +
            bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
      bc = (bip_ptr->colorCorrection[6]*r +
            bip_ptr->colorCorrection[7]*g +
            bip_ptr->colorCorrection[8]*b)*8 + 0x8000;
      rc = (rc>>16) + bip_ptr->colorCorrection[9];
      gc = (gc>>16) + bip_ptr->colorCorrection[10];
      bc = (bc>>16) + bip_ptr->colorCorrection[11];
      rc=CLIPIT(rc);
      gc=CLIPIT(gc);
      bc=CLIPIT(bc);

      /*
      ** Write out the resize value
      */
      /*
      **  Table Lookup
      */
      rc=bip_ptr->gammaTable[rc];
      out = r5xx[rc];

      gc=bip_ptr->gammaTable[gc];
      out += gx6x[gc];

      bc=bip_ptr->gammaTable[bc];
      out += bxx5[bc];

      *out_ptr = out;
      out_ptr++;
    }
  }

  MSG_LOW("ipl_bip marker_100\n");

  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_downsize_med

DESCRIPTION
  This function downsizes YCbCr Input.  It uses a 9 point averaging scheme.
  The minimum downsize factor is 4 in each dimension. ie the output should 
  be smaller than 1/16 the input size.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image

ARGUMENTS IN
  output_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_downsize_med
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,         /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  uint32 i,j;
  uint32 in_dim_x, in_dim_y;
  uint32 out_dim_x, out_dim_y;
  uint32 v_step, v_half_step, v_edge;
  uint32 h_step, h_half_step, h_edge;
  uint32 v_ptr, v_up_ptr, v_down_ptr,  h_ptr;
  uint8  *optr, *iptr;
  uint32 in_ptr, in_up_ptr, in_down_ptr;
  uint32 row_len;
  uint32 sum1, sum2, sum3;
  uint8* out_buf;
  ipl_image_type temp_img;

  MSG_LOW("ipl_downsize_med marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr  || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_downsize_med marker_200\n");
    return IPL_FAILURE;
  }  

  IPLMSG("Inside ipl_downsize_med"); //lint !e774 !e506

  /* 
  ** make sure the Input & output format are the right format
  ** For bayer input, the output should be rgb565
  ** For YCbCr4:2:2 nput, the output should be YCbCr4:2:2
  */
  if (i_img_ptr->cFormat==IPL_YCbCr) 
	{
    if (o_img_ptr->cFormat!=IPL_YCbCr) 
    {
      MSG_LOW("ipl_downsize_med marker_201\n");
      return IPL_FAILURE;          
    }
  } 
  else if (i_img_ptr->cFormat == IPL_BAYER_GBRG)
  {
    if (o_img_ptr->cFormat!=IPL_RGB565 && o_img_ptr->cFormat!=IPL_YCbCr) 
    {
      MSG_LOW("ipl_downsize_med marker_202\n");
      return IPL_FAILURE;          
    }
  } 
  else if (i_img_ptr->cFormat == IPL_BAYER_BGGR)
  {
    if (o_img_ptr->cFormat!=IPL_RGB565 && o_img_ptr->cFormat!=IPL_YCbCr) 
    {
      MSG_LOW("ipl_downsize_med marker_203\n");
      return IPL_FAILURE;          
    }
  } 
  else if (i_img_ptr->cFormat == IPL_BAYER_GRBG)
  {
    if (o_img_ptr->cFormat!=IPL_RGB565 && o_img_ptr->cFormat!=IPL_YCbCr) 
    {
      MSG_LOW("ipl_downsize_med marker_204\n");
      return IPL_FAILURE;          
    }
  } 
  else if (i_img_ptr->cFormat == IPL_BAYER_RGGB)
  {
    if (o_img_ptr->cFormat!=IPL_RGB565 && o_img_ptr->cFormat!=IPL_YCbCr) 
    {
      MSG_LOW("ipl_downsize_med marker_205\n");
      return IPL_FAILURE;          
    }
  } else 
  {
    MSG_LOW("ipl_downsize_med marker_206\n");
    return IPL_FAILURE;          
  }

  MSG_LOW("ipl_downsize_med marker_1\n");

  if (o_img_ptr->cFormat == IPL_RGB565)
  {
    /*
    ** Input is Bayer
    */
    out_buf = ipl_malloc(o_img_ptr->dx*o_img_ptr->dy*3*sizeof(uint8));
    if (!out_buf)
    {
      MSG_LOW("ipl_downsize_med marker_207\n");
      return IPL_NO_MEMORY;
    }
    /*
    **  Get Output in RGB 888
    */
    if ( ipl_bayer_to_rgb_downsize(i_img_ptr->imgPtr,i_img_ptr->dx,
                                   i_img_ptr->dy,i_img_ptr->cFormat,
          out_buf,o_img_ptr->dx,o_img_ptr->dy) != IPL_SUCCESS)
    {
      ipl_sys_free(out_buf);
      MSG_LOW("ipl_downsize_med marker_208\n");
      return IPL_FAILURE;
    }
    temp_img.imgPtr = out_buf;

    /*
    ** Do BIP on output and convert to RGB 565
    */
    if (bip_ptr != NULL)
    {
      if (ipl_bip(&temp_img,o_img_ptr,bip_ptr) != IPL_SUCCESS )
      {
        ipl_sys_free(out_buf);
        MSG_LOW("ipl_downsize_med marker_209\n");
        return IPL_FAILURE;
      }
    }
    else
    {
      ipl_RGB8882RGB565(temp_img.imgPtr, (uint16*) o_img_ptr->imgPtr, 
                        o_img_ptr->dx, o_img_ptr->dy);
    }

    ipl_sys_free(out_buf);

    MSG_LOW("ipl_downsize_med marker_100\n");

    return IPL_SUCCESS;
  }
	else if (i_img_ptr->cFormat == IPL_YCbCr)
  { 
    /* examine input & output dimension */
    in_dim_x=i_img_ptr->dx;
    in_dim_y=i_img_ptr->dy;
    out_dim_x=o_img_ptr->dx;
    out_dim_y=o_img_ptr->dy;
    row_len=2*in_dim_x;        /* how many bytes in the input row (YCbCr 422)*/
  
    /* make sure the resize factor is large enough for using this method */
    if (in_dim_x<(4*out_dim_x) || in_dim_y<(4*out_dim_y)) 
    {
      MSG_LOW("ipl_downsize_med marker_210\n");
      /* the I/O dimension is not suitable for using this method */
      return IPL_FAILURE;        
    }
  
  
    /* determine vertical sampling period */
    v_half_step= in_dim_y/(2*out_dim_y);
    /* vertical sampling period */
    v_step=in_dim_y/out_dim_y;        
    /* boundary pixels on top and bottom */
    v_edge= (in_dim_y - v_step*out_dim_y)/2;  
  
    /* determine horizontal sampling period */
    h_half_step= in_dim_x/(2*out_dim_x);
    if (h_half_step & 0x01) 
    {
      /* make sure h_half_step is an even number */
      h_half_step-=1;        
    }

    h_half_step <<= 1;            /* has to double it to take care of y & c */
    h_step=in_dim_x/out_dim_x;    /* horizontal sampling period */
    h_edge=(in_dim_x-h_step*out_dim_x)/2;
    h_step <<=1;                  /* has to double it to take care of y & c */
    if (h_edge & 0x01) h_edge+=1; /* make sure h_edge is an even number */
    h_edge <<=1;                  /* has to double it to take care of y & c */
  
    /* initialze pointers */
    optr=o_img_ptr->imgPtr;       /* output pointer */
    iptr=i_img_ptr->imgPtr;       /* input pointer */
  
    v_ptr=v_edge+v_half_step;     /* point to current row */
    v_up_ptr = v_ptr - v_half_step;    /* point to a few rows above */
    v_down_ptr = v_ptr + v_half_step;  /* points to a few rows below */
  
    /* perform down scaling by 9-point averaging */
    for (i=0; i < out_dim_y; i++)
    {
      h_ptr= h_edge+h_half_step;
      in_up_ptr=v_up_ptr*row_len+h_ptr;
      in_ptr=v_ptr*row_len+h_ptr;    /* move input pointer for a new row */

      in_down_ptr= v_down_ptr*row_len+h_ptr; 
      /* babakf says in_down_ptr must be one line shy so as to not go past
       * image */ 
      // in_down_ptr= v_down_ptr*row_len+h_ptr-row_len; 

      for (j=0;j < out_dim_x; j++)
      {
        if (j & 0x01)
        {    
          /* odd output column position (Cr & Y) */
          /* should output Cr and Y pixels */
          if (h_ptr & 0x02)
          { 
            /* input y index is odd */

            /* compute Cr */
            sum1=iptr[in_up_ptr-h_half_step]+iptr[in_up_ptr]+
              iptr[in_up_ptr+h_half_step];
            sum2=iptr[in_ptr-h_half_step]+iptr[in_ptr]+
              iptr[in_ptr+h_half_step];
            sum3=iptr[in_down_ptr-h_half_step]+
              iptr[in_down_ptr]+iptr[in_down_ptr+h_half_step];
            *optr=(uint8)(((sum1+sum2+sum3)*455) >> 12); /* divided by 9 */
            optr++;  /* increment output pointer */

            in_up_ptr+=1;  /* move the pointer to the Y pixel to the right */
            in_ptr+=1;     /* move the pointer to the Y pixel to the right */
            in_down_ptr+=1;/* move the pointer to the Y pixel to the right */
  
            /* compute Y */
            sum1=iptr[in_up_ptr-h_half_step]+ iptr[in_up_ptr]+
              iptr[in_up_ptr+h_half_step];
            sum2=iptr[in_ptr-h_half_step]+iptr[in_ptr]+
              iptr[in_ptr+h_half_step];
            sum3=iptr[in_down_ptr-h_half_step]+iptr[in_down_ptr]+
              iptr[in_down_ptr+h_half_step];
            *optr=(uint8)(((sum1+sum2+sum3)*455) >> 12); /* divided by 9 */
            optr++;  /* increment output pointer */

            in_up_ptr+=h_step-1;  
            /* move the input pointer to the sample position */
            in_ptr+=h_step-1;        
            /* move the input pointer to the sample position */
            in_down_ptr+=h_step-1;    
            /* move the input pointer to the sample position */
          } 
          else 
          { /* input y index is even */
            in_up_ptr-=2;        /*use the Cr pixel to the left */
            in_ptr-=2;           /*use the Cr pixel to the left */
            in_down_ptr-=2;      /*use the Cr pixel to the left */

            /* compute Cr */
            sum1=iptr[in_up_ptr-h_half_step]+iptr[in_up_ptr]+
            iptr[in_up_ptr+h_half_step];
            sum2=iptr[in_ptr-h_half_step]+iptr[in_ptr]+
            iptr[in_ptr+h_half_step];
            sum3=iptr[in_down_ptr-h_half_step]+iptr[in_down_ptr]+
            iptr[in_down_ptr+h_half_step];
            *optr=(uint8)(((sum1+sum2+sum3)*455) >> 12); /* divided by 9 */
            optr++;         /* increment output pointer */
            in_up_ptr+=1;   /* move the pointer to the Y pixel to the right */
            in_ptr+=1;      /* move the pointer to the Y pixel to the right */
            in_down_ptr+=1; /* move the pointer to the Y pixel to the right */

            /* compute Y */
            sum1=iptr[in_up_ptr-h_half_step]+
            iptr[in_up_ptr]+iptr[in_up_ptr+h_half_step];
            sum2=iptr[in_ptr-h_half_step]+
            iptr[in_ptr]+iptr[in_ptr+h_half_step];
            sum3=iptr[in_down_ptr-h_half_step]+
            iptr[in_down_ptr]+iptr[in_down_ptr+h_half_step];
            *optr=(uint8)(((sum1+sum2+sum3)*455) >> 12); /* divided by 9 */
            optr++;                /* increment output pointer */
            in_up_ptr+=h_step+1;   /* move the pointer to the sample position */
            in_ptr+=h_step+1;      /* move the pointer to the sample position */
            in_down_ptr+=h_step+1; /* move pointer to the sample position */
          }
        } 
        else 
        {    
          /* even output column position (Cb & Y) */
          /* should output Cb and Y pixels */
          if (h_ptr & 0x02)
          {    
            /* input y index is odd */
            in_up_ptr-=2;        /*use the Cb pixel to the left */
            in_ptr-=2;           /*use the Cb pixel to the left */
            in_down_ptr-=2;      /*use the Cb pixel to the left */

            /* compute Cb */
            sum1=iptr[in_up_ptr-h_half_step]+iptr[in_up_ptr]+
            iptr[in_up_ptr+h_half_step];
            sum2=iptr[in_ptr-h_half_step]+iptr[in_ptr]+iptr[in_ptr+h_half_step];
            sum3=iptr[in_down_ptr-h_half_step]+
            iptr[in_down_ptr]+iptr[in_down_ptr+h_half_step];
            *optr=(uint8)(((sum1+sum2+sum3)*455) >> 12); /* divided by 9 */
            optr++;  /* increment output pointer */
            in_up_ptr+=1; /* move the pointer to the Y pixel to the right */
            in_ptr+=1;    /* move the pointer to the Y pixel to the right */
            in_down_ptr+=1; /* move the pointer to the Y pixel to the right */

            /* compute Y */
            sum1=iptr[in_up_ptr-h_half_step]+
            iptr[in_up_ptr]+iptr[in_up_ptr+h_half_step];
            sum2=iptr[in_ptr-h_half_step]+
            iptr[in_ptr]+iptr[in_ptr+h_half_step];
            sum3=iptr[in_down_ptr-h_half_step]+
            iptr[in_down_ptr]+iptr[in_down_ptr+h_half_step];
            *optr=(uint8)(((sum1+sum2+sum3)*455) >> 12); /* divided by 9 */
            optr++;  /* increment output pointer */
            in_up_ptr+=h_step+1;/* move the pointer to the sample position */
            in_ptr+=h_step+1;   /* move the pointer to the sample position */
            in_down_ptr+=h_step+1; /* move pointer to the sample position */
          } 
          else 
          {    
            /* input y index is even */

            /* compute Cb */
            sum1=iptr[in_up_ptr-h_half_step]+iptr[in_up_ptr]+
            iptr[in_up_ptr+h_half_step];
            sum2=iptr[in_ptr-h_half_step]+iptr[in_ptr]+iptr[in_ptr+h_half_step];
            sum3=iptr[in_down_ptr-h_half_step]+
            iptr[in_down_ptr]+iptr[in_down_ptr+h_half_step];
            *optr=(uint8)(((sum1+sum2+sum3)*455) >> 12); /* divided by 9 */
            optr++;         /* increment output pointer */
            in_up_ptr+=1;   /* move the pointer to the Y pixel to the right */
            in_ptr+=1;      /* move the pointer to the Y pixel to the right */
            in_down_ptr+=1; /* move pointer to the Y pixel to the right */

            /* compute Y */
            sum1=iptr[in_up_ptr-h_half_step]+iptr[in_up_ptr]+
            iptr[in_up_ptr+h_half_step];
            sum2=iptr[in_ptr-h_half_step]+iptr[in_ptr]+iptr[in_ptr+h_half_step];
            sum3=iptr[in_down_ptr-h_half_step]+
            iptr[in_down_ptr]+iptr[in_down_ptr+h_half_step];
            *optr=(uint8)(((sum1+sum2+sum3)*455) >> 12); /* divided by 9 */
            optr++;  /* increment output pointer */
            in_up_ptr+=h_step-1;/* move the pointer to the sample position */
            in_ptr+=h_step-1;   /* move the pointer to the sample position */
            in_down_ptr+=h_step-1; /* move pointer to the sample position */
          }
        }
        h_ptr+=h_step;  /* move to next pixel to the right */
      } /* end of for loop j */
      /* modify the vertical pointer position for the next row */
      v_ptr+=v_step;
      v_up_ptr= v_ptr-v_half_step;
      v_down_ptr=v_ptr+v_half_step;
    }    /* end of for loop i */

    MSG_LOW("ipl_downsize_med marker_101\n");

    return IPL_SUCCESS;
  }
	else
	{
    /* We must have bayer input, and want YCbCr output */

    /*
    **  Get Output in RGB 888
    */
    if
      (ipl_bayer_to_ycbcr_downsize(
             i_img_ptr->imgPtr,
             i_img_ptr->dx,
             i_img_ptr->dy,
             i_img_ptr->cFormat,
             o_img_ptr->imgPtr,
             o_img_ptr->dx,
             o_img_ptr->dy,
             bip_ptr) != IPL_SUCCESS )
    {
      MSG_LOW("ipl_downsize_med marker_211\n");
      return IPL_FAILURE;
    }

    MSG_LOW("ipl_downsize_med marker_102\n");

    return IPL_SUCCESS;
  }
}  







/* <EJECT> */
/*===========================================================================

FUNCTION ipl_downsize_mn

DESCRIPTION
  This function is used to do resize and color convert.

  The input can be YCbCr, rgb565 or Bayer or rgb888
  The output can be YCbCr or rgb565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image
  bip_ptr points to a bip structure (for Mega Pixel Color Processing Support)
          The bip_ptr can be set to NULL if no Color Processing is desired.
          Demosaicing will still be done when the bip_ptr is set to NULL.

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_downsize_mn
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  int32 k,m,n,mv,nv;
  int32 count=0,step=0,accum_r=0,accum_g=0,accum_b=0;
  int32* row_map;
  int32* col_map;
  uint16 out;
  uint32 row,col;
  int32 i,j;
  int32 r,g,b;

  uint8 r2,g2,b2;
  int32 cdb,cdr,lumad1,lumad2;
  int32 rc,gc,bc;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are = 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  uint8 failed = 0;
  uint32 resizeFactorX;
  uint32 resizeFactorY;

  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  int16 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_downsize_mn marker_0\n");

  IPLMSG("Inside ipl_downsize_mn"); //lint !e774 !e506

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_downsize_mn marker_200\n");
    return IPL_FAILURE;
  }  

  if ((i_img_ptr->dx < o_img_ptr->dx) ||
      (i_img_ptr->dy < o_img_ptr->dy))
  {
    MSG_LOW("ipl_downsize_mn marker_201\n");
    /*
    ** Strictly a downsize
    */
    return IPL_FAILURE;
  }

  // make sure input size is reasonable
  if (!i_img_ptr->dx || !i_img_ptr->dy || !o_img_ptr->dx || !o_img_ptr->dy)
  {
    MSG_LOW("ipl_downsize_mn marker_201.0\n");
    return IPL_FAILURE;
  }

  /*
  ** Malloc the 2 line buffers
  */
  row_map = (int32*)ipl_malloc((o_img_ptr->dx+1)*sizeof(int32));
  if (!row_map) 
  {
    MSG_LOW("ipl_downsize_mn marker_202\n");
    return IPL_NO_MEMORY;
  }

  col_map = (int32*)ipl_malloc((o_img_ptr->dy+1)*sizeof(int32));
  if (!col_map) 
  {
    ipl_sys_free(row_map);
    MSG_LOW("ipl_downsize_mn marker_203\n");
    return IPL_NO_MEMORY;
  }

  MSG_LOW("ipl_downsize_mn marker_1\n");

  /*
  ** Set starting bin to 0
  */
  row_map[0] = -1;
  col_map[0] = -1;
  m = o_img_ptr->dx;
  n = i_img_ptr->dx;
  mv = o_img_ptr->dy;
  nv = i_img_ptr->dy;

  /*
  ** Q7 resize Factors
  */
  resizeFactorX = (n*128)/m;
  resizeFactorY = (nv*128)/mv;

  step = 1;
  count = 0;

  for (k=0;k<n;k++)
  {
    count = count + m;
    if (count >= n)
    {
      count = count - n;
      row_map[step++] = (int32)k;
    }
  }

  step = 1;
  count = 0;

  for (k=0;k<nv;k++)
  {
    count = count + mv;
    if (count >= nv)
    {
      count = count - nv;
      col_map[step++] = (int32)k;
    }
  }

  if (IPL_XFORM_DEBUG) //lint !e774 !e506
  {
    printf("Col Map\n");
    for (col=0;col<o_img_ptr->dx;col++)
      printf("%lu ", col_map[col]);
    printf("\n");

    printf("Row Map\n");
    for (row=0;row<o_img_ptr->dy;row++)
      printf("%lu ", row_map[row]);
    printf("\n");
  }


  if (i_img_ptr->cFormat == IPL_BAYER_GBRG)
  {
    /*
    ** Input is Bayer GBRG
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (int32)(((row*resizeFactorY<<9) + 0x8000L)>>16);
              j = (int32)(((col*resizeFactorX<<9) + 0x8000L)>>16);
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(out >>1);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(out >>1);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(out >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (int32)((((col+1)*resizeFactorX<<9) + 0x8000L)>>16);
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(out >>1);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(out >>1);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(out >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2))= (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2+1))= (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */

          /*
          ** Do some Image Processing
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col++)
            {
              count = (col_map[row+1]-col_map[row]) * 
                (row_map[col+1]-row_map[col]);
              if (count == 0) count = 1;
              accum_r=0;
              accum_g=0;
              accum_b=0;
              for (i = col_map[row]+1;i<=col_map[row+1];i++)
              {
                for (j = row_map[col]+1;j<=row_map[col+1];j++)
                {
                  if (!(j%2) && (i%2))
                  {
                    /*
                    ** R Input Position
                    */
                    r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    b = (uint8)(bc >>2);
                  } else if ((j%2) && !(i%2))
                  {
                    /*
                    ** B Input Position
                    */
                    b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate G and R
                    */
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find R
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    r = (uint8)(rc >>2);
                  } else if ((j%2) && (i%2))
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    r = (uint8)(rc >>1);
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    b = (uint8)(bc >>1);
                  } else
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    b = (uint8)(bc >>1);
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    r = (uint8)(rc >>1);
                  }
                  accum_r += r;
                  accum_g += g;
                  accum_b += b;
                }
              }
              accum_r = (accum_r<<3)/count;
              accum_r += 0x4;
              accum_r = accum_r >> 3;
              accum_g = (accum_g<<3)/count;
              accum_g += 0x4;
              accum_g = accum_g >> 3;
              accum_b = (accum_b<<3)/count;
              accum_b += 0x4;
              accum_b = accum_b >> 3;

              /*
              ** Now do the gains
              */
              r = accum_r*bip_ptr->rGain*8 + 0x8000;
              g = accum_g*bip_ptr->gGain*8 + 0x8000;
              b = accum_b*bip_ptr->bGain*8 + 0x8000;
              r = r>>16;
              g = g>>16;
              b = b>>16;
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);

              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (bip_ptr->colorCorrection[0]*r + 
                    bip_ptr->colorCorrection[1]*g + 
                    bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
              gc = (bip_ptr->colorCorrection[3]*r + 
                    bip_ptr->colorCorrection[4]*g + 
                    bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
              bc = (bip_ptr->colorCorrection[6]*r + 
                    bip_ptr->colorCorrection[7]*g + 
                    bip_ptr->colorCorrection[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);

              /*
              ** Write out the resize value
              */
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];

              if (!(col%2))
              {
                /* Cb Path */
                lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
                lumad1 = CLIPIT(lumad1);
                cdb = (bip_ptr->rgbToYcbcr[5]*rc + 
                       bip_ptr->rgbToYcbcr[6]*gc + 
                       bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
                cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
                cdb = CLIPIT(cdb);
                *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad1;
              } else {
                /* Cr Path */
                lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
                lumad2 = CLIPIT(lumad2);
                cdr = (bip_ptr->rgbToYcbcr[9]*rc + 
                       bip_ptr->rgbToYcbcr[10]*gc + 
                       bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
                cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
                cdr = CLIPIT(cdr);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdr;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad2;
              }
            } /* End of row for_loop */
          } /* End of Image for_loop */
        }
      } else {
        failed = 1;
      }
    }
  } else if (i_img_ptr->cFormat == IPL_BAYER_GRBG)
  {
    /*
    ** Input is Bayer GRBG
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint32)(((row*resizeFactorY<<9) + 0x8000L)>>16);
              j = (uint32)(((col*resizeFactorX<<9) + 0x8000L)>>16);
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(out >>1);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(out >>1);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(out >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint32)((((col+1)*resizeFactorX<<9) + 0x8000L)>>16);
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(out >>1);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(out >>1);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(out >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2))= (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2+1))= (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */

          /*
          ** Do some Image Processing
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col++)
            {
              count = (col_map[row+1]-col_map[row]) * 
                (row_map[col+1]-row_map[col]);
              if (count == 0) count = 1;
              accum_r=0;
              accum_g=0;
              accum_b=0;
              for (i = col_map[row]+1;i<=col_map[row+1];i++)
              {
                for (j = row_map[col]+1;j<=row_map[col+1];j++)
                {
                  if ((j%2) && !(i%2))
                  {
                    /*
                    ** R Input Position
                    */
                    r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    b = (uint8)(bc >>2);
                  } else if (!(j%2) && (i%2))
                  {
                    /*
                    ** B Input Position
                    */
                    b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate G and R
                    */
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find R
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    r = (uint8)(rc >>2);
                  } else if (!(j%2) && !(i%2))
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    r = (uint8)(rc >>1);
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    b = (uint8)(bc >>1);
                  } else
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    b = (uint8)(bc >>1);
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    r = (uint8)(rc >>1);
                  }
                  accum_r += r;
                  accum_g += g;
                  accum_b += b;
                }
              }
              accum_r = (accum_r<<3)/count;
              accum_r += 0x4;
              accum_r = accum_r >> 3;
              accum_g = (accum_g<<3)/count;
              accum_g += 0x4;
              accum_g = accum_g >> 3;
              accum_b = (accum_b<<3)/count;
              accum_b += 0x4;
              accum_b = accum_b >> 3;

              /*
              ** Now do the gains
              */
              r = accum_r*bip_ptr->rGain*8 + 0x8000;
              g = accum_g*bip_ptr->gGain*8 + 0x8000;
              b = accum_b*bip_ptr->bGain*8 + 0x8000;
              r = r>>16;
              g = g>>16;
              b = b>>16;
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);

              /*
              **  Color Correction and Multiplicative Gain
              */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (bip_ptr->colorCorrection[0]*r + 
                    bip_ptr->colorCorrection[1]*g + 
                    bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
              gc = (bip_ptr->colorCorrection[3]*r + 
                    bip_ptr->colorCorrection[4]*g + 
                    bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
              bc = (bip_ptr->colorCorrection[6]*r + 
                    bip_ptr->colorCorrection[7]*g + 
                    bip_ptr->colorCorrection[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);

              /*
              ** Write out the resize value
              */
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];

              if (!(col%2))
              {
                /* Cb Path */
                lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
                lumad1 = CLIPIT(lumad1);
                cdb = (bip_ptr->rgbToYcbcr[5]*rc + 
                       bip_ptr->rgbToYcbcr[6]*gc + 
                       bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
                cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
                cdb = CLIPIT(cdb);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdb;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad1;
              } else {
                /* Cr Path */
                lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
                lumad2 = CLIPIT(lumad2);
                cdr = (bip_ptr->rgbToYcbcr[9]*rc + 
                       bip_ptr->rgbToYcbcr[10]*gc + 
                       bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
                cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
                cdr = CLIPIT(cdr);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdr;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad2;
              }
            } /* End of row for_loop */
          } /* End of Image for_loop */
        }
      } 
      else 
      {
        failed = 1;
      }
    }
  } else if (i_img_ptr->cFormat == IPL_BAYER_RGGB)
  {
    /*
    ** Input is Bayer RGGB
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint32)(((row*resizeFactorY<<9) + 0x8000L)>>16);
              j = (uint32)(((col*resizeFactorX<<9) + 0x8000L)>>16);
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(out >>1);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(out >>1);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(out >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint32)((((col+1)*resizeFactorX<<9) + 0x8000L)>>16);
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(out >>1);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(out >>1);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(out >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2))= (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2+1))= (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */

          /*
          ** Do some Image Processing
          */
          /*
          ** *colorCorrection; * a11 a12 a13 a21 a22 a23 a31 a32 a33
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col++)
            {
              count = (col_map[row+1]-col_map[row]) * 
                (row_map[col+1]-row_map[col]);
              if (count == 0) count = 1;
              accum_r=0;
              accum_g=0;
              accum_b=0;
              for (i = col_map[row]+1;i<=col_map[row+1];i++)
              {
                for (j = row_map[col]+1;j<=row_map[col+1];j++)
                {
                  if (!(j%2) && !(i%2))
                  {
                    /*
                    ** R Input Position
                    */
                    r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    b = (uint8)(bc >>2);
                  } else if ((j%2) && (i%2))
                  {
                    /*
                    ** B Input Position
                    */
                    b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate G and R
                    */
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find R
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    r = (uint8)(rc >>2);
                  } else if ((j%2) && !(i%2))
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    r = (uint8)(rc >>1);
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    b = (uint8)(bc >>1);
                  } else
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    b = (uint8)(bc >>1);
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    r = (uint8)(rc >>1);
                  }
                  accum_r += r;
                  accum_g += g;
                  accum_b += b;
                }
              }
              accum_r = (accum_r<<3)/count;
              accum_r += 0x4;
              accum_r = accum_r >> 3;
              accum_g = (accum_g<<3)/count;
              accum_g += 0x4;
              accum_g = accum_g >> 3;
              accum_b = (accum_b<<3)/count;
              accum_b += 0x4;
              accum_b = accum_b >> 3;

              /*
              ** Now do the gains
              */
              r = accum_r*bip_ptr->rGain*8 + 0x8000;
              g = accum_g*bip_ptr->gGain*8 + 0x8000;
              b = accum_b*bip_ptr->bGain*8 + 0x8000;
              r = r>>16;
              g = g>>16;
              b = b>>16;
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);

              /*
              **  Color Correction and Multiplicative Gain
              */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (bip_ptr->colorCorrection[0]*r + 
                    bip_ptr->colorCorrection[1]*g + 
                    bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
              gc = (bip_ptr->colorCorrection[3]*r + 
                    bip_ptr->colorCorrection[4]*g + 
                    bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
              bc = (bip_ptr->colorCorrection[6]*r + 
                    bip_ptr->colorCorrection[7]*g + 
                    bip_ptr->colorCorrection[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);

              /*
              ** Write out the resize value
              */
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];

              if (!(col%2))
              {
                /* Cb Path */
                lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc + bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
                lumad1 = CLIPIT(lumad1);
                cdb = (bip_ptr->rgbToYcbcr[5]*rc + 
                       bip_ptr->rgbToYcbcr[6]*gc + 
                       bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
                cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
                cdb = CLIPIT(cdb);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdb;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad1;
              } else {
                /* Cr Path */
                lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
                lumad2 = CLIPIT(lumad2);
                cdr = (bip_ptr->rgbToYcbcr[9]*rc + 
                       bip_ptr->rgbToYcbcr[10]*gc + 
                       bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
                cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
                cdr = CLIPIT(cdr);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdr;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad2;
              }
            } /* End of row for_loop */
          } /* End of Image for_loop */
        }
      } else {
        failed = 1;
      }
    }
  } else if (i_img_ptr->cFormat == IPL_BAYER_BGGR)
  {
    /*
    ** Input is Bayer BGGR
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } 
    else 
    {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint32)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);
              j = (uint32)(((uint32)(col*resizeFactorX<<9) + 0x8000L)>>16);
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(out >>1);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(out >>1);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(out >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint32)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000L)>>16);
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(out >>1);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(out >>1);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(out >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + 
                        ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + 
                     ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + 
                        ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + 
                     ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2))= (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2+1))= (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */

          /*
          ** Do some Image Processing
          */
          /*
          ** *colorCorrection; * a11 a12 a13 a21 a22 a23 a31 a32 a33
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col++)
            {
              count = (col_map[row+1]-col_map[row]) * 
                (row_map[col+1]-row_map[col]);
              if (count == 0) count = 1;
              accum_r=0;
              accum_g=0;
              accum_b=0;
              for (i = col_map[row]+1;i<=col_map[row+1];i++)
              {
                for (j = row_map[col]+1;j<=row_map[col+1];j++)
                {
                  if ((j%2) && (i%2))
                  {
                    /*
                    ** R Input Position
                    */
                    r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    b = (uint8)(bc >>2);
                  } else if (!(j%2) && !(i%2))
                  {
                    /*
                    ** B Input Position
                    */
                    b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate G and R
                    */
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find R
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    r = (uint8)(rc >>2);
                  } else if (!(j%2) && (i%2))
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    r = (uint8)(rc >>1);
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    b = (uint8)(bc >>1);
                  } else
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    b = (uint8)(bc >>1);
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    r = (uint8)(rc >>1);
                  }
                  accum_r += r;
                  accum_g += g;
                  accum_b += b;
                }
              }
              accum_r = (accum_r<<3)/count;
              accum_r += 0x4;
              accum_r = accum_r >> 3;
              accum_g = (accum_g<<3)/count;
              accum_g += 0x4;
              accum_g = accum_g >> 3;
              accum_b = (accum_b<<3)/count;
              accum_b += 0x4;
              accum_b = accum_b >> 3;

              /*
              ** Now do the gains
              */
              r = accum_r*bip_ptr->rGain*8 + 0x8000;
              g = accum_g*bip_ptr->gGain*8 + 0x8000;
              b = accum_b*bip_ptr->bGain*8 + 0x8000;
              r = r>>16;
              g = g>>16;
              b = b>>16;
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);

              /*
              **  Color Correction and Multiplicative Gain
              */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (bip_ptr->colorCorrection[0]*r + 
                    bip_ptr->colorCorrection[1]*g + 
                    bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
              gc = (bip_ptr->colorCorrection[3]*r + 
                    bip_ptr->colorCorrection[4]*g + 
                    bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
              bc = (bip_ptr->colorCorrection[6]*r + 
                    bip_ptr->colorCorrection[7]*g + 
                    bip_ptr->colorCorrection[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);

              /*
              ** Write out the resize value
              */
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];

              if (!(col%2))
              {
                /* Cb Path */
                lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
                lumad1 = CLIPIT(lumad1);
                cdb = (bip_ptr->rgbToYcbcr[5]*rc + 
                       bip_ptr->rgbToYcbcr[6]*gc + 
                       bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
                cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
                cdb = CLIPIT(cdb);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdb;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad1;
              } else {
                /* Cr Path */
                lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
                lumad2 = CLIPIT(lumad2);
                cdr = (bip_ptr->rgbToYcbcr[9]*rc + 
                       bip_ptr->rgbToYcbcr[10]*gc + 
                       bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
                cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
                cdr = CLIPIT(cdr);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdr;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad2;
              }
            } /* End of row for_loop */
          } /* End of Image for_loop */
        }
      } 
      else 
      {
        failed = 1;
      }
    }
  } else 
  {
    ipl_rect_type cropi;
    ipl_rect_type cropo;

    cropi.x = 0;
    cropi.y = 0;
    cropi.dx = i_img_ptr->dx;
    cropi.dy = i_img_ptr->dy;

    cropo.x = 0;
    cropo.y = 0;
    cropo.dx = o_img_ptr->dx;
    cropo.dy = o_img_ptr->dy;

    ipl_sys_free(row_map);
    ipl_sys_free(col_map);
    return(ipl_crop_downsize_mn(i_img_ptr,o_img_ptr,&cropi,&cropo,NULL));
  }

  if ((i_img_ptr->cFormat == IPL_BAYER_BGGR) ||
      (i_img_ptr->cFormat == IPL_BAYER_GBRG) ||
      (i_img_ptr->cFormat == IPL_BAYER_RGGB) ||
      (i_img_ptr->cFormat == IPL_BAYER_GRBG))
  {
    uint8 * ptr8a;
    uint8 * ptr8b;

    // now we have to fix the top and bottom rows
    ptr8a = o_img_ptr->imgPtr;
    ptr8b = o_img_ptr->imgPtr + (2*o_img_ptr->dx * (o_img_ptr->dy-1)); 

    for (col=2*o_img_ptr->dx; col; col--)
    {
      *ptr8a = *(ptr8a + 2*o_img_ptr->dx); 
      *ptr8b = *(ptr8b - 2*o_img_ptr->dx); 
      ptr8a++;
      ptr8b++;
    }

    // now we have to fix the left columsn Cb Y 
    // and right columns Cr Y
    ptr8a = o_img_ptr->imgPtr;
    ptr8b = o_img_ptr->imgPtr + (2*o_img_ptr->dx -2); 
    for (row=o_img_ptr->dy; row; row--)
    {
      *ptr8a     = *(ptr8a + 4);  // fix bad Cb
      *(ptr8a+1) = *(ptr8a + 3);  // fix bad Y

      *ptr8b     = *(ptr8a - 4);  // fix bad Cb
      *(ptr8b+1) = *(ptr8a - 1);  // fix bad Y
    }
  }

  ipl_sys_free(row_map);
  ipl_sys_free(col_map);
  if (failed)
  {
    MSG_LOW("ipl_downsize_mn marker_204\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_downsize_mn marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_downsize_mn() */


#if 0


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_downsize_filter

DESCRIPTION
  This function is used to do resize and color convert.

  The input can be YCbCr, rgb565 or Bayer or rgb888
  The output can be YCbCr or rgb565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image
  bip_ptr points to a bip structure (for Mega Pixel Color Processing Support)
          The bip_ptr can be set to NULL if no Color Processing is desired.
          Demosaicing will still be done when the bip_ptr is set to NULL.

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_downsize_filter
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  int32 k,m,n,mv,nv;
  int32 count=0,step=0,accum_r=0,accum_g=0,accum_b=0;
  int32* row_map;
  int32* col_map;
  uint16 out;
  int32 i,j;
  int32 r,g,b;
  uint32 row,col;

  uint8 r2,g2,b2;
  uint8 cb,cr,luma1,luma2;
  int32 cdb,cdr,lumad1,lumad2;
  int32 rc,gc,bc;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are = 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int16 ycbcr2rgb_convert[6] = {8, 25803, -3071, -7672,30399,12};
  uint8 failed = 0;
  uint32 resizeFactorX;
  uint32 resizeFactorY;

  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  int16 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};
   
  MSG_LOW("ipl_downsize_filter marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_downsize_filter marker_200\n");
    return IPL_FAILURE;
  }  

  if ((i_img_ptr->dx < o_img_ptr->dx) ||
      (i_img_ptr->dy < o_img_ptr->dy))
  {
    MSG_LOW("ipl_downsize_filter marker_201\n");
    /*
    ** Strictly a downsize
    */
    return IPL_FAILURE;
  }

  /*
  ** Malloc the 2 line buffers
  */
  row_map = (int32*)ipl_malloc((o_img_ptr->dx+1)*sizeof(int32));
  if (!row_map) 
  {
    MSG_LOW("ipl_downsize_filter marker_202\n");
    return IPL_NO_MEMORY;
  }

  col_map = (int32*)ipl_malloc((o_img_ptr->dy+1)*sizeof(int32));
  if (!col_map) 
  {
    ipl_sys_free(row_map);
    MSG_LOW("ipl_downsize_filter marker_203\n");
    return IPL_NO_MEMORY;
  }

  MSG_LOW("ipl_downsize_filter marker_1\n");

  /*
  ** Set starting bin to 0
  */
  row_map[0] = -1;
  col_map[0] = -1;
  m = o_img_ptr->dx;
  n = i_img_ptr->dx;
  mv = o_img_ptr->dy;
  nv = i_img_ptr->dy;

  /*
  ** Q7 resize Factors
  */
  resizeFactorX = (n*128)/m;
  resizeFactorY = (nv*128)/mv;

  step = 1;
  count = 0;

  for (k=0;k<n;k++)
  {
    count = count + m;
    if (count >= n)
    {
      count = count - n;
      row_map[step++] = (int32)k;
    }
  }

  step = 1;
  count = 0;

  for (k=0;k<nv;k++)
  {
    count = count + mv;
    if (count >= nv)
    {
      count = count - nv;
      col_map[step++] = (int32)k;
    }
  }

  if (i_img_ptr->cFormat == IPL_RGB565)
  {
    if (o_img_ptr->cFormat == IPL_RGB565)
    {
      /*
      ** Resize from RGB565 to RGB565
      */
      for (row=0;row<o_img_ptr->dy;row++)
      {
        for (col=0;col<o_img_ptr->dx;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          accum_r=0;
          accum_g=0;
          accum_b=0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              out = *((uint16*)(i_img_ptr->imgPtr + (i*n+j)*2));
              unpack_rgb565(out,&r2,&g2,&b2);
              accum_r += r2;
              accum_g += g2;
              accum_b += b2;
            }
          }
          accum_r = (accum_r<<3)/count;
          accum_r += 0x4;
          accum_r = accum_r >> 3;
          accum_g = (accum_g<<3)/count;
          accum_g += 0x4;
          accum_g = accum_g >> 3;
          accum_b = (accum_b<<3)/count;
          accum_b += 0x4;
          accum_b = accum_b >> 3;
          /*
          ** Write out the resize value
          */
          k = k+1;
          out = pack_rgb565(accum_r,accum_g,accum_b);
          *(uint16*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= out;
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } else {
      /*
      ** Resize from RGB565 to YCbCr
      */
      failed = 1;
    }
  } else if (i_img_ptr->cFormat == IPL_YCbCr)
  {
    /* Input is YCbCr */
    if (o_img_ptr->cFormat == IPL_RGB565)
    {
      /*
      ** Resize from YCbCr to RGB565
      */
      for (row=0;row<o_img_ptr->dy;row++)
      {
        for (col=0;col<o_img_ptr->dx;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          accum_r=0;
          accum_g=0;
          accum_b=0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              if (!(j%2))
              {
                /* Cb */
                cb = *(uint8*)(i_img_ptr->imgPtr + (i*n+j)*2);
                luma1 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2+1));
                cr = *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)*2);
                luma2 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j+1)*2+1));
                rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                      ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
                gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                      ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
                bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                      ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
                cdb = luma1 + (rc>>16);
                cdr = luma1 + (gc>>16);
                lumad1 = luma1 + (bc>>16);
                r= (uint8)CLIPIT(cdb);
                g= (uint8)CLIPIT(cdr);
                b= (uint8)CLIPIT(lumad1);
              } else {
                /* cr */
                cr = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2));
                luma2 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2+1));
                cb = *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)*2);
                luma1 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j-1)*2+1));

                rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                      ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
                gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                      ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
                bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                      ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
                cdb = luma2 + (rc>>16);
                cdr = luma2 + (gc>>16);
                lumad1 = luma2 + (bc>>16);
                r= (uint8)CLIPIT(cdb);
                g= (uint8)CLIPIT(cdr);
                b= (uint8)CLIPIT(lumad1);
              }
              accum_r += r;
              accum_g += g;
              accum_b += b;
            }
          }
          accum_r = (accum_r<<3)/count;
          accum_r += 0x4;
          accum_r = accum_r >> 3;
          accum_g = (accum_g<<3)/count;
          accum_g += 0x4;
          accum_g = accum_g >> 3;
          accum_b = (accum_b<<3)/count;
          accum_b += 0x4;
          accum_b = accum_b >> 3;
          /*
          ** Write out the resize value
          */
          out = pack_rgb565(accum_r,accum_g,accum_b);
          *(uint16*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= out;
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } else if (o_img_ptr->cFormat == IPL_YCbCr)
    {
      /*
      ** Resize from YCbCr to YCbCr
      */
      for (row=0;row<o_img_ptr->dy;row++)
      {
        for (col=0;col<o_img_ptr->dx;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          accum_r=0;
          accum_g=0;
          accum_b=0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              if (!(j%2))
              {
                /* Cb */
                cb = *(uint8*)(i_img_ptr->imgPtr + (i*n+j)*2);
                luma1 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2+1));
                cr = *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)*2);
                luma2 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j+1)*2+1));
                rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                      ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
                gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                      ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
                bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                      ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
                cdb = luma1 + (rc>>16);
                cdr = luma1 + (gc>>16);
                lumad1 = luma1 + (bc>>16);
                r= (uint8)CLIPIT(cdb);
                g= (uint8)CLIPIT(cdr);
                b= (uint8)CLIPIT(lumad1);
              } else {
                /* Cr */
                cr = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2));
                luma2 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2+1));
                cb = *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)*2);
                luma1 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j-1)*2+1));
                rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                      ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
                gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                      ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
                bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                      ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
                cdb = luma2 + (rc>>16);
                cdr = luma2 + (gc>>16);
                lumad1 = luma2 + (bc>>16);
                r= (uint8)CLIPIT(cdb);
                g= (uint8)CLIPIT(cdr);
                b= (uint8)CLIPIT(lumad1);
              }
              accum_r += r;
              accum_g += g;
              accum_b += b;
            }
          }
          accum_r = (accum_r<<3)/count;
          accum_r += 0x4;
          accum_r = accum_r >> 3;
          accum_g = (accum_g<<3)/count;
          accum_g += 0x4;
          accum_g = accum_g >> 3;
          accum_b = (accum_b<<3)/count;
          accum_b += 0x4;
          accum_b = accum_b >> 3;
          /*
          ** Write out the resize value
          */
          if (!(col%2))
          {
            /* Cb Path */
            lumad1 = (ycbcr_convert[0]*accum_r + 
                      ycbcr_convert[1]*accum_g + 
                      ycbcr_convert[2]*accum_b)*4+0x8000;
            lumad1 = (lumad1>>16) + 16;
            lumad1 = CLIPIT(lumad1);
            cdb = (ycbcr_convert[3]*accum_r + 
                   ycbcr_convert[4]*accum_g + 
                   ycbcr_convert[5]*accum_b)*4+0x8000;
            cdb = (cdb>>16) + 128;
            cdb = CLIPIT(cdb);
            *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
            *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad1;
          } else {
            /* Cr Path */
            lumad2 = (ycbcr_convert[0]*accum_r + 
                      ycbcr_convert[1]*accum_g + 
                      ycbcr_convert[2]*accum_b)*4+0x8000;
            lumad2 = (lumad2>>16) + 16;
            lumad2 = CLIPIT(lumad2);
            cdr = (ycbcr_convert[6]*accum_r + 
                   ycbcr_convert[7]*accum_g + 
                   ycbcr_convert[8]*accum_b)*4+0x8000;
            cdr = (cdr>>16) + 128;
            cdr = CLIPIT(cdr);
            *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdr;
            *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad2;
          }
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } else {
      failed = 1;
    }
  } 
  else if (i_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
  {
    if (o_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
    {
      for (row=0;row<o_img_ptr->dy;row++)
      {
        for (col=0;col<o_img_ptr->dx;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + (i*n+j));
              cdb += *(i_img_ptr->clrPtr + (i*n+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + (i*n+2*(j/2))+1);
            }
          }
          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;
          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;
          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          /*
          ** Write out the resize value
          */
          *(o_img_ptr->imgPtr + ((row*m)+col)) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((row*m)+2*(col/2))) = (unsigned char) cdb;
          *(o_img_ptr->clrPtr + ((row*m)+2*(col/2))+1) = (unsigned char) cdr;
        } 
      } 
    } 
    else 
      failed = 1;
  } 
  else if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    if (o_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
    {
      for (row=0;row<o_img_ptr->dy;row++)
      {
        for (col=0;col<o_img_ptr->dx;col++)
        {
          count = (col_map[row+1]-col_map[row]) * (row_map[col+1]-row_map[col]);
          if (count == 0) count = 1;
          cdb = cdr = lumad1 = 0;
          for (i = col_map[row]+1;i<=col_map[row+1];i++)
          {
            for (j = row_map[col]+1;j<=row_map[col+1];j++)
            {
              lumad1 += *(i_img_ptr->imgPtr + (i*n+j));
              cdb += *(i_img_ptr->clrPtr + ((i/2)*n+2*(j/2)));
              cdr += *(i_img_ptr->clrPtr + ((i/2)*n+2*(j/2))+1);
            }
          }

          cdb = (cdb <<3)/count;
          cdb += 0x4;
          cdb >>= 3;
          cdr = (cdr <<3)/count;
          cdr += 0x4;
          cdr >>= 3;
          lumad1 = (lumad1 <<3)/count;
          lumad1 += 0x4;
          lumad1 >>= 3;

          /*
          ** Write out the resize value
          */
          *(o_img_ptr->imgPtr + ((row*m)+col)) = (unsigned char) lumad1;
          *(o_img_ptr->clrPtr + ((row/2)*m+2*(col/2))) = (unsigned char) cdb;
          *(o_img_ptr->clrPtr + ((row/2)*m+2*(col/2))+1) = (unsigned char) cdr;
        } 
      } 
    } 
    else 
      failed = 1;
  } 
  else if (i_img_ptr->cFormat == IPL_BAYER_GBRG)
  {
    /*
    ** Input is Bayer GBRG
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint32)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);
              j = (uint32)(((uint32)(col*resizeFactorX<<9) + 0x8000L)>>16);
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(out >>1);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(out >>1);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(out >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint32)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000L)>>16);
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(out >>1);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(out >>1);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(out >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2))= (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2+1))= (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */

          /*
          ** Do some Image Processing
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col++)
            {
              count = (col_map[row+1]-col_map[row]) * 
                (row_map[col+1]-row_map[col]);
              if (count == 0) count = 1;
              accum_r=0;
              accum_g=0;
              accum_b=0;
              for (i = col_map[row]+1;i<=col_map[row+1];i++)
              {
                for (j = row_map[col]+1;j<=row_map[col+1];j++)
                {
                  if (!(j%2) && (i%2))
                  {
                    /*
                    ** R Input Position
                    */
                    r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    b = (uint8)(bc >>2);
                  } else if ((j%2) && !(i%2))
                  {
                    /*
                    ** B Input Position
                    */
                    b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate G and R
                    */
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find R
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    r = (uint8)(rc >>2);
                  } else if ((j%2) && (i%2))
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    r = (uint8)(rc >>1);
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    b = (uint8)(bc >>1);
                  } else
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    b = (uint8)(bc >>1);
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    r = (uint8)(rc >>1);
                  }
                  accum_r += r;
                  accum_g += g;
                  accum_b += b;
                }
              }
              accum_r = (accum_r<<3)/count;
              accum_r += 0x4;
              accum_r = accum_r >> 3;
              accum_g = (accum_g<<3)/count;
              accum_g += 0x4;
              accum_g = accum_g >> 3;
              accum_b = (accum_b<<3)/count;
              accum_b += 0x4;
              accum_b = accum_b >> 3;

              /*
              ** Now do the gains
              */
              r = accum_r*bip_ptr->rGain*8 + 0x8000;
              g = accum_g*bip_ptr->gGain*8 + 0x8000;
              b = accum_b*bip_ptr->bGain*8 + 0x8000;
              r = r>>16;
              g = g>>16;
              b = b>>16;
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);

              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (bip_ptr->colorCorrection[0]*r + 
                    bip_ptr->colorCorrection[1]*g + 
                    bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
              gc = (bip_ptr->colorCorrection[3]*r + 
                    bip_ptr->colorCorrection[4]*g + 
                    bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
              bc = (bip_ptr->colorCorrection[6]*r + 
                    bip_ptr->colorCorrection[7]*g + 
                    bip_ptr->colorCorrection[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);

              /*
              ** Write out the resize value
              */
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];

              if (!(col%2))
              {
                /* Cb Path */
                lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
                lumad1 = CLIPIT(lumad1);
                cdb = (bip_ptr->rgbToYcbcr[5]*rc + 
                       bip_ptr->rgbToYcbcr[6]*gc + 
                       bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
                cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
                cdb = CLIPIT(cdb);
                *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad1;
              } else {
                /* Cr Path */
                lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
                lumad2 = CLIPIT(lumad2);
                cdr = (bip_ptr->rgbToYcbcr[9]*rc + 
                       bip_ptr->rgbToYcbcr[10]*gc + 
                       bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
                cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
                cdr = CLIPIT(cdr);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdr;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad2;
              }
            } /* End of row for_loop */
          } /* End of Image for_loop */
        }
      } else {
        failed = 1;
      }
    }
  } else if (i_img_ptr->cFormat == IPL_BAYER_GRBG)
  {
    /*
    ** Input is Bayer GRBG
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint32)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);
              j = (uint32)(((uint32)(col*resizeFactorX<<9) + 0x8000L)>>16);
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(out >>1);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(out >>1);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(out >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint32)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000L)>>16);
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(out >>1);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(out >>1);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(out >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2))= (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2+1))= (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */

          /*
          ** Do some Image Processing
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col++)
            {
              count = (col_map[row+1]-col_map[row]) * 
                (row_map[col+1]-row_map[col]);
              if (count == 0) count = 1;
              accum_r=0;
              accum_g=0;
              accum_b=0;
              for (i = col_map[row]+1;i<=col_map[row+1];i++)
              {
                for (j = row_map[col]+1;j<=row_map[col+1];j++)
                {
                  if ((j%2) && !(i%2))
                  {
                    /*
                    ** R Input Position
                    */
                    r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    b = (uint8)(bc >>2);
                  } else if (!(j%2) && (i%2))
                  {
                    /*
                    ** B Input Position
                    */
                    b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate G and R
                    */
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find R
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    r = (uint8)(rc >>2);
                  } else if (!(j%2) && !(i%2))
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    r = (uint8)(rc >>1);
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    b = (uint8)(bc >>1);
                  } else
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    b = (uint8)(bc >>1);
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    r = (uint8)(rc >>1);
                  }
                  accum_r += r;
                  accum_g += g;
                  accum_b += b;
                }
              }
              accum_r = (accum_r<<3)/count;
              accum_r += 0x4;
              accum_r = accum_r >> 3;
              accum_g = (accum_g<<3)/count;
              accum_g += 0x4;
              accum_g = accum_g >> 3;
              accum_b = (accum_b<<3)/count;
              accum_b += 0x4;
              accum_b = accum_b >> 3;

              /*
              ** Now do the gains
              */
              r = accum_r*bip_ptr->rGain*8 + 0x8000;
              g = accum_g*bip_ptr->gGain*8 + 0x8000;
              b = accum_b*bip_ptr->bGain*8 + 0x8000;
              r = r>>16;
              g = g>>16;
              b = b>>16;
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);

              /*
              **  Color Correction and Multiplicative Gain
              */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (bip_ptr->colorCorrection[0]*r + 
                    bip_ptr->colorCorrection[1]*g + 
                    bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
              gc = (bip_ptr->colorCorrection[3]*r + 
                    bip_ptr->colorCorrection[4]*g + 
                    bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
              bc = (bip_ptr->colorCorrection[6]*r + 
                    bip_ptr->colorCorrection[7]*g + 
                    bip_ptr->colorCorrection[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);

              /*
              ** Write out the resize value
              */
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];

              if (!(col%2))
              {
                /* Cb Path */
                lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
                lumad1 = CLIPIT(lumad1);
                cdb = (bip_ptr->rgbToYcbcr[5]*rc + 
                       bip_ptr->rgbToYcbcr[6]*gc + 
                       bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
                cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
                cdb = CLIPIT(cdb);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdb;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad1;
              } else {
                /* Cr Path */
                lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
                lumad2 = CLIPIT(lumad2);
                cdr = (bip_ptr->rgbToYcbcr[9]*rc + 
                       bip_ptr->rgbToYcbcr[10]*gc + 
                       bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
                cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
                cdr = CLIPIT(cdr);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdr;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad2;
              }
            } /* End of row for_loop */
          } /* End of Image for_loop */
        }
      } else {
        failed = 1;
      }
    }
  } else if (i_img_ptr->cFormat == IPL_BAYER_RGGB)
  {
    /*
    ** Input is Bayer RGGB
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint32)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);
              j = (uint32)(((uint32)(col*resizeFactorX<<9) + 0x8000L)>>16);
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(out >>1);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(out >>1);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(out >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint32)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000L)>>16);
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(out >>1);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(out >>1);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(out >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2))= (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2+1))= (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */

          /*
          ** Do some Image Processing
          */
          /*
          ** *colorCorrection; * a11 a12 a13 a21 a22 a23 a31 a32 a33
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col++)
            {
              count = (col_map[row+1]-col_map[row]) * 
                (row_map[col+1]-row_map[col]);
              if (count == 0) count = 1;
              accum_r=0;
              accum_g=0;
              accum_b=0;
              for (i = col_map[row]+1;i<=col_map[row+1];i++)
              {
                for (j = row_map[col]+1;j<=row_map[col+1];j++)
                {
                  if (!(j%2) && !(i%2))
                  {
                    /*
                    ** R Input Position
                    */
                    r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    b = (uint8)(bc >>2);
                  } else if ((j%2) && (i%2))
                  {
                    /*
                    ** B Input Position
                    */
                    b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate G and R
                    */
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find R
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    r = (uint8)(rc >>2);
                  } else if ((j%2) && !(i%2))
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    r = (uint8)(rc >>1);
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    b = (uint8)(bc >>1);
                  } else
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    b = (uint8)(bc >>1);
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    r = (uint8)(rc >>1);
                  }
                  accum_r += r;
                  accum_g += g;
                  accum_b += b;
                }
              }
              accum_r = (accum_r<<3)/count;
              accum_r += 0x4;
              accum_r = accum_r >> 3;
              accum_g = (accum_g<<3)/count;
              accum_g += 0x4;
              accum_g = accum_g >> 3;
              accum_b = (accum_b<<3)/count;
              accum_b += 0x4;
              accum_b = accum_b >> 3;

              /*
              ** Now do the gains
              */
              r = accum_r*bip_ptr->rGain*8 + 0x8000;
              g = accum_g*bip_ptr->gGain*8 + 0x8000;
              b = accum_b*bip_ptr->bGain*8 + 0x8000;
              r = r>>16;
              g = g>>16;
              b = b>>16;
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);

              /*
              **  Color Correction and Multiplicative Gain
              */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (bip_ptr->colorCorrection[0]*r + 
                    bip_ptr->colorCorrection[1]*g + 
                    bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
              gc = (bip_ptr->colorCorrection[3]*r + 
                    bip_ptr->colorCorrection[4]*g + 
                    bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
              bc = (bip_ptr->colorCorrection[6]*r + 
                    bip_ptr->colorCorrection[7]*g + 
                    bip_ptr->colorCorrection[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);

              /*
              ** Write out the resize value
              */
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];

              if (!(col%2))
              {
                /* Cb Path */
                lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc + bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
                lumad1 = CLIPIT(lumad1);
                cdb = (bip_ptr->rgbToYcbcr[5]*rc + 
                       bip_ptr->rgbToYcbcr[6]*gc + 
                       bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
                cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
                cdb = CLIPIT(cdb);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdb;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad1;
              } else {
                /* Cr Path */
                lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
                lumad2 = CLIPIT(lumad2);
                cdr = (bip_ptr->rgbToYcbcr[9]*rc + 
                       bip_ptr->rgbToYcbcr[10]*gc + 
                       bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
                cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
                cdr = CLIPIT(cdr);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdr;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad2;
              }
            } /* End of row for_loop */
          } /* End of Image for_loop */
        }
      } else {
        failed = 1;
      }
    }
  } else if (i_img_ptr->cFormat == IPL_BAYER_BGGR)
  {
    /*
    ** Input is Bayer BGGR
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } 
    else 
    {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint32)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);
              j = (uint32)(((uint32)(col*resizeFactorX<<9) + 0x8000L)>>16);
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(out >>1);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(out >>1);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(out >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint32)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000L)>>16);
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(out >>1);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)));
                g2 = (uint8)(out>>1);
                /*
                ** Find R
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(out >>1);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(out >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(out >>1);
                out = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(out >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + 
                        ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + 
                     ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + 
                        ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + 
                     ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2))= (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col+1)*2+1))= (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */

          /*
          ** Do some Image Processing
          */
          /*
          ** *colorCorrection; * a11 a12 a13 a21 a22 a23 a31 a32 a33
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col++)
            {
              count = (col_map[row+1]-col_map[row]) * 
                (row_map[col+1]-row_map[col]);
              if (count == 0) count = 1;
              accum_r=0;
              accum_g=0;
              accum_b=0;
              for (i = col_map[row]+1;i<=col_map[row+1];i++)
              {
                for (j = row_map[col]+1;j<=row_map[col+1];j++)
                {
                  if ((j%2) && (i%2))
                  {
                    /*
                    ** R Input Position
                    */
                    r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    b = (uint8)(bc >>2);
                  } else if (!(j%2) && !(i%2))
                  {
                    /*
                    ** B Input Position
                    */
                    b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate G and R
                    */
                    /*
                    ** Find G
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                      *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    rc = rc*rc;
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                      *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    bc = bc*bc;
                    if (rc > bc)
                    {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                        *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                    } else {
                      gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                        *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                    }
                    g = (uint8)(gc>>1);
                    /*
                    ** Find R
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                    r = (uint8)(rc >>2);
                  } else if (!(j%2) && (i%2))
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    r = (uint8)(rc >>1);
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    b = (uint8)(bc >>1);
                  } else
                  {
                    /*
                    ** G Input Position
                    */
                    g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                    /*
                    ** Interpolate R and B
                    */
                    bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                    b = (uint8)(bc >>1);
                    rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                           *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                    r = (uint8)(rc >>1);
                  }
                  accum_r += r;
                  accum_g += g;
                  accum_b += b;
                }
              }
              accum_r = (accum_r<<3)/count;
              accum_r += 0x4;
              accum_r = accum_r >> 3;
              accum_g = (accum_g<<3)/count;
              accum_g += 0x4;
              accum_g = accum_g >> 3;
              accum_b = (accum_b<<3)/count;
              accum_b += 0x4;
              accum_b = accum_b >> 3;

              /*
              ** Now do the gains
              */
              r = accum_r*bip_ptr->rGain*8 + 0x8000;
              g = accum_g*bip_ptr->gGain*8 + 0x8000;
              b = accum_b*bip_ptr->bGain*8 + 0x8000;
              r = r>>16;
              g = g>>16;
              b = b>>16;
              r=CLIPIT(r);
              g=CLIPIT(g);
              b=CLIPIT(b);

              /*
              **  Color Correction and Multiplicative Gain
              */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (bip_ptr->colorCorrection[0]*r + 
                    bip_ptr->colorCorrection[1]*g + 
                    bip_ptr->colorCorrection[2]*b)*8 + 0x8000;
              gc = (bip_ptr->colorCorrection[3]*r + 
                    bip_ptr->colorCorrection[4]*g + 
                    bip_ptr->colorCorrection[5]*b)*8 + 0x8000;
              bc = (bip_ptr->colorCorrection[6]*r + 
                    bip_ptr->colorCorrection[7]*g + 
                    bip_ptr->colorCorrection[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);

              /*
              ** Write out the resize value
              */
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];

              if (!(col%2))
              {
                /* Cb Path */
                lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
                lumad1 = CLIPIT(lumad1);
                cdb = (bip_ptr->rgbToYcbcr[5]*rc + 
                       bip_ptr->rgbToYcbcr[6]*gc + 
                       bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
                cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
                cdb = CLIPIT(cdb);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdb;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad1;
              } else {
                /* Cr Path */
                lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + 
                          bip_ptr->rgbToYcbcr[2]*gc + 
                          bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
                lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
                lumad2 = CLIPIT(lumad2);
                cdr = (bip_ptr->rgbToYcbcr[9]*rc + 
                       bip_ptr->rgbToYcbcr[10]*gc + 
                       bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
                cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
                cdr = CLIPIT(cdr);
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2))= (uint8)cdr;
                *(uint8*)(o_img_ptr->imgPtr+(((row*m)+col)*2+1))= (uint8)lumad2;
              }
            } /* End of row for_loop */
          } /* End of Image for_loop */
        }
      } 
      else 
      {
        failed = 1;
      }
    }
  } else 
  {
    failed = 1;
  }

  if ((i_img_ptr->cFormat == IPL_BAYER_BGGR) ||
      (i_img_ptr->cFormat == IPL_BAYER_GBRG) ||
      (i_img_ptr->cFormat == IPL_BAYER_RGGB) ||
      (i_img_ptr->cFormat == IPL_BAYER_GRBG))
  {
    uint8 * ptr8a;
    uint8 * ptr8b;

    // now we have to fix the top and bottom rows
    ptr8a = o_img_ptr->imgPtr;
    ptr8b = o_img_ptr->imgPtr + (2*o_img_ptr->dx * (o_img_ptr->dy-1)); 

    for (col=2*o_img_ptr->dx; col; col--)
    {
      *ptr8a = *(ptr8a + 2*o_img_ptr->dx); 
      *ptr8b = *(ptr8b - 2*o_img_ptr->dx); 
      ptr8a++;
      ptr8b++;
    }

    // now we have to fix the left columsn Cb Y 
    // and right columns Cr Y
    ptr8a = o_img_ptr->imgPtr;
    ptr8b = o_img_ptr->imgPtr + (2*o_img_ptr->dx -2); 
    for (row=o_img_ptr->dy; row; row--)
    {
      *ptr8a     = *(ptr8a + 4);  // fix bad Cb
      *(ptr8a+1) = *(ptr8a + 3);  // fix bad Y

      *ptr8b     = *(ptr8a - 4);  // fix bad Cb
      *(ptr8b+1) = *(ptr8a - 1);  // fix bad Y
    }
  }

  ipl_sys_free(row_map);
  ipl_sys_free(col_map);
  if (failed)
  {
    MSG_LOW("ipl_downsize_filter marker_204\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_downsize_filter marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_downsize_mn() */


#endif



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_downsize

DESCRIPTION
  This function is used to do resize and color convert.
	It will call downsize_mn or downsize_med to the actual work

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image
  bip_ptr points to a bip structure (for Mega Pixel Color Processing Support)
          The bip_ptr can be set to NULL if no Color Processing is desired.
          Demosaicing will still be done when the bip_ptr is set to NULL.

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_downsize
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  ipl_status_type retval;
  int v_half_step, h_half_step;

  MSG_LOW("ipl_downsize marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_downsize marker_201\n");
    return IPL_FAILURE;
  }  

  // quick sanity check
  if (i_img_ptr->dx == 0 || i_img_ptr->dy == 0 ||
      o_img_ptr->dx == 0 || o_img_ptr->dx == 0)
  {
    MSG_LOW("ipl_downsize marker_200\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_downsize marker_1\n");

  switch (i_img_ptr->cFormat)
  {
    case IPL_RGB565:
      if (o_img_ptr->cFormat == IPL_RGB565)
          //ipl2_downsize(i_img_ptr, o_img_ptr, NULL);
		      retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, NULL);
      else if (o_img_ptr->cFormat == IPL_YCbCr)
          retval = ipl2_downsize(i_img_ptr, o_img_ptr, NULL);
      else 
          retval = IPL_FAILURE;
    break;

    case IPL_YCbCr:
      if (o_img_ptr->cFormat == IPL_RGB565)
        //retval = ipl2_downsize(i_img_ptr, o_img_ptr, NULL);
		    retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, NULL);
      else if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        // criteria for using downsize_med
        h_half_step=(i_img_ptr->dx-8)/(2*o_img_ptr->dx+1);
        v_half_step=(i_img_ptr->dy-8)/(2*o_img_ptr->dy+1);

        if (h_half_step<2 || v_half_step<2)
			    retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, bip_ptr);
		    else
          retval = ipl_downsize_med(i_img_ptr, o_img_ptr, bip_ptr);
      }
      else if (o_img_ptr->cFormat == IPL_LUMA_ONLY)
      {
			  retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, bip_ptr);
      }
      else 
        retval = IPL_FAILURE;
    break;

    //case IPL_YCbCr420_LINE_PK:
    //    retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, NULL);
    //break;

    case IPL_YCrCb420_LINE_PK:
		    retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, NULL);
    break;

    //case IPL_YCbCr422_LINE_PK:
		//    retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, NULL);
    //break;

    case IPL_YCrCb422_LINE_PK:
		    retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, NULL);
    break;

    case IPL_YCbCr420_FRAME_PK:
      if (o_img_ptr->cFormat == IPL_RGB565)
        retval = ipl2_downsize(i_img_ptr, o_img_ptr, NULL);
      if (o_img_ptr->cFormat == IPL_RGB666)
        retval = ipl2_downsize(i_img_ptr, o_img_ptr, NULL);
      else if (o_img_ptr->cFormat == IPL_YCbCr)
        retval = ipl2_downsize(i_img_ptr, o_img_ptr, NULL);
      else
        retval = IPL_FAILURE;
    break;

    case IPL_BAYER_GBRG: 
    case IPL_BAYER_BGGR: 
    case IPL_BAYER_GRBG: 
    case IPL_BAYER_RGGB:
      // criteria for using downsize_med
      h_half_step=(i_img_ptr->dx-8)/(2*o_img_ptr->dx+1);
      v_half_step=(i_img_ptr->dy-8)/(2*o_img_ptr->dy+1);

      if (o_img_ptr->cFormat == IPL_RGB565)
      {
        if ((i_img_ptr->dx != o_img_ptr->dx)
            && (h_half_step < 2 || v_half_step < 2))
			    retval = IPL_FAILURE;
		    else
          retval = ipl_downsize_med(i_img_ptr, o_img_ptr, bip_ptr);
      }
      else if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (h_half_step<2 || v_half_step<2)
			    retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, bip_ptr);
		    else
          retval = ipl_downsize_med(i_img_ptr, o_img_ptr, bip_ptr);
      }
      else
			  retval = IPL_FAILURE;
    break;

    default:
		  retval = ipl_downsize_mn(i_img_ptr, o_img_ptr, NULL);
    break;
  }

  if (retval == IPL_FAILURE)
    MSG_LOW("ipl_downsize marker_202\n");
  else
    MSG_LOW("ipl_downsize marker_100\n");

	return retval;
}


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_rotate90_frame

DESCRIPTION
  This function performs 90 degree clockwise rotation of frames with special 
  processing for transparent pixels. Four types of rotation are supported: 
  0 degrees, 90 degrees, 180 degrees, and 270 degrees.
  
  Input and output frame images must be in RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr          pointer to the input frame image
  o_img_ptr          pointer to the output frame image
  rotate             type of rotation to perform
  transparentValue   16-bit transparent pixel value

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_rotate90_frame
(
  ipl_image_type* i_img_ptr,        /* Points to the input image      */
  ipl_image_type* o_img_ptr,        /* Points to the output image     */
  ipl_rotate90_type rotate,         /* Type of rotation               */
  uint16 transparentValue           /* Transparent pixel value        */
)
{
  /* Take an image of size of any size and converts it to black and white
  */
  int32 index,rowInc=0,colInc=0,startPos=0;
  uint32 row,col;
  ipl_image_type* input_img_ptr;
  ipl_image_type* output_img_ptr;

  MSG_LOW("ipl_rotate90_frame marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_rotate90_frame marker_200\n");
    return IPL_FAILURE;
  }
  input_img_ptr = i_img_ptr;
  output_img_ptr = o_img_ptr;
  

  if ((input_img_ptr->cFormat != IPL_RGB565) ||
      (output_img_ptr->cFormat != IPL_RGB565))
  {
    MSG_LOW("ipl_rotate90_frame marker_201\n");
    /* Only RGB565 frames currently supported */
    return IPL_FAILURE;
  }
  if ((rotate==IPL_NOROT) || (rotate==IPL_ROT180))
  {
    if ((input_img_ptr->dx != output_img_ptr->dx) ||
        (input_img_ptr->dy != output_img_ptr->dy))
    {
      MSG_LOW("ipl_rotate90_frame marker_202\n");
      /* Invalid dimensions */
      return IPL_FAILURE;
    }
  } else {
    if ((input_img_ptr->dx != output_img_ptr->dy) ||
        (input_img_ptr->dy != output_img_ptr->dx))
    {
      MSG_LOW("ipl_rotate90_frame marker_203\n");
      /* Invalid dimensions */
      return IPL_FAILURE;
    }
  }

  if ((rotate == IPL_NOROT))
  {
    /* Dont rotate or reflect */
    startPos = 0;
    colInc = 1;
    rowInc = 1;
  } else if ((rotate==IPL_ROT90))
  {
    /* Rotate 90 clockwise */
    startPos = input_img_ptr->dx*(input_img_ptr->dy-1);
    colInc = -(int32)input_img_ptr->dx;
    rowInc = (input_img_ptr->dx)*(input_img_ptr->dy-1)+1;
  } else if ((rotate==IPL_ROT180))
  {
    /* Rotate 180 clockwise */
    startPos = (input_img_ptr->dx*input_img_ptr->dy)-1;
    colInc = -1;
    rowInc = -1;
  } else if (rotate==IPL_ROT270)
  {
    /* Rotate 270 clockwise */
    startPos = input_img_ptr->dx-1;
    colInc = input_img_ptr->dx;
    rowInc = -(int32)(input_img_ptr->dx)*(input_img_ptr->dy-1)-1;
  } else
  {
    MSG_LOW("ipl_rotate90_frame marker_204\n");
    /* Option Not supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_rotate90_frame marker_1\n");

  /* initialize the index to starting position */
  index = startPos<<1;  /* byte addressed */
  /* Now loop through the image once */
  for(row = 0; row < output_img_ptr->dy; row++){
    for(col = 0; col < (output_img_ptr->dx); col++){
      if (*((uint16*)(input_img_ptr->imgPtr + index)) == transparentValue)
      {
        /* Write transparent pixel in output */
        *((uint16*)(output_img_ptr->imgPtr+((row*output_img_ptr->dx+col)<<1)))=
          transparentValue;
      } else {
        /* Copy the input to output */
        *((uint16*)(output_img_ptr->imgPtr+((row*output_img_ptr->dx+col)<<1)))=
          *((uint16*)(input_img_ptr->imgPtr + index));
      }
      index = index + (colInc<<1); /* byte addressed */
    } /* End of col loop */
    index = index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
  } /* End of row loop */

  MSG_LOW("ipl_rotate90_frame marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_rotate90_frame */



/* <EJECT> */
/*===========================================================================

FUNCTION image_rot_add_crop_rgbi_colrow

DESCRIPTION
  This function is an optimized version to do rotation, adding frame,cropping
  and color conversion from YCbCr to RGB565.

  It is assumed that the caller will set the input, frame and output
  image structures to have the correct dimensions after
  and rotation.

  The frame is assumed to be established in 2 pixel units. ie. the minimum 
  length of any frame edge is 2 pixels.

  Input is assumed to be in rgb565 format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  input_frame_ptr points to the frame image
  transparentValue is the transparent pixel value
  output_img_ptr points to the output image
  startPos is the start increment
  colInc is the col incr
  rowInc is the row incr

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type image_rot_add_crop_rgbi_colrow
( 
  ipl_image_type* i_img_ptr,    /* Input Image Pointer                     */
  ipl_image_type* i_frame_ptr,  /* Input Frame Pointer                     */
  uint16 transparentValue, /* transparent value of pixel                   */
  ipl_image_type* o_img_ptr,   /* Output Image Pointer                     */
  int32 startPos,
  int32 colInc,
  int32 rowInc,
  ipl_rect_type* crop
)
{
  int32 src_index,dest_index,destInc;
  uint32 row,col;
  ipl_image_type* input_frame_ptr;
  ipl_image_type* output_img_ptr;
  register uint16* inputImgPtr;
  register uint16* outputImgPtr;
  register uint16* frameImgPtr = NULL;
  register uint16 out,out2;
  register uint32 out32;
  register uint8 r;
  register uint32* data_out;
  //register int32 rowIncr = (rowInc<<1) - (colInc<<1);

  register int32 rowIncr = (rowInc<<1);
  register int32 colIncr = colInc<<1;
  register int32 frameIncr = 0;
  register int32 framePresent = 0;

  MSG_LOW("image_rot_add_crop_rgbi_colrow marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr ||
      !crop)
  {
    MSG_LOW("image_rot_add_crop_rgbi_colrow marker_200\n");
    return IPL_FAILURE;
  }

  input_frame_ptr = i_frame_ptr;
  output_img_ptr = o_img_ptr;
  inputImgPtr = (uint16*)i_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  data_out = (uint32*)o_img_ptr->imgPtr;
  dest_index = 0;

  if (input_frame_ptr)
  {
    frameImgPtr = (uint16*)(input_frame_ptr->imgPtr);
    frameIncr = (1 - input_frame_ptr->dx*crop->dy)<<1;
    framePresent = 1;
  }

  MSG_LOW("image_rot_add_crop_rgbi_colrow marker_1\n");

  if ( output_img_ptr->cFormat == IPL_RGB565)
  {
    /*
    ** begin of Input = rgb565, output = rgb565
    */

    /* initialize the index to starting position */
    src_index = startPos<<1;  /* byte addressed */
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;

    outputImgPtr = (uint16*)((uint32)outputImgPtr +  dest_index);
    inputImgPtr = (uint16*)((uint32)inputImgPtr + src_index);

    destInc = (1 - output_img_ptr->dx*crop->dy) * 2;


    if (frameImgPtr)
    {
      /* Now loop through the image once */
      for(col = crop->dx; col; col--)
      {
        for(row = crop->dy; row; row--)
        {
          if (((*(uint16*)(frameImgPtr) != transparentValue)))
          {
            /*
            ** Copy Over the Frame
            */
            *outputImgPtr = *frameImgPtr;

            outputImgPtr += output_img_ptr->dx;
            frameImgPtr += input_frame_ptr->dx;
            inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
          } 
          else 
          {
            /*
            ** Copy Over the Image
            */
            frameImgPtr += input_frame_ptr->dx;
            *outputImgPtr = *inputImgPtr;
            outputImgPtr += output_img_ptr->dx;
            inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
          }
        }

        inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
        outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
        frameImgPtr = (uint16*)((uint32)frameImgPtr + frameIncr);
      }
    }
    else
    {
      /* Now loop through the image once */
      for(col = crop->dx; col; col--)
      {
        for(row = crop->dy; row; row--)
        {
          *outputImgPtr = *inputImgPtr;
          outputImgPtr += output_img_ptr->dx;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        }

        inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
        outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
      }
    }
    MSG_LOW("image_rot_add_crop_rgbi_colrow marker_100\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_RGB444)
  {
    /* initialize the index to starting position */
    src_index = startPos<<1;  /* byte addressed */
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
    outputImgPtr = (uint16*)((uint32)outputImgPtr +  dest_index);
    inputImgPtr = (uint16*)((uint32)inputImgPtr + src_index);
    destInc = (1 - output_img_ptr->dx*crop->dy) * 2;

    if (IPL_XFORM_DEBUG) //lint !e774 !e506
    {
      printf("src_index  %ld\n", src_index);
      printf("dest_index %ld\n", dest_index);
      printf("destInc    %ld\n", destInc);
      printf("colIncr    %ld\n", colIncr);
      printf("rowIncr    %ld\n", rowIncr);
      printf("frameIncr  %ld\n", frameIncr);
    }

    /* Now loop through the image once */
    if (framePresent && frameImgPtr)
    {
      for(col = crop->dx; col; col--)
      {
        for(row = crop->dy; row; row--)
        {
          if (*(uint16*)(frameImgPtr) != transparentValue)
          {
            /*
            ** Copy Over the Frame and convert from rgb565 to rgb444
            */
            out = *(uint16*)(frameImgPtr);
            r = (unsigned char)((out&0xF800)>>8);
            out2 = r444[r];
            r = (unsigned char)((out&0x07E0)>>3);
            out2 += g444[r];
            r = (unsigned char)((out&0x001F)<<3);
            out2 += b444[r];
            *outputImgPtr = out2;
          }
          else
          {
            *outputImgPtr = *inputImgPtr;
          }

          outputImgPtr += output_img_ptr->dx;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
          frameImgPtr += input_frame_ptr->dx;
        }

        inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
        outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
        frameImgPtr = (uint16*)((uint32)frameImgPtr + frameIncr);
      }
    }
    else
    {
      for(col = crop->dx; col; col--)
      {
        for(row = crop->dy; row; row--)
        {
          /*
          ** Copy Over the Frame and convert from rgb565 to rgb444
          */
          out = *(uint16*)(inputImgPtr);
          r = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r];
          r = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r];
          r = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r];
          *outputImgPtr = out2;

          outputImgPtr += output_img_ptr->dx;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        }

        inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
        outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
      }
    }

    MSG_LOW("image_rot_add_crop_rgbi_colrow marker_101\n");
    return IPL_SUCCESS;
  }
  else if (output_img_ptr->cFormat == IPL_RGB666)
  {
    /*
    ** begin of Input = rgb565, output = rgb666
    */
    /* initialize the index to starting position */
    src_index = startPos<<1;  /* byte addressed */
    outputImgPtr = (uint16*)((uint32)outputImgPtr +  dest_index);
    inputImgPtr = (uint16*)((uint32)inputImgPtr + src_index);
    destInc = (1 - output_img_ptr->dx*crop->dy);

    data_out += (crop->x + output_img_ptr->dx*crop->y);

    if (frameImgPtr)
    {
      /* Now loop through the image once */
      for(col = crop->dx; col; col--)
      {
        for(row = crop->dy; row; row--)
        {
          if (*(uint16*)(frameImgPtr) != transparentValue)
          {
            /*
            ** Copy Over the Frame and convert from rgb565 to rgb666
            */
            out = *(uint16*)(frameImgPtr);
            r = (unsigned char)((out&0xF800)>>8);
            out32 = r666[r];
            r = (unsigned char)((out&0x07E0)>>3);
            out32 += g666[r];
            r = (unsigned char)((out&0x001F)<<3);
            out32 += b666[r];
            *data_out = out32;

          }
          else 
          {
            out = *(uint16*)(inputImgPtr);
            r = (unsigned char)((out&0xF800)>>8);
            out32 = r666[r];
            r = (unsigned char)((out&0x07E0)>>3);
            out32 += g666[r];
            r = (unsigned char)((out&0x001F)<<3);
            out32 += b666[r];
            *data_out = out32;

          }

          data_out += (output_img_ptr->dx);
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
          frameImgPtr += (input_frame_ptr->dx);
        }

        data_out += destInc;
        inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
        frameImgPtr = (uint16*)((uint32)frameImgPtr + frameIncr);
      }
    }
    else
    {
      /* Now loop through the image once */
      for(col = crop->dx; col; col--)
      {
        for(row = crop->dy; row; row--)
        {
          out = *(uint16*)(inputImgPtr);
          r = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r];
          if (IPL_XFORM_DEBUG) //lint !e774 !e506
            printf("r %d r666 %ld out32 %ld\n",r,r666[r], out32);

          r = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r];
          if (IPL_XFORM_DEBUG) //lint !e774 !e506
            printf("r %d g666 %ld out32 %ld\n",r,g666[r], out32);

          r = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r];
          if (IPL_XFORM_DEBUG) //lint !e774 !e506
            printf("r %d b666 %ld out32 %ld\n",r,b666[r], out32);

          *data_out = out32;

          data_out += (output_img_ptr->dx);
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        }

        data_out += destInc;
        inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
      }
    }
    MSG_LOW("image_rot_add_crop_rgbi_colrow marker_102\n");
    return IPL_SUCCESS;
  } else {
    MSG_LOW("image_rot_add_crop_rgbi_colrow marker_201\n");
    /* Only YCbCr or RGB565 output supported  */
    return IPL_FAILURE;
  }
} 


/* <EJECT> */
/*===========================================================================

FUNCTION image_rot_add_crop_rgbi

DESCRIPTION
  This function is an optimized version to do rotation, adding frame,cropping
  and color conversion from YCbCr to RGB565.

  It is assumed that the caller will set the input, frame and output
  image structures to have the correct dimensions after
  and rotation.

  The frame is assumed to be established in 2 pixel units. ie. the minimum 
  length of any frame edge is 2 pixels.

  Input is assumed to be in rgb565 format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  input_frame_ptr points to the frame image
  transparentValue is the transparent pixel value
  output_img_ptr points to the output image
  startPos is the start increment
  colInc is the col incr
  rowInc is the row incr

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type image_rot_add_crop_rgbi
( 
  ipl_image_type* i_img_ptr,    /* Input Image Pointer                     */
  ipl_image_type* i_frame_ptr,  /* Input Frame Pointer                     */
  uint16 transparentValue, /* transparent value of pixel                   */
  ipl_image_type* o_img_ptr,   /* Output Image Pointer                     */
  int32 startPos,
  int32 colInc,
  int32 rowInc,
  ipl_rect_type* crop
)
{
  int32 src_index,dest_index,destInc;
  int32 frame_index;
  uint32 row,col;
  ipl_image_type* input_frame_ptr;
  ipl_image_type* output_img_ptr;
  register uint16* inputImgPtr;
  register uint16* outputImgPtr;
  register uint16* frameImgPtr = NULL;
  unsigned char r1,g1,b1,r2,g2,b2;
  register uint16 out,out2;
  register uint32 out32;
  register uint8 r;
  register uint32* data_out;
  register int32 rowIncr = (rowInc<<1) - (colInc<<1);
  register int32 colIncr = colInc<<1;
  int32 luma1,luma2,cb,cr;
  register int32 frameIncr = 0;
  register int32 framePresent = 0;


  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("image_rot_add_crop_rgbi marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr ||
      !crop)
  {
    MSG_LOW("image_rot_add_crop_rgbi marker_200\n");
    return IPL_FAILURE;
  }

  input_frame_ptr = i_frame_ptr;
  output_img_ptr = o_img_ptr;
  inputImgPtr = (uint16*)i_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  data_out = (uint32*)o_img_ptr->imgPtr;

  frame_index = 0;
  if (input_frame_ptr)
  {
    frameImgPtr = (uint16*) input_frame_ptr->imgPtr;
    frameIncr = (input_frame_ptr->dx - crop->dx)<<1;
    framePresent = 1;
  }
    
  MSG_LOW("image_rot_add_crop_rgbi marker_1\n");

  if ( output_img_ptr->cFormat == IPL_RGB565 )
  {
    /*
    ** begin of Input = rgb565, output = rgb565
    */

    /* initialize the index to starting position */
    src_index = startPos<<1;  /* byte addressed */
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
    outputImgPtr = (uint16*)((uint32)outputImgPtr +  dest_index);
    inputImgPtr = (uint16*)((uint32)inputImgPtr + src_index);
    destInc = ( output_img_ptr->dx - crop->dx) * 2;

    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = crop->dy; row; row--)
    {
      for(col = crop->dx; col; col--)
      {
        if (*(uint16*)(frameImgPtr) != transparentValue) {
          /*
          ** Copy Over the Frame
          */
          *outputImgPtr = *frameImgPtr;
          outputImgPtr++;
          frameImgPtr++;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        } 
        else 
        {
          /*
          ** Copy Over the Image
          */
          //if (frameImgPtr) frameImgPtr++;
          frameImgPtr++;
          *outputImgPtr = *inputImgPtr;
          outputImgPtr++;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        }
      }
      inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
      outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
      frameImgPtr = (uint16*)((uint32)frameImgPtr + frameIncr);
    }
    }
    else
    {
      /* Now loop through the image once */
      for(row = crop->dy; row; row--)
      {
        for(col = crop->dx; col; col--)
        {
          *outputImgPtr = *inputImgPtr;
          outputImgPtr++;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        }
        inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
        outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
      }
    }
    MSG_LOW("image_rot_add_crop_rgbi marker_100\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_RGB666)
  {
    /*
    ** begin of Input = rgb565, output = rgb666
    */
    /* initialize the index to starting position */
    src_index = startPos<<1;  /* byte addressed */
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<2;
    data_out = (uint32*)((uint32)data_out +  dest_index);
    inputImgPtr = (uint16*)((uint32)inputImgPtr + src_index);
    destInc = ( output_img_ptr->dx - crop->dx) * 4;
    if (frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = crop->dy; row; row--){
      for(col = crop->dx; col; col--){
        if (*(uint16*)(frameImgPtr) != transparentValue) {
          /*
          ** Copy Over the Frame and convert from rgb565 to rgb666
          */
          out = *(uint16*)(frameImgPtr);
          r = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r];
          r = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r];
          r = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r];
          *data_out = out32;
          data_out++;
          frameImgPtr++;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        } else {
          /*
          ** Copy Over the Image and convert from rgb565 to rgb666
          */
          frameImgPtr++;
          out = *(uint16*)(inputImgPtr);
          r = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r];
          r = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r];
          r = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r];
          *data_out = out32;
          data_out++;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        }
      }
      inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
      data_out = (uint32*)((uint32)data_out + destInc);
      frameImgPtr = (uint16*)((uint32)frameImgPtr + frameIncr);
    }
    }
    else
    {
      /* Now loop through the image once */
      for(row = crop->dy; row; row--){
        for(col = crop->dx; col; col--){
          out = *(uint16*)(inputImgPtr);
          r = (unsigned char)((out&0xF800)>>8);
          out32 = r666[r];
          r = (unsigned char)((out&0x07E0)>>3);
          out32 += g666[r];
          r = (unsigned char)((out&0x001F)<<3);
          out32 += b666[r];
          *data_out = out32;
          data_out++;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        }
        inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
        data_out = (uint32*)((uint32)data_out + destInc);
      }
    }
    MSG_LOW("image_rot_add_crop_rgbi marker_101\n");
    return IPL_SUCCESS;
    /*
    ** end of Input = rgb565, output = rgb666
    */
  } else if (output_img_ptr->cFormat == IPL_RGB444)
  {
    /* initialize the index to starting position */
    src_index = startPos<<1;  /* byte addressed */
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
    outputImgPtr = (uint16*)((uint32)outputImgPtr +  dest_index);
    inputImgPtr = (uint16*)((uint32)inputImgPtr + src_index);
    destInc = ( output_img_ptr->dx - crop->dx) * 2;

    if (framePresent && frameImgPtr)
    {
    /* Now loop through the image once */
    for(row = crop->dy; row; row--){
      for(col = crop->dx; col; col--){
        if (*(uint16*)(frameImgPtr) != transparentValue) {
          /*
          ** Copy Over the Frame and convert from rgb565 to rgb444
          */
          out = *(uint16*)(frameImgPtr);
          r = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r];
          r = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r];
          r = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r];
          *outputImgPtr = out2;
          outputImgPtr++;
          frameImgPtr++;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        } else {
          /*
          ** Copy Over the Image and convert from rgb565 to rgb444
          */
          frameImgPtr ++;
          out = *(uint16*)(inputImgPtr);
          r = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r];
          r = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r];
          r = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r];
          *outputImgPtr = out2;
          outputImgPtr++;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
        }
      }
      inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
      outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
      frameImgPtr = (uint16*)((uint32)frameImgPtr + frameIncr);
    }
    }
    else
    {
      for(row = crop->dy; row; row--){
      for(col = crop->dx; col; col--){
          out = *(uint16*)(inputImgPtr);
          r = (unsigned char)((out&0xF800)>>8);
          out2 = r444[r];
          r = (unsigned char)((out&0x07E0)>>3);
          out2 += g444[r];
          r = (unsigned char)((out&0x001F)<<3);
          out2 += b444[r];
          *outputImgPtr = out2;
          outputImgPtr++;
          inputImgPtr = (uint16*)((uint32)inputImgPtr + colIncr);
      }
      inputImgPtr = (uint16*)((uint32)inputImgPtr + rowIncr);
      outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
    }
    }
    MSG_LOW("image_rot_add_crop_rgbi marker_102\n");
    return IPL_SUCCESS;
    /*
    ** end of Input = rgb565, output = rgb444
    */
  } else if (output_img_ptr->cFormat == IPL_YCbCr)
  {
    /*
    ** begin of Input = rgb565, output = YCbCr
    ** No Need to optimize path
    */
    /* initialize the index to starting position */
    src_index = startPos<<1;  /* byte addressed */
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
    destInc = ( output_img_ptr->dx - crop->dx) * 2;
    frame_index = 0;
    /* Now loop through the image once */
    if (frameImgPtr)
    {
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
        if ((*(uint16*)((uint32)frameImgPtr+frame_index))!=transparentValue)
        {
          /* Use frame */
          /* Convert frame to YCbCr */
          out = *((uint16*)((uint32)frameImgPtr + frame_index));
          unpack_rgb565(out,&r1,&g1,&b1);
          luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + 
                   ycbcr_convert[2]*b1)*4+0x8000;
          luma1 = (luma1>>16) + 16;
          luma1= CLIPIT(luma1);
          cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + 
                ycbcr_convert[5]*b1)*4+0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);
          /* 2nd pixel */
          out = *((uint16*)((uint32)frameImgPtr + frame_index + 2));
          unpack_rgb565(out,&r2,&g2,&b2);
          luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                   ycbcr_convert[2]*b2)*4+0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                ycbcr_convert[8]*b2)*4+0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);
          *((uint8*)((uint32)outputImgPtr + dest_index)) = (uint8) cb;
          *((uint8*)((uint32)outputImgPtr + dest_index + 1)) = (uint8) luma1;
          *((uint8*)((uint32)outputImgPtr + dest_index + 2)) = (uint8) cr;
          *((uint8*)((uint32)outputImgPtr + dest_index + 3)) = (uint8) luma2;
        } else {
          out = *((unsigned short*)((uint32)inputImgPtr + src_index));
          unpack_rgb565(out,&r1,&g1,&b1);
          luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + 
                   ycbcr_convert[2]*b1)*4+0x8000;
          luma1 = (luma1>>16) + 16;
          luma1= CLIPIT(luma1);
          cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + 
                ycbcr_convert[5]*b1)*4+0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);
          /* 2nd pixel */
          out = *((unsigned short*)((uint32)inputImgPtr + 
                                    src_index+(colInc<<1)));
          unpack_rgb565(out,&r2,&g2,&b2);
          luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                   ycbcr_convert[2]*b2)*4+0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                ycbcr_convert[8]*b2)*4+0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);
          *((uint8*)((uint32)outputImgPtr + dest_index)) = (uint8) cb;
          *((uint8*)((uint32)outputImgPtr + dest_index + 1)) = (uint8) luma1;
          *((uint8*)((uint32)outputImgPtr + dest_index + 2)) = (uint8) cr;
          *((uint8*)((uint32)outputImgPtr + dest_index + 3)) = (uint8) luma2;
        }
        src_index += (colInc<<2); /* byte addressed */
        dest_index += 4;

        if (frameImgPtr)
          frame_index += 4;

      } /* End of col loop */
      src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);

      if (frameImgPtr)
        frame_index = frame_index + ((i_frame_ptr->dx - crop->dx) <<1 );
    } /* End of row loop */
    }
    else
    {
      for(row = 0; row < crop->dy; row++)
      {
        for(col = 0; col < (crop->dx); col=col+2)
        {
          out = *((unsigned short*)((uint32)inputImgPtr + src_index));
          unpack_rgb565(out,&r1,&g1,&b1);
          luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + 
                   ycbcr_convert[2]*b1)*4+0x8000;
          luma1 = (luma1>>16) + 16;
          luma1= CLIPIT(luma1);
          cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + 
                ycbcr_convert[5]*b1)*4+0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);
          /* 2nd pixel */
          out = *((unsigned short*)((uint32)inputImgPtr + 
                                    src_index+(colInc<<1)));
          unpack_rgb565(out,&r2,&g2,&b2);
          luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                   ycbcr_convert[2]*b2)*4+0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                ycbcr_convert[8]*b2)*4+0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);
          *((uint8*)((uint32)outputImgPtr + dest_index)) = (uint8) cb;
          *((uint8*)((uint32)outputImgPtr + dest_index + 1)) = (uint8) luma1;
          *((uint8*)((uint32)outputImgPtr + dest_index + 2)) = (uint8) cr;
          *((uint8*)((uint32)outputImgPtr + dest_index + 3)) = (uint8) luma2;

          src_index += (colInc<<2); /* byte addressed */
          dest_index += 4;
        }
        src_index = src_index + (rowInc<<1) - (colInc<<1);  /* byte addressed */
        dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
      } /* End of row loop */
    }
    MSG_LOW("image_rot_add_crop_rgbi marker_103\n");
    return IPL_SUCCESS;
  } 
  else 
  {
    MSG_LOW("image_rot_add_crop_rgbi marker_202\n");
    /* Only YCbCr or RGB565 output supported  */
    return IPL_FAILURE;
  }
} /* image_rot_add_crop_rgbi */



/* <EJECT> */
/*===========================================================================

FUNCTION image_rot_bayer_o

DESCRIPTION
  This function does a rotation of a bayer image.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image
  startPos is the start increment
  colInc is the col incr
  rowInc is the row incr

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type image_rot_bayer_o
( 
  ipl_image_type* i_img_ptr,    /* Input Image Pointer                     */
  ipl_image_type* o_img_ptr,   /* Output Image Pointer                     */
  int32 startPos,int32 colInc,int32 rowInc
)
{
  int32 src_index;
  uint32 row,col;
  ipl_image_type* output_img_ptr;
  register uint8* inputImgPtr;
  register uint8* outputImgPtr;
  register int32 rowIncr;
  register int32 colIncr;

  MSG_LOW("image_rot_bayer_o marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("image_rot_bayer_o marker_200\n");
    return IPL_FAILURE;
  }
  
  output_img_ptr = o_img_ptr;
  inputImgPtr=i_img_ptr->imgPtr;
  outputImgPtr = output_img_ptr->imgPtr;
  rowIncr = (rowInc) - (colInc);
  colIncr = colInc;

  MSG_LOW("image_rot_bayer_o marker_1\n");

  /* initialize the index to starting position */
  src_index = startPos;  /* byte addressed */
  inputImgPtr = (uint8*)((uint32)inputImgPtr + src_index);
  /* Now loop through the image once */
  for(row = output_img_ptr->dy; row; row--){
    for(col = output_img_ptr->dx; col; col--){
      *outputImgPtr = *inputImgPtr;
      outputImgPtr++;
      inputImgPtr = (uint8*)((uint32)inputImgPtr + colIncr);
    }
    inputImgPtr = (uint8*)((uint32)inputImgPtr + rowIncr);
  }

  MSG_LOW("image_rot_bayer_o marker_100\n");
  return IPL_SUCCESS;
} /* image_rot_bayer_o */




/* <EJECT> */
/*===========================================================================

FUNCTION image_crop_o

DESCRIPTION
  Optimized Crop Subroutine with no frame addition.

  The input should be in YCbCr.
  The output can be either YCbCr or RGB 565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr points to the input image
  o_img_ptr points to the output image
  crop is a structure informing ipl how to crop

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type image_crop_o
(
  ipl_image_type* i_img_ptr,        /* Points to the input image      */
  ipl_image_type* o_img_ptr,        /* Points to the output image     */
  ipl_rect_type* crop               /* Crop config                    */
)
{
  int32 src_index,rowInc=0,dest_index,destInc;
  uint32 row,col;
  ipl_image_type* input_img_ptr;
  ipl_image_type* output_img_ptr;
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr;
  register unsigned short out;
  register uint32 out32;
  register uint8 lumaa1;
  unsigned char cbb=0,crr=0,lumaa2=0;
  register int32 cb,cr;
  register long rc,gc,bc,r;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  //short ycbcr2rgb_convert[6] = {8, 25803, -3071, -7672,30399,12};
  short ycbcr2rgb_convert1 = 25803;
  short ycbcr2rgb_convert2 = -3071;
  short ycbcr2rgb_convert3 = -7672;
  short ycbcr2rgb_convert4 = 30399;
  short ycbcr2rgb_convert5 = 12;
  register uint32* data_out;

  MSG_LOW("image_crop_o marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr ||
      !crop)
  {
    MSG_LOW("image_crop_o marker_200\n");
    return IPL_FAILURE;
  }
  input_img_ptr = i_img_ptr;
  output_img_ptr = o_img_ptr;
  inputImgPtr=i_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  data_out = (uint32*)output_img_ptr->imgPtr;
 
  MSG_LOW("image_crop_o marker_1\n");

  /* initialize the index to starting position */
  src_index = 0;  /* byte addressed */
  dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
  rowInc = (input_img_ptr->dx - crop->dx )* 2;

  if ( output_img_ptr->cFormat == IPL_RGB565 )
  {
    outputImgPtr = (uint16*)((uint32)outputImgPtr +  dest_index);
    rowInc = (input_img_ptr->dx - crop->dx )* 2;
    destInc = ( output_img_ptr->dx - crop->dx) * 2;
    /* Now loop through the image once */
    for(row = crop->dy; row; row--){
      for(col = crop->dx/2; col; col--){
          /*
          ** Convert input to rgb
          */
          /* This is Cb */
          cb = (uint32)(*((uint8*)(inputImgPtr++)))-128;
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr++));
          /* Next byte is cr */
          cr = (uint32)(*((uint8*)(inputImgPtr++)))-128;
          rc = (8*cb + ycbcr2rgb_convert1*cr)*4+0x8000;
          rc=rc>>16;
          gc = (ycbcr2rgb_convert2*cb + ycbcr2rgb_convert3*cr)*4+0x8000;
          gc = gc>>16;
          bc = (ycbcr2rgb_convert4*cb + ycbcr2rgb_convert5*cr)*4+0x8000;
          bc = bc>>16;
          r = lumaa1 + rc;
          r = CLIPIT(r);
          out = r5xx[r];
          r = lumaa1 + gc;
          r = CLIPIT(r);
          out += gx6x[r];
          r = lumaa1 + bc;
          r = CLIPIT(r);
          out += bxx5[r];
          *outputImgPtr = out;
          outputImgPtr++;
          /* Next byte is luma of 2nd pixel */
          lumaa1 = *((uint8*)(inputImgPtr++));
          r = lumaa1 + rc;
          r = CLIPIT(r);
          out = r5xx[r];
          r = lumaa1 + gc;
          r = CLIPIT(r);
          out += gx6x[r];
          r = lumaa1 + bc;
          r = CLIPIT(r);
          out += bxx5[r];
          *outputImgPtr = out;
          outputImgPtr++;
      } /* End of col loop */
      inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc);
      outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
    } /* End of row loop */
    MSG_LOW("image_crop_o marker_100\n");
    return IPL_SUCCESS;
  } else if (output_img_ptr->cFormat == IPL_YCbCr)
  {
    /*
    ** No need to optimize this since it is
    ** not used in real time
    */
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
          /* This is Cb */
          cbb = *((uint8*)(inputImgPtr + src_index));
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr + src_index+1));
          /* Next byte is cr */
          crr = *((uint8*)(inputImgPtr + src_index+2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(inputImgPtr + src_index+3));
          *((uint8*)((uint32)outputImgPtr + dest_index)) = (unsigned char) cbb;
          *((uint8*)((uint32)outputImgPtr + dest_index + 1)) = (unsigned char) lumaa1;
          *((uint8*)((uint32)outputImgPtr + dest_index + 2)) = (unsigned char) crr;
          *((uint8*)((uint32)outputImgPtr + dest_index + 3)) = (unsigned char) lumaa2;
          src_index += 4; /* byte addressed */
          dest_index += 4;
      } /* End of col loop */
      src_index = src_index + rowInc;  /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
    } /* End of row loop */

    MSG_LOW("image_crop_o marker_101\n");
    return IPL_SUCCESS;
  } else if ( output_img_ptr->cFormat == IPL_RGB666 )
  {
    data_out = (uint32*)((uint32)data_out +  (dest_index<<1));
    rowInc = (input_img_ptr->dx - crop->dx )<<1;
    destInc = ( output_img_ptr->dx - crop->dx)<<2;
    /* Now loop through the image once */
    for(row = crop->dy; row; row--){
      for(col = crop->dx/2; col; col--){
          /*
          ** Convert input to rgb
          */
          /* This is Cb */
          cb = (uint32)(*((uint8*)(inputImgPtr++)))-128;
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr++));
          /* Next byte is cr */
          cr = (uint32)(*((uint8*)(inputImgPtr++)))-128;
          rc = (8*cb + ycbcr2rgb_convert1*cr)*4+0x8000;
          rc=rc>>16;
          gc = (ycbcr2rgb_convert2*cb + ycbcr2rgb_convert3*cr)*4+0x8000;
          gc = gc>>16;
          bc = (ycbcr2rgb_convert4*cb + ycbcr2rgb_convert5*cr)*4+0x8000;
          bc = bc>>16;
          r = lumaa1 + rc;
          r = CLIPIT(r);
          out32 = r666[r];
          r = lumaa1 + gc;
          r = CLIPIT(r);
          out32 += g666[r];
          r = lumaa1 + bc;
          r = CLIPIT(r);
          out32 += b666[r];
          *data_out = out32;
          data_out++;
          /* Next byte is luma of 2nd pixel */
          lumaa1 = *((uint8*)(inputImgPtr++));
          r = lumaa1 + rc;
          r = CLIPIT(r);
          out32 = r666[r];
          r = lumaa1 + gc;
          r = CLIPIT(r);
          out32 += g666[r];
          r = lumaa1 + bc;
          r = CLIPIT(r);
          out32 += b666[r];
          *data_out = out32;
          data_out++;
      } /* End of col loop */
      inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc);
      data_out = (uint32*)((uint32)data_out + destInc);
    } /* End of row loop */

    MSG_LOW("image_crop_o marker_102\n");
    return IPL_SUCCESS;
  } else if ( output_img_ptr->cFormat == IPL_RGB444 )
  {
    outputImgPtr = (uint16*)((uint32)outputImgPtr +  dest_index);
    rowInc = (input_img_ptr->dx - crop->dx )* 2;
    destInc = ( output_img_ptr->dx - crop->dx) * 2;
    /* Now loop through the image once */
    for(row = crop->dy; row; row--){
      for(col = crop->dx/2; col; col--){
          /*
          ** Convert input to rgb
          */
          /* This is Cb */
          cb = (uint32)(*((uint8*)(inputImgPtr++)))-128;
          /* Next Byte is luma of first pixel */
          lumaa1 = *((uint8*)(inputImgPtr++));
          /* Next byte is cr */
          cr = (uint32)(*((uint8*)(inputImgPtr++)))-128;
          rc = (8*cb + ycbcr2rgb_convert1*cr)*4+0x8000;
          rc=rc>>16;
          gc = (ycbcr2rgb_convert2*cb + ycbcr2rgb_convert3*cr)*4+0x8000;
          gc = gc>>16;
          bc = (ycbcr2rgb_convert4*cb + ycbcr2rgb_convert5*cr)*4+0x8000;
          bc = bc>>16;
          r = lumaa1 + rc;
          r = CLIPIT(r);
          out = r444[r];
          r = lumaa1 + gc;
          r = CLIPIT(r);
          out += g444[r];
          r = lumaa1 + bc;
          r = CLIPIT(r);
          out += b444[r];
          *outputImgPtr = out;
          outputImgPtr++;
          /* Next byte is luma of 2nd pixel */
          lumaa1 = *((uint8*)(inputImgPtr++));
          r = lumaa1 + rc;
          r = CLIPIT(r);
          out = r444[r];
          r = lumaa1 + gc;
          r = CLIPIT(r);
          out += g444[r];
          r = lumaa1 + bc;
          r = CLIPIT(r);
          out += b444[r];
          *outputImgPtr = out;
          outputImgPtr++;
      } /* End of col loop */
      inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc);
      outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
    } /* End of row loop */

    MSG_LOW("image_crop_o marker_103\n");
    return IPL_SUCCESS;
  } else {
    MSG_LOW("image_crop_o marker_201\n");
    /*
    ** Only RGB565, RGB666, RGB444 and ycbcr supported
    */
    return IPL_FAILURE;
  }
}  /* image_crop_o */


/* <EJECT> */
/*===========================================================================

FUNCTION image_crop_rgbi_o

DESCRIPTION
  Optimized Crop Subroutine with no frame addition.

  The input should be in RGB 565.
  The output can be either YCbCr or RGB 565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr points to the input image
  o_img_ptr points to the output image
  crop is a structure informing ipl how to crop

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type image_crop_rgbi_o
(
  ipl_image_type* i_img_ptr,        /* Points to the input image      */
  ipl_image_type* o_img_ptr,        /* Points to the output image     */
  ipl_rect_type* crop               /* Crop config                    */
)
{
  int32 src_index,rowInc,dest_index,destInc;
  uint32 row,col;
  ipl_image_type* input_img_ptr;
  ipl_image_type* output_img_ptr;
  register uint16* inputImgPtr;
  register uint16* outputImgPtr;
  register uint16 out;
  register uint16 out2;
  register uint32 out32;
  register uint32* data_out;
  register uint8 r;
  uint8 r1,g1,b1;
  int32 luma1,luma2,cb,cr;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("image_crop_rgbi_o marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr ||
      !crop)
  {
    MSG_LOW("image_crop_rgbi_o marker_200\n");
    return IPL_FAILURE;
  }
  input_img_ptr = i_img_ptr;
  output_img_ptr = o_img_ptr;
  inputImgPtr=(uint16*)i_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  data_out = (uint32*)output_img_ptr->imgPtr;

  MSG_LOW("image_crop_rgbi_o marker_1\n");

  if ( output_img_ptr->cFormat == IPL_RGB565 )
  {
    rowInc = (input_img_ptr->dx - crop->dx)<<1;
    destInc = (output_img_ptr->dx - crop->dx)<<1;
    src_index = 0;  /* byte addressed */
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
    outputImgPtr = (uint16*)((uint32)outputImgPtr + dest_index);
    /* Now loop through the image once */
    for(row = crop->dy; row; row--)
    {
      for(col = crop->dx; col; col--)
      {
        *outputImgPtr = *inputImgPtr;
        outputImgPtr++;
        inputImgPtr++;
      }
      outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
      inputImgPtr = (uint16*)((uint32)inputImgPtr + rowInc);
    }
    MSG_LOW("image_crop_rgbi_o marker_100\n");
    return IPL_SUCCESS;
  } else if (output_img_ptr->cFormat == IPL_RGB666)
  {
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<2;
    data_out = (uint32*)((uint32)data_out +  dest_index);
    rowInc = (input_img_ptr->dx - crop->dx )<<1;
    destInc = ( output_img_ptr->dx - crop->dx)<<2;
    /* Now loop through the image once */
    for(row = crop->dy; row; row--)
    {
      for(col = crop->dx; col; col--)
      {
        out = *(uint16*)(inputImgPtr);
        inputImgPtr++;
        r = (unsigned char)((out&0xF800)>>8);
        out32 = r666[r];
        r = (unsigned char)((out&0x07E0)>>3);
        out32 += g666[r];
        r = (unsigned char)((out&0x001F)<<3);
        out32 += b666[r];
        *data_out = out32;
        data_out++;
      } /* End of col loop */
      inputImgPtr = (uint16*)((uint32)inputImgPtr + rowInc);
      data_out = (uint32*)((uint32)data_out + destInc);
    } /* End of row loop */
    MSG_LOW("image_crop_rgbi_o marker_101\n");
    return IPL_SUCCESS;
  } 
  else if (output_img_ptr->cFormat == IPL_RGB444)
  {
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
    outputImgPtr = (uint16*)((uint32)outputImgPtr +  (dest_index));
    rowInc = (input_img_ptr->dx - crop->dx )<<1;
    destInc = ( output_img_ptr->dx - crop->dx)<<1;
    /* Now loop through the image once */
    for(row = crop->dy; row; row--){
      for(col = crop->dx; col; col--){
        /*
        ** Convert input from rgb565 to rgb444
        */
        out = *(uint16*)(inputImgPtr);
        inputImgPtr++;
        r = (unsigned char)((out&0xF800)>>8);
        out2 = r444[r];
        r = (unsigned char)((out&0x07E0)>>3);
        out2 += g444[r];
        r = (unsigned char)((out&0x001F)<<3);
        out2 += b444[r];
        *outputImgPtr = out2;
        outputImgPtr++;
      } /* End of col loop */
      inputImgPtr = (uint16*)((uint32)inputImgPtr + rowInc);
      data_out = (uint32*)((uint32)data_out + destInc);
    } /* End of row loop */
    MSG_LOW("image_crop_rgbi_o marker_102\n");
    return IPL_SUCCESS;
  } else if (output_img_ptr->cFormat == IPL_YCbCr)
  {
    rowInc = (input_img_ptr->dx - crop->dx)<<1;
    destInc = (output_img_ptr->dx - crop->dx)<<1;
    src_index = 0;  /* byte addressed */
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
    /* Now loop through the image once */
    for(row = 0; row < crop->dy; row++){
      for(col = 0; col < (crop->dx); col=col+2){
          out = *((uint16*)((uint32)inputImgPtr + src_index));
          unpack_rgb565(out,&r1,&g1,&b1);
          luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + 
                   ycbcr_convert[2]*b1)*4+0x8000;
          luma1 = (luma1>>16) + 16;
          luma1=CLIPIT(luma1);
          cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + 
                ycbcr_convert[5]*b1)*4+0x8000;
          cb = (cb>>16) + 128;
          cb=CLIPIT(cb);
          /* 2nd pixel */
          out = *((unsigned short*)((uint32)inputImgPtr + src_index+2));
          unpack_rgb565(out,&r1,&g1,&b1);
          luma2 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + 
                   ycbcr_convert[2]*b1)*4+0x8000;
          luma2 = (luma2>>16) + 16;
          luma2=CLIPIT(luma2);
          cr = (ycbcr_convert[6]*r1 + ycbcr_convert[7]*g1 + 
                ycbcr_convert[8]*b1)*4+0x8000;
          cr = (cr>>16) + 128;
          cr=CLIPIT(cr);
          *((uint8*)((uint32)outputImgPtr + dest_index)) = (uint8)cb;
          *((uint8*)((uint32)outputImgPtr + dest_index + 1)) = (uint8)luma1;
          *((uint8*)((uint32)outputImgPtr + dest_index + 2)) = (uint8)cr;
          *((uint8*)((uint32)outputImgPtr + dest_index + 3)) = (uint8)luma2;
          src_index += 4; /* byte addressed */
          dest_index += 4;
      } /* End of col loop */
      src_index = src_index + rowInc;  /* byte addressed */
      dest_index = dest_index + ( (output_img_ptr->dx - crop->dx) << 1);
    } /* End of row loop */
    MSG_LOW("image_crop_rgbi_o marker_103\n");
    return IPL_SUCCESS;
  } else {
    MSG_LOW("image_crop_rgbi_o marker_201\n");
    /* Only YCbCr or RGB565 output supported  */
    return IPL_FAILURE;
  }
} /* image_crop_rgbi_o */







/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_downsize_rot

DESCRIPTION
  This function performs cropping on input, downsizing, and then clockwise 
  rotation of an image.

  The input must be YCxCx420 or YCxCx422 line pack;

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         pointer to the input image
  o_img_ptr         pointer to the output image
  crop              region on input to be cropped
  rotate            type of rotation

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_crop_downsize_rot
(
  ipl_image_type* i_img_ptr,        /* Points to the input image        */
  ipl_image_type* o_img_ptr,        /* Points to the output image       */
  ipl_rect_type* crop,              /* Crop config                      */
  ipl_rotate90_type rotate          /* Rotatation                       */
)
{
  ipl_rect_type inCrop;

  MSG_LOW("ipl_crop_downsize_rot marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_downsize_rot marker_200\n");
    return IPL_FAILURE;
  }
  
  if (crop == NULL)
  {
    inCrop.x = 0;
    inCrop.y = 0;
    inCrop.dx = i_img_ptr->dx; 
    inCrop.dy = i_img_ptr->dy; 
  }
  else
  {
    inCrop.x = crop->x; 
    inCrop.y = crop->y; 
    inCrop.dx = crop->dx; 
    inCrop.dy = crop->dy; 
  }

  if ((inCrop.x + inCrop.dx > i_img_ptr->dx) ||
      (inCrop.y + inCrop.dy > i_img_ptr->dy))
  {
    MSG_LOW("ipl_crop_downsize_rot marker_201\n");
    return IPL_FAILURE;
  }
  // strict downsize
  if (rotate == IPL_NOROT || rotate == IPL_ROT180)
  {
    if ((inCrop.dx < o_img_ptr->dx) || 
        (inCrop.dy < o_img_ptr->dy))
    {
      MSG_LOW("ipl_crop_downsize_rot marker_202\n");
      return IPL_FAILURE;
    }
  }
  else
  {
    if ((inCrop.dx < o_img_ptr->dy) || 
        (inCrop.dy < o_img_ptr->dx))
    {
      MSG_LOW("ipl_crop_downsize_rot marker_203\n");
      return IPL_FAILURE;
    }
  }


  if ((i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (i_img_ptr->cFormat == IPL_YCrCb422_LINE_PK))
  {
    MSG_LOW("ipl_crop_downsize_rot marker_100\n");
    return(ipl_crop_downsize_rot_ycbcr(i_img_ptr, o_img_ptr, &inCrop, rotate));
  }
  else
  {
    MSG_LOW("ipl_crop_downsize_rot marker_204\n");
    return IPL_FAILURE;
  }
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_rot_add_crop

DESCRIPTION
  This function performs real-time clockwise rotation and addition of a frame. 
  Four types of rotations are supported: 0 degrees, 90 degrees, 180 degrees,
  and 270 degrees. Icons are processed only if both input and output are  
  RGB565.

  The frame must be in RGB565 and must have the same dimensions as the output.
  The input can be either YCbCr 4:2:2 or 420 line pack, 422 line pack,RGB565.
  The output can be either YCbCr 4:2:2 or RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         pointer to the input image
  i_frame_ptr       pointer to the frame image
  o_img_ptr         pointer to the output image
  icrop             region of input to be cropped (before rotation)
  ocrop             output region to be cropped
  rotate            type of rotation
  icon_list_ptr     pointer to a NULL-terminated list of icons
  transparentValue  16-bit transparent pixel value

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_crop_rot_add_crop
(
  ipl_image_type* i_img_ptr,        /* Points to the input image        */
  ipl_image_type* i_frame_ptr,      /* Points to the frame              */
  ipl_image_type* o_img_ptr,        /* Points to the output image       */
  ipl_rect_type* icrop,             /* Crop input                       */
  ipl_rect_type* ocrop,             /* Crop config                      */
  ipl_rotate90_type rotate,         /* Rotatation                       */
  ipl_icon_type** icon_list_ptr,    /* Ptr to null terminated icon list */
  uint16 transparentValue           /* Transparent pixel value          */
)
{
  MSG_LOW("ipl_crop_rot_add_crop marker_0\n");


  // if we have  crop on the input and we are line pack, call these special
  // routines, otherwise, go to good ol trusted ipl_rot_add_crop
  if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK && icrop != NULL)
  {
    // we need to crop the input
    if (rotate == IPL_NOROT)
    {
      MSG_LOW("ipl_crop_rot_add_crop marker_001\n");
      return(ipl2_Rot000Frame_CropYCrCb420lpToRGB(i_img_ptr, i_frame_ptr, transparentValue, o_img_ptr, icrop, ocrop));
    }
    else if (rotate == IPL_ROT90)
    {
      MSG_LOW("ipl_crop_rot_add_crop marker_002\n");
      return(ipl2_Rot090Frame_CropYCrCb420lpToRGB(i_img_ptr, i_frame_ptr, transparentValue, o_img_ptr, icrop, ocrop));
    }
    else if (rotate == IPL_ROT180)
    {
      MSG_LOW("ipl_crop_rot_add_crop marker_003\n");
      return(ipl2_Rot180Frame_CropYCrCb420lpToRGB(i_img_ptr, i_frame_ptr, transparentValue, o_img_ptr, icrop, ocrop));
    }
    else if (rotate == IPL_ROT270)
    {
      MSG_LOW("ipl_crop_rot_add_crop marker_004\n");
      return(ipl2_Rot270Frame_CropYCrCb420lpToRGB(i_img_ptr, i_frame_ptr, transparentValue, o_img_ptr, icrop, ocrop));
    }
    else
    {
      MSG_LOW("ipl_crop_rot_add_crop marker_005\n");
      return (ipl_rot_add_crop(i_img_ptr, i_frame_ptr, o_img_ptr, ocrop, rotate, icon_list_ptr, transparentValue));
    }
  }
  else
  {
    // cropping on input not supported, call old ipl_rot_add_crop
      MSG_LOW("ipl_crop_rot_add_crop marker_004\n");
    return (ipl_rot_add_crop(i_img_ptr, i_frame_ptr, o_img_ptr, ocrop, rotate, icon_list_ptr, transparentValue));
  }

  //MSG_LOW("ipl_crop_rot_add_crop marker_100\n");
  //return (IPL_SUCCESS); //lint !e527
  // don't uncomment, cause 7K compiler gives warning and bm gets mad
}








/* <EJECT> */
/*===========================================================================

FUNCTION ipl_rot_add_crop

DESCRIPTION
  This function performs real-time clockwise rotation and addition of a frame. 
  Four types of rotations are supported: 0 degrees, 90 degrees, 180 degrees,
  and 270 degrees. Icons are processed only if both input and output are  
  RGB565.

  The frame must be in RGB565 and must have the same dimensions as the output.
  The input can be either YCbCr 4:2:2 or RGB565.
  The output can be either YCbCr 4:2:2 or RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         pointer to the input image
  i_frame_ptr       pointer to the frame image
  o_img_ptr         pointer to the output image
  crop              region to be cropped
  rotate            type of rotation
  icon_list_ptr     pointer to a NULL-terminated list of icons
  transparentValue  16-bit transparent pixel value

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_rot_add_crop
(
  ipl_image_type* i_img_ptr,        /* Points to the input image        */
  ipl_image_type* i_frame_ptr,      /* Points to the frame              */
  ipl_image_type* o_img_ptr,        /* Points to the output image       */
  ipl_rect_type* icrop,              /* Crop config                      */
  ipl_rotate90_type rotate,         /* Rotatation                       */
  ipl_icon_type** icon_list_ptr,    /* Ptr to null terminated icon list */
  uint16 transparentValue           /* Transparent pixel value          */
)
{
  int32 src_index,rowInc=0,colInc=0,startPos=0,dest_index,destInc;
  int32 frame_index;
  uint32 row,col;
  ipl_image_type* input_img_ptr;
  ipl_image_type* input_frame_ptr;
  ipl_image_type* output_img_ptr;
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr;
  register uint16* frameImgPtr = NULL;
  unsigned char r1,g1,b1,r2,g2,b2;
  register uint16 out,out2;
  register uint32 out32;
  register int32 luma1,luma2,cb,cr;
  register unsigned char cbb=0,crr=0,lumaa1=0,lumaa2=0;
  register long rc,gc,bc,r;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  short ycbcr2rgb_convert1 = 25803;
  short ycbcr2rgb_convert2 = -3071;
  short ycbcr2rgb_convert3 = -7672;
  short ycbcr2rgb_convert4 = 30399;
  short ycbcr2rgb_convert5 = 12;
  register uint32* data_out;
  ipl_rect_type* crop;
  ipl_rect_type tCrop;

  //boolean process_icons = FALSE;
  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_rot_add_crop marker_0\n");

  if (icrop == NULL)
  {
    tCrop.x = 0;
    tCrop.y = 0;
    tCrop.dx = o_img_ptr->dx;
    tCrop.dy = o_img_ptr->dy;
    crop = &tCrop;
  }
  else
  {
    crop = icrop;
  }


  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_rot_add_crop marker_200\n");
    return IPL_FAILURE;
  }
  input_img_ptr = i_img_ptr;
  input_frame_ptr = i_frame_ptr;
  
  if (input_frame_ptr != NULL)
    frameImgPtr = (uint16*)(input_frame_ptr->imgPtr);
  
  output_img_ptr = o_img_ptr;
  inputImgPtr = i_img_ptr->imgPtr;
  outputImgPtr =(uint16*) output_img_ptr->imgPtr;
  data_out = (uint32*)output_img_ptr->imgPtr;

  input_img_ptr = i_img_ptr;

  if ( (input_frame_ptr != NULL) && (input_frame_ptr->cFormat != IPL_RGB565) )
  {
    MSG_LOW("ipl_rot_add_crop marker_201\n");
    /* Only RGB Frame format currently supported */
    return IPL_FAILURE;
  }

  /*
  ** Icons are only supported in RGB565 input and RGB565 output
  */
  if (icon_list_ptr && !((i_img_ptr->cFormat == IPL_RGB565) &&
      (o_img_ptr->cFormat == IPL_RGB565)))
  {
    MSG_LOW("ipl_rot_add_crop marker_202\n");
    return IPL_FAILURE;
  }

  /*
  ** Make sure cropping parameters are ok
  */
  if ((crop->x+crop->dx)> output_img_ptr->dx ||
      (crop->y + crop->dy) > output_img_ptr->dy)
  {
    MSG_LOW("ipl_rot_add_crop marker_203\n");
    /* Invalid dimensions set */
    return IPL_FAILURE;
  }

  if ((input_frame_ptr != NULL) &&
      (rotate == IPL_ROT90      || rotate == IPL_ROT270 ||
       rotate == IPL_ROT90_XREF || rotate == IPL_ROT270_XREF ||
       rotate == IPL_ROT90_YREF || rotate == IPL_ROT270_YREF) &&
      ((input_frame_ptr->dx != input_img_ptr->dy) || 
      (input_frame_ptr->dy != input_img_ptr->dx)))
  {
    MSG_LOW("ipl_rot_add_crop marker_204\n");
    /* Frame should be the same size as input b/4 cropping */
    return IPL_FAILURE;
  }

  if ((input_frame_ptr != NULL) &&
      (rotate == IPL_ROT180      || rotate == IPL_NOROT ||
       rotate == IPL_ROT180_XREF || rotate == IPL_ROT180_YREF) &&
      ((input_frame_ptr->dx != input_img_ptr->dx) || 
       (input_frame_ptr->dy != input_img_ptr->dy)))
  {
    MSG_LOW("ipl_rot_add_crop marker_205\n");
    /* Frame should be the same size as input b/4 cropping */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_rot_add_crop marker_1\n");

  /*
  ** input in YCbCr
  */
  if ( input_img_ptr->cFormat == IPL_YCbCr )
  {
    /*
    ** Input is in YCbCr format
    */
    /*
    ** First carry out the rotation
    */
    if (rotate == IPL_NOROT)
    {
      if (input_img_ptr->dx < crop->dx)
      {
        MSG_LOW("ipl_rot_add_crop marker_206\n");
        return IPL_FAILURE;
      }
      /*
      ** Dont rotate
      ** Call a more optimized version
      */
      if (input_frame_ptr == NULL)
      {
        if(image_crop_o(input_img_ptr, output_img_ptr,crop) != IPL_SUCCESS) 
        {
          MSG_LOW("ipl_rot_add_crop marker_207\n");
          return IPL_FAILURE;
        } 
        else 
        {
          MSG_LOW("ipl_rot_add_crop marker_100\n");
          return IPL_SUCCESS;
        }
      } 
      else 
      {
        startPos = 0;
        colInc = 1;
        rowInc = 1 + input_img_ptr->dx - crop->dx;
      }
    } 
    else if (rotate == IPL_ROT90)
    {
      /* Rotate 90 clockwise. */
      startPos = input_img_ptr->dx*(input_img_ptr->dy-1);
      colInc = -(int32)input_img_ptr->dx;
      rowInc = (input_img_ptr->dx)*(input_img_ptr->dy-1)+1 -
        (input_img_ptr->dy - crop->dx)*input_img_ptr->dx;

      if (input_img_ptr->dy < crop->dx)
      {
        MSG_LOW("ipl_rot_add_crop marker_208\n");
        return IPL_FAILURE;
      }
      if (image_swap_row_cb_cr(input_img_ptr, input_frame_ptr, 
                                transparentValue, output_img_ptr,
                                startPos,colInc,rowInc,crop) != IPL_SUCCESS) 
      {
        MSG_LOW("ipl_rot_add_crop marker_209\n");
        return IPL_FAILURE;
      } 
      else 
      {
        MSG_LOW("ipl_rot_add_crop marker_101\n");
        return IPL_SUCCESS;
      }
    } 
    else if (rotate==IPL_ROT180 )
    {
      /* Rotate 180 clockwise.  Dont reflect */
      startPos = (input_img_ptr->dx*input_img_ptr->dy)-1;
      colInc = -1;
      rowInc = -((int32) input_img_ptr->dx - (int32) crop->dx) - 1;

      if (input_img_ptr->dx < crop->dx)
      {
        MSG_LOW("ipl_rot_add_crop marker_210\n");
        return IPL_FAILURE;
      }
      if (image_swap_cb_cr(input_img_ptr, input_frame_ptr, transparentValue,
        output_img_ptr,startPos,colInc,rowInc,crop) != IPL_SUCCESS) 
      {
        MSG_LOW("ipl_rot_add_crop marker_211\n");
        return IPL_FAILURE;
      } 
      else 
      {
        MSG_LOW("ipl_rot_add_crop marker_102\n");
        return IPL_SUCCESS;
      }
    } 
    else if ( rotate==IPL_ROT270 )
    {
      /* Rotate 270 clockwise.  Dont reflect */
      startPos = input_img_ptr->dx-1;
      colInc = input_img_ptr->dx;
      rowInc = -(int32)(input_img_ptr->dx)*(input_img_ptr->dy-1) -1 +
               (input_img_ptr->dy - crop->dx)*input_img_ptr->dx;

      if (input_img_ptr->dy < crop->dx)
      {
        MSG_LOW("ipl_rot_add_crop marker_212\n");
        return IPL_FAILURE;
      }
      if (image_swap_row_cr_cb(input_img_ptr, input_frame_ptr, 
                               transparentValue, output_img_ptr,startPos,
                               colInc,rowInc,crop) != IPL_SUCCESS) 
      {
        MSG_LOW("ipl_rot_add_crop marker_213\n");
        return IPL_FAILURE;
      } 
      else 
      {
        MSG_LOW("ipl_rot_add_crop marker_103\n");
        return IPL_SUCCESS;
      }
    } 
    else if (rotate == IPL_NOROT_YREF || 
             rotate == IPL_ROT180_XREF)
    {
      if (input_img_ptr->dx < crop->dx)
      {
        MSG_LOW("ipl_rot_add_crop marker_214\n");
        return IPL_FAILURE;
      }

      /*
      ** Dont rotate
      ** Call a more optimized version
      */
      if (input_frame_ptr == NULL)
      {
        /* Rotate 180 clockwise.  Dont reflect */
        startPos = input_img_ptr->dx-1;
        colInc = -1;
        rowInc = crop->dx + input_img_ptr->dx - 1;

        if (image_swap_cb_cr(input_img_ptr, input_frame_ptr, transparentValue,
        output_img_ptr,startPos,colInc,rowInc,crop) != IPL_SUCCESS) 
        {
          MSG_LOW("ipl_rot_add_crop marker_215\n");
          return IPL_FAILURE;
        } 
        else 
        {
          MSG_LOW("ipl_rot_add_crop marker_104\n");
          return IPL_SUCCESS;
        }
      } 
      else 
      {
        startPos = 0;
        colInc = 1;
        rowInc = 1 + input_img_ptr->dx - crop->dx;
      }
    } 
    else if (rotate == IPL_NOROT_XREF ||
             rotate == IPL_ROT180_YREF)
    {
      if (input_img_ptr->dx < crop->dx)
      {
        MSG_LOW("ipl_rot_add_crop marker_216\n");
        return IPL_FAILURE;
      }
      /*
      ** Dont rotate
      ** Call a more optimized version
      */
      if (input_frame_ptr == NULL)
      {
        /* Rotate 180 clockwise.  Dont reflect */
        startPos = input_img_ptr->dx*(input_img_ptr->dy)-1;
        colInc = 1;
        //rowInc = -2*input_img_ptr->dx + 1;
        rowInc = -((int32)crop->dx) - ((int32)input_img_ptr->dx) + 1;

        if (image_swap_cb_cr(input_img_ptr, input_frame_ptr, transparentValue,
        output_img_ptr,startPos,colInc,rowInc,crop) != IPL_SUCCESS) 
        {
          MSG_LOW("ipl_rot_add_crop marker_217\n");
          return IPL_FAILURE;
        } 
        else 
        {
          MSG_LOW("ipl_rot_add_crop marker_105\n");
          return IPL_SUCCESS;
        }
      } 
      else 
      {
        startPos = 0;
        colInc = 1;
        rowInc = 1 + input_img_ptr->dx - crop->dx;
      }
    } 
    else if (rotate == IPL_ROT90_XREF ||
             rotate == IPL_ROT270_YREF)
    {
      /* Rotate 90 clockwise. */
      startPos = input_img_ptr->dx*(input_img_ptr->dy)-1;
      colInc = -1 * (int32) input_img_ptr->dx;
      rowInc = ((int32) input_img_ptr->dx)*((int32)crop->dx-1)-1;

      if (input_img_ptr->dy < crop->dx)
      {
        MSG_LOW("ipl_rot_add_crop marker_218\n");
        return IPL_FAILURE;
      }

      if (image_swap_row_cr_cb(input_img_ptr, input_frame_ptr, 
                                transparentValue, output_img_ptr,
                                startPos,colInc,rowInc,crop) != IPL_SUCCESS) 
      {
        MSG_LOW("ipl_rot_add_crop marker_219\n");
        return IPL_FAILURE;
      } 
      else 
      {
        MSG_LOW("ipl_rot_add_crop marker_106\n");
        return IPL_SUCCESS;
      }
    } 
    else if (rotate == IPL_ROT90_YREF ||
             rotate == IPL_ROT270_XREF)
    {
      /* Rotate 90 clockwise. */
      startPos = 0;
      colInc = (int32) input_img_ptr->dx;
      rowInc = -((int32)input_img_ptr->dx)*((int32)crop->dx-1)+1;

      if (input_img_ptr->dy < crop->dx)
      {
        MSG_LOW("ipl_rot_add_crop marker_220\n");
        return IPL_FAILURE;
      }
      if (image_swap_row_cb_cr(input_img_ptr, input_frame_ptr, 
                                transparentValue, output_img_ptr,
                                startPos,colInc,rowInc,crop) != IPL_SUCCESS) 
      {
        MSG_LOW("ipl_rot_add_crop marker_221\n");
        return IPL_FAILURE;
      } 
      else 
      {
        MSG_LOW("ipl_rot_add_crop marker_107\n");
        return IPL_SUCCESS;
      }
    } 
    else
    {
      MSG_LOW("ipl_rot_add_crop marker_222\n");
      return IPL_FAILURE;
    }


  //  MSG_LOW("ipl_rot_add_crop marker_2\n");

    /*
    ** Enter here for no rotation but framing
    */

    if (frameImgPtr == NULL)
    {
      MSG_LOW("ipl_rot_add_crop marker_223\n");
      return IPL_FAILURE;
    }
    /* initialize the index to starting position */
    dest_index = (crop->x + output_img_ptr->dx*crop->y) <<1;
    rowInc = (input_img_ptr->dx - crop->dx )* 2;

    if ( output_img_ptr->cFormat == IPL_RGB565 )
    {
      outputImgPtr = (uint16*)((uint32)outputImgPtr +  dest_index);
      rowInc = (input_img_ptr->dx - crop->dx )* 2;
      destInc = ( output_img_ptr->dx - crop->dx) * 2;
      /* Now loop through the image once */
      for(row = crop->dy; row; row--){
        for(col = crop->dx/2; col; col--){
          if (*(uint16*)(frameImgPtr) != transparentValue) {
            /*
            ** Copy Over the Frame
            */
            *outputImgPtr = *frameImgPtr;
            outputImgPtr++;
            frameImgPtr++;
            *outputImgPtr = *frameImgPtr;
            outputImgPtr++;
            frameImgPtr++;
            inputImgPtr+=4;
          } else {
            /*
            ** Copy Over the Image
            */
            frameImgPtr += 2;
            /*
            ** Convert input to rgb
            */
            /* This is Cb */
            cb = (uint32)(*((uint8*)(inputImgPtr++)))-128;
            /* Next Byte is luma of first pixel */
            lumaa1 = *((uint8*)(inputImgPtr++));
            /* Next byte is cr */
            cr = (uint32)(*((uint8*)(inputImgPtr++)))-128;
            rc = (8*cb + ycbcr2rgb_convert1*cr)*4+0x8000;
            rc=rc>>16;
            gc = (ycbcr2rgb_convert2*cb + ycbcr2rgb_convert3*cr)*4+0x8000;
            gc = gc>>16;
            bc = (ycbcr2rgb_convert4*cb + ycbcr2rgb_convert5*cr)*4+0x8000;
            bc = bc>>16;
            r = lumaa1 + rc;
            r = CLIPIT(r);
            out = r5xx[r];
            r = lumaa1 + gc;
            r = CLIPIT(r);
            out += gx6x[r];
            r = lumaa1 + bc;
            r = CLIPIT(r);
            out += bxx5[r];
            *outputImgPtr = out;
            outputImgPtr++;
            /* Next byte is luma of 2nd pixel */
            lumaa1 = *((uint8*)(inputImgPtr++));
            r = lumaa1 + rc;
            r = CLIPIT(r);
            out = r5xx[r];
            r = lumaa1 + gc;
            r = CLIPIT(r);
            out += gx6x[r];
            r = lumaa1 + bc;
            r = CLIPIT(r);
            out += bxx5[r];
            *outputImgPtr = out;
            outputImgPtr++;
          }
        } /* End of col loop */
        inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc);
        frameImgPtr = (uint16*)((uint32)frameImgPtr + rowInc);
        outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
      } /* End of row loop */

      MSG_LOW("ipl_rot_add_crop marker_108\n");
      return IPL_SUCCESS;
    } else if (output_img_ptr->cFormat == IPL_YCbCr)
    {
      /*
      ** No need to optimize this since it is
      ** not used in real time
      */
      frame_index = 0;
      src_index = 0;
      rowInc = (input_img_ptr->dx - crop->dx )* 2;
      destInc = ( output_img_ptr->dx - crop->dx) * 2;
      /* Now loop through the image once */
      for(row = 0; row < crop->dy; row++){
        for(col = 0; col < (crop->dx); col=col+2){
          if((input_frame_ptr != NULL) && 
             ((*(uint16*)((uint32)frameImgPtr+frame_index))!=transparentValue))
          {
            /* Use frame */
            /* Convert frame to YCbCr */
            out = *((uint16*)((uint32)frameImgPtr + frame_index));
            unpack_rgb565(out,&r1,&g1,&b1);
            luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + 
                     ycbcr_convert[2]*b1)*4+0x8000;
            luma1 = (luma1>>16) + 16;
            luma1= CLIPIT(luma1);
            cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + 
                  ycbcr_convert[5]*b1)*4+0x8000;
            cb = (cb>>16) + 128;
            cb = CLIPIT(cb);
            /* 2nd pixel */
            out = *((uint16*)((uint32)frameImgPtr + frame_index + 2));
            unpack_rgb565(out,&r2,&g2,&b2);
            luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                     ycbcr_convert[2]*b2)*4+0x8000;
            luma2 = (luma2>>16) + 16;
            luma2 = CLIPIT(luma2);
            cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                  ycbcr_convert[8]*b2)*4+0x8000;
            cr = (cr>>16) + 128;
            cr = CLIPIT(cr);
            *((uint8*)((uint32)outputImgPtr + dest_index)) = (uint8) cb;
            *((uint8*)((uint32)outputImgPtr + dest_index + 1)) = (uint8) luma1;
            *((uint8*)((uint32)outputImgPtr + dest_index + 2)) = (uint8) cr;
            *((uint8*)((uint32)outputImgPtr + dest_index + 3)) = (uint8) luma2;
          } else {
            /* This is Cb */
            cbb = *((uint8*)((uint32)inputImgPtr + src_index));
            /* Next Byte is luma of first pixel */
            lumaa1 = *((uint8*)((uint32)inputImgPtr + src_index+1));
            /* Next byte is cr */
            crr = *((uint8*)((uint32)inputImgPtr + src_index+2));
            /* Next byte is luma of 2nd pixel */
            lumaa2 = *((uint8*)((uint32)inputImgPtr + src_index+3));
            *((uint8*)((uint32)outputImgPtr + dest_index)) = (unsigned char) cbb;
            *((uint8*)((uint32)outputImgPtr + dest_index + 1)) = (unsigned char) lumaa1;
            *((uint8*)((uint32)outputImgPtr + dest_index + 2)) = (unsigned char) crr;
            *((uint8*)((uint32)outputImgPtr + dest_index + 3)) = (unsigned char) lumaa2;
            //*((uint16*)(outputImgPtr + dest_index))=0;
          }
          src_index += 4; /* byte addressed */
          dest_index += 4;
          frame_index += 4;
        } /* End of col loop */
        src_index = src_index + rowInc;  /* byte addressed */
        frame_index = frame_index + rowInc;  /* byte addressed */
        dest_index = dest_index + destInc;
      } /* End of row loop */

      MSG_LOW("ipl_rot_add_crop marker_109\n");
      return IPL_SUCCESS;
    } else if ( output_img_ptr->cFormat == IPL_RGB666 )
    {
      data_out = (uint32*)((uint32)data_out +  (dest_index<<1));
      rowInc = (input_img_ptr->dx - crop->dx )<<1;
      destInc = ( output_img_ptr->dx - crop->dx)<<2;
      /* Now loop through the image once */
      for(row = crop->dy; row; row--){
        for(col = crop->dx/2; col; col--){
          if (*(uint16*)(frameImgPtr) != transparentValue) {
            /*
            ** Copy Over the Frame and convert from 565 to 666
            */
            out = *(uint16*)(frameImgPtr);
            r1 = (unsigned char)((out&0xF800)>>8);
            out32 = r666[r1];
            r1 = (unsigned char)((out&0x07E0)>>3);
            out32 += g666[r1];
            r1 = (unsigned char)((out&0x001F)<<3);
            out32 += b666[r1];
            *data_out = out32;
            data_out++;
            frameImgPtr++;
            out = *(uint16*)(frameImgPtr);
            r1 = (unsigned char)((out&0xF800)>>8);
            out32 = r666[r1];
            r1 = (unsigned char)((out&0x07E0)>>3);
            out32 += g666[r1];
            r1 = (unsigned char)((out&0x001F)<<3);
            out32 += b666[r1];
            *data_out = out32;
            data_out++;
            frameImgPtr++;
            inputImgPtr+=4;
          } else {
            /*
            ** Copy Over the Image
            */
            frameImgPtr += 2;
            /*
            ** Convert input to rgb
            */
            /* This is Cb */
            cb = (uint32)(*((uint8*)(inputImgPtr++)))-128;
            /* Next Byte is luma of first pixel */
            lumaa1 = *((uint8*)(inputImgPtr++));
            /* Next byte is cr */
            cr = (uint32)(*((uint8*)(inputImgPtr++)))-128;
            rc = (8*cb + ycbcr2rgb_convert1*cr)*4+0x8000;
            rc=rc>>16;
            gc = (ycbcr2rgb_convert2*cb + ycbcr2rgb_convert3*cr)*4+0x8000;
            gc = gc>>16;
            bc = (ycbcr2rgb_convert4*cb + ycbcr2rgb_convert5*cr)*4+0x8000;
            bc = bc>>16;
            r = lumaa1 + rc;
            r = CLIPIT(r);
            out32 = r666[r];
            r = lumaa1 + gc;
            r = CLIPIT(r);
            out32 += g666[r];
            r = lumaa1 + bc;
            r = CLIPIT(r);
            out32 += b666[r];
            *data_out = out32;
            data_out++;
            /* Next byte is luma of 2nd pixel */
            lumaa1 = *((uint8*)(inputImgPtr++));
            r = lumaa1 + rc;
            r = CLIPIT(r);
            out32 = r666[r];
            r = lumaa1 + gc;
            r = CLIPIT(r);
            out32 += g666[r];
            r = lumaa1 + bc;
            r = CLIPIT(r);
            out32 += b666[r];
            *data_out = out32;
            data_out++;
          }
        } /* End of col loop */
        inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc);
        frameImgPtr = (uint16*)((uint32)frameImgPtr + rowInc);
        data_out = (uint32*)((uint32)data_out + destInc);
      } /* End of row loop */

      MSG_LOW("ipl_rot_add_crop marker_110\n");
      return IPL_SUCCESS;
    } else if ( output_img_ptr->cFormat == IPL_RGB444 )
    {
      outputImgPtr = (uint16*)((uint32)outputImgPtr +  dest_index);
      rowInc = (input_img_ptr->dx - crop->dx )* 2;
      destInc = ( output_img_ptr->dx - crop->dx) * 2;
      /* Now loop through the image once */
      for(row = crop->dy; row; row--){
        for(col = crop->dx/2; col; col--){
          if (*(uint16*)(frameImgPtr) != transparentValue) {
            /*
            ** Copy Over the Frame and convert from 565 to 444
            */
            out = *(uint16*)(frameImgPtr);
            r1 = (unsigned char)((out&0xF800)>>8);
            out2 = r444[r1];
            r1 = (unsigned char)((out&0x07E0)>>3);
            out2 += g444[r1];
            r1 = (unsigned char)((out&0x001F)<<3);
            out2 += b444[r1];
            *outputImgPtr = out2;
            outputImgPtr++;
            frameImgPtr++;
            out = *(uint16*)(frameImgPtr);
            r1 = (unsigned char)((out&0xF800)>>8);
            out2 = r444[r1];
            r1 = (unsigned char)((out&0x07E0)>>3);
            out2 += g444[r1];
            r1 = (unsigned char)((out&0x001F)<<3);
            out2 += b444[r1];
            *outputImgPtr = out2;
            outputImgPtr++;
            frameImgPtr++;
            inputImgPtr+=4;
          } else {
            /*
            ** Copy Over the Image
            */
            frameImgPtr += 2;
            /*
            ** Convert input to rgb
            */
            /* This is Cb */
            cb = (uint32)(*((uint8*)(inputImgPtr++)))-128;
            /* Next Byte is luma of first pixel */
            lumaa1 = *((uint8*)(inputImgPtr++));
            /* Next byte is cr */
            cr = (uint32)(*((uint8*)(inputImgPtr++)))-128;
            rc = (8*cb + ycbcr2rgb_convert1*cr)*4+0x8000;
            rc=rc>>16;
            gc = (ycbcr2rgb_convert2*cb + ycbcr2rgb_convert3*cr)*4+0x8000;
            gc = gc>>16;
            bc = (ycbcr2rgb_convert4*cb + ycbcr2rgb_convert5*cr)*4+0x8000;
            bc = bc>>16;
            r = lumaa1 + rc;
            r = CLIPIT(r);
            out = r444[r];
            r = lumaa1 + gc;
            r = CLIPIT(r);
            out += g444[r];
            r = lumaa1 + bc;
            r = CLIPIT(r);
            out += b444[r];
            *outputImgPtr = out;
            outputImgPtr++;
            /* Next byte is luma of 2nd pixel */
            lumaa1 = *((uint8*)(inputImgPtr++));
            r = lumaa1 + rc;
            r = CLIPIT(r);
            out = r444[r];
            r = lumaa1 + gc;
            r = CLIPIT(r);
            out += g444[r];
            r = lumaa1 + bc;
            r = CLIPIT(r);
            out += b444[r];
            *outputImgPtr = out;
            outputImgPtr++;
          }
        } /* End of col loop */
        inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc);
        frameImgPtr = (uint16*)((uint32)frameImgPtr + rowInc);
        outputImgPtr = (uint16*)((uint32)outputImgPtr + destInc);
      } /* End of row loop */
     
      MSG_LOW("ipl_rot_add_crop marker_111\n");
      return IPL_SUCCESS;
    } else {
      MSG_LOW("ipl_rot_add_crop marker_224\n");
      /*
      ** Only RGB565, RGB666, RGB444 and ycbcr supported
      */
      return IPL_FAILURE;
    }
  }
  else if (input_img_ptr->cFormat == IPL_RGB565)
  {
    /*
    ** Input is in RGB565 format
    */
    /*
    ** First carry out the rotation
    */
    if (rotate == IPL_NOROT)
    {
      if ((input_img_ptr->dx < crop->dx) || (input_img_ptr->dy < crop->dy))
      {
        MSG_LOW("ipl_rot_add_crop marker_225\n");
        return IPL_FAILURE;
      }
      /*
      ** Dont rotate
      ** Call a more optimized version
      */
      if (input_frame_ptr == NULL)
      {
        if(image_crop_rgbi_o(input_img_ptr, output_img_ptr,crop)!= IPL_SUCCESS)
        {
          MSG_LOW("ipl_rot_add_crop marker_226\n");
          return IPL_FAILURE;
        }
      } 
      else 
      {
        startPos = 0;
        colInc = 1;
        rowInc = 1 + input_img_ptr->dx - crop->dx;

        if (image_rot_add_crop_rgbi(input_img_ptr,input_frame_ptr, 
                                  transparentValue, output_img_ptr,startPos,
                                  colInc,rowInc,crop) != IPL_SUCCESS)
        {
          MSG_LOW("ipl_rot_add_crop marker_227\n");
          return IPL_FAILURE;
        }
      }
    } 
    else if (rotate == IPL_ROT90)
    {
      if ((input_img_ptr->dy < crop->dx) || (input_img_ptr->dx < crop->dy))
      {
        MSG_LOW("ipl_rot_add_crop marker_228\n");
        return IPL_FAILURE;
      }
      if (output_img_ptr->cFormat == IPL_YCbCr)
      {
        /* Rotate 90 clockwise. */
        startPos = input_img_ptr->dx*(input_img_ptr->dy-1);
        colInc = -(int32)input_img_ptr->dx;
        rowInc = (input_img_ptr->dx)*(input_img_ptr->dy-1)+1 -
                        (input_img_ptr->dy - crop->dx)*input_img_ptr->dx;

        if(image_rot_add_crop_rgbi(input_img_ptr,input_frame_ptr, 
                                 transparentValue, output_img_ptr,startPos,
                                 colInc,rowInc,crop) != IPL_SUCCESS)
        {
          MSG_LOW("ipl_rot_add_crop marker_229\n");
          return IPL_FAILURE;
        }

      }
      else
      {
        // we can reduce cache misses if going from rgb to rgb and doing 90 
        // or 270 rotation
        //
        /* Rotate 90 clockwise. */
        startPos = input_img_ptr->dx*(input_img_ptr->dy-1);
        colInc = 1;
        rowInc = -((int32)crop->dy) - input_img_ptr->dx;

        if(image_rot_add_crop_rgbi_colrow(input_img_ptr,input_frame_ptr, 
                                 transparentValue, output_img_ptr,startPos,
                                 colInc,rowInc,crop) != IPL_SUCCESS)
        {
          MSG_LOW("ipl_rot_add_crop marker_230\n");
          return IPL_FAILURE;
        }
      }
    } 
    else if (rotate==IPL_ROT180)
    {
      /* Rotate 180 clockwise.  Dont reflect */
      startPos = (input_img_ptr->dx*input_img_ptr->dy)-1;
      colInc = -1;
      rowInc = -((int32)input_img_ptr->dx - (int32)crop->dx) - 1;

      if ((input_img_ptr->dx < crop->dx) || (input_img_ptr->dy < crop->dy))
      {
        MSG_LOW("ipl_rot_add_crop marker_231\n");
        return IPL_FAILURE;
      }
      if (image_rot_add_crop_rgbi(input_img_ptr,input_frame_ptr, 
                                  transparentValue, output_img_ptr,startPos,
                                  colInc,rowInc,crop) != IPL_SUCCESS)
      {
        MSG_LOW("ipl_rot_add_crop marker_232\n");
        return IPL_FAILURE;
      }
    } 
    else if ( rotate==IPL_ROT270 )
    {
      if ((input_img_ptr->dy < crop->dx) || (input_img_ptr->dx < crop->dy))
      {
        MSG_LOW("ipl_rot_add_crop marker_233\n");
        return IPL_FAILURE;
      }
      if (output_img_ptr->cFormat == IPL_YCbCr)
      {
        /* Rotate 270 clockwise.  Dont reflect */
        startPos = input_img_ptr->dx-1;
        colInc = input_img_ptr->dx;
        rowInc = -(int32)(input_img_ptr->dx)*(input_img_ptr->dy-1) -1 + 
          (input_img_ptr->dy - crop->dx)*input_img_ptr->dx;

        if(image_rot_add_crop_rgbi(input_img_ptr,input_frame_ptr, 
                                 transparentValue, output_img_ptr,startPos,
                                 colInc,rowInc,crop) != IPL_SUCCESS)
        {
          MSG_LOW("ipl_rot_add_crop marker_234\n");
          return IPL_FAILURE;
        }
      }
      else
      {
        // we can reduce cache misses if going from rgb to rgb and doing 90 
        // or 270 rotation
        //
        /* Rotate 270 clockwise.  Dont reflect */
        startPos = input_img_ptr->dx-1;
        colInc = -1;
        rowInc = crop->dy + input_img_ptr->dx;

        if(image_rot_add_crop_rgbi_colrow(input_img_ptr,input_frame_ptr, 
                                 transparentValue, output_img_ptr,startPos,
                                 colInc,rowInc,crop) != IPL_SUCCESS)
        {
          MSG_LOW("ipl_rot_add_crop marker_235\n");
          return IPL_FAILURE;
        }
      }
    } 
    else
    {
      MSG_LOW("ipl_rot_add_crop marker_236\n");
      return IPL_FAILURE;
    }
  } 
  else if (input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    if (ipl2_rot_add_crop(i_img_ptr, i_frame_ptr, o_img_ptr, icrop, rotate,
        icon_list_ptr,    transparentValue))
    {
      MSG_LOW("ipl_rot_add_crop marker_237\n");
      return IPL_FAILURE;
    }
  }
  else
  {
    MSG_LOW("ipl_rot_add_crop marker_238\n");
    return IPL_FAILURE;
  }

  if (icon_list_ptr) //lint !e774
  {
    /*
    ** Lets process icons.  Only valid for input = IPL_RGB565 and
    ** output = IPL_RGB565
    */
    frame_index = 0;
    /*
    ** Now iterate over the various icons..
    */
    while ((frame_index<100) && icon_list_ptr[frame_index] != NULL)
    {
      if (icon_list_ptr[frame_index]->cFormat != IPL_RGB565)
      {
        MSG_LOW("ipl_rot_add_crop marker_239\n");
        return IPL_FAILURE;
      }
      outputImgPtr = (uint16*)output_img_ptr->imgPtr;
      luma1 = icon_list_ptr[frame_index]->x +
        icon_list_ptr[frame_index]->y*output_img_ptr->dx;
      outputImgPtr = outputImgPtr + luma1;
      frameImgPtr = (uint16*)icon_list_ptr[frame_index]->imgPtr;
      luma1 = output_img_ptr->dx - icon_list_ptr[frame_index]->dx;
      cb = icon_list_ptr[frame_index]->pitch - 
           (icon_list_ptr[frame_index]->dx*2);
      for (row=0;row < icon_list_ptr[frame_index]->dy;row++)
      {
        for (col=0;col<icon_list_ptr[frame_index]->dx;col++)
        {
          if (*frameImgPtr != transparentValue)
          {
            *outputImgPtr = *frameImgPtr;
          }
          outputImgPtr++;
          frameImgPtr++;
        }
        frameImgPtr = (uint16*)((uint32)frameImgPtr + cb);
        outputImgPtr += luma1;
      }
      frame_index++;
    }
  }

  MSG_LOW("ipl_rot_add_crop marker_112\n");
  return IPL_SUCCESS;
} /* End ipl_rot_add_crop */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_rotate90

DESCRIPTION
  This function will do rotation and color conversion if needed.
  If the input is bayer, it is assumed that the caller will
  set the appropriate output color format.  For example:
  If the input is IPL_BAYER_GBRG, a rotation of 90 degrees will
  cause an output format of IPL_BAYER_RGGB.

  The rotation is always clockwise.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr points to the input image.  Input can be YCbCr or Bayer
  o_img_ptr points to the output image
  rotate is the type of rotation to do

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_rotate90
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  ipl_rotate90_type rotate           /* Type of rotation               */
)
{
  /* Take an image of size of any size and converts it to black and white
  */
  int32 index,rowInc=0,colInc=0,startPos=0;
  uint32 luma1=0,luma2=0,row,col;
  unsigned short out;
  uint32 cb=0,cr=0,cb1=0,cr1=0;
  long rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  short ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
  ipl_image_type* input_img_ptr;
  ipl_image_type* output_img_ptr;
  boolean convert = TRUE;

  MSG_LOW("ipl_rotate90 marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_rotate90 marker_200\n");
    return IPL_FAILURE;
  }
  input_img_ptr = i_img_ptr;
  output_img_ptr = o_img_ptr;



  // quick check on dimensions
  if ((rotate == IPL_NOROT))
  {
    if ((input_img_ptr->dx != output_img_ptr->dx) &&
        (input_img_ptr->dy != output_img_ptr->dy))
    {
      MSG_LOW("ipl_rotate90 marker_201\n");
      return IPL_FAILURE;
    }
  } 
  else if ((rotate==IPL_ROT90))
  {
    if ((input_img_ptr->dx != output_img_ptr->dy) &&
        (input_img_ptr->dy != output_img_ptr->dx))
    {
      MSG_LOW("ipl_rotate90 marker_202\n");
      return IPL_FAILURE;
    }
  } 
  else if ((rotate==IPL_ROT180))
  {
    if ((input_img_ptr->dx != output_img_ptr->dx) &&
        (input_img_ptr->dy != output_img_ptr->dy))
    {
      MSG_LOW("ipl_rotate90 marker_203\n");
      return IPL_FAILURE;
    }
  } 
  else if (rotate==IPL_ROT270)
  {
    if ((input_img_ptr->dx != output_img_ptr->dy) && 
        (input_img_ptr->dy != output_img_ptr->dx))
    {
      MSG_LOW("ipl_rotate90 marker_204\n");
      return IPL_FAILURE;
    }
  } 

  MSG_LOW("ipl_rotate90 marker_1\n");

  if (input_img_ptr->cFormat == IPL_RGB565)
  {
    uint16 *in16, *out16;

    if ((rotate == IPL_NOROT))
    {
      /* Dont rotate or reflect */
      startPos = 0;
      colInc = 1;
      rowInc = 1;
    } 
    else if ((rotate==IPL_ROT90))
    {
      /* Rotate 90 clockwise */
      startPos = input_img_ptr->dx*(input_img_ptr->dy-1);
      colInc = -(int32)input_img_ptr->dx;
      rowInc = (input_img_ptr->dx)*(input_img_ptr->dy-1)+1;
    } 
    else if ((rotate==IPL_ROT180))
    {
      /* Rotate 180 clockwise */
      startPos = (input_img_ptr->dx*input_img_ptr->dy)-1;
      colInc = -1;
      rowInc = -1;
    } 
    else if (rotate==IPL_ROT270)
    {
      /* Rotate 270 clockwise */
      startPos = input_img_ptr->dx-1;
      colInc = input_img_ptr->dx;
      rowInc = -(int32)(input_img_ptr->dx)*(input_img_ptr->dy-1)-1;
    } 
    else
    {
      MSG_LOW("ipl_rotate90 marker_205\n");
      /* Option Not supported */
      return IPL_FAILURE;
    }

    /* initialize the index to starting position */
    index = startPos;  

    in16 = (uint16 *) input_img_ptr->imgPtr;
    out16 = (uint16 *) output_img_ptr->imgPtr;

    /* Now loop through the image once */
    for(row = 0; row < output_img_ptr->dy; row++)
    {
      for(col = 0; col < (output_img_ptr->dx); col++)
      {
        /* No need for RGB conversion, simply output YCbCr data */
        out16[(col+row*(output_img_ptr->dx))] = in16[index]; 

        index = index + (colInc); /* byte addressed */
      } 
      index = index + (rowInc) - (colInc);  /* byte addressed */
    } 
  } 
  else if (input_img_ptr->cFormat == IPL_RGB888)
  {
    uint8 *in8, *out8;
    int dx3 = 3*input_img_ptr->dx;
    int dy = input_img_ptr->dy;

    if ((rotate == IPL_NOROT))
    {
      /* Dont rotate or reflect */
      startPos = 0;
      colInc = 3;
      rowInc = 3;
    } 
    else if ((rotate==IPL_ROT90))
    {
      /* Rotate 90 clockwise */
      startPos = dx3*(dy-1);
      colInc = -dx3;
      rowInc = dx3*(dy-1)+3;
    } 
    else if ((rotate==IPL_ROT180))
    {
      /* Rotate 180 clockwise */
      startPos = (dx3*dy)-3;
      colInc = -3;
      rowInc = -3;
    } 
    else if (rotate==IPL_ROT270)
    {
      /* Rotate 270 clockwise */
      startPos = dx3-3;
      colInc = dx3;
      rowInc = -dx3*(dy-1)-3;
    } 
    else
    {
      MSG_LOW("ipl_rotate90 marker_206\n");
      /* Option Not supported */
      return IPL_FAILURE;
    }
    /* initialize the index to starting position */
    index = startPos;  

    in8 = input_img_ptr->imgPtr;
    out8 = output_img_ptr->imgPtr;

    /* Now loop through the image once */
    for(row = 0; row < output_img_ptr->dy; row++)
    {
      for(col = 0; col < output_img_ptr->dx*3; col += 3)
      {
        /* No need for RGB conversion, simply output YCbCr data */
        out8[col+(row*output_img_ptr->dx*3)] = in8[index]; 
        out8[col+(row*output_img_ptr->dx*3)+1] = in8[index+1]; 
        out8[col+(row*output_img_ptr->dx*3)+2] = in8[index+2]; 

        index = index + (colInc); /* byte addressed */
      } 
      index = index + (rowInc) - (colInc);  /* byte addressed */
    } 
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr)
  {
    /*
     ** YCbCr Input
     */
    if (output_img_ptr->cFormat == input_img_ptr->cFormat)
    {
      /* We don't need to color convert */
      convert = FALSE;
    }

    if ((rotate == IPL_NOROT))
    {
      /* Dont rotate or reflect */
      startPos = 0;
      colInc = 1;
      rowInc = 1;
    } else if ((rotate==IPL_ROT90))
    {
      /* Rotate 90 clockwise */
      startPos = input_img_ptr->dx*(input_img_ptr->dy-1);
      colInc = -(int32)input_img_ptr->dx;
      rowInc = (input_img_ptr->dx)*(input_img_ptr->dy-1)+1;
    } else if ((rotate==IPL_ROT180))
    {
      /* Rotate 180 clockwise */
      startPos = (input_img_ptr->dx*input_img_ptr->dy)-1;
      colInc = -1;
      rowInc = -1;
    } else if (rotate==IPL_ROT270)
    {
      /* Rotate 270 clockwise */
      startPos = input_img_ptr->dx-1;
      colInc = input_img_ptr->dx;
      rowInc = -(int32)(input_img_ptr->dx)*(input_img_ptr->dy-1)-1;
    } 
    else
    {
      MSG_LOW("ipl_rotate90 marker_207\n");
      /* Option Not supported */
      return IPL_FAILURE;
    }

    /* initialize the index to starting position */
    index = startPos<<1;  /* byte addressed */

    /* Now loop through the image once */
    for(row = 0; row < output_img_ptr->dy; row++)
    {
      for(col = 0; col < (output_img_ptr->dx); col++)
      {
        /* First Byte is is either Cb or Cr. CbY CrY CbY CrY CbY CrY */
        if (!(col%2)) 
        {
          if (rotate == IPL_ROT270)
          {
            if (!(row%2))
            {
              /* This is Cb */
              cb=input_img_ptr->imgPtr[index];
              cb1=input_img_ptr->imgPtr[index-2];
            }
            else
            {
              /* This is Cb */
              cb=input_img_ptr->imgPtr[index];
              cb1=input_img_ptr->imgPtr[index+2];
            }
          }
          else
          {
            if (!(row%2))
            {
              /* This is Cb */
              cb=input_img_ptr->imgPtr[index];
              cb1=input_img_ptr->imgPtr[index+2];
            }
            else
            {
              /* This is Cb */
              cb=input_img_ptr->imgPtr[index];
              cb1=input_img_ptr->imgPtr[index-2];
            }
          }

          /* Next Byte is the luma */
          luma1 =  input_img_ptr->imgPtr[index+1]; //Byte addressed
        } 
        else 
        {
          if (rotate == IPL_ROT270)
          {
            /* This is Cr */
            if (!(row%2))
            {
              cr = input_img_ptr->imgPtr[index];
              cr1= input_img_ptr->imgPtr[index-2];
            }
            else
            {
              cr = input_img_ptr->imgPtr[index];
              cr1= input_img_ptr->imgPtr[index+2];
            }
          }
          else
          {
            /* This is Cr */
            if (!(row%2))
            {
              cr = input_img_ptr->imgPtr[index];
              cr1= input_img_ptr->imgPtr[index+2];
            }
            else
            {
              cr = input_img_ptr->imgPtr[index];
              cr1= input_img_ptr->imgPtr[index-2];
            }
          }

          /* Next Byte is the luma */
          luma2 = input_img_ptr->imgPtr[index+1];

          if (rotate == IPL_ROT90)
          {
#if 1
            if (row%2)
              cb = cb1;
            else 
              cr = cr1;
#else
            if (row%2)
            {
              //cb = (cb1 + cr1) >> 1;
              //cr = (cb + cr)   >> 1;
              cb = cb1;
            }
            else
            {
              //cb = (cb + cr)   >> 1;
              //cr = (cr1 + cb1) >> 1;
              cr = cr1;
            }
#endif
          }
          else if (rotate == IPL_ROT180)
          {
            /* Swap cb and cr */
            cr1 = cb;
            cb = cr;
            cr = cr1;
          }
          else if (rotate == IPL_ROT270)
          {
            if (row%2)
              cr = cr1;
            else 
              cb = cb1;
          } 

          if (convert)
          {
            /* YCbCr2RGB lags behind by 2 pixels always */
            rc = (ycbcr_convert[0]*(cb-128) + 
                  ycbcr_convert[1]*(cr-128))*4+0x8000;
            gc = (ycbcr_convert[2]*(cb-128) + 
                  ycbcr_convert[3]*(cr-128))*4+0x8000;
            bc = (ycbcr_convert[4]*(cb-128) + 
                  ycbcr_convert[5]*(cr-128))*4+0x8000;
            r = luma1 + (rc>>16);
            g = luma1 + (gc>>16);
            b = luma1 + (bc>>16);
            r=CLIPIT(r);
            g=CLIPIT(g);
            b=CLIPIT(b);
            /*
            ** Masking bits for 5 ==> 0xF8 and 6==> 0xFC
            ** Order of the 2 bytes is R5G3 G3B5
            */
            out = pack_rgb565(r,g,b);
            *((unsigned short *)(output_img_ptr->imgPtr + \
            (col-1+row*(output_img_ptr->dx))*2) )= (unsigned short) out;
            r = luma2 + (rc>>16);
            g = luma2 + (gc>>16);
            b = luma2 + (bc>>16);
            r=CLIPIT(r);
            g=CLIPIT(g);
            b=CLIPIT(b);
            out = pack_rgb565(r,g,b);
            *((unsigned short*)(output_img_ptr->imgPtr + \
            (col+row*(output_img_ptr->dx))*2 ) )= (unsigned short) out;
          } 
          else 
          {
            /* No need for RGB conversion, simply output YCbCr data */
            output_img_ptr->imgPtr[((col+row*(output_img_ptr->dx))<<1)-2] = 
              (unsigned char) cb;
            output_img_ptr->imgPtr[((col+row*(output_img_ptr->dx))<<1)-1] = 
              (unsigned char)luma1;
            output_img_ptr->imgPtr[((col+row*(output_img_ptr->dx))<<1)  ] = 
              (unsigned char) cr;
            output_img_ptr->imgPtr[((col+row*(output_img_ptr->dx))<<1)+1] = 
              (unsigned char)luma2;
          }
        } /* end of Cb Cr */
        index = index + (colInc<<1); /* byte addressed */

      } /* End of col loop */
      index = index + (rowInc<<1) - (colInc<<1);  /* byte addressed */

    } /* End of row loop */
  }
  else if ((input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
           (input_img_ptr->cFormat == IPL_YCrCb422_LINE_PK))
  {
    if (rotate == IPL_NOROT)
    {
      ipl_copy_and_paste(input_img_ptr, output_img_ptr, NULL, NULL);
    }
    else if (rotate == IPL_ROT90)
    {
      ipl_rot90lp(input_img_ptr, output_img_ptr, NULL);
    }
    else if (rotate == IPL_ROT180)
    {
      ipl_rot180lp(input_img_ptr, output_img_ptr, NULL);
    }
    else if (rotate == IPL_ROT270)
    {
      ipl_rot270lp(input_img_ptr, output_img_ptr, NULL);
    }
    else
    {
      return IPL_FAILURE;
    }
  }
  else if ((input_img_ptr->cFormat == IPL_BAYER_BGGR) || 
            (input_img_ptr->cFormat == IPL_BAYER_GBRG) ||
            (input_img_ptr->cFormat == IPL_BAYER_RGGB) ||
            (input_img_ptr->cFormat == IPL_BAYER_GRBG))
  {
    /*
    ** Bayer Input
    */
    /*
    ** First carry out the rotation
    */
    if (rotate == IPL_NOROT)
    {
      /*
      ** Rotation not needed
      */
      memcpy(output_img_ptr->imgPtr,input_img_ptr->imgPtr,
             input_img_ptr->dx*input_img_ptr->dy);

      MSG_LOW("ipl_rotate90 marker_100\n");
      return IPL_SUCCESS;
    } else if (rotate == IPL_ROT90)
    {
      /* Rotate 90 clockwise. */
      startPos = input_img_ptr->dx*(input_img_ptr->dy-1);
      colInc = -(int32)input_img_ptr->dx;
      rowInc = (input_img_ptr->dx)*(input_img_ptr->dy-1)+1;
      if (input_img_ptr->dy != output_img_ptr->dx)
      {
        MSG_LOW("ipl_rotate90 marker_208\n");
        /* Invalid settings */
        return IPL_FAILURE;
      }
    } else if ( rotate==IPL_ROT180 )
    {
      /* Rotate 180 clockwise.  Dont reflect */
      startPos = (input_img_ptr->dx*input_img_ptr->dy)-1;
      colInc = -1;
      rowInc = -1;
      if (input_img_ptr->dx != output_img_ptr->dx)
      {
        MSG_LOW("ipl_rotate90 marker_209\n");
        /* Invalid settings */
        return IPL_FAILURE;
      }
    } else if ( rotate==IPL_ROT270 )
    {
      /* Rotate 270 clockwise.  Dont reflect */
      startPos = input_img_ptr->dx-1;
      colInc = input_img_ptr->dx;
      rowInc = -(int32)(input_img_ptr->dx)*(input_img_ptr->dy-1) -1;
      if (input_img_ptr->dy != output_img_ptr->dx)
      {
        MSG_LOW("ipl_rotate90 marker_210\n");
        /* Invalid settings */
        return IPL_FAILURE;
      }
    } else
    {
      MSG_LOW("ipl_rotate90 marker_211\n");
      /* Rotation Option Not supported */
      return IPL_FAILURE;
    }
    if(image_rot_bayer_o( \
      input_img_ptr,output_img_ptr,startPos,colInc,rowInc) != IPL_SUCCESS) {
        MSG_LOW("ipl_rotate90 marker_212\n");
        return IPL_FAILURE;
    }
  }
  else
  {
    MSG_LOW("ipl_rotate90 marker_213\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_rotate90 marker_101\n");
  return IPL_SUCCESS;
} /* End ipl_rotate90 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_reflect

DESCRIPTION
  This function performs reflection and color conversion if needed.
  Only RGB565 and YCbCr 4:2:2 inputs are supported.
  If input is RGB565, only RGB565 output is supported.
  If input is YCbCr 4:2:2, RGB565 and YCbCr 4:2:2 output are supported.

  If output image is NULL, the reflection will happen in place.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image
  output_img_ptr  pointer to the output image
  reflect         type of reflection

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_reflect
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  ipl_reflect_type reflect           /* Type of reflection             */
)
{
  /* Take an image of size of any size and converts it to black and white
  */
  int32 startIndex, index = 0;
  int32 indexColInc, indexRowInc;
  uint32 rowMax, colMax;
  uint32 luma1=0,luma2=0,row,col;
  uint16 out, out2;
  uint32 cb=0,cr=0;
  ipl_image_type temp_o_img_ptr;
  long r;
  register ipl_image_type* input_img_ptr;
  register ipl_image_type* output_img_ptr;
  uint32 dx, dy;

  MSG_LOW("ipl_reflect marker_0\n");

  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  //short ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};

  if (!i_img_ptr || !i_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_reflect marker_200\n");
    return IPL_FAILURE;
  }
  input_img_ptr = i_img_ptr;

  // if user passes NULL for output ptr, then assume he wants us to do it
  // inplace.
  if (o_img_ptr == NULL)
  {
    temp_o_img_ptr.dx = i_img_ptr->dx;
    temp_o_img_ptr.dy = i_img_ptr->dy;
    temp_o_img_ptr.cFormat = i_img_ptr->cFormat;
    temp_o_img_ptr.imgPtr = i_img_ptr->imgPtr; // do this reflection inplace!
    temp_o_img_ptr.clrPtr = i_img_ptr->clrPtr; // do this reflection inplace!

    output_img_ptr = &temp_o_img_ptr;
  }
  else
  {
    output_img_ptr = o_img_ptr;
  }

  if ((input_img_ptr->dx != output_img_ptr->dx) ||
      (input_img_ptr->dy != output_img_ptr->dy))
  {
    MSG_LOW("ipl_reflect marker_201\n");
    return IPL_FAILURE;
  }

  dx = i_img_ptr->dx;
  dy = i_img_ptr->dy;

  if ((reflect == IPL_YREF))
  {
    /* Reflect on y axis */
    if (input_img_ptr->cFormat == IPL_RGB888)
    {
      startIndex = 3*dx-3;
      indexColInc = -6; 
    }
    else
    {
      startIndex = dx-1;
      indexColInc = -2; 
    }

    indexRowInc = 0; 

    rowMax = dy; 
    colMax = dx/2; 

    // we mulitply by two since we advance one each time, so we must
    // reduce the distance between swapped pixels 2x as much each time
  } 
  else if ((reflect == IPL_XREF))
  {
    /* Reflect on x axis. */
    if (input_img_ptr->cFormat == IPL_RGB888)
    {
      startIndex = (3*dx*(dy-1));
      indexRowInc = 6*dx;
    }
    else
    {
      startIndex = (dx*(dy-1));
      indexRowInc = 2*dx;
    }

    indexColInc = 0;

    colMax = dx;
    rowMax = dy/2; 
    // we mulitply by two since we advance one each time, so we must
    // reduce the distance between swapped pixels 2x as much each time
  } 
  else
  {
    MSG_LOW("ipl_reflect marker_202\n");
    /* Option Not supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_reflect marker_1\n");

  if (input_img_ptr->cFormat == IPL_RGB565 &&
      output_img_ptr->cFormat == IPL_RGB565)
  {
    // if we are doing it in place, do it this optimized way
    // otherwise we have another optimized method
    if (o_img_ptr == NULL)
    {
      uint16 * inImgPtr = (uint16 *) input_img_ptr->imgPtr;
      uint16 * outImgPtr = (uint16 *) output_img_ptr->imgPtr;
      uint16 data;

      /* Now loop through the image once */
      for(row = 0; row < rowMax; row++)
      {
        outImgPtr =((uint16 *) output_img_ptr->imgPtr + row*output_img_ptr->dx);
        inImgPtr = ((uint16 *) input_img_ptr->imgPtr  + row*input_img_ptr->dx);
        index = startIndex - row*indexRowInc;

        for(col = 0; col < colMax; col++)
        {
          data = *inImgPtr;
          *outImgPtr = *(inImgPtr + index);
          *(outImgPtr + index) = data;

          inImgPtr++;
          outImgPtr++;

          index += indexColInc; 
        } 

        // if odd width, copy the center column exactly over
        if (reflect == IPL_YREF && dx%2) 
          *outImgPtr     = *(inImgPtr);
      } 


      // if odd height, copy the center row exactly over
      if (reflect == IPL_XREF && dy%2) 
      {
        for(col = 0; col < colMax; col++)
          *outImgPtr++ = *(inImgPtr++);
      }
    }
    else
    {
      // we are not doing it in place, so use a different algorithm
      uint16 * inImgPtr = (uint16 *) input_img_ptr->imgPtr;
      uint16 * outImgPtr = (uint16 *) output_img_ptr->imgPtr;

      if (reflect == IPL_YREF)
      {
        outImgPtr += (dx-1);
        for(row = dy; row; row--)
        {
          for(col = dx; col; col--)
            *outImgPtr-- = *inImgPtr++;

          outImgPtr += 2*dx;
        } 
      }
      else
      {
        outImgPtr += (dx*(dy-1));
        for(row = dy; row; row--)
        {
          for(col = dx; col; col--)
            *outImgPtr++ = *inImgPtr++;

          outImgPtr -= 2*dx;
        } 
      }
    }  
  }
  else if (input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    // only support inplace right now
    if (o_img_ptr == NULL)
    {
      uint8 * inPtr1;
      uint8 * inPtr2;
      uint8 data;

      if (reflect == IPL_XREF) 
      {
        inPtr1 = input_img_ptr->imgPtr;
        inPtr2 = inPtr1 + dx*(dy-1); 

        // do luma first
        for(row = 0; row < dy/2; row++)
        {
          for(col = 0; col < dx; col++)
          {
            data = *inPtr2;
            *inPtr2 = *inPtr1;
            *inPtr1 = data;

            inPtr1++;
            inPtr2++;
          }
          inPtr2 -= (2*dx);
        }

        // now do chroma 
        inPtr1 = input_img_ptr->clrPtr;
        inPtr2 = inPtr1 + dx*(dy/2-1);
        for(row = 0; row < dy/4; row++)
        {
          for(col = 0; col < dx; col++)
          {
            data = *inPtr2;
            *inPtr2 = *inPtr1;
            *inPtr1 = data;

            inPtr1++;
            inPtr2++;
          }
          inPtr2 -= (2*dx);
        }
      } 
      else if (reflect == IPL_YREF) 
      {
        inPtr1 = input_img_ptr->imgPtr;
        inPtr2 = inPtr1 + dx-1;

        // do luma
        for(row = 0; row < dy; row++)
        {
          for(col = 0; col < dx/2; col++)
          {
            data = *inPtr2;
            *inPtr2 = *inPtr1;
            *inPtr1 = data;

            inPtr1++;
            inPtr2--;
          }
          inPtr1 += (dx/2);
          inPtr2 += (dx/2 + dx);
        }

        // now do chroma 
        inPtr1 = input_img_ptr->clrPtr;
        inPtr2 = inPtr1 + dx-1;
        for(row = 0; row < dy/2; row++)
        {
          for(col = 0; col < dx/4; col++)
          {
            data = *(inPtr2-1);
            *(inPtr2-1) = *inPtr1;
            *inPtr1 = data;

            data = *inPtr2;
            *inPtr2 = *(inPtr1+1);
            *(inPtr1+1) = data;

            inPtr1 += 2;
            inPtr2 -= 2;
          }
          inPtr1 += (dx/2);
          inPtr2 += (dx/2 + dx);
        }
      }
      else
      {
        return( IPL_FAILURE );
      }
    }
    else
    {
      return( IPL_FAILURE );
    }
  } 
  else if (input_img_ptr->cFormat == IPL_RGB888 &&
           output_img_ptr->cFormat == IPL_RGB888)
  {
    uint8 * inImgPtr = input_img_ptr->imgPtr;
    uint8 * outImgPtr = output_img_ptr->imgPtr;
    uint8 data, data2, data3;

    /* Now loop through the image once */
    for(row = 0; row < rowMax; row++)
    {
      outImgPtr =(output_img_ptr->imgPtr + 3*row*dx);
      inImgPtr = (input_img_ptr->imgPtr  + 3*row*dx);
      index = startIndex - row*indexRowInc;

      for(col = 0; col < colMax; col++)
      {
        data = *inImgPtr;
        data2 = *(inImgPtr+1);
        data3 = *(inImgPtr+2);

        *outImgPtr     = *(inImgPtr + index);
        *(outImgPtr+1) = *(inImgPtr + index + 1);
        *(outImgPtr+2) = *(inImgPtr + index + 2);

        *(outImgPtr + index)   = data;
        *(outImgPtr + index+1) = data2;
        *(outImgPtr + index+2) = data3;


        inImgPtr += 3;
        outImgPtr += 3;

        index += indexColInc; 
      } 

      // if odd width, copy the center column exactly over
      if (reflect == IPL_YREF && dx%2) 
      {
        *outImgPtr     = *(inImgPtr);
        *(outImgPtr+1) = *(inImgPtr + 1);
        *(outImgPtr+2) = *(inImgPtr + 2);
      }

    } 

    // if odd height, copy the center row exactly over
    if (reflect == IPL_XREF && dy%2) 
    {
      for(col = 0; col < colMax; col++)
      {
        *outImgPtr     = *(inImgPtr);
        *(outImgPtr+1) = *(inImgPtr + 1);
        *(outImgPtr+2) = *(inImgPtr + 2);

        inImgPtr += 3;
        outImgPtr += 3;

        index += indexColInc; 
      } 
    }
  } 
  else if (input_img_ptr->cFormat == IPL_YCbCr &&
           output_img_ptr->cFormat == IPL_YCbCr)
  {
    uint8 * inImgPtr = input_img_ptr->imgPtr;
    uint8 * outImgPtr = output_img_ptr->imgPtr;


    // if we are doing it in place, do it this optimized way
    // otherwise we have another optimized method
    if (o_img_ptr == NULL)
    {
      if ((reflect == IPL_YREF))
        startIndex = 2*input_img_ptr->dx-4;
      else
        startIndex = (2*dx*(dy-1));

      /* Now loop through the image once */
      if (reflect == IPL_YREF)
      {
        for(row = 0; row < rowMax; row++)
        {
          outImgPtr=((uint8 *)output_img_ptr->imgPtr+ row*2*dx);
          inImgPtr= ((uint8 *)input_img_ptr->imgPtr + row*2*dx);
          index = startIndex - 2*row*indexRowInc;

          for(col = 0; col < colMax; col += 2)
          {
            cb    = *inImgPtr;
            luma1 = *(inImgPtr+1); 
            cr    = *(inImgPtr+2);
            luma2 = *(inImgPtr+3);

            *(outImgPtr) = *(inImgPtr + index);
            *(outImgPtr+1) = *(inImgPtr + index+3);
            *(outImgPtr+2) = *(inImgPtr + index+2);
            *(outImgPtr+3) = *(inImgPtr + index+1);

            *(outImgPtr + index)   = (uint8) cb;
            *(outImgPtr + index+1) = (uint8) luma2;
            *(outImgPtr + index+2) = (uint8) cr;
            *(outImgPtr + index+3) = (uint8) luma1;

            inImgPtr += 4;
            outImgPtr += 4;

            index += (4*indexColInc); 
          }
        } 
      } 
      else if (reflect == IPL_XREF)
      {
        for(row = 0; row < rowMax; row++)
        {
          outImgPtr=((uint8 *) output_img_ptr->imgPtr+row*2*dx);
          inImgPtr = ((uint8 *) input_img_ptr->imgPtr +row*2*dx);
          index = startIndex - 2*row*indexRowInc;

          for(col = 0; col < colMax; col += 2)
          {
            cb    = *inImgPtr;
            luma1 = *(inImgPtr+1); 
            cr    = *(inImgPtr+2);
            luma2 = *(inImgPtr+3);

            *(outImgPtr) = *(inImgPtr + index);
            *(outImgPtr+1) = *(inImgPtr + index+1);
            *(outImgPtr+2) = *(inImgPtr + index+2);
            *(outImgPtr+3) = *(inImgPtr + index+3);

            *(outImgPtr + index) = (unsigned char) cb;
            *(outImgPtr + index+1) = (unsigned char) luma1;
            *(outImgPtr + index+2) = (unsigned char) cr;
            *(outImgPtr + index+3) = (unsigned char) luma2;

            inImgPtr += 4;
            outImgPtr += 4;

            index += (4*indexColInc); 
          }
        }  
      }
    }
    else
    {
      if (reflect == IPL_YREF)
      {
        outImgPtr += (2*dx-1);
        for(row = dy; row; row--)
        {
          for(col = dx/2; col; col--)
          {
            *outImgPtr-- = *(inImgPtr + 1); // Y1
            *outImgPtr-- = *(inImgPtr + 2); // Cr 
            *outImgPtr-- = *(inImgPtr + 3); // Y2
            *outImgPtr-- = *(inImgPtr);     // Cb
            inImgPtr += 4;
          }
          outImgPtr += 4*dx;
        } 
      }
      else
      {
        outImgPtr += (2*dx*(dy-1));
        for(row = dy; row; row--)
        {
          for(col = 2*dx; col; col--)
            *outImgPtr++ = *inImgPtr++;

          outImgPtr -= 4*dx;
        } 
      }
    }
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr &&
           output_img_ptr->cFormat == IPL_RGB565)
  {
    uint8 * inImgPtr = input_img_ptr->imgPtr;
    uint8 * outImgPtr = output_img_ptr->imgPtr;
    uint32 t_cb, t_cr;
    uint32 t_luma1, t_luma2;

    if ( ipl2_init() != IPL_SUCCESS )
    {
      IPL2_MSG_FATAL( " ipl2_upsize_QCIF_133 :: /\
      Could not initialize IPL2_IPL lookup tables");
      MSG_LOW("ipl_reflect marker_203\n");
      return( IPL_FAILURE );
    }

    if ((reflect == IPL_YREF))
      startIndex = 2*dx-4;
    else
      startIndex = (2*dx*(dy-1));

    /* Now loop through the image once */
    if (reflect == IPL_YREF)
    {
      for(row = 0; row < rowMax; row++)
      {
        outImgPtr=((uint8 *) output_img_ptr->imgPtr + row*2*dx);
        inImgPtr= ((uint8 *) input_img_ptr->imgPtr  + row*2*dx);
        index = startIndex - 2*row*indexRowInc;

        for(col = 0; col < colMax; col += 2)
        {
          // copy to temp buff
          t_cb    = *inImgPtr;
          t_luma2 = *(inImgPtr+1); 
          t_cr    = *(inImgPtr+2);
          t_luma1 = *(inImgPtr+3);


          // get values from p2 that we are going to put in p1
          cb =    *(inImgPtr + index);
          luma2 = *(inImgPtr + index+1);
          cr =    *(inImgPtr + index+2);
          luma1 = *(inImgPtr + index+3);

          // use the faster, yet still great quality color conversion
          IPL2_CONVERT_YCBCR_RGB(luma1, luma2, cr, cb, r, out, out2, \
                                        ipl2_r5xx, ipl2_gx6x,ipl2_bxx5);
          *((unsigned short *)outImgPtr)= (unsigned short) out;
          *((unsigned short *)outImgPtr+1)= (unsigned short) out2;
           
          // use the faster, yet still great quality color conversion
          IPL2_CONVERT_YCBCR_RGB(t_luma1, t_luma2, t_cr, t_cb, r, out, out2, \
                                        ipl2_r5xx, ipl2_gx6x,ipl2_bxx5);
          *((unsigned short *)outImgPtr+index/2)= (unsigned short) out;
          *((unsigned short *)outImgPtr+index/2+1)= (unsigned short) out2;

          inImgPtr += 4;
          outImgPtr += 4;

          index += (4*indexColInc); 
        }
      } 
    } 
    else if (reflect == IPL_XREF)
    {
      for(row = 0; row < rowMax; row++)
      {
        outImgPtr=((uint8 *) output_img_ptr->imgPtr + row*2*dx);
        inImgPtr= ((uint8 *) input_img_ptr->imgPtr  + row*2*dx);
        index = startIndex - 2*row*indexRowInc;

        for(col = 0; col < colMax; col += 2)
        {
          // copy to temp buff
          t_cb    = *inImgPtr;
          t_luma1 = *(inImgPtr+1); 
          t_cr    = *(inImgPtr+2);
          t_luma2 = *(inImgPtr+3);


          // get values from p2 that we are going to put in p1
          cb =    *(inImgPtr + index);
          luma2 = *(inImgPtr + index+3);
          cr =    *(inImgPtr + index+2);
          luma1 = *(inImgPtr + index+1);

          // use the faster, yet still great quality color conversion
          IPL2_CONVERT_YCBCR_RGB(luma1, luma2, cr, cb, r, out, out2, \
                                        ipl2_r5xx, ipl2_gx6x,ipl2_bxx5);
          *((unsigned short *)outImgPtr)= (unsigned short) out;
          *((unsigned short *)outImgPtr+1)= (unsigned short) out2;
           
          // use the faster, yet still great quality color conversion
          IPL2_CONVERT_YCBCR_RGB(t_luma1, t_luma2, t_cr, t_cb, r, out, out2, \
                                        ipl2_r5xx, ipl2_gx6x,ipl2_bxx5);
          *((unsigned short *)outImgPtr+index/2)= (unsigned short) out;
          *((unsigned short *)outImgPtr+index/2+1)= (unsigned short) out2;

          inImgPtr += 4;
          outImgPtr += 4;

          index += (4*indexColInc); 
        }
      } 
    }
  }
  else
  {
    MSG_LOW("ipl_reflect marker_204\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_reflect marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_reflect */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycbcr420lp_to_rgb

DESCRIPTION
  This functin will accept YCbCr 4:2:0 line pack as input and output RGB
  as a double word as follows.  

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  cropin:         Crop in input  (can be NULL)
  cropout:        Crop in output (can be NULL)

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr: Pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_crop_ycbcr420lp_to_rgb
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* cropin, 
  ipl_rect_type* cropout
)
{
  uint16* data_out;
  uint16* data2_out;
  uint8* y_ptr;
  uint8* yr2_ptr;
  uint8* c_ptr;


  uint32 row,col;
  ipl_rect_type cin, cout;
  int32 pitchi, pitcho;
  register int32 cb, cr, r_factor;
  register int32 luma1, luma2, luma3, luma4;
  register uint16 out, out2, out3, out4;
  register int32 r;
  const uint16 *rTable;
  const uint16 *gTable;
  const uint16 *bTable;

  MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_0\n");

  if (!input_img_ptr || !input_img_ptr->imgPtr || 
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_200\n");
    return IPL_FAILURE;
  }


  // initialze color tables
  if ( ipl2_init() != IPL_SUCCESS )
  {
    MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_201\n");
    return IPL_FAILURE;
  }

  /*
  **  Verify Input is either IPL_YCrCb or IPL_RGB565
  */
  if (input_img_ptr->cFormat != IPL_YCbCr420_LINE_PK)
  {
    MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_202\n");
    return IPL_FAILURE;
  }


  if (cropin == NULL)
  {
    cropin= &cin;
    cropin->x = 0;
    cropin->y = 0;
    cropin->dx = input_img_ptr->dx;
    cropin->dy = input_img_ptr->dy;
  }

  if (cropout == NULL)
  {
    cropout = &cout;
    cropout->x = 0;
    cropout->y = 0;
    cropout->dx = output_img_ptr->dx;
    cropout->dy = output_img_ptr->dy;
  }


  if (cropout->dx % 2 || cropin->dx % 2)
  {
    MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_203\n");
    return IPL_FAILURE;
  }

  if (cropout->dy % 2 || cropin->dy % 2)
  {
    MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_204\n");
    return IPL_FAILURE;
  }


  /* crop dimensions must be same size */
  if ((cropin->dx != cropout->dx) || (cropin->dy != cropout->dy))
  {
    MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_205\n");
    return IPL_FAILURE;
  }

  /*
  ** Verify the output is RGB565
  */
  if ((cropout->x + cropout->dx > output_img_ptr->dx) ||
      (cropout->y + cropout->dy > output_img_ptr->dy))
  {
    MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_206\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_1\n");

  // setup table for doing color conversion
  switch (output_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_207\n");
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );

    default:
      MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_208\n");
      return( IPL_FAILURE );
  }

  // setup output pointers, two rows at a time
  data_out = (uint16 *) output_img_ptr->imgPtr;
  data_out += (cropout->x + output_img_ptr->dx*cropout->y);
  data2_out = data_out + output_img_ptr->dx;

  // setup input pointers, two rows at a time
  y_ptr = input_img_ptr->imgPtr;
  y_ptr += (cropin->x + input_img_ptr->dx*cropin->y);
  yr2_ptr = y_ptr + input_img_ptr->dx;
  c_ptr = input_img_ptr->clrPtr + (cropin->x+input_img_ptr->dx*(cropin->y/2));

  /* make sure first Chroma is Cr if starting on odd column */
  if (cropin->x%2)
    c_ptr--;

  // computer pitch 
  pitcho = output_img_ptr->dx + (output_img_ptr->dx - cropout->dx);
  pitchi = input_img_ptr->dx + (input_img_ptr->dx - cropin->dx);

  /* Now loop through the image once */
  for(row = cropin->dy; row; row -= 2)
  {
    for(col = cropin->dx; col; col -= 2)
    {
      /*
      ** Work on 4 pixels at one time
      */
      luma1 = *y_ptr++;
      luma2 = *y_ptr++;
      luma3 = *yr2_ptr++;
      luma4 = *yr2_ptr++;
      cb = *c_ptr++;
      cr = *c_ptr++;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD(luma1, luma2, luma3, \
       luma4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
       rTable, gTable, bTable );

      // write output
      *data_out++  = out;
      *data_out++  = out2;
      *data2_out++ = out3;
      *data2_out++ = out4;
    }

    y_ptr += pitchi;
    yr2_ptr += pitchi;
    c_ptr += (input_img_ptr->dx - cropin->dx);

    data_out += pitcho;
    data2_out += pitcho;
  }

  MSG_LOW("ipl_crop_ycbcr420lp_to_rgb marker_100\n");
  return IPL_SUCCESS;
} 






/* <EJECT> */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycrcb420lp_to_rgb

DESCRIPTION
  This functin will accept YCrCb 4:2:0 line pack as input and output RGB
  as a double word as follows.  

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  cropin:         Crop in input  (can be NULL)
  cropout:        Crop in output (can be NULL)

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr: Pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_crop_ycrcb420lp_to_rgb
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* cropin, 
  ipl_rect_type* cropout
)
{
  uint16* data_out;
  uint16* data2_out;
  uint8* y_ptr;
  uint8* yr2_ptr;
  uint8* c_ptr;


  uint32 row,col;
  ipl_rect_type cin, cout;
  int32 pitchi, pitcho;
  register int32 cb, cr, r_factor;
  register int32 luma1, luma2, luma3, luma4;
  register uint16 out, out2, out3, out4;
  register int32 r;
  const uint16 *rTable;
  const uint16 *gTable;
  const uint16 *bTable;

  MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_0\n");

  if (!input_img_ptr || !input_img_ptr->imgPtr || 
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_200\n");
    return IPL_FAILURE;
  }
  
  // initialze color tables
  if ( ipl2_init() != IPL_SUCCESS )
  {
    MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_201\n");
    return IPL_FAILURE;
  }

  /*
  **  Verify Input is either IPL_YCrCb or IPL_RGB565
  */
  if (input_img_ptr->cFormat != IPL_YCrCb420_LINE_PK)
  {
    MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_202\n");
    return IPL_FAILURE;
  }

  if (cropin == NULL)
  {
    cropin= &cin;
    cropin->x = 0;
    cropin->y = 0;
    cropin->dx = input_img_ptr->dx;
    cropin->dy = input_img_ptr->dy;
  }

  if (cropout == NULL)
  {
    cropout = &cout;
    cropout->x = 0;
    cropout->y = 0;
    cropout->dx = output_img_ptr->dx;
    cropout->dy = output_img_ptr->dy;
  }


  if (cropout->dx % 2 || cropin->dx % 2)
  {
    MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_203\n");
    return IPL_FAILURE;
  }
  if (cropout->dy % 2 || cropin->dy % 2)
  {
    MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_204\n");
    return IPL_FAILURE;
  }

  /* crop dimensions must be same size */
  if ((cropin->dx != cropout->dx) || (cropin->dy != cropout->dy))
  {
    MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_205\n");
    return IPL_FAILURE;
  }

  /*
  ** Verify the output is RGB565
  */
  if ((cropout->x + cropout->dx > output_img_ptr->dx) ||
      (cropout->y + cropout->dy > output_img_ptr->dy))
  {
    MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_206\n");
    return IPL_FAILURE;
  }


  MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_1\n");

  // setup table for doing color conversion
  switch (output_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_207\n");

      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );

    default:
      MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_208\n");
      return( IPL_FAILURE );
  }

  // setup output pointers, two rows at a time
  data_out = (uint16 *) output_img_ptr->imgPtr;
  data_out += (cropout->x + output_img_ptr->dx*cropout->y);
  data2_out = data_out + output_img_ptr->dx;

  // setup input pointers, two rows at a time
  y_ptr = input_img_ptr->imgPtr;
  y_ptr += (cropin->x + input_img_ptr->dx*cropin->y);
  yr2_ptr = y_ptr + input_img_ptr->dx;
  c_ptr = input_img_ptr->clrPtr + (cropin->x+input_img_ptr->dx*(cropin->y/2));

  /* make sure first Chroma is Cr if starting on odd column */
  if (cropin->x%2)
    c_ptr--;

  // computer pitch 
  pitcho = output_img_ptr->dx + (output_img_ptr->dx - cropout->dx);
  pitchi = input_img_ptr->dx + (input_img_ptr->dx - cropin->dx);

  /* Now loop through the image once */
  for(row = cropin->dy; row; row -= 2)
  {
    for(col = cropin->dx; col; col -= 2)
    {
      /*
      ** Work on 4 pixels at one time
      */
      luma1 = *y_ptr++;
      luma2 = *y_ptr++;
      luma3 = *yr2_ptr++;
      luma4 = *yr2_ptr++;
      cr = *c_ptr++;
      cb = *c_ptr++;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD(luma1, luma2, luma3, \
       luma4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
       rTable, gTable, bTable );

      // write output
      *data_out++  = out;
      *data_out++  = out2;
      *data2_out++ = out3;
      *data2_out++ = out4;
    }

    y_ptr += pitchi;
    yr2_ptr += pitchi;
    c_ptr += (input_img_ptr->dx - cropin->dx);

    data_out += pitcho;
    data2_out += pitcho;
  }

  // using one pointer proves slower
#if 0
  /* Now loop through the image once */
  for(row = cropin->dy; row; row -= 2)
  {
    for(col = cropin->dx; col; col -= 2)
    {
      /*
      ** Work on 4 pixels at one time
      */
      luma1 = *(y_ptr);
      luma3 = *(y_ptr+idx);
      y_ptr++;
      luma2 = *(y_ptr);
      luma4 = *(y_ptr + idx);
      y_ptr++;
      cr = *c_ptr++;
      cb = *c_ptr++;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD(luma1, luma2, luma3, \
       luma4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
       rTable, gTable, bTable );

      // write output
      *(data_out ) = out;
      *(data_out + odx) = out3;
      data_out++;
      *(data_out) = out2;
      *(data_out + odx) = out4;
      data_out++;
    }

    y_ptr += pitchi;
    c_ptr += (input_img_ptr->dx - cropin->dx);
    data_out += pitcho;
  }
#endif

  MSG_LOW("ipl_crop_ycrcb420lp_to_rgb marker_100\n");
  return IPL_SUCCESS;
} 






/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycbcr422lp_to_rgb

DESCRIPTION
  This functin will accept YCrCb 4:2:0 line pack as input and output RGB
  as a double word as follows.  

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  cropin:         Crop in input  (can be NULL)
  cropout:        Crop in output (can be NULL)

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr: Pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_crop_ycbcr422lp_to_rgb
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* cropin, 
  ipl_rect_type* cropout
)
{
  uint16* data_out;
  uint8* y_ptr;
  uint8* c_ptr;


  uint32 row,col;
  ipl_rect_type cin, cout;
  int32 pitchi, pitcho;
  register int32 cb, cr;
  register int32 luma1, luma2;
  register uint16 out, out2;
  register int32 r;
  const uint16 *rTable;
  const uint16 *gTable;
  const uint16 *bTable;


  MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_0\n");

  if (!input_img_ptr || !input_img_ptr->imgPtr || 
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_200\n");
    return IPL_FAILURE;
  }


  // initialze color tables
  if ( ipl2_init() != IPL_SUCCESS )
  {
    MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_201\n");
    return IPL_FAILURE;
  }


  if (input_img_ptr->cFormat != IPL_YCbCr422_LINE_PK)
  {
    MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_202\n");
    return IPL_FAILURE;
  }

  if (cropin == NULL)
  {
    cropin= &cin;
    cropin->x = 0;
    cropin->y = 0;
    cropin->dx = input_img_ptr->dx;
    cropin->dy = input_img_ptr->dy;
  }

  if (cropout == NULL)
  {
    cropout = &cout;
    cropout->x = 0;
    cropout->y = 0;
    cropout->dx = output_img_ptr->dx;
    cropout->dy = output_img_ptr->dy;
  }

  if (cropout->dx % 2 || cropin->dx % 2)
  {
    MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_203\n");
    return IPL_FAILURE;
  }


  /* crop dimensions must be same size */
  if ((cropin->dx != cropout->dx) || (cropin->dy != cropout->dy))
  {
    MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_204\n");
    return IPL_FAILURE;
  }

  /*
  ** Verify the output is RGB565
  */
  if ((cropout->x + cropout->dx > output_img_ptr->dx) ||
      (cropout->y + cropout->dy > output_img_ptr->dy))
  {
    MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_205\n");
    return IPL_FAILURE;
  }


  MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_1\n");
  // setup table for doing color conversion
  switch (output_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_206\n");
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );

    default:
      MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_207\n");
      return( IPL_FAILURE );
  }

  // setup output pointers, two rows at a time
  data_out = (uint16 *) output_img_ptr->imgPtr;
  data_out += (cropout->x + output_img_ptr->dx*cropout->y);

  // setup input pointers, two rows at a time
  y_ptr = input_img_ptr->imgPtr;
  y_ptr += (cropin->x + input_img_ptr->dx*cropin->y);
  c_ptr = input_img_ptr->clrPtr + (cropin->x+input_img_ptr->dx*(cropin->y));

  /* make sure first Chroma is Cr if starting on odd column */
  if (cropin->x%2)
    c_ptr--;

  // computer pitch 
  pitcho = (output_img_ptr->dx - cropout->dx);
  pitchi = (input_img_ptr->dx - cropin->dx);

  /* Now loop through the image once */
  for(row = cropin->dy; row; row--)
  {
    for(col = cropin->dx; col; col-=2)
    {
      /*
      ** Work on 4 pixels at one time
      */
      luma1 = *y_ptr++;
      luma2 = *y_ptr++;
      cb = *c_ptr++;
      cr = *c_ptr++;

      IPL2_CONVERT_YCBCR_RGB(luma1, luma2, cr, cb, r, out, out2, \
                             rTable, gTable, bTable);

      // write output
      *data_out++  = out;
      *data_out++  = out2;
    }

    y_ptr += pitchi;
    c_ptr += pitchi;
    data_out += pitcho;
  }
  
  MSG_LOW("ipl_crop_ycbcr422lp_to_rgb marker_100\n");
  return IPL_SUCCESS;
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycrcb422lp_to_rgb

DESCRIPTION
  This functin will accept YCrCb 4:2:0 line pack as input and output RGB
  as a double word as follows.  

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  cropin:         Crop in input  (can be NULL)
  cropout:        Crop in output (can be NULL)

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr: Pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_crop_ycrcb422lp_to_rgb
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* cropin, 
  ipl_rect_type* cropout
)
{
  uint16* data_out;
  uint8* y_ptr;
  uint8* c_ptr;


  uint32 row,col;
  ipl_rect_type cin, cout;
  int32 pitchi, pitcho;
  register int32 cb, cr;
  register int32 luma1, luma2;
  register uint16 out, out2;
  register int32 r;
  const uint16 *rTable;
  const uint16 *gTable;
  const uint16 *bTable;

  MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_0\n");

  if (!input_img_ptr || !input_img_ptr->imgPtr || 
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_200\n");
    return IPL_FAILURE;
  }

  // initialze color tables
  if ( ipl2_init() != IPL_SUCCESS )
  {
    MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_201\n");
    return IPL_FAILURE;
  }


  if (input_img_ptr->cFormat != IPL_YCrCb422_LINE_PK)
  {
    MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_202\n");
    return IPL_FAILURE;
  }


  if (cropin == NULL)
  {
    cropin= &cin;
    cropin->x = 0;
    cropin->y = 0;
    cropin->dx = input_img_ptr->dx;
    cropin->dy = input_img_ptr->dy;
  }

  if (cropout == NULL)
  {
    cropout = &cout;
    cropout->x = 0;
    cropout->y = 0;
    cropout->dx = output_img_ptr->dx;
    cropout->dy = output_img_ptr->dy;
  }

  if (cropout->dx % 2 || cropin->dx % 2)
  {
    MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_203\n");
    return IPL_FAILURE;
  }


  /* crop dimensions must be same size */
  if ((cropin->dx != cropout->dx) || (cropin->dy != cropout->dy))
  {
    MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_204\n");
    return IPL_FAILURE;
  }

  /*
  ** Verify the output is RGB565
  */
  if ((cropout->x + cropout->dx > output_img_ptr->dx) ||
      (cropout->y + cropout->dy > output_img_ptr->dy))
  {
    MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_205\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_1\n");
  // setup table for doing color conversion
  switch (output_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_206\n");
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );

    default:
      MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_207\n");
      return( IPL_FAILURE );
  }

  // setup output pointers, two rows at a time
  data_out = (uint16 *) output_img_ptr->imgPtr;
  data_out += (cropout->x + output_img_ptr->dx*cropout->y);

  // setup input pointers, two rows at a time
  y_ptr = input_img_ptr->imgPtr;
  y_ptr += (cropin->x + input_img_ptr->dx*cropin->y);
  c_ptr = input_img_ptr->clrPtr + (cropin->x+input_img_ptr->dx*(cropin->y));

  /* make sure first Chroma is Cr if starting on odd column */
  if (cropin->x%2)
    c_ptr--;

  // computer pitch 
  pitcho = (output_img_ptr->dx - cropout->dx);
  pitchi = (input_img_ptr->dx - cropin->dx);

  /* Now loop through the image once */
  for(row = cropin->dy; row; row--)
  {
    for(col = cropin->dx; col; col-=2)
    {
      /*
      ** Work on 4 pixels at one time
      */
      luma1 = *y_ptr++;
      luma2 = *y_ptr++;
      cr = *c_ptr++;
      cb = *c_ptr++;

      IPL2_CONVERT_YCBCR_RGB(luma1, luma2, cr, cb, r, out, out2, \
                             rTable, gTable, bTable);

      // write output
      *data_out++  = out;
      *data_out++  = out2;
    }

    y_ptr += pitchi;
    c_ptr += pitchi;
    data_out += pitcho;
  }

  MSG_LOW("ipl_crop_ycrcb422lp_to_rgb marker_100\n");
  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycbcr_to_rgb

DESCRIPTION
  This functin will accept YCbCr 4:2:2 as input and output RGB
  as a double word as follows.  

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  cropin:         Crop in input  (can be NULL)
  cropout:        Crop in output (can be NULL)

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr: Pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_crop_ycbcr_to_rgb
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* cropin, 
  ipl_rect_type* cropout
)
{
  uint16* data_out;
  uint8* y_ptr;


  uint32 row,col;
  ipl_rect_type cin, cout;
  int32 pitchi, pitcho;
  register int32 cb, cr;
  register int32 luma1, luma2;
  register uint16 out, out2;
  register int32 r;
  const uint16 *rTable;
  const uint16 *gTable;
  const uint16 *bTable;

  MSG_LOW("ipl_crop_ycbcr_to_rgb marker_0\n");

  if (!input_img_ptr || !input_img_ptr->imgPtr || 
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_ycbcr_to_rgb marker_200\n");
    return IPL_FAILURE;
  }

  // initialze color tables
  if ( ipl2_init() != IPL_SUCCESS )
  {
    MSG_LOW("ipl_crop_ycbcr_to_rgb marker_201\n");
    return IPL_FAILURE;
  }


  if (input_img_ptr->cFormat != IPL_YCbCr)
  {
    MSG_LOW("ipl_crop_ycbcr_to_rgb marker_202\n");
    return IPL_FAILURE;
  }


  if (cropin == NULL)
  {
    cropin= &cin;
    cropin->x = 0;
    cropin->y = 0;
    cropin->dx = input_img_ptr->dx;
    cropin->dy = input_img_ptr->dy;
  }

  if (cropout == NULL)
  {
    cropout = &cout;
    cropout->x = 0;
    cropout->y = 0;
    cropout->dx = output_img_ptr->dx;
    cropout->dy = output_img_ptr->dy;
  }

  if (cropout->dx % 2 || cropin->dx % 2)
  {
    MSG_LOW("ipl_crop_ycbcr_to_rgb marker_203\n");
    return IPL_FAILURE;
  }


  /* crop dimensions must be same size */
  if ((cropin->dx != cropout->dx) || (cropin->dy != cropout->dy))
  {
    MSG_LOW("ipl_crop_ycbcr_to_rgb marker_204\n");
    return IPL_FAILURE;
  }

  /*
  ** Verify the output is RGB565
  */
  if ((cropout->x + cropout->dx > output_img_ptr->dx) ||
      (cropout->y + cropout->dy > output_img_ptr->dy))
  {
    MSG_LOW("ipl_crop_ycbcr_to_rgb marker_205\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_crop_ycbcr_to_rgb marker_1\n");

  // setup table for doing color conversion
  switch (output_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      MSG_LOW("ipl_crop_ycbcr_to_rgb marker_206\n");
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );

    default:
      MSG_LOW("ipl_crop_ycbcr_to_rgb marker_207\n");
      return( IPL_FAILURE );
  }

  // setup output pointers, two rows at a time
  data_out = (uint16 *) output_img_ptr->imgPtr;
  data_out += (cropout->x + output_img_ptr->dx*cropout->y);

  // setup input pointers, two rows at a time
  y_ptr = input_img_ptr->imgPtr;
  y_ptr += (2*cropin->x + 2*input_img_ptr->dx*cropin->y);

  /* make sure go to the Chroma of this group of pixels */
  if (cropin->x%2)
    y_ptr -= 2;

  // computer pitch 
  pitcho = (output_img_ptr->dx - cropout->dx);
  pitchi = 2*(input_img_ptr->dx - cropin->dx);

  /* Now loop through the image once */
  for(row = cropin->dy; row; row--)
  {
    for(col = cropin->dx; col; col-=2)
    {
      /*
      ** Work on 4 pixels at one time
      */
      cb = *y_ptr++;
      luma1 = *y_ptr++;
      cr = *y_ptr++;
      luma2 = *y_ptr++;

      IPL2_CONVERT_YCBCR_RGB(luma1, luma2, cr, cb, r, out, out2, \
                             rTable, gTable, bTable);

      // write output
      *data_out++  = out;
      *data_out++  = out2;
    }

    y_ptr += pitchi;
    data_out += pitcho;
  }

  MSG_LOW("ipl_crop_ycbcr_to_rgb marker_100\n");
  return IPL_SUCCESS;
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_YCbCrToRGB

DESCRIPTION
  This functin will accept YCbCr 4:2:0 as input and output RGB666
  or RGB565, or YCrCb 420 and output RGB 565. The output is a double word 
  as follows:  The Most Significant 14 bits are 0, followed by 6 bits of R, 
  then G and then B.

  Input should be 4:2:0 line packed Format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  cropin          Ared in input to crop (can be NULL)
  cropout         Where to put in output (must have same dim as input crop)
                  Can be NULL.

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr: Pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_crop_YCbCrToRGB
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* cropin,
  ipl_rect_type* cropout
)
{
  ipl_status_type status;

  MSG_LOW("ipl_crop_YCbCrToRGB marker_0\n");

  if (!input_img_ptr || !input_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_crop_YCbCrToRGB marker_200\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_crop_YCbCrToRGB marker_1\n");

  if (input_img_ptr->cFormat == IPL_YCbCr)
    status = ipl_crop_ycbcr_to_rgb(input_img_ptr, output_img_ptr, cropin, cropout);
  else if (input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
    status = ipl_crop_ycrcb420lp_to_rgb(input_img_ptr, output_img_ptr, cropin, cropout);
  //else if (input_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)
  //  status = ipl_crop_ycbcr420lp_to_rgb(input_img_ptr, output_img_ptr, cropin, cropout);
  //else if (input_img_ptr->cFormat == IPL_YCbCr422_LINE_PK)
  //  status = ipl_crop_ycbcr422lp_to_rgb(input_img_ptr, output_img_ptr, cropin, cropout);
  else if (input_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
    status = ipl_crop_ycrcb422lp_to_rgb(input_img_ptr, output_img_ptr, cropin, cropout);
  else
    status = IPL_FAILURE;

  if (status == IPL_FAILURE)
    MSG_LOW("ipl_crop_YCbCrToRGB marker_201\n");
  else
    MSG_LOW("ipl_crop_YCbCrToRGB marker_100\n");

  return status;
}


/*===========================================================================

FUNCTION ipl_rotate_rgb565

DESCRIPTION
  This function performs arbitrary rotation. The user must specify 
  the angle of rotation in degrees and the pixel value used to fill 
  in blank regions. Any part of the input that is rotated outside
  image boundaries is cut off. Negative angles produce clockwise 
  rotation. Positive angles produce counterclockwise rotation. Angles 
  can be any integer between positive and negative infinity.
  
  Input and output image sizes must be equal.
  
  Inputs and output must be in RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr     pointer to the input image
  o_img_ptr     pointer to the output image
  angle         angle of rotation in degrees
  fillerPixel   filler pixel value

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_rotate_rgb565
(
  ipl_image_type* i_img_ptr,   /* Points to the input image           */
  ipl_image_type* o_img_ptr,   /* Points to the output image          */
  int16 angle,                 /* Angle of rotation in degrees        */
  uint16 fillerPixel           /* Pixel value to fill in blank spaces */
)
{
  uint32 radian;
  int8 sinSign, cosSign;
  int32 sintheta, costheta;
  uint32 outWidth, outHeight, xout, yout, xin, yin, xo, yo, beta;

  MSG_LOW("ipl_rotate_rgb565 marker_0\n");


  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_rotate_rgb565 marker_200\n");
    return IPL_FAILURE;
  }  
  /* Initialize image pointers and local variables */

  outWidth = (uint16) o_img_ptr->dx;
  outHeight = (uint16) o_img_ptr->dy;

  xo = outWidth >> 1;
  yo = outHeight >> 1;

  sinSign = 1;
  cosSign = 1;

  MSG_LOW("ipl_rotate_rgb565 marker_1\n");

  while (angle < 0) 
    angle += 360;

  angle %= 360;

  if (angle <= 90) 
  {
    beta = angle;
  } 
  else if (angle <= 180) 
  {
    beta = 180 - angle;
    cosSign = -1;
  } 
  else if (angle <= 270) 
  {
    beta = angle - 180;
    sinSign = -1;
    cosSign = -1;
  } 
  else 
  {
    beta = 360 - angle;
    sinSign = -1;
  }

  /* angle in radians */
  radian = beta * 9;
  if (radian <= 360) 
  {
    sintheta = ((radian*(262144-(radian*radian)/6))>>8)*sinSign;
    costheta = (524288-radian*radian)*cosSign;
  } 
  else 
  {
    if (radian == 810) 
    {
      sintheta = 524288*sinSign;
      costheta = 0;
    } 
    else 
    {
      sintheta = (((radian*(262144-(radian*radian)/6))>>8) +
                 (((((((((radian*radian*radian)/120)>>3)*radian)>>10)*
                 radian)/42)*((11010048-radian*radian)>>10))>>21))*sinSign;

      costheta = ((524288-radian*radian) +
                 (((((((radian*radian*radian)/24)>>3)*radian/30)>>10)*
                 ((7864320-radian*radian)>>10))>>12))*cosSign;
    }
  }

  if (i_img_ptr->cFormat == IPL_RGB565)
  {
    uint16 *inImgPtr, *outImgPtr;
    inImgPtr = (uint16*) i_img_ptr->imgPtr;
    outImgPtr = (uint16*) o_img_ptr->imgPtr;

    for(yout = 0; yout < outHeight; yout++) 
    {
      for(xout = 0; xout < outWidth; xout++) 
      {
        xin = (uint16)(xo+(((int32)(xout-xo)*costheta+
                          (int32)(yout-yo)*sintheta)>>19));
        yin = (uint16)(yo+(((int32)(xo-xout)*sintheta+
                          (int32)(yout-yo)*costheta)>>19));
        if (xin < outWidth && yin < outHeight) 
        {
          *outImgPtr++ = *((uint16*)(inImgPtr + yin*outWidth + xin));
        } 
        else 
        {
          *outImgPtr++ = fillerPixel;
        } 
      } 
    }
  }
  else if (i_img_ptr->cFormat == IPL_RGB888)
  {
    uint8 *inImgPtr, *outImgPtr;
    uint8 fillerPixelR = 0;
    uint8 fillerPixelG = 0;
    uint8 fillerPixelB = 0;

    inImgPtr = i_img_ptr->imgPtr;
    outImgPtr = o_img_ptr->imgPtr;


    // unpack the filler pixel
    unpack_rgb565(fillerPixel, &fillerPixelR, &fillerPixelG, &fillerPixelB);

    for(yout = 0; yout < outHeight; yout++) 
    {
      for(xout = 0; xout < outWidth; xout++) 
      {
        xin = (uint16)(xo+(((int32)(xout-xo)*costheta+
                          (int32)(yout-yo)*sintheta)>>19));
        yin = (uint16)(yo+(((int32)(xo-xout)*sintheta+
                          (int32)(yout-yo)*costheta)>>19));

        if (xin < outWidth && yin < outHeight) 
        {
          *outImgPtr++ = *(inImgPtr + 3*yin*outWidth + 3*xin);
          *outImgPtr++ = *(inImgPtr + 3*yin*outWidth + 3*xin + 1);
          *outImgPtr++ = *(inImgPtr + 3*yin*outWidth + 3*xin + 2);
        }
        else 
        {
          *outImgPtr++ = fillerPixelR;
          *outImgPtr++ = fillerPixelG;
          *outImgPtr++ = fillerPixelB;
        }
      } 
    }
  }
  else
  {
    MSG_LOW("ipl_rotate_rgb565 marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_rotate_rgb565 marker_100\n");
  return IPL_SUCCESS;

} /* End ipl_rotate_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_rotate_ycbcr

DESCRIPTION
  This function performs arbitrary rotation. The user must specify 
  the angle of rotation in degrees and the pixel value used to fill 
  in blank regions. Any part of the input that is rotated outside
  image boundaries is cut off. Negative angles produce clockwise 
  rotation. Positive angles produce counterclockwise rotation. Angles 
  can be any integer between positive and negative infinity.
  
  Input and output image sizes must be equal.
  
  Inputs and output must be in YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr     pointer to the input image
  o_img_ptr     pointer to the output image
  angle         angle of rotation in degrees
  fillerPixel   filler pixel value

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_rotate_ycbcr
(
  ipl_image_type* i_img_ptr,   /* Points to the input image           */
  ipl_image_type* o_img_ptr,   /* Points to the output image          */
  int16 angle,                 /* Angle of rotation in degrees        */
  uint16 fillerPixel           /* Pixel value to fill in blank spaces */
)
{
  unsigned char *inImgPtr, *outImgPtr, *fullImgPtr;
  unsigned char cb, cr, luma1, luma2;
  uint32 outWidth, outHeight, xout, yout, xin, yin, xo, yo, beta;
  uint32 index, radian;
  int8 sinSign, cosSign;
  int32 sintheta, costheta;

  MSG_LOW("ipl_rotate_ycbcr marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_rotate_ycbcr marker_200\n");
    return IPL_FAILURE;
  }  

  /* Initialize image pointers and local variables */
  inImgPtr = i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;
  outWidth = (uint16) o_img_ptr->dx;
  outHeight = (uint16) o_img_ptr->dy;
  fullImgPtr = ipl_malloc(outWidth*outHeight*3*sizeof(unsigned char));
  if (!fullImgPtr) 
  {
    MSG_LOW("ipl_rotate_ycbcr marker_201\n");
    return IPL_NO_MEMORY;
  }
  xo = outWidth >> 1;
  yo = outHeight >> 1;
  sinSign = 1;
  cosSign = 1;

  MSG_LOW("ipl_rotate_ycbcr marker_1\n");
  /* Get the equivalent angle in the range (-360, 360) degrees */
  while (angle < 0) {
    angle += 360;
  }
  angle %= 360;
  /* 
  ** Get the equivalent angle in the range [0, 90] degrees
  ** and set the sine and cosines signs accordingly 
  */
  if (angle <= 90) {
    beta = angle;
  } else if (angle <= 180) {
    beta = 180 - angle;
    cosSign = -1;
  } else if (angle <= 270) {
    beta = angle - 180;
    sinSign = -1;
    cosSign = -1;
  } else {
    beta = 360 - angle;
    sinSign = -1;
  }
  /* Angle in radians */
  radian = beta * 9;
  if (radian <= 360) {
    sintheta = ((radian*(262144-(radian*radian)/6))>>8)*sinSign;
    costheta = (524288-radian*radian)*cosSign;
  } else {
    if (radian == 810) {
      sintheta = 524288*sinSign;
      costheta = 0;
    } else {
      sintheta = (((radian*(262144-(radian*radian)/6))>>8) +
                 (((((((((radian*radian*radian)/120)>>3)*radian)>>10)*
                 radian)/42)*((11010048-radian*radian)>>10))>>21))*sinSign;
      costheta = ((524288-radian*radian) +
                 (((((((radian*radian*radian)/24)>>3)*radian/30)>>10)*
                 ((7864320-radian*radian)>>10))>>12))*cosSign;
    }
  }

  /* Create full YCbCr image */
  index = 0;
  for (yout = 0; yout < outHeight; yout++) {
    for (xout = 0; xout < outWidth; xout+=2) {
      cb = *inImgPtr++;
      luma1 = *inImgPtr++;
      cr = *inImgPtr++;
      luma2 = *inImgPtr++;
      *((unsigned char*)(fullImgPtr+index++)) = cb;
      *((unsigned char*)(fullImgPtr+index++)) = cr;
      *((unsigned char*)(fullImgPtr+index++)) = luma1;
      *((unsigned char*)(fullImgPtr+index++)) = cb;
      *((unsigned char*)(fullImgPtr+index++)) = cr;
      *((unsigned char*)(fullImgPtr+index++)) = luma2;
    } /* end xout loop */
  } /* end yout loop */
  
  /* Do rotation */
  for (yout = 0; yout < outHeight; yout++) {
    for (xout = 0; xout < outWidth; xout++) {
      xin = (uint16)(xo+(((int32)(xout-xo)*costheta+
                          (int32)(yout-yo)*sintheta)>>19));
      yin = (uint16)(yo+(((int32)(xo-xout)*sintheta+
                          (int32)(yout-yo)*costheta)>>19));
      if (xin < outWidth && yin < outHeight) {
        index = (xin + (uint32)yin*outWidth)*3;
        if (xout%2) {
          /* get Cr */
          *outImgPtr++ = *((unsigned char*)(fullImgPtr+index+1));
          *outImgPtr++ = *((unsigned char*)(fullImgPtr+index+2));
        } else {
          /* get Cb */
          *outImgPtr++ = *((unsigned char*)(fullImgPtr+index));
          *outImgPtr++ = *((unsigned char*)(fullImgPtr+index+2));
        }
      } else {
        /* blank area */
        *outImgPtr++ = (fillerPixel&0xFF00)>>8;
        *outImgPtr++ = fillerPixel&0x00FF;
      }
    } /* end xout loop */
  } /* end yout loop */

  /* sys_free memory */
  ipl_sys_free(fullImgPtr);

  MSG_LOW("ipl_rotate_ycbcr marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_rotate_ycbcr */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_rotate

DESCRIPTION
  This function performs arbitrary rotation. The user must specify 
  the angle of rotation in degrees and the pixel value used to fill 
  in blank regions. Any part of the input that is rotated outside
  image boundaries is cut off. Only rotations of 0, 90, 180, and 270 
  degrees do not cut off any part of the input image. Negative angles 
  produce clockwise rotation. Positive angles produce counterclockwise 
  rotation. Angles can be any integer between positive and negative 
  infinity.
  
  Input and output image sizes must be equal.
  
  Inputs and output must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr     pointer to the input image
  o_img_ptr     pointer to the output image
  angle         angle of rotation in degrees
  fillerPixel   filler pixel value

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_rotate
(
  ipl_image_type* i_img_ptr,   /* Points to the input image           */
  ipl_image_type* o_img_ptr,   /* Points to the output image          */
  int16 angle,                 /* Angle of rotation in degrees        */
  uint16 fillerPixel 
)
{
  ipl_status_type retval = IPL_FAILURE;

  MSG_LOW("ipl_rotate marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_rotate marker_200\n");
    return IPL_FAILURE;
  }
  
  /* get equivalent positive angle, less than 360 */
  while (angle < 0) angle += 360;
    angle %= 360;

  MSG_LOW("ipl_rotate marker_1\n");

  if (i_img_ptr->cFormat == IPL_YCbCr444)
  {
    ipl_image_type tmpYin, tmpYout;

    tmpYin.cFormat = IPL_YCbCr;
    tmpYin.dx = i_img_ptr->dx;
    tmpYin.dy = i_img_ptr->dy;
    if (ipl_malloc_img(&tmpYin))
    {
      MSG_LOW("ipl_rotate marker_201\n");
      return IPL_NO_MEMORY;
    }
    if (ipl_convert_image(i_img_ptr, &tmpYin)) 
    {
      ipl_free_img(&tmpYin);
      MSG_LOW("ipl_rotate marker_202\n");
      return IPL_FAILURE;
    }

    tmpYout.cFormat = IPL_YCbCr;
    tmpYout.dx = o_img_ptr->dx;
    tmpYout.dy = o_img_ptr->dy;
    if (ipl_malloc_img(&tmpYout))
    {
      ipl_free_img(&tmpYin);
      MSG_LOW("ipl_rotate marker_203\n");
      return IPL_NO_MEMORY;
    }

    /* call ycbcr 4:2:2 function */
    if (ipl_rotate(&tmpYin, &tmpYout, angle, fillerPixel))
    {
      ipl_free_img(&tmpYin);
      ipl_free_img(&tmpYout);
      MSG_LOW("ipl_rotate marker_204\n");
      return IPL_FAILURE;
    }

    /* get YCbCr 4:2:0 output */
    if (ipl_convert_image(&tmpYout, o_img_ptr))
    {
      ipl_free_img(&tmpYin);
      ipl_free_img(&tmpYout);
      MSG_LOW("ipl_rotate marker_205\n");
      return IPL_FAILURE;
    }

    /* sys_free memory */
    ipl_free_img(&tmpYin);
    ipl_free_img(&tmpYout);

    retval = IPL_SUCCESS;
  }
  else
  {
    switch (angle)
    {
      case 0:
        retval = ipl_copy_and_paste(i_img_ptr, o_img_ptr, NULL, NULL);
      break;

      case 90:
        retval = ipl_rotate90(i_img_ptr, o_img_ptr, IPL_ROT90);
      break;

      case 180:
        retval = ipl_rotate90(i_img_ptr, o_img_ptr, IPL_ROT180);
      break;

      case 270:
        retval = ipl_rotate90(i_img_ptr, o_img_ptr, IPL_ROT270);
      break;

      default:
        if (i_img_ptr->cFormat != o_img_ptr->cFormat)
        {
          MSG_LOW("ipl_rotate marker_206\n");
          return IPL_FAILURE;
        }

        if (i_img_ptr->dx != o_img_ptr->dx || i_img_ptr->dy != o_img_ptr->dy)
        {
          MSG_LOW("ipl_rotate marker_207\n");
          return IPL_FAILURE;
        }

        /* Call the appropriate function */
        if (i_img_ptr->cFormat == IPL_RGB565 || i_img_ptr->cFormat==IPL_RGB888)
          retval = ipl_rotate_rgb565(i_img_ptr, o_img_ptr, angle, fillerPixel);
        else if (i_img_ptr->cFormat == IPL_YCbCr) 
          retval = ipl_rotate_ycbcr(i_img_ptr, o_img_ptr, angle, fillerPixel);
        else 
        {
          ipl_image_type tmpYin, tmpYout;

          tmpYin.cFormat = IPL_YCbCr;
          tmpYin.dx = i_img_ptr->dx;
          tmpYin.dy = i_img_ptr->dy;
          if (ipl_malloc_img(&tmpYin))
          {
            MSG_LOW("ipl_rotate marker_201\n");
            return IPL_NO_MEMORY;
          }
          if (ipl_convert_image(i_img_ptr, &tmpYin)) 
          {
            ipl_free_img(&tmpYin);
            MSG_LOW("ipl_rotate marker_202\n");
            return IPL_FAILURE;
          }

          tmpYout.cFormat = IPL_YCbCr;
          tmpYout.dx = o_img_ptr->dx;
          tmpYout.dy = o_img_ptr->dy;
          if (ipl_malloc_img(&tmpYout))
          {
            ipl_free_img(&tmpYin);
            MSG_LOW("ipl_rotate marker_203\n");
            return IPL_NO_MEMORY;
          }

          /* call ycbcr 4:2:2 function */
          if (ipl_rotate(&tmpYin, &tmpYout, angle, fillerPixel))
          {
            ipl_free_img(&tmpYin);
            ipl_free_img(&tmpYout);
            MSG_LOW("ipl_rotate marker_204\n");
            return IPL_FAILURE;
          }

          /* get YCbCr 4:2:0 output */
          if (ipl_convert_image(&tmpYout, o_img_ptr))
          {
            ipl_free_img(&tmpYin);
            ipl_free_img(&tmpYout);
            MSG_LOW("ipl_rotate marker_205\n");
            return IPL_FAILURE;
          }

          /* sys_free memory */
          ipl_free_img(&tmpYin);
          ipl_free_img(&tmpYout);

          retval = IPL_SUCCESS;
        }
        break;
    }
  }

  if (retval == IPL_FAILURE)
    MSG_LOW("ipl_rotate marker_208\n");
  else
    MSG_LOW("ipl_rotate marker_100\n");

  return retval;
} /* End ipl_rotate */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_perspective_rgb565

DESCRIPTION
  This function creates a perspective image shaped like a trapezoid.  
  The user specifies 4 input parameters: (1) the length of the top 
  width of the trapezoid, (2) the x-coordinate of the starting point 
  of the top width, (3) a pixel value to fill in blank output regions 
  outside of the trapezoid, and (4) the orientation of the trapezoid. 
  The top width must be at least 1 pixel and no more than the width 
  of the input image. A long top width corresponds to a vanishing 
  point that is far away from the viewer or camera. A short top width 
  indicates a vanishing point near the camera. The filler pixel value 
  must be given in a format compatible with the output color format. For 
  instance, black is 0 for RGB565 and 0x8010 for YCbCr. White is 0xFFFF 
  for RGB565 and 0x80EB for YCbCr. Notice that for YCbCr, the chroma 
  value (Cb or Cr) comes before the luma value, so white is 0x80EB 
  instead of 0xEB80. The trapezoid’s height and bottom width are assumed 
  to be the input height and width for "up" and "down" orientations. 
  For "left" and "right" orientations, the trapezoid's height and bottom 
  width are the input width and height, respectively.

  The orientation of the trapezoid can be one of four values: 
  up ( 0), down (1), left (2), or right (3), as illustrated below.

  For any orientation, the top width is defined as the smaller 
  of the two parallel edges of the trapezoid.

  Input and output images must be in RGB565.

  ( 0) up:
         ----
        /    \
       /      \
       --------
  (1) down:
       --------
       \      /
        \    /
         ----
  (2) left:
        __ --- |
      |        |
      | __     |
           --- |
  (3) right:
      | --- __
      |        |
      |     __ |
      | ---


DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr     pointer to the input image
  out_img_ptr    pointer to the output image
  topWidth       top width of the trapezoidal output image
  xStart         starting x-coordinate of the top width
  fillerPixel    pixel value for the filler region
  orientation    orientation of the output image

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_perspective_rgb565
(
  ipl_image_type* in_img_ptr,   /* Points to the input image          */
  ipl_image_type* out_img_ptr,  /* Points to the output image         */
  uint32 topWidth,              /* Top width of trapezoidal output    */
  int32 xStart,                 /* Starting x-coordinate of top width */
  uint16 fillerPixel,           /* Pixel value for filler region      */
  uint8 orientation             /* Orientation of output image        */
)
{
  uint16 *rotImgPtr, *protImgPtr;
  uint32 *yMap;
  uint32 inIndex, outIndex;
  ipl_rotate90_type rot90 = IPL_NOROT;
  ipl_image_type rotImg;
  ipl_image_type protImg;
  int32 xout, yin, yprev, yout, y, c1, c2, c3, a, k, k2, xs, d, 
        xin, xend;
  uint32 w, h; 

  
  MSG_LOW("ipl_perspective_rgb565 marker_0\n");

  if (!in_img_ptr || !in_img_ptr->imgPtr || 
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_perspective_rgb565 marker_200\n");
    return IPL_FAILURE;
  }  

  //if ((in_img_ptr == NULL) || (out_img_ptr == NULL))
  //{
  //  MSG_LOW("ipl_perspective_rgb565 marker_201\n");
  //  return IPL_FAILURE;
  //}

  /* Initialize local variables */
  if (orientation == 0) 
  {
    /* up */
    w = out_img_ptr->dx;
    h = out_img_ptr->dy;
  } 
  else if (orientation == 1) 
  {
    /* down */
    rot90 = IPL_ROT180;
    w = out_img_ptr->dx;
    h = out_img_ptr->dy;
    xStart = w - xStart - topWidth;
  } 
  else if (orientation == 2) 
  {
    /* left */
    rot90 = IPL_ROT90;
    w = out_img_ptr->dy;
    h = out_img_ptr->dx;
    xStart = w - xStart - topWidth;
  } 
  else 
  {
    /* right */
    rot90 = IPL_ROT270;
    w = out_img_ptr->dy;
    h = out_img_ptr->dx;
  }

  MSG_LOW("ipl_perspective_rgb565 marker_1\n");

  if (orientation != 0) 
  {
    /* Initialize rotated image */

    rotImg.cFormat = IPL_RGB565;
    rotImg.dx = w;
    rotImg.dy = h;
    if (ipl_malloc_img(&rotImg))
    {
      MSG_LOW("ipl_perspective_rgb565 marker_202\n");
      return IPL_NO_MEMORY;
    }
    if (ipl_rotate90_frame(in_img_ptr, &rotImg, rot90, 0) != IPL_SUCCESS) 
    {
      ipl_free_img(&rotImg);
      MSG_LOW("ipl_perspective_rgb565 marker_203\n");
      return IPL_FAILURE;
    }
    rotImgPtr = (uint16*)rotImg.imgPtr;


    protImg.cFormat = IPL_RGB565;
    protImg.dx = w;
    protImg.dy = h;
    if (ipl_malloc_img(&protImg))
    {
      ipl_free_img(&rotImg);
      MSG_LOW("ipl_perspective_rgb565 marker_204\n");
      return IPL_NO_MEMORY;
    }
    protImgPtr = (uint16*) protImg.imgPtr;
  } 
  else 
  {
    rotImgPtr = (uint16*)in_img_ptr->imgPtr;
    protImgPtr = (uint16*)out_img_ptr->imgPtr;
  }

  if ((rotImgPtr == NULL) || (protImgPtr == NULL))
  {
    MSG_LOW("ipl_perspective_rgb565 marker_205\n");
    return IPL_FAILURE;
  }
  /* 
  ** Malloc yMap: mapping from output y-coordinates to input y-coordinates.
  ** Coordinate system is defined with the origin at the top left corner of 
  ** the image, with the positive x-axis extending to the right and the 
  ** positive y-axis extending downward.
  */
  yMap = ipl_malloc(h*sizeof(uint32));
  if (!yMap) 
  {   
    if (orientation != 0) 
    {
      ipl_free_img(&rotImg);
      ipl_free_img(&protImg);
    }
    MSG_LOW("ipl_perspective_rgb565 marker_206\n");
    return IPL_NO_MEMORY;
  }
  
  /* Initialize starting bin to zero */
  yMap[0] = 0;
  yprev = 0;
  a = h - 1;
  if (a == 0) a = 1;
  k2 = ((1054*(topWidth<<8))/w + 58777298) - 
       (((1922*(topWidth<<8)/w)<<8)/w*topWidth);

  /* Create yMap */
  for (yin = 1; yin < (int32) h; yin++) 
  {
    k = (k2+(int32)(1038*((a<<16)/((yin<<1)+a))))>>16;
    c1 = ((k*(a-yin))>>1)/a;
    c2 = (((c1*(a-yin))*k)/a)>>10;
    c3 = ((((c2*7*(a-yin))>>6)*k)/a)>>4;
    yout = (yin * (65536 + 89*c1 + 31*c2 + c3)) >> 16;
    if (yout >= (int32) h) 
    {
      yout = h - 1;
    }

    /* Assign mapping from output y-coordinate to input y-coordinate */
    yMap[yout] = yin;
    if (yout > (yprev + 1)) 
    {
      for (y = yprev + 1; y < yout; y++) 
      {
        yMap[y] = yin;
      }
    }
    yprev = yout;
  }

  /* Loop through input image */
  outIndex = 0;
  for (yout = 0; yout < (int32) h; yout++) 
  {
    /* Start and end coordinates for trapezoid region */
    xs = (((((xStart*(a-yout)))<<10)/a)>>10);
    xend = (w-1)-(((((w-topWidth-xStart)*(a-yout))<<10)/a)>>10);
    d = xend - xs;
    if (d == 0) 
    {
      d++;
    }
    k = ((w-1)<<10)/d;

    /* Left side outside of trapezoid */
    for (xout = 0; xout < xs; xout++) 
    {
      *((uint16*)(protImgPtr+outIndex++)) = fillerPixel;
    }

    /* Trapezoid */
    for (xout = xs; xout < xend; xout++) 
    {
      xin = ((xout-xs)*k)>>10;
      inIndex = xin + w*(a-yMap[a-yout]);
      *((uint16*)(protImgPtr+outIndex++)) = *((uint16*)(rotImgPtr+inIndex));
    }

    /* Right side outside trapezoid */
    for (xout = xend; xout < (int32) w; xout++) 
    {
      *((uint16*)(protImgPtr+outIndex++)) = fillerPixel;
    } /* end xout loop */
  } /* end yout loop */
  
  /* Rotate back to get output */
  if (orientation != 0) 
  {
    if (orientation == 1) 
    {
      /* down */
      rot90 = IPL_ROT180;
    } 
    else if (orientation == 2) 
    {
      /* left */
      rot90 = IPL_ROT270;
    } else 
    {
      /* right */
      rot90 = IPL_ROT90;
    }

    if (ipl_rotate90_frame(&protImg, out_img_ptr, rot90, 0) != IPL_SUCCESS) 
    {
      ipl_free_img(&rotImg);
      ipl_free_img(&protImg);
      ipl_sys_free(yMap);
      MSG_LOW("ipl_perspective_rgb565 marker_207\n");
      return IPL_FAILURE;
    }
  }
  
  /* Free memory */
  ipl_sys_free(yMap);
  if (orientation != 0) 
  {
    ipl_free_img(&rotImg);
    ipl_free_img(&protImg);
  }

  MSG_LOW("ipl_perspective_rgb565 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_perspective_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_perspective_ycbcr

DESCRIPTION
  This function creates a perspective image shaped like a trapezoid.  
  The user specifies 4 input parameters: (1) the length of the top 
  width of the trapezoid, (2) the x-coordinate of the starting point 
  of the top width, (3) a pixel value to fill in blank output regions 
  outside of the trapezoid, and (4) the orientation of the trapezoid. 
  The top width must be at least 1 pixel and no more than the width 
  of the input image. A long top width corresponds to a vanishing 
  point that is far away from the viewer or camera. A short top width 
  indicates a vanishing point near the camera. The filler pixel value 
  must be given in a format compatible with the output color format. For 
  instance, black is 0 for RGB565 and 0x8010 for YCbCr. White is 0xFFFF 
  for RGB565 and 0x80EB for YCbCr. Notice that for YCbCr, the chroma 
  value (Cb or Cr) comes before the luma value, so white is 0x80EB 
  instead of 0xEB80. The trapezoid’s height and bottom width are assumed 
  to be the input height and width for "up" and "down" orientations. 
  For "left" and "right" orientations, the trapezoid's height and bottom 
  width are the input width and height, respectively.

  The orientation of the trapezoid can be one of four values: 
  up ( 0), down (1), left (2), or right (3), as illustrated below.

  For any orientation, the top width is defined as the smaller 
  of the two parallel edges of the trapezoid.

  Input and output images must be in YCbCr 4:2:2.

  ( 0) up:
         ----
        /    \
       /      \
       --------
  (1) down:
       --------
       \      /
        \    /
         ----
  (2) left:
        __ --- |
      |        |
      | __     |
           --- |
  (3) right:
      | --- __
      |        |
      |     __ |
      | ---


DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr     pointer to the input image
  out_img_ptr    pointer to the output image
  topWidth       top width of the trapezoidal output image
  xStart         starting x-coordinate of the top width
  fillerPixel    pixel value for the filler region
  orientation    orientation of the output image

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_perspective_ycbcr
(
  ipl_image_type* in_img_ptr,   /* Points to the input image          */
  ipl_image_type* out_img_ptr,  /* Points to the output image         */
  uint32 topWidth,              /* Top width of trapezoidal output    */
  int32 xStart,                 /* Starting x-coordinate of top width */
  uint16 fillerPixel,           /* Pixel value for filler region      */
  uint8 orientation             /* Orientation of output image        */
)
{
  unsigned char *rotImgPtr, *fullRotImgPtr, *fullProtImgPtr, *protImgPtr;
  unsigned char cb, cr, luma1, luma2, fillerChroma, fillerLuma;
  uint32 *yMap;
  uint32 inIndex, outIndex;
  ipl_rotate90_type rot90 = IPL_NOROT;

  ipl_image_type rotImg, protImg, fullRotImg, fullProtImg;

  int32 xout, yout, xin, yin, yprev, x, y, c1, c2, c3, a,
        k, k2, xs, d, xend;
  uint32 w, h;

  MSG_LOW("ipl_perspective_ycbcr marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr || 
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_perspective_ycbcr marker_200\n");
    return IPL_FAILURE;
  }  

  //if ((in_img_ptr == NULL) || (out_img_ptr == NULL))
  //{
  //  MSG_LOW("ipl_perspective_ycbcr marker_201\n");
  //  return IPL_FAILURE;
  //}

  /* Initialize local variables */
  fillerChroma = (unsigned char)(fillerPixel >> 8);
  fillerLuma = (unsigned char)(fillerPixel & 0x00FF);

  if (orientation == 0) 
  {
    /* up */
    w = out_img_ptr->dx;
    h = out_img_ptr->dy;
  } 
  else if (orientation == 1) 
  {
    /* down */
    rot90 = IPL_ROT180;
    w = out_img_ptr->dx;
    h = out_img_ptr->dy;
    xStart = w - xStart - topWidth;
  } 
  else if (orientation == 2) 
  {
    /* left */
    rot90 = IPL_ROT90;
    w = out_img_ptr->dy;
    h = out_img_ptr->dx;
    xStart = w - xStart - topWidth;
  } 
  else 
  {
    /* right */
    rot90 = IPL_ROT270;
    w = out_img_ptr->dy;
    h = out_img_ptr->dx;
  }

  MSG_LOW("ipl_perspective_ycbcr marker_1\n");

  if (orientation != 0) 
  {
    rotImg.cFormat = IPL_YCbCr;
    rotImg.dx = w;
    rotImg.dy = h;
    if (ipl_malloc_img(&rotImg))
    {
      MSG_LOW("ipl_perspective_ycbcr marker_202\n");
      return IPL_NO_MEMORY;
    }
    if (ipl_rotate90(in_img_ptr, &rotImg, rot90) != IPL_SUCCESS) 
    {
      ipl_free_img(&rotImg);
      MSG_LOW("ipl_perspective_ycbcr marker_203\n");
      return IPL_NO_MEMORY;
    }
    rotImgPtr = rotImg.imgPtr;


    protImg.cFormat = IPL_YCbCr;
    protImg.dx = w;
    protImg.dy = h;
    if (ipl_malloc_img(&protImg))
    {
      ipl_free_img(&rotImg);
      MSG_LOW("ipl_perspective_ycbcr marker_204\n");
      return IPL_NO_MEMORY;
    }
    protImgPtr = protImg.imgPtr;
  }
  else 
  {
    rotImgPtr = in_img_ptr->imgPtr;
    protImgPtr = out_img_ptr->imgPtr;
  }

  if ((rotImgPtr == NULL) || (protImgPtr == NULL))
  {
    if (orientation != 0) {
      ipl_free_img(&rotImg);
      ipl_free_img(&protImg);
    }
    return IPL_FAILURE;
  }

  /* Malloc full YCbCr rotated perspective image */
  fullRotImg.cFormat = IPL_YCbCr444;
  fullRotImg.dx = w;
  fullRotImg.dy = h;
  if (ipl_malloc_img(&fullRotImg)) 
  {

    MSG_LOW("ipl_perspective_ycbcr marker_206\n");
    if (orientation != 0) {
      ipl_free_img(&rotImg);
      ipl_free_img(&protImg);
    }
    return IPL_NO_MEMORY;
  }
  fullRotImgPtr = fullRotImg.imgPtr;

  /* Create full YCbCr rotated input image */
  inIndex = 0;
  outIndex = 0;
  for (y = 0; y < (int32) h; y++) 
  {
    for (x = 0; x < (int32) w; x+=2) 
    {
      cb = *((unsigned char*)(rotImgPtr+inIndex++));
      luma1 = *((unsigned char*)(rotImgPtr+inIndex++));
      cr = *((unsigned char*)(rotImgPtr+inIndex++));
      luma2 = *((unsigned char*)(rotImgPtr+inIndex++));
      *((unsigned char*)(fullRotImgPtr+outIndex++)) = cb;
      *((unsigned char*)(fullRotImgPtr+outIndex++)) = cr;
      *((unsigned char*)(fullRotImgPtr+outIndex++)) = luma1;
      *((unsigned char*)(fullRotImgPtr+outIndex++)) = cb;
      *((unsigned char*)(fullRotImgPtr+outIndex++)) = cr;
      *((unsigned char*)(fullRotImgPtr+outIndex++)) = luma2;
    } /* end xout loop */
  } /* end yout loop */


  /* Malloc full YCbCr rotated perspective image */
  fullProtImg.cFormat = IPL_YCbCr444;
  fullProtImg.dx = w;
  fullProtImg.dy = h;
  if (ipl_malloc_img(&fullProtImg)) 
  {
    ipl_free_img(&fullRotImg);
    if (orientation != 0) 
    {
      ipl_free_img(&rotImg);
      ipl_free_img(&protImg);
    }
    MSG_LOW("ipl_perspective_ycbcr marker_207\n");
    return IPL_NO_MEMORY;
  }
  fullProtImgPtr = fullProtImg.imgPtr;

  /* 
  ** Malloc yMap: mapping from output y-coordinates to input y-coordinates.
  ** Coordinate system is defined with the origin at the top left corner of 
  ** the image, with the positive x-axis extending to the right and the 
  ** positive y-axis extending downward.
  */
  yMap = ipl_malloc(h*sizeof(uint32));
  if (!yMap) 
  {   
    ipl_free_img(&fullRotImg);
    ipl_free_img(&fullProtImg);
    if (orientation != 0) 
    {
      ipl_free_img(&rotImg);
      ipl_free_img(&protImg);
    }
    MSG_LOW("ipl_perspective_ycbcr marker_208\n");
    return IPL_NO_MEMORY;
  }
  
  /* Initialize starting bin to zero */
  yMap[0] = 0;
  yprev = 0;
  a = h - 1;
  if (a == 0) a=1;
  k2 = ((1054*(topWidth<<8))/w + 58777298) - 
       (((1922*(topWidth<<8)/w)<<8)/w*topWidth);

  /* Create yMap */
  for (yin = 1; yin < (int32) h; yin++) 
  {
    k = (k2+(int32)(1038*((a<<16)/((yin<<1)+a))))>>16;
    c1 = ((k*(a-yin))>>1)/a;
    c2 = (((c1*(a-yin))*k)/a)>>10;
    c3 = ((((c2*7*(a-yin))>>6)*k)/a)>>4;
    yout = (yin * (65536 + 89*c1 + 31*c2 + c3)) >> 16;

    if (yout < yprev) 
    {
      yout = yprev;
    }

    if (yout >= (int32) h) 
    {
      yout = h - 1;
    }

    /* Assign mapping from output y-coordinate to input y-coordinate */
    yMap[yout] = yin;
    if (yout > (yprev + 1)) 
    {
      for (y = yprev + 1; y < yout; y++) 
      {
        yMap[y] = yin;
      }
    }
    yprev = yout;
  }

  /* Loop through input image */
  outIndex = 0;
  for (yout = 0; yout < (int32) h; yout++) 
  {
    /* Start and end coordinates for trapezoid region */
    xs = (((((xStart*(a-yout)))<<10)/a)>>10);
    xend = (w-1)-(((((w-topWidth-xStart)*(a-yout))<<10)/a)>>10);
    d = xend - xs;
    if (d == 0) 
    {
      d++;
    }

    k = ((w-1)<<10)/d;
    /* Left side outside of trapezoid */
    for (xout = 0; xout < xs; xout++) 
    {
      *((unsigned char*)(fullProtImgPtr+outIndex++)) = fillerChroma;
      *((unsigned char*)(fullProtImgPtr+outIndex++)) = fillerChroma;
      *((unsigned char*)(fullProtImgPtr+outIndex++)) = fillerLuma;
    }

    /* Trapezoid */
    for (xout = xs; xout < xend; xout++) 
    {
      xin = ((xout-xs)*k)>>10;
      inIndex = (xin + w*(a-yMap[a-yout]))*3;
      *((unsigned char*)(fullProtImgPtr+outIndex++)) = 
        *((unsigned char*)(fullRotImgPtr+inIndex++));
      *((unsigned char*)(fullProtImgPtr+outIndex++)) = 
        *((unsigned char*)(fullRotImgPtr+inIndex++));
      *((unsigned char*)(fullProtImgPtr+outIndex++)) = 
        *((unsigned char*)(fullRotImgPtr+inIndex));
    }

    /* Right side outside trapezoid */
    for (xout = xend; xout < (int32) w; xout++) 
    {
      *((unsigned char*)(fullProtImgPtr+outIndex++)) = fillerChroma;
      *((unsigned char*)(fullProtImgPtr+outIndex++)) = fillerChroma;
      *((unsigned char*)(fullProtImgPtr+outIndex++)) = fillerLuma;
    } /* end xout loop */
  } /* end yout loop */

  /* Get subsampled YCbCr output */
  inIndex = 0;
  outIndex = 0;
  for (y = 0; y < (int32) h; y++) 
  {
    for (x = 0; x < (int32) w; x+=2) 
    {
      /* get Cb */
      *((unsigned char*)(protImgPtr + outIndex++)) =
        *((unsigned char*)(fullProtImgPtr + inIndex++));
      inIndex++;
      *((unsigned char*)(protImgPtr + outIndex++)) =
        *((unsigned char*)(fullProtImgPtr + inIndex++));
      /* get Cr */
      inIndex++;
      *((unsigned char*)(protImgPtr + outIndex++)) =
        *((unsigned char*)(fullProtImgPtr + inIndex++));
      *((unsigned char*)(protImgPtr + outIndex++)) =
        *((unsigned char*)(fullProtImgPtr + inIndex++));
    } /* end x loop */
  } /* end y loop */

  /* Rotate back to get output */
  if (orientation != 0) 
  {
    if (orientation == 1) 
    {
      /* down */
      rot90 = IPL_ROT180;
    } 
    else if (orientation == 2) 
    {
      /* left */
      rot90 = IPL_ROT270;
    } 
    else 
    {
      /* right */
      rot90 = IPL_ROT90;
    }

    if (ipl_rotate90(&protImg, out_img_ptr, rot90) != IPL_SUCCESS)
    {
      ipl_sys_free(yMap);
      ipl_free_img(&fullRotImg);
      ipl_free_img(&fullProtImg);

      ipl_free_img(&rotImg);
      ipl_free_img(&protImg);

      MSG_LOW("ipl_perspective_ycbcr marker_209\n");
      return IPL_FAILURE;
    }

    ipl_free_img(&rotImg);
    ipl_free_img(&protImg);
  }
  
  /* Free memory */
  ipl_sys_free(yMap);
  ipl_free_img(&fullRotImg);
  ipl_free_img(&fullProtImg);

  MSG_LOW("ipl_perspective_ycbcr marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_perspective_ycbcr */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_perspective

DESCRIPTION
  This function creates a perspective image shaped like a trapezoid.  
  The user specifies 4 input parameters: (1) the length of the top 
  width of the trapezoid, (2) the x-coordinate of the starting point 
  of the top width, (3) a pixel value to fill in blank output regions 
  outside of the trapezoid, and (4) the orientation of the trapezoid. 
  The top width must be at least 1 pixel and no more than the width 
  of the input image. A long top width corresponds to a vanishing 
  point that is far away from the viewer or camera. A short top width 
  indicates a vanishing point near the camera. The filler pixel value 
  must be given in a format compatible with the output color format. For 
  instance, black is 0 for RGB565 and 0x8010 for YCbCr. White is 0xFFFF 
  for RGB565 and 0x80EB for YCbCr. Notice that for YCbCr, the chroma 
  value (Cb or Cr) comes before the luma value, so white is 0x80EB 
  instead of 0xEB80. The trapezoid’s height and bottom width are assumed 
  to be the input height and width for "up" and "down" orientations. 
  For "left" and "right" orientations, the trapezoid's height and bottom 
  width are the input width and height, respectively.

  The orientation of the trapezoid can be one of four values: 
  up ( 0), down (1), left (2), or right (3), as illustrated below.

  For any orientation, the top width is defined as the smaller 
  of the two parallel edges of the trapezoid.

  Input and output images must have the same color format, which 
  can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

  ( 0) up:
         ----
        /    \
       /      \
       --------
  (1) down:
       --------
       \      /
        \    /
         ----
  (2) left:
        __ --- |
      |        |
      | __     |
           --- |
  (3) right:
      | --- __
      |        |
      |     __ |
      | ---


DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr     pointer to the input image
  out_img_ptr    pointer to the output image
  topWidth       top width of the trapezoidal output image
  xStart         starting x-coordinate of the top width
  fillerPixel    pixel value for the filler region
  orientation    orientation of the output image

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_perspective
(
  ipl_image_type* in_img_ptr,   /* Points to the input image          */
  ipl_image_type* out_img_ptr,  /* Points to the output image         */
  uint32 topWidth,              /* Top width of trapezoidal output    */
  int32 xStart,                 /* Starting x-coordinate of top width */
  uint16 fillerPixel,           /* Pixel value for filler region      */
  uint8 orientation             /* Orientation of output image        */
)
{
  ipl_status_type retval;

  MSG_LOW("ipl_perspective marker_0\n");

  if (!in_img_ptr || !in_img_ptr->imgPtr || 
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_perspective marker_200\n");
    return IPL_FAILURE;
  }
  
  /* Input and output color formats must be the same */
  if (in_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_perspective marker_201\n");
    return IPL_FAILURE;
  }
  
  if (xStart < 0)
    xStart = 1;

  if (topWidth == 0)
    topWidth = 1; 

  /*
  ** Orientation must be top ( 0), bottom (1), left (2),
  ** or right (3).  Orientation is an unsigned integer, so 
  ** only need to check whether it is greater than 3.
  */
  if (orientation > 3) {
    MSG_LOW("ipl_perspective marker_202\n");
    return IPL_FAILURE;
  }
  /* Bottom width must be larger than or equal to top width */
  if ((orientation <= 1 && out_img_ptr->dx < (topWidth + xStart)) ||
    (orientation > 1 && out_img_ptr->dy < (topWidth + xStart))) 
  {
    MSG_LOW("ipl_perspective marker_203\n");
    return IPL_FAILURE;
  }
  /* Input and output width and height must be equal*/
  if (in_img_ptr->dx != out_img_ptr->dx || 
    in_img_ptr->dy != out_img_ptr->dy)
  {
    MSG_LOW("ipl_perspective marker_204\n");
    return IPL_FAILURE;
  }
  /* Starting coordinate for top width must not exceed output width */
  if (xStart >= (int32) out_img_ptr->dx) {
    MSG_LOW("ipl_perspective marker_205\n");
    return IPL_FAILURE;
  }

  /* 
  ** Set output image equal to input image if topWidth 
  ** equals bottom width 
  */
  if ((orientation <= 1 && out_img_ptr->dx == topWidth) ||
    (orientation > 1 && out_img_ptr->dy == topWidth))
  {
    out_img_ptr->imgPtr = in_img_ptr->imgPtr;
    MSG_LOW("ipl_perspective marker_100\n");
    return IPL_SUCCESS;
  }

  MSG_LOW("ipl_perspective marker_1\n");
  /* Call the appropriate function */
  if (in_img_ptr->cFormat == IPL_RGB565)
  {
    /* RGB 565 */
    retval = ipl_perspective_rgb565(in_img_ptr,out_img_ptr,topWidth,xStart,fillerPixel,orientation);
    if (retval != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_perspective marker_206\n");
      return retval;
    }
  } 
  else if (in_img_ptr->cFormat == IPL_YCbCr)
  {
    /* YCbCr 4:2:2 */
    if (xStart%2) 
    {
      xStart++;
    }

    retval = ipl_perspective_ycbcr(in_img_ptr,out_img_ptr,topWidth,xStart,fillerPixel,orientation);
    if (retval != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_perspective marker_207\n");
      return retval;
    }
  } 
  else if (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    ipl_image_type innew_img, out422_img;
    unsigned char *innew_data, *out422_data;
    /* allocate memory for YCbCr4:2:2 images */
    innew_data = ipl_malloc(in_img_ptr->dx*in_img_ptr->dy*2* sizeof(unsigned char));
    if (!innew_data) 
    {
      MSG_LOW("ipl_perspective marker_208\n");
      return IPL_NO_MEMORY;
    }
    
    out422_data=ipl_malloc(out_img_ptr->dx*out_img_ptr->dy*2*sizeof(unsigned char));
    if (!out422_data) 
    {
      ipl_sys_free(innew_data);
      MSG_LOW("ipl_perspective marker_209\n");
      return IPL_NO_MEMORY;
    }

    /* input 1 */
    innew_img.cFormat = IPL_YCbCr;
    innew_img.dx = in_img_ptr->dx;
    innew_img.dy = in_img_ptr->dy;
    innew_img.imgPtr = innew_data;
    if (ipl_convert_image(in_img_ptr, &innew_img) != IPL_SUCCESS) 
    {
      ipl_sys_free(innew_data);
      ipl_sys_free(out422_data);
      MSG_LOW("ipl_perspective marker_210\n");
      return IPL_FAILURE;
    }
    /* ycbcr 4:2:2 output */
    out422_img.cFormat = IPL_YCbCr;
    out422_img.dx = out_img_ptr->dx;
    out422_img.dy = out_img_ptr->dy;
    out422_img.imgPtr = out422_data;
    /* call ycbcr 4:2:2 function */

    retval = ipl_perspective_ycbcr(&innew_img,&out422_img,topWidth,xStart,fillerPixel, orientation);
    if (retval != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_perspective marker_211\n");
      ipl_sys_free(innew_data);
      ipl_sys_free(out422_data);
      return retval;
    }


    /* get YCbCr 4:2:0 output */
    if (ipl_convert_image(&out422_img, out_img_ptr)
      != IPL_SUCCESS) 
    {
      ipl_sys_free(innew_data);
      ipl_sys_free(out422_data);
      MSG_LOW("ipl_perspective marker_212\n");
      return IPL_FAILURE;
    }

    /* sys_free memory */
    ipl_sys_free(innew_data);
    ipl_sys_free(out422_data);
  } 
  else 
  {
    MSG_LOW("ipl_perspective marker_213\n");
    /* Other formats not supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_perspective marker_101\n");
  return IPL_SUCCESS;
} /* End ipl_perspective */




#if 0


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_xform_Resize_cubicHigh

DESCRIPTION
  This function is used to do cubic upsize
  The input should be RGB565 and the output should be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_xform_Resize_cubicHigh
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  cropin,
  ipl_rect_type*  cropout
)
{
  uint16 row,col;
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint32 pitchx;

  uint8 xf, yf;
  register uint16 x, y;
  register int32 w; //cubic weights require SIGNED 32bit int's
  uint16* ptr16; 

  uint8 * rgb_buf;
  uint8 * out;

  uint16 cix, ciy, cidx, cidy, cidx3;
  uint16 cox, coy, codx, cody;
  uint16 idx,idy;
  uint16 odx,ody;
  //accum_r, accum_g and accum_b are uint16 for bilinear 
  //16 bits are not enough for bicubic
  //AND they can be negative for bicubic - hence switch to int32
  int32 accum_r, accum_g, accum_b; 

  MSG_LOW("inside ipl_xform_Resize_cubicHigh\n");



  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
    return IPL_FAILURE;

  /* how we lookup a point in crop space */
  //#define P(x,y,c) (*(rgb_buf + (y)*cidx3 + (x)*3+c))

  /* if user doesnt provide output crop, assume it is the same as the output
   * image */
  if (cropout == NULL)
  {
    cox = 0;
    coy = 0;
    codx = o_img_ptr->dx;
    cody = o_img_ptr->dy;
  }
  else
  {
    cox = cropout->x;
    coy = cropout->y;
    codx = cropout->dx;
    cody = cropout->dy;
  }

  /* if user doesnt provide input crop, assume it is the same as input 
   * image */
  if (cropin == NULL)
  {
    cix = 0;
    ciy = 0;
    cidx = i_img_ptr->dx;
    cidy = i_img_ptr->dy;
  }
  else
  { 
    cix = cropin->x;
    ciy = cropin->y;
    cidx = cropin->dx;
    cidy = cropin->dy;
  }

  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;
  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;
  cidx3 = (cidx+3)*3;     

  /* strictly upsize */
  /*
  if ((cidx > codx) || (cidy > cody))
    return IPL_FAILURE;
  */

  /* make sure valid input size */
  if (idx < 1 || idy < 1)
    return IPL_FAILURE;


  /* dont allow cropping of area bigger than image */
  if ((cidx+cix > idx) || (cidy+ciy > idy))
    return IPL_FAILURE;

  /*
  ** Only RGB565 Input and RGB565 Output is supported
  */
  if (!((i_img_ptr->cFormat == IPL_RGB565) &&
        (o_img_ptr->cFormat == IPL_RGB565)))
  {
    return IPL_FAILURE;
  }

  /*
  * Choose Q9 so you can upsize area from 10x10 to at 5120x5120, or
  * i.e. at least a factor of 512, 2^9
  * (dont do any rounding!)
  */

  resizeFactorX = (cidx<<9) / codx;
  resizeFactorY = (cidy<<9) / cody;

  if (IPL_XFORM_DEBUG) //lint !e774 !e506
  {
    printf("Input          (%d,%d)\n", idx, idy);
    printf("CropI  (%d,%d) (%d,%d)\n", cix,ciy,cidx,cidy);
    printf("CropO  (%d,%d) (%d,%d)\n", cox,coy,codx,cody);
    printf("Output         (%d,%d)\n", odx, ody);
    printf("ResizeX:  %ld\n", resizeFactorX);
    printf("ResizeY:  %ld\n", resizeFactorY);
  }


  /*
  * Create and fill intermediate buffer with unpacked data.
  * Make crop area 3 pixels bigger in each dimension so we dont
  * segfault.
  */
	rgb_buf= (uint8 *) ipl_malloc((cidx+3)*(cidy+3)*3);
	if (!rgb_buf)
    return IPL_NO_MEMORY;
  
  if (cix > 1 && ciy > 1 && cidx < idx-2 && cidy < idy-2) 
  {
    // for cubic interpolation we need 1 extra pixel on top and left
    // and 2 extra pixels on the bottom and right
	  ptr16 = (uint16*) i_img_ptr->imgPtr + (ciy*idx+cix-1);
	  out = rgb_buf;
    for (row=0; row < cidy+3; row++)
    {
      for (col=0; col < cidx+3; col++)
      {
        /* unpack_rgb565(*ptr16, out, out+1, out+2); */
        *out     = (unsigned char)((*ptr16 & 0xF800) >>8);
        *(out+1) = (unsigned char)((*ptr16 & 0x07E0) >>3);
        *(out+2) = (unsigned char)((*ptr16 & 0x001F) <<3);

        if (IPL_XFORM_DEBUG) //lint !e774 !e506
          printf("rgb_buf@(%d,%d) (%d,%d,%d)\n",col,row,
                 *out,*(out+1),*(out+2));

        ptr16++;
        out += 3;
      }
      /* move to the beginning of next line */
      ptr16 += (idx - cidx - 1);
    }
  }
  else
  {
    // If we can't make crop area bigger, then assign extra space 
	  // and mirror copy. top line and leftmost column, 
    // two bottom lines and two rightmost columns should be mirror copied.
	  ptr16 = (uint16*) i_img_ptr->imgPtr + (ciy*idx+cix);
	  out = rgb_buf;
    memset (rgb_buf, 0, (cidx+3)*(cidy+3)*3);

	  //MIRROR COPY TOP LINE
	  *out     = (unsigned char)((*ptr16 & 0xF800) >>8);
    *(out+1) = (unsigned char)((*ptr16 & 0x07E0) >>3);
    *(out+2) = (unsigned char)((*ptr16 & 0x001F) <<3);
	  out += 3;
    for (col=0; col < cidx; col++) 
    {
      /* unpack_rgb565(*ptr16, out, out+1, out+2); */
      *out     = (unsigned char)((*ptr16 & 0xF800) >>8);
      *(out+1) = (unsigned char)((*ptr16 & 0x07E0) >>3);
      *(out+2) = (unsigned char)((*ptr16 & 0x001F) <<3);

	    ptr16++;
      out += 3;
    }

	  /* "mirror copy" for the last two columns of the 
	  current line AND the first column of the next line */
    *out     = *(out-3);    
    *(out+1) = *(out-2); 
    *(out+2) = *(out-1); 
    
	  out+=3;
	  *out     = *(out-9);    
    *(out+1) = *(out-8); 
    *(out+2) = *(out-7);
  
	  /* go to start of next line */
    out += 3;

    //RESET IMAGE POINTER
	  ptr16 = (uint16*) i_img_ptr->imgPtr + (ciy*idx+cix);
	  for (row=0; row < cidy; row++) 
	  {
	    *out = (unsigned char)((*ptr16 & 0xF800) >>8);
      *(out+1) = (unsigned char)((*ptr16 & 0x07E0) >>3);
      *(out+2) = (unsigned char)((*ptr16 & 0x001F) <<3);
	    out += 3;
      for (col=0; col < cidx; col++) 
      {
        /* unpack_rgb565(*ptr16, out, out+1, out+2); */
        *out     = (unsigned char)((*ptr16 & 0xF800) >>8);
        *(out+1) = (unsigned char)((*ptr16 & 0x07E0) >>3);
        *(out+2) = (unsigned char)((*ptr16 & 0x001F) <<3);

        if (IPL_XFORM_DEBUG) //lint !e774 !e506
          printf("rgb_buf@(%d,%d) (%d,%d,%d)\n",col,row,
                 *out,*(out+1),*(out+2));
        ptr16++;
        out += 3;
      }

      /* "mirror copy" for the last two columns of the 
	    current line AND the first column of the next line */
      *out     = *(out-3);    
      *(out+1) = *(out-2); 
      *(out+2) = *(out-1); 
      
	    out+=3;
	    *out     = *(out-9);    
      *(out+1) = *(out-8); 
      *(out+2) = *(out-7);

	    /* go to start of next line */
      out += 3;
      
      /* move to the beginning of next line */
      ptr16 += (idx - cidx);
    }

    /* "mirror copy" for the last two lines */
    for (col=0; col <= cidx3; col++)
    {
      *out = *(out-cidx3);
	    *(out+cidx3) = *(out-2*cidx3);
      out++;
    }
  }

  /* Go through each output pixel and fill with weighted sum from temp buffer*/
  // ptr16 = (uint16*)o_img_ptr->imgPtr;

  ptr16 = (uint16*)o_img_ptr->imgPtr + cox + coy*o_img_ptr->dx;
  pitchx = o_img_ptr->dx - codx;
  for (row=0; row < cody; row++)
  {
    /* shift back by 9 place for integer part */
    y  = (uint16) (((uint32) (row*resizeFactorY) >> 9) + 1);

    /* shift 6 places and AND with 0111 to get 3 bits in remainder */
    yf = (uint8)  ((uint32)((row*resizeFactorY) >> 6) & 0x7);

    /* advance to next row in temp buffer */
    for (col=0; col < codx; col++)
    {
      x  = (uint16) (((uint32)  (col*resizeFactorX) >> 9) + 1);
      xf = (uint8)  ((uint32) ((col*resizeFactorX) >> 6) & 0x7);

      if (IPL_XFORM_DEBUG) //lint !e774 !e506
      {
        printf("\n(col,row): %d,%d\n", col,row);
        printf("x, xf: %d, %d\n", x, xf);
        printf("y, yf: %d, %d\n", y, yf);
      }

      /*
      **    +--------+--------+
      **    |        |        |
      **    |        |        |
      **    |   A    |   B    |
      **    |        |        |
      **    +--------+--------+
      **    |      X |        |       X value is A*(1-fx)*(1-fy)
      **    |        |        |                  B*(fx)*(1-fy) 
      **    |   C    |   D    |                  C*(1-fx)*(fy)
      **    |        |        |                  D*(fx)*(fy)
      **    +--------+--------+
      */

	    /*  
	    **	(A)----------(B)---X------(C)----------(D)
	    **
	    **	                 3     2
	    **	X =   f(k-1) ( -s  + 2s  - s ) / 2
      **                     3     2
	    **		+ f(k)   ( 3s  - 5s  + 2 ) / 2
	    **                     3     2
	    **		+ f(k+1) (-3s  + 4s  + s ) / 2
      **                     3     2
	    **		+ f(k+2) (  s  -  s ) / 2
	    **
	    **
	    **	s is fx or fy depending on the interpolation direction 
	    **	vertical   -> s=yf
	    **	horizontal -> s=xf
	    **
	    **	A11		A12		A13		A14
	    **
	    **
	    **	A21		A22		A23		A24
	    **				X
	    **
	    **	A31		A32		A33		A34
	    **
	    **
	    **	A41		A42		A43		A44
	    **
	    **
	    **
	    */

      if (IPL_XFORM_DEBUG) //lint !e774 !e506
      {
        printf("wA%d wB%d wC%d wD%d\n", (8-xf)*(8-yf),(xf)*(8-yf),
               (8-xf)*(yf),(xf)*(yf));
        printf("tA%d tB%d tC%d tD%d\n\n", biWeights[xf][yf][0],
        biWeights[xf][yf][1], biWeights[xf][yf][3], biWeights[xf][yf][2]);
      }

	    /* A11 pixel */
	    w = cubWeights[xf][yf][0];
      accum_r = w * P(x-1,y-1,0);
      accum_g = w * P(x-1,y-1,1);
      accum_b = w * P(x-1,y-1,2);

	    /* A12 pixel */
	    w = cubWeights[xf][yf][1];
      accum_r += (w * P(x,y-1,0));
      accum_g += (w * P(x,y-1,1));
      accum_b += (w * P(x,y-1,2));

	    /* A13 pixel */
	    w = cubWeights[xf][yf][2];
      accum_r += (w * P(x+1,y-1,0));
      accum_g += (w * P(x+1,y-1,1));
      accum_b += (w * P(x+1,y-1,2));

	    /* A14 pixel */
	    w = cubWeights[xf][yf][3];
      accum_r += (w * P(x+2,y-1,0));
      accum_g += (w * P(x+2,y-1,1));
      accum_b += (w * P(x+2,y-1,2));

	    /* A21 pixel */
	    w = cubWeights[xf][yf][4];
      accum_r += (w * P(x-1,y,0));
      accum_g += (w * P(x-1,y,1));
      accum_b += (w * P(x-1,y,2));

      /* A22 pixel */
      w = cubWeights[xf][yf][5];
      accum_r += (w * P(x,y,0));
      accum_g += (w * P(x,y,1));
      accum_b += (w * P(x,y,2));

      /* A23 pixel */
      w = cubWeights[xf][yf][6];
      accum_r += (w * P(x+1,y,0));
      accum_g += (w * P(x+1,y,1));
      accum_b += (w * P(x+1,y,2));

	    /* A24 pixel */
	    w = cubWeights[xf][yf][7];
      accum_r += (w * P(x+2,y,0));
      accum_g += (w * P(x+2,y,1));
      accum_b += (w * P(x+2,y,2));

	    /* A31 pixel */
	    w = cubWeights[xf][yf][8];
      accum_r += (w * P(x-1,y+1,0));
      accum_g += (w * P(x-1,y+1,1));
      accum_b += (w * P(x-1,y+1,2));

      /* A32 pixel */
      w = cubWeights[xf][yf][9];
      accum_r += (w * P(x,y+1,0));
      accum_g += (w * P(x,y+1,1));
      accum_b += (w * P(x,y+1,2));

      /* A33 pixel */ 
      w = cubWeights[xf][yf][10];
      accum_r += (w * P(x+1,y+1,0));
      accum_g += (w * P(x+1,y+1,1));
      accum_b += (w * P(x+1,y+1,2));

	    /* A34 pixel */
	    w = cubWeights[xf][yf][11];
      accum_r += (w * P(x+2,y+1,0));
      accum_g += (w * P(x+2,y+1,1));
      accum_b += (w * P(x+2,y+1,2));

	    /* A41 pixel */
	    w = cubWeights[xf][yf][12];
      accum_r += (w * P(x-1,y+2,0));
      accum_g += (w * P(x-1,y+2,1));
      accum_b += (w * P(x-1,y+2,2));

	    /* A42 pixel */
	    w = cubWeights[xf][yf][13];
      accum_r += (w * P(x,y+2,0));
      accum_g += (w * P(x,y+2,1));
      accum_b += (w * P(x,y+2,2));

	    /* A43 pixel */
	    w = cubWeights[xf][yf][14];
      accum_r += (w * P(x+1,y+2,0));
      accum_g += (w * P(x+1,y+2,1));
      accum_b += (w * P(x+1,y+2,2));

	    /* A44 pixel */
	    w = cubWeights[xf][yf][15];
      accum_r += (w * P(x+2,y+2,0));
      accum_g += (w * P(x+2,y+2,1));
      accum_b += (w * P(x+2,y+2,2));

      /*
      if (IPL_XFORM_DEBUG) //lint !e774 !e506
        printf("Crgb %d,%d,%d\n", accum_r, accum_g, accum_b);
      */
      
	    /* divide by (8^6 * 4) = 2^20 */
      accum_r = accum_r >> 20;
      accum_g = accum_g >> 20;
      accum_b = accum_b >> 20;

	    /* CUBIC INTERPOLATION REQUIRES HARD CLIP*/
	    /* hard-clip to 0-255*/
	    if(accum_r < 0) 
		    accum_r=0;
	    if(accum_g < 0) 
		    accum_g=0;
	    if(accum_b < 0) 
		    accum_b=0;

	    if(accum_r > 255) 
		    accum_r=255;
	    if(accum_g > 255) 
		    accum_g=255;
	    if(accum_b > 255) 
		    accum_b=255;

      /*
      if (IPL_XFORM_DEBUG) //lint !e774 !e506
        printf("Final RGB %d,%d,%d\n", accum_r, accum_g, accum_b);
      */

      *ptr16 = pack_rgb565(accum_r, accum_g, accum_b);
      ptr16++;
    } 
    ptr16 += pitchx;
  } 

  ipl_sys_free(rgb_buf);


  return IPL_SUCCESS;
}


#endif






/* <EJECT> */
/*===========================================================================

FUNCTION ipl_upsize

DESCRIPTION
  This function is used to do upsize
  The input should be RGB565 and the output should be RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   points to the input image
  output_img_ptr  points to the output image
  crop            point to where in input to crop
  quality         refers to quality of output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_upsize
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  crop,
  ipl_quality_type  quality 
)
{
  ipl_status_type retval;
  ipl2_image_upsize_param_type i_param;
  uint32 i, j;  // generic variables, user for several things

  MSG_LOW("ipl_upsize marker_0\n");
  /*------------------------------------------------------------------------
      Switch on the input image type
  ------------------------------------------------------------------------*/

#define SCALE_TOLERANCE   10
  MSG_LOW("Scale Tolerance is %d\n", SCALE_TOLERANCE);


  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_upsize marker_200\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_upsize marker_1\n");

  switch(i_img_ptr->cFormat)
  {
    case IPL_YCbCr:

    case IPL_YCrCb420_FRAME_PK:
    case IPL_YCbCr420_FRAME_PK:

    case IPL_YCrCb420_LINE_PK:

      // in ipl world, crop means crop input, so this code  goes before 
      // checking for crop in and out
      if (i_img_ptr->cFormat == IPL_YCbCr && o_img_ptr->cFormat == IPL_YCbCr)
      {
        MSG_LOW("ipl_upsize marker_100\n");
        return(ipl_crop_resize_rot(i_img_ptr, o_img_ptr, crop, NULL,
                                   (ipl_rotate90_type) 0, quality));
      }

      // here (QVP code) , crop means croping on the output, whereas in IPL
      // we use cropping to refer to the input.
      if (crop != NULL)
      {
        i_param.crop.x = crop->x;
        i_param.crop.y = crop->y;
        i_param.crop.dx = crop->dx;
        i_param.crop.dy = crop->dy;
        i_param.qual = quality;
      }
      else
      {
        i_param.crop.x = 0;
        i_param.crop.y = 0;
        i_param.crop.dx = o_img_ptr->dx;
        i_param.crop.dy = o_img_ptr->dy;
        i_param.qual = quality;
      }

      // see which dimension has the biggest scaling ratio
      i = ((i_param.crop.dx + i_param.crop.dx/SCALE_TOLERANCE)/i_img_ptr->dx); 
      j = ((i_param.crop.dy + i_param.crop.dy/SCALE_TOLERANCE)/i_img_ptr->dy); 

      // based on that, see how much we are upsizing this axis
      if (i >= j)
      { 
        MSG_LOW("We are scaling width greater than height, %lu to %lu\n",
                i_img_ptr->dx,o_img_ptr->dx);
        i = i_img_ptr->dx;
        j = i_param.crop.dx;
      }
      else
      {
        MSG_LOW("We are scaling height greater than width, %lu to %lu\n",
                i_img_ptr->dy,o_img_ptr->dy);
        i = i_img_ptr->dy;
        j = i_param.crop.dy;
      }

      /* see if we are upsizing by 1.33 */
      if ( (((i * 4) / 3) <= j + j/SCALE_TOLERANCE) &&
           (((i * 4) / 3) >= j - j/SCALE_TOLERANCE))
      {
        MSG_LOW("calling QCIF_133\n");
        MSG_LOW("ipl_upsize marker_101\n");
        retval = (ipl2_upsize_QCIF_133(i_img_ptr, o_img_ptr, &i_param));
      }
      else if ((i * 2 <= j + j/SCALE_TOLERANCE) && 
               (i * 2 >= j - j/SCALE_TOLERANCE))
      {
        /* this function handles only 2x upsize and then crops. This function
         * will handle YCbCr422 and YCbCr420 */
        MSG_LOW("Doing 2x upsize\n");

        // we cant do 2x at low quality, set to medium
        if (i_param.qual == IPL_QUALITY_LOW)
          i_param.qual = IPL_QUALITY_MEDIUM;

        MSG_LOW("ipl_upsize marker_102\n");
        retval = (ipl2_handle_upSize(i_img_ptr, o_img_ptr, &i_param));
      }
      else
      {
        // we can only upsize any dimension at low quality
        MSG_LOW("Setting quality to low and calling ipl2_upsize\n");
        i_param.qual = IPL_QUALITY_LOW;
        MSG_LOW("ipl_upsize marker_103\n");
        return(ipl2_handle_upSize(i_img_ptr, o_img_ptr, &i_param));
      }
      break;

    case IPL_RGB565:
      if (quality == IPL_QUALITY_LOW || quality == IPL_QUALITY_MEDIUM)
      {
        MSG_LOW("ipl_upsize marker_104\n");

        //return(ipl_crop_resize_rot(i_img_ptr, o_img_ptr, crop, NULL,
        //                           (ipl_rotate90_type) 0,(ipl_quality_type) 0));
        retval = (ipl_xform_Upsize_qLow(i_img_ptr, o_img_ptr, crop));
      }
      else
      {
        MSG_LOW("ipl_upsize marker_105\n");
        return(ipl_xform_Resize_qHigh(i_img_ptr, o_img_ptr, crop, NULL));
      }
      break;

    case IPL_RGB888:
      if (quality == IPL_QUALITY_LOW || quality == IPL_QUALITY_MEDIUM)
      {
        MSG_LOW("ipl_upsize marker_106\n");
        retval = (ipl_xform_Upsize_Crop_qLow(i_img_ptr, o_img_ptr, crop, NULL));
      }
      else
      {
        MSG_LOW("ipl_upsize marker_107\n");
        retval = (ipl_xform_Resize_qHigh(i_img_ptr, o_img_ptr, crop, NULL));
      }
      break;

    case IPL_LUMA_ONLY:
        MSG_LOW("ipl_upsize marker_108\n");
        retval = (ipl_xform_Upsize_qLow(i_img_ptr, o_img_ptr, crop));
        break;

    default:
      IPL2_MSG_FATAL( "ipl2_upsize_QCIF failed  : Unsupported i/p color /\
               format  " );
      MSG_LOW("ipl_upsize marker_201\n");
      retval = IPL_FAILURE;
      break;

  }

  MSG_LOW("ipl_upsize marker_109\n"); //lint !e527 !e827
  return retval;
}


#ifndef FEATURE_IPL_LIGHT


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_shear_rgb565

DESCRIPTION
  This function performs shearing, which is an affine transformation.  
  The user specifies the horizontal and vertical shear factors, alpha 
  and beta, which can be any integer between negative infinity and positive 
  infinity. The output image size must be large enough to fit the
  parallelogram-shaped result from shearing. The user must specify
  a pixel value used to fill in blank output outside of the parallelogram.
  
  Both inputs and output must be in RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  out_img_ptr points to the output image
  alpha is the horizontal shear factor
  beta is the vertical shear factor
  xmin is the minimum x coordinate
  xmax is the maximum x coordinate
  ymin is the minimum y coordinate
  ymax is the maximum y coordinate
  fillerPixel is the filler pixel value

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_shear_rgb565
(
  ipl_image_type* i_img_ptr,   /* Points to the input image  */
  ipl_image_type* o_img_ptr,   /* Points to the output image */
  int32 alpha,                 /* Horizontal shear factor    */
  int32 beta,                  /* Vertical shear factor      */
  int32 xmin,                  /* Min x coordinate           */
  int32 xmax,                  /* Max x coordinate           */
  int32 ymin,                  /* Min y coordinate           */
  int32 ymax,                  /* Max y coordinate           */
  uint16 fillerPixel           /* Filler pixel value         */
)
{
	uint16 *inImgPtr, *outImgPtr;
	uint32 inIndex, outIndex;
	uint32 w, h, x, y;

	int32 xin, yin;

  MSG_LOW("ipl_shear_rgb565 marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_shear_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

	/* Initialize image pointers and local variables */
	inImgPtr = (uint16*)i_img_ptr->imgPtr;
	outImgPtr = (uint16*)o_img_ptr->imgPtr;
	w = (int32)i_img_ptr->dx;
	h = (int32)i_img_ptr->dy;

  MSG_LOW("ipl_shear_rgb565 marker_1\n");
	
  if ((alpha == 0) || (beta == 0)) {
		/* Wrap around output image to fit parallelogram */
		outIndex = 0;
		for(y = 0; y < h; y++) {
			for(x = 0; x < w; x++) {
				xin = ((100+alpha*beta)*x - 10*alpha*y)/100;
				while (xin < 0) {
					xin += (int32) w;
				}
				xin %= (int32) w;
				yin = (10*y - beta*x)/10;
				while (yin < 0) {
					yin += (int32) h;
				}
				yin %= (int32) h;
				inIndex = xin + yin*w;
				*((uint16*)(outImgPtr+outIndex)) = *((uint16*)(inImgPtr+inIndex));
				outIndex++;
			} /* end xin loop */
		} /* end yin loop */
	} else {
		/* Create enlarged output image to fit parallelogram */
		outIndex = 0;
		for(y = (uint32) ymin; y <= (uint32) ymax; y++) {
			for(x = (uint32) xmin; x <= (uint32) xmax; x++) {
				xin = ((100+alpha*beta)*x - 10*alpha*y)/100;
				yin = (10*y - beta*x)/10;
				if (xin >= 0 && (xin < (int32) w) && yin >= 0 && (yin < (int32) h)) 
        {
					inIndex = xin + yin*w;
					*((uint16*)(outImgPtr+outIndex)) = *((uint16*)(inImgPtr+inIndex));
				} else {
					*((uint16*)(outImgPtr+outIndex)) = fillerPixel;
				}
				outIndex++;
			} /* end x loop */
		} /* end y loop */
	} /* end if */

  MSG_LOW("ipl_shear_rgb565 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_shear_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_shear_ycbcr

DESCRIPTION
  This function performs shearing, which is an affine transformation.  
  The user specifies the horizontal and vertical shear factors, alpha 
  and beta, which can be any integer between negative infinity and 
  positive infinity. The output image size must be large enough to 
  fit the parallelogram-shaped result from shearing. The user must 
  specify a pixel value used to fill in blank output outside of the 
  parallelogram.
  
  Both inputs and output must be in YCbCr.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  out_img_ptr points to the output image
  alpha is the horizontal shear factor
  beta is the vertical shear factor
  xmin is the minimum x coordinate
  xmax is the maximum x coordinate
  ymin is the minimum y coordinate
  ymax is the maximum y coordinate
  fillerPixel is the filler pixel value

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_shear_ycbcr
(
  ipl_image_type* i_img_ptr,   /* Points to the input image  */
  ipl_image_type* o_img_ptr,   /* Points to the output image */
  int32 alpha,                 /* Horizontal shear factor    */
  int32 beta,                  /* Vertical shear factor      */
  int32 xmin,                  /* Min x coordinate           */
  int32 xmax,                  /* Max x coordinate           */
  int32 ymin,                  /* Min y coordinate           */
  int32 ymax,                  /* Max y coordinate           */
  uint16 fillerPixel           /* Filler pixel value         */
)
{
  uint8 *inImgPtr, *outImgPtr, *fullImgPtr, *fullOutImgPtr;
  uint8 cb, cr, yOne, yTwo, fillerChroma, fillerLuma;
  uint32 inIndex, outIndex;
  uint32 w, h, wout, hout, x, y;
  int32  xin, yin;

  MSG_LOW("ipl_shear_ycbcr marker_0\n");
  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_shear_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;
  fillerChroma = (uint8)(fillerPixel >> 8);
  fillerLuma = (uint8)(fillerPixel & 0x00FF);
  w = (int32)i_img_ptr->dx;
  h = (int32)i_img_ptr->dy;
  if ((alpha == 0) || (beta == 0)) {
    wout = w;
    hout = h;
    fullImgPtr = ipl_malloc(w*h*3*sizeof(uint8));
    if (!fullImgPtr) {
      MSG_LOW("ipl_shear_ycbcr marker_201\n");
      return IPL_NO_MEMORY;
    }
    fullOutImgPtr = ipl_malloc(wout*hout*3*sizeof(uint8));
    if (!fullOutImgPtr) {
      ipl_sys_free(fullImgPtr);
      MSG_LOW("ipl_shear_ycbcr marker_202\n");
      return IPL_NO_MEMORY;
    }
  } else {
    wout = (int32)o_img_ptr->dx;
    hout = (int32)o_img_ptr->dy;
    fullImgPtr = ipl_malloc(w*h*3*sizeof(uint8));
    if (!fullImgPtr) {
      MSG_LOW("ipl_shear_ycbcr marker_203\n");
      return IPL_NO_MEMORY;
    }
    fullOutImgPtr = ipl_malloc(wout*hout*3*sizeof(uint8));
    if (!fullOutImgPtr) {
      ipl_sys_free(fullImgPtr);
      MSG_LOW("ipl_shear_ycbcr marker_204\n");
      return IPL_NO_MEMORY;
    }
  }

  MSG_LOW("ipl_shear_ycbcr marker_1\n");
  
  /* Create full YCbCr input image */
  inIndex = 0;
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x+=2) {
      cb = *inImgPtr++;
      yOne = *inImgPtr++;
      cr = *inImgPtr++;
      yTwo = *inImgPtr++;
      *((uint8*)(fullImgPtr+inIndex++)) = cb;
      *((uint8*)(fullImgPtr+inIndex++)) = cr;
      *((uint8*)(fullImgPtr+inIndex++)) = yOne;
      *((uint8*)(fullImgPtr+inIndex++)) = cb;
      *((uint8*)(fullImgPtr+inIndex++)) = cr;
      *((uint8*)(fullImgPtr+inIndex++)) = yTwo;
    } /* end x loop */
  } /* end y loop */

  if ((alpha == 0) || (beta == 0)) {
    /* Wrap around output image to fit parallelogram */
    outIndex = 0;
    for(y = 0; y < h; y++) {
      for(x = 0; x < w; x++) {
        xin = (uint32) ((100+alpha*beta)*x - 10*alpha*y)/100;
        while (xin < 0) {
          xin += (int32) w;
        }
        xin %= (int32) w;
        yin = (10*y - beta*x)/10;
        while (yin < 0) {
          yin += (int32) h;
        }
        yin %= h;
        inIndex = (xin + yin*w)*3;
        *((uint8*)(fullOutImgPtr+outIndex++)) = 
          *((uint8*)(fullImgPtr+inIndex++));
        *((uint8*)(fullOutImgPtr+outIndex++)) = 
          *((uint8*)(fullImgPtr+inIndex++));
        *((uint8*)(fullOutImgPtr+outIndex++)) = 
          *((uint8*)(fullImgPtr+inIndex));
      } /* end xin loop */
    } /* end yin loop */
  } else {
    /* Create enlarged output image to fit parallelogram */
    outIndex = 0;
    if ((xmax-xmin+1)%2) {
      xmax++;
    }
    for(y = (uint32) ymin; y <= (uint32) ymax; y++) {
      for(x = (uint32) xmin; x <= (uint32) xmax; x++) {
        xin = ((100+alpha*beta)*x - 10*alpha*y)/100;
        yin = (10*y - beta*x)/10;
				if (xin >= 0 && (xin < (int32) w) && yin >= 0 && (yin < (int32) h)) 
        {
          inIndex = (xin + yin*w)*3;
          *((uint8*)(fullOutImgPtr+outIndex++)) = 
            *((uint8*)(fullImgPtr+inIndex++));
          *((uint8*)(fullOutImgPtr+outIndex++)) = 
            *((uint8*)(fullImgPtr+inIndex++));
          *((uint8*)(fullOutImgPtr+outIndex++)) = 
            *((uint8*)(fullImgPtr+inIndex));
        } else {
          *((uint8*)(fullOutImgPtr+outIndex++)) = fillerChroma;
          *((uint8*)(fullOutImgPtr+outIndex++)) = fillerChroma;
          *((uint8*)(fullOutImgPtr+outIndex++)) = fillerLuma;
        }
      } /* end x loop */
    } /* end y loop */
  } /* end if */

  /* Convert back to subsampled YCbCr 4:2:2 output */
  inIndex = 0;
  outIndex = 0;
  for (y = 0; y < hout; y++) {
    for (x = 0; x < wout; x+=2) {
      /* get Cb */
      *((uint8*)(outImgPtr + outIndex++)) =
        *((uint8*)(fullOutImgPtr + inIndex++));
      inIndex++;
      *((uint8*)(outImgPtr + outIndex++)) =
        *((uint8*)(fullOutImgPtr + inIndex++));
      /* get Cr */
      inIndex++;
      *((uint8*)(outImgPtr + outIndex++)) =
        *((uint8*)(fullOutImgPtr + inIndex++));
      *((uint8*)(outImgPtr + outIndex++)) =
        *((uint8*)(fullOutImgPtr + inIndex++));
    } /* end x loop */
  } /* end y loop */


  ipl_sys_free(fullImgPtr);
  ipl_sys_free(fullOutImgPtr);

  MSG_LOW("ipl_shear_ycbcr marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_shear_ycbcr */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_shear

DESCRIPTION
  This function performs shearing, which is an affine transformation.  
  The user specifies the horizontal and vertical shear factors, alpha 
  and beta, which can be any integer between negative infinity and 
  positive infinity. The output image size must be large enough to 
  fit the parallelogram-shaped result from shearing. The user must 
  specify a pixel value used to fill in blank output outside of the 
  parallelogram.
  
  Both input and output images must have the same color format, which  
  can be RGB565, YCbCr 4:2:2, YCbCr 4:2:0 line packed, or YCrCb 4:2:0 
  line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    points to the input image
  out_img_ptr   points to the output image
  alpha         horizontal shear factor
  beta          vertical shear factor
  fillerPixel   filler pixel value

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_shear
(
  ipl_image_type* i_img_ptr,   /* Points to the input image  */
  ipl_image_type* o_img_ptr,   /* Points to the output image */
  int32 alpha,                 /* Horizontal shear factor    */
  int32 beta,                  /* Vertical shear factor      */
  uint16 fillerPixel           /* Filler pixel value         */
)
{
  uint32 xmin, xmax, ymin, ymax, x, y, w, h;
  ipl_image_type outImg2;
  ipl_status_type retval;

  MSG_LOW("ipl_shear marker_0\n");
  
  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_shear marker_200\n");
    return IPL_FAILURE;
  }
  /* Input and output must have the same color format */
  if (i_img_ptr->cFormat != o_img_ptr->cFormat)
  {
    MSG_LOW("ipl_shear marker_201\n");
    return IPL_FAILURE;
  }
  /* Output width must be even for a YCbCr image */
  if ((i_img_ptr->cFormat == IPL_YCbCr) && (o_img_ptr->dx%2)) 
  {
    MSG_LOW("ipl_shear marker_202\n");
    return IPL_FAILURE;
  }

  /* Input and output image sizes must be equal */
  if ((i_img_ptr->dx != o_img_ptr->dx) ||
      (i_img_ptr->dy != o_img_ptr->dy)) 
  {
    MSG_LOW("ipl_shear marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_shear marker_1\n");

  if ((alpha == 0) || (beta == 0)) 
  {
    /* Call the appropriate function */
    if (i_img_ptr->cFormat == IPL_RGB565)
    {
      retval = ipl_shear_rgb565(i_img_ptr,o_img_ptr,alpha,beta,0,0,0,0,fillerPixel);
      if (retval != IPL_SUCCESS) 
      {
        MSG_LOW("ipl_shear marker_204\n");
        return retval;
      }
    } 
    else if (i_img_ptr->cFormat == IPL_YCbCr)
    {
      retval = ipl_shear_ycbcr(i_img_ptr,o_img_ptr,alpha,beta,0,0,0,0,fillerPixel);
      if (retval != IPL_SUCCESS) 
      {
        MSG_LOW("ipl_shear marker_205\n");
        return retval;
      }
    } 
    else if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
    {
      ipl_image_type innew_img, out422_img;

      /* allocate memory for YCbCr4:2:2 images */

      /* input 1 */
      innew_img.cFormat = IPL_YCbCr;
      innew_img.dx = i_img_ptr->dx;
      innew_img.dy = i_img_ptr->dy;
      if (ipl_malloc_img(&innew_img))
      {
        MSG_LOW("ipl_shear marker_206\n");
        return IPL_NO_MEMORY;
      }
      if (ipl_convert_image(i_img_ptr, &innew_img) != IPL_SUCCESS) 
      {
        ipl_free_img(&innew_img);
        MSG_LOW("ipl_shear marker_207\n");
        return IPL_NO_MEMORY;
      }

      /* ycbcr 4:2:2 output */
      out422_img.cFormat = IPL_YCbCr;
      out422_img.dx = o_img_ptr->dx;
      out422_img.dy = o_img_ptr->dy;
      if (ipl_malloc_img(&out422_img))
      {
        ipl_free_img(&innew_img);
        MSG_LOW("ipl_shear marker_208\n");
        return IPL_NO_MEMORY;
      }

      retval = ipl_shear_ycbcr(&innew_img, &out422_img, alpha, beta, 0, 0, 0, 0, fillerPixel);
      if (retval != IPL_SUCCESS) 
      {
        ipl_free_img(&innew_img);
        ipl_free_img(&out422_img);
        MSG_LOW("ipl_shear marker_209\n");
        return retval;
      }

      /* get 4:2:0 output */
      if (ipl_convert_image(&out422_img, o_img_ptr) != IPL_SUCCESS) 
      {
        ipl_free_img(&innew_img);
        ipl_free_img(&out422_img);
        MSG_LOW("ipl_shear marker_210\n");
        return IPL_FAILURE;
      }

      /* sys_free memory */
      ipl_free_img(&innew_img);
      ipl_free_img(&out422_img);
    } 
    else 
    {
      MSG_LOW("ipl_shear marker_211\n");
      /* other formats not supported */
      return IPL_FAILURE;
    }
  } 
  else 
  {
    /* 
    ** Output image size must be large enough to fit shear result.
    ** Find the necessary output size by checking 4 corners:
    ** (0,0), (0,h-1), (w-1,0), (w-1,h-1).
    */
    w = (int32)i_img_ptr->dx;
    h = (int32)i_img_ptr->dy;

    /* (0,0) will always map to (0,0) in shear result */
    xmin = 0;
    xmax = 0;
    ymin = 0;
    ymax = 0;

    /* (0,h-1) */
    x = (alpha*(h-1))/10;
    y = ((100+alpha*beta)*(h-1))/100;
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;

    /* (w-1,0) */
    x = w-1;
    y = (beta*(w-1))/10;
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;

    /* (w-1,h-1) */
    x = ((w-1)*10 + (alpha*(h-1)))/10;
    y = (10*beta*(w-1) + (100+alpha*beta)*(h-1))/100;
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;

    /* Large output */
    w = xmax-xmin+1;
    h = ymax-ymin+1;
    if ((o_img_ptr->cFormat == IPL_YCbCr ||
         o_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) && (w%2)) 
    {
      w++;
      xmax++;
    }
    if ((o_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) && (h%2))
    {
      h++;
      ymax++;
    }

      
    /* Call the appropriate function */
    if (i_img_ptr->cFormat == IPL_RGB565)
    {
      outImg2.dx = w;
      outImg2.dy = h;
      outImg2.cFormat = o_img_ptr->cFormat;
      if (ipl_malloc_img(&outImg2))
      {
        MSG_LOW("ipl_shear marker_212\n");
        return IPL_NO_MEMORY;
      }
      if (ipl_shear_rgb565(i_img_ptr, &outImg2, alpha, beta, xmin, xmax, 
                           ymin, ymax, fillerPixel) != IPL_SUCCESS) 
      {
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_213\n");
        return IPL_FAILURE;
      }

      /* Downsize to get final output image */
      if (ipl_downsize(&outImg2, o_img_ptr, NULL) != IPL_SUCCESS ) 
      {
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_214\n");
        return IPL_FAILURE;
      }
    } 
    else if (i_img_ptr->cFormat == IPL_YCbCr)
    {
      outImg2.dx = w;
      outImg2.dy = h;
      outImg2.cFormat = o_img_ptr->cFormat;
      if (ipl_malloc_img(&outImg2))
      {
        MSG_LOW("ipl_shear marker_215\n");
        return IPL_NO_MEMORY;
      }
      if (ipl_shear_ycbcr(i_img_ptr, &outImg2, alpha, beta, xmin, xmax, ymin, 
                          ymax, fillerPixel) != IPL_SUCCESS) 
      {
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_216\n");
        return IPL_FAILURE;
      }

      /* Downsize to get final output image */
      if (ipl_downsize(&outImg2, o_img_ptr, NULL) != IPL_SUCCESS ) 
      {
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_217\n");
        return IPL_FAILURE;
      }
    } 
    else if (i_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
    {
      ipl_image_type innew_img, out422_img;

      outImg2.dx = w;
      outImg2.dy = h;
      outImg2.cFormat = IPL_YCbCr;
      if (ipl_malloc_img(&outImg2))
      {
        MSG_LOW("ipl_shear marker_218\n");
        return IPL_NO_MEMORY;
      }
      /* input 1 */
      innew_img.cFormat = IPL_YCbCr;
      innew_img.dx = i_img_ptr->dx;
      innew_img.dy = i_img_ptr->dy;
      if (ipl_malloc_img(&innew_img))
      {
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_219\n");
        return IPL_NO_MEMORY;
      }

      if (ipl_convert_image(i_img_ptr, &innew_img) != IPL_SUCCESS) 
      {
        ipl_free_img(&innew_img);
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_220\n");
        return IPL_FAILURE;
      }

      /* ycbcr 4:2:2 output */
      out422_img.cFormat = IPL_YCbCr;
      out422_img.dx = i_img_ptr->dx;
      out422_img.dy = i_img_ptr->dy;
      if (ipl_malloc_img(&out422_img))
      {
        ipl_free_img(&innew_img);
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_221\n");
        return IPL_FAILURE;
      }

      /* call ycbcr 4:2:2 function */
      if (ipl_shear_ycbcr(&innew_img, &outImg2, alpha, beta, 
            xmin, xmax, ymin, ymax, fillerPixel) != IPL_SUCCESS) 
      {
        ipl_free_img(&innew_img);
        ipl_free_img(&out422_img);
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_222\n");
        return IPL_FAILURE;
      }

      /* Downsize to get final output image */
      if (ipl_downsize(&outImg2, &out422_img, NULL) != IPL_SUCCESS) 
      {
        ipl_free_img(&innew_img);
        ipl_free_img(&out422_img);
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_223\n");
        return IPL_FAILURE;
      }

      /* get 4:2:0 output */
      if (ipl_convert_image(&out422_img, o_img_ptr) != IPL_SUCCESS) 
      {
        ipl_free_img(&innew_img);
        ipl_free_img(&out422_img);
        ipl_free_img(&outImg2);
        MSG_LOW("ipl_shear marker_224\n");
        return IPL_FAILURE;
      }

      /* sys_free memory */
      ipl_free_img(&innew_img);
      ipl_free_img(&out422_img);
    } 
    else 
    {
      MSG_LOW("ipl_shear marker_225\n");
      /* other formats not supported */
      return IPL_FAILURE;
    }

    /* Free memory */
    ipl_free_img(&outImg2);
  }

  MSG_LOW("ipl_shear marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_shear */


#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_pinch_rgb565

DESCRIPTION
  This function pinches the input image in a circular region of 
  arbitrary location and radius.  Pixels in this region are pinched
  towards the center. The center of the pinched region must be 
  within input image boundaries. Areas outside the pinched region are 
  unchanged. If part of the pinched region extends beyond the image 
  boundaries, only the part within the boundaries is displayed. 

  Input and output image sizes must be equal.
  
  Both input and output images must be in RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    points to the input image
  out_img_ptr   points to the output image
  circle        circular region of pinching

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_pinch_rgb565
(
  ipl_image_type* in_img_ptr,  /* Points to the input image        */
  ipl_image_type* out_img_ptr, /* Points to the output image       */
  ipl_circle_type* circle      /* Circular region of pinching      */
)
{
  uint16 *inImgPtr, *outImgPtr, *blankPixels;
  uint32 outWidth, outHeight, xin, yin, xout, yout, rSq, 
         endx1, endy1, endx2, endy2, mapWidth, mapHeight, mapSize, 
         xytemp, outIndex, inIndex, mapIndex, nbIndex, inc,
         xo, yo, r;
  int16 *xMap, *yMap;
  int8 *nbXMap, *nbYMap;
  uint16 rsum, gsum, bsum;
  uint8 rval, gval, bval, rgbCnt;
  uint32 k1, k2, factor;
  int32 xa, ya, xan, yan;
  uint32 xb, yb, xbn, ybn;

  MSG_LOW("ipl_pinch_rgb565 marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !circle)
  {
    MSG_LOW("ipl_pinch_rgb565 marker_200\n");
    return IPL_FAILURE;
  }


  /* Initialize image pointers and local variables */
  inImgPtr = (uint16*)in_img_ptr->imgPtr;
  outImgPtr = (uint16*)out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;
  xo = circle->x;
  yo = circle->y;
  r = circle->r;

  /* Initialize index endpoints and local variables */
  if (xo < r) {
    endx1 = 0;
  } else {
    endx1 = xo - r;
  }
  endx2 = xo + r + 1;
  if (endx2 > outWidth) {
    endx2 = outWidth;
  }
  if (yo < r) {
    endy1 = 0;
  } else {
    endy1 = yo - r;
  }
  endy2 = yo + r + 1;
  if (endy2 > outHeight) {
    endy2 = outHeight;
  }
  mapWidth = endx2 - endx1;
  mapHeight = endy2 - endy1;
  mapSize = mapWidth * mapHeight;
  rSq = r*r;

  /*
  ** Malloc buffers for mapping output pixels to input pixels:
  ** xMap maps output (x,y) coordinates to input x-coordinates
  ** yMap maps output (x,y) coordinates to input y-coordinates
  */
  xMap = ipl_malloc(mapSize*sizeof(int16));
  if (!xMap) { 
    MSG_LOW("ipl_pinch_rgb565 marker_201\n");
    return IPL_NO_MEMORY;
  }
  yMap = ipl_malloc(mapSize*sizeof(int16));
  if (!yMap) 
  {  
    ipl_sys_free(xMap);
    MSG_LOW("ipl_pinch_rgb565 marker_202\n");
    return IPL_NO_MEMORY;
  }
  
  MSG_LOW("ipl_pinch_rgb565 marker_1\n");
  
  /* Initialize xMap and yMap with "blank" points (-1, -1) */
  for (mapIndex = 0; mapIndex < mapSize; mapIndex++) {
    *((int16*)(xMap + mapIndex)) = -1;
  }
  for (mapIndex = 0; mapIndex < mapSize; mapIndex++) {
    *((int16*)(yMap + mapIndex)) = -1;
  }

  /* Create xMap, yMap: mapping from (xout, yout) to (xin, yin) */
  k1 = 5767168 / r;
  k2 = 2048 / r;
  for (yin = endy1; yin < endy2; yin++) {
    for (xin = endx1; xin < endx2; xin++) {
      xytemp = (uint32)((int32)(xin-xo)*(int32)(xin-xo) +
                (int32)(yin-yo)*(int32)(yin-yo));
      if (xytemp <= rSq) {
        /* within magnified region */
        /* (1) xin, yin */
        factor = 8388608 - (5767168-xytemp*k1/r) +
                 (2048-xytemp*k2/r)*(1024-(xytemp*k2>>1)/r) - 
                 (128-xytemp*128/rSq)*(64-xytemp*64/rSq)*
                 (64-xytemp*64/rSq);
        xout = (uint32)(xo + (((int32)factor*(int32)(xin-xo))>>23));
        yout = (uint32)(yo + (((int32)factor*(int32)(yin-yo))>>23));
        if (xout >= endx1 && xout < endx2 && yout >= endy1 && 
            yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                    (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

    /* values used in the calculations below */
        xa = ((int32)(xin - xo) << 3) + 3;
        ya = ((int32)(yin - yo) << 3) + 3;
        xan = ((int32)(xin - xo) << 3) - 3;
        yan = ((int32)(yin - yo) << 3) - 3;
        xb = (uint32)(xo+(((int32)factor*(int32)(xin-xo+1))>>23));
        yb = (uint32)(yo+(((int32)factor*(int32)(yin-yo+1))>>23));
        xbn = (uint32)(xo+(((int32)factor*(int32)(xin-xo-1))>>23));
        ybn = (uint32)(yo+(((int32)factor*(int32)(yin-yo-1))>>23));

        /* (2) xin + 0.4, yin (in fixed point arithmetic) */
        xout = (uint32)(xo + ((((int32)factor>>3)*xa)>>23));
        /* yout already calculated */
        /* Check xin + 1, yin */
        if (xout != xb && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                    (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (3) xin + 0.4, yin + 0.4 (in fixed point arithmetic)*/
        /* xout already calculated */
        yout = (uint32)(yo + ((((int32)factor>>3)*ya)>>23));
        /* Check xin + 1, yin + 1 */
        if (xout != xb && yout != yb && xout >= endx1 && 
            xout < endx2 && yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (4) xin, yin + 0.4 (in fixed point arithmetic) */
        xout = (uint32)(xo + (((int32)factor*(int32)(xin-xo))>>23));
        /* yout already calculated */
        /* Check xin, yin + 1 */
        if (yout != yb && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (5) xin, yin - 0.4 (in fixed point arithmetic) */
        /* xout already calculated */
        yout = (uint32)(yo + ((((int32)factor>>3)*yan)>>23));
        /* Check xin, yin - 1 */
        if (yout != ybn && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (6) xin - 0.4, yin - 0.4 (in fixed point arithmetic) */
        xout = (uint32)(xo + ((((int32)factor>>3)*xan)>>23));
        /* yout already calculated */
        /* Check xin - 1, yin - 1 */
        if (xout != xbn && yout != ybn && xout >= endx1 && 
            xout < endx2 && yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (7) xin - 0.4, yin (in fixed point arithmetic) */
        /* xout already calculated */
        yout = (uint32)(yo + (((int32)factor*(int32)(yin-yo))>>23));
        /* Check xin - 1, yin */
        if (xout != xbn && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */
      } /* end if (xin, yin) within magnified region */
    } /* end yin loop */
  } /* end xin loop */

  MSG_LOW("ipl_pinch_rgb565 marker_2\n");
  /*
  ** Fill in output pixels in the following order:
  ** |---------------------------|
  ** |            (1)            |
  ** |---------------------------|
  ** | (2a)    |  (2b)  |   (2c) |
  ** |---------------------------|
  ** |            (3)            |
  ** |---------------------------|
  */
  /* (1) non-magnified region ABOVE magnified region */
  outIndex = 0;
  for (yout = 0; yout < endy1; yout++) {
    for (xout = 0; xout < outWidth; xout++) {
      *((uint16*)(outImgPtr+outIndex)) = 
        *((uint16*)(inImgPtr+outIndex));
      outIndex++;
    }
  } /* end (1) */

  /* (2) region around magnified region */
  mapIndex = 0;
  for (yout = endy1; yout < endy2; yout++) {
    /* (2a) non-magnified region to the left of the magnified area */
    for (xout = 0; xout < endx1; xout++) {
      *((uint16*)(outImgPtr+outIndex)) = 
        *((uint16*)(inImgPtr+outIndex));
      outIndex++;
    }
    /* (2b) region affected by magnification */
    for (xout = endx1; xout < endx2; xout++) {
    if (*((int16*)(xMap + mapIndex)) == -1) {
        *((uint16*)(outImgPtr + outIndex++)) = 0;
      } else {
        inIndex = *((int16*)(xMap+mapIndex)) + 
                  *((int16*)(yMap+mapIndex)) * outWidth;
        *((uint16*)(outImgPtr+outIndex++)) = *((uint16*)(inImgPtr+inIndex));
      }
      mapIndex++;
    }
    /* (2c) non-magnified region to the right of the magnified area */
    for (xout = endx2; xout < outWidth; xout++) {
      *((uint16*)(outImgPtr+outIndex)) = 
        *((uint16*)(inImgPtr+outIndex));
      outIndex++;
    }
  } /* end (2) */

  /* (3) non-magnified region BELOW magnified area */
  for (yout = endy2; yout < outHeight; yout++) {
    for (xout = 0; xout < outWidth; xout++) {
      *((uint16*)(outImgPtr+outIndex)) = 
        *((uint16*)(inImgPtr+outIndex));
      outIndex++;
    }
  } /* end (3) */

  /*
  ** Malloc 3 buffers:
  ** blankPixels stores interpolated pixel values for 
  **     pixels left blank in the previous step
  ** nbXMap, nbYMap stores the order in which  
  **     neighboring pixels are used during the 
  **     interpolation process
  */
  blankPixels = ipl_malloc(mapSize*sizeof(uint16));
  if (!blankPixels) 
  {
    ipl_sys_free(xMap);
    ipl_sys_free(yMap);
    MSG_LOW("ipl_pinch_rgb565 marker_203\n");
    return IPL_FAILURE;
  }

  nbXMap = ipl_malloc(12*sizeof(int8));
  if (!nbXMap) 
  { 
    ipl_sys_free(xMap);
    ipl_sys_free(yMap);
    ipl_sys_free(blankPixels);
    MSG_LOW("ipl_pinch_rgb565 marker_204\n");
    return IPL_NO_MEMORY;
  }

  nbYMap = ipl_malloc(12*sizeof(int8));
  if (!nbYMap) 
  {
    ipl_sys_free(xMap);
    ipl_sys_free(yMap);
    ipl_sys_free(nbXMap);
    ipl_sys_free(blankPixels);
    MSG_LOW("ipl_pinch_rgb565 marker_205\n");
    return IPL_NO_MEMORY;
  }

  /* Initialize nbXMap, nbYMap */
  *nbXMap = -1;
  *((int8*)(nbXMap+1)) = 0;
  *((int8*)(nbXMap+2)) = 1;
  *((int8*)(nbXMap+3)) = -1;
  *((int8*)(nbXMap+4)) = 1;
  *((int8*)(nbXMap+5)) = -1;
  *((int8*)(nbXMap+6)) = 0;
  *((int8*)(nbXMap+7)) = -1;
  *((int8*)(nbXMap+8)) = 0;
  *((int8*)(nbXMap+9)) = -2;
  *((int8*)(nbXMap+10)) = 0;
  *((int8*)(nbXMap+11)) = 2;
  *nbYMap = -1;
  *((int8*)(nbYMap+1)) = -1;
  *((int8*)(nbYMap+2)) = -1;
  *((int8*)(nbYMap+3)) = 0;
  *((int8*)(nbYMap+4)) = 0;
  *((int8*)(nbYMap+5)) = 1;
  *((int8*)(nbYMap+6)) = 1;
  *((int8*)(nbYMap+7)) = 1;
  *((int8*)(nbYMap+8)) = -2;
  *((int8*)(nbYMap+9)) = 0;
  *((int8*)(nbYMap+10)) = 2;
  *((int8*)(nbYMap+11)) = 0;
    
  /* Compute missing values for area affected by magnification */
  mapIndex = 0;
  for (yout = endy1; yout < endy2; yout++) {
    for (xout = endx1; xout < endx2; xout++) {
      if (*((int16*)(xMap + mapIndex)) == -1) {
        /* blank pixel */
        xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                 (int32)(yout-yo)*(int32)(yout-yo));
        if (xytemp > rSq) {
          /* outside magnified region */
          inIndex = xout + yout * outWidth;
          *((uint16*)(blankPixels+mapIndex)) = 
            *((uint16*)(inImgPtr+inIndex));
        } else {
          /* inside magnified region */
          nbIndex = 0;
          rsum = 0;
          gsum = 0;
          bsum = 0;
          rgbCnt = 0;
          while (nbIndex < 12) {
            /* neighboring pixel */
            xin = (uint32)((int32)xout + 
                  (int32)*((int8*)(nbXMap+nbIndex)));
            yin = (uint32)((int32)yout + 
                  (int32)*((int8*)(nbYMap+nbIndex)));
            if (xin < outWidth && yin < outHeight) {
              /* within image boundaries */
              outIndex = xin + yin * outWidth;
              /* unpack to get rgb values */
              rval = (uint8)((*((uint16*)(outImgPtr+outIndex)) 
                      & 0xF800)>>8);
              gval = (uint8)((*((uint16*)(outImgPtr+outIndex)) 
                      & 0x07E0)>>3);
              bval = (uint8)((*((uint16*)(outImgPtr+outIndex)) 
                      & 0x001F)<<3);
              if (rval != 0 || gval != 0 || bval != 0) {
                rsum += rval;
                gsum += gval;
                bsum += bval;
                rgbCnt++;
              }
            }
            nbIndex++;
          } /* end while loop */
          /* get average */
          if (rgbCnt > 0) {
            rsum /= rgbCnt;
            gsum /= rgbCnt;
            bsum /= rgbCnt;
          }
          /* pack into rgb565 format */
          *((uint16*)(blankPixels + mapIndex)) = 
            pack_rgb565(rsum, gsum, bsum);
        } /* end if blank pixel outside magnified region */
      } /* end if blank pixel */
      else {
        /* non-blank pixel */
        outIndex = xout + yout * outWidth;
        *((uint16*)(blankPixels + mapIndex)) = 
          *((uint16*)(outImgPtr + outIndex));
      }
      /* increment the index */
      mapIndex++;
    } /* end xout loop */
  } /* end yout loop */

  /*
  ** Fill in missing values in final output for area 
  ** affected by magnification
  */
  mapIndex = 0;
  inc = outWidth - mapWidth;
  outIndex = endx1 + endy1 * outWidth;
  for (yout = endy1; yout < endy2; yout++) {
    for (xout = endx1; xout < endx2; xout++) {
      *((uint16*)(outImgPtr + outIndex++)) =  
        *((uint16*)(blankPixels + mapIndex++));
    }
    outIndex += inc;
  }

  /* Free memory */
  ipl_sys_free(xMap);
  ipl_sys_free(yMap);
  ipl_sys_free(blankPixels);
  ipl_sys_free(nbXMap);
  ipl_sys_free(nbYMap);

  MSG_LOW("ipl_pinch_rgb565 marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_pinch_rgb565 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_pinch_ycbcr

DESCRIPTION
  This function pinches the input image in a circular region of 
  arbitrary location and radius.  Pixels in this region are pinched
  towards the center. The center of the pinched region must be 
  within input image boundaries. Areas outside the pinched region are 
  unchanged. If part of the pinched region extends beyond the image 
  boundaries, only the part within the boundaries is displayed. 

  Input and output image sizes must be equal.
  
  Input and output images must be in YCbCr 4:2:2 format.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    points to the input image
  out_img_ptr   points to the output image
  circle        circular region of pinching

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_pinch_ycbcr
(
  ipl_image_type* in_img_ptr,  /* Points to the input image        */
  ipl_image_type* out_img_ptr, /* Points to the output image       */
  ipl_circle_type* circle      /* Circular region of pinching      */
)
{
  unsigned char *inImgPtr, *outImgPtr, *blankPixels, *fullImgPtr, 
                *outFullImgPtr;
  unsigned char cb, cr, luma1, luma2;
  uint32 outWidth, outHeight, xin, yin, xout, yout, rSq, 
         endx1, endy1, endx2, endy2, mapWidth, mapHeight, mapSize, 
         xytemp, outIndex, inIndex, mapIndex, nbIndex, inc,
     xo, yo, r;
  int16 *xMap, *yMap;
  int8 *nbXMap, *nbYMap;
  uint16 lumasum, cbsum, crsum;
  uint8 cnt;
  uint32 k1, k2, factor;
  int32 xa, ya, xan, yan;
  uint32 xb, yb, xbn, ybn;

  MSG_LOW("ipl_pinch_ycbcr marker_0\n");
  
  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !circle)
  {
    MSG_LOW("ipl_pinch_ycbcr marker_200\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = in_img_ptr->imgPtr;
  outImgPtr = out_img_ptr->imgPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;
  xo = circle->x;
  yo = circle->y;
  r = circle->r;
  fullImgPtr = ipl_malloc(outWidth*outHeight*3*sizeof(unsigned char));
  if (!fullImgPtr) 
  {
    MSG_LOW("ipl_pinch_ycbcr marker_201\n");
    return IPL_NO_MEMORY;
  }
  outFullImgPtr = ipl_malloc(outWidth*outHeight*3*sizeof(unsigned char));
  if (!outFullImgPtr) 
  {
    ipl_sys_free(fullImgPtr);
    MSG_LOW("ipl_pinch_ycbcr marker_202\n");
    return IPL_NO_MEMORY;
  }

  /* Initialize index endpoints and local variables */
  if (xo < r) {
    endx1 = 0;
  } else {
    endx1 = xo - r;
  }
  endx2 = xo + r + 1;
  if (endx2 > outWidth) {
    endx2 = outWidth;
  }
  if (yo < r) {
    endy1 = 0;
  } else {
    endy1 = yo - r;
  }
  endy2 = yo + r + 1;
  if (endy2 > outHeight) {
    endy2 = outHeight;
  }
  mapWidth = endx2 - endx1;
  mapHeight = endy2 - endy1;
  mapSize = mapWidth * mapHeight;
  rSq = r*r;

  /*
  ** Malloc buffers for mapping output pixels to input pixels:
  ** xMap maps output (x,y) coordinates to input x-coordinates
  ** yMap maps output (x,y) coordinates to input y-coordinates
  */
  xMap = ipl_malloc(mapSize*sizeof(int16));
  if (!xMap) 
  { 
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    MSG_LOW("ipl_pinch_ycbcr marker_203\n");
    return IPL_NO_MEMORY;
  }
  yMap = ipl_malloc(mapSize*sizeof(int16));
  if (!yMap) 
  {
    ipl_sys_free(xMap);
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    MSG_LOW("ipl_pinch_ycbcr marker_204\n");
    return IPL_NO_MEMORY;
  }

  MSG_LOW("ipl_pinch_ycbcr marker_1\n");

  /* Initialize xMap and yMap with "blank" points (-1, -1) */
  for (mapIndex = 0; mapIndex < mapSize; mapIndex++) {
    *((int16*)(xMap + mapIndex)) = -1;
  }
  for (mapIndex = 0; mapIndex < mapSize; mapIndex++) {
    *((int16*)(yMap + mapIndex)) = -1;
  }

  /* Create xMap, yMap: mapping from (xout, yout) to (xin, yin) */
  k1 = 5767168 / r;
  k2 = 2048 / r;
  for (yin = endy1; yin < endy2; yin++) {
    for (xin = endx1; xin < endx2; xin++) {
      xytemp = (uint32)((int32)(xin-xo)*(int32)(xin-xo) +
               (int32)(yin-yo)*(int32)(yin-yo));
      if (xytemp <= rSq) {
        /* within magnified region */
        /* (1) xin, yin */
        factor = 8388608 - (5767168-xytemp*k1/r) +
                 (2048-xytemp*k2/r)*(1024-(xytemp*k2>>1)/r) - 
                 (128-xytemp*128/rSq)*(64-xytemp*64/rSq)*
                 (64-xytemp*64/rSq);
        xout = (uint32)(xo + (((int32)factor*(int32)(xin-xo))>>23));
        yout = (uint32)(yo + (((int32)factor*(int32)(yin-yo))>>23));
        if (xout >= endx1 && xout < endx2 && yout >= endy1 && 
            yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

    /* values used in the calculations below */
        xa = ((int32)(xin - xo) << 3) + 3;
        ya = ((int32)(yin - yo) << 3) + 3;
        xan = ((int32)(xin - xo) << 3) - 3;
        yan = ((int32)(yin - yo) << 3) - 3;
        xb = (uint32)(xo+(((int32)factor*(int32)(xin-xo+1))>>23));
        yb = (uint32)(yo+(((int32)factor*(int32)(yin-yo+1))>>23));
        xbn = (uint32)(xo+(((int32)factor*(int32)(xin-xo-1))>>23));
        ybn = (uint32)(yo+(((int32)factor*(int32)(yin-yo-1))>>23));

        /* (2) xin + 0.4, yin (in fixed point arithmetic) */
        xout = (uint32)(xo + ((((int32)factor>>3)*xa)>>23));
        /* yout already calculated */
        /* Check xin + 1, yin */
        if (xout != xb && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (3) xin + 0.4, yin + 0.4 (in fixed point arithmetic) */
        /* xout already calculated */
        yout = (uint32)(yo + ((((int32)factor>>3)*ya)>>23));
        /* Check xin + 1, yin + 1 */
        if (xout != xb && yout != yb && xout >= endx1 && 
            xout < endx2 && yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (4) xin, yin + 0.4 (in fixed point arithmetic) */
        xout = (uint32)(xo + (((int32)factor*(int32)(xin-xo))>>23));
        /* yout already calculated */
        /* Check xin, yin + 1 */
        if (yout != yb && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (5) xin, yin - 0.4 (in fixed point arithmetic) */
        /* xout already calculated */
        yout = (uint32)(yo + ((((int32)factor>>3)*yan)>>23));
        /* Check xin, yin - 1 */
        if (yout != ybn && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (6) xin - 0.4, yin - 0.4 (in fixed point arithmetic) */
        xout = (uint32)(xo + ((((int32)factor>>3)*xan)>>23));
        /* yout already calculated */
        /* Check xin - 1, yin - 1 */
        if (xout != xbn && yout != ybn && xout >= endx1 && 
            xout < endx2 && yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (7) xin - 0.4, yin (in fixed point arithmetic) */
        /* xout already calculated */
        yout = (uint32)(yo + (((int32)factor*(int32)(yin-yo))>>23));
        /* Check xin - 1, yin */
        if (xout != xbn && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

      } /* end if (xin, yin) within magnified region */
    } /* end yin loop */
  } /* end xin loop */

  /* Create full YCbCr image */
  outIndex = 0;
  for (yout = 0; yout < outHeight; yout++) {
    for (xout = 0; xout < outWidth; xout+=2) {
      cb = *inImgPtr++;
      luma1 = *inImgPtr++;
      cr = *inImgPtr++;
      luma2 = *inImgPtr++;
      *((unsigned char*)(fullImgPtr+outIndex++)) = cb;
      *((unsigned char*)(fullImgPtr+outIndex++)) = cr;
      *((unsigned char*)(fullImgPtr+outIndex++)) = luma1;
      *((unsigned char*)(fullImgPtr+outIndex++)) = cb;
      *((unsigned char*)(fullImgPtr+outIndex++)) = cr;
      *((unsigned char*)(fullImgPtr+outIndex++)) = luma2;
    } /* end xout loop */
  } /* end yout loop */

  MSG_LOW("ipl_pinch_ycbcr marker_2\n");

  /*
  ** Fill in output pixels in the following order:
  ** |---------------------------|
  ** |            (1)            |
  ** |---------------------------|
  ** | (2a)    |  (2b)  |   (2c) |
  ** |---------------------------|
  ** |            (3)            |
  ** |---------------------------|
  */
  /* (1) non-magnified region ABOVE magnified region */
  outIndex = 0;
  for (yout = 0; yout < endy1; yout++) {
    for (xout = 0; xout < outWidth; xout++) {
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
    }
  } /* end (1) */

  /* (2) region around magnified region */
  mapIndex = 0;
  for (yout = endy1; yout < endy2; yout++) {
    /* (2a) non-magnified region to the left of the magnified area */
    for (xout = 0; xout < endx1; xout++) {
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
    }
    /* (2b) region affected by magnification */
    for (xout = endx1; xout < endx2; xout++) {
      if (*((int16*)(xMap + mapIndex)) == -1) {
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 128;
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 128;
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 16;
      } else {
        inIndex = (*((int16*)(xMap+mapIndex)) + 
               *((int16*)(yMap+mapIndex)) * outWidth)*3;
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 
          *((unsigned char*)(fullImgPtr + inIndex++));
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 
          *((unsigned char*)(fullImgPtr + inIndex++));
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 
          *((unsigned char*)(fullImgPtr + inIndex));
      }
      mapIndex++;
    }
    /* (2c) non-magnified region to the right of the magnified area */
    for (xout = endx2; xout < outWidth; xout++) {
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
    }
  } /* end (2) */

  /* (3) non-magnified region BELOW magnified area */
  for (yout = endy2; yout < outHeight; yout++) {
    for (xout = 0; xout < outWidth; xout++) {
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
    }
  } /* end (3) */

  /*
  ** Malloc 3 buffers:
  ** blankPixels stores interpolated pixel values for 
  **     pixels left blank in the previous step
  ** nbXMap, nbYMap stores the order in which  
  **     neighboring pixels are used during the 
  **     interpolation process
  */
  blankPixels = ipl_malloc(mapSize*3*sizeof(unsigned char));
  if (!blankPixels) 
  {
    ipl_sys_free(xMap);
    ipl_sys_free(yMap);
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    MSG_LOW("ipl_pinch_ycbcr marker_205\n");
    return IPL_NO_MEMORY;
  }
  nbXMap = ipl_malloc(12*sizeof(int8));
  if (!nbXMap) 
  {   
    ipl_sys_free(xMap);
    ipl_sys_free(yMap);
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    ipl_sys_free(blankPixels);
    MSG_LOW("ipl_pinch_ycbcr marker_206\n");
    return IPL_NO_MEMORY;
  }
  nbYMap = ipl_malloc(12*sizeof(int8));
  if (!nbYMap) 
  {  
    ipl_sys_free(xMap);
    ipl_sys_free(yMap);
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    ipl_sys_free(blankPixels);
    ipl_sys_free(nbXMap);
    MSG_LOW("ipl_pinch_ycbcr marker_207\n");
    return IPL_NO_MEMORY;
  }

  /* Initialize nbXMap, nbYMap */
  *nbXMap = -1;
  *((int8*)(nbXMap+1)) = 0;
  *((int8*)(nbXMap+2)) = 1;
  *((int8*)(nbXMap+3)) = -1;
  *((int8*)(nbXMap+4)) = 1;
  *((int8*)(nbXMap+5)) = -1;
  *((int8*)(nbXMap+6)) = 0;
  *((int8*)(nbXMap+7)) = -1;
  *((int8*)(nbXMap+8)) = 0;
  *((int8*)(nbXMap+9)) = -2;
  *((int8*)(nbXMap+10)) = 0;
  *((int8*)(nbXMap+11)) = 2;
  *nbYMap = -1;
  *((int8*)(nbYMap+1)) = -1;
  *((int8*)(nbYMap+2)) = -1;
  *((int8*)(nbYMap+3)) = 0;
  *((int8*)(nbYMap+4)) = 0;
  *((int8*)(nbYMap+5)) = 1;
  *((int8*)(nbYMap+6)) = 1;
  *((int8*)(nbYMap+7)) = 1;
  *((int8*)(nbYMap+8)) = -2;
  *((int8*)(nbYMap+9)) = 0;
  *((int8*)(nbYMap+10)) = 2;
  *((int8*)(nbYMap+11)) = 0;
    
  /* Compute missing values for area affected by magnification */
  mapIndex = 0;
  outIndex = 0;
  for (yout = endy1; yout < endy2; yout++) {
    for (xout = endx1; xout < endx2; xout++) {
      if (*((int16*)(xMap + mapIndex)) == -1) {
        /* blank pixel */
        xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                 (int32)(yout-yo)*(int32)(yout-yo));
        if (xytemp > rSq) {
          /* outside magnified region */
          inIndex = (xout + yout * outWidth) * 3;
          *((unsigned char*)(blankPixels + outIndex++)) = 
            *((unsigned char*)(fullImgPtr + inIndex++));
          *((unsigned char*)(blankPixels + outIndex++)) = 
            *((unsigned char*)(fullImgPtr + inIndex++));
          *((unsigned char*)(blankPixels + outIndex++)) = 
            *((unsigned char*)(fullImgPtr + inIndex));
        } else {
          /* inside magnified region */
          nbIndex = 0;
          lumasum = 0;
          crsum = 0;
          cbsum = 0;
          cnt = 0;
          while (nbIndex < 12) {
            /* neighboring pixel */
            xin = (uint32)((int32)xout + 
                  (int32)*((int8*)(nbXMap+nbIndex)));
            yin = (uint32)((int32)yout + 
                  (int32)*((int8*)(nbYMap+nbIndex)));
            if (xin < outWidth && yin < outHeight) {
              /* within image boundaries */
              inIndex = (xin + yin * outWidth) * 3;
              cb = *((unsigned char*)(outFullImgPtr+inIndex++));
              cr = *((unsigned char*)(outFullImgPtr+inIndex++));
              luma1 = *((unsigned char*)(outFullImgPtr+inIndex));
              if (cb != 128 || cr != 128 || luma1 != 16) {
                cbsum += cb;
                crsum += cr;
                lumasum += luma1;
                cnt++;
              }
            }
            nbIndex++;
          } /* end while loop */
          /* get average */
          if (cnt > 0) {
            cbsum /= cnt;
            crsum /= cnt;
            lumasum /= cnt;
          } else {
            cbsum = 128;
            crsum = 128;
            lumasum = 16;
          }
          /* set output pixel value */
          *((unsigned char*)(blankPixels+outIndex++)) = 
            (unsigned char)cbsum;
          *((unsigned char*)(blankPixels+outIndex++)) = 
            (unsigned char)crsum;
          *((unsigned char*)(blankPixels+outIndex++)) = 
            (unsigned char)lumasum;
        } /* end if blank pixel outside magnified region */
      } /* end if blank pixel */
      else {
        /* non-blank pixel */
        inIndex = (xout + yout * outWidth) * 3;
        *((unsigned char*)(blankPixels + outIndex++)) = 
          *((unsigned char*)(outFullImgPtr + inIndex++));
        *((unsigned char*)(blankPixels + outIndex++)) = 
          *((unsigned char*)(outFullImgPtr + inIndex++));
        *((unsigned char*)(blankPixels + outIndex++)) = 
          *((unsigned char*)(outFullImgPtr + inIndex));
      }
      /* increment the index */
      mapIndex++;
    } /* end xout loop */
  } /* end yout loop */

  /*
  ** Fill in missing values in full YCbCr output for area 
  ** affected by magnification
  */
  inc = (outWidth - mapWidth)*3;
  outIndex = (endx1 + endy1 * outWidth)*3;
  inIndex = 0;
  for (yout = endy1; yout < endy2; yout++) {
    for (xout = endx1; xout < endx2; xout++) {
      *((unsigned char*)(outFullImgPtr + outIndex++)) = 
        *((unsigned char*)(blankPixels + inIndex++));
      *((unsigned char*)(outFullImgPtr + outIndex++)) = 
        *((unsigned char*)(blankPixels + inIndex++));
      *((unsigned char*)(outFullImgPtr + outIndex++)) = 
        *((unsigned char*)(blankPixels + inIndex++));
    }
    outIndex += inc;
  }

  /* Get YCbCr 4:2:2 output */
  inIndex = 0;
  for (yout = 0; yout < outHeight; yout++) 
  {
    for (xout = 0; xout < outWidth; xout+=2) 
    {
      /* get Cb */
      *outImgPtr++ = *((unsigned char*)(outFullImgPtr + inIndex++));
      inIndex++;
      *outImgPtr++ = *((unsigned char*)(outFullImgPtr + inIndex++));

      /* get Cr */
      inIndex++;
      *outImgPtr++ = *((unsigned char*)(outFullImgPtr + inIndex++));
      *outImgPtr++ = *((unsigned char*)(outFullImgPtr + inIndex++));
    } /* end xout loop */
  } /* end yout loop */

  /* Free memory */
  ipl_sys_free(xMap);
  ipl_sys_free(yMap);
  ipl_sys_free(blankPixels);
  ipl_sys_free(nbXMap);
  ipl_sys_free(nbYMap);
  ipl_sys_free(fullImgPtr);
  ipl_sys_free(outFullImgPtr);

  MSG_LOW("ipl_pinch_ycbcr marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_pinch_ycbcr */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_pinch_420lp

DESCRIPTION
  This function pinches the input image in a circular region of 
  arbitrary location and radius.  Pixels in this region are pinched
  towards the center. The center of the pinched region must be 
  within input image boundaries. Areas outside the pinched region are 
  unchanged. If part of the pinched region extends beyond the image 
  boundaries, only the part within the boundaries is displayed. 

  Input and output image sizes must be equal.
  
  Input and output images must have the same color format, which 
  can be either YCbCr 4:2:0 line packed or YCrCb 4:2:0 line packed format.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    points to the input image
  out_img_ptr   points to the output image
  circle        circular region of pinching

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_pinch_420lp
(
  ipl_image_type* in_img_ptr,  /* Points to the input image        */
  ipl_image_type* out_img_ptr, /* Points to the output image       */
  ipl_circle_type* circle      /* Circular region of pinching      */
)
{
  unsigned char *inImgPtr, *outImgPtr, *blankPixels, *fullImgPtr, 
                *outFullImgPtr, *inClrPtr, *outClrPtr, *in2ImgPtr,
                *out2ImgPtr, *full2ImgPtr, *outFull2ImgPtr;
  unsigned char cb, cr, luma;
  uint32 outWidth, outHeight, xin, yin, xout, yout, rSq, 
         endx1, endy1, endx2, endy2, mapWidth, mapHeight, mapSize, 
         xytemp, outIndex, inIndex, mapIndex, nbIndex, inc,
         xo, yo, r;
  int16 *xMap, *yMap;
  int8 *nbXMap, *nbYMap;
  uint16 lumasum, cbsum, crsum;
  uint8 cnt;
  uint32 k1, k2, factor;
  int32 xa, ya, xan, yan;
  uint32 xb, yb, xbn, ybn;

  MSG_LOW("ipl_pinch_420lp marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !circle)
  {
    MSG_LOW("ipl_pinch_420lp marker_200\n");
    return IPL_FAILURE;
  }
  /* Initialize image pointers and local variables */
  inImgPtr = in_img_ptr->imgPtr;
  inClrPtr = in_img_ptr->clrPtr;
  outImgPtr = out_img_ptr->imgPtr;
  outClrPtr = out_img_ptr->clrPtr;
  outWidth = out_img_ptr->dx;
  outHeight = out_img_ptr->dy;
  xo = circle->x;
  yo = circle->y;
  r = circle->r;
  fullImgPtr = ipl_malloc(outWidth*outHeight*3*sizeof(unsigned char));
  if (!fullImgPtr) {
    MSG_LOW("ipl_pinch_420lp marker_201\n");
    return IPL_NO_MEMORY;
  }
  outFullImgPtr = ipl_malloc(outWidth*outHeight*3*sizeof(unsigned char));
  if (!outFullImgPtr) 
  {
    ipl_sys_free(fullImgPtr);
    MSG_LOW("ipl_pinch_420lp marker_202\n");
    return IPL_NO_MEMORY;
  }

  /* Initialize index endpoints and local variables */
  if (xo < r) {
    endx1 = 0;
  } else {
    endx1 = xo - r;
  }
  endx2 = xo + r + 1;
  if (endx2 > outWidth) {
    endx2 = outWidth;
  }
  if (yo < r) {
    endy1 = 0;
  } else {
    endy1 = yo - r;
  }
  endy2 = yo + r + 1;
  if (endy2 > outHeight) {
    endy2 = outHeight;
  }
  mapWidth = endx2 - endx1;
  mapHeight = endy2 - endy1;
  mapSize = mapWidth * mapHeight;
  rSq = r*r;

  /*
  ** Malloc buffers for mapping output pixels to input pixels:
  ** xMap maps output (x,y) coordinates to input x-coordinates
  ** yMap maps output (x,y) coordinates to input y-coordinates
  */
  xMap = ipl_malloc(mapSize*sizeof(int16));
  if (!xMap) 
  { 
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    MSG_LOW("ipl_pinch_420lp marker_203\n");
    return IPL_NO_MEMORY;
  }

  yMap = ipl_malloc(mapSize*sizeof(int16));
  if (!yMap) 
  {
    ipl_sys_free(xMap);
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    MSG_LOW("ipl_pinch_420lp marker_204\n");
    return IPL_NO_MEMORY;
  }

  MSG_LOW("ipl_pinch_420lp marker_1\n");

  /* Initialize xMap and yMap with "blank" points (-1, -1) */
  for (mapIndex = 0; mapIndex < mapSize; mapIndex++) {
    *((int16*)(xMap + mapIndex)) = -1;
  }
  for (mapIndex = 0; mapIndex < mapSize; mapIndex++) {
    *((int16*)(yMap + mapIndex)) = -1;
  }

  /* Create xMap, yMap: mapping from (xout, yout) to (xin, yin) */
  k1 = 5767168 / r;
  k2 = 2048 / r;
  for (yin = endy1; yin < endy2; yin++) {
    for (xin = endx1; xin < endx2; xin++) {
      xytemp = (uint32)((int32)(xin-xo)*(int32)(xin-xo) +
               (int32)(yin-yo)*(int32)(yin-yo));
      if (xytemp <= rSq) {
        /* within magnified region */
        /* (1) xin, yin */
        factor = 8388608 - (5767168-xytemp*k1/r) +
                 (2048-xytemp*k2/r)*(1024-(xytemp*k2>>1)/r) - 
                 (128-xytemp*128/rSq)*(64-xytemp*64/rSq)*
                 (64-xytemp*64/rSq);
        xout = (uint32)(xo + (((int32)factor*(int32)(xin-xo))>>23));
        yout = (uint32)(yo + (((int32)factor*(int32)(yin-yo))>>23));
        if (xout >= endx1 && xout < endx2 && yout >= endy1 && 
            yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

    /* values used in the calculations below */
        xa = ((int32)(xin - xo) << 3) + 3;
        ya = ((int32)(yin - yo) << 3) + 3;
        xan = ((int32)(xin - xo) << 3) - 3;
        yan = ((int32)(yin - yo) << 3) - 3;
        xb = (uint32)(xo+(((int32)factor*(int32)(xin-xo+1))>>23));
        yb = (uint32)(yo+(((int32)factor*(int32)(yin-yo+1))>>23));
        xbn = (uint32)(xo+(((int32)factor*(int32)(xin-xo-1))>>23));
        ybn = (uint32)(yo+(((int32)factor*(int32)(yin-yo-1))>>23));

        /* (2) xin + 0.4, yin (in fixed point arithmetic) */
        xout = (uint32)(xo + ((((int32)factor>>3)*xa)>>23));
        /* yout already calculated */
        /* Check xin + 1, yin */
        if (xout != xb && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (3) xin + 0.4, yin + 0.4 (in fixed point arithmetic) */
        /* xout already calculated */
        yout = (uint32)(yo + ((((int32)factor>>3)*ya)>>23));
        /* Check xin + 1, yin + 1 */
        if (xout != xb && yout != yb && xout >= endx1 && 
            xout < endx2 && yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (4) xin, yin + 0.4 (in fixed point arithmetic) */
        xout = (uint32)(xo + (((int32)factor*(int32)(xin-xo))>>23));
        /* yout already calculated */
        /* Check xin, yin + 1 */
        if (yout != yb && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (5) xin, yin - 0.4 (in fixed point arithmetic) */
        /* xout already calculated */
        yout = (uint32)(yo + ((((int32)factor>>3)*yan)>>23));
        /* Check xin, yin - 1 */
        if (yout != ybn && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (6) xin - 0.4, yin - 0.4 (in fixed point arithmetic) */
        xout = (uint32)(xo + ((((int32)factor>>3)*xan)>>23));
        /* yout already calculated */
        /* Check xin - 1, yin - 1 */
        if (xout != xbn && yout != ybn && xout >= endx1 && 
            xout < endx2 && yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

        /* (7) xin - 0.4, yin (in fixed point arithmetic) */
        /* xout already calculated */
        yout = (uint32)(yo + (((int32)factor*(int32)(yin-yo))>>23));
        /* Check xin - 1, yin */
        if (xout != xbn && xout >= endx1 && xout < endx2 && 
            yout >= endy1 && yout < endy2) 
        {
          /* within mapped region */
          xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                   (int32)(yout-yo)*(int32)(yout-yo));
          if (xytemp <= rSq) {
            /* within magnified region */
            mapIndex = (xout-endx1) + mapWidth*(yout-endy1);
            *((int16*)(xMap + mapIndex)) = (int16)xin;
            *((int16*)(yMap + mapIndex)) = (int16)yin;
          } /* end if (xout, yout) within magnified region */
        } /* end if (xout, yout) within mapped region */

      } /* end if (xin, yin) within magnified region */
    } /* end yin loop */
  } /* end xin loop */

  /* Create full YCbCr image */
  outIndex = 0;
  inc = outWidth * 3;
  full2ImgPtr = fullImgPtr + inc;
  in2ImgPtr = inImgPtr + outWidth;
  for (yout = 0; yout < outHeight; yout+=2) 
  {
    for (xout = 0; xout < outWidth; xout+=2) 
    {
      /* Process 4 pixels at a time */
      cb = *inClrPtr++;
      cr = *inClrPtr++;
      /* 1st pixel */
      *((unsigned char*)(fullImgPtr+outIndex)) = cb;
      *((unsigned char*)(fullImgPtr+outIndex+1)) = cr;
      *((unsigned char*)(fullImgPtr+outIndex+2)) = *inImgPtr++;
      /* 2nd pixel */
      *((unsigned char*)(fullImgPtr+outIndex+3)) = cb;
      *((unsigned char*)(fullImgPtr+outIndex+4)) = cr;
      *((unsigned char*)(fullImgPtr+outIndex+5)) = *inImgPtr++;
      /* 3rd pixel */
      *((unsigned char*)(full2ImgPtr+outIndex++)) = cb;
      *((unsigned char*)(full2ImgPtr+outIndex++)) = cr;
      *((unsigned char*)(full2ImgPtr+outIndex++)) = *in2ImgPtr++;
      /* 4th pixel */
      *((unsigned char*)(full2ImgPtr+outIndex++)) = cb;
      *((unsigned char*)(full2ImgPtr+outIndex++)) = cr;
      *((unsigned char*)(full2ImgPtr+outIndex++)) = *in2ImgPtr++;
    } /* end xout loop */
    inImgPtr += outWidth;
    in2ImgPtr += outWidth;
    outIndex += inc;
  } /* end yout loop */

  MSG_LOW("ipl_pinch_420lp marker_2\n");

  /*
  ** Fill in output pixels in the following order:
  ** |---------------------------|
  ** |            (1)            |
  ** |---------------------------|
  ** | (2a)    |  (2b)  |   (2c) |
  ** |---------------------------|
  ** |            (3)            |
  ** |---------------------------|
  */
  /* (1) non-magnified region ABOVE magnified region */
  outIndex = 0;
  for (yout = 0; yout < endy1; yout++) 
  {
    for (xout = 0; xout < outWidth; xout++) 
    {
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
    }
  } /* end (1) */

  /* (2) region around magnified region */
  mapIndex = 0;
  for (yout = endy1; yout < endy2; yout++) {
    /* (2a) non-magnified region to the left of the magnified area */
    for (xout = 0; xout < endx1; xout++) {
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
    }
    /* (2b) region affected by magnification */
    for (xout = endx1; xout < endx2; xout++) {
      if (*((int16*)(xMap + mapIndex)) == -1) {
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 128;
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 128;
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 16;
      } else {
        inIndex = (*((int16*)(xMap+mapIndex)) + 
               *((int16*)(yMap+mapIndex)) * outWidth)*3;
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 
          *((unsigned char*)(fullImgPtr + inIndex++));
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 
          *((unsigned char*)(fullImgPtr + inIndex++));
        *((unsigned char*)(outFullImgPtr + outIndex++)) = 
          *((unsigned char*)(fullImgPtr + inIndex));
      }
      mapIndex++;
    }
    /* (2c) non-magnified region to the right of the magnified area */
    for (xout = endx2; xout < outWidth; xout++) {
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
    }
  } /* end (2) */

  /* (3) non-magnified region BELOW magnified area */
  for (yout = endy2; yout < outHeight; yout++) {
    for (xout = 0; xout < outWidth; xout++) {
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
      *((unsigned char*)(outFullImgPtr + outIndex)) = 
        *((unsigned char*)(fullImgPtr + outIndex));
      outIndex++;
    }
  } /* end (3) */

  /*
  ** Malloc 3 buffers:
  ** blankPixels stores interpolated pixel values for 
  **     pixels left blank in the previous step
  ** nbXMap, nbYMap stores the order in which  
  **     neighboring pixels are used during the 
  **     interpolation process
  */
  blankPixels = ipl_malloc(mapSize*3*sizeof(unsigned char));
  if (!blankPixels) 
  {
    ipl_sys_free(yMap);
    ipl_sys_free(xMap);
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    MSG_LOW("ipl_pinch_420lp marker_205\n");
    return IPL_NO_MEMORY;
  }

  nbXMap = ipl_malloc(12*sizeof(int8));
  if (!nbXMap) 
  {   
    ipl_sys_free(yMap);
    ipl_sys_free(xMap);
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    ipl_sys_free(blankPixels);
    MSG_LOW("ipl_pinch_420lp marker_206\n");
    return IPL_NO_MEMORY;
  }
  nbYMap = ipl_malloc(12*sizeof(int8));
  if (!nbYMap) 
  {  
    ipl_sys_free(yMap);
    ipl_sys_free(xMap);
    ipl_sys_free(fullImgPtr);
    ipl_sys_free(outFullImgPtr);
    ipl_sys_free(nbXMap);
    ipl_sys_free(blankPixels);
    MSG_LOW("ipl_pinch_420lp marker_207\n");
    return IPL_NO_MEMORY;
  }

  /* Initialize nbXMap, nbYMap */
  *nbXMap = -1;
  *((int8*)(nbXMap+1)) = 0;
  *((int8*)(nbXMap+2)) = 1;
  *((int8*)(nbXMap+3)) = -1;
  *((int8*)(nbXMap+4)) = 1;
  *((int8*)(nbXMap+5)) = -1;
  *((int8*)(nbXMap+6)) = 0;
  *((int8*)(nbXMap+7)) = -1;
  *((int8*)(nbXMap+8)) = 0;
  *((int8*)(nbXMap+9)) = -2;
  *((int8*)(nbXMap+10)) = 0;
  *((int8*)(nbXMap+11)) = 2;
  *nbYMap = -1;
  *((int8*)(nbYMap+1)) = -1;
  *((int8*)(nbYMap+2)) = -1;
  *((int8*)(nbYMap+3)) = 0;
  *((int8*)(nbYMap+4)) = 0;
  *((int8*)(nbYMap+5)) = 1;
  *((int8*)(nbYMap+6)) = 1;
  *((int8*)(nbYMap+7)) = 1;
  *((int8*)(nbYMap+8)) = -2;
  *((int8*)(nbYMap+9)) = 0;
  *((int8*)(nbYMap+10)) = 2;
  *((int8*)(nbYMap+11)) = 0;
    
  /* Compute missing values for area affected by magnification */
  mapIndex = 0;
  outIndex = 0;
  for (yout = endy1; yout < endy2; yout++) {
    for (xout = endx1; xout < endx2; xout++) {
      if (*((int16*)(xMap + mapIndex)) == -1) {
        /* blank pixel */
        xytemp = (uint32)((int32)(xout-xo)*(int32)(xout-xo) +
                 (int32)(yout-yo)*(int32)(yout-yo));
        if (xytemp > rSq) {
          /* outside magnified region */
          inIndex = (xout + yout * outWidth) * 3;
          *((unsigned char*)(blankPixels + outIndex++)) = 
            *((unsigned char*)(fullImgPtr + inIndex++));
          *((unsigned char*)(blankPixels + outIndex++)) = 
            *((unsigned char*)(fullImgPtr + inIndex++));
          *((unsigned char*)(blankPixels + outIndex++)) = 
            *((unsigned char*)(fullImgPtr + inIndex));
        } else {
          /* inside magnified region */
          nbIndex = 0;
          lumasum = 0;
          crsum = 0;
          cbsum = 0;
          cnt = 0;
          while (nbIndex < 12) {
            /* neighboring pixel */
            xin = (uint32)((int32)xout + 
                  (int32)*((int8*)(nbXMap+nbIndex)));
            yin = (uint32)((int32)yout + 
                  (int32)*((int8*)(nbYMap+nbIndex)));
            if (xin < outWidth && yin < outHeight) {
              /* within image boundaries */
              inIndex = (xin + yin * outWidth) * 3;
              cb = *((unsigned char*)(outFullImgPtr+inIndex++));
              cr = *((unsigned char*)(outFullImgPtr+inIndex++));
              luma = *((unsigned char*)(outFullImgPtr+inIndex));
              if (cb != 128 || cr != 128 || luma != 16) {
                cbsum += cb;
                crsum += cr;
                lumasum += luma;
                cnt++;
              }
            }
            nbIndex++;
          } /* end while loop */
          /* get average */
          if (cnt > 0) {
            cbsum /= cnt;
            crsum /= cnt;
            lumasum /= cnt;
          } else {
            cbsum = 128;
            crsum = 128;
            lumasum = 16;
          }
          /* set output pixel value */
          *((unsigned char*)(blankPixels+outIndex++)) = 
            (unsigned char)cbsum;
          *((unsigned char*)(blankPixels+outIndex++)) = 
            (unsigned char)crsum;
          *((unsigned char*)(blankPixels+outIndex++)) = 
            (unsigned char)lumasum;
        } /* end if blank pixel outside magnified region */
      } /* end if blank pixel */
      else {
        /* non-blank pixel */
        inIndex = (xout + yout * outWidth) * 3;
        *((unsigned char*)(blankPixels + outIndex++)) = 
          *((unsigned char*)(outFullImgPtr + inIndex++));
        *((unsigned char*)(blankPixels + outIndex++)) = 
          *((unsigned char*)(outFullImgPtr + inIndex++));
        *((unsigned char*)(blankPixels + outIndex++)) = 
          *((unsigned char*)(outFullImgPtr + inIndex));
      }
      /* increment the index */
      mapIndex++;
    } /* end xout loop */
  } /* end yout loop */

  /*
  ** Fill in missing values in full YCbCr output for area 
  ** affected by magnification
  */
  inc = (outWidth - mapWidth)*3;
  outIndex = (endx1 + endy1 * outWidth)*3;
  inIndex = 0;
  for (yout = endy1; yout < endy2; yout++) {
    for (xout = endx1; xout < endx2; xout++) {
      *((unsigned char*)(outFullImgPtr + outIndex++)) = 
        *((unsigned char*)(blankPixels + inIndex++));
      *((unsigned char*)(outFullImgPtr + outIndex++)) = 
        *((unsigned char*)(blankPixels + inIndex++));
      *((unsigned char*)(outFullImgPtr + outIndex++)) = 
        *((unsigned char*)(blankPixels + inIndex++));
    }
    outIndex += inc;
  }

  /* Get YCbCr 4:2:0 line packed output */
  inIndex = 0;
  inc = outWidth * 3;
  out2ImgPtr = outImgPtr + outWidth;
  outFull2ImgPtr = outFullImgPtr + inc;
  for (yout = 0; yout < outHeight; yout+=2) 
  {
    for (xout = 0; xout < outWidth; xout+=2) 
    {
      /* Process 4 pixels at a time */
      /* 1st pixel */
      *outImgPtr++ = *((unsigned char*)(outFullImgPtr+inIndex+2));
      /* 2nd pixel */
      *outImgPtr++ = *((unsigned char*)(outFullImgPtr+inIndex+5));
      /* 3rd pixel */
      inIndex += 2;
      *out2ImgPtr++ = *((unsigned char*)(outFull2ImgPtr+inIndex++));
      /* CbCr */
      *outClrPtr++ = *((unsigned char*)(outFull2ImgPtr+inIndex++));
      *outClrPtr++ = *((unsigned char*)(outFull2ImgPtr+inIndex++));
      /* 4th pixel */
      *out2ImgPtr++ = *((unsigned char*)(outFull2ImgPtr+inIndex++));
    } /* end xout loop */
    outImgPtr += outWidth;
    out2ImgPtr += outWidth;
    inIndex += inc;
  } /* end yout loop */

  /* Free memory */
  ipl_sys_free(xMap);
  ipl_sys_free(yMap);
  ipl_sys_free(blankPixels);
  ipl_sys_free(nbXMap);
  ipl_sys_free(nbYMap);
  ipl_sys_free(fullImgPtr);
  ipl_sys_free(outFullImgPtr);

  MSG_LOW("ipl_pinch_420lp marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_pinch_420lp */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_pinch

DESCRIPTION
  This function pinches the input image in a circular region of 
  arbitrary location and radius.  Pixels in this region are pinched
  towards the center. The center of the pinched region must be 
  within input image boundaries. Areas outside the pinched region are 
  unchanged. If part of the pinched region extends beyond the image 
  boundaries, only the part within the boundaries is displayed. 

  Input and output image sizes must be equal.
  
  Input and output images must have the same color format, which 
  can be RGB565, YCbCr 4:2:2, YCbCr 4:2:0 line packed, or YCrCb 4:2:0 
  line packed format.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    points to the input image
  out_img_ptr   points to the output image
  circle        circular region of pinching

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_pinch
(
  ipl_image_type* in_img_ptr,  /* Points to the input image        */
  ipl_image_type* out_img_ptr, /* Points to the output image       */
  ipl_circle_type* circle      /* Circular region of pinching      */
)
{
  ipl_status_type retval;

  MSG_LOW("ipl_pinch marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr ||
      !out_img_ptr || !out_img_ptr->imgPtr ||
      !circle)
  {
    MSG_LOW("ipl_pinch marker_200\n");
    return IPL_FAILURE;
  }

  if (in_img_ptr->cFormat != out_img_ptr->cFormat)
  {
    MSG_LOW("ipl_pinch marker_201\n");
    /* input and output color formats must be the same */
    return IPL_FAILURE;
  }

  if (in_img_ptr->dx != out_img_ptr->dx || 
      in_img_ptr->dy != out_img_ptr->dy) 
  {
    MSG_LOW("ipl_pinch marker_202\n");
    /* input and output image sizes must be the same */
    return IPL_FAILURE;
  }

  if (circle->r == 0 || circle->x >= out_img_ptr->dx || 
      circle->y >= out_img_ptr->dy) 
  {
    MSG_LOW("ipl_pinch marker_203\n");
    /* 
     ** radius must be positive; x- and y-coordinates must be 
     ** within image boundaries 
    */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_pinch marker_1\n");

  /* Call the appropriate function */
  if (in_img_ptr->cFormat == IPL_RGB565) 
  {
    retval = ipl_pinch_rgb565(in_img_ptr, out_img_ptr, circle);
    if (retval != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_pinch marker_204\n");
      return IPL_FAILURE;
    }
  } 
  else if (in_img_ptr->cFormat == IPL_YCbCr) 
  {
    retval = ipl_pinch_ycbcr(in_img_ptr, out_img_ptr, circle);
    if (retval != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_pinch marker_205\n");
      return IPL_FAILURE;
    }
  } 
  else if (in_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
  {
    retval = ipl_pinch_420lp(in_img_ptr, out_img_ptr, circle);
    if (retval != IPL_SUCCESS) 
    {
      MSG_LOW("ipl_pinch marker_206\n");
      return IPL_FAILURE;
    }
  } 
  else 
  {
    MSG_LOW("ipl_pinch marker_207\n");
    /* other formats not supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_pinch marker_100\n");
  return IPL_SUCCESS;
} /* End ipl_pinch */


#endif


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_pinch

DESCRIPTION
  This function pinches the input image in a circular region of 
  arbitrary location and radius.  Pixels in this region are pinched
  towards the center. The center of the pinched region must be 
  within input image boundaries. Areas outside the pinched region are 
  unchanged. If part of the pinched region extends beyond the image 
  boundaries, only the part within the boundaries is displayed. 

  Input and output image sizes must be equal.
  
  Input and output images must have the same color format, which 
  can be RGB565, YCbCr 4:2:2, YCbCr 4:2:0 line packed, or YCrCb 4:2:0 
  line packed format.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    points to the input image
  out_img_ptr   points to the output image
  circle        circular region of pinching

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_pinch
(
  ipl_image_type* in_img_ptr,  /* Points to the input image        */
  ipl_image_type* out_img_ptr, /* Points to the output image       */
  ipl_circle_type* circle      /* Circular region of pinching      */
)
{
  return IPL_FAILURE;
}



#define PHASE_MASK              0x1F
#define VER_OFFSET              4096
#define REM_OFFSET              256
#define HOR_RIGHT_SHIFT         5
#define VER_RIGHT_SHIFT         13
#define IN_STEP_LEFT_SHIFT_BITS 16
#define PHASE_LEFT_SHIFT_BITS   11
#define PHASE0_HOR_LEFT_SHIFT   4
#define PHASE0_VER_OFFSET       8


static const int16 polyphase_fir_default[32][4] =
{
 {0, 512, 0, 0},
 {-7, 510, 8, 0},
 {-14, 507, 19, 0},
 {-19, 501, 32, -2},
 {-24, 493, 46, -3},
 {-28, 483, 62, -5},
 {-31, 472, 78, -7},
 {-34, 458, 96, -9},
 {-36, 444, 116, -12},
 {-37, 427, 135, -14},
 {-37, 410, 156, -17},
 {-37, 391, 177, -19},
 {-37, 372, 199, -22},
 {-36, 352, 221, -25},
 {-35, 331, 243, -27},
 {-33, 309, 265, -29},
 {-32, 288, 288, -32},
 {-29, 265, 309, -33},
 {-27, 243, 331, -35},
 {-25, 221, 352, -36},
 {-22, 199, 372, -37},
 {-19, 177, 391, -37},
 {-17, 156, 410, -37},
 {-14, 135, 427, -37},
 {-12, 116, 444, -36},
 {-9, 96, 458, -34},
 {-7, 78, 472, -31},
 {-5, 62, 483, -28},
 {-3, 46, 493, -24},
 {-2, 32, 501, -19},
 {0, 19, 507, -14},
 {0, 8, 510, -7}
};


static int16 polyphase_fir[32][4];




/* <EJECT> */
/*===========================================================================

FUNCTION scaling_bicubic

DESCRIPTION

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    points to the input image
  out_img_ptr   points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type 
scaling_bicubic(ipl_image_type * i_img_ptr, ipl_image_type * o_img_ptr, ipl_channel_type chan, int sharpen)
{
  uint32 i,j;
  uint32 in_idx;
  uint32 in_idx_int;

  uint8 *in_ptr;
  uint8 *out_ptr;
  int32 *tmp_ptr;

  uint8 *oin_ptr;
  int32 *otmp_ptr;

  uint16 phase;
  int16 *coeff;
  int32 int32_temp;
  int hor_step, ver_step;
  int idx, idy, odx, ody;
  int mdx, mdy;

  if (i_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK ||
      i_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK)
  {
    if (chan == IPL_CHANNEL_LUMA)
    {
      idx = i_img_ptr->dx;
      idy = i_img_ptr->dy;
      odx = o_img_ptr->dx;
      ody = o_img_ptr->dy;
      in_ptr = i_img_ptr->imgPtr;
      out_ptr = o_img_ptr->imgPtr;
    }
    else if (chan ==IPL_CHANNEL_Cb)
    {
      idx = i_img_ptr->dx/2;
      idy = i_img_ptr->dy/2;
      odx = o_img_ptr->dx/2;
      ody = o_img_ptr->dy/2;
      in_ptr = i_img_ptr->clrPtr;
      out_ptr = o_img_ptr->clrPtr;
    }
    else if (chan ==IPL_CHANNEL_Cr)
    {
      idx = i_img_ptr->dx/2;
      idy = i_img_ptr->dy/2;
      odx = o_img_ptr->dx/2;
      ody = o_img_ptr->dy/2;
      in_ptr = (i_img_ptr->clrPtr + idx*idy);
      out_ptr = (o_img_ptr->clrPtr + odx*ody);
    }
    else 
    {
      return IPL_FAILURE;
    }
  }
  else 
  {
    return IPL_FAILURE;
  }


  hor_step = (idx << IN_STEP_LEFT_SHIFT_BITS) / odx;
  ver_step = (idy << IN_STEP_LEFT_SHIFT_BITS) / ody;

  mdx = IPL_MAX(idx,odx);
  mdy = IPL_MAX(idy,ody);
  tmp_ptr = (int32 *) sys_malloc(mdx*mdy*sizeof(int32));
  if (tmp_ptr == NULL)
    return IPL_FAILURE;

  oin_ptr = in_ptr;
  otmp_ptr = tmp_ptr; 

  if ((idx == odx) && (sharpen == 0))
  {
    for (i = 0; i < (uint32)(idx * idy); i++)
    {
      *tmp_ptr++ = (*in_ptr++) << PHASE0_HOR_LEFT_SHIFT;
    }
  }
  else
  {
    in_idx= REM_OFFSET;

    for (i = 0; i < (uint32)odx; i++)
    {
      in_idx_int = in_idx >> IN_STEP_LEFT_SHIFT_BITS;
      phase = (uint16)((in_idx>>PHASE_LEFT_SHIFT_BITS) & PHASE_MASK);
    
      if (((sharpen == 0) && (phase==0)) ||
           (in_idx_int == 0)||(in_idx_int > (uint32_t)(idx-3)))
      {
        in_ptr = oin_ptr + in_idx_int;
        tmp_ptr = otmp_ptr + i;

        for (j = 0; j < (uint32)idy; j++)
        {
          *tmp_ptr = (*in_ptr) << PHASE0_HOR_LEFT_SHIFT;
          in_ptr  += idx;
          tmp_ptr += odx;
        }
      }
      else
      {
        coeff = &polyphase_fir[phase][0];

        in_ptr = oin_ptr + in_idx_int-1;
        tmp_ptr = otmp_ptr + i;

        for (j = 0; j < (uint32)idy; j++)
        {
          *tmp_ptr = (in_ptr[0]*coeff[0]+in_ptr[1]*coeff[1]+in_ptr[2]*coeff[2]+in_ptr[3]*coeff[3]) >> HOR_RIGHT_SHIFT;
          in_ptr  += idx;
          tmp_ptr += odx;
        }

      }
      in_idx += hor_step;
    }
  }


  if ((idy == ody) && (sharpen == 0))
  {
    tmp_ptr = otmp_ptr;

    for (i = 0; i < (uint32)(odx*ody); i++)
    {
      int32_temp = ((*tmp_ptr++)+PHASE0_VER_OFFSET)>>PHASE0_HOR_LEFT_SHIFT;

      if (int32_temp > 255)
        *out_ptr++ = 255;
      else if (int32_temp < 0)
        *out_ptr++ = 0;
      else
        *out_ptr++= (uint8) int32_temp;
    }
  }
  else
  {
    in_idx = REM_OFFSET;

    for (i = 0; i < (uint32_t)ody; i++)
    {
      in_idx_int = in_idx >> IN_STEP_LEFT_SHIFT_BITS;
      phase = (uint16)((in_idx >> PHASE_LEFT_SHIFT_BITS) & PHASE_MASK);

      if (((sharpen == 0) && (phase==0)) ||
          (in_idx_int==0)||(in_idx_int>(uint32_t)(idy-3)))
      {
        tmp_ptr= otmp_ptr + in_idx_int * odx;

        for (j = 0; j < (uint32)odx; j++)
        {
          int32_temp= ((*tmp_ptr++)+PHASE0_VER_OFFSET)>>PHASE0_HOR_LEFT_SHIFT;

          if (int32_temp>255)
            *out_ptr++= 255;
          else if (int32_temp<0)
            *out_ptr++= 0;
          else
            *out_ptr++= (uint8)int32_temp;
        }
      }
      else
      {
        coeff = &polyphase_fir[phase][0];

        tmp_ptr = otmp_ptr + (in_idx_int-1)*odx;

        for (j = 0; j < (uint32)odx; j++)
        {
          int32_temp= (tmp_ptr[0]*coeff[0]+tmp_ptr[odx]*coeff[1]+ tmp_ptr[odx<<1]*coeff[2]+tmp_ptr[(odx<<1)+odx]*coeff[3]+VER_OFFSET)>>VER_RIGHT_SHIFT;

          if (int32_temp>255)
            *out_ptr++= 255;
          else if (int32_temp<0)
            *out_ptr++= 0;
          else
            *out_ptr++= (uint8)int32_temp;
          tmp_ptr++;
        }
      }

      in_idx += ver_step;
    }   
  }

  sys_free(otmp_ptr);
  
  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION scaling_repeat

DESCRIPTION

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    points to the input image
  out_img_ptr   points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type scaling_repeat(ipl_image_type * i_img_ptr, ipl_image_type * o_img_ptr, ipl_channel_type chan, int sharpen)
{
  uint32 i,j;
  uint32 in_idx;

  uint8 *in_ptr;
  uint8 *out_ptr;
  uint8 *tmp_ptr;

  uint8 *otmp_ptr;

  int idx, idy, odx, ody;
  int mdx, mdy; 
  int hor_step, ver_step;

  if (i_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK ||
      i_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK)
  {
    if (chan == IPL_CHANNEL_LUMA)
    {
      idx = i_img_ptr->dx;
      idy = i_img_ptr->dy;
      odx = o_img_ptr->dx;
      ody = o_img_ptr->dy;
      in_ptr = i_img_ptr->imgPtr;
      out_ptr = o_img_ptr->imgPtr;
    }
    else if (chan == IPL_CHANNEL_Cb)
    {
      idx = i_img_ptr->dx/2;
      idy = i_img_ptr->dy/2;
      odx = o_img_ptr->dx/2;
      ody = o_img_ptr->dy/2;
      in_ptr = i_img_ptr->clrPtr;
      out_ptr = o_img_ptr->clrPtr;
    }
    else if (chan == IPL_CHANNEL_Cr)
    {
      idx = i_img_ptr->dx/2;
      idy = i_img_ptr->dy/2;
      odx = o_img_ptr->dx/2;
      ody = o_img_ptr->dy/2;
      in_ptr = (i_img_ptr->clrPtr + idx*idy);
      out_ptr = (o_img_ptr->clrPtr + odx*ody);
    }
    else 
    {
      return IPL_FAILURE;
    }
  }
  else 
  {
    return IPL_FAILURE;
  }


  hor_step = (idx << IN_STEP_LEFT_SHIFT_BITS) / odx;
  ver_step = (idy << IN_STEP_LEFT_SHIFT_BITS) / ody;


  mdx = IPL_MAX(idx,odx);
  mdy = IPL_MAX(idy,ody);
  tmp_ptr = (uint8 *) sys_malloc(mdx*mdy*sizeof(uint8));
  if (tmp_ptr == NULL)
    return IPL_FAILURE;
  otmp_ptr = tmp_ptr;

  if ((idx == odx) && (sharpen == 0))
  {
    memcpy(tmp_ptr, in_ptr, idx*idy);
  }
  else
  {
    for (i = 0; i < (uint32)idy; i++)
    {
      in_idx = REM_OFFSET;

      for (j = 0; j < (uint32)odx; j++)
      {
        *tmp_ptr++ = *(in_ptr + (in_idx >> IN_STEP_LEFT_SHIFT_BITS));
        in_idx += hor_step;
      }

      in_ptr += idx;
    }
  }

  if ((idy == ody) && (sharpen == 0))
  {
    tmp_ptr = otmp_ptr;
    memcpy(out_ptr, tmp_ptr, odx*ody);
  }
  else
  {
    in_idx= REM_OFFSET;
    tmp_ptr = otmp_ptr;

    for (i = 0; i < (uint32)ody; i++)
    {
      in_ptr = tmp_ptr + (in_idx >> IN_STEP_LEFT_SHIFT_BITS)*odx;
      memcpy(out_ptr, in_ptr, odx);

      out_ptr += odx;
      in_idx += ver_step;
    }
  }

  sys_free(otmp_ptr);

  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_resize_filter

DESCRIPTION

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    points to the input image
  out_img_ptr   points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_resizeFilter
(
  ipl_image_type* i_img_ptr,  /* Points to the input image        */
  ipl_image_type* o_img_ptr, /* Points to the output image       */
  ipl_quality_type y_scaling_method,
  ipl_quality_type uv_scaling_method,
  int sharpen,
  int numPhases
)
{
  ipl_status_type retval;
  int i;

  MSG_LOW("ipl_resize_filter marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_resize_filter marker_200\n");
    return IPL_FAILURE;
  }

  if (i_img_ptr->cFormat != o_img_ptr->cFormat)
  {
    MSG_LOW("ipl_resize_filter marker_201\n");
    /* input and output color formats must be the same */
    return IPL_FAILURE;
  }
  
  if (numPhases > 32)
  {
    MSG_LOW("ipl_resize_filter marker_202\n");
    return IPL_FAILURE;
  }


  // setup filter
  if (sharpen == 0)
  {
    memcpy(&polyphase_fir[0][0], &polyphase_fir_default[0][0], 32*4*sizeof(int16));
  }
  else
  {
    for (i = 0; i < numPhases; i++)
    {
      polyphase_fir[i][0] = (int16)(((sharpen+256)*polyphase_fir_default[i][0]-sharpen*polyphase_fir_default[i][1]+128)>>8);

      polyphase_fir[i][1] = (int16)((-1*sharpen*polyphase_fir_default[i][0]+(2*sharpen+256)*polyphase_fir_default[i][1]-sharpen*polyphase_fir_default[i][2]+128)>>8);

      polyphase_fir[i][2] = (int16)((-1*sharpen*polyphase_fir_default[i][1]+(2*sharpen+256)*polyphase_fir_default[i][2]-sharpen*polyphase_fir_default[i][3]+128)>>8);

      polyphase_fir[i][3] = (int16)((-1*sharpen*polyphase_fir_default[i][2]+(sharpen+256)*polyphase_fir_default[i][3]+128)>>8);
    }
  }

  //for (i = 0; i < numPhases; i++)
  //{
  //  printf("%d %d %d %d\n", polyphase_fir[i][0],polyphase_fir[i][1], 
  //      polyphase_fir[i][2], polyphase_fir[i][3]);
  //}

  /* Call the appropriate function */
  if (i_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK || 
      i_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK) 
  {
    if (y_scaling_method == IPL_QUALITY_OFF)
    {
      retval = IPL_SUCCESS;
    }
    else if (y_scaling_method == IPL_QUALITY_LOW)
    {
      retval = scaling_repeat(i_img_ptr, o_img_ptr, IPL_CHANNEL_LUMA, sharpen);
    }
    else if (y_scaling_method == IPL_QUALITY_MAX)
    {
      retval = scaling_bicubic(i_img_ptr, o_img_ptr, IPL_CHANNEL_LUMA, sharpen);
    }
    else
    {
      retval = scaling_bicubic(i_img_ptr, o_img_ptr, IPL_CHANNEL_LUMA, sharpen);
    }

    if (retval == IPL_SUCCESS)
    {
      if (uv_scaling_method == IPL_QUALITY_OFF)
      {
        retval = IPL_SUCCESS;
      }
      else if (uv_scaling_method == IPL_QUALITY_LOW)
      {
        retval = scaling_repeat(i_img_ptr, o_img_ptr, IPL_CHANNEL_Cb, sharpen);
        retval = scaling_repeat(i_img_ptr, o_img_ptr, IPL_CHANNEL_Cr, sharpen);
      }
      else if (uv_scaling_method == IPL_QUALITY_MAX)
      {
        retval = scaling_bicubic(i_img_ptr, o_img_ptr, IPL_CHANNEL_Cb, sharpen);
        retval = scaling_bicubic(i_img_ptr, o_img_ptr, IPL_CHANNEL_Cr, sharpen);
      }
      else
      {
        retval = scaling_bicubic(i_img_ptr, o_img_ptr, IPL_CHANNEL_Cb, sharpen);
        retval = scaling_bicubic(i_img_ptr, o_img_ptr, IPL_CHANNEL_Cr, sharpen);
      }
    }
  }
  else
  {
    MSG_LOW("ipl_pinch marker_203\n");
    retval = IPL_FAILURE;
  } 

  MSG_LOW("ipl_pinch marker_100\n");
  return retval;
}









#endif







