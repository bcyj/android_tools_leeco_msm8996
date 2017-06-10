/******************************************************************************
  @file    qmi_platform.c
  @brief   The QMI management layer.  This includes system-wide intialization
  and configuration funtions.

  DESCRIPTION
  QMI management.  Routines for client, system-wide initialization
  and de-initialization

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_init() and qmi_connection_init() needs to be called before starting
  any of the specific service clients.

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <sys/time.h>
#include <errno.h>
#include "qmi_i.h"
#include "qmi_platform_config.h"
#include "ds_util.h"
#ifdef FEATURE_QMI_ANDROID
#include "cutils/properties.h"
#else
#define PROPERTY_VALUE_MAX 40
#endif
#include <sys/types.h>
#include <time.h>

#define QMI_PLATFORM_NUM_CHARS_PER_BYTE (3) /* 2 hex chars per byte + space */
#define QMI_PLATFORM_NUM_BYTES_PER_LINE (16)

#define QMI_PLATFORM_GET_HEX_CHAR(x)  \
  (((x) > 0x0F) ? '*' : hex_digit_to_char_tbl[(x)])

static const char hex_digit_to_char_tbl[] =
{
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/* These strings need to correspond to the qmi_connection_id_type enum.  The first
** string must correspond to QMI_CONN_ID_RMNET_1, etc.  Note that we are currently only
** supporting the non-broadcast ports.
*/
static struct {
  qmi_connection_id_type  conn_id;
  const char *dev_name;
} dev_id_table[] =
{
  { QMI_CONN_ID_RMNET_0,            QMI_PORT_RMNET_0         },
  { QMI_CONN_ID_RMNET_1,            QMI_PORT_RMNET_1         },
  { QMI_CONN_ID_RMNET_2,            QMI_PORT_RMNET_2         },
  { QMI_CONN_ID_RMNET_3,            QMI_PORT_RMNET_3         },
  { QMI_CONN_ID_RMNET_4,            QMI_PORT_RMNET_4         },
  { QMI_CONN_ID_RMNET_5,            QMI_PORT_RMNET_5         },
  { QMI_CONN_ID_RMNET_6,            QMI_PORT_RMNET_6         },
  { QMI_CONN_ID_RMNET_7,            QMI_PORT_RMNET_7         },
  { QMI_CONN_ID_RMNET_8,            QMI_PORT_RMNET_8         },
  { QMI_CONN_ID_RMNET_9,            QMI_PORT_RMNET_9         },
  { QMI_CONN_ID_RMNET_10,           QMI_PORT_RMNET_10        },
  { QMI_CONN_ID_RMNET_11,           QMI_PORT_RMNET_11        },

  { QMI_CONN_ID_REV_RMNET_0,        QMI_PORT_REV_RMNET_0     },
  { QMI_CONN_ID_REV_RMNET_1,        QMI_PORT_REV_RMNET_1     },
  { QMI_CONN_ID_REV_RMNET_2,        QMI_PORT_REV_RMNET_2     },
  { QMI_CONN_ID_REV_RMNET_3,        QMI_PORT_REV_RMNET_3     },
  { QMI_CONN_ID_REV_RMNET_4,        QMI_PORT_REV_RMNET_4     },
  { QMI_CONN_ID_REV_RMNET_5,        QMI_PORT_REV_RMNET_5     },
  { QMI_CONN_ID_REV_RMNET_6,        QMI_PORT_REV_RMNET_6     },
  { QMI_CONN_ID_REV_RMNET_7,        QMI_PORT_REV_RMNET_7     },
  { QMI_CONN_ID_REV_RMNET_8,        QMI_PORT_REV_RMNET_8     },

  { QMI_CONN_ID_RMNET_SDIO_0,       QMI_PORT_RMNET_SDIO_0    },
  { QMI_CONN_ID_RMNET_SDIO_1,       QMI_PORT_RMNET_SDIO_1    },
  { QMI_CONN_ID_RMNET_SDIO_2,       QMI_PORT_RMNET_SDIO_2    },
  { QMI_CONN_ID_RMNET_SDIO_3,       QMI_PORT_RMNET_SDIO_3    },
  { QMI_CONN_ID_RMNET_SDIO_4,       QMI_PORT_RMNET_SDIO_4    },
  { QMI_CONN_ID_RMNET_SDIO_5,       QMI_PORT_RMNET_SDIO_5    },
  { QMI_CONN_ID_RMNET_SDIO_6,       QMI_PORT_RMNET_SDIO_6    },
  { QMI_CONN_ID_RMNET_SDIO_7,       QMI_PORT_RMNET_SDIO_7    },

  { QMI_CONN_ID_RMNET_USB_0,        QMI_PORT_RMNET_USB_0     },
  { QMI_CONN_ID_RMNET_USB_1,        QMI_PORT_RMNET_USB_1     },
  { QMI_CONN_ID_RMNET_USB_2,        QMI_PORT_RMNET_USB_2     },
  { QMI_CONN_ID_RMNET_USB_3,        QMI_PORT_RMNET_USB_3     },
  { QMI_CONN_ID_RMNET_USB_4,        QMI_PORT_RMNET_USB_4     },
  { QMI_CONN_ID_RMNET_USB_5,        QMI_PORT_RMNET_USB_5     },
  { QMI_CONN_ID_RMNET_USB_6,        QMI_PORT_RMNET_USB_6     },
  { QMI_CONN_ID_RMNET_USB_7,        QMI_PORT_RMNET_USB_7     },

  { QMI_CONN_ID_REV_RMNET_USB_0,    QMI_PORT_REV_RMNET_USB_0 },
  { QMI_CONN_ID_REV_RMNET_USB_1,    QMI_PORT_REV_RMNET_USB_1 },
  { QMI_CONN_ID_REV_RMNET_USB_2,    QMI_PORT_REV_RMNET_USB_2 },
  { QMI_CONN_ID_REV_RMNET_USB_3,    QMI_PORT_REV_RMNET_USB_3 },
  { QMI_CONN_ID_REV_RMNET_USB_4,    QMI_PORT_REV_RMNET_USB_4 },
  { QMI_CONN_ID_REV_RMNET_USB_5,    QMI_PORT_REV_RMNET_USB_5 },
  { QMI_CONN_ID_REV_RMNET_USB_6,    QMI_PORT_REV_RMNET_USB_6 },
  { QMI_CONN_ID_REV_RMNET_USB_7,    QMI_PORT_REV_RMNET_USB_7 },
  { QMI_CONN_ID_REV_RMNET_USB_8,    QMI_PORT_REV_RMNET_USB_8 },

  { QMI_CONN_ID_RMNET_SMUX_0,       QMI_PORT_RMNET_SMUX_0    },

  { QMI_CONN_ID_RMNET_12,           "not used"               },
  { QMI_CONN_ID_RMNET_13,           "not used"               },

  { QMI_CONN_ID_RMNET_MDM2_0,       QMI_PORT_RMNET2_USB_0    },
  { QMI_CONN_ID_RMNET_MDM2_1,       QMI_PORT_RMNET2_USB_1    },
  { QMI_CONN_ID_RMNET_MDM2_2,       QMI_PORT_RMNET2_USB_2    },
  { QMI_CONN_ID_RMNET_MDM2_3,       QMI_PORT_RMNET2_USB_3    },
  { QMI_CONN_ID_RMNET_MDM2_4,       QMI_PORT_RMNET2_USB_4    },
  { QMI_CONN_ID_RMNET_MDM2_5,       QMI_PORT_RMNET2_USB_5    },
  { QMI_CONN_ID_RMNET_MDM2_6,       QMI_PORT_RMNET2_USB_6    },
  { QMI_CONN_ID_RMNET_MDM2_7,       QMI_PORT_RMNET2_USB_7    },


  { QMI_CONN_ID_RMNET_MHI_0,        QMI_PORT_RMNET_MHI_0    },
  { QMI_CONN_ID_RMNET_MHI_1,        QMI_PORT_RMNET_MHI_1    },

  { QMI_CONN_ID_PROXY,              QMI_PORT_PROXY           }
};

static const char *qmi_linux_invalid_dev_name = "rmnet_invalid";

#define PORT_NAME_TABLE_SIZE (sizeof (dev_id_table) / sizeof (dev_id_table[0]))
#define NANO_SEC 1000000000
int
qmi_linux_wait_for_sig_with_timeout
(
  qmi_linux_signal_data_type  *signal_ptr,
  int                         timeout_milli_secs
)
{
  int rc = QMI_NO_ERR;
  struct timeval curr_time;
  struct timespec wait_till_time;

  /* Get current time of day */
  gettimeofday (&curr_time,NULL);

  /* Set wait time seconds to current + the number of seconds needed for timeout */
  wait_till_time.tv_sec =  curr_time.tv_sec + (timeout_milli_secs/1000);
  wait_till_time.tv_nsec = (curr_time.tv_usec * 1000) +  ((timeout_milli_secs % 1000) * 1000 * 1000);

  /* Check the nano sec overflow */
  if (wait_till_time.tv_nsec >= NANO_SEC ) {

      wait_till_time.tv_sec +=  wait_till_time.tv_nsec/NANO_SEC;
      wait_till_time.tv_nsec %= NANO_SEC;
  }

  while ((signal_ptr)->cond_predicate == FALSE)
  {
    if (pthread_cond_timedwait (&(signal_ptr)->cond_var,
                                &(signal_ptr)->cond_mutex,
                                &wait_till_time) == ETIMEDOUT)
    {
      rc = QMI_TIMEOUT_ERR;
      break;
    }
  }
  pthread_mutex_unlock (&(signal_ptr)->cond_mutex);

  return rc;
}


qmi_connection_id_type
qmi_linux_get_conn_id_by_name
(
  const char *name
)
{
  unsigned int i;
  size_t name_len;
  qmi_connection_id_type rc = QMI_CONN_ID_INVALID;

  if (name != NULL)
  {
    name_len = strlen (name);

    for (i = 0; i < PORT_NAME_TABLE_SIZE; i++)
    {
      /* Make sure strings are same length */
      if (name_len != strlen (dev_id_table[i].dev_name))
      {
        continue;
      }

      /* Do string in-sensitive comparison to see if they are
      ** equivalent
      */
      if (strncasecmp (dev_id_table[i].dev_name,name,name_len) == 0)
      {
        break;
      }
    }

    if (i < PORT_NAME_TABLE_SIZE)
    {
      rc = dev_id_table[i].conn_id;
    }
  }

  return rc;
}


const char *
qmi_linux_get_name_by_conn_id
(
  qmi_connection_id_type conn_id
)
{
  unsigned int i;
  const char *rc = qmi_linux_invalid_dev_name;
  for (i = 0; i < PORT_NAME_TABLE_SIZE; i++)
  {
    if (dev_id_table[i].conn_id == conn_id)
    {
      rc = dev_id_table[i].dev_name;
      break;
    }
  }
  return rc;
}

qmi_connection_id_type qmi_linux_get_conn_id_by_name_ex
(
  const char    *dev_id,
  int           *ep_type,
  int           *epid,
  int           *mux_id
)
{
  ds_target_t target;
  qmi_connection_id_type rc = QMI_CONN_ID_INVALID;
  int conn_id = -1;

  if (NULL == dev_id || NULL == ep_type || NULL == epid || NULL == mux_id)
  {
    QMI_ERR_MSG_0("Invalid parameters");
    return QMI_CONN_ID_INVALID;
  }

  target  = ds_get_target();
  *ep_type = -1;
  *epid = -1;
  *mux_id = -1;

  if (!strncmp(dev_id, QMI_PLATFORM_RMNET_DATA_PREFIX,
               strlen(QMI_PLATFORM_RMNET_DATA_PREFIX)))
  {
    conn_id = ds_atoi(dev_id + strlen(QMI_PLATFORM_RMNET_DATA_PREFIX));
    if (0 > conn_id || QMI_PLATFORM_RMNET_DATA_MAX_IFACES <= conn_id)
    {
      QMI_DEBUG_MSG("Invalid dev id [%s] passed", dev_id);
      return QMI_CONN_ID_INVALID;
    }

    if (DS_TARGET_MSM8994 == target ||
        DS_TARGET_MSM8992 == target)
    {
      /* MSM8994 would uses first conn id and appropriate mux binding */
      rc = QMI_CONN_ID_RMNET_0;
      ds_get_epid(QMI_PORT_RMNET_IPA_0, ep_type, epid);
      *mux_id = conn_id + 1;
    }
    else if (DS_TARGET_FUSION4_5_PCIE == target)
    {
      /* Fusion4.5 with PCIe would use single MHI port and mux port binding. */
      rc = QMI_CONN_ID_RMNET_MHI_0;
      ds_get_epid(QMI_PORT_RMNET_MHI_0, ep_type, epid);
      *mux_id = conn_id + 1;
    }
    else if (DS_TARGET_MDM == target)
    {
      /* MDM targets (Fusion3, Fusion4, Fusion4.5 HSIC etc) would use rmnet_usb
          ports and no mux port binding. */
      rc = (int) QMI_CONN_ID_RMNET_USB_0 + conn_id;
    }
    else if (DS_TARGET_LE_MDM9X35 == target)
    {
      /* LE 9x35 uses first conn_id and uses appropriate mux port binding */
      rc = QMI_CONN_ID_RMNET_0;
      ds_get_epid(QMI_PORT_RMNET_IPA_0, ep_type, epid);
      *mux_id = conn_id + 1;
    }
    else if(DS_TARGET_MSM == target)
    {
      /* MSM targets use rmnet interface and no mux port binding */
      rc = (int) QMI_CONN_ID_RMNET_0 + conn_id;
    }
    else if (DS_TARGET_DPM_2_0 == target ||
             DS_TARGET_JOLOKIA == target )
    {
      /* DPM 2.0 */
      rc = QMI_CONN_ID_RMNET_0;
      *mux_id = conn_id + 1;
      ds_get_epid(QMI_PORT_RMNET_0, ep_type, epid);
    }

    else if(DS_TARGET_LE_MDM9X15 == target ||
            DS_TARGET_LE_MDM9X25 == target ||
            DS_TARGET_LE_LEGACY == target)
    {
      /* Older LE targets use rmnet ports with no mux port binding */
      rc = (int) QMI_CONN_ID_RMNET_0 + conn_id;
    }
  }
#ifdef FEATURE_QMI_IWLAN
  else if (!strncmp(dev_id, QMI_PLATFORM_REV_RMNET_DATA_PREFIX,
               strlen(QMI_PLATFORM_REV_RMNET_DATA_PREFIX)))
  {
    /* Reverse rmnet channel */
    conn_id = ds_atoi(dev_id + strlen(QMI_PLATFORM_REV_RMNET_DATA_PREFIX))
              + QMI_PLATFORM_NUM_FORWARD_CONN_IDS;
    if (0 > conn_id || QMI_PLATFORM_RMNET_DATA_MAX_IFACES <= conn_id)
    {
      QMI_DEBUG_MSG("Invalid dev id [%s] passed", dev_id);
      return QMI_CONN_ID_INVALID;
    }

    if (DS_TARGET_MSM8994 == target ||
        DS_TARGET_MSM8992 == target)
    {
      /* MSM8994 would uses first conn id and appropriate mux binding */
      rc = QMI_CONN_ID_RMNET_0;
      ds_get_epid(QMI_PORT_RMNET_IPA_0, ep_type, epid);
      *mux_id = conn_id + 1;
    }
    else if (DS_TARGET_DPM_2_0 == target ||
             DS_TARGET_JOLOKIA == target)
    {
      /* DPM 2.0 */
      rc = QMI_CONN_ID_RMNET_0;
      *mux_id = conn_id + 1;
      ds_get_epid(QMI_PORT_RMNET_0, ep_type, epid);
    }
    else if (DS_TARGET_FUSION4_5_PCIE == target)
    {
      /* Fusion4.5 with PCIe would use single MHI port and mux port binding. */
      rc = QMI_CONN_ID_RMNET_MHI_0;
      ds_get_epid(QMI_PORT_RMNET_MHI_0, ep_type, epid);
      *mux_id = conn_id + 1;
    }
    else if (DS_TARGET_MDM == target)
    {
      /* MDM targets (Fusion3, Fusion4, Fusion4.5 HSIC etc) would use rmnet_usb
       * ports and no mux port binding. */
      rc = (int) QMI_CONN_ID_REV_RMNET_USB_0
          + (conn_id - QMI_PLATFORM_NUM_FORWARD_CONN_IDS);
    }
    else if(DS_TARGET_MSM == target)
    {
      /* MSM targets use rmnet interface and no mux port binding */
      rc = (int) QMI_CONN_ID_REV_RMNET_0
          + (conn_id - QMI_PLATFORM_NUM_FORWARD_CONN_IDS);
    }
  }
#endif /* FEATURE_QMI_IWLAN */
  else
  {
    /* Default */
    rc = qmi_linux_get_conn_id_by_name(dev_id);
    QMI_DEBUG_MSG("qmi_linux_get_conn_id_by_name: returned [%d]", rc);
    if (QMI_CONN_ID_RMNET_0 == rc)
    {
      conn_id = ds_atoi(dev_id + strlen(QMI_PLATFORM_RMNET_PREFIX));
      if (0 > conn_id || QMI_PLATFORM_RMNET_DATA_MAX_IFACES <= conn_id)
      {
        QMI_DEBUG_MSG("Invalid dev id [%s] passed", dev_id);
      }
      else
      {
        ds_get_epid(QMI_PORT_RMNET_0, ep_type, epid);
        *mux_id = conn_id + 1;
      }
    }
  }

  QMI_DEBUG_MSG_3("Target: %s, dev_name %s, conn_name %s",
                   ds_get_target_str(target), dev_id, qmi_linux_get_name_by_conn_id(rc));
  QMI_DEBUG_MSG_2("mux_id %d, epid 0x%x", *mux_id, *epid);

  return rc;
}

#ifdef FEATURE_DATA_LOG_QXDM

/*=========================================================================
  FUNCTION:  qmi_format_diag_log_msg

===========================================================================*/
/*!
    @brief
    Format debug message for logging.

    @return
    None
*/
/*=========================================================================*/
void qmi_format_diag_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
)
{
  va_list ap;

  /*-----------------------------------------------------------------------*/

  if ( buf_ptr == NULL || buf_size <= 0 )
  {
    return;
  }

  /*-----------------------------------------------------------------------*/

  va_start( ap, fmt );

  vsnprintf( buf_ptr, (size_t)buf_size, fmt, ap );

  va_end( ap );

} /* dsc_format_log_msg */

