/**********************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __HDR_GB_LIB_H__
#define __HDR_GB_LIB_H__

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*============================================================================
  MACRO DEFINITION
============================================================================*/
/**
 * TRUE/FALSE
 **/

#if !defined(TRUE) || !defined(FALSE)
#define TRUE (1==1)
#define FALSE (!TRUE)
#endif

/** HDR_BIT_VAL:
 *
 * Sets the HDR configuration bit
 *
 * Returns value 1<<bit
 **/
#define HDR_BIT_VAL(name) (1 << name)

/** CONTRAST_CONTROL_Q4:
 *
 * Defines the contrast control parameter to control strength of HDR
 *
 * Returns the contrast control parameter to control strength of HDR
 **/
#define CONTRAST_CONTROL_Q4 24

/** EXPOSURE_RATIO:
 *
 * Defines the exposure ratio for tone map, can be used to control strength  HDR CONTRAST_CONTROL_Q4*EXPOSURE_RATIO determines strength
 *
 * Returns the exposure ratio parameter to control strength of HDR
 **/
#define EXPOSURE_RATIO 2

/** DEFINED_WEIGHT_FACTOR:
 *
 * Defines the weighing factor to modulate cdf
 *
 * Returns the weighing factor to modulate cdf
 **/
#define DEFINED_WEIGHT_FACTOR 32

/** CLAMP255:
 *
 * Defines the macro to clamp to 255
 *
 * Returns the clamped value to 255
 **/
#define CLAMP255(x) ( ((x) & 0xFFFFFF00) ? ((~(x) >> 31) & 0xFF) : (x))

/** Q_FACTOR:
 *
 * Defines the Q-factor for fixed point format
 *
 * Returns the Q factor for fixed point format
 **/
#define Q_FACTOR 15

/** Q_FACTOR_ROUNDOFF:
 *
 * Defines rounding factor for Q15 format
 *
 * Returns the rounding factor for Q15 format
 **/
#define Q_FACTOR_ROUNDOFF 16384


/** PIVOT_PWR_2:
 *
 * Defines the square root of (PIVOT_PTS+1)
 *
 * Returns the square root of (PIVOT_PTS+1)
 **/
#define PIVOT_PWR_2 3

/** PIVOT_PTS:
 *
 * Defines the number of pivot points
 *
 * Returns the number of pivot points
 **/
#define PIVOT_PTS ((1<<PIVOT_PWR_2)-1)

/** FRAME_1:
 *
 * Defines the macro for frame1
 *
 * Returns the macro for frame1
 **/
#define FRAME_1 1

/** FRAME_2:
 *
 * Defines the macro for frame2
 *
 * Returns the macro for frame2
 **/
#define FRAME_2 2

/** divisionLookupTable:
 *
 * Constant division lookup table (1<<20)/value, calue from 1 to 1024
 *
 **/
