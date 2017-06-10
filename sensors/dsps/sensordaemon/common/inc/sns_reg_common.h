#ifndef _SNS_REG_COMMON_H_
#define _SNS_REG_COMMON_H_
/*============================================================================
  @file sns_reg_common.h

  @brief
  Common header file for the Sensors Registry shared between registry and drivers.

  <br><br>

  DEPENDENCIES: None.

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential
  ============================================================================*/

/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#include "fixed_point.h"
#include "comdef.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

/**
 * Default Values for items in group SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V01
 */
#define SNS_REG_ACC_DEF_X_BIAS 0
#define SNS_REG_ACC_DEF_Y_BIAS 0
#define SNS_REG_ACC_DEF_Z_BIAS 0
#define SNS_REG_ACC_DEF_X_SCALE 65536
#define SNS_REG_ACC_DEF_Y_SCALE 65536
#define SNS_REG_ACC_DEF_Z_SCALE 65536

/**
 * Default Values for items in group SNS_REG_DRIVER_GROUP_PROX_LIGHT_V01
 */
#define SNS_REG_DEFAULT_ALP_VISIBLE_LIGHT_TRANS_RATIO 0
#define SNS_REG_DEFAULT_ALP_IR_LIGHT_TRANS_RATIO      0
#define SNS_REG_DEFAULT_ALP_DC_OFFSET                 0
#define SNS_REG_DEFAULT_ALP_NEAR_THRESHOLD            0
#define SNS_REG_DEFAULT_ALP_FAR_THRESHOLD             0
#define SNS_REG_DEFAULT_ALP_PRX_FACTOR                0
#define SNS_REG_DEFAULT_ALP_ALS_FACTOR                0
#define SNS_REG_DEFAULT_ALP_DS                        0

/**
 * Default accelerometer sampling rate for QMD algorithms
 */
#define SNS_REG_AMD_DEF_ACC_SAMP_RATE_HZ (10)
#define SNS_REG_VMD_DEF_ACC_SAMP_RATE_HZ (25)
#define SNS_REG_RMD_DEF_ACC_SAMP_RATE_HZ (30)

/**
 * Default Values for items in group SNS_REG_GROUP_SENSOR_TEST_V01
 */
#define SNS_REG_DEF_RAW_DATA_MODE      0
#define SNS_REG_DEF_TEST_EN_FLAGS      0

/**
 * Default Values for items in group SNS_REG_SAM_GROUP_AMD_V01
 */
#define SNS_REG_AMD_DEF_ACC_SAMP_RATE_HZ_Q16  \
        (FX_CONV_Q16(SNS_REG_AMD_DEF_ACC_SAMP_RATE_HZ, 0))

/* ADXL346 */
#define SNS_REG_AMD_DEF_INT_CFG_PARAM1_ADXL346   (FX_FLTTOFIX_Q16(0.1170723))
/* BMA150 */
#define SNS_REG_AMD_DEF_INT_CFG_PARAM1_BMA150   (FX_FLTTOFIX_Q16(0.06584465))
/* BMA250 */
#define SNS_REG_AMD_DEF_INT_CFG_PARAM1_BMA250   (FX_FLTTOFIX_Q16(0.145))
/* BMA2X2 */
#define SNS_REG_AMD_DEF_INT_CFG_PARAM1_BMA2X2   (FX_FLTTOFIX_Q16(0.0636))
#define SNS_REG_AMD_DEF_INT_CFG_PARAM2_BMA2X2   (FX_FLTTOFIX_Q16(0.8))
/* 8960: LIS3DH */
#define SNS_REG_AMD_DEF_INT_CFG_PARAM1_LIS3DH   (FX_FLTTOFIX_Q16(0.1))
/* LIS3DSH */
#define SNS_REG_AMD_DEF_INT_CFG_PARAM1_LIS3DSH  (FX_FLTTOFIX_Q16(0.0566))
/* 8960, 8974: MPU6050 */
#define SNS_REG_AMD_DEF_INT_CFG_PARAM1_MPU6050   (FX_FLTTOFIX_Q16(0.06))
/* 8974: MPU6500 */
#define SNS_REG_AMD_DEF_INT_CFG_PARAM1_MPU6500   (FX_FLTTOFIX_Q16(0.06))

/* Set the default to MPU6050 */
#define SNS_REG_AMD_DEF_INT_CFG_PARAM1 SNS_REG_AMD_DEF_INT_CFG_PARAM1_MPU6050
#define SNS_REG_AMD_DEF_INT_CFG_PARAM2 (FX_FLTTOFIX_Q16(0.5))

#define SNS_REG_AMD_DEF_SENSOR_REPORT_RATE     1
#define SNS_REG_AMD_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_AMD_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default Values for items in group SNS_REG_SAM_GROUP_VMD_V01
 */
#define SNS_REG_VMD_DEF_ACC_SAMP_RATE_HZ_Q16  \
        (FX_CONV_Q16(SNS_REG_VMD_DEF_ACC_SAMP_RATE_HZ, 0))

/* ADXL346 */
#define SNS_REG_VMD_DEF_INT_CFG_PARAM1_ADXL346   (FX_FLTTOFIX_Q16(0.1170723))
/* 8960: LIS3DH, MPU6050; 8660: BMA150 */
#define SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150   (FX_FLTTOFIX_Q16(0.06584465))
// TODO: Get value for LIS3DSH
/* LIS3DSH */
#define SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DSH SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150

#define SNS_REG_VMD_DEF_INT_CFG_PARAM1 SNS_REG_VMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050_BMA150
#define SNS_REG_VMD_DEF_INT_CFG_PARAM2 (FX_FLTTOFIX_Q16(1.0))

/**
 * Default Values for items in group SNS_REG_SAM_GROUP_RMD_V01
 */
#define SNS_REG_RMD_DEF_ACC_SAMP_RATE_HZ_Q16  \
        (FX_CONV_Q16(SNS_REG_RMD_DEF_ACC_SAMP_RATE_HZ, 0))

/* ADXL346 */
#define SNS_REG_RMD_DEF_INT_CFG_PARAM1_ADXL346   (FX_FLTTOFIX_Q16(300.0*G/1000.0))
#define SNS_REG_RMD_DEF_INT_CFG_PARAM3_ADXL346   (FX_FLTTOFIX_Q16(6.0*PI/180.0))
/* 8660: BMA150 */
#define SNS_REG_RMD_DEF_INT_CFG_PARAM1_BMA150   (FX_FLTTOFIX_Q16(300.0*G/1000.0))
#define SNS_REG_RMD_DEF_INT_CFG_PARAM3_BMA150   (FX_FLTTOFIX_Q16(11.5*PI/180.0))
/* BMA2X2 */
#define SNS_REG_RMD_DEF_INT_CFG_PARAM1_BMA2X2   (FX_FLTTOFIX_Q16(2.0))
#define SNS_REG_RMD_DEF_INT_CFG_PARAM3_BMA2X2   (FX_FLTTOFIX_Q16(6*PI/180.0))
#define SNS_REG_RMD_DEF_INT_CFG_PARAM2_BMA2X2   (FX_FLTTOFIX_Q16(0.27))
#define SNS_REG_RMD_DEF_INT_CFG_PARAM4_BMA2X2   (FX_FLTTOFIX_Q16(0.27))
/* 8960: LIS3DH, MPU6050 (CIC) */
#define SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050   (FX_FLTTOFIX_Q16(153.0*G/1000.0))
#define SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050   (FX_FLTTOFIX_Q16(5.7*PI/180.0))
/* LIS3DSH */
#define SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DSH  (FX_FLTTOFIX_Q16(1.7))
#define SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DSH  (FX_FLTTOFIX_Q16(5*PI/180.0))

#define SNS_REG_RMD_DEF_INT_CFG_PARAM1 SNS_REG_RMD_DEF_INT_CFG_PARAM1_LIS3DH_MPU6050
#define SNS_REG_RMD_DEF_INT_CFG_PARAM2 (FX_FLTTOFIX_Q16(0.2))
#define SNS_REG_RMD_DEF_INT_CFG_PARAM3 SNS_REG_RMD_DEF_INT_CFG_PARAM3_LIS3DH_MPU6050
#define SNS_REG_RMD_DEF_INT_CFG_PARAM4 (FX_FLTTOFIX_Q16(0.2))

#define SNS_REG_RMD_DEF_SENSOR_REPORT_RATE     5
#define SNS_REG_RMD_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_RMD_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default gyroscope calibration sampling rate
 */
#define SNS_REG_GYRO_CAL_DEF_SAMP_RATE_HZ (10)
#define SNS_REG_GYRO_CAL_DEF_REPORT_RATE  (0)
//#define SNS_REG_GYRO_CAL_DEF_REPORT_RATE  (1)

/**
 * Default Values for items in group SNS_REG_SCM_GROUP_GYRO_CAL_V01
 */
#define SNS_REG_GYRO_CAL_DEF_SAMP_RATE_HZ_Q16    (FX_CONV_Q16(SNS_REG_GYRO_CAL_DEF_SAMP_RATE_HZ, 0))
#define SNS_REG_GYRO_CAL_DEF_NUM_SAMPLES         (32)
#define SNS_REG_GYRO_CAL_DEF_VARIANCE_THOLD      (0x20000)
#define SNS_REG_GYRO_CAL_ENABLE_ALGO             (1)
#define SNS_REG_GYRO_CAL_DEF_REPORT_RATE_Q16     (FX_CONV_Q16(SNS_REG_GYRO_CAL_DEF_REPORT_RATE, 0))

/**
 * Default Values for items in group SNS_REG_SCM_GROUP_ACCEL_DYN_CAL_PARAMS_V01
 */
#define SNS_REG_ACC_DEF_DYN_X_BIAS 0
#define SNS_REG_ACC_DEF_DYN_Y_BIAS 0
#define SNS_REG_ACC_DEF_DYN_Z_BIAS 0
#define SNS_REG_ACC_DEF_DYN_X_SCALE 65536
#define SNS_REG_ACC_DEF_DYN_Y_SCALE 65536
#define SNS_REG_ACC_DEF_DYN_Z_SCALE 65536

