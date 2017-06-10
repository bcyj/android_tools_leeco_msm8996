/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    F I L E

DESCRIPTION
  This file contains the implementation of the IPL APIs.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/


/*===========================================================================
                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_attic.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
08/09/05   kwa     Coverity - fixed RESOURCE_LEAK in function
                   sip_downsize_rgb565
                   Coverity - fixed RESOURCE_LEAK in function
                   sip_downsize_ycbcr
10/25/04   bf      Optimzed bad_pixel_correct, mode old code into this file
10/05/04   mz      Updated function APIs and comments. Corrected tab spacings 
                   and line widths.
09/01/04   bf      Created file. See ipl.h for previous history.
===========================================================================*/

/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include "ipl_types.h"
#include "ipl_helper.h"


// Turn off some lint warnings
/*lint -save -e504, all shifts are okay */
/*lint -save -e573, all shifts are okay */
/*lint -save -e704, all shifts are okay */
/*lint -save -e703, all shifts are okay */
/*lint -save -e715, okay that bip_ptr not used */
/*lint -save -e508, extra extern is okay */
/*lint -save -e534, let me call printf in piece, god */

/*lint -save -e506, constant value boolean is totally okay */
/*lint -save -e774, constant value boolean is totally okay */

/*lint -save -e701, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e702, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e713, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e732, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e737, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e734, loss of precision,signed to unsigned,usigned to signed OK*/

/*lint -save -e818, lets not worry about const optimization right now */
/*lint -save -e834, this warning is just dumb */
/*lint -save -e826, we should look into this someday */

/*===========================================================================
                        FUNCTION DECLARATIONS
===========================================================================*/


static ipl_status_type ipl_downsize_fast_rgb565
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  uint32 m,n,mv,nv;
  uint16 out;
  uint16 row,col,i,j;
  uint8 failed = 0;
  uint32 resizeFactorX;
  uint32 resizeFactorY;

  if (!o_img_ptr || !o_img_ptr->imgPtr || !i_img_ptr || !i_img_ptr->imgPtr)
    return IPL_FAILURE;

  m = o_img_ptr->dx;
  n = i_img_ptr->dx;
  mv = o_img_ptr->dy;
  nv = i_img_ptr->dy;

  /*
  ** Q7 resize Factors
  */
  resizeFactorX = (n*128 +(m>>1))/m;
  resizeFactorY = (nv*128 + (mv>>1))/mv;

  {
    if (o_img_ptr->cFormat == IPL_RGB565)
    {
      /*
      ** Resize from RGB565 to RGB565
      */
      for (row=0;row<o_img_ptr->dy;row++)
      {
        i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);
        for (col=0;col<o_img_ptr->dx;col++)
        {
          j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000L)>>16);
          out = *((uint16*)(i_img_ptr->imgPtr + (i*n+j)*2));
          *(uint16*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= out;
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } else {
      /*
      ** Resize from RGB565 to YCbCr
      */
      failed = 1;
    }
  }

  if (failed)
  {
    return IPL_FAILURE;
  }
  return IPL_SUCCESS;
} /* End ipl_downsize_fast_rgb565() */


static ipl_status_type ipl_downsize_fast_ycbcr
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  uint32 m,n,mv,nv;
  uint16 out;
  uint16 row,col,i,j;
  uint8 r,g,b;
  uint8 cb,cr,luma1,luma2;
  int32 cdb,cdr,lumad1;
  int32 rc,gc,bc;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int16 ycbcr2rgb_convert[6] = {8, 25803, -3071, -7672,30399,12};
  uint8 failed = 0;
  uint32 resizeFactorX;
  uint32 resizeFactorY;


  if (!o_img_ptr || !o_img_ptr->imgPtr || !i_img_ptr || !i_img_ptr->imgPtr)
    return IPL_FAILURE;

  m = o_img_ptr->dx;
  n = i_img_ptr->dx;
  mv = o_img_ptr->dy;
  nv = i_img_ptr->dy;

  /*
  ** Q7 resize Factors
  */
  resizeFactorX = (n*128 +(m>>1))/m;
  resizeFactorY = (nv*128 + (mv>>1))/mv;

  {
    /* Input is YCbCr */
    if (o_img_ptr->cFormat == IPL_RGB565)
    {
      /*
      ** Resize from YCbCr to RGB565
      */
      for (row=0;row<o_img_ptr->dy;row++)
      {
        i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);
        for (col=0;col<o_img_ptr->dx;col++)
        {
          j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000L)>>16);
          if (!(j%2))
          {
            /* Cb Input Pixel */
            cb = *(uint8*)(i_img_ptr->imgPtr + (i*n+j)*2);
            luma1 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2+1));
            cr = *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)*2);
            luma2 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j+1)*2+1));
            rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                  ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
            gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                  ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
            bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                  ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
            cdb = luma1 + (rc>>16);
            cdr = luma1 + (gc>>16);
            lumad1 = luma1 + (bc>>16);
            r = (uint8)CLIPIT(cdb);
            g = (uint8)CLIPIT(cdr);
            b = (uint8)CLIPIT(lumad1);
          } else {
            /* Cr Input Pixel */
            cr = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2));
            luma2 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2+1));
            cb = *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)*2);
            luma1 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j-1)*2+1));
            rc = (ycbcr2rgb_convert[0]*(cb-128) + 
                  ycbcr2rgb_convert[1]*(cr-128))*4+0x8000;
            gc = (ycbcr2rgb_convert[2]*(cb-128) + 
                  ycbcr2rgb_convert[3]*(cr-128))*4+0x8000;
            bc = (ycbcr2rgb_convert[4]*(cb-128) + 
                  ycbcr2rgb_convert[5]*(cr-128))*4+0x8000;
            cdb = luma2 + (rc>>16);
            cdr = luma2 + (gc>>16);
            lumad1 = luma2 + (bc>>16);
            r= (uint8)CLIPIT(cdb);
            g= (uint8)CLIPIT(cdr);
            b= (uint8)CLIPIT(lumad1);
          }
          out = pack_rgb565(r,g,b);
          *(uint16*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= out;
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } else if (o_img_ptr->cFormat == IPL_YCbCr)
    {
      /*
      ** Resize from YCbCr to YCbCr
      */
      for (row=0;row<o_img_ptr->dy;row++)
      {
        i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000L)>>16);
        for (col=0;col<o_img_ptr->dx;col++)
        {
          j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000L)>>16);
          if (((col%2)+(j%2))%2)
          {
            j = j+1;
          }
          if (!(j%2))
          {
            /* Cb Input Pixel */
            cb = *(uint8*)(i_img_ptr->imgPtr + (i*n+j)*2);
            luma1 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2+1));
            *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cb;
            *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)luma1;
          } else {
            /* Cr Input Pixel */
            cr = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2));
            luma2 = *((uint8*)(i_img_ptr->imgPtr + (i*n+j)*2+1));
            *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2))= (uint8)cr;
            *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1))= (uint8)luma2;
          }
        } /* End of row for_loop */
      } /* End of Image for_loop */
    } else {
      failed = 1;
    }
  }

  if (failed)
  {
    return IPL_FAILURE;
  }
  return IPL_SUCCESS;
} /* End ipl_downsize_fast_ycbcr() */


static ipl_status_type ipl_downsize_fast_bggr
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  uint32 m,n,mv,nv;
  uint16 row,col,i,j;
  uint8 r,g,b;
  uint8 r2,g2,b2;
  int32 cdb,cdr,lumad1,lumad2;
  int32 rc,gc,bc;
  uint8 failed = 0;
  int32 clr_correct[9];
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint32 redAverage=0;
  uint32 greenAverage=0;
  uint32 blueAverage=0;
  uint32 mean = 0;
  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  int16 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};


  if (!o_img_ptr || !o_img_ptr->imgPtr || !i_img_ptr || !i_img_ptr->imgPtr)
    return IPL_FAILURE;

  m = o_img_ptr->dx;
  n = i_img_ptr->dx;
  mv = o_img_ptr->dy;
  nv = i_img_ptr->dy;
  /*
  ** Q7 resize Factors
  */
  resizeFactorX = (n*128 +(m>>1))/m;
  resizeFactorY = (nv*128 + (mv>>1))/mv;

  {
    /*
    ** Input is Bayer BGGR
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2)) = (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1)) = 
                (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2)) = (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2+1)) = 
                (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */
          /*
          ** Do some Image Processing
          */
          /*
          ** Fuse Color Correction and multiplicative gains in one
          */
          /*
          ** *colorCorrection; * a11 a12 a13 a21 a22 a23 a31 a32 a33
          */
          for (i=0;i<3;i++)
          {
            clr_correct[3*i] = (int32)bip_ptr->colorCorrection[3*i];
            clr_correct[3*i+1] = (int32)bip_ptr->colorCorrection[3*i+1];
            clr_correct[3*i+2] = (int32)bip_ptr->colorCorrection[3*i+2];
          }
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (clr_correct[0]*r + clr_correct[1]*g + 
                    clr_correct[2]*b)*8 + 0x8000;
              gc = (clr_correct[3]*r + clr_correct[4]*g + 
                    clr_correct[5]*b)*8 + 0x8000;
              bc = (clr_correct[6]*r + clr_correct[7]*g + 
                    clr_correct[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              redAverage += (uint32) rc;
              greenAverage += (uint32) gc;
              blueAverage += (uint32) bc;
              /* Cr Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = ((int32)clr_correct[0]*r2 + clr_correct[1]*g2 + 
                    clr_correct[2]*b2)*8 + 0x8000;
              gc = ((int32)clr_correct[3]*r2 + clr_correct[4]*g2 + 
                    clr_correct[5]*b2)*8 + 0x8000;
              bc = ((int32)clr_correct[6]*r2 + clr_correct[7]*g2 + 
                    clr_correct[8]*b2)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              /*
              ** *rgbToYcbcr;   by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              redAverage += (uint32) rc;
              greenAverage += (uint32) gc;
              blueAverage += (uint32) bc;
            } /* End of row for_loop */
          } /* End of Image for_loop */
          redAverage = redAverage / (o_img_ptr->dy*o_img_ptr->dx);
          greenAverage = greenAverage / (o_img_ptr->dy*o_img_ptr->dx);
          blueAverage = blueAverage / (o_img_ptr->dy*o_img_ptr->dx);
          mean = (uint32)((redAverage+blueAverage+greenAverage)/3);
          redAverage = (redAverage + ((mean-redAverage)>>2));
          if (redAverage)
          {
            redAverage = (mean*256)/redAverage;
          } else {
            redAverage = 255;
          }
          greenAverage = (greenAverage + ((mean-greenAverage)>>2));
          if (greenAverage)
          {
            greenAverage = (mean*256)/greenAverage;
          } else {
            greenAverage = 255;
          }
          blueAverage = (blueAverage + ((mean-blueAverage)>>2));
          if (blueAverage)
          {
            blueAverage = (mean*256)/blueAverage;
          } else {
            blueAverage = 255;
          }
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if ((j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /* Cb Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (clr_correct[0]*r + clr_correct[1]*g + 
                    clr_correct[2]*b)*8 + 0x8000;
              gc = (clr_correct[3]*r + clr_correct[4]*g + 
                    clr_correct[5]*b)*8 + 0x8000;
              bc = (clr_correct[6]*r + clr_correct[7]*g + 
                    clr_correct[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              rc = (rc*redAverage+128)>>8;
              gc = (gc*greenAverage+128)>>8;
              bc = (bc*blueAverage + 128)>>8;
              lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc +
                        bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
              lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
              lumad1 = CLIPIT(lumad1);
              cdb = (bip_ptr->rgbToYcbcr[5]*rc + bip_ptr->rgbToYcbcr[6]*gc + 
                     bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
              cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2)) = (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1)) = 
                (uint8)lumad1;
              /* Cr Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = ((int32)clr_correct[0]*r2 + clr_correct[1]*g2 + 
                    clr_correct[2]*b2)*8 + 0x8000;
              gc = ((int32)clr_correct[3]*r2 + clr_correct[4]*g2 + 
                    clr_correct[5]*b2)*8 + 0x8000;
              bc = ((int32)clr_correct[6]*r2 + clr_correct[7]*g2 + 
                    clr_correct[8]*b2)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              /*
              ** *rgbToYcbcr;  by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              rc = (rc*redAverage+128)>>8;
              gc = (gc*greenAverage+128)>>8;
              bc = (bc*blueAverage + 128)>>8;
              lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc +
                        bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
              lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
              lumad2 = CLIPIT(lumad2);
              cdr = (bip_ptr->rgbToYcbcr[9]*rc + bip_ptr->rgbToYcbcr[10]*gc + 
                     bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
              cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2)) = (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2+1)) = 
                (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
          /* Cb Path */
        }
      } else {
        failed = 1;
      }
    }
  }

  if (failed)
  {
    return IPL_FAILURE;
  }
  return IPL_SUCCESS;
} /* End ipl_downsize_fast_bggr() */



