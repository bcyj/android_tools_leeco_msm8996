#ifndef HDR_H
#define HDR_H

/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                               *
**********************************************************************/

#define HDR_PROFILING

/*============================================================================
INCLUDE FILES
============================================================================*/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <camera_dbg.h>

#if(FRAME_PROC_HDR_DEBUG)
  #include <utils/Log.h>
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-HDR"
  #define CDBG_HDR(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_HDR(fmt, args...) do{}while(0)
#endif
#define CDBG_ERROR(fmt, args...) LOGE(fmt, ##args)

/*============================================================================
DEFINITIONS and CONSTANTS
============================================================================*/

#define HDR_BIT_VAL(name) (1 << name)
#define CONTRAST_CONTROL_Q4 24
#define EXPOSURE_RATIO 1
#define DEFINED_WEIGHT_FACTOR 32
#define CAMERA_WAVELET_DENOISING_DATA_SIZE 24
#define MAX_LEVEL 4

#define FILTER_LENGTH 5
#define FILTER_HALF_LENGTH 2

#define CLAMP255(x) ( ((x) & 0xFFFFFF00) ? ((~(x) >> 31) & 0xFF) : (x))

#define parameter_q15_factor 15
#define parameter_q15_roundoff (1 << (parameter_q15_factor-1))

#define EP_LUMA   204   // (uint32)(255 * 0.8  + 0.5)
#define EP_CHROMA 510   // (uint32)(255 * 2 + 0.5)

#define WEIGHT_LUMA 6554 // (uint32)(0.2*(1<<parameter_q15_factor)+0.5)
#define WEIGHT_CHROMA 0  // (uint32)(0.0*(1<<parameter_q15_factor)+0.5)

#define parameter_q20_roundoff (1<< (20-1))
#define parameter_q21_roundoff (1<< (21-1))
#define NUMBER_OF_LEVELS 4
#define Q_FACTOR 15
#define Q_FACTOR_ROUNDOFF 16384

/*============================================================================
  MACRO DEFINITION
============================================================================*/

#if !defined ABS
  #define  ABS(x) ((x)>0 ? (x) : -(x))
#endif

#if !defined MAX
  #define  MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif

#if !defined MIN
  #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif

#define FRAME_1 1
#define FRAME_2 2

