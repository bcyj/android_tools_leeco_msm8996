/*===========================================================================
                           usf_p2p.cpp

DESCRIPTION: Implementation of the P2P daemon.


INITIALIZATION AND SEQUENCING REQUIREMENTS:
  If not started through java app then make sure to have
  correct /data/usf/p2p/usf_p2p.cfg file linked to the wanted cfg file
  placed in /data/usf/p2p/cfg/.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_p2p"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <ual.h>
#include <cutils/properties.h>
#include <ual_util.h>
#include <P2PExports.h>
#include <stdlib.h>
#include <errno.h>
#include "ual_util_frame_file.h"
#include "usf_unix_domain_socket.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define LINK_CFG_FILE_LOCATION "/data/usf/p2p/usf_p2p.cfg"
#define FRAME_FILE_DIR_PATH "/data/usf/p2p/rec/"
#define US_MAX_EVENTS 20
#define STATISTIC_PRINT_INTERVAL 500
#define SOCKET_PROBLEM_MSG_INTERVAL 500
#define MAX_SUPPORTED_MIC 8
#define SIZE_32_OFFSET 31
#define SIZE_32_MASK 0xffffffe0
#define BYTE_OFFSET_OF_GROUP_IN_TX_TRANSPARENT 2
#define BYTE_OFFSET_OF_SKIP_IN_TX_TRANSPARENT 0
#define BYTE_OFFSET_OF_SAMPLES_PER_FRAME_IN_TX_TRANSPARENT 4

enum p2p_event_dest
{
  DEST_UAL = 0x01,
  DEST_SOCKET = 0x02,
};

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
  p2pContext holds information needed for P2P
  calculation.
*/
typedef struct
{
  signed char       *m_p2p_workspace;
  bool              m_send_points_to_ual; // For future use
  bool              m_send_points_to_socket;
  int               m_next_frame_seq_num;
  int16_t*          m_pattern;
  uint32_t          m_pattern_size_samples;
  uint32_t          m_pattern_size; // In bytes
  int               m_last_event_seq_num;
  bool              m_stub;
  bool              m_rx_started;
  int               m_socket_sending_prob;
  int               m_socket_keep_alive_counter;
  P2POutput         m_last_lib_result;
} P2PContext;

/**
  P2PStats holds information about statistics from
  usf_p2p running.
*/
typedef struct
{
  int   m_nPointsCalculated;
  int   m_nTotalFrames;
  int   m_nLostFrames;
  int   m_nOutOfOrderErrors;
} P2PStats;

/**
  This struct represents a single p2p event generated
  from the p2p library in the dsp.
*/
struct p2p_dsp_event
{
  int                      timestamp;
  int                      seq_num;
  P2POutput                result;
};

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Ptr to the cfg file. The ptr is global bacause we want to
  close the file before exit in the function p2p_exit.
  This function is called also from the signal_handler function
  which is not familiar with the cfg file.
*/
static FILE *cfgFile = NULL;


/**
  p2pContext will hold all the information needed for P2P
  calculation.
*/
static P2PContext p2pContext;


/**
  p2pStats will hold all the statistics needed.
*/
static P2PStats p2pStats;

/**
  m_p2p_data_socket is pointer to the thread who handles data
  socket communication with a P2P client (such as demo app).
*/
static DataUnSocket *m_p2p_data_socket;

/**
  Byte offset of "sequence number" field in the US data frame
  TODO check offset
*/
static const uint16_t ECHO_FRAME_SEQNUM_OFFSET = 8;

/**
  Byte offset of "sequence number" field in the DSP event
  TODO check offset
*/
static const uint16_t DSP_EVENT_SEQNUM_OFFSET = 4;

/**
  calculated events counter
*/
static int s_eventCounter = 0;

/**
  P2P calculator name
*/
static const char* CLIENT_NAME =  "p2p";

/**
  P2P calculator version
*/
static const char* CLIENT_VERSION = "2.0";

/**
 * The name of the file containg the pid of the daemon
 */
static const char* PID_FILE_NAME = "usf_p2p.pid";

/**
 * Maximum number of iterations of ual_read loop before
 * we forcibly send update through socket (for keep alive)
 */
static const int SOCKET_MAX_ITERATIONS_FOR_KEEP_ALIVE = 16; // ~1 second

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
 * Ptr to the frame file recording.
 */
FILE* frame_file = NULL;

