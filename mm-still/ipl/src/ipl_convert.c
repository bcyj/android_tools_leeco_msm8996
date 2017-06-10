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

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_convert.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/05/05   babakf  Added improved the quality of rgb565 to 420line pack
11/28/05   babakf  Added IPL_HSV to ipl_convert_image function
11/28/05   babakf  Added normalized version of hsv_to_ycbcr and ycbcr to hsv
09/30/05   babakf  Added rgb888 to ycbcr, ycbcr422lp and ycbcr420lp with odd
                   width support. 
09/30/05   babakf  Removed many lint warnings
09/01/05   babakf  Added ycrcb422lp <-> ycbcr422lp
                         ycrcb420lp <-> ycbcr420lp
                   to ipl_convert_image
06/17/05   babakf  Added ycbcr422 and ycbcr444 to ycbcr444pad
                   Added rgb565 to rgb888pad
03/14/05   babakf  Added odd input width support to RGB8882YCbCr and
                   RGB5652YCbCr
03/09/05   babakf  Added ipl_convert_image as top most func to call almost 
                   all conversion functions 
02/10/05   babakf  Added 422 output support to 420mb_to_420lp. 
02/04/05   babakf  Added ipl2_convert_rgb565_to_ycbcr420lp,
                   Added ipl2_convert_rgb565_to_ycbcr422lp,
                   Added ipl2_convert_ycbcr420lp_to_rgb565,
                   Added ipl2_convert_ycbcr422lp_to_rgb565,
01/14/05   mz      Added ipl_convert_ycbcr422_to_ycbcr444 
                     and ipl_convert_ycbcr444_to_ycbcr422.
01/13/05   mz      Added ipl_convert_rgb565_to_ycbcr444,
                         ipl_convert_frame_rgb565_to_ycbcr444,
                         ipl_convert_ycrcb420lp_to_ycbcr422,
                         ipl_convert_ycbcr422_to_ycrcb422lp,
                     and ipl_convert_ycrcb422lp_to_ycbcr422.
12/07/05   babakf  Added ipl_convert_ycbcr422_to_ycrcb422lp
                   Added odd and even pitch support to 
                   ycrcb420_to_rgb565 and ycrcb422_to_rgb565
                   Added ipl_convert_swap_chroma_order, to swap cb and cr
                   for line packed images.
11/22/04   mz      Added ipl_convert_rgb565_to_ycrcb420lp,
                         ipl_convert_rgb565_to_ycrcb422lp,
                         ipl_convert_ycrcb422lp_to_rgb565,
                     and ipl_convert_ycrcb420lp_to_rgb565.
11/12/04   mz      Added ipl_convert_ycrcb420lp_to_rgb565.
11/11/04   mz      Added ipl_convert_ycbcr422_to_ycrcb420lp.
10/21/04   mz      Added 10 new color conversion functions: 
                   ipl_convert_ycbcr420lp_to_rgb444, 
                   ipl_convert_ycbcr420lp_to_rgb666, 
                   ipl_convert_ycbcr420lp_to_ycbcr422lp, 
                   ipl_convert_ycbcr420lp_to_ycbcr444lp, 
                   ipl_convert_ycbcr422_to_ycbcr422lp, 
                   ipl_convert_rgb444_to_ycbcr420lp, 
                   ipl_convert_rgb666_to_ycbcr420lp, 
                   ipl_convert_ycbcr422lp_to_ycbcr420lp, 
                   ipl_convert_ycbcr444lp_to_ycbcr420lp, 
                   ipl_convert_ycbcr422lp_to_ycbcr422.
10/05/04   mz      Fixed bug in ipl_convert_ycbcr420lp_to_ycbcr422. Updated 
                   function APIs and comments. Corrected tab spacing and line 
                   widths.
09/01/04   bf      Created. See ipl_util.c for previous history.
===========================================================================*/

/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include <stdio.h>
#include <string.h>
#include "ipl_types.h"
#include "ipl_helper.h"
#include "ipl_compose.h"
#include "ipl_convert.h"
#include "ipl_xform.h"
#include "ipl_qvp.h"




// Turn off some lint warnings

/*lint -save -e504, all shifts are okay */
/*lint -save -e506, constant value boolean is totally okay */
/*lint -save -e508, extra extern is okay */
/*lint -save -e534, let me call printf in piece, god */
/*lint -save -e573, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e701, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e702, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e703, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e704, all shifts are okay */
/*lint -save -e713, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e732, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e734, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e737, loss of precision,signed to unsigned,usigned to signed OK*/
/*lint -save -e774, constant value boolean is totally okay */
/*lint -save -e818, lets not worry about const optimization right now */
/*lint -save -e834, this warning is just dumb */
/*lint -save -e826, we should look into this someday */





/*--------------------------------------------------------------------------
  * Lookup tables that simplify conversion from YCbCr to RGB565.
  * These tables store computationally intensive intermediate
  * arithmetic operations. Look at ipl_init_lookup_tables for more.
  * Selection of these smaller tables and co-locating them
  * is to aid in better cache performance.
--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
            Lookup table for RGB To YCbCr conversion.
--------------------------------------------------------------------------*/
extern int16 ipl2_rgb565ToYR[];/* R to Y conversion normalized RGB565 */
extern int16 ipl2_rgb565ToYG[];/* G to Y conversion normalized RGB565 */
extern int16 ipl2_rgb565ToYB[];/* B to Y conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCbR[];/* R to Cb conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCbG[];/* G to Cb conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCbB[];/* B to Cb conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCrR[];/* R to Cr conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCrG[];/* G to Cr conversion normalized RGB565 */
extern int16 ipl2_rgb565ToCrB[];/* B to Cr conversion normalized RGB565 */
//extern uint8 ipl2_rgb565B2ToG[];
//extern uint8 ipl2_rgb565B1ToG[];


extern int16 ipl2_CrToRTable[];
extern int16 ipl2_CrToGTable[];
extern int16 ipl2_CbToGTable[];
extern int16 ipl2_CbToBTable[];


extern const uint16 ipl2_r5xx[512]; 
extern const uint16 ipl2_gx6x[340];
extern const uint16 ipl2_bxx5[384]; 

const int16 ipl2_Cr2RTable[256] = {
  -202, -200, -198, -197, -195, -194, -192, -191, 
  -189, -187, -186, -184, -183, -181, -180, -178, 
  -176, -175, -173, -172, -170, -169, -167, -165, 
  -164, -162, -161, -159, -157, -156, -154, -153, 
  -151, -150, -148, -146, -145, -143, -142, -140, 
  -139, -137, -135, -134, -132, -131, -129, -128, 
  -126, -124, -123, -121, -120, -118, -117, -115, 
  -113, -112, -110, -109, -107, -106, -104, -102, 
  -101, -99, -98, -96, -94, -93, -91, -90, 
  -88, -87, -85, -83, -82, -80, -79, -77, 
  -76, -74, -72, -71, -69, -68, -66, -65, 
  -63, -61, -60, -58, -57, -55, -54, -52, 
  -50, -49, -47, -46, -44, -43, -41, -39, 
  -38, -36, -35, -33, -31, -30, -28, -27, 
  -25, -24, -22, -20, -19, -17, -16, -14, 
  -13, -11, -9, -8, -6, -5, -3, -2, 
  0, 2, 3, 5, 6, 8, 9, 11, 
  13, 14, 16, 17, 19, 20, 22, 24, 
  25, 27, 28, 30, 31, 33, 35, 36, 
  38, 39, 41, 43, 44, 46, 47, 49, 
  50, 52, 54, 55, 57, 58, 60, 61, 
  63, 65, 66, 68, 69, 71, 72, 74, 
  76, 77, 79, 80, 82, 83, 85, 87, 
  88, 90, 91, 93, 94, 96, 98, 99, 
  101, 102, 104, 106, 107, 109, 110, 112, 
  113, 115, 117, 118, 120, 121, 123, 124, 
  126, 128, 129, 131, 132, 134, 135, 137, 
  139, 140, 142, 143, 145, 146, 148, 150, 
  151, 153, 154, 156, 157, 159, 161, 162, 
  164, 165, 167, 169, 170, 172, 173, 175, 
  176, 178, 180, 181, 183, 184, 186, 187, 
  189, 191, 192, 194, 195, 197, 198, 200
};

const int16 ipl2_Cr2GTable[256] = {
  -84, -83, -83, -83, -82, -82, -81, -81, 
  -80, -80, -79, -79, -78, -78, -77, -77, 
  -76, -76, -76, -75, -75, -74, -74, -73, 
  -73, -72, -72, -71, -71, -70, -70, -69, 
  -69, -68, -68, -68, -67, -67, -66, -66, 
  -65, -65, -64, -64, -63, -63, -62, -62, 
  -61, -61, -61, -60, -60, -59, -59, -58, 
  -58, -57, -57, -56, -56, -55, -55, -54, 
  -54, -53, -53, -53, -52, -52, -51, -51, 
  -50, -50, -49, -49, -48, -48, -47, -47, 
  -46, -46, -46, -45, -45, -44, -44, -43, 
  -43, -42, -42, -41, -41, -40, -40, -39, 
  -39, -39, -38, -38, -37, -37, -36, -36, 
  -35, -35, -34, -34, -33, -33, -32, -32, 
  -31, -31, -31, -30, -30, -29, -29, -28, 
  -28, -27, -27, -26, -26, -25, -25, -24, 
  -24, -24, -23, -23, -22, -22, -21, -21, 
  -20, -20, -19, -19, -18, -18, -17, -17, 
  -17, -16, -16, -15, -15, -14, -14, -13, 
  -13, -12, -12, -11, -11, -10, -10, -9, 
  -9, -9, -8, -8, -7, -7, -6, -6, 
  -5, -5, -4, -4, -3, -3, -2, -2, 
  -2, -1, -1, 0, 0, 1, 1, 2, 
  2, 3, 3, 4, 4, 5, 5, 6, 
  6, 6, 7, 7, 8, 8, 9, 9, 
  10, 10, 11, 11, 12, 12, 13, 13, 
  13, 14, 14, 15, 15, 16, 16, 17, 
  17, 18, 18, 19, 19, 20, 20, 20, 
  21, 21, 22, 22, 23, 23, 24, 24, 
  25, 25, 26, 26, 27, 27, 28, 28, 
  28, 29, 29, 30, 30, 31, 31, 32, 
  32, 33, 33, 34, 34, 35, 35, 35
};

const int16 ipl2_Cb2GTable[256] = {
  0, 0, 0, 1, 1, 1, 1, 1, 
  1, 2, 2, 2, 2, 2, 3, 3, 
  3, 3, 3, 4, 4, 4, 4, 4, 
  4, 5, 5, 5, 5, 5, 6, 6, 
  6, 6, 6, 7, 7, 7, 7, 7, 
  7, 8, 8, 8, 8, 8, 9, 9, 
  9, 9, 9, 10, 10, 10, 10, 10, 
  10, 11, 11, 11, 11, 11, 12, 12, 
  12, 12, 12, 13, 13, 13, 13, 13, 
  13, 14, 14, 14, 14, 14, 15, 15, 
  15, 15, 15, 16, 16, 16, 16, 16, 
  16, 17, 17, 17, 17, 17, 18, 18, 
  18, 18, 18, 19, 19, 19, 19, 19, 
  19, 20, 20, 20, 20, 20, 21, 21, 
  21, 21, 21, 22, 22, 22, 22, 22, 
  22, 23, 23, 23, 23, 23, 24, 24, 
  24, 24, 24, 25, 25, 25, 25, 25, 
  25, 26, 26, 26, 26, 26, 27, 27, 
  27, 27, 27, 28, 28, 28, 28, 28, 
  28, 29, 29, 29, 29, 29, 30, 30, 
  30, 30, 30, 31, 31, 31, 31, 31, 
  31, 32, 32, 32, 32, 32, 33, 33, 
  33, 33, 33, 34, 34, 34, 34, 34, 
  34, 35, 35, 35, 35, 35, 36, 36, 
  36, 36, 36, 37, 37, 37, 37, 37, 
  37, 38, 38, 38, 38, 38, 39, 39, 
  39, 39, 39, 40, 40, 40, 40, 40, 
  40, 41, 41, 41, 41, 41, 42, 42, 
  42, 42, 42, 43, 43, 43, 43, 43, 
  43, 44, 44, 44, 44, 44, 45, 45, 
  45, 45, 45, 46, 46, 46, 46, 46, 
  46, 47, 47, 47, 47, 47, 48, 48
};

const int16 ipl2_Cb2BTable[256] = {
  -237, -236, -234, -232, -230, -228, -226, -225,
  -223, -221, -219, -217, -215, -213, -212, -210, 
  -208, -206, -204, -202, -200, -199, -197, -195, 
  -193, -191, -189, -187, -186, -184, -182, -180, 
  -178, -176, -174, -173, -171, -169, -167, -165, 
  -163, -161, -160, -158, -156, -154, -152, -150, 
  -148, -147, -145, -143, -141, -139, -137, -135, 
  -134, -132, -130, -128, -126, -124, -122, -121, 
  -119, -117, -115, -113, -111, -109, -108, -106, 
  -104, -102, -100, -98, -96, -95, -93, -91, 
  -89, -87, -85, -83, -82, -80, -78, -76, 
  -74, -72, -71, -69, -67, -65, -63, -61, 
  -59, -58, -56, -54, -52, -50, -48, -46, 
  -45, -43, -41, -39, -37, -35, -33, -32, 
  -30, -28, -26, -24, -22, -20, -19, -17, 
  -15, -13, -11, -9, -7, -6, -4, -2, 
  0, 2, 4, 6, 7, 9, 11, 13, 
  15, 17, 19, 20, 22, 24, 26, 28, 
  30, 32, 33, 35, 37, 39, 41, 43, 
  45, 46, 48, 50, 52, 54, 56, 58, 
  59, 61, 63, 65, 67, 69, 71, 72, 
  74, 76, 78, 80, 82, 83, 85, 87, 
  89, 91, 93, 95, 96, 98, 100, 102, 
  104, 106, 108, 109, 111, 113, 115, 117, 
  119, 121, 122, 124, 126, 128, 130, 132, 
  134, 135, 137, 139, 141, 143, 145, 147, 
  148, 150, 152, 154, 156, 158, 160, 161, 
  163, 165, 167, 169, 171, 173, 174, 176, 
  178, 180, 182, 184, 186, 187, 189, 191, 
  193, 195, 197, 199, 200, 202, 204, 206, 
  208, 210, 212, 213, 215, 217, 219, 221, 
  223, 225, 226, 228, 230, 232, 234, 236
};



// create by ipl_create_look_ups in ipl_helper.c
const int32 ipl_crr[256] = {
-179, -177, -176, -175, -173, -172, -170, -169, -168, -166,
-165, -163, -162, -161, -159, -158, -156, -155, -154, -152,
-151, -149, -148, -147, -145, -144, -142, -141, -139, -138,
-137, -135, -134, -132, -131, -130, -128, -127, -125, -124,
-123, -121, -120, -118, -117, -116, -114, -113, -111, -110,
-109, -107, -106, -104, -103, -102, -100, -99, -97, -96,
-95, -93, -92, -90, -89, -88, -86, -85, -83, -82,
-81, -79, -78, -76, -75, -74, -72, -71, -69, -68,
-67, -65, -64, -62, -61, -60, -58, -57, -55, -54,
-53, -51, -50, -48, -47, -46, -44, -43, -41, -40,
-39, -37, -36, -34, -33, -32, -30, -29, -27, -26,
-25, -23, -22, -20, -19, -18, -16, -15, -13, -12,
-11, -9, -8, -6, -5, -4, -2, -1, 1, 2,
4, 5, 6, 8, 9, 11, 12, 13, 15, 16,
18, 19, 20, 22, 23, 25, 26, 27, 29, 30,
32, 33, 34, 36, 37, 39, 40, 41, 43, 44,
46, 47, 48, 50, 51, 53, 54, 55, 57, 58,
60, 61, 62, 64, 65, 67, 68, 69, 71, 72,
74, 75, 76, 78, 79, 81, 82, 83, 85, 86,
88, 89, 90, 92, 93, 95, 96, 97, 99, 100,
102, 103, 104, 106, 107, 109, 110, 111, 113, 114,
116, 117, 118, 120, 121, 123, 124, 125, 127, 128,
130, 131, 132, 134, 135, 137, 138, 139, 141, 142,
144, 145, 147, 148, 149, 151, 152, 154, 155, 156,
158, 159, 161, 162, 163, 165, 166, 168, 169, 170,
172, 173, 175, 176, 177, 179};

// create by ipl_create_look_ups in ipl_helper.c
const int32 ipl_cbb[256] = {
-226, -224, -222, -221, -219, -217, -215, -214, -212, -210,
-208, -206, -205, -203, -201, -199, -198, -196, -194, -192,
-190, -189, -187, -185, -183, -182, -180, -178, -176, -175,
-173, -171, -169, -167, -166, -164, -162, -160, -159, -157,
-155, -153, -152, -150, -148, -146, -144, -143, -141, -139,
-137, -136, -134, -132, -130, -128, -127, -125, -123, -121,
-120, -118, -116, -114, -113, -111, -109, -107, -105, -104,
-102, -100, -98, -97, -95, -93, -91, -89, -88, -86,
-84, -82, -81, -79, -77, -75, -74, -72, -70, -68,
-66, -65, -63, -61, -59, -58, -56, -54, -52, -51,
-49, -47, -45, -43, -42, -40, -38, -36, -35, -33,
-31, -29, -27, -26, -24, -22, -20, -19, -17, -15,
-13, -12, -10, -8, -6, -4, -3, -1, 1, 3,
4, 6, 8, 10, 12, 13, 15, 17, 19, 20,
22, 24, 26, 27, 29, 31, 33, 35, 36, 38,
40, 42, 43, 45, 47, 49, 51, 52, 54, 56,
58, 59, 61, 63, 65, 66, 68, 70, 72, 74,
75, 77, 79, 81, 82, 84, 86, 88, 89, 91,
93, 95, 97, 98, 100, 102, 104, 105, 107, 109,
111, 113, 114, 116, 118, 120, 121, 123, 125, 127,
128, 130, 132, 134, 136, 137, 139, 141, 143, 144,
146, 148, 150, 152, 153, 155, 157, 159, 160, 162,
164, 166, 167, 169, 171, 173, 175, 176, 178, 180,
182, 183, 185, 187, 189, 190, 192, 194, 196, 198,
199, 201, 203, 205, 206, 208, 210, 212, 214, 215,
217, 219, 221, 222, 224, 226};

// create by ipl_create_look_ups in ipl_helper.c
const int32 ipl_crg[256] = {
5967255, 5920453, 5873651, 5826849, 5780047, 5733245, 5686443,
5639641, 5592839, 5546037, 5499235, 5452433, 5405631, 5358829,
5312027, 5265225, 5218423, 5171621, 5124819, 5078017, 5031215,
4984413, 4937611, 4890809, 4844007, 4797205, 4750403, 4703601,
4656799, 4609997, 4563195, 4516393, 4469591, 4422789, 4375987,
4329185, 4282383, 4235581, 4188779, 4141977, 4095175, 4048373,
4001571, 3954769, 3907967, 3861165, 3814363, 3767561, 3720759,
3673957, 3627155, 3580353, 3533551, 3486749, 3439947, 3393145,
3346343, 3299541, 3252739, 3205937, 3159135, 3112333, 3065531,
3018729, 2971927, 2925125, 2878323, 2831521, 2784719, 2737917,
2691115, 2644313, 2597511, 2550709, 2503907, 2457105, 2410303,
2363501, 2316699, 2269897, 2223095, 2176293, 2129491, 2082689,
2035887, 1989085, 1942283, 1895481, 1848679, 1801877, 1755075,
1708273, 1661471, 1614669, 1567867, 1521065, 1474263, 1427461,
1380659, 1333857, 1287055, 1240253, 1193451, 1146649, 1099847,
1053045, 1006243, 959441, 912639, 865837, 819035, 772233,
725431, 678629, 631827, 585025, 538223, 491421, 444619,
397817, 351015, 304213, 257411, 210609, 163807, 117005,
70203, 23401, -23401, -70203, -117005, -163807, -210609,
-257411, -304213, -351015, -397817, -444619, -491421, -538223,
-585025, -631827, -678629, -725431, -772233, -819035, -865837,
-912639, -959441, -1006243, -1053045, -1099847, -1146649, -1193451,
-1240253, -1287055, -1333857, -1380659, -1427461, -1474263, -1521065,
-1567867, -1614669, -1661471, -1708273, -1755075, -1801877, -1848679,
-1895481, -1942283, -1989085, -2035887, -2082689, -2129491, -2176293,
-2223095, -2269897, -2316699, -2363501, -2410303, -2457105, -2503907,
-2550709, -2597511, -2644313, -2691115, -2737917, -2784719, -2831521,
-2878323, -2925125, -2971927, -3018729, -3065531, -3112333, -3159135,
-3205937, -3252739, -3299541, -3346343, -3393145, -3439947, -3486749,
-3533551, -3580353, -3627155, -3673957, -3720759, -3767561, -3814363,
-3861165, -3907967, -3954769, -4001571, -4048373, -4095175, -4141977,
-4188779, -4235581, -4282383, -4329185, -4375987, -4422789, -4469591,
-4516393, -4563195, -4609997, -4656799, -4703601, -4750403, -4797205,
-4844007, -4890809, -4937611, -4984413, -5031215, -5078017, -5124819,
-5171621, -5218423, -5265225, -5312027, -5358829, -5405631, -5452433,
-5499235, -5546037, -5592839, -5639641, -5686443, -5733245, -5780047,
-5826849, -5873651, -5920453, -5967255};

// create by ipl_create_look_ups in ipl_helper.c
const int32 ipl_cbg[256] = {
2908403, 2885849, 2863295, 2840741, 2818187, 2795633, 2773079,
2750525, 2727971, 2705417, 2682863, 2660309, 2637755, 2615201,
2592647, 2570093, 2547539, 2524985, 2502431, 2479877, 2457323,
2434769, 2412215, 2389661, 2367107, 2344553, 2321999, 2299445,
2276891, 2254337, 2231783, 2209229, 2186675, 2164121, 2141567,
2119013, 2096459, 2073905, 2051351, 2028797, 2006243, 1983689,
1961135, 1938581, 1916027, 1893473, 1870919, 1848365, 1825811,
1803257, 1780703, 1758149, 1735595, 1713041, 1690487, 1667933,
1645379, 1622825, 1600271, 1577717, 1555163, 1532609, 1510055,
1487501, 1464947, 1442393, 1419839, 1397285, 1374731, 1352177,
1329623, 1307069, 1284515, 1261961, 1239407, 1216853, 1194299,
1171745, 1149191, 1126637, 1104083, 1081529, 1058975, 1036421,
1013867, 991313, 968759, 946205, 923651, 901097, 878543,
855989, 833435, 810881, 788327, 765773, 743219, 720665,
698111, 675557, 653003, 630449, 607895, 585341, 562787,
540233, 517679, 495125, 472571, 450017, 427463, 404909,
382355, 359801, 337247, 314693, 292139, 269585, 247031,
224477, 201923, 179369, 156815, 134261, 111707, 89153,
66599, 44045, 21491, -1063, -23617, -46171, -68725,
-91279, -113833, -136387, -158941, -181495, -204049, -226603,
-249157, -271711, -294265, -316819, -339373, -361927, -384481,
-407035, -429589, -452143, -474697, -497251, -519805, -542359,
-564913, -587467, -610021, -632575, -655129, -677683, -700237,
-722791, -745345, -767899, -790453, -813007, -835561, -858115,
-880669, -903223, -925777, -948331, -970885, -993439, -1015993,
-1038547, -1061101, -1083655, -1106209, -1128763, -1151317, -1173871,
-1196425, -1218979, -1241533, -1264087, -1286641, -1309195, -1331749,
-1354303, -1376857, -1399411, -1421965, -1444519, -1467073, -1489627,
-1512181, -1534735, -1557289, -1579843, -1602397, -1624951, -1647505,
-1670059, -1692613, -1715167, -1737721, -1760275, -1782829, -1805383,
-1827937, -1850491, -1873045, -1895599, -1918153, -1940707, -1963261,
-1985815, -2008369, -2030923, -2053477, -2076031, -2098585, -2121139,
-2143693, -2166247, -2188801, -2211355, -2233909, -2256463, -2279017,
-2301571, -2324125, -2346679, -2369233, -2391787, -2414341, -2436895,
-2459449, -2482003, -2504557, -2527111, -2549665, -2572219, -2594773,
-2617327, -2639881, -2662435, -2684989, -2707543, -2730097, -2752651,
-2775205, -2797759, -2820313, -2842867};






/*===========================================================================
                        FUNCTION DECLARATIONS
===========================================================================*/





/*===========================================================================

FUNCTION ipl_convert_from_ycbcr444pad

DESCRIPTION
  This function converts from GRP2 padded YCbCr 4:4:4 format to RGB565 or 
  RGB 444.  Input and output image sizes must be equal.

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
static ipl_status_type ipl_convert_from_ycbcr444pad
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
)
{
  register unsigned char *inImgPtr;
  register uint16 *outImgPtr;
  register uint16 out;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  uint32 x;

   MSG_LOW("ipl_convert_from_ycbcr444pad marker_0\n");




  // do some checks on input to see if everything looks in order
  if (!input_img_ptr  || !input_img_ptr->imgPtr  ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_from_ycbcr444pad marker_200\n");
    return IPL_FAILURE;
  }

  if (input_img_ptr->cFormat != IPL_YCbCr444_PAD)
  {
    MSG_LOW("ipl_convert_from_ycbcr444pad marker_201\n");
    return IPL_FAILURE;
  }

  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_from_ycbcr444pad marker_202\n");
    return IPL_FAILURE;
  }

  // setup our LUTs to do the conversion and packing
  rTable = &(ipl2_r5xx[0]);
  gTable = &(ipl2_gx6x[0]);
  bTable = &(ipl2_bxx5[0]);

  /* Initialize image pointers */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = (uint16 *) output_img_ptr->imgPtr;

  MSG_LOW("ipl_convert_from_ycbcr444pad marker_1\n");

  // do this for every 3 input bytes to create on output 16bit pixel
  for (x = (output_img_ptr->dx*output_img_ptr->dy); x; x--) 
  {
#if 1
    // input to function ic Cb Y Cr
    IPL2_CONVERT_YCBCR_RGB_SINGLE(inImgPtr[1],inImgPtr[3],inImgPtr[2],
                                  r,out,rTable,gTable,bTable);
#else
    // input to function ic 0 Y Cb Cr
    IPL2_CONVERT_YCBCR_RGB_SINGLE(inImgPtr[2],inImgPtr[3],inImgPtr[1],
                                  r,out,rTable,gTable,bTable);
#endif
    *outImgPtr++ = out;
    inImgPtr += 4;
  }

  MSG_LOW("ipl_convert_from_ycbcr444pad marker_100\n");
  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_to_ycbcr444pad

