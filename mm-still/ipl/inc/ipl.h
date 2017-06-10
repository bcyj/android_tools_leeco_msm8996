#ifndef IPL_H
#define IPL_H


/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl.h#1 $
===========================================================================*/







/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------
 
                              DATA TYPES

                   e.g.: what is ipl_image_type, or ipl_rect_type, etc
                   file: ipl_types.h

  -------------------------------------------------------------------------
  -------------------------------------------------------------------------*/
#include "ipl_types.h"      /* data types */


//
// #define API_EXTERN extern               /* functions seen by user */
//
//
// /* Usually, this code is returned as a parameter in IPL call back
// ** functions.
// */
// typedef enum
// {
//   IPL_SUCCESS = 0,      /* IPL Function was successful                     */
//   IPL_FAILURE,          /* IPL Function failed                             */
//   IPL_NO_MEMORY,        /* IPL Function ran out of memory                  */
//   IPL_UNSUPPORTED,      /* IPL Function not supported                      */
//   IPL_ERR_MAX           /* Maximum number of status/error codes            */
// } ipl_status_type;
//
//
// /* This structure defines the various color types
// */
// typedef enum
// {
//   IPL_YCbCr = 0,        /* YCbCr pixel color format 4:2:2                  */
//   IPL_YCbCr420_FRAME_PK,/* YCbCr 4:2:0 Frame Packed Format                 */
//   IPL_YCbCr420_LINE_PK, /* YCbCr 4:2:0 Line Packed Format                  */
//   IPL_YCbCr420_MB_PK,   /* YCbCr 4:2:0 Line Packed Format                  */
//   IPL_RGB565,           /* RGB 565 color format                            */
//   IPL_RGB888,           /* RGB 888 color format                            */
//   IPL_BAYER_GBRG,       /* Mega Pixel GBRG format                          */
//   IPL_BAYER_BGGR,       /* Mega Pixel BGGR format                          */
//   IPL_BAYER_GRBG,       /* Mega Pixel GRBG format                          */
//   IPL_BAYER_RGGB,       /* Mega Pixel RGGB format                          */
//   IPL_RGB666,           /* RGB 666 format                                  */
//   IPL_RGB444,           /* RGB 444 format                                  */
//   IPL_YCbCr422_LINE_PK, /* YCbCr 4:2:2 Line Packed Format                  */
//   IPL_YCbCr444_LINE_PK, /* YCbCr 4:4:4 Line Packed Format                  */
//   IPL_YCrCb420_LINE_PK, /* YCrCb 4:2:0 Line Packed Format                  */
//   IPL_YCrCb422_LINE_PK, /* YCrCb 4:2:2 Line Packed Format                  */
//   IPL_YCrCb444_LINE_PK, /* YCrCb 4:4:4 Line Packed Format                  */
//   IPL_YCbCr444,         /* YCbCr 4:4:4                                     */
//   IPL_YCrCb420_MB_PK,   /* YCrCb 4:2:0 Macro Block                         */
//   IPL_YCbCr422_MB_PK,   /* YCrCb 4:2:0 Macro Block                         */
//   IPL_YCrCb422_MB_PK,   /* YCrCb 4:2:0 Macro Block                         */
//   IPL_YCrCb420_FRAME_PK,/* YCrCb 4:2:0 Frame Packed Format                 */
//   IPL_H1V1MCU_CbCr,     /* H1V1 MCU data, usually from JPEG decoder        */
//   IPL_H1V2MCU_CbCr,     /* H1V2 MCU data, usually from JPEG decoder        */
//   IPL_H2V1MCU_CbCr,     /* H2V1 MCU data, usually from JPEG decoder        */
//   IPL_H2V2MCU_CbCr,     /* H1V2 MCU data, usually from JPEG decoder        */
//   IPL_MCU_GRAY,         /* MCU data, but only y since gray scale           */
//   IPL_YCbCr444_PAD,     /* 0YCbCr data (32 bit word, padded on high order) */
//   IPL_RGB888_PAD,       /* 0RGB data (32 bit word, padded on high order)   */
//   IPL_LUMA_ONLY,        /* Just Y (luma) data                              */
//   IPL_ALPHA,            /* Just 8bit alpha channel                         */
//   IPL_HSV,              /* Hue saturation value format                     */
//   IPL_COL_MAX           /* Maximum Number of color formats                 */
// } ipl_col_for_type;
//
//
// /*
// ** This enum defines the various rotation modes
// */
// typedef enum
// {
//   IPL_NOROT,           /* Dont rotate                                     */
//   IPL_ROT90,           /* rotate 90 deg clockwise                         */
//   IPL_ROT180,          /* rotate 180 deg clockwise                        */
//   IPL_ROT270,          /* rotate 270 deg clockwise                        */
//
//   IPL_NOROT_XREF,      /* rot 0 reflect about x axis  */
//   IPL_NOROT_YREF,      /* rot 0 reflect about y axis  */
//   IPL_ROT90_XREF,      /* rot 90 reflect about x axis  */
//   IPL_ROT90_YREF,      /* rot 90 reflect about y axis  */
//   IPL_ROT180_XREF,     /* rot 180 reflect about x axis */
//   IPL_ROT180_YREF,     /* rot 180 reflect about y axis */
//   IPL_ROT270_XREF,     /* rot 270 reflect about x axis */
//   IPL_ROT270_YREF,     /* rot 270 reflect about y axis */
//
//   IPL_ROT_MAX          /* max number of rotation values                  */
// } ipl_rotate90_type;
//
// /*
// ** This enum defines the various quality types
// */
// typedef enum
// {
//   IPL_QUALITY_LOW,            /* fastest code, lowest quality */
//   IPL_QUALITY_MEDIUM,         /* fast code, good quality      */
//   IPL_QUALITY_HIGH,           /* slow code, great quality     */
//   IPL_QUALITY_MAX             /* slowest code, but highest quality */
// } ipl_quality_type;
//
//
// /* This enum defines the various reflection modes
// */
// typedef enum
// {
//   IPL_NOREF,           /* Dont reflect                                     */
//   IPL_XREF,            /* reflect about x axis                             */
//   IPL_YREF,            /* reflect about y axis                             */
//   IPL_REF_MAX           /* max number of reflection values                 */
// } ipl_reflect_type;
//
//
//
// /* This enum defines the various filters supported in ipl
// */
// typedef enum
// {
//   IPL_BLUR=0,          /* Low Pass Filter                 */
//   IPL_SHARP,           /* High Pass Filter                */
//   IPL_BLUR_GAUSSIAN,   /* Gaussian Filter                 */
//   IPL_BLUR_UNIFORM,    /* Uniform Filter                  */
//   IPL_BLUR_MEDIAN,     /* Applies a median filter         */
//   IPL_ADAPTIVE,        /* Sharpend and blurs based on edge */
//   IPL_FILTER_MAX       /* Max num of filtering modes      */
// } ipl_filter_type;
//
//
// /* This enum defines the various ways of combining two images
// */
// typedef enum
// {
//   IPL_BLEND=0,          /* blend two images */
//   IPL_BLEND_BLACK,      /* blend an image with black */
//   IPL_BLEND_WHITE,      /* blend an image with white */
//   IPL_CENTER,           /* put one image in the center of another */
//   IPL_FILMSTRIP,        /* combine two images as if on a moving film strip */
//   IPL_HORIZONTAL,       /* add one image to the left/right side of another */
//   IPL_VERTICAL,         /* add one image on top of or below another */
//   IPL_OVERLAP,          /* overlap one image on part of another */
//   IPL_SUBTRACT_LUMA,    /* subtract luma of two images */
//   IPL_SUBTRACT_YCBCR,   /* subtract y from y, cb-cb, cr-cr */
//   IPL_BLEND_PER_PIXEL,  /* per pixel alpha blending */
//   IPL_BLEND_COLOR,      /* blend an image with white */
// } ipl_compose_type;
//
//
// /* This structure defines the various equalizations supported
// */
// typedef enum
// {
//   IPL_GHE = 0,           /* Global Histogram Equalization                  */
//   IPL_AHE,               /* Adaptive Histogram Equalization                */
//   IPL_INT_EQ_MAX         /* Max num of equalization  modes                 */
// } ipl_int_eq_type;
//
//
//
//
//
// /* This structure defines the format of an image
// */
// typedef struct ipl_image_struct
// {
//   uint32  dx;               /* Number of pixels in the x dirctn or in a row*/
//   uint32  dy;               /* Number of pixels in the y dirctn or in a col*/
//   ipl_col_for_type cFormat; /* Color Format for image                      */
//   unsigned char* imgPtr;    /* Pointer to the image data                   */
//   uint8* clrPtr;            /* Pointer to the Color data                   */
// } ipl_image_type;
//
//
// /*
// ** This structure defines the cropping parameters
// */
// typedef struct ipl_crop_struct
// {
//   uint32 x;             /* x pos of rectangle                              */
//   uint32 y;             /* y pos of rectangle                              */
//   uint32 dx;            /* dx of rectangle                                 */
//   uint32 dy;            /* dy of rectangle                                 */
// } ipl_rect_type;
//
//
// /*
// ** This structure defines the parameters for a circle:
// **    (x, y) = x- and y-coordinates of the center of the circle,
// **    r = radius of the circle.
// ** The coordinate system is defined to have an origin
// ** at the upper left corner of the image, the x-axis
// ** extending to the right, and the y-axis extending
// ** downward. This structure is used by ipl_magnify.
// */
// typedef struct ipl_circle_struct
// {
//   uint32 x;         /* x-coordinate of the center */
//   uint32 y;         /* y-coordinate of the center */
//   uint32 r;         /* radius                     */
// } ipl_circle_type;
//
//
//
// /*
// ** This structure defines a straight line:
// **    (x1, y1) and (x2, y2) are the coordinates of two
// **    points on the line.
// ** The coordinate system is defined with an origin
// ** at the upper left corner of the image, an x-axis
// ** extending to the right, and a y-axis extending
// ** downward.
// */
// typedef struct ipl_line_struct
// {
//   uint32 x1;         /* x-coordinate of first point  */
//   uint32 y1;         /* y-coordinate of first point  */
//   uint32 x2;         /* x-coordinate of second point */
//   uint32 y2;         /* y-coordinate of second point */
// } ipl_line_type;
//
//
//
// /*
// ** This structure is a 2 dimention grid of points which make a grid
// ** used for warping. Warping algorithm takes a base mesh (usually evenly
// ** spaced) and a destination mesh and warp accordingly.
// */
// #define IPL_MAX_GRID  100
// typedef struct ipl_mesh
// {
//   uint32 cvx[IPL_MAX_GRID];
//   uint32 cvy[IPL_MAX_GRID];
// } ipl_grid;
//
//
//
// /*
// ** This structure holds our convultion kernel.
// */
// #define IPL_MAX_KERNEL 100
// typedef struct ipl_kernel
// {
//   int k[IPL_MAX_KERNEL];
//   uint32 size;
// } ipl_kernel_type;
//
//
// /*
// ** This structure is for Bayer Image Processing
// */
// typedef struct ipl_bip_struct
// {
//   uint16 rGain;
//   uint16 gGain;
//   uint16 bGain;
//   int16  *rgbToYcbcr;  /* by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33  */
//                        /* (12 elements in total)                          */
//   int16  *colorCorrection; 
//                        /* a11 a12 a13 a21 a22 a23 a31 a32 a33 r g b       */
//                        /* (12 elements in total)                          */
//   uint16  *gammaTable; /* Lookup Table Ptr 256 elements                   */
// } ipl_bip_type;
//
//
// /*
// ** This structure is used for adaptive image processing
// */
// typedef struct ipl_image_stats_struct
// {
//   uint8 redAverage;     /* Red Average                                     */
//   uint8 greenAverage;   /* Green Average                                   */
//   uint8 blueAverage;    /* Blue Average                                    */
// } ipl_image_stats_type;
//
//
// /*
// ** This structure defines an icon
// */
// typedef struct ipl_icon_struct
// {
//   uint32   x;              /* x-pos of icon                                */
//   uint32   y;              /* y-pos of icon                                */
//   uint32  dx;              /* Number of pixels in the x dirctn or in a row */
//   uint32  dy;              /* Number of pixels in the y dirctn or in a col */
//   ipl_col_for_type cFormat;/* Color Format for image                       */
//   uint8* imgPtr;           /* Pointer to the image data                    */
//   int32  pitch;            /* Number of bytes per row in icon              */
// } ipl_icon_type;
//
//
//
// /* This structure defines the various channel types
// */
// typedef enum
// {
//   IPL_CHANNEL_LUMA,
//   IPL_CHANNEL_Cb,
//   IPL_CHANNEL_Cr,
//   IPL_CHANNEL_RED,
//   IPL_CHANNEL_GREEN,
//   IPL_CHANNEL_BLUE,
//   IPL_CHANNEL_ALPHA
// } ipl_channel_type;
//
//
//
// /*
// ** This structure is used for adaptive image processing
// */
// #define IPL_MAX_HIST 256
// typedef struct ipl_histogram_type
// {
//   uint16 size;
//   ipl_channel_type channel;
//   uint32 v[IPL_MAX_HIST];
// } ipl_histogram_type;
//
//
//
//
// /*
// ** define a curve
// */
// #define IPL_MAX_CURVE_POINTS  256
// typedef struct ipl_curve_type
// {
//   uint16 size;
//   int32 v[IPL_MAX_CURVE_POINTS];
// } ipl_curve_type;










