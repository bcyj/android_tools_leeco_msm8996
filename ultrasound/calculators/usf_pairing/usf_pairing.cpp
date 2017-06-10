/*===========================================================================
                           usf_pairing.cpp

DESCRIPTION: Implementation of the pen pairing daemon.

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_pairing"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "eposexports.h"
#include "usf_log.h"
#include <stdlib.h>
#include <ual_util.h>
#include <ual_util_frame_file.h>
#include <cutils/properties.h>
#include "pplexports.h"
#include <usf_unix_domain_socket.h>
#include <dirent.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define LINK_CFG_FILE_LOCATION "/data/usf/pairing/usf_pairing.cfg"
#define CLIENT_NAME "pairing"
#define FRAME_FILE_DIR_PATH "/data/usf/pairing/rec/"

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Ptr to the cfg file. The ptr is global bacause we want to
  close the file before exit in the function tester_exit.
  Also, the needs to be closed from signal_handler so this
  has to be global.
*/
static FILE *cfgFile = NULL;

/**
 * The name of the file containg the pid of the daemon.
 */
static const char* PID_FILE_NAME = "usf_pairing.pid";

/**
 * Ptr to the frame file recording.
 */
FILE* frame_file = NULL;

/**
 * The prefix of series calib file name.
 */
static const char* SERIES_CALIB_FILE_NAME_PREFIX = "series_calib%d_";

/**
 * Configuration parameters for the daemon.
 */
static us_all_info paramsStruct;

/**
  Pairing calculator version.
*/
static const char *CLIENT_VERSION = "1.0";

/**
  The workspace for the pairing library.
 */
static void *pairing_workspace = NULL;

static const int MAX_DSP_VERSION_STRING_LEN = 256;

static const int MAX_DSP_VERSION_SIZE = 4;

static const time_t SOCKET_ACCEPT_TIMEOUT_SECS = 10LL;

static const int MAX_CYCLE_NUM = 2;

static const int MAX_TIME_SERIES_VERIFICATION_TRUE_MSECS = 16000;

static const int MAX_TIME_SERIES_VERIFICATION_FALSE_MSECS = 1000;

static const int MIN_NUMBER_OF_SERIES_FILES_FOR_TIMEOUT = 20;

/**
 * The semaphore on which the main thread waits until the client
 * connects to the socket
 */
static sem_t sem;

/**
 * The socket through which the client gets information
 * about current pairing status.
 */
static DataUnSocket *sck;

/**
 * The number of pen series files found.
 */
static int s_num_series = 0;

/**
  The daemon running control
*/
static volatile bool daemon_run = true;

/*----------------------------------------------------------------------------
  Typedefs
----------------------------------------------------------------------------*/
typedef struct {
  // Pointer to memory where data resides
  void *calib_packet;
  uint32_t calib_packet_len;
} pairing_calib_packet;

typedef enum {
  PAIRING_NOT_FOUND,
  PAIRING_STARTED,
  PAIRING_REPOSITION,
  PAIRING_BLOCKED,
  PAIRING_SUCCESS,
  PAIRING_FAILURE,
  PAIRING_TIMEOUT
} pairing_result;

/*----------------------------------------------------------------------------
  Function definitions
----------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  get_current_time
==============================================================================*/
/**
  returns the current time.
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
  FUNCTION:  pairing_exit
==============================================================================*/
/**
  Perform clean exit of the pairing daemon.
*/
int pairing_exit (int status)
{
  // Time before call to ual_close - used to measure how much time it takes
  // to stop ual
  double timeBeforeUalClose = get_current_time();

  bool error_state = (status != EXIT_SUCCESS);
  int rc = ual_close(error_state);
  if (1 != rc)
  {
    LOGW("%s: ual_close: rc=%d;",
         __FUNCTION__,
         rc);
  }

  // Time after ual_close
  double closeUalDuration =  get_current_time() - timeBeforeUalClose;

  LOGW("%s: Duration of ual_close() is: %f msec",
       __FUNCTION__,
       closeUalDuration);

  if (NULL != pairing_workspace)
  {
    ReleasePPL(pairing_workspace,
               NULL);
  }

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

  if (NULL != pairing_workspace)
  {
    free(pairing_workspace);
    pairing_workspace = NULL;
  }

  LOGI("%s: Pairing end. status=%d",
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
    pairing_exit(EXIT_FAILURE);
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
    pairing_exit(EXIT_FAILURE);
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
    pairing_exit(EXIT_FAILURE);
  }

  ual_util_print_US_version(CLIENT_NAME,
                            CLIENT_VERSION);
}

