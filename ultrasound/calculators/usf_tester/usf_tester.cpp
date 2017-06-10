/*===========================================================================
                           usf_tester.cpp

DESCRIPTION: Implementation of the tester daemon.


INITIALIZATION AND SEQUENCING REQUIREMENTS:
  If not started through java app then make sure to have
  correct /data/usf/tester/usf_tester.cfg file linked to the wanted cfg file
  placed in /data/usf/tester/cfg/.
  If using cfg file with tx then make sure to have
  pattern file with matching name to the name in the cfg file
  placed in /data/usf/tester/pattern/.
  The recording file with name as in the cfg file
  will be found in /data/usf/tester/rec/.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_tester"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <ual.h>
#include <cutils/properties.h>
#include <ual_util.h>
#include <fcntl.h>
#include "ual_util_frame_file.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define LINK_CFG_FILE_LOCATION "/data/usf/tester/usf_tester.cfg"
#define PATTERN_DIR_PATH "/data/usf/tester/pattern/"
#define FRAME_FILE_DIR_PATH "/data/usf/tester/rec/"
#define CLIENT_NAME "tester"
#define EXIT_SIGTERM 3

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Ptr to the cfg file. The ptr is global bacause we want to
  close the file before exit in the function tester_exit.
  This function is called also from the signal_handler function
  which doesn't know the cfg file.
*/
static FILE *cfgFile = NULL;

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/**
 * The name of the file containg the pid of the daemon
 */
static const char* PID_FILE_NAME = "usf_tester.pid";

/**
  The daemon running control
*/
static volatile bool sb_run = true;

/*==============================================================================
  FUNCTION:  tester_exit
==============================================================================*/
/**
  Perform clean exit of the tester.
*/
int tester_exit (int status)
{
  // Time before call to ual_close - used to measure how much time it takes
  // to stop ual
  timespec tsCur;
  clock_gettime (CLOCK_REALTIME, &tsCur);
  double timeBeforeUalClose = ((double)(tsCur.tv_nsec) +
                               ((double)tsCur.tv_sec) * 1000000000L) /
                              1000000L; // msec

  bool error_state = (status != EXIT_SUCCESS);
  int rc = ual_close(error_state);
  if (1 != rc)
  {
    LOGW("%s: ual_close: rc=%d;",
         __FUNCTION__,
         rc);
  }

  // Time before after ual_close
  clock_gettime (CLOCK_REALTIME, &tsCur);
  double closeUalDuration =  (((double)(tsCur.tv_nsec) +
                              ((double)tsCur.tv_sec) * 1000000000L) /
                             1000000L) - timeBeforeUalClose; // msec

  LOGW("%s: Duration of ual_close() is: %f msec",
       __FUNCTION__,
       closeUalDuration);

  if (cfgFile != NULL)
  {
    fclose(cfgFile);
    cfgFile = NULL;
  }
  LOGI("%s: Tester end. status=%d",
       __FUNCTION__,
       status);

  int ret = ual_util_remove_declare_pid(PID_FILE_NAME);
  if (0 != ret)
  {
    LOGW("%s: Removing pid file failed",
         __FUNCTION__);
  }

  // Must update flag, so that init would not restart the daemon.
  ret = property_set ("ctl.stop",
                          "usf_tester");
  if (ret != 0)
  {
    LOGW("%s: property_set failed",
         __FUNCTION__);
  }

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
  LOGD("%s: Received signal %d; sb_run=%d",
         __FUNCTION__, sig, sb_run);
  // All supportd signals cause the daemon exit
  sb_run = false;
}

