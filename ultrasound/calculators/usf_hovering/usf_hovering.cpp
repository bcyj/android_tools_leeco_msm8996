/*===========================================================================
                           usf_hovering.cpp

DESCRIPTION: Implementation of the Hovering daemon.


INITIALIZATION AND SEQUENCING REQUIREMENTS:
  If not started through java app then make sure to have
  correct /data/usf/hovering/usf_hovering.cfg file linked to the wanted cfg file
  placed in /data/usf/hovering/cfg/.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_hovering"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <ual.h>
#include <cutils/properties.h>
#include <ual_util.h>
#include <HoveringExports.h>
#include <stdlib.h>
#include <errno.h>
#include "ual_util_frame_file.h"
#include <usf_unix_domain_socket.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define LINK_CFG_FILE_LOCATION "/data/usf/hovering/usf_hovering.cfg"
#define USF_DSP_VER_FILE "/data/usf/hovering/usf_dsp_ver.txt"
#define FRAME_FILE_DIR_PATH "/data/usf/hovering/rec/"
#define PATTERN_DIR_PATH "/data/usf/hovering/pattern/"

#define BUFFER_SIZE                 500
#define US_MAX_EVENTS               100
#define STATISTIC_PRINT_INTERVAL    500
#define SOCKET_PROBLEM_MSG_INTERVAL 50
#define EXIT_SIGTERM                3
#define PIX_PER_MM                  6.135
// EPSILON (mm Units)
#define EPSILON                     1
// Since the Z we get from the lib is very small, we multiply it by this define
#define CMM_PER_MM                  100

enum hovering_event_dest
{
  DEST_UAL = 0x01,
  DEST_SOCKET = 0x02,
};

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
  hoveringParams holds information needed for Hovering calculation.
*/
typedef struct
{
  signed char       *m_hovering_workspace;
  usf_event_type    m_events[US_MAX_EVENTS];  // Array of struct from sys/stat.h
  bool              m_send_points_to_ual;
  bool              m_send_points_to_socket;
  int               m_nNextFrameSeqNum;
  int               m_socket_sending_prob;
  uint8_t*          m_pPattern;
  int               m_patternSize;
  bool              m_stub;
  int               m_nLastEventSeqNum;
  float             m_fuzz[3];
} HoveringParams;

/**
  HoveringStats holds information about statistics from
  usf_hovering running.
*/
typedef struct
{
  int   m_nPointsCalculated;
  int   m_nTotalFrames;
  int   m_nLostFrames;
  int   m_nOutOfOrderErrors;
  int   numPoints;
} HoveringStats;

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Ptr to the cfg file. The ptr is global because we want to
  close the file before exit in the function hovering_exit.
  This function is called also from the signal_handler function
  which doesn't know the cfg file.
*/
static FILE *cfgFile = NULL;


/**
  hoveringParams will hold all the information needed for
  Hovering calculation.
*/
static HoveringParams hoveringParams;


/**
  hoveringStats will hold all the statistics needed.
*/
static HoveringStats hoveringStats;

/**
  m_hovering_data_socket is pointer to the thread which handles
  data socket communication with the service.
*/
static DataUnSocket *m_hovering_data_socket;


const uint16_t ECHO_FRAME_SEQNUM_OFFSET = 8; // bytes

/**
  Hovering calculator name
*/
static const char* CLIENT_NAME =  "hovering";

/**
  Hovering calculator version
*/
static const char* CLIENT_VERSION = "1.3.2";

/**
 * The name of the file containg the pid of the daemon
 */
static const char* PID_FILE_NAME = "usf_hovering.pid";