/* <EJECT> */
/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------
 
                        IMAGE COMPOSITING FUNCTIONS 

                   e.g. : copy and paste, add frames,... 
                   files: ipl_compose.c, ipl_compose.h 

  -------------------------------------------------------------------------
  -------------------------------------------------------------------------*/
#include "ipl_compose.h"    


// Mapping names created by multiple authors over time to consistent API name
#define ipl_comp_CopyAndPasteBlend(a,b,c,d,e,f) ipl_copy_and_paste_blend(a,b,c,d,e,f)
#define ipl_comp_AddFrame(a,b,c,d)        ipl_image_add(a,b,c,d)
#define ipl_comp_AddFrameInPlace(a,b,c,d) ipl_image_add_inplace(a,b,c,d)
#define ipl_comp_AddImage(a,b,c,d,e)      ipl_compose(a,b,c,d,e)
#define ipl_comp_CopyAndPaste(a,b,c,d)    ipl_copy_and_paste(a,b,c,d)
#define ipl_comp_CreateFrame(a,b,c)       ipl_frame_compose(a,b,c)
#define ipl_comp_CutAndPaste(a,b,c,d,e)   ipl_cut_and_paste(a,b,c,d,e)
#define ipl_comp_DrawBox(a,b,c,d,e)       ipl_draw_box(a,b,c,d,e)
#define ipl_comp_HandJitterReduction(a,b,c,d,e) ipl_shjr(a,b,c,d,e)
#define ipl_comp_DrawHistogram(a,b,c,d,e,f,g)  ipl_draw_histogram(a,b,c,d,e,f,g)
#define ipl_comp_DrawHistograms(a,b,c,d,e,f,g) ipl_draw_histogram(a,b,c,d,e,f,g)

#define ipl_comp_DrawMchanHistogram(a,b,c,d,e,f,g,h,i)  ipl_draw_mchan_histogram(a,b,c,d,e,f,g,h,i)
#define ipl_comp_DrawMchanHistograms(a,b,c,d,e,f,g,h,i) ipl_draw_mchan_histogram(a,b,c,d,e,f,g,h,i)
#define ipl_comp_StitchFrames(a,b,c,d,e,f) ipl_stitchFrames(a,b,c,d,e,f)






