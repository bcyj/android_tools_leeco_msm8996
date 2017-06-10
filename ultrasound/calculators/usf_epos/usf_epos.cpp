/*===========================================================================
                           usf_epos.cpp

DESCRIPTION: Implementation of the EPOS daemon.


INITIALIZATION AND SEQUENCING REQUIREMENTS:
  If not started through java app then make sure to have
  correct /data/usf/epos/usf_epos.cfg file linked to the wanted cfg file
  placed in /data/usf/epos/cfg/.
  In addition, make sure to have the
  /data/usf/pnt_epos/epos_calib.dat file.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_epos"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <stdlib.h>
#include <fcntl.h>
#include <ual.h>
#include <cutils/properties.h>
#include <usf_epos_feedback_handlers.h>
#include "ual_util_frame_file.h"
#include "usf_coord_transformer.h"
#include <semaphore.h>
#include "usf_epos_dynamic_lib_proxy.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define LINK_CFG_FILE_LOCATION "/data/usf/epos/usf_epos.cfg"
#define USF_DSP_VER_FILE "/data/usf/epos/usf_dsp_ver.txt"
#define FRAME_FILE_DIR_PATH "/data/usf/epos/rec/"
#define BUFFER_SIZE 500
#define X_COL 0
#define Y_COL 1
#define MAPPED 0
#define RAW 1
#define EPOS_PACKET_SIZE_DWORDS 256
#define STATISTIC_PRINT_INTERVAL 10000
#define POINTS_STATISTICS_PRINT_INTERVAL 500
#define SOCKET_PROBLEM_MSG_INTERVAL 5000
#define EXIT_SIGTERM 3
// Factor on-screen is always 1 to 1
#define ON_SCREEN_X_FACTOR 1
#define ON_SCREEN_Y_FACTOR 1
// Defines for enabling/disabling touchscreen
enum ts_state_t
{
  ENABLED_TS = 0,
  DISABLED_TS = 1
};
// Diff between low and high threshold for enabling/disabling
// touchscreen. Units of 1/1000 cm
#define ENABLE_TS_Z_THRESHOLDS_DIFF 500
#define MAX_TIME_WITHOUT_VALID_POINT_ENABLE_TS_MSEC 500
// Shift for library reports in Q15 format
#define Q15_SHIFT 15

#define EPOS_LIB_TAG "EPOS Lib"

enum epos_point_dest
{
  DEST_UAL = 0x01,
  DEST_SOCKET = 0x02,
};

/**
  MS display orientation
*/
static const int ORIENTATION_90 =  1;
static const int ORIENTATION_270 = 3;

/**
  min and max values of the tilts in X,Y axis
*/
static const int MIN_TILT = -90;
static const int MAX_TILT = 90;

/**
  min and max values of pressure on surface
*/
static const int MIN_PRESSURE = 0;
static const int MAX_PRESSURE = 2047;

/**
  version buffer size
*/
static const uint16_t VERSION_SIZE = 20;

/**
  Epos calculator version
*/
static const char *CLIENT_VERSION = "1.5.5";

static const char *DEFAULT_EPOS_CALIB_FILES[] = {
  "/persist/usf/epos/product_calib.dat",
  "/persist/usf/epos/unit_calib.dat",
  "/data/usf/epos/persistent_calib.dat",
  "/data/usf/epos/series_calib.dat"
};

/**
  The enabled state touchscreen
*/
static ts_state_t ts_state_enable = ENABLED_TS;

/**
 * The name of the file containg the pid of the daemon
 */
static const char* PID_FILE_NAME = "usf_epos.pid";

/**
  The daemon running control
*/
static volatile bool sb_run = true;

/**
 * The information for the creation of the off screen
 * transformation matrix.
 */
static transformation_properties off_screen_trans_props;

/**
 * The information for the creation of the on screen
 * transformation matrix.
 */
static transformation_properties on_screen_trans_props;

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
  EposStats holds information about statistics from usf_epos
  running.
*/
typedef struct
{
  int              m_nPointsCalculated;
  int              m_nPointsDropped;
  int              m_nTotalFrames;
  int              m_nLostFrames;
  int              m_nOutOfOrderErrors;
  double           m_nTotalTimeOfEposLibCalc;
  int              m_nTimesOfEposLibCalc;
  bool             m_bLastPointDropped;
  uint16_t         m_lastEventType;
  bool             m_lastPenPressed;
  region_zone_t    m_lastRegionZone;
  Vector           m_lastRawPoint;
  Vector           m_lastRawTilt;
  Vector           m_lastTransformedPoint;
  Vector           m_lastTransformedTilt;
} EposStats;

/**
  EposPacket holds information about structure of EPOS packet.
*/
typedef struct
{
  int32_t  header;
  int32_t  packet_size;
  int32_t  channel_num;
  int32_t  packet_num;
  int32_t  payload [EPOS_PACKET_SIZE_DWORDS];
  int32_t  checksum;
} EposPacket;

/**
  eposParams will hold all the information needed for EPOS
  calculation.
*/
EposParams eposParams;

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Ptr to the cfg file. The ptr is global bacause we want to
  close the file before exit in the function epos_exit.
  This function is called also from the signal_handler function
  which doesn't know the cfg file.
*/
static FILE *cfgFile = NULL;

/**
  eposStats will hold all the statistics needed.
*/
static EposStats eposStats;

/**
  m_epos_data_socket is pointer to the thread who handles data socket
  communication with the service.
*/
static DataUnSocket *m_epos_data_socket;

/**
  m_epos_control is pointer to the thread who handles control socket
  communication with the service.
*/
static ControlUnSocket *m_epos_control_socket;

/**
  Value, reported by the calculator about US absence.
*/
static const int32_t NO_US_SIGNAL = -1;

/**
  Max Tilt delta (1 degree)
*/
static const uint16_t MAX_TILT_DELTA = 1;

/**
  Parameters, used by work functions.
*/
work_params_type s_work_params;

/**
  pointer to the dynamic library of the pen.
*/
UsfEposDynamicLibProxy *epos_lib_proxy;

/**
  Dispatch control among PS state's work functions
*/
extern bool usf_ps_work(work_params_type& work_params);

/**
  Control transit timer in the PS Active state.
*/
extern void  usf_ps_act_set_transit_timer(bool b_us, ps_transit_enum& ps_transit);

/**
  Configure PS mechanism.
*/
extern void configure_ps(us_all_info& paramsStruct);

/**
  Undefined active channels status
*/
static const uint16_t UNDEF_ACT_CHANNELS = 0xffff;

/**
  This thread is intended to periodically save the persistent calibration data
  to the file system.
*/
static pthread_t s_calib_thread = 0;

/**
  This semaphore is used to control the flow of the calib_thread by the main
  thread.
*/
static sem_t s_calib_thread_sem;

/*----------------------------------------------------------------------------
  Function definitions
----------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  usf_get_time
==============================================================================*/
/**
  Get current time in msec
*/
static inline double usf_get_time()
{
  timespec tsCur;

  clock_gettime(CLOCK_REALTIME, &tsCur);
  return (((double)(tsCur.tv_nsec) +
                            ((double)tsCur.tv_sec) * 1000000000L) /
                            1000000L);
}

/*============================================================================
  FUNCTION:  is_usf_epos_running
============================================================================*/
/**
  Provide info whether usf_epos is running
*/
bool is_usf_epos_running(void)
{
  return sb_run;
}

/*============================================================================
  FUNCTION:  map_priority_epos_android
============================================================================*/
/**
  Maps EPOS library priority to Android priority
*/
inline int map_priority_epos_android(int epos_priority)
{
  switch (epos_priority)
  {
  case TRACE_ERROR:
    return ANDROID_LOG_ERROR;
  case TRACE_WARN:
    return ANDROID_LOG_WARN;
  case TRACE_INFO:
    return ANDROID_LOG_INFO;
  case TRACE_DEBUG:
    return ANDROID_LOG_DEBUG;
  default:
    return -1;
  }
}

/*============================================================================
  FUNCTION:  trace_cb
============================================================================*/
/**
  Trace callback function to be supplied to EPOS library for printing messages
*/
void trace_cb(
  int trace_level,
  const char *fmt,
  ...)
{
  if (trace_level > s_work_params.paramsStruct.epos_lib_max_trace_level)
  {
    return;
  }

  int pri = map_priority_epos_android(trace_level);
  if (-1 == pri)
  {
    LOGW("%s: Bad trace level: %d",
         __FUNCTION__,
         trace_level);
    return;
  }
  va_list ap;
  va_start(ap,
           fmt);
  LOG_PRI_VA(pri,
             EPOS_LIB_TAG,
             fmt,
             ap);
  va_end(ap);
}


/*==============================================================================
  FUNCTION:  enable_disable_ts
==============================================================================*/
/**
  Determines whether to enable/disable touchscreen.
*/
static void enable_disable_ts (bool pen_down,
                               int32_t z_coord,
                               region_zone_t rz)
{
  ts_state_t prev_ts_state_enable = ts_state_enable;

  switch (rz)
  {
  case NOT_IN_ANY_ACTIVE_ZONE:
  case OFF_SCREEN_ACTIVE_ZONE_BORDER:
    ts_state_enable = ENABLED_TS;
    break;
  case OFF_SCREEN_WORK_ZONE:
    // Disable ts only if send_to_ual is enabled,
    // the pen is pressed and in duplicate mode
    if (eposParams.m_off_screen_send_points_to_ual &&
        pen_down &&
        OFF_SCREEN_MODE_DUPLICATE == eposParams.m_off_screen_mode)
    {
      ts_state_enable = DISABLED_TS;
    }
    else
    {
      ts_state_enable = ENABLED_TS;
    }
    break;
  case ON_SCREEN_WORK_ZONE:
  // Sometimes, when the pen is held inside the active-zone, close to the border,
  // the library returns X-Y coordinate in the active-zone border.
  // Therefore, the active-zone border is treated the same as the active-zone.
  case ON_SCREEN_ACTIVE_ZONE_BORDER:
    // Disable ts only if send_to_ual is enabled
    // and Z is under the threshold
    if (eposParams.m_on_screen_send_points_to_ual &&
        eposParams.m_touch_disable_threshold > z_coord)
    {
      ts_state_enable = DISABLED_TS;
    }
    // Enable ts only if Z is above the threshold
    if (eposParams.m_touch_disable_threshold + ENABLE_TS_Z_THRESHOLDS_DIFF <= z_coord)
    {
      ts_state_enable = ENABLED_TS;
    }
    break;
  default:
    LOGE("%s: Invalid region zone",
            __FUNCTION__);
  }

  if (ts_state_enable != prev_ts_state_enable)
  {
    if (ENABLED_TS == ts_state_enable)
    {
      LOGD("%s: TS is enabled",
            __FUNCTION__);
    }
    else
    {
      LOGD("%s: TS is disabled",
           __FUNCTION__);
    }
  }

  ual_set_event_filters(ts_state_enable);
}

/*============================================================================
  FUNCTION:  send_event
============================================================================*/
/**
  Check: "ultrasound/ual_util/usf_unix_domain_socket.cpp" for information.
*/
int send_event(
  int event_num,
  int extra1,
  int extra2
)
{
  if (m_epos_control_socket != NULL)
    return m_epos_control_socket->send_event(event_num,
                                             extra1,
                                             extra2);
  return -1;
}
/*============================================================================
  FUNCTION:  control_socket_connected
============================================================================*/
/**
  Report going alive through the control socket, when a client gets connected
  to it (this function is supplied as a callback for the control socket)
*/
void control_socket_connected()
{
  send_event(POWER_STATE_CHANGED,
             PS_STATE_ACTIVE, // current state
             PS_STATE_OFF);   // prev state
}

/*============================================================================
  FUNCTION:  update_persistent_data
============================================================================*/
/**
  Updates the persistent data by calling EPOS's GetPersistentData, after
  cleaning up previous memory and allocating new memory
*/
static int update_persistent_data()
{
  if (NULL == eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet ||
      MAX_PERSIST_DATA_SIZE * sizeof(int32_t) >
        eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet_len)
  {
    eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet =
        realloc(eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet,
                MAX_PERSIST_DATA_SIZE * sizeof(int32_t));
    if (NULL == eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet)
    {
      LOGE("%s: Could not allocate memory for persistent data",
           __FUNCTION__);
      return -1;
    }
  }
  eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet_len =
        epos_lib_proxy->get_persistent_data(
          eposParams.m_epos_workspace,
          (int32_t *)eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet,
          MAX_PERSIST_DATA_SIZE);
  if (0 >= eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet_len)
  {
    LOGW("%s: Error while trying to get persistent data, ret: %d",
         __FUNCTION__,
         eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet_len);
    return -1;
  }
  return 0;
}

