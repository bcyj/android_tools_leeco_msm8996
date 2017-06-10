/*============================================================================
  Copyright (c) 2009-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  FILE:         fm_qsoc_patches.c

  OVERVIEW:     This file implements functions to download firmware into the
                Core's program RAM. Once this code has been downloaded, one can
                Apply a ROM patch, Add custom RDS group processing, Execute
                downloaded code

============================================================================*/


/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
/* Necessary includes */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <media/tavarua.h>
#ifdef ANDROID
#include <cutils/properties.h>
#endif

#include "qmi.h"
#include "qmi_client.h"
#include "network_access_service_v01.h"

#define DEV_TYPE_FFA "Ffa"
#define DEV_TYPE_SURF "Surf"
#define HW_PLATFORM  "/sys/devices/soc0/hw_platform"
#define CAL_DATA_SIZE 23
#define V4L2_CID_PRIVATE_IRIS_DO_CALIBRATION 0x800002a
#define CAL_DATA_BUF 10
#define V4L2_BUF_LENGTH 128
#define CAL_FILE_PATH  "/data/app/Riva_fm_cal"
#define RADIO_DEVICE_PATH "/dev/radio0"
#define DLD_RF_PATCHES 11
#define SPUR_TABLE_REG_ADDR   (0x0BB7)

const char* fm_i2c_path_7x27A_SURF = "/dev/i2c-1";
const char* fm_i2c_path_7x30_SURF = "/dev/i2c-2";
const char* fm_i2c_path_8x60_SURF = "/dev/i2c-4";
static int is_marimba = -1;

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define MARIMBA_ADDR                  (0x0C)
#define TAVARUA_ADDR                  (0x2A)

/*
	WCN2243/BAHAMA related register addresses
*/
#define BAHAMA_RBIAS_CTL1       0x07
#define BAHAMA_FM_MODE_REG      0xFD
#define BAHAMA_FM_CTL1_REG      0xFE
#define BAHAMA_FM_CTL0_REG      0xFF
#define BAHAMA_FM_MODE_NORMAL   0x00
#define BAHAMA_ADDR 0x0C


/* FM6500 A0 chip version.
 *  */
#define FM6500_A0_VERSION             (0x01010013)
/**
 *  * FM6500 2.0 chip version.
 *   */
#define FMQSOCCOM_FM6500_20_VERSION   (0x01010010)
/**
 *  * FM6500 2.1 chip version.
 *   */
#define FMQSOCCOM_FM6500_21_VERSION   (0x02010204)
/**
 *  WCN 2243 1.0's FM chip version.
 */
#define FMQSOCCOM_FM6500_WCN2243_10_VERSION   (0x0302010A)

/**
 *  WCN 2243 2.0's FM chip version.
 */
#define FMQSOCCOM_FM6500_WCN2243_20_VERSION   (0x04020205)

/**
 *  WCN 2243 2.1's FM chip version.
 */
#define FMQSOCCOM_FM6500_WCN2243_21_VERSION   (0x04020309)

#define FM_QSOC_EDL_PATCH_INDEX       (0x03)
#define FM_QSOC_EDLMODE_INDEX         (0x3F)

#define XFRCTRL                       (0x1F)
#define LPNOTCHREG	         	(0x30)
#define HPNOTCHREG			(0x33)
#define INT_CTRL_MODE                 (0x19)
#define XFRDAT0                       (0x20)
#define INTSTAT_0                     (0x0)
#define INTSTAT_1                     (0x1)
#define INTSTAT_2                     (0x2)
#define CHIP_VERSION_XFR              (0x1B)
#define CHIPVER                       (2)
#define FWMAIN                        (5)
#define FWMIN                         (6)
#define FWREV                         (7)
#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(*a))

/* radio modes */
#define PROCESS_CAL_MODE              (0x05)
#define DC_CAL_MODE                   (0x06)
#define NORMAL_MODE                   (0x00)

#define XFRCTL_WRITE_MODE             (0x80)

#define FM_READY_ISR_BITMASK          (1<<0)
#define FM_TRANSFER_ISR_BITMASK       (1<<16)

#define CAL_DAT0_BUF_INDEX            (0)
#define CAL_DAT1_BUF_INDEX            (1)
#define CAL_DAT2_BUF_INDEX            (2)
#define CAL_DAT3_BUF_INDEX            (3)
#define CAL_DAT3_BUF_NUM              (CAL_DAT3_BUF_INDEX+1)

FILE* debug_file=NULL;

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
/* Timeout value for FM interrupt */
#define FMQSOCPATCHES_EXT_INT_TIMEOUT            (35)

#define FMQSOCPATCHES_XFR_SIZE                   (16)

/***********************************/
/*        Rx Timers Macros         */
/***********************************/

/**
 *  Time required for analog PLL tuning and settling to a new
 *  frequency.
 */
#define FM_RX_QSOC_TIMER_RADIO                   (1)

/**
 * Time required for digital modem settle before the audio can be turned on.
 */
#define FM_RX_QSOC_TIMER_MODEM                   (4)

/**
 * Periodic time interval for monitoring channel condition monitoring.
 */
#define FM_RX_QSOC_TIMER_CHCOND                  (20)

/**
 * Periodic time interval for interference detector monitoring.
 */
#define FM_RX_QSOC_TIMER_INTDET                  (17)

/**
 * Time required for a graceful hard mute to avoid "plopping" sound.
 */
#define FM_RX_QSOC_TIMER_SFHARD                  (3)

/**
 * Time to wait for RDS interrupt before declaring no RDS on channel.
 */
#define FM_RX_QSOC_TIMER_RDSTIMEOUT              (50)

/**
 * Periodic time interval to perform PLL tune voltage temperature compensation
 * (default 1 minute).
 */
#define FM_RX_QSOC_TIMER_TEMP                    (12000)

/**
 * Time to wait after Pilot PLL detected before enabling the RDS block.
 */
#define FM_RX_QSOC_TIMER_PILOTPLL                (10)

/**
 * Fast modem settling time for AF tuning.
 */
#define FM_RX_QSOC_TIMER_FASTMDM                 (4)

/**
 * Time required for digital modem to settle before reading signal metrics in
 * search modes.Currently set to 20ms (4 * 5).  This is the
 * lowest setting.
 */
#define FM_RX_QSOC_TIMER_SEARCH                  (10)

/**
 * Time to wait for Pilot PLL detected before transitioning to free running RDS
 * Non-Coherent mode.
 */
#define FM_RX_QSOC_TIMER_NONCOHERENT             (600)


#define XFR_MODE_RX_TIMERS                       (0x16)

#define FM_QSOC_MSB_BYTE_UINT16                  (0xFF00)
#define FM_QSOC_MSB_BYTE_UINT16_SHFT             (8)
#define FM_QSOC_LSB_BYTE_UINT16                  (0xFF)
#define FM_QSOC_LSB_BYTE_UINT16_SHFT             (0)

#define FM_RX_QSOC_TIMER_TEMP_MSB                ((FM_RX_QSOC_TIMER_TEMP & \
                                                   FM_QSOC_MSB_BYTE_UINT16) >> \
                                                   FM_QSOC_MSB_BYTE_UINT16_SHFT)
#define FM_RX_QSOC_TIMER_TEMP_LSB                ((FM_RX_QSOC_TIMER_TEMP & \
                                                   FM_QSOC_LSB_BYTE_UINT16) >> \
                                                   FM_QSOC_LSB_BYTE_UINT16_SHFT)
#define FM_RX_QSOC_TIMER_NONCOHERENT_TEMP_MSB    ((FM_RX_QSOC_TIMER_NONCOHERENT & \
                                                   FM_QSOC_MSB_BYTE_UINT16) >> \
                                                   FM_QSOC_MSB_BYTE_UINT16_SHFT)
#define FM_RX_QSOC_TIMER_NONCOHERENT_TEMP_LSB    ((FM_RX_QSOC_TIMER_NONCOHERENT & \
                                                   FM_QSOC_LSB_BYTE_UINT16) >> \
                                                   FM_QSOC_LSB_BYTE_UINT16_SHFT)

#define FM_QSOC_SPUR_POKE_DAT_1_IDX              (3)
#define FM_QSOC_MEMACC_CTRL_WRITE                (1<<7)
#define FM_QSOC_XFRCTRL_MEMACCESS                (1<<6)
#define FM_QSOC_XFRCTRL_MEMMODE_BLOCK            (0)
#define FM_QSOC_MEMACC_LENGTH_MASK               (0x0f)

/*
 * Timeout duration for waiting of interrupts from SOC
 */
#define TIMEOUT_DURATION			 (10)

/*
  This constant represents the RMSSI delta for searches.  For a station to be
  considered valid, there must be an N change from the noise floor.  This
  constant is this N value.
  For a 6dB change, use 0x86.
  For a 4dB change, use 0x84.
  For a 10dB change, use 0x8A.
*/
#define FMQSOCPATCHES_SEARCH_RMSSI_DELTA                (0x8B)

/*
  For FMC6500 2.1, the following is the INF_DET_OUT threhold high and low values.
  For a station to be found in search functions, the station must have an
  INF_DET_OUT value between the following high & low thresholds.  The
  detection window value MUST be a multiple of 6.25.
*/
#define FMQSOCPATCHES_SEARCH_INFDETOUT_LO(detWindow)    ((700 - detWindow)/6.25)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_HI(detWindow)    ((700 + detWindow)/6.25)

/* More restrictive to less restrictive */
#define FMQSOCPATCHES_SEARCH_INFDETOUT_12P5_LO          FMQSOCPATCHES_SEARCH_INFDETOUT_LO(12.5)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_12P5_HI          FMQSOCPATCHES_SEARCH_INFDETOUT_HI(12.5)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_18P75_LO         FMQSOCPATCHES_SEARCH_INFDETOUT_LO(18.75)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_18P75_HI         FMQSOCPATCHES_SEARCH_INFDETOUT_HI(18.75)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_25_LO            FMQSOCPATCHES_SEARCH_INFDETOUT_LO(25)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_25_HI            FMQSOCPATCHES_SEARCH_INFDETOUT_HI(25)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_31P25_LO         FMQSOCPATCHES_SEARCH_INFDETOUT_LO(31.25)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_31P25_HI         FMQSOCPATCHES_SEARCH_INFDETOUT_HI(31.25)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_31P25_LO         FMQSOCPATCHES_SEARCH_INFDETOUT_LO(31.25)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_31P25_HI         FMQSOCPATCHES_SEARCH_INFDETOUT_HI(31.25)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_37P5_LO          FMQSOCPATCHES_SEARCH_INFDETOUT_LO(37.5)
#define FMQSOCPATCHES_SEARCH_INFDETOUT_37P5_HI          FMQSOCPATCHES_SEARCH_INFDETOUT_HI(37.5)

/* The latest patches have additional checks to ensure stations at the end of
   the band are found.  This check can be disabled if needed. */
#define FMQSOCPATCHES_SEARCH_BANDEDGE_CHK               0x80, 0x07
#define FMQSOCPATCHES_SEARCH_NO_BANDEDGE_CHK            0x00, 0x00


/* WCN2243 (Bahama) Initial Noise Floor estimate for searches */
/* Initial noise floor estimate in dB */
#define FMQSOCPATCHES_SEARCH_NFE                        (-96)

/* WCN2243 (Bahama) RMSSI Noise Floor delta for searches */
/* Search Channel filter based on RMSSI above noise floor */
#define FMQSOCPATCHES_SEARCH_NF_DELTA                   (3)


/**
*   FM QSoC Patches Defaults type
*/
typedef struct
{
  unsigned char   ucMode;
  /**< Xfr mode. */

  unsigned char   ucDefaults[FMQSOCPATCHES_XFR_SIZE];
  /**< Xfr default payload. */

} tsFmQSocPatchesDefaultType;

typedef unsigned char FM_QSOC_XFRCTRL;


/**
*   FM QSoC Spur poke type
*/
typedef struct
{
  const unsigned char* spurPtr;
  //**< Pointer to spurs to be poked. */

  unsigned int         spurPokeAddr;
  /**< Address where spurs should be poked. */

  unsigned char        spurSize;
  /**< Size of spurs to be poked. */

} tsFmQSocPatchesSpurPokeType;

/**
* Array that stores the FM QSOC spur related data.
*/
static const unsigned char FMSocWCN2243spurFilteringData_0[] = \
{
   0x00, 0x0D, 0xAE
};
#define FMQSOCPATCHES_SPUR_FREQ_0_ADDR        (0x0BB8)
#define FMQSOCPATCHES_SPUR_FREQ_0_PTR         (&FMSocWCN2243spurFilteringData_0[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_0      (sizeof(FMSocWCN2243spurFilteringData_0) / \
                                               sizeof(FMSocWCN2243spurFilteringData_0[0]))

static const unsigned char FMSocWCN2243spurFilteringData_1[] = \
{
   0x00, 0x0E, 0xAE
};
#define FMQSOCPATCHES_SPUR_FREQ_1_ADDR        (0x0BBB)
#define FMQSOCPATCHES_SPUR_FREQ_1_PTR         (&FMSocWCN2243spurFilteringData_1[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_1      (sizeof(FMSocWCN2243spurFilteringData_1) / \
                                               sizeof(FMSocWCN2243spurFilteringData_1[0]))

static const unsigned char FMSocWCN2243spurFilteringData_2[] = \
{
   0x00, 0x0F, 0xAE
};
#define FMQSOCPATCHES_SPUR_FREQ_2_ADDR        (0x0BBE)
#define FMQSOCPATCHES_SPUR_FREQ_2_PTR         (&FMSocWCN2243spurFilteringData_2[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_2      (sizeof(FMSocWCN2243spurFilteringData_2) / \
                                               sizeof(FMSocWCN2243spurFilteringData_2[0]))

static const unsigned char FMSocWCN2243spurFilteringData_3[] = \
{
   0x00, 0x10, 0xAE
};
#define FMQSOCPATCHES_SPUR_FREQ_3_ADDR        (0x0BC1)
#define FMQSOCPATCHES_SPUR_FREQ_3_PTR         (&FMSocWCN2243spurFilteringData_3[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_3      (sizeof(FMSocWCN2243spurFilteringData_3) / \
                                               sizeof(FMSocWCN2243spurFilteringData_3[0]))

static const unsigned char FMSocWCN2243spurFilteringData_4[] = \
{
   0x00, 0x11, 0xAE
};
#define FMQSOCPATCHES_SPUR_FREQ_4_ADDR        (0x0BC4)
#define FMQSOCPATCHES_SPUR_FREQ_4_PTR         (&FMSocWCN2243spurFilteringData_4[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_4      (sizeof(FMSocWCN2243spurFilteringData_4) / \
                                               sizeof(FMSocWCN2243spurFilteringData_4[0]))

static const unsigned char FMSocWCN2243spurFilteringData_5[] = \
{
   0x00, 0x89, 0xAB
};
#define FMQSOCPATCHES_SPUR_FREQ_5_ADDR        (0x0BC7)
#define FMQSOCPATCHES_SPUR_FREQ_5_PTR         (&FMSocWCN2243spurFilteringData_5[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_5      (sizeof(FMSocWCN2243spurFilteringData_5) / \
                                               sizeof(FMSocWCN2243spurFilteringData_5[0]))

static const unsigned char FMSocWCN2243spurFilteringData_6[] = \
{
   0x00, 0x93, 0xAB
};
#define FMQSOCPATCHES_SPUR_FREQ_6_ADDR        (0x0BCA)
#define FMQSOCPATCHES_SPUR_FREQ_6_PTR         (&FMSocWCN2243spurFilteringData_6[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_6      (sizeof(FMSocWCN2243spurFilteringData_6) / \
                                               sizeof(FMSocWCN2243spurFilteringData_6[0]))

static const unsigned char FMSocWCN2243spurFilteringData_7[] = \
{
   0x00, 0x9D, 0xAB
};
#define FMQSOCPATCHES_SPUR_FREQ_7_ADDR        (0x0BCD)
#define FMQSOCPATCHES_SPUR_FREQ_7_PTR         (&FMSocWCN2243spurFilteringData_7[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_7      (sizeof(FMSocWCN2243spurFilteringData_7) / \
                                               sizeof(FMSocWCN2243spurFilteringData_7[0]))

static const unsigned char FMSocWCN2243spurFilteringData_8[] = \
{
   0x00, 0xC7, 0xA6
};
#define FMQSOCPATCHES_SPUR_FREQ_8_ADDR        (0x0BD0)
#define FMQSOCPATCHES_SPUR_FREQ_8_PTR         (&FMSocWCN2243spurFilteringData_8[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_8      (sizeof(FMSocWCN2243spurFilteringData_8) / \
                                               sizeof(FMSocWCN2243spurFilteringData_8[0]))

static const unsigned char FMSocWCN2243spurFilteringData_9[] = \
{
   0x00, 0xC8, 0xA6
};
#define FMQSOCPATCHES_SPUR_FREQ_9_ADDR        (0x0BD3)
#define FMQSOCPATCHES_SPUR_FREQ_9_PTR         (&FMSocWCN2243spurFilteringData_9[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_9      (sizeof(FMSocWCN2243spurFilteringData_9) / \
                                               sizeof(FMSocWCN2243spurFilteringData_9[0]))

static const unsigned char FMSocWCN2243spurFilteringData_10[] = \
{
   0x00, 0xC9, 0xA6
};
#define FMQSOCPATCHES_SPUR_FREQ_10_ADDR        (0x0BD6)
#define FMQSOCPATCHES_SPUR_FREQ_10_PTR         (&FMSocWCN2243spurFilteringData_10[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_10      (sizeof(FMSocWCN2243spurFilteringData_10) / \
                                               sizeof(FMSocWCN2243spurFilteringData_10[0]))

static const unsigned char FMSocWCN2243spurFilteringData_11[] = \
{
   0x01, 0x39, 0xAD
};
#define FMQSOCPATCHES_SPUR_FREQ_11_ADDR        (0x0BD9)
#define FMQSOCPATCHES_SPUR_FREQ_11_PTR         (&FMSocWCN2243spurFilteringData_11[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_11      (sizeof(FMSocWCN2243spurFilteringData_11) / \
                                               sizeof(FMSocWCN2243spurFilteringData_11[0]))

static const unsigned char FMSocWCN2243spurFilteringData_12[] = \
{
   0x01, 0x43, 0xAD
};
#define FMQSOCPATCHES_SPUR_FREQ_12_ADDR        (0x0BDC)
#define FMQSOCPATCHES_SPUR_FREQ_12_PTR         (&FMSocWCN2243spurFilteringData_12[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_12      (sizeof(FMSocWCN2243spurFilteringData_12) / \
                                               sizeof(FMSocWCN2243spurFilteringData_12[0]))

static const unsigned char FMSocWCN2243spurFilteringData_13[] = \
{
   0x01, 0x4D, 0xAD
};
#define FMQSOCPATCHES_SPUR_FREQ_13_ADDR        (0x0BDF)
#define FMQSOCPATCHES_SPUR_FREQ_13_PTR         (&FMSocWCN2243spurFilteringData_13[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_13      (sizeof(FMSocWCN2243spurFilteringData_13) / \
                                               sizeof(FMSocWCN2243spurFilteringData_13[0]))

static const unsigned char FMSocWCN2243spurFilteringData_14[] = \
{
   0x01, 0x8D, 0xBC
};
#define FMQSOCPATCHES_SPUR_FREQ_14_ADDR        (0x0BE2)
#define FMQSOCPATCHES_SPUR_FREQ_14_PTR         (&FMSocWCN2243spurFilteringData_14[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_14      (sizeof(FMSocWCN2243spurFilteringData_14) / \
                                               sizeof(FMSocWCN2243spurFilteringData_14[0]))

static const unsigned char FMSocWCN2243spurFilteringData_15[] = \
{
   0x01, 0x8E, 0xBC
};
#define FMQSOCPATCHES_SPUR_FREQ_15_ADDR        (0x0BE5)
#define FMQSOCPATCHES_SPUR_FREQ_15_PTR         (&FMSocWCN2243spurFilteringData_15[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_15      (sizeof(FMSocWCN2243spurFilteringData_15) / \
                                               sizeof(FMSocWCN2243spurFilteringData_15[0]))

static const unsigned char FMSocWCN2243spurFilteringData_16[] = \
{
   0x01, 0x8F, 0xBC
};
#define FMQSOCPATCHES_SPUR_FREQ_16_ADDR        (0x0BE8)
#define FMQSOCPATCHES_SPUR_FREQ_16_PTR         (&FMSocWCN2243spurFilteringData_16[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_16      (sizeof(FMSocWCN2243spurFilteringData_16) / \
                                               sizeof(FMSocWCN2243spurFilteringData_16[0]))

static const unsigned char FMSocWCN2243spurFilteringData_17[] = \
{
   0x01, 0x90, 0xBC
};
#define FMQSOCPATCHES_SPUR_FREQ_17_ADDR        (0x0BEB)
#define FMQSOCPATCHES_SPUR_FREQ_17_PTR         (&FMSocWCN2243spurFilteringData_17[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_17      (sizeof(FMSocWCN2243spurFilteringData_17) / \
                                               sizeof(FMSocWCN2243spurFilteringData_17[0]))

static const unsigned char FMSocWCN2243spurFilteringData_18[] = \
{
   0x01, 0x91, 0xBC
};
#define FMQSOCPATCHES_SPUR_FREQ_18_ADDR        (0x0BEE)
#define FMQSOCPATCHES_SPUR_FREQ_18_PTR         (&FMSocWCN2243spurFilteringData_18[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_18      (sizeof(FMSocWCN2243spurFilteringData_18) / \
                                               sizeof(FMSocWCN2243spurFilteringData_18[0]))

static const unsigned char FMSocWCN2243spurFilteringData_19[] = \
{
   0x02, 0x0D, 0xB5
};
#define FMQSOCPATCHES_SPUR_FREQ_19_ADDR        (0x0BF1)
#define FMQSOCPATCHES_SPUR_FREQ_19_PTR         (&FMSocWCN2243spurFilteringData_19[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_19      (sizeof(FMSocWCN2243spurFilteringData_19) / \
                                               sizeof(FMSocWCN2243spurFilteringData_19[0]))

static const unsigned char FMSocWCN2243spurFilteringData_20[] = \
{
   0x02, 0x0E, 0xB5
};
#define FMQSOCPATCHES_SPUR_FREQ_20_ADDR        (0x0BF4)
#define FMQSOCPATCHES_SPUR_FREQ_20_PTR         (&FMSocWCN2243spurFilteringData_20[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_20      (sizeof(FMSocWCN2243spurFilteringData_20) / \
                                               sizeof(FMSocWCN2243spurFilteringData_20[0]))

static const unsigned char FMSocWCN2243spurFilteringData_21[] = \
{
   0x02, 0x0F, 0xB5
};
#define FMQSOCPATCHES_SPUR_FREQ_21_ADDR        (0x0BF7)
#define FMQSOCPATCHES_SPUR_FREQ_21_PTR         (&FMSocWCN2243spurFilteringData_21[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_21      (sizeof(FMSocWCN2243spurFilteringData_21) / \
                                               sizeof(FMSocWCN2243spurFilteringData_21[0]))

static const unsigned char FMSocWCN2243spurFilteringData_22[] = \
{
   0x02, 0x10, 0xB5
};
#define FMQSOCPATCHES_SPUR_FREQ_22_ADDR        (0x0BFA)
#define FMQSOCPATCHES_SPUR_FREQ_22_PTR         (&FMSocWCN2243spurFilteringData_22[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_22      (sizeof(FMSocWCN2243spurFilteringData_22) / \
                                               sizeof(FMSocWCN2243spurFilteringData_22[0]))

static const unsigned char FMSocWCN2243spurFilteringData_23[] = \
{
   0x02, 0x11, 0xB5
};
#define FMQSOCPATCHES_SPUR_FREQ_23_ADDR        (0x0BFD)
#define FMQSOCPATCHES_SPUR_FREQ_23_PTR         (&FMSocWCN2243spurFilteringData_23[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_23      (sizeof(FMSocWCN2243spurFilteringData_23) / \
                                               sizeof(FMSocWCN2243spurFilteringData_23[0]))

static const unsigned char FMSocWCN2243spurFilteringData_24[] = \
{
   0x00, 0xCE, 0xA0
};
#define FMQSOCPATCHES_SPUR_FREQ_24_ADDR        (0x0C00)
#define FMQSOCPATCHES_SPUR_FREQ_24_PTR         (&FMSocWCN2243spurFilteringData_24[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_24      (sizeof(FMSocWCN2243spurFilteringData_24) / \
                                               sizeof(FMSocWCN2243spurFilteringData_24[0]))