/*==============================================================================
  FUNCTION:  pairing_load_single_coeffs
==============================================================================*/
/**
  Loads a single calibration packet using the LoadSeries method
  from the pairing library.
*/
/**
  Loads a single calibration packet using the LoadSeries method
  from the pairing library.

  @param calib_file_path - The calibration file path
  @param is_series - True if the file is a pen series packet,
                     False if other type of calibration packet
 */
static void pairing_load_single_coeffs(const char* calib_file_path,
                                       bool is_series)
{
   pairing_calib_packet packet;
   packet.calib_packet =
        ual_util_malloc_read(calib_file_path,
                             packet.calib_packet_len);

   if (NULL == packet.calib_packet ||
       0 == packet.calib_packet_len)
   {
     LOGE("%s: %s - File not found or empty.",
          __FUNCTION__,
          calib_file_path);
     pairing_exit(EXIT_FAILURE);
   }

   int rc;

   if (is_series)
   {
     rc = LoadSeries(pairing_workspace,
                     packet.calib_packet,
                     packet.calib_packet_len / sizeof(int32_t));
   }
   else
   {
     rc = LoadPPLCoeffs(pairing_workspace,
                        packet.calib_packet,
                        packet.calib_packet_len / sizeof(int32_t));
   }

   if (rc)
   {
     LOGE("%s: Load coeffs failed for calib file: %s ret: %d",
          __FUNCTION__,
          calib_file_path,
          rc);
     pairing_exit(EXIT_FAILURE);
   }
}

/*==============================================================================
  FUNCTION:  series_alphasort
==============================================================================*/
/**
  Compare two series file names.

  @return -1, 0, or 1 if the first series number is less than, equal to
          or greater than the second series number
 */
static int series_alphasort(const struct dirent **a,
                            const struct dirent **b)
{
  int ser1 = 0, ser2 = 0;
  sscanf((*a)->d_name, SERIES_CALIB_FILE_NAME_PREFIX, &ser1);
  sscanf((*b)->d_name, SERIES_CALIB_FILE_NAME_PREFIX, &ser2);
  if (ser1 < ser2)
  {
    return -1;
  }
  if (ser2 < ser1)
  {
    return 1;
  }
  return 0;
}

/*==============================================================================
  FUNCTION:  pairing_load_coeffs
==============================================================================*/
/**
  Loads the calibrations files and calls the LoadCoeffs method
  from the pairing library.
  Updates the nubmer of pen series files.
*/
static void pairing_load_coeffs()
{
  struct dirent** series_files = NULL;
  char series_calib_path[FILE_PATH_MAX_LEN];
  int num_series = 0;

  // Load default calib files
  for (int i = 0; i < PAIRING_DEFAULT_CALIB_FILE_COUNT; ++i)
  {
    pairing_load_single_coeffs(paramsStruct.usf_pairing_default_calib_files[i],
                               false);
  }

  // Scanning the directory of the pen series
  num_series = scandir(paramsStruct.usf_pairing_calib_files_path_prefix,
                &series_files,
                0,
                series_alphasort);
  if (0 > num_series)
  {
    LOGE("%s: Failed to open series directory %s",
        __FUNCTION__,
        paramsStruct.usf_pairing_calib_files_path_prefix);
    pairing_exit(EXIT_FAILURE);
  }

  s_num_series = num_series;

  // Load series files
  for(int i = 0 ; i < num_series; ++i)
  {
    // ignore current and parent directories
    if (!strcmp(series_files[i]->d_name, ".") ||
        !strcmp(series_files[i]->d_name, ".."))
    {
      s_num_series--;
      continue;
    }
    LOGE("%s: load %s",
         __FUNCTION__,
         series_files[i]->d_name);

    snprintf(series_calib_path,
             FILE_PATH_MAX_LEN,
             "%s/%s",
             paramsStruct.usf_pairing_calib_files_path_prefix,
             series_files[i]->d_name);

    pairing_load_single_coeffs(series_calib_path,
                               true);
    free(series_files[i]);
  }
  free(series_files);
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
    pairing_exit(EXIT_FAILURE);
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
     pairing_exit(EXIT_FAILURE);
  }
}

