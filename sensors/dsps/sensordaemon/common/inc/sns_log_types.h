#ifndef SNS_LOG_TYPES_H
#define SNS_LOG_TYPES_H

/*============================================================================

@file
  sns_log_types.h

@brief
  Contains type definitions for sensors logs.

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*=====================================================================
                 INCLUDE FILES
=======================================================================*/

#include <stddef.h>

#include "sns_common.h"

#include "log_codes.h"
#include "log.h"

#include "fixed_point.h"

/*=======================================================================
                  INTERNAL DEFINITIONS AND TYPES
========================================================================*/

/** Maximum size of payload */
#define SNS_LOG_MAX_SIZE  (2048 - 64)   /* Make this value to be same as SMR_MAX_BODY_LEN */

#define SNS_LOG_MAX_DATA_TYPES_PER_SENSOR 20

/** Sensors log packet mask type */
typedef uint64_t sns_log_mask_t;

/** Sensors internal log ids
 *  should be maintained in sync with SNS_LOG_QXDM_ID */

#define SNS_TIMESTAMP_DSPS  0
#define SNS_TIMESTAMP_APPS  1
#define SNS_TIMESTAMP_MODEM 2

#ifndef LOG_SNS_LATENCY_SAMPLING_POLL_C
#define LOG_SNS_LATENCY_SAMPLING_POLL_C  0x17FB
#endif

#ifndef LOG_SNS_LATENCY_SAMPLING_DRI_C
#define LOG_SNS_LATENCY_SAMPLING_DRI_C   0x17FC
#endif

#ifndef LOG_SNS_LATENCY_DELIVERY_C
#define LOG_SNS_LATENCY_DELIVERY_C       0x17FD
#endif

typedef enum {
   SNS_LOG_SENSOR1_REQUEST,
   SNS_LOG_SENSOR1_RESPONSE,
   SNS_LOG_SENSOR1_INDICATION,
   SNS_LOG_SMR_REQUEST,
   SNS_LOG_SMR_RESPONSE,
   SNS_LOG_SMR_INDICATION,
   SNS_LOG_CONVERTED_SENSOR_DATA,
   SNS_LOG_QMD_CONFIG,
   SNS_LOG_QMD_RESULT,
   SNS_LOG_QMD_REPORT,
   SNS_LOG_DSPS_PWR,
   SNS_LOG_RAW_I2C_DATA,
   SNS_LOG_FNS_CONFIG,
   SNS_LOG_FNS_RESULT,
   SNS_LOG_FNS_REPORT,
   SNS_LOG_BTE_CONFIG,
   SNS_LOG_BTE_RESULT,
   SNS_LOG_BTE_REPORT,
   SNS_LOG_GYRO_CAL_CONFIG,
   SNS_LOG_GYRO_CAL_RESULT,
   SNS_LOG_GYRO_CAL_REPORT,
   SNS_LOG_QUAT_CONFIG,
   SNS_LOG_QUAT_RESULT,
   SNS_LOG_QUAT_REPORT,
   SNS_LOG_GRAVITY_CONFIG,
   SNS_LOG_GRAVITY_RESULT,
   SNS_LOG_GRAVITY_REPORT,
   SNS_LOG_DRV_MD_IRQ,
   SNS_LOG_MAG_CAL_CONFIG,
   SNS_LOG_MAG_CAL_RESULT,
   SNS_LOG_MAG_CAL_REPORT,
   SNS_LOG_ROTATION_VECTOR_CONFIG,
   SNS_LOG_ROTATION_VECTOR_RESULT,
   SNS_LOG_ROTATION_VECTOR_REPORT,
   SNS_LOG_FMV_CONFIG,
   SNS_LOG_FMV_QUAT_UPDATE,
   SNS_LOG_FMV_MAG_UPDATE,
   SNS_LOG_FMV_GYRO_UPDATE,
   SNS_LOG_FMV_REPORT,
   SNS_LOG_TIME_REPORT,
   SNS_LOG_BASIC_GESTURES_CONFIG,
   SNS_LOG_BASIC_GESTURES_RESULT,
   SNS_LOG_BASIC_GESTURES_REPORT,
   SNS_LOG_FACING_CONFIG,
   SNS_LOG_FACING_RESULT,
   SNS_LOG_FACING_REPORT,
   SNS_LOG_INTEG_ANGLE_CONFIG,
   SNS_LOG_INTEG_ANGLE_REPORT,
   SNS_LOG_INTEG_ANGLE_RESULT,
   SNS_LOG_GYRO_TAP_CONFIG,
   SNS_LOG_GYRO_TAP_REPORT,
   SNS_LOG_GYRO_TAP_RESULT,
   SNS_LOG_ACCEL_CAL_CONFIG,
   SNS_LOG_ACCEL_CAL_RESULT,
   SNS_LOG_ACCEL_CAL_REPORT,
   SNS_LOG_PED_CONFIG,
   SNS_LOG_PED_RESULT,
   SNS_LOG_PED_REPORT,
   SNS_LOG_PAM_CONFIG,
   SNS_LOG_PAM_REPORT,
   SNS_LOG_PAM_RESULT,
   SNS_LOG_SENSOR_FIFO_EVENT,
   SNS_LOG_CMC_CONFIG,
   SNS_LOG_CMC_REPORT,
   SNS_LOG_CMC_RESULT,
   SNS_LOG_CMC_RESULT2,
   SNS_LOG_DISTANCE_BOUND_CONFIG,
   SNS_LOG_DISTANCE_BOUND_REPORT,
   SNS_LOG_DISTANCE_BOUND_RESULT,
   SNS_LOG_SMD_CONFIG,
   SNS_LOG_SMD_RESULT,
   SNS_LOG_SMD_REPORT,
   SNS_LOG_LATENCY_SAMPLING_POLL,
   SNS_LOG_LATENCY_SAMPLING_DRI,
   SNS_LOG_LATENCY_DELIVERY,
   SNS_LOG_GAME_ROTATION_VECTOR_CONFIG,
   SNS_LOG_GAME_ROTATION_VECTOR_RESULT,
   SNS_LOG_GAME_ROTATION_VECTOR_REPORT,
   SNS_LOG_NUM_IDS
} sns_log_id_e;

/** QXDM global log id
    should be maintained in sync with sns_log_id */
#define SNS_LOG_QXDM_ID             \
{                                   \
   LOG_SNS_SENSOR1_REQUEST_C,       \
   LOG_SNS_SENSOR1_RESPONSE_C,      \
   LOG_SNS_SENSOR1_INDICATION_C,    \
   LOG_SNS_SMR_REQUEST_C,           \
   LOG_SNS_SMR_RESPONSE_C,          \
   LOG_SNS_SMR_INDICATION_C,        \
   LOG_CONVERTED_SENSOR_DATA_C,     \
   LOG_SNS_QMD_CONFIG_C,            \
   LOG_SNS_QMD_RESULT_C,            \
   LOG_SNS_QMD_REPORT_C,            \
   LOG_SNS_DSPS_PWR_C,              \
   LOG_SNS_RAW_I2C_DATA_C,          \
   LOG_SNS_FNS_CONFIG_C,            \
   LOG_SNS_FNS_RESULT_C,            \
   LOG_SNS_FNS_REPORT_C,            \
   LOG_SNS_BTE_CONFIG_C,            \
   LOG_SNS_BTE_RESULT_C,            \
   LOG_SNS_BTE_REPORT_C,            \
   LOG_SNS_GYRO_SIMP_CAL_CONFIG_C,  \
   LOG_SNS_GYRO_SIMP_CAL_RESULT_C,  \
   LOG_SNS_GYRO_SIMP_CAL_REPORT_C,  \
   LOG_SNS_QUAT_CONFIG_C,           \
   LOG_SNS_QUAT_RESULT_C,           \
   LOG_SNS_QUAT_REPORT_C,           \
   LOG_SNS_GRAVITY_CONFIG_C,        \
   LOG_SNS_GRAVITY_RESULT_C,        \
   LOG_SNS_GRAVITY_REPORT_C,        \
   LOG_SNS_DRV_MD_IRQ_C,            \
   LOG_SNS_MAG_CAL_CONFIG_C,        \
   LOG_SNS_MAG_CAL_RESULT_C,        \
   LOG_SNS_MAG_CAL_REPORT_C,        \
   LOG_SNS_ROTATION_VECTOR_CONFIG_C,\
   LOG_SNS_ROTATION_VECTOR_RESULT_C,\
   LOG_SNS_ROTATION_VECTOR_REPORT_C,\
   LOG_SNS_FMV_CONFIG_C,            \
   LOG_SNS_FMV_QUAT_UPDATE_C,       \
   LOG_SNS_FMV_MAG_UPDATE_C,        \
   LOG_SNS_FMV_GYRO_UPDATE_C,       \
   LOG_SNS_FMV_REPORT_C,            \
   LOG_SNS_TIME_SERVICE_OFFSETS_C,  \
   LOG_SNS_BASIC_GESTURES_CONFIG_C, \
   LOG_SNS_BASIC_GESTURES_RESULT_C, \
   LOG_SNS_BASIC_GESTURES_REPORT_C, \
   LOG_SNS_FACING_CONFIG_C,         \
   LOG_SNS_FACING_RESULT_C,         \
   LOG_SNS_FACING_REPORT_C,         \
   LOG_INTEG_ANGLE_CONFIG_C,        \
   LOG_INTEG_ANGLE_REPORT_C,        \
   LOG_INTEG_ANGLE_RESULT_C,        \
   LOG_GTAP_CONFIG_C,               \
   LOG_GTAP_REPORT_C,               \
   LOG_GTAP_RESULT_C,               \
   LOG_SNS_ACCEL_CAL_CONFIG_C,      \
   LOG_SNS_ACCEL_CAL_RESULT_C,      \
   LOG_SNS_ACCEL_CAL_REPORT_C,      \
   LOG_SNS_PED_CONFIG_C,            \
   LOG_SNS_PED_RESULT_C,            \
   LOG_SNS_PED_REPORT_C,            \
   LOG_PAM_CONFIG_C,                \
   LOG_PAM_REPORT_C,                \
   LOG_PAM_RESULT_C,                \
   LOG_SENSOR_FIFO_EVENT_C,         \
   LOG_SNS_CMC_CONFIG_C,            \
   LOG_SNS_CMC_REPORT_C,            \
   LOG_SNS_CMC_RESULT_C,            \
   LOG_SNS_CMC_RESULT2_C,           \
   LOG_SNS_DB_CONFIG_C,             \
   LOG_SNS_DB_REPORT_C,             \
   LOG_SNS_DB_RESULT_C,             \
   LOG_SNS_SMD_CONFIG_C,            \
   LOG_SNS_SMD_RESULT_C,            \
   LOG_SNS_SMD_REPORT_C,            \
   LOG_SNS_LATENCY_SAMPLING_POLL_C, \
   LOG_SNS_LATENCY_SAMPLING_DRI_C,  \
   LOG_SNS_LATENCY_DELIVERY_C,      \
   LOG_SNS_GAME_ROTATION_VECTOR_CONFIG_C,  \
   LOG_SNS_GAME_ROTATION_VECTOR_RESULT_C,  \
   LOG_SNS_GAME_ROTATION_VECTOR_REPORT_C,  \
}

