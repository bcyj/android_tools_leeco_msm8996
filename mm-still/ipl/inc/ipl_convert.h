#ifndef IPL_CONVERT_H
#define IPL_CONVERT_H

/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_convert.h#1 $
===========================================================================*/


extern const int32 ipl_crr[];
extern const int32 ipl_cbb[];
extern const int32 ipl_crg[];
extern const int32 ipl_cbg[];



/*===========================================================================

                      FUNCTION PROTOTYPES 

===========================================================================*/


// when we do color conversion of HxVx data to RGB (565 or 888), should we 
// assume Luma is from 16 to 235 or 0 to 255? 
//
// By not enabling this, we are assuming input data can go fromm 0 to 255 
//#define FEATURE_IPL_HXVX_16_235_TO_RGB_0_255



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_image

DESCRIPTION
  This function converts between many types of images types

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  12/19/04  Created

===========================================================================*/
API_EXTERN ipl_status_type ipl_convert_image
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr422_to_ycbcr444

DESCRIPTION
  This function converts from YCbCr 4:2:2 to YCbCr 4:4:4.
  Input must be in YCbCr 4:2:2 format.
  Output must be in YCbCr 4:4:4 format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  12/19/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr422_to_ycbcr444
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr444_to_ycbcr422

DESCRIPTION
  This function converts from YCbCr 4:4:4 to YCbCr 4:2:2.
  Input must be in YCbCr 4:4:4 format.
  Output must be in YCbCr 4:2:2 format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  01/13/05  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr444_to_ycbcr422
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb565_to_ycbcr444

DESCRIPTION
  This function converts from RGB565 to YCbCr 4:4:4 format. 

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr       points to the input image

ARGUMENTS OUT
  o_img_ptr       points to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  1/13/05  Created

===========================================================================*/
extern ipl_status_type ipl_convert_rgb565_to_ycbcr444
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_frame_rgb565_to_ycbcr444

DESCRIPTION
  This function converts a frame image from RGB565 to YCbCr 4:4:4 format, 
  with special treatment of transparent pixels. A 2-pass approach is used. 
  In the first pass, the transparent RGB565 pixels are converted to 
  YCbCr 4:4:4, where only the Y value is set to a transparent Y value given 
  by the input parameter transparentY. The Cb and Cr values are not set to 
  special transparent values. In the second pass, the function checks for 
  pixels that are not supposed to be transparent but happen to have Y values 
  equal to transparentY. These spurious transparent pixels are corrected by 
  decreasing their Y values by 1. The number of pixels corrected is stored 
  and returned in the parameter numCorrected.

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr           points to the input image
  transparentValue    transparent RGB565 pixel value
  transparentY        transparent luma value

ARGUMENTS OUT
  o_img_ptr           points to the output image
  numCorrected        number of spurious transparent pixels corrected

RETURN VALUE
  IPL_SUCCESS         indicates operation was successful
  IPL_FAILURE         otherwise

SIDE EFFECTS
  None

MODIFIED
  1/13/05  Created