/*==============================================================================
  FUNCTION:  pairing_init
==============================================================================*/
/**
  Init pairing resources.
*/
void pairing_init()
{
  char szVersion[MAX_DSP_VERSION_STRING_LEN] = {0};
  unsigned char nVersionNum[MAX_DSP_VERSION_SIZE] = {0};
  int rc;
  int pairing_points_per_pen, pairing_max_pens, pairing_workspace_size;
  EPoint *out_points;

  setup_signal_handling();

  initialize_ual_util();

  GetPPLAllocationSizes(&pairing_points_per_pen,
                        &pairing_max_pens,
                        MAX_SERIES_PACKET_SIZE,
                        MAX_NUM_OF_SERIES_PACKETS,
                        &pairing_workspace_size);

  // Allocate memory for pairing algorithm.

  pairing_workspace = (void *)malloc(pairing_workspace_size);

  out_points = (EPoint *)malloc(pairing_points_per_pen *
                                pairing_max_pens *
                                sizeof(EPoint));

  if ((NULL == pairing_workspace) ||
      (NULL == out_points))
  {
    LOGE("%s: Out of memory",
         __FUNCTION__);
    pairing_exit(EXIT_FAILURE);
  }

  GetPPLVersion(szVersion,
                nVersionNum);

  if (0 == strcmp(szVersion, "stub_version"))
  {
    LOGW("%s: Stub init.",
         __FUNCTION__);
  }

  LOGD("%s: Resetting pairing library",
       __FUNCTION__);

  if ((rc = ResetPPL(out_points,
                     pairing_workspace)))
  {
    LOGE("%s: Could not init pairing library, ret: %d",
         __FUNCTION__,
         rc);
    pairing_exit(EXIT_FAILURE);
  }

  pairing_load_coeffs();

  LOGD("%s: Done initializing pairing library",
       __FUNCTION__);
}