static const unsigned char FMSocWCN2243spurFilteringData_25[] = \
{
   0x00, 0xCF, 0xA0
};
#define FMQSOCPATCHES_SPUR_FREQ_25_ADDR        (0x0C03)
#define FMQSOCPATCHES_SPUR_FREQ_25_PTR         (&FMSocWCN2243spurFilteringData_25[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_25      (sizeof(FMSocWCN2243spurFilteringData_25) / \
                                               sizeof(FMSocWCN2243spurFilteringData_25[0]))

static const unsigned char FMSocWCN2243spurFilteringData_26[] = \
{
   0x00, 0xD0, 0xA0
};
#define FMQSOCPATCHES_SPUR_FREQ_26_ADDR        (0x0C06)
#define FMQSOCPATCHES_SPUR_FREQ_26_PTR         (&FMSocWCN2243spurFilteringData_26[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_26      (sizeof(FMSocWCN2243spurFilteringData_26) / \
                                               sizeof(FMSocWCN2243spurFilteringData_26[0]))

static const unsigned char FMSocWCN2243spurFilteringData_27[] = \
{
   0x01, 0x00, 0x9B
};
#define FMQSOCPATCHES_SPUR_FREQ_27_ADDR        (0x0C09)
#define FMQSOCPATCHES_SPUR_FREQ_27_PTR         (&FMSocWCN2243spurFilteringData_27[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_27      (sizeof(FMSocWCN2243spurFilteringData_27) / \
                                               sizeof(FMSocWCN2243spurFilteringData_27[0]))

static const unsigned char FMSocWCN2243spurFilteringData_28[] = \
{
   0x01, 0x60, 0x9C
};
#define FMQSOCPATCHES_SPUR_FREQ_28_ADDR        (0x0C0C)
#define FMQSOCPATCHES_SPUR_FREQ_28_PTR         (&FMSocWCN2243spurFilteringData_28[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_28      (sizeof(FMSocWCN2243spurFilteringData_28) / \
                                               sizeof(FMSocWCN2243spurFilteringData_28[0]))

static const unsigned char FMSocWCN2243spurFilteringData_29[] = \
{
   0x01, 0xF0, 0xA0
};
#define FMQSOCPATCHES_SPUR_FREQ_29_ADDR        (0x0C0F)
#define FMQSOCPATCHES_SPUR_FREQ_29_PTR         (&FMSocWCN2243spurFilteringData_29[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_29      (sizeof(FMSocWCN2243spurFilteringData_29) / \
                                               sizeof(FMSocWCN2243spurFilteringData_29[0]))

static const unsigned char FMSocWCN2243spurFilteringData_30[] = \
{
   0x02, 0x50, 0x9F
};
#define FMQSOCPATCHES_SPUR_FREQ_30_ADDR        (0x0C12)
#define FMQSOCPATCHES_SPUR_FREQ_30_PTR         (&FMSocWCN2243spurFilteringData_30[0])
#define SIZEOF_FMQSOCPATCHES_SPUR_FREQ_30      (sizeof(FMSocWCN2243spurFilteringData_30) / \
                                               sizeof(FMSocWCN2243spurFilteringData_30[0]))

static const tsFmQSocPatchesSpurPokeType FMQSocSpurData[] = \
{
  {FMQSOCPATCHES_SPUR_FREQ_0_PTR,  FMQSOCPATCHES_SPUR_FREQ_0_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_0},
  {FMQSOCPATCHES_SPUR_FREQ_1_PTR,  FMQSOCPATCHES_SPUR_FREQ_1_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_1},
  {FMQSOCPATCHES_SPUR_FREQ_2_PTR,  FMQSOCPATCHES_SPUR_FREQ_2_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_2},
  {FMQSOCPATCHES_SPUR_FREQ_3_PTR,  FMQSOCPATCHES_SPUR_FREQ_3_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_3},
  {FMQSOCPATCHES_SPUR_FREQ_4_PTR,  FMQSOCPATCHES_SPUR_FREQ_4_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_4},
  {FMQSOCPATCHES_SPUR_FREQ_5_PTR,  FMQSOCPATCHES_SPUR_FREQ_5_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_5},
  {FMQSOCPATCHES_SPUR_FREQ_6_PTR,  FMQSOCPATCHES_SPUR_FREQ_6_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_6},
  {FMQSOCPATCHES_SPUR_FREQ_7_PTR,  FMQSOCPATCHES_SPUR_FREQ_7_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_7},
  {FMQSOCPATCHES_SPUR_FREQ_8_PTR,  FMQSOCPATCHES_SPUR_FREQ_8_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_8},
  {FMQSOCPATCHES_SPUR_FREQ_9_PTR,  FMQSOCPATCHES_SPUR_FREQ_9_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_9},
  {FMQSOCPATCHES_SPUR_FREQ_10_PTR, FMQSOCPATCHES_SPUR_FREQ_10_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_10},
  {FMQSOCPATCHES_SPUR_FREQ_11_PTR, FMQSOCPATCHES_SPUR_FREQ_11_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_11},
  {FMQSOCPATCHES_SPUR_FREQ_12_PTR, FMQSOCPATCHES_SPUR_FREQ_12_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_12},
  {FMQSOCPATCHES_SPUR_FREQ_13_PTR, FMQSOCPATCHES_SPUR_FREQ_13_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_13},
  {FMQSOCPATCHES_SPUR_FREQ_14_PTR, FMQSOCPATCHES_SPUR_FREQ_14_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_14},
  {FMQSOCPATCHES_SPUR_FREQ_15_PTR, FMQSOCPATCHES_SPUR_FREQ_15_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_15},
  {FMQSOCPATCHES_SPUR_FREQ_16_PTR, FMQSOCPATCHES_SPUR_FREQ_16_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_16},
  {FMQSOCPATCHES_SPUR_FREQ_17_PTR, FMQSOCPATCHES_SPUR_FREQ_17_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_17},
  {FMQSOCPATCHES_SPUR_FREQ_18_PTR, FMQSOCPATCHES_SPUR_FREQ_18_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_18},
  {FMQSOCPATCHES_SPUR_FREQ_19_PTR, FMQSOCPATCHES_SPUR_FREQ_19_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_19},
  {FMQSOCPATCHES_SPUR_FREQ_20_PTR, FMQSOCPATCHES_SPUR_FREQ_20_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_20},
  {FMQSOCPATCHES_SPUR_FREQ_21_PTR, FMQSOCPATCHES_SPUR_FREQ_21_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_21},
  {FMQSOCPATCHES_SPUR_FREQ_22_PTR, FMQSOCPATCHES_SPUR_FREQ_22_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_22},
  {FMQSOCPATCHES_SPUR_FREQ_23_PTR, FMQSOCPATCHES_SPUR_FREQ_23_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_23},
  {FMQSOCPATCHES_SPUR_FREQ_24_PTR, FMQSOCPATCHES_SPUR_FREQ_24_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_24},
  {FMQSOCPATCHES_SPUR_FREQ_25_PTR, FMQSOCPATCHES_SPUR_FREQ_25_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_25},
  {FMQSOCPATCHES_SPUR_FREQ_26_PTR, FMQSOCPATCHES_SPUR_FREQ_26_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_26},
  {FMQSOCPATCHES_SPUR_FREQ_27_PTR, FMQSOCPATCHES_SPUR_FREQ_27_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_27},
  {FMQSOCPATCHES_SPUR_FREQ_28_PTR, FMQSOCPATCHES_SPUR_FREQ_28_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_28},
  {FMQSOCPATCHES_SPUR_FREQ_29_PTR, FMQSOCPATCHES_SPUR_FREQ_29_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_29},
  {FMQSOCPATCHES_SPUR_FREQ_30_PTR, FMQSOCPATCHES_SPUR_FREQ_30_ADDR,
    SIZEOF_FMQSOCPATCHES_SPUR_FREQ_30},
};
#define SIZEOF_FMQSOCPATCHES_SPURDATA    (sizeof(FMQSocSpurData) / \
                                            sizeof(FMQSocSpurData[0]))

int FmQSocCom_Poke (int fd, const tsFmQSocPatchesSpurPokeType *spur_data);
int spurFrequencyFilter(int fd)
{
  int retval = 0;
  unsigned char i = 0;
  unsigned char spurData[XFR_REG_NUM] = {0};
  tsFmQSocPatchesSpurPokeType tsFmQSocPatchesSpurPokeType;

  /* Configure the SPUR Table Size */
  spurData[0] = SIZEOF_FMQSOCPATCHES_SPURDATA;
  tsFmQSocPatchesSpurPokeType.spurPtr = spurData;
  tsFmQSocPatchesSpurPokeType.spurPokeAddr = SPUR_TABLE_REG_ADDR;
  tsFmQSocPatchesSpurPokeType.spurSize = 1;
  /* Poke the SPUR Table Size */
  fprintf(debug_file, "\n\t\tConfiguring the SPUR FREQUENCY TABLE SIZE...\n");
  retval = FmQSocCom_Poke(fd, &tsFmQSocPatchesSpurPokeType);
  if (retval < 0) {
    fprintf(debug_file, "spurFrequencyFilter: Error in poking SPUR FREQ TABLE"
      " SIZE\n");
    return retval;
  } else
    fprintf(debug_file, "spurFrequencyFilter: SPUR FREQ TABLE Size(%d) "
      "configured successfully\n", spurData[0]);

  /*Poke the Spur Frequency data into respective registers */
  fprintf(debug_file, "\n\n\t\tConfiguring the SPUR FREQUENCY Data...\n\n");
  for (i = 0; i < SIZEOF_FMQSOCPATCHES_SPURDATA; i++) {
    fprintf(debug_file, "Spur Freq [%d] started downloading\n", i);
    /* Apply the poke to the HW */
    retval = FmQSocCom_Poke(fd, &FMQSocSpurData[i]);
    if (retval < 0) {
      fprintf(debug_file, "spurFrequencyFilter: Error in poking "
        "SPUR FREQ [%d]\n", i);
      return retval;
    }
    fprintf(debug_file, "Spur Freq [%d] download success\n\n", i);
  }
  fprintf(debug_file, "Spur Table download success\n");
  return retval;
}

  /*-------------------------------------------------------------------------*/
  /*                            FM6500 1.0 Patches                           */
  /*-------------------------------------------------------------------------*/

static const unsigned char FMQSocA0Patch0[] = \
{
  0x00, 0x5B, 0x0F, 0xF8, 0x00, 0x06, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /***************************************************************************/
  /* PATCH 0: This patches the Tx Frequency 1-bit vs. 2-bit SDM select       */
  /* Description: Tx Frequency 1-bit vs. 2-bit SDM Mode                      */
  /***************************************************************************/

  0x02, 0xF8, 0x06, 0x02, 0xF8, 0x0C, 0x12, 0xB1, 0xB2, 0xE4, 0x80, 0x02, 0x74,
  0x08, 0x90, 0x80, 0x5A, 0xF0, 0x90, 0x0A, 0x94, 0x02, 0x5B, 0x15
};

#define FMQSOC_A0_PATCH0_PTR     (&FMQSocA0Patch0[0])
#define SIZEOF_FMQSOC_A0_PATCH0  (sizeof(FMQSocA0Patch0))

static const unsigned char FMQSocA0Patch1[] = \
{
  0x01, 0x5A, 0x45, 0xF8, 0x20, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /***************************************************************************/
  /* PATCH 1: This patches the PLL Frequency Under-Flow Condition            */
  /* Description: Tune 76.00 & 76.05 Japan Band and 87.50 US Band (1 of 2)   */
  /***************************************************************************/

  0x02, 0xF8, 0x23, 0xBC, 0xFF, 0x23, 0xBD, 0xFE, 0x04, 0x7C, 0x28, 0x80, 0x02,
  0x7C, 0x34, 0x90, 0x0A, 0x87, 0x74, 0x02, 0xF0, 0xA3, 0x74, 0x06, 0xF0, 0xA3,
  0x74, 0x11, 0xF0, 0xA3, 0xE4, 0xF0, 0xA3, 0xEC, 0xF0, 0xA3, 0xE4, 0xF0, 0xA3,
  0xF0, 0x22, 0x12, 0xB0, 0x59, 0x02, 0x5A, 0x48
};

#define FMQSOC_A0_PATCH1_PTR     (&FMQSocA0Patch1[0])
#define SIZEOF_FMQSOC_A0_PATCH1  (sizeof(FMQSocA0Patch1))

static const unsigned char FMQSocA0Patch2[] = \
{
  0x02, 0x5D, 0xC5, 0xF8, 0x60, 0x02, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /***************************************************************************/
  /* PATCH 2: This patches the PLL Frequency Under-Flow Condition            */
  /* Description: Tune 76.00 & 76.05 Japan Band and 87.50 US Band (2 of 2)   */
  /***************************************************************************/

  0x24, 0x00
};

#define FMQSOC_A0_PATCH2_PTR     (&FMQSocA0Patch2[0])
#define SIZEOF_FMQSOC_A0_PATCH2  (sizeof(FMQSocA0Patch2))

static const unsigned char FMQSocA0Patch3[] = \
{
  0x03, 0x6C, 0x62, 0xF8, 0x90, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /***************************************************************************/
  /* PATCH 3: Check if RDS sync before calling RISMA algorithm               */
  /* Description: Check RDS Sync Before Enable RISMA Algorithm               */
  /***************************************************************************/

  0x02, 0xF8, 0x93, 0x90, 0x0A, 0xDF, 0xE0, 0xB4, 0x01, 0x03, 0x12, 0x21, 0x97,
  0x02, 0x6C, 0x65
};

#define FMQSOC_A0_PATCH3_PTR     (&FMQSocA0Patch3[0])
#define SIZEOF_FMQSOC_A0_PATCH3  (sizeof(FMQSocA0Patch3))

static const unsigned char FMQSocA0Patch5[] = \
{
  0x05, 0x93, 0x65, 0xF9, 0x10, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /***************************************************************************/
  /* PATCH 5: Enable Temperature Calibration Timer                           */
  /* Description: Enable Temperatue Calibration Timer                        */
  /***************************************************************************/

  0x02, 0xF9, 0x13, 0x12, 0xB4, 0x9F, 0x90, 0x04, 0x2B, 0xE0, 0xFC, 0xA3, 0xE0,
  0xFD, 0x7F, 0x0A, 0x12, 0x37, 0xC8, 0x02, 0x93, 0x68

};

#define FMQSOC_A0_PATCH5_PTR     (&FMQSocA0Patch5[0])
#define SIZEOF_FMQSOC_A0_PATCH5  (sizeof(FMQSocA0Patch5))

static const unsigned char FMQSocA0Patch6A[] = \
{
  0x06, 0x5B, 0xC1, 0xF9, 0x40, 0x03, 0x00, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /***************************************************************************/
  /* PATCH 6A: Charge Pump Setting for LO Frequencies Below 76 MHz           */
  /* Description: Charge Pump Setting for 76.00 & 76.05 MHz                  */
  /***************************************************************************/

  /* Patch Array 0 */
  0x02, 0xF9, 0x43, 0x90, 0x0A, 0x8E, 0xE0, 0x94, 0xFF, 0x70, 0x38, 0xA3, 0xE0,
  0x94, 0xFE, 0x70, 0x14, 0x90, 0x0A, 0x97, 0x74, 0x42, 0xF0, 0xA3, 0x74, 0x97,
  0xF0, 0xA3, 0x74, 0xCC, 0xF0, 0xA3, 0x74, 0xCD, 0xF0, 0x80, 0x17, 0xE0, 0x94,
  0xFF, 0x70, 0x19, 0x90, 0x0A, 0x97, 0x74, 0x42, 0xF0, 0xA3, 0x74, 0x97

};

#define FMQSOC_A0_PATCH6A_PTR     (&FMQSocA0Patch6A[0])
#define SIZEOF_FMQSOC_A0_PATCH6A  (sizeof(FMQSocA0Patch6A))

static const unsigned char FMQSocA0Patch6B[] = \
{
  0x06, 0x5B, 0xC1, 0xF9, 0x40, 0x03, 0x01, 0x01,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /***************************************************************************/
  /* PATCH 6B: Charge Pump Setting for LO Frequencies Below 76 MHz           */
  /* Description: Charge Pump Setting for 76.00 & 76.05 MHz                  */
  /***************************************************************************/

  /* Patch Array 1 */
  0xF0, 0xA3, 0x74, 0xE6, 0xF0, 0xA3, 0x74, 0x66, 0xF0, 0x90, 0x0A, 0x8E, 0xE4,
  0xF0, 0xA3, 0xF0, 0x12, 0x57, 0x3B, 0x02, 0x5B, 0xC4

};

#define FMQSOC_A0_PATCH6B_PTR     (&FMQSocA0Patch6B[0])
#define SIZEOF_FMQSOC_A0_PATCH6B  (sizeof(FMQSocA0Patch6B))

/**
* Array of pointers to FM QSOC patch related data.
*/
static const unsigned char *FMQSocPatchA0Data[] = \
{
  FMQSOC_A0_PATCH0_PTR
  ,FMQSOC_A0_PATCH1_PTR
  ,FMQSOC_A0_PATCH2_PTR
  ,FMQSOC_A0_PATCH3_PTR
  ,FMQSOC_A0_PATCH5_PTR
  ,FMQSOC_A0_PATCH6A_PTR
  ,FMQSOC_A0_PATCH6B_PTR

};

/**
* Array that stores sizes of FM QSOC patch related data.
*/
static const unsigned char FMQSocPatchA0Size[] = \
{
  SIZEOF_FMQSOC_A0_PATCH0
  ,SIZEOF_FMQSOC_A0_PATCH1
  ,SIZEOF_FMQSOC_A0_PATCH2
  ,SIZEOF_FMQSOC_A0_PATCH3
  ,SIZEOF_FMQSOC_A0_PATCH5
  ,SIZEOF_FMQSOC_A0_PATCH6A
  ,SIZEOF_FMQSOC_A0_PATCH6B

};

#define SIZEOF_FMQSOC_PATCHA0DATA  (sizeof(FMQSocPatchA0Data) / \
                                    sizeof(&FMQSocPatchA0Data[0]))

  /*-------------------------------------------------------------------------*/
  /*                            FM6500 2.0 Patches                           */
  /*-------------------------------------------------------------------------*/

  /***************************************************************************/
  /* PATCH 0: This configures the I2C interrupt so that it is no longer      */
  /* edge detect, but level detect.  This prevents issues where the          */
  /* FM SoC appears to be unresponsive to I2C writes.                         */
  /***************************************************************************/
static const unsigned char FMQSocPatchesV20Patch0[] = \
{
  0x00, 0x1E, 0x08, 0xF8, 0x15, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x18, 0xC2, 0x88, 0xC2, 0x8A, 0x12,
  0x27, 0xE8, 0x02, 0x1E, 0x0B
};

#define FMQSOCPATCHES_20_PATCH0_PTR     (&FMQSocPatchesV20Patch0[0])
#define SIZEOF_FMQSOCPATCHES_20_PATCH0  (sizeof(FMQSocPatchesV20Patch0))


/***************************************************************************/
/* PATCH 1: This allows search for weakest stations to find stations below */
/* the search threshold.                                                   */
/***************************************************************************/
static const unsigned char FMQSocPatchesV20Patch1[] = \
{
  0x01, 0x6D, 0x5E, 0xF8, 0x22, 0x01, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x40
};

#define FMQSOCPATCHES_20_PATCH1_PTR     (&FMQSocPatchesV20Patch1[0])
#define SIZEOF_FMQSOCPATCHES_20_PATCH1  (sizeof(FMQSocPatchesV20Patch1))

/***************************************************************************/
/* PATCH 2: This fixes an issue where the RDS HW block does not lose sync. */
/***************************************************************************/
static const unsigned char FMQSocPatchesV20Patch2[] = \
{
  0x02, 0x45, 0x5C, 0xF8, 0x26, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x29, 0x90, 0x07, 0xCC, 0xE0, 0x90, 0x0A, 0x62,
  0x70, 0x0B, 0xE0, 0x04, 0xF0, 0xD3, 0x94, 0x10, 0x40, 0x05,
  0x12, 0x8D, 0xF3, 0xE4, 0xF0, 0x12, 0x05, 0x72, 0x02, 0x45,
  0x5F
};

#define FMQSOCPATCHES_20_PATCH2_PTR     (&FMQSocPatchesV20Patch2[0])
#define SIZEOF_FMQSOCPATCHES_20_PATCH2  (sizeof(FMQSocPatchesV20Patch2))

/***************************************************************************/
/* PATCH 3: FM SoC doesn't handle I2C contention.                          */
/***************************************************************************/
static const unsigned char FMQSocPatchesV20Patch3[] = \
{
  0x02, 0x26, 0xF9, 0xF8, 0x46, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x49, 0x12, 0xAC, 0xA1, 0xA3, 0xE0, 0xFF, 0xA3,
  0xE0, 0xFD, 0x90, 0x8D, 0x00, 0x8F, 0x82, 0xED, 0xF0, 0x90,
  0x8C, 0x0C, 0xEF, 0xC3, 0x13, 0xC3, 0x13, 0xC3, 0x13, 0xC3,
  0x24, 0x0C, 0xF5, 0x82, 0xE0, 0x70, 0xE2, 0x02, 0x27, 0x4B
};

#define FMQSOCPATCHES_20_PATCH3_PTR     (&FMQSocPatchesV20Patch3[0])
#define SIZEOF_FMQSOCPATCHES_20_PATCH3  (sizeof(FMQSocPatchesV20Patch3))

/***************************************************************************/
/* PATCH 4: Disable 8051 idle mode to remove spurs.                        */
/***************************************************************************/
static const unsigned char FMQSocPatchesV20Patch4[] = \
{
  0x03, 0x1E, 0x36, 0xF8, 0x6E, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x00, 0x00, 0x00
};

#define FMQSOCPATCHES_20_PATCH4_PTR     (&FMQSocPatchesV20Patch4[0])
#define SIZEOF_FMQSOCPATCHES_20_PATCH4  (sizeof(FMQSocPatchesV20Patch4))

/***************************************************************************/
/* PATCH 5: FM65 2.0 LO shift does not happen using I2C registers when SoC */
/* is already tuned to forbidden frequency.                                */
/***************************************************************************/
static const unsigned char FMQSocPatchesV20Patch5[] = \
{
  0x04, 0x2B, 0xD4, 0xF8, 0x71, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x74, 0x90, 0x07, 0xD8, 0xE4, 0x75,
  0xF0, 0xE6, 0x12, 0x0A, 0x26, 0x02, 0x2B, 0xFD
};

#define FMQSOCPATCHES_20_PATCH5_PTR     (&FMQSocPatchesV20Patch5[0])
#define SIZEOF_FMQSOCPATCHES_20_PATCH5  (sizeof(FMQSocPatchesV20Patch5))

/**
* Array of pointers to FM QSOC patch related data.
*/
static const unsigned char *FMQSocPatchesV20Data[] = \
{
   FMQSOCPATCHES_20_PATCH0_PTR
  ,FMQSOCPATCHES_20_PATCH1_PTR
  ,FMQSOCPATCHES_20_PATCH2_PTR
  ,FMQSOCPATCHES_20_PATCH3_PTR
  ,FMQSOCPATCHES_20_PATCH4_PTR
  ,FMQSOCPATCHES_20_PATCH5_PTR
};

/**
* Array that stores sizes of FM QSOC patch related data.
*/
static const unsigned char FMQSocPatchesV20Size[] = \
{
   SIZEOF_FMQSOCPATCHES_20_PATCH0
  ,SIZEOF_FMQSOCPATCHES_20_PATCH1
  ,SIZEOF_FMQSOCPATCHES_20_PATCH2
  ,SIZEOF_FMQSOCPATCHES_20_PATCH3
  ,SIZEOF_FMQSOCPATCHES_20_PATCH4
  ,SIZEOF_FMQSOCPATCHES_20_PATCH5
};

