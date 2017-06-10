#ifndef IPL_XFORM_H
#define IPL_XFORM_H


/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_xform.h#1 $
===========================================================================*/



/*===========================================================================

                          FUNCTION PROTOTYPES

===========================================================================*/


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_downsize

DESCRIPTION
  This function performs resizing and color conversion. The bip_ptr can be 
  set to NULL if no color processing is desired. Demosaicing is still 
  performed when the bip_ptr is set to NULL.

  The input can be YCbCr 4:2:2, RGB565, Bayer, or RGB888.
  The output can be YCbCr 4:2:2 or RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr    pointer to the input image
  output_img_ptr   pointer to the output image
  bip_ptr          pointer to a BIP structure (for Mega Pixel Color 
                   Processing Support)

RETURN VALUE
  IPL_SUCCESS      indicates operation was successful
  IPL_FAILURE      otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_downsize
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
);



/* <EJECT> */
/*===========================================================================
 *
 * FUNCTION ipl_downsize_med
 *
 * DESCRIPTION
 *   This function downsizes YCbCr Input.  It uses a 9 point averaging scheme.
 *   The minimum downsize factor is 4 in each dimension. ie the output should
 *   be smaller than 1/16 the input size.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   input_img_ptr points to the input image
 *
 * ARGUMENTS IN
 *   output_img_ptr points to the output image
 *
 * RETURN VALUE
 *   IPL_SUCCESS   is operation was succesful
 *   IPL_FAILURE   otherwise
 *
 *  SIDE EFFECTS
 *    None
 *
 *===========================================================================*/
extern ipl_status_type ipl_downsize_med
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
);


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
);



#if 0

/* <EJECT> */
/*===========================================================================
 *
 * FUNCTION ipl_xform_Resize_cubicHigh
 *
 * DESCRIPTION
 *   This function is used to do cubic upsize
 *   The input should be RGB565 and the output should be RGB565.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   input_img_ptr points to the input image
 *   output_img_ptr points to the output image
 *
 * RETURN VALUE
 *   IPL_SUCCESS   is operation was succesful
 *   IPL_FAILURE   otherwise
 *
 * SIDE EFFECTS
 *   None
 *
 *===========================================================================*/
extern ipl_status_type ipl_xform_Resize_cubicHigh
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  cropin,
  ipl_rect_type*  cropout
);

#endif



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_resize_rot

DESCRIPTION
  This function performs cropping on input, downsizing/upsizep, and then
  convestion to rgb

  The input must be YCxCx420 or YCxCx422 line pack;

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         pointer to the input image
  o_img_ptr         pointer to the output image
  cropin            region on input to be cropped
  cropout           region on output to be cropped
  rot               0,90,180,270 rotation
  qual              quality type

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_crop_resize_rot
(
  ipl_image_type* i_img_ptr,        /* Points to the input image        */
  ipl_image_type* o_img_ptr,        /* Points to the output image       */
  ipl_rect_type* cropin,            /* Crop input config                */
  ipl_rect_type* cropout,           /* Crop output config               */
  ipl_rotate90_type rot,           /* rotation param */
  ipl_quality_type qual
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_downsize_rot

DESCRIPTION
  This function performs cropping, downsizing and then rotation
  of an image.

  The input can be either YCxCx422 or YCxCx420 line pack

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         pointer to the input image
  o_img_ptr         pointer to the output image
  crop              region to be cropped (optional, can set to NULL)
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
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_reflect

DESCRIPTION
  This function performs reflection and color conversion if needed.
  Only RGB565 and YCbCr 4:2:2 inputs are supported.
  If input is RGB565, only RGB565 output is supported.
  If input is YCbCr 4:2:2, RGB565 and YCbCr 4:2:2 output are supported.

  If the output is NULL, the reflection will happen in place.

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
);



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
);


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
  ipl_rect_type* crop,              /* Crop config                      */
  ipl_rotate90_type rotate,         /* Rotatation                       */
  ipl_icon_type** icon_list_ptr,    /* Ptr to null terminated icon list */
  uint16 transparentValue           /* Transparent pixel value          */
);




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
);



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
);



/*===========================================================================

FUNCTION ipl_xform_Upsize_Crop_qLow

DESCRIPTION
  This function upsizes (i.e. enlarges) an image.
  Input and output must be in RGB565 format.
  Fast, but low quality

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image
  output_img_ptr  pointer to the output image
  cropi           area in input to crop
  cropo           area in input to crop

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_xform_Upsize_Crop_qLow
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type* cropi,             /* area in input to crop               */
  ipl_rect_type* cropo              /* area in output to crop               */
);



/*===========================================================================

FUNCTION ipl_xform_Upsize_qLow

DESCRIPTION
  This function upsizes (i.e. enlarges) an image.
  Input and output must be in RGB565 format.
  Fast, but low quality

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
extern ipl_status_type ipl_xform_Upsize_qLow
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type* crop               /* area in input to crop               */
);




/*===========================================================================

FUNCTION ipl_xform_Resize_qHigh

DESCRIPTION
  This function upsizes (i.e. enlarges) an image.
  Input and output must be in RGB565 format.

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
extern ipl_status_type ipl_xform_Resize_qHigh
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type* crop,              /* area in input to crop               */
  ipl_rect_type* cropout            /* where to put output                 */
);