/* <EJECT> */
static ipl_status_type ipl_downsize_fast_gbrg
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  uint32 m,n,mv,nv;
  uint16 row,col,i,j;
  uint8 r,g,b;
  uint8 r2,g2,b2;
  int32 cdb,cdr,lumad1,lumad2;
  int32 rc,gc,bc;
  uint8 failed = 0;
  int32 clr_correct[9];
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint32 redAverage=0;
  uint32 greenAverage=0;
  uint32 blueAverage=0;
  uint32 mean = 0;
  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  int16 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  if (!o_img_ptr || !o_img_ptr->imgPtr || !i_img_ptr || !i_img_ptr->imgPtr)
    return IPL_FAILURE;

  m = o_img_ptr->dx;
  n = i_img_ptr->dx;
  mv = o_img_ptr->dy;
  nv = i_img_ptr->dy;
  /*
  ** Q7 resize Factors
  */
  resizeFactorX = (n*128 +(m>>1))/m;
  resizeFactorY = (nv*128 + (mv>>1))/mv;

  {
    /*
    ** Input is Bayer GBRG
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2)) = (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1)) = 
                (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2)) = (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2+1)) = 
                (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */
          /*
          ** Do some Image Processing
          */
          /*
          ** Fuse Color Correction and multiplicative gains in one
          */
          /*
          ** *colorCorrection; * a11 a12 a13 a21 a22 a23 a31 a32 a33
          */
          for (i =0;i<3;i++)
          {
            clr_correct[3*i] = (int32)bip_ptr->colorCorrection[3*i];
            clr_correct[3*i+1] = (int32)bip_ptr->colorCorrection[3*i+1];
            clr_correct[3*i+2] = (int32)bip_ptr->colorCorrection[3*i+2];
          }
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (clr_correct[0]*r + clr_correct[1]*g + 
                    clr_correct[2]*b)*8 + 0x8000;
              gc = (clr_correct[3]*r + clr_correct[4]*g + 
                    clr_correct[5]*b)*8 + 0x8000;
              bc = (clr_correct[6]*r + clr_correct[7]*g + 
                    clr_correct[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              redAverage += (uint32) rc;
              greenAverage += (uint32) gc;
              blueAverage += (uint32) bc;
              /* Cr Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = ((int32)clr_correct[0]*r2 + clr_correct[1]*g2 + 
                    clr_correct[2]*b2)*8 + 0x8000;
              gc = ((int32)clr_correct[3]*r2 + clr_correct[4]*g2 + 
                    clr_correct[5]*b2)*8 + 0x8000;
              bc = ((int32)clr_correct[6]*r2 + clr_correct[7]*g2 + 
                    clr_correct[8]*b2)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              /*
              ** *rgbToYcbcr;    by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              redAverage += (uint32) rc;
              greenAverage += (uint32) gc;
              blueAverage += (uint32) bc;
            } /* End of row for_loop */
          } /* End of Image for_loop */
          redAverage = redAverage / (o_img_ptr->dy*o_img_ptr->dx);
          greenAverage = greenAverage / (o_img_ptr->dy*o_img_ptr->dx);
          blueAverage = blueAverage / (o_img_ptr->dy*o_img_ptr->dx);
          mean = (uint32)((redAverage+blueAverage+greenAverage)/3);
          redAverage = (redAverage + ((mean-redAverage)>>2));
          if (redAverage)
          {
            redAverage = (mean*256)/redAverage;
          } else {
            redAverage = 255;
          }
          greenAverage = (greenAverage + ((mean-greenAverage)>>2));
          if (greenAverage)
          {
            greenAverage = (mean*256)/greenAverage;
          } else {
            greenAverage = 255;
          }
          blueAverage = (blueAverage + ((mean-blueAverage)>>2));
          if (blueAverage)
          {
            blueAverage = (mean*256)/blueAverage;
          } else {
            blueAverage = 255;
          }
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if (!(j%2) && (i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /* Cb Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (clr_correct[0]*r + clr_correct[1]*g + 
                    clr_correct[2]*b)*8 + 0x8000;
              gc = (clr_correct[3]*r + clr_correct[4]*g + 
                    clr_correct[5]*b)*8 + 0x8000;
              bc = (clr_correct[6]*r + clr_correct[7]*g + 
                    clr_correct[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */

              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              rc = (rc*redAverage+128)>>8;
              gc = (gc*greenAverage+128)>>8;
              bc = (bc*blueAverage + 128)>>8;
              lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc +
                        bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
              lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
              lumad1 = CLIPIT(lumad1);
              cdb = (bip_ptr->rgbToYcbcr[5]*rc + bip_ptr->rgbToYcbcr[6]*gc + 
                     bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
              cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2)) = (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1)) = 
                (uint8)lumad1;
              /* Cr Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = ((int32)clr_correct[0]*r2 + clr_correct[1]*g2 + 
                    clr_correct[2]*b2)*8 + 0x8000;
              gc = ((int32)clr_correct[3]*r2 + clr_correct[4]*g2 + 
                    clr_correct[5]*b2)*8 + 0x8000;
              bc = ((int32)clr_correct[6]*r2 + clr_correct[7]*g2 + 
                    clr_correct[8]*b2)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              /*
              ** *rgbToYcbcr;    by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              rc = (rc*redAverage+128)>>8;
              gc = (gc*greenAverage+128)>>8;
              bc = (bc*blueAverage + 128)>>8;
              lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc +
                        bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
              lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
              lumad2 = CLIPIT(lumad2);
              cdr = (bip_ptr->rgbToYcbcr[9]*rc + bip_ptr->rgbToYcbcr[10]*gc + 
                     bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
              cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2)) = (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2+1)) = 
                (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
          /* Cb Path */
        }
      } else {
        failed = 1;
      }
    }
  }

  if (failed)
  {
    return IPL_FAILURE;
  }
  return IPL_SUCCESS;
} /* End ipl_downsize_fast_gbrg() */




