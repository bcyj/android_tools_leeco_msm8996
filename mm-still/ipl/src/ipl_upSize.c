/*===========================================================================

                        IPL2_UPSIZE . C

GENERAL DESCRIPTION

  This file contains the function handlers to take care of various
  downsizing/downsampling filters.

  CALL FLOW OF THIS MODULE

  1 IPL2_HANDLE_UPSIZE

                        |
                        |
              ipl2_handle_upSize
                        |
|-----------------------|------------------------------------
|RGB565/444             |                                   |
|(only low qual )       |                                   |
ipl2_upsize_rgb      |YCbCr4:4:2                         |(YCbCr420
                        | (only RGB o/p)                    | RGB O/P )
               ipl2_upsize_YCbCr442()         ipl2_upsize_YCbCr420()
               |        |           |            |            |       |
     (low qual)|        |           |            | (low qual) |(med)  |
               |        | (med qal) |            |            |       |
               |        |           |            |            |  ipl2_upSizeAverageAndCrop_YCbCr420To2RGB
               |        |           |            |            |
               |        |           |            |            |
               |        |           |            |  ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB
               |        |           |            |
               |        |           |ipl2_upSize2AndCrop_YCbCr420ToRGB
               |        |           |
               |        |           |
               |        |  ipl2_upSizeAverageAndCrop_YCbCr2RGB
               |        |
               |   ipl2_upSizeRowAvgAndCrop_YCbCr2RGB
               |
ipl2_upSizeAndCrop_YCbCr2RGB


                      2 IPL2_UPSIZEQCIF_YCBCR420_133()
                                   |
                                   |
                                   |
                      ipl2_upsizeQCIF_YCbCr420_133
                                   |
     ------------------------------|-------------------
     |                                                |
     |                                                |
     |LS mode                                         |Portriat Mode
     |                                                |
     |                                                |
     |                                                |
     |                                                |
     |                                                |
     |                                           _____|_____
 ____|______                                     |high     | Med
|           |                                    |         |
|           |                                    |         ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_Med_133_PT
|           |                                    |
|High       | Med Qual                ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133_PT
|         ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133
| qual
|
ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133


  3 IPL2_UPSIZEQCIF_YCBCR_133 ()



                                   |
                                   |
                                   |
                      ipl2_upsizeQCIF_YCbCr_133
                                   |
     ------------------------------|-------------------
     |                                                |
     |                                                |
     |LS mode                                         |Portriat Mode
     |                                                |
     |                                                |
     |                                                |
     |                                                |
     |                                                |
     |                                           _____|_____
 ____|______                                     |high     | Med
|           |                                    |         |
|           |                                    |         ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIF_Med_133_PT
|           |                                    |
|High       | Med Qual                ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIF_High_133_PT
|         ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133
| qual
|
ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIF_High_133


EXTERNALIZED FUNCTIONS

  ipl2_handle_upSize()

    This function takes in a input image an then upsamples the image to an
    abitrarily sized o/p image. The crop parameters controlls the decides
    the crop deimensions.

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


   Copyright (c) 2004 by Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary.  Export of this technology or software is
  regulated by the U.S. Government. Diversion contrary to U.S.
  law prohibited.
===========================================================================*/

/*===========================================================================

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_upSize.c#1 $

                            EDIT HISTORY FOR FILE


when        who    what, where, why
--------    ---    ----------------------------------------------------------
07/19/04   srk    1.33 upscaling for QCIF in lanscape and portrait mode
06/09/04   srk    Linted version
===========================================================================*/
/*=========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#include <stdio.h>
#include <string.h>

#include "ipl_types.h"     
#include "ipl_xform.h"     
#include "ipl_helper.h"     

#include "ipl_qvp.h"     
#include "ipl_upSize.h"     

#define IPL_UPSIZE_DEBUG 0





// this table holds the number of rows we upsize/downsize to/from based
// on resize ratio in Q8
static const int32 resizeFactor[165] = 
{
//Mag(Q8)  To    From   Index   Magnification
  28,      1,    9,     // 0       0.111111
  32,      1,    8,     // 3       0.125000
  36,      1,    7,     // 6       0.142857
  42,      1,    6,     // 9       0.166667
  51,      1,    5,     // 12      0.200000
  56,      2,    9,     // 15      0.222222
  64,      1,    4,     // 18      0.250000
  73,      2,    7,     // 21      0.285714
  85,      1,    3,     // 24      0.333333
  96,      3,    8,     // 27      0.375000
  102,     2,    5,     // 30      0.400000
  109,     3,    7,     // 33      0.428571
  113,     4,    9,     // 36      0.444444
  128,     1,    2,     // 39      0.500000
  142,     5,    9,     // 42      0.555556
  146,     4,    7,     // 45      0.571429
  153,     3,    5,     // 48      0.600000
  160,     5,    8,     // 51      0.625000
  170,     2,    3,     // 54      0.666667
  182,     5,    7,     // 57      0.714286
  192,     3,    4,     // 60      0.750000
  199,     7,    9,     // 63      0.777778
  204,     4,    5,     // 66      0.800000
  213,     5,    6,     // 69      0.833333
  219,     6,    7,     // 72      0.857143
  224,     7,    8,     // 75      0.875000
  227,     8,    9,     // 78      0.888889

  256,     1,    1,     // 81      1.000000
  288,     9,    8,     // 84      1.125000
  292,     8,    7,     // 87      1.142857
  298,     7,    6,     // 90      1.166667
  307,     6,    5,     // 93      1.200000
  320,     5,    4,     // 96      1.250000
  329,     9,    7,     // 99      1.285714
  341,     4,    3,     // 102     1.333333
  358,     7,    5,     // 105     1.400000
  384,     3,    2,     // 108     1.500000 
  409,     8,    5,     // 111     1.600000
  426,     5,    3,     // 114     1.666667
  448,     7,    4,     // 117     1.750000
  460,     9,    5,     // 120     1.800000
  512,     4,    2,     // 123     2.000000 // dont do 1->2 but 2->4

  576,     9,    4,     // 126     2.250000
  597,     7,    3,     // 129     2.333333
  640,     5,    2,     // 132     2.500000
  682,     8,    3,     // 135     2.666667
  768,     6,    2,     // 138     3.000000 // dont do 1->3 but 3->6
  896,     7,    2,     // 141     3.500000 // quality can't be that good
  1024,    4,    1,     // 144     4.000000 // quality can't be that good
  1152,    9,    2,     // 147     4.500000 // quality can't be that good
  1280,    5,    1,     // 150     5.000000 // quality can't be that good
  1536,    6,    1,     // 153     6.000000 // quality can't be that good
  1792,    7,    1,     // 156     7.000000 // quality can't be that good
  2048,    8,    1,     // 159     8.000000 // quality can't be that good
  2304,    9,    1      // 162     9.000000 // quality can't be that good
};


/*==========================================================================
                    FUNCTION PROTOTYPES
===========================================================================*/








/*lint -save -e504, constant value boolean is totally okay */
/*lint -save -e506, constant value boolean is totally okay */
/*lint -save -e508, constant value boolean is totally okay */
/*lint -save -e509, constant value boolean is totally okay */
/*lint -save -e527, unreachable code is okay, put breaks; for readibility */
/*lint -save -e534, ignore return value from unpack function */
/*lint -save -e574, signed and unsigned mixing okay */
/*lint -save -e613  don worry about checking if input is null */
/*lint -save -e703, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e704, let me use if(1) */
/*lint -save -e713, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e732, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e734, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e736, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e737, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e747, errors in create gamma curve are okay */
/*lint -save -e774, constant value boolean is totally okay */
/*lint -save -e818, lets not worry about const optimization right now */
/*lint -save -e834, this warning is just dumb */
/*lint -save -e826, we should look into this someday */
/*lint -save -e831, this warning just repeats earliner warning */
/*lint -save -e825, fallthrough in case is so we dont get lint err on return */
/*lint -save -e702, dont worry about signed shit, it will be + */
/*lint -save -e573, dont worry about signed unsigned multiplied and divided */


/*--------------------------------------------------------------------------
 * lookup table which simplifies the conversion from YCBR to RGB565
 * these tables store computationally intensive intermediate
 * functions. Look at ipl_init_lookup_tables for more .
 * Selection of these smaller tables and co-locating them
 * is to aid better cache performance.
 * --------------------------------------------------------------------------*/
extern int16 ipl2_CrToRTable[];
extern int16 ipl2_CrToGTable[];
extern int16 ipl2_CbToGTable[];
extern int16 ipl2_CbToBTable[];

/*--------------------------------------------------------------------------
 * shifted and quantized values for RGB565/444/666
 * encoded images
 * --------------------------------------------------------------------------*/
extern const uint16 ipl2_r5xx[];
extern const uint16 ipl2_gx6x[];
extern const uint16 ipl2_bxx5[];
extern const uint16 ipl2_r444[];
extern const uint16 ipl2_g444[];
extern const uint16 ipl2_b444[];






/*==========================================================================
                    FUNCTION DEFINITIONS
===========================================================================*/


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
)
{
  MSG_LOW("inside ipl2_upsize\n");

  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_handle_upSize :: /\
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }

/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
    Make sure we have valid inputs
  ------------------------------------------------------------------------*/
  if(!i_img_ptr || !o_img_ptr || !i_param)
  {
    IPL2_MSG_FATAL( "ipl2_upsize_crop failed for passing NULL  /\
     img structs i_img_ptr = %p, o_img_ptr = %p, i_param = %p",
     i_img_ptr,  o_img_ptr, i_param );

    return(IPL_FAILURE);
  }

  /*------------------------------------------------------------------------
    Make sure we have good image buffers inside.
  ------------------------------------------------------------------------*/
  if( !i_img_ptr->imgPtr || !i_img_ptr->imgPtr )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_crop failed for passing NULL  /\
     img structs i_img_ptr->imgPtr = %p, o_img_ptr->imgPtr = %p",
     i_img_ptr->imgPtr,  o_img_ptr->imgPtr );

    return(IPL_FAILURE);

  }

  /*------------------------------------------------------------------------
      See if we got valid buffers inside
  ------------------------------------------------------------------------*/
  if( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
  {

    IPL2_MSG_FATAL( "ipl2_upsize_crop failed for passing NULL  /\
       img buffer(s) i_img_ptr->imgPtr = %p, o_img_ptr->imgPtr = %p",
         i_img_ptr->imgPtr,  o_img_ptr->imgPtr );

    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
    Call the handler for downsize
  ------------------------------------------------------------------------*/
  return(ipl2_handle_upSize(i_img_ptr, o_img_ptr, i_param));
}





/*===========================================================================

FUNCTION IPL2_UPSIZE_QCIF_133()

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
extern ipl_status_type ipl2_upsize_QCIF_133
(
  ipl_image_type*                 i_img_ptr,  /* input image   */
  ipl_image_type*                 o_img_ptr,  /*  output image */
  ipl2_image_upsize_param_type *i_param    /* upsize param  */
)
{
  MSG_LOW("inside ipl2_upsize_QCIF_133\n");




/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_upsize_QCIF_133 :: /\
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
           NULL check the bare minimum parameters required
   -----------------------------------------------------------------------*/
  if ( !i_param || !i_img_ptr || !o_img_ptr )
  {
    IPL2_MSG_FATAL( "ipl2_rot_add_crop failed i_param = %lu, /\
                   i_img_ptr = %lu, o_img_ptr = %lu", (uint32) i_param,
                   (uint32) i_img_ptr, (uint32) o_img_ptr);
    return( IPL_FAILURE );
  }
  else
  {
    /*----------------------------------------------------------------------
        See if images have actual buffers
     ---------------------------------------------------------------------*/

    if ( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
    {
      IPL2_MSG_FATAL( "ipl2_upsize_QCIF failed  i_img_ptr->imgPtr = %lu, /\
                   o_img_ptr->imgPtr = %lu", (uint32) i_img_ptr->imgPtr,
                   (uint32) o_img_ptr->imgPtr);

      return( IPL_FAILURE );
    }
  }


  /*------------------------------------------------------------------------
      For now we support only RGB o/p
  ------------------------------------------------------------------------*/
  if( !( ( o_img_ptr->cFormat == IPL_RGB565 ) ||
        ( o_img_ptr->cFormat == IPL_RGB444 ) ||
        ( o_img_ptr->cFormat == IPL_RGB666 )  ) )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_QCIF failed  : Unsupported o/p color /\
                   format  " );
    return( IPL_FAILURE );

  }


  /*------------------------------------------------------------------------
      Switch on the input image type
  ------------------------------------------------------------------------*/
  switch( i_img_ptr->cFormat)
  {
    case IPL_YCbCr:
      return( ipl2_upsizeQCIF_YCbCr_133( i_img_ptr, o_img_ptr, i_param) );
      /*NOTREACHED*/
    break;

    case IPL_YCbCr420_FRAME_PK :
      return( ipl2_upsizeQCIF_YCbCr420_133( i_img_ptr, o_img_ptr, i_param ) );
      /*NOTREACHED*/
    break;

    case IPL_YCrCb420_LINE_PK :
      return( ipl2_upsizeQCIF_YCrCb420lp_133( i_img_ptr, o_img_ptr, i_param ) );
      /*NOTREACHED*/
    break;

    default:
      IPL2_MSG_FATAL( "ipl2_upsize_QCIF failed  : Unsupported i/p color /\
               format  " );
      return( IPL_FAILURE );
      /*NOTREACHED*/
    break;
  }


  /*NOTREACHED*/
  return( IPL_SUCCESS );

} /* end of function ipl2_upsize_QCIF */



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
)
{
  MSG_LOW("inside ipl2_upsizeQCIF_YCbCr_133\n");



  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_upsizeQCIF_YCbCr_133 :: /\
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }





  /*------------------------------------------------------------------------
      Determine if this is portrait or Landscape mode QCIF frame
  ------------------------------------------------------------------------*/
  if( i_img_ptr->dx ==  IPL2_QCIF_WIDTH )
  {
    /*------------------------------------------------------------------------
     Switch based on quality
    ------------------------------------------------------------------------*/
    switch( i_param->qual )
    {
      case IPL_QUALITY_LOW :
      case IPL_QUALITY_MEDIUM :
        return(
              ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133(
                i_img_ptr,
                o_img_ptr,
                i_param
                ) );
      case IPL_QUALITY_HIGH:
        return(
            ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133(
                                                      i_img_ptr,
                                                      o_img_ptr,
                                                      i_param
                                                      ) );
      default:
        return( IPL_FAILURE );
    } /* end of switch */
  }
  else if (i_img_ptr->dx == IPL2_QCIF_HEIGHT )
  {
    /*----------------------------------------------------------------------
      We entered portrait mode
    ----------------------------------------------------------------------*/

    /*----------------------------------------------------------------------
      Switch based on quality
    ------------------------------------------------------------------------*/
    switch( i_param->qual )
    {
      case IPL_QUALITY_LOW :
      case IPL_QUALITY_MEDIUM :
        return(
             ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133_PT(
                i_img_ptr,
                o_img_ptr,
                i_param
                ) );
      case IPL_QUALITY_HIGH:
        return(
          ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133PT(
                                                      i_img_ptr,
                                                      o_img_ptr,
                                                      i_param
                                                      ) );
      default:
        return( IPL_FAILURE );
    } 
  }
  return( IPL_FAILURE );
} /* end of function ipl2_upsizeQCIF_YCbCr_133 */

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
)
{
  MSG_LOW("inside ipl2_upsizeQCIF_YCbCr420_133\n");




  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_upsize_QCIF_YCbCr420_133 :: /\
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }


  /*------------------------------------------------------------------------
      Determine if this is portrait or Landscape mode QCIF frame
  ------------------------------------------------------------------------*/
  if( i_img_ptr->dx ==  IPL2_QCIF_WIDTH )
  {
    /*------------------------------------------------------------------------
        Switch based on quality
    ------------------------------------------------------------------------*/
    switch( i_param->qual )
    {



      case IPL_QUALITY_LOW :
      case IPL_QUALITY_MEDIUM :
        return(
            ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133(
                i_img_ptr,
                o_img_ptr,
                i_param
                ) );
        /*NOTREACHED*/
        break;

      case IPL_QUALITY_HIGH:
        return(
        ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133(
                                                      i_img_ptr,
                                                      o_img_ptr,
                                                      i_param
                                                      ) );
        /*NOTREACHED*/
        break;



      default:
        return( IPL_FAILURE );
        /*NOTREACHED*/
        break;
    } /* end of switch */

  }
  else
  {
        /*------------------------------------------------------------------------
        Switch based on quality
    ------------------------------------------------------------------------*/
    switch( i_param->qual )
    {



      case IPL_QUALITY_LOW :
      case IPL_QUALITY_MEDIUM :
        return(
            ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133_PT(
                i_img_ptr,
                o_img_ptr,
                i_param
                ) );
        /*NOTREACHED*/
        break;

      case IPL_QUALITY_HIGH:
        return(
        ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133_PT(
                                                      i_img_ptr,
                                                      o_img_ptr,
                                                      i_param
                                                      ) );
        /*NOTREACHED*/
        break;



      default:
        return( IPL_FAILURE );
        /*NOTREACHED*/
        break;
    } /* end of switch */


  }



} /* end of function ipl2_upsizeQCIF_YCbCr420_133 */










/*===========================================================================

FUNCTION  ipl2_upsizeQCIF_YCrCb420lp_133()

DESCRIPTION

    See ipl2_upsizeQCIF_YCrCb420lp_133

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

extern ipl_status_type ipl2_upsizeQCIF_YCrCb420lp_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  MSG_LOW("inside ipl2_upsizeQCIF_YCrCb420lp_133\n");




  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_upsize_QCIF_YCbCr420_133 :: /\
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }


  /*------------------------------------------------------------------------
      Determine if this is portrait or Landscape mode QCIF frame
  ------------------------------------------------------------------------*/
  if( i_img_ptr->dx ==  IPL2_QCIF_WIDTH )
  {
    /*------------------------------------------------------------------------
        Switch based on quality
    ------------------------------------------------------------------------*/
    switch( i_param->qual )
    {
      case IPL_QUALITY_LOW :
      case IPL_QUALITY_MEDIUM :
      case IPL_QUALITY_HIGH:
        return(
        ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133(
                                                      i_img_ptr,
                                                      o_img_ptr,
                                                      i_param
                                                      ) );
        //break;
      default:
        return( IPL_FAILURE );
        //break;
    } 
  }
  else
  {
   /*------------------------------------------------------------------------
        Switch based on quality
    ------------------------------------------------------------------------*/
    switch( i_param->qual )
    {
      case IPL_QUALITY_LOW :
      case IPL_QUALITY_MEDIUM :
      case IPL_QUALITY_HIGH:
        return(
        ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133_PT(
                                                      i_img_ptr,
                                                      o_img_ptr,
                                                      i_param
                                                      ) );
        //break;
      default:
        return( IPL_FAILURE );
        //break;
    } 
  }
} 


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
  )
{
  MSG_LOW("inside ipl2_handle_upSize\n");



  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_handle_upSize :: /\
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }

/*------------------------------------------------------------------------*/

  switch ( i_img_ptr->cFormat )
  {

    case IPL_RGB565 :
      /*------------------------------------------------------------------
          We need to form a function to handle this case
      ------------------------------------------------------------------*/
      return(ipl_upsize(i_img_ptr, o_img_ptr, NULL, IPL_QUALITY_LOW));
    //break;

    case IPL_YCbCr:

      /*------------------------------------------------------------------
          Call the handler for YCbCr420 upsize
      ------------------------------------------------------------------*/
      return( ipl2_upsize_YCbCr442( i_img_ptr, o_img_ptr, i_param ) );
      /*NOTREACHED*/
    //break;

    case IPL_YCbCr420_FRAME_PK :
      /*-----------------------------------------------------------------
          Call the handler to do upsizing for YUV420 Frame Pack
      ------------------------------------------------------------------*/
      return( ipl2_upsize_YCbCr420( i_img_ptr, o_img_ptr, i_param ) );
      /*NOTREACHED*/
    //break;

    case IPL_YCrCb420_LINE_PK :
      /*-----------------------------------------------------------------
          Call the handler to do upsizing for YUV420 Frame Pack
      ------------------------------------------------------------------*/
      return( ipl2_upsize_YCrCb420lp( i_img_ptr, o_img_ptr, i_param ) );
      /*NOTREACHED*/
    //break;

    default:
      return( IPL_FAILURE );
      /*NOTREACHED*/
    //break;
  }

} /* end of function ipl2_handle_upSize */

/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCBCR2RGB()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr based image to RGB
  image - larger in size. The image is multiplied by 2 in dimensions
   (both Widh and height). If the Crop params are less than the actual muliti
   plied size there will be cropping. This is a combined routine and hence
   will have more CPU optimized performance.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr  - input image frame
  output_img_ptr - Points to the output image
  crop           -  Crop config


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAndCrop_YCbCr2RGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  )
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2;
  register uint8 cb,cr;
  register int32 lumaa1;
  register int32 lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 outDx = o_img_ptr->dx;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint8 *out_row_start; /* for saving start of row */

  uint32 row,col;
  int32 rowInc=0,dest_index,destInc;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAndCrop_YCbCr2RGB\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0]);
      bTable = &(ipl2_bxx5[0]);
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0]);
      bTable = &(ipl2_b444[0]);
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Since we are doing the
    mat on the UpScaled dimensions -- there is no need to do * 2. Its
    already the byte increments needed for each row.
   -----------------------------------------------------------------------*/
  rowInc = (i_img_ptr->dx * 2 - i_param->crop.dx );


  outputImgPtr = (uint16 *) ((uint32)outputImgPtr +  dest_index);
  destInc = ( o_img_ptr->dx  - i_param->crop.dx) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for ( row = i_param->crop.dy/2; row; row-- )
  {
    /*----------------------------------------------------------------------
                This is the row we will create by dumping pixels.
    ----------------------------------------------------------------------*/
    outRawDump = ( outputImgPtr + outDx);
    out_row_start = (uint8 *)outputImgPtr;

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dx/4; col; col-- )
    {
      /*--------------------------------------------------------------------
                  This is Cb
      --------------------------------------------------------------------*/
      cb = (*((uint8*)(inputImgPtr++)));

      /*--------------------------------------------------------------------
                  Next Byte is luma of first pixel
       -------------------------------------------------------------------*/
      lumaa1 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
              Next byte is cr
      --------------------------------------------------------------------*/
      cr = (*((uint8*) (inputImgPtr++)));

      /*--------------------------------------------------------------------
           get the next luma value
       -------------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb, r, out, out2, \
        rTable, gTable, bTable);

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out;

      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out2;
      *outputImgPtr++ = out2;

    } /* End of col loop */

    /*----------------------------------------------------------------------
        Copy the currently formed row to the next one .. Pixel duplication
    ----------------------------------------------------------------------*/
    memcpy(outRawDump, out_row_start, (i_param->crop.dx << 1) );

    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
     ---------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc);

    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
     ---------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr + destInc) + outDx;

  } /* End of row loop */

  return(IPL_SUCCESS);
}  /* end of function */





/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCBCR()

DESCRIPTION
  This function uses nearest neighboor to do upsizing.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr  - Input image frame
  output_img_ptr - Points to the output image
  crop           - Crop config


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
API_EXTERN ipl_status_type ipl2_upSizeAndCrop_YCbCr
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  int32 m,n,mv,nv;
  register uint32 j,i;
  register uint8* row_start;
  uint32 destInc;
  uint32 row,col;
  int32 resizeFactorX;
  int32 resizeFactorY;
  register uint8* out_ptr = (uint8*)o_img_ptr->imgPtr;


  // this array tells that what we should add to our image pointers
  // if we expted Cb Y Cr Y (x axis of table) and got Cb Y Cr Y (y axis
  // of table)
  int nn[4][4] = 
  {
    { 0, -1,  -2,  -3},
    { 1,  0,  1,  0},
    { 2,  1,  0, -1},
    { 1,  0,  1,  0}
  };

  MSG_LOW("inside ipl2_upSizeAndCrop_YCbCr\n");

  destInc = ( o_img_ptr->dx  - i_param->crop.dx) *2;

  out_ptr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  if ( (i_img_ptr->dx > o_img_ptr->dx) ||
    (i_img_ptr->dy > o_img_ptr->dy) )
  {
    /*
    ** Strictly an upsize
    */
    return(IPL_FAILURE);
  }

  m = o_img_ptr->dx;
  n = i_img_ptr->dx;
  mv = o_img_ptr->dy;
  nv = i_img_ptr->dy;

  /*
  ** Q7 resize Factors
  */
  // the m>>1 is to help with for rounding 
  resizeFactorX = (n*128 +(m>>1))/m;
  resizeFactorY = (nv*128 + (mv>>1))/mv;


  for (row=0;row < i_param->crop.dy;row++)
  {
    i = (uint16) (((uint32)(row*resizeFactorY))>>7);
    row_start = (i_img_ptr->imgPtr + ( (i * n) << 1 ) );

    for (col=0; col < 2*i_param->crop.dx; col++)
    {
      // get closest Y
      j = (uint16)(((uint32)(col*resizeFactorX) )>>7);
      j += nn[col%4][j%4];
      *out_ptr++ = *(row_start + j);
    } 

    out_ptr = ( uint8 * ) ( (uint8 *) out_ptr + destInc );

  } /* End of Image for_loop */


  return(IPL_SUCCESS);

}  /* end of function */


/*===========================================================================

FUNCTION IPL2_UPSIZEAVERAGEANDCROP_YCBCR2RGB()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr based image to RGB
  image - larger in size. Dumped pixels are neighborhood avearages as
  oppposed to blid repeat. The image is multiplied by 2 in dimensions
   (both Widh and height). If the Crop params are less than the actual muliti
   plied size there will be cropping. This is a combined routine and hence
   will have more CPU optimized performance.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr  - input image frame
  output_img_ptr - Points to the output image
  crop           -  Crop config


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  )
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cr, cb, r_factor;
  register int32 lumaa1, lumaa3;
  register int32 lumaa2, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 outDx = o_img_ptr->dx;
  int32    inDx = ( i_img_ptr->dx ) * 2;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint32 row,col;
  int32 rowInc=0, dest_index, destInc;

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCbCr2RGB\n");
  /*-----------------------------------------------------------------------*/

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Since we are doing the
    mat on the UpScaled dimensions -- there is no need to do * 2. Its
    already the byte increments needed for each row.
   -----------------------------------------------------------------------*/
  rowInc = (i_img_ptr->dx * 2 - i_param->crop.dx );


  outputImgPtr = (uint16 *) ((uint32)outputImgPtr +  dest_index);
  destInc = ( o_img_ptr->dx  - i_param->crop.dx) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for ( row = i_param->crop.dy/2; row; row-- )
  {
    /*----------------------------------------------------------------------
                This is the row we will create by dumping pixels.
    ----------------------------------------------------------------------*/
    outRawDump = ( outputImgPtr + outDx);

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dx/4; col; col-- )
    {

      /*--------------------------------------------------------------------
                  This is Cb
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*)(inputImgPtr++)));

      /*--------------------------------------------------------------------
                  Next Byte is luma of first pixel
       -------------------------------------------------------------------*/
      lumaa1 = *((uint8*)(inputImgPtr));

      /*--------------------------------------------------------------------
          Pickup candidate for neighbor hood avearage
      --------------------------------------------------------------------*/
      lumaa3 = *((uint8*)(inputImgPtr++ + inDx));

      /*--------------------------------------------------------------------
              Next byte is cr
      --------------------------------------------------------------------*/
      cr = (int32) (*((uint8*) (inputImgPtr++)));

      /*--------------------------------------------------------------------
           get the next luma value
       -------------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr));

      /*--------------------------------------------------------------------
          Pickup candidate for neighbor hood avearage
      --------------------------------------------------------------------*/
      lumaa4 = *((uint8*)(inputImgPtr + inDx));

      lumaa2 = (lumaa1 + lumaa2)  >> 1;
      lumaa4 = (lumaa2 + lumaa4) >> 1;
      lumaa3 = (lumaa1 + lumaa3)  >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );
      /*--------------------------------------------------------------------
          Store all 4
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;
      *outRawDump++ = out3;
      *outRawDump++  = out4;
      /*--------------------------------------------------------------------
          Get the next 4 pixels
      --------------------------------------------------------------------*/
      lumaa1 = *( (uint8 *) inputImgPtr );
      lumaa2 = *((uint8*)(inputImgPtr + 2));
      lumaa3 = *((uint8*)(inputImgPtr + inDx));
      lumaa4 = *((uint8*)(inputImgPtr++  + inDx + 2));


      lumaa2 = (lumaa1 + lumaa2)  >> 1;
      lumaa3 = (lumaa1 + lumaa3)  >> 1;
      lumaa4 = (lumaa2 + lumaa4) >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );
      /*--------------------------------------------------------------------
          Store all 4
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;
      *outRawDump++ = out3;
      *outRawDump++  = out4;

    } /* End of col loop */

    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc);

    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr + destInc) + outDx;

  } /* End of row loop */

  return(IPL_SUCCESS);

}/* end of function ipl2_upSizeAverageAndCrop_YCbCr2RGB */

/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCBCR420TORGB()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
extern ipl_status_type ipl2_upSizeRowAvgAndCrop_YCbCr2RGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  )
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register uint8 cb,cr;
  register int32 lumaa1;
  register int32 lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint8 *out_row_start; /* for saving start of row */
  register uint32 row,col;
  int32 rowInc=0,dest_index,destInc;

  MSG_LOW("inside ipl2_upSizeRowAvgAndCrop_YCbCr2RGB\n");
  /*-----------------------------------------------------------------------*/

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Since we are doing the
    mat on the UpScaled dimensions -- there is no need to do * 2. Its
    already the byte increments needed for each row.
   -----------------------------------------------------------------------*/
  rowInc = (i_img_ptr->dx * 2 - i_param->crop.dx );


  outputImgPtr = (uint16 *) ((uint32)outputImgPtr +  dest_index);
  destInc = ( o_img_ptr->dx  - i_param->crop.dx) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for ( row = i_param->crop.dy/2; row; row-- )
  {
    /*----------------------------------------------------------------------
                This is the row we will create by dumping pixels.
    ----------------------------------------------------------------------*/
    outRawDump = ( outputImgPtr + outDx);
    out_row_start = (uint8 *)outputImgPtr;

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dx/4 - 1; col; col-- )
    {
      /*--------------------------------------------------------------------
                  This is Cb
      --------------------------------------------------------------------*/
      cb = (*((uint8*)(inputImgPtr++)));

      /*--------------------------------------------------------------------
                  Next Byte is luma of first pixel
       -------------------------------------------------------------------*/
      lumaa1 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
              Next byte is cr
      --------------------------------------------------------------------*/
      cr = (*((uint8*) (inputImgPtr++)));

      /*--------------------------------------------------------------------
           get the next luma value
       -------------------------------------------------------------------*/
      lumaa3 = *((uint8*)(inputImgPtr++));

      lumaa2 = (lumaa1 + lumaa3) >> 1;

      lumaa4 = (lumaa3 + *(inputImgPtr + 1) ) >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3,
        lumaa4, cb, cr,  r, out, out2, out3, out4,  rTable, gTable, bTable );


      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;

      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;

    } /* End of col loop */

    /*----------------------------------------------------------------------
      We cant properly average last pixel. So we need to keep it out of the
      loop.
    ----------------------------------------------------------------------*/
    /*----------------------------------------------------------------------
                This is Cb
    ----------------------------------------------------------------------*/
    cb = (*((uint8*)(inputImgPtr++)));

    /*--------------------------------------------------------------------
                Next Byte is luma of first pixel
     -------------------------------------------------------------------*/
    lumaa1 = *((uint8*)(inputImgPtr++));

    /*----------------------------------------------------------------------
            Next byte is cr
    ----------------------------------------------------------------------*/
    cr = (*((uint8*) (inputImgPtr++)));

    /*----------------------------------------------------------------------
         get the next luma value
     ---------------------------------------------------------------------*/
    lumaa3 = *((uint8*)(inputImgPtr++));

    lumaa2 = (lumaa1 + lumaa3) >> 1;

    lumaa4 = lumaa3;

    /*----------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    ----------------------------------------------------------------------*/
    IPL2_CONVERT_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3,
      lumaa4, cb, cr,  r, out, out2, out3, out4,  rTable, gTable, bTable );


    /*----------------------------------------------------------------------
        Dump the first RGB values into the 3 neighboring spots
        as shown below
        IX
        XX
        the I is pixel from input vector and Xs are the dumped pixels
     ---------------------------------------------------------------------*/
    *outputImgPtr++ = out;
    *outputImgPtr++ = out2;

    /*----------------------------------------------------------------------
         Now dump the second set of pixels using the same scheme
    ----------------------------------------------------------------------*/
    *outputImgPtr++ = out3;
    *outputImgPtr++ = out4;


    /*----------------------------------------------------------------------
        Copy the currently formed row to the next one .. Pixel duplication
    ----------------------------------------------------------------------*/
    memcpy(outRawDump, out_row_start, (i_param->crop.dx << 1) );

    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc);

    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/

    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr + destInc) + outDx;

  } /* End of row loop */

  return(IPL_SUCCESS);

} /* end of function ipl2_upSizeRowAvgAndCrop_YCbCr2RGB */

/*===========================================================================

FUNCTION IPL2_UPSIZE_YCBCR420()

DESCRIPTION
    Function which handles the upsizing of an existing existing YCBCR420
    image and positioning it into an o/p buffer

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image
    i_param   - upsize params

ARGUMENTS IN
    o_img_ptr - o/p image


RETURN VALUE
Success or Failure

SIDE EFFECTS
None

===========================================================================*/

extern  ipl_status_type ipl2_upsize_YCbCr420
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  )
{
  MSG_LOW("inside ipl2_upsize_YCbCr420\n");
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      We only support RGB565/RGB444/666 as an o/p format
  ------------------------------------------------------------------------*/
  if ( ! ( ( o_img_ptr->cFormat == IPL_RGB565 ) ||
    ( o_img_ptr->cFormat == IPL_RGB444 ) ||
    ( o_img_ptr->cFormat == IPL_RGB666  ) ) )
  {

    /*----------------------------------------------------------------------
        Log a message and exit
    ----------------------------------------------------------------------*/
    IPL2_MSG_FATAL( "ipl2_upSizeYCbCr442 failed for unsupported o/p /\
                  color   = %d", o_img_ptr->cFormat );

  }

  /*------------------------------------------------------------------------
      switch on quality value desired
  ------------------------------------------------------------------------*/
  switch ( i_param->qual )
  {

    case IPL_QUALITY_LOW :
      return( ipl2_upSizeAndCrop_YCbCr420ToRGB( i_img_ptr,
        o_img_ptr,
        i_param ) );
      /*NOTREACHED*/
      //break;

    case IPL_QUALITY_MEDIUM :
      return( ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB( i_img_ptr,
        o_img_ptr,
        i_param ) );

      /*NOTREACHED*/
      //break;

    case IPL_QUALITY_HIGH :
      /*--------------------------------------------------------------------
        We will call the medium for high as well. We need to revamp the
        algorithm for crisper images.
      --------------------------------------------------------------------*/
      return( ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB( i_img_ptr,
        o_img_ptr,
        i_param ) );

      /*NOTREACHED*/
      //break;

    default:
      return( IPL_FAILURE );
      //break;


  } /* end of switch */

  /*NOTREACHED*/
  return( IPL_SUCCESS );

} /* end of function ipl2_upsize_YCbCr442 */



/*===========================================================================

FUNCTION IPL2_UPSIZE_YCRCB420lp()

DESCRIPTION
    Function which handles the upsizing of an existing existing YCBCR420
    line pack image and positioning it into an o/p buffer

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image
    i_param   - upsize params

ARGUMENTS IN
    o_img_ptr - o/p image


RETURN VALUE
Success or Failure

SIDE EFFECTS
None

===========================================================================*/

extern  ipl_status_type ipl2_upsize_YCrCb420lp
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  )
{

  MSG_LOW("inside ipl2_upsize_YCrCb420lp\n");
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      We only support RGB565/RGB444/666 as an o/p format
  ------------------------------------------------------------------------*/
  if ( ! ( ( o_img_ptr->cFormat == IPL_RGB565 ) ||
    ( o_img_ptr->cFormat == IPL_RGB444 ) ||
    ( o_img_ptr->cFormat == IPL_RGB666  ) ) )
  {

    /*----------------------------------------------------------------------
        Log a message and exit
    ----------------------------------------------------------------------*/
    IPL2_MSG_FATAL( "ipl2_upSizeYCrCb420lp failed for unsupported o/p /\
                  color   = %d", o_img_ptr->cFormat );

  }

  /*------------------------------------------------------------------------
      switch on quality value desired
  ------------------------------------------------------------------------*/
  switch ( i_param->qual )
  {

    case IPL_QUALITY_LOW :
    case IPL_QUALITY_MEDIUM :
      return(ipl2_upSizeAndCrop_YCrCb420lpToRGB(i_img_ptr,o_img_ptr,i_param));
      //break;
    case IPL_QUALITY_HIGH :
      return(ipl2_upSizeRowAvgAndCrop_YCrCb420lpToRGB(i_img_ptr, o_img_ptr,
        i_param ) );
      //break;
    default:
      return( IPL_FAILURE );
      //break;
  } /* end of switch */

  /*NOTREACHED*/
  return( IPL_SUCCESS );

} 



/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCBCR420TORGB()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
extern ipl_status_type ipl2_upSizeAndCrop_YCbCr420ToRGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  )
{

  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register uint8 cb,cr;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint8 *out_row_start; /* for saving start of row */
  register uint8 *cb_ptr;
  uint32 rowInc = 0, destInc, cr_offset;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  int32 dest_index;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAndCrop_YCbCr420ToRGB\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Crop.dx is in terms of
    upscaled dimensions.
   -----------------------------------------------------------------------*/
  rowInc = (i_img_ptr->dx  - (i_param->crop.dx / 2) );


  outputImgPtr = (uint16 *) ( (uint32)outputImgPtr +  dest_index );
  destInc = ( o_img_ptr->dx  - i_param->crop.dx ) * 2;

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cb_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
      Takes us to the offset for Cb Vals
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy  ) >> 2 /* /4 */;


  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for ( row = i_param->crop.dy/4; row; row-- )
  {
    /*----------------------------------------------------------------------
                This is the row we will create
                for adding the second luamma block in YUV420 packing
    ----------------------------------------------------------------------*/
    outRawDump = ( outputImgPtr + 2 * outDx );
    out_row_start = (uint8 *) outputImgPtr;

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dx/4; col; col-- )
    {

      /*--------------------------------------------------------------------
          Take Luma of all four pixels. The Lumma Packing is as shown
          below

          |Y1|Y2|             (this processing)   |RGB1|RGB2|
                ---Cb, Cr  ------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
      lumaa2 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

      /*--------------------------------------------------------------------
            Following line gets you Y3 and then base increments on
            inputImgPtr .. Hopefully
      --------------------------------------------------------------------*/
      lumaa3 = *(inputImgPtr++ + input_row_size);
      lumaa4 = *( inputImgPtr++ + input_row_size);

      /*--------------------------------------------------------------------
          Cb is for all the 4  Yavals
      --------------------------------------------------------------------*/
      cb = *(cb_ptr);

      /*--------------------------------------------------------------------
          Get the Cr for all 4 Y vals
      --------------------------------------------------------------------*/
      cr = *(cb_ptr + cr_offset);

      /*--------------------------------------------------------------------
          We got both CB and Cr for this iteration we offset cb_ptr.
          If you look at the pics you can see that they both move in
          tandem
      --------------------------------------------------------------------*/
      ++cb_ptr;


      /*--------------------------------------------------------------------
          The following is macro which replces a repetitive
          block of code which takes 4 lummas and a Cr/Cb combination
          to convert it to a RGB flavor with out and out 2 in two words.
          cannot be used for RGB666. because 2 pixels cant fit in one
          word
                                                  (out)
          Y | Y           (this macro     RGB444/565 | RGB444/565
                   + CB/CR ----------->
          Y | Y                           RGB444/565 | RGB444/565
                                                   (out2)
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3,
        lumaa4, cb, cr,  r, out,
        out2, out3, out4,
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out;


      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out2;
      *outputImgPtr++ = out2;

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *(outRawDump++ ) = (uint16) out3;
      *(outRawDump++ ) = (uint16) out3;
      *(outRawDump++ ) = (uint16) out4;
      *(outRawDump++ ) = (uint16) out4;

    } /* End of col loop */

    /*----------------------------------------------------------------------
        Copy the currently formed row to the next one .. Pixel duplication
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outputImgPtr + destInc ), out_row_start,
      (i_param->crop.dx << 1) );

    /*----------------------------------------------------------------------
        Copy 3rd row to 4 th
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outRawDump + destInc ),
      ( uint16 *) ( ( uint8 *) outputImgPtr + destInc )
      + outDx, (i_param->crop.dx << 1) );


    inputImgPtr += (uint32) (rowInc  + input_row_size);

    /*----------------------------------------------------------------------
      Remember each four pixels has same CB/CR combination. So we need
      to add number of pixels offset divided by 4

      YY
      -----------------maps to one cr/cb --> cr/cb
      YY

    ----------------------------------------------------------------------*/
    cb_ptr += (rowInc) >> 1 /* divided by 2 */;

    /*----------------------------------------------------------------------
      We need to offset the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
     ---------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ( (uint32 )outRawDump + destInc) +
                   ( outDx  );

  } /* End of row loop */

  return(IPL_SUCCESS);

} /* end of function ipl2_upSizeAndCrop_YCbCr420ToRGB */



/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCRCB420LPTORGB()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCrCb420lp based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
extern ipl_status_type ipl2_upSizeAndCrop_YCrCb420lpToRGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  )
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register uint8 cb,cr;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint8 *out_row_start; /* for saving start of row */
  register uint8 *cr_ptr;
  uint32 rowInc = 0, destInc;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  int32 dest_index;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAndCrop_YCrCb420lpToRGB\n");

  // cropping not currently supported
  if ((i_param->crop.x != 0) ||
      (i_param->crop.y != 0) ||
      (i_param->crop.dx != o_img_ptr->dx) ||
      (i_param->crop.dy != o_img_ptr->dy))
      return( IPL_FAILURE );

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Crop.dx is in terms of
    upscaled dimensions.
   -----------------------------------------------------------------------*/
  rowInc = (i_img_ptr->dx  - (i_param->crop.dx / 2) );


  outputImgPtr = (uint16 *) ( (uint32)outputImgPtr +  dest_index );
  destInc = ( o_img_ptr->dx  - i_param->crop.dx ) * 2;

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cr_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for ( row = i_param->crop.dy/4; row; row-- )
  {
    /*----------------------------------------------------------------------
                This is the row we will create
                for adding the second luamma block in YUV420 packing
    ----------------------------------------------------------------------*/
    outRawDump = ( outputImgPtr + 2 * outDx );
    out_row_start = (uint8 *) outputImgPtr;

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dx/4; col; col-- )
    {

      /*--------------------------------------------------------------------
          Take Luma of all four pixels. The Lumma Packing is as shown
          below

          |Y1|Y2|             (this processing)   |RGB1|RGB2|
                ---Cb, Cr  ------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
      lumaa2 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

      /*--------------------------------------------------------------------
            Following line gets you Y3 and then base increments on
            inputImgPtr .. Hopefully
      --------------------------------------------------------------------*/
      lumaa3 = *(inputImgPtr++ + input_row_size);
      lumaa4 = *( inputImgPtr++ + input_row_size);

      /*--------------------------------------------------------------------
          Cb is for all the 4  Yavals
      --------------------------------------------------------------------*/
      cr = *(cr_ptr++);

      /*--------------------------------------------------------------------
          Get the Cr for all 4 Y vals
      --------------------------------------------------------------------*/
      cb = *(cr_ptr++);

      /*--------------------------------------------------------------------
          We got both CB and Cr for this iteration we offset cb_ptr.
          If you look at the pics you can see that they both move in
          tandem
      --------------------------------------------------------------------*/
      //cb_ptr += 2;


      /*--------------------------------------------------------------------
          The following is macro which replces a repetitive
          block of code which takes 4 lummas and a Cr/Cb combination
          to convert it to a RGB flavor with out and out 2 in two words.
          cannot be used for RGB666. because 2 pixels cant fit in one
          word
                                                  (out)
          Y | Y           (this macro     RGB444/565 | RGB444/565
                   + CB/CR ----------->
          Y | Y                           RGB444/565 | RGB444/565
                                                   (out2)
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3,
        lumaa4, cb, cr,  r, out,
        out2, out3, out4,
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out;


      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out2;
      *outputImgPtr++ = out2;

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *(outRawDump++ ) = (uint16) out3;
      *(outRawDump++ ) = (uint16) out3;
      *(outRawDump++ ) = (uint16) out4;
      *(outRawDump++ ) = (uint16) out4;

    } /* End of col loop */

    /*----------------------------------------------------------------------
        Copy the currently formed row to the next one .. Pixel duplication
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outputImgPtr + destInc ), out_row_start,
      (i_param->crop.dx << 1) );

    /*----------------------------------------------------------------------
        Copy 3rd row to 4 th
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outRawDump + destInc ),
      ( uint16 *) ( ( uint8 *) outputImgPtr + destInc )
      + outDx, (i_param->crop.dx << 1) );


    inputImgPtr += (uint32) (rowInc  + input_row_size);

    /*----------------------------------------------------------------------
      Remember each four pixels has same CB/CR combination. So we need
      to add number of pixels offset divided by 4

      YY
      -----------------maps to one cr/cb --> cr/cb
      YY

    ----------------------------------------------------------------------*/
    cr_ptr += (rowInc) >> 0 /* divided by 2 */;

    /*----------------------------------------------------------------------
      We need to offset the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
     ---------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ( (uint32 )outRawDump + destInc) +
                   ( outDx  );

  } /* End of row loop */

  return(IPL_SUCCESS);

} 


/*===========================================================================

FUNCTION ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot90

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2.5 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot90
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  unsigned char* inputImgPtrStart, * cbPtrStart;
  uint16* outputImgPtrStart;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb1, cr1, r_factor;
  register int32 cb2, cr2;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4, lumaa5;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  register uint8 *cb_ptr;
  uint32 input_row_size, cr_offset;
  uint32 row,col;


  MSG_LOW("inside ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot90\n");
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  input_row_size = i_img_ptr->dx;

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);
  outputImgPtrStart = outputImgPtr;

  // setup our Cb and Cr pointer for this frame pack data
  inputImgPtr = i_img_ptr->imgPtr + (i_img_ptr->dx * (i_img_ptr->dy - 1));
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy) >> 2 /* /4 */;
  cb_ptr = i_img_ptr->clrPtr + cr_offset - (i_img_ptr->dx / 2);


  // we do 5 output lines at a time
  for (row = 0; row < i_param->crop.dx; row += 5)
  {
    // setup the pointers to row 2 and 3 which are duplicates of 1st row.
    outputImgPtr = (outputImgPtrStart + row);

    // initialze our pointers which hold our place so we can come back
    inputImgPtrStart = inputImgPtr;
    cbPtrStart = cb_ptr;
    
    for (col = 0; col < i_param->crop.dy; col += 10)
    {
      // Do first set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr + 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr + 2) + lumaa1) >> 1;  
      inputImgPtr += 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 1;

      // convert to rgb565
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      *(outputImgPtr+2)= out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      *(outputImgPtr+2)= out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      *(outputImgPtr+2)= out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      *(outputImgPtr+2)= out4;
      outputImgPtr += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      *(outputImgPtr+2)= out;
      outputImgPtr += outDx;


      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr + 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr + 2) + lumaa1) >> 1;  
      inputImgPtr += 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 1;

      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      *(outputImgPtr+2)= out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      *(outputImgPtr+2)= out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      *(outputImgPtr+2)= out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      *(outputImgPtr+2)= out4;
      outputImgPtr += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      *(outputImgPtr+2)= out;
      outputImgPtr += outDx;
    }

    // prepare where we are going to write the next two output lines
    outputImgPtr = (outputImgPtrStart + row+3);

    // setup the pointers to write out rows 4 and 5. We need to move the input
    // Cb back to where it was.
    inputImgPtr = inputImgPtrStart - input_row_size;
    cb_ptr = cbPtrStart;

    // now take next row of Ys and make two lines out of it
    for (col = 0; col < i_param->crop.dy; col += 10)
    {
      // Do first set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr + 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr + 2) + lumaa1) >> 1;  
      inputImgPtr += 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 1;

      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      // only duplicate this row once (not twice like above)
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      outputImgPtr += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += outDx;


      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr + 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr + 2) + lumaa1) >> 1;  
      inputImgPtr += 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 1;

      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      outputImgPtr += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += outDx;
    }

    inputImgPtr = inputImgPtrStart - 2*input_row_size;
    cb_ptr = cbPtrStart - input_row_size/2;
  } 


  return(IPL_SUCCESS);
} 



/*===========================================================================

FUNCTION ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot270

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2.5 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot270
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  unsigned char* inputImgPtrStart, * cbPtrStart;
  uint16* outputImgPtrStart;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb1, cr1, r_factor;
  register int32 cb2, cr2;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4, lumaa5;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  register uint8 *cb_ptr;
  uint32 input_row_size, cr_offset;
  uint32 row,col;


  MSG_LOW("inside ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot270\n");
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  input_row_size = i_img_ptr->dx;

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);
  outputImgPtrStart = outputImgPtr;

  // setup our Cb and Cr pointer for this frame pack data
  inputImgPtr = i_img_ptr->imgPtr + i_img_ptr->dx - 1;
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy) >> 2 /* /4 */;
  cb_ptr = i_img_ptr->clrPtr + (input_row_size / 2 - 1);

  // we do 5 output lines at a time
  for (row = 0; row < i_param->crop.dx; row += 5)
  {
    // setup the pointers to row 2 and 3 which are duplicates of 1st row.
    outputImgPtr = (outputImgPtrStart + row);

    // initialze our pointers which hold our place so we can come back
    inputImgPtrStart = inputImgPtr;
    cbPtrStart = cb_ptr;
    
    for (col = 0; col < i_param->crop.dy; col += 10)
    {
      // Do first set of 2 -> 5 pixels 
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr - 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr - 2) + lumaa1) >> 1;  
      inputImgPtr -= 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 1;

      // convert to rgb565
      //cb1 = cb2 = cr1 = cr2 = 128;
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      *(outputImgPtr+2)= out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      *(outputImgPtr+2)= out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      *(outputImgPtr+2)= out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      *(outputImgPtr+2)= out4;
      outputImgPtr += outDx;

      //cb1 = cb2 = cr1 = cr2 = 128;
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      *(outputImgPtr+2)= out;
      outputImgPtr += outDx;


      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr - 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr - 2) + lumaa1) >> 1;  
      inputImgPtr -= 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 1;

      //cb1 = cb2 = cr1 = cr2 = 128;
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      *(outputImgPtr+2)= out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      *(outputImgPtr+2)= out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      *(outputImgPtr+2)= out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      *(outputImgPtr+2)= out4;
      outputImgPtr += outDx;

      //cb1 = cb2 = cr1 = cr2 = 128;
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      *(outputImgPtr+2)= out;
      outputImgPtr += outDx;
    }

    // prepare where we are going to write the next two output lines
    outputImgPtr = (outputImgPtrStart + row+3);

    // setup the pointers to write out rows 4 and 5. We need to move the input
    // Cb back to where it was.
    inputImgPtr = inputImgPtrStart + input_row_size;
    cb_ptr = cbPtrStart;

    // now take next row of Ys and make two lines out of it
    for (col = 0; col < i_param->crop.dy; col += 10)
    {
      // Do first set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr - 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr - 2) + lumaa1) >> 1;  
      inputImgPtr -= 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 1;

      //cb1 = cb2 = cr1 = cr2 = 128;
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      // only duplicate this row once (not twice like above)
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      outputImgPtr += outDx;

      //cb1 = cb2 = cr1 = cr2 = 128;
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += outDx;


      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr - 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr - 2) + lumaa1) >> 1;  
      inputImgPtr -= 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 1;

      //cb1 = cb2 = cr1 = cr2 = 128;
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      outputImgPtr += outDx;

      //cb1 = cb2 = cr1 = cr2 = 128;
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += outDx;
    }

    inputImgPtr = inputImgPtrStart + 2*input_row_size;
    cb_ptr = cbPtrStart + input_row_size/2;
  } 


  return(IPL_SUCCESS);
} 




/*===========================================================================

FUNCTION ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot180

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2.5 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot180
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  unsigned char* inputImgPtrStart, * cbPtrStart;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb1, cr1, r_factor;
  register int32 cb2, cr2;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4, lumaa5;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  register uint8 *cb_ptr;
  uint16 *outRawDump;  /* For every row there is an identical row dumped */
  uint16 *outRawDump2; /* For every row there is an identical row dumped */
  uint32 input_row_size, rowInc, destInc, cr_offset;
  uint32 row,col;


  MSG_LOW("inside ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot180\n");
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  input_row_size = i_img_ptr->dx;

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  // setup our Cb and Cr pointer for this frame pack data
  inputImgPtr = i_img_ptr->imgPtr + (i_img_ptr->dx * i_img_ptr->dy - 1);
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy) >> 2 /* /4 */;
  cb_ptr = i_img_ptr->clrPtr + (cr_offset - 1);


  // For every 10 output pixels, we use 4 input pixels. We can only
  // output multiple of 10. Because of this, figure out how
  // many pixel on the input we need to skip per row
  rowInc = (i_img_ptr->dx  - (4*(i_param->crop.dx/10)));

  // see how many pixel on the output we need to skip per row
  destInc = (o_img_ptr->dx  - 10*(i_param->crop.dx/10));

  // we do 5 output lines at a time
  for (row = 0; row < i_param->crop.dy; row += 5)
  {
    // setup the pointers to row 2 and 3 which are duplicates of 1st row.
    outRawDump = ( outputImgPtr +  outDx);
    outRawDump2 = ( outputImgPtr + 2*outDx);

    // initialze our pointers which hold our place so we can come back
    inputImgPtrStart = inputImgPtr;
    cbPtrStart = cb_ptr;
    
    for (col = 0; col < i_param->crop.dx; col += 10)
    {
      // Do first set of 2 -> 5 pixels 
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr - 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr - 2) + lumaa1) >> 1;  
      inputImgPtr -= 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 1;

      // convert to rgb565
      // write 5 pixel, dupliacted for 3 rows
      //cb1 = cb2 = cr1 = cb2 = 128;
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr++ = out;
      *outRawDump++ = out;
      *outRawDump2++ = out;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;
      *outRawDump2++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;
      *outRawDump2++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;
      *outRawDump2++ = out4;

      //cb1 = cb2 = cr1 = cb2 = 128;
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;
      *outRawDump2++ = out;


      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr - 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr - 2) + lumaa1) >> 1;  
      inputImgPtr -= 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 1;


      //cb1 = cb2 = cr1 = cb2 = 128;
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr++ = out;
      *outRawDump++ = out;
      *outRawDump2++ = out;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;
      *outRawDump2++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;
      *outRawDump2++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;
      *outRawDump2++ = out4;


      //cb1 = cb2 = cr1 = cb2 = 128;
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;
      *outRawDump2++ = out;
    }

    // setup the pointers to write out rows 4 and 5. We need to move the input
    // Cb back to where it was.
    outputImgPtr += (destInc + 2*outDx);
    outRawDump = (outputImgPtr + outDx);

    inputImgPtr -= rowInc;
    cb_ptr += (2*(i_param->crop.dx/10));

    // now take next row of Ys and make two lines out of it
    for (col = 0; col < i_param->crop.dx; col += 10)
    {
      // Do first set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr - 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr - 2) + lumaa1) >> 1;  
      inputImgPtr -= 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 1;

      //cb1 = cb2 = cr1 = cb2 = 128;
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      // only duplicate this row once (not twice like above)
      *outputImgPtr++ = out;
      *outRawDump++ = out;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;

      //cb1 = cb2 = cr1 = cb2 = 128;
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;


      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr - 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr - 2) + lumaa1) >> 1;  
      inputImgPtr -= 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 1;

      //cb1 = cb2 = cr1 = cb2 = 128;
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr++ = out;
      *outRawDump++ = out;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;

      //cb1 = cb2 = cr1 = cb2 = 128;
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;
    }

    inputImgPtr = inputImgPtrStart - 2*input_row_size;
    cb_ptr = cbPtrStart - input_row_size/2;


    // skip a row since we just dumped two
    outputImgPtr += (destInc + outDx);
  } 


  return(IPL_SUCCESS);
} 



/*===========================================================================

FUNCTION ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2.5 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb1, cr1, r_factor;
  register int32 cb2, cr2;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4, lumaa5;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint16 *outRawDump;  /* For every row there is an identical row dumped */
  uint16 *outRawDump2; /* For every row there is an identical row dumped */
  register uint8 *cb_ptr;
  uint32 rowInc, destInc, cr_offset;
  uint32 row,col;


  MSG_LOW("inside ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB\n");
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  // setup our Cb and Cr pointer for this frame pack data
  cb_ptr = i_img_ptr->clrPtr;
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy) >> 2 /* /4 */;

  // For every 10 output pixels, we use 4 input pixels. We can only
  // output multiple of 10. Because of this, figure out how
  // many pixel on the input we need to skip per row
  rowInc = (i_img_ptr->dx  - (4*(i_param->crop.dx/10)));

  // see how many pixel on the output we need to skip per row
  destInc = (o_img_ptr->dx  - 10*(i_param->crop.dx/10));

  // we do 5 output lines at a time
  for ( row = i_param->crop.dy/5; row; row-- )
  {
    // setup the pointers to row 2 and 3 which are duplicates of 1st row.
    outRawDump = ( outputImgPtr +  1*outDx);
    outRawDump2 = ( outputImgPtr + 2*outDx);

    for (col = i_param->crop.dx/10; col; col-- )
    {
      // Do first set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr + 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr + 2) + lumaa1) >> 1;  
      inputImgPtr += 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 1;

      // convert to rgb565
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr++ = out;
      *outRawDump++ = out;
      *outRawDump2++ = out;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;
      *outRawDump2++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;
      *outRawDump2++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;
      *outRawDump2++ = out4;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;
      *outRawDump2++ = out;


      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr + 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr + 2) + lumaa1) >> 1;  
      inputImgPtr += 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 1;

      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr++ = out;
      *outRawDump++ = out;
      *outRawDump2++ = out;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;
      *outRawDump2++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;
      *outRawDump2++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;
      *outRawDump2++ = out4;


      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;
      *outRawDump2++ = out;
    }

    // setup the pointers to write out rows 4 and 5. We need to move the input
    // Cb back to where it was.
    outputImgPtr += (destInc + 2*outDx);
    outRawDump = (outputImgPtr + outDx);

    inputImgPtr += rowInc;
    cb_ptr -= (2*(i_param->crop.dx/10));


    // now take next row of Ys and make two lines out of it
    for ( col = i_param->crop.dx/10 - 0; col; col-- )
    {
      // Do first set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr + 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr + 2) + lumaa1) >> 1;  
      inputImgPtr += 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 1;

      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      // only duplicate this row once (not twice like above)
      *outputImgPtr++ = out;
      *outRawDump++ = out;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;


      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;



      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      lumaa1 = *inputImgPtr;  
      lumaa4 = *(inputImgPtr + 1);  
      lumaa2 = (lumaa1 + lumaa1 + lumaa4)/3;     
      lumaa3 = (lumaa1 + lumaa4 + lumaa4)/3;     
      lumaa5 = (*(inputImgPtr + 2) + lumaa1) >> 1;  
      inputImgPtr += 2;  

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);

      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 1;

      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr++ = out;
      *outRawDump++ = out;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;


      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;
    }


    // skip a row since we just dumped two
    outputImgPtr += (destInc + outDx);

    // skip a row on the input pointers
    inputImgPtr += rowInc;

    // we dont need to skip Cb ptr
    cb_ptr += ((rowInc) >> 1);
  } 


  return(IPL_SUCCESS);
} 


/*===========================================================================

FUNCTION ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 1.5x in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb1, cr1, r_factor;
  register int32 cb2, cr2;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4, lumaa5, lumaa6;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  register uint8 *cb_ptr;
  uint32 rowInc, destInc, cr_offset;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  // figure out where in the output we want to write image
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  // setup pointers to Cb and Cr of input
  cb_ptr = i_img_ptr->clrPtr;
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy) >> 2;

  // For every 6 output pixels, we use 4 input pixels. We can only
  // output multiple of 6. Because of this, figure out how
  // many pixel on the input we need to skip per row
  rowInc = (i_img_ptr->dx  - (4*(i_param->crop.dx/6)));
  destInc = (o_img_ptr->dx  - 6*(i_param->crop.dx/6));



  // we output 3 lines for every 2 input lines red
  for ( row = i_param->crop.dy/3; row; row-- )
  {
    outRawDump = ( outputImgPtr + 1*outDx);

    for (col = i_param->crop.dx/6; col; col-- )
    {
      // make 4 pixels into 6
      lumaa1 = *inputImgPtr;  

      lumaa2 = *(inputImgPtr + 1);  

      lumaa3 = *(inputImgPtr + 2);  
      lumaa5 = *(inputImgPtr + 3);  

      lumaa4 = lumaa3 + lumaa5;     
      lumaa4 >>= 1;

      lumaa6 = (lumaa5 + *(inputImgPtr+4)); 
      lumaa6 >>= 1;

      lumaa2 = lumaa1 + lumaa3;  
      lumaa2 >>= 1;

      lumaa3 += (*(inputImgPtr+1));
      lumaa3 >>= 1;

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);
      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 2;


      // convert to rgb565
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;


      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa6,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;


      // make 4 pixels into 6
      lumaa1 = *(inputImgPtr + input_row_size);  
  
      lumaa2 = *(inputImgPtr + 1 + input_row_size);  

      lumaa3 = *(inputImgPtr + 2 + input_row_size);  
      lumaa5 = *(inputImgPtr + 3 + input_row_size);  

      lumaa4 = lumaa3 + lumaa5;     
      lumaa4 >>= 1;
  
      lumaa6 = (lumaa5 + *(inputImgPtr+ 4 + input_row_size)); 
      lumaa6 >>= 1;

      lumaa2 = lumaa1 + lumaa3;  
      lumaa2 >>= 1;

      lumaa3 += (*(inputImgPtr + 1 + input_row_size));
      lumaa3 >>= 1;

      inputImgPtr += 4;

      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
          lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
          rTable, gTable, bTable );
  
      *(outRawDump + outDx) = (uint16) out;
      *(outRawDump++) = (uint16) out;

      *(outRawDump + outDx) = (uint16) out2;
      *(outRawDump++) = (uint16) out2;

      *(outRawDump + outDx) = (uint16) out3;
      *(outRawDump++) = (uint16) out3;

      *(outRawDump + outDx) = (uint16) out4;
      *(outRawDump++) = (uint16) out4;


      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *(outRawDump + outDx) = (uint16) out;
      *(outRawDump++) = (uint16) out;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa6,cr2,cb2,r,out,rTable,gTable,bTable);
      *(outRawDump + outDx) = (uint16) out;
      *(outRawDump++) = (uint16) out;
    }
    outputImgPtr += (destInc + 2*outDx);

    // skip a row on the input pointers
    inputImgPtr += (uint32) (rowInc  + input_row_size);

    // we dont need to skip Cb ptr
    cb_ptr += ((rowInc) >> 1);

  } 


  return(IPL_SUCCESS);

} 


/*===========================================================================

FUNCTION ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot90

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2.5 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot90
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  unsigned char* inputImgPtrStart, * cbPtrStart;
  uint16* outputImgPtrStart;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register uint16* outRawDump;
  register int32 cb1, cr1, r_factor;
  register int32 cb2, cr2;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4, lumaa5, lumaa6;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  register uint8 *cb_ptr;
  uint32 input_row_size, cr_offset;
  uint32 row,col;
  uint32 rowTodo,colTodo;


  MSG_LOW("inside ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot90\n");
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  input_row_size = i_img_ptr->dx;

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);
  outputImgPtrStart = outputImgPtr;

  // setup our Cb and Cr pointer for this frame pack data
  inputImgPtr = i_img_ptr->imgPtr + (i_img_ptr->dx * (i_img_ptr->dy - 1));
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy) >> 2 /* /4 */;
  cb_ptr = i_img_ptr->clrPtr + cr_offset - (i_img_ptr->dx / 2);



  // we do 3 output lines at a time for every 2 lines
  rowTodo = IPL_MIN(i_param->crop.dy, ((i_img_ptr->dy * 3) / 2)); 
  colTodo = IPL_MIN(i_param->crop.dx, ((i_img_ptr->dx * 3) / 2)); 

  for (row = 0; row < rowTodo; row += 3)
  {
    // setup the pointers to row 2 and 3 which are duplicates of 1st row.
    outputImgPtr = (outputImgPtrStart + row);

    // initialze our pointers which hold our place so we can come back
    inputImgPtrStart = inputImgPtr;
    cbPtrStart = cb_ptr;

    for (col = 0; col < colTodo; col += 6)
    {
      // make 4 pixels into 6
      lumaa1 = *inputImgPtr;
      lumaa2 = *(inputImgPtr + 1);
      lumaa3 = *(inputImgPtr + 2);
      lumaa5 = *(inputImgPtr + 3);

      lumaa4 = lumaa3 + lumaa5;
      lumaa4 >>= 1;

      lumaa6 = (lumaa5 + *(inputImgPtr+4));
      lumaa6 >>= 1;

      lumaa2 = lumaa1 + lumaa3;
      lumaa2 >>= 1;

      lumaa3 += (*(inputImgPtr+1));
      lumaa3 >>= 1;
      inputImgPtr += 4;
     
      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);
      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 2;


      // convert to rgb565
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      outputImgPtr += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      outputImgPtr += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa6,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      outputImgPtr += outDx;
    }

    // prepare where we are going to write the next two output lines
    outputImgPtr = (outputImgPtrStart + row+1);

    // setup the pointers to row 3 which duplicate
    outRawDump = (outputImgPtrStart + row+2);

    // setup the pointers to write out rows 4 and 5. We need to
    // move the input Cb back to where it was.
    inputImgPtr = inputImgPtrStart - input_row_size;
    cb_ptr = cbPtrStart;

    // make 4 pixels into 6, and duplicate it as well
    // duplicate this column
    for (col = 0; col < i_param->crop.dy; col += 6)
    {
      lumaa1 = *inputImgPtr;
      lumaa2 = *(inputImgPtr + 1);
      lumaa3 = *(inputImgPtr + 2);
      lumaa5 = *(inputImgPtr + 3);

      lumaa4 = lumaa3 + lumaa5;
      lumaa4 >>= 1;

      lumaa6 = (lumaa5 + *(inputImgPtr+4));
      lumaa6 >>= 1;

      lumaa2 = lumaa1 + lumaa3;
      lumaa2 >>= 1;

      lumaa3 += (*(inputImgPtr+1));
      lumaa3 >>= 1;
      inputImgPtr += 4;

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr+1);
      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr + 1 + cr_offset);
      cb_ptr += 2;
      

      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      *outRawDump = out;
      outputImgPtr += outDx;
      outRawDump += outDx;

      *outputImgPtr = out2;
      *outRawDump = out2;
      outputImgPtr += outDx;
      outRawDump += outDx;

      *outputImgPtr = out3;
      *outRawDump = out3;
      outputImgPtr += outDx;
      outRawDump += outDx;

      *outputImgPtr = out4;
      *outRawDump = out4;
      outputImgPtr += outDx;
      outRawDump += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *outRawDump = out;
      outputImgPtr += outDx;
      outRawDump += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa6,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *outRawDump = out;
      outputImgPtr += outDx;
      outRawDump += outDx;
    }

    // setup the pointers to write out rows 4 and 5. We need to move the input
    // Cb back to where it was.
    inputImgPtr = inputImgPtrStart - 2*input_row_size;
    cb_ptr = cbPtrStart - input_row_size/2;
  } 


  return(IPL_SUCCESS);
} 



/*===========================================================================

FUNCTION ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot180

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2.5 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot180
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  unsigned char* inputImgPtrStart, * cbPtrStart;
  uint16* outputImgPtrStart;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register uint16* outRawDump;
  register int32 cb1, cr1, r_factor;
  register int32 cb2, cr2;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4, lumaa5, lumaa6;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  register uint8 *cb_ptr;
  uint32 input_row_size, cr_offset;
  uint32 row,col;
  uint32 rowTodo,colTodo;


  MSG_LOW("inside ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot180\n");
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  input_row_size = i_img_ptr->dx;

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);
  outputImgPtrStart = outputImgPtr;

  // setup our Cb and Cr pointer for this frame pack data
  inputImgPtr = i_img_ptr->imgPtr + (i_img_ptr->dx*i_img_ptr->dy - 1);
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy) >> 2 /* /4 */;
  cb_ptr = i_img_ptr->clrPtr + (cr_offset - 1);

  rowTodo = IPL_MIN(i_param->crop.dy, ((i_img_ptr->dy * 3) / 2)); 
  colTodo = IPL_MIN(i_param->crop.dx, ((i_img_ptr->dx * 3) / 2)); 

  // we do 3 output lines at a time for every 2 lines
  for (row = 0; row < rowTodo; row += 3)
  {
    // setup the pointers to row 2 and 3 which are duplicates of 1st row.
    outputImgPtr = (outputImgPtrStart + row*outDx);

    // initialze our pointers which hold our place so we can come back
    inputImgPtrStart = inputImgPtr;
    cbPtrStart = cb_ptr;

    for (col = 0; col < colTodo; col += 6)
    {
      // make 4 pixels into 6
      lumaa1 = *inputImgPtr;
      lumaa2 = *(inputImgPtr - 1);
      lumaa3 = *(inputImgPtr - 2);
      lumaa5 = *(inputImgPtr - 3);

      lumaa4 = lumaa3 + lumaa5;
      lumaa4 >>= 1;

      lumaa6 = (lumaa5 + *(inputImgPtr-4));
      lumaa6 >>= 1;

      lumaa2 = lumaa1 + lumaa3;
      lumaa2 >>= 1;

      lumaa3 += (*(inputImgPtr-1));
      lumaa3 >>= 1;
      inputImgPtr -= 4;
     
      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);
      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 2;


      // convert to rgb565
      //cb1 = cb2 = cr1 = cr2 = 128;
      //lumaa1 = lumaa2 = lumaa3 = lumaa4 = lumaa5 = lumaa6 = 128; 
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;

      //lumaa1 = lumaa2 = lumaa3 = lumaa4 = lumaa5 = lumaa6 = 128; 
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;

      //lumaa1 = lumaa2 = lumaa3 = lumaa4 = lumaa5 = lumaa6 = 128; 
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa6,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
    }

    // prepare where we are going to write the next two output lines
    outputImgPtr = (outputImgPtrStart + (row+1)*outDx);

    // setup the pointers to row 3 which duplicate
    outRawDump = (outputImgPtr + outDx);

    // setup the pointers to write out rows 4 and 5. We need to
    // move the input Cb back to where it was.
    inputImgPtr = inputImgPtrStart - input_row_size;
    cb_ptr = cbPtrStart;

    // make 4 pixels into 6, and duplicate it as well
    // duplicate this column
    for (col = 0; col < i_param->crop.dx; col += 6)
    {
      lumaa1 = *inputImgPtr;
      lumaa2 = *(inputImgPtr - 1);
      lumaa3 = *(inputImgPtr - 2);
      lumaa5 = *(inputImgPtr - 3);

      lumaa4 = lumaa3 + lumaa5;
      lumaa4 >>= 1;

      lumaa6 = (lumaa5 + *(inputImgPtr-4));
      lumaa6 >>= 1;

      lumaa2 = lumaa1 + lumaa3;
      lumaa2 >>= 1;

      lumaa3 += (*(inputImgPtr-1));
      lumaa3 >>= 1;
      inputImgPtr -= 4;

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);
      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 2;
      

      //cb1 = cb2 = cr1 = cr2 = 128;
      //lumaa1 = lumaa2 = lumaa3 = lumaa4 = lumaa5 = lumaa6 = 128; 
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr++ = out;
      *outRawDump++ = out;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;

      //lumaa1 = lumaa2 = lumaa3 = lumaa4 = lumaa5 = lumaa6 = 128; 
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;

      //lumaa1 = lumaa2 = lumaa3 = lumaa4 = lumaa5 = lumaa6 = 128; 
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa6,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr++ = out;
      *outRawDump++ = out;
    }

    // setup the pointers to write out rows 4 and 5. We need to move the input
    // Cb back to where it was.
    inputImgPtr = inputImgPtrStart - 2*input_row_size;
    cb_ptr = cbPtrStart - input_row_size/2;
  } 


  return(IPL_SUCCESS);
} 





/*===========================================================================

FUNCTION ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot270

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2.5 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot270
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  unsigned char* inputImgPtrStart, * cbPtrStart;
  uint16* outputImgPtrStart;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register uint16* outRawDump;
  register int32 cb1, cr1, r_factor;
  register int32 cb2, cr2;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4, lumaa5, lumaa6;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  register uint8 *cb_ptr;
  uint32 input_row_size, cr_offset;
  uint32 row,col;
  uint32 rowTodo,colTodo;


  MSG_LOW("inside ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot270\n");
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  input_row_size = i_img_ptr->dx;

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);
  outputImgPtrStart = outputImgPtr;

  // setup our Cb and Cr pointer for this frame pack data
  inputImgPtr = i_img_ptr->imgPtr + i_img_ptr->dx - 1;
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy) >> 2 /* /4 */;
  cb_ptr = i_img_ptr->clrPtr + (input_row_size / 2 - 1);

  rowTodo = IPL_MIN(i_param->crop.dy, ((i_img_ptr->dy * 3) / 2)); 
  colTodo = IPL_MIN(i_param->crop.dx, ((i_img_ptr->dx * 3) / 2)); 

  // we do 3 output lines at a time for every 2 lines
  for (row = 0; row < rowTodo; row += 3)
  {
    // setup the pointers to row 2 and 3 which are duplicates of 1st row.
    outputImgPtr = (outputImgPtrStart + row);

    // initialze our pointers which hold our place so we can come back
    inputImgPtrStart = inputImgPtr;
    cbPtrStart = cb_ptr;

    for (col = 0; col < colTodo; col += 6)
    {
      // make 4 pixels into 6
      lumaa1 = *inputImgPtr;
      lumaa2 = *(inputImgPtr - 1);
      lumaa3 = *(inputImgPtr - 2);
      lumaa5 = *(inputImgPtr - 3);

      lumaa4 = lumaa3 + lumaa5;
      lumaa4 >>= 1;

      lumaa6 = (lumaa5 + *(inputImgPtr-4));
      lumaa6 >>= 1;

      lumaa2 = lumaa1 + lumaa3;
      lumaa2 >>= 1;

      lumaa3 += (*(inputImgPtr-1));
      lumaa3 >>= 1;
      inputImgPtr -= 4;
     
      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);
      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 2;


      // convert to rgb565
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      outputImgPtr += outDx;

      *outputImgPtr = out2;
      outputImgPtr += outDx;

      *outputImgPtr = out3;
      outputImgPtr += outDx;

      *outputImgPtr = out4;
      outputImgPtr += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      outputImgPtr += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa6,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      outputImgPtr += outDx;
    }

    // prepare where we are going to write the next two output lines
    outputImgPtr = (outputImgPtrStart + row+1);

    // setup the pointers to row 3 which duplicate
    outRawDump = (outputImgPtrStart + row+2);

    // setup the pointers to write out rows 4 and 5. We need to
    // move the input Cb back to where it was.
    inputImgPtr = inputImgPtrStart + input_row_size;
    cb_ptr = cbPtrStart;

    // make 4 pixels into 6, and duplicate it as well
    // duplicate this column
    for (col = 0; col < i_param->crop.dy; col += 6)
    {
      lumaa1 = *inputImgPtr;
      lumaa2 = *(inputImgPtr - 1);
      lumaa3 = *(inputImgPtr - 2);
      lumaa5 = *(inputImgPtr - 3);

      lumaa4 = lumaa3 + lumaa5;
      lumaa4 >>= 1;

      lumaa6 = (lumaa5 + *(inputImgPtr-4));
      lumaa6 >>= 1;

      lumaa2 = lumaa1 + lumaa3;
      lumaa2 >>= 1;

      lumaa3 += (*(inputImgPtr-1));
      lumaa3 >>= 1;
      inputImgPtr -= 4;

      cb1 = *(cb_ptr);
      cb2 = *(cb_ptr-1);
      cr1 = *(cb_ptr + cr_offset);
      cr2 = *(cb_ptr - 1 + cr_offset);
      cb_ptr -= 2;
      

      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb1, cr1,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      *outputImgPtr = out;
      *outRawDump = out;
      outputImgPtr += outDx;
      outRawDump += outDx;

      *outputImgPtr = out2;
      *outRawDump = out2;
      outputImgPtr += outDx;
      outRawDump += outDx;

      *outputImgPtr = out3;
      *outRawDump = out3;
      outputImgPtr += outDx;
      outRawDump += outDx;

      *outputImgPtr = out4;
      *outRawDump = out4;
      outputImgPtr += outDx;
      outRawDump += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa5,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *outRawDump = out;
      outputImgPtr += outDx;
      outRawDump += outDx;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa6,cr2,cb2,r,out,rTable,gTable,bTable);
      *outputImgPtr = out;
      *outRawDump = out;
      outputImgPtr += outDx;
      outRawDump += outDx;
    }

    // setup the pointers to write out rows 4 and 5. We need to move the input
    // Cb back to where it was.
    inputImgPtr = inputImgPtrStart + 2*input_row_size;
    cb_ptr = cbPtrStart + input_row_size/2;
  } 


  return(IPL_SUCCESS);
} 





/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCBCR420TORGB()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
extern ipl_status_type ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb, cr, r_factor;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint8 *out_row_start; /* for saving start of row */
  register uint8 *cb_ptr;
  uint32 rowInc = 0, destInc, cr_offset;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  int32 dest_index;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Crop.dx is in terms of
    upscaled dimensions.
   -----------------------------------------------------------------------*/
  rowInc = (i_img_ptr->dx  - (i_param->crop.dx / 2) );


  outputImgPtr = (uint16 *) ( (uint32)outputImgPtr +  dest_index );
  destInc = ( o_img_ptr->dx  - i_param->crop.dx ) * 2;

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cb_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
      Takes us to the offset for Cb Vals
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy  ) >> 2 /* /4 */;


  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for ( row = i_param->crop.dy/4; row; row-- )
  {
    /*----------------------------------------------------------------------
                This is the row we will create
                for adding the second luamma block in YUV420 packing
    ----------------------------------------------------------------------*/
    outRawDump = ( outputImgPtr + 2 * outDx );
    out_row_start = (uint8 *) outputImgPtr;

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dx/4 - 1; col; col-- )
    {

      /*--------------------------------------------------------------------
          Take Luma of all four pixels. The Lumma Packing is as shown
          below

          |Y1|Y2|             (this processing)   |RGB1|RGB2|
                ---Cb, Cr  ------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */

      lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

      lumaa2 = ( lumaa3 + lumaa1 ) >> 1;

      lumaa4 = *(inputImgPtr + 2);  /* corresponds to Y2 in the pic */

      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
              Cb is for all the 4  Yavals
      --------------------------------------------------------------------*/
      cb = *(cb_ptr);

      /*--------------------------------------------------------------------
          Get the Cr for all 4 Y vals
      --------------------------------------------------------------------*/
      cr = *(cb_ptr + cr_offset);

      /*--------------------------------------------------------------------
          We got both CB and Cr for this iteration we offset cb_ptr.
          If you look at the pics you can see that they both move in
          tandem
      --------------------------------------------------------------------*/
      ++cb_ptr;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;


      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;

      /*--------------------------------------------------------------------
            Following line gets you Y3 and then base increments on
            inputImgPtr .. Hopefully
      --------------------------------------------------------------------*/
      lumaa1 = *(inputImgPtr++ + input_row_size);
      lumaa3 = *( inputImgPtr++ + input_row_size);

      lumaa2 = (lumaa3 + lumaa1) >> 1;

      lumaa4 = *( inputImgPtr  + input_row_size);

      lumaa4 = ( lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *(outRawDump++ ) = (uint16) out;
      *(outRawDump++ ) = (uint16) out2;
      *(outRawDump++ ) = (uint16) out3;
      *(outRawDump++ ) = (uint16) out4;

    } /* End of col loop */

    /*--------------------------------------------------------------------
        Take Luma of all four pixels. The Lumma Packing is as shown
        below

        |Y1|Y2|             (this processing)   |RGB1|RGB2|
              ---Cb, Cr  ------------------->   |RGB3|RGB4|
        |Y3|Y4|
    --------------------------------------------------------------------*/
    lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */

    lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

    lumaa2 = ( lumaa3 + lumaa1 ) >> 1;


    lumaa4 = lumaa3;

    /*-0--------------------------------------------------------------------
                Cb is for all the 4  Yavals
     ---------------------------------------------------------------------*/
    cb = *(cb_ptr);

    /*--------------------------------------------------------------------
        Get the Cr for all 4 Y vals
    --------------------------------------------------------------------*/
    cr = *(cb_ptr + cr_offset);

    /*--------------------------------------------------------------------
        We got both CB and Cr for this iteration we offset cb_ptr.
        If you look at the pics you can see that they both move in
        tandem
    --------------------------------------------------------------------*/
    ++cb_ptr;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
        Dump the first RGB values into the 3 neighboring spots
        as shown below
        IX
        XX
        the I is pixel from input vector and Xs are the dumped pixels
     -------------------------------------------------------------------*/
    *outputImgPtr++ = out;
    *outputImgPtr++ = out2;


    /*--------------------------------------------------------------------
         Now dump the second set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *outputImgPtr++ = out3;
    *outputImgPtr++ = out4;

    /*--------------------------------------------------------------------
          Following line gets you Y3 and then base increments on
          inputImgPtr .. Hopefully
    --------------------------------------------------------------------*/
    lumaa1 = *(inputImgPtr++ + input_row_size);
    lumaa3 = *( inputImgPtr++ + input_row_size);

    lumaa2 = (lumaa3 + lumaa1) >> 1;


    lumaa4 = lumaa3;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
         Now dump the third set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *(outRawDump++ ) = (uint16) out;
    *(outRawDump++ ) = (uint16) out2;
    *(outRawDump++ ) = (uint16) out3;
    *(outRawDump++ ) = (uint16) out4;


    /*----------------------------------------------------------------------
        Copy the currently formed row to the next one .. Pixel duplication
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outputImgPtr + destInc ), out_row_start,
      (i_param->crop.dx << 1) );

    /*----------------------------------------------------------------------
        Copy 3rd row to 4 th
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outRawDump + destInc ),
      ( uint16 *) ( ( uint8 *) outputImgPtr + destInc )
      + outDx, (i_param->crop.dx << 1) );


    inputImgPtr += (uint32) (rowInc  + input_row_size);

    /*----------------------------------------------------------------------
      Remember each four pixels has same CB/CR combination. So we need
      to add number of pixels offset divided by 4

      YY
      -----------------maps to one cr/cb --> cr/cb
      YY

    ----------------------------------------------------------------------*/
    cb_ptr += (rowInc) >> 1 /* divided by 2 */;

    /*----------------------------------------------------------------------
      We need to offset the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
     ---------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ( (uint32 )outRawDump + destInc) +
                   ( outDx  );

  } /* End of row loop */

  return(IPL_SUCCESS);

} /* end of function ipl2_upSizeAndCrop_YCbCr420ToRGB */


/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCBCR420TORGB_Med_rot90()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
  (both Widh and height). If the Crop params are less than the actual
  mulitiplied size there will be cropping. This is a combined routine and
  hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB_Med_rot90
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  uint16* outputImgPtrStart =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb, cr, r_factor;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register uint8 *cb_ptr;
  register uint16 *outRawDump; 
  uint32 rowInc = 0, cr_offset;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  int32 dx;

  /*-----------------------------------------------------------------------*/
  // General algorithm:
  //
  // When parsing theinput we work forward (to the right) in the x-dimension, 
  // but go up in the y-direction. When writting the output, we swap 
  // dimensions.
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB_Med_rot90\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }
  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Crop.dx is in terms of
    upscaled dimensions.
   -----------------------------------------------------------------------*/
  //rowInc =  (i_img_ptr->dy  - (i_param->crop.dy / 2) );
  rowInc =  (i_param->crop.dy / 2);
  outputImgPtrStart += (i_param->crop.x + o_img_ptr->dx*i_param->crop.y);


  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy  ) >> 2 /* /4 */;

  inputImgPtr = i_img_ptr->imgPtr + (i_img_ptr->dx * (i_img_ptr->dy - 1));

  cb_ptr = i_img_ptr->clrPtr + cr_offset - (i_img_ptr->dx / 2);

  /*------------------------------------------------------------------------
      Takes us to the offset for Cb Vals
  ------------------------------------------------------------------------*/
  dx = o_img_ptr->dx;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for (row = 0; row < i_param->crop.dx; row += 4)
  {
    /*----------------------------------------------------------------------
                This is the row we will create
                for adding the second luamma block in YUV420 packing
    ----------------------------------------------------------------------*/
    
    outputImgPtr = (outputImgPtrStart + row);
    //outputImgPtr = (uint16 *) (o_img_ptr->imgPtr + 2*row*dx);

    outRawDump = ( outputImgPtr + 2);
    //outRawDump = ( outputImgPtr + 2 * dx);
   

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dy/4. Dont do last 4, we 
      do those a special way.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dy/4 - 1; col; col-- )
    {

      /*--------------------------------------------------------------------
          Take Luma of all four pixels. The Lumma Packing is as shown
          below

          |Y1|Y2|             (this processing)   |RGB1|RGB2|
                ---Cb, Cr  ------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
      lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */
      lumaa2 = (lumaa3 + lumaa1 ) >> 1;
      lumaa4 = *(inputImgPtr + 2);  /* corresponds to Y2 in the pic */
      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
              Cb is for all the 4  Yavals
      --------------------------------------------------------------------*/
      cb = *(cb_ptr);
      //cb = 128;

      /*--------------------------------------------------------------------
          Get the Cr for all 4 Y vals
      --------------------------------------------------------------------*/
      cr = *(cb_ptr + cr_offset);
      //cr = 128;

      /*--------------------------------------------------------------------
          We got both CB and Cr for this iteration we offset cb_ptr.
          If you look at the pics you can see that they both move in
          tandem
      --------------------------------------------------------------------*/
      ++cb_ptr;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += dx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      outputImgPtr += dx;


      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      outputImgPtr += dx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      outputImgPtr += dx;

      /*--------------------------------------------------------------------
            Following line gets you Y3 and then base increments on
            inputImgPtr .. Hopefully
      --------------------------------------------------------------------*/
      lumaa1 = *(inputImgPtr++ - input_row_size);
      lumaa3 = *(inputImgPtr++ - input_row_size);
      lumaa2 = (lumaa3 + lumaa1) >> 1;
      lumaa4 = *(inputImgPtr  - input_row_size);
      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *(outRawDump) = (uint16) out;
      *(outRawDump+1) = (uint16) out;
      outRawDump += dx;

      *(outRawDump) = (uint16) out2;
      *(outRawDump+1) = (uint16) out2;
      outRawDump += dx;

      *(outRawDump) = (uint16) out3;
      *(outRawDump+1) = (uint16) out3;
      outRawDump += dx;

      *(outRawDump) = (uint16) out4;
      *(outRawDump+1) = (uint16) out4;
      outRawDump += dx;
    } 


    //--------------------------------------------------------------------
    // now we must do last 4 pixel by differently
    //--------------------------------------------------------------------
    lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
    lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */
    lumaa2 = ( lumaa3 + lumaa1 ) >> 1;
    lumaa4 = lumaa3;

    /*----------------------------------------------------------------------
                Cb is for all the 4  Yavals
     ---------------------------------------------------------------------*/
    cb = *(cb_ptr);
    //cb = 128;

    /*--------------------------------------------------------------------
        Get the Cr for all 4 Y vals
    --------------------------------------------------------------------*/
    cr = *(cb_ptr + cr_offset);
    //cr = 128;

    /*--------------------------------------------------------------------
        We got both CB and Cr for this iteration we offset cb_ptr.
        If you look at the pics you can see that they both move in
        tandem
    --------------------------------------------------------------------*/
    ++cb_ptr;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
        Dump the first RGB values into the 3 neighboring spots
        as shown below
        IX
        XX
        the I is pixel from input vector and Xs are the dumped pixels
     -------------------------------------------------------------------*/
    *outputImgPtr = out;
    *(outputImgPtr+1) = out;
    outputImgPtr += dx;

    *outputImgPtr = out2;
    *(outputImgPtr+1) = out2;
    outputImgPtr += dx;


    /*--------------------------------------------------------------------
         Now dump the second set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *outputImgPtr = out3;
    *(outputImgPtr+1) = out3;
    outputImgPtr += dx;

    *outputImgPtr = out4;
    *(outputImgPtr+1) = out4;
    outputImgPtr += dx;







    /*--------------------------------------------------------------------
          Following line gets you Y3 and then base increments on
          inputImgPtr .. Hopefully
    --------------------------------------------------------------------*/
    lumaa1 = *(inputImgPtr++ - input_row_size);
    lumaa3 = *(inputImgPtr++ - input_row_size);
    lumaa2 = (lumaa3 + lumaa1) >> 1;
    lumaa4 = lumaa3;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
         Now dump the third set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *(outRawDump) = (uint16) out;
    *(outRawDump+1) = (uint16) out;
    outRawDump += dx;

    *(outRawDump) = (uint16) out2;
    *(outRawDump+1) = (uint16) out2;
    outRawDump += dx;

    *(outRawDump) = (uint16) out3;
    *(outRawDump+1) = (uint16) out3;
    outRawDump += dx;

    *(outRawDump) = (uint16) out4;
    *(outRawDump+1) = (uint16) out4;
    outRawDump += dx;

    inputImgPtr -= (2*input_row_size + rowInc);
    //cb_ptr -= (rowInc+2*input_row_size/2);
    cb_ptr -= (rowInc/2+input_row_size/2);
  } 


  return(IPL_SUCCESS);

} 


/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCBCR420TORGB_Med_rot270()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
  (both Widh and height). If the Crop params are less than the actual
  mulitiplied size there will be cropping. This is a combined routine and
  hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB_Med_rot270
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  uint16* outputImgPtrStart;
  register int32 cb, cr, r_factor;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  register uint8 *cb_ptr;
  uint32 rowInc = 0, cr_offset;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  int32 dx;

  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB_Med_rot270\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }
  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Crop.dx is in terms of
    upscaled dimensions.
   -----------------------------------------------------------------------*/
  rowInc = (i_param->crop.dy / 2);

  outputImgPtrStart = (outputImgPtr + (i_param->crop.x + o_img_ptr->dx 
                                  * i_param->crop.y));

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy  ) >> 2 /* /4 */;

  inputImgPtr = i_img_ptr->imgPtr + i_img_ptr->dx - 1;

  cb_ptr = i_img_ptr->clrPtr + (i_img_ptr->dx / 2 - 1);

  /*------------------------------------------------------------------------
      Takes us to the offset for Cb Vals
  ------------------------------------------------------------------------*/


  dx = o_img_ptr->dx;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for (row = 0; row < i_param->crop.dx; row += 4)
  {
    /*----------------------------------------------------------------------
                This is the row we will create
                for adding the second luamma block in YUV420 packing
    ----------------------------------------------------------------------*/
    
    outputImgPtr = (outputImgPtrStart + row);
    //outputImgPtr = (uint16 *) (o_img_ptr->imgPtr + 2*row*dx);

    outRawDump = ( outputImgPtr + 2);
    //outRawDump = ( outputImgPtr + 2 * dx);
   
    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dy/4 - 1; col; col-- )
    {

      /*--------------------------------------------------------------------
          Take Luma of all four pixels. The Lumma Packing is as shown
          below

          |Y1|Y2|             (this processing)   |RGB1|RGB2|
                ---Cb, Cr  ------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
      lumaa3 = *(inputImgPtr - 1);  /* corresponds to Y2 in the pic */
      lumaa2 = (lumaa3 + lumaa1 ) >> 1;
      lumaa4 = *(inputImgPtr - 2);  /* corresponds to Y2 in the pic */
      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
              Cb is for all the 4  Yavals
      --------------------------------------------------------------------*/
      cb = *(cb_ptr);
      //cb = 128;

      /*--------------------------------------------------------------------
          Get the Cr for all 4 Y vals
      --------------------------------------------------------------------*/
      cr = *(cb_ptr + cr_offset);
      //cr = 128;

      /*--------------------------------------------------------------------
          We got both CB and Cr for this iteration we offset cb_ptr.
          If you look at the pics you can see that they both move in
          tandem
      --------------------------------------------------------------------*/
      --cb_ptr;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr = out;
      *(outputImgPtr+1) = out;
      outputImgPtr += dx;

      *outputImgPtr = out2;
      *(outputImgPtr+1) = out2;
      outputImgPtr += dx;


      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr = out3;
      *(outputImgPtr+1) = out3;
      outputImgPtr += dx;

      *outputImgPtr = out4;
      *(outputImgPtr+1) = out4;
      outputImgPtr += dx;

      /*--------------------------------------------------------------------
            Following line gets you Y3 and then base increments on
            inputImgPtr .. Hopefully
      --------------------------------------------------------------------*/
      lumaa1 = *(inputImgPtr-- + input_row_size);
      lumaa3 = *(inputImgPtr-- + input_row_size);
      lumaa2 = (lumaa3 + lumaa1) >> 1;
      lumaa4 = *(inputImgPtr  + input_row_size);
      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *(outRawDump) = (uint16) out;
      *(outRawDump+1) = (uint16) out;
      outRawDump += dx;

      *(outRawDump) = (uint16) out2;
      *(outRawDump+1) = (uint16) out2;
      outRawDump += dx;

      *(outRawDump) = (uint16) out3;
      *(outRawDump+1) = (uint16) out3;
      outRawDump += dx;

      *(outRawDump) = (uint16) out4;
      *(outRawDump+1) = (uint16) out4;
      outRawDump += dx;
    } 


    //--------------------------------------------------------------------
    // now we must do last 4 pixel by differently
    //--------------------------------------------------------------------
    lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
    lumaa3 = *(inputImgPtr - 1);  /* corresponds to Y2 in the pic */
    lumaa2 = (lumaa3 + lumaa1 ) >> 1;
    lumaa4 = lumaa3;

    /*----------------------------------------------------------------------
                Cb is for all the 4  Yavals
     ---------------------------------------------------------------------*/
    cb = *(cb_ptr);
    //cb = 128;

    /*--------------------------------------------------------------------
        Get the Cr for all 4 Y vals
    --------------------------------------------------------------------*/
    cr = *(cb_ptr + cr_offset);
    //cr = 128;

    /*--------------------------------------------------------------------
        We got both CB and Cr for this iteration we offset cb_ptr.
        If you look at the pics you can see that they both move in
        tandem
    --------------------------------------------------------------------*/
    //--cb_ptr;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
        Dump the first RGB values into the 3 neighboring spots
        as shown below
        IX
        XX
        the I is pixel from input vector and Xs are the dumped pixels
     -------------------------------------------------------------------*/
    *outputImgPtr = out;
    *(outputImgPtr+1) = out;
    outputImgPtr += dx;

    *outputImgPtr = out2;
    *(outputImgPtr+1) = out2;
    outputImgPtr += dx;


    /*--------------------------------------------------------------------
         Now dump the second set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *outputImgPtr = out3;
    *(outputImgPtr+1) = out3;
    outputImgPtr += dx;

    *outputImgPtr = out4;
    *(outputImgPtr+1) = out4;
    outputImgPtr += dx;







    /*--------------------------------------------------------------------
          Following line gets you Y3 and then base increments on
          inputImgPtr .. Hopefully
    --------------------------------------------------------------------*/
    lumaa1 = *(inputImgPtr-- + input_row_size);
    lumaa3 = *(inputImgPtr + input_row_size);
    lumaa2 = (lumaa3 + lumaa1) >> 1;
    lumaa4 = lumaa3;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
         Now dump the third set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *(outRawDump) = (uint16) out;
    *(outRawDump+1) = (uint16) out;
    outRawDump += dx;

    *(outRawDump) = (uint16) out2;
    *(outRawDump+1) = (uint16) out2;
    outRawDump += dx;

    *(outRawDump) = (uint16) out3;
    *(outRawDump+1) = (uint16) out3;
    outRawDump += dx;

    *(outRawDump) = (uint16) out4;
    *(outRawDump+1) = (uint16) out4;
    outRawDump += dx;


    // lets get ready for the next two rows!
    //inputImgPtr += ((uint32) 3*(rowInc + input_row_size) - 1);
    inputImgPtr += ((uint32) (rowInc + 2*input_row_size) - 1);

    //cb_ptr += ((rowInc + input_row_size) - 1);
    cb_ptr += ((rowInc/2 + input_row_size/2) - 1);
  } 


  return(IPL_SUCCESS);

} 



/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCBCR420TORGB_Med_rot180()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
  (both Widh and height). If the Crop params are less than the actual
  mulitiplied size there will be cropping. This is a combined routine and
  hence will have more CPU optimized performance.

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
static ipl_status_type ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB_Med_rot180
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  uint16* outputImgPtrStart;
  register uint16 out, out2, out3, out4;
  register int32 cb, cr, r_factor;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  register uint8 *cb_ptr;
  uint32 rowInc = 0, cr_offset;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  int32 dx;

  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB_Med_rot180\n");

  switch ( o_img_ptr->cFormat)
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }
  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Crop.dx is in terms of
    upscaled dimensions.
   -----------------------------------------------------------------------*/
  rowInc = (i_param->crop.dx / 2);

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cr_offset = (i_img_ptr->dx * i_img_ptr->dy  ) >> 2 /* /4 */;

  inputImgPtr = i_img_ptr->imgPtr + (i_img_ptr->dx * i_img_ptr->dy) - 1;

  cb_ptr = i_img_ptr->clrPtr + (cr_offset - 1);

  outputImgPtrStart = (outputImgPtr + (i_param->crop.x + o_img_ptr->dx 
                                  * i_param->crop.y));

  dx = o_img_ptr->dx;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for (row = 0; row < i_param->crop.dy; row += 4)
  {
    /*----------------------------------------------------------------------
                This is the row we will create
                for adding the second luamma block in YUV420 packing
    ----------------------------------------------------------------------*/
    outputImgPtr = outputImgPtrStart + row*dx;
    outRawDump = (outputImgPtr + 2*dx);
   
    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dx/4 - 1; col; col-- )
    {

      /*--------------------------------------------------------------------
          Take Luma of all four pixels. The Lumma Packing is as shown
          below

          |Y1|Y2|             (this processing)   |RGB1|RGB2|
                ---Cb, Cr  ------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
      lumaa3 = *(inputImgPtr - 1);  /* corresponds to Y2 in the pic */
      lumaa2 = (lumaa3 + lumaa1 ) >> 1;
      lumaa4 = *(inputImgPtr - 2);  /* corresponds to Y2 in the pic */
      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
              Cb is for all the 4  Yavals
      --------------------------------------------------------------------*/
      cb = *(cb_ptr);
      //cb = 128;

      /*--------------------------------------------------------------------
          Get the Cr for all 4 Y vals
      --------------------------------------------------------------------*/
      cr = *(cb_ptr + cr_offset);
      //cr = 128;

      /*--------------------------------------------------------------------
          We got both CB and Cr for this iteration we offset cb_ptr.
          If you look at the pics you can see that they both move in
          tandem
      --------------------------------------------------------------------*/
      --cb_ptr;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr = out;
      *(outputImgPtr+dx) = out;
      outputImgPtr++;

      *outputImgPtr = out2;
      *(outputImgPtr+dx) = out2;
      outputImgPtr++;

      *outputImgPtr = out3;
      *(outputImgPtr+dx) = out3;
      outputImgPtr++;

      *outputImgPtr = out4;
      *(outputImgPtr+dx) = out4;
      outputImgPtr++;

      /*--------------------------------------------------------------------
            Following line gets you Y3 and then base increments on
            inputImgPtr .. Hopefully
      --------------------------------------------------------------------*/
      lumaa1 = *(inputImgPtr-- - input_row_size);
      lumaa3 = *(inputImgPtr-- - input_row_size);
      lumaa2 = (lumaa3 + lumaa1) >> 1;
      lumaa4 = *(inputImgPtr  - input_row_size);
      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *(outRawDump) = (uint16) out;
      *(outRawDump+dx) = (uint16) out;
      outRawDump++;

      *(outRawDump) = (uint16) out2;
      *(outRawDump+dx) = (uint16) out2;
      outRawDump++;

      *(outRawDump) = (uint16) out3;
      *(outRawDump+dx) = (uint16) out3;
      outRawDump++;

      *(outRawDump) = (uint16) out4;
      *(outRawDump+dx) = (uint16) out4;
      outRawDump++;
    } 

    //--------------------------------------------------------------------
    // now we must do last 4 pixel by differently
    //--------------------------------------------------------------------
    lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
    lumaa3 = *(inputImgPtr - 1);  /* corresponds to Y2 in the pic */
    lumaa2 = (lumaa3 + lumaa1 ) >> 1;
    lumaa4 = lumaa3;

    /*----------------------------------------------------------------------
                Cb is for all the 4  Yavals
     ---------------------------------------------------------------------*/
    cb = *(cb_ptr);
    //cb = 128;

    /*--------------------------------------------------------------------
        Get the Cr for all 4 Y vals
    --------------------------------------------------------------------*/
    cr = *(cb_ptr + cr_offset);
    //cr = 128;

    /*--------------------------------------------------------------------
        We got both CB and Cr for this iteration we offset cb_ptr.
        If you look at the pics you can see that they both move in
        tandem
    --------------------------------------------------------------------*/
    //--cb_ptr;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
        Dump the first RGB values into the 3 neighboring spots
        as shown below
        IX
        XX
        the I is pixel from input vector and Xs are the dumped pixels
     -------------------------------------------------------------------*/
    *outputImgPtr = out;
    *(outputImgPtr+dx) = out;
    outputImgPtr++;

    *outputImgPtr = out2;
    *(outputImgPtr+dx) = out2;
    outputImgPtr++;


    /*--------------------------------------------------------------------
         Now dump the second set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *outputImgPtr = out3;
    *(outputImgPtr+dx) = out3;
    outputImgPtr++;

    *outputImgPtr = out4;
    *(outputImgPtr+dx) = out4;
    outputImgPtr++;


    /*--------------------------------------------------------------------
          Following line gets you Y3 and then base increments on
          inputImgPtr .. Hopefully
    --------------------------------------------------------------------*/
    lumaa1 = *(inputImgPtr-- - input_row_size);
    lumaa3 = *(inputImgPtr - input_row_size);
    lumaa2 = (lumaa3 + lumaa1) >> 1;
    lumaa4 = lumaa3;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
         Now dump the third set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *(outRawDump) = (uint16) out;
    *(outRawDump+dx) = (uint16) out;
    outRawDump++;

    *(outRawDump) = (uint16) out2;
    *(outRawDump+dx) = (uint16) out2;
    outRawDump++;

    *(outRawDump) = (uint16) out3;
    *(outRawDump+dx) = (uint16) out3;
    outRawDump++;

    *(outRawDump) = (uint16) out4;
    *(outRawDump+dx) = (uint16) out4;
    outRawDump++;

    // lets get ready for the next two rows!
    inputImgPtr += (uint32) (rowInc - 2*input_row_size - 1);
    cb_ptr += (rowInc/2 - input_row_size/2 - 1);
  } 


  return(IPL_SUCCESS);

} 




/*===========================================================================

FUNCTION ipl2_upSize2x_CropInOut_YCbCr420lpToRGB

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 line pack
  based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr  - input image frame
  cropin         - input crop
  cropout        - output crop

ARGUMENTS OUT
  output_img_ptr - Points to the output image

RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSize2x_CropInOut_YCbCr420lpToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl_rect_type* cropin,
  ipl_rect_type* cropout
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb, cr, r_factor;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint32 pitchi;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint8 *out_row_start; /* for saving start of row */
  register uint8 *cr_ptr;
  uint32 destInc;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  int32 dest_index;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSize2x_CropInOut_YCrCb420lpToRGB\n");

  if ( ipl2_init() != IPL_SUCCESS )
    return( IPL_FAILURE );

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (cropout->x + o_img_ptr->dx * cropout->y) * 2;

  outputImgPtr = (uint16 *) ( (uint32)outputImgPtr +  dest_index );
  destInc = ( o_img_ptr->dx  - cropout->dx ) * 2;

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/

  cr_ptr = i_img_ptr->clrPtr + (2*(cropin->x/2))+(cropin->y/2 * i_img_ptr->dx);
  inputImgPtr = i_img_ptr->imgPtr + cropin->x + cropin->y*i_img_ptr->dx;
  pitchi = (i_img_ptr->dx - cropin->dx);

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for ( row = cropout->dy/4; row; row-- )
  {
    /*----------------------------------------------------------------------
                This is the row we will create
                for adding the second luamma block in YUV420 packing
    ----------------------------------------------------------------------*/
    outRawDump = ( outputImgPtr + 2 * outDx );
    out_row_start = (uint8 *) outputImgPtr;

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = cropout->dx/4 - 1; col; col-- )
    {

      /*--------------------------------------------------------------------
          Take Luma of all four pixels. The Lumma Packing is as shown
          below

          |Y1|Y2|             (this processing)   |RGB1|RGB2|
                ---Cb, Cr  ------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */

      lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

      lumaa2 = ( lumaa3 + lumaa1 ) >> 1;

      lumaa4 = *(inputImgPtr + 2);  /* corresponds to Y2 in the pic */

      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
              Cb is for all the 4  Yavals
      --------------------------------------------------------------------*/
      cb = *(cr_ptr++);

      /*--------------------------------------------------------------------
          Get the Cr for all 4 Y vals
      --------------------------------------------------------------------*/
      cr = *(cr_ptr++);

      /*--------------------------------------------------------------------
          We got both CB and Cr for this iteration we offset cb_ptr.
          If you look at the pics you can see that they both move in
          tandem
      --------------------------------------------------------------------*/
      //cb_ptr += 2;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;


      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;

      /*--------------------------------------------------------------------
            Following line gets you Y3 and then base increments on
            inputImgPtr .. Hopefully
      --------------------------------------------------------------------*/
      lumaa1 = *(inputImgPtr++ + input_row_size);
      lumaa3 = *( inputImgPtr++ + input_row_size);

      lumaa2 = (lumaa3 + lumaa1) >> 1;

      lumaa4 = *( inputImgPtr  + input_row_size);

      lumaa4 = ( lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *(outRawDump++ ) = (uint16) out;
      *(outRawDump++ ) = (uint16) out2;
      *(outRawDump++ ) = (uint16) out3;
      *(outRawDump++ ) = (uint16) out4;

    } /* End of col loop */

    /*--------------------------------------------------------------------
        Take Luma of all four pixels. The Lumma Packing is as shown
        below

        |Y1|Y2|             (this processing)   |RGB1|RGB2|
              ---Cb, Cr  ------------------->   |RGB3|RGB4|
        |Y3|Y4|
    --------------------------------------------------------------------*/
    lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */

    lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

    lumaa2 = ( lumaa3 + lumaa1 ) >> 1;


    lumaa4 = lumaa3;

    /*-0--------------------------------------------------------------------
                Cb is for all the 4  Yavals
     ---------------------------------------------------------------------*/
    cb = *(cr_ptr++);

    /*--------------------------------------------------------------------
        Get the Cr for all 4 Y vals
    --------------------------------------------------------------------*/
    cr = *(cr_ptr++);

    /*--------------------------------------------------------------------
        We got both CB and Cr for this iteration we offset cb_ptr.
        If you look at the pics you can see that they both move in
        tandem
    --------------------------------------------------------------------*/
    // cb_ptr += 2;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
        Dump the first RGB values into the 3 neighboring spots
        as shown below
        IX
        XX
        the I is pixel from input vector and Xs are the dumped pixels
     -------------------------------------------------------------------*/
    *outputImgPtr++ = out;
    *outputImgPtr++ = out2;


    /*--------------------------------------------------------------------
         Now dump the second set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *outputImgPtr++ = out3;
    *outputImgPtr++ = out4;

    /*--------------------------------------------------------------------
          Following line gets you Y3 and then base increments on
          inputImgPtr .. Hopefully
    --------------------------------------------------------------------*/
    lumaa1 = *(inputImgPtr++ + input_row_size);
    lumaa3 = *( inputImgPtr++ + input_row_size);

    lumaa2 = (lumaa3 + lumaa1) >> 1;


    lumaa4 = lumaa3;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
         Now dump the third set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *(outRawDump++ ) = (uint16) out;
    *(outRawDump++ ) = (uint16) out2;
    *(outRawDump++ ) = (uint16) out3;
    *(outRawDump++ ) = (uint16) out4;


    /*----------------------------------------------------------------------
        Copy the currently formed row to the next one .. Pixel duplication
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outputImgPtr + destInc ), out_row_start,
      (cropout->dx << 1) );

    /*----------------------------------------------------------------------
        Copy 3rd row to 4 th
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outRawDump + destInc ),
      ( uint16 *) ( ( uint8 *) outputImgPtr + destInc )
      + outDx, (cropout->dx << 1) );


    inputImgPtr += (uint32) (pitchi + input_row_size);

    /*----------------------------------------------------------------------
      Remember each four pixels has same CB/CR combination. So we need
      to add number of pixels offset divided by 4

      YY
      -----------------maps to one cr/cb --> cr/cb
      YY

    ----------------------------------------------------------------------*/
    cr_ptr += (pitchi) >> 0 /* divided by 2 */;

    /*----------------------------------------------------------------------
      We need to offset the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
     ---------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ( (uint32 )outRawDump + destInc) +
                   ( outDx  );

  } /* End of row loop */

  return(IPL_SUCCESS);
} 



/*===========================================================================

FUNCTION ipl2_upSize2x_CropInOut_YCrCb420lpToRGB

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 line pack
  based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr  - input image frame
  cropin         - input crop
  cropout        - output crop

ARGUMENTS OUT
  output_img_ptr - Points to the output image

RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSize2x_CropInOut_YCrCb420lpToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl_rect_type* cropin,
  ipl_rect_type* cropout
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb, cr, r_factor;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint32 pitchi;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint8 *out_row_start; /* for saving start of row */
  register uint8 *cr_ptr;
  uint32 destInc;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  int32 dest_index;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSize2x_CropInOut_YCrCb420lpToRGB\n");

  if ( ipl2_init() != IPL_SUCCESS )
    return( IPL_FAILURE );

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (cropout->x + o_img_ptr->dx * cropout->y) * 2;

  outputImgPtr = (uint16 *) ( (uint32)outputImgPtr +  dest_index );
  destInc = ( o_img_ptr->dx  - cropout->dx ) * 2;

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/

  cr_ptr = i_img_ptr->clrPtr + (2*(cropin->x/2))+(cropin->y/2 * i_img_ptr->dx);
  inputImgPtr = i_img_ptr->imgPtr + cropin->x + cropin->y*i_img_ptr->dx;
  pitchi = (i_img_ptr->dx - cropin->dx);

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for ( row = cropout->dy/4; row; row-- )
  {
    /*----------------------------------------------------------------------
                This is the row we will create
                for adding the second luamma block in YUV420 packing
    ----------------------------------------------------------------------*/
    outRawDump = ( outputImgPtr + 2 * outDx );
    out_row_start = (uint8 *) outputImgPtr;

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = cropout->dx/4 - 1; col; col-- )
    {

      /*--------------------------------------------------------------------
          Take Luma of all four pixels. The Lumma Packing is as shown
          below

          |Y1|Y2|             (this processing)   |RGB1|RGB2|
                ---Cb, Cr  ------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */

      lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

      lumaa2 = ( lumaa3 + lumaa1 ) >> 1;

      lumaa4 = *(inputImgPtr + 2);  /* corresponds to Y2 in the pic */

      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
              Cb is for all the 4  Yavals
      --------------------------------------------------------------------*/
      cr = *(cr_ptr++);

      /*--------------------------------------------------------------------
          Get the Cr for all 4 Y vals
      --------------------------------------------------------------------*/
      cb = *(cr_ptr++);

      /*--------------------------------------------------------------------
          We got both CB and Cr for this iteration we offset cb_ptr.
          If you look at the pics you can see that they both move in
          tandem
      --------------------------------------------------------------------*/
      //cb_ptr += 2;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;


      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;

      /*--------------------------------------------------------------------
            Following line gets you Y3 and then base increments on
            inputImgPtr .. Hopefully
      --------------------------------------------------------------------*/
      lumaa1 = *(inputImgPtr++ + input_row_size);
      lumaa3 = *( inputImgPtr++ + input_row_size);

      lumaa2 = (lumaa3 + lumaa1) >> 1;

      lumaa4 = *( inputImgPtr  + input_row_size);

      lumaa4 = ( lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *(outRawDump++ ) = (uint16) out;
      *(outRawDump++ ) = (uint16) out2;
      *(outRawDump++ ) = (uint16) out3;
      *(outRawDump++ ) = (uint16) out4;

    } /* End of col loop */

    /*--------------------------------------------------------------------
        Take Luma of all four pixels. The Lumma Packing is as shown
        below

        |Y1|Y2|             (this processing)   |RGB1|RGB2|
              ---Cb, Cr  ------------------->   |RGB3|RGB4|
        |Y3|Y4|
    --------------------------------------------------------------------*/
    lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */

    lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

    lumaa2 = ( lumaa3 + lumaa1 ) >> 1;


    lumaa4 = lumaa3;

    /*-0--------------------------------------------------------------------
                Cb is for all the 4  Yavals
     ---------------------------------------------------------------------*/
    cr = *(cr_ptr++);

    /*--------------------------------------------------------------------
        Get the Cr for all 4 Y vals
    --------------------------------------------------------------------*/
    cb = *(cr_ptr++);

    /*--------------------------------------------------------------------
        We got both CB and Cr for this iteration we offset cb_ptr.
        If you look at the pics you can see that they both move in
        tandem
    --------------------------------------------------------------------*/
    // cb_ptr += 2;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
        Dump the first RGB values into the 3 neighboring spots
        as shown below
        IX
        XX
        the I is pixel from input vector and Xs are the dumped pixels
     -------------------------------------------------------------------*/
    *outputImgPtr++ = out;
    *outputImgPtr++ = out2;


    /*--------------------------------------------------------------------
         Now dump the second set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *outputImgPtr++ = out3;
    *outputImgPtr++ = out4;

    /*--------------------------------------------------------------------
          Following line gets you Y3 and then base increments on
          inputImgPtr .. Hopefully
    --------------------------------------------------------------------*/
    lumaa1 = *(inputImgPtr++ + input_row_size);
    lumaa3 = *( inputImgPtr++ + input_row_size);

    lumaa2 = (lumaa3 + lumaa1) >> 1;


    lumaa4 = lumaa3;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
         Now dump the third set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *(outRawDump++ ) = (uint16) out;
    *(outRawDump++ ) = (uint16) out2;
    *(outRawDump++ ) = (uint16) out3;
    *(outRawDump++ ) = (uint16) out4;


    /*----------------------------------------------------------------------
        Copy the currently formed row to the next one .. Pixel duplication
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outputImgPtr + destInc ), out_row_start,
      (cropout->dx << 1) );

    /*----------------------------------------------------------------------
        Copy 3rd row to 4 th
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outRawDump + destInc ),
      ( uint16 *) ( ( uint8 *) outputImgPtr + destInc )
      + outDx, (cropout->dx << 1) );


    inputImgPtr += (uint32) (pitchi + input_row_size);

    /*----------------------------------------------------------------------
      Remember each four pixels has same CB/CR combination. So we need
      to add number of pixels offset divided by 4

      YY
      -----------------maps to one cr/cb --> cr/cb
      YY

    ----------------------------------------------------------------------*/
    cr_ptr += (pitchi) >> 0 /* divided by 2 */;

    /*----------------------------------------------------------------------
      We need to offset the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
     ---------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ( (uint32 )outRawDump + destInc) +
                   ( outDx  );

  } /* End of row loop */

  return(IPL_SUCCESS);
} 





/*===========================================================================

FUNCTION IPL2_UPSIZEANDCROP_YCRCB420LPTORGB()

DESCRIPTION
  This function uses pixel dumping to do upsize a YCbCr420 line pack
  based image to
  RGB image - larger in size.  The image is multiplied by 2 in dimensions
   (both Widh and height). If the Crop params are less than the actual
   mulitiplied size there will be cropping. This is a combined routine and
   hence will have more CPU optimized performance.

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
extern ipl_status_type ipl2_upSizeRowAvgAndCrop_YCrCb420lpToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3, out4;
  register int32 cb, cr, r_factor;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint8 *out_row_start; /* for saving start of row */
  register uint8 *cr_ptr;
  uint32 rowInc = 0, destInc;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;
  int32 dest_index;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeRowAvgAndCrop_YCrCb420lpToRGB\n");

  // cropping not currently supported
  if ((i_param->crop.x != 0) ||
      (i_param->crop.y != 0) ||
      (i_param->crop.dx != o_img_ptr->dx) ||
      (i_param->crop.dy != o_img_ptr->dy))
      return( IPL_FAILURE );

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
    We are doubling the number of pixels. We are cropping the processing of
    input image so that those pixels falling in these intervals are not
    dumped or used.  The rowInc is in terms of bytes. Crop.dx is in terms of
    upscaled dimensions.
   -----------------------------------------------------------------------*/
  rowInc = (i_img_ptr->dx  - (i_param->crop.dx / 2) );


  outputImgPtr = (uint16 *) ( (uint32)outputImgPtr +  dest_index );
  destInc = ( o_img_ptr->dx  - i_param->crop.dx ) * 2;

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cr_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  for ( row = i_param->crop.dy/4; row; row-- )
  {
    /*----------------------------------------------------------------------
                This is the row we will create
                for adding the second luamma block in YUV420 packing
    ----------------------------------------------------------------------*/
    outRawDump = ( outputImgPtr + 2 * outDx );
    out_row_start = (uint8 *) outputImgPtr;

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dx/4 - 1; col; col-- )
    {

      /*--------------------------------------------------------------------
          Take Luma of all four pixels. The Lumma Packing is as shown
          below

          |Y1|Y2|             (this processing)   |RGB1|RGB2|
                ---Cb, Cr  ------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */

      lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

      lumaa2 = ( lumaa3 + lumaa1 ) >> 1;

      lumaa4 = *(inputImgPtr + 2);  /* corresponds to Y2 in the pic */

      lumaa4 = (lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
              Cb is for all the 4  Yavals
      --------------------------------------------------------------------*/
      cr = *(cr_ptr++);

      /*--------------------------------------------------------------------
          Get the Cr for all 4 Y vals
      --------------------------------------------------------------------*/
      cb = *(cr_ptr++);

      /*--------------------------------------------------------------------
          We got both CB and Cr for this iteration we offset cb_ptr.
          If you look at the pics you can see that they both move in
          tandem
      --------------------------------------------------------------------*/
      //cb_ptr += 2;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
          Dump the first RGB values into the 3 neighboring spots
          as shown below
          IX
          XX
          the I is pixel from input vector and Xs are the dumped pixels
       -------------------------------------------------------------------*/
      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;


      /*--------------------------------------------------------------------
           Now dump the second set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;

      /*--------------------------------------------------------------------
            Following line gets you Y3 and then base increments on
            inputImgPtr .. Hopefully
      --------------------------------------------------------------------*/
      lumaa1 = *(inputImgPtr++ + input_row_size);
      lumaa3 = *( inputImgPtr++ + input_row_size);

      lumaa2 = (lumaa3 + lumaa1) >> 1;

      lumaa4 = *( inputImgPtr  + input_row_size);

      lumaa4 = ( lumaa4 + lumaa3) >> 1;

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
        lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
        rTable, gTable, bTable );

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *(outRawDump++ ) = (uint16) out;
      *(outRawDump++ ) = (uint16) out2;
      *(outRawDump++ ) = (uint16) out3;
      *(outRawDump++ ) = (uint16) out4;

    } /* End of col loop */

    /*--------------------------------------------------------------------
        Take Luma of all four pixels. The Lumma Packing is as shown
        below

        |Y1|Y2|             (this processing)   |RGB1|RGB2|
              ---Cb, Cr  ------------------->   |RGB3|RGB4|
        |Y3|Y4|
    --------------------------------------------------------------------*/
    lumaa1 = *inputImgPtr;  /* corresponds to Y1 in the pic */

    lumaa3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */

    lumaa2 = ( lumaa3 + lumaa1 ) >> 1;


    lumaa4 = lumaa3;

    /*-0--------------------------------------------------------------------
                Cb is for all the 4  Yavals
     ---------------------------------------------------------------------*/
    cr = *(cr_ptr++);

    /*--------------------------------------------------------------------
        Get the Cr for all 4 Y vals
    --------------------------------------------------------------------*/
    cb = *(cr_ptr++);

    /*--------------------------------------------------------------------
        We got both CB and Cr for this iteration we offset cb_ptr.
        If you look at the pics you can see that they both move in
        tandem
    --------------------------------------------------------------------*/
    // cb_ptr += 2;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
        Dump the first RGB values into the 3 neighboring spots
        as shown below
        IX
        XX
        the I is pixel from input vector and Xs are the dumped pixels
     -------------------------------------------------------------------*/
    *outputImgPtr++ = out;
    *outputImgPtr++ = out2;


    /*--------------------------------------------------------------------
         Now dump the second set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *outputImgPtr++ = out3;
    *outputImgPtr++ = out4;

    /*--------------------------------------------------------------------
          Following line gets you Y3 and then base increments on
          inputImgPtr .. Hopefully
    --------------------------------------------------------------------*/
    lumaa1 = *(inputImgPtr++ + input_row_size);
    lumaa3 = *( inputImgPtr++ + input_row_size);

    lumaa2 = (lumaa3 + lumaa1) >> 1;


    lumaa4 = lumaa3;

    /*--------------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
    --------------------------------------------------------------------*/
    IPL2_CONVERTAVG_YCBCR420TORGB_HALFWORD_INTER( lumaa1, lumaa2, lumaa3, \
      lumaa4, r_factor, cb, cr,  r, out, out2, out3, out4,  \
      rTable, gTable, bTable );

    /*--------------------------------------------------------------------
         Now dump the third set of pixels using the same scheme
    --------------------------------------------------------------------*/
    *(outRawDump++ ) = (uint16) out;
    *(outRawDump++ ) = (uint16) out2;
    *(outRawDump++ ) = (uint16) out3;
    *(outRawDump++ ) = (uint16) out4;


    /*----------------------------------------------------------------------
        Copy the currently formed row to the next one .. Pixel duplication
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outputImgPtr + destInc ), out_row_start,
      (i_param->crop.dx << 1) );

    /*----------------------------------------------------------------------
        Copy 3rd row to 4 th
    ----------------------------------------------------------------------*/
    memcpy( ( (uint8 *) outRawDump + destInc ),
      ( uint16 *) ( ( uint8 *) outputImgPtr + destInc )
      + outDx, (i_param->crop.dx << 1) );


    inputImgPtr += (uint32) (rowInc  + input_row_size);

    /*----------------------------------------------------------------------
      Remember each four pixels has same CB/CR combination. So we need
      to add number of pixels offset divided by 4

      YY
      -----------------maps to one cr/cb --> cr/cb
      YY

    ----------------------------------------------------------------------*/
    cr_ptr += (rowInc) >> 0 /* divided by 2 */;

    /*----------------------------------------------------------------------
      We need to offset the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
     ---------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ( (uint32 )outRawDump + destInc) +
                   ( outDx  );

  } /* End of row loop */

  return(IPL_SUCCESS);
} 



/*===========================================================================

FUNCTION ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIF_133()

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2;
  register int32 cr, cb, cb_dash, cr_dash;
  register int32 lumaa1, lumaa_dash;
  register int32 lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */

  uint32 row,col_max, row_max;
  uint32 rowInc=0, dest_index, destInc;
  uint32   row_cnt, col;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).

      be careful, if .dx is 0, coolmax is -#
  ------------------------------------------------------------------------*/
  col_max =  ( ( i_param->crop.dx - 2)  / 4 );

  /*------------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
  ------------------------------------------------------------------------*/
  if ( col_max > IPL2_QCIF_COL_CNTLS_BY_3 )
  {
    col_max = IPL2_QCIF_COL_CNTLS_BY_3;
  }
  else if ( col_max & 0x1 )
  {
    col_max--;
  }



  /*------------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic

      = 0 since 176 - (174 + 2)
  ------------------------------------------------------------------------*/
  rowInc = ( IPL2_QCIF_WIDTH
             - ( col_max * 3   + 2)  ) * 2;


  outputImgPtr = (uint16 *) ((uint32)outputImgPtr +  dest_index);

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 + 2 ) ) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

  /*------------------------------------------------------------------------
      Limit the maximun no of rows to the QCIF size
  ------------------------------------------------------------------------*/
  if ( row_max > IPL2_QCIF_HEIGHT )
  {
    row_max = IPL2_QCIF_HEIGHT;
  }


  /*------------------------------------------------------------------------
      iterate through the image with row_max and clo_max for the loop
  ------------------------------------------------------------------------*/
  for ( row = row_max; row; row-- )
  {


    /*----------------------------------------------------------------------
                 This is Cb
   -----------------------------------------------------------------------*/
    cb = (int32) (*((uint8*)(inputImgPtr++)));

    /*--------------------------------------------------------------------
                Lumaa
     -------------------------------------------------------------------*/
    lumaa1 = *((uint8*)(inputImgPtr++));

    /*--------------------------------------------------------------------
              cr
     -------------------------------------------------------------------*/
    cr = (int32) (*((uint8*) (inputImgPtr++)));

    /*----------------------------------------------------------------------
        Find the opening pixel for the current row.
    ----------------------------------------------------------------------*/
    IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
      rTable, gTable, bTable);

    /*----------------------------------------------------------------------
        render the opening pixel
    ----------------------------------------------------------------------*/
    *outputImgPtr++ = out;

    /*----------------------------------------------------------------------
        The rows can be devided in three groups. One which does not
        participate in averaging one which does and one after the avraged
        row and participate in ageraging.

        ____________________________________________________
        |__________________________row0_____________________|
        |_______________________row1________________________|
        |_______________________avg_________________________|
        |________________________row2_______________________|
        |   ...........................................     |
        |___________________________________________________|


        2 rows are copied as is and then there is avg = (row1 + row2) /2

        where the addition is matrix addition.

        This cycle repeats. So the o/p is upsized by

        4/3. Because each 3 input rows create 4 o/p rows. Actually each row
        needs to be weighted aveaged. But we limit it to the row which
        falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/
    if ( row_cnt == 0 )
    {


      /*--------------------------------------------------------------------
          iterate in for the col_max by 2. This takes 6 input pixels
          in a row.
      --------------------------------------------------------------------*/
      for ( col = col_max ; col; col-=2 )
      {


        /*------------------------------------------------------------------
          Pixel 2
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Pixel = pixel 1 + ( pixel2 - pixel 1 ) * 0.75 or 3/4
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


        /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          Deal with Distance 1.5 pitch 1 - combo
        ------------------------------------------------------------------*/


        /*------------------------------------------------------------------
            Take the next pixel pair and try averaging it
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            pixel = ( pixel( n ) + pixel(n + 1 ) ) / 2
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1) ;
        cb = cb + ( ( cb_dash - cb ) >> 1);
        cr = cr + ( ( cr_dash - cr ) >> 1 );

        /*------------------------------------------------------------------
            Render the just aceraged pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with sampling distance of 0.25
        ----------------------------------------------------------------*/
        /*----------------------------------------------------------------
            Pixel 3 + n * 3
        ----------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        lumaa_dash = lumaa1;


        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Render 0.25 and 0 distance pixels
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*----------------------------------------------------------------
            Take the next pixel pair
       ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
              0.75 sampling distance averaging
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
            Next sample at 0.75 distance
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
              0.5 averaging
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;




        /*------------------------------------------------------------------
          Deal with Distance .25 distance
        ------------------------------------------------------------------*/
        cb = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = *((uint8*)(inputImgPtr++));

        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file. Convert single pixel
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

      } /* End of col loop */


      /*--------------------------------------------------------------------
      Deal with the closing pixel
      --------------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        The closing pixel is assumed in 0.75 sample space. This might not
        be very accurate when the i/p introduces additional cropping from
        234. But for now this is a very good approx
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


      /*--------------------------------------------------------------------
      color convert and store the closing pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;

      /*--------------------------------------------------------------------
        Increment the row_cnt so that we will move to the case where we
        need row averaging
      --------------------------------------------------------------------*/
      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
          Average with previous row. Starts at the outRawDump pointer
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx - 1);


      cb_dash  = ( *(outRawDump + 2) + cb ) >> 1;
      cr_dash  = ( *(outRawDump + 4) + cr ) >> 1;

      /*--------------------------------------------------------------------
        To get the even aligned half words we encode the row in this
        way.

        __________
        |B|L1.....
        ----------


        Where B is a blank value with is padding. The L1 starts the
        opening pixel.

        The Cb and Cr are taken from next pixel. THis is a very good appro-
        ximation
      --------------------------------------------------------------------*/
      lumaa_dash   = ( *( outRawDump + 1) + lumaa1 ) >> 1;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash, r, out,  \
        rTable, gTable, bTable);

      /*--------------------------------------------------------------------
              render the first averaged pixel
      --------------------------------------------------------------------*/
      * ( (uint16 *) outRawDump ) = out;
      outRawDump += 2;

      /*--------------------------------------------------------------------
          iterate in for the col_max by 2. This takes 6 input pixels
          in a row.
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {

        /*------------------------------------------------------------------
          Get the remaining lumaa2 of previous pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Deal with Distance and 0.75 sampling interval
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2) ;


        /*------------------------------------------------------------------
            Average the values with previous row.

            pixel(n - dx)  = ( pixel(n) + pixel(n - dx ) ) >> 1
        ------------------------------------------------------------------*/
        *(outRawDump ) =    (uint8) ((*outRawDump  + cb ) >> 1);
        *(outRawDump + 1) = (uint8) ((*(outRawDump + 1) + lumaa1 ) >> 1);
        *(outRawDump + 2) = (uint8) ((*(outRawDump + 2) + cr ) >> 1);

        /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 0.25
        ----------------------------------------------------------------*/

        cb_dash = *((uint8*)(inputImgPtr++));

        /*----------------------------------------------------------------
            Pixel 2 + n* 3
        ----------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1) ;

        cb = cb + ( ( cb_dash - cb ) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );
        /*----------------------------------------------------------------
          Do average with previous row
        ----------------------------------------------------------------*/
        *( outRawDump + 3) = (uint8) ((*(outRawDump + 3) + lumaa2) >> 1);

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 2.25 pitch 2 - 3 combo no need for Cr Cb change
        ----------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.25 distance averaging
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
         we are saving lumaa1 since the following macro distroys the
         value as it exceutes
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;

        /*------------------------------------------------------------------
            Average the values with previous row.

            pixel(n - dx)  = ( pixel(n) + pixel(n - dx ) ) >> 1
        ------------------------------------------------------------------*/
        *( outRawDump + 4 ) = (uint8) ((*(outRawDump + 4) + cb_dash) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*(outRawDump + 5) + lumaa2)  >> 1);
        *( outRawDump + 6 ) = (uint8) ((*(outRawDump + 6) + cr_dash) >> 1);
        *( outRawDump + 7 ) = (uint8) ((*(outRawDump + 7) + lumaa1)  >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
            Now we have completed pixels for the averaged row
            lets read those out and render them after color conversion
        ------------------------------------------------------------------*/
        cb = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
          r, out, out2, \
          rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            read the next pixel pair in the averaged row.
        ------------------------------------------------------------------*/
        cb = *(outRawDump );
        lumaa2 = *(outRawDump+1);
        cr = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
          r, out, out2, \
          rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


        /*------------------------------------------------------------------
              Next set of pixel pairs
       -------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            distance 0.75 sampled interval
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);
        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.5 interval just the plain avreage
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 ) ;

        /*----------------------------------------------------------------
            Average the values with previous row.
        ----------------------------------------------------------------*/
        *( outRawDump )     = (uint8) ((*(outRawDump) + cb_dash ) >> 1);
        *( outRawDump + 1 ) = (uint8) ((*(outRawDump + 1) + lumaa1 ) >> 1);
        *( outRawDump + 2 ) = (uint8) ((*(outRawDump + 2) + cr_dash ) >> 1);
        *( outRawDump + 3 ) = (uint8) ((*(outRawDump + 3) + lumaa2 ) >> 1);

        /*------------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all
          the templates used in this file
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*----------------------------------------------------------------
                Deal with Distance 0.25
        ----------------------------------------------------------------*/
        cb = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            distance averaging for 0.25 sampled interval
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2) ;

        /*------------------------------------------------------------------
            average with the previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 4 ) = (uint8) ((*(outRawDump + 4) + cb ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*(outRawDump + 5) + lumaa_dash ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*(outRawDump + 6) + cr ) >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          average with previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 7 ) = (uint8) ((*(outRawDump + 7) + lumaa1 ) >> 1);

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Read out the averaged values from previous row and render them
            after color conversion
        ------------------------------------------------------------------*/
        cb_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            Secoond pixel pair after color conversion
        ------------------------------------------------------------------*/
        cb_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


      } /* End of col loop */


      /*----------------------------------------------------------------
        This is the closing pixel in the QCIF image
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*------------------------------------------------------------------
          distance 0.75 sampled interval
      ------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2) ;

      /*----------------------------------------------------------------
          Average the values with previous row.
      ----------------------------------------------------------------*/
      *(outRawDump ) =    (uint8) ((*outRawDump  + cb ) >> 1);
      *(outRawDump + 1) = (uint8) ((*(outRawDump + 1) + lumaa1 ) >> 1);
      *(outRawDump + 2) = (uint8) ((*(outRawDump + 2) + cr ) >> 1);

      /*----------------------------------------------------------------
      Following piece of code is a template for the YCrCB to RGB
      conversion when you find some bug on this make sure you fix all the
      templates used in this file
      ----------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;

      /*--------------------------------------------------------------------
          Closing pixel for the averaged row.
      --------------------------------------------------------------------*/
      cb = *(outRawDump );
      lumaa2 = *(outRawDump + 1);
      cr = *(outRawDump + 2);
      lumaa_dash = *(outRawDump + 3);

      /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      ---------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
        r, out, out2, \
        rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

      /*--------------------------------------------------------------------
          Once we reach the third row we will start the iteration all over

        The rows can be devided in three groups. One which does not
        participate in averaging one which does and one after the avraged
        row and participate in ageraging.

        ____________________________________________________
        |__________________________row0_____________________|
        |_______________________row1________________________|
        |_______________________avg_________________________|
        |________________________row2_______________________|
        |   ...........................................     |
        |___________________________________________________|


        2 rows are copied as is and then there is avg = (row1 + row2) /2

        where the addition is matrix addition.

        This cycle repeats. So the o/p is upsized by

        4/3. Because each 3 input rows create 4 o/p rows. Actually each row
        needs to be weighted aveaged. But we limit it to the row which
        falls in the 0.5 sampled interval

      --------------------------------------------------------------------*/
      row_cnt = 0;

    }
    else if ( row_cnt == 1 )
    {

      /*----------------------------------------------------------------
          Average with previous row
      ----------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx - 1);

      /*--------------------------------------------------------------------
          To get the even aligned half words we encode the row in this
          way.

          __________
          |B|L1.....
          ----------


          Where B is a blank value with is padding. The L1 starts the
          opening pixel.

          The Cb and Cr are taken from next pixel. THis is a very good appro-
          ximation
      --------------------------------------------------------------------*/
      outRawDump++;
      *outRawDump++ = (uint8) lumaa1;

      for ( col = col_max ; col; col-=2 )
      {


        /*----------------------------------------------------------------
          Get the next lumma2 of the past pixel
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            weighted average for the 0.75 distance from the previous lumaa
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);



        /*----------------------------------------------------------------
           Store thre resluting  pixel pair in the next row.
        ----------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr;

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all
          the templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
              Get the next pixel pair and Deal with Distance 0.5 distance.
        ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            render the pixel right in the middle of 2 pixel pairs. Need
            to avrage everything
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );
        cb = cb + ( ( cb_dash - cb ) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );

        /*------------------------------------------------------------------
              Render the lone pixel with just lumaa on the previous row.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
            Render the stand alone averaged pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          Deal with Distance 0.25 pitch sample distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            weighted average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( ( lumaa1 - lumaa_dash ) >> 2 );


        /*------------------------------------------------------------------
            Copy the new formed pixel pair to the next row we are averaring
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa2;
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            Save the Lumaa1 so that the macro wont destory it. This is
            distance 0.25 and 0
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;

        /*------------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next up is 0.75 distance and next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3 >> 2 );

        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Distance 0.5 sample space
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );

        /*------------------------------------------------------------------
            copy the values to the previous row.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa2;

        /*----------------------------------------------------------------
          Color convert and store the pixel pair
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*----------------------------------------------------------------
            Distance 0.25 from the current input
        ----------------------------------------------------------------*/
        cb = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.25 weighted averaging on Lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa_dash;
        *outRawDump++ = (uint8) cr;

        /*----------------------------------------------------------------
          Color convert and store the pixel pair
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Copy the stand alone lumaa to the previous row.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa1;

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

      } /* End of col loop */


      /*--------------------------------------------------------------------
              Closing pixel
      --------------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
          Weighted average for 0.75 sample distance
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

      /*----------------------------------------------------------------
         Store the pixel in the next row for previous averaging
      ----------------------------------------------------------------*/
      *outRawDump++ = (uint8) cb;
      *outRawDump++ = (uint8) lumaa1;
      *outRawDump++ = (uint8) cr;

      /*----------------------------------------------------------------
          Color convert and store the pixel
      ----------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                      rTable, gTable, bTable);
      *outputImgPtr++ = out;

      /*------------------------------------------------------------------
        Since we just created a row for averaging we need to add the size
        of one row the outputImgPtr. When we add the destInc the pointer
        will align to the next row in output
      ------------------------------------------------------------------*/
      outputImgPtr += outDx;

      row_cnt++;

    } /* end of col loop  */


    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  );

    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

}/* end of function ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133 */






/*===========================================================================

FUNCTION ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIF_133()

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2 ;
  register int32 cr, cb, cb_dash, cr_dash;
  register int32 lumaa1, lumaa_dash;
  register int32 lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */
  uint32 row,col_max, row_max;
  uint32 rowInc=0, dest_index, destInc;
  uint32   row_cnt, col;
  /*-----------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133PT\n");


   /*------------------------------------------------------------------------
      QCIF in the portrait  mode   not very common
  ------------------------------------------------------------------------*/
  if( !( (i_img_ptr->dy == IPL2_QCIF_WIDTH ) &&
        (i_img_ptr->dx == IPL2_QCIF_HEIGHT ) ) )
  {

   /*----------------------------------------------------------------------
      We do not support non QCIF images. They should be QCIF rotated in any
      angle.
    ----------------------------------------------------------------------*/
    return( IPL_FAILURE );


  }

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;



  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

    /*--------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
    ----------------------------------------------------------------------*/
    col_max =  ( ( i_param->crop.dx )  / 4 );


    /*----------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
    ----------------------------------------------------------------------*/
    if ( col_max > IPL2_QCIF_COL_CNTPT_BY_3 )
    {
      col_max = IPL2_QCIF_COL_CNTPT_BY_3;
    }
    else if ( col_max & 0x1 )
    {
      col_max--;
    }

    /*----------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
    ----------------------------------------------------------------------*/
    rowInc = ( IPL2_QCIF_HEIGHT
             - ( col_max * 3   )  ) * 2;



    /*------------------------------------------------------------------------
        Limit the maximun no of rows to the QCIF size
    ------------------------------------------------------------------------*/
    if ( row_max > IPL2_QCIF_WIDTH )
    {
      row_max = IPL2_QCIF_WIDTH;
    }





  outputImgPtr = (uint16 *) ((uint32)outputImgPtr +  dest_index);

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 ) ) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;




  /*------------------------------------------------------------------------
      iterate through the image with row_max and clo_max for the loop
  ------------------------------------------------------------------------*/
  for ( row = row_max; row; row-- )
  {


    /*----------------------------------------------------------------------
                 This is Cb
   -----------------------------------------------------------------------*/
    cb = (int32) (*((uint8*)(inputImgPtr++)));

    /*--------------------------------------------------------------------
                Lumaa
     -------------------------------------------------------------------*/
    lumaa1 = *((uint8*)(inputImgPtr++));

    /*--------------------------------------------------------------------
              cr
     -------------------------------------------------------------------*/
    cr = (int32) (*((uint8*) (inputImgPtr++)));


    /*----------------------------------------------------------------------
        The rows can be devided in three groups. One which does not
        participate in averaging one which does and one after the avraged
        row and participate in ageraging.

        ____________________________________________________
        |__________________________row0_____________________|
        |_______________________row1________________________|
        |_______________________avg_________________________|
        |________________________row2_______________________|
        |   ...........................................     |
        |___________________________________________________|


        2 rows are copied as is and then there is avg = (row1 + row2) /2

        where the addition is matrix addition.

        This cycle repeats. So the o/p is upsized by

        4/3. Because each 3 input rows create 4 o/p rows. Actually each row
        needs to be weighted aveaged. But we limit it to the row which
        falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/
    if ( row_cnt == 0 )
    {


      /*--------------------------------------------------------------------
          iterate in for the col_max by 2. This takes 6 input pixels
          in a row.
      --------------------------------------------------------------------*/
      for ( col = col_max ; col; col-=2 )
      {


        /*------------------------------------------------------------------
          Pixel 2
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Pixel = pixel 1 + ( pixel2 - pixel 1 ) * 0.75 or 3/4
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


        /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          Deal with Distance 1.5 pitch 1 - combo
        ------------------------------------------------------------------*/


        /*------------------------------------------------------------------
            Take the next pixel pair and try averaging it
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            pixel = ( pixel( n ) + pixel(n + 1 ) ) / 2
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1) ;
        cb = cb + ( ( cb_dash - cb ) >> 1);
        cr = cr + ( ( cr_dash - cr ) >> 1 );

        /*------------------------------------------------------------------
            Render the just aceraged pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with sampling distance of 0.25
        ----------------------------------------------------------------*/
        /*----------------------------------------------------------------
            Pixel 3 + n * 3
        ----------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        lumaa_dash = lumaa1;


        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Render 0.25 and 0 distance pixels
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*----------------------------------------------------------------
            Take the next pixel pair
       ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
              0.75 sampling distance averaging
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
            Next sample at 0.75 distance
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
              0.5 averaging
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;




        /*------------------------------------------------------------------
          Deal with Distance .25 distance
        ------------------------------------------------------------------*/
        cb = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = *((uint8*)(inputImgPtr++));

        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file. Convert single pixel
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

      } /* End of col loop */


      /*--------------------------------------------------------------------
        Increment the row_cnt so that we will move to the case where we
        need row averaging
      --------------------------------------------------------------------*/
      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
          Average with previous row. Starts at the outRawDump pointer
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx );


      /*--------------------------------------------------------------------
        To get the even aligned half words we encode the row in this
        way.

        __________
        |B|L1.....
        ----------


        Where B is a blank value with is padding. The L1 starts the
        opening pixel.

        The Cb and Cr are taken from next pixel. THis is a very good appro-
        ximation
      --------------------------------------------------------------------*/

      /*--------------------------------------------------------------------
          iterate in for the col_max by 2. This takes 6 input pixels
          in a row.
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {

        /*------------------------------------------------------------------
          Get the remaining lumaa2 of previous pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Deal with Distance and 0.75 sampling interval
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2) ;


        /*------------------------------------------------------------------
            Average the values with previous row.

            pixel(n - dx)  = ( pixel(n) + pixel(n - dx ) ) >> 1
        ------------------------------------------------------------------*/
        *(outRawDump ) =    (uint8) ((*outRawDump  + cb ) >> 1);
        *(outRawDump + 1) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *(outRawDump + 2) = (uint8) ((*( outRawDump + 2) + cr ) >> 1);

        /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 0.25
        ----------------------------------------------------------------*/

        cb_dash = *((uint8*)(inputImgPtr++));

        /*----------------------------------------------------------------
            Pixel 2 + n* 3
        ----------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1) ;

        cb = cb + ( ( cb_dash - cb ) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );
        /*----------------------------------------------------------------
          Do average with previous row
        ----------------------------------------------------------------*/
        *( outRawDump + 3) = (uint8) ((*(outRawDump + 3) + lumaa2 ) >> 1);

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 2.25 pitch 2 - 3 combo no need for Cr Cb change
        ----------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.25 distance averaging
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
         we are saving lumaa1 since the following macro distroys the
         value as it exceutes
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;

        /*------------------------------------------------------------------
            Average the values with previous row.

            pixel(n - dx)  = ( pixel(n) + pixel(n - dx ) ) >> 1
        ------------------------------------------------------------------*/
        *( outRawDump + 4 ) = (uint8) ((*(outRawDump + 4 ) + cb_dash ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*(outRawDump + 5 ) + lumaa2 ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*(outRawDump + 6 ) + cr_dash ) >> 1);
        *( outRawDump + 7 ) = (uint8) ((*(outRawDump + 7 ) + lumaa1 ) >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
            Now we have completed pixels for the averaged row
            lets read those out and render them after color conversion
        ------------------------------------------------------------------*/
        cb = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
          r, out, out2, \
          rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            read the next pixel pair in the averaged row.
        ------------------------------------------------------------------*/
        cb = *(outRawDump );
        lumaa2 = *(outRawDump +1);
        cr = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
          r, out, out2, \
          rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


        /*------------------------------------------------------------------
              Next set of pixel pairs
       -------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            distance 0.75 sampled interval
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);
        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.5 interval just the plain avreage
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 ) ;

        /*----------------------------------------------------------------
            Average the values with previous row.
        ----------------------------------------------------------------*/
        *( outRawDump )     = (uint8) ((*( outRawDump) + cb_dash ) >> 1);
        *( outRawDump + 1 ) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *( outRawDump + 2 ) = (uint8) ((*( outRawDump + 2) + cr_dash ) >> 1);
        *( outRawDump + 3 ) = (uint8) ((*( outRawDump + 3) + lumaa2 ) >> 1);

        /*------------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all
          the templates used in this file
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*----------------------------------------------------------------
                Deal with Distance 0.25
        ----------------------------------------------------------------*/
        cb = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            distance averaging for 0.25 sampled interval
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2) ;

        /*------------------------------------------------------------------
            average with the previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 4 ) = (uint8) ((*(outRawDump + 4) + cb ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*(outRawDump + 5) + lumaa_dash ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*(outRawDump + 6) + cr ) >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          average with previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 7 ) = (uint8) ((*(outRawDump + 7) + lumaa1 ) >> 1);

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Read out the averaged values from previous row and render them
            after color conversion
        ------------------------------------------------------------------*/
        cb_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            Secoond pixel pair after color conversion
        ------------------------------------------------------------------*/
        cb_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


      } /* End of col loop */



      /*--------------------------------------------------------------------
          Once we reach the third row we will start the iteration all over

        The rows can be devided in three groups. One which does not
        participate in averaging one which does and one after the avraged
        row and participate in ageraging.

        ____________________________________________________
        |__________________________row0_____________________|
        |_______________________row1________________________|
        |_______________________avg_________________________|
        |________________________row2_______________________|
        |   ...........................................     |
        |___________________________________________________|


        2 rows are copied as is and then there is avg = (row1 + row2) /2

        where the addition is matrix addition.

        This cycle repeats. So the o/p is upsized by

        4/3. Because each 3 input rows create 4 o/p rows. Actually each row
        needs to be weighted aveaged. But we limit it to the row which
        falls in the 0.5 sampled interval

      --------------------------------------------------------------------*/
      row_cnt = 0;

    }
    else if ( row_cnt == 1 )
    {

      /*----------------------------------------------------------------
          Average with previous row
      ----------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx );

      /*--------------------------------------------------------------------
          To get the even aligned half words we encode the row in this
          way.

          __________
          |B|L1.....
          ----------


          Where B is a blank value with is padding. The L1 starts the
          opening pixel.

          The Cb and Cr are taken from next pixel. THis is a very good appro-
          ximation
      --------------------------------------------------------------------*/

      for ( col = col_max ; col; col-=2 )
      {


        /*----------------------------------------------------------------
          Get the next lumma2 of the past pixel
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            weighted average for the 0.75 distance from the previous lumaa
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);



        /*----------------------------------------------------------------
           Store thre resluting  pixel pair in the next row.
        ----------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr;

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all
          the templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
              Get the next pixel pair and Deal with Distance 0.5 distance.
        ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            render the pixel right in the middle of 2 pixel pairs. Need
            to avrage everything
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );
        cb = cb + ( ( cb_dash - cb ) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );

        /*------------------------------------------------------------------
              Render the lone pixel with just lumaa on the previous row.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
            Render the stand alone averaged pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          Deal with Distance 0.25 pitch sample distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            weighted average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( ( lumaa1 - lumaa_dash ) >> 2 );


        /*------------------------------------------------------------------
            Copy the new formed pixel pair to the next row we are averaring
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa2;
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            Save the Lumaa1 so that the macro wont destory it. This is
            distance 0.25 and 0
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;

        /*------------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next up is 0.75 distance and next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3 >> 2 );

        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Distance 0.5 sample space
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );

        /*------------------------------------------------------------------
            copy the values to the previous row.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa2;

        /*----------------------------------------------------------------
          Color convert and store the pixel pair
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*----------------------------------------------------------------
            Distance 0.25 from the current input
        ----------------------------------------------------------------*/
        cb = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.25 weighted averaging on Lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa_dash;
        *outRawDump++ = (uint8) cr;

        /*----------------------------------------------------------------
          Color convert and store the pixel pair
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Copy the stand alone lumaa to the previous row.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa1;

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

      } /* End of col loop */


      /*--------------------------------------------------------------------
        Since we just created a row for averaging we need to add the size
        of one row the outputImgPtr. When we add the destInc the pointer
        will align to the next row in output
      --------------------------------------------------------------------*/
      outputImgPtr += outDx;

      row_cnt++;

    } /* end of col loop  */


    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  - 3 );

    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

}/* end of function ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFHigh_133PT */


/*===========================================================================

FUNCTION IPL2_UPSIZEAVERAGEANDCROP_YCBCR2RGBQVGAMED()

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3;
  register int32 cr, cb, cb_dash, cr_dash;
  register int32 lumaa1, lumaa_dash;
  register int32 lumaa2, lumaa2_dash;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */
  uint32 row,col_max, row_max;
  uint32 rowInc=0, dest_index, destInc;
  uint32   row_cnt, col;
/*------------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
  ------------------------------------------------------------------------*/
  col_max =  ( ( i_param->crop.dx - 2)  / 4 );

  /*------------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
  ------------------------------------------------------------------------*/
  if ( col_max > IPL2_QCIF_COL_CNTLS_BY_3 )
  {
    col_max = IPL2_QCIF_COL_CNTLS_BY_3;
  }
  else if ( col_max & 0x1 )
  {
    col_max--;
  }



  /*------------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
  ------------------------------------------------------------------------*/
  rowInc = ( IPL2_QCIF_WIDTH
             - ( col_max * 3   + 2)  ) * 2;

  outputImgPtr = (uint16 *) ((uint32)outputImgPtr +  dest_index);

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 + 2 ) ) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

  /*------------------------------------------------------------------------
      Limit the maximun no of rows to the QCIF size
  ------------------------------------------------------------------------*/
  if ( row_max > IPL2_QCIF_HEIGHT )
  {
    row_max = IPL2_QCIF_HEIGHT;
  }

  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  for ( row = row_max; row; row-- )
  {

    /*----------------------------------------------------------------------
      The rows can be devided in three groups. One which does not
      participate in averaging one which does and one after the avraged
      row and participate in ageraging.

      ____________________________________________________
      |__________________________row0_____________________|
      |_______________________row1________________________|
      |_______________________avg_________________________|
      |________________________row2_______________________|
      |   ...........................................     |
      |___________________________________________________|


      2 rows are copied as is and then there is avg = (row1 + row2) /2

      where the addition is matrix addition.

      This cycle repeats. So the o/p is upsized by

      4/3. Because each 3 input rows create 4 o/p rows. Actually each row
      needs to be weighted aveaged. But we limit it to the row which
      falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/

    if ( row_cnt == 0 )
    {

      /*--------------------------------------------------------------------
          Previous calculation of col_max guarantees that the col_max
          is a loweer even so we can count 2 zero and do 6 pixels at
          a time. Works well with most cases. If someone is using an odd
          size they will lose couple pixels. But then they should ask
          themselves why are they doing it ..?
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {


        /*--------------------------------------------------------------------
                   Opening pixel Cb/l1/cr/l2 of the first pixel pair
         --------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(inputImgPtr++)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (inputImgPtr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));


        /*------------------------------------------------------------------
          save the current lumaa before the macro kills it
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
          color convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
          cr, cb, r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixels as is
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
             read the next pixel pairs first pixel info
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            This pixel needs to be averaged its sampled at 0.5. Remainings
            are approximated
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;


        /*------------------------------------------------------------------
              Color convert and store the averaged pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Next half of the current pixel which is just lumaa2
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));


        /*------------------------------------------------------------------
            This is a 0.25 and a zero distance averges. 0.25 is averaged to
            nearest neighbor
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;




        /*----------------------------------------------------------------
             get the next pixel pair
       ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            In this pixel pair one should be averaged
        ------------------------------------------------------------------*/
        lumaa_dash = (lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
             These 3 pixels share the same cr and cb so just do the
             more efficient conversion macro.
         ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, out3, \
          rTable, gTable, bTable);
        *( outputImgPtr++)= out;
        *( outputImgPtr++)= out2;
        *( outputImgPtr++)= out3;


      } /* End of col loop */


      /*--------------------------------------------------------------------
              read out the closing pixel
      --------------------------------------------------------------------*/
      cb = *((uint8*)(inputImgPtr++));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = *((uint8*)(inputImgPtr++));
      lumaa2 = *((uint8*)(inputImgPtr++));




      /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      ----------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb, r, out, out2,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;

      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
          The previous row should be averaged with current row and convert
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx );

      /*--------------------------------------------------------------------
          Sixe pixels in oe pass
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {

        /*------------------------------------------------------------------
                   get the first pixel pair
        ------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(inputImgPtr++)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (inputImgPtr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*----------------------------------------------------------------
            Average the values with previous row.
        ----------------------------------------------------------------*/
        cb_dash = ( *outRawDump  + cb ) >> 1;
        lumaa_dash = ( *( outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = ( *( outRawDump + 2) + cr ) >> 1;
        lumaa2_dash = ( *( outRawDump + 3) + lumaa2 ) >> 1;

        /*------------------------------------------------------------------
              Color convert and render the this pixel pair to the
              previous row
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa_dash, lumaa2_dash,
          cr_dash, cb_dash, r, out, out2, \
          rTable, gTable, bTable);

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
              Save Lumaa2 for the next averaging the conversion macro
              destroys second Lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
            Color convert and render the pixels in the current row
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
          cr, cb, r, out, out2, \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
            Get the next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Just average and render from previous pixel to
            pixel 1 of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;

        /*------------------------------------------------------------------
            Unrelated pixel we need to convert as a single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Average this with the previous rows pixel lumaa
        ------------------------------------------------------------------*/
        lumaa2 = ( *outRawDump  + lumaa2 ) >> 1;

        /*------------------------------------------------------------------
            Render this stand alone pixel use the cb and cr of this row.
            We are approximating this here.
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *((uint16 *) outRawDump) = out;
        outRawDump += 2;


        /*------------------------------------------------------------------
            Get the last lumaa of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          save the lumaa2 because the following macro will destroy this
          on execution
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all
          the templates used in this file
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair in o/p array
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
           Average the values with previous row.
       -------------------------------------------------------------------*/
        cb_dash = ( *outRawDump  + cb_dash ) >> 1;
        lumaa_dash = ( *( outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = ( *( outRawDump + 2) + cr_dash ) >> 1;
        lumaa2_dash = ( *( outRawDump + 3) + lumaa_dash ) >> 1;

        /*------------------------------------------------------------------
          find the RGB values and store in the averaged row
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa_dash, lumaa2_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);
        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


        /*------------------------------------------------------------------
                  Next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        lumaa_dash = ( lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
            Convert and store the single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash, cr_dash, cb_dash, r,
          out, rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Single stand aloe pixel which needs averaging
        ------------------------------------------------------------------*/
        *( outputImgPtr + 1 )= out;


        /*------------------------------------------------------------------
          Save Lumaa2 for future use
        ------------------------------------------------------------------*/
        lumaa2_dash = lumaa2;

        /*------------------------------------------------------------------
              Convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Pixel before the stand alone pixel
        ------------------------------------------------------------------*/
        *( outputImgPtr)= out;

        /*------------------------------------------------------------------
          Pixel after the stand alone pixel
        ------------------------------------------------------------------*/
        *( outputImgPtr + 2)  = out2;

        /*------------------------------------------------------------------
            account for the three pixels we rendered now.
        ------------------------------------------------------------------*/
        outputImgPtr += 3;


        /*------------------------------------------------------------------
            Average the previous row
        ------------------------------------------------------------------*/
        cb_dash = (  ( *outRawDump ) + cb_dash ) >> 1;
        lumaa1 = (  *(outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = (  *(outRawDump + 2) + cr_dash ) >> 1;
        lumaa2 = (  *(outRawDump + 3) + lumaa2_dash) >> 1;
        lumaa2_dash = (  *( outRawDump  + 4) + lumaa_dash ) >> 1;

        /*------------------------------------------------------------------
            3 pixel conversion.
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, out3, \
          rTable, gTable, bTable);\

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out3;
        outRawDump += 2;

      } /* End of col loop */


      /*--------------------------------------------------------------------
        get the closing pixel pair
      --------------------------------------------------------------------*/
      cb = *((uint8*)(inputImgPtr++));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = *((uint8*)(inputImgPtr++));
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
          Get the average for closing pixel in the previous row.
      --------------------------------------------------------------------*/
      *(outRawDump ) =    (uint8) ((*outRawDump  + cb ) >> 1);
      *(outRawDump + 1) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
      *(outRawDump + 2) = (uint8) ((*( outRawDump + 2) + cr ) >> 1);
      *(outRawDump + 3) = (uint8) ((*( outRawDump + 3) + lumaa2 ) >> 1);

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb, r, out, out2,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;


      cb = *(outRawDump );
      lumaa1 = *(outRawDump + 1);
      cr = *(outRawDump + 2);
      lumaa2 = *(outRawDump + 3);

      /*--------------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb,
        r, out, out2, \
        rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


      /*--------------------------------------------------------------------
          Roll back row count for the next iteration
      --------------------------------------------------------------------*/
      row_cnt = 0;
    }
    else if ( row_cnt == 1 )
    {

      /*--------------------------------------------------------------------
          Get the next row which needs the averaging
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx );


      for ( col = col_max ; col; col-=2 )
      {

        /*------------------------------------------------------------------
                  Read the first pixel pair in the row.
         -----------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(inputImgPtr++)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (inputImgPtr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Store this pixel pair in the next pixel pair.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr;
        *outRawDump++ = (uint8) lumaa2;


        /*------------------------------------------------------------------
            Save the pixel for the
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
            convert and Store the double pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
          cr, cb, r, out, out2, \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
              next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average stand alone pixel
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;


        /*------------------------------------------------------------------
          convert and store the stand alone pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
                Store this stand alonde pixel in the next row and
                then hafl word align the row. This aids easy processing
                later. We are not storing cb and cr we will get it from the
                next row.
        ------------------------------------------------------------------*/
        ( *outRawDump++ )      = (uint8) lumaa2;
        outRawDump++;

        /*------------------------------------------------------------------
            closing lumaa of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            store the current pixel pair in the average row
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ =  (uint8) lumaa2;

        /*------------------------------------------------------------------
          Convert and store the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
              Next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average pixel falls in the middle of the pixel pair. No need to
            average cb and cr cool saving
        ------------------------------------------------------------------*/
        lumaa_dash = ( lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
              Store the pixel triplet in the next row and then increment
              the pointer by one. Again this will get half word alighned
              and the third pixel will not overwrite on the remaining pixel
        ------------------------------------------------------------------*/
        *outRawDump++  = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++   = (uint8) cr_dash;
        *outRawDump ++=  (uint8) lumaa2;
        *outRawDump ++=  (uint8) lumaa_dash;
        outRawDump ++;


        /*------------------------------------------------------------------
          color convert and store the pixel triplet
         -----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, out3, \
          rTable, gTable, bTable);\

          *( outputImgPtr++)= out;
        *( outputImgPtr++)  = out2;
        *( outputImgPtr++)  = out3;

      } /* end of col loop */

      /*--------------------------------------------------------------------
        Get the closing pixel pair in the row
      --------------------------------------------------------------------*/
      cb = *((uint8*)(inputImgPtr++));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = *((uint8*)(inputImgPtr++));
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*---------------------------------------------------------------------
           Store the pixel pair in the following row which is being averaged
       --------------------------------------------------------------------*/
      *(outRawDump++) = (uint8) cb;
      *(outRawDump++) = (uint8) lumaa1;
      *(outRawDump++) = (uint8) cr;
      *(outRawDump++) = (uint8) lumaa2;

      /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
      ----------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb, r, out, out2,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;

      /*--------------------------------------------------------------------
          Increment the row cnt to get into the next stage
      --------------------------------------------------------------------*/
      row_cnt++;

      /*--------------------------------------------------------------------
          Advance to o/p to account for the row we just averaged
      --------------------------------------------------------------------*/
      outputImgPtr += outDx;

    } /* end of row_cnt == 1 */


    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  );

    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

}/* end of function  ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133*/

/*===========================================================================

FUNCTION IPL2_UPSIZEAVERAGEANDCROP_YCBCR2RGBQVGAMED()

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133_PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3;
  register int32 cr, cb, cb_dash, cr_dash;
  register int32 lumaa1, lumaa_dash;
  register int32 lumaa2, lumaa2_dash;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */
  uint32 row,col_max, row_max;
  uint32 rowInc=0, destInc;
  uint32   row_cnt, col;
/*------------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133PT\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

    /*--------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
    ----------------------------------------------------------------------*/
    col_max =  ( ( i_param->crop.dx )  / 4 );


    /*----------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
    ----------------------------------------------------------------------*/
    if ( col_max > IPL2_QCIF_COL_CNTPT_BY_3 )
    {
      col_max = IPL2_QCIF_COL_CNTPT_BY_3;
    }
    else if ( col_max & 0x1 )
    {
      col_max--;
    }

    /*----------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
    ----------------------------------------------------------------------*/
    rowInc = ( IPL2_QCIF_HEIGHT
             - ( col_max * 3   )  ) * 2;



    /*------------------------------------------------------------------------
        Limit the maximun no of rows to the QCIF size
    ------------------------------------------------------------------------*/
    if ( row_max > IPL2_QCIF_WIDTH )
    {
      row_max = IPL2_QCIF_WIDTH;
    }


  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 ) ) * 2;


  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;

  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  for ( row = row_max; row; row-- )
  {

    /*----------------------------------------------------------------------
      The rows can be devided in three groups. One which does not
      participate in averaging one which does and one after the avraged
      row and participate in ageraging.

      ____________________________________________________
      |__________________________row0_____________________|
      |_______________________row1________________________|
      |_______________________avg_________________________|
      |________________________row2_______________________|
      |   ...........................................     |
      |___________________________________________________|


      2 rows are copied as is and then there is avg = (row1 + row2) /2

      where the addition is matrix addition.

      This cycle repeats. So the o/p is upsized by

      4/3. Because each 3 input rows create 4 o/p rows. Actually each row
      needs to be weighted aveaged. But we limit it to the row which
      falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/

    if ( row_cnt == 0 )
    {

      /*--------------------------------------------------------------------
          Previous calculation of col_max guarantees that the col_max
          is a loweer even so we can count 2 zero and do 6 pixels at
          a time. Works well with most cases. If someone is using an odd
          size they will lose couple pixels. But then they should ask
          themselves why are they doing it ..?
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {


        /*--------------------------------------------------------------------
                   Opening pixel Cb/l1/cr/l2 of the first pixel pair
         --------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(inputImgPtr++)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (inputImgPtr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));


        /*------------------------------------------------------------------
          save the current lumaa before the macro kills it
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
          color convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
          cr, cb, r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixels as is
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
             read the next pixel pairs first pixel info
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            This pixel needs to be averaged its sampled at 0.5. Remainings
            are approximated
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;


        /*------------------------------------------------------------------
              Color convert and store the averaged pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Next half of the current pixel which is just lumaa2
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));


        /*------------------------------------------------------------------
            This is a 0.25 and a zero distance averges. 0.25 is averaged to
            nearest neighbor
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;




        /*----------------------------------------------------------------
             get the next pixel pair
       ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            In this pixel pair one should be averaged
        ------------------------------------------------------------------*/
        lumaa_dash = (lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
             These 3 pixels share the same cr and cb so just do the
             more efficient conversion macro.
         ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, out3, \
          rTable, gTable, bTable);
        *( outputImgPtr++)= out;
        *( outputImgPtr++)= out2;
        *( outputImgPtr++)= out3;


      } /* End of col loop */

      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
          The previous row should be averaged with current row and convert
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx );

      /*--------------------------------------------------------------------
          Sixe pixels in oe pass
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {

        /*------------------------------------------------------------------
                   get the first pixel pair
        ------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(inputImgPtr++)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (inputImgPtr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*----------------------------------------------------------------
            Average the values with previous row.
        ----------------------------------------------------------------*/
        cb_dash = ( *outRawDump  + cb ) >> 1;
        lumaa_dash = ( *( outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = ( *( outRawDump + 2) + cr ) >> 1;
        lumaa2_dash = ( *( outRawDump + 3) + lumaa2 ) >> 1;

        /*------------------------------------------------------------------
              Color convert and render the this pixel pair to the
              previous row
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa_dash, lumaa2_dash,
          cr_dash, cb_dash, r, out, out2, \
          rTable, gTable, bTable);

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
              Save Lumaa2 for the next averaging the conversion macro
              destroys second Lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
            Color convert and render the pixels in the current row
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
          cr, cb, r, out, out2, \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
            Get the next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Just average and render from previous pixel to
            pixel 1 of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;

        /*------------------------------------------------------------------
            Unrelated pixel we need to convert as a single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Average this with the previous rows pixel lumaa
        ------------------------------------------------------------------*/
        lumaa2 = ( *outRawDump  + lumaa2 ) >> 1;

        /*------------------------------------------------------------------
            Render this stand alone pixel use the cb and cr of this row.
            We are approximating this here.
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *((uint16 *) outRawDump) = out;
        outRawDump += 2;


        /*------------------------------------------------------------------
            Get the last lumaa of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          save the lumaa2 because the following macro will destroy this
          on execution
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all
          the templates used in this file
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair in o/p array
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
           Average the values with previous row.
       -------------------------------------------------------------------*/
        cb_dash = ( *outRawDump  + cb_dash ) >> 1;
        lumaa_dash = ( *( outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = ( *( outRawDump + 2) + cr_dash ) >> 1;
        lumaa2_dash = ( *( outRawDump + 3) + lumaa_dash ) >> 1;

        /*------------------------------------------------------------------
          find the RGB values and store in the averaged row
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa_dash, lumaa2_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);
        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


        /*------------------------------------------------------------------
                  Next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        lumaa_dash = ( lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
            Convert and store the single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash, cr_dash, cb_dash, r,
          out, rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Single stand aloe pixel which needs averaging
        ------------------------------------------------------------------*/
        *( outputImgPtr + 1 )= out;


        /*------------------------------------------------------------------
          Save Lumaa2 for future use
        ------------------------------------------------------------------*/
        lumaa2_dash = lumaa2;

        /*------------------------------------------------------------------
              Convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Pixel before the stand alone pixel
        ------------------------------------------------------------------*/
        *( outputImgPtr)= out;

        /*------------------------------------------------------------------
          Pixel after the stand alone pixel
        ------------------------------------------------------------------*/
        *( outputImgPtr + 2)  = out2;

        /*------------------------------------------------------------------
            account for the three pixels we rendered now.
        ------------------------------------------------------------------*/
        outputImgPtr += 3;


        /*------------------------------------------------------------------
            Average the previous row
        ------------------------------------------------------------------*/
        cb_dash = (  ( *outRawDump ) + cb_dash ) >> 1;
        lumaa1 = (  *(outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = (  *(outRawDump + 2) + cr_dash ) >> 1;
        lumaa2 = (  *(outRawDump + 3) + lumaa2_dash) >> 1;
        lumaa2_dash = (  *( outRawDump  + 4) + lumaa_dash ) >> 1;

        /*------------------------------------------------------------------
            3 pixel conversion.
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, out3, \
          rTable, gTable, bTable);\
       
        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out3;
        outRawDump += 2;

      } /* End of col loop */


      /*--------------------------------------------------------------------
          Roll back row count for the next iteration
      --------------------------------------------------------------------*/
      row_cnt = 0;
    }
    else if ( row_cnt == 1 )
    {

      /*--------------------------------------------------------------------
          Get the next row which needs the averaging
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx );


      for ( col = col_max ; col; col-=2 )
      {

        /*------------------------------------------------------------------
                  Read the first pixel pair in the row.
         -----------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(inputImgPtr++)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (inputImgPtr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Store this pixel pair in the next pixel pair.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr;
        *outRawDump++ =  (uint8) lumaa2;


        /*------------------------------------------------------------------
            Save the pixel for the
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
            convert and Store the double pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
          cr, cb, r, out, out2, \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
              next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average stand alone pixel
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;


        /*------------------------------------------------------------------
          convert and store the stand alone pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
                Store this stand alonde pixel in the next row and
                then hafl word align the row. This aids easy processing
                later. We are not storing cb and cr we will get it from the
                next row.
        ------------------------------------------------------------------*/
        ( *outRawDump++ )      = (uint8) lumaa2;
        outRawDump++;

        /*------------------------------------------------------------------
            closing lumaa of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            store the current pixel pair in the average row
        ------------------------------------------------------------------*/
        *outRawDump++       = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++   = (uint8) cr_dash;
        *outRawDump++ =  (uint8) lumaa2;

        /*------------------------------------------------------------------
          Convert and store the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
              Next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = *((uint8*)(inputImgPtr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(inputImgPtr++));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average pixel falls in the middle of the pixel pair. No need to
            average cb and cr cool saving
        ------------------------------------------------------------------*/
        lumaa_dash = ( lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
              Store the pixel triplet in the next row and then increment
              the pointer by one. Again this will get half word alighned
              and the third pixel will not overwrite on the remaining pixel
        ------------------------------------------------------------------*/
        *outRawDump++  = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++   = (uint8) cr_dash;
        *outRawDump ++=  (uint8) lumaa2;
        *outRawDump ++=  (uint8) lumaa_dash;
        outRawDump ++;


        /*------------------------------------------------------------------
          color convert and store the pixel triplet
         -----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, out3, \
          rTable, gTable, bTable);\

          *( outputImgPtr++)= out;
        *( outputImgPtr++)  = out2;
        *( outputImgPtr++)  = out3;

      } /* end of col loop */


      /*--------------------------------------------------------------------
          Increment the row cnt to get into the next stage
      --------------------------------------------------------------------*/
      row_cnt++;

      /*--------------------------------------------------------------------
          Advance to o/p to account for the row we just averaged
      --------------------------------------------------------------------*/
      outputImgPtr += outDx;

    } /* end of row_cnt == 1 */


    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  );

    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

}/* end of function  ipl2_upSizeAverageAndCrop_YCbCr2RGBQCIFMed_133_PT*/




/*===========================================================================

FUNCTION IPL2_UPSIZEAVERAGEANDCROP_YCBCR420TORGBQVGA()

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register uint16 out, out2;
  register int32 cb,cr;
  register int32 lumaa1, lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */
  register uint8 *cb_ptr;
  uint32 rowInc = 0, destInc, cr_offset;
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register int32 cb_dash, cr_dash;
  register int32  lumaa_dash;
  int32 row, col_max, row_max;
  int32   row_cnt, col;
/*------------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133\n");

  /*------------------------------------------------------------------------
      Determine the conversion table depending on the o/p color
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  } /* end of switch */

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cb_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
      Takes us to the offset for Cb Vals
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy  ) >> 2 /* /4 */;

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
  ------------------------------------------------------------------------*/
  col_max =  ( ( i_param->crop.dx - 2)  / 4 );

  /*------------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
  ------------------------------------------------------------------------*/
  if ( col_max > IPL2_QCIF_COL_CNTLS_BY_3 )
  {
    col_max = IPL2_QCIF_COL_CNTLS_BY_3;
  }
  else if ( col_max & 0x1 )
  {
    col_max--;
  }



  /*------------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
      For YCbCr420 this is eavh takes only 1 byte.
  ------------------------------------------------------------------------*/
  rowInc = ( IPL2_QCIF_WIDTH
             - ( col_max * 3   + 2)  ) ;

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 + 2 ) ) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

  /*------------------------------------------------------------------------
      Limit the maximun no of rows to the QCIF size
  ------------------------------------------------------------------------*/
  if ( row_max > IPL2_QCIF_HEIGHT )
  {
    row_max = IPL2_QCIF_HEIGHT;
  }

  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  for ( row = 0; row < row_max; row++ )
  {


    /*----------------------------------------------------------------------
        For every odd row we need to move the cb/cr ptrs so that they repeate
        the same for the this row as well. In 420 encoding 2 pixels in the
        next row share the same cr/cb values
    ----------------------------------------------------------------------*/
    if ( row & 0x1 )
    {
      cb_ptr -= ( i_img_ptr->dx) >> 1;
    }

    /*----------------------------------------------------------------------
      The rows can be devided in three groups. One which does not
      participate in averaging one which does and one after the avraged
      row and participate in ageraging.

      ____________________________________________________
      |__________________________row0_____________________|
      |_______________________row1________________________|
      |_______________________avg_________________________|
      |________________________row2_______________________|
      |   ...........................................     |
      |___________________________________________________|


      2 rows are copied as is and then there is avg = (row1 + row2) /2

      where the addition is matrix addition.

      This cycle repeats. So the o/p is upsized by

      4/3. Because each 3 input rows create 4 o/p rows. Actually each row
      needs to be weighted aveaged. But we limit it to the row which
      falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/

    /*----------------------------------------------------------------------
        Deal with row0 in the comment
    ----------------------------------------------------------------------*/
    if ( row_cnt == 0 )
    {

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*)(cb_ptr)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));

      /*--------------------------------------------------------------------
          Convert and render the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);
      *outputImgPtr++ = out;



      for ( col = col_max ; col; col-=2 )
      {


        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );



        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cb = ( cb +  cb_dash ) >> 1;
        cr = ( cr + cr_dash  ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = *((uint8*)(cb_ptr++ + cr_offset));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

      } /* End of col loop */


      /*----------------------------------------------------------------
      Read the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        Closing pixel is assumed to be in the 0.75 sampling distance. This
        may not be true if the o/p image is less thatn 234. But this is
        pretty colse assumption. So do it regardless of size
        to avoid complication
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


      /*--------------------------------------------------------------------
         color convert and save the pixel info
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;


      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
          Average with previous row
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx );


      /*--------------------------------------------------------------------
      Read out the opening pixel
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*)(cb_ptr)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));

      /*--------------------------------------------------------------------
      Color convert and store the opeing pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;

      /*--------------------------------------------------------------------
        Do the averaging from previous row and then sore it into the o/p
        buffer
      --------------------------------------------------------------------*/
      cb_dash  = ( *(outRawDump  + 2) + cb ) >> 1;
      cr_dash  = ( *( outRawDump + 4) + cr ) >> 1;
      lumaa_dash   = ( *( outRawDump + 1) + lumaa1 ) >> 1;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash, r,
        out, rTable, gTable, bTable);

      * ( (uint16 *) outRawDump ) = out;

      /*--------------------------------------------------------------------
        Advance the dumped row by one
      --------------------------------------------------------------------*/
      outRawDump += 2;

      /*--------------------------------------------------------------------
        iterate for one complete row .. 6 complete pixels
        on each iterations. This will give us a repetitive pattern for
        1.33 ( 4/3) upsizing)
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {


        /*----------------------------------------------------------------
      Get the L2 of the pixel pair from the previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          this is 0.75 pixels. Look at the position after a 6 pixel cycle
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2) ;

        /*----------------------------------------------------------------
            Average the values with previous row.
        ----------------------------------------------------------------*/
        *(outRawDump ) =    (uint8) ((*outRawDump  + cb ) >> 1);
        *(outRawDump + 1) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *(outRawDump + 2) = (uint8) ((*( outRawDump + 2) + cr ) >> 1);

        /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*----------------------------------------------------------------
            Calculate RGB for Lumaa4 1 - 2 combo
        ----------------------------------------------------------------*/

        /*----------------------------------------------------------------
          Deal with Distance 1.5 pitch 1 - combo
        ----------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));

        /*----------------------------------------------------------------
            Pixel 2 + n* 3
        ----------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));

        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1) ;

        cb = cb + ( ( cb_dash - cb ) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );

        /*------------------------------------------------------------------
          Do average with previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 3) = (uint8) ((*(outRawDump + 3) + lumaa2 ) >> 1);

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 2.25 pitch 2 - 3 combo no need for Cr Cb change
        ----------------------------------------------------------------*/
        /*----------------------------------------------------------------
            Pixel 3 + n * 3
        ----------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        lumaa_dash = lumaa1;

        *( outRawDump + 4 ) = (uint8) ((*( outRawDump + 4 ) + cb_dash ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*( outRawDump + 5 ) + lumaa2 ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*( outRawDump + 6 ) + cr_dash ) >> 1);
        *( outRawDump + 7 ) = (uint8) ((*( outRawDump + 7 ) + lumaa1 ) >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);


        *outputImgPtr++ = out;

        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
          Read out the averaged pixels from previous row
        ------------------------------------------------------------------*/
        cb = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
            Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
                                    r, out, out2, \
                                    rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
          read out the second pixel pair from the prevous row
        ------------------------------------------------------------------*/
        cb = *(outRawDump );
        lumaa2 = *(outRawDump +1);
        cr = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          COnvert and store the pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
                                r, out, out2, \
                                rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


        /*------------------------------------------------------------------
            Next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));

        /*------------------------------------------------------------------
            0.75 pixel distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.5 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 ) ;

        /*------------------------------------------------------------------
            Average the values with previous row.
        ------------------------------------------------------------------*/
        *( outRawDump )     = (uint8) ((*( outRawDump) + cb_dash ) >> 1);
        *( outRawDump + 1 ) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *( outRawDump + 2 ) = (uint8) ((*( outRawDump + 2) + cr_dash ) >> 1);
        *( outRawDump + 3 ) = (uint8) ((*( outRawDump + 3) + lumaa2 ) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
          Open the next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));

        /*------------------------------------------------------------------
            distance average for 0.25 sampling distnace
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2) ;

        /*------------------------------------------------------------------
          Average this pixel with the preevious pixel
        ------------------------------------------------------------------*/
        *( outRawDump + 4 ) = (uint8) ((*( outRawDump + 4) + cb ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*( outRawDump + 5) + lumaa_dash ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*( outRawDump + 6) + cr ) >> 1);

        /*------------------------------------------------------------------
            convert and store the single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
                                          r, out,  \
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            averaging the pixel lumaa with the previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 7 ) = (uint8) ((*( outRawDump + 7) + lumaa1 ) >> 1);

        /*------------------------------------------------------------------
              Convert and store this single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
                                          r, out,  \
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Take out the averaged values
        ------------------------------------------------------------------*/
        cb_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
            convert and store the pixel pair
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            Take out the next pixel pair from the averaged row
        ------------------------------------------------------------------*/
        cb_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


      } /* End of col loop */


      /*----------------------------------------------------------------
        Take out the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        The closing pixel is assumed in 0.75 sample space. This might not
        be very accurate when the i/p introduces additional cropping from
        234. But for now this is a very good approx
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2) ;


      /*--------------------------------------------------------------------
          Average the values with previous row.
      --------------------------------------------------------------------*/
      *(outRawDump ) =    (uint8) ((*outRawDump  + cb ) >> 1);
      *(outRawDump + 1) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
      *(outRawDump + 2) = (uint8) ((*( outRawDump + 2) + cr ) >> 1);

      /*--------------------------------------------------------------------
        Convert and store the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                      rTable, gTable, bTable);

      *outputImgPtr++ = out;

      /*--------------------------------------------------------------------
          Take the pixel pair from the averaged row and render it back
      --------------------------------------------------------------------*/
      cb = *(outRawDump );
      lumaa2 = *(outRawDump + 1);
      cr = *(outRawDump + 2);
      lumaa_dash = *(outRawDump + 3);

      /*--------------------------------------------------------------------
          Convert and store the pixel pair in the
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
        r, out, out2, \
        rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

      row_cnt = 0;
    }
    else if ( row_cnt == 1 )
    {

      /*--------------------------------------------------------------------
          Next row should be the average of 2 rows
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx);


      /*--------------------------------------------------------------------
        Take the opening half pixel for this row
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*)(cb_ptr)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                        rTable, gTable, bTable);

      *outputImgPtr++ = out;

      /*--------------------------------------------------------------------
        To get the even aligned half words we encode the row in this
        way.

        __________
        |B|L1.....
        ----------


        Where B is a blank value with is padding. The L1 starts the
        opening pixel.

        The Cb and Cr are taken from next pixel. THis is a very good appro-
        ximation
      --------------------------------------------------------------------*/
      outRawDump++;
      *outRawDump++ = (uint8) lumaa1;

      /*--------------------------------------------------------------------
          Iteration for 6 pixels in one pass
      --------------------------------------------------------------------*/
      for ( col = col_max ; col; col-=2 )
      {


        /*----------------------------------------------------------------
          Next half pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Distance average for 0.75 sample space
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Store the half pixel in the next row
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr;

        /*------------------------------------------------------------------
            Convert and store the half pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));

        /*------------------------------------------------------------------
          closing pixel of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));

        /*------------------------------------------------------------------
              The distance 0.5
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );
        cb = cb + ( ( cb_dash - cb ) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );

        /*------------------------------------------------------------------
          save lumaa2 in the average
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
          Convert and store the pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Next pixel pair
        -------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          0.25 pixel distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( ( lumaa1 - lumaa_dash ) >> 2 );


        /*------------------------------------------------------------------
            Store the pixel pair for alter averaging
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa2;
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            Store the l1 for later use
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;

        /*------------------------------------------------------------------
                convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
                    next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));

        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset )));

        /*------------------------------------------------------------------
            The 0.75 distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3 >> 2 );

        /*------------------------------------------------------------------
          Second pixel pair
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          0.5 pixel distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );

        /*------------------------------------------------------------------
          Store for next row averge
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
            Convert and sotre the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            The cb for the nxt pixel pair
        ------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cb_ptr)));

        /*------------------------------------------------------------------
          Next pixel lumaa
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Cr from correspoding to this pixel
        ------------------------------------------------------------------*/
        cr = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));


        /*------------------------------------------------------------------
            0.25 pixel distance
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
          Save the pixel pair for late averaging
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa_dash;
        *outRawDump++ = (uint8) cr;

        /*------------------------------------------------------------------
          Convert and store the image pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
                                          r, out,  \
                                          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        /*------------------------------------------------------------------
            Just store the pixel for averaging.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            convert and store the image
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
                                        r, out,  \
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;

      } /* End of col loop */


      /*--------------------------------------------------------------------
        Closing pixels
      --------------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        We are assuming the 0.75 distance. This might not be true when the
        o/p is cropped more than 234 pixels. But this is a pretty good
        approximation
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);


      /*--------------------------------------------------------------------
        Store and average later with the next row
      --------------------------------------------------------------------*/
      *outRawDump++ = (uint8) cb;
      *outRawDump++ = (uint8) lumaa1;
      *outRawDump++ = (uint8) cr;

      /*--------------------------------------------------------------------
        Convert and store the pixel pairs
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                        rTable, gTable, bTable);

      *outputImgPtr++ = out;


      /*--------------------------------------------------------------------
          Next row is averaging row so we move the o/p by one row
          so that averaged row ins not overwritten
      --------------------------------------------------------------------*/
      outputImgPtr += outDx;


      /*--------------------------------------------------------------------
        Increment the row by one so will go to the average and store state
      --------------------------------------------------------------------*/
      row_cnt++;


    } /* end of row_cnt == 1 */

    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  );

    cb_ptr += rowInc >> 1;
    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

} /* end of function ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133 */

/*===========================================================================

FUNCTION IPL2_UPSIZEAVERAGEANDCROP_YCBCR420TORGBQVGA()

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133_PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register uint16 out, out2;
  register int32 cb,cr;
  register int32 lumaa1, lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */
  register uint8 *cb_ptr;
  uint32 rowInc = 0, destInc, cr_offset;
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register int32 cb_dash, cr_dash;
  register int32  lumaa_dash;
  int32 row, col_max, row_max;
  int32   row_cnt, col;
/*------------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133_PT\n");

  /*------------------------------------------------------------------------
      Determine the conversion table depending on the o/p color
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  } /* end of switch */

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cb_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
      Takes us to the offset for Cb Vals
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy  ) >> 2 /* /4 */;

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

    /*--------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
    ----------------------------------------------------------------------*/
    col_max =  ( ( i_param->crop.dx )  / 4 );


    /*----------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
    ----------------------------------------------------------------------*/
    if ( col_max > IPL2_QCIF_COL_CNTPT_BY_3 )
    {
      col_max = IPL2_QCIF_COL_CNTPT_BY_3;
    }
    else if ( col_max & 0x1 )
    {
      col_max--;
    }

    /*----------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
    ----------------------------------------------------------------------*/
    rowInc = ( IPL2_QCIF_HEIGHT
             - ( col_max * 3   )  ) ;



    /*------------------------------------------------------------------------
        Limit the maximun no of rows to the QCIF size
    ------------------------------------------------------------------------*/
    if ( row_max > IPL2_QCIF_WIDTH )
    {
      row_max = IPL2_QCIF_WIDTH;
    }

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 ) ) * 2;


  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;


  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  for ( row = 0; row < row_max; row++ )
  {


    /*----------------------------------------------------------------------
        For every odd row we need to move the cb/cr ptrs so that they repeate
        the same for the this row as well. In 420 encoding 2 pixels in the
        next row share the same cr/cb values
    ----------------------------------------------------------------------*/
    if ( row & 0x1 )
    {
      cb_ptr -= ( i_img_ptr->dx) >> 1;
    }

    /*----------------------------------------------------------------------
      The rows can be devided in three groups. One which does not
      participate in averaging one which does and one after the avraged
      row and participate in ageraging.

      ____________________________________________________
      |__________________________row0_____________________|
      |_______________________row1________________________|
      |_______________________avg_________________________|
      |________________________row2_______________________|
      |   ...........................................     |
      |___________________________________________________|


      2 rows are copied as is and then there is avg = (row1 + row2) /2

      where the addition is matrix addition.

      This cycle repeats. So the o/p is upsized by

      4/3. Because each 3 input rows create 4 o/p rows. Actually each row
      needs to be weighted aveaged. But we limit it to the row which
      falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/

    /*----------------------------------------------------------------------
        Deal with row0 in the comment
    ----------------------------------------------------------------------*/
    if ( row_cnt == 0 )
    {

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*)(cb_ptr)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));



      for ( col = col_max ; col; col-=2 )
      {


        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );



        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cb = ( cb +  cb_dash ) >> 1;
        cr = ( cr + cr_dash  ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = *((uint8*)(cb_ptr++ + cr_offset));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

      } /* End of col loop */


      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
          Average with previous row
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx );


      /*--------------------------------------------------------------------
      Read out the opening pixel
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*)(cb_ptr)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));

      /*--------------------------------------------------------------------
        iterate for one complete row .. 6 complete pixels
        on each iterations. This will give us a repetitive pattern for
        1.33 ( 4/3) upsizing)
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {


        /*----------------------------------------------------------------
      Get the L2 of the pixel pair from the previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          this is 0.75 pixels. Look at the position after a 6 pixel cycle
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2) ;

        /*----------------------------------------------------------------
            Average the values with previous row.
        ----------------------------------------------------------------*/
        *(outRawDump ) =    (uint8) ((*outRawDump  + cb ) >> 1);
        *(outRawDump + 1) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *(outRawDump + 2) = (uint8) ((*( outRawDump + 2) + cr ) >> 1);

        /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*----------------------------------------------------------------
            Calculate RGB for Lumaa4 1 - 2 combo
        ----------------------------------------------------------------*/

        /*----------------------------------------------------------------
          Deal with Distance 1.5 pitch 1 - combo
        ----------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));

        /*----------------------------------------------------------------
            Pixel 2 + n* 3
        ----------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));

        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1) ;

        cb = cb + ( ( cb_dash - cb ) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );

        /*------------------------------------------------------------------
          Do average with previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 3) = (uint8) ((*(outRawDump + 3) + lumaa2 ) >> 1);

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 2.25 pitch 2 - 3 combo no need for Cr Cb change
        ----------------------------------------------------------------*/
        /*----------------------------------------------------------------
            Pixel 3 + n * 3
        ----------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        lumaa_dash = lumaa1;

        *( outRawDump + 4 ) = (uint8) ((*( outRawDump + 4 ) + cb_dash) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*( outRawDump + 5 ) + lumaa2)  >> 1);
        *( outRawDump + 6 ) = (uint8) ((*( outRawDump + 6 ) + cr_dash) >> 1);
        *( outRawDump + 7 ) = (uint8) ((*( outRawDump + 7 ) + lumaa1)  >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);


        *outputImgPtr++ = out;

        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
          Read out the averaged pixels from previous row
        ------------------------------------------------------------------*/
        cb = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
            Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
                                    r, out, out2, \
                                    rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
          read out the second pixel pair from the prevous row
        ------------------------------------------------------------------*/
        cb = *(outRawDump );
        lumaa2 = *(outRawDump +1);
        cr = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          COnvert and store the pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
                                r, out, out2, \
                                rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


        /*------------------------------------------------------------------
            Next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));

        /*------------------------------------------------------------------
            0.75 pixel distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.5 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 ) ;

        /*------------------------------------------------------------------
            Average the values with previous row.
        ------------------------------------------------------------------*/
        *( outRawDump )     = (uint8) ((*( outRawDump) + cb_dash ) >> 1);
        *( outRawDump + 1 ) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *( outRawDump + 2 ) = (uint8) ((*( outRawDump + 2) + cr_dash ) >> 1);
        *( outRawDump + 3 ) = (uint8) ((*( outRawDump + 3) + lumaa2 ) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
          Open the next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));

        /*------------------------------------------------------------------
            distance average for 0.25 sampling distnace
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2) ;

        /*------------------------------------------------------------------
          Average this pixel with the preevious pixel
        ------------------------------------------------------------------*/
        *( outRawDump + 4 ) = (uint8) ((*( outRawDump + 4) + cb ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*( outRawDump + 5) + lumaa_dash ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*( outRawDump + 6) + cr ) >> 1);

        /*------------------------------------------------------------------
            convert and store the single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
                                          r, out,  \
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            averaging the pixel lumaa with the previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 7 ) = (uint8) ((*( outRawDump + 7) + lumaa1 ) >> 1);

        /*------------------------------------------------------------------
              Convert and store this single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
                                          r, out,  \
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Take out the averaged values
        ------------------------------------------------------------------*/
        cb_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
            convert and store the pixel pair
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            Take out the next pixel pair from the averaged row
        ------------------------------------------------------------------*/
        cb_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cr_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


      } /* End of col loop */


      row_cnt = 0;
    }
    else if ( row_cnt == 1 )
    {

      /*--------------------------------------------------------------------
          Next row should be the average of 2 rows
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx);


      /*--------------------------------------------------------------------
        Take the opening half pixel for this row
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*)(cb_ptr)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));


      /*--------------------------------------------------------------------
        To get the even aligned half words we encode the row in this
        way.

        __________
        |B|L1.....
        ----------


        Where B is a blank value with is padding. The L1 starts the
        opening pixel.

        The Cb and Cr are taken from next pixel. THis is a very good appro-
        ximation
      --------------------------------------------------------------------*/
      /*--------------------------------------------------------------------
          Iteration for 6 pixels in one pass
      --------------------------------------------------------------------*/
      for ( col = col_max ; col; col-=2 )
      {


        /*----------------------------------------------------------------
          Next half pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Distance average for 0.75 sample space
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Store the half pixel in the next row
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr;

        /*------------------------------------------------------------------
            Convert and store the half pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));

        /*------------------------------------------------------------------
          closing pixel of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));

        /*------------------------------------------------------------------
              The distance 0.5
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );
        cb = cb + ( ( cb_dash - cb ) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );

        /*------------------------------------------------------------------
          save lumaa2 in the average
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
          Convert and store the pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Next pixel pair
        -------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          0.25 pixel distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( ( lumaa1 - lumaa_dash ) >> 2 );


        /*------------------------------------------------------------------
            Store the pixel pair for alter averaging
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa2;
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            Store the l1 for later use
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;

        /*------------------------------------------------------------------
                convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
                    next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = (int32) (*((uint8*)(cb_ptr)));

        lumaa2 = *((uint8*)(inputImgPtr++));
        cr_dash = (int32) (*((uint8*)(cb_ptr++ + cr_offset )));

        /*------------------------------------------------------------------
            The 0.75 distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3 >> 2 );

        /*------------------------------------------------------------------
          Second pixel pair
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          0.5 pixel distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );

        /*------------------------------------------------------------------
          Store for next row averge
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
            Convert and sotre the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            The cb for the nxt pixel pair
        ------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cb_ptr)));

        /*------------------------------------------------------------------
          Next pixel lumaa
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Cr from correspoding to this pixel
        ------------------------------------------------------------------*/
        cr = (int32) (*((uint8*)(cb_ptr++ + cr_offset)));


        /*------------------------------------------------------------------
            0.25 pixel distance
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
          Save the pixel pair for late averaging
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa_dash;
        *outRawDump++ = (uint8) cr;

        /*------------------------------------------------------------------
          Convert and store the image pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
                                          r, out,  \
                                          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        /*------------------------------------------------------------------
            Just store the pixel for averaging.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            convert and store the image
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
                                        r, out,  \
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;

      } /* End of col loop */



      /*--------------------------------------------------------------------
          Next row is averaging row so we move the o/p by one row
          so that averaged row ins not overwritten
      --------------------------------------------------------------------*/
      outputImgPtr += outDx;


      /*--------------------------------------------------------------------
        Increment the row by one so will go to the average and store state
      --------------------------------------------------------------------*/
      row_cnt++;


    } /* end of row_cnt == 1 */

    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  - 1 );

    cb_ptr +=  (rowInc >> 1) - 1;
    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

} /* end of function ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133 */




/*===========================================================================

FUNCTION ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register uint16 out, out2;
  register int32 cb,cr;
  register int32 lumaa1, lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */
  register uint8 *cr_ptr;
  uint32 rowInc = 0, destInc;
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register int32 cb_dash, cr_dash;
  register int32  lumaa_dash;
  int32 row, col_max, row_max;
  int32   row_cnt, col;
/*------------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133\n");

  /*------------------------------------------------------------------------
      Determine the conversion table depending on the o/p color
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  } /* end of switch */

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cr_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y/2) * 2;

  /*------------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
  ------------------------------------------------------------------------*/
  col_max =  ( ( i_param->crop.dx - 2)  / 4 );

  /*------------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
  ------------------------------------------------------------------------*/
  if ( col_max > IPL2_QCIF_COL_CNTLS_BY_3 )
  {
    col_max = IPL2_QCIF_COL_CNTLS_BY_3;
  }
  else if ( col_max & 0x1 )
  {
    col_max--;
  }



  /*------------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
      For YCbCr420 this is eavh takes only 1 byte.
  ------------------------------------------------------------------------*/
  rowInc = ( IPL2_QCIF_WIDTH
             - ( col_max * 3   + 2)  ) ;

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 + 2 ) ) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

  /*------------------------------------------------------------------------
      Limit the maximun no of rows to the QCIF size
  ------------------------------------------------------------------------*/
  if ( row_max > IPL2_QCIF_HEIGHT )
  {
    row_max = IPL2_QCIF_HEIGHT;
  }

  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  for ( row = 0; row < row_max; row++ )
  {


    /*----------------------------------------------------------------------
        For every odd row we need to move the cb/cr ptrs so that they repeate
        the same for the this row as well. In 420 encoding 2 pixels in the
        next row share the same cr/cb values
    ----------------------------------------------------------------------*/
    if ( row & 0x1 )
    {
      //cb_ptr -= ( i_img_ptr->dx) >> 1;
      cr_ptr -= ( i_img_ptr->dx) >> 0;
    }

    /*----------------------------------------------------------------------
      The rows can be devided in three groups. One which does not
      participate in averaging one which does and one after the avraged
      row and participate in ageraging.

      ____________________________________________________
      |__________________________row0_____________________|
      |_______________________row1________________________|
      |_______________________avg_________________________|
      |________________________row2_______________________|
      |   ...........................................     |
      |___________________________________________________|


      2 rows are copied as is and then there is avg = (row1 + row2) /2

      where the addition is matrix addition.

      This cycle repeats. So the o/p is upsized by

      4/3. Because each 3 input rows create 4 o/p rows. Actually each row
      needs to be weighted aveaged. But we limit it to the row which
      falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/

    /*----------------------------------------------------------------------
        Deal with row0 in the comment
    ----------------------------------------------------------------------*/
    if ( row_cnt == 0 )
    {

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      cr = (int32) (*((uint8*)(cr_ptr++)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cb = (int32) (*((uint8*) (cr_ptr++)));

      /*--------------------------------------------------------------------
          Convert and render the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);
      *outputImgPtr++ = out;



      for ( col = col_max ; col; col-=2 )
      {


        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );



        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cr_dash = *((uint8*)(cr_ptr++));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cb_dash = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cr = ( cr + cr_dash  ) >> 1;
        cb = ( cb + cb_dash ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cr_dash = *((uint8*)(cr_ptr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cb_dash = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cr = *((uint8*)(cr_ptr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cb = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

      } /* End of col loop */


      /*----------------------------------------------------------------
      Read the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        Closing pixel is assumed to be in the 0.75 sampling distance. This
        may not be true if the o/p image is less thatn 234. But this is
        pretty colse assumption. So do it regardless of size
        to avoid complication
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


      /*--------------------------------------------------------------------
         color convert and save the pixel info
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;


      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
          Average with previous row
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx );


      /*--------------------------------------------------------------------
      Read out the opening pixel
      --------------------------------------------------------------------*/
      cr = (int32) (*((uint8*)(cr_ptr++)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cb = (int32) (*((uint8*) (cr_ptr++)));

      /*--------------------------------------------------------------------
      Color convert and store the opeing pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;

      /*--------------------------------------------------------------------
        Do the averaging from previous row and then sore it into the o/p
        buffer
      --------------------------------------------------------------------*/
      cr_dash  = ( *(outRawDump + 4) + cr ) >> 1;
      cb_dash  = ( *(outRawDump + 2) + cb ) >> 1;
      lumaa_dash   = ( *( outRawDump + 1) + lumaa1 ) >> 1;

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash, r,
        out, rTable, gTable, bTable);

      * ( (uint16 *) outRawDump ) = out;

      /*--------------------------------------------------------------------
        Advance the dumped row by one
      --------------------------------------------------------------------*/
      outRawDump += 2;

      /*--------------------------------------------------------------------
        iterate for one complete row .. 6 complete pixels
        on each iterations. This will give us a repetitive pattern for
        1.33 ( 4/3) upsizing)
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {


        /*----------------------------------------------------------------
      Get the L2 of the pixel pair from the previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          this is 0.75 pixels. Look at the position after a 6 pixel cycle
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2) ;

        /*----------------------------------------------------------------
            Average the values with previous row.
        ----------------------------------------------------------------*/
        *(outRawDump ) =    (uint8) ((*outRawDump  + cr ) >> 1);
        *(outRawDump + 1) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *(outRawDump + 2) = (uint8) ((*( outRawDump + 2) + cb ) >> 1);

        /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*----------------------------------------------------------------
            Calculate RGB for Lumaa4 1 - 2 combo
        ----------------------------------------------------------------*/

        /*----------------------------------------------------------------
          Deal with Distance 1.5 pitch 1 - combo
        ----------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*----------------------------------------------------------------
            Pixel 2 + n* 3
        ----------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1) ;

        cr = cr + ( ( cr_dash - cr ) >> 1 );
        cb = cb + ( ( cb_dash - cb ) >> 1 );

        /*------------------------------------------------------------------
          Do average with previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 3) = (uint8) ((*(outRawDump + 3) + lumaa2 ) >> 1);

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 2.25 pitch 2 - 3 combo no need for Cr Cb change
        ----------------------------------------------------------------*/
        /*----------------------------------------------------------------
            Pixel 3 + n * 3
        ----------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        lumaa_dash = lumaa1;

        *( outRawDump + 4 ) = (uint8) ((*( outRawDump + 4 ) + cr_dash ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*( outRawDump + 5 ) + lumaa2 ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*( outRawDump + 6 ) + cb_dash ) >> 1);
        *( outRawDump + 7 ) = (uint8) ((*( outRawDump + 7 ) + lumaa1 ) >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);


        *outputImgPtr++ = out;

        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
          Read out the averaged pixels from previous row
        ------------------------------------------------------------------*/
        cr = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cb = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
            Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
                                    r, out, out2, \
                                    rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
          read out the second pixel pair from the prevous row
        ------------------------------------------------------------------*/
        cr = *(outRawDump );
        lumaa2 = *(outRawDump +1);
        cb = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          COnvert and store the pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
                                r, out, out2, \
                                rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


        /*------------------------------------------------------------------
            Next pixel pair
       -------------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
            0.75 pixel distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.5 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 ) ;

        /*------------------------------------------------------------------
            Average the values with previous row.
        ------------------------------------------------------------------*/
        *( outRawDump )     = (uint8) ((*( outRawDump) + cr_dash ) >> 1);
        *( outRawDump + 1 ) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *( outRawDump + 2 ) = (uint8) ((*( outRawDump + 2) + cb_dash ) >> 1);
        *( outRawDump + 3 ) = (uint8) ((*( outRawDump + 3) + lumaa2 ) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
          Open the next pixel pair
        ------------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
            distance average for 0.25 sampling distnace
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2) ;

        /*------------------------------------------------------------------
          Average this pixel with the preevious pixel
        ------------------------------------------------------------------*/
        *( outRawDump + 4 ) = (uint8) ((*( outRawDump + 4) + cr ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*( outRawDump + 5) + lumaa_dash ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*( outRawDump + 6) + cb ) >> 1);

        /*------------------------------------------------------------------
            convert and store the single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
                                          r, out,  \
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            averaging the pixel lumaa with the previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 7 ) = (uint8) ((*( outRawDump + 7) + lumaa1 ) >> 1);

        /*------------------------------------------------------------------
              Convert and store this single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
                                          r, out,  \
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Take out the averaged values
        ------------------------------------------------------------------*/
        cr_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cb_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
            convert and store the pixel pair
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            Take out the next pixel pair from the averaged row
        ------------------------------------------------------------------*/
        cr_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cb_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


      } /* End of col loop */


      /*----------------------------------------------------------------
        Take out the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        The closing pixel is assumed in 0.75 sample space. This might not
        be very accurate when the i/p introduces additional cropping from
        234. But for now this is a very good approx
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2) ;


      /*--------------------------------------------------------------------
          Average the values with previous row.
      --------------------------------------------------------------------*/
      *(outRawDump ) = (uint8)    ((*outRawDump  + cr ) >> 1);
      *(outRawDump + 1) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
      *(outRawDump + 2) = (uint8) ((*( outRawDump + 2) + cb ) >> 1);

      /*--------------------------------------------------------------------
        Convert and store the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                      rTable, gTable, bTable);

      *outputImgPtr++ = out;

      /*--------------------------------------------------------------------
          Take the pixel pair from the averaged row and render it back
      --------------------------------------------------------------------*/
      cr = *(outRawDump );
      lumaa2 = *(outRawDump + 1);
      cb = *(outRawDump + 2);
      lumaa_dash = *(outRawDump + 3);

      /*--------------------------------------------------------------------
          Convert and store the pixel pair in the
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
        r, out, out2, \
        rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

      row_cnt = 0;
    }
    else if ( row_cnt == 1 )
    {

      /*--------------------------------------------------------------------
          Next row should be the average of 2 rows
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx);


      /*--------------------------------------------------------------------
        Take the opening half pixel for this row
      --------------------------------------------------------------------*/
      cr = (int32) (*((uint8*)(cr_ptr++)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cb = (int32) (*((uint8*) (cr_ptr++)));

      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                        rTable, gTable, bTable);

      *outputImgPtr++ = out;

      /*--------------------------------------------------------------------
        To get the even aligned half words we encode the row in this
        way.

        __________
        |B|L1.....
        ----------


        Where B is a blank value with is padding. The L1 starts the
        opening pixel.

        The Cb and Cr are taken from next pixel. THis is a very good appro-
        ximation
      --------------------------------------------------------------------*/
      outRawDump++;
      *outRawDump++ = (uint8) lumaa1;

      /*--------------------------------------------------------------------
          Iteration for 6 pixels in one pass
      --------------------------------------------------------------------*/
      for ( col = col_max ; col; col-=2 )
      {


        /*----------------------------------------------------------------
          Next half pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Distance average for 0.75 sample space
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Store the half pixel in the next row
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cr;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cb;

        /*------------------------------------------------------------------
            Convert and store the half pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          Next pixel pair
        ------------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
          closing pixel of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
              The distance 0.5
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );
        cb = cb + ( ( cb_dash - cb ) >> 1 );

        /*------------------------------------------------------------------
          save lumaa2 in the average
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
          Convert and store the pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Next pixel pair
        -------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          0.25 pixel distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( ( lumaa1 - lumaa_dash ) >> 2 );


        /*------------------------------------------------------------------
            Store the pixel pair for alter averaging
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa2;
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            Store the l1 for later use
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;

        /*------------------------------------------------------------------
                convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
                    next pixel pair
       -------------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
            The 0.75 distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3 >> 2 );

        /*------------------------------------------------------------------
          Second pixel pair
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          0.5 pixel distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );

        /*------------------------------------------------------------------
          Store for next row averge
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
            Convert and sotre the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            The cb for the nxt pixel pair
        ------------------------------------------------------------------*/
        cr = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
          Next pixel lumaa
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Cr from correspoding to this pixel
        ------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cr_ptr++)));


        /*------------------------------------------------------------------
            0.25 pixel distance
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
          Save the pixel pair for late averaging
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cr;
        *outRawDump++ = (uint8) lumaa_dash;
        *outRawDump++ = (uint8) cb;

        /*------------------------------------------------------------------
          Convert and store the image pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
                                          r, out,  \
                                          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        /*------------------------------------------------------------------
            Just store the pixel for averaging.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            convert and store the image
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
                                        r, out,  \
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;

      } /* End of col loop */


      /*--------------------------------------------------------------------
        Closing pixels
      --------------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        We are assuming the 0.75 distance. This might not be true when the
        o/p is cropped more than 234 pixels. But this is a pretty good
        approximation
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);


      /*--------------------------------------------------------------------
        Store and average later with the next row
      --------------------------------------------------------------------*/
      *outRawDump++ = (uint8) cr;
      *outRawDump++ = (uint8) lumaa1;
      *outRawDump++ = (uint8) cb;

      /*--------------------------------------------------------------------
        Convert and store the pixel pairs
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                        rTable, gTable, bTable);

      *outputImgPtr++ = out;


      /*--------------------------------------------------------------------
          Next row is averaging row so we move the o/p by one row
          so that averaged row ins not overwritten
      --------------------------------------------------------------------*/
      outputImgPtr += outDx;


      /*--------------------------------------------------------------------
        Increment the row by one so will go to the average and store state
      --------------------------------------------------------------------*/
      row_cnt++;


    } /* end of row_cnt == 1 */

    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  );

    cr_ptr += rowInc >> 0;
    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

} /* end of function ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIF_High_133 */



#if 0

/*===========================================================================

FUNCTION ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_Med_133_rot180

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

  It will rotate the image 180 degress. 

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
static ipl_status_type ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_Med_133_rot180
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register uint16 out, out2;
  register int32 cb,cr;
  register int32 lumaa1, lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register uint8 *cr_ptr;
  uint8 *startCrPtr;
  //uint32 rowInc = 0, destInc;
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  unsigned char* startInputImgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16* outputImgPtrAvg;
  register int32 cb_dash, cr_dash;
  register int32  lumaa_dash;
  int32 row, col_max, row_max;
  int32   row_cnt, col;
  int32 cropOffset;
  uint32 off;

/*------------------------------------------------------------------------*/
  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133_rot180\n");

  /*------------------------------------------------------------------------
      Determine the conversion table depending on the o/p color
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  } /* end of switch */

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cr_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  cropOffset = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  /*------------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
  ------------------------------------------------------------------------*/
  col_max =  ( ( i_param->crop.dx - 2)  / 4 );

  /*------------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
  ------------------------------------------------------------------------*/
  if ( col_max > IPL2_QCIF_COL_CNTLS_BY_3 )
  {
    col_max = IPL2_QCIF_COL_CNTLS_BY_3;
  }
  else if ( col_max & 0x1 )
  {
    col_max--;
  }



  /*------------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
      For YCbCr420 this is eavh takes only 1 byte.
  ------------------------------------------------------------------------*/
  //rowInc = ( IPL2_QCIF_WIDTH - ( col_max * 3   + 2)  ) ;

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  //destInc = ( o_img_ptr->dx  - ( col_max * 4 + 2 ) ) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

  /*------------------------------------------------------------------------
      Limit the maximun no of rows to the QCIF size
  ------------------------------------------------------------------------*/
  if ( row_max > IPL2_QCIF_HEIGHT )
  {
    row_max = IPL2_QCIF_HEIGHT;
  }

  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/

  startInputImgPtr = i_img_ptr->imgPtr + (i_img_ptr->dx * i_img_ptr->dy)-1;
  startCrPtr = i_img_ptr->clrPtr + i_img_ptr->dx * i_img_ptr->dy/2 - 1;

  off = 0;
  for ( row = 0; row < row_max; row++ )
  {
    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    //inputImgPtr = (uint8*)((uint32)inputImgPtr + rowInc  );
    inputImgPtr = (uint8*)((uint32)startInputImgPtr - row*i_img_ptr->dx);

    //cr_ptr += rowInc >> 0;
    cr_ptr = startCrPtr - ((row/2) * i_img_ptr->dx);
    
    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    //outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;
    outputImgPtr = (uint16*) o_img_ptr->imgPtr + cropOffset + off*o_img_ptr->dx;


    /*----------------------------------------------------------------------
      The rows can be devided in three groups. One which does not
      participate in averaging one which does and one after the avraged
      row and participate in ageraging.

      ____________________________________________________
      |__________________________row0_____________________|
      |_______________________row1________________________|
      |_______________________avg_________________________|
      |________________________row2_______________________|
      |   ...........................................     |
      |___________________________________________________|


      2 rows are copied as is and then there is avg = (row1 + row2) /2

      where the addition is matrix addition.

      This cycle repeats. So the o/p is upsized by

      4/3. Because each 3 input rows create 4 o/p rows. Actually each row
      needs to be weighted aveaged. But we limit it to the row which
      falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/

    /*----------------------------------------------------------------------
        Deal with row0 in the comment
    ----------------------------------------------------------------------*/
    if ( row_cnt == 0 || row_cnt == 1)
    {

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      // since we are going backwards, grab Cb first!
      cb = (int32) (*((uint8*) (cr_ptr--)));
      lumaa1 = *((uint8*)(inputImgPtr--));
      cr = (int32) (*((uint8*)(cr_ptr--)));

      /*--------------------------------------------------------------------
          Convert and render the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;

      for ( col = col_max ; col; col-=2 )
      {
        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr--));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );



        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cr_ptr--));
        lumaa_dash = *((uint8*)(inputImgPtr--));
        cr_dash = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cr = ( cr + cr_dash  ) >> 1;
        cb = ( cb + cb_dash ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr--));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cr_ptr--));
        lumaa2 = *((uint8*)(inputImgPtr--));
        cr_dash = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr--));
        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb = *((uint8*)(cr_ptr--));
        lumaa1 = *((uint8*)(inputImgPtr--));
        cr = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

      } /* End of col loop */


      /*----------------------------------------------------------------
      Read the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr--));

      /*--------------------------------------------------------------------
        Closing pixel is assumed to be in the 0.75 sampling distance. This
        may not be true if the o/p image is less thatn 234. But this is
        pretty colse assumption. So do it regardless of size
        to avoid complication
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


      /*--------------------------------------------------------------------
         color convert and save the pixel info
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;


      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt++;
      off += 1;
    }
    else if ( row_cnt == 2 )
    {
      outputImgPtr += o_img_ptr->dx;
      outputImgPtrAvg = outputImgPtr - o_img_ptr->dx;

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*) (cr_ptr--)));
      lumaa1 = *((uint8*)(inputImgPtr--));
      cr = (int32) (*((uint8*)(cr_ptr--)));

      /*--------------------------------------------------------------------
          Convert and render the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;
      *outputImgPtrAvg++ = out;

      for ( col = col_max ; col; col-=2 )
      {
        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr--));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );



        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;
        *outputImgPtrAvg++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cr_ptr--));
        lumaa_dash = *((uint8*)(inputImgPtr--));
        cr_dash = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cr = ( cr + cr_dash  ) >> 1;
        cb = ( cb + cb_dash ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtrAvg++ = out;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr--));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;
        *outputImgPtrAvg++ = out;
        *outputImgPtrAvg++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cr_ptr--));
        lumaa2 = *((uint8*)(inputImgPtr--));
        cr_dash = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr--));
        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;
        *outputImgPtrAvg++ = out;
        *outputImgPtrAvg++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb = *((uint8*)(cr_ptr--));
        lumaa1 = *((uint8*)(inputImgPtr--));
        cr = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtrAvg++ = out;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtrAvg++ = out;

      } /* End of col loop */


      /*----------------------------------------------------------------
      Read the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr--));

      /*--------------------------------------------------------------------
        Closing pixel is assumed to be in the 0.75 sampling distance. This
        may not be true if the o/p image is less thatn 234. But this is
        pretty colse assumption. So do it regardless of size
        to avoid complication
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


      /*--------------------------------------------------------------------
         color convert and save the pixel info
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;
      *outputImgPtrAvg++ = out;

      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt = 0;
      off += 2;
    }
  } /* End of row loop */

  return(IPL_SUCCESS);
} 




/*===========================================================================

FUNCTION ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_Med_133_rot90

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

  This function will also rotate the image 90 degress.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
static ipl_status_type ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_Med_133_rot90
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register uint16 out, out2;
  register int32 cb,cr;
  register int32 lumaa1, lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register uint8 *cr_ptr;
  uint8 *startCrPtr;
  //uint32 rowInc = 0, destInc;
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  unsigned char* startInputImgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16* outputImgPtrAvg;
  register int32 cb_dash, cr_dash;
  register int32  lumaa_dash;
  int32 row, col_max, row_max;
  int32 row_cnt, col;
  int32 cropOffset;

  uint32 off;
  uint32 destColInc;

/*------------------------------------------------------------------------*/
  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133_rot90\n");

  /*------------------------------------------------------------------------
      Determine the conversion table depending on the o/p color
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  } /* end of switch */

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cr_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  cropOffset = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  /*------------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
  ------------------------------------------------------------------------*/
  col_max =  ( ( i_param->crop.dx - 2)  / 4 );

  /*------------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
  ------------------------------------------------------------------------*/
  if ( col_max > IPL2_QCIF_COL_CNTLS_BY_3 )
  {
    col_max = IPL2_QCIF_COL_CNTLS_BY_3;
  }
  else if ( col_max & 0x1 )
  {
    col_max--;
  }



  /*------------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
      For YCbCr420 this is eavh takes only 1 byte.
  ------------------------------------------------------------------------*/
  //rowInc = ( IPL2_QCIF_WIDTH - ( col_max * 3   + 2)  ) ;

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  //destInc = ( o_img_ptr->dx  - ( col_max * 4 + 2 ) ) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

  /*------------------------------------------------------------------------
      Limit the maximun no of rows to the QCIF size
  ------------------------------------------------------------------------*/
  if ( row_max > IPL2_QCIF_HEIGHT )
  {
    row_max = IPL2_QCIF_HEIGHT;
  }


  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  destColInc = o_img_ptr->dx; 
  startInputImgPtr = i_img_ptr->imgPtr + (i_img_ptr->dx * (i_img_ptr->dy-1));
  startCrPtr = i_img_ptr->clrPtr + (i_img_ptr->dx * ((i_img_ptr->dy/2)-1));
  off = 0;
  for ( row = 0; row < row_max; row++ )
  {
    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)startInputImgPtr - row*i_img_ptr->dx);
    cr_ptr = startCrPtr - ((row/2) * i_img_ptr->dx);
    
    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    //outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;
    outputImgPtr = (uint16*) o_img_ptr->imgPtr + cropOffset + off;


    /*----------------------------------------------------------------------
      The rows can be devided in three groups. One which does not
      participate in averaging one which does and one after the avraged
      row and participate in ageraging.

      ____________________________________________________
      |__________________________row0_____________________|
      |_______________________row1________________________|
      |_______________________avg_________________________|
      |________________________row2_______________________|
      |   ...........................................     |
      |___________________________________________________|


      2 rows are copied as is and then there is avg = (row1 + row2) /2

      where the addition is matrix addition.

      This cycle repeats. So the o/p is upsized by

      4/3. Because each 3 input rows create 4 o/p rows. Actually each row
      needs to be weighted aveaged. But we limit it to the row which
      falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/

    /*----------------------------------------------------------------------
        Deal with row0 in the comment
    ----------------------------------------------------------------------*/
    if ( row_cnt == 0 || row_cnt == 1)
    {

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      cr = (int32) (*((uint8*)(cr_ptr++)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cb = (int32) (*((uint8*) (cr_ptr++)));

      /*--------------------------------------------------------------------
          Convert and render the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr = out;
      outputImgPtr += destColInc;

      for ( col = col_max ; col; col-=2 )
      {
        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );



        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr = out;
        outputImgPtr += destColInc;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cr_dash = *((uint8*)(cr_ptr++));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cb_dash = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cr = ( cr + cr_dash  ) >> 1;
        cb = ( cb + cb_dash ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr = out;
        outputImgPtr += destColInc;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;
        *outputImgPtr = out2;
        outputImgPtr += destColInc;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cr_dash = *((uint8*)(cr_ptr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cb_dash = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;
        *outputImgPtr = out2;
        outputImgPtr += destColInc;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cr = *((uint8*)(cr_ptr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cb = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;

      } /* End of col loop */


      /*----------------------------------------------------------------
      Read the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        Closing pixel is assumed to be in the 0.75 sampling distance. This
        may not be true if the o/p image is less thatn 234. But this is
        pretty colse assumption. So do it regardless of size
        to avoid complication
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


      /*--------------------------------------------------------------------
         color convert and save the pixel info
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr = out;
      outputImgPtr += destColInc;


      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt++;
      off += 1;
    }
    else if ( row_cnt == 2 )
    {
      outputImgPtr++;
      outputImgPtrAvg = outputImgPtr - 1;

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      cr = (int32) (*((uint8*)(cr_ptr++)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cb = (int32) (*((uint8*) (cr_ptr++)));

      /*--------------------------------------------------------------------
          Convert and render the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr = out;
      outputImgPtr += destColInc;

      *outputImgPtrAvg = out;
      outputImgPtrAvg += destColInc;



      for ( col = col_max ; col; col-=2 )
      {
        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );



        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr = out;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cr_dash = *((uint8*)(cr_ptr++));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cb_dash = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cr = ( cr + cr_dash  ) >> 1;
        cb = ( cb + cb_dash ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr = out;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;
        *outputImgPtr = out2;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;
        *outputImgPtrAvg = out2;
        outputImgPtrAvg += destColInc;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cr_dash = *((uint8*)(cr_ptr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cb_dash = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;
        *outputImgPtr = out2;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;
        *outputImgPtrAvg = out2;
        outputImgPtrAvg += destColInc;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cr = *((uint8*)(cr_ptr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cb = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;

      } /* End of col loop */


      /*----------------------------------------------------------------
      Read the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        Closing pixel is assumed to be in the 0.75 sampling distance. This
        may not be true if the o/p image is less thatn 234. But this is
        pretty colse assumption. So do it regardless of size
        to avoid complication
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


      /*--------------------------------------------------------------------
         color convert and save the pixel info
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr = out;
      outputImgPtr += destColInc;

      *outputImgPtrAvg = out;
      outputImgPtrAvg += destColInc;

      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt = 0;
      off += 2;
    }
  } /* End of row loop */

  return(IPL_SUCCESS);
} 



/*===========================================================================

FUNCTION ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_Med_133_rot270

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

  This function will also rotate the image 270 degress.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
static ipl_status_type ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_Med_133_rot270
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register uint16 out, out2;
  register int32 cb,cr;
  register int32 lumaa1, lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register uint8 *cr_ptr;
  uint8 *startCrPtr;
  //uint32 rowInc = 0, destInc;
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  unsigned char* startInputImgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16* outputImgPtrAvg;
  register int32 cb_dash, cr_dash;
  register int32  lumaa_dash;
  int32 row, col_max, row_max;
  int32   row_cnt, col;
  int32 cropOffset;

  uint32 off;
  uint32 destColInc;

/*------------------------------------------------------------------------*/
  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133_rot270\n");

  /*------------------------------------------------------------------------
      Determine the conversion table depending on the o/p color
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  } /* end of switch */

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

  ------------------------------------------------------------------------*/


  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  cropOffset = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  /*------------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
  ------------------------------------------------------------------------*/
  col_max =  ( ( i_param->crop.dx - 2)  / 4 );

  /*------------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
  ------------------------------------------------------------------------*/
  if ( col_max > IPL2_QCIF_COL_CNTLS_BY_3 )
  {
    col_max = IPL2_QCIF_COL_CNTLS_BY_3;
  }
  else if ( col_max & 0x1 )
  {
    col_max--;
  }


  /*------------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
      For YCbCr420 this is eavh takes only 1 byte.
  ------------------------------------------------------------------------*/
  //rowInc = ( IPL2_QCIF_WIDTH - ( col_max * 3   + 2)  ) ;

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  //destInc = ( o_img_ptr->dx  - ( col_max * 4 + 2 ) ) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

  /*------------------------------------------------------------------------
      Limit the maximun no of rows to the QCIF size
  ------------------------------------------------------------------------*/
  if ( row_max > IPL2_QCIF_HEIGHT )
  {
    row_max = IPL2_QCIF_HEIGHT;
  }

  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  destColInc = o_img_ptr->dx; 

  startInputImgPtr = i_img_ptr->imgPtr + i_img_ptr->dx - 1;
  startCrPtr = i_img_ptr->clrPtr + i_img_ptr->dx - 1;

  off = 0;
  for ( row = 0; row < row_max; row++ )
  {
    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)startInputImgPtr + 
                           row*i_img_ptr->dx);
    cr_ptr = startCrPtr + ((row/2) * i_img_ptr->dx);
    
    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16*) o_img_ptr->imgPtr + cropOffset + off;


    /*----------------------------------------------------------------------
      The rows can be devided in three groups. One which does not
      participate in averaging one which does and one after the avraged
      row and participate in ageraging.

      ____________________________________________________
      |__________________________row0_____________________|
      |_______________________row1________________________|
      |_______________________avg_________________________|
      |________________________row2_______________________|
      |   ...........................................     |
      |___________________________________________________|


      2 rows are copied as is and then there is avg = (row1 + row2) /2

      where the addition is matrix addition.

      This cycle repeats. So the o/p is upsized by

      4/3. Because each 3 input rows create 4 o/p rows. Actually each row
      needs to be weighted aveaged. But we limit it to the row which
      falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/

    /*----------------------------------------------------------------------
        Deal with row0 in the comment
    ----------------------------------------------------------------------*/
    if ( row_cnt == 0 || row_cnt == 1)
    {

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*) (cr_ptr--)));
      lumaa1 = *((uint8*)(inputImgPtr--));
      cr = (int32) (*((uint8*)(cr_ptr--)));

      /*--------------------------------------------------------------------
          Convert and render the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr = out;
      outputImgPtr += destColInc;

      for ( col = col_max ; col; col-=2 )
      {
        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr--));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );



        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr = out;
        outputImgPtr += destColInc;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cr_ptr--));
        lumaa_dash = *((uint8*)(inputImgPtr--));
        cr_dash = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cr = ( cr + cr_dash  ) >> 1;
        cb = ( cb + cb_dash ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr = out;
        outputImgPtr += destColInc;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr--));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;
        *outputImgPtr = out2;
        outputImgPtr += destColInc;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cr_ptr--));
        lumaa2 = *((uint8*)(inputImgPtr--));
        cr_dash = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr--));
        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;
        *outputImgPtr = out2;
        outputImgPtr += destColInc;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb = *((uint8*)(cr_ptr--));
        lumaa1 = *((uint8*)(inputImgPtr--));
        cr = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;

      } /* End of col loop */


      /*----------------------------------------------------------------
      Read the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr--));

      /*--------------------------------------------------------------------
        Closing pixel is assumed to be in the 0.75 sampling distance. This
        may not be true if the o/p image is less thatn 234. But this is
        pretty colse assumption. So do it regardless of size
        to avoid complication
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


      /*--------------------------------------------------------------------
         color convert and save the pixel info
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr = out;
      outputImgPtr += destColInc;


      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt++;
      off += 1;
    }
    else if ( row_cnt == 2 )
    {
      outputImgPtr++;
      outputImgPtrAvg = outputImgPtr - 1;

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      cb = (int32) (*((uint8*) (cr_ptr--)));
      lumaa1 = *((uint8*)(inputImgPtr--));
      cr = (int32) (*((uint8*)(cr_ptr--)));

      /*--------------------------------------------------------------------
          Convert and render the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr = out;
      outputImgPtr += destColInc;

      *outputImgPtrAvg = out;
      outputImgPtrAvg += destColInc;



      for ( col = col_max ; col; col-=2 )
      {
        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr--));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );



        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr = out;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cr_ptr--));
        lumaa_dash = *((uint8*)(inputImgPtr--));
        cr_dash = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cr = ( cr + cr_dash  ) >> 1;
        cb = ( cb + cb_dash ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr = out;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr--));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;
        *outputImgPtr = out2;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;
        *outputImgPtrAvg = out2;
        outputImgPtrAvg += destColInc;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cr_ptr--));
        lumaa2 = *((uint8*)(inputImgPtr--));
        cr_dash = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr--));

        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;
        *outputImgPtr = out2;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;
        *outputImgPtrAvg = out2;
        outputImgPtrAvg += destColInc;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cb = *((uint8*)(cr_ptr--));
        lumaa1 = *((uint8*)(inputImgPtr--));
        cr = *((uint8*)(cr_ptr--));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr = out;
        outputImgPtr += destColInc;

        *outputImgPtrAvg = out;
        outputImgPtrAvg += destColInc;

      } /* End of col loop */


      /*----------------------------------------------------------------
      Read the closing pixel
      ----------------------------------------------------------------*/
      lumaa2 = *((uint8*)(inputImgPtr--));

      /*--------------------------------------------------------------------
        Closing pixel is assumed to be in the 0.75 sampling distance. This
        may not be true if the o/p image is less thatn 234. But this is
        pretty colse assumption. So do it regardless of size
        to avoid complication
      --------------------------------------------------------------------*/
      lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


      /*--------------------------------------------------------------------
         color convert and save the pixel info
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
        rTable, gTable, bTable);

      *outputImgPtr = out;
      outputImgPtr += destColInc;

      *outputImgPtrAvg = out;
      outputImgPtrAvg += destColInc;

      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt = 0;
      off += 2;
    }
  } /* End of row loop */

  return(IPL_SUCCESS);
} 

#endif


/*===========================================================================

FUNCTION ipl2_upsize_rot_crop_133

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. (This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

  This function will also rotate the image.

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
)
{
#if 1 
  // dont call this fucntion anymore, call the new IPL faster fucntion,
  // ipl_crop_resize_rot
  
  ipl_rect_type crop;
  MSG_LOW("inside ipl2_upsize_rot_crop_133 which defaults to ipl_crr\n");


  if (i_param == NULL)
  {
    IPL2_MSG_FATAL( " ipl2_upsize_rot_crop_133 Bad i_param");
    return( IPL_FAILURE );
  }

  crop.x = i_param->crop.x;
  crop.y = i_param->crop.y;
  crop.dx = i_param->crop.dx;
  crop.dy = i_param->crop.dy;
  
  return (ipl_crop_resize_rot(i_img_ptr, o_img_ptr, NULL, &crop, rot, i_param->qual)); 
#else

  MSG_LOW("inside ipl2_upsize_rot_crop_133\n");


  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_upsize_rot_crop_133 \
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
           NULL check the bare minimum parameters required
   -----------------------------------------------------------------------*/
  if ( !i_param || !i_img_ptr || !o_img_ptr )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_133 failed i_param = %x, /\
                   i_img_ptr = %x, o_img_ptr = %x", (uint32) i_param,
                   (uint32) i_img_ptr, (uint32) o_img_ptr);
    return( IPL_FAILURE );
  }
  else
  {
    /*----------------------------------------------------------------------
        See if images have actual buffers
     ---------------------------------------------------------------------*/

    if ( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
    {
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_133 failed i_img_ptr->imgPtr=%x, /\
                   o_img_ptr->imgPtr = %x", (uint32) i_img_ptr->imgPtr,
                   (uint32) o_img_ptr->imgPtr, 0);

      return( IPL_FAILURE );
    }
  }


  /*------------------------------------------------------------------------
      For now we support only RGB o/p
  ------------------------------------------------------------------------*/
  if( !( ( o_img_ptr->cFormat == IPL_RGB565 ) ||
        ( o_img_ptr->cFormat == IPL_RGB444 ) ||
        ( o_img_ptr->cFormat == IPL_RGB666 )  ) )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_133 failed  : Unsupported o/p color /\
                   format  ", 0, 0, 0 );
    return( IPL_FAILURE );

  }


  /*------------------------------------------------------------------------
      Switch on the input image type
  ------------------------------------------------------------------------*/
  switch( i_img_ptr->cFormat)
  {
    case IPL_YCbCr:
      if (rot == IPL_NOROT)
        return( ipl2_upsizeQCIF_YCbCr_133( i_img_ptr, o_img_ptr, i_param) );
      else
      {
        IPL2_MSG_FATAL("ipl2_upsize_rot_crop_133 failed: rotation not \
                      supported");
        return( IPL_FAILURE);
      }
      //break;

    case IPL_YCbCr420_FRAME_PK :
      if (rot == IPL_NOROT)
        return( ipl2_upsizeQCIF_YCbCr420_133( i_img_ptr, o_img_ptr, i_param ) );
      else
      {
        IPL2_MSG_FATAL("ipl2_upsize_rot_crop_133 failed: rotation not \
                      supported");
        return( IPL_FAILURE);
      }
      //break;

    case IPL_YCrCb420_LINE_PK :
      if (rot == IPL_NOROT)
      {
        return(ipl2_upsizeQCIF_YCrCb420lp_133(i_img_ptr,o_img_ptr,i_param));
      }
      else if (rot == IPL_ROT90)
      {
        return(ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_Med_133_rot90(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT180)
      {
        return(ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_Med_133_rot180(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT270)
      {
        return(ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_Med_133_rot270(i_img_ptr, o_img_ptr, i_param));
      }
      else
        return( IPL_FAILURE );
      //break;

    default:
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_133 failed:Unsupported i/p color /\
               format  ", 0, 0, 0 );
      return( IPL_FAILURE );
      //break;
  }
  //return( IPL_SUCCESS );
#endif

} 


/*===========================================================================

FUNCTION ipl2_upsize_rot_crop_150x

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. (This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

  This function will also rotate the image.

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
)
{
  MSG_LOW("inside ipl2_upsize_rot_crop_150x\n");





  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_upsize_rot_crop_150x \
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
           NULL check the bare minimum parameters required
   -----------------------------------------------------------------------*/
  if ( !i_param || !i_img_ptr || !o_img_ptr )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_150x failed i_param = %lu, /\
                   i_img_ptr = %lu, o_img_ptr = %lu", (uint32) i_param,
                   (uint32) i_img_ptr, (uint32) o_img_ptr);
    return( IPL_FAILURE );
  }
  else
  {
    /*----------------------------------------------------------------------
        See if images have actual buffers
     ---------------------------------------------------------------------*/

    if ( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
    {
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_150x failed i_img_ptr->imgPtr=%lu, /\
                   o_img_ptr->imgPtr = %lu", (uint32) i_img_ptr->imgPtr,
                   (uint32) o_img_ptr->imgPtr);

      return( IPL_FAILURE );
    }
  }


  /*------------------------------------------------------------------------
      For now we support only RGB o/p
  ------------------------------------------------------------------------*/
  if( !( ( o_img_ptr->cFormat == IPL_RGB565 ) ||
        ( o_img_ptr->cFormat == IPL_RGB444 ) ||
        ( o_img_ptr->cFormat == IPL_RGB666 )  ) )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_150x failed  : Unsupported o/p color /\
                   format  " );
    return( IPL_FAILURE );

  }


  /*------------------------------------------------------------------------
      Switch on the input image type
  ------------------------------------------------------------------------*/
  switch( i_img_ptr->cFormat)
  {

    case IPL_YCbCr420_FRAME_PK :
      if (rot == IPL_NOROT)
      {
        return(ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT90)
      {
        return(ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot90(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT180)
      {
        return(ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot180(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT270)
      {
        return(ipl2_upSize_150x_RowAvgAndCrop_YCbCr420ToRGB_rot270(i_img_ptr, o_img_ptr, i_param));
      }
      else 
        return( IPL_FAILURE );
      //break;

    default:
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_150x failed:Unsupported i/p color /\
               format  " );
      return( IPL_FAILURE );
      //break;
  }
  //return( IPL_SUCCESS );

} 



/*===========================================================================

FUNCTION ipl2_upsize_rot_crop_2x

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. (This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

  This function will also rotate the image.

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
)
{
  MSG_LOW("inside ipl2_upsize_rot_crop_2x\n");





  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_upsize_rot_crop_2x \
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
           NULL check the bare minimum parameters required
   -----------------------------------------------------------------------*/
  if ( !i_param || !i_img_ptr || !o_img_ptr )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_2x failed i_param = %lu, /\
                   i_img_ptr = %lu, o_img_ptr = %lu", (uint32) i_param,
                   (uint32) i_img_ptr, (uint32) o_img_ptr);
    return( IPL_FAILURE );
  }
  else
  {
    /*----------------------------------------------------------------------
        See if images have actual buffers
     ---------------------------------------------------------------------*/

    if ( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
    {
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_2x failed i_img_ptr->imgPtr=%lu, /\
                   o_img_ptr->imgPtr = %lu", (uint32) i_img_ptr->imgPtr,
                   (uint32) o_img_ptr->imgPtr);

      return( IPL_FAILURE );
    }
  }


  /*------------------------------------------------------------------------
      For now we support only RGB o/p
  ------------------------------------------------------------------------*/
  if( !( ( o_img_ptr->cFormat == IPL_RGB565 ) ||
        ( o_img_ptr->cFormat == IPL_RGB444 ) ||
        ( o_img_ptr->cFormat == IPL_RGB666 )  ) )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_2x failed  : Unsupported o/p color /\
                   format  " );
    return( IPL_FAILURE );

  }


  /*------------------------------------------------------------------------
      Switch on the input image type
  ------------------------------------------------------------------------*/
  switch( i_img_ptr->cFormat)
  {
    case IPL_YCbCr:
      if (rot == IPL_NOROT)
        return(ipl2_upsize_YCbCr442( i_img_ptr, o_img_ptr, i_param ) );
      else
      {
        IPL2_MSG_FATAL("ipl2_upsize_rot_crop_2x failed: rotation not \
                      supported");
        return(IPL_FAILURE);
        }
      //break;

    case IPL_YCrCb420_LINE_PK :
      if (rot == IPL_NOROT)
        return( ipl2_upsize_YCrCb420lp( i_img_ptr, o_img_ptr, i_param ) );
      else
      {
        IPL2_MSG_FATAL("ipl2_upsize_rot_crop_2x failed: rotation not \
                      supported");
        return(IPL_FAILURE);
      }
      //break;

    case IPL_YCbCr420_FRAME_PK :
      if (rot == IPL_NOROT)
      {
        return(ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT90)
      {
        return(ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB_Med_rot90(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT180)
      {
        return(ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB_Med_rot180(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT270)
      {
        return(ipl2_upSizeRowAvgAndCrop_YCbCr420ToRGB_Med_rot270(i_img_ptr, o_img_ptr, i_param));
      }
      else 
        return( IPL_FAILURE );
      //break;

    default:
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_2x failed:Unsupported i/p color /\
               format  " );
      return( IPL_FAILURE );
      //break;
  }
  //return( IPL_SUCCESS );

} 



/*===========================================================================

FUNCTION ipl2_upsize_rot_crop_250x

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. (This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor.

  This function will also rotate the image.

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
)
{
  MSG_LOW("inside ipl2_upsize_rot_crop_250x\n");





  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_upsize_rot_crop_250x \
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
           NULL check the bare minimum parameters required
   -----------------------------------------------------------------------*/
  if ( !i_param || !i_img_ptr || !o_img_ptr )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_250x failed i_param = %lu, /\
                   i_img_ptr = %lu, o_img_ptr = %lu", (uint32) i_param,
                   (uint32) i_img_ptr, (uint32) o_img_ptr);
    return( IPL_FAILURE );
  }
  else
  {
    /*----------------------------------------------------------------------
        See if images have actual buffers
     ---------------------------------------------------------------------*/

    if ( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
    {
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_250x failed i_img_ptr->imgPtr=%lu, /\
                   o_img_ptr->imgPtr = %lu", (uint32) i_img_ptr->imgPtr,
                   (uint32) o_img_ptr->imgPtr);

      return( IPL_FAILURE );
    }
  }


  /*------------------------------------------------------------------------
      For now we support only RGB o/p
  ------------------------------------------------------------------------*/
  if( !( ( o_img_ptr->cFormat == IPL_RGB565 ) ||
        ( o_img_ptr->cFormat == IPL_RGB444 ) ||
        ( o_img_ptr->cFormat == IPL_RGB666 )  ) )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_250x failed  : Unsupported o/p color /\
                   format  " );
    return( IPL_FAILURE );

  }


  /*------------------------------------------------------------------------
      Switch on the input image type
  ------------------------------------------------------------------------*/
  switch( i_img_ptr->cFormat)
  {

    case IPL_YCbCr420_FRAME_PK :
      if (rot == IPL_NOROT)
      {
        return(ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT90)
      {
        return(ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot90(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT180)
      {
        return(ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot180(i_img_ptr, o_img_ptr, i_param));
      }
      else if (rot == IPL_ROT270)
      {
        return(ipl2_upSize_250x_RowAvgAndCrop_YCbCr420ToRGB_rot270(i_img_ptr, o_img_ptr, i_param));
      }
      else 
        return( IPL_FAILURE );
      //break;

    default:
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_250x failed:Unsupported i/p color /\
               format  " );
      return( IPL_FAILURE );
      //break;
  }
  //return( IPL_SUCCESS );

} 


#if 0

/*===========================================================================


FUNCTION ipl_upSize_qcif2qvga_YCrCb420lpToRGB_high

DESCRIPTION
  This function upsize from qcif to qvga while color converting from
  ycrcb420 line pack to rgb565

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
extern ipl_status_type ipl_upSize_qcif2qvga_YCrCb420lpToRGB_high
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  unsigned char* inputImgPtr  = i_img_ptr->imgPtr;
  unsigned char* inputImgPtr2 = i_img_ptr->imgPtr + i_img_ptr->dx;
  unsigned char* cr_ptr = i_img_ptr->clrPtr;

  uint16* outputImgPtr  =(uint16 *) o_img_ptr->imgPtr;
  uint16* outputImgPtrb =(uint16 *) o_img_ptr->imgPtr + o_img_ptr->dx;
  uint16* outputImgPtr2 =(uint16 *) o_img_ptr->imgPtr + o_img_ptr->dx*2;

  uint16 out;
  int32 cb = 0, cr = 0;
  int32 lumaa1, lumaa2, lumaa3;
  int32 lumaa12, lumaa22, lumaa32;

  int32 r;
  const uint16 *rTable;
  const uint16 *gTable;
  const uint16 *bTable;

  int outDx = o_img_ptr->dx;
  uint32 rowInc, destInc;
  uint32 row,col;
  int i;
  int inputRow;

  MSG_LOW("inside ipl2_upSize_QCIF2QVGA_YCbCr420lpToRGB \n");

  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
    return( IPL_FAILURE );

  /*------------------------------------------------------------------------
           NULL check the bare minimum parameters required
   -----------------------------------------------------------------------*/
  if ( !i_param || !i_img_ptr || !o_img_ptr )
  {
    return( IPL_FAILURE );
  }
  else
  {
    if ( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
    {
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_180x failed i_img_ptr->imgPtr=%x, /\
                   o_img_ptr->imgPtr = %x", (uint32) i_img_ptr->imgPtr,
                   (uint32) o_img_ptr->imgPtr, 0);

      return( IPL_FAILURE );
    }
  }


  /*------------------------------------------------------------------------
      For now we support only RGB o/p
  ------------------------------------------------------------------------*/
  if(!((o_img_ptr->cFormat == IPL_RGB565 ) ||
        (o_img_ptr->cFormat == IPL_RGB444 ) ||
        (o_img_ptr->cFormat == IPL_RGB666 )))
    return( IPL_FAILURE );



  switch (o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  outputImgPtrb += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);
  outputImgPtr2 += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  // For every 9 output pixels, we use 5 input pixels. We can only
  // output multiple of 9. Because of this, figure out how
  // many pixel on the input we need to skip per row
  rowInc = (i_img_ptr->dx  - (5*(i_param->crop.dx/9)));

  // see how many pixel on the output we need to skip per row
  destInc = (o_img_ptr->dx  - 9*(i_param->crop.dx/9));

  // we do 5 output lines at a time
  inputRow = 0;
  for (row = 0; row < i_param->crop.dy/5; row++)
  {
    // do 3 rows
    for (i = 0; i < 3; i++)
    {
      // setup our Cr ptr
      cr_ptr = i_img_ptr->clrPtr + (inputRow/2)*i_img_ptr->dx;

      for (col = 0; col < i_param->crop.dx/9; col++)
      {
        if (!(col%2))
        {

          // mapping of Cr and Cb to luma
          //
          //  0 0   0 0   0     // original pixels
          //   X  X  X  X       // new pixels
          //  c1    c2    c3    // macthing chroma
          
          //
          // First 3 output pixels, where two input rows also makes three rows
          //
          lumaa1 = *inputImgPtr++;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     

          // the next three all share the same chroma  
          cr = *cr_ptr++;
          cb = *cr_ptr++;

         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // now do the second line
          lumaa12 = *inputImgPtr2++;
          lumaa32 = *inputImgPtr2++;
          lumaa22 = (lumaa12 + lumaa32)>>1;     

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa12,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr2++ = out;

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa22,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr2++ = out;

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa32,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr2++ = out;

          // now fill in the line between the to lines with the avg.
          lumaa1 = (lumaa1 + lumaa12) >> 1;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtrb++ = out;

          lumaa2 = (lumaa2 + lumaa22) >> 1;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtrb++ = out;

          lumaa3 = (lumaa3 + lumaa32) >> 1;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtrb++ = out;










          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     

          // next guy is avg of cr cb at border
          cr = (cr + *(cr_ptr))>>1;
          cb = (cb + *(cr_ptr+1))>>1;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is cr2 cb2
          cr = *cr_ptr++;
          cb = *cr_ptr++;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     
          // next guy is cr2 cb2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          // next guy is cr2 cb2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is avg of cr cb at border
          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     
          cr = (cr + *(cr_ptr))   >>1;
          cb = (cb + *(cr_ptr+1)) >>1;

         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is cr3 cb3
          cr = *cr_ptr++;
          cb = *cr_ptr++;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;
        }
        else
        {
          //
          //   0   0 0   0 0
          //     X  X  X  X 
          //  c3    c1    c2
          //  
          
          lumaa1 = *inputImgPtr++;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     

          // first pixel uses c3 from last guy
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          // next guy is avg of c3 and c1
          cr = (cr + (*cr_ptr++))>>1;
          cb = (cb + (*cr_ptr++))>>1;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          // next guy is c1 
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is c1 
          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is c1
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     
          // next guy is avg of c1 and c2
          cr = (cr + (*cr_ptr++))>>1;
          cb = (cb + (*cr_ptr++))>>1;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          // next guy is c2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     

          // next guy is c2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is c2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;
        }
      }

      outputImgPtr += destInc;
      inputImgPtr += rowInc;


      // do the next row as we did above, but skip a line since we are later
      // going to comeback and fill it in. Only skip two output rows for every 
      // 3 input rows
      if (i < 2)
        outputImgPtr += outDx;

      inputRow++;
    }  

  }

  MSG_LOW("done ipl2_upSize_QCIF2QVGA_YCbCr420lpToRGB_high \n");

  return(IPL_SUCCESS);
}

#endif



/*===========================================================================


FUNCTION ipl_upSize_qcif2qvga_YCrCb420lpToRGB

DESCRIPTION
  This function upsize from qcif to qvga while color converting from
  ycrcb420 line pack to rgb565

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
extern ipl_status_type ipl_upSize_qcif2qvga_YCrCb420lpToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  unsigned char* inputImgPtr;
  unsigned char* cr_ptr;
  uint16* outputImgPtr;

  uint16 out;
  int32 cb = 0, cr = 0;
  int32 lumaa1, lumaa2, lumaa3;

  int32 r;
  const uint16 *rTable;
  const uint16 *gTable;
  const uint16 *bTable;

  int outDx = o_img_ptr->dx;
  uint32 rowInc, destInc;
  uint32 row,col;
  int i;
  int firstPart, secondPart;
  int inputRow;

  MSG_LOW("inside ipl2_upSize_QCIF2QVGA_YCbCr420lpToRGB \n");


  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
    return( IPL_FAILURE );

  /*------------------------------------------------------------------------
           NULL check the bare minimum parameters required
   -----------------------------------------------------------------------*/
  if ( !i_param || !i_img_ptr || !o_img_ptr )
  {
    return( IPL_FAILURE );
  }
  else
  {
    if ( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
    {
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_180x failed i_img_ptr->imgPtr=%lu, /\
                   o_img_ptr->imgPtr = %lu", (uint32) i_img_ptr->imgPtr,
                   (uint32) o_img_ptr->imgPtr);

      return( IPL_FAILURE );
    }
  }

  inputImgPtr = i_img_ptr->imgPtr;
  cr_ptr = i_img_ptr->clrPtr;
  outputImgPtr =(uint16 *) o_img_ptr->imgPtr;

  /*------------------------------------------------------------------------
      For now we support only RGB o/p
  ------------------------------------------------------------------------*/
  if(!((o_img_ptr->cFormat == IPL_RGB565 ) ||
        (o_img_ptr->cFormat == IPL_RGB444 ) ||
        (o_img_ptr->cFormat == IPL_RGB666 )))
    return( IPL_FAILURE );



  switch (o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  // For every 9 output pixels, we use 5 input pixels. We can only
  // output multiple of 9. Because of this, figure out how
  // many pixel on the input we need to skip per row
  rowInc = (i_img_ptr->dx  - (5*(i_param->crop.dx/9)));

  // see how many pixel on the output we need to skip per row
  destInc = (o_img_ptr->dx  - 9*(i_param->crop.dx/9));

  firstPart = destInc/2;
  secondPart = (destInc % 2)? firstPart+1 : firstPart;

  // we do 5 output lines at a time
  inputRow = 0;
  for (row = 0; row < i_param->crop.dy/5; row++)
  {
    // do 3 rows
    for (i = 0; i < 3; i++)
    {
      // setup our Cr ptr
      cr_ptr = i_img_ptr->clrPtr + (inputRow/2)*i_img_ptr->dx;

      // now, we are short desInc pixels in the output, so for now,
      // just repeat the last pixel
      lumaa1 = *inputImgPtr;
      cr = *(cr_ptr);
      cb = *(cr_ptr+1);
      IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,cr,cb,r,out,rTable,gTable,bTable);

      for (col = firstPart; col; col-- )
        *outputImgPtr++ = out;

      for (col = 0; col < i_param->crop.dx/9; col++)
      {
        if (!(col%2))
        {

          // mapping of Cr and Cb to luma
          //
          //  0 0   0 0   0     // original pixels
          //   X  X  X  X       // new pixels
          //  c1    c2    c3    // macthing chroma
          
          lumaa1 = *inputImgPtr++;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     

          // the next three all share the same chroma  
          cr = *cr_ptr++;
          cb = *cr_ptr++;

         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     

          // next guy is avg of cr cb at border
          cr = (cr + *(cr_ptr))>>1;
          cb = (cb + *(cr_ptr+1))>>1;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is cr2 cb2
          cr = *cr_ptr++;
          cb = *cr_ptr++;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     
          // next guy is cr2 cb2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          // next guy is cr2 cb2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is avg of cr cb at border
          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     
          cr = (cr + *(cr_ptr))   >>1;
          cb = (cb + *(cr_ptr+1)) >>1;

         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is cr3 cb3
          cr = *cr_ptr++;
          cb = *cr_ptr++;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;
        }
        else
        {
          //
          //   0   0 0   0 0
          //     X  X  X  X 
          //  c3    c1    c2
          //  
          
          lumaa1 = *inputImgPtr++;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     

          // first pixel uses c3 from last guy
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          // next guy is avg of c3 and c1
          cr = (cr + (*cr_ptr++))>>1;
          cb = (cb + (*cr_ptr++))>>1;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          // next guy is c1 
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is c1 
          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is c1
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     
          // next guy is avg of c1 and c2
          cr = (cr + (*cr_ptr++))>>1;
          cb = (cb + (*cr_ptr++))>>1;
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;


          // next guy is c2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          lumaa1 = lumaa3;
          lumaa3 = *inputImgPtr++;
          lumaa2 = (lumaa1 + lumaa3)>>1;     

          // next guy is c2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;

          // next guy is c2
         IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa3,cr,cb,r,out,rTable,gTable,bTable);
          *outputImgPtr++ = out;
        }
      }

      // now, we are short desInc pixels in the output, so for now,
      // just repeat the last pixel
      for (col = secondPart; col; col-- )
        *outputImgPtr++ = out;

      outputImgPtr += (o_img_ptr->dx - i_param->crop.dx);
      inputImgPtr += rowInc;

      // do the next row as we did above, but skip a line since we are later
      // going to comeback and fill it in. Only skip two output rows for every 
      // 3 input rows
      if (i < 2)
        outputImgPtr += outDx;

      inputRow++;
    }  

  }

  // now go and fill in the rows we skipped 
  outputImgPtr = (uint16 *) o_img_ptr->imgPtr;
  for (row = 0; row < i_param->crop.dy/5; row++)
  {
    memcpy(outputImgPtr + outDx, outputImgPtr, outDx*2);
    outputImgPtr += (outDx*2);

    memcpy(outputImgPtr + outDx, outputImgPtr, outDx*2);
    outputImgPtr += (outDx*3);
  }

  MSG_LOW("done ipl2_upSize_QCIF2QVGA_YCbCr420lpToRGB \n");

  return(IPL_SUCCESS);
}




#if 0

/*===========================================================================

FUNCTION ipl2_upsize_rot_crop_180x

DESCRIPTION
  This function uses an optimized avreaging scheme to upsize a QCIF
  sized (176 * 144) image to 1.8 bigger.
  
  This function will also rotate the image.

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
API_EXTERN ipl_status_type ipl_upSize_qcif2qvga_rot_crop
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param,    /* processing param */
  ipl_rotate90_type rot
)
{
  MSG_LOW("inside ipl2_upsize_rot_crop_180x\n");


  /*------------------------------------------------------------------------
          Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( " ipl2_upsize_rot_crop_180x \
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
           NULL check the bare minimum parameters required
   -----------------------------------------------------------------------*/
  if ( !i_param || !i_img_ptr || !o_img_ptr )
  {
    IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_180x failed i_param = %x, /\
                   i_img_ptr = %x, o_img_ptr = %x", (uint32) i_param,
                   (uint32) i_img_ptr, (uint32) o_img_ptr);
    return( IPL_FAILURE );
  }
  else
  {
    /*----------------------------------------------------------------------
        See if images have actual buffers
     ---------------------------------------------------------------------*/

    if ( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
    {
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_180x failed i_img_ptr->imgPtr=%x, /\
                   o_img_ptr->imgPtr = %x", (uint32) i_img_ptr->imgPtr,
                   (uint32) o_img_ptr->imgPtr, 0);

      return( IPL_FAILURE );
    }
  }


  /*------------------------------------------------------------------------
      For now we support only RGB o/p
  ------------------------------------------------------------------------*/
  if( !(( o_img_ptr->cFormat == IPL_RGB565 ) ||
        ( o_img_ptr->cFormat == IPL_RGB444 ) ||
        ( o_img_ptr->cFormat == IPL_RGB666 )  ) )
  {
    IPL2_MSG_FATAL("ipl2_upsize_rot_crop_180x failed  : Unsupported o/p color /\
                   format  ", 0, 0, 0 );
    return( IPL_FAILURE );

  }


  /*------------------------------------------------------------------------
      Switch on the input image type
  ------------------------------------------------------------------------*/
  switch( i_img_ptr->cFormat)
  {

    case IPL_YCrCb420_LINE_PK :
    case IPL_YCbCr420_LINE_PK :
      if (rot == IPL_NOROT)
      {
        return(ipl2_upSize_QCIF2QVGA_YCbCr420lpToRGB(i_img_ptr, o_img_ptr, i_param));
      }
      else 
        return( IPL_FAILURE );
      //break;

    default:
      IPL2_MSG_FATAL( "ipl2_upsize_rot_crop_180x failed:Unsupported i/p color /\
               format  ", 0, 0, 0 );
      return( IPL_FAILURE );
      //break;
  }
  //return( IPL_SUCCESS );
} 


#endif



/*===========================================================================

FUNCTION ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133_PT

DESCRIPTION

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133_PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register uint16 out, out2;
  register int32 cb,cr;
  register int32 lumaa1, lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */
  register uint8 *cr_ptr;
  uint32 rowInc = 0, destInc;
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register int32 cb_dash, cr_dash;
  register int32  lumaa_dash;
  int32 row, col_max, row_max;
  int32   row_cnt, col;
/*------------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCrCb420lpToRGBQCIF_High_133_PT\n");

  /*------------------------------------------------------------------------
      Determine the conversion table depending on the o/p color
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*-----------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  } /* end of switch */

  /*------------------------------------------------------------------------
          The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
  cr_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  outputImgPtr += (i_param->crop.x/1 + o_img_ptr->dx * i_param->crop.y/1) * 2;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

    /*--------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
    ----------------------------------------------------------------------*/
    col_max =  ( ( i_param->crop.dx )  / 4 );


    /*----------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
    ----------------------------------------------------------------------*/
    if ( col_max > IPL2_QCIF_COL_CNTPT_BY_3 )
    {
      col_max = IPL2_QCIF_COL_CNTPT_BY_3;
    }
    else if ( col_max & 0x1 )
    {
      col_max--;
    }

    /*----------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
    ----------------------------------------------------------------------*/
    rowInc = ( IPL2_QCIF_HEIGHT
             - ( col_max * 3   )  ) ;



    /*------------------------------------------------------------------------
        Limit the maximun no of rows to the QCIF size
    ------------------------------------------------------------------------*/
    if ( row_max > IPL2_QCIF_WIDTH )
    {
      row_max = IPL2_QCIF_WIDTH;
    }

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 ) ) * 2;


  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;


  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  for ( row = 0; row < row_max; row++ )
  {


    /*----------------------------------------------------------------------
        For every odd row we need to move the cb/cr ptrs so that they repeate
        the same for the this row as well. In 420 encoding 2 pixels in the
        next row share the same cr/cb values
    ----------------------------------------------------------------------*/
    if ( row & 0x1 )
    {
      cr_ptr -= ( i_img_ptr->dx) >> 0;
    }

    /*----------------------------------------------------------------------
      The rows can be devided in three groups. One which does not
      participate in averaging one which does and one after the avraged
      row and participate in ageraging.

      ____________________________________________________
      |__________________________row0_____________________|
      |_______________________row1________________________|
      |_______________________avg_________________________|
      |________________________row2_______________________|
      |   ...........................................     |
      |___________________________________________________|


      2 rows are copied as is and then there is avg = (row1 + row2) /2

      where the addition is matrix addition.

      This cycle repeats. So the o/p is upsized by

      4/3. Because each 3 input rows create 4 o/p rows. Actually each row
      needs to be weighted aveaged. But we limit it to the row which
      falls in the 0.5 sampled interval
    ----------------------------------------------------------------------*/

    /*----------------------------------------------------------------------
        Deal with row0 in the comment
    ----------------------------------------------------------------------*/
    if ( row_cnt == 0 )
    {

      /*--------------------------------------------------------------------
             Read the opeing pixel and render separately. This is is
             to simplify the loop becasue it expcets alway l2 of a pixel in
             the beginnig
      --------------------------------------------------------------------*/
      cr = (int32) (*((uint8*) (cr_ptr++)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cb = (int32) (*((uint8*)(cr_ptr++)));

      for ( col = col_max ; col; col-=2 )
      {
        /*----------------------------------------------------------------
          closing pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            We need to find the 0.75 sample distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2 );


        /*----------------------------------------------------------------
            Convert and store the 0.75 distance picture.
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);
        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 1.5 sampling distance
        ----------------------------------------------------------------*/
        cr_dash = *((uint8*)(cr_ptr++));
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cb_dash = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            Average this pixel pair L1 to previous pixel pair L2
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa2 + lumaa_dash)  >> 1 ;

        /*------------------------------------------------------------------
            Average cb/cr pair
        ------------------------------------------------------------------*/
        cr = ( cr + cr_dash  ) >> 1;
        cb = ( cb + cb_dash ) >> 1;

        /*------------------------------------------------------------------
          convert and store the solo pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*------------------------------------------------------------------
            0.25 sampling distance
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average for 0.25 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
            Save for next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;


        /*------------------------------------------------------------------
              Convert and render the current pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cr_dash = *((uint8*)(cr_ptr++));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cb_dash = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            second 0.75 distance average
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Next lumaa
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        /*------------------------------------------------------------------
            0.4 distance average
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 +  ( (lumaa_dash - lumaa2) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixal pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);

        /*------------------------------------------------------------------
            Store the pixel pair
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            Next pixel pair
        ------------------------------------------------------------------*/
        cr = *((uint8*)(cr_ptr++));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cb = *((uint8*)(cr_ptr++));

        /*------------------------------------------------------------------
            0.25 average
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        /*------------------------------------------------------------------
            Color convert the current pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
          r, out,  \
          rTable, gTable, bTable);
        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          This single pixel needs a different cr and cb thats why its
          calculated single
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
          r, out,  \
          rTable, gTable, bTable);


        /*------------------------------------------------------------------
          Store the single pixel
        ------------------------------------------------------------------*/
        *outputImgPtr++ = out;

      } /* End of col loop */


      /*--------------------------------------------------------------------
        Advance the row to start averaging
      --------------------------------------------------------------------*/
      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
          Average with previous row
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx );


      /*--------------------------------------------------------------------
      Read out the opening pixel
      --------------------------------------------------------------------*/
      cr = (int32) (*((uint8*) (cr_ptr++)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cb = (int32) (*((uint8*)(cr_ptr++)));

      /*--------------------------------------------------------------------
        iterate for one complete row .. 6 complete pixels
        on each iterations. This will give us a repetitive pattern for
        1.33 ( 4/3) upsizing)
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {


        /*----------------------------------------------------------------
      Get the L2 of the pixel pair from the previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          this is 0.75 pixels. Look at the position after a 6 pixel cycle
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2) ;

        /*----------------------------------------------------------------
            Average the values with previous row.
        ----------------------------------------------------------------*/
        *(outRawDump ) =    (uint8) ((*outRawDump  + cr ) >> 1);
        *(outRawDump + 1) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *(outRawDump + 2) = (uint8) ((*( outRawDump + 2) + cb ) >> 1);

        /*----------------------------------------------------------------
        Following piece of code is a template for the YCrCB to RGB
        conversion when you find some bug on this make sure you fix all the
        templates used in this file
        ----------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*----------------------------------------------------------------
            Calculate RGB for Lumaa4 1 - 2 combo
        ----------------------------------------------------------------*/

        /*----------------------------------------------------------------
          Deal with Distance 1.5 pitch 1 - combo
        ----------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*----------------------------------------------------------------
            Pixel 2 + n* 3
        ----------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));
        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1) ;

        cr = cr + ( ( cr_dash - cr ) >> 1 );
        cb = cb + ( ( cb_dash - cb ) >> 1 );

        /*------------------------------------------------------------------
          Do average with previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 3) = (uint8) ((*(outRawDump + 3) + lumaa2 ) >> 1);

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);

        *outputImgPtr++ = out;


        /*----------------------------------------------------------------
          Deal with Distance 2.25 pitch 2 - 3 combo no need for Cr Cb change
        ----------------------------------------------------------------*/
        /*----------------------------------------------------------------
            Pixel 3 + n * 3
        ----------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        lumaa2 = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 ) ;

        lumaa_dash = lumaa1;

        *( outRawDump + 4 ) = (uint8) ((*( outRawDump + 4 ) + cr_dash ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*( outRawDump + 5 ) + lumaa2 ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*( outRawDump + 6 ) + cb_dash ) >> 1);
        *( outRawDump + 7 ) = (uint8) ((*( outRawDump + 7 ) + lumaa1 ) >> 1);

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);


        *outputImgPtr++ = out;

        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
          Read out the averaged pixels from previous row
        ------------------------------------------------------------------*/
        cr = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cb = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
            Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
                                    r, out, out2, \
                                    rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
          read out the second pixel pair from the prevous row
        ------------------------------------------------------------------*/
        cr = *(outRawDump );
        lumaa2 = *(outRawDump +1);
        cb = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          COnvert and store the pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr, cb,
                                r, out, out2, \
                                rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


        /*------------------------------------------------------------------
            Next pixel pair
       -------------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
            0.75 pixel distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            0.5 distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 ) ;

        /*------------------------------------------------------------------
            Average the values with previous row.
        ------------------------------------------------------------------*/
        *( outRawDump )     = (uint8) ((*( outRawDump) + cr_dash ) >> 1);
        *( outRawDump + 1 ) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
        *( outRawDump + 2 ) = (uint8) ((*( outRawDump + 2) + cb_dash ) >> 1);
        *( outRawDump + 3 ) = (uint8) ((*( outRawDump + 3) + lumaa2 ) >> 1);

        /*------------------------------------------------------------------
          convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
          Open the next pixel pair
        ------------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
            distance average for 0.25 sampling distnace
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2) ;

        /*------------------------------------------------------------------
          Average this pixel with the preevious pixel
        ------------------------------------------------------------------*/
        *( outRawDump + 4 ) = (uint8) ((*( outRawDump + 4) + cr ) >> 1);
        *( outRawDump + 5 ) = (uint8) ((*( outRawDump + 5) + lumaa_dash ) >> 1);
        *( outRawDump + 6 ) = (uint8) ((*( outRawDump + 6) + cb ) >> 1);

        /*------------------------------------------------------------------
            convert and store the single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
                                          r, out,  \
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            averaging the pixel lumaa with the previous row
        ------------------------------------------------------------------*/
        *( outRawDump + 7 ) = (uint8) ((*( outRawDump + 7) + lumaa1 ) >> 1);

        /*------------------------------------------------------------------
              Convert and store this single pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
                                          r, out,  \
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Take out the averaged values
        ------------------------------------------------------------------*/
        cr_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cb_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*----------------------------------------------------------------
            convert and store the pixel pair
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            Take out the next pixel pair from the averaged row
        ------------------------------------------------------------------*/
        cr_dash = *(outRawDump );
        lumaa2 = *(outRawDump + 1);
        cb_dash = *(outRawDump + 2);
        lumaa_dash = *(outRawDump + 3);

        /*------------------------------------------------------------------
          Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


      } /* End of col loop */


      row_cnt = 0;
    }
    else if ( row_cnt == 1 )
    {

      /*--------------------------------------------------------------------
          Next row should be the average of 2 rows
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx);


      /*--------------------------------------------------------------------
        Take the opening half pixel for this row
      --------------------------------------------------------------------*/
      cr = (int32) (*((uint8*) (cr_ptr++)));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cb = (int32) (*((uint8*)(cr_ptr++)));


      /*--------------------------------------------------------------------
        To get the even aligned half words we encode the row in this
        way.

        __________
        |B|L1.....
        ----------


        Where B is a blank value with is padding. The L1 starts the
        opening pixel.

        The Cb and Cr are taken from next pixel. THis is a very good appro-
        ximation
      --------------------------------------------------------------------*/
      /*--------------------------------------------------------------------
          Iteration for 6 pixels in one pass
      --------------------------------------------------------------------*/
      for ( col = col_max ; col; col-=2 )
      {


        /*----------------------------------------------------------------
          Next half pixel from previous iteration
        ----------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Distance average for 0.75 sample space
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3  >> 2);

        /*------------------------------------------------------------------
          Store the half pixel in the next row
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cr;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cb;

        /*------------------------------------------------------------------
            Convert and store the half pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb, r, out,  \
                                          rTable, gTable, bTable);

        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          Next pixel pair
        ------------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
          closing pixel of the current pixel pair
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
              The distance 0.5
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );
        cr = cr + ( ( cr_dash - cr ) >> 1 );
        cb = cb + ( ( cb_dash - cb ) >> 1 );

        /*------------------------------------------------------------------
          save lumaa2 in the average
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
          Convert and store the pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
            Next pixel pair
        -------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          0.25 pixel distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( ( lumaa1 - lumaa_dash ) >> 2 );


        /*------------------------------------------------------------------
            Store the pixel pair for alter averaging
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa2;
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            Store the l1 for later use
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa1;

        /*------------------------------------------------------------------
                convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa2, lumaa_dash, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
                    next pixel pair
       -------------------------------------------------------------------*/
        cr_dash = (int32) (*((uint8*)(cr_ptr++)));
        lumaa2 = *((uint8*)(inputImgPtr++));
        cb_dash = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
            The 0.75 distance
        ------------------------------------------------------------------*/
        lumaa1 = lumaa1 + ( ( lumaa2 - lumaa1 ) * 3 >> 2 );

        /*------------------------------------------------------------------
          Second pixel pair
        ------------------------------------------------------------------*/
        lumaa_dash = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          0.5 pixel distance
        ------------------------------------------------------------------*/
        lumaa2 = lumaa2 + ( (lumaa_dash - lumaa2) >> 1 );

        /*------------------------------------------------------------------
          Store for next row averge
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cr_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa2;

        /*------------------------------------------------------------------
            Convert and sotre the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,  cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*------------------------------------------------------------------
            The cb for the nxt pixel pair
        ------------------------------------------------------------------*/
        cr = (int32) (*((uint8*)(cr_ptr++)));

        /*------------------------------------------------------------------
          Next pixel lumaa
        ------------------------------------------------------------------*/
        lumaa1 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Cr from correspoding to this pixel
        ------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cr_ptr++)));


        /*------------------------------------------------------------------
            0.25 pixel distance
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa_dash + ( ( lumaa1 - lumaa_dash ) >> 2 );

        /*------------------------------------------------------------------
          Save the pixel pair for late averaging
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cr;
        *outRawDump++ = (uint8) lumaa_dash;
        *outRawDump++ = (uint8) cb;

        /*------------------------------------------------------------------
          Convert and store the image pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash,  cr_dash, cb_dash,
                                          r, out,  \
                                          rTable, gTable, bTable);

        *outputImgPtr++ = out;
        /*------------------------------------------------------------------
            Just store the pixel for averaging.
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) lumaa1;

        /*------------------------------------------------------------------
            convert and store the image
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa1,  cr, cb,
                                        r, out,  \
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;

      } /* End of col loop */



      /*--------------------------------------------------------------------
          Next row is averaging row so we move the o/p by one row
          so that averaged row ins not overwritten
      --------------------------------------------------------------------*/
      outputImgPtr += outDx;


      /*--------------------------------------------------------------------
        Increment the row by one so will go to the average and store state
      --------------------------------------------------------------------*/
      row_cnt++;


    } /* end of row_cnt == 1 */

    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
      --------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  - 1 );

    //cb_ptr +=  (rowInc >> 1) - 1;
    cr_ptr +=  (rowInc >> 0) - 2;
    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

} 



/*===========================================================================

FUNCTION IPL2_UPSIZEAVERAGEANDCROP_YCBCR420TORGBQVGAMED()

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor. Input color scheme is
  YCBCr 4:2:0

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3;
  register int32 cr, cb, cb_dash, cr_dash;
  register int32 lumaa1, lumaa_dash;
  register int32 lumaa2, lumaa2_dash;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */
  uint32 row,col_max, row_max;
  uint32 rowInc=0, destInc, cr_offset;
  uint32   row_cnt, col;
  register uint8 *cb_ptr;
/*------------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133\n");

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
        The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
      cb_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
      Takes us to the offset for Cb Vals
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy  ) >> 2 /* /4 */;


  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
  ------------------------------------------------------------------------*/
  col_max =  ( ( i_param->crop.dx - 2)  / 4 );

  /*------------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
  ------------------------------------------------------------------------*/
  if ( col_max > IPL2_QCIF_COL_CNTLS_BY_3 )
  {
    col_max = IPL2_QCIF_COL_CNTLS_BY_3;
  }
  else if ( col_max & 0x1 )
  {
    col_max--;
  }



  /*------------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
  ------------------------------------------------------------------------*/
  rowInc = ( IPL2_QCIF_WIDTH
             - ( col_max * 3   + 2)  );

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 + 2 ) ) * 2;

  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;

  /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

  /*------------------------------------------------------------------------
      Limit the maximun no of rows to the QCIF size
  ------------------------------------------------------------------------*/
  if ( row_max > IPL2_QCIF_HEIGHT )
  {
    row_max = IPL2_QCIF_HEIGHT;
  }

  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  for ( row = 0; row < row_max; row++ )
  {

    /*----------------------------------------------------------------------
    For every odd row we need to move the cb/cr ptrs so that they repeate
    the same for the this row as well. In 420 encoding 2 pixels in the
    next row share the same cr/cb values
    ----------------------------------------------------------------------*/
    if ( row & 0x1 )
    {
      cb_ptr -= ( i_img_ptr->dx) >> 1;
    }

    if ( row_cnt == 0 )
    {

      for ( col = col_max; col; col-=2 )
      {


        /*------------------------------------------------------------------
          Opening pixel pair
        ------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cb_ptr)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Save the lumaa for future calculations
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
            convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
                                  cr, cb, r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
            next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset ));

        /*------------------------------------------------------------------
          average values with the pnextsous pair
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;


        /*------------------------------------------------------------------
          Convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;




        /*------------------------------------------------------------------
          Closing pixel from the current pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));


        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);


        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;




        /*----------------------------------------------------------------
            Next pixel pair
       ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Average falls in the middle of the pixel pair we dont need to
          average the Cb/Cr pair
        ------------------------------------------------------------------*/
        lumaa_dash = (lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
          Convert and store these 3 pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
                                            cr_dash, cb_dash, \
                                            r, out, out2, out3, \
                                            rTable, gTable, bTable);

        *( outputImgPtr++)= out;
        *( outputImgPtr++)= out2;
        *( outputImgPtr++)= out3;


      } /* End of col loop */


      /*--------------------------------------------------------------------
          Closing pixel pair of the row
      --------------------------------------------------------------------*/
      cb = *((uint8*)(cb_ptr));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = *((uint8*)( cb_ptr++ + cr_offset ));
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
        Convert and store the pixel
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb, r, out, out2,  \
                                rTable, gTable, bTable);

      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;


      /*--------------------------------------------------------------------
        Move to the averaging case
      --------------------------------------------------------------------*/
      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
        We need to average from the previous row and store it back
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx );

      /*--------------------------------------------------------------------
        Sixe pixel per pass iteration
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {


        /*------------------------------------------------------------------
                  opening pixel pair
         -----------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cb_ptr)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average the values with previous row.
        ------------------------------------------------------------------*/
        cb_dash = ( *outRawDump  + cb ) >> 1;
        lumaa_dash = ( *( outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = ( *( outRawDump + 2) + cr ) >> 1;
        lumaa2_dash = ( *( outRawDump + 3) + lumaa2 ) >> 1;

        /*------------------------------------------------------------------
          Convert and store the pixels back
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa_dash, lumaa2_dash,
                                  cr_dash, cb_dash, r, out, out2, \
                                  rTable, gTable, bTable);

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
          Save the pixel fore more calculation the macro destroys it
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
                                  cr, cb, r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
          Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));

        /*------------------------------------------------------------------
          need to averge from the previoous pxel pair
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;


        /*------------------------------------------------------------------
          Store the single average pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          The previous row row need to be averaged and stored back
        ------------------------------------------------------------------*/
        lumaa2 = ( *outRawDump  + lumaa2 ) >> 1;

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                        rTable, gTable, bTable);
        *(uint16 *) outRawDump = out;
        outRawDump += 2;


        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Save this lumaa for more averaging the macro kills it
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
          Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);


        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*----------------------------------------------------------------
            Average with previous row
       ----------------------------------------------------------------*/
        cb_dash = ( *outRawDump  + cb_dash ) >> 1;
        lumaa_dash = ( *( outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = ( *( outRawDump + 2) + cr_dash ) >> 1;
        lumaa2_dash = ( *( outRawDump + 3) + lumaa_dash ) >> 1;

        /*------------------------------------------------------------------
          Color convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa_dash, lumaa2_dash, cr_dash, cb_dash,
                                    r, out, out2, \
                                    rTable, gTable, bTable);

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            Next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset ));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Average falls in the middle of the pair. No need to average cb/cr
        ------------------------------------------------------------------*/
        lumaa_dash = ( lumaa1 + lumaa2) >> 1;


        /*------------------------------------------------------------------
          Convert and store the averaged pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash, cr_dash, cb_dash, \
                                          r, out, \
                                          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel one off. Beacuse theres is one before and one
          after. We are doing this to save these pixels for averaging with
          previous row
        ------------------------------------------------------------------*/
        *( outputImgPtr + 1 )= out;



        /*------------------------------------------------------------------
          Save this pixel before the macro kills it
        ------------------------------------------------------------------*/
        lumaa2_dash = lumaa2;

        /*------------------------------------------------------------------
          Convert and store
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
                                  cr_dash, cb_dash, \
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *( outputImgPtr)= out;
        *( outputImgPtr + 2)  = out2;

        /*------------------------------------------------------------------
          We rendered total 3 pixels
        ------------------------------------------------------------------*/
        outputImgPtr += 3;

        /*------------------------------------------------------------------
          average with 3 pixels
        ------------------------------------------------------------------*/
        cb_dash = (  ( *outRawDump ) + cb_dash ) >> 1;
        lumaa1 = (  *(outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = (  *(outRawDump + 2) + cr_dash ) >> 1;
        lumaa2 = (  *(outRawDump + 3) + lumaa2_dash) >> 1;
        lumaa2_dash = (  *( outRawDump  + 4) + lumaa_dash ) >> 1;

        /*------------------------------------------------------------------
          Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, out3, \
          rTable, gTable, bTable);\
        

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out3;
        outRawDump += 2;

      } /* End of col loop */


      /*--------------------------------------------------------------------
        Closing pixel pair
      --------------------------------------------------------------------*/
      cb = *((uint8*)(cb_ptr));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = *((uint8*)(cb_ptr++ + cr_offset ));
      lumaa2 = *((uint8*)(inputImgPtr++));




      /*----------------------------------------------------------------
          Average the values with previous row.
      ----------------------------------------------------------------*/
      *(outRawDump ) =    (uint8) ((*outRawDump  + cb ) >> 1);
      *(outRawDump + 1) = (uint8) ((*( outRawDump + 1) + lumaa1 ) >> 1);
      *(outRawDump + 2) = (uint8) ((*( outRawDump + 2) + cr ) >> 1);
      *(outRawDump + 3) = (uint8) ((*( outRawDump + 3) + lumaa2 ) >> 1);

      /*--------------------------------------------------------------------
        Convert and store pixel pair
      -----------------------------i--------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb, r, out, out2,  \
                                rTable, gTable, bTable);
      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;


      /*--------------------------------------------------------------------
          Take out the average the pixel pair from previous row
      --------------------------------------------------------------------*/
      cb = *(outRawDump );
      lumaa1 = *(outRawDump + 1);
      cr = *(outRawDump + 2);
      lumaa2 = *(outRawDump + 3);

      /*--------------------------------------------------------------------
          convert and store the pixel pair
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb,
                                  r, out, out2, \
                                  rTable, gTable, bTable );

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;


      /*--------------------------------------------------------------------
        wrap the row count and start the average cycle
      --------------------------------------------------------------------*/
      row_cnt = 0;
    }
    else if ( row_cnt == 1 )
    {

      /*----------------------------------------------------------------
        Next row where you keep the averaged pixels
      ----------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx );


      for ( col = col_max ; col; col-=2 )
      {
        /*------------------------------------------------------------------
                First pixel pair
         -----------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cb_ptr)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset )));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Saving for averaging later
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr;
        *outRawDump++ =  (uint8) lumaa2;


        /*------------------------------------------------------------------
            Save lumaa for the next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
           Color convert and store
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
                                  cr, cb, r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));

        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;

        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;



        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        ( *outRawDump++ )      = (uint8) lumaa2;
        outRawDump++;


        lumaa2 = *((uint8*)(inputImgPtr++));

        *outRawDump++       = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++   = (uint8) cr_dash;
        *outRawDump++ =  (uint8) lumaa2;

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);


        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*----------------------------------------------------------------
           next pixel pair
       ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));
        lumaa2 = *((uint8*)(inputImgPtr++));

        lumaa_dash = ( lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
          Saving pixels for averaging with next row
        ------------------------------------------------------------------*/
        *outRawDump++  = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++   = (uint8) cr_dash;
        *outRawDump ++=  (uint8) lumaa2;
        *outRawDump ++=  (uint8) lumaa_dash;
        outRawDump ++;

        /*------------------------------------------------------------------
          Color convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
                                            cr_dash, cb_dash, \
                                            r, out, out2, out3, \
                                            rTable, gTable, bTable);\

        *( outputImgPtr++)= out;
        *( outputImgPtr++)  = out2;
        *( outputImgPtr++)  = out3;

      } /* end of col loop */

      /*--------------------------------------------------------------------
         The closing pixel pair
      --------------------------------------------------------------------*/
      cb = *((uint8*)(cb_ptr));
      lumaa1 = *((uint8*)(inputImgPtr++));
      cr = *((uint8*)(cb_ptr++ + cr_offset));
      lumaa2 = *((uint8*)(inputImgPtr++));

      /*--------------------------------------------------------------------
          Store pixel pair for later averaging
      --------------------------------------------------------------------*/
      *(outRawDump++) = (uint8) cb;
      *(outRawDump++) = (uint8) lumaa1;
      *(outRawDump++) = (uint8) cr;
      *(outRawDump++) = (uint8) lumaa2;

      /*--------------------------------------------------------------------
        color convert and store the pixels
      --------------------------------------------------------------------*/
      IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr, cb, r, out, out2,  \
        rTable, gTable, bTable);

      *outputImgPtr++ = out;
      *outputImgPtr++ = out2;

      row_cnt++;
      outputImgPtr += outDx;


    } /* end of row_cnt == 1 */

    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
    ----------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  );

    /*----------------------------------------------------------------------
        We need to move the cb_ptr in tandom with the inputImgPtr
    ----------------------------------------------------------------------*/
    cb_ptr += rowInc >> 1;


    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

}/* end of function ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133 */


/*===========================================================================

FUNCTION IPL2_UPSIZE_YCBCR442()

DESCRIPTION
    Function which handles the upsizing of an existing existing YCBCR442
    image and positioning it into an o/p buffer

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image
    i_param   - upsize params

ARGUMENTS IN
    o_img_ptr - o/p image


RETURN VALUE
Success or Failure

SIDE EFFECTS
None

===========================================================================*/

extern  ipl_status_type ipl2_upsize_YCbCr442
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
  )
{
  MSG_LOW("inside ipl2_upsize_YCbCr442\n");

  if (ipl2_init() != IPL_SUCCESS)
  {
    IPL2_MSG_FATAL("Call to ipl2_init() failed.\n");
    return(IPL_FAILURE);
  }

  /*------------------------------------------------------------------------
      We only support RGB565/RGB444/666 as an o/p format
  ------------------------------------------------------------------------*/
  if ( o_img_ptr->cFormat == IPL_YCbCr)
  {
    return(ipl2_upSizeAndCrop_YCbCr(i_img_ptr, o_img_ptr,i_param));
  }
  else if ( !((o_img_ptr->cFormat == IPL_RGB565) ||
              (o_img_ptr->cFormat == IPL_RGB444) ||
              (o_img_ptr->cFormat == IPL_RGB666)))
  {
    /*----------------------------------------------------------------------
        Log a message and exit
    ----------------------------------------------------------------------*/
    IPL2_MSG_FATAL( "ipl2_upSizeYCbCr442 failed for unsupported o/p /\
                  color   = %d", o_img_ptr->cFormat );
  }

  /*------------------------------------------------------------------------
      switch on quality value desired
  ------------------------------------------------------------------------*/
  switch ( i_param->qual )
  {

    case IPL_QUALITY_LOW :
      return( ipl2_upSizeAndCrop_YCbCr2RGB( i_img_ptr,
        o_img_ptr,
        i_param ) );
      /*NOTREACHED*/
      //break;

    case IPL_QUALITY_MEDIUM :
      return( ipl2_upSizeRowAvgAndCrop_YCbCr2RGB( i_img_ptr,
        o_img_ptr,
        i_param ) );
      /*NOTREACHED*/
      //break;

    case IPL_QUALITY_HIGH :
      return( ipl2_upSizeAverageAndCrop_YCbCr2RGB(i_img_ptr,
        o_img_ptr,
        i_param ) );
      /*NOTREACHED*/
      //break;

    default:
      return( IPL_FAILURE );
      //break;


  } /* end of switch */

  /*NOTREACHED*/
  return( IPL_SUCCESS );

} /* end of function ipl2_upsize_YCbCr442 */


/*===========================================================================

FUNCTION IPL2_UPSIZEAVERAGEANDCROP_YCBCR420TORGBQVGAMED()

DESCRIPTION
  This function uses an optimized bilinear avreaging scheme to upsize a QCIF
  sized (176 * 144) image to a QVGA fitted image. Size of the QVGA display
  is assumed 240 * 320. So 176 * 144 --> 240 * 320. This is approximaed to
  an upsample of 4/3. ( This ration gives us a lot of computational
  advantage. The quality is medium because the samples of 0.75 and 0.25
  interpolations are approximated to nearest neighbor. Input color scheme is
  YCBCr 4:2:0

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr     - input image frame
  output_img_ptr    - Points to the output image
  i_param           -  Parameters relating to upsize.


RETURN VALUE
  Status - success or failure

SIDE EFFECTS
  Changes the output image buffer

===========================================================================*/
extern ipl_status_type ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133_PT
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register unsigned char* inputImgPtr=i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out, out2, out3;
  register int32 cr, cb, cb_dash, cr_dash;
  register int32 lumaa1, lumaa_dash;
  register int32 lumaa2, lumaa2_dash;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 outDx = o_img_ptr->dx;
  uint8 *outRawDump; /* For every row there is an identical row dumped */
  uint32 row,col_max, row_max;
  uint32 rowInc=0, destInc, cr_offset;
  uint32   row_cnt, col;
  register uint8 *cb_ptr;
/*------------------------------------------------------------------------*/

  MSG_LOW("inside ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133_PT\n");


  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      //break;

    default:
      return( IPL_FAILURE );
      //break;
  }

  /*------------------------------------------------------------------------
        The format of the YCbCr420 packing as follows

  |   (dx*dy) no of Y values = cr_offset )   |1/4 Yvals      |1/4 Yvals    |
  |------------------------------------------|---------------|-------------|
  |------------------------------------------|---------------|-------------|
              Y values                          Cb Vals          Cr Vals

        Following calculation takes us to the beginning of the Cb Vals
  ------------------------------------------------------------------------*/
      cb_ptr = i_img_ptr->clrPtr;

  /*------------------------------------------------------------------------
      Takes us to the offset for Cb Vals
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy  ) >> 2 /* /4 */;


  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;


    /*------------------------------------------------------------------------
      The number rows = o/p size * 0.75
  ------------------------------------------------------------------------*/
  row_max = ( i_param->crop.dy * 3  ) >> 2;

    /*--------------------------------------------------------------------
      Each row has an opening and closing pixel. This will work when the
      upscaling is 234 or more. Because this will guarantee that we use all
      the 176 pixels. to create a row of stength 234. ( Still short of the
      ideal 240).
    ----------------------------------------------------------------------*/
    col_max =  ( ( i_param->crop.dx )  / 4 );


    /*----------------------------------------------------------------------
      Limit the number of cols to the number of trplets which can be fit
      into the QCIF image. which is limited to 58. 58 * 3 runs to 174.
      Plus will open and close with one pixel.
    ----------------------------------------------------------------------*/
    if ( col_max > IPL2_QCIF_COL_CNTPT_BY_3 )
    {
      col_max = IPL2_QCIF_COL_CNTPT_BY_3;
    }
    else if ( col_max & 0x1 )
    {
      col_max--;
    }

    /*----------------------------------------------------------------------
      rowInc = QCIF row size - number of cols needed to render the pic
    ----------------------------------------------------------------------*/
    rowInc = ( IPL2_QCIF_HEIGHT
             - ( col_max * 3   )  ) ;



    /*------------------------------------------------------------------------
        Limit the maximun no of rows to the QCIF size
    ------------------------------------------------------------------------*/
    if ( row_max > IPL2_QCIF_WIDTH )
    {
      row_max = IPL2_QCIF_WIDTH;
    }

  /*------------------------------------------------------------------------
     The destInc = actual frame size - the number of cols rendered in
     every row pass.
  ------------------------------------------------------------------------*/
  destInc = ( o_img_ptr->dx  - ( col_max * 4 ) ) * 2;


  /*------------------------------------------------------------------------
              Now loop through the image once
  ------------------------------------------------------------------------*/
  row_cnt = 0;



  /*------------------------------------------------------------------------
    iterate through the image with row_max and col_max as the number of
    ro/col iterations. Each 3 ros iterated creates a fouth one as average.
  ------------------------------------------------------------------------*/
  for ( row = 0; row < row_max; row++ )
  {

    /*----------------------------------------------------------------------
    For every odd row we need to move the cb/cr ptrs so that they repeate
    the same for the this row as well. In 420 encoding 2 pixels in the
    next row share the same cr/cb values
    ----------------------------------------------------------------------*/
    if ( row & 0x1 )
    {
      cb_ptr -= ( i_img_ptr->dx) >> 1;
    }

    if ( row_cnt == 0 )
    {

      for ( col = col_max; col; col-=2 )
      {


        /*------------------------------------------------------------------
          Opening pixel pair
        ------------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cb_ptr)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Save the lumaa for future calculations
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
            convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
                                  cr, cb, r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
            next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset ));

        /*------------------------------------------------------------------
          average values with the pnextsous pair
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;


        /*------------------------------------------------------------------
          Convert and store the pixel pair
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                        rTable, gTable, bTable);
        *outputImgPtr++ = out;




        /*------------------------------------------------------------------
          Closing pixel from the current pixel pair
        ------------------------------------------------------------------*/
        lumaa2 = *((uint8*)(inputImgPtr++));


        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);


        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;




        /*----------------------------------------------------------------
            Next pixel pair
       ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Average falls in the middle of the pixel pair we dont need to
          average the Cb/Cr pair
        ------------------------------------------------------------------*/
        lumaa_dash = (lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
          Convert and store these 3 pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
                                            cr_dash, cb_dash, \
                                            r, out, out2, out3, \
                                            rTable, gTable, bTable);

        *( outputImgPtr++)= out;
        *( outputImgPtr++)= out2;
        *( outputImgPtr++)= out3;


      } /* End of col loop */




      /*--------------------------------------------------------------------
        Move to the averaging case
      --------------------------------------------------------------------*/
      row_cnt++;

    }
    else if ( row_cnt == 2 )
    {

      /*--------------------------------------------------------------------
        We need to average from the previous row and store it back
      --------------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr - outDx );

      /*--------------------------------------------------------------------
        Sixe pixel per pass iteration
      --------------------------------------------------------------------*/
      for ( col = col_max; col; col-=2 )
      {


        /*------------------------------------------------------------------
                  opening pixel pair
         -----------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cb_ptr)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset)));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Average the values with previous row.
        ------------------------------------------------------------------*/
        cb_dash = ( *outRawDump  + cb ) >> 1;
        lumaa_dash = ( *( outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = ( *( outRawDump + 2) + cr ) >> 1;
        lumaa2_dash = ( *( outRawDump + 3) + lumaa2 ) >> 1;

        /*------------------------------------------------------------------
          Convert and store the pixels back
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa_dash, lumaa2_dash,
                                  cr_dash, cb_dash, r, out, out2, \
                                  rTable, gTable, bTable);

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
          Save the pixel fore more calculation the macro destroys it
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
                                  cr, cb, r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*------------------------------------------------------------------
          Next pixel pair
        ------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));

        /*------------------------------------------------------------------
          need to averge from the previoous pxel pair
        ------------------------------------------------------------------*/
        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;
        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;


        /*------------------------------------------------------------------
          Store the single average pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        /*------------------------------------------------------------------
          The previous row row need to be averaged and stored back
        ------------------------------------------------------------------*/
        lumaa2 = ( *outRawDump  + lumaa2 ) >> 1;

        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
                                        rTable, gTable, bTable);
        *((uint16 *) outRawDump) = out;
        outRawDump += 2;


        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Save this lumaa for more averaging the macro kills it
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
          Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
                                  r, out, out2, \
                                  rTable, gTable, bTable);


        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        /*----------------------------------------------------------------
            Average with previous row
       ----------------------------------------------------------------*/
        cb_dash = ( *outRawDump  + cb_dash ) >> 1;
        lumaa_dash = ( *( outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = ( *( outRawDump + 2) + cr_dash ) >> 1;
        lumaa2_dash = ( *( outRawDump + 3) + lumaa_dash ) >> 1;

        /*------------------------------------------------------------------
          Color convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa_dash, lumaa2_dash, cr_dash, cb_dash,
                                    r, out, out2, \
                                    rTable, gTable, bTable);

        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;

        /*------------------------------------------------------------------
            Next pixel pair
       -------------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset ));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
          Average falls in the middle of the pair. No need to average cb/cr
        ------------------------------------------------------------------*/
        lumaa_dash = ( lumaa1 + lumaa2) >> 1;


        /*------------------------------------------------------------------
          Convert and store the averaged pixel
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa_dash, cr_dash, cb_dash, \
                                          r, out, \
                                          rTable, gTable, bTable);

        /*------------------------------------------------------------------
          Store the pixel one off. Beacuse theres is one before and one
          after. We are doing this to save these pixels for averaging with
          previous row
        ------------------------------------------------------------------*/
        *( outputImgPtr + 1 )= out;



        /*------------------------------------------------------------------
          Save this pixel before the macro kills it
        ------------------------------------------------------------------*/
        lumaa2_dash = lumaa2;

        /*------------------------------------------------------------------
          Convert and store
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
                                  cr_dash, cb_dash, \
                                  r, out, out2, \
                                  rTable, gTable, bTable);

        *( outputImgPtr)= out;
        *( outputImgPtr + 2)  = out2;

        /*------------------------------------------------------------------
          We rendered total 3 pixels
        ------------------------------------------------------------------*/
        outputImgPtr += 3;

        /*------------------------------------------------------------------
          average with 3 pixels
        ------------------------------------------------------------------*/
        cb_dash = (  ( *outRawDump ) + cb_dash ) >> 1;
        lumaa1 = (  *(outRawDump + 1) + lumaa1 ) >> 1;
        cr_dash = (  *(outRawDump + 2) + cr_dash ) >> 1;
        lumaa2 = (  *(outRawDump + 3) + lumaa2_dash) >> 1;
        lumaa2_dash = (  *( outRawDump  + 4) + lumaa_dash ) >> 1;

        /*------------------------------------------------------------------
          Convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
          cr_dash, cb_dash, \
          r, out, out2, out3, \
          rTable, gTable, bTable);\
       
        *((uint16 *) outRawDump) = out;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out2;
        outRawDump += 2;
        *((uint16 *) outRawDump) = out3;
        outRawDump += 2;

      } /* End of col loop */



      /*--------------------------------------------------------------------
        wrap the row count and start the average cycle
      --------------------------------------------------------------------*/
      row_cnt = 0;
    }
    else if ( row_cnt == 1 )
    {

      /*----------------------------------------------------------------
        Next row where you keep the averaged pixels
      ----------------------------------------------------------------*/
      outRawDump =  (uint8 *) (outputImgPtr + outDx );


      for ( col = col_max ; col; col-=2 )
      {
        /*------------------------------------------------------------------
                First pixel pair
         -----------------------------------------------------------------*/
        cb = (int32) (*((uint8*)(cb_ptr)));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr = (int32) (*((uint8*) (cb_ptr++ + cr_offset )));
        lumaa2 = *((uint8*)(inputImgPtr++));

        /*------------------------------------------------------------------
            Saving for averaging later
        ------------------------------------------------------------------*/
        *outRawDump++ = (uint8) cb;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++ = (uint8) cr;
        *outRawDump++ =  (uint8) lumaa2;


        /*------------------------------------------------------------------
            Save lumaa for the next averaging
        ------------------------------------------------------------------*/
        lumaa_dash = lumaa2;

        /*------------------------------------------------------------------
           Color convert and store
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2,
                                  cr, cb, r, out, out2, \
                                  rTable, gTable, bTable);

        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;


        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));

        lumaa2 = ( lumaa_dash + lumaa1 ) >> 1;

        cb = ( cb +  cb_dash  ) >> 1;
        cr = ( cr +  cr_dash ) >> 1 ;



        IPL2_CONVERT_YCBCR_RGB_SINGLE(lumaa2, cr, cb, r, out,
          rTable, gTable, bTable);
        *outputImgPtr++ = out;

        ( *outRawDump++ )      = (uint8) lumaa2;
        outRawDump++;


        lumaa2 = *((uint8*)(inputImgPtr++));

        *outRawDump++       = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++   = (uint8) cr_dash;
        *outRawDump++ =  (uint8) lumaa2;

        /*----------------------------------------------------------------
          Following piece of code is a template for the YCrCB to RGB
          conversion when you find some bug on this make sure you fix all the
          templates used in this file
        ---------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB(lumaa1, lumaa2, cr_dash, cb_dash,
          r, out, out2, \
          rTable, gTable, bTable);


        *outputImgPtr++ = out;
        *outputImgPtr++ = out2;

        /*----------------------------------------------------------------
           next pixel pair
       ----------------------------------------------------------------*/
        cb_dash = *((uint8*)(cb_ptr));
        lumaa1 = *((uint8*)(inputImgPtr++));
        cr_dash = *((uint8*)(cb_ptr++ + cr_offset));
        lumaa2 = *((uint8*)(inputImgPtr++));

        lumaa_dash = ( lumaa1 + lumaa2) >> 1;

        /*------------------------------------------------------------------
          Saving pixels for averaging with next row
        ------------------------------------------------------------------*/
        *outRawDump++  = (uint8) cb_dash;
        *outRawDump++ = (uint8) lumaa1;
        *outRawDump++   = (uint8) cr_dash;
        *outRawDump ++=  (uint8) lumaa2;
        *outRawDump ++=  (uint8) lumaa_dash;
        outRawDump ++;

        /*------------------------------------------------------------------
          Color convert and store the pixels
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCRRGB_3HALFWORD(lumaa1,   lumaa_dash, lumaa2,
                                            cr_dash, cb_dash, \
                                            r, out, out2, out3, \
                                            rTable, gTable, bTable);\

        *( outputImgPtr++)= out;
        *( outputImgPtr++)  = out2;
        *( outputImgPtr++)  = out3;

      } /* end of col loop */


      row_cnt++;
      outputImgPtr += outDx;


    } /* end of row_cnt == 1 */

    /*----------------------------------------------------------------------
          Adjust the pointers to account for Cropping and
          magnification
    ----------------------------------------------------------------------*/
    inputImgPtr = (uint8*)((uint32)inputImgPtr   + rowInc  );

    /*----------------------------------------------------------------------
        We need to move the cb_ptr in tandom with the inputImgPtr
    ----------------------------------------------------------------------*/
    cb_ptr += ( rowInc >> 1 ) ;


    /*----------------------------------------------------------------------
      We need to offst the aleady rendered pesent row. We will start with a
      row offset. We will dump pixels on one more row offset. This done in
      beginning of the row Loop
      --------------------------------------------------------------------*/
    outputImgPtr = (uint16 *) ((uint32 )outputImgPtr   + destInc  ) ;

  } /* End of row loop */

  return(IPL_SUCCESS);

}/* end of function ipl2_upSizeAverageAndCrop_YCbCr420ToRGBQCIFMed_133 */







#if 0
extern ipl_status_type   ipl2_create_NeghborTable
  (
  ipl_rect_type *in_size,
  ipl_rect_type *out_size,
  ipl2_image_upsize_param_type *context
  )
{
  uint32 i, index;
  float xFactor, yFactor;
  uint32 neighbor = 0xffffffff, old_neighbor = 0xffffffff;
/*------------------------------------------------------------------------*/

  xFactor = (float) in_size->dx / (float) context->crop.dx;
  yFactor = (float) in_size->dy / (float) context->crop.dy;

  /*------------------------------------------------------------------------
      Build the neighbors which need to be rendered in the o/p buffer.
      a true means rendered that many times. Later we will add avreage
      weight so we can do generic upsampling by mult and shift.
  ------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      row-wise neighbor map
  ------------------------------------------------------------------------*/
  for ( i = 0; i < context->crop.dx; i++ )
  {
    /*----------------------------------------------------------------------
      ith colomn in o/p should map to neghbor colomn in input.
    ----------------------------------------------------------------------*/
    neighbor = ipl2_roundFloatVal( float(i) * xFactor );

    if ( neighbor == old_neighbor )
    {

      index++;
      if ( index >= IPL2_MAX_UPSIZE )
      {
        return( IPL_FAILURE );
      }

    }
    else
    {
      index = 0;
    }

    context->col_map[neigbor][index] = TRUE;

    old_neighbor = neighbor;

  } /* end of i loop */

  neighbor = 0xffffffff;
  old_neighbor = 0xffffffff;

  /*------------------------------------------------------------------------
      colomn-wise neighbor map
  ------------------------------------------------------------------------*/
  for ( i = 0; i < context->crop.dy; i++ )
  {
    /*----------------------------------------------------------------------
      ith colomn in o/p should map to neghbor colomn in input.
    ----------------------------------------------------------------------*/
    neighbor = ipl2_roundFloatVal( float(i) * yFactor );

    if ( neighbor == old_neighbor )
    {

      index++;
      if ( index >= IPL2_MAX_UPSIZE )
      {
        return( IPL_FAILURE );
      }

    }
    else
    {
      index = 0;
    }

    context->col_map[neigbor][index] = TRUE;

    old_neighbor = neighbor;

  } /* end of i loop */

  /*------------------------------------------------------------------------
      Precalculate the x_max and y_max the upsamling routines will be
       very simple
  ------------------------------------------------------------------------*/
  context->x_max =   ipl2_roundFloatVal( xFactor *
                       (float) context->crop.dx );
  context->y_max =   ipl2_roundFloatVal( yFactor *
                       (float) context->crop.dy );


  return( IPL_FAILURE );

} /* end of function ipl2_create_NeghborTable */


extern ipl_status_type ipl2_upsizeNearestNeighbor_YCbCrToRGB
  (
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type *context
  )
{
  register uint32 row, col;
  register uint16 *rTable;
  register uint16 *gTable;
  register uint16 *bTable;
  register neighbor_map_type  *row_map = context->row_map;
  register neighbor_map_type  *col_map = context->row_map;
/*------------------------------------------------------------------------*/

  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
            initialize the conversion with RGB444 table
       -------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    case IPL_RGB666 :
      /*--------------------------------------------------------------------
            We do not support RGB666 upsample for now
       -------------------------------------------------------------------*/
      return( IPL_FAILURE );
      break;

    default:
      return( IPL_FAILURE );
      break;
  }


  /*------------------------------------------------------------------------
      Iterate once through the the i/p image
  ------------------------------------------------------------------------*/
  for ( row = context->y_max; row; row-- )
  {
    for ( col = context->x_max / 2; col; col-- )
    {

      /*--------------------------------------------------------------------
        Stuff the pixel once
      --------------------------------------------------------------------*/

      /*--------------------------------------------------------------------
          take one YCbCr pair and do the conversion
      --------------------------------------------------------------------*/
      if ( row_map[col][2] )
      {
        /*------------------------------------------------------------------
          Stuff this pixel twice on the o/p buffer
        ------------------------------------------------------------------*/
      }

      if ( row_map[col + 1][2] )
      {
        /*------------------------------------------------------------------
          Stuff this pixel twice on the o/p buffer
        ------------------------------------------------------------------*/
      }

    }

    if ( col_map[row][2] )
    {

      /*--------------------------------------------------------------------
            Copy the just formed row once more into the o/p buffer
      --------------------------------------------------------------------*/

    }

    /*----------------------------------------------------------------------
        offset input by context->rowInc
    ----------------------------------------------------------------------*/

    /*----------------------------------------------------------------------
        Ofsset o/p by context->destInc
    ----------------------------------------------------------------------*/
  }

  return( IPL_FAILURE );

} /* end of function ipl2_upsize_nearestNeighbor */

#endif




/*===========================================================================

FUNCTION ipl2_upSize_arb_low_RGBToRGB

DESCRIPTION
  Fast arbitrary upsize

DEPENDENCIE
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
)
{
  register uint16* inputImgPtr = (uint16*) i_img_ptr->imgPtr;
  register uint16* outputImgPtr = (uint16*) o_img_ptr->imgPtr;

  int outDx = o_img_ptr->dx;
  uint32 row,col;
  int colDup;
  int rowDup;
  int colDupTmp;
  int fromCols, toCols, fromRows, toRows;
  int resize;
  int i;
  int rowId = 0;
  int pitchI, pitchO;
  uint32 inputRowsToUse, inputColsToUse;
  uint32 ocdx, ocdy, icdx, icdy;
  ipl_status_type retval = IPL_SUCCESS;


  int extraInputCols, extraInputColsTmp;

  MSG_LOW("inside ipl2_upSize_arb_low_RGBToRGB\n");


  // we dont stretch from 176 to 320, but 175 to 319.
  ocdx = ocrop->dx - 1; // the first pixel we do for sys_free 
  ocdy = ocrop->dy; 
  icdx = icrop->dx - 1; // the first pixel we do for sys_free
  icdy = icrop->dy; 

  // lets compute how many input colums go how many output columns
  resize = (ocdx << 8) / icdx;
  i = 0;

  // we have 160 entries in our resizeFactor table which tells us for a given
  // resize amount, how many input cols should be mapped to how many Y cols
  while (i < 160 && resize >= resizeFactor[i+3]) i += 3;  
  toCols = resizeFactor[i+1]; 
  fromCols = resizeFactor[i+2]; 

  // print some stats: Resize methoed is a table that tells us how to
  // make say 5 columns out of 3 columns, i.e. dump 1st line, then dump 
  // and duplicate next two lines
  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("Upsizing from %ld cols to %ld cols, or %d times, match is %ld,\nor every %d col to %d cols\n", 
           icdx, ocdx, resize, resizeFactor[i],fromCols, toCols);
  }
 
  // lets compute how many input rows go to how many output rows
  resize = (ocdy << 8) / icdy;
  i = 0;
  while (i < 160 && resize >= resizeFactor[i+3]) i += 3;  
  toRows = resizeFactor[i+1]; 
  fromRows = resizeFactor[i+2]; 

  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("Upsizing from %ld rows to %ld rows, or %d times, match is %ld,\nor every %d rows to %d rows \n",
           icdy, ocdy, resize, resizeFactor[i],
           fromRows, toRows);
  }



  // lets compute how many input columns we are going to use
  // i.e. how many times should we do the 3 pixels go to 5 pixels. Make
  // sure that we dont go over our input width
  if (fromCols*(ocdx/toCols) <= icdx)
    inputColsToUse = fromCols*(ocdx/toCols);
  else
    inputColsToUse = fromCols*(icdx/fromCols);
 
  // lets compute how many input rows we are going to use
  if (fromRows*(ocdy/toRows) <= icdy)
    inputRowsToUse = fromRows*(ocdy/toRows);
  else
    inputRowsToUse = fromRows*(icdy/fromRows);

  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("Should we use %ld input cols or %ld? %ld\n", 
           fromCols*(ocdx/toCols), 
           fromCols*(icdx/fromCols),
           inputColsToUse);
    printf("Should we use %ld input rows or %ld? %ld\n", 
           fromRows*(ocdy/toRows), 
           fromRows*(icdy/fromRows),
           inputRowsToUse);
  }

  // Figure out how many rows we need to duplicate to get exact desired output
  // given from->to row/col ratio. I.e. if we are taking 5 columns and 
  // making 9, that gives us 315 columns if we wanted 320, so the diff 
  // of 5 means we need to dup 5.
  colDup = (ocdx - toCols*(inputColsToUse/fromCols));
  rowDup = (ocdy - toRows*(inputRowsToUse/fromRows));


  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("Our repeat uses %ld input pixel to make %ld output pixels\n", 
           inputColsToUse, toCols*inputColsToUse/fromCols); 
    printf("Need to dup an extra %d pixels\n", colDup);
  }


  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("Our repeat uses %ld input rows to make %ld output rows\n", 
           inputRowsToUse, toRows*inputRowsToUse/fromRows); 
    printf("Need to dup an extra %d rows \n", rowDup);
  }




  // build an array that will tell use exactly how to dump rows
  // We dont want to dup the first line, or the last line # times, rather,
  // we want to intersparce this throughout the row/col
  //
  // because we only use say, 140 out of 144 input lines, there is a little
  // bit of cropping, i.e. if your input image has a 1 pixel wide line around
  // it, the line may get cut off at the bottom.
  rowId = 0;

  // compute extraInputCols. This is the number of input colums pixels
  // we do not use. I know, it is bad to throw out input, but we kinda
  // have in order to not complicate the logic even further
  extraInputCols = icdx - (inputColsToUse);
  if (colDup == 0)
    extraInputCols = 0;
  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
    printf("icdx %ld, ictouse %ld, therefore, Extra input cols is %d\n", 
           icdx, inputColsToUse,
           extraInputCols);


  // compute pitch
  pitchO = o_img_ptr->dx - (ocdx + 1);
  pitchI = i_img_ptr->dx - (inputColsToUse + 1 + extraInputCols);

  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
    printf("input pitch is %d, output pitch is %d\n", pitchI, pitchO);

  // figure out where in the input and output we want to read/write from
  outputImgPtr += (ocrop->x + o_img_ptr->dx * ocrop->y);
  inputImgPtr += (icrop->x + i_img_ptr->dx * icrop->y);

  // lets get down to business!
  rowId = 0;
  for (row = inputRowsToUse; row; row--)
  {

    colDupTmp = colDup;
    extraInputColsTmp = extraInputCols;  

    // always copy the first pixel to the first output and inc
    *outputImgPtr++ = *inputImgPtr++;
    
    
    for (col = inputColsToUse/fromCols; col; col--)
    {
      switch (fromCols)
      {

        // going from 1 input pixel too....
        case 1:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;


          }
        }
        break;

        // going from 2 input pixel too....
        case 2:
        {

          switch (toCols)
          {    
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr++;    
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;    
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;    
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;    
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;    
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;    
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;    
            break;

            default:
              retval = IPL_FAILURE;
            break;

          }
        }
        break;

        // going from 3 input pixel too....
        case 3:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr++;    
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;    
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // going from 4 input pixel too....
        case 4:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // going from 5 input pixel too....
        case 5:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // going from 6 input pixel too....
        case 6:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 5;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2; 
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // going from 7 input pixel too....
        case 7:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 6;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 5;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;

              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    

              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // going from 8 input pixel too....
        case 8:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 7;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 6;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 5;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // going from 9 input pixel too....
        case 9:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 8;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 7;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 6;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 5;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              inputImgPtr += 1;
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        default:
          retval = IPL_FAILURE;
        break;
      }

      if (retval == IPL_FAILURE)
        break;

      // see how many pixels get copied over twice due to us being short,
      // e.g turning 3 rows to 4 makes 176 rows to 232, but if we want 240,
      // then need to dump 8 more columns
      if (colDupTmp > 0)
      {
        if (extraInputColsTmp) 
        {
          *outputImgPtr++ = *inputImgPtr++;
          extraInputColsTmp--; 
        }
        else
        {
          *outputImgPtr = *(outputImgPtr-1);    
          outputImgPtr++;
        }
        colDupTmp--;
      }
    }

    if (retval == IPL_FAILURE)
      break;

    // add in the pitch
    outputImgPtr += pitchO;
    inputImgPtr += pitchI;

    if (rowDup > 0)
    {
      //if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
      //  printf("Going to repeat the first row %d times\n", rowDup);
      for (col = ocdx+1; col; col--)
      {
        *outputImgPtr = *(outputImgPtr - outDx);
        outputImgPtr++;
      }
      outputImgPtr += pitchO;

      rowDup--;
    }
   
    // now, take this last row we created, and see how many times we need
    // to dup it (if it is time to dump that is)
    rowId++;
    if (rowId == fromRows)
    {
      //if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
      //  printf("Time to take %d rows and make %d rows\n", fromRows, toRows);

      for (col = (toRows - fromRows)*outDx; col; col--)
      {
        *outputImgPtr = *(outputImgPtr - outDx);
        outputImgPtr++;
      }
      rowId = 0;
    }

  } 

  return(retval);

} 


/*===========================================================================

FUNCTION ipl2_upSize_qcif2qvga_RGBToRGB

DESCRIPTION
  This function upsize and RGB image 1.8 times in x-dim.
  This function upsize and RGB image 1.6 times in y-dim.

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
extern ipl_status_type ipl_upSize_qcif2qvga_RGBToRGB
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_upsize_param_type* i_param    /* processing param */
)
{
  register uint16* inputImgPtr = (uint16*) i_img_ptr->imgPtr;
  register uint16* outputImgPtr = (uint16*) o_img_ptr->imgPtr;
  register int outDx = o_img_ptr->dx;
  uint32 inInc, destInc;
  uint32 row,col;
  int firstPart, secondPart;

  MSG_LOW("inside ipl2_upSize_qcif2qvga_RGBToRGB\n");

  // figure out where in the output we want to write image
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  // For every 9 output columns, we use 5 input columns. 
  
  // We can only
  // output multiple of 9. Because of this, figure out how
  // many pixel on the input we need to skip per row
  destInc = (i_param->crop.dx - 9*(i_param->crop.dx/9));
  inInc = (i_img_ptr->dx  - (5*(i_param->crop.dx/9)));

  firstPart = destInc/2;
  secondPart = (destInc % 2)? firstPart+1 : firstPart;

  //printf("firstPart is %d, secondPart is %d, to make destInc %d\n",
  //       firstPart, secondPart, destInc);

  for (row = 3*i_param->crop.dy/5; row; row--)
  {
    // now, we are short desInc pixels in the output, so for now,
    // just repeat the last pixel
    for (col = firstPart; col; col-- )
      *outputImgPtr++ = *inputImgPtr;  

    for (col = i_param->crop.dx/9; col; col-- )
    {
      // make 5 pixels into 9
      *outputImgPtr++ = *(inputImgPtr++);  // 1 
      *outputImgPtr++ = *(inputImgPtr);    // 2 
      *outputImgPtr++ = *(inputImgPtr++);  // 3 
      *outputImgPtr++ = *(inputImgPtr);    // 4 
      *outputImgPtr++ = *(inputImgPtr++);  // 5 
      *outputImgPtr++ = *(inputImgPtr);    // 6 
      *outputImgPtr++ = *(inputImgPtr++);  // 7 
      *outputImgPtr++ = *(inputImgPtr);    // 8 
      *outputImgPtr++ = *(inputImgPtr++);  // 9 
    }

    // now, we are short desInc pixels in the output, so for now,
    // just repeat the last pixel
    for (col = secondPart; col; col-- )
      *outputImgPtr++ = *inputImgPtr;  

    // add in the pitch
    outputImgPtr += (o_img_ptr->dx - i_param->crop.dx);

    inputImgPtr += (inInc);

    // duplicate the row above
    if (row%3 != 0) 
    {
      for (col = i_param->crop.dx; col; col--)
      {
        *outputImgPtr = *(outputImgPtr - outDx);
        outputImgPtr++;
      }

      // add in the pitch
      outputImgPtr += (o_img_ptr->dx - i_param->crop.dx);
    }

  } 


  return(IPL_SUCCESS);

} 




/*===========================================================================

FUNCTION ipl2_upSize_arb_med_RGBToRGB

DESCRIPTION
  Fast arbitrary upsize

DEPENDENCIE
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
)
{
  register uint16* inputImgPtr = (uint16*) i_img_ptr->imgPtr;
  register uint16* outputImgPtr = (uint16*) o_img_ptr->imgPtr;
  register uint16 out1, out2, out3;
  register uint8 rTemp1, gTemp1, bTemp1;

  int outDx = o_img_ptr->dx;
  uint32 row,col;
  int colDup;
  int rowDup;
  int colDupTmp;
  int fromCols, toCols, fromRows, toRows;
  int resize;
  int i;
  uint32 outdx, outdy;
  int rowId = 0;
  int whenToDumpARow;
  int rowIdTotal = 0;
  int rowIdDup = 0;
  //int upRows = 0;
  int pitchI, pitchO;
  uint32 inputRowsToUse, inputColsToUse;
  uint32 ocdx, ocdy, icdx, icdy;
  ipl_status_type retval = IPL_SUCCESS;


  int extraInputCols, extraInputColsTmp;

  MSG_LOW("inside ipl2_upSize_arb_med_RGBToRGB\n");


  // we dont stretch from 176 to 320, but 175 to 319.
  ocdx = ocrop->dx - 1; // the first pixel we do for sys_free 
  ocdy = ocrop->dy; 
  icdx = icrop->dx - 1; // the first pixel we do for sys_free
  icdy = icrop->dy; 

  // is resize ratio based on output crop or output size?
  //outdx = o_img_ptr->dx - 1;
  //outdy = o_img_ptr->dy;
  outdx = ocdx;
  outdy = ocdy;

  // lets compute how many input colums go how many output columns
  resize = (outdx << 8) / icdx;
  i = 0;

  // we have 160 entries in our resizeFactor table which tells us for a given
  // resize amount, how many input cols should be mapped to how many Y cols
  while (i < 160 && resize >= resizeFactor[i+3]) i += 3;  
  toCols = resizeFactor[i+1]; 
  fromCols = resizeFactor[i+2]; 

  // print some stats: Resize methoed is a table that tells us how to
  // make say 5 columns out of 3 columns, i.e. dump 1st line, then dump 
  // and duplicate next two lines
  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("first pixel we always just dump, so\n");
    printf("Upsizing from %ld cols to %ld cols, %d match %ld, every %d cols to %d cols\n", icdx, outdx, resize, resizeFactor[i],fromCols, toCols);
  }
 
  // lets compute how many input rows go to how many output rows
  //resize = (ocdy << 8) / icdy;
  resize = (outdy << 8) / icdy;
  i = 0;
  while (i < 160 && resize >= resizeFactor[i+3]) i += 3;  
  toRows = resizeFactor[i+1]; 
  fromRows = resizeFactor[i+2]; 

  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("Upsizing from %ld rows to %ld rows, %d match %ld, every %d rows to %d rows \n", icdy, outdy, resize, resizeFactor[i], fromRows, toRows);
  }



  // lets compute how many input columns we are going to use
  // i.e. how many times should we do the 3 pixels go to 5 pixels. Make
  // sure that we dont go over our input width
  if (fromCols*(outdx/toCols) <= icdx)
    inputColsToUse = fromCols*(outdx/toCols);
  else
    inputColsToUse = fromCols*(icdx/fromCols);
 
  // lets compute how many input rows we are going to use
  if (fromRows*(outdy/toRows) <= icdy)
    inputRowsToUse = fromRows*(outdy/toRows);
  else
    inputRowsToUse = fromRows*(icdy/fromRows);

  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("Should we use %ld input cols or %ld? %ld\n", 
           fromCols*(outdx/toCols), 
           fromCols*(icdx/fromCols),
           inputColsToUse);
    printf("Should we use %ld input rows or %ld? %ld\n", 
           fromRows*(outdy/toRows), 
           fromRows*(icdy/fromRows),
           inputRowsToUse);

  }

  // Figure out how many rows we need to duplicate to get exact desired output
  // given from->to row/col ratio. I.e. if we are taking 5 columns and 
  // making 9, that gives us 315 columns if we wanted 320, so the diff 
  // of 5 means we need to dup 5.
  //colDup = (ocdx - toCols*(inputColsToUse/fromCols));
  colDup = (outdx - toCols*(inputColsToUse/fromCols));
  rowDup = (outdy - toRows*(inputRowsToUse/fromRows));


  if (rowDup != 0)
    whenToDumpARow = outdy/rowDup;
  else
    whenToDumpARow = 100000; // some big number for image height



  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("Our repeat uses %ld input col pixel to make %ld output col pixels\n", 
           inputColsToUse, toCols*inputColsToUse/fromCols); 
    printf("Need to dup an extra %d cols\n", colDup);
  }


  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
  {
    printf("Our repeat uses %ld input rows to make %ld output rows\n", 
           inputRowsToUse, toRows*inputRowsToUse/fromRows); 
    printf("Need to dup an extra %d rows \n", rowDup);

    //printf("Gonna dump that row every %d rows \n", whenToDumpARow);
  }




  // build an array that will tell use exactly how to dump rows
  // We dont want to dup the first line, or the last line # times, rather,
  // we want to intersparce this throughout the row/col
  rowId = 0;

  // compute extraInputCols. This is the number of input colums pixels
  // we do not use. I know, it is bad to throw out input, but we kinda
  // have in order to not complicate the logic even further
  extraInputCols = icdx - (inputColsToUse);
  if (colDup == 0)
    extraInputCols = 0;
  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
    printf("icdx %ld, ictouse %ld, therefore, Extra input cols is %d\n", 
           icdx, inputColsToUse,
           extraInputCols);
  // compute pitch
  pitchO = o_img_ptr->dx - (ocdx + 1);
  pitchI = i_img_ptr->dx - (inputColsToUse + 1 + extraInputCols);



  if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
    printf("input pitch is %d, output pitch is %d\n", pitchI, pitchO);

  // figure out where in the input and output we want to read/write from
  outputImgPtr += (ocrop->x + o_img_ptr->dx * ocrop->y);
  inputImgPtr += (icrop->x + i_img_ptr->dx * icrop->y);

  // lets get down to business!
  rowId = 0;
  //rowIdTotal = 0;
  //upRows = 0;
  
  

  for (row = inputRowsToUse; row; row--)
  {
    // see if we need to skip creating this row when downsizing


    colDupTmp = colDup;
    extraInputColsTmp = extraInputCols;  

    // always copy the first pixel to the first output and inc
    *outputImgPtr++ = *inputImgPtr++;
    
    for (col = inputColsToUse/fromCols; col; col--)
    {
      switch (fromCols)
      {

        // going from 1 input pixel too....
        // from 1 cols
        case 1:
        {
          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;

          }
        }
        break;

        // from 2 cols
        case 2:
        {

          switch (toCols)
          {    
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr++;    
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 3:
              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;

            break;

            case 4:
              out1 = *(inputImgPtr++);   // old 
              out2 = *(inputImgPtr++);   // new1 
              *outputImgPtr++ = out1;

              // pixel 1 is 3/4 old, 1/4 new1
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)*26) + 
                                         (((out2 & 0xF800)>>8)*6))>>5);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)*26) + 
                                         (((out2 & 0x07E0)>>3)*6))>>5);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)*26) + 
                                         (((out2 & 0x001F)<<3)*6))>>5);
              //printf ("2: r %d,, g %d, b %d\n", rTemp1, gTemp1, bTemp1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);


              // pixel 1 is 1/4 old, 3/4 new1
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)*6) + 
                                         (((out2 & 0xF800)>>8)*26))>>5);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)*6) + 
                                         (((out2 & 0x07E0)>>3)*26))>>5);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)*6) + 
                                         (((out2 & 0x001F)<<3)*26))>>5);
              //printf ("3: r %d,, g %d, b %d\n", rTemp1, gTemp1, bTemp1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              *outputImgPtr++ = out2;
            break;


            case 5:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);

              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;    
            break;


            case 6:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;    
            break;

            case 7:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;    
            break;


            case 8:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;    
            break;

            case 9:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;    
            break;

            default:
              retval = IPL_FAILURE;
            break;

          }
        }
        break;

        // from 3 cols
        case 3:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr++;    
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 4:
              /* a b c
               *
               * a d e c  where d 1/2a 1/2b and e is 1/2b 1/2c
               *
               */

              out1 = *(inputImgPtr++);   
              out2 = *(inputImgPtr++);   

              // write out a
              *outputImgPtr++ = out1;

              // compute d 
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                         (((out2 & 0xF800)>>8)))>>1);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                         (((out2 & 0x07E0)>>3)))>>1);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                         (((out2 & 0x001F)<<3)))>>1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              out1 = *(inputImgPtr++);   

              // compute e 
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                         (((out2 & 0xF800)>>8)))>>1);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                         (((out2 & 0x07E0)>>3)))>>1);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                         (((out2 & 0x001F)<<3)))>>1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // write out c
              *outputImgPtr++ = out1;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;


              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;

              *outputImgPtr++ = out2; // babak not good
            break;


            case 6:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 7:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 8:
              out1 = *(inputImgPtr++);   
              out2 = *(inputImgPtr++);    

              // pixel 1
              *outputImgPtr++ = out1;

              // pixel 2 is 3/4 old, 1/4 new1
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)*26) + 
                                         (((out2 & 0xF800)>>8)*6))>>5);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)*26) + 
                                         (((out2 & 0x07E0)>>3)*6))>>5);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)*26) + 
                                         (((out2 & 0x001F)<<3)*6))>>5);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);


              // pixel 3 is 1/4 old, 3/4 new1
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)*6) + 
                                         (((out2 & 0xF800)>>8)*26))>>5);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)*6) + 
                                         (((out2 & 0x07E0)>>3)*26))>>5);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)*6) + 
                                         (((out2 & 0x001F)<<3)*26))>>5);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // pixel 4
              *outputImgPtr++ = out2;


              // pixel 5
              *outputImgPtr++ = out2;

              out1 = out2;
              out2 = *(inputImgPtr++);    

              // pixel 6 is 3/4 old, 1/4 new1
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)*26) + 
                                         (((out2 & 0xF800)>>8)*6))>>5);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)*26) + 
                                         (((out2 & 0x07E0)>>3)*6))>>5);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)*26) + 
                                         (((out2 & 0x001F)<<3)*6))>>5);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);


              // pixel 7 is 1/4 old, 3/4 new1
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)*6) + 
                                         (((out2 & 0xF800)>>8)*26))>>5);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)*6) + 
                                         (((out2 & 0x07E0)>>3)*26))>>5);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)*6) + 
                                         (((out2 & 0x001F)<<3)*26))>>5);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // pixel 5
              *outputImgPtr++ = out2;
            break;

            case 9:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // from 4 cols
        case 4:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;

              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;
            break;


            case 6:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 7:
              /* a b c d
               *
               * a 1 b 2 c 3 d
               *
               */

              out1 = *(inputImgPtr++);  // a 
              out2 = *(inputImgPtr++);  // b 

              // write out a
              *outputImgPtr++ = out1;

              // compute 1 
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                         (((out2 & 0xF800)>>8)))>>1);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                         (((out2 & 0x07E0)>>3)))>>1);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                         (((out2 & 0x001F)<<3)))>>1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // write out b
              *outputImgPtr++ = out2;

              out1 = *(inputImgPtr++);  // c 

              // compute 2 
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                         (((out2 & 0xF800)>>8)))>>1);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                         (((out2 & 0x07E0)>>3)))>>1);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                         (((out2 & 0x001F)<<3)))>>1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // write out c
              *outputImgPtr++ = out1;

              out2 = *(inputImgPtr++);  // d 

              // compute 3 
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                         (((out2 & 0xF800)>>8)))>>1);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                         (((out2 & 0x07E0)>>3)))>>1);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                         (((out2 & 0x001F)<<3)))>>1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // write out d
              *outputImgPtr++ = out2;
            break;

            case 8:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 9:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // from 5 cols
        case 5:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;

              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;

              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;




              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;

            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr++;

              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;



              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;

              // babak //dump this not good 
              *outputImgPtr++ = out2;
            break;

            case 9:
              /*
               * a b c d e     a 1 b 2 c 3 d 4 e
               *
               */
              out1 = *(inputImgPtr++); // a   
              out2 = *(inputImgPtr++); // b

              // write out a
              *outputImgPtr++ = out1;

              // write out 1 
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                         (((out2 & 0xF800)>>8)))>>1);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                         (((out2 & 0x07E0)>>3)))>>1);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                         (((out2 & 0x001F)<<3)))>>1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // write out b
              *outputImgPtr++ = out2;

              // write out 2
              out1 = *(inputImgPtr++);  // c 
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                         (((out2 & 0xF800)>>8)))>>1);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                         (((out2 & 0x07E0)>>3)))>>1);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                         (((out2 & 0x001F)<<3)))>>1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // write out c
              *outputImgPtr++ = out1;


              // write out 3 
              out2 = *(inputImgPtr++);  // d
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                         (((out2 & 0xF800)>>8)))>>1);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                         (((out2 & 0x07E0)>>3)))>>1);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                         (((out2 & 0x001F)<<3)))>>1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // write out d
              *outputImgPtr++ = out2;

              // write out 4
              out1 = *(inputImgPtr++); // e   
              rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                         (((out2 & 0xF800)>>8)))>>1);
              gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                         (((out2 & 0x07E0)>>3)))>>1);
              bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                         (((out2 & 0x001F)<<3)))>>1);
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);

              // write out e
              *outputImgPtr++ = out1;
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // from 6 cols
        case 6:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 5;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;

              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;
            break;


            case 8:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            case 9:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: DUMPING %d to %d columns\n", fromCols, toCols);
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // from 7 cols
        case 7:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 6;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 5;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;

              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              
              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;

              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;

              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // from 8 cols
        case 8:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 7;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 6;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 5;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 1;
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    

              // make 3 out of 2
              out1 = *(inputImgPtr++);    
              out2 = *(inputImgPtr++);    
              rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                   ((out2 & 0xF800)>>8))>>1);
              gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                   ((out2 & 0x07E0)>>3))>>1);
              bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                   ((out2 & 0x001F)<<3))>>1);
              *outputImgPtr++ = out1;
              *outputImgPtr++ = pack_rgb565(rTemp1,gTemp1,bTemp1);
              *outputImgPtr++ = out2;
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }
        break;

        // from 9 cols
        case 9:
        {

          switch (toCols)
          {
            case 1:
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 8;
            break;

            case 2:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 7;
            break;

            case 3:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 6;
            break;

            case 4:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 5;
            break;


            case 5:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 4;
            break;


            case 6:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 3;
            break;

            case 7:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              inputImgPtr += 2;
            break;


            case 8:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              inputImgPtr += 1;
            break;

            case 9:
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;
              *outputImgPtr++ = *inputImgPtr++;    
            break;

            default:
              retval = IPL_FAILURE;
            break;
          }
        }    
        break;

        default:
          retval = IPL_FAILURE;
        break;
      }

      if (retval == IPL_FAILURE)
        break;

      // to optimize, we can do this at the end...
      //
      // spread the colDup evenly, not just dump at the end.
      // if we have any extra input columns, use those first, if not,
      // then start to dump pixels to make up for the colDup
      if (colDupTmp > 0)
      {
        if (extraInputColsTmp) 
        {
          *outputImgPtr++ = *inputImgPtr++;
          extraInputColsTmp--; 
          if (IPL_UPSIZE_DEBUG && 0) //lint !e774 !e506
             printf("taking extraInputCol putting at end %d %d\n", colDupTmp, extraInputColsTmp); 
        }
        else
        {
          *outputImgPtr = *(outputImgPtr-1);    
          outputImgPtr++;
          if (IPL_UPSIZE_DEBUG && 0) //lint !e774 !e506
             printf("just taking last input col and repe %d %d\n", colDupTmp, extraInputColsTmp); 
        }
        colDupTmp--;
      }
    }

    if (retval == IPL_FAILURE)
      break;


    // it is possible that the number of columns we needed to dup wasn't
    // completely duped above, and so we have to unfortunately dup the
    // rest here.
    while (colDupTmp-- > 0)
    {
      *outputImgPtr = *(outputImgPtr-1);    
      outputImgPtr++;
      if (IPL_UPSIZE_DEBUG && 0) //lint !e774 !e506
         printf("kinda scarry, but we have to repeat the last pixel of row\n"); 
    }
   
    
   

    // add in the pitch
    outputImgPtr += pitchO;
    inputImgPtr += pitchI;

    rowIdDup++;
    if (rowIdDup == whenToDumpARow)
      rowIdDup = 0;



#if 1
    // if there are any rows to dump, dump just one at a time, not all at once.
    //if (rowDup > 0 && rowIdDup == 0)
    if (rowDup > 0)
    {
      if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
        printf("Going to dump a row 1 time out of %d times: rowIdTotal %d\n", 
            rowDup, rowIdTotal);

    #if 1
      // dump a row every ocdx/rowDup lines
      for (col = ocdx+1; col; col--)
      {
        *outputImgPtr = *(outputImgPtr - outDx);
        outputImgPtr++;
      }
    #else
      // put a black line for debug purposes
      for (col = ocdx+1; col; col--)
      {
        *outputImgPtr = 0;
        outputImgPtr++;
      }
    #endif
      
      outputImgPtr += pitchO;
      rowDup--;
    }
#endif

    // we just created a new row
    rowId++;
    rowIdTotal++;


#if 1
    // if we are done creating the number of fromRows, then lets pause for
    // a moment while we take these rows and make toRows out of them.
    if (rowId == fromRows)
    {
      rowId = 0;

      switch (fromRows)
      {
        // from 1 row 
        case 1:
        {
          if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
            printf("med: makeing 2 rows from 1 DUMP\n");

          // just duplicate the row x number of times
          for (col = (toRows - fromRows)*outDx; col; col--)
          {
            *outputImgPtr = *(outputImgPtr - outDx);
            outputImgPtr++;
          }
        }
        break;

        // from 2 row 
        case 2:
        {
          switch (toRows)
          {
            case 1:
              // downsize
            break;

            case 2:
              // noop
            break;
  
            case 3:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);

              // take the new row we wrote, and the one above it, find the
              // average, and put the average in the middle of the two 
              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      c  (1/2a 1/2b)
                 *   >       b
                 */
                out1 = *(outputImgPtr-2*outDx);    
                out2 = *(outputImgPtr-outDx);    
  
                rTemp1 = (unsigned char) ((((out1 & 0xF800)>>8) + 
                                           ((out2 & 0xF800)>>8))>>1);
                gTemp1 = (unsigned char) ((((out1 & 0x07E0)>>3) + 
                                           ((out2 & 0x07E0)>>3))>>1);
                bTemp1 = (unsigned char) ((((out1 & 0x001F)<<3) + 
                                           ((out2 & 0x001F)<<3))>>1);
  
                *(outputImgPtr-outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);
                *(outputImgPtr) = out2;
                outputImgPtr++;
              }
              outputImgPtr += pitchO;
  
            break;
  
            case 4:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);

              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      c  (3/4a 1/4b)
                 *   >       d  (1/4a 3/4b)
                 *           b
                 */
                out1 = *(outputImgPtr-2*outDx);    
                out2 = *(outputImgPtr-outDx);    
  
                // pixel c is 3/4 old, 1/4 new1
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)*26) + 
                                           (((out2 & 0xF800)>>8)*6))>>5);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)*26) + 
                                           (((out2 & 0x07E0)>>3)*6))>>5);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)*26) + 
                                           (((out2 & 0x001F)<<3)*6))>>5);
                *(outputImgPtr - outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);


                // pixel d is 1/4 old, 3/4 new1
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)*6) + 
                                           (((out2 & 0xF800)>>8)*26))>>5);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)*6) + 
                                           (((out2 & 0x07E0)>>3)*26))>>5);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)*6) + 
                                           (((out2 & 0x001F)<<3)*26))>>5);
                *outputImgPtr = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // move b to last row
                *(outputImgPtr + outDx) = out2;

                outputImgPtr++;
              }
              outputImgPtr += (pitchO + o_img_ptr->dx);
  
            break;
  
            // just dump the same row over and over again for now
            default:
            {
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows DUMP\n",fromRows,toRows);
              
              // just duplicate the row x number of times
              if (toRows > fromRows)
              {
                for (col = (toRows - fromRows)*outDx; col; col--)
                {
                  *outputImgPtr = *(outputImgPtr - outDx);
                  outputImgPtr++;
                }
              }
            }
            break;
          }
        }
        break;

        // from 3 row 
        case 3:
        {
          switch (toRows)
          {
            case 1:
              // downsize
            break;

            case 2:
              // downsize
            break;
  
            case 3:
              // noop
            break;
  
            case 4:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);

              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      1  (1/2a 1/2b)
                 *    c      2  (1/2b 1/2c)
                 *   >       c
                 */
                out1 = *(outputImgPtr-3*outDx);  // a
                out2 = *(outputImgPtr-2*outDx);  // b
                out3 = *(outputImgPtr-outDx);    // c
  
                // write pixel 1: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 2*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);


                // write pixel 2: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out3 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out3 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out3 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel c:
                *(outputImgPtr) = out3;

                outputImgPtr++;
              }
              outputImgPtr += pitchO;
  
            break;

            case 5:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      1  (1/2a 1/2b)
                 *    c      b  (1/2b 1/2c)
                 *   >       2
                 *           c
                 */
                out1 = *(outputImgPtr-3*outDx);  // a
                out2 = *(outputImgPtr-2*outDx);  // b
  
                // write pixel 1: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 2*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write out pixel b
                out1 = *(outputImgPtr-outDx);  // c
                *(outputImgPtr - outDx) = out2;

                // write pixel 2: 1/2 b , 1/2 c
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *outputImgPtr = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel c:
                *(outputImgPtr + outDx) = out1;

                outputImgPtr++;
              }
              outputImgPtr += (outDx + pitchO);
            break;
  
  
            // just dump the same row over and over again for now
            default:
            {
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows DUMP\n",fromRows,toRows);
              
              // just duplicate the row x number of times
              if (toRows > fromRows)
              {
                for (col = (toRows - fromRows)*outDx; col; col--)
                {
                  *outputImgPtr = *(outputImgPtr - outDx);
                  outputImgPtr++;
                }
              }
            }
          }
        }
        break;

        // from 4 row 
        case 4:
        {
          switch (toRows)
          {
            case 1:
              // downsize
            break;

            case 2:
              // downsize
            break;
  
            case 3:
              // downsize
            break;

            case 4:
              // noop
            break;
  
            case 5:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);

              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      b  
                 *    c      c 
                 *    d      1  (1/2c 1/2d)  
                 *   >       d
                 */
                out1 = *(outputImgPtr-2*outDx);  // c
                out2 = *(outputImgPtr-outDx);    // d
  
                // write pixel 1: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel d:
                *(outputImgPtr) = out2;
                outputImgPtr++;
              }
              outputImgPtr += pitchO;
            break;

            case 6:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      1  (1/2a 1/2b)
                 *    c      b  
                 *    d      c
                 *   >       2  (1/2c 1/2d)
                 *           d
                 */
                out1 = *(outputImgPtr-4*outDx);  // a
                out2 = *(outputImgPtr-3*outDx);  // b

                // a in right place
  
                // write pixel 1: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 3*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel b
                out1 = *(outputImgPtr-2*outDx);  // c
                *(outputImgPtr - 2*outDx) = out2;

                // write pixel c
                *(outputImgPtr - 1*outDx) = out1;
                out2 = *(outputImgPtr-1*outDx);  // d

                // write pixel 2: 1/2 b , 1/2 c
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel d:
                *(outputImgPtr + outDx) = out1;

                outputImgPtr++;
              }
              outputImgPtr += (outDx + pitchO);
            break;

            case 7:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      1  (1/2a 1/2b)
                 *    c      b 
                 *    d      2  (1/2b 1/2c)
                 *   >       c
                 *           3  (1/2c 1/2d)
                 *           d
                 */
                out1 = *(outputImgPtr-4*outDx);  // a
                out2 = *(outputImgPtr-3*outDx);  // b

                // a in right place
  
                // write pixel 1: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 3*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write out pixel b
                out1 = *(outputImgPtr-2*outDx);  // c
                *(outputImgPtr - 2*outDx) = out2;

                // write pixel 2: 1/2 b , 1/2 c
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                out2 = *(outputImgPtr-outDx);  // d
                *(outputImgPtr - outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel c:
                *(outputImgPtr) = out1;

                // write pixel 3: 1/2 c , 1/2 d
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr + outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel d:
                *(outputImgPtr + 2*outDx) = out2;

                outputImgPtr++;
              }
              outputImgPtr += (2*outDx + pitchO);
            break;
  
            // just dump the same row over and over again for now
            default:
            {
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows DUMP\n",fromRows,toRows);
              
              // just duplicate the row x number of times
              if (toRows > fromRows)
              {
                for (col = (toRows - fromRows)*outDx; col; col--)
                {
                  *outputImgPtr = *(outputImgPtr - outDx);
                  outputImgPtr++;
                }
              }
            }
          }
        }
        break;

        // from 5 row 
        case 5:
        {
          switch (toRows)
          {
            case 1:
              // downsize
            break;

            case 2:
              // downsize
            break;
  
            case 3:
              // downsize
            break;

            case 4:
              // downsize
            break;

            case 5:
              // noop
            break;
  
            case 6:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      b  
                 *    c      c 
                 *    d      d   
                 *    e      1  (1/2d 1/2e)    
                 *   >       e
                 */
                out1 = *(outputImgPtr-2*outDx);  // d
                out2 = *(outputImgPtr-outDx);    // e
  
                // write pixel 1: 1/2d , 1/2 e
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel d:
                *(outputImgPtr) = out2;
                outputImgPtr++;
              }
              outputImgPtr += pitchO;
            break;

            case 7:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 * 5  a      a
                 * 4  b      b  
                 * 3  c      1  (1/2b 1/2c)
                 * 2  d      c
                 * 1  e      d
                 *   >       2  (1/2d 1/2e)
                 *           e
                 */
                out1 = *(outputImgPtr-4*outDx);  // b
                out2 = *(outputImgPtr-3*outDx);  // c

                // b in right place
  
                // write pixel 1: 1/2 b , 1/2 c
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 3*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel c
                out1 = *(outputImgPtr-2*outDx);  // d
                *(outputImgPtr - 2*outDx) = out2;

                // write pixel d
                out2 = *(outputImgPtr-1*outDx);  // e
                *(outputImgPtr - 1*outDx) = out1;

                // write pixel 2: 1/2 d , 1/2 e
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel e:
                *(outputImgPtr + outDx) = out2;

                outputImgPtr++;
              }
              outputImgPtr += (outDx + pitchO);
            break;

            case 8:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      b  
                 *    c      1  (1/2b 1/2c) 
                 *    d      c 
                 *    e      2  (1/2c 1/2d)
                 *   >       d
                 *           3  (1/2d 1/2e)
                 *           e
                 */
                out1 = *(outputImgPtr-4*outDx);  // b
                out2 = *(outputImgPtr-3*outDx);  // c

                // b in right place
  
                // write pixel 1: 1/2 b , 1/2 c
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 3*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write out pixel c
                out1 = *(outputImgPtr-2*outDx);  // d
                *(outputImgPtr - 2*outDx) = out2;

                // write pixel 2: 1/2 c , 1/2 d
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                out2 = *(outputImgPtr-outDx);  // e
                *(outputImgPtr - outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel d:
                *(outputImgPtr) = out1;

                // write pixel 3: 1/2 d , 1/2 e
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr + outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel e:
                *(outputImgPtr + 2*outDx) = out2;

                outputImgPtr++;
              }
              outputImgPtr += (2*outDx + pitchO);
            break;


            case 9:
            {
              uint16 out4, out5;
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);

              for (col = ocdx+1; col; col--)
              {
                /*
                 *5   a      a
                 *4   b      1 (1/2a 1/2b) 
                 *3   c      b   
                 *2   d      2 (1/2b 1/2c)
                 *1   e      c 
                 *   >       3 (1/2c 1/2d)
                 *           d  
                 *           4 (1/2d 1/2e)
                 *           e
                 */

                out1 = *(outputImgPtr-5*outDx);  // a
                out2 = *(outputImgPtr-4*outDx);  // b
                out3 = *(outputImgPtr-3*outDx);  // c
                out4 = *(outputImgPtr-2*outDx);  // d
                out5 = *(outputImgPtr-1*outDx);  // e

                // write pixel 1: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 4*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel b
                *(outputImgPtr - 3*outDx) = out2;
  
                // write pixel 2: 1/2 b , 1/2 c
                rTemp1 = (unsigned char) (((((out2 & 0xF800)>>8)) + 
                                           (((out3 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out2 & 0x07E0)>>3)) + 
                                           (((out3 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out2 & 0x001F)<<3)) + 
                                           (((out3 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 2*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel c
                *(outputImgPtr - outDx) = out3;

                // write pixel 3: 1/2 c , 1/2 d
                rTemp1 = (unsigned char) (((((out3 & 0xF800)>>8)) + 
                                           (((out4 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out3 & 0x07E0)>>3)) + 
                                           (((out4 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out3 & 0x001F)<<3)) + 
                                           (((out4 & 0x001F)<<3)))>>1);

                *(outputImgPtr) = pack_rgb565(rTemp1,gTemp1,bTemp1); // wr 3

                // write pixel d:
                *(outputImgPtr + outDx) = out4;

                // write pixel 4: 1/2 d , 1/2 e
                rTemp1 = (unsigned char) (((((out4 & 0xF800)>>8)) + 
                                           (((out5 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out4 & 0x07E0)>>3)) + 
                                           (((out5 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out4 & 0x001F)<<3)) + 
                                           (((out5 & 0x001F)<<3)))>>1);
                *(outputImgPtr + 2*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel e:
                *(outputImgPtr + 3*outDx) = out5;

                outputImgPtr++;

#if 0
                out1 = *(outputImgPtr-5*outDx);  // a
                out2 = *(outputImgPtr-4*outDx);  // b

                // a in right place
  
                // write pixel 1: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 4*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel b
                out1 = *(outputImgPtr-3*outDx);  // c
                *(outputImgPtr - 3*outDx) = out2;
  
                // write pixel 2: 1/2 b , 1/2 c
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                out2 = *(outputImgPtr-2*outDx);  // d
                *(outputImgPtr - 2*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel 3: 1/2 c , 1/2 d
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);

                out3 = *(outputImgPtr-outDx);   // e
                *(outputImgPtr - outDx) = out1; // wr c
                *(outputImgPtr) = pack_rgb565(rTemp1,gTemp1,bTemp1); // wr 3

                // write pixel d:
                *(outputImgPtr + outDx) = out2;

                // write pixel 4: 1/2 d , 1/2 e
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out3 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out3 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out3 & 0x001F)<<3)))>>1);
                *(outputImgPtr + 2*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel e:
                *(outputImgPtr + 3*outDx) = out3;

                outputImgPtr++;
#endif
              }
              outputImgPtr += (3*outDx + pitchO);
            }
            break;

            // just dump the same row over and over again for now
            default:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows DUMP\n",fromRows,toRows);
              
              // just duplicate the row x number of times
              if (toRows > fromRows)
              {
                for (col = (toRows - fromRows)*outDx; col; col--)
                {
                  *outputImgPtr = *(outputImgPtr - outDx);
                  outputImgPtr++;
                }
              }
              break;
          }
        }
        break;

        // from 6 row 
        case 6:
        {
          switch (toRows)
          {
            case 1:
              // downsize
            break;

            case 2:
              // downsize
            break;
  
            case 3:
              // downsize
            break;

            case 4:
              // downsize
            break;
  
            case 5:
              // downsize
            break;

            case 6:
              // noop
            break;

            case 7:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      b  
                 *    c      c 
                 *    d      d  
                 *    e      e
                 *    f      1 (1/2e 1/2f)
                 *   >       f
                 */
                out1 = *(outputImgPtr-2*outDx);  // e
                out2 = *(outputImgPtr-outDx);    // f

                // write pixel 1: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write out pixel f
                *(outputImgPtr) = out2;

                outputImgPtr++;
              }
              outputImgPtr += pitchO;
            break;
  
            // just dump the same row over and over again for now
            default:
            {
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows DUMP\n",fromRows,toRows);
              
              // just duplicate the row x number of times
              if (toRows > fromRows)
              {
                for (col = (toRows - fromRows)*outDx; col; col--)
                {
                  *outputImgPtr = *(outputImgPtr - outDx);
                  outputImgPtr++;
                }
              }
            }
            break;
          }
        }
        break;

        // from 7 row 
        case 7:
        {
          switch (toRows)
          {
            case 1:
              // downsize
            break;

            case 2:
              // downsize
            break;

            case 3:
              // downsize
            break;

            case 4:
              // downsize
            break;
  
            case 5:
              // downsize
            break;

            case 6:
              // downsize
            break;

            case 7:
              // noop
            break;
  
            case 8:
              //printf("making 8 from 7, rowID:%d time this done %d\n", 
              //    rowIdTotal, ++upRows);
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      b  
                 *    c      c 
                 *    d      d    
                 *    e      e    
                 *    f      f    
                 *    g      1 (1/2f 1/2g)
                 *   >       g
                 */
                out1 = *(outputImgPtr-2*outDx);  // f
                out2 = *(outputImgPtr-outDx);    // g
  
                // write pixel 1: 1/2 f , 1/2 g
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel d:
                *(outputImgPtr) = out2;
                outputImgPtr++;
              }
              outputImgPtr += pitchO;
            break;

            case 9:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 * 7  a      a
                 * 6  b      b  
                 * 5  c      c 
                 * 4  d      d    
                 * 3  e      1 (1/2d 1/2e)   
                 * 2  f      e 
                 * 1  g      f 
                 *   >       2 (1/2f 1/2g)
                 *           g
                 *
                 */
                out1 = *(outputImgPtr-4*outDx);  // d
                out2 = *(outputImgPtr-3*outDx);  // e
  
                // write pixel 1: 1/2 e , 1/2 f
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - 3*outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel e:
                out1 = *(outputImgPtr-2*outDx);  // f
                *(outputImgPtr-2*outDx) = out2;

                // write pixel f:
                out2 = *(outputImgPtr-outDx);  // g
                *(outputImgPtr-outDx) = out1;

                // write pixel 2: 1/2 f , 1/2 g
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel g:
                *(outputImgPtr + outDx) = out2;

                outputImgPtr++;

              }
              outputImgPtr += (pitchO + outDx);
            break;

            // just dump the same row over and over again for now
            default:
            {
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows DUMP\n",fromRows,toRows);
              
              // just duplicate the row x number of times
              if (toRows > fromRows)
              {
                for (col = (toRows - fromRows)*outDx; col; col--)
                {
                  *outputImgPtr = *(outputImgPtr - outDx);
                  outputImgPtr++;
                }
              }
            }
            break;
          }
        }
        break;

        // from 8 row 
        case 8:
        {
          switch (toRows)
          {
            case 1:
              // downsize
            break;

            case 2:
              // downsize
            break;
  
            case 3:
              // downsize
            break;

            case 4:
              // downsize
            break;

            case 5:
              // downsize
            break;

            case 6:
              // downsize
            break;
  
            case 7:
              // downsize
            break;

            case 8:
              // noop
            break;
  
            case 9:
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows avg\n",fromRows,toRows);
              for (col = ocdx+1; col; col--)
              {
                /*
                 *    a      a
                 *    b      b  
                 *    c      c 
                 *    d      1  (1/2c 1/2d)  
                 *   >       d
                 */
                out1 = *(outputImgPtr-2*outDx);  // c
                out2 = *(outputImgPtr-outDx);    // d
  
                // write pixel 1: 1/2 a , 1/2 b
                rTemp1 = (unsigned char) (((((out1 & 0xF800)>>8)) + 
                                           (((out2 & 0xF800)>>8)))>>1);
                gTemp1 = (unsigned char) (((((out1 & 0x07E0)>>3)) + 
                                           (((out2 & 0x07E0)>>3)))>>1);
                bTemp1 = (unsigned char) (((((out1 & 0x001F)<<3)) + 
                                           (((out2 & 0x001F)<<3)))>>1);
                *(outputImgPtr - outDx) = pack_rgb565(rTemp1,gTemp1,bTemp1);

                // write pixel d:
                *(outputImgPtr) = out2;
                outputImgPtr++;
              }
              outputImgPtr += pitchO;
            break;

            // just dump the same row over and over again for now
            default:
            {
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows DUMP\n",fromRows,toRows);
              
              // just duplicate the row x number of times
              if (toRows > fromRows)
              {
                for (col = (toRows - fromRows)*outDx; col; col--)
                {
                  *outputImgPtr = *(outputImgPtr - outDx);
                  outputImgPtr++;
                }
              }
            }
            break;
          }
        }
        break;


        // from 9 row 
        case 9:
        {
          switch (toRows)
          {
            case 1:
              // downsize
            break;

            case 2:
              // downsize
            break;
  
            case 3:
              // downsize
            break;

            case 4:
              // downsize
            break;

            case 5:
              // downsize
            break;

            case 6:
              // downsize
            break;
  
            case 7:
              // downsize
            break;

            case 8:
              // downsize
            break;
  
            case 9:
              // noop
            break;

            // just dump the same row over and over again for now
            default:
            {
              if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
                printf("med: turning %d to %d rows DUMP\n",fromRows,toRows);
              
              // just duplicate the row x number of times
              if (toRows > fromRows)
              {
                for (col = (toRows - fromRows)*outDx; col; col--)
                {
                  *outputImgPtr = *(outputImgPtr - outDx);
                  outputImgPtr++;
                }
              }
            }
            break;
          }
        }
        break;

        // just dump the same row over and over again for now
        default:
        {
          if (IPL_UPSIZE_DEBUG) //lint !e774 !e506
             printf("med: turning %d to %d rows DUMP\n",fromRows,toRows);
              
          // just duplicate the row x number of times
          if (toRows > fromRows)
          {
            for (col = (toRows - fromRows)*outDx; col; col--)
            {
              *outputImgPtr = *(outputImgPtr - outDx);
              outputImgPtr++;
            }
          }
        }
        break;
      }
    } 
#endif



  }

  return(retval);

} 



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
)
{
  register uint16* inputImgPtr = (uint16*) i_img_ptr->imgPtr;
  register uint16* outputImgPtr = (uint16*) o_img_ptr->imgPtr;
  register uint16 out1, out2, out3, out4;
  register int outDx = o_img_ptr->dx;
  uint8 rTemp1, rTemp2, rFin;
  uint8 gTemp1, gTemp2, gFin;
  uint8 bTemp1, bTemp2, bFin;
  uint32 inInc, destInc;
  uint32 row,col;
  int32 repX, repY;

  MSG_LOW("inside ipl2_upSize_133x_RGBToRGB\n");
  out1 = out2 = out3 = out4 = 0;

  // figure out where in the output we want to write image
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  // For every 4 output pixels, we use 3 input pixels. We can only
  // output multiple of 4. Because of this, figure out how
  // many pixel on the input we need to skip per row
  //inInc = (i_img_ptr->dx  - (3*(i_param->crop.dx/4)));
  //destInc = (o_img_ptr->dx  - 4*(i_param->crop.dx/4));
  inInc = (i_img_ptr->dx  - (3*(i_param->crop.dx/4)));
  destInc = (o_img_ptr->dx  - i_param->crop.dx);

  repX = i_param->crop.dx - (4*(i_param->crop.dx/4));
  repY = i_param->crop.dy - (3*(i_param->crop.dy/4) + 
    (3*(i_param->crop.dy/4))/3);

  // we output 4 lines for every 3 input lines red
  for ( row = 3*i_param->crop.dy/4; row; row-- )
  {
    for (col = i_param->crop.dx/4; col; col-- )
    {
      // make 4 pixels into 6
      out1 = *(inputImgPtr++);  
      out2 = *(inputImgPtr++);  
      out4 = *(inputImgPtr++);  

      // average 2 and 4 to get 3
      unpack_rgb565(out2, &rTemp1, &gTemp1, &bTemp1);
      unpack_rgb565(out4, &rTemp2, &gTemp2, &bTemp2);
      rFin = (rTemp1 + rTemp2) >> 1;
      gFin = (gTemp1 + gTemp2) >> 1;
      bFin = (bTemp1 + bTemp2) >> 1;
      out3 = pack_rgb565(rFin,gFin,bFin);

      *outputImgPtr++ = out1;
      *outputImgPtr++ = out2;
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;
    }

    // repeat the last pixel a few times to get it exact width
    for (col = repX; col; col--)
      *outputImgPtr++ = out4;

    // add in the pitch
    outputImgPtr += (destInc);
    inputImgPtr += (inInc);

    // every 2nd row, avergage with 3rd
    if (row%3 == 0)
    {
      for (col = i_param->crop.dx; col; col--)
      {
        *outputImgPtr = *(outputImgPtr - outDx);
        outputImgPtr++;
      }

      // add in the pitch
      outputImgPtr += (destInc);
    }
  } 

  // repeat the last row a few lines
  for (row = repY; row; row-- )
  {
    for (col = i_param->crop.dx; col; col--)
    {
      *outputImgPtr = *(outputImgPtr - outDx);
      outputImgPtr++;
    }

    // add in the pitch
    outputImgPtr += (destInc);
  }





  return(IPL_SUCCESS);

} 



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
)
{
  register uint16* inputImgPtr = (uint16*) i_img_ptr->imgPtr;
  register uint16* outputImgPtr = (uint16*) o_img_ptr->imgPtr;
  register uint16 out1, out2, out3, out4, out5, out6;
  register int outDx = o_img_ptr->dx;
  uint8 rTemp1, rTemp2, rTemp3, rFin;
  uint8 gTemp1, gTemp2, gTemp3, gFin;
  uint8 bTemp1, bTemp2, bTemp3, bFin;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint32 inInc, destInc;
  uint32 input_row_size = i_img_ptr->dx;
  uint32 row,col;

  MSG_LOW("inside ipl2_upSize_150x_RGBToRGB\n");

  // figure out where in the output we want to write image
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  // For every 6 output pixels, we use 4 input pixels. We can only
  // output multiple of 6. Because of this, figure out how
  // many pixel on the input we need to skip per row
  inInc = (i_img_ptr->dx  - (4*(i_param->crop.dx/6)));
  destInc = (o_img_ptr->dx  - 6*(i_param->crop.dx/6));

  // we output 3 lines for every 2 input lines red
  for ( row = i_param->crop.dy/3; row; row-- )
  {
    outRawDump = ( outputImgPtr + 1*outDx);

    for (col = i_param->crop.dx/6; col; col-- )
    {
      // make 4 pixels into 6
      out1 = *inputImgPtr;  
      out2 = *(inputImgPtr + 1);  
      out3 = *(inputImgPtr + 2);  
      out5 = *(inputImgPtr + 3);  

      out6 = *(inputImgPtr + 4);  



      // average 3 and 5 to get 4
      unpack_rgb565(out3, &rTemp1, &gTemp1, &bTemp1);
      unpack_rgb565(out5, &rTemp2, &gTemp2, &bTemp2);
      rFin = (rTemp1 + rTemp2) >> 1;
      gFin = (gTemp1 + gTemp2) >> 1;
      bFin = (bTemp1 + bTemp2) >> 1;
      out4 = pack_rgb565(rFin,gFin,bFin);

      // average 1 and 3 to get 2
      unpack_rgb565(out1, &rTemp3, &gTemp3, &bTemp3);
      rFin = (rTemp1 + rTemp3) >> 1;
      gFin = (gTemp1 + gTemp3) >> 1;
      bFin = (bTemp1 + bTemp3) >> 1;
      out2 = pack_rgb565(rFin,gFin,bFin);

      // average 5 and 6 to get 6
      unpack_rgb565(out6, &rTemp3, &gTemp3, &bTemp3);
      rFin = (rTemp2 + rTemp3) >> 1;
      gFin = (gTemp2 + gTemp3) >> 1;
      bFin = (bTemp2 + bTemp3) >> 1;
      out6 = pack_rgb565(rFin,gFin,bFin);


      // 3 is avg of 3 and 2
      unpack_rgb565(out2, &rTemp3, &gTemp3, &bTemp3);
      rFin = (rTemp1 + rTemp3) >> 1;
      gFin = (gTemp1 + gTemp3) >> 1;
      bFin = (bTemp1 + bTemp3) >> 1;
      out3 = pack_rgb565(rFin,gFin,bFin);


      *outputImgPtr++ = out1;
      *outputImgPtr++ = out2;
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;
      *outputImgPtr++ = out5;
      *outputImgPtr++ = out6;


      // make 4 pixels into 6
      out1 = *(inputImgPtr + input_row_size);  
      out2 = *(inputImgPtr + 1 + input_row_size);  
      out3 = *(inputImgPtr + 2 + input_row_size);  
      out5 = *(inputImgPtr + 3 + input_row_size);  

      out6 = *(inputImgPtr + 4 + input_row_size);  


      // average 3 and 5 to get 4
      unpack_rgb565(out3, &rTemp1, &gTemp1, &bTemp1);
      unpack_rgb565(out5, &rTemp2, &gTemp2, &bTemp2);
      rFin = (rTemp1 + rTemp2) >> 1;
      gFin = (gTemp1 + gTemp2) >> 1;
      bFin = (bTemp1 + bTemp2) >> 1;
      out4 = pack_rgb565(rFin,gFin,bFin);

      // average 1 and 3 to get 2
      unpack_rgb565(out1, &rTemp3, &gTemp3, &bTemp3);
      rFin = (rTemp1 + rTemp3) >> 1;
      gFin = (gTemp1 + gTemp3) >> 1;
      bFin = (bTemp1 + bTemp3) >> 1;
      out2 = pack_rgb565(rFin,gFin,bFin);

      // average 5 and 6 to get 6
      unpack_rgb565(out6, &rTemp3, &gTemp3, &bTemp3);
      rFin = (rTemp2 + rTemp3) >> 1;
      gFin = (gTemp2 + gTemp3) >> 1;
      bFin = (bTemp2 + bTemp3) >> 1;
      out6 = pack_rgb565(rFin,gFin,bFin);


      // 3 is avg of 3 and 2
      unpack_rgb565(out2, &rTemp3, &gTemp3, &bTemp3);
      rFin = (rTemp1 + rTemp3) >> 1;
      gFin = (gTemp1 + gTemp3) >> 1;
      bFin = (bTemp1 + bTemp3) >> 1;
      out3 = pack_rgb565(rFin,gFin,bFin);


      inputImgPtr += 4;

 
      // write our output
      *(outRawDump + outDx) = out1;
      *(outRawDump++) = out1;

      *(outRawDump + outDx) = out2;
      *(outRawDump++) = out2;

      *(outRawDump + outDx) = out3;
      *(outRawDump++) = out3;

      *(outRawDump + outDx) = out4;
      *(outRawDump++) = out4;

      *(outRawDump + outDx) = out5;
      *(outRawDump++) = out5;

      *(outRawDump + outDx) = out6;
      *(outRawDump++) = out6;
    }

    outputImgPtr += (destInc + 2*outDx);

    // skip a row on the input pointers
    inputImgPtr += (inInc  + input_row_size);
  } 


  return(IPL_SUCCESS);

} 




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
)
{
  register uint16* inputImgPtr= (uint16 *) i_img_ptr->imgPtr;
  register uint16* outputImgPtr =(uint16 *) o_img_ptr->imgPtr;
  register uint16 out1, out2, out3, out4;
  register int outDx = o_img_ptr->dx;
  uint8 rTemp1, rTemp4, rTemp3, rFin;
  uint8 gTemp1, gTemp4, gTemp3, gFin;
  uint8 bTemp1, bTemp4, bTemp3, bFin;
  uint16 *outRawDump; /* For every row there is an identical row dumped */
  uint16 *out_row_start; /* for saving start of row */
  uint32 row,col;
  int32 dest_index;

  MSG_LOW("ipl2_upSize_2x_RGBToRGB\n");

  /*------------------------------------------------------------------------
    initialize the index to starting position in the output buffer.
    It is x offset (no of coloumns) + y offset (no of rows) * rowSize
  ------------------------------------------------------------------------*/
  dest_index = (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);
  outputImgPtr += dest_index;
  out_row_start = outputImgPtr;

  // we write two output rows for every input row
  for (row = 0; row < i_param->crop.dy; row+=2)
  {
    outputImgPtr = out_row_start + row*outDx;
    outRawDump = (outputImgPtr + outDx);
    inputImgPtr = ((uint16 *) i_img_ptr->imgPtr) + (row*i_img_ptr->dx/2);

    /*----------------------------------------------------------------------
      Each row and the adjacent row is rendered in this loop. Since we do
      4 pixels at a time we initialze the col to dx/4.
    ----------------------------------------------------------------------*/
    for ( col = i_param->crop.dx/4 - 1; col; col-- )
    {
      out1 = *inputImgPtr;  /* corresponds to Y1 in the pic */
      out3 = *(inputImgPtr + 1);  /* corresponds to Y2 in the pic */
      out4 = *(inputImgPtr + 2);  /* corresponds to Y2 in the pic */

      // average 1 and 3 to get 2
      unpack_rgb565(out1, &rTemp1, &gTemp1, &bTemp1);
      unpack_rgb565(out3, &rTemp3, &gTemp3, &bTemp3);
      rFin = (rTemp1 + rTemp3) >> 1;
      gFin = (gTemp1 + gTemp3) >> 1;
      bFin = (bTemp1 + bTemp3) >> 1;
      out2 = pack_rgb565(rFin,gFin,bFin);

      // average 4 and 3 to get 4
      unpack_rgb565(out4, &rTemp4, &gTemp4, &bTemp4);
      rFin = (rTemp4 + rTemp3) >> 1;
      gFin = (gTemp4 + gTemp3) >> 1;
      bFin = (bTemp4 + bTemp3) >> 1;
      out4 = pack_rgb565(rFin,gFin,bFin);

      *outputImgPtr++ = out1;
      *outputImgPtr++ = out2;
      *outputImgPtr++ = out3;
      *outputImgPtr++ = out4;

      /*--------------------------------------------------------------------
           Now dump the third set of pixels using the same scheme
      --------------------------------------------------------------------*/
      *outRawDump++ = out1;
      *outRawDump++ = out2;
      *outRawDump++ = out3;
      *outRawDump++ = out4;

      inputImgPtr += 2;
    } 
  } 


  return(IPL_SUCCESS);
} 




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
)
{
  register uint16* inputImgPtr = (uint16 *) i_img_ptr->imgPtr;
  register uint16* outputImgPtr = (uint16 *) o_img_ptr->imgPtr;
  register uint16 out1, out2, out3, out4, out5;
  register int outDx = o_img_ptr->dx;
  uint8 rTemp1, rTemp4, rFin;
  uint8 gTemp1, gTemp4, gFin;
  uint8 bTemp1, bTemp4, bFin;
  uint16 *out_row_start; /* for saving start of row */
  uint16 *outRawDump;  /* For every row there is an identical row dumped */
  uint16 *outRawDump2; /* For every row there is an identical row dumped */
  uint32 row,col;


  MSG_LOW("inside ipl_upSize_250x_RGBToRGB\n");

  // see where we should start writing our output
  outputImgPtr += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y);

  // we do 5 output lines at a time
  out_row_start = outputImgPtr;
  for (row = 0; row < i_param->crop.dy; row += 5)
  {
    // setup the pointers to row 2 and 3 which are duplicates of 1st row.
    outputImgPtr = out_row_start + row*outDx;
    outRawDump = (outputImgPtr +  1*outDx);
    outRawDump2 = (outputImgPtr + 2*outDx);

    inputImgPtr = ((uint16 *) i_img_ptr->imgPtr) + (2*(row/5)*i_img_ptr->dx);

    for (col = i_param->crop.dx/10; col; col--)
    {
      // Do first set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      out1 = *inputImgPtr;  
      out4 = *(inputImgPtr + 1);  
      out5 = *(inputImgPtr + 2);  

      // average 1 and 3 to get 2
      unpack_rgb565(out1, &rTemp1, &gTemp1, &bTemp1);
      unpack_rgb565(out4, &rTemp4, &gTemp4, &bTemp4);
      rFin = (2*rTemp1 + rTemp4)/3;
      gFin = (2*gTemp1 + gTemp4)/3;
      bFin = (2*bTemp1 + bTemp4)/3;
      out2 = pack_rgb565(rFin,gFin,bFin);

      rFin = (rTemp1 + 2*rTemp4)/3;
      gFin = (gTemp1 + 2*gTemp4)/3;
      bFin = (bTemp1 + 2*bTemp4)/3;
      out3 = pack_rgb565(rFin,gFin,bFin);

      unpack_rgb565(out5, &rTemp1, &gTemp1, &bTemp1);
      rFin = (rTemp1 + rTemp4)/2;
      gFin = (gTemp1 + gTemp4)/2;
      bFin = (bTemp1 + bTemp4)/2;
      out5 = pack_rgb565(rFin,gFin,bFin);

      inputImgPtr += 2;  

      *outputImgPtr++ = out1;
      *outRawDump++ = out1;
      *outRawDump2++ = out1;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;
      *outRawDump2++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;
      *outRawDump2++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;
      *outRawDump2++ = out4;

      *outputImgPtr++ = out5;
      *outRawDump++ = out5;
      *outRawDump2++ = out5;


      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      out1 = *inputImgPtr;  
      out4 = *(inputImgPtr + 1);  
      out5 = *(inputImgPtr + 2);  

      // average 1 and 3 to get 2
      unpack_rgb565(out1, &rTemp1, &gTemp1, &bTemp1);
      unpack_rgb565(out4, &rTemp4, &gTemp4, &bTemp4);
      rFin = (2*rTemp1 + rTemp4)/3;
      gFin = (2*gTemp1 + gTemp4)/3;
      bFin = (2*bTemp1 + bTemp4)/3;
      out2 = pack_rgb565(rFin,gFin,bFin);

      rFin = (rTemp1 + 2*rTemp4)/3;
      gFin = (gTemp1 + 2*gTemp4)/3;
      bFin = (bTemp1 + 2*bTemp4)/3;
      out3 = pack_rgb565(rFin,gFin,bFin);

      unpack_rgb565(out5, &rTemp1, &gTemp1, &bTemp1);
      rFin = (rTemp1 + rTemp4)/2;
      gFin = (gTemp1 + gTemp4)/2;
      bFin = (bTemp1 + bTemp4)/2;
      out5 = pack_rgb565(rFin,gFin,bFin);


      inputImgPtr += 2;  

      *outputImgPtr++ = out1;
      *outRawDump++ = out1;
      *outRawDump2++ = out1;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;
      *outRawDump2++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;
      *outRawDump2++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;
      *outRawDump2++ = out4;

      *outputImgPtr++ = out5;
      *outRawDump++ = out5;
      *outRawDump2++ = out5;
    }

    // setup the pointers to row 2 and 3 which are duplicates of 1st row.
    outputImgPtr = out_row_start + (row+3)*outDx;
    outRawDump = (outputImgPtr +  outDx);
    inputImgPtr = ((uint16 *) i_img_ptr->imgPtr) +((2*(row/5)+1)*i_img_ptr->dx);

    // now take next row of Ys and make two lines out of it
    for ( col = i_param->crop.dx/10 - 0; col; col-- )
    {
      // Do first set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      out1 = *inputImgPtr;  
      out4 = *(inputImgPtr + 1);  
      out5 = *(inputImgPtr + 2);  

      // average 1 and 3 to get 2
      unpack_rgb565(out1, &rTemp1, &gTemp1, &bTemp1);
      unpack_rgb565(out4, &rTemp4, &gTemp4, &bTemp4);
      rFin = (2*rTemp1 + rTemp4)/3;
      gFin = (2*gTemp1 + gTemp4)/3;
      bFin = (2*bTemp1 + bTemp4)/3;
      out2 = pack_rgb565(rFin,gFin,bFin);

      rFin = (rTemp1 + 2*rTemp4)/3;
      gFin = (gTemp1 + 2*gTemp4)/3;
      bFin = (bTemp1 + 2*bTemp4)/3;
      out3 = pack_rgb565(rFin,gFin,bFin);

      unpack_rgb565(out5, &rTemp1, &gTemp1, &bTemp1);
      rFin = (rTemp1 + rTemp4)/2;
      gFin = (gTemp1 + gTemp4)/2;
      bFin = (bTemp1 + bTemp4)/2;
      out5 = pack_rgb565(rFin,gFin,bFin);


      inputImgPtr += 2;  

      *outputImgPtr++ = out1;
      *outRawDump++ = out1;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;

      *outputImgPtr++ = out5;
      *outRawDump++ = out5;


      // Do second set of 2 -> 5 pixels 
      // We are output to row 1,2,3
      out1 = *inputImgPtr;  
      out4 = *(inputImgPtr + 1);  
      out5 = *(inputImgPtr + 2);  

      // average 1 and 3 to get 2
      unpack_rgb565(out1, &rTemp1, &gTemp1, &bTemp1);
      unpack_rgb565(out4, &rTemp4, &gTemp4, &bTemp4);
      rFin = (2*rTemp1 + rTemp4)/3;
      gFin = (2*gTemp1 + gTemp4)/3;
      bFin = (2*bTemp1 + bTemp4)/3;
      out2 = pack_rgb565(rFin,gFin,bFin);

      rFin = (rTemp1 + 2*rTemp4)/3;
      gFin = (gTemp1 + 2*gTemp4)/3;
      bFin = (bTemp1 + 2*bTemp4)/3;
      out3 = pack_rgb565(rFin,gFin,bFin);

      unpack_rgb565(out5, &rTemp1, &gTemp1, &bTemp1);
      rFin = (rTemp1 + rTemp4)/2;
      gFin = (gTemp1 + gTemp4)/2;
      bFin = (bTemp1 + bTemp4)/2;
      out5 = pack_rgb565(rFin,gFin,bFin);


      inputImgPtr += 2;  

      *outputImgPtr++ = out1;
      *outRawDump++ = out1;

      *outputImgPtr++ = out2;
      *outRawDump++ = out2;

      *outputImgPtr++ = out3;
      *outRawDump++ = out3;

      *outputImgPtr++ = out4;
      *outRawDump++ = out4;

      *outputImgPtr++ = out5;
      *outRawDump++ = out5;
    }
  } 


  return(IPL_SUCCESS);
} 



/*lint -restore */