#define SIZEOF_FMQSOCPATCHES_PATCHV20DATA  (sizeof(FMQSocPatchesV20Data) / \
                                            sizeof(&FMQSocPatchesV20Data[0]))

  /*-------------------------------------------------------------------------*/
  /*                            FM6500 2.1 Patches                           */
  /*-------------------------------------------------------------------------*/

  /***************************************************************************/
  /* PATCH 0: Disable 8051 idle mode to remove spurs.                        */
  /***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch0[] = \
{
  0x00, 0x1E, 0x36, 0xF8, 0x00, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x00, 0x00, 0x00
};

#define FMQSOCPATCHES_21_PATCH0_PTR     (&FMQSocPatchesV21Patch0[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH0  (sizeof(FMQSocPatchesV21Patch0))


  /***************************************************************************/
  /* PATCH 1: FM65 2.1 LO shift does not happen using I2C registers when SoC */
  /* is already tuned to forbidden frequency.                                */
  /***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch1[] = \
{
  0x01, 0x66, 0x1E, 0xF8, 0x03, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x06, 0x90, 0x07, 0xDC, 0xE4, 0x75,
  0xF0, 0xE6, 0x12, 0x0A, 0x26, 0x02, 0x66, 0x49
};

#define FMQSOCPATCHES_21_PATCH1_PTR     (&FMQSocPatchesV21Patch1[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH1  (sizeof(FMQSocPatchesV21Patch1))

/***************************************************************************/
/* PATCH 2: FM65 2.1 does not update FREQ during searches. This is         */
/* required for WM7.                                                       */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch2[] = \
{
  0x02, 0x67, 0x3E, 0xF8, 0x13, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x16, 0x90, 0x09, 0x40, 0xE0, 0xFE,
  0xA3, 0xE0, 0xFF, 0x12, 0x7E, 0xF7, 0x90, 0x09,
  0x3C, 0x02, 0x67, 0x41
};

#define FMQSOCPATCHES_21_PATCH2_PTR     (&FMQSocPatchesV21Patch2[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH2  (sizeof(FMQSocPatchesV21Patch2))

/***************************************************************************/
/* PATCH 3a: FM65 2.1 searches incorrectly find adjacent frequencies       */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch3a[] = \
{
  0x03, 0xA6, 0xA1, 0xF9, 0xFB, 0x03, 0x00, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF9, 0xFE, 0x90, 0x09, 0x3C, 0xE0, 0xB4,
  0x03, 0x14, 0x90, 0x03, 0xF7, 0xE0, 0xFF, 0x90,
  0x07, 0x85, 0xE0, 0xFE, 0xC3, 0x12, 0xAE, 0x2A,
  0x40, 0x02, 0xD3, 0x22, 0xC3, 0x22, 0x90, 0x0B,
  0x87, 0xE0, 0x64, 0x01, 0x60, 0x41, 0x90, 0x0B,
  0x85, 0xE0, 0xFF, 0x90, 0x07, 0x85, 0x12, 0xAE,
  0x27, 0x40, 0x0B

};

#define FMQSOCPATCHES_21_PATCH3A_PTR     (&FMQSocPatchesV21Patch3a[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH3A  (sizeof(FMQSocPatchesV21Patch3a))

/***************************************************************************/
/* PATCH 3b: FM65 2.1 searches incorrectly find adjacent frequencies       */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch3b[] = \
{
  0x03, 0xA6, 0xA1, 0xF9, 0xFB, 0x03, 0x00, 0x01,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xEE, 0x9F, 0xFF, 0x90, 0x0B, 0x86, 0xE0, 0x2F,
  0xF0, 0x80, 0x29, 0x90, 0x0B, 0x86, 0xE0, 0xC3,
  0x64, 0x80, 0x94, FMQSOCPATCHES_SEARCH_RMSSI_DELTA, 0x40, 0x19, 0xA3, 0x74,
  0x01, 0xF0, 0x12, 0xFA, 0xE7, 0x60, 0x08, 0x12,
  0x68, 0x00, 0x12, 0x68, 0x00, 0x80, 0x06, 0x12,
  0xA8, 0x00, 0x12, 0xA8, 0x00, 0xC3, 0x22, 0xE4,
  0x90, 0x0B, 0x86
};

#define FMQSOCPATCHES_21_PATCH3B_PTR     (&FMQSocPatchesV21Patch3b[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH3B  (sizeof(FMQSocPatchesV21Patch3b))

/***************************************************************************/
/* PATCH 3c: FM65 2.1 searches incorrectly find adjacent frequencies       */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch3c[] = \
{
  0x03, 0xA6, 0xA1, 0xF9, 0xFB, 0x03, 0x00, 0x02,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xF0, 0x90, 0x07, 0x85, 0xE0, 0xFF, 0x90, 0x0B,
  0x85, 0xF0, 0x90, 0x0B, 0x87, 0xE0, 0xB4, 0x01,
  0x08, 0xE4, 0xF0, 0x90, 0x0B, 0x86, 0xF0, 0x80,
  0x41, 0x12, 0xFA, 0xE7, 0x60, 0x11, 0x12, 0xFA,
  0xEF, 0xFD, 0xC3, 0x90, 0x09, 0x41, 0xE0, 0x90,
  0x09, 0x40, 0x12, 0xFA, 0xF7, 0x40, 0x15, 0x12,
  0xFA, 0xE7, 0x70
};

#define FMQSOCPATCHES_21_PATCH3C_PTR     (&FMQSocPatchesV21Patch3c[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH3C  (sizeof(FMQSocPatchesV21Patch3c))

/***************************************************************************/
/* PATCH 3d: FM65 2.1 searches incorrectly find adjacent frequencies       */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch3d[] = \
{
  0x03, 0xA6, 0xA1, 0xF9, 0xFB, 0x03, 0x00, 0x03,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x24, 0x12, 0xFB, 0x08, 0xC3, 0x90, 0x09, 0x49,
  0xE0, 0x90, 0x09, 0x48, 0x12, 0xFA, 0xF7, 0x50,
  0x14, 0x90, 0x0B, 0x86, 0xE0, 0xD3, 0x64, 0x80,
  0x94, FMQSOCPATCHES_SEARCH_RMSSI_DELTA, 0x40, 0x04, 0xE4, 0xF0, FMQSOCPATCHES_SEARCH_BANDEDGE_CHK,
  0xE4, 0x90, 0x0B, 0x86, 0xF0, 0xC3, 0x22, 0x90,
  0x03, 0xF7, 0xE0, 0xC3, 0x12, 0xB2, 0xC0, 0x40,
  0x20, 0x90, 0x03
};

#define FMQSOCPATCHES_21_PATCH3D_PTR     (&FMQSocPatchesV21Patch3d[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH3D  (sizeof(FMQSocPatchesV21Patch3d))

/***************************************************************************/
/* PATCH 3e: FM65 2.1 searches incorrectly find adjacent frequencies       */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch3e[] = \
{
  0x03, 0xA6, 0xA1, 0xF9, 0xFB, 0x03, 0x00, 0x04,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xFB, 0xE0, 0xFF, 0x90, 0x07, 0x86, 0xE0, 0xFE,
  0x12, 0xAE, 0x2A, 0x50, 0x11, 0x90, 0x88, 0x26,
  0xE0, 0xFF, 0xC3, 0x94, FMQSOCPATCHES_SEARCH_INFDETOUT_37P5_LO, 0x40, 0x07, 0xEF,
  0xD3, 0x94, FMQSOCPATCHES_SEARCH_INFDETOUT_37P5_HI, 0x50, 0x01, 0x22, 0xC3, 0x22,
  0x90, 0x09, 0x3B, 0xE0, 0x54, 0x01, 0x00, 0x22,
  0x90, 0x09, 0x46, 0xE0, 0xFC, 0xA3, 0xE0, 0x22,
  0x9D, 0xFD, 0xE0
};

#define FMQSOCPATCHES_21_PATCH3E_PTR     (&FMQSocPatchesV21Patch3e[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH3E  (sizeof(FMQSocPatchesV21Patch3e))

/***************************************************************************/
/* PATCH 3f: FM65 2.1 searches incorrectly find adjacent frequencies       */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch3f[] = \
{
  0x03, 0xA6, 0xA1, 0xF9, 0xFB, 0x03, 0x01, 0x05,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x9C, 0xFC, 0x90, 0x09, 0x3D, 0xE0, 0xFB, 0xC3,
  0xED, 0x9B, 0xEC, 0x94, 0x00, 0x22, 0x90, 0x09,
  0x40, 0xE0, 0xFC, 0xA3, 0xE0, 0xFD, 0x22
};

#define FMQSOCPATCHES_21_PATCH3F_PTR     (&FMQSocPatchesV21Patch3f[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH3F  (sizeof(FMQSocPatchesV21Patch3f))

/***************************************************************************/
/* PATCH 4: FM65 2.1 FW doesn't clear the GPIO status after an INTSTAT read*/
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch4[] = \
{
  0x04, 0x8A, 0x62, 0xF8, 0x40, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x43, 0x90, 0x94, 0x04, 0xE0, 0x64,
  0x01, 0xF0, 0x12, 0xBD, 0xD7, 0x02, 0x8A, 0x65
};

#define FMQSOCPATCHES_21_PATCH4_PTR     (&FMQSocPatchesV21Patch4[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH4  (sizeof(FMQSocPatchesV21Patch4))

/***************************************************************************/
/* PATCH 5: FM65 2.1 FW doesn't include all SPUR frequencies               */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch5[] = \
{
  0x05, 0x63, 0x7D, 0xF8, 0x61, 0x13, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x94, 0x19, 0x50, 0x20, 0xEF, 0x75, 0xF0, 0x03,
  0xA4, 0x24, 0x00, 0xF9, 0x74, 0x0C, 0x35, 0xF0,
  0xFA, 0x7B, 0x01
};

#define FMQSOCPATCHES_21_PATCH5_PTR     (&FMQSocPatchesV21Patch5[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH5  (sizeof(FMQSocPatchesV21Patch5))

/***************************************************************************/
/* PATCH 6: FM65 2.1 FW sometimes ignored I2C writes                       */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch6[] = \
{
  0x06, 0x5E, 0x4C, 0xF8, 0x74, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x12, 0xF8, 0x77, 0xFF, 0x12, 0xB0, 0x37, 0xF0,
  0x12, 0xAD, 0x0D, 0xE0, 0xFF, 0xA3, 0xE0, 0xFD,
  0x12, 0x5F, 0x37, 0x90, 0x00, 0x02, 0x02, 0x0F,
  0x2D
};

#define FMQSOCPATCHES_21_PATCH6_PTR     (&FMQSocPatchesV21Patch6[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH6  (sizeof(FMQSocPatchesV21Patch6))

/***************************************************************************/
/* PATCH 7: FM65 2.1 Searching not allowed in tuning state                 */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch7[] = \
{
  0x07, 0xA9, 0xC6, 0xF8, 0x8D, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x90, 0x24, 0xFD, 0x60, 0x03, 0x02,
  0xA9, 0xCA, 0x12, 0xAB, 0x78, 0xB4, 0x07, 0x14,
  0x12, 0xB9, 0x50, 0x40, 0x0A, 0x12, 0xAB, 0x68,
  0x7F, 0x04, 0x12, 0x7C, 0xE4, 0x80, 0x03, 0x12,
  0x1E, 0x46, 0x80, 0x08, 0x12, 0xAB, 0xD7, 0x7F,
  0x03, 0x12, 0x7C, 0xE4, 0x02, 0xAA, 0x09
};

#define FMQSOCPATCHES_21_PATCH7_PTR     (&FMQSocPatchesV21Patch7[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH7  (sizeof(FMQSocPatchesV21Patch7))

/***************************************************************************/
/* PATCH 8: FM65 2.1 FW incorrectly updates queue count                    */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch8[] = \
{
  0x08, 0xA2, 0xDB, 0xF9, 0x2C, 0x04, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x12, 0xF9, 0x40, 0x00, 0x74, 0x08, 0x9F, 0xFF,
  0xE0, 0x2F, 0xFF, 0x22, 0xE0, 0xFF, 0xA3, 0xE0,
  0xFE, 0xD3, 0x9F, 0x22, 0x90, 0x09, 0xBC, 0x12,
  0xF9, 0x38, 0x50, 0x05, 0xC3, 0xEF, 0x9E, 0xFF,
  0x22, 0x90, 0x09, 0xBD, 0xE0, 0xFF, 0xC3, 0x90,
  0x09, 0xBC, 0x02, 0xF9, 0x30
};

#define FMQSOCPATCHES_21_PATCH8_PTR     (&FMQSocPatchesV21Patch8[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH8  (sizeof(FMQSocPatchesV21Patch8))

/***************************************************************************/
/* PATCH 9: FM65 2.1 FW incorrectly updates queue count (2)                */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch9[] = \
{
  0x09, 0x24, 0xEC, 0xF9, 0x59, 0x04, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x12, 0xF9, 0x40, 0x00
};

#define FMQSOCPATCHES_21_PATCH9_PTR     (&FMQSocPatchesV21Patch9[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH9  (sizeof(FMQSocPatchesV21Patch9))

/***************************************************************************/
/* PATCH 10: Spur frequencies don't work for Hi-Side		           */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch10[] = \
{
  0x0A, 0x63, 0x45, 0xF9, 0x5D, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF9, 0x60, 0x90, 0x06, 0x62, 0xE0, 0x64,
  0x02, 0x70, 0x0A, 0x90, 0x07, 0xB3, 0xE0, 0xFF,
  0xC3, 0x74, 0x14, 0x9F, 0xF0, 0x90, 0x07, 0xB3,
  0x02, 0x63, 0x48
};

#define FMQSOCPATCHES_21_PATCH10_PTR     (&FMQSocPatchesV21Patch10[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH10  (sizeof(FMQSocPatchesV21Patch10))

/***************************************************************************/
/* PATCH 11: Ultimate SNR degraded at XO harmonic channels                 */
/* (enable spur rotator for G0)                                            */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch11[] = \
{
  0x0B, 0x45, 0x83, 0xF9, 0x78, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF9, 0x7B, 0xC3, 0x90, 0x89, 0x3A, 0xE0,
  0x54, 0x03, 0x70, 0x0D, 0x90, 0x89, 0x45, 0xE0,
  0x54, 0xEF, 0xF0, 0x12, 0x72, 0x17, 0x02, 0x45,
  0x86, 0x90, 0x89, 0x45, 0xE0, 0x44, 0x11, 0xF0,
  0x80, 0xF1
};

#define FMQSOCPATCHES_21_PATCH11_PTR     (&FMQSocPatchesV21Patch11[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH11  (sizeof(FMQSocPatchesV21Patch11))

/***************************************************************************/
/* PATCH 12a: Hi/Lo switch once for the newly tuned frequency (1/2)        */
/* Note: X:0x0B82-0x0B83 current freq                                      */
/* Note: X:0x0B84 HiLoFlag                                                 */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch12a[] = \
{
  0x0C, 0x80, 0x6C, 0xF9, 0x99, 0x03, 0x00, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF9, 0x9C, 0x7A, 0x07, 0x79, 0x8C, 0x12,
  0xAB, 0xFA, 0x90, 0x07, 0x8C, 0xE0, 0xFE, 0xA3,
  0xE0, 0xFF, 0x90, 0x0B, 0x82, 0xE0, 0x6E, 0xF8,
  0xA3, 0xE0, 0x6F, 0x48, 0x60, 0x05, 0xE4, 0x90,
  0x0B, 0x84, 0xF0, 0x90, 0x0B, 0x82, 0xEE, 0xF0,
  0xA3, 0xEF, 0xF0, 0x74, 0xA0, 0xFF, 0x90, 0x0B,
  0x86, 0xE4, 0xF0
};

#define FMQSOCPATCHES_21_PATCH12A_PTR     (&FMQSocPatchesV21Patch12a[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH12A  (sizeof(FMQSocPatchesV21Patch12a))

/***************************************************************************/
/* PATCH 12b: Hi/Lo switch once for the newly tuned frequency (1/2)        */
/* Note: X:0x0B82-0x0B83 current freq                                      */
/* Note: X:0x0B84 HiLoFlag                                                 */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch12b[] = \
{
  0x0C, 0x80, 0x6C, 0xF9, 0x99, 0x03, 0x01, 0x01,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xA3, 0xF0, 0x90, 0x88, 0xA8, 0xE0, 0x90, 0x0B,
  0x85, 0xF0, 0xD3, 0x9F, 0x50, 0x01, 0x22, 0x90,
  0x03, 0xFA, 0x02, 0x80, 0x6F
};

#define FMQSOCPATCHES_21_PATCH12B_PTR     (&FMQSocPatchesV21Patch12b[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH12B  (sizeof(FMQSocPatchesV21Patch12b))

/***************************************************************************/
/* PATCH 13: Hi/Lo switch once for the newly tuned frequency (2/2)        */
/* Note: X:0x0B82-0x0B83 current freq                                      */
/* Note: X:0x0B84 HiLoFlag                                                 */
/***************************************************************************/
static const unsigned char FMQSocPatchesV21Patch13[] = \
{
  0x0D, 0x81, 0x51, 0xF9, 0xE1, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF9, 0xE4, 0x90, 0x04, 0x83, 0xE0, 0x20,
  0xE7, 0x0A, 0x90, 0x0B, 0x84, 0xE0, 0x60, 0x01,
  0x22, 0x74, 0x01, 0xF0, 0x12, 0x4F, 0xF4, 0x02,
  0x81, 0x54
};

#define FMQSOCPATCHES_21_PATCH13_PTR     (&FMQSocPatchesV21Patch13[0])
#define SIZEOF_FMQSOCPATCHES_21_PATCH13  (sizeof(FMQSocPatchesV21Patch13))

/**
* Array of pointers to FM QSOC patch related data.
*/
static const unsigned char *FMQSocPatchesV21Data[] = \
{
   FMQSOCPATCHES_21_PATCH0_PTR
  ,FMQSOCPATCHES_21_PATCH1_PTR
  ,FMQSOCPATCHES_21_PATCH2_PTR
  ,FMQSOCPATCHES_21_PATCH3A_PTR
  ,FMQSOCPATCHES_21_PATCH3B_PTR
  ,FMQSOCPATCHES_21_PATCH3C_PTR
  ,FMQSOCPATCHES_21_PATCH3D_PTR
  ,FMQSOCPATCHES_21_PATCH3E_PTR
  ,FMQSOCPATCHES_21_PATCH3F_PTR
  ,FMQSOCPATCHES_21_PATCH4_PTR
  ,FMQSOCPATCHES_21_PATCH5_PTR
  ,FMQSOCPATCHES_21_PATCH6_PTR
  ,FMQSOCPATCHES_21_PATCH7_PTR
  ,FMQSOCPATCHES_21_PATCH8_PTR
  ,FMQSOCPATCHES_21_PATCH9_PTR
  ,FMQSOCPATCHES_21_PATCH10_PTR
  , FMQSOCPATCHES_21_PATCH11_PTR
  , FMQSOCPATCHES_21_PATCH12A_PTR
  , FMQSOCPATCHES_21_PATCH12B_PTR
  , FMQSOCPATCHES_21_PATCH13_PTR
};

/**
* Array that stores sizes of FM QSOC patch related data.
*/
static const unsigned char FMQSocPatchesV21Size[] = \
{
   SIZEOF_FMQSOCPATCHES_21_PATCH0
  ,SIZEOF_FMQSOCPATCHES_21_PATCH1
  ,SIZEOF_FMQSOCPATCHES_21_PATCH2
  ,SIZEOF_FMQSOCPATCHES_21_PATCH3A
  ,SIZEOF_FMQSOCPATCHES_21_PATCH3B
  ,SIZEOF_FMQSOCPATCHES_21_PATCH3C
  ,SIZEOF_FMQSOCPATCHES_21_PATCH3D
  ,SIZEOF_FMQSOCPATCHES_21_PATCH3E
  ,SIZEOF_FMQSOCPATCHES_21_PATCH3F
  ,SIZEOF_FMQSOCPATCHES_21_PATCH4
  ,SIZEOF_FMQSOCPATCHES_21_PATCH5
  ,SIZEOF_FMQSOCPATCHES_21_PATCH6
  ,SIZEOF_FMQSOCPATCHES_21_PATCH7
  ,SIZEOF_FMQSOCPATCHES_21_PATCH8
  ,SIZEOF_FMQSOCPATCHES_21_PATCH9
  ,SIZEOF_FMQSOCPATCHES_21_PATCH10
  ,SIZEOF_FMQSOCPATCHES_21_PATCH11
  ,SIZEOF_FMQSOCPATCHES_21_PATCH12A
  ,SIZEOF_FMQSOCPATCHES_21_PATCH12B
  ,SIZEOF_FMQSOCPATCHES_21_PATCH13
};

#define SIZEOF_FMQSOCPATCHES_PATCHV21DATA  (sizeof(FMQSocPatchesV21Data) / \
                                            sizeof(FMQSocPatchesV21Data[0]))

  /*-------------------------------------------------------------------------*/
  /*                            WCN2243 1.0 FM Patches                           */
  /*-------------------------------------------------------------------------*/

  /***************************************************************************/
  /* PATCH 0: Overwrite ROM code for timers                                  */
  /***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_10_Patch0[] = \
{
  0x00, 0x0A, 0x34, 0xF8, 0x00, 0x04, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xF0, 0xC5, 0xF0, 0xF8
};

#define FMQSOCPATCHES_WCN2243_10_PATCH0_PTR     (&FMQSocPatches_WCN2243_10_Patch0[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH0  (sizeof(FMQSocPatches_WCN2243_10_Patch0))

  /***************************************************************************/
  /* PATCH 1: Process cal does not generate the correct ADC value (1/2)      */
  /***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_10_Patch1[] = \
{
  0x01, 0x53, 0xD8, 0xF8, 0x05, 0x08, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x00, 0x00, 0x00, 0x80, 0x1C, 0x02, 0xF8, 0x0D, 0x12, 0xC2, 0xA3, 0xCF, 0x12,0x59, 0x44, 0x12, 0xC2, 0xA3, 0x02, 0x53, 0xE0
};

#define FMQSOCPATCHES_WCN2243_10_PATCH1_PTR     (&FMQSocPatches_WCN2243_10_Patch1[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH1  (sizeof(FMQSocPatches_WCN2243_10_Patch1))

  /***************************************************************************/
  /* PATCH 2: Process cal does not generate the correct ADC value (2/2)      */
  /***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_10_Patch2[] = \
{
  0x02, 0x59, 0x59, 0xF8, 0x1A, 0x01, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x9D
};

#define FMQSOCPATCHES_WCN2243_10_PATCH2_PTR     (&FMQSocPatches_WCN2243_10_Patch2[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH2  (sizeof(FMQSocPatches_WCN2243_10_Patch2))

  /***************************************************************************/
  /* PATCH 3:  FW sometimes ignores I2C writes                               */
  /***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_10_Patch3[] = \
{
  0x03, 0x8F, 0xB6, 0xF8, 0x1B, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x12, 0xF8, 0x1E, 0x12, 0xB3, 0x0A, 0xF0, 0x12, 0xAF, 0xCA, 0xE0, 0xFF, 0xA3, 0xE0, 0xFD, 0x12, 0x91, 0x91, 0x90, 0x00, 0x02, 0x02, 0x0F, 0x05
};

#define FMQSOCPATCHES_WCN2243_10_PATCH3_PTR     (&FMQSocPatches_WCN2243_10_Patch3[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH3  (sizeof(FMQSocPatches_WCN2243_10_Patch3))

  /***************************************************************************/
  /* PATCH 4:  Searches incorrectly find adjacent frequencies.               */
  /***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_10_Patch4[] = \
{
  0x04, 0x69, 0x58, 0xF8, 0x33, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x36, 0x50, 0x11, 0xC3, 0x90, 0x88, 0x26, 0xE0, 0x94, 0x73, 0x50, 0x08, 0xE0, 0xC3, 0x94, 0x6D, 0x40, 0x02, 0xD3, 0x22, 0x02, 0x69, 0x5B
};

#define FMQSOCPATCHES_WCN2243_10_PATCH4_PTR     (&FMQSocPatches_WCN2243_10_Patch4[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH4  (sizeof(FMQSocPatches_WCN2243_10_Patch4))

/***************************************************************************/
/* PATCH 5:  INTDET threshold is incorrectly set to unsigned.               */
/***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_10_Patch5[] = \
{
  0x05, 0x5B, 0xC7, 0xF8, 0x4C, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x12, 0xF8, 0x4F, 0xE0, 0xFE, 0xD3, 0xEF, 0x64, 0x80, 0xF8, 0xEE, 0x64, 0x80, 0x98, 0x22
};

#define FMQSOCPATCHES_WCN2243_10_PATCH5_PTR     (&FMQSocPatches_WCN2243_10_Patch5[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH5  (sizeof(FMQSocPatches_WCN2243_10_Patch5))

/***************************************************************************/
/* PATCH 6:  FW no longer clears 0x8003 on tuning.                         */
/***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_10_Patch6[] = \
{
  0x06, 0x44, 0x3E, 0xFB, 0x58, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x00, 0x00, 0x00
};

#define FMQSOCPATCHES_WCN2243_10_PATCH6_PTR     (&FMQSocPatches_WCN2243_10_Patch6[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH6  (sizeof(FMQSocPatches_WCN2243_10_Patch6))

/***************************************************************************/
/* PATCH 7:  FW no longer clears 0x8003 on tuning.                         */
/***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_10_Patch7[] = \
{
  0x07, 0x40, 0x0C, 0xF8, 0x5E, 0x09, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x74, 0x00, 0xF0, 0xD2, 0x03, 0x80, 0x08, 0x74, 0x0C
};

#define FMQSOCPATCHES_WCN2243_10_PATCH7_PTR     (&FMQSocPatches_WCN2243_10_Patch7[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH7  (sizeof(FMQSocPatches_WCN2243_10_Patch7))

/**
* Array of pointers to FM QSOC patch related data.
*/
static const unsigned char *FMQSocPatches_WCN2243_10_Data[] = \
{
   FMQSOCPATCHES_WCN2243_10_PATCH0_PTR,
   FMQSOCPATCHES_WCN2243_10_PATCH1_PTR,
   FMQSOCPATCHES_WCN2243_10_PATCH2_PTR,
   FMQSOCPATCHES_WCN2243_10_PATCH3_PTR,
   FMQSOCPATCHES_WCN2243_10_PATCH4_PTR,
   FMQSOCPATCHES_WCN2243_10_PATCH5_PTR,
   FMQSOCPATCHES_WCN2243_10_PATCH6_PTR,
   FMQSOCPATCHES_WCN2243_10_PATCH7_PTR
};