/*===========================================================================

FUNCTION ipl_upsize

DESCRIPTION
  This function upsizes (i.e. enlarges) an image.
  Input and output must be in RGB565 format.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image
  output_img_ptr  pointer to the output image
  crop            area in the output to crop
  quality         quality of output

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_upsize
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type* crop,              /* area in output to crop               */
  ipl_quality_type qualty           /* quality parameter */
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycrcb420lp_to_rgb

DESCRIPTION
  This functin will accept YCbCr 4:2:0 as input and output RGB565 or 444
  as a double word as follows.  The Most Significant 14 bits are 0, followed 
  by 6 bits of R, then G and then B.

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
extern ipl_status_type ipl_crop_ycrcb420lp_to_rgb
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* cropin, 
  ipl_rect_type* cropout
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycbcr422lp_to_rgb

DESCRIPTION
  This functin will accept YCbCr 4:2:2 as input and output RGB444 or 565
  Input should be 4:2:2 line packed Format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  cropin:         Crop in input
  cropout:        Where to put in output

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
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycbcr420lp_to_rgb

DESCRIPTION
  This functin will accept YCbCr 4:2:0 as input and output RGB565 or 444
  as a double word as follows.  The Most Significant 14 bits are 0, followed 
  by 6 bits of R, then G and then B.

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
extern ipl_status_type ipl_crop_ycbcr420lp_to_rgb
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* cropin, 
  ipl_rect_type* cropout
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycrcb422lp_to_rgb

DESCRIPTION
  This functin will accept YCrCb 4:2:2 as input and output RGB444 or 565
  Input should be 4:2:2 line packed Format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  cropin:         Crop in input
  cropout:        Where to put in output

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
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycbcr_to_rgb

DESCRIPTION
  This functin will accept YCbCr 4:2:2 as input and output RGB565 or 444
  as a double word as follows.  

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  cropin:         Crop intput 
  cropout:        Crop output

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
);


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
  cropin          Ared in input to crop
  cropout         Where to put in output (must have same dim as input crop)

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
);


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
  up (0), down (1), left (2), or right (3), as illustrated below.

  For any orientation, the top width is defined as the smaller 
  of the two parallel edges of the trapezoid.

  Input and output images must have the same color format, which 
  can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

  (0) up:
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
);



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
);


/*===========================================================================
 *
 * FUNCTION ipl_upsize_rot_crop
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
 *   i_param           - Parameters relating to upsize.
 *   rdx               - how much to upsize image to in dx (pre rotation dim)
 *   rdy               - how much to upsize image to in dy (pre rotation dim)
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
extern ipl_status_type
ipl_upsize_rot_crop
(
   ipl_image_type*        i_img_ptr, /* Points to the input image   */
   ipl_image_type*        o_img_ptr, /* Points to the output image  */
   int                    rdx,
   int                    rdy,
   ipl_rect_type*         cropout,
   ipl_rotate90_type      rot,
   ipl_quality_type       qual
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_resizeFrame

DESCRIPTION
  This function is used to do upsize and downsize usin nearest neighbor
  The input should be RGB565 and the output should be RGB565.
  It preserves transparency.

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
API_EXTERN ipl_status_type ipl_resizeFrame
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_rect_type*  cropout,
  uint16 transparentValue           /* Transparent pixel value          */
);




/* <EJECT> */
/*===========================================================================
 *
 * FUNCTION ipl_crop_resize_ycrcblp
 *
 * DESCRIPTION
 *   This function performs cropping on input, upsizing
 *
 *   The input must be YCxCx420 or YCxCx422 line pack;
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   i_img_ptr         pointer to the input image
 *   o_img_ptr         pointer to the output image
 *   cropin            region on input to be cropped
 *   cropout           region on output to be cropped
 *
 * RETURN VALUE
 *   IPL_SUCCESS       indicates operation was successful
 *   IPL_FAILURE       otherwise
 *   SIDE EFFECTS      None
 *
 *===========================================================================*/
extern ipl_status_type ipl_crop_resize_ycrcblp
(
  ipl_image_type* i_img_ptr,        /* Points to the input image        */
  ipl_image_type* o_img_ptr,        /* Points to the output image       */
  ipl_rect_type* cropin,            /* Crop config                      */
  ipl_rect_type* cropout,           /* Crop config                      */
  ipl_quality_type qual
);




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
);




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
);



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
);



// Misc interface changes
//#define ipl_upsize(a,b,c,d) ipl_upsize_crop(a,b,NULL,c,d)
#define ipl_crop_ycbcr420lp_to_rgb666(a,b,c,d) ipl_crop_ycbcr420lp_to_rgb(a,b,c,d)
#define ipl_crop_ycrcb420lp_to_rgb666(a,b,c,d) ipl_crop_ycrcb420lp_to_rgb(a,b,c,d)
#define ipl_crop_ycbcr420lp_to_rgb565(a,b,c,d) ipl_crop_ycbcr420lp_to_rgb(a,b,c,d)
#define ipl_crop_ycrcb420lp_to_rgb565(a,b,c,d) ipl_crop_ycrcb420lp_to_rgb(a,b,c,d)
#define ipl_crop_ycbcr422lp_to_rgb565(a,b,c,d) ipl_crop_ycbcr422lp_to_rgb(a,b,c,d)
#define ipl_crop_ycrcb422lp_to_rgb565(a,b,c,d) ipl_crop_ycrcb422lp_to_rgb(a,b,c,d)


#endif   /* IPL_XFORM_H  */
