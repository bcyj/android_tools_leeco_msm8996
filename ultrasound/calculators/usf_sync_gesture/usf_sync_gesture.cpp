/*===========================================================================
                           usf_sync_gesture.cpp

DESCRIPTION: Implementation of the Synchronized Gesture daemon.


INITIALIZATION AND SEQUENCING REQUIREMENTS:
  If not started through java app then make sure to have
  correct /data/usf/sync_gesture/usf_sync_gesture.cfg file linked to the wanted cfg file
  placed in /data/usf/sync_gesture/cfg/.

Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_sync_gesture"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <ual.h>
#include <cutils/properties.h>
#include <ual_util.h>
#include <SyncGestureExports.h>
#include <stdlib.h>
#include <errno.h>
#include "ual_util_frame_file.h"
#include "us_adapter_factory.h"
#include <framework_adapter.h>
#include "sync_gesture_socket_adapter.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define LINK_CFG_FILE_LOCATION "/data/usf/sync_gesture/usf_sync_gesture.cfg"
#define FRAME_FILE_DIR_PATH "/data/usf/sync_gesture/rec/"
#define US_MAX_EVENTS 20
#define STATISTIC_PRINT_INTERVAL 500
#define SOCKET_PROBLEM_MSG_INTERVAL 50
#define MAX_SUPPORTED_MIC 8
#define SIZE_32_OFFSET 31
#define SIZE_32_MASK 0xffffffe0
#define BYTE_OFFSET_OF_GROUP_IN_TX_TRANSPARENT 2
#define BYTE_OFFSET_OF_SKIP_IN_TX_TRANSPARENT 0
#define GESTURE_KEY_BASE QC_US_GESTURE_LIB_RESULT_SELECT
#define GESTURE_LAST_KEY QC_US_GESTURE_LIB_RESULT_REPEAT
#define NUM_KEYS ((GESTURE_LAST_KEY - GESTURE_KEY_BASE) + 1)

enum gesture_event_dest
{
  DEST_UAL = 0x01,
  DEST_SOCKET = 0x02,
};

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
  gestureContext holds information needed for Sync Gesture
  calculation.
*/
typedef struct
{
  signed char       *m_gesture_workspace;
  usf_event_type    m_events[US_MAX_EVENTS]; // Array of struct from sys/stat.h
  bool              m_send_points_to_ual; // For future use
  bool              m_send_points_to_socket;
  int               m_next_frame_seq_num;
  int16_t*          m_pattern;
  uint32_t          m_pattern_size; // In bytes
  // Mapping from gesture received by the library to a key
  uint8_t           m_keys[NUM_KEYS];
  int               m_last_event_seq_num;
  bool              m_stub;
} GestureContext;

/**
  GestureStats holds information about statistics from
  usf_sync_gesture running.
*/
typedef struct
{
  int   m_nPointsCalculated;
  int   m_nTotalFrames;
  int   m_nLostFrames;
  int   m_nOutOfOrderErrors;
} GestureStats;

/**
  This struct represents a single sync gesture event generated
  from the gesture library in the dsp.
*/
struct gesture_dsp_event
{
  int                      timestamp;
  int                      seq_num;
  QcUsGestureLibResultType result;
  int velocity;
};

/**
 * Ptr to the frame file recording.
 */
FILE* frame_file = NULL;

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Ptr to the cfg file. The ptr is global bacause we want to
  close the file before exit in the function gesture_exit.
  This function is called also from the signal_handler function
  which is not familiar with the cfg file.
*/
static FILE *cfgFile = NULL;


/**
  GestureContext will hold all the information needed for Sync
  Gesture calculation.
*/
static GestureContext gestureContext;


/**
  gestureStats will hold all the statistics needed.
*/
static GestureStats gestureStats;

/**
  Byte offset of "sequence number" field in the US data frame
*/
static const uint16_t ECHO_FRAME_SEQNUM_OFFSET = 8;

/**
  Byte offset of "sequence number" field in the DSP event
*/
static const uint16_t DSP_EVENT_SEQNUM_OFFSET = 4;

/**
  calculated events counter
*/
static int s_eventCounter = 0;

/**
  Sync Gesture calculator name
*/
static const char* CLIENT_NAME =  "sync_gesture";

/**
  Sync Gesture calculator version
*/
static const char* CLIENT_VERSION = "1.0";

/**
 * The name of the file containg the pid of the daemon
 */