static uint32_t divisionLookupTable[]=
{
  1048576 ,    524288 ,    349525 ,    262144 ,    209715 ,    174762 ,    149796 ,    131072 ,    116508 ,    104857 ,    95325  ,
  87381 ,    80659  ,    74898  ,    69905  ,    65536  ,    61680  ,    58254  ,    55188  ,    52428  ,    49932  ,    47662  ,
  45590 ,    43690  ,    41943  ,    40329  ,    38836  ,    37449  ,    36157  ,    34952  ,    33825  ,    32768  ,    31775  ,
  30840 ,    29959  ,    29127  ,    28339  ,    27594  ,    26886  ,    26214  ,    25575  ,    24966  ,    24385  ,    23831  ,
  23301 ,    22795  ,    22310  ,    21845  ,    21399  ,    20971  ,    20560  ,    20164  ,    19784  ,    19418  ,    19065  ,
  18724 ,    18396  ,    18078  ,    17772  ,    17476  ,    17189  ,    16912  ,    16644  ,    16384  ,    16131  ,    15887  ,
  15650 ,    15420  ,    15196  ,    14979  ,    14768  ,    14563  ,    14364  ,    14169  ,    13981  ,    13797  ,    13617  ,
  13443 ,    13273  ,    13107  ,    12945  ,    12787  ,    12633  ,    12483  ,    12336  ,    12192  ,    12052  ,    11915  ,
  11781 ,    11650  ,    11522  ,    11397  ,    11275  ,    11155  ,    11037  ,    10922  ,    10810  ,    10699  ,    10591  ,
  10485 ,    10381  ,    10280  ,    10180  ,    10082  ,    9986 ,    9892 ,    9799 ,    9709 ,    9619 ,    9532 ,
  9446  ,    9362 ,    9279 ,    9198 ,    9118 ,    9039 ,    8962 ,    8886 ,    8811 ,    8738 ,    8665 ,
  8594  ,    8525 ,    8456 ,    8388 ,    8322 ,    8256 ,    8192 ,    8128 ,    8065 ,    8004 ,    7943 ,
  7884  ,    7825 ,    7767 ,    7710 ,    7653 ,    7598 ,    7543 ,    7489 ,    7436 ,    7384 ,    7332 ,
  7281  ,    7231 ,    7182 ,    7133 ,    7084 ,    7037 ,    6990 ,    6944 ,    6898 ,    6853 ,    6808 ,
  6765  ,    6721 ,    6678 ,    6636 ,    6594 ,    6553 ,    6512 ,    6472 ,    6432 ,    6393 ,    6355 ,
  6316  ,    6278 ,    6241 ,    6204 ,    6168 ,    6132 ,    6096 ,    6061 ,    6026 ,    5991 ,    5957 ,
  5924  ,    5890 ,    5857 ,    5825 ,    5793 ,    5761 ,    5729 ,    5698 ,    5667 ,    5637 ,    5607 ,
  5577  ,    5548 ,    5518 ,    5489 ,    5461 ,    5433 ,    5405 ,    5377 ,    5349 ,    5322 ,    5295 ,
  5269  ,    5242 ,    5216 ,    5190 ,    5165 ,    5140 ,    5115 ,    5090 ,    5065 ,    5041 ,    5017 ,
  4993  ,    4969 ,    4946 ,    4922 ,    4899 ,    4877 ,    4854 ,    4832 ,    4809 ,    4788 ,    4766 ,
  4744  ,    4723 ,    4702 ,    4681 ,    4660 ,    4639 ,    4619 ,    4599 ,    4578 ,    4559 ,    4539 ,
  4519  ,    4500 ,    4481 ,    4462 ,    4443 ,    4424 ,    4405 ,    4387 ,    4369 ,    4350 ,    4332 ,
  4315  ,    4297 ,    4279 ,    4262 ,    4245 ,    4228 ,    4211 ,    4194 ,    4177 ,    4161 ,    4144 ,
  4128  ,    4112 ,    4096 ,    4080 ,    4064 ,    4048 ,    4032 ,    4017 ,    4002 ,    3986 ,    3971 ,
  3956  ,    3942 ,    3927 ,    3912 ,    3898 ,    3883 ,    3869 ,    3855 ,    3840 ,    3826 ,    3813 ,
  3799  ,    3785 ,    3771 ,    3758 ,    3744 ,    3731 ,    3718 ,    3705 ,    3692 ,    3679 ,    3666 ,
  3653  ,    3640 ,    3628 ,    3615 ,    3603 ,    3591 ,    3578 ,    3566 ,    3554 ,    3542 ,    3530 ,
  3518  ,    3506 ,    3495 ,    3483 ,    3472 ,    3460 ,    3449 ,    3437 ,    3426 ,    3415 ,    3404 ,
  3393  ,    3382 ,    3371 ,    3360 ,    3350 ,    3339 ,    3328 ,    3318 ,    3307 ,    3297 ,    3287 ,
  3276  ,    3266 ,    3256 ,    3246 ,    3236 ,    3226 ,    3216 ,    3206 ,    3196 ,    3187 ,    3177 ,
  3167  ,    3158 ,    3148 ,    3139 ,    3130 ,    3120 ,    3111 ,    3102 ,    3093 ,    3084 ,    3075 ,
  3066  ,    3057 ,    3048 ,    3039 ,    3030 ,    3021 ,    3013 ,    3004 ,    2995 ,    2987 ,    2978 ,
  2970  ,    2962 ,    2953 ,    2945 ,    2937 ,    2928 ,    2920 ,    2912 ,    2904 ,    2896 ,    2888 ,
  2880  ,    2872 ,    2864 ,    2857 ,    2849 ,    2841 ,    2833 ,    2826 ,    2818 ,    2811 ,    2803 ,
  2796  ,    2788 ,    2781 ,    2774 ,    2766 ,    2759 ,    2752 ,    2744 ,    2737 ,    2730 ,    2723 ,
  2716  ,    2709 ,    2702 ,    2695 ,    2688 ,    2681 ,    2674 ,    2668 ,    2661 ,    2654 ,    2647 ,
  2641  ,    2634 ,    2628 ,    2621 ,    2614 ,    2608 ,    2601 ,    2595 ,    2589 ,    2582 ,    2576 ,
  2570  ,    2563 ,    2557 ,    2551 ,    2545 ,    2538 ,    2532 ,    2526 ,    2520 ,    2514 ,    2508 ,
  2502  ,    2496 ,    2490 ,    2484 ,    2478 ,    2473 ,    2467 ,    2461 ,    2455 ,    2449 ,    2444 ,
  2438  ,    2432 ,    2427 ,    2421 ,    2416 ,    2410 ,    2404 ,    2399 ,    2394 ,    2388 ,    2383 ,
  2377  ,    2372 ,    2366 ,    2361 ,    2356 ,    2351 ,    2345 ,    2340 ,    2335 ,    2330 ,    2325 ,
  2319  ,    2314 ,    2309 ,    2304 ,    2299 ,    2294 ,    2289 ,    2284 ,    2279 ,    2274 ,    2269 ,
  2264  ,    2259 ,    2255 ,    2250 ,    2245 ,    2240 ,    2235 ,    2231 ,    2226 ,    2221 ,    2216 ,
  2212  ,    2207 ,    2202 ,    2198 ,    2193 ,    2189 ,    2184 ,    2179 ,    2175 ,    2170 ,    2166 ,
  2162  ,    2157 ,    2153 ,    2148 ,    2144 ,    2139 ,    2135 ,    2131 ,    2126 ,    2122 ,    2118 ,
  2114  ,    2109 ,    2105 ,    2101 ,    2097 ,    2092 ,    2088 ,    2084 ,    2080 ,    2076 ,    2072 ,
  2068  ,    2064 ,    2060 ,    2056 ,    2052 ,    2048 ,    2044 ,    2040 ,    2036 ,    2032 ,    2028 ,
  2024  ,    2020 ,    2016 ,    2012 ,    2008 ,    2004 ,    2001 ,    1997 ,    1993 ,    1989 ,    1985 ,
  1982  ,    1978 ,    1974 ,    1971 ,    1967 ,    1963 ,    1959 ,    1956 ,    1952 ,    1949 ,    1945 ,
  1941  ,    1938 ,    1934 ,    1931 ,    1927 ,    1923 ,    1920 ,    1916 ,    1913 ,    1909 ,    1906 ,
  1903  ,    1899 ,    1896 ,    1892 ,    1889 ,    1885 ,    1882 ,    1879 ,    1875 ,    1872 ,    1869 ,
  1865  ,    1862 ,    1859 ,    1855 ,    1852 ,    1849 ,    1846 ,    1842 ,    1839 ,    1836 ,    1833 ,
  1829  ,    1826 ,    1823 ,    1820 ,    1817 ,    1814 ,    1811 ,    1807 ,    1804 ,    1801 ,    1798 ,
  1795  ,    1792 ,    1789 ,    1786 ,    1783 ,    1780 ,    1777 ,    1774 ,    1771 ,    1768 ,    1765 ,
  1762  ,    1759 ,    1756 ,    1753 ,    1750 ,    1747 ,    1744 ,    1741 ,    1738 ,    1736 ,    1733 ,
  1730  ,    1727 ,    1724 ,    1721 ,    1718 ,    1716 ,    1713 ,    1710 ,    1707 ,    1705 ,    1702 ,
  1699  ,    1696 ,    1693 ,    1691 ,    1688 ,    1685 ,    1683 ,    1680 ,    1677 ,    1675 ,    1672 ,
  1669  ,    1667 ,    1664 ,    1661 ,    1659 ,    1656 ,    1653 ,    1651 ,    1648 ,    1646 ,    1643 ,
  1640  ,    1638 ,    1635 ,    1633 ,    1630 ,    1628 ,    1625 ,    1623 ,    1620 ,    1618 ,    1615 ,
  1613  ,    1610 ,    1608 ,    1605 ,    1603 ,    1600 ,    1598 ,    1596 ,    1593 ,    1591 ,    1588 ,
  1586  ,    1583 ,    1581 ,    1579 ,    1576 ,    1574 ,    1572 ,    1569 ,    1567 ,    1565 ,    1562 ,
  1560  ,    1558 ,    1555 ,    1553 ,    1551 ,    1548 ,    1546 ,    1544 ,    1542 ,    1539 ,    1537 ,
  1535  ,    1533 ,    1530 ,    1528 ,    1526 ,    1524 ,    1521 ,    1519 ,    1517 ,    1515 ,    1513 ,
  1510  ,    1508 ,    1506 ,    1504 ,    1502 ,    1500 ,    1497 ,    1495 ,    1493 ,    1491 ,    1489 ,
  1487  ,    1485 ,    1483 ,    1481 ,    1478 ,    1476 ,    1474 ,    1472 ,    1470 ,    1468 ,    1466 ,
  1464  ,    1462 ,    1460 ,    1458 ,    1456 ,    1454 ,    1452 ,    1450 ,    1448 ,    1446 ,    1444 ,
  1442  ,    1440 ,    1438 ,    1436 ,    1434 ,    1432 ,    1430 ,    1428 ,    1426 ,    1424 ,    1422 ,
  1420  ,    1418 ,    1416 ,    1415 ,    1413 ,    1411 ,    1409 ,    1407 ,    1405 ,    1403 ,    1401 ,
  1399  ,    1398 ,    1396 ,    1394 ,    1392 ,    1390 ,    1388 ,    1387 ,    1385 ,    1383 ,    1381 ,
  1379  ,    1377 ,    1376 ,    1374 ,    1372 ,    1370 ,    1368 ,    1367 ,    1365 ,    1363 ,    1361 ,
  1360  ,    1358 ,    1356 ,    1354 ,    1353 ,    1351 ,    1349 ,    1347 ,    1346 ,    1344 ,    1342 ,
  1340  ,    1339 ,    1337 ,    1335 ,    1334 ,    1332 ,    1330 ,    1328 ,    1327 ,    1325 ,    1323 ,
  1322  ,    1320 ,    1318 ,    1317 ,    1315 ,    1314 ,    1312 ,    1310 ,    1309 ,    1307 ,    1305 ,
  1304  ,    1302 ,    1300 ,    1299 ,    1297 ,    1296 ,    1294 ,    1292 ,    1291 ,    1289 ,    1288 ,
  1286  ,    1285 ,    1283 ,    1281 ,    1280 ,    1278 ,    1277 ,    1275 ,    1274 ,    1272 ,    1271 ,
  1269  ,    1267 ,    1266 ,    1264 ,    1263 ,    1261 ,    1260 ,    1258 ,    1257 ,    1255 ,    1254 ,
  1252  ,    1251 ,    1249 ,    1248 ,    1246 ,    1245 ,    1243 ,    1242 ,    1240 ,    1239 ,    1237 ,
  1236  ,    1235 ,    1233 ,    1232 ,    1230 ,    1229 ,    1227 ,    1226 ,    1224 ,    1223 ,    1222 ,
  1220  ,    1219 ,    1217 ,    1216 ,    1215 ,    1213 ,    1212 ,    1210 ,    1209 ,    1208 ,    1206 ,
  1205  ,    1203 ,    1202 ,    1201 ,    1199 ,    1198 ,    1197 ,    1195 ,    1194 ,    1192 ,    1191 ,
  1190  ,    1188 ,    1187 ,    1186 ,    1184 ,    1183 ,    1182 ,    1180 ,    1179 ,    1178 ,    1176 ,
  1175  ,    1174 ,    1172 ,    1171 ,    1170 ,    1168 ,    1167 ,    1166 ,    1165 ,    1163 ,    1162 ,
  1161  ,    1159 ,    1158 ,    1157 ,    1156 ,    1154 ,    1153 ,    1152 ,    1151 ,    1149 ,    1148 ,
  1147  ,    1145 ,    1144 ,    1143 ,    1142 ,    1140 ,    1139 ,    1138 ,    1137 ,    1136 ,    1134 ,
  1133  ,    1132 ,    1131 ,    1129 ,    1128 ,    1127 ,    1126 ,    1125 ,    1123 ,    1122 ,    1121 ,
  1120  ,    1119 ,    1117 ,    1116 ,    1115 ,    1114 ,    1113 ,    1111 ,    1110 ,    1109 ,    1108 ,
  1107  ,    1106 ,    1104 ,    1103 ,    1102 ,    1101 ,    1100 ,    1099 ,    1097 ,    1096 ,    1095 ,
  1094  ,    1093 ,    1092 ,    1091 ,    1089 ,    1088 ,    1087 ,    1086 ,    1085 ,    1084 ,    1083 ,
  1082  ,    1081 ,    1079 ,    1078 ,    1077 ,    1076 ,    1075 ,    1074 ,    1073 ,    1072 ,    1071 ,
  1069  ,    1068 ,    1067 ,    1066 ,    1065 ,    1064 ,    1063 ,    1062 ,    1061 ,    1060 ,    1059 ,
  1058  ,    1057 ,    1055 ,    1054 ,    1053 ,    1052 ,    1051 ,    1050 ,    1049 ,    1048 ,    1047 ,
  1046  ,    1045 ,    1044 ,    1043 ,    1042 ,    1041 ,    1040 ,    1039 ,    1038 ,    1037 ,    1036 ,
  1035  ,    1034 ,    1033 ,    1032 ,    1031 ,    1030 ,    1029 ,    1028 ,    1027 ,    1026 ,    1025 ,
  1024
};