/*------------------------------------------------------------------------------
  Prototypes (forward declarations)
------------------------------------------------------------------------------*/
void p2p_fill_p2p_lib_configuration(us_all_info *paramsStruct, P2PCfg* cfg);

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  p2p_free_resources
==============================================================================*/
/**
  Clean daemon parameters.
*/
void p2p_free_resources (int status)
{
  bool error_state = (status != EXIT_SUCCESS);
  int rc = ual_close(error_state);
  LOGD("%s: ual_close: rc=%d;",
       __FUNCTION__,
       rc);

  if (NULL != p2pContext.m_p2p_workspace)
  {
    free(p2pContext.m_p2p_workspace);
    p2pContext.m_p2p_workspace = NULL;
  }

  if (NULL != p2pContext.m_pattern)
  {
    free(p2pContext.m_pattern);
    p2pContext.m_pattern = NULL;
  }
}

/*==============================================================================
  FUNCTION:  p2p_exit
==============================================================================*/
/**
  Perform clean exit of the daemon.
*/
int p2p_exit (int status)
{
  p2p_free_resources(status);

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
                     "usf_p2p");

  if (0 != ret)
  {
    LOGW("%s: property_set failed",
         __FUNCTION__);
  }

  _exit(status);
}

/*==============================================================================
  FUNCTION:  p2p_params_init
==============================================================================*/
/**
  Init p2pParam struct.
*/
void p2p_params_init (us_all_info *paramsStruct)
{
  uint32_t port;
  char *temp = NULL, *ip = NULL;
  int ret;

  if (paramsStruct->usf_p2p_event_dest & DEST_UAL)
  {
    p2pContext.m_send_points_to_ual = true;
  }
  else
  {
    p2pContext.m_send_points_to_ual = false;
  }

  if (paramsStruct->usf_p2p_event_dest & DEST_SOCKET)
  {
    p2pContext.m_send_points_to_socket = true;
  }
  else
  {
    p2pContext.m_send_points_to_socket = false;
  }

  p2pContext.m_next_frame_seq_num = -1;

  p2pStats.m_nPointsCalculated = 0;
  p2pStats.m_nTotalFrames = 0;
  p2pStats.m_nLostFrames = 0;
  p2pStats.m_nOutOfOrderErrors = 0;

  p2pContext.m_last_event_seq_num = 0;
  p2pContext.m_stub = false;

  // create data socket for sending p2p events
  // TODO: Insert path to config file
  m_p2p_data_socket =
    new DataUnSocket("/data/usf/p2p/data_socket");
  if (m_p2p_data_socket->start() != 0)
  {
    LOGE("%s: Starting data socket failed.",
         __FUNCTION__);
    p2p_exit(EXIT_FAILURE);
  }

  LOGI("%s: p2p_params_init finished.",
       __FUNCTION__);
}

/*==============================================================================
  FUNCTION:  p2p_lib_init
==============================================================================*/
/**
  Init P2P library resources.
*/
void p2p_lib_init (us_all_info *paramsStruct)
{
  P2PCfg *p2p_config = NULL;
  uint32_t workspace_size = 0, pattern_size_samples = 0;
  uint32_t i;

  p2pContext.m_p2p_workspace = NULL;
  p2p_config = (P2PCfg *) malloc(sizeof(P2PCfg));
  if (NULL == p2p_config)
  {
    LOGE("%s: Failed to allocate.",
         __FUNCTION__);
    p2p_exit(EXIT_FAILURE);
  }

  // Fill in the requested algorithm configuration.
  // the configuration is part of the transparent data
  // TODO currently we use free form (echo) service and not the p2p service, so
  // we init the library configuration separately because it is not in the transparent
  // data yet
#if 0
  P2PExternalCfg *extCfg = (P2PExternalCfg*)(paramsStruct->usf_tx_transparent_data);

  memcpy(p2p_config,
         &extCfg->libConfig,
         sizeof(P2PCfg));
#endif
  p2p_fill_p2p_lib_configuration(paramsStruct,p2p_config);

  int ret = QcUsP2PLibGetSizes(p2p_config,
                               &workspace_size,
                               &pattern_size_samples);
  if (ret)
  {
    free(p2p_config);
    LOGE("%s: Error while getting size from p2p library",
         __FUNCTION__);
    p2p_exit(EXIT_FAILURE);
  }

  p2pContext.m_pattern_size_samples = pattern_size_samples;
  p2pContext.m_pattern_size = pattern_size_samples *
    (paramsStruct->usf_tx_sample_width / BYTE_WIDTH);

  p2pContext.m_p2p_workspace =
    (signed char *)malloc(workspace_size * sizeof(signed char));
  if (NULL == p2pContext.m_p2p_workspace)
  {
    free(p2p_config);
    LOGE("%s: Failed to allocate %d bytes.",
         __FUNCTION__,
         workspace_size);
    p2p_exit(EXIT_FAILURE);
  }

  p2pContext.m_pattern = (int16_t *) malloc(p2pContext.m_pattern_size);
  if (NULL == p2pContext.m_pattern)
  {
    free(p2p_config);
    LOGE("%s: Failed to allocate %d bytes",
         __FUNCTION__,
         p2pContext.m_pattern_size);
    p2p_exit(EXIT_FAILURE);
  }

  // TODO inject configuration parameters from paramsStruct to transparent data
  ret = QcUsP2PLibInit(p2p_config,
                       p2pContext.m_p2p_workspace,
                       workspace_size);
  if (ret)
  {
    free(p2p_config);
    LOGE("%s: Init algorithm failed.",
         __FUNCTION__);
    p2p_exit(0);
  }
  free(p2p_config);

  // initial result - all users not present
  for (i=0; i<QC_US_P2P_MAX_USERS; i++)
  {
      p2pContext.m_last_lib_result.usersStatus[i].distance = QC_US_P2P_DISTANCE_NOT_PRESENT;
  }

  LOGI("%s: P2P lib init completed.",
       __FUNCTION__);
}

