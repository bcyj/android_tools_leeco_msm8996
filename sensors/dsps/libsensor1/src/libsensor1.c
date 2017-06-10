/*============================================================================
  @file libsensor1.c

  @brief
    This implements the remoting for sensor1 APIs using Linux sockets.

  <br><br>

  DEPENDENCIES: Linux

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential
  ============================================================================*/

/*============================================================================

  INCLUDE FILES

  ============================================================================*/
//#include "sensor1.h"
#include "libsensor1.h"
/*
#include "sns_common_v01.h"
#include "sns_smgr_api_v01.h"
#include "sns_sam_amd_v01.h"
#include "sns_sam_vmd_v01.h"
#include "sns_sam_rmd_v01.h"
#include "sns_sam_fns_v01.h"
#include "sns_sam_bte_v01.h"
#include "sns_sam_quaternion_v01.h"
#include "sns_sam_gravity_vector_v01.h"
#include "sns_reg_api_v01.h"
#include "sns_reg_api_v02.h"
#include "sns_debug_interface_v01.h"
#include "sns_sam_rotation_vector_v01.h"
#include "sns_sam_filtered_mag_v01.h"
#include "sns_sam_mag_cal_v01.h"
#include "sns_sam_sensor_thresh_v01.h"
#include "sns_time_api_v01.h"
#include "sns_sam_orientation_v01.h"
#include "sns_time_api_v02.h"
#include "sns_sam_sensor_thresh_v01.h"
#include "sns_sam_basic_gestures_v01.h"
#include "sns_sam_tap_v01.h"
#include "sns_sam_facing_v01.h"
#include "sns_sam_integ_angle_v01.h"
#include "sns_sam_gyro_tap2_v01.h"
#include "sns_sam_gyrobuf_v01.h"
#include "sns_sam_gyroint_v01.h"
#include "sns_sam_ped_v01.h"
#include "sns_sam_pam_v01.h"
#include "sns_sam_cmc_v02.h"
#include "sns_sam_distance_bound_v01.h"
#include "sns_sam_smd_v01.h"
#include "sns_sam_game_rotation_vector_v01.h"
#include "sns_sam_tilt_detector_v01.h"
#include "sns_smgr_restricted_api_v01.h"
#include "sns_sam_dpc_v01.h"
#include "sns_sam_event_gated_sensor_v01.h"

//#include "qmi_idl_lib.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/system_properties.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>

#define LOG_TAG "libsensor1"
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NVDEBUG 0
#include <cutils/log.h>
#include <common_log.h>

*/
/*============================================================================
  Externalized Function Definitions
  ============================================================================*/
/*===========================================================================

  FUNCTION:   sensor1_open

  ===========================================================================*/
/*
typedef int sensor1_handle_s;  
typedef int sensor1_error_e;
typedef void* sensor1_notify_data_cb_t;
typedef void* intptr_t;
*/
sensor1_error_e
sensor1_open( sensor1_handle_s **hndl,
              sensor1_notify_data_cb_t data_cbf,
              intptr_t cb_data )
{
  UNREFERENCED_PARAMETER(hndl);
  UNREFERENCED_PARAMETER(data_cbf);
  UNREFERENCED_PARAMETER(cb_data);
  return 0;
}

/*===========================================================================

  FUNCTION:   sensor1_close

  ===========================================================================*/
sensor1_error_e
sensor1_close( sensor1_handle_s *hndl )
{
  UNREFERENCED_PARAMETER(hndl);
  return SENSOR1_SUCCESS;
}


/*===========================================================================

  FUNCTION:   sensor1_write

  ===========================================================================*/
sensor1_error_e
sensor1_write( sensor1_handle_s     *hndl,
               sensor1_msg_header_s *msg_hdr,
               void                 *msg_ptr )
{
  UNREFERENCED_PARAMETER(hndl);
  UNREFERENCED_PARAMETER(msg_hdr);
  UNREFERENCED_PARAMETER(msg_ptr);
  return SENSOR1_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sensor1_writable

  ===========================================================================*/
sensor1_error_e
sensor1_writable( sensor1_handle_s  *hndl,
                  sensor1_write_cb_t cbf,
                  intptr_t           cb_data,
                  uint32_t           service_number )
{
  UNREFERENCED_PARAMETER(hndl);
  UNREFERENCED_PARAMETER(cbf);
  UNREFERENCED_PARAMETER(cb_data);
  UNREFERENCED_PARAMETER(service_number);
  return SENSOR1_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sensor1_alloc_msg_buf

  ===========================================================================*/
sensor1_error_e
sensor1_alloc_msg_buf(sensor1_handle_s *hndl,
                      uint16_t          size,
                      void            **buffer )
{
  UNREFERENCED_PARAMETER(hndl);

  UNREFERENCED_PARAMETER(size);
  UNREFERENCED_PARAMETER(buffer);
  return SENSOR1_SUCCESS;
}


/*===========================================================================

  FUNCTION:   sensor1_free_msg_buf

  ===========================================================================*/
sensor1_error_e
sensor1_free_msg_buf(sensor1_handle_s *hndl,
                     void             *msg_buf )
{
  UNREFERENCED_PARAMETER(hndl);

  UNREFERENCED_PARAMETER(msg_buf);
  return SENSOR1_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sensor1_init_once

  ===========================================================================*/
void
sensor1_init_once( void )
{

}


/*===========================================================================

  FUNCTION:   sensor1_init

  ===========================================================================*/
sensor1_error_e
sensor1_init( void )
{
  return SENSOR1_SUCCESS;
}

