#ifndef SNS_DEBUG_API_H
#define SNS_DEBUG_API_H

/*============================================================================

@file
sns_debug_api.h

@brief
Contains the numeric string identifiers for debug strings. This file
is commong to the apps and DSPS processors. These numbers
are maintained in a separate file so that the DSPS need not include the string database adding to code memory.

Copyright (c) 2010-2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
============================================================================*/
#include "sns_diag_dsps_v01.h"

/* Sensors debug module ids
 * Ensure this list remains consistent with module_name_arrs[] in
 * sns_debug_main.c, contained within the function sns_fill_debug_str_mask().
 */
typedef enum {
   SNS_DBG_MOD_APPS_SMR,
   SNS_DBG_MOD_APPS_DIAG,
   SNS_DBG_MOD_APPS_ACM,
   SNS_DBG_MOD_APPS_INIT,
   SNS_DBG_MOD_DSPS_SMGR,
   SNS_DBG_MOD_DSPS_SAM,
   SNS_DBG_MOD_DSPS_SMR,
   SNS_DBG_MOD_DSPS_DD_ACCEL,
   SNS_DBG_MOD_DSPS_DD_GYRO,
   SNS_DBG_MOD_DSPS_DD_ALSPRX,
   SNS_DBG_MOD_DSPS_DD_MAG8975,
   SNS_DBG_MOD_APPS_MAIN,
   SNS_DBG_MOD_EM,
   SNS_DBG_MOD_APPS_PWR,
   SNS_DBG_MOD_APPS_SAM,
   SNS_DBG_MOD_DSPS_SCM,
   SNS_DBG_MOD_APPS_SCM,

   SNS_DBG_MOD_MDM_SMR,
   SNS_DBG_MOD_MDM_DIAG,
   SNS_DBG_MOD_MDM_ACM,
   SNS_DBG_MOD_MDM_INIT,
   SNS_DBG_MOD_MDM_MAIN,
   SNS_DBG_MOD_MDM_PWR,

   SNS_DBG_MOD_DSPS_DAL,
   SNS_DBG_MOD_DSPS_DDF,

   SNS_DBG_MOD_APPS_REG,
   SNS_DBG_MOD_APPS_TIME,
   SNS_DBG_MOD_DSPS_DIAG,
   SNS_DBG_MOD_DSPS_PWR,
   SNS_DBG_MOD_APPS_FILE,
   SNS_DBG_MOD_APPS_FSA,
   /* Last module ID */
   SNS_DBG_NUM_MOD_IDS
} sns_debug_module_id_e;

/** Sensors debug message mask type */
typedef uint64_t sns_debug_mask_t;

/* Defines to identify the string.
 * The define names have the following format for
 * easy readability
 * SNS_DEBUG_<module_name>_<string identifier>_ID
 */

/*=====================================================================
                 DIAG STRING IDENTIFIERS
=======================================================================*/
/* Important Note to add a new string:
 * Please Add your identifier after the last string
 */

//Format: SNS_DEBUG_<module_name>_<string identifier>
#define SNS_DEBUG_DIAG_TEST_STRING0_ID       0
#define SNS_DEBUG_DIAG_TEST_STRING1_ID       1
#define SNS_DEBUG_DIAG_TEST_STRING2_ID       2
#define SNS_DEBUG_DIAG_TEST_STRING3_ID       3


/*=====================================================================
                 SENSOR MANAGER (DSPS) IDENTIFIERS
=======================================================================*/
/* Important Note to add a new string:
 * Please Add your identifier after the last string
 */