/*==============================================================================
  FUNCTION:  p2p_get_new_rx_pattern
==============================================================================*/
/**
  Get a new RX pattern from the P2P library.
  @return 0 on success, other value on failure
*/
int p2p_get_new_rx_pattern(us_all_info* paramsStruct)
{
  int ret;

  ret = QcUsP2PLibGetPattern(p2pContext.m_pattern,
                             p2pContext.m_pattern_size_samples);
  if (ret)
  {
    LOGE("%s: QcUsP2PLibGetPattern failed.",
         __FUNCTION__);
  }
  else
  {
    LOGI("%s: Received pattern from library with %d samples",
         __FUNCTION__,
         p2pContext.m_pattern_size_samples);

    LOGI("Pattern received begins with: %d,%d,%d,%d",
         p2pContext.m_pattern[0],
         p2pContext.m_pattern[1],
         p2pContext.m_pattern[2],
         p2pContext.m_pattern[3]);
  }

  return ret;
}

/*==============================================================================
  FUNCTION:  p2p_send_rx_pattern_to_dsp
==============================================================================*/
/**
  Send an updated RX pattern to the LPASS
  new updated RX pattern will be transmitted immediately
  @return 0 on success, other value on failure
*/
int p2p_send_rx_pattern_to_dsp(us_all_info *paramsStruct)
{
  int ret = 0;

  // Pattern is transmitted only once. DSP transmits pattern in loop.
  int rc = ual_write((uint8_t *)p2pContext.m_pattern,
                     p2pContext.m_pattern_size);
  if (1 != rc)
  {
    LOGE("%s: ual_write failed.",
         __FUNCTION__);
    ret = -1;
  }

  return ret;
}

/*==============================================================================
  FUNCTION:  p2p_stop_rx_in_dsp
==============================================================================*/
/**
  Tell LPASS to stop transmitting RX pattern
  @return 0 on success, other value on failure
*/
int p2p_stop_rx_in_dsp(us_all_info *paramsStruct)
{
  int ret = 0;

  // We can't really stop the RX, instead we send a "silence" pattern
  memset(p2pContext.m_pattern, 0, p2pContext.m_pattern_size);
  return p2p_send_rx_pattern_to_dsp(paramsStruct);
}