/**
* Array that stores sizes of FM QSOC patch related data.
*/
static const unsigned char FMQSocPatches_WCN2243_10_Size[] = \
{
   SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH0,
   SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH1,
   SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH2,
   SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH3,
   SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH4,
   SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH5,
   SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH6,
   SIZEOF_FMQSOCPATCHES_WCN2243_10_PATCH7
};

#define SIZEOF_FMQSOCPATCHES_PATCH_WCN2243_10_DATA  (sizeof(FMQSocPatches_WCN2243_10_Data) / \
                                                 sizeof(&FMQSocPatches_WCN2243_10_Data[0]))



  /*-------------------------------------------------------------------------*/
  /*                            WCN2243 2.0 FM Patches                           */
  /*-------------------------------------------------------------------------*/

  /***************************************************************************/
  /* PATCH 0: ROM spur frequencies not being applied                         */
  /***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch0[] = \
{
  0x00, 0x64, 0xAE, 0xF8, 0x00, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x03, 0x7E, 0xFF, 0x90, 0x08, 0xB8,
  0x02, 0x64, 0xB1
};

#define FMQSOCPATCHES_WCN2243_20_PATCH0_PTR     (&FMQSocPatches_WCN2243_20_Patch0[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH0  (sizeof(FMQSocPatches_WCN2243_20_Patch0))

  /***************************************************************************/
  /* PATCH 1: ROM Spur frequencies don't work for Hi-Side                    */
  /***************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch1[] = \
{
  0x01, 0x64, 0x82, 0xF8, 0x0B, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x0E, 0x90, 0x06, 0xC3, 0xE0, 0x64,
  0x02, 0x70, 0x13, 0x90, 0x08, 0xBA, 0xE0, 0xFF,
  0xA3, 0xE0, 0xFE, 0xC3, 0x74, 0xC8, 0x9E, 0xF0,
  0x90, 0x08, 0xBA, 0xE4, 0x9F, 0xF0, 0x90, 0x08,
  0xBA, 0x02, 0x64, 0x85
};

#define FMQSOCPATCHES_WCN2243_20_PATCH1_PTR     (&FMQSocPatches_WCN2243_20_Patch1[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH1  (sizeof(FMQSocPatches_WCN2243_20_Patch1))

  /******************************************************************************************************/
  /* PATCH 2: Ultimate SNR degraded at XO harmonic channels 1/2 (store DC IF Offset)                    */
  /******************************************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch2[] = \
{
  0x02, 0x65, 0xBC, 0xF8, 0x30, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x33, 0xEF, 0xF0, 0x90, 0x0B, 0x80,
  0xEE, 0xF0, 0xA3, 0xEF, 0xF0, 0x22
};

#define FMQSOCPATCHES_WCN2243_20_PATCH2_PTR     (&FMQSocPatches_WCN2243_20_Patch2[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH2  (sizeof(FMQSocPatches_WCN2243_20_Patch2))

  /******************************************************************************************************
  * PATCH 3a: Ultimate SNR degraded at XO harmonic channels 2/2 (enable spur rotator for G0)            *
  *   Note: X:0x0B80-0x0B81 allocated for DC IF Offset storage	                                        *
  ******************************************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch3a[] = \
{
  0x03, 0x73, 0xCD, 0xF8, 0x3E, 0x03, 0x00, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0x41, 0xC3, 0x90, 0x89, 0x3A, 0xE0,
  0x54, 0x03, 0x70, 0x1C, 0x90, 0x0B, 0x80, 0xE0,
  0xFE, 0xA3, 0xE0, 0xFF, 0x90, 0x89, 0x59, 0xF0,
  0xA3, 0xEE, 0xF0, 0x90, 0x89, 0x45, 0xE0, 0x44,
  0x01, 0xF0, 0x12, 0x88, 0x00, 0x02, 0x73, 0xD0,
  0x90, 0x89, 0x59, 0x74, 0xE5, 0xF0, 0xA3, 0x74,
  0x29, 0xF0, 0x90
};

#define FMQSOCPATCHES_WCN2243_20_PATCH3A_PTR     (&FMQSocPatches_WCN2243_20_Patch3a[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH3A  (sizeof(FMQSocPatches_WCN2243_20_Patch3a))

  /******************************************************************************************************
  * PATCH 3b: Ultimate SNR degraded at XO harmonic channels 2/2 (enable spur rotator for G0)            *
  ******************************************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch3b[] = \
{
  0x03, 0x73, 0xCD, 0xF8, 0x3E, 0x03, 0x01, 0x01,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x89, 0x45, 0xE0, 0x54, 0xFE, 0xF0, 0x80, 0xE7
};

#define FMQSOCPATCHES_WCN2243_20_PATCH3B_PTR     (&FMQSocPatches_WCN2243_20_Patch3b[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH3B  (sizeof(FMQSocPatches_WCN2243_20_Patch3b))

  /******************************************************************************************************
  * PATCH 4a: Hi/Lo switch once for the newly tuned frequency (1/2)                                     *
  * Note: X:0x0B82-0x0B83 current freq                                                                  *
  * Note: X:0x0B84 HiLoFlag
  ******************************************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch4a[] = \
{
  0x04, 0x87, 0x06, 0xF8, 0xB7, 0x11, 0x00, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0xC8, 0xBF, 0x01, 0x04, 0x7F, 0x02,
  0x80, 0x09, 0x90, 0x01, 0xAD, 0xE0, 0x00, 0x00,
  0x00, 0x90, 0x01, 0xAD, 0xE0, 0x20, 0xE7, 0x0A,
  0x90, 0x0B, 0x84, 0xE0, 0x60, 0x01, 0x22, 0x74,
  0x01, 0xF0, 0x12, 0x5F, 0x83, 0x02, 0x87, 0x09,
  0x90, 0x0B, 0x91, 0xE4, 0xF0, 0xA3, 0xF0, 0xA3,
  0xF0, 0x7F, 0x04

};

#define FMQSOCPATCHES_WCN2243_20_PATCH4A_PTR     (&FMQSocPatches_WCN2243_20_Patch4a[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH4A  (sizeof(FMQSocPatches_WCN2243_20_Patch4a))

  /******************************************************************************************************
  * PATCH 4b: Hi/Lo switch once for the newly tuned frequency (1/2)                                     *
  ******************************************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch4b[] = \
{
  0x04, 0x87, 0x06, 0xF8, 0xB7, 0x11, 0x00, 0x01,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x12, 0xB9, 0x6E, 0x7A, 0x0B, 0x79, 0x98, 0x12,
  0x0F, 0x63, 0x90, 0x01, 0x10, 0xE0, 0x90, 0x0B,
  0x94, 0xF0, 0x90, 0x88, 0xA8, 0xE0, 0x90, 0x0B,
  0x85, 0xF0, 0xFE, 0x90, 0x88, 0xA7, 0xE0, 0xFF,
  0x64, 0x80, 0x94, 0x83, 0xEE, 0x40, 0x03, 0x2F,
  0x80, 0x00, 0x90, 0x0B, 0x86, 0xF0, 0x7B, 0x01,
  0x7A, 0x0B, 0x79
};

#define FMQSOCPATCHES_WCN2243_20_PATCH4B_PTR     (&FMQSocPatches_WCN2243_20_Patch4b[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH4B  (sizeof(FMQSocPatches_WCN2243_20_Patch4b))

  /******************************************************************************************************
  * PATCH 4b: Hi/Lo switch once for the newly tuned frequency (1/2)                                     *
  ******************************************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch4c[] = \
{
  0x04, 0x87, 0x06, 0xF8, 0xB7, 0x11, 0x01, 0x02,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x88, 0x12, 0xFB, 0x33, 0x02, 0xF8, 0x7D
};

#define FMQSOCPATCHES_WCN2243_20_PATCH4C_PTR     (&FMQSocPatches_WCN2243_20_Patch4c[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH4C  (sizeof(FMQSocPatches_WCN2243_20_Patch4c))

  /******************************************************************************************************
  * PATCH 5a: Hi/Lo switch once for the newly tuned frequency (2/2)                                     *
  * Note: X:0x0B82-0x0B83 current freq                                                                  *
  * Note: X:0x0B84 HiLoFlag                                                                             *
  ******************************************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch5a[] = \
{
  0x05, 0x86, 0x21, 0xF8, 0x7A, 0x03, 0x00, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF8, 0xDF, 0x7A, 0x08, 0x79, 0x8E, 0x12,
  0xB3, 0x84, 0x90, 0x08, 0x8E, 0xE0, 0xFE, 0xA3,
  0xE0, 0xFF, 0x90, 0x0B, 0x82, 0xE0, 0x6E, 0xF8,
  0xA3, 0xE0, 0x6F, 0x48, 0x60, 0x05, 0xE4, 0x90,
  0x0B, 0x84, 0xF0, 0x90, 0x0B, 0x82, 0xEE, 0xF0,
  0xA3, 0xEF, 0xF0, 0x74, 0xA0, 0xFF, 0x90, 0x88,
  0xA8, 0x12, 0xB4

};

#define FMQSOCPATCHES_WCN2243_20_PATCH5A_PTR     (&FMQSocPatches_WCN2243_20_Patch5a[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH5A  (sizeof(FMQSocPatches_WCN2243_20_Patch5a))

  /******************************************************************************************************
  * PATCH 5b: Hi/Lo switch once for the newly tuned frequency (2/2)                                     *
  * Note: X:0x0B82-0x0B83 current freq                                                                  *
  * Note: X:0x0B84 HiLoFlag                                                                             *
  ******************************************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch5b[] = \
{
  0x05, 0x86, 0x21, 0xF8, 0x7A, 0x03, 0x01, 0x01,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xD8, 0x50, 0x01, 0x22, 0x90, 0x01, 0x13, 0x02,
  0x86, 0x24
};

#define FMQSOCPATCHES_WCN2243_20_PATCH5B_PTR     (&FMQSocPatches_WCN2243_20_Patch5b[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH5B  (sizeof(FMQSocPatches_WCN2243_20_Patch5b))

  /******************************************************************************************************
  * PATCH 6: PLL locks to adjacent channels                                                             *
  ******************************************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch6[] = \
{
  0x06, 0x5B, 0xC1, 0xF9, 0x24, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF9, 0x27, 0x90, 0x80, 0x03, 0xE0, 0x54,
  0xBF, 0xF0, 0x12, 0xC1, 0x6C, 0x02, 0x5B, 0xC4
};

#define FMQSOCPATCHES_WCN2243_20_PATCH6_PTR     (&FMQSocPatches_WCN2243_20_Patch6[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH6  (sizeof(FMQSocPatches_WCN2243_20_Patch6))

  /*********************************************************************************
  * Patch 7a: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7a[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xF9, 0x37, 0x90, 0x0A, 0x43, 0xE0, 0xB4,
  0x03, 0x11, 0x90, 0x01, 0x10, 0x12, 0xFC, 0x92,
  0xFE, 0xC3, 0x12, 0xB4, 0xDB, 0x40, 0x02, 0xD3,
  0x22, 0xC3, 0x22, 0x7B, 0x01, 0x7A, 0x0B, 0x79,
  0x8C, 0x12, 0xFB, 0x33, 0x90, 0x0B, 0x94, 0xE0,
  0x24, 0xFD, 0xFF, 0x90, 0x08, 0x87, 0xE0, 0xFE,
  0xC3, 0x12, 0xB4

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7A_PTR     (&FMQSocPatches_WCN2243_20_Patch7a[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7A  (sizeof(FMQSocPatches_WCN2243_20_Patch7a))

  /*********************************************************************************
  * Patch 7b: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7b[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x01,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xDB, 0x50, 0x27, 0x7F, 0x04, 0x12, 0xB9, 0x6E,
  0x7A, 0x0B, 0x79, 0x98, 0x12, 0x0F, 0x63, 0x90,
  0x0B, 0x91, 0xE4, 0xF0, 0x90, 0x08, 0x87, 0xE0,
  0x90, 0x0B, 0x98, 0xF0, 0x90, 0x0B, 0x94, 0xF0,
  0x90, 0x0B, 0x92, 0x74, 0x01, 0xF0, 0xA3, 0xF0,
  0x41, 0x14, 0x12, 0xFC, 0x8F, 0xFE, 0xC3, 0x9F,
  0xFF, 0xC3, 0x64

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7B_PTR     (&FMQSocPatches_WCN2243_20_Patch7b[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7B  (sizeof(FMQSocPatches_WCN2243_20_Patch7b))

  /*********************************************************************************
  * Patch 7c: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7c[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x02,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x80, 0x94, 0x80, 0x50, 0x06, 0xEF, 0xF4, 0x04,
  0xFF, 0x80, 0x09, 0x90, 0x0B, 0x94, 0xE0, 0xFF,
  0xC3, 0xEE, 0x9F, 0xFF, 0xD3, 0xEF, 0x64, 0x80,
  0x94, 0x83, 0x50, 0x5E, 0x90, 0x08, 0x87, 0xE0,
  0xFF, 0x90, 0x0B, 0x92, 0xE0, 0xFE, 0x24, 0x98,
  0xF5, 0x82, 0xE4, 0x34, 0x0B, 0xF5, 0x83, 0xEF,
  0xF0, 0x90, 0x0B

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7C_PTR     (&FMQSocPatches_WCN2243_20_Patch7c[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7C  (sizeof(FMQSocPatches_WCN2243_20_Patch7c))

  /*********************************************************************************
  * Patch 7d: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7d[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x03,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x92, 0xE0, 0x04, 0xF0, 0xA3, 0xE0, 0xC3, 0x94,
  0x04, 0x50, 0x03, 0xE0, 0x04, 0xF0, 0x90, 0x0B,
  0x91, 0xE0, 0xFF, 0xC3, 0xEE, 0x9F, 0xFF, 0xC3,
  0x94, 0x00, 0x50, 0x06, 0xEF, 0xF4, 0x04, 0xFF,
  0x80, 0x07, 0x90, 0x0B, 0x91, 0x12, 0xFC, 0x99,
  0xFF, 0xEF, 0xC3, 0x94, 0x04, 0x40, 0x0D, 0x90,
  0x0B, 0x91, 0xE0

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7D_PTR     (&FMQSocPatches_WCN2243_20_Patch7d[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7D  (sizeof(FMQSocPatches_WCN2243_20_Patch7d))

  /*********************************************************************************
  * Patch 7e: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7e[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x04,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x04, 0xF0, 0xE0, 0x94, 0x04, 0x40, 0x02, 0xE4,
  0xF0, 0x90, 0x0B, 0x92, 0xE0, 0xC3, 0x94, 0x04,
  0x40, 0x02, 0xE4, 0xF0, 0x90, 0x0B, 0x93, 0xE0,
  0xD3, 0x94, 0x00, 0x40, 0x52, 0x90, 0x0B, 0x95,
  0xE4, 0xF0, 0xA3, 0xF0, 0xA3, 0xF0, 0x90, 0x0B,
  0x93, 0xE0, 0xFF, 0x90, 0x0B, 0x97, 0xE0, 0xFE,
  0xC3, 0x12, 0xB4

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7E_PTR     (&FMQSocPatches_WCN2243_20_Patch7e[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7E  (sizeof(FMQSocPatches_WCN2243_20_Patch7e))

  /*********************************************************************************
  * Patch 7f: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7f[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x05,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xDB, 0x50, 0x25, 0xEE, 0xFD, 0x33, 0x95, 0xE0,
  0xFC, 0x74, 0x98, 0x2D, 0xF5, 0x82, 0x74, 0x0B,
  0x3C, 0xF5, 0x83, 0xE0, 0xFD, 0x33, 0x95, 0xE0,
  0x90, 0x0B, 0x95, 0x8D, 0xF0, 0x12, 0x0A, 0x2E,
  0x90, 0x0B, 0x97, 0xE0, 0x04, 0xF0, 0x80, 0xCB,
  0xEF, 0xFD, 0x7C, 0x00, 0x90, 0x0B, 0x95, 0xE0,
  0xFE, 0xA3, 0xE0

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7F_PTR     (&FMQSocPatches_WCN2243_20_Patch7f[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7F  (sizeof(FMQSocPatches_WCN2243_20_Patch7f))

  /*********************************************************************************
  * Patch 7g: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7g[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x06,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xFF, 0x12, 0xFB, 0xEE, 0x90, 0x0B, 0x94, 0xEF,
  0xF0, 0x12, 0x5F, 0x83, 0xBF, 0x02, 0x07, 0x12,
  0xFC, 0xC4, 0xFC, 0x12, 0xFC, 0xBE, 0x90, 0x0A,
  0x42, 0xE0, 0x30, 0xE0, 0x07, 0x12, 0xFC, 0xC4,
  0xFC, 0x12, 0xFC, 0xBE, 0x12, 0xC3, 0x66, 0x64,
  0x80, 0x94, 0x83, 0x90, 0x08, 0x87, 0xE0, 0x40,
  0x03, 0x2F, 0x80

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7G_PTR     (&FMQSocPatches_WCN2243_20_Patch7g[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7G  (sizeof(FMQSocPatches_WCN2243_20_Patch7g))

  /*********************************************************************************
  * Patch 7h: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7h[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x07,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x00, 0x90, 0x0B, 0x90, 0xF0, 0x90, 0x0B, 0x86,
  0xE0, 0xC3, 0x64, 0x80, 0x94, 0x49, 0x40, 0x21,
  0xA3, 0xE0, 0x64, 0x80, 0x94, 0x9E, 0x40, 0x19,
  0xA3, 0x12, 0x0C, 0xDE, 0xEC, 0x33, 0x90, 0x0B,
  0x88, 0x50, 0x08, 0x12, 0x0C, 0xDE, 0x12, 0x0C,
  0x3B, 0x80, 0x03, 0x12, 0x0C, 0xDE, 0x12, 0xFC,
  0xBE, 0x12, 0xFC

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7H_PTR     (&FMQSocPatches_WCN2243_20_Patch7h[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7H  (sizeof(FMQSocPatches_WCN2243_20_Patch7h))

  /*********************************************************************************
  * Patch 7i: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7i[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x08,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x8F, 0xC3, 0x9F, 0xFF, 0x90, 0x01, 0x14, 0xE0,
  0xC3, 0x12, 0xFC, 0xDA, 0x40, 0x54, 0x90, 0x0B,
  0x8C, 0x12, 0x0C, 0xDE, 0xEC, 0x33, 0x90, 0x0B,
  0x8C, 0x50, 0x08, 0x12, 0x0C, 0xDE, 0x12, 0x0C,
  0x3B, 0x80, 0x03, 0x12, 0x0C, 0xDE, 0xE4, 0x12,
  0xFD, 0x12, 0x00, 0x00, 0x00, 0xC3, 0x12, 0xFC,
  0x31, 0x40, 0x2F

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7I_PTR     (&FMQSocPatches_WCN2243_20_Patch7i[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7I  (sizeof(FMQSocPatches_WCN2243_20_Patch7i))

  /*********************************************************************************
  * Patch 7i: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7j[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x09,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xE4, 0x12, 0xFD, 0x21, 0x00, 0x00, 0x00, 0x90,
  0x0B, 0x88, 0x12, 0x0C, 0xFA, 0xC3, 0x12, 0xFC,
  0x31, 0x40, 0x1C, 0x12, 0xFC, 0xA0, 0x90, 0x0A,
  0x0D, 0xE0, 0x30, 0xE7, 0x10, 0x02, 0xFD, 0x30,
  0xFD, 0xA3, 0x12, 0xFD, 0x0C, 0xFF, 0xEF, 0x6D,
  0x30, 0xE0, 0x02, 0xC3, 0x22, 0xD3, 0x22, 0x12,
  0xFC, 0xA0, 0xC3

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7J_PTR     (&FMQSocPatches_WCN2243_20_Patch7j[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7J  (sizeof(FMQSocPatches_WCN2243_20_Patch7j))

  /*********************************************************************************
  * Patch 7k: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7k[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x0A,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x22, 0x90, 0x08, 0x8A, 0x12, 0x0D, 0x9F, 0x91,
  0x64, 0xEC, 0xC0, 0xE0, 0xED, 0xC0, 0xE0, 0xEE,
  0xC0, 0xE0, 0xEF, 0xC0, 0xE0, 0xA3, 0x12, 0xFC,
  0x54, 0x78, 0x10, 0x12, 0x0C, 0x6D, 0xD0, 0xE0,
  0xFB, 0xD0, 0xE0, 0xFA, 0xD0, 0xE0, 0xF9, 0xD0,
  0xE0, 0xF8, 0x12, 0x0C, 0x21, 0x91, 0x82, 0x12,
  0xFC, 0x51, 0x91

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7K_PTR     (&FMQSocPatches_WCN2243_20_Patch7k[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7K  (sizeof(FMQSocPatches_WCN2243_20_Patch7k))

  /*********************************************************************************
  * Patch 7l: searches incorrectly find adjacent frequencies.                      *
  * Note: X:0x0B85 SrchPrevRmssi                                                   *
  * Note: X:0x0B86 SrchRunRmssi                                                    *
  * Note: X:0x0B87 ucDCCCheck                                                      *
  * Note: X:0x0B88 ucKhz                                                           *
  * Note: X:0x0B89 - X:0x0B8C lMpx_Dcc                                             *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7l[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x0B,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x5B, 0x12, 0x0C, 0xA0, 0x91, 0x64, 0xEC, 0xC0,
  0xE0, 0xED, 0xC0, 0xE0, 0xEE, 0xC0, 0xE0, 0xEF,
  0xC0, 0xE0, 0xA3, 0x12, 0xFC, 0x54, 0x78, 0x10,
  0x12, 0x0C, 0x6D, 0xD0, 0xE0, 0xFB, 0xD0, 0xE0,
  0xFA, 0xD0, 0xE0, 0xF9, 0xD0, 0xE0, 0xF8, 0x12,
  0x0C, 0x21, 0x91, 0x82, 0x12, 0xFC, 0x51, 0x91,
  0x5B, 0x12, 0xFC

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7L_PTR     (&FMQSocPatches_WCN2243_20_Patch7l[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7L  (sizeof(FMQSocPatches_WCN2243_20_Patch7l))

  /*********************************************************************************
  * Patch 7m: searches incorrectly find adjacent frequencies.                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7m[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x0C,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xEC, 0xC3, 0x12, 0xFC, 0x31, 0x60, 0x30, 0x91,
  0x64, 0xEC, 0xC0, 0xE0, 0xED, 0xC0, 0xE0, 0xEE,
  0xC0, 0xE0, 0xEF, 0xC0, 0xE0, 0xA3, 0x12, 0xFC,
  0x54, 0x78, 0x10, 0x12, 0x0C, 0x6D, 0xD0, 0xE0,
  0xFB, 0xD0, 0xE0, 0xFA, 0xD0, 0xE0, 0xF9, 0xD0,
  0xE0, 0xF8, 0x12, 0x0C, 0x21, 0x91, 0x82, 0x12,
  0xFC, 0x51, 0x91

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7M_PTR     (&FMQSocPatches_WCN2243_20_Patch7m[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7M  (sizeof(FMQSocPatches_WCN2243_20_Patch7m))

  /*********************************************************************************
  * Patch 7n: searches incorrectly find adjacent frequencies.                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7n[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x0D,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x5B, 0x12, 0x0C, 0xA0, 0x91, 0x5E, 0x12, 0x0C,
  0x80, 0xE4, 0xFF, 0xFE, 0xED, 0x54, 0x08, 0xFD,
  0xE4, 0xFC, 0xED, 0x60, 0x0D, 0x12, 0x0C, 0x80,
  0xED, 0x44, 0xF0, 0xFD, 0x74, 0xFF, 0xFC, 0x12,
  0x0C, 0xA0, 0x22, 0xC2, 0xD5, 0xEC, 0x30, 0xE7,
  0x09, 0xB2, 0xD5, 0xE4, 0xC3, 0x9D, 0xFD, 0xE4,
  0x9C, 0xFC, 0xEE

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7N_PTR     (&FMQSocPatches_WCN2243_20_Patch7n[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7N  (sizeof(FMQSocPatches_WCN2243_20_Patch7n))

  /*********************************************************************************
  * Patch 7o: searches incorrectly find adjacent frequencies.                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7o[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x0E,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x30, 0xE7, 0x15, 0xB2, 0xD5, 0xE4, 0xC3, 0x9F,
  0xFF, 0xE4, 0x9E, 0xFE, 0x12, 0x09, 0xD9, 0xC3,
  0xE4, 0x9D, 0xFD, 0xE4, 0x9C, 0xFC, 0x80, 0x03,
  0x12, 0x09, 0xD9, 0x30, 0xD5, 0x07, 0xC3, 0xE4,
  0x9F, 0xFF, 0xE4, 0x9E, 0xFE, 0x22, 0xCF, 0xF4,
  0xCF, 0xCE, 0xF4, 0xCE, 0xCD, 0xF4, 0xCD, 0xCC,
  0xF4, 0xCC, 0x22

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7O_PTR     (&FMQSocPatches_WCN2243_20_Patch7o[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7O  (sizeof(FMQSocPatches_WCN2243_20_Patch7o))

  /*********************************************************************************
  * Patch 7p: searches incorrectly find adjacent frequencies.                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7p[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x0F,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xEB, 0x9F, 0xF5, 0xF0, 0xEA, 0x9E, 0x42, 0xF0,
  0xE9, 0x9D, 0x42, 0xF0, 0xEC, 0x64, 0x80, 0xC8,
  0x64, 0x80, 0x98, 0x45, 0xF0, 0x22, 0xBB, 0x01,
  0x07, 0x89, 0x82, 0x8A, 0x83, 0x02, 0x0C, 0xFA,
  0x90, 0x88, 0xC2, 0xE0, 0xFF, 0xE4, 0xFC, 0xFD,
  0xFE, 0x22, 0x12, 0x0C, 0x21, 0x90, 0x08, 0x8A,
  0x02, 0x0D, 0x7F

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7P_PTR     (&FMQSocPatches_WCN2243_20_Patch7p[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7P  (sizeof(FMQSocPatches_WCN2243_20_Patch7p))

  /*********************************************************************************
  * Patch 7q: searches incorrectly find adjacent frequencies.                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7q[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x10,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x90, 0x88, 0xC3, 0xE0, 0xFF, 0xE4, 0x78, 0x08,
  0xFC, 0xFD, 0xFE, 0x02, 0x0C, 0x6D, 0xE9, 0xFB,
  0xC3, 0xED, 0x9B, 0xFD, 0xEC, 0x94, 0x00, 0xFC,
  0xED, 0x2F, 0xFF, 0xEC, 0x3E, 0x22, 0xC8, 0xEC,
  0xC8, 0xC9, 0xED, 0xC9, 0xCA, 0xEE, 0xCA, 0xCB,
  0xEF, 0xCB, 0x22, 0x90, 0x0B, 0x94, 0xE0, 0xFF,
  0x90, 0x08, 0x87

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7Q_PTR     (&FMQSocPatches_WCN2243_20_Patch7q[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7Q  (sizeof(FMQSocPatches_WCN2243_20_Patch7q))

  /*********************************************************************************
  * Patch 7r: searches incorrectly find adjacent frequencies.                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7r[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x11,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xE0, 0x22, 0xE0, 0xFF, 0xA3, 0xE0, 0xC3, 0x9F,
  0x22, 0x90, 0x08, 0x87, 0xE0, 0x90, 0x0B, 0x85,
  0xF0, 0x90, 0x0B, 0x90, 0xE0, 0x90, 0x0B, 0x86,
  0xF0, 0x90, 0x08, 0x88, 0xE0, 0x90, 0x0B, 0x87,
  0xF0, 0x90, 0x0B, 0x8C, 0x12, 0x0C, 0xDE, 0x90,
  0x0B, 0x88, 0x02, 0x0D, 0x36, 0x90, 0x0B, 0x88,
  0x12, 0x0C, 0xDE

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7R_PTR     (&FMQSocPatches_WCN2243_20_Patch7r[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7R  (sizeof(FMQSocPatches_WCN2243_20_Patch7r))

  /*********************************************************************************
  * Patch 7s: searches incorrectly find adjacent frequencies.                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7s[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x12,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x12, 0xFC, 0x24, 0xEF, 0x24, 0x01, 0xFF, 0xE4,
  0x3E, 0xFE, 0xE4, 0x3D, 0xFD, 0xE4, 0x3C, 0x22,
  0x64, 0x80, 0xF8, 0xEF, 0x64, 0x80, 0x98, 0x22,
  0x90, 0x0A, 0x47, 0xE0, 0xC3, 0x13, 0xA3, 0xE0,
  0x13, 0x22, 0xBB, 0x01, 0x07, 0x89, 0x82, 0x8A,
  0x83, 0x02, 0x0C, 0xFA, 0x50, 0x05, 0xE9, 0xF8,
  0x02, 0x12, 0x0B

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7S_PTR     (&FMQSocPatches_WCN2243_20_Patch7s[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7S  (sizeof(FMQSocPatches_WCN2243_20_Patch7s))

  /*********************************************************************************
  * Patch 7t: searches incorrectly find adjacent frequencies.                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7t[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x13,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xBB, 0xFE, 0x05, 0xE9, 0xF8, 0x02, 0x12, 0x18,
  0x89, 0x82, 0x8A, 0x83, 0x02, 0x12, 0x25, 0x90,
  0x0A, 0x4B, 0x02, 0xFC, 0xE5, 0x90, 0x0B, 0xA8,
  0xE0, 0xF8, 0xA3, 0xE0, 0xF9, 0xA3, 0xE0, 0xFA,
  0xA3, 0xE0, 0xFB, 0x22, 0x90, 0x0B, 0xAC, 0xE0,
  0xFC, 0xA3, 0xE0, 0xFD, 0xA3, 0xE0, 0xFE, 0xA3,
  0xE0, 0xFF, 0x22

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7T_PTR     (&FMQSocPatches_WCN2243_20_Patch7t[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7T  (sizeof(FMQSocPatches_WCN2243_20_Patch7t))

  /*********************************************************************************
  * Patch 7u: searches incorrectly find adjacent frequencies.                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7u[] = \
{
  0x07, 0x5F, 0x4D, 0xF9, 0x34, 0x03, 0x00, 0x14,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x90, 0x0A, 0x44, 0xE0, 0xB4, 0x01, 0x13, 0x90,
  0x0A, 0x47, 0xA3, 0xE0, 0xFF, 0x90, 0x0A, 0x4B,
  0xA3, 0xE0, 0x6F, 0x30, 0xE0, 0x02, 0xC3, 0x22,
  0xD3, 0x22, 0x12, 0xFC, 0xE2, 0x02, 0xFB, 0x1F

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7U_PTR     (&FMQSocPatches_WCN2243_20_Patch7u[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7U  (sizeof(FMQSocPatches_WCN2243_20_Patch7u))

  /*********************************************************************************
  * Patch 7v: Support for filtering spurs during search.                           *
  * Note: X:0x0BB7 Table Size.                                                     *
  * Note: X:0x0BB8 Spur Table.                                                     *
  * Note: (X:0x0BB8 + (3 * spur index) - spur freq @ spur index.                   *
  * Note: (X:0x0BB8 + 2 + (3 * spur index) - spur rmssi @ spur index.              *
  **********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7v[] = \
{
  0x07, 0x5F, 0x4D, 0xFE, 0x78, 0x03, 0x00, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xFE, 0x7B, 0x12, 0xF9, 0x34, 0x50, 0x74,
  0x90, 0x0A, 0x47, 0xE0, 0xFF, 0xA3, 0xE0, 0x90,
  0x0B, 0xB5, 0xCF, 0xF0, 0xA3, 0xEF, 0xF0, 0x7B,
  0x01, 0x7A, 0x0B, 0x79, 0xB5, 0x12, 0x65, 0xBF,
  0xE4, 0x90, 0x0B, 0xB4, 0xF0, 0x90, 0x0B, 0xB7,
  0xE0, 0xFF, 0x90, 0x0B, 0xB4, 0xE0, 0xFE, 0xC3,
  0x9F, 0x50, 0x47

};

#define FMQSOCPATCHES_WCN2243_20_PATCH7V_PTR     (&FMQSocPatches_WCN2243_20_Patch7v[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7V  (sizeof(FMQSocPatches_WCN2243_20_Patch7v))

  /*********************************************************************************
  * Patch 7w: Support for filtering spurs during search.                           *
  * Note: X:0x0BB7 Table Size.                                                     *
  * Note: X:0x0BB8 Spur Table.                                                     *
  * Note: (X:0x0BB8 + (3 * spur index) - spur freq @ spur index.                   *
  * Note: (X:0x0BB8 + 2 + (3 * spur index) - spur rmssi @ spur index.              *
  **********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7w[] = \
{
  0x07, 0x5F, 0x4D, 0xFE, 0x78, 0x03, 0x00, 0x01,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xEE, 0x75, 0xF0, 0x03, 0xA4, 0x24, 0xB8, 0xF5,
  0x82, 0xE4, 0x34, 0x0B, 0xF5, 0x83, 0xE0, 0xFE,
  0xA3, 0xE0, 0xFF, 0x90, 0x0B, 0xB5, 0xE0, 0x6E,
  0x70, 0x03, 0xA3, 0xE0, 0x6F, 0x70, 0x20, 0x90,
  0x0B, 0xB4, 0xE0, 0x75, 0xF0, 0x03, 0xA4, 0x24,
  0xBA, 0xF5, 0x82, 0xE4, 0x34, 0x0B, 0xF5, 0x83,
  0xE0, 0xFF, 0x90
};

#define FMQSOCPATCHES_WCN2243_20_PATCH7W_PTR     (&FMQSocPatches_WCN2243_20_Patch7w[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7W  (sizeof(FMQSocPatches_WCN2243_20_Patch7w))

  /*********************************************************************************
  * Patch 7x: Support for filtering spurs during search.                           *
  * Note: X:0x0BB7 Table Size.                                                     *
  * Note: X:0x0BB8 Spur Table.                                                     *
  * Note: (X:0x0BB8 + (3 * spur index) - spur freq @ spur index.                   *
  * Note: (X:0x0BB8 + 2 + (3 * spur index) - spur rmssi @ spur index.              *
  **********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch7x[] = \
{
  0x07, 0x5F, 0x4D, 0xFE, 0x78, 0x03, 0x01, 0x02,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x08, 0x87, 0xE0, 0xFE, 0xC3, 0x12, 0xB4, 0xDB,
  0x50, 0x02, 0xC3, 0x22, 0x90, 0x0B, 0xB4, 0xE0,
  0x04, 0xF0, 0x80, 0xAB, 0xD3, 0x22, 0xC3, 0x22
};

#define FMQSOCPATCHES_WCN2243_20_PATCH7X_PTR     (&FMQSocPatches_WCN2243_20_Patch7x[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7X  (sizeof(FMQSocPatches_WCN2243_20_Patch7x))

  /*********************************************************************************
  * Patch 8a: 200 KHz search jumps in 150/50 KHz steps.                            *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch8a[] = \
{
  0x08, 0x67, 0x4C, 0xFD, 0x50, 0x03, 0x00, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xFD, 0x53, 0x90, 0x0A, 0x44, 0xE0, 0x64,
  0x02, 0x70, 0x44, 0x90, 0x0A, 0x0D, 0xE0, 0x30,
  0xE7, 0x3D, 0x12, 0xBB, 0x41, 0x90, 0x0A, 0x4B,
  0xA3, 0xE0, 0x6F, 0x20, 0xE0, 0x19, 0x90, 0x0A,
  0x42, 0xE0, 0x30, 0xE0, 0x09, 0xB1, 0xB6, 0x12,
  0xFD, 0xBC, 0xB1, 0xB6, 0x80, 0x19, 0xB1, 0xB9,
  0x12, 0xFD, 0xBC

};

#define FMQSOCPATCHES_WCN2243_20_PATCH8A_PTR     (&FMQSocPatches_WCN2243_20_Patch8a[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH8A  (sizeof(FMQSocPatches_WCN2243_20_Patch8a))

  /*********************************************************************************
  * Patch 8b: 200 KHz search jumps in 150/50 KHz steps.                            *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch8b[] = \
{
  0x08, 0x67, 0x4C, 0xFD, 0x50, 0x03, 0x00, 0x01,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0xB1, 0xB9, 0x80, 0x10, 0x12, 0xFD, 0xBC, 0x90,
  0x0A, 0x42, 0xE0, 0x30, 0xE0, 0x04, 0xB1, 0xB6,
  0x80, 0x02, 0xB1, 0xB9, 0x90, 0x0A, 0x44, 0x74,
  0x02, 0xF0, 0x80, 0x0D, 0x90, 0x0A, 0x42, 0xE0,
  0x30, 0xE0, 0x04, 0xB1, 0xB6, 0x80, 0x02, 0xB1,
  0xB9, 0x90, 0x0A, 0x42, 0xE0, 0x54, 0xC7, 0xF0,
  0x02, 0x67, 0x5B

};

#define FMQSOCPATCHES_WCN2243_20_PATCH8B_PTR     (&FMQSocPatches_WCN2243_20_Patch8b[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH8B  (sizeof(FMQSocPatches_WCN2243_20_Patch8b))

  /*********************************************************************************
  * Patch 8c: 200 KHz search jumps in 150/50 KHz steps.                            *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch8c[] = \
{
  0x08, 0x67, 0x4C, 0xFD, 0x50, 0x03, 0x01, 0x02,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xB0, 0x00, 0x02, 0xAE, 0x53, 0x90, 0x0A,
  0x44, 0x74, 0x01, 0xF0, 0x22

};

#define FMQSOCPATCHES_WCN2243_20_PATCH8C_PTR     (&FMQSocPatches_WCN2243_20_Patch8c[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH8C  (sizeof(FMQSocPatches_WCN2243_20_Patch8c))

  /*********************************************************************************
  * Patch 9: AF List is cleared on AF Jump.                                        *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch9[] = \
{
  0x09, 0x5F, 0xE0, 0xFD, 0xC4, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xFD, 0xC7, 0x90, 0x0A, 0xC2, 0xE0, 0xB4,
  0x04, 0x01, 0x22, 0x02, 0x62, 0xF7
};

#define FMQSOCPATCHES_WCN2243_20_PATCH9_PTR     (&FMQSocPatches_WCN2243_20_Patch9[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH9  (sizeof(FMQSocPatches_WCN2243_20_Patch9))

  /*********************************************************************************
  * Patch 10: Enable RDS no longer requires RMSSI Check.                           *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch10[] = \
{
  0x0A, 0x7C, 0x6E, 0xFD, 0xD2, 0x02, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x00, 0x00
};

#define FMQSOCPATCHES_WCN2243_20_PATCH10_PTR     (&FMQSocPatches_WCN2243_20_Patch10[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH10  (sizeof(FMQSocPatches_WCN2243_20_Patch10))

  /*********************************************************************************
  * Patch 11: Added protection for RDS queue counter in FM_RX_RDS_GetEvent().      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch11[] = \
{
  0x0B, 0xA4, 0x11, 0xFD, 0xD4, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xFD, 0xD7, 0xC2, 0xAF, 0x90, 0x07, 0x98,
  0xE0, 0xD3, 0x94, 0x00, 0x40, 0x15, 0x90, 0x07,
  0x97, 0x12, 0xC0, 0x2E, 0xFC, 0xAD, 0x82, 0x90,
  0x07, 0x98, 0xE0, 0x14, 0xF0, 0x90, 0x07, 0x97,
  0x12, 0xC4, 0xFA, 0xD2, 0xAF, 0x02, 0xC2, 0xB9
};

#define FMQSOCPATCHES_WCN2243_20_PATCH11_PTR     (&FMQSocPatches_WCN2243_20_Patch11[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH11  (sizeof(FMQSocPatches_WCN2243_20_Patch11))

  /*********************************************************************************
  * Patch 12: Adding support for restarting RDS Sync #1.                           *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch12[] = \
{
  0x0C, 0xAD, 0xC2, 0xFD, 0xFC, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xFD, 0xFF, 0x90, 0x08, 0x86, 0xE0, 0xB4,
  0x08, 0x0D, 0x7F, 0x09, 0x12, 0x47, 0xF7, 0x12,
  0x67, 0xF1, 0x12, 0x6F, 0x23, 0x80, 0x0B, 0x90,
  0x08, 0x86, 0xE0, 0xFF, 0xB4, 0x09, 0x03, 0x12,
  0x75, 0x75, 0x02, 0xAD, 0xE6
};

#define FMQSOCPATCHES_WCN2243_20_PATCH12_PTR     (&FMQSocPatches_WCN2243_20_Patch12[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH12  (sizeof(FMQSocPatches_WCN2243_20_Patch12))

  /*********************************************************************************
  * Patch 13: Adding support for restarting RDS Sync #2.                           *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch13[] = \
{
  0x0D, 0x76, 0xA5, 0xFE, 0x21, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xFE, 0x24, 0x30, 0xE0, 0x0F, 0x12, 0x57,
  0x70, 0xBF, 0x04, 0x09, 0x7D, 0x14, 0x7C, 0x00,
  0x7F, 0x09, 0x12, 0x47, 0x8F, 0x90, 0x08, 0xC4,
  0xE0, 0x30, 0xE2, 0x08, 0x7F, 0x09, 0x12, 0x47,
  0xF7, 0x02, 0x76, 0xA8, 0x02, 0x76, 0xE4
};

#define FMQSOCPATCHES_WCN2243_20_PATCH13_PTR     (&FMQSocPatches_WCN2243_20_Patch13[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH13  (sizeof(FMQSocPatches_WCN2243_20_Patch13))

  /*********************************************************************************
  * Patch 14: Force non-coherent free running mode for AF Jump.                   *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch14[] = \
{
  0x0E, 0x56, 0xC0, 0xFE, 0x48, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xFE, 0x4B, 0x12, 0x57, 0x70, 0xBF, 0x04,
  0x08, 0x90, 0x89, 0x3F, 0x74, 0x01, 0x02, 0x56,
  0xE0, 0x90, 0x02, 0xEF, 0x02, 0x56, 0xC3
};

#define FMQSOCPATCHES_WCN2243_20_PATCH14_PTR     (&FMQSocPatches_WCN2243_20_Patch14[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH14  (sizeof(FMQSocPatches_WCN2243_20_Patch14))

  /*********************************************************************************
  * Patch 15: Clear RDS Queue on RDS Disable.                                      *
  *********************************************************************************/
