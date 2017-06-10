/*===========================================================================
                           usf_sw_calib.cpp

DESCRIPTION: Implementation of the sw calib daemon.

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_sw_calib"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <stdlib.h>
#include <unistd.h>
#include <ual_util.h>
#include <ual_util_frame_file.h>
#include <cutils/properties.h>
#include "dpencalib.h"
#include <usf_unix_domain_socket.h>
#include "usf_coord_transformer.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define LINK_CFG_FILE_LOCATION "/data/usf/sw_calib/usf_sw_calib.cfg"
#define CLIENT_NAME "sw_calib"
#define FRAME_FILE_DIR_PATH "/data/usf/sw_calib/rec/"

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Ptr to the cfg file. The ptr is global bacause we want to
  close the file before exit in the function sw_calib_exit.
  Also, the needs to be closed from signal_handler so this
  has to be global.
*/
static FILE *cfgFile = NULL;

/**
 * The name of the file containg the pid of the daemon.
 */
static const char* PID_FILE_NAME = "usf_sw_calib.pid";

/**
 * Ptr to the frame file recording.
 */
FILE* frame_file = NULL;

/**
 * Configuration parameters for the daemon.
 */
static us_all_info paramsStruct;

/**
  Pairing calculator version.
*/
static const char *CLIENT_VERSION = "1.0";

/**
  The workspace for the sw_calib library.
 */
static void *sw_calib_workspace = NULL;

/**
  Value, reported by the calculator about US absence.
*/
static const time_t SOCKET_ACCEPT_TIMEOUT_SECS = 10LL;

/**
  time to wait before requesting a new statistic from the
  sw_calib library.
*/
static const uint32_t TESTER_SLEEP_TIME_MSECS = 50; // 20 times per second

/**
  Threshold for interference at a channel when is tester mode.
*/
static const uint32_t SPUR_POWER_MEAN_INTERFERENCE_THRESHOLD = 10;

/**
 * The semaphore on which the main thread waits until the client
 * connects to the socket
 */
static sem_t sem;

/**
 * The socket through which the client gets information
 * about current sw_calib status.
 */
static DataUnSocket *sck;

/**
 * The information for the creation of the transformation
 * matrix.
 */
static transformation_properties transformation_props;

/**
  The daemon running control
*/
static volatile bool daemon_run = true;

/*----------------------------------------------------------------------------
  Typedefs
----------------------------------------------------------------------------*/
typedef enum {
  SW_CALIB_SUCCESS,
  SW_CALIB_CALIBRATING,
  SW_CALIB_FAILURE,
  SW_CALIB_TIMEOUT
} sw_calib_result;

/**
 * TESTER_MSG_TYPE constants for the tester mode
 */
typedef enum
{
  TESTER_MSG_TYPE_MIN_SCALE_POWER =     0,
  TESTER_MSG_TYPE_MAX_SCALE_POWER =     1,
  TESTER_MSG_TYPE_THRESHOLD_POWER =     2,
  TESTER_MSG_TYPE_MIN_SCALE_QUALITY =   3,
  TESTER_MSG_TYPE_MAX_SCALE_QUALITY =   4,
  TESTER_MSG_TYPE_THRESHOLD_QUALITY =   5,
  TESTER_MSG_TYPE_NUM_CONSTANTS =       6,
  TESTER_MSG_TYPE_MEASUREMENT_POWER =   6,
  TESTER_MSG_TYPE_MEASUREMENT_QUALITY = 7,
  TESTER_MSG_TYPE_INTERFERENCE =        8,
} tester_msg_t;

/*----------------------------------------------------------------------------
  Function definitions
----------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  get_current_time
==============================================================================*/
/**
  returns the current time in ms.
*/
static inline double get_current_time()
{
  timespec ts_cur;
  clock_gettime (CLOCK_REALTIME, &ts_cur);
  return ((double)(ts_cur.tv_nsec) +
          ((double)ts_cur.tv_sec) * 1000000000L) /
         1000000L; // msec
}

/*==============================================================================
  FUNCTION:  on_socket_connect
==============================================================================*/
/**
  The callback function passed to the socket.
  This releases the main thread when a connection
  is accepted to the socket.
*/

