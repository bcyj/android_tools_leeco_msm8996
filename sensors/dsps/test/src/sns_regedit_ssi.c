/*============================================================================
  @file sns_regedit_ssi.c

  @brief
    Editor for SSI values in the registry

  Copyright (c) 2012-2014 QUALCOMM Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Confidential and Proprietary
  ==========================================================================*/

#include "sensor1.h"
#include "sns_common_v01.h"
#include "sns_reg_common.h"
#include "sns_reg_api_v02.h"
#include "sns_smgr_api_v01.h"

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>

#include <cutils/properties.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define NUM_REG_ITEMS 15

#define OPTSTRING "harws:d:f:"

// All errors should be negative numbers
#define INIT_SUCCESS       0
#define INIT_ERR_FAILED   -1

// Number of registry groups allocated in sns_reg_api_v02.h
// for sensor dependent registry item groups
#define NUM_SENSOR_DEP_REGISTRY_GROUPS 2

// MSM types
typedef enum
{
  SNS_MSM_UNKNOWN,
  SNS_MSM_8974,
  SNS_MSM_8226,
  SNS_APQ_8084,
  SNS_MSM_8994
} sns_msm_id;

//Platform types
typedef enum
{
  SNS_PLATFORM_FLUID,
  SNS_PLATFORM_MTP,
  SNS_PLATFORM_UNKNOWN,
  SNS_PLATFORM_CDP,
  SNS_PLATFORM_LIQUID,
  SNS_PLATFORM_QRD
} sns_platform;

typedef enum {
  READ,
  WRITE,
  AUTODETECT,
  INVALID
} operation_e;

typedef struct data_s {
  sensor1_handle_s *hndl;
  uint8_t           resp_data[2048];
  int               resp_data_size;
  pthread_mutex_t   mutex;
  pthread_cond_t    cond;
} data_t;

typedef struct dev_info_s {
  sns_msm_id    msm_id;
  sns_platform  platform;
  bool          is_dri_mode;
  bool          is_fifo_mode;
}dev_info_t;

typedef struct regitems_s {
  int32_t item_id;
  int32_t item_data;
} regitems_t;

typedef int (*sensor_cfg_init_fptr)( sns_reg_ssi_smgr_cfg_group_s *cfg_array_array,
                                      dev_info_t *dev_info,
                                      regitems_t *regitems );

typedef struct uuid_s {
  char                 name[50];
  uint8_t              byte[16];
  sensor_cfg_init_fptr cfg_init_fptr;
} uuid_t;

typedef struct sensor_dep_reg_item_s {
  uint16_t reg_item_id;
  uint8_t size;
} sensor_dep_reg_item_t;

typedef struct sensor_dep_reg_values_s {
  uint8_t drvuuid[16];
  int32_t* val_ptr;
} sensor_dep_reg_values_t;

typedef struct sensor_dep_reg_table_s {
  char name[10];
  uint8_t num_reg_items;
  sensor_dep_reg_item_t* items;
  uint8_t num_sensors;
  sensor_dep_reg_values_t* values;
} sensor_dep_reg_map_t;

/*
 * init functions:
 *
 * @return  INIT_SUCCESS    : initialization successful with the given SMGR CFG group
 *          INIT_ERR_FAILED : initialization fails
 */
int init_mpu6050_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6500_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6050_8974_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6500_8974_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6050_8974_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6500_8974_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6050_mpu6500_8974_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems, bool b_is_mpu6050 );
int init_mpu6050_mpu6500_8974_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems, bool b_is_mpu6050 );
int init_akm8963_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_bmp085_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_lis3dh_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_lis3dh_gen5_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_lis3dsh_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_lis3dsh_8226_tap_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_lis3dsh_gen5j_8974_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_lis3dsh_gen5j_8974_tap_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_l3gd20_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_lsm303dlhc_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_yas530_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_lps331ap_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_apds99xx_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_accelnull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_gyronull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_magnull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_proxnull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_pressnull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_bma2x2_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_hscdtd008_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_tmd277x_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_ltr55x_8226_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu3050_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_max44009_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6515_gen7_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6515_gen7_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6515_gen7_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6515_gen7_akm8963_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6515_gen7_bmp280_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6515_gen7_stepevt_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6515_gen7_stepcount_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6515_gen7_smd_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_mpu6515_gen7_gamerv_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_shtc1_8xxx_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_apds9950_8xxx_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_bmi058_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_bmi058_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_lps25h_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_tmg399x_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
int init_tmg399x_rgb_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems );
/* The uuid_array holds the names, UUIDs and initialization functions for all sensors known
 * by this tool.
 * Note that the UUIDs are defined in sns_reg_common.h, and are common to the AP and SSC. */
uuid_t uuid_array[] = {
  {"ADXL350", SNS_REG_UUID_ADXL350, NULL },
  {"BMA150", SNS_REG_UUID_BMA150, NULL },
  {"BMA250", SNS_REG_UUID_BMA250, NULL },
  {"LIS3DH", SNS_REG_UUID_LIS3DH, init_lis3dh_8974_config },
  {"ISL29011", SNS_REG_UUID_ISL29011, NULL },
  {"ISL29028", SNS_REG_UUID_ISL29028, NULL },
  {"ISL29147", SNS_REG_UUID_ISL29147, NULL },
  {"BMP085", SNS_REG_UUID_BMP085, init_bmp085_config },
  {"APDS99XX", SNS_REG_UUID_APDS99XX, init_apds99xx_config },
  {"MPU3050", SNS_REG_UUID_MPU3050, init_mpu3050_config },
  {"L3G4200D", SNS_REG_UUID_L3G4200D, NULL },
  {"L3GD20", SNS_REG_UUID_L3GD20, init_l3gd20_config },
  {"AKM8973", SNS_REG_UUID_AKM8973, NULL },
  {"AKM8963", SNS_REG_UUID_AKM8963, init_akm8963_config },
  {"AKM8975", SNS_REG_UUID_AKM8975, NULL },
  {"MEMSIC34160PJ", SNS_REG_UUID_M34160PJ, NULL },
  {"LSM303DLHC", SNS_REG_UUID_LSM303DLHC, init_lsm303dlhc_8974_config },
  {"MPU6050", SNS_REG_UUID_MPU6050, init_mpu6050_8974_config },
  {"MPU6050_ACCEL", SNS_REG_UUID_MPU6050, init_mpu6050_8974_accel_config },
  {"MPU6050_GYRO", SNS_REG_UUID_MPU6050, init_mpu6050_8974_gyro_config },
  {"AMI306", SNS_REG_UUID_AMI306, NULL },
  {"LPS331AP", SNS_REG_UUID_LPS331AP, init_lps331ap_config },
  {"APDS91XX", SNS_REG_UUID_APDS91XX, NULL },
  {"YAS530", SNS_REG_UUID_YAS530, init_yas530_8974_config },
  {"BMC050", SNS_REG_UUID_BMC050, NULL },
  /* LIS3DH has 2 configurations here, since there's the on-8974 part (listed
     above) and the gen5-board part (listed here) */
  {"LIS3DH_GEN5", SNS_REG_UUID_LIS3DH, init_lis3dh_gen5_8974_config },
  {"BMA2X2", SNS_REG_UUID_BMA2X2, init_bma2x2_config },
  {"HSCDTD008", SNS_REG_UUID_HSCD008, init_hscdtd008_config },
  {"TMD277X", SNS_REG_UUID_TMD277X, init_tmd277x_config },
  {"LTR55X", SNS_REG_UUID_LTR55X, init_ltr55x_8226_config },
  {"MAX44009", SNS_REG_UUID_MAX44009, init_max44009_config },
  {"MPU6500", SNS_REG_UUID_MPU6500, init_mpu6500_8974_config },
  {"MPU6500_ACCEL", SNS_REG_UUID_MPU6500, init_mpu6500_8974_accel_config },
  {"MPU6500_GYRO", SNS_REG_UUID_MPU6500, init_mpu6500_8974_gyro_config },
  {"LIS3DSH", SNS_REG_UUID_LIS3DSH, init_lis3dsh_config },
  {"LIS3DSH_TAP", SNS_REG_UUID_LIS3DSH, init_lis3dsh_8226_tap_config },
  {"LIS3DSH_GEN5J",       SNS_REG_UUID_LIS3DSH, NULL },
  {"LIS3DSH_GEN5J_ACCEL", SNS_REG_UUID_LIS3DSH, init_lis3dsh_gen5j_8974_accel_config },
  {"LIS3DSH_GEN5J_TAP",   SNS_REG_UUID_LIS3DSH, init_lis3dsh_gen5j_8974_tap_config },
  {"MPU6515_GEN7",              SNS_REG_UUID_MPU6515, init_mpu6515_gen7_config },
  {"MPU6515_GEN7_ACCEL",        SNS_REG_UUID_MPU6515, init_mpu6515_gen7_accel_config },
  {"MPU6515_GEN7_GYRO",         SNS_REG_UUID_MPU6515, init_mpu6515_gen7_gyro_config },
  {"MPU6515_GEN7_AKM8963",      SNS_REG_UUID_MPU6515_AKM8963, init_mpu6515_gen7_akm8963_config },
  {"MPU6515_GEN7_BMP280",       SNS_REG_UUID_MPU6515_BMP280, init_mpu6515_gen7_bmp280_config },
  {"MPU6515_GEN7_STEP_EVENT",   SNS_REG_UUID_MPU6515, init_mpu6515_gen7_stepevt_config    },
  {"MPU6515_GEN7_STEP_COUNT",   SNS_REG_UUID_MPU6515, init_mpu6515_gen7_stepcount_config  },
  {"MPU6515_GEN7_SMD",          SNS_REG_UUID_MPU6515, init_mpu6515_gen7_smd_config        },
  {"MPU6515_GEN7_GAME_RV",      SNS_REG_UUID_MPU6515, init_mpu6515_gen7_gamerv_config     },
  {"SHTC1",   SNS_REG_UUID_SHTC1, init_shtc1_8xxx_config },
  {"APDS9950", SNS_REG_UUID_APDS9950, init_apds9950_8xxx_config },
  {"BMI058_ACCEL", SNS_REG_UUID_BMA2X2, init_bmi058_accel_config },
  {"BMI058_GYRO", SNS_REG_UUID_BMG160, init_bmi058_gyro_config },
  {"LPS25H", SNS_REG_UUID_LPS25H, init_lps25h_config },
  {"TMG399X", SNS_REG_UUID_TMG399X, init_tmg399x_config },
  {"TMG399X_RGB", SNS_REG_UUID_TMG399X, init_tmg399x_rgb_config },
  {"ADI7146", SNS_REG_UUID_ADI7146, NULL },
  /* Add new sensors here */

  /* The following should be at the end of this array. Add new items above
   * this spot */
  {"NULL", SNS_REG_UUID_NULL, NULL },
  {"ACCEL_NULL", SNS_REG_UUID_NULL, init_accelnull_config },
  {"GYRO_NULL", SNS_REG_UUID_NULL, init_gyronull_config },
  {"MAG_NULL", SNS_REG_UUID_NULL, init_magnull_config },
  {"PROX_NULL", SNS_REG_UUID_NULL, init_proxnull_config },
  {"PRESS_NULL", SNS_REG_UUID_NULL, init_pressnull_config },
};

