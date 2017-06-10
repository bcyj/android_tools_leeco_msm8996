#ifndef SNS_REG_SVC_SERVICE_H
#define SNS_REG_SVC_SERVICE_H
/**
  @file sns_reg_api_v01.h
  
  @brief This is the public header file which defines the SNS_REG_SVC service Data structures.

  This header file defines the types and structures that were defined in 
  SNS_REG_SVC. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were 
  defined in the IDL as messages contain mandatory elements, optional 
  elements, a combination of mandatory and optional elements (mandatory 
  always come before optionals in the structure), or nothing (null message)
   
  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to. 
   
  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:
   
  uint32_t test_opaque_len;
  uint8_t test_opaque[16];
   
  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set 
  by the decode routine and should be checked so that the correct number of 
  elements in the array will be accessed. 

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5 
   It requires encode/decode library version 4 or later
   It was generated on: Fri Sep  7 2012
   From IDL File: sns_reg_api_v01.idl */

/** @defgroup SNS_REG_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_REG_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_REG_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_REG_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_REG_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_REG_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_REG_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_REG_SVC_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SNS_REG_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_REG_SVC_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_REG_SVC_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define SNS_REG_SVC_V01_MAX_MESSAGE_ID 0x0006;
/** 
    @} 
  */


/** @addtogroup SNS_REG_SVC_qmi_consts 
    @{ 
  */

/**  Indication status code
   */
#define SNS_REG_IND_STATUS_OK_V01 0
#define SNS_REG_IND_STATUS_RD_ERR_V01 1
#define SNS_REG_IND_STATUS_WR_ERR_V01 2
#define SNS_REG_IND_STATUS_INVALID_V01 3

/**  Constant declaration 
   */
#define SNS_REG_MAX_ITEM_BYTE_COUNT_V01 8
#define SNS_REG_MAX_GROUP_BYTE_COUNT_V01 256

/**  Group ID's for SMGR 

 Group ID's required by SMGR for Accel Sensor  */
#define SNS_REG_SMGR_GROUP_ACCEL_V01 0
#define SNS_REG_SMGR_GROUP_ACCEL_2_V01 1
#define SNS_REG_SMGR_GROUP_ACCEL_3_V01 2
#define SNS_REG_SMGR_GROUP_ACCEL_4_V01 3
#define SNS_REG_SMGR_GROUP_ACCEL_5_V01 4

/**  Group ID's required by SMGR for Gyro Sensor  */
#define SNS_REG_SMGR_GROUP_GYRO_V01 10
#define SNS_REG_SMGR_GROUP_GYRO_2_V01 11
#define SNS_REG_SMGR_GROUP_GYRO_3_V01 12
#define SNS_REG_SMGR_GROUP_GYRO_4_V01 13
#define SNS_REG_SMGR_GROUP_GYRO_5_V01 14

/**  Group ID's required by SMGR for Mag Sensor  */
#define SNS_REG_SMGR_GROUP_MAG_V01 20
#define SNS_REG_SMGR_GROUP_MAG_2_V01 21
#define SNS_REG_SMGR_GROUP_MAG_3_V01 22
#define SNS_REG_SMGR_GROUP_MAG_4_V01 23
#define SNS_REG_SMGR_GROUP_MAG_5_V01 24

/**  Group ID's required by SMGR for Pressure Sensor  */
#define SNS_REG_SMGR_GROUP_PRESSURE_V01 30
#define SNS_REG_SMGR_GROUP_PRESSURE_2_V01 31
#define SNS_REG_SMGR_GROUP_PRESSURE_3_V01 32
#define SNS_REG_SMGR_GROUP_PRESSURE_4_V01 33
#define SNS_REG_SMGR_GROUP_PRESSURE_5_V01 34

/**  Group ID's required by SMGR for ALS/Prox Sensor  */
#define SNS_REG_SMGR_GROUP_PROX_LIGHT_V01 40
#define SNS_REG_SMGR_GROUP_PROX_LIGHT_2_V01 41
#define SNS_REG_SMGR_GROUP_PROX_LIGHT_3_V01 42
#define SNS_REG_SMGR_GROUP_PROX_LIGHT_4_V01 43
#define SNS_REG_SMGR_GROUP_PROX_LIGHT_5_V01 44

/**  Group ID's for items not specific to any one sensor */
#define SNS_REG_GROUP_SENSOR_TEST_V01 50

