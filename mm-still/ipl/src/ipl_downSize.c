

/*===========================================================================

                        IPL2_DOWNSIZE . C

GENERAL DESCRIPTION

  This file contains the function handlers to take care of various
  downsizing/downsampling filters.

  CALL FLOW for IPL2_DOWNSIZE_YCBCR420
                    ipl2_handle_downSize
                          |
                      ____|______________________
                      |                         |
                      | (i_param==NULL)         |(RGB5665/444 )
                      |                    ipl2_downsize_crop()
               ipl2_downsize_ycbcr420
                       |
  ----------------------------------
  |               |                |
  |(YCbCr442 o/p) |(RGB565/444 o/p)| (RGB666 o/p)
  |               |                |
  |               |     ipl2_downsize_ycbcr420ToRGB666
  |  ipl2_downsize_ycbcr420ToRGB
ipl2_downsize_ycbcr420ToYcbCr442

EXTERNALIZED FUNCTIONS

  ipl2_downsize_ycbcr420()

    Function which handles the downsizing of an existing YUV420 encoded
    image to an arbitrarily smaller sized RGB flavour.


INITIALIZATION AND SEQUENCING REQUIREMENTS

  The library will function properly only if the ipl2_init()
  function is called before calling anything else.

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_downSize.c#1 $

                            EDIT HISTORY FOR FILE


when        who    what, where, why
--------    ---    ----------------------------------------------------------
01/27/05   babakf sped up color conversion rgb to ycbcr
05/24/04   srk    Linted version
05/20/04   srk    Initial Creation.
===========================================================================*/

/*=========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#include <stdio.h>
#include <string.h>

#include "ipl_types.h"          
#include "ipl_xform.h"          

#include "ipl_qvp.h"          
#include "ipl_downSize.h"          



/*lint -save -e504, constant value boolean is totally okay */
/*lint -save -e506, constant value boolean is totally okay */
/*lint -save -e508, constant value boolean is totally okay */
/*lint -save -e509, constant value boolean is totally okay */
/*lint -save -e613, dont worry about null input here */
/*lint -save -e703, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e704, let me use if(1) */
/*lint -save -e713, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e734, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e736, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e737, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e747, errors in create gamma curve are okay */
/*lint -save -e774, constant value boolean is totally okay */
/*lint -save -e818, lets not worry about const optimization right now */
/*lint -save -e834, this warning is just dumb */
/*lint -save -e826, we should look into this someday */


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
extern const uint32 ipl2_r666[];
extern const uint32 ipl2_g666[];
extern const uint32 ipl2_b666[];


/*--------------------------------------------------------------------------
 *             Lookup table for RGB To YCbCr conversion.
 *--------------------------------------------------------------------------*/
extern int16 ipl2_rgb565ToYR[];/* R to Y conversion normalized RGB565 */
extern int16 ipl2_rgb565ToYG[];/* G to Y conversion normalized RGB565 */
extern int16 ipl2_rgb565ToYB[];/* B to Y conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCbR[];/* R to Cb conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCbG[];/* G to Cb conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCbB[];/* B to Cb conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCrR[];/* R to Cr conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCrG[];/* G to Cr conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCrB[];/* B to Cr conversion normalized RGB565 */


/*===========================================================================
                          FUNCTION PROTOTYPES
===========================================================================*/
static ipl_status_type ipl2_handle_downSize
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_downsize_param_type* i_param    /* processing param */
);


static ipl_status_type ipl2_downsize_ycbcr420
(
  ipl_image_type* i_img_ptr,     /* Points to the input image   */
  ipl_image_type* o_img_ptr    /* Points to the output image  */
);

static ipl_status_type ipl2_downsize_ycbcr420lp
(
  ipl_image_type* i_img_ptr,     /* Points to the input image   */
  ipl_image_type* o_img_ptr    /* Points to the output image  */
);

static ipl_status_type ipl2_downsize_ycrcb420lp
(
  ipl_image_type* i_img_ptr,     /* Points to the input image   */
  ipl_image_type* o_img_ptr    /* Points to the output image  */
);


static ipl_status_type ipl2_downsize_ycbcr420ToYcbCr442
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
);

static ipl_status_type ipl2_downsize_ycbcr420ToRGB
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
);

static ipl_status_type ipl2_downsize_ycbcr420lpToRGB
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
);

static ipl_status_type ipl2_downsize_ycrcb420lpToRGB
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
);


static ipl_status_type ipl2_downsize_ycbcr420ToRGB666
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
);

static ipl_status_type ipl2_downsize_rgb565
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_downsize_param_type* i_param    /* processing param */

);

static ipl_status_type ipl2_downsize_RGB565ToYCbCr
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl2_image_downsize_param_type* i_param
);

static ipl_status_type ipl2_downsize_RGB565ToRGB565
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl2_image_downsize_param_type* i_param
);




