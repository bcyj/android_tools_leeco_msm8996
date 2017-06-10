/*===========================================================================

              B I T    M A N I P U L A T I O N    S E R V I C E S

                            D E C L A R A T I O N S

DESCRIPTION
  The following declarations are for use with the Bit Manipulation Services.

Copyright (c) 1991, 1992, 1998 by Qualcomm Technologies, Inc.  All Rights Reserved.
Copyright (c) 1999--2001 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/


/*===========================================================================

$Header: 
   
===========================================================================*/

#include "bit.h"

/*===========================================================================

                      FUNCTION DECLARATIONS

===========================================================================*/

/*============================================================================

FUNCTION B_PACKB

DESCRIPTION
  Packs the given byte into the destination at the given offset for the
  given number of length bits

DEPENDENCIES
  None
   
RETURN VALUE
  None

SIDE EFFECTS
  None
   
============================================================================*/
void b_packb(
   byte src, 
   byte dst[], 
   word pos, 
   word len 
)
{
   word   t_pos = pos % 8;
   word   bits  = 8 - t_pos;

   dst += (pos+len-1)/8;

   if ( bits >= len )
   {
       *dst &= (byte) ~MASK_B(t_pos, len);
       *dst |= (byte) (MASK_B(t_pos, len) & (src << (bits - len)));
   }
   else /* len > bits */
   {
       dst--;
       *dst &= (byte) ~MASK_B(t_pos, bits);
       *dst |= (byte) (MASK_B(t_pos, bits) & (src >> (len - bits)));

       dst++;
       *dst &= (byte) ~MASK_B(0, (len - bits));
       *dst |= (byte) (MASK_B(0, (len - bits)) & (src << (8 - (len - bits))));
   }
} /* END b_packb */

/*============================================================================

FUNCTION B_PACKW

DESCRIPTION
  Packs the given word into the destination at the given offset for the
  given number of length bits

DEPENDENCIES
  None
   
RETURN VALUE
  None

SIDE EFFECTS
  None
   
============================================================================*/
void b_packw(
   word src, 
   byte dst[], 
   word pos, 
   word len 
)
{
   int bits, start;
   byte   mask;
 
   dst += (len+pos-1)/8;        /* point to last byte to be written */
   pos  = (len+pos-1)%8;        /* index of last bit to be written */
   
   if (len > pos)  /* if we are filling all of the left part of the byte */
   {
     start = 0;  
   }
   else            /* There are going to be untouched bits at left of 
                   ** destination byte.                                   */
   {
     start = (pos+1) - len;     
   }
   bits = (pos - start) + 1;    /* # of bits to be written in this byte */
   
   *dst &= (byte) ~MASK_B(start,bits);  /* clear the bits to be written */
   
   *dst |= (byte) (   ( src << (7 - pos) )    /* left-shift src to line up */
                    & MASK_B(start, bits) );  /* only touch desired bits */
   
   dst--;                /* back up one byte */
   src >>= bits;         /* get rid of bits we've consumed already */
   
   if(len > bits)        /* if we need to write into other bytes */
   {
     len -= bits;        /* compute remaining length  */
     
     /* for full bytes, we can just overwrite the old value with the new */
     for ( ; len >= 8 ; len -= 8 ) {
       *dst = (byte)( src );
       dst--;                         /* back up one byte */
       src >>= 8;                     /* get rid of used bits */
     }
 
     if (len > 0)     /* if some bits are leftover... */
     {
       mask = (byte) (0xff << len);   
       *dst &= mask;                  /* clear bits on right side of byte */
       *dst |= ( (byte)( src ) & ~mask);        /* set appropriate bits */
     }
 
   }
} /* END b_packw */

/*============================================================================

FUNCTION B_UNPACKB

DESCRIPTION
  Given a buffer and an offset, unpacks the requested number of bits into
  a byte

DEPENDENCIES
  None
   
RETURN VALUE
  Unpacked item

SIDE EFFECTS
  None
   
============================================================================*/
byte b_unpackb(
   byte *src, 
   word pos, 
   word len
)
{
   byte result = 0;
   int rshift = 0;

   src += pos/8;
   pos %= 8;
   
   rshift = MAX( 8 - (pos + len), 0);

   if ( rshift > 0 ) {

     result = MASK_AND_SHIFT(len, pos, rshift, *src);
  
   } else {

     result = MASK(8-pos, pos, *src);
     src++;
     len -= 8 - pos;

      if ( len > 0 ) result = ( result<<len ) | (*src >> (8-len));  // if any bits left
   }

   return result;
} /* END b_unpackb */

/*============================================================================

FUNCTION B_UNPACKW

DESCRIPTION
  Given a buffer and an offset, unpacks the requested number of bits into
  a word

DEPENDENCIES
  None
   
RETURN VALUE
  Unpacked item

SIDE EFFECTS
  None
   
============================================================================*/
word b_unpackw(
   byte src[], 
   word pos, 
   word len
)
{ 
   word result = 0;
   int rshift = 0;

   src += pos/8;
   pos %= 8;

   rshift = MAX( 8 - (pos + len), 0);

   if ( rshift > 0 ) {

     result = MASK_AND_SHIFT(len, pos, rshift, *src);
  
   } else {

      result = MASK(8-pos, pos, *src);
      src++;
      len -= 8 - pos;

      for ( ; len >= 8  ; len-=8)
      {
         result = ( result<<8 ) | *src++;
      }

      if ( len > 0 ) result = ( result<<len ) | (*src >> (8-len));  // if any bits left
   }

   return result;
} /* END b_unpackw */