/*============================================================================
  FUNCTION:  calib_save_thread
============================================================================*/
/**
  Updates the persistent data by calling EPOS's GetPersistentData, after
  cleaning up previous memory and allocating new memory
*/
static void *calib_save_thread(void *arg)
{
  const int SLEEP_TIME_SEC = 5 * 60 /* 5 mins */;
  struct timespec wait_time;
  int rc;

  while (sb_run)
  {
    // Prepare to wait on semaphore for SLEEP_TIME_SEC
    clock_gettime(CLOCK_REALTIME, &wait_time);
    wait_time.tv_sec = wait_time.tv_sec + SLEEP_TIME_SEC;

    // Continues to update the calibration when timer expires.
    // Exit when semaphore is posted or on error.
    rc = sem_timedwait(&s_calib_thread_sem,
                       &wait_time);
    if (!rc)
    {
      // posted semaphore is the signal to exit
      break;
    }
    else if (ETIMEDOUT == errno)
    {
      int rc = update_persistent_data();
      if (!rc)
      {
        // Write current persistent data to persistent file
        ual_util_write_file(
           eposParams.m_calib_files[EPOS_PERSISTENT_FILE].path,
           eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet,
           eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet_len,
           sizeof(uint32_t));
        LOGD("%s: Done writing calibration file",
           __FUNCTION__);
      }
    }
    else
    {
      LOGD("%s: sem_timedwait error, rc: %d, errno: %d",
           __FUNCTION__,
           rc,
           errno);
      break;
    }
  }
  LOGD("%s: Calib thread exit",
       __FUNCTION__);
  return NULL;
}

/*============================================================================
  FUNCTION:  epos_exit
============================================================================*/
/**
  Perform clean exit of the daemon.
*/
int epos_exit (int status)
{
  bool error_state = (status != EXIT_SUCCESS);
  int rc = ual_close(error_state);

  LOGD("%s: ual_close: rc=%d;",
       __FUNCTION__,
       rc);

  if (s_calib_thread > 0)
  {
    // Post on semaphore to terminate calib_thread
    sem_post(&s_calib_thread_sem);

    pthread_join(s_calib_thread,
      NULL);
    s_calib_thread = 0;
    sem_destroy(&s_calib_thread_sem);
  }
  if (NULL != eposParams.m_epos_workspace)
  {
    if (!update_persistent_data())
    {
      // Write current persistent data to persistent file
      ual_util_write_file(
        eposParams.m_calib_files[EPOS_PERSISTENT_FILE].path,
        eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet,
        eposParams.m_calib_files[EPOS_PERSISTENT_FILE].calib_packet_len,
        sizeof(uint32_t));
    }
    epos_lib_proxy->release_dsp(eposParams.m_epos_workspace,
                                NULL);
  }

  int ret = ual_util_remove_declare_pid(PID_FILE_NAME);
  if (0 != ret)
  {
    LOGW("%s: Removing pid file failed",
         __FUNCTION__);
  }

  ual_util_close_and_sync_file(s_work_params.frameFile);
  s_work_params.frameFile = NULL;

  ual_util_close_and_sync_file(s_work_params.coordFile);
  s_work_params.coordFile = NULL;

  delete(epos_lib_proxy);
  epos_lib_proxy = NULL;

  // Must update flag, so that init would not restart the daemon.
  ret = property_set("ctl.stop",
                         "usf_epos");
  if (0 != ret)
  {
    LOGW("%s: property_set failed",
         __FUNCTION__);
  }

  _exit(status);
}

/*==============================================================================
  FUNCTION:  init_calib_file_path
==============================================================================*/
/**
  Init the calibration file path at the given index
*/
static inline void init_calib_file_path(const char* cfg_file_path,
                                        int i)
{
  if (NULL == cfg_file_path || i >= EPOS_CALIB_FILE_COUNT)
  {
    LOGE("%s: Invalid arguments, cfg_file_path: %p, index: %d",
         __FUNCTION__,
         cfg_file_path,
         i);
    return;
  }
  const char *file_path = (0 == cfg_file_path[0])?
                          DEFAULT_EPOS_CALIB_FILES[i]:
                          cfg_file_path;

  strlcpy(eposParams.m_calib_files[i].path,
          file_path,
          FILE_PATH_MAX_LEN);
}

/*==============================================================================
  FUNCTION:  rotate_off_screen_plane
==============================================================================*/
/**
 * Rotates the given off screen plane about the given axis.
 *
 * @param usf_epos_rotation_axis_origin[in] origin of rotation axis
 * @param usf_epos_rotation_axis_direction[in] direction of rotation axis
 * @param smarter_stand_angle[in] the smarter stand angle measured
 * @param off_screen_plane[in,out] the off screen plane properties.
 */
void rotate_off_screen_plane(char *usf_epos_rotation_axis_origin,
                             char *usf_epos_rotation_axis_direction,
                             const double smarter_stand_angle,
                             plane_properties &off_screen_plane)
{
  Vector axis_origin, axis_direction;
  axis_origin = usf_epos_rotation_axis_origin;
  axis_direction = usf_epos_rotation_axis_direction;

  // Rotate origin
  rotate_point_about_axis(smarter_stand_angle,
                          axis_origin,
                          axis_direction,
                          off_screen_plane.origin);

  // Rotate end x
  rotate_point_about_axis(smarter_stand_angle,
                          axis_origin,
                          axis_direction,
                          off_screen_plane.point_end_x);

  // Rotate end y
  rotate_point_about_axis(smarter_stand_angle,
                          axis_origin,
                          axis_direction,
                          off_screen_plane.point_end_y);
}

/*==============================================================================
  FUNCTION:  calc_plane_matrix
==============================================================================*/
/**
 * Calculates necessary parameters and creates the given plane's
 * matrix.
 *
 * @param plane_props[in] the rotated plane properties (origin, the end point of the x axis and
 * y axis of the rotated plane.
 * @param trans_props[out] the transformation properties struct to be filled with suitable
 * values
 * @param act_zone_area_border[in] the distances from the work area.
 */
void calc_plane_matrix(plane_properties plane_props,
                       transformation_properties &trans_props,
                       Vector act_zone_area_border)
{
  calc_transformation_properties(plane_props,
                                 act_zone_area_border,
                                 trans_props);
  create_transformation_matrix(plane_props,
                               trans_props.transformation_matrix);
}

/*==============================================================================
  FUNCTION:  init_transform_settings
==============================================================================*/
/**
  Updates transformation matrix settings.
*/
void init_transform_settings(us_all_info *paramsStruct)
{
  /* Transformation matrix creation */
  Vector on_act_zone_area_border;
  on_act_zone_area_border = paramsStruct->usf_epos_on_screen_act_zone_border;
  // On screen transformation matrix
  plane_properties on_screen_plane_props;
  on_screen_plane_props.origin = paramsStruct->usf_on_screen_transform_origin;
  on_screen_plane_props.point_end_x = paramsStruct->usf_on_screen_transform_end_X;
  on_screen_plane_props.point_end_y = paramsStruct->usf_on_screen_transform_end_Y;
  on_screen_plane_props.hover_max_range = paramsStruct->usf_epos_on_screen_hover_max_range;
  calc_plane_matrix(on_screen_plane_props,
                    on_screen_trans_props,
                    on_act_zone_area_border);

  // Off screen transformation matrix
  Vector off_act_zone_area_border;
  off_act_zone_area_border = paramsStruct->usf_epos_off_screen_act_zone_border;
  plane_properties off_screen_plane_props;
  off_screen_plane_props.origin = paramsStruct->usf_epos_off_screen_transform_origin;
  off_screen_plane_props.point_end_x = paramsStruct->usf_epos_off_screen_transform_end_X;
  off_screen_plane_props.point_end_y = paramsStruct->usf_epos_off_screen_transform_end_Y;

  eposParams.m_off_screen_plane = off_screen_plane_props;

  rotate_off_screen_plane(paramsStruct->usf_epos_rotation_axis_origin,
                          paramsStruct->usf_epos_rotation_axis_direction,
                          eposParams.m_smarter_stand_angle,
                          off_screen_plane_props);
  off_screen_plane_props.hover_max_range = paramsStruct->usf_epos_off_screen_hover_max_range;
  calc_plane_matrix(off_screen_plane_props,
                    off_screen_trans_props,
                    off_act_zone_area_border);
}

/*==============================================================================
  FUNCTION:  init_dynamic_config_params
==============================================================================*/
/**
  Inits dynamic configuration parameters in the eposParams struct.
*/
void init_dynamic_config_params(us_all_info *paramsStruct)
{
  eposParams.m_on_screen_send_points_to_ual = paramsStruct->usf_epos_on_screen_event_dest & DEST_UAL;
  eposParams.m_on_screen_send_points_to_socket = paramsStruct->usf_epos_on_screen_event_dest & DEST_SOCKET;
  eposParams.m_off_screen_send_points_to_ual = paramsStruct->usf_epos_off_screen_event_dest & DEST_UAL;
  eposParams.m_off_screen_send_points_to_socket = paramsStruct->usf_epos_off_screen_event_dest & DEST_SOCKET;
  eposParams.m_off_screen_mode = paramsStruct->usf_epos_off_screen_mode;
  eposParams.m_smarter_stand_angle = (paramsStruct->usf_epos_smarter_stand_angle * DEG_TO_RAD);
  eposParams.m_smarter_stand_enable = eposParams.m_smarter_stand_angle ? true : false;
  eposParams.m_event_type = paramsStruct->usf_event_type;
  eposParams.m_eraser_button_index = static_cast<eraser_button_index_t>(paramsStruct->eraser_button_index);
  eposParams.m_eraser_button_mode = static_cast<eraser_button_mode_t>(paramsStruct->eraser_button_mode);
  eposParams.m_on_screen_hover_icon_mode = paramsStruct->usf_epos_on_screen_hover_icon_mode;
  eposParams.m_off_screen_hover_icon_mode = paramsStruct->usf_epos_off_screen_hover_icon_mode;
  eposParams.m_touch_disable_threshold = paramsStruct->usf_epos_touch_disable_threshold;
}

/*==============================================================================
  FUNCTION:  epos_params_init
==============================================================================*/
/**
  Init eposParam struct.
*/
void epos_params_init (us_all_info *paramsStruct)
{
  uint32_t port;
  char *temp = NULL, *ip = NULL;
  int ret;

  eposParams.m_epos_workspace = NULL;
  eposParams.m_out_basic_points = NULL;
  eposParams.m_out_points = NULL;
  eposParams.m_cfg_point_downscale = paramsStruct->usf_epos_cfg_point_downscale;
  eposParams.m_nDownscalePointCounter = eposParams.m_cfg_point_downscale;
  eposParams.m_nLastEventSeqNum = 0;
  eposParams.m_bLastValidPointUalSent = false;

  eposParams.m_patternSize = paramsStruct->usf_rx_pattern_size *
                             paramsStruct->usf_rx_sample_width/BYTE_WIDTH;

  // Init all calibration file paths
  for (int i = 0; i < EPOS_CALIB_FILE_COUNT; ++i)
  {
    init_calib_file_path(paramsStruct->usf_epos_calib_files[i],
                         i);
  }

  // Update mandatory for files
  eposParams.m_calib_files[EPOS_PRODUCT_FILE].mandatory    = true;
  eposParams.m_calib_files[EPOS_UNIT_FILE].mandatory       = true;
  eposParams.m_calib_files[EPOS_PERSISTENT_FILE].mandatory = false;
  eposParams.m_calib_files[EPOS_PEN_SERIES_FILE].mandatory = true;

  init_dynamic_config_params(paramsStruct);

  eposParams.m_coord_type_on_disp = paramsStruct->usf_epos_coord_type_on_disp;
  eposParams.m_coord_type_off_disp = paramsStruct->usf_epos_coord_type_off_disp;
  eposParams.m_nNextFrameSeqNum = -1;

  ret = sscanf(paramsStruct->usf_fuzz_params,
               "%f ,%f ,%f",
               &eposParams.m_fuzz[X_IND],
               &eposParams.m_fuzz[Y_IND],
               &eposParams.m_fuzz[Z_IND]);
  if (3 != ret)
  {
    LOGW("%s: usf_fuzz_params should have three dims, ret from scanf=%d",
         __FUNCTION__,
         ret);
    eposParams.m_fuzz[X_IND] = 0;
    eposParams.m_fuzz[Y_IND] = 0;
    eposParams.m_fuzz[Z_IND] = 0;
  }

  init_transform_settings(paramsStruct);

  eposParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;
  eposParams.m_group = paramsStruct->usf_tx_port_count;

  eposStats.m_nPointsCalculated = 0;
  eposStats.m_nPointsDropped = 0;
  eposStats.m_nTotalFrames = 0;
  eposStats.m_nLostFrames = 0;
  eposStats.m_nOutOfOrderErrors = 0;
  s_work_params.numPoints = 0;
  s_work_params.pointLogFrameCounter = 0;
  s_work_params.recorded_coord_counter = 0;
  s_work_params.act_mics = UNDEF_ACT_CHANNELS;
  s_work_params.bad_writing_scenarios = UNDEF_ACT_CHANNELS;
  s_work_params.act_mics_bad_scenarios_test_time = 0;
  s_work_params.is_spur = false;
  s_work_params.spurs_test_time = 0;

  // Init EPOS lib
  epos_lib_proxy = new UsfEposDynamicLibProxy();
  if (!epos_lib_proxy->open_lib(paramsStruct->usf_epos_lib_path))
  {
    LOGE("%s: Init EPOS dynamic lib failed.",
         __FUNCTION__);
    epos_exit(EXIT_FAILURE);
  }

  // TODO: Insert path to config file
  m_epos_data_socket =
    new DataUnSocket("/data/usf/epos/data_socket");
  if (m_epos_data_socket->start() != 0)
  {
    LOGE("%s: Starting data socket failed.",
         __FUNCTION__);
    epos_exit(EXIT_FAILURE);
  }

  eposParams.m_dconfig.config_changed = false;
  eposParams.m_dconfig.angle_changed = false;
  // TODO: Insert path to config file
  m_epos_control_socket = new ControlUnSocket("/data/usf/epos/control_socket",
                                              &eposParams.m_dconfig,
                                              control_socket_connected);

  if (m_epos_control_socket->start() < 0)
  {
    LOGE("%s: Starting control socket failed.",
         __FUNCTION__);
    epos_exit(EXIT_FAILURE);
  }
}

