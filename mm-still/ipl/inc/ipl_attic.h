#ifndef IPL_ATTIC_H
#define IPL_ATTIC_H


/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_attic.h#1 $
$DateTime: 2009/02/02 00:28:06 $ $Author: ninadp $
===========================================================================*/

/*===========================================================================
                      FUNCTION PROTOTYPES
===========================================================================*/


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_downsize_fast

DESCRIPTION
  This function performs resizing and color conversion. The bip_ptr can be 
  set to NULL if no color processing is desired. Demosaicing is still 
  performed when the bip_ptr is set to NULL.

  This function is optimized for fast execution speed.

  The input must be YCbCr 4:2:2, RGB565, Bayer, or RGB888.
  The output must be YCbCr 4:2:2 or RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr    pointer to the input image
  output_img_ptr   pointer to the output image
  bip_ptr          pointer to a BIP structure 
                   (for Mega Pixel Color Processing Support)

RETURN VALUE
  IPL_SUCCESS      indicates operation was successful
  IPL_FAILURE      otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_downsize_fast
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr           /* Points to the BIP structure         */
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_merge

DESCRIPTION
  This function adds 2 images, clips result, and performs color conversion 
  if needed.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr    pointer to the input image
  input_img2_ptr   pointer to the input image
  output_img_ptr   pointer to the output image

RETURN VALUE
  IPL_SUCCESS      indicates operation was successful
  IPL_FAILURE      otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_image_merge
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* i_img2_ptr,        /* Points to the frame image      */
  ipl_image_type* o_img_ptr          /* Points to the output image     */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_bad_pixel_correct_old

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
  min_factor is the multiplicant of the min pixel value Q12 Unsigned
  max_factor is the multiplicant of the max pixel value Q12 Unsigned

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_bad_pixel_correct_old
(
  ipl_image_type* img_ptr,          /* Points to the input image           */
  uint16 min_factor,                /* min factor threshold                */
  uint16 max_factor                 /* max factor threshold                */
);


#if 0
/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycbcr420_to_rgb565

DESCRIPTION
  This functin will accept YCrCb 4:2:0 as input and output RGB565
  Input should be 4:2:0 line packed Format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  crop:           Crop in Output

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
extern ipl_status_type ipl_crop_ycbcr420_to_rgb565
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* crop
);
#endif


/* <EJECT> */
/*===========================================================================

FUNCTION ycbcr_to_hsv

DESCRIPTION
  This function converts from YCbCr 4:2:2 to HSV.

DEPENDENCIES
  None

ARGUMENTS IN/OUT
  input_img_ptr     pointer to the input image
  hsv_buffer        output buffer where the HSV data is written

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ycbcr_to_hsv
(
  ipl_image_type* input_img_ptr,     /* Points to the input image   */
  int16* hsv_buffer                  /* Output HSV buffer           */
);


#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION hsv_to_ycbcr

DESCRIPTION
  This function converts from HSV to YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN/OUT
  output_img_ptr   pointer to output image
  hsv_buffer       input buffer containing the HSV data

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type hsv_to_ycbcr
(
  ipl_image_type* output_img_ptr,    /* Points to the output image   */
  int16* hsv_buffer                  /* Input HSV buffer             */
);

#endif



#if 0
/* <EJECT> */
/*===========================================================================
 *
 * FUNCTION ipl_whiteBoard
 *
 * DESCRIPTION
 *   This function takes an image taken of a white board, or chalk board, and
 *   cleans it up for easy viewing and printing.
 *
 *
 * DEPENDENCIES
 *  None
 *
 * ARGUMENTS IN
 *  i_img_ptr      pointer to the input image
 *  o_img_ptr      pointer to the output image
 *  blackmode      0 = auto, 1 = do whiteboard, 2 = do blackboard*
 *  th_noise       noise threshold, 10 (out of 255) is good number
 *
 * RETURN VALUE
 *  IPL_SUCCESS    indicates operation was successful
 *  IPL_FAILURE    otherwise
 *
 * SIDE EFFECTS
 *  None
 *
 *===========================================================================*/
extern ipl_status_type ipl_whiteboard
(
  ipl_image_type * in_img_ptr,
  ipl_image_type *out_img_ptr,
  int blackMode, 
  int th_noise
);

#endif



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_YCbCr2RGB