DESCRIPTION
  This function converts from a variety of inputs to GRP2's padded YCbCr 
  4:4:4 format. Input and output image sizes must be equal.

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
static ipl_status_type ipl_convert_to_ycbcr444pad
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
)
{
  register unsigned char *inImgPtr, *outImgPtr;
  register unsigned char *inClrPtr;
  register uint32 x;
  uint8 cb, cr, luma;
  uint32 y;
  unsigned char r1, g1, b1;
  uint16 out;

  MSG_LOW("ipl_convert_to_ycbcr444pad marker_0\n");

   // do some checks on input to see if everything looks in order
  if (!input_img_ptr  || !input_img_ptr->imgPtr  ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_to_ycbcr444pad marker_200\n");
    return IPL_FAILURE;
  }

  if (output_img_ptr->cFormat != IPL_YCbCr444_PAD)
  {
    MSG_LOW("ipl_convert_to_ycbcr444pad marker_201\n");
    return IPL_FAILURE;
  }

  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_to_ycbcr444pad marker_202\n");
    return IPL_FAILURE;
  }
  
  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;

  MSG_LOW("ipl_convert_to_ycbcr444pad marker_1\n");
  
  if (input_img_ptr->cFormat == IPL_YCbCr444) 
  {

    for (x = (output_img_ptr->dx*output_img_ptr->dy); x; x--) 
    {
      /* Process 2 pixels at a time */
#if 1
      // input  is Cb Cr Y
      // output is 0  Y  Cb Cr
      
      cb = *inImgPtr++;
      cr = *inImgPtr++;
      luma = *inImgPtr++;

      /* Pixel 1 */
      *outImgPtr++ = 0;
      *outImgPtr++ = luma;
      *outImgPtr++ = cb;
      *outImgPtr++ = cr;
#else
      // input  is Cb Cr Y
      // output is 0  Cb Y Cr
      *outImgPtr++ = 0;                 // 0 
      *outImgPtr++ = *inImgPtr++;       // Copy Cb over 
      //*outImgPtr++ = *(inImgPtr++ + 1); // Copy Y over 
      //*outImgPtr++ = *(inImgPtr++ - 1); // Copy Cr over 
      *outImgPtr++ = *(inImgPtr + 1); 
      *outImgPtr++ = *(inImgPtr); 
      inImgPtr += 2; 
#endif
    }

  }
  else if (input_img_ptr->cFormat == IPL_YCbCr) 
  {

    for (x = (output_img_ptr->dx*output_img_ptr->dy)>>1; x; x--) 
    {
#if 1
      cb = *inImgPtr++;
      luma = *inImgPtr++;
      cr = *inImgPtr++;

      *outImgPtr++ = 0;
      *outImgPtr++ = luma;
      *outImgPtr++ = cb;
      *outImgPtr++ = cr;

      *outImgPtr++ = 0;
      *outImgPtr++ = *inImgPtr++;
      *outImgPtr++ = cb;
      *outImgPtr++ = cr;
#else
      // input  is Cb Y  Cr Y
      // output is 0  Cb Y  Cr   and    0 Cb Y Cr
      
      *outImgPtr++ = 0;                 // 0 
      *outImgPtr++ = *inImgPtr++;       // Copy Cb over 
      *outImgPtr++ = *inImgPtr++;       // Copy Y over 
      *outImgPtr++ = *inImgPtr++;       // Copy Cr over 

      *outImgPtr++ = 0;                 // 0 
      *outImgPtr++ = *(outImgPtr - 4);  // Copy Cb over 
      *outImgPtr++ = *inImgPtr++;       // Copy Y over 
      *outImgPtr++ = *(outImgPtr - 4);  // Copy Cr over 
#endif
    }

  }
  else if (input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
  {
    int idx = input_img_ptr->dx;
    int odx = 4*output_img_ptr->dx;

    inClrPtr = input_img_ptr->clrPtr;

    for (y = input_img_ptr->dy/2; y; y--) 
    {
      for (x = input_img_ptr->dx/2; x; x--) 
      {
        cr = *inClrPtr++;
        cb = *inClrPtr++;

        //  P1 P2 
        //  P3 P4
        
#if 1
        // Output is 0 Y Cb Cr 
        // do P1 and P3
        *outImgPtr = 0;
        *(outImgPtr++ + odx) = 0;

        *outImgPtr = *inImgPtr;
        *(outImgPtr++ + odx) = *(inImgPtr++ + idx);

        *outImgPtr = cb;
        *(outImgPtr++ + odx) = cb;

        *outImgPtr = cr;
        *(outImgPtr++ + odx) = cr;

        // do P2 and P4
        *outImgPtr = 0;
        *(outImgPtr++ + odx) = 0;

        *outImgPtr = *inImgPtr;
        *(outImgPtr++ + odx) = *(inImgPtr++ + idx);

        *outImgPtr = cb;
        *(outImgPtr++ + odx) = cb;

        *outImgPtr = cr;
        *(outImgPtr++ + odx) = cr;
#else
        // Output is 0 Cb Y Cr 
        // do P1 and P3
        *outImgPtr = 0;
        *(outImgPtr++ + odx) = 0;

        *outImgPtr = cb;
        *(outImgPtr++ + odx) = cb;

        *outImgPtr = *inImgPtr;
        *(outImgPtr++ + odx) = *(inImgPtr++ + idx);

        *outImgPtr = cr;
        *(outImgPtr++ + odx) = cr;

        // do P2 and P4
        *outImgPtr = 0;
        *(outImgPtr++ + odx) = 0;

        *outImgPtr = cb;
        *(outImgPtr++ + odx) = cb;

        *outImgPtr = *inImgPtr;
        *(outImgPtr++ + odx) = *(inImgPtr++ + idx);

        *outImgPtr = cr;
        *(outImgPtr++ + odx) = cr;
#endif
      }

      // since we just did two rows, skip a line in input and output
      outImgPtr += odx;
      inImgPtr += idx;
    }
  }
  else if (input_img_ptr->cFormat == IPL_RGB565) 
  {
    uint16* in16ptr;

    if (ipl2_init() != IPL_SUCCESS)
    {
      MSG_LOW("ipl_convert_to_ycbcr444pad marker_203\n");
      return(IPL_FAILURE);
    }

    in16ptr = (uint16 *) input_img_ptr->imgPtr;


    for (x = (output_img_ptr->dx*output_img_ptr->dy); x; x--) 
    {
      out = *in16ptr++;
      r1 = out >> 8;
      g1 = (out >> 3) & 0xff;
      b1 = out & 0xff;

#if 1
      // input  is R  G  B 
      // output is 0  Cb Y  Cr   and    0 Cb Y Cr
      *outImgPtr++ = 0;
      *outImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];
      *outImgPtr++ = ipl2_rgb565ToCbR[r1] + ipl2_rgb565ToCbG[g1] 
        + ipl2_rgb565ToCbB[b1];
      *outImgPtr++ = ipl2_rgb565ToCrR[r1] + ipl2_rgb565ToCrG[g1] 
        + ipl2_rgb565ToCrB[b1];
#else
      *outImgPtr++ = 0;

      // copy Cb
      *outImgPtr++ = ipl2_rgb565ToCbR[r1] + ipl2_rgb565ToCbG[g1] +
        ipl2_rgb565ToCbB[b1];

      // copy Y 
      *outImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] +
        ipl2_rgb565ToYB[b1];

      // copy Cr 
      *outImgPtr++ = ipl2_rgb565ToCrR[r1] + ipl2_rgb565ToCrG[g1] 
        + ipl2_rgb565ToCrB[b1];
#endif
    }

  }
  else if ((input_img_ptr->cFormat == IPL_YCbCr420_MB_PK)  ||
           (input_img_ptr->cFormat == IPL_YCrCb420_MB_PK)) 
  {
    int mbx, mby;
    int dx, nmbx, nmby;

    nmbx = input_img_ptr->dx/16;
    nmby = input_img_ptr->dy/16;
    dx = 4*output_img_ptr->dx;

    inClrPtr = input_img_ptr->clrPtr;


    for(mby = 0; mby < nmby; mby++)
    {
      for(mbx = 0; mbx < nmbx; mbx++)
      {
        outImgPtr = output_img_ptr->imgPtr + mbx*64 + mby*(1024*nmbx);

        /* two two rows at a time */
        for(y = 8; y; y--)
        {
          /* two two colums at a time */
          for(x = 8; x ; x--)
          {
            // do we really need to do this, or can we memset?
            *outImgPtr = 0;
            *(outImgPtr + 4) = 0;
            *(outImgPtr + dx) = 0;
            *(outImgPtr + 4 + dx) = 0;
            outImgPtr++;

            // copy Y
            *outImgPtr = *inImgPtr;
            *(outImgPtr + 4) = *(inImgPtr+1);
            *(outImgPtr + dx) = *(inImgPtr+16);
            *(outImgPtr + 4 + dx) = *(inImgPtr+17);
            outImgPtr++;
            inImgPtr += 2;


            // copy Cb 
            *outImgPtr = *(inClrPtr+1);
            *(outImgPtr + 4) = *(inClrPtr+1);
            *(outImgPtr + dx) = *(inClrPtr+1);
            *(outImgPtr + 4 + dx) = *(inClrPtr + 1);
            outImgPtr++;


            // copy Cr 
            *outImgPtr = *inClrPtr;
            *(outImgPtr + 4) = *inClrPtr;
            *(outImgPtr + dx) = *inClrPtr;
            *(outImgPtr + 4 + dx) = *inClrPtr;
            outImgPtr++;
            inClrPtr += 2;

            // skip the next 0YCbCr since we just did it.
            outImgPtr += 4;
          }

          // since we did 2 lines a time, skip on enow
          inImgPtr += 16;
          outImgPtr += (2*dx-64);
        }
      }
    }

  }
  else
  {
    MSG_LOW("ipl_convert_to_ycbcr444pad marker_204\n");
    return IPL_FAILURE;
  }
  MSG_LOW("ipl_convert_to_ycbcr444pad marker_100\n");
  return IPL_SUCCESS;
} 



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
)
{
  unsigned char *inImgPtr, *outImgPtr;
  uint32 x;
  uint8 cb, cr, luma;

  MSG_LOW("ipl_convert_ycbcr422_to_ycbcr444 marker_0\n");

  // do some checks on input to see if everything looks in order
  if (!input_img_ptr  || !input_img_ptr->imgPtr  ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycbcr444 marker_200\n");
    return IPL_FAILURE;
  }

  if (input_img_ptr->cFormat != IPL_YCbCr ||
      output_img_ptr->cFormat != IPL_YCbCr444) 
  {
     MSG_LOW("ipl_convert_ycbcr422_to_ycbcr444 marker_201\n");
     return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycbcr444 marker_202\n");
    return IPL_FAILURE;
  }

   /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;

  MSG_LOW("ipl_convert_ycbcr422_to_ycbcr444 marker_1\n");

  for (x = (output_img_ptr->dx*output_img_ptr->dy)>>1; x; x--) {
    /* Process 2 pixels at a time */
    cb = *inImgPtr++;
    luma = *inImgPtr++;
    cr = *inImgPtr++;
    /* Pixel 1 */
    *outImgPtr++ = cb;
    *outImgPtr++ = cr;
    *outImgPtr++ = luma;
    /* Pixel 2 */
    *outImgPtr++ = cb;
    *outImgPtr++ = cr;
    *outImgPtr++ = *inImgPtr++;
  } /* end x loop */

  MSG_LOW("ipl_convert_ycbcr422_to_ycbcr444 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr422_to_ycbcr444*/



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr444_to_ycbcr422

DESCRIPTION
  This function converts from YCbCr 4:4:4 to YCbCr 4:2:0. line pack
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
static ipl_status_type ipl_convert_ycbcr444_to_ycrcb420lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
)
{
  unsigned char *inImgPtr, *outImgPtr;
  unsigned char *outClrPtr;
  uint32 row, col, dx;
  
  MSG_LOW("ipl_convert_ycbcr444_to_ycrcb420lp marker_0\n");

  // do some checks on input to see if everything looks in order
  if (!input_img_ptr  || !input_img_ptr->imgPtr  ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr444_to_ycrcb420lp marker_200\n");
    return IPL_FAILURE;
  }

  if (input_img_ptr->cFormat != IPL_YCbCr444 ||
      output_img_ptr->cFormat != IPL_YCrCb420_LINE_PK) 
  {
    MSG_LOW("ipl_convert_ycbcr444_to_ycrcb420lp marker_201\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr444_to_ycrcb420lp marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;

  outImgPtr = output_img_ptr->imgPtr;
  outClrPtr = output_img_ptr->clrPtr;

  dx = output_img_ptr->dx;

  MSG_LOW("ipl_convert_ycbcr444_to_ycrcb420lp marker_1\n");

  for (row = 0; row < output_img_ptr->dy/2; row++) 
  {
    for (col = 0; col < output_img_ptr->dx/2; col++) 
    {
      /* Process 4 pixels at a time */
      *outClrPtr++ = *(inImgPtr+1); // copy over Cb
      *outClrPtr++ = *(inImgPtr);   // copy over Cr
      inImgPtr+=2;

      *(outImgPtr     ) = *(inImgPtr       ); // copy over ul Y
      *(outImgPtr + dx) = *(inImgPtr + dx*3); // copy over ll Y 
      inImgPtr += 3;
      outImgPtr++;

      *(outImgPtr     ) = *(inImgPtr       );  // copy over ur Y
      *(outImgPtr + dx) = *(inImgPtr + dx*3);  // copy over lr Y
      inImgPtr += 1;
      outImgPtr++;
    }
    outImgPtr += dx;
    inImgPtr  += (dx*3);
  }

  MSG_LOW("ipl_convert_ycbcr444_to_ycrcb420lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr444_to_ycrcb420lp*/


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
)
{
  unsigned char *inImgPtr, *outImgPtr;
  uint32 x;
  
  MSG_LOW("ipl_convert_ycbcr444_to_ycbcr422 marker_0\n");

  // do some checks on input to see if everything looks in order
  if (!input_img_ptr  || !input_img_ptr->imgPtr  ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr444_to_ycbcr422 marker_200\n");
    return IPL_FAILURE;
  }

  if (input_img_ptr->cFormat != IPL_YCbCr444 ||
      output_img_ptr->cFormat != IPL_YCbCr) 
  {
     MSG_LOW("ipl_convert_ycbcr444_to_ycbcr422 marker_201\n");
     return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr444_to_ycbcr422 marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;

  MSG_LOW("ipl_convert_ycbcr444_to_ycbcr422 marker_1\n");

  for (x = (output_img_ptr->dx*output_img_ptr->dy)>>1; x; x--) {
    /* Process 2 pixels at a time */
    /* Pixel 1: CbY */
    *outImgPtr++ = *inImgPtr++;
    inImgPtr++;
    *outImgPtr++ = *inImgPtr++;
    /* Pixel 2: CrY */
    inImgPtr++;
    *outImgPtr++ = *inImgPtr++;
    *outImgPtr++ = *inImgPtr++;
  }

  MSG_LOW("ipl_convert_ycbcr444_to_ycbcr422 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr444_to_ycbcr422*/

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_to_luma_only

DESCRIPTION
  This function converts from RGB565 to Y only data

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
static ipl_status_type ipl_convert_to_luma_only
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
)
{
  register uint16* inImgPtr;
  register uint8* outImgPtr;
  uint8 r, g, b;
  uint32 x;
  int32 luma;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_convert_to_luma_only marker_0\n");

  // do some checks on input to see if everything looks in order
  if (!i_img_ptr  || !i_img_ptr->imgPtr  ||
      !o_img_ptr  || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_to_luma_only marker_200\n");
    return IPL_FAILURE;
  }

  inImgPtr = (uint16*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;

  MSG_LOW("ipl_convert_to_luma_only marker_1\n");

  for (x = o_img_ptr->dx*o_img_ptr->dy; x; x--) 
  {
    unpack_rgb565(*inImgPtr++,&r,&g,&b);
    luma = (((ycbcr_convert[0]*r + 
              ycbcr_convert[1]*g + 
              ycbcr_convert[2]*b)*4+0x8000) >> 16) + 16;
    *outImgPtr++ = (uint8)CLIPIT(luma);
  } 

  MSG_LOW("ipl_convert_to_luma_only marker_100\n");

  return IPL_SUCCESS;
} 


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
)
{
  register uint16* inImgPtr;
  register uint8* outImgPtr;
  uint8 r, g, b;
  uint32 x;
  int32 luma, cb, cr;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_convert_rgb565_to_ycbcr444 marker_0\n");

  // do some checks on input to see if everything looks in order
  if (!i_img_ptr  || !i_img_ptr->imgPtr  ||
      !o_img_ptr  || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb565_to_ycbcr444 marker_200\n");
    return IPL_FAILURE;
  }

  inImgPtr = (uint16*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;

  MSG_LOW("ipl_convert_rgb565_to_ycbcr444 marker_1\n");

  /* Loop through input image */
  for (x = o_img_ptr->dx*o_img_ptr->dy; x; x--) {
    unpack_rgb565(*inImgPtr++,&r,&g,&b);
    cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
          ycbcr_convert[5]*b)*4+0x8000;
    cb = (cb>>16) + 128;
    *outImgPtr++ = (uint8)CLIPIT(cb);
    cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
          ycbcr_convert[8]*b)*4+0x8000;
    cr = (cr>>16) + 128;
    *outImgPtr++ = (uint8)CLIPIT(cr);
    luma = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
            ycbcr_convert[2]*b)*4+0x8000;
    luma = (luma >> 16) + 16;
    *outImgPtr++ = (uint8)CLIPIT(luma);
  } /* End of x loop */

  MSG_LOW("ipl_convert_rgb565_to_ycbcr444 marker_100\n");

  return IPL_SUCCESS;
} /* end of ipl_convert_rgb565_to_ycbcr444 */


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
)
{
  register uint16* inImgPtr;
  register uint8* outImgPtr;
  uint8 r, g, b;
  uint32 x;
  int32 luma, cb, cr;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

   MSG_LOW("ipl_convert_frame_rgb565_to_ycbcr444 marker_0\n");

  // do some checks on input to see if everything looks in order
  if (!i_img_ptr  || !i_img_ptr->imgPtr  ||
      !o_img_ptr  || !o_img_ptr->imgPtr ||
      !numCorrected)
  {
    MSG_LOW("ipl_convert_frame_rgb565_to_ycbcr444 marker_200\n");
    return IPL_FAILURE;
  }
  inImgPtr = (uint16*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;

  MSG_LOW("ipl_convert_frame_rgb565_to_ycbcr444 marker_1\n");

  /* 1st Pass */
  for (x = o_img_ptr->dx*o_img_ptr->dy; x; x--) {
    unpack_rgb565(*inImgPtr,&r,&g,&b);
    cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
          ycbcr_convert[5]*b)*4+0x8000;
    cb = (cb>>16) + 128;
    *outImgPtr++ = (uint8)CLIPIT(cb);
    cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
          ycbcr_convert[8]*b)*4+0x8000;
    cr = (cr>>16) + 128;
    *outImgPtr++ = (uint8)CLIPIT(cr);
    if (*inImgPtr++ == transparentValue) {
      *outImgPtr++ = transparentY;
    } else {
      luma = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
              ycbcr_convert[2]*b)*4+0x8000;
      luma = (luma >> 16) + 16;
      *outImgPtr++ = (uint8)CLIPIT(luma);
    }
  } /* End of x loop */

  /* 2nd Pass */
  inImgPtr = (uint16*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr + 2;
  *numCorrected = 0;
  
  for (x = o_img_ptr->dx*o_img_ptr->dy; x; x--) {
    if (*inImgPtr++ != transparentValue && *outImgPtr == transparentY) {
      *outImgPtr -= 1;
      *numCorrected += 1;
    }
    outImgPtr += 3;
  } /* End of x loop */

  MSG_LOW("ipl_convert_frame_rgb565_to_ycbcr444 marker_100\n");

  return IPL_SUCCESS;
} /* end of ipl_convert_frame_rgb565_to_ycbcr444 */


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
)
{
  register uint8 cb, cr;
  register uint32 w;
  register uint8* data_out; 
  register uint8* data2_out;
  uint32 row, col;
  uint8* y_ptr;
  uint8* yr2_ptr;
  uint8* c_ptr;
  int32 dest_index;


  MSG_LOW("ipl_convert_ycrcb420lp_to_ycbcr422 marker_0\n");

  // do some checks on input to see if everything looks in order
  if (!input_img_ptr   || !input_img_ptr->imgPtr  ||
      !output_img_ptr  || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycrcb420lp_to_ycbcr422 marker_200\n");
    return IPL_FAILURE;
  }

  y_ptr = input_img_ptr->imgPtr;
  c_ptr = input_img_ptr->clrPtr;
  data_out = (uint8*)output_img_ptr->imgPtr;
  w = output_img_ptr->dx;
  dest_index = w<<1;


  /* Verify Arguments */
  if (input_img_ptr->cFormat != IPL_YCrCb420_LINE_PK)
  {
    MSG_LOW("ipl_convert_ycrcb420lp_to_ycbcr422 marker_201\n");
    return IPL_FAILURE;
  }
  if (output_img_ptr->cFormat != IPL_YCbCr)
  {
    MSG_LOW("ipl_convert_ycrcb420lp_to_ycbcr422 marker_202\n");
    return IPL_FAILURE;
  }
  if ((input_img_ptr->dx != w) || 
      (input_img_ptr->dy != output_img_ptr->dy))
  {
    MSG_LOW("ipl_convert_ycrcb420lp_to_ycbcr422 marker_203\n");
    return IPL_FAILURE;
  }

  /* Now loop through the image once */
  data2_out = data_out + (w<<1);
  yr2_ptr = y_ptr + w;

  MSG_LOW("ipl_convert_ycrcb420lp_to_ycbcr422 marker_1\n");

  for (row = output_img_ptr->dy>>1; row; row--) {
    for (col = w>>1; col; col--) {
      /*
      ** Work on 4 pixels at a time
      */
      cr = *c_ptr++;
      cb = *c_ptr++;
      /*
      ** First and Second Pixels
      */
      *data_out++ = cb;
      *data_out++ = *y_ptr++;
      *data_out++ = cr;
      *data_out++ = *y_ptr++;
      /*
      ** Third and Fourth Pixels
      */
      *data2_out++ = cb;
      *data2_out++ = *yr2_ptr++;
      *data2_out++ = cr;
      *data2_out++ = *yr2_ptr++;
    } /* end col loop */
    y_ptr += w;
    yr2_ptr += w;
    data_out += dest_index;
    data2_out += dest_index;
  } /* end row loop */

  MSG_LOW("ipl_convert_ycrcb420lp_to_ycbcr422 marker_100\n");

  return IPL_SUCCESS;
} /* ipl_convert_ycrcb420lp_to_ycbcr422 */


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
)
{
  unsigned char *inImgPtr, *outImgPtr, *inClrPtr;
  uint32 x;
  uint8 cr;

  MSG_LOW("ipl_convert_ycrcb422lp_to_ycbcr422 marker_0\n");

  // do some checks on input to see if everything looks in order
  if (!input_img_ptr   || !input_img_ptr->imgPtr  ||
      !output_img_ptr  || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycrcb422lp_to_ycbcr422 marker_200\n");
    return IPL_FAILURE;
  }

  if (input_img_ptr->cFormat != IPL_YCrCb422_LINE_PK ||
      output_img_ptr->cFormat != IPL_YCbCr) 
  {
     MSG_LOW("ipl_convert_ycrcb422lp_to_ycbcr422 marker_201\n");
     return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycrcb422lp_to_ycbcr422 marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  inClrPtr = input_img_ptr->clrPtr;

  MSG_LOW("ipl_convert_ycrcb422lp_to_ycbcr422 marker_1\n");

  for (x = (output_img_ptr->dx*output_img_ptr->dy)>>1; x; x--) {
    /* Process 2 pixels at a time */
    cr = *inClrPtr++;
    /* Pixel 1: CbY */
    *outImgPtr++ = *inClrPtr++;
    *outImgPtr++ = *inImgPtr++;
    /* Pixel 2: CrY */
    *outImgPtr++ = cr;
    *outImgPtr++ = *inImgPtr++;
  } /* end x loop */

  MSG_LOW("ipl_convert_ycrcb422lp_to_ycbcr422 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycrcb422lp_to_ycbcr422 */





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb888_to_ycrcb420lp

DESCRIPTION
  This function converts from RGB888 to YCrCb 4:2:0 line packed format. 

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
static ipl_status_type ipl_convert_rgb888_to_ycrcb420lp
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
)
{
  uint32 x, y;
  register uint8* inImgPtr;
  register uint8* outImgPtr;
  register uint8* outClrPtr;
  uint8* in2ImgPtr;
  uint8* out2ImgPtr;
  uint8 r, g, b;
  int32 luma1, luma2, cb, cr;
  uint32 iw, ow, w, h;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};
  int oddWidth = 0, oddHeight = 0;

  MSG_LOW("ipl_convert_rgb888_to_ycrcb420lp marker_0\n");



  // do some checks on input to see if everything looks in order
  if (!i_img_ptr || !i_img_ptr->imgPtr  ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb888_to_ycrcb420lp marker_200\n");
    return IPL_FAILURE;
  }
  
  inImgPtr = (uint8*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;
  outClrPtr = o_img_ptr->clrPtr;
  iw = 3*i_img_ptr->dx;
  ow = o_img_ptr->dx;
  w = i_img_ptr->dx;
  h = i_img_ptr->dy;



  if (w % 2)
  {
    oddWidth = 1;
    w--;

    if (i_img_ptr->dx +1 != o_img_ptr->dx) 
    {
      MSG_LOW("ipl_convert_rgb888_to_ycrcb420lp marker_201\n");
      return IPL_FAILURE;
    }
  }

  if (h % 2)
  {
    oddHeight = 1;
    h++;
    if (i_img_ptr->dy+1 != o_img_ptr->dy)
    {
      MSG_LOW("ipl_convert_rgb888_to_ycrcb420lp marker_202\n");
      return IPL_FAILURE;
    }
  }


  in2ImgPtr = inImgPtr + iw;
  out2ImgPtr = outImgPtr + ow;

  MSG_LOW("ipl_convert_rgb888_to_ycrcb420lp marker_1\n");

  /* Loop through input image */
  for (y = h >> 1; y; y--) 
  {
    // this is the last row on an odd image.
    if (y == 1 && oddHeight)
    {
      in2ImgPtr = inImgPtr; 
    }

    for (x = w >>1; x; x--) 
    {
      /* Pixel 1 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      /* Pixel 2 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;
      luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                ycbcr_convert[2]*b)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      /* Output luma */
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma2;


      /* Pixel 3 */
      r = *in2ImgPtr++;
      g = *in2ImgPtr++;
      b = *in2ImgPtr++;
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      /* Pixel 4 */
      r = *in2ImgPtr++;
      g = *in2ImgPtr++;
      b = *in2ImgPtr++;
      luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);

      /* Output luma */
      *out2ImgPtr++ = (uint8)luma1;
      *out2ImgPtr++ = (uint8)luma2;

      /* Output chroma */
      *outClrPtr++ = (uint8)cr;
      *outClrPtr++ = (uint8)cb;
    } /* End of col loop */


    if (oddWidth)
    {
      /* Pixel 1 and 2 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      /* Output luma */
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma1;


      /* Pixel 3 and 4 */
      r = *in2ImgPtr++;
      g = *in2ImgPtr++;
      b = *in2ImgPtr++;
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);

      /* Output luma */
      *out2ImgPtr++ = (uint8)luma1;
      *out2ImgPtr++ = (uint8)luma1;

      /* Output chroma */
      *outClrPtr++ = (uint8)cr;
      *outClrPtr++ = (uint8)cb;
    }

    inImgPtr += iw;
    in2ImgPtr += iw;
    outImgPtr += ow;
    out2ImgPtr += ow;
  } /* End of row loop */

  MSG_LOW("ipl_convert_rgb888_to_ycrcb420lp marker_100\n");

  return IPL_SUCCESS;
} /* end of ipl_convert_rgb888_to_ycrcb420lp */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb888_to_ycrcb444

