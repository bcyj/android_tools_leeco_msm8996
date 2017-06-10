#ifndef IPL_UPIZE_H
#define IPL_UPIZE_H


/*===========================================================================

                        IPL2_UPSIZE . H

GENERAL DESCRIPTION

  This file contains the function handlers to take care of various
  upsizeing/upsampling filters.


EXTERNALIZED FUNCTIONS

  ipl2_upSize2AndCrop_YCbCr2RGB()

    This function takes in a input image an then upsamples the image to an
    abitrarily sized o/p image. The crop parameters controlls the decides
    the crop deimenstiions

  ipl2_upSizeAverageAndCrop_YCbCr2RGB()

    Same functionality as ipl2_upSize2AndCrop_YCbCr2RGB but this function
    employs a slower but more just neighbor hood averaging.

  ipl2_upsizeQCIF_YCbCr420_133()

    This routine roughly upsizes a QCIF frame to QVGA frame in Portrait mode.
    Very useful for phones which has QVGA displays or phones which has QCIF
    LCD with Land Scape mode. This function operates on YUV420 frame.

  ipl2_upsizeQCIF_YCbCr_133()

    This routine roughly upsizes a QCIF frame to QVGA frame in Portrait mode.
    Very useful for phones which has QVGA displays or phones which has QCIF
    LCD with Land Scape mode. This function operates on YCbCr 422 frame.


INITIALIZATION AND SEQUENCING REQUIREMENTS

  The library will function properly only if the ipl2_init()
  function is called before calling anything else.


Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_upSize.h#1 $

                            EDIT HISTORY FOR FILE


when        who    what, where, why
--------    ---    ----------------------------------------------------------
07/19/04   srk    QCIF upscaling with upscale ration 1.33
06/09/04   srk    Linted version
===========================================================================*/





/*--------------------------------------------------------------------------
    Following macro do the conversion of 4 YCBCR pixels  into RGB
    same time saves the intermediate values so that next 4 upsampled
    pixels can run faster. cb and cr over written with b and g factor
    (intermediates) while r_factor is introduced as a new variable
--------------------------------------------------------------------------*/
#define IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  rTable, gTable, bTable ) \
  (r_factor) = ipl2_CrToRTable[(cr)]; \
  (r) = (lumaa1) + r_factor ; \
  if ( (r) > 255 ) \
  { \
    (out) = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out) = rTable[ (r) ]; \
  } \
  else \
  { \
    (out) = 0; \
  } \
  (r) = (lumaa2) + (r_factor); \
  if ( (r) > 255 ) \
  { \
    (out2) = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out2) = rTable[ (r) ]; \
  } \
  else \
  { \
    (out2) = 0; \
  } \
  (r) = (lumaa3) + (r_factor); \
  if ( (r) > 255 ) \
  { \
    (out3) = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out3) = rTable[ (r) ]; \
  } \
  else \
  { \
    (out3) = 0; \
  } \
  (r) = (lumaa4) + (r_factor); \
  if ( (r) > 255 ) \
  { \
    (out4) = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out4) = rTable[ (r) ]; \
  } \
  else \
  { \
    (out4) = 0; \
  } \
  (cr) = - ( ipl2_CbToGTable[(cb)] + ipl2_CrToGTable[(cr)] ); \
  (r) = (lumaa1) + (cr) ; \
  if ( (r) > 255 ) \
  { \
    (out) += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out) += gTable[(r)]; \
  } \
  (r) = (lumaa2) + (cr); \
  if ( (r) > 255 ) \
  { \
    (out2) += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out2) += gTable[(r)]; \
  } \
  (r) = (lumaa3)  + (cr); \
  if ( (r) > 255 ) \
  { \
    (out3) += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out3) += gTable[(r)]; \
  } \
  (r) = (lumaa4) + cr; \
  if ( (r) > 255 ) \
  { \
    (out4) += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out4) += gTable[(r)]; \
  } \
  (cb) = ipl2_CbToBTable[(cb)]; \
  (r) = (lumaa1) + (cb); \
  if ( (r) > 255 ) \
  { \
    (out) += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out) += bTable[(r)]; \
  } \
  (r) = (lumaa2) + cb; \
  if ( (r) > 255 ) \
  { \
    (out2) += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out2) += bTable[(r)]; \
  } \
  (r) = (lumaa3) + (cb); \
  if ( (r) > 255 ) \
  { \
    (out3) += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out3) += bTable[(r)]; \
  } \
  (r) = (lumaa4) + (cb); \
  if ( (r) > 255 ) \
  { \
    (out4) += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out4) += bTable[(r)]; \
  }