/**  Group ID's required by device drivers. All group ID's used
    by device drivers must start from 1000 onwards.

 Group ID's required by Accel device drivers  */
#define SNS_REG_DRIVER_GROUP_ACCEL_V01 1000
#define SNS_REG_DRIVER_GROUP_ACCEL_2_V01 1001
#define SNS_REG_DRIVER_GROUP_ACCEL_3_V01 1002
#define SNS_REG_DRIVER_GROUP_ACCEL_4_V01 1003
#define SNS_REG_DRIVER_GROUP_ACCEL_5_V01 1004

/**  Group ID's required by Gyro device drivers  */
#define SNS_REG_DRIVER_GROUP_GYRO_V01 1010
#define SNS_REG_DRIVER_GROUP_GYRO_2_V01 1011
#define SNS_REG_DRIVER_GROUP_GYRO_3_V01 1012
#define SNS_REG_DRIVER_GROUP_GYRO_4_V01 1013
#define SNS_REG_DRIVER_GROUP_GYRO_5_V01 1014

/**  Group ID's required by Mag device drivers  */
#define SNS_REG_DRIVER_GROUP_MAG_V01 1020
#define SNS_REG_DRIVER_GROUP_MAG_2_V01 1021
#define SNS_REG_DRIVER_GROUP_MAG_3_V01 1022
#define SNS_REG_DRIVER_GROUP_MAG_4_V01 1023
#define SNS_REG_DRIVER_GROUP_MAG_5_V01 1024

/**  Group ID's required by pressure device drivers  */
#define SNS_REG_DRIVER_GROUP_PRESSURE_V01 1030
#define SNS_REG_DRIVER_GROUP_PRESSURE_2_V01 1031
#define SNS_REG_DRIVER_GROUP_PRESSURE_3_V01 1032
#define SNS_REG_DRIVER_GROUP_PRESSURE_4_V01 1033
#define SNS_REG_DRIVER_GROUP_PRESSURE_5_V01 1034

/**  Group ID's required by ALS/Prox device drivers  */
#define SNS_REG_DRIVER_GROUP_PROX_LIGHT_V01 1040
#define SNS_REG_DRIVER_GROUP_PROX_LIGHT_2_V01 1041
#define SNS_REG_DRIVER_GROUP_PROX_LIGHT_3_V01 1042
#define SNS_REG_DRIVER_GROUP_PROX_LIGHT_4_V01 1043
#define SNS_REG_DRIVER_GROUP_PROX_LIGHT_5_V01 1044

/**  Group ID's required by SAM for QMD algorithms */
#define SNS_REG_SAM_GROUP_AMD_V01 2000
#define SNS_REG_SAM_GROUP_VMD_V01 2001
#define SNS_REG_SAM_GROUP_RMD_V01 2002

/**  Group ID's required by SCM for Factory Calibration  */
#define SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V01 0
#define SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V01 10

/**  Group ID's required by SCM for GYRO_CAL  */
#define SNS_REG_SCM_GROUP_GYRO_CAL_ALGO_V01 2500
#define SNS_REG_SCM_GROUP_GYRO_CAL_V01 2500

/**  Group ID's required by SCM for Accel Sensor Dyanmic Bias Calibration */
#define SNS_REG_SCM_GROUP_ACCEL_DYN_CAL_PARAMS_V01 2600

/**  Group ID's required by SCM for Gyro Sensor Dyanmic Bias Calibration */
#define SNS_REG_SCM_GROUP_GYRO_DYN_CAL_PARAMS_V01 2610

/**  Group ID's required by SAM for filtering magnetic vector  */
#define SNS_REG_SAM_GROUP_FMV_PARAMS_V01 2620

/**  Item ID's

 Item ID's corresponding to group SNS_REG_SMGR_GROUP_ACCEL_V01 & SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS (Group Id:0) */
#define SNS_REG_ITEM_ACC_X_BIAS_V01 0
#define SNS_REG_ITEM_ACC_Y_BIAS_V01 1
#define SNS_REG_ITEM_ACC_Z_BIAS_V01 2
#define SNS_REG_ITEM_ACC_X_SCALE_V01 3
#define SNS_REG_ITEM_ACC_Y_SCALE_V01 4
#define SNS_REG_ITEM_ACC_Z_SCALE_V01 5