/*==============================================================================
  FUNCTION:  load_coeffs
==============================================================================*/
/**
  Loads the calibration file at index file_num into the EPOS library, if a
  mandatory
*/
static void load_coeffs(int file_num) {
  int rc = 0;
  char msg[MAX_FILE_NAME_LEN] = "";
  if (NULL == eposParams.m_calib_files[file_num].calib_packet ||
      0    == eposParams.m_calib_files[file_num].calib_packet_len)
  {
    rc = -2;
    snprintf(msg,
             MAX_FILE_NAME_LEN,
             "%s: %s - File not found or file empty",
             __FUNCTION__,
             eposParams.m_calib_files[file_num].path);
  }
  else
  {
    if ((rc = epos_lib_proxy->load_coeffs(
      eposParams.m_epos_workspace,
      eposParams.m_calib_files[file_num].calib_packet,
      eposParams.m_calib_files[file_num].calib_packet_len / sizeof(int32_t))))
    {
      snprintf(msg,
             MAX_FILE_NAME_LEN,
             "%s: Load coeffs failed for calib file: %s ret: %d",
             __FUNCTION__,
             eposParams.m_calib_files[file_num].path,
             rc);
    }
  }
  if (rc)
  {
    if (eposParams.m_calib_files[file_num].mandatory) {
      LOGE("%s. Mandatory file - exiting", msg);
      epos_exit(EXIT_FAILURE);
    }
    LOGW("%s - continuing", msg);
  }
}

/*==============================================================================
  FUNCTION:  load_calib_file
==============================================================================*/
/**
  Loads calibration file at index i, by calling ual_util_malloc_read the file
  gets loaded into memory and a pointer to that piece of memory is returned,
  calib_packet_len gets also updated by the function.
*/
static void load_calib_file(int file_num)
{
  eposParams.m_calib_files[file_num].calib_packet =
      ual_util_malloc_read(eposParams.m_calib_files[file_num].path,
                           eposParams.m_calib_files[file_num].calib_packet_len);
}

/*==============================================================================
  FUNCTION:  epos_init_smarter_stand
==============================================================================*/
/**
  Init library's smarter stand parameters.
*/
void epos_init_smarter_stand()
{
  // library set_rotation_axis() function uses units of 0.1mm (unlike other
  // library functions which use units of 0.01mm)
  const int mm_scaling_factor = 10;

  // Smarter stand parameters
  Vector axis_origin, axis_direction, on_screen_origin, off_screen_origin;
  axis_origin = s_work_params.paramsStruct.usf_epos_rotation_axis_origin;
  axis_direction = s_work_params.paramsStruct.usf_epos_rotation_axis_direction;
  on_screen_origin = s_work_params.paramsStruct.usf_on_screen_transform_origin;
  off_screen_origin = eposParams.m_off_screen_plane.origin;

  axis_origin.vector_mult_by_scalar(mm_scaling_factor);
  // Rotation direction is the opposite for the library
  // Translate number to Q15 format
  axis_direction.vector_mult_by_scalar(-(1 << Q15_SHIFT));;

  int32_t origin[3] = {
    (int32_t)axis_origin.get_element(X),
    (int32_t)axis_origin.get_element(Y),
    (int32_t)axis_origin.get_element(Z)
    };

  int32_t direction[3] =
  {
    (int32_t)axis_direction.get_element(X),
    (int32_t)axis_direction.get_element(Y),
    (int32_t)axis_direction.get_element(Z)
  };

  // Off screen and on screen planes are assumed to be parallel
  int32_t screen_distance = fabs(on_screen_origin.get_element(Z) -
                             off_screen_origin.get_element(Z)) * mm_scaling_factor;

  epos_lib_proxy->set_rotation_axis(eposParams.m_epos_workspace,
                                    origin,
                                    direction,
                                    screen_distance);

  LOGD("Library smart stand params: %d",
       screen_distance);
  axis_origin.print_vector();
  axis_direction.print_vector();

}

/*==============================================================================
  FUNCTION:  epos_init
==============================================================================*/
/**
  Init EPOS resources.
*/
void epos_init
(
  // A flag which indicates whether it's the first time this function is called
  // if yes, all the allocations will be made
  bool first_time
)
{
  char szVersion [256] = {0};
  unsigned char nVersionNum[4] = {0};
  int rc;

  // Allocate memory for EPOS algorithm.
  int32_t epos_points_per_pen, epos_max_pens, epos_workspace_size;

  if (first_time)
  {
    epos_lib_proxy->get_allocation_sizes(&epos_points_per_pen,
                                         &epos_max_pens,
                                         &epos_workspace_size);

    eposParams.m_epos_workspace = (void *)malloc(epos_workspace_size);
    eposParams.m_out_basic_points = (EPoint *)malloc(epos_max_pens *
                                                     epos_points_per_pen *
                                                     sizeof(EPoint));
    eposParams.m_out_points = (usf_extended_epoint_t *)malloc(
      epos_max_pens *
      epos_points_per_pen *
      sizeof(usf_extended_epoint_t));

    if (NULL == eposParams.m_epos_workspace ||
        NULL == eposParams.m_out_basic_points ||
        NULL ==  eposParams.m_out_points)
    {
      LOGE("%s: Out of memory",
           __FUNCTION__);
      epos_exit(EXIT_FAILURE);
    }

    epos_lib_proxy->get_dsp_version(szVersion,
                                    nVersionNum);

    if (strcmp(szVersion, "stub_version") == 0)
    {
      LOGW("%s: Stub init.",
           __FUNCTION__);
      return;
    }

    for (int i = 0; i < EPOS_CALIB_FILE_COUNT; ++i)
    {
      load_calib_file(i);
    }

    LOGD("%s: Done loading parameters for epos library",
         __FUNCTION__);

    eposParams.m_pPattern = NULL;

    if (0 > sem_init(&s_calib_thread_sem,
                     0,
                     0))
    {
      LOGE("%s:  Could not init semaphore for calib thread; errno=%d",
           __FUNCTION__,
           errno);
      epos_exit(EXIT_FAILURE);
    }
    else
    {
      if (pthread_create(&s_calib_thread,
                         NULL,
                         calib_save_thread,
                         NULL))
      {
        LOGE("%s:  calib_thread create failure; errno=%d",
             __FUNCTION__,
             errno);
        sem_destroy(&s_calib_thread_sem);
        epos_exit(EXIT_FAILURE);
      }
    }
  } // if(first_time)

  LOGD("%s: Reseting epos library",
       __FUNCTION__);

  if ((rc = epos_lib_proxy->reset_dsp(eposParams.m_out_basic_points,
                                      eposParams.m_epos_workspace)))
  {
    LOGE("%s: Could not init EPOS library, ret: %d",
         __FUNCTION__,
         rc);
    epos_exit(EXIT_FAILURE);
  }

  epos_lib_proxy->set_dsp_trace_callback(trace_cb);

  // Load the calib files into the EPOS library
  for (int i = 0; i < EPOS_CALIB_FILE_COUNT; ++i)
  {
    load_coeffs(i);
  }

  s_work_params.act_mics = UNDEF_ACT_CHANNELS;
  s_work_params.act_mics_bad_scenarios_test_time = 0;

  epos_init_smarter_stand();
}

/*============================================================================
  FUNCTION:  remove_last_event
============================================================================*/
/**
  Clears the last event
*/
void remove_last_event()
{
  if (s_work_params.numPoints <= 0)
    return;
  usf_event_type *pEvent;
  pEvent = &eposParams.m_events[--s_work_params.numPoints];
  --eposParams.m_nLastEventSeqNum;
  // The event is going to be removed
  pEvent->event_type_ind = USF_MAX_EVENT_IND;
}

/*============================================================================
  FUNCTION:  get_tool_rubber_mask
============================================================================*/
/**
  Returns a mask with the btn_tool_rubber corresponding bit set or clear
  according to correct internal and button states.
*/
static uint16_t get_tool_rubber_mask(
  bool primary_pressed,
  bool secondary_pressed
)
{
    // internal states for toggle mode of the eraser button.
    // All states relate to TOGGLE_MODE.
    // State TOGGLE_DONT_SEND - don't send rubber tooltype;
    //       starting state, or side button has been released in state TOGGLE_PRESSED_ON_SEND.
    // State TOGGLE_PRESSED_ON_DONT_SEND - don't send rubber tooltype;
    //       side button has been pressed (not yet released) in state TOGGLE_DONT_SEND.
    // State TOGGLE_SEND - send rubber tooltype;
    //       side button has been released in state TOGGLE_PRESSED_ON_DONT_SEND.
    // State TOGGLE_PRESSED_ON_SEND - send rubber tooltype;
    //       side button has been pressed (not yet released) in state TOGGLE_SEND.
    typedef enum
    {
      TOGGLE_DONT_SEND,
      TOGGLE_PRESSED_ON_DONT_SEND,
      TOGGLE_SEND,
      TOGGLE_PRESSED_ON_SEND
    } eraser_state_t;

    static eraser_state_t eraser_state = TOGGLE_DONT_SEND;
    uint16_t tool_rubber_mask = 0;
    bool is_sb_pressed = false;
    bool send_rubber_tooltype = false;

    switch(eposParams.m_eraser_button_index)
    {
    case ERASER_BUTTON_INDEX_BUTTON1:
      is_sb_pressed = primary_pressed;
      break;
    case ERASER_BUTTON_INDEX_BUTTON2:
      is_sb_pressed = secondary_pressed;
      break;
    default:
      is_sb_pressed = false;
    }

    switch (eposParams.m_eraser_button_mode)
    {
    case ERASER_BUTTON_MODE_DISABLED:
        send_rubber_tooltype = false;
        break;
    case ERASER_BUTTON_MODE_TOGGLE_ERASE:
        switch (eraser_state)
        {
        case TOGGLE_DONT_SEND:
            send_rubber_tooltype = false;
            if (is_sb_pressed)
            {
                eraser_state = TOGGLE_PRESSED_ON_DONT_SEND;
            }
            break;
        case TOGGLE_PRESSED_ON_DONT_SEND:
            send_rubber_tooltype = false;
            if (!is_sb_pressed)
            {
                eraser_state = TOGGLE_SEND;
            }
            break;
        case TOGGLE_SEND:
            send_rubber_tooltype = true;
            if (is_sb_pressed)
            {
                eraser_state = TOGGLE_PRESSED_ON_SEND;
            }
            break;
        case TOGGLE_PRESSED_ON_SEND:
            send_rubber_tooltype = true;
            if (!is_sb_pressed)
            {
                eraser_state = TOGGLE_DONT_SEND;
            }
            break;
        }
        break;
    case ERASER_BUTTON_MODE_HOLD_TO_ERASE:
        send_rubber_tooltype = is_sb_pressed;
        break;
    default:
        LOGE("%s: Illegal eraser_button_mode [%d]",
             __FUNCTION__,
             (int) eposParams.m_eraser_button_mode);
    }

    tool_rubber_mask = (((int) send_rubber_tooltype) << BTN_TOOL_RUBBER_SHIFT);
    return tool_rubber_mask;
}

