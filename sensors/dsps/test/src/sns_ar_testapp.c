/*============================================================================
  @file sns_ar_testapp.c

  @brief
  Test for the AR HAL layer.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ==========================================================================*/

#include "hardware/activity_recognition.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define OPTSTRING "h:"
#define COMMAND_CHARS "edfx12345678"

#define HAL_LIB_PATH32 "/system/vendor/lib/hw/"
#define HAL_LIB_PATH64 "/system/vendor/lib64/hw/"

#define HAL_LIB_NAME32 HAL_LIB_PATH32"activity_recognition."BOARD_PLATFORM".so"
#define HAL_LIB_NAME64 HAL_LIB_PATH64"activity_recognition."BOARD_PLATFORM".so"

#define NSEC_PER_MS 1000000

activity_recognition_device_t *dev;
activity_recognition_module_t *hal_sym;
char **list = NULL;

typedef enum
{
    ENABLE = 0,
    DISABLE,
    FLUSH,
    EXIT,
    NUM_COMMANDS
} testapp_command_e;

static void testapp_cb(const struct activity_recognition_callback_procs* procs,
                       const activity_event_t* events, int count) {
    int i = 0;
    fprintf(stderr,"\n%s: count: %d", __FUNCTION__, count);

    for(i = 0; i < count; i++) {
        fprintf(stderr,"\n%s: activity: %d event_type: %d timestamp: %lld",
                __FUNCTION__, events->activity, events->event_type, events->timestamp);
        events++;
    }
}

activity_recognition_callback_procs_t cb_procs = {
    .activity_callback = testapp_cb,
};

/* Print out the usage statement in interactive mode */
void print_usage_msg( char **list, int num_activities )
{
    int i;
    char const **list_p = list;

    fprintf(stderr,"AR HAL TEST APP, version 1\n");
    fprintf(stderr,"Usage:\n");
    fprintf(stderr,"\tChoose a activity to interact with by inputting the activity handle as determined by the value within the square brackets [] in the 'Activities list'\n");
    fprintf(stderr,"\tNext, choose a command, by inputting one of the following characters:\n");
    fprintf(stderr,"\t\te - Enable the activity event. The program will prompt for additional information.\n");
    fprintf(stderr,"\t\td - Disable the activity event. The program will prompt for additional information.\n");
    fprintf(stderr,"\t\tf - Flush. This will Flush all the batch FIFOs\n");
    fprintf(stderr,"\t\tx - Exit\n");
    fprintf(stderr,"Activities list:\n");
    for(i=0; i < num_activities; i++) {
        fprintf(stderr,"\t[Handle:%2d] Name:%s \n", i, list_p[i]);
    }
    fprintf(stderr,"\n");
}