static uint8_t null_uuid[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static const uint16_t smgr_cfg_id[] = {
  SNS_REG_GROUP_SSI_SMGR_CFG_V02,
  SNS_REG_GROUP_SSI_SMGR_CFG_2_V02,
  SNS_REG_GROUP_SSI_SMGR_CFG_3_V02
};

// Accelerometer dependent registry items
static sensor_dep_reg_item_t accel_dep_reg_items[] = {
  { SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02, 7 },
  { SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V02, 7 },
  { SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02, 7 },
  { SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02, 7 },
  { SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V02, 7 },
  { SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02, 7 },
  { SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V02, 7 }
};

// Default values for ADXL350 accel. All items listed in
// accel_dep_reg_items[] should be initialized here
static int32_t adxl350_reg_item_values[] = {
  SNS_REG_AMD_DEF_INT_CFG_PARAM1_ADXL346,   //Using ADXL346 val
  SNS_REG_AMD_DEF_INT_CFG_PARAM2,
  SNS_REG_VMD_DEF_INT_CFG_PARAM1_ADXL346,   //Using ADXL346 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM1_ADXL346,   //Using ADXL346 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM3_ADXL346,   //Using ADXL346 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM4 };

// Default values for BMA150 accel. All items listed in
// accel_dep_reg_items[] should be initialized here
static int32_t bma150_reg_item_values[] = {
  SNS_REG_AMD_DEF_INT_CFG_PARAM1_BMA150,
  SNS_REG_AMD_DEF_INT_CFG_PARAM2,
  SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150,
  SNS_REG_RMD_DEF_INT_CFG_PARAM1_BMA150,
  SNS_REG_RMD_DEF_INT_CFG_PARAM2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM3_BMA150,
  SNS_REG_RMD_DEF_INT_CFG_PARAM4 };

// Default values for BMA250 accel. All items listed in
// accel_dep_reg_items[] should be initialized here
static int32_t bma250_reg_item_values[] = {
  SNS_REG_AMD_DEF_INT_CFG_PARAM1_BMA250,
  SNS_REG_AMD_DEF_INT_CFG_PARAM2,
  SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150, //Using BMA150 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM1_BMA150,                //Using BMA150 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM3_BMA150,                //Using BMA150 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM4 };

// Default values for BMA2x2 accel. All items listed in
// accel_dep_reg_items[] should be initialized here
static int32_t bma2x2_reg_item_values[] = {
  SNS_REG_AMD_DEF_INT_CFG_PARAM1_BMA2X2,
  SNS_REG_AMD_DEF_INT_CFG_PARAM2_BMA2X2,
  SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150, //Using BMA150 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM1_BMA2X2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM2_BMA2X2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM3_BMA2X2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM4_BMA2X2 };

// Default values for LIS3DH accel. All items listed in
// accel_dep_reg_items[] should be initialized here
static int32_t lis3dh_reg_item_values[] = {
  SNS_REG_AMD_DEF_INT_CFG_PARAM1_LIS3DH,
  SNS_REG_AMD_DEF_INT_CFG_PARAM2,
  SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150,
  SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050,
  SNS_REG_RMD_DEF_INT_CFG_PARAM2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050,
  SNS_REG_RMD_DEF_INT_CFG_PARAM4 };

// Default values for MPU3050 accel. All items listed in
// accel_dep_reg_items[] should be initialized here
static int32_t mpu3050_reg_item_values[] = {
  SNS_REG_AMD_DEF_INT_CFG_PARAM1_MPU6050,               // Using MPU6050 val
  SNS_REG_AMD_DEF_INT_CFG_PARAM2,
  SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150, // Using MPU6050 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050,        // Using MPU6050 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050,        // Using MPU6050 val
  SNS_REG_RMD_DEF_INT_CFG_PARAM4 };

// Default values for MPU6050 accel. All items listed in
// accel_dep_reg_items[] should be initialized here
static int32_t mpu6050_reg_item_values[] = {
  SNS_REG_AMD_DEF_INT_CFG_PARAM1_MPU6050,
  SNS_REG_AMD_DEF_INT_CFG_PARAM2,
  SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150,
  SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050,
  SNS_REG_RMD_DEF_INT_CFG_PARAM2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050,
  SNS_REG_RMD_DEF_INT_CFG_PARAM4 };

// Default values for MPU6500 accel. All items listed in
// accel_dep_reg_items[] should be initialized here
static int32_t mpu6500_reg_item_values[] = {
  SNS_REG_AMD_DEF_INT_CFG_PARAM1_MPU6500,
  SNS_REG_AMD_DEF_INT_CFG_PARAM2,
  SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150,
  SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050,
  SNS_REG_RMD_DEF_INT_CFG_PARAM2,
  SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050,
  SNS_REG_RMD_DEF_INT_CFG_PARAM4 };

// Table mapping UUIDs and default registry item values
// FIXME: Could not find UUID for ADXL346. Using ADXL350 instead
static sensor_dep_reg_values_t accel_dep_reg_item_values[] = {
  { SNS_REG_UUID_ADXL350, adxl350_reg_item_values },
  { SNS_REG_UUID_BMA150,  bma150_reg_item_values  },
  { SNS_REG_UUID_BMA250,  bma250_reg_item_values  },
  { SNS_REG_UUID_BMA2X2,  bma2x2_reg_item_values  },
  { SNS_REG_UUID_LIS3DH,  lis3dh_reg_item_values  },
  { SNS_REG_UUID_MPU3050, mpu3050_reg_item_values },
  { SNS_REG_UUID_MPU6050, mpu6050_reg_item_values },
  { SNS_REG_UUID_MPU6500, mpu6500_reg_item_values }
};

// Table mapping sensor types, dependent registy items and values
static sensor_dep_reg_map_t sensor_dep_reg_item_table[] = {
  { "ACCEL", 7, accel_dep_reg_items, 8, accel_dep_reg_item_values},
  { "GYRO",  0, NULL,                0, NULL,                    },
  { "MAG",   0, NULL,                0, NULL,                    },
  { "LIGHT", 0, NULL,                0, NULL,                    },
  { "PRESS", 0, NULL,                0, NULL,                    }
};

static uint16_t sensor_dep_group_ids[NUM_SENSOR_DEP_REGISTRY_GROUPS] = {
  SNS_REG_GROUP_SSI_SENSOR_DEP_CFG0_V02,
  SNS_REG_GROUP_SSI_SENSOR_DEP_CFG1_V02 };

static uint16_t get_sensor_dep_reg_group_id( uint8_t index )
{
  if( index >= NUM_SENSOR_DEP_REGISTRY_GROUPS ) {
    return 0;
  }
  return sensor_dep_group_ids[index];
}

static void add_to_regitems( regitems_t *regitems,
                             int32_t item_id,
                             int64_t item_data )
{
  int i;
  if( regitems != NULL ) {
    for( i = 0; i < NUM_REG_ITEMS; i++ ) {
      if( regitems[i].item_id == -1 || regitems[i].item_id == item_id ) {
        regitems[i].item_id = item_id;
        regitems[i].item_data = (int32_t)item_data;
        break;
      }
    }
  }
}

static int uuid_to_index( const uint8_t byte[16] )
{
  int i;
  for( i = 0; i < (int)ARRAY_SIZE(uuid_array); i ++ ) {
    if( memcmp( byte, uuid_array[i].byte, sizeof(uuid_array[i].byte) ) == 0 ) {
      return i;
    }
  }
  return -1;
}

static void print_uuid( const uint8_t *u )
{
  int index;
  index = uuid_to_index( u );
  if( index != -1 ) {
    printf("%s %2.2x%2.2x%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x"
           "-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x\n",
           uuid_array[index].name,
           u[0],u[1],u[2],u[3],u[4],u[5],u[6],u[7],
           u[8],u[9],u[10],u[11],u[12],u[13],u[14],u[15] );
  } else {
    printf("(UNKNOWN DRV) %2.2x%2.2x%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x"
           "-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x\n",
           u[0],u[1],u[2],u[3],u[4],u[5],u[6],u[7],
           u[8],u[9],u[10],u[11],u[12],u[13],u[14],u[15] );
  }
}

static void print_drv_cfg( const sns_reg_ssi_smgr_cfg_group_s *cfg, int i, uint8_t cfg_idx )
{
  int dev_sel_id;
  int base_id = 0;

  dev_sel_id = (cfg_idx==0) ? SNS_REG_ITEM_SSI_SMGR_CFG0_DEVICE_SELECT_V02:
                              ((cfg_idx==1) ? SNS_REG_ITEM_SSI_SMGR_CFG0_2_DEVICE_SELECT_V02:
                              SNS_REG_ITEM_SSI_SMGR_CFG0_3_DEVICE_SELECT_V02);

  switch( i ) {
    case 0:
      base_id = (cfg_idx==0) ? SNS_REG_ITEM_SSI_SMGR_CFG0_UUID_HIGH_V02:
                               ((cfg_idx==1) ? SNS_REG_ITEM_SSI_SMGR_CFG0_2_UUID_HIGH_V02:
                               SNS_REG_ITEM_SSI_SMGR_CFG0_3_UUID_HIGH_V02);
      break;
    case 1:
      base_id = (cfg_idx==0) ? SNS_REG_ITEM_SSI_SMGR_CFG1_UUID_HIGH_V02:
                               ((cfg_idx==1) ? SNS_REG_ITEM_SSI_SMGR_CFG1_2_UUID_HIGH_V02:
                               SNS_REG_ITEM_SSI_SMGR_CFG1_3_UUID_HIGH_V02);
      break;
    case 2:
      base_id = (cfg_idx==0) ? SNS_REG_ITEM_SSI_SMGR_CFG2_UUID_HIGH_V02:
                               ((cfg_idx==1) ? SNS_REG_ITEM_SSI_SMGR_CFG2_2_UUID_HIGH_V02:
                               SNS_REG_ITEM_SSI_SMGR_CFG2_3_UUID_HIGH_V02);
      break;
    case 3:
      base_id = (cfg_idx==0) ? SNS_REG_ITEM_SSI_SMGR_CFG3_UUID_HIGH_V02:
                               ((cfg_idx==1) ? SNS_REG_ITEM_SSI_SMGR_CFG3_2_UUID_HIGH_V02:
                               SNS_REG_ITEM_SSI_SMGR_CFG3_3_UUID_HIGH_V02);
      break;
    case 4:
      base_id = (cfg_idx==0) ? SNS_REG_ITEM_SSI_SMGR_CFG4_UUID_HIGH_V02:
                               ((cfg_idx==1) ? SNS_REG_ITEM_SSI_SMGR_CFG4_2_UUID_HIGH_V02:
                               SNS_REG_ITEM_SSI_SMGR_CFG4_3_UUID_HIGH_V02);
      break;
    default:
      return;
  }
  printf("item-registry-ID: name: value...\n");
  printf("%d-%d: UUID: ", base_id, base_id+1);
  base_id += 2;
  print_uuid( cfg->drv_cfg[i].drvuuid );
  printf("%d: off_to_idle:   %6d\t",base_id++,cfg->drv_cfg[i].off_to_idle);
  printf("%d: idle_to_ready: %6d\t",base_id++,cfg->drv_cfg[i].idle_to_ready);
  printf("%d: i2c_bus:         0x%02x\n",base_id++,cfg->drv_cfg[i].i2c_bus);
  printf("%d: reg_group_id:  %6d\t",base_id++,cfg->drv_cfg[i].reg_group_id);
  printf("%d: cal_grp_id:    %6d\t",base_id++,cfg->drv_cfg[i].cal_pri_group_id);
  printf("%d: gpio1:         0x%04x\n",base_id++,cfg->drv_cfg[i].gpio1);
  printf("%d: gpio2:         0x%04x\t",base_id++,cfg->drv_cfg[i].gpio2);
  printf("%d: sensor_id:     %6d\t",base_id++,cfg->drv_cfg[i].sensor_id);
  printf("%d: i2c_address:     0x%02x\n",base_id++,cfg->drv_cfg[i].i2c_address);
  printf("%d: data_type1:    %6d\t",base_id++,cfg->drv_cfg[i].data_type1);
  printf("%d: data_type2:    %6d\t",base_id++,cfg->drv_cfg[i].data_type2);
  printf("%d: rel_sns_idx:   %6d\n",base_id++,cfg->drv_cfg[i].related_sensor_index);
  printf("%d: sens_default:  %6d\t",base_id++,cfg->drv_cfg[i].sensitivity_default);
  printf("%d: flags:           0x%02x\n",base_id++,cfg->drv_cfg[i].flags);
  printf("%d: device_select: %6d\n",dev_sel_id+i,cfg->drv_cfg[i].device_select);
}

static int init_default_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, regitems_t *regitems, dev_info_t *dev_info )
{
  int cfg_idx;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  /* Initialize sensors for Qualcomm MTP/Fluid platforms in polling (non-DRI) mode */
  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];
    memset( cfg, 0, sizeof(cfg_array[0]) );
    cfg->maj_ver_no = 1;
    cfg->min_ver_no = 1;
  }

  /* Align with the Item IDs in sensor_def_qcomdev.conf. Configure more sensors when needed */
  cfg_array[0].drv_cfg[0].sensor_id = SNS_SMGR_ID_ACCEL_V01;
  cfg_array[0].drv_cfg[1].sensor_id = SNS_SMGR_ID_GYRO_V01;
  cfg_array[0].drv_cfg[2].sensor_id = SNS_SMGR_ID_MAG_V01;
  cfg_array[0].drv_cfg[3].sensor_id = SNS_SMGR_ID_PRESSURE_V01;
  cfg_array[0].drv_cfg[4].sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;

  cfg_array[1].drv_cfg[0].sensor_id = SNS_SMGR_ID_HUMIDITY_V01;
  cfg_array[1].drv_cfg[1].sensor_id = SNS_SMGR_ID_RGB_V01;

  switch(dev_info->msm_id) {
    case SNS_MSM_8974:
    {
      init_mpu6050_8974_config( cfg_array, dev_info, regitems );
      init_akm8963_config( cfg_array, dev_info, regitems );
      init_bmp085_config( cfg_array, dev_info, regitems );
      init_apds99xx_config( cfg_array, dev_info, regitems );
    }
    break;
    case SNS_MSM_8226:
    {
      init_bma2x2_config( cfg_array, dev_info, regitems );
      init_mpu3050_config( cfg_array, dev_info, regitems );
      init_hscdtd008_config( cfg_array, dev_info, regitems );
      init_pressnull_config( cfg_array, dev_info, regitems );
      init_tmd277x_config( cfg_array, dev_info, regitems );
    }
    break;
    case SNS_APQ_8084:
    {
      init_lis3dsh_config( cfg_array, dev_info, regitems );
      init_l3gd20_config( cfg_array, dev_info, regitems );
      init_akm8963_config( cfg_array, dev_info, regitems );
      init_bmp085_config( cfg_array, dev_info, regitems );
      init_apds9950_8xxx_config( cfg_array, dev_info, regitems );
    }
    break;
    case SNS_MSM_8994:
    {
      /*Sensors in DRI and FIFO mode by default */
      dev_info->is_dri_mode = true;
      dev_info->is_fifo_mode = true;
      init_bmi058_accel_config( cfg_array, dev_info, regitems );
      init_bmi058_gyro_config( cfg_array, dev_info, regitems );
      /*Sensors in Polling mode by default */
      dev_info->is_dri_mode = false;
      dev_info->is_fifo_mode = false;
      init_hscdtd008_config( cfg_array, dev_info, regitems );
      init_lps25h_config( cfg_array, dev_info, regitems );
      /*Sensors in DRI mode by default */
      dev_info->is_dri_mode = true;
      dev_info->is_fifo_mode = false;
      init_tmg399x_config( cfg_array, dev_info, regitems );
      init_tmg399x_rgb_config( cfg_array, dev_info, regitems );
      init_shtc1_8xxx_config( cfg_array, dev_info, regitems );
    }
    break;
    default:
    {
      printf("MSM ID not detected or MSM ID is invalid/unknown\n");
    }
  }

  return INIT_SUCCESS;
}

static void notify_data_cb( intptr_t data,
                            sensor1_msg_header_s *msg_hdr_ptr,
                            sensor1_msg_type_e msg_type,
                            void *msg_ptr )
{
  data_t *data_info = (data_t*)data;
  if( msg_type == SENSOR1_MSG_TYPE_RESP ) {
    pthread_mutex_lock( &data_info->mutex );
    memcpy(data_info->resp_data, msg_ptr, (size_t)MIN(2048, msg_hdr_ptr->msg_size));
    data_info->resp_data_size = msg_hdr_ptr->msg_size;
    pthread_cond_signal( &data_info->cond);
    pthread_mutex_unlock( &data_info->mutex );
  } else {
    printf("rx invalid msg type %i\n", msg_type);
  }
  if( NULL != msg_ptr ) {
    sensor1_free_msg_buf( data_info->hndl, msg_ptr );
  }
}


static sensor1_error_e read_reg_data( data_t *data_info,
                                      sns_reg_ssi_smgr_cfg_group_s *cfg,
                                      uint8_t cfg_idx,
                                      regitems_t *regitems)
{
  sensor1_msg_header_s msg_hdr;
  sns_reg_group_read_req_msg_v02 *msg;
  sns_reg_group_read_resp_msg_v02 *resp_msg;
  sensor1_error_e err;

  msg_hdr.service_number = SNS_REG2_SVC_ID_V01;
  msg_hdr.msg_id = SNS_REG_GROUP_READ_REQ_V02;
  msg_hdr.msg_size = sizeof(sns_reg_group_read_req_msg_v02);
  msg_hdr.txn_id = 1;

  err = sensor1_alloc_msg_buf( data_info->hndl, msg_hdr.msg_size, (void**)&msg );
  if( err != SENSOR1_SUCCESS ) {
    return err;
  }
  msg->group_id = smgr_cfg_id[cfg_idx];

  pthread_mutex_lock( &data_info->mutex );
  data_info->resp_data_size = -1;
  err = sensor1_write( data_info->hndl,
                       &msg_hdr,
                       msg );
  if( err == SENSOR1_SUCCESS ) {
    while( data_info->resp_data_size == -1 ) {
      pthread_cond_wait( &data_info->cond, &data_info->mutex );
    }
    resp_msg = (sns_reg_group_read_resp_msg_v02*)data_info->resp_data;
    if( resp_msg->resp.sns_result_t == 0 && resp_msg->resp.sns_err_t == SENSOR1_SUCCESS &&
        resp_msg->data_len == sizeof(sns_reg_ssi_smgr_cfg_group_s) ) {
      printf("SSI Group registry read successful\n");
      memcpy( cfg, resp_msg->data, MIN(sizeof(sns_reg_ssi_smgr_cfg_group_s),resp_msg->data_len) );
      err = SENSOR1_SUCCESS;
    } else {
      printf("Registry read FAILED %x %x, size %x\n",
             resp_msg->resp.sns_result_t, resp_msg->resp.sns_err_t,
             resp_msg->data_len );
      err = SENSOR1_EFAILED;
    }
  } else {
    printf( "Registry read failed. sensor1_write request failed %d\n",
            err );
  }
  /* TODO : Read SAM items */
  pthread_mutex_unlock( &data_info->mutex );
  return err;
}

static sensor1_error_e write_reg_data( data_t *data_info,
                                       const sns_reg_ssi_smgr_cfg_group_s *cfg,
                                       uint8_t cfg_idx,
                                       regitems_t *regitems )
{
  sensor1_msg_header_s msg_hdr;
  sns_reg_group_write_req_msg_v02 *msg;
  sns_reg_group_write_resp_msg_v02 *resp_msg = NULL;
  int i;
  sns_reg_single_write_req_msg_v02 *single_msg;
  sns_reg_single_write_resp_msg_v02 *single_resp_msg;
  sensor1_error_e err;

  msg_hdr.service_number = SNS_REG2_SVC_ID_V01;
  msg_hdr.msg_id = SNS_REG_GROUP_WRITE_REQ_V02;
  msg_hdr.msg_size = sizeof(sns_reg_group_write_req_msg_v02);
  msg_hdr.txn_id = 3;

  err = sensor1_alloc_msg_buf( data_info->hndl, msg_hdr.msg_size, (void**)&msg );
  if( err != SENSOR1_SUCCESS ) {
    return err;
  }
  msg->group_id = smgr_cfg_id[cfg_idx];

  msg->data_len = sizeof(sns_reg_ssi_smgr_cfg_group_s);
  memcpy(msg->data, cfg, msg->data_len );

  pthread_mutex_lock( &data_info->mutex );
  data_info->resp_data_size = -1;

  err = sensor1_write( data_info->hndl,
                       &msg_hdr,
                       msg );
  if( err != SENSOR1_SUCCESS ) {
    printf("SSI Group registry write failed. sensor1_write error %d\n",
           err);
  } else {
    while( data_info->resp_data_size == -1 ) {
      pthread_cond_wait( &data_info->cond, &data_info->mutex );
    }
    resp_msg = (sns_reg_group_write_resp_msg_v02*)data_info->resp_data;
    if( resp_msg->resp.sns_result_t == 0 && resp_msg->resp.sns_err_t == SENSOR1_SUCCESS ) {
      printf("SSI Group registry write successful\n");
    } else {
      printf("SSI Group registry write FAILED %x %x\n",
             resp_msg->resp.sns_result_t, resp_msg->resp.sns_err_t );
    }
  }


  i = 0;
  msg_hdr.msg_id = SNS_REG_SINGLE_WRITE_REQ_V02;
  msg_hdr.msg_size = sizeof(sns_reg_single_write_req_msg_v02);
  msg_hdr.txn_id = 2;
  while( regitems[i].item_id != -1 ) {
    err =sensor1_alloc_msg_buf( data_info->hndl, msg_hdr.msg_size, (void**)&single_msg );
    if( err != SENSOR1_SUCCESS ) {
      printf("Error allocating memory\n");
      break;
    }
    single_msg->item_id = (int16_t)regitems[i].item_id;
    /* All SAM items are Q16s. So assume 4-bytes here */
    single_msg->data_len = 4;
    memcpy( single_msg->data, &(regitems[i].item_data), single_msg->data_len );
    data_info->resp_data_size = -1;
    err = sensor1_write( data_info->hndl,
                         &msg_hdr,
                         single_msg );
    if( err != SENSOR1_SUCCESS ) {
      printf("Registry item %d write FAILED. sensor1_write error %d\n",
             regitems[i].item_id, err );
      break;
    }
    while( data_info->resp_data_size == -1 ) {
      pthread_cond_wait( &data_info->cond, &data_info->mutex );
    }
    single_resp_msg = (sns_reg_single_write_resp_msg_v02*)data_info->resp_data;
    if( single_resp_msg->resp.sns_result_t == 0 &&
        resp_msg != NULL && resp_msg->resp.sns_err_t == SENSOR1_SUCCESS ) {
      printf("Registry item %d write successful (val 0x%x)\n",
             regitems[i].item_id,
             regitems[i].item_data);
    } else {
      printf("Registry item %d write FAILED %x %x\n",
             regitems[i].item_id,
             single_resp_msg->resp.sns_result_t, single_resp_msg->resp.sns_err_t );
    }
    i++;
  }
  pthread_mutex_unlock( &data_info->mutex );
  return err;
}

static sensor1_error_e write_devinfo_data( data_t *data_info,
                                           uint16_t devinfo_group_id,
                                           const sns_reg_ssi_devinfo_group_s *cfg )

{
  sensor1_msg_header_s msg_hdr;
  sns_reg_group_write_req_msg_v02 *msg;
  sns_reg_group_write_resp_msg_v02 *resp_msg;
  sensor1_error_e err;

  msg_hdr.service_number = SNS_REG2_SVC_ID_V01;
  msg_hdr.msg_id = SNS_REG_GROUP_WRITE_REQ_V02;
  msg_hdr.msg_size = sizeof(sns_reg_group_write_req_msg_v02);
  msg_hdr.txn_id = (uint8_t)(devinfo_group_id);

  err = sensor1_alloc_msg_buf( data_info->hndl, msg_hdr.msg_size, (void**)&msg );
  if( err != SENSOR1_SUCCESS ) {
    return err;
  }
  msg->group_id = devinfo_group_id;
  msg->data_len = sizeof(sns_reg_ssi_devinfo_group_s);
  memcpy(msg->data, cfg, msg->data_len );

  pthread_mutex_lock( &data_info->mutex );
  data_info->resp_data_size = -1;

  err = sensor1_write( data_info->hndl,
                       &msg_hdr,
                       msg );
  if( err == SENSOR1_SUCCESS ) {
    while( data_info->resp_data_size == -1 ) {
      pthread_cond_wait( &data_info->cond, &data_info->mutex );
    }
    resp_msg = (sns_reg_group_write_resp_msg_v02*)data_info->resp_data;
    if( resp_msg->resp.sns_result_t == SNS_RESULT_SUCCESS_V01 &&
        resp_msg->resp.sns_err_t == SENSOR1_SUCCESS ) {
      printf("SSI Devinfo Group %d registry write successful\n", devinfo_group_id );
    } else {
      printf("SSI Devinfo Group %d registry write FAILED %x %x\n",
             devinfo_group_id, resp_msg->resp.sns_result_t, resp_msg->resp.sns_err_t );
    }
  } else {
    printf("SSI Devinfo Group %d registry write FAILED. sensor1_write error %d\n",
           devinfo_group_id, err );
  }
  pthread_mutex_unlock( &data_info->mutex );
  return err;
}

void print_smgr_cfg_group( const sns_reg_ssi_smgr_cfg_group_s *cfg, uint8_t cfg_idx, regitems_t *regitems )
{
  int i;
  printf("\n==========   SMGR Config Group %d   ====================\n", cfg_idx);
  printf("Version: %x.%x\n", cfg->maj_ver_no, cfg->min_ver_no);
  for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
    printf("drv_cfg[%x]\n-----------------------------\n",i);
    print_drv_cfg(cfg, i, cfg_idx);
  }
  i = 0;
  printf("SAM Registry data:\n");
  while( regitems[i].item_id != -1 ) {
    printf("%s(%i): 0x%x\n",
           regitems[i].item_id==SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02?"SNS_REG_ITEM_AMD_INT_CFG_PARAM1":
           regitems[i].item_id==SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02?"SNS_REG_ITEM_VMD_INT_CFG_PARAM1":
           regitems[i].item_id==SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02?"SNS_REG_ITEM_RMD_INT_CFG_PARAM1":
           regitems[i].item_id==SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02?"SNS_REG_ITEM_RMD_INT_CFG_PARAM3":
           "Unknown item",
           regitems[i].item_id,
           regitems[i].item_data );
    i++;
  }
}