/*--------------------------------------------------------------------------
    This macro works in conjunction with the previous this one does color
    conversion using the intermediates left behind by the previous one.
--------------------------------------------------------------------------*/
#define IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  rTable, gTable, bTable ) \
  (r) = (lumaa1) + r_factor ; \
  if ( (r) > 255 ) \
  { \
    (out) = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out) = rTable[ (r) ]; \
  } \
  else \
  { \
    (out) = 0; \
  } \
  (r) = (lumaa2) + (r_factor); \
  if ( (r) > 255 ) \
  { \
    (out2) = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out2) = rTable[ (r) ]; \
  } \
  else \
  { \
    (out2) = 0; \
  } \
  (r) = (lumaa3) + (r_factor); \
  if ( (r) > 255 ) \
  { \
    (out3) = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out3) = rTable[ (r) ]; \
  } \
  else \
  { \
    (out3) = 0; \
  } \
  (r) = (lumaa4) + (r_factor); \
  if ( (r) > 255 ) \
  { \
    (out4) = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out4) = rTable[ (r) ]; \
  } \
  else \
  { \
    (out4) = 0; \
  } \
  (r) = (lumaa1) + (cr) ; \
  if ( (r) > 255 ) \
  { \
    (out) += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out) += gTable[(r)]; \
  } \
  (r) = (lumaa2) + (cr); \
  if ( (r) > 255 ) \
  { \
    (out2) += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out2) += gTable[(r)]; \
  } \
  (r) = (lumaa3)  + (cr); \
  if ( (r) > 255 ) \
  { \
    (out3) += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out3) += gTable[(r)]; \
  } \
  (r) = (lumaa4) + cr; \
  if ( (r) > 255 ) \
  { \
    (out4) += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out4) += gTable[(r)]; \
  } \
  (r) = (lumaa1) + (cb); \
  if ( (r) > 255 ) \
  { \
    (out) += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out) += bTable[(r)]; \
  } \
  (r) = (lumaa2) + cb; \
  if ( (r) > 255 ) \
  { \
    (out2) += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out2) += bTable[(r)]; \
  } \
  (r) = (lumaa3) + (cb); \
  if ( (r) > 255 ) \
  { \
    (out3) += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out3) += bTable[(r)]; \
  } \
  (r) = (lumaa4) + (cb); \
  if ( (r) > 255 ) \
  { \
    (out4) += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    (out4) += bTable[(r)]; \
  }

#define IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1, lumaa_dash, lumaa2, cr_dash, cb_dash,  r, out, out2, out3, rTable, gTable, bTable) \
  (lumaa2) -= (lumaa_dash); \
  (lumaa_dash) -= (lumaa1); \
  (r) = (lumaa1) + ipl2_CrToRTable[(cr)]; \
  if ( (r) > 255 ) \
  { \
    out = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    out = rTable[ (r) ]; \
  } \
  else \
  { \
    out = 0; \
  } \
  (r) += (lumaa_dash); \
  if ( (r) > 255 ) \
  { \
    out2 = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    out2 = rTable[ (r) ]; \
  } \
  else \
  { \
    out2 = 0; \
  } \
  (r) += (lumaa2); \
  if ( (r) > 255 ) \
  { \
    out3 = rTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    out3 = rTable[ (r) ]; \
  } \
  else \
  { \
    out3 = 0; \
  } \
  (r) = (lumaa1) - ipl2_CbToGTable[(cb)] - ipl2_CrToGTable[(cr)]; \
  if ( (r) > 255 ) \
  { \
    out += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    out += gTable[(r)]; \
  } \
  (r) += (lumaa_dash); \
  if ( (r) > 255 ) \
  { \
    out2 += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    out2 += gTable[(r)]; \
  } \
  (r) += (lumaa2); \
  if ( (r) > 255 ) \
  { \
    out3 += gTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    out3 += gTable[(r)]; \
  } \
  (r) = (lumaa1) + ipl2_CbToBTable[(cb)]; \
  if ( (r) > 255 ) \
  { \
    out += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    out += bTable[(r)]; \
  } \
  (r) += (lumaa_dash); \
  if ( (r) > 255 ) \
  { \
    out2 += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    out2 += bTable[(r)]; \
  } \
  (r) += (lumaa2); \
  if ( (r) > 255 ) \
  { \
    out3 += bTable[ (255) ]; \
  } \
  else if ( (r) > 0 ) \
  { \
    out3 += bTable[(r)]; \
  }