/** Log Packet Type -- types defined in log_codes.h */
typedef sns_log_id_e log_pkt_t;

/**
   Enum defining which sensor1 API is being logged.
   For use in sns_log_sensor1_request_s.
 */
typedef enum {
  SNS_LOG_SENSOR1_API_OPEN  = 1,
  SNS_LOG_SENSOR1_API_WRITE = 2,
  SNS_LOG_SENSOR1_API_CLOSE = 3
} sns_log_sensor1_api_e;

#if defined(SNS_PCSIM) || defined(_WIN32)
#pragma pack(push,1)
#endif

/**
   sns_log_sensor1_request_s
   This type is used to log function calls to sensor1_open, sensor1_write,
   and sensor1_close.

   The usage of values below may change depending on the function type.

   It is used for the following log types:
   LOG_SNS_SENSOR1_REQUEST_C
 */

/** Version of the sensor1 logging structures */
#define SNS_LOG_STRUCT_VERSION 2

typedef PACK(struct) {
  log_hdr_type   log_type;          /**< Type, as defined in log_codes.h */
  uint8_t        version;           /**< Set to SNS_LOG_STRUCT_VERSION */
  uint8_t        logging_processor; /**< 1 == Modem; 2 == Apps; 3 == DSPS */
  uint64_t       timestamp;         /**< Timestamp the log was created */
  uint8_t        sensor1_fn;        /**< As defined in sns_log_sensor1_api_e */
  int32_t        sensor1_rv;        /**< Return value from sensor1 API call */
  uint8_t        ext_clnt_id;      /**< External client ID assigned by ACM */
  uint64_t       cb_data;           /**< Callback data assigned to this client */
  uint64_t       context_handle;    /**< Unique handle identifying an external
                                       client.
                                       Assigned when the client calls
                                       sensor1_open */
  uint8_t        svc_num;           /**< QMI Service number
                                       Only valid when sensor1_fn == 2
                                       (sensor1_write) */
  uint16_t       msg_id;            /**< Message ID, as defined by the service */
  uint8_t        msg_type;          /**< Message Type: Response
                                       Only valid when sensor1_fn == 2
                                       (sensor1_write) */
  uint8_t        txn_id;            /**< Transaction ID chosen by the client */
  uint16_t       request_size;      /**< Size of the request field.
                                       Only valid when sensor1_fn == 2
                                       (sensor1_write).  */
  uint8_t        request[1];        /**< The QMI-encoded message, of length
                                       request_size.
                                       Only valid when sensor1_fn == 2
                                       (sensor1_write) */
} sns_log_sensor1_request_s;


/**
   sns_log_sensor1_resp_ind_s
   This type is used to log response and indication packets sent to clients via
   the sensor1 API.

   It is used for the following log types:
   LOG_SNS_SENSOR1_RESPONSE_C
   LOG_SNS_SENSOR1_INDICATION_C
 */
typedef PACK(struct) {
  log_hdr_type   log_type;          /**< Type, as defined in log_codes.h */
  uint8_t        version;           /**< Set to SNS_LOG_STRUCT_VERSION */
  uint8_t        logging_processor; /**< 1 == Modem; 2 == Apps; 3 == DSPS */
  uint64_t       timestamp;         /**< Timestamp the log was created */
  uint64_t       cb_data;           /**< Callback data assigned to this client */
  uint64_t       context_handle;    /**< Unique handle identifying an external
                                       client.
                                       Assigned when the client calls
                                       sensor1_open */
  uint8_t        ext_clnt_id;       /**< External client ID assigned by ACM */
  uint8_t        svc_num;           /**< QMI Service number */
  uint16_t       msg_id;            /**< Message ID, as defined by the service */
  uint8_t        msg_type;          /**< Message Type: Response or Indication */
  uint8_t        txn_id;            /**< Transaction ID chosen by the client */
  uint16_t       resp_ind_size;     /**< Size of the resp_ind field. */
  uint8_t        resp_ind[1];       /**< The QMI-encoded response or indication,
                                       of length resp_ind_size. */
} sns_log_sensor1_resp_ind_s;


/**
   sns_log_smr_pkt_s
   This type is used to log requests routed through the Sensor Message
   Router.
   It is used for the following log types:
   LOG_SNS_SMR_REQUEST_C
   LOG_SNS_SMR_RESPONSE_C
   LOG_SNS_SMR_INDICATION_C
 */
typedef PACK(struct) {
  log_hdr_type   log_type;          /**< Type, as defined in log_codes.h */
  uint8_t        version;           /**< Set to SNS_LOG_STRUCT_VERSION */
  uint8_t        logging_processor; /**< 1 == Modem; 2 == Apps; 3 == DSPS */
  uint64_t       timestamp;         /**< Timestamp the log was created */
  uint8_t        dst_module;        /**< SMR Destination Module, defined in
                                       sns_common.h */
  uint8_t        src_module;        /**< SMR Source Module, defined in
                                       sns_common.h */
  uint8_t        priority;          /**< Message Priority. */
  uint8_t        txn_id;            /**< Transaction ID chosen by the client */
  uint8_t        ext_client_id;     /**< External client ID */
  uint8_t        svc_num;           /**< QMI Service number */
  uint16_t       msg_id;            /**< Message ID, as defined by the service */
  uint8_t        msg_type;          /**< Message Type */
  uint16_t       pkt_size;          /**< Size of the pkt field. */
  uint8_t        pkt[1];            /**< The QMI-encoded message, of length
                                       pkt_size. */
} sns_log_smr_pkt_s;

/**
   sns_log_sensor_data_pkt_s
   This type is used to log sensor data as reported to the sensors manager.
   It is used for the following log types:
   LOG_CONVERTED_SENSOR_DATA_C
 */
#define SNS_LOG_SENSOR_DATA_PKT_VERSION 3
typedef PACK(struct) {
  log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
  uint8_t      version;          /**< version of the log packet */
  uint64_t     timestamp;        /**< time when the sensor data was sampled */
  uint32_t     sensor_id;        /**< type of sensor being logged smgr sensor id */
  uint8_t      vendor_id;        /**< vendor for the sensor device */
  uint8_t      num_data_types;   /**< sample dimension being logged */
  int32_t      data[SNS_LOG_MAX_DATA_TYPES_PER_SENSOR]; /**< sensor data */
  uint64_t     end_timestamp;    /**< end  time when the last sensor data was
                                      sampled in this FIFO batch */
  uint16_t     num_samples;      /**< Number of samples in this log, if num_sample is 0
                                     ( to be backward compatible with older version )or 1,
                                      put sample in data field; if num_sample is 1, put
                                      sample in both data and samples */
  int32_t      samples[1];       /**< Size will depend on num_samples, fifo
                                      samples will be cascaded here, the axis info
                                      is represented by num_data_types
                                      bytes=(num_samples*num_data_types*4) */
} sns_log_sensor_data_pkt_s;

/**
   sns_log_qmd_config_s
   This type logs motion detection algorithm configuration
   LOG_SNS_QMD_CONFIG_C
 */