//Format: DBG_<module_name>_<string identifier>
#define DBG_SMGR_GENERIC_STRING0               0
#define DBG_SMGR_GENERIC_STRING1               1
#define DBG_SMGR_GENERIC_STRING2               2
#define DBG_SMGR_GENERIC_STRING3               3
#define DBG_SMGR_MESSAGE_FLAG                  4
#define DBG_SMGR_DD_INIT_FLAG                  5
#define DBG_SMGR_DD_SERVICE_FLAG               6
#define DBG_SMGR_SCHED_FLAG                    7
#define DBG_SMGR_DATA_CYCLE_FLAG               8
#define DBG_SMGR_PLAN_CYCLE_FLAG               9
#define DBG_SMGR_SENSOR_ID                     10
#define DBG_SMGR_SENSOR_ACTION                 11
#define DBG_SMGR_SENSOR_STATE                  12
#define DBG_SMGR_SENSOR_READ_STATE             13
#define DBG_SMGR_SENSOR_DATA_STATE             14
#define DBG_SMGR_SENSOR_INIT_STATE             15
#define DBG_SMGR_SENSOR_POWER_STATE            16
#define DBG_SMGR_SENSOR_ALL_INIT_STATE         17
#define DBG_SMGR_HEADER_ABSTRACT               18
#define DBG_SMGR_RCVD_REQ                      19
#define DBG_SMGR_RESPONSE                      20
#define DBG_SMGR_ERR_RESP                      21
#define DBG_SMGR_REPORT_IND                    22
#define DBG_SMGR_CURR_TIMESTAMP                23
#define DBG_SMGR_REGISTER_TIMER                24
#define DBG_SMGR_LAST_TICK                     25
#define DBG_SMGR_CURR_INTERVAL                 26
#define DBG_SMGR_NEXT_HB                       27
#define DBG_SMGR_DATACYCLE_VAR                 28
#define DBG_SMGR_TUNING                        29
#define DBG_SMGR_ASYNC_DATA_STATUS             30
#define DBG_SMGR_ASYNC_DATA_VALUE              31
#define DBG_SMGR_HIGH_WATER_MARK             32
#define DBG_SMGR_BLOCK_SUMMARY               33
#define DBG_SMGR_BLOCK_DETAIL1               34
#define DBG_SMGR_BLOCK_DETAIL2               35
#define DBG_SMGR_LINE_DIVIDER                36


/*=====================================================================
               ALS PRX DEVICE DRIVER (DSPS) IDENTIFIERS
=======================================================================*/
/* Important Note to add a new string:
 * Please Add your identifier after the last string
 */

