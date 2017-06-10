#ifndef IPL_DOWNSIZE_H
#define IPL_DOWNSIZE_H




/*===========================================================================

                         IPL2_DOWNSIZE . H

GENERAL DESCRIPTION

  This file contains the function handlers to take care of various downsizing
  downsampling filters.

EXTERNALIZED FUNCTIONS
  ipl2_downsize_ycbcr420()

    Function which handles the downsizing of an existing YUV420 encoded
    image to an arbitrarily smaller sized RGB flavour.


INITIALIZATION AND SEQUENCING REQUIREMENTS

  The library will function properly only if the ipl2_init_lookupTables()
  function is called before calling anything else.

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_downSize.h#1 $

                            EDIT HISTORY FOR FILE


when        who    what, where, why
--------    ---    ----------------------------------------------------------
05/20/04   srk    Initial Creation.
===========================================================================*/

/*=========================================================================

                     GLOBAL DECLARATIONS

===========================================================================*/




/*===========================================================================

FUNCTION ipl2_downsize()

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
API_EXTERN ipl_status_type ipl2_downsize
(
  ipl_image_type*                   i_img_ptr, /* input image   */
  ipl_image_type*                   o_img_ptr, /* output image  */
  ipl2_image_downsize_param_type * i_param  /* processing param */
);






#endif 