#define SNS_LOG_QMD_CONFIG_VERSION 2
typedef PACK(struct) {
   log_hdr_type log_type;        /**< Type, as defined in log_codes.h */
   uint8_t      version;         /**< version of the log packet */
   uint64_t     timestamp;       /**< time when the sensor data was sampled */
   int32_t      sample_rate;     /**< accelerometer sampling rate */
   uint8_t      enable_amd;      /**< enable absolute motion detection algorithm */
   uint8_t      enable_vmd;      /**< enable vehicle motion detection algorithm */
   uint8_t      enable_rmd;      /**< enable relative motion detection algorithm */
   uint8_t      algo_inst_id;    /**< algorithm instance id */
   q16_t        sensor_report_rate; /**< Requested sensor report rate in Hz, Q16 */
} sns_log_qmd_config_s;

/**
   sns_log_qmd_result_s
   This type logs motion detection algorithm result
   LOG_SNS_QMD_RESULT_C
 */
#define SNS_LOG_QMD_RESULT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;        /**< Type, as defined in log_codes.h */
   uint8_t      version;         /**< version of the log packet */
   uint64_t     timestamp;       /**< time when the sensor data was sampled */
   int32_t      accel_x;         /**< acceleration along x axis */
   int32_t      accel_y;         /**< acceleration along y axis */
   int32_t      accel_z;         /**< acceleration along z axis */
   uint8_t      amd_result;      /**< absolute motion detection algorithm result */
   uint8_t      vmd_result;      /**< vehicle motion detection algorithm result */
   uint8_t      rmd_result;      /**< relative motion detection algorithm result */
   uint8_t      algo_inst_id;    /**< algorithm instance identifier */
} sns_log_qmd_result_s;

/**
   sns_log_qmd_report_s
   This type logs motion detection algorithm client report
   LOG_SNS_QMD_REPORT_C
 */
#define SNS_LOG_QMD_REPORT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;        /**< Type, as defined in log_codes.h */
   uint8_t      version;         /**< version of the log packet */
   uint64_t     timestamp;       /**< time when the sensor data was sampled */
   uint32_t     report_id;       /**< report identifier */
   uint8_t      report_type;     /**< report type */
   uint8_t      client_id;       /**< client identifier */
   uint8_t      qmd_result;      /**< motion detection algorithm result */
   uint8_t      algo_service_id; /**< algorithm service identifier */
   uint8_t      algo_inst_id;    /**< algorithm instance identifier */
} sns_log_qmd_report_s;

/**
 * sns_log_qmd_irq_s
 * This type logs motion detection interrupt
 * LOG_SNS_QMD_IRQ
 */
#define SNS_LOG_MD_IRQ_VERSION 1
typedef PACK(struct) {
    log_hdr_type  log_type;      /**< Type, as defined in log_codes.h */
    uint8_t       version;       /**< version of the log packet */
    uint64_t      timestamp;     /**< time when the interrupt was detected */
    uint16_t      threshold;     /**< threshold, in milliG for motion detect */
} sns_log_md_irq_s;

/**
   sns_log_pwr_report_s
   This type logs power on/off state of the DSPS
   SNS_LOG_DSPS_PWR
*/
#ifndef DSPS_RPM_SUPPORT
/* No RPM support */
  #define SNS_LOG_DSPS_PWR_VERSION 1

/* These defines are used for the Power State */
  #define SNS_LOG_DSPS_PWR_ST_OFF 0
  #define SNS_LOG_DSPS_PWR_ST_ON  1
#else
/* RPM support */
  #define SNS_LOG_DSPS_PWR_VERSION 2

/* These defines are used for the Power State */
  #define SNS_LOG_DSPS_PWR_ST_HIBERNATION 0
  #define SNS_LOG_DSPS_PWR_ST_DORMANT     1
  #define SNS_LOG_DSPS_PWR_ST_LOW         2
  #define SNS_LOG_DSPS_PWR_ST_HIGH        3
  #define SNS_LOG_DSPS_PWR_ST_UNKNOWN     4
#endif /* DSPS_RPM_SUPPORT */


typedef PACK(struct) {
   log_hdr_type log_type;        /**< Type, as defined in log_codes.h */
   uint8_t      version;         /**< version of the log packet */
   uint64_t     timestamp;       /**< time when the state changed */
   uint32_t     powerstate;      /**< Power state, as defined above */
} sns_log_dsps_pwr_s;

/**
   sns_log_raw_i2c_data_s
   This type is used to log the data for I2C read or write operation.
   It is used for the following log types:
   LOG_SNS_RAW_I2C_DATA_C
 */
#define SNS_LOG_RAW_I2C_DATA_VERSION 1
#define SNS_LOG_MAX_RAW_I2C_PAYLOAD 57
typedef PACK(struct) {
  log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
  uint8_t      version;          /**< version of the log packet */
  uint64_t     timestamp;        /**< time when the I2C transcation completed */
  uint8_t      i2c_op;           /**< i2c operation code */
  uint8_t      status;           /**< i2c operation status */
  uint8_t      slv_addr;         /**< i2c slave device address */
  uint8_t      reg_addr;         /**< i2c register address for the operation */
  uint8_t      data_len;         /**< length of i2c data payload */
  uint8_t      data_buf[SNS_LOG_MAX_RAW_I2C_PAYLOAD]; /**< sensor data */
} sns_log_raw_i2c_data_s;

#define SNS_LOG_FNS_CONFIG_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;            /* Version                */
  uint8_t      timestamp_type;     /* type of timestamp      */
  uint64_t     timestamp;          /* timestamp              */
  uint32_t     sample_rate;        /* Sampling rate          */
  uint32_t     facing_angle_thresh;/* facing angle threshold */
  uint32_t     basic_sleep;        /* basic sleep param      */
  uint32_t     basic_shake_thresh; /* basic shake threshold  */
  uint32_t     timeout;            /* Timeout for algo       */
  uint32_t     ic_param_1;         /* internal config param  */
  uint32_t     ic_param_2;         /* internal config param  */
  uint32_t     ic_param_3;         /* internal config param  */
  uint32_t     ic_param_4;         /* internal config param  */
  uint8_t      algo_instance_id;   /* algorithm instance id  */
} sns_log_fns_config_s;



#define SNS_LOG_FNS_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;            /* Version                */
  uint8_t      timestamp_type;     /* type of timestamp      */
  uint64_t     timestamp;          /* timestamp              */

  int32_t      accel_x;           /* acceleration along x axis */
  int32_t      accel_y;           /* acceleration along y axis */
  int32_t      accel_z;           /* acceleration along z axis */
  uint8_t      fns_result;        /* algo result               */
  uint8_t      fns_state;         /* algo state                */
  uint8_t      axis_state;        /* axis algo state           */
  uint8_t      algo_inst_id;      /* algo instance id          */
} sns_log_fns_result_s;

#define SNS_LOG_FNS_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;            /* Version                */
  uint8_t      timestamp_type;     /* type of timestamp      */
  uint64_t     timestamp;          /* timestamp              */

  int32_t      report_id;         /* report id               */
  uint8_t      report_type;       /* report type             */
  uint8_t      client_id;         /* client id               */
  uint8_t      fns_result;        /* algo result             */
  uint8_t      algo_inst_id;      /* algo instance id        */
} sns_log_fns_report_s;

#define SNS_LOG_BTE_CONFIG_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;            /* Version                    */
  uint8_t      timestamp_type;     /* type of timestamp          */
  uint64_t     timestamp;          /* timestamp                  */
  uint32_t     sample_rate;        /* Sampling rate              */
  uint32_t     facing_angle_thresh;/* facing angle threshold     */
  uint32_t     horiz_angle_thresh; /* horizontal angle threshold */
  uint32_t     vert_angle_thresh;  /* vertical angle threshold   */
  uint32_t     prox_enabled;       /* proximity enabled          */
  uint32_t     ic_param_1;         /* internal config param      */
  uint32_t     ic_param_2;         /* internal config param      */
  uint32_t     ic_param_3;         /* internal config param      */
  uint32_t     ic_param_4;         /* internal config param      */
  uint8_t      algo_instance_id;   /* algorithm instance id      */
} sns_log_bte_config_s;

#define SNS_LOG_BTE_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /* Version                   */
  uint8_t      timestamp_type;    /* type of timestamp         */
  uint64_t     timestamp;         /* timestamp                 */

  int32_t      accel_x;           /* acceleration along x axis */
  int32_t      accel_y;           /* acceleration along y axis */
  int32_t      accel_z;           /* acceleration along z axis */
  int32_t      proximity;         /* proximity                 */
  uint8_t      bte_result;        /* algo result               */
  uint8_t      bte_state;         /* algo state                */
  uint8_t      axis_state;        /* axis algo state           */
  uint8_t      algo_inst_id;      /* algo instance id          */
} sns_log_bte_result_s;

#define SNS_LOG_BTE_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /* Version                 */
  uint8_t      timestamp_type;    /* type of timestamp       */
  uint64_t     timestamp;         /* timestamp               */

  int32_t      report_id;         /* report id               */
  uint8_t      report_type;       /* report type             */
  uint8_t      client_id;         /* client id               */
  uint8_t      bte_result;        /* algo result             */
  uint8_t      algo_inst_id;      /* algo instance id        */
} sns_log_bte_report_s;

