/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    F I L E

DESCRIPTION
  This file contains the implementation of the IPL APIs.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
===========================================================================*/


/*===========================================================================
                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_helper.c#1 $
when       who     what, where, why
--------   ---     ----------------------------------------------------------
===========================================================================*/



/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipl.h"

// decide if we want to log memory usage or not
#ifdef MALLOC_LOGGING
  #include "sys_malloc_mgr.h"
  #define ipl_mm_sys_malloc(a,b,c,d)  mm_sys_malloc(a,b,c,d); 
  #define ipl_mm_sys_free(a,b,c)      mm_sys_free(a,b,c);  
#elif defined SYS_MALLOC_LOGGING
  #define IPL_GROUP 1 //lint !e750
  #define ipl_mm_sys_malloc(a,b,c,d)  sys_malloc(a);
  #define ipl_mm_sys_free(a,b,c)      sys_free(a);
#else 
  #define IPL_GROUP 1 //lint !e750
  #define ipl_mm_sys_malloc(a,b,c,d)  malloc(a);
  #define ipl_mm_sys_free(a,b,c)      free(a);
#endif



// Turn off some lint warnings
/*lint -save -e506, constant value boolean is totally okay */
/*lint -save -e774, constant value boolean is totally okay */
/*lint -save -e534, let me call printf in piece, god */
/*lint -save -e715, okay that bip_ptr not used */
/*lint -save -e737, clamp function is fine */
/*lint -save -e502, clamp function is fine */
/*lint -save -e704, clamp function is fine */

/*lint -save -e525, dont worry about indentation */
/*lint -save -e64,  dont worry warning in ipl_profile */


/*----------------------------------------------------------------------------
 *   Temp Buffer
 *----------------------------------------------------------------------------*/


// feature should be defined to save a few ms by avoiding sys_malloc/sys_free 30 
// times a second when qvp or mdp or other apps call certain functions 
// frequently. 
#ifdef FEATURE_IPL_USE_TEMP_BUFFER
  //#define IPL_TEMP_BUFFER_MAX       (320*240*2 + 10)
  #define IPL_TEMP_BUFFER_MAX         (1*1*2 + 10)
  #define IPL_TEMP_LINE_BUFFER_MAX    (1)
#else
  //#define IPL_TEMP_BUFFER_MAX       (320*240*2 + 10)
  #define IPL_TEMP_BUFFER_MAX         (1*1*2 + 10)
  #define IPL_TEMP_LINE_BUFFER_MAX    (1)
#endif


#ifndef IPL_DEBUG_STANDALONE
__align(4) uint8 ipl_temp_buffer[IPL_TEMP_BUFFER_MAX];
__align(4) uint8 ipl_temp_line_buffer[IPL_TEMP_LINE_BUFFER_MAX];
//#define IPL_DEBUG_WT  0  // debug write tile
#else
uint8 ipl_temp_buffer[IPL_TEMP_BUFFER_MAX];
uint8 ipl_temp_line_buffer[IPL_TEMP_LINE_BUFFER_MAX];
//#define IPL_DEBUG_WT  0  // debug write tile
#endif

uint8 ipl_temp_buffer_inuse = 0;
uint8 ipl_temp_line_buffer_inuse = 0;





#if 0
static const char extension[32][15] = 
{
  "ycbcr",       /* YCbCr pixel color format 4:2:2                  */
  "ycbcr420fp",  /* YCbCr 4:2:0 Frame Packed Format                 */
  "ycbcr420lp",  /* YCbCr 4:2:0 Line Packed Format                  */
  "ycbcr420mb",  /* YCbCr 4:2:0 Line Packed Format                  */
  "rgb",         /* RGB 565 color format                            */
  "rgb888",      /* RGB 888 color format                            */
  "gbrg",        /* Mega Pixel GBRG format                          */
  "bggr",        /* Mega Pixel BGGR format                          */
  "grbg",        /* Mega Pixel GRBG format                          */
  "rggb",        /* Mega Pixel RGGB format                          */
  "rgb666",      /* RGB 666 format                                  */
  "rgb444",      /* RGB 444 format                                  */
  "ycbcr422lp",  /* YCbCr 4:2:2 Line Packed Format                  */
  "ycbcr444lp",  /* YCbCr 4:4:4 Line Packed Format                  */
  "ycrcb420lp",  /* YCrCb 4:2:0 Line Packed Format                  */
  "ycbcr422lp",  /* YCrCb 4:2:2 Line Packed Format                  */
  "ycrcb444lp",  /* YCrCb 4:4:4 Line Packed Format                  */
  "ycbcr444",    /* YCbCr 4:4:4                                     */
  "ycrcb424mb",  /* YCrCb 4:2:0 Macro Block                         */
  "ycbcr424mb",  /* YCrCb 4:2:0 Macro Block                         */
  "ycrcb444mb",  /* YCrCb 4:2:0 Macro Block                         */
  "ycrcb420fp",  /* YCrCb 4:2:0 Frame Packed Format                 */
  "h1v1",        /* H1V1 MCU data, usually from JPEG decoder        */
  "h1v2",        /* H1V2 MCU data, usually from JPEG decoder        */
  "h2v1",        /* H2V1 MCU data, usually from JPEG decoder        */
  "h2v2",        /* H1V2 MCU data, usually from JPEG decoder        */
  "mcu_gray",    /* MCU data, but only y since gray scale           */
  "ycbcr444pad", /* 0YCbCr data (32 bit word, padded on high order) */
  "rgba888",     /* 0RGB data (32 bit word, padded on high order)   */
  "luma",        /* Just Y (luma) data                              */
  "alpha",       /* Just 8bit alpha channel                         */
  "hsv"          /* Hue saturation value format                     */
};
#endif






/*----------------------------------------------------------------------------
 *   Color Converstion Tables
 *----------------------------------------------------------------------------*/

const uint8 rgb565_table[512] = {
  0, 0, 0, 0, 8, 8, 8, 8,
  8, 8, 8, 8, 16, 16, 16, 16,
  16, 16, 16, 16, 24, 24, 24, 24,
  24, 24, 24, 24, 32, 32, 32, 32,
  32, 32, 32, 32, 40, 40, 40, 40,
  40, 40, 40, 40, 48, 48, 48, 48,
  48, 48, 48, 48, 56, 56, 56, 56,
  56, 56, 56, 56, 64, 64, 64, 64,
  64, 64, 64, 64, 72, 72, 72, 72,
  72, 72, 72, 72, 80, 80, 80, 80,
  80, 80, 80, 80, 88, 88, 88, 88,
  88, 88, 88, 88, 96, 96, 96, 96,
  96, 96, 96, 96, 104, 104, 104, 104,
  104, 104, 104, 104, 112, 112, 112, 112,
  112, 112, 112, 112, 120, 120, 120, 120,
  120, 120, 120, 120, 128, 128, 128, 128,
  128, 128, 128, 128, 136, 136, 136, 136,
  136, 136, 136, 136, 144, 144, 144, 144,
  144, 144, 144, 144, 152, 152, 152, 152,
  152, 152, 152, 152, 160, 160, 160, 160,
  160, 160, 160, 160, 168, 168, 168, 168,
  168, 168, 168, 168, 176, 176, 176, 176,
  176, 176, 176, 176, 184, 184, 184, 184,
  184, 184, 184, 184, 192, 192, 192, 192,
  192, 192, 192, 192, 200, 200, 200, 200,
  200, 200, 200, 200, 208, 208, 208, 208,
  208, 208, 208, 208, 216, 216, 216, 216,
  216, 216, 216, 216, 224, 224, 224, 224,
  224, 224, 224, 224, 232, 232, 232, 232,
  232, 232, 232, 232, 240, 240, 240, 240,
  240, 240, 240, 240, 248, 248, 248, 248,
  248, 248, 248, 248, 248, 248, 248, 248,
  0, 0, 4, 4, 4, 4, 8, 8,
  8, 8, 12, 12, 12, 12, 16, 16,
  16, 16, 20, 20, 20, 20, 24, 24,
  24, 24, 28, 28, 28, 28, 32, 32,
  32, 32, 36, 36, 36, 36, 40, 40,
  40, 40, 44, 44, 44, 44, 48, 48,
  48, 48, 52, 52, 52, 52, 56, 56,
  56, 56, 60, 60, 60, 60, 64, 64,
  64, 64, 68, 68, 68, 68, 72, 72,
  72, 72, 76, 76, 76, 76, 80, 80,
  80, 80, 84, 84, 84, 84, 88, 88,
  88, 88, 92, 92, 92, 92, 96, 96,
  96, 96, 100, 100, 100, 100, 104, 104,
  104, 104, 108, 108, 108, 108, 112, 112,
  112, 112, 116, 116, 116, 116, 120, 120,
  120, 120, 124, 124, 124, 124, 128, 128,
  128, 128, 132, 132, 132, 132, 136, 136,
  136, 136, 140, 140, 140, 140, 144, 144,
  144, 144, 148, 148, 148, 148, 152, 152,
  152, 152, 156, 156, 156, 156, 160, 160,
  160, 160, 164, 164, 164, 164, 168, 168,
  168, 168, 172, 172, 172, 172, 176, 176,
  176, 176, 180, 180, 180, 180, 184, 184,
  184, 184, 188, 188, 188, 188, 192, 192,
  192, 192, 196, 196, 196, 196, 200, 200,
  200, 200, 204, 204, 204, 204, 208, 208,
  208, 208, 212, 212, 212, 212, 216, 216,
  216, 216, 220, 220, 220, 220, 224, 224,
  224, 224, 228, 228, 228, 228, 232, 232,
  232, 232, 236, 236, 236, 236, 240, 240,
  240, 240, 244, 244, 244, 244, 248, 248,
  248, 248, 252, 252, 252, 252, 252, 252
};

const uint32 r666[256] = {
0x0, 0x0, 0x1000, 0x1000, 0x1000, 0x1000, 0x2000, 0x2000,
0x2000, 0x2000, 0x3000, 0x3000, 0x3000, 0x3000, 0x4000, 0x4000,
0x4000, 0x4000, 0x5000, 0x5000, 0x5000, 0x5000, 0x6000, 0x6000,
0x6000, 0x6000, 0x7000, 0x7000, 0x7000, 0x7000, 0x8000, 0x8000,
0x8000, 0x8000, 0x9000, 0x9000, 0x9000, 0x9000, 0xa000, 0xa000,
0xa000, 0xa000, 0xb000, 0xb000, 0xb000, 0xb000, 0xc000, 0xc000,
0xc000, 0xc000, 0xd000, 0xd000, 0xd000, 0xd000, 0xe000, 0xe000,
0xe000, 0xe000, 0xf000, 0xf000, 0xf000, 0xf000, 0x10000, 0x10000,
0x10000, 0x10000, 0x11000, 0x11000, 0x11000, 0x11000, 0x12000, 0x12000,
0x12000, 0x12000, 0x13000, 0x13000, 0x13000, 0x13000, 0x14000, 0x14000,
0x14000, 0x14000, 0x15000, 0x15000, 0x15000, 0x15000, 0x16000, 0x16000,
0x16000, 0x16000, 0x17000, 0x17000, 0x17000, 0x17000, 0x18000, 0x18000,
0x18000, 0x18000, 0x19000, 0x19000, 0x19000, 0x19000, 0x1a000, 0x1a000,
0x1a000, 0x1a000, 0x1b000, 0x1b000, 0x1b000, 0x1b000, 0x1c000, 0x1c000,
0x1c000, 0x1c000, 0x1d000, 0x1d000, 0x1d000, 0x1d000, 0x1e000, 0x1e000,
0x1e000, 0x1e000, 0x1f000, 0x1f000, 0x1f000, 0x1f000, 0x20000, 0x20000,
0x20000, 0x20000, 0x21000, 0x21000, 0x21000, 0x21000, 0x22000, 0x22000,
0x22000, 0x22000, 0x23000, 0x23000, 0x23000, 0x23000, 0x24000, 0x24000,
0x24000, 0x24000, 0x25000, 0x25000, 0x25000, 0x25000, 0x26000, 0x26000,
0x26000, 0x26000, 0x27000, 0x27000, 0x27000, 0x27000, 0x28000, 0x28000,
0x28000, 0x28000, 0x29000, 0x29000, 0x29000, 0x29000, 0x2a000, 0x2a000,
0x2a000, 0x2a000, 0x2b000, 0x2b000, 0x2b000, 0x2b000, 0x2c000, 0x2c000,
0x2c000, 0x2c000, 0x2d000, 0x2d000, 0x2d000, 0x2d000, 0x2e000, 0x2e000,
0x2e000, 0x2e000, 0x2f000, 0x2f000, 0x2f000, 0x2f000, 0x30000, 0x30000,
0x30000, 0x30000, 0x31000, 0x31000, 0x31000, 0x31000, 0x32000, 0x32000,
0x32000, 0x32000, 0x33000, 0x33000, 0x33000, 0x33000, 0x34000, 0x34000,
0x34000, 0x34000, 0x35000, 0x35000, 0x35000, 0x35000, 0x36000, 0x36000,
0x36000, 0x36000, 0x37000, 0x37000, 0x37000, 0x37000, 0x38000, 0x38000,
0x38000, 0x38000, 0x39000, 0x39000, 0x39000, 0x39000, 0x3a000, 0x3a000,
0x3a000, 0x3a000, 0x3b000, 0x3b000, 0x3b000, 0x3b000, 0x3c000, 0x3c000,
0x3c000, 0x3c000, 0x3d000, 0x3d000, 0x3d000, 0x3d000, 0x3e000, 0x3e000,
0x3e000, 0x3e000, 0x3f000, 0x3f000, 0x3f000, 0x3f000, 0x3f000, 0x3f000
};