/*===========================================================================
                          FUNCTION DEFINITIONS
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
)
{




/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
            Call the init routine. Mulitiple initializations does not hurt.
  ------------------------------------------------------------------------*/
  if ( ipl2_init() != IPL_SUCCESS )
  {
    IPL2_MSG_FATAL( "ipl2_downsize :: /\
                    Could not initialize IPL2_IPL lookup tables");
    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
      if somebody passing as NULLs we will return FAIL
  ------------------------------------------------------------------------*/
  if( !i_img_ptr || !o_img_ptr )
  {
    IPL2_MSG_FATAL( "ipl2_downsize failed for passing NULL pointer(s) /\
           i_frame_ptr= %p, o_img_ptr = %p", i_img_ptr,
           o_img_ptr );
    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
      Check to see both input and o/p has valid buffers
  ------------------------------------------------------------------------*/
  if( !i_img_ptr->imgPtr || !o_img_ptr->imgPtr )
  {

    IPL2_MSG_FATAL( "ipl2_downsize failed for passing NULL img_pointer(s) /\
       i_img_ptr->imgPtr = %p, o_img_ptr->imgPtr = %p", i_img_ptr->imgPtr,
       o_img_ptr->imgPtr );

    return( IPL_FAILURE );
  }

  /*------------------------------------------------------------------------
      This is a strict down size
  ------------------------------------------------------------------------*/
  if ( (i_img_ptr->dx < o_img_ptr->dx) ||
      (i_img_ptr->dy < o_img_ptr->dy))
  {
    /*----------------------------------------------------------------------
        If parameters does not seem like a downsize lets return.
    ----------------------------------------------------------------------*/
    IPL2_MSG_FATAL( "ipl2_downsize failed for invalid diemnsions /\
       i_img_ptr->dx= %lu, o_img_ptr->dx = %lu", i_img_ptr->dx,
       o_img_ptr->dx );

    IPL2_MSG_FATAL( "ipl2_downsize failed for invalid diemnsions /\
       i_img_ptr->dy = %lu, o_img_ptr->dy = %lu", i_img_ptr->dy,
       o_img_ptr->dy );

    return (IPL_FAILURE);
  }


  /*------------------------------------------------------------------------
    Call the handler for downsize
  ------------------------------------------------------------------------*/
  return( ipl2_handle_downSize( i_img_ptr, o_img_ptr, i_param) );
} /* End of ipl2_downsize */


/*===========================================================================

FUNCTION IPL2_HANDLE_DOWNSIZE()

DESCRIPTION
    Function which handles the downsizing of an existing existing image and
    positioning it into an o/p buffer ( if the size is defined as part of
    the param ).

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
static ipl_status_type ipl2_handle_downSize
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_downsize_param_type* i_param    /* processing param */
)
{
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
    If we param is NULL this is a straight downsize
  ------------------------------------------------------------------------*/
  if( !i_param )
  {
    switch( i_img_ptr->cFormat)
    {

      case IPL_YCbCr420_FRAME_PK :
        return( ipl2_downsize_ycbcr420 ( i_img_ptr, o_img_ptr ) );

      case IPL_YCbCr420_LINE_PK :
        return( ipl2_downsize_ycbcr420lp ( i_img_ptr, o_img_ptr ) );

      case IPL_YCrCb420_LINE_PK :
        return( ipl2_downsize_ycrcb420lp ( i_img_ptr, o_img_ptr ) );


      default:

        /*------------------------------------------------------------------
            Call the legacy IPL
        ------------------------------------------------------------------*/
        return( ipl_downsize(
                          i_img_ptr,
                          o_img_ptr,
                          NULL
                        ) );
        /*NOTREACHED*/
          return( IPL_FAILURE );
        //break;


    } /* end of switch */

  }
  else
  {
    /*----------------------------------------------------------------------
      Param is valid and we need to relocate the image
    ----------------------------------------------------------------------*/
    switch( i_img_ptr->cFormat )
    {
      case IPL_RGB565 :
      /*--------------------------------------------------------------------
          create the function which will take crop as param and downsize
          and render the image
      --------------------------------------------------------------------*/
        return( ipl2_downsize_rgb565(i_img_ptr,o_img_ptr,i_param ) );

      case IPL_YCbCr420_LINE_PK :
        return( ipl2_downsize_ycbcr420lp ( i_img_ptr, o_img_ptr ) );

      case IPL_YCrCb420_LINE_PK :
        return( ipl2_downsize_ycrcb420lp ( i_img_ptr, o_img_ptr ) );

      default:
        return( IPL_FAILURE );
        /*NOTREACHED*/
    }
  }
} /* end of function ipl2_handle_downSize */

/*===========================================================================

FUNCTION IPL2_DOWNSIZE_YCBCR420()

DESCRIPTION
    Function which handles the downsizing of an existing YUV420 encoded
    image to an arbitrarily smaller sized RGB or YCbCr4:4:2 flavour.

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
static ipl_status_type ipl2_downsize_ycbcr420
(
  ipl_image_type* i_img_ptr,    /* Points to the input image           */
  ipl_image_type* o_img_ptr     /* Points to the output image          */
)
{
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      Lets switch based on o/p now
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {

    case IPL_RGB444 :
    case IPL_RGB565 :
      return( ipl2_downsize_ycbcr420ToRGB(
                                    i_img_ptr,
                                    o_img_ptr
                                    ) );

    case IPL_RGB666:
      return(ipl2_downsize_ycbcr420ToRGB666(
                                              i_img_ptr,
                                              o_img_ptr
                                              ) );

    case IPL_YCbCr:
      return( ipl2_downsize_ycbcr420ToYcbCr442 (
                                              i_img_ptr,
                                              o_img_ptr ) );

    default:
      return( IPL_FAILURE );

  } /* end of switch */
} /* end of function ipl2_downsize_ycbcr420 */



/*===========================================================================

FUNCTION IPL2_DOWNSIZE_YCBCR420()

DESCRIPTION
    Function which handles the downsizing of an existing YUV420 encoded
    image to an arbitrarily smaller sized RGB or YCbCr4:4:2 flavour.

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
static ipl_status_type ipl2_downsize_ycbcr420lp
(
  ipl_image_type* i_img_ptr,    /* Points to the input image           */
  ipl_image_type* o_img_ptr     /* Points to the output image          */
)
{
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      Lets switch based on o/p now
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {

    case IPL_RGB444 :
    case IPL_RGB565 :
      return( ipl2_downsize_ycbcr420lpToRGB( i_img_ptr, o_img_ptr) );

    default:
      return( IPL_FAILURE );

  } 
} /* end of function ipl2_downsize_ycbcr420 */


/*===========================================================================

FUNCTION IPL2_DOWNSIZE_YCRCB420()

DESCRIPTION
    Function which handles the downsizing of an existing YCrCb420 encoded
    image to an arbitrarily smaller sized RGB or YCrCb:422 flavour.

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
static ipl_status_type ipl2_downsize_ycrcb420lp
(
  ipl_image_type* i_img_ptr,    /* Points to the input image           */
  ipl_image_type* o_img_ptr     /* Points to the output image          */
)
{
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      Lets switch based on o/p now
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {

    case IPL_RGB444 :
    case IPL_RGB565 :
      return( ipl2_downsize_ycrcb420lpToRGB( i_img_ptr, o_img_ptr) );

    default:
      return( IPL_FAILURE );

  } 
} /* end of function ipl2_downsize_ycbcr420 */