#define SNS_REG_ACC_DEF_HEADER 0
#define SNS_REG_ACC_DEF_TEMP_BIN_SIZE 10

#define SNS_REG_ACC_DEF_VALID_FLAG_GROUP1 0
#define SNS_REG_ACC_DEF_TEMP_MIN_GROUP1 0
#define SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP1 0
#define SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP1 0
#define SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP1 0
#define SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP1 65536
#define SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP1 65536
#define SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP1 65536

#define SNS_REG_ACC_DEF_VALID_FLAG_GROUP2 0
#define SNS_REG_ACC_DEF_TEMP_MIN_GROUP2 0
#define SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP2 0
#define SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP2 0
#define SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP2 0
#define SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP2 65536
#define SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP2 65536
#define SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP2 65536

#define SNS_REG_ACC_DEF_VALID_FLAG_GROUP3 0
#define SNS_REG_ACC_DEF_TEMP_MIN_GROUP3 0
#define SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP3 0
#define SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP3 0
#define SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP3 0
#define SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP3 65536
#define SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP3 65536
#define SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP3 65536

#define SNS_REG_ACC_DEF_VALID_FLAG_GROUP4 0
#define SNS_REG_ACC_DEF_TEMP_MIN_GROUP4 0
#define SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP4 0
#define SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP4 0
#define SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP4 0
#define SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP4 65536
#define SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP4 65536
#define SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP4 65536

#define SNS_REG_ACC_DEF_VALID_FLAG_GROUP5 0
#define SNS_REG_ACC_DEF_TEMP_MIN_GROUP5 0
#define SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP5 0
#define SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP5 0
#define SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP5 0
#define SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP5 65536
#define SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP5 65536
#define SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP5 65536

#define SNS_REG_ACC_DEF_VALID_FLAG_GROUP6 0
#define SNS_REG_ACC_DEF_TEMP_MIN_GROUP6 0
#define SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP6 0
#define SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP6 0
#define SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP6 0
#define SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP6 65536
#define SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP6 65536
#define SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP6 65536

#define SNS_REG_ACC_DEF_VALID_FLAG_GROUP7 0
#define SNS_REG_ACC_DEF_TEMP_MIN_GROUP7 0
#define SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP7 0
#define SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP7 0
#define SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP7 0
#define SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP7 65536
#define SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP7 65536
#define SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP7 65536

#define SNS_REG_ACC_DEF_VALID_FLAG_GROUP8 0
#define SNS_REG_ACC_DEF_TEMP_MIN_GROUP8 0
#define SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP8 0
#define SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP8 0
#define SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP8 0
#define SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP8 65536
#define SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP8 65536
#define SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP8 65536
/**
 * Backwards Compatible Default Values for items in group SNS_REG_SCM_GROUP_ACCEL_DYN_CAL_PARAMS_V01
 */
#define SNS_REG_DEFAULT_ACC_X_BIAS  0
#define SNS_REG_DEFAULT_ACC_Y_BIAS  0
#define SNS_REG_DEFAULT_ACC_Z_BIAS  0
#define SNS_REG_DEFAULT_ACC_X_SCALE 65536
#define SNS_REG_DEFAULT_ACC_Y_SCALE 65536
#define SNS_REG_DEFAULT_ACC_Z_SCALE 65536



/**
 * Default Values for items in group SNS_REG_SCM_GROUP_GYRO_DYN_CAL_PARAMS_V01
 */
#define SNS_REG_GYRO_DEF_DYN_X_BIAS 0
#define SNS_REG_GYRO_DEF_DYN_Y_BIAS 0
#define SNS_REG_GYRO_DEF_DYN_Z_BIAS 0
#define SNS_REG_GYRO_DEF_DYN_X_SCALE 65536
#define SNS_REG_GYRO_DEF_DYN_Y_SCALE 65536
#define SNS_REG_GYRO_DEF_DYN_Z_SCALE 65536

/**
 * Default Values for items in group SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V01
 */
#define SNS_REG_GYRO_DEF_X_BIAS 0
#define SNS_REG_GYRO_DEF_Y_BIAS 0
#define SNS_REG_GYRO_DEF_Z_BIAS 0
#define SNS_REG_GYRO_DEF_X_SCALE 65536
#define SNS_REG_GYRO_DEF_Y_SCALE 65536
#define SNS_REG_GYRO_DEF_Z_SCALE 65536

/**
 * Default Values for Sensor Fusion algos
 */
#define SNS_SAM_FUSION_DEFAULT_REPORT_RATE_HZ (1)
#define SNS_SAM_FUSION_DEFAULT_REPORT_RATE_HZ_Q16 (FX_CONV_Q16(SNS_SAM_FUSION_DEFAULT_REPORT_RATE_HZ, 0))
#define SNS_SAM_FUSION_MIN_SAMPLE_RATE_HZ (5)
#define SNS_SAM_FUSION_MIN_SAMPLE_RATE_HZ_Q16 (FX_CONV_Q16(SNS_SAM_FUSION_MIN_SAMPLE_RATE_HZ, 0))
#define SNS_SAM_FUSION_MAX_SAMPLE_RATE_HZ (200)
#define SNS_SAM_FUSION_MAX_SAMPLE_RATE_HZ_Q16 (FX_CONV_Q16(SNS_SAM_FUSION_MAX_SAMPLE_RATE_HZ, 0))

/**
 * Default Values for items in group SNS_REG_SAM_GROUP_FMV_PARAMS_V02
 */
#define SNS_REG_DEF_FMV_TC_ACCURACY_0       3000
#define SNS_REG_DEF_FMV_TC_ACCURACY_1       1500
#define SNS_REG_DEF_FMV_TC_ACCURACY_2       1500
#define SNS_REG_DEF_FMV_TC_ACCURACY_3       3000
#define SNS_REG_DEF_FMV_GYRO_GAP_THRESH     500
#define SNS_REG_DEF_FMV_MAG_GAP_FACTOR      1.0f
#define SNS_REG_DEF_FMV_ROT_FOR_ZUPT        0.001f
#define SNS_REG_DEF_FMV_MAX_MAG_INNOV       9.0f
#define SNS_REG_FMV_DEF_SENSOR_REPORT_RATE  5
#define SNS_REG_FMV_DEF_SENSOR_REPORT_RATE_Q16  (FX_CONV_Q16(SNS_REG_FMV_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default Values for items in group SNS_REG_SAM_GROUP_GRAVITY_VECTOR_PARAMS_V02
 */
#define SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM1       4000
#define SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM2       600
#define SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM3       600
#define SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM4       4000
#define SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM5       0.16f
#define SNS_REG_GRAVITY_VECTOR_DEF_SENSOR_REPORT_RATE   5
#define SNS_REG_GRAVITY_VECTOR_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_GRAVITY_VECTOR_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default Values for items in group SNS_REG_SAM_GROUP_ORIENTATION_PARAMS_V02
 */
#define SNS_REG_DEF_ORIENTATION_INT_CFG_PARAM1 300
#define SNS_REG_DEF_MAG_CAL_LAT_NUM_SAMPLES    8
#define SNS_REG_DEF_FUSION_MIN_SAMP_RATE_HZ    10
#define SNS_REG_ORIENTATION_DEF_SENSOR_REPORT_RATE 5
#define SNS_REG_ORIENTATION_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_ORIENTATION_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default values for items in group SNS_REG_DRIVER_GROUP_ACCEL_V02
 */
#define SNS_REG_DRIVER_ACCEL_X_ORIENT       1
#define SNS_REG_DRIVER_ACCEL_Y_ORIENT       2
#define SNS_REG_DRIVER_ACCEL_Z_ORIENT       3

/**
 * Default values for items in group SNS_REG_DRIVER_GROUP_ACCEL_2_V02
 */
#define SNS_REG_DRIVER_ACCEL2_X_ORIENT      1
#define SNS_REG_DRIVER_ACCEL2_Y_ORIENT      2
#define SNS_REG_DRIVER_ACCEL2_Z_ORIENT      3

/**
 * Default values for items in group SNS_REG_DRIVER_GROUP_GYRO_V02
 */
#define SNS_REG_DRIVER_GYRO_X_ORIENT        1
#define SNS_REG_DRIVER_GYRO_Y_ORIENT        2
#define SNS_REG_DRIVER_GYRO_Z_ORIENT        3

/**
 * Default values for items in group SNS_REG_DRIVER_GROUP_MAG_V02
 */
#define SNS_REG_DRIVER_MAG_X_ORIENT         1
#define SNS_REG_DRIVER_MAG_Y_ORIENT         2
#define SNS_REG_DRIVER_MAG_Z_ORIENT         3

/**
 * Default values for items in group SNS_REG_DRIVER_GROUP_GAME_RV_V02
 */
#define SNS_REG_DRIVER_GAME_RV_X_ORIENT     1
#define SNS_REG_DRIVER_GAME_RV_Y_ORIENT     2
#define SNS_REG_DRIVER_GAME_RV_Z_ORIENT     3

/**
 * Default values for items in group SNS_REG_SAM_GROUP_GYRO_AMD
 */
#define SNS_REG_DEF_GYRO_AMD_INT_CFG_PARAM1 0.000061f
#define SNS_REG_DEF_GYRO_AMD_INT_CFG_PARAM2 0.25f
#define SNS_REG_DEF_GYRO_AMD_INT_CFG_PARAM3 0.0149f

/**
 * Default values for items in group SNS_REG_SAM_GROUP_GYRO_TAP
 */
#define SNS_REG_GYRO_TAP_DEF_VERSION        0x00010001
#define SNS_REG_GYRO_INVALID_SCENARIO       0xFFFFFFFF

/** Default values for items in group SNS_REG_SETTINGS */
#define SNS_REG_SETTINGS_MG_WORD            0x736e732e72656700ULL
#define SNS_REG_SETTINGS_VERSION_MAJOR      SNS_REG2_SVC_V02_IDL_MAJOR_VERS
#define SNS_REG_SETTINGS_VERSION_MINOR      SNS_REG2_SVC_V02_IDL_MINOR_VERS


/**
 * Default gyroscope calibration sampling rate
 */
#define SNS_REG_ACCEL_CAL_DEF_SAMP_RATE_HZ (20)

