/*========================================================================


*//** @file jps.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2010-11 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //depot/asic/sandbox/users/staceyw/jps/dev/latest/inc/jps.h#2 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/15/10   staceyw     Created file.

========================================================================== */

#ifndef _JPS_H
#define _JPS_H

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "os_int.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* JPS Layout Type */
typedef enum
{
    INTERLEAVED  = 0x01,      // image data is in an alternating line format
    SIDE_BY_SIDE = 0x02,      // image data is in a side-by-side format
    OVER_UNDER   = 0x03,      // image data is in a over-under format
    ANAGLYPH     = 0x04,      // image data is in an anaglyph format

    LAYOUT_TYPE_MAX,

} jps_layout_t;


/* The structure defining the JPS height flag. */
typedef enum
{
    FULL_HEIGHT = 0x00,       // image data is full height
    HALF_HEIGHT = 0x01,       // image data is half height

    HEIGHT_MAX,

} jps_height_t;

/* The structure defining the JPS width flag. */
typedef enum
{
    FULL_WIDTH = 0x00,       // image data is full width
    HALF_WIDTH = 0x01,       // image data is half width

    WIDTH_MAX,

} jps_width_t;

/* The structure defining the JPS field order flag. */
typedef enum
{
    RIGHT_FIELD_FIRST = 0x00, // right field first
    LEFT_FIELD_FIRST  = 0x01, // left field first

    FIELD_ORDER_MAX,

} jps_field_order_t;


/******************************************************************************
*  The structure defining the 3D JPS configuration:
*
*  Corresponding values will be written into steroscopic descriptor(32 bits)
*  of APP3 header:
*
*  1) bits 0 to 7 are media type which is 0x01 for steroscopic image.
*
*  2) bits 8 to 15 are Layout, which has the value of 0x0200 for side-by-side
*     format, and has the value of 0x0300 for over-under format.
*
*  3) bit 16 is HEIGHT bit while 0 stands for full height and 0x010000 stands
*     for half height.
*     bit 17 is WIDTH bit while 0 stands for full width and 0x020000 stands for
*     half height.
*     bit 18 is FIELD order while 0 means right field first and 0x040000 means
*     left field first.
*
*  4) bits 24 to 32 are for seperations: this unsigned 8-bit value specifies
*     the separation between left and right fields in the image file.
*     Typically this will be 0x00.
******************************************************************************/
typedef struct
{
    // layout: side-by-side or over-and-under
    jps_layout_t       layout;

    // height: 0x00:  image data is full height
    //         0x01:  image data is half height
    jps_height_t       height_flag;

    // width:  0x00:  image data is full width
    //         0x02:  image data is half height
    jps_width_t        width_flag;

    // field order bit:  0x00: right field first
    //                   0x04: left field first
    jps_field_order_t  field_order;

    // If layout is side-by-side, value defines horizontal seperation between two fields
    // If layout is over-and-under, value defines vertical seperation between two fields
    uint8_t            separation;

} jps_cfg_3d_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* JPS flags macros */
#define IS_HALF_HEIGHT(f)   f & HALF_HEIGHT
#define IS_FULL_HEIGHT(f)   !(f & HALF_HEIGHT)
#define IS_HALF_WIDTH(f)    f & HALF_WIDTH
#define IS_FULL_WIDTH(f)    !(f & HALF_WIDTH)
#define IS_LEFT_FIRST(f)    f & LEFT_FIELD_FIRST
#define IS_RIGHT_FIRST(f)   !(f & LEFT_FIELD_FIRST)

#define JPS_FLAGS(field_order, width_bit, height_bit)    ((field_order << 2) | (width_bit << 1) | (height_bit))

#endif // #ifndef _JPS_H