/*===========================================================================
 *
 *
 *
 *
 *
 *
 *=========================================================================*/
static ipl_status_type ipl2_downsize_rgb565
(
  ipl_image_type*               i_img_ptr, /* Points to the input image   */
  ipl_image_type*               o_img_ptr, /* Points to the output image  */
  ipl2_image_downsize_param_type* i_param    /* processing param */

)
{
  switch( o_img_ptr->cFormat )
  {
    case IPL_YCbCr:
      return( ipl2_downsize_RGB565ToYCbCr( i_img_ptr, o_img_ptr,
                                              i_param ) );
      /*NOTREACHED*/
    break;

    case IPL_RGB565:
      return( ipl2_downsize_RGB565ToRGB565( i_img_ptr, o_img_ptr,
                                               i_param ) );
      /*NOTREACHED*/
    break;

    default:
      return( IPL_FAILURE );
      /*NOTREACHED*/
    break;
  }

}
/*===========================================================================

FUNCTION IPL2_DOWNSIZE_YCBCR420TOYCBCR442()

DESCRIPTION
    Function which handles the downsizing of an existing YUV420 encoded
    image to an arbitrarily smaller sized YCbCr image.

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image

ARGUMENTS OUT
    o_img_ptr - o/p image


RETURN VALUE
    returns success or failure

SIDE EFFECTS
None

===========================================================================*/
static ipl_status_type ipl2_downsize_ycbcr420ToYcbCr442
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
)
{
  uint32 odx;   /* size of output row in no of pixels */
  uint32 idx;    /* size of input row in no pixels */
  uint32 ody;   /* size of output coloumn in no of pixels */
  uint32 idy;    /* size of output coloumn in no of pixels */
  uint32 resizeFactorX;  /* sampling facotr in in X dirction */
  uint32 resizeFactorY;  /* sampling factor in Y direction */
  uint32 row, col, i, j; /* index variables */
  uint32 cr_offset;      /* offset from cb_ptr to get the right cr */
  uint8 cb, cr, luma;    /* YCbCr variables */
  uint8 *cb_ptr;         /* start of cb array in the i/p buffer */
/*------------------------------------------------------------------------*/


  /*------------------------------------------------------------------------
      initialize i/p and o/p sizes
  ------------------------------------------------------------------------*/
  odx = o_img_ptr->dx;
  idx = i_img_ptr->dx;
  ody = o_img_ptr->dy;
  idy = i_img_ptr->dy;


  /*------------------------------------------------------------------------
      calculate the cb plane pointer = start of image + number of pixels
      in image.
  ------------------------------------------------------------------------*/
  cb_ptr = i_img_ptr->imgPtr + ( i_img_ptr->dx * i_img_ptr->dy );

  /*------------------------------------------------------------------------
      calculate the offset from cb array to get the corresponding cr val
      ignoring lint error 703
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy ) >> 2 ; //lint !e703

  /*------------------------------------------------------------------------
      Calculate X and Y resize factors
  ------------------------------------------------------------------------*/
  resizeFactorX =
    ( idx * 128 + ( odx >> 1 ) ) / odx; //lint !e703
  resizeFactorY =
      ( idy * 128 + (ody >> 1)) / ody; //lint !e703



  /*------------------------------------------------------------------------
       We are creating a one to one mapping b/w o/p and iput the mapped
       pixels are determined by resize factor. Lets find out who the
       lucky ones are.
                                              ____________________________
       _______________                        |                           |
       |              |                       |                           |
       |     O/P      |  (mapp sampled pixels |          i/P              |
       |              |  <------------------->|                           |
       |              |                       |                           |
       |              |                       |                           |
       |______________|                       |                           |
                                              |                           |
                                              |___________________________|

  ------------------------------------------------------------------------*/
  for ( row=0; row < o_img_ptr->dy; row++ )
  {

    /*----------------------------------------------------------------------
      i will hold the row we will sample for this particular O/P row
      'row'
                                        _______________________________
                                        |                              |
      ____________________              |                              |
      |                   |             |                              |
      |      O/P          |             |                              |
      |___________________| sampled row |______________________________|
      |____'row'th row____|<----------->|_______ row i in O/P__________|
      |                   |             |                              |
      |___________________|             |                              |
                                        |                              |
                                        |______________________________|

         The row no 'row' corresponds to samples from row no i
        after the following calculation
   -----------------------------------------------------------------------*/
    i = (uint16)( ( (uint32)( (row * resizeFactorY) << 9) + 0x8000L) >> 16); //lint !e703

    for ( col=0; col < o_img_ptr->dx; col++ )
    {
      /*--------------------------------------------------------------------
            j calculation will pin point the exact colomn from where we
            will sample this pixel
      --------------------------------------------------------------------*/

      j = (uint16) (((uint32) ( (col * resizeFactorX) <<9 ) + 0x8000L)>>16); //lint !e703

      /*--------------------------------------------------------------------
          If coloumn and j are both add increment j
      --------------------------------------------------------------------*/
      if ( ( (col & 0x1 ) + ( j & 0x1) ) & 0x1 )
      {
        j++;
      }

      /*--------------------------------------------------------------------
                          Pick and store Cb for even cols
      --------------------------------------------------------------------*/
      if( ! (j & 0x1) )
      {

        cb = *(uint8*) (cb_ptr + (
             (  (  ( i >> 1) * ( idx >> 1 ) ) //lint !e703
                + ( j >> 1 ) ) ) ); //lint !e703

        luma =
          *((uint8*) (i_img_ptr->imgPtr + (i* idx + j)));

        *(uint8*)(o_img_ptr->imgPtr +
                  (((row * odx ) + col) * 2)) =
                                                        (uint8)cb;
      }
      /*------------------------------------------------------------------
                    pick store cr of odd coloumns Compltes
                    |Cb|Y1|Cr|Y2 packing of a pixel pair
      ------------------------------------------------------------------*/
      else
      {
        cr = *( (uint8*) (cb_ptr + cr_offset
                   + ( ( (i >> 1) * (idx  >> 1) ) + //lint !e703
                    ( ( j  >> 1 )  )  ) ) ); //lint !e703
        luma = *((uint8*)(i_img_ptr->imgPtr + (i * idx + j) ) );

        *(uint8*)(o_img_ptr->imgPtr +
                  ( ( (row * odx ) + col ) * 2 ) ) = (uint8)cr;
      }

      *(uint8*)(o_img_ptr->imgPtr +
          (((row * odx) + col ) * 2 + 1)) =
                                                  (uint8)luma;

    } /* End of row for_loop */

  } /* End of Image for_loop */

  return( IPL_SUCCESS );

} /* end of finction ipl2_downsize_ycbcr420ToYcbCr442 */