void print_devinfo_group( const sns_reg_ssi_devinfo_group_s *devinfo )
{
  int i;

  printf("min_ver_no: %d\t", devinfo->min_ver_no );
  printf("num_uuid_dev_info_valid: %d\n", devinfo->num_uuid_dev_info_valid );
  for( i = 0; i < SNS_REG_SSI_DEVINFO_NUM_CFGS && i < devinfo->num_uuid_dev_info_valid; i++ ) {
    printf("UUID: ");
    print_uuid( devinfo->uuid_cfg[i].drvuuid );
    printf("off_to_idle: %d\tidle_to_ready: %d\tgpio1:%d\n",
           devinfo->uuid_cfg[i].off_to_idle,
           devinfo->uuid_cfg[i].idle_to_ready,
           devinfo->uuid_cfg[i].gpio1 );
    printf("reg_group_id: %d\tcal_pri_group_id: %d\ti2c_bus: %d\n",
           devinfo->uuid_cfg[i].reg_group_id,
           devinfo->uuid_cfg[i].cal_pri_group_id,
           devinfo->uuid_cfg[i].i2c_bus );
    printf("i2c_address: 0x%x\tsensitivity_default: %d\tflags: 0x%x\n",
           devinfo->uuid_cfg[i].i2c_address,
           devinfo->uuid_cfg[i].sensitivity_default,
           devinfo->uuid_cfg[i].flags );
  }
}