#define SNS_LOG_GYRO_CAL_CONFIG_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;        /**< Type, as defined in log_codes.h */
   uint8_t      version;         /**< version of the log packet */
   uint64_t     timestamp;       /**< time when the sensor data was sampled */
   int32_t      sample_rate;     /**< gyroscope sampling rate */
   uint8_t      algo_inst_id;    /**< algorithm instance id */
} sns_log_gyro_cal_config_s;

#define SNS_LOG_GYRO_CAL_RESULT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
   uint8_t      version;          /**< version of the log packet */
   uint64_t     timestamp;        /**< time when sensor data was sampled */
   int32_t      gyro_x;           /**< gyro measurement along x axis */
   int32_t      gyro_y;           /**< gyro measurement along y axis */
   int32_t      gyro_z;           /**< gyro measurement along z axis */
   int32_t      gyro_zero_bias_x; /**< gyro x axis zero bias estimate */
   int32_t      gyro_zero_bias_y; /**< gyro y axis zero bias estimate */
   int32_t      gyro_zero_bias_z; /**< gyro z axis zero bias estimate */
   uint8_t      algo_inst_id;     /**< algorithm instance id */
} sns_log_gyro_cal_result_s;

#define SNS_LOG_GYRO_CAL_REPORT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;              /**< Type, as defined in log_codes.h */
   uint8_t      version;               /**< version of the log packet */
   uint64_t     timestamp;             /**< time when sensor data sampled */
   int32_t      gyro_x;                /**< gyro measurement along x axis */
   int32_t      gyro_y;                /**< gyro measurement along y axis */
   int32_t      gyro_z;                /**< gyro measurement along z axis */
   int32_t      gyro_zero_bias_corr_x; /**< gyro x axis zero bias correction */
   int32_t      gyro_zero_bias_corr_y; /**< gyro y axis zero bias correction */
   int32_t      gyro_zero_bias_corr_z; /**< gyro z axis zero bias correction */
} sns_log_gyro_cal_report_s;

#define SNS_LOG_ACCEL_CAL_CONFIG_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;        /**< Type, as defined in log_codes.h */
   uint8_t      version;         /**< version of the log packet */
   uint64_t     timestamp;       /**< time when the sensor data was sampled */
   int32_t      sample_rate;     /**< gyroscope sampling rate */
   uint8_t      algo_inst_id;    /**< algorithm instance id */
} sns_log_accel_cal_config_s;

#define SNS_LOG_ACCEL_CAL_RESULT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
   uint8_t      version;          /**< version of the log packet */
   uint64_t     timestamp;        /**< time when sensor data was sampled */
   int32_t      accel_x;           /**< accel measurement along x axis */
   int32_t      accel_y;           /**< accel measurement along y axis */
   int32_t      accel_z;           /**< accel measurement along z axis */
   int32_t      temp;              /**< temp measurement */
   int32_t      accel_zero_bias_x; /**< accel x axis zero bias estimate */
   int32_t      accel_zero_bias_y; /**< accel y axis zero bias estimate */
   int32_t      accel_zero_bias_z; /**< accel z axis zero bias estimate */
   int32_t      accel_scale_factor_x; /**< accel x axis scale factor estimate */
   int32_t      accel_scale_factor_y; /**< accel y axis scale factor estimate */
   int32_t      accel_scale_factor_z; /**< accel z axis scale factor estimate */
   uint8_t      algo_inst_id;     /**< algorithm instance id */
} sns_log_accel_cal_result_s;

#define SNS_LOG_ACCEL_CAL_REPORT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;              /**< Type, as defined in log_codes.h */
   uint8_t      version;               /**< version of the log packet */
   uint64_t     timestamp;             /**< time when sensor data sampled */
   int32_t      accel_x;                /**< accel measurement along x axis */
   int32_t      accel_y;                /**< accel measurement along y axis */
   int32_t      accel_z;                /**< accel measurement along z axis */
   int32_t      temp;              /**< temp measurement */
   int32_t      accel_zero_bias_x; /**< accel x axis zero bias estimate */
   int32_t      accel_zero_bias_y; /**< accel y axis zero bias estimate */
   int32_t      accel_zero_bias_z; /**< accel z axis zero bias estimate */
   int32_t      accel_scale_factor_x; /**< accel x axis scale factor estimate */
   int32_t      accel_scale_factor_y; /**< accel y axis scale factor estimate */
   int32_t      accel_scale_factor_z; /**< accel z axis scale factor estimate */
   double       cal_report_decision; /**< 1:report sent to Smgr 0: report not sent */
   double       temp_op_min; /**< [deg] lower limit of the current operational temperature range*/
   double       count_kf_update; /**< number of KF updates for the current operational bin*/
   double       covar_accel_x_bias; /**< [milliG^2] KF covariance for Accel X Bias*/
   double       covar_accel_y_bias; /**< [milliG^2] KF covariance for Accel Y Bias*/
   double       covar_accel_z_bias; /**< [milliG^2] KF covariance for Accel Z Bias*/
   double       covar_accel_x_sf; /**< [milliG^2] KF covariance for Accel X SF*/
   double       covar_accel_y_sf; /**< [milliG^2] KF covariance for Accel Y SF*/
   double       covar_accel_z_sf; /**< [milliG^2] KF covariance for Accel Z SF*/
   double       kf_accel_x_bias; /**< [milliG] KF Bias X Estimate*/
   double       kf_accel_y_bias; /**< [milliG] KF Bias Y Estimate*/
   double       kf_accel_z_bias; /**< [milliG] KF Bias Z Estimate*/
   double       kf_accel_x_sf; /**< [milliG] KF SF X Estimate*/
   double       kf_accel_y_sf; /**< [milliG] KF SF Y Estimate*/
   double       kf_accel_z_sf; /**< [milliG] KF SF Z Estimate*/
} sns_log_accel_cal_report_s;

#define SNS_LOG_QUATERNION_CONFIG_VERSION 2
typedef PACK(struct) {
   log_hdr_type log_type;              /**< Type, as defined in log_codes.h */
   uint8_t      version;               /**< Version of the log packet       */
   uint8_t      timestamp_type;        /**< type of timestamp               */
   uint64_t     timestamp;             /**< timestamp of quaternion config  */
   uint32_t     report_period;         /**< quaternion report period        */
   uint32_t     sample_rate;           /**< gyroscope sampling rate         */
   uint8_t      algo_inst_id;          /**< algorithm instance id           */
   q16_t        sensor_report_rate; /**< Requested sensor report rate in Hz, Q16 */
} sns_log_quaternion_config_s;

#define SNS_LOG_QUATERNION_RESULT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
   uint8_t      version;          /**< Version of the log packet       */
   uint8_t      timestamp_type;   /**< type of timestamp               */
   uint64_t     timestamp;        /**< timestamp of quaternion result  */
   int32_t      gyro_x;           /**< gyro measurement along x_axis   */
   int32_t      gyro_y;           /**< gyro measurement along y_axis   */
   int32_t      gyro_z;           /**< gyro measurement along z_axis   */
   float        quaternion[4];    /**< quaternion vector result        */
   int32_t      delta_t;          /**< time interval of gyro samples   */
   uint8_t      algo_inst_id;     /**< algorithm instance id           */
} sns_log_quaternion_result_s;

#define SNS_LOG_QUATERNION_REPORT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
   uint8_t      version;          /**< Version of the log packet       */
   uint8_t      timestamp_type;   /**< type of timestamp               */
   uint64_t     timestamp;        /**< timestamp of quaternion report  */
   int32_t      report_id;        /**< report id                       */
   uint8_t      report_type;      /**< report type                     */
   uint8_t      client_id;        /**< client id                       */
   float        quaternion[4];    /**< quaternion vector result        */
   uint8_t      algo_inst_id;     /**< algorithm instance id           */
}sns_log_quaternion_report_s;

#define SNS_LOG_GRAVITY_CONFIG_VERSION 2
#define SNS_LOG_ORIENTATION_GRAVITY_CONFIG_VERSION 100
typedef PACK(struct) {
   log_hdr_type log_type;              /**< Type, as defined in log_codes.h */
   uint8_t      version;               /**< Version of the log packet       */
   uint8_t      timestamp_type;        /**< type of timestamp               */
   uint64_t     timestamp;             /**< timestamp of gravity config     */
   uint32_t     report_period;         /**< gravity vector report period    */
   uint32_t     sample_rate;           /**< accel/gyro sampling rate        */
   uint32_t     time_const_abs;        /**< absolute rest time constant     */
   uint32_t     time_const_rel;        /**< relative rest time constant     */
   uint32_t     time_const_mot;        /**< motion time constant            */
   uint8_t      algo_inst_id;          /**< algorithm instance id           */
   q16_t        sensor_report_rate; /**< Requested sensor report rate in Hz, Q16 */
} sns_log_gravity_config_s;