DESCRIPTION
  This function converts from RGB888 to YCbCr 4:4:4 format. 

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
static ipl_status_type ipl_convert_rgb888_to_ycbcr444
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
)
{
  uint32 x, y;
  register uint8* inImgPtr;
  register uint8* outImgPtr;
  uint8 r, g, b;
  int32 luma1, cb, cr;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_convert_rgb888_to_ycrcb444 marker_0\n");


  // do some checks on input to see if everything looks in order
  if (!i_img_ptr || !i_img_ptr->imgPtr  ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb888_to_ycrcb444 marker_200\n");
    return IPL_FAILURE;
  }
  
  inImgPtr = (uint8*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;

  MSG_LOW("ipl_convert_rgb888_to_ycrcb444 marker_1\n");

  /* Loop through input image */
  for (y = o_img_ptr->dy; y; y--) 
  {
    for (x = o_img_ptr->dx; x; x--) 
    {
      /* Pixel 1 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;

      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);

      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);

      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      *outImgPtr++ = (uint8)cb;
      *outImgPtr++ = (uint8)cr;
      *outImgPtr++ = (uint8)luma1;
    } 
  } 

  MSG_LOW("ipl_convert_rgb888_to_ycrcb444 marker_100\n");

  return IPL_SUCCESS;
} 


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb888_to_ycrcb422lp

DESCRIPTION
  This function converts from RGB888 to YCrCb 4:2:2 line packed format. 

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
static ipl_status_type ipl_convert_rgb888_to_ycrcb422lp
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
)
{
  uint32 x, y;
  register uint8* inImgPtr;
  register uint8* outImgPtr;
  register uint8* outClrPtr;
  uint8 r, g, b;
  int32 luma1, luma2, cb, cr;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};
  uint32 w;
  int oddWidth = 0; 

  MSG_LOW("ipl_convert_rgb888_to_ycrcb422lp marker_0\n");




  // do some checks on input to see if everything looks in order
  if (!i_img_ptr || !i_img_ptr->imgPtr  ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb888_to_ycrcb422lp marker_200\n");
    return IPL_FAILURE;
  }
  
  inImgPtr = (uint8*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;
  outClrPtr = o_img_ptr->clrPtr;
  w = i_img_ptr->dx;


  if (w % 2)
  {
    oddWidth = 1;
    w--;

    if (i_img_ptr->dx +1 != o_img_ptr->dx)
    {
      MSG_LOW("ipl_convert_rgb888_to_ycrcb422lp marker_201\n");
      return IPL_FAILURE;
    }
  }

  MSG_LOW("ipl_convert_rgb888_to_ycrcb422lp marker_1\n");

  /* Loop through input image */
  for (y = o_img_ptr->dy; y; y--) 
  {
    for (x = w>>1; x; x--) 
    {
      /* Pixel 1 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);

      /* Pixel 2 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;
      luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                ycbcr_convert[2]*b)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma2;
      *outClrPtr++ = (uint8)cr;
      *outClrPtr++ = (uint8)cb;
    } 


    if (oddWidth)
    {
      /* Pixel 1 and 2 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      /* Output luma */
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma1;
      *outClrPtr++ = (uint8)cr;
      *outClrPtr++ = (uint8)cb;
    }
  } 

  MSG_LOW("ipl_convert_rgb888_to_ycrcb422lp marker_100\n");

  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb888_to_ycbcr

DESCRIPTION
  This function converts from RGB888 to YCbCr 4:2:2 interleaved

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
static ipl_status_type ipl_convert_rgb888_to_ycbcr
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
)
{
  uint32 x, y;
  register uint8* inImgPtr;
  register uint8* outImgPtr;
  uint8 r, g, b;
  int32 luma1, luma2, cb, cr;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};
  uint32 w;
  int oddWidth = 0; 

  MSG_LOW("ipl_convert_rgb888_to_ycbcr marker_0\n");



  // do some checks on input to see if everything looks in order
  if (!i_img_ptr || !i_img_ptr->imgPtr  ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb888_to_ycbcr marker_200\n");
    return IPL_FAILURE;
  }
  
  inImgPtr = (uint8*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;
  w = i_img_ptr->dx;


  if (w % 2)
  {
    oddWidth = 1;
    w--;

    if (i_img_ptr->dx +1 != o_img_ptr->dx) 
    {
      MSG_LOW("ipl_convert_rgb888_to_ycbcr marker_201\n");
      return IPL_FAILURE;
    }
  }

  MSG_LOW("ipl_convert_rgb888_to_ycbcr marker_1\n");

  /* Loop through input image */
  for (y = o_img_ptr->dy; y; y--) 
  {
    for (x = w>>1; x; x--) 
    {
      /* Pixel 1 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);

      /* Pixel 2 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;
      luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                ycbcr_convert[2]*b)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      *outImgPtr++ = (uint8)cb;
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)cr;
      *outImgPtr++ = (uint8)luma2;
    } 


    if (oddWidth)
    {
      /* Pixel 1 and 2 */
      r = *inImgPtr++;
      g = *inImgPtr++;
      b = *inImgPtr++;
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      /* Output luma */
      *outImgPtr++ = (uint8)cb;
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)cr;
      *outImgPtr++ = (uint8)luma1;
    }
  } 

  MSG_LOW("ipl_convert_rgb888_to_ycbcr marker_100\n");

  return IPL_SUCCESS;
} 


#if 0

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb565_to_ycrcb420lp_fast

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
static extern ipl_status_type ipl_convert_rgb565_to_ycrcb420lp_fast
(
  ipl_image_type* i_img_ptr,       /* Points to the input image      */
  ipl_image_type* o_img_ptr        /* Points to the output image     */
)
{
  uint32 x, y;
  register uint16* inImgPtr;
  register uint8* outImgPtr;
  register uint8* outClrPtr;
  uint16* in2ImgPtr;
  uint8* out2ImgPtr;
  uint8 r, g, b;
  int32 luma1, luma2, cb, cr;
  uint32 iw, ow, w, h;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};
  int oddWidth = 0, oddHeight = 0;

  MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp_fast marker_0\n");



  // do some checks on input to see if everything looks in order
  if (!i_img_ptr || !i_img_ptr->imgPtr  ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp_fast marker_200\n");
    return IPL_FAILURE;
  }
  
  inImgPtr = (uint16*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;
  outClrPtr = o_img_ptr->clrPtr;
  iw = i_img_ptr->dx;
  ow = o_img_ptr->dx;
  w = i_img_ptr->dx;
  h = i_img_ptr->dy;


  if (w % 2)
  {
    oddWidth = 1;
    w--;

    if (i_img_ptr->dx +1 != o_img_ptr->dx) 
    {
      MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp_fast marker_201\n");
      return IPL_FAILURE;
    }
  }

  if (h % 2)
  {
    oddHeight = 1;
    h++;
    if (i_img_ptr->dy+1 != o_img_ptr->dy)
    {
      MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp_fast marker_202\n");
      return IPL_FAILURE;
    }
  }


  in2ImgPtr = inImgPtr + iw;
  out2ImgPtr = outImgPtr + ow;

  MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp_fast marker_1\n");

  /* Loop through input image */
  for (y = h >> 1; y; y--) 
  {
    // this is the last row on an odd image.
    if (y == 1 && oddHeight)
    {
      in2ImgPtr = inImgPtr; 
    }

    for (x = w >>1; x; x--) 
    {
      /* Pixel 1 */
      unpack_rgb565(*inImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      /* Pixel 2 */
      unpack_rgb565(*inImgPtr++,&r,&g,&b);
      luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                ycbcr_convert[2]*b)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      /* Output luma */
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma2;


      /* Pixel 3 */
      unpack_rgb565(*in2ImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      /* Pixel 4 */
      unpack_rgb565(*in2ImgPtr++,&r,&g,&b);
      luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);

      /* Output luma */
      *out2ImgPtr++ = (uint8)luma1;
      *out2ImgPtr++ = (uint8)luma2;

      /* Output chroma */
      *outClrPtr++ = (uint8)cr;
      *outClrPtr++ = (uint8)cb;
    } /* End of col loop */


    if (oddWidth)
    {
      /* Pixel 1 and 2 */
      unpack_rgb565(*inImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      /* Output luma */
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma1;


      /* Pixel 3 and 4 */
      unpack_rgb565(*in2ImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);

      /* Output luma */
      *out2ImgPtr++ = (uint8)luma1;
      *out2ImgPtr++ = (uint8)luma1;

      /* Output chroma */
      *outClrPtr++ = (uint8)cr;
      *outClrPtr++ = (uint8)cb;
    }

    inImgPtr += iw;
    in2ImgPtr += iw;
    outImgPtr += ow;
    out2ImgPtr += ow;
  } /* End of row loop */

  MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp_fast marker_100\n");

  return IPL_SUCCESS;
} /* end of ipl_convert_rgb565_to_ycrcb420lp */


#endif


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb565_to_ycrcb420lp

DESCRIPTION
  This function converts from RGB565 to YCrCb 4:2:0 line packed format. 
  This uses HIGH QUALITY color conversion.

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
)
{
  uint32 x, y;
  register uint16* inImgPtr;
  register uint8* outImgPtr;
  register uint8* outClrPtr;
  uint16* in2ImgPtr;
  uint8* out2ImgPtr;
  uint8 r, g, b;
  int32 luma1, luma2, cb, cr;
  uint32 iw, ow, w, h;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};
  int oddWidth = 0, oddHeight = 0;

  MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp marker_0\n");



  // do some checks on input to see if everything looks in order
  if (!i_img_ptr || !i_img_ptr->imgPtr  ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp marker_200\n");
    return IPL_FAILURE;
  }
  
  inImgPtr = (uint16*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;
  outClrPtr = o_img_ptr->clrPtr;
  iw = i_img_ptr->dx;
  ow = o_img_ptr->dx;
  w = i_img_ptr->dx;
  h = i_img_ptr->dy;



  if (w % 2)
  {
    oddWidth = 1;
    w--;

    if (i_img_ptr->dx +1 != o_img_ptr->dx) 
    {
      MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp marker_201\n");
      return IPL_FAILURE;
    }
  }

  if (h % 2)
  {
    oddHeight = 1;
    h++;
    if (i_img_ptr->dy+1 != o_img_ptr->dy) 
    {
      MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp marker_202\n");
      return IPL_FAILURE;
    }
  }


  in2ImgPtr = inImgPtr + iw;
  out2ImgPtr = outImgPtr + ow;

  MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp marker_1\n");

  /* Loop through input image */
  for (y = h >> 1; y; y--) 
  {
    // this is the last row on an odd image.
    if (y == 1 && oddHeight)
    {
      in2ImgPtr = inImgPtr; 
    }

    for (x = w >>1; x; x--) 
    {
      /* Pixel 1 */
      unpack_rgb565(*inImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;

      /* Pixel 2 */
      unpack_rgb565(*inImgPtr++,&r,&g,&b);
      luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                ycbcr_convert[2]*b)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cr += (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
             ycbcr_convert[8]*b)*4+0x8000;
      cb += (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
             ycbcr_convert[5]*b)*4+0x8000;

      /* Output luma */
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma2;


      /* Pixel 3 */
      unpack_rgb565(*in2ImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr += (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
             ycbcr_convert[8]*b)*4+0x8000;
      cb += (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
             ycbcr_convert[5]*b)*4+0x8000;

      /* Pixel 4 */
      unpack_rgb565(*in2ImgPtr++,&r,&g,&b);
      luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cr += (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
             ycbcr_convert[8]*b)*4+0x8000;
      cb += (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
             ycbcr_convert[5]*b)*4+0x8000;

      /* Output luma */
      *out2ImgPtr++ = (uint8)luma1;
      *out2ImgPtr++ = (uint8)luma2;

      /* Output chroma */
      // shift twice to average, 16 time for Q 
      cr = (cr>>18) + 128;
      cr = CLIPIT(cr);
      *outClrPtr++ = (uint8)cr;

      cb = (cb>>18) + 128;
      cb = CLIPIT(cb);
      *outClrPtr++ = (uint8)cb;
    } /* End of col loop */


    if (oddWidth)
    {
      /* Pixel 1 and 2 */
      unpack_rgb565(*inImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;

      /* Output luma */
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma1;


      /* Pixel 3 and 4 */
      unpack_rgb565(*in2ImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr += (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
             ycbcr_convert[8]*b)*4+0x8000;
      cb += (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
             ycbcr_convert[5]*b)*4+0x8000;

      /* Output luma */
      *out2ImgPtr++ = (uint8)luma1;
      *out2ImgPtr++ = (uint8)luma1;

      /* Output chroma */
      // shift once to average, 16 time for Q 
      cb = (cb>>17) + 128;
      cb = CLIPIT(cb);

      cr = (cr>>17) + 128;
      cr = CLIPIT(cr);

      *outClrPtr++ = (uint8)cr;
      *outClrPtr++ = (uint8)cb;
    }

    inImgPtr += iw;
    in2ImgPtr += iw;
    outImgPtr += ow;
    out2ImgPtr += ow;
  } /* End of row loop */

  MSG_LOW("ipl_convert_rgb565_to_ycrcb420lp marker_100\n");

  return IPL_SUCCESS;
} /* end of ipl_convert_rgb565_to_ycrcb420lp */




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
)
{
  uint32 x, y;
  register uint16* inImgPtr;
  register uint8* outImgPtr;
  register uint8* outClrPtr;
  uint8 r, g, b;
  int32 luma1, luma2, cb, cr;
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};
  uint32 w;
  int oddWidth = 0; 

  MSG_LOW("ipl_convert_rgb565_to_ycrcb422lp marker_0\n");



  // do some checks on input to see if everything looks in order
  if (!i_img_ptr || !i_img_ptr->imgPtr  ||
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb565_to_ycrcb422lp marker_200\n");
    return IPL_FAILURE;
  }
  
  inImgPtr = (uint16*)i_img_ptr->imgPtr;
  outImgPtr = o_img_ptr->imgPtr;
  outClrPtr = o_img_ptr->clrPtr;
  w = i_img_ptr->dx;


  if (w % 2)
  {
    oddWidth = 1;
    w--;

    if (i_img_ptr->dx +1 != o_img_ptr->dx)
    {
      MSG_LOW("ipl_convert_rgb565_to_ycrcb422lp marker_201\n");
      return IPL_FAILURE;
    }
  }

  MSG_LOW("ipl_convert_rgb565_to_ycrcb422lp marker_1\n");

  /* Loop through input image */
  for (y = o_img_ptr->dy; y; y--) 
  {
    for (x = w>>1; x; x--) 
    {
      /* Pixel 1 */
      unpack_rgb565(*inImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;


      /* Pixel 2 */
      unpack_rgb565(*inImgPtr++,&r,&g,&b);
      luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                ycbcr_convert[2]*b)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cb += (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cr += (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;

      // shift once for avg, 16 times for Q number
      cr = (cr>>17) + 128;
      cr = CLIPIT(cr);
      cb = (cb>>17) + 128;
      cb = CLIPIT(cb);

      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma2;
      *outClrPtr++ = (uint8)cr;
      *outClrPtr++ = (uint8)cb;
    } 


    if (oddWidth)
    {
      /* Pixel 1 and 2 */
      unpack_rgb565(*inImgPtr++,&r,&g,&b);
      luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
               ycbcr_convert[2]*b)*4+0x8000;
      luma1 = (luma1 >> 16) + 16;
      luma1 = CLIPIT(luma1);
      cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
            ycbcr_convert[8]*b)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
            ycbcr_convert[5]*b)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);

      /* Output luma */
      *outImgPtr++ = (uint8)luma1;
      *outImgPtr++ = (uint8)luma1;
      *outClrPtr++ = (uint8)cr;
      *outClrPtr++ = (uint8)cb;
    }
  } 

  MSG_LOW("ipl_convert_rgb565_to_ycrcb422lp marker_100\n");

  return IPL_SUCCESS;
} /* end of ipl_convert_rgb565_to_ycrcb422lp */



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
)
{
  register uint8 *ptr;
  register uint8 temp;
  uint32 swapTimes, swapCount, index;

  MSG_LOW("ipl_convert_swap_chroma_order marker_0\n");



  // do some checks on input to see if everything looks in order
  if (!input_img_ptr || !input_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_swap_chroma_order marker_200\n");
    return IPL_FAILURE;
  }
  
  if (input_img_ptr->cFormat == IPL_YCbCr420_LINE_PK ||
      input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    swapTimes = input_img_ptr->dx*input_img_ptr->dy/4; 
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr422_LINE_PK ||
           input_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
  {
    swapTimes = input_img_ptr->dx*input_img_ptr->dy/2;
  }
  else
  {
    MSG_LOW("ipl_convert_swap_chroma_order marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_swap_chroma_order marker_1\n");

  ptr = input_img_ptr->clrPtr;
  for (swapCount = 0, index = 0; swapCount < swapTimes; swapCount++, index += 2)
  {
    temp = ptr[index];
    ptr[index] = ptr[index+1];
    ptr[index+1] = temp;
  }

  MSG_LOW("ipl_convert_swap_chroma_order marker_100\n");

  return IPL_SUCCESS;
}


#if 0

/* <EJECT> */
/*==========================================================================

FUNCTION    ipl_convert_ycrcb420lp_to_rgb565_fast

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
extern ipl_status_type ipl_convert_ycrcb420lp_to_rgb565_fast
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr;
  register uint32 out, out2, out3, out4;
  register uint8 cb, cr;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register uint16 *rTable;
  register uint16 *gTable;
  register uint16 *bTable;
  uint32 input_row_size;
  uint32 output_row_size;
  uint8 *cb_ptr;
  uint32 row, col;
  int32 pitchx;
  boolean oddWidth = 0;
  uint32 * y1Ptr;
  uint32 * y2Ptr;
  uint32 luma1;
  uint32 luma2;
  int i;


  // do some checks on input to see if everything looks in order
  //
  if (!input_img_ptr || !input_img_ptr->imgPtr  ||
      !output_img_ptr || !output_img_ptr->imgPtr)
    return IPL_FAILURE;


  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  input_row_size = input_img_ptr->dx;
  output_row_size = output_img_ptr->dx;
  cb_ptr = input_img_ptr->clrPtr;



  /* output has to be smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
    return IPL_FAILURE;

  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  /*--------------------------------------------------------------------
        Initialize RGB565 conversion table
  --------------------------------------------------------------------*/
  rTable = ipl2_r5xx;
  gTable = ipl2_gx6x;
  bTable = ipl2_bxx5;

  /*------------------------------------------------------------------------
      Process 2 rows at a time, so only need half the number of row 
      iterations
  ------------------------------------------------------------------------*/

  for (row = input_img_ptr->dy/2; row; row--)
  {
    /*----------------------------------------------------------------------
      Process 2 columns at a time, so only need half the number of 
      column iterations
    ----------------------------------------------------------------------*/
    //for (col = (output_row_size - oddWidth)/2; col; col--)
    for (col = (output_row_size - oddWidth)/4; col; col--)
    {
#if 0
      y1Ptr = (uint32 *) inputImgPtr;        
      y2Ptr = (uint32 *) (inputImgPtr + input_row_size);        

      luma1 = *y1Ptr;
      luma2 = *y2Ptr;

      for (i = 2; i; i--)
      {
        if (i == 2)
        {
          lumaa1 = (luma1 & 0xFF000000) >> 24; 
          lumaa2 = (luma1 & 0x00FF0000) >> 16; 
          lumaa3 = (luma2 & 0xFF000000) >> 24; 
          lumaa4 = (luma2 & 0x00FF0000) >> 16; 
        }
        else
        {
          lumaa1 = (luma1 & 0x0000FF00) >> 8; 
          lumaa2 = (luma1 & 0x000000FF); 
          lumaa3 = (luma2 & 0x0000FF00) >> 8; 
          lumaa4 = (luma2 & 0x000000FF); 
        }
#else
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;        /* corresponds to Y1 */
      lumaa2 = *(inputImgPtr + 1);  /* corresponds to Y2 */

      /*--------------------------------------------------------------------
            The following gets you Y3 and Y4 and increments the pointer
      --------------------------------------------------------------------*/
      lumaa3 = *(inputImgPtr++ + input_row_size);
      lumaa4 = *(inputImgPtr++ + input_row_size);


      //printf("match old way        %d,%d,%d,%d\n", 
      //       lumaa1, lumaa2, lumaa3, lumaa4);
#endif

      /*--------------------------------------------------------------------
          Get Cr and Cb for all 4 Y values
      --------------------------------------------------------------------*/
      cr = *cb_ptr++;
      cb = *cb_ptr++;

      /*--------------------------------------------------------------------
        Find the delta/difference between Y3 and Y4. This is for faster
        computation as you will see later. We are doing this in reverse 
        order because we will lose the original delta if we do it in 
        the normal order.
      --------------------------------------------------------------------*/
      lumaa4 -= lumaa3;

      /*--------------------------------------------------------------------
        Find the delta between Y2 to Y3. This is for faster computation,
        as you will see later.
      --------------------------------------------------------------------*/
      lumaa3 -= lumaa2;

      /*--------------------------------------------------------------------
        Find the delta between Y1 to Y2. This is for faster computation,
        as you will see later.
      --------------------------------------------------------------------*/
      lumaa2 -= lumaa1;


      /*--------------------------------------------------------------------
        Now do color conversion from YCbCr to RGB domain
      --------------------------------------------------------------------*/

      /* 
      ** R values 
      */
      r = lumaa1 + ipl2_Cr2RTable[cr];
      /*--------------------------------------------------------------------
          If r is negative, clip to 0
      --------------------------------------------------------------------*/
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out = rTable[ r ];
      }
      else
      {
        out = 0;
      }

      /*--------------------------------------------------------------------
          Add the delta in Y2 to calculate the next R value
      --------------------------------------------------------------------*/
      r += lumaa2;

      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out2 = rTable[ r ];
      }
      else
      {
        out2 = 0;
      }

      /*--------------------------------------------------------------------
          Add the delta in Y3 to calculate R3
      --------------------------------------------------------------------*/
      r += lumaa3;

      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out3 = rTable[ r ];
      }
      else
      {
        out3 = 0;
      }

      /*--------------------------------------------------------------------
          Add the delta in Y4 to calculate R4
      --------------------------------------------------------------------*/
      r += lumaa4;

      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out4 = rTable[ r ];
      }
      else
      {
        out4 = 0;
      }


      /* 
      ** G values 
      */
      r = lumaa1 - ipl2_Cb2GTable[cb] - ipl2_Cr2GTable[cr];
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out += gTable[r];
      }

      r += lumaa2;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out2 += gTable[r];
      }

      r += lumaa3;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out3 += gTable[r];
      }

      r += lumaa4;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out4 += gTable[r];
      }


      /* 
      ** B values 
      */
      r = lumaa1 + ipl2_Cb2BTable[cb];
      /*--------------------------------------------------------------------
         If we have a nonpositive value, we don't need to do anything since 
         the translation or effect of that component on the whole pixel
         value is bound to be zero.
      --------------------------------------------------------------------*/
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out += bTable[r];
      }

      r += lumaa2;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out2 += bTable[r];
      }

      r += lumaa3;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out3 += bTable[r];
      }

      r += lumaa4;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out4 += bTable[r];
      }

      *outputImgPtr = (uint16) out;
      *(outputImgPtr + 1) = (uint16) out2;
      *(outputImgPtr++ + output_row_size) = (uint16) out3;
      *(outputImgPtr++ + output_row_size) = (uint16) out4;



      //}



    } /* end col loop */


    /*----------------------------------------------------------------------
        Add on row increments
    ----------------------------------------------------------------------*/
    outputImgPtr = (uint16*) ((uint8*) outputImgPtr + 
      ((oddWidth + output_row_size) << 1));
    inputImgPtr += (input_row_size + oddWidth + pitchx);
    cb_ptr += (pitchx + oddWidth);
  } /* end row loop */


  /* fill in last row since we skipped it earliear */
  if (oddWidth)
  {
    /* leave this outer for loop in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = (uint16*) output_img_ptr->imgPtr + 
        (output_row_size - 1 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy/2; row; row--)
      {
        lumaa1 = *inputImgPtr;        
        inputImgPtr += input_row_size;
        lumaa2 = *inputImgPtr;        
        inputImgPtr += input_row_size;

        lumaa2 -= lumaa1;

        cr = *cb_ptr;
        cb = *cb_ptr+1;
        cb_ptr += (input_row_size);

        /* R values */
        r = lumaa1 + ipl2_Cr2RTable[cr];
        if (r > 255) r = 255;
        if ( r > 0 )
          out = rTable[r];
        else
          out = 0;
        r += lumaa2;
        if (r > 255) r = 255;
        if ( r > 0 )
          out2 = rTable[r];
        else
          out2 = 0;

        /* G values */
        r = lumaa1 - ipl2_Cb2GTable[cb] - ipl2_Cr2GTable[cr];
        if (r > 255) r = 255;
        if ( r > 0 )
          out += gTable[r];
        r += lumaa2;
        if (r > 255) r = 255;
        if ( r > 0 )
          out2 += gTable[r];

        /* B values */
        r = lumaa1 + ipl2_Cb2BTable[cb];
        if (r > 255) r = 255;
        if ( r > 0 )
          out += bTable[r];
        r += lumaa2;
        if (r > 255) r = 255;
        if ( r > 0 )
          out2 += bTable[r];

        *outputImgPtr = (uint16) out;
        outputImgPtr += output_row_size;
        *outputImgPtr = (uint16) out2;
        outputImgPtr += output_row_size;
      }
    }
  }

  
  return IPL_SUCCESS;
} /* end of function ipl_convert_ycrcb420lp_to_rgb565 */


#endif




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
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr;
  register uint32 out, out2, out3, out4;
  register uint8 cb, cr;
  register int32 lumaa1, lumaa2, lumaa3, lumaa4;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  uint32 input_row_size;
  uint32 output_row_size;
  uint8 *cb_ptr;
  uint32 row, col;
  int32 pitchx;
  boolean oddWidth = 0;

  MSG_LOW("ipl2_convert_ycrcb420lp_to_rgb565 marker_0\n");


  // do some checks on input to see if everything looks in order
  //
  if (!input_img_ptr || !input_img_ptr->imgPtr  ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl2_convert_ycrcb420lp_to_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  input_row_size = input_img_ptr->dx;
  output_row_size = output_img_ptr->dx;
  cb_ptr = input_img_ptr->clrPtr;



  /* output has to be smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
  {
    MSG_LOW("ipl2_convert_ycrcb420lp_to_rgb565 marker_201\n");
    return IPL_FAILURE;
  }

  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  /*--------------------------------------------------------------------
        Initialize RGB565 conversion table
  --------------------------------------------------------------------*/
  rTable = ipl2_r5xx;
  gTable = ipl2_gx6x;
  bTable = ipl2_bxx5;

  /*------------------------------------------------------------------------
      Process 2 rows at a time, so only need half the number of row 
      iterations
  ------------------------------------------------------------------------*/
  MSG_LOW("ipl2_convert_ycrcb420lp_to_rgb565 marker_1\n");

  for (row = input_img_ptr->dy/2; row; row--)
  {
    /*----------------------------------------------------------------------
      Process 2 columns at a time, so only need half the number of 
      column iterations
    ----------------------------------------------------------------------*/
    for (col = (output_row_size - oddWidth)/2; col; col--)
    {
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;        /* corresponds to Y1 */
      lumaa2 = *(inputImgPtr + 1);  /* corresponds to Y2 */

      /*--------------------------------------------------------------------
            The following gets you Y3 and Y4 and increments the pointer
      --------------------------------------------------------------------*/
      lumaa3 = *(inputImgPtr++ + input_row_size);
      lumaa4 = *(inputImgPtr++ + input_row_size);

      /*--------------------------------------------------------------------
          Get Cr and Cb for all 4 Y values
      --------------------------------------------------------------------*/
      cr = *cb_ptr++;
      cb = *cb_ptr++;

      /*--------------------------------------------------------------------
        Find the delta/difference between Y3 and Y4. This is for faster
        computation as you will see later. We are doing this in reverse 
        order because we will lose the original delta if we do it in 
        the normal order.
      --------------------------------------------------------------------*/
      lumaa4 -= lumaa3;

      /*--------------------------------------------------------------------
        Find the delta between Y2 to Y3. This is for faster computation,
        as you will see later.
      --------------------------------------------------------------------*/
      lumaa3 -= lumaa2;

      /*--------------------------------------------------------------------
        Find the delta between Y1 to Y2. This is for faster computation,
        as you will see later.
      --------------------------------------------------------------------*/
      lumaa2 -= lumaa1;


      /*--------------------------------------------------------------------
        Now do color conversion from YCbCr to RGB domain
      --------------------------------------------------------------------*/

      /* 
      ** R values 
      */
      r = lumaa1 + ipl2_Cr2RTable[cr];
      /*--------------------------------------------------------------------
          If r is negative, clip to 0
      --------------------------------------------------------------------*/
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out = rTable[ r ];
      }
      else
      {
        out = 0;
      }

      /*--------------------------------------------------------------------
          Add the delta in Y2 to calculate the next R value
      --------------------------------------------------------------------*/
      r += lumaa2;

      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out2 = rTable[ r ];
      }
      else
      {
        out2 = 0;
      }

      /*--------------------------------------------------------------------
          Add the delta in Y3 to calculate R3
      --------------------------------------------------------------------*/
      r += lumaa3;

      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out3 = rTable[ r ];
      }
      else
      {
        out3 = 0;
      }

      /*--------------------------------------------------------------------
          Add the delta in Y4 to calculate R4
      --------------------------------------------------------------------*/
      r += lumaa4;

      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out4 = rTable[ r ];
      }
      else
      {
        out4 = 0;
      }


      /* 
      ** G values 
      */
      r = lumaa1 - ipl2_Cb2GTable[cb] - ipl2_Cr2GTable[cr];
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out += gTable[r];
      }

      r += lumaa2;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out2 += gTable[r];
      }

      r += lumaa3;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out3 += gTable[r];
      }

      r += lumaa4;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out4 += gTable[r];
      }


      /* 
      ** B values 
      */
      r = lumaa1 + ipl2_Cb2BTable[cb];
      /*--------------------------------------------------------------------
         If we have a nonpositive value, we don't need to do anything since 
         the translation or effect of that component on the whole pixel
         value is bound to be zero.
      --------------------------------------------------------------------*/
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out += bTable[r];
      }

      r += lumaa2;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out2 += bTable[r];
      }

      r += lumaa3;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out3 += bTable[r];
      }

      r += lumaa4;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out4 += bTable[r];
      }

      *outputImgPtr = (uint16) out;
      *(outputImgPtr + 1) = (uint16) out2;
      *(outputImgPtr++ + output_row_size) = (uint16) out3;
      *(outputImgPtr++ + output_row_size) = (uint16) out4;
    } /* end col loop */


    /*----------------------------------------------------------------------
        Add on row increments
    ----------------------------------------------------------------------*/
    outputImgPtr = (uint16*) ((uint8*) outputImgPtr + 
      ((oddWidth + output_row_size) << 1));
    inputImgPtr += (input_row_size + oddWidth + pitchx);
    cb_ptr += (pitchx + oddWidth);
  } /* end row loop */

  /* fill in last row since we skipped it earliear */
  if (oddWidth)
  {
  
    /* leave this outer for loop in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = (uint16*) output_img_ptr->imgPtr + 
        (output_row_size - 1 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy/2; row; row--)
      {
        lumaa1 = *inputImgPtr;        
        inputImgPtr += input_row_size;
        lumaa2 = *inputImgPtr;        
        inputImgPtr += input_row_size;

        lumaa2 -= lumaa1;

        cr = *cb_ptr;
        cb = *cb_ptr+1;
        cb_ptr += (input_row_size);

        /* R values */
        r = lumaa1 + ipl2_Cr2RTable[cr];
        if (r > 255) r = 255;
        if ( r > 0 )
          out = rTable[r];
        else
          out = 0;
        r += lumaa2;
        if (r > 255) r = 255;
        if ( r > 0 )
          out2 = rTable[r];
        else
          out2 = 0;

        /* G values */
        r = lumaa1 - ipl2_Cb2GTable[cb] - ipl2_Cr2GTable[cr];
        if (r > 255) r = 255;
        if ( r > 0 )
          out += gTable[r];
        r += lumaa2;
        if (r > 255) r = 255;
        if ( r > 0 )
          out2 += gTable[r];

        /* B values */
        r = lumaa1 + ipl2_Cb2BTable[cb];
        if (r > 255) r = 255;
        if ( r > 0 )
          out += bTable[r];
        r += lumaa2;
        if (r > 255) r = 255;
        if ( r > 0 )
          out2 += bTable[r];

        *outputImgPtr = (uint16) out;
        outputImgPtr += output_row_size;
        *outputImgPtr = (uint16) out2;
        outputImgPtr += output_row_size;
      }
    }
  }

  MSG_LOW("ipl2_convert_ycrcb420lp_to_rgb565 marker_100\n");

  return IPL_SUCCESS;
} /* end of function ipl_convert_ycrcb420lp_to_rgb565 */





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
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr;
  register uint32 out, out2;
  register uint8 cb, cr;
  register int32 lumaa1, lumaa2;
  register int32 r;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;
  uint32 output_row_size;
  uint32 input_row_size;
  uint8 *cb_ptr;
  uint32 row, col;
  int32 pitchx;
  boolean oddWidth = 0;

  MSG_LOW("ipl2_convert_ycrcb422lp_to_rgb565 marker_0\n");



  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl2_convert_ycrcb422lp_to_rgb565 marker_200\n");
    return IPL_FAILURE;
  }
  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  output_row_size = output_img_ptr->dx;
  input_row_size = input_img_ptr->dx;
  cb_ptr = input_img_ptr->clrPtr;


  /* output has to be smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
  {
    MSG_LOW("ipl2_convert_ycrcb422lp_to_rgb565 marker_201\n");
    return IPL_FAILURE;
  }
  
  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  /*--------------------------------------------------------------------
        Initialize RGB565 conversion table
  --------------------------------------------------------------------*/
  rTable = ipl2_r5xx;
  gTable = ipl2_gx6x;
  bTable = ipl2_bxx5;

  MSG_LOW("ipl2_convert_ycrcb422lp_to_rgb565 marker_1\n");

  /*------------------------------------------------------------------------
      Process 2 rows at a time, so only need half the number of row 
      iterations
  ------------------------------------------------------------------------*/
  for (row = input_img_ptr->dy; row; row--)
  {
    /*----------------------------------------------------------------------
      Process 2 columns at a time, so only need half the number of 
      column iterations
    ----------------------------------------------------------------------*/
    for (col = (output_row_size - oddWidth)/2; col; col--)
    {
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      lumaa1 = *inputImgPtr;        /* corresponds to Y1 */
      lumaa2 = *(inputImgPtr + 1);  /* corresponds to Y2 */
      inputImgPtr++;
      inputImgPtr++;

      /*--------------------------------------------------------------------
          Get Cr and Cb for all 4 Y values
      --------------------------------------------------------------------*/
      cr = *cb_ptr++;
      cb = *cb_ptr++;

      /*--------------------------------------------------------------------
        Find the delta between Y1 to Y2. This is for faster computation,
        as you will see later.
      --------------------------------------------------------------------*/
      lumaa2 -= lumaa1;


      /*--------------------------------------------------------------------
        Now do color conversion from YCbCr to RGB domain
      --------------------------------------------------------------------*/

      /* 
      ** R values 
      */
      r = lumaa1 + ipl2_Cr2RTable[cr];
      /*--------------------------------------------------------------------
          If r is negative, clip to 0
      --------------------------------------------------------------------*/
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out = rTable[ r ];
      }
      else
      {
        out = 0;
      }

      /*--------------------------------------------------------------------
          Add the delta in Y2 to calculate the next R value
      --------------------------------------------------------------------*/
      r += lumaa2;

      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out2 = rTable[ r ];
      }
      else
      {
        out2 = 0;
      }


      /* 
      ** G values 
      */
      r = lumaa1 - ipl2_Cb2GTable[cb] - ipl2_Cr2GTable[cr];
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out += gTable[r];
      }

      r += lumaa2;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out2 += gTable[r];
      }

      /* 
      ** B values 
      */
      r = lumaa1 + ipl2_Cb2BTable[cb];
      /*--------------------------------------------------------------------
         If we have a nonpositive value, we don't need to do anything since 
         the translation or effect of that component on the whole pixel
         value is bound to be zero.
      --------------------------------------------------------------------*/
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out += bTable[r];
      }

      r += lumaa2;
      if (r > 255) r = 255;
      if ( r > 0 )
      {
        out2 += bTable[r];
      }

      *outputImgPtr++ = (uint16) out;
      *outputImgPtr++ = (uint16) out2;
    } /* end col loop */

    outputImgPtr += oddWidth;
    inputImgPtr += (pitchx + oddWidth);
    cb_ptr += (pitchx + oddWidth);

  } /* end row loop */
 
  /* fill in last row since we skipped it earliear */
  if (oddWidth)
  {
    /* leave this in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = (uint16*) output_img_ptr->imgPtr + 
        (output_row_size - 1 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy; row; row--)
      {
        lumaa1 = *inputImgPtr;        
        inputImgPtr += input_row_size;

        cr = *cb_ptr;
        cb = *cb_ptr+1;
        cb_ptr += input_row_size;

        /* R values */
        r = lumaa1 + ipl2_Cr2RTable[cr];
        if (r > 255) r = 255;
        if ( r > 0 )
          out = rTable[r];
        else
          out = 0;

        /* G values */
        r = lumaa1 - ipl2_Cb2GTable[cb] - ipl2_Cr2GTable[cr];
        if (r > 255) r = 255;
        if ( r > 0 )
          out += gTable[r];

        /* B values */
        r = lumaa1 + ipl2_Cb2BTable[cb];
        if (r > 255) r = 255;
        if ( r > 0 )
          out += bTable[r];
        *outputImgPtr = (uint16) out;
        outputImgPtr += output_row_size;
      }
    }
  }

  MSG_LOW("ipl2_convert_ycrcb422lp_to_rgb565 marker_100\n");

  return IPL_SUCCESS;
} 



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
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr;
  register uint8 cb, cr;
  register int32 luma;
  register int32 r,g,b;
  int32 rc, gc, bc; 
  uint32 input_row_size;
  uint32 output_row_size;
  uint8 *cb_ptr;
  uint32 row, col;
  int32 pitchx;
  boolean oddWidth = 0;
  short ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};

  MSG_LOW("ipl_convert_ycrcb420lp_to_rgb565 marker_0\n");




  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycrcb420lp_to_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  input_row_size = input_img_ptr->dx;
  output_row_size = output_img_ptr->dx;
  cb_ptr = input_img_ptr->clrPtr;

  /* output has to be smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
  {
    MSG_LOW("ipl_convert_ycrcb420lp_to_rgb565 marker_201\n");
    return IPL_FAILURE;
  }

  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  MSG_LOW("ipl_convert_ycrcb420lp_to_rgb565 marker_1\n");

  for (row = input_img_ptr->dy/2; row; row--)
  {
    for (col = (output_row_size - oddWidth)/2; col; col--)
    {
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/

      cr = *cb_ptr++;
      cb = *cb_ptr++;
      rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
      gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
      bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;

      luma = *inputImgPtr;        /* corresponds to Y1 */
      r = luma + (rc>>16);
      g = luma + (gc>>16);
      b = luma + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *(outputImgPtr) = pack_rgb565(r,g,b);

      luma = *(inputImgPtr + 1);  /* corresponds to Y2 */
      r = luma + (rc>>16);
      g = luma + (gc>>16);
      b = luma + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *(outputImgPtr + 1) = pack_rgb565(r,g,b);

      luma = *(inputImgPtr++ + input_row_size);
      r = luma + (rc>>16);
      g = luma + (gc>>16);
      b = luma + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *(outputImgPtr++ + output_row_size) = pack_rgb565(r,g,b);

      luma = *(inputImgPtr++ + input_row_size);
      r = luma + (rc>>16);
      g = luma + (gc>>16);
      b = luma + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *(outputImgPtr++ + output_row_size) = pack_rgb565(r,g,b);
    } /* end col loop */


    /*----------------------------------------------------------------------
        Add on row increments
    ----------------------------------------------------------------------*/
    outputImgPtr = (uint16*) ((uint8*) outputImgPtr + 
      ((oddWidth + output_row_size) << 1));
    inputImgPtr += (input_row_size + oddWidth + pitchx);
    cb_ptr += (pitchx + oddWidth);
  } /* end row loop */
  
  /* fill in last row since we skipped it earliear */
  if (oddWidth)
  {
    /* leave this outer for loop in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = (uint16*) output_img_ptr->imgPtr + 
        (output_row_size - 1 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy/2; row; row--)
      {
        cr = *cb_ptr;
        cb = *cb_ptr+1;
        cb_ptr += (input_row_size);
        rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
        gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
        bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = luma + (rc>>16);
        g = luma + (gc>>16);
        b = luma + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        *(outputImgPtr) = pack_rgb565(r,g,b);
        outputImgPtr += output_row_size;

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = luma + (rc>>16);
        g = luma + (gc>>16);
        b = luma + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        *(outputImgPtr) = pack_rgb565(r,g,b);
        outputImgPtr += output_row_size;
      }
    }
  }
 
  MSG_LOW("ipl_convert_ycrcb420lp_to_rgb565 marker_100\n");

  return IPL_SUCCESS;
} /* end of function ipl_convert_ycrcb420lp_to_rgb565 */



/* <EJECT> */
/*==========================================================================

FUNCTION    ipl3_convert_ycbcr422lp_to_rgb888

DESCRIPTION
  This function converts from YCbCr 4:2:2 line packed format to RGB888 

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
static ipl_status_type ipl_convert_ycbcr422lp_to_rgb888
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
)
{
  register unsigned char* inputImgPtr;
  register uint8* outputImgPtr;
  register uint8 cb, cr;
  register int32 luma;
  uint8   r,g,b;
  uint32 input_row_size;
  uint32 output_row_size;
  uint8 *cb_ptr;
  uint32 row, col;
  int32 pitchx;
  int oddWidth = 0;

  MSG_LOW("ipl_convert_ycbcr422lp_to_rgb888 marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr422lp_to_rgb888 marker_200\n");
    return IPL_FAILURE;
  }

  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = output_img_ptr->imgPtr;

  input_row_size = input_img_ptr->dx;
  output_row_size = 3*output_img_ptr->dx;
  cb_ptr = input_img_ptr->clrPtr;

  /* output can smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
  {
    MSG_LOW("ipl_convert_ycbcr422lp_to_rgb888 marker_201\n");
    return IPL_FAILURE;
  }

  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  MSG_LOW("ipl_convert_ycbcr422lp_to_rgb888 marker_1\n");


  for (row = input_img_ptr->dy; row; row--)
  {
    for (col = (output_img_ptr->dx - oddWidth)/2; col; col--)
    {
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   
      --------------------------------------------------------------------*/
      cb = *cb_ptr++;
      cr = *cb_ptr++;

      luma = *inputImgPtr++;        /* corresponds to Y1 */
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));
      *outputImgPtr++   = r;
      *outputImgPtr++ = g;
      *outputImgPtr++ = b;

      luma = *inputImgPtr++;  /* corresponds to Y2 */
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));

      *outputImgPtr++ = r;
      *outputImgPtr++ = g;
      *outputImgPtr++ = b;
    } /* end col loop */


    /*----------------------------------------------------------------------
        Add on row increments
    ----------------------------------------------------------------------*/
    outputImgPtr += (3*oddWidth);
    inputImgPtr += (oddWidth + pitchx);
    cb_ptr += (pitchx + oddWidth);
  } 
 

  /* fill in last row since we skipped it earliear */
  if (oddWidth)
  {
    /* leave this outer for loop in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = output_img_ptr->imgPtr + (output_row_size - 3 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy; row; row--)
      {
        cb = *cb_ptr;
        cr = *cb_ptr+1;
        cb_ptr += (input_row_size);

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = CLIPIT((luma + ipl_crr[cr]));
        g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
        b = CLIPIT((luma + ipl_cbb[cb]));
        *(outputImgPtr) = r;
        *(outputImgPtr+1) = g;
        *(outputImgPtr+2) = b;
        outputImgPtr += output_row_size;
      }
    }
  }

  /* fill in last row since we skipped it earliear */
  if (input_img_ptr->dy % 2 && output_img_ptr->dy >= 3)
  {
   memcpy(output_img_ptr->imgPtr+(output_img_ptr->dx*(output_img_ptr->dy-1)*3), 
          output_img_ptr->imgPtr+(output_img_ptr->dx*(output_img_ptr->dy-2)*3), 
          output_img_ptr->dx*3);
  }
 
  MSG_LOW("ipl3_convert_ycbcr422lp_to_rgb888 marker_100\n");

  return IPL_SUCCESS;
} /* end of function ipl_convert_ycbcr422lp_to_rgb888 */