void print_sensor_dep_group( const sns_reg_ssi_sensor_dep_reg_group_s *sensor_dep_group )
{
  int i, j;

  printf("Version no: %d\t", sensor_dep_group->ver_no );
  printf("Sensor type: %d\t", sensor_dep_group->sensor_type );
  printf("Next group id: %d\n", sensor_dep_group->next_group_id );
  for( i = 0; i < SNS_REG_MAX_SENSORS_WITH_DEP_REG_ITEMS; ++i ) {
    printf("UUID: ");
    print_uuid( sensor_dep_group->uuid_reg_cfg[i].drvuuid );
    for( j = 0; j < SNS_REG_MAX_SENSOR_DEP_REG_ITEMS; ++j ) {
      printf("reg item id: %d\tsize: %d\tvalue:%2.2x%2.2x%2.2x%2.2x\n",
             sensor_dep_group->uuid_reg_cfg[i].reg_items[j].reg_item_id,
             sensor_dep_group->uuid_reg_cfg[i].reg_items[j].size,
             sensor_dep_group->uuid_reg_cfg[i].reg_items[j].value[0],
             sensor_dep_group->uuid_reg_cfg[i].reg_items[j].value[1],
             sensor_dep_group->uuid_reg_cfg[i].reg_items[j].value[2],
             sensor_dep_group->uuid_reg_cfg[i].reg_items[j].value[3] );
    }
  }
}

void print_usage( char * const filename )
{
  int i;
  fprintf(stderr, "Usage: %s [-r|-w|-a] [-s|-d <SENSOR>]\n"
          "\t-r: Read registry SSI data\n"
          "\t-w: Write SSI data.\n"
          "\t\t--Manually select sensor configuration using -s or -d to choose sensors\n"
          "\t\t--If no sensors are selected, will configure the default MSM8974, MSM8226, APQ8084 and MSM8994 sensors\n"
          "\t\t--The MSM ID is detected by the program and need not be passed by the user.\n"
          "\t-a: Write SSI Autodetect data.\n"
          "\t\t--Specify sensors to autodetect with -s or -d, in order of preference\n"
          "\t\t--If no sensors are selected, will autodetect only the default MSM8974, MSM8226 and APQ8084 sensors\n"
          "\n"
          "\t-s <SENSOR>: Configure to use/autodetect a certain sensor part in polling mode\n"
          "\t-d <SENSOR>: Configure to use/autodetect a certain sensor part in DRI mode\n",
          filename );
  fprintf(stderr, "The following parts are supported:\n");
  for( i=0; i < (int)ARRAY_SIZE(uuid_array); i++ ) {
    if( uuid_array[i].cfg_init_fptr != NULL ) {
      fprintf(stderr, "\t%s\n", uuid_array[i].name);
    }
  }
}

void setup_related_sensor( sns_reg_ssi_smgr_cfg_group_s *cfg_array )
{
  int i, j, cfg_idx;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG; cfg_idx++) {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      cfg->drv_cfg[i].related_sensor_index = -1;
    }

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS-1; i++ ) {
      for( j = i+1; j < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; j++ ) {
        if( memcmp(cfg->drv_cfg[i].drvuuid,
                   cfg->drv_cfg[j].drvuuid,
                   sizeof(cfg->drv_cfg[i].drvuuid)) == 0 )
        {
          cfg->drv_cfg[i].related_sensor_index = (int8_t)j;
          cfg->drv_cfg[j].related_sensor_index = (int8_t)i;
        }
      }
    }
  }
}