#define SNS_LOG_GRAVITY_RESULT_VERSION 1
#define SNS_LOG_ORIENTATION_GRAVITY_RESULT_VERSION 100
typedef PACK(struct) {
   log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
   uint8_t      version;          /**< Version of the log packet       */
   uint8_t      timestamp_type;   /**< type of timestamp               */
   uint64_t     timestamp;        /**< timestamp of gravity result     */
   float        quaternion_x;     /**< quaternion x_axis               */
   float        quaternion_y;     /**< quaternion y_axis               */
   float        quaternion_z;     /**< quaternion z_axis               */
   int32_t      accel_x;          /**< accel measurement along x_axis  */
   int32_t      accel_y;          /**< accel measurement along y_axis  */
   int32_t      accel_z;          /**< accel measurement along z_axis  */
   uint8_t      abs_rest_state;   /**< unknown=0, rest=1, motion=2     */
   uint8_t      rel_rest_state;   /**< unknown=0, rest=1, motion=2     */
   float        gravity[3];       /**< gravity vector result           */
   float        lin_accel[3];     /**< linear accelaration result      */
   int32_t      delta_t;          /**< time interval of gyro samples   */
   int32_t      alpha;            /**< smoothing factor for gravity    */
   uint8_t      algo_inst_id;     /**< algorithm instance id           */
} sns_log_gravity_result_s;

#define SNS_LOG_GRAVITY_REPORT_VERSION 1
#define SNS_LOG_ORIENTATION_GRAVITY_REPORT_VERSION 100
typedef PACK(struct) {
   log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
   uint8_t      version;          /**< Version of the log packet       */
   uint8_t      timestamp_type;   /**< type of timestamp               */
   uint64_t     timestamp;        /**< timestamp of gravity report     */
   int32_t      report_id;        /**< report id                       */
   uint8_t      report_type;      /**< report type                     */
   uint8_t      client_id;        /**< client id                       */
   float        gravity[3];       /**< gravity vector result           */
   float        lin_accel[3];     /**< linear accelaration result      */
   uint8_t      algo_inst_id;     /**< algorithm instance id           */
}sns_log_gravity_report_s;

#define SNS_LOG_MAG_CAL_CONFIG_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;              /**< Type, as defined in log_codes.h */
   uint8_t      version;               /**< Version of the log packet       */
   uint8_t      timestamp_type;        /**< type of timestamp               */
   uint64_t     timestamp;             /**< timestamp of mag cal config  */
   uint32_t     report_period;         /**< gravity vector report period    */
   uint32_t     sample_rate;           /**< mag sampling rate         */
   uint8_t      algo_inst_id;          /**< algorithm instance id           */
} sns_log_mag_cal_config_s;

#define SNS_LOG_MAG_CAL_RESULT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
   uint8_t      version;          /**< Version of the log packet       */
   uint8_t      timestamp_type;   /**< type of timestamp               */
   uint64_t     timestamp;        /**< timestamp of quaternion result  */
   int32_t      mag_x;            /**< raw mag measurement along x_axis   */
   int32_t      mag_y;            /**< raw mag measurement along y_axis   */
   int32_t      mag_z;            /**< raw mag measurement along z_axis   */
   int32_t      magcal[3];        /**< calibrated mag result        */
   int32_t      magcalbias[3];    /**< bias offset result        */
   uint8_t      accuracy;         /**< accuracy of the calibrated sample */
   uint8_t      algo_inst_id;     /**< algorithm instance id           */
} sns_log_mag_cal_result_s;

#define SNS_LOG_MAG_CAL_REPORT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;         /**< Type, as defined in log_codes.h */
   uint8_t      version;          /**< Version of the log packet       */
   uint8_t      timestamp_type;   /**< type of timestamp               */
   uint64_t     timestamp;        /**< timestamp of quaternion report  */
   int32_t      report_id;        /**< report id                       */
   uint8_t      report_type;      /**< report type                     */
   uint8_t      client_id;        /**< client id                       */
   int32_t      magcal[3];        /**< calibrated mag result        */
   int32_t      magcalbias[3];    /**< bias offset result        */
   uint8_t      accuracy;         /**< accuracy of the calibrated sample */
   uint8_t      algo_inst_id;     /**< algorithm instance id           */
}sns_log_mag_cal_report_s;

#define SNS_LOG_ROTATION_VECTOR_CONFIG_VERSION 2
#define SNS_LOG_ORIENTATION_ROTVEC_CONFIG_VERSION 100
typedef PACK(struct) {
   log_hdr_type log_type;              /**< Type, as defined in log_codes.h     */
   uint8_t      version;               /**< Version of the log packet           */
   uint8_t      timestamp_type;        /**< type of timestamp                   */
   uint64_t     timestamp;             /**< timestamp of gravity config         */
   uint32_t     report_period;         /**< gravity vector report period        */
   uint32_t     sample_rate;           /**< Gravity Vector/Magnetic Vector sampling rate  */
   uint8_t      coordinate_sys;        /**< coordinate system used in the ouput */
   uint8_t      algo_inst_id;          /**< algorithm instance id               */
   q16_t        sensor_report_rate; /**< Requested sensor report rate in Hz, Q16 */
} sns_log_rotation_vector_config_s;

#define SNS_LOG_ROTATION_VECTOR_RESULT_VERSION 1
#define SNS_LOG_ORIENTATION_ROTVEC_RESULT_VERSION 100
typedef PACK(struct) {
   log_hdr_type log_type;                 /**< Type, as defined in log_codes.h      */
   uint8_t      version;                  /**< Version of the log packet            */
   uint8_t      timestamp_type;           /**< type of timestamp                    */
   uint64_t     timestamp;                /**< timestamp of rotation vector result  */
   float        gravity[3];               /**< gravity vector input                 */
   uint64_t     gravity_timestamp;        /**< gravity vector timestamp             */
   uint8_t      gravity_accuracy;         /**< gravity vector accuracy              */
   float        filtered_mag[3];          /**< magnetic vector input                */
   uint64_t     filtered_mag_timestamp;   /**< magnetic vector timestamp            */
   uint8_t      filtered_mag_accuracy;    /**< magnetic vector accuracy             */
   float        rotation_vector[4];       /**< rotation vector result               */
   uint8_t      accuracy;                 /**< rotation vector accuracy             */
   uint8_t      coordinate_sys;           /**< coordinate system used in the ouput  */
   uint8_t      algo_inst_id;             /**< algorithm instance id                */
} sns_log_rotation_vector_result_s;

#define SNS_LOG_ROTATION_VECTOR_REPORT_VERSION 1
#define SNS_LOG_ORIENTATION_ROTVEC_REPORT_VERSION 100
typedef PACK(struct) {
   log_hdr_type log_type;                 /**< Type, as defined in log_codes.h      */
   uint8_t      version;                  /**< Version of the log packet            */
   uint8_t      timestamp_type;           /**< type of timestamp                    */
   uint64_t     timestamp;                /**< timestamp of rotation vector report  */
   int32_t      report_id;                /**< report id                            */
   uint8_t      report_type;              /**< report type                          */
   uint8_t      client_id;                /**< client id                            */
   float        rotation_vector[4];       /**< rotation vector result               */
   uint8_t      accuracy;                 /**< rotation vector accuracy             */
   uint8_t      coordinate_sys;           /**< coordinate system used in the ouput  */
   uint8_t      algo_inst_id;             /**< algorithm instance id                */
}sns_log_rotation_vector_report_s;

#define SNS_LOG_FMV_CONFIG_VERSION     2
#define SNS_FMV_LOG_NUM_ACCURACY_BINS  4
typedef PACK(struct) {
   log_hdr_type log_type;                 /**< Type, as defined in log_codes.h      */
   uint8_t      version;                  /**< Version of the log packet            */
   uint8_t      timestamp_type;           /**< type of timestamp                    */
   uint64_t     timestamp;                /**< timestamp                            */
   uint32_t     report_period;            /**< Report Period - Q16 format           */
   uint32_t     sample_rate;              /**< Samping Rate - 0 ==> Invalid         */
   uint8_t      num_accuracy_bins;        /**< Number of accuracy bins              */
   float        accuracy_bin_tcs[SNS_FMV_LOG_NUM_ACCURACY_BINS];/**< filter tcs      */
   float        gyro_gap_thresh;          /**< Gyro delta thresh to declare timeout */
   float        mag_gap_thresh;           /**< magn delta thresh to declare timeout */
   float        min_turn_rate;            /**< Turn rate threshold to declare ZUPT  */
   float        max_mag_innovation;       /**< maximum innovation allowed           */
   uint8_t      algo_inst_id;             /**< algorithm instance id                */
   q16_t        sensor_report_rate;    /**< Requested sensor report rate in Hz, Q16 */
}sns_log_fmv_config_s;

#define SNS_LOG_FMV_QUAT_UPDATE_VERSION     1
typedef PACK(struct) {
   log_hdr_type log_type;                 /**< Type, as defined in log_codes.h      */
   uint8_t      version;                  /**< Version of the log packet            */
   uint8_t      timestamp_type;           /**< type of timestamp                    */
   uint64_t     timestamp_quat;           /**< timestamp of the quaternion input    */
   uint64_t     timestamp;                /**< timestamp of the fmv output          */
   float        q[4];                     /**< input quaternion                     */
   float        fmv[3];                   /**< filtered magnetic vector             */
   uint8_t      accuracy;                 /**< algo instance id                     */
   uint8_t      algo_inst_id;             /**< algorithm instance id                */
}sns_log_fmv_quat_update_s;

