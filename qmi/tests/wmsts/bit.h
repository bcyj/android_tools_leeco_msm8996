#ifndef BIT_H
#define BIT_H
/*===========================================================================

              B I T    M A N I P U L A T I O N    S E R V I C E S

                            D E C L A R A T I O N S

DESCRIPTION
  The following declarations are for use with the Bit Manipulation Services.

Copyright (c) 1991-2001 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/


/*===========================================================================

$Header:
   
===========================================================================*/

#include "comdef.h"

/*===========================================================================

                            MACRO DECLARATIONS

===========================================================================*/

/*===========================================================================
MACRO bitsize

DESCRIPTION
   Computes size in bits of the specified data type.
===========================================================================*/
#define bitsize(type) (sizeof(type) * 8)

/*===========================================================================
MACRO copymask

DESCRIPTION
   Creates a mask of bits sized to the number of bits in the given type.
===========================================================================*/
#define copymask(type) ((0xffffffff) >> (32 - bitsize(type)))

/*===========================================================================
MACRO MASK

DESCRIPTION
   Masks the bits in data at the given offset for given number of width bits.
===========================================================================*/
#define MASK(width, offset, data) \
    /*lint -e701 shift left  of signed quantity  */  \
    /*lint -e702 shift right of signed quantity  */  \
    /*lint -e572 Excessive shift value           */  \
    /*lint -e573 Signed-unsigned mix with divide */  \
    /*lint -e506 Constant value boolean          */  \
    /*lint -e649 Sign fill during constant shift */  \
                                                     \
   (((width) == bitsize(data)) ? (data) :   \
   ((((copymask(data) << (bitsize(data) - ((width) % bitsize(data)))) & copymask(data)) >>  (offset)) & (data))) \
                     \
    /*lint +e701 */  \
    /*lint +e702 */  \
    /*lint +e572 */  \
    /*lint +e573 */  \
    /*lint +e506 */  \
    /*lint +e649 */  

/*===========================================================================
MACRO MASK_AND_SHIFT

DESCRIPTION
   Same as the macro MASK except also shifts the data in the result by the
   given number of shift bits.
===========================================================================*/
#define MASK_AND_SHIFT(width, offset, shift, data)  \
    /*lint -e504 Unusual shifter value */  \
                  ((((signed) (shift)) < 0) ?       \
                    MASK((width), (offset), (data)) << -(shift) :  \
                    MASK((width), (offset), (data)) >>  (((unsigned) (shift)))) \
    /*lint +e504 */

/*===========================================================================
MACRO MASK_B

DESCRIPTION
   Masks the number of bits give by length starting at the given offset.
   Unlike MASK and MASK_AND_SHIFT, this macro only creates that mask, it
   does not operate on the data buffer.
===========================================================================*/
#define MASK_B(offset, len) \
  ((0xff >> offset) & (0xff << (8 - (offset + len))))

/*===========================================================================

                      FUNCTION DECLARATIONS

===========================================================================*/

extern void  b_packb ( byte  val, byte buf[ ], word pos, word len );
extern void  b_packw ( word  val, byte buf[ ], word pos, word len );

extern byte   b_unpackb ( byte buf[ ], word pos, word len );
extern word   b_unpackw ( byte buf[ ], word pos, word len );

#endif /* BIT_H */
