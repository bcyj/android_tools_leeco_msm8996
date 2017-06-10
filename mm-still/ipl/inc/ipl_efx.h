#ifndef IPL_EFX_H
#define IPL_EFX_H
/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_efx.h#1 $
===========================================================================*/

extern ipl_grid ipl_srcMesh;
extern ipl_grid ipl_topStretch;
extern ipl_grid ipl_centerPinch;


/*===========================================================================
                           FUNCTION PROTOTYPES 
===========================================================================*/





#ifdef FEATURE_IPL_FUNC_CORNER_FOLD

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_corner_fold

DESCRIPTION
  This function folds over a corner of the image. The folded corner can 
  be the upper left (0), upper right (1), lower left (2), or lower 
  right (3) corner. The first input image is the image whose corner is 
  folded. The second input image is behind the first input image and 
  will be partially revealed by the folded corner.
  
  Input and output image sizes must be equal.

  Input and output must have the same color format, which can be  
  RGB565, YCbCr 4:2:2, YCbCr 4:2:0 line packed, or YCrCb 4:2:0 line 
  packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   points to the first input image
  in2_img_ptr   points to the second input image
  o_img_ptr     points to the output image
  position      position of the corner fold

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_corner_fold
(
  ipl_image_type* in1_img_ptr,  /* Points to the first input image  */
  ipl_image_type* in2_img_ptr,  /* Points to the second input image */
  ipl_image_type* out_img_ptr,  /* Points to the output image       */
  uint8 position                /* Position of corner fold          */
);
#endif



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_change_color

DESCRIPTION
  This function changes the color scheme of an image. There are 5 
  color-change options, each producing a different change in the 
  color scheme. The options are number from 0 through 4.
  
  Both inputs and output must have the same color format, which 
  can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr      points to the first input image
  out_img_ptr     points to the output image
  option          color change option

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_color_change
(
  ipl_image_type* in_img_ptr,    /* Points to the first input image  */
  ipl_image_type* out_img_ptr,   /* Points to the output image       */
  uint8 option                   /* Color change option              */
);



/*===========================================================================
 *
 * FUNCTION ipl_filter
 * 
 * DESCRIPTION
 *   This function performs filtering.  If input is YCbCr 4:2:2, the function 
 *   can perform Blur and Sharpen and also output YCbCr 4:2:2 or RGB565.
 *   If input is RGB565, the function supports only blurring and output must 
 *   be RGB565.
 * 
 * DEPENDENCIES
 *   None
 * 
 * ARGUMENTS IN
 *   i_img_ptr      pointer to the input image
 *   o_img_ptr      pointer to the output image
 *   filter         type of filtering operation
 * 
 * RETURN VALUE
 *   IPL_SUCCESS    indicates operation was successful
 *   IPL_FAILURE    otherwise
 * 
 * SIDE EFFECTS
 *   None
 *
===========================================================================*/
extern ipl_status_type ipl_filter
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  ipl_filter_type filter_i           /* Type of filtering operation    */
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_emboss

DESCRIPTION
  This function performs embossing of an image.  If input is YCbCr 4:2:2, 
  the function outputs YCbCr 4:2:2 or RGB565. If input is RGB565, the 
  function outputs RGB565.

  By nature, the mag most right columns and mag bottom rows cannot be emobossed
  and are hence, left black.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr      pointer to the input image
  o_img_ptr      pointer to the output image
  mag            strengh of operation

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_emboss
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  uint32 mag 
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_sketch

DESCRIPTION
  This function performs sketch effect on an image.  If input is YCbCr 4:2:2, 
  the function outputs YCbCr 4:2:2 or RGB565. If input is RGB565, the 
  function outputs RGB565.

  By nature, the mag most right columns and mag bottom rows cannot be emobossed
  and are hence, left black.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr      pointer to the input image
  o_img_ptr      pointer to the output image

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_sketch
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr          /* Points to the output image     */
);