static const unsigned char FMQSocPatches_WCN2243_20_Patch15[] = \
{
  0x0F, 0x77, 0xC7, 0xFE, 0x5F, 0x03, 0x01, 0x00,
  /*I     R           R           S     C    R
    D     O           A           I     T    A
    X     M           M           Z     L    Moffset
  */

  /* Patch Payload */
  0x02, 0xFE, 0x62, 0xC2, 0xAF, 0x7F, 0x1B, 0x12,
  0xC0, 0x0D, 0x7A, 0x07, 0x79, 0x96, 0x12, 0x0F,
  0x63, 0xD2, 0xAF, 0x12, 0x1E, 0x91, 0x02, 0x77,
  0xCA
};

#define FMQSOCPATCHES_WCN2243_20_PATCH15_PTR     (&FMQSocPatches_WCN2243_20_Patch15[0])
#define SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH15  (sizeof(FMQSocPatches_WCN2243_20_Patch15))

/**
* Array of pointers to FM QSOC patch related data.
*/
static const unsigned char *FMQSocPatches_WCN2243_20_Data[] = \
{
   FMQSOCPATCHES_WCN2243_20_PATCH0_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH1_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH2_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH3A_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH3B_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH4A_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH4B_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH4C_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH5A_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH5B_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH6_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7A_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7B_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7C_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7D_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7E_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7F_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7G_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7H_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7I_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7J_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7K_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7L_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7M_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7N_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7O_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7P_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7Q_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7R_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7S_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7T_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7U_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7V_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7W_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH7X_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH8A_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH8B_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH8C_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH9_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH10_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH11_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH12_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH13_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH14_PTR,
   FMQSOCPATCHES_WCN2243_20_PATCH15_PTR
};

/**
* Array that stores sizes of FM QSOC patch related data.
*/
static const unsigned char FMQSocPatches_WCN2243_20_Size[] = \
{
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH0,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH1,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH2,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH3A,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH3B,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH4A,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH4B,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH4C,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH5A,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH5B,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH6,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7A,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7B,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7C,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7D,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7E,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7F,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7G,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7H,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7I,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7J,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7K,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7L,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7M,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7N,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7O,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7P,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7Q,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7R,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7S,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7T,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7U,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7V,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7W,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH7X,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH8A,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH8B,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH8C,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH9,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH10,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH11,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH12,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH13,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH14,
   SIZEOF_FMQSOCPATCHES_WCN2243_20_PATCH15
};

#define SIZEOF_FMQSOCPATCHES_PATCH_WCN2243_20_DATA  (sizeof(FMQSocPatches_WCN2243_20_Data) / \
                                                     sizeof(&FMQSocPatches_WCN2243_20_Data[0]))

  /*-------------------------------------------------------------------------*/
  /*                            FM6500 Marimba New Defaults                  */
  /*-------------------------------------------------------------------------*/

static const tsFmQSocPatchesDefaultType gFMQSocMarimbaPatchesXfrDefaults[] = \
{
    {XFR_MODE_RX_TIMERS,
      {FM_RX_QSOC_TIMER_RADIO,
       FM_RX_QSOC_TIMER_MODEM,
       FM_RX_QSOC_TIMER_CHCOND,
       FM_RX_QSOC_TIMER_INTDET,
       FM_RX_QSOC_TIMER_SFHARD,
       FM_RX_QSOC_TIMER_RDSTIMEOUT,
       FM_RX_QSOC_TIMER_TEMP_MSB,
       FM_RX_QSOC_TIMER_TEMP_LSB,
       FM_RX_QSOC_TIMER_PILOTPLL,
       FM_RX_QSOC_TIMER_FASTMDM,
       FM_RX_QSOC_TIMER_SEARCH,
       FM_RX_QSOC_TIMER_NONCOHERENT_TEMP_MSB,
       FM_RX_QSOC_TIMER_NONCOHERENT_TEMP_LSB,
       0x00,
       0x00,
       0x00}
    },

    {0x31, {0x01, 0x3C, 0x0B, 0x4C, 0x00, 0xD6, 0x0C, 0xD1, 0x08, 0x41,
            0x00, 0xF8, 0x02, 0x00, 0x00, 0x00}}
/*
    Example for new defaults:
    ,{mode, {payload}}
*/
};


#define FMQSOCMARIMBAPATCHES_DEFAULT_PTR           (&gFMQSocMarimbaPatchesXfrDefaults[0])
#define SIZEOF_FMQSOCMARIMBAPATCHES_DEFAULT        (sizeof(gFMQSocMarimbaPatchesXfrDefaults) / \
							sizeof(gFMQSocMarimbaPatchesXfrDefaults[0]))


  /*-------------------------------------------------------------------------*/
  /*                            FM6500 WCN2243 Defaults                  */
  /*-------------------------------------------------------------------------*/

