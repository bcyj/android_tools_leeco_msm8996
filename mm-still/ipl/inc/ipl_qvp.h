#ifndef _IPL2_API_H_


/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_qvp.h#1 $
===========================================================================*/
#ifndef IPL_DEBUG_STANDALONE

#include "linux/msg.h"
#define IPL2_MSG_LOW(xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
        MSG_3(MSG_SSID_APPS_QVP, MSG_LEGACY_LOW,\
              xx_fmt, xx_arg1, xx_arg2, xx_arg3) 

#define IPL2_MSG_MED(xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
        MSG_3(MSG_SSID_APPS_QVP, MSG_LEGACY_MED,\
              xx_fmt, xx_arg1, xx_arg2, xx_arg3) 

#define IPL2_MSG_HIGH(xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
        MSG_3(MSG_SSID_APPS_QVP, MSG_LEGACY_HIGH,\
              xx_fmt, xx_arg1, xx_arg2, xx_arg3) 

#define IPL2_MSG_ERROR(xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
        MSG_3(MSG_SSID_APPS_QVP, MSG_LEGACY_ERROR,\
              xx_fmt, xx_arg1, xx_arg2, xx_arg3) 

#define IPL2_MSG_FATAL(xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
        MSG_3(MSG_SSID_APPS_QVP, MSG_LEGACY_FATAL,\
              xx_fmt, xx_arg1, xx_arg2, xx_arg3) 

#define IPL2_MSG_STAT_LOW(xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
		MSG_3(MSG_SSID_APPS_IPL2_STATISTICS, MSG_LEGACY_LOW,\
			  xx_fmt, xx_arg1, xx_arg2, xx_arg3) 

#define IPL2_MSG_STAT_MED(xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
        MSG_3(MSG_SSID_APPS_IPL2_STATISTICS, MSG_LEGACY_MED,\
		      xx_fmt, xx_arg1, xx_arg2, xx_arg3) 

#define IPL2_MSG_STAT_HIGH(xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
        MSG_3(MSG_SSID_APPS_IPL2_STATISTICS, MSG_LEGACY_HIGH,\
		   	  xx_fmt, xx_arg1, xx_arg2, xx_arg3) 

#else

#if 1
  #define IPL2_MSG_FATAL  printf
  #define IPL2_MSG_HIGH   printf
  #define IPL2_MSG_MED    printf
  #define IPL2_MSG_LOW    printf
  #define IPL2_MSG_ERROR  printf
#else
  #define IPL2_MSG_FATAL  ipl_noop
  #define IPL2_MSG_HIGH   ipl_noop
  #define IPL2_MSG_MED    ipl_noop
  #define IPL2_MSG_LOW    ipl_noop
  #define IPL2_MSG_ERROR  ipl_noop
#endif

#endif



/*============================================================================
        GLOBAL CONSTANTS/MACROS
============================================================================*/
#define IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb, r, out, out2, rTable, gTable, bTable)  \
  (lumaa2) -= (lumaa1); \
  (r) = (lumaa1) + ipl2_CrToRTable[(uint8)(cr)]; \
  if( (r) > 255) \
    (out) = rTable[ 255 ]; \
  else if( (r) > 0) \
    (out) = rTable[ (r) ]; \
  else \
    out = 0; \
  (r) += (lumaa2); \
  if( (r) > 255) \
   (out2) = rTable[ 255 ]; \
  else if ( (r) > 0 ) \
   (out2) = rTable[ (r) ]; \
  else \
    (out2) = 0; \
  (r) = (lumaa1) - ipl2_CbToGTable[(uint8)(cb)]-ipl2_CrToGTable[(uint8)(cr)];  \
  if( (r) > 255) \
    (out) += gTable[ 255 ]; \
  else if ( (r) > 0 ) \
    (out) += (gTable)[(r)]; \
  (r) += (int32) (lumaa2); \
  if( (r) > 255) \
    (out2) += gTable[ 255 ]; \
  else if( (r) > 0) \
    (out2) += (gTable)[(r)]; \
  (r) = (lumaa1) + ipl2_CbToBTable[(cb)]; \
  if( (r) > 255) \
    (out) += bTable[ 255 ]; \
  else if ( (r) > 0 ) \
    out += bTable[r]; \
  r += lumaa2; \
  if( (r) > 255) \
    (out2) += bTable[ 255 ]; \
  else if ( r > 0 ) \
    out2 += bTable[r];