const uint32 g666[256] = {
0x0, 0x0, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80,
0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0x100, 0x100,
0x100, 0x100, 0x140, 0x140, 0x140, 0x140, 0x180, 0x180,
0x180, 0x180, 0x1c0, 0x1c0, 0x1c0, 0x1c0, 0x200, 0x200,
0x200, 0x200, 0x240, 0x240, 0x240, 0x240, 0x280, 0x280,
0x280, 0x280, 0x2c0, 0x2c0, 0x2c0, 0x2c0, 0x300, 0x300,
0x300, 0x300, 0x340, 0x340, 0x340, 0x340, 0x380, 0x380,
0x380, 0x380, 0x3c0, 0x3c0, 0x3c0, 0x3c0, 0x400, 0x400,
0x400, 0x400, 0x440, 0x440, 0x440, 0x440, 0x480, 0x480,
0x480, 0x480, 0x4c0, 0x4c0, 0x4c0, 0x4c0, 0x500, 0x500,
0x500, 0x500, 0x540, 0x540, 0x540, 0x540, 0x580, 0x580,
0x580, 0x580, 0x5c0, 0x5c0, 0x5c0, 0x5c0, 0x600, 0x600,
0x600, 0x600, 0x640, 0x640, 0x640, 0x640, 0x680, 0x680,
0x680, 0x680, 0x6c0, 0x6c0, 0x6c0, 0x6c0, 0x700, 0x700,
0x700, 0x700, 0x740, 0x740, 0x740, 0x740, 0x780, 0x780,
0x780, 0x780, 0x7c0, 0x7c0, 0x7c0, 0x7c0, 0x800, 0x800,
0x800, 0x800, 0x840, 0x840, 0x840, 0x840, 0x880, 0x880,
0x880, 0x880, 0x8c0, 0x8c0, 0x8c0, 0x8c0, 0x900, 0x900,
0x900, 0x900, 0x940, 0x940, 0x940, 0x940, 0x980, 0x980,
0x980, 0x980, 0x9c0, 0x9c0, 0x9c0, 0x9c0, 0xa00, 0xa00,
0xa00, 0xa00, 0xa40, 0xa40, 0xa40, 0xa40, 0xa80, 0xa80,
0xa80, 0xa80, 0xac0, 0xac0, 0xac0, 0xac0, 0xb00, 0xb00,
0xb00, 0xb00, 0xb40, 0xb40, 0xb40, 0xb40, 0xb80, 0xb80,
0xb80, 0xb80, 0xbc0, 0xbc0, 0xbc0, 0xbc0, 0xc00, 0xc00,
0xc00, 0xc00, 0xc40, 0xc40, 0xc40, 0xc40, 0xc80, 0xc80,
0xc80, 0xc80, 0xcc0, 0xcc0, 0xcc0, 0xcc0, 0xd00, 0xd00,
0xd00, 0xd00, 0xd40, 0xd40, 0xd40, 0xd40, 0xd80, 0xd80,
0xd80, 0xd80, 0xdc0, 0xdc0, 0xdc0, 0xdc0, 0xe00, 0xe00,
0xe00, 0xe00, 0xe40, 0xe40, 0xe40, 0xe40, 0xe80, 0xe80,
0xe80, 0xe80, 0xec0, 0xec0, 0xec0, 0xec0, 0xf00, 0xf00,
0xf00, 0xf00, 0xf40, 0xf40, 0xf40, 0xf40, 0xf80, 0xf80,
0xf80, 0xf80, 0xfc0, 0xfc0, 0xfc0, 0xfc0, 0xfc0, 0xfc0
};

const uint32 b666[256] = {
0x0, 0x0, 0x1, 0x1, 0x1, 0x1, 0x2, 0x2,
0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x4, 0x4,
0x4, 0x4, 0x5, 0x5, 0x5, 0x5, 0x6, 0x6,
0x6, 0x6, 0x7, 0x7, 0x7, 0x7, 0x8, 0x8,
0x8, 0x8, 0x9, 0x9, 0x9, 0x9, 0xa, 0xa,
0xa, 0xa, 0xb, 0xb, 0xb, 0xb, 0xc, 0xc,
0xc, 0xc, 0xd, 0xd, 0xd, 0xd, 0xe, 0xe,
0xe, 0xe, 0xf, 0xf, 0xf, 0xf, 0x10, 0x10,
0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x12, 0x12,
0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x14, 0x14,
0x14, 0x14, 0x15, 0x15, 0x15, 0x15, 0x16, 0x16,
0x16, 0x16, 0x17, 0x17, 0x17, 0x17, 0x18, 0x18,
0x18, 0x18, 0x19, 0x19, 0x19, 0x19, 0x1a, 0x1a,
0x1a, 0x1a, 0x1b, 0x1b, 0x1b, 0x1b, 0x1c, 0x1c,
0x1c, 0x1c, 0x1d, 0x1d, 0x1d, 0x1d, 0x1e, 0x1e,
0x1e, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x20, 0x20,
0x20, 0x20, 0x21, 0x21, 0x21, 0x21, 0x22, 0x22,
0x22, 0x22, 0x23, 0x23, 0x23, 0x23, 0x24, 0x24,
0x24, 0x24, 0x25, 0x25, 0x25, 0x25, 0x26, 0x26,
0x26, 0x26, 0x27, 0x27, 0x27, 0x27, 0x28, 0x28,
0x28, 0x28, 0x29, 0x29, 0x29, 0x29, 0x2a, 0x2a,
0x2a, 0x2a, 0x2b, 0x2b, 0x2b, 0x2b, 0x2c, 0x2c,
0x2c, 0x2c, 0x2d, 0x2d, 0x2d, 0x2d, 0x2e, 0x2e,
0x2e, 0x2e, 0x2f, 0x2f, 0x2f, 0x2f, 0x30, 0x30,
0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 0x32, 0x32,
0x32, 0x32, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34,
0x34, 0x34, 0x35, 0x35, 0x35, 0x35, 0x36, 0x36,
0x36, 0x36, 0x37, 0x37, 0x37, 0x37, 0x38, 0x38,
0x38, 0x38, 0x39, 0x39, 0x39, 0x39, 0x3a, 0x3a,
0x3a, 0x3a, 0x3b, 0x3b, 0x3b, 0x3b, 0x3c, 0x3c,
0x3c, 0x3c, 0x3d, 0x3d, 0x3d, 0x3d, 0x3e, 0x3e,
0x3e, 0x3e, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
};

const uint16 r444[] = {
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200,
0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200,
0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300,
0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300,
0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400,
0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400,
0x500, 0x500, 0x500, 0x500, 0x500, 0x500, 0x500, 0x500,
0x500, 0x500, 0x500, 0x500, 0x500, 0x500, 0x500, 0x500,
0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600,
0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600,
0x700, 0x700, 0x700, 0x700, 0x700, 0x700, 0x700, 0x700,
0x700, 0x700, 0x700, 0x700, 0x700, 0x700, 0x700, 0x700,
0x800, 0x800, 0x800, 0x800, 0x800, 0x800, 0x800, 0x800,
0x800, 0x800, 0x800, 0x800, 0x800, 0x800, 0x800, 0x800,
0x900, 0x900, 0x900, 0x900, 0x900, 0x900, 0x900, 0x900,
0x900, 0x900, 0x900, 0x900, 0x900, 0x900, 0x900, 0x900,
0xa00, 0xa00, 0xa00, 0xa00, 0xa00, 0xa00, 0xa00, 0xa00,
0xa00, 0xa00, 0xa00, 0xa00, 0xa00, 0xa00, 0xa00, 0xa00,
0xb00, 0xb00, 0xb00, 0xb00, 0xb00, 0xb00, 0xb00, 0xb00,
0xb00, 0xb00, 0xb00, 0xb00, 0xb00, 0xb00, 0xb00, 0xb00,
0xc00, 0xc00, 0xc00, 0xc00, 0xc00, 0xc00, 0xc00, 0xc00,
0xc00, 0xc00, 0xc00, 0xc00, 0xc00, 0xc00, 0xc00, 0xc00,
0xd00, 0xd00, 0xd00, 0xd00, 0xd00, 0xd00, 0xd00, 0xd00,
0xd00, 0xd00, 0xd00, 0xd00, 0xd00, 0xd00, 0xd00, 0xd00,
0xe00, 0xe00, 0xe00, 0xe00, 0xe00, 0xe00, 0xe00, 0xe00,
0xe00, 0xe00, 0xe00, 0xe00, 0xe00, 0xe00, 0xe00, 0xe00,
0xf00, 0xf00, 0xf00, 0xf00, 0xf00, 0xf00, 0xf00, 0xf00,
0xf00, 0xf00, 0xf00, 0xf00, 0xf00, 0xf00, 0xf00, 0xf00,
0xf00, 0xf00, 0xf00, 0xf00, 0xf00, 0xf00, 0xf00, 0xf00
};

const uint16 g444[] = {
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50,
0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50,
0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70,
0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0,
0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0,
0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0,
0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0,
0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0,
0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0,
0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
};

const uint16 b444[] = {
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4,
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4,
0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7,
0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7,
0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa,
0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa,
0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb,
0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb,
0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc,
0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc,
0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd,
0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd,
0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe,
0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe,
0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf
};



/*lint -save -e508, dont stress over extern issues */


#if 0
/*===========================================================================

FUNCTION   CREATE_COLOR_TABLES

DESCRIPTION
  Use this function to create color conversion tables for efficient 
  YCbCr to RGB conversion.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

============================================================================*/
extern void ipl_create_look_ups(void)
{
  int32 i, k;

  printf("int32 ipl_crr[256] = {\n");
  for (i = 0; i <= 255; i++)
  {
    k = (i * 2) - 255;
    ipl_crr[i] = ( IPL_FIX(1.40200/2)  * k + IPL_ONE_HALF) >> IPL_SCALEBITS;
    printf("%d, ", ipl_crr[i]);
    if (i % 10 == 9) printf("\n");
  }
  printf("};\n\n");


  printf("int32 ipl_cbb[256] = {\n");
  for (i = 0; i <= 255; i++)
  {
    k = (i * 2) - 255;

    ipl_cbb[i] = ( IPL_FIX(1.77200/2)  * k + IPL_ONE_HALF) >> IPL_SCALEBITS;
    printf("%d, ", ipl_cbb[i]);
    if (i % 10 == 9) printf("\n");
  }
  printf("};\n\n");

  printf("int32 ipl_crg[256] = {\n");
  for (i = 0; i <= 255; i++)
  {
    k = (i * 2) - 255;
    ipl_crg[i] = (-IPL_FIX(0.71414/2)) * k;
    printf("%d, ", ipl_crg[i]);
    if (i % 7 == 6) printf("\n");
  }
  printf("};\n\n");


  printf("int32 ipl_cbg[256] = {\n");
  for (i = 0; i <= 255; i++)
  {
    k = (i * 2) - 255;
    ipl_cbg[i] = (-IPL_FIX(0.34414/2)) * k + IPL_ONE_HALF;
    printf("%d, ", ipl_cbg[i]);
    if (i % 7 == 6) printf("\n");
  }
  printf("};\n\n");
}
#endif


extern uint8 ipl_clamp(int32 i)
{
  if (i & 0xFFFFFF00)
    i = (((~i) >> 31) & 0xFF);
  return ((uint8) i);
}



/*===========================================================================

FUNCTION ipl_unpack_rgb565

DESCRIPTION
  This function takes 16 bits rgb565 and unpacks into 24 bit r g b

DEPENDENCIES
  None

ARGUMENTS IN
  in is a word input rgb565

ARGUMENTS OUT
  r - address of R value
  g - address of G value
  b - address of B value


RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type unpack_rgb565
(
  unsigned short in,
  unsigned char* r,
  unsigned char* g,
  unsigned char* b
)
{
  /*------------------------------------------------------------------------
      ** First 5 bits are R
      ** then  6 bits are G
      ** then  5 bits are B
  ------------------------------------------------------------------------*/

  *r = (unsigned char)((in&0xF800)>>8); //lint !e613
  *g = (unsigned char)((in&0x07E0)>>3); //lint !e613
  *b = (unsigned char)((in&0x001F)<<3); //lint !e613
  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION unpack_rgb444