//Format: DBG_<module_name>_<string identifier>
#define DBG_DD_ALSPRX_WR_CMD2_REG_ERR                 0
#define DBG_DD_ALSPRX_WR_CMD1_REG_ERR                 1
#define DBG_DD_ALSPRX_DEV_STATE_PEND                  2
#define DBG_DD_ALSPRX_PRX_STATE_PEND                  3
#define DBG_DD_ALSPRX_ALS_RES_INVALID                 4
#define DBG_DD_ALSPRX_ALS_RNG_INVALID                 5
#define DBG_DD_ALSPRX_MLUX_TOO_LARGE                  6
#define DBG_DD_ALSPRX_ALS_RSP_READY                   7
#define DBG_DD_ALSPRX_ALS_NXT_RNG_SEL                 8
#define DBG_DD_ALSPRX_ALS_DATA_READY_ERR              9
#define DBG_DD_ALSPRX_ALS_DATA_READY_MAX_ATTEMPTS     10
#define DBG_DD_ALSPRX_ALS_DATA_READY_NO_RESAMPLE1     11
#define DBG_DD_ALSPRX_ALS_DATA_READY_RESAMPLE1        12
#define DBG_DD_ALSPRX_ALS_DATA_READY_NO_RESAMPLE2     13
#define DBG_DD_ALSPRX_ALS_DATA_READY_RESAMPLE2        14
#define DBG_DD_ALSPRX_PRX_BINARY_NEAR                 15
#define DBG_DD_ALSPRX_PRX_BINARY_FAR                  16
#define DBG_DD_ALSPRX_PRX_NOT_PENDING                 17
#define DBG_DD_ALSPRX_PRX_RSP_READY                   18
#define DBG_DD_ALSPRX_I2C_CMD1_READ_ERR               19
#define DBG_DD_ALSPRX_I2C_DATA_LSB_READ_ERR           20
#define DBG_DD_ALSPRX_I2C_READ_CMD1_NO_INT            21
#define DBG_DD_ALSPRX_I2C_CMD1_READ_WRONG_DATA_TYPE   22
#define DBG_DD_ALSPRX_I2C_CMD2_WR_ERR                 23
#define DBG_DD_ALSPRX_ALS_INIT                        24
#define DBG_DD_ALSPRX_PRX_INIT                        25
#define DBG_DD_ALSPRX_I2C_HI_THRESH_WR_ERR            26
#define DBG_DD_ALSPRX_I2C_LO_THRESH_WR_ERR            27
#define DBG_DD_ALSPRX_CMN_INIT_ERR                    28
#define DBG_DD_ALSPRX_ALS_INIT_ERR                    29
#define DBG_DD_ALSPRX_PRX_INIT_ERR                    30
#define DBG_DD_ALSPRX_SET_PWR_ST_ERR                  31
#define DBG_DD_ALSPRX_GET_DATA_STATE_ERR              32
#define DBG_DD_ALSPRX_GET_DATA_INVALID_SENSOR_ERR     33
#define DBG_DD_ALSPRX_GET_DATA_REQUEST_ERR            34
#define DBG_DD_ALSPRX_GET_ATTRIB_RES                  35
#define DBG_DD_ALSPRX_GET_ATTRIB_PWR                  36
#define DBG_DD_ALSPRX_GET_ATTRIB_ACC                  37
#define DBG_DD_ALSPRX_GET_ATTRIB_THRESH               38
#define DBG_DD_ALSPRX_GET_ATTRIB_ERR                  39
#define DBG_DD_ALSPRX_GET_ATTRIB_REQ                  40
#define DBG_DD_ALSPRX_GET_ATTRIB_SENSOR_TYPE_ERR      41
#define DBG_DD_ALSPRX_I2C_CMD1_WR_ERR                 42
#define DBG_DD_ALSPRX_GET_DATA_REQ                    43
#define DBG_DD_ALSPRX_HANDLE_TIMER                    44
#define DBG_DD_ALSPRX_GET_ATTRIB_RES_ADC              45
#define DBG_DD_ALSPRX_I2C_RD_ERR                      46
#define DBG_DD_ALSPRX_I2C_WR_ERR                      47
#define DBG_DD_ALSPRX_ALS_CONVERSION1                 48
#define DBG_DD_ALSPRX_ALS_CONVERSION2                 49
#define DBG_DD_ALSPRX_HANDLE_IRQ                      50
#define DBG_DD_ALSPRX_WRONG_IRQ                       51
#define DBG_DD_ALSPRX_IRQ_NO_INT_FLAG                 52
#define DBG_DD_ALSPRX_NV_USE_DRIVER_DEFAULTS          53
#define DBG_DD_ALSPRX_NV_PARAMS                       54
#define DBG_DD_ALSPRX_NV_DATA_FROM_SMGR               55
#define DBG_DD_ALSPRX_STRING0                         56
#define DBG_DD_ALSPRX_STRING1                         57
#define DBG_DD_ALSPRX_STRING2                         58
#define DBG_DD_ALSPRX_STRING3                         59

/*=====================================================================
               AKM MAG 8975 DEVICE DRIVER (DSPS) IDENTIFIERS
=======================================================================*/
/* Important Note to add a new string:
 * Please Add your identifier after the last string
 */