/*============================================================================
  FUNCTION:  get_buttons_state_bitmap
============================================================================*/
/**
  Mapping digital pen button states into buttons bitmap
*/
static uint16_t get_buttons_state_bitmap(us_all_info *paramsStruct,
                                         int32_t type)
{
  uint16_t buttons_state_bitmap = 0;
  int32_t primary_switch_state = 0;
  int32_t secondary_switch_state = 0;

  if (paramsStruct->req_buttons_bitmap & BTN_STYLUS_MASK)
  {
    int32_t ret_code = 0;
    int32_t rc = epos_lib_proxy->query_epoint(eposParams.m_epos_workspace,
                               POINT_PrimarySwitch,
                               0,
                               &primary_switch_state,
                               sizeof(primary_switch_state)/sizeof(int32_t),
                               &ret_code);

    if ((rc < 0) || (ret_code <= 0))
    {
      primary_switch_state = !!(type & COORD_SW2);
    }

    buttons_state_bitmap |= (primary_switch_state << BTN_STYLUS_SHIFT);
  }

  if (paramsStruct->req_buttons_bitmap & BTN_STYLUS2_MASK)
  {
    int32_t ret_code = 0;
    int32_t rc = epos_lib_proxy->query_epoint(eposParams.m_epos_workspace,
                               POINT_SecondarySwitch,
                               0,
                               &secondary_switch_state,
                               sizeof(secondary_switch_state)/sizeof(int32_t),
                               &ret_code);

    if ((rc < 0) || (ret_code <= 0))
    {
      secondary_switch_state = !!(type & COORD_SW1);
    }

    buttons_state_bitmap |= (secondary_switch_state << BTN_STYLUS2_SHIFT);
  }

  if (paramsStruct->req_buttons_bitmap & BTN_TOOL_PEN_MASK)
  {
    buttons_state_bitmap |=
      (1 << BTN_TOOL_PEN_SHIFT); // Always send PEN tool type if
                                 // requested (for now)
  }

  if (paramsStruct->req_buttons_bitmap & BTN_TOOL_RUBBER_MASK)
  {
    buttons_state_bitmap |=
      get_tool_rubber_mask(primary_switch_state, secondary_switch_state);
  }

  return buttons_state_bitmap;
}

/*==============================================================================
  FUNCTION:  get_current_hover_icon_state
==============================================================================*/
/**
 * Determines whether the hovering icon should be currently shown, given the
 * current point's information and the hovering icon settings
 *
 * @param point_rz the region zone of the point
 * @param pen_down the pen down state for the current point
 * @param z        the pen z coordinate
*/
static bool get_current_hover_icon_state(
  region_zone_t point_rz,
  bool pen_down,
  int z
)
{
  // hovering icon is shown when:
  // - the hovering icon is enabled for the point's region zone
  // - stylus not touching the screen (not pen_down)
  // - stylus is in Z range that disables touch

  bool on_screen_condition =
    ((HOVER_ICON_PALM_AREA ==
      eposParams.m_on_screen_hover_icon_mode) &&
     (ON_SCREEN_WORK_ZONE == point_rz));

  bool off_screen_condition =
    ((HOVER_ICON_PALM_AREA ==
      eposParams.m_off_screen_hover_icon_mode) &&
     (OFF_SCREEN_WORK_ZONE == point_rz) &&
     (OFF_SCREEN_MODE_DUPLICATE == eposParams.m_off_screen_mode));

  bool pen_proximity_condition = (!pen_down &&
                                  z < eposParams.m_touch_disable_threshold);

  if ((on_screen_condition || off_screen_condition) && pen_proximity_condition)
  {
    return true;
  }

  return false;
}

/*==============================================================================
  FUNCTION:  get_pen_pressure
==============================================================================*/
/**
 * Returns the pen pressure. Supports also pre ref3 pens
 * (where pressure is not supported)
 *
 * @param pressure the pen pressure which was reported from the pen lib
 * @param pen_down the pen down state for the current point
*/
static int get_pen_pressure(
  int pressure,
  bool pen_down
)
{
  if (pen_down && !pressure)
  {
    return MAX_PRESSURE;
  }
  if (!pen_down)
  {
    return MIN_PRESSURE;
  }
  return pressure;
}

/*============================================================================
  FUNCTION:  add_event_point
============================================================================*/
/**
  Creates an event and adds it to the eposParams.m_events[].
*/
static bool add_event_point(
  int x,
  int y,
  int z,
  int lib_pressure,
  int tiltX,
  int tiltY,
  bool force_buttons_up,
  region_zone_t rz,
  int32_t type,
  uint16_t event_type_ind
)
{
  // Fill in the usf_event_type struct
  if (US_MAX_EVENTS <= s_work_params.numPoints)
  {
    LOGE("%s: No more events to send max counter=%d",
         __FUNCTION__,
         US_MAX_EVENTS);
    return false;
  }
  usf_event_type *pEvent;
  pEvent = &eposParams.m_events[s_work_params.numPoints++];
  pEvent->seq_num = eposParams.m_nLastEventSeqNum++;
  pEvent->timestamp = (uint32_t)(clock ());
  pEvent->event_type_ind = event_type_ind;
  bool pen_down = type & EPOS_TYPE_PEN_DOWN_BIT;
  int pressure_to_report = get_pen_pressure(lib_pressure,
                                            pen_down);

  switch (event_type_ind)
  {
    case USF_TSC_EXT_EVENT_IND:
    case USF_TSC_EVENT_IND:
    case USF_TSC_PTR_EVENT_IND:
      pEvent->event_data.point_event.pressure = pressure_to_report;
      if (force_buttons_up)
      {
        pEvent->event_data.point_event.buttons_state_bitmap = 0;
      }
      else
      {
        pEvent->event_data.point_event.buttons_state_bitmap =
        get_buttons_state_bitmap(&(s_work_params.paramsStruct),
                                     type);

        if (get_current_hover_icon_state(rz, pen_down, z))
        {
          pEvent->event_data.point_event.buttons_state_bitmap |=
                  BTN_USF_HOVERING_ICON_MASK;
        }
      }

      pEvent->event_data.point_event.coordinates[X_IND] = x;
      pEvent->event_data.point_event.coordinates[Y_IND] = y;
      pEvent->event_data.point_event.coordinates[Z_IND] = z;
      pEvent->event_data.point_event.inclinations[X_IND] = tiltX;
      pEvent->event_data.point_event.inclinations[Y_IND] = tiltY;
    break;
    case USF_MOUSE_EVENT_IND:
      pEvent->event_data.mouse_event.rels[X_IND] = x;
      pEvent->event_data.mouse_event.rels[Y_IND] = y;
      pEvent->event_data.mouse_event.rels[Z_IND] = z;
      pEvent->event_data.mouse_event.buttons_states =
                                                  (pen_down)?
                                                  USF_BUTTON_LEFT_MASK:
                                                  0;
    break;
    default: // The case is not supported
      // Remove the newly added event
      remove_last_event();
      return false;
  }
  return true;
}

/*==============================================================================
  FUNCTION:  is_diff_point
==============================================================================*/
/**
 Returns true if the provided point differs from the previous one
*/
static inline bool is_diff_point(const usf_extended_epoint_t &point)
{
  static usf_extended_epoint_t s_prev_point;
  bool rc = false;
  Vector tilt(point.TiltX, point.TiltY, point.TiltZ),
    prev_tilt(s_prev_point.TiltX, s_prev_point.TiltY, s_prev_point.TiltZ);
  tilt.vector_mult_by_scalar(1.0 / TILT_Q_FACTOR);
  prev_tilt.vector_mult_by_scalar(1.0 / TILT_Q_FACTOR);
  tilt = tilt.get_tilt_angle_vector();
  prev_tilt = prev_tilt.get_tilt_angle_vector();

  if ((abs(point.X - s_prev_point.X) > eposParams.m_fuzz[X_IND]) ||
      (abs(point.Y - s_prev_point.Y) > eposParams.m_fuzz[Y_IND]) ||
      (abs(point.Z - s_prev_point.Z) > eposParams.m_fuzz[Z_IND]) ||
      (abs(tilt.get_element(X) - prev_tilt.get_element(X)) > MAX_TILT_DELTA) ||
      (abs(tilt.get_element(Y) - prev_tilt.get_element(Y)) > MAX_TILT_DELTA) ||
      (point.Type != s_prev_point.Type))
  {
    rc = true;
    s_prev_point = point;
  }

  return rc;
}

/*==============================================================================
  FUNCTION:  get_spurs
==============================================================================*/
/**
 Check if there are spurs in the microphones
*/
static void get_spurs(void *InWorkspace, double& time)
{
  static const double SPURS_TIME = 200; // msec

  if (!s_work_params.spurs_test_time)
  { // first time entry
    s_work_params.spurs_test_time = time + SPURS_TIME;
  }
  else if (time >= s_work_params.spurs_test_time)
  { // It's time to check spurs
      struct usf_epos_command cmd_spurs;

      cmd_spurs.cmd_type = CMD_TYPE_GET;
      cmd_spurs.cmd_num = COMMAND_Get_Spur_Frequency;
      int ret = epos_lib_proxy->command(InWorkspace,
                                        (int32_t*)&cmd_spurs);
      if (!ret)
      {
        bool isSpur = (bool) cmd_spurs.args[0];
        if (isSpur != s_work_params.is_spur)
        {
          s_work_params.is_spur = isSpur;
          ret = send_event(SPUR_STATE,
                     isSpur,
                     0);

          LOGD("%s: ret=%d; is_spur=%d; spur_freq=%d",
               __FUNCTION__,
               ret,
               (int) s_work_params.is_spur,
               cmd_spurs.args[0]);
        } // active channels state was changed
      } // valid response
      s_work_params.spurs_test_time = time + SPURS_TIME;
  }
}

/*==============================================================================
  FUNCTION:  get_act_channels
==============================================================================*/
/**
 Calculates the number of blocked channels and sends this number to the socket
*/
static void get_act_channels(void *InWorkspace, double& time)
{
  static const double ACT_CHANNEL_TIME = 200; // msec

  if (!s_work_params.act_mics_bad_scenarios_test_time)
  { // first time entry
    s_work_params.act_mics_bad_scenarios_test_time = time + ACT_CHANNEL_TIME;
  }
  else
  {
    if (time >= s_work_params.act_mics_bad_scenarios_test_time)
    { // It's time to check active channels
      struct usf_epos_command cmd_act_channels;

      cmd_act_channels.cmd_type = CMD_TYPE_GET;
      cmd_act_channels.cmd_num = CMD_GET_ACT_CHANNELS;
      int ret = epos_lib_proxy->command(InWorkspace,
                                        (int32_t*)&cmd_act_channels);
      if (!ret)
      {
        uint16_t mics = cmd_act_channels.args[0] & 0x00FF;
        uint16_t scenarios = cmd_act_channels.args[0] >> 16;
        if (mics != s_work_params.act_mics)
        {
          int num_of_blocked_mic = 0;
          s_work_params.act_mics = mics;

          for (int i = 0; i < s_work_params.paramsStruct.usf_tx_port_count; i++)
          {
            uint16_t act_channel_mask = 1 << i;
            bool mic_i_open = s_work_params.act_mics & act_channel_mask;
            if (!mic_i_open)
            {
              num_of_blocked_mic++;
            }
          }

          ret = send_event(MIC_BLOCKED,
                     num_of_blocked_mic,
                     0);

          LOGD("%s: ret=%d; act_channels=%hu; num_of_blocked_mic=%d",
               __FUNCTION__,
               ret,
               s_work_params.act_mics,
               num_of_blocked_mic);
        } // active channels blocked mics state was changed

        if (scenarios != s_work_params.bad_writing_scenarios)
        {
          s_work_params.bad_writing_scenarios = scenarios;

          ret = send_event(BAD_SCENARIO,
                     scenarios,
                     0);

          LOGD("%s: ret=%d; bad_scenarios=%hu;",
               __FUNCTION__,
               ret,
               s_work_params.bad_writing_scenarios);
        } // active channels bad scenarios state was changed
      } // valid response
      s_work_params.act_mics_bad_scenarios_test_time = time + ACT_CHANNEL_TIME;
    }
  }
}

/*==============================================================================
  FUNCTION:  get_battery_state
==============================================================================*/
/**
 * Gets the pen battery state from the pen lib and sends it to the socket
 *
 * @param InWorkspace the pen lib workspace
*/
static void get_battery_state(void *InWorkspace)
{
  static int32_t s_last_battery_state = BATTERY_OK;
  // valid battery levels are -1 and above so to get the first event,
  // this param is set to an invalid value
  static int32_t s_last_battery_level = -2;
  int32_t battery_state = BATTERY_OK;
  int32_t battery_level = 0;

  int32_t ret_code = 0;
  int32_t rc = epos_lib_proxy->query_epoint(InWorkspace,
                               POINT_BatteryLevel,
                               0,
                               &battery_level,
                               sizeof(battery_level)/sizeof(int32_t),
                               &ret_code);

  if (0 <= battery_level)
  {
    // Translate number to Q15 format
    battery_level = (battery_level * 100) / (1 << Q15_SHIFT);
  }

  if (battery_level <=
      s_work_params.paramsStruct.usf_epos_battery_low_level_threshold)
  {
    battery_state = BATTERY_LOW;
  }

  if ((s_last_battery_state != battery_state) ||
      (s_last_battery_level != battery_level))
  {
    int ret = send_event(BATTERY_STATE,
                         battery_state,
                         battery_level);

    LOGD("%s: ret=%d; battery_state=%d, battery_level=%d",
         __FUNCTION__,
         ret,
         battery_state,
         battery_level);

    s_last_battery_state = battery_state;
    s_last_battery_level = battery_level;
  }
}