#define IPL2_QCIF_COL_CNTLS_BY_3 58  /* the 176 /3 takes to 58 */
#define IPL2_QCIF_COL_CNTPT_BY_3 48  /* the 144 /3 takes to 48 */
#define IPL2_QCIF_WIDTH        176 /* width of the standard QCIF frame */
#define IPL2_QCIF_HEIGHT       144 /* height od the QCIF exact frame */









/*===========================================================================

FUNCTION IPL2_UPSIZE()

DESCRIPTION
    This API will take an input image of any size and then upsample the
    image to create a bigger image in the o/p buffer. For YCbCr flavours
    this API will do color conversion also. The API will decide on the
    cropping and upsize ration so that the lease of information is lost in
    the process.


DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr       - Input image pointet
    crop            - Crop value required for the o/p dimensions

ARGUMENTS OUT
   o_img_ptr        - o/P image

RETURN VALUE
Status  - Success or failure

SIDE EFFECTS
None

LIMITATIONS:
  The routine only supports an RGB flavor as o/p.
===========================================================================*/
API_EXTERN ipl_status_type ipl2_upsize
(
  ipl_image_type*                   i_img_ptr,
  ipl_image_type*                   o_img_ptr,
  ipl2_image_upsize_param_type*  i_param
);


/*===========================================================================

FUNCTION IPL2_HANDLE_UPSIZE()

DESCRIPTION
    Function which handles the upsizing of an existing existing image and
    positioning it into an o/p buffer

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image

ARGUMENTS IN
    o_img_ptr - o/p image


RETURN VALUE
Success or Failure

SIDE EFFECTS
None

===========================================================================*/
extern ipl_status_type ipl2_handle_upSize
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);





/*===========================================================================

FUNCTION  ipl2_upsizeQCIF_YCbCr420_133()

DESCRIPTION

    This API will take an input image of any QCIF size and upsample
    the image to fill in a QVGA frame oriented in portrait mode. ie : 240
    X 320. This API can be used for phones which has a QCIF in panorama mode.
    Also for QCIF displays which will allow rotation can use the same
    function. That is size of 144 X 176 can also be used as input.

    The input must be YCbCr 420 always

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

IMPORTANT NOTE ON UPSIZE RATIO.

    This function only does the upsize ratio of 1.33. This is done in
    purpose for CPU advantage. When the routine becomes generic there is
    more work per pixel and it reflects badly on CPU.

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
extern ipl_status_type ipl2_upsizeQCIF_YCbCr420_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upsizeQCIF_YCrCb420lp_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);



extern ipl_status_type ipl2_upsize_QCIF_133
(
  ipl_image_type*                 i_img_ptr,  /* input image   */
  ipl_image_type*                 o_img_ptr,  /*  output image */
  ipl2_image_upsize_param_type *i_param    /* upsize param  */
);


/*===========================================================================

FUNCTION  ipl2_upsizeQCIF_YCbCr_133()

DESCRIPTION

    This API will take an input image of any QCIF size and upsample
    the image to fill in a QVGA frame oriented in portrait mode. ie : 240
    X 320. This API can be used for phones which has a QCIF in panorama mode.
    Also for QCIF displays which will allow rotation can use the same
    function. That is size of 144 X 176 can also be used as input.

    The input must be YCbCr 422 always

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

IMPORTANT NOTE ON UPSIZE RATIO.

    This function only does the upsize ratio of 1.33. This is done in
    purpose for CPU advantage. When the routine becomes generic there is
    more work per pixel and it reflects badly on CPU.

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
extern ipl_status_type ipl2_upsizeQCIF_YCbCr_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);



/* TO BE CODED FULLY */
#if 0
extern ipl_status_type ipl2_create_NeghborTable
(
  ipl_rect_type *in_size,
  ipl_rect_type *out_size,
  ipl2_upsSize_context_type *context
);
#endif


/*=========================================================================

                    extern FORWARD DECLARATIONS

===========================================================================*/
#if 0
extern ipl_status_type ipl2_upsize_rgb
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );
#endif

extern ipl_status_type ipl2_upsize_YCbCr442
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

extern ipl_status_type ipl2_upSizeAndCrop_YCbCr2RGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

extern ipl_status_type ipl2_upSizeRowAvgAndCrop_YCbCr2RGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

extern  ipl_status_type ipl2_upsize_YCbCr420
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

extern  ipl_status_type ipl2_upsize_YCrCb420lp
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

extern ipl_status_type ipl2_upSizeAndCrop_YCbCr420ToRGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

extern ipl_status_type ipl2_upSizeAndCrop_YCrCb420lpToRGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

extern ipl_status_type ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