//Format: DBG_<module_name>_<string identifier>
#define DBG_DD_MAG8975_INITIALIZING            	0
#define DBG_DD_MAG8975_GET_ATTRIB_REQ           1
#define DBG_DD_MAG8975_SET_ATTRIB_REQ           2
#define DBG_DD_MAG8975_GET_DATA_REQ             3
#define DBG_DD_MAG8975_GET_ERROR                4
#define DBG_DD_MAG8975_HANDLE_IRQ_REQ           5
#define DBG_DD_MAG8975_GET_POWER_INFO_REQ       6
#define DBG_DD_MAG8975_GET_RANGE_REQ            7
#define DBG_DD_MAG8975_GET_RESOLUTION_REQ       8
#define DBG_DD_MAG8975_GET_DELAYS_REQ           9
#define DBG_DD_MAG8975_GET_DRIVER_INFO_REQ      10
#define DBG_DD_MAG8975_GET_DEVICE_INFO_REQ      11
#define DBG_DD_MAG8975_SET_POWER_STATE_REQ      12
#define DBG_DD_MAG8975_SET_RESOLUTION_REQ       13
#define DBG_DD_MAG8975_SET_RANGE_REQ            14
#define DBG_DD_MAG8975_FAILED_TO_PWR_DOWN       15
#define DBG_DD_MAG8975_PWR_MODE_NOT_SUPPORTED   16
#define DBG_DD_MAG8975_SENSITIVITY_DATA         17
#define DBG_DD_MAG8975_READ_FAILURE             18
#define DBG_DD_MAG8975_TEST                     19

/*=====================================================================
              ACCEL DEVICE DRIVER (DSPS) IDENTIFIERS
=======================================================================*/
/* Important Note to add a new string:
 * Please Add your identifier after the last string
 */

//Format: DBG_<module_name>_<string identifier>
#define DBG_DD_ACCEL_INIT                                  0

/*=====================================================================
                GYRO DEVICE DRIVER (DSPS) IDENTIFIERS
=======================================================================*/
/* Important Note to add a new string:
 * Please Add your identifier after the last string
 */

//Format: DBG_<module_name>_<string identifier>
#define DBG_DD_GYRO_INIT                                   0



/*=====================================================================
                 SENSOR ALGORITHM MANAGER (DSPS) IDENTIFIERS
=======================================================================*/
/* Important Note to add a new string:
 * Please Add your identifier after the last string
 */

