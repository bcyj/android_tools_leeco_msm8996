/*===========================================================================
                           usf_proximity.cpp

DESCRIPTION: Implementation of the proximity daemon.


INITIALIZATION AND SEQUENCING REQUIREMENTS:
  If not started through java app then make sure to have
  correct /data/usf/proximity/usf_proximity.cfg file linked to the wanted cfg file
  placed in /data/usf/proximity/cfg/.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#define LOG_TAG "usf_proximity"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <cutils/properties.h>
#include <ual_util.h>
#include <usf_unix_domain_socket.h>
#include <stdlib.h>
#include <errno.h>
#include <ProximityExports.h>
#include "ual_util_frame_file.h"
#include "us_adapter_factory.h"
#include <framework_adapter.h>


/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define LINK_CFG_FILE_LOCATION "/data/usf/proximity/usf_proximity.cfg"
#define USF_DSP_VER_FILE "/data/usf/proximity/usf_dsp_ver.txt"
#define FRAME_FILE_DIR_PATH "/data/usf/proximity/rec/"
#define PATTERN_DIR_PATH "/data/usf/proximity/pattern/"
#define STATISTIC_PRINT_INTERVAL 500
#define SOCKET_PROBLEM_MSG_INTERVAL 50
#define EXIT_SIGTERM 3
#define OPEN_RETRIES 10
#define SLEEP_TIME   0.1

// PROX DATA TYPE
#define PROX_RAW_DATA_MASK 1
#define PROX_EVENTS_VAL   2
#define PROX_MOTION_VAL   4
#define PROX_DSP_MASK     6


enum proximity_event_dest
{
  DEST_UAL = 0x01,
  DEST_SOCKET = 0x02,
};

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/
/**
  proximityParams holds information needed for Proximity calculation.
*/
typedef struct
{
  void              *m_proximity_workspace;
  int               m_nNextFrameSeqNum;
  int               m_socket_sending_prob;
  int               m_nLastEventSeqNum;
  bool              m_stub;
  uint8_t           *m_pPattern;
  int               m_patternSize;
} ProximityParams;

/**
  ProximityStats holds information about statistics from
  usf_proximity running.
*/
typedef struct
{
  int   m_nPointsCalculated;
  int   m_nTotalFrames;
  int   m_nLostFrames;
  int   m_nOutOfOrderErrors;
} ProximityStats;

/**
  This struct represents a single proximity event generated from the proximity
  library.
*/
struct proximity_event
{
  int   prox_event_timestamp;
  int   prox_event_seq_num;
  int   prox_event_result;
};

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Ptr to the cfg file. The ptr is global bacause we want to
  close the file before exit in the function proximity_exit.
  This function is called also from the signal_handler function
  which doesn't know the cfg file.
*/
static FILE *cfgFile = NULL;

/**
 * Ptr to the frame file recording.
 */
FILE* frame_file = NULL;

/**
  proximityParams will hold all the information needed for proximity
  calculation.
*/
static ProximityParams proximityParams;

/**
  calculated events counter (upon 1 call ual_read() )
*/
static int s_eventCounter = 0;

/**
  proximityStats will hold all the statistics needed.
*/
static ProximityStats proximityStats;

/**
  Place of "sequence number" field in the US data frame
*/
static const uint16_t ECHO_FRAME_SEQNUM_OFFSET = 8; // bytes

/**
  Proximity calculator name
*/
static const char *CLIENT_NAME = "proximity";

/**
  Proximity calculator version
*/
static const char *CLIENT_VERSION = "2.1";

/**
  The name of the file containg the pid of the daemon
*/
static const char *PID_FILE_NAME = "usf_proximity.pid";

/**
  m_proximity_data_socket is pointer to the thread which handles data socket
  communication.
*/
static DataUnSocket *m_proximity_data_socket;

/**
  Holds the current state for proximity, initialized to no prox
*/
static int cur_prox_state = RES_NO_PROX;

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
  FUNCTION:  on_client_connected
==============================================================================*/
/**
  This callback function is registered with the data socket, so that an event
  with the current state is sent for the process that is connected to the
  socket.
*/
static void on_client_connected()
{
  if (NULL != m_proximity_data_socket)
  {
    m_proximity_data_socket->send_proximity_event(0, // Timestamp not available
                                                  0, // seq_num not available
                                                  cur_prox_state);
  }
}