extern ipl_status_type ipl2_upSizeRowAvgAndCrop_YCrCb420lpToRGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );

#if 0
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr420ToRGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  );
#endif

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133_PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133_PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133_PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);

extern ipl_status_type ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133_PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);






/*===========================================================================

FUNCTION ipl2_upsize_rot_crop_2.5x

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize an
  image 2x.  This function will also rotate the image.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.
  rot               - Parameter relating to rotation


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
API_EXTERN ipl_status_type ipl2_upsize_rot_crop_250x
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param,    /* processing param */
  ipl_rotate90_type rot
);

/*===========================================================================

FUNCTION ipl2_upsize_rot_crop_1.5x

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize an
  image 2x.  This function will also rotate the image.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.
  rot               - Parameter relating to rotation


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
API_EXTERN ipl_status_type ipl2_upsize_rot_crop_150x
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param,    /* processing param */
  ipl_rotate90_type rot
);




/*===========================================================================

FUNCTION ipl2_upsize_rot_crop_2x

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize an
  image 2x.  This function will also rotate the image.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.
  rot               - Parameter relating to rotation


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
API_EXTERN ipl_status_type ipl2_upsize_rot_crop_2x
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param,    /* processing param */
  ipl_rotate90_type rot
);




/*===========================================================================

FUNCTION ipl2_upsize_rot_crop_133

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize an
  image 133x.  This function will also rotate the image.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.
  rot               - Parameter relating to rotation


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
API_EXTERN ipl_status_type ipl2_upsize_rot_crop_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param,    /* processing param */
  ipl_rotate90_type rot
);





/*===========================================================================
 *
 * FUNCTION ipl_bilinUpsize_Rot_Crop_YCbCr420ToRGB_Med
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
extern ipl_status_type
ipl_bilinUpsize_Rot_Crop_YCbCr420ToRGB_Med
(
   ipl_image_type*        i_img_ptr, /* Points to the input image   */
   ipl_image_type*        o_img_ptr, /* Points to the output image  */
   ipl_rect_type*         cropout,
   ipl_rotate90_type      rot,
   ipl_quality_type       qual
);




/*===========================================================================

FUNCTION ipl2_upSize_250x_RGBToRGB

DESCRIPTION
  This function uses RGB by 2.5 times.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr  - input image frame
  i_param        - image params

ARGUMENTS OUT
  output_img_ptr - Points to the output image

RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
API_EXTERN ipl_status_type ipl_upSize_250x_RGBToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);




/*===========================================================================

FUNCTION ipl2_upSize_150x_RGBToRGB

DESCRIPTION
  This function upsize and RGB image 1.5 times.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr  - input image frame
  i_param        -  image params

ARGUMENTS OUT
  output_img_ptr - Points to the output image

RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
API_EXTERN ipl_status_type ipl_upSize_150x_RGBToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);



/*===========================================================================

FUNCTION ipl2_upSize_133x_RGBToRGB

DESCRIPTION
  This function upsize and RGB image 1.3 times.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr  - input image frame
  i_param        -  image params

ARGUMENTS OUT
  output_img_ptr - Points to the output image

RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
API_EXTERN ipl_status_type ipl_upSize_133x_RGBToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);





/*===========================================================================

FUNCTION ipl2_upSize_200x_RGBToRGB

DESCRIPTION
  This function upsizes an RGB image 2x.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr  - input image frame
  i_param        -  image params

ARGUMENTS OUT
  output_img_ptr - Points to the output image

RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
API_EXTERN ipl_status_type ipl_upSize_200x_RGBToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);




/*===========================================================================
 *
 * FUNCTION IPL2_UPSIZEANDCROP_YCBCR()
 *
 * DESCRIPTION
 *   This function uses nearest neighboor to do upsizing.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   input_img_ptr  - Input image frame
 *   output_img_ptr - Points to the output image
 *   crop           - Crop config
 *
 *
 * RETURN VALUE
 *   Status - success or failure
 *
 * SIDE EFFECTS
 *  Changes the output image buffer
 *
 *===========================================================================*/
API_EXTERN ipl_status_type ipl2_upSizeAndCrop_YCbCr
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);