#define IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1, cr, cb, r, out, rTable, gTable, bTable)  \
  (r) = (lumaa1) + ipl2_CrToRTable[(uint8)(cr)]; \
  if( (r) > 255) \
    (out) = rTable[ 255 ]; \
  else if( (r) > 0) \
    (out) = rTable[ (r) ]; \
  else \
    out = 0; \
  (r) = (lumaa1) - ipl2_CbToGTable[(uint8)(cb)]-ipl2_CrToGTable[(uint8)(cr)];  \
  if( (r) > 255) \
   (out) += gTable[ 255 ]; \
  else if ( (r) > 0 ) \
    (out) += (gTable)[(r)]; \
  (r) = (lumaa1) + ipl2_CbToBTable[(cb)]; \
  if( (r) > 255) \
    (out) += bTable[ 255 ]; \
  else if ( (r) > 0 ) \
    out += bTable[r];

/*----------------------------------------------------------------------------
  Packs 8 bit RGB values into one uint16 formed in RGB565 format
----------------------------------------------------------------------------*/
#define IPL2_PACK_RGB565( r, g, b ) r5xx[(r)] + gx6x[(g)] + bxx5[(b)];


/*===========================================================================

            USEFUL COLOR CONVERSION MACROS

===========================================================================*/
#define IPL2_CONVERT_YCBCR420TORGB_WORD( lumaa1, lumaa2, lumaa3, lumaa4, cb, cr,  r, out, out2, rTable, gTable, bTable ) \
  (lumaa4) -= (lumaa3); \
  (lumaa3) -= (lumaa2); \
  (lumaa2) -= (lumaa1); \
 (r) = (lumaa1) + ipl2_CrToRTable[(cr)]; \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out) = (rTable)[ r ]; \
  } \
  else \
  { \
    (out) = 0; \
  } \
  (r) += (lumaa2); \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out) += (rTable)[ (r) ] << 16; \
  } \
  (r) += (lumaa3); \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out2) = (rTable)[ (r) ]; \
  } \
  else \
  { \
    (out2) = 0; \
  } \
  (r) += (lumaa4); \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out2) += (rTable)[ (r) ] << 16; \
  } \
  (r) = (lumaa1) - ipl2_CbToGTable[(cb)] - ipl2_CrToGTable[(cr)]; \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out) += (gTable)[(r)]; \
  } \
  (r) += (lumaa2); \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out) += (gTable)[(r)] << 16; \
  } \
  (r) += (lumaa3); \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out2) += (gTable)[(r)]; \
  } \
  (r) += (lumaa4); \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out2) += (gTable)[(r)] << 16; \
  } \
  (r) = (lumaa1) + ipl2_CbToBTable[(cb)]; \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out) += (bTable)[(r)]; \
  } \
  (r) += (lumaa2); \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out) += (bTable)[(r)] << 16; \
  } \
  (r) += (lumaa3); \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out2) += (bTable)[(r)]; \
  } \
  (r) += (lumaa4); \
  if ( (r) > 255 ) r = 255;\
  if ( (r) > 0 ) \
  { \
    (out2) += (bTable)[(r)] << 16; \
  }