/* <EJECT> */
/*==========================================================================

FUNCTION    ipl3_convert_ycrcb422lp_to_rgb888

DESCRIPTION
  This function converts from YCrCb 4:2:2 line packed format to RGB888 

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
static ipl_status_type ipl_convert_ycrcb422lp_to_rgb888
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
)
{
  register unsigned char* inputImgPtr;
  register uint8* outputImgPtr;
  register uint8 cb, cr;
  register int32 luma;
  uint8   r,g,b;
  uint32 input_row_size;
  uint32 output_row_size;
  uint8 *cb_ptr;
  uint32 row, col;
  int32 pitchx;
  int oddWidth = 0;

  MSG_LOW("ipl_convert_ycrcb422lp_to_rgb888 marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycrcb422lp_to_rgb888 marker_200\n");
    return IPL_FAILURE;
  }

  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = output_img_ptr->imgPtr;

  input_row_size = input_img_ptr->dx;
  output_row_size = 3*output_img_ptr->dx;
  cb_ptr = input_img_ptr->clrPtr;

  /* output can smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
  {
    MSG_LOW("ipl_convert_ycrcb422lp_to_rgb888 marker_201\n");
    return IPL_FAILURE;
  }

  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  MSG_LOW("ipl_convert_ycrcb422lp_to_rgb888 marker_1\n");


  for (row = input_img_ptr->dy; row; row--)
  {
    for (col = (output_img_ptr->dx - oddWidth)/2; col; col--)
    {
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   
      --------------------------------------------------------------------*/
      cr = *cb_ptr++;
      cb = *cb_ptr++;

      luma = *inputImgPtr++;        /* corresponds to Y1 */
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));
      *outputImgPtr++   = r;
      *outputImgPtr++ = g;
      *outputImgPtr++ = b;

      luma = *inputImgPtr++;  /* corresponds to Y2 */
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));
      *outputImgPtr++ = r;
      *outputImgPtr++ = g;
      *outputImgPtr++ = b;
    } /* end col loop */


    /*----------------------------------------------------------------------
        Add on row increments
    ----------------------------------------------------------------------*/
    outputImgPtr += (3*oddWidth);
    inputImgPtr += (oddWidth + pitchx);
    cb_ptr += (pitchx + oddWidth);
  } 
 

  /* fill in last row since we skipped it earliear */
  if (oddWidth)
  {
    /* leave this outer for loop in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = output_img_ptr->imgPtr + (output_row_size - 3 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy; row; row--)
      {
        cr = *cb_ptr;
        cb = *cb_ptr+1;
        cb_ptr += (input_row_size);

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = CLIPIT((luma + ipl_crr[cr]));
        g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
        b = CLIPIT((luma + ipl_cbb[cb]));
        *(outputImgPtr) = r;
        *(outputImgPtr+1) = g;
        *(outputImgPtr+2) = b;
        outputImgPtr += output_row_size;
      }
    }
  }

  /* fill in last row since we skipped it earliear */
  if (input_img_ptr->dy % 2 && output_img_ptr->dy >= 3)
  {
   memcpy(output_img_ptr->imgPtr+(output_img_ptr->dx*(output_img_ptr->dy-1)*3), 
          output_img_ptr->imgPtr+(output_img_ptr->dx*(output_img_ptr->dy-2)*3), 
          output_img_ptr->dx*3);
  }
 
  MSG_LOW("ipl3_convert_ycrcb422lp_to_rgb888 marker_100\n");

  return IPL_SUCCESS;
} /* end of function ipl_convert_ycrcb422lp_to_rgb888 */





/* <EJECT> */
/*==========================================================================

FUNCTION    ipl3_convert_ycrcb420lp_to_rgb888

DESCRIPTION
  This function converts from YCrCb 4:2:0 line packed format to RGB888. 

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
static ipl_status_type ipl_convert_ycrcb420lp_to_rgb888
(
  ipl_image_type* input_img_ptr,       /* Pointer to the input image  */
  ipl_image_type* output_img_ptr       /* Pointer to the output image */
)
{
  register unsigned char* inputImgPtr;
  register uint8* outputImgPtr;
  register uint8 cb, cr;
  register int32 luma;
  uint8   r,g,b;
  uint32 input_row_size;
  uint32 output_row_size;
  uint8 *cb_ptr;
  uint32 row, col;
  int32 pitchx;
  int oddWidth = 0;

  MSG_LOW("ipl_convert_ycrcb420lp_to_rgb888 marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycrcb420lp_to_rgb888 marker_200\n");
    return IPL_FAILURE;
  }

  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = output_img_ptr->imgPtr;

  input_row_size = input_img_ptr->dx;
  output_row_size = 3*output_img_ptr->dx;
  cb_ptr = input_img_ptr->clrPtr;

  /* output can smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
  {
    MSG_LOW("ipl_convert_ycrcb420lp_to_rgb888 marker_201\n");
    return IPL_FAILURE;
  }

  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  MSG_LOW("ipl_convert_ycrcb420lp_to_rgb888 marker_1\n");


  for (row = input_img_ptr->dy/2; row; row--)
  {
    for (col = (output_img_ptr->dx - oddWidth)/2; col; col--)
    {
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/
      cr = *cb_ptr++;
      cb = *cb_ptr++;

      luma = *inputImgPtr;        /* corresponds to Y1 */
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));
      *(outputImgPtr)   = r;
      *(outputImgPtr+1) = g;
      *(outputImgPtr+2) = b;

      luma = *(inputImgPtr + 1);  /* corresponds to Y2 */
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));
      *(outputImgPtr+3) = r;
      *(outputImgPtr+4) = g;
      *(outputImgPtr+5) = b;

      luma = *(inputImgPtr++ + input_row_size);
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));

      *(outputImgPtr++ + output_row_size) = r;
      *(outputImgPtr++ + output_row_size) = g;
      *(outputImgPtr++ + output_row_size) = b;

      luma = *(inputImgPtr++ + input_row_size);
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));
      *(outputImgPtr++ + output_row_size) = r;
      *(outputImgPtr++ + output_row_size) = g;
      *(outputImgPtr++ + output_row_size) = b;
    } /* end col loop */


    /*----------------------------------------------------------------------
        Add on row increments
    ----------------------------------------------------------------------*/
    outputImgPtr += (3*oddWidth) + output_row_size;
    inputImgPtr += (input_row_size + oddWidth + pitchx);
    cb_ptr += (pitchx + oddWidth);
  } 
 

  /* fill in last column since we skipped it earliear */
  if (oddWidth)
  {
    /* leave this outer for loop in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = output_img_ptr->imgPtr + (output_row_size - 3 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy/2; row; row--)
      {
        cr = *cb_ptr;
        cb = *cb_ptr+1;
        cb_ptr += (input_row_size);

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = CLIPIT((luma + ipl_crr[cr]));
        g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
        b = CLIPIT((luma + ipl_cbb[cb]));
        *(outputImgPtr) = r;
        *(outputImgPtr+1) = g;
        *(outputImgPtr+2) = b;
        outputImgPtr += output_row_size;

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = CLIPIT((luma + ipl_crr[cr]));
        g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
        b = CLIPIT((luma + ipl_cbb[cb]));
        *(outputImgPtr) = r;
        *(outputImgPtr+1) = g;
        *(outputImgPtr+2) = b;
        outputImgPtr += output_row_size;
      }
    }
  }


 
 
  MSG_LOW("ipl3_convert_ycrcb420lp_to_rgb888 marker_100\n");

  return IPL_SUCCESS;
} /* end of function ipl_convert_ycrcb420lp_to_rgb888 */



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
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr;
  register uint8 cb, cr;
  register int32 luma;
  register int32 r,g,b;
  uint32 input_row_size;
  uint32 output_row_size;
  uint8 *cb_ptr;
  uint32 row, col;
  int32 pitchx;
  boolean oddWidth = 0;
  int32 crr, crg, cbg, cbb;

  MSG_LOW("ipl3_convert_ycrcb420lp_to_rgb565 marker_0\n");




  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl3_convert_ycrcb420lp_to_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;

  input_row_size = input_img_ptr->dx;
  output_row_size = output_img_ptr->dx;
  cb_ptr = input_img_ptr->clrPtr;

  /* output has to be smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
  {
    MSG_LOW("ipl3_convert_ycrcb420lp_to_rgb565 marker_201\n");
    return IPL_FAILURE;
  }

  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  MSG_LOW("ipl3_convert_ycrcb420lp_to_rgb565 marker_1\n");

  for (row = input_img_ptr->dy/2; row; row--)
  {
    for (col = (output_row_size - oddWidth)/2; col; col--)
    {
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/

      cr = *cb_ptr++;
      cb = *cb_ptr++;

      crr = ipl_crr[cr];
      crg = ipl_crg[cr];
      cbg = ipl_cbg[cb];
      cbb = ipl_cbb[cb];

      luma = *inputImgPtr;        
      r = CLIPIT((luma + crr));
      g = CLIPIT((luma + ((crg + cbg) >> 16)));
      b = CLIPIT((luma + cbb));
      *(outputImgPtr) = pack_rgb565(r,g,b);

      luma = *(inputImgPtr + 1);  
      r = CLIPIT((luma + crr));
      g = CLIPIT((luma + ((crg + cbg) >> 16)));
      b = CLIPIT((luma + cbb));
      *(outputImgPtr + 1) = pack_rgb565(r,g,b);

      luma = *(inputImgPtr++ + input_row_size);
      r = CLIPIT((luma + crr));
      g = CLIPIT((luma + ((crg + cbg) >> 16)));
      b = CLIPIT((luma + cbb));
      *(outputImgPtr++ + output_row_size) = pack_rgb565(r,g,b);

      luma = *(inputImgPtr++ + input_row_size);
      r = CLIPIT((luma + crr));
      g = CLIPIT((luma + ((crg + cbg) >> 16)));
      b = CLIPIT((luma + cbb));
      *(outputImgPtr++ + output_row_size) = pack_rgb565(r,g,b);

    } /* end col loop */


    /*----------------------------------------------------------------------
        Add on row increments
    ----------------------------------------------------------------------*/
    outputImgPtr = (uint16*) ((uint8*) outputImgPtr + 
      ((oddWidth + output_row_size) << 1));
    inputImgPtr += (input_row_size + oddWidth + pitchx);
    cb_ptr += (pitchx + oddWidth);
  } /* end row loop */
  
  /* fill in last row since we skipped it earliear */
  if (oddWidth)
  {
    /* leave this outer for loop in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = (uint16*) output_img_ptr->imgPtr + 
        (output_row_size - 1 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy/2; row; row--)
      {
        cr = *cb_ptr;
        cb = *cb_ptr+1;
        cb_ptr += (input_row_size);

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = CLIPIT((luma + ipl_crr[cr]));
        g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
        b = CLIPIT((luma + ipl_cbb[cb]));
        *(outputImgPtr) = pack_rgb565(r,g,b);
        outputImgPtr += output_row_size;

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = CLIPIT((luma + ipl_crr[cr]));
        g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
        b = CLIPIT((luma + ipl_cbb[cb]));
        *(outputImgPtr) = pack_rgb565(r,g,b);
        outputImgPtr += output_row_size;
      }
    }
  }

  /* fill in last row since we skipped it earliear */
  if (input_img_ptr->dy % 2 && output_img_ptr->dy >= 3)
  {
   memcpy(output_img_ptr->imgPtr+(output_img_ptr->dx*(output_img_ptr->dy-1)*2), 
          output_img_ptr->imgPtr+(output_img_ptr->dx*(output_img_ptr->dy-2)*2), 
          output_img_ptr->dx*2);
  }
 
  MSG_LOW("ipl3_convert_ycrcb420lp_to_rgb565 marker_100\n");

  return IPL_SUCCESS;
} /* end of function ipl_convert_ycrcb420lp_to_rgb565 */



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
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr;
  register uint8 cb, cr;
  register int32 luma;
  register int32 r,g,b;
  register uint8 *cb_ptr;
  uint32 output_row_size;
  uint32 input_row_size;
  uint32 row, col;
  int32 pitchx;
  boolean oddWidth = 0;

  MSG_LOW("ipl3_convert_ycrcb422lp_to_rgb565 marker_0\n");



  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl3_convert_ycrcb422lp_to_rgb565 marker_200\n");
    return IPL_FAILURE;
  }
  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  cb_ptr = input_img_ptr->clrPtr;
  output_row_size = output_img_ptr->dx;
  input_row_size = input_img_ptr->dx;

  /* output has to be smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
  {
    MSG_LOW("ipl3_convert_ycrcb422lp_to_rgb565 marker_201\n");
    return IPL_FAILURE;
  }
  
  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  MSG_LOW("ipl3_convert_ycrcb422lp_to_rgb565 marker_1\n");

  for (row = input_img_ptr->dy; row; row--)
  {
    for (col = (output_row_size - oddWidth)/2; col; col--)
    {
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/

      /*--------------------------------------------------------------------
          Get Cr and Cb for all 4 Y values
      --------------------------------------------------------------------*/
      cr = *cb_ptr++;
      cb = *cb_ptr++;

      luma = *inputImgPtr;        /* corresponds to Y1 */
      inputImgPtr++;
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));
      *outputImgPtr++ = pack_rgb565(r,g,b);


      luma = *inputImgPtr;        /* corresponds to Y1 */
      inputImgPtr++;
      r = CLIPIT((luma + ipl_crr[cr]));
      g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
      b = CLIPIT((luma + ipl_cbb[cb]));
      *outputImgPtr++ = pack_rgb565(r,g,b);
    } 

    outputImgPtr += oddWidth;
    inputImgPtr += (pitchx + oddWidth);
    cb_ptr += (pitchx + oddWidth);

  } /* end row loop */

  /* fill in last row since we skipped it earliear */
  if (oddWidth)
  {
     /* leave this in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = (uint16*) output_img_ptr->imgPtr + 
        (output_row_size - 1 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy; row; row--)
      {
        cr = *cb_ptr;
        cb = *cb_ptr+1;
        cb_ptr += input_row_size;

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = CLIPIT((luma + ipl_crr[cr]));
        g = CLIPIT((luma + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
        b = CLIPIT((luma + ipl_cbb[cb]));
        *outputImgPtr = pack_rgb565(r,g,b);
        outputImgPtr += output_row_size;
      }
    }
  }

  MSG_LOW("ipl3_convert_ycrcb422lp_to_rgb565 marker_100\n");

  return IPL_SUCCESS;
} /* end of function ipl3_convert_ycrcb422lp_to_rgb565 */




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
)
{
  register unsigned char* inputImgPtr;
  register uint16* outputImgPtr;
  register uint8 cb, cr;
  register int32 luma;
  register int32 r,g,b;
  register uint8 *cb_ptr;
  int32 rc, gc, bc; 
  uint32 output_row_size;
  uint32 input_row_size;
  uint32 row, col;
  int32 pitchx;
  boolean oddWidth = 0;
  short ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};

  MSG_LOW("ipl_convert_ycrcb422lp_to_rgb565 marker_0\n");



  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycrcb422lp_to_rgb565 marker_200\n");
    return IPL_FAILURE;
  }
  inputImgPtr = input_img_ptr->imgPtr;
  outputImgPtr = (uint16*)output_img_ptr->imgPtr;
  cb_ptr = input_img_ptr->clrPtr;
  output_row_size = output_img_ptr->dx;
  input_row_size = input_img_ptr->dx;

  /* output has to be smaller than input */
  pitchx = (input_img_ptr->dx - output_img_ptr->dx);
  if (pitchx < 0)
  {
    MSG_LOW("ipl_convert_ycrcb422lp_to_rgb565 marker_201\n");
    return IPL_FAILURE;
  }
  
  /* if use wants us to output odd width file, then leave it blank for now */
  /* and we will go back and fill in that column later */
  if (output_img_ptr->dx%2)
    oddWidth = 1;

  MSG_LOW("ipl_convert_ycrcb422lp_to_rgb565 marker_1\n");

  for (row = input_img_ptr->dy; row; row--)
  {
    for (col = (output_row_size - oddWidth)/2; col; col--)
    {
      /*--------------------------------------------------------------------
          Process 4 pixels at a time in a 2x2 block.

          Take luma of all four pixels. The luma packing is as shown
          below.

          |Y1|Y2|             (color conversion)      |RGB1|RGB2|
                --- Cb, Cr ----------------------->   |RGB3|RGB4|
          |Y3|Y4|
      --------------------------------------------------------------------*/

      /*--------------------------------------------------------------------
          Get Cr and Cb for all 4 Y values
      --------------------------------------------------------------------*/
      cr = *cb_ptr++;
      cb = *cb_ptr++;

      rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
      gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
      bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;

      luma = *inputImgPtr;        /* corresponds to Y1 */
      r = luma + (rc>>16);
      g = luma + (gc>>16);
      b = luma + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *outputImgPtr++ = pack_rgb565(r,g,b);


      luma = *(inputImgPtr + 1);  /* corresponds to Y2 */
      inputImgPtr++;
      inputImgPtr++;
      r = luma + (rc>>16);
      g = luma + (gc>>16);
      b = luma + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *outputImgPtr++ = pack_rgb565(r,g,b);
    } 

    outputImgPtr += oddWidth;
    inputImgPtr += (pitchx + oddWidth);
    cb_ptr += (pitchx + oddWidth);

  } /* end row loop */

  /* fill in last row since we skipped it earliear */
  if (oddWidth)
  {
     /* leave this in for debugging purposes */
    for (col = 0; col < 1; col++) 
    {
      outputImgPtr = (uint16*) output_img_ptr->imgPtr + 
        (output_row_size - 1 + col);
      inputImgPtr = input_img_ptr->imgPtr + input_row_size - pitchx - 1 + col;
      cb_ptr = input_img_ptr->clrPtr + input_row_size - pitchx - 1 + 2*(col/2);

      for (row = input_img_ptr->dy; row; row--)
      {

        cr = *cb_ptr;
        cb = *cb_ptr+1;
        cb_ptr += input_row_size;

        rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
        gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
        bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;

        luma = *inputImgPtr;        
        inputImgPtr += input_row_size;
        r = luma + (rc>>16);
        g = luma + (gc>>16);
        b = luma + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        *outputImgPtr = pack_rgb565(r,g,b);
        outputImgPtr += output_row_size;
      }
    }
  }

  MSG_LOW("ipl_convert_ycrcb422lp_to_rgb565 marker_100\n");

  return IPL_SUCCESS;
} /* end of function ipl_convert_ycrcb422lp_to_rgb565 */



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
)
{
  register uint8 cb, cr, cb2, cr2;
  uint32 x, y;
  register uint8* data1_in;
  register uint8* data2_in;
  uint8* data1_out;
  uint8* data2_out;
  uint8* c_ptr;
  int32 dest_index;
  uint32 clr;
  int oddHeight;

  MSG_LOW("ipl_convert_ycbcr422_to_ycrcb420lp marker_0\n");



  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycrcb420lp marker_200\n");
    return IPL_FAILURE;
  }

  data1_in = input_img_ptr->imgPtr;
  data2_in = (uint8*)((uint32)input_img_ptr->imgPtr + input_img_ptr->dx*2);
  data1_out = output_img_ptr->imgPtr;
  data2_out = output_img_ptr->imgPtr + output_img_ptr->dx;
  c_ptr = output_img_ptr->clrPtr;
  dest_index = output_img_ptr->dx;

  if (input_img_ptr->dx % 2)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycrcb420lp marker_201\n");
    return IPL_FAILURE;
  }
  oddHeight = (input_img_ptr->dy % 2)? 1 : 0; 
  if (oddHeight)
  {
    if ((input_img_ptr->dy + 1) != (output_img_ptr->dy)) 
    {
      MSG_LOW("ipl_convert_ycbcr422_to_ycrcb420lp marker_202\n");
      return IPL_FAILURE;
    }
  }


  /*
  **  Verify Arguments
  */
  if (input_img_ptr->cFormat != IPL_YCbCr)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycrcb420lp marker_203\n");
    return IPL_FAILURE;
  }
  if (output_img_ptr->cFormat != IPL_YCrCb420_LINE_PK)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycrcb420lp marker_204\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycrcb420lp marker_205\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_ycbcr422_to_ycrcb420lp marker_1\n");

  /* Now loop through the image once */
  for(y = input_img_ptr->dy - oddHeight; y; y-=2)
  {
    for(x = input_img_ptr->dx; x; x-=2)
    {
      /*
      ** Work on 4 pixels at a time
      */
      /*
      ** First 2 pixels
      */
      cb = *data1_in++;
      *data1_out++ = *data1_in++;
      cr = *data1_in++;
      *data1_out++ = *data1_in++;
      /*
      ** Next 2 pixels
      */
      cb2 = *data2_in++;
      *data2_out++ = *data2_in++;
      cr2 = *data2_in++;
      *data2_out++ = *data2_in++;

      /*
      ** Now calculate 1 cr and 1 cb
      */
      clr = cr + cr2;
      clr >>= 1;
      *c_ptr++ = (uint8)clr;
      clr = cb + cb2;
      clr >>= 1;
      *c_ptr++ = (uint8)clr;
    } /* end x loop */


    /* 
    ** Increment pointers
    */
    data1_in += input_img_ptr->dx*2;
    data2_in += input_img_ptr->dx*2;
    data1_out += dest_index;
    data2_out += dest_index;
  } /* end y loop */

  // if odd height, duplicate the last row
  if (oddHeight)
  {
    for(x = output_img_ptr->dx; x; x-=2)
    {
      *c_ptr++ = *data1_in++;                      // Cb
      *data2_out++ = *data1_out++ = *data1_in++;   // Luma 
      *c_ptr++ = *data1_in++;                      // Cr
      *data2_out++ = *data1_out++ = *data1_in++;   // Luma
    } 
  } 

  MSG_LOW("ipl_convert_ycbcr422_to_ycrcb420lp marker_100\n");

  return IPL_SUCCESS;
} /* ipl_convert_ycbcr422_to_ycrcb420lp */


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
)
{
  uint16 *inImgPtr, *in2ImgPtr;
  unsigned char *outImgPtr, *out2ImgPtr, *outClrPtr;
  uint32 x, y, w, h;
  unsigned char r1, g1, b1, r2, g2, b2;
  uint16 out1, out2;
  int32 luma1, luma2, cb, cr;
  /*
  ** RGB to YCbCr conversion array in Q14 signed. In Q0, the coefficients
  ** are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071.
  */
  short rgb2ycbcr[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_convert_rgb444_to_ycbcr420lp marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb444_to_ycbcr420lp marker_200\n");
    return IPL_FAILURE;
  }
  
  if (input_img_ptr->cFormat != IPL_RGB444)
  {
    MSG_LOW("ipl_convert_rgb444_to_ycbcr420lp marker_201\n");
    return IPL_FAILURE;
  }
  
  if (output_img_ptr->cFormat != IPL_YCbCr420_LINE_PK && 
      output_img_ptr->cFormat != IPL_YCrCb420_LINE_PK)
  {
    MSG_LOW("ipl_convert_rgb444_to_ycbcr420lp marker_202\n");
    return IPL_FAILURE;
  }
  
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_rgb444_to_ycbcr420lp marker_203\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = (uint16*)input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  outClrPtr = output_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;
  in2ImgPtr = inImgPtr + w;
  out2ImgPtr = outImgPtr + w;

  MSG_LOW("ipl_convert_rgb444_to_ycbcr420lp marker_1\n");

  for (y = 0; y < h; y+=2) {
    for (x = 0; x < w; x+=2) {
      /* Process 4 pixels at a time */
      /* Unpack RGB value of pixel 1 */
      out1 = *inImgPtr++;
      unpack_rgb444(out1, &r1, &g1, &b1);
      luma1 = (rgb2ycbcr[0]*r1 + rgb2ycbcr[1]*g1 + rgb2ycbcr[2]*b1)*4 + 0x8000;
      luma1 = (luma1>>16) + 16;
      if (luma1 < 0) luma1 = 0;
      cb = (rgb2ycbcr[3]*r1 + rgb2ycbcr[4]*g1 + rgb2ycbcr[5]*b1)*4 + 0x8000;
      cb = (cb>>16) + 128;
      if (cb < 0) cb = 0;
      /* Unpack RGB value of pixel 2 */
      out2 = *inImgPtr++;
      unpack_rgb444(out2, &r2, &g2, &b2);
      luma2 = (rgb2ycbcr[0]*r2 + rgb2ycbcr[1]*g2 + rgb2ycbcr[2]*b2)*4 + 0x8000;
      luma2 = (luma2>>16) + 16;
      if (luma2 < 0) luma2 = 0;
      cr = (rgb2ycbcr[6]*r2 + rgb2ycbcr[7]*g2 + rgb2ycbcr[8]*b2)*4 + 0x8000;
      cr = (cr>>16) + 128;
      if (cr < 0) cr = 0;
      /* Output chroma */
      *outClrPtr++ = (unsigned char)cb;
      *outClrPtr++ = (unsigned char)cr;
      /* Output pixels 1 and 2 */
      *outImgPtr++ = (unsigned char)luma1;
      *outImgPtr++ = (unsigned char)luma2;
      /* Unpack RGB value of pixel 3 */
      out1 = *in2ImgPtr++;
      unpack_rgb444(out1, &r1, &g1, &b1);
      luma1 = (rgb2ycbcr[0]*r1 + rgb2ycbcr[1]*g1 + rgb2ycbcr[2]*b1)*4 + 0x8000;
      luma1 = (luma1>>16) + 16;
      if (luma1 < 0) luma1 = 0;
      /* Unpack RGB value of pixel 4 */
      out2 = *in2ImgPtr++;
      unpack_rgb444(out2, &r2, &g2, &b2);
      luma2 = (rgb2ycbcr[0]*r2 + rgb2ycbcr[1]*g2 + rgb2ycbcr[2]*b2)*4 + 0x8000;
      luma2 = (luma2>>16) + 16;
      if (luma2 < 0) luma2 = 0;
      /* Output pixels 1 and 2 */
      *out2ImgPtr++ = (unsigned char)luma1;
      *out2ImgPtr++ = (unsigned char)luma2;
    } /* end x loop */
    inImgPtr += w;
    in2ImgPtr += w;
    outImgPtr += w;
    out2ImgPtr += w;
  } /* end y loop */

  MSG_LOW("ipl_convert_rgb444_to_ycbcr420lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_rgb444_to_ycbcr420lp */


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
)
{
  unsigned char *inImgPtr, *in2ImgPtr, *inClrPtr;
  uint16 *outImgPtr, *out2ImgPtr;
  uint32 x, y, w, h;
  unsigned char cb, cr, luma1, luma2;
  int32 rc, gc, bc, r, g, b;
  /* 
  ** YCbCr to RGB conversion array in Q14 Signed. In Q0, the coefficients are
  ** 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 
  */
  int16 ycbcr2rgb[6] = {8, 25803, -3071, -7672, 30399, 12};

  MSG_LOW("ipl_convert_ycbcr420lp_to_rgb444 marker_0\n");



  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_rgb444 marker_200\n");
    return IPL_FAILURE;
  }

  if ((input_img_ptr->cFormat != IPL_YCbCr420_LINE_PK && 
       input_img_ptr->cFormat != IPL_YCrCb420_LINE_PK) ||
      output_img_ptr->cFormat != IPL_RGB444) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_rgb444 marker_201\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_rgb444 marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = (uint16*)output_img_ptr->imgPtr;
  inClrPtr = input_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;
  in2ImgPtr = inImgPtr + w;
  out2ImgPtr = outImgPtr + w;

  MSG_LOW("ipl_convert_ycbcr420lp_to_rgb444 marker_1\n");

  for (y = 0; y < h; y+=2) {
    for (x = 0; x < w; x+=2) {
      /* Process 4 pixels at a time */
      cb = *inClrPtr++;
      cr = *inClrPtr++;
      rc = (ycbcr2rgb[0]*(cb-128) + ycbcr2rgb[1]*(cr-128))*4 + 0x8000;
      gc = (ycbcr2rgb[2]*(cb-128) + ycbcr2rgb[3]*(cr-128))*4 + 0x8000;
      bc = (ycbcr2rgb[4]*(cb-128) + ycbcr2rgb[5]*(cr-128))*4 + 0x8000;
      /* Pixel 1 */
      luma1 = *inImgPtr++;
      r = luma1 + (rc>>16);
      g = luma1 + (gc>>16);
      b = luma1 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *outImgPtr++ = pack_rgb444(r, g, b);
      /* Pixel 2 */
      luma2 = *inImgPtr++;
      r = luma2 + (rc>>16);
      g = luma2 + (gc>>16);
      b = luma2 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *outImgPtr++ = pack_rgb444(r, g, b);
      /* Pixel 3 */
      luma1 = *in2ImgPtr++;
      r = luma1 + (rc>>16);
      g = luma1 + (gc>>16);
      b = luma1 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *out2ImgPtr++ = pack_rgb444(r, g, b);
      /* Pixel 2 */
      luma2 = *in2ImgPtr++;
      r = luma2 + (rc>>16);
      g = luma2 + (gc>>16);
      b = luma2 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *out2ImgPtr++ = pack_rgb444(r, g, b);
    } /* end x loop */
    inImgPtr += w;
    in2ImgPtr += w;
    outImgPtr += w;
    out2ImgPtr += w;
  } /* end y loop */

  MSG_LOW("ipl_convert_ycbcr420lp_to_rgb444 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr420lp_to_rgb444 */


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
)
{
  uint32 *inImgPtr, *in2ImgPtr;
  unsigned char *outImgPtr, *out2ImgPtr, *outClrPtr;
  uint32 x, y, w, h;
  unsigned char r1, g1, b1, r2, g2, b2;
  uint32 out1, out2;
  int32 luma1, luma2, cb, cr;
  /*
  ** RGB to YCbCr conversion array in Q14 signed. In Q0, the coefficients
  ** are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071.
  */
  short rgb2ycbcr[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_convert_rgb666_to_ycbcr420lp marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb666_to_ycbcr420lp marker_200\n");
    return IPL_FAILURE;
  }
  if ((output_img_ptr->cFormat != IPL_YCbCr420_LINE_PK && 
       output_img_ptr->cFormat != IPL_YCrCb420_LINE_PK) ||
       input_img_ptr->cFormat != IPL_RGB666) 
  {
    MSG_LOW("ipl_convert_rgb666_to_ycbcr420lp marker_201\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_rgb666_to_ycbcr420lp marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = (uint32*)input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  outClrPtr = output_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;
  in2ImgPtr = inImgPtr + w;
  out2ImgPtr = outImgPtr + w;

  MSG_LOW("ipl_convert_rgb666_to_ycbcr420lp marker_1\n");

  for (y = 0; y < h; y+=2) {
    for (x = 0; x < w; x+=2) {
      /* Process 4 pixels at a time */
      /* Unpack RGB value of pixel 1 */
      out1 = *inImgPtr++;
      unpack_rgb666(out1, &r1, &g1, &b1);
      luma1 = (rgb2ycbcr[0]*r1 + rgb2ycbcr[1]*g1 + rgb2ycbcr[2]*b1)*4 + 0x8000;
      luma1 = (luma1>>16) + 16;
      if (luma1 < 0) luma1 = 0;
      cb = (rgb2ycbcr[3]*r1 + rgb2ycbcr[4]*g1 + rgb2ycbcr[5]*b1)*4 + 0x8000;
      cb = (cb>>16) + 128;
      if (cb < 0) cb = 0;
      /* Unpack RGB value of pixel 2 */
      out2 = *inImgPtr++;
      unpack_rgb666(out2, &r2, &g2, &b2);
      luma2 = (rgb2ycbcr[0]*r2 + rgb2ycbcr[1]*g2 + rgb2ycbcr[2]*b2)*4 + 0x8000;
      luma2 = (luma2>>16) + 16;
      if (luma2 < 0) luma2 = 0;
      cr = (rgb2ycbcr[6]*r2 + rgb2ycbcr[7]*g2 + rgb2ycbcr[8]*b2)*4 + 0x8000;
      cr = (cr>>16) + 128;
      if (cr < 0) cr = 0;
      /* Output chroma */
      *outClrPtr++ = (unsigned char)cb;
      *outClrPtr++ = (unsigned char)cr;
      /* Output pixels 1 and 2 */
      *outImgPtr++ = (unsigned char)luma1;
      *outImgPtr++ = (unsigned char)luma2;
      /* Unpack RGB value of pixel 3 */
      out1 = *in2ImgPtr++;
      unpack_rgb666(out1, &r1, &g1, &b1);
      luma1 = (rgb2ycbcr[0]*r1 + rgb2ycbcr[1]*g1 + rgb2ycbcr[2]*b1)*4 + 0x8000;
      luma1 = (luma1>>16) + 16;
      if (luma1 < 0) luma1 = 0;
      /* Unpack RGB value of pixel 4 */
      out2 = *in2ImgPtr++;
      unpack_rgb666(out2, &r2, &g2, &b2);
      luma2 = (rgb2ycbcr[0]*r2 + rgb2ycbcr[1]*g2 + rgb2ycbcr[2]*b2)*4 + 0x8000;
      luma2 = (luma2>>16) + 16;
      if (luma2 < 0) luma2 = 0;
      /* Output pixels 1 and 2 */
      *out2ImgPtr++ = (unsigned char)luma1;
      *out2ImgPtr++ = (unsigned char)luma2;
    } /* end x loop */
    inImgPtr += w;
    in2ImgPtr += w;
    outImgPtr += w;
    out2ImgPtr += w;
  } /* end y loop */

  MSG_LOW("ipl_convert_rgb666_to_ycbcr420lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_rgb666_to_ycbcr420lp */


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
)
{
  unsigned char *inImgPtr, *in2ImgPtr, *inClrPtr;
  uint32 *outImgPtr, *out2ImgPtr;
  uint32 x, y, w, h;
  unsigned char cb, cr, luma1, luma2;
  int32 rc, gc, bc, r, g, b;
  /* 
  ** YCbCr to RGB conversion array in Q14 Signed. In Q0, the coefficients are
  ** 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 
  */
  int16 ycbcr2rgb[6] = {8, 25803, -3071, -7672, 30399, 12};

  MSG_LOW("ipl_convert_ycbcr420lp_to_rgb666 marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_rgb666 marker_200\n");
    return IPL_FAILURE;
  }
  
  if ((input_img_ptr->cFormat != IPL_YCrCb420_LINE_PK && 
       input_img_ptr->cFormat != IPL_YCbCr420_LINE_PK) ||
      output_img_ptr->cFormat != IPL_RGB666) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_rgb666 marker_201\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_rgb666 marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = (uint32*)output_img_ptr->imgPtr;
  inClrPtr = input_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;
  in2ImgPtr = inImgPtr + w;
  out2ImgPtr = outImgPtr + w;

  MSG_LOW("ipl_convert_ycbcr420lp_to_rgb666 marker_1\n");
  for (y = 0; y < h; y+=2) {
    for (x = 0; x < w; x+=2) {
      /* Process 4 pixels at a time */
      cb = *inClrPtr++;
      cr = *inClrPtr++;
      rc = (ycbcr2rgb[0]*(cb-128) + ycbcr2rgb[1]*(cr-128))*4 + 0x8000;
      gc = (ycbcr2rgb[2]*(cb-128) + ycbcr2rgb[3]*(cr-128))*4 + 0x8000;
      bc = (ycbcr2rgb[4]*(cb-128) + ycbcr2rgb[5]*(cr-128))*4 + 0x8000;
      /* Pixel 1 */
      luma1 = *inImgPtr++;
      r = luma1 + (rc>>16);
      g = luma1 + (gc>>16);
      b = luma1 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *outImgPtr++ = pack_rgb666(r, g, b);
      /* Pixel 2 */
      luma2 = *inImgPtr++;
      r = luma2 + (rc>>16);
      g = luma2 + (gc>>16);
      b = luma2 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *outImgPtr++ = pack_rgb666(r, g, b);
      /* Pixel 3 */
      luma1 = *in2ImgPtr++;
      r = luma1 + (rc>>16);
      g = luma1 + (gc>>16);
      b = luma1 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *out2ImgPtr++ = pack_rgb666(r, g, b);
      /* Pixel 2 */
      luma2 = *in2ImgPtr++;
      r = luma2 + (rc>>16);
      g = luma2 + (gc>>16);
      b = luma2 + (bc>>16);
      r = CLIPIT(r);
      g = CLIPIT(g);
      b = CLIPIT(b);
      *out2ImgPtr++ = pack_rgb666(r, g, b);
    } /* end x loop */
    inImgPtr += w;
    in2ImgPtr += w;
    outImgPtr += w;
    out2ImgPtr += w;
  } /* end y loop */

  MSG_LOW("ipl_convert_ycbcr420lp_to_rgb666 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr420lp_to_rgb666 */


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
)
{
  unsigned char *inImgPtr, *outImgPtr, *inClrPtr, *outClrPtr, *out2ClrPtr;
  uint32 x, y, w, h;

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422lp marker_0\n");



  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422lp marker_200\n");
    return IPL_FAILURE;
  }
  
  if (input_img_ptr->cFormat != IPL_YCrCb420_LINE_PK &&
      input_img_ptr->cFormat != IPL_YCbCr420_LINE_PK) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422lp marker_201\n");
    return IPL_FAILURE;
  }


  if (output_img_ptr->cFormat != IPL_YCrCb422_LINE_PK &&
      output_img_ptr->cFormat != IPL_YCbCr422_LINE_PK) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422lp marker_202\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422lp marker_203\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  inClrPtr = input_img_ptr->clrPtr;
  outClrPtr = output_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;
  out2ClrPtr = outClrPtr + w;

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422lp marker_1\n");

  /* Y values */
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      *outImgPtr++ = *inImgPtr++;
    } /* end x loop */
  } /* end y loop */

  /* CbCr values */
  for (y = 0; y < h; y+=2) {
    for (x = 0; x < w; x+=2) {
      /* 4 pixels at a time */
      *outClrPtr++ = *inClrPtr;
      *out2ClrPtr++ = *inClrPtr++;
      *outClrPtr++ = *inClrPtr;
      *out2ClrPtr++ = *inClrPtr++;
    } /* end x loop */
    outClrPtr += w;
    out2ClrPtr += w;
  } /* end y loop */

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr420lp_to_ycbcr422lp */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr420lp_to_ycbcr420fp

DESCRIPTION
  This function converts from YCbCr 4:2:0 line packed to YCbCr 4:2:0 line 
  packed format.  Input and output image sizes must be equal.

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
static ipl_status_type ipl_convert_ycbcr420lp_to_ycbcr420fp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
)
{
  register unsigned char *inClrPtr; 
  register unsigned char *outCbPtr; 
  register unsigned char *outCrPtr; 

  uint32 x, w, h;
  boolean swap;

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr420fp marker_0\n");



  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr420fp marker_200\n");
    return IPL_FAILURE;
  }

  if ((input_img_ptr->cFormat != IPL_YCbCr420_LINE_PK &&
       input_img_ptr->cFormat != IPL_YCrCb420_LINE_PK) || 
      (output_img_ptr->cFormat != IPL_YCbCr420_FRAME_PK &&
       output_img_ptr->cFormat != IPL_YCrCb420_FRAME_PK))
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr420fp marker_201\n");
    return IPL_FAILURE;
  }



  swap = 0;
  if (output_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK &&
      input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
    swap = 1;
  else if (output_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK &&
           input_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) 
    swap = 1;

  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr420fp marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;

  inClrPtr = input_img_ptr->clrPtr;
  outCbPtr = output_img_ptr->clrPtr;
  outCrPtr = outCbPtr + w*h/4;

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr420fp marker_1\n");

  /* Y values */
  // only copy luma if imgPtrs are different
  if (output_img_ptr->imgPtr != input_img_ptr->imgPtr)
    memcpy(output_img_ptr->imgPtr, input_img_ptr->imgPtr, w*h);

  /* CbCr values */
  if (swap)
  {
    for (x = w*h/4; x; x--) 
    {
      *outCrPtr++ = *inClrPtr++;
      *outCbPtr++ = *inClrPtr++;
    } 
  }
  else
  {
    for (x = w*h/4; x; x--) 
    {
      *outCbPtr++ = *inClrPtr++;
      *outCrPtr++ = *inClrPtr++;
    } 
  }
  
  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr420fp marker_100\n");

  return IPL_SUCCESS;
} 



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
)
{
  unsigned char *inImgPtr, *inCbPtr, *inCrPtr; 
  unsigned short *outImgPtr;
  register int32 r;
  register uint32 out;
  register uint8 luma;
  uint8 cb, cr;
  uint32 x, y, w;
  register const uint16 *rTable;
  register const uint16 *gTable;
  register const uint16 *bTable;

  MSG_LOW("ipl_convert_ycbcr420fp_to_rgb565 marker_0\n");

  // make sure we have valid input
  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr420fp_to_rgb565 marker_200\n");
    return IPL_FAILURE;
  }

  // make sure out input and output image types are okay
  if ((input_img_ptr->cFormat != IPL_YCbCr420_FRAME_PK &&
       input_img_ptr->cFormat != IPL_YCrCb420_FRAME_PK) || 
       output_img_ptr->cFormat != IPL_RGB565)
  {
    MSG_LOW("ipl_convert_ycbcr420fp_to_rgb565 marker_201\n");
    return IPL_FAILURE;
  }


  // make sure we have smae size input and output
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr420fp_to_rgb565 marker_202\n");
    return IPL_FAILURE;
  }

 /*------------------------------------------------------------------------
  *       Call the init routine. Mulitiple initializations does not hurt.
  *------------------------------------------------------------------------*/

  if (ipl2_init() != IPL_SUCCESS)
  {
    MSG_LOW("ipl_convert_ycbcr420fp_to_rgb565 marker_203\n");
    return IPL_FAILURE;
  }
  
  // Initialize RGB565 conversion table
  rTable = ipl2_r5xx;
  gTable = ipl2_gx6x;
  bTable = ipl2_bxx5;

  w = output_img_ptr->dx;

  inImgPtr = input_img_ptr->imgPtr;
  inCbPtr = input_img_ptr->clrPtr;
  inCrPtr = inCbPtr + (w * output_img_ptr->dy)/4;

  outImgPtr = (unsigned short *) output_img_ptr->imgPtr;

  MSG_LOW("ipl_convert_ycbcr420fp_to_rgb565 marker_1\n");

  if (input_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK)
  {
    /* do 4 at a time */
    for (y = 0; y < output_img_ptr->dy/2; y++) 
    {
      for (x = 0; x < w/2; x++) 
      {
        cb = *inCbPtr++;
        cr = *inCrPtr++;
     
        // ul Y
        luma = *inImgPtr;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma,cr,cb,r,out,rTable,gTable,bTable);
        *outImgPtr = (unsigned short) out;

        // ll Y
        luma = *(inImgPtr + w);
        inImgPtr++;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma,cr,cb,r,out,rTable,gTable,bTable);
        *(outImgPtr + w) = (unsigned short) out;
        outImgPtr++;
     
        // ur Y
        luma = *inImgPtr;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma,cr,cb,r,out,rTable,gTable,bTable);
        *outImgPtr = (unsigned short) out;
      
        // lr Y
        luma = *(inImgPtr + w);
        inImgPtr++;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma,cr,cb,r,out,rTable,gTable,bTable);
        *(outImgPtr + w) = (unsigned short) out;
        outImgPtr++;
      }

      inImgPtr += w;
      outImgPtr += w;
    }
  }
  else if (input_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK)
  {
    /* do 4 at a time */
    for (y = 0; y < output_img_ptr->dy/2; y++) 
    {
      for (x = 0; x < w/2; x++) 
      {
        cr = *inCbPtr++;
        cb = *inCrPtr++;
     
        // ul Y
        luma = *inImgPtr;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma,cr,cb,r,out,rTable,gTable,bTable);
        *outImgPtr = (unsigned short) out;

        // ll Y
        luma = *(inImgPtr + w);
        inImgPtr++;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma,cr,cb,r,out,rTable,gTable,bTable);
        *(outImgPtr + w) = (unsigned short) out;
        outImgPtr++;
     
        // ur Y
        luma = *inImgPtr;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma,cr,cb,r,out,rTable,gTable,bTable);
        *outImgPtr = (unsigned short) out;
      
        // lr Y
        luma = *(inImgPtr + w);
        inImgPtr++;
        IPL2_CONVERT_YCBCR_RGB_SINGLE(luma,cr,cb,r,out,rTable,gTable,bTable);
        *(outImgPtr + w) = (unsigned short) out;
        outImgPtr++;
      }

      inImgPtr += w;
      outImgPtr += w;
    }
  }
  else 
  {
    return IPL_FAILURE;
  }


  MSG_LOW("ipl_convert_ycbcr420fp_to_rgb565 marker_100\n");

  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycbcr420fp_to_ycbcr420lp