int init_mpu6050_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int err = INIT_SUCCESS;
  err += init_mpu6050_mpu6500_8974_accel_config( cfg_array, dev_info, regitems, true );
  err += init_mpu6050_mpu6500_8974_gyro_config( cfg_array, dev_info, regitems, true );

  return (err==INIT_SUCCESS) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6500_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int err = INIT_SUCCESS;
  err += init_mpu6050_mpu6500_8974_accel_config( cfg_array, dev_info, regitems, false );
  err += init_mpu6050_mpu6500_8974_gyro_config( cfg_array, dev_info, regitems, false );

  return (err==INIT_SUCCESS) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6050_8974_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int err = INIT_SUCCESS;
  err += init_mpu6050_mpu6500_8974_accel_config( cfg_array, dev_info, regitems, true );

  return (err==INIT_SUCCESS) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6500_8974_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int err = INIT_SUCCESS;
  err += init_mpu6050_mpu6500_8974_accel_config( cfg_array, dev_info, regitems, false );

  return (err==INIT_SUCCESS) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6050_8974_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int err = INIT_SUCCESS;
  err += init_mpu6050_mpu6500_8974_gyro_config( cfg_array, dev_info, regitems, true );

  return (err==INIT_SUCCESS) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6500_8974_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int err = INIT_SUCCESS;
  err += init_mpu6050_mpu6500_8974_gyro_config( cfg_array, dev_info, regitems, false );

  return (err==INIT_SUCCESS) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6515_gen7_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int err = INIT_SUCCESS;
  err += init_mpu6515_gen7_accel_config( cfg_array, dev_info, regitems );
  err += init_mpu6515_gen7_gyro_config( cfg_array, dev_info, regitems );
  err += init_mpu6515_gen7_akm8963_config( cfg_array, dev_info, regitems );
  err += init_mpu6515_gen7_bmp280_config( cfg_array, dev_info, regitems );
  err += init_mpu6515_gen7_stepevt_config( cfg_array, dev_info, regitems );
  err += init_mpu6515_gen7_stepcount_config( cfg_array, dev_info, regitems );
  err += init_mpu6515_gen7_smd_config( cfg_array, dev_info, regitems );
  err += init_mpu6515_gen7_gamerv_config( cfg_array, dev_info, regitems );

  return (err==INIT_SUCCESS) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6050_mpu6500_8974_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems, bool b_is_mpu6050 )
{
  int i, cfg_idx;
  uint8_t uuid_mpu6050[16] = SNS_REG_UUID_MPU6050;
  uint8_t uuid_mpu6500[16] = SNS_REG_UUID_MPU6500;
  uint8_t uuid[16];
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  if( b_is_mpu6050 )
  {
    memcpy(uuid,uuid_mpu6050,sizeof(uuid));
  }
  else
  {
    memcpy(uuid,uuid_mpu6500,sizeof(uuid));
  }

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      /* the mpu6050 accel is an accel. So overwrite an accel part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_ACCEL_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 12;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio1 = 66;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_ACCEL_V01;
      if( SNS_MSM_8974 == dev_info->msm_id ) {
        cfg->drv_cfg[i].i2c_address = ( true == b_is_mpu6050 )?0x68:0x69;
      } else {
        cfg->drv_cfg[i].i2c_address = 0x68;
      }
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_ACCEL;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].device_select = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg_done = true;

      if( b_is_mpu6050 )
      {
         add_to_regitems( regitems,
                         SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02,
                         SNS_REG_AMD_DEF_INT_CFG_PARAM1_MPU6050 );
      }
      else
      {
        add_to_regitems( regitems,
                         SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02,
                         SNS_REG_AMD_DEF_INT_CFG_PARAM1_MPU6500 );
      }

      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02,
                       SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM4 );
    }
  }

  return (cfg_done ? INIT_SUCCESS : INIT_ERR_FAILED);
}

int init_mpu6050_mpu6500_8974_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems, bool b_is_mpu6050 )
{
  int i, cfg_idx;
  uint8_t uuid_mpu6050[16] = SNS_REG_UUID_MPU6050;
  uint8_t uuid_mpu6500[16] = SNS_REG_UUID_MPU6500;
  uint8_t uuid[16];
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  if( b_is_mpu6050 )
  {
    memcpy(uuid,uuid_mpu6050,sizeof(uuid));
  }
  else
  {
    memcpy(uuid,uuid_mpu6500,sizeof(uuid));
  }

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      /* the MPU6050 gyro is a gyro. So overwrite a gyro part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_GYRO_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 12;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_GYRO_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio1 = 66;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_GYRO_V01;
      if( SNS_MSM_8974 == dev_info->msm_id ) {
        cfg->drv_cfg[i].i2c_address = ( true == b_is_mpu6050 )?0x68:0x69;
      } else {
        cfg->drv_cfg[i].i2c_address = 0x68;
      }
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_GYRO;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_TEMP;
      cfg->drv_cfg[i].sensitivity_default = 3;
      cfg->drv_cfg[i].device_select = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg_done = true;
    }
  }
  return (cfg_done ? INIT_SUCCESS : INIT_ERR_FAILED);
}

int init_mpu3050_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_MPU3050;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the MPU3050 gyro is a gyro. So overwrite a gyro part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_GYRO_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 150000;
      cfg->drv_cfg[i].idle_to_ready = 100000;
      if( dev_info->msm_id == SNS_MSM_8226 ) {
         cfg->drv_cfg[i].i2c_bus = 2;
      } else if( dev_info->msm_id == SNS_APQ_8084 ) {
         cfg->drv_cfg[i].i2c_bus = 8;
      }
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_GYRO_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V02;
      if( dev_info->msm_id == SNS_MSM_8226 ) {
         cfg->drv_cfg[i].gpio1 = 64;
      } else if( dev_info->msm_id == SNS_APQ_8084 ) {
         cfg->drv_cfg[i].gpio1 = 60;
      }
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_GYRO_V01;
      cfg->drv_cfg[i].i2c_address = 0x68;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_GYRO;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_TEMP;
      cfg->drv_cfg[i].sensitivity_default = 3;
      cfg->drv_cfg[i].device_select = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_akm8963_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_AKM8963;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the AKM8963 is a mag. So overwrite a mag part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_MAG_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 0;
      cfg->drv_cfg[i].idle_to_ready = 0;
      if( dev_info->msm_id == SNS_MSM_8226 ) {
        cfg->drv_cfg[i].i2c_bus = 2;
        cfg->drv_cfg[i].i2c_address = 0x0F;
      } else if( dev_info->msm_id == SNS_MSM_8974 ) {
        cfg->drv_cfg[i].i2c_bus = 3;
        cfg->drv_cfg[i].i2c_address = 0x0C;
      } else if( dev_info->msm_id == SNS_APQ_8084 ) {
        cfg->drv_cfg[i].i2c_bus = 12;
        cfg->drv_cfg[i].i2c_address = 0x0C;
      }
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_MAG_V02;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      cfg->drv_cfg[i].gpio1 = 0xFFFF;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_MAG_V01;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_MAG;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 0;
      cfg->drv_cfg[i].flags = 0;
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_bmp085_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_BMP085;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the BMP085 is a presure sensor. So overwrite a pressure part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PRESSURE_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 10000;
      cfg->drv_cfg[i].idle_to_ready = 0;
      if( dev_info->msm_id == SNS_MSM_8226 ) {
        cfg->drv_cfg[i].i2c_bus = 2;
      } else if( dev_info->msm_id == SNS_MSM_8974 ) {
        cfg->drv_cfg[i].i2c_bus = 3;
      } else if( dev_info->msm_id == SNS_APQ_8084 ) {
        cfg->drv_cfg[i].i2c_bus = 12;
      }
      cfg->drv_cfg[i].reg_group_id = 0xFFFF;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      cfg->drv_cfg[i].gpio1 = 0xFFFF;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PRESSURE_V01;
      cfg->drv_cfg[i].i2c_address = 0xEE/2;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_PRESSURE;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_TEMP;
      cfg->drv_cfg[i].sensitivity_default = 0;
      cfg->drv_cfg[i].flags = 0;
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_l3gd20_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_L3GD20;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the L3GD20 gyro is a gyro. So overwrite a gyro part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_GYRO_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 150000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      if( dev_info->msm_id == SNS_MSM_8226 ) {
        cfg->drv_cfg[i].i2c_bus = 2;
        cfg->drv_cfg[i].gpio1 = 0xFFFF;
        cfg->drv_cfg[i].i2c_address = 0x6B;
      } else if( dev_info->msm_id == SNS_MSM_8974 ) {
        cfg->drv_cfg[i].i2c_bus = 12;
        cfg->drv_cfg[i].gpio1 = 66;
        cfg->drv_cfg[i].i2c_address = 0x6B;
      } else if( dev_info->msm_id == SNS_APQ_8084 ) {
        cfg->drv_cfg[i].i2c_bus = 8;
        cfg->drv_cfg[i].gpio1 = 60;
        cfg->drv_cfg[i].i2c_address = 0x6A;
      }
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_GYRO_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_GYRO_V01;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_GYRO;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 2;
      cfg->drv_cfg[i].device_select = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_lis3dh_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array,
                              dev_info_t *dev_info,
                              regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_LIS3DH;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the LIS3DH is an accel. So overwrite an accel part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_ACCEL_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 1000;
      cfg->drv_cfg[i].idle_to_ready = 100000;
      cfg->drv_cfg[i].i2c_bus = 12;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio1 = 0xFFFF;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_ACCEL_V01;
      cfg->drv_cfg[i].i2c_address = 0x18;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_ACCEL;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;

      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM1_LIS3DH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02,
                       SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM4 );
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_lis3dh_gen5_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array,
                                   dev_info_t *dev_info,
                                   regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_LIS3DH;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the LIS3DH is an accel. So overwrite an accel part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_ACCEL_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 1000;
      cfg->drv_cfg[i].idle_to_ready = 100000;
      cfg->drv_cfg[i].i2c_bus = 12;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio1 = 0xFFFF;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_ACCEL_V01;
      cfg->drv_cfg[i].i2c_address = 0x19;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_ACCEL;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM1_LIS3DH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02,
                       SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM4 );
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_lis3dsh_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array,
                              dev_info_t *dev_info,
                              regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_LIS3DSH;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the LIS3DSH is an accel. So overwrite an accel part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_ACCEL_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 10000;
      cfg->drv_cfg[i].idle_to_ready = 30000;
      if( dev_info->msm_id == SNS_MSM_8226 ) {
         cfg->drv_cfg[i].i2c_bus = 2;
         cfg->drv_cfg[i].gpio1 = 63;
         cfg->drv_cfg[i].i2c_address = 0x1D;
      } else if( dev_info->msm_id == SNS_APQ_8084 ) {
         cfg->drv_cfg[i].i2c_bus = 8;
         cfg->drv_cfg[i].gpio1 = 76;
         cfg->drv_cfg[i].i2c_address = 0x1E;
      }
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_ACCEL_V01;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_ACCEL;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM1_LIS3DSH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02,
                       SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DSH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DSH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DSH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM4 );
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_lis3dsh_8226_tap_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array,
                                  dev_info_t *dev_info,
                                  regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx, err = INIT_ERR_FAILED;
  uint8_t uuid[16] = SNS_REG_UUID_LIS3DSH;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the LIS3DSH has a doubletap sensor. So overwrite if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_TAP_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 10000;
      cfg->drv_cfg[i].idle_to_ready = 30000;
      cfg->drv_cfg[i].i2c_bus = 2;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = 0xFFFF;
      cfg->drv_cfg[i].gpio1 = 63;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_TAP_V01;
      cfg->drv_cfg[i].i2c_address = 0x1D;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_DOUBLETAP;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;;
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  if(cfg_done) {
    err = INIT_SUCCESS;
  }

  return err;
}

int init_lis3dsh_gen5j_8974_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array,
                                           dev_info_t *dev_info,
                                           regitems_t *regitems )
{
  int i, cfg_idx, err = INIT_ERR_FAILED;
  uint8_t uuid[16] = SNS_REG_UUID_LIS3DSH;
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      /* the LIS3DSH is an accel. So overwrite an accel part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_ACCEL_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 10000;
      cfg->drv_cfg[i].idle_to_ready = 30000;
      cfg->drv_cfg[i].i2c_bus = 0x0c;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio1 = 0x0049;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_ACCEL_V01;
      cfg->drv_cfg[i].i2c_address = 0x1D;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_ACCEL;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg->drv_cfg[i].device_select = 0;

      cfg_done = true;
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM1_LIS3DSH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02,
                       SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DSH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DSH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DSH );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM4 );
    }
  }

  if(cfg_done) {
    err = INIT_SUCCESS;
  }

  return err;
}

int init_lis3dsh_gen5j_8974_tap_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array,
                                        dev_info_t *dev_info,
                                        regitems_t *regitems )
{
  int i, cfg_idx, err = INIT_ERR_FAILED;
  uint8_t uuid[16] = SNS_REG_UUID_LIS3DSH;
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      /* the LIS3DSH has a doubletap sensor. So overwrite if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_TAP_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 10000;
      cfg->drv_cfg[i].idle_to_ready = 30000;
      cfg->drv_cfg[i].i2c_bus = 0x0c;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = 0xFFFF;
      cfg->drv_cfg[i].gpio1 = 0x0049;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_TAP_V01;
      cfg->drv_cfg[i].i2c_address = 0x1D;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_DOUBLETAP;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  if(cfg_done) {
    err = INIT_SUCCESS;
  }

  return err;
}

int init_mpu6515_gen7_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int i, cfg_idx, err = INIT_ERR_FAILED;
  uint8_t uuid[16] = SNS_REG_UUID_MPU6515;
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_ACCEL_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 0x0c;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio1 = 0x0042;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_ACCEL_V01;
      cfg->drv_cfg[i].i2c_address = 0x69;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_ACCEL;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY | SNS_REG_SSI_FLAG_SELF_SCHED | SNS_REG_SSI_FLAG_ENABLE_FIFO;
      cfg->drv_cfg[i].device_select = 0;

      cfg_done = true;
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM1_MPU6500 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02,
                       SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM4 );
    }
  }

  if(cfg_done) {
    err = INIT_SUCCESS;
  }

  return err;
}

int init_mpu6515_gen7_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_MPU6515;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_GYRO_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 0x0C;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_GYRO_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio1 = 0x0042;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_GYRO_V01;
      cfg->drv_cfg[i].i2c_address = 0x69;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_GYRO;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].device_select = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY | SNS_REG_SSI_FLAG_SELF_SCHED | SNS_REG_SSI_FLAG_ENABLE_FIFO;

      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6515_gen7_akm8963_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_MPU6515_AKM8963;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_MAG_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 0x0C;
      cfg->drv_cfg[i].i2c_address = 0x0D;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_MAG_V02;
      cfg->drv_cfg[i].cal_pri_group_id = 0xFFFF;
      cfg->drv_cfg[i].gpio1 = 0x0042;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_MAG_V01;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_MAG;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY | SNS_REG_SSI_FLAG_SELF_SCHED | SNS_REG_SSI_FLAG_ENABLE_FIFO;
      cfg->drv_cfg[i].device_select = 0;

      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6515_gen7_bmp280_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_MPU6515_BMP280;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PRESSURE_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 0x0C;
      cfg->drv_cfg[i].i2c_address = 0x76;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_PRESSURE_V02;
      cfg->drv_cfg[i].cal_pri_group_id = 0xFFFF;
      cfg->drv_cfg[i].gpio1 = 0x0042;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PRESSURE_V01;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_PRESSURE;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY | SNS_REG_SSI_FLAG_SELF_SCHED | SNS_REG_SSI_FLAG_ENABLE_FIFO;
      cfg->drv_cfg[i].device_select = 0;

      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_mpu6515_gen7_stepevt_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int i, cfg_idx, err = INIT_ERR_FAILED;
  uint8_t uuid[16] = SNS_REG_UUID_MPU6515;
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_STEP_EVENT_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 0x0c;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = 0xFFFF;
      cfg->drv_cfg[i].gpio1 = 0x0042;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_STEP_EVENT_V01;
      cfg->drv_cfg[i].i2c_address = 0x69;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_STEP_EVENT;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY | SNS_REG_SSI_FLAG_SELF_SCHED | SNS_REG_SSI_FLAG_ENABLE_FIFO;
      cfg->drv_cfg[i].device_select = 0;

      cfg_done = true;
    }
  }

  if(cfg_done) {
    err = INIT_SUCCESS;
  }

  return err;
}

int init_mpu6515_gen7_stepcount_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int i, cfg_idx, err = INIT_ERR_FAILED;
  uint8_t uuid[16] = SNS_REG_UUID_MPU6515;
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_STEP_COUNT_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 0x0c;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = 0xFFFF;
      cfg->drv_cfg[i].gpio1 = 0x0042;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_STEP_COUNT_V01;
      cfg->drv_cfg[i].i2c_address = 0x69;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_STEP_COUNT;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY | SNS_REG_SSI_FLAG_SELF_SCHED | SNS_REG_SSI_FLAG_ENABLE_FIFO;
      cfg->drv_cfg[i].device_select = 0;

      cfg_done = true;
    }
  }

  if(cfg_done) {
    err = INIT_SUCCESS;
  }

  return err;
}

int init_mpu6515_gen7_smd_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int i, cfg_idx, err = INIT_ERR_FAILED;
  uint8_t uuid[16] = SNS_REG_UUID_MPU6515;
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_SMD_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 0x0c;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = 0xFFFF;
      cfg->drv_cfg[i].gpio1 = 0x0042;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_SMD_V01;
      cfg->drv_cfg[i].i2c_address = 0x69;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_SMD;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY | SNS_REG_SSI_FLAG_SELF_SCHED | SNS_REG_SSI_FLAG_ENABLE_FIFO;
      cfg->drv_cfg[i].device_select = 0;

      cfg_done = true;
    }
  }

  if(cfg_done) {
    err = INIT_SUCCESS;
  }

  return err;
}

int init_mpu6515_gen7_gamerv_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  int i, cfg_idx, err = INIT_ERR_FAILED;
  uint8_t uuid[16] = SNS_REG_UUID_MPU6515;
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i++ ) {
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_GAME_ROTATION_VECTOR_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 250000;
      cfg->drv_cfg[i].i2c_bus = 0x0c;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = 0xFFFF;
      cfg->drv_cfg[i].gpio1 = 0x0042;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_GAME_ROTATION_VECTOR_V01;
      cfg->drv_cfg[i].i2c_address = 0x69;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_GAME_RV;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY | SNS_REG_SSI_FLAG_SELF_SCHED | SNS_REG_SSI_FLAG_ENABLE_FIFO;
      cfg->drv_cfg[i].device_select = 0;

      cfg_done = true;
    }
  }

  if(cfg_done) {
    err = INIT_SUCCESS;
  }

  return err;
}

int init_bma2x2_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_BMA2X2;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the BMA222E is an accel. So overwrite an accel part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_ACCEL_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 2000;
      cfg->drv_cfg[i].idle_to_ready = 100000;
      if( dev_info->msm_id == SNS_MSM_8226 ) {
         cfg->drv_cfg[i].i2c_bus = 2;
         cfg->drv_cfg[i].gpio1 = 63;
      } else if( dev_info->msm_id == SNS_APQ_8084 ) {
         cfg->drv_cfg[i].i2c_bus = 8;
         cfg->drv_cfg[i].gpio1 = 76;
      }
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_ACCEL_V01;
      cfg->drv_cfg[i].i2c_address = 0x18;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_ACCEL;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM1_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM2_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02,
                       SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM1_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM3_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM2_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM4_BMA2X2 );
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_lsm303dlhc_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_LSM303DLHC;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the LSM303DLHC is a mag. So overwrite an mag part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_MAG_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 0;
      cfg->drv_cfg[i].idle_to_ready = 0;
      cfg->drv_cfg[i].i2c_bus = 12;
      cfg->drv_cfg[i].reg_group_id = 0xFFFF;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      cfg->drv_cfg[i].gpio1 = 0xFFFF;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_MAG_V01;
      cfg->drv_cfg[i].i2c_address = 0x1E;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_MAG;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_yas530_8974_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_YAS530;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the YAS530 is a mag. So overwrite an mag part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_MAG_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 0;
      cfg->drv_cfg[i].idle_to_ready = 0;
      cfg->drv_cfg[i].i2c_bus = 12;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_MAG_V02;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      cfg->drv_cfg[i].gpio1 = 0xFFFF;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_MAG_V01;
      cfg->drv_cfg[i].i2c_address = 0x2E;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_MAG;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_hscdtd008_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_HSCD008;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the HSCDTD is a mag. So overwrite mag part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_MAG_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 100000;
      cfg->drv_cfg[i].idle_to_ready = 0;
      if( SNS_MSM_8226 == dev_info->msm_id ) {
         cfg->drv_cfg[i].i2c_bus = 2;
      } else if( SNS_APQ_8084 == dev_info->msm_id ) {
         cfg->drv_cfg[i].i2c_bus = 8; /* On NA993 LES vertigo board. Need overall verification */
      } else if( SNS_MSM_8994 == dev_info->msm_id ) {
         cfg->drv_cfg[i].off_to_idle = 0;
         cfg->drv_cfg[i].i2c_bus = 5;
      }
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_MAG_V02;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      if( SNS_MSM_8226 == dev_info->msm_id ) {
         cfg->drv_cfg[i].gpio1 = 66;
      } else if( SNS_APQ_8084 == dev_info->msm_id ) {
         cfg->drv_cfg[i].gpio1 = 0xFFFF;
      } else if( SNS_MSM_8994 == dev_info->msm_id ) {
         cfg->drv_cfg[i].gpio1 = 0xFFFF;
      }
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_MAG_V01;
      cfg->drv_cfg[i].i2c_address = 0x0C;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_MAG;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}