/*==============================================================================
  FUNCTION:  is_point_valid
==============================================================================*/
/**
  Determines whether the given point is valid.
*/
static inline bool is_point_valid(usf_extended_epoint_t *pPoint)
{
  return (pPoint->Type & EPOS_TYPE_VALID_BIT);
}

/*==============================================================================
  FUNCTION:  stats_store_raw_point
==============================================================================*/
/**
  Stores raw point information in statistics structure.
*/
void stats_store_raw_point(usf_extended_epoint_t const &point,
                           bool is_pressed)
{
  Vector raw_point(point.X, point.Y, point.Z);
  eposStats.m_lastRawPoint = raw_point;
  Vector raw_tilt(point.TiltX, point.TiltY, point.TiltZ);
  raw_tilt.vector_mult_by_scalar(1.0 / TILT_Q_FACTOR);

  eposStats.m_lastRawTilt = raw_tilt;
  eposStats.m_lastPenPressed = is_pressed;
  // Assuming this function is called prior to stats_store_trasformed_point
  // Initializing parameter to dropped. When reaching
  // stats_store_trasformed_point function, point was not
  // dropped, and changing value to false
  eposStats.m_bLastPointDropped = true;
}

/*==============================================================================
  FUNCTION:  stats_store_trasformed_point
==============================================================================*/
/**
  Stores transformed point information in statistics structure.
*/
void stats_store_trasformed_point(usf_extended_epoint_t const &point,
                                  uint16_t index_type,
                                  region_zone_t rz)
{
  Vector transformed_point(point.X, point.Y, point.Z);
  eposStats.m_lastTransformedPoint = transformed_point;
  Vector transformed_tilt(point.TiltX, point.TiltY, 0);
  eposStats.m_lastTransformedTilt = transformed_tilt;
  eposStats.m_lastRegionZone = rz;
  eposStats.m_lastEventType = index_type;
  // When reached here, point was not dropped
  eposStats.m_bLastPointDropped = false;
}

/*==============================================================================
  FUNCTION:  record_coord_file
==============================================================================*/
/**
  Records coordinates to a file
*/
static void record_coord_file(int num_points)
{
  if (NULL != s_work_params.coordFile)
  {
    // Write valid points to file
    for (int coord_idx = 0; (s_work_params.recorded_coord_counter <
                             s_work_params.paramsStruct.usf_epos_coord_count) &&
          (coord_idx < num_points); coord_idx++)
    {
      if (!is_point_valid(&eposParams.m_out_points[coord_idx]))
      {
        continue;
      }

      size_t st = fwrite(&eposParams.m_out_points[coord_idx],
                         sizeof(eposParams.m_out_points[coord_idx]),
                         1,
                         s_work_params.coordFile);

      s_work_params.recorded_coord_counter++;
    }

    if (s_work_params.recorded_coord_counter ==
        s_work_params.paramsStruct.usf_epos_coord_count)
    {
      LOGD("%s: Done recording EPOS coordinates",
         __FUNCTION__);
      ual_util_close_and_sync_file(s_work_params.coordFile);
      s_work_params.coordFile = NULL;
      // Mark recording as done
      int ret = property_set("debug.usf_epos.coord_rec_done",
                             "1");
      if (0 != ret)
      {
        LOGW("%s: setting recording done property failed",
             __FUNCTION__);
      }
    }
  }
}

/*==============================================================================
  FUNCTION:  get_zone
==============================================================================*/
/**
 * Returns the plane's zone.
 * When point is off screen and off screen is disabled point is considered
 * out of active zone.
 *
 * @param rz    The current point's region zone
 * @param point The current point
 *
 * @return zone_t The region's zone.
 */
static zone_t get_zone(region_zone_t rz,
                       const usf_extended_epoint_t &point)
{
  if (!eposParams.m_off_screen_send_points_to_ual &&
      !eposParams.m_off_screen_send_points_to_socket &&
      (OFF_SCREEN_ACTIVE_ZONE_BORDER == rz ||
       OFF_SCREEN_WORK_ZONE == rz))
  { // Off screen is disabled
    return NOT_ACTIVE_ZONE;
  }
  if (ON_SCREEN_ACTIVE_ZONE_BORDER == rz ||
      OFF_SCREEN_ACTIVE_ZONE_BORDER == rz)
  {
    return ACTIVE_ZONE_BORDER;
  }
  if (!(point.Type & EPOS_TYPE_PEN_DOWN_BIT))
  { // Sometimes, when the pen is held close to the surface of
    // the device (without pressing), the library returns Z=0.
    // When hovering is disabled this workaround is needed.
    if (ON_SCREEN_WORK_ZONE == rz &&
        0 == on_screen_trans_props.hover_max_range)
    {
      if (on_screen_trans_props.act_zone_area_border.get_element(Z) > 0)
      {
        return ACTIVE_ZONE_BORDER;
      }
      return NOT_ACTIVE_ZONE;
    }

    if (OFF_SCREEN_WORK_ZONE == rz &&
        0 == off_screen_trans_props.hover_max_range)
    {
      if (off_screen_trans_props.act_zone_area_border.get_element(Z) > 0)
      {
        return ACTIVE_ZONE_BORDER;
      }
      return NOT_ACTIVE_ZONE;
    }
  }

  if (ON_SCREEN_WORK_ZONE == rz ||
      OFF_SCREEN_WORK_ZONE == rz)
  {
    return WORKING_ZONE;
  }
  return NOT_ACTIVE_ZONE;
}

/*==============================================================================
  FUNCTION:  get_event_type_index
==============================================================================*/
static uint16_t get_event_type_index(region_zone_t rz)
{
  if (OFF_SCREEN_MODE_EXTEND == eposParams.m_off_screen_mode &&
      OFF_SCREEN_WORK_ZONE == rz)
  {
    return USF_TSC_EXT_EVENT_IND;
  }
  else
  {
    // The TSC event type depends on upper layer application
    return (USF_TSC_EVENT & eposParams.m_event_type)?
            USF_TSC_EVENT_IND: USF_TSC_PTR_EVENT_IND;
  }
}

/*==============================================================================
  FUNCTION:  send_socket_event
==============================================================================*/
static void send_socket_event(usf_extended_epoint_t const &point,
                              side_channel_region_t region)
{
  int pressure = get_pen_pressure(point.P,
                                  point.Type & EPOS_TYPE_PEN_DOWN_BIT);

  int ret = m_epos_data_socket->send_epos_point(point.X,
                                                point.Y,
                                                point.Z,
                                                point.TiltX,
                                                point.TiltY,
                                                point.TiltZ,
                                                pressure,
                                                point.Type,
                                                region);
  if (0 > ret)
  { // If we got here there is some problem in sending points to socket.
    // The m_socket_sending_prob starts from SOCKET_PROBLEM_MSG_INTERVAL
    // and only when it gets to 0 a warning msg is shown to the user and
    // m_socket_sending_prob set to SOCKET_PROBLEM_MSG_INTERVAL again.
    eposParams.m_socket_sending_prob--;

    if (0 == eposParams.m_socket_sending_prob)
    {
      LOGW("%s: Sending point to socket failed.", __FUNCTION__);
      eposParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;
    }
  }
  else
  {
    eposParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;
  }
}

/*==============================================================================
  FUNCTION:  is_button_pressed
==============================================================================*/
/**
 * Returns whether the given type describes a point that has
 * a button pressed.
 *
 * @param type The type of the point to be checked.
 *
 * @return bool true - the point has a button pressed
 *              false - otherwise.
 */
static bool is_button_pressed(int type)
{
  return (type & (~EPOS_TYPE_VALID_BIT));
}

/*==============================================================================
  FUNCTION:  handle_outside_work_zone_event
==============================================================================*/
/**
 * According to touch protocol every series of points needs to send an up event.
 * This function handles the cases where the point is outside the working zone
 * but we still need to send the up event.
 *
 * @return bool true - success, false - failure
 */
static bool handle_outside_work_zone_event()
{
  // Last point was sent to input module
  if (eposParams.m_bLastValidPointUalSent)
  {
    LOGD("Sending up event");
    eposParams.m_bLastValidPointUalSent = false;
    return add_event_point(eposParams.m_nLastPoint.X,
                           eposParams.m_nLastPoint.Y,
                           eposParams.m_nLastPoint.Z,
                           MIN_PRESSURE,
                           eposParams.m_nLastPoint.TiltX,
                           eposParams.m_nLastPoint.TiltY,
                           // Force buttons up
                           true,
                           NOT_IN_ANY_ACTIVE_ZONE,
                           0,
                           eposParams.m_nLastEventIndex);
  }

  return true;
}

/*==============================================================================
  FUNCTION:  handle_work_zone_event
==============================================================================*/
/**
 * Sends point according to the socket and input module configuration
 *
 * @param rz The region zone of the point to send.
 * @param transformed_point the point to send.
 * @param raw_point the raw point to send.
 * @param event_type_index the event type of the point.
 */
static void handle_work_zone_event(region_zone_t rz,
                                   usf_extended_epoint_t const &transformed_point,
                                   usf_extended_epoint_t const &raw_point,
                                   uint16_t event_type_index)
{
  bool send_to_input_module = (eposParams.m_on_screen_send_points_to_ual &&
                              ON_SCREEN_WORK_ZONE == rz) ||
                              (eposParams.m_off_screen_send_points_to_ual &&
                              OFF_SCREEN_WORK_ZONE == rz);
  eposParams.m_bLastValidPointUalSent = send_to_input_module;

  if (send_to_input_module)
  {
    bool rc = add_event_point(transformed_point.X,
                              transformed_point.Y,
                              transformed_point.Z,
                              transformed_point.P,
                              transformed_point.TiltX,
                              transformed_point.TiltY,
                              false,
                              rz,
                              transformed_point.Type,
                              event_type_index);
    if (!rc)
    {
      ++eposStats.m_nPointsDropped;
    }
  }

  if (ON_SCREEN_WORK_ZONE == rz &&
      eposParams.m_on_screen_send_points_to_socket)
  {
    send_socket_event(eposParams.m_socket_on_screen_mapped ? transformed_point:
                                                             raw_point,
                      SIDE_CHANNEL_REGION_ON_SCREEN);
    return;
  }

  if (OFF_SCREEN_WORK_ZONE == rz &&
      eposParams.m_off_screen_send_points_to_socket)
  {
    send_socket_event(eposParams.m_socket_off_screen_mapped ? transformed_point:
                                                              raw_point,
                      SIDE_CHANNEL_REGION_OFF_SCREEN);
    return;
  }

  if (eposParams.m_send_all_events_to_side_channel)
  {
    send_socket_event(raw_point,
                      SIDE_CHANNEL_REGION_ALL);
  }
}

/*==============================================================================
  FUNCTION:  library_calculate_points
==============================================================================*/
/**
  Handles library calculations of points.
*/
int library_calculate_points(int32_t *pPacket,
                             void *InWorkspace,
                             FeedbackInfo *OutFeedback)
{
  // Time before call to epos lib calc
  double timeBeforeCalc = usf_get_time();
  int num_points = epos_lib_proxy->get_points(pPacket,
                                              InWorkspace,
                                              OutFeedback);
  if (!epos_lib_proxy->extend_epoints(InWorkspace,
                                      num_points,
                                      eposParams.m_out_basic_points,
                                      eposParams.m_out_points))
  {
    LOGE("%s: extend_epoints failed",
         __FUNCTION__);
    epos_exit(EXIT_FAILURE);
  }
  // Time after call to epos lib calc
  double calcDuration = usf_get_time() - timeBeforeCalc;

  get_act_channels(InWorkspace, timeBeforeCalc);

  get_spurs(InWorkspace,timeBeforeCalc);

  // After calling GetPoints, the EPOS lib returns a feedback
  usf_epos_handle_feedback(InWorkspace,
                           OutFeedback);

  eposStats.m_nTimesOfEposLibCalc += 1;
  eposStats.m_nTotalTimeOfEposLibCalc += (calcDuration / (double)eposParams.m_group);

  if (0 > num_points)
  {
    LOGE("%s: EPOS GetPoints returned %d",
         __FUNCTION__,
         num_points);
    epos_exit(EXIT_FAILURE);
  }

  return num_points;
}