// /*===========================================================================
//
// FUNCTION ipl_cut_and_paste
//
// DESCRIPTION
//   This function performs cut-and-paste. A region is cut from the input
//   image and pasted onto a region in the output image. Input and output
//   image sizes can be different. If the input "cut" region is strictly
//   larger than the output "paste" region, the input region will be downsized
//   to fit the output region. If the images are RGB565, an input "cut" region
//   can also be upsized if it is strictly smaller than the output "paste"
//   region.
//
//   Input and output images must be have the same color format, which can be
//   RGB565 or YCbCr 4:2:2.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_ptr points to the input image
//   cut_area is the input area to be cut out
//   paste_area is the output area to be pasted onto
//   fillerPixel is the pixel color used to replace the cut area
//
// ARGUMENTS IN/OUT
//   out_img_ptr points to the output image
//   in_img_ptr  points to input image which now has cut region of
//               fillerPixel color
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_cut_and_paste
// (
//   ipl_image_type* in_img_ptr,  /* Points to the input image           */
//   ipl_image_type* out_img_ptr, /* Points to the output image          */
//   ipl_rect_type* cut_area,     /* Input region to be cut out          */
//   ipl_rect_type* paste_area,   /* Output region to be pasted onto     */
//   uint32  fillerPixel          /* color value of pixel to replace cut area */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_copy_and_paste
//
// DESCRIPTION
//   This function performs cut-and-paste. A region is copied from the input
//   image and pasted onto a region in the output image. Input and output
//   image sizes can be different. If the input "copy" region is strictly
//   larger than the output "paste" region, the input region will be downsized
//   to fit the output region. If the images are RGB565, an input "copy" region
//   can also be upsized if it is strictly smaller than the output "paste"
//   region.
//
//   Input and output images must be:
//   both rgb565 or,
//   both YCbCr or,
//   YCxCx42y line pack <---> YCxCx42y line pack (where x is r|b and y is 2|0),
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_ptr points to the input image
//   copy_area is the input area to be copied out
//   paste_area is the output area to be pasted onto
//
// ARGUMENTS IN/OUT
//   out_img_ptr points to the output image
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_copy_and_paste
// (
//   ipl_image_type* in_img_ptr,   /* Points to the input image           */
//   ipl_image_type* out_img_ptr,  /* Points to the output image          */
//   ipl_rect_type* copy_area,      /* Input region to be copied out          */
//   ipl_rect_type* paste_area     /* Output region to be pasted onto     */
// );
//
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_copy_and_paste_blend
//
// DESCRIPTION
//   This function performs cut-and-paste. A region is copied from the input
//   image and pasted onto a region in the output image. Input and output
//   image sizes can be different. If the input "copy" region is strictly
//   larger than the output "paste" region, the input region will be downsized
//   to fit the output region. If the images are RGB565, an input "copy" region
//   can also be upsized if it is strictly smaller than the output "paste"
//   region.
//
//   Input and output images must be:
//   both rgb565 or,
//   both YCbCr or,
//   YCxCx42y line pack <---> YCxCx42y line pack (where x is r|b and y is 2|0),
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_ptr points to the input image
//   copy_area is the input area to be copied out
//   paste_area is the output area to be pasted onto
//   blend, width of blending
//   whereBlend, which 4 sides to blend
//
// ARGUMENTS IN/OUT
//   out_img_ptr points to the output image
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_copy_and_paste_blend
// (
//    ipl_image_type* in_img_ptr,   /* Points to the input image           */
//    ipl_image_type* out_img_ptr,  /* Points to the output image          */
//    ipl_rect_type* copy_area,     /* Input region to be copy out          */
//    ipl_rect_type* paste_area,    /* Output region to be pasted onto     */
//    uint32 blend,
//    uint32 whereBlend
// );
//
//
// /*==========================================================================
//
// FUNCTION    ipl_alpha_blend_area
//
// DESCRIPTION
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr         points to the input image
//   i_frame_ptr       points to the frame image
//                     reveal
//
// ARGUMENTS IN/OUT
//
//
// RETURN VALUE
//   IPL_SUCCESS       indicates operation was successful
//   IPL_FAILURE       otherwise
//
// SIDE EFFECTS
//   None
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_alpha_blend_area
// (
//   ipl_image_type* i_img_ptr,
//   ipl_image_type* i_frame_ptr,
//   ipl_rect_type*  area,
//   uint32          alpha,
//   uint16          transparentValue
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_compose_image
//
// DESCRIPTION
//   This function combines two images in various ways.
//
//   Input and output images must have the same color format, which can
//   be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in1_img_ptr   pointer to the first input image
//   in2_img_ptr   pointer to the second input image
//   out_img_ptr   pointer to the output image
//   method        method of compositing the two inputs
//   boundary      boundary location
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_compose_image
// (
//   ipl_image_type* in1_ptr,    /* Points to the first input image  */
//   ipl_image_type* in2_ptr,    /* Points to the second input image */
//   ipl_image_type* out_ptr,    /* Points to the output image       */
//   ipl_compose_type method,    /* Method of compositing            */
//   void * misc1,               /* misc input1 */
//   int misc2,               /* misc input2 */
//   int misc3,               /* misc input3 */
//   int misc4                /* misc input4 */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_image_add
//
// DESCRIPTION
//   This function adds a frame around an image and performs color conversion
//   if needed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr      pointer to the input image
//   input_frame_ptr    pointer to the frame image
//   transparentPixel   16-bit transparent pixel value
//   output_img_ptr     pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS        indicates operation was successful
//   IPL_FAILURE        otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_image_add
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   ipl_image_type* i_frame_ptr,       /* Points to the frame image      */
//   uint16 transparentValue,           /* Transparent pixel value        */
//   ipl_image_type* o_img_ptr          /* Points to the output image     */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_frame_compose
//
// DESCRIPTION
//   This function iterates through a list of icons and composes a frame
//   from the list. The list should be terminated by a NULL.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_canvas_ptr    pointer to the canvas on which the icons are composed
//   icon_list_ptr       pointer to a list of icons ptrs terminated by a NULL
//   transparentValue    16-bit transparent pixel value
//
// ARGUMENT OUT
//   output_canvas_ptr   pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS         indicates operation was successful
//   IPL_FAILURE         otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_frame_compose
// (
//   ipl_image_type* input_canvas_ptr,/* Points to input canvas image        */
//   ipl_icon_type** icon_list_ptr,   /* Points to NULL-terminated icon list */
//   uint16 transparentValue,         /* Transparent pixel value             */
//   ipl_image_type* output_canvas_ptr/* Points to output canvas image       */
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_draw_box
//
// DESCRIPTION
//   This function draws/fills a rectangular region of the input image filler
//   Pixel and returns the result in output. If output is NULL, the operation
//   happens on the input.
//
//   Input and output images must have the same color format, which can be
//   RGB565, YCbCr 4:2:2.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_ptr*    pointer to input image
//   out_img_ptr*   pointer to output image,if NULL, operation happens on input.
//   fill_rect      pointer to a structure indicating the fill_rect region
//   fillerPixel    color to fill region with
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_draw_box
// (
//   ipl_image_type* in_img_ptr,    /* Points to the first input image  */
//   ipl_image_type* out_img_ptr,   /* Points to the output image       */
//   ipl_rect_type* fill_rect,      /* Overlap region                   */
//   int fill_too,                  /* should we will it too */
//   uint32 fillerPixel             /* color to fill region with        */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_image_add_bill
//
// DESCRIPTION
//   This function adds a RGB 565 frame around an YCxCx 420 line pack image
//   and output YCxCx 420 line pack in place.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr      pointer to the input image
//   input_frame_ptr    pointer to the frame image, rgb565
//   loc                where to put frame. Can be NULL
//   transparentPixel   16-bit transparent pixel value
//
// RETURN VALUE
//   IPL_SUCCESS        indicates operation was successful
//   IPL_FAILURE        otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_image_add_inplace
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   const ipl_image_type* i_frame_ptr, /* Points to the frame image      */
//   ipl_rect_type*  loc,               /* where to put frame, can be NULL */
//   uint16 transparentValue            /* Transparent pixel value        */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_image_add_histogram
//
// DESCRIPTION
//   This function adds a 2D bar chart over an image.
//
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr      pointer to the input image
//   hist               the histogram to print  (computed with ipl_calc_hist)
//   loc                upper left and hand corner and size of graph.
//                      LEAVE NULL for a nice default location/size in b/r.
//   r,g,b              color of the chart
//
//
// RETURN VALUE
//   IPL_SUCCESS        indicates operation was successful
//   IPL_FAILURE        otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_draw_histogram
// (
//   ipl_image_type*     i_img_ptr,         /* Points to the input image      */
//   ipl_image_type*     o_img_ptr,         /* Points to the output image     */
//   ipl_histogram_type* hist,
//   ipl_rect_type*      loc,
//   uint8               r,
//   uint8               g,
//   uint8               b
// );
//
//
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_hjr
//
// DESCRIPTION
//
//   This function will perform hand jitter reduction on a series of images.
//   Input and output image sizes must be equal and of the color format.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_array   points to an array of input images the first input image
//   oout_img_ptr points to the output image
//   searchX      how much to look in X dim before quitting registration
//   searchY      how much to look in Y dim before quitting registration
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_hjr
// (
//   ipl_image_type** in_img_array,    /* Points to the input imageS */
//   ipl_image_type* out_img,          /* Points to the output image */
//   int searchX,
//   int searchY
// );


//===========================================================================
//
//FUNCTION    ipl_stitchFrames
//
//DESCRIPTION
//
//  This function will take a pointer to an array of input images and x,y
//  offset and keep pasting the next image over the previous until there
//  aren no more images in input array. The output image needs to be big enough
//  to hold all the input imges minus the overlapy widths which will be a
//  blended combination of images 1 and 2 or images 2 and 3. We currenly
//  support only 3 images stitching.
//
//  Input frames, and output images must both be YCbCr 422.
//
//DEPENDENCIES
//  None
//
//ARGUMENTS IN
//  hdiff     horizontal offset, i.e. where should 2nd image be placed over
//            over the 1st.
//
//  vdiff     vertical offset (not currently supported)
//
//  blendWidth How wide should the blend width be once an image is placed
//             over the other one.
//
//  smartSeam should the seam be placed intelligently? (not supported)
//
//ARGUMENTS IN/OUT
//  out     output image
//
//RETURN VALUE
//  IPL_SUCCESS       indicates operation was successful
//  IPL_FAILURE       otherwise
//
//SIDE EFFECTS
//  None
//
//===========================================================================
//API_EXTERN ipl_status_type
//ipl_stitchFrames
//(
//    ipl_image_type **in_img_array,
//    ipl_image_type * out,
//    int hdiff,
//    int vdiff,
//    int blendWidth,
//    int smartSeam
//);






















/* <EJECT> */
 /*-------------------------------------------------------------------------
   -------------------------------------------------------------------------


                         IMAGE COLOR CONVERSION FUNCTIONS

                         e.g. : rgb565 to ycbcr422, etc.)
                         files: ipl_convert.c, ipl_convert.h

   -------------------------------------------------------------------------
   -------------------------------------------------------------------------*/
#include "ipl_convert.h"    /* prototypes for color format funcs */

// Mapping names created by multiple authors over time to consistent API name
#define ipl_conv_Frame(a,b,c)                   ipl_conv_frame(a,b,c)
#define ipl_conv_Image(a,b)                     ipl_convert_image(a,b)
#define ipl_conv_RGB888ToRGB565plt(a,b,c,d,e)   ipl_RGB8882RGB565plt(a,b,c,d,e)
#define ipl_conv_RGBA888ToRGB565plt(a,b,c,d,e)  ipl_RGB8882RGB565plt(a,b,c,d,e)