#define IPL2_CONVERT_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, lumaa4, cb, cr,  r, out, out2, out3, out4,  rTable, gTable, bTable ) \
  (lumaa4) -= (lumaa3); \
  (lumaa3) -= (lumaa2); \
  (lumaa2) -= (lumaa1); \
  (r) = (lumaa1) + ipl2_CrToRTable[(cr)]; \
  if ( (r) > 255 ) \
    out = rTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out = rTable[ (r) ]; \
  } \
  else \
  { \
    out = 0; \
  } \
  (r) += (lumaa2); \
  if ( (r) > 255 ) \
    out2 = rTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out2 = rTable[ (r) ]; \
  } \
  else \
  { \
    out2 = 0; \
  } \
  (r) += (lumaa3); \
  if ( (r) > 255 ) \
    out3 = rTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out3 = rTable[ (r) ]; \
  } \
  else \
  { \
    out3 = 0; \
  } \
  (r) += (lumaa4); \
  if ( (r) > 255 ) \
    out4 = rTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out4 = rTable[ (r) ]; \
  } \
  else \
  { \
    out4 = 0; \
  } \
  (r) = (lumaa1) - ipl2_CbToGTable[(cb)] - ipl2_CrToGTable[(cr)]; \
  if ( (r) > 255 ) \
    out += gTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out += gTable[(r)]; \
  } \
  (r) += (lumaa2); \
  if ( (r) > 255 ) \
    out2 += gTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out2 += gTable[(r)]; \
  } \
  (r) += (lumaa3); \
  if ( (r) > 255 ) \
    out3 += gTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out3 += gTable[(r)]; \
  } \
  (r) += (lumaa4); \
  if ( (r) > 255 ) \
    out4 += gTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out4 += gTable[(r)]; \
  } \
  (r) = (lumaa1) + ipl2_CbToBTable[(cb)]; \
  if ( (r) > 255 ) \
    out += bTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out += bTable[(r)]; \
  } \
  (r) += (lumaa2); \
  if ( (r) > 255 ) \
    out2 += bTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out2 += bTable[(r)]; \
  } \
  (r) += (lumaa3); \
  if ( (r) > 255 ) \
    out3 += bTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out3 += bTable[(r)]; \
  } \
  (r) += (lumaa4); \
  if ( (r) > 255 ) \
    out4 += bTable[ 255 ]; \
  else if ( (r) > 0 ) \
  { \
    out4 += bTable[(r)]; \
  }





/* <EJECT> */
/*===========================================================================
                        DATA DECLARATIONS
===========================================================================*/


#define IPL2_MAX_UPSIZE 2

/*--------------------------------------------------------------------------
    Following structure holds paramters needed for the downsize. Typically
    used in conjunction with the API ipl2_downsize()
--------------------------------------------------------------------------*/
typedef struct
{

  ipl_rect_type             crop; /* the dimensions for cropping
                                   *  and relocation
                                   */

} ipl2_image_downsize_param_type;

/*--------------------------------------------------------------------------
    Following structure holds paramters needed for the upsize. Typically
    used in conjunction with the API ipl2_upsize()
--------------------------------------------------------------------------*/
typedef struct
{

  ipl_rect_type             crop; /* the dimensions for cropping
                                   *  and relocation
                                   */
  ipl_quality_type  qual; /*
                                   *  Image quality of the upsapled image
                                   *  higher the quality higher
                                   *  CPU
                                   */

} ipl2_image_upsize_param_type;

/*--------------------------------------------------------------------------
    Paramter structure which will be used as an argument for the resize
    routines.
--------------------------------------------------------------------------*/
typedef struct
{

  ipl_rect_type             crop;     /* the dimensions for cropping
                                       *  and relocation
                                       */
  ipl_quality_type  qual;     /*
                                       *  Image quality of the upsapled image
                                       *  higher the quality higher
                                       *  CPU
                                       */
  ipl_image_type*            fillFrame;/* to fill the frame when
                                        * images with aspect ratio
                                        * confilcts are resized. This
                                        * will for the unrendeed  part
                                        */

  uint16                     fillPixel;/* fill pixel for centring the image
                                        *
                                        */

} ipl2_image_resize_param_type;

