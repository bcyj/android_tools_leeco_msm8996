#ifndef IPL_COMPOSE_H
#define IPL_COMPOSE_H

/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_compose.h#1 $
===========================================================================*/


/*===========================================================================
                            FUNCTION PROTOTYPES 
===========================================================================*/



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
);


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
);


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
  RGB565 or YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  cut_area is the input area to be cut out
  paste_area is the output area to be pasted onto
  fillerPixel is the pixel color used to replace the cut area

ARGUMENTS IN/OUT
  out_img_ptr points to the output image
  in_img_ptr  points to input image which now has cut region of 
              fillerPixel color

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
  ipl_rect_type* cut_area,      /* Input region to be cut out          */
  ipl_rect_type* paste_area,    /* Output region to be pasted onto     */
  uint32   fillerPixel          /* color value of pixel to replace cut area */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste

DESCRIPTION
  This function performs cut-and-paste. A region is copied from the input 
  image and pasted onto a region in the output image. Input and output 
  image sizes can be different. If the input "copy" region is strictly 
  larger than the output "paste" region, the input region will be downsized 
  to fit the output region. If the images are RGB565, an input "copy" region 
  can also be upsized if it is strictly smaller than the output "paste" 
  region.

  Input and output images must be:
  both rgb565 or,
  both YCbCr or,
  YCxCx42y line pack <---> YCxCx42y line pack (where x is r|b and y is 2|0),

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copied out
  paste_area is the output area to be pasted onto

ARGUMENTS IN/OUT
  out_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_copy_and_paste
(
  ipl_image_type* in_img_ptr,   /* Points to the input image           */
  ipl_image_type* out_img_ptr,  /* Points to the output image          */
  ipl_rect_type* copy_area,      /* Input region to be copied out          */
  ipl_rect_type* paste_area     /* Output region to be pasted onto     */
);





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_copy_and_paste_blend

DESCRIPTION
  This function performs cut-and-paste. A region is copied from the input 
  image and pasted onto a region in the output image. Input and output 
  image sizes can be different. If the input "copy" region is strictly 
  larger than the output "paste" region, the input region will be downsized 
  to fit the output region. If the images are RGB565, an input "copy" region 
  can also be upsized if it is strictly smaller than the output "paste" 
  region.

  Input and output images must be:
  both rgb565 or,
  both YCbCr or,
  YCxCx42y line pack <---> YCxCx42y line pack (where x is r|b and y is 2|0),

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr points to the input image
  copy_area is the input area to be copied out
  paste_area is the output area to be pasted onto
  blend, width of blending
  whereBlend, which 4 sides to blend

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
);


/*==========================================================================

FUNCTION    ipl_alpha_blend_area

DESCRIPTION

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr         points to the input image
  i_frame_ptr       points to the frame image
                    reveal

ARGUMENTS IN/OUT


RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None
===========================================================================*/
API_EXTERN ipl_status_type ipl_alpha_blend_area
(
  ipl_image_type* i_img_ptr,
  ipl_image_type* i_frame_ptr,
  ipl_rect_type*  area,
  uint32          alpha,
  uint16          transparentValue
);




/*==========================================================================

FUNCTION    ipl_overlay_inplace

DESCRIPTION
  This function takes three images. One is the image on which overlay is to be
  done and the another is the overlay image. A third image is the alpha channel
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
);


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
  out_img_ptr   pointer to the output image
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
  ipl_image_type* out_ptr,    /* Points to the output image       */
  ipl_compose_type method,    /* Method of compositing            */
  void * misc1                /* misch input */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_compose_image

DESCRIPTION
  This function combines two images in various ways.
  
  Input and output images must have the same color format, which can 
  be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.

DEPENDENCIES
  None

ARGUMENTS IN
  in1_img_ptr   pointer to the first input image
  in2_img_ptr   pointer to the second input image
  out_img_ptr   pointer to the output image
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
  ipl_image_type* out_ptr,    /* Points to the output image       */
  ipl_compose_type method,    /* Method of compositing            */
  void * misc1,               /* misc input1 */
  int misc2,               /* misc input2 */
  int misc3,               /* misc input3 */
  int misc4                /* misc input4 */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_add

DESCRIPTION
  This function adds a frame around an image and performs color conversion 
  if needed.

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
);


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
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_draw_box

DESCRIPTION
  This function draws/fills a rectangular region of the input image with filler
  Pixel and returns the result in output. If output is NULL, the operation
  happens on the input.
  
  Input and output images must have the same color format, which can be 
  RGB565, YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN
  in_img_ptr*    pointer to input image
  out_img_ptr*   pointer to output image, if NULL, operation happens on input.
  fill_rect      pointer to a structure indicating the fill_rect region
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
  int fill_too,                  /* should we will it too */
  uint32 fillerPixel             /* color to fill region with        */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_add_bill

DESCRIPTION
  This function adds a RGB 565 frame around an YCxCx 420 line pack image 
  and output YCxCx 420 line pack in place.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr      pointer to the input image
  input_frame_ptr    pointer to the frame image, rgb565
  loc                where to put frame. Can be NULL
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
  ipl_rect_type*  loc,               /* where to put frame, can be NULL */
  uint16 transparentValue            /* Transparent pixel value        */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_draw_mchan_histogram

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
);





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_draw_histogram

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
);






/* <EJECT> */
/*===========================================================================

FUNCTION ipl_hjr

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
  int strips                        /* should we process in strips */
);


/* <EJECT> */
/*===========================================================================
===========================================================================*/
extern ipl_status_type hjr_correlate_frames
(
  ipl_image_type * in1, 
  ipl_image_type * in2, 
  int searchX, 
  int searchY, 
  int * jitter
);



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
  tY                 luma value of transparent color
  tCb                cb value of transparent color
  tCr                cr value of transparent color
  tTolerance         tolerance for above 3 values. I.e. if tTolerance is
                     10 and tY is 99, then allow lumas of 89 and 109 to
                     also be considered transparent.

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_add_overlay_inplace
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  const ipl_image_type* i_frame_ptr, /* Points to the frame image      */
  ipl_rect_type* loc,                /* where and how much of the frame 2 add*/
  int tY,                            /* Transparent pixel value        */
  int tCb,
  int tCr,
  int tTolerance
);



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
    ipl_image_type * out,
    int hdiff,
    int vdiff,
    int blendWidth,
    int smartSeam
);



#endif  /* IPL_COMPOSE_H */
