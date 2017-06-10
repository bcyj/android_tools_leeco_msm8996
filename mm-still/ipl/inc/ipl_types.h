#ifndef IPL_TYPES_H
#define IPL_TYPES_H


/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_types.h#1 $
===========================================================================*/



/* <EJECT> */
/*===========================================================================
                        DATA DECLARATIONS
===========================================================================*/
//#define API_EXTERN extern               /* functions seen by user */

// Should always be turned off (commented out) on customer build.
// This is used to help IPL developer profile functions on SURF
//#define IPL_DEBUG_PROFILE


#ifdef __cplusplus
 #define API_EXTERN extern "C"  /* functions seen by user */
#else 
 #define API_EXTERN extern  /* functions seen by user */
#endif

//#define TRUE   1                        /* Boolean true value.   */
//#define FALSE  0                        /* Boolean false value.  */
typedef unsigned char      boolean;     /* Boolean value type.   */
typedef unsigned long int  uint32;      /* Unsigned 32 bit value */
typedef unsigned short     uint16;      /* Unsigned 16 bit value */
typedef unsigned char      uint8;       /* Unsigned 8  bit value */
typedef signed long int    int32;       /* Signed 32 bit value   */
typedef signed short       int16;       /* Signed 16 bit value   */
typedef signed char        int8;        /* Signed 8  bit value   */

#define MSG_LOW     printf 
#define MSG_MEDIUM  printf
#define MSG_HIGH    printf
#define MSG_FATAL   printf

// Since some funtions take "long time" to process, we have noticed 
// watch dog timers are running out causing crash. 
// It is better to tell the user that his action cannot be run
// for such a large image. 
//
// Currently, those funtions are ipl_int, ipl_hue, ipl_sat
// on ARM9 and ARM11 processors. OEMs may wnat to increase this
// limit if their processor is faster.

#ifndef FEATURE_IPL_NO_LIMITED_MIPS
  #define IPL_IMAGE_MAX   (800*600)
#else
  // else, our IPL functions can process "any" size image
  #define IPL_IMAGE_MAX   (10000*10000)
#endif

                                      


/*
 * To come up with unique start ID, add following number to
 *
 * ipl_util    1000
 * ipl_compose 1100
 * ipl_convert 1300
 * ipl_xform   1500
 * ipl_efx     1700
 *
 * to the sum of four characters after ipl_<type>_XXXX... where,
 *
 * a b c d e f g h i j k  l   m  n  o  p  q  r  s  t  u  v  w  x  y  z
 * 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 
 *
 * Therefore ipl_efx_adaptiveFilter would have 0700 + 0+3+0+15 for logic
 * analyzer timetest.
 *
 */


/* Usually, this code is returned as a parameter in IPL call back
** functions.
*/
typedef enum
{
  IPL_SUCCESS = 0,      /* IPL Function was successful                     */
  IPL_FAILURE,          /* IPL Function failed                             */
  IPL_NO_MEMORY,        /* IPL Function ran out of memory                  */
  IPL_UNSUPPORTED,      /* IPL Function not supported                      */
  IPL_2CPU_INTENSIVE,   /* IPL Function could not completed in time        */
  IPL_ERR_MAX           /* Maximum number of status/error codes            */
} ipl_status_type;