typedef uint8 neighbor_map_type[IPL2_MAX_UPSIZE];

typedef struct
{

  ipl2_image_upsize_param_type   param;

  /*----- used internally -----------------*/
  neighbor_map_type                 *row_map;
  neighbor_map_type                 *col_map;
  uint32                            dx_max;
  uint32                            dy_max;

} ipl2_upsSize_context_type;


/*--------------------------------------------------------------------------
    TYPEDEF IPL2_ROT_PARAMS
    Carries the rotation related parameters for the image
--------------------------------------------------------------------------*/
typedef struct
{
  int32 startPos; /* start taking pixels from this position on  input
                   * image */
  int32 rowInc;   /* increments rows by this number */
  int32 colInc;   /* increment coloumns by this number */
} ipl2_rot_param_type;




/*===========================================================================
===========================================================================*/
extern int32 ipl2_roundFloatVal 
( 
  float valToRound
);


/*===========================================================================

FUNCTION IPL2_INIT()

DESCRIPTION
This function will create intermediate values which are used in YCB to RGB
565 conversion. The functions are self explanatory and the initiliazation is
is to populate the values before its used.

DEPENDENCIES
None

ARGUMENTS IN
None

RETURN VALUE
Status - this not used right now. But it is kept to account for possible
errors when we move this to heap.

SIDE EFFECTS
None

===========================================================================*/
extern ipl_status_type   ipl2_init
(
  void
);




/*===========================================================================
                          API INTERFACE
===========================================================================*/

/* <EJECT> */
/*===========================================================================
                        QVP's FASTER INTERFACE
===========================================================================*/