/*==============================================================================
  FUNCTION:  p2p_process_lib_request
==============================================================================*/
/**
  Process requests from P2P library. Mainly related to
  start/stop RX and update RX pattern.
  Note if library is running on LPASS these requests will be handled in
  the LPASS and not passed here.
*/
int p2p_process_lib_request(us_all_info *paramsStruct, P2POutput& p2p_output)
{
  int ret = 1;
  uint16_t request = p2p_output.request;
  switch (request)
  {
  case QC_US_P2P_REQUEST_NONE:
      ret = 0;
      break;
  case QC_US_P2P_REQUEST_UPDATE_RX:
    ret = p2p_get_new_rx_pattern(paramsStruct);
    if (ret == 0)
    {
      if (p2pContext.m_rx_started)
      {
        ret = p2p_send_rx_pattern_to_dsp(paramsStruct);
      }
    }
    break;
  case QC_US_P2P_REQUEST_START_RX:
    if (p2pContext.m_rx_started)
    {
      LOGW("%s: RX already started, start request ignored", __FUNCTION__);
    }
    else
    {
      ret = p2p_send_rx_pattern_to_dsp(paramsStruct);
      if (ret == 0)
      {
        p2pContext.m_rx_started = true;
      }
    }
    break;
  case QC_US_P2P_REQUEST_STOP_RX:
    if (!p2pContext.m_rx_started)
    {
      LOGW("%s: RX already stopped, stop request ignored", __FUNCTION__);
    }
    else
    {
      ret = p2p_stop_rx_in_dsp(paramsStruct);
    }
    break;
  case QC_US_P2P_REQUEST_UPDATE_AND_START_RX:
    ret = p2p_get_new_rx_pattern(paramsStruct);
    if (ret == 0)
    {
      ret = p2p_send_rx_pattern_to_dsp(paramsStruct);
    }
    break;
  }

  return ret;
}

/*==============================================================================
  FUNCTION:  get_p2p_output_decision
==============================================================================*/
/**
  Get decision from suitable p2p libraries and return final
  result decision.
  TODO this function need to be reworked for P2P
*/
void get_p2p_output_decision(short *packet,
                                 us_all_info *paramsStruct,
                                 P2POutput& output)
{
  output.request = QC_US_P2P_REQUEST_NONE;
  output.result = QC_US_P2P_RESULT_IDLE;

  if (paramsStruct->usf_output_type & OUTPUT_TYPE_P2P_EVENT_MASK)
  { // Events handling
    struct p2p_dsp_event *dsp_event = (struct p2p_dsp_event *) packet;
    output = dsp_event->result;
    // Update pointer to point to next raw data place
    packet += sizeof(struct p2p_dsp_event) / sizeof(short);
  }
  else if (paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK)
  { // Raw data handling, only if no DSP processing
    int rc = QcUsP2PLibEngine(packet,
                              &output);
    if (rc)
    {
      LOGE("%s: QcUsP2PLibEngine failed.",
           __FUNCTION__);
      p2p_exit(EXIT_FAILURE);
    }
  }
}

/*==============================================================================
  FUNCTION:  p2p_send_socket_event
==============================================================================*/
static void p2p_send_socket_event(us_all_info *paramsStruct, P2POutput& p2p_output)
{
  int ret;
  uint32_t i;
  uint32_t peerId = 0;

  for (i=0; i<paramsStruct->usf_p2p_num_users; i++)
  {
      // skip own user
      if (i != paramsStruct->usf_p2p_user_index)
      {
          // DEBUGGING
          LOGI("%s: send_p2p_event userIndex %d peerId %d angle %d distance %d",
               __FUNCTION__, (int)paramsStruct->usf_p2p_user_index, (int)peerId,
               (int)p2p_output.usersStatus[i].angle, (int)p2p_output.usersStatus[i].distance);
          ret = m_p2p_data_socket->send_p2p_event(paramsStruct->usf_p2p_user_index,
                                                  (int)peerId,
                                                  p2p_output.usersStatus[i].angle,
                                                  p2p_output.usersStatus[i].distance);
          if (0 > ret)
          { // If we got here there is some problem in sending points to socket.
            // The m_socket_sending_prob starts from SOCKET_PROBLEM_MSG_INTERVAL
            // and only when it gets to 0 a warning msg is shown to the user and
            // m_socket_sending_prob set to SOCKET_PROBLEM_MSG_INTERVAL again.
            // exit the daemon on EPIPE error, this means the remote app was disconnected,
            // TODO instead of exiting the daemon we should restart the socket
            if (ret == -EPIPE)
            {
              LOGE("%s: socket broken, exiting daemon", __FUNCTION__);
              p2p_exit(EXIT_FAILURE);
            }
            p2pContext.m_socket_sending_prob--;

            if (0 == p2pContext.m_socket_sending_prob)
            {
              LOGW("%s: Sending p2p event to socket failed.", __FUNCTION__);
              p2pContext.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;
            }
          }
          else
          {
            p2pContext.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;
          }
      }

      peerId++;
  }
}