static const tsFmQSocPatchesDefaultType gFMQSocBahamaPatchesXfrDefaults[] = \
{

    /* Download new SPUR 0 rotation frequencies */
    /* 78.85, 81.45, 86.4, 79.95, 105.6 */
    {0x32, {0x00, 0x80, 0x39, 0xD2, 0x80, 0x6D, 0x0A, 0x80, 0xD0, 0x64,
	0x80, 0x13, 0xBE, 0x82, 0x50, 0x64} },

    /* Download new SPUR 1 rotation frequencies */
    /* 88.4, 101.6 */
    {0x32, {0x01, 0x80, 0xF8, 0x64, 0x82, 0x00, 0x64, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },

    /* DIG_DCC updates */
    {0x31, {0x0C, 0x01, 0x0B, 0x4D, 0x06, 0x52, 0x06, 0x06, 0x06, 0xE0,
            0x04, 0xA8, 0x02, 0x00, 0x00, 0x00} },

    /* DIG_RX updates */
    {0x34, {0x06, 0xFB, 0x02, 0x0F, 0x02, 0x11, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },

    /* DIG_AGC_0 updates : Download AGC values */
    {0x28, {0xB0, 0x22, 0x04, 0x00, 0x0E, 0x1F, 0x2E, 0x11, 0x0F, 0x0D,
            0x7F, 0x00, 0x00, 0x00, 0x00, 0x00} },

    /* DIG_AGC_1: RX_NOISE_ENERGY updates */
    {0x29, {0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
	0x00, 0x03, 0x00, 0x00, 0x00, 0x03} },

    /* DIG_AGC_2 updates */
    {0x2A, {0x81, 0xBA, 0xCB, 0xDF, 0xBF, 0xCE, 0xE2, 0x7F, 0xB3, 0xC1,
	0xD3, 0x7F, 0xBF, 0xCE, 0xE2, 0x7F} },

    /* RX_CONFIG - Rx Configuration Mode updates */
    {0x15, {FMQSOCPATCHES_SEARCH_NFE, 0xA0, 0x00, 0x0A, FMQSOCPATCHES_SEARCH_NF_DELTA, 0xFF, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },

    /* RX_TIMERS - Rx Timers - Reduce tune/search timers */
    {0x16, {FM_RX_QSOC_TIMER_RADIO,
            FM_RX_QSOC_TIMER_MODEM,
            FM_RX_QSOC_TIMER_CHCOND,
            FM_RX_QSOC_TIMER_INTDET,
            FM_RX_QSOC_TIMER_SFHARD,
            FM_RX_QSOC_TIMER_RDSTIMEOUT,
            FM_RX_QSOC_TIMER_TEMP_MSB,
            FM_RX_QSOC_TIMER_TEMP_LSB,
            FM_RX_QSOC_TIMER_PILOTPLL,
            FM_RX_QSOC_TIMER_FASTMDM,
            FM_RX_QSOC_TIMER_SEARCH,
            FM_RX_QSOC_TIMER_NONCOHERENT_TEMP_MSB,
            FM_RX_QSOC_TIMER_NONCOHERENT_TEMP_LSB,
            0x00,
            0x00,
            0x00} }

/*
    Example for new defaults:
    ,{mode, {payload}}
*/
};

/*-------------------------------------------------------------------------*/
  /*                            FM6500 WCN2243 2.1 Defaults                  */
  /*-------------------------------------------------------------------------*/

static const tsFmQSocPatchesDefaultType gFMQSocBahamaB1PatchesXfrDefaults[] = \
{

    /* Download new SPUR 0 rotation frequencies */
    /* 78.85, 81.45, 86.4, 79.95, 105.6 */
    {0x32, {0x00, 0x80, 0x39, 0xD2, 0x80, 0x6D, 0x0A, 0x80, 0xD0, 0x64,
        0x80, 0x13, 0xBE, 0x82, 0x50, 0x64} },

       /* DIG_DCC updates */
    {0x31, {0x0C, 0x01, 0x0B, 0x4D, 0x06, 0x52, 0x06, 0x06, 0x06, 0xE0,
            0x04, 0xA8, 0x02} },

    /* DIG_RX updates */
    {0x34, {0x06, 0xFB, 0x02, 0x0F, 0x02, 0x11, 0x00} },

    /* DIG_AGC_0 updates : Download AGC values */
    {0x28, {0xB0, 0x22, 0x04, 0x00, 0x0E, 0x1F, 0x2E, 0x11, 0x13, 0x0D,
            0x7F} },

    /* DIG_AGC_1: RX_NOISE_ENERGY updates */
    {0x29, {0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x00, 0x03, 0x00, 0x00, 0x00, 0x03} },

    /* DIG_AGC_2 updates */
    {0x2A, {0x81, 0xB0, 0xBF, 0xD5, 0xBA, 0xCA, 0xDF, 0x7F, 0xB4, 0xC4,
        0xD9, 0x7F, 0x11, 0x13, 0x0D, 0x7F} },

    /* RX_CONFIG - Rx Configuration Mode updates */
    {0x15, {0XA6, 0xA0, 0x00, 0x0A, 0x7F, 0x73, 0x6D, 0x9A} },

    /* RX_TIMERS - Rx Timers - Reduce tune/search timers */
    {0x16, {0X01, 0X04, 0X14, 0X11, 0X03, 0X5A, 0X2E, 0XE0, 0X0A, 0X14,
        0X0A, 0X02, 0X58}},

    /*XFR_EXT - SRCH_CFG_1 : Search control mode - Basic */
    {0X3F, {0X02, 0X00, 0X00, 0X4E, 0X20, 0X00, 0X00, 0X4E, 0X20, 0X04,
        0X00, 0X00, 0XA0, 0X00, 0X00, 0X00}},

    /*XFR_EXT - SRCH_CFG_2 : Search control mode - Enhanced*/
    {0x3F, {0X03, 0X00, 0X00, 0X4E, 0X20, 0X00, 0X00, 0X4E, 0X20, 0X00,
        0X00, 0X6B, 0X6C}},

    /* RDS_PS_0 updates */
    {0x01, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},

    /* DIG_RXRDS_0 updates */
    {0x30, {0x25, 0x32, 0x34, 0x07, 0x0A, 0xF0, 0x3B, 0x00, 0x01, 0x10,
        0x08, 0x35, 0x11, 0x64, 0x07, 0x18}}
/*
    Example for new defaults:
    ,{mode, {payload}}
*/
};

#define FMQSOCBAHAMAPATCHES_DEFAULT_PTR           (&gFMQSocBahamaPatchesXfrDefaults[0])
#define SIZEOF_FMQSOCBAHAMAPATCHES_DEFAULT        (sizeof(gFMQSocBahamaPatchesXfrDefaults) / \
							sizeof(gFMQSocBahamaPatchesXfrDefaults[0]))


  /*-------------------------------------------------------------------------*/
  /*                            FM6500 2.1 SPUR values                       */
  /*-------------------------------------------------------------------------*/

static const unsigned char gFMQSocPatches21Spurs_1[] = \
{
  0x00, 0x02, 0x08, 0x00, 0x06, 0x0C, 0x00, 0x10, 0x0A, 0x00, 0x1A, 0x08, 0x00, 0x22
};
#define FMQSOCPATCHES_21SPURS_1_ADDR          (0x0C00)
#define FMQSOCPATCHES_21SPURS_1_PTR           (&gFMQSocPatches21Spurs_1[0])
#define SIZEOF_FMQSOCPATCHES_21SPURS_1        (sizeof(gFMQSocPatches21Spurs_1) / \
                                              sizeof(gFMQSocPatches21Spurs_1[0]))

static const unsigned char gFMQSocPatches21Spurs_2[] = \
{
  0x10, 0x00, 0x36, 0x0C, 0x00, 0x4E, 0x0C, 0x00, 0x52, 0x10, 0x00, 0x5D, 0x09, 0x00
};

#define FMQSOCPATCHES_21SPURS_2_ADDR          (0x0C0E)
#define FMQSOCPATCHES_21SPURS_2_PTR           (&gFMQSocPatches21Spurs_2[0])
#define SIZEOF_FMQSOCPATCHES_21SPURS_2        (sizeof(gFMQSocPatches21Spurs_2) / \
                                              sizeof(gFMQSocPatches21Spurs_2[0]))

static const unsigned char gFMQSocPatches21Spurs_3[] = \
{
  0x5E, 0x04, 0x00, 0xAA, 0x08, 0x00, 0xBE, 0x04, 0x00, 0xF0, 0x0A, 0x00, 0xF7, 0x07
};

#define FMQSOCPATCHES_21SPURS_3_ADDR          (0x0C1C)
#define FMQSOCPATCHES_21SPURS_3_PTR           (&gFMQSocPatches21Spurs_3[0])
#define SIZEOF_FMQSOCPATCHES_21SPURS_3        (sizeof(gFMQSocPatches21Spurs_3) / \
                                              sizeof(gFMQSocPatches21Spurs_3[0]))

static const unsigned char gFMQSocPatches21Spurs_4[] = \
{
  0x00, 0xFA, 0x08, 0x01, 0x1E, 0x04, 0x01, 0x44, 0x06, 0x01, 0x6A, 0x08, 0x01, 0x90
};

#define FMQSOCPATCHES_21SPURS_4_ADDR          (0x0C2A)
#define FMQSOCPATCHES_21SPURS_4_PTR           (&gFMQSocPatches21Spurs_4[0])
#define SIZEOF_FMQSOCPATCHES_21SPURS_4        (sizeof(gFMQSocPatches21Spurs_4) / \
                                              sizeof(gFMQSocPatches21Spurs_4[0]))

static const unsigned char gFMQSocPatches21Spurs_5[] = \
{
  0x0A, 0x01, 0xB6, 0x0C, 0x01, 0xDD, 0x09, 0x02, 0x02, 0x10, 0x02, 0x2A, 0x08, 0x02
};

#define FMQSOCPATCHES_21SPURS_5_ADDR          (0x0C38)
#define FMQSOCPATCHES_21SPURS_5_PTR           (&gFMQSocPatches21Spurs_5[0])
#define SIZEOF_FMQSOCPATCHES_21SPURS_5        (sizeof(gFMQSocPatches21Spurs_5) / \
                                              sizeof(gFMQSocPatches21Spurs_5[0]))

static const unsigned char gFMQSocPatches21Spurs_6[] = \
{
  0x4E, 0x14, 0x02, 0x77, 0x07
};

#define FMQSOCPATCHES_21SPURS_6_ADDR          (0x0C46)
#define FMQSOCPATCHES_21SPURS_6_PTR           (&gFMQSocPatches21Spurs_6[0])
#define SIZEOF_FMQSOCPATCHES_21SPURS_6        (sizeof(gFMQSocPatches21Spurs_6) / \
                                              sizeof(gFMQSocPatches21Spurs_6[0]))

/**
* Array that stores sizes of FM QSOC spur related data.
*/
static const tsFmQSocPatchesSpurPokeType gFMQSocPatches21Spurs[] = \
{
   {FMQSOCPATCHES_21SPURS_1_PTR, FMQSOCPATCHES_21SPURS_1_ADDR, SIZEOF_FMQSOCPATCHES_21SPURS_1}
  ,{FMQSOCPATCHES_21SPURS_2_PTR, FMQSOCPATCHES_21SPURS_2_ADDR, SIZEOF_FMQSOCPATCHES_21SPURS_2}
  ,{FMQSOCPATCHES_21SPURS_3_PTR, FMQSOCPATCHES_21SPURS_3_ADDR, SIZEOF_FMQSOCPATCHES_21SPURS_3}
  ,{FMQSOCPATCHES_21SPURS_4_PTR, FMQSOCPATCHES_21SPURS_4_ADDR, SIZEOF_FMQSOCPATCHES_21SPURS_4}
  ,{FMQSOCPATCHES_21SPURS_5_PTR, FMQSOCPATCHES_21SPURS_5_ADDR, SIZEOF_FMQSOCPATCHES_21SPURS_5}
  ,{FMQSOCPATCHES_21SPURS_6_PTR, FMQSOCPATCHES_21SPURS_6_ADDR, SIZEOF_FMQSOCPATCHES_21SPURS_6}

};

#define SIZEOF_FMQSOCPATCHES_SPUR21DATA    (sizeof(gFMQSocPatches21Spurs) / \
                                            sizeof(gFMQSocPatches21Spurs[0]))

static const unsigned char gFMQSocPatches2243_20_Poke1[] = \
{
  0x00, 0x00, 0x19, 0x22
};

#define FMQSOCPATCHES_2243_20_POKE_1_ADDR     (0x0BA8)
#define FMQSOCPATCHES_2243_20_POKE_1_PTR       (&gFMQSocPatches2243_20_Poke1[0])
#define SIZEOF_FMQSOCPATCHES_2243_20_POKE_1    (sizeof(gFMQSocPatches2243_20_Poke1) / \
                                              sizeof(gFMQSocPatches2243_20_Poke1[0]))

static const unsigned char gFMQSocPatches2243_20_Poke2[] = \
{
  0x00, 0x00, 0x32, 0x44
};

#define FMQSOCPATCHES_2243_20_POKE_2_ADDR     (0x0BAC)
#define FMQSOCPATCHES_2243_20_POKE_2_PTR       (&gFMQSocPatches2243_20_Poke2[0])
#define SIZEOF_FMQSOCPATCHES_2243_20_POKE_2    (sizeof(gFMQSocPatches2243_20_Poke2) / \
                                              sizeof(gFMQSocPatches2243_20_Poke2[0]))

/**
* Array that stores sizes of FM QSOC spur related data.
*/
static const tsFmQSocPatchesSpurPokeType gFMQSocPatches2243_20_Poke[] = \
{
   {FMQSOCPATCHES_2243_20_POKE_1_PTR, FMQSOCPATCHES_2243_20_POKE_1_ADDR, SIZEOF_FMQSOCPATCHES_2243_20_POKE_1}
  ,{FMQSOCPATCHES_2243_20_POKE_2_PTR, FMQSOCPATCHES_2243_20_POKE_2_ADDR, SIZEOF_FMQSOCPATCHES_2243_20_POKE_2}

};

#define SIZEOF_FMQSOCPATCHES_2243_20_POKEDATA    (sizeof(gFMQSocPatches2243_20_Poke) / \
                                                  sizeof(gFMQSocPatches2243_20_Poke[0]))

/*----------------------------------------------------------------------------
 * Enumerations
 * -------------------------------------------------------------------------*/

typedef enum
{
  EXIT_EDL_MODE      = 0x00,
  ENTER_EDL_MODE     = 0x01

} teFmEdlCtrlType;

/*----------------------------------------------------------------------------
 * Structure and global Definitions
 * -------------------------------------------------------------------------*/
static char transport[PROPERTY_VALUE_MAX];
static char factory_testing_mode[PROPERTY_VALUE_MAX];
/*----------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------*/

/*==============================================================
FUNCTION:  do_rdwr
==============================================================*/

static int do_rdwr(int fd, struct i2c_msg *msgs, int nmsgs) {
   struct i2c_rdwr_ioctl_data msgset = {
      .msgs = msgs,
      .nmsgs = nmsgs,
   };

   if (NULL == msgs || nmsgs <= 0) {
      return -1;
   }

   if (ioctl(fd, I2C_RDWR, &msgset) < 0) {
      return -1;
   }

   return 0;
}



/*==============================================================
FUNCTION:  i2c_read
==============================================================*/
static int i2c_read(int fd, unsigned char offset, unsigned char* buf, int count)
{

        unsigned char offset_data[] =  {offset};
        struct i2c_msg msgs[] = {
           [0] = {
              .addr = TAVARUA_ADDR,
              .flags = 0,
              .buf = (void *)offset_data,
              .len = ARRAY_SIZE(offset_data),
           },
           [1] = {
              .addr = TAVARUA_ADDR,
              .flags = I2C_M_RD,
              .buf = (void *)buf,
              .len = count,
           },
        };

        return do_rdwr(fd, msgs, ARRAY_SIZE(msgs));
}

/*==============================================================
FUNCTION:  i2c_write
==============================================================*/

int i2c_write(int fd, unsigned char offset,const unsigned char* buf, unsigned char len)
{
        unsigned char offset_data[((1 + len) * sizeof(unsigned char))];
        struct i2c_msg msgs[] = {
                [0] = {
                        .addr = TAVARUA_ADDR,
                        .flags = 0,
                        .buf = (void *)offset_data,
                        .len = (1 + len) * sizeof(*offset_data),
                },
        };

        if (NULL == offset_data) {
                return -1;
        }

        offset_data[0] = offset;
        memcpy(offset_data + 1, buf, len);

        return do_rdwr(fd, msgs, ARRAY_SIZE(msgs));
}

/*==============================================================
FUNCTION:  FMQSocPatches_WaitIsr
==============================================================*/
/**
* This function waits until the interrupt bit(s) are set by
* the FM hardware.
*
* @return  int - negative number on failure.
*
*/
static int FMQSocPatches_WaitIsr
(
  int fd,
        unsigned int bitMask,
        unsigned int waitTimeMs
)
{
        int rc;
        unsigned int maxWaitIterations=0;
        unsigned int readData=0;
        unsigned char buf[4] = {0};
        fprintf(debug_file, "FMQSocPatches_WaitIsr: Interrupt Expected: 0x%08x\n", bitMask);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
        while (    ((readData & bitMask) != bitMask)
                                        && (maxWaitIterations < 10)
                          ) {
                usleep(waitTimeMs*1000);
                /* Read the 3 interrupt registers */
                rc = i2c_read(fd, INTSTAT_0, buf, 3);
                if (rc < 0) {
                        return -1;
                }
                readData |= buf[0];
                readData |= buf[1] << 8;
                readData |= buf[2] << 16;
                maxWaitIterations++;
                fprintf(debug_file,"In loop Intr: 0x%08x Expected: 0x%08x\n", readData, bitMask);
        }

        if((readData & bitMask) != bitMask)
        {
               if (is_marimba) {
                   rc = 0;
               } else {
                   rc = -1;
               }
                fprintf(debug_file,"FMQSocPatches_WaitIsr: Intr: 0x%08x Expected: 0x%08x\n", readData, bitMask);
        }
        else
        {
                rc = 0;
        }
        return rc;
}


/*==============================================================
FUNCTION:  marimba_write
==============================================================*/
/**
* This function provides bus interface to write to the Marimba chip
*
* @return  int - negative number on failure.
*
*/
int marimba_write
(
        int fd,
        unsigned char offset,
        const unsigned char* buf,
        unsigned char len
)
{
        unsigned char offset_data[((1 + len) * sizeof(unsigned char))];
        struct i2c_msg msgs[] = {
                [0] = {
                        .addr = MARIMBA_ADDR,
                        .flags = 0,
                        .buf = (void *)offset_data,
                        .len = (1 + len) * sizeof(*offset_data),
                },
        };

        if (NULL == offset_data) {
                return -1;
        }

        offset_data[0] = offset;
        memcpy(offset_data + 1, buf, len);

        return do_rdwr(fd, msgs, ARRAY_SIZE(msgs));
}
/*==============================================================
FUNCTION:  bahama_write
==============================================================*/
/**
* This function provides bus interface to write to the Marimba chip
*
* @return  int - negative number on failure.
*
*/
int bahama_write
(
        int fd,
        unsigned char offset,
        const unsigned char* buf,
        unsigned char len
)
{
        unsigned char offset_data[((1 + len) * sizeof(unsigned char))];
        struct i2c_msg msgs[] = {
                [0] = {
                        .addr = BAHAMA_ADDR,
                        .flags = 0,
                        .buf = (void *)offset_data,
                        .len = (1 + len) * sizeof(*offset_data),
                },
        };

        offset_data[0] = offset;
        memcpy(offset_data + 1, buf, len);

        return do_rdwr(fd, msgs, ARRAY_SIZE(msgs));
}
/*==============================================================
FUNCTION:  enableSoC
==============================================================*/
/**
* This functions brings up the SoC in normal mode
*
* @return  int - negative number on failure.
*
*/
int enableSoC(int i2c_fd)
{

        int rc;
        unsigned char val;
        unsigned char buf[] = {0x00, 0x48, 0x8A, 0x8E, 0x97, 0xB7};

        /* power FM core back up is normal mode */
        val = FM_ENABLE;
        rc = marimba_write(i2c_fd,MARIMBA_XO_BUFF_CNTRL,&val,1);
        if (rc < 0)
        {
		fprintf(debug_file,"MARIMBA_XO_BUFF_CNTRL (FM_ENABLE 0) marimba_write failed\n");
		return rc;
        }
        rc = i2c_write(i2c_fd,LEAKAGE_CNTRL, buf, ARRAY_SIZE(buf));
        if (rc < 0)
        {
                return rc;
        }
        rc  = FMQSocPatches_WaitIsr(i2c_fd, FM_READY_ISR_BITMASK, 50);
        if (rc < 0)
        {
                fprintf(debug_file, "__func__ wait for ready failed\n");
                return rc;
        }

        return 0;

}

int bringup_fmsoc( int i2c_fd, unsigned char fm_mode )
{
    //Fm calibraion data for Bahama
    unsigned char fm_ctl0_part1[] = { 0xCA, 0xCE, 0xD6 };
    unsigned char fm_ctl1[] ={ 0x03 };
    unsigned char fm_ctl0_part2[] = { 0xB6, 0xB7 };
    unsigned char val;
    int retval,rc=0,i;

    /* Power down FM at bahama level */
    if( fm_mode != NORMAL_MODE ) {
       /*Don't need to Power off and on
        in Normal mode
       */
       val = 0x00;
       rc = bahama_write(i2c_fd,BAHAMA_RBIAS_CTL1,&val,1);
       if (rc < 0)
       {
          fprintf(debug_file, "BAHAMA_RBIAS_CTL1 bahama_write (off) failed\n");
          return rc;
       }

       /* Power up FM at marimba level */
       val = FM_ENABLE;
       rc = bahama_write(i2c_fd,BAHAMA_RBIAS_CTL1,&val,1);
       if (rc < 0)
       {
          fprintf(debug_file, "BAHAMA_RBIAS_CTL1 (FM_ENABLE 0) bahama_write failed\n");
          return rc;
       }
    }
    //write FM mode
    retval = i2c_write(i2c_fd, BAHAMA_FM_MODE_REG, &fm_mode, 1);
    if (retval < 0) {
        fprintf(debug_file, "%s: failed to set the FM mode\n", __func__);
        return -1;
    }
    //Write first sequence of bytes to FM_CTL0
    for( i =0; i<3; i++ )   {
        retval = i2c_write(i2c_fd,BAHAMA_FM_CTL0_REG,&fm_ctl0_part1[i],1 );
        if (retval < 0) {
            fprintf(debug_file, "%s: failed to write the bring up sequence to FM_CTL0:1st set\n",
                    __func__);
            return -1;
	}
    }
    //Write the FM_CTL1 sequence
    for( i =0; i<1; i++ )   {
        retval = i2c_write(i2c_fd, BAHAMA_FM_CTL1_REG, &fm_ctl1[i], 1 );
        if (retval < 0) {
            fprintf(debug_file, "%s: failed to write the bring up sequence to FM_CTL1\n",
                    __func__);
            return -1;
        }
    }
    //Write second sequence of bytes to FM_CTL0
    for( i =0; i<2; i++ )   {
        retval = i2c_write(i2c_fd, BAHAMA_FM_CTL0_REG, &fm_ctl0_part2[i], 1 );
        if (retval < 0) {
            fprintf(debug_file, "%s: failed to write the bring up sequence to FM_CTL0:2nd set\n",
                    __func__);
            return -1;
        }
    }

    rc  = FMQSocPatches_WaitIsr(i2c_fd, FM_READY_ISR_BITMASK, 200 );
    if (rc < 0)
    {
            fprintf(debug_file, "wait for ready failed : cal mode: %d\n",fm_mode );
            return -1;
    }
    fprintf(debug_file, "%s: Returning from mode: %x:: val :%d\n",
                    __func__, fm_mode, rc);

    return rc;
}


