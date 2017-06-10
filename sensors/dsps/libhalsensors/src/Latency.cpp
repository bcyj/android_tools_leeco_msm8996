/*============================================================================
  @file Latency.cpp

  @brief
  Latency class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <errno.h>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "Latency.h"
#include "sns_reg_api_v02.h"
#include "sns_reg_common.h"
extern "C" {
#include "sensor_reg.h"
#include "sns_log_types.h"
}

bool Latency::is_latency_measure_enabled = false;
bool Latency::latency_en_table[HAL_LTCY_NUM_TYPES];
uint32_t Latency::latency_measure_dsps_tick = 0;

/*============================================================================
  Latency constructor
============================================================================*/
Latency::Latency()
{

}

/*============================================================================
  Latency destructor
============================================================================*/
Latency::~Latency()
{

}
/*============================================================================
    FUNCTION: isLatencyMeasureEnabled
    This function returns the value of is_latency_measure_enabled.
 *=============================================================================*/
bool Latency::isLatencyMeasureEnabled()
{
    return is_latency_measure_enabled;
}

/*============================================================================
    FUNCTION: setTick
    Set the TimeStamp to latency_measure_dsps_tick.
 *=============================================================================*/
void Latency::setTick(uint32 TimeStamp)
{
    latency_measure_dsps_tick = TimeStamp;
}
/*===========================================================================

  FUNCTION: latencyCheckSSIRegFlag

  @brief
  checks a particular CFG group about whether latency measurement enabled or
  not for the corresponding sensor, then update the latency enable control
  member and latency measurement enable table in global variable g_sensor_control.

  @param[i] ssi_flag_id CFG group flag ID
  @param[i] ssi_sensor_id CFG group sensor ID

===========================================================================*/
void Latency::latencyCheckSSIRegFlag(int ssi_flag_id, int ssi_sensor_id)
{
    uint8_t *reg_value = NULL;
    uint8_t reg_length = 0;
    int error;
    error = sensor_reg_open();
    if (SENSOR_REG_SUCCESS != error) {
        HAL_LOG_ERROR( "%s: Error in sensor_reg_open() : %d",
                    __FUNCTION__, error );
        return;
    }
    error = sensor_reg_read(ssi_flag_id, 1, &reg_value, &reg_length);
    if (SENSOR_REG_SUCCESS != error || NULL == reg_value) {
        HAL_LOG_ERROR( "%s: Error in sensor_reg_read() for reg flag: %d",
                    __FUNCTION__, error );
    }
    else {
        if (*reg_value & SNS_REG_SSI_FLAG_LTCY_ENABLE) {
            free( reg_value );
            error = sensor_reg_read(ssi_sensor_id, 1, &reg_value, &reg_length);
            if (SENSOR_REG_SUCCESS != error) {
                HAL_LOG_ERROR( "%s: Error in sensor_reg_read() for reg sensor id: %i",
                    __FUNCTION__, error );
            }
            else {
                if (NULL != reg_value) {
                    switch( *reg_value ) {
                        case SNS_SMGR_ID_ACCEL_V01:
                            latency_en_table[HAL_LTCY_MEASURE_ACCEL] = true;
                            break;
                        case SNS_SMGR_ID_GYRO_V01:
                            latency_en_table[HAL_LTCY_MEASURE_GYRO] = true;
                            break;
                        case SNS_SMGR_ID_MAG_V01:
                            latency_en_table[HAL_LTCY_MEASURE_MAG] = true;
                            break;
                        case SNS_SMGR_ID_PRESSURE_V01:
                            latency_en_table[HAL_LTCY_MEASURE_PRESSURE] = true;
                            break;
                        case SNS_SMGR_ID_PROX_LIGHT_V01:
                            latency_en_table[HAL_LTCY_MEASURE_PROX_LIGHT] = true;
                            break;
                        case SNS_SMGR_ID_HUMIDITY_V01:
                            latency_en_table[HAL_LTCY_MEASURE_HUMIDITY] = true;
                            break;
                        case SNS_SMGR_ID_RGB_V01:
                            latency_en_table[HAL_LTCY_MEASURE_RGB] = true;
                            break;
                        case SNS_SMGR_ID_IR_GESTURE_V01:
                            latency_en_table[HAL_LTCY_MEASURE_IR_GESTURES] = true;
                            break;
                        case SNS_SMGR_ID_SAR_V01:
                            latency_en_table[HAL_LTCY_MEASURE_SAR] = true;
                            break;
                        case SNS_SMGR_ID_HALL_EFFECT_V01:
                            latency_en_table[HAL_LTCY_MEASURE_HALL_EFFECT] = true;
                            break;
                        default:
                            break;
                    }
                    is_latency_measure_enabled = true;
                }
            }
        }
    }
    if (NULL != reg_value) {
        free( reg_value );
    }
    sensor_reg_close();
}

/*============================================================================
    FUNCTION: latencyCheckMeasure
    This function checks whether latency measurement enabled or not for each
    physical sensor.
 *=============================================================================*/