typedef enum {
  GAMMA_TBL_ALL = 0,
  GAMMA_TBL_R,
  GAMMA_TBL_G,
  GAMMA_TBL_B,
} hdr_gamma_tbl_type_t;

typedef enum {
  HDR_NO_MEMORY = -2,
  HDR_ERROR,
  HDR_SUCESS,
} hdr_return_t;
/*Sub sample format*/
typedef enum {
  HDR_H2V2 = 0,
  HDR_H2V1 = 1,
  HDR_H1V2 = 2,
  HDR_H1V1 = 3,
  HDR_SUBSAMPLE_MAX

}subsample_format_type;


/*============================================================================
  Type DECLARATIONS
============================================================================*/
typedef struct {
  hdr_gamma_tbl_type_t gamma_t;
  uint32_t entry;
  void *gamma_tbl;
} hdr_gamma_table_struct_t;
//See whether these global variables need to be moved later
typedef struct {
  uint32_t valid;
  /// This is the noise profile data size, should be less than CAMERA_WAVELET_DENOISING_DATA_SIZE
  uint32_t denoiseDataSize;
  /// This is the array of noise profile data
  uint32_t  noiseProfileData[CAMERA_WAVELET_DENOISING_DATA_SIZE];
} hdr_denoising_type_t;

typedef struct {
  uint32_t mCalculatedExposureRatioG;
  uint32_t *mpRedInverseGammatable;
  uint32_t *mpGreenInverseGammatable;
  uint32_t *mpBlueInverseGammatable;
  uint32_t *mpRedNewGammatable;
  uint32_t *mpGreenNewGammatable;
  uint32_t *mpBlueNewGammatable;
}hdr_parameter_struct_t;
typedef enum {
  YCRCB,
  YCBCR
} hdr_chroma_order_t;