DESCRIPTION
  This function takes 16 bits rgb444 and unpacks into 24 bit r g b

DEPENDENCIES
  None

ARGUMENTS IN
  in is a word input rgb444

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  r, g and b of input as char

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  04/09/04  Created

===========================================================================*/
extern ipl_status_type unpack_rgb444
(
  unsigned short in, 
  unsigned char* r, 
  unsigned char* g,
  unsigned char* b
)
{
  /*
  ** First 4 bits are zero
  ** then  4 bits are R
  ** then  4 bits are G
  ** then  4 bits are B
  */
  /* Shift each up so that values occupy MS bits of each byte */
  *r = (unsigned char)((in&0x0F00)>>4); //lint !e613
  *g = (unsigned char)(in&0x00F0);      //lint !e613
  *b = (unsigned char)((in&0x000F)<<4); //lint !e613
  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION unpack_rgb666

DESCRIPTION
  This function takes 32 bits rgb666 and unpacks into 24 bit r g b

DEPENDENCIES
  None

ARGUMENTS IN
  in is a 32 bit word (4 byte) input rgb666

ARGUMENTS IN/OUT
  None

ARGUMENTS OUT
  r, g and b of input as char

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  04/09/04  Created

===========================================================================*/
extern ipl_status_type unpack_rgb666
(
  unsigned long in, 
  unsigned char* r, 
  unsigned char* g,
  unsigned char* b
)
{
  /*
  ** First 14 bits are zero
  ** then  6 bits are R
  ** then  6 bits are G
  ** then  6 bits are B
  */
  /* Shift each up so that values occupy MS bits of each byte */
  *r = (unsigned char)((in&0x0003F000) >>10 ); //lint !e613
  *g = (unsigned char)((in&0x00000FC0) >>4  ); //lint !e613
  *b = (unsigned char)((in&0x0000003F) <<2  ); //lint !e613
  return IPL_SUCCESS;
}

/*===========================================================================

FUNCTION unpack_ycbcr 

DESCRIPTION
  This function takes 32 bits and unpacks into 24 bit y cb cr

DEPENDENCIES
  None

ARGUMENTS IN
  in is a word input rgb565

ARGUMENTS OUT
  y  - address of Y value
  cb - address of Cb value
  cr - address of Cr value


RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type unpack_ycbcr
(
  unsigned int in,
  unsigned char* y,
  unsigned char* cb,
  unsigned char* cr
)
{

  /*------------------------------------------------------------------------
      ** First 8 bits are Y
      ** then  8 bits are Cb
      ** then  8 bits are Cr
  ------------------------------------------------------------------------*/

  *y  = (unsigned char) ((in >> 16));             //lint !e613
  *cb = (unsigned char) ((in & 0x0000FF00) >> 8); //lint !e613
  *cr = (unsigned char) ((in & 0x000000FF));      //lint !e613
  return IPL_SUCCESS;
} 




/* <EJECT> */
/*===========================================================================

FUNCTION min3

DESCRIPTION
  This function will find min of 3 numbers

DEPENDENCIES
  None

ARGUMENTS IN
  a,b,c are the 3 input numbers

RETURN VALUE
  It returns as output minimum of a,b,c

SIDE EFFECTS
  None

===========================================================================*/
extern int32 min3(int32 a, int32 b, int32 c)
{
  /* returns minimum of a,b and c */
  if (a <= b)
  {
    if (a<= c)
    {
      return a;
    } 
    else
    {
      return c;
    }
  } 
  else 
  {
    if (b <=c)
    {
      return b;
    } 
    else 
    {
      return c;
    }
  }
} /* End min3 */




/* <EJECT> */
/*===========================================================================

FUNCTION max3

DESCRIPTION
  This function will find max of 3 numbers

DEPENDENCIES
  None

ARGUMENTS IN
  a,b,c are the 3 input numbers

RETURN VALUE
  It returns as output maximum of a,b,c

SIDE EFFECTS
  None

===========================================================================*/
extern int32 max3(int32 a, int32 b, int32 c)
{
  /* returns maximum of a,b and c */
  if (a >= b)
  {
    if (a>= c)
    {
      return a;
    } 
    else 
    {
      return c;
    }
  } 
  else 
  {
    if (b >= c)
    {
      return b;
    } 
    else 
    {
      return c;
    }
  }
} /* End max3 */



#if 0


// this guy generates the ciruclar mask for various apps
#include <stdio.h>

main()
{
  int i,j,k,e;
  float dist;
  float dx = 32.0;
  int maxK = 33;
  int minK = 4;

  unsigned long accum;
  char fname[100];

#if 1
  printf("int circularMask[%d][128][128] = {\n", maxK-minK,dx,dx);
  e = 0;
  for (k = minK; k < maxK; k++)
  { 
    dx = k*4;

    printf("/* Entry: %d, Mask for diameter %.0f */\n", e++, dx);
    printf("{\n", k,dx,dx);
    for (j = 0; j < dx; j++)
  {
      printf("{", k,dx,dx);
      for (i = 0; i < dx; i++)
      {
        dist = sqrt((i-dx/2.0)*(i-dx/2.0) + (j-dx/2.0)*(j-dx/2.0));
        //printf("(%d,%d) dist is %d\n", i,j,dist);
        if (dist <= (dx/2.0 - dx/2.0*0.2))
          printf("1,");
        else
          printf("0,");
      }
      printf("},\n");
    }
    printf("},\n");
  }
  printf("};\n");
#endif
}



#endif



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_memory_needed

DESCRIPTION
  This function returns the amount of memory imgPtr and clrPtr should point
  to for a given image size and type.

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   points to the input image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_memory_needed
(
  const ipl_image_type * in, uint32 * isize, uint32 * csize
)
{

  switch (in->cFormat)
  {
    case IPL_BAYER_GBRG:
  	case IPL_BAYER_BGGR:
  	case IPL_BAYER_GRBG: 
  	case IPL_BAYER_RGGB:
    case IPL_LUMA_ONLY:
    case IPL_MCU_GRAY:
    case IPL_ALPHA:
  	  *isize = in->dx * in->dy;
      *csize = 0;
    break;
  
    case IPL_RGB565:
    case IPL_RGB444:
    case IPL_YCbCr:
    case IPL_H1V2MCU_CbCr:
    case IPL_H2V1MCU_CbCr:
  	  *isize = 2 * in->dx * in->dy;
      *csize = 0;
    break;

    case IPL_YCbCr422_LINE_PK: 
    case IPL_YCrCb422_LINE_PK:
    case IPL_YCbCr422_MB_PK:
    case IPL_YCrCb422_MB_PK:
  	  *isize = in->dx * in->dy;
  	  *csize = in->dx * in->dy;
    break;

    case IPL_YCbCr420_LINE_PK:
    case IPL_YCrCb420_LINE_PK: 
    case IPL_YCbCr420_MB_PK:
    case IPL_YCrCb420_MB_PK:
  	  *isize = in->dx * in->dy;
  	  *csize = (in->dx * in->dy) >> 1;
    break;

    case IPL_YCbCr420_FRAME_PK: 
    case IPL_YCrCb420_FRAME_PK: 
  	  *isize = in->dx * in->dy;
  	  *csize = (in->dx * in->dy) >> 1;
    break;

    case IPL_H2V2MCU_CbCr:      
  	  *isize = (in->dx * in->dy * 3) >> 1;
      *csize = 0;
    break;

    case IPL_H1V1MCU_CbCr:
    case IPL_RGB888:
    case IPL_YCbCr444:
    case IPL_HSV:
  	  *isize = (3 * in->dx * in->dy);
      *csize = 0;
    break;

    case IPL_RGB666:
    case IPL_YCbCr444_PAD:
    case IPL_RGB888_PAD:
  	  *isize = (4 * in->dx * in->dy);
      *csize = 0;
    break;

    case IPL_YCrCb444_LINE_PK:
    case IPL_YCbCr444_LINE_PK:
  	  *isize = in->dx * in->dy;
  	  *csize = 2*(in->dx * in->dy);
    break;


    default:
  	  *isize = 0;
      *csize = 0;
      return IPL_FAILURE;
    //break;
  }

  return IPL_SUCCESS;
}


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_malloc

DESCRIPTION
  This function is used to get memory for a given ptr 

DEPENDENCIES
  None

ARGUMENTS IN
  bytes number of bytes to sys_malloc

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN void * ipl_malloc
(
  size_t bytes
)
{
  void * retval = NULL;

  MSG_LOW("ipl_malloc: requesting %d bytes\n", bytes);

  // see if we can use our temp buffer
  if (!ipl_temp_buffer_inuse && (bytes <= IPL_TEMP_BUFFER_MAX))
  {
    ipl_temp_buffer_inuse = 1;
    MSG_LOW("IPL using temp buffer\n");

	  retval = ipl_temp_buffer;
  }
  else
  {
    // Even though we have buffer, it was not big enough to use  
    // therefore, lets sys_malloc
    MSG_LOW("ipl_malloc_img: sys_mallocing memory\n");

	  retval = (uint8 *) ipl_mm_sys_malloc(bytes, IPL_GROUP, __FILE__, __LINE__);

	  if (!retval)
    {
      return NULL;
    }
  }

  return retval;
}




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_sys_free

DESCRIPTION
  This function sys_frees memory alloced by ipl_malloc

DEPENDENCIES
  None

ARGUMENTS IN
  in  points to the buffer to sys_free

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN void ipl_sys_free (void * in)
{
  MSG_LOW("ipl_sys_free: sys_freeing bytes\n");

  // see if this guy used our temp buffer
  if (in == ipl_temp_buffer)
  {
    ipl_temp_buffer_inuse = 0;
    MSG_LOW("ipl_free_img: releasing use of temp buffer\n");
  }
  else
  {
    if (in == NULL)
      MSG_LOW("ipl_free_img: not going to sys_free NULL imgPtr\n");
    else
    {
      MSG_LOW("ipl_free_img: sys_freeing imgPtr\n");
      ipl_mm_sys_free(in, __FILE__, __LINE__);
    }
  }
  in = NULL;
}

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_malloc_img