void Latency::latencyCheckMeasure()
{
    int cfg_flag_id[] = {   SNS_REG_ITEM_SSI_SMGR_CFG0_FLAGS_V02,
                            SNS_REG_ITEM_SSI_SMGR_CFG1_FLAGS_V02,
                            SNS_REG_ITEM_SSI_SMGR_CFG2_FLAGS_V02,
                            SNS_REG_ITEM_SSI_SMGR_CFG3_FLAGS_V02,
                            SNS_REG_ITEM_SSI_SMGR_CFG4_FLAGS_V02};
    int cfg_sensor_id[] = { SNS_REG_ITEM_SSI_SMGR_CFG0_SENSOR_ID_V02,
                            SNS_REG_ITEM_SSI_SMGR_CFG1_SENSOR_ID_V02,
                            SNS_REG_ITEM_SSI_SMGR_CFG2_SENSOR_ID_V02,
                            SNS_REG_ITEM_SSI_SMGR_CFG3_SENSOR_ID_V02,
                            SNS_REG_ITEM_SSI_SMGR_CFG4_SENSOR_ID_V02};

    int i, num = sizeof(cfg_flag_id) / sizeof(int);
    for (i=0; i<num; i++) {
        latencyCheckSSIRegFlag(cfg_flag_id[i], cfg_sensor_id[i]);
    }
}
/*===========================================================================

  FUNCTION: latencyMeasure
  @brief
  measure the delivery latency and report the result using
  log packet

  @param[i] curr_ts current apps timestamp
  @param[i] data[] array of sensor data got by hal_sensors_data_poll
  @param[i] rcv_data_num is the number of data got from hal_sensors_data_poll
===========================================================================*/
void Latency::latencyMeasure(uint64_t curr_ts,
                             const sensors_event_t data[],
                             int rcv_data_num)
{
    int err, j;
    /* set the default one as accel */
    int32_t sensor_id = SNS_SMGR_ID_ACCEL_V01;
    /* default value */
    hal_ltcy_measure_t latency_measure_type = HAL_LTCY_NUM_TYPES;

    for (j = 0; j < rcv_data_num; j++) {
        switch (data[j].sensor) {
            case HANDLE_ACCELERATION:
                sensor_id = SNS_SMGR_ID_ACCEL_V01;
                latency_measure_type = HAL_LTCY_MEASURE_ACCEL;
                break;
            case HANDLE_MAGNETIC_FIELD:
            case HANDLE_MAGNETIC_FIELD_UNCALIBRATED:
            case HANDLE_MAGNETIC_FIELD_SAM:
            case HANDLE_MAGNETIC_FIELD_UNCALIBRATED_SAM:
                sensor_id = SNS_SMGR_ID_MAG_V01;
                latency_measure_type = HAL_LTCY_MEASURE_MAG;
                break;
            case HANDLE_PROXIMITY:
                sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;
                latency_measure_type = HAL_LTCY_MEASURE_PROX_LIGHT;
                break;
            case HANDLE_GYRO:
                sensor_id = SNS_SMGR_ID_GYRO_V01;
                latency_measure_type = HAL_LTCY_MEASURE_GYRO;
                break;
            case HANDLE_PRESSURE:
                sensor_id = SNS_SMGR_ID_PRESSURE_V01;
                latency_measure_type = HAL_LTCY_MEASURE_PRESSURE;
                break;
            case HANDLE_RELATIVE_HUMIDITY:
            case HANDLE_AMBIENT_TEMPERATURE:
                sensor_id = SNS_SMGR_ID_HUMIDITY_V01;
                latency_measure_type = HAL_LTCY_MEASURE_HUMIDITY;
                break;
            case HANDLE_RGB:
                sensor_id = SNS_SMGR_ID_RGB_V01;
                latency_measure_type = HAL_LTCY_MEASURE_RGB;
                break;
            case HANDLE_IR_GESTURE:
                sensor_id = SNS_SMGR_ID_IR_GESTURE_V01;
                latency_measure_type = HAL_LTCY_MEASURE_IR_GESTURES;
                break;
            case HANDLE_SAR:
                sensor_id = SNS_SMGR_ID_SAR_V01;
                latency_measure_type = HAL_LTCY_MEASURE_SAR;
                break;
            case HANDLE_HALL_EFFECT:
                sensor_id = SNS_SMGR_ID_HALL_EFFECT_V01;
                latency_measure_type = HAL_LTCY_MEASURE_HALL_EFFECT;
                break;
            default:
                break;
        }

        if (latency_measure_type >= HAL_LTCY_NUM_TYPES) {
            HAL_LOG_ERROR("%s: reading data[].sensor error with handle: %d",
                                                __FUNCTION__, data[j].sensor);
            break;
        }

        if (latency_en_table[latency_measure_type]) {
            /* test with sensor1 response log packet type */
            sns_log_latency_delivery_s  *log_ptr;
            void                        *temp_ptr;
            sns_log_id_e                 log_type;
            uint32_t                     logpkt_size;

            log_type = SNS_LOG_LATENCY_DELIVERY;
            logpkt_size = SNS_LOG_MAX_SIZE + sizeof(sns_log_latency_delivery_s);

            /* log packet malloc */
            temp_ptr = log_alloc(LOG_SNS_LATENCY_DELIVERY_C, logpkt_size);
            if (temp_ptr == NULL) {
                HAL_LOG_ERROR("log_alloc error!");
                break;
            }
            else {
                log_ptr = (sns_log_latency_delivery_s *)temp_ptr;
                /* allocate the log packet */
                log_ptr->version = SNS_LOG_LATENCY_DELIVERY_VERSION;
                log_ptr->timestamp = curr_ts;
                log_ptr->sensor_id = sensor_id;
                log_ptr->data_timestamp = latency_measure_dsps_tick;
                log_ptr->delivery_latency = (int64_t)curr_ts - data[j].timestamp;

                /*log packet commit */
                log_commit( (void*)log_ptr);
            }
        }
    }
}