//Format: DBG_<module_name>_<string identifier>
#define DBG_SAM_TIMER_CB_SIGNALERR              0
#define DBG_SAM_REG_TIMER_STARTED               1
#define DBG_SAM_REG_TIMER_FAILED                2
#define DBG_SAM_DEREG_TIMER_DELETED             3
#define DBG_SAM_DEREG_TIMER_FAILED              4
#define DBG_SAM_SAM_TASK_SAMSTARTED             5
#define DBG_SAM_PROCESS_EVT_UNKWN_EVT           6
#define DBG_SAM_PROCESS_MSG_RCVD_MSG            7
#define DBG_SAM_PROCESS_MSG_STATUS              8
#define DBG_SAM_HNDL_REPORT_STATUS              9
#define DBG_SAM_PROCESS_REQ_DISABLE_ERR        10
#define DBG_SAM_PROCESS_REQ_INVALID_ALGOREQ    11
#define DBG_SAM_PROCESS_REQ_INVALID_REQ        12
#define DBG_SAM_SMGR_RESP_DROPPED              13
#define DBG_SAM_SMGR_RESP_SUCCESS              14
#define DBG_SAM_SMGR_RESP_ACK_VAL              15
#define DBG_SAM_SMGR_RESP_RESPINFO             16
#define DBG_SAM_SMGR_IND_MSGID                 17
#define DBG_SAM_SMGR_IND_DROPPED               18
#define DBG_SAM_SMGR_IND_STATUS                19
#define DBG_SAM_SMGR_IND_INVALID               20
#define DBG_SAM_SMGR_IND_DELIVERY_SUCC         21
#define DBG_SAM_REQ_SNSR_DATA_MSG              22
#define DBG_SAM_SEND_RSP_MSG                   23
#define DBG_SAM_RPT_IND_MSG                    24
#define DBG_SAM_RPT_ERRIND_MSG                 25
#define DBG_SAM_ADD_CLIENT_MAX_ERR             26
#define DBG_SAM_ADD_CLIENT_INFO                27
#define DBG_SAM_DELETE_CLIENT_INFO             28
#define DBG_SAM_GET_ALGO_INDX_ERR              29
#define DBG_SAM_REG_ALGO_SUCCESS               30
#define DBG_SAM_REG_ALGO_ERR                   31
#define DBG_SAM_REG_ALGO_DFLT_ERR              32
#define DBG_SAM_ENABLE_ALGO_STATE_NULL         33
#define DBG_SAM_ENABLE_ALGO                    34
#define DBG_SAM_DISABLE_ALGO_INSTANCE_ERR      35
#define DBG_SAM_DISABLE_ALGO                   36
#define DBG_SAM_ALGO_MEM_INFO                  37
#define DBG_SAM_ALGO_STATE_MEM_INFO            38
#define DBG_SAM_STOP_SNSR_DATA_MSG             39
#define DBG_SAM_POWER_VOTE_INFO                40
#define DBG_SAM_MOTION_REG_INFO                41
#define DBG_SAM_MOTION_REG_FAIL                42
#define DBG_SAM_MOTION_IND_DROP                43
#define DBG_SAM_MOTION_INT_RESP                44
#define DBG_SAM_MOTION_INT_IND                 45
#define DBG_SAM_MOTION_INT_OCCUR               46
#define DBG_SAM_MOTION_STATE_UPDATE            47
#define DBG_SAM_MOTION_STATE_START             48
#define DBG_SAM_MOTION_STATE_STOP              49
#define DBG_SAM_REG_REQ_COUNT                  50
#define DBG_SAM_REG_REQ_SUCCESS                51
#define DBG_SAM_REG_REQ_FAIL                   52
#define DBG_SAM_REG_RESP_ERR                   53
#define DBG_SAM_REG_RESP_PROC_ERR              54
#define DBG_SAM_PROC_MSG_HDR_ERR               55
#define DBG_SAM_MSG_DROP                       56
#define DBG_SAM_MUTEX_ERR                      57
#define DBG_SAM_ALLOC_ERR                      58
#define DBG_SAM_SVC_HNDL_ERR                   59
#define DBG_SAM_QMI_REG_ERR                    60
#define DBG_SAM_EXT_CLNT_ID_ERR                61
#define DBG_SAM_INIT_SVC_ERR                   62
#define DBG_SAM_QCSI_CB                        63
#define DBG_SAM_QMI_ERR                        64
#define DBG_SAM_INS_ID_ERR                     65
#define DBG_SAM_MOTION_INT_CLIENT_INFO         66
#define DBG_SAM_NULL_PTR                       67
#define DBG_SAM_REG_ALGO_PENDING               68

/*=====================================================================
                 SENSOR CALIBRATION MANAGER (DSPS) IDENTIFIERS
=======================================================================*/
/* Important Note to add a new string:
 * Please Add your identifier after the last string
 */