#endif /* FEATURE_DATA_LOG_QXDM */


const char * qmi_linux_get_internal_use_port
(
  void
)
{
  const char *rc = NULL;
  ds_target_t target;
  const char *target_str;

  target = ds_get_target();
  target_str = ds_get_target_str(target);

  QMI_DEBUG_MSG_2 ("qmuxd: get_internal_port(): Target Configuration: [%d]: [%s]\n", target, target_str);

  /* CSFB target has only SDIO transport ports */
  if (DS_TARGET_MDM == target)
  {
    rc = QMI_PORT_RMNET_USB_0;
  }
  /* MDMUSB target replaces SDIO transport with USB, suppresses SMD/BAM */
  else if (DS_TARGET_CSFB == target)
  {
    rc = QMI_PORT_RMNET_SDIO_0;
  }
  else if (DS_TARGET_SGLTE == target || DS_TARGET_DSDA3 == target)
  {
    rc = QMI_PORT_RMNET_SMUX_0;
  }
  else if (DS_TARGET_DSDA == target)
  {
    rc = QMI_PORT_RMNET_USB_0;
  }
  else if (DS_TARGET_DSDA2 == target)
  {
    rc = QMI_PORT_RMNET_USB_0;
  }
  else if (DS_TARGET_FUSION4_5_PCIE == target)
  {
    rc = QMI_PORT_RMNET_MHI_0;
  }
  else
  {
    rc = QMI_PORT_RMNET_0;
  }

  if (rc)
  {
    QMI_DEBUG_MSG_1 ("Setting internal use port to %s\n",rc);
  }
  else
  {
    QMI_ERR_MSG_0 ("Internal use port is unset!!\n");
  }

  return rc;
}

