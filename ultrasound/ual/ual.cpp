/*===========================================================================
                           ual.cpp

DESCRIPTION: Ultrasound Abstract Layer (UAL) implementation

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/


/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#define LOG_TAG "UAL"
#include "usf_log.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include "usf.h"
#include "ual.h"
#include "ual_if_defs.h"

/*----------------------------------------------------------------------------
  Macros and constants
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
// The required prefix of the calculator name
static  const char* UAL_CALC_NAME_PREFIX = "USF_";
// The calculator name
static  char s_calc_name[USF_MAX_CLIENT_NAME_SIZE];

// UAL version
static const char* UAL_VERSION = "1.7.1";

// Supported USF version
static const char* SUPPORTED_USF_VERSION = "1.7";

/* Version buffer size */
static const uint16_t USF_VERSION_SIZE = 20;

// The current USF version
static  char s_usf_version[USF_VERSION_SIZE];

// "IF with the USF" file descriptor
static  int s_usf_fd = -1;

// Supported mode of the USF work activity
// It mainly used for USF power consuption studies
static  ual_work_mode_type  s_work_mode = UAL_MODE_STANDARD;

// Sleep time (sec) in the case of "UAL_MODE_IDLE_USF_DATA_PATH" or
// "UAL_MODE_IDLE_ALL_DATA_PATH" work modes
// during "read data"
static const uint16_t  UAL_MODE_IDLE_DATA_SLEEP_TIME = 30;

// Selected TX sound device ID
static  int s_tx_dev_id = -1;

// One buffer (in the cyclic queue) is for one group of the US frames
// The US data buffer size (bytes) in the cyclic queue
static uint32_t  s_tx_buf_size = 0;
// Number of the buffers in the cyclic queue
static uint16_t  s_tx_buf_number = 0;

// TX US data memory (shared with the USF) size
static uint32_t s_tx_alloc_size = 0;

// TX US data memory, shared with the USF
static uint8_t* s_tx_user_addr = NULL;

// Free region (in the cyclic queue) for writing by the USF
// Pointer (read index) to the end of available region
// in the shared US data memory
static uint16_t s_tx_read_index = 0;

// Ready region (in the cyclic queue) with US data for a client
// Pointer (write index) to the end of ready US data region
// in the shared memory
static uint16_t s_tx_write_index = 0;

// Selected RX sound device ID
static  int s_rx_dev_id = -1;

// One buffer (in the cyclic queue) is for one group of the US frames
// The US data buffer size (bytes) in the cyclic queue
static uint32_t  s_rx_buf_size = 0;
// Number of the buffers in the cyclic queue
static uint16_t  s_rx_buf_number = 0;

// RX US data memory (shared with the USF) size
static uint32_t s_rx_alloc_size = 0;
// RX US data memory, shared with the USF
static uint8_t* s_rx_user_addr = NULL;

// Free region (in the cyclic queue) for writing by client
static uint16_t s_rx_read_index = 0;
// Ready region (in the cyclic queue) for US data transmition
static uint16_t s_rx_write_index = 0;

// Events (from conflicting devs) to be disabled/enabled
static uint16_t s_event_filters;

// The monitor thread identification
static  pthread_t s_monitor_thread;

// The monitoring control
static bool sb_monitoring = true;

// The UAL status
static uint32_t s_ual_status = 0;

// RX path control mutex
static pthread_mutex_t s_ual_RX_control_mutex = PTHREAD_MUTEX_INITIALIZER;

// TX path control mutex
static pthread_mutex_t s_ual_TX_control_mutex = PTHREAD_MUTEX_INITIALIZER;

//  Mutex, controlling the usf driver open/close
static pthread_mutex_t s_ual_control_mutex = PTHREAD_MUTEX_INITIALIZER;

/*------------------------------------------------------------------------------
  Function declarations
------------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  ual_alsa_open_device
==============================================================================*/
/**
  The function opens sound device - configures the codec
*/
extern int ual_alsa_open_device
(
  // Configuration struct
  us_xx_info_type& us_xx_info,
  // Direction (Tx, Rx)
  ual_direction_type direction
);

/*==============================================================================
  FUNCTION:  ual_alsa_close_device
==============================================================================*/
/**
  The function closes the device - releases the codec resources
*/
extern bool ual_alsa_close_device
(
  // Direction (Tx, Rx)
  ual_direction_type direction
);

/*==============================================================================
  FUNCTION:  ual_alsa_open
==============================================================================*/
/**
  The function opens the ual_alsa sub-system
*/
extern bool ual_alsa_open();