DESCRIPTION
  This function converts from YCbCr 4:2:0 frame packed to YCbCr 4:2:0 line 
  packed format.
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
static ipl_status_type ipl_convert_ycbcr420fp_to_ycbcr420lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
)
{
  register unsigned char *inCrPtr; 
  register unsigned char *inCbPtr;
  register unsigned char *outClrPtr;
  uint32 x, w, h;

  MSG_LOW("ipl_convert_ycbcr420fp_to_ycbcr420lp marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr420fp_to_ycbcr420lp marker_200\n");
    return IPL_FAILURE;
  }

  if (!((input_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK &&
         output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) || 
       (input_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK &&
        output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)))
  {
    MSG_LOW("ipl_convert_ycbcr420fp_to_ycbcr420lp marker_201\n");
    return IPL_FAILURE;
  }

  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr420fp_to_ycbcr420lp marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;

  inCbPtr = input_img_ptr->clrPtr;
  inCrPtr = inCbPtr + w*h/4;
  outClrPtr = output_img_ptr->clrPtr;

  MSG_LOW("ipl_convert_ycbcr420fp_to_ycbcr420lp marker_1\n");

  /* Y values */
  if (output_img_ptr->imgPtr != input_img_ptr->imgPtr)
    memcpy(output_img_ptr->imgPtr, input_img_ptr->imgPtr, w*h);

  /* CbCr values */
  for (x = w*h/4; x; x--) 
  {
    *outClrPtr++ = *inCbPtr++;
    *outClrPtr++ = *inCrPtr++;
  } 

  MSG_LOW("ipl_convert_ycbcr420fp_to_ycbcr420lp marker_100\n");

  return IPL_SUCCESS;
} 




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
)
{
  unsigned char *inImgPtr, *outImgPtr, *inClrPtr, *outClrPtr;
  uint32 x, y, w, h;

  MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr420lp marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr420lp marker_200\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->cFormat != IPL_YCrCb422_LINE_PK &&
      input_img_ptr->cFormat != IPL_YCbCr422_LINE_PK) 
  {
    MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr420lp marker_201\n");
    return IPL_FAILURE;
  }

  if (output_img_ptr->cFormat != IPL_YCrCb420_LINE_PK &&
      output_img_ptr->cFormat != IPL_YCbCr420_LINE_PK) 
  {
    MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr420lp marker_202\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr420lp marker_203\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  inClrPtr = input_img_ptr->clrPtr;
  outClrPtr = output_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;

  MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr420lp marker_1\n");

  /* Y values */
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      *outImgPtr++ = *inImgPtr++;
    } /* end x loop */
  } /* end y loop */

  /* CbCr values */
  for (y = 0; y < h; y+=2) {
    for (x = 0; x < w; x+=2) {
      /* 4 pixels at a time */
      *outClrPtr++ = *inClrPtr++;
      *outClrPtr++ = *inClrPtr++;
    } /* end x loop */
    inClrPtr += w;
  } /* end y loop */

  MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr420lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr422lp_to_ycbcr420lp */


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
)
{
  unsigned char *inImgPtr, *outImgPtr, *inClrPtr, *outClrPtr, *out2ClrPtr;
  uint32 x, y, w, h;
  unsigned char cb, cr;

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr444lp marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr444lp marker_200\n");
    return IPL_FAILURE;
  }
  
  if ((input_img_ptr->cFormat != IPL_YCrCb420_LINE_PK && 
       input_img_ptr->cFormat != IPL_YCbCr420_LINE_PK) ||
      (output_img_ptr->cFormat != IPL_YCrCb444_LINE_PK && 
       output_img_ptr->cFormat != IPL_YCbCr444_LINE_PK)) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr444lp marker_201\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr444lp marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  inClrPtr = input_img_ptr->clrPtr;
  outClrPtr = output_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;
  out2ClrPtr = outClrPtr + w;

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr444lp marker_1\n");

  /* Y values */
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      *outImgPtr++ = *inImgPtr++;
    } /* end x loop */
  } /* end y loop */

  /* CbCr values */
  for (y = 0; y < h; y+=2) {
    for (x = 0; x < w; x+=2) {
      /* Process 4 pixels at a time */
      cb = *inClrPtr++;
      cr = *inClrPtr++;
      /* Pixel 1, row 1 */
      *outClrPtr++ = cb;
      *outClrPtr++ = cr;
      /* Pixel 2, row 1 */
      *outClrPtr++ = cb;
      *outClrPtr++ = cr;
      /* Pixel 1, row 2 */
      *out2ClrPtr++ = cb;
      *out2ClrPtr++ = cr;
      /* Pixel 2, row 2 */
      *out2ClrPtr++ = cb;
      *out2ClrPtr++ = cr;
    } /* end x loop */
    outClrPtr += w;
    out2ClrPtr += w;
  } /* end y loop */

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr444lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr420lp_to_ycbcr444lp */


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
)
{
  unsigned char *inImgPtr, *outImgPtr, *inClrPtr, *outClrPtr;
  uint32 x, y, w, h;

  MSG_LOW("ipl_convert_ycbcr444lp_to_ycbcr420lp marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr444lp_to_ycbcr420lp marker_200\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->cFormat != IPL_YCbCr444_LINE_PK ||
      (output_img_ptr->cFormat != IPL_YCbCr420_LINE_PK && 
       output_img_ptr->cFormat != IPL_YCrCb420_LINE_PK)) 
  {
    MSG_LOW("ipl_convert_ycbcr444lp_to_ycbcr420lp marker_201\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr444lp_to_ycbcr420lp marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  inClrPtr = input_img_ptr->clrPtr;
  outClrPtr = output_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;

  MSG_LOW("ipl_convert_ycbcr444lp_to_ycbcr420lp marker_1\n");

  /* Y values */
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      *outImgPtr++ = *inImgPtr++;
    } /* end x loop */
  } /* end y loop */

  /* CbCr values */
  for (y = 0; y < h; y+=2) {
    for (x = 0; x < w; x+=2) {
      /* 4 pixels at a time */
      *outClrPtr++ = *inClrPtr++;
      *outClrPtr++ = *inClrPtr++;
      inClrPtr += 2;
    } /* end x loop */
    inClrPtr += w;
  } /* end y loop */

  MSG_LOW("ipl_convert_ycbcr444lp_to_ycbcr420lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr444lp_to_ycbcr420lp */


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
)
{
  unsigned char *inImgPtr, *outImgPtr, *outClrPtr;
  uint32 x, y, w, h;

  MSG_LOW("ipl_convert_ycbcr422_to_ycbcr422lp marker_0\n");
 
  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycbcr422lp marker_200\n");
    return IPL_FAILURE;
  }
  
  if (input_img_ptr->cFormat != IPL_YCbCr ||
      output_img_ptr->cFormat != IPL_YCbCr422_LINE_PK) 
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycbcr422lp marker_201\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycbcr422lp marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  outClrPtr = output_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;

  MSG_LOW("ipl_convert_ycbcr422_to_ycbcr422lp marker_1\n");

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x+=2) {
      /* Process 2 pixels at a time */
      /* Pixel 1: CbY */
      *outClrPtr++ = *inImgPtr++;
      *outImgPtr++ = *inImgPtr++;

      /* Pixel 2: CrY */
      *outClrPtr++ = *inImgPtr++;
      *outImgPtr++ = *inImgPtr++;
    } /* end x loop */
  } /* end y loop */

  MSG_LOW("ipl_convert_ycbcr422_to_ycbcr422lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr422_to_ycbcr422lp */


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
  11/18/04  Created

===========================================================================*/
extern ipl_status_type ipl_convert_ycbcr422_to_ycrcb422lp
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
)
{
  unsigned char *inImgPtr, *outImgPtr, *outClrPtr;
  uint32 x;
  uint8 cb, cr;

  MSG_LOW("ipl_convert_ycbcr422_to_ycrcb422lp marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycrcb422lp marker_200\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->cFormat != IPL_YCbCr ||
      output_img_ptr->cFormat != IPL_YCrCb422_LINE_PK) 
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycrcb422lp marker_201\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycrcb422lp marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  outClrPtr = output_img_ptr->clrPtr;

  MSG_LOW("ipl_convert_ycbcr422_to_ycrcb422lp marker_1\n");

  for (x = (output_img_ptr->dx*output_img_ptr->dy)>>1; x; x--) {
    /* Process 2 pixels at a time */
    /* Pixel 1: CbY */
    cb = *inImgPtr++;
    *outImgPtr++ = *inImgPtr++;
    /* Pixel 2: CrY */
    cr = *inImgPtr++;
    *outImgPtr++ = *inImgPtr++;
    /* Output chroma values */
    *outClrPtr++ = cr;
    *outClrPtr++ = cb;
  } /* end x loop */

  MSG_LOW("ipl_convert_ycbcr422_to_ycrcb422lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr422_to_ycrcb422lp */



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
)
{
  unsigned char *inImgPtr, *outImgPtr, *inClrPtr;
  uint32 x, y, w, h;

  MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr422 marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr ||
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr422 marker_200\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->cFormat != IPL_YCbCr422_LINE_PK ||
      output_img_ptr->cFormat != IPL_YCbCr) 
  {
    MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr422 marker_201\n");
    return IPL_FAILURE;
  }
  if (input_img_ptr->dx != output_img_ptr->dx ||
      input_img_ptr->dy != output_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr422 marker_202\n");
    return IPL_FAILURE;
  }

  /* Initialize image pointers and local variables */
  inImgPtr = input_img_ptr->imgPtr;
  outImgPtr = output_img_ptr->imgPtr;
  inClrPtr = input_img_ptr->clrPtr;
  w = output_img_ptr->dx;
  h = output_img_ptr->dy;

  MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr422 marker_1\n");
  
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x+=2) {
      /* Process 2 pixels at a time */
      /* Pixel 1: CbY */
      *outImgPtr++ = *inClrPtr++;
      *outImgPtr++ = *inImgPtr++;
      /* Pixel 2: CrY */
      *outImgPtr++ = *inClrPtr++;
      *outImgPtr++ = *inImgPtr++;
    } /* end x loop */
  } /* end y loop */

  MSG_LOW("ipl_convert_ycbcr422lp_to_ycbcr422 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr422lp_to_ycbcr422 */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_rgb888pad

DESCRIPTION
  This function convert from RGB888 to RGB888_PAD format

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
static ipl_status_type ipl_convert_rgb888pad
(
  ipl_image_type* input_img_ptr,        /* Points to the input image  */
  ipl_image_type* output_img_ptr        /* Points to the output image */
)
{
  uint32 i;

  MSG_LOW("ipl_convert_rgb888pad marker_0\n");

  if (!input_img_ptr  || !input_img_ptr->imgPtr  || 
      !output_img_ptr || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb888pad marker_200\n");
    return IPL_FAILURE;
  }
  
  if (input_img_ptr->dx != output_img_ptr->dx || 
      input_img_ptr->dy != output_img_ptr->dy)
  {
    MSG_LOW("ipl_convert_rgb888pad marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_rgb888pad marker_1\n");

  /* Check input parameters */
  if (input_img_ptr->cFormat == IPL_RGB565) 
  {
    uint16 * data_in;
    unsigned char *data_out;

    data_in  = (uint16*) input_img_ptr->imgPtr;
    data_out = output_img_ptr->imgPtr;

    for(i = input_img_ptr->dx*input_img_ptr->dy; i; i--)
    {
      *data_out++ = (unsigned char) ((*data_in & 0xF800)>>8);
      *data_out++ = (unsigned char) ((*data_in & 0x07E0)>>3);
      *data_out++ = (unsigned char) ((*data_in & 0x001F)<<3);
      *data_out++ = 0;

      data_in++;
    }

    MSG_LOW("ipl_convert_rgb888pad marker_100\n");
    
    return IPL_SUCCESS;
  }
  else if (input_img_ptr->cFormat == IPL_RGB888) 
  {
    unsigned char * data_in;
    unsigned char * data_out;

    data_in = input_img_ptr->imgPtr;
    data_out = output_img_ptr->imgPtr;


    for(i = input_img_ptr->dx*input_img_ptr->dy; i; i--)
    {
      *data_out++ = *data_in++;
      *data_out++ = *data_in++;
      *data_out++ = *data_in++;
      *data_out++ = 0;
    }

    MSG_LOW("ipl_convert_rgb888pad marker_101\n");

    return IPL_SUCCESS;
  }
  else if (input_img_ptr->cFormat == IPL_RGB888_PAD) 
  {
    if (output_img_ptr->cFormat == IPL_RGB888) 
    {
      unsigned char * data_in;
      unsigned char * data_out;

      data_in = input_img_ptr->imgPtr;
      data_out = output_img_ptr->imgPtr;

      for(i = input_img_ptr->dx*input_img_ptr->dy; i; i--)
      {
        *data_out++ = *data_in++;
        *data_out++ = *data_in++;
        *data_out++ = *data_in++;
        data_in++;
      }
      MSG_LOW("ipl_convert_rgb888pad marker_102\n");

      return IPL_SUCCESS;
    }
    else
    {
      MSG_LOW("ipl_convert_rgb888pad marker_202\n");
      return IPL_FAILURE;
    }
  }
  else
  {
    MSG_LOW("ipl_convert_rgb888pad marker_203\n");
    return IPL_FAILURE;
  }

}


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB888RGB565be

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
)
{
  int32 i,j;

  MSG_LOW("ipl_RGB8882RGB565be marker_0\n");

  if (!data_in || !data_out)
  {
    MSG_LOW("ipl_RGB8882RGB565be marker_200\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_RGB8882RGB565be marker_1\n");

  for(i = 0; i < height; i++)
  {
    for(j = 0; j < width; j++)
    {
      *data_out = pack_rgb565(data_in[2], data_in[1], data_in[0]);
      data_in += 3;
      data_out++;
    }
  }

  MSG_LOW("ipl_RGB8882RGB565be marker_100\n");

  return IPL_SUCCESS;
}



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB888RGB565

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
)
{
  int32 i,j;

  MSG_LOW("ipl_RGB8882RGB565 marker_0\n");

  if (!data_in || !data_out)
  {
    MSG_LOW("ipl_RGB8882RGB565 marker_200\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_RGB8882RGB565 marker_1\n");

  for(i = 0; i < height; i++)
  {
    for(j = 0; j < width; j++)
    {
      *data_out = pack_rgb565(data_in[0], data_in[1], data_in[2]);
      data_in += 3;
      data_out++;
    }
  }

  MSG_LOW("ipl_RGB8882RGB565 marker_100\n");

  return IPL_SUCCESS;
}



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
)
{
  uint32 row,col;
  int32 frame_index = 0;
  unsigned short out;
  int32 luma1=0,luma2=0,cb=0,cr=0;
  int32 tY, tCb, tCr;
  unsigned char cbb=0,crr=0,lumaa1=0,lumaa2=0;
  long rc,gc,bc,r=0,g=0,b=0;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  short ycbcr2rgb_convert[6] = {8, 25803, -3071, -7672, 30399, 12};
  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};
  uint8* input_frame_ptr;
  uint8* output_frame_ptr;

  uint8*  outImgPtr;
  uint8*  outClrPtr;
  uint16* inImgPtr;
  int dx;
  uint16 v1,v2,v3,v4;


  MSG_LOW("ipl_conv_frame marker_0\n");

  if (!i_frame_ptr || !i_frame_ptr->imgPtr  || 
      !o_frame_ptr || !o_frame_ptr->imgPtr)
  {
    MSG_LOW("ipl_conv_frame marker_200\n");
    return IPL_FAILURE;
  }

  input_frame_ptr = i_frame_ptr->imgPtr;
  output_frame_ptr = o_frame_ptr->imgPtr;

  if (i_frame_ptr->cFormat == IPL_RGB565)
  {
    if (o_frame_ptr->cFormat == IPL_RGB565)
    {
      /* Input and output have the same color format */
      memcpy(output_frame_ptr, input_frame_ptr, 
             i_frame_ptr->dx*i_frame_ptr->dy*2);

      MSG_LOW("ipl_conv_frame marker_100\n");

      return IPL_SUCCESS;
    }
  } else if (i_frame_ptr->cFormat == IPL_YCbCr)
  {
    if (o_frame_ptr->cFormat == IPL_YCbCr)
    {
      /* Input and output have the same color format */
      memcpy(output_frame_ptr, input_frame_ptr, 
             i_frame_ptr->dx*i_frame_ptr->dy*2);

      MSG_LOW("ipl_conv_frame marker_101\n");

      return IPL_SUCCESS;
    }
  } else {
    /* Only YCbCr 4:2:2 or RGB565 is supported */
    MSG_LOW("ipl_conv_frame marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_conv_frame marker_1\n");

  /*
  ** If we come to this point, we need to do color conversion.
  */
  if (o_frame_ptr->cFormat == IPL_RGB565)
  {
    /* Do a YCbCr 4:2:2 -> RGB565 color conversion */
    /* Now loop through the image once */
    for(row = 0; row < i_frame_ptr->dy; row++){
      for(col = 0; col < (i_frame_ptr->dx); col=col+2){
        if (((*(uint16*)(input_frame_ptr + frame_index)) == transparentValue))
        {
          /* Copy transparent pixel over */
          *((uint16*)(output_frame_ptr + frame_index)) = transparentValue;
          *((uint16*)(output_frame_ptr + frame_index + 2)) = transparentValue;
        } else {
          /*
          ** Convert frame to rgb
          */
          /* This is Cb */
          cbb = *((uint8*)(input_frame_ptr + frame_index));
          /* Next byte is luma of first pixel */
          lumaa1 = *((uint8*)(input_frame_ptr + frame_index + 1));
          /* Next byte is cr */
          crr = *((uint8*)(input_frame_ptr + frame_index + 2));
          /* Next byte is luma of 2nd pixel */
          lumaa2 = *((uint8*)(input_frame_ptr + frame_index + 3));

          rc = (ycbcr2rgb_convert[0]*(cbb-128) + 
                ycbcr2rgb_convert[1]*(crr-128))*4+0x8000;
          gc = (ycbcr2rgb_convert[2]*(cbb-128) + 
                ycbcr2rgb_convert[3]*(crr-128))*4+0x8000;
          bc = (ycbcr2rgb_convert[4]*(cbb-128) + 
                ycbcr2rgb_convert[5]*(crr-128))*4+0x8000;
          r = lumaa1 + (rc>>16);
          g = lumaa1 + (gc>>16);
          b = lumaa1 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(output_frame_ptr + frame_index))=out;
          if (((*(uint16*)(output_frame_ptr + frame_index)) == 
              transparentValue))
          {
            *(uint16*)(output_frame_ptr + frame_index) = transparentValue + 1;
          }
          r = lumaa2 + (rc>>16);
          g = lumaa2 + (gc>>16);
          b = lumaa2 + (bc>>16);
          r = CLIPIT(r);
          g = CLIPIT(g);
          b = CLIPIT(b);
          out = pack_rgb565(r,g,b);
          *((uint16*)(output_frame_ptr + frame_index + 2)) = out;
          if (((*(uint16*)(output_frame_ptr + frame_index+2)) == 
              transparentValue))
          {
            *(uint16*)(output_frame_ptr + frame_index+2) = transparentValue+1;
          }
        }
        frame_index += 4;
      } /* End of col loop */
    } /* End of row loop */
  } 
  else if (o_frame_ptr->cFormat == IPL_YCbCr)
  {
    /* Do a RGB565 -> YCbCr 4:2:2 color conversion */
    /* Now loop through the image once */
    for(row = 0; row < i_frame_ptr->dy; row++){
      for(col = 0; col < (i_frame_ptr->dx); col=col+2){
        if ( ((*(uint16*)(input_frame_ptr + frame_index)) == transparentValue))
        {
          /* Copy transparent pixel over */
          *((uint16*)(output_frame_ptr + frame_index)) = transparentValue;
          *((uint16*)(output_frame_ptr + frame_index + 2)) = transparentValue;
        } else {
          /*
          ** Convert frame to YCbCr 4:2:2
          */
          out = *((uint16*)(input_frame_ptr + frame_index));
          unpack_rgb565(out,(uint8*)&r,(uint8*)&g,(uint8*)&b);
          luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                   ycbcr_convert[2]*b)*4 + 0x8000;
          luma1 = (luma1>>16) + 16;
          luma1 = CLIPIT(luma1);
          cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g + 
                ycbcr_convert[5]*b)*4 + 0x8000;
          cb = (cb>>16) + 128;
          cb = CLIPIT(cb);
          /* 2nd pixel */
          out = *((uint16*)(input_frame_ptr + frame_index+2));
          unpack_rgb565(out,(uint8*)&r,(uint8*)&g,(uint8*)&b);
          luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g + 
                   ycbcr_convert[2]*b)*4 + 0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g + 
                ycbcr_convert[8]*b)*4 + 0x8000;
          cr = (cr>>16) + 128;
          cr = CLIPIT(cr);
          *((uint8*)(output_frame_ptr + frame_index)) = (uint8)cb;
          *((uint8*)(output_frame_ptr + frame_index+1)) = (uint8)luma1;
          if (((*(uint16*)(output_frame_ptr + frame_index)) == 
              transparentValue))
          {
            *(uint16*)(output_frame_ptr + frame_index) = transparentValue + 1;
          }
          *((uint8*)(output_frame_ptr + frame_index+2)) = (uint8)cr;
          *((uint8*)(output_frame_ptr + frame_index+3)) = (uint8)luma2;
          if (((*(uint16*)(output_frame_ptr + frame_index+2)) == 
              transparentValue))
          {
            *(uint16*)(output_frame_ptr + frame_index+2) = transparentValue+1;
          }
        }
        frame_index += 4;
      } /* End of col loop */
    } /* End of row loop */
  } 
  else if (o_frame_ptr->cFormat == IPL_YCrCb420_LINE_PK ||
           o_frame_ptr->cFormat == IPL_YCbCr420_LINE_PK)
  {

    if (i_frame_ptr->dx != o_frame_ptr->dx ||
        i_frame_ptr->dy != o_frame_ptr->dy)
    {
      MSG_FATAL("ipl_conv_frame marker_202\n");
      return IPL_FAILURE;
    }

    if (i_frame_ptr->dx%2 || i_frame_ptr->dy%2)
    {
      MSG_FATAL("ipl_conv_frame marker_203\n");
      return IPL_FAILURE;
    }

    inImgPtr  = (uint16 *) i_frame_ptr->imgPtr;
    outImgPtr = o_frame_ptr->imgPtr;
    outClrPtr = o_frame_ptr->clrPtr;

    dx = i_frame_ptr->dx;

    unpack_rgb565(transparentValue, (uint8 *) &r,(uint8 *) &g,(uint8 *) &b);
    //printf("r %d, g %d, b %d\n", r,g,b);
    tY = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
          ycbcr_convert[2]*b)*4+0x8000;
    tY = (tY >> 16) + 16;
    tY = CLIPIT(tY);

    tCr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g+ycbcr_convert[8]*b)*4+0x8000;
    tCr = (tCr >> 16) + 128;
    tCr = CLIPIT(tCr);

    tCb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g+ycbcr_convert[5]*b)*4+0x8000;
    tCb = (tCb >> 16) + 128;
    tCb = CLIPIT(tCb);

    //printf("tY %d, tCb %d, tCr %d\n", tY, tCb, tCr);
    

    for (row = 0; row < i_frame_ptr->dy; row += 2)
    {
      for (col = 0; col < i_frame_ptr->dx; col += 2)
      {
        v1 = *(inImgPtr);         
        v2 = *(inImgPtr + 1);     
        v3 = *(inImgPtr + dx);  
        v4 = *(inImgPtr + dx + 1);

        inImgPtr  += 2;

        if (v1 == transparentValue || v2 == transparentValue ||
            v3 == transparentValue || v4 == transparentValue)
        {
          // set all 4 lumas to transparent
          *(outImgPtr )         = tY;  //107
          *(outImgPtr + 1)      = tY;  //107
          *(outImgPtr + dx)     = tY;  //107
          *(outImgPtr + dx + 1) = tY;  //107

          outImgPtr += 2;

          // set cr and cb to transparent
          *outClrPtr++ = tCr;  // 222;
          *outClrPtr++ = tCb;  // 202;
        } 
        else 
        {
          /* Pixel 1 */
          unpack_rgb565(v1, (uint8 *) &r,(uint8 *) &g,(uint8 *) &b);
          luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                   ycbcr_convert[2]*b)*4+0x8000;
          luma1 = (luma1 >> 16) + 16;
          luma1 = CLIPIT(luma1);
          cr = (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
                ycbcr_convert[8]*b)*4+0x8000;
          cb = (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
                ycbcr_convert[5]*b)*4+0x8000;
    
          /* Pixel 2 */
          unpack_rgb565(v2, (uint8 *) &r,(uint8 *) &g,(uint8 *) &b);
          luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                    ycbcr_convert[2]*b)*4+0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr += (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
                 ycbcr_convert[8]*b)*4+0x8000;
          cb += (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
                 ycbcr_convert[5]*b)*4+0x8000;
    
          /* Output luma */
          *outImgPtr     = (uint8)luma1;
          *(outImgPtr+1) = (uint8)luma2;
    
    
          /* Pixel 3 */
          unpack_rgb565(v3, (uint8 *) &r,(uint8 *) &g,(uint8 *) &b);
          luma1 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                   ycbcr_convert[2]*b)*4+0x8000;
          luma1 = (luma1 >> 16) + 16;
          luma1 = CLIPIT(luma1);
          cr += (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
                 ycbcr_convert[8]*b)*4+0x8000;
          cb += (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
                 ycbcr_convert[5]*b)*4+0x8000;
    
          /* Pixel 4 */
          unpack_rgb565(v4, (uint8 *) &r,(uint8 *) &g,(uint8 *) &b);
          luma2 = (ycbcr_convert[0]*r + ycbcr_convert[1]*g +
                   ycbcr_convert[2]*b)*4+0x8000;
          luma2 = (luma2>>16) + 16;
          luma2 = CLIPIT(luma2);
          cr += (ycbcr_convert[6]*r + ycbcr_convert[7]*g +
                 ycbcr_convert[8]*b)*4+0x8000;
          cb += (ycbcr_convert[3]*r + ycbcr_convert[4]*g +
                 ycbcr_convert[5]*b)*4+0x8000;
    
          /* Output luma */
          *(outImgPtr+dx)   = (uint8)luma1;
          *(outImgPtr+dx+1) = (uint8)luma2;

          outImgPtr += 2;

    
          /* Output chroma */
          // shift twice to average, 16 time for Q 
          cr = (cr>>18) + 128;
          cr = CLIPIT(cr);
          *outClrPtr++ = (uint8)cr;
    
          cb = (cb>>18) + 128;
          cb = CLIPIT(cb);
          *outClrPtr++ = (uint8)cb;
        }

      } /* End of col loop */

      inImgPtr  += dx;
      outImgPtr += dx;
    } /* End of row loop */
  } 
  else 
  {
    MSG_LOW("ipl_conv_frame marker_202\n");
    /* Only YCbCr 4:2:2 or RGB565 output supported */
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_conv_frame marker_102\n");

  return IPL_SUCCESS;
} /* End ipl_conv_frame */



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
)
{
  ipl_status_type status;

  MSG_LOW("ipl_convert_to_rgb444666888 marker_0\n");

  if (!output_img_ptr)
  {
    MSG_LOW("ipl_convert_to_rgb444666888 marker_200\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_to_rgb444666888 marker_1\n");
  
  switch (output_img_ptr->cFormat)
  {
    case IPL_RGB444:
      status = ipl_convert_to_rgb444(input_img_ptr, output_img_ptr);
    break;

    case IPL_RGB666:
      status = ipl_convert_to_rgb666(input_img_ptr, output_img_ptr);
    break;

    case IPL_RGB888:
      status = ipl_convert_to_rgb888(input_img_ptr, output_img_ptr);
    break;

    default:
      status = IPL_FAILURE;
    break;
  }

  MSG_LOW("ipl_convert_to_rgb444666888 marker_100\n");

  return status;
} /* ipl_convert_to_rgb444666888 */



/* <EJECT> */
/*===========================================================================

FUNCTION hsv_to_ycbcr_pixel

DESCRIPTION
  This function converts from HSV to YCbCr.

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
  int32 h,
  int32 s,
  int32 i,
  unsigned char *luma,
  unsigned char *cb,
  unsigned char *cr
)
{
  int32 luma1=0,red1,g1,b1,frac,p,q,t;
  int32 cb1,cr1;

  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int32 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  // normalize between 0 and 255 also
  h = ((h*360) / 255);


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
  } 
  else if (h<=120)
  {
    red1 = q;
    g1 = i;
    b1 = p;
  } else if (h<=180)
  {
    red1 = p;
    g1 = i;
    b1 = t;
  } 
  else if (h<=240)
  {
    red1 = p;
    g1 = q;
    b1 = i;
  } 
  else if (h<=300)
  {
    red1 = t;
    g1 = p;
    b1 = i;
  } 
  else 
  {
    red1 = i;
    g1 = p;
    b1 = q;
  }

  /* Convert to Y Cb Cr */
  luma1 = (ycbcr_convert[0]*red1 + ycbcr_convert[1]*g1 + 
               ycbcr_convert[2]*b1)*4 + 0x8000;
  luma1 = (luma1>>16) + 16;
  luma1 = CLIPIT(luma1);
  cb1 = (ycbcr_convert[3]*red1 + ycbcr_convert[4]*g1 + 
            ycbcr_convert[5]*b1)*4 + 0x8000;
  cb1 = (cb1>>16) + 128;
  cb1 = CLIPIT(cb1);

  cr1 = (ycbcr_convert[6]*red1 + ycbcr_convert[7]*g1 + 
            ycbcr_convert[8]*b1)*4 + 0x8000;
  cr1 = (cr1>>16) + 128;
  cr1 = CLIPIT(cr1);

  *cb = (unsigned char) cb1;
  *cr = (unsigned char) cr1;
  *luma = (unsigned char) luma1;

  return IPL_SUCCESS;
}  



