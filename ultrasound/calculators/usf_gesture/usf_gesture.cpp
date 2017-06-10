/*===========================================================================
                           usf_gesture.cpp

DESCRIPTION: Implementation of the Gesture daemon.


INITIALIZATION AND SEQUENCING REQUIREMENTS:
  If not started through java app then make sure to have
  correct /data/usf/gesture/usf_gesture.cfg file linked to the wanted cfg file
  placed in /data/usf/gesture/cfg/.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_gesture"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <ual.h>
#include <cutils/properties.h>
#include <ual_util.h>
#include <GestureExports.h>
#include <stdlib.h>
#include <errno.h>
#include "ual_util_frame_file.h"
#include "us_adapter_factory.h"
#include <framework_adapter.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define LINK_CFG_FILE_LOCATION "/data/usf/gesture/usf_gesture.cfg"
#define USF_DSP_VER_FILE "/data/usf/gesture/usf_dsp_ver.txt"
#define FRAME_FILE_DIR_PATH "/data/usf/gesture/rec/"
#define PATTERN_DIR_PATH "/data/usf/gesture/pattern/"
#define BUFFER_SIZE 500
#define US_MAX_EVENTS 20
#define STATISTIC_PRINT_INTERVAL 500
#define SOCKET_PROBLEM_MSG_INTERVAL 50
#define EXIT_SIGTERM 3
#define NUM_KEYS 4
#define OPEN_RETRIES 10
#define SLEEP_TIME   0.1


const double ECHO_KEY_PRESS_DURATION = 200; // msec

enum gesture_event_dest
{
  DEST_UAL = 0x01,
  DEST_SOCKET = 0x02,
};

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
  gestureParams holds information needed for Gesture calculation.
*/
typedef struct
{
  signed char       *m_gesture_workspace;
  usf_event_type    m_events[US_MAX_EVENTS]; // Array of struct from sys/stat.h
  bool              m_send_points_to_ual; // For future use
  bool              m_send_points_to_socket;
  int               m_nNextFrameSeqNum;
  int               m_socket_sending_prob;
  uint8_t*          m_pPattern;
  int               m_patternSize;
  int               m_keys[NUM_KEYS];
  int               m_nLastEventSeqNum;
  bool              m_stub;
} GestureParams;

/**
  GestureStats holds information about statistics from
  usf_gesture running.
*/
typedef struct
{
  int   m_nPointsCalculated;
  int   m_nTotalFrames;
  int   m_nLostFrames;
  int   m_nOutOfOrderErrors;
} GestureStats;

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Ptr to the cfg file. The ptr is global bacause we want to
  close the file before exit in the function gesture_exit.
  This function is called also from the signal_handler function
  which doesn't know the cfg file.
*/
static FILE *cfgFile = NULL;


/**
  gestureParams will hold all the information needed for Gesture
  calculation.
*/
static GestureParams gestureParams;


/**
  gestureStats will hold all the statistics needed.
*/
static GestureStats gestureStats;

/**
  Place of "sequence number" field in the US data frame
*/
static const uint16_t ECHO_FRAME_SEQNUM_OFFSET = 8; // bytes

/**
  calculated events counter (upon 1 call ual_read() )
*/
static int s_eventCounter = 0;

/**
  Gesture calculator name
*/
static const char* CLIENT_NAME =  "gesture";

/**
  Gesture calculator version
*/
static const char* CLIENT_VERSION = "2.0";

/**
 * The name of the file containg the pid of the daemon
 */
static const char* PID_FILE_NAME = "usf_gesture.pid";

/**
  The daemon running control
*/
static volatile bool sb_run = true;

/**
  The daemon running control.
*/
static volatile bool daemon_run = true;

/**
  Pointer to the gesture framework adapter
 */
static FrameworkAdapter *adapter;
/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  gesture_free_resources
==============================================================================*/
/**
  Clean daemon parameters.
*/
void gesture_free_resources (bool error_state)
{
  QcGestureAlgorithmTerminate(gestureParams.m_gesture_workspace);

  int rc = ual_close(error_state);
  LOGD("%s: ual_close: rc=%d;",
       __FUNCTION__,
       rc);

  if (NULL != gestureParams.m_gesture_workspace)
  {
    free(gestureParams.m_gesture_workspace);
    gestureParams.m_gesture_workspace = NULL;
  }

  if (NULL != gestureParams.m_pPattern)
  {
    free(gestureParams.m_pPattern);
    gestureParams.m_pPattern = NULL;
  }
}