/*==============================================================================
  FUNCTION:  ual_alsa_close
==============================================================================*/
/**
  The function closes the ual_alsa sub-system - releases the local resources
*/
extern void ual_alsa_close();

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  ual_get_status
==============================================================================*/
/**
  Get UAL status: TX(1), RX(2)
*/
uint32_t ual_get_status
(
  void
)
{
  return s_ual_status;
}

/*==============================================================================
  FUNCTION:  ual_monitor
==============================================================================*/
static void* ual_monitor
(
  void* arg
)
{
  while (sb_monitoring)
  {
    const uint32_t UAL_MONITOR_PERIOD = 20*1000; // 20 msec
    struct stat statbuf;

    usleep(UAL_MONITOR_PERIOD);
    if (0 == stat(UAL_STOP_RX_TRIGGER_FILE_NAME, &statbuf))
    {
      // Media server (voice call) asked to release RX, but continue with TX
      LOGD("%s: RX stop",
           __FUNCTION__);
      (void)ual_stop_RX();
      break;
    } // Notification handle
  } // lifetime loop

  LOGD("%s: Monitor exit",
        __FUNCTION__);

  return NULL;
}

/*==============================================================================
  FUNCTION:  ual_start_monitor
==============================================================================*/
static bool ual_start_monitor
(
  void
)
{
  bool rc = true;

  sb_monitoring = true;
  int ret = pthread_create(&s_monitor_thread, NULL, ual_monitor, NULL);

  if (ret)
  {
    LOGE("%s:  monitor pthread_create failure; errno=%d",
          __FUNCTION__,
             errno);
    rc = false;
    sb_monitoring = false;
  }

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_get_write_buf
==============================================================================*/
static uint8_t *ual_get_write_buf
(
  void
)
{
  uint8_t *pBuf = NULL;

  if (NULL == s_rx_user_addr)
  {
    LOGE("%s:  Write buf is NULL",
         __FUNCTION__);
    return NULL;
  }

  int next_write_ind = s_rx_write_index + 1;
  if (next_write_ind == s_rx_buf_number)
  {
    next_write_ind = 0;
  }

  // queue (1 gap) isn't full
  if (next_write_ind != s_rx_read_index)
  {
    pBuf = s_rx_user_addr + s_rx_write_index * s_rx_buf_size;
    LOGD("%s:  Returning write_index %d at 0x%p",
         __FUNCTION__,
         s_rx_write_index,
         pBuf);
    s_rx_write_index = next_write_ind;
  }

  return pBuf;
}

/*==============================================================================
  FUNCTION:  ual_open
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_open
(
  // The session configuration
  ual_cfg_type *cfg
)
{
  bool rc = true;

  if (NULL == cfg)
  {
    LOGE("%s:  cfg is NULL",
         __FUNCTION__);
    return false;
  }

  LOGD("%s:  usf_dev_id = %d, ual_mode = %d",
       __FUNCTION__,
       cfg->usf_dev_id,
       cfg->ual_mode);

  (void)pthread_mutex_lock(&s_ual_control_mutex);

  if (-1 == s_usf_fd)
  {
    // Successful Open USF is accomplished only one time
    char device_name[20];
    snprintf(device_name,
             sizeof(device_name),
             "/dev/usf%d",
             cfg->usf_dev_id);

    s_usf_fd = open(device_name,
                    O_RDWR);

    if (0 > s_usf_fd)
    {
      rc = false;
      LOGE("%s:  Could not open usf device %s",
           __FUNCTION__,
           device_name);
    }
    else
    {
      us_version_info_type version_info =
      {
        sizeof(s_usf_version),
        s_usf_version
      };

      int ret = ioctl(s_usf_fd,
                      US_GET_VERSION,
                      &version_info);
      if (ret < 0)
      {
        LOGE("%s:  US_GET_VERSION failed. ret = %d (%s)",
             __FUNCTION__,
             ret,
             strerror(errno));
        close(s_usf_fd);
        s_usf_fd = -1;
        rc = false;
      }
      else
      {
        int len = strlen(SUPPORTED_USF_VERSION);
        LOGD("%s: USF version[%s]; supported one[%s]",
               __FUNCTION__,
               s_usf_version,
               SUPPORTED_USF_VERSION);
        if (strncmp(s_usf_version, SUPPORTED_USF_VERSION, len))
        {
          LOGE("%s: USF version[%s] != supported one[%s]",
                 __FUNCTION__, s_usf_version, SUPPORTED_USF_VERSION);
          close(s_usf_fd);
          s_usf_fd = -1;
          rc = false;
        }
        else
        {
          if (!ual_alsa_open())
          {
            close(s_usf_fd);
            s_usf_fd = -1;
            rc = false;
          }
          else
          {
            s_work_mode = cfg->ual_mode;
          }
        }
      } // the current and supported usf versions compare
    } // usf is opened
  } // usf open block

  (void)pthread_mutex_unlock(&s_ual_control_mutex);

  return rc;
} // ual_open

/*==============================================================================
  FUNCTION:  ual_configure_TX
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_configure_TX
(
  // US Tx device (& stream) configuration
  us_tx_info_type *tx_info
)
{
  bool rc = true;
  uint8_t  port_id_backup[USF_MAX_PORT_NUM];

 (void)pthread_mutex_lock(&s_ual_TX_control_mutex);

  if ((NULL == tx_info) ||
      (-1 == s_usf_fd))
  {
    LOGE("%s:  Wrong tx_info or s_usf_fd (%d)",
         __FUNCTION__,
         s_usf_fd);
    (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
    return false;
  }

  // The required port_ids backup
  (void)memcpy(port_id_backup, tx_info->us_xx_info.port_id, USF_MAX_PORT_NUM);

  // Obtain audio device ID
  s_tx_dev_id = ual_alsa_open_device(tx_info->us_xx_info,
                                     UAL_TX_DIRECTION);
  if (0 > s_tx_dev_id)
  {
    LOGE("%s: Could not obtain audio device ID (port=%d)",
         __FUNCTION__,
         tx_info->us_xx_info.port_id[0]);
    (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
    return false;
  }

  LOGD("%s:  Got audio device ID %d",
       __FUNCTION__,
       s_tx_dev_id);

  // Device switch only
  if (UAL_MODE_IDLE_ALL_DATA_PATH == s_work_mode)
  {
    LOGD("%s:  Only device switch",
         __FUNCTION__);
    // The required port_ids restore
    (void)memcpy(tx_info->us_xx_info.port_id,port_id_backup,USF_MAX_PORT_NUM);
    (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
    return false;
  }

  tx_info->us_xx_info.dev_id = s_tx_dev_id;

  // In Tx the calculator's name should have specific prefix
  if (strncmp(tx_info->us_xx_info.client_name,
              UAL_CALC_NAME_PREFIX,
              strlen(UAL_CALC_NAME_PREFIX)))
  {
    // The name has no required prefix - add it
    snprintf(s_calc_name,
             sizeof(s_calc_name),
             "%s%s",
             UAL_CALC_NAME_PREFIX,
             tx_info->us_xx_info.client_name);
    tx_info->us_xx_info.client_name = s_calc_name;
  }

  LOGD("TX_INFO rate = %d, bsize = %d, bnum = %d, bps = %d, " \
       "max_param_bsize = %d, name = %s",
       tx_info->us_xx_info.sample_rate,
       tx_info->us_xx_info.buf_size,
       tx_info->us_xx_info.buf_num,
       tx_info->us_xx_info.bits_per_sample,
       tx_info->us_xx_info.max_get_set_param_buf_size,
       tx_info->us_xx_info.client_name);

  int ret = ioctl(s_usf_fd,
                  US_SET_TX_INFO,
                  tx_info);

  // The required port_ids restore
  (void)memcpy(tx_info->us_xx_info.port_id,port_id_backup,USF_MAX_PORT_NUM);

  if (0 > ret)
  {
    LOGE( "ioctl(US_SET_TX_INFO) failed:  ret = %d err = %d (%s)",
          ret,
          errno,
          strerror(errno));
    (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
    return false;
  }

  s_tx_buf_size = tx_info->us_xx_info.buf_size;
  s_tx_buf_number = tx_info->us_xx_info.buf_num;
  s_tx_alloc_size = s_tx_buf_size * s_tx_buf_number;

  if (0 < s_tx_alloc_size)
  { // Data path is required
    // request memory mapping
    s_tx_user_addr = static_cast<uint8_t*>(mmap(0,
                                                s_tx_alloc_size,
                                                PROT_READ, MAP_SHARED,
                                                s_usf_fd,
                                                0));

    LOGD("%s:  Received 0x%p from mmap; size = %d",
         __FUNCTION__,
         s_tx_user_addr,
         s_tx_alloc_size);

    if (MAP_FAILED == s_tx_user_addr)
    {
      LOGE("%s:  mmap failed():  err = %d (%s)",
           __FUNCTION__,
           errno,
           strerror(errno));

      s_tx_user_addr = NULL;
      s_tx_alloc_size = 0;
      s_tx_buf_size = 0;
      s_tx_buf_number = 0;
      rc = false;
    }
  }

  if (rc)
  {
    s_ual_status |= UAL_TX_STATUS_ON;
  }

 (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_configure_RX
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_configure_RX
(
  // US RX device (& stream) configuration
  us_rx_info_type *rx_info
)
{
  bool rc = true;

  (void)pthread_mutex_lock(&s_ual_RX_control_mutex);
  if ((NULL == rx_info) ||
      (-1 == s_usf_fd))
  {
    LOGE("%s:  Wrong rx_info or s_usf_fd[%d]",
         __FUNCTION__, s_usf_fd);
    (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
    return false;
  }

  // obtain audio device ID
  s_rx_dev_id = ual_alsa_open_device(rx_info->us_xx_info,
                                         UAL_RX_DIRECTION);
  if (0 > s_rx_dev_id)
  {
    LOGE("%s:  Could not obtain audio device ID. Stop",
         __FUNCTION__);
    (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
    return false;
  }
  LOGI("%s:  Got audio device ID %d",
       __FUNCTION__, s_rx_dev_id);

   // Only device switch
  if (UAL_MODE_IDLE_ALL_DATA_PATH == s_work_mode)
  {
    LOGD("%s:  Only device switch",
         __FUNCTION__);
    (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
    return false;
  }

  rx_info->us_xx_info.dev_id = s_rx_dev_id;

  LOGD("%s:  RX_INFO s_rate = %d, buf_size = %d buf_num = %d " \
       "bits_per_sample = %d, max_param_bsize = %d",
       __FUNCTION__,
       rx_info->us_xx_info.sample_rate,
       rx_info->us_xx_info.buf_size,
       rx_info->us_xx_info.buf_num,
       rx_info->us_xx_info.bits_per_sample,
       rx_info->us_xx_info.max_get_set_param_buf_size);

  int ret = ioctl(s_usf_fd,
                  US_SET_RX_INFO,
                  rx_info);
  if (0 > ret)
  {
    LOGE("%s:  ioctl(US_SET_RX_INFO) failed. ret = %d err = %d (%s)",
         __FUNCTION__,
          ret,
         errno,
         strerror(errno));
    (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
    return false;
  }

  s_rx_buf_size = rx_info->us_xx_info.buf_size;
  s_rx_buf_number = rx_info->us_xx_info.buf_num;
  s_rx_alloc_size = s_rx_buf_size * s_rx_buf_number;

  // Data path is required --> Request memory mapping
  if (s_rx_alloc_size > 0)
  {
    s_rx_user_addr = static_cast<uint8_t*>(mmap(0,
                                                s_rx_alloc_size,
                                                PROT_WRITE,
                                                MAP_SHARED,
                                                s_usf_fd,
                                                0));

    LOGD("%s:  Received 0x%p from mmap; size = %d",
         __FUNCTION__,
         s_rx_user_addr,
         s_rx_alloc_size);

    if(MAP_FAILED == s_rx_user_addr)
    {
      LOGE("%s:  mmap failed(); err = %d (%s)",
           __FUNCTION__,
           errno,
           strerror(errno));

      s_rx_user_addr = NULL;
      s_rx_alloc_size = 0;
      s_rx_buf_size = 0;
      s_rx_buf_number = 0;
      rc = false;
    }
  }

  if (rc)
  {
    rc = ual_start_monitor();
    if (rc)
    {
      s_ual_status |= UAL_RX_STATUS_ON;
    }
  }

  (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_start_TX
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_start_TX(void)
{
  bool  rc = true;

 (void)pthread_mutex_lock(&s_ual_TX_control_mutex);

  if (-1 == s_usf_fd)
  {
    LOGE("%s:  UAL is not opened",
         __FUNCTION__);

    rc = false;
  }
  else
  {
    int ret = ioctl(s_usf_fd,
                    US_START_TX,
                    NULL);
    rc = (ret >= 0);
  }

  LOGD("%s:  rc = %d",
       __FUNCTION__,
       rc);

  (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_start_RX
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_start_RX(void)
{
  bool  rc = true;

  (void)pthread_mutex_lock(&s_ual_RX_control_mutex);
  if (- 1== s_usf_fd)
  {
    LOGE("%s:  UAL is not opened",
         __FUNCTION__);

    rc = false;
  }
  else
  {
    int ret = ioctl(s_usf_fd,
                    US_START_RX,
                    NULL);

    rc = (ret >= 0);
  }

  LOGD("%s:  rc = %d",
       __FUNCTION__,
       rc);

  (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
  return rc;
}

/*==============================================================================
  FUNCTION:  ual_stop_TX
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_stop_TX(void)
{
  bool  rc = true;

  (void)pthread_mutex_lock(&s_ual_TX_control_mutex);

  if (-1 == s_usf_fd)
  {
    LOGE("%s:  UAL is not opened",
         __FUNCTION__);

    rc = false;
  }
  else
  {
    // Release Tx sound device
    if (s_tx_dev_id >= 0)
    {
      int ret = ioctl(s_usf_fd,
                      US_STOP_TX,
                      NULL);

      LOGD("%s:  Done US_STOP_TX ret = %d; errno = %d",
           __FUNCTION__,
           ret,
           errno);

      rc = (ret >= 0);

      // Release Tx shared US data memory
      if (NULL != s_tx_user_addr)
      {
        ret = munmap(s_tx_user_addr,
                     s_tx_alloc_size);

        rc = rc && (ret == 0);

        LOGD("%s:  Done munmap (%p, %d) ret = %d, errno = %d",
             __FUNCTION__,
             s_tx_user_addr,
             s_tx_alloc_size,
             ret,
             errno);
      }

      bool rc1 = ual_alsa_close_device(UAL_TX_DIRECTION);
      if (!rc1)
      {
        LOGD("%s:  ual_alsa_close_device(TX) failed, but continue",
             __FUNCTION__);
      }

      s_ual_status &= ~UAL_TX_STATUS_ON;
    }

    s_tx_user_addr = NULL;
    s_tx_alloc_size = 0;
    s_tx_buf_number = 0;
    s_tx_buf_size = 0;
    s_tx_dev_id = -1;
    s_tx_read_index = 0;
    s_tx_write_index = 0;
  }

  (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_stop_RX
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_stop_RX(void)
{
  bool  rc = true;

 (void)pthread_mutex_lock(&s_ual_RX_control_mutex);

  if (-1 == s_usf_fd)
  {
    LOGE("%s:  UAL is not opened",
         __FUNCTION__);

    rc = false;
  }
  else
  {
    // Release Rx sound device
    if (s_rx_dev_id >= 0)
    {
      bool rc1 = ual_alsa_close_device(UAL_RX_DIRECTION);
      if (!rc1)
      {
        LOGD("%s:  ual_alsa_close_device(RX) failed, but continue",
             __FUNCTION__);
      }

      int ret = ioctl(s_usf_fd,
                      US_STOP_RX,
                      NULL);

      LOGD("%s:  Done US_STOP_RX ret = %d, errno = %d",
           __FUNCTION__,
           ret,
           errno);

      rc = (ret >= 0);

      // Release Rx shared US data memory
      if (NULL != s_rx_user_addr)
      {
        ret = munmap(s_rx_user_addr,
                     s_rx_alloc_size);

        rc = rc && (ret == 0);
        LOGD("%s:  Done munmap (%p, %d) ret = %d, errno = %d",
             __FUNCTION__,
             s_rx_user_addr,
             s_rx_alloc_size,
             ret,
             errno);
      }

      // Stop monitor
      sb_monitoring = false;
      s_ual_status &= ~UAL_RX_STATUS_ON;
    }

    s_rx_user_addr = NULL;
    s_rx_alloc_size = 0;
    s_rx_buf_number = 0;
    s_rx_buf_size = 0;
    s_rx_dev_id = -1;
    s_rx_read_index = 0;
    s_rx_write_index = 0;
  }
  (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);

  return rc;
}


bool wait_for_sound_card_state(
    const char *req_card_state,
    int req_times_detected,
    int num_retries,
    int sleep_interval)
{
    const char *path = "/proc/asound/card0/state";
    bool state_found = false;
    int times_detected = 0;

    do
    {
        int fd;

        fd = open(path, O_RDONLY);
        if (fd == -1)
        {
            LOGE("Open %s failed : %s", path, strerror(errno));
        }
        else
        {
            const int MAX_CARD_STATE_STR_LEN = 9;

            char rd_buf[MAX_CARD_STATE_STR_LEN];
            read(fd, (void *)rd_buf, sizeof(rd_buf));
            close(fd);

            rd_buf[sizeof(rd_buf)-1]='\0';
            LOGD("%s:  Read from %s : %s",
                 __FUNCTION__,
                 path,
                 rd_buf);

            if(strstr(rd_buf, req_card_state))
            {
                times_detected++;

                LOGD("%s: %s detected %d times in a row, required %d",
                 __FUNCTION__,
                 req_card_state,
                 times_detected,
                 req_times_detected);

                if(times_detected == req_times_detected)
                {
                    state_found = true;
                    break;
                }
            }
            else
            {
                times_detected = 0;
            }
        }

        num_retries--;
        usleep(sleep_interval*1000);
    } while(num_retries);

    return state_found;
}

/*==============================================================================
  FUNCTION:  ual_close
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_close(
    // If true, close will be done under the assumption that an error occured (this
    // affects sequence with lower layers during close)
    bool error_state)
{
  LOGD("%s:  Enter",
       __FUNCTION__);

  if(error_state)
  {
    const char* req_card_state = "ONLINE";

    LOGD("%s:  Waiting for sound card state %s before starting close sequence",
       __FUNCTION__,
         req_card_state);

    if(!wait_for_sound_card_state(req_card_state, 4, 30, 100))
    {
      LOGE("%s:  Sound card state %s not detected, stream might not close correctly",
       __FUNCTION__,
       req_card_state);
    }

    LOGD("%s: Continuing with close sequence",
         __FUNCTION__);
  }

  bool  rc = true;

  if (-1 == s_usf_fd)
  {
    LOGE("%s:  UAL is not opened",
         __FUNCTION__);

    rc = false;
  }
  else
  {
    rc = ual_stop_RX();
    bool rc1 = ual_stop_TX();
    rc = rc && rc1;
    (void)pthread_mutex_lock(&s_ual_control_mutex);
    close(s_usf_fd);
    s_usf_fd = -1;
    ual_alsa_close();
    (void)pthread_mutex_unlock(&s_ual_control_mutex);
  }

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_write
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_write
(
  // US RX data to send
  uint8_t *data,
  // Size of US RX data to send
  uint32_t data_size
)
{
  (void)pthread_mutex_lock(&s_ual_RX_control_mutex);

  if (UAL_MODE_IDLE_ALL_DATA_PATH == s_work_mode)
  {
    LOGD("%s:  Only device switch",
         __FUNCTION__);
    (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
    return true;
  }

  if ((s_usf_fd == -1 ) ||
      (data == NULL) ||
      (data_size > s_rx_buf_size))
  {
    LOGE("%s:  Wrong parameters: data = %p, data = %d or UAL is not opened (%d)",
         __FUNCTION__,
         data,
         data_size,
         s_usf_fd);

    (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
    return false;
  }

  uint8_t* pBuf = ual_get_write_buf();
  if (NULL == pBuf)
  {
    LOGE("%s:  Rx path full",
         __FUNCTION__);

    (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
    return false;
  }

  us_rx_update_info_type update_info;

  memcpy(pBuf,
         data,
         data_size);
  update_info.ready_region = s_rx_write_index;

  update_info.params_data = NULL; // not supported by USF
  update_info.params_data_size = 0; // not supported by USF;

  bool rc = true;
  int ret = ioctl(s_usf_fd,
                  US_SET_RX_UPDATE,
                  &update_info);

  if (ret < 0)
  {
    LOGE("%s:  (US_SET_RX_UPDATE) failed. ret = %d (%s)",
         __FUNCTION__,
         ret,
         strerror(errno));

    rc = false;
  }
  else
  {
    s_rx_read_index = update_info.free_region;
    LOGD("%s:  (US_SET_RX_UPDATE): read(freereg) = %d write(readyreg) = %d",
         __FUNCTION__,
         update_info.free_region,
         update_info.ready_region);
  }

  (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_read
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_read
(
  // Place for received Tx US data
  ual_data_type *data,
  // The calculated events
  usf_event_type *events,
  // The number of calculated events
  uint16_t event_counter,
  // Time (sec) to wait for data
  uint32_t timeout
)
{
  (void)pthread_mutex_lock(&s_ual_TX_control_mutex);

  if (-1 == s_usf_fd)
  {
    (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
    LOGE("%s:UAL is not opened",
         __FUNCTION__);
    return false;
  }

  us_tx_update_info_type update_info;
  bool rc = true;

  update_info.timeout = timeout;
  if ((events != NULL) &&
      (event_counter >  0) &&
      (s_work_mode != UAL_MODE_NO_INJECT_IN_EVENTS))
  {

    update_info.event = events;
    update_info.event_counter = event_counter;
  }
  else
  {
    update_info.event = NULL;
    update_info.event_counter = 0;
  }

  // ual_read() call means, the region from the previous call
  // may be released (that is, the queue is empty)
  s_tx_read_index = s_tx_write_index;

  if ((s_work_mode == UAL_MODE_IDLE_ALL_DATA_PATH) ||
      (s_work_mode == UAL_MODE_IDLE_USF_DATA_PATH))
  {  // (s_tx_write_index == s_tx_read_index) is invariant
    sleep(UAL_MODE_IDLE_DATA_SLEEP_TIME);
  }
  else
  {
    update_info.params_data = NULL;
    update_info.params_data_size = 0;
    update_info.free_region = s_tx_read_index;
    update_info.event_filters = s_event_filters;

    int ret = ioctl(s_usf_fd,
                    US_GET_TX_UPDATE,
                    &update_info);

    if (ret < 0)
    {
      if (ETIME != errno)
      {
        LOGE("%s:  (US_GET_TX_UPDATE) failed. ret = %d (%s)",
             __FUNCTION__,
             ret,
             strerror(errno));
        (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
        return false;
      }
      else
      {
        LOGD("%s:  (US_GET_TX_UPDATE) failed. errno = ETIME; continue..",
             __FUNCTION__);
      }
    }
    else
    {
      s_tx_write_index =  update_info.ready_region;
    }
  }

  if (NULL != data)
  {
    /* Data isn't required or the queue is empty or
       UAL_MODE_NO_CALC_IN_EVENTS work mode */
    if ((NULL == s_tx_user_addr) ||
        (s_tx_write_index == s_tx_read_index) ||
        (s_work_mode == UAL_MODE_NO_CALC_IN_EVENTS))
    {
      data->region[0].data_buf = NULL;
      data->region[0].data_buf_size = 0;
      data->region[1].data_buf = NULL;
      data->region[1].data_buf_size = 0;
    }
    else
    {
      if (s_tx_write_index > s_tx_read_index)
      { // 1 region
        data->region[0].data_buf = s_tx_user_addr +
                                   s_tx_buf_size*s_tx_read_index;
        data->region[0].data_buf_size = s_tx_buf_size *
                                        (s_tx_write_index - s_tx_read_index);
        data->region[1].data_buf = NULL;
        data->region[1].data_buf_size = 0;
      }
      else
      { // 2 regions
        data->region[0].data_buf = s_tx_user_addr +
                                   s_tx_buf_size*s_tx_read_index;
        data->region[0].data_buf_size = s_tx_buf_size *
                                        (s_tx_buf_number - s_tx_read_index);
        data->region[1].data_buf = s_tx_user_addr;
        data->region[1].data_buf_size = s_tx_buf_size *
                                        s_tx_write_index;
      }
    }
  }

  (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);

  return rc;
} // ual_read

