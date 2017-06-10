/*============================================================================
  @file Sensor.h

  @brief
  Sensor class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_SENSOR_H
#define ANDROID_SENSOR_H

#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <hardware/hardware.h>
#include <hardware/sensors.h>

extern "C" {
#include "sensor1.h"
#define __bool_true_false_are_defined 1
#include "sensors_hal.h"
}

struct sensors_event_t;

/*============================================================================
 * Class Sensor
 *=============================================================================*/

class Sensor {
protected:
    /* The sensor enable/disable status */
    int enabled;
    /* The sensor working frequency in Hz requested by client
     * This value is used by the *sns_smgr_buffering_req_item_s_v01->SamplingRate */
    float freq;
    /* The sensor report frequency in Hz
     * This value is used by the *sns_smgr_buffering_req_msg_v01->ReportRate */
    uint32_t report_rate;
    /* Batching or not */
    bool batching;
    /* Increment if a flush has been requested for this sensor.
     * When the flush is completed, it will be decremented. */
    int flush_requested;
    /* The trigger_mode represents the sensor report data method. */
    sensor_trigger_mode trigger_mode;
    /* hal_data_cb_t has three members.
     * One data_mutex is used to protect the sensor event data_cond.
     * One data_cond is used to indicate there is one new sensor event data.
     * One flag is used to indicate the callback has arrived. */
    hal_data_cb_t *data_cb;
    /* True if sensor attributes can be obtained, false otherwise */
    bool is_attrib_ok;
    /* The sensor attributes used by the framework sensor list.
     * They are defined in the hardware/libhardware/include/hardware/sensors.h file. */
    /* Name of this sensor.
     * All sensors of the same "type" must have a different "name".
     */
    char name[SNS_MAX_SENSOR_NAME_SIZE];
    /* vendor of the hardware part */
    char vendor[SNS_MAX_VENDOR_NAME_SIZE];
    /* version of the hardware part + driver. The value of this field
     * must increase when the driver is updated in a way that changes the
     * output of this sensor. This is important for fused sensors when the
     * fusion algorithm is updated.
     */
    int version;
    /* handle that identifies this sensors. This handle is used to reference
     * this sensor throughout the HAL API.
     */
    int handle;
    /* sensor type
     * The sensor type is defined in the hardware/sensors.h file
     */
    int type;
    /* maximum range of this sensor's value in SI units */
    float max_range;
    /* smallest difference between two values reported by this sensor */
    float resolution;
    /* rough estimate of this sensor's power consumption in mA */
    float power;
    /* maximum working frequency of the sensor */
    float max_freq;
    /* minimum working frequency of the sensor */
    float min_freq;
    /* number of samples this sensor can buffer */
    int max_buffered_samples;
    /* flags associated like continuous,on-change,wakeup etc */
    int flags;
    /* string type of the sensor */
    char strtype[SNS_MAX_SENSOR_NAME_SIZE];
    /* permissions required to see the sensor */
    char reqpermission[SNS_MAX_PERMISSION_SIZE];
    /* to check sensor is of wakeup or normal varient */
    bool bWakeUp;
    /* min sampling freq */
    float min_sample_freq;
    /* max sampling freq */
    float max_sample_freq;
/*============================================================================
  FUNCTION calcSampleRate
    Calculate the sensor sample rate
    Parameters:
        @ns : The sensor report data period
    Return value
        @uint32_t : The calculated sample rate in Hz
============================================================================*/
    float calcSampleRate(uint64_t ns);

public:
/*============================================================================
  Constructor
    Parameter
        @handle : the sensor handle passed to the constructor
============================================================================*/
    Sensor(int handle);
/*============================================================================
  Destructor
============================================================================*/
    virtual ~Sensor();
/*============================================================================
  FUNCTION enable
    Enable the sensor of handle. This is a pure virtual function. It will be
    Implemented in the SMGRSensor and SAMSensor class.
============================================================================*/
    virtual int enable(int en) = 0;
/*============================================================================
  FUNCTION batch
    Enable the batch and the freq and rpt_data for the sensor of handle .
    This is a pure virtual function. It will be Implemented in the SMGRSensor
    and SAMSensor class.
============================================================================*/
    virtual int batch(int flags, int64_t period_ns, int64_t timeout) = 0;
/*============================================================================
  FUNCTION flush
    Flush the sensor data of handle. This is a pure virtual function. It will be
    Implemented in the SMGRSensor and SAMSensor class.
============================================================================*/
    virtual int flush() = 0;
/*===========================================================================
  FUNCTION:  flushSendCmplt
    Send the flush complete response
===========================================================================*/
    void flushSendCmplt();
/*============================================================================
  FUNCTION getEnabled
    Get the sensor enabled status.
    Return value
        1 : The sensor is enabled.
        0 : The sensor is not enabled.
============================================================================*/
    int getEnabled();
/*============================================================================
  FUNCTION getTriggerMode
    Get the sensor trigger mode.
    Return value
        SENSOR_MODE_CONT : The sensor mode is continuous.
        SENSOR_MODE_EVENT : The sensor mode is event.
        SENSOR_MODE_SPECIAL : The sensor mode is special.
        SENSOR_MODE_TRIG : The sensor mode is trigger.
============================================================================*/
    sensor_trigger_mode getTriggerMode();
/*============================================================================
  FUNCTION getAttribOK
    Get the is_attrib_ok value.
    Return value
        true : The sensor attribute is OK.
        false : The sensor attribute is not OK. Will not show in the sensor list.
============================================================================*/
    bool getAttribOK();
/*============================================================================
  FUNCTION getName
    Get the sensor name.
    Return value
        @char* : The sensor name string.
============================================================================*/
    char* getName();
/*============================================================================
  FUNCTION getVendor
    Get the sensor vendor.
    Return value
        @char* : The sensor vendor string.
============================================================================*/
    char* getVendor();
/*============================================================================
  FUNCTION getVersion
    Get the sensor version.
    Return value
        int : The sensor version value.
============================================================================*/
    int getVersion();
/*============================================================================
  FUNCTION getHandle
    Get the sensor handle.
    Return value
        @int : The sensor handle value.
============================================================================*/
    int getHandle();
/*============================================================================
  FUNCTION getType
    Get the sensor type.
    Return value
        @int : The sensor type value.
============================================================================*/
    int getType();
/*============================================================================
  FUNCTION getMaxRange
    Get the sensor MAX range.
    Return value
        @float : The sensor MAX range value.
============================================================================*/
    float getMaxRange();
/*============================================================================
  FUNCTION getResolution
    Get the sensor resolution.
    Return value
        @float : The sensor resolution value.
============================================================================*/
    float getResolution();
/*============================================================================
  FUNCTION getPower
    Get the sensor power.
    Return value
        @float : The sensor power value.
============================================================================*/
    float getPower();
/*============================================================================
  FUNCTION getMaxFreq
    Get the sensor MAX frequency.
    Return value
        @float : The sensor MAX frequency value.
============================================================================*/
    float getMaxFreq();
/*============================================================================
  FUNCTION getMinFreq
    Get the sensor MIN frequency.
    Return value
        @float : The sensor MIN frequency value.
============================================================================*/
    float getMinFreq();
/*============================================================================
  FUNCTION getMaxSampleFreq
    Get the sensor MAX sampling frequency.
    Return value
        @float : The sensor MAX sampling frequency value.
============================================================================*/
    float getMaxSampleFreq();
/*============================================================================
  FUNCTION getMinSampleFreq
    Get the sensor MIN sampling frequency.
    Return value
        @float : The sensor MIN sampling frequency value.
============================================================================*/
    float getMinSampleFreq();
/*============================================================================
  FUNCTION getMaxBufferedSamples
    Get the sensor MAX buffered samples.
    Return value
        @int : The sensor MAX buffered samples value.
============================================================================*/
    int getMaxBufferedSamples();
/*============================================================================
  FUNCTION getFlags
    Get the flags like wakeup/continuous/one-shot etc.
    Return value
        @int: Flags like continuous,on-change,wakeup
============================================================================*/
    int getFlags();
/*============================================================================
  FUNCTION getStringType
    Get the string type defined in sensor.h like android.sensor.xxxxx etc.
    Return value
        @string: Sensor name as string.
============================================================================*/
    char * getStringType();
/*============================================================================
  FUNCTION getPermissionString
    Get the permission defined in sensor.h like android.permission.xxx etc.
    Return value
        @string: permission required to see this sensor, register to it and
                    receive data. Set to "" if no permission is required
============================================================================*/
    char * getPermissionString();

/*============================================================================
  FUNCTION setAttribOK
    Set the sensor is_attrib_ok status.
    Parameters:
        @is_attrib_ok: The sensor's is_attrib_ok flag.
============================================================================*/
    void setAttribOK(bool is_attrib_ok);
/*============================================================================
  FUNCTION setName
    Set the sensor name.
    Parameters:
        @name: the sensor name
============================================================================*/
    void setName(const char *name);
/*============================================================================
  FUNCTION setVendor
    Set the sensor vendor.
    Parameters:
        @vendor: the sensor vendor
============================================================================*/
    void setVendor(const char *vendor);
/*============================================================================
  FUNCTION setVersion
    Set the sensor version.
    Parameters:
        @version: the sensor version
============================================================================*/
    void setVersion(int version);
/*============================================================================
  FUNCTION setHandle
    Set the sensor handle.
    Parameters:
        @handle: the sensor handle to be set
============================================================================*/
    void setHandle(int handle);
/*============================================================================
  FUNCTION setType
    Set the sensor type
    Parameters:
        @type: the sensor type
============================================================================*/
    void setType(int type);
/*============================================================================
  FUNCTION setMaxRange
    Set the sensor MAX range.
    Parameters:
        @max_range: the sensor MAX range
============================================================================*/
    void setMaxRange(float max_range);
/*============================================================================
  FUNCTION setResolution
    Set the sensor resolution.
    Parameters:
        @resolution: the sensor resolution
============================================================================*/
    void setResolution(float resolution);
/*============================================================================
  FUNCTION setPower
    Set the sensor power
    Parameters:
        @power: the sensor power
============================================================================*/
    void setPower(float power);
/*============================================================================
  FUNCTION setMaxFreq
    Set the sensor MAX frequency.
    Parameters:
        @max_freq: the sensor MAX frequency
============================================================================*/
    void setMaxFreq(float max_freq);
/*============================================================================
  FUNCTION setMinFreq
    Set the sensor min frequency.
    Parameters:
        @sensor: the sensor min frequency
============================================================================*/
    void setMinFreq(float min_freq);
/*============================================================================
  FUNCTION setMaxSampleFreq
    Set the sensor MAX sampling frequency.
    Parameters:
        @max_freq: the sensor MAX sampling frequency
============================================================================*/
    void setMaxSampleFreq(float max_sample_freq);
/*============================================================================
  FUNCTION setMinSampleFreq
    Set the sensor min sampling frequency.
    Parameters:
        @sensor: the sensor min sampling frequency
============================================================================*/
    void setMinSampleFreq(float min_sample_freq);
/*============================================================================
  FUNCTION setMaxBufferedSamples
    Set the sensor MAX buffered samples.
    Parameters:
        @max_buffered_samples: the sensor MAX buffered samples
============================================================================*/
    void setMaxBufferedSamples(int MAX_buffered_samples);
/*============================================================================
  FUNCTION setFlags
    Set the flags like wakeup/continuous/one-shot etc.
    Parameters:
        @flags: continuous,on-change,wakeup etc
============================================================================*/
    void setFlags(int flags);
/*============================================================================
  FUNCTION setStringType
    Set the string type defined in sensor.h like android.sensor.xxxxx etc.
    Parameters:
        @str_type : sensors name string.
============================================================================*/
    void setStringType(const char* str_type);
/*============================================================================
  FUNCTION setPermissionString
    set the permission defined in sensor.h like android.permission.xxx etc.
    Parameters:
        @permission_str : permissions like "android.permission.BODY_SENSORS"
============================================================================*/
    void setPermissionString(const char* permission_str);
};

#endif