/* <EJECT> */
static ipl_status_type ipl_downsize_fast_rggb
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  uint32 m,n,mv,nv;
  uint16 row,col,i,j;
  uint8 r,g,b;
  uint8 r2,g2,b2;
  int32 cdb,cdr,lumad1,lumad2;
  int32 rc,gc,bc;
  uint8 failed = 0;
  int32 clr_correct[9];
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint32 redAverage=0;
  uint32 greenAverage=0;
  uint32 blueAverage=0;
  uint32 mean = 0;
  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  int16 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};


  if (!o_img_ptr || !o_img_ptr->imgPtr || !i_img_ptr || !i_img_ptr->imgPtr)
    return IPL_FAILURE;

  m = o_img_ptr->dx;
  n = i_img_ptr->dx;
  mv = o_img_ptr->dy;
  nv = i_img_ptr->dy;
  /*
  ** Q7 resize Factors
  */
  resizeFactorX = (n*128 +(m>>1))/m;
  resizeFactorY = (nv*128 + (mv>>1))/mv;

  {
    /*
    ** Input is Bayer RGGB
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2)) = (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1)) = 
                (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2)) = (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2+1)) = 
                (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */
          /*
          ** Do some Image Processing
          */
          /*
          ** Fuse Color Correction and multiplicative gains in one
          */
          /*
          ** *colorCorrection; * a11 a12 a13 a21 a22 a23 a31 a32 a33
          */
          for (i=0;i<3;i++)
          {
            clr_correct[3*i] = (int32)bip_ptr->colorCorrection[3*i];
            clr_correct[3*i+1] = (int32)bip_ptr->colorCorrection[3*i+1];
            clr_correct[3*i+2] = (int32)bip_ptr->colorCorrection[3*i+2];
          }
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                       *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                         *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                         *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (clr_correct[0]*r + clr_correct[1]*g + 
                    clr_correct[2]*b)*8 + 0x8000;
              gc = (clr_correct[3]*r + clr_correct[4]*g + 
                    clr_correct[5]*b)*8 + 0x8000;
              bc = (clr_correct[6]*r + clr_correct[7]*g + 
                    clr_correct[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              redAverage += (uint32) rc;
              greenAverage += (uint32) gc;
              blueAverage += (uint32) bc;
              /* Cr Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = ((int32)clr_correct[0]*r2 + clr_correct[1]*g2 + 
                    clr_correct[2]*b2)*8 + 0x8000;
              gc = ((int32)clr_correct[3]*r2 + clr_correct[4]*g2 + 
                    clr_correct[5]*b2)*8 + 0x8000;
              bc = ((int32)clr_correct[6]*r2 + clr_correct[7]*g2 + 
                    clr_correct[8]*b2)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc = (rc*bip_ptr->rGain + 0x1000)>>13;
              gc = (gc*bip_ptr->gGain + 0x1000)>>13;
              bc = (bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc = CLIPIT(rc);
              gc = CLIPIT(gc);
              bc = CLIPIT(bc);
              /*
              **  Table Lookup
              */
              /*
              ** *rgbToYcbcr;    by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33
              */
              rc = bip_ptr->gammaTable[rc];
              gc = bip_ptr->gammaTable[gc];
              bc = bip_ptr->gammaTable[bc];
              redAverage += (uint32) rc;
              greenAverage += (uint32) gc;
              blueAverage += (uint32) bc;
            } /* End of row for_loop */
          } /* End of Image for_loop */
          redAverage = redAverage / (o_img_ptr->dy*o_img_ptr->dx);
          greenAverage = greenAverage / (o_img_ptr->dy*o_img_ptr->dx);
          blueAverage = blueAverage / (o_img_ptr->dy*o_img_ptr->dx);
          mean = (uint32)((redAverage+blueAverage+greenAverage)/3);
          redAverage = (redAverage + ((mean-redAverage)>>2));
          if (redAverage)
          {
            redAverage = (mean*256)/redAverage;
          } else {
            redAverage = 255;
          }
          greenAverage = (greenAverage + ((mean-greenAverage)>>2));
          if (greenAverage)
          {
            greenAverage = (mean*256)/greenAverage;
          } else {
            greenAverage = 255;
          }
          blueAverage = (blueAverage + ((mean-blueAverage)>>2));
          if (blueAverage)
          {
            blueAverage = (mean*256)/blueAverage;
          } else {
            blueAverage = 255;
          }
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if (!(j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if ((j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if ((j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /* Cb Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (clr_correct[0]*r + clr_correct[1]*g + 
                    clr_correct[2]*b)*8 + 0x8000;
              gc = (clr_correct[3]*r + clr_correct[4]*g + 
                    clr_correct[5]*b)*8 + 0x8000;
              bc = (clr_correct[6]*r + clr_correct[7]*g + 
                    clr_correct[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=(rc*bip_ptr->rGain + 0x1000)>>13;
              gc=(gc*bip_ptr->gGain + 0x1000)>>13;
              bc=(bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];
              rc=(rc*redAverage+128)>>8;
              gc=(gc*greenAverage+128)>>8;
              bc=(bc*blueAverage + 128)>>8;
              lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc +
                        bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
              lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
              lumad1 = CLIPIT(lumad1);
              cdb = (bip_ptr->rgbToYcbcr[5]*rc + bip_ptr->rgbToYcbcr[6]*gc + 
                     bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
              cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2)) = (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1)) = 
                (uint8)lumad1;
              /* Cr Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = ((int32)clr_correct[0]*r2 + clr_correct[1]*g2 + 
                    clr_correct[2]*b2)*8 + 0x8000;
              gc = ((int32)clr_correct[3]*r2 + clr_correct[4]*g2 + 
                    clr_correct[5]*b2)*8 + 0x8000;
              bc = ((int32)clr_correct[6]*r2 + clr_correct[7]*g2 + 
                    clr_correct[8]*b2)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=(rc*bip_ptr->rGain + 0x1000)>>13;
              gc=(gc*bip_ptr->gGain + 0x1000)>>13;
              bc=(bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);
              /*
              **  Table Lookup
              */
              /*
              ** *rgbToYcbcr;    by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];
              rc=(rc*redAverage+128)>>8;
              gc=(gc*greenAverage+128)>>8;
              bc=(bc*blueAverage + 128)>>8;
              lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc +
                        bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
              lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
              lumad2 = CLIPIT(lumad2);
              cdr = (bip_ptr->rgbToYcbcr[9]*rc + bip_ptr->rgbToYcbcr[10]*gc + 
                     bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
              cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2)) = (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2+1)) = 
                (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
          /* Cb Path */
        }
      } else {
        failed = 1;
      }
    }
  }

  if (failed)
  {
    return IPL_FAILURE;
  }
  return IPL_SUCCESS;
} /* End ipl_downsize_fast_rggb() */




/* <EJECT> */
static ipl_status_type ipl_downsize_fast_grbg
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr
)
{
  uint32 m,n,mv,nv;
  uint16 row,col,i,j;
  uint8 r,g,b;
  uint8 r2,g2,b2;
  int32 cdb,cdr,lumad1,lumad2;
  int32 rc,gc,bc;
  uint8 failed = 0;
  int32 clr_correct[9];
  uint32 resizeFactorX;
  uint32 resizeFactorY;
  uint32 redAverage=0;
  uint32 greenAverage=0;
  uint32 blueAverage=0;
  uint32 mean = 0;
  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  int16 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};


  if (!o_img_ptr || !o_img_ptr->imgPtr || !i_img_ptr || !i_img_ptr->imgPtr)
    return IPL_FAILURE;

  m = o_img_ptr->dx;
  n = i_img_ptr->dx;
  mv = o_img_ptr->dy;
  nv = i_img_ptr->dy;
  /*
  ** Q7 resize Factors
  */
  resizeFactorX = (n*128 +(m>>1))/m;
  resizeFactorY = (nv*128 + (mv>>1))/mv;

  {
    /*
    ** Input is Bayer GRBG
    */
    /*
    ** Output should be even in width
    */
    if (o_img_ptr->dx&0x1)
    {
      failed = 1;
    } else {
      if (o_img_ptr->cFormat == IPL_YCbCr)
      {
        if (!bip_ptr)
        {
          /* Dont do any processing just demosaic */
          /*
          ** Output is YCbCr
          */
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              lumad1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                        ycbcr_convert[2]*b)*4+0x8000;
              lumad1 = (lumad1>>16) + 16;
              lumad1 = CLIPIT(lumad1);
              cdb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                     ycbcr_convert[5]*b)*4+0x8000;
              cdb = (cdb>>16) + 128;
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2)) = (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1)) = 
                (uint8)lumad1;
              /* Cr Path */
              lumad2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + 
                        ycbcr_convert[2]*b2)*4+0x8000;
              lumad2 = (lumad2>>16) + 16;
              lumad2 = CLIPIT(lumad2);
              cdr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + 
                     ycbcr_convert[8]*b2)*4+0x8000;
              cdr = (cdr>>16) + 128;
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2)) = (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2+1)) = 
                (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
        } else {
          /*
          ** Output is YCbCr
          */
          /*
          ** Do some Image Processing
          */
          /*
          ** Fuse Color Correction and multiplicative gains in one
          */
          /*
          ** *colorCorrection; * a11 a12 a13 a21 a22 a23 a31 a32 a33
          */
          for (i =0;i<3;i++)
          {
            clr_correct[3*i] = (int32)bip_ptr->colorCorrection[3*i];
            clr_correct[3*i+1] = (int32)bip_ptr->colorCorrection[3*i+1];
            clr_correct[3*i+2] = (int32)bip_ptr->colorCorrection[3*i+2];
          }
          /*
          ** RGB data for 1 line
          */
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /*
              ** Write out the resize value
              */
              /* Cb Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (clr_correct[0]*r + clr_correct[1]*g + 
                    clr_correct[2]*b)*8 + 0x8000;
              gc = (clr_correct[3]*r + clr_correct[4]*g + 
                    clr_correct[5]*b)*8 + 0x8000;
              bc = (clr_correct[6]*r + clr_correct[7]*g + 
                    clr_correct[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=(rc*bip_ptr->rGain + 0x1000)>>13;
              gc=(gc*bip_ptr->gGain + 0x1000)>>13;
              bc=(bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];
              redAverage += (uint32) rc;
              greenAverage += (uint32) gc;
              blueAverage += (uint32) bc;
              /* Cr Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = ((int32)clr_correct[0]*r2 + clr_correct[1]*g2 + 
                    clr_correct[2]*b2)*8 + 0x8000;
              gc = ((int32)clr_correct[3]*r2 + clr_correct[4]*g2 + 
                    clr_correct[5]*b2)*8 + 0x8000;
              bc = ((int32)clr_correct[6]*r2 + clr_correct[7]*g2 + 
                    clr_correct[8]*b2)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=(rc*bip_ptr->rGain + 0x1000)>>13;
              gc=(gc*bip_ptr->gGain + 0x1000)>>13;
              bc=(bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);
              /*
              **  Table Lookup
              */
              /*
              ** *rgbToYcbcr;    by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];
              redAverage += (uint32) rc;
              greenAverage += (uint32) gc;
              blueAverage += (uint32) bc;
            } /* End of row for_loop */
          } /* End of Image for_loop */
          redAverage = redAverage / (o_img_ptr->dy*o_img_ptr->dx);
          greenAverage = greenAverage / (o_img_ptr->dy*o_img_ptr->dx);
          blueAverage = blueAverage / (o_img_ptr->dy*o_img_ptr->dx);
          mean = (uint32)((redAverage+blueAverage+greenAverage)/3);
          redAverage = (redAverage + ((mean-redAverage)>>2));
          if (redAverage)
          {
            redAverage = (mean*256)/redAverage;
          } else {
            redAverage = 255;
          }
          greenAverage = (greenAverage + ((mean-greenAverage)>>2));
          if (greenAverage)
          {
            greenAverage = (mean*256)/greenAverage;
          } else {
            greenAverage = 255;
          }
          blueAverage = (blueAverage + ((mean-blueAverage)>>2));
          if (blueAverage)
          {
            blueAverage = (mean*256)/blueAverage;
          } else {
            blueAverage = 255;
          }
          for (row=0;row<o_img_ptr->dy;row++)
          {
            for (col=0;col<o_img_ptr->dx;col=col+2)
            {
              i = (uint16)(((uint32)(row*resizeFactorY<<9) + 0x8000)>>16);
              j = (uint16)(((uint32)(col*resizeFactorX<<9) + 0x8000)>>16);
              /*
              ** First pixel
              */
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b = (uint8)(bc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r = (uint8)(rc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r = (uint8)(rc >>1);
              }
              /*
              ** Do the second pixel
              */
              j = (uint16)(((uint32)((col+1)*resizeFactorX<<9) + 0x8000)>>16);
              if ((j%2) && !(i%2))
              {
                /*
                ** R Input Position
                */
                r2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                b2 = (uint8)(bc >>2);
              } else if (!(j%2) && (i%2))
              {
                /*
                ** B Input Position
                */
                b2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate G and R
                */
                /*
                ** Find G
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) - \
                  *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                rc = rc*rc;
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) - \
                  *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                bc = bc*bc;
                if (rc > bc)
                {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                    *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)));
                } else {
                  gc = ( *(uint8*)(i_img_ptr->imgPtr + (i*n+j-1)) + \
                    *(uint8*)(i_img_ptr->imgPtr + (i*n+j+1)));
                }
                g2 = (uint8)(gc>>1);
                /*
                ** Find R
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j+1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j+1)));
                r2 = (uint8)(rc >>2);
              } else if (!(j%2) && !(i%2))
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                r2 = (uint8)(rc >>1);
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                b2 = (uint8)(bc >>1);
              } else
              {
                /*
                ** G Input Position
                */
                g2 = *(uint8*)(i_img_ptr->imgPtr + (i*n+j));
                /*
                ** Interpolate R and B
                */
                bc = ( *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j-1)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i)*n+j+1)) );
                b2 = (uint8)(bc >>1);
                rc = ( *(uint8*)(i_img_ptr->imgPtr + ((i-1)*n+j)) + \
                       *(uint8*)(i_img_ptr->imgPtr + ((i+1)*n+j)) );
                r2 = (uint8)(rc >>1);
              }
              /* Cb Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = (clr_correct[0]*r + clr_correct[1]*g + 
                    clr_correct[2]*b)*8 + 0x8000;
              gc = (clr_correct[3]*r + clr_correct[4]*g + 
                    clr_correct[5]*b)*8 + 0x8000;
              bc = (clr_correct[6]*r + clr_correct[7]*g + 
                    clr_correct[8]*b)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=(rc*bip_ptr->rGain + 0x1000)>>13;
              gc=(gc*bip_ptr->gGain + 0x1000)>>13;
              bc=(bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */

              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);
              /*
              **  Table Lookup
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];
              rc=(rc*redAverage+128)>>8;
              gc=(gc*greenAverage+128)>>8;
              bc=(bc*blueAverage + 128)>>8;
              lumad1 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc +
                        bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
              lumad1 = (lumad1>>16) + bip_ptr->rgbToYcbcr[0];
              lumad1 = CLIPIT(lumad1);
              cdb = (bip_ptr->rgbToYcbcr[5]*rc + bip_ptr->rgbToYcbcr[6]*gc + 
                     bip_ptr->rgbToYcbcr[7]*bc)*8+0x8000;
              cdb = (cdb>>16) + bip_ptr->rgbToYcbcr[4];
              cdb = CLIPIT(cdb);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2)) = (uint8)cdb;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col)*2+1)) = 
                (uint8)lumad1;
              /* Cr Path */
              /*
              **  Color Correction and Multiplicative Gain
              */
              rc = ((int32)clr_correct[0]*r2 + clr_correct[1]*g2 + 
                    clr_correct[2]*b2)*8 + 0x8000;
              gc = ((int32)clr_correct[3]*r2 + clr_correct[4]*g2 + 
                    clr_correct[5]*b2)*8 + 0x8000;
              bc = ((int32)clr_correct[6]*r2 + clr_correct[7]*g2 + 
                    clr_correct[8]*b2)*8 + 0x8000;
              rc = (rc>>16) + bip_ptr->colorCorrection[9];
              gc = (gc>>16) + bip_ptr->colorCorrection[10];
              bc = (bc>>16) + bip_ptr->colorCorrection[11];
              rc=(rc*bip_ptr->rGain + 0x1000)>>13;
              gc=(gc*bip_ptr->gGain + 0x1000)>>13;
              bc=(bc*bip_ptr->bGain + 0x1000)>>13;
              /*
              ** Write out the resize value
              */
              rc=CLIPIT(rc);
              gc=CLIPIT(gc);
              bc=CLIPIT(bc);
              /*
              **  Table Lookup
              */
              /*
              ** *rgbToYcbcr;    by a11 a12 a13 bcb a21 a22 a23 bcr a31 a32 a33
              */
              rc=bip_ptr->gammaTable[rc];
              gc=bip_ptr->gammaTable[gc];
              bc=bip_ptr->gammaTable[bc];
              rc=(rc*redAverage+128)>>8;
              gc=(gc*greenAverage+128)>>8;
              bc=(bc*blueAverage + 128)>>8;
              lumad2 = (bip_ptr->rgbToYcbcr[1]*rc + bip_ptr->rgbToYcbcr[2]*gc +
                        bip_ptr->rgbToYcbcr[3]*bc)*8+0x8000;
              lumad2 = (lumad2>>16) + bip_ptr->rgbToYcbcr[0];
              lumad2 = CLIPIT(lumad2);
              cdr = (bip_ptr->rgbToYcbcr[9]*rc + bip_ptr->rgbToYcbcr[10]*gc + 
                     bip_ptr->rgbToYcbcr[11]*bc)*8+0x8000;
              cdr = (cdr>>16) + bip_ptr->rgbToYcbcr[8];
              cdr = CLIPIT(cdr);
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2)) = (uint8)cdr;
              *(uint8*)(o_img_ptr->imgPtr + (((row*m)+col+1)*2+1)) = 
                (uint8)lumad2;
            } /* End of row for_loop */
          } /* End of Image for_loop */
          /* Cb Path */
        }
      } else {
        failed = 1;
      }
    }
  }

  if (failed)
  {
    return IPL_FAILURE;
  }
  return IPL_SUCCESS;
} /* End ipl_downsize_fast_grbg() */



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_downsize_fast