int init_lps331ap_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_LPS331AP;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the LPS331AP is a pressure sensor. So overwrite a press part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PRESSURE_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 10000;
      cfg->drv_cfg[i].idle_to_ready = 0;
      if( SNS_MSM_8226 == dev_info->msm_id ) {
        cfg->drv_cfg[i].i2c_bus = 2;
      } else {
        cfg->drv_cfg[i].i2c_bus = 12;
      }
      cfg->drv_cfg[i].reg_group_id = 0xFFFF;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      cfg->drv_cfg[i].gpio1 = 0xFFFF;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PRESSURE_V01;
      cfg->drv_cfg[i].i2c_address = 0x5D;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_PRESSURE;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_TEMP;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_apds99xx_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_APDS99XX;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the APDS99XX is a prox/light sensor. So overwrite a part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PROX_LIGHT_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 5000;
      cfg->drv_cfg[i].idle_to_ready = 0;
      if( SNS_MSM_8974 == dev_info->msm_id )
      {
        cfg->drv_cfg[i].i2c_bus = 3;
      }
      else if( SNS_MSM_8226 == dev_info->msm_id )
      {
        cfg->drv_cfg[i].i2c_bus = 2;
      }
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_PROX_LIGHT_V02;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      if( dev_info->is_dri_mode ) {
        if( SNS_MSM_8974 == dev_info->msm_id )
        {
          cfg->drv_cfg[i].gpio1 = (dev_info->platform == SNS_PLATFORM_LIQUID)?9:74;
        }
        else if( SNS_MSM_8226 == dev_info->msm_id )
        {
          cfg->drv_cfg[i].gpio1 = 65;
        }
      } else {
        cfg->drv_cfg[i].gpio1 = 0xFFFF;
      }
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;
      cfg->drv_cfg[i].i2c_address = 0x39;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_PROXIMITY;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_AMBIENT;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_apds9950_8xxx_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_APDS9950;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the APDS99XX is a prox/light sensor. So overwrite a part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PROX_LIGHT_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 10000;
      cfg->drv_cfg[i].idle_to_ready = 10000;
      if( SNS_MSM_8974 == dev_info->msm_id ) {
        cfg->drv_cfg[i].i2c_bus = 3;
      } else if( SNS_MSM_8226 == dev_info->msm_id ) {
        cfg->drv_cfg[i].i2c_bus = 2;
      } else if( SNS_APQ_8084 == dev_info->msm_id ) {
        cfg->drv_cfg[i].i2c_bus = 12;
      }
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_PROX_LIGHT_V02;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      if( dev_info->is_dri_mode ) {
        if( SNS_MSM_8974 == dev_info->msm_id ) {
          cfg->drv_cfg[i].gpio1 = (dev_info->platform == SNS_PLATFORM_LIQUID)?9:74;
        } else if( SNS_MSM_8226 == dev_info->msm_id ) {
          cfg->drv_cfg[i].gpio1 = 65;
        } else if( SNS_APQ_8084 == dev_info->msm_id ) {
          cfg->drv_cfg[i].gpio1 = 77;
        }
      } else {
        cfg->drv_cfg[i].gpio1 = 0xFFFF;
      }
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;
      cfg->drv_cfg[i].i2c_address = 0x39;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_PROXIMITY;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_AMBIENT;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_tmd277x_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_TMD277X;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the TMD277X is a prox/light sensor. So overwrite a part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PROX_LIGHT_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 5000;
      cfg->drv_cfg[i].idle_to_ready = 0;
      if( SNS_MSM_8226 == dev_info->msm_id ) {
         cfg->drv_cfg[i].i2c_bus = 2;
      } else if ( SNS_APQ_8084 == dev_info->msm_id ) {
         cfg->drv_cfg[i].i2c_bus = 8; /* On NA993 LES vertigo board. Need overall verification */
      }
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_PROX_LIGHT_V02;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      if( dev_info->is_dri_mode ) {
        if( SNS_MSM_8226 == dev_info->msm_id ) {
           cfg->drv_cfg[i].gpio1 = 65;
        } else if ( SNS_APQ_8084 == dev_info->msm_id ) {
           cfg->drv_cfg[i].gpio1 = 77;
        }
      } else {
        cfg->drv_cfg[i].gpio1 = 0xFFFF;
      }
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;
      cfg->drv_cfg[i].i2c_address = 0x39;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_PROXIMITY;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_AMBIENT;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_ltr55x_8226_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_LTR55X;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the LTR55X is a prox/light sensor. So overwrite a part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PROX_LIGHT_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 5000;
      cfg->drv_cfg[i].idle_to_ready = 0;
      cfg->drv_cfg[i].i2c_bus = 2;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_PROX_LIGHT_V02;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].gpio1 = 65;
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].gpio1 = 0xFFFF;
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;
      cfg->drv_cfg[i].i2c_address = 0x2E;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_PROXIMITY;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_AMBIENT;
      cfg->drv_cfg[i].sensitivity_default = 0;
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_max44009_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_MAX44009;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the MAX44009 is a light sensor. So overwrite a part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PROX_LIGHT_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 1000000;
      cfg->drv_cfg[i].idle_to_ready = 1000000;
      cfg->drv_cfg[i].i2c_bus = 3;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_PROX_LIGHT_V02;
      cfg->drv_cfg[i].cal_pri_group_id = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].gpio1 = 49;
      } else {
        cfg->drv_cfg[i].gpio1 = 0xFFFF;
      }
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;
      cfg->drv_cfg[i].i2c_address = 0x4A;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_AMBIENT;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_shtc1_8xxx_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_SHTC1;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the SHTC1 is a humidity sensor. So overwrite a humidity part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_HUMIDITY_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 10000;
      cfg->drv_cfg[i].idle_to_ready = 10000;
      if( SNS_MSM_8994 == dev_info->msm_id ) {
        cfg->drv_cfg[i].i2c_bus = 7;
      } else {
        cfg->drv_cfg[i].i2c_bus = 3;
      }
      cfg->drv_cfg[i].reg_group_id = -1;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      cfg->drv_cfg[i].gpio1 = 0xFFFF;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_HUMIDITY_V01;
      cfg->drv_cfg[i].i2c_address = 0x70;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_HUMIDITY;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_AMBIENT_TEMP;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_bmi058_accel_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_BMA2X2;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /*Overwrite an accel part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_ACCEL_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 3000;
      cfg->drv_cfg[i].idle_to_ready = 257800;
      cfg->drv_cfg[i].i2c_bus = 5;
      cfg->drv_cfg[i].gpio1 = 64;

      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_ACCEL_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_ACCEL_V01;
      cfg->drv_cfg[i].i2c_address = 0x18;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_ACCEL;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 1;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM1_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V02,
                       SNS_REG_AMD_DEF_INT_CFG_PARAM2_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V02,
                       SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM1_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM3_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM2_BMA2X2 );
      add_to_regitems( regitems,
                       SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V02,
                       SNS_REG_RMD_DEF_INT_CFG_PARAM4_BMA2X2 );
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_bmi058_gyro_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_BMG160;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the BMI058/BMG160 is a gyro. So overwrite a gyro part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_GYRO_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 30000;
      cfg->drv_cfg[i].idle_to_ready = 60000;
      cfg->drv_cfg[i].i2c_bus = 5;
      cfg->drv_cfg[i].gpio1 = 42;
      cfg->drv_cfg[i].i2c_address = 0x68;

      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_GYRO_V02;
      cfg->drv_cfg[i].cal_pri_group_id = SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V02;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_GYRO_V01;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_GYRO;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_NONE;
      cfg->drv_cfg[i].sensitivity_default = 4;
      cfg->drv_cfg[i].device_select = 0;
      cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_NO_SENSITIVITY;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_SELF_SCHED;
      }
      if( dev_info->is_fifo_mode ) {
        cfg->drv_cfg[i].flags |= SNS_REG_SSI_FLAG_ENABLE_FIFO;
      }
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_lps25h_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_LPS25H;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the LPS25H is a pressure sensor. So overwrite a press part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PRESSURE_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 10000;
      cfg->drv_cfg[i].idle_to_ready = 0;
      cfg->drv_cfg[i].i2c_bus = 7;
      cfg->drv_cfg[i].reg_group_id = 0xFFFF;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      cfg->drv_cfg[i].gpio1 = 0xFFFF;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PRESSURE_V01;
      cfg->drv_cfg[i].i2c_address = 0x5C;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_PRESSURE;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_TEMP;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_tmg399x_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_TMG399X;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the TMG399X is a prox/light sensor. So overwrite a part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_PROX_LIGHT_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 5000;
      cfg->drv_cfg[i].idle_to_ready = 5000;
      cfg->drv_cfg[i].i2c_bus = 7;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_PROX_LIGHT_V02;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].gpio1 = 40;
      } else {
        cfg->drv_cfg[i].gpio1 = 0xFFFF;
      }
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;
      cfg->drv_cfg[i].i2c_address = 0x39;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_PROXIMITY;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_AMBIENT;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_tmg399x_rgb_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  bool cfg_done = false;
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  uint8_t uuid[16] = SNS_REG_UUID_TMG399X;

  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG && !cfg_done; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      /* the TMG399X is a RGB sensor. So overwrite a part if found */
      if( cfg->drv_cfg[i].sensor_id == SNS_SMGR_ID_RGB_V01 ||
          memcmp(cfg->drv_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0 ) {
        break;
      }
    }
    if( i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS ) {
      memcpy(cfg->drv_cfg[i].drvuuid, uuid, sizeof(uuid));
      cfg->drv_cfg[i].off_to_idle = 5000;
      cfg->drv_cfg[i].idle_to_ready = 5000;
      cfg->drv_cfg[i].i2c_bus = 7;
      cfg->drv_cfg[i].reg_group_id = SNS_REG_DRIVER_GROUP_PROX_LIGHT_V02;
      cfg->drv_cfg[i].cal_pri_group_id = -1;
      cfg->drv_cfg[i].gpio2 = 0xFFFF;
      cfg->drv_cfg[i].sensor_id = SNS_SMGR_ID_RGB_V01;
      cfg->drv_cfg[i].i2c_address = 0x39;
      cfg->drv_cfg[i].data_type1 = SNS_REG_SSI_DATA_TYPE_RGB;
      cfg->drv_cfg[i].data_type2 = SNS_REG_SSI_DATA_TYPE_CT_C;
      cfg->drv_cfg[i].sensitivity_default = 0;
      if( dev_info->is_dri_mode ) {
        cfg->drv_cfg[i].gpio1 = 40;
        cfg->drv_cfg[i].flags = SNS_REG_SSI_FLAG_SELF_SCHED;
      } else {
        cfg->drv_cfg[i].flags = 0;
        cfg->drv_cfg[i].gpio1 = 0xFFFF;
      }
      cfg->drv_cfg[i].device_select = 0;
      cfg_done = true;
    }
  }

  return (cfg_done) ? INIT_SUCCESS : INIT_ERR_FAILED;
}