/* This structure defines the various color types
*/
typedef enum
{
  IPL_YCbCr = 0,        /* YCbCr pixel color format 4:2:2                  */
  IPL_YCbCr420_FRAME_PK,/* YCbCr 4:2:0 Frame Packed Format                 */
  IPL_YCbCr420_LINE_PK, /* YCbCr 4:2:0 Line Packed Format                  */
  IPL_YCbCr420_MB_PK,   /* YCbCr 4:2:0 Line Packed Format                  */
  IPL_RGB565,           /* RGB 565 color format                            */
  IPL_RGB888,           /* RGB 888 color format                            */
  IPL_BAYER_GBRG,       /* Mega Pixel GBRG format                          */
  IPL_BAYER_BGGR,       /* Mega Pixel BGGR format                          */
  IPL_BAYER_GRBG,       /* Mega Pixel GRBG format                          */
  IPL_BAYER_RGGB,       /* Mega Pixel RGGB format                          */
  IPL_RGB666,           /* RGB 666 format                                  */
  IPL_RGB444,           /* RGB 444 format                                  */
  IPL_YCbCr422_LINE_PK, /* YCbCr 4:2:2 Line Packed Format                  */
  IPL_YCbCr444_LINE_PK, /* YCbCr 4:4:4 Line Packed Format                  */
  IPL_YCrCb420_LINE_PK, /* YCrCb 4:2:0 Line Packed Format                  */
  IPL_YCrCb422_LINE_PK, /* YCrCb 4:2:2 Line Packed Format                  */
  IPL_YCrCb444_LINE_PK, /* YCrCb 4:4:4 Line Packed Format                  */
  IPL_YCbCr444,         /* YCbCr 4:4:4                                     */
  IPL_YCrCb420_MB_PK,   /* YCrCb 4:2:0 Macro Block                         */
  IPL_YCbCr422_MB_PK,   /* YCrCb 4:2:0 Macro Block                         */
  IPL_YCrCb422_MB_PK,   /* YCrCb 4:2:0 Macro Block                         */
  IPL_YCrCb420_FRAME_PK,/* YCrCb 4:2:0 Frame Packed Format                 */
  IPL_H1V1MCU_CbCr,     /* H1V1 MCU data, usually from JPEG decoder        */
  IPL_H1V2MCU_CbCr,     /* H1V2 MCU data, usually from JPEG decoder        */
  IPL_H2V1MCU_CbCr,     /* H2V1 MCU data, usually from JPEG decoder        */
  IPL_H2V2MCU_CbCr,     /* H1V2 MCU data, usually from JPEG decoder        */
  IPL_MCU_GRAY,         /* MCU data, but only y since gray scale           */
  IPL_YCbCr444_PAD,     /* 0YCbCr data (32 bit word, padded on high order) */
  IPL_RGB888_PAD,       /* 0RGB data (32 bit word, padded on high order)   */
  IPL_LUMA_ONLY,        /* Just Y (luma) data                              */
  IPL_ALPHA,            /* Just 8bit alpha channel                         */
  IPL_HSV,              /* Hue saturation value format                     */
  IPL_COL_MAX           /* Maximum Number of color formats                 */
} ipl_col_for_type;


/*
** This enum defines the various rotation modes
*/
typedef enum
{
  IPL_NOROT,           /* Dont rotate                                     */
  IPL_ROT90,           /* rotate 90 deg clockwise                         */
  IPL_ROT180,          /* rotate 180 deg clockwise                        */
  IPL_ROT270,          /* rotate 270 deg clockwise                        */

  IPL_NOROT_XREF,      /* rot 0 reflect about x axis  */
  IPL_NOROT_YREF,      /* rot 0 reflect about y axis  */
  IPL_ROT90_XREF,      /* rot 90 reflect about x axis  */
  IPL_ROT90_YREF,      /* rot 90 reflect about y axis  */
  IPL_ROT180_XREF,     /* rot 180 reflect about x axis */
  IPL_ROT180_YREF,     /* rot 180 reflect about y axis */
  IPL_ROT270_XREF,     /* rot 270 reflect about x axis */
  IPL_ROT270_YREF,     /* rot 270 reflect about y axis */

  IPL_ROT_MAX          /* max number of rotation values                  */
} ipl_rotate90_type;

/*
** This enum defines the various quality types 
*/
typedef enum
{
  IPL_QUALITY_LOW,            /* fastest code, lowest quality */
  IPL_QUALITY_MEDIUM,         /* fast code, good quality      */
  IPL_QUALITY_HIGH,           /* slow code, great quality     */
  IPL_QUALITY_MAX,            /* slowest code, but highest quality */
  IPL_QUALITY_OFF             /* do nothing */
} ipl_quality_type;


/* This enum defines the various reflection modes
*/
typedef enum
{
  IPL_NOREF,           /* Dont reflect                                     */
  IPL_XREF,            /* reflect about x axis                             */
  IPL_YREF,            /* reflect about y axis                             */
  IPL_REF_MAX           /* max number of reflection values                 */
} ipl_reflect_type;