static const uint32_t divisionLookupTable[] =
{
  1048576,   524288,   349525,   262144,   209715,   174762,
   149796,   131072,   116508,   104857,    95325,
    87381,    80659,    74898,    69905,    65536,    61680,
    58254,    55188,    52428,    49932,    47662,
    45590,    43690,    41943,    40329,    38836,    37449,
    36157,    34952,    33825,    32768,    31775,
    30840,    29959,    29127,    28339,    27594,    26886,
    26214,    25575,    24966,    24385,    23831,
    23301,    22795,    22310,    21845,    21399,    20971,
    20560,    20164,    19784,    19418,    19065,
    18724,    18396,    18078,    17772,    17476,    17189,
    16912,    16644,    16384,    16131,    15887,
    15650,    15420,    15196,    14979,    14768,    14563,
    14364,    14169,    13981,    13797,    13617,
    13443,    13273,    13107,    12945,    12787,    12633,
    12483,    12336,    12192,    12052,    11915,
    11781,    11650,    11522,    11397,    11275,    11155,
    11037,    10922,    10810,    10699,    10591,
    10485,    10381,    10280,    10180,    10082,    9986,
     9892,     9799,     9709,     9619,     9532,
     9446,     9362,     9279,     9198,     9118,    9039,
     8962,     8886,     8811,     8738,     8665,
     8594,     8525,     8456,     8388,     8322,    8256,
     8192,     8128,     8065,     8004,     7943,
     7884,     7825,     7767,     7710,     7653,    7598,
     7543,     7489,     7436,     7384,     7332,
     7281,     7231,     7182,     7133,     7084,    7037,
     6990,     6944,     6898,     6853,     6808,
     6765,     6721,     6678,     6636,     6594,    6553,
     6512,     6472,     6432,     6393,     6355,
     6316,     6278,     6241,     6204,     6168,    6132,
     6096,     6061,     6026,     5991,     5957,
     5924,     5890,     5857,     5825,     5793,    5761,
     5729,     5698,     5667,     5637,     5607,
     5577,     5548,     5518,     5489,     5461,    5433,
     5405,     5377,     5349,     5322,     5295,
     5269,     5242,     5216,     5190,     5165,    5140,
     5115,     5090,     5065,     5041,     5017,
     4993,     4969,     4946,     4922,     4899,    4877,
     4854,     4832,     4809,     4788,     4766,
     4744,     4723,     4702,     4681,     4660,    4639,
     4619,     4599,     4578,     4559,     4539,
     4519,     4500,     4481,     4462,     4443,    4424,
     4405,     4387,     4369,     4350,     4332,
     4315,     4297,     4279,     4262,     4245,    4228,
     4211,     4194,     4177,     4161,     4144,
     4128,     4112,     4096,     4080,     4064,    4048,
     4032,     4017,     4002,     3986,     3971,
     3956,     3942,     3927,     3912,     3898,    3883,
     3869,     3855,     3840,     3826,     3813,
     3799,     3785,     3771,     3758,     3744,    3731,
     3718,     3705,     3692,     3679,     3666,
     3653,     3640,     3628,     3615,     3603,    3591,
     3578,     3566,     3554,     3542,     3530,
     3518,     3506,     3495,     3483,     3472,    3460,
     3449,     3437,     3426,     3415,     3404,
     3393,     3382,     3371,     3360,     3350,    3339,
     3328,     3318,     3307,     3297,     3287,
     3276,     3266,     3256,     3246,     3236,    3226,
     3216,     3206,     3196,     3187,     3177,
     3167,     3158,     3148,     3139,     3130,    3120,
     3111,     3102,     3093,     3084,     3075,
     3066,     3057,     3048,     3039,     3030,    3021,
     3013,     3004,     2995,     2987,     2978,
     2970,     2962,     2953,     2945,     2937,    2928,
     2920,     2912,     2904,     2896,     2888,
     2880,     2872,     2864,     2857,     2849,    2841,
     2833,     2826,     2818,     2811,     2803,
     2796,     2788,     2781,     2774,     2766,    2759,
     2752,     2744,     2737,     2730,     2723,
     2716,     2709,     2702,     2695,     2688,    2681,
     2674,     2668,     2661,     2654,     2647,
     2641,     2634,     2628,     2621,     2614,    2608,
     2601,     2595,     2589,     2582,     2576,
     2570,     2563,     2557,     2551,     2545,    2538,
     2532,     2526,     2520,     2514,     2508,
     2502,     2496,     2490,     2484,     2478,    2473,
     2467,     2461,     2455,     2449,     2444,
     2438,     2432,     2427,     2421,     2416,    2410,
     2404,     2399,     2394,     2388,     2383,
     2377,     2372,     2366,     2361,     2356,    2351,
     2345,     2340,     2335,     2330,     2325,
     2319,     2314,     2309,     2304,     2299,    2294,
     2289,     2284,     2279,     2274,     2269,
     2264,     2259,     2255,     2250,     2245,    2240,
     2235,     2231,     2226,     2221,     2216,
     2212,     2207,     2202,     2198,     2193,    2189,
     2184,     2179,     2175,     2170,     2166,
     2162,     2157,     2153,     2148,     2144,    2139,
     2135,     2131,     2126,     2122,     2118,
     2114,     2109,     2105,     2101,     2097,    2092,
     2088,     2084,     2080,     2076,     2072,
     2068,     2064,     2060,     2056,     2052,    2048,
     2044,     2040,     2036,     2032,     2028,
     2024,     2020,     2016,     2012,     2008,    2004,
     2001,     1997,     1993,     1989,     1985,
     1982,     1978,     1974,     1971,     1967,    1963,
     1959,     1956,     1952,     1949,     1945,
     1941,     1938,     1934,     1931,     1927,    1923,
     1920,     1916,     1913,     1909,     1906,
     1903,     1899,     1896,     1892,     1889,    1885,
     1882,     1879,     1875,     1872,     1869,
     1865,     1862,     1859,     1855,     1852,    1849,
     1846,     1842,     1839,     1836,     1833,
     1829,     1826,     1823,     1820,     1817,    1814,
     1811,     1807,     1804,     1801,     1798,
     1795,     1792,     1789,     1786,     1783,    1780,
     1777,     1774,     1771,     1768,     1765,
     1762,     1759,     1756,     1753,     1750,    1747,
     1744,     1741,     1738,     1736,     1733,
     1730,     1727,     1724,     1721,     1718,    1716,
     1713,     1710,     1707,     1705,     1702,
     1699,     1696,     1693,     1691,     1688,    1685,
     1683,     1680,     1677,     1675,     1672,
     1669,     1667,     1664,     1661,     1659,    1656,
     1653,     1651,     1648,     1646,     1643,
     1640,     1638,     1635,     1633,     1630,    1628,
     1625,     1623,     1620,     1618,     1615,
     1613,     1610,     1608,     1605,     1603,    1600,
     1598,     1596,     1593,     1591,     1588,
     1586,     1583,     1581,     1579,     1576,    1574,
     1572,     1569,     1567,     1565,     1562,
     1560,     1558,     1555,     1553,     1551,    1548,
     1546,     1544,     1542,     1539,     1537,
     1535,     1533,     1530,     1528,     1526,    1524,
     1521,     1519,     1517,     1515,     1513,
     1510,     1508,     1506,     1504,     1502,    1500,
     1497,     1495,     1493,     1491,     1489,
     1487,     1485,     1483,     1481,     1478,    1476,
     1474,     1472,     1470,     1468,     1466,
     1464,     1462,     1460,     1458,     1456,    1454,
     1452,     1450,     1448,     1446,     1444,
     1442,     1440,     1438,     1436,     1434,    1432,
     1430,     1428,     1426,     1424,     1422,
     1420,     1418,     1416,     1415,     1413,    1411,
     1409,     1407,     1405,     1403,     1401,
     1399,     1398,     1396,     1394,     1392,    1390,
     1388,     1387,     1385,     1383,     1381,
     1379,     1377,     1376,     1374,     1372,    1370,
     1368,     1367,     1365,     1363,     1361,
     1360,     1358,     1356,     1354,     1353,    1351,
     1349,     1347,     1346,     1344,     1342,
     1340,     1339,     1337,     1335,     1334,    1332,
     1330,     1328,     1327,     1325,     1323,
     1322,     1320,     1318,     1317,     1315,    1314,
     1312,     1310,     1309,     1307,     1305,
     1304,     1302,     1300,     1299,     1297,    1296,
     1294,     1292,     1291,     1289,     1288,
     1286,     1285,     1283,     1281,     1280,    1278,
     1277,     1275,     1274,     1272,     1271,
     1269,     1267,     1266,     1264,     1263,    1261,
     1260,     1258,     1257,     1255,     1254,
     1252,     1251,     1249,     1248,     1246,    1245,
     1243,     1242,     1240,     1239,     1237,
     1236,     1235,     1233,     1232,     1230,    1229,
     1227,     1226,     1224,     1223,     1222,
     1220,     1219,     1217,     1216,     1215,    1213,
     1212,     1210,     1209,     1208,     1206,
     1205,     1203,     1202,     1201,     1199,    1198,
     1197,     1195,     1194,     1192,     1191,
     1190,     1188,     1187,     1186,     1184,    1183,
     1182,     1180,     1179,     1178,     1176,
     1175,     1174,     1172,     1171,     1170,    1168,
     1167,     1166,     1165,     1163,     1162,
     1161,     1159,     1158,     1157,     1156,    1154,
     1153,     1152,     1151,     1149,     1148,
     1147,     1145,     1144,     1143,     1142,    1140,
     1139,     1138,     1137,     1136,     1134,
     1133,     1132,     1131,     1129,     1128,    1127,
     1126,     1125,     1123,     1122,     1121,
     1120,     1119,     1117,     1116,     1115,    1114,
     1113,     1111,     1110,     1109,     1108,
     1107,     1106,     1104,     1103,     1102,    1101,
     1100,     1099,     1097,     1096,     1095,
     1094,     1093,     1092,     1091,     1089,    1088,
     1087,     1086,     1085,     1084,     1083,
     1082,     1081,     1079,     1078,     1077,    1076,
     1075,     1074,     1073,     1072,     1071,
     1069,     1068,     1067,     1066,     1065,    1064,
     1063,     1062,     1061,     1060,     1059,
     1058,     1057,     1055,     1054,     1053,    1052,
     1051,     1050,     1049,     1048,     1047,
     1046,     1045,     1044,     1043,     1042,    1041,
     1040,     1039,     1038,     1037,     1036,
     1035,     1034,     1033,     1032,     1031,    1030,
     1029,     1028,     1027,     1026,     1025,
     1024
};

