#ifndef IPL_ROTADDCROP_H
#define IPL_ROTADDCROP_H

#define IPL2_MAX_RGB_ICONS           100

/*==========================================================================
  
Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/



/*==========================================================================

FUNCTION        IPL2_ROT_ADD_CROP

DESCRIPTION

  INITIALIZATION

  This module requires some of the conversion tables initialized before
  triggering this function. It is suggested that the user should call the
  API ipl2_init() before calling this API.

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
  crop is a structure informing ipl how to crop
  rotate is the rotation to do
  icon_list_ptr is a ptr to a NULL terminated list of icons
  Icons are only processed if input is IPL_RGB565 and output is IPL_RGB565
  transparentValue is the 16 bit transparent pixel value

ARGUMENTS OUT
  o_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  Modifies the output image buffer

==========================================================================*/
API_EXTERN ipl_status_type ipl2_rot_add_crop
(
  ipl_image_type* i_img_ptr,        /* Points to the input image        */
  ipl_image_type* input_frame_ptr,  /* Points to the frame              */
  ipl_image_type* o_img_ptr,        /* Points to the output image       */
  ipl_rect_type* crop,              /* Crop config                      */
  ipl_rotate90_type rotate,         /* Rotatation                       */
  ipl_icon_type** icon_list_ptr,    /* Ptr to null terminated icon list */
  uint16 transparentValue           /* Transparent pixel value          */
);





extern ipl_status_type ipl2_Rot000Frame_CropYCrCb420lpToRGB
(
  ipl_image_type* input_img_ptr,    /* Points to the input image        */
  ipl_image_type* i_frame_ptr,      /* Points to the frame              */
  uint16 transparentValue,          /* Transparent pixel value          */
  ipl_image_type* output_img_ptr,   /* Points to the output image       */
  ipl_rect_type* icrop,             /* Crop config                      */
  ipl_rect_type* ocrop              /* Crop config                      */
);

extern ipl_status_type ipl2_Rot090Frame_CropYCrCb420lpToRGB
(
  ipl_image_type* input_img_ptr,    /* Points to the input image        */
  ipl_image_type* i_frame_ptr,      /* Points to the frame              */
  uint16 transparentValue,          /* Transparent pixel value          */
  ipl_image_type* output_img_ptr,   /* Points to the output image       */
  ipl_rect_type* icrop,             /* Crop config                      */
  ipl_rect_type* ocrop              /* Crop config                      */
);


extern ipl_status_type ipl2_Rot180Frame_CropYCrCb420lpToRGB
(
  ipl_image_type* input_img_ptr,    /* Points to the input image        */
  ipl_image_type* i_frame_ptr,      /* Points to the frame              */
  uint16 transparentValue,          /* Transparent pixel value          */
  ipl_image_type* output_img_ptr,   /* Points to the output image       */
  ipl_rect_type* icrop,             /* Crop config                      */
  ipl_rect_type* ocrop              /* Crop config                      */
);

extern ipl_status_type ipl2_Rot270Frame_CropYCrCb420lpToRGB
(
  ipl_image_type* input_img_ptr,    /* Points to the input image        */
  ipl_image_type* i_frame_ptr,      /* Points to the frame              */
  uint16 transparentValue,          /* Transparent pixel value          */
  ipl_image_type* output_img_ptr,   /* Points to the output image       */
  ipl_rect_type* icrop,             /* Crop config                      */
  ipl_rect_type* ocrop              /* Crop config                      */
);






#endif /* end of IPL_ROTADDCROP_H */