/* This enum defines the various filters supported in ipl
*/
typedef enum
{
  IPL_BLUR=0,          /* Low Pass Filter                 */
  IPL_SHARP,           /* High Pass Filter                */
  IPL_BLUR_GAUSSIAN,   /* Gaussian Filter                 */
  IPL_BLUR_UNIFORM,    /* Uniform Filter                  */
  IPL_BLUR_MEDIAN,     /* Applies a median filter         */
  IPL_ADAPTIVE,        /* Sharpend and blurs based on edge */
  IPL_FILTER_MAX       /* Max num of filtering modes      */
} ipl_filter_type;


/* This enum defines the various ways of combining two images
*/
typedef enum
{
  IPL_BLEND=0,          /* blend two images */
  IPL_BLEND_BLACK,      /* blend an image with black */
  IPL_BLEND_WHITE,      /* blend an image with white */
  IPL_CENTER,           /* put one image in the center of another */
  IPL_FILMSTRIP,        /* combine two images as if on a moving film strip */
  IPL_HORIZONTAL,       /* add one image to the left/right side of another */
  IPL_VERTICAL,         /* add one image on top of or below another */
  IPL_OVERLAP,          /* overlap one image on part of another */
  IPL_SUBTRACT_LUMA,    /* subtract luma of two images */
  IPL_SUBTRACT_YCBCR,   /* subtract y from y, cb-cb, cr-cr */
  IPL_BLEND_PER_PIXEL,  /* per pixel alpha blending */
  IPL_BLEND_COLOR,      /* blend an image with white */
} ipl_compose_type;


/* This structure defines the various equalizations supported
*/
typedef enum
{
  IPL_GHE = 0,           /* Global Histogram Equalization                  */
  IPL_AHE,               /* Adaptive Histogram Equalization                */
  IPL_INT_EQ_MAX         /* Max num of equalization  modes                 */
} ipl_int_eq_type;





/* This structure defines the format of an image
*/
typedef struct ipl_image_struct 
{
  uint32  dx;                /* Number of pixels in the x dirctn or in a row*/
  uint32  dy;                /* Number of pixels in the y dirctn or in a col*/
  ipl_col_for_type cFormat;  /* Color Format for image                      */

  unsigned char* imgPtr;     /* Pointer to the image data                   */

#if defined(FEATURE_NATIVELINUX)
  uint32 imgPtr_id;          /* image data memory region id for linux build */
#endif /* FEATURE_NATIVELINUX */

  uint8* clrPtr;             /* Pointer to the Color data                   */
} ipl_image_type;


/*
** This structure defines the cropping parameters
*/
typedef struct ipl_crop_struct 
{
  uint32 x;             /* x pos of rectangle                              */
  uint32 y;             /* y pos of rectangle                              */
  uint32 dx;            /* dx of rectangle                                 */
  uint32 dy;            /* dy of rectangle                                 */
} ipl_rect_type;


/*
** This structure defines the parameters for a circle:
**    (x, y) = x- and y-coordinates of the center of the circle,
**    r = radius of the circle.
** The coordinate system is defined to have an origin 
** at the upper left corner of the image, the x-axis 
** extending to the right, and the y-axis extending 
** downward. This structure is used by ipl_magnify.
*/
typedef struct ipl_circle_struct 
{
  uint32 x;         /* x-coordinate of the center */
  uint32 y;         /* y-coordinate of the center */
  uint32 r;         /* radius                     */
} ipl_circle_type;



/*
** This structure defines a straight line:
**    (x1, y1) and (x2, y2) are the coordinates of two
**    points on the line.
** The coordinate system is defined with an origin 
** at the upper left corner of the image, an x-axis 
** extending to the right, and a y-axis extending 
** downward.
*/
typedef struct ipl_line_struct 
{
  uint32 x1;         /* x-coordinate of first point  */
  uint32 y1;         /* y-coordinate of first point  */
  uint32 x2;         /* x-coordinate of second point */
  uint32 y2;         /* y-coordinate of second point */
} ipl_line_type;