/**
 * Default Values for items in group SNS_REG_SCM_GROUP_GYRO_CAL_V01
 */
#define SNS_REG_ACCEL_CAL_DEF_SAMP_RATE_HZ_Q16    (FX_CONV_Q16(SNS_REG_ACCEL_CAL_DEF_SAMP_RATE_HZ, 0))
#define SNS_REG_ACCEL_CAL_ENABLE_ALGO               (0)

/**
 * Default values for items in group SNS_REG_SAM_GROUP_PED_V02
 */
#define SNS_REG_PED_DEF_STEP_THRESHOLD        10.1f
#define SNS_REG_PED_DEF_SWING_THRESHOLD       10.5f
#define SNS_REG_PED_DEF_STEP_PROB_THRESHOLD   0.49f
#define SNS_REG_PED_DEF_SENSOR_REPORT_RATE    1
#define SNS_REG_PED_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_PED_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default values for items in group SNS_REG_SAM_GROUP_GYRO_AMD
 */
#define SNS_REG_DEF_PAM_INT_CFG_PARAM1 20
#define SNS_REG_DEF_PAM_INT_CFG_PARAM2 2
#define SNS_REG_DEF_PAM_INT_CFG_PARAM3 1310720
#define SNS_REG_DEF_PAM_INT_CFG_PARAM4 20

/**
 * Default values for items in group SNS_REG_GROUP_SSI_SMGR
 */
#define SNS_REG_DEF_ITEM_SSI_SMGR_MAJ_VER_NO 1
#define SNS_REG_DEF_ITEM_SSI_SMGR_MIN_VER_NO 1

/**
 * Default values for items in group SNS_REG_GROUP_SSI_GPIO_CFG
 */
#define SNS_REG_DEF_ITEM_SSI_GPIO_CFG_MAJ_VER_NO                  1
#define SNS_REG_DEF_ITEM_SSI_GPIO_CFG_MIN_VER_NO                  1
#define SNS_REG_DEF_ITEM_SSI_GPIO_CFG_GPIO                        0xFFFF

/**
 * Default values for items in group SNS_REG_SAM_GROUP_BASIC_GESTURES
 */
#define SNS_REG_BASIC_GESTURES_DEF_SENSOR_REPORT_RATE     5
#define SNS_REG_BASIC_GESTURES_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_BASIC_GESTURES_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default values for items in group SNS_REG_SAM_GROUP_FACING
 */
#define SNS_REG_FACING_DEF_SENSOR_REPORT_RATE     5
#define SNS_REG_FACING_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_FACING_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default values for items in group SNS_REG_SAM_GROUP_QUATERNION
 */
#define SNS_REG_QUATERNION_DEF_SENSOR_REPORT_RATE     5
#define SNS_REG_QUATERNION_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_QUATERNION_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default values for items in group SNS_REG_SAM_GROUP_ROTATION_VECTOR
 */
#define SNS_REG_ROTATION_VECTOR_DEF_SENSOR_REPORT_RATE     5
#define SNS_REG_ROTATION_VECTOR_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_ROTATION_VECTOR_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default values for items in group
 * SNS_REG_SAM_GROUP_DISTANCE_BOUND
 */
#define SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_UNKNOWN      30.0f
#define SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_STATIONARY   0.0f
#define SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_INMOTION     30.0f
#define SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_FIDDLE       0.0f
#define SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_PEDESTRIAN   5.0f
#define SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_VEHICLE      30.0f
#define SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_WALK         2.0f
#define SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_RUN          5.0f
#define SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_BIKE         15.0f

/**
 * Default values for items in group SNS_REG_SAM_GROUP_CMC
 */
#define SNS_REG_CMC_DEF_SENSOR_REPORT_RATE     1
#define SNS_REG_CMC_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_CMC_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default values for items in group SNS_REG_SAM_GROUP_DPC
 */
#define SNS_REG_DPC_DEF_SENSOR_REPORT_RATE     2
#define SNS_REG_DPC_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_DPC_DEF_SENSOR_REPORT_RATE, 0))

/**
 * Default values for items in group SNS_REG_GROUP_QMAG_CAL_ALGO
 */
#define SNS_REG_GROUP_QMAG_CAL_DEF_SAMPLE_RATE              50 /* Default Sample rate in Hz */
#define SNS_REG_GROUP_QMAG_CAL_DEF_REPORT_RATE              01 /* Default Report rate in Hz */
#define SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_VERSION             01
#define SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_ENABLE_ALGO         0
#define SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_ENABLE_SI_CAL       0
#define SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_ENABLE_HI_CAL       1
#define SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_SAMPLE_RATE_Q16     (FX_CONV_Q16(SNS_REG_GROUP_QMAG_CAL_DEF_SAMPLE_RATE, 0))
#define SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_REPORT_RATE_Q16     (FX_CONV_Q16(SNS_REG_GROUP_QMAG_CAL_DEF_REPORT_RATE, 0))
#define SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_ENABLE_GYRO_ASSIST  0

/**
 * Default values for items in group SNS_REG_GROUP_MAG_DYN_CAL_PARAMS
 */
#define SNS_REG_MAG_DYN_CAL_DEF_COMP_MAT_DIAG_ELEMENT         1
#define SNS_REG_MAG_DYN_CAL_DEF_COMP_MAT_NONDIAG_ELEMENT      0
#define SNS_REG_MAG_DYN_CAL_DEF_VERSION                       1
#define SNS_REG_MAG_DYN_CAL_DEF_BIAS_VALID                    0
#define SNS_REG_MAG_DYN_CAL_DEF_CAL_MAT_VALID                 0
#define SNS_REG_MAG_DYN_CAL_DEF_COMP_MAT_DIAG_ELEMENT_Q16      (FX_CONV_Q16(SNS_REG_MAG_DYN_CAL_DEF_COMP_MAT_DIAG_ELEMENT, 0))
#define SNS_REG_MAG_DYN_CAL_DEF_COMP_MAT_NONDIAG_ELEMENT_Q16      (FX_CONV_Q16(SNS_REG_MAG_DYN_CAL_DEF_COMP_MAT_NONDIAG_ELEMENT, 0))

/**
 * Default values for items in group SNS_REG_SAM_GROUP_GAME_ROTATION_VECTOR
 */
#define SNS_REG_GAME_ROT_VEC_DEF_SENSOR_REPORT_RATE     5
#define SNS_REG_GAME_ROT_VEC_DEF_SENSOR_REPORT_RATE_Q16 (FX_CONV_Q16(SNS_REG_GAME_ROT_VEC_DEF_SENSOR_REPORT_RATE, 0))

/**  Default values for items in group
 *   SNS_REG_SCM_GROUP_QMAG_CAL_STATE_V02 (Group Id: 2971) */
#define SNS_REG_QMAG_CAL_STATE_DEF_VERSION 1
#define SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_X_HI 0
#define SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_Y_HI 0
#define SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_Z_HI 0
#define SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_ACCURACY_HI 0
#define SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_TIME 0
#define SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_VALID 0

/**
 * Default values for items in group SNS_REG_SAM_GROUP_SMD
 */
#define SNS_REG_SMD_DEF_VERSION                  1
#define SNS_REG_SMD_DEF_SENSOR_REPORT_RATE       1
#define SNS_REG_SMD_DEF_SENSOR_REPORT_RATE_HZ_Q16  (FX_CONV_Q16(SNS_REG_SMD_DEF_SENSOR_REPORT_RATE, 0))
#define SNS_REG_SMD_DEF_SAMPLE_RATE              10
#define SNS_REG_SMD_DEF_SAMPLE_RATE_HZ_Q16       (FX_CONV_Q16(SNS_REG_SMD_DEF_SAMPLE_RATE, 0))
#define SNS_REG_SMD_DEF_VAR_DECISION_LATENCY     1      // fixed latency (0), variable latency (1)
#define SNS_REG_SMD_DEF_MAX_LATENCY              10     // max latency if fixed, in sec
#define SNS_REG_SMD_DEF_SELF_TRANS_PROB_SM       (FX_FLTTOFIX_Q16(0.9f))   // self transition probability for sig motion
#define SNS_REG_SMD_DEF_ACCEL_WINDOW_TIME        2      // accel window time in secs
#define SNS_REG_SMD_DEF_DETECT_THRESH            (sqrtFxQ16(FX_FLTTOFIX_Q16(0.025))) // sqrt(0.05/2), Q16
#define SNS_REG_SMD_DEF_STEP_COUNT_THRESH        5      // pedometer step count threshold
#define SNS_REG_SMD_DEF_STEP_WINDOW_TIME         3      // pedometer window
#define SNS_REG_SMD_DEF_EIGEN_THRESH             (FX_CONV_Q16(6, 0))  // eigen value threshold, Q16
#define SNS_REG_SMD_DEF_ACC_NORM_STD_DEV_THRESH  (FX_CONV_Q16(2, 0))  // accel norm std dev threshold, Q16
#define SNS_REG_SMD_DEF_DELTA_RESET_TIME         0
#define SNS_REG_SMD_DEF_LONG_ACCEL_WINDOW_TIME   5

/**
 * Default values for items in group SNS_REG_GROUP_MAG_FAC_CAL_PARAMS
 */
#define SNS_REG_MAG_FAC_CAL_DEF_COMP_MAT_DIAG_ELEMENT      1
#define SNS_REG_MAG_FAC_CAL_DEF_COMP_MAT_NONDIAG_ELEMENT   0
#define SNS_REG_MAG_FAC_CAL_DEF_VERSION                    1
#define SNS_REG_MAG_FAC_CAL_DEF_BIAS_VALID                 0
#define SNS_REG_MAG_FAC_CAL_DEF_CAL_MAT_VALID              0
#define SNS_REG_MAG_FAC_CAL_DEF_COMP_MAT_DIAG_ELEMENT_Q16 (FX_CONV_Q16(SNS_REG_MAG_FAC_CAL_DEF_COMP_MAT_DIAG_ELEMENT, 0))
#define SNS_REG_MAG_FAC_CAL_DEF_COMP_MAT_NONDIAG_ELEMENT_Q16 (FX_CONV_Q16(SNS_REG_MAG_FAC_CAL_DEF_COMP_MAT_NONDIAG_ELEMENT, 0))