/*==============================================================================
  FUNCTION:  p2p_get_points
==============================================================================*/
/**
  Call QcP2PAlgorithmEngine() from P2P lib.
  Returns eventCounter for number of points goes to UAL.
*/
int p2p_get_points(short *packet,
                       us_all_info *paramsStruct)
{
  int     rc = 1;
  P2POutput p2p_output;
  bool has_event;

  get_p2p_output_decision(packet, paramsStruct, p2p_output);

  has_event = p2p_output.result || p2p_output.request;
  if (has_event)
  {
    LOGD("%s: p2p[result=%d,request=%d]",
         __FUNCTION__,
         p2p_output.result, p2p_output.request);
  }

  // TODO need to rework the event type to send P2P events
  int mapped_gesture = 0;

  // Send socket event
  if (p2pContext.m_send_points_to_socket && (m_p2p_data_socket != NULL))
  {
    if (p2p_output.result)
    {
      p2p_send_socket_event(paramsStruct, p2p_output);
      p2pContext.m_last_lib_result = p2p_output;
      p2pContext.m_socket_keep_alive_counter = 0;
    }
    else
    {
      p2pContext.m_socket_keep_alive_counter++;
      if (p2pContext.m_socket_keep_alive_counter > SOCKET_MAX_ITERATIONS_FOR_KEEP_ALIVE)
      {
        // we send events to socket regularly to keep it alive. send last reported result
        p2p_send_socket_event(paramsStruct, p2pContext.m_last_lib_result);
        p2pContext.m_socket_keep_alive_counter = 0;
      }
    }
  }

  // process request from library, mainly related to RX update
  if(p2p_process_lib_request(paramsStruct, p2p_output))
  {
    LOGW("%s: failed to process p2p lib request", __FUNCTION__);
  }

  return s_eventCounter;
}