/*============================================================================
  Type DECLARATIONS
============================================================================*/

/** hdr_return_t
 *    @HDR_NO_MEMORY -2
 *    @HDR_ERROR -1
 *    @HDR_SUCESS 0
 *
 *  hdr return status
 **/

typedef enum {
  HDR_NO_MEMORY = -2,
  HDR_ERROR,
  HDR_SUCESS,
} hdr_return_t;

/** subsample_format_type
 *    @HDR_H2V2
 *    @HDR_H2V1
 *    @HDR_H1V2
 *    @HDR_H1V1
 *    @HDR_SUBSAMPLE_MAX
 *
 *  Only H2V2 is supported
 *  subsample_format_type
 **/

typedef enum {
  HDR_H2V2 = 0,
  HDR_H2V1 = 1,
  HDR_H1V2 = 2,
  HDR_H1V1 = 3,
  HDR_SUBSAMPLE_MAX
}subsample_format_type;


/** hdr_gamma_tbl_type_t
 *    @GAMMA_TBL_ALL
 *    @GAMMA_TBL_R
 *    @GAMMA_TBL_G
 *    @GAMMA_TBL_B
 *  Gamma table for color component, if same table is used GAMMA_TBL_ALL can be used to send everything
 *
 *  hdr_gamma_tbl_type_t
 **/

typedef enum {
  GAMMA_TBL_ALL = 0,
  GAMMA_TBL_R,
  GAMMA_TBL_G,
  GAMMA_TBL_B,
} hdr_gamma_tbl_type_t;


/** hdr_gamma_table_struct_t
 *    @gamma_t - gamma table type
 *    @entry
 *    @gamma_tbl
 *  Gamma table structure
 *
 *  hdr_gamma_table_struct_t
 **/

typedef struct {
  hdr_gamma_tbl_type_t gamma_t;
  uint32_t entry;
  void *gamma_tbl;
} hdr_gamma_table_struct_t;


/** hdr_param_t
 *    @mCalculatedExposureRatioG
 *    @mpRedInverseGammatable
 *    @mpGreenInverseGammatable
 *    @mpBlueInverseGammatable
 *    @mpRedNewGammatable
 *    @mpGreenNewGammatable
 *    @mpBlueNewGammatable
 *  Gamma parameter structure
 *
 *  hdr_param_t
 **/
typedef struct {
  uint32_t mCalculatedExposureRatioG;
  uint32_t *mpRedInverseGammatable;
  uint32_t *mpGreenInverseGammatable;
  uint32_t *mpBlueInverseGammatable;
  uint32_t *mpRedNewGammatable;
  uint32_t *mpGreenNewGammatable;
  uint32_t *mpBlueNewGammatable;
} hdr_param_t;

/** hdr_chroma_order_t
 *    @YCRCB
 *    @YCBCR
 *  Chroma order
 *
 *  hdr_chroma_order_t
 **/
typedef enum {
  YCRCB,
  YCBCR
} hdr_chroma_order_t;

/** hdr_pixel_component_type_t
 *    @Y
 *    @CR
 *    @CB
 *  pixel color component
 *
 *  hdr_pixel_component_type_t
 **/
typedef enum {
  Y,
  CR,
  CB
} hdr_pixel_component_type_t;

/** hdr_projection_t
 *    @pH1
 *    @pH2
 *    @pH3
 *    @pH4
 *    @pH5
 *    @pH6
 *    @pH7
 *    @pH8
 *    @pV1
 *    @pV2
 *    @pV3
 *    @pV4
 *    @pV5
 *    @pV6
 *    @pV7
 *    @pV8
 *  Horizontal and Vertical Projections
 *
 *  hdr_projection_t
 **/
typedef struct {
  uint32_t *pH1;
  uint32_t *pH2;
  uint32_t *pH3;
  uint32_t *pH4;
  uint32_t *pH5;
  uint32_t *pH6;
  uint32_t *pH7;
  uint32_t *pH8;
  uint32_t *pV1;
  uint32_t *pV2;
  uint32_t *pV3;
  uint32_t *pV4;
  uint32_t *pV5;
  uint32_t *pV6;
  uint32_t *pV7;
  uint32_t *pV8;
}hdr_projection_t;

/** hdr_motion_vector_t
 *    @vert[] - vertical vectors
 *    @hori[] - horizontal vectors
 *  Motion vector structure
 *
 *  hdr_motion_vector_t
 **/
typedef struct
{
  int32_t vert[(PIVOT_PTS+2)*(PIVOT_PTS+2)];
  int32_t hori[(PIVOT_PTS+2)*(PIVOT_PTS+2)];
}hdr_motion_vector_t;

/** crop_dim_t
 *    @x - starting x
 *    @y - starting y
 *    @dx - Crop width
 *    @dy - Crop height
 *
 *  crop_dim_t
 **/
typedef struct {
  int32_t x;
  int32_t y;
  int32_t dx;
  int32_t dy;
}crop_dim_t;