int init_null_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, int sensor_id )
{
  sns_reg_ssi_smgr_cfg_group_s *cfg;
  int i, cfg_idx;
  for(cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG; cfg_idx++)
  {
    cfg = &cfg_array[cfg_idx];

    for( i = 0; i < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; i ++ ) {
      if( cfg->drv_cfg[i].sensor_id == sensor_id ) {
        memset( &cfg->drv_cfg[i], 0, sizeof((cfg->drv_cfg[i])) );
        cfg->drv_cfg[i].sensor_id = sensor_id;
        cfg->drv_cfg[i].drvuuid[0] = 0xFF;
      }
    }
  }

  return INIT_SUCCESS;
}
int init_accelnull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  return init_null_config( cfg_array, SNS_SMGR_ID_ACCEL_V01 );
}
int init_gyronull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  return init_null_config( cfg_array, SNS_SMGR_ID_GYRO_V01 );
}
int init_magnull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  return init_null_config( cfg_array, SNS_SMGR_ID_MAG_V01 );
}
int init_proxnull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  return init_null_config( cfg_array, SNS_SMGR_ID_PROX_LIGHT_V01 );
}
int init_pressnull_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, dev_info_t *dev_info, regitems_t *regitems )
{
  return init_null_config( cfg_array, SNS_SMGR_ID_PRESSURE_V01 );
}

int init_sensor_config( sns_reg_ssi_smgr_cfg_group_s *cfg_array, const char * string,
                        dev_info_t *dev_info, regitems_t *regitems )
{
  int i, err = INIT_ERR_FAILED;

  for( i=0; i < (int)ARRAY_SIZE(uuid_array); i++ ) {
    if( strcasecmp( uuid_array[i].name, string ) == 0 ) {
      if( uuid_array[i].cfg_init_fptr == NULL ) {
        printf("No init function for %s\n", string);
      } else {
        err = uuid_array[i].cfg_init_fptr( cfg_array, dev_info, regitems );
        printf("Updating registry with driver %s: %s\n", uuid_array[i].name, (err==INIT_SUCCESS) ? "OK" : "FAILED" );
        break;
      }
    }
  }

  if(i >= (int)ARRAY_SIZE(uuid_array)) {
    printf("Unknown driver %s\n", string);
  }

  return err;
}

void populate_devinfo_group_from_smgr_cfg( const sns_reg_ssi_smgr_cfg_group_s *smgr_cfg_array,
                                           sns_reg_ssi_devinfo_group_s *devinfo_accel,
                                           sns_reg_ssi_devinfo_group_s *devinfo_gyro,
                                           sns_reg_ssi_devinfo_group_s *devinfo_mag,
                                           sns_reg_ssi_devinfo_group_s *devinfo_pressure,
                                           sns_reg_ssi_devinfo_group_s *devinfo_prox_light )
{
  int i,j,cfg_idx;
  sns_reg_ssi_devinfo_group_s *devinfo;
  const sns_reg_ssi_smgr_cfg_group_s *smgr_cfg;

  for( cfg_idx = 0; cfg_idx < SNS_REG_SSI_NUM_SMGR_CFG; cfg_idx++) {
    smgr_cfg = &smgr_cfg_array[cfg_idx];

    for( j = 0; j < SNS_REG_SSI_SMGR_CFG_NUM_SENSORS; j++ ) {
      if( memcmp( smgr_cfg->drv_cfg[j].drvuuid, null_uuid, sizeof(null_uuid) ) == 0 ) {
        continue;
      }
      switch( smgr_cfg->drv_cfg[j].sensor_id ) {
        case SNS_SMGR_ID_ACCEL_V01:
          devinfo = devinfo_accel;
          break;
        case SNS_SMGR_ID_GYRO_V01:
          devinfo = devinfo_gyro;
          break;
        case SNS_SMGR_ID_MAG_V01:
          devinfo = devinfo_mag;
          break;
        case SNS_SMGR_ID_PRESSURE_V01:
          devinfo = devinfo_pressure;
          break;
        case SNS_SMGR_ID_PROX_LIGHT_V01:
          devinfo = devinfo_prox_light;
          break;
        default:
          devinfo = NULL;
          fprintf( stderr, "Internal error! Invalid/unknown sensor_id: %d\n",
                   smgr_cfg->drv_cfg[j].sensor_id );
          printf("Error with UUID: ");
          print_uuid( smgr_cfg->drv_cfg[j].drvuuid );
          continue;
          break;
      }

      devinfo->min_ver_no = 1;
      devinfo->reserved = 0;

      for( i = 0; i < SNS_REG_SSI_DEVINFO_NUM_CFGS; i++ ) {
        if(memcmp(devinfo->uuid_cfg[i].drvuuid, null_uuid, sizeof(null_uuid)) == 0) {
          sns_reg_ssi_devinfo_uuid_cfg_s *cfg = &devinfo->uuid_cfg[i];
          devinfo->num_uuid_dev_info_valid++;

          memcpy( devinfo->uuid_cfg[i].drvuuid,
                  smgr_cfg->drv_cfg[j].drvuuid,
                  sizeof(smgr_cfg->drv_cfg[j].drvuuid) );
          cfg->off_to_idle         = smgr_cfg->drv_cfg[j].off_to_idle;
          cfg->idle_to_ready       = smgr_cfg->drv_cfg[j].idle_to_ready;
          cfg->gpio1               = smgr_cfg->drv_cfg[j].gpio1;
          cfg->reg_group_id        = smgr_cfg->drv_cfg[j].reg_group_id;
          cfg->cal_pri_group_id    = smgr_cfg->drv_cfg[j].cal_pri_group_id;
          cfg->i2c_bus             = smgr_cfg->drv_cfg[j].i2c_bus;
          cfg->i2c_address         = smgr_cfg->drv_cfg[j].i2c_address;
          cfg->sensitivity_default = smgr_cfg->drv_cfg[j].sensitivity_default;
          cfg->flags               = smgr_cfg->drv_cfg[j].flags;
          cfg->reserved1 = 0;
          cfg->reserved2 = 0;
          cfg->reserved3 = 0;
          break;
        }
      }
      if( i == SNS_REG_SSI_DEVINFO_NUM_CFGS ) {
        fprintf(stderr,
                "Too many sensors of SMGR type %d! Max sensors per type is 6\n",
                smgr_cfg->drv_cfg[j].sensor_id);
      }
    }
  }
}

static sensor1_error_e write_sensor_dep_group_to_registry(
  sns_reg_ssi_sensor_dep_reg_group_s * data,
  uint16_t group_id,
  data_t *data_info)
{
  sensor1_error_e err;
  sensor1_msg_header_s msg_hdr;
  sns_reg_group_write_req_msg_v02 *msg;
  sns_reg_group_write_resp_msg_v02 *resp_msg;

  msg_hdr.service_number = SNS_REG2_SVC_ID_V01;
  msg_hdr.msg_id = SNS_REG_GROUP_WRITE_REQ_V02;
  msg_hdr.msg_size = sizeof(sns_reg_group_write_req_msg_v02);
  msg_hdr.txn_id = 10;

  err = sensor1_alloc_msg_buf( data_info->hndl, msg_hdr.msg_size, (void**)&msg );
  if( err != SENSOR1_SUCCESS ) {
    printf("Failed to allocate memory for sensor dep group %d write request\n", group_id);
    return err;
  }
  msg->group_id = group_id;
  msg->data_len = sizeof(sns_reg_ssi_sensor_dep_reg_group_s);
  memcpy(msg->data, data, msg->data_len );

  pthread_mutex_lock( &data_info->mutex );
  data_info->resp_data_size = -1;

  err = sensor1_write( data_info->hndl,
                       &msg_hdr,
                       msg );
  if( err == SENSOR1_SUCCESS ) {
    while( data_info->resp_data_size == -1 ) {
      pthread_cond_wait( &data_info->cond, &data_info->mutex );
    }
    resp_msg = (sns_reg_group_write_resp_msg_v02*)data_info->resp_data;
    if( resp_msg->resp.sns_result_t == 0 && resp_msg->resp.sns_err_t == SENSOR1_SUCCESS ) {
      printf("SSI sensor dependent group %d registry write successful\n", group_id);
    } else {
      printf("SSI sensor dependent group %d registry write FAILED %x %x\n",
             group_id, resp_msg->resp.sns_result_t, resp_msg->resp.sns_err_t );
    }
  } else {
    printf("Registry write failed. sensor1_write request failed %d\n", err );
  }
  pthread_mutex_unlock( &data_info->mutex );
  return err;
}

uint8_t write_sensor_dependent_registry_defaults(
  int index,
  uint8_t group_id_index,
  data_t *data_info)
{
  sensor_dep_reg_map_t * sensor_cfg = &sensor_dep_reg_item_table[index];
  sensor_dep_reg_item_t * items = sensor_cfg->items;
  int num_reg_items = sensor_cfg->num_reg_items;
  sensor_dep_reg_values_t * values = sensor_cfg->values;
  int num_sensors = sensor_cfg->num_sensors;
  int i_start = 0, j_start = 0;
  uint16_t group_id;
  uint8_t num_groups_added = 0;

  printf("---------------------------------------------------------------------\n");
  printf("Populating default values for %s dependent registry items....\n",sensor_cfg->name);
  printf("Num of %s sensors: %d, Num of registry items per sensor: %d\n",
         sensor_cfg->name, num_sensors, num_reg_items );
  printf("Max num of sensors per group: %d, Max num of registry items per group: %d\n",
         SNS_REG_MAX_SENSORS_WITH_DEP_REG_ITEMS, SNS_REG_MAX_SENSOR_DEP_REG_ITEMS );

  if( num_sensors > 0 && num_reg_items > 0 ) {
    group_id = get_sensor_dep_reg_group_id( group_id_index );
    if( group_id == 0 ) {
      printf("Insufficient sensor dependent registry group ids. Could not continue...\n");
      return 0;
    }
    while( group_id != 0 ) {
      sns_reg_ssi_sensor_dep_reg_group_s sensor_dep_cfg;
      int i1, j1, i2, j2 = 0;
      bool all_done, space_available;

      memset( &sensor_dep_cfg, 0, sizeof(sns_reg_ssi_sensor_dep_reg_group_s) );
      sensor_dep_cfg.ver_no = 1;
      sensor_dep_cfg.sensor_type = index+1;
      sensor_dep_cfg.next_group_id = 0;

      i1 = 0; j1 = 0;
      // Fill up current group
      do {
        all_done = true;
        space_available = false;

        for( i2 = i_start; (i1 < SNS_REG_MAX_SENSORS_WITH_DEP_REG_ITEMS) && (i2 < num_sensors); ++i1, ++i2 ) {
          memcpy( sensor_dep_cfg.uuid_reg_cfg[i1].drvuuid,
                values[i2].drvuuid, sizeof(values[i2].drvuuid) );
          for( j1 = 0, j2 = j_start; (j1 < SNS_REG_MAX_SENSOR_DEP_REG_ITEMS) && (j2 < num_reg_items) ; ++j1, ++j2 ) {
            sensor_dep_cfg.uuid_reg_cfg[i1].reg_items[j1].reg_item_id =
                    items[j2].reg_item_id;
            sensor_dep_cfg.uuid_reg_cfg[i1].reg_items[j1].size =
                    items[j2].size;
            memcpy( sensor_dep_cfg.uuid_reg_cfg[i1].reg_items[j1].value,
                    &values[i2].val_ptr[j2],
                    sensor_dep_cfg.uuid_reg_cfg[i1].reg_items[j1].size);
          }
        }
        if( i1 < SNS_REG_MAX_SENSORS_WITH_DEP_REG_ITEMS ) {
          space_available = true;
        }
        if( i2 < num_sensors ) {
          // Pending sensors
          i_start = i2;
          j_start = 0;
          all_done = false;
        }
        else if( j2 < num_reg_items ) {
          // Pending registry items
          i_start = 0;
          j_start = j2;
          all_done = false;
        }
      } while( space_available && !all_done );

      // If group is completely full, need to create another one for the remaining items
      if( !space_available && !all_done ) {
        printf("Won't fit in one reg entry. Need to create one more \n");
        group_id_index++;
        // Link to second group
        sensor_dep_cfg.next_group_id = get_sensor_dep_reg_group_id( group_id_index );
      }

      // Write SNS_REG_GROUP_SSI_SENSOR_DEP_CFG* to registry
      printf("------------------------------------------------\n");
      printf("Writing to registry group id: %d\n", group_id);
      print_sensor_dep_group( &sensor_dep_cfg );
      if( SENSOR1_SUCCESS == write_sensor_dep_group_to_registry( &sensor_dep_cfg, group_id, data_info) ) {
        num_groups_added++;

        // goto next group
        group_id = sensor_dep_cfg.next_group_id;
        if( ( group_id == 0 ) && !all_done ) {
          // Not enough registry groups. Need more group ids
          printf("Insufficient registry group ids. Not all items were processed!!!\n");
        }
      } else {
        printf("Failed to write sensor dependent registry group id %d to registry. Bailing out!!!\n",group_id);
        break;
      }
    }
  }
  return num_groups_added;
}