// /*===========================================================================
//
// FUNCTION ipl_convert_image
//
// DESCRIPTION
//   This function converts between many types of images types
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr   pointer to the input image
//
// ARGUMENTS IN/OUT
//   None
//
// ARGUMENTS OUT
//   output_img_ptr  pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was succesful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// MODIFIED
//   12/19/04  Created
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_convert_image
// (
//   ipl_image_type* input_img_ptr,        /* Points to the input image  */
//   ipl_image_type* output_img_ptr        /* Points to the output image */
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_conv_frame
//
// DESCRIPTION
//   This function will do frame color conversion. It ensures that the
//   transparent pixel value is unique after color conversion.
//
//   Input frame can be RGB565 or YCbCr 4:2:2.
//   Output frame can be RGB565 or YCbCr 4:2:2.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_frame_ptr        pointer to the input frame
//   o_frame_ptr        pointer to the output frame
//   transparentValue   16-bit transparent pixel value
//
// RETURN VALUE
//   IPL_SUCCESS        indicates operation was successful
//   IPL_FAILURE        otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_conv_frame
// (
//   ipl_image_type* i_frame_ptr,      /* Points to the input frame      */
//   ipl_image_type* o_frame_ptr,      /* Points to the output frame     */
//   uint16 transparentValue           /* Transparent pixel value        */
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_RGB8882RGB565plt
//
// DESCRIPTION
//   This function performs color conversion from RGB888 to RGB565 using a
//   pallete to do lookup.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   data_in     input data
//   width       width of input frame
//   height      height of the input frame
//
// ARGUMENTS IN/OUT
//   None
//
// ARGUMENTS OUT
//   data_out    output data
//
// RETURN VALUE
//   None
//
// SIDE EFFECTS
//   None
//
// MODIFIED
//   07/31/02  Created
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_RGB8882RGB565plt
// (
//     uint8* data_in,
//     uint8* palette,
//     uint16* data_out,
//     int16 width,
//     int16 height
// );
//
// /*===========================================================================
//  *
//  * FUNCTION ipl_RGBA8882RGB565plt
//  *
//  * DESCRIPTION
//  *   This function performs color conversion from RGB888 to RGB565 using a
//  *   RGBA pallete to do lookup. This function differs from
//  *   ipl_RGBA8882RGB565plt
//  *   in that the palette is RGBA and different endian order.
//  *
//  * DEPENDENCIES
//  *   None
//  *
//  * ARGUMENTS IN
//  *   data_in     input data
//  *   palette     input RGBA palette
//  *   width       width of input frame
//  *   height      height of the input frame
//  *
//  * ARGUMENTS IN/OUT
//  *   None
//  *
//  * ARGUMENTS OUT
//  *   data_out    output data
//  *
//  * RETURN VALUE
//  *   None
//  *
//  * SIDE EFFECTS
//  *   None
//  *
//  * MODIFIED
//  *  07/31/02  Created
//  *
//  *===========================================================================*/
// API_EXTERN ipl_status_type ipl_RGBA8882RGB565plt
// (
//   uint8* data_in,
//   uint8* palette,
//   uint16* data_out,
//   int16 width,
//   int16 height
// );
//
//
// /*==========================================================================
//
// FUNCTION    ipl_convert_swap_chroma_order
//
// DESCRIPTION
//   This function swaps CrCb to CbCr or from CbCr to CrCb. Supports 4:2:2
//   or 4:2:0 LINE_PK only.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr     points to the input image
//
// RETURN VALUE
//   IPL_SUCCESS       indicates operation was successful
//   IPL_FAILURE       otherwise
//
// SIDE EFFECTS
//   None
//
// MODIFIED
//   11/09/04  Created
//
// ==========================================================================*/
// API_EXTERN ipl_status_type ipl_convert_swap_chroma_order
// (
//   ipl_image_type* input_img_ptr            /* Pointer to the input image  */
// );






/* <EJECT> */
/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------
                        SPECIAL EFFECTS FUNCTIONS
                        e.g. : sepia, warp, neon
                        files: ipl_efx.c, ipl_efx.h
  -------------------------------------------------------------------------
  -------------------------------------------------------------------------*/
#include "ipl_efx.h"        /* prototypes for special effects funcs */


// Mapping names created by multiple authors over time to consistent API name
#define ipl_efx_ColorChange(a,b,c)        ipl_color_change(a,b,c)
#define ipl_efx_Emboss(a,b,c)             ipl_emboss(a,b,c)
#define ipl_efx_Sketch(a,b)               ipl_sketch(a,b)
#define ipl_efx_Brightness(a,b,c,d,e)     ipl_set_brightness(a,b,c,d,e)
#define ipl_efx_Fade(a,b,c,d,e)           ipl_fadein_fadeout(a,b,c,d,e)
#define ipl_efx_Grayscale(a,b)            ipl_image_grayscale(a,b)
#define ipl_efx_Negative(a,b)             ipl_image_negative(a,b)
#define ipl_efx_Posterize(a,b,c)          ipl_image_posterize(a,b,c)
#define ipl_efx_Solarize(a,b,c)           ipl_image_solarize(a,b,c)
#define ipl_efx_SetContrast(a,b,c,d)      ipl_set_contrast_Nentries(a,b,c,d)
//#define ipl_efx_DoContrast(a,b,c)         ipl_DoContrast(a,b,c)
#define ipl_efx_Sepia(a,b)                ipl_image_sepia(a,b)
#define ipl_efx_SmartFlash(a,b)           ipl_smart_flash(a,b)
#define ipl_efx_Convolution(a,b,c,d)      ipl_convolution(a,b,c,d)
#define ipl_efx_Filter(a,b,c,d)           ipl_filter_image(a,b,c,d)


#ifndef FEATURE_IPL_LIGHT
  #define ipl_efx_LumaAdaptation(a,b,c,d)   ipl_luma_adaptation(a,b,c,d)
  #define ipl_efx_Hue(a,b,c)                ipl_hue(a,b,c)
  #define ipl_efx_Saturation(a,b,c)         ipl_sat(a,b,c)
  #define ipl_efx_Intensity(a,b,c)          ipl_int(a,b,c)
  #define ipl_efx_Magnify(a,b,c)            ipl_magnify(a,b,c)
  #define ipl_efx_MotionBlur(a,b,c,d)       ipl_motion_blur(a,b,c,d)
  #define ipl_efx_Neon(a,b)                 ipl_neon(a,b)
  #define ipl_efx_Fog(a,b,c)                ipl_fog(a,b,c)
  #define ipl_efx_RedEyeRemoval(a,b,c,d,e,f) ipl_red_eye(a,b,c,d,e,f)
  #define ipl_efx_Watercolor(a,b,c)         ipl_watercolor(a,b,c)
  #define ipl_efx_RadialBlur(a,b,c)         ipl_radial_blur(a,b,c)
  #define ipl_efx_Warp(a,b,c,d)             ipl_efx_meshWarp(a,b,c,d)


// enable if on "older" build which calls 4 argument whiteboard
//#define FEATURE_WHITEBOARD_V1

#ifdef FEATURE_WHITEBOARD_V1
  #define ipl_efx_Whiteboard(a,b,c,d)       ipl_whiteBoard(a,b,c,d,0,1)
#else
  #define ipl_efx_Whiteboard(a,b,c,d,e,f)   ipl_whiteBoard(a,b,c,d,e,f)
#endif


#endif



#ifdef FEATURE_IPL_FUNC_CORNER_FOLD
   #define ipl_efx_CornerFold(a,b,c,d)       ipl_corner_fold(a,b,c,d)
#endif