/**  Item ID's corresponding to group SNS_REG_SCM_GROUP_ACCEL_DYN_CAL (Group Id: 2600) */
#define SNS_REG_ITEM_ACC_X_DYN_BIAS_V01 6
#define SNS_REG_ITEM_ACC_Y_DYN_BIAS_V01 7
#define SNS_REG_ITEM_ACC_Z_DYN_BIAS_V01 8
#define SNS_REG_ITEM_ACC_X_DYN_SCALE_V01 9
#define SNS_REG_ITEM_ACC_Y_DYN_SCALE_V01 10
#define SNS_REG_ITEM_ACC_Z_DYN_SCALE_V01 11

/**  Item ID's corresponding to group SNS_REG_GROUP_SENSOR_TEST (Group Id: 50) */
#define SNS_REG_ITEM_RAW_DATA_MODE_V01 50

/**  Item ID's corresponding to group SNS_REG_DRIVER_GROUP_PROX_LIGHT_V01 (Group Id: 1040) */
#define SNS_REG_ITEM_ALP_VISIBLE_LIGHT_TRANS_RATIO_V01 100
#define SNS_REG_ITEM_ALP_IR_LIGHT_TRANS_RATIO_V01 101
#define SNS_REG_ITEM_ALP_DC_OFFSET_V01 102
#define SNS_REG_ITEM_ALP_NEAR_THRESHOLD_V01 103
#define SNS_REG_ITEM_ALP_FAR_THRESHOLD_V01 104
#define SNS_REG_ITEM_ALP_PRX_FACTOR_V01 105
#define SNS_REG_ITEM_ALP_ALS_FACTOR_V01 106

/**  Item ID's corresponding to group SNS_REG_SAM_GROUP_AMD_V01 (Group Id: 2000) */
#define SNS_REG_ITEM_AMD_DEF_ACC_SAMP_RATE_V01 200
#define SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V01 201
#define SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V01 202

/**  Item ID's corresponding to group SNS_REG_SAM_GROUP_VMD_V01 (Group Id: 2001) */
#define SNS_REG_ITEM_VMD_DEF_ACC_SAMP_RATE_V01 210
#define SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V01 211
#define SNS_REG_ITEM_VMD_INT_CFG_PARAM2_V01 212

/**  Item ID's corresponding to group SNS_REG_SAM_GROUP_RMD_V01 (Group Id: 2002) */
#define SNS_REG_ITEM_RMD_DEF_ACC_SAMP_RATE_V01 220
#define SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V01 221
#define SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V01 222
#define SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V01 223
#define SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V01 224

/**  Item ID's corresponding to group SNS_REG_SMGR_GROUP_GYRO & SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS (Group Id: 10) */
#define SNS_REG_ITEM_GYRO_X_BIAS_V01 300
#define SNS_REG_ITEM_GYRO_Y_BIAS_V01 301
#define SNS_REG_ITEM_GYRO_Z_BIAS_V01 302
#define SNS_REG_ITEM_GYRO_X_SCALE_V01 303
#define SNS_REG_ITEM_GYRO_Y_SCALE_V01 304
#define SNS_REG_ITEM_GYRO_Z_SCALE_V01 305

/**  Item ID's corresponding to group SNS_REG_SCM_GROUP_GYRO_DYN_CAL (Group Id: 2650) */
#define SNS_REG_ITEM_GYRO_X_DYN_BIAS_V01 306
#define SNS_REG_ITEM_GYRO_Y_DYN_BIAS_V01 307
#define SNS_REG_ITEM_GYRO_Z_DYN_BIAS_V01 308
#define SNS_REG_ITEM_GYRO_X_DYN_SCALE_V01 309
#define SNS_REG_ITEM_GYRO_Y_DYN_SCALE_V01 310
#define SNS_REG_ITEM_GYRO_Z_DYN_SCALE_V01 311

/**   Item ID's corresponding to group SNS_REG_SCM_GROUP_GYRO_CAL (Group Id: 2500)  */
#define SNS_REG_ITEM_GYRO_CAL_DEF_SAMP_RATE_V01 500
#define SNS_REG_ITEM_GYRO_CAL_DEF_NUM_SAMP_V01 501
#define SNS_REG_ITEM_GYRO_CAL_DEF_VAR_THOLD_V01 502
#define SNS_REG_ITEM_GYRO_CAL_DEF_ENABLE_ALGO_V01 503