/*
** This structure is a 2 dimention grid of points which make a grid
** used for warping. Warping algorithm takes a base mesh (usually evenly
** spaced) and a destination mesh and warp accordingly.
*/
#define IPL_MAX_GRID  11
typedef struct ipl_mesh
{
  uint32 cvx[IPL_MAX_GRID];
  uint32 cvy[IPL_MAX_GRID];
} ipl_grid;



/*
** This structure holds our convultion kernel.
*/
#define IPL_MAX_KERNEL 25
typedef struct ipl_kernel
{
  int k[IPL_MAX_KERNEL];
  uint32 size;
} ipl_kernel_type;


/*
** This structure is for Bayer Image Processing
*/
typedef struct ipl_bip_struct 
{
  uint16 rGain;
  uint16 gGain;
  uint16 bGain;
  int16  *rgbToYcbcr;      /* by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33  */
                           /* (12 elements in total)                          */
  int16  *colorCorrection; /* a11 a12 a13 a21 a22 a23 a31 a32 a33 r g b       */
                           /* (12 elements in total)                          */
  uint16  *gammaTable;     /* Lookup Table Ptr 256 elements                   */
} ipl_bip_type;


/*
** This structure is used for adaptive image processing
*/
typedef struct ipl_image_stats_struct 
{
  uint8 redAverage;     /* Red Average                                     */
  uint8 greenAverage;   /* Green Average                                   */
  uint8 blueAverage;    /* Blue Average                                    */
} ipl_image_stats_type;


/*
** This structure defines an icon
*/
typedef struct ipl_icon_struct 
{
  uint32   x;                /* x-pos of icon                                */
  uint32   y;                /* y-pos of icon                                */
  uint32  dx;                /* Number of pixels in the x dirctn or in a row */
  uint32  dy;                /* Number of pixels in the y dirctn or in a col */
  ipl_col_for_type cFormat;  /* Color Format for image                       */
  uint8* imgPtr;             /* Pointer to the image data                    */
#if defined(FEATURE_NATIVELINUX) && \
    defined(FEATURE_PMEM_V2)
  uint32 imgPtr_id;          /* image data memory region id for linux build  */
#endif
  int32  pitch;              /* Number of bytes per row in icon              */
} ipl_icon_type;



/* This structure defines the various channel types
*/
typedef enum
{
  IPL_CHANNEL_LUMA,
  IPL_CHANNEL_Cb,
  IPL_CHANNEL_Cr,
  IPL_CHANNEL_RED,
  IPL_CHANNEL_GREEN,
  IPL_CHANNEL_BLUE,
  IPL_CHANNEL_ALPHA,
  IPL_CHANNEL_YRGB          // this tells calc_histogram, to compute histogram
                            // for luma, red, green and blue
} ipl_channel_type;



/*
** This structure is used for adaptive image processing
*/
#define IPL_MAX_HIST      256
#define IPL_MAX_HIST_CHAN 4

typedef struct ipl_histogram_type
{
  uint16 size;              // number of elements per channel in histogram

  ipl_channel_type channel; // histogram holds which types of colors

  uint32 active[IPL_MAX_HIST_CHAN]; // backward compatibility

  uint32 v[IPL_MAX_HIST];           // single channel histogram (kept for 
                                    // where histogram is kept
  uint32 mv[1][1];                  // backward compatibility 
} ipl_histogram_type;

typedef struct ipl_mchan_histogram_type
{
  uint16 size;              // number of elements per channel in histogram

  ipl_channel_type channel; // histogram holds which types of colors

  uint32 active[IPL_MAX_HIST_CHAN]; // when drawing, which histogram is valid

  uint32 mv[IPL_MAX_HIST][IPL_MAX_HIST_CHAN]; // new, multi-channel histogram
} ipl_mchan_histogram_type;

/*
** define a curve
*/
#define IPL_MAX_CURVE_POINTS  256
typedef struct ipl_curve_type
{
  uint16 size;
  int32 v[IPL_MAX_CURVE_POINTS];
} ipl_curve_type;
#endif /* IPL_TYPES_H */