// /*===========================================================================
//
// FUNCTION ipl_change_color
//
// DESCRIPTION
//   This function changes the color scheme of an image. There are 5
//   color-change options, each producing a different change in the
//   color scheme. The options are number from 0 through 4.
//
//   Both inputs and output must have the same color format, which
//   can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_ptr      points to the first input image
//   out_img_ptr     points to the output image
//   option          color change option
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was successful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_color_change
// (
//   ipl_image_type* in_img_ptr,    /* Points to the first input image  */
//   ipl_image_type* out_img_ptr,   /* Points to the output image       */
//   uint8 option                   /* Color change option              */
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_emboss
//
// DESCRIPTION
//   This function performs embossing of an image.  If input is YCbCr 4:2:2,
//   the function outputs YCbCr 4:2:2 or RGB565. If input is RGB565, the
//   function outputs RGB565.
//
//   By nature, the mag most right columns and mag bottom rows cannot be 
//   emobossed
//   and are hence, left black.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr      pointer to the input image
//   o_img_ptr      pointer to the output image
//   mag            strengh of operation
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_emboss
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   ipl_image_type* o_img_ptr,         /* Points to the output image     */
//   uint32 mag
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_sketch
//
// DESCRIPTION
//   This function performs sketch effect on an image.  If input is YCbCr 4:2:2,
//   the function outputs YCbCr 4:2:2 or RGB565. If input is RGB565, the
//   function outputs RGB565.
//
//   By nature, the mag most right columns and mag bottom rows cannot be 
//   emobossed and are hence, left black.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr      pointer to the input image
//   o_img_ptr      pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_sketch
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   ipl_image_type* o_img_ptr          /* Points to the output image     */
// );
//
//
//
// /*===========================================================================
//  *
//  * FUNCTION ipl_convolution
//  *
//  * DESCRIPTION
//  *   This function performs a convolution of the input kernel on the input
//  *   and stores the output in the output image. If output image is NULL, the
//  *   proces happens inplace. numCycles notes how many times to repeat the
//  *   process.
//  *
//  * DEPENDENCIES
//  *   None
//  *
//  * ARGUMENTS IN
//  *   i_img_ptr      pointer to the input image
//  *   o_img_ptr      pointer to the output image
//  *   kernel         kernel to be used.
//  *   numCycles      the number of times to run the kernel on the image
//  *
//  * RETURN VALUE
//  *   IPL_SUCCESS    indicates operation was successful
//  *   IPL_FAILURE    otherwise
//  *
//  * SIDE EFFECTS
//  *  None
//  *
//  *=========================================================================*/
// API_EXTERN ipl_status_type ipl_convolution
// (
//   ipl_image_type* input_img_ptr,     /* Points to the input image      */
//   ipl_image_type* o_img_ptr,         /* Points to the output image     */
//   ipl_kernel_type *kernel,           /* kernel to  use  */
//   int numCycles                      /* how many times to run kernel */
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_fadein_fadeout
//
// DESCRIPTION
//   This function performs fade-in/fade-out for a sequence of still images.
//   The user specifies the number of image frames in the sequence and the
//   width and height of each frame.  Fading in is option 0 and fading out is
//   option 1. All images in the sequence must have the same size.  The
//   user provides the still image to fade to/from.
//
//   Input and output images must have the same color format, which can
//   be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in1_img_ptr    pointer to the input image sequence
//   in2_img_ptr    pointer to the still image to fade to/from
//   out_img_ptr    pointer to the output image sequence
//   numFrames      number of frames to use in the sequence
//   in_out         indicator for fade-in (0) or fade-out (1)
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_fadein_fadeout
// (
//   ipl_image_type* in1_img_ptr, /* Points to input image sequence           */
//   ipl_image_type* in2_img_ptr, /* Points to still image to fade to/from    */
//   ipl_image_type* out_img_ptr, /* Points to output image sequence          */
//   uint16 numFrames,            /* Number of image frames in input sequence */
//   uint8 in_out                 /* Indicates fade-in(0) or fade-out(1)      */
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_filter_image
//
// DESCRIPTION
//   This function performs filtering. Two types of filtering are supported,
//   Blur and Sharpen.
//
//   BLUR:
//   There are three types of blurring filters: (1) Gaussian blur,
//   (2) uniform blur, and (3) simple blur. Gaussian and uniform blur support
//   arbitrary sizes for the blurring window. The window size must be an odd,
//   positive integer in the range 3 <= size <= 1/2 * min(width, height).
//   Simple blur supports only a window size of 3.
//
//   Gaussian and uniform blur support inputs that are RGB565, YCbCr 4:2:2,
//   or YCbCr 4:2:0 line packed format. Output images are always in the
//   same format as inputs.
//
//   Simple blur supports inputs that are RGB565 or YCbCr 4:2:2. Outputs
//   can be RGB565 or YCbCr 4:2:2.
//
//   SHARPEN:
//   The sharpen filter uses a window size of 3. Input images must be in
//   YCbCr 4:2:2. Output can be RGB565 or YCbCr 4:2:2.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr      pointer to the input image
//   o_img_ptr      pointer to the output image
//   filter         type of filtering operation
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_filter_image
// (
//   ipl_image_type* i_img_ptr,     /* Points to the input image      */
//   ipl_image_type* o_img_ptr,     /* Points to the output image     */
//   ipl_filter_type filter,        /* Type of filtering operation    */
//   uint32 val                     /* Size of blurring window        */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_image_grayscale
//
// DESCRIPTION
//   This function converts a color YCbCr 4:2:2 or YCbCr 4:2:0 line packed image
//   to grayscale by ignoring the chroma component.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr   pointer to the input image
//   output_img_ptr  pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was successful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_image_grayscale
// (
//   ipl_image_type* input_img_ptr,        /* Points to the input image      */
//   ipl_image_type* output_img_ptr        /* Points to the output image     */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_image_negative
//
// DESCRIPTION
//   This function creates the negative of an image.
//   The input color format must be YCbCr 4:2:2 or YCbCr 4:2:0 line packed.
//   The output color format must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line
//   packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr   pointer to the input image
//   output_img_ptr  pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was successful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_image_negative
// (
//   ipl_image_type* i_img_ptr,        /* Points to the input image      */
//   ipl_image_type* o_img_ptr         /* Points to the output image     */
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_set_brightness
//
// DESCRIPTION
//   This function sets the multipleication coefficients based on
//   the desired brightness level. The brightness is multipleicative.
//   The delta step is 0.25. The dynamic range is a function of
//   max_value - min_value.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   desired_value    desired brightness setting
//   min_value        minimum brightness value
//   max_value        maximum brightness value
//   input_ptr        pointer to the input image
//
// ARGUMENTS OUT
//   output_ptr       pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS      indicates operation was successful
//   IPL_FAILURE      otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_set_brightness
// (
//   int32 desired_value,         /* Desired brightness setting */
//   int32 min_value,             /* Minimum brightness value   */
//   int32 max_value,             /* Maximum brightness value   */
//   uint16 *input_ptr,           /* Points to the input image  */
//   uint16 *output_ptr           /* Points to the output image */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_image_sepia
//
// DESCRIPTION
//   This function converts an image to sepia.
//   The input must be YCbCr 4:2:2 or YCbCr 4:2:0 line packed.
//   The output must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr   pointer to the input image
//   output_img_ptr  pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was successful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_image_sepia
// (
//   ipl_image_type* input_img_ptr,         /* Points to the input image      */
//   ipl_image_type* output_img_ptr         /* Points to the output image     */
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_image_posterize
//
// DESCRIPTION
//   This function posterizes an image. The number of quantization levels must
//   be within the range [1..255].
//   Input must be in YCbCr 4:2:2 or YCbCr 4:2:0 line packed.
//   Output must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr   pointer to the input image
//   output_img_ptr  pointer to the output image
//   steps           number of quantization levels (range is [1..255])
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was successful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_image_posterize
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   ipl_image_type* o_img_ptr,         /* Points to the output image     */
//   uint8 steps                        /* Number of quantization levels  */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_image_solarize
//
// DESCRIPTION
//   This function solarizes an image. The threshold value for solarization
//   must be in the range [0..255].
//   Input must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//   Output must be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr    pointer to the input image
//   output_img_ptr   pointer to the output image
//   thresh           threshold value used for inversion (range is [0..255])
//
// RETURN VALUE
//   IPL_SUCCESS      indicates operation was successful
//   IPL_FAILURE      otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_image_solarize
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   ipl_image_type* o_img_ptr,         /* Points to the output image     */
//   uint8 thresh                       /* Threshold to use for inversion */
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_smart_flash
//
// DESCRIPTION
//   This function performs intensity equalization and color conversion
//   if required.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr     pointer to the input image
//   o_img_ptr     pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_smart_flash
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   ipl_image_type* o_img_ptr          /* Points to the output image     */
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_set_contrast_Nentires
//
// DESCRIPTION
//   This function warps the gamma table for a particular contrast.
//   Always output 8-bit values.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   contrast        index to which gamma table to use
//   ibits           number of bit required to represent number of entires,
//                   i.e. enter 8 for 256 (0-255) entires, 10-bit for 1024
//                   (0 to 1023).
//   input_table     input gamma table
//
// ARGUMENTS OUT
//   output_table    output gamma table
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was successful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_set_contrast_Nentries
// (
//   int32 contrast,                    /* Desired contrast value            */
//   int32 ibits,                       /* number of bits needed for number
//                                         of entries (8,10,...)             */
//   uint8* input_table,               /* Input gamma table                 */
//   uint8* output_table               /* Output gamma table                */
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_hue
//
// DESCRIPTION
//   This function performs hue scaling and color conversion if required.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr      pointer to the input image
//   o_img_ptr      pointer to the output image
//   hue_factor     an additive hue scale factor (range is -360 to 360)
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_hue
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image          */
//   ipl_image_type* o_img_ptr,         /* Points to the output image         */
//   int32 hue_factor                   /* Hue scaling factor                 */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_fog
//
// DESCRIPTION
//   This function creates a foggy image. The user specifies the level of
//   fogginess, either light (0) or heavy (1).
//
//   Input and output images must have the same color format, which can
//   be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_ptr    pointer to the input image
//   out_img_ptr   pointer to the output image
//   level         fog level, either light (0) or heavy (1)
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_fog
// (
//   ipl_image_type* in_img_ptr,    /* Points to the input image   */
//   ipl_image_type* out_img_ptr,   /* Points to the output image  */
//   uint8 level                    /* Level of fogginess          */
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_radial_blur
//
// DESCRIPTION
//   This function performs radial blur. The blurring level can be set
//   to an integer between 0 and 100, inclusive. No blurring occurs
//   when the blurring level is 0.
//
//   Input and output image sizes must be equal.
//
//   Input and output images must have the same color format, which
//   can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr points to the input image
//   o_img_ptr points to the output image
//   level is the level of blurring (range: 0 to 100)
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_radial_blur
// (
//   ipl_image_type* i_img_ptr,   /* Points to the input image           */
//   ipl_image_type* o_img_ptr,   /* Points to the output image          */
//   int32 xo,                    /* x coordinate of center of ratation  */
//   int32 yo,                    /* y coordinate of center of ratation  */
//   uint8 level                  /* Blurring level                      */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_sat
//
// DESCRIPTION
//   This function performs saturation scaling and color conversion if required.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr     pointer to the input image
//   o_img_ptr     pointer to the output image
//   sat_factor    additive saturation scale factor (range is -255 to 255)
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_sat
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image          */
//   ipl_image_type* o_img_ptr,         /* Points to the output image         */
//   int32 sat_factor                   /* Saturation scaling factor          */
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_int
//
// DESCRIPTION
//   This function performs intensity scaling.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr     pointer to the input image
//   o_img_ptr     pointer to the output image
//   int_factor    intensity scale factor (range is -255 to 255)
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_int
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image          */
//   ipl_image_type* o_img_ptr,         /* Points to the output image         */
//   int32 int_factor                   /* Intensity scaling factor           */
// );
//
// /*===========================================================================
//
// FUNCTION ipl_magnify
//
// DESCRIPTION
//   This function magnifies an input image in a circular region of
//   arbitrary location and radius.  The center of the magnified region
//   must be within the input image boundaries. Areas outside the circular
//   region are not magnified. If part of the magnified region extends
//   beyond image boundaries, only the part within boundaries is
//   displayed.
//
//   Input and output image sizes must be equal.
//
//   Input and output images must have the same color format, which
//   can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_ptr     pointer to the input image
//   out_img_ptr    pointer to the output image
//   circle         the circular region of magnification
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_magnify
// (
//   ipl_image_type* in_img_ptr,    /* Points to the input image        */
//   ipl_image_type* out_img_ptr,   /* Points to the output image       */
//   ipl_circle_type* circle        /* Circular region of magnification */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_motion_blur
//
// DESCRIPTION
//   This function produces motion blur in any arbitrary direction. The size
//   of the blurring window must be an odd, positive integer in the range
//   3 <= size <= 1/2 * min(w, h), where w and h are the width and height of
//   the input image. The direction of motion is specified by a positive integer
//   angle measured in degrees from the horizontal.
//
//   Input and output images must have the same size.
//   Input and output images must be in the same color format, which can be
//   RGB565, YCbCr 4:2:2, YCbCr 4:2:0 line packed, or YCrCb 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_ptr    pointer to the input image
//   out_img_ptr   pointer to the output image
//   size          size of blurring window
//   angle         angle of motion blur
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_motion_blur
// (
//   ipl_image_type* in_img_ptr,   /* Points to the input image  */
//   ipl_image_type* out_img_ptr,  /* Points to the output image */
//   uint32 size,                  /* Width of blurring window   */
//   uint16 angle                  /* Angle of motion blur       */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_neon
//
// DESCRIPTION
//   This function creates a neon version of the input image.  Input and
//   output images must be the same size.
//
//   Input and output images must have the same color format, which can
//   be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr     pointer to the input image
//   o_img_ptr     pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_neon
// (
//   ipl_image_type* i_img_ptr,    /* Points to the input image  */
//   ipl_image_type* o_img_ptr     /* Points to the output image */
// );
//
//
//
// /*===========================================================================
//  *
//  * FUNCTION ipl_luma_adaptation
//  *
//  * DESCRIPTION
//  *   This function stretches the histogram of an image to provide more detail
//  *   in the lights and darks.
//  *
//  * DEPENDENCIES
//  *   None
//  *
//  * ARGUMENTS IN
//  *   i_img_ptr      pointer to the input image
//  *   o_img_ptr      pointer to the output image
//  *
//  * RETURN VALUE
//  *   IPL_SUCCESS    indicates operation was successful
//  *   IPL_FAILURE    otherwise
//  *
//  * SIDE EFFECTS
//  *   None
//  *
//  *===========================================================================*/
// API_EXTERN ipl_status_type ipl_luma_adaptation
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   ipl_image_type* o_img_ptr,         /* Points to the output image     */
//   ipl_histogram_type *hist,
//   uint16 thresholdQ8
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_efx_meshWarp
//
// DESCRIPTION
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr   pointer to the input image
//   output_img_ptr  pointer to the output image
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was successful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_efx_meshWarp
// (
//   ipl_image_type* input_img_ptr,         /* Points to the input image      */
//   ipl_image_type* output_img_ptr,        /* Points to the output image     */
//   ipl_grid* dstMesh,
//   boolean opp
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_watercolor
//
// DESCRIPTION
//   This function performs watercolor effect. If input is YCbCr 4:2:2,
//   the function outputs YCbCr 4:2:2.  If input is RGB565, the function
//   outputs RGB565.
//
//   The kernel size is 3x3. This size kernel is not adequate for images
//   having resolution greatern than 500x500 and larger kenrnel should be used.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr      pointer to the input image
//   o_img_ptr      pointer to the output image
//   mag            magnitude of blur
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_watercolor
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   ipl_image_type* o_img_ptr,         /* Points to the output image     */
//   uint32 mag
// );
//
//
//
// /*===========================================================================
//  *
//  * FUNCTION ipl_whiteBoard
//  *
//  * DESCRIPTION
//  *   This function takes an image taken of a white board, or chalk board, and
//  *   cleans it up for easy viewing and printing.
//  *
//  *
//  * DEPENDENCIES
//  *  None
//  *
//  * ARGUMENTS IN
//  *  i_img_ptr      pointer to the input image
//  *  o_img_ptr      pointer to the output image
//  *  blackmode      0 = auto, 1 = do whiteboard, 2 = do blackboard*
//  *  th_noise       noise threshold, 10 (out of 255) is good number
//  *  rm_border      tile size when trying to remove area outside whiteboard
//  *  gray           0 = auto, 1 =  make grayscale, 0 = dont make grayscale
//  *
//  * RETURN VALUE
//  *  IPL_SUCCESS    indicates operation was successful
//  *  IPL_FAILURE    otherwise
//  *
//  * SIDE EFFECTS
//  *  None
//  *
//  *=========================================================================*/
// API_EXTERN ipl_status_type ipl_whiteBoard
// (
//   ipl_image_type * in_img_ptr,
//   ipl_image_type *out_img_ptr,
//   int blackMode,
//   int th_noise,
//   int rm_border,
//   int gray
// );
//
//
//
// /*===========================================================================
// FUNCTION ipl_red_eye
//
// DESCRIPTION
//   This function finds and fixes red eye
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in      pointer to the input image
//   out     pointer to the output image
//   icrip   where to search
//   ocrip   where to put output
//   arg1    future use
//   arg2    future use
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
// =========================================================================== */
// API_EXTERN ipl_status_type ipl_red_eye(ipl_image_type * in,
//                                       ipl_image_type * out,
//                                       ipl_rect_type * icrop,
//                                       ipl_rect_type * ocrop,
//                                       void * arg1,
//                                       void * arg2);