/*===========================================================================

FUNCTION ipl_xform_RotateAddCrop2

DESCRIPTION

  INITIALIZATION

  This module requires some of the conversion tables initialized before
  triggering this function. It is suggested that the user should call the
  API ipl2_init_lookupTables before calling this API.

 FUNCTIONALITY

  This function is the entry point to a whole variety of display image
  manipulations like color conversion, cropping, framing, rotation and a
  limited overlay of icons. (Read more to know why limited).

  The frame should be in RGB 565 and should be same dimensions  i/p.
  The input can be in YCbCr or RGB565.
  The output can be either YCbCr or RGBxxx where xxx is 565 or 444 or 666.
  |<----- i/p dx ---------------->     |<---------- frame.dx ------------->
  _______________________________      ___________________________________
  |                              |     |                                  |
  |                              |     |                                  |
  |ip.dy                         |     | frame.dy                         |
  |                              |<1:1>|                                  |
  |                              |     |                                  |
  |______________________________|     |__________________________________|

        i/p image                          Frame i/p

   The frame should have a one to one pixel to pixel mapping against
   the input image.

  input image should contain the number of byte's required to present  the
  image. For eg: if your imput is 565 then this routine assumes the image
  pointer inside having dx*dy * 2 bytes. All containing valid pixels.

  The i_frame_ptr is not mandatory. If you are not using an overlay image
  frame, then this image frame should be NULL. The expected behavior is if
  the input image pixel value does not match with the transparency value
  provided then that pixel is copied into the corresponding output pixel
  after doing suitable color conversion/rotation.

  o_img_ptr should be adequately allocated and this is where the output is
  written into.

  LIMITATIONS TO INPUT BUFFER POINTERS
  All the image buffers are assumed to be word aligned. This assumption is
  very critical for faster memory access.

  Crop parameter should be supplied all the time. The crop.x and y specifies
  the position (co ordinate where the image is rendered in the output image
  frame.
        o_img_ptr.dx
  -------------------------------------
  |       |                           |
  |       | (crop.y)                  |
  |       |     (crop.dx)             |
  |--------|----------------------|   |
  |(crop.x)|                      |   |
  |        |                      |   |
  |        |                      |   | (o_img_ptr.dy)
  |        |(crop.dy)             |   |
  |        |                      |   |
  |        |                      |   |
  |        ------------------------   |
  |------------------------------------

  LIMITATIONS TO CROP PARAMETERS.
    Current implementation for YCbCr442 based images only support even number
    of columns are supported. So if the col.dx is 7 as passed value the
    actual crop operation will truncate it to nearest lower even no. In this
    case it is 6. This is a tradeoff for a faster conversion.

    Current implementation for YCCbCr420 based images only even number of
    columns and rows are supported. So if the col.dx is 7 as passed value
    the actual crop operation will truncate it to nearest lower even no. In
    this case it is 6. This is a tradeoff for a faster conversion. Same
    applies with rows as well. This can be enhanced for odd no of rows
    in some cases.

    Whatever the image format is the crop.x which is the X relocation factor
    cannot be a odd number. This is because the IPL writes to the display
    buffer 2 bytes at a time and if the offset is odd boundary we will get
    unexpected results.


  Figure which shows the lay out of cropped image in out put frame.
  Remember the rest of the portion of the output image is left alone.
      From this figure obviously we cant have crop.dx > o_im_ptr.dx and
      same thing applies for dy.


  parameter rotate will specify the rotation you need with respect to the
  input image. This is a clock wise rotation.

  icon_list_ptr - is an optional parameter. If you are not using this
  feature pass a NULL in. If you are using the feature pass a list
  of icons in the pointer. The last icon should be always NULL which tells
  the rendering routine to stop rendering.

  Transparent value - This variable has effect only when the i_frame_ptr is
  valid. For more see the description of i_frame_ptr parameter.


  SPEED OF EXECUTION

  The functionality of the library is divided in such a way that the least
  complex takes least time. For instance if the arguments does not
  specify a crop parameter then the pixel manipulation routines which
  handles the input goes through a different faster code path. In general
  it is advised that the user use the least amount of variance to get the
  display done.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr points to the input image
  i_frame_ptr points to the frame image
  o_img_ptr points to the output image
  crop is a structure informing ipl how to crop
  rotate is the rotation to do
  icon_list_ptr is a ptr to a NULL terminated list of icons
    Icons are only processed if input is IPL_RGB565 and output is IPL_RGB565
  transparentValue is the 16 bit transparent pixel value

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS ipl_rot_add_crop
  None

===========================================================================*/
#define ipl_xform_RotateAddCrop2(a,b,c,d,e,f,g) ipl2_rot_add_crop(a,b,c,d,e,f,g)




/*===========================================================================

FUNCTION ipl_xform_Downsize2

DESCRIPTION
    This API will take an input image of any size and then dwonsample the
    image to create a smaller image in the o/p buffer. For YCbCr flavours
    this API will do color conversion also.

    ASPECT RATIO :
      IT IS BETTER TO downsize by an integer value to get best aspect ratio
      preservaton. If the downsize factor happens to be a fraction but to
      the same value for row and col we can expect an aspect ration preserved
      image most time. Some time the shift math will lose precision and will
      have some fish eye.

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr       - Input image pointet
    i_param         - Not always needed. If non-NULL this will contain
                      the crop param while the o/p image pointer will
                      contain the place where the image is rendered.

ARGUMENTS OUT
   o_img_ptr        - o/P image
RETURN VALUE
Status  - Success or failure

SIDE EFFECTS
None

===========================================================================*/
#define ipl_xform_Downsize2(a,b,c)           ipl2_downsize(a,b,c)