DESCRIPTION
  This function performs resizing and color conversion. The bip_ptr can be 
  set to NULL if no color processing is desired. Demosaicing is still 
  performed when the bip_ptr is set to NULL.

  This function is optimized for fast execution speed.

  The input must be YCbCr 4:2:2, RGB565, Bayer, or RGB888.
  The output must be YCbCr 4:2:2 or RGB565.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr    pointer to the input image
  output_img_ptr   pointer to the output image
  bip_ptr          pointer to a BIP structure 
                   (for Mega Pixel Color Processing Support)

RETURN VALUE
  IPL_SUCCESS      indicates operation was successful
  IPL_FAILURE      otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_downsize_fast
(
  ipl_image_type* i_img_ptr,        /* Points to the input image           */
  ipl_image_type* o_img_ptr,        /* Points to the output image          */
  ipl_bip_type*   bip_ptr           /* Points to the BIP structure         */
)
{
  if (!o_img_ptr || !o_img_ptr->imgPtr || !i_img_ptr || !i_img_ptr->imgPtr)
    return IPL_FAILURE;

  if ((i_img_ptr->dx < o_img_ptr->dx) ||
      (i_img_ptr->dy < o_img_ptr->dy))
  {
    /*
    ** Strictly a downsize
    */
    return IPL_FAILURE;
  }

  if (i_img_ptr->cFormat == IPL_RGB565)
  {
    return ipl_downsize_fast_rgb565(i_img_ptr, o_img_ptr, bip_ptr);
  } else if (i_img_ptr->cFormat == IPL_YCbCr)
  {
    return ipl_downsize_fast_ycbcr(i_img_ptr, o_img_ptr, bip_ptr);
  } else if (i_img_ptr->cFormat == IPL_BAYER_GBRG)
  {
    return ipl_downsize_fast_gbrg(i_img_ptr, o_img_ptr, bip_ptr);
  } else if (i_img_ptr->cFormat == IPL_BAYER_GRBG)
  {
    return ipl_downsize_fast_grbg(i_img_ptr, o_img_ptr, bip_ptr);
  } else if (i_img_ptr->cFormat == IPL_BAYER_RGGB)
  {
    return ipl_downsize_fast_rggb(i_img_ptr, o_img_ptr, bip_ptr);
  } else if (i_img_ptr->cFormat == IPL_BAYER_BGGR)
  {
    return ipl_downsize_fast_bggr(i_img_ptr, o_img_ptr, bip_ptr);
  } else {
    return IPL_FAILURE;
  }
} /* End ipl_downsize_fast() */







#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_image_merge

DESCRIPTION
  This function adds 2 images, clips result, and performs color conversion 
  if needed.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr    pointer to the input image
  input_img2_ptr   pointer to the input image
  output_img_ptr   pointer to the output image

RETURN VALUE
  IPL_SUCCESS      indicates operation was successful
  IPL_FAILURE      otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_image_merge
(
  ipl_image_type* i_img_ptr,         /* Points to the input image      */
  ipl_image_type* i_img2_ptr,        /* Points to the frame image      */
  ipl_image_type* o_img_ptr          /* Points to the output image     */
)
{
  /* Take an image of size of any size and converts it to black and white
  */
  int32 index;
  uint32 luma1=0,luma2=0,row,col;
  unsigned short out;
  uint32 cb=0,cr=0;
  long rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  short ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
  unsigned char* frameImgPtr;

  ipl_image_type* input_img_ptr = i_img_ptr;
  ipl_image_type* input_frame_ptr = i_img2_ptr;
  ipl_image_type* output_img_ptr = o_img_ptr;

  unsigned char* inputImgPtr;
  unsigned char* outputImgPtr;

  boolean convert = TRUE;

  if (!o_img_ptr || !o_img_ptr->imgPtr || !i_img_ptr || !i_img_ptr->imgPtr)
    return IPL_FAILURE;

  if (!i_img2_ptr || !i_img2_ptr->imgPtr)
    return IPL_FAILURE;

  if (!input_img_ptr || !input_img_ptr->imgPtr)
    return IPL_FAILURE;

  if (!output_img_ptr || !output_img_ptr->imgPtr)
    return IPL_FAILURE;

  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = output_img_ptr->imgPtr;

  if (i_img2_ptr != NULL)
    frameImgPtr = i_img2_ptr->imgPtr;
  else
    return IPL_FAILURE;

  if (input_img_ptr->cFormat != IPL_YCbCr)
  {
    /* Only YCbCr Input format currently supported */
    return IPL_FAILURE;
  }

  if ((input_frame_ptr != NULL) && 
      (!((output_img_ptr->dx == input_frame_ptr->dx) &&
      (output_img_ptr->dy == input_frame_ptr->dy))))
  {
    /* Frame should be the same size as the output image */
    return IPL_FAILURE;
  }

  if (output_img_ptr->cFormat == input_img_ptr->cFormat)
  {
    /* We don't need to color convert */
    convert = FALSE;
  }

  /* Now loop through the image once */
  for(row = 0; row < output_img_ptr->dy; row++)
  {
    for(col = 0; col < (output_img_ptr->dx); col=col+2)
    {
      index = (col + row*output_img_ptr->dx)<<1;  /* byte addressed */
      /* First Byte is is either Cb or Cr. CbY CrY CbY CrY CbY CrY */
      cb=inputImgPtr[index];
      /* Next Byte is the luma */
      luma1 =  inputImgPtr[index+1]; //Byte addressed
      /* This is Cr */
      cr = inputImgPtr[index+2];
      /* Next Byte is the luma */
      luma2 = inputImgPtr[index+3];
      /* Now lets add the image */
      cr = cr + frameImgPtr[index+2];
      luma2 = luma2 + frameImgPtr[index+3];
      cb = cb + frameImgPtr[index];
      luma1 = luma1 + frameImgPtr[index+1];
      cr=CLIPIT((int32)cr);
      luma2=CLIPIT((int32)luma2);
      cb=CLIPIT((int32)cb);
      luma1=CLIPIT((int32)luma1);

      if (convert)
      {
        /* YCbCr2RGB lags behind by 2 pixels always */
        rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
        gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
        bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;
        r = luma1 + (rc>>16);
        g = luma1 + (gc>>16);
        b = luma1 + (bc>>16);
        r=CLIPIT(r);
        g=CLIPIT(g);
        b=CLIPIT(b);
        /*
        ** Masking bits for 5 ==> 0xF8 and 6==> 0xFC
        ** Order of the 2 bytes is R5G3 G3B5
        */
        out = pack_rgb565(r,g,b);
        *((unsigned short *)(outputImgPtr + index) ) = (unsigned short) out;
        r = luma2 + (rc>>16);
        g = luma2 + (gc>>16);
        b = luma2 + (bc>>16);
        r=CLIPIT(r);
        g=CLIPIT(g);
        b=CLIPIT(b);
        out = pack_rgb565(r,g,b);
        *((unsigned short*)(outputImgPtr + index+2 ) )= (unsigned short) out;
      } else {
        /* Dont need to RGB conversion, simply output data in YCbCr format */
        output_img_ptr->imgPtr[index] =  (unsigned char) cb;
        output_img_ptr->imgPtr[index+1] =  (unsigned char)luma1;
        output_img_ptr->imgPtr[index+2] =  (unsigned char) cr;
        output_img_ptr->imgPtr[index+3] =  (unsigned char)luma2;
      }
    } /* End of col loop */
  } /* End of row loop */
  return IPL_SUCCESS;
} /* End ipl_image_merge */