/*==============================================================================
  FUNCTION:  update_pattern
==============================================================================*/
/**
  Writes pattern to the DSP
  TODO need to enhance for P2P. P2P library needs the ability to start/stop RX,
  and update RX pattern while running
*/
void update_pattern()
{
  LOGD("%s: Update pattern.",
       __FUNCTION__);

  // Pattern is transmitted only once. DSP transmits pattern in loop.
  int rc = ual_write((uint8_t *)p2pContext.m_pattern,
                     p2pContext.m_pattern_size);
  if (1 != rc)
  {
    LOGE("%s: ual_write failed.",
         __FUNCTION__);
    p2p_exit(EXIT_FAILURE);
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
  FUNCTION:  p2p_fill_p2p_lib_configuration
==============================================================================*/
/**
  Fill a configuration structure for P2P library init, based
  on configuration file
  cfg must not be NULL
*/
void p2p_fill_p2p_lib_configuration(us_all_info *paramsStruct, P2PCfg* cfg)
{
  cfg->numChannels = paramsStruct->usf_tx_port_count;

  if (-1 == ual_util_prefill_ports_num_and_id(paramsStruct))
  {
    LOGE("%s: ual_util_prefill_ports_num_and_id failed.",
         __FUNCTION__);
    p2p_exit(EXIT_FAILURE);
  }

  float mic_info[COORDINATES_DIM];
  for (int i = 0; i < paramsStruct->usf_tx_port_count; i++)
  {
    // Get current mic info
    if (-1 == ual_util_get_mic_config(i, mic_info))
    {
      LOGE("%s: get_mic_config for mic failed for mic #%d.",
           __FUNCTION__,
           i);
      p2p_exit(EXIT_FAILURE);
    }

    cfg->micPositionsX[i] = (int32_t)mic_info[0];
    cfg->micPositionsY[i] = (int32_t)mic_info[1];
    cfg->micPositionsZ[i] = (int32_t)mic_info[2];
  }

  cfg->appType = paramsStruct->usf_p2p_app_type;
  cfg->posAlgType = paramsStruct->usf_p2p_pos_alg_type;
  cfg->dataAlgType = paramsStruct->usf_p2p_data_alg_type;
  cfg->numUsers = paramsStruct->usf_p2p_num_users;
  cfg->userIdx = paramsStruct->usf_p2p_user_index;
  cfg->samplesPerFrame = paramsStruct->usf_p2p_samples_per_frame;
  for (int i=0; i<cfg->numUsers; i++)
  {
    cfg->sequenceIdx[i] = paramsStruct->usf_p2p_sequence_index[i];
  }
  cfg->firstBin = paramsStruct->usf_p2p_first_bin;
  cfg->lastBin = paramsStruct->usf_p2p_last_bin;
  cfg->resamplerFreqPpm = paramsStruct->usf_p2p_resampler_freq_ppm;
  cfg->fftSize = paramsStruct->usf_p2p_fft_size;
  cfg->rxPatternMode = paramsStruct->usf_p2p_rx_pattern_mode;
  cfg->p2aThreshold = paramsStruct->usf_p2p_p2a_threshold;
  cfg->losWindowLenRatio = paramsStruct->usf_p2p_los_window_len_ratio;
  cfg->losPeakThreasholdRatio = paramsStruct->usf_p2p_los_peak_threashold_ratio;
  cfg->libraryMode = paramsStruct->usf_p2p_library_mode;
}

/*==============================================================================
  FUNCTION:  inject_config_params_to_tx_transparent_data
==============================================================================*/
/**
  Injects mic distances, output type and group factor to the tx transparent data.
  For sync free form (sff)
*/
void inject_sff_config_params_to_tx_transparent_data(us_all_info *paramsStruct)
{
  // Inject skip and group factor to transparent data
  uint32_t skip_index = BYTE_OFFSET_OF_SKIP_IN_TX_TRANSPARENT;
  uint32_t group_index = BYTE_OFFSET_OF_GROUP_IN_TX_TRANSPARENT;
  uint32_t samples_per_frame_index = BYTE_OFFSET_OF_SAMPLES_PER_FRAME_IN_TX_TRANSPARENT;

  if (ual_util_inject_to_trans_data(paramsStruct->usf_tx_transparent_data,
                                    &skip_index,
                                    FILE_PATH_MAX_LEN,
                                    paramsStruct->usf_tx_skip,
                                    sizeof(uint16_t)))
  {
    p2p_exit(EXIT_FAILURE);
  }
  if (ual_util_inject_to_trans_data(paramsStruct->usf_tx_transparent_data,
                                    &group_index,
                                    FILE_PATH_MAX_LEN,
                                    paramsStruct->usf_tx_group,
                                    sizeof(uint16_t)))
  {
    p2p_exit(EXIT_FAILURE);
  }

  if (ual_util_inject_to_trans_data(paramsStruct->usf_tx_transparent_data,
                                    &samples_per_frame_index,
                                    FILE_PATH_MAX_LEN,
                                    paramsStruct->usf_p2p_samples_per_frame,
                                    sizeof(uint32_t)))
  {
    p2p_exit(EXIT_FAILURE);
  }


  float mic_info[COORDINATES_DIM], spkr_info[COORDINATES_DIM];
  // Required for ual_util_get_mic_config & ual_util_get_speaker_config
  if (-1 == ual_util_prefill_ports_num_and_id(paramsStruct))
  {
    LOGE("%s: ual_util_prefill_ports_num_and_id failed.",
         __FUNCTION__);
    p2p_exit(EXIT_FAILURE);
  }

  // Get speaker info - 0 is for the first spkr since we only have one spkr
  if (-1 == ual_util_get_speaker_config(0, spkr_info))
  {
    LOGE("%s: ual_util_get_speaker_config for speaker failed.",
         __FUNCTION__);
    p2p_exit(EXIT_FAILURE);
  }

  for (int i = 0; i < paramsStruct->usf_tx_port_count; i++)
  {
    // Get current mic info
    if (-1 == ual_util_get_mic_config(i, mic_info))
    {
      LOGE("%s: get_mic_config for mic failed for mic #%d.",
           __FUNCTION__,
           i);
      p2p_exit(EXIT_FAILURE);
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

	// override distance = 0 for p2p, it needs to preserve offsets between microphones
	distance = 0;

    // Update TX transparent data
    if (ual_util_inject_to_trans_data(paramsStruct->usf_tx_transparent_data,
                                      &(paramsStruct->usf_tx_transparent_data_size),
                                      FILE_PATH_MAX_LEN,
                                      distance,
                                      sizeof(int)))
    {
      p2p_exit(EXIT_FAILURE);
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
      p2p_exit(EXIT_FAILURE);
    }
  }

  LOGD("%s: Transparent after injection: size: %d content:",
       __FUNCTION__,
       paramsStruct->usf_tx_transparent_data_size);
  print_transparent_data(*paramsStruct,
                         paramsStruct->usf_tx_transparent_data_size);
}

/*==============================================================================
  FUNCTION:  inject_config_params_to_tx_transparent_data
==============================================================================*/
/**
  Injects P2P configuration to TX tranparent data
*/
void inject_config_params_to_tx_transparent_data(us_all_info *paramsStruct)
{
  P2PExternalCfg extCfg;
  uint32_t size = sizeof(extCfg);
  P2PCfg *cfg = &extCfg.libConfig;

  // verify our transparent data is not too large
  if (size > MAX_TRANSPARENT_DATA_SIZE)
  {
     LOGE("%s: configuration is too large for transparent data. Max %d need %u",
           __FUNCTION__, MAX_TRANSPARENT_DATA_SIZE, size);
     p2p_exit(EXIT_FAILURE);
  }
  // fill the external cfg structure from parameters, it matches the
  // transparent data structure.
  extCfg.skipFactor = paramsStruct->usf_tx_skip;
  extCfg.groupingFactor = paramsStruct->usf_tx_group;
  extCfg.outputType = paramsStruct->usf_output_type;

  // fill the library configuration part
  p2p_fill_p2p_lib_configuration(paramsStruct, cfg);

  // Inject the transparent data
  memcpy(paramsStruct->usf_tx_transparent_data, &extCfg, size);
  paramsStruct->usf_tx_transparent_data_size = size;

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
  if (paramsStruct->usf_output_type & OUTPUT_TYPE_P2P_EVENT_MASK)
  { // Getting sequence number from DSP event
    seqNum = *((int *)(nextFrame + DSP_EVENT_SEQNUM_OFFSET));
  }
  else
  { // Getting sequence number from raw data header file
    seqNum = *((int *)(nextFrame + ECHO_FRAME_SEQNUM_OFFSET));
  }
  // If this is the first iteration then the frames
  // counter is -1 and we need to update the frames counter.
  if (p2pContext.m_next_frame_seq_num == -1)
  {
    p2pContext.m_next_frame_seq_num = seqNum;
  }
  // This is not the first iteration.
  else
  {
    if (p2pContext.m_next_frame_seq_num != seqNum)
    {
      // We lost some frames so we add the number of lost frames
      // to the statistics.
      if (p2pContext.m_next_frame_seq_num < seqNum)
      {
        p2pStats.m_nLostFrames +=
          (seqNum - p2pContext.m_next_frame_seq_num)/
          paramsStruct->usf_tx_skip;
      }
      // We got out of order frames so we add the number of
      // out of order frames to the statistics.
      else
      {
        p2pStats.m_nOutOfOrderErrors +=
          (p2pContext.m_next_frame_seq_num - seqNum)/
          paramsStruct->usf_tx_skip;
      }

      // Update the frames counter to the correct count.
      p2pContext.m_next_frame_seq_num = seqNum;
    }
  }
  p2pStats.m_nTotalFrames++;
  // Update the frames counter to the expected count in the next
  // iteration.
  p2pContext.m_next_frame_seq_num += paramsStruct->usf_tx_skip;

  *packetCounter = *packetCounter + 1;
  if (STATISTIC_PRINT_INTERVAL == *packetCounter)
  {
    LOGI("%s: Statistics (printed every %d frames):",
         __FUNCTION__,
         STATISTIC_PRINT_INTERVAL);
    LOGI("Points calculated: %d, total frames: %d, lost frames: %d,"
         "out of order: %d",
         p2pStats.m_nPointsCalculated,
         p2pStats.m_nTotalFrames,
         p2pStats.m_nLostFrames,
         p2pStats.m_nOutOfOrderErrors);
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
  FUNCTION:  p2p_init
==============================================================================*/
/**
  Inits the p2p daemon parameters, signal handlers etc`.
*/
void p2p_init(us_all_info *paramsStruct)
{
  // Setup signal handling
  setup_signal_handlers();

  if (false == ual_util_is_supported((char *)CLIENT_NAME))
  {
    LOGE("%s: Daemon is not supported",
         __FUNCTION__);
    p2p_exit(EXIT_FAILURE);
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
    p2p_exit(EXIT_FAILURE);
  }

  ual_util_print_US_version(CLIENT_NAME,
                            CLIENT_VERSION);

  // TODO we currently use free form (echo) until P2P service is implemented
  //inject_config_params_to_tx_transparent_data(paramsStruct);
  if(paramsStruct->usf_tx_data_format == 1)
  {
	// free form
    ual_util_set_echo_tx_transparent_data(paramsStruct);
  }
  else if(paramsStruct->usf_tx_data_format == 4)
  {
    // sync free form
    inject_sff_config_params_to_tx_transparent_data(paramsStruct);
  }
  else
  {
    LOGE("%s: unsupported data format %d", __FUNCTION__, paramsStruct->usf_tx_data_format);
	p2p_exit(EXIT_FAILURE);
  }


  ual_util_set_buf_size(&paramsStruct->usf_tx_buf_size,
                        paramsStruct->usf_tx_port_data_size,
                        paramsStruct->usf_tx_sample_width,
                        paramsStruct->usf_tx_port_count,
                        paramsStruct->usf_tx_frame_hdr_size,
                        paramsStruct->usf_tx_group,
                        "tx",
                        (paramsStruct->usf_output_type & OUTPUT_TYPE_P2P_EVENT_MASK),
                        (paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK),
                        sizeof(struct p2p_dsp_event));

  // Build rx_transparent_data manually. In the future (hopefully),
  // this will be removed.
  // TODO check if this is needed for P2P
  ual_util_set_echo_rx_transparent_data(paramsStruct);

  ual_util_set_rx_buf_size(paramsStruct);
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
      p2p_exit(EXIT_FAILURE);
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

    if (paramsStruct->usf_output_type & OUTPUT_TYPE_P2P_EVENT_MASK)
    {
      // Skip dsp events when recording
      pGroupData += sizeof(struct p2p_dsp_event);
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
  FUNCTION:  p2p_run
==============================================================================*/
/**
  Main running loop of the p2p daemon.
*/
void p2p_run(us_all_info *paramsStruct)
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
  if (paramsStruct->usf_output_type & OUTPUT_TYPE_P2P_EVENT_MASK)
  {
    combined_frame_size += sizeof(struct p2p_dsp_event);
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
    uint8_t* nextFrame = NULL;

    uint32_t timeout = USF_DEFAULT_TIMEOUT;
    if (!(paramsStruct->usf_output_type & OUTPUT_TYPE_RAW_MASK))
    { // In events only mode, need to wait infinitely for events.
      timeout = USF_INFINITIVE_TIMEOUT;
    }

    if (!ual_read(&data,
                  NULL,
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
          p2p_get_points ((short *)nextFrame,
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
 if (ual_util_ual_open_retries(paramsStruct))
 {
   p2p_exit(EXIT_FAILURE);
 }

 // TODO adapt for P2P
 paramsStruct->usf_event_type = (paramsStruct->usf_gesture_event_dest & DEST_UAL) ?
                               USF_KEYBOARD_EVENT :
                               USF_NO_EVENT;

 if (ual_util_tx_config(paramsStruct,
                        (char *)CLIENT_NAME))
 {
   LOGE("%s: ual_util_tx_config failed.",
        __FUNCTION__);
   p2p_exit(EXIT_FAILURE);
 }

 if (ual_util_rx_config(paramsStruct,
                        (char* )CLIENT_NAME))
 {
   LOGE("%s: ual_util_rx_config failed.",
        __FUNCTION__);
   p2p_exit(EXIT_FAILURE);
 }

 p2p_params_init(paramsStruct);
 p2p_lib_init(paramsStruct);
 // must send RX pattern otherwise TX-RX sync will not work and TX will never start
 // library can supply a 0s pattern to transmit silence, but TX-RX sync will not work.
 (void)p2p_get_new_rx_pattern(paramsStruct);
 (void)p2p_send_rx_pattern_to_dsp(paramsStruct);
}

/*==============================================================================
  FUNCTION:  main
==============================================================================*/
/**
  Main function of the P2P daemon. Handle all the P2P operations.
*/
int main (void)
{
  int ret;
  static us_all_info paramsStruct;
  bool rc = false;

  LOGI("%s: P2P start",
       __FUNCTION__);

  p2p_init(&paramsStruct);

  while (daemon_run)
  {
    // Clear pending alarm signal that might disrupt this function work.  Alarm
    // signal is used to release daemons from blocking functions.
    alarm(0);

    init(&paramsStruct);

    p2p_run(&paramsStruct);

    if (true == daemon_run)
    { // Received deactivate
      // Free resources before next activate
      p2p_free_resources(EXIT_SUCCESS);
    }
  } // End daemon_run
  p2p_exit(EXIT_SUCCESS);
}

