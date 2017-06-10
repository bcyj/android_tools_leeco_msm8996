#ifndef IPL_UTIL_H

#define IPL_UTIL_H

/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_util.h#1 $
===========================================================================*/


/*===========================================================================
                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------

                   Do not add history to this file. Add history to resepctive
                   file's .c where change was made. Thank you. This history
                   predates the splitting of ipl.c into the 6 various .cs.

08/08/04   bf      Split ipl.h and ipl.c into categories, ipl_efx.c, etc
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


extern const uint8  rgb565_table[];
extern const uint32 r666[];
extern const uint32 g666[];
extern const uint32 b666[];
extern const uint16 r444[];
extern const uint16 g444[];
extern const uint16 b444[];

#define IPL_IS_QNUMBER(x) switch (x)\
                          {\
                            case 128:  x=7; break;\
                            case 256:  x=8; break;\
                            case 8:    x=3; break;\
                            case 16:   x=4; break;\
                            case 32:   x=5; break;\
                            case 64:   x=6; break;\
                            case 512:  x=9; break;\
                            case 1024: x=10; break;\
                            case 2048: x=11; break;\
                            case 4096: x=12; break;\
                            case 8192: x=13; break;\
                            case 16384:x=14; break;\
                            case 32768:x=15; break;\
                            case 65536:x=16; break;\
                            case 0:    x=0; break;\
                            case 2:    x=1; break;\
                            case 4:    x=2; break;\
                            default:   x=-1; break;\
                          };


/* <EJECT> */
/*===========================================================================
                             UTILITY TABLES 
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
);




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
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_bad_pixel_correct

DESCRIPTION
  This function performs bad pixel correction.
  The input image is overwritten by the output image.
  The input must be in either IPL_BAYER_BGGR or IPL_BAYER_RGGB format.
  The output must be in the same color format as the input.

DEPENDENCIES
  None

ARGUMENTS IN
  img_ptr        pointer to the input and also the output image
  min_factor     multiplicant of the min pixel value in Q12 Unsigned
  max_factor     multiplicant of the max pixel value in Q12 Unsigned

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_bad_pixel_correct
(
  ipl_image_type* img_ptr          /* Points to the input image           */
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_make_gamma_table

DESCRIPTION
  This function creates a gamma table for a given gamma factor.

DEPENDENCIES
  None