/** hdr_config_t
 *    @pHdrBuffer1Y - 2x Y, Need to be filled in by caller, Final HDR Image Y will be here
 *    @pHdrBuffer1C - 2x C, Need to be filled in by caller, Final HDR Image C will be here
 *    @pHdrBuffer1R - Internal use for lib
 *    @pHdrBuffer1G - Internal use for lib
 *    @pHdrBuffer1B - Internal use for lib
 *    @pHdrBuffer2Y - 0.5x Y, Need to be filled in by caller
 *    @pHdrBuffer2C - 0.5x C, Need to be filled in by caller
 *    @pHdrBuffer2R - Internal use for lib
 *    @pHdrBuffer2G - Internal use for lib
 *    @pHdrBuffer2B - Internal use for lib
 *    @pHdrBuffer3Y - x Y, Need to be filled in by caller
 *    @pHdrBuffer3C - x C, Need to be filled in by caller
 *    @pMotionMask  - Internal use for lib
 *    @pRefImgIndexMask - Internal use for lib
 *    @pHdrBuffer2RC - Internal use for lib
 *    @pHdrBuffer2GC - Internal use for lib
 *    @pHdrBuffer2BC - Internal use for lib
 *    @imageWidth - Image width including stride, filled by caller
 *    @imageHeight - Image height, filled by caller
 *    @subSampleFormat - Subsample format, only H2V2 is supported, passed by caller
 *    @thumbMainIndicator - Thumbnail/Main image indicator, passed by caller
 *    @maxLag - Maximum search distance, passed by caller
 *    @calculatedExposureRatioG - Calculated exposure ratio
 *    @pRedGammaInvTable - New red inverse table, passed by caller
 *    @pGreenGammaInvTable - New green inverse table , passed by caller
 *    @pBlueGammaInvTable - New blue inverse table, passed by caller
 *    @pRedGammaTable - New red gamma table , passed by caller
 *    @pGreenGammaTable - New green gamma table , passed by caller
 *    @pBlueGammaTable - New blue gamma table, passed by caller
 *    @frameid - Internal use for lib
 *    @pProjection - Internal use for lib
 *    @pCorrectedMv - Internal use for lib
 *    @pProjection1 - Internal use for lib
 *    @pProjection2 - Internal use for lib
 *    @pOrigMv - Internal use for lib
 *    @pMask - Internal use for lib
 *    @pMarker - Internal use for lib
 *    @pVoidHistogramArray - Internal use for lib
 *    @startingLine - Internal use for lib
 *    @endingLine - Internal use for lib
 *    @startingRange - Internal use for lib
 *    @endingRange - Internal use for lib
 *    @chromaOrder - Chroma order, passed by caller
 *    @pixelComponent - Internal use for lib
 *    @pHomographyLH - Internal use for lib
 *    @minI - Internal use for lib
 *    @maxI - Internal use for lib
 *    @minJ - Internal use for lib
 *    @maxJ - Internal use for lib
 *    @cropdimension - Algorithm fills in for the caller,  crop dimensions
 *    @contrastControl - contrast control parameter tunable
 *    @chromaSat_wgt - chroma desaturation control overall
 *    @chromaSat_clamp - chroma desaturation control bright
 *    @chromaSat_shift - chroma desaturation control dark
 *    @LUT_256 - Internal use for lib
 *    @LUT_1024 - Internal use for lib
 *    @pGammaStruct - Gamma R 64 entry 16 bit
 *    @pGammaStructG - Gamma G 64 entry 16 bit
 *    @pGammaStructB - Gamma B 64 entry 16 bit
 *   HDR Configuration structure hdr_config_t
 **/
typedef struct {
  void  *pHdrBuffer1Y;
  void  *pHdrBuffer1C;
  void  *pHdrBuffer1R;
  void  *pHdrBuffer1G;
  void  *pHdrBuffer1B;
  void  *pHdrBuffer2Y;
  void  *pHdrBuffer2C;
  void  *pHdrBuffer2R;
  void  *pHdrBuffer2G;
  void  *pHdrBuffer2B;
  void  *pHdrBuffer3Y;
  void  *pHdrBuffer3C;
  uint8_t *pMotionMask;
  uint8_t *pRefImgIndexMask;
  void  *pHdrBuffer2RC;
  void  *pHdrBuffer2GC;
  void  *pHdrBuffer2BC;
  uint32_t imageWidth;
  uint32_t imageHeight;
  subsample_format_type subSampleFormat;
  uint32_t thumbMainIndicator;
  uint32_t maxLag;
  uint32_t calculatedExposureRatioG;
  uint32_t *pRedGammaInvTable;
  uint32_t *pGreenGammaInvTable;
  uint32_t *pBlueGammaInvTable;
  uint32_t *pRedGammaTable;
  uint32_t *pGreenGammaTable;
  uint32_t *pBlueGammaTable;
  uint8_t frameid;
  hdr_projection_t *pProjection;
  hdr_motion_vector_t *pCorrectedMv;
  hdr_projection_t *pProjection1;
  hdr_projection_t *pProjection2;
  hdr_motion_vector_t *pOrigMv;
  uint32_t segmentOutputHeight;
  uint8_t *pMask;
  uint8_t *pMarker;
  void *pVoidHistogramArray;
  uint32_t startingLine;
  uint32_t endingLine;
  uint32_t startingRange;
  uint32_t endingRange;
  hdr_chroma_order_t chromaOrder;
  hdr_pixel_component_type_t pixelComponent;
  float *pHomographyLH;
  int32_t minI;
  int32_t maxI;
  int32_t minJ;
  int32_t maxJ;
  crop_dim_t cropdimension;
  uint32_t contrastControl;
  float chromaSat_wgt;
  uint8_t chromaSat_clamp;
  uint8_t chromaSat_shift;
  int32_t LUT_256[256];
  uint32_t LUT_1024[1024];
  hdr_gamma_table_struct_t *pGammaStruct;
  hdr_gamma_table_struct_t *pGammaStructG;
  hdr_gamma_table_struct_t *pGammaStructB;
} hdr_config_t;


/** hdrTwoFrameCore:
 *    @pIn: Pointer to the image configuration struture (pIn).
 *    @pReturnStatus: Return status from HDR
 * Entry point for 2 frame HDR algorithm
 *
 * Returns NULL
 **/

void hdrTwoFrameCore (hdr_config_t *pIn, hdr_return_t *pReturnStatus);

/** hdrCalculateInverseGamma:
 *    @pGammaTableStruct: Gamma table structure
 *    @inverseGammatable: Output inverse gamma table
 * Function to calculate and set inverse gamma tables
 *
 * Returns success/failure
 **/

int hdrCalculateInverseGamma(hdr_gamma_table_struct_t *pGammaTableStruct,uint32_t * inverseGammatable);

/** hdrCalculateInverseGamma:
 *    @pGammaTableStruct: Gamma table structure
 *    @newGammatable: Output new gamma table
 * Function to calculate and set new gamma tables
 *
 * Returns NULL
 **/

void hdrCalculateNewGammaTable(hdr_gamma_table_struct_t *pGammaTableStruct,uint32_t * newGammatable);