/*==============================================================================
  FUNCTION:  proximity_free_resources
==============================================================================*/
/**
  Frees proximity allocated resources, this function is called when daemon
  is deactivated.
*/
void proximity_free_resources(bool error_state)
{
  int rc = ual_close(error_state);

  LOGD("%s: ual_close: rc=%d;",
       __FUNCTION__,
       rc);

  if (NULL != proximityParams.m_proximity_workspace)
  {
    free(proximityParams.m_proximity_workspace);
    proximityParams.m_proximity_workspace = NULL;
  }

  if (NULL != m_proximity_data_socket)
  {
    delete m_proximity_data_socket;
    m_proximity_data_socket = NULL;
  }

  if (NULL != proximityParams.m_pPattern)
  {
    free(proximityParams.m_pPattern);
    proximityParams.m_pPattern = NULL;
  }
}


/*==============================================================================
  FUNCTION:  proximity_exit
==============================================================================*/
/**
  Perform clean exit of the daemon.
*/
int proximity_exit(int status)
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

  proximity_free_resources(error_state);

  ual_util_close_and_sync_file(frame_file);
  frame_file = NULL;

  int ret = ual_util_remove_declare_pid(PID_FILE_NAME);
  if (0 != ret)
  {
    LOGW("%s: Removing pid file failed",
         __FUNCTION__);
  }

  // Must update flag, so that init would not restart the daemon.
  ret = property_set("ctl.stop",
                     "usf_proximity");
  if (0 != ret)
  {
    LOGW("%s: property_set failed",
         __FUNCTION__);
  }

  exit(status);
}