void on_socket_connect() {
    LOGD("%s: Client connected to socket",
       __FUNCTION__);
    sem_post(&sem);
}

/*==============================================================================
  FUNCTION:  sw_calib_exit
==============================================================================*/
/**
  Perform clean exit of the sw_calib daemon.
*/
int sw_calib_exit (int status)
{
  // Time before call to ual_close - used to measure how much time it takes
  // to stop ual
  double time_before_ual_close = get_current_time();

  bool error_state = (status != EXIT_SUCCESS);
  int rc = ual_close(error_state);
  if (1 != rc)
  {
    LOGW("%s: ual_close: rc=%d;",
         __FUNCTION__,
         rc);
  }

  // Time after ual_close
  double close_ual_duration =  get_current_time() - time_before_ual_close;

  LOGW("%s: Duration of ual_close() is: %f msec",
       __FUNCTION__,
       close_ual_duration);

  if (NULL != cfgFile)
  {
    fclose(cfgFile);
    cfgFile = NULL;
  }

  ual_util_close_and_sync_file(frame_file);
  frame_file = NULL;

  int ret = ual_util_remove_declare_pid(PID_FILE_NAME);
  if (0 != ret)
  {
    LOGW("%s: Removing pid file failed",
         __FUNCTION__);
  }

  if (NULL != sck)
  {
      delete sck;
      sck = NULL;
  }

  if (NULL != sw_calib_workspace)
  {
    free(sw_calib_workspace);
    sw_calib_workspace = NULL;
  }

  LOGI("%s: Sw calib end. status=%d",
       __FUNCTION__,
       status);

  _exit(status);
}


/*==============================================================================
  FUNCTION:  signal_handler
==============================================================================*/
/**
  Perform clean exit after receive signal.
*/
void signal_handler (int sig)
{
  LOGD("%s: Received signal %d",
         __FUNCTION__, sig);
  daemon_run = false;
  // Repeat exit request to wake-up blocked functions
  alarm(1);
}

/*==============================================================================
  FUNCTION:  setup_signal_handling
==============================================================================*/
/**
  Sets signal_handler function as the handler for supported signals.
*/
void setup_signal_handling()
{
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
}

/*==============================================================================
  FUNCTION:  initialize_ual_util
==============================================================================*/
/**
  Checks that daemon is supported, open ual, and initializes the
  configuration parameters in paramsStruct.
*/
void initialize_ual_util()
{
  int rc;

  if (ual_util_declare_pid(getpid(),
                           PID_FILE_NAME))
  {
    LOGE("%s: Declare_pid failed",
         __FUNCTION__);
  }

  if (ual_util_daemon_init(&paramsStruct,
                           (char *)LINK_CFG_FILE_LOCATION,
                           cfgFile,
                           (char *)CLIENT_NAME))
  {
    LOGE("%s: ual_util init failed",
         __FUNCTION__);
    sw_calib_exit(EXIT_FAILURE);
  }

  ual_cfg_type cfg;
  cfg.usf_dev_id = 1;
  cfg.ual_mode = static_cast<ual_work_mode_type>(paramsStruct.ual_work_mode);
  rc = ual_open(&cfg);
  if (!rc)
  {
    LOGE("%s: ual_open: rc=%d",
         __FUNCTION__,
         rc);
    sw_calib_exit(EXIT_FAILURE);
  }

  // Build tx_transparent_data manually. In the future (hopefully),
  // this will be removed.
  ual_util_set_epos_tx_transparent_data(&paramsStruct);

  ual_util_set_tx_buf_size(&paramsStruct);

  if (ual_util_tx_config(&paramsStruct,
                         (char *)CLIENT_NAME))
  {
    LOGE("%s: ual_util_tx_config failed",
         __FUNCTION__);
    sw_calib_exit(EXIT_FAILURE);
  }

  ual_util_print_US_version(CLIENT_NAME,
                            CLIENT_VERSION);
}

/*==============================================================================
  FUNCTION:  sw_calib_load_single_coeffs
==============================================================================*/
/**
 * This function loads a calibration file and configs the library with it.
 *
 * @param calib_file_path The calibration file path.
 */