typedef enum {
  Y,
  CR,
  CB
} hdr_pixel_component_type_t;
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

typedef struct {
  int32_t vert[49];
  int32_t hori[49];
}hdr_motion_vector_t;
//Wavelet noise reduction struct
typedef struct {
  uint8_t  *pInputImage;
  int16_t  *pWorkingImage;
  int16_t  *pLineImage;
  hdr_denoising_type_t *pNoiseProfile;
  uint32_t  inputWidth;
  uint32_t  inputHeight;
  uint32_t  levels;
  hdr_pixel_component_type_t  lumaChromaIndicator;  //0-Y 1- Chroma
}
hdr_dwt_noise_reduction_struct_t;
typedef struct {
  void *buffer;
  uint32_t y_offset;
  uint32_t cbcr_offset;
  uint32_t output_flag;  /* tell algorithm to write back*/
} hdr_frame_type_t;

typedef struct {
  uint32_t         frame_num;
  hdr_frame_type_t *frames;
} hdr_frames_t;

typedef struct {
  //Longer Exposure Time for multiframe HDR OR
  //Normal frame for single frame hdr
  void  *pHdrBuffer1Y;
  void  *pHdrBuffer1C;
  void  *pHdrBuffer1R;
  void  *pHdrBuffer1G;
  void  *pHdrBuffer1B;
  //Shorter Exposure Time for multiframe HDR
  //Not used for single frame HDR
  void  *pHdrBuffer2Y;
  void  *pHdrBuffer2C;
  void  *pHdrBuffer2R;
  void  *pHdrBuffer2G;
  void  *pHdrBuffer2B;
  //Temp buffers circular
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
  uint32_t segmentOutputHeight;
  uint8_t *pMask;
  uint8_t *pMarker;
  void *pVoidHistogramArray;
  uint32_t startingLine;
  uint32_t endingLine;
  uint32_t startingRange;
  uint32_t endingRange;
  hdr_chroma_order_t chromaOrder;
  hdr_denoising_type_t *pNoiseProfile;
  hdr_dwt_noise_reduction_struct_t *pNoiseReduction;
  hdr_pixel_component_type_t pixelComponent;
} hdr_config_t;

typedef struct {
  hdr_gamma_table_struct_t pGammaTableStruct;
  hdr_parameter_struct_t mParamterStruct;
  hdr_config_t structHdrConfig;
  int num_hal_buf;
} hdr_t;

/**
* This structure contains information of denoising data.
* The information can be used to used by PPF to reduce noise
*/

/*============================================================================
  FUNCTION DECLARATIONS
============================================================================*/

int hdr_process (void *Ctrl,hdr_t *hdrCtrl);
void hdr_stop ( hdr_t *hdrCtrl);
int hdr_calculate_gammatbl(void *Ctrl, hdr_t *hdrCtrl);
#ifndef MM_CAMERA_NEON_ENABLED
  #define hdrChromaUpsampleHori(pChroma, pChromaUpsample, length) NULL
  #define hdrChromaUpsample2d(pChromaNear, pChromaFar,pChromaUpsample, length) NULL
  #define hdrColorConvertH2V2(Y, C, R, G,B, Height, Width, crcb) NULL
  #define hdrTwoFrameInterpolateMv(dc, diffPerStep, size,pOutVec) NULL
  #define  hdrTwoFrameInterpolateMvRounded(dc, diffPerStep, size, pOutVec) NULL
  #define  hdrTwoFrameCombine( ScaleCopy, R1,G1, B1, R2, G2, B2, len) NULL
  #define hdrTwoFrameCalculateProjectionsFirstFrame(Img, pH,pV1, pV2, pV3,pV4,pV5,pV6,pV7,pV8,Width) NULL
  #define hdrTwoFrameCalculateProjectionsSecondFrame(Img,pH,pV1,pV2,pV3,pV4,pV5,pV6,pV7,pV8,Width,ipRatio) NULL
  #define hdrTwoFrameChromaProcess(ChromaIn, ChromaOut,pChromaScale, length) NULL
  #define hdrYuvtorgb(pY, pCrCb, pRed, pGreen, pBlue, length, crcbindicator) NULL
  #define hdrYuvtog(pY, pCrCb, pGreen, length, crcbindicator) NULL