/*==============================================================================
  FUNCTION:  ppl_result_to_pairing_result
==============================================================================*/
/**
  Gets a find_series_result and turns it into a pairing_result
*/
static pairing_result ppl_result_to_pairing_result(int32_t find_ser_res,
                                                   int32_t ppl_res)
{
  static int cycle_num = 0;

  if ((ERR_INIT == find_ser_res) ||
      (ERR_INVALID_SER_PACKET == find_ser_res))
  {
    return PAIRING_FAILURE;
  }

  if (1 == ppl_res)
  {
    return PAIRING_SUCCESS;
  }

  // Check the "end of check cycle error codes" which
  // are reported only after all series were checked
  if ((ERR_FATAL_DUPL_POS_SERIES == find_ser_res) ||
      (ERR_FULL_CYCLE_NO_SERIES == find_ser_res) ||
      (ERR_FULL_CYCLE_BAD_VALID == find_ser_res))
  {
    cycle_num++;
    if (MAX_CYCLE_NUM <= cycle_num)
    {
      return PAIRING_TIMEOUT;
    }
  }

  return PAIRING_STARTED;
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
      bytes_write_to_file - *recorded_bytes_counter;

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
  FUNCTION:  find_series
==============================================================================*/
/**
  Tries to get a pen ID from ultrasound samples.
  Returns the relevant status.
*/
static pairing_result find_series()
{
  int rc;
  int32_t find_ser_res;
  static int32_t last_series = 1;
  static uint32_t recorded_bytes_counter = 0;
  pairing_result ret = PAIRING_NOT_FOUND;
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
  PPLFeedbackStruct fb_info;
  static pairing_result prev_sent_event = PAIRING_NOT_FOUND;
  static int32_t prev_ppl_res = ERR_NOERROR;

  fb_info.Result=-1;

  rc = ual_read(&data,
                &event,
                0);
  if (true != rc)
  {
    LOGE("%s: error in ual_read, returned: %d",
         __FUNCTION__,
         rc);
    pairing_exit(EXIT_FAILURE);
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
        pairing_exit(EXIT_FAILURE);
      }
      next_packet = group_data;

      // Recording
      write_frame_recording(&recorded_bytes_counter,
                            bytes_write_to_file,
                            group_data_size,
                            group_data);

      for (int f = 0; f < num_of_frames; ++f)
      {
        next_packet += paramsStruct.usf_tx_frame_hdr_size;

        for (int m = 0; m < paramsStruct.usf_tx_port_count; ++m)
        {
          int *packet = (int *)next_packet;

          find_ser_res = FindSeries(packet,
                                    pairing_workspace,
                                    &fb_info,
                                    paramsStruct.usf_pairing_circle_x,
                                    paramsStruct.usf_pairing_circle_y,
                                    paramsStruct.usf_pairing_circle_r);

          if ((last_series != fb_info.CurrentSeriesNumber) ||
              (prev_ppl_res != find_ser_res) ||
              (1 == fb_info.Result))
          {
            LOGD("%s: Validation: %d, Result: %d, TX0 Series: %d, Return value: %d",
                 __FUNCTION__,
                 fb_info.ConfidenceLevel,
                 fb_info.Result,
                 fb_info.CurrentSeriesNumber,
                 find_ser_res);

            if (1 == fb_info.Result)
            {
              LOGD("%s: Series found: Tx0: %d, Tx1: %d",
                 __FUNCTION__,
                 fb_info.CurrentSeriesNumber,
                 PRIME_NUM - fb_info.CurrentSeriesNumber);
            }

            last_series = fb_info.CurrentSeriesNumber;
            prev_ppl_res = find_ser_res;
          }

          pairing_result res = ppl_result_to_pairing_result(find_ser_res,
                                                            fb_info.Result);
          if ((MIN_NUMBER_OF_SERIES_FILES_FOR_TIMEOUT > s_num_series) &&
              (PAIRING_TIMEOUT == res))
          {
            // In case there is a small number of series files,
            // the timeout of two cycles is too small
            res = PAIRING_NOT_FOUND;
          }

          if (PAIRING_NOT_FOUND != res && prev_sent_event != res)
          {
            prev_sent_event = res;
            int series_number = (PAIRING_SUCCESS == res) ? fb_info.FinalSeriesNumber : -1;
            sck->send_pairing_event((int) res, series_number);
            LOGD("%s: Sent status %d to client",
                 __FUNCTION__,
                 (int) res);

            if (PAIRING_STARTED != res) // Return on success or error.
            {
              return res;
            }
            ret = PAIRING_STARTED;
          }

          next_packet += packet_size_in_bytes;
        } // Mics

      } // Frames

      group_data += paramsStruct.usf_tx_buf_size;

    } // Buffers
  } // Regions

  return ret;
}

/*==============================================================================
  FUNCTION:  main
==============================================================================*/
/**
  Main function of the pairing daemon. Handle all the pairing operations.
*/
int main()
{
  LOGI("%s: pairing start",
       __FUNCTION__);

  int rc;
  pairing_result res = PAIRING_FAILURE;

  pairing_init();

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
      pairing_exit(EXIT_FAILURE);
    }
  }

  static const double max_aquisition_time_msecs =
    ((s_num_series - 1) *
     MAX_TIME_SERIES_VERIFICATION_FALSE_MSECS) +
    MAX_TIME_SERIES_VERIFICATION_TRUE_MSECS;
  double timeout_msecs = max_aquisition_time_msecs;
  bool timer_started = false;
  double start_time = get_current_time();
  while (daemon_run &&
         0 < timeout_msecs)
  {
    res = find_series();

    if ((PAIRING_NOT_FOUND != res) &&    // Exit if there is a problem
        (PAIRING_STARTED != res))        // or if found pen
    {
      LOGD("%s: Exiting daemon on %s - error: %d",
           __FUNCTION__,
           (PAIRING_SUCCESS == res) ? "success" : "failure",
           (int) res);
      break;
    }

    if (timer_started) {
        timeout_msecs = max_aquisition_time_msecs -
                        (get_current_time() - start_time);
    }
    else if (PAIRING_STARTED == res)
    {
      start_time = get_current_time();
      timer_started = true;
    }
  }

  if (0 >= timeout_msecs)
  {
    sck->send_pairing_event(PAIRING_TIMEOUT, -1);
    LOGE("%s: Can't find pen series - Timeout exceeded",
         __FUNCTION__);
  }

  pairing_exit(EXIT_SUCCESS);
}