/**
 * Default Values for items in group
 * SNS_REG_DRIVER_GROUP_IR_GESTURE_V02
 */
#define SNS_REG_DEFAULT_IR_GESTURE_DS                        0

/**
 * Default Values for items in group
 * SNS_REG_DRIVER_GROUP_RGB_V02 and
 * SNS_REG_DRIVER_GROUP_RGB_2_V02
 */
#define SNS_REG_DEFAULT_RGB_FACTOR                         100

/**
 * Default values for items in group SNS_REG_SAM_GROUP_TILT_DETECTOR
 */
#define SNS_REG_TILT_DETECTOR_DEF_SENSOR_REPORT_RATE       1.0f  // Accel report rate, in Hz
#define SNS_REG_TILT_DETECTOR_DEF_SENSOR_REPORT_RATE_Q16   FX_FLTTOFIX_Q16(SNS_REG_TILT_DETECTOR_DEF_SENSOR_REPORT_RATE)
#define SNS_REG_TILT_DETECTOR_DEF_SAMPLE_RATE              10    // Accel sample rate, in Hz
#define SNS_REG_TILT_DETECTOR_DEF_SAMPLE_RATE_Q16          FX_CONV_Q16(SNS_REG_TILT_DETECTOR_DEF_SAMPLE_RATE,0);
#define SNS_REG_TILT_DETECTOR_DEF_INIT_ACCEL_WINDOW_TIME   1.0f  // Initial gravity averaging time-window, in sec
#define SNS_REG_TILT_DETECTOR_DEF_ACCEL_WINDOW_TIME        2.0f  // Gravity averaging time-window, in sec
#define SNS_REG_TILT_DETECTOR_DEF_TILT_ANGLE_THRESH_DEG    35    // Tilt angle threshold, in degrees
#define SNS_REG_TILT_DETECTOR_DEF_TILT_ANGLE_THRESH_RAD    SNS_REG_TILT_DETECTOR_DEF_TILT_ANGLE_THRESH_DEG*PI/180.0    // Tilt angle threshold, in radians

/*============================================================================
  Type Declarations
  ============================================================================*/

#ifdef _WIN32
#pragma pack(push,1)
#endif

/**
 *  Registry Data Group for ambient light and proximity sensor
 */
typedef PACK(struct)
{
  uint8_t  visible_light_trans_ratio; /* visible light transmission ratio of the glass/pipe*/
  uint8_t  ir_light_trans_ratio;      /* IR transmission ratio of light glass/pipe */
  uint16_t dc_offset;                 /* DC offset in ADC count */
  uint16_t near_threshold;            /* in ADC count */
  uint16_t far_threshold;             /* in ADC count */
  uint16_t  prx_factor;
  uint16_t  als_factor;
  uint32_t  ds1;                      /* Device/Driver Specific data fields */
  uint32_t  ds2;
  uint32_t  ds3;
  uint32_t  ds4;
  uint32_t  ds5;
  uint32_t  ds6;
  uint32_t  ds7;
  uint32_t  ds8;
  uint32_t  ds9;
  uint32_t  ds10;
  uint32_t  ds11;
  uint32_t  ds12;
  uint32_t  ds13;
  uint32_t  ds14;
  uint32_t  ds15;
  uint32_t  ds16;
  uint32_t  ds17;
  uint32_t  ds18;
  uint32_t  ds19;
  uint32_t  ds20;
  uint32_t  ds21;
  uint32_t  ds22;
  uint32_t  ds23;
  uint32_t  ds24;
  uint32_t  ds25;
  uint32_t  ds26;
  uint32_t  ds27;
  uint32_t  ds28;
  uint32_t  ds29;
} sns_reg_alp_data_group_s;

/**
 *  Registry Data Group for accelerometer
 */
typedef PACK(struct)
{
  int32_t accelx_bias; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale; /* accel z axis scale factor in Q16.*/
} sns_reg_acc_data_group_s;

/**
 *  Registry Data Group for accelerometer driver
 */
typedef PACK(struct)
{
    // Axis orientation refers to the device's orientation with respect to the
    // phone. Values must be in the range [-3, 3] excluding zero, where the
    // number indicates a mapping to a particular axis (1=x, 2=y, 3=z) and the
    // sign indicates direction. E.g., to map the device's x axis to the phone's
    // -y axis, set accelx_orient = -2.

    int8_t accelx_orient;
    int8_t accely_orient;
    int8_t accelz_orient;
} sns_reg_acc_driver_group_s;

/**
 *  Registry Data Group for gyroscope
 */
typedef PACK(struct)
{
  int32_t gyrox_bias; /* gyro x axis bias in Q16. This will be in the sensor unit*/
  int32_t gyroy_bias; /* gyro y axis bias in Q16. This will be in the sensor unit*/
  int32_t gyroz_bias; /* gyro z axis bias in Q16. This will be in the sensor unit*/
  uint32_t gyrox_scale; /* gyro x axis scale factor in Q16.*/
  uint32_t gyroy_scale; /* gyro y axis scale factor in Q16.*/
  uint32_t gyroz_scale; /* gyro z axis scale factor in Q16.*/
} sns_reg_gyro_data_group_s;

/**
 *  Registry Data Group for gyroscope driver
 */
typedef PACK(struct)
{
    // Axis orientation refers to the device's orientation with respect to the
    // phone. Values must be in the range [-3, 3] excluding zero, where the
    // number indicates a mapping to a particular axis (1=x, 2=y, 3=z) and the
    // sign indicates direction. E.g., to map the device's x axis to the phone's
    // -y axis, set accelx_orient = -2.

    int8_t gyrox_orient;
    int8_t gyroy_orient;
    int8_t gyroz_orient;
} sns_reg_gyro_driver_group_s;

/**
 *  Registry Data Group for magnetometer driver
 */
typedef PACK(struct)
{
    // Axis orientation refers to the device's orientation with respect to the
    // phone. Values must be in the range [-3, 3] excluding zero, where the
    // number indicates a mapping to a particular axis (1=x, 2=y, 3=z) and the
    // sign indicates direction. E.g., to map the device's x axis to the phone's
    // -y axis, set accelx_orient = -2.

    int8_t magx_orient;
    int8_t magy_orient;
    int8_t magz_orient;
} sns_reg_mag_driver_group_s;


/**
 *  Registry Data Group for Absolute Motion Detector algorithm
 */
typedef PACK(struct)
{
   int32_t def_acc_samp_rate;/*Default accelerometer sampling rate Hz (Q16 format)*/
   int32_t int_cfg_param1;   /*Absolute motion detector internal config parameter 1*/
   int32_t int_cfg_param2;   /*Absolute motion detector internal config parameter 2*/
   int32_t sensor_report_rate;/* Default sensor report rate in Hz (Q16) */
} sns_reg_amd_data_group_s;

/**
 *  Registry Data Group for Vehicle Motion Detector algorithm
 */
typedef PACK(struct)
{
   int32_t def_acc_samp_rate;/*Default accelerometer sampling rate Hz (Q16 format)*/
   int32_t int_cfg_param1;   /*Vehicle motion detector internal config parameter 1*/
   int32_t int_cfg_param2;   /*Vehicle motion detector internal config parameter 2*/
} sns_reg_vmd_data_group_s;

/**
 *  Registry Data Group for Relative Motion Detector algorithm
 */
typedef PACK(struct)
{
   int32_t def_acc_samp_rate;/*Default accelerometer sampling rate Hz (Q16 format)*/
   int32_t int_cfg_param1;   /*Relative motion detector internal config parameter 1*/
   int32_t int_cfg_param2;   /*Relative motion detector internal config parameter 2*/
   int32_t int_cfg_param3;   /*Relative motion detector internal config parameter 3*/
   int32_t int_cfg_param4;   /*Relative motion detector internal config parameter 4*/
   int32_t sensor_report_rate;/* Default sensor report rate in Hz (Q16) */
} sns_reg_rmd_data_group_s;

/**
 *  Registry Data Group for gyro calibration
 */
typedef PACK(struct)
{
 int64_t def_gyro_var_thold;              /*Stationary detect variance threshold*/
 int32_t def_gyro_sample_rate;            /*Default gyro sampling rate Hz (Q16 format)*/
 int32_t def_gyro_num_samples;            /*Number of samples*/
 uint8_t def_gyro_cal_algo_state;   /*Toggle algorithm instantiation: 1 for enable, 0 for disable*/
 int32_t def_sensor_report_rate;          /*Sensor report rate in Hz, Q16*/
} sns_reg_gyro_cal_data_group_s;

/**
 *  Registry Data Group for sensor test
 */
typedef PACK(struct)
{
 uint8_t raw_data_mode;               /*Whether raw data mode is enabled*/
 int32_t test_en_flags;               /*Meaningful by bit fields*/
} sns_reg_sensor_test_data_group_s;

/**
 *  Registry Data Group for filtered magnetic vector
 */
#define FMV_MAX_TCS_IN_REGISTRY 4
typedef PACK(struct)
{
  uint32_t accuracies[FMV_MAX_TCS_IN_REGISTRY];
  uint32_t  gyro_gap_thresh;
  float  mag_gap_factor;
  float  gyro_thresh_for_zupt;
  float  max_mag_innovation;
  int32_t sensor_report_rate;/* Default sensor report rate in Hz (Q16) */
} sns_reg_fmv_data_group_s;

/**
 *  Registry Data Group for gravity vector
 */
#define GRAVITY_MAX_CONFIG_ARR1 4
typedef PACK(struct)
{
  uint32_t int_cfg_arr1[GRAVITY_MAX_CONFIG_ARR1];
  float  int_cfg_param1;
  int32_t sensor_report_rate;/* Default sensor report rate in Hz (Q16) */
} sns_reg_gravity_data_group_s;

/**
 *  Registry Data Group for orientation
 */
typedef PACK(struct)
{
  uint32_t int_cfg_param1;
  uint8_t mag_cal_lat_num_samps;
  uint8_t fusion_min_samp_rate_hz;
  int32_t sensor_report_rate;/* Default sensor report rate in Hz (Q16) */
} sns_reg_orientation_data_group_s;