/* <EJECT> */
/*===========================================================================

FUNCTION to_bayer

DESCRIPTION
  This function converts images to BAYER format

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
static ipl_status_type to_bayer
(
  ipl_image_type* i_img_ptr,     /* Points to the input image   */
  ipl_image_type* o_img_ptr      /* Points to the output image  */
)
{
  uint32 row,col; 
  uint16 * iptr;
  uint8  * optr;
  int val;

  // do some sanity checks
  if (!i_img_ptr || !o_img_ptr)
    return IPL_FAILURE;

  if (i_img_ptr->dx % 2 || i_img_ptr->dy % 2)
    return IPL_FAILURE;

  iptr = (uint16 *) i_img_ptr->imgPtr;
  optr = (uint8 *) o_img_ptr->imgPtr;

  MSG_LOW("to_bayer marker_1\n");

  /* Now loop through the image once */
  if (i_img_ptr->cFormat == IPL_RGB565)
  {
    if (o_img_ptr->cFormat == IPL_BAYER_BGGR)
    {
      for(row = 0; row < i_img_ptr->dy; row++)
      {
        for(col = 0; col < i_img_ptr->dx; col++)
        {
          val = *iptr++;

          if (row & 0x0001)
          {
            // . .
            // . R
            if (col & 0x0001)
            {
              *optr++ = (uint8) ((val&0xF800) >> 8);
            }
            // . .
            // G .
            else
            {
              *optr++ = (uint8) ((val&0x07E0) >> 3);
            }
          }
          else
          {
            // . G
            // . .
            if (col & 0x0001)
            {
              *optr++ = (uint8) ((val&0x07E0) >> 3);
            }
            // B .
            // . .
            else
            {
              *optr++ = (uint8) ((val&0x001F) << 3);
            }
          }
        }  
      }  
    }
    else
      return IPL_FAILURE;
  }
  else
    return IPL_FAILURE;

  MSG_LOW("to_bayer marker_2\n");

  return IPL_SUCCESS;
} 




/* <EJECT> */
/*===========================================================================

FUNCTION ycbcr_to_hsv_pixel

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
extern ipl_status_type ycbcr_to_hsv_pixel
(
  int32 luma,
  int32 cb,
  int32 cr,
  int32 *hout,
  int32 *sout,
  int32 *vout
)
{
  uint32 s,delta;
  int32 r1,g1,b1,min,max,h;
  int32 rc,gc,bc;

  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int32 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};

  /* Get RGB 24 bit */
  rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
  gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
  bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;

  r1 = luma + (rc>>16);
  g1 = luma + (gc>>16);
  b1 = luma + (bc>>16);

  r1=CLIPIT(r1);
  g1=CLIPIT(g1);
  b1=CLIPIT(b1);

  /*
  **  Do HSV for 1st pixel
  **    Hue is between 0 - 360
  **    Saturation is between 0 - 255
  **    Value is between 0 - 255
  */
  min = min3(r1,g1,b1);
  max = max3(r1,g1,b1);
  delta = max - min;

  if (max!=0)
  {
    s = ((delta<<16)*255)/max;
    s = (s + 0x8000)>>16;
  } 
  else 
  {
    /* r = g = b =0 */
    s = 0;
    h = 0;
  }

  if (delta != 0)
  {
    if (r1 == max)
      h = (((int32)((int32)g1 - b1)<<16)*60)/(int32)delta; /* h is a Q16 */
    else if (g1 == max)
      h = ((((int32)((int32)b1 - r1)<<16)*60)/(int32)delta) + 
              (120<<16); /* h is a Q16 */
    else 
      h = ((((int32)((int32)r1 - g1)<<16)*60)/(int32)delta) + 
              (240<<16); /* h is a Q16 */

    h = (h+0x8000)>>16;
  } 
  else 
  {
    /* r = g = b */
    s = 0;
    h = 0;
  }

  if (h < 0)
    h = h + 360;

  // normalize between 0 and 255 also
  h = ((h*255) / 360);

  *hout = h;
  *sout = s;
  *vout = max;

  return IPL_SUCCESS;
}  



/* <EJECT> */
/*===========================================================================

FUNCTION to_hsv_normalized

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
static ipl_status_type to_hsv_normalized
(
  ipl_image_type* input_img_ptr,     /* Points to the input image   */
  ipl_image_type* hsv_buffer_in
)
{
  uint32 row,col, s,i,delta;
  int32 r1,g1,b1,min,max,h;
  uint8 * hsv_buffer;
  uint16 * in;
  uint32 val;

  MSG_LOW("to_hsv_normalized marker_0\n");

  if(!hsv_buffer_in || !hsv_buffer_in->imgPtr || 
     !input_img_ptr || !input_img_ptr->imgPtr)
  {
    MSG_LOW("to_hsv_normalized marker_200\n");
    return IPL_FAILURE;
  }

  hsv_buffer = hsv_buffer_in->imgPtr;
  in = (uint16*) input_img_ptr->imgPtr;

  MSG_LOW("to_hsv_normalized marker_1\n");

  /* Now loop through the image once */
  for(row = 0; row < input_img_ptr->dy; row++)
  {
    for(col = 0; col < input_img_ptr->dx; col++)
    {
      val = *in++;

      r1 = (uint8) ((val&0xF800) >>8);
      g1   = (uint8) ((val&0x07E0) >>3);
      b1   = (uint8) ((val&0x001F) <<3);

      /*
      **  Do HSV for 1st pixel
      **    Hue is between 0 - 360
      **    Saturation is between 0 - 255
      **    Value is between 0 - 255
      */
      min = min3(r1,g1,b1);
      max = max3(r1,g1,b1);
      delta = max - min;
      if (max!=0)
      {
        s = ((delta<<16)*255)/max;
        s = (s + 0x8000)>>16;
      } 
      else 
      {
        /* r = g = b =0 */
        s = 0;
        h = 0;
      }

      if (delta!=0)
      {
        if (r1 == max)
        {
          h = (((int32)((int32)g1 - b1)<<16)*60)/(int32)delta; /* h is a Q16 */
        } 
        else if (g1 == max)
        {
          h = ((((int32)((int32)b1 - r1)<<16)*60)/(int32)delta) + 
              (120<<16); /* h is a Q16 */
        } 
        else 
        {
          h = ((((int32)((int32)r1 - g1)<<16)*60)/(int32)delta) + 
              (240<<16); /* h is a Q16 */
        }
        h = (h+0x8000)>>16;
      } 
      else 
      {
        /* r = g = b */
        s = 0;
        h = 0;
      }

      if (h < 0)
      {
        h = h + 360;
      }
      // normalize between 0 and 255 also
      h = ((h*255) / 360);

      i = max;

      *hsv_buffer++ = (uint8) h;
      *hsv_buffer++ = (uint8) s;
      *hsv_buffer++ = (uint8) i;
    } /* End of col loop */
  } /* End of row loop */

  MSG_LOW("to_hsv_normalized marker_100\n");

  return IPL_SUCCESS;
}  /* End to_hsv */




/* <EJECT> */
/*===========================================================================

FUNCTION ycbcr_to_hsv_normalized

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
extern ipl_status_type ycbcr_to_hsv_normalized
(
  ipl_image_type* input_img_ptr,     /* Points to the input image   */
  ipl_image_type* hsv_buffer_in
)
{
  uint32 row,col,index,s,i,delta;
  int32 cb,cr,luma1=0,luma2=0,red1,g1,b1,red2,g2,b2,min,max,h;
  int32 rc,gc,bc;
  uint8 * hsv_buffer;

  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int32 ycbcr_convert[6] = {8, 25803, -3071, -7672,30399,12};

  MSG_LOW("ycbcr_to_hsv_normalized marker_0\n");

  if(!hsv_buffer_in || !hsv_buffer_in->imgPtr || 
     !input_img_ptr || !input_img_ptr->imgPtr)
  {
    MSG_LOW("ycbcr_to_hsv_normalized marker_200\n");
    return IPL_FAILURE;
  }

  hsv_buffer = hsv_buffer_in->imgPtr;

  MSG_LOW("ycbcr_to_hsv_normalized marker_1\n");

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
      // normalize between 0 and 255 also
      h = ((h*255) / 360);



      i = max;
      index = ((row*input_img_ptr->dx) + col)*3;




      hsv_buffer[index] = (uint8) h;
      hsv_buffer[index+1] = (uint8) s;
      hsv_buffer[index+2] = (uint8) i;

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
      // normalize between 0 and 255 also
      h = ((h*255) / 360);
      i = max;



      hsv_buffer[index+3] = (uint8) h;
      hsv_buffer[index+4] = (uint8) s;
      hsv_buffer[index+5] = (uint8) i;
    } /* End of col loop */
  } /* End of row loop */

  MSG_LOW("ycbcr_to_hsv_normalized marker_100\n");

  return IPL_SUCCESS;
}  /* End ycbcr_to_hsv */




/* <EJECT> */
/*===========================================================================

FUNCTION hsv_to_normalized

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
static ipl_status_type hsv_to_normalized
(
  ipl_image_type* hsv_buffer,
  ipl_image_type* output_img_ptr    /* Points to the output image   */
)
{
  uint32 row,col,h,s,i;
  int32 r1,g1,b1,frac,p,q,t;
  uint8 * in; 
  uint16 * out; 

  MSG_LOW("hsv_to_normalized marker_0\n");

  if(!hsv_buffer || !hsv_buffer->imgPtr ||
     !output_img_ptr  || !output_img_ptr->imgPtr)
  {
    MSG_LOW("hsv_to_normalized marker_200\n");
    return IPL_FAILURE;
  }

  MSG_LOW("hsv_to_normalized marker_1\n");

  in = hsv_buffer->imgPtr; 
  out = (uint16 *) output_img_ptr->imgPtr; 

  if (output_img_ptr->cFormat == IPL_RGB565)
  {
    /* Now loop through the image once */
    for(row = 0; row < output_img_ptr->dy; row++)
    {
      for(col = 0; col < output_img_ptr->dx; col++)
      {
        /* Read the hsv data for 1st pixel */
        h = *in++;
        s = *in++;
        i = *in++;

        // normalize between 0 and 255 also
        h = ((h*360) / 255);

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
          r1 = i;
          g1 = t;
          b1 = p;
        }  
        else if (h<=120)
        {
          r1 = q;
          g1 = i;
          b1 = p;
        } 
        else if (h<=180)
        {
          r1 = p;
          g1 = i;
          b1 = t;
        }  
        else if (h<=240)
        {
          r1 = p;
          g1 = q;
          b1 = i;
        } 
        else if (h<=300)
        {
          r1 = t;
          g1 = p;
          b1 = i;
        } 
        else 
        {
          r1 = i;
          g1 = p;
          b1 = q;
        }

        *out++ = pack_rgb565(r1, g1, b1);
      } /* End of col loop */
    } /* End of row loop */
  }
  else
    return IPL_FAILURE;


  MSG_LOW("hsv_to_normalized marker_100\n");
  return IPL_SUCCESS;
} /* End hsv_to_ycbcr */