/*==============================================================================
  FUNCTION:  gesture_exit
==============================================================================*/
/**
  Perform clean exit of the daemon.
*/
int gesture_exit (int status)
{
  bool error_state = (status != EXIT_SUCCESS);

  if (NULL != adapter)
  {
    if (error_state)
    {
      adapter->on_error();
    }
    adapter->disconnect();

    destroy_adapter();
  }

  gesture_free_resources(error_state);

  if (NULL != cfgFile)
  {
    fclose(cfgFile);
    cfgFile = NULL;
  }

  int ret = ual_util_remove_declare_pid(PID_FILE_NAME);
  if (0 != ret)
  {
    LOGW("%s: Removing pid file failed",
         __FUNCTION__);
  }

  LOGI("%s: Gesture end. status=%d",
       __FUNCTION__,
       status);

  // Must update flag, so that init would not restart the daemon.
  ret = property_set("ctl.stop",
                     "usf_gesture");

  if (0 != ret)
  {
    LOGW("%s: property_set failed",
         __FUNCTION__);
  }

  _exit(status);
}

/*==============================================================================
  FUNCTION:  gesture_params_init
==============================================================================*/
/**
  Init gestureParam struct.
*/
void gesture_params_init (us_all_info *paramsStruct)
{
  uint32_t port;
  char *temp = NULL, *ip = NULL;
  int ret;

  gestureParams.m_gesture_workspace = NULL;

  gestureParams.m_patternSize = paramsStruct->usf_rx_pattern_size *
                                paramsStruct->usf_rx_sample_width/BYTE_WIDTH;

  gestureParams.m_pPattern = (uint8_t *) malloc(gestureParams.m_patternSize);
  if (NULL == gestureParams.m_pPattern)
  {
    LOGE("%s: Failed to allocate %d bytes",
         __FUNCTION__,
         paramsStruct->usf_rx_pattern_size *
         sizeof(paramsStruct->usf_rx_sample_width));
    gesture_exit(EXIT_FAILURE);
  }

  if (paramsStruct->usf_gesture_event_dest & DEST_UAL)
  {
    gestureParams.m_send_points_to_ual = true;
  }
  else
  {
    gestureParams.m_send_points_to_ual = false;
  }

  if (paramsStruct->usf_gesture_event_dest & DEST_SOCKET)
  {
    gestureParams.m_send_points_to_socket = true;
  }
  else
  {
    gestureParams.m_send_points_to_socket = false;
  }

  gestureParams.m_nNextFrameSeqNum = -1;

  gestureParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;

  gestureStats.m_nPointsCalculated = 0;
  gestureStats.m_nTotalFrames = 0;
  gestureStats.m_nLostFrames = 0;
  gestureStats.m_nOutOfOrderErrors = 0;

  ret = sscanf(paramsStruct->usf_gesture_keys,
               "%d,%d,%d,%d",
               &gestureParams.m_keys[0],
               &gestureParams.m_keys[1],
               &gestureParams.m_keys[2],
               &gestureParams.m_keys[3]);
  if (4 != ret)
  {
    LOGE("%s: usf_gesture_keys=4, ret from scnf=%d",
         __FUNCTION__,
         ret);
    gesture_exit(EXIT_FAILURE);
  }

  gestureParams.m_nLastEventSeqNum = 0;
  gestureParams.m_stub = false;

  LOGI("%s: gesture_params_init finished.",
       __FUNCTION__);
}