//Format: DBG_<module_name>_<string identifier>
#define DBG_SCM_TIMER_CB_SIGNALERR              0
#define DBG_SCM_REG_TIMER_STARTED               1
#define DBG_SCM_REG_TIMER_FAILED                2
#define DBG_SCM_DEREG_TIMER_DELETED             3
#define DBG_SCM_DEREG_TIMER_FAILED              4
#define DBG_SCM_TASK_STARTED                    5
#define DBG_SCM_PROCESS_EVT_UNKWN_EVT           6
#define DBG_SCM_PROCESS_MSG_RCVD                7
#define DBG_SCM_PROCESS_MSG_STATUS              8
#define DBG_SCM_REQ_ALGO_SVC                    9
#define DBG_SCM_SMGR_RESP_TYPE_DROP             10
#define DBG_SCM_SMGR_RESP_DATA_DROP             11
#define DBG_SCM_SMGR_RESP_DROPPED               12
#define DBG_SCM_SMGR_RESP_SUCCESS               13
#define DBG_SCM_SMGR_RESP_ACK_VAL               14
#define DBG_SCM_SMGR_RESP_INFO                  15
#define DBG_SCM_SMGR_IND_DROPPED                16
#define DBG_SCM_SMGR_IND_INFO                   17
#define DBG_SCM_SMGR_IND_RATE_INFO              18
#define DBG_SCM_REQ_SNSR_DATA_INFO              19
#define DBG_SCM_REQ_SNSR_STATUS_INFO            20
#define DBG_SCM_SEND_RSP_INFOMSG                21
#define DBG_SCM_SMGR_IND_INVALID                22
#define DBG_SCM_SAM_RESP_INFO                   23
#define DBG_SCM_SAM_RESP_DROPPED                24
#define DBG_SCM_SAM_RESP_TYPE_DROP              25
#define DBG_SCM_GET_ALGO_INDX_ERR               26
#define DBG_SCM_REG_ALGO_ERR                    27
#define DBG_SCM_REG_ALGO_DFLT_ERR               28
#define DBG_SCM_ENABLE_ALGO_STATE_NULL          29
#define DBG_SCM_DISABLE_ALGO_INSTANCE_ERR       30
#define DBG_SCM_GYRO_CAL_REQ_INFO               31
#define DBG_SCM_LOG_CONFIG_ERR                  32
#define DBG_SCM_LOG_RESULT_ERR                  33
#define DBG_SCM_ALGO_MEM_INFO                   34
#define DBG_SCM_ALGO_STATE_MEM_INFO             35
#define DBG_SCM_SAM_IND_INFO                    36
#define DBG_SCM_STOP_SNSR_DATA                  37
#define DBG_SCM_ENABLE_ALGO                     38
#define DBG_SCM_DISABLE_ALGO                    39
#define DBG_SCM_REG_REQ_COUNT                   40
#define DBG_SCM_REG_REQ_SUCCESS                 41
#define DBG_SCM_REG_REQ_FAIL                    42
#define DBG_SCM_REG_RESP_ERR                    43
#define DBG_SCM_REG_RESP_PROC_ERR               44
#define DBG_SCM_PROC_MSG_HDR_ERR                45
#define DBG_SCM_MSG_DROP                        46

/*=====================================================================
                 SMR STRING IDENTIFIERS
=======================================================================*/
/* Important Note to add a new string:
 * Please Add your identifier after the last string
 */

//Format: DBG_<module_name>_<string identifier>
#define DBG_SMR_MSG_HDR_DETAILS1             0
#define DBG_SMR_MSG_HDR_DETAILS2             1
#define DBG_SMR_MSG_HDR_DETAILS3             2
#define DBG_SMR_SEND_STATUS                  3
#define DBG_SMR_SEND_DEST_MODULE             4
#define DBG_SMR_SEND_GET_SRVC_OBJ            5
#define DBG_SMR_DST_MODULE                   6
#define DBG_SMR_ALLOC_ERR                    7
#define DBG_SMR_BODY_STR                     8
#define DBG_SMR_LOWMEM                       9

/*===========================================================================

  FUNCTION:   sns_diag_dsps_set_debug_mask

===========================================================================*/
/*!
  @brief
  Sets the debug mask on DSPS


  @param[i]
  msg_ptr: pointer to message containing debug mask that indicates which
           debug messages are enabled/disabled.

  @return
  none
*/
/*=========================================================================*/
void sns_diag_dsps_set_debug_mask(sns_diag_set_debug_mask_req_msg_v01* msg_ptr);

#endif /* SNS_DEBUG_API_H */