/*===========================================================================

FUNCTION IPL2_DOWNSIZE_YCBCR420TORGB()

DESCRIPTION
    Function which handles the downsizing of an existing YUV420 encoded
    image to an arbitrarily smaller sized RGB flavour.

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image

ARGUMENTS OUT
    o_img_ptr - o/p image


RETURN VALUE
  returns success or failure

SIDE EFFECTS
None

===========================================================================*/
static ipl_status_type ipl2_downsize_ycbcr420ToRGB
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
)
{
  register uint8 *in_img_buf = i_img_ptr->imgPtr;  /* O/P image buffer */
  register uint16 *out_img_buf = (uint16 *) o_img_ptr->imgPtr;  /* O/P image buffer */
  uint32 odx;   /* size of output row in no of pixels */
  uint32 idx;    /* size of input row in no pixels */
  uint32 ody;   /* size of output coloumn in no of pixels */
  uint32 idy;    /* size of output coloumn in no of pixels */
  uint32 resizeFactorX;  /* sampling facotr in in X dirction */
  uint32 resizeFactorY;  /* sampling factor in Y direction */
  register uint32 row, col, i, j; /* index variables */
  register uint32 cr_offset;      /* offset from cb_ptr to get the right cr */
  uint8 cb, cr, luma;    /* YCbCr variables */
  uint8 *cb_ptr;         /* start of cb array in the i/p buffer */
  register uint16 out;            /* the o/p RGB variable */
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 r;
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      Initialize the conversion table to RGB 565 or 444
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
      --------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
        initialize the conversion with RGB444 table
      --------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    default:
      return( IPL_FAILURE );
      /*--------------------------------------------------------------------
        Lint complains -- ignoring
       -------------------------------------------------------------------*/
      /*NOTREACHED*/
      break;
  }

  /*------------------------------------------------------------------------
      initialize i/p and o/p sizes
  ------------------------------------------------------------------------*/
  odx = o_img_ptr->dx;
  idx = i_img_ptr->dx;
  ody = o_img_ptr->dy;
  idy = i_img_ptr->dy;


  /*------------------------------------------------------------------------
      calculate the cb plane pointer
  ------------------------------------------------------------------------*/
  cb_ptr = i_img_ptr->imgPtr + ( i_img_ptr->dx * i_img_ptr->dy );

  /*------------------------------------------------------------------------
      calculate the offset from cb array to get the corresponding cr val
      ignoring lint error 703
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy ) >> 2 ; //lint !e703

  /*------------------------------------------------------------------------
      Calculate X and Y resize factors
  ------------------------------------------------------------------------*/
  resizeFactorX =
    ( idx * 128 + ( odx >> 1 ) ) / odx; //lint !e703
  resizeFactorY =
      ( idy * 128 + (ody >> 1)) / ody; //lint !e703



  /*------------------------------------------------------------------------
       We are creating a one to one mapping b/w o/p and iput the mapped
       pixels are determined by resize factor. Lets find out who the
       lucky ones are.
                                              ____________________________
       _______________                        |                           |
       |              |                       |                           |
       |     O/P      |  (mapp sampled pixels |          i/P              |
       |              |  <------------------->|                           |
       |              |                       |                           |
       |              |                       |                           |
       |______________|                       |                           |
                                              |                           |
                                              |___________________________|

  ------------------------------------------------------------------------*/
  for ( row=0; row < o_img_ptr->dy; row++ )
  {

    /*----------------------------------------------------------------------
          i will hold the row we will sample for this particular O/P row
          row]
                                            _______________________________
                                            |                              |
          ____________________              |                              |
          |                   |             |                              |
          |      O/P          |             |                              |
          |___________________| sampled row |______________________________|
          |____'row'th row____|<----------->|_______ row i in O/P__________|
          |                   |             |                              |
          |___________________|             |                              |
                                            |                              |
                                            |______________________________|

             The row no 'row'will corresponds to samples from row no i
            after the following calculation
    -----------------------------------------------------------------------*/
    i = (uint16)(( (uint32)( (row * resizeFactorY) << 9) + 0x8000L) >> 16); //lint !e703

    for ( col=0; col < o_img_ptr->dx; col++ )
    {

      /*--------------------------------------------------------------------
            j calculation will pin point the exact colomn from where we
            will sample this pixel
      --------------------------------------------------------------------*/
      j = (uint16) (((uint32) ( (col * resizeFactorX) <<9 ) + 0x8000L)>>16); //lint !e703


      /*--------------------------------------------------------------------
                          Pick and store Cb for even cols
      --------------------------------------------------------------------*/
      if( ! (j & 0x1) )
      {

        cb = *(uint8*) (cb_ptr + (
             (  ( ( i >> 1 ) * ( idx >> 1) ) //lint !e703
                + ( j >> 1 ) ) ) ); //lint !e703

        luma =
          *((uint8*) (in_img_buf + (i* idx + j)));

        cr = *(uint8*) (cb_ptr + cr_offset + (
             (  ( ( i >> 1 ) * ( idx >> 1) ) //lint !e703
                + ( j >> 1 ) ) ) ); //lint !e703

        /*------------------------------------------------------------------
            Convert this pixel into RGB
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);

      }

      /*--------------------------------------------------------------------
                    pick store cr of odd coloumns
      --------------------------------------------------------------------*/
      else
      {
        cb = *( (uint8*) (cb_ptr + ( ( ( i >> 1)
                          * ( idx >> 1 ) ) + //lint !e703
            ( ( j ) >> 1 )  ) ) ); //lint !e703

        cr = *( (uint8*) (cb_ptr + cr_offset
                   + ( ( ( i >> 1 ) * ( idx  >> 1) ) + //lint !e703
                    (  j  >> 1 )  ) ) ); //lint !e703
        luma = *((uint8*)(i_img_ptr->imgPtr + (i * idx + j) ) );

        /*------------------------------------------------------------------
            Convert this pixel into RGB
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);

      }

      /*--------------------------------------------------------------------
          Store the RGB coded pixel in the o/p buffer
      --------------------------------------------------------------------*/
      *(uint16 *) (  out_img_buf + ( (row * odx ) + col) ) = out;


    } /* End of row for_loop */

  } /* End of Image for_loop */

  return( IPL_SUCCESS );

} /* end of finction ipl2_downsize_ycbcr420ToRGB */



/*===========================================================================

FUNCTION IPL2_DOWNSIZE_YCBCR420LPTORGB()

DESCRIPTION
    Function which handles the downsizing of an existing YUV420 encoded
    image to an arbitrarily smaller sized RGB flavour.

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image

ARGUMENTS OUT
    o_img_ptr - o/p image


RETURN VALUE
  returns success or failure

SIDE EFFECTS
None

===========================================================================*/
static ipl_status_type ipl2_downsize_ycbcr420lpToRGB
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
)
{
  register uint16 *out_img_buf = (uint16 *) o_img_ptr->imgPtr;  /* O/P image buffer */
  uint32 odx;   /* size of output row in no of pixels */
  uint32 idx;    /* size of input row in no pixels */
  uint32 ody;   /* size of output coloumn in no of pixels */
  uint32 idy;    /* size of output coloumn in no of pixels */
  uint32 resizeFactorX;  /* sampling facotr in in X dirction */
  uint32 resizeFactorY;  /* sampling factor in Y direction */
  register uint32 row, col, i, j; /* index variables */
  uint8 cb, cr, luma;    /* YCbCr variables */
  register uint16 out;            /* the o/p RGB variable */
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 r;
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      Initialize the conversion table to RGB 565 or 444
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
      --------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
        initialize the conversion with RGB444 table
      --------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    default:
      return( IPL_FAILURE );
      /*--------------------------------------------------------------------
        Lint complains -- ignoring
       -------------------------------------------------------------------*/
      /*NOTREACHED*/
      break;
  }

  /*------------------------------------------------------------------------
      initialize i/p and o/p sizes
  ------------------------------------------------------------------------*/
  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;
  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;


  /*------------------------------------------------------------------------
      Calculate X and Y resize factors
  ------------------------------------------------------------------------*/
  resizeFactorX =
    ( idx * 128 + ( odx >> 1 ) ) / odx; //lint !e703
  resizeFactorY =
      ( idy * 128 + (ody >> 1)) / ody; //lint !e703



  /*------------------------------------------------------------------------
       We are creating a one to one mapping b/w o/p and iput the mapped
       pixels are determined by resize factor. Lets find out who the
       lucky ones are.
                                              ____________________________
       _______________                        |                           |
       |              |                       |                           |
       |     O/P      |  (mapp sampled pixels |          i/P              |
       |              |  <------------------->|                           |
       |              |                       |                           |
       |              |                       |                           |
       |______________|                       |                           |
                                              |                           |
                                              |___________________________|

  ------------------------------------------------------------------------*/
  for ( row=0; row < o_img_ptr->dy; row++ )
  {

    /*----------------------------------------------------------------------
          i will hold the row we will sample for this particular O/P row
          row]
                                            _______________________________
                                            |                              |
          ____________________              |                              |
          |                   |             |                              |
          |      O/P          |             |                              |
          |___________________| sampled row |______________________________|
          |____'row'th row____|<----------->|_______ row i in O/P__________|
          |                   |             |                              |
          |___________________|             |                              |
                                            |                              |
                                            |______________________________|

             The row no 'row'will corresponds to samples from row no i
            after the following calculation
    -----------------------------------------------------------------------*/
    i = (uint16)(( (uint32)( (row * resizeFactorY) << 9) + 0x8000L) >> 16); 

    for ( col=0; col < o_img_ptr->dx; col++ )
    {

      /*--------------------------------------------------------------------
            j calculation will pin point the exact colomn from where we
            will sample this pixel
      --------------------------------------------------------------------*/
      j = (uint16) (((uint32) ( (col * resizeFactorX) <<9 ) + 0x8000L)>>16); 


      /*--------------------------------------------------------------------
                          Pick and store Cb for even cols
      --------------------------------------------------------------------*/
      if( ! (j & 0x1) )
      {
        cb = *(uint8*)(i_img_ptr->clrPtr + (i/2*idx+j));
        cr = *(uint8*)(i_img_ptr->clrPtr + (i/2*idx+j+1));
        luma = *((uint8*)(i_img_ptr->imgPtr + (i*idx+j)));

        /*------------------------------------------------------------------
            Convert this pixel into RGB
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);

      }

      /*--------------------------------------------------------------------
                    pick store cr of odd coloumns
      --------------------------------------------------------------------*/
      else
      {
        cr = *(uint8*)(i_img_ptr->clrPtr + (i/2*idx+j));
        cb = *(uint8*)(i_img_ptr->clrPtr + (i/2*idx+j+1));
        luma = *((uint8*)(i_img_ptr->imgPtr + (i*idx+j)));

        /*------------------------------------------------------------------
            Convert this pixel into RGB
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);

      }

      /*--------------------------------------------------------------------
          Store the RGB coded pixel in the o/p buffer
      --------------------------------------------------------------------*/
      *(uint16 *) (  out_img_buf + ( (row * odx ) + col) ) = out;


    } /* End of row for_loop */

  } /* End of Image for_loop */

  return( IPL_SUCCESS );

} /* end of finction ipl2_downsize_ycbcr420ToRGB */



/*===========================================================================

FUNCTION IPL2_DOWNSIZE_YCBCR420LPTORGB()

DESCRIPTION
    Function which handles the downsizing of an existing YUV420 encoded
    image to an arbitrarily smaller sized RGB flavour.

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image

ARGUMENTS OUT
    o_img_ptr - o/p image


RETURN VALUE
  returns success or failure

SIDE EFFECTS
None

===========================================================================*/
static ipl_status_type ipl2_downsize_ycrcb420lpToRGB
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
)
{
  register uint16 *out_img_buf = (uint16 *) o_img_ptr->imgPtr;  /* O/P image buffer */
  uint32 odx;   /* size of output row in no of pixels */
  uint32 idx;    /* size of input row in no pixels */
  uint32 ody;   /* size of output coloumn in no of pixels */
  uint32 idy;    /* size of output coloumn in no of pixels */
  uint32 resizeFactorX;  /* sampling facotr in in X dirction */
  uint32 resizeFactorY;  /* sampling factor in Y direction */
  register uint32 row, col, i, j; /* index variables */
  uint8 cb, cr, luma;    /* YCbCr variables */
  register uint16 out;            /* the o/p RGB variable */
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  register int32 r;
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      Initialize the conversion table to RGB 565 or 444
  ------------------------------------------------------------------------*/
  switch ( o_img_ptr->cFormat )
  {
    case IPL_RGB565:
      /*--------------------------------------------------------------------
            initialize the conversion with RGB 565 packing table
      --------------------------------------------------------------------*/
      rTable = &(ipl2_r5xx[0]);
      gTable = &(ipl2_gx6x[0] );
      bTable = &(ipl2_bxx5[0] );
      break;

    case IPL_RGB444 :
      /*--------------------------------------------------------------------
        initialize the conversion with RGB444 table
      --------------------------------------------------------------------*/
      rTable = &(ipl2_r444[0]);
      gTable = &(ipl2_g444[0] );
      bTable = &(ipl2_b444[0] );
      break;

    default:
      return( IPL_FAILURE );
      /*--------------------------------------------------------------------
        Lint complains -- ignoring
       -------------------------------------------------------------------*/
      /*NOTREACHED*/
      break;
  }

  /*------------------------------------------------------------------------
      initialize i/p and o/p sizes
  ------------------------------------------------------------------------*/
  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;
  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;


  /*------------------------------------------------------------------------
      calculate the cb plane pointer
  ------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      Calculate X and Y resize factors
  ------------------------------------------------------------------------*/
  resizeFactorX =
    ( idx * 128 + ( odx >> 1 ) ) / odx; //lint !e703
  resizeFactorY =
      ( idy * 128 + (ody >> 1)) / ody; //lint !e703



  /*------------------------------------------------------------------------
       We are creating a one to one mapping b/w o/p and iput the mapped
       pixels are determined by resize factor. Lets find out who the
       lucky ones are.
                                              ____________________________
       _______________                        |                           |
       |              |                       |                           |
       |     O/P      |  (mapp sampled pixels |          i/P              |
       |              |  <------------------->|                           |
       |              |                       |                           |
       |              |                       |                           |
       |______________|                       |                           |
                                              |                           |
                                              |___________________________|

  ------------------------------------------------------------------------*/
  for ( row=0; row < o_img_ptr->dy; row++ )
  {

    /*----------------------------------------------------------------------
          i will hold the row we will sample for this particular O/P row
          row]
                                            _______________________________
                                            |                              |
          ____________________              |                              |
          |                   |             |                              |
          |      O/P          |             |                              |
          |___________________| sampled row |______________________________|
          |____'row'th row____|<----------->|_______ row i in O/P__________|
          |                   |             |                              |
          |___________________|             |                              |
                                            |                              |
                                            |______________________________|

             The row no 'row'will corresponds to samples from row no i
            after the following calculation
    -----------------------------------------------------------------------*/
    i = (uint16)(( (uint32)( (row * resizeFactorY) << 9) + 0x8000L) >> 16); 

    for ( col=0; col < o_img_ptr->dx; col++ )
    {
      /*--------------------------------------------------------------------
            j calculation will pin point the exact colomn from where we
            will sample this pixel
      --------------------------------------------------------------------*/
      j = (uint16) (((uint32) ( (col * resizeFactorX) <<9 ) + 0x8000L)>>16); 

      //printf("for output (%d,%d), we map to input (%d,%d)\n", col, row, j, i);

      /*--------------------------------------------------------------------
                          Pick and store Cb for even cols
      --------------------------------------------------------------------*/
      if( ! (j & 0x1) )
      {
        cr = *(uint8*)(i_img_ptr->clrPtr + (i/2*idx+j));
        cb = *(uint8*)(i_img_ptr->clrPtr + (i/2*idx+j+1));
        luma = *((uint8*)(i_img_ptr->imgPtr + (i*idx+j)));

        /*------------------------------------------------------------------
            Convert this pixel into RGB
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);

      }

      /*--------------------------------------------------------------------
                    pick store cr of odd coloumns
      --------------------------------------------------------------------*/
      else
      {
        cb = *(uint8*)(i_img_ptr->clrPtr + (i/2*idx+j));
        cr = *(uint8*)(i_img_ptr->clrPtr + (i/2*idx+j-1));
        luma = *((uint8*)(i_img_ptr->imgPtr + (i*idx+j)));

        /*------------------------------------------------------------------
            Convert this pixel into RGB
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);

      }

      /*--------------------------------------------------------------------
          Store the RGB coded pixel in the o/p buffer
      --------------------------------------------------------------------*/
      *(uint16 *) (  out_img_buf + ( (row * odx ) + col) ) = out;


    } /* End of row for_loop */

  } /* End of Image for_loop */

  return( IPL_SUCCESS );

} /* end of finction ipl2_downsize_ycbcr420ToRGB */






/*===========================================================================

FUNCTION IPL2_DOWNSIZE_YCBCR420TORGB666()

DESCRIPTION
    Function which handles the downsizing of an existing YUV420 encoded
    image to an arbitrarily smaller sized RGB666 image.

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image

ARGUMENTS OUT
    o_img_ptr - o/p image


RETURN VALUE
    Status - Success or failure

SIDE EFFECTS
None

===========================================================================*/
static ipl_status_type ipl2_downsize_ycbcr420ToRGB666
(
  ipl_image_type* i_img_ptr,     /* Points to the input image           */
  ipl_image_type* o_img_ptr      /* Points to the output image          */
)
{
  register uint8 *in_img_buf = i_img_ptr->imgPtr;  /* O/P image buffer */
  register uint32 *out_img_buf =
                  (uint32 *) o_img_ptr->imgPtr;  /* O/P image buffer */
  uint32 odx;   /* size of output row in no of pixels */
  uint32 idx;    /* size of input row in no pixels */
  uint32 ody;   /* size of output coloumn in no of pixels */
  uint32 idy;    /* size of output coloumn in no of pixels */
  uint32 resizeFactorX;  /* sampling facotr in in X dirction */
  uint32 resizeFactorY;  /* sampling factor in Y direction */
  register uint32 row, col, i, j; /* index variables */
  register uint32 cr_offset;      /* offset from cb_ptr to get the right cr */
  uint8 cb, cr, luma;    /* YCbCr variables */
  uint8 *cb_ptr;         /* start of cb array in the i/p buffer */
  register uint32 out;            /* the o/p RGB variable */
  register const uint32 *rTable;
  register const uint32 *gTable;
  register const uint32 *bTable;
  register int32 r;
/*------------------------------------------------------------------------*/


  /*------------------------------------------------------------------------
        initialize the conversion with RGB 565 packing table
  ------------------------------------------------------------------------*/
  rTable = &(ipl2_r666[0]);
  gTable = &(ipl2_g666[0] );
  bTable = &(ipl2_b666[0] );

  /*------------------------------------------------------------------------
      initialize i/p and o/p sizes
  ------------------------------------------------------------------------*/
  odx = o_img_ptr->dx;
  idx = i_img_ptr->dx;
  ody = o_img_ptr->dy;
  idy = i_img_ptr->dy;


  /*------------------------------------------------------------------------
      calculate the cb plane pointer
  ------------------------------------------------------------------------*/
  cb_ptr = i_img_ptr->imgPtr + ( i_img_ptr->dx * i_img_ptr->dy );

  /*------------------------------------------------------------------------
      calculate the offset from cb array to get the corresponding cr val
      ignoring lint error 703
  ------------------------------------------------------------------------*/
  cr_offset = ( i_img_ptr->dx * i_img_ptr->dy ) >> 2 ; //lint !e703

  /*------------------------------------------------------------------------
      Calculate X and Y resize factors
  ------------------------------------------------------------------------*/
  resizeFactorX =
    ( idx * 128 + ( odx >> 1 ) ) / odx; //lint !e703
  resizeFactorY =
      ( idy * 128 + (ody >> 1)) / ody; //lint !e703



  /*------------------------------------------------------------------------
      We are creating a one to one mapping b/w o/p and iput the mapped
      pixels are determined by resize factor. Lets find out who the
      lucky ones are.
                                             ____________________________
      _______________                        |                           |
      |              |                       |                           |
      |     O/P      |  (mapp sampled pixels |          i/P              |
      |              |  <------------------->|                           |
      |              |                       |                           |
      |              |                       |                           |
      |______________|                       |                           |
                                             |                           |
                                             |___________________________|

  ------------------------------------------------------------------------*/
  for ( row=0; row < o_img_ptr->dy; row++ )
  {

    i = (uint16)(( (uint32)( (row * resizeFactorY) << 9) + 0x8000L) >> 16); //lint !e703

    for ( col=0; col < o_img_ptr->dx; col++ )
    {
      /*--------------------------------------------------------------------
            j calculation will pin point the exact colomn from where we
            will sample this pixel
      --------------------------------------------------------------------*/

      j = (uint16) (((uint32) ( (col * resizeFactorX) <<9 ) + 0x8000L)>>16); //lint !e703


      /*--------------------------------------------------------------------
                          Pick and store Cb for even cols
      --------------------------------------------------------------------*/
      if( ! (j & 0x1) )
      {


        cb = *(uint8*) (cb_ptr + (
             (  ( ( i >> 1 ) * ( idx >> 1) ) //lint !e703
                + ( j >> 1 ) ) ) ); //lint !e703

        luma =
          *((uint8*) (i_img_ptr->imgPtr + (i* idx + j)));

        cr = *(uint8*) (cb_ptr + cr_offset + (
             (  ( ( i >> 1 ) * ( idx >> 1) ) //lint !e703
                + ( j >> 1 ) ) ) ); //lint !e703

        /*------------------------------------------------------------------
            Convert this pixel into RGB
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);

      }

      /*--------------------------------------------------------------------
                    pick store cr of odd coloumns
      --------------------------------------------------------------------*/
      else
      {
        cb = *( (uint8*) (cb_ptr + ( ( ( i >> 1)
                          * ( idx >> 1 ) ) + //lint !e703
            ( ( j ) >> 1 )  ) ) ); //lint !e703

        cr = *( (uint8*) (cb_ptr + cr_offset
                   + ( ( ( i >> 1 ) * ( idx  >> 1) ) + //lint !e703
                    (  j  >> 1 )  ) ) ); //lint !e703
        luma = *((uint8*)(in_img_buf + (i * idx + j) ) );

        /*------------------------------------------------------------------
            Convert this pixel into RGB
        ------------------------------------------------------------------*/
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma, cr, cb, r, out,
                                         rTable, gTable, bTable);

      }

      /*--------------------------------------------------------------------
          Store the RGB coded pixel in the o/p buffer
      --------------------------------------------------------------------*/
      *(uint32 *) (  out_img_buf + ( (row * odx ) + col) ) = out;


    } /* End of row for_loop */

  } /* End of Image for_loop */

  return( IPL_SUCCESS );

} /* end of function ipl2_downsize_ycbcr420ToRGB666 */

/*===========================================================================

FUNCTION IPL2_DOWNSIZE_RGB565ToYCbCr()

DESCRIPTION
    Function which handles the downsizing of an existing RGB565 encoded
    image to an arbitrarily smaller sized YCbCr image. This takes crop as
    a paramter so that application can render the image on a larger screen
    in one shot.

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image

ARGUMENTS OUT
    o_img_ptr - o/p image


RETURN VALUE
    Status - Success or failure

SIDE EFFECTS
None

===========================================================================*/
static ipl_status_type ipl2_downsize_RGB565ToYCbCr
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl2_image_downsize_param_type* i_param
)
{
  int32 odx, idx, ody, idy;
  register uint8 *out_img_buf = o_img_ptr->imgPtr;  /* O/P image buffer */
  register uint16 out;
  uint16 row,col,i,j;
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint8 r1,g1,b1;
  uint32 destInc;

  odx = o_img_ptr->dx;
  idx = i_img_ptr->dx;
  ody = o_img_ptr->dy;
  idy = i_img_ptr->dy;
/*------------------------------------------------------------------------*/

  /*------------------------------------------------------------------------
      Calculate X and Y resize factors
  ------------------------------------------------------------------------*/
  resizeFactorX =
    ( idx * 128 + ( odx >> 1 ) ) / odx; //lint !e703
  resizeFactorY =
      ( idy * 128 + (ody >> 1)) / ody; //lint !e703


  destInc =  ( (o_img_ptr->dx - i_param->crop.dx) * 2 );

  out_img_buf += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  // memcpy(out_img_buf, i_img_ptr->imgPtr, 100000);


  /*------------------------------------------------------------------------
            Resize RTGB565
  ------------------------------------------------------------------------*/
  for (row=0; row < i_param->crop.dy ;row++)
  {
    i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);
    for (col=0; col < i_param->crop.dx; col+=2)
    {
      /* 1st pixel */
      j = (uint16) (((uint32)(col * resizeFactorX << 9) + 0x8000L) >> 16);
      out = *((uint16*)(i_img_ptr->imgPtr+(i*idx+j)*2));
      r1 = out >> 8 ;
      g1 = (out >> 3) & 0xff ;
      b1 = out & 0xff;
      *out_img_buf++ = 
        ipl2_rgb565ToCbR[r1] + ipl2_rgb565ToCbG[g1] + ipl2_rgb565ToCbB[b1];
      *out_img_buf++ = 
        ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] + ipl2_rgb565ToYB[b1];

      /* 2nd pixel */
      j = (uint16) (((uint32)((col+1)*resizeFactorX<<9) + 0x8000L)>>16);
      out = *((uint16*)(i_img_ptr->imgPtr+(i*idx+j)*2));
      r1 = out >> 8;
      g1 = (out >> 3) & 0xff;
      b1 = out & 0xff;
      *out_img_buf++ = 
        ipl2_rgb565ToCrR[r1] + ipl2_rgb565ToCrG[g1] + ipl2_rgb565ToCrB[b1];
      *out_img_buf++ = 
        ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] + ipl2_rgb565ToYB[b1];

    } /* End of row for_loop */

    /*----------------------------------------------------------------------
      Increment the o/p pointer
    ----------------------------------------------------------------------*/
    out_img_buf += destInc;

  } /* End of Image for_loop */

  return IPL_SUCCESS;

} /* End of function ipl2_downsize_RGB565ToYCbCr()  */