DESCRIPTION
  This function performs color conversion from YCbCr 4:2:2 to RGB565.
  It considers 2 pixels Y1Cb and Y2Cr and calculates Rc Gc Bc
  as follows:
  [Rc Gc Bc] = convert(3x2) * [Cb-128 Cr-128]
  R G B = [Y Y Y] + [Rc Gc Bc]
  and then the R G B are packed in 5 6 5 format.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in      pointer to the input image data
  width        width of the input image
  height       height of the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out     pointer to the output image data

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_YCbCr2RGB
(
  unsigned char* data_in,           /* Points to the input image data    */
  unsigned short* data_out,         /* Points to the output image data   */
  short width,                      /* Width of the input image          */
  short height                      /* Height of the input image         */
);


#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_YCbCr2RGB_pitch

DESCRIPTION
  This function does color conversion from YCbCr to RGB 565.
  It considers 2 pixels Y1Cb and Y2Cr and calculates Rc Gc Bc
  as follows:
  [Rc Gc Bc] = convert(3x2) * [Cb-128 Cr-128]
  R G B = [Y Y Y] + [Rc Gc Bc]
  and then the R G B are packed in 5 6 5 format.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in is the input data which needs to be color converted
  width is the width of input frame.
  height is the height of the input frame
  pitchx is the number of pixels to skip in x when converting 

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out is where the output data is stored

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_YCbCr2RGB_pitch(
        unsigned char* data_in,
        unsigned short* data_out, 
        short width, 
        short height,
        short pitchx
        );

#endif


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB2YCbCr

DESCRIPTION
  This function performs color conversion from RGB565 to YCbCr 4:2:2.
  It considers 2 pixels at a time, unpacks the rgb values, and outputs
  Y1Cb and Y2Cr.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in    pointer to the input image data
  width      width of the input image
  height     height of the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out   pointer to the output image data
RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_RGB2YCbCr
(
  unsigned char* data_in,         /* Points to the input image data    */
  unsigned char* data_out,        /* Points to the output image data   */
  short width,                    /* Width of the input image          */
  short height                    /* Height of the input image         */
);



// these are old #define we should carry forward for backward compatibility
#define ipl_conv_YCbCr422ToYCbCr444(a,b) ipl_convert_ycbcr422_to_ycbcr444(a,b)
#define ipl_conv_YCbCr444ToYCbCr422(a,b) ipl_convert_ycbcr444_to_ycbcr422(a,b)
#define ipl_conv_RGB565ToYCbCr444(a,b) ipl_convert_rgb565_to_ycbcr444(a,b)
#define ipl_conv_FrameRGB565ToYCbCr444(a,b,c,d,e)  ipl_convert_frame_rgb565_to_ycbcr444(a,b,c,d,e)
#define ipl_conv_YCrCb420lpToYCbCr422(a,b) ipl_convert_ycrcb420lp_to_ycbcr422(a,b)
#define ipl_conv_YCrCb422lpToYCbCr422(a,b) ipl_convert_ycrcb422lp_to_ycbcr422(a,b)
#define ipl_conv_RGB565ToYCrCb420lp(a,b) ipl_convert_rgb565_to_ycrcb420lp(a,b)
#define ipl_conv_RGB565ToYCrCb422lp(a,b) ipl_convert_rgb565_to_ycrcb422lp(a,b)
#define ipl2_conv_YCrCb420lpToRGB565(a,b) ipl2_convert_ycrcb420lp_to_rgb565(a,b)
#define ipl2_conv_YCrCb422lpToRGB565(a,b) ipl2_convert_ycrcb422lp_to_rgb565(a,b)
#define ipl_conv_YCrCb420lpToRGB565(a,b) ipl_convert_ycrcb420lp_to_rgb565(a,b)
#define ipl_conv_YCrCb422lpToRGB565(a,b) ipl_convert_ycrcb422lp_to_rgb565(a,b)
#define ipl_conv_YCbCr422ToYCrCb420lp(a,b) ipl_convert_ycbcr422_to_ycrcb420lp(a,b)
#define ipl_conv_RGB444ToYCbCr420lp(a,b)  ipl_convert_rgb444_to_ycbcr420lp(a,b)
#define ipl_conv_YCbCr420lpToRGB444(a,b)  ipl_convert_ycbcr420lp_to_rgb444(a,b)
#define ipl_conv_RGB666ToYCbCr420lp(a,b)  ipl_convert_rgb666_to_ycbcr420lp(a,b)
#define ipl_conv_YCbCr420lpToRGB666(a,b)  ipl_convert_ycbcr420lp_to_rgb666(a,b)
#define ipl_conv_420lpTo422lp(a,b)  ipl_convert_ycbcr420lp_to_ycbcr422lp(a,b)
#define ipl_conv_422lpTo420lp(a,b)  ipl_convert_ycbcr422lp_to_ycbcr420lp(a,b)
#define ipl_conv_420lpTo444lp(a,b)  ipl_convert_ycbcr420lp_to_ycbcr444lp(a,b)
#define ipl_conv_444lpTo420lp(a,b)  ipl_convert_ycbcr444lp_to_ycbcr420lp(a,b)
#define ipl_conv_422To422lp(a,b)      ipl_convert_ycbcr422_to_ycbcr422lp(a,b)
#define ipl_conv_422brTo422rblp(a,b)   ipl_convert_ycbcr422_to_ycbcr422lp(a,b)
#define ipl_conv_422lpTo422(a,b)      ipl_convert_ycbcr422lp_to_ycbcr422(a,b)
#define ipl_conv_ToRGB444666888(a,b)     ipl_convert_to_rgb444666888(a,b)
#define ipl_conv_RGBToYCbCr(a,b,c,d)     ipl_RGB2YCbCr(a,b,c,d)
#define ipl_conv_RGB888ToRGB565(a,b,c,d) ipl_RGB8882RGB565(a,b,c,d)
#define ipl_conv_RGB888ToRGB565pal(a,b,c,d,e) ipl_RGB8882RGB565plt(a,b,c,d,e)
#define ipl_conv_RGBA888ToRGB565pal(a,b,c,d,e) ipl_RGB8882RGB565plt(a,b,c,d,e)
#define ipl_conv_RGB888ToYCbCr(a,b,c,d)  ipl_RGB8882YCbCr(a,b,c,d)
#define ipl_conv_YCbCrToRGB(a,b,c,d)     ipl_YCbCr2RGB(a,b,c,d)
#define ipl_conv_420lpTo422(a,b)      ipl_convert_ycbcr420lp_to_ycbcr422(a,b)
#define ipl_conv_422To420lp(a,b)      ipl_convert_ycbcr422_to_ycbcr420lp(a,b)
#define ipl_convert_ycrcb420mb_to_ycrcb422lp(a,b)   ipl_convert_ycrcb420mb_to_ycrcb420lp(a,b)