int get_cal_data_bahama( int i2c_fd )
{
    int rc =0, i;
    int cal_file;
    unsigned char val;
    unsigned char cal_bufs[CAL_DAT3_BUF_NUM][XFR_REG_NUM];

    //Bring up the FM in Process calibration mode
    fprintf(debug_file, "Process calibration initiated\n");

    rc = bringup_fmsoc( i2c_fd, PROCESS_CAL_MODE );
    if (rc < 0)
    {
        fprintf(debug_file, "Process calibration failed\n");
        return rc;
    }
    //Read the Calibration data of Process calib
    /* initiate CAL_DAT0 xfr */
    val = CAL_DAT_0;
    rc = i2c_write(i2c_fd,XFRCTRL,&val,1);
    if (rc < 0)
    {
            fprintf(debug_file, "CAL_DAT_0 i2c_write failed\n");
            return rc;
    }
    //Wait for Transfer interrupt to occur.
    rc  = FMQSocPatches_WaitIsr(i2c_fd, FM_TRANSFER_ISR_BITMASK, 10);
    if (rc < 0)
    {
            fprintf(debug_file, "CAL_DAT_0 wait for xfr failed\n");
            return rc;
    }
    /* Read 16 bytes of calibration data into */
    rc = i2c_read(i2c_fd, XFRDAT0, (cal_bufs[CAL_DAT0_BUF_INDEX]), XFR_REG_NUM);
    if (rc < 0)
    {
            fprintf(debug_file, "XFRDAT0 i2c_read failed\n");
            return rc;
    }

    //Bring up the FM in DCcalibration mode
    rc = bringup_fmsoc( i2c_fd, DC_CAL_MODE );
    if (rc < 0)
    {
        fprintf(debug_file, "DC calibration failed\n");
        return rc;
    }
    //Read the Calibration data of DC calib
    for(i=0;i<3;i++) {
        /* initiate CAL_DAT0 xfr */
        val = CAL_DAT_1 + i;
        rc = i2c_write(i2c_fd,XFRCTRL,&val,1);
        if (rc < 0)
        {
                fprintf(debug_file, "CAL_DAT_%d i2c_write failed\n", i+1);
                return rc;
        }
        //Wait for Transfer interrupt to occur.
        rc  = FMQSocPatches_WaitIsr(i2c_fd, FM_TRANSFER_ISR_BITMASK, 10);
        if (rc < 0)
        {
                fprintf(debug_file, "CAL_DAT_%d wait for xfr failed\n", i+1);
                return rc;
        }
        /* Read 16 bytes of calibration data into */
        rc = i2c_read(i2c_fd, XFRDAT0, (cal_bufs[CAL_DAT1_BUF_INDEX+i]), XFR_REG_NUM);
        if (rc < 0)
        {
                fprintf(debug_file, "CAL_DAT_%d read XFRDAT0 failed\n", i+1);
                return rc;
        }
    }
    //Bring up the FM in Normal mode
    rc = bringup_fmsoc( i2c_fd, NORMAL_MODE );
    if (rc < 0)
    {
        fprintf(debug_file, "Normal mode on failed\n");
        return rc;
    }
    /* write calibration data to a file */
    cal_file = open("/data/app/fm_cal", O_RDWR | O_CREAT, S_IRUSR | S_IRGRP);
    if (cal_file < 0)
    {
                    return cal_file;
    }

    for(i=0; i<CAL_DAT3_BUF_NUM; i++) {
            if (write(cal_file,cal_bufs[i],XFR_REG_NUM) < XFR_REG_NUM) {
                    close(cal_file);
                    return -1;
            }
    }
    fprintf(debug_file, "perform calibration complete\n");

    close(cal_file);
    return 0;
}

/*==============================================================
FUNCTION:  get_cal_data
==============================================================*/
/**
* This function extracts calibration data in case it does not exists
*
* @return  int - negative number on failure.
*
*/
int get_cal_data_marimba(int i2c_fd)
{
        int i;
        int rc = 0;
        int cal_file;
        unsigned char radio_mode[] = {0x00, 0x48, 0x8A, 0x8E, 0x97, 0xB7};
        unsigned char cal_bufs[CAL_DAT3_BUF_NUM][XFR_REG_NUM];
        unsigned char val;

	/* Power down FM at marimba level */
        val = 0x00;
        rc = marimba_write(i2c_fd,MARIMBA_XO_BUFF_CNTRL,&val,1);
        if (rc < 0)
        {
                  fprintf(debug_file, "MARIMBA_XO_BUFF_CNTRL marimba_write (off) failed\n");
                        return rc;
        }

	/* Power up FM at marimba level */
        val = FM_ENABLE;
        rc = marimba_write(i2c_fd,MARIMBA_XO_BUFF_CNTRL,&val,1);
        if (rc < 0)
        {
                        fprintf(debug_file, "MARIMBA_XO_BUFF_CNTRL (FM_ENABLE 0) marimba_write failed\n");
                        return rc;
        }

        /* put FM core is process calibration mode */
        radio_mode[0] = PROCESS_CAL_MODE;
        rc = i2c_write(i2c_fd,LEAKAGE_CNTRL,radio_mode,ARRAY_SIZE(radio_mode));
        if (rc < 0)
        {
                  fprintf(debug_file, "LEAKAGE_CNTRL (process cal) i2c_write failed\n");
                        return rc;
        }
        //Wait for Ready Interrupt to occur.
        rc  = FMQSocPatches_WaitIsr(i2c_fd, FM_READY_ISR_BITMASK, 50);
        if (rc < 0)
        {
                fprintf(debug_file, "LEAKAGE_CNTRL (process cal) wait for ready failed\n");
                return rc;
        }

        /* initiate CAL_DAT0 xfr */
        val = CAL_DAT_0;
        rc = i2c_write(i2c_fd,XFRCTRL,&val,1);
        if (rc < 0)
        {
                fprintf(debug_file, "CAL_DAT_0 i2c_write failed\n");
                return rc;
        }
        //Wait for Transfer interrupt to occur.
        rc  = FMQSocPatches_WaitIsr(i2c_fd, FM_TRANSFER_ISR_BITMASK, 10);
        if (rc < 0)
        {
                fprintf(debug_file, "CAL_DAT_0 wait for xfr failed\n");
                return rc;
        }
        /* Read 16 bytes of calibration data into */
        rc = i2c_read(i2c_fd, XFRDAT0, (cal_bufs[CAL_DAT0_BUF_INDEX]), XFR_REG_NUM);
        if (rc < 0)
        {
                fprintf(debug_file, "XFRDAT0 i2c_read failed\n");
                return rc;
        }

        /* power down FM core */
        val = 0x00;
        rc = marimba_write(i2c_fd,MARIMBA_XO_BUFF_CNTRL,&val,1);
        if (rc < 0)
        {
                  fprintf(debug_file, "MARIMBA_XO_BUFF_CNTRL marimba_write (off) failed\n");
                        return rc;
        }

        /* power FM core back on */
        val = FM_ENABLE;
        rc = marimba_write(i2c_fd,MARIMBA_XO_BUFF_CNTRL,&val,1);
        if (rc < 0)
        {
                  fprintf(debug_file, "MARIMBA_XO_BUFF_CNTRL marimba_write (on) failed\n");
                        return rc;
        }

        /* put FM core in DC calibration mode */
        radio_mode[0] = DC_CAL_MODE;
        rc = i2c_write(i2c_fd,LEAKAGE_CNTRL,radio_mode,ARRAY_SIZE(radio_mode));
        if (rc < 0)
        {
                  fprintf(debug_file, "LEAKAGE_CNTRL (dc cal) i2c_write failed\n");
                        return rc;
        }

        //Wait for Ready Interrupt to occur.
        rc  = FMQSocPatches_WaitIsr(i2c_fd, FM_READY_ISR_BITMASK, 200);
        if (rc < 0)
        {
                        fprintf(debug_file, "LEAKAGE_CNTRL (dc cal) wait for ready failed\n");
                        return rc;
        }

        for(i=0;i<3;i++) {
                /* initiate CAL_DAT0 xfr */
                val = CAL_DAT_1 + i;
                rc = i2c_write(i2c_fd,XFRCTRL,&val,1);
                if (rc < 0)
                {
                        fprintf(debug_file, "CAL_DAT_%d i2c_write failed\n", i+1);
                        return rc;
                }
                //Wait for Transfer interrupt to occur.
                rc  = FMQSocPatches_WaitIsr(i2c_fd, FM_TRANSFER_ISR_BITMASK, 10);
                if (rc < 0)
                {
                        fprintf(debug_file, "CAL_DAT_%d wait for xfr failed\n", i+1);
                        return rc;
                }
                /* Read 16 bytes of calibration data into */
                rc = i2c_read(i2c_fd, XFRDAT0, (cal_bufs[CAL_DAT1_BUF_INDEX+i]), XFR_REG_NUM);
                if (rc < 0)
                {
                        fprintf(debug_file, "CAL_DAT_%d read XFRDAT0 failed\n", i+1);
                        return rc;
                }
        }

        /* power down FM core */
        //val = 0x00;
        //marimba_write(i2c_fd,MARIMBA_XO_BUFF_CNTRL,&val,1);
        /* power FM core back up is normal mode */
        //val = FM_ENABLE;
        //marimba_write(i2c_fd,MARIMBA_XO_BUFF_CNTRL,&val,1);
        radio_mode[0] = NORMAL_MODE;
        rc = i2c_write(i2c_fd,LEAKAGE_CNTRL,radio_mode,1);
        if (rc < 0)
        {
                return rc;
        }

        /* write calibration data to a file */
        cal_file = open("/data/app/fm_cal", O_RDWR | O_CREAT, S_IRUSR | S_IRGRP);
        if (cal_file < 0)
        {
                        return cal_file;
        }

        for(i=0; i<CAL_DAT3_BUF_NUM; i++) {
                if (write(cal_file,cal_bufs[i],XFR_REG_NUM) < XFR_REG_NUM) {
                        close(cal_file);
                        return -1;
                }
        }
        fprintf(debug_file, "perform calibration complete\n");

        close(cal_file);
        return 0;
}

static int chip_is_Marimba(int  chipVersion)
{
	int is_marimba = -1;

	switch(chipVersion){

        case FM6500_A0_VERSION:
        case FMQSOCCOM_FM6500_20_VERSION:
        case FMQSOCCOM_FM6500_21_VERSION:
                is_marimba = 1;
                break;
        case FMQSOCCOM_FM6500_WCN2243_10_VERSION:
	case FMQSOCCOM_FM6500_WCN2243_20_VERSION:
        case FMQSOCCOM_FM6500_WCN2243_21_VERSION:
		is_marimba = 0;
		break;
	default:
		fprintf(debug_file, "Unknown Chip version\n");
		is_marimba = -1;
        }
	return is_marimba;
}

int get_cal_data(int i2c_fd, int chipVer )
{
	int retVal = -1;
	int is_marimba = -1;

	retVal = chip_is_Marimba(chipVer);
	if(is_marimba == -1 )
	{
		return retVal;
	}
	if(is_marimba){
		retVal = get_cal_data_marimba( i2c_fd );
	}
	else {
		 retVal = get_cal_data_bahama( i2c_fd );
	}

	return retVal;
}

/*==============================================================
FUNCTION:  FmQSocPatches_EDLCoreRegWr
==============================================================*/
/**
* This function provides bus interface to FM SOC while in Embedded Downloader
* mode. It also handles corresponding FM soc alert to the host (TRANSFER bit
* in INTSTAT3)
*
* @see     FmQSocPatches_WritePatches
*
* @return  int - negative number on failure.
*
*/
static int  FmQSocPatches_EDLCoreRegWr
(
  int                       fd,
  const unsigned char *regWrPtr,
  /**< [in] Pointer to unsigned char array from where the data will be written
       to the bus.
  */
  unsigned char       startAddr,
  /**< [in] Start address where the data will be written on the bus
  */
  unsigned char       length
  /**< [in] Length of the data to be written on the bus
  */
)
{
  int rc;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /* Now Transmit bytes over bus */
  rc  = i2c_write(fd, startAddr, regWrPtr, length);
  return rc;
}

static int FmQSocCom_XfrModeDataWr
(
   int fd,
   unsigned char  *fmRegWrPtr,
   unsigned char   wrLen
)
{

  int  rc;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Now Transmit bytes over I2C bus */
  rc = i2c_write(fd, XFRCTRL, fmRegWrPtr, wrLen);

  if (rc < 0) {
    close (fd);
    return rc;
  }

  //Wait for Transfer interrupt to occur.
  rc  = FMQSocPatches_WaitIsr(fd, FM_TRANSFER_ISR_BITMASK, 10);

  if (rc < 0) {
    close (fd);
    return rc;
  }

  return rc;


}

/*==============================================================
FUNCTION:  FMQSocPatches_EnterEDLMode
==============================================================*/
/**
* EDL mode can be started and exited by writing to the EDLMODE register.
* While not in embedded download mode, the host can write a 0x01 to the
* EDLMODE register to enter embedded download mode.
*
* @return  int - negative number on failure.
*
*/
static int FMQSocPatches_EnterEDLMode
(
  int fd

)
{
        int rc;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
        unsigned char buf[] = {ENTER_EDL_MODE};
        rc  = i2c_write(fd,FM_QSOC_EDLMODE_INDEX,buf,ARRAY_SIZE(buf));
        //Wait for Transfer interrupt to occur.
        rc  = FMQSocPatches_WaitIsr(fd, FM_TRANSFER_ISR_BITMASK, 10);
        return rc;
}

/*==============================================================
FUNCTION:  FMQSocPatches_ExitEDLMode
==============================================================*/
/**
* The host can exit embedded download mode by writing a 0x00 to
* EDLMODE while in embedded download mode.
*
* @return  int - negative number on failure.
*
*/
static int FMQSocPatches_ExitEDLMode
(
  int fd

)
{
        int rc;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  unsigned char buf[] = {EXIT_EDL_MODE};
  rc  = i2c_write(fd,FM_QSOC_EDLMODE_INDEX,buf,ARRAY_SIZE(buf));
        //Wait for Transfer interrupt to occur.
        rc  = FMQSocPatches_WaitIsr(fd, FM_TRANSFER_ISR_BITMASK, 10);
        return rc;
}




static int FMQSocPatches_DisableIntrs_TEST
(
  int fd, int chipVersion

)
{
        int rc = -1;
	unsigned char buf[3] = {0, 0, 0};
	unsigned char xfrmode=0;

	if( chipVersion == FMQSOCCOM_FM6500_21_VERSION ||  chipVersion == FMQSOCCOM_FM6500_WCN2243_10_VERSION
			|| chipVersion == FMQSOCCOM_FM6500_WCN2243_20_VERSION
                        || chipVersion == FMQSOCCOM_FM6500_WCN2243_21_VERSION) {
		rc  = i2c_write(fd, XFRDAT0, buf, ARRAY_SIZE(buf));
		if( rc < 0 ) {
			fprintf(debug_file, "failed to Write the XFRDAT0 to XFRDATA2 regs %d\n", rc);
			return rc;
		}
		xfrmode = XFRCTL_WRITE_MODE|INT_CTRL;
		rc  = i2c_write(fd, XFRCTRL, &xfrmode, 1);
		if( rc < 0 ) {
			fprintf(debug_file, "failed to Write the XFRCTRL reg:  %d\n", rc);
			return rc;
		}
		usleep(10*1000);
	}
	else {
		rc  = i2c_write(fd,INTSTAT_0,buf,ARRAY_SIZE(buf));
		usleep(10*1000);
	}

        return rc;
}

static int FMQSocPatches_EnableIntrs_TEST
(
  int fd, int chipVersion
)
{
        int rc = -1;
        unsigned char buf[3] = {0xFF, 0x38, 0x81};
	unsigned char xfrmode=0;

        if( chipVersion == FMQSOCCOM_FM6500_21_VERSION || chipVersion == FMQSOCCOM_FM6500_WCN2243_10_VERSION
		|| chipVersion == FMQSOCCOM_FM6500_WCN2243_20_VERSION
                || chipVersion == FMQSOCCOM_FM6500_WCN2243_21_VERSION) {
                rc  = i2c_write(fd, XFRDAT0, buf, ARRAY_SIZE(buf));
                if( rc < 0 ) {
                        fprintf(debug_file, "failed to Write the XFRDAT0 to XFRDATA2 regs %d\n", rc);
                        return rc;
                }
		xfrmode = XFRCTL_WRITE_MODE|INT_CTRL;
		rc  = i2c_write(fd, XFRCTRL, &xfrmode, 1);

		if( rc < 0 ) {
			fprintf(debug_file, "failed to Write the XFRCTRL reg:  %d\n", rc);
			return rc;
		}
		//Wait for the XFR Transfer event
		rc = FMQSocPatches_WaitIsr(fd, FM_TRANSFER_ISR_BITMASK, TIMEOUT_DURATION);
		if( rc < 0 ) {
			fprintf(debug_file, "failed to to get the XFR transfer interrupt:  %d\n", rc);
			return rc;
		}
	}
	else {
		rc  = i2c_write(fd,INTSTAT_0,buf,ARRAY_SIZE(buf));
		usleep(10*1000);
	}
	return rc;
}

/*==============================================================
FUNCTION:  FmQSocCom_Poke
==============================================================*/
/**
*Write data directly to the FM HW
*
* @return  int - negative number on failure.
*
*/

int FmQSocCom_Poke
(
   int fd,
   const tsFmQSocPatchesSpurPokeType *spur_data
)
{
  int retval =0, i;
  unsigned char mem_acc_mode =0;
  unsigned char xfrRegs[XFR_REG_NUM + 3];
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if(NULL == spur_data) {
     return -1;
  }
  //Reset the Xfrregs
  memset(xfrRegs, 0, XFR_REG_NUM+1);

  /*Set the Write, Mem Access and Memory access mode
  for the xfr registers
  Also write the length for mem access
  */
  mem_acc_mode = (FM_QSOC_MEMACC_CTRL_WRITE|FM_QSOC_XFRCTRL_MEMACCESS|FM_QSOC_XFRCTRL_MEMMODE_BLOCK);
  mem_acc_mode = mem_acc_mode | (((spur_data->spurSize)&FM_QSOC_MEMACC_LENGTH_MASK)<<1);

  xfrRegs[0] = mem_acc_mode;
  /* Write the MSB and LSB of the starting address
   to the XFRDATA0 and XFRDATA1 registers
   */
  xfrRegs[1] =( (0xff00&spur_data->spurPokeAddr)>>8 );
  xfrRegs[2] = (0x00ff&spur_data->spurPokeAddr);

  /*Copy the SPUR values to XFRDATA3 to XFRDATA16
   */
  if(spur_data->spurSize <= XFR_REG_NUM) {
     memmove(&xfrRegs[3], spur_data->spurPtr, spur_data->spurSize);
  }else {
     return -1;
  }
  fprintf(debug_file, "xfrRegs[0]: XFRCTRL MODE\t = 0x%x\n", xfrRegs[0]);
  fprintf(debug_file, "xfrRegs[1]: MSB of Reg.Addr.\t = 0x%x\n", xfrRegs[1]);
  fprintf(debug_file, "xfrRegs[2]: LSB of Reg.Addr.\t = 0x%x\n", xfrRegs[2]);

  for( i = 3; i < (spur_data->spurSize + 3); i++ )
	fprintf(debug_file, "xfrRegs[%d]: DATA = 0x%x\n", i, xfrRegs[i]);

  /* Now Transmit the XFRCTRL, XFRDATA0 to XFRDATA15 over I2C */
  retval = i2c_write(fd, XFRCTRL, xfrRegs, XFR_REG_NUM+1);

  if (retval < 0) {
    fprintf(debug_file, "I2C Write failure for XFRCTRL, XFRDATA0 to XFRDATA15\n");
    return retval;
  }

  /* We wait for transfer interrupt, if we receive some other interrupt wait
   * again
  */
  retval = FMQSocPatches_WaitIsr(fd, FM_TRANSFER_ISR_BITMASK, 10);
  return retval;
}


int band_may_affect_fm_rf( int current_band )
{
	int retval = 0;

	switch(current_band)	{
	case 4:
	case 6:
	case 8:
	case 14:
	case 15:
	case 16:
	case 17:
	case 47:
	case 48:
	case 80:
	case 81:
	case 82:
	case 83:
	case 88:
	case 90:
	case 110:
	case 111:
	case 112:
	case 113:
	case 114:
	case 115:
	case 116:
	case 117:
	case 118:
	case 119:
	case 120:
	case 121:
	case 122:
	case 123:
	case 126:
	case 128:
	case 129:
	case 130:
	case 135:
	case 136:
	case 137:
	case 138:
	case 139:
	case 140:
	case 141:
	case 142:/*In all these case, band may cross 1GHZ*/
		retval = 1;
	break;
	default:
		retval = 0;
	}

	return retval;
}

int set_notch_filter(int fd, int low_nf_value, int high_nf_value )
{
	unsigned char buf = 0;
	int rc = 0;

	rc = i2c_read(fd, LPNOTCHREG, &buf, 1);
	if(rc < 0 ){
		fprintf(debug_file, "Error: not able to read LPNOTCHREG\n");
		return rc;
	}
	if (low_nf_value)
		buf = buf | (1<<3);
	else
		buf = buf & ~(1<<3);

	rc = i2c_write(fd, LPNOTCHREG,&buf, 1 );
	if(rc < 0 ){
		fprintf(debug_file, "Error:Not able to write to LPNOTCHREG ");
		return rc;
	}
	rc = i2c_read(fd, HPNOTCHREG, &buf, 1);
	if(rc < 0 ){
		fprintf(debug_file, "Error: not able to read HPNOTCHREG\n");
		return rc;
	}
	if (high_nf_value)
		buf = buf | (1<<3);
	else
		buf = buf & ~(1<<3);

	rc = i2c_write(fd, HPNOTCHREG,&buf, 1 );

	if(rc < 0 ){
		fprintf(debug_file, "Error:Not able to write to HPNOTCHREG ");
		return rc;
	}
	return rc;
}
int set_desired_notch_filter(int fd, int wcm_mode)	{

	qmi_client_error_type qmi_client_err;
	qmi_client_type my_qmi_client_handle;
	nas_get_rf_band_info_resp_msg_v01 resp_msg;
	qmi_idl_service_object_type nas_service;
	int i = 0;

	if(wcm_mode == 2)	{
		/*Reset Notch filters*/
		fprintf(debug_file, "Resetting the notchfilters");
		if ( (3 == strlen(transport)) && ( !strncmp("smd", transport, 3)) ) {
			property_set("notch.value","NONE");
		}
		else
			set_notch_filter(fd, 0, 0);
	}
	else	{
		/*Initialize the qmi handle*/
		int qmi_handle = qmi_init(NULL, NULL);

		if( qmi_handle < 0 )    {
			fprintf(debug_file, "qmi_init() failure: %d\n", qmi_handle);
			return -1;
		}

		nas_service = nas_get_service_object_v01();

		qmi_client_err = qmi_client_init(QMI_PORT_RMNET_0, nas_service, NULL, NULL, &my_qmi_client_handle);

		if(qmi_client_err != QMI_NO_ERR){
			fprintf(debug_file, "Error: while Initializing qmi_client_init : %d\n", qmi_client_err);
			if(my_qmi_client_handle != NULL) {
				free(my_qmi_client_handle);
			}
			return -1;
		}

		qmi_client_err = qmi_client_send_msg_sync(my_qmi_client_handle,
			QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01, NULL, 0, &resp_msg, sizeof(resp_msg), 500/*TIMEOUT*/);

		if(qmi_client_err != QMI_NO_ERR){
			fprintf(debug_file, "Error: while getting the QMI_NAS_GET_RF_BAND_INFO: %d\n", qmi_client_err);
			return -1;
		}

		//Got the band info
		if(resp_msg.rf_band_info_list_len > 1)	{
			fprintf(debug_file, "Error: can't determine if more than one band is serving! Is this possible!?\n");
			return -1;
		}
	        for ( i = 0; i < (int)resp_msg.rf_band_info_list_len; i++ )
		{
	                fprintf(debug_file, "%d: radio_if: %d  active_band: %d\n",
				i, resp_msg.rf_band_info_list[i].radio_if, resp_msg.rf_band_info_list[i].active_band);
	        }

		//Cleauo QMI stuff
	        qmi_client_err = qmi_client_release(my_qmi_client_handle);
	        if(qmi_client_err != QMI_NO_ERR){
			fprintf(debug_file, "Error: while releasing qmi_client : %d\n", qmi_client_err);
			//Ignore cleanup error
		}

	        qmi_handle = qmi_release(qmi_handle);
		if( qmi_handle < 0 )    {
			fprintf(debug_file, "Error: while releasing qmi %d\n", qmi_handle);
			//Ignore cleanup errors
		}

		if(band_may_affect_fm_rf(resp_msg.rf_band_info_list[resp_msg.rf_band_info_list_len-1].active_band))	{
			fprintf(debug_file, "It is High band");
			if( (3 == strlen(transport)) && ( !strncmp("smd", transport, 3)) ) {
				property_set("notch.value", "HIGH");
			}
			else
				set_notch_filter(fd, 0, 1);

		} else	{
			fprintf(debug_file, "It is Low band");
			if( (3 == strlen(transport)) && ( !strncmp("smd", transport, 3)) ) {
				property_set("notch.value", "LOW");
			}
			else
				set_notch_filter(fd, 1, 0);
		}
	}
	return 0;
}
char * detect_device(void)
{
	char * fm_i2c_path = NULL;
	char product_board_platform_type[PROPERTY_VALUE_MAX] = {0};

	property_get("ro.board.platform", product_board_platform_type, NULL);

	if(!strncmp("msm8660",product_board_platform_type,strlen(product_board_platform_type)))
		fm_i2c_path = (char *)fm_i2c_path_8x60_SURF;
	else if(!strncmp("msm7630_surf",product_board_platform_type,strlen(product_board_platform_type)))
		fm_i2c_path = (char *)fm_i2c_path_7x30_SURF;
	else if(!strncmp("msm7627a",product_board_platform_type,strlen(product_board_platform_type)))
		fm_i2c_path = (char *)fm_i2c_path_7x27A_SURF;
	return fm_i2c_path;
}

/*===================================================================
			DAC Configuration
====================================================================*/