/*==============================================================================
  FUNCTION:  ual_start_us_detection
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_start_us_detection
(
  // US detection info
  us_detect_info_type& detect_info
)
{
  (void)pthread_mutex_lock(&s_ual_TX_control_mutex);
  if ((-1 == s_usf_fd) ||
      (US_DETECT_DISABLED_MODE == detect_info.us_detect_mode))
  {
    LOGE("%s: UAL is not opened or wrong detect mode (%d)",
         __FUNCTION__,
         detect_info.us_detect_mode);
    (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
    return false;
  }

  bool rc = true;
  int ret = ioctl(s_usf_fd,
                  US_SET_DETECTION,
                  &detect_info);
  if (ret < 0)
  {
    LOGE("%s:  US_SET_DETECTION failed. ret = %d (%s)",
         __FUNCTION__,
         ret,
         strerror(errno));
    rc = false;
  }

  (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);

  return rc;
} // ual_start_us_detection

/*==============================================================================
  FUNCTION:  ual_stop_us_detection
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_stop_us_detection
(
  // US detection info
  us_detect_info_type detect_info
)
{
  (void)pthread_mutex_lock(&s_ual_TX_control_mutex);
  if ((-1 == s_usf_fd) ||
      (US_DETECT_DISABLED_MODE != detect_info.us_detect_mode))
  {
    LOGE("%s: UAL is not opened or wrong detect mode (%d)",
         __FUNCTION__,
         detect_info.us_detect_mode);
    (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
    return false;
  }

  bool rc = true;
  int ret = ioctl(s_usf_fd,
                  US_SET_DETECTION,
                  &detect_info);
  if (ret < 0)
  {
    LOGE("%s:  US_SET_DETECTION failed. ret = %d (%s)",
         __FUNCTION__,
         ret,
         strerror(errno));
    rc = false;
  }

  (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);

  return rc;
} // ual_stop_us_detection

/*==============================================================================
  FUNCTION:  ual_get_version
==============================================================================*/
/**
  See documentation in header file
*/
void ual_get_version
(
  us_versions_type& us_versions
)
{
  us_versions.p_usf_version = s_usf_version;
  us_versions.p_ual_version = UAL_VERSION;
}

