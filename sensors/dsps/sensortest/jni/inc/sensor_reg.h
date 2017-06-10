/*==========================================================================
@file sensor_reg.h

@brief
Provides functions to acquire and set settings in the sensors registry.

Copyright (c) 2011,2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==========================================================================*/

#ifndef SNS_LA_SIM
# include <jni.h>
#endif
#include <stdint.h>

#ifndef _SENSOR_REG_
#define _SENSOR_REG_

typedef enum sensor_reg_error_e {
   SENSOR_REG_SUCCESS = 0, /* No error */
   SENSOR_REG_INTERNAL = -1, /* Internal Error */
   SENSOR_REG_RESP = -2, /* Response error - timeout */
   SENSOR_REG_PROCESS = -3, /* Unable to process response message */
   SENSOR_REG_SENSOR1 = -4, /* Sensor1 Error */
   SENSOR_REG_LEN = -5, /* Range error: len */
   SENSOR_REG_OPEN = -6/* Did not call open() first */
} sensor_reg_error_e;

/**
 * Requests the present value for an item within the sensors registry
 *
 * @param [i] id ID of the item in the registry; as specified in sns_reg_api
 * @param [i] time_out Seconds to wait for a response before returning -2.
 * @param [o] data Where to place the returned registry value.  Client must free.
 * @param [o] data_len Length of data
 * @return  0: Success
 *         -1: Internal error
 *         -2: Response error - Timeout
 *         -3: Unable to process response message
 *         -4: Sensor1 error
 *         -6: Did not call open() first
 */
int sensor_reg_read(uint16_t id, uint8_t time_out, uint8_t **data, uint8_t *data_len);

/**
 * Writes a new value to the sensors registry.  This new value will persist
 * future calls to close() and device reboots.
 *
 * @param [i] id ID of the item in the registry; as specified in sns_reg_api
 * @param [i] data Array of bytes to be placed into the registry
 * @param [i] len Size of the registry entry; must correspond with data.
 * @param [i] time_out Seconds to wait for a response before returning -2.
 * @return  0: Success
 *         -1: Internal error
 *         -2: Response error - Timeout
 *         -3: Unable to process response message
 *         -4: Sensor1 error
 *         -5: Range Error: len
 *         -6: Did not call open() first
 */
int sensor_reg_write(uint16_t id, uint8_t const *data, int len, uint8_t time_out);

/**
 * Initializes the internal data structures used to read/write registry values.
 * Must be called prior to using any other function.
 *
 * @return 0 upon success; <0 upon error
 */
int sensor_reg_open();

/**
 * Closes and/or destroys the internal data structures created by sensor_reg_open.
 *
 * @return 0 upon success; <0 upon error
 */
int sensor_reg_close();

#ifndef SNS_LA_SIM
/*
 * Class:     com_qualcomm_sensors_sensortest_SensorsReg
 * Method:    setRegistryValue
 * Signature: (S[BB)I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_sensors_sensortest_SensorsReg_setRegistryValue
  (JNIEnv *, jclass, jint, jbyteArray, jbyte);

/*
 * Class:     com_qualcomm_sensors_sensortest_SensorsReg
 * Method:    getRegistryValue
 * Signature: (S)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_qualcomm_sensors_sensortest_SensorsReg_getRegistryValue
  (JNIEnv *, jclass, jint);

/*
 * Class:     com_qualcomm_sensors_sensortest_SensorsReg
 * Method:    open
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_sensors_sensortest_SensorsReg_open
  (JNIEnv *, jclass);

/*
 * Class:     com_qualcomm_sensors_sensortest_SensorsReg
 * Method:    close
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_sensors_sensortest_SensorsReg_close
  (JNIEnv *, jclass);
#endif /* SNS_LA_SIM */
#endif /* _SENSOR_REG_ */
