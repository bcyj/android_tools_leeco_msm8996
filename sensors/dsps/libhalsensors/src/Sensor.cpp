/*============================================================================
  @file Sensor.cpp

  @brief
  Sensor class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "Sensor.h"
#include "Utility.h"
#include "SensorsContext.h"

/*============================================================================
  Sensor constructor
============================================================================*/
Sensor::Sensor(int handle)
    :enabled(0),
    freq(0),
    report_rate(0),
    batching(false),
    flush_requested(0),
    is_attrib_ok(false),
    max_buffered_samples(0)
{
    this->handle = handle;
}

/*============================================================================
  Sensor destructor
============================================================================*/
Sensor::~Sensor()
{

}

/*============================================================================
  FUNCTION calcSampleRate
    Calculate the sensor sample rate
    Parameters:
    @ns : the sensor report data delay time
============================================================================*/
float Sensor::calcSampleRate(uint64_t ns)
{
    float sample_rate = 0;
    if (0 == this->max_freq)
        this->max_freq = 1;

    if (this->trigger_mode != SENSOR_MODE_CONT) {
        sample_rate = this->max_freq;
    }
    /* Convert frequency from Android values to sensor1 values. Only support
     * rates between "fastest" and minimun support by the sensor. */
    else if (ns <= 65535000000) {
        if(ns == 0) {
            ns = 1;
        }
        /* frequency in Hz */
        sample_rate = NSEC_TO_HZ(ns);
        /* limit rate to min allowed */
        if(sample_rate < this->min_freq) {
            sample_rate = this->min_freq;
        }
        /* limit rate to max allowed */
        if(sample_rate > this->max_freq) {
            sample_rate = this->max_freq;
        }
    }
    else {
        /* default to minimum supported freq */
        sample_rate = this->min_freq;
    }
    return sample_rate;
}

/*===========================================================================
  FUNCTION:  flushSendCmplt
    Send the flush complete response for the sensor
===========================================================================*/
void Sensor::flushSendCmplt()
{
    sensors_event_t flush_evt;
    flush_evt.version = META_DATA_VERSION;
    flush_evt.sensor = 0;
    flush_evt.type = SENSOR_TYPE_META_DATA;
    flush_evt.reserved0 = 0;
    flush_evt.timestamp = 0;
    flush_evt.meta_data.what = META_DATA_FLUSH_COMPLETE;
    flush_evt.meta_data.sensor = handle;
    if (flush_requested > 0) {
        HAL_LOG_DEBUG("%s: handle=%d", __FUNCTION__, handle);
        flush_requested--;
        if (Utility::insertQueue(&flush_evt)){
            Utility::signalInd(data_cb);
        }
    }
}

/*============================================================================
  FUNCTION getEnabled
    Get the sensor enabled status.
============================================================================*/
int Sensor::getEnabled()
{
    return this->enabled;
}

/*============================================================================
  FUNCTION getTriggerMode
    Get the sensor trigger mode.
        SENSOR_MODE_CONT
        SENSOR_MODE_EVENT
        SENSOR_MODE_SPECIAL
        SENSOR_MODE_TRIG
============================================================================*/
sensor_trigger_mode Sensor::getTriggerMode()
{
    return this->trigger_mode;
}

/*============================================================================
  FUNCTIONS
    Get and Set functions for various Sensor attributes.
============================================================================*/
char* Sensor::getName()
{
    return this->name;
}

char* Sensor::getVendor()
{
    return this->vendor;
}

int Sensor::getVersion()
{
    return this->version;
}

int Sensor::getHandle()
{
    return this->handle;
}

int Sensor::getType()
{
    return this->type;
}

float Sensor::getMaxRange()
{
    return this->max_range;
}

float Sensor::getResolution()
{
    return this->resolution;
}

float Sensor::getPower()
{
    return this->power;
}

float Sensor::getMaxFreq()
{
    return this->max_freq;
}

float Sensor::getMinFreq()
{
    return this->min_freq;
}

float Sensor::getMaxSampleFreq()
{
    return this->max_sample_freq;
}

float Sensor::getMinSampleFreq()
{
    return this->min_sample_freq;
}

int Sensor::getMaxBufferedSamples()
{
    return this->max_buffered_samples;
}

bool Sensor::getAttribOK()
{
    return this->is_attrib_ok;
}

int Sensor::getFlags()
{
    return this->flags;
}

char* Sensor::getStringType()
{
    return this->strtype;
}

char * Sensor::getPermissionString()
{
    return this->reqpermission;
}

void Sensor::setName(const char *name)
{
    strlcpy(this->name, name, SNS_MAX_SENSOR_NAME_SIZE);
}

void Sensor::setVendor(const char *vendor)
{
    strlcpy(this->vendor, vendor, SNS_MAX_SENSOR_NAME_SIZE);
}

void Sensor::setVersion(int version)
{
    this->version = version;
}

void Sensor::setHandle(int handle)
{
    this->handle = handle;
}

void Sensor::setType(int type)
{
    this->type = type;
}

void Sensor::setMaxRange(float max_range)
{
    this->max_range = max_range;
}

void Sensor::setResolution(float resolution)
{
    this->resolution = resolution;
}

void Sensor::setPower(float power)
{
    this->power = power;
}

void Sensor::setMaxFreq(float max_freq)
{
    this->max_freq = max_freq;
}

void Sensor::setMinFreq(float min_freq)
{
    this->min_freq = min_freq;
}

void Sensor::setMaxSampleFreq(float max_sample_freq)
{
    this->max_sample_freq = max_sample_freq;
}

void Sensor::setMinSampleFreq(float min_sample_freq)
{
    this->min_sample_freq = min_sample_freq;
}

void Sensor::setMaxBufferedSamples(int max_buffered_samples)
{
    this->max_buffered_samples = max_buffered_samples;
}

void Sensor::setAttribOK(bool is_attrib_ok)
{
    this->is_attrib_ok = is_attrib_ok;
}

void Sensor::setFlags(int flags)
{
    this->flags = flags;
}

void Sensor::setStringType(const char* str_type)
{
    strlcpy(this->strtype,str_type,SNS_MAX_SENSOR_NAME_SIZE);
}

void Sensor::setPermissionString(const char* permission_str)
{
    strlcpy(this->reqpermission,permission_str,SNS_MAX_PERMISSION_SIZE);
}