DESCRIPTION
  This function is used to get memory for a given image of type ipl_image_type

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   points to the input image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_malloc_img
(
  ipl_image_type * in
)
{
  uint32 isize, csize;

  if (in == NULL)
	  return IPL_FAILURE;

  if (in->dx == 0 || in->dy == 0)
  {
	  return IPL_FAILURE;
  }

  // see how much memory we need
  ipl_memory_needed(in, &isize, &csize);

  if (isize == 0 && csize == 0)
  {
    IPL2_MSG_FATAL("ipl_malloc_img: did not know how much memory to allocate\n" );
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_malloc_img: requesting %lu bytes\n", isize);
  MSG_LOW("ipl_malloc_img: requesting %lu bytes\n", csize);

  // see if we can use our temp buffer
  if (!ipl_temp_buffer_inuse && (isize+csize <= IPL_TEMP_BUFFER_MAX))
  {
    ipl_temp_buffer_inuse = 1;
    MSG_LOW("IPL using temp buffer\n");

	  in->imgPtr = ipl_temp_buffer;

    // see if we need to setup clrPtr
    if (in->cFormat == IPL_YCbCr422_LINE_PK || 
        in->cFormat == IPL_YCrCb422_LINE_PK ||
        in->cFormat == IPL_YCbCr420_LINE_PK ||
        in->cFormat == IPL_YCrCb420_LINE_PK ||

        in->cFormat == IPL_YCbCr420_FRAME_PK ||
        in->cFormat == IPL_YCrCb420_FRAME_PK ||

        in->cFormat == IPL_YCbCr422_MB_PK   ||
        in->cFormat == IPL_YCrCb422_MB_PK   ||
        in->cFormat == IPL_YCbCr420_MB_PK   ||
        in->cFormat == IPL_YCrCb420_MB_PK)
    {
	    in->clrPtr = &ipl_temp_buffer[isize];
    }
    else
    {
      // init to NULL
	    in->clrPtr = NULL;
    }
  }
  else
  {
    // Even though we have buffer, it was not big enough to use  
    // therefore, lets sys_malloc
	  in->imgPtr = (uint8 *)ipl_mm_sys_malloc(isize, IPL_GROUP, __FILE__, __LINE__);

	  if (in->imgPtr == NULL)
    {
      MSG_LOW("ipl_malloc_img: sys_malloc failed!\n");
      return IPL_NO_MEMORY;
    }

	  //sys_malloc(isize);
	  //in->imgPtr = sys_malloc(isize);

	  if (in->imgPtr == NULL) 
      return IPL_NO_MEMORY;

    if (csize != 0)
    {
	    in->clrPtr = (uint8 *) ipl_mm_sys_malloc(csize,IPL_GROUP, __FILE__, __LINE__);
	    if (!in->clrPtr)
      {
        /* sys_free(in->imgPtr); */
        free(in->imgPtr);
        return IPL_NO_MEMORY;
      }
    }
    else
    {
	    in->clrPtr = NULL;
    }
  }

  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_free_img

DESCRIPTION
  This function is used to sys_free memory for a given image of type ipl_image_type

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   points to the input image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_free_img (ipl_image_type * in)
{
  MSG_LOW("ipl_free_img: sys_freeing bytes\n");

  // see if this guy used our temp buffer
  if (in->imgPtr == ipl_temp_buffer)
  {
    ipl_temp_buffer_inuse = 0;
    MSG_LOW("ipl_free_img: releasing use of temp buffer\n");
  }
  else
  {
	  if (in->imgPtr == NULL)
    {
      MSG_LOW("ipl_free_img: not going to sys_free NULL imgPtr\n");
    }
    else
    {
      MSG_LOW("ipl_free_img: sys_freeing imgPtr\n");
	    /* sys_free(in->imgPtr); */
	    free(in->imgPtr);
    }

    if (in->cFormat == IPL_YCbCr422_LINE_PK || 
        in->cFormat == IPL_YCrCb422_LINE_PK ||
        in->cFormat == IPL_YCbCr420_LINE_PK ||
        in->cFormat == IPL_YCrCb420_LINE_PK ||

        in->cFormat == IPL_YCbCr420_FRAME_PK ||
        in->cFormat == IPL_YCrCb420_FRAME_PK ||

        in->cFormat == IPL_YCbCr422_MB_PK   ||
        in->cFormat == IPL_YCrCb422_MB_PK   ||
        in->cFormat == IPL_YCbCr420_MB_PK   ||
        in->cFormat == IPL_YCrCb420_MB_PK)
    {
      if (in->clrPtr == NULL)
        MSG_LOW("ipl_free_img: not going to sys_free NULL clrPtr\n");
      else
      {
        MSG_LOW("ipl_free_img: releasing clrPtr\n");
	      ipl_mm_sys_free(in->clrPtr, __FILE__, __LINE__);
      }
    }
  }
  in->imgPtr = NULL;
  in->clrPtr = NULL;

  return IPL_SUCCESS;
}


#ifndef IPL_DEBUG_STANDALONE

// this function should not be included in AMMS builds
extern ipl_status_type
ipl_debug_write_tile(const ipl_image_type *in, const ipl_rect_type *crop, const char * str)
{
  // do nothing;
  return IPL_SUCCESS;
}

extern ipl_status_type
ipl_debug_read_tile(const ipl_image_type *in, const char * ifname)
{
  // do nothing;
  return IPL_SUCCESS;
}

#else

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_debug_write_tile

DESCRIPTION
  Write image to disk

DEPENDENCIES
  None

ARGUMENTS IN
  in            points to the input image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type
ipl_debug_write_tile(ipl_image_type *in, const ipl_rect_type *crop, const char * str)
{
  FILE * fpout;
  uint32 isize, csize;
  //char format[20];
  char * format;
  ipl_rect_type copy;
  ipl_image_type temp;
  char fname[200];

  if (str == NULL)
    return IPL_FAILURE;

  if (crop == NULL)
  {
    temp.dx = in->dx;
    temp.dy = in->dy;

    copy.x = 0;
    copy.y = 0;
    copy.dx = in->dx;
    copy.dy = in->dy;
  }
  else
  {
    temp.dx = crop->dx;
    temp.dy = crop->dy;

    copy.x = crop->x;
    copy.y = crop->y;
    copy.dx = crop->dx;
    copy.dy = crop->dy;
  }
  temp.cFormat = in->cFormat;

  //format = &extension[in->cFormat][0];
  format = "ycbcr222";

  if (ipl_malloc_img(&temp))
  {
    MSG_HIGH("Could not sys_malloc memory for temp image\n");
    return IPL_NO_MEMORY;
  }

  ipl_copy_and_paste(in, &temp, &copy, NULL);

  //if (crop == NULL)
  //  std_strlcpy(fname, str, sizeof(fname));
  //else
  //  sprintf(fname, "%s_%d,%d_%dx_%d.%s", str, (int)copy.x, (int)copy.y, (int)copy.dx, (int)copy.dy, format);

  // sprintf(fname, "%s_%dx_%d.%s", str, (int)copy.dx, (int)copy.dy, format);
 
  //  std_strlprintf(fname, "%s_%d,%d_%dx_%d.%s", str, (int)copy.x, (int)copy.y, (int)copy.dx, (int)copy.dy, format, sizeof(fname));

  if ((fpout = fopen(fname, "w")) == NULL)
  {
    fprintf(stderr, "Error opening output file %s\n", str);
    return IPL_FAILURE;
  }

  ipl_memory_needed(&temp, &isize, &csize);
  if (fwrite(temp.imgPtr, isize, 1, fpout) == 0)
  {
    fprintf(stderr, "Error writing to Output file\n");
    fclose(fpout);
    return IPL_FAILURE;
  }

  if (fwrite(temp.clrPtr, csize, 1, fpout) == 0)
  {
    fprintf(stderr, "Error writing chroma data to Output file\n");
    fclose(fpout);
    return IPL_FAILURE;
  }

  // clean up
  ipl_free_img(&temp);
  fclose(fpout);

  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_debug_read_tile

DESCRIPTION
  Read image from disk 

DEPENDENCIES
  None

ARGUMENTS IN
  in            points to the input image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type
ipl_debug_read_tile(const ipl_image_type *in, const char * ifname)
{
  uint32 isize, csize;
  FILE *fp;

  ipl_memory_needed(in, &isize, &csize);
  //system("pwd");
  fp = fopen (ifname, "rb");
  if (!fp)
  {
    printf("Error opening Input1 file %s\n", ifname);
    return IPL_FAILURE;
  }

  if (fread(in->imgPtr, isize, 1, fp) == 0)
  {
    printf("Error reading from Input1 file img %s\n", ifname);
    fclose(fp);
    return IPL_FAILURE;
  }

  if (csize)
  {
    if (fread(in->clrPtr, csize, 1, fp) == 0)
    {
      printf("Error reading from Input1 file clr %s\n", ifname);
      return IPL_FAILURE;
    }
  }
  fclose(fp);

  return IPL_SUCCESS;
}


#endif






#ifdef IPL_DEBUG_PROFILE


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_profile

DESCRIPTION
  This function is used to measure run time of various functions SURFS

DEPENDENCIES
  None

ARGUMENTS IN
  in      pointer to the input image
  out     pointer to the output image

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_profile(ipl_image_type * in, 
                                   ipl_image_type * out, 
                                   const ipl_rect_type  * icrop,
                                   const ipl_rect_type  * ocrop,
                                   const void * arg1,
                                   const void * arg2)
{
  ipl_status_type retval;
  int i, j, x, y;

  ipl_image_type lVGA_420;
  ipl_image_type lVGA_420fp;

  ipl_image_type lQVGA_565;
  ipl_image_type lQVGA_888;
  ipl_image_type lQVGA_YCC;
  ipl_image_type lQVGA_420;
  ipl_image_type lQVGA_422;
  ipl_image_type lQVGA_420fp;
  ipl_image_type pQVGA_565;
  ipl_image_type pQVGA_YCC;


  ipl_image_type lQCIF_565;
  ipl_image_type lQCIF_888;
  ipl_image_type lQCIF_YCC;
  ipl_image_type lQCIF_420;
  ipl_image_type lQCIF_422;
  ipl_image_type lQCIF_420fp;
  ipl_image_type pQCIF_565;
  ipl_image_type pQCIF_YCC;

  ipl_image_type l2MPa;
  ipl_image_type l2MPb;
  ipl_image_type l2MPc;


  ipl_image_type lTEST1;
  ipl_image_type lTEST2;
  ipl_image_type lTEST3;
  ipl_image_type lTEST4;

  ipl_rect_type REC_16x16; 
  ipl_rect_type REC_QVGA; 
  ipl_rect_type REC_QCIF; 
  ipl_rect_type loc;

  ipl_mchan_histogram_type regHist[10];

  ipl_upsize_type iiuptd;


  IPL2_MSG_FATAL("____________Inside IPL Profile_______________\n");

  // do not use ipls temp buffer for storting these images
  if (ipl_temp_buffer_inuse == 0)
    ipl_temp_buffer_inuse = 2;


  lVGA_420.dx = 640;
  lVGA_420.dy = 480;
  lVGA_420.cFormat = IPL_YCrCb420_LINE_PK;
  ipl_malloc_img(&lVGA_420);

  lVGA_420fp.dx = 640;
  lVGA_420fp.dy = 480;
  lVGA_420fp.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lVGA_420fp);



  lQVGA_565.dx = 320;
  lQVGA_565.dy = 240;
  lQVGA_565.cFormat = IPL_RGB565;
  ipl_malloc_img(&lQVGA_565);

  lQVGA_YCC.dx = 320;
  lQVGA_YCC.dy = 240;
  lQVGA_YCC.cFormat = IPL_YCbCr;
  ipl_malloc_img(&lQVGA_YCC);

  lQVGA_420.dx = 320;
  lQVGA_420.dy = 240;
  lQVGA_420.cFormat = IPL_YCrCb420_LINE_PK;
  ipl_malloc_img(&lQVGA_420);

  lQVGA_420fp.dx = 320;
  //lQVGA_420fp.dy = 240;
  lQVGA_420fp.dy = 180;
  lQVGA_420fp.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lQVGA_420fp);


  lQCIF_YCC.dx = 176;
  lQCIF_YCC.dy = 144;
  lQCIF_YCC.cFormat = IPL_YCbCr;
  ipl_malloc_img(&lQCIF_YCC);

  lQCIF_565.dx = 176;
  lQCIF_565.dy = 144;
  lQCIF_565.cFormat = IPL_RGB565;
  ipl_malloc_img(&lQCIF_565);

  lQCIF_420.dx = 176;
  lQCIF_420.dy = 144;
  lQCIF_420.cFormat = IPL_YCrCb420_LINE_PK;
  ipl_malloc_img(&lQCIF_420);

  lQCIF_420fp.dx = 176;
  lQCIF_420fp.dy = 144;
  lQCIF_420fp.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lQCIF_420fp);


#if 0
  // QVGA data
  // landscape

  lQVGA_888.dx = 320;
  lQVGA_888.dy = 240;
  lQVGA_888.cFormat = IPL_RGB888;
  ipl_malloc_img(&lQVGA_888);



  lQVGA_422.dx = 320;
  lQVGA_422.dy = 240;
  lQVGA_422.cFormat = IPL_YCbCr422_LINE_PK;
  ipl_malloc_img(&lQVGA_422);


  // portrait
  pQVGA_565.dx = 240;
  pQVGA_565.dy = 320;
  pQVGA_565.cFormat = IPL_RGB565;
  ipl_malloc_img(&pQVGA_565);

  pQVGA_YCC.dx = 240;
  pQVGA_YCC.dy = 320;
  pQVGA_YCC.cFormat = IPL_YCbCr;
  ipl_malloc_img(&pQVGA_YCC);



  // QCIF data
  // landscape

  lQCIF_888.dx = 176;
  lQCIF_888.dy = 144;
  lQCIF_888.cFormat = IPL_RGB888;
  ipl_malloc_img(&lQCIF_888);



  lQCIF_422.dx = 176;
  lQCIF_422.dy = 144;
  lQCIF_422.cFormat = IPL_YCbCr422_LINE_PK;
  ipl_malloc_img(&lQCIF_422);


  pQCIF_565.dx = 144;
  pQCIF_565.dy = 176;
  pQCIF_565.cFormat = IPL_RGB565;
  ipl_malloc_img(&pQCIF_565);

  pQCIF_YCC.dx = 144;
  pQCIF_YCC.dy = 176;
  pQCIF_YCC.cFormat = IPL_YCbCr;
  ipl_malloc_img(&pQCIF_YCC);


  // 2MP data
  l2MPa.dx = 1600;
  l2MPa.dy = 1200;
  l2MPa.cFormat = IPL_YCrCb420_LINE_PK;
  ipl_malloc_img(&l2MPa);

  l2MPb.dx = 1600;
  l2MPb.dy = 1200;
  l2MPb.cFormat = IPL_YCrCb420_LINE_PK;
  ipl_malloc_img(&l2MPb);

  l2MPc.dx = 1600;
  l2MPc.dy = 1200;
  l2MPc.cFormat = IPL_YCrCb420_LINE_PK;
  ipl_malloc_img(&l2MPc);
#endif


  // setup test output area
  // just pick any image format that will allocate memory off the
  // imgPtr and clrPtr. Make the dx and dy big enough to hold
  // rgb565 or rgb888 data off the imgPtr
  lTEST1.dx = 1000;
  lTEST1.dy = 1000;
  lTEST1.cFormat = IPL_YCrCb422_LINE_PK;
  ipl_malloc_img(&lTEST1);

  lTEST2.dx = 1000;
  lTEST2.dy = 1000;
  lTEST2.cFormat = IPL_YCrCb422_LINE_PK;
  ipl_malloc_img(&lTEST2);

  lTEST3.dx = 1024;
  lTEST3.dy = 768;
  lTEST3.cFormat = IPL_YCbCr;
  ipl_malloc_img(&lTEST3);

  lTEST4.dx = 1024;
  lTEST4.dy = 768;
  lTEST4.cFormat = IPL_YCbCr;
  ipl_malloc_img(&lTEST4);



  // setup crop
  REC_16x16.x = 0; 
  REC_16x16.y = 0; 
  REC_16x16.dx = 16; 
  REC_16x16.dy = 16; 

  REC_QVGA.x = 0; 
  REC_QVGA.y = 0; 
  REC_QVGA.dx = 320; 
  REC_QVGA.dy = 240; 

  REC_QCIF.x = 0; 
  REC_QCIF.y = 0; 
  REC_QCIF.dx = 176; 
  REC_QCIF.dy = 144; 


  iiuptd.crop.x = 0;
  iiuptd.crop.y = 0;
  iiuptd.crop.dx = 320;
  iiuptd.crop.dy = 240;
  iiuptd.qual = IPL_QUALITY_MEDIUM;


  // now let other applications use ipls temp buffer if needed
  if (ipl_temp_buffer_inuse == 2)
    ipl_temp_buffer_inuse = 0;


#if 0
  // Input is QVGA 320,240 YCbCr 
  //ipl_copy_and_paste(in, &lQVGA_YCC, NULL, NULL);
  if (ipl_crop_resize_rot(in, &lQVGA_YCC, NULL, NULL, IPL_NOROT,IPL_QUALITY_MEDIUM))
  {
    IPL2_MSG_FATAL("################# FAILED 0 0 0 0 #############\n");
    return IPL_FAILURE;
  }
  
  if (ipl_copy_and_paste(&lQVGA_YCC, &lQCIF_YCC, &REC_QCIF, NULL)) 
  {
    IPL2_MSG_FATAL("################# FAILED 0 0 0 0 #############\n");
    return IPL_FAILURE;
  }
#elif 0
  // Input is QCIF YCbCr 
  //ipl_copy_and_paste(in, &lQCIF_YCC, NULL, NULL);
  //ipl_convert_image(in, &lQCIF_420);
  ipl_convert_image(in, &lQVGA_420);
  //ipl_debug_write_tile(&lQCIF_420, NULL, "lQCIF420_176x_144.ycrcb420lp");
  
#else
  ipl_copy_and_paste(in, &lQVGA_420fp, NULL, NULL);
#endif



  // convert to popular formats

  //ipl_convert_image(&lQVGA_YCC, &lQVGA_420);
  //ipl_convert_image(&lQVGA_420, &lQVGA_565);
  //ipl_convert_image(&lQVGA_420, &lQVGA_420fp);

  //ipl_convert_image(&lQCIF_YCC, &lQCIF_420);
  //ipl_convert_image(&lQCIF_YCC, &lQCIF_565);
  //ipl_convert_image(&lQCIF_420, &lQCIF_420fp);
  //ipl_convert_image(&lQVGA_420, &lQVGA_420fp);
  
  //ipl_debug_write_tile(&lQCIF_420fp, NULL, "lQCIF420_176x_144.ycrcb420fp");
  //ipl_debug_write_tile(&lQVGA_420fp, NULL, "lQVGA420fp_320x_240.ycrcb420fp");
  //ipl_debug_write_tile(&lQVGA_420fp, NULL, "lQVGA420fp_320x_180.ycrcb420fp");


  IPL2_MSG_FATAL("### \n");
  //IPL2_MSG_FATAL("### ###\n");

  IPL2_MSG_FATAL("__________ Start IPL Profiling ___________\n");


#if 0
  //
  // COLOR CONVERT
  //
  IPL2_MSG_FATAL("### COLOR_CONVERT \n");


  // QVGA 
  // YCC in
  if (ipl_convert_image(&lQVGA_YCC, &lQVGA_565)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA YCC to QVGA 565\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA YCC to QVGA 565 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQVGA_YCC, &lQVGA_420)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA YCC to QVGA 420 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA YCC to QVGA 420 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQVGA_YCC, &lQVGA_422)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA YCC to QVGA 422 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA YCC to QVGA 422 0 0 0 0 ###\n");
  }
 
  if (ipl_convert_image(&lQVGA_YCC, &lQVGA_888)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA YCC to QVGA 888\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA YCC to QVGA 888 0 0 0 ###\n");
  }



  // RGB in
  if (ipl_convert_image(&lQVGA_565, &lQVGA_YCC)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA 565 to QVGA YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA 565 to QVGA YCC 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQVGA_565, &lQVGA_420)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA 565 to QVGA 420 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA 565 to QVGA 420 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQVGA_565, &lQVGA_422)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA 565 to QVGA 422 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA 565 to QVGA 422 0 0 0 0 ###\n");
  }


  // 420 in
  if (ipl_convert_image(&lQVGA_420, &lQVGA_565)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA 420 to QVGA 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA 420 to QVGA 565 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQVGA_420, &lQVGA_YCC)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA 420 to QVGA YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA 420 to QVGA YCC 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQVGA_420, &lQVGA_422)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA 420 to QVGA 422 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA 420 to QVGA 422 0 0 0 0 ###\n");
  }


  // 422 in
  if (ipl_convert_image(&lQVGA_422, &lQVGA_565)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA 422 to QVGA 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA 422 to QVGA 565 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQVGA_422, &lQVGA_YCC)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA 422 to QVGA YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA 422 to QVGA YCC 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQVGA_422, &lQVGA_420)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QVGA 422 to QVGA 420 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QVGA 422 to QVGA 420 0 0 0 0 ###\n");
  }


  // QCIF 
  // YCC in
  if (ipl_convert_image(&lQCIF_YCC, &lQCIF_565)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF YCC to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF YCC to QCIF 565 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_YCC, &lQCIF_420)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF YCC to QCIF 420 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF YCC to QCIF 420 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_YCC, &lQCIF_422)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF YCC to QCIF 422 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF YCC to QCIF 422 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_YCC, &lQCIF_888)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF YCC to QCIF 888\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF YCC to QCIF 888 0 0 0 ###\n");
  }



  // RGB in
  if (ipl_convert_image(&lQCIF_565, &lQCIF_YCC)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 565 to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 565 to QCIF YCC 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_565, &lQCIF_420)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 565 to QCIF 420 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 565 to QCIF 420 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_565, &lQCIF_422)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 565 to QCIF 422 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 565 to QCIF 422 0 0 0 0 ###\n");
  }


  // 420 in
  if (ipl_convert_image(&lQCIF_420, &lQCIF_565)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 420 to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 420 to QCIF 565 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_420, &lQCIF_420fp)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 420 to QCIF 420fp\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 420 to QCIF 420fp 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_420, &lQCIF_YCC)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 420 to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 420 to QCIF YCC 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_420, &lQCIF_422)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 420 to QCIF 422 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 420 to QCIF 422 0 0 0 0 ###\n");
  }


  // 422 in
  if (ipl_convert_image(&lQCIF_422, &lQCIF_565)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 422 to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 422 to QCIF 565 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_422, &lQCIF_YCC)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 422 to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 422 to QCIF YCC 0 0 0 0 ###\n");
  }

  if (ipl_convert_image(&lQCIF_422, &lQCIF_420)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_image QCIF 422 to QCIF 420 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_image QCIF 422 to QCIF 420 0 0 0 0 ###\n");
  }