/** hdrChromaUpsampleVert:
 *    @pChromaNear: Near chroma line
 *    @pChromaFar: Far chroma line
 *    @pChromaUpsample:  chroma upsampled line
 *    @length: Length
 * Function to upsample chroma vertically
 *
 * Returns NULL
 **/

void hdrChromaUpsampleVert(uint8_t *pChromaNear,
  uint8_t *pChromaFar,
  uint8_t *pChromaUpsample,
  uint32_t length);

#ifdef MM_CAMERA_NEON_ENABLED

/** hdrChromaUpsampleVert:
 *    @pChroma: Near chroma line
 *    @pChromaUpsample:  chroma upsampled line
 *    @length: Length
 * Function to upsample chroma horizontally
 *
 * Returns NULL
 **/
void hdrChromaUpsampleHori(uint8_t *pChroma,
  uint8_t *pChromaUpsample,
  uint32_t length);

/** hdrChromaUpsample2d:
 *    @pChromaNear: Near chroma line
 *    @pChromaFar: Far chroma line
 *    @pChromaUpsample:  chroma upsampled line in 2 directions
 *    @length: Length
 * Function to upsample chroma in both directions
 *
 * Returns NULL
 **/

void hdrChromaUpsample2d(uint8_t *pChromaNear,
  uint8_t *pChromaFar,
  uint8_t *pChromaUpsample,
  uint32_t length);
#endif

/** hdrUpsampleScale2d:
 *    @pInput: input
 *    @pOutput: output
 *    @ipwidth: input width
 *    @ipheight: input height
 *    @opwidth: output width
 *    @opheight: output height
 *    @startingLine:  Starting line to start upscale
 *    @endingLine: ending line of segment
 * Function to upscale in both directions
 *
 * Returns hdr return status
 **/

hdr_return_t hdrUpsampleScale2d(uint8_t *pInput,
  uint8_t *pOutput,
  uint32_t ipwidth,
  uint32_t ipheight,
  uint32_t opwidth,
  uint32_t opheight,
  uint32_t startingLine,
  uint32_t endingLine);

/** hdrDownsampleScale16:
 *    @pInput: input
 *    @pOutput: output
 *    @ipwidth: input width
 * Function to downscale by 4 in both directions
 *
 * Returns NULL
 **/

void hdrDownsampleScale16(uint8_t *pInput,
  uint8_t *pOutput,
  uint32_t ipwidth);


/** hdrInvGammaGreenScale:
 *    @pRed: Pointer to input red
 *    @pGreen: Pointer to input Green, output Green will also be there in same place
 *    @pBlue: Pointer to input Blue
 *    @pMask: Output scale mask
 *    @pRedGammaInverseTable: Input Red inverse gamma table
 *    @pGreenGammaInverseTable: Input Green inverse gamma table
 *    @pBlueGammaInverseTable: Input Blue inverse gamma table
 *    @length - width
 *  Compute inverse Gamma and output Green component
 *  Also determine and output scale map. This function is called for frame1
 *
 * Returns NULL
 **/

void hdrInvGammaGreenScale(uint16_t  *pRed,
  uint16_t *pGreen,
  uint16_t *pBlue,
  uint8_t  *pMask,
  uint32_t *pRedGammaInverseTable,
  uint32_t *pGreenGammaInverseTable,
  uint32_t *pBlueGammaInverseTable,
  uint32_t  length);


/** hdrInvGammaGreen:
 *    @pGreen: Pointer to input Green, output Green will also be there in same place
 *    @pGreenGammaInverseTable: Input Green inverse gamma table
 *    @length - width
 *  Compute inverse Gamma and output Green component,This function is called for frame2
 *
 * Returns NULL
 **/

void hdrInvGammaGreen(uint16_t *pGreen,
  uint32_t *pGreenGammaInverseTable,
  uint32_t  length);


/** hdrInvGammaRGB:
 *    @pRed: Input and output red
 *    @PGreen:Input and Output green
 *    @pBlue:Input and Output blue
 *    @pRedGammaInverseTable:Inverse gamma table for red
 *    @pGreenGammaInverseTable:Inverse gamma table for green
 *    @pBlueGammaInverseTable:Inverse gamma table for blue
 *    @length - width
 *    @frameid - frame id
 *    @inputRatio - ratio for frame2
 *  Compute inverse Gamma and output Red,Green,Blue component. This function is called for frame 1 and frame 2
 *
 * Returns NULL
 **/

void hdrInvGammaRGB(uint16_t *pRed,
  uint16_t *pGreen,
  uint16_t *pBlue,
  uint32_t *pRedGammaInverseTable,
  uint32_t *pGreenGammaInverseTable,
  uint32_t *pBlueGammaInverseTable,
  uint32_t  length,
  uint32_t frameid,
  uint32_t inputRatio);

#ifdef MM_CAMERA_NEON_ENABLED

/** hdrYuvtorgb:
 *    @pY: Input 8 bit Y
 *    @pCrCb:Input 8 bit CrCb
 *    @pRed:Output Red
 *    @pGreen:Output Green
 *    @pBlue:Output Blue
 *    @length - width
 *    @crcbindicator - crcb order
 *  Convert 8 bit YUV to 8 bit RGB, extend to 16 bit
 *
 * Returns NULL
 **/

void hdrYuvtorgb         (uint8_t  *pY,
  uint8_t  *pCrCb,
  uint16_t *pRed,
  uint16_t *pGreen,
  uint16_t *pBlue,
  uint32_t  length,
  uint32_t crcbindicator);

/** hdrYuvtog:
 *    @pY: Input 8 bit Y
 *    @pCrCb:Input 8 bit CrCb
 *    @pGreen:Output Green
 *    @length - width
 *    @crcbindicator - crcb order
 *  Convert 8 bit YUV to 8 bit Green, extend to 16 bit
 *
 * Returns NULL
 **/

void hdrYuvtog         (uint8_t  *pY,
  uint8_t  *pCrCb,
  uint16_t *pGreen,
  uint32_t  length,
  uint32_t crcbindicator);
#endif

/** hdrGammaConvert:
 *    @pRedGammaTable: Red gamma table
 *    @pGreenGammaTable:Green gamma table
 *    @pBlueGammaTable:Blue gamma table
 *    @pInputImageR: Input/OutputR
 *    @pInputImageG:Input/OutputG
 *    @pInputImageB:Input/Output B
 *    @imageHeight - height
 *    @imageWidth - width
 *  Does gamma correction for R,G,B
 *
 * Returns NULL
 **/