/* <EJECT> */
/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------

                        UTILITY FUNCTIONS
                        e.g. : fix boundary,
                        files: ipl_util.c, ipl_util.h

  -------------------------------------------------------------------------
  -------------------------------------------------------------------------*/
// Mapping names created by multiple authors over time to consistent API name
#include "ipl_util.h"

#define ipl_util_AlignFrame(a,b,c)        ipl_align_frame(a,b,c)
#define ipl_util_BadPixelCorrect(a)       ipl_bad_pixel_correct(a)
#define ipl_util_CalcAvg(a,b)             ipl_calc_avg(a,b)
#define ipl_util_CalcHistograms(a,b,c,d)  ipl_calc_histograms(a,b,c,d)
#define ipl_util_CalcMchanHistograms(a,b,c,d) ipl_calc_mchan_histograms(a,b,c,d)
#define ipl_util_CalcMchanHistogram(a,b,c,d)  ipl_calc_mchan_histograms(a,b,c,d)
#define ipl_util_CalcMedian(a,b)          ipl_calc_median(a,b)
#define ipl_util_CalcPSNR(a,b,c,d)        ipl_fix_boundary(a,b,c,d)
#define ipl_util_FixBoundary(a,b,c)       ipl_fix_boundary(a,b,c)


/*===========================================================================
//
// FUNCTION ipl_fix_boundary
//
// DESCRIPTION
//     This function boundaries between transparent and non-transparent
//     regions in frames in RGB565.  After the fix, pixels along the boundary
//     that were close in color to the transparent pixels are set to be
//     transparent.  This prevents pseudo-color bleeding from the
//     transparent region.
//
//     Both input and output frames must be RGB565.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr            pointer to the input frame
//   o_img_ptr            pointer to the output frame
//   transparentValue     16-bit transparent pixel value
//
// RETURN VALUE
//   IPL_SUCCESS          indicates operation was successful
//   IPL_FAILURE          otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_fix_boundary
// (
//   ipl_image_type* i_img_ptr,        /* Points to the input frame  */
//   ipl_image_type* o_img_ptr,        /* Points to the output frame */
//   uint32 transparentValue           /* Value of transparent pixel */
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_align_frame
//
// DESCRIPTION
//     This function aligns frames in RGB565.
//     After alignment, no transitions between transparent and non-
//     transparent pixel values will occur on odd-numbered pixels.
//     This prevents color bleeding when a YCbCr4:2:2 image is converted
//     to other formats, such as RGB565, and has a frame added.
//     Bleeding occurs because IPL functions that add frames look at
//     pairs of pixels when the input is YCbCr4:2:2 for speed purposes.
//     This causes transparent pixels to be only looked at on even
//     boundaries.  If a particular frame has transparent pixels on an
//     odd boundary, it may cause color bleeding.  To overcome this,
//     we recommend that OEMs call this function once to make sure
//     there are no odd pixel transitions.  Whenever an odd pixel
//     transition is found, the transparent region is contracted.
//     Frames which don't have odd pixel transitions are left
//     untouched.
//
//     Input and output frames must be in RGB565.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr          pointer to the input frame
//   o_img_ptr          pointer to the output frame
//   transparentValue   16-bit transparent pixel value
//
// RETURN VALUE
//   IPL_SUCCESS        indicates operation was successful
//   IPL_FAILURE        otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_align_frame
// (
//   ipl_image_type* i_img_ptr,        /* Points to the input frame  */
//   ipl_image_type* o_img_ptr,        /* Points to the output frame */
//   uint32 transparentValue            /* Value of transparent pixel */
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_bad_pixel_correct
//
// DESCRIPTION
//   This function performs bad pixel correction.
//   The input image is overwritten by the output image.
//   The input must be in either IPL_BAYER_BGGR or IPL_BAYER_RGGB format.
//   The output must be in the same color format as the input.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   img_ptr        pointer to the input and also the output image
//   min_factor     multipleicant of the min pixel value in Q12 Unsigned
//   max_factor     multipleicant of the max pixel value in Q12 Unsigned
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_bad_pixel_correct
// (
//   ipl_image_type* img_ptr          /* Points to the input image           */
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_bad_pixel_correct_5x5
//
// DESCRIPTION
//   This function performs bad pixel correction.
//   The input image is overwritten by the output image.
//   The input must be in either IPL_BAYER_BGGR or IPL_BAYER_RGGB format.
//   The output must be in the same color format as the input.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   img_ptr        pointer to the input and also the output image
//   min_factor     multipleicant of the min pixel value in Q12 Unsigned
//   max_factor     multipleicant of the max pixel value in Q12 Unsigned
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_bad_pixel_correct_5x5
// (
//   ipl_image_type* img_ptr          /* Points to the input image           */
// );
//
//
//
//
//
// /*===========================================================================
//  *
//  * FUNCTION    ipl_calc_minmax
//  *
//  * DESCRIPTION
//  *
//  *   This function computes the min and max for crop area of pixels.
//  *
//  * DEPENDENCIES
//  *   None
//  *
//  * ARGUMENTS IN
//  *   in          input image
//  *   crop        area in question
//  *
//  * ARGUMENTS IN/OUT
//  *
//  *   min Returns min Y,Cb,Cr packed as 24 bits into uint32
//  *   max Returns min Y,Cb,Cr packed as 24 bits into uint32
//  *
//  * RETURN VALUE
//  *
//  *  IPL_SUCCESS          indicates operation was successful
//  *  IPL_FAILURE          otherwise
//  *
//  * SIDE EFFECTS
//  *  None
//  *
//  *=========================================================================*/
// API_EXTERN ipl_status_type ipl_calc_minmax
// (
//   ipl_image_type *in,
//   ipl_rect_type *crop,
//   uint32 * min,
//   uint32 * max
// );
//
//
// /*===========================================================================
//  *
//  * FUNCTION    ipl_calc_avg
//  *
//  * DESCRIPTION
//  *
//  *   This function computes the average for crop area of pixels.
//  *
//  * DEPENDENCIES
//  *   None
//  *
//  * ARGUMENTS IN
//  *   in          input image
//  *   crop        area in question
//  *
//  * ARGUMENTS IN/OUT
//  *
//  *   Returns average Y,Cb,Cr packed as 24 bits into uint32
//  *
//  * RETURN VALUE
//  *
//  *   Returns average Y,Cb,Cr packed as 24 bits into uint32
//  *
//  * SIDE EFFECTS
//  *  None
//  *
//  *=========================================================================*/
// API_EXTERN uint32 ipl_calc_avg
// (
//   ipl_image_type *in,
//   ipl_rect_type *crop
// );
//
//
//
// /*===========================================================================
//  *
//  * FUNCTION    ipl_calc_median
//  *
//  * DESCRIPTION
//  *
//  *   This function computes the median for crop area of pixels.
//  *
//  * DEPENDENCIES
//  *   None
//  *
//  * ARGUMENTS IN
//  *   in          input image
//  *   crop        area in question
//  *
//  * ARGUMENTS IN/OUT
//  *
//  *   Returns median Y,Cb,Cr packed as 24 bits into uint32
//  *
//  * RETURN VALUE
//  *
//  *   Returns median Y,Cb,Cr packed as 24 bits into uint32
//  *
//  * SIDE EFFECTS
//  *  None
//  *
//  *===========================================================================*/
// API_EXTERN uint32 ipl_calc_median
// (
//   ipl_image_type *in,
//   ipl_rect_type *crop
// );
//
//
//
// /*===========================================================================
//  *
//  * FUNCTION    ipl_histogram
//  *
//  * DESCRIPTION
//  *
//  *   This function computes the luma histogram for crop area of pixels.
//  *   It takes in RGB565 or YCbCr
//  *
//  * DEPENDENCIES
//  *   None
//  *
//  * ARGUMENTS IN
//  *   in          input image
//  *   crop        area in question
//  *
//  * ARGUMENTS IN/OUT
//  *   hist        Returns luma histogram
//  *
//  * RETURN VALUE
//  *
//  * SIDE EFFECTS
//  *  None
//  *
//  *=========================================================================*/
// API_EXTERN ipl_status_type ipl_calc_histogram
// (
//   ipl_image_type *in,
//   ipl_rect_type *crop,
//   ipl_histogram_type *hist
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION    ipl_count_membership
//
// DESCRIPTION
//
//   This function computes how many pixels are of a given color.
//
// DEPENDENCIES
//
//   None
//
// ARGUMENTS IN
//
//   in          input image
//   crop        area in question
//   action      what to count
//
// ARGUMENTS IN/OUT
//
//   Returns value in either a1, a2, or a3
//
// RETURN VALUE
//
//   IPL_SUCCESS if all goes well
//   IPL_FAILURE if not all goes well
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_count_members
// (
//   ipl_image_type *in,
//   ipl_rect_type *crop,
//   int action,
//   uint32 * a1n,
//   uint32 * a2,
//   uint32 * a3
// );