#endif




  // 
  // RESIZE 
  //
#if 0
  IPL2_MSG_FATAL("### RESIZE \n");
  
  // QVGA to QCIF
  if (ipl_downsize(&lQVGA_565, &lQCIF_565, NULL))
  {
    IPL2_MSG_FATAL("NOTSUP ipl_downsize QVGA 565 to QCIF 565 \n");
  }
  else
  {
   IPL2_MSG_FATAL("### ipl_downsize QVGA 565 to QCIF 565 0 0 0 0 ###\n");
  }

  if (ipl_downsize(&lQVGA_YCC, &lQCIF_YCC, NULL))
  {
    IPL2_MSG_FATAL("NOTSUP ipl_downsize QVGA YCC to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_downsize QVGA YCC to QCIF YCC 0 0 0 0 ###\n");
  }
  
  if (ipl_downsize(&lQVGA_420, &lQCIF_420, NULL))
  {
    IPL2_MSG_FATAL("NOTSUP ipl_downsize QVGA 420 to QCIF 420 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_downsize QVGA 420 to QCIF 420 0 0 0 0 ###\n");
  }


  if (ipl_downsize(&lQVGA_422, &lQCIF_422, NULL)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_downsize QVGA 422 to QCIF 422 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_downsize QVGA 422 to QCIF 422 0 0 0 0 ###\n");
  }


  // QCIF to QVGA
  if (ipl_upsize(&lQCIF_565, &lQVGA_565, 0, 0))
  {
    IPL2_MSG_FATAL("NOTSUP ipl_upsize QCIF 565 to QVGA 565 0 0 0 0 ### \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_upsize QCIF 565 to QVGA 565 0 0 0 0 ###\n");
  }

  if (ipl_upsize(&lQCIF_YCC, &lQVGA_YCC, 0, 0))
  {
    IPL2_MSG_FATAL("NOTSUP ipl_upsize QCIF YCC to QVGA YCC 0 0 0 0 ###\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_upsize QCIF YCC to QVGA YCC 0 0 0 0 ###\n");
  }
#endif
  



  // 
  // DOWNSIZE AND COLOR CONVERT 
  //