static void sw_calib_load_single_coeffs(int i)
{
  struct calib_file calib = paramsStruct.usf_sw_calib_calibration_file[i];
   calib.calib_packet =
        ual_util_malloc_read(calib.path,
                             calib.calib_packet_len);

   if (NULL == calib.calib_packet ||
       0 == calib.calib_packet_len)
   {
     if (calib.mandatory)
     {
       LOGE("%s: %s - File not found or empty - Mandatory, exiting.",
            __FUNCTION__,
            calib.path);
       sw_calib_exit(EXIT_FAILURE);
     }
     else
     {
       LOGE("%s: %s - File not found or empty - Optional, continuing.",
            __FUNCTION__,
            calib.path);
       return;
     }
   }

   int rc = dpencalib_calib(sw_calib_workspace,
                            calib.calib_packet,
                            calib.calib_packet_len);
   if (DPENCALIB_SUCCESS != rc)
   {
     LOGE("%s: Load coeffs failed for calib file: %s ret: %d",
          __FUNCTION__,
          calib.path,
          rc);
     sw_calib_exit(EXIT_FAILURE);
   }
}

/*==============================================================================
  FUNCTION:  sw_calib_load_coeffs
==============================================================================*/
/**
  Loads the calibrations files and calls the LoadCoeffs method
  from the sw_calib library.
*/
static void sw_calib_load_coeffs()
{
  // Load default calib files
  for (int i = 0; i < SW_CALIB_DEFAULT_CALIB_FILE_COUNT; ++i)
  {
    sw_calib_load_single_coeffs(i);
  }
}

/*==============================================================================
  FUNCTION:  init_socket
==============================================================================*/
/**
  Initialize socket resources.
*/
void init_socket() {
  sck = new DataUnSocket(paramsStruct.usf_socket_path, on_socket_connect);
  timespec sem_timeout;

  if (0 > sem_init(&sem,
                   0,
                   0))
  {
    LOGE("%s: semaphore init failed",
         __FUNCTION__);
    sw_calib_exit(EXIT_FAILURE);;
  }

  sck->start();

  clock_gettime (CLOCK_REALTIME, &sem_timeout);
  sem_timeout.tv_sec += SOCKET_ACCEPT_TIMEOUT_SECS;

  // Go to sleep until the application connects to the socket,
  // or sem_timeout passed
  if(-1 == sem_timedwait(&sem,&sem_timeout))
  {
     LOGE("%s: wait for socket client timeout",
         __FUNCTION__);
     sw_calib_exit(EXIT_FAILURE);
  }
}

/*==============================================================================
  FUNCTION:  init_transformation_properties
==============================================================================*/
/**
 * Init information required for using transformation matrix.
 */
void init_transformation_properties()
{
  plane_properties plane_props;
  plane_props.origin = paramsStruct.usf_on_screen_transform_origin;
  plane_props.point_end_x = paramsStruct.usf_on_screen_transform_end_X;
  plane_props.point_end_y = paramsStruct.usf_on_screen_transform_end_Y;
  plane_props.hover_max_range = 0;

  calc_transformation_properties(plane_props,
                                 Vector(0, 0, 0),
                                 transformation_props);

  create_transformation_matrix(plane_props,
                               transformation_props.transformation_matrix);

}