/**
 *  Registry Data Group for Gyro based AMD
 */
typedef PACK(struct)
{
   float int_cfg_param1;
   float int_cfg_param2;
   float int_cfg_param3;
} sns_reg_gyro_amd_data_group_s;

/**
 *  Registry Data Group for Gyro Tap
 */
typedef PACK(struct)
{
  int32_t gyro_tap_scenario;
  int32_t gyro_tap_flags;
  int32_t gyro_tap_time_secs_0;
  int32_t gyro_tap_time_secs_1;
  int32_t gyro_tap_time_secs_2;
  int32_t gyro_tap_time_secs_3;
  int32_t gyro_tap_time_secs_4;
  int32_t gyro_tap_ratio_0;
  int32_t gyro_tap_ratio_1;
  int32_t gyro_tap_thresh_0;
  int32_t gyro_tap_thresh_1;
  int32_t gyro_tap_thresh_2;
  int32_t gyro_tap_thresh_3;
  int32_t gyro_tap_thresh_4;
} sns_reg_gyro_tap_scenario_s;

#define SNS_REG_MAX_GYRO_TAP_SCENARIOS 3
typedef PACK(struct)
{
  int32_t gyro_tap_version;
  sns_reg_gyro_tap_scenario_s scn[SNS_REG_MAX_GYRO_TAP_SCENARIOS];
} sns_reg_gyro_tap_group_s;

#define SNS_REG_SETTINGS_NUM_FILES 3
#define SNS_REG_SETTINGS_FILE_NAME_LEN 8

typedef PACK(struct)
{
  char     name[SNS_REG_SETTINGS_FILE_NAME_LEN];
  uint32_t version;
} sns_reg_settings_conf_info_s;

typedef PACK(struct)
{
  uint64_t                     magic_word;
  uint32_t                     version_major;
  uint32_t                     version_minor;
  sns_reg_settings_conf_info_s conf[SNS_REG_SETTINGS_NUM_FILES];
} sns_reg_settings_group_s;

/**
 *  Registry Data Group for accel calibration algorithm
 */
typedef PACK(struct)
{
 int32_t def_accel_sample_rate;           /*Default accel sampling rate Hz (Q16 format)*/
 uint8_t def_accel_cal_algo_state;        /*Toggle algorithm instantiation: 1 for enable, 0 for disable*/
} sns_reg_accel_cal_algo_data_group_s;

/**
 *  Registry Data Group for accelerometer dynamic calibration
 *  parameters
 */
typedef PACK(struct)
{
  int32_t accelx_bias; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale; /* accel z axis scale factor in Q16.*/

  uint8_t header; /* version number of the accel dyn cal registry group*/
  uint8_t temp_bin_size; /* the temperature bin width of each sub group in deg */

  uint8_t valid_flag_group1; /* 0: do not use calibration values in group, 1: cal paras in group can be used*/
  int8_t  temp_min_group1; /* the minimum temperature associated with the subgroup*/
  int32_t accelx_bias_group1; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias_group1; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias_group1; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale_group1; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale_group1; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale_group1; /* accel z axis scale factor in Q16.*/

  uint8_t valid_flag_group2; /* 0: do not use calibration values in group, 1: cal paras in group can be used*/
  int8_t  temp_min_group2; /* the minimum temperature associated with the subgroup*/
  int32_t accelx_bias_group2; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias_group2; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias_group2; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale_group2; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale_group2; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale_group2; /* accel z axis scale factor in Q16.*/

  uint8_t valid_flag_group3; /* 0: do not use calibration values in group, 1: cal paras in group can be used*/
  int8_t  temp_min_group3; /* the minimum temperature associated with the subgroup*/
  int32_t accelx_bias_group3; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias_group3; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias_group3; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale_group3; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale_group3; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale_group3; /* accel z axis scale factor in Q16.*/

  uint8_t valid_flag_group4; /* 0: do not use calibration values in group, 1: cal paras in group can be used*/
  int8_t  temp_min_group4; /* the minimum temperature associated with the subgroup*/
  int32_t accelx_bias_group4; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias_group4; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias_group4; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale_group4; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale_group4; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale_group4; /* accel z axis scale factor in Q16.*/

  uint8_t valid_flag_group5; /* 0: do not use calibration values in group, 1: cal paras in group can be used*/
  int8_t  temp_min_group5; /* the minimum temperature associated with the subgroup*/
  int32_t accelx_bias_group5; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias_group5; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias_group5; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale_group5; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale_group5; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale_group5; /* accel z axis scale factor in Q16.*/

  uint8_t valid_flag_group6; /* 0: do not use calibration values in group, 1: cal paras in group can be used*/
  int8_t  temp_min_group6; /* the minimum temperature associated with the subgroup*/
  int32_t accelx_bias_group6; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias_group6; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias_group6; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale_group6; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale_group6; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale_group6; /* accel z axis scale factor in Q16.*/

  uint8_t valid_flag_group7; /* 0: do not use calibration values in group, 1: cal paras in group can be used*/
  int8_t  temp_min_group7; /* the minimum temperature associated with the subgroup*/
  int32_t accelx_bias_group7; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias_group7; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias_group7; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale_group7; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale_group7; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale_group7; /* accel z axis scale factor in Q16.*/

  uint8_t valid_flag_group8; /* 0: do not use calibration values in group, 1: cal paras in group can be used*/
  int8_t  temp_min_group8; /* the minimum temperature associated with the subgroup*/
  int32_t accelx_bias_group8; /* accel x axis bias in Q16. This will be in the sensor unit*/
  int32_t accely_bias_group8; /* accel y axis bias in Q16. This will be in the sensor unit*/
  int32_t accelz_bias_group8; /* accel z axis bias in Q16. This will be in the sensor unit*/
  uint32_t accelx_scale_group8; /* accel x axis scale factor in Q16.*/
  uint32_t accely_scale_group8; /* accel y axis scale factor in Q16.*/
  uint32_t accelz_scale_group8; /* accel z axis scale factor in Q16.*/
} sns_reg_accel_dyn_cal_params_data_group_s;

/**
 *  Registry Data Group for Pedometer algorithm
 */
typedef PACK(struct)
{
   float step_threshold;      /* Minimum acceleration magnitude to be considered a step in m/s/s */
   float swing_threshold;     /* Minimum acceleration magnitude for swing detection in m/s/s */
   float step_prob_threshold; /* Probability threshold used for step detection */
   int32_t sensor_report_rate;/* Default sensor report rate in Hz (Q16) */
} sns_reg_ped_data_group_s;

/**
 *  Registry Data Group for PAM
 */
typedef PACK(struct)
{
   uint32_t int_cfg_param1;
   uint32_t int_cfg_param2;
   uint32_t int_cfg_param3;
   uint8_t  int_cfg_param4;
} sns_reg_pam_data_group_s;

/**
 *  Registry Data Group for SSI SMGR configuration parameters
 */

typedef PACK(struct)
{
  uint8_t  drvuuid[16];
  int32_t  off_to_idle;
  int32_t  idle_to_ready;
  int16_t  i2c_bus;
  uint16_t reg_group_id;
  uint16_t cal_pri_group_id;
  uint16_t gpio1;
  uint16_t gpio2;
  uint8_t  sensor_id;
  uint8_t  i2c_address;
  int8_t   data_type1;
  int8_t   data_type2;
  int8_t   related_sensor_index;
  int8_t   sensitivity_default;
  uint8_t  flags;
  int8_t   device_select; /* Added in struct revision 1.1 */
  int16_t  reserved2;
  int32_t  reserved3;
} sns_reg_ssi_smgr_cfg_group_drvcfg_s;

#define SNS_REG_SSI_NUM_SMGR_CFG          3
#define SNS_REG_SSI_SMGR_CFG_NUM_SENSORS  5
typedef PACK(struct)
{
  uint8_t  maj_ver_no;
  uint8_t  min_ver_no;
  uint16_t reserved1;
  uint32_t reserved2;
  uint32_t reserved3;
  uint32_t reserved4;
  sns_reg_ssi_smgr_cfg_group_drvcfg_s drv_cfg[SNS_REG_SSI_SMGR_CFG_NUM_SENSORS];
} sns_reg_ssi_smgr_cfg_group_s;

typedef PACK(struct)
{
  uint8_t   maj_ver_no;
  uint8_t   min_ver_no;
  uint16_t  i2c_sda_1;
  uint16_t  i2c_scl_1;
  uint16_t  i2c_sda_2;
  uint16_t  i2c_scl_2;
  uint16_t  sns_reset;
  uint16_t  sns_test;
  uint16_t  sns_accel_md;
  uint16_t  sns_accel_dri;
  uint16_t  sns_gyro_dri;
  uint16_t  sns_mag_dri;
  uint16_t  sns_alsp_int;
  uint16_t  sns_gest_int;
  uint16_t  sns_press_int;
  uint16_t  sns_sar_int;
  uint16_t  sns_fp_int;
  uint16_t  sns_hall_int;
} sns_reg_ssi_gpio_cfg_group_s;

/**
 *  Registry Data Group for IR Gesture
 *  These are generic fields that the IR gesture driver may
 *  use as necessary.
 */