#endif


#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_bad_pixel_correct_old

DESCRIPTION
  This function is used to do bad pixel correction.
  The Input Image is overwritten with the output image.
  The input should be in either IPL_BAYER_BGGR or in
  IPL_BAYER_RGGB format.
  The output will be in the same color format as input.

DEPENDENCIES
  None

ARGUMENTS IN
  img_ptr points to the input and also the output image.
  min_factor is the multiplicant of the min pixel value Q12 Unsigned
  max_factor is the multiplicant of the max pixel value Q12 Unsigned

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_bad_pixel_correct_old
(
  ipl_image_type* img_ptr,          /* Points to the input image           */
  uint16 min_factor,                /* min factor threshold                */
  uint16 max_factor                 /* max factor threshold                */
)
{
  uint16 i,j;
  uint8* data_ptr;
  uint8* p1;
  uint8* p2;
  int32 max,min;
  int32 temp;
  int fix = 0;
  int looked = 0;

  if (!img_ptr || !img_ptr->imgPtr)
    return IPL_FAILURE;


  data_ptr = img_ptr->imgPtr;
  /*
  data_ptr += img_ptr->dx*2 + 3; // points to g
  p1 = img_ptr->imgPtr;
  p1 += img_ptr->dx +2;
  p2 = img_ptr->imgPtr;
  p2 += img_ptr->dx*3 + 2;
  */

  data_ptr += img_ptr->dx*1 + 2; // points to g
  p1 = img_ptr->imgPtr;
  p1 += 1;
  p2 = img_ptr->imgPtr;
  p2 += img_ptr->dx*2 + 1;

  /* for (i=0;i<img_ptr->dy-4;i++) */
  for (i=0;i<img_ptr->dy-2;i++)
  {
    /* for (j=0;j<(img_ptr->dx-4)>>1;j++) */
    for (j=0;j<(img_ptr->dx-2)>>1;j++)
    {
      if (IPL_ATTIC_PROFILE) looked++;

      max = *p1;
      min = *p1;
      p1 += 2;
      if (*p1 > max)
      {
        max = *p1;
      }
      if (*p1 < min)
      {
        min = *p1;
      }
      /*
      ** bottom 2 pixels
      */
      if (*p2 > max)
      {
        max = *p2;
      }
      if (*p2 < min)
      {
        min = *p2;
      }
      p2 += 2;
      if (*p2 > max)
      {
        max = *p2;
      }
      if (*p2 < min)
      {
        min = *p2;
      }

      /*
      ** Now compare max and min with actual pixel
      */
      temp = (max*max_factor)>>12;
      temp = (int32)CLIPIT(temp);
      if (*data_ptr > temp)
      {
        if (IPL_ATTIC_PROFILE) fix++;
        if (IPL_ATTIC_DEBUG) //lint !e774 
          printf("G (%d,%d) Setting %d to %ld (t%ld)\n", i,j,*data_ptr,max,temp);
        *data_ptr = (uint8)max;
      }
      temp = (min*min_factor)>>12;
      temp = (int32)CLIPIT(temp);
      if (*data_ptr < temp)
      {
        if (IPL_ATTIC_PROFILE) fix++;
        if (IPL_ATTIC_DEBUG) //lint !e774 
          printf("G (%d,%d) Setting %d to %ld (t%ld)\n", i,j,*data_ptr,min,temp);
        *data_ptr = (uint8)min;
      }
      data_ptr += 2;
    }
    if ((i%2))
    {
      data_ptr += 3;
      p1 += 3;
      p2 += 3;
    } else {
      data_ptr += 1;
      p1 += 1;
      p2 += 1;
    }
  }

  /*
  ** Correct Blue Pixels
  */
  /*
  ** Init pointers
  */
  data_ptr = img_ptr->imgPtr;
  data_ptr += img_ptr->dx*2 + 2; // points to b
  p1 = img_ptr->imgPtr + 2;
  p2 = img_ptr->imgPtr;
  p2 += img_ptr->dx*4 + 2;

  for (i=0;i<(img_ptr->dy-4)>>1;i++)
  {
    for (j=0;j<(img_ptr->dx-4)>>1;j++)
    {
      if (IPL_ATTIC_PROFILE) looked++;

      /*
      ** Find max and min
      */
      /*
      ** Top 1 Pixels, Assign max right away
      */
      max = *p1;
      min = *p1;
      p1 += 2;
      /*
      ** bottom 1 pixels
      */
      if (*p2 > max)
      {
        max = *p2;
      }
      if (*p2 < min)
      {
        min = *p2;
      }
      p2 += 2;
      /*
      ** Compare with Left Pixel
      */
      if (*(data_ptr-2) > max)
      {
        max = *(data_ptr-2);
      }
      if (*(data_ptr-2) < min)
      {
        min = *(data_ptr-2);
      }

      /*
      ** Compare with Right Pixel
      */
      if (*(data_ptr+2) > max)
      {
        max = *(data_ptr+2);
      }
      if (*(data_ptr+2) < min)
      {
        min = *(data_ptr+2);
      }

      /*
      ** Now compare max and min with actual pixel
      */
      /*
      ** Now compare max and min with actual pixel
      */
      temp = (max*max_factor)>>12;
      temp = (int32)CLIPIT(temp);
      if (*data_ptr > temp)
      {
        if (IPL_ATTIC_PROFILE) fix++;
        if (IPL_ATTIC_DEBUG) //lint !e774 
          printf("B (%d,%d) Setting %d to %ld (t%ld)\n", i,j,*data_ptr,max,temp);
        *data_ptr = (uint8)max;
      }
      temp = (min*min_factor)>>12;
      temp = (int32)CLIPIT(temp);
      if (*data_ptr < temp)
      {
        if (IPL_ATTIC_PROFILE) fix++;
        if (IPL_ATTIC_DEBUG) //lint !e774 
          printf("B (%d,%d) Setting %d to %ld (t%ld)\n", i,j,*data_ptr,min,temp);
        *data_ptr = (uint8)min;
      }
      data_ptr += 2;
    }
    data_ptr += (4 + img_ptr->dx);
    p1 += (4 + img_ptr->dx);
    p2 += (4 + img_ptr->dx);
  }

  /*
  ** Correct Red Pixels
  */
  /*
  ** Init pointers
  */
  data_ptr = img_ptr->imgPtr;
  data_ptr += img_ptr->dx*3 + 3; // points to r
  p1 = img_ptr->imgPtr;
  p1 += img_ptr->dx + 3;
  p2 = img_ptr->imgPtr;
  p2 += img_ptr->dx*5 + 3;

  for (i=0;i<(img_ptr->dy-4)>>1;i++)
  {
    for (j=0;j<(img_ptr->dx-4)>>1;j++)
    {
      if (IPL_ATTIC_PROFILE) looked++;

      /*
      ** Find max and min
      */
      /*
      ** Top 1 Pixels, Assign max right away
      */
      max = *p1;
      min = *p1;
      p1 += 2;
      /*
      ** bottom 1 pixels
      */
      if (*p2 > max)
      {
        max = *p2;
      }
      if (*p2 < min)
      {
        min = *p2;
      }
      p2 += 2;
      /*
      ** Compare with Left Pixel
      */
      if (*(data_ptr-2) > max)
      {
        max = *(data_ptr-2);
      }
      if (*(data_ptr-2) < min)
      {
        min = *(data_ptr-2);
      }

      /*
      ** Compare with Right Pixel
      */
      if (*(data_ptr+2) > max)
      {
        max = *(data_ptr+2);
      }
      if (*(data_ptr+2) < min)
      {
        min = *(data_ptr+2);
      }

      /*
      ** Now compare max and min with actual pixel
      */
      /*
      ** Now compare max and min with actual pixel
      */
      temp = (max*max_factor)>>12;
      temp = (int32)CLIPIT(temp);

      if (*data_ptr > temp)
      {
        if (IPL_ATTIC_PROFILE) fix++;
        if (IPL_ATTIC_DEBUG) //lint !e774 
          printf("R (%d,%d) Setting %d to %ld (t%ld)\n", i,j,*data_ptr,max,temp);
        *data_ptr = (uint8)max;
      }
      temp = (min*min_factor)>>12;
      temp = (int32)CLIPIT(temp);
      if (*data_ptr < temp)
      {
        if (IPL_ATTIC_PROFILE) fix++;
        if (IPL_ATTIC_DEBUG) //lint !e774 
          printf("R (%d,%d) Setting %d to %ld (t%ld)\n", i,j,*data_ptr,min,temp);
        *data_ptr = (uint8)min;
      }
      data_ptr += 2;
    }
    data_ptr += (4 + img_ptr->dx);
    p1 += (4 + img_ptr->dx);
    p2 += (4 + img_ptr->dx);
  }

  if (IPL_ATTIC_PROFILE) 
  {
    printf ("new version  looked %d pixels\n", looked);
    printf ("new version   fixed %d bad pixels\n", fix);
  }

  return IPL_SUCCESS;
}