/* <EJECT> */
 /*-------------------------------------------------------------------------
   -------------------------------------------------------------------------


                         IMAGE TRANSFORMATION FUNCTIONS

                         e.g. : rotate, upsize, downsize
                         files: ipl_xform.c, ipl_xform.h

   -------------------------------------------------------------------------
   -------------------------------------------------------------------------*/
#include "ipl_xform.h"      /* prototypes for rot/crop/scale funcs */

// Mapping names created by multiple authors over time to consistent API name
#define ipl_xform_CropResizeRot(a,b,c,d,e,f)  ipl_crop_resize_rot(a,b,c,d,e,f)
#define ipl_xform_Downsize(a,b,c)             ipl_downsize(a,b,c)
#define ipl_xform_Perspective(a,b,c,d,e,f)    ipl_perspective(a,b,c,d,e,f)
#define ipl_xform_Reflect(a,b,c)              ipl_reflect(a,b,c)
#define ipl_xform_ResizeFrame(a,b,c,d)        ipl_resizeFrame(a,b,c,d)
#define ipl_xform_Rotate(a,b,c,d)             ipl_rotate(a,b,c,d)
#define ipl_xform_Rotate90Frame(a,b,c,d)      ipl_rotate90_frame(a,b,c,d)
#define ipl_xform_RotateAddCrop(a,b,c,d,e,f,g) ipl_rot_add_crop(a,b,c,d,e,f,g)
#define ipl_xform_CropRotateAddCrop(a,b,c,d,e,f,g,h) ipl_crop_rot_add_crop(a,b,c,d,e,f,g,h)
#define ipl_xform_Upsize(a,b,c,d)             ipl_upsize(a,b,c,d)

#ifndef FEATURE_IPL_LIGHT
  #define ipl_xform_Shear(a,b,c,d,e)            ipl_shear(a,b,c,d,e)
  #define ipl_xform_Pinch(a,b,c)                ipl_pinch(a,b,c)
#endif


