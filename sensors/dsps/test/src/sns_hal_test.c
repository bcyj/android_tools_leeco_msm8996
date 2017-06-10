/*============================================================================
  @file sns_hal_test.c

  @brief
    Test the QC implementation of the Android sensors HAL.

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ==========================================================================*/

#include <stdlib.h>
#include "hardware/sensors.h"
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

extern struct sensors_module_t HAL_MODULE_INFO_SYM;

struct sensors_poll_device_t *dev;


void *hal_poll( void *threadid )
{
  struct sensors_event_t data[10];
  int count;
  int i;
  while( 1 ) {
    count = dev->poll( dev, data, 10 );
    for( i = 0; i < count ; i++ ) {
      printf("%s: x %f; y %f; z %f\n",
             (data[i].type == SENSOR_TYPE_ACCELEROMETER) ? "Accel" :
             (data[i].type == SENSOR_TYPE_MAGNETIC_FIELD) ? "Mag" :
             (data[i].type == SENSOR_TYPE_ORIENTATION) ? "Ori" :
             (data[i].type == SENSOR_TYPE_GYROSCOPE) ? "Gyro" :
             (data[i].type == SENSOR_TYPE_LIGHT) ? "Light" :
             (data[i].type == SENSOR_TYPE_PRESSURE) ? "Press" :
             (data[i].type == SENSOR_TYPE_TEMPERATURE) ? "Temp" :
             (data[i].type == SENSOR_TYPE_PROXIMITY) ? "Prox" :
             (data[i].type == SENSOR_TYPE_GRAVITY) ? "Grav" :
             (data[i].type == SENSOR_TYPE_LINEAR_ACCELERATION) ? "Lin Acc" :
             (data[i].type == SENSOR_TYPE_ROTATION_VECTOR) ? "Rot" :
             (data[i].type == SENSOR_TYPE_RELATIVE_HUMIDITY) ? "Rel Humidity" :
             (data[i].type == SENSOR_TYPE_AMBIENT_TEMPERATURE) ? "Amb Temp" :
             (data[i].type == 14) ? "QC Gestures" :
             (data[i].type == 15) ? "QC Gyro Tap" :
             (data[i].type == 16) ? "QC Facing" :
             (data[i].type == 17) ? "QC Tilt" : "Unknown",
             data[i].acceleration.x, data[i].acceleration.y, data[i].acceleration.z );
    }
  }
}

int main( void )
{
  struct sensor_t const *list;
  struct sensor_t const *list_p;
  pthread_t poll_thread;
  int i;
  int sensor_count;
  int sensor;

  if( 0 != HAL_MODULE_INFO_SYM.common.methods->open( NULL, SENSORS_HARDWARE_POLL, (hw_device_t**)&dev ) )
  {
    printf("Hal open failure\n");
    exit(1);
  }
  sleep(1);
  pthread_create( &poll_thread, NULL, hal_poll, NULL );
  sleep(1);

  HAL_MODULE_INFO_SYM.get_sensors_list( NULL, ((struct sensor_t const**)&list) );

  list_p = list;
  sensor_count = 0;
  printf("Sensors list\n:");
  while( list_p->name ) {
    printf("\tName:%s Vendor:%s Version:%d Handle:%d Type:%d\n",
           list_p->name, list_p->vendor, list_p->version, list_p->handle, list_p->type);
    list_p++;
    sensor_count++;
  }

  for( i = 0; i < 100; i++ ) {
    sensor = rand() % sensor_count;
    dev->activate( dev, list[sensor].handle ,true /*enabled*/);
    /* Android always ask for 5Hz first, followed by another rate */
    dev->setDelay( dev, list[sensor].handle, 200000000 );
    dev->setDelay( dev, list[sensor].handle, 100000000 );
    sleep(1);
    dev->activate( dev, list[sensor].handle ,false /*enabled*/);
    /* For some reason, Android ICS also calls setDelay even after
       disabling a sensor */
    dev->setDelay( dev, list[sensor].handle, 100000000 );
    usleep(1000);
  }


  pthread_cancel( poll_thread );
  dev->common.close( (hw_device_t*)dev );

  return 0;
}