/**   Item ID's corresponding to group SNS_REG_SAM_GROUP_FMV_PARAMS (Group Id: 2620)  */
#define SNS_REG_ITEM_FMV_TC_ACCURACY_0_V01 600
#define SNS_REG_ITEM_FMV_TC_ACCURACY_1_V01 601
#define SNS_REG_ITEM_FMV_TC_ACCURACY_2_V01 602
#define SNS_REG_ITEM_FMV_TC_ACCURACY_3_V01 603
#define SNS_REG_ITEM_FMV_TC_ACCURACY_4_V01 604
#define SNS_REG_ITEM_FMV_GYRO_GAP_THRESH_V01 605
#define SNS_REG_ITEM_FMV_MAG_GAP_FACTOR_V01 606
#define SNS_REG_ITEM_FMV_ROT_FOR_ZUPT_V01 607
#define SNS_REG_ITEM_FMV_MAX_MAG_INNOVATION_V01 608
/**
    @}
  */

/** @addtogroup SNS_REG_SVC_qmi_messages
    @{
  */
/** Request Message; This command reads a single item from the registry. */
typedef struct {

  /* Mandatory */
  uint16_t item_id;
  /**<   ID of the registry entry to be read  */
}sns_reg_single_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_REG_SVC_qmi_messages
    @{
  */
/** Response Message; This command reads a single item from the registry. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Mandatory */
  uint16_t item_id;

  /* Mandatory */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[SNS_REG_MAX_ITEM_BYTE_COUNT_V01];
}sns_reg_single_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_REG_SVC_qmi_messages
    @{
  */
/** Request Message; This command writes a single item to the registry.

This command writes a single item to the registry. */
typedef struct {

  /* Mandatory */
  uint16_t item_id;
  /**<   ID of the registry entry to be written */

  /* Mandatory */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[SNS_REG_MAX_ITEM_BYTE_COUNT_V01];
  /**<   Item data to be written to the registry. Data types of uint8/16/32/64 and 
       int8/16/32/64 are supported */
}sns_reg_single_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_REG_SVC_qmi_messages
    @{
  */
/** Response Message; This command writes a single item to the registry.

This command writes a single item to the registry. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
}sns_reg_single_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_REG_SVC_qmi_messages
    @{
  */
/** Request Message; This command reads a single item from the registry. */
typedef struct {

  /* Mandatory */
  uint16_t group_id;
  /**<   ID of the group of registry entries to be read  */
}sns_reg_group_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_REG_SVC_qmi_messages
    @{
  */
/** Response Message; This command reads a single item from the registry. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Mandatory */
  uint16_t group_id;

  /* Mandatory */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[SNS_REG_MAX_GROUP_BYTE_COUNT_V01];
}sns_reg_group_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_REG_SVC_qmi_messages
    @{
  */
/** Request Message; This command writes a single item to the registry.

This command writes a single item to the registry. */
typedef struct {

  /* Mandatory */
  uint16_t group_id;
  /**<   ID of the registry entry to be written */

  /* Mandatory */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[SNS_REG_MAX_GROUP_BYTE_COUNT_V01];
  /**<   Item data to be written to the registry. Data types of uint8/16/32/64 and 
       int8/16/32/64 are supported */
}sns_reg_group_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_REG_SVC_qmi_messages
    @{
  */
/** Response Message; This command writes a single item to the registry.

This command writes a single item to the registry. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
}sns_reg_group_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup SNS_REG_SVC_qmi_msg_ids
    @{
  */
#define SNS_REG_CANCEL_REQ_V01 0x0001
#define SNS_REG_CANCEL_RESP_V01 0x0001
#define SNS_REG_VERSION_REQ_V01 0x0002
#define SNS_REG_VERSION_RESP_V01 0x0002
#define SNS_REG_SINGLE_READ_REQ_V01 0x0003
#define SNS_REG_SINGLE_READ_RESP_V01 0x0003
#define SNS_REG_SINGLE_WRITE_REQ_V01 0x0004
#define SNS_REG_SINGLE_WRITE_RESP_V01 0x0004
#define SNS_REG_GROUP_READ_REQ_V01 0x0005
#define SNS_REG_GROUP_READ_RESP_V01 0x0005
#define SNS_REG_GROUP_WRITE_REQ_V01 0x0006
#define SNS_REG_GROUP_WRITE_RESP_V01 0x0006
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_REG_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_REG_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_REG_SVC_get_service_object_v01( ) \
          SNS_REG_SVC_get_service_object_internal_v01( \
            SNS_REG_SVC_V01_IDL_MAJOR_VERS, SNS_REG_SVC_V01_IDL_MINOR_VERS, \
            SNS_REG_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