#endif


#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_create_bilinear_weights()

DESCRIPTION
  This function is used to create the table used by ipl_upsize to do High 
  Quality upsize via bilinear interpolation. That table is called
  static const uint8 biWeights[8][8][4].

DEPENDENCIES
  None

ARGUMENTS IN
  None

ARGUMENTS IN
  Prints table

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern void ipl_create_bilinear_weights()
{
  int x, y;

  for (y = 0; y < 8; y++)
  {
    for (x = 0; x < 8; x++)
      printf("{%d %d %d %d}, ", abs((8-x)*(8-y)),
                                abs(x*(8-y)), 
                                abs((8-x)*y), 
                                abs(x*y));
    printf("\n");
  }
}

#endif




/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    F I L E

DESCRIPTION
  This file contains the implementation of the IPL APIs.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================
                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_attic.c#1 $


when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/05/04   mz      Removed ipl_frame_compose (should be in ipl_comp.c). 
                   Corrected tab spacings and line widths.
09/01/04   bf      Created. See ipl_util.c for previous history
===========================================================================*/


/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/


#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ResizeImage

DESCRIPTION
  This function does downsampling 4:1 with an averaging filter.
  The data is assumed to be in CbY CrY format ie the chroma
  is the first byte.

  For example,
  data_in can be a CIF 4:2:2 frame 352 * 288.
  data_out will be a QCIF 4:2:2 frame 176 * 144.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in is the input data which needs to be down sampled
  width is the width of input frame.
  height is the height of the input frame

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out is where the output data is stored

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern void ResizeImage
(
    unsigned char* data_in, 
    unsigned char* data_out,
    short width, 
    short height
)
{
  short i,j,chroma,luma;

  if (!data_in || !data_out)
    return;

  for(i = 0; i < (height>>1); i++){
    for(j = 0; j < (width>>1); j++){
      /* First Byte is is either Cb or Cr. CbY CrY CbY CrY CbY CrY */
      if (!(j%2)) {
        /* This is Cb */
        chroma = (short)((data_in[2*(2*j+2*i*width)] + 
                          data_in[2*(2*j +2*i*width)+4] + 
                          data_in[2*(2*j + (2*i+1)*width)] + 
                          data_in[2*(2*j+(2*i+1)*width)+4])>>2);
      } else {
        /* This is Cr */
        chroma = (short)((data_in[2*(2*j+2*i*width)-2] + 
                          data_in[2*(2*j +2*i*width)+2] + 
                          data_in[2*(2*j + (2*i+1)*width)-2] + 
                          data_in[2*(2*j+(2*i+1)*width)+2])>>2);
      } /* end of Cb Cr */
      /* Next Byte is the luma */
      luma = (short)((data_in[2*(2*j+2*i*width)+1] + 
                      data_in[2*(2*j+2*i*width)+3] + 
                      data_in[2*(2*j+(2*i+1)*width)+1] + 
                      data_in[2*(2*j+(2*i+1)*width)+3])>>2);

      /* Can try to not do averaging to compare the difference here */
      /*
      luma = data_in[2*j+1+2*i*width];
      chroma = data_in[2*j+2*i*width];
      */
      data_out[(j+i*(width>>1))*2] = (unsigned char) chroma;
      data_out[(j+i*(width>>1))*2+1] = (unsigned char) luma;
    }
  }
}  

#endif



/* <EJECT> */
/*===========================================================================

FUNCTION mult_ipl_r

DESCRIPTION
  This function takes 2 16 bit numbers and a qfactor
  It multiplies the 2 numbers and returns a 16 bit result with rounding.

DEPENDENCIES
  None

ARGUMENTS IN
  arg1, arg2 and qFactor are uint16 inputs to functions

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  None

RETURN VALUE
  uint16 output is the result of the multiplication with rounding

SIDE EFFECTS
  None

MODIFIED
  09/10/02  Created

===========================================================================*/
__inline uint16 mult_ipl_r(uint16 arg1, uint16 arg2, uint16 arg3) {
  uint32 a = ((uint32)(arg1*arg2<<(16-arg3)) + 0x8000L)>>16;
  return ((uint16)a);
}

#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION create_frame

DESCRIPTION
  This function will create a rectangular frame from an arbitrary image.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  output_img_ptr points to the output image
  transparentValue is the pixel value to imply a non-frame pixel
  border is the width in number of pixels of the frame

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type create_frame
(
  ipl_image_type* input_img_ptr,         /* Points to the input image      */
  ipl_image_type* output_img_ptr,        /* Points to the output image     */
  uint16 transparentValue,               /* value of non-frame pixel       */
  uint32 border                          /* width of the border            */
)
{
  /* Take an image of size of any size and create a rectangular frame from it.
  */
  uint32 row,col,index;
  uint32 luma1=0,luma2=0;


  if (!input_img_ptr || !input_img_ptr->imgPtr)
    return IPL_FAILURE;

  if (!output_img_ptr || !output_img_ptr->imgPtr)
    return IPL_FAILURE;


  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */

  if (input_img_ptr->cFormat != IPL_YCbCr)
  {
    //MSG_FATAL("Only YCbCr Input format currently supported",0,0,0);
    return IPL_FAILURE;
  }

  if (!((input_img_ptr->dy == output_img_ptr->dy) &&
        (input_img_ptr->dx == output_img_ptr->dx)))
  {
    //MSG_FATAL("Input and output image should be of same size",0,0,0);
    return IPL_FAILURE;
  } else {
    /* Now loop through the image once */
    for(row = 0; row < input_img_ptr->dy; row++){
      for(col = 0; col < (input_img_ptr->dx); col++){
        index = ((row*input_img_ptr->dx) + col)<<1;
        if ((col<=border)||(row<=border)||(col>= input_img_ptr->dx - border)||
            (row >= input_img_ptr->dy - border))
        {
          /* copy the input image as a frame pixel */
          /* First Byte is is either Cb or Cr. CbY CrY CbY CrY CbY CrY */
          if (!(col%2)) {
            /* This is Cb */
            /* Next Byte is the luma */
            luma1 =  input_img_ptr->imgPtr[index+1]; //Byte addressed
            if (luma1 == transparentValue)
            {
              luma1 += 1;
            }
          } else {
            /* Next Byte is the luma */
            luma2 = input_img_ptr->imgPtr[index+1];
            if (luma2 == transparentValue)
            {
              luma2 += 1;
            }
            /* No need for RGB conversion, simply output YCbCr data */
            output_img_ptr->imgPtr[index] =  input_img_ptr->imgPtr[index];
            output_img_ptr->imgPtr[index+1] =  input_img_ptr->imgPtr[index+1];
            output_img_ptr->imgPtr[index-2] =  input_img_ptr->imgPtr[index-2];
            output_img_ptr->imgPtr[index-1] =  input_img_ptr->imgPtr[index-1];
          } /* end of Cb Cr */
        } else {
          /* create a non-frame pixel */
          output_img_ptr->imgPtr[index] =  (unsigned char) transparentValue;
          output_img_ptr->imgPtr[index+1] =  (unsigned char) transparentValue;
        }
      } /* End of col loop */
    } /* End of row loop */
  }
  return IPL_SUCCESS;
} /* End create_frame */

#endif


#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_crop_ycbcr420_to_rgb565

DESCRIPTION
  This functin will accept YCrCb 4:2:0 as input and output RGB565
  Input should be 4:2:0 line packed Format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr:  Pointer to the Input Image
  crop:           Crop in Output

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  output_img_ptr: Pointer to the Output Image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
static ipl_status_type ipl_crop_ycbcr420_to_rgb565
(
  ipl_image_type* input_img_ptr,
  ipl_image_type* output_img_ptr,
  ipl_rect_type* crop
)
{
  register uint8 y,cb,cr,y2;
  uint32 row,col;

  register uint16* data_out;
  register uint16* data2_out;
  uint8* y_ptr;
  uint8* yr2_ptr;
  uint8* c_ptr;
  int32 dest_index;


  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0,coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int16 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};
  int32 rc,gc,bc,r,g,b;

  if (!input_img_ptr || !input_img_ptr->imgPtr)
    return IPL_FAILURE;

  if (!output_img_ptr ||!output_img_ptr->imgPtr)
    return IPL_FAILURE;

  if (!crop)
    return IPL_FAILURE;

  data_out = (uint16*)output_img_ptr->imgPtr;
  data2_out = (uint16*)((uint32)output_img_ptr->imgPtr+output_img_ptr->dx*2);
  y_ptr = input_img_ptr->imgPtr;
  yr2_ptr = input_img_ptr->imgPtr + input_img_ptr->dx;
  c_ptr = input_img_ptr->clrPtr;
  dest_index = (crop->x + output_img_ptr->dx*crop->y);


  /*
  **  Verify Input is either IPL_YCbCr or IPL_RGB565
  */
  if (input_img_ptr->cFormat != IPL_YCbCr420_LINE_PK)
  {
    return IPL_FAILURE;
  }

  /*
  ** Verify the output is RGB666
  */
  if (output_img_ptr->cFormat != IPL_RGB565)
  {
    return IPL_FAILURE;
  }
  /*
  ** Verify that the input and output dimensions are identical
  */
  if (!((input_img_ptr->dx == crop->dx) &&
      (input_img_ptr->dy == crop->dy)))
  {
    return IPL_FAILURE;
  }

  data_out += dest_index;
  data2_out += dest_index;
  dest_index = 2*output_img_ptr->dx - crop->dx;
  /* Now loop through the image once */
  for(row = crop->dy; row; row=row-2){
    for(col = crop->dx; col; col=col-2){
      /*
      ** Work on 4 pixels at one time
      */
      y = *y_ptr++;
      y2 = *y_ptr++;
      cb = *c_ptr++;
      cr = *c_ptr++;
      rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
      rc = rc>>16;
      /*
      ** First Pixel
      */
      r = y + (rc);
      r = CLIPIT(r);
      gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
      gc = gc>>16;
      g = y + (gc);
      g = CLIPIT(g);
      bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;
      bc = bc>>16;
      b = y + (bc);
      b = CLIPIT(b);
      *data_out = pack_rgb565(r,g,b);
      data_out++;
      /*
      ** Second Pixel
      */
      r = y2 + (rc);
      r = CLIPIT(r);
      g = y2 + (gc);
      g = CLIPIT(g);
      b = y2 + (bc);
      b = CLIPIT(b);
      *data_out = pack_rgb565(r,g,b);
      data_out++;
      /*
      ** Third Pixel
      */
      y = *yr2_ptr++;
      y2 = *yr2_ptr++;
      r = y + (rc);
      r = CLIPIT(r);
      g = y + (gc);
      g = CLIPIT(g);
      b = y + (bc);
      b = CLIPIT(b);
      *data2_out = pack_rgb565(r,g,b);
      data2_out++;
      /*
      ** Fourth Pixel
      */
      r = y2 + (rc);
      r = CLIPIT(r);
      g = y2 + (gc);
      g = CLIPIT(g);
      b = y2 + (bc);
      b = CLIPIT(b);
      *data2_out = pack_rgb565(r,g,b);
      data2_out++;
    }
    y_ptr += input_img_ptr->dx;
    yr2_ptr += input_img_ptr->dx;
    data_out += dest_index;
    data2_out += dest_index;
  }
  return IPL_SUCCESS;
} /* ipl_crop_ycbcr420_to_rgb565 */