/*==============================================================================
  FUNCTION:  gesture_init
==============================================================================*/
/**
  Init Gesture resources.
*/
void gesture_init (us_all_info *paramsStruct)
{
  // Allocate memory for Gesture algorithm.
  int gesture_workspace_size = 0;
  int mic, spkr, dim;
  float mics_info[US_FORM_FACTOR_CONFIG_MAX_MICS][COORDINATES_DIM] = {{0}};
  float spkrs_info[US_FORM_FACTOR_CONFIG_MAX_SPEAKERS][COORDINATES_DIM] = {{0}};
  float mics_coords[COORDINATES_DIM][US_FORM_FACTOR_CONFIG_MAX_MICS] = {{0}};
  float speaks_coord[COORDINATES_DIM][US_FORM_FACTOR_CONFIG_MAX_SPEAKERS] = {{0}};

  QcGestureAlgorithmGetSizes(&gesture_workspace_size);
  gestureParams.m_gesture_workspace =
    (signed char *)malloc(gesture_workspace_size * sizeof(signed char));
  if (NULL == gestureParams.m_gesture_workspace)
  {
    LOGE("%s: Failed to allocate %d bytes.",
         __FUNCTION__,
         gesture_workspace_size);
    gesture_exit(EXIT_FAILURE);
  }

  int mics_num = paramsStruct->usf_tx_port_count;
  for (mic = 0; mic < mics_num; mic++)
  {
    if (-1 == ual_util_get_mic_config (mic, mics_info[mic]))
    {
      LOGE("%s: get_mic_config for mic %d failed.",
           __FUNCTION__,
           mic);
      gesture_exit(EXIT_FAILURE);
    }
  }

  int spkr_num = paramsStruct->usf_rx_port_count;
  for (spkr = 0; spkr < spkr_num; spkr++)
  {
    if (-1 == ual_util_get_speaker_config (spkr, spkrs_info[spkr]))
    {
      LOGE("%s: ual_util_get_speaker_config for speaker %d failed.",
           __FUNCTION__,
           spkr);
      gesture_exit(EXIT_FAILURE);
    }
  }

  // Fiting mics and speakers info to the Gesture lib API
  for (dim = 0; dim < COORDINATES_DIM; dim++)
  {
    for (int mic = 0; mic < mics_num; mic++)
    {
      mics_coords[dim][mic] = mics_info[mic][dim];
    }
    for (int spkr = 0; spkr < spkr_num; spkr++)
    {
      speaks_coord[dim][spkr] = spkrs_info[spkr][dim];
    }
  }

  LOGD("mics_num=%d; Mic 1 = (%.5f, %.5f), Mic 2 = (%.5f, %.5f), Mic 3 = (%.5f, %.5f), "
       "spkr_num=%d; Spkr = (%.5f, %.5f)",
       mics_num,
       mics_coords[X_IND][0],
       mics_coords[Y_IND][0],
       mics_coords[X_IND][1],
       mics_coords[Y_IND][1],
       mics_coords[X_IND][2],
       mics_coords[Y_IND][2],
       spkr_num,
       speaks_coord[X_IND][0],
       speaks_coord[Y_IND][0]);

  int rc = QcGestureAlgorithmInit(gestureParams.m_gesture_workspace,
                                  mics_coords[0],
                                  mics_coords[1],
                                  mics_coords[2],
                                  speaks_coord[0],
                                  speaks_coord[1],
                                  speaks_coord[2],
                                  paramsStruct->usf_tx_port_data_size,
                                  paramsStruct->usf_rx_port_data_size);

  if (rc)
  {
    LOGE("%s: AlgorithmInit failed.",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  LOGI("%s: Gesture lib init completed.",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  add_event_key
============================================================================*/
/**
  Creates event and add it to the gestureParams.m_events[].
*/
void add_event_key (int key,
                    int nPressure)
{
  // fill in the usf_event_type struct
  if (US_MAX_EVENTS <= s_eventCounter)
  {
    LOGW("%s: No more events to send max counter=%d",
         __FUNCTION__,
         US_MAX_EVENTS);
    return;
  }

  usf_event_type *pEvent = &gestureParams.m_events[s_eventCounter];

  memset (pEvent, 0, sizeof (usf_event_type));

  pEvent->seq_num = gestureParams.m_nLastEventSeqNum ++;
  pEvent->timestamp = (uint32_t)(clock ());
  pEvent->event_type_ind = USF_KEYBOARD_EVENT_IND;
  pEvent->event_data.key_event.key = key;
  pEvent->event_data.key_event.key_state = nPressure;
  LOGD("%s: Added event [%d]: key[%d] nPressure[%d]",
       __FUNCTION__,
       s_eventCounter,
       key,
       nPressure);
  ++s_eventCounter;
}

/*==============================================================================
  FUNCTION:  check_adapter_status
==============================================================================*/
static int check_adapter_status()
{
  if (NULL != adapter)
  {
    switch (adapter->get_status())
    {
    case DEACTIVATE:
      sb_run = false;
      return 0;
    case DISCONNECT:
      return 1;
    case ACTIVATE:
      return 0;
    case SHUTDOWN:
      return 1;
    default:
      LOGE("%s: invalid adapter status",
           __FUNCTION__);
      return -1;
    }
  }
  return 0;
}

/*==============================================================================
  FUNCTION:  gesture_get_points
==============================================================================*/
/**
  Call QcGestureAlgorithmEngine() from Gesture lib.
  Returns eventCounter for number of points goes to UAL.
*/
int gesture_get_points(short *pPacket)
{
  int     rc = 1;
  int     gesture = 0;
  char    cOutputValid = 0;
  int     iPatternUpdate = 0;
  int     seqNum = 0;

  if (NULL != pPacket)
  {
    seqNum = *(pPacket + ECHO_FRAME_SEQNUM_OFFSET / sizeof(short));
  }

  QcGestureAlgorithmEngine(pPacket,
                           (short int *) gestureParams.m_pPattern,
                           &gesture,
                           seqNum,
                           &iPatternUpdate);



  if (gestureParams.m_stub)
  {
    return 0;
  }

  if (gesture != 0)
  {
    LOGD("%s: gesture[%d]; seqNum[%d]; iPatternUpdate[%d]",
         __FUNCTION__,
         gesture,
         seqNum,
         iPatternUpdate);
  }

  // If pPacket is NULL then we try to update pattern from Gesture lib
  // for the first time (and not from pattern file received from cfg file).
  if ((NULL == pPacket) && (1 != iPatternUpdate))
  {
    LOGE("%s: QcGestureAlgorithmEngine failed.",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  // Update pattern on runtime by Gesture lib.
  if (1 == iPatternUpdate)
  {
    if ((NULL == gestureParams.m_pPattern))
    {
      LOGE("%s: QcGestureAlgorithmEngine failed.",
           __FUNCTION__);
      gesture_exit(EXIT_FAILURE);
    }
    else
    {
      LOGD("%s: Update pattern from Gesture lib.",
           __FUNCTION__);

      // Pattern is transmitted only once. DSP transmits pattern in loop.
      rc = ual_write(gestureParams.m_pPattern,
                     gestureParams.m_patternSize);
      if (1 != rc)
      {
        LOGE("%s: ual_write failed.",
             __FUNCTION__);
        gesture_exit(EXIT_FAILURE);

      }
    }
  }

  if ((gesture > 0) &&
      (gesture <= (int)(sizeof(gestureParams.m_keys) /
                        sizeof(gestureParams.m_keys[0])) ) )
  {
    // There is a gesture.
    // Assumption: a key duration << time between 2 gestures.

    // Send start press to UAL
    if (gestureParams.m_send_points_to_ual)
    {
      add_event_key (gestureParams.m_keys[gesture-1],
                     1);
      add_event_key (gestureParams.m_keys[gesture-1],
                     0);
    }

    // Send start press to socket
    if (gestureParams.m_send_points_to_socket)
    {
      if (NULL != adapter)
      {
        int event = (adapter->get_event_mapping() == MAPPED) ?
                    gestureParams.m_keys[gesture-1] :
                    gesture;

        if (1 == adapter->send_event(event, EVENT_SOURCE_APSS, 0))
        {
          LOGE("%s: adapter send_event failed.",
               __FUNCTION__);
        }
      }
    }

    gestureStats.m_nPointsCalculated++;
  }

  if (0 > rc)
  {
    // If we got here there is some problem in sending gesture to socket.
    // The m_socket_sending_prob starts from SOCKET_PROBLEM_MSG_INTERVAL
    // and only when it gets to 0 a warning msg is shown to the user and
    // m_socket_sending_prob set to SOCKET_PROBLEM_MSG_INTERVAL again.
    gestureParams.m_socket_sending_prob--;
    if (0 == gestureParams.m_socket_sending_prob)
    {
      LOGW("%s: SendGesture() failed.",
           __FUNCTION__);
      gestureParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;
    }
  }
  else
  {
    gestureParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;
  }

  return s_eventCounter;
}

/*==============================================================================
  FUNCTION:  print_DSP_ver
==============================================================================*/
/**
  Print DSP version to file.
*/
void print_DSP_ver()
{
  char szVersion [256];
  uint32_t szVersionSize = sizeof(szVersion) - 1;
  FILE* fp = fopen (USF_DSP_VER_FILE,
                    "wt");
  if (fp == NULL)
  {
    LOGE("%s: Could not open %s - %s",
         __FUNCTION__,
         USF_DSP_VER_FILE,
         strerror(errno));
    gesture_exit(EXIT_FAILURE);
  }

  QcGestureAlgorithmGetVersion(szVersion,
                               (int*)&szVersionSize);
  if (szVersionSize > sizeof(szVersion) - 1)
  {
    LOGW("%s: Wrong version size (%d)",
         __FUNCTION__,
         szVersionSize);
  }
  else
  {
    szVersion[szVersionSize] = 0;
    fprintf (fp, "%s\n",
             szVersion);
  }

  if (strncmp(szVersion, STUB_VERSION, strlen(STUB_VERSION)) == 0)
  {
    gestureParams.m_stub = true;
  }

  fclose (fp);
}

/*==============================================================================
  FUNCTION:  signal_handler
==============================================================================*/
/**
  Perform clean exit after receive signal.
*/
void signal_handler (int sig)
{
  LOGD("%s: Received signal %d; sb_run=%d",
         __FUNCTION__, sig, sb_run);
  // All supportd signals cause the daemon exit
  sb_run = false;
  daemon_run = false;
  // Repeat exit request to wake-up blocked functions
  alarm(1);
}

/*==============================================================================
  FUNCTION:  main
==============================================================================*/
/**
  Main function of the Gesture daemon. Handle all the Gesture operations.
*/
int main (void)
{
  int ret, ind = 0, numPoints = 0, packetCounter = 0;
  int iPatternUpdate = 0;
  FILE *frameFile = NULL;
  static us_all_info paramsStruct;
  bool rc = false, frame_file_created = false;
  ual_data_type data;
  uint32_t frame_hdr_size_in_bytes;
  uint32_t packet_size_in_bytes, packets_in_frame, frame_size_in_bytes;

  LOGI("%s: Gesture start",
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

  if (false == ual_util_is_supported((char *)CLIENT_NAME))
  {
    LOGE("%s: Daemon is not supported",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

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
    gesture_exit(EXIT_FAILURE);
  }

  // Get suitable adapter
  if (0 != create_adapter(&adapter,
                          paramsStruct.usf_adapter_lib))
  {
    LOGE("%s: create_adapter failed",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  ual_util_print_US_version(CLIENT_NAME,
                            CLIENT_VERSION);

  // Build tx_transparent_data manually. In the future (hopefully),
  // this will be removed.
  ual_util_set_echo_tx_transparent_data(&paramsStruct);

  ual_util_set_tx_buf_size(&paramsStruct);

  // Build rx_transparent_data manually. In the future (hopefully),
  // this will be removed.
  ual_util_set_echo_rx_transparent_data(&paramsStruct);

  ual_util_set_rx_buf_size(&paramsStruct);

  while (daemon_run)
  {
    // Clear pending alarm signal that might disrupt this function work.  Alarm
    // signal is used to release daemons from blocking functions.
    alarm(0);

    int wait_status = (NULL != adapter) ? adapter->wait_n_update() : 0;
    switch (wait_status)
    {
    case ACTIVATE:
      sb_run = true;
      break;
    case SHUTDOWN:
      gesture_exit(EXIT_SUCCESS);
    case FAILURE:
    case INTERRUPT:
    default:
      gesture_exit(EXIT_FAILURE);
    }

    ual_cfg_type cfg;
    cfg.usf_dev_id = 1;
    cfg.ual_mode = static_cast<ual_work_mode_type>(paramsStruct.ual_work_mode);
    for (int i = 0; i < OPEN_RETRIES; i++)
    {
      rc = ual_open(&cfg);

      if (rc)
      {
        break;
      }
      if (i == OPEN_RETRIES)
      {
        LOGE("%s: ual_open failed.",
             __FUNCTION__);
        gesture_exit(EXIT_FAILURE);
      }

      LOGW("%s: Trying to open ual, %d...",
           __FUNCTION__,
           i + 1);
      sleep(SLEEP_TIME);
    }

    paramsStruct.usf_event_type = (paramsStruct.usf_gesture_event_dest & DEST_UAL) ?
                                  USF_KEYBOARD_EVENT :
                                  USF_NO_EVENT;

    if (ual_util_tx_config(&paramsStruct,
                           (char *)CLIENT_NAME))
    {
      LOGE("%s: ual_util_tx_config failed.",
           __FUNCTION__);
      gesture_exit(EXIT_FAILURE);
    }

    if (ual_util_rx_config(&paramsStruct,
                           (char* )CLIENT_NAME))
    {
      LOGE("%s: ual_util_rx_config failed.",
           __FUNCTION__);
      gesture_exit(EXIT_FAILURE);
    }

    gesture_params_init(&paramsStruct);

    gesture_init(&paramsStruct);

    print_DSP_ver();

    // Send pattern to UAL for the first time

    // Pattern is taken from file named in the cfg file
    if (0 != paramsStruct.usf_rx_pattern[0])
    {
      LOGW("%s: Update pattern from file.",
           __FUNCTION__);
      rc = !(ual_util_read_pattern(gestureParams.m_pPattern,
                                   &paramsStruct,
                                   (char *)PATTERN_DIR_PATH));
      if (1 != rc)
      {
        LOGE("%s: ual_util_read_pattern failed.",
             __FUNCTION__);
        gesture_exit(EXIT_FAILURE);
      }

      // Pattern is transmitted only once. DSP transmits pattern in loop.
      rc = ual_write(gestureParams.m_pPattern,
                     gestureParams.m_patternSize);
      if (1 != rc)
      {
        LOGE("%s: ual_write failed.",
             __FUNCTION__);
        gesture_exit(EXIT_FAILURE);
      }

    }
    // Pattern is taken from gesture lib
    else
    {
      LOGW("%s: Update pattern from lib.",
           __FUNCTION__);
      gesture_get_points(NULL);
    }

    if (0 >= paramsStruct.usf_frame_count)
    {
      LOGD("%s: usf_frame_count is %d. No record has made.",
           __FUNCTION__,
           paramsStruct.usf_frame_count);
    }

    frame_hdr_size_in_bytes =
      paramsStruct.usf_tx_frame_hdr_size;

    packet_size_in_bytes =
      paramsStruct.usf_tx_port_data_size *
      sizeof(paramsStruct.usf_tx_sample_width);

    packets_in_frame =
      paramsStruct.usf_tx_port_count;

    frame_size_in_bytes =
      packet_size_in_bytes * packets_in_frame +
      frame_hdr_size_in_bytes;

    uint32_t numOfBytes = 0;
    int num_of_regions = sizeof(data.region) / sizeof(ual_data_region_type);
    uint32_t bytesWriteToFile = paramsStruct.usf_frame_count *
                                frame_size_in_bytes;
    uint32_t bytestFromRegion = 0;

    // Must add daemon_run flag as well, in case signal is received
    // right before assigning sb_run = true in the above switch.
    while (sb_run &&
           daemon_run)
    {

      if (0 != check_adapter_status())
      {
        LOGE("%s: Framework requested termination",
             __FUNCTION__);
        gesture_exit(EXIT_FAILURE);
      }

      uint8_t* nextFrame = NULL;

      rc = ual_read(&data,
                    gestureParams.m_events,
                    s_eventCounter);
      if (!rc)
      {
        LOGE("%s: ual read failed",
             __FUNCTION__);
        // Breaking the while will make the daemon re-allocate resources
        break;
      }

      // frameFile is not yet allocated
      if (0 < paramsStruct.usf_frame_count &&
          NULL == frameFile &&
          false == frame_file_created)
      {
        frame_file_created = true;
        LOGD("%s, framefile is null then getting file",
             __FUNCTION__);
        // Open frame file from cfg file
        frameFile = ual_util_get_frame_file (&paramsStruct,
                                             (char *)FRAME_FILE_DIR_PATH);
        LOGD("%s, returning from getfile",
             __FUNCTION__);
        if (NULL == frameFile)
        {
          LOGE("%s: ual_util_get_frame_file failed",
               __FUNCTION__);
          gesture_exit(EXIT_FAILURE);
        }
      }


      s_eventCounter = 0;
      if (0 == data.region[0].data_buf_size)
      {
        continue;
      }

      // Underlay layer provides US data frames in buffers.
      // Each buffer includes one group of the frames.
      // A number of frames is defined by configurable group factor.
      int numberOfFrames = paramsStruct.usf_tx_buf_size / frame_size_in_bytes;
      int group_data_size = numberOfFrames * frame_size_in_bytes;

      for (int r = 0; r < num_of_regions; r++)
      {
        int num_of_groups = data.region[r].data_buf_size /
                            paramsStruct.usf_tx_buf_size;
        uint8_t *pGroupData = data.region[r].data_buf;
        for (int g = 0; g < num_of_groups; g++)
        {
          nextFrame =  pGroupData;
          // Recording
          if (numOfBytes < bytesWriteToFile)
          {
            uint32_t bytestFromGroup =
              (numOfBytes + group_data_size <= bytesWriteToFile) ?
              group_data_size :
              bytesWriteToFile - numOfBytes;
            ual_util_frame_file_write(pGroupData,
                                      sizeof(uint8_t),
                                      bytestFromGroup,
                                      &paramsStruct,
                                      frameFile);
            numOfBytes += bytestFromGroup;

            if (numOfBytes >= bytesWriteToFile)
            {
              if (NULL != frameFile)
              {
                fclose(frameFile);
                frameFile = NULL;
              }
            }
          }

          for (int f = 0; f < numberOfFrames ; f++)
          {
            // Statistics
            int seqNum = *((int *)(nextFrame + ECHO_FRAME_SEQNUM_OFFSET));
            // If this is the first iteration then the frames
            // counter is -1 and we need to update the frames counter.
            if (gestureParams.m_nNextFrameSeqNum == -1)
            {
              gestureParams.m_nNextFrameSeqNum = seqNum;
            }
            // This is not the first iteration.
            else
            {
              if (gestureParams.m_nNextFrameSeqNum != seqNum)
              {
                // We lost some frames so we add the number of lost frames
                // to the statistics.
                if (gestureParams.m_nNextFrameSeqNum < seqNum)
                {
                  gestureStats.m_nLostFrames +=
                    (seqNum - gestureParams.m_nNextFrameSeqNum)/
                    paramsStruct.usf_tx_skip;
                }
                // We got out of order frames so we add the number of
                // out of order frames to the statistics.
                else
                {
                  gestureStats.m_nOutOfOrderErrors +=
                    (gestureParams.m_nNextFrameSeqNum - seqNum)/
                    paramsStruct.usf_tx_skip;
                }

                // Update the frames counter to the correct count.
                gestureParams.m_nNextFrameSeqNum = seqNum;
              }
            }
            gestureStats.m_nTotalFrames++;
            // Update the frames counter to the expected count in the next
            // iteration.
            gestureParams.m_nNextFrameSeqNum += paramsStruct.usf_tx_skip;

            packetCounter++;
            if (STATISTIC_PRINT_INTERVAL == packetCounter)
            {
              LOGI("%s: Statistics (printed every %d frames):",
                   __FUNCTION__,
                   STATISTIC_PRINT_INTERVAL);
              LOGI("Points calculated: %d, total frames: %d, lost frames: %d,"
                   "out of order: %d",
                   gestureStats.m_nPointsCalculated,
                   gestureStats.m_nTotalFrames,
                   gestureStats.m_nLostFrames,
                   gestureStats.m_nOutOfOrderErrors);
              packetCounter = 0;
            }

            // Calculation
            numPoints = gesture_get_points ((short *)nextFrame);
            if (numPoints < 0)
            {
              if (NULL != frameFile)
              {
                fclose(frameFile);
                frameFile = NULL;
              }
              break;
            }

            nextFrame += frame_size_in_bytes;
          } // f (frames) loop

          pGroupData += paramsStruct.usf_tx_buf_size;
        } // g (groups) loop
      } // r (regions) loop
    } // main loop

    if (true == daemon_run)
    { // Received deactivate
      // Free resources before next activate
      gesture_free_resources(false);
    }

    if (NULL == adapter)
    {
      gesture_exit(rc? EXIT_SUCCESS : EXIT_FAILURE);
    }
    else if (adapter->get_status() == ACTIVATE)
    { // Disconnect only while in active state, it's not necessary in other
      // states.
      adapter->disconnect();
    }
  } // End daemon_run

  gesture_exit(EXIT_SUCCESS);

}