typedef PACK(struct)
{
  uint32_t  ds1;
  uint32_t  ds2;
  uint32_t  ds3;
  uint32_t  ds4;
  uint32_t  ds5;
  uint32_t  ds6;
  uint32_t  ds7;
  uint32_t  ds8;
  uint32_t  ds9;
  uint32_t  ds10;
  uint32_t  ds11;
  uint32_t  ds12;
  uint32_t  ds13;
  uint32_t  ds14;
  uint32_t  ds15;
  uint32_t  ds16;
  uint32_t  ds17;
  uint32_t  ds18;
  uint32_t  ds19;
  uint32_t  ds20;
  uint32_t  ds21;
  uint32_t  ds22;
  uint32_t  ds23;
  uint32_t  ds24;
  uint32_t  ds25;
  uint32_t  ds26;
  uint32_t  ds27;
  uint32_t  ds28;
  uint32_t  ds29;
  uint32_t  ds30;
  uint32_t  ds31;
  uint32_t  ds32;
  uint32_t  ds33;
  uint32_t  ds34;
  uint32_t  ds35;
  uint32_t  ds36;
  uint32_t  ds37;
  uint32_t  ds38;
  uint32_t  ds39;
  uint32_t  ds40;
  uint32_t  ds41;
  uint32_t  ds42;
  uint32_t  ds43;
  uint32_t  ds44;
  uint32_t  ds45;
  uint32_t  ds46;
  uint32_t  ds47;
  uint32_t  ds48;
  uint32_t  ds49;
  uint32_t  ds50;
  uint32_t  ds51;
  uint32_t  ds52;
  uint32_t  ds53;
  uint32_t  ds54;
  uint32_t  ds55;
  uint32_t  ds56;
  uint32_t  ds57;
  uint32_t  ds58;
  uint32_t  ds59;
  uint32_t  ds60;
  uint32_t  ds61;
  uint32_t  ds62;
  uint32_t  ds63;
  uint32_t  ds64;
} sns_reg_ir_ges_data_group_s;

/**
 *  Registry Data Group for RGB
 *  These are generic fields that the IR gesture driver may
 *  use as necessary.
 */
typedef PACK(struct)
{
  uint32_t  r_factor;
  uint32_t  g_factor;
  uint32_t  b_factor;
  uint32_t  c_factor;
  uint32_t  ir_factor;
  uint32_t  ds1;
  uint32_t  ds2;
  uint32_t  ds3;
  uint32_t  ds4;
  uint32_t  ds5;
  uint32_t  ds6;
  uint32_t  ds7;
  uint32_t  ds8;
  uint32_t  ds9;
  uint32_t  ds10;
  uint32_t  ds11;
  uint32_t  ds12;
  uint32_t  ds13;
  uint32_t  ds14;
  uint32_t  ds15;
  uint32_t  ds16;
  uint32_t  ds17;
  uint32_t  ds18;
  uint32_t  ds19;
  uint32_t  ds20;
  uint32_t  ds21;
  uint32_t  ds22;
  uint32_t  ds23;
  uint32_t  ds24;
  uint32_t  ds25;
  uint32_t  ds26;
  uint32_t  ds27;
} sns_reg_rgb_data_group_s;

/**
 *  Registry Data Group for game rotation vector driver
 */
typedef PACK(struct)
{
  // Axis orientation refers to the device's orientation with respect to the
  // phone. Values must be in the range [-3, 3] excluding zero, where the
  // number indicates a mapping to a particular axis (1=x, 2=y, 3=z) and the
  // sign indicates direction. E.g., to map the device's x axis to the phone's
  // -y axis, set accelx_orient = -2.
  int8_t grv_x_orient;
  int8_t grv_y_orient;
  int8_t grv_z_orient;
} sns_reg_grv_driver_group_s;

/**
 * Registry Data Group for Tilt Detector
 */
typedef PACK(struct)
{
  q16_t    sensor_report_rate;     /* Default sensor report rate in Hz (Q16) */
  q16_t    sensor_sample_rate;     /* Default sensor sample rate in Hz (Q16) */
  float    init_accel_window_time; /* Initial accel box filter window time in secs */
  float    accel_window_time;      /* Accel box filter window time in secs */
  float    def_tilt_angle_thresh;  /* Default tilt angle threshold in radians */
} sns_reg_tilt_detector_data_group_s;

/**
 * Available sensor types, for use in
 * sns_reg_ssi_smgr_cfg_group_s.drv_cfg[i].data_type
 *
 * Should match sns_ddf_sensor_e in sns_ddf_common.h
 */
typedef enum
{
  SNS_REG_SSI_DATA_TYPE_NONE,
  SNS_REG_SSI_DATA_TYPE_ACCEL,
  SNS_REG_SSI_DATA_TYPE_MAG,
  SNS_REG_SSI_DATA_TYPE_GYRO,
  SNS_REG_SSI_DATA_TYPE_TEMP,
  SNS_REG_SSI_DATA_TYPE_PROXIMITY,
  SNS_REG_SSI_DATA_TYPE_AMBIENT,
  SNS_REG_SSI_DATA_TYPE_PRESSURE,
  SNS_REG_SSI_DATA_TYPE_MAG_6D,
  SNS_REG_SSI_DATA_TYPE_GYRO_6D,
  SNS_REG_SSI_DATA_TYPE_DOUBLETAP,
  SNS_REG_SSI_DATA_TYPE_SINGLETAP,
  SNS_REG_SSI_DATA_TYPE_IR_GESTURE,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_01,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_02,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_03,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_04,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_05,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_06,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_07,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_08,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_09,
  SNS_REG_SSI_DATA_TYPE_OEM_SENSOR_10,
  SNS_REG_SSI_DATA_TYPE_STEP_EVENT,
  SNS_REG_SSI_DATA_TYPE_STEP_COUNT,
  SNS_REG_SSI_DATA_TYPE_SMD,
  SNS_REG_SSI_DATA_TYPE_GAME_RV,
  SNS_REG_SSI_DATA_TYPE_HUMIDITY,
  SNS_REG_SSI_DATA_TYPE_RGB,
  SNS_REG_SSI_DATA_TYPE_CT_C,
  SNS_REG_SSI_DATA_TYPE_SAR,
  SNS_REG_SSI_DATA_TYPE_HALL_EFFECT,
  SNS_REG_SSI_DATA_TYPE_AMBIENT_TEMP,
  SNS_REG_SSI_DATA_TYPE_HEART_RATE,
  /**<Add new types here */
  SNS_REG_SSI_DATA_TYPE__ALL,        /**< Addresses all sensors */
  SNS_REG_SSI_DATA_TYPE_LAST
} sns_reg_ssi_data_type_e;

/**
 * Flag values for use in
 * sns_reg_ssi_smgr_cfg_group_s.drv_cfg[i].flags
 *
 * These values should be ORed together if a sensor supports
 * more than one flag
 */
/**
 * Flag true if sensor allows setting of internal sampling rate and that rate
 * determines the hardware filter bandwidth. */
#define SNS_REG_SSI_FLAG_LPF_FROM_RATE           0x20

/**
 * Flag to disable sensitivity and dynamic range control. Ignore
 * sensitivity parameter in sensor request.
 * Use SNS_REG_SSI_FLAG_xx_SENSITIVITY_DEFAULT value as the fixed range */
#define SNS_REG_SSI_FLAG_NO_SENSITIVITY          0x40

/**
 * Flag true if the device driver is capable of self scheduling */
#define SNS_REG_SSI_FLAG_SELF_SCHED              0x80

/* Flag value to control enable/disable of latency measurement for a sensor */
#define SNS_REG_SSI_FLAG_LTCY_ENABLE             0x08

/* Flag to indicate that FIFO is enabled in the device */
#define SNS_REG_SSI_FLAG_ENABLE_FIFO             0x10

/**
 *  Registry Data Group for SSI DEVINFO parameters
 */
#define SNS_REG_SSI_DEVINFO_NUM_CFGS 6
typedef PACK(struct)
{
  uint8_t drvuuid[16];
  int32_t off_to_idle;
  int32_t idle_to_ready;
  uint16_t gpio1;
  uint16_t reg_group_id;
  uint16_t cal_pri_group_id;
  uint16_t i2c_bus;
  uint8_t i2c_address;
  uint8_t sensitivity_default;
  uint8_t flags;
  uint8_t reserved1;
  uint32_t reserved2;
  uint16_t reserved3;
} sns_reg_ssi_devinfo_uuid_cfg_s;

typedef PACK(struct)
{
  int8_t min_ver_no;
  int8_t num_uuid_dev_info_valid;
  uint16_t reserved;
  sns_reg_ssi_devinfo_uuid_cfg_s uuid_cfg[SNS_REG_SSI_DEVINFO_NUM_CFGS];
} sns_reg_ssi_devinfo_group_s;

/**
 *  Registry Data Group for SSI SENSOR_DEP_CFG parameters
 */
typedef PACK(struct)
{
  uint16_t reg_item_id; /* Registry item id */
  uint8_t size;         /* Size of registry item, 0-4 */
  uint8_t value[4];     /* Item value */
} sns_reg_ssi_sensor_dep_reg_item_s;

#define SNS_REG_MAX_SENSOR_DEP_REG_ITEMS 4
typedef PACK(struct)
{
  uint8_t drvuuid[16];   /* Driver's UUID */
  sns_reg_ssi_sensor_dep_reg_item_s reg_items[SNS_REG_MAX_SENSOR_DEP_REG_ITEMS]; /* Array of registry items - max 4 */
} sns_reg_ssi_sensor_dep_uuid_cfg_s;

#define SNS_REG_MAX_SENSORS_WITH_DEP_REG_ITEMS 5
typedef PACK(struct)
{
  uint8_t ver_no;         /* version no */
  uint8_t sensor_type;    /* 0-unused, 1-accel, 2-gyro, 3-mag, 4-light, 5-pressure */
  uint16_t next_group_id; /* Id of next group, if available */
  sns_reg_ssi_sensor_dep_uuid_cfg_s uuid_reg_cfg[SNS_REG_MAX_SENSORS_WITH_DEP_REG_ITEMS]; /* Sensor dependent registry items */
} sns_reg_ssi_sensor_dep_reg_group_s;

/**
 *  UUIDs used by SSI
 */
#define SNS_REG_UUID_NULL \
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define SNS_REG_UUID_ADXL350 \
  {0x1f,0xc7,0xe4,0x1b,0xfe,0x89,0x47,0x2e,0xad,0x7c,0x52,0x28,0x20,0x7b,0xf2,0xae}
#define SNS_REG_UUID_BMA150 \
  {0x20,0x45,0x83,0xd2,0x26,0xa4,0x41,0xc4,0xa2,0x54,0x15,0xbc,0xe1,0x29,0x36,0x31}
#define SNS_REG_UUID_BMA250 \
  {0xf3,0x7a,0x60,0xdf,0x0c,0x82,0x4a,0xc7,0xba,0xd7,0x98,0x94,0x5e,0x58,0x21,0x8a}