/* <EJECT> */
/*===========================================================================

FUNCTION hsv_to_ycbcr_normalized

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
extern ipl_status_type hsv_to_ycbcr_normalized
(
  ipl_image_type* hsv_buffer,
  ipl_image_type* output_img_ptr    /* Points to the output image   */
)
{
  uint32 row,col,index,h,s,i;
  int32 luma1=0,luma2=0,cb,cr,red1,g1,b1,red2,g2,b2,frac,p,q,t;

  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int32 ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("hsv_to_ycbcr_normalized marker_0\n");

  if(!hsv_buffer || !hsv_buffer->imgPtr ||
     !output_img_ptr  || !output_img_ptr->imgPtr)
  {
    MSG_LOW("hsv_to_ycbcr_normalized marker_200\n");
    return IPL_FAILURE;
  }

  MSG_LOW("hsv_to_ycbcr_normalized marker_1\n");
  /* Now loop through the image once */
  for(row = 0; row < output_img_ptr->dy; row++)
  {
    for(col = 0; col < (output_img_ptr->dx); col=col+2)
    {
      index = ((row*output_img_ptr->dx) + col)*3;

      /* Read the hsv data for 1st pixel */
      h = hsv_buffer->imgPtr[index];
      s = hsv_buffer->imgPtr[index+1];
      i = hsv_buffer->imgPtr[index+2];

      // normalize between 0 and 255 also
      h = ((h*360) / 255);


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
      h = hsv_buffer->imgPtr[index+3];
      s = hsv_buffer->imgPtr[index+4];
      i = hsv_buffer->imgPtr[index+5];

      // normalize between 0 and 255 also
      h = ((h*360) / 255);


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

  MSG_LOW("hsv_to_ycbcr_normalized marker_100\n");
  return IPL_SUCCESS;
} /* End hsv_to_ycbcr */


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
)
{
  uint8 cb=0,cr=0,luma1=0,luma2=0;
  uint16 i,j;
  register uint8* data_out;
  register uint8* data2_out;
  register uint16* data_in;
  register uint8* data8_in;
  int32 rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0, coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int16 ycbcr_convert[6] = {8, 25803, -3071, -7672, 30399, 12};
  uint16 width;
  uint16 in;
  uint8 rs,gs,bs;
  uint8 r_mask[2][2] = {{0,0},{0,0}};
  uint8 g_mask[2][2] = {{0,0},{0,0}};
  uint8 b_mask[2][2] = {{0,0},{0,0}};
  register uint8 rowm2,colm2;

  MSG_LOW("ipl_convert_to_rgb888 marker_0\n");
  
  if (!input_img_ptr   || !input_img_ptr->imgPtr || 
      !output_img_ptr  || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_to_rgb888 marker_200\n");
    return IPL_FAILURE;
  }

  data_out = output_img_ptr->imgPtr;
  data2_out = output_img_ptr->imgPtr;
  data_in = (uint16*)input_img_ptr->imgPtr;
  data8_in = input_img_ptr->imgPtr;
  width = (uint16)input_img_ptr->dx;

  /*
  **  Verify that the input is RGB565, YCbCr 4:2:2, or Bayer
  */
  if (!((input_img_ptr->cFormat == IPL_YCbCr)
        || (input_img_ptr->cFormat == IPL_RGB565)
        || (input_img_ptr->cFormat == IPL_YCbCr444)
        || (input_img_ptr->cFormat == IPL_BAYER_GBRG)
        || (input_img_ptr->cFormat == IPL_BAYER_BGGR)
        || (input_img_ptr->cFormat == IPL_BAYER_GRBG)
        || (input_img_ptr->cFormat == IPL_BAYER_RGGB)))
  {
    MSG_LOW("ipl_convert_to_rgb888 marker_201\n");
    return IPL_FAILURE;
  }
  /*
  ** Verify that the output is RGB888
  */
  if (output_img_ptr->cFormat != IPL_RGB888)
  {
    MSG_LOW("ipl_convert_to_rgb888 marker_202\n");
    return IPL_FAILURE;
  }
  /*
  ** Verify that the input and output dimensions are identical
  */
  if (!((input_img_ptr->dx == output_img_ptr->dx) &&
      (input_img_ptr->dy == output_img_ptr->dy)))
  {
    MSG_LOW("ipl_convert_to_rgb888 marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_to_rgb888 marker_1\n");

  if (input_img_ptr->cFormat == IPL_YCbCr)
  {
    /*
    ** Convert from YCbCr 4:2:2 to RGB 888
    */
    for(i = 0; i < input_img_ptr->dy; i++)
    {
      for(j = 0; j < input_img_ptr->dx; j=j+2)
      {
        /* We will do conversion on 2 pixels at a time */
        /* This is Cb */
        cb = *data8_in;
        data8_in++;
        /* Next byte is luma of 1st pixel */
        luma1 = *data8_in;
        data8_in++;
        /* Next byte is Cr */
        cr = *data8_in;
        data8_in++;
        /* Next byte is luma of 2nd pixel */
        luma2 = *data8_in;
        data8_in++;
        rc = (ycbcr_convert[0]*(cb-128) + 
              ycbcr_convert[1]*(cr-128))*4 + 0x8000;
        gc = (ycbcr_convert[2]*(cb-128) + 
              ycbcr_convert[3]*(cr-128))*4 + 0x8000;
        bc = (ycbcr_convert[4]*(cb-128) + 
              ycbcr_convert[5]*(cr-128))*4 + 0x8000;
        r = luma1 + (rc>>16);
        g = luma1 + (gc>>16);
        b = luma1 + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        *data_out = (uint8)r;
        data_out++;
        *data_out = (uint8)g;
        data_out++;
        *data_out = (uint8)b;
        data_out++;
        r = luma2 + (rc>>16);
        g = luma2 + (gc>>16);
        b = luma2 + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        *data_out = (uint8)r;
        data_out++;
        *data_out = (uint8)g;
        data_out++;
        *data_out = (uint8)b;
        data_out++;
      }
    }
  } else if (input_img_ptr->cFormat == IPL_RGB565)
  {
    /*
    ** Input is RGB565
    */
    /*
    ** Convert from RGB565 to RGB666
    */
    for(i = 0; i < input_img_ptr->dy; i++)
    {
      for(j = 0; j < input_img_ptr->dx; j++)
      {
        /*
        ** Read in pixel, unpack, and write out
        */
        in = *data_in;
        data_in++;
        rs = (uint8)((in&0xF800)>>8);
        gs = (uint8)((in&0x07E0)>>3);
        bs = (uint8)((in&0x001F)<<3);
        *data_out = rs;
        data_out++;
        *data_out = gs;
        data_out++;
        *data_out = bs;
        data_out++;
      }
    }
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr444)
  {
    /*
    ** Convert from YCbCr 4:4:4 to RGB 888
    */
    for(i = 0; i < input_img_ptr->dy; i++)
    {
      for(j = 0; j < input_img_ptr->dx; j++)
      {
        /* This is Cb */
        cb = *data8_in;
        data8_in++;

        /* Next byte is Cr */
        cr = *data8_in;
        data8_in++;

        /* Next byte is luma of 1st pixel */
        luma1 = *data8_in;
        data8_in++;

        r = CLIPIT((luma1 + ipl_crr[cr]));
        g = CLIPIT((luma1 + ((ipl_crg[cr] + ipl_cbg[cb]) >> 16)));
        b = CLIPIT((luma1 + ipl_cbb[cb]));


        /*
        rc = (ycbcr_convert[0]*(cb-128) + 
              ycbcr_convert[1]*(cr-128))*4 + 0x8000;
        gc = (ycbcr_convert[2]*(cb-128) + 
              ycbcr_convert[3]*(cr-128))*4 + 0x8000;
        bc = (ycbcr_convert[4]*(cb-128) + 
              ycbcr_convert[5]*(cr-128))*4 + 0x8000;
        r = luma1 + (rc>>16);
        g = luma1 + (gc>>16);
        b = luma1 + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        */

        *data_out = (uint8)r;
        data_out++;
        *data_out = (uint8)g;
        data_out++;
        *data_out = (uint8)b;
        data_out++;
      }
    }
  } 
  else 
  {
    /*
    ** Bayer input
    */
    if (input_img_ptr->cFormat == IPL_BAYER_GBRG)
    {
      r_mask[1][0] = 1;
      g_mask[1][1] = 1;
      b_mask[0][1] = 1;
    } else if (input_img_ptr->cFormat == IPL_BAYER_BGGR)
    {
      r_mask[1][1] = 1;
      g_mask[1][0] = 1;
      b_mask[0][0] = 1;
    } else if (input_img_ptr->cFormat == IPL_BAYER_GRBG)
    {
      r_mask[0][1] = 1;
      g_mask[0][0] = 1;
      b_mask[1][0] = 1;
    } else if (input_img_ptr->cFormat == IPL_BAYER_RGGB)
    {
      r_mask[0][0] = 1;
      g_mask[0][1] = 1;
      b_mask[1][1] = 1;
    } else {
      MSG_LOW("ipl_convert_to_rgb888 marker_204\n");
      /*
      ** Unsupported format
      */
      return IPL_FAILURE;
    }

    /*
    ** Now process pixels in the inner rectangle
    */
    data_out += (output_img_ptr->dx + 1) * 3;

    for (i=1;i<output_img_ptr->dy-1;i++)
    {
      rowm2 = i%2;
      for (j=1;j<output_img_ptr->dx-1;j++)
      {
        colm2 = j%2;
        if (r_mask[rowm2][colm2])
        {
          /*
          ** R Input Position
          */
          r = *(uint8*)(input_img_ptr->imgPtr + (i*width+j));
          /*
          ** Find G
          */
          rc = ( *(uint8*)(input_img_ptr->imgPtr + (i*width+j-1)) - \
            *(uint8*)(input_img_ptr->imgPtr + (i*width+j+1)));
          rc = rc*rc;
          bc = ( *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j)) - \
            *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j)));
          bc = bc*bc;
          if (rc > bc)
          {
            gc = ( *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j)) + \
              *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j)));
          } else {
            gc = ( *(uint8*)(input_img_ptr->imgPtr + (i*width+j-1)) + \
              *(uint8*)(input_img_ptr->imgPtr + (i*width+j+1)));
          }
          g = (uint8)(gc>>1);
          /*
          ** Find B
          */
          bc = ( *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j-1)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j+1)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j-1)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j+1)));
          b = (uint8)(bc >>2);
        } else if (b_mask[rowm2][colm2])
        {
          /*
          ** B Input Position
          */
          b = *(uint8*)(input_img_ptr->imgPtr + (i*width+j));
          /*
          ** Interpolate G and R
          */
          /*
          ** Find G
          */
          rc = ( *(uint8*)(input_img_ptr->imgPtr + (i*width+j-1)) - \
            *(uint8*)(input_img_ptr->imgPtr + (i*width+j+1)));
          rc = rc*rc;
          bc = ( *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j)) - \
            *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j)));
          bc = bc*bc;
          if (rc > bc)
          {
            gc = ( *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j)) + \
              *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j)));
          } else {
            gc = ( *(uint8*)(input_img_ptr->imgPtr + (i*width+j-1)) + \
              *(uint8*)(input_img_ptr->imgPtr + (i*width+j+1)));
          }
          g = (uint8)(gc>>1);
          /*
          ** Find R
          */
          rc = ( *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j-1)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j+1)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j-1)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j+1)));
          r = (uint8)(rc >>2);
        } else if (g_mask[rowm2][colm2])
        {
          /*
          ** G Input Position
          */
          g = *(uint8*)(input_img_ptr->imgPtr + (i*width+j));
          /*
          ** Interpolate R and B
          */
          rc = ( *(uint8*)(input_img_ptr->imgPtr + ((i)*width+j-1)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i)*width+j+1)) );
          r = (uint8)(rc >>1);
          bc = ( *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j)) );
          b = (uint8)(bc >>1);
        } else
        {
          /*
          ** G Input Position
          */
          g = *(uint8*)(input_img_ptr->imgPtr + (i*width+j));
          /*
          ** Interpolate R and B
          */
          bc = ( *(uint8*)(input_img_ptr->imgPtr + ((i)*width+j-1)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i)*width+j+1)) );
          b = (uint8)(bc >>1);
          rc = ( *(uint8*)(input_img_ptr->imgPtr + ((i-1)*width+j)) + \
                 *(uint8*)(input_img_ptr->imgPtr + ((i+1)*width+j)) );
          r = (uint8)(rc >>1);
        }
        *data_out = (uint8)r;
        data_out++;
        *data_out = (uint8)g;
        data_out++;
        *data_out = (uint8)b;
        data_out++;
      }
      data_out += 6;
    }
    
    /*
    ** Now Process the N and S rows
    */
    data_out = output_img_ptr->imgPtr;
    data2_out = output_img_ptr->imgPtr + output_img_ptr->dx*3;

    for (j=0;j<output_img_ptr->dx*3;j++)
    {
      *data_out = *data2_out;
      data_out++;
      data2_out++;
    }

    data_out = output_img_ptr->imgPtr + 
               output_img_ptr->dx*(output_img_ptr->dy-1)*3;
    data2_out = output_img_ptr->imgPtr + 
                output_img_ptr->dx*(output_img_ptr->dy-2)*3;

    for (j=0; j<output_img_ptr->dx*3; j++)
    {
      *data_out = *data2_out;
      data_out++;
      data2_out++;
    }

    /*
    ** Now Process the W and E cols
    */
    data_out = output_img_ptr->imgPtr;
    data2_out = data_out + 3;
    for (i = 0; i<output_img_ptr->dy;i++)
    {
      *data_out = *data2_out;
      data_out++;
      data2_out++;
      *data_out = *data2_out;
      data_out++;
      data2_out++;
      *data_out = *data2_out;
      data_out++;
      data2_out++;
      data_out += (output_img_ptr->dx-1)*3;
      data2_out += (output_img_ptr->dx-1)*3;
    }

    data_out = output_img_ptr->imgPtr + (output_img_ptr->dx-1)*3;
    data2_out = output_img_ptr->imgPtr + (output_img_ptr->dx-2)*3;
    for (i = 0; i<output_img_ptr->dy;i++)
    {
      *data_out = *data2_out;
      data_out++;
      data2_out++;
      *data_out = *data2_out;
      data_out++;
      data2_out++;
      *data_out = *data2_out;
      data_out++;
      data2_out++;
      data_out += (output_img_ptr->dx-1)*3;
      data2_out += (output_img_ptr->dx-1)*3;
    }

  }

  MSG_LOW("ipl_convert_to_rgb888 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_to_rgb888 */


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
)
{
  uint8 cb=0,cr=0,luma1=0,luma2=0;
  uint16 i,j;
  uint32* data_out;
  uint16* data_in;
  int32 rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0,coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int16 ycbcr_convert[6] = {8, 25803, -3071, -7672, 30399, 12};
  uint16 width;
  uint16 in;
  uint8 rs,gs,bs;

  MSG_LOW("ipl_convert_to_rgb666 marker_0\n");

  if (!input_img_ptr   || !input_img_ptr->imgPtr || 
      !output_img_ptr  || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_to_rgb666 marker_200\n");
    return IPL_FAILURE;
  }
  
  data_out = (uint32*)output_img_ptr->imgPtr;
  data_in = (uint16*)input_img_ptr->imgPtr;
  width = (uint16)input_img_ptr->dx;

  /*
  **  Verify that input is either RGB565 or YCbCr 4:2:2
  */
  if (!((input_img_ptr->cFormat == IPL_YCbCr)
       || (input_img_ptr->cFormat == IPL_RGB565)))
  {
    MSG_LOW("ipl_convert_to_rgb666 marker_201\n");
    return IPL_FAILURE;
  }
  /*
  ** Verify that output is RGB666
  */
  if (output_img_ptr->cFormat != IPL_RGB666)
  {
    MSG_LOW("ipl_convert_to_rgb666 marker_202\n");
    return IPL_FAILURE;
  }
  /*
  ** Verify that input and output dimensions are identical
  */
  if (!((input_img_ptr->dx == output_img_ptr->dx) &&
      (input_img_ptr->dy == output_img_ptr->dy)))
  {
    MSG_LOW("ipl_convert_to_rgb666 marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_to_rgb666 marker_1\n");

  if (input_img_ptr->cFormat == IPL_YCbCr)
  {

    /*
    ** Convert from YCbCr 4:2:2 to RGB666
    */
    for(i = 0; i < input_img_ptr->dy; i++)
    {
      for(j = 0; j < input_img_ptr->dx; j=j+2)
      {
        /* We will do the conversion on 2 pixels at a time */
        /* This is Cb */
        cb = input_img_ptr->imgPtr[2*(j+i*width)];
        /* Next byte is luma of 1st pixel */
        luma1 = input_img_ptr->imgPtr[2*(j+i*width)+1];
        /* Next byte is Cr */
        cr = input_img_ptr->imgPtr[2*(j+i*width)+2];
        /* Next byte is luma of 2nd pixel */
        luma2 = input_img_ptr->imgPtr[2*(j+i*width)+3];
        rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
        gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
        bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;
        r = luma1 + (rc>>16);
        g = luma1 + (gc>>16);
        b = luma1 + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        data_out[j+(i*width)] = pack_rgb666(r,g,b);
        r = luma2 + (rc>>16);
        g = luma2 + (gc>>16);
        b = luma2 + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        data_out[j+(i*width)+1] = pack_rgb666(r,g,b);
      }
    }

  } else {

    /*
    ** Convert from RGB565 to RGB666
    */
    for(i = 0; i < input_img_ptr->dy; i++)
    {
      for(j = 0; j < input_img_ptr->dx; j++)
      {
        /*
        ** Read in pixel, unpack, and write out
        */
        in = data_in[j+i*width];
        rs = (uint8)((in&0xF800)>>8);
        gs = (uint8)((in&0x07E0)>>3);
        bs = (uint8)((in&0x001F)<<3);
        data_out[j+i*width] = pack_rgb666(rs,gs,bs);
      }
    }
  }

  MSG_LOW("ipl_convert_to_rgb666 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_to_rgb666 */



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
)
{
  uint8 cb=0,cr=0,luma1=0,luma2=0;
  uint16 i,j;
  uint16* data_out;
  uint16* data_in;
  int32 rc,gc,bc,r,g,b;
  /* ycbcr_convert is the conversion array of coeffs in Q14 Signed */
  /* In Q0,coeffs are 4.8828e-4 1.57489 -0.18744 -0.46826 1.8554 7.3242e-4 */
  int16 ycbcr_convert[6] = {8, 25803, -3071, -7672, 30399, 12};
  uint16 width;
  uint16 in;
  uint8 rs,gs,bs;

  MSG_LOW("ipl_convert_to_rgb444 marker_0\n");

  if (!input_img_ptr   || !input_img_ptr->imgPtr || 
      !output_img_ptr  || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_to_rgb444 marker_200\n");
    return IPL_FAILURE;
  }

  data_out = (uint16*)output_img_ptr->imgPtr;
  data_in = (uint16*)input_img_ptr->imgPtr;
  width = (uint16)input_img_ptr->dx;

  /*
  **  Verify that input is either RGB565 or YCbCr 4:2:2
  */
  if (!((input_img_ptr->cFormat == IPL_YCbCr)
        || (input_img_ptr->cFormat == IPL_RGB565)))
  {
    MSG_LOW("ipl_convert_to_rgb444 marker_201\n");
    return IPL_FAILURE;
  }
  /*
  ** Verify that output is RGB444
  */
  if (output_img_ptr->cFormat != IPL_RGB444)
  {
    MSG_LOW("ipl_convert_to_rgb444 marker_202\n");
    return IPL_FAILURE;
  }
  /*
  ** Verify that input and output dimensions are identical
  */
  if (!((input_img_ptr->dx == output_img_ptr->dx) &&
      (input_img_ptr->dy == output_img_ptr->dy)))
  {
    MSG_LOW("ipl_convert_to_rgb444 marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_to_rgb444 marker_1\n");

  if (input_img_ptr->cFormat == IPL_YCbCr)
  {
    /*
    ** Convert from YCbCr 4:2:2 to RGB444
    */
    for(i = 0; i < input_img_ptr->dy; i++)
    {
      for(j = 0; j < input_img_ptr->dx; j=j+2)
      {
        /* We will do the conversion on 2 pixels at a time */
        /* This is Cb */
        cb = input_img_ptr->imgPtr[2*(j+i*width)];
        /* Next byte is luma of 1st pixel */
        luma1 = input_img_ptr->imgPtr[2*(j+i*width)+1];
        /* Next byte is Cr */
        cr = input_img_ptr->imgPtr[2*(j+i*width)+2];
        /* Next byte is luma of 2nd pixel */
        luma2 = input_img_ptr->imgPtr[2*(j+i*width)+3];
        rc = (ycbcr_convert[0]*(cb-128) + ycbcr_convert[1]*(cr-128))*4+0x8000;
        gc = (ycbcr_convert[2]*(cb-128) + ycbcr_convert[3]*(cr-128))*4+0x8000;
        bc = (ycbcr_convert[4]*(cb-128) + ycbcr_convert[5]*(cr-128))*4+0x8000;
        r = luma1 + (rc>>16);
        g = luma1 + (gc>>16);
        b = luma1 + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        data_out[j+(i*width)] = pack_rgb444(r,g,b);
        r = luma2 + (rc>>16);
        g = luma2 + (gc>>16);
        b = luma2 + (bc>>16);
        r = CLIPIT(r);
        g = CLIPIT(g);
        b = CLIPIT(b);
        data_out[j+(i*width)+1] = pack_rgb444(r,g,b);
      }
    }

  } else {

    /*
    ** Convert from RGB565 to RGB444
    */
    for(i = 0; i < input_img_ptr->dy; i++)
    {
      for(j = 0; j < input_img_ptr->dx; j++)
      {
        /*
        ** Read in pixel, unpack, and write out
        */
        in = data_in[j+i*width];
        rs = (uint8)((in&0xF800)>>8);
        gs = (uint8)((in&0x07E0)>>3);
        bs = (uint8)((in&0x001F)<<3);
        data_out[j+i*width] = pack_rgb444(rs,gs,bs);
      }
    }
  }

  MSG_LOW("ipl_convert_to_rgb444 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_to_rgb444 */




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB8882YCbCr

DESCRIPTION
  This function does color conversion from RGB 888 to YCbCr 4:2:2
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
  01/08/02  Created

===========================================================================*/
extern ipl_status_type 
ipl_RGB8882YCbCr(
     unsigned char* data_in,
     unsigned char* data_out,
     short width, 
     short height)
{
  unsigned char r1,g1,b1,r2,g2,b2;
  unsigned short i,j;
  short w;
  int32 luma1,luma2,cb,cr;

  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_RGB8882YCBCr marker_0\n");

  if (!data_in || !data_out)
  {
    MSG_LOW("ipl_RGB8882YCBCr marker_200\n");
    return IPL_FAILURE;
  }
  w = width;

  if (w%2)
  {
    MSG_LOW("ipl_RGB8882YCBCr marker_201\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_RGB8882YCBCr marker_1\n");

  for(i = 0; i < height; i++){
    for(j = 0; j < w; j=j+2){
      /* We will do the conversion on 2 pixels at a time */
      /* Read in the RGB 16 for the 2 values  */
      r1 = *((unsigned char*)(data_in + (j+i*width)*3 ) );
      g1 = *((unsigned char*)(data_in + (j+i*width)*3 + 1) );
      b1 = *((unsigned char*)(data_in + (j+i*width)*3 + 2) );
      luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + ycbcr_convert[2]*b1)*4+0x8000;
      luma1 = (luma1>>16) + 16;
      luma1=CLIPIT(luma1);
      cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + ycbcr_convert[5]*b1)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);
      /* 2nd pixel */
      r2 = *((unsigned char*)(data_in + (j+i*width+1)*3 ) );
      g2 = *((unsigned char*)(data_in + (j+i*width+1)*3 + 1) );
      b2 = *((unsigned char*)(data_in + (j+i*width+1)*3 + 2) );
      luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + ycbcr_convert[2]*b2)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + ycbcr_convert[8]*b2)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      data_out[(j+(i*w))<<1] = (unsigned char)cb;
      data_out[((j+(i*w))<<1)+1] = (unsigned char)luma1;
      data_out[((j+(i*w))<<1)+2] = (unsigned char)cr;
      data_out[((j+(i*w))<<1)+3] = (unsigned char)luma2;
    }
  }

  MSG_LOW("ipl_RGB8882YCBCr marker_100\n");
  return IPL_SUCCESS;
}

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGB4442YCbCr

DESCRIPTION
  This function does color conversion from RGB 444 to YCbCr 4:2:2
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
  04/09/04  Created

===========================================================================*/
extern ipl_status_type ipl_RGB4442YCbCr(
      unsigned char* data_in,
      unsigned char* data_out,
      short width, 
      short height)
{
  unsigned char r1,g1,b1,r2,g2,b2;
  unsigned short i,j,out,out2;
  int32 luma1,luma2,cb,cr;

  /*
  **
  ** ycbcr_convert is the conversion array of coeffs in Q14 Signed
  ** In Q0, coeffs are .257 .504 .098 -.148 -.291 .439 .439 -.368 -.071
  **
  */
  short ycbcr_convert[9] = {4210,8257,1605,-2424,-4767,7192,7192,-6029,-1163};

  MSG_LOW("ipl_RGB4442YCBCr marker_0\n");



  if (!data_in || !data_out)
  {
    MSG_LOW("ipl_RGB4442YCBCr marker_200\n");
    return IPL_FAILURE;
  }
 
  MSG_LOW("ipl_RGB4442YCBCr marker_1\n");

  for(i = 0; i < height; i++){
    for(j = 0; j < width; j=j+2){
      /* We will do the conversion on 2 pixels at a time */
      /* Read in the RGB 16 for the 2 values  */
      out = *((unsigned short*)(data_in + (j+i*width)*2 ) );
      unpack_rgb444(out,&r1,&g1,&b1);
      luma1 = (ycbcr_convert[0]*r1 + ycbcr_convert[1]*g1 + ycbcr_convert[2]*b1)*4+0x8000;
      luma1 = (luma1>>16) + 16;
      luma1 = CLIPIT(luma1);
      cb = (ycbcr_convert[3]*r1 + ycbcr_convert[4]*g1 + ycbcr_convert[5]*b1)*4+0x8000;
      cb = (cb>>16) + 128;
      cb = CLIPIT(cb);
      /* 2nd pixel */
      out2 = *((unsigned short*)(data_in + (j+i*width+1)*2 ) );
      unpack_rgb444(out2,&r2,&g2,&b2);
      luma2 = (ycbcr_convert[0]*r2 + ycbcr_convert[1]*g2 + ycbcr_convert[2]*b2)*4+0x8000;
      luma2 = (luma2>>16) + 16;
      luma2 = CLIPIT(luma2);
      cr = (ycbcr_convert[6]*r2 + ycbcr_convert[7]*g2 + ycbcr_convert[8]*b2)*4+0x8000;
      cr = (cr>>16) + 128;
      cr = CLIPIT(cr);
      data_out[(j+(i*width))<<1] = (unsigned char)cb;
      data_out[((j+(i*width))<<1)+1] = (unsigned char)luma1;
      data_out[((j+(i*width))<<1)+2] = (unsigned char)cr;
      data_out[((j+(i*width))<<1)+3] = (unsigned char)luma2;
    }
  }

  MSG_LOW("ipl_RGB4442YCBCr marker_100\n");
  return IPL_SUCCESS;
}




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
)
{
  register uint8 y,cb,cr,y2;
  uint32 row,col;
  register uint8* data_out;
  register uint8* data2_out;
  uint8* y_ptr;
  uint8* yr2_ptr;
  uint8* c_ptr;
  int32 dest_index;

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422 marker_0\n");

  if (!input_img_ptr   || !input_img_ptr->imgPtr || 
      !output_img_ptr  || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422 marker_200\n");
    return IPL_FAILURE;
  }
  data_out = (uint8*)output_img_ptr->imgPtr;
  data2_out = (uint8*)((uint32)output_img_ptr->imgPtr + output_img_ptr->dx*2);
  y_ptr = input_img_ptr->imgPtr;
  yr2_ptr = input_img_ptr->imgPtr + input_img_ptr->dx;
  c_ptr = input_img_ptr->clrPtr;
  dest_index = output_img_ptr->dx*2;

  /*
  **  Verify Arguments
  */
  if (input_img_ptr->cFormat != IPL_YCbCr420_LINE_PK && 
      input_img_ptr->cFormat != IPL_YCrCb420_LINE_PK)
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422 marker_201\n");
    return IPL_FAILURE;
  }

  if (output_img_ptr->cFormat != IPL_YCbCr)
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422 marker_202\n");
    return IPL_FAILURE;
  }
  if ((input_img_ptr->dx != output_img_ptr->dx) || 
      (input_img_ptr->dy != output_img_ptr->dy))
  {
    MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422 marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422 marker_1\n");

  /* Now loop through the image once */
  for(row = output_img_ptr->dy; row; row=row-2)
  {
    for(col = output_img_ptr->dx; col; col=col-2)
    {
      /*
      ** Work on 4 pixels at a time
      */
      /*
      ** First 2 pixels
      */
      y = *y_ptr++;
      y2 = *y_ptr++;
      cb = *c_ptr++;
      cr = *c_ptr++;
      /*
      ** First Pixel and Second Pixel
      */
      *data_out = cb;
      data_out++;
      *data_out = y;
      data_out++;
      *data_out = cr;
      data_out++;
      *data_out = y2;
      data_out++;
      /*
      ** Third Pixel and Fourth Pixel
      */
      y = *yr2_ptr++;
      y2 = *yr2_ptr++;
      *data2_out = cb;
      data2_out++;
      *data2_out = y;
      data2_out++;
      *data2_out = cr;
      data2_out++;
      *data2_out = y2;
      data2_out++;
    }
    y_ptr += input_img_ptr->dx;
    yr2_ptr += input_img_ptr->dx;
    data_out += dest_index;
    data2_out += dest_index;
  }

  MSG_LOW("ipl_convert_ycbcr420lp_to_ycbcr422 marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr420lp_to_ycbcr422 */


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
)
{
  register uint8 cb,cr,cb2,cr2;
  uint32 row,col;
  register uint8* data1_in;
  register uint8* data2_in;
  uint8* data1_out;
  uint8* data2_out;
  uint8* c_ptr;
  int32 dest_index;
  uint32 clr;
  int oddHeight;

  MSG_LOW("ipl_convert_ycbcr422_to_ycbcr420lp marker_0\n");

  if (!input_img_ptr   || !input_img_ptr->imgPtr || 
      !output_img_ptr  || !output_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycbcr420lp marker_200\n");
    return IPL_FAILURE;
  }

  data1_in = input_img_ptr->imgPtr;
  data2_in = (uint8*)((uint32)input_img_ptr->imgPtr + input_img_ptr->dx*2);
  data1_out = output_img_ptr->imgPtr;
  data2_out = output_img_ptr->imgPtr + output_img_ptr->dx;
  c_ptr = output_img_ptr->clrPtr;
  dest_index = output_img_ptr->dx;


  oddHeight = (input_img_ptr->dy % 2)? 1 : 0; 
  if (oddHeight)
  {
    if ((input_img_ptr->dy + 1) != (output_img_ptr->dy)) 
    {
      MSG_LOW("ipl_convert_ycbcr422_to_ycbcr420lp marker_201\n");
      return IPL_FAILURE;
    }
  }

  /*
  **  Verify Arguments
  */
  if (input_img_ptr->cFormat != IPL_YCbCr)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycbcr420lp marker_202\n");
    return IPL_FAILURE;
  }

  if (output_img_ptr->cFormat != IPL_YCbCr420_LINE_PK &&
      output_img_ptr->cFormat != IPL_YCrCb420_LINE_PK)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycbcr420lp marker_203\n");
    return IPL_FAILURE;
  }

  if (input_img_ptr->dx != output_img_ptr->dx)
  {
    MSG_LOW("ipl_convert_ycbcr422_to_ycbcr420lp marker_204\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_ycbcr422_to_ycbcr420lp marker_1\n");

  /* Now loop through the image once */
  for(row = input_img_ptr->dy - oddHeight; row; row=row-2)
  {
    for(col = input_img_ptr->dx; col; col=col-2)
    {
      /*
      ** Work on 4 pixels at a time
      */
      /*
      ** First 2 pixels
      */
      cb = *data1_in;
      data1_in++;
      *data1_out = *data1_in;
      data1_out++;
      data1_in++;
      cr = *data1_in;
      data1_in++;
      *data1_out = *data1_in;
      data1_out++;
      data1_in++;

      cb2 = *data2_in;
      data2_in++;
      *data2_out = *data2_in;
      data2_out++;
      data2_in++;
      cr2 = *data2_in;
      data2_in++;
      *data2_out = *data2_in;
      data2_out++;
      data2_in++;

      /*
      ** Now calculate 1 Cb and 1 Cr
      */
      clr = cb + cb2;
      clr = clr>>1;
      *c_ptr = (uint8) clr;
      c_ptr++;
      clr = cr + cr2;
      clr = clr>>1;
      *c_ptr = (uint8) clr;
      c_ptr++;
    }
    data1_in += input_img_ptr->dx*2;
    data2_in += input_img_ptr->dx*2;
    data1_out += dest_index;
    data2_out += dest_index;
  }
  
  // if odd height, duplicate the last row
  if (oddHeight)
  {
    for(col = output_img_ptr->dx; col; col-=2)
    {
      *c_ptr++ = *data1_in++;                       // Cb
      *data2_out++ = *data1_out++ = *data1_in++;   // Luma 
      *c_ptr++ = *data1_in++;                       // Cr
      *data2_out++ = *data1_out++ = *data1_in++;   // Luma
    } 
  } 

  MSG_LOW("ipl_convert_ycbcr422_to_ycbcr420lp marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_convert_ycbcr422_to_ycbcr420lp */



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
)
{
  short int i, j;
  int index = 0;
  int cidx = 0;
  uint8 *rgb;

  MSG_LOW("ipl_RGB8882RGB565plt marker_0\n");

  if (!data_in || !palette || !data_out)
  {
    MSG_LOW("ipl_RGB8882RGB565plt marker_200\n");
    return IPL_FAILURE;
  }
  
  MSG_LOW("ipl_RGB8882RGB565plt marker_1\n");

  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      cidx = data_in[index];
      rgb = &palette[cidx * 3];
      data_out[index] = pack_rgb565(rgb[2], rgb[1], rgb[0]);
      index++;
    }
  }

  MSG_LOW("ipl_RGB8882RGB565plt marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_RGB8882RGB565plt */


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_RGBA8882RGB565plt

DESCRIPTION
  This function performs color conversion from RGB888 to RGB565 using a 
  RGBA pallete to do lookup. This function differs from ipl_RGBA8882RGB565plt
  in that the palette is RGBA and different endian order.

DEPENDENCIES
  None

ARGUMENTS IN
  data_in     input data
  palette     input RGBA palette
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
API_EXTERN ipl_status_type ipl_RGBA8882RGB565plt
(
  uint8* data_in, 
  uint8* palette, 
  uint16* data_out,
  int16 width, 
  int16 height
)
{
  short int i, j;
  int index = 0;
  int cidx = 0;
  uint32 *pal;
  uint32 rgb;
  unsigned char r, g, b;

  MSG_LOW("ipl_RGBA8882RGB565plt marker_0\n");

  if (!data_in || !palette || !data_out)
  {
    MSG_LOW("ipl_RGBA8882RGB565plt marker_200\n");
    return IPL_FAILURE;
  }
  
  pal = (uint32 *) palette;
  MSG_LOW("ipl_RGBA8882RGB565plt marker_1\n");

  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      cidx = data_in[index];

      rgb = pal[cidx];
      r = (unsigned char) ((rgb >> 16) & 0xFF);
      g = (unsigned char) ((rgb >> 8) & 0xFF);
      b = (unsigned char) (rgb & 0xFF);
      data_out[index] = pack_rgb565(r, g, b);

      index++;
    }
  }

  MSG_LOW("ipl_RGBA8882RGB565plt marker_100\n");

  return IPL_SUCCESS;
} /* End ipl_RGBA8882RGB565plt */




/*===========================================================================

FUNCTION ipl_convert_rgb565_to_ycbcr

DESCRIPTION
  This function is an optimized version to do convert an 
  RGB565 to image YCbCr 422 

  Input is assumed to be in rgb565 format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  crop          where and how much of input to convert and paste
                if NULL, as much as the input that can be is converted

ARGUMENTS OUT
  output_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
static ipl_status_type ipl_convert_rgb565_to_ycbcr
(
  ipl_image_type* i_img_ptr,    
  ipl_image_type* o_img_ptr
)
{
  register unsigned char r1,g1,b1;
  register uint16 out;
  uint32 row,col;
  uint16 *inputImgPtr;
  uint8 *outputImgPtr;
  int oddWidth;

  MSG_LOW("ipl_convert_rgb565_to_ycbcr marker_0\n");

  if (!i_img_ptr  || !i_img_ptr->imgPtr || 
      !o_img_ptr  || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_rgb565_to_ycbcr marker_200\n");
    return IPL_FAILURE;
  }
  
  inputImgPtr = (uint16*) i_img_ptr->imgPtr;
  outputImgPtr = o_img_ptr->imgPtr;

  /*------------------------------------------------------------------------
  *       Call the init routine. Mulitiple initializations does not hurt.
  *------------------------------------------------------------------------*/

  if (ipl2_init() != IPL_SUCCESS)
  {
    MSG_LOW("ipl_convert_rgb565_to_ycbcr marker_201\n");
    return IPL_FAILURE;
  }
  
  // if input is odd width, we will duplicate last column. 
  // Make sure we have room for it though
  oddWidth = (i_img_ptr->dx % 2)? 1 : 0; 
  if (oddWidth )
  {
    if ((i_img_ptr->dx + 1) != (o_img_ptr->dx))
    {
      MSG_LOW("ipl_convert_rgb565_to_ycbcr marker_202\n");
      return IPL_FAILURE;
    }
  }

  if (i_img_ptr->dy != o_img_ptr->dy) 
  {
    MSG_LOW("ipl_convert_rgb565_to_ycbcr marker_203\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_rgb565_to_ycbcr marker_1\n");

  for (row = i_img_ptr->dy; row; row--)
  {
    for (col = i_img_ptr->dx - oddWidth; col; col -= 2)
    {
      out = *inputImgPtr++;
      r1 = out >> 8 ;
      g1 = (out >> 3) & 0xff ;
      b1 = out & 0xff;
      // Write Cb
      *outputImgPtr++ = ipl2_rgb565ToCbR[r1] + ipl2_rgb565ToCbG[g1] 
        + ipl2_rgb565ToCbB[b1];
      // Write Y
      *outputImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];

      out = *inputImgPtr++;
      r1 = out >> 8;
      g1 = (out >> 3) & 0xff;
      b1 = out & 0xff;
      // Write Cr
      *outputImgPtr++ = ipl2_rgb565ToCrR[r1] + ipl2_rgb565ToCrG[g1] 
        + ipl2_rgb565ToCrB[b1];
      // Write Y 
      *outputImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];
    } 

    if (oddWidth)
    {
      inputImgPtr++;   
      outputImgPtr += 4;
    }
  } 

  /* fill in last row since we skipped it earlier */
  if (oddWidth)
  {
    inputImgPtr = ((uint16 *) i_img_ptr->imgPtr + i_img_ptr->dx - 1);
    outputImgPtr = o_img_ptr->imgPtr + 2*o_img_ptr->dx - 4;

    for (row = i_img_ptr->dy; row; row--)
    {
      out = *inputImgPtr;
      inputImgPtr += i_img_ptr->dx;

      r1 = out >> 8 ;
      g1 = (out >> 3) & 0xff ;
      b1 = out & 0xff;
      // Write Cb
      *outputImgPtr++ = ipl2_rgb565ToCbR[r1] + ipl2_rgb565ToCbG[g1] 
        + ipl2_rgb565ToCbB[b1];
      // Write Y
      *outputImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];

      // Write Cr
      *outputImgPtr++ = ipl2_rgb565ToCrR[r1] + ipl2_rgb565ToCrG[g1] 
        + ipl2_rgb565ToCrB[b1];
      // Write Y
      *outputImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];

      outputImgPtr += (o_img_ptr->dx*2 - 4);
    }
  }

  MSG_LOW("ipl_convert_rgb565_to_ycbcr marker_100\n");

  return (IPL_SUCCESS);
} 




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_ycrcb420mb_to_ycrcb420lp