/**
  The daemon running control
*/
static volatile bool sb_run = true;

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  hovering_exit
==============================================================================*/
/**
  Perform clean exit of the daemon.
*/
int hovering_exit (int status)
{
  QcUsHoveringLibTerminate(hoveringParams.m_hovering_workspace);

  bool error_state = (status != EXIT_SUCCESS);
  int rc = ual_close(error_state);
  LOGD("%s: ual_close: rc=%d;",
       __FUNCTION__,
       rc);

  if (NULL != cfgFile)
  {
    fclose(cfgFile);
    cfgFile = NULL;
  }

  if (NULL != hoveringParams.m_hovering_workspace)
  {
    free(hoveringParams.m_hovering_workspace);
    hoveringParams.m_hovering_workspace = NULL;
  }

  if (NULL != hoveringParams.m_pPattern)
  {
    free(hoveringParams.m_pPattern);
    hoveringParams.m_pPattern = NULL;
  }

  if (NULL != m_hovering_data_socket)
  {
    delete m_hovering_data_socket;
    m_hovering_data_socket = NULL;
  }

  LOGI("%s: Hovering end. status=%d",
       __FUNCTION__,
       status);

  int ret = ual_util_remove_declare_pid(PID_FILE_NAME);
  if (0 != ret)
  {
    LOGW("%s: Removing pid file failed",
         __FUNCTION__);
  }

  // Must update flag, so that init would not restart the daemon.
  ret = property_set("ctl.stop",
                         "usf_hovering");
  if (0 != ret)
  {
    LOGW("%s: property_set failed",
         __FUNCTION__);
  }


  exit(status);
}

/*==============================================================================
  FUNCTION:  hovering_params_init
==============================================================================*/
/**
  Init hoveringParam struct.
*/
void hovering_params_init (us_all_info *paramsStruct)
{
  uint32_t port;
  char *temp = NULL, *ip = NULL;
  int ret;

  hoveringParams.m_hovering_workspace = NULL;

  hoveringParams.m_patternSize = paramsStruct->usf_rx_pattern_size *
                                 paramsStruct->usf_rx_sample_width/BYTE_WIDTH;

  hoveringParams.m_stub = false;

  hoveringParams.m_pPattern = (uint8_t *) malloc(hoveringParams.m_patternSize);

  if (NULL == hoveringParams.m_pPattern)
  {
    LOGE("%s: Failed to allocate %d bytes",
         __FUNCTION__,
         paramsStruct->usf_rx_pattern_size *
         sizeof(paramsStruct->usf_rx_sample_width));
    hovering_exit(EXIT_FAILURE);
  }

  if (paramsStruct->usf_hovering_event_dest & DEST_UAL)
  {
    hoveringParams.m_send_points_to_ual = true;
  }
  else
  {
    hoveringParams.m_send_points_to_ual = false;
  }

  if (paramsStruct->usf_hovering_event_dest & DEST_SOCKET)
  {
    hoveringParams.m_send_points_to_socket = true;
  }
  else
  {
    hoveringParams.m_send_points_to_socket = false;
  }


  // FIXME: this code is copy pasted.
  ret = sscanf(paramsStruct->usf_fuzz_params,
               "%f ,%f ,%f",
               &hoveringParams.m_fuzz[X_IND],
               &hoveringParams.m_fuzz[Y_IND],
               &hoveringParams.m_fuzz[Z_IND]);
  if (3 != ret)
  {
    LOGW("%s: usf_fuzz_params should have three dims, ret from scanf=%d",
         __FUNCTION__,
         ret);
    hoveringParams.m_fuzz[X_IND] = 0;
    hoveringParams.m_fuzz[Y_IND] = 0;
    hoveringParams.m_fuzz[Z_IND] = 0;
  }

  hoveringParams.m_nNextFrameSeqNum = -1;

  hoveringParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;

  hoveringStats.m_nPointsCalculated = 0;
  hoveringStats.m_nTotalFrames = 0;
  hoveringStats.m_nLostFrames = 0;
  hoveringStats.m_nOutOfOrderErrors = 0;
  hoveringStats.numPoints = 0;
  hoveringParams.m_nLastEventSeqNum = 0;

  if (hoveringParams.m_send_points_to_socket)
  {
    m_hovering_data_socket =
      new DataUnSocket("/data/usf/hovering/data_socket");

    if (0 != m_hovering_data_socket->start())
    {
      LOGE("%s: Starting data socket failed.",
           __FUNCTION__);
      hovering_exit(EXIT_FAILURE);
    }
  }
}