/*==============================================================================
  FUNCTION:  main
==============================================================================*/
/**
  Main function of the tester daemon. Handle all the tester operations.
*/
int main (void)
{
  int ret, frameFileNameLen;
  FILE *frameFile = NULL;
  static us_all_info paramsStruct;
  bool rc = false, frame_file_created = false;
  char frameFileName[MAX_FILE_NAME_LEN] = {0};

  LOGI("%s: Tester start",
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
    tester_exit(EXIT_FAILURE);
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
    tester_exit(EXIT_FAILURE);
  }

  if ((paramsStruct.use_tx && 0 == paramsStruct.usf_tx_transparent_data_size) ||
      (paramsStruct.use_rx && 0 == paramsStruct.usf_rx_transparent_data_size) )
  {
    LOGE("%s: Error with transparent data file",
         __FUNCTION__);
    tester_exit(EXIT_FAILURE);
  }

  ual_cfg_type cfg;
  cfg.usf_dev_id = 1;
  cfg.ual_mode = static_cast<ual_work_mode_type>(paramsStruct.ual_work_mode);
  rc = ual_open(&cfg);
  if (1 != rc)
  {
    LOGE("%s: ual_open: rc=%d",
         __FUNCTION__,
         rc);
    tester_exit(EXIT_FAILURE);
  }

  // Time before call to tx_config - used to measure time until first ual_read
  timespec tsCur;
  clock_gettime (CLOCK_REALTIME, &tsCur);
  double timeBeforeTxConfig = ((double)(tsCur.tv_nsec) +
                               ((double)tsCur.tv_sec) * 1000000000L) /
                              1000000L; // msec

  if (ual_util_tx_config(&paramsStruct,
                         (char* )CLIENT_NAME) == -1)
  {
    LOGE("%s: ual_util_tx_config failed",
         __FUNCTION__);
    tester_exit(EXIT_FAILURE);
  }

  if (ual_util_rx_config(&paramsStruct,
                         (char* )CLIENT_NAME) == -1)
  {
    LOGE("%s: ual_util_rx_config failed",
         __FUNCTION__);
    tester_exit(EXIT_FAILURE);
  }

  if (paramsStruct.use_rx)
  {
    uint8_t* pattern = (uint8_t *)malloc(
                            paramsStruct.usf_rx_pattern_size *
                            paramsStruct.usf_rx_sample_width/BYTE_WIDTH);
    if (NULL == pattern)
    {
      LOGE("%s: Failed to allocate %d bytes",
           __FUNCTION__,
           paramsStruct.usf_rx_pattern_size *
           sizeof(paramsStruct.usf_rx_sample_width));
      tester_exit(EXIT_FAILURE);
    }

    rc = !(ual_util_read_pattern(pattern,
                                 &paramsStruct,
                                 (char *)PATTERN_DIR_PATH));
    if (1 != rc)
    {
      LOGE("%s: ual_util_read_pattern: rc=%d;",
           __FUNCTION__,
           rc);
      free(pattern);
      tester_exit(EXIT_FAILURE);
    }
    // Pattern is transmitted only once.
    rc = ual_write(pattern,
                   paramsStruct.usf_rx_pattern_size *
                   paramsStruct.usf_rx_sample_width/BYTE_WIDTH);
    if (1 != rc)
    {
      LOGE("%s: ual_write: rc=%d;",
           __FUNCTION__,
           rc);
      free(pattern);
      tester_exit(EXIT_FAILURE);
    }

    free(pattern);

    // If only RX then we allow transmit until user stops the tester.
    if (!paramsStruct.use_tx)
    {
      // Wait for signal to end the loop.
      LOGW("%s: Transmition only.",
           __FUNCTION__);
      pause();
    }
  }

  if (paramsStruct.use_tx)
  {
    ual_data_type data;
    usf_event_type event;

    if (0 >= paramsStruct.usf_frame_count)
    {
      LOGD("%s: usf_frame_count is %d. No record has made.",
           __FUNCTION__,
           paramsStruct.usf_frame_count);
    }

    uint32_t numOfBytes = 0;

    uint32_t frame_size_in_bytes =
      paramsStruct.usf_tx_port_data_size *
      sizeof(paramsStruct.usf_tx_sample_width) *
      paramsStruct.usf_tx_port_count +
      paramsStruct.usf_tx_frame_hdr_size;

    uint32_t bytesWriteToFile = paramsStruct.usf_frame_count *
                                frame_size_in_bytes;

    int num_of_regions = sizeof(data.region) / sizeof(ual_data_region_type);

    // Time before first ual_read
    clock_gettime (CLOCK_REALTIME, &tsCur);
    double initUalDuration =  (((double)(tsCur.tv_nsec) +
                               ((double)tsCur.tv_sec) * 1000000000L) /
                              1000000L) - timeBeforeTxConfig; // msec

    LOGW("%s: Time passed between tx_config() to first ual_read() is: %f msec",
         __FUNCTION__,
         initUalDuration);

    while (sb_run && (numOfBytes < bytesWriteToFile))
    {
      rc = ual_read(&data,
                    &event,
                    0);
      if (1 != rc)
      {
        LOGE("%s: ual_read return: %d",
             __FUNCTION__,
             rc);
        if (NULL != frameFile)
        {
          fclose(frameFile);
          frameFile = NULL;
        }
        tester_exit(EXIT_FAILURE);
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
          tester_exit(EXIT_FAILURE);
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
          pGroupData += paramsStruct.usf_tx_buf_size;
        } // group loop
      }  // region loop
    } // main loop

    if (NULL != frameFile)
    {
      if (0 != fsync(fileno(frameFile)))
      {
        LOGW("%s: Flushing frame file to disk failed. Reason: %s",
             __FUNCTION__,
             strerror(errno));
      }
      fclose(frameFile);
      frameFile = NULL;
      // Mark recording as done
      ret = property_set("debug.usf_tester.frame_rec_done",
                         "1");
      if (0 != ret)
      {
        LOGW("%s: setting recording done property failed",
             __FUNCTION__);
      }
    }
    LOGI("%s: Write total %d bytes = %d frames to frameFile",
         __FUNCTION__,
         numOfBytes,
         numOfBytes / frame_size_in_bytes);

  } // End if (paramsStruct.use_tx)

  tester_exit(EXIT_SUCCESS);
}