/*==============================================================================
  FUNCTION:  proximity_params_init
==============================================================================*/
/**
  Init proximityParam struct.
*/
void proximity_params_init(us_all_info *paramsStruct)
{
  uint32_t port;
  char *temp = NULL, *ip = NULL;
  int ret;

  proximityParams.m_patternSize = paramsStruct->usf_rx_pattern_size *
                                 paramsStruct->usf_rx_sample_width/BYTE_WIDTH;

  // More details on why pattern is used for proximity are available in the
  // Ultrasound Hexagon ISOD
  proximityParams.m_pPattern = (uint8_t *) malloc(proximityParams.m_patternSize);

  if (NULL == proximityParams.m_pPattern)
  {
    LOGE("%s: Could not allocate space for pattern",
          __FUNCTION__);
    proximity_exit(EXIT_FAILURE);
  }

  proximityParams.m_nNextFrameSeqNum = -1;

  proximityParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;

  proximityStats.m_nPointsCalculated = 0;
  proximityStats.m_nTotalFrames = 0;
  proximityStats.m_nLostFrames = 0;
  proximityStats.m_nOutOfOrderErrors = 0;

  // TODO: Insert path to config file
  m_proximity_data_socket =
    new DataUnSocket("/data/usf/proximity/data_socket",
                     on_client_connected);

  if (m_proximity_data_socket->start() != 0)
  {
    LOGE("%s: Starting data socket failed.",
         __FUNCTION__);
    proximity_exit(EXIT_FAILURE);
  }

  proximityParams.m_nLastEventSeqNum = 0;
  proximityParams.m_stub = false;

  LOGI("%s: proximity_params_init finished.",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  proximity_init
============================================================================*/
/**
  Inits all of proximity daemon's parameters
*/
void proximity_init(us_all_info *paramsStruct) {
  int size = QcUsProximityLibGetSizes();
  if (size < 0) {
    LOGE("%s: Error while getting size from proximity library",
         __FUNCTION__);
    proximity_exit(0);
  }
  proximityParams.m_proximity_workspace = malloc(size);
  if (proximityParams.m_proximity_workspace == NULL) {
    LOGE("%s: Error while allocating memory for proximity workspace",
         __FUNCTION__);
    proximity_exit(0);
  }
  uint8_t params[(paramsStruct->usf_tx_transparent_data_size) +
                (paramsStruct->usf_rx_transparent_data_size)];
  memcpy(params,
         paramsStruct->usf_rx_transparent_data,
         paramsStruct->usf_rx_transparent_data_size);
  memcpy(params + paramsStruct->usf_rx_transparent_data_size,
         paramsStruct->usf_tx_transparent_data,
         paramsStruct->usf_tx_transparent_data_size);

  QcUsProximityLibInit((signed char *)proximityParams.m_proximity_workspace,
                       (int *)params);
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
  FILE *fp = fopen(USF_DSP_VER_FILE,
                   "wt");
  if (fp == NULL)
  {
    LOGE("%s: Could not open %s - %s",
         __FUNCTION__,
         USF_DSP_VER_FILE,
         strerror(errno));
    proximity_exit(EXIT_FAILURE);
  }

  fclose(fp);
}

/*==============================================================================
  FUNCTION:  signal_handler
==============================================================================*/
/**
  This signal handler sets running flag to false, daemon will exit shortly
  afterwards.
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
  FUNCTION:  load_pattern_into_libs
==============================================================================*/
/**
  Reads the pattern from the file path mentioned in the config file and loads it
  into DSP/APPS libraries in case no file path is specified in the config file,
  this function does nothing
*/
static void load_pattern_into_libs(us_all_info *pParamsStruct)
{
  bool rc;
  int irc;
  // Send pattern to UAL for the first time
  // Pattern is taken from file named in the cfg file
  if (0 != pParamsStruct->usf_rx_pattern[0])
  {
    irc = ual_util_read_pattern(proximityParams.m_pPattern,
                                 pParamsStruct,
                                 (char *)PATTERN_DIR_PATH);
    if (0 != irc)
    {
      LOGE("%s: ual_util_read_pattern failed. rc: %d",
           __FUNCTION__,
           irc);
      proximity_exit(EXIT_FAILURE);
    }

    // Pattern is transmitted only once. DSP transmits pattern in a loop.
    rc = ual_write(proximityParams.m_pPattern,
                   proximityParams.m_patternSize);
    if (!rc)
    {
      LOGE("%s: ual_write failed. rc: %d",
           __FUNCTION__,
           rc);
      proximity_exit(EXIT_FAILURE);
    }
    irc = QcUsProximityLibUpdatePattern((int16_t*)proximityParams.m_pPattern,
                                       proximityParams.m_patternSize/2);
    if (0 != irc)
    {
      LOGE("%s: QcUsProximityLibUpdatePattern failed. rc: %d",
           __FUNCTION__,
           irc);
      proximity_exit(EXIT_FAILURE);
    }
    LOGD("%s: Finished writing pattern",
         __FUNCTION__);
  }
}

/*==============================================================================
  FUNCTION:  main
==============================================================================*/
/**
  Main function of the Proximity daemon. Handle all the Proximity operations.
*/
int main (void)
{
  const uint32_t SIZE_32_OFFSET = 31;
  const uint32_t SIZE_32_MASK = 0xffffffe0;

  int ret, ind = 0, numPoints = 0, packetCounter = 0;
  int iPatternUpdate = 0;
  static us_all_info paramsStruct;
  bool rc = false, frame_file_created = false;
  ual_data_type data;
  uint32_t frame_hdr_size_in_bytes;
  uint32_t packet_size_in_bytes, packets_in_frame, frame_size_in_bytes;
  uint32_t mic_id, spkr_id;
  float mic_info[COORDINATES_DIM], spkr_info[COORDINATES_DIM];
  char temp[FILE_PATH_MAX_LEN];

  LOGI("%s: Proximity start",
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
    proximity_exit(EXIT_FAILURE);
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
    proximity_exit(EXIT_FAILURE);
  }

  // Get suitable adapter
  if (0 != create_adapter(&adapter,
                          paramsStruct.usf_adapter_lib))
  {
    LOGE("%s: create_adapter failed",
         __FUNCTION__);
    proximity_exit(EXIT_FAILURE);
  }

  if (adapter != NULL)
  {
    adapter->set_pid(getpid());
  }

  packets_in_frame =
    paramsStruct.usf_tx_port_count;

  frame_hdr_size_in_bytes =
    paramsStruct.usf_tx_frame_hdr_size;

  packet_size_in_bytes =
    paramsStruct.usf_tx_port_data_size *
    sizeof(paramsStruct.usf_tx_sample_width);

  int raw_data_buf_size = (packet_size_in_bytes * packets_in_frame) +
                                    frame_hdr_size_in_bytes;

  ual_util_set_buf_size(&paramsStruct.usf_tx_buf_size,
                        paramsStruct.usf_tx_port_data_size,
                        paramsStruct.usf_tx_sample_width,
                        paramsStruct.usf_tx_port_count,
                        paramsStruct.usf_tx_frame_hdr_size,
                        1,
                        "tx",
                        (paramsStruct.usf_output_type & PROX_DSP_MASK),
                        (paramsStruct.usf_output_type & PROX_RAW_DATA_MASK),
                        sizeof(struct proximity_event));

  // Required for ual_util_get_mic_config & ual_util_get_speaker_config
  if (-1 == ual_util_prefill_ports_num_and_id(&paramsStruct))
  {
    LOGE("%s: ual_util_prefill_ports_num_and_id failed.",
         __FUNCTION__);
    proximity_exit(EXIT_FAILURE);
  }
  // Get speaker/mic info - 0 is for the first mic/spkr since we only have one
  // mic/spkr
  if (-1 == ual_util_get_mic_config(0, mic_info))
  {
    LOGE("%s: get_mic_config for mic failed.",
         __FUNCTION__);
    proximity_exit(EXIT_FAILURE);
  }
  if (-1 == ual_util_get_speaker_config(0, spkr_info))
  {
    LOGE("%s: ual_util_get_speaker_config for speaker failed.",
         __FUNCTION__);
    proximity_exit(EXIT_FAILURE);
  }
  // Calculate distance between mic and speaker
  int distance = (int)(sqrt((mic_info[X_IND] - spkr_info[X_IND]) *
                            (mic_info[X_IND] - spkr_info[X_IND]) +
                            (mic_info[Y_IND] - spkr_info[Y_IND]) *
                            (mic_info[Y_IND] - spkr_info[Y_IND]))/10);
  LOGD("%s: distance = %d mm",
       __FUNCTION__,
       distance);

  // Update TX transparent data
  if (ual_util_inject_to_trans_data(paramsStruct.usf_tx_transparent_data,
                                    &paramsStruct.usf_tx_transparent_data_size,
                                    FILE_PATH_MAX_LEN,
                                    distance,
                                    sizeof(int)))
  {
    proximity_exit(EXIT_FAILURE);
  }
  if (ual_util_inject_to_trans_data(paramsStruct.usf_tx_transparent_data,
                                    &paramsStruct.usf_tx_transparent_data_size,
                                    FILE_PATH_MAX_LEN,
                                    paramsStruct.usf_output_type,
                                    sizeof(int)))
  {
    proximity_exit(EXIT_FAILURE);
  }
  // Update RX transparent data
  if (ual_util_inject_to_trans_data(paramsStruct.usf_rx_transparent_data,
                                    &paramsStruct.usf_rx_transparent_data_size,
                                    FILE_PATH_MAX_LEN,
                                    paramsStruct.usf_rx_port_data_size,
                                    sizeof(int)))
  {
    proximity_exit(EXIT_FAILURE);
  }
  paramsStruct.usf_rx_group = 1; // group factor for proximity rx is always 1

  ual_util_set_rx_buf_size(&paramsStruct);

  ual_util_print_US_version(CLIENT_NAME, CLIENT_VERSION);

  while (daemon_run)
  {
    int wait_status = (NULL != adapter) ? adapter->wait_n_update() : 0;
    switch (wait_status)
    {
    case ACTIVATE:
      sb_run = true;
      break;
    case SHUTDOWN:
      proximity_exit(EXIT_SUCCESS);
    case INTERRUPT:
    case FAILURE:
    default:
      proximity_exit(EXIT_FAILURE);
    }

    if (ual_util_ual_open_retries(&paramsStruct))
    {
      proximity_exit(EXIT_FAILURE);
    }

    // Recording init
    if (0 >= paramsStruct.usf_frame_count)
    {
      LOGD("%s: usf_frame_count is %d. No record has made.",
           __FUNCTION__,
           paramsStruct.usf_frame_count);
    }

    paramsStruct.usf_event_type = USF_NO_EVENT;


    // Config RX & TX again after change transparent data
    if (ual_util_tx_config(&paramsStruct,
                           (char *)CLIENT_NAME))
    {
      LOGE("%s: ual_util_tx_config failed.",
           __FUNCTION__);
      proximity_exit(EXIT_FAILURE);
    }

    if (ual_util_rx_config(&paramsStruct,
                           (char *)CLIENT_NAME))
    {
      LOGE("%s: ual_util_rx_config failed.",
           __FUNCTION__);
      proximity_exit(EXIT_FAILURE);
    }

    proximity_params_init(&paramsStruct);

    proximity_init(&paramsStruct);

    print_DSP_ver();

    load_pattern_into_libs(&paramsStruct);

    frame_size_in_bytes =
      packet_size_in_bytes * packets_in_frame +
      frame_hdr_size_in_bytes;

    uint32_t numOfBytes = 0;

    uint32_t bytesWriteToFile = paramsStruct.usf_frame_count *
                                frame_size_in_bytes;

    int num_of_regions = sizeof(data.region) / sizeof(ual_data_region_type);
    // Must add daemon_run flag as well, in case signal is received
    // right before assigning sb_run = true in the above switch.
    while (sb_run &&
           daemon_run)
    {
      if (0 != check_adapter_status())
      {
        LOGE("%s: Framework requested termination",
             __FUNCTION__);
        proximity_exit(EXIT_FAILURE);
      }

      uint8_t *nextFrame = NULL;
      rc = ual_read(&data,
                    NULL, /* No events are sent to ual for now */
                    s_eventCounter,
                    USF_INFINITIVE_TIMEOUT);
      if (!rc)
      {
        // Disable pending alarm signal. It won't be needed anymore, because
        // no further blocking functions are to come
        alarm(0);
        LOGE("%s: ual read failed",
             __FUNCTION__);
        // Breaking the while will make the daemon re-allocate resources
        break;
      }

      // frame_file is not yet allocated
      if (0 < paramsStruct.usf_frame_count &&
          NULL == frame_file &&
          false == frame_file_created)
      {
        frame_file_created = true;
        // Open frame file from cfg file
        frame_file = ual_util_get_frame_file(&paramsStruct,
                                            (char *)FRAME_FILE_DIR_PATH);
        if (NULL == frame_file)
        {
          LOGE("%s: ual_util_get_frame_file failed",
               __FUNCTION__);
          proximity_exit(EXIT_FAILURE);
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

      if (!(paramsStruct.usf_output_type & PROX_RAW_DATA_MASK))
      {
        numberOfFrames = 1; // Frame number in case of proximity type only
      }

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

            if (paramsStruct.usf_output_type & PROX_DSP_MASK)
            {
              // Skip proxiomity events when recording is activated while
              // proximity is on
              pGroupData += sizeof(struct proximity_event);
            }

            ual_util_frame_file_write(pGroupData,
                                    sizeof(uint8_t),
                                    bytestFromGroup,
                                    &paramsStruct,
                                    frame_file);

            if (paramsStruct.usf_output_type & PROX_DSP_MASK)
            {
              // Bring the events back after finishing recording
              pGroupData -= sizeof(struct proximity_event);
            }

            numOfBytes += bytestFromGroup;

            if (numOfBytes >= bytesWriteToFile)
            {
              if (NULL != frame_file)
              {
                ual_util_close_and_sync_file(frame_file);
                frame_file = NULL;
              }
            }
          }
          for (int f = 0; f < numberOfFrames ; f++)
          {
            // Statistics
            int seqNum = *(nextFrame + ECHO_FRAME_SEQNUM_OFFSET /
                         sizeof(short));

            // If this is the first iteration then the frames
            // counter is -1 and we need to update the frames counter.
            if (proximityParams.m_nNextFrameSeqNum == -1)
            {
              proximityParams.m_nNextFrameSeqNum = seqNum;
            }
            // This is not the first iteration.
            else
            {
              if (proximityParams.m_nNextFrameSeqNum != seqNum)
              {
                // We lost some frames so we add the number of lost frames
                // to the statistics.
                if (proximityParams.m_nNextFrameSeqNum < seqNum)
                {
                  proximityStats.m_nLostFrames +=
                    (seqNum - proximityParams.m_nNextFrameSeqNum);
                }
                // We got out of order frames so we add the number of
                // out of order frames to the statistics.
                else
                {
                  proximityStats.m_nOutOfOrderErrors +=
                    (proximityParams.m_nNextFrameSeqNum - seqNum);
                }

                // Update the frames counter to the correct count.
                proximityParams.m_nNextFrameSeqNum = seqNum;
              }
            }
            proximityStats.m_nTotalFrames++;
            packetCounter++;
            if (STATISTIC_PRINT_INTERVAL == packetCounter)
            {
              LOGI("%s: Statistics (printed every %d frames):",
                   __FUNCTION__,
                   STATISTIC_PRINT_INTERVAL);
              LOGI("Points calculated: %d, total frames: %d, lost frames: %d, "
                   "out of order: %d",
                   proximityStats.m_nPointsCalculated,
                   proximityStats.m_nTotalFrames,
                   proximityStats.m_nLostFrames,
                   proximityStats.m_nOutOfOrderErrors);
              packetCounter = 0;
            }

            // Handle data received from DSP
            struct proximity_event *dsp_event  = NULL;
            int                     lib_result = 0;
            int                     dsp_result = 0;

            if (paramsStruct.usf_output_type & PROX_DSP_MASK)
            { // Events handling

              dsp_event  = (struct proximity_event *) nextFrame;
              dsp_result = dsp_event->prox_event_result;
              if (dsp_result)
              {
                LOGD("%s: Received event: 0x%x 0x%x 0x%x",
                     __FUNCTION__,
                     dsp_event->prox_event_timestamp,
                     dsp_event->prox_event_seq_num,
                     dsp_result);
              }

              // Update pointer to point to next data place
              nextFrame += sizeof(struct proximity_event);
            }

            if (paramsStruct.usf_output_type & PROX_RAW_DATA_MASK)
            { // Raw data handling

              lib_result = QcUsProximityLibEngine((short *)nextFrame);
              // Update pointer to point to next data place
              nextFrame += raw_data_buf_size;
            }

            if (lib_result &&
                dsp_result &&
                lib_result != dsp_result)
            {
              LOGE("%s: Library result and DSP results are not equal, lib: %d, dsp:"
                   "%d",
                   __FUNCTION__,
                   lib_result,
                   dsp_result);
            }

            // APPS result has the priority over DSP result
            if (lib_result)
            {
              cur_prox_state = lib_result;
            }
            else if (dsp_result)
            {
              cur_prox_state = dsp_result;
            }

            // Send proximity event to socket
            if (NULL != m_proximity_data_socket)
            {
              int timestamp = 0;
              int seq_num   = 0;

              if (dsp_event)
              { // Update timestamp/seq_num from received dsp_event
                timestamp = dsp_event->prox_event_timestamp;
                seq_num   = dsp_event->prox_event_seq_num;
              }
              if (cur_prox_state)
              {
                if (NULL != adapter)
                {
                  // Since we don't have any mapped events, we only send unmapped
                  // events for now.

                  event_source_t event_source = (paramsStruct.usf_output_type & OUTPUT_TYPE_RAW_MASK) ?
                      EVENT_SOURCE_APSS : EVENT_SOURCE_DSP;

                  if (1 == adapter->send_event(cur_prox_state, event_source, 0))
                  {
                    LOGE("%s: adapter send_event failed.",
                         __FUNCTION__);
                  }
                }
                m_proximity_data_socket->send_proximity_event(
                                                      timestamp,
                                                      seq_num,
                                                      cur_prox_state);
              }
            }

            proximityStats.m_nPointsCalculated++;
          } // End for f (frames)
          pGroupData += paramsStruct.usf_tx_buf_size;
        } // g (groups) loop
      } // End for r (regions)

      uint32_t ual_status = ual_get_status();
      if (!(ual_status & UAL_RX_STATUS_ON))
      { // Earpiece is now used by another device, stopping dameon
        sb_run = false;
      }
    } // while (sb_run && daemon_run)
    if (true == daemon_run)
    { // Received deactivate
      // Free resources before next activate
      proximity_free_resources(false);
    }

    if (NULL == adapter)
    {
      proximity_exit(rc? EXIT_SUCCESS : EXIT_FAILURE);
    }
    else if (adapter->get_status() == ACTIVATE)
    { // Disconnect only while in active state, it's not necessary in other
      // states.
      adapter->disconnect();
    }
  } // while(daemon run)

  proximity_exit(EXIT_SUCCESS);
}