void hdrGammaConvert(uint32_t *pRedGammaTable,
  uint32_t *pGreenGammaTable,
  uint32_t *pBlueGammaTable,
  uint16_t *pInputImageR,
  uint16_t *pInputImageG,
  uint16_t *pInputImageB,
  uint32_t imageHeight,
  uint32_t imageWidth);

#ifdef MM_CAMERA_NEON_ENABLED
/*===========================================================================

Function           :  hdrColorConvertH2V2

Description        : This function does color conversion RGB->YCbCR420.

Input parameter(s) :
pInputImageR
pInputImageG
pInputImageB
imageheight
imagewidth

Output parameter(s): pOutputImageY
pOutputImageC

Return Value       : None

Side Effects       : None

=========================================================================== */

/** hdrGammaConvert:
 *    @pOutputImageY: Red gamma table
 *    @pOutputImageC:Green gamma table
 *    @pInputImageR: InputR
 *    @pInputImageG:InputG
 *    @pInputImageB:InputB
 *    @imageHeight - height
 *    @imageWidth - width
 *    @chromorder - chroma order
 *  Color converts RGB 16 bit to YUV 16 bit
 *
 * Returns NULL
 **/

void hdrColorConvertH2V2(uint16_t *pOutputImageY,
  int16_t *pOutputImageC,
  uint16_t *pInputImageR,
  uint16_t *pInputImageG,
  uint16_t *pInputImageB,
  uint32_t imageHeight,
  uint32_t imageWidth,
  uint32_t crcbindicator);
#endif


/** hdrReconstruct:
 *    @pMask: Red gamma table
 *    @pMarker:Green gamma table
 *    @iHeight - height
 *    @iWidth - width
 *  Reconstructs scale mask
 *
 * Returns return status
 **/

hdr_return_t hdrReconstruct(uint8_t *pMask,
  uint8_t *pMarker,
  uint32_t iHeight,
  uint32_t iWidth);

/*===========================================================================

Function            : hdrHistogramProcess

Description         : Histogram process
1) Divide Y image into 64 blocks and Compute histogram of each block
2) Intra-filtering of each histogram block
3) Contrast limiting of each histogram block
4) Compute CDF of each histogram block
5) Inter-filtering between CDFs of neighboring histogram blocks

Input parameter(s)  : Image config structure,(pIn)
pVoidHistogramArray
Image height,(imageHeight)
Image Width, (imageWidth)

Output parameter(s) : pIn->pHdrBuffer1Y - Y HDR output
pIn->pHdrBuffer1C - CbCr HDR output

Return Value        : hdrreturnstatus

Side Effects        : None

=========================================================================== */

/** hdrHistogramProcess:
 *    @pIn - image config structure
 *    @pVoidHistogramArray - histogram array
 *    @imageHeight - height
 *    @imageWidth - width
 *  Histogram process
 * 1) Divide Y image into 64 blocks and Compute histogram of each block
 * 2) Intra-filtering of each histogram block
 * 3) Contrast limiting of each histogram block
 * 4) Compute CDF of each histogram block
 * 5) Inter-filtering between CDFs of neighboring histogram blocks
 *
 * Returns return status
 **/

hdr_return_t hdrHistogramProcess   (hdr_config_t *pIn,
  void * pVoidHistogramArray,
  uint32_t imageHeight,
  uint32_t imageWidth);


/** hdrTonemapIntraFilterHistogram:
 *    @pHistLocal - Input histogram
 *    @pTmpHist - Filtered histogram
 *  Function does intra filtering of histogram
 *
 * Returns void
 **/

void hdrTonemapIntraFilterHistogram  (uint32_t *pHistLocal,
  uint32_t *pTmpHist);


/** hdrTonemapIntraFilterHistogram:
 *    @pHistLocal - Input and output normalized histogram
 *  Normalize the histogram block
 *
 * Returns void
 **/

void hdrTonemapHistogramNorm (uint32_t *pHistLocal);

/** hdrTonemapContrastControl:
 *    @pHistLocal - Input and output normalized histogram
 *    @pHistTemp1 - scratch
 *    @pHistTemp2 - scratch
 *    @pContrastFunction - contrast function formodulation
 *  This function does contrast limiting. Modulates the given
 *  histogram with the contrast function . After that it computes CDF
 * Returns void
 **/

void hdrTonemapContrastControl  (uint32_t *pHistLocal,
  uint32_t *pHistTemp1,
  uint32_t *pHistTemp2,
  uint32_t *pContrastFunction);

/** hdrAverageHistogramThreeBlocks:
 *    @pHist1 - histip(i,j)
 *    @pHist2 - histip(i-1,j) or histip(i,j-1)
 *    @pHist3 - histip(i+1,j) or histip(i,j+1)
 *    @pHistOut - output histogram block
 *  Calculate the average of histogram block  from the
 *   neighbouring 2 blocks.This is applicable for all histogram blocks
 *   in the 4 edges except the ones at the 4 corners
 * Returns void
 **/

void hdrAverageHistogramThreeBlocks(uint32_t *pHist1,
  uint32_t *pHist2,
  uint32_t *pHist3,
  uint32_t *pHistOut);

/** hdrAverageHistogramFiveBlocks:
 *    @pHist1 - histip(i,j)
 *    @pHist2 - histip(i-1,j)
 *    @pHist3 - histip(i+1,j)
 *    @pHist4 - histip(i,j-1)
 *    @pHist5 - histip(i,j+1)
 *    @pHistOut - output histogram block
 *    Calculate the average of histogram block  from the
 *    neighbouring 4 blocks
 * Returns void
 **/

void hdrAverageHistogramFiveBlocks (uint32_t *pHist1,
  uint32_t *pHist2,
  uint32_t *pHist3,
  uint32_t *pHist4,
  uint32_t *pHist5,
  uint32_t *pHistOut);

/** hdrTwoFrameGetRatio:
 *    @pIn - Image configuration structure
 *    @pInputRatioG - Green ratio
 *    This function calculates green ratio between frames
 * Returns void
 **/

void hdrTwoFrameGetRatio(hdr_config_t *pIn,
  uint32_t *pInputRatioG);


#ifdef MM_CAMERA_NEON_ENABLED

/** hdrTwoFrameInterpolateMv:
 *    @dc - starting value
 *    @diffPerStep - step size
 *    @ size - size
 *    @ pOutVec - output vector
 *    Interpolates MV
 * Returns void
 **/