/*==============================================================================
  FUNCTION:  ual_set_event_filters
==============================================================================*/
/**
  See documentation in header file
*/
void ual_set_event_filters
(
  uint16_t event_filters
)
{
  s_event_filters = event_filters;
}

/*==============================================================================
  FUNCTION:  ual_set_TX_param
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_set_TX_param
(
  // US TX stream configuration
  us_stream_param_type& stream_param
)
{
  (void)pthread_mutex_lock(&s_ual_TX_control_mutex);
  if (-1 == s_usf_fd)
  {
    LOGE("%s: UAL is not opened",
         __FUNCTION__);
    (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
    return false;
  }

  bool rc = true;
  int ret = ioctl(s_usf_fd,
                  US_SET_TX_STREAM_PARAM,
                  &stream_param);
  if (ret < 0)
  {
    LOGE("%s:  US_SET_TX_STREAM_PARAM failed. ret = %d (%s)",
         __FUNCTION__,
         ret,
         strerror(errno));
    rc = false;
  }

  (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);

  return rc;
} // ual_set_TX_param

/*==============================================================================
  FUNCTION:  ual_get_TX_param
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_get_TX_param
(
  // US TX stream configuration
  us_stream_param_type& stream_param
)
{
  (void)pthread_mutex_lock(&s_ual_TX_control_mutex);
  if (-1 == s_usf_fd)
  {
    LOGE("%s: UAL is not opened",
         __FUNCTION__);
    (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);
    return false;
  }

  bool rc = true;
  int ret = ioctl(s_usf_fd,
                  US_GET_TX_STREAM_PARAM,
                  &stream_param);
  if (ret < 0)
  {
    LOGE("%s:  US_GET_TX_STREAM_PARAM failed. ret = %d (%s)",
         __FUNCTION__,
         ret,
         strerror(errno));
    rc = false;
  }

  (void)pthread_mutex_unlock(&s_ual_TX_control_mutex);

  return rc;
} // ual_get_TX_param

/*==============================================================================
  FUNCTION:  ual_set_RX_param
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_set_RX_param
(
  // US TX stream configuration
  us_stream_param_type& stream_param
)
{
  (void)pthread_mutex_lock(&s_ual_RX_control_mutex);
  if (-1 == s_usf_fd)
  {
    LOGE("%s: UAL is not opened",
         __FUNCTION__);
    (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
    return false;
  }

  bool rc = true;
  int ret = ioctl(s_usf_fd,
                  US_SET_RX_STREAM_PARAM,
                  &stream_param);
  if (ret < 0)
  {
    LOGE("%s:  US_SET_RX_STREAM_PARAM failed. ret = %d (%s)",
         __FUNCTION__,
         ret,
         strerror(errno));
    rc = false;
  }

  (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);

  return rc;
} // ual_set_RX_param

/*==============================================================================
  FUNCTION:  ual_get_RX_param
==============================================================================*/
/**
  See documentation in header file
*/
bool ual_get_RX_param
(
  // US TX stream configuration
  us_stream_param_type& stream_param
)
{
  (void)pthread_mutex_lock(&s_ual_RX_control_mutex);
  if (-1 == s_usf_fd)
  {
    LOGE("%s: UAL is not opened",
         __FUNCTION__);
    (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);
    return false;
  }

  bool rc = true;
  int ret = ioctl(s_usf_fd,
                  US_GET_RX_STREAM_PARAM,
                  &stream_param);
  if (ret < 0)
  {
    LOGE("%s:  US_GET_RX_STREAM_PARAM failed. ret = %d (%s)",
         __FUNCTION__,
         ret,
         strerror(errno));
    rc = false;
  }

  (void)pthread_mutex_unlock(&s_ual_RX_control_mutex);

  return rc;
} // ual_get_RX_param