int main( int argc, char * const argv[] )
{
    int num_activities;
    int i;
    uint32_t activity;
    uint32_t event_type;
    int64_t timeout;
    int error;
    int opt;
    char command;
    bool valid_input = false;
    char input_buffer[80];
    void *hal_lib;
    char **list_p;
    int num_scanned;

    fprintf(stderr,"Opening 32bit HAL lib (%s)\n", HAL_LIB_NAME32 );
    hal_lib = dlopen( HAL_LIB_NAME32, RTLD_LAZY );

    if( NULL == hal_lib ) {
        fprintf(stderr,"dlopen of 32bit HAL lib (%s) failed!\n", HAL_LIB_NAME32 );
        fprintf(stderr,"trying to open 64bit HAL lib (%s) now. . .\n", HAL_LIB_NAME64 );
        hal_lib = dlopen( HAL_LIB_NAME64, RTLD_LAZY );
        if( NULL == hal_lib ) {
            fprintf(stderr,"dlopen of 64bit HAL lib (%s) failed as well!\n", HAL_LIB_NAME64 );
            fprintf(stderr,"exiting. . . check HAL lib path!\n");
            exit(1);
        }
    }

    dlerror();
    hal_sym = dlsym( hal_lib, HAL_MODULE_INFO_SYM_AS_STR );
    if( dlerror() != NULL ) {
        fprintf(stderr,"dlsym(%s) failed\n", HAL_MODULE_INFO_SYM_AS_STR );
        exit(1);
    }

    if( 0 != hal_sym->common.methods->open( NULL,
        ACTIVITY_RECOGNITION_HARDWARE_INTERFACE, (hw_device_t**)&dev ) ) {
        fprintf(stderr,"Hal open failure\n");
        exit(1);
    }

    fprintf(stderr,"HAL open\n");
    fprintf(stderr,"HAL module_api_version: 0x%d\n", hal_sym->common.module_api_version );
    fprintf(stderr,"HAL hal_api_version   : 0x%d\n", hal_sym->common.hal_api_version );
    fprintf(stderr,"HAL hal_id            : %s\n", hal_sym->common.id );
    fprintf(stderr,"HAL hal_name          : %s\n", hal_sym->common.name );
    fprintf(stderr,"HAL hal_author        : %s\n", hal_sym->common.author );

    num_activities = hal_sym->get_supported_activities_list( NULL, ((char const* const**)&list) );

    if ( num_activities == 0 ) {
        fprintf(stderr,"ERROR: No activities in the list");
        dev->common.close( (hw_device_t*)dev );
        exit(1);
    }

    list_p = list;
    fprintf( stderr, "Number of supported activities: %d\n", num_activities);
    fprintf( stderr, "Supported Activities:\n");
    for( i=0; i < num_activities; i++) {
        fprintf(stderr,"- Name:%s \n", list_p[i]);
    }

    dev->register_activity_callback( dev, &cb_procs );

    /* Print */
    while( (opt = getopt(argc, argv, OPTSTRING)) != -1 ) {
        if( opt == 'h' || opt == '?') {
            if( opt == '?' ) {
                fprintf(stderr, "Unknown option \'%c\'.\n", optopt);
            }
            print_usage_msg( list, num_activities );
            exit(1);
        }
    }

    do
    {
        print_usage_msg(list, num_activities);

        //fflush(NULL);

        /* Get the activity */
        do {
            fprintf(stderr,"Please choose a listed activity handle to interact with> ");
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
            num_scanned = sscanf(input_buffer, "%d", &activity);
            valid_input = true;
            if ( num_scanned != 1 ) {
                valid_input = false;
                fprintf(stderr,"ERROR: Unknown input.\n");
            }
            else {
                for( i = 0; i < num_activities; i++ ) {
                    if( i == activity ) {
                        break;
                    }
                }
                /*
                if ( activity > num_activities - 1 ||
                     activity < 0 ||
                     !list_p[activity] ) {
                    valid_input = false;
                    fprintf(stderr,"ERROR: Activity out of bounds.\n");
                }
                */
            }
        } while ( !valid_input );

        /* Get the command */
        do {
            fprintf(stderr,"Please choose a command (e,d,f,x)> ");
            fgets(input_buffer,80,stdin);
            fflush(stdin);
            num_scanned = sscanf(input_buffer, "%c", &command);
            valid_input = true;
            if ( num_scanned != 1 ) {
                valid_input = false;
                fprintf(stderr,"ERROR: Unknown input.\n");
            }
            else if ( NULL == strchr(COMMAND_CHARS, command) ) {
                valid_input = false;
                fprintf(stderr,"ERROR: Invalid command.\n");
            }
        } while ( !valid_input );

        switch(command) {
            case 'E':
            case 'e':
              /* Enable Activity Event */
              /* Get event_type */
              do {
                  fprintf(stderr,"\nPlease specify event type (1 for ENTER, 2 for EXIT)> ");
                  fgets(input_buffer,80,stdin);
                  fflush(stdin);
                  num_scanned = sscanf(input_buffer, "%u", &event_type);
                  valid_input = true;
                  if ( num_scanned != 1 ) {
                    valid_input = false;
                    fprintf(stderr,"ERROR: Unknown input.\n");
                  }
                  /*
                  else if ( event_type > 2 || event_type < 1 ) {
                    valid_input = false;
                    fprintf(stderr,"ERROR: Invalid event type.\n");
                  }
                  */
              } while ( !valid_input );

              /* Get max_batch_report_latency */
              do {
                  fprintf(stderr,"Please specify a timeout (in ms)> ");
                  fgets(input_buffer,80,stdin);
                  fflush(stdin);
                  num_scanned = sscanf(input_buffer, "%lld", &timeout);
                  valid_input = true;
                  if ( num_scanned != 1 ) {
                    valid_input = false;
                    fprintf(stderr,"ERROR: Unknown input.\n");
                  } else {
                    timeout *= 1000000;
                  }
              } while ( !valid_input );

              fprintf(stderr,"Enable Activity %s @ handle %d ...\n", list_p[activity], activity);
              printf("Enabling Activity %s @ handle %d ...\n", list_p[activity], activity);
              error = dev->enable_activity_event( dev, activity, event_type, timeout );
              if( error ) {
                  fprintf(stderr,"Error %d enabling activity\n", error );
              }
              break;

            case 'D':
            case 'd':
              /* Disable Activity Event */
              /* Get event_type */
              do {
                  fprintf(stderr,"Please specify event type (1 for ENTER, 2 for EXIT)> ");
                  fgets(input_buffer,80,stdin);
                  fflush(stdin);
                  num_scanned = sscanf(input_buffer, "%u", &event_type);
                  valid_input = true;
                  if ( num_scanned != 1 ) {
                    valid_input = false;
                    fprintf(stderr,"ERROR: Unknown input.\n");
                  }
                  else if ( event_type > 2 || event_type < 1 ) {
                    valid_input = false;
                    fprintf(stderr,"ERROR: Invalid event type.\n");
                  }
              } while ( !valid_input );

              fprintf(stderr,"Dsable Activity %s @ handle %d ...\n", list_p[activity], activity);
              printf("Disabling Activity %s @ handle %d ...\n", list_p[activity], activity);
              error = dev->disable_activity_event( dev, activity, event_type );
              if( error ) {
                  fprintf(stderr,"Error %d disabling activity\n", error );
              }
              break;

            case 'F':
            case 'f':
              /* Flush */
              fprintf(stderr,"flushing activity %s @ handle %d ...\n", list_p[activity], activity);
              error = dev->flush( dev );
              if( error ) {
                  fprintf(stderr,"Error %d in flush\n", error );
              } else {
                  fprintf(stderr,"flush success\n");
              }
              break;
            case 'X':
            case 'x':
              /* EXIT */
              break;
        }
    } while( command != 'x' && command != 'X' );

    fprintf(stderr,"Exiting AR test app...\n");
    dev->common.close( (hw_device_t*)dev );
    dlclose(hal_lib);

    return 0;
}