void hdrTwoFrameInterpolateMv(int32_t dc,
  int32_t diffPerStep,
  uint32_t size,
  int32_t *pOutVec);

/** hdrTwoFrameInterpolateMvRounded:
 *    @dc - starting value
 *    @diffPerStep - step size
 *    @ size - size
 *    @ pOutVec - output vector
 *    Interpolate motion vectors each line and round and shift by 20
 * Returns void
 **/

void hdrTwoFrameInterpolateMvRounded(int32_t dc,
  int32_t diffPerStep,
  uint32_t size,
  int32_t *pOutVec);

#endif

/** hdrTwoFrameMotionCorrection:
 *    @pIn - Image configuration structure
 *    Interpolates and find motion vector of each pixel and
 * motion compensates the second frame. Also combines frame1 and frame2 based on scale.
 * All these are done in RGB domain and converted back to YUV. Works on a piece by piece basis
 * Returns hdr return status
 **/

hdr_return_t hdrTwoFrameMotionCorrection(hdr_config_t *pIn);

#ifdef MM_CAMERA_NEON_ENABLED


/** hdrTwoFrameCombine:
 *    @pScaleCopy - scale mask
 *    @pImageBuffer1R - Frame 1 red,green and blue
 *    @pImageBuffer1G
 *    @pImageBuffer1B
 *    @pImageBuffer2R - Frame 2 red,green and blue
 *    @pImageBuffer2G
 *    @pImageBuffer2B
 *    @length - number of pixels
 *    Combines frame 1 and frame 2using scale mask
 *    Returns NULL
 *
 **/

void hdrTwoFrameCombine(uint8_t *pScaleCopy,
  uint16_t *pImageBuffer1R,
  uint16_t *pImageBuffer1G,
  uint16_t *pImageBuffer1B,
  uint16_t *pImageBuffer2R,
  uint16_t *pImageBuffer2G,
  uint16_t *pImageBuffer2B,
  uint32_t length);

#endif

/** hdrSqrt32:
 *    @inputNumber - input number
 *    Determines square root of 64 bit number
 *    Returns squareroot
 **/

uint32_t hdrSqrt32(uint64_t inputNumber);

/** hdrTwoFrameTonemap:
 *    @pIn -image confugration structure
 *    performs tone mapping for 2 frames
 * 1) Divide image into 64 blocks and Compute histogram of each block
 * 2) Intra-filtering of each histogram block
 * 3) Contrast limiting of each histogram block and
 * Compute CDF of each histogram block
 * 4) Inter-filtering between CDFs of neighboring histogram blocks
 * 5) Adaptive histogram equalization using bilinear interpolation
 *    Returns return status
 **/

hdr_return_t hdrTwoFrameTonemap(hdr_config_t *pIn);

/** hdrTwoFrameTonemap:
 *    @pInputImageY -Y
 *    @pVoidHistogramArray - histogram output
 *    @imageHeight - image Height
 *    @imageWidth - image Width
 *    @ startingLine- staring segment Line
 *    @engingLine - enging Segment Line
 *    This function computes histogram for 2 frame HDR
 *    Returns Null
 **/

void hdrTwoFrameHistogram (uint16_t *pInputImageY,
  void * pVoidHistogramArray,
  uint32_t imageHeight,
  uint32_t imageWidth,
  uint32_t startingLine,
  uint32_t endingLine);

/** hdrTwoFrameEqualize:
 *    @pIn - image configuration structure
 *    This function does adaptive histogram equalization for 2 frame HDR
 *    Returns hdr return status
 **/

hdr_return_t hdrTwoFrameEqualize (hdr_config_t *pIn);

#ifdef MM_CAMERA_NEON_ENABLED

/** hdrTwoFrameChromaProcess:
 *    @pchromain - Input chroma pixel value
 *    @pChromaOut - output chroma
 *    @pChromaScale - chroma scale map
 *    @length - image Width
 *    Generates chroma output based on input chroma pixel and chroma scale
 *    Returns NULL
 **/

void hdrTwoFrameChromaProcess(int16_t *pChromaIn,
  uint8_t *pChromaOut,
  uint16_t *pChromaScale,
  uint32_t length);

#endif

/** hdrTwoColorGammaInverseMaskH2V2:
 *    @pInputImageY - Input Y
 *    @pInputImageC - InputC
 *    @pRedGammaInverseTable - Red inverse gamma table
 *    @pGreenGammaInverseTable - Green inverse gamma table
 *    @pBlueGammaInverseTable - Blue inverse gamma table
 *    @pOutputMask - Output scale mask
 *    @imageHeight - height
 *    @imageWidth - width
 *    @frameid - frame 1 or frame2
 *    @crcbindicator - chroma order
 *    This function does color conversion Ycbcr->Green and gamma inverse for H2V2 and returns green.
 *     For frame 1 it outputs the scale mask
 *    Returns hdr return status
 **/

hdr_return_t hdrTwoColorGammaInverseMaskH2V2(uint8_t *pInputImageY,
  uint8_t *pInputImageC,
  uint32_t *pRedGammaInverseTable,
  uint32_t *pGreenGammaInverseTable,
  uint32_t *pBlueGammaInverseTable,
  uint8_t *pOutputMask,
  uint16_t *pOutputImageG,
  uint32_t imageHeight,
  uint32_t imageWidth,
  uint32_t frameid,
  uint32_t crcbindicator);

/*===========================================================================

Function           :  hdrTwoColorGammaInverseH2V2

Description        : This function does color conversion Ycbcr->RGB and gamma inverse for H2V2 and returns red blue green.
For frame 2 it multiplies with input green ratio

Input parameter(s) :  pInputImageY
pInputImageC
redgammainversetable
pGreenGammaInverseTable
bluegammainversetable
imageheight
imagewidth
frameid
inputratio

Output parameter(s):  pOutputImageR
pOutputImageG
pOutputImageB

Return Value       : hdr_return_t

Side Effects       : None

=========================================================================== */

/** hdrTwoColorGammaInverseH2V2:
 *    @pInputImageY - Input Y
 *    @pInputImageC - InputC
 *    @pRedGammaInverseTable - Red inverse gamma table
 *    @pGreenGammaInverseTable - Green inverse gamma table
 *    @pBlueGammaInverseTable - Blue inverse gamma table
 *    @pInputImageR - Output R
 *    @pInputImageG - Output G
 *    @pInputImageB - Output B
 *    @imageHeight - height
 *    @imageWidth - width
 *    @frameid - frame 1 or frame2
 *    @inputratio - ratio for frame 2
 *    @crcbindicator - chroma order
 *    This function does color conversion Ycbcr->RGB and gamma inverse for H2V2 and returns red blue green.
 *     For frame 2 it multiplies with input green ratio
 *    Returns hdr return status
 **/