/*==============================================================================
  FUNCTION:  epos_get_points
==============================================================================*/
/**
  Calculate points using the library.
  Transform the points and send them to socket and input module.
*/
static void epos_get_points (int32_t *pPacket,
                             void *InWorkspace,
                             FeedbackInfo *OutFeedback)
{
  bool change_ts_state = false;
  bool all_points_invalid = true;
  region_zone_t region_zone = NOT_IN_ANY_ACTIVE_ZONE;
  static double time_of_last_valid_point = usf_get_time();

  int num_points = library_calculate_points(pPacket, InWorkspace, OutFeedback);
  for (int i = 0; i < num_points; i++)
  {
    if (!sb_run)
    { // It's signal to stop daemon
      epos_exit(EXIT_SUCCESS);
    }

    stats_store_raw_point(eposParams.m_out_points[i],
                          (eposParams.m_out_points[i].Type &
                           EPOS_TYPE_PEN_DOWN_BIT));

    if (!is_point_valid(&eposParams.m_out_points[i]))
    {
      // Current point is marked as invalid, skip
      continue;
    }
    all_points_invalid = false;
    time_of_last_valid_point = usf_get_time();
    eposStats.m_nPointsCalculated++;
    get_battery_state(InWorkspace);

    usf_extended_epoint_t outPoint;
    region_zone = transform_point(eposParams.m_out_points[i],
                                  &outPoint,
                                  on_screen_trans_props,
                                  off_screen_trans_props);
    zone_t zone = get_zone(region_zone,
                           eposParams.m_out_points[i]);

    s_work_params.bActZone = (ACTIVE_ZONE_BORDER == zone ||
                              WORKING_ZONE == zone);

    enable_disable_ts(outPoint.Type & EPOS_TYPE_PEN_DOWN_BIT,
                      outPoint.Z,
                      region_zone);

    if (!is_diff_point(eposParams.m_out_points[i]) ||
        eposParams.m_nDownscalePointCounter)
    { // If the point is not different than its predecessor or
      // the point should be downscaled, then it is dropped
      eposParams.m_nDownscalePointCounter = (eposParams.m_nDownscalePointCounter) ?
      (eposParams.m_nDownscalePointCounter - 1) : eposParams.m_cfg_point_downscale;
      ++eposStats.m_nPointsDropped;
      continue;
    }

    uint16_t event_type_index = get_event_type_index(region_zone);

    stats_store_trasformed_point(outPoint,
                                 event_type_index,
                                 region_zone);

    if (ACTIVE_ZONE_BORDER == zone ||
        NOT_ACTIVE_ZONE == zone)
    {
      handle_outside_work_zone_event();
      // Send side channel event
      if (eposParams.m_send_all_events_to_side_channel)
      {
        send_socket_event(eposParams.m_out_points[i],
                          SIDE_CHANNEL_REGION_ALL);
      }
    }
    else
    { // In work area
      handle_work_zone_event(region_zone,
                             outPoint,
                             eposParams.m_out_points[i],
                             event_type_index);
    }
    // When reaching here, m_nDownscalePointCounter == 0
    eposParams.m_nDownscalePointCounter = eposParams.m_cfg_point_downscale;

    eposParams.m_nLastPoint = outPoint;
    eposParams.m_nLastEventIndex = event_type_index;
  } // End for

  if (all_points_invalid)
  {
    if (MAX_TIME_WITHOUT_VALID_POINT_ENABLE_TS_MSEC <=
        (usf_get_time() - time_of_last_valid_point))
    {
      // If last point was in working zone and pressed, and there
      // are no points anymore, need to send up event.
      handle_outside_work_zone_event();
      // If EPOS lib continuously returns 0 valid points,
      // the touchscreen is enabled
      enable_disable_ts(false,
                        eposParams.m_touch_disable_threshold,
                        NOT_IN_ANY_ACTIVE_ZONE);
      time_of_last_valid_point = usf_get_time();
    }
  }

  record_coord_file(num_points);
}

/*==============================================================================
  FUNCTION:  print_DSP_ver
==============================================================================*/
/**
  Print DSP version to file.
*/
void print_DSP_ver()
{
  char szVersion [256] = {0};
  unsigned char nVersionNum[4] = {0};

  FILE* fp = fopen (USF_DSP_VER_FILE,
                    "wt");
  if (fp == NULL)
  {
    LOGE("%s: Could not open %s - %s",
         __FUNCTION__,
         USF_DSP_VER_FILE,
         strerror(errno));
    epos_exit(EXIT_FAILURE);
  }

  epos_lib_proxy->get_dsp_version(szVersion,
                                  nVersionNum);

  fprintf (fp,
           "%x.%x.%x.%x\n",
           nVersionNum[0],
           nVersionNum[1],
           nVersionNum[2],
           nVersionNum[3]);
  fclose (fp);
}


/*==============================================================================
  FUNCTION:  signal_handler
==============================================================================*/
/**
  Signals handler
*/
void signal_handler (int sig)
{
  LOGD("%s: Received signal %d; sb_run=%d",
         __FUNCTION__, sig, sb_run);
  // All supportd signals (except ALARM) cause the daemon exit
  sb_run = false;
  // Repeat exit request to wake-up blocked functions
  alarm(1);
}

/*==============================================================================
  FUNCTION:  print_debug_points
==============================================================================*/
/**
  Prints point debug information to the log.
*/
void print_debug_points()
{
  if (s_work_params.paramsStruct.usf_epos_debug_print_interval == s_work_params.pointLogFrameCounter)
  {
    if (eposStats.m_bLastPointDropped)
    {
      LOGD("Raw point X:%f Y:%f Z:%f, tilt X:%f Y:%f Z:%f pressed:%d, point dropped.",
           eposStats.m_lastRawPoint.get_element(X),
           eposStats.m_lastRawPoint.get_element(Y),
           eposStats.m_lastRawPoint.get_element(Z),
           eposStats.m_lastRawTilt.get_element(X),
           eposStats.m_lastRawTilt.get_element(Y),
           eposStats.m_lastRawTilt.get_element(Z),
           eposStats.m_lastPenPressed);
    }
    else
    {
      LOGD("Raw point X:%f Y:%f Z:%f, tilt X:%f Y:%f Z:%f pressed:%d, transformed X:%f Y:%f Z:%f tilt X:%f Y:%f event type:%d region zone: %d",
           eposStats.m_lastRawPoint.get_element(X),
           eposStats.m_lastRawPoint.get_element(Y),
           eposStats.m_lastRawPoint.get_element(Z),
           eposStats.m_lastRawTilt.get_element(X),
           eposStats.m_lastRawTilt.get_element(Y),
           eposStats.m_lastRawTilt.get_element(Z),
           eposStats.m_lastPenPressed,
           eposStats.m_lastTransformedPoint.get_element(X),
           eposStats.m_lastTransformedPoint.get_element(Y),
           eposStats.m_lastTransformedPoint.get_element(Z),
           eposStats.m_lastTransformedTilt.get_element(X),
           eposStats.m_lastTransformedTilt.get_element(Y),
           eposStats.m_lastEventType,
           eposStats.m_lastRegionZone);
    }
    s_work_params.pointLogFrameCounter = 0;
  }
}

/*==============================================================================
  FUNCTION:  usf_epos_get_points
==============================================================================*/
/**
  Calculates points upon US data
*/
bool usf_epos_get_points(bool& b_us)
{
  EposPacket *eposPacket = NULL;

  /* Epos lib update fb_info.ActiveState only as it's changed */
  // By default, no ultrasound signal exists
  static FeedbackInfo fb_info = {NO_US_SIGNAL, 0, {0, 0, 0, 0}};

  for (int f = 0; f < s_work_params.numberOfFrames ; f++)
  {
    s_work_params.nextPacket += s_work_params.frame_hdr_size_in_bytes;
    eposPacket = (EposPacket *)s_work_params.nextPacket;
    // Statistics
    // If this is the first iteration then the frames
    // counter is -1 and we need to update the frames counter.
    // During sniffing mode dropped frames are ignored.
    if (eposParams.m_nNextFrameSeqNum == -1 || !s_work_params.bActZone)
    {
      eposParams.m_nNextFrameSeqNum = eposPacket->packet_num;
    }
    // This is not the first iteration.
    else
    {
      if (eposParams.m_nNextFrameSeqNum != eposPacket->packet_num)
      {
        // We lost some frames so we add the number of lost frames
        // to the statistics.
        if (eposParams.m_nNextFrameSeqNum < eposPacket->packet_num)
        {
          eposStats.m_nLostFrames += (eposPacket->packet_num -
                                      eposParams.m_nNextFrameSeqNum)/
                                      s_work_params.paramsStruct.usf_tx_skip;
        }
        // We got out of order frames so we add the number of
        // out of order frames to the statistics.
        else
        {
          eposStats.m_nOutOfOrderErrors +=
            (eposParams.m_nNextFrameSeqNum - eposPacket->packet_num)/
            s_work_params.paramsStruct.usf_tx_skip;
        }

        // Update the frames counter to the correct count.
        eposParams.m_nNextFrameSeqNum = eposPacket->packet_num;
      }
    }
    eposStats.m_nTotalFrames++;
    // Update the frames counter to the expected count in the next
    // iteration.
    eposParams.m_nNextFrameSeqNum += s_work_params.paramsStruct.usf_tx_skip;
    s_work_params.packetCounter++;
    if (STATISTIC_PRINT_INTERVAL == s_work_params.packetCounter)
    {
      LOGD("%s: Statistics (printed every %d frames):",
           __FUNCTION__,
           STATISTIC_PRINT_INTERVAL);
      LOGD("Points calculated: %d, points dropped: %d, "
           "calculated frames: %d, lost frames: %d, "
           "out of order: %d, mean time of epos lib calculation: %.3f",
           eposStats.m_nPointsCalculated,
           eposStats.m_nPointsDropped,
           eposStats.m_nTotalFrames,
           eposStats.m_nLostFrames,
           eposStats.m_nOutOfOrderErrors,
           eposStats.m_nTotalTimeOfEposLibCalc /
           (double)eposStats.m_nTimesOfEposLibCalc);
      s_work_params.packetCounter = 0;
    }
    s_work_params.pointLogFrameCounter++;
    print_debug_points();
    // Calculation
    for (int mic = 0; mic < s_work_params.paramsStruct.usf_tx_port_count; mic++)
    {
      epos_get_points((int32_t*)s_work_params.nextPacket,
                      eposParams.m_epos_workspace,
                      &fb_info);

      s_work_params.nextPacket += s_work_params.packet_size_in_bytes;
    } // End for mic (microphones)
  } // End for f (frames)

  b_us = (NO_US_SIGNAL != fb_info.ActiveState);

  return true;
} // usf_epos_get_points

/*==============================================================================
  FUNCTION:  send_smart_stand_angle_command
==============================================================================*/
/**
 * Sends Command to the library with the given smart stand angle
 *
 * @param in_angle[in] angle to send to the library
 * @param out_angle[out] the angle to be used by the the daemon.
 *
 * @return int 0 - success, 1 - failure
 */
int send_smart_stand_angle_command(double in_angle,
                                   double &out_angle)
{
  struct usf_epos_command cmd_smart_stand;
  cmd_smart_stand.cmd_type = CMD_TYPE_SET;
  cmd_smart_stand.cmd_num = COMMAND_RotationAngle;
  int rotation_angle_q_factor = (1 << Q15_SHIFT);
  cmd_smart_stand.args[0] = round(in_angle * rotation_angle_q_factor);
  int ret = epos_lib_proxy->command(eposParams.m_epos_workspace,
                                    (int32_t*)&cmd_smart_stand);
  if (ret)
  {
    LOGW("%s, Command for setting rotation angle failed.",
         __FUNCTION__);
    return 1;
  }
  else
  {
    // Use the same rotation angle as in the library
    out_angle = ((double)cmd_smart_stand.args[0] / rotation_angle_q_factor) * DEG_TO_RAD;
    LOGD("Setting accelerometer angle to: %f", out_angle / DEG_TO_RAD);
    return 0;
  }
}