/*===========================================================================

FUNCTION IPL2_DOWNSIZE_RGB565ToRGB565()

DESCRIPTION
    Function which handles the downsizing of an existing RGB565 encoded
    image to an arbitrarily smaller sized RGB565 image. This takes crop as
    a paramter so that application can render the image on a larger screen
    in one shot.

DEPENDENCIES
None

ARGUMENTS IN
    i_img_ptr - i/p image

ARGUMENTS OUT
    o_img_ptr - o/p image


RETURN VALUE
    Status - Success or failure

SIDE EFFECTS
None

===========================================================================*/
static ipl_status_type ipl2_downsize_RGB565ToRGB565
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl2_image_downsize_param_type* i_param
)
{
  int32 odx, idx, ody, idy;
  /*
  register uint8 *in_img_buf = i_img_ptr->imgPtr;  *//* O/P image buffer */
  register uint16 *out_img_buf = (uint16 *)
                                  o_img_ptr->imgPtr;  /* O/P image buffer */
  register uint16 out;
  uint16 row,col,i,j;
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  /*
  uint8 cb, cr, lumaa1, lumaa2;    
  unsigned char r2,g2,b2;
  */
  uint32 destInc;
/*------------------------------------------------------------------------*/

  odx = o_img_ptr->dx;
  idx = i_img_ptr->dx;
  ody = o_img_ptr->dy;
  idy = i_img_ptr->dy;

  /*------------------------------------------------------------------------
      Calculate X and Y resize factors
  ------------------------------------------------------------------------*/
  resizeFactorX =
    ( idx * 128 + ( odx >> 1 ) ) / odx; //lint !e703
  resizeFactorY =
      ( idy * 128 + (ody >> 1)) / ody; //lint !e703


  destInc =  ( (o_img_ptr->dx - i_param->crop.dx) );

  out_img_buf += (i_param->crop.x + o_img_ptr->dx * i_param->crop.y) * 2;

  /*------------------------------------------------------------------------
            Resize RTGB565
  ------------------------------------------------------------------------*/
  for (row=0; row < i_param->crop.dy ;row++)
  {

    i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);

    for (col=0; col < i_param->crop.dx; col++)
    {

    /*----------------------------------------------------------------------
            Get the  RGB value
    ----------------------------------------------------------------------*/
      j = (uint16)(((uint32)(col * resizeFactorX<<9) + 0x8000L)>>16);

      out = *((uint16*)(i_img_ptr->imgPtr +
                        ( i * idx + j )*2));

      *out_img_buf++ = out;

    } /* End of row for_loop */

    /*----------------------------------------------------------------------
      Increment the o/p pointer
    ----------------------------------------------------------------------*/
    out_img_buf += destInc;

  } /* End of Image for_loop */

  return IPL_SUCCESS;

} /* end of function ipl2_downsize_RGB565ToRGB565 */



/*lint -restore */