static const char* PID_FILE_NAME = "usf_sync_gesture.pid";

/**
  The daemon running control
*/
static volatile bool sb_run = true;

/**
  The daemon running control. Includes allocating and freeing
  resources, and is mainly used for the adapter control.
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
  int rc = ual_close(error_state);
  LOGD("%s: ual_close: rc=%d;",
       __FUNCTION__,
       rc);

  if (NULL != gestureContext.m_gesture_workspace)
  {
    free(gestureContext.m_gesture_workspace);
    gestureContext.m_gesture_workspace = NULL;
  }

  if (NULL != gestureContext.m_pattern)
  {
    free(gestureContext.m_pattern);
    gestureContext.m_pattern = NULL;
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

  ual_util_close_and_sync_file(frame_file);
  frame_file = NULL;

  int ret = ual_util_remove_declare_pid(PID_FILE_NAME);
  if (0 != ret)
  {
    LOGW("%s: Removing pid file failed",
         __FUNCTION__);
  }

  LOGI("%s: Sync Gesture end. status=%d",
       __FUNCTION__,
       status);

  // Must update flag, so that init would not restart the daemon.
  ret = property_set("ctl.stop",
                     "usf_sync_gesture");

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

  if (paramsStruct->usf_gesture_event_dest & DEST_UAL)
  {
    gestureContext.m_send_points_to_ual = true;
  }
  else
  {
    gestureContext.m_send_points_to_ual = false;
  }

  if (paramsStruct->usf_gesture_event_dest & DEST_SOCKET)
  {
    gestureContext.m_send_points_to_socket = true;
  }
  else
  {
    gestureContext.m_send_points_to_socket = false;
  }

  gestureContext.m_next_frame_seq_num = -1;

  gestureStats.m_nPointsCalculated = 0;
  gestureStats.m_nTotalFrames = 0;
  gestureStats.m_nLostFrames = 0;
  gestureStats.m_nOutOfOrderErrors = 0;

  // Gesture keys
  char usf_gesture_keys_cpy[FILE_PATH_MAX_LEN];
  memcpy(usf_gesture_keys_cpy,
         paramsStruct->usf_gesture_keys,
         FILE_PATH_MAX_LEN);

  if (false == ual_util_parse_string2array(NUM_KEYS,
                                           usf_gesture_keys_cpy,
                                           gestureContext.m_keys))
  {
    gesture_exit(EXIT_FAILURE);
  }

  gestureContext.m_last_event_seq_num = 0;
  gestureContext.m_stub = false;

  LOGI("%s: gesture_params_init finished.",
       __FUNCTION__);
}

/*==============================================================================
  FUNCTION:  get_directions_subset
==============================================================================*/
/**
  Return subset of directions. Return subset if the property
  debug.gesture_subset is defined and greater than 0 or if working with
  gesture framework.
*/
static int get_directions_subset()
{
  // Gets property value with "0" as default
  char prop_val[PROPERTY_VALUE_MAX];
  property_get("debug.gesture_subset",
               prop_val,
               "0");
  if (strcmp("0", prop_val) < 0)
  {
    int directions = 0;
    sscanf(prop_val, "%d", &directions);
    LOGD("%s: Subset defined by property, %d",
         __FUNCTION__,
         directions);
    return directions;
  }

  if (NULL != adapter)
  {
    LOGD("%s: Subset defined by adapter, %d",
         __FUNCTION__,
         (uint32_t)adapter->get_config().sub_mode);
    return (uint32_t)adapter->get_config().sub_mode;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  gesture_lib_init
==============================================================================*/
/**
  Init Gesture library resources.
*/
void gesture_lib_init (us_all_info *paramsStruct)
{
  GestureCfg *gesture_config = NULL;
  uint32_t workspace_size = 0, pattern_size_samples = 0;

  gestureContext.m_gesture_workspace = NULL;
  gesture_config = (GestureCfg *) malloc(sizeof(GestureCfg) +
                                  paramsStruct->usf_algo_transparent_data_size);
  if (NULL == gesture_config)
  {
    LOGE("%s: Failed to allocate.",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  // Fill in the requested algorithm configuration
  gesture_config->TxTransparentDataSize = paramsStruct->usf_algo_transparent_data_size;
  memcpy(gesture_config->TxTransparentData,
         paramsStruct->usf_algo_transparent_data,
         gesture_config->TxTransparentDataSize);

  int ret = QcUsGestureLibGetSizes(gesture_config,
                                   &workspace_size,
                                   &pattern_size_samples);
  if (ret)
  {
    free(gesture_config);
    LOGE("%s: Error while getting size from sync gesture library",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  gestureContext.m_pattern_size = pattern_size_samples *
    (paramsStruct->usf_tx_sample_width / BYTE_WIDTH);

  gestureContext.m_gesture_workspace =
    (signed char *)malloc(workspace_size * sizeof(signed char));
  if (NULL == gestureContext.m_gesture_workspace)
  {
    free(gesture_config);
    LOGE("%s: Failed to allocate %d bytes.",
         __FUNCTION__,
         workspace_size);
    gesture_exit(EXIT_FAILURE);
  }

  gestureContext.m_pattern = (int16_t *) malloc(gestureContext.m_pattern_size);
  if (NULL == gestureContext.m_pattern)
  {
    free(gesture_config);
    LOGE("%s: Failed to allocate %d bytes",
         __FUNCTION__,
         gestureContext.m_pattern_size);
    gesture_exit(EXIT_FAILURE);
  }

  ret = QcUsGestureLibInit(gesture_config,
                           gestureContext.m_gesture_workspace,
                           workspace_size);
  if (ret)
  {
    free(gesture_config);
    LOGE("%s: Init algorithm failed.",
         __FUNCTION__);
    gesture_exit(0);
  }
  free(gesture_config);

  usm_param_id_sync_gesture_dynanic_cfg_t dynamic_config_payload;
  dynamic_config_payload.sync_gesture_dynamic_cfg_version =
    USM_API_VERSION_SYNC_GESTURE_DYNAMIC_CONFIG;
  dynamic_config_payload.directions = get_directions_subset();

  if (0 < dynamic_config_payload.directions)
  {
    ret = QcUsGestureLibSetDynamicConfig(
      (int *)&dynamic_config_payload.sync_gesture_dynamic_cfg_version,
      sizeof(dynamic_config_payload)/sizeof(int));
    if (ret)
    {
      LOGE("%s: Set dynamic config failed.",
           __FUNCTION__);
      gesture_exit(0);
    }
  }

  ret = QcUsGestureLibGetPattern(gestureContext.m_pattern,
                                 pattern_size_samples);
  if (ret)
  {
    LOGE("%s: QcUsGestureLibGetPattern failed.",
         __FUNCTION__);
    gesture_exit(0);
  }

  LOGI("%s: Received pattern from library with %d samples",
       __FUNCTION__,
       pattern_size_samples);

  LOGI("%s: Sync Gesture lib init completed.",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  add_event_key
============================================================================*/
/**
  Creates event and add it to the gestureContext.m_events[].
*/
void add_event_key (int key,
                    int nPressure)
{
  // fill in the usf_event_type struct
  if (US_MAX_EVENTS <= s_eventCounter)
  {
    LOGW("%s: Queue is full=%d, event would not be sent",
         __FUNCTION__,
         US_MAX_EVENTS);
    return;
  }

  usf_event_type *pEvent = &gestureContext.m_events[s_eventCounter];

  memset (pEvent, 0, sizeof (usf_event_type));

  pEvent->seq_num = gestureContext.m_last_event_seq_num ++;
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
/**
 * Checks the adapter status and acts accordingly.
 *
 * @return int 0 - Framework requested to carry on running.
 *             1 - Framework requested termination.
 */
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
  FUNCTION:  get_gesture_output_decision
==============================================================================*/
/**
  Get decision from suitable sync gesture libraries and return final
  result decision.
*/
GestureOutput get_gesture_output_decision(short *packet,
                                          us_all_info *paramsStruct)
{
  QcUsGestureLibResultType dsp_result = QC_US_GESTURE_LIB_RESULT_IDLE;
  GestureOutput gesture_lib_output;
  gesture_lib_output.gesture = QC_US_GESTURE_LIB_RESULT_IDLE;
  int velocity = 0;

  if (paramsStruct->usf_output_type & OUTPUT_TYPE_GESTURE_EVENT_MASK)
  { // Events handling
    struct gesture_dsp_event *dsp_event = (struct gesture_dsp_event *) packet;
    dsp_result = dsp_event->result;
    velocity = dsp_event->velocity;
    // Update pointer to point to next raw data place
    packet += sizeof(struct gesture_dsp_event) / sizeof(short);
  }

  if ((paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK) &&
      !paramsStruct->usf_gesture_app_lib_bypass)
  { // Raw data handling
    int rc = QcUsGestureLibEngine(packet,
                              &gesture_lib_output);
    if (rc)
    {
      LOGE("%s: QcUsGestureLibEngine failed.",
           __FUNCTION__);
      gesture_exit(EXIT_FAILURE);
    }
  }

  /* TODO: uncomment when dsp and apps libraries report accurate events.
  if (QC_US_GESTURE_LIB_RESULT_IDLE != gesture_lib_output.gesture &&
      QC_US_GESTURE_LIB_RESULT_IDLE != dsp_result &&
      gesture_lib_output.gesture != dsp_result)
  {
    LOGW("%s: Library result and DSP result are not the same, lib: %d, dsp: %d",
         __FUNCTION__,
         gesture_lib_output.gesture,
         dsp_result);
  }
  */

  // APPS result has the priority over DSP result
  if (QC_US_GESTURE_LIB_RESULT_IDLE != gesture_lib_output.gesture)
  {
    return gesture_lib_output;
  }
  else if (QC_US_GESTURE_LIB_RESULT_IDLE != dsp_result)
  {
    gesture_lib_output.gesture = dsp_result;
    gesture_lib_output.velocity = velocity;
    return gesture_lib_output;
  }
  gesture_lib_output.gesture = QC_US_GESTURE_LIB_RESULT_IDLE;
  return gesture_lib_output;
}

/*==============================================================================
  FUNCTION:  gesture_get_points
==============================================================================*/
/**
  Call QcGestureAlgorithmEngine() from Sync Gesture lib.
  Returns eventCounter for number of points goes to UAL.
*/
int gesture_get_points(short *packet,
                       us_all_info *paramsStruct)
{
  int     rc = 1;
  GestureOutput gesture_output;

  gesture_output = get_gesture_output_decision(packet,
                                               paramsStruct);

  if (QC_US_GESTURE_LIB_RESULT_IDLE != gesture_output.gesture)
  {
    LOGD("%s: gesture[%d]",
         __FUNCTION__,
         gesture_output.gesture);
  }

  int mapped_gesture = (gesture_output.gesture > 0) ?
                       gestureContext.m_keys[gesture_output.gesture - GESTURE_KEY_BASE] : 0;

  event_source_t event_source = ((paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK) &&
                                 !paramsStruct->usf_gesture_app_lib_bypass) ?
                       EVENT_SOURCE_APSS : EVENT_SOURCE_DSP;

  // Send socket event
  if (gestureContext.m_send_points_to_socket &&
      QC_US_GESTURE_LIB_RESULT_IDLE != gesture_output.gesture)
  {
    if (NULL != adapter)
    {
      int event = (adapter->get_event_mapping() == MAPPED) ?
        // Framework requests mapped events
                  mapped_gesture :
        // Framework requests raw events
                  gesture_output.gesture;
      if (1 == adapter->send_event(event, event_source, gesture_output.velocity))
      {
        LOGE("%s: adapter send_event failed.",
             __FUNCTION__);
      }
    }
  }

  if ((gesture_output.gesture > 0) &&
      (gesture_output.gesture <= (int)(sizeof(gestureContext.m_keys) /
                                sizeof(gestureContext.m_keys[0])) ) )
  {
    // There is a gesture.
    // Assumption: a key duration << time between 2 gestures.

    // Send start press to UAL
    if (gestureContext.m_send_points_to_ual)
    {
      add_event_key (mapped_gesture,
                     1);
      add_event_key (mapped_gesture,
                     0);
    }

    gestureStats.m_nPointsCalculated++;
  }

  return s_eventCounter;
}

/*==============================================================================
  FUNCTION:  update_pattern
==============================================================================*/
/**
  Writes pattern to the DSP
*/
void update_pattern()
{
  LOGD("%s: Update pattern.",
       __FUNCTION__);

  // Pattern is transmitted only once. DSP transmits pattern in loop.
  int rc = ual_write((uint8_t *)gestureContext.m_pattern,
                     gestureContext.m_pattern_size);
  if (1 != rc)
  {
    LOGE("%s: ual_write failed.",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }
}

/*==============================================================================
  FUNCTION:  print_transparent_data
==============================================================================*/
/**
  Prints 0-fill_in_index values of transparent data.
*/
void print_transparent_data(us_all_info paramsStruct, int print_till_index)
{
  for (int i = 0; i < print_till_index; i++)
  {
    LOGD("[%d]%x",
         i,
         paramsStruct.usf_tx_transparent_data[i]);
  }
}

/*==============================================================================
  FUNCTION:  inject_config_params_to_tx_transparent_data
==============================================================================*/
/**
  Injects mic distances, output type and group factor to the tx transparent data.
*/
void inject_config_params_to_tx_transparent_data(us_all_info *paramsStruct)
{
  // Inject skip and group factor to transparent data
  uint32_t skip_index = BYTE_OFFSET_OF_SKIP_IN_TX_TRANSPARENT;
  uint32_t group_index = BYTE_OFFSET_OF_GROUP_IN_TX_TRANSPARENT;
  if (ual_util_inject_to_trans_data(paramsStruct->usf_tx_transparent_data,
                                    &skip_index,
                                    FILE_PATH_MAX_LEN,
                                    paramsStruct->usf_tx_skip,
                                    sizeof(uint16_t)))
  {
    gesture_exit(EXIT_FAILURE);
  }
  if (ual_util_inject_to_trans_data(paramsStruct->usf_tx_transparent_data,
                                    &group_index,
                                    FILE_PATH_MAX_LEN,
                                    paramsStruct->usf_tx_group,
                                    sizeof(uint16_t)))
  {
    gesture_exit(EXIT_FAILURE);
  }

  float mic_info[COORDINATES_DIM], spkr_info[COORDINATES_DIM];
  // Required for ual_util_get_mic_config & ual_util_get_speaker_config
  if (-1 == ual_util_prefill_ports_num_and_id(paramsStruct))
  {
    LOGE("%s: ual_util_prefill_ports_num_and_id failed.",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  // Get speaker info - 0 is for the first spkr since we only have one spkr
  if (-1 == ual_util_get_speaker_config(0, spkr_info))
  {
    LOGE("%s: ual_util_get_speaker_config for speaker failed.",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  for (int i = 0; i < paramsStruct->usf_tx_port_count; i++)
  {
    // Get current mic info
    if (-1 == ual_util_get_mic_config(i, mic_info))
    {
      LOGE("%s: get_mic_config for mic failed for mic #%d.",
           __FUNCTION__,
           i);
      gesture_exit(EXIT_FAILURE);
    }

    // Calculate distance between speaker and the curr mic
    int distance = (int)(sqrt((mic_info[X_IND] - spkr_info[X_IND]) *
                              (mic_info[X_IND] - spkr_info[X_IND]) +
                              (mic_info[Y_IND] - spkr_info[Y_IND]) *
                              (mic_info[Y_IND] - spkr_info[Y_IND]))/10);
    LOGD("%s: mic #%d distance = %d mm",
         __FUNCTION__,
         i,
         distance);

    // Update TX transparent data
    if (ual_util_inject_to_trans_data(paramsStruct->usf_tx_transparent_data,
                                      &(paramsStruct->usf_tx_transparent_data_size),
                                      FILE_PATH_MAX_LEN,
                                      distance,
                                      sizeof(int)))
    {
      gesture_exit(EXIT_FAILURE);
    }
  }

  // Fill in other mic (which do not exist on current configuration) with zeros.
  for (int i = paramsStruct->usf_tx_port_count; i < MAX_SUPPORTED_MIC; i++)
  {
    // Update TX transparent data
    if (ual_util_inject_to_trans_data(paramsStruct->usf_tx_transparent_data,
                                      &(paramsStruct->usf_tx_transparent_data_size),
                                      FILE_PATH_MAX_LEN,
                                      0,
                                      sizeof(int)))
    {
      gesture_exit(EXIT_FAILURE);
    }
  }
  // Fill in tx transparent data with gesture output type.
  if (ual_util_inject_to_trans_data(paramsStruct->usf_tx_transparent_data,
                                    &paramsStruct->usf_tx_transparent_data_size,
                                    FILE_PATH_MAX_LEN,
                                    paramsStruct->usf_output_type,
                                    sizeof(int)))
  {
    gesture_exit(EXIT_FAILURE);
  }

  LOGD("%s: Transparent after injection: size: %d content:",
       __FUNCTION__,
       paramsStruct->usf_tx_transparent_data_size);
  print_transparent_data(*paramsStruct,
                         paramsStruct->usf_tx_transparent_data_size);
}

/*==============================================================================
  FUNCTION:  calculate_statistics
==============================================================================*/
/**
  Calculates statistics about the frames received, and prints them.
*/
void calculate_statistics(us_all_info *paramsStruct,
                          uint8_t* nextFrame,
                          int *packetCounter)
{
  // Statistics
  int seqNum;
  if (paramsStruct->usf_output_type & OUTPUT_TYPE_GESTURE_EVENT_MASK)
  {
    if (paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK)
    {
      // We have gesture event followed by raw data, take the sequence number from the raw data.
      // The sequence number in the gesture event can be out of order when gesture library is busy
      // in the ADSP
      seqNum = *((int*)(nextFrame + sizeof(struct gesture_dsp_event) + ECHO_FRAME_SEQNUM_OFFSET));
    }
    else
    {
      // Getting sequence number from event header
      seqNum = *((int *)(nextFrame + DSP_EVENT_SEQNUM_OFFSET));
    }
  }
  else
  { // Getting sequence number from raw data header file
    seqNum = *((int *)(nextFrame + ECHO_FRAME_SEQNUM_OFFSET));
  }
  // If this is the first iteration then the frames
  // counter is -1 and we need to update the frames counter.
  if (gestureContext.m_next_frame_seq_num == -1)
  {
    LOGD(" %s First iteration, seq: %d",
         __FUNCTION__,
         seqNum);
    gestureContext.m_next_frame_seq_num = seqNum;
  }
  // This is not the first iteration.
  else
  {
    if (gestureContext.m_next_frame_seq_num != seqNum)
    {
      // We lost some frames so we add the number of lost frames
      // to the statistics.
      if (gestureContext.m_next_frame_seq_num < seqNum)
      {
        gestureStats.m_nLostFrames +=
          (seqNum - gestureContext.m_next_frame_seq_num)/
          paramsStruct->usf_tx_skip;
        LOGD("%s Lost frames: expected %d received: %d",
             __FUNCTION__,
             gestureContext.m_next_frame_seq_num, seqNum);
      }
      // We got out of order frames so we add the number of
      // out of order frames to the statistics.
      else
      {
        gestureStats.m_nOutOfOrderErrors +=
          (gestureContext.m_next_frame_seq_num - seqNum)/
          paramsStruct->usf_tx_skip;
      }

      // Update the frames counter to the correct count.
      gestureContext.m_next_frame_seq_num = seqNum;
    }
  }
  gestureStats.m_nTotalFrames++;
  // Update the frames counter to the expected count in the next
  // iteration.
  gestureContext.m_next_frame_seq_num += paramsStruct->usf_tx_skip;

  *packetCounter = *packetCounter + 1;
  if (STATISTIC_PRINT_INTERVAL == *packetCounter)
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
    *packetCounter = 0;
  }
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

void setup_signal_handlers()
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
  FUNCTION:  gesture_init
==============================================================================*/
/**
  Inits the sync gesture daemon parameters, signal handlers, adapter etc`.
*/
void gesture_init(us_all_info *paramsStruct)
{
  // Setup signal handling
  setup_signal_handlers();

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

  if (ual_util_daemon_init(paramsStruct,
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
                          paramsStruct->usf_adapter_lib))
  {
    LOGE("%s: create_adapter failed",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  ual_util_print_US_version(CLIENT_NAME,
                            CLIENT_VERSION);

  inject_config_params_to_tx_transparent_data(paramsStruct);

  ual_util_set_buf_size(&paramsStruct->usf_tx_buf_size,
                        paramsStruct->usf_tx_port_data_size,
                        paramsStruct->usf_tx_sample_width,
                        paramsStruct->usf_tx_port_count,
                        paramsStruct->usf_tx_frame_hdr_size,
                        paramsStruct->usf_tx_group,
                        "tx",
                        (paramsStruct->usf_output_type & OUTPUT_TYPE_GESTURE_EVENT_MASK),
                        (paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK),
                        sizeof(struct gesture_dsp_event));

  // Build rx_transparent_data manually. In the future (hopefully),
  // this will be removed.
  ual_util_set_echo_rx_transparent_data(paramsStruct);

  ual_util_set_rx_buf_size(paramsStruct);
}

/*==============================================================================
  FUNCTION:  adapter_control
==============================================================================*/
/**
  Finds the next adapter command and acts accordingly.
*/
void adapter_control()
{
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
}

/*==============================================================================
  FUNCTION:  init_frame_recording
==============================================================================*/
/**
 * Inits frame file recording.
 *
 * @param paramsStruct - Daemon configuration parameters
 * @param frame_file - The file to hold the frame recording
 */
void init_frame_recording(us_all_info const *paramsStruct)
{
  if (0 >= paramsStruct->usf_frame_count)
  {
    LOGD("%s: usf_frame_count is %d. Frames will not be recorded.",
         __FUNCTION__,
         paramsStruct->usf_frame_count);
    return;
  }
  // frame_file is not yet allocated
  if (NULL == frame_file)
  {
    LOGD("%s, Opening frame file",
         __FUNCTION__);
    // Open frame file from cfg file
    frame_file = ual_util_get_frame_file(paramsStruct,
                                          (char *)FRAME_FILE_DIR_PATH);
    if (NULL == frame_file)
    {
      LOGE("%s: ual_util_get_frame_file failed",
           __FUNCTION__);
      gesture_exit(EXIT_FAILURE);
    }
  }
}

/*==============================================================================
  FUNCTION:  write_frame_recording
==============================================================================*/
/**
  Writes frame recording and closes file when finished.
*/
void write_frame_recording(us_all_info const *paramsStruct,
                           uint32_t *numOfBytes,
                           uint32_t bytesWriteToFile,
                           int group_data_size,
                           uint8_t *pGroupData)
{
  if (*numOfBytes < bytesWriteToFile)
  {
    uint32_t bytestFromGroup =
      (*numOfBytes + group_data_size <= bytesWriteToFile) ?
      group_data_size :
      bytesWriteToFile - *numOfBytes;

    if (paramsStruct->usf_output_type & OUTPUT_TYPE_GESTURE_EVENT_MASK)
    {
      // Skip dsp events when recording
      pGroupData += sizeof(struct gesture_dsp_event);
    }

    ual_util_frame_file_write(pGroupData,
                              sizeof(uint8_t),
                              bytestFromGroup,
                              paramsStruct,
                              frame_file);

    *numOfBytes += bytestFromGroup;
    if (*numOfBytes >= bytesWriteToFile)
    {
      ual_util_close_and_sync_file(frame_file);
      frame_file = NULL;
    }
  }
}

/*==============================================================================
  FUNCTION:  gesture_run
==============================================================================*/
/**
  Main running loop of the sync gesture daemon.
*/
void gesture_run(us_all_info *paramsStruct)
{
  int packetCounter = 0;
  ual_data_type data;
  uint32_t numOfBytes = 0, bytestFromRegion = 0;

  uint32_t frame_hdr_size_in_bytes = paramsStruct->usf_tx_frame_hdr_size;

  uint32_t packet_size_in_bytes = paramsStruct->usf_tx_port_data_size *
    sizeof(paramsStruct->usf_tx_sample_width);

  uint32_t packets_in_frame = paramsStruct->usf_tx_port_count;

  uint32_t frame_size_in_bytes = packet_size_in_bytes * packets_in_frame +
    frame_hdr_size_in_bytes;

  int combined_frame_size = 0;
  if (paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK)
  {
    combined_frame_size += frame_size_in_bytes;
  }
  if (paramsStruct->usf_output_type & OUTPUT_TYPE_GESTURE_EVENT_MASK)
  {
    combined_frame_size += sizeof(struct gesture_dsp_event);
  }

  int num_of_regions = sizeof(data.region) / sizeof(ual_data_region_type);
  uint32_t bytesWriteToFile = paramsStruct->usf_frame_count *
                              frame_size_in_bytes;

  init_frame_recording(paramsStruct);

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

    uint32_t timeout = USF_DEFAULT_TIMEOUT;
    // If the library work mode is DSP and the output is event only
    // (without raw data), then ual_read is blocking until gesture event.
    // If the framework sends disconnect during the ual_read, the daemon
    // will check it only in the next gesture event. Therefore a
    // short timeout for ual_read is needed if working with the framework.
    if (!(paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK) &&
        (NULL == adapter))
    { // In events only mode, need to wait infinitely for events.
      timeout = USF_INFINITIVE_TIMEOUT;
    }

    if (!ual_read(&data,
                  gestureContext.m_events,
                  s_eventCounter,
                  timeout))
    {
      LOGE("%s: ual read failed",
           __FUNCTION__);
      // Breaking the while will make the daemon re-allocate resources
      break;
    }

    s_eventCounter = 0;
    if (0 == data.region[0].data_buf_size)
    {
      continue;
    }

    // Underlay layer provides US data frames in buffers.
    // Each buffer includes one group of the frames.
    // A number of frames is defined by configurable group factor.
    int numberOfFrames = paramsStruct->usf_tx_buf_size / combined_frame_size;
    int group_data_size = numberOfFrames * frame_size_in_bytes;

    for (int r = 0; r < num_of_regions; r++)
    {
      int num_of_groups = data.region[r].data_buf_size /
                          paramsStruct->usf_tx_buf_size;
      uint8_t *pGroupData = data.region[r].data_buf;
      for (int g = 0; g < num_of_groups; g++)
      {
        nextFrame =  pGroupData;
        // Recording
        write_frame_recording(paramsStruct,
                              &numOfBytes,
                              bytesWriteToFile,
                              group_data_size,
                              pGroupData);

        for (int f = 0; f < numberOfFrames ; f++)
        {
          calculate_statistics(paramsStruct, nextFrame, &packetCounter);
          // Calculation
          gesture_get_points ((short *)nextFrame,
                              paramsStruct);
          nextFrame += combined_frame_size;
        } // f (frames) loop
        pGroupData += paramsStruct->usf_tx_buf_size;
      } // g (groups) loop
    } // r (regions) loop
  } // main loop
}

/*==============================================================================
  FUNCTION:  init
==============================================================================*/
/**
  General initialization.
*/
void init(us_all_info *paramsStruct)
{
  us_stream_param_type stream_param;

  adapter_control();

  if (ual_util_ual_open_retries(paramsStruct))
  {
    gesture_exit(EXIT_FAILURE);
  }

  paramsStruct->usf_event_type = (paramsStruct->usf_gesture_event_dest & DEST_UAL) ?
                                 USF_KEYBOARD_EVENT :
                                 USF_NO_EVENT;

  if (ual_util_tx_config(paramsStruct,
                         (char *)CLIENT_NAME))
  {
    LOGE("%s: ual_util_tx_config failed.",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  stream_param.module_id = USM_SYNC_GESTURE_LIB_MODULE_ID;
  stream_param.param_id = USM_PARAM_ID_SYNC_GESTURE_ALGO_CONFIG;
  stream_param.buf_size = paramsStruct->usf_algo_transparent_data_size;
  stream_param.pbuf = (uint8_t *)paramsStruct->usf_algo_transparent_data;
  if (!ual_set_TX_param(stream_param))
  {
    LOGE("%s: ual_set_TX_param failed",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  stream_param.param_id = USM_PARAM_ID_SYNC_GESTURE_DYNAMIC_CONFIG;
  usm_param_id_sync_gesture_dynanic_cfg_t dynamic_config_payload;
  dynamic_config_payload.sync_gesture_dynamic_cfg_version =
    USM_API_VERSION_SYNC_GESTURE_DYNAMIC_CONFIG;
  dynamic_config_payload.directions = get_directions_subset();
  stream_param.buf_size = sizeof(dynamic_config_payload);
  stream_param.pbuf = (uint8_t *)(&dynamic_config_payload);
  if (0 < dynamic_config_payload.directions)
  {
    if (!ual_set_TX_param(stream_param))
    {
      LOGE("%s: ual_set_TX_param failed",
           __FUNCTION__);
      gesture_exit(EXIT_FAILURE);
    }
  }

  if (ual_util_rx_config(paramsStruct,
                         (char* )CLIENT_NAME))
  {
    LOGE("%s: ual_util_rx_config failed.",
         __FUNCTION__);
    gesture_exit(EXIT_FAILURE);
  }

  gesture_params_init(paramsStruct);

  if ((paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK) &&
      !paramsStruct->usf_gesture_app_lib_bypass)
  {
    gesture_lib_init(paramsStruct);
    update_pattern();
  }
  else
  {
    LOGI("%s: Sync Gesture bypass APPS lib",
         __FUNCTION__);
  }
}

/*==============================================================================
  FUNCTION:  main
==============================================================================*/
/**
  Main function of the Gesture daemon. Handle all the Gesture operations.
*/
int main (void)
{
  int ret;
  static us_all_info paramsStruct;
  bool rc = false;

  LOGI("%s: Sync Gesture start",
       __FUNCTION__);

  gesture_init(&paramsStruct);

  while (daemon_run)
  {
    // Clear pending alarm signal that might disrupt this function work.  Alarm
    // signal is used to release daemons from blocking functions.
    alarm(0);

    init(&paramsStruct);

    gesture_run(&paramsStruct);

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