/*===========================================================================
 *
 * FUNCTION ipl_convolution
 *
 * DESCRIPTION
 *   This function performs a convolution of the input kernel on the input
 *   and stores the output in the output image. If output image is NULL, the
 *   proces happens inplace. numCycles notes how many times to repeat the
 *   process.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   i_img_ptr      pointer to the input image
 *   o_img_ptr      pointer to the output image
 *   kernel         kernel to be used.
 *   numCycles      the number of times to run the kernel on the image
 *
 * RETURN VALUE
 *   IPL_SUCCESS    indicates operation was successful
 *   IPL_FAILURE    otherwise
 *
 * SIDE EFFECTS
 *  None
 *
 *===========================================================================*/
API_EXTERN ipl_status_type ipl_convolution
(
  ipl_image_type* input_img_ptr,     /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  ipl_kernel_type *kernel,           /* kernel to  use  */ 
  int numCycles                      /* how many times to run kernel */
);


#if 0
/* <EJECT> */
/*===========================================================================
 *
 * FUNCTION ipl_deblur
 *
 * DESCRIPTION
 *   This function deblurs and image
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 * s_img_ptr        Points to the sharp image  
 * b_img_ptr,       Points to the blurry image 
 *
 * ARGUMENTS OUT
 *   output_ptr       pointer to the output image
 *
 * RETURN VALUE
 *   IPL_SUCCESS      indicates operation was successful
 *   IPL_FAILURE      otherwise
 *
 * SIDE EFFECTS
 *   None
 *
 *===========================================================================*/
extern ipl_status_type ipl_deblur
(
  ipl_image_type *s_img_ptr,  /* Points to the sharp image  */
  ipl_image_type *b_img_ptr   /* Points to the blurry image */
);
#endif



/*===========================================================================

FUNCTION    ipl_test_and_set_block

DESCRIPTION

  This function modifies blodkcDx*blockDy blocks of an image based on some
  rules. Most often, if avg color of block does not match desired, block is
  turned white or black.

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
extern ipl_status_type
ipl_test_and_set_block(
  ipl_image_type *in, 
  ipl_image_type *in2,
  ipl_image_type *out,
  ipl_rect_type * crop, 
  int operation, 
  int blockDx,
  int blockDy, 
  uint32 a1, 
  uint32 a2, 
  uint32 a3,
  uint32 a4
);





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_brightness

DESCRIPTION
  This function makes and image brighter or darker. 

DEPENDENCIES
  None

ARGUMENTS IN
  desired_value    desired brightness setting from 0 to 100. 50 mean no change.
  input_ptr        pointer to the input image

ARGUMENTS OUT
  output_ptr       pointer to the output image

RETURN VALUE
  IPL_SUCCESS      indicates operation was successful
  IPL_FAILURE      otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_image_brightness
(
  ipl_image_type *input_img_ptr,  /* Points to the input image  */
  ipl_image_type *output_img_ptr, /* Points to the output image */
  int32 mult,                     /* what to multipl pix by */
  int32 add                       /* what to post add to pix by */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_fadein_fadeout