/*=========================================================================
  FUNCTION:  qmi_platform_log_raw_qmi_msg

===========================================================================*/
/*!
    @brief
    Logs the raw QMI message

    @return
    None
*/
/*=========================================================================*/
void qmi_platform_log_raw_qmi_msg
(
  const unsigned char  *msg,
  int                  msg_len
)
{
  int i, j;
  char buff[QMI_PLATFORM_NUM_CHARS_PER_BYTE*QMI_PLATFORM_NUM_BYTES_PER_LINE+1] = "";
  unsigned char upper_half;
  unsigned char lower_half;
  const unsigned char *data = msg;

  if (NULL == msg)
  {
    return;
  }

  for (i = 1, j = 0; i <= msg_len; ++i, ++data)
  {
    upper_half = (*data) >> 4;
    lower_half = (*data) & 0x0F;
    buff[j++]  = QMI_PLATFORM_GET_HEX_CHAR(upper_half);
    buff[j++]  = QMI_PLATFORM_GET_HEX_CHAR(lower_half);
    buff[j++]  = ' ';

    if (i % QMI_PLATFORM_NUM_BYTES_PER_LINE == 0)
    {
      buff[j] = '\0';
      QMI_DEBUG_MSG_1 ("%s\n", buff);
      j = 0;
    }
  }

  /* Print any remaining data */
  if (j > 0)
  {
    buff[j] = '\0';
    QMI_DEBUG_MSG_1 ("%s\n", buff);
  }
}

#ifdef FEATURE_DATA_LOG_FILE
/*=========================================================================
  FUNCTION:  qmi_platform_get_current_time

===========================================================================*/
/*!
    @brief
    Get the current time

    @return
    None
*/
/*=========================================================================*/
void qmi_platform_get_current_time(char *str, size_t str_size)
{
  time_t t;
  struct tm *tmp;
  struct timespec tp;

  t = time(NULL);
  tmp = localtime(&t);

  if (tmp == NULL)
  {
    perror("localtime");
  }

 clock_gettime(CLOCK_REALTIME, &tp);

 snprintf(str, str_size, "%02d-%02d %02d:%02d:%02d.%03d",
          tmp->tm_mon+1,
          tmp->tm_mday,
          tmp->tm_hour,
          tmp->tm_min,
          tmp->tm_sec,
          (int)(tp.tv_nsec/1000000));
}
#endif /* FEATURE_DATA_LOG_FILE */