#define ipl_convert_ycrcb420mb_to_ycrcb422lp(a,b) ipl_convert_ycrcb420mb_to_ycrcb420lp(a,b)
#define ipl_xform_CropDownsizeRot(a,b,c,d)  ipl_crop_downsize_rot(a,b,c,d)
#define ipl_comp_FillRect(a,b,c,d)        ipl_fill_rect(a,b,c,d)
#define ipl_util_FindAvg(a,b)             ipl_calc_avg(a,b)
#define ipl_util_FindMedian(a,b)          ipl_calc_median(a,b)
#define ipl_efx_Contrast(a,b,c)           ipl_set_contrast_8bit(a,b,c)
#define ipl_efx_ContrastNEntires(a,b,c,d) ipl_set_contrast_Nentries(a,b,c,d)
#define ipl_convert_frame(a,b,c)          ipl_conv_frame(a,b,c)
#define ipl_xform_CropYCbCrToRGB(a,b,c,d)     ipl_crop_YCbCrToRGB(a,b,c,d)
#define ipl_conv_SwapChromaOrder(a)             ipl_convert_swap_chroma_order(a)
#define ipl_util_BadPixelCorrect_5x5(a)   ipl_bad_pixel_correct_5x5(a)
#define ipl_hjr(a,b,c,d)                  ipl_shjr(a,b,c,d,0)

#ifndef FEATURE_IPL_LIGHT
  #define ipl_efx_Shear(a,b,c,d,e)          ipl_shear(a,b,c,d,e)
  #define ipl_efx_Pinch(a,b,c)              ipl_pinch(a,b,c)
#endif

#define ipl_efx_WhiteBoard(a,b,c,d,e)     ipl_whiteBoard(a,b,c,d,e,0)

// antiquated, since we added quality parameter to speed up calculation
#define ipl_util_CalcHistogram(a,b,c)     ipl_calc_histograms(a,b,c,IPL_QUALITY_MAX)
#define ipl_calc_histogram(a,b,c)         ipl_calc_histograms(a,b,c,IPL_QUALITY_MAX)


#endif