/*===========================================================================

FUNCTION ipl_xform_Upsize2

DESCRIPTION

    This API will take an input image of any QCIF size and upsample
    the image to fill in a QVGA frame oriented in portrait mode. ie : 240
    X 320. This API can be used for phones which has a QCIF in panorama mode.
    Also for QCIF displays which will allow rotation can use the same
    function. That is size of 144 X 176 can also be used as input.

    Quality settings and CPU usage.

      The input parameter to this function has a field to specify the
      desired quality. The more the quality is specified the more the CPU
      usage. Applications/platforms can pick different upsize quality
      depending on the CPU budget. However given CPU is a non issue always
      pick the highest Quality.


      CASE I


      THE UPSIZE IN IF THE DISPLAY IS NOT ROTATED BY 90/270

                                            o_img_ptr->dx >= i_param->crop.dx
                                   -----------------------------------------
  i_img_ptr->dx = 176              |        crp.dx <= 234                  |
-----------------------------      | |------------------------------------||
|                           |      | |                                    ||
|                           |      | |                                    ||
|                           |      | |                                    ||
| i_img_ptr->dy =144        | <--> | |                                    ||
|                           |      | |i_param->crop.dy <= 192             || o_img_ptr->dy
|                           |      | |                                    || >= i_param->
|                           |      | |                                    ||    crop.dy
|----------------------------      | |                                    ||
                                   | |                                    ||
                                   | |____________________________________||
                                   |----------------------------------------


      CASE II

      THE UPSIZE IN IF THE DISPLAY ROTATED BY 90/270

                                         o_img_ptr->dx >= i_param->crop.dx
                                   -----------------------------------------
  i_img_ptr->dx = 144              |        crp.dx <= 234                  |
-------------------------|         | |------------------------------------||
|                        |         | |                                    ||
|                        |         | |                                    ||
|                        |         | |                                    ||
| i_img_ptr->dy =176     | <-->    | |                                    ||
|                        |         | |i_param->crop.dy <= 192             || o_img_ptr->dy
|                        |         | |                                    || >= i_param->
|                        |         | |                                    ||    crop.dy
|                        |         | |                                    ||
|                        |         | |                                    ||
|------------------------|         | |                                    ||
                                   | |                                    ||
                                   | |____________________________________||
                                   |----------------------------------------


The input can be rotated to 144 X 176 or 176 X 144 the o/p is always
upsapmpled  to the size of 234 X 192 and then cropped to fit the o/p image
and crop input supplied. This can be used for the rotated QCIF displays with
220 X 176 size. This will lose approx 5 pixels on each side - not a
significant  loss by any standard.

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr       - Input image pointet
    i_param         - Upsize related parameters.

ARGUMENTS OUT
   o_img_ptr        - o/P image
RETURN VALUE
Status  - Success or failure

SIDE EFFECTS
None

===========================================================================*/
#define ipl_xform_Upsize2(a,b,c)             ipl2_upsize(a,b,c)





#define ipl_upsize_type ipl2_image_upsize_param_type


#define ipl_xform_Upsize133RotCrop(a,b,c,d)    ipl2_upsize_rot_crop_133(a,b,c,d)
#define ipl_xform_Upsize133xRotCrop(a,b,c,d)   ipl2_upsize_rot_crop_133(a,b,c,d)
#define ipl_xform_Upsize150xRotCrop(a,b,c,d)  ipl2_upsize_rot_crop_150x(a,b,c,d)
#define ipl_xform_Upsize2xRotCrop(a,b,c,d)    ipl2_upsize_rot_crop_2x(a,b,c,d)
#define ipl_xform_Upsize200xRotCrop(a,b,c,d)    ipl2_upsize_rot_crop_2x(a,b,c,d)
#define ipl_xform_Upsize250xRotCrop(a,b,c,d)  ipl2_upsize_rot_crop_250x(a,b,c,d)


#define ipl_xform_Upsize150xCrop(a,b,c) ipl_upSize_150x_RGBToRGB(a,b,c)
#define ipl_xform_Upsize200xCrop(a,b,c) ipl_upSize_200x_RGBToRGB(a,b,c)
#define ipl_xform_Upsize250xCrop(a,b,c) ipl_upSize_250x_RGBToRGB(a,b,c)



#endif 