#define SNS_REG_UUID_LIS3DH \
  {0x14,0xb6,0xfa,0xeb,0xe0,0x28,0x4b,0x3a,0xbf,0xff,0x7d,0x04,0xf5,0x75,0xac,0x14}
#define SNS_REG_UUID_LIS3DSH \
  {0xfb,0x81,0xab,0x89,0x0d,0x10,0x42,0xc4,0xb9,0x06,0x33,0xa5,0x46,0x9c,0xa6,0xac}
#define SNS_REG_UUID_ISL29011 \
  {0xb7,0x7d,0xac,0xdf,0x55,0x22,0x43,0xee,0x93,0x65,0x8f,0x25,0x87,0x0f,0x80,0x28}
#define SNS_REG_UUID_ISL29028 \
  {0xca,0xcc,0xc8,0xa5,0x0b,0xe9,0x4a,0x99,0x9a,0xfc,0x4c,0x27,0x4b,0xaa,0x37,0xec}
#define SNS_REG_UUID_ISL29147 \
  {0xa9,0xa0,0xb0,0x5f,0xe1,0xa3,0x45,0x26,0x94,0xb5,0x7f,0xfd,0xcd,0xfc,0x7b,0xd6}
#define SNS_REG_UUID_BMP085 \
  {0x96,0x4f,0x5e,0xc5,0x45,0x4d,0x49,0xf2,0x84,0xa7,0x38,0x2c,0x0f,0x93,0x24,0x86}
#define SNS_REG_UUID_APDS99XX \
  {0xe6,0x79,0xa8,0xd8,0xc3,0xfb,0x4b,0x4c,0xaf,0xfa,0x31,0x4d,0x9d,0x06,0xe6,0x0c}
#define SNS_REG_UUID_MPU3050 \
  {0x19,0xc8,0x1d,0x07,0x0e,0x93,0x46,0xc5,0x82,0xc0,0x93,0x5f,0xb8,0x0d,0x81,0x1b}
#define SNS_REG_UUID_L3G4200D \
  {0xf3,0x6c,0x06,0xdc,0x63,0xea,0x40,0x75,0xa2,0xc7,0xb9,0xcf,0x71,0x88,0xb8,0xfb}
#define SNS_REG_UUID_L3GD20 \
  {0x7d,0x8c,0x17,0xc2,0xcd,0x1b,0x4f,0xfd,0xb6,0x9d,0x56,0xc3,0xdf,0x14,0x4a,0x1f}
#define SNS_REG_UUID_AKM8973 \
  {0xc8,0xf5,0x0a,0x9e,0x9f,0x44,0x47,0x88,0x80,0x83,0x7a,0x73,0xbc,0x47,0xb2,0xf8}
#define SNS_REG_UUID_AKM8963 \
  {0xd4,0xb2,0x19,0x99,0xdd,0x43,0x4e,0x9e,0xb6,0xd8,0x9e,0x68,0x01,0x5a,0x5c,0x35}
#define SNS_REG_UUID_AKM8975 \
  {0x53,0xad,0xb1,0x5f,0x44,0xbf,0x43,0x5e,0x86,0xbf,0xd0,0x05,0xa9,0x76,0x78,0x3c}
#define SNS_REG_UUID_LSM303DLHC \
  {0x3f,0xf0,0x8b,0xb4,0xa7,0xbf,0x40,0xe5,0x95,0xe3,0x27,0xa2,0x0b,0x3b,0x3e,0x7c}
#define SNS_REG_UUID_MPU6050 \
  {0x60,0x2d,0x46,0x7b,0x75,0x75,0x45,0xcc,0xad,0x20,0x48,0x52,0x42,0xae,0x79,0x8d}
#define SNS_REG_UUID_AMI306 \
  {0x6f,0x04,0xce,0x18,0x47,0x20,0x45,0x10,0x88,0x60,0x0c,0xdb,0x95,0x5e,0x9a,0xea}
#define SNS_REG_UUID_LPS331AP \
  {0x08,0x1b,0xf0,0x3e,0x75,0x0f,0x46,0xa5,0x86,0xf9,0x2c,0xcb,0x34,0x1d,0xf0,0x0a}
#define SNS_REG_UUID_APDS91XX \
  {0x6b,0x9c,0x7f,0x67,0xae,0x6d,0x43,0x10,0xaa,0xc2,0x4c,0x63,0x4f,0x92,0x68,0xfe}
#define SNS_REG_UUID_YAS530 \
  {0x05,0x79,0x5b,0xb1,0x29,0x31,0x40,0xbd,0xb3,0x27,0xa8,0x46,0xd3,0x9a,0xc7,0x1d}
#define SNS_REG_UUID_BMC050 \
  {0x1c,0xc8,0xd8,0x96,0x0f,0x75,0x40,0x5b,0xa0,0x00,0x31,0x1e,0x0d,0x27,0x3e,0xe9}
#define SNS_REG_UUID_BMG160 \
  {0x45,0x03,0xe9,0x5b,0x57,0xc5,0x47,0xc3,0x8d,0x49,0x90,0xe9,0xbe,0xf9,0xb3,0xc0}
#define SNS_REG_UUID_BMM050 \
  {0xbc,0xfd,0xbf,0x4d,0xa5,0x5d,0x42,0x8c,0xa6,0x0a,0x97,0xbf,0x6b,0xc0,0x1b,0x29}
#define SNS_REG_UUID_GYRO_EWTZNA \
  {0x33,0xb9,0x0e,0x55,0xd1,0xb3,0x43,0x71,0x93,0xc1,0xc9,0x5b,0x6a,0x26,0xc8,0x1d}
#define SNS_REG_UUID_BMA2X2 \
  {0x5d,0x96,0x26,0x3f,0x36,0x44,0x46,0xaf,0x96,0x7c,0x05,0xd4,0xee,0xfa,0x0f,0xc1}
#define SNS_REG_UUID_TMD277X \
  {0xf7,0x69,0xe5,0xe9,0x04,0xf9,0x44,0xcc,0x8a,0x21,0xc0,0xa1,0x6e,0x1a,0xb1,0x9a}
#define SNS_REG_UUID_HSCD008 \
  {0xd2,0x7a,0xc1,0x7f,0x74,0x1a,0x45,0xe1,0x9e,0x69,0x51,0x30,0x35,0x6b,0x7b,0x6b}
#define SNS_REG_UUID_LTR55X \
  {0x6d,0xf3,0x19,0x90,0x54,0x7a,0x4a,0x19,0xb3,0xa8,0x9a,0x7b,0x64,0x31,0x15,0x2f}
#define SNS_REG_UUID_AL3320B \
  {0x24,0xda,0x64,0x8d,0x54,0x49,0x47,0x53,0xbc,0x8a,0x3e,0x2c,0xc9,0xf4,0xe4,0xb0}
#define SNS_REG_UUID_CM36283 \
  {0xdc,0x5d,0xae,0xcd,0xb0,0x6f,0xd6,0xe4,0x78,0xfb,0x3a,0x7e,0xae,0xb8,0x96,0x7f}
#define SNS_REG_UUID_MAX44009 \
  {0xf0,0x2a,0x1e,0xf3,0xf4,0xab,0x43,0x35,0x97,0x69,0xd0,0x97,0x16,0x8a,0xdc,0x1b}
#define SNS_REG_UUID_MPU6500 \
  {0xaa,0x32,0xfe,0x2e,0x36,0xcd,0x44,0xde,0xa0,0x2f,0x55,0xac,0x9c,0xf4,0xd1,0x9c}
#define SNS_REG_UUID_QDSP_SIM_PLAYBACK \
  {0x23,0xde,0x4b,0xea,0x7b,0xc6,0x4d,0xdc,0xb9,0xb3,0xa6,0xfc,0x19,0xeb,0x0d,0x69}
#define SNS_REG_UUID_SHTC1 \
  {0xbe,0xfb,0x7c,0xc6,0xaf,0x0c,0x41,0x40,0x95,0xa7,0x6d,0x71,0x78,0x14,0x63,0xcc}
#define SNS_REG_UUID_APDS9950 \
  {0x39,0xd9,0xe0,0x38,0xc6,0x43,0x4c,0xdb,0xa8,0x85,0x1d,0x2c,0xcb,0x16,0x36,0x94}
#define SNS_REG_UUID_MAX88120 \
  {0x2f,0xff,0x09,0xc4,0x10,0x31,0x4f,0x96,0xbd,0xc2,0xd5,0x37,0xb1,0xd1,0xb5,0x11}
#define SNS_REG_UUID_TMG399X \
  {0x32,0xfb,0x90,0xdb,0xc7,0x0b,0x40,0xf1,0xa2,0xf1,0xcd,0xd4,0xe1,0x9d,0xb6,0x8a}
#define SNS_REG_UUID_KXCJK \
  {0x50,0xa7,0xb4,0xbd,0x8e,0x57,0x43,0x2a,0x85,0x7f,0x3f,0xf8,0x8b,0x88,0x26,0x68}
#define SNS_REG_UUID_MMA8452 \
  {0xa4,0x1b,0xea,0x31,0x32,0x90,0x36,0x2e,0x93,0x3f,0x0e,0xb1,0x0c,0xd4,0x9a,0x96}
#define SNS_REG_UUID_AP3216C \
  {0x03,0x91,0x80,0xe2,0x13,0x24,0x44,0xc4,0xbf,0xa8,0xc9,0xa5,0x65,0xc2,0xe4,0x1d}
#define SNS_REG_UUID_ISL29044A \
  {0xBF,0xF5,0xE7,0x4A,0xFD,0xF7,0x77,0xCC,0xE1,0x96,0xAA,0xC5,0xCF,0x2B,0x82,0xB0}
#define SNS_REG_UUID_AKM09912 \
  {0xf4,0x57,0x90,0x35,0xf0,0x45,0x4f,0x57,0x8d,0xfc,0x8f,0x47,0x49,0xf9,0x14,0x92}
#define SNS_REG_UUID_BH1721 \
  {0x63,0xcb,0x8e,0x53,0xa1,0x9b,0x41,0x03,0x9e,0x4a,0x76,0x83,0x07,0x4f,0x9f,0xb1}