DESCRIPTION
  This function performs fade-in/fade-out for a sequence of still images.
  The user specifies the number of image frames in the sequence and the 
  width and height of each frame.  Fading in is option 0 and fading out is 
  option 1. All images in the sequence must have the same size.  The 
  user provides the still image to fade to/from.

  Input and output images must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr    pointer to the input image sequence
  in2_img_ptr    pointer to the still image to fade to/from
  out_img_ptr    pointer to the output image sequence
  numFrames      number of frames to use in the sequence
  in_out         indicator for fade-in (0) or fade-out (1)

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_fadein_fadeout
(
  ipl_image_type* in1_img_ptr, /* Points to input image sequence           */
  ipl_image_type* in2_img_ptr, /* Points to still image to fade to/from    */
  ipl_image_type* out_img_ptr, /* Points to output image sequence          */
  uint16 numFrames,            /* Number of image frames in input sequence */
  uint8 in_out                 /* Indicates fade-in(0) or fade-out(1)      */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_filter_image

DESCRIPTION
  This function performs filtering. Two types of filtering are supported, 
  Blur and Sharpen.
  
  BLUR:
  There are three types of blurring filters: (1) Gaussian blur, 
  (2) uniform blur, and (3) simple blur. Gaussian and uniform blur support 
  arbitrary sizes for the blurring window. The window size must be an odd, 
  positive integer in the range 3 <= size <= 1/2 * min(width, height). 
  Simple blur supports only a window size of 3.

  Gaussian and uniform blur support inputs that are RGB565, YCbCr 4:2:2, 
  or YCbCr 4:2:0 line packed format. Output images are always in the 
  same format as inputs.

  Simple blur supports inputs that are RGB565 or YCbCr 4:2:2. Outputs 
  can be RGB565 or YCbCr 4:2:2.

  SHARPEN:
  The sharpen filter uses a window size of 3. Input images must be in 
  YCbCr 4:2:2. Output can be RGB565 or YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr      pointer to the input image
  o_img_ptr      pointer to the output image
  filter         type of filtering operation

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_filter_image
(
  ipl_image_type* i_img_ptr,     /* Points to the input image      */
  ipl_image_type* o_img_ptr,     /* Points to the output image     */
  ipl_filter_type filter,        /* Type of filtering operation    */
  uint32 val                     /* Size of blurring window        */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_grayscale

DESCRIPTION
  This function converts a color YCbCr 4:2:2 or YCbCr 4:2:0 line packed image 
  to grayscale by ignoring the chroma component.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_image_grayscale
(
  ipl_image_type* input_img_ptr,        /* Points to the input image      */
  ipl_image_type* output_img_ptr        /* Points to the output image     */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_negative

DESCRIPTION
  This function creates the negative of an image.
  The input color format must be YCbCr 4:2:2 or YCbCr 4:2:0 line packed.
  The output color format must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line
  packed.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_image_negative
(
  ipl_image_type* i_img_ptr,        /* Points to the input image      */
  ipl_image_type* o_img_ptr         /* Points to the output image     */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_set_brightness

DESCRIPTION
  This function sets the multiplication coefficients based on
  the desired brightness level. The brightness is multiplicative.
  The delta step is 0.25. The dynamic range is a function of
  max_value - min_value.

DEPENDENCIES
  None

ARGUMENTS IN
  desired_value    desired brightness setting
  min_value        minimum brightness value
  max_value        maximum brightness value
  input_ptr        pointer to the input image

ARGUMENTS OUT
  output_ptr       pointer to the output image

RETURN VALUE
  IPL_SUCCESS      indicates operation was successful
  IPL_FAILURE      otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_set_brightness
(
  int32 desired_value,         /* Desired brightness setting */
  int32 min_value,             /* Minimum brightness value   */
  int32 max_value,             /* Maximum brightness value   */
  uint16 *input_ptr,           /* Points to the input image  */
  uint16 *output_ptr           /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_set_contrast

DESCRIPTION
  This function warps the gamma table for a particular contrast.

DEPENDENCIES
  None

ARGUMENTS IN
  contrast          index to which gamma table to use
  input_table     input gamma table

ARGUMENTS OUT
  output_table    output gamma table

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_set_contrast
(
  int32 contrast,                    /* Desired contrast value        */
  uint16* input_table,               /* Input gamma table             */
  uint16* output_table               /* Output gamma table            */
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_set_contrast_8bit

DESCRIPTION
  This function warps the gamma table for a particular contrast.

DEPENDENCIES
  None

ARGUMENTS IN
  contrast          index to which gamma table to use
  input_table     input gamma table

ARGUMENTS OUT
  output_table    output gamma table

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_set_contrast_8bit
(
  int32 contrast,                    /* Desired contrast value        */
  uint8* input_table,               /* Input gamma table             */
  uint8* output_table               /* Output gamma table            */
);




/* <EJECT> */
/*===========================================================================
 *
 * FUNCTION ipl_DoContrast
 *
 * DESCRIPTION
 *   This function modifies contrast for a given input image.
 *
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   i_img_ptr      pointer to the input image
 *   o_img_ptr      pointer to the output image (set to NULL if want inplace)
 *   curve          which gamma curve table to use (0 to 10) see curves above.
 *
 *
 * RETURN VALUE
 *   IPL_SUCCESS    indicates operation was successful
 *   IPL_FAILURE    otherwise
 *
 * SIDE EFFECTS
 *   None
 *
 *===========================================================================*/
API_EXTERN ipl_status_type ipl_DoContrast
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  int32 contrast                     /* how to adjust the gamma        */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_sepia

DESCRIPTION
  This function converts an image to sepia.
  The input must be YCbCr 4:2:2 or YCbCr 4:2:0 line packed.
  The output must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_image_sepia
(
  ipl_image_type* input_img_ptr,         /* Points to the input image      */
  ipl_image_type* output_img_ptr         /* Points to the output image     */
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_posterize

DESCRIPTION
  This function posterizes an image. The number of quantization levels must 
  be within the range [1..255].
  Input must be in YCbCr 4:2:2 or YCbCr 4:2:0 line packed.
  Output must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image
  output_img_ptr  pointer to the output image
  steps           number of quantization levels (range is [1..255])

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_image_posterize
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  uint8 steps                        /* Number of quantization levels  */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_solarize

DESCRIPTION
  This function solarizes an image. The threshold value for solarization 
  must be in the range [0..255].
  Input must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
  Output must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr    pointer to the input image
  output_img_ptr   pointer to the output image
  thresh           threshold value used for inversion (range is [0..255])

RETURN VALUE
  IPL_SUCCESS      indicates operation was successful
  IPL_FAILURE      otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_image_solarize
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  uint8 thresh                       /* Threshold to use for inversion */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_smart_flash

DESCRIPTION
  This function performs intensity equalization and color conversion 
  if required.

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
API_EXTERN ipl_status_type ipl_smart_flash
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr          /* Points to the output image     */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_set_contrast_Nentires

DESCRIPTION
  This function warps the gamma table for a particular contrast.
  Always output 8-bit values. 

DEPENDENCIES
  None

ARGUMENTS IN
  contrast        index to which gamma table to use
  ibits           number of bit required to represent number of entires,
                  i.e. enter 8 for 256 (0-255) entires, 10-bit for 1024 
                  (0 to 1023).
  input_table     input gamma table

ARGUMENTS OUT
  output_table    output gamma table

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_set_contrast_Nentries
(
  int32 contrast,                    /* Desired contrast value            */
  int32 ibits,                       /* number of bits needed for number
                                        of entries (8,10,...)             */
  uint8* input_table,               /* Input gamma table                 */
  uint8* output_table               /* Output gamma table                */
);


#ifndef FEATURE_IPL_LIGHT



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_hue

DESCRIPTION
  This function performs hue scaling and color conversion if required.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr      pointer to the input image
  o_img_ptr      pointer to the output image
  hue_factor     an additive hue scale factor (range is -360 to 360)

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_hue
(
  ipl_image_type* i_img_ptr,         /* Points to the input image          */
  ipl_image_type* o_img_ptr,         /* Points to the output image         */
  int32 hue_factor                   /* Hue scaling factor                 */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_fog

DESCRIPTION
  This function creates a foggy image. The user specifies the level of 
  fogginess, either light (0) or heavy (1).
  
  Input and output images must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    pointer to the input image
  out_img_ptr   pointer to the output image
  level         fog level, either light (0) or heavy (1)

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_fog
(
  ipl_image_type* in_img_ptr,    /* Points to the input image   */
  ipl_image_type* out_img_ptr,   /* Points to the output image  */
  uint8 level                    /* Level of fogginess          */
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_radial_blur

DESCRIPTION
  This function performs radial blur. The blurring level can be set 
  to an integer between 0 and 100, inclusive. No blurring occurs 
  when the blurring level is 0.

  Input and output image sizes must be equal.
  
  Input and output images must have the same color format, which 
  can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr points to the input image
  o_img_ptr points to the output image
  level is the level of blurring (range: 0 to 100)

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_radial_blur
(
  ipl_image_type* i_img_ptr,   /* Points to the input image           */
  ipl_image_type* o_img_ptr,   /* Points to the output image          */
  int32 xo,                    /* x coordinate of center of ratation  */
  int32 yo,                    /* y coordinate of center of ratation  */
  uint8 level                  /* Blurring level                      */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_sat

DESCRIPTION
  This function performs saturation scaling and color conversion if required.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr     pointer to the input image
  o_img_ptr     pointer to the output image
  sat_factor    additive saturation scale factor (range is -255 to 255)

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_sat
(
  ipl_image_type* i_img_ptr,         /* Points to the input image          */
  ipl_image_type* o_img_ptr,         /* Points to the output image         */
  int32 sat_factor                   /* Saturation scaling factor          */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_int

DESCRIPTION
  This function performs intensity scaling.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr     pointer to the input image
  o_img_ptr     pointer to the output image
  int_factor    intensity scale factor (range is -255 to 255)

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_int
(
  ipl_image_type* i_img_ptr,         /* Points to the input image          */
  ipl_image_type* o_img_ptr,         /* Points to the output image         */
  int32 int_factor                   /* Intensity scaling factor           */
);

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_magnify

DESCRIPTION
  This function magnifies an input image in a circular region of 
  arbitrary location and radius.  The center of the magnified region 
  must be within the input image boundaries. Areas outside the circular 
  region are not magnified. If part of the magnified region extends 
  beyond image boundaries, only the part within boundaries is 
  displayed. 
  
  Input and output image sizes must be equal.
  
  Input and output images must have the same color format, which 
  can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr     pointer to the input image
  out_img_ptr    pointer to the output image
  circle         the circular region of magnification

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_magnify
(
  ipl_image_type* in_img_ptr,    /* Points to the input image        */
  ipl_image_type* out_img_ptr,   /* Points to the output image       */
  ipl_circle_type* circle        /* Circular region of magnification */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_motion_blur

DESCRIPTION
  This function produces motion blur in any arbitrary direction. The size 
  of the blurring window must be an odd, positive integer in the range 
  3 <= size <= 1/2 * min(w, h), where w and h are the width and height of 
  the input image. The direction of motion is specified by a positive integer 
  angle measured in degrees from the horizontal.

  Input and output images must have the same size.
  Input and output images must be in the same color format, which can be 
  RGB565, YCbCr 4:2:2, YCbCr 4:2:0 line packed, or YCrCb 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    pointer to the input image
  out_img_ptr   pointer to the output image
  size          size of blurring window
  angle         angle of motion blur

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_motion_blur
(
  ipl_image_type* in_img_ptr,   /* Points to the input image  */
  ipl_image_type* out_img_ptr,  /* Points to the output image */
  uint32 size,                  /* Width of blurring window   */
  uint16 angle                  /* Angle of motion blur       */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_neon

DESCRIPTION
  This function creates a neon version of the input image.  Input and 
  output images must be the same size.
  
  Input and output images must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

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
API_EXTERN ipl_status_type ipl_neon
(
  ipl_image_type* i_img_ptr,    /* Points to the input image  */
  ipl_image_type* o_img_ptr     /* Points to the output image */
);



/* <EJECT> */
/*===========================================================================
 *
 * FUNCTION ipl_luma_adaptation
 *
 * DESCRIPTION
 *   This function stretches the histogram of an image to provide more detail
 *   in the lights and darks.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   i_img_ptr      pointer to the input image
 *   o_img_ptr      pointer to the output image
 *
 * RETURN VALUE
 *   IPL_SUCCESS    indicates operation was successful
 *   IPL_FAILURE    otherwise
 *
 * SIDE EFFECTS
 *   None
 *
 *===========================================================================*/
API_EXTERN ipl_status_type ipl_luma_adaptation
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  ipl_histogram_type *hist,
  uint16 thresholdQ8
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_median_blur

DESCRIPTION
  This function performs median blurring.  If input is YCbCr 4:2:2,
  the function outputs YCbCr 4:2:2.  If input is RGB565, the function
  outputs RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr      pointer to the input image
  o_img_ptr      pointer to the output image
  mag            magnitude of blur

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_median_blur
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  uint32 mag
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_efx_adaptiveFilter

DESCRIPTION

  This function processes Y channel data only. It sharpens the edges while
  reducing noise if noise is detected.

  Input and output images must have the same color format, which can 
  be YCbCr 4:2:2 packed format with Y starting on second byte.

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
extern ipl_status_type ipl_efx_adaptiveFilter
(
  ipl_image_type* iptr,    /* Points to the input image  */
  ipl_image_type* optr     /* Points to the output image */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_uniform_blur

DESCRIPTION
  This function produces uniform blur using a uniform window of arbitrary 
  size. The window size must be an odd, positive integer in the range 
  1 <= size <= 1/2 * min(width, height), where width and height are the 
  width and height of the input image. Size of 1 is no blur.

  Input and output images must have the same size.

  Input and output images must have the same color format, which can be
  RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    pointer to the input image
  out_img_ptr   pointer to the output image
  size          size of the uniform window

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_uniform_blur
(
  ipl_image_type* in_img_ptr,    /* Points to the input image  */
  ipl_image_type* out_img_ptr,   /* Points to the output image */
  uint32 size                    /* Size of blurring window */
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_gaussian_blur

DESCRIPTION
  This function produces Gaussian blur using a Gaussian window of arbitrary 
  size. The window size must be an odd, positive integer in the range 
  3 <= size <= 1/2 * min(width, height), where width and height are the 
  width and height of the input image.

  Input and output images must be the same size.
  
  Input and output images must have the same color format, which can be 
  RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr    pointer to the input image
  out_img_ptr   pointer to the output image
  size          size of Gaussian window

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_gaussian_blur
(
  ipl_image_type* in_img_ptr,       /* Points to the input image  */
  ipl_image_type* out_img_ptr,      /* Points to the output image */
  uint32 size                       /* Size of blurring window */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_efx_meshWarp

DESCRIPTION

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_efx_meshWarp
(
  ipl_image_type* input_img_ptr,         /* Points to the input image      */
  ipl_image_type* output_img_ptr,        /* Points to the output image     */
  ipl_grid* dstMesh,
  boolean opp
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_watercolor

DESCRIPTION
  This function performs watercolor effect. If input is YCbCr 4:2:2, 
  the function outputs YCbCr 4:2:2.  If input is RGB565, the function 
  outputs RGB565.
  
  The kernel size is 3x3. This size kernel is not adequate for images
  having resolution greatern than 500x500 and larger kenrnel should be used.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr      pointer to the input image
  o_img_ptr      pointer to the output image
  mag            magnitude of blur

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_watercolor
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* o_img_ptr,         /* Points to the output image     */
  uint32 mag
);



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
 *  rm_border      tile size when trying to remove area outside whiteboard
 *  gray           0 = auto, 1 =  make grayscale, 0 = dont make grayscale
 *
 * RETURN VALUE
 *  IPL_SUCCESS    indicates operation was successful
 *  IPL_FAILURE    otherwise
 *
 * SIDE EFFECTS
 *  None
 *
 *===========================================================================*/
API_EXTERN ipl_status_type ipl_whiteBoard
(
  ipl_image_type * in_img_ptr,
  ipl_image_type *out_img_ptr,
  int blackMode, 
  int th_noise,
  int rm_border,
  int gray
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_analyze_histogram

DESCRIPTION
  This function stretches the histogram of an image to provide more detail
  in the lights and darks.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr      pointer to the input image
  o_img_ptr      pointer to the output image
  thresholdQ8

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_analyze_histogram
(
  ipl_image_type      * i_img_ptr,
  ipl_histogram_type  * hist,
  ipl_rect_type       * crop,
  uint32         mode
);



/* <EJECT> */
/*===========================================================================
FUNCTION ipl_red_eye

DESCRIPTION
  This function finds and fixes red eye

DEPENDENCIES
  None

ARGUMENTS IN
  in      pointer to the input image
  out     pointer to the output image
  icrip   where to search
  ocrip   where to put output
  arg1    future use
  arg2    future use

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None
=========================================================================== */
API_EXTERN ipl_status_type ipl_red_eye(ipl_image_type * in,
                                      ipl_image_type * out,
                                      ipl_rect_type * icrop,
                                      ipl_rect_type * ocrop,
                                      void * arg1,
                                      void * arg2);


#endif







#endif
