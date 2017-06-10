/*===========================================================================
          ual_if.cpp

DESCRIPTION:
Implementation of interface between AHAL and UAL

INITIALIZATION AND SEQUENCING REQUIREMENTS:

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/*----------------------------------------------------------------------------
   Macros and constants
----------------------------------------------------------------------------*/
#define LOG_TAG  "UAL_IF"

/*----------------------------------------------------------------------------
   Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <utils/Log.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include "ual_alsa_defs.h"
#include "ual_if_defs.h"

// PCM handler of file, used for US RX
static  struct pcm *s_pcm_fh = NULL;
// Descriptor of file, used for US RX
static  int s_pcm_fd = -1;

/*==============================================================================
   FUNCTION:  ualIfNotifyActiveState
==============================================================================*/
/**
   Notify UAL about device active state
*/
extern "C" bool ualIfNotifyActiveState
(
  // Current use case, that is going to be set in active state
  const char* use_case
)
{
  const int UAL_IF_NO_HANDLING = -1;

  static int s_rx_pcm_ind = 0;
  static char s_pcm_dev_name[MAX_PCM_DEV_NAME_SIZE];
  int tx_pcm_ind = 0;
  int rc = 0;
  FILE* pcm_inds_file = NULL;
  int pcm_flags = PCM_MONO | PCM_NMMAP;

  LOGD("%s: devs = %s; pid=%d; ppid=%d",
           __FUNCTION__,
           use_case,
           getpid(),
           getppid() );
  LOGD("%s: s_pcm_fd=%d; s_rx_pcm_ind=%d",
             __FUNCTION__,
             s_pcm_fd,
             s_rx_pcm_ind);

  if ((UAL_IF_NO_HANDLING == s_rx_pcm_ind) || // handling was canceled
      (-1 != s_pcm_fd))
  {
    LOGD("%s: s_pcm_fd=%d; s_rx_pcm_ind=%d",
             __FUNCTION__,
             s_pcm_fd,
             s_rx_pcm_ind);
    return true;
  }

  if (0 == s_rx_pcm_ind)
  { // PCM file name was not built
    // One time action
    FILE* pcm_inds_file = fopen(UAL_PCM_INDS_NAME, "r");
    if (NULL == pcm_inds_file)
    {
      LOGE("%s: pcm_port_file(%s) open failed; errno=%d",
           __FUNCTION__,
           UAL_PCM_INDS_NAME,
           errno);
      s_rx_pcm_ind = UAL_IF_NO_HANDLING; // Stop handling
      return true;
    }

    rc = fscanf(pcm_inds_file, "%d %d", &tx_pcm_ind, &s_rx_pcm_ind);
    fclose(pcm_inds_file);
    if (PCM_INDS_NUM != rc)
    {
      LOGE("%s: wrong content of pcm_inds_file(%s)",
           __FUNCTION__,
           UAL_PCM_INDS_NAME);
      s_rx_pcm_ind = UAL_IF_NO_HANDLING; // Stop handling
      return true;
    }

    snprintf(s_pcm_dev_name, sizeof(s_pcm_dev_name), "%s%d",
             UAL_RX_PCM_DEVICE_PREF, s_rx_pcm_ind);
  } // US PCM file name build

  s_pcm_fh = pcm_open(pcm_flags,
                     const_cast<char*>(s_pcm_dev_name));
  if (-1 == s_pcm_fh->fd)
  {
    // Ask US to stop
    int trig_fd = open(UAL_STOP_RX_TRIGGER_FILE_NAME,
                       O_RDONLY | O_CREAT | O_EXCL,
                       0777);

    if (-1 != trig_fd)
    {
      const uint32_t USF_RELEASE_TIME = 20*1000; // 20 msec
      const uint16_t MAX_RELEASE_TRY = 30;
      uint16_t itry = 0;

      close(trig_fd);
      for (itry=0; itry<MAX_RELEASE_TRY; ++itry)
      {
        usleep(USF_RELEASE_TIME);
        s_pcm_fh = pcm_open(pcm_flags,
                           const_cast<char*>(s_pcm_dev_name));
        if (-1 == s_pcm_fh->fd)
        {
          // maybe RX is yet busy by US
          LOGD("%s: USF device [%s] open failed; errno=%d",
             __FUNCTION__,
             s_pcm_dev_name,
             errno);
        }
        else
        {
          s_pcm_fd = s_pcm_fh->fd;
          LOGD("%s: itry=%d",
               __FUNCTION__,
               itry);
          return true;
        }
      } // loop

      rc = unlink(UAL_STOP_RX_TRIGGER_FILE_NAME);
      LOGD("%s: cancel US failed; rc=%d",
           __FUNCTION__,
           rc);
    }
    else
    {
        LOGE("%s: notify US failed; errno=%d",
           __FUNCTION__,
           errno);
        s_rx_pcm_ind = UAL_IF_NO_HANDLING; // Stop handling
    }
  } // PCM file is busy
  else
  {
    s_pcm_fd = s_pcm_fh->fd;
  }

  return true;
}

/*==========================================================================
   FUNCTION:  ualIfNotifyInactiveState
===========================================================================*/
/**
   Notify UAL about device inactive state
*/
extern "C" bool ualIfNotifyInactiveState
(
  // Current use case, that has been set in inactive state
  const char* use_case
)
{
  if (-1 == s_pcm_fd)
  {
    return true;
  }

  pcm_close(s_pcm_fh);
  LOGD("%s: RX US PCM device was closed [use_case=%s]",
       __FUNCTION__,
       use_case);
  s_pcm_fd = -1;
  s_pcm_fh = NULL;
  (void)unlink(UAL_STOP_RX_TRIGGER_FILE_NAME);

  return true;
}