/*==============================================================================
  FUNCTION:  sw_calib_init
==============================================================================*/
/**
  Init sw_calib resources.
*/
void sw_calib_init()
{
  unsigned int sw_calib_workspace_size;
  int rc;
  int sw_calib_points_per_pen, sw_calib_max_pens;

  setup_signal_handling();

  initialize_ual_util();

  init_transformation_properties();

  if (DPENCALIB_SUCCESS != dpencalib_get_size(&sw_calib_workspace_size))
  {
    LOGE("%s: Failed to get workspace size for library",
         __FUNCTION__);
    sw_calib_exit(EXIT_FAILURE);
  }

  // Allocate memory for sw_calib algorithm.
  sw_calib_workspace = (void *)malloc(sw_calib_workspace_size);
  if (NULL == sw_calib_workspace)
  {
    LOGE("%s: Out of memory",
         __FUNCTION__);
    sw_calib_exit(EXIT_FAILURE);
  }

  if (DPENCALIB_SUCCESS != dpencalib_init(sw_calib_workspace))
  {
    LOGE("%s: Failed to init library",
         __FUNCTION__);
    sw_calib_exit(EXIT_FAILURE);
  }

  int major, minor, subminor;
  if (DPENCALIB_SUCCESS != dpencalib_version(&major, &minor, &subminor))
  {
    LOGE("%s: Failed to get library version",
         __FUNCTION__);
  }
  else
  {
    LOGD("%s: sw_calib library version: %d.%d.%d",
         __FUNCTION__,
         major,
         minor,
         subminor);
  }

  sw_calib_load_coeffs();

  LOGD("%s: Done initializing sw_calib library",
       __FUNCTION__);
}

/*==============================================================================
  FUNCTION:  write_frame_recording
==============================================================================*/
/**
  Writes frame recording and closes file when finished.
*/
void write_frame_recording(uint32_t *recorded_bytes_counter,
                           uint32_t bytes_write_to_file,
                           int group_data_size,
                           uint8_t *group_data)
{
  if (*recorded_bytes_counter < bytes_write_to_file)
  {
    // There are couple of frames in a group and the recorded units are frames.
    // Therefore, the recording could stop in the middle of the group and
    // the calculation of how many bytes left to record is needed.
    uint32_t bytes_from_group =
      (*recorded_bytes_counter + group_data_size <= bytes_write_to_file) ?
      group_data_size :
      (bytes_write_to_file - *recorded_bytes_counter);

    ual_util_frame_file_write(group_data,
                              sizeof(uint8_t),
                              bytes_from_group,
                              &paramsStruct,
                              frame_file);

    *recorded_bytes_counter += bytes_from_group;
    if (*recorded_bytes_counter >= bytes_write_to_file)
    {
      ual_util_close_and_sync_file(frame_file);
      frame_file = NULL;
    }
  }
}

/*==============================================================================
  FUNCTION:  transform_calibration_point
==============================================================================*/
/**
 * Transform the given point according to the initialized rotation matrix parameters.
 *
 * @param x The x axis of the point to transform
 * @param y The y axis of the point to transform
 *
 * @return Vector The transformed point
 */
static Vector transform_calibration_point(int x, int y)
{
  Vector point(x, y, 0);

  if (WORKING_ZONE != get_rotated_point(point,
                                        transformation_props))
  {
    LOGW("%s, Calibration point is outside working area",
         __FUNCTION__);
  }

  Vector transformed(point.get_element(X) / TSC_LOGICAL_MAX_X,
                     point.get_element(Y) / TSC_LOGICAL_MAX_Y,
                     0);
  return transformed;
}

/*==============================================================================
  FUNCTION:  process_frame
==============================================================================*/
/**
 * Processes the given frame and sends events to the socket reporting of issues
 * and of success.
 *
 * @param next_packet The packet to process
 *
 * @return sw_calib_result
 */