hdr_return_t hdrTwoColorGammaInverseH2V2(uint8_t *pInputImageY,
  uint8_t *pInputImageC,
  uint32_t *pRedGammaInverseTable,
  uint32_t *pGreenGammaInverseTable,
  uint32_t *pBlueGammaInverseTable,
  uint16_t *pInputImageR,
  uint16_t *pInputImageG,
  uint16_t *pInputImageB,
  uint32_t imageHeight,
  uint32_t imageWidth,
  uint32_t frameid,
  uint32_t inputratio,
  uint32_t crcbindicator);


/** Pxl_interpol_bilinear:
 *    @pScaleCopy - scale copy
 *    @imWidth - width
 *    @numOfRowInTempBuff - Number of rows in circular buffer
 *    @tempBuffCurrentRow - Current row
 *    @pOneRowV - Vertical mv for the row
 *    @pOneRowH - Horizontal mv for the row
 *    @imHeight - height
 *    @i - vertical index in 16 bit increments
 *    @k - vertical index 0-15
 *    @pTempBuffStartG - Input Green
 *    @pTempBuffStartR - Input Red
 *    @pTempBuffStartB - Input Blue
 *    @pImageBuffer2G - Output motion compensated Green
 *    @pImageBuffer2R - Output motion compensated Red
 *    @pImageBuffer2B - Output motion compensated blue
 *    @minI - min vert shift
 *    @maxI - min vert shift
 *    @minJ - min hori shift
 *    @maxJ - max hori shift
 *    Bilinear interpolation for the mvs,  and motion comepnsation
 *    Returns NULL
 **/

void Pxl_interpol_bilinear (uint8_t *pScaleCopy,
                            int32_t imWidth,
                            int32_t numOfRowInTempBuff,
                            int32_t tempBuffCurrentRow,
                            int32_t *pOneRowV,
                            int32_t *pOneRowH,
                            int32_t imHeight,
                            uint32_t i,
                            uint32_t k,
                            uint16_t *pTempBuffStartG,
                            uint16_t *pTempBuffStartR,
                            uint16_t *pTempBuffStartB,
                            uint16_t *pImageBuffer2G,
                            uint16_t *pImageBuffer2R,
                            uint16_t *pImageBuffer2B,
                            int32_t minI,
                            int32_t maxI,
                            int32_t minJ,
                            int32_t maxJ);

/*===========================================================================

   Function            : hdrSingleFramePreprocess

   Description         : Pre-process single frame
   1) Subtract 128 from CbCr
   2) Scale down CbCr if luma pixel is less than threshold

   Input parameter(s)  : Pointer to the input image Y,(pInputImageY)
   Pointer to the input image CbCr,(pInputImageC)
   chromaHeight,(chromaHeight)
   chroma Width, (chromaWidth)

   Output parameter(s) : Processed Chroma (pInputImageC)

   Return Value        : None

   Side Effects        : None

   =========================================================================== */
   void hdrSingleFramePreprocess(uint8_t *pInputImageY,
     uint8_t *pInputImageC,
     uint32_t chromaHeight,
     uint32_t chromaWidth,
     uint32_t xSkipFactor,
     uint32_t ySkipFactor);

/*===========================================================================
Function           : hdrSingleFrameHistogram

Description        : This function computes histogram for 1 frame HDR

Input parameter(s) : Image config structure,(pIn)
imageHeight
imageWidth

Output parameter(s): histogramArray (pVoidHistogramArray)

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrSingleFrameHistogram (hdr_config_t *pIn,
  void * pVoidHistogramArray,
  uint32_t imageHeight,
  uint32_t imageWidth);

/*===========================================================================
Function           : hdrSingleFrameEqualize

Description        : This function does adaptive histogram equalization for 1 frame HDR

Input parameter(s) : Image config structure,(pIn)

Output parameter(s): pIn->pHdrBuffer1Y - Y HDR output
pIn->pHdrBuffer1C - CbCr HDR output

Return Value       : hdrreturnstatus

Side Effects       : None

=========================================================================== */

hdr_return_t hdrSingleFrameEqualize (hdr_config_t *pIn);

/*===========================================================================

Function           : *

Description        : Determine projections for first frame

Input parameter(s) :
pImage
image width

Output parameter(s):
pH
pV1
pV2
pV3
pV4
pV5
pV6
pV7
pV8

Return Value       : New value of pimage

Side Effects       : None

=========================================================================== */
uint16_t * hdrTwoFrameCalculateProjectionsFirstFrame(uint16_t *pImage,
  uint32_t *pH,
  uint32_t *pV1,
  uint32_t *pV2,
  uint32_t *pV3,
  uint32_t *pV4,
  uint32_t *pV5,
  uint32_t *pV6,
  uint32_t *pV7,
  uint32_t *pV8,
  uint32_t imageWidth);

/*===========================================================================

Function           : hdrTwoFrameCalculateProjectionsSecondFrame

Description        : Determine projections for second frame

Input parameter(s) :
pImage
image width
inputratio

Output parameter(s):
pH
pV1
pV2
pV3
pV4
pV5
pV6
pV7
pV8

Return Value       : New value of pimage

Side Effects       : None

=========================================================================== */
uint16_t * hdrTwoFrameCalculateProjectionsSecondFrame(uint16_t *pImage,
  uint32_t *pH,
  uint32_t *pV1,
  uint32_t *pV2,
  uint32_t *pV3,
  uint32_t *pV4,
  uint32_t *pV5,
  uint32_t *pV6,
  uint32_t *pV7,
  uint32_t *pV8,
  uint32_t imageWidth,
  uint32_t inputRatio);

/*===========================================================================

Function           : hdrTwoFrameNormalizeProjections

Description        : Normalize projections

Input parameter(s) : Input projections
Projection length

Output parameter(s): Output normalized projections

Return Value       : None

Side Effects       : None

=========================================================================== */
void hdrTwoFrameNormalizeProjections(uint32_t *pInputProj,
  int64_t * pOutputProj,
  uint32_t projLen);

#endif //__HDR_GB_LIB_H__