/*===========================================================================
 *
 * FUNCTION ipl2_upSize2x_CropInOut_YCrCb420lpToRGB
 *
 * DESCRIPTION
 *   This function uses pixel dumping to do upsize a YCbCr420 line pack
 *   based image to
 *   RGB image - larger in size.  The image is multiplied by 2 in dimensions
 *   (both Widh and height). If the Crop params are less than the actual
 *   mulitiplied size there will be cropping. This is a combined
 *   routine and
 *   hence will have more CPU optimized performance.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   input_img_ptr  - input image frame
 *   cropin         - input crop
 *   cropout        - output crop
 *
 * ARGUMENTS OUT
 *   output_img_ptr - Points to the output image
 *
 * RETURN VALUE
 *   Status - success or failure
 *
 * SIDE EFFECTS
 *   Changes the output image buffer
 *
 *===========================================================================*/
extern ipl_status_type ipl2_upSize2x_CropInOut_YCrCb420lpToRGB
(
  ipl_image_type* i_img_ptr, /* Points to the input image   */
  ipl_image_type* o_img_ptr, /* Points to the output image  */
  ipl_rect_type * cropin,
  ipl_rect_type * cropout
);


/*===========================================================================
 *
 * FUNCTION ipl2_upSize2x_CropInOut_YCbCr420lpToRGB
 *
 * DESCRIPTION
 *   This function uses pixel dumping to do upsize a YCbCr420 line pack
 *   based image to
 *   RGB image - larger in size.  The image is multiplied by 2 in dimensions
 *   (both Widh and height). If the Crop params are less than the actual
 *   mulitiplied size there will be cropping. This is a combined
 *   routine and
 *   hence will have more CPU optimized performance.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   input_img_ptr  - input image frame
 *   cropin         - input crop
 *   cropout        - output crop
 *
 * ARGUMENTS OUT
 *   output_img_ptr - Points to the output image
 *
 * RETURN VALUE
 *   Status - success or failure
 *
 * SIDE EFFECTS
 *   Changes the output image buffer
 *
 *===========================================================================*/
extern ipl_status_type ipl2_upSize2x_CropInOut_YCbCr420lpToRGB
(
  ipl_image_type* i_img_ptr, /* Points to the input image   */
  ipl_image_type* o_img_ptr, /* Points to the output image  */
  ipl_rect_type * cropin,
  ipl_rect_type * cropout
);






/*===========================================================================
 *
 * FUNCTION ipl2_upSize_qcif2qvga_RGBToRGB
 *
 * DESCRIPTION
 *   This function upsize and RGB image 1.8 times in x-dim.
 *   This function upsize and RGB image 1.6 times in y-dim.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   input_img_ptr  - input image frame
 *   i_param        -  image params
 *
 * ARGUMENTS OUT
 *   output_img_ptr - Points to the output image
 *
 * RETURN VALUE
 *   Status - success or failure
 *
 * SIDE EFFECTS
 *   Changes the output image buffer
 *
 *===========================================================================*/
extern ipl_status_type ipl_upSize_qcif2qvga_RGBToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);



/*===========================================================================

FUNCTION ipl2_upSize_arb_low_RGBToRGB

DESCRIPTION
  Fast arbitrary upsize

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr - input image frame
  o_img_ptr - input image frame
  icrop     - input crop
  ocrop     - output crop

ARGUMENTS OUT
  o_img_ptr - input image frame

RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl_upSize_arb_low_RGBToRGB
(
  ipl_image_type *i_img_ptr, /* Points to the input image   */
  ipl_image_type *o_img_ptr, /* Points to the output image  */
  ipl_rect_type  *icrop,
  ipl_rect_type  *ocrop
);


/*===========================================================================

FUNCTION ipl2_upSize_arb_med_RGBToRGB

DESCRIPTION
  Fast arbitrary upsize

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr - input image frame
  o_img_ptr - input image frame
  icrop     - input crop
  ocrop     - output crop

ARGUMENTS OUT
  o_img_ptr - input image frame

RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl_upSize_arb_med_RGBToRGB
(
  ipl_image_type *i_img_ptr, /* Points to the input image   */
  ipl_image_type *o_img_ptr, /* Points to the output image  */
  ipl_rect_type  *icrop,
  ipl_rect_type  *ocrop
);


/*===========================================================================
 *
 *
 * FUNCTION ipl_upSize_qcif2qvga_YCrCb420lpToRGB
 *
 * DESCRIPTION
 *   This function upsize from qcif to qvga while color converting from
 *   ycrcb420 line pack to rgb565
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   input_img_ptr  - input image frame
 *   i_param        -  image params
 *
 * ARGUMENTS OUT
 *   output_img_ptr - Points to the output image
 *
 * RETURN VALUE
 *   Status - success or failure
 *
 * SIDE EFFECTS
 *   Changes the output image buffer
 *
 *===========================================================================*/
extern ipl_status_type ipl_upSize_qcif2qvga_YCrCb420lpToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
);


#endif 