sw_calib_result process_frame(uint8_t *next_packet)
{
  static Vector prev_point = Vector(-1, -1, -1);
  static int prev_status = -1;
  // Epos packet sizes
  const int PACKET_LENGTH_SIZE = 4;
  const int PACKET_HEADER_SIZE = 4;
  const int PACKET_TYPE_SIZE = 4;
  const int PACKET_NUMBER_SIZE = 4;
  uint8_t *packet = next_packet + PACKET_HEADER_SIZE + PACKET_LENGTH_SIZE;

  dpencalib_frame frame;
  frame.channel = *((int *)packet);
  frame.timestamp = *((int *)(packet + PACKET_TYPE_SIZE));
  frame.samples = (short *)(packet + PACKET_TYPE_SIZE + PACKET_NUMBER_SIZE);

  int rc = dpencalib_process_frame(sw_calib_workspace,
                                   &frame);
  if (DPENCALIB_SUCCESS != rc)
  {
    LOGE("%s: dpencalib_process_frame failed: %d",
         __FUNCTION__,
         rc);
    sw_calib_exit(EXIT_FAILURE);
  }

  if (!paramsStruct.usf_sw_calib_is_tester_mode) {
      struct dpencalib_status status;
      rc = dpencalib_get_status(sw_calib_workspace,
                                &status);
      if (DPENCALIB_SUCCESS != rc)
      {
        LOGE("%s: dpencalib_get_status failed: %d",
             __FUNCTION__,
             rc);
        sw_calib_exit(EXIT_FAILURE);
      }

      if (status.point_number == -1)
      { // No more frames are needed, finished calibration
        return SW_CALIB_SUCCESS;
      }

      Vector transformed = transform_calibration_point(status.x, status.y);

      const int CALIBRATION_POINT_SCALING_FACTOR = 1000;
      transformed.vector_mult_by_scalar(CALIBRATION_POINT_SCALING_FACTOR);
      if (prev_point.get_element(X) != transformed.get_element(X) ||
          prev_point.get_element(Y) != transformed.get_element(Y) ||
          prev_status != status.status)
      {
        LOGE("%s: Sending point status: %d channel: %d point (%lf,%lf) point num:%d",
             __FUNCTION__,
             status.status,
             status.channel,
             transformed.get_element(X) * 25.7 / CALIBRATION_POINT_SCALING_FACTOR,
             transformed.get_element(Y) * 14.4 / CALIBRATION_POINT_SCALING_FACTOR,
             status.point_number);
        sck->send_sw_calib_event(status.status, transformed.get_element(X), transformed.get_element(Y));
        prev_point = transformed;
        prev_status = status.status;
      }
  }

  return SW_CALIB_CALIBRATING;
}

/*==============================================================================
  FUNCTION:  send_tester_statistics
==============================================================================*/
/**
  Sends statistics over the socket to the tester application
*/
void send_tester_statistics() {
    static double last_send_time = 0;

    if (last_send_time > get_current_time() - TESTER_SLEEP_TIME_MSECS) {
        return;
    }
    last_send_time = get_current_time();

    struct dpencalib_noise statistics_container;

    for (int mic_num = 0; mic_num < paramsStruct.usf_tx_port_count; mic_num++) {
        int res;
        if (DPENCALIB_SUCCESS != (res = dpencalib_noise_statistics(sw_calib_workspace, &statistics_container, mic_num))) {
            LOGE("%s: dpencalib_noise_statistics failed for mic %d with status %d",
                 __FUNCTION__,
                 mic_num,
                 res);
        }

        sck->send_sw_calib_tester_event(TESTER_MSG_TYPE_MEASUREMENT_POWER, mic_num, statistics_container.power_mean);
        sck->send_sw_calib_tester_event(TESTER_MSG_TYPE_MEASUREMENT_QUALITY, mic_num, statistics_container.peak2rms_mean[0]);
        if (statistics_container.spur_frequency_mean != 0 &&
            statistics_container.spur_power_mean > SPUR_POWER_MEAN_INTERFERENCE_THRESHOLD) {
            sck->send_sw_calib_tester_event(TESTER_MSG_TYPE_INTERFERENCE, mic_num, 0);
        }
    }
}