#define SNS_LOG_FMV_MAG_UPDATE_VERSION     1
typedef PACK(struct) {
   log_hdr_type log_type;                 /**< Type, as defined in log_codes.h      */
   uint8_t      version;                  /**< Version of the log packet            */
   uint8_t      timestamp_type;           /**< type of timestamp                    */
   uint64_t     timestamp_mag;            /**< timestamp of the mag input           */
   uint64_t     timestamp;                /**< timestamp of fmv output              */
   float        gyro[3];                  /**< input gyro                           */
   float        b[3];                     /**< input mag                            */
   float        fmv[3];                   /**< filtered magnetic vector             */
   uint8_t      accuracy;                 /**< accuracy of the filtered output      */
   uint8_t      algo_inst_id;             /**< algorithm instance id                */
}sns_log_fmv_mag_update_s;

#define SNS_LOG_FMV_GYRO_UPDATE_VERSION     1
typedef PACK(struct) {
   log_hdr_type log_type;                 /**< Type, as defined in log_codes.h      */
   uint8_t      version;                  /**< Version of the log packet            */
   uint8_t      timestamp_type;           /**< type of timestamp                    */
   uint64_t     timestamp;                /**< timestamp of the gyro input          */
   float        gyro[3];                  /**< input gyro                           */
   uint8_t      algo_inst_id;             /**< algorithm instance id                */
}sns_log_fmv_gyro_update_s;

#define SNS_LOG_FMV_REPORT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;                 /**< Type, as defined in log_codes.h      */
   uint8_t      version;                  /**< Version of the log packet            */
   uint8_t      timestamp_type;           /**< type of timestamp                    */
   uint64_t     timestamp;                /**< timestamp of rotation vector report  */
   int32_t      report_id;                /**< report id                            */
   uint8_t      report_type;              /**< report type                          */
   uint8_t      client_id;                /**< client id                            */
   float        fmv[3];                   /**< filtered magnetic vector             */
   uint8_t      accuracy;                 /**< rotation vector accuracy             */
   uint8_t      algo_inst_id;             /**< algorithm instance id                */
}sns_log_fmv_report_s;

/**
   sns_log_time_report_s
   This type logs current timestamp offsets between processors
   LOG_SNS_TIME_SERVICE_OFFSETS_C
 */
#define SNS_LOG_TIME_REPORT_VERSION 1
typedef PACK(struct) {
   log_hdr_type log_type;    /**< Type, as defined in log_codes.h */
   uint8_t version;          /**< version of the log packet */
   int64_t apps_dsps_offset; /**< Signed offset (in ns) from the Apps processor to the DSPS timestamp. */
   int64_t apps_mdm_offset;  /**< Signed offset (in ns) from the Apps processor to the Modem timestamp.d */
} sns_log_time_report_s;

#define SNS_LOG_BASIC_GESTURES_CONFIG_VERSION 2
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;                 /**< Version                    */
  uint8_t      timestamp_type;          /**< type of timestamp          */
  uint64_t     timestamp;               /**< timestamp                  */
  uint32_t     sample_rate;             /**< Sampling rate              */
  int32_t      sleep;                   /**< Sleep                      */
  uint32_t     push_threshold;          /**< push threshold             */
  uint32_t     pull_threshold;          /**< pull threshold             */
  uint32_t     shake_threshold;         /**< shake threshold            */
  int32_t      internal_config_param_1; /**< internal config param      */
  int32_t      internal_config_param_2; /**< internal config param      */
  int32_t      internal_config_param_3; /**< internal config param      */
  int32_t      internal_config_param_4; /**< internal config param      */
  uint8_t      algo_instance_id;        /**< algorithm instance id      */
  q16_t        sensor_report_rate;   /**< Requested sensor report rate in Hz, Q16 */
} sns_log_basic_gestures_config_s;

#define SNS_LOG_BASIC_GESTURES_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version                   */
  uint8_t      timestamp_type;    /**< type of timestamp         */
  uint64_t     timestamp;         /**< timestamp                 */
  int32_t      accel_x;           /**< acceleration along x axis */
  int32_t      accel_y;           /**< acceleration along y axis */
  int32_t      accel_z;           /**< acceleration along z axis */
  uint8_t      motion_state;      /**< motion state              */
  uint8_t      basic_state;       /**< algo state                */
  uint8_t      algo_inst_id;      /**< algo instance id          */
} sns_log_basic_gestures_result_s;

#define SNS_LOG_BASIC_GESTURES_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version                 */
  uint8_t      timestamp_type;    /**< type of timestamp       */
  uint64_t     timestamp;         /**< timestamp               */
  int32_t      report_id;         /**< report id               */
  uint8_t      report_type;       /**< report type             */
  uint8_t      client_id;         /**< client id               */
  uint8_t      basic_state;       /**< algo result             */
  uint8_t      algo_inst_id;      /**< algo instance id        */
} sns_log_basic_gestures_report_s;

#define SNS_LOG_FACING_CONFIG_VERSION 2
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;                 /**< Version                */
  uint8_t      timestamp_type;          /**< type of timestamp      */
  uint64_t     timestamp;               /**< timestamp              */
  uint32_t     sample_rate;             /**< Sampling rate          */
  int32_t      facing_angle_threshold;  /**< angle, radians, Q16    */
  int32_t      report_neutral;          /**< flag for Neutral state */
  uint8_t      algo_instance_id;        /**< algorithm instance id  */
  q16_t        sensor_report_rate;   /**< Requested sensor report rate in Hz, Q16 */
} sns_log_facing_config_s;

#define SNS_LOG_FACING_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version                    */
  uint8_t      timestamp_type;    /**< type of timestamp          */
  uint64_t     timestamp;         /**< timestamp                  */
  int32_t      accel_x;           /**< acceleration along x axis  */
  int32_t      accel_y;           /**< acceleration along y axis  */
  int32_t      accel_z;           /**< acceleration along z axis  */
  uint8_t      motion_state;      /**< motion state               */
  uint8_t      facing_state;      /**< algo result                */
  uint8_t      algo_inst_id;      /**< algo instance id           */
} sns_log_facing_result_s;

#define SNS_LOG_FACING_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version            */
  uint8_t      timestamp_type;    /**< type of timestamp  */
  uint64_t     timestamp;         /**< timestamp          */
  int32_t      report_id;         /**< report id          */
  uint8_t      report_type;       /**< report type        */
  uint8_t      client_id;         /**< client id          */
  uint8_t      facing_state;      /**< algo result        */
  uint8_t      algo_inst_id;      /**< algo instance id   */
} sns_log_facing_report_s;

#define SNS_LOG_INTEG_ANGLE_CONFIG_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version                  */
  uint8_t      timestamp_type;    /**< type of timestamp        */
  uint64_t     timestamp;         /**< timestamp                */
  int32_t      sample_rate;       /**< Sampling Rate, Hz, Q16   */
  int32_t      clip_threshold;    /**< Clip threshold, rad, Q16 */
  uint8_t      algo_inst_id;      /**< algo instance id         */
} sns_log_integ_angle_config_s;

#define SNS_LOG_INTEG_ANGLE_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version                    */
  uint8_t      timestamp_type;    /**< type of timestamp          */
  uint64_t     timestamp;         /**< timestamp                  */
  int32_t      g[3];              /**< gyro samples, rad/s Q16    */
  int32_t      ia[3];             /**< integrated angle , rad Q16 */
  uint8_t      algo_inst_id;      /**< algo instance id           */
} sns_log_integ_angle_result_s;

#define SNS_LOG_INTEG_ANGLE_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version                    */
  uint8_t      timestamp_type;    /**< type of timestamp          */
  uint64_t     timestamp;         /**< timestamp                  */
  int32_t      report_id;         /**< report id                  */
  uint8_t      report_type;       /**< report type                */
  uint8_t      client_id;         /**< client id                  */
  int32_t      ia[3];             /**< integrated angle , rad Q16 */
  uint8_t      algo_inst_id;      /**< algo instance id           */
}sns_log_integ_angle_report_s;

#define SNS_LOG_GYRO_TAP_CONFIG_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version            */
  uint8_t      timestamp_type;    /**< type of timestamp  */
  uint64_t     timestamp;         /**< timestamp          */
  int32_t      sample_rate;
  int32_t      tap_time_win;
  int32_t      tap_time_sleep;
  int32_t      tap_dir_win;
  int32_t      accel_tap_thr;
  int32_t      lr_accel_jerk_min_thresh_min;
  int32_t      lr_gyro_jerk_min_thresh_min;
  int32_t      tb_accel_jerk_min_thr;
  int32_t      tb_gyro_jerk_min_thr;
  int32_t      jerk_win;
  int32_t      lr_accel_rat_jerk_yx;
  int32_t      lr_accel_rat_jerk_yz;
  int32_t      lr_gyro_rat_jerk_zy;
  int32_t      lr_gyro_rat_jerk_zx;
  int32_t      tb_accel_rat_jerk_xy;
  int32_t      tb_accel_rat_jerk_xz;
  int32_t      tb_gyro_rat_jerk_yx;
  int32_t      tb_gyro_rat_jerk_yz;
  int32_t      tb_accel_z_thresh;
  int32_t      tb_accel_z_rat_zx;
  int32_t      tb_accel_z_rat_zy;
  int32_t      ori_change_win;
  int32_t      ori_check_win;
  int32_t      ori_change_thr;
  int32_t      z_axis_inc;
  int32_t      loaded_axis_3_valid;
  int32_t      lr_min_accel_jerk_thresh_min;
  int32_t      lr_max_accel_jerk_thresh_min;
  int32_t      lr_min_gyro_jerk_thresh_min;
  int32_t      lr_max_gyro_jerk_thresh_min;
  int32_t      mild_accel_tap_thresh;
  int32_t      loaded_z_axis_anamoly;
  int32_t      ori_change_reject_mode;
  int32_t      stress_right_left;
  uint8_t      algo_inst_id;      /**< algo instance id   */
}sns_log_gyro_tap_config_s;