#endif
/*===========================================================================

Function           : hdrSingleFrameCore

Description        : Entry point for single frame HDR algorithm
1. Create chroma preprocess thread and run it in background
2. Calculate luma histogram
3. Intra Filter luma histogram, contrast control, and inter filter histogram
4. Wait for preprocess thread to join
5. Create and dispatch threads to perform adaptive equalization

Input parameter(s) : Pointer to the image configuration struture (pIn).
store Return status

Output parameter(s): pIn->pHdrBuffer1Y -> HDR output Y
pIn->pHdrBuffer1C -> HDR output CbCr
pReturnStatus containing return code

Return Value       : none

Side Effects       : None

=========================================================================== */

void hdrSingleFrameCore (hdr_config_t *pIn, hdr_return_t *pReturnStatus);

/*===========================================================================

Function           : hdrTwoFrameCore

Description        : Entry point for 2 frame HDR algorithm

Input parameter(s) : Pointer to the image configuration struture (pIn).
structure to store Return status (pReturnStatus)

Output parameter(s): pIn->pHdrBuffer1Y -> HDR output Y
pIn->pHdrBuffer1C -> HDR output CbCr
pReturnStatus containing return code

Return Value       : none

Side Effects       : None

=========================================================================== */

void hdrTwoFrameCore (hdr_config_t *pIn, hdr_return_t *pReturnStatus);

/*===========================================================================

Function           : hdrCalculateInverseGamma

Description        : Internal function to calculate and set inverse gamma tables

Input parameter(s) : pGammaTableStruct

Output parameter(s): inverseGammatable

Return Value       : none

Side Effects       : None

=========================================================================== */

int hdrCalculateInverseGamma(hdr_gamma_table_struct_t *pGammaTableStruct,uint32_t * inverseGammatable);

/*===========================================================================

Function           : hdrCalculateNewGammaTable

Description        : Internal function to calculate new gamma tables

Input parameter(s) : pGammaTableStruct

Output parameter(s): newGammatable

Return Value       : hdr_return_t

Side Effects       : None

=========================================================================== */

void hdrCalculateNewGammaTable(hdr_gamma_table_struct_t *pGammaTableStruct,uint32_t * newGammatable);
/*===========================================================================

Function           : hdrChromaUpsampleVert

Description        : Upsample Chroma line H1Vx -> H1Vx (vertical Upsampling)

Input parameter(s) : pChromaNear
pchromafar
length

Output parameter(s): pChromaUpsample

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrChromaUpsampleVert(uint8_t *pChromaNear,
  uint8_t *pChromaFar,
  uint8_t *pChromaUpsample,
  uint32_t length);

#ifdef MM_CAMERA_NEON_ENABLED
/*===========================================================================

Function           : hdrChromaUpsampleHori

Description        : Upsample Chroma line H2Vx -> H1Vx (Horizontal Upsampling)

Input parameter(s) : pChroma
length

Output parameter(s): pChromaUpsample

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrChromaUpsampleHori(uint8_t *pChroma,
  uint8_t *pChromaUpsample,
  uint32_t length);

/*===========================================================================

Function           : hdrChromaUpsample2d

Description        : Upsample Chroma line both horizontal and vertical
with an additional neighboring chroma lines

Input parameter(s) : pChromaNear
pChromaFar
length

Output parameter(s): pChromaUpsample

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrChromaUpsample2d(uint8_t *pChromaNear,
  uint8_t *pChromaFar,
  uint8_t *pChromaUpsample,
  uint32_t length);
#endif
/*===========================================================================

Function           : hdrUpsampleScale2d

Description        : Upsample scale map in 2 dimensions

Input parameter(s) : pInput
ipwidth
ipheight
opwidth
opheight
startingLine
endingLine

Output parameter(s): pOutput

Return Value       : hdr_return_t

Side Effects       : None

=========================================================================== */
hdr_return_t hdrUpsampleScale2d(uint8_t *pInput,
  uint8_t *pOutput,
  uint32_t ipwidth,
  uint32_t ipheight,
  uint32_t opwidth,
  uint32_t opheight,
  uint32_t startingLine,
  uint32_t endingLine);

/*===========================================================================

Function           : hdrDownsampleScale16

Description        : Downscale map in 2 dimensions, by 4 each,
Input is 4 line buffer , output is subsampled version 4 in each direction

Input parameter(s) : pInput
ipwidth

Output parameter(s): pOutput

Return Value       : hdr_return_t

Side Effects       : None

=========================================================================== */
void hdrDownsampleScale16(uint8_t *pInput,
  uint8_t *pOutput,
  uint32_t ipwidth);


/*===========================================================================

Function           : hdrInvGammaGreenScale

Description        : Compute inverse Gamma and output Green component
Also determine and output scale map. This function is called for frame1

Input parameter(s) : pRed
pGreen
pBlue
pRedGammaInverseTable
pGreenGammaInverseTable
pBlueGammaInverseTable
length

Output parameter(s): pGreen - Inverse Gamma green
pMask


Return Value       : None

Side Effects       : None

=========================================================================== */


void hdrInvGammaGreenScale(uint16_t  *pRed,
  uint16_t *pGreen,
  uint16_t *pBlue,
  uint8_t  *pMask,
  uint32_t *pRedGammaInverseTable,
  uint32_t *pGreenGammaInverseTable,
  uint32_t *pBlueGammaInverseTable,
  uint32_t  length);

/*===========================================================================

Function           : hdrInvGammaGreen

Description        : Compute inverse Gamma and output Green component
This function is called for frame2

Input parameter(s) : pGreen
pGreenGammaInverseTable
length

Output parameter(s): pGreen - Inverse Gamma green


Return Value       : None

Side Effects       : None

=========================================================================== */


void hdrInvGammaGreen(uint16_t *pGreen,
  uint32_t *pGreenGammaInverseTable,
  uint32_t  length);


