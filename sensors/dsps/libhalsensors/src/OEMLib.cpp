/*============================================================================
  @file OEMLib.cpp

  @brief
  OEMLib class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <dlfcn.h>
#include "OEMLib.h"
#include "Utility.h"
#include "Light.h"
#include "Proximity.h"

poll_dev_t *OEMLib::OEM_device;

/*============================================================================
  OEMLib Constructor
============================================================================*/
OEMLib::OEMLib()
{
    OEM_module = NULL;
    OEM_device = NULL;
    OEM_poll_thread = 0;
    thread_counter = 0;
    OEM_light = new Light(HANDLE_OEM_LIGHT);
    OEM_proximity = new Proximity(HANDLE_OEM_PROXIMITY);
}

/*============================================================================
  OEMLib Destructor
============================================================================*/
OEMLib::~OEMLib()
{
    delete OEM_light;
    delete OEM_proximity;

    /* close kernel driver instance if enabled */
    if (OEM_device != NULL) {
        int err;
        err = sensors_close( (struct sensors_poll_device_t*)OEM_device );
        if(err != 0) {
            HAL_LOG_ERROR("%s sensors_close() failed %d (%s)",
                          __FUNCTION__, err, strerror(-err));
        }
    }
}

/*============================================================================
  FUNCTION OEMLoadLib
============================================================================*/
int OEMLib::OEMLoadLib()
{
    int status = -1;
    void *oem_hal_lib_handle;
    struct hw_module_t *hmi;

    oem_hal_lib_handle = dlopen(OEM_LIB_PATH, RTLD_NOW);
    if (NULL == oem_hal_lib_handle) {
        HAL_LOG_DEBUG("%s: Could not open OEM HAL library %s", __FUNCTION__, OEM_LIB_PATH );
        return status;
    }

    hmi = (struct hw_module_t *)dlsym(oem_hal_lib_handle, HAL_MODULE_INFO_SYM_AS_STR);
    if (NULL == hmi) {
        HAL_LOG_ERROR("%s: ERROR: Could not find symbol %s", __FUNCTION__,HAL_MODULE_INFO_SYM_AS_STR );
        dlclose(oem_hal_lib_handle);
        oem_hal_lib_handle = NULL;
        return status;
    }

    hmi->dso = oem_hal_lib_handle;
    status = 0;
    OEM_module = (struct sensors_module_t *)hmi;
    HAL_LOG_DEBUG("loaded OEM HAL path=%s hmi=%p handle=%p", OEM_LIB_PATH, hmi, oem_hal_lib_handle);

    /* open the kernel driver instance */
    if (OEM_module != NULL) {
        status = sensors_open(&OEM_module->common, (struct sensors_poll_device_t**)&OEM_device);
        if (status) {
            HAL_LOG_ERROR("%s: OEM sensors_open failed %d", __FUNCTION__, status);
            OEM_module = NULL;
        }
        else {
            HAL_LOG_DEBUG("%s: OEM sensors_open success", __FUNCTION__);
        }
    }

    return status;
}