#endif




/* <EJECT> */
/*===========================================================================

FUNCTION calc_img_hist

DESCRIPTION
  This function will calculate the image histogram in Q8 unsigned

DEPENDENCIES
  None

ARGUMENTS IN/OUT
  input_img is the input image whose histogram needs to be calculated
  hist_ptr is a pointer to where the output should be written. (Q8)
  gray_levels is the number of gray levels

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type calc_img_hist
(
  ipl_image_type* input_img_ptr,
  uint32* img_hist,
  uint32 gray_levels,
  uint32 start_loc,
  uint32 row_dim,
  uint32 col_dim
)
{
  uint32 row,col,index,pixels;
  uint32 hist_ptr[256];
  pixels = row_dim*col_dim;


  if (!input_img_ptr || !input_img_ptr->imgPtr)
    return IPL_FAILURE;

  if (!img_hist)
    return IPL_FAILURE;


  /* Initialize the hist_ptr */
  memset(hist_ptr, 0, 256*sizeof(uint32));
  /*
  for(row = 0; row < gray_levels; row++)
  {
    hist_ptr[row] = 0;
  }
  */

  for(row = 0; row < row_dim; row++)
  {
    for(col = 0; col < (col_dim); col++)
    {
      index = (((row*input_img_ptr->dx) + col)<<1)+1+start_loc; /* Luma Byte */
      hist_ptr[input_img_ptr->imgPtr[index]]++;
    } 
  } 

  /* Now lets find the cdf in place */

  for(row = 0; row < gray_levels; row++)
  {
    /* Divide by # of pixels and leave answer in Q8 */
    hist_ptr[row] = (( hist_ptr[row] * (gray_levels-1))<<8)/pixels;
    img_hist[row] = hist_ptr[row];

    for (col=0;col < row;col++)
    {
      img_hist[row] = img_hist[row] + hist_ptr[col];
    }
    img_hist[row] = img_hist[row]>>8;
  }
  return IPL_SUCCESS;
}



#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ycbcr_to_hsv

DESCRIPTION
  This function converts from YCbCr 4:2:2 to HSV.

DEPENDENCIES
  None

ARGUMENTS IN/OUT
  input_img_ptr     pointer to the input image
  hsv_buffer        output buffer where the HSV data is written

RETURN VALUE
  IPL_SUCCESS       indicates operation was successful
  IPL_FAILURE       otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ycbcr_to_hsv
(
  const ipl_image_type* input_img_ptr,     /* Points to the input image   */
  int16* hsv_buffer                  /* Output HSV buffer           */
)
{
  uint32 row,col,index,s,i,delta;
  int32 cb,cr,luma1=0,luma2=0,red1,g1,b1,red2,g2,b2,min,max,h;
  int32 rc,gc,bc;

  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int32 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};

  if(!hsv_buffer || !input_img_ptr  || !input_img_ptr->imgPtr)
    return IPL_FAILURE;

  /* Now loop through the image once */
  for(row = 0; row < input_img_ptr->dy; row++){
    for(col = 0; col < (input_img_ptr->dx); col=col+2){
      index = ((row*input_img_ptr->dx) + col)<<1;
      /* First byte is either Cb or Cr. CbY CrY CbY CrY CbY CrY */
      /* This is Cb */
      cb = input_img_ptr->imgPtr[index];
      /* Next byte is the luma */
      luma1 =  input_img_ptr->imgPtr[index+1]; //Byte addressed
      /* This is Cr */
      cr = input_img_ptr->imgPtr[index+2];
      /* Next byte is the luma */
      luma2 = input_img_ptr->imgPtr[index+3];
      /* Get RGB 24 bit */
      rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
      gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
      bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;
      red1 = luma1 + (rc>>16);
      g1 = luma1 + (gc>>16);
      b1 = luma1 + (bc>>16);
      red1=CLIPIT(red1);
      g1=CLIPIT(g1);
      b1=CLIPIT(b1);
      /*
      **  Do HSV for 1st pixel
      **    Hue is between 0 - 360
      **    Saturation is between 0 - 255
      **    Value is between 0 - 255
      */
      min = min3(red1,g1,b1);
      max= max3(red1,g1,b1);
      delta = max - min;
      if (max!=0)
      {
        s = ((delta<<16)*255)/max;
        s = (s + 0x8000)>>16;
      } else {
        /* r = g = b =0 */
        s = 0;
        h = 0;
      }
      if (delta!=0)
      {
        if (red1 == max)
        {
          h = (((int32)((int32)g1 - b1)<<16)*60)/(int32)delta; /* h is a Q16 */
        } else if (g1 == max)
        {
          h = ((((int32)((int32)b1 - red1)<<16)*60)/(int32)delta) + 
              (120<<16); /* h is a Q16 */
        } else {
          h = ((((int32)((int32)red1 - g1)<<16)*60)/(int32)delta) + 
              (240<<16); /* h is a Q16 */
        }
        h = (h+0x8000)>>16;
      } else {
        /* r = g = b */
        s = 0;
        h = 0;
      }
      if (h < 0)
      {
        h = h + 360;
      }
      i = max;
      index = ((row*input_img_ptr->dx) + col)*3;
      hsv_buffer[index] = (int16) h;
      hsv_buffer[index+1] = (int16) s;
      hsv_buffer[index+2] = (int16) i;
      /* Do HSV for 2nd pixel */
      red2 = luma2 + (rc>>16);
      g2 = luma2 + (gc>>16);
      b2 = luma2 + (bc>>16);
      red2=CLIPIT(red2);
      g2=CLIPIT(g2);
      b2=CLIPIT(b2);
      /*
      **    Hue is between 0 - 360
      **    Saturation is between 0 - 255
      **    Value is between 0 - 255
      */
      min = min3(red2,g2,b2);
      max= max3(red2,g2,b2);
      delta = max - min;
      if (max!=0)
      {
        s = ((delta<<16)*255)/max;
        s = (s + 0x8000)>>16;
      } else {
        /* r = g = b =0 */
        s = 0;
        h = 0;
      }
      if (delta!=0)
      {
        if (red2 == max)
        {
          h = (((int32)((int32)g2 - b2)<<16)*60)/(int32)delta; /* h is a Q16 */
        } else if (g2 == max)
        {
          h = ((((int32)((int32)b2 - red2)<<16)*60)/(int32)delta) + 
              (120<<16); /* h is a Q16 */
        } else {
          h = ((((int32)((int32)red2 - g2)<<16)*60)/(int32)delta) + 
              (240<<16); /* h is a Q16 */
        }
        h = (h+0x8000)>>16;
      } else {
        /* r = g = b */
        h=0;
        s=0;
      }
      if (h<0)
      {
        h = h + 360;
      }
      i = max;
      hsv_buffer[index+3] = (int16) h;
      hsv_buffer[index+4] = (int16) s;
      hsv_buffer[index+5] = (int16) i;
    } /* End of col loop */
  } /* End of row loop */

  return IPL_SUCCESS;
}  /* End ycbcr_to_hsv */


/* <EJECT> */
/*===========================================================================

FUNCTION hsv_to_ycbcr

DESCRIPTION
  This function converts from HSV to YCbCr 4:2:2.

DEPENDENCIES
  None

ARGUMENTS IN/OUT
  output_img_ptr   pointer to output image
  hsv_buffer       input buffer containing the HSV data

RETURN VALUE
  IPL_SUCCESS   indicates operation was successful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type hsv_to_ycbcr
(
  ipl_image_type* output_img_ptr,    /* Points to the output image   */
  int16* hsv_buffer                  /* Input HSV buffer             */
)
{
  uint32 row,col,index,h,s,i;
  int32 luma1=0,luma2=0,cb,cr,red1,g1,b1,red2,g2,b2,frac,p,q,t;

  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int32 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};


  if(!hsv_buffer || !output_img_ptr  || !output_img_ptr->imgPtr)
    return IPL_FAILURE;


  /* Now loop through the image once */
  for(row = 0; row < output_img_ptr->dy; row++)
  {
    for(col = 0; col < (output_img_ptr->dx); col=col+2)
    {
      index = ((row*output_img_ptr->dx) + col)*3;

      /* Read the hsv data for 1st pixel */
      h = hsv_buffer[index];
      s = hsv_buffer[index+1];
      i = hsv_buffer[index+2];
      /* Convert to 8 bit RGB */
      frac=(h%60);
      p = ((i*(255-s))<<16)/255;
      p = (p+0x8000)>>16;
      q = (i*(255*60-s*frac)<<8)/(255*60);
      q = (q+0x80)>>8;
      t =  (i*(255*60-s*(60-frac))<<8)/(255*60);
      t = (t+0x80)>>8;
      if (h<=60)
      {
        red1 = i;
        g1 = t;
        b1 = p;
      } else if (h<=120)
      {
        red1 = q;
        g1 = i;
        b1 = p;
      } else if (h<=180)
      {
        red1 = p;
        g1 = i;
        b1 = t;
      } else if (h<=240)
      {
        red1 = p;
        g1 = q;
        b1 = i;
      } else if (h<=300)
      {
        red1 = t;
        g1 = p;
        b1 = i;
      } else {
        red1 = i;
        g1 = p;
        b1 = q;
      }
      /* Convert to Y Cb Cr */
      luma1 = (ycbcr_convert[0]*red1 + ycbcr_convert[1]*g1 + 
               ycbcr_convert[2]*b1)*4 + 0x8000;
      luma1 = (luma1>>16) + 16;
      luma1 = CLIPIT(luma1);
      cb = (ycbcr_convert[3]*red1 + ycbcr_convert[4]*g1 + 
            ycbcr_convert[5]*b1)*4 + 0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);
      /* Read the hsv data for 2nd pixel */
      h = hsv_buffer[index+3];
      s = hsv_buffer[index+4];
      i = hsv_buffer[index+5];
      frac = (h%60);
      p = ((i*(255-s))<<16)/255;
      p = (p+0x8000)>>16;
      q = (i*(255*60-s*frac)<<8)/(255*60);
      q = (q+0x80)>>8;
      t =  (i*(255*60-s*(60-frac))<<8)/(255*60);
      t = (t+0x80)>>8;
      if (h<=60)
      {
        red2 = i;
        g2 = t;
        b2 = p;
      } else if (h<=120)
      {
        red2 = q;
        g2 = i;
        b2 = p;
      } else if (h<=180)
      {
        red2 = p;
        g2 = i;
        b2 = t;
      } else if (h<=240)
      {
        red2 = p;
        g2 = q;
        b2 = i;
      } else if (h<=300)
      {
        red2 = t;
        g2 = p;
        b2 = i;
      } else {
        red2 = i;
        g2 = p;
        b2 = q;
      }
      /* 2nd pixel */
      luma2 = (ycbcr_convert[0]*red2 + ycbcr_convert[1]*g2 + 
               ycbcr_convert[2]*b2)*4 + 0x8000;
      luma2 = (luma2>>16) + 16;
      luma2=CLIPIT(luma2);
      cr = (ycbcr_convert[6]*red2 + ycbcr_convert[7]*g2 + 
            ycbcr_convert[8]*b2)*4 + 0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      index = ((row*output_img_ptr->dx) + col)<<1;
      output_img_ptr->imgPtr[index] = (unsigned char)cb;
      output_img_ptr->imgPtr[index+1] = (unsigned char)luma1;
      output_img_ptr->imgPtr[index+2] = (unsigned char)cr;
      output_img_ptr->imgPtr[index+3] = (unsigned char)luma2;
    } /* End of col loop */
  } /* End of row loop */

  return IPL_SUCCESS;
} /* End hsv_to_ycbcr */