ARGUMENTS IN
  first   power of first gamma curve  (0.5)
  lasta   power of second gamma curve  (2.6)
  step    step (0.1)

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_make_gamma_table
(
  float first, 
  float last, 
  float step
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_bad_pixel_correct_5x5

DESCRIPTION
  This function performs bad pixel correction.
  The input image is overwritten by the output image.
  The input must be in either IPL_BAYER_BGGR or IPL_BAYER_RGGB format.
  The output must be in the same color format as the input.

DEPENDENCIES
  None

ARGUMENTS IN
  img_ptr        pointer to the input and also the output image
  min_factor     multiplicant of the min pixel value in Q12 Unsigned
  max_factor     multiplicant of the max pixel value in Q12 Unsigned

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_bad_pixel_correct_5x5
(
  ipl_image_type* img_ptr          /* Points to the input image           */
);




/*===========================================================================

FUNCTION ipl_unpack_rgb565

DESCRIPTION
  This function takes 16 bits rgb565 and unpacks into 24 bit r g b

DEPENDENCIES
  None

ARGUMENTS IN
  in is a word input rgb565

ARGUMENTS OUT
  r - address of R value
  g - address of G value
  b - address of B value


RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type unpack_rgb565
(
  unsigned short in,
  unsigned char* r,
  unsigned char* g,
  unsigned char* b
);


/* <EJECT> */
/*===========================================================================

FUNCTION unpack_rgb444

DESCRIPTION
  This function takes 16 bits rgb444 and unpacks into 24 bit r g b

DEPENDENCIES
  None

ARGUMENTS IN
  in is a word input rgb444

ARGUMENTS OUT
  r, g and b of input as char

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  04/09/04  Created

===========================================================================*/
extern ipl_status_type unpack_rgb444
(
  unsigned short in, 
  unsigned char* r, 
  unsigned char* g,
  unsigned char* b
);




/* <EJECT> */
/*===========================================================================

FUNCTION unpack_rgb666

DESCRIPTION
  This function takes 32 bits rgb666 and unpacks into 24 bit r g b

DEPENDENCIES
  None

ARGUMENTS IN
  in is a 32 bit word (4 byte) input rgb666

ARGUMENTS OUT
  r, g and b of input as char

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  04/09/04  Created

===========================================================================*/
extern ipl_status_type unpack_rgb666
(
  unsigned long in, 
  unsigned char* r, 
  unsigned char* g,
  unsigned char* b
);



/* <EJECT> */
/*===========================================================================

FUNCTION min3

DESCRIPTION
  This function will find min of 3 numbers

DEPENDENCIES
  None

ARGUMENTS IN
  a,b,c are the 3 input numbers

RETURN VALUE
  It returns as output minimum of a,b,c

SIDE EFFECTS
  None

===========================================================================*/
extern int32 min3(int32 a, int32 b, int32 c);


/*===========================================================================

FUNCTION max3

DESCRIPTION
  This function will find max of 3 numbers

DEPENDENCIES
  None

ARGUMENTS IN
  a,b,c are the 3 input numbers

RETURN VALUE
  It returns as output maximum of a,b,c

SIDE EFFECTS
  None

===========================================================================*/
extern int32 max3(int32 a, int32 b, int32 c);



/* <EJECT> */
/*===========================================================================

FUNCTION calc_img_hist

DESCRIPTION
  This function will calculate the image histogram in Q8 unsigned

DEPENDENCIES
  None

ARGUMENTS IN/OUT
  input_img is the input image whose histogram needs to be calculated
  hist_ptr is a pointer to where the output should be written. (Q8)
  gray_levels is the number of gray levels

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type calc_img_hist
(
  ipl_image_type* input_img_ptr,
  uint32* img_hist,
  uint32 gray_levels,
  uint32 start_loc,
  uint32 row_dim,
  uint32 col_dim
);





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
  in1                  area of input 1 image to process
  in2                  area of input 1 image to process

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
  ipl_rect_type * in1,
  ipl_rect_type * in2
);


/*===========================================================================
 *
 * FUNCTION    ipl_calc_minmax
 *
 * DESCRIPTION
 *
 *   This function computes the min and max for crop area of pixels.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   in          input image
 *   crop        area in question
 *
 * ARGUMENTS IN/OUT
 *
 *   min Returns min Y,Cb,Cr packed as 24 bits into uint32
 *   max Returns min Y,Cb,Cr packed as 24 bits into uint32
 *
 * RETURN VALUE
 *
 *  IPL_SUCCESS          indicates operation was successful
 *  IPL_FAILURE          otherwise
 *
 * SIDE EFFECTS
 *  None
 *
 *===========================================================================*/
API_EXTERN ipl_status_type ipl_calc_minmax
(
  ipl_image_type *in, 
  ipl_rect_type *crop,
  uint32 * min, 
  uint32 * max
);


/*===========================================================================
 *
 * FUNCTION    ipl_calc_avg
 *
 * DESCRIPTION
 *
 *   This function computes the average for crop area of pixels.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   in          input image
 *   crop        area in question
 *
 * ARGUMENTS IN/OUT
 *
 *   Returns average Y,Cb,Cr packed as 24 bits into uint32
 *
 * RETURN VALUE
 *
 *   Returns average Y,Cb,Cr packed as 24 bits into uint32
 *
 * SIDE EFFECTS
 *  None
 *
 *===========================================================================*/
API_EXTERN uint32 ipl_calc_avg
(
  ipl_image_type *in, 
  ipl_rect_type *crop
);



/*===========================================================================
 *
 * FUNCTION    ipl_calc_median
 *
 * DESCRIPTION
 *
 *   This function computes the median for crop area of pixels.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   in          input image
 *   crop        area in question
 *
 * ARGUMENTS IN/OUT
 *
 *   Returns median Y,Cb,Cr packed as 24 bits into uint32
 *
 * RETURN VALUE
 *
 *   Returns median Y,Cb,Cr packed as 24 bits into uint32
 *
 * SIDE EFFECTS
 *  None
 *
 *===========================================================================*/
API_EXTERN uint32 ipl_calc_median
(
  ipl_image_type *in, 
  ipl_rect_type *crop
);


/*===========================================================================
 *
 * FUNCTION    ipl_calc_mchan_histogram
 *
 * DESCRIPTION
 *
 *   This function computes the luma histogram for crop area of pixels.
 *   It takes in RGB565 or YCbCr
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   in          input image
 *   crop        area in question
 *
 * ARGUMENTS IN/OUT
 *   hist        Returns luma histogram
 *
 * RETURN VALUE
 *
 * SIDE EFFECTS
 *  None
 *
 *===========================================================================*/
API_EXTERN ipl_status_type ipl_calc_mchan_histograms
(
  ipl_image_type *in, 
  ipl_rect_type *crop,
  ipl_mchan_histogram_type *hist,
  ipl_quality_type qual
);


/*===========================================================================
 *
 * FUNCTION    ipl_histogram
 *
 * DESCRIPTION
 *
 *   This function computes the luma histogram for crop area of pixels.
 *   It takes in RGB565 or YCbCr
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   in          input image
 *   crop        area in question
 *
 * ARGUMENTS IN/OUT
 *   hist        Returns luma histogram
 *
 * RETURN VALUE
 *
 * SIDE EFFECTS
 *  None
 *
 *===========================================================================*/
API_EXTERN ipl_status_type ipl_calc_histograms
(
  ipl_image_type *in, 
  ipl_rect_type *crop,
  ipl_histogram_type *hist,
  ipl_quality_type qual
);




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
API_EXTERN ipl_status_type ipl_count_members
(
  ipl_image_type *in,
  ipl_rect_type *crop, 
  int action, 
  uint32 * a1n, 
  uint32 * a2, 
  uint32 * a3
);




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
);


/*===========================================================================
 * ===========================================================================*/
void ipl_noop(char * str, void * a, void * b, void * c);


#endif 