/*============================================================================
  FUNCTION OEMAddSensors
============================================================================*/
void OEMLib::OEMAddSensors()
{
    struct sensor_t const *OEM_list;
    int num_OEMSensors = 0;
    HAL_LOG_DEBUG("%s", __FUNCTION__ );

    if (NULL == this->OEM_module) {
        HAL_LOG_ERROR("%s : The OEM_module is NULL!", __FUNCTION__);
        return;
    }
    if ((num_OEMSensors = OEM_module->get_sensors_list(OEM_module, &OEM_list)) > 0) {
        int i;
        for (i=0; i < num_OEMSensors; i++) {
            if (SENSOR_TYPE_LIGHT == OEM_list[i].type) {
                HAL_LOG_DEBUG("%s: Got type SENSOR_TYPE_LIGHT", __FUNCTION__);
                OEM_light->setName(OEM_list[i].name);
                OEM_light->setVendor(OEM_list[i].vendor);
                OEM_light->setVersion(OEM_list[i].version);
                OEM_light->setHandle(HANDLE_OEM_LIGHT);
                OEM_light->setType(SENSOR_TYPE_LIGHT);
                OEM_light->setFlags(SENSOR_FLAG_ON_CHANGE_MODE);
                OEM_light->setMaxRange(OEM_list[i].maxRange);
                OEM_light->setResolution(OEM_list[i].resolution);
                OEM_light->setPower(OEM_list[i].power);
                OEM_light->setMaxFreq((OEM_list[i].minDelay == 0)? FREQ_FASTEST_HZ: (1000/(USEC_TO_MSEC(OEM_list[i].minDelay))));
                OEM_light->setMinFreq(1);
                OEM_light->setAttribOK(true);
                OEM_handle[HANDLE_OEM_LIGHT] = OEM_list[i].handle;
            }
            else if (SENSOR_TYPE_PROXIMITY == OEM_list[i].type) {
                HAL_LOG_DEBUG("%s: Got type SENSOR_TYPE_PROXIMITY", __FUNCTION__);
                OEM_proximity->setName(OEM_list[i].name);
                OEM_proximity->setVendor(OEM_list[i].vendor);
                OEM_proximity->setVersion(OEM_list[i].version);
                OEM_proximity->setHandle(HANDLE_OEM_PROXIMITY);
                OEM_proximity->setType(SENSOR_TYPE_PROXIMITY);
                OEM_proximity->setFlags(SENSOR_FLAG_ON_CHANGE_MODE|SENSOR_FLAG_WAKE_UP);
                OEM_proximity->setMaxRange(OEM_list[i].maxRange);
                OEM_proximity->setResolution(OEM_list[i].resolution);
                OEM_proximity->setPower(OEM_list[i].power);
                OEM_proximity->setMaxFreq((OEM_list[i].minDelay == 0)? FREQ_FASTEST_HZ: (1000/(USEC_TO_MSEC(OEM_list[i].minDelay))));
                OEM_proximity->setMinFreq(1);
                OEM_proximity->setAttribOK(true);
                OEM_handle[HANDLE_OEM_PROXIMITY] = OEM_list[i].handle;
            }
        }
    }
    HAL_LOG_DEBUG("%s: get_sensors_list returned num_OEMSensors=%d", __FUNCTION__,num_OEMSensors);
}

/*============================================================================
  FUNCTION OEMActivate
============================================================================*/
int OEMLib::OEMActivate(int handle, int enabled)
{
    int err;
    HAL_LOG_DEBUG("%s %d", __FUNCTION__, enabled);
    if (NULL == OEM_device) {
        HAL_LOG_ERROR("%s : The OEM_Device is NULL!", __FUNCTION__);
        return -1;
    }
    /* activate/deactivate the kernel driver */
    err = OEM_device->activate((struct sensors_poll_device_t*)OEM_device, handle, enabled);
    if (err != 0) {
        HAL_LOG_ERROR( "%s: OEM activate() for handle %d failed with error %d (%s)", __FUNCTION__, handle, err, strerror(-err));
        return -1;
    }
    return 0;
}

/*============================================================================
  FUNCTION OEMSetDelay
============================================================================*/
int OEMLib::OEMSetDelay(int handle, int64_t ns)
{
    int err;
    HAL_LOG_DEBUG("%s %"PRId64, __FUNCTION__, ns);
    if (NULL == OEM_device) {
        HAL_LOG_ERROR("%s : The OEM_Device is NULL!", __FUNCTION__);
        return -1;
    }
    err = OEM_device->setDelay((struct sensors_poll_device_t*)OEM_device, handle, ns);
    if (err != 0) {
        HAL_LOG_ERROR("%s: OEM setDelay() for handle %d failed with error %d (%s)", __FUNCTION__, handle, err, strerror(-err));
        return -1;
    }
    return 0;
}