/*==============================================================================
  FUNCTION:  calibrate
==============================================================================*/
/**
  Processes frames and tries to calibrate the pen.
*/
static sw_calib_result calibrate()
{
  int rc;
  static uint32_t recorded_bytes_counter = 0;
  // Size of a single packet (samples from a single mic)
  int packet_size_in_bytes = paramsStruct.usf_tx_port_data_size *
                             (paramsStruct.usf_tx_sample_width / BYTE_WIDTH);
  // Size of a frame (including the frame header).
  // Every frame consists of packet from all activated mics.
  int frame_size_in_bytes = (packet_size_in_bytes *
                             paramsStruct.usf_tx_port_count) +
                             paramsStruct.usf_tx_frame_hdr_size;
  uint32_t bytes_write_to_file = paramsStruct.usf_frame_count *
                                 frame_size_in_bytes;

  ual_data_type data;
  usf_event_type event;

  rc = ual_read(&data,
                &event,
                0);
  if (true != rc)
  {
    LOGE("%s: error in ual_read, returned: %d",
         __FUNCTION__,
         rc);
    sw_calib_exit(EXIT_FAILURE);
  }

  int num_regions = sizeof(data.region) /
                    sizeof(ual_data_region_type);

  for (int r = 0; r < num_regions &&
                  0 < data.region[r].data_buf_size; ++r)
  {
    uint8_t *next_packet = data.region[r].data_buf;
    int num_of_buffers = data.region[r].data_buf_size /
                         paramsStruct.usf_tx_buf_size;

    uint8_t *group_data = data.region[r].data_buf;

    for (int b = 0; b < num_of_buffers; ++b)
    {
      int num_of_frames = paramsStruct.usf_tx_buf_size /
                          frame_size_in_bytes;
      uint32_t group_data_size = num_of_frames * frame_size_in_bytes;
      if (group_data_size > data.region[r].data_buf_size)
      {
        LOGE("%s: Group data size is not smaller than number of bytes received from DSP",
             __FUNCTION__);
        sw_calib_exit(EXIT_FAILURE);
      }

      next_packet = group_data;

      // Recording
      write_frame_recording(&recorded_bytes_counter,
                            bytes_write_to_file,
                            group_data_size,
                            group_data);
      // Number of frames depends on grouping factor
      for (int f = 0; f < num_of_frames; ++f)
      {
        next_packet += paramsStruct.usf_tx_frame_hdr_size;

        for (int m = 0; m < paramsStruct.usf_tx_port_count; ++m)
        {
          if (SW_CALIB_SUCCESS == process_frame(next_packet))
          {
            return SW_CALIB_SUCCESS;
          }
          next_packet += packet_size_in_bytes;
        } // Mics
        if (paramsStruct.usf_sw_calib_is_tester_mode) {
            send_tester_statistics();
        }
      } // Frames

      group_data += paramsStruct.usf_tx_buf_size;

    } // Buffers
  } // Regions

  return SW_CALIB_CALIBRATING;
}

/*==============================================================================
  FUNCTION:  write_calibration_file
==============================================================================*/
/**
 * Get calibration file from the library and write it to the disk.
 *
 * @return int 0 for success
 *             -1 for failure to write calibration file
 */
int write_calibration_file()
{
  // Get size of calibration packet
  int calib_size = 0;
  dpencalib_get_calib_packet(sw_calib_workspace,
                             NULL,
                             &calib_size);
  void *calib_packet = malloc(calib_size);
  if (NULL == calib_packet)
  {
    LOGE("%s: Out of memory",
         __FUNCTION__);
    return -1;
  }
  // Get calibration packet
  int ret = dpencalib_get_calib_packet(sw_calib_workspace,
                                       calib_packet,
                                       &calib_size);
  if (ret != DPENCALIB_SUCCESS)
  {
    LOGD("%s: Failed to get calib packet, ret: %d",
       __FUNCTION__,
         ret);
    free(calib_packet);
    return -1;
  }
  if (0 != ual_util_write_file(paramsStruct.usf_sw_calib_calibration_file[SW_CALIB_UNIT_FILE].path,
                               calib_packet,
                               calib_size,
                               sizeof(uint8_t)))
  {
    free(calib_packet);
    return -1;
  }
  free(calib_packet);
  LOGD("%s: Finished writing unit calib file of size: %d",
       __FUNCTION__,
       calib_size);
  return 0;
}

/*==============================================================================
  FUNCTION:  fill_array_from_string
==============================================================================*/
/**
  Fills the "to" array from the given string "from" with format "[<double0>,<double1>,...]"
*/
void fill_array_from_string(double* to, const char* from, int len) {
    char *endptr = NULL;
    int port_ind;

    for (port_ind = 0; port_ind < len; ++port_ind) {
        double val = strtod(from, &endptr);
        if ((ERANGE == val) ||
            (endptr == from))
        {
          LOGW("%s: Wrong constants string [%s]",
               __FUNCTION__,
               from);
          return;
        }
        to[port_ind] = val;
        if ('\0' == *endptr)
        {
          ++port_ind; // number of the ports
          break;
        }
        else
        {
          from = endptr+1;
        }
    }

    if (port_ind != len) {
        LOGW("%s: Port count does not match to number of "
             "ports given in usf_tx_ports."
             "port_cnt=%d, ret from scnf of usf_tx_ports=%d",
             __FUNCTION__,
             len,
             port_ind);
    }
}

