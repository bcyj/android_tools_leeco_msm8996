/*============================================================================
  @file sns_espplus_testapp.c

  @brief
  Test the QC/Invensense Hybrid approach at the HAL layer.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ==========================================================================*/

#include "hardware/sensors.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define OPTSTRING "h:"
#define COMMAND_CHARS "adsfbe12345678"

#ifdef SNS_LA_SIM
#define HAL_LIB_PATH ""
#else
#define HAL_LIB_PATH32 "/system/vendor/lib/hw/"
#define HAL_LIB_PATH64 "/system/vendor/lib64/hw/"
#endif
#define HAL_LIB_NAME32 HAL_LIB_PATH32"sensors."BOARD_PLATFORM".so"
#define HAL_LIB_NAME64 HAL_LIB_PATH64"sensors."BOARD_PLATFORM".so"

#define NSEC_PER_MS 1000000

struct sensors_module_t *hal_sym;

struct sensors_poll_device_1 *dev;
struct sensors_poll_device_t *dev_old;
struct sensor_t *list = NULL;

typedef enum
  {
    ACTIVATE = 0,
    DEACTIVATE,
    SET_DELAY,
    BATCH,
    EXIT,
    NUM_COMMANDS
  } testapp_command_e;

typedef enum
  {
    INACTIVE = 0,
    STREAMING,
    BATCHING
  } sensor_operating_state_e;

static char *handle_to_string( struct sensor_t *list, int handle )
{
  struct sensor_t *list_p = list;

  while( list_p->name ) {
    if( list_p->handle == handle ) {
      return (char*)list_p->name;
    }
    list_p++;
  }
  return "<no-name>";
}

static char *type_to_string( int type )
{
  return (type == SENSOR_TYPE_ACCELEROMETER) ? "Accel" :
    (type == SENSOR_TYPE_MAGNETIC_FIELD) ? "Mag" :
    (type == SENSOR_TYPE_ORIENTATION) ? "Ori" :
    (type == SENSOR_TYPE_GYROSCOPE) ? "Gyro" :
    (type == SENSOR_TYPE_LIGHT) ? "Light" :
    (type == SENSOR_TYPE_PRESSURE) ? "Press" :
    (type == SENSOR_TYPE_TEMPERATURE) ? "Temp" :
    (type == SENSOR_TYPE_PROXIMITY) ? "Prox" :
    (type == SENSOR_TYPE_GRAVITY) ? "Grav" :
    (type == SENSOR_TYPE_LINEAR_ACCELERATION) ? "Lin_Acc" :
    (type == SENSOR_TYPE_ROTATION_VECTOR) ? "Rot_Vec" :
    (type == SENSOR_TYPE_RELATIVE_HUMIDITY) ? "Rel_Humidity" :
    (type == SENSOR_TYPE_AMBIENT_TEMPERATURE) ? "Amb_Temp" :
    (type == SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED) ? "Uncal_Mag" :
    (type == SENSOR_TYPE_GAME_ROTATION_VECTOR) ? "Game_Rot_Vec" :
    (type == SENSOR_TYPE_GYROSCOPE_UNCALIBRATED) ? "Uncal_Gyro" :
    (type == SENSOR_TYPE_SIGNIFICANT_MOTION) ? "SMD" :
    (type == SENSOR_TYPE_STEP_DETECTOR) ? "Step_Detector" :
    (type == SENSOR_TYPE_STEP_COUNTER) ? "Step_Count" :
    (type == SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR) ? "GeoMag_Rot_Vec" :
#ifdef SENSORS_DEVICE_API_VERSION_1_1
    (type == SENSOR_TYPE_META_DATA) ? "Meta Data (FLUSH_CMPLT)" :
#endif
    /* (type == 14) ? "QC Gestures" :
       (type == 15) ? "QC Gyro Tap" :
       (type == 16) ? "QC Facing" :
       (type == 17) ? "QC Tilt" : */ "Unknown";
}