/*============================================================================
  FUNCTION OEMBatch
============================================================================*/
int OEMLib::OEMBatch(int handle, int flags, int64_t ns, int64_t timeout)
{
    int err;
    if (NULL == OEM_device) {
        HAL_LOG_ERROR("%s : The OEM_Device is NULL!", __FUNCTION__);
        return -1;
    }
    err = OEM_device->batch(OEM_device, handle, flags, ns, timeout);
    if (err != 0) {
        HAL_LOG_ERROR("%s: OEM batch() for handle %d failed with error %d (%s)", __FUNCTION__, handle, err, strerror(-err));
        return -1;
    }
    return 0;
}

/*============================================================================
  FUNCTION OEMFlush
============================================================================*/
int OEMLib::OEMFlush(int handle)
{
    int err;
    if (NULL == OEM_device) {
        HAL_LOG_ERROR("%s : The OEM_Device is NULL!", __FUNCTION__);
        return -1;
    }
    err = OEM_device->flush(OEM_device, handle);
    if (err != 0) {
        HAL_LOG_ERROR("%s: OEMFlush() for handle %d failed with error %d (%s)", __FUNCTION__, handle, err, strerror(-err));
        return -1;
    }
    return 0;
}

/*============================================================================
  FUNCTION OEMDataPoll
============================================================================*/
void* OEMLib::OEMDataPoll(void *ptr)
{
    UNREFERENCED_PARAMETER(ptr);
    sensors_event_t buf[2];
    int n;
    int count;
    hal_data_cb_t *data_cb = Utility::getDataCb();
    HAL_LOG_INFO("%s", __FUNCTION__);

    if (NULL == OEM_device) {
        HAL_LOG_ERROR("%s : The OEM_Device is NULL!", __FUNCTION__);
        return 0;
    }

    while(1) {
        n = OEM_device->poll((struct sensors_poll_device_t*)OEM_device, buf, 2);
        if (n< 0) {
            HAL_LOG_ERROR("%s: poll() failed", __FUNCTION__ );
            break;
        }
        for (count = 0; count < n; count++) {
            if (buf[count].type != SENSOR_TYPE_LIGHT && buf[count].type != SENSOR_TYPE_PROXIMITY)
                continue;

            if (SENSOR_TYPE_LIGHT == buf[count].type) {
                buf[count].sensor = HANDLE_OEM_LIGHT;
                HAL_LOG_DEBUG("%s: Received LIGHT DATA loopCnt-%d light value-%f", __FUNCTION__,count, buf[count].light);
            }
            if (SENSOR_TYPE_PROXIMITY == buf[count].type) {
                buf[count].sensor = HANDLE_OEM_PROXIMITY;
                HAL_LOG_DEBUG("%s: Received PROXIMITY DATA loopCnt-%d prox value-%f", __FUNCTION__,count, buf[count].distance);
            }
            pthread_mutex_lock(&data_cb->data_mutex);
            if (Utility::insertQueue(&buf[count])) {
                Utility::signalInd(data_cb);
            }
            pthread_mutex_unlock(&data_cb->data_mutex);
        }
    }
    return 0;
}

/*============================================================================
  FUNCTION OEMCreateThread
============================================================================*/
void OEMLib::OEMCreateThread()
{
    pthread_attr_t thread_attr;
    if (this->thread_counter == 0) {
        if (0 == pthread_attr_init(&thread_attr)) {
            if (0 == pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED)) {
                pthread_create(&OEM_poll_thread, &thread_attr, OEMLib::OEMDataPoll, NULL);
                HAL_LOG_INFO("%s: Created OEM poll thread", __FUNCTION__);
                this->thread_counter = 1;
            }
            pthread_attr_destroy(&thread_attr);
        }
    }
}

/*============================================================================
  FUNCTION getOEMProximity
============================================================================*/
Sensor* OEMLib::getOEMProximity()
{
    return this->OEM_proximity;
}

/*============================================================================
  FUNCTION getOEMLight
============================================================================*/
Sensor* OEMLib::getOEMLight()
{
    return this->OEM_light;
}

/*============================================================================
  FUNCTION getOEMSensorHandle
============================================================================*/
int OEMLib::getOEMSensorHandle(int handle)
{
    return this->OEM_handle[handle];
}