/*==============================================================================
  FUNCTION:  send_constants
==============================================================================*/
/**
  Send the constants (min scale value, max scale value, threshold) for each microphone
  and each one of the two charts (power and quality)
*/
void send_constants() {
    char* tester_constants[] = {
                                 paramsStruct.usf_tester_power_min_scales,
                                 paramsStruct.usf_tester_power_max_scales,
                                 paramsStruct.usf_tester_power_thresholds,
                                 paramsStruct.usf_tester_quality_min_scales,
                                 paramsStruct.usf_tester_quality_max_scales,
                                 paramsStruct.usf_tester_quality_thresholds
                               };
    tester_msg_t constants_tester_msg_types[] = {
                                       TESTER_MSG_TYPE_MIN_SCALE_POWER,
                                       TESTER_MSG_TYPE_MAX_SCALE_POWER,
                                       TESTER_MSG_TYPE_THRESHOLD_POWER,
                                       TESTER_MSG_TYPE_MIN_SCALE_QUALITY,
                                       TESTER_MSG_TYPE_MAX_SCALE_QUALITY,
                                       TESTER_MSG_TYPE_THRESHOLD_QUALITY,
                                     };

    for (int i = 0; i < TESTER_MSG_TYPE_NUM_CONSTANTS; ++i) {
         double tester_constants_for_mic[paramsStruct.usf_tx_port_count];
         fill_array_from_string(tester_constants_for_mic, tester_constants[i], paramsStruct.usf_tx_port_count);

         for (int mic_num = 0; mic_num < paramsStruct.usf_tx_port_count; ++mic_num) {
             sck->send_sw_calib_tester_event(constants_tester_msg_types[i], mic_num, tester_constants_for_mic[mic_num]);
         }
    }
}

/*==============================================================================
  FUNCTION:  main
==============================================================================*/
/**
  Main function of the sw_calib daemon. Handle all the sw_calib operations.
*/
int main()
{
  LOGI("%s: sw_calib start",
       __FUNCTION__);

  int rc;
  sw_calib_result res = SW_CALIB_FAILURE;

  sw_calib_init();

  init_socket();

  // Open frame file from cfg file
  if (0 < paramsStruct.usf_frame_count)
  {
    frame_file = ual_util_get_frame_file(&paramsStruct,
                                         (char *)FRAME_FILE_DIR_PATH);
    if (NULL == frame_file)
    {
      LOGE("%s: ual_util_get_frame_file failed",
           __FUNCTION__);
      sw_calib_exit(EXIT_FAILURE);
    }
  }

  if (paramsStruct.usf_sw_calib_is_tester_mode) {
      LOGI("%s: sw_calib starting tester mode",
       __FUNCTION__);
      send_constants();
  }

  double timeout_msecs = paramsStruct.usf_sw_calib_timeout_msec;
  double start_time = get_current_time();

  while (daemon_run &&
         (paramsStruct.usf_sw_calib_is_tester_mode ||
          get_current_time() < start_time + timeout_msecs))
  {
    if (SW_CALIB_SUCCESS == calibrate())
    {
      LOGD("%s: Finished calibrating successfuly",
           __FUNCTION__);
      break;
    }
  }

  if (get_current_time() >= start_time + timeout_msecs)
  {
    LOGE("%s: Couldn't calibrate pen - Timeout exceeded",
         __FUNCTION__);
    sck->send_sw_calib_event(-1, 0, 0);
  }
  else
  {
    if (0 != write_calibration_file())
    {
      sck->send_sw_calib_event(-1, 0, 0);
    }
    else
    {
      sck->send_sw_calib_event(DPCSTATUS_OK, -1, -1);
    }
  }

  sw_calib_exit(EXIT_SUCCESS);
}