DESCRIPTION
  This function converts YCrCb 4:2:0 MacroBlock to YCxCx 4:2:x line packed 

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
  ipl_image_type* in_img_ptr,    /* Pointer to the input image */
  ipl_image_type* out_img_ptr    /* Pointer to the output image */
)
{
  uint32 nmbx, nmby,mbx,mby,x,y,cjump,yjump,dx;
  register unsigned char *inImgPtr, *inImgPtr2, *inClrPtr;
  register unsigned char *outImgPtr, *outImgPtr2, *outClrPtr;
  unsigned char *outClrPtr2;

  MSG_LOW("ipl_convert_ycrcb420mb_to_ycrcb420lp marker_0\n");

  if (!in_img_ptr  || !in_img_ptr->imgPtr || 
      !out_img_ptr || !out_img_ptr->imgPtr)
  {
    MSG_LOW("ipl_convert_ycrcb420mb_to_ycrcb420lp marker_200\n");
    return IPL_FAILURE;
  }
  
  /*
  **  Verify Arguments
  */
  if (in_img_ptr->cFormat != IPL_YCrCb420_MB_PK)
  {
    MSG_LOW("ipl_convert_ycrcb420mb_to_ycrcb420lp marker_201\n");
    return IPL_FAILURE;
  }

  if ((out_img_ptr->cFormat != IPL_YCrCb420_LINE_PK) &&
      (out_img_ptr->cFormat != IPL_YCrCb422_LINE_PK) && 
      (out_img_ptr->cFormat != IPL_YCbCr420_LINE_PK) &&
      (out_img_ptr->cFormat != IPL_YCbCr422_LINE_PK))
  {
    MSG_LOW("ipl_convert_ycrcb420mb_to_ycrcb420lp marker_202\n");
    return IPL_FAILURE;
  }

  if ((in_img_ptr->dx != out_img_ptr->dx) || 
      (in_img_ptr->dy != out_img_ptr->dy))
  {
    MSG_LOW("ipl_convert_ycrcb420mb_to_ycrcb420lp marker_203\n");
    return IPL_FAILURE;
  }
  dx = out_img_ptr->dx;

  MSG_LOW("ipl_convert_ycrcb420mb_to_ycrcb420lp marker_1\n");

  if ((out_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) ||
      (out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK))
  {
    /* go to upper left of where we cut and where we are going to paste */
    dx = out_img_ptr->dx;

    nmbx = dx/16;
    nmby = in_img_ptr->dy/16;
    yjump = 2*dx-16;
    cjump = dx-16;

    inImgPtr = in_img_ptr->imgPtr;
    inImgPtr2 = in_img_ptr->imgPtr+16;
    inClrPtr = in_img_ptr->clrPtr;

    outImgPtr = out_img_ptr->imgPtr;
    outImgPtr2 = out_img_ptr->imgPtr+dx;
    outClrPtr = out_img_ptr->clrPtr;

    for(mby = 0; mby < nmby; mby++)
    {
      for(mbx = 0; mbx < nmbx; mbx++)
      {
        outImgPtr = out_img_ptr->imgPtr + mbx*16 + mby*(256*nmbx);
        outImgPtr2 = out_img_ptr->imgPtr + mbx*16 + mby*(256*nmbx) + dx;
        outClrPtr = out_img_ptr->clrPtr + mbx*16 + mby*(128*nmbx);

        /* two two rows at a time */
        for(y = 8; y; y--)
        {
          /* copy first row of ys */
          /* copy next row of ys */
          /* copy chroma over */
          for(x = 16; x ; x--)
          {
            *outImgPtr++ = *inImgPtr++;
            *outImgPtr2++ = *inImgPtr2++;
            *outClrPtr++ = *inClrPtr++;
          }

          inImgPtr += 16;
          inImgPtr2 += 16; 
          outImgPtr += yjump;
          outImgPtr2 += yjump;
          outClrPtr += cjump;
        }
      }
    }

  }
  else if ((out_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) ||
           (out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK))
  {
    dx = out_img_ptr->dx;

    nmbx = dx/16;
    nmby = in_img_ptr->dy/16;
    yjump = 2*dx-16;
    cjump = 2*dx-16;

    inImgPtr = in_img_ptr->imgPtr;
    inImgPtr2 = in_img_ptr->imgPtr+16;
    inClrPtr = in_img_ptr->clrPtr;

    outImgPtr = out_img_ptr->imgPtr;
    outImgPtr2 = out_img_ptr->imgPtr+dx;
    outClrPtr = out_img_ptr->clrPtr;
    outClrPtr2 = out_img_ptr->clrPtr+dx;

    for(mby = 0; mby < nmby; mby++)
    {
      for(mbx = 0; mbx < nmbx; mbx++)
      {
        outImgPtr = out_img_ptr->imgPtr + mbx*16 + mby*(256*nmbx);
        outImgPtr2 = out_img_ptr->imgPtr + mbx*16 + mby*(256*nmbx) + dx;
        outClrPtr = out_img_ptr->clrPtr + mbx*16 + mby*(256*nmbx);
        outClrPtr2 = out_img_ptr->clrPtr + mbx*16 + mby*(256*nmbx) + dx;

        /* two two rows at a time */
        for(y = 8; y; y--)
        {
          /* copy first row of ys */
          /* copy next row of ys */
          /* copy chroma over */
          for(x = 16; x ; x--)
          {
            *outImgPtr++ = *inImgPtr++;
            *outImgPtr2++ = *inImgPtr2++;

            *outClrPtr++ = *inClrPtr;
            *outClrPtr2++ = *inClrPtr++;
          }

          inImgPtr += 16;
          inImgPtr2 += 16; 

          outImgPtr += yjump;
          outImgPtr2 += yjump;
          outClrPtr += cjump;
          outClrPtr2 += cjump;
        }
      }
    }
  }
  else
  {
    MSG_LOW("ipl_convert_ycrcb420mb_to_ycrcb420lp marker_204\n");
    return IPL_FAILURE;
  }

  /* see if we need to swap Cr and Cb */
  if (in_img_ptr->cFormat == IPL_YCrCb420_MB_PK && 
      out_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)
  {
    ipl_convert_swap_chroma_order(out_img_ptr);
  }
  if (in_img_ptr->cFormat == IPL_YCrCb420_MB_PK && 
      out_img_ptr->cFormat == IPL_YCbCr422_LINE_PK)
  {
    ipl_convert_swap_chroma_order(out_img_ptr);
  }

  MSG_LOW("ipl_convert_ycrcb420mb_to_ycrcb420lp marker_100\n");
  return IPL_SUCCESS;
} 



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_convert_image

DESCRIPTION
  This function converts between image types

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
)
{
  ipl_status_type retval = IPL_FAILURE;

  MSG_LOW("ipl_convert_image marker_0\n");

  // we can do swapping chroma order inplace as long as user passes in 
  // output->cFormat, but does not allocate any memeory for it.
  if (!input_img_ptr || !input_img_ptr->imgPtr || !output_img_ptr)
  {
    MSG_LOW("ipl_convert_image marker_200\n");
    return IPL_FAILURE;
  }

  MSG_LOW("ipl_convert_image marker_1\n");

  if (input_img_ptr->cFormat == output_img_ptr->cFormat)
  {
    retval = ipl_copy_and_paste(input_img_ptr,output_img_ptr,NULL,NULL);
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr)
  {

    if (output_img_ptr->cFormat == IPL_RGB565)
      retval = ipl_crop_ycbcr_to_rgb(input_img_ptr,output_img_ptr,NULL,NULL);
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
      retval = ipl_convert_ycbcr422_to_ycrcb420lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) 
      retval = ipl_convert_ycbcr422_to_ycbcr420lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) 
      retval = ipl_convert_ycbcr422_to_ycrcb422lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr422_LINE_PK)
      retval = ipl_convert_ycbcr422_to_ycbcr422lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr444)
      retval = ipl_convert_ycbcr422_to_ycbcr444(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB444)
      retval = ipl_convert_to_rgb444(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB666)
      retval = ipl_convert_to_rgb666(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB888) 
      retval = ipl_convert_to_rgb888(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr444_PAD) 
      retval=ipl_convert_to_ycbcr444pad(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_HSV) 
      retval = ycbcr_to_hsv_normalized(input_img_ptr, output_img_ptr);
    else
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_RGB565)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr) 
      retval = ipl_convert_rgb565_to_ycbcr(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
      retval = ipl_convert_rgb565_to_ycrcb420lp(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) 
    {
      output_img_ptr->cFormat = IPL_YCrCb420_LINE_PK;
      retval = ipl_convert_rgb565_to_ycrcb420lp(input_img_ptr, output_img_ptr);
      output_img_ptr->cFormat = IPL_YCbCr420_LINE_PK;
      ipl_convert_swap_chroma_order(output_img_ptr);
    }
    else if (output_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) 
      retval = ipl_convert_rgb565_to_ycrcb422lp(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr422_LINE_PK) 
    {
      output_img_ptr->cFormat = IPL_YCrCb422_LINE_PK;
      retval = ipl_convert_rgb565_to_ycrcb422lp(input_img_ptr, output_img_ptr);
      output_img_ptr->cFormat = IPL_YCbCr422_LINE_PK;
      ipl_convert_swap_chroma_order(output_img_ptr);
    }
    else if (output_img_ptr->cFormat == IPL_YCbCr444) 
      retval = ipl_convert_rgb565_to_ycbcr444(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB444) 
      retval = ipl_convert_to_rgb444(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB666) 
      retval = ipl_convert_to_rgb666(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB888) 
      retval = ipl_convert_to_rgb888(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr444_PAD)
      retval = ipl_convert_to_ycbcr444pad(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB888_PAD)
      retval = ipl_convert_rgb888pad(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_LUMA_ONLY)
      retval = ipl_convert_to_luma_only(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_HSV) 
      retval = to_hsv_normalized(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_BAYER_BGGR) 
      retval = to_bayer(input_img_ptr, output_img_ptr);
    else 
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr) 
      retval = ipl_convert_ycrcb420lp_to_ycbcr422(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB565) 
      retval = ipl2_convert_ycrcb420lp_to_rgb565(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB888) 
      retval = ipl_convert_ycrcb420lp_to_rgb888(input_img_ptr,output_img_ptr);
    else if ((output_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK) || 
             (output_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK))
      retval = ipl_convert_ycbcr420lp_to_ycbcr420fp(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr444_PAD) 
      retval=ipl_convert_to_ycbcr444pad(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) 
    {
      if (output_img_ptr->imgPtr != NULL)
      {
        memcpy(output_img_ptr->imgPtr, input_img_ptr->imgPtr,
              input_img_ptr->dx*input_img_ptr->dy);
        memcpy(output_img_ptr->clrPtr, input_img_ptr->clrPtr,
              input_img_ptr->dx*input_img_ptr->dy/2);
        retval = ipl_convert_swap_chroma_order(output_img_ptr);
      }
      else
      {
        retval = ipl_convert_swap_chroma_order(input_img_ptr);
      }
    }
    else if (output_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) 
      retval=ipl_convert_ycbcr420lp_to_ycbcr422lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr422_LINE_PK) 
    {
      retval=ipl_convert_ycbcr420lp_to_ycbcr422lp(input_img_ptr,output_img_ptr);
      retval = ipl_convert_swap_chroma_order(output_img_ptr);
    }
    else if (output_img_ptr->cFormat == IPL_YCrCb444_LINE_PK) 
      retval=ipl_convert_ycbcr420lp_to_ycbcr444lp(input_img_ptr,output_img_ptr);
    else
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr) 
      retval = ipl_convert_ycbcr420lp_to_ycbcr422(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
    {
      if (output_img_ptr->imgPtr != NULL)
      {
        memcpy(output_img_ptr->imgPtr, input_img_ptr->imgPtr,
              input_img_ptr->dx*input_img_ptr->dy);
        memcpy(output_img_ptr->clrPtr, input_img_ptr->clrPtr,
              input_img_ptr->dx*input_img_ptr->dy/2);
        retval = ipl_convert_swap_chroma_order(output_img_ptr);
      }
      else
      {
        retval = ipl_convert_swap_chroma_order(input_img_ptr);
      }
    }
    else if (output_img_ptr->cFormat == IPL_YCbCr422_LINE_PK) 
      retval=ipl_convert_ycbcr420lp_to_ycbcr422lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) 
    {
      output_img_ptr->cFormat = IPL_YCbCr422_LINE_PK; 
      retval=ipl_convert_ycbcr420lp_to_ycbcr422lp(input_img_ptr,output_img_ptr);
      output_img_ptr->cFormat = IPL_YCrCb422_LINE_PK; 
      ipl_convert_swap_chroma_order(output_img_ptr);
    }
    else if (output_img_ptr->cFormat == IPL_YCbCr444_LINE_PK) 
      retval=ipl_convert_ycbcr420lp_to_ycbcr444lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB565) 
      retval =
        ipl_crop_ycbcr420lp_to_rgb565(input_img_ptr,output_img_ptr,NULL,NULL);
    else if (output_img_ptr->cFormat == IPL_RGB666) 
      retval = ipl_convert_ycbcr420lp_to_rgb666(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB444)
      retval = ipl_convert_ycbcr420lp_to_rgb444(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
    {
      memcpy(output_img_ptr->imgPtr, input_img_ptr->imgPtr,
             input_img_ptr->dx*input_img_ptr->dy);
      memcpy(output_img_ptr->clrPtr, input_img_ptr->clrPtr,
             input_img_ptr->dx*input_img_ptr->dy/2);
      retval = ipl_convert_swap_chroma_order(output_img_ptr);
    }
    else if ((output_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK) || 
             (output_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK))
      retval = ipl_convert_ycbcr420lp_to_ycbcr420fp(input_img_ptr, output_img_ptr);
    else
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr) 
      retval =ipl_convert_ycrcb422lp_to_ycbcr422(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB565) 
      retval = ipl2_convert_ycrcb422lp_to_rgb565(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB888) 
      retval = ipl_convert_ycrcb422lp_to_rgb888(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr422_LINE_PK) 
    {
      if (output_img_ptr->imgPtr != NULL)
      {
        memcpy(output_img_ptr->imgPtr, input_img_ptr->imgPtr,
               input_img_ptr->dx*input_img_ptr->dy);
        memcpy(output_img_ptr->clrPtr, input_img_ptr->clrPtr,
               input_img_ptr->dx*input_img_ptr->dy);
        retval = ipl_convert_swap_chroma_order(output_img_ptr);
      }
      else
      {
        retval = ipl_convert_swap_chroma_order(input_img_ptr);
      }
    }
    else
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr422_LINE_PK)
  {
    if      (output_img_ptr->cFormat == IPL_YCbCr) 
      retval =ipl_convert_ycbcr422lp_to_ycbcr422(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) 
      retval=ipl_convert_ycbcr422lp_to_ycbcr420lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
    {
      output_img_ptr->cFormat = IPL_YCbCr420_LINE_PK;
      retval=ipl_convert_ycbcr422lp_to_ycbcr420lp(input_img_ptr,output_img_ptr);
      output_img_ptr->cFormat = IPL_YCrCb420_LINE_PK;
      ipl_convert_swap_chroma_order(output_img_ptr);
    }
    else if (output_img_ptr->cFormat == IPL_RGB888) 
      retval = ipl_convert_ycbcr422lp_to_rgb888(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB565) 
      retval =
        ipl_crop_ycbcr422lp_to_rgb(input_img_ptr,output_img_ptr,NULL,NULL);
    else if (output_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) 
    {
      if (output_img_ptr->imgPtr != NULL)
      {
        memcpy(output_img_ptr->imgPtr, input_img_ptr->imgPtr,
              input_img_ptr->dx*input_img_ptr->dy);
        memcpy(output_img_ptr->clrPtr, input_img_ptr->clrPtr,
              input_img_ptr->dx*input_img_ptr->dy);
        retval = ipl_convert_swap_chroma_order(output_img_ptr);
      }
      else
      {
        retval = ipl_convert_swap_chroma_order(input_img_ptr);
      }
    }
    else
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr444_LINE_PK)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) 
      retval=ipl_convert_ycbcr444lp_to_ycbcr420lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
    {
      output_img_ptr->cFormat = IPL_YCbCr420_LINE_PK;
      retval=ipl_convert_ycbcr444lp_to_ycbcr420lp(input_img_ptr,output_img_ptr);
      output_img_ptr->cFormat = IPL_YCrCb420_LINE_PK;
      ipl_convert_swap_chroma_order(output_img_ptr);
    }
    else 
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr444)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr) 
      retval = ipl_convert_ycbcr444_to_ycbcr422(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr444_PAD) 
      retval=ipl_convert_to_ycbcr444pad(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB888) 
      retval = ipl_convert_to_rgb888(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
      retval=ipl_convert_ycbcr444_to_ycrcb420lp(input_img_ptr, output_img_ptr);
    else
      retval = IPL_FAILURE;
  }
  else if ((input_img_ptr->cFormat == IPL_YCrCb420_MB_PK) ||
           (input_img_ptr->cFormat == IPL_YCbCr420_MB_PK))
  {
    if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK)
      retval=ipl_convert_ycrcb420mb_to_ycrcb420lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK)
      retval=ipl_convert_ycrcb420mb_to_ycrcb420lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb422_LINE_PK)
      retval=ipl_convert_ycrcb420mb_to_ycrcb420lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr422_LINE_PK)
      retval=ipl_convert_ycrcb420mb_to_ycrcb420lp(input_img_ptr,output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr444_PAD)
      retval=ipl_convert_to_ycbcr444pad(input_img_ptr,output_img_ptr);
    else 
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_RGB444)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) 
      retval = ipl_convert_rgb444_to_ycbcr420lp(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
    {
      output_img_ptr->cFormat = IPL_YCbCr420_LINE_PK;
      retval = ipl_convert_rgb444_to_ycbcr420lp(input_img_ptr, output_img_ptr);
      output_img_ptr->cFormat = IPL_YCrCb420_LINE_PK;
      ipl_convert_swap_chroma_order(output_img_ptr);
    }
    else if (output_img_ptr->cFormat == IPL_YCbCr) 
    {
      retval = ipl_RGB4442YCbCr((unsigned char *) input_img_ptr->imgPtr, 
                                (unsigned char *) output_img_ptr->imgPtr,
                                (short) input_img_ptr->dx, 
                                (short) input_img_ptr->dy);
    }
    else 
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_RGB666)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) 
      retval = ipl_convert_rgb666_to_ycbcr420lp(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
    {
      output_img_ptr->cFormat = IPL_YCbCr420_LINE_PK;
      retval = ipl_convert_rgb666_to_ycbcr420lp(input_img_ptr, output_img_ptr);
      output_img_ptr->cFormat = IPL_YCrCb420_LINE_PK;
      ipl_convert_swap_chroma_order(output_img_ptr);
    }
    else 
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_RGB888)
  {
    if (output_img_ptr->cFormat == IPL_RGB565)
    {
      retval = ipl_RGB8882RGB565((uint8 *) input_img_ptr->imgPtr, 
                                 (uint16 *) output_img_ptr->imgPtr,
                                 (int16) input_img_ptr->dx, 
                                 (int16) input_img_ptr->dy);
    }
    else if (output_img_ptr->cFormat == IPL_YCbCr)
    {
      retval = ipl_convert_rgb888_to_ycbcr(input_img_ptr, output_img_ptr);
    }
    else if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
      retval = ipl_convert_rgb888_to_ycrcb420lp(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) 
      retval = ipl_convert_rgb888_to_ycrcb422lp(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB888_PAD)
      retval = ipl_convert_rgb888pad(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_YCbCr444)
      retval = ipl_convert_rgb888_to_ycbcr444(input_img_ptr, output_img_ptr);
    else 
      retval = IPL_FAILURE;
  }
  else if ((input_img_ptr->cFormat == IPL_H1V1MCU_CbCr) ||
           (input_img_ptr->cFormat == IPL_H1V2MCU_CbCr) ||
           (input_img_ptr->cFormat == IPL_H2V1MCU_CbCr) ||
           (input_img_ptr->cFormat == IPL_H2V2MCU_CbCr) ||
           (input_img_ptr->cFormat == IPL_MCU_GRAY)) 
  {
    if (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK) 
      retval = ipl_copy_and_paste(input_img_ptr, output_img_ptr, NULL, NULL);
    else if (output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) 
      retval = ipl_copy_and_paste(input_img_ptr, output_img_ptr, NULL, NULL);
    else if (output_img_ptr->cFormat == IPL_YCbCr422_LINE_PK) 
      retval = ipl_copy_and_paste(input_img_ptr, output_img_ptr, NULL, NULL);
    else if (output_img_ptr->cFormat == IPL_YCrCb422_LINE_PK) 
      retval = ipl_copy_and_paste(input_img_ptr, output_img_ptr, NULL, NULL);
    else if (output_img_ptr->cFormat == IPL_RGB565) 
      retval = ipl_copy_and_paste(input_img_ptr, output_img_ptr, NULL, NULL);
    else if (output_img_ptr->cFormat == IPL_RGB888) 
      retval = ipl_copy_and_paste(input_img_ptr, output_img_ptr, NULL, NULL);
    else
      retval = IPL_FAILURE;
  }
  else if ((input_img_ptr->cFormat == IPL_BAYER_GBRG) || 
           (input_img_ptr->cFormat == IPL_BAYER_BGGR) || 
           (input_img_ptr->cFormat == IPL_BAYER_GRBG) || 
           (input_img_ptr->cFormat == IPL_BAYER_RGGB))
  {
    if ((output_img_ptr->cFormat == IPL_RGB565) ||
        (output_img_ptr->cFormat == IPL_YCbCr))
      retval = ipl_downsize(input_img_ptr, output_img_ptr, NULL);
    else 
      retval = IPL_FAILURE;
  }
  else if ((input_img_ptr->cFormat == IPL_YCbCr420_FRAME_PK) || 
           (input_img_ptr->cFormat == IPL_YCrCb420_FRAME_PK))
  {
    if ((output_img_ptr->cFormat == IPL_YCbCr420_LINE_PK) ||
        (output_img_ptr->cFormat == IPL_YCrCb420_LINE_PK))
      retval = ipl_convert_ycbcr420fp_to_ycbcr420lp(input_img_ptr, output_img_ptr);
    else if (output_img_ptr->cFormat == IPL_RGB565)
      retval = ipl_convert_ycbcr420fp_to_rgb565(input_img_ptr, output_img_ptr);
    else 
      retval = IPL_FAILURE;
  }
  else if (input_img_ptr->cFormat == IPL_YCbCr444_PAD)
  {
    if (output_img_ptr->cFormat == IPL_RGB565) 
      retval = ipl_convert_from_ycbcr444pad(input_img_ptr, output_img_ptr);
  }
  else if (input_img_ptr->cFormat == IPL_RGB888_PAD)
  {
    if (output_img_ptr->cFormat == IPL_RGB888) 
      retval = ipl_convert_rgb888pad(input_img_ptr, output_img_ptr);
  }
  else if (input_img_ptr->cFormat == IPL_HSV)
  {
    if (output_img_ptr->cFormat == IPL_YCbCr) 
      retval = hsv_to_ycbcr_normalized(input_img_ptr, output_img_ptr);
    if (output_img_ptr->cFormat == IPL_RGB565) 
      retval = hsv_to_normalized(input_img_ptr, output_img_ptr);
  }
  else retval = IPL_FAILURE;

  MSG_LOW("ipl_convert_image marker_100\n");

  return retval;
}





/*===========================================================================

FUNCTION ipl2_convert_rgb565_to_ycbcr422lp

DESCRIPTION
  This function is an optimized version to do convert an 
  RGB565 to image YCbCr 422 line pack

  Input is assumed to be in rgb565 format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  crop          where and how much of input to convert and paste
                if NULL, as much as the input that can be is converted

ARGUMENTS OUT
  output_img_ptr points to the output image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
// put in attic?
extern ipl_status_type ipl2_convert_rgb565_to_ycbcr422lp
(
  ipl_image_type* i_img_ptr,    
  ipl_image_type* o_img_ptr,    
  ipl_rect_type* crop              /* where to put image, and how much */
)
{
  register unsigned char r1,g1,b1;
  register uint16 out;
  uint32 row,col;
  uint16 *inputImgPtr;
  uint8 *outputImgPtr;
  uint8 *chroma;
  uint32  inInc, outInc;
  ipl_rect_type mcrop;

  uint32 idx, idy, odx, ody, cx, cy, cdx, cdy;

  MSG_LOW("ipl2_convert_rgb565_to_ycbcr422lp marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl2_convert_rgb565_to_ycbcr422lp marker_200\n");
    return IPL_FAILURE;
  }
  
  inputImgPtr = (uint16*)i_img_ptr->imgPtr;
  outputImgPtr = o_img_ptr->imgPtr;
  chroma = o_img_ptr->clrPtr;


  /*------------------------------------------------------------------------
  *       Call the init routine. Mulitiple initializations does not hurt.
  *------------------------------------------------------------------------*/
  if (ipl2_init() != IPL_SUCCESS)
  {
    MSG_LOW("ipl2_convert_rgb565_to_ycbcr422lp marker_201\n");
    return IPL_FAILURE;
  }
  
  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;
  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;

  /* if no crop parameter, make one which includes entire images */
  if (!crop)
  {
    mcrop.x = 0;
    mcrop.y = 0;
    mcrop.dx = IPL_MIN(idx, odx);
    mcrop.dy = IPL_MIN(idy, ody);
    crop = &mcrop;
  }

  cx = crop->x;
  cy = crop->y;
  cdx = crop->dx;
  cdy = crop->dy;

  /* we want to crop on Cb data */
  if (cx % 2)
    cx--;

  /* make sure crop isnt too big, if so, make is smaller */
  if ((cx + cdx) > odx)
    cdx = odx - cx;

  if ((cy + cdy) > ody)
    cdy = ody - cy;

  /* currently we only handle even number of rgb pixels to convert */
  if (cdx % 2)
    cdx--;

  outputImgPtr += (cx + odx * cy);
  chroma += (2*(cx/2) + odx * cy);

  inInc =  (idx - cdx);
  outInc = (odx - cdx);

  MSG_LOW("ipl2_convert_rgb565_to_ycbcr422lp marker_1\n");

  for (row = cdy; row; row--)
  {
    for (col = cdx/2; col; col--)
    {
      out = *inputImgPtr++;
      r1 = out >> 8 ;
      g1 = (out >> 3) & 0xff ;
      b1 = out & 0xff;
      *outputImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];
      *chroma++ = ipl2_rgb565ToCbR[r1] + ipl2_rgb565ToCbG[g1] 
        + ipl2_rgb565ToCbB[b1];

      out = *inputImgPtr++;
      r1 = out >> 8;
      g1 = (out >> 3) & 0xff;
      b1 = out & 0xff;
      *outputImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];
      *chroma++ = ipl2_rgb565ToCrR[r1] + ipl2_rgb565ToCrG[g1] 
        + ipl2_rgb565ToCrB[b1];
    } 

    inputImgPtr += inInc;   /* half word increment */

    outputImgPtr += outInc; /* byte increment */
    chroma += outInc;
  }
  MSG_LOW("ipl2_convert_rgb565_to_ycbcr422lp marker_100\n");

  return (IPL_SUCCESS);
} 


/*===========================================================================

FUNCTION ipl2_convert_rgb565_to_ycbcr420lp

DESCRIPTION
  This function is an optimized version to do convert an 
  RGB565 to image YCbCr 420 line pack

  Input is assumed to be in rgb565 format

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr points to the input image
  crop          where and how much of input to convert and paste
                if NULL, as much as the input that can be is converted

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
)
{
  register unsigned char r1, g1, b1;
  register uint16 out;

  uint16 *inImgPtr;
  uint16 *inImgPtr2;
  unsigned char *outImgPtr;
  unsigned char *outImgPtr2;
  unsigned char *outClrPtr;
  ipl_rect_type mcrop;

  uint32 x,y,idx, idy, odx, ody, cx, cy, cdx, cdy;
  uint32 inInc;
  uint32 outInc;

  MSG_LOW("ipl2_convert_rgb565_to_ycbcr420lp marker_0\n");

  if (!i_img_ptr || !i_img_ptr->imgPtr || 
      !o_img_ptr || !o_img_ptr->imgPtr)
  {
    MSG_LOW("ipl2_convert_rgb565_to_ycbcr420lp marker_200\n");
    return IPL_FAILURE;
  }

  /*------------------------------------------------------------------------
  *       Call the init routine. Mulitiple initializations does not hurt.
  *------------------------------------------------------------------------*/
  if (ipl2_init() != IPL_SUCCESS)
  {
    MSG_LOW("ipl2_convert_rgb565_to_ycbcr420lp marker_201\n");
    return IPL_FAILURE;
  }
  
  idx = i_img_ptr->dx;
  idy = i_img_ptr->dy;
  odx = o_img_ptr->dx;
  ody = o_img_ptr->dy;

  /* if no crop parameter, make one which includes entire images */
  if (!crop)
  {
    mcrop.x = 0;
    mcrop.y = 0;
    mcrop.dx = IPL_MIN(idx, odx);
    mcrop.dy = IPL_MIN(idy, ody);
    crop = &mcrop;
  }

  cx = crop->x;
  cy = crop->y;
  cdx = crop->dx;
  cdy = crop->dy;

  /* we want to crop on Cb data */
  if (cx % 2)
    cx--;
  if (cy % 2)
    cy--;

  /* make sure crop isnt too big, if so, make is smaller */
  if ((cx + cdx) > odx)
    cdx = odx - cx;

  if ((cy + cdy) > ody)
    cdy = ody - cy;

  /* currently we only handle even number of rgb pixels to convert */
  if (cdx % 2)
    cdx--;

  /* setup our input pointers */
  inImgPtr = (uint16 *) i_img_ptr->imgPtr;         
  inImgPtr2 = (uint16 *) i_img_ptr->imgPtr + idx;

  outImgPtr = o_img_ptr->imgPtr + (cy * odx + cx);
  outImgPtr2 = outImgPtr + odx;
  outClrPtr = o_img_ptr->clrPtr + ((cy/2)*odx) + 2*(cx/2);

  /* our offsets */
  inInc =  (idx - cdx);
  outInc = (odx - cdx);

  MSG_LOW("ipl2_convert_rgb565_to_ycbcr420lp marker_1\n");
  /* do two rows at a time */
  for (y = cdy/2; y; y--) 
  {
    /* do two pixels at a time */
    for (x = cdx/2; x; x--)
    {
      /* We work on 4 pixels at a time
       *
       * RGB  RGB        Y1 Y3
       * RGB  RGB        Y2 Y4
       *
       *                 Cb Cr
       * */
      /* 1st column */
      out = *inImgPtr++;
      r1 = ( out ) >> 8 ;
      g1 = ( out >> 3) & 0xff ;
      b1 = out & 0xff;
      *outImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];

      out = *inImgPtr2++;
      r1 = ( out ) >> 8 ;
      g1 = ( out >> 3) & 0xff ;
      b1 = out & 0xff;
      *outImgPtr2++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];

      /* 2nd column */
      out = *inImgPtr++;
      r1 = ( out ) >> 8 ;
      g1 = ( out >> 3) & 0xff ;
      b1 = out & 0xff;
      *outImgPtr++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];

      out = *inImgPtr2++;
      r1 = ( out ) >> 8 ;
      g1 = ( out >> 3) & 0xff ;
      b1 = out & 0xff;
      *outImgPtr2++ = ipl2_rgb565ToYR[r1] + ipl2_rgb565ToYG[g1] 
        + ipl2_rgb565ToYB[b1];


      /* Use last guys rgb values to compute Cb and Cr;
       *
       * Cb Cr
       *
       */
      *outClrPtr++ = ipl2_rgb565ToCbR[r1] + ipl2_rgb565ToCbG[g1] +
        ipl2_rgb565ToCbB[b1];

      *outClrPtr++ = ipl2_rgb565ToCrR[r1] + ipl2_rgb565ToCrG[g1] + 
        ipl2_rgb565ToCrB[b1];
    } 

    /* each pointer goes 2 lines down */
    inImgPtr += (inInc + idx);
    inImgPtr2 += (inInc + idx);

    /* each pointer goes 2 lines down */
    outImgPtr += (outInc + odx);
    outImgPtr2 += (outInc + odx);
    outClrPtr += (outInc);
  } 

  MSG_LOW("ipl2_convert_rgb565_to_ycbcr420lp marker_100\n");
  return (IPL_SUCCESS);
}




/*lint -restore  */