// /*===========================================================================
//
// FUNCTION ipl_downsize
//
// DESCRIPTION
//   This function performs resizing and color conversion. The bip_ptr can be
//   set to NULL if no color processing is desired. Demosaicing is still
//   performed when the bip_ptr is set to NULL.
//
//   The input can be YCbCr 4:2:2, RGB565, Bayer, or RGB888.
//   The output can be YCbCr 4:2:2 or RGB565.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr    pointer to the input image
//   output_img_ptr   pointer to the output image
//   bip_ptr          pointer to a BIP structure (for Mega Pixel Color
//                    Processing Support)
//
// RETURN VALUE
//   IPL_SUCCESS      indicates operation was successful
//   IPL_FAILURE      otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_downsize
// (
//   ipl_image_type* i_img_ptr,        /* Points to the input image           */
//   ipl_image_type* o_img_ptr,        /* Points to the output image          */
//   ipl_bip_type*   bip_ptr
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_crop_resize_rot
//
// DESCRIPTION
//   This function performs cropping on input, downsizing/upsizep, and then
//   convestion to rgb
//
//   The input must be YCxCx420 or YCxCx422 line pack;
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr         pointer to the input image
//   o_img_ptr         pointer to the output image
//   cropin            region on input to be cropped
//   cropout           region on output to be cropped
//   rot               0,90,180,270 rotation
//   qual              quality type
//
// RETURN VALUE
//   IPL_SUCCESS       indicates operation was successful
//   IPL_FAILURE       otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_crop_resize_rot
// (
//   ipl_image_type* i_img_ptr,        /* Points to the input image        */
//   ipl_image_type* o_img_ptr,        /* Points to the output image       */
//   ipl_rect_type* cropin,            /* Crop input config                */
//   ipl_rect_type* cropout,           /* Crop output config               */
//   ipl_rotate90_type rot,           /* rotation param */
//   ipl_quality_type qual
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_reflect
//
// DESCRIPTION
//   This function performs reflection and color conversion if needed.
//   Only RGB565 and YCbCr 4:2:2 inputs are supported.
//   If input is RGB565, only RGB565 output is supported.
//   If input is YCbCr 4:2:2, RGB565 and YCbCr 4:2:2 output are supported.
//
//   If the output is NULL, the reflection will happen in place.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr   pointer to the input image
//   output_img_ptr  pointer to the output image
//   reflect         type of reflection
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was successful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_reflect
// (
//   ipl_image_type* i_img_ptr,         /* Points to the input image      */
//   ipl_image_type* o_img_ptr,         /* Points to the output image     */
//   ipl_reflect_type reflect           /* Type of reflection             */
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_rot_add_crop
//
// DESCRIPTION
//   This function performs realtime clockwise rotation and addition of a frame.
//   Four types of rotations are supported: 0 degrees, 90 degrees, 180 degrees,
//   and 270 degrees. Icons are processed only if both input and output are
//   RGB565.
//
//   The frame must be RGB565 and must have the same dimensions as the output.
//   The input can be either YCbCr 4:2:2 or RGB565.
//   The output can be either YCbCr 4:2:2 or RGB565.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr         pointer to the input image
//   i_frame_ptr       pointer to the frame image
//   o_img_ptr         pointer to the output image
//   crop              region to be cropped
//   rotate            type of rotation
//   icon_list_ptr     pointer to a NULL-terminated list of icons
//   transparentValue  16-bit transparent pixel value
//
// RETURN VALUE
//   IPL_SUCCESS       indicates operation was successful
//   IPL_FAILURE       otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_rot_add_crop
// (
//   ipl_image_type* i_img_ptr,        /* Points to the input image        */
//   ipl_image_type* i_frame_ptr,      /* Points to the frame              */
//   ipl_image_type* o_img_ptr,        /* Points to the output image       */
//   ipl_rect_type* crop,              /* Crop config                      */
//   ipl_rotate90_type rotate,         /* Rotatation                       */
//   ipl_icon_type** icon_list_ptr,    /* Ptr to null terminated icon list */
//   uint16 transparentValue           /* Transparent pixel value          */
// );
//
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_rotate90_frame
//
// DESCRIPTION
//   This function performs 90 degree clockwise rotation of frames with special
//   processing for transparent pixels. Four types of rotation are supported:
//   0 degrees, 90 degrees, 180 degrees, and 270 degrees.
//
//   Input and output frame images must be in RGB565.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr          pointer to the input frame image
//   o_img_ptr          pointer to the output frame image
//   rotate             type of rotation to perform
//   transparentValue   16-bit transparent pixel value
//
// RETURN VALUE
//   IPL_SUCCESS        indicates operation was successful
//   IPL_FAILURE        otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_rotate90_frame
// (
//   ipl_image_type* i_img_ptr,        /* Points to the input image      */
//   ipl_image_type* o_img_ptr,        /* Points to the output image     */
//   ipl_rotate90_type rotate,         /* Type of rotation               */
//   uint16 transparentValue           /* Transparent pixel value        */
// );
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_upsize
//
// DESCRIPTION
//   This function upsizes (i.e. enlarges) an image.
//   Input and output must be in RGB565 format.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr   pointer to the input image
//   output_img_ptr  pointer to the output image
//   crop            area in the output to crop
//   quality         quality of output
//
// RETURN VALUE
//   IPL_SUCCESS     indicates operation was successful
//   IPL_FAILURE     otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_upsize
// (
//   ipl_image_type* i_img_ptr,        /* Points to the input image           */
//   ipl_image_type* o_img_ptr,        /* Points to the output image          */
//   ipl_rect_type* crop,              /* area in output to crop              */
//   ipl_quality_type qualty           /* quality parameter */
// );
//
//
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_perspective
//
// DESCRIPTION
//   This function creates a perspective image shaped like a trapezoid.
//   The user specifies 4 input parameters: (1) the length of the top
//   width of the trapezoid, (2) the x-coordinate of the starting point
//   of the top width, (3) a pixel value to fill in blank output regions
//   outside of the trapezoid, and (4) the orientation of the trapezoid.
//   The top width must be at least 1 pixel and no more than the width
//   of the input image. A long top width corresponds to a vanishing
//   point that is far away from the viewer or camera. A short top width
//   indicates a vanishing point near the camera. The filler pixel value
//   must be given in a format compatible with the output color format. For
//   instance, black is 0 for RGB565 and 0x8010 for YCbCr. White is 0xFFFF
//   for RGB565 and 0x80EB for YCbCr. Notice that for YCbCr, the chroma
//   value (Cb or Cr) comes before the luma value, so white is 0x80EB
//   instead of 0xEB80. The trapezoid’s height and bottom width are assumed
//   to be the input height and width for "up" and "down" orientations.
//   For "left" and "right" orientations, the trapezoid's height and bottom
//   width are the input width and height, respectively.
//
//   The orientation of the trapezoid can be one of four values:
//   up (0), down (1), left (2), or right (3), as illustrated below.
//
//   For any orientation, the top width is defined as the smaller
//   of the two parallel edges of the trapezoid.
//
//   Input and output images must have the same color format, which
//   can be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
//   (0) up:
//          ----
//         |    |
//        |      |
//        --------
//   (1) down:
//        --------
//        |      |
//         |    |
//          ----
//   (2) left:
//         __ --- |
//       |        |
//       | __     |
//            --- |
//   (3) right:
//       | --- __
//       |        |
//       |     __ |
//       | ---
//
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   in_img_ptr     pointer to the input image
//   out_img_ptr    pointer to the output image
//   topWidth       top width of the trapezoidal output image
//   xStart         starting x-coordinate of the top width
//   fillerPixel    pixel value for the filler region
//   orientation    orientation of the output image
//
// RETURN VALUE
//   IPL_SUCCESS    indicates operation was successful
//   IPL_FAILURE    otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_perspective
// (
//   ipl_image_type* in_img_ptr,   /* Points to the input image          */
//   ipl_image_type* out_img_ptr,  /* Points to the output image         */
//   uint32 topWidth,              /* Top width of trapezoidal output    */
//   int32 xStart,                 /* Starting x-coordinate of top width */
//   uint16 fillerPixel,           /* Pixel value for filler region      */
//   uint8 orientation             /* Orientation of output image        */
// );
//
//
//
// /*===========================================================================
//
// FUNCTION ipl_rotate
//
// DESCRIPTION
//   This function performs arbitrary rotation. The user must specify
//   the angle of rotation in degrees and the pixel value used to fill
//   in blank regions. Any part of the input that is rotated outside
//   image boundaries is cut off. Only rotations of 0, 90, 180, and 270
//   degrees do not cut off any part of the input image. Negative angles
//   produce clockwise rotation. Positive angles produce counterclockwise
//   rotation. Angles can be any integer between positive and negative
//   infinity.
//
//   Input and output image sizes must be equal.
//
//   Inputs and output must have the same color format, which can
//   be RGB565, YCbCr 4:2:2, or YCbCr 4:2:0 line packed.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   i_img_ptr     pointer to the input image
//   o_img_ptr     pointer to the output image
//   angle         angle of rotation in degrees
//   fillerPixel   filler pixel value
//
// RETURN VALUE
//   IPL_SUCCESS   indicates operation was successful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_rotate
// (
//   ipl_image_type* i_img_ptr,   /* Points to the input image           */
//   ipl_image_type* o_img_ptr,   /* Points to the output image          */
//   int16 angle,                 /* Angle of rotation in degrees        */
//   uint16 fillerPixel
// );
//
//
// /*===========================================================================
//
// FUNCTION ipl_resizeFrame
//
// DESCRIPTION
//   This function is used to do upsize and downsize usin nearest neighbor
//   The input should be RGB565 and the output should be RGB565.
//   It preserves transparency.
//
// DEPENDENCIES
//   None
//
// ARGUMENTS IN
//   input_img_ptr points to the input image
//   output_img_ptr points to the output image
//
// RETURN VALUE
//   IPL_SUCCESS   is operation was succesful
//   IPL_FAILURE   otherwise
//
// SIDE EFFECTS
//   None
//
// ===========================================================================*/
// API_EXTERN ipl_status_type ipl_resizeFrame
// (
//   ipl_image_type* i_img_ptr,        /* Points to the input image           */
//   ipl_image_type* o_img_ptr,        /* Points to the output image          */
//   ipl_rect_type*  cropout,
//   uint16 transparentValue           /* Transparent pixel value          */
// );



/*===========================================================================
                     SUPPORTING INCLUDE FILES
===========================================================================*/
#include "ipl_attic.h"      /* prototypes for old funcs (to be phased out) */
#include "ipl_qvp.h"        /* prototypes for qvp's faster routines */
#include "ipl_redeye.h"     /* prototypes for red eye removal */
#include "ipl_tone.h"       /* prototypes for skin tone detecion */


#endif // #ifndef IPL_H