#endif





#if 0
/* <EJECT> */
/*===========================================================================

FUNCTION ipl_whiteboard

DESCRIPTION
  This function takes an image taken of a white board, or chalk board, and 
  cleans it up for easy viewing and printing.
  

DEPENDENCIES
  None

ARGUMENTS IN
  i_img_ptr      pointer to the input image
  o_img_ptr      pointer to the output image
  th_noise       noise threshold, 10 is a good staring point
  gain           gain, 3 is a good staring point


RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_whiteBoard(ipl_image_type *in_img_ptr,
    ipl_image_type *out_img_ptr, int blackMode, int th_noise) 
{
  // by default, send 1 to rm_border to whiteBoard to remove boreder
  return ipl_whiteBoard(in_img_ptr, out_img_ptr, blackMode, th_noise, 1);
}
#endif


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB2YCbCr

DESCRIPTION
  This function does color conversion from RGB 565 to YCbCr 4:2:2
  It considers 2 pixels unpacks the rgb values and outputs
  Y1Cb and Y2Cr

DEPENDENCIES
  None

ARGUMENTS IN
  data_in is the input data which needs to be color converted
  width is the width of input frame.
  height is the height of the input frame

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out is where the output data is stored

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_RGB2YCbCr(
     unsigned char* data_in,
     unsigned char* data_out,
     short width,
     short height)
{
  unsigned char r1,g1,b1,r2,g2,b2;
  unsigned short i,j,out,out2;
  int32 luma1,luma2,cb,cr;
  short w;

#if 0
  //  we could attempt to support odd width input with this function, but
  //  it would be too dangerous. In other words, if odd input is provided, 
  //  then we must have even output. The rule is that pad the right most column
  //  i.e. 133 input -> 134 output. However, since this function is very old
  //  and the user does not pass in ipl_image_type structs where we can check
  //  the output buffer size, we must return IPL failure and aske the 
  //  called to call the new function ipl_convert_image, and ensure that otuput
  //  width for YCbCr image always be even, and 1 more than input width if
  //  input is odd.

  ipl_image_type ini;
  ipl_image_type outi;



  ini.dx = width;
  ini.dy = height;
  ini.cFormat = IPL_RGB565;

  if (width%2)
    outi.dx = width+1;
  else
    outi.dx = width;
  outi.dy = height;
  outi.cFormat = IPL_YCbCr;

  return (ipl_convert_image(&ini, &outi));
#else
  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};


  if (!data_in || !data_out)
  {
    MSG_LOW("ipl_RGB2YCbCr marker_200\n");
    return IPL_FAILURE;
  }

  // make sure that if rgb input is odd, we output even YCbCr
  w = width;
  if (w%2) w--;

  MSG_LOW("ipl_RGB2YCbCr marker_1\n");

  /* We will do the conversion on 2 pixels at a time */
  for(i = 0; i < height; i++)
  {
    for(j = 0; j < w; j=j+2)
    {
      /* Read in the RGB 16 for the 2 values  */
      out = *((unsigned short*)(data_in + (j+i*width)*2 ) );
      unpack_rgb565(out,&r1,&g1,&b1);
      luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1
               + ycbcr_convert[2]*b1)*4+0x8000;
      luma1 = (luma1>>16) + 16;
      luma1 = CLIPIT(luma1);
      cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1
            + ycbcr_convert[5]*b1)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      /* 2nd pixel */
      out2 = *((unsigned short*)(data_in + (j+i*width+1)*2 ) );
      unpack_rgb565(out2,&r2,&g2,&b2);
      luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2
               + ycbcr_convert[2]*b2)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2
            + ycbcr_convert[8]*b2)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);

      data_out[(j+(i*w))<<1] = (unsigned char)cb;
      data_out[((j+(i*w))<<1)+1] = (unsigned char)luma1;
      data_out[((j+(i*w))<<1)+2] = (unsigned char)cr;
      data_out[((j+(i*w))<<1)+3] = (unsigned char)luma2;
    }
  }
#endif

  MSG_LOW("ipl_RGB2YCbCr marker_100\n");

  return IPL_SUCCESS;
}


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_YCbCr2RGB

DESCRIPTION
  This function does color conversion from YCbCr to RGB 565.
  It considers 2 pixels Y1Cb and Y2Cr and calculates Rc Gc Bc
  as follows:
  [Rc Gc Bc] = convert(3x2) * [Cb-128 Cr-128]
  R G B = [Y Y Y] + [Rc Gc Bc]
  and then the R G B are packed in 5 6 5 format.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in is the input data which needs to be color converted
  width is the width of input frame.
  height is the height of the input frame

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out is where the output data is stored

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_YCbCr2RGB(
        unsigned char* data_in,
        unsigned short* data_out, 
        short width, 
        short height)
{
  unsigned char cb=0,cr=0,luma1=0,luma2=0;
  unsigned short i,j,out;
  long rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are = 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  short ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};

  MSG_LOW("ipl_YCbCr2RGB marker_0\n");

  if (!data_in || !data_out)
  {
    MSG_LOW("ipl_YCbCr2RGB marker_200\n");
    return IPL_FAILURE;
  }
  
  MSG_LOW("ipl_YCbCr2RGB marker_1\n");

  for(i = 0; i < height; i++)
  {
    for(j = 0; j < width; j=j+2)
    {
      /* We will do the conversion on 2 pixels at a time */
      /* This is Cb */
      cb = data_in[2*(j+i*width)];
      /* Next Byte is luma of first pixel */
      luma1 = data_in[2*(j+i*width)+1];
      /* Next byte is cr */
      cr = data_in[2*(j+i*width)+2];
      /* Next byte is luma of 2nd pixel */
      luma2 = data_in[2*(j+i*width)+3];

      rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
      gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
      bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;

      r = luma1 + (rc>>16);
      g = luma1 + (gc>>16);
      b = luma1 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      out=pack_rgb565(r,g,b);
      data_out[(j+(i*width))] = out;

      r = luma2 + (rc>>16);
      g = luma2 + (gc>>16);
      b = luma2 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      out=pack_rgb565(r,g,b);
      data_out[(j+(i*width)+1)] = out;
    }
  }

  MSG_LOW("ipl_YCbCr2RGB marker_100\n");

  return IPL_SUCCESS;
} /* ipl_YCbCr2RGB */



#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_YCbCr2RGB_pitch

DESCRIPTION
  This function does color conversion from YCbCr to RGB 565.
  It considers 2 pixels Y1Cb and Y2Cr and calculates Rc Gc Bc
  as follows:
  [Rc Gc Bc] = convert(3x2) * [Cb-128 Cr-128]
  R G B = [Y Y Y] + [Rc Gc Bc]
  and then the R G B are packed in 5 6 5 format.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in is the input data which needs to be color converted
  width is the width of input frame.
  height is the height of the input frame
  pitchx is the number of pixels to skip in x when converting 

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  data_out is where the output data is stored

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  07/31/02  Created

===========================================================================*/
extern ipl_status_type ipl_YCbCr2RGB_pitch(
        unsigned char* data_in,
        unsigned short* data_out, 
        short width, 
        short height,
        short pitchx
        )
{
  unsigned char cb=0,cr=0,luma1=0,luma2=0;
  unsigned short i,j,out, pitch=0;
  long rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are = 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  short ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};

  MSG_LOW("ipl_YCbCr2RGB_pitch marker_0\n",0,0,0);

  if (!data_in || !data_out)
  {
    MSG_LOW("ipl_YCbCr2RGB_pitch marker_200\n",0,0,0);
    return IPL_FAILURE;
  }
 
  MSG_LOW("ipl_YCbCr2RGB_pitch marker_1\n",0,0,0);

  for(i = 0; i < height; i++)
  {
    for(j = 0; j < width; j=j+2)
    {
      /* We will do the conversion on 2 pixels at a time */
      /* This is Cb */
      cb = data_in[2*(j+i*width)];
      /* Next Byte is luma of first pixel */
      luma1 = data_in[2*(j+i*width)+1];
      /* Next byte is cr */
      cr = data_in[2*(j+i*width)+2];
      /* Next byte is luma of 2nd pixel */
      luma2 = data_in[2*(j+i*width)+3];

      rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
      gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
      bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;

      r = luma1 + (rc>>16);
      g = luma1 + (gc>>16);
      b = luma1 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      out=pack_rgb565(r,g,b);
      data_out[(j+(i*width+pitch*pitchx))] = out;

      r = luma2 + (rc>>16);
      g = luma2 + (gc>>16);
      b = luma2 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      out=pack_rgb565(r,g,b);
      data_out[(j+(i*width)+1+pitch*pitchx)] = out;
    }
    pitch++;
  }

  MSG_LOW("ipl_YCbCr2RGB_pitch marker_100\n",0,0,0);

  return IPL_SUCCESS;
} /* ipl_YCbCr2RGB_pitch */


#endif


/*lint -restore */