/*==============================================================================
  FUNCTION:  update_config_parameters
==============================================================================*/
/**
  Updates the configuration parameters in epos work parameters.
  This function should be called when holding control_socket_mutex
*/
void update_config_parameters()
{
  switch (eposParams.m_dconfig.off_screen_mode)
  {
    case DC_OFF_SCREEN_MODE_DISABLED:
      eposParams.m_off_screen_send_points_to_ual = false;
      eposParams.m_off_screen_send_points_to_socket = false;
      break;
    case DC_OFF_SCREEN_MODE_EXTEND:
      eposParams.m_off_screen_mode = OFF_SCREEN_MODE_EXTEND;
      eposParams.m_off_screen_send_points_to_ual =
        ((eposParams.m_dconfig.off_screen_destination == DP_COORD_DESTINATION_MOTION_EVENT) ||
         (eposParams.m_dconfig.off_screen_destination == DP_COORD_DESTINATION_BOTH));
      eposParams.m_off_screen_send_points_to_socket =
        ((eposParams.m_dconfig.off_screen_destination == DP_COORD_DESTINATION_SOCKET) ||
         (eposParams.m_dconfig.off_screen_destination == DP_COORD_DESTINATION_BOTH));
      break;
    case DC_OFF_SCREEN_MODE_DUPLICATE:
      eposParams.m_off_screen_mode = OFF_SCREEN_MODE_DUPLICATE;
      eposParams.m_off_screen_send_points_to_ual =
        ((eposParams.m_dconfig.off_screen_destination == DP_COORD_DESTINATION_MOTION_EVENT) ||
         (eposParams.m_dconfig.off_screen_destination == DP_COORD_DESTINATION_BOTH));
      eposParams.m_off_screen_send_points_to_socket =
        ((eposParams.m_dconfig.off_screen_destination == DP_COORD_DESTINATION_SOCKET) ||
         (eposParams.m_dconfig.off_screen_destination == DP_COORD_DESTINATION_BOTH));
      break;
  default:
      LOGW("%s, Unidentified off screen mode: %d",
           __FUNCTION__,
           eposParams.m_dconfig.off_screen_mode);
      break;
  }
  eposParams.m_on_screen_send_points_to_ual =
        ((eposParams.m_dconfig.on_screen_destination == DP_COORD_DESTINATION_MOTION_EVENT) ||
         (eposParams.m_dconfig.on_screen_destination == DP_COORD_DESTINATION_BOTH));
      eposParams.m_on_screen_send_points_to_socket =
        ((eposParams.m_dconfig.on_screen_destination == DP_COORD_DESTINATION_SOCKET) ||
         (eposParams.m_dconfig.on_screen_destination == DP_COORD_DESTINATION_BOTH));

  on_screen_trans_props.hover_max_range = eposParams.m_dconfig.on_screen_hover_max_range;
  eposParams.m_event_type = eposParams.m_dconfig.event_type;
  eposParams.m_off_screen_plane = eposParams.m_dconfig.off_screen_plane;
  if (eposParams.m_smarter_stand_enable &&
      !eposParams.m_dconfig.smarter_stand_enable)
  {
    send_smart_stand_angle_command(0.0,
                                   eposParams.m_smarter_stand_angle);
  }
  eposParams.m_smarter_stand_enable = eposParams.m_dconfig.smarter_stand_enable;
  // Called to update the rotation axis, which also determines the Z coordinate
  // of the off-screen area which may have been updated (even if smarter stand is
  // disabled)
  epos_init_smarter_stand();

  // Eraser
  eposParams.m_eraser_button_index = eposParams.m_dconfig.eraser_button_index;
  eposParams.m_eraser_button_mode = eposParams.m_dconfig.eraser_button_mode;

  // Hovering icon
  eposParams.m_on_screen_hover_icon_mode =
    (eposParams.m_dconfig.show_on_screen_hover_icon ?
     HOVER_ICON_PALM_AREA : HOVER_ICON_DISABLED);

  eposParams.m_off_screen_hover_icon_mode =
    (eposParams.m_dconfig.show_off_screen_hover_icon ?
     HOVER_ICON_PALM_AREA : HOVER_ICON_DISABLED);

  eposParams.m_touch_disable_threshold = eposParams.m_dconfig.touch_disable_threshold;
  // Side-channel
  eposParams.m_send_all_events_to_side_channel = eposParams.m_dconfig.send_all_events_to_side_channel;
  eposParams.m_socket_on_screen_mapped = eposParams.m_dconfig.on_screen_mapping;
  eposParams.m_socket_off_screen_mapped = eposParams.m_dconfig.off_screen_mapping;


  LOGD("Config was updated: on screen ual:%d, off screen ual:%d, on screen socket:%d, off screen socket:%d, event type:%d, off screen mode:%d, eraser button index:%d, eraser button mode:%d, on screen hover icon mode: %d, off screen hover icon mode: %d, touch range: %d, all events side channel: %d, on screen mapping: %d, off screen mapping: %d",
       eposParams.m_on_screen_send_points_to_ual,
       eposParams.m_off_screen_send_points_to_ual,
       eposParams.m_on_screen_send_points_to_socket,
       eposParams.m_off_screen_send_points_to_socket,
       eposParams.m_event_type,
       eposParams.m_off_screen_mode,
       eposParams.m_eraser_button_index,
       eposParams.m_eraser_button_mode,
       eposParams.m_on_screen_hover_icon_mode,
       eposParams.m_off_screen_hover_icon_mode,
       eposParams.m_touch_disable_threshold,
       eposParams.m_send_all_events_to_side_channel,
       eposParams.m_socket_on_screen_mapped,
       eposParams.m_socket_off_screen_mapped);
}

/*==============================================================================
  FUNCTION:  zero_angle_rounding
==============================================================================*/
/**
  Sets angle to 0 when it is close by threshold to 0.
*/
void zero_angle_rounding(int thres,
                         double &angle)
{
  if ((0 < angle &&
       thres > angle) ||
      (360 > angle &&
       360 - thres < angle))
  {
    angle = 0;
  }
}

/*==============================================================================
  FUNCTION:  update_config
==============================================================================*/
/**
  Updates epos work parameters from dynamic config, when changed.
*/
void update_config(us_all_info *paramsStruct)
{
  bool update_transform = false;
  int rc = pthread_mutex_lock(&control_socket_mutex);
  if(rc)
  {
    LOGE("%s: pthread_mutex_lock failed: %d",
         __FUNCTION__,
         rc);
    epos_exit(EXIT_FAILURE);
  }

  if (eposParams.m_dconfig.config_changed)
  {
    update_config_parameters();
    // Need to update suitable transform settings
    update_transform = true;
  }
  if (eposParams.m_dconfig.angle_changed &&
      eposParams.m_smarter_stand_enable)
  {
    zero_angle_rounding(paramsStruct->usf_epos_zero_angle_thres,
                        eposParams.m_dconfig.smarter_stand_angle);
    int ret = send_smart_stand_angle_command(eposParams.m_dconfig.smarter_stand_angle,
                                             eposParams.m_smarter_stand_angle);
    if (!ret)
    {
      // Need to update suitable transform settings
      update_transform = true;
    }
  }

  // All config parameters were updated
  eposParams.m_dconfig.config_changed = false;
  eposParams.m_dconfig.angle_changed = false;

  rc = pthread_mutex_unlock(&control_socket_mutex);
  if(rc)
  {
    LOGE("%s: pthread_mutex_unlock failed: %d",
         __FUNCTION__,
         rc);
    epos_exit(EXIT_FAILURE);
  }

  if (update_transform)
  {
    plane_properties off_screen_plane = eposParams.m_off_screen_plane;
    rotate_off_screen_plane(paramsStruct->usf_epos_rotation_axis_origin,
                            paramsStruct->usf_epos_rotation_axis_direction,
                            eposParams.m_smarter_stand_angle,
                            off_screen_plane);
    Vector act_zone_area_border;
    act_zone_area_border = paramsStruct->usf_epos_off_screen_act_zone_border;
    calc_plane_matrix(off_screen_plane,
                      off_screen_trans_props,
                      act_zone_area_border);
  }
}

/*==============================================================================
  FUNCTION:  set_sniffing_mode
==============================================================================*/
/**
  Sets sniffing mode of the library on and off.
  when on, library calculates points, even if the result is inaccurate.
*/
void set_sniffing_mode(bool value)
{
  static bool curr_sniff_mode = false;
  if (curr_sniff_mode == value)
  { // Sniffing already set to this value
    return;
  }
  LOGD("%s: to: %d ", __FUNCTION__, value);
  struct usf_epos_command cmd_sniffing_mode;
  cmd_sniffing_mode.cmd_type = CMD_TYPE_SET;
  cmd_sniffing_mode.cmd_num = CMD_SNIFFING_MODE;
  cmd_sniffing_mode.args[0] = value;
  int ret = epos_lib_proxy->command(eposParams.m_epos_workspace,
                                    (int32_t*)&cmd_sniffing_mode);
  // TODO: Uncomment when next library returns 0 on success and 1
  // for failure
  /*if (ret)
  {
    LOGW("%s, Command for sniffing mode failed.",
         __FUNCTION__);
    return;
  }*/
  curr_sniff_mode = value;
}

/*==============================================================================
  FUNCTION:  usf_handle_zone
==============================================================================*/
/**
  Handle active and non-active zones

  In active zone the service is in normal active work state.

  "no active zone" behavior includes 2 repeated stages: sleep and probe.
  During the sleep stage there is no USND data traffic and points calculation.
  In the probe stage, the service is returned to normal active work state
  Moving to "no active zone" behavior takes place after
  predefined number of "empty" frames.
  "Empty" frames mean, no points were calculated upon them
*/
static void usf_handle_zone(uint32_t frames_counter)
{
  // It's time (msec) to start sleep stage
  // of the "no active zone" behavior
  static double s_no_act_zone_end_probe_time = 0;
  // The current amount of "empty" frames
  // "Empty" frames mean, no points were calculated upon them
  static uint32_t s_empty_frames_counter = 0;

  if (0 < s_work_params.paramsStruct.no_act_zone_sleep_duration)
  { // no active zone power saving is enabled
    double current_time_msec = usf_get_time();

    if (s_work_params.bActZone)
    {
      // preparing for the next "no active zone" situation
      s_empty_frames_counter = 0;
      set_sniffing_mode(false);
    }
    else
    {
      s_empty_frames_counter += frames_counter;
      if ((s_work_params.paramsStruct.no_act_zone_empty_frames_count < s_empty_frames_counter) &&
          (current_time_msec >= s_no_act_zone_end_probe_time))
      {
        set_sniffing_mode(true);
        ual_data_type data;
        // sleep stage of the "no active zone" behavior
        int ret = usleep(s_work_params.paramsStruct.no_act_zone_sleep_duration *
                         1000);
        if (-1 == ret)
        {
          LOGW("%s: no act zone: interrupt; time=%f; endProbeTime=%f",
               __FUNCTION__,
               current_time_msec,
               s_no_act_zone_end_probe_time);
        }

        // Clean old data from the shared memory
        s_work_params.numPoints = 0;
        (void)ual_read(&data,
                       NULL,
                       0);

        // start of the "probe" stage of the "no active zone" behavior
        current_time_msec = usf_get_time();
        s_no_act_zone_end_probe_time = current_time_msec +
                       s_work_params.paramsStruct.no_act_zone_probe_duration;
      }
    }
  } // no active zone power saving
}

/*==============================================================================
  FUNCTION:  simulate_input_events
==============================================================================*/
/**
  The function simulates pre-defined input events,
  taking into account time period, which simulation takes place in, and
  "3D" configuration parameter.
  The time periods are defined by pen down/up time durations.
  The durations are 4-byte little-endian numbers,
  stored in some configurable file.
  The numbers in even indexes (the first number is in index 0)
  are pen down durations; the numbers in odd indexes are pen up durations.
*/
void simulate_input_events(us_all_info *paramsStruct)
{
  static bool sb_init = false;
  static int s_fd = 0;
  static int s_duration_num = 0;
  static const uint32_t MAX_DURATIONS_AMOUNT = 1000;
  static uint32_t s_durations[MAX_DURATIONS_AMOUNT];
  static uint32_t s_ind = 0;
  static double s_end_period = 0;
  static const int SIMULATED_PRESSURE_DOWN = 1;
  static const int SIMULATED_PRESSURE_UP = 0;
  // initiate up2down switch for the first duration
  static int s_pressure = SIMULATED_PRESSURE_UP;
  static const int SIMULATED_X1 = 30000;
  static const int SIMULATED_X2 = 31000;
  static const int SIMULATED_Y = 30000;
  static const int SIMULATED_Z_UP = 100;
  static int s_x = SIMULATED_X1;
  static int s_z = 0;
  static const uint32_t MAX_INT32 = 0x7FFFFFFF;
  static uint32_t s_event_counter = MAX_INT32;

  // The simulation isn't required
  if (-1 == s_fd)
    return;

  if (!sb_init)
  {
    sb_init = true;
    s_fd = open(paramsStruct->usf_epos_durations_file,
                O_RDONLY);
    if (-1 == s_fd)
    {
      LOGW("%s: file(%s) open failure: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_durations_file,
           errno);
      return;
    }
  }

  double current_time_msec = usf_get_time();

  if (current_time_msec > s_end_period)
  { // it's time to switch between down & up
    if (0 == s_duration_num)
    { // refill durations array
      // the first (ind=0) & all even numbers are "down" durations
      s_duration_num = read(s_fd,
                            s_durations,
                            sizeof(s_durations));
      if (0 >= s_duration_num)
      {
        LOGW("%s: file(%s) read end or failure; ret=%d; err=%d",
             __FUNCTION__,
             paramsStruct->usf_epos_durations_file,
             s_duration_num,
             errno);
        close(s_fd);
        s_fd = -1;
        epos_exit(EXIT_SUCCESS);
      }

      s_duration_num = s_duration_num /
                       sizeof(s_durations[0]);
      s_ind = 0;
    } // refill durations array

    if (SIMULATED_PRESSURE_DOWN == s_pressure)
    {
      // Re-send the last point, but with "UP" Z coordinate
      s_x = (SIMULATED_X1 + SIMULATED_X2) - s_x;
      s_z = SIMULATED_Z_UP;
      s_pressure = SIMULATED_PRESSURE_UP;
      // In the case of 2D, only one UP event is sent
      s_event_counter = (paramsStruct->usf_epos_on_screen_hover_max_range)?
                        MAX_INT32: 1;
   }
    else
    {
      s_z = 0;
      s_pressure = SIMULATED_PRESSURE_DOWN;
      s_event_counter = MAX_INT32;
    }

    LOGD("%s: s_ind=%d; s_pressure=%d; duration=%d",
           __FUNCTION__,
           s_ind,
           s_pressure,
           s_durations[s_ind]);

    s_end_period = current_time_msec + s_durations[s_ind];
    ++s_ind;
    --s_duration_num;
  } // switch between down & up

  // Prepare input event
  if (s_event_counter)
  {
    --s_event_counter;
    usf_event_type *pEvent = &eposParams.m_events[0];
    s_work_params.numPoints = 1;
    pEvent->seq_num = eposParams.m_nLastEventSeqNum++;
    pEvent->timestamp = (uint32_t)(clock ());
    pEvent->event_type_ind = USF_TSC_EVENT_IND;
    pEvent->event_data.point_event.pressure = s_pressure;
    pEvent->event_data.point_event.coordinates[X_IND] = s_x;
    pEvent->event_data.point_event.coordinates[Y_IND] = SIMULATED_Y;
    pEvent->event_data.point_event.coordinates[Z_IND] = s_z;

    // Prepare for the next itteration
    s_x = (SIMULATED_X1 + SIMULATED_X2) - s_x;
  }
}