/*===========================================================================

Function           : hdrInvGammaRGB

Description        : Compute inverse Gamma and output Red,Green,Blue component
This function is called for frame1 and frame2


Input parameter(s) : pRed
pGreen
pBlue
pRedGammaInverseTable
pGreenGammaInverseTable
pBlueGammaInverseTable
length
frameid

Output parameter(s): pRed, PBlue,pGreen



Return Value       : None

Side Effects       : None

=========================================================================== */

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
/*===========================================================================

Function           : hdrYuvtorgb

Description        : Converting 8-bit YUV to RGB 888. This function returns
red,green and blue

Input parameter(s) : p_y
p_crcb
length

Output parameter(s): pRed
pGreen
pBlue

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrYuvtorgb         (uint8_t  *pY,
  uint8_t  *pCrCb,
  uint16_t *pRed,
  uint16_t *pGreen,
  uint16_t *pBlue,
  uint32_t  length,
  uint32_t crcbindicator);
/*===========================================================================

Function           : hdrYuvtog

Description        : Converting 8-bit YUV to G 8. This function returns
green

Input parameter(s) : p_y
p_crcb
length

Output parameter(s):
pGreen

Return Value       : None

Side Effects       : None

=========================================================================== */


void hdrYuvtog         (uint8_t  *pY,
  uint8_t  *pCrCb,
  uint16_t *pGreen,
  uint32_t  length,
  uint32_t crcbindicator);
#endif

/*===========================================================================

Function           : hdrGammaConvert

Description        : Does gamma correction for R,G,B


Input parameter(s) :
pRedGammaInverseTable
pGreenGammaInverseTable
pBlueGammaInverseTable
Red
pGreen
pBlue
imageHeight
imageWidth

Output parameter(s): pRed, PBlue,pGreen



Return Value       : None

Side Effects       : None

=========================================================================== */


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
void hdrColorConvertH2V2(uint16_t *pOutputImageY,
  int16_t *pOutputImageC,
  uint16_t *pInputImageR,
  uint16_t *pInputImageG,
  uint16_t *pInputImageB,
  uint32_t imageHeight,
  uint32_t imageWidth,
  uint32_t crcbindicator);
#endif
/*===========================================================================

Function           : hdrReconstruct

Description        : Imfill function does morphological image filling

Input parameter(s) : pMask
pMarker
height
width

Output parameter(s): pMask

Return Value       : hdr_return_t

Side Effects       : None

=========================================================================== */

hdr_return_t hdrReconstruct(uint8_t *pMask,
  uint8_t *pMarker,
  uint32_t iHeight,
  uint32_t iWidth);
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
hdr_return_t hdrHistogramProcess   (hdr_config_t *pIn,
  void * pVoidHistogramArray,
  uint32_t imageHeight,
  uint32_t imageWidth);

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

Function           : hdrTonemapIntraFilterHistogram

Description        : This function does intra-filtering of histogram

Input parameter(s) : pHistLocal,pTmpHist
(input histogram,scratch area )

Output parameter(s): pHistLocal
(filtered histogram)

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrTonemapIntraFilterHistogram  (uint32_t *pHistLocal,
  uint32_t *pTmpHist);

/*===========================================================================

Function           : hdrTonemapHistogramNorm

Description        : Normalize the histogram block

Input parameter(s) : pHistLocal
represents
input histogram block (one among 64 blocks)

Output parameter(s): pHistLocal
represents
normalized histogram block

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrTonemapHistogramNorm (uint32_t *pHistLocal);

/*===========================================================================

Function           : hdrTonemapContrastControl

Description        : This function does contrast limiting. Modulates the given
histogram with the contrast function . After that it computes
CDF

Input parameter(s) : pHistLocal,pHistTemp1,pHistTemp2,pContrastFunction
(Given histogram pdf, scratch area, scratch area, contrast function)

Output parameter(s): pHistLocal
(contrast limited CDF)

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrTonemapContrastControl  (uint32_t *pHistLocal,
  uint32_t *pHistTemp1,
  uint32_t *pHistTemp2,
  uint32_t *pContrastFunction);

/*===========================================================================

Function           : hdrAverageHistogramThreeBlocks

Description        : Calculate the average of histogram block  from the
neighbouring 2 blocks.This is applicable for all histogram blocks
in the 4 edges except the ones at the 4 corners

Input parameter(s) : pHist1,pHist2,pHist3
represents
histip(i,j),histip(i-1,j),histip(i+1,j) for blocks in vertical edges
other than corners
OR
histip(i,j),histip(i,j-1),histip(i,j+1) for blocks in horizontal edges
other than corners

Output parameter(s): pHistOut
represents
filteredhistop(i,j)

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrAverageHistogramThreeBlocks(uint32_t *pHist1,
  uint32_t *pHist2,
  uint32_t *pHist3,
  uint32_t *pHistOut);

/*===========================================================================
Function           :  hdrAverageHistogramFiveBlocks

Description        :  Calculate the average of histogram block  from the
neighbouring 4 blocks

Input parameter(s) :  pHist1,pHist2,pHist3,pHist4,pHist5
represents
histip(i,j),histip(i-1,j),histip(i+1,j),histip(i,j-1),histip(i,j+1)
(i,j) i=0:7,j=0:7 are the 64 histogram blocks

Output parameter(s):  pHistOut
represents
filteredhistop(i,j)

Return Value       :  None

Side Effects       :  None

=========================================================================== */

void hdrAverageHistogramFiveBlocks (uint32_t *pHist1,
  uint32_t *pHist2,
  uint32_t *pHist3,
  uint32_t *pHist4,
  uint32_t *pHist5,
  uint32_t *pHistOut);
/*===========================================================================

Function           : hdrTwoFrameGetRatio

Description        : This function calculates green ratio between frames

Input parameter(s) : Image structure (pIn)

Output parameter(s):
pInputRatioG computed green ratio between frames

Return Value       : void

Side Effects       : None

=========================================================================== */
void hdrTwoFrameGetRatio(hdr_config_t *pIn,
  uint32_t *pInputRatioG);