#define SNS_LOG_GYRO_TAP_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version                    */
  uint8_t      timestamp_type;    /**< type of timestamp          */
  uint64_t     timestamp;         /**< timestamp                  */
  int32_t      g[3];              /**< gyro samples, rad/s Q16    */
  int32_t      a[3];              /**< accel samples, m/s/s   Q16 */
  int32_t      tap_event;         /**< tap event type             */
  int32_t      algo_state;        /**< reserved                   */
  uint8_t      algo_inst_id;      /**< algo instance id           */
} sns_log_gyro_tap_result_s;

#define SNS_LOG_GYRO_TAP_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /**< Version                    */
  uint8_t      timestamp_type;    /**< type of timestamp          */
  uint64_t     timestamp;         /**< timestamp                  */
  int32_t      report_id;         /**< report id                  */
  uint8_t      report_type;       /**< report type                */
  uint8_t      client_id;         /**< client id                  */
  int32_t      tap_event;         /**< tap event type             */
  uint8_t      algo_inst_id;      /**< algo instance id           */
}sns_log_gyro_tap_report_s;

#define SNS_LOG_PED_CONFIG_VERSION 2
typedef PACK(struct) {
  log_hdr_type log_type;             /**< Type, as defined in log_codes.h                       */
  uint8_t      version;              /**< Version of the log packet                             */
  uint64_t     timestamp;            /**< Time in SSC ticks when algorithm is configured        */
  uint32_t     sample_rate;          /**< Sample rate in Hz, Q16                                */
  uint32_t     step_count_threshold; /**< No of steps after which client has to be notified     */
  float        step_threshold;       /**< Min accel magnitude to be considered a step, in m/s/s */
  float        swing_threshold;      /**< Min accel magnitude for swing detection, in m/s/s     */
  float        step_prob_threshold;  /**< Probability threshold for step detection              */
  uint8_t      algo_inst_id;         /**< Algorithm instance id                                 */
  q16_t        sensor_report_rate; /**< Requested sensor report rate in Hz, Q16              */
} sns_log_ped_config_s;

#define SNS_LOG_PED_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;             /**< Type, as defined in log_codes.h                              */
  uint8_t      version;              /**< Version of the log packet                                    */
  uint64_t     timestamp;            /**< Timestamp of input                                           */
  int32_t      accel[3];             /**< Accel input  (x, y, z), m/s/s, Q16                           */
  float        sample_rate;          /**< Actual sample rate (Hz) used for step calculations           */
  uint8_t      client_id;            /**< Client id                                                    */
  uint8_t      reset_stats;          /**< 0 - if new sample triggered update; 1 - for reset request    */
  uint8_t      step_event;           /**< 0 - No step; 1 - Step detected, since the last reset         */
  uint8_t      step_confidence;      /**< Confidence with which latest step was detected - 0 to 100    */
  uint32_t     step_count;           /**< Count of steps since the last client initiated reset         */
  int32_t      step_count_error;     /**< Error metric associated with reported step count, in steps   */
  uint32_t     swing_step_count;     /**< No of steps detected in swing detection phase                */
  float        step_rate;            /**< Rate at which steps are detected since the last report/reset */
  uint8_t      algo_inst_id;         /**< Algorithm instance id                                        */
} sns_log_ped_result_s;

#define SNS_LOG_PED_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;             /**< Type, as defined in log_codes.h                                     */
  uint8_t      version;              /**< Version of the log packet                                           */
  uint64_t     timestamp;            /**< Timestamp of input with which latest step was detected in SSC ticks */
  int32_t      report_id;            /**< Report id                                                           */
  uint8_t      report_type;          /**< Report type                                                         */
  uint8_t      client_id;            /**< Client id                                                           */
  uint8_t      step_event;           /**< 0 - No step; 1 - Step detected, since the last reset                */
  uint8_t      step_confidence;      /**< Confidence with which latest step was detected - 0 to 100           */
  uint32_t     step_count;           /**< Count of steps since the last client initiated reset                */
  int32_t      step_count_error;     /**< Error metric associated with reported step count, in steps          */
  float        step_rate;            /**< Rate at which steps are detected since the last report/reset        */
  uint8_t      algo_inst_id;         /**< Algorithm instance id                                               */
}sns_log_ped_report_s;

#define SNS_LOG_PAM_CONFIG_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;              /* version of the log packet*/
  uint8_t      timestamp_type;       /* type of timestamp  */
  uint64_t     timestamp;            /* timestamp in clock ticks*/
  uint32_t     measurement_period;   /* measurement period in seconds*/
  uint32_t     step_count_threshold; /* reporting threshold in steps*/
  uint32_t     sample_rate;         /* sample rate in Q16 Hz*/
  uint8_t      duty_cycle_on_percent;/* percentage of time for which algo is active*/
  uint8_t      algo_inst_id;         /* algorithm instance id   */
} sns_log_pam_config_s;

#define SNS_LOG_PAM_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;                      /* version of the log packet */
  uint8_t      timestamp_type;               /* type of timestamp          */
  uint64_t     timestamp;                    /* timestamp in clock ticks*/
  int32_t      pedometer_step_count;         /*step count input from pedometer*/
  uint8_t      motion_state;                 /*motion state input from amd*/
  uint8_t      data_type;                    /*input type*/
  int32_t      pedometer_last_step_count;    /*last step count input from pedometer*/
  int32_t      pam_current_step_count;       /*current step count computed by pam*/
  int32_t      pam_last_reported_step_count; /*step count last reported by pam*/
  uint8_t      pam_report_decision;          /*0- don't report, 1- report*/
  uint8_t      algo_inst_id;                 /* algorithm instance id   */
} sns_log_pam_result_s;

#define SNS_LOG_PAM_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;           /* version of the log packet */
  uint8_t      timestamp_type;    /* type of timestamp          */
  uint64_t     timestamp;         /* timestamp in clock ticks*/
  int32_t      step_count;        /* step count or current device motion state */
  uint8_t      algo_inst_id;      /* algo instance id           */
}sns_log_pam_report_s;

/**
   sns_log_fifo_event_s
   This type is used to log FIFO event. It
   is used for the following log types: LOG_SENSOR_FIFO_EVENT_C
 */

typedef enum
{
  SNS_DDF_FIFO_ENABLED =0,
  SNS_DDF_FIFO_DISABLED =1,
  SNS_DDF_FIFO_AVAILABLE =2,
  SNS_DDF_FIFO_WM_INT =3,
  SNS_DDF_FIFO_OVERFLOW =4,
  SNS_DDF_FIFO_ODR_CHANGED=5,
  SNS_DDF_FIFO_READ_WM=6,
  SNS_DDF_FIFO_READ_FLUSH=7,
  SNS_DDF_FIFO_DRAIN=8
} sns_fifo_event_e;

#define SNS_LOG_FIFO_EVENT_VERSION 1
typedef PACK(struct) {
  log_hdr_type  log_type;                     /**< Type, as defined in log_codes.h                  */
  uint8_t       version;                      /**< version of the log packet                        */
  uint64_t      timestamp;                    /**< Time in SSC ticks when the sensor event happened */
  uint32_t      sensor_id;                    /**< smgr sensor id of the sensor being logged        */
  uint8_t       fifo_event;                   /**< FIFO Event as specified in sns_fifo_event_e      */
  uint8_t       watermark_depth;              /**< water mark (num of samples )                     */
  uint16_t      odr;                          /**< current ODR                                      */
  uint8_t       hw_watermark_int_supported;   /**< if hardware watermark is supported               */
  uint16_t      max_watermark;                /**< maximum watermark supported by the sensor        */
} sns_log_fifo_event_s;

#define SNS_LOG_CMC_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;                      /* version of the log packet */
  uint8_t      timestamp_type;               /* type of timestamp          */
  uint64_t     timestamp;                    /* timestamp in SSC ticks*/
  uint8_t      input_type;                   /* input type, 0:accel input, 1: QMD Rest Input*/
  int32_t      accel[3];                     /* accel input (x, y, z), m/s^2, Q16 */
  uint16_t     sample_count;                 /* number of samples in measurement period*/
  float        accel_prev[3];                /* last received accel input (x, y, z), m/s^2*/
  float        accel_sum[3];                 /* sum of accel (x, y, z) in current measurement period m/s^2*/
  float        accel_norm_sum;               /* sum of accel norm in current measurement period m/s^2*/
  float        accel_norm_sq_sum;            /* sum of accel norm square in current measurement period m^2/s^4*/
  uint8_t      algo_inst_id;                 /* algorithm instance id   */
} sns_log_cmc_result_s;