/*==============================================================================
  FUNCTION:  hovering_init
==============================================================================*/
/**
  Init Hovering resources.
*/
void hovering_init (us_all_info *paramsStruct)
{
  // Allocate memory for Hovering algorithm.
  int hovering_workspace_size = 0;
  int mic, spkr, dim;
  float mics_info[US_FORM_FACTOR_CONFIG_MAX_MICS][COORDINATES_DIM] = {{0}};
  float spkrs_info[US_FORM_FACTOR_CONFIG_MAX_SPEAKERS][COORDINATES_DIM] = {{0}};
  float mics_coords[COORDINATES_DIM][US_FORM_FACTOR_CONFIG_MAX_MICS] = {{0}};
  float speaks_coord[COORDINATES_DIM][US_FORM_FACTOR_CONFIG_MAX_SPEAKERS] = {{0}};

  QcUsHoveringLibGetSizes(&hovering_workspace_size);
  hoveringParams.m_hovering_workspace =
    (signed char *)malloc(hovering_workspace_size * sizeof(signed char));
  if (NULL == hoveringParams.m_hovering_workspace)
  {
    LOGE("%s: Failed to allocate %d bytes.",
         __FUNCTION__,
         hovering_workspace_size);
    hovering_exit(EXIT_FAILURE);
  }

  int mics_num = paramsStruct->usf_tx_port_count;
  for (mic = 0; mic < mics_num; mic++)
  {
    if (-1 == ual_util_get_mic_config (mic, mics_info[mic]))
    {
      LOGE("%s: get_mic_config for mic %d failed.",
           __FUNCTION__,
           mic);
      hovering_exit(EXIT_FAILURE);
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
      hovering_exit(EXIT_FAILURE);
    }
  }

  // Fiting mics and speakers info to the Hovering lib API
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

  LOGD("mics_num=%d; Mic 1 = (%.5f, %.5f), Mic 2 = (%.5f, %.5f), Mic 3 = (%.5f, %.5f),"
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

  int rc = QcUsHoveringLibInit(hoveringParams.m_hovering_workspace,
                               mics_num,
                               mics_coords[0],
                               mics_coords[1],
                               mics_coords[2],
                               spkr_num,
                               speaks_coord[0],
                               speaks_coord[1],
                               speaks_coord[2],
                               paramsStruct->usf_tx_port_data_size,
                               paramsStruct->usf_rx_port_data_size);

  if (rc)
  {
    LOGE("%s: AlgorithmInit failed.",
         __FUNCTION__);
    hovering_exit(EXIT_FAILURE);
  }

  LOGD("%s: Hovering lib init completed.",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  add_event_point
============================================================================*/
/**
  Creates event and add it to the HoveringParams.m_events[].
  FIXME: this code is copy pasted.
*/
static bool add_event_point
(
  float x,
  float y,
  float z
)
{
  // Fill in the usf_event_type struct
  if (US_MAX_EVENTS <= hoveringStats.numPoints)
  {
    LOGE("%s: No more events to send max counter=%d",
         __FUNCTION__,
         US_MAX_EVENTS);
    return false;
  }

  usf_event_type *pEvent = &hoveringParams.m_events[hoveringStats.numPoints++];

  pEvent->seq_num = hoveringParams.m_nLastEventSeqNum++;
  pEvent->timestamp = (uint32_t)(clock());
  pEvent->event_type_ind = USF_TSC_EVENT_IND;
  // No pressure while hovering
  pEvent->event_data.point_event.pressure = 0;
  // Since coords are in MM we change them to PIX
  pEvent->event_data.point_event.coordinates[X_IND] = (double)x*PIX_PER_MM;
  pEvent->event_data.point_event.coordinates[Y_IND] = (double)y*PIX_PER_MM;
  pEvent->event_data.point_event.coordinates[Z_IND] = z*CMM_PER_MM;
  pEvent->event_data.point_event.inclinations[X_IND] = 0;
  pEvent->event_data.point_event.inclinations[Y_IND] = 0;

  return true;
}

/*==============================================================================
  FUNCTION:  is_diff_point
==============================================================================*/
/**
  FIXME: this code is copy pasted.
  Returns true if the given point should be sent as an event, this is determined
  by:
    1) the provided point is out of previous point fuzz cube
    2) the provided point have z > 0 coordinate (because it's a hover event)
  NOTE: Points are in mm
*/
static inline bool is_diff_point
(
  float x,
  float y,
  float z
)
{
  // by initing pz to -1, we indicate that this is the first time this function is called.
  static float px, py, pz = -1;
  bool rc = false;

  if ( // Fuzzing
      ((abs(x - px) >= hoveringParams.m_fuzz[X_IND]) ||
       (abs(y - py) >= hoveringParams.m_fuzz[Y_IND]) ||
       (abs(z - pz) >= hoveringParams.m_fuzz[Z_IND]) ||
       -1 == pz))
  {
    rc = true;
    px = x;
    py = y;
    pz = z;
  }

  return rc;
}

/*==============================================================================
  FUNCTION:  hovering_get_points
==============================================================================*/
/**
  Call QcUsHoveringLibEngine() from Hovering lib.
  Returns 0 for number of points goes to UAL.
*/
int hovering_get_points(short *pPacket)
{
  int     rc = 0;
  float   fX = 0;
  float   fY = 0;
  float   fZ = 0;
  char    cOutputValid = 0;
  int     iPatternUpdate = 0;

  QcUsHoveringLibEngine(pPacket,
                        (short int *) hoveringParams.m_pPattern,
                        &fX,
                        &fY,
                        &fZ,
                        &cOutputValid,
                        0,
                        &iPatternUpdate);

  if (hoveringParams.m_stub)
  {
    return 0;
  }

  // If pPacket is NULL then we try to update pattern from Hovering lib
  // for the first time (and not from pattern file received from cfg file).
  if ((NULL == pPacket) && (1 != iPatternUpdate))
  {
    LOGE("%s: QcUsHoveringLibEngine failed.",
         __FUNCTION__);
    hovering_exit(EXIT_FAILURE);
  }

  // Update pattern on runtime by Hovering lib.
  if (1 == iPatternUpdate)
  {
    if ((NULL == hoveringParams.m_pPattern))
    {
      LOGE("%s: QcUsHoveringLibEngine failed.",
           __FUNCTION__);
      hovering_exit(EXIT_FAILURE);
    }
    else
    {
      LOGD("%s: Update pattern from Hovering lib.",
           __FUNCTION__);

      // Pattern is transmitted only once. DSP transmits pattern in loop.
      rc = ual_write(hoveringParams.m_pPattern,
                     hoveringParams.m_patternSize);
      if (1 != rc)
      {
        LOGE("%s: ual_write failed.",
             __FUNCTION__);
        hovering_exit(EXIT_FAILURE);
      }
    }
  }
  if (!is_diff_point(fX,
                     fY,
                     fZ))
  { // Fuzzing filter
    return 0;
  }
  if (hoveringParams.m_send_points_to_socket)
  {
    // Points from Hovering lib are in mm.
    rc = m_hovering_data_socket->send_hovering_event(cOutputValid,
                                                     fX,
                                                     fY,
                                                     fZ);
    if (0 > rc)
    {
      // If we got here there is some problem in sending hovering to socket.
      // The m_socket_sending_prob starts from SOCKET_PROBLEM_MSG_INTERVAL
      // and only when it gets to 0 a warning msg is shown to the user and
      // m_socket_sending_prob set to SOCKET_PROBLEM_MSG_INTERVAL again.
      hoveringParams.m_socket_sending_prob--;
      if (0 == hoveringParams.m_socket_sending_prob)
      {
        LOGW("%s: SendHover() failed.",
             __FUNCTION__);
        hoveringParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;
      }
    }
    else
    {
      hoveringParams.m_socket_sending_prob = SOCKET_PROBLEM_MSG_INTERVAL;
    }

    hoveringStats.m_nPointsCalculated++;
  }

  if (hoveringParams.m_send_points_to_ual)
  {
    // Points from Hovering lib are in mm.
    if (!add_event_point(fX,
                         fY,
                         fZ))
    {
      LOGE("%s: Could not send event for hovering", __FUNCTION__);
    }
  }

  return 0;
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
    hovering_exit(EXIT_FAILURE);
  }

  QcUsHoveringLibGetVersion(szVersion,
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
    hoveringParams.m_stub = true;
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
}

/*==============================================================================
  FUNCTION:  main
==============================================================================*/
/**
  Main function of the Hovering daemon. Handle all the Hovering operations.
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

  LOGW("%s: Hovering start",
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

  if (false == ual_util_is_supported((char *)CLIENT_NAME))
  {
    LOGE("%s: Daemon is not supported",
         __FUNCTION__);
    hovering_exit(EXIT_FAILURE);
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
    hovering_exit(EXIT_FAILURE);
  }

  ual_cfg_type cfg;
  cfg.usf_dev_id = 1;
  cfg.ual_mode = static_cast<ual_work_mode_type>(paramsStruct.ual_work_mode);
  rc = ual_open(&cfg);

  if (1 != rc)
  {
    LOGE("%s: ual_open failed.",
         __FUNCTION__);
    hovering_exit(EXIT_FAILURE);
  }

  ual_util_print_US_version(CLIENT_NAME, CLIENT_VERSION);

  paramsStruct.usf_event_type = USF_TSC_EVENT;

  // Build tx_transparent_data manually. In the future (hopefully),
  // this will be removed.
  ual_util_set_echo_tx_transparent_data(&paramsStruct);

  ual_util_set_tx_buf_size(&paramsStruct);

  if (ual_util_tx_config(&paramsStruct,
                         (char *)CLIENT_NAME))
  {
    LOGE("%s: ual_util_tx_config failed.",
         __FUNCTION__);
    hovering_exit(EXIT_FAILURE);
  }

  // Build rx_transparent_data manually. In the future (hopefully),
  // this will be removed.
  ual_util_set_echo_rx_transparent_data(&paramsStruct);

  ual_util_set_rx_buf_size(&paramsStruct);

  if (ual_util_rx_config(&paramsStruct,
                         (char* )CLIENT_NAME))
  {
    LOGE("%s: ual_util_rx_config failed.",
         __FUNCTION__);
    hovering_exit(EXIT_FAILURE);
  }

  hovering_params_init(&paramsStruct);

  hovering_init(&paramsStruct);

  print_DSP_ver();

  // Send pattern to UAL for the first time

  // Pattern is taken from file named in the cfg file
  if (0 != paramsStruct.usf_rx_pattern[0])
  {
    rc = !(ual_util_read_pattern(hoveringParams.m_pPattern,
                                 &paramsStruct,
                                 (char *)PATTERN_DIR_PATH));
    if (1 != rc)
    {
      LOGE("%s: ual_util_read_pattern failed.",
           __FUNCTION__);
      hovering_exit(EXIT_FAILURE);
    }

    // Pattern is transmitted only once. DSP transmits pattern in loop.
    rc = ual_write(hoveringParams.m_pPattern,
                   hoveringParams.m_patternSize);
    if (1 != rc)
    {
      LOGE("%s: ual_write failed.",
           __FUNCTION__);
      hovering_exit(EXIT_FAILURE);
    }

  }
  // Pattern is taken from Hovering lib
  else
  {
    hovering_get_points(NULL);
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

  uint32_t bytesWriteToFile = paramsStruct.usf_frame_count *
                              frame_size_in_bytes;

  int num_of_regions = sizeof(data.region) / sizeof(ual_data_region_type);

  while (sb_run)
  {
    uint8_t* nextFrame = NULL;

    rc = ual_read(&data,
                  hoveringParams.m_events,
                  hoveringStats.numPoints);
    // As in the rest of the services
    hoveringStats.numPoints = 0;
    if (rc != 1)
    {
      LOGE("%s: ual_read failed.",
           __FUNCTION__);
      if (NULL != frameFile)
      {
        fclose(frameFile);
        frameFile = NULL;
      }
      hovering_exit(EXIT_FAILURE);
    }

    if (0 < paramsStruct.usf_frame_count &&
        false == frame_file_created)
    {
      frame_file_created = true;
      // Open frame file from cfg file
      frameFile = ual_util_get_frame_file (&paramsStruct,
                                           (char *)FRAME_FILE_DIR_PATH);
      if (NULL == frameFile)
      {
        LOGE("%s: ual_util_get_frame_file failed",
             __FUNCTION__);
        hovering_exit(EXIT_FAILURE);
      }
    }

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
        } // recording

        for (int f = 0; f < numberOfFrames ; f++)
        {
          // Statistics
          int seqNum = *(nextFrame + ECHO_FRAME_SEQNUM_OFFSET /
                       sizeof(short));

          // If this is the first iteration then the frames
          // counter is -1 and we need to update the frames counter.
          if (hoveringParams.m_nNextFrameSeqNum == -1)
          {
            hoveringParams.m_nNextFrameSeqNum = seqNum;
          }
          // This is not the first iteration.
          else
          {
            if (hoveringParams.m_nNextFrameSeqNum != seqNum)
            {
              // We lost some frames so we add the number of lost frames
              // to the statistics.
              if (hoveringParams.m_nNextFrameSeqNum < seqNum)
              {
                hoveringStats.m_nLostFrames +=
                  (seqNum - hoveringParams.m_nNextFrameSeqNum)/
                  paramsStruct.usf_tx_skip;
              }
              // We got out of order frames so we add the number of
              // out of order frames to the statistics.
              else
              {
                hoveringStats.m_nOutOfOrderErrors +=
                  (hoveringParams.m_nNextFrameSeqNum - seqNum)/
                                    paramsStruct.usf_tx_skip;
              }

              // Update the frames counter to the correct count.
              hoveringParams.m_nNextFrameSeqNum = seqNum;
            }
          }
          hoveringStats.m_nTotalFrames++;
          // Update the frames counter to the expected count in the next
          // iteration.
          hoveringParams.m_nNextFrameSeqNum += paramsStruct.usf_tx_skip;

          packetCounter++;
          if (STATISTIC_PRINT_INTERVAL == packetCounter)
          {
            LOGI("%s: Statistics (printed every %d frames):",
                 __FUNCTION__,
                 STATISTIC_PRINT_INTERVAL);
            LOGI("Points calculated: %d, total frames: %d, lost frames: %d, "
                 "out of order: %d",
                 hoveringStats.m_nPointsCalculated,
                 hoveringStats.m_nTotalFrames,
                 hoveringStats.m_nLostFrames,
                 hoveringStats.m_nOutOfOrderErrors);
            packetCounter = 0;
          }

          // Calculation
          numPoints = hovering_get_points((short *)nextFrame);
          if (numPoints < 0)
          {
            if (NULL != frameFile)
            {
              fclose(frameFile);
              frameFile = NULL;
            }
            hovering_exit(EXIT_FAILURE);
          }
          nextFrame += frame_size_in_bytes;
        } // f (frames) loop
        pGroupData += paramsStruct.usf_tx_buf_size;
      } // g (groups) loop
    } // r (regions) loop
  } // main loop

  hovering_exit(EXIT_SUCCESS);
}