void write_all_sensor_dependent_registry_defaults(data_t *data_info)
{
  int i;
  uint8_t group_id_index = 0;

  for( i = 0; i < (int)ARRAY_SIZE(sensor_dep_reg_item_table); ++i ) {
    sensor_dep_reg_map_t * sensor_cfg = &sensor_dep_reg_item_table[i];
    if( sensor_cfg->items != NULL && sensor_cfg->values != NULL ) {
      group_id_index += write_sensor_dependent_registry_defaults( i, group_id_index, data_info);
    }
  }
}

static void perform_read( data_t *data_info)
{
  regitems_t regitems[NUM_REG_ITEMS];
  sns_reg_ssi_smgr_cfg_group_s cur_cfg[SNS_REG_SSI_NUM_SMGR_CFG];
  int i, j;

  printf("Reading existing registry...\n");
  for( i = 0; i < NUM_REG_ITEMS; i++ ) {
    regitems[i].item_id = -1;
  }
  for(j = 0; j < SNS_REG_SSI_NUM_SMGR_CFG; j++) {
    if( SENSOR1_SUCCESS == read_reg_data( data_info, &cur_cfg[j], j, regitems ) ) {
      print_smgr_cfg_group( &cur_cfg[j], j, regitems );
    }
  }
}

void perform_write( data_t *data_info, dev_info_t *dev_info, int argc, char * const argv[] )
{
  sns_reg_ssi_smgr_cfg_group_s msm_cfg[SNS_REG_SSI_NUM_SMGR_CFG];
  regitems_t                   regitems[NUM_REG_ITEMS];
  int                          i;
  int                          opt;
  int                          n;

  for( i = 0; i < NUM_REG_ITEMS; i++ ) {
    regitems[i].item_id = -1;
  }

  init_default_config( msm_cfg, regitems, dev_info);

  optind = 1;
  /* Parse the sensor options */
  while( (opt = getopt(argc, argv, OPTSTRING) ) != -1 ) {
    switch(opt) {
      case 's':
        dev_info->is_dri_mode = false;
        dev_info->is_fifo_mode = false;
        init_sensor_config( msm_cfg, optarg, dev_info, regitems );
        break;
      case 'd':
        dev_info->is_dri_mode = true;
        dev_info->is_fifo_mode = false;
        init_sensor_config( msm_cfg, optarg, dev_info, regitems );
        break;
      case 'f':
        dev_info->is_dri_mode = true;
        dev_info->is_fifo_mode = true;
        init_sensor_config( msm_cfg, optarg, dev_info, regitems );
        break;
      default:
        break;
    }
  }

  printf("Programming registry...\n");
  setup_related_sensor( msm_cfg );
  for(n = 0; n < SNS_REG_SSI_NUM_SMGR_CFG; n++) {
    if( SENSOR1_SUCCESS == write_reg_data( data_info, &msm_cfg[n], n, regitems ) ) {
      print_smgr_cfg_group( &msm_cfg[n], n, regitems );
    }
  }
}

void perform_autodetect( data_t *data_info, dev_info_t *dev_info, int argc, char * const argv[] )
{
  sns_reg_ssi_smgr_cfg_group_s msm_cfg[SNS_REG_SSI_NUM_SMGR_CFG];
  sns_reg_ssi_smgr_cfg_group_s null_cfg[SNS_REG_SSI_NUM_SMGR_CFG];
  regitems_t                   regitems[NUM_REG_ITEMS];
  int                          i, n;
  int                          opt;
  bool                         use_default_config = true;
  sns_reg_ssi_devinfo_group_s  devinfo_accel;
  sns_reg_ssi_devinfo_group_s  devinfo_gyro;
  sns_reg_ssi_devinfo_group_s  devinfo_mag;
  sns_reg_ssi_devinfo_group_s  devinfo_prox_light;
  sns_reg_ssi_devinfo_group_s  devinfo_pressure;
  sensor1_error_e              err;

  for( i = 0; i < NUM_REG_ITEMS; i++ ) {
    regitems[i].item_id = -1;
  }

  init_default_config( msm_cfg, regitems, dev_info );
  memset( &null_cfg, 0, sizeof( null_cfg ) );

  printf("Clearing SSI registry device selection...\n");
  for(n = 0; n < SNS_REG_SSI_NUM_SMGR_CFG; n++) {
    err = write_reg_data( data_info, &null_cfg[n], n, regitems );
    if( SENSOR1_SUCCESS != err ) {
      return;
    }
  }

  printf("Populating devinfo database...\n");
  memset( &devinfo_accel, 0, sizeof( devinfo_accel ) );
  memset( &devinfo_gyro, 0, sizeof( devinfo_gyro ) );
  memset( &devinfo_mag, 0, sizeof( devinfo_mag ) );
  memset( &devinfo_prox_light, 0, sizeof( devinfo_prox_light ) );
  memset( &devinfo_pressure, 0, sizeof( devinfo_pressure ) );

  /* Fill in configurations to search for */
  optind = 1;
  while( (opt = getopt(argc, argv, OPTSTRING) ) != -1 ) {
    switch(opt) {
      case 's': /* Fallthrough...*/
      case 'd':
      case 'f':
        {
          use_default_config = false;
          dev_info->is_dri_mode = (opt=='s')?false:true;
          dev_info->is_fifo_mode = (opt=='f')?true:false;

          init_sensor_config( null_cfg, optarg, dev_info, regitems );
          populate_devinfo_group_from_smgr_cfg( null_cfg, &devinfo_accel,
                                                &devinfo_gyro, &devinfo_mag,
                                                &devinfo_pressure, &devinfo_prox_light );
          memset( &null_cfg, 0, sizeof( null_cfg ) );
        }
      break;
      default:
        break;
    }
  }

  if( use_default_config ) {
    /* Use default 8974/8226/8084 sensors depending on msm type. To be changed based on hardware configurations
     * to be detected */
    printf("Using default 8974/8226/8084 sensors for auto-detect.\n");
    populate_devinfo_group_from_smgr_cfg( msm_cfg, &devinfo_accel,
                                          &devinfo_gyro, &devinfo_mag,
                                          &devinfo_pressure, &devinfo_prox_light );
  }

  printf("Writing devinfo blocks to registry...\n");

  printf("ACCEL Devifo:\n");
  err = write_devinfo_data( data_info, SNS_REG_GROUP_SSI_DEVINFO_ACCEL_V02, &devinfo_accel );
  if( SENSOR1_SUCCESS != err ) {
    return;
  }
  print_devinfo_group( &devinfo_accel );

  printf("GYRO Devifo:\n");
  err = write_devinfo_data( data_info, SNS_REG_GROUP_SSI_DEVINFO_GYRO_V02, &devinfo_gyro );
  if( SENSOR1_SUCCESS != err ) {
    return;
  }
  print_devinfo_group( &devinfo_gyro );

  printf("MAG Devifo:\n");
  err = write_devinfo_data( data_info, SNS_REG_GROUP_SSI_DEVINFO_MAG_V02, &devinfo_mag );
  if( SENSOR1_SUCCESS != err ) {
    return;
  }
  print_devinfo_group( &devinfo_mag );

  printf("PROX_LIGHT Devifo:\n");
  err = write_devinfo_data( data_info, SNS_REG_GROUP_SSI_DEVINFO_PROX_LIGHT_V02, &devinfo_prox_light );
  if( SENSOR1_SUCCESS != err ) {
    return;
  }
  print_devinfo_group( &devinfo_prox_light );

  printf("PRESSURE Devifo:\n");
  err = write_devinfo_data( data_info, SNS_REG_GROUP_SSI_DEVINFO_PRESSURE_V02, &devinfo_pressure );
  if( SENSOR1_SUCCESS != err ) {
    return;
  }
  print_devinfo_group( &devinfo_pressure );

  // Write default values for sensor dependent registry items
  write_all_sensor_dependent_registry_defaults( data_info );
}

/*===========================================================================

  FUNCTION:   sns_detect_msm

  ===========================================================================*/
/*!
  @brief Detects the msm type

*/
/*=========================================================================*/
static sns_msm_id detect_msm()
{
  sns_msm_id msm_id = SNS_MSM_UNKNOWN;
  char board_platform[PROPERTY_VALUE_MAX];

  msm_id = SNS_MSM_UNKNOWN;
  property_get("ro.board.platform", board_platform, "");

  if (!strcmp("msm8226", board_platform))
  {
    msm_id = SNS_MSM_8226;
  }
  else if (!strcmp("msm8974", board_platform))
  {
    msm_id = SNS_MSM_8974;
  }
  else if (!strcmp("apq8084", board_platform))
  {
    msm_id = SNS_APQ_8084;
  }
  else if (!strcmp("msm8994", board_platform))
  {
    msm_id = SNS_MSM_8994;
  }
  else
  {
    printf("device platform not detected\n");
  }

  return msm_id;
}

/*===========================================================================

  FUNCTION:   detect_platform_type

  ===========================================================================*/
/*!
  @brief Detects the platform type

*/
/*=========================================================================*/
static sns_platform detect_platform_type()
{
  FILE *fp;
  char line[100];
  sns_platform platform = SNS_PLATFORM_UNKNOWN;

  fp = fopen("/sys/devices/soc0/hw_platform", "r" );
  if( fp == NULL )
  {
    printf( "hw_platform fopen failed %i", errno );
    return platform;
  }
  else if( fgets(line, sizeof(line), fp) == NULL )
  {
    printf( "hw_platform fgets failed %i", ferror( fp ) );
    return platform;
  }
  else
  {
    if( strstr( line, "FFA" ) != NULL ||
        strstr( line, "MTP" ) != NULL )
    {
      printf("detected SNS_PLATFORM_MTP...\n");
      platform = SNS_PLATFORM_MTP;
    }
    else if( strstr( line, "Surf" ) != NULL )
    {
      printf("detected SNS_PLATFORM_CDP...\n");
      platform = SNS_PLATFORM_CDP;
    }
    else if( strstr( line, "Fluid" ) != NULL )
    {
      printf("detected SNS_PLATFORM_FLUID...\n");
      platform = SNS_PLATFORM_FLUID;
    }
    else if( strstr( line, "Liquid" ) != NULL )
    {
      printf("detected SNS_PLATFORM_LIQUID...\n");
      platform = SNS_PLATFORM_LIQUID;
    }
    else if( strstr( line, "QRD" ) != NULL )
    {
      printf("detected SNS_PLATFORM_QRD...\n");
      platform = SNS_PLATFORM_QRD;
    }
  }

  if( 0 != fclose( fp ) )
  {
    printf("fclose failed: %d",errno );
  }
  return platform;
}

int main( int argc, char * const argv[] )
{
  data_t data_info;
  dev_info_t dev_info;
  int opt;
  operation_e operation = INVALID;
  sensor1_error_e err;

  opterr = 0;

  pthread_mutex_init( &data_info.mutex, NULL );
  pthread_cond_init( &data_info.cond, NULL );

  dev_info.msm_id = detect_msm();
  dev_info.platform = detect_platform_type();

  err = sensor1_open( &data_info.hndl,
                      notify_data_cb,
                      (intptr_t)&data_info );
  if( err != SENSOR1_SUCCESS ) {
    fprintf(stderr, "Unable to open sensor interface. sensor1_open error %d\n", err);
    exit(2);
  }

  if( argc < 2 ) {
    print_usage( argv[0] );
    exit(1);
  }

  /* Did the user ask for help? */
  while( (opt = getopt(argc, argv, OPTSTRING)) != -1 ) {
    if( opt == 'h' || opt == '?') {
      if( opt == '?' ) {
        fprintf(stderr, "Unknown option \'%c\'.\n", optopt);
      }
      print_usage( argv[0] );
      exit(1);
    }
  }
  optind = 1;

  /* Find the user-requested operation */
  while( (opt = getopt(argc, argv, OPTSTRING)) != -1 ) {
    if( ( opt == 'a' || opt == 'r' || opt == 'w' ) &&
        operation != INVALID ) {
      fprintf(stderr,"Error: Must select only ONE operation (-a, -r or -w)\n");
      print_usage( argv[0] );
      exit(1);
    }
    switch(opt) {
      case 'a':
        operation = AUTODETECT;
        break;
      case 'r':
        operation = READ;
        break;
      case 'w':
        operation = WRITE;
        break;
      default:
        break;
    }
  }

  if( operation == READ ) {
    perform_read( &data_info );
  } else if( operation == WRITE ) {
    perform_write( &data_info, &dev_info, argc, argv );
  } else if( operation == AUTODETECT ) {
    perform_autodetect( &data_info, &dev_info, argc, argv );
  } else {
    fprintf(stderr, "Must select one of read/write/autodetect opertations! (-r, -w or -a)\n");
    print_usage( argv[0] );
    exit(1);
  }

  return 0;
}