// Print out the usage statement in interactive mode
void print_usage_msg( struct sensor_t const *list )
{
  int count = 0;
  struct sensor_t const *list_p = list;

  fprintf(stderr,"Sensors HAL TEST APP, version 1\n");
  fprintf(stderr,"Usage:\n");
  fprintf(stderr,"\tChoose a sensor to interact with by inputting the sensor's handle as determined by the value within the square brackets [] in the 'Sensors list'\n");
  fprintf(stderr,"\tNext, choose a command, by inputting one of the following characters:\n");
  fprintf(stderr,"\t\ta - Activate the sensor that was previously chosen.\n");
  fprintf(stderr,"\t\td - Deactivate the sensor.\n");
  fprintf(stderr,"\t\tf - Flush the sensor.\n");
  fprintf(stderr,"\t\ts - Set Delay. The program will prompt for the delay.\n");
  fprintf(stderr,"\t\tb - Batch. The program will prompt for additional information.\n");
  fprintf(stderr,"\t\te - Exit\n");
  fprintf(stderr,"Sensors list:\n");
  while( list_p->name ) {
    fprintf(stderr,"\t[Handle:%2d] Name:%s Vendor:%s Version:%d Type:%d(%s)\n",
            list_p->handle, list_p->name, list_p->vendor, list_p->version, list_p->type,
            type_to_string(list_p->type));
    list_p++;
    count++;
  }
  fprintf(stderr,"\n");
}

// Constantly polls data from HAL
void *hal_poll( void *threadid )
{
#define POLL_NUM_EVENTS 100
  struct sensors_event_t data[POLL_NUM_EVENTS];
  int count;
  int i;
  struct timespec now_rt;
  uint64_t now_ns;

  while( 1 ) {
    count = dev->poll( dev_old, data, POLL_NUM_EVENTS );
    clock_gettime( CLOCK_REALTIME, &now_rt );
    now_ns = (uint64_t)now_rt.tv_sec * 1000000000LL + (uint64_t)now_rt.tv_nsec;

    for( i = 0; i < count ; i++ ) {
#ifdef SENSORS_DEVICE_API_VERSION_1_1
      if( data[i].type == SENSOR_TYPE_META_DATA ) {
        printf("%lld.%06lld, %s/%s, %lld.%06lld %d\n",
               now_ns/NSEC_PER_MS, now_ns%NSEC_PER_MS, "META_DATA",
               (data[i].meta_data.what == META_DATA_FLUSH_COMPLETE) ?"FLUSH_COMPLETE":"Unk",
               data[i].timestamp/NSEC_PER_MS, data[i].timestamp%NSEC_PER_MS,
               data[i].meta_data.sensor );
      } else
#endif /* SENSORS_DEVICE_API_VERSION_1_1 */
      {
        printf("%lld.%06lld, %s/%s, %lld.%06lld, %f, %f, %f, %f\n",
               now_ns/NSEC_PER_MS, now_ns%NSEC_PER_MS,
               type_to_string( data[i].type ), handle_to_string(list, data[i].sensor),
               data[i].timestamp/NSEC_PER_MS, data[i].timestamp%NSEC_PER_MS,
               data[i].data[0], data[i].data[1], data[i].data[2], data[i].data[3] );
      }
    }
    fflush(stdout);
  }
}