typedef struct dacConfig
{
	unsigned char regAddr;
	unsigned char data;
}dacConfig;

/* Digital DAC config registers */
#define CDC_RESET_CTRL       0xD0
#define CDC_RX1_CTL          0xD1
#define CDC_CH_CTL1          0xD2
#define CDC_LRXG1            0xD3
#define CDC_RRXG1            0xD4
#define CDC_RX_PGA_TIMER     0xD5
#define CDC_GCTL1            0xD6
#define CDC_LRX_DCOFFSET1    0xD7
#define CDC_RRX_DCOFFSET1    0xD8
#define CDC_BYPASS_CTL1      0xD9

/* Analog DAC config registers */
#define DAC_CONFIG_REG_1     0xA4
#define DAC_CONFIG_REG_2     0xA5
#define DAC_CONFIG_REG_3     0xA6
#define DAC_CONFIG_REG_4     0xA7
#define DAC_CONFIG_REG_5     0xA8

#define SIZE_OF_STRUCT(x) (sizeof(x) / sizeof((x)[0]))

static const dacConfig gFMDACInitConfigData[] = \
{
	{BAHAMA_FM_CTL1_REG, 0x01},
	{BAHAMA_FM_CTL1_REG, 0x05},
	{CDC_RESET_CTRL,     0x01},
	{CDC_RESET_CTRL,     0x00}
};

static const dacConfig  gFMDACDigitalConfigData[] = \
{
	{CDC_RESET_CTRL,    0x00},
	{CDC_RX1_CTL,       0x00},
	{CDC_CH_CTL1,       0x07},
	{CDC_LRXG1,         0x00},
	{CDC_RRXG1,         0x00},
	{CDC_RX_PGA_TIMER,  0x00},
	{CDC_GCTL1,         0x0C},
	{CDC_LRX_DCOFFSET1, 0x00},
	{CDC_RRX_DCOFFSET1, 0x00},
	{CDC_BYPASS_CTL1,   0x05}
};

static const dacConfig gFMDACAnalogConfigData[] = \
{
	{DAC_CONFIG_REG_5, 0x17},
	{DAC_CONFIG_REG_1, 0x53},
	{DAC_CONFIG_REG_2, 0x53},
	{DAC_CONFIG_REG_3, 0x01},
	{DAC_CONFIG_REG_4, 0x01}
};

static const dacConfig gFMDACDisableData[] = \
{
	{BAHAMA_FM_CTL1_REG, 0x01},
	{BAHAMA_FM_CTL1_REG, 0x05},
	{CDC_RESET_CTRL,     0x01},
	{CDC_RESET_CTRL,     0x00},
	{CDC_CH_CTL1,        0x00} // Disable CLK for DAC
};


int FmDacInit(int fd)
{
	int i=0;
	int ret = 0;
	unsigned int waitTime = 20; // in millisecs
	int size = SIZE_OF_STRUCT(gFMDACInitConfigData);
	for (i=0; i<size; i++){
		ret = i2c_write(fd, gFMDACInitConfigData[i].regAddr, &gFMDACInitConfigData[i].data, 1);
		if (ret < 0){
			fprintf(debug_file, "%s: i2c_write failed for %d \n", __func__, i);
			return -1;
		}
		usleep(waitTime*1000);
	}
	return 0;
}

int FmDacDisable(int fd)
{
	int i=0;
	int ret = 0;
	unsigned int waitTime = 20; // in millisecs
	int size = SIZE_OF_STRUCT(gFMDACDisableData);
	for (i=0; i<size; i++){
		ret = i2c_write(fd, gFMDACDisableData[i].regAddr, &gFMDACDisableData[i].data, 1);
		if (ret < 0){
			fprintf(debug_file, "%s: i2c_write failed for %d \n", __func__, i);
			return -1;
		}
		usleep(waitTime*1000);
	}
	return 0;
}


int FmDacCodecDigitalConfig( int fd )
{
	int i=0;
	int ret=0;
	unsigned int waitTime = 20; // in millisecs
	int size = SIZE_OF_STRUCT(gFMDACDigitalConfigData);
	for (i=0;i<size;i++){
		ret = i2c_write(fd, gFMDACDigitalConfigData[i].regAddr, &gFMDACDigitalConfigData[i].data, 1);
		if (ret < 0){
			fprintf(debug_file, "%s : i2c_write failed for %d \n", __func__, i);
			return -1;
		}
		usleep(waitTime*1000);
	}
	return 0;
}

int FmDacCodecAnalogConfig( int fd )
{
	int i=0;
	int ret=0;
	unsigned int waitTime = 20; //in millisecs
	int size = SIZE_OF_STRUCT(gFMDACAnalogConfigData);
	for (i=0; i<size; i++){
		ret = i2c_write(fd, gFMDACAnalogConfigData[i].regAddr, &gFMDACAnalogConfigData[i].data, 1);
		if (ret < 0){
			fprintf(debug_file, "%s : i2c_write failed for %d \n", __func__, i);
			return -1;
		}
		usleep(waitTime*1000);
	}
	return 0;
}

int FmConfigueDAC( int fd )
{
	int rc = 0;
	/* Reseting FM DAC */
	rc = FmDacInit(fd);
	if (rc < 0){
		fprintf(debug_file, "Reset FM Dac : Failed\n");
		return -1;
	}
	/*DAC digital configuration*/
	rc = 	FmDacCodecDigitalConfig(fd);
	if (rc < 0){
		fprintf(debug_file, "DAC Digital Config : Failed\n");
		return -1;
	}

	/*DAC Analog configuration*/
	rc = 	FmDacCodecAnalogConfig(fd);
	if (rc < 0){
		fprintf(debug_file, "DAC Analog Config : Failed\n");
		return -1;
	}
	return 0;
}

int isAnalogpathSupported()
{
	char fm_low_power[PROPERTY_VALUE_MAX] = {0};
	property_get("ro.fm.analogpath.supported", fm_low_power, "false");
	fprintf(debug_file,"ro.fm.analogpath.supported = %s\n", fm_low_power);
	if( 0 == strcmp(fm_low_power,"true"))
		return 1;
	else
		return 0;
}

int  do_riva_calibration(){
	char  *str;
	int err;
	FILE* calib_file = NULL;
	int fd;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer v4l2_buf;
	memset(&reqbuf, 0, sizeof (reqbuf));
	enum v4l2_buf_type type = V4L2_BUF_TYPE_PRIVATE;
	reqbuf.type = V4L2_BUF_TYPE_PRIVATE;
	reqbuf.memory = V4L2_MEMORY_USERPTR;
	memset(&v4l2_buf, 0, sizeof (v4l2_buf));
	v4l2_buf.index = CAL_DATA_BUF;
	v4l2_buf.type = type;
	v4l2_buf.length = V4L2_BUF_LENGTH;
	struct v4l2_control control;

	str = (char*)malloc(CAL_DATA_SIZE);
	if(str == NULL)
		return -1 ;
	memset ( str, 0x00, CAL_DATA_SIZE);

	fd = open(RADIO_DEVICE_PATH, O_RDONLY, O_NONBLOCK);
	if(fd < 0)
	{
                free(str);
		return -1;
	}
	/*Added to allow insmod operation to complete.
	 will remove once static linking is done */
	sleep(1);
	control.id = V4L2_CID_PRIVATE_IRIS_DO_CALIBRATION;
	err = ioctl(fd,VIDIOC_G_CTRL,&control);
	if(err >= 0)
	{
		v4l2_buf.m.userptr = (unsigned long)str;
		err = ioctl(fd,VIDIOC_DQBUF,&v4l2_buf);
		if(err >= 0)
		{
			calib_file = fopen(CAL_FILE_PATH, "w+");
			if (calib_file == NULL)
			{
				goto error_handle ;
		        } else if (fwrite(str,1, CAL_DATA_SIZE,
					calib_file) < CAL_DATA_SIZE)
			{
		           fclose(calib_file);
		           goto error_handle;
                        } else {
                                fclose(calib_file);
                                close(fd);
                                chmod(CAL_FILE_PATH, 0644);
                                free(str);
                                return 0;
                        }
		}
	}
	error_handle:
	close(fd);
	free(str);

	return -1;
}


/*==============================================================
FUNCTION:  main
==============================================================*/
/**
* This function enables Host processor to Apply a ROM patch into the Core's
* program RAM. Host can apply up to 16 ROM patches (0-15) to override code in
* ROM.
*
* @return  int - negative number on failure.
*
*/
int main( int argc, char *argv[] )
{
	unsigned char i = 0;
	const unsigned char **fmPatchPtr = NULL;
	const unsigned char *fmPatchSizePtr = NULL;
	unsigned char fmPatchCount = 0;
	unsigned char tmp[17] = "";
	int cal_file = -1;
	int chipVersion = 0;
	int fd = -1;
	int rc = 0;
	int size = 0;
	unsigned char gucFmQSocComRegPtr[17] = "";
	tsFmQSocPatchesDefaultType *gFMQSocPatchesXfrDefaults;
	char * fm_i2c_path = NULL;
	int analogMode = 0;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	if( !(argc == 3 || argc == 4) ) // Making analogMode argument optional
	{
		printf("USAGE:  fm_qsoc_patches <ChipVersion> <0, 1, 2 for normal/wa_enable/wa_disable> [true/false for Analogmode]\n");
		return -1;
	}

	if(atoi(argv[2]) == 1 ||  atoi(argv[2]) == 2) {
#ifdef ANDROID
        debug_file = fopen("/data/app/fm_wa_mode", "w+");
#else
	debug_file = fopen("/var/log/fm_wa_mode", "w+");
#endif
		if(debug_file == NULL)
		{
			printf("Failed to open debug file\n");
			return -1;
		}
                fprintf(debug_file, "In Set Wan Band mode \n");
		chipVersion = atoi(argv[1]);
		fprintf(debug_file,"Chip version is: %x\n",chipVersion);

		property_get("ro.qualcomm.bt.hci_transport", transport, NULL);
		fprintf(debug_file,"ro.qualcomm.bt.hci_transport = %s\n", transport);
		if( (3 == strlen(transport)) && ( !strncmp("smd", transport, 3)) ){
			rc = set_desired_notch_filter(fd, atoi(argv[2]));
		}else {
			fm_i2c_path=detect_device();
			if(fm_i2c_path == NULL) {
				fprintf(debug_file, "Failed to get the I2C device path\n");
				return -1;
			}
			fd = open(fm_i2c_path , O_RDWR);

			if (fd < 0) {
				fprintf(debug_file, "Could not open file node\n");
				return fd;
			}
			rc = set_desired_notch_filter(fd, atoi(argv[2]));
		}
		fprintf(debug_file,"set_current_wan_band: %d\n",rc);

		fclose(debug_file);
		close(fd);
		//Always ignore the WAN concurrency failures!?
		return 0;
	}

	if (argc == 4) {
		if (0 == strcmp(argv[3],"true")) {
			analogMode = 1; // Analog Mode is enabled, configure DAC
		}
	}
	/*Set analog/digital modes*/
	if(atoi(argv[2]) == 3) {
#ifdef ANDROID
		debug_file = fopen("/data/app/fm_dacconfig_mode", "w+");
#else
		debug_file = fopen("/var/log/fm_dacconfig_mode", "w+");
#endif
		if(debug_file == NULL)
		{
			printf("Failed to open debug file\n");
			return -1;
		}
		fprintf(debug_file, "In DAC config mode \n");
		chipVersion = atoi(argv[1]);
		fprintf(debug_file,"Chip version is: %x\n",chipVersion);

		fm_i2c_path=detect_device();
		if(fm_i2c_path == NULL) {
			fprintf(debug_file, "Failed to get the I2C device path\n");
			return -1;
		}
		fd = open(fm_i2c_path , O_RDWR);

		if (fd < 0) {
			fprintf(debug_file, "Could not open file node\n");
			return fd;
		}
		/* If hardware supports analog path and is enabled from
		   UI, configure DAC */
		if (isAnalogpathSupported())
		{
			if (analogMode) {
				rc = FmConfigueDAC(fd);
				if (rc >=0){
					fprintf(debug_file, "Configuring DAC : Success\n");
				}
			}
			else {
				/* Reseting FM DAC */
				rc = FmDacDisable(fd);
				if (rc < 0){
					fprintf(debug_file, "Reset FM Dac : Failed\n");
                                        close(fd);
					return -1;
				}
				fprintf(debug_file, "UnConfiguring DAC : Success\n");
			}
		}
		close(fd);
		fclose(debug_file);
		return 0;
	}

	property_get("ro.qualcomm.bt.hci_transport", transport, NULL);
	if( (3 == strlen(transport)) && ( !strncmp("smd", transport, 3)) ){
		cal_file = open("/data/app/Riva_fm_cal", O_RDONLY);
		if (cal_file < 0) {
			/* get_cal_data writes the calibration data into the file*/
			rc = do_riva_calibration();
			return rc;
		}else {
                        close(cal_file);
			return 0;
		}
	}

#ifdef ANDROID
	debug_file = fopen("/data/app/fm_dld_enable", "w+");
#else
	debug_file = fopen("/var/log/fm_dld_enable", "w+");
#endif
	if(debug_file == NULL)
	{
		printf("Failed to open debug file\n");
		return -1;
	}
	fprintf(debug_file, "Starting fm d/l \n");

	/*1st argument is ChipID*/
	chipVersion = atoi(argv[1]);
        fprintf(debug_file,"Chip version is: %x\n",chipVersion);

	fm_i2c_path=detect_device();
	if(fm_i2c_path == NULL) {
                fprintf(debug_file, "Failed to get the I2C device path\n");
		return -1;
	}
        fd = open(fm_i2c_path, O_RDWR);
	fprintf(debug_file, "opened FM i2c device node : %s\n",fm_i2c_path );

	if (fd < 0) {
		fprintf(debug_file, "Could not open file node\n");
		return fd;
	}

	rc = FMQSocPatches_DisableIntrs_TEST(fd, chipVersion);
	if (rc < 0) {
		fprintf(debug_file, "FMQSocPatches_DisableIntrs_TEST failed \n");
		close(fd);
		return rc;
	}

	is_marimba = chip_is_Marimba(chipVersion);

        /* Perform the calibration */
        /* Is the calibration data already available in a file? */
        cal_file = open("/data/app/fm_cal", O_RDONLY);
        if (cal_file < 0) {
                fprintf(debug_file, "cal_file  failed \n");
                /* get_cal_data writes the calibration data into the file*/
                rc = get_cal_data(fd, chipVersion);

                if (rc < 0){
                        fprintf(debug_file, "get_cal_data  failed \n");
                        close(fd);
                        return rc;
                }
                /* Reopen the calibration file */
                cal_file = open("/data/app/fm_cal", O_RDONLY);
        }

        if (cal_file >= 0)
        {
                fprintf(debug_file, "cal_file > 0   \n");
                /* init FM chip */
/*                rc = enableSoC(fd);
                if (rc < 0) {
                        fprintf(debug_file, "enableSoC failed  \n");
                        return rc;
		}
*/
                for (i=0; i<CAL_DAT3_BUF_NUM; i++) {
                        tmp[0] =  (XFRCTL_WRITE_MODE | (CAL_DAT_0 + i));
                        /* Read the calibration data */
                        if (read(cal_file,&tmp[1],XFR_REG_NUM) < XFR_REG_NUM){
                                fprintf(debug_file, " < XFR_REG_NUM %d \n", i);
                                close(cal_file);
                                close(fd);
                                rc = -1;
                                return rc;
                        }

                        /* Write the calibration data onto the device */
                        rc = i2c_write(fd,XFRCTRL,tmp,XFR_REG_NUM+1);
                        if (rc < 0){
                                fprintf(debug_file, " i2c_write failed for Write the calibration data  \n");
                                close(cal_file);
                                close(fd);
                                return rc;
                        }
                        /*
                         * After the writing to the core register wait
                         * for the transfer interrupt
                         */
                        rc  = FMQSocPatches_WaitIsr(fd, FM_TRANSFER_ISR_BITMASK, 10);
                        if (rc < 0) {
                                fprintf(debug_file, "FMQSocPatches_WaitIsr failed for FM_TRANSFER_ISR_BITMASK %d \n", i);
                                close(cal_file);
                                close(fd);
                                return rc;
                        }
                }
                fprintf(debug_file,"write calibration complete\n");
        }
        close(cal_file);

  /* Check if this is FM6500 A0 chip version */
  if (FM6500_A0_VERSION == chipVersion)
  {
    fmPatchPtr      = FMQSocPatchA0Data;
    fmPatchSizePtr  = FMQSocPatchA0Size;
    fmPatchCount    = SIZEOF_FMQSOC_PATCHA0DATA;
  }
  else if (FMQSOCCOM_FM6500_20_VERSION == chipVersion)
  {
    fmPatchPtr      = FMQSocPatchesV20Data;
    fmPatchSizePtr  = FMQSocPatchesV20Size;
    fmPatchCount    = SIZEOF_FMQSOCPATCHES_PATCHV20DATA;
  }
  else if (FMQSOCCOM_FM6500_21_VERSION == chipVersion)
  {
    fmPatchPtr      = FMQSocPatchesV21Data;
    fmPatchSizePtr  = FMQSocPatchesV21Size;
    fmPatchCount    = SIZEOF_FMQSOCPATCHES_PATCHV21DATA;
  }
  else if(FMQSOCCOM_FM6500_WCN2243_10_VERSION == chipVersion)
  {
    fmPatchPtr      = FMQSocPatches_WCN2243_10_Data;
    fmPatchSizePtr  = FMQSocPatches_WCN2243_10_Size;
    fmPatchCount    = SIZEOF_FMQSOCPATCHES_PATCH_WCN2243_10_DATA;
    fprintf(debug_file,"FmQSocPatches_WritePatches: Patches Written for WCN2243 1.0: count: %d \n", fmPatchCount );
  }
  else if(FMQSOCCOM_FM6500_WCN2243_20_VERSION == chipVersion)
  {
    fmPatchPtr      = FMQSocPatches_WCN2243_20_Data;
    fmPatchSizePtr  = FMQSocPatches_WCN2243_20_Size;
    fmPatchCount    = SIZEOF_FMQSOCPATCHES_PATCH_WCN2243_20_DATA;
    fprintf(debug_file,"FmQSocPatches_WritePatches: Patches Written for WCN2243 2.0: count: %d \n", fmPatchCount );
  } else if (FMQSOCCOM_FM6500_WCN2243_21_VERSION == chipVersion) {
    fprintf(debug_file, "FmQSocPatches_WritePatches: Bahama B1\n");
  }
  else
  {
    fprintf(debug_file,"FmQSocPatches_WritePatches: unknown chip version\n");
    /** Even if the chip version is wrong, we allow the initialization to succeed.
    */
    close(fd);
    return 0;
  }

  if (fmPatchCount > 0)	{
     /* Enter Patch Mode */
     rc = FMQSocPatches_EnterEDLMode(fd);
     if (rc < 0) {
        close(fd);
        return rc;
     }
     if(FMQSOCCOM_FM6500_WCN2243_20_VERSION == chipVersion) {
         property_get("mode.factory.testing", factory_testing_mode, "false");
         if(0 == strcmp(factory_testing_mode,"true")) {
             fprintf(debug_file,"In Factory Testing Mode");
             fmPatchCount = DLD_RF_PATCHES;
         } else
             fprintf(debug_file,"Not in Factory Testing Mode");
     }

     /* Download Patches */
     for(i = 0; i < fmPatchCount; i++)
     {
        fprintf(debug_file, "Patch [%d] started downloading\n", i);
        /* Now Transmit bytes over I2C bus */
        rc = FmQSocPatches_EDLCoreRegWr(fd,fmPatchPtr[i],
                                 FM_QSOC_EDL_PATCH_INDEX,
                                 fmPatchSizePtr[i]);
        if (rc < 0) {
           close(fd);
           return rc;
        }
        /*
        * After the writing to the core register wait
        * for the transfer interrupt
        */
        rc  = FMQSocPatches_WaitIsr(fd, FM_TRANSFER_ISR_BITMASK, 30);
        if (rc < 0) {
           fprintf(debug_file, "Patch [%d] download failed\n", i);
           close(fd);
           return rc;
        }
        fprintf(debug_file, "Patch [%d] download success\n", i);
     }

     /* Exit Patch Mode */
     rc = FMQSocPatches_ExitEDLMode(fd);
     if (rc < 0) {
        close(fd);
        return rc;
     }
  }/*fmPatchcount>0*/

  if (!is_marimba && (FMQSOCCOM_FM6500_WCN2243_21_VERSION == chipVersion)) {
     fprintf(debug_file, "Bahama B1 default patch download\n");
     gFMQSocPatchesXfrDefaults = (tsFmQSocPatchesDefaultType *)gFMQSocBahamaB1PatchesXfrDefaults;
     size = sizeof(gFMQSocBahamaB1PatchesXfrDefaults)/sizeof(gFMQSocBahamaB1PatchesXfrDefaults[0]);
  } else if (!is_marimba) {
     gFMQSocPatchesXfrDefaults = (tsFmQSocPatchesDefaultType *)gFMQSocBahamaPatchesXfrDefaults;
     size = sizeof(gFMQSocBahamaPatchesXfrDefaults)/sizeof(gFMQSocBahamaPatchesXfrDefaults[0]);
  } else {
     gFMQSocPatchesXfrDefaults = (tsFmQSocPatchesDefaultType *)gFMQSocMarimbaPatchesXfrDefaults;
     size = sizeof(gFMQSocMarimbaPatchesXfrDefaults)/sizeof(gFMQSocMarimbaPatchesXfrDefaults[0]);
  }

  /* Cycle through all defaults which are different from ROM */
  for (i = 0; i < size; i++)
  {
    fprintf(debug_file, "Defaults [%d] started downloading\n", i);

    gucFmQSocComRegPtr[0] = (gFMQSocPatchesXfrDefaults->ucMode | XFRCTL_WRITE_MODE);
    memcpy(&gucFmQSocComRegPtr[1],
		gFMQSocPatchesXfrDefaults->ucDefaults,
		FMQSOCPATCHES_XFR_SIZE);

    /* Apply new default. */
    rc = FmQSocCom_XfrModeDataWr(fd,
                                 gucFmQSocComRegPtr,
                                 FMQSOCPATCHES_XFR_SIZE);
    if (rc < 0) {
       fprintf(debug_file, "Defaults [%d] download failed\n", i);
       close (fd);
       return rc;
    }
    gFMQSocPatchesXfrDefaults++;
    fprintf(debug_file, "Defaults [%d] download success\n", i);
  }

  if (FMQSOCCOM_FM6500_21_VERSION == chipVersion)
  {
    /* NOTE: Due to a FW limitation, the host must poke the spur value to FW's
       memory. This is expected to be resolved in WCN */

    for (i = 0; i < SIZEOF_FMQSOCPATCHES_SPUR21DATA; i++)
    {
      fprintf(debug_file, "Spur [%d] started downloading\n", i);
      /* Apply the poke to the HW */
      rc = FmQSocCom_Poke(fd, &gFMQSocPatches21Spurs[i]);
      if (rc <0) {

        fprintf(debug_file, "FmQSocPatches_WritePatches: error in poking digital spur [%d]\n", i);
        close(fd);
        return rc;

      }
      fprintf(debug_file, "Spur [%d] download success\n", i);
    }
  }

  if (FMQSOCCOM_FM6500_WCN2243_20_VERSION == chipVersion)
  {
    /* NOTE: Due to a FW limitation, host must poke the spur value to FW's memory. */
    for (i = 0; i < SIZEOF_FMQSOCPATCHES_2243_20_POKEDATA; i++)
    {
      fprintf(debug_file, "\noffset threshold [%d] started downloading\n", i);
      /* Apply the poke to the HW */
      rc = FmQSocCom_Poke(fd, &gFMQSocPatches2243_20_Poke[i]);
      if (rc <0) {

        fprintf(debug_file, "FmQSocPatches_WritePatches: error in poking offset thresholds [%d]\n", i);
        close(fd);
        fclose (debug_file);
        return rc;

      }
      fprintf(debug_file, "poke [%d] download success\n\n", i);
    }
  }

if (FMQSOCCOM_FM6500_WCN2243_21_VERSION != chipVersion) {
  /* Poke the Spur Frequency Rotation Table */
  rc = spurFrequencyFilter(fd);
  if (rc < 0) {
     close(fd);
     fclose (debug_file);
     return rc;
  }
}
  rc = FMQSocPatches_EnableIntrs_TEST(fd, chipVersion);
  if (rc < 0) {
     close(fd);
     return rc;
  }

  close(fd);
  fprintf(debug_file, "fm_qsoc_patches succeeded: %d\n", rc);
  fclose (debug_file);
  return rc;
}