/*==============================================================================
  FUNCTION:  usf_act_work_func
==============================================================================*/
/**
  Work function in the PS Active state.
*/
bool usf_act_work_func(work_params_type& work_params,
                       ps_transit_enum& ps_transit)
{
  ual_data_type data;
  bool rc = true;
  static uint32_t s_prev_ual_status = 0;
  static bool frame_file_created = false;
  // Number of frames, handled in the previous call
  static uint32_t s_prev_frames_counter = 0;
  uint32_t ual_status = ual_get_status();

  if (s_prev_ual_status != ual_status)
  {
    if (!(ual_status & UAL_RX_STATUS_ON))
    {
      usf_epos_notify_RX_stop(eposParams.m_epos_workspace);
    }
    s_prev_ual_status = ual_status;
  }

  update_config(&work_params.paramsStruct);

  simulate_input_events(&work_params.paramsStruct);

  usf_handle_zone(s_prev_frames_counter);

  rc = ual_read(&data,
                eposParams.m_events,
                work_params.numPoints);
  if (!rc)
  {
    LOGE("%s: ual_read failed",
         __FUNCTION__);
    if (NULL != work_params.frameFile)
    {
      fclose(work_params.frameFile);
      work_params.frameFile = NULL;
    }
    if (NULL != work_params.coordFile)
    {
      fclose(work_params.coordFile);
      s_work_params.coordFile = NULL;
    }

    epos_exit(EXIT_FAILURE);
  }

  // Frames record
  if (0 < work_params.paramsStruct.usf_frame_count &&
      false == frame_file_created)
  {
    frame_file_created = true;
    // Open frame file from cfg file
    work_params.frameFile = ual_util_get_frame_file(&work_params.paramsStruct,
                                                    (char *)FRAME_FILE_DIR_PATH);
    if (NULL == work_params.frameFile)
    {
      LOGE("%s: ual_util_get_frame_file failed",
           __FUNCTION__);
      return false;
    }
  }

  if (0 == data.region[0].data_buf_size)
  {
    return true;
  }

  // prepare for the new points calculation upon new data
  work_params.numPoints = 0;
  s_work_params.bActZone = false;

  bool b_us = true;

  // Underlay layer provides US data frames in buffers.
  // Each buffer includes one group of the frames.
  // A number of frames is defined by configurable group factor.
  work_params.numberOfFrames = work_params.paramsStruct.usf_tx_buf_size /
                               work_params.frame_size_in_bytes;
  int group_data_size = work_params.numberOfFrames *
                        work_params.frame_size_in_bytes;
  s_prev_frames_counter = 0;
  for (int r = 0; r < work_params.num_of_regions; r++)
  {
    int num_of_groups = data.region[r].data_buf_size /
                                  work_params.paramsStruct.usf_tx_buf_size;
    s_prev_frames_counter += num_of_groups * work_params.paramsStruct.usf_tx_group;
    uint8_t *pGroupData = data.region[r].data_buf;
    for (int g = 0; g < num_of_groups; g++)
    {
      work_params.nextPacket =  pGroupData;

      // Recording to frameFile
      if (work_params.numOfBytes < work_params.bytesWriteToFile)
      {
        bool bRes = ((work_params.numOfBytes + group_data_size) <=
                     work_params.bytesWriteToFile);
        uint32_t bytestFromGroup = (bRes) ? group_data_size :
                      (work_params.bytesWriteToFile - work_params.numOfBytes);
        ual_util_frame_file_write(pGroupData,
                                  sizeof(uint8_t),
                                  bytestFromGroup,
                                  &(s_work_params.paramsStruct),
                                  work_params.frameFile);
        work_params.numOfBytes += bytestFromGroup;
        if (work_params.numOfBytes >= work_params.bytesWriteToFile)
        {
          if (NULL != work_params.frameFile)
          {
            fclose(work_params.frameFile);
            work_params.frameFile = NULL;
          }
        }
       }
       rc = usf_epos_get_points(b_us);
       if (!rc)
       {
         if (NULL != work_params.frameFile)
         {
           fclose(work_params.frameFile);
           work_params.frameFile = NULL;
         }
         if (NULL != work_params.coordFile)
         {
           fclose(work_params.coordFile);
           work_params.coordFile = NULL;
         }
         return false;
       }
       pGroupData += work_params.paramsStruct.usf_tx_buf_size;
    } // End for g (groups)
  } // End for r (regions)

  usf_ps_act_set_transit_timer(b_us, ps_transit);
  // ps_transit may be changed in the usf_ps_act_set_transit_timer

  return true;
} // usf_act_work_func

/*============================================================================
  FUNCTION:  timeoutExpired
============================================================================*/
/**
 * Specifies whether the given timeout expired.
 *
 * @param start_time The time from which the timeout should be considered.
 * @param timeout_sec The timeout in seconds
 *
 * @return bool True - the timeout expired
 *              False - otherwise
 */
bool timeoutExpired(timespec start_time, int timeout_sec)
{
  timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time);
  time_t duration_sec =  current_time.tv_sec - start_time.tv_sec;

  if (duration_sec > timeout_sec || (duration_sec == timeout_sec &&
                                     current_time.tv_nsec >= start_time.tv_nsec))
  {
    return true;
  }
  return false;
}

/*============================================================================
  FUNCTION:  main
============================================================================*/
/**
  Main function of the EPOS daemon. Handle all the EPOS operations.
*/
int main(void)
{
  bool rc = false;
  ual_data_type data;
  ps_transit_enum ps_transit = PS_NO_TRANSIT;

  LOGI("%s: Epos start",
       __FUNCTION__);

  // Setup signal handling
  signal(SIGHUP,
         signal_handler);
  signal(SIGTERM,
         signal_handler);
  signal(SIGINT,
         signal_handler);
  signal(SIGQUIT,
         signal_handler);
  signal(SIGALRM,
         ual_util_alarm_handler);

  if (ual_util_declare_pid(getpid(),
                           PID_FILE_NAME))
  {
    LOGE("%s: Declare_pid failed",
         __FUNCTION__);
  }

  if (false == ual_util_is_supported((char *)CLIENT_NAME))
  {
    LOGE("%s: Daemon is not supported",
         __FUNCTION__);
    epos_exit(EXIT_FAILURE);
  }

  if (ual_util_daemon_init(&s_work_params.paramsStruct,
                           (char *)LINK_CFG_FILE_LOCATION,
                           cfgFile,
                           (char *)CLIENT_NAME))
  {
    LOGE("%s: ual_util init failed",
         __FUNCTION__);
    epos_exit(EXIT_FAILURE);
  }

  ual_cfg_type cfg;
  cfg.usf_dev_id = 1;
  cfg.ual_mode = static_cast<ual_work_mode_type>(s_work_params.paramsStruct.ual_work_mode);
  rc = ual_open(&cfg);
  if (!rc)
  {
    LOGE("%s: ual_open: rc=%d",
         __FUNCTION__,
         rc);
    epos_exit(EXIT_FAILURE);
  }

  ual_util_print_US_version(CLIENT_NAME,
                            CLIENT_VERSION);

  s_work_params.paramsStruct.usf_x_tilt[MIN_IND] = MIN_TILT;
  s_work_params.paramsStruct.usf_x_tilt[MAX_IND] = MAX_TILT;
  s_work_params.paramsStruct.usf_y_tilt[MIN_IND] = MIN_TILT;
  s_work_params.paramsStruct.usf_y_tilt[MAX_IND] = MAX_TILT;

  s_work_params.paramsStruct.usf_tsc_pressure[MAX_IND] = MIN_PRESSURE;
  s_work_params.paramsStruct.usf_tsc_pressure[MAX_IND] = MAX_PRESSURE;

  s_work_params.paramsStruct.conflicting_event_types = USF_TSC_EVENT |
                                                       USF_TSC_PTR_EVENT;

  // US TX should support the three TSC event types
  // Backup event_type from the configuration file
  uint32_t temp_event_type = s_work_params.paramsStruct.usf_event_type;
  s_work_params.paramsStruct.usf_event_type = USF_TSC_EVENT |
                                              USF_TSC_PTR_EVENT |
                                              USF_TSC_EXT_EVENT;

  // Build tx_transparent_data manually. In the future (hopefully),
  // this will be removed.
  ual_util_set_epos_tx_transparent_data(&s_work_params.paramsStruct);

  ual_util_set_tx_buf_size(&s_work_params.paramsStruct);

  if (ual_util_tx_config(&s_work_params.paramsStruct,
                         (char *)CLIENT_NAME))
  {
    LOGE("%s: ual_util_tx_config failed",
         __FUNCTION__);
    epos_exit(EXIT_FAILURE);
  }

  // Restore event_type from the configuration file
  s_work_params.paramsStruct.usf_event_type = temp_event_type;
  epos_params_init(&s_work_params.paramsStruct);

  configure_ps(s_work_params.paramsStruct);

  // true for first_time call
  epos_init(true);

  print_DSP_ver();

  // Frames record
  if (0 >= s_work_params.paramsStruct.usf_frame_count)
  {
    LOGD("%s: usf_frame_count is %d. No record has made.",
         __FUNCTION__,
         s_work_params.paramsStruct.usf_frame_count);
  }

  // Epos coordinate record
  if (0 >= s_work_params.paramsStruct.usf_epos_coord_count)
  {
    LOGW("%s: usf_epos_coord_count is %d. No record has made.",
         __FUNCTION__,
         s_work_params.paramsStruct.usf_epos_coord_count);
  }
  else
  {
    // Open coord file from cfg file
    s_work_params.coordFile = ual_util_get_file(
                                &s_work_params.paramsStruct,
                                (char *)FRAME_FILE_DIR_PATH,
                                true);
    if (NULL == s_work_params.coordFile)
    {
      LOGE("%s: ual_util_get_coord_file failed",
           __FUNCTION__);
      epos_exit(EXIT_FAILURE);
    }
  }

  s_work_params.frame_hdr_size_in_bytes =
    s_work_params.paramsStruct.usf_tx_frame_hdr_size;

  s_work_params.packet_size_in_bytes =
    s_work_params.paramsStruct.usf_tx_port_data_size *
    (s_work_params.paramsStruct.usf_tx_sample_width / BYTE_WIDTH);

  s_work_params.frame_size_in_bytes =
    s_work_params.packet_size_in_bytes *
    s_work_params.paramsStruct.usf_tx_port_count +
    s_work_params.frame_hdr_size_in_bytes;

  s_work_params.bytesWriteToFile =
                              s_work_params.paramsStruct.usf_frame_count *
                              s_work_params.frame_size_in_bytes;

  s_work_params.num_of_regions = sizeof(data.region) /
                                  sizeof(ual_data_region_type);

  // Time before first point from epos lib calc
  timespec start_time;
  clock_gettime(CLOCK_REALTIME, &start_time);
  int timeout = s_work_params.paramsStruct.usf_epos_timeout_to_coord_rec;

  while (rc && sb_run)
  {
    rc = usf_ps_work(s_work_params);

    if (0 < s_work_params.paramsStruct.usf_epos_coord_count &&
        s_work_params.recorded_coord_counter ==
          s_work_params.paramsStruct.usf_epos_coord_count)
    {
      LOGI("%s: Finished recording coordinates file. Exiting.",
             __FUNCTION__);
      break;
    }

    if (0 < timeout)
    {
      if (timeoutExpired(start_time, timeout))
      {
        LOGW("%s: Timeout expired for calibration.",
             __FUNCTION__);
        // Mark recording as not done
        int ret = property_set("debug.usf_epos.coord_rec_done",
                               "2");
        if (0 != ret)
        {
          LOGW("%s: setting recording done property failed",
               __FUNCTION__);
        }
        break;
      }
    }
  } // work loop

  epos_exit(EXIT_SUCCESS);
} // main