int main( int argc, char * const argv[] )
{
  struct sensor_t const *list_p;
  pthread_t poll_thread;
  int num_scanned;
  int sensor_count;
  int handle;
  int handle2;
  int handle3;
  int flags;
  int64_t period;
  int64_t timeout;
  int error;
  int sensor;
  int opt;
  char command;
  bool valid_input = false;
  char input_buffer[80];
  void* hal_lib;

  hal_lib = dlopen( HAL_LIB_NAME32, RTLD_LAZY );

  if( NULL == hal_lib ) {
    fprintf(stderr,"dlopen of 32bit HAL lib (%s) failed\n", HAL_LIB_NAME32 );
    fprintf(stderr,"trying to open 64bit HAL lib (%s) now\n", HAL_LIB_NAME64 );
    hal_lib = dlopen( HAL_LIB_NAME64, RTLD_LAZY );
    if( NULL == hal_lib ) {
      fprintf(stderr,"dlopen of 64bit HAL lib (%s) failed\n", HAL_LIB_NAME64 );
      fprintf(stderr,"exiting now. . .\n", HAL_LIB_NAME64 );
      exit(1);
    }
  }

  dlerror();
  hal_sym = dlsym( hal_lib, HAL_MODULE_INFO_SYM_AS_STR );
  if( dlerror() != NULL ) {
    fprintf(stderr,"dlsym(%s) failed\n", HAL_MODULE_INFO_SYM_AS_STR );
    exit(1);
  }

  if( 0 != hal_sym->common.methods->open( NULL, SENSORS_HARDWARE_POLL, (hw_device_t**)&dev ) )
  {
    fprintf(stderr,"Hal open failure\n");
    exit(1);
  }

  fprintf(stderr,"HAL open\n");
  fprintf(stderr,"HAL module_api_version: 0x%d\n", hal_sym->common.module_api_version );
  fprintf(stderr,"HAL hal_api_version   : 0x%d\n", hal_sym->common.hal_api_version );
  fprintf(stderr,"HAL hal_id            : %s\n", hal_sym->common.id );
  fprintf(stderr,"HAL hal_name          : %s\n", hal_sym->common.name );
  fprintf(stderr,"HAL hal_author        : %s\n", hal_sym->common.author );


  dev_old = (struct sensors_poll_device_t*) dev;
  usleep(50000);
  pthread_create( &poll_thread, NULL, hal_poll, NULL );
  usleep(50000);

  hal_sym->get_sensors_list( NULL, ((struct sensor_t const**)&list) );

  list_p = list;
  fprintf( stderr, "Sensor list:\n");
  while( list_p->name ) {
    fprintf(stderr,"- Name:%s Vendor:%s Version:%d Handle:%d Type:%d(%s)\n\t"
            "maxRange:%f resolution:%f power:%f minDelay:%d\n",
            list_p->name, list_p->vendor, list_p->version, list_p->handle, list_p->type,
            type_to_string(list_p->type), list_p->maxRange, list_p->resolution,
            list_p->power, list_p->minDelay);
    list_p++;
  }

  list_p = list;
  sensor_count = 0;
  while( list_p->name ) {
    list_p++;
    sensor_count++;
  }
  if ( sensor_count == 0 )
  {
    fprintf(stderr,"ERROR: No sensors in the list");
    dev->common.close( (hw_device_t*)dev );
    exit(1);
  }
  // Print
  while( (opt = getopt(argc, argv, OPTSTRING)) != -1 ) {
    if( opt == 'h' || opt == '?') {
      if( opt == '?' ) {
        fprintf(stderr, "Unknown option \'%c\'.\n", optopt);
      }
      print_usage_msg( list );
      exit(1);
    }
  }

  do
  {
    print_usage_msg(list);

    //fflush(NULL);

    // Get the sensor
    do
    {
      fprintf(stderr,"Please choose a listed sensor handle to interact with> ");
      if( NULL == fgets(input_buffer,80,stdin) ) {
        while(1) {
          fprintf(stderr,"Will exit after 600 seconds of EOF\n");
          fprintf(stdout,"Will exit after 600 seconds of EOF\n");
          sleep(600);
          fprintf(stderr,"Exiting after 600 seconds of EOF\n");
          fprintf(stdout,"Exiting after 600 seconds of EOF\n");
          exit(0);
        }
      }
      fflush(stdin);
      num_scanned = sscanf(input_buffer, "%d", &sensor);
      valid_input = true;
      if ( num_scanned != 1 )
      {
        valid_input = false;
        fprintf(stderr,"ERROR: Unknown input.\n");
      }
      else
      {
        list_p = list;
        while( list_p->name ) {
          if( list_p->handle == sensor ) {
            break;
          }
          list_p++;
        }
        if ( !list_p->name ) {
          valid_input = false;
          fprintf(stderr,"ERROR: Sensor out of bounds.\n");
        }
      }
    } while ( !valid_input );

    // Get the command
    do
    {
      fprintf(stderr,"Please choose a command (a,d,s,b,f,e)> ");
      fgets(input_buffer,80,stdin);
      fflush(stdin);
      num_scanned = sscanf(input_buffer, "%c", &command);
      valid_input = true;
      if ( num_scanned != 1 )
      {
        valid_input = false;
        fprintf(stderr,"ERROR: Unknown input.\n");
      }
      else if ( NULL == strchr(COMMAND_CHARS, command) )
      {
        valid_input = false;
        fprintf(stderr,"ERROR: Invalid command.\n");
      }
    } while ( !valid_input );

    switch(command)
    {
      case 'A':
      case 'a':
        //Activate;
        fprintf(stderr,"Activating sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        printf("Activating sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        error = dev->activate( dev_old, list_p->handle, true );
        if( error ) {
          fprintf(stderr,"Error %d activating sensor\n", error );
        }
      break;

      case 'D':
      case 'd':
        //DEACTIVATE;
        fprintf(stderr,"Deactivating sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        printf("Deactivating sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        error = dev->activate( dev_old, list_p->handle, false );
        if( error ) {
          fprintf(stderr,"Error %d deactivating sensor\n", error );
        }
      break;

      case 'S':
      case 's':
        //SET_DELAY;
        do
        {
          fprintf(stderr,"Please specify a delay period (in ms)> ");
          fgets(input_buffer,80,stdin);
          fflush(stdin);
          num_scanned = sscanf(input_buffer, "%lld", &period);
          valid_input = true;
          if ( num_scanned != 1 )
          {
            valid_input = false;
            fprintf(stderr,"ERROR: Unknown input.\n");
          } else {
            period *= 1000000;
          }
        } while ( !valid_input );
        fprintf(stderr,"Setting the delay for sensor %s @ handle %d to %lld ns...\n", list_p->name, list_p->handle, period );
        printf("Setting the delay for sensor %s @ handle %d to %lld ns...\n", list_p->name, list_p->handle, period );
        error = dev->setDelay( dev_old, list_p->handle, period );
        if( error ) {
          fprintf(stderr,"Error %d in setDelay\n", error);
        }
        break;

      case 'B':
      case 'b':
        //BATCH;
        do
        {
          fprintf(stderr,"Please specify any flags (1 for Dry Run, 2 for WuFF)> ");
          fgets(input_buffer,80,stdin);
          fflush(stdin);
          num_scanned = sscanf(input_buffer, "%u", &flags);
          valid_input = true;
          if ( num_scanned != 1 )
          {
            valid_input = false;
            fprintf(stderr,"ERROR: Unknown input.\n");
          }
          else if ( flags > 3 )
          {
            valid_input = false;
            fprintf(stderr,"ERROR: Invalid flags.\n");
          }
        } while ( !valid_input );
        do
        {
          fprintf(stderr,"Please specify a delay period (in ms)> ");
          fgets(input_buffer,80,stdin);
          fflush(stdin);
          num_scanned = sscanf(input_buffer, "%lld", &period);
          valid_input = true;
          if ( num_scanned != 1 )
          {
            valid_input = false;
            fprintf(stderr,"ERROR: Unknown input.\n");
          } else {
            period *= 1000000;
          }
        } while ( !valid_input );
        do
        {
          fprintf(stderr,"Please specify a timeout (in ms)> ");
          fgets(input_buffer,80,stdin);
          fflush(stdin);
          num_scanned = sscanf(input_buffer, "%lld", &timeout);
          valid_input = true;
          if ( num_scanned != 1 )
          {
            valid_input = false;
            fprintf(stderr,"ERROR: Unknown input.\n");
          } else {
            timeout *= 1000000;
          }
        } while ( !valid_input );

        fprintf(stderr,"Setting batch mode for sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        error = dev->batch( dev, list_p->handle, flags, period, timeout );
        if( error ) {
          fprintf(stderr,"Error %d in batch\n", error );
        } else {
          fprintf(stderr,"batch success\n");
        }
        break;

      case 'F':
      case 'f':
        //Flush;
#ifdef SENSORS_DEVICE_API_VERSION_1_1
        fprintf(stderr,"flushing sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        error = dev->flush( dev, list_p->handle );
        if( error ) {
          fprintf(stderr,"Error %d in flush\n", error );
        } else {
          fprintf(stderr,"flush success\n");
        }
#else
        fprintf(stderr,"Flush not supported in this API version\n" );
#endif /* SENSORS_DEVICE_API_VERSION_1_1 */
        break;
      case 'E':
      case 'e':
        //EXIT;
        break;
    }
  } while( command != 'e' && command != 'E' );

  fprintf(stderr,"Exiting ESP+ test app...\n");
  dev->common.close( (hw_device_t*)dev_old );

  return 0;
}