#define SNS_LOG_CMC_RESULT2_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;                      /* version of the log packet */
  uint8_t      timestamp_type;               /* type of timestamp          */
  uint64_t     timestamp;                    /* timestamp in SSC ticks*/
  float        sa;                           /* sum of accel norm square in current measurement period m^2/s^4*/
  float        rm;                           /* sum of accel norm square in current measurement period m^2/s^4*/
  float        unfilttered_probability[5];   /* unfiltered motion state marginal probabilities*/
  float        alpha[5];                     /* hmm forward algorithm parameters*/
  float        beta[5];                      /* hmm backward algorithm parameters*/
  float        filttered_probability[8];     /* filtered motion state marginal probabilities*/
  uint8_t      motion_state_curr;            /* current motion state of the device*/
  uint8_t      motion_state_prev;            /* previous motion state of the device*/
  uint8_t      motion_state_persist_time;    /* time for which current motion state has persisted, sec*/
  float        motion_state_curr_probability;/* probability of current motion state*/
  uint8_t      algo_inst_id;                 /* algorithm instance id   */
} sns_log_cmc_result2_s;

#define SNS_LOG_CMC_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;
  uint8_t      version;                         /* version of the log packet */
  uint8_t      timestamp_type;                  /* type of timestamp          */
  uint64_t     timestamp;                       /* timestamp in SSC ticks*/
  uint8_t      motion_state;                    /* final reported motion state of the device */
  float        motion_state_probability;        /* probability of the reported motion state*/
  uint8_t      algo_inst_id;                    /* algo instance id */
}sns_log_cmc_report_s;

#define SNS_LOG_DISTANCE_BOUND_CONFIG_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;                  /**< Type, as defined in log_codes.h                */
  uint8_t      version;                   /**< Version of the log packet                      */
  uint8_t      timestamp_type;            /**< Type of timestamp                              */
  uint64_t     timestamp;                 /**< Time in SSC ticks when algorithm is configured */
  uint8_t      client_req_id;             /**< ID of client requesting for service            */
  float        ms_unknown_speedbound;     /**< speed bound for unknown state                  */
  float        ms_stationary_speedbound;  /**< speed bound for stationary state               */
  float        ms_inmotion_speedbound;    /**< speed bound for inmotion state                 */
  float        ms_fiddle_speedbound;      /**< speed bound for fiddle state                   */
  float        ms_pedestrian_speedbound;  /**< speed bound for pedestrian state               */
  float        ms_vehicle_speedbound;     /**< speed bound for vehicle state                  */
  float        ms_walk_speedbound;        /**< speed bound for walk state                     */
  float        ms_run_speedbound;         /**< speed bound for run state                      */
  uint8_t      algo_inst_id;              /**< Algorithm instance id                          */
} sns_log_distance_bound_config_s;

#define SNS_LOG_DISTANCE_BOUND_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;                  /**< Type, as defined in log_codes.h                */
  uint8_t      version;                   /**< Version of the log packet                      */
  uint8_t      timestamp_type;            /**< Type of timestamp                              */
  uint64_t     timestamp;                 /**< Time in SSC ticks when algorithm is configured */
  uint8_t      input_type;                /**< 0: set distance bound 1: cmc input             */
  float        input_distance_bound;      /**< distance bound setting from client             */
  uint8_t      input_cmc_motion_state;    /**< Motion State input from CMC                    */
  uint8_t      algo_state;                /**< 0: inactive 1: active                          */
  uint64_t     prev_timestamp;            /**< Time stamp at last distance bound computation  */
  float        prev_speed;                /**< Speed at last distance bound computation       */
  float        prev_distance_bound;       /**< Distance to bound                              */
  float        time_to_breach;            /**< Time to breach of client set bound             */
  uint8_t      session_key;               /**< uinque session identifier provided by client   */
  uint8_t      algo_inst_id;              /**< Algorithm instance id                          */
} sns_log_distance_bound_result_s;

#define SNS_LOG_DISTANCE_BOUND_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;                   /**< Type, as defined in log_codes.h                */
  uint8_t      version;                    /**< Version of the log packet                      */
  uint8_t      timestamp_type;             /**< Type of timestamp                              */
  uint64_t     timestamp;                  /**< Time in SSC ticks when algorithm is configured */
  uint8_t      distance_bound_breach_event;/**< 1: breach occurred 0: breach not occurred      */
  uint8_t      session_key;                /**< uinque session identifier provided by client   */
  uint8_t      algo_inst_id;               /**< Algorithm instance id                          */
} sns_log_distance_bound_report_s;

#define SNS_LOG_LATENCY_SAMPLING_POLL_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_typel;
  uint8_t      version;           /* Version                    */
  uint64_t     timestamp;         /* timestamp                  */
  int32_t      sensor_id;         /* sensor id                  */
  uint64_t     data_timestamp;    /* data timestamp             */
  int64_t      sampling_latency;  /* sampling latency           */
  int64_t      polling_latency;   /* polling latency            */
}sns_log_latency_sampling_poll_s;

#define SNS_LOG_LATENCY_SAMPLING_DRI_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_typel;
  uint8_t      version;                /* Version                    */
  uint64_t     timestamp;              /* timestamp                  */
  int32_t      sensor_id;              /* sensor id                  */
  uint64_t     data_timestamp;         /* data timestamp             */
  int64_t      sampling_latency;       /* sampling latency           */
  int64_t      dri_get_data_latency;   /* getting data latency       */
}sns_log_latency_sampling_dri_s;

#define SNS_LOG_LATENCY_DELIVERY_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_typel;
  uint8_t      version;                /* Version                    */
  uint64_t     timestamp;              /* timestamp                  */
  int32_t      sensor_id;              /* sensor id                  */
  int64_t      data_timestamp;         /* data timestamp             */
  int64_t      delivery_latency;       /* delivery latency           */
}sns_log_latency_delivery_s;

#define SNS_LOG_GAME_ROTATION_VECTOR_CONFIG_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;                  /**< Type, as defined in log_codes.h                   */
  uint8_t      version;                   /**< Version of the log packet                         */
  uint8_t      timestamp_type;            /**< Type of timestamp                                 */
  uint64_t     timestamp;                 /**< Time in SSC ticks when algorithm is configured    */
  uint32_t     report_period;             /**< Report period                                     */
  uint32_t     sample_rate;               /**< Rate at which the sensors are sampled             */
  uint8_t      coordinate_sys;            /**< coordinate system used in the output              */
  float        abs_rest_gyro_az_tilt_tc;  /**< tilt filter time constant for absolute rest state */
  float        rel_rest_gyro_az_tilt_tc;  /**< tilt filter time constant for relative rest state */
  float        motion_gyro_az_tilt_tc;    /**< tilt filter time constant for motion state        */
  float        gyro_noise_threshold;      /**< gyro quat noise rejection threshold               */
  float        max_gyro_az_tilt_freeze;   /**< max movable gyro-azimuth tilt                     */
  float        max_gyro_az_tilt_reset;    /**< max resettable gyro-azimuth tilt                  */
  uint8_t      algo_inst_id;              /**< Algorithm instance id                             */
} sns_log_game_rotation_vector_config_s;

#define SNS_LOG_GAME_ROTATION_VECTOR_RESULT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;                 /**< Type, as defined in log_codes.h      */
  uint8_t      version;                  /**< Version of the log packet            */
  uint8_t      timestamp_type;           /**< type of timestamp                    */
  uint64_t     timestamp;                /**< timestamp of GameRV result           */
  float        gravity[3];               /**< gravity vector input                 */
  uint8_t      gravity_accuracy;         /**< gravity vector accuracy              */
  uint64_t     gravity_timestamp;        /**< gravity vector timestamp             */
  uint8_t      gravity_dev_mot_state;    /**< device motion state (from Gravity)   */
  float        gyro_quaternion[4];       /**< gyro quaternion input                */
  uint64_t     gyro_quat_timestamp;      /**< gyro quaternion timestamp            */
  float        game_rotation_vector[4];  /**< game rotation vector result          */
  uint8_t      accuracy;                 /**< game rotation vector accuracy        */
  uint8_t      coordinate_sys;           /**< coordinate system used in the output */
  uint8_t      algo_inst_id;             /**< algorithm instance id                */
} sns_log_game_rotation_vector_result_s;

#define SNS_LOG_GAME_ROTATION_VECTOR_REPORT_VERSION 1
typedef PACK(struct) {
  log_hdr_type log_type;                 /**< Type, as defined in log_codes.h      */
  uint8_t      version;                  /**< Version of the log packet            */
  uint8_t      timestamp_type;           /**< type of timestamp                    */
  uint64_t     timestamp;                /**< timestamp of GameRV report           */
  int32_t      report_id;                /**< report id                            */
  uint8_t      report_type;              /**< report type                          */
  uint8_t      client_id;                /**< client id                            */
  float        game_rotation_vector[4];  /**< game rotation vector result          */
  uint8_t      accuracy;                 /**< game rotation vector accuracy        */
  uint8_t      coordinate_sys;           /**< coordinate system used in the output */
  uint8_t      algo_inst_id;             /**< algorithm instance id                */
}sns_log_game_rotation_vector_report_s;

#if defined(SNS_PCSIM) || defined(_WIN32)
#pragma pack(pop)
#endif

#endif /* SNS_LOG_TYPES_H */