#if 0
  {
    IPL2_MSG_FATAL("### DOWNSIZE_COLOR_CONVERT \n");
  }

  // from YCC
  if (ipl_downsize(&lQVGA_YCC, &lQCIF_565, NULL)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_downsize QVGA YCC to QCIF 565 \n");
  }
  else
  {
   IPL2_MSG_FATAL("### ipl_downsize QVGA YCC to QCIF 565 0 0 0 0 ###\n");
  }


  // from 420 
  if (ipl_downsize(&lQVGA_420, &lQCIF_565, NULL)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_downsize QVGA 420 to QCIF 565 \n");
  }
  else
  {
   IPL2_MSG_FATAL("### ipl_downsize QVGA 420 to QCIF 565 0 0 0 0 ###\n");
  }


  // 
  // DOWNSIZE AND COLOR CONVERT: METHOD 2 
  //

  // from YCC
  if (ipl_crop_resize_rot(&lQVGA_YCC, &lQCIF_565, NULL, NULL, 0, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QVGA YCC to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_crop_resize_rot QVGA YCC to QCIF 565 0 0 0 L ###\n");
  }


  // from 420 
  if (ipl_crop_resize_rot(&lQVGA_420, &lQCIF_565, NULL, NULL, 0, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QVGA 420 to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_crop_resize_rot QVGA 420 to QCIF 565 0 0 0 L ###\n");
  }


  // from 422 
  if (ipl_crop_resize_rot(&lQVGA_422, &lQCIF_565, NULL, NULL, 0, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QVGA 422 to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_crop_resize_rot QVGA 422 to QCIF 565 0 0 0 L ###\n");
  }
#endif






  // 
  // UPSIZE AND COLOR CONVERT 
  //
  IPL2_MSG_FATAL("### UPSIZE_COLOR_CONVERT \n");

  // new function
#if 0
  lTEST1.dx = 240;
  lTEST1.dy = 190;
  lTEST1.cFormat = IPL_RGB565;

  ipl_copy_and_paste(in, &lQCIF_420, NULL, NULL); 
  //ipl_debug_write_tile(&lQCIF_420, NULL, "lQCIF420_176x_144.ycrcb420lp");

  IPL2_MSG_FATAL("\n### ipl_crr QCIF 420 to lTEST1 565 0 0 0 L ###\n");
  if (ipl_crop_resize_rot(&lQCIF_420, &lTEST1, NULL, NULL, 0,IPL_QUALITY_LOW)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_crr QCIF 420 to lTEST1 565 0 0 0 L ###\n");
  }
  IPL2_MSG_FATAL("### ipl_crr QCIF 420 to lTEST1 565 0 0 0 L ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1crrL_240x_190.rgb");


  IPL2_MSG_FATAL("\n### ipl_crr QCIF 420 to lTEST1 565 0 0 0 M ###\n");
  if (ipl_crop_resize_rot(&lQCIF_420,&lTEST1,NULL,NULL,0,IPL_QUALITY_MEDIUM)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_crr QCIF 420 to lTEST1 565 0 0 0 M ###\n");
  }
  IPL2_MSG_FATAL("### ipl_crr QCIF 420 to lTEST1 565 0 0 0 M ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1crrM_240x_190.rgb");


  IPL2_MSG_FATAL("\n### ipl_crr QCIF 420 to lTEST1 565 0 0 0 H ###\n");
  if (ipl_crop_resize_rot(&lQCIF_420, &lTEST1, NULL, NULL, 0, IPL_QUALITY_HIGH)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_crr QCIF 420 to lTEST1 565 0 0 0 H ###\n");
  }
  IPL2_MSG_FATAL("### ipl_crr QCIF 420 to lTEST1 565 0 0 0 H ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1crrH_240x_190.rgb");



  
  IPL2_MSG_FATAL("\n### ipl_upsize QCIF 420 to lTEST1 565 0 0 0 L ###\n");
  if (ipl_upsize(&lQCIF_420, &lTEST1, NULL, IPL_QUALITY_LOW)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_upsize QCIF 420 to lTEST1 565 0 0 0 L ###\n");
  }
  IPL2_MSG_FATAL("### ipl_upsize QCIF 420 to lTEST1 565 0 0 0 L ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1upsizeL_240x_190.rgb");


  IPL2_MSG_FATAL("\n### ipl_upsize QCIF 420 to lTEST1 565 0 0 0 M ###\n");
  if (ipl_upsize(&lQCIF_420, &lTEST1, NULL, IPL_QUALITY_MEDIUM)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_upsize QCIF 420 to lTEST1 565 0 0 0 M ###\n");
  }
  IPL2_MSG_FATAL("### ipl_upsize QCIF 420 to lTEST1 565 0 0 0 M ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1upsizeM_240x_190.rgb");


  IPL2_MSG_FATAL("\n### ipl_upsize QCIF 420 to lTEST1 565 0 0 0 H ###\n");
  if (ipl_upsize(&lQCIF_420, &lTEST1, NULL, IPL_QUALITY_HIGH)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_upsize QCIF 420 to lTEST1 565 0 0 0 H ###\n");
  }
  IPL2_MSG_FATAL("### ipl_upsize QCIF 420 to lTEST1 565 0 0 0 H ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1upsizeH_240x_190.rgb");
#endif



  
#if 0
  IPL2_MSG_FATAL("### UPSIZE_COLOR_CONVERT \n");

  // new function
  if (ipl_upSize_qcif2qvga_YCrCb420lpToRGB(&lQCIF_420, &lQVGA_565, &iiuptd, 0)) 
  {
   IPL2_MSG_FATAL("NOTSUP custom QCIF 420 to QVGA565 0 0 0 M ###\n");
  }
  else
  {
    IPL2_MSG_FATAL("### custom QCIF 420 to QVGA565 0 0 0 M ###\n");
  }
  IPL2_MSG_FATAL("### custom QCIF 420 to QVGA565 0 0 0 M ###\n");
  //ipl_debug_write_tile(&lQVGA_565, NULL, "lQVGA565new_320x_240.rgb");
#endif

#if 0
  // 420 in
  for (i = 0; i < 1; i++)
  {
    if (ipl_crop_resize_rot(&lQCIF_420, &lQVGA_565, NULL, NULL, 0, 1)) 
    {
      IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QCIF 420 to QVGA 565 0 0 0 L ###\n");
    }
    else
    {
      IPL2_MSG_FATAL("### ipl_crop_resize_rot QCIF 420 to QVGA 565 0 0 0 L ###\n");
    }
  }
  IPL2_MSG_FATAL("### ipl_crop_resize_rot QCIF 420 to QVGA 565 0 0 0 L ###\n");
  //ipl_debug_write_tile(&lQVGA_565, NULL, "lQVGA565crr_320x_240.rgb");


  if (ipl_upsize(&lQCIF_420, &lQVGA_565, NULL, 1)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_upsize QCIF 420 to QVGA 565 0 0 0 M ###\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_upsize QCIF 420 to QVGA 565 0 0 0 M ###\n");
  }
  IPL2_MSG_FATAL("### ipl_upsize QCIF 420 to QVGA 565 0 0 0 M ###\n");
  //ipl_debug_write_tile(&lQVGA_565, NULL, "lQVGA565up_320x_240.rgb");








  if (ipl_crop_resize_rot(&lQCIF_420, &lQVGA_565, NULL, NULL, 0, 1)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QCIF 420 to QVGA 565 0 0 0 M ###\n");
  }
  else
  {
   IPL2_MSG_FATAL("### ipl_crop_resize_rot QCIF 420 to QVGA 565 0 0 0 M ###\n");
  }

  if (ipl_crop_resize_rot(&lQCIF_420, &lQVGA_565, NULL, NULL, 0, 2)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QCIF 420 to QVGA 565 0 0 0 H ###\n");
  }
  else
  {
   IPL2_MSG_FATAL("### ipl_crop_resize_rot QCIF 420 to QVGA 565 0 0 0 H ###\n");
  }

  // RGB in
  if (ipl_crop_resize_rot(&lQCIF_565, &lQVGA_YCC, NULL, NULL, 0, 0)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QCIF 565 to QVGA YCC 0 0 0 L ###\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_crop_resize_rot QCIF 565 to QVGA YCC 0 0 0 L ###\n");
  }

  // YCC in
  if (ipl_crop_resize_rot(&lQCIF_YCC, &lQVGA_565, NULL, NULL, 0, 0)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QCIF YCC to QVGA 565 0 0 0 L ###\n");
  }
  else
  {
   IPL2_MSG_FATAL("### ipl_crop_resize_rot QCIF YCC to QVGA 565 0 0 0 L ###\n");
  }

  if (ipl_crop_resize_rot(&lQCIF_YCC, &lQVGA_565, NULL, NULL, 0, 1)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QCIF YCC to QVGA 565 0 0 0 M ###\n");
  }
  else
  {
   IPL2_MSG_FATAL("### ipl_crop_resize_rot QCIF YCC to QVGA 565 0 0 0 M ###\n");
  }


  // 420 frame pack 
  if (ipl_crop_resize_rot(&lQCIF_420fp, &lQVGA_565, NULL, NULL, 0, 1)) 
  {
   IPL2_MSG_FATAL("NOTSUP ipl_crop_resize_rot QCIF 420fp to QVGA 565 0 0 0 M ###\n");
  }
  else
  {
   IPL2_MSG_FATAL("### ipl_crop_resize_rot QCIF 420fp to QVGA 565 0 0 0 M ###\n");
  }
#endif



  // 
  // Rotate
  //
#if 0 
  IPL2_MSG_FATAL("### ROTATION \n");

  if (ipl_rotate(&lQCIF_YCC, &pQCIF_YCC, 90, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rotate QCIF YCC to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rotate QCIF YCC to QCIF YCC 0 0 90 0 ###\n");
  }

  if (ipl_rotate(&lQCIF_YCC, &lQCIF_YCC, 180, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rotate QCIF YCC to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rotate QCIF YCC to QCIF YCC 0 0 180 0 ###\n");
  }

  if (ipl_rotate(&lQCIF_YCC, &lQCIF_YCC, 74, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rotate QCIF YCC to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rotate QCIF YCC to QCIF YCC 0 0 74 0 ###\n");
  }



  if (ipl_rotate(&lQCIF_565, &pQCIF_565, 90, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rotate QCIF 565 to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rotate QCIF 565 to QCIF 565 0 0 90 0 ###\n");
  }

  if (ipl_rotate(&lQCIF_565, &lQCIF_565, 180, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rotate QCIF 565 to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rotate QCIF 565 to QCIF 565 0 0 180 0 ###\n");
  }

  if (ipl_rotate(&lQCIF_565, &lQCIF_565, 74, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rotate QCIF 565 to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rotate QCIF 565 to QCIF 565 0 0 74 0 ###\n");
  }
#endif


  // 
  // Reflection
  //
#if 0
  IPL2_MSG_FATAL("### REFLECTION \n");

  if (ipl_reflect(&lQCIF_YCC, &lQCIF_YCC, 1)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_reflect QCIF YCC to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_reflect QCIF YCC to QCIF YCC 0 H 0 0 ###\n");
  }

  if (ipl_reflect(&lQCIF_YCC, &lQCIF_YCC, 2)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_reflect QCIF YCC to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_reflect QCIF YCC to QCIF YCC 0 V 0 0 ###\n");
  }

  if (ipl_reflect(&lQCIF_565, &lQCIF_565, 1)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_reflect QCIF 565 to QCIF 565\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_reflect QCIF 565 to QCIF 565 0 H 0 0 ###\n");
  }

  if (ipl_reflect(&lQCIF_565, &lQCIF_565, 2)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_reflect QCIF 565 to QCIF 565\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_reflect QCIF 565 to QCIF 565 0 V 0 0 ###\n");
  }
#endif



  // 
  // Rot Add Crop 
  //
#if 0
  IPL2_MSG_FATAL("### ROT_ADD_CROP \n");

  if (ipl_rot_add_crop(&lQCIF_YCC, &lQCIF_565, &lQCIF_YCC, NULL, 0, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF YCC to QCIF YCC\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF YCC to QCIF YCC 1 0 0 0 ###\n");
  }

  if (ipl_rot_add_crop(&lQCIF_YCC, &lQCIF_565, &lQCIF_565, NULL, 0, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF YCC to QCIF 565\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF YCC to QCIF 565 1 0 0 0 ###\n");
  }


  if (ipl_rot_add_crop(&lQCIF_565, &lQCIF_565, &lQCIF_565, NULL, 0, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF 565 to QCIF 565\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF 565 to QCIF 565 1 0 0 0 ###\n");
  }

  if (ipl_rot_add_crop(&lQCIF_565, &lQCIF_565, &lQCIF_YCC, NULL, 0, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF 565 to QCIF YCC\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF 565 to QCIF YCC 1 0 0 0 ###\n");
  }

  // with rotation
  if (ipl_rot_add_crop(&lQCIF_YCC, &pQCIF_565, &pQCIF_YCC, NULL, 1, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF YCC to QCIF YCC\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF YCC to QCIF YCC 1 0 90 0 ###\n");
  }

  if (ipl_rot_add_crop(&lQCIF_YCC, &pQCIF_565, &pQCIF_565, NULL, 1, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF YCC to QCIF 565\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF YCC to QCIF 565 1 0 90 0 ###\n");
  }


  if (ipl_rot_add_crop(&lQCIF_565, &pQCIF_565, &pQCIF_565, NULL, 1, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF 565 to QCIF 565\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF 565 to QCIF 565 1 0 90 0 ###\n");
  }

  if (ipl_rot_add_crop(&lQCIF_565, &pQCIF_565, &pQCIF_YCC, NULL, 1, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF 565 to QCIF YCC\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF 565 to QCIF YCC 1 0 90 0 ###\n");
  }
#endif



  // 
  // Copy and Paste 
  //
#if 0
  IPL2_MSG_FATAL("### COPY_PASTE \n");

  // 565
  if (ipl_copy_and_paste(&lQCIF_565, &lQCIF_565, NULL, NULL)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_copy_and_paste QCIF 565 to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_copy_and_paste QCIF 565 to QCIF 565 0 0 0 0 ###\n");
  }

  // YCC
  if (ipl_copy_and_paste(&lQCIF_YCC, &lQCIF_YCC, NULL, NULL)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_copy_and_paste QCIF YCC to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_copy_and_paste QCIF YCC to QCIF YCC 0 0 0 0 ###\n");
  }

  // 420 
  if (ipl_copy_and_paste(&lQCIF_420, &lQCIF_420, NULL, NULL)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_copy_and_paste QCIF 420 to QCIF 420 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_copy_and_paste QCIF 420 to QCIF 420 0 0 0 0 ###\n");
  }
#endif



  // 
  // Add frames 
  //
#if 0
  IPL2_MSG_FATAL("### ADD_FRAMES \n");

  if (ipl_rot_add_crop(&lQCIF_565, &lQCIF_565, &lQCIF_565, NULL, 0, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF 565 to QCIF 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF 565 to QCIF 565 1 0 0 0 ###\n");
  }

  if (ipl_rot_add_crop(&lQCIF_YCC, &lQCIF_565, &lQCIF_YCC, NULL, 0, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_rot_add_crop QCIF YCC to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_rot_add_crop QCIF YCC to QCIF YCC 1 0 0 0 ###\n");
  }

  if (ipl_image_add(&lQCIF_YCC, &lQCIF_565, 0, &lQCIF_YCC)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_image_add QCIF YCC to QCIF YCC \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_image_add QCIF YCC to QCIF YCC 1 0 0 0 ###\n");
  }





  if (ipl_image_add_inplace(&lQCIF_420, &lQCIF_565, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_image_add_inplace QCIF 420 to QCIF 420\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_image_add_inplace QCIF 420 to QCIF 420 1 0 0 0 ###\n");
  }


  if (ipl_image_add_inplace(&lQVGA_420, &lQVGA_565, NULL, 0)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_image_add_inplace QVGA 420 to QVGA 420\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_image_add_inplace QVGA 420 to QVGA 420 1 0 0 0 ###\n");
  }
  //ipl_debug_write_tile(&lQVGA_420, NULL, "lQVGA420inplace_320x_240.ycrcb420lp");


  if (ipl_alpha_blend_area(&lQVGA_420, &lQVGA_420, &REC_QVGA, 64, 0xF81F)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_alpha_blend_area QVGA 420 to QVGA 420\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_alpha_blend_area QVGA 420 to QVGA 420 420 1 0 0 0 ###\n");
  }
  //ipl_debug_write_tile(&lQVGA_420, NULL, "lQVGA420blend_320x_240.ycrcb420lp");
#endif


  // 
  // HJR
  //
 
#if 0
  ipl_image_type * imgHolder[10];

  imgHolder[0] = &l2MPa;
  imgHolder[1] = &l2MPb;
  imgHolder[2] = &l2MPc;
  imgHolder[3] = NULL;
  if (ipl_hjr(imgHolder, NULL, 28, 28))
  {
    IPL2_MSG_FATAL("NOTSUP ipl_hjr 2MP 420 to 2MP 420 0 0 0 0 ###\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_hjr 2MP 420 to 2MP 420 0 0 0 0 ###\n");
  }

#endif


  // 
  // Luma Adaptation 
  //

#if 0
  // luma adaptation
  if (ipl_efx_LumaAdaptation(&l2MPa, NULL, NULL, 3))
  //if (ipl_luma_adaptation(&l2MPa, NULL, NULL, 3))
  {
    IPL2_MSG_FATAL("NOTSUP ipl_luma_adaptation 2MP 420 to 2MP 420 0 0 0 0 ###\n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_luma_adapation 2MP 420 to 2MP 420 0 0 0 0 ###\n");
  }
#endif



  // 
  // Misc tests
  //



#if 0
  if (ipl_convert_ycrcb420lp_to_rgb565(&lQVGA_420, &lQVGA_565)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_ycrcb420 QVGA 420 to QVGA 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_ycrcb420 QVGA 420 to QVGA 565 0 0 0 0 ###\n");
  }

  if (ipl2_convert_ycrcb420lp_to_rgb565(&lQVGA_420, &lQVGA_565)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl2_convert_ycrcb420 QVGA 420 to QVGA 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl2_convert_ycrcb420 QVGA 420 to QVGA 565 0 0 0 0 ###\n");
  }

  if (ipl_convert_ycrcb420lp_to_rgb565_fast(&lQVGA_420, &lQVGA_565)) 
  {
    IPL2_MSG_FATAL("NOTSUP ipl_convert_ycrcb420_fast QVGA 420 to QVGA 565 \n");
  }
  else
  {
    IPL2_MSG_FATAL("### ipl_convert_ycrcb420_fast QVGA 420 to QVGA 565 0 0 0 0 ###\n");
  }
#endif


  
  IPL2_MSG_FATAL("### MISC TESTS\n");

  // ipl_crop_resize_rot timing for Nish 
  // going form 176x144 to 1.25x bigger, and rotating, and color converting.
  
  
  //ipl_debug_write_tile(&lQVGA_YCC, NULL, "lQVGAYCC_320x_240.ycbcr");
  //ipl_debug_write_tile(&lQCIF_YCC, NULL, "lQCIFYCC_176x_144.ycbcr");
  //ipl_debug_write_tile(&lQCIF_420, NULL, "lQCIF420_176x_144.ycrcb420lp");

#if 1 
  // test andy's upsize

#if 0
  lTEST1.dx = 320;
  lTEST1.dy = 240;
  lTEST1.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lTEST1);

  IPL2_MSG_FATAL("### TEST 1 fp to fp upsize and filer ###\n");

  // 4ms
  memset(lTEST1.imgPtr, 0, 320*240);
  memset(lTEST1.clrPtr, 0, 320*240/4);
  ipl_resizeFilter(&lQCIF_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_LOW,0,32);
  IPL2_MSG_FATAL("### TEST 2 fp to fp upsize and filer ###\n");
  ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_OL_320x_240.ycbcr420fp");

  // 19ms
  memset(lTEST1.imgPtr, 0, 320*240);
  memset(lTEST1.clrPtr, 0, 320*240/4);
  ipl_resizeFilter(&lQCIF_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 3 fp to fp upsize and filer ###\n");
  ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_OM_320x_240.ycbcr420fp");

  // 7ms
  memset(lTEST1.imgPtr, 0, 320*240);
  memset(lTEST1.clrPtr, 0, 320*240/4);
  ipl_resizeFilter(&lQCIF_420fp, &lTEST1,IPL_QUALITY_LOW,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 4 fp to fp upsize and filer ###\n");
  ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_LO_320x_240.ycbcr420fp");

  // 39ms
  memset(lTEST1.imgPtr, 0, 320*240);
  memset(lTEST1.clrPtr, 0, 320*240/4);
  ipl_resizeFilter(&lQCIF_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 5 fp to fp upsize and filer ###\n");
  ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_MO_320x_240.ycbcr420fp");


  // 10ms
  memset(lTEST1.imgPtr, 0, 320*240);
  memset(lTEST1.clrPtr, 0, 320*240/4);
  ipl_resizeFilter(&lQCIF_420fp, &lTEST1,IPL_QUALITY_LOW,IPL_QUALITY_LOW,0,32);
  IPL2_MSG_FATAL("### TEST 6 fp to fp upsize and filer ###\n");
  ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_LL_320x_240.ycbcr420fp");

  // 26ms
  memset(lTEST1.imgPtr, 0, 320*240);
  memset(lTEST1.clrPtr, 0, 320*240/4);
  ipl_resizeFilter(&lQCIF_420fp, &lTEST1,IPL_QUALITY_LOW,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 7 fp to fp upsize and filer ###\n");
  ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_LM_320x_240.ycbcr420fp");

  // 43ms
  memset(lTEST1.imgPtr, 0, 320*240);
  memset(lTEST1.clrPtr, 0, 320*240/4);
  ipl_resizeFilter(&lQCIF_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_LOW,0,32);
  IPL2_MSG_FATAL("### TEST 8 fp to fp upsize and filer ###\n");
  ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_ML_320x_240.ycbcr420fp");

  // 57ms
  memset(lTEST1.imgPtr, 0, 320*240);
  memset(lTEST1.clrPtr, 0, 320*240/4);
  ipl_resizeFilter(&lQCIF_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 9 fp to fp upsize and filer ###\n");
  ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_MM_320x_240.ycbcr420fp");

  // 57ms
  memset(lTEST1.imgPtr, 0, 320*240);
  memset(lTEST1.clrPtr, 0, 320*240/4);
  ipl_resizeFilter(&lQCIF_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_MAX,0,16);
  IPL2_MSG_FATAL("### TEST 10 fp to fp upsize and filer ###\n");
  ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_MM_16p_320x_240.ycbcr420fp");
#endif



#if 1
  lTEST1.dx = 400;
  lTEST1.dy = 240;
  lTEST1.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lTEST1);

  // ms
  //memset(lTEST1.imgPtr, 0, 400*240);
  //memset(lTEST1.clrPtr, 0, 400*240/2);
  retval = ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_LOW,0,32);
  IPL2_MSG_FATAL("### TEST 2 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_OL_400x_240.ycbcr420fp");

  // ms
  //memset(lTEST1.imgPtr, 0, 400*240);
  //memset(lTEST1.clrPtr, 0, 400*240/2);
  retval = ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 3 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_OM_400x_240.ycbcr420fp");

  // ms
  //memset(lTEST1.imgPtr, 0, 400*240);
  //memset(lTEST1.clrPtr, 0, 400*240/2);
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_LOW,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 4 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_LO_400x_240.ycbcr420fp");

  // ms
  //memset(lTEST1.imgPtr, 0, 400*240);
  //memset(lTEST1.clrPtr, 0, 400*240/2);
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 5 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_MO_400x_240.ycbcr420fp");

  // ms
  //memset(lTEST1.imgPtr, 0, 400*240);
  //memset(lTEST1.clrPtr, 0, 400*240/2);
  retval = ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 5a fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1_MM_400x_240.ycbcr420fp");



  lTEST1.dx = 240;
  lTEST1.dy = 180;
  lTEST1.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lTEST1);

  // 4ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_LOW,0,32);
  IPL2_MSG_FATAL("### TEST 6 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST2_OL_240x_180.ycbcr420fp");

  // 19ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 7 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST2_OM_240x_180.ycbcr420fp");

  // 7ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_LOW,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 8 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST2_LO_240x_180.ycbcr420fp");

  // 39ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 9 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST2_MO_240x_180.ycbcr420fp");

  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 9a fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST2_MM_240x_180.ycbcr420fp");
#endif

#if 1

  lQVGA_420fp.dx = 320;
  lQVGA_420fp.dy = 180;
  lQVGA_420fp.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lQVGA_420fp);

  lTEST1.dx = 400;
  lTEST1.dy = 180;
  lTEST1.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lTEST1);

  // 4ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_LOW,0,32);
  IPL2_MSG_FATAL("### TEST 10 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST3_OL_400x_180.ycbcr420fp");

  // 19ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 11 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST3_OM_400x_180.ycbcr420fp");

  // 7ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_LOW,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 12 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST3_LO_400x_180.ycbcr420fp");

  // 39ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 13 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST3_MO_400x_180.ycbcr420fp");

  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 13a fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST3_MM_400x_180.ycbcr420fp");




  lTEST1.dx = 400;
  lTEST1.dy = 240;
  lTEST1.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lTEST1);

  // 4ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_LOW,0,32);
  IPL2_MSG_FATAL("### TEST 14 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST4_OL_400x_240.ycbcr420fp");

  // 19ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 15 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST4_OM_400x_240.ycbcr420fp");

  // 7ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_LOW,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 16 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST4_LO_400x_240.ycbcr420fp");

  // 39ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 17 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST4_MO_400x_240.ycbcr420fp");

  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 17a fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST4_MM_400x_240.ycbcr420fp");


  lTEST1.dx = 240;
  lTEST1.dy = 136;
  lTEST1.cFormat = IPL_YCbCr420_FRAME_PK;
  ipl_malloc_img(&lTEST1);

  // 4ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_LOW,0,32);
  IPL2_MSG_FATAL("### TEST 18 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST5_OL_240x_136.ycbcr420fp");

  // 19ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_OFF,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 19 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST5_OM_240x_136.ycbcr420fp");

  // 7ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_LOW,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 20 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST5_LO_240x_136.ycbcr420fp");

  // 39ms
  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_OFF,0,32);
  IPL2_MSG_FATAL("### TEST 21 fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST5_MO_240x_136.ycbcr420fp");

  ipl_resizeFilter(&lQVGA_420fp, &lTEST1,IPL_QUALITY_MAX,IPL_QUALITY_MAX,0,32);
  IPL2_MSG_FATAL("### TEST 21a fp to fp upsize and filer ###\n");
  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST5_MM_240x_136.ycbcr420fp");
#endif


#endif


  
#if 0 
  // upsize and no color convert
  IPL2_MSG_FATAL("### TEST 1 lp to lp, no rot, up 1.25x###\n");
  lTEST1.dx = 220;
  lTEST1.dy = 180;
  lTEST1.cFormat = IPL_YCrCb420_LINE_PK;
  ipl_crop_resize_rot(&lQCIF_420, &lTEST1, NULL, NULL, IPL_NOROT, IPL_QUALITY_MEDIUM);
  //ipl_debug_write_tile(&lTEST1,    NULL, "lTEST1_220x_180.ycrcb420lp");

  // upsize and color convert
  IPL2_MSG_FATAL("### TEST 2 lp to rgb, no rot, up 1.25x###\n");
  lTEST2.dx = 220;
  lTEST2.dy = 180;
  lTEST2.cFormat = IPL_RGB565;
  ipl_crop_resize_rot(&lQCIF_420, &lTEST2, NULL, NULL, IPL_NOROT, IPL_QUALITY_MEDIUM);
  //ipl_debug_write_tile(&lTEST2,    NULL, "lTEST2_220x_180.rgb565");


  IPL2_MSG_FATAL("### TEST 3 lp to lp, rot90, up 1.25x###\n");

  // upsize and no color convert, but rotate 90
  // currently not supported
  lTEST3.dx = 180;
  lTEST3.dy = 220;
  lTEST3.cFormat = IPL_YCrCb420_LINE_PK;
  ipl_crop_resize_rot(&lQCIF_420, &lTEST3, NULL, NULL, IPL_ROT90, IPL_QUALITY_MEDIUM);
  //ipl_debug_write_tile(&lTEST3,    NULL, "lTEST3_180x_220.ycrcb420lp");


  IPL2_MSG_FATAL("### TEST 4 lp to rgb, rot90, up 1.25x###\n");

  // upsize and color convert, but rotate 90
  lTEST4.dx = 180;
  lTEST4.dy = 220;
  lTEST4.cFormat = IPL_RGB565;
  ipl_crop_resize_rot(&lQCIF_420, &lTEST4, NULL, NULL, IPL_ROT90, IPL_QUALITY_MEDIUM);

  IPL2_MSG_FATAL("### DONE ###\n");
  //ipl_debug_write_tile(&lTEST4,    NULL, "lTEST4_180x_220.rgb565");
#endif


#if 0
  IPL2_MSG_FATAL("### create input image ###\n");

  // upsize and color convert, but rotate 90
  if (ipl_crop_resize_rot(&lQVGA_YCC, &lTEST3, NULL, NULL, IPL_NOROT,IPL_QUALITY_MEDIUM))
    IPL2_MSG_FATAL("bad1");
  //ipl_debug_write_tile(&lTEST3, NULL, "lTEST3_1024x_768.ycbcr");

  IPL2_MSG_FATAL("### TEST Whiteboard ###\n");

  if (ipl_whiteBoard(&lTEST3, &lTEST4, 0, 10, 0, 0))
    IPL2_MSG_FATAL("bad2");

  ipl_debug_write_tile(&lTEST4, NULL, "lTEST4_1024x_768.ycbcr");
  IPL2_MSG_FATAL("### TEST Whiteboard DONE ###\n");
#endif


#if 0

  // calc and draw hstogram for simon on 7K
  // upsize and no color convert
  lTEST1.dx = 320;
  lTEST1.dy = 240;
  lTEST1.cFormat = IPL_YCrCb420_LINE_PK;
  //ipl_convert_image(in, &lTEST1);
  ipl_convert_image(&lQVGA_420, &lTEST1);

  regHist[0].channel = IPL_CHANNEL_YRGB;
  regHist[0].size = 256;
  regHist[0].active[0] = 1;
  regHist[0].active[1] = 1;
  regHist[0].active[2] = 1;
  regHist[0].active[3] = 1;
  
#if 0
  // do yrgb
  IPL2_MSG_FATAL("### TEST 1 compute lrgb histogram low ###\n");
  if (ipl_util_CalcHistograms(&lTEST1, NULL, &regHist[0], IPL_QUALITY_LOW))
    IPL2_MSG_FATAL("bad1");

  IPL2_MSG_FATAL("### TEST 2 compute lrgb histogram med ###\n");
  if (ipl_util_CalcHistograms(&lTEST1, NULL, &regHist[0], IPL_QUALITY_MEDIUM))
    IPL2_MSG_FATAL("bad2");

  IPL2_MSG_FATAL("### TEST 3 compute lrgb histogram high ###\n");
  if (ipl_util_CalcHistograms(&lTEST1, NULL, &regHist[0], IPL_QUALITY_HIGH))
    IPL2_MSG_FATAL("bad3");

  IPL2_MSG_FATAL("### TEST 4 compute lrgb histogram max ###\n");
  if (ipl_util_CalcHistograms(&lTEST1, NULL, &regHist[0], IPL_QUALITY_MAX))
    IPL2_MSG_FATAL("bad4");

  IPL2_MSG_FATAL("### TEST 5 draw lrgb histogram ###\n");
  if (ipl_comp_DrawHistogram(&lTEST1, NULL, &regHist[0],NULL,128,128,128))
    IPL2_MSG_FATAL("bad5");
  IPL2_MSG_FATAL("### END OF TEST ###\n");
  
  if (ipl_convert_image(&lTEST1, out))
    IPL2_MSG_FATAL("bad6");

  ipl_debug_write_tile(out, NULL, "lTEST1yrgb_320x_240.rgb");
#endif


#if 1
  // do luma only
  regHist[0].active[0] = 1;
  regHist[0].active[1] = 0;
  regHist[0].active[2] = 0;
  regHist[0].active[3] = 0;

  
  // do luma
  IPL2_MSG_FATAL("### TEST 1 compute luma histogram low ###\n");
  if (ipl_calc_mchan_histograms(&lTEST1, NULL, &regHist[0], IPL_QUALITY_LOW))
    IPL2_MSG_FATAL("bad1");

  IPL2_MSG_FATAL("### TEST 2 compute luma histogram med ###\n");
  if (ipl_calc_mchan_histograms(&lTEST1, NULL, &regHist[0], IPL_QUALITY_MEDIUM))
    IPL2_MSG_FATAL("bad2");

  IPL2_MSG_FATAL("### TEST 3 compute luma histogram high ###\n");
  if (ipl_calc_mchan_histograms(&lTEST1, NULL, &regHist[0], IPL_QUALITY_HIGH))
    IPL2_MSG_FATAL("bad3");

  IPL2_MSG_FATAL("### TEST 4 compute luma histogram max ###\n");
  if (ipl_calc_mchan_histograms(&lTEST1, NULL, &regHist[0], IPL_QUALITY_MAX))
    IPL2_MSG_FATAL("bad4");


  IPL2_MSG_FATAL("### TEST 5 draw lrgb histogram ###\n");
  if (ipl_draw_mchan_histogram(&lTEST1, NULL, &regHist[0],NULL,IPL_NOROT,0,128,128,128))
    IPL2_MSG_FATAL("bad5");
  IPL2_MSG_FATAL("### END OF TEST ###\n");
  


  if (ipl_convert_image(&lTEST1, out))
    IPL2_MSG_FATAL("bad6");

  //ipl_debug_write_tile(&lTEST1, NULL, "lTEST1yrgb_320x_240.ycrcb420lp");

#endif
 
#endif





#if 0
  if (ipl_crop_resize_rot(&lQVGA_420, &lVGA_420, NULL, NULL,0,3))
    IPL2_MSG_FATAL("bad0");

  //ipl_debug_write_tile(&lVGA_420, NULL,"lTEST1input_640x_480.ycrcb420lp");

  IPL2_MSG_FATAL("### TEST 2 convert 420lp to 420fp ###\n");
  if (ipl_convert_image(&lVGA_420, &lVGA_420fp))
    IPL2_MSG_FATAL("bad1");
  IPL2_MSG_FATAL("### TEST END ###\n");
  //ipl_debug_write_tile(&lVGA_420fp, NULL, "lTEST1_640x_480.ycrcb420fp");


  IPL2_MSG_FATAL("### TEST 1 convert 420fp to 420lp ###\n");
  if (ipl_convert_image(&lVGA_420fp, &lVGA_420))
    IPL2_MSG_FATAL("bad1");
  IPL2_MSG_FATAL("### TEST END ###\n");
  //ipl_debug_write_tile(&lVGA_420, NULL, "lTEST1_640x_480.ycrcb420lp");

#endif



#if 0
  if (IPL_DEBUG_WT) 
    ipl_debug_write_tile(&lQCIF_420fp, NULL, "lQCIF420fp_176x_144.ycrcb420fp");
  
  // fp to lp qcif
  IPL2_MSG_FATAL("### TEST 1 convert 420fp to 420lp ###\n");
  if (ipl_convert_image(&lQCIF_420fp, &lQCIF_420))
    IPL2_MSG_FATAL("bad0");
  IPL2_MSG_FATAL("### TEST END ###\n");
  if (IPL_DEBUG_WT) 
    ipl_debug_write_tile(&lQCIF_420, NULL, "lQCIF420_176x_144.ycrcb420lp");

  // lp upsize 176x144 to 240x196
  lTEST1.dx = 240;
  lTEST1.dy = 196;
  lTEST1.cFormat = IPL_YCrCb420_LINE_PK;
  IPL2_MSG_FATAL("### TEST 2 upsize qcif 420 to 240x196 ###\n");
  if (ipl_crop_resize_rot(&lQCIF_420, &lTEST1, NULL, NULL,0,3))
    IPL2_MSG_FATAL("bad1");
  IPL2_MSG_FATAL("### TEST END ###\n");
  if (IPL_DEBUG_WT) 
    ipl_debug_write_tile(&lTEST1, NULL,"lTEST1_240x_196.ycrcb420lp");

#if 0
  // lp upsize 176x144 to 196 x 240 due to rotation
  lTEST1.dy = 240;
  lTEST1.dx = 196;
  lTEST1.cFormat = IPL_YCrCb420_LINE_PK;
  IPL2_MSG_FATAL("### TEST 3 upsize qcif 420 to 196x240 & rot ###\n");
  if (ipl_crop_resize_rot(&lQCIF_420, &lTEST1, NULL, NULL,IPL_ROT90,3))
    IPL2_MSG_FATAL("bad2");
  IPL2_MSG_FATAL("### TEST END ###\n");
  if (IPL_DEBUG_WT) 
    ipl_debug_write_tile(&lTEST1, NULL,"lTEST1_196x_240.ycrcb420lp");
  
  // lp upsize 176x144 to 196 x 240 due to rotation
  lTEST1.dx = 320;
  lTEST1.dy = 240;
  lTEST1.cFormat = IPL_YCrCb420_LINE_PK;
  IPL2_MSG_FATAL("### TEST 4 upsize qcif 420 to 320x240 & rot ###\n");
  if (ipl_crop_resize_rot(&lQCIF_420, &lTEST1, NULL, NULL,0,3))
    IPL2_MSG_FATAL("bad3");
  IPL2_MSG_FATAL("### TEST END ###\n");
  if (IPL_DEBUG_WT)
    ipl_debug_write_tile(&lTEST1, NULL,"lTEST1_320x_240.ycrcb420lp");

  // lp upsize 176x144 to 196 x 240 due to rotation
  lTEST1.dx = 320;
  lTEST1.dy = 240;
  lTEST1.cFormat = IPL_RGB565;
  IPL2_MSG_FATAL("### TEST 5 upsize qcif 420lp to 320x240 565 ###\n");
  if (ipl_crop_resize_rot(&lQCIF_420, &lTEST1, NULL, NULL,0,3))
    IPL2_MSG_FATAL("bad4");
  IPL2_MSG_FATAL("### TEST END ###\n");
  if (IPL_DEBUG_WT)
    ipl_debug_write_tile(&lTEST1, NULL,"lTEST1_320x_240.rgb565");

#endif

  // lp upsize 176x144 
  lTEST1.dx = 224;
  lTEST1.dy = 168;
  lTEST1.cFormat = IPL_RGB565;
  IPL2_MSG_FATAL("### TEST 3 upsize qcif 420 to 224x168 rgb ###\n");
  if (ipl_crop_resize_rot(&lQCIF_420, &lTEST1, NULL, NULL,0,3))
    IPL2_MSG_FATAL("bad2");
  IPL2_MSG_FATAL("### TEST END ###\n");
  if (IPL_DEBUG_WT) 
    ipl_debug_write_tile(&lTEST1, NULL,"lTEST1_224x_168.rgb565");
  

  // lp upsize 176x144 
  lTEST1.dx = 224;
  lTEST1.dy = 168;
  lTEST1.cFormat = IPL_YCrCb420_LINE_PK;
  IPL2_MSG_FATAL("### TEST 3 upsize qcif 420 to 224x168 lp ###\n");
  if (ipl_crop_resize_rot(&lQCIF_420, &lTEST1, NULL, NULL,0,3))
    IPL2_MSG_FATAL("bad2");
  IPL2_MSG_FATAL("### TEST END ###\n");
  if (IPL_DEBUG_WT) 
    ipl_debug_write_tile(&lTEST1, NULL,"lTEST1_224x_168.ycrcb420lp");
  

#endif






  IPL2_MSG_FATAL("___________ End IPL Profiling ____________\n");

  // sys_free up our memory
#if 0 
  ipl_free_img(&lQVGA_565);
  ipl_free_img(&lQVGA_888);
  ipl_free_img(&lQVGA_YCC);
  ipl_free_img(&lQVGA_420);
  ipl_free_img(&lQVGA_420fp);
  ipl_free_img(&lQVGA_422);
  ipl_free_img(&pQVGA_565);
  ipl_free_img(&pQVGA_YCC);

  ipl_free_img(&lQCIF_565);
  ipl_free_img(&lQCIF_888);
  ipl_free_img(&lQCIF_YCC);
  ipl_free_img(&lQCIF_420);
  ipl_free_img(&lQCIF_420fp);
  ipl_free_img(&lQCIF_422);
  ipl_free_img(&pQCIF_565);
  ipl_free_img(&pQCIF_YCC);

  ipl_free_img(&l2MPa);
  ipl_free_img(&l2MPb);
  ipl_free_img(&l2MPc);

  ipl_free_img(&lTEST1);
  ipl_free_img(&lTEST2);
  ipl_free_img(&lTEST3);
  ipl_free_img(&lTEST4);
#endif


  return IPL_SUCCESS;
}


#endif

















/*lint -restore */