===========================================================================*/
extern ipl_status_type ipl_convert_frame_rgb565_to_ycbcr444
(
  ipl_image_type* i_img_ptr,      /* Points to the input image      */
  ipl_image_type* o_img_ptr,      /* Points to the output image     */
  uint16 transparentValue,        /* Transparent RGB565 pixel value */
  uint8 transparentY,             /* Transparent luma value         */
  uint32* numCorrected            /* Number of pixels corrected     */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycrcb420lp_to_ycbcr422

DESCRIPTION
  This function converts from YCrCb 4:2:0 line packed format to YCbCr 4:2:2.
  Input should be YCrCb 4:2:0 line packed.
  Output should be YCbCr 4:2:2.
  Input and output image sizes should be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the Input Image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  11/22/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycrcb420lp_to_ycbcr422
(
  ipl_image_type* input_img_ptr,        /* Pointer to input image  */
  ipl_image_type* output_img_ptr        /* Pointer to output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycrcb422lp_to_ycbcr422

DESCRIPTION
  This function converts from YCrCb 4:2:2 line packed format to YCbCr 4:2:2.
  Input must be YCbCr 4:2:2 format.
  Output must be YCbCr 4:2:2 line packed.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  11/22/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycrcb422lp_to_ycbcr422
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb565_to_ycrcb420lp

DESCRIPTION
  This function converts from RGB565 to YCrCb 4:2:0 line packed format. 

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr       points to the input image

ARGUMENTS OUT
  o_img_ptr       points to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  11/19/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_rgb565_to_ycrcb420lp
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
);



#if 0
/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb565_to_ycrcb420lp

DESCRIPTION
  This function converts from RGB565 to YCrCb 4:2:0 line packed format. 

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr       points to the input image

ARGUMENTS OUT
  o_img_ptr       points to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  11/19/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_rgb565_to_ycrcb420lp_fast
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
);
#endif



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb565_to_ycrcb422lp

DESCRIPTION
  This function converts from RGB565 to YCrCb 4:2:2 line packed format. 

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr       points to the input image

ARGUMENTS OUT
  o_img_ptr       points to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  11/19/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_rgb565_to_ycrcb422lp
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
);


/*===========================================================================

FUNCTION ipl2_convert_rgb565_to_ycbcr422lp

DESCRIPTION
  This function is an optimized version to do convert an 
  RGB565 based image YCbCr

  Input is assumed to be in rgb565 format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
   crop - says the diemnsions on output

ARGUMENTS OUT
  output_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl2_convert_rgb565_to_ycbcr422lp
(
  ipl_image_type* i_img_ptr,    
  ipl_image_type* o_img_ptr,    
  ipl_rect_type* crop
);

/*===========================================================================

FUNCTION ipl2_convert_rgb565_to_ycbcr420lp

DESCRIPTION
  This function is an optimized version to do convert an 
  RGB565 based image YCbCr

  Input is assumed to be in rgb565 format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
   crop - says the diemnsions on output

ARGUMENTS OUT
  output_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
ipl_status_type ipl2_convert_rgb565_to_ycbcr420lp
(
  ipl_image_type* i_img_ptr,    /* Input Image Pointer            */
  ipl_image_type* o_img_ptr,    /* Output Image Pointer            */
  ipl_rect_type* crop
);



/* <EJECT> */
/*==========================================================================

FUNCTION    ipl3_convert_ycrcb422lp_to_rgb565

DESCRIPTION
  This function converts from YCrCb 4:2:2 line packed format to RGB565. 

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     points to the input image

ARGUMENTS OUT
  output_img_ptr    points to the output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/19/04  Created

==========================================================================*/
extern ipl_status_type ipl3_convert_ycrcb422lp_to_rgb565
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
);

/* <EJECT> */
/*==========================================================================

FUNCTION    ipl2_convert_ycrcb422lp_to_rgb565

DESCRIPTION
  This function converts from YCrCb 4:2:2 line packed format to RGB565. 

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     points to the input image

ARGUMENTS OUT
  output_img_ptr    points to the output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/19/04  Created

==========================================================================*/
extern ipl_status_type ipl2_convert_ycrcb422lp_to_rgb565
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
);



/* <EJECT> */
/*==========================================================================

FUNCTION    ipl_convert_ycrcb420lp_to_rgb565

DESCRIPTION
  This function converts from YCrCb 4:2:0 line packed format to RGB565. 

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     points to the input image

ARGUMENTS OUT
  output_img_ptr    points to the output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/09/04  Created

==========================================================================*/
extern ipl_status_type ipl_convert_ycrcb420lp_to_rgb565
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
);