#define SNS_REG_UUID_M34160PJ \
  {0xb2,0x07,0x3e,0xa7,0xd2,0x8a,0x47,0xe1,0x82,0x21,0xfd,0x00,0x2a,0x21,0x02,0xbe}
#define SNS_REG_UUID_KXTIK \
  {0x62,0x9d,0x45,0xef,0x8a,0xbb,0x47,0x4a,0x8d,0xbc,0x2d,0x30,0x9a,0xf3,0xd1,0x23}
#define SNS_REG_UUID_MPU6515 \
  {0xa3,0x87,0x37,0xc5,0x50,0x03,0x43,0xe9,0x84,0x9c,0x6c,0xfb,0xc1,0xaa,0x37,0xe4}
#define SNS_REG_UUID_MPU6515_AKM8963 \
  {0xf5,0xab,0x99,0x6e,0xf4,0x35,0x4c,0xb1,0x84,0xfb,0x83,0xd2,0xf7,0x51,0x47,0x4a}
#define SNS_REG_UUID_MPU6515_BMP280 \
  {0x7a,0x74,0x82,0x94,0xbc,0x9a,0x4c,0xa9,0x88,0x38,0x34,0x8b,0x4a,0x11,0x34,0xca}
#define SNS_REG_UUID_SHTC1 \
  {0xbe,0xfb,0x7c,0xc6,0xaf,0x0c,0x41,0x40,0x95,0xa7,0x6d,0x71,0x78,0x14,0x63,0xcc}
#define SNS_REG_UUID_APDS9950 \
  {0x39,0xd9,0xe0,0x38,0xc6,0x43,0x4c,0xdb,0xa8,0x85,0x1d,0x2c,0xcb,0x16,0x36,0x94}
#define SNS_REG_UUID_ADI7146 \
  {0xa8,0xdd,0x53,0x19,0x9c,0xd1,0x4f,0x7f,0xb4,0x04,0xd6,0x9e,0xa8,0xfa,0x8c,0x9b}
#define SNS_REG_UUID_M34160PJ \
  {0xb2,0x07,0x3e,0xa7,0xd2,0x8a,0x47,0xe1,0x82,0x21,0xfd,0x00,0x2a,0x21,0x02,0xbe}
#define SNS_REG_UUID_PLAYBACK \
  {0x23,0xde,0x4b,0xea,0x7b,0xc6,0x4d,0xdc,0xb9,0xb3,0xa6,0xfc,0x19,0xeb,0x0d,0x69}
#define SNS_REG_UUID_MAX44006 \
  {0xf2,0x7a,0x90,0xe3,0x0f,0xe7,0x40,0xc0,0x85,0x68,0x77,0x74,0xb4,0x6d,0xed,0x4e}
#define SNS_REG_UUID_MC3410 \
  {0x34,0x38,0x7b,0x68,0x55,0x7b,0x4c,0x4f,0xb5,0xa4,0xa6,0x6e,0x4c,0x7f,0x12,0x7a}
#define SNS_REG_UUID_AKM09911 \
  {0xCB,0x8D,0xB3,0x85,0xB9,0x2B,0xA4,0xA7,0x67,0xE7,0x81,0x21,0x6A,0x1B,0xD0,0xF0}
#define SNS_REG_UUID_BU52061NVX \
  {0x1e,0xc8,0x2e,0xd2,0x94,0x24,0x41,0x7e,0x84,0xbd,0xc7,0xd3,0x55,0xcb,0xa5,0x5d}
#define SNS_REG_UUID_LPS25H \
  {0x5c,0x19,0x4e,0xe4,0xbc,0x90,0x44,0xf8,0xbf,0x94,0x42,0x16,0x44,0xe6,0xe4,0x04}
#define SNS_REG_UUID_HSPPAD038A \
  {0xe8,0xdd,0xb0,0x38,0x70,0x9b,0x46,0x69,0x90,0x64,0x3f,0xe0,0x4d,0xc1,0x88,0x55}
#define SNS_REG_UUID_ISL29033 \
  {0x56,0x39,0xb1,0x9e,0x90,0xed,0x4b,0x45,0x88,0x77,0x8b,0x29,0xe9,0x5f,0xae,0xaa}

/* Generic UUIDs available for vendor specific drivers */
#define SNS_REG_UUID_VENDOR_1 \
  {0x61,0xc6,0x95,0x37,0x96,0x75,0x40,0xc1,0xbc,0x46,0xfd,0x1b,0x43,0xb8,0x25,0x0b}
#define SNS_REG_UUID_VENDOR_2 \
  {0xd3,0x65,0x13,0x8b,0x98,0x92,0x49,0xc0,0x9d,0xbd,0x68,0xde,0xfb,0xaa,0x95,0x2c}

/**
 *  Registry Data Group for Basic Gestures
 */
typedef PACK(struct)
{
   int32_t sensor_report_rate; /* Default sensor report rate in Hz (Q16) */
} sns_reg_basic_gestures_data_group_s;

/**
 *  Registry Data Group for Facing
 */
typedef PACK(struct)
{
   int32_t sensor_report_rate; /* Default sensor report rate in Hz (Q16) */
} sns_reg_facing_data_group_s;

/**
 *  Registry Data Group for Quaternion
 */
typedef PACK(struct)
{
   int32_t sensor_report_rate; /* Default sensor report rate in Hz (Q16) */
} sns_reg_quaternion_data_group_s;

/**
 *  Registry Data Group for Rotation Vector
 */
typedef PACK(struct)
{
   int32_t sensor_report_rate; /* Default sensor report rate in Hz (Q16) */
} sns_reg_rotation_vector_data_group_s;

/**
 *  Registry Data Group for Distance Bound
 */
typedef PACK(struct)
{
   float speedbounds_unknown;
   float speedbounds_stationary;
   float speedbounds_inmotion;
   float speedbounds_fiddle;
   float speedbounds_pedestrian;
   float speedbounds_vehicle;
   float speedbounds_walk;
   float speedbounds_run;
   float speedbounds_bike;
} sns_reg_distance_bound_data_group_s;

/**
 *  Registry Data Group for Coarse Motion Classifier
 */
typedef PACK(struct)
{
   int32_t sensor_report_rate; /* Default sensor report rate in Hz (Q16) */
} sns_reg_cmc_data_group_s;

/**
 *  Registry Data Groups for QMAG Calibration
 */
#define QMAG_CAL_CALIBRATION_HISTORY (5)
#define QMAG_CAL_RADIUS_HISTORY (5)
#define QMAG_CAL_COL (3)
typedef PACK(struct)
{
   uint32_t  version_no;
   uint8_t   enable_algo;
   uint8_t   enable_si_cal;
   uint8_t   enable_hi_cal;
   int32_t   sample_rate;
   int32_t   report_rate;
   uint8_t   enable_gyro_assist;
   float     offset_x_hi_publish;
   float     offset_y_hi_publish;
   float     offset_z_hi_publish;
   int32_t   accuracy_hi_publish;
   float     radius_hi_buff[QMAG_CAL_RADIUS_HISTORY];
   uint8_t   radius_hi_buff_count;
   float     offset_hi_buff[QMAG_CAL_CALIBRATION_HISTORY][QMAG_CAL_COL];
   uint8_t   offset_hi_buff_count;
   float     accuracy_hi_buff[QMAG_CAL_CALIBRATION_HISTORY];
   uint8_t   accuracy_hi_buff_count;
}sns_reg_qmag_cal_algo_data_group_s;

#define MAG_DYN_CAL_CAL_MAT_ROW 3
#define MAG_DYN_CAL_CAL_MAT_COL 3
typedef PACK(struct)
{
   uint32_t  version_no;
   uint8_t   bias_valid;
   uint8_t   cal_mat_valid;
   int32_t   x_dyn_bias;
   int32_t   y_dyn_bias;
   int32_t   z_dyn_bias;
   int32_t   compensation_matrix[MAG_DYN_CAL_CAL_MAT_ROW][MAG_DYN_CAL_CAL_MAT_COL];
   int32_t   accuracy;
}sns_reg_mag_dyn_cal_params_data_group_s;

typedef PACK(struct)
{
   uint32_t version_no;
   float    published_offs_HI_x;
   float    published_offs_HI_y;
   float    published_offs_HI_z;
   uint8_t published_accuracy_HI;
   uint32_t published_offset_time;
   uint8_t published_offset_valid;
}sns_reg_qmag_cal_state_data_group_s;

/**
 *  Registry Data Group for Game Rotation Vector
 */
typedef PACK(struct)
{
   int32_t sensor_report_rate; /* Default sensor report rate in Hz (Q16) */
} sns_reg_game_rot_vec_data_group_s;

/**
 * Registry Data Group for SMD
 */
typedef PACK(struct)
{
   uint8_t   version;
   uint8_t   variable_decision_latency;
   uint8_t   max_latency;
   uint8_t   accel_window_time;
   uint8_t   pedometer_stepcount_thr;
   uint8_t   pedometer_time;
   uint8_t   delta_reset_time;
   uint8_t   long_accel_window_time;
   int32_t   self_transition_prob_sm;
   int32_t   detect_threshold;
   int32_t   eigen_thr;
   int32_t   acc_norm_std_dev_thresh;
   int32_t   sensor_report_rate;  /* Default sensor report rate in Hz (Q16) */
   int32_t   sample_rate;         /* Default sample rate in Hz (Q16) */
} sns_reg_smd_data_group_s;

#define MAG_FAC_CAL_CAL_MAT_ROW 3
#define MAG_FAC_CAL_CAL_MAT_COL 3
typedef PACK(struct)
{
   uint32_t  version_no;
   uint8_t   bias_valid;
   uint8_t   cal_mat_valid;
   int32_t   x_fac_bias;
   int32_t   y_fac_bias;
   int32_t   z_fac_bias;
   int32_t   compensation_matrix[MAG_FAC_CAL_CAL_MAT_ROW][MAG_FAC_CAL_CAL_MAT_COL];
   int32_t   accuracy;
}sns_reg_mag_fac_cal_params_data_group_s;

#ifdef _WIN32
#pragma pack(pop)
#endif /* _WIN32 */

#endif /* _SNS_REG_COMMON_H_ */