/*===========================================================================

Function           : hdrFreeProjections

Description        : Free projections

Input parameter(s) :  pProjection1
pProjection2

Output parameter(s): None

Return Value       : void

Side Effects       : None

=========================================================================== */
void hdrFreeProjections(hdr_projection_t *pProjection1,
  hdr_projection_t *pProjection2);

/*===========================================================================

Function           : hdrTwoFrameProjections

Description        : Calculate projections for all frame, overlapped segment quarter width, quarter height
If frame id is 2, we need to multiply by input ratio and make
sure it is limited to 1023.

Input parameter(s) :  Image structure (pIn)
frameid


Output parameter(s): pProjection

Return Value       : void

Side Effects       : None

=========================================================================== */
void hdrTwoFrameProjections(hdr_config_t *pIn,
  hdr_projection_t *pProjection,
  uint8_t frameid);

/*===========================================================================

Function           : hdrTwoFrameCorrelation

Description        : Compute the motion vector for each block
There are 49 blocks in each frame. The code takes one block from first and second frame
move the block from -lag to +lag each time finding the difference of (length-lag)
projections
and find the distance where it finds the highest correlation

Input parameter(s) :  Projection array1
Projection array2
block length
lag

Output parameter(s): pResult (motion vector result)

Return Value       : uint32_t

Side Effects       : None

=========================================================================== */
uint32_t hdrTwoFrameCorrelation(uint32_t *pInArray1,
  uint32_t *pInArray2,
  uint32_t length,
  uint32_t lag,
  int32_t *pResult);

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

/*===========================================================================

Function           : hdrTwoFrameCorrectMotionVector

Description        : Corrects the 49 motion vectors

Input parameter(s) : Input motion vector (49 horizontal and 49 vertical)

Output parameter(s): Corrected output motion vector

Return Value       : None

Side Effects       : None

=========================================================================== */
void hdrTwoFrameCorrectMotionVector(hdr_motion_vector_t *pInVec,
  hdr_motion_vector_t *pOutVec);
#ifdef MM_CAMERA_NEON_ENABLED
/*===========================================================================

Function           : hdrTwoFrameInterpolateMv

Description        : Interpolate motion vectors each line

Input parameter(s) : dc - init value
diffstep - step size
size - image width

Output parameter(s): output motion vector for each line

Return Value       : None

Side Effects       : None

=========================================================================== */
void hdrTwoFrameInterpolateMv(int32_t dc,
  int32_t diffPerStep,
  uint32_t size,
  int32_t *pOutVec);
/*===========================================================================

Function           : hdrTwoFrameInterpolateMvRounded

Description        : Interpolate motion vectors each line and round and shift by 20

Input parameter(s) : dc - init value
diffstep - step size
size - image width

Output parameter(s): output motion vector for each line

Return Value       : None

Side Effects       : None

=========================================================================== */
void hdrTwoFrameInterpolateMvRounded(int32_t dc,
  int32_t diffPerStep,
  uint32_t size,
  int32_t *pOutVec);

#endif
/*===========================================================================

Function           : hdrTwoFrameMotionCorrection

Description        : Interpolates and find motion vector of each pixel and
motion compensates the second frame. Also combines frame1 and frame2 based on scale.
All these are done in RGB domain and converted back to YUV. Works on a piece by piece basis

Input parameter(s) : Image structure

Output parameter(s): None

Return Value       : hdr_return_t

Side Effects       : None

=========================================================================== */
hdr_return_t hdrTwoFrameMotionCorrection(hdr_config_t *pIn);

#ifdef MM_CAMERA_NEON_ENABLED
/*===========================================================================

Function           : hdrTwoFrameCombine

Description        : Combine frame 1 and frame 2 using scale

Input parameter(s) :
pScaleCopy
pImageBuffer1R
pImageBuffer1G
pImageBuffer1B
pImageBuffer2R
pImageBuffer2G
pImageBuffer2B
length

Output parameter(s):
pImageBuffer1R
pImageBuffer1G
pImageBuffer1B

Return Value       : None

Side Effects       : None

=========================================================================== */
void hdrTwoFrameCombine(uint8_t *pScaleCopy,
  uint16_t *pImageBuffer1R,
  uint16_t *pImageBuffer1G,
  uint16_t *pImageBuffer1B,
  uint16_t *pImageBuffer2R,
  uint16_t *pImageBuffer2G,
  uint16_t *pImageBuffer2B,
  uint32_t length);

#endif
/*===========================================================================

Function           : hdrSqrt32

Description        : Determines square root of 64 bit number

Input parameter(s) : inputNumber.(output is in place)

Output parameter(s): None

Return Value       : uint32_t square root

Side Effects       : None

=========================================================================== */

uint32_t hdrSqrt32(uint64_t inputNumber);

/*===========================================================================

Function           : hdrTwoFrameTonemap

Description        : performs tone mapping for 2 frames
1) Divide image into 64 blocks and Compute histogram of each block
2) Intra-filtering of each histogram block
3) Contrast limiting of each histogram block and
Compute CDF of each histogram block
4) Inter-filtering between CDFs of neighboring histogram blocks
5) Adaptive histogram equalization using bilinear interpolation

Input parameter(s) : Image config structure

Output parameter(s): HDR Luma (pIn->pHdrBuffer1Y)
HDR chroma (pIn->pHdrBuffer1C)

Return Value       : hdr_return_t

Side Effects       : None

=========================================================================== */

hdr_return_t hdrTwoFrameTonemap(hdr_config_t *pIn);

/*===========================================================================
Function           : hdrTwoFrameHistogram

Description        : This function computes histogram for 2 frame HDR

Input parameter(s) : Pointer to the input image Y,(pInputImageY)
imageHeight
imageWidth
startingLine
endingLine

Output parameter(s): histogramArray (voidHistogramArrayPtr)

Return Value       : None

Side Effects       : None

=========================================================================== */

void hdrTwoFrameHistogram (uint16_t *pInputImageY,
  void * pVoidHistogramArray,
  uint32_t imageHeight,
  uint32_t imageWidth,
  uint32_t startingLine,
  uint32_t endingLine);