/* <EJECT> */
/*==========================================================================

FUNCTION    ipl_convert_ycrcb422lp_to_rgb565

DESCRIPTION
  This function converts from YCrCb 4:2:2 line packed format to RGB565. 

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     points to the input image

ARGUMENTS OUT
  output_img_ptr    points to the output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/19/04  Created

==========================================================================*/
extern ipl_status_type ipl_convert_ycrcb422lp_to_rgb565
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
);



/* <EJECT> */
/*==========================================================================

FUNCTION    ipl_convert_ycrcb422lp_to_rgb565_fast

DESCRIPTION
  This function converts from YCrCb 4:2:2 line packed format to RGB565. 

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     points to the input image

ARGUMENTS OUT
  output_img_ptr    points to the output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/19/04  Created

==========================================================================*/
extern ipl_status_type ipl_convert_ycrcb420lp_to_rgb565_fast
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr422_to_ycrcb420lp

DESCRIPTION
  This function converts YCbCr 4:2:2 to YCrCb 4:2:0 line packed format.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     pointer to the Input Image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr    pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS       indicates operation was succesful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/09/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr422_to_ycrcb420lp
(
  ipl_image_type* input_img_ptr,    /* Pointer to the input image */
  ipl_image_type* output_img_ptr    /* Pointer to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb444_to_ycbcr420lp

DESCRIPTION
  This function converts from RGB444 to YCbCr 4:2:0 line packed format.
  Input must be in RGB444 format.
  Output must be in YCbCr 4:2:0 line packed format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/18/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_rgb444_to_ycbcr420lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr420lp_to_rgb444

DESCRIPTION
  This function converts from YCbCr 4:2:0 line packed to RGB444 format.
  Input must be in YCbCr 4:2:0 line packed format.
  Output must be in RGB444 format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/18/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr420lp_to_rgb444
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb666_to_ycbcr420lp

DESCRIPTION
  This function converts from RGB666 to YCbCr 4:2:0 line packed format.
  Input must be in RGB666 format.
  Output must be in YCbCr 4:2:0 line packed format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/18/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_rgb666_to_ycbcr420lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr420lp_to_rgb666

DESCRIPTION
  This function converts from YCbCr 4:2:0 line packed to RGB666 format.
  Input must be in YCbCr 4:2:0 line packed format.
  Output must be in RGB666 format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/18/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr420lp_to_rgb666
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr420lp_to_ycbcr422lp

DESCRIPTION
  This function converts from YCbCr 4:2:0 line packed to YCbCr 4:2:2 line 
  packed format.
  Input must be in YCbCr 4:2:0 line packed format.
  Output must be in YCbCr 4:2:2 line packed format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/12/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr420lp_to_ycbcr422lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr422lp_to_ycbcr420lp

DESCRIPTION
  This function converts from YCbCr 4:2:2 line packed to YCbCr 4:2:0 line 
  packed format.
  Input must be in YCbCr 4:2:2 line packed format.
  Output must be in YCbCr 4:2:0 line packed format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/12/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr422lp_to_ycbcr420lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr420lp_to_ycbcr444lp

DESCRIPTION
  This function converts from YCbCr 4:2:0 line packed to YCbCr 4:4:4 line 
  packed format.
  Input must be in YCbCr 4:2:0 line packed format.
  Output must be in YCbCr 4:4:4 line packed format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/12/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr420lp_to_ycbcr444lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr444lp_to_ycbcr420lp

DESCRIPTION
  This function converts from YCbCr 4:4:4 line packed to YCbCr 4:2:0 line 
  packed format.
  Input must be in YCbCr 4:4:4 line packed format.
  Output must be in YCbCr 4:2:0 line packed format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/12/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr444lp_to_ycbcr420lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr422_to_ycbcr422lp

DESCRIPTION
  This function converts from YCbCr 4:2:2 to YCbCr 4:2:2 line packed format.
  Input must be in YCbCr 4:2:2 format.
  Output must be in YCbCr 4:2:2 line packed format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/11/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr422_to_ycbcr422lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr422_to_ycrcb422lp

DESCRIPTION
  This function converts from YCbCr 4:2:2 to YCrCb 4:2:2 line packed format.
  Input must be in YCbCr 4:2:2 format.
  Output must be in YCrCb 4:2:2 line packed format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/11/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr422_to_ycrcb422lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr422lp_to_ycbcr422

DESCRIPTION
  This function converts from YCbCr 4:2:2 to YCbCr 4:2:2 line packed format.
  Input must be in YCbCr 4:2:2 line packed format.
  Output must be in YCbCr 4:2:2 format.
  Input and output image sizes must be equal.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/11/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr422lp_to_ycbcr422
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_conv_frame

DESCRIPTION
  This function will do frame color conversion. It ensures that the 
  transparent pixel value is unique after color conversion.
  
  Input frame can be RGB565 or YCbCr 4:2:2.
  Output frame can be RGB565 or YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN
  i_frame_ptr        pointer to the input frame
  o_frame_ptr        pointer to the output frame
  transparentValue   16-bit transparent pixel value

RETURN VALUE
  IPL_SUCCESS        indicates operation was successful
  IPL_FAILURE        otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_conv_frame
(
  ipl_image_type* i_frame_ptr,      /* Points to the input frame      */
  ipl_image_type* o_frame_ptr,      /* Points to the output frame     */
  uint16 transparentValue           /* Transparent pixel value        */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB8882RGB565be

DESCRIPTION
  This function performs color conversion from RGB888 to RGB565be.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in     pointer to input image data
  width       width of input image
  height      height of input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out    pointer to output image data

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_RGB8882RGB565be
(
  uint8* data_in,            /* Points to the input image data     */
  uint16* data_out,          /* Points to the output image data    */
  int32 width,               /* Width of the input image           */
  int32 height               /* Height of the input image          */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB8882RGB565

DESCRIPTION
  This function performs color conversion from RGB888 to RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in     pointer to input image data
  width       width of input image
  height      height of input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out    pointer to output image data

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_RGB8882RGB565
(
  uint8* data_in,            /* Points to the input image data     */
  uint16* data_out,          /* Points to the output image data    */
  int32 width,               /* Width of the input image           */
  int32 height               /* Height of the input image          */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB8882YCbCr

DESCRIPTION
  This function performs color conversion from RGB888 to YCbCr 4:2:2
  It considers 2 pixels at a time, unpacks the rgb values, and outputs
  Y1Cb and Y2Cr.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in     pointer to input image data
  width       width of input image
  height      height of input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out    pointer to output image data

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  01/08/02  Created

===========================================================================*/
extern ipl_status_type ipl_RGB8882YCbCr
(
  unsigned char* data_in,               /* Points to input image data     */
  unsigned char* data_out,              /* Points to output image data    */
  short width,                          /* Width of the input image       */
  short height                          /* Height of the input image      */
);





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr420lp_to_ycbcr422

DESCRIPTION
  This function converts from YCbCr 4:2:0 line packed format to YCbCr 4:2:2.
  Input must be in YCbCr 4:2:0 line packed format.
  Output must be in YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  08/28/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr420lp_to_ycbcr422
(
  ipl_image_type* input_img_ptr,    /* Points to the input image    */
  ipl_image_type* output_img_ptr    /* Points to the output image   */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr422_to_ycbcr420lp

DESCRIPTION
  This function converts from YCbCr 4:2:2 to YCbCr 4:2:0 line packed format.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr422_to_ycbcr420lp
(
  ipl_image_type* input_img_ptr,    /* Points to the input image  */
  ipl_image_type* output_img_ptr    /* Points to the output image */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_to_rgb444666888

DESCRIPTION
  This function accepts YCbCr 4:2:2 or RGB565 as input and outputs RGB444,
  RGB666, or RGB888 as a double word as follows: The most significant 4 bits 
  are 0, followed by 4 bits of R, then G and then B.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  04/15/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_to_rgb444666888
(
  ipl_image_type* input_img_ptr,           /* Points to the input image  */
  ipl_image_type* output_img_ptr           /* Points to the output image */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ycbcr_to_hsv_normalized

DESCRIPTION
  This function converts from YCbCr 4:2:2 to HSV.

DEPENDENCIES
  None

ARGUMENTS IN/OUT
  input_img_ptr     pointer to the input image
  output_img_ptr    pointer to the input image
                    (H, S, and V are 0 to 255)

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ycbcr_to_hsv_normalized
(
  ipl_image_type* input_img_ptr,     /* Points to the input image   */
  ipl_image_type* output_img_ptr     /* Points to the output image   */
);


/* <EJECT> */

/*===========================================================================

FUNCTION hsv_to_ycbcr_normalized

DESCRIPTION
  This function converts from HSV to YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN/OUT
  input_img_ptr     pointer to the input image
  output_img_ptr    pointer to the input image
                    (H, S, and V are 0 to 255)
                

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type hsv_to_ycbcr_normalized
(
  ipl_image_type* intput_img_ptr,    /* Points to the input image   */
  ipl_image_type* output_img_ptr     /* Points to the output image   */
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB4442YCbCr

DESCRIPTION
  This function performs color conversion from RGB444 to YCbCr 4:2:2.
  It considers 2 pixels at time, unpacks the RGB values, and outputs
  Y1Cb and Y2Cr.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in     pointer to the input image data
  width       width of the input image
  height      height of the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out    pointer to the output image data

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  04/09/04  Created

===========================================================================*/
extern ipl_status_type ipl_RGB4442YCbCr
(
  unsigned char* data_in,         /* Points to the input image data   */
  unsigned char* data_out,        /* Points to the output image data  */
  short width,                    /* Width of the input image         */
  short height                    /* Height of the input image        */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_to_rgb444

DESCRIPTION
  This function converts from RGB565 or YCbCr 4:2:2 to RGB444 format as a 
  double word as follows:  The most significant 4 bits are 0, followed by
  4 bits of R, then G, and then B.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  04/15/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_to_rgb444
(
  ipl_image_type* input_img_ptr,     /* Points to the input image   */
  ipl_image_type* output_img_ptr     /* Points to the output image  */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_to_rgb666

DESCRIPTION
  This function converts from RGB565 or YCbCr 4:2:2 to RGB666 format as a 
  double word as follows:  The most significant 14 bits are 0, followed by
  6 bits of R, then G, and then B.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was successful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_convert_to_rgb666
(
  ipl_image_type* input_img_ptr,     /* Points to the input image  */
  ipl_image_type* output_img_ptr     /* Points to the output image */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_to_rgb888

DESCRIPTION
  This function converts from RGB565, YCbCr 4:2:2, or Bayer to RGB888 packed 
  format.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr    pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr   pointer to the output image

RETURN VALUE
  IPL_SUCCESS      indicates operation was successful
  IPL_FAILURE      otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_convert_to_rgb888
(
  ipl_image_type* input_img_ptr,   /* Points to the input image   */
  ipl_image_type* output_img_ptr   /* Points to the output image  */
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB8882RGB565plt

DESCRIPTION
  This function performs color conversion from RGB888 to RGB565 using a 
  pallete to do lookup.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in     input data
  width       width of input frame
  height      height of the input frame

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out    output data

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
API_EXTERN ipl_status_type ipl_RGB8882RGB565plt
(
    uint8* data_in, 
    uint8* palette, 
    uint16* data_out,
    int16 width, 
    int16 height
);

/* <EJECT> */
/*===========================================================================
 *
 * FUNCTION ipl_RGBA8882RGB565plt
 *
 * DESCRIPTION
 *   This function performs color conversion from RGB888 to RGB565 using a
 *   RGBA pallete to do lookup. This function differs from
 *   ipl_RGBA8882RGB565plt
 *   in that the palette is RGBA and different endian order.
 *
 * DEPENDENCIES
 *   None
 *
 * ARGUMENTS IN
 *   data_in     input data
 *   palette     input RGBA palette
 *   width       width of input frame
 *   height      height of the input frame
 *
 * ARGUMENTS IN/OUT
 *   None
 *
 * ARGUMENTS OUT
 *   data_out    output data
 *
 * RETURN VALUE
 *   None
 *
 * SIDE EFFECTS
 *   None
 *
 * MODIFIED
 *  07/31/02  Created
 *
 *===========================================================================*/
API_EXTERN ipl_status_type ipl_RGBA8882RGB565plt
(
  uint8* data_in, 
  uint8* palette, 
  uint16* data_out,
  int16 width, 
  int16 height
);


/* <EJECT> */
/*==========================================================================

FUNCTION    ipl_convert_swap_chroma_order

DESCRIPTION
  This function swaps CrCb to CbCr or from CbCr to CrCb. Supports 4:2:2 
  or 4:2:0 LINE_PK only.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     points to the input image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/09/04  Created

==========================================================================*/
API_EXTERN ipl_status_type ipl_convert_swap_chroma_order
(
  ipl_image_type* input_img_ptr            /* Pointer to the input image  */
);

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycrcb420mb_to_ycrcb420lp

DESCRIPTION
  This function converts YCrCb 4:2:0 MacroBlock to YCxCx 4:2:y line packed 

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     pointer to the Input Image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr    pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS       indicates operation was succesful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/09/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycrcb420mb_to_ycrcb420lp
(
  ipl_image_type* input_img_ptr,   /* Points to the input image   */
  ipl_image_type* output_img_ptr   /* Points to the output image  */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr420fp_to_rgb565

DESCRIPTION
  This function converts from YCbCr 4:2:0 frame packed to rgb565
  Input and output image sizes must be equal size.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   pointer to the input image

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr  pointer to the output image

RETURN VALUE
  IPL_SUCCESS     indicates operation was succesful
  IPL_FAILURE     otherwise

SIDE EFFECTS
  None

MODIFIED
  10/12/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr420fp_to_rgb565
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
);

/* <EJECT> */
/*==========================================================================

FUNCTION    ipl3_convert_ycrcb420lp_to_rgb565

DESCRIPTION
  This function converts from YCrCb 4:2:0 line packed format to RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     points to the input image

ARGUMENTS OUT
  output_img_ptr    points to the output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/09/04  Created

==========================================================================*/
extern ipl_status_type ipl3_convert_ycrcb420lp_to_rgb565
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
);

/* <EJECT> */
/*==========================================================================

FUNCTION    ipl2_convert_ycrcb420lp_to_rgb565

DESCRIPTION
  This function converts from YCrCb 4:2:0 line packed format to RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     points to the input image

ARGUMENTS OUT
  output_img_ptr    points to the output image

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

MODIFIED
  11/09/04  Created

==========================================================================*/
extern ipl_status_type ipl2_convert_ycrcb420lp_to_rgb565
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
);



/* <EJECT> */
/*===========================================================================

FUNCTION ycbcr_to_hsv_pixel

DESCRIPTION
  This function converts from YCbCr 4:2:2 to HSV.

DEPENDENCIES
  None

ARGUMENTS IN/OUT

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type hsv_to_ycbcr_pixel
(
  int32 hout,
  int32 sout,
  int32 vout,
  unsigned char *y,
  unsigned char *cb,
  unsigned char *cr
);



/* <EJECT> */
/*===========================================================================

FUNCTION ycbcr_to_hsv_pixel

DESCRIPTION
  This function converts from YCbCr 4:2:2 to HSV.

DEPENDENCIES
  None

ARGUMENTS IN/OUT

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ycbcr_to_hsv_pixel
(
  int32 y,
  int32 cb,
  int32 cr,
  int32 *hout,
  int32 *sout,
  int32 *vout
);


#endif /* IPL_CONVERT_H */