/*===========================================================================
Function           : hdrTwoFrameEqualize

Description        : This function does adaptive histogram equalization for 2 frame HDR

Input parameter(s) : pIn

Output parameter(s): pIn->pHdrBuffer1Y - Y HDR output
pIn->pHdrBuffer1c - CbCr HDR output

Return Value       : hdrreturnstatus

Side Effects       : None

=========================================================================== */
hdr_return_t hdrTwoFrameEqualize (hdr_config_t *pIn);

#ifdef MM_CAMERA_NEON_ENABLED
/*===========================================================================

Function           : hdrTwoFrameChromaProcess

Description        : Generates chroma output based on input chroma pixel and chroma scale

Input parameter(s) : pchromain - Input chroma pixel value
pchromascale - chroma scale map
length - image width

Output parameter(s): pchromaout1 - output chroma first component
pChromaOut2 - output chroma second component

Return Value       : None

Side Effects       : None

=========================================================================== */
void hdrTwoFrameChromaProcess(int16_t *pChromaIn,
  uint8_t *pChromaOut,
  uint16_t *pChromaScale,
  uint32_t length);

/*===========================================================================

Function           : hdrTwoFrameCalculateProjectionsFirstFrame

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
#endif
/*===========================================================================

Function           :  hdrTwoColorGammaInverseMaskH2V2

Description        : This function does color conversion Ycbcr->Green and gamma inverse for H2V2 and returns green.
For frame 1 it outputs the scale mask

Input parameter(s) :  pInputImageY
pInputImageC
redgammainversetable
pGreenGammaInverseTable
bluegammainversetable
imageheight
imagewidth
frameid


Output parameter(s):  pOutputImageG
pOutputMask

Return Value       : hdr_return_t

Side Effects       : None

=========================================================================== */
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

void hdrDwt53tab (
  uint8_t  *pInput,
  int16_t  *pWorkBufferInt16,
  const uint32_t  levelWidth,
  const uint32_t  levelHeight,
  const uint32_t  imageHeight,
  int16_t  *pLineBufferInt16);

void hdrDwtHaarRot (
  uint8_t  *pInput,
  int16_t  *pWorkBufferInt16,
  const uint32_t  imageWidth,
  const uint32_t  imageHeight,
  int16_t  *pLineBufferInt16);

void hdrWaveletTransformInverse2d2lines (
  uint8_t  *pInput,
  int16_t  *pWorkBufferInt16,
  const uint32_t  levelWidth,
  const uint32_t  levelHeight,
  const uint32_t  imageHeight,
  int16_t  *pLineBufferInt16);

void hdrWaveletTransformInverse2drothaar (
  uint8_t  *pInput,
  int16_t  *pWorkBufferInt16,
  const uint32_t  imageWidth,
  const uint32_t  imageHeight,
  int16_t  *pLineBufferInt16);

void edge_weighting (int16_t  *p_input,  // Pointer to the int16 p_input
  const  uint32_t  width,  // Input buffer width
  const  uint32_t  height,  // Input buffer height
  const  uint32_t  stride,  // Input buffer stride
  const  uint32_t  weight,  // Given weight
  const  uint32_t  edge_limit,  // Given edge limit
  const  uint32_t  hl_threshold,  // Given HL threshold
  const  uint32_t  hh_threshold,  //Given HH threshold
  const  uint32_t edge_denoising_factor);  // Given edge denoising factor
void dwt_haar_uint8_randomwrite (
  int16_t *pOutputLowpass,
  uint8_t *pInput,
  const uint32_t length,
  const uint32_t outputIncrement);

void dwt_haar_uint8_randomwrite_4lines (
  int16_t *pOutputLowpass,
  uint8_t *pInput,
  const uint32_t length,
  const uint32_t outputIncrement) ;

void dwt_haar_int16_seqwrite (
  int16_t *pOutputLowpass,
  int16_t *pInput,
  const uint32_t length,
  const uint32_t outputIncrement);

void dwt_53tab_int16_randomwrite (
  int16_t *pOutputLowpass,
  int16_t *pInput,
  const uint32_t length,
  const uint32_t outputIncrement);

void dwt_53tab_int16_randomwrite_2lines (
  int16_t *pOutputLowpass,
  int16_t *pInput,
  const uint32_t length,
  const uint32_t outputIncrement);

void wavelet_transform_inverse_1d_int16_2lines(
  int16_t *pOutputLowpass,
  int16_t *pInput,
  const uint32_t length,
  const uint32_t outputIncrement);

void wavelet_transform_inverse_1d_int16(
  int16_t *pOutputLowpass,
  int16_t *pInput,
  const uint32_t length,
  const uint32_t outputIncrement);

void wavelet_transform_inverse_1d_int16_haar_4lines(
  int16_t *pOutputLowpass,
  int16_t *pInput,
  const uint32_t length,
  const uint32_t outputIncrement);

void hdrWaveletTransformInverse1dint16haar(
  int16_t *pOutputLowpass,
  int16_t *pInput,
  const uint32_t length,
  const uint32_t outputIncrement);

void wavelet_transform_inverse_1d_uint8_haar (
  uint8_t *pOutputLowpass,
  int16_t *pInput,
  const uint32_t length);
/*===========================================================================
FUNCTION        hdrWaveletDenoisingSegment

DESCRIPTION     High level function performs wavelet denoising for HDR

DEPENDENCIES    None

RETURN VALUE    hdr_return_t

SIDE EFFECTS    None
===========================================================================*/
hdr_return_t  hdrWaveletDenoisingSegment(hdr_dwt_noise_reduction_struct_t *arg);

/*===========================================================================
FUNCTION        epsilonFilterSmooth

DESCRIPTION     epsilon filter to smooth on final level

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/

void epsilonFilterSmooth(
  uint8_t  *pInputUint8,
  int16_t  *pWorkBufferInt16,
  const int32_t   levelWidth,
  const int32_t   levelHeight,
  const int32_t   imageHeight,
  const int32_t   epsilon);
#endif   /* HDR_INTERFACE_H */
