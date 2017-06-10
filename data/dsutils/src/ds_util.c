/******************************************************************************

                        D S _ U T I L . C

******************************************************************************/

/******************************************************************************

  @file    ds_util.c
  @brief   Data Services Utility Functions Implementation File

  DESCRIPTION
  Implementation file for DS utility functions.

  ---------------------------------------------------------------------------
  Copyright (c) 2008,2010,2011,2013,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id: ds_util.c,v 1.2 2010/02/12 20:16:45 randrew Exp $

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/04/08   vk         Initial version

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

#ifndef FEATURE_DSUTILS_OFFTARGET
#include <sys/capability.h>
#endif

#ifdef FEATURE_DSUTILS_OFFTARGET
#include <sys/types.h>
#include <sys/syscall.h>
#endif

#include "ds_string.h"
#include "ds_util.h"
#include <netdb.h>
#include <stdint.h>
#include <linux/if.h>
#include <sys/socket.h>
#include <sys/wait.h>

#ifdef FEATURE_DS_UTILS_NON_EMULATOR
#include <linux/rmnet_data.h>
#include <linux/msm_rmnet.h>
#endif

#ifdef FEATURE_DATA_LOG_SYSLOG
#include <syslog.h>
#endif

#ifdef FEATURE_DATA_LOG_ADB
#include <utils/Log.h>

#ifndef DS_LOG_TAG
#define DS_LOG_TAG "QC-DS-LIB"
#endif

#endif /*FEATURE_DATA_LOG_ADB*/

#include <linux/capability.h>
#include <linux/prctl.h>

#ifdef FEATURE_DS_LINUX_ANDROID

#ifndef FEATURE_DSUTILS_OFFTARGET
#include <private/android_filesystem_config.h>
#endif

#include <cutils/properties.h>
#include "mdm_detect.h"
#endif


/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
/*---------------------------------------------------------------------------
  Undef unsupported logging
---------------------------------------------------------------------------*/
#ifndef FEATURE_DATA_LOG_ADB
#undef LOG
#define LOG(...)
#define LOG_ERROR
#endif

#ifndef FEATURE_DATA_LOG_QXDM
#define MSG_SPRINTF_1(...)
#define MSG_SSID_LINUX_DATA
#define MSG_LEGACY_LOW
#endif


/*---------------------------------------------------------------------------
   Log file pointer. All log messages are written to this file
---------------------------------------------------------------------------*/
FILE * ds_logFp = NULL;

/*---------------------------------------------------------------------------
   Constant string prepended to every logged message
---------------------------------------------------------------------------*/
#define DS_LOG_PREFIX "DS: "

#define DS_LOG_MAXLEN 128
#define DS_UTIL_MAX_LOG_MSG_SIZE  512
#define DS_UTIL_MAX_CMD_LENGTH    512
/*---------------------------------------------------------------------------
   Collection of program's logging configuration information
---------------------------------------------------------------------------*/
static ds_log_cfg_t ds_log_cfg;

/*---------------------------------------------------------------------------
   Constant table mapping DSC debug levels to syslog message levels
---------------------------------------------------------------------------*/
#ifdef FEATURE_DATA_LOG_SYSLOG
static const int ds_log_syslog_level_map[] = {
    LOG_INFO,       /* DS_DBG_LEVEL_LOW */
    LOG_NOTICE,     /* DS_DBG_LEVEL_MEDIUM */
    LOG_WARNING,    /* DS_DBG_LEVEL_HIGH */
    LOG_ERR         /* DS_DBG_LEVEL_ERROR */
};
#endif

#if defined(FEATURE_DATA_LOG_ADB) || defined(FEATURE_DATA_LOG_STDERR)
static const int ds_log_syslog_level_map[] = {
    1,              /* DS_DBG_LEVEL_LOW */
    2,              /* DS_DBG_LEVEL_MEDIUM */
    3,              /* DS_DBG_LEVEL_HIGH */
    4               /* DS_DBG_LEVEL_ERROR */
};
#endif

/*---------------------------------------------------------------------------
   Constant values related to Android net.logmask property
---------------------------------------------------------------------------*/
#define DS_LOG_KEY "persist.net.logmask"
#define DS_LOG_MAX_LEN 10 /* qxdm, adb, stdout etc. */
#define DS_LOG_MAX_NUM_MASKS 3
#define DS_LOG_MASK_DELIM ":"
#define DS_LOG_MASK_QXDM_STR "qxdm"
#define DS_LOG_MASK_ADB_STR "adb"
#define DS_LOG_MASK_STDOUT_STR "stdout"
#define DS_LOG_MASK_QXDM 0x01
#define DS_LOG_MASK_ADB 0x02
#define DS_LOG_MASK_STDOUT 0x04
/* default log mask is set to qxdm only */
static int ds_log_mask = DS_LOG_MASK_QXDM;

#define DS_DEVICE_RMNET_MHI "rmnet_mhi"
#define DS_DEVICE_RMNET_IPA "rmnet_ipa"
#define DS_DEVICE_RMNET     "rmnet0"

/*---------------------------------------------------------------------------
   Target configuration property name and values
---------------------------------------------------------------------------*/
#define DS_BASEBAND_PROP                "ro.baseband"
#define DS_BASEBAND_PROP_MSM            "msm"
#define DS_BASEBAND_PROP_APQ            "apq"
#define DS_BASEBAND_PROP_SVLTE1         "svlte1"
#define DS_BASEBAND_PROP_SVLTE2         "svlte2a"
#define DS_BASEBAND_PROP_CSFB           "csfb"
#define DS_BASEBAND_PROP_SGLTE          "sglte"
#define DS_BASEBAND_PROP_SGLTE2         "sglte2"
#define DS_BASEBAND_PROP_DSDA           "dsda"
#define DS_BASEBAND_PROP_DSDA2          "dsda2"
#define DS_BASEBAND_PROP_DSDA3          "dsda3"
#define DS_BASEBAND_PROP_MDM            "mdm"
#define DS_BASEBAND_PROP_MDM2           "mdm2"

#define DS_TARGET_PROP                  "persist.data.target"
#define DS_TARGET_PROP_FUSION4_5_PCIE   "fusion4_5_pcie"
#define DS_TARGET_PROP_FUSION4_5_HSIC   "fusion4_5_hsic"
#define DS_TARGET_PROP_DPM1             "dpm1"

#define DS_ESOC_MDM_NAME_9X25           "MDM9x25"
#define DS_ESOC_MDM_NAME_9X35           "MDM9x35"

#define DS_ESOC_MDM_LINK_HSIC           "HSIC"
#define DS_ESOC_MDM_LINK_PCIE           "PCIe"
#define DS_ESOC_MDM_LINK_HSIC_PCIE      "HSIC+PCIe"

#define DS_BOARD_PLATFORM_PROP          "ro.board.platform"
#define DS_BOARD_PLATFORM_MSM8994       "msm8994"
#define DS_BOARD_PLATFORM_MSM8916       "msm8916"
#define DS_BOARD_PLATFORM_MSM8936       "msm8936"
#define DS_BOARD_PLATFORM_MSM8939       "msm8939"
#define DS_BOARD_PLATFORM_MSM8909       "msm8909"
#define DS_BOARD_PLATFORM_MSM8929       "msm8929"
#define DS_BOARD_PLATFORM_MSM8629       "msm8629"
#define DS_BOARD_PLATFORM_MSM8229       "msm8229"
#define DS_BOARD_PLATFORM_MSM8609       "msm8609"

typedef struct ds_target_str_map_s
{
  ds_target_t target;
  const char *target_str;
} ds_target_str_map_t;

static const ds_target_str_map_t ds_target_str_map[] =
{
    { DS_TARGET_UNDEFINED,        "undefined"       },
    { DS_TARGET_MSM,              "msm"             },
    { DS_TARGET_MSM8994,          "msm8994"         },
    { DS_TARGET_APQ,              "apq"             },
    { DS_TARGET_SVLTE1,           "svlte1"          },
    { DS_TARGET_SVLTE2,           "svlte2a"         },
    { DS_TARGET_CSFB,             "csfb"            },
    { DS_TARGET_SGLTE,            "sglte"           },
    { DS_TARGET_SGLTE2,           "sglte2"          },
    { DS_TARGET_DSDA,             "dsda"            },
    { DS_TARGET_DSDA2,            "dsda2"           },
    { DS_TARGET_DSDA3,            "dsda3"           },
    { DS_TARGET_MDM,              "mdm"             },
    { DS_TARGET_FUSION4_5_PCIE,   "fusion4_5_pcie"  },
    { DS_TARGET_LE_MDM9X35,       "mdm9x35-le"      },
    { DS_TARGET_LE_MDM9X25,       "mdm9x25-le"      },
    { DS_TARGET_LE_MDM9X15,       "mdm9x15-le"      },
    { DS_TARGET_LE_LEGACY,        "legacy-le"       },
    { DS_TARGET_DPM_2_0,          "dpm_2_0"         },
    { DS_TARGET_JOLOKIA,          "jolokia"         },
    { DS_TARGET_MSM8992,          "msm8992"         },
};

#if (defined(FEATURE_DATA_LOG_SYSLOG) || defined (FEATURE_DATA_LOG_ADB))
/*===========================================================================
  FUNCTION  ds_log_syslog_level
===========================================================================*/
/*!
@brief
  Returns the syslog logging levelg given a debug level.

@return
  int - syslog logging level

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static __inline__ int
ds_log_syslog_level (ds_dbg_level_t dbglvl)
{
    return ds_log_syslog_level_map[dbglvl];
}
#endif

/*===========================================================================
  FUNCTION  ds_drop_bounding_set_caps
===========================================================================*/
/*!
@brief
  Drops the capabilities other than the ones provided in the required caps
  input parameter from the capability bounding set.

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void ds_drop_bounding_set_caps(uint64_t req_caps)
{
  uint32_t cap;

  /* update the capabilities */
  for (cap = 0; cap <= CAP_LAST_CAP; cap++)
  {
    uint64_t cap_mask = (1ULL << cap);

    if (req_caps & cap_mask)
    {
      continue;
    }

    if (prctl(PR_CAPBSET_READ, cap, 0, 0, 0) > 0)
    {
      if (prctl(PR_CAPBSET_DROP, cap, 0, 0, 0) < 0)
      {
        ds_log_high("failed to drop capability=%d, errno=[%d:%s]\n",
                    cap, errno, strerror(errno));
      }
    }
  }
}

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  ds_log_init
===========================================================================*/
/*!
@brief
  Initializes logging to use the specified log file.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void ds_log_init(FILE * logfp) {
    if (logfp) {
        ds_logFp = logfp;
    }
}

/*===========================================================================
  FUNCTION  ds_log_init2
===========================================================================*/
/*!
@brief
  Initialization routine for logging functionality.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
ds_log_init2 (int threshold, int mode)
{
#ifdef FEATURE_DATA_LOG_SYSLOG
    /* Use default debug level if an invalid debug level was specified on
    ** the command line.
    */
    threshold = (threshold < (int)DS_DBG_LEVEL_MIN) ?
        (int)DS_DBG_LEVEL_DFLT : threshold;
    threshold = (threshold > (int)DS_DBG_LEVEL_MAX) ?
        (int)DS_DBG_LEVEL_DFLT : threshold;

    /* Set debug level in configuration blob */
    ds_log_cfg.threshold = (ds_dbg_level_t)threshold;

    /* Use default logging mode if an invalid log mode was specified on the
    ** command line.
    */
    mode = (mode < (int)DS_LOG_MODE_MIN) ?
        (int)DS_LOG_MODE_DFLT : mode;
    mode = (mode > (int)DS_LOG_MODE_MAX) ?
        (int)DS_LOG_MODE_DFLT : mode;

    /* Set logging mode in configuration blob */
    ds_log_cfg.mode = (ds_log_mode_t)mode;

    switch (ds_log_cfg.mode)
    {
    case DS_LOG_MODE_SYSLOG:
        /* If logging to syslog, initialize logging */
        openlog(DS_LOG_PREFIX, LOG_NDELAY, LOG_USER);
        break;
    default:
        break;
    }
#else
    (void)threshold; (void)mode;
#endif /* FEATURE_DATA_LOG_SYSLOG */

    /* Also initialize logging to stderr for ds utility functions; note that
    ** this may be null device if dsc is running as a daemon process.
    */
    ds_log_init(stderr);

    return;
}

/*===========================================================================
  FUNCTION:  ds_log_set_mask
===========================================================================*/
/*!
    @brief
    enables log mask based on the given token

    @return
    none
*/
/*=========================================================================*/
void ds_log_set_mask(const char * token)
{
  if (!std_stricmp(token, DS_LOG_MASK_QXDM_STR))
  {
    ds_log_mask |= DS_LOG_MASK_QXDM;
  }
  else if (!std_stricmp(token, DS_LOG_MASK_ADB_STR))
  {
    ds_log_mask |= DS_LOG_MASK_ADB;
  }
  else if (!std_stricmp(token, DS_LOG_MASK_STDOUT_STR))
  {
    ds_log_mask |= DS_LOG_MASK_STDOUT;
  }
  else
  {
    ds_log_err("ds_log_set_mask: %s token not recognized", token);
  }
}

/*===========================================================================
  FUNCTION  ds_log_multicast_init
===========================================================================*/
/*!
@brief
  Initializes logging to use Android property persist.net.logmask to enable
  logging for various output streams. This function will read the property
  to set logmask bits.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void ds_log_multicast_init()
{
#if (!defined(FEATURE_DSUTILS_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))
  char value[PROPERTY_VALUE_MAX];
  char default_value[PROPERTY_VALUE_MAX] = "QXDM";
  char * token = NULL;
  char *save_ptr = NULL;
  int i=0;

  /* read persist.net.logmask property */
  property_get(DS_LOG_KEY, value, default_value);
  token = strtok_r(value, DS_LOG_MASK_DELIM, &save_ptr);

  for(i=0; i<DS_LOG_MAX_NUM_MASKS && NULL!=token; i++)
  {
    ds_log_set_mask(token);
    token = strtok_r(NULL, DS_LOG_MASK_DELIM, &save_ptr);
  }
#endif
}

/*=========================================================================
  FUNCTION:  ds_format_log_msg

===========================================================================*/
/*!
    @brief
    Format debug message for logging.

    @return
    None
*/
/*=========================================================================*/
void ds_format_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
)
{
  va_list ap;

  /*-----------------------------------------------------------------------*/

  /* validate input param */
  if( NULL == buf_ptr || buf_size <= 0)
  {
    ds_log_err("ds_format_log_msg: Bad Param buf_ptr:[%p], buf_size:%d",buf_ptr, buf_size);
    return;
  }

  /*-----------------------------------------------------------------------*/

  va_start( ap, fmt );

  vsnprintf( buf_ptr, (size_t)buf_size, fmt, ap );

  va_end( ap );

} /* ds_format_log_msg */


/*===========================================================================
  FUNCTION  ds_log_write
===========================================================================*/
/*!
@brief
  Log printf-style formatted string using specified debug level.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
ds_log_write
(
    ds_dbg_level_t dbglvl,
    int ln,
    const char * fmt,
    ...
)
{
  va_list arglist;
  char prep_fmt[DS_LOG_MAXLEN];

  if(fmt == NULL)
  {
    return;
  }
  memset(prep_fmt, 0, (size_t)DS_LOG_MAXLEN);

  /* Only log message if logging threshold is hit or exceeded */
  if (dbglvl < ds_log_cfg.threshold)
  {
    return;
  }

  /* Prepend message with source filename and line number */
  snprintf(prep_fmt, (size_t)DS_LOG_MAXLEN, "%d: ", ln);
  strlcat(prep_fmt, fmt, (size_t)DS_LOG_MAXLEN);

  /* We get a compiler warning if the last name argument in the function
  ** is not passed to va_start, so set fmt to prep_fmt.
  */
  fmt = prep_fmt;

  va_start(arglist, fmt);

  /* Log message appropriately based on log mode */
  switch (ds_log_cfg.mode)
  {
    case DS_LOG_MODE_FPRINTF:
      /* Print message on stderr if log mode is printf */
      (void)vfprintf(stderr, fmt, arglist);
      fprintf(stderr, "%s", "\n");
      break;
    case DS_LOG_MODE_SYSLOG:
      /* Log to syslog if log mode is syslog */
#ifdef FEATURE_DATA_LOG_SYSLOG
      vsyslog(ds_log_syslog_level(dbglvl), fmt, arglist);
#endif
#ifdef FEATURE_DATA_LOG_ADB
      switch (ds_log_syslog_level(dbglvl))
      {
        case 0:
          ds_log_low( fmt, arglist );
          break;

        case 1:
          ds_log_high( fmt, arglist );
          break;

        case 2:
          ds_log_high( fmt, arglist );
          break;

        case 3:
          ds_log_err( fmt, arglist );
          break;

        default:
          ds_log_dflt( fmt, arglist );
      }/* switch() */
#endif
      break;
    default:
      /* This should never be reached */
      ds_abort();
  }

  va_end(arglist);

  return;
}

/*===========================================================================
  FUNCTION  ds_log_multicast
===========================================================================*/
/*!
@brief
  Log to various output streams based on the logmask. If n bits are set
  in the mask, enable corresponding streams for logging, thus the messages
  should be sent to n different output streams

  Example: if bit for ADB and STDOUT are set in the logmask, the log
  messages are sent to ADB as well as STDOUT.

@return
  none

@note

  - Dependencies
    - log bit mask must be set at power up
    - FEATURE_DATA_LOG_QXDM and FEATURE_DATA_LOG_ADB must be defined.

  - Side Effects
    - log messages are sent to one or more output streams
*/
/*=========================================================================*/
void ds_log_multicast(int lvl, char * fmt, ...)
{
  char buf[DS_UTIL_MAX_LOG_MSG_SIZE];
  va_list ap;
  int adb_lvl = 0;

  va_start( ap, fmt );
  vsnprintf( buf, (size_t)DS_UTIL_MAX_LOG_MSG_SIZE, fmt, ap );
  va_end( ap );

  if (ds_log_mask & DS_LOG_MASK_STDOUT)
  {
    /* print log message to stdout */
    fprintf(stdout, "%s", buf);
    fprintf(stdout, "%s", "\n");
  }

  /* adb logcat */
  /* TODO: only LOG_ERROR msgs show up in logcat, investigate
   *       later as why that is so and then we can use all logging
   *       levels  */
  switch(lvl)
  {
  case DS_DBG_LEVEL_LOW:
    if (ds_log_mask & DS_LOG_MASK_ADB)
    {
      /* should be LOG_DEBUG */
      LOG(LOG_ERROR, DS_LOG_TAG, "%s", buf);
    }
    if (ds_log_mask & DS_LOG_MASK_QXDM)
    {
      MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_LOW, "%s", buf);
    }
    break;
  case DS_DBG_LEVEL_MEDIUM:
    if (ds_log_mask & DS_LOG_MASK_ADB)
    {
      /* should be LOG_INFO */
      LOG(LOG_ERROR, DS_LOG_TAG, "%s", buf);
    }
    /* Until we figure out how to pass caller file/line to diag
     * macro, we will rely on caller to call this from their land
    if (ds_log_mask & DS_LOG_MASK_QXDM)
    {
      MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_MED, "%s", buf);
    }
    */
    break;
  case DS_DBG_LEVEL_HIGH:
    if (ds_log_mask & DS_LOG_MASK_ADB)
    {
      /* should be LOG_INFO */
      LOG(LOG_ERROR, DS_LOG_TAG, "%s", buf);
    }
    /* Until we figure out how to pass caller file/line to diag
     * macro, we will rely on caller to call this from their land
    if (ds_log_mask & DS_LOG_MASK_QXDM)
    {
      MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, "%s", buf);
    }
    */
    break;
  case DS_DBG_LEVEL_ERROR:
    if (ds_log_mask & DS_LOG_MASK_ADB)
    {
      LOG(LOG_ERROR, DS_LOG_TAG, "%s", buf);
    }
    /* Until we figure out how to pass caller file/line to diag
     * macro, we will rely on caller to call this from their land
    if (ds_log_mask & DS_LOG_MASK_QXDM)
    {
      MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_ERROR, "%s", buf);
    }
    */
    break;
  default:
    if (ds_log_mask & DS_LOG_MASK_ADB)
    {
      /* should be LOG_INFO */
      LOG(LOG_ERROR, DS_LOG_TAG, "%s", buf);
    }
    /* Until we figure out how to pass caller file/line to diag
     * macro, we will rely on caller to call this from their land
    if (ds_log_mask & DS_LOG_MASK_QXDM)
    {
      MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, "%s", buf);
    }
    */
    break;
  }
}

/*===========================================================================
  FUNCTION  ds_atoi
===========================================================================*/
/*!
@brief
  since stdlib atoi and strtol can't distinguish between "0" and "invalid
  numeric string", and returns 0 in both the cases, we need our own
  version of atoi.

@return
  int - numeric value of string (>=0) on success, -1 otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int ds_atoi (const char * str)
{
  int ret = -1;
  char * tmp_str = (char *)str;

  if (NULL == tmp_str)
  {
    return ret;
  }

  ret = atoi(tmp_str);

  if (ret == 0)
  {
    ret = -1;
    /* if we find at least one digit in str,
       that means atoi really converted it to 0,
       other wise atoi returned 0 because string
       did not have any digits */
    do
    {
      if (isdigit(*tmp_str))
      {
  ret = 0;
  break;
      }

      tmp_str++;
    } while (*tmp_str != '\0');

    if (ret == -1)
    {
      ds_log_err("string %s does not contain any valid digits",
      str);
    }

  }

  return ret;
}


/*===========================================================================
  FUNCTION  ds_malloc
===========================================================================*/
/*!
@brief
  A general purpose, reentrant memory allocator.

@return
  void * - pointer to memory block

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void *
ds_malloc (size_t size)
{
  void* ptr = malloc(size);
  return ptr;
}

/*===========================================================================
  FUNCTION  ds_free
===========================================================================*/
/*!
@brief
 Deallocates memory previously allocated using ds_malloc(). This is a
 reentrant function.

@return
  void

@note

  - Dependencies
    - Given memory block must have been allocated using ds_malloc().

  - Side Effects
    - None
*/
/*=========================================================================*/
void
ds_free (void * ptr)
{
  free(ptr);
}


/*===========================================================================
  FUNCTION  ds_system_call
===========================================================================*/
/*!
@brief
  Execute a shell command.

@return
  int - numeric value 0 on success, -1 otherwise

@note
  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
int
ds_system_call
(
  const char   *command,
  unsigned int  cmdlen
)
{
  int result = 0;
  FILE *stream = NULL;
  unsigned int vallen = (unsigned int)std_strlen( command );

  if( vallen != cmdlen ) {
    ds_log_err( "system call length mismatch: %d != %d", cmdlen, vallen );
    return -1;
  }

  ds_log_med("system call: %s", command);

  stream = popen( command, "w" );
  if( stream == NULL )
  {
    ds_log_sys_err("system command failed");
    result = -1;
  }
  else if( 0 > pclose( stream ) )
  {
    ds_log_sys_err("pclose command failed");
  }

  return result;
}

/*===========================================================================
  FUNCTION  ds_system_call2
===========================================================================*/
/*!
@brief
  Execute a shell command with message logging control capability.

@return
  int - numeric value 0 on success, -1 otherwise

@note
  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
int
ds_system_call2
(
  const char   *command,
  unsigned int  cmdlen,
  boolean       logmsg
)
{
  int result = 0;
  FILE *stream = NULL;
  unsigned int vallen = (unsigned int)std_strlen( command );

  if( vallen != cmdlen ) {
    ds_log_err( "system call length mismatch: %d != %d", cmdlen, vallen );
    return -1;
  }

  if ((boolean)TRUE == logmsg)
  {
    ds_log_med("system call: %s", command);
  }

  stream = popen( command, "w" );
  if( stream == NULL )
  {
    ds_log_sys_err("system command failed");
    result = -1;
  }
  else if( 0 > pclose( stream ) )
  {
    ds_log_sys_err("pclose command failed");
  }

  return result;
}

/*===========================================================================
  FUNCTION  ds_system_call3
===========================================================================*/
/*!
@brief
  Execute a shell command with message logging control capability

@param
command        - The command string to execute
cmdlen         - The length of the command string
cmd_result     - The return status string
cmd_result_len - Length of return status string

@return
  int - numeric value 0 on success, -1 otherwise

@note
  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
int ds_system_call3
(
  const char    *command,
  unsigned int  cmdlen,
  char          *cmd_result,
  unsigned int  cmd_result_len,
  boolean       logmsg
)
{
  int result = 0;
  int return_val = 0;
  FILE *stream = NULL;
  unsigned int vallen = (unsigned int)std_strlen( command );

  /* cmd_string - Redirect stdout of command to stderr
   * err_string - Storing stderr output */
  char cmd_string[DS_UTIL_MAX_CMD_LENGTH] = "";
  char err_string[DS_UTIL_MAX_CMD_LENGTH] = "";

  if( vallen != cmdlen )
  {
    ds_log_err("%s(): Length mismatch: %d != %d", __func__, cmdlen, vallen);
    return -1;
  }

  if ((boolean)TRUE == logmsg)
  {
    ds_log_med("%s(): Command to execute: %s", __func__, command);
  }

  /* Redirect stdout to stderr so that we can read the stream to
   * check for return codes or error messages */
  snprintf(cmd_string,
           sizeof(cmd_string),
           "%s 2>&1",
           command);

  stream = popen( cmd_string, "r" );
  if( stream == NULL )
  {
    ds_log_err("%s(): Failed to execute command!", __func__);
    result = -1;
    goto bail;
  }

  /* Read the stream to check for return message from the command */
  while (fgets(err_string, sizeof(err_string), stream) != NULL)
  {
    /* Some commands like 'ndc' will print status for
     * both successful and failed execution. It is up
     * to the client to decipher the error string and
     * determine success or failure. The client can
     * check the return buffer contents and proceed
     * accordingly */
    ds_log_med("%s(): %s", __func__, err_string);
    strlcat( cmd_result, err_string, cmd_result_len );
  }

  /* Check if there was any error since fgets returns
   * NULL for both error and end-of-file */
  if (ferror(stream))
  {
    ds_log_err("%s(): Error reading command output stream", __func__);
    result = -1;
  }

  /* Close the stream */
  return_val = WIFEXITED(pclose(stream));

  /* Print the return code */ 
  ds_log_low("%s(): Return code [%d]", __func__, return_val);

  if( 0 > return_val)
  {
    ds_log_sys_err("pclose command failed");
  }

bail:
  return result;
}

/*===========================================================================
  FUNCTION  ds_change_user_cap
===========================================================================*/
/*!
@brief
  Changes the uid/gid and sets the capabilities. uid is a system user id,
  gid is a system group id.  Capabilities should be passed as per
  requirement of capset system call.

@return
  int - numeric value 0 on success, -1 otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
ds_change_user_cap
(
  int uid,
  int gid,
  uint64_t caps
)
{
  struct __user_cap_data_struct cap_data[2];
  struct __user_cap_header_struct cap_hdr;
  int ret = -1;
  uid_t curr_uid=0;
  gid_t curr_gid=0;
  uint32_t cap;

  do
  {
    if (uid < 1)
    {
      ds_log_err("not allowed to set uid to [%d]", uid);
      break;
    }

    if (0 == caps)
    {
      ds_log_err("not allowed wipe out all capabilities");
      break;
    }

    /* make sure prior capabilities are retained when uid is changed */
    if (0 != prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0))
    {
      ds_log_sys_err("could not set PR_SET_KEEPCAPS on this process");
      break;
    }

    ds_drop_bounding_set_caps(caps|(1ULL << CAP_SETUID)|(1ULL << CAP_SETGID));

    /* change gid */
    if (DS_UTIL_INVALID_GID != gid)
    {
      curr_gid = getgid();
      if (0 != setgid((gid_t)gid))
      {
        ds_log_sys_err("could not set gid");
        break;
      }
      else
      {
        ds_log_high("process now running as [%d] gid", gid);
      }
    }

    /* change uid */
    curr_uid = getuid();
    if (0 != setuid((uid_t)uid))
    {
      ds_log_sys_err("could not set uid");
      break;
    }
    else
    {
      ds_log_high("process now running as [%d] uid", uid);
    }

    memset(cap_data, 0, sizeof(cap_data));
    memset(&cap_hdr, 0, sizeof(cap_hdr));

    cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
    /* 0 is considered self pid */
    cap_hdr.pid = 0;

    /* display current capabilities */
    if (0 != capget(&cap_hdr, cap_data))
    {
      ds_log_sys_err("capget failed");
      break;
    }
    else
    {
      ds_log_high("permitted set = [0x%x] [0x%x]", cap_data[0].permitted, cap_data[1].permitted);
      ds_log_high("effective set = [0x%x] [0x%x]", cap_data[0].effective, cap_data[1].effective);
    }

    /* limit the permitted and effective sets to the capabilites that we really need */
    memset(cap_data, 0, sizeof(cap_data));

    /* update the capabilities */
    for (cap = 0; cap <= CAP_LAST_CAP; cap++)
    {
      uint64_t cap_mask = (1ULL << cap);

      if (caps & cap_mask)
      {
        cap_data[CAP_TO_INDEX(cap)].permitted |= CAP_TO_MASK(cap);
        cap_data[CAP_TO_INDEX(cap)].effective |= CAP_TO_MASK(cap);
      }
    }

    /* change capabilities */
    cap_data[0].inheritable = cap_data[1].inheritable = 0;
    if (0 != capset(&cap_hdr, cap_data))
    {
      ds_log_sys_err("capset failed");
      break;
    }
    else
    {
      ds_log_high("capabilities set to [0x%llx]", caps);
    }

    /* display new capabilities */
    if (0 != capget(&cap_hdr, cap_data))
    {
      ds_log_sys_err("capget failed");
    }
    else
    {
      ds_log_high("permitted set = [0x%x] [0x%x]", cap_data[0].permitted, cap_data[1].permitted);
      ds_log_high("effective set = [0x%x] [0x%x]", cap_data[0].effective, cap_data[1].effective);
    }

    ret = 0;
  }while(0);

  /* if we could not execute this function, reset the uid
   * back to previous uid */
  if (-1 == ret)
  {
    ds_log_high("ds_change_user_cap failed");
    if (0 != curr_uid)
    {
      if (0 != setuid(curr_uid))
      {
        ds_log_sys_err("could not reset uid");
      }
      else
      {
        ds_log_high("reset uid back to [%d]",curr_uid);
      }
    }
  }

  return ret;
}

/*===========================================================================
  FUNCTION  ds_daemonize
===========================================================================*/
/*!
@brief
 Performs typical tasks required to run a program as a daemon process.

@return
  0 on Success -1 otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - Original program will exit and a child is forked which will continue
      execution as the daemon.
*/
/*=========================================================================*/
int ds_daemonize (void)
{
  pid_t pid;
  pid_t sid;
  int   ret = -1;

  /* Fork and exit parent process to ensure that process is not a process
  ** group leader.
  */
  if ((pid = fork()) > 0)
  {
    exit(0);
  }

  if (pid < 0)
  {
    /* Could not create child process. Exit */
    ds_log_err("ds_daemonize: Could not create child process\n");
    goto bail;
  }

  /* Become session group leader to disassociate from controlling terminal */
  sid = setsid();

  if (sid < 0)
  {
    ds_log_err("ds_daemonize: setsid() failed\n");
    goto bail;
  }

  /* Set file mode creation mask to 0, to avoid having permissions of created
  ** files being inadvertently changed.
  */
  (void)umask(0);

  /* Change directory to root */
  if ((chdir("/")) < 0)
  {
    ds_log_err("ds_daemonize: chdir to root failed\n");
    goto bail;
  }

  /* Redirect stdin, stdout and stderr to /dev/null. If running as a daemon,
  ** it is assumed that logging will be to syslog.
  */
  if (freopen("/dev/null", "r", stdin) == NULL)
  {
    ds_log_err("ds_daemonize: freopen of stdin failed\n");
    goto bail;
  }

  if (freopen("/dev/null", "w", stdout) == NULL)
  {
    ds_log_err("ds_daemonize: freopen of stdout failed\n");
    goto bail;
  }

  if (freopen("/dev/null", "w", stderr) == NULL)
  {
    ds_log_err("ds_daemonize: freopen of stderr failed\n");
    goto bail;
  }

  ret = 0;

bail:
  return ret;
}

/*===========================================================================
  FUNCTION  ds_get_num_bits_set_count
===========================================================================*/
/*!
@brief
 This function returns the count of bits that are set (1) in the given input
 parameter x

@return
  Count of bits set

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int ds_get_num_bits_set_count
(
  unsigned int x
)
{
  int count = 0;
  unsigned int input = x;

  while (x)
  {
    /* Unset the rightmost set bit */
    x &= (unsigned int)(x - 1) ;
    count++;
  }

  ds_log_low("ds_get_num_bits_set_count: number of bits set in input=%u is %d",
             input, count);

  return count;
}
/*===========================================================================
  FUNCTION  ds_hex_to_dec
===========================================================================*/
/*!
@brief
 Read a char and returns the ASCII value.

@return
  int value

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int ds_hex_to_dec(char ch)
{
  if ((ch >= '0') && (ch <= '9'))
    return (int)(ch - '0');
  ch = (char)tolower(ch);
  if ((ch >= 'a') && (ch <= 'f'))
    return (int)(ch - 'a' + 10);
  return -1;
}

/*===========================================================================
  FUNCTION ds_mac_addr_pton
  ===========================================================================
  @brief
  This function converts the MAC address from a presentation (string) format
  to a network format (Hex value).

  @input:
  char pointer pointing to MAC address in string format ,
  uint8 pointer which will store resultant MAC address in hex

  @return:
  boolean

  @dependencies
  usr to provide input

  @sideefects
  None
  =========================================================================*/
boolean ds_mac_addr_pton(const char *mac_addr_str, uint8 *mac_addr_int)
{
  int i;
  memset(mac_addr_int,0,(size_t)MAC_ADDR_LEN_HEX);
  /* XX:XX:XX:XX:XX:XX */
  if ( strncmp(mac_addr_str,MAC_NULL_STRING,(size_t)MAC_ADDR_NUM_CHARS) == 0 )
     return FALSE;
  if (strlen(mac_addr_str) < (size_t)(MAC_ADDR_NUM_CHARS - 1))
     return FALSE;
  /* Don't dirty result unless string is valid MAC. */
  for (i = 0; i < MAC_ADDR_LEN_HEX; i++)
  {
    if (!strchr(MAC_HEX_STRING, mac_addr_str[i * 3]))
      return FALSE;
    if (!strchr(MAC_HEX_STRING, mac_addr_str[i * 3 + 1]))
      return FALSE;
    if ( ( i != MAC_ADDR_LEN_HEX - 1 ) &&
         (( mac_addr_str[i * 3 + 2] != ':') && (mac_addr_str[i * 3 + 2] != '-' )) )
      return FALSE;
  }
  for (i = 0; i < MAC_ADDR_LEN_HEX; i++)
  {
    mac_addr_int[i] = (uint8)((ds_hex_to_dec(mac_addr_str[i * 3]) << 4) |
                       ds_hex_to_dec(mac_addr_str[i * 3 + 1]));
  }
  return TRUE;
}

/*===========================================================================
  FUNCTION ds_mac_addr_ntop
  ===========================================================================
  @brief
  This function converts the MAC address from a network (hex) format
  to a presentation format (string).

  @input:
  uint8 pointer to MAC address in hex format ,
  char pointer which will store resultant MAC address in string format

  @return:
  void

  @dependencies
  usr to provide input

  @sideefects
  None
  =========================================================================*/

void ds_mac_addr_ntop(const uint8 *mac, char *mac_addr_str)
{
  memset(mac_addr_str,0,(size_t)MAC_ADDR_NUM_CHARS);
  snprintf(mac_addr_str,(size_t)MAC_ADDR_NUM_CHARS,"%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  return;
}

#ifdef FEATURE_DS_UTILS_NON_EMULATOR
/*===========================================================================
  FUNCTION:  ds_get_epid
===========================================================================*/
/*!
  @brief
  This function returns the EPID/EP-TYPE information for a network
  device.

  @params[in] net_dev: network device name
  @params[out] ep_type: End point type.
  @params[out] epid: End point ID obtained through IOCTL.

  @return None (non-zero EPID indicates success)
*/
/*=========================================================================*/
void ds_get_epid
(
  char          *net_dev,
  ds_ep_type_t  *ep_type,
  int           *epid
)
{
  int fd;
  struct ifreq ifr;
  struct rmnet_ioctl_extended_s *ext_ioctl_arg;

  if (NULL == net_dev || NULL == epid || NULL == ep_type)
  {
    ds_log_err("%s", "dsi_get_epid(): Invalid args");
    return;
  }

  *ep_type = DS_EP_TYPE_INVALID;
  *epid = -1;

  ds_log_med("ds_get_epid(): entry: net_dev %s", net_dev);
  if (!strncmp(DS_DEVICE_RMNET_MHI, net_dev, strlen(DS_DEVICE_RMNET_MHI)))
  {
    *ep_type = DS_EP_TYPE_PCIE;
  }
  else if (!strncmp(DS_DEVICE_RMNET_IPA, net_dev, strlen(DS_DEVICE_RMNET_IPA)))
  {
    *ep_type = DS_EP_TYPE_EMBEDDED;
  }
  else if (!strncmp(DS_DEVICE_RMNET, net_dev, strlen(DS_DEVICE_RMNET)))
  {
    *ep_type = DS_EP_TYPE_BAM_DMUX;
  }

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    ds_log_err("%s", "get_epid: socket failed");
    goto bail;
  }

  /* Allocate the extended IOCTL argument */
  ext_ioctl_arg = (void *) malloc (sizeof(struct rmnet_ioctl_extended_s));
  if (NULL == ext_ioctl_arg)
  {
    ds_log_err("%s", "get_epid: malloc for ifru_data failed:");
    close(fd);
    goto bail;
  }

  /* Initialize the ioctl req struct */
  memset(&ifr, 0, sizeof(ifr));
  (void) std_strlcpy(ifr.ifr_name, net_dev, sizeof(ifr.ifr_name));
  ifr.ifr_ifru.ifru_data = ext_ioctl_arg;
  memset(ext_ioctl_arg, 0, sizeof(struct rmnet_ioctl_extended_s));
  ext_ioctl_arg->extended_ioctl = RMNET_IOCTL_GET_EPID;

  /* Get current if flags for the device */
  if (ioctl(fd, RMNET_IOCTL_EXTENDED, &ifr) < 0)
  {
    ds_log_err("%s", "get_epid: ioctl RMNET_IOCTL_GET_EPID failed");
    close(fd);
    free(ext_ioctl_arg);
    goto bail;
  }

  *epid = (int)ext_ioctl_arg->u.data;
  ds_log_med("ds_get_epid(): success: ep_type[%d], epid[0x%x]", *ep_type, *epid);

  /* close temporary socket */
  close(fd);
  free(ext_ioctl_arg);
  return;

bail:
  ds_log_err("%s", "get_epid failed");
  *ep_type = DS_EP_TYPE_INVALID;
  *epid = -1;
  return;
}
#else

void ds_get_epid
(
  char          *net_dev,
  ds_ep_type_t  *ep_type,
  int           *epid
)
{
  ds_log_med("ds_get_epid(): emulator platform, not supported");
  return;
}

#endif


/*===========================================================================
  FUNCTION  ds_get_target_str()
===========================================================================*/
/*!
@brief
 Gets the current target name string.

@return
 Const char pointer to string name.
*/
/*=========================================================================*/
const char *ds_get_target_str
(
  ds_target_t target
)
{
  const char *target_str = ds_target_str_map[0].target_str;
  unsigned int i;

  for (i = 0; i < sizeof(ds_target_str_map)/sizeof(ds_target_str_map[0]); i++)
  {
    if (target == ds_target_str_map[i].target)
    {
      target_str = ds_target_str_map[i].target_str;
      break;
    }
  }

  return target_str;
}

#ifdef FEATURE_DSUTILS_OFFTARGET

/*===========================================================================
  FUNCTION  ds_get_target_offtarget()
===========================================================================*/
/*!
@brief
  Gets the current target name on an off-target platform based on
  env variable DS_TARGET

@details
  Env names are same as ds_target_t enumeration.
  DS_TARGET_MSM
  DS_TARGET_APQ
  DS_TARGET_SVLTE1
  DS_TARGET_SVLTE2
  DS_TARGET_CSFB
  DS_TARGET_SGLTE
  DS_TARGET_SGLTE2
  DS_TARGET_DSDA
  DS_TARGET_DSDA2
  DS_TARGET_DSDA3
  DS_TARGET_MDM
  DS_TARGET_FUSION4_5_PCIE
  DS_TARGET_LE_MDM9X35
  DS_TARGET_LE_MDM9X25
  DS_TARGET_LE_MDM9X15
  DS_TARGET_LE_LEGACY

@return Enum containing target name.
*/
/*=========================================================================*/
ds_target_t ds_get_target_offtarget()
{
  ds_target_t target = DS_TARGET_MSM;
  const char *env_ds_target = getenv(DS_TARGET_ENV);

  if (env_ds_target == NULL)
    target = DS_TARGET_FUSION4_5_PCIE;
  else if (!strcmp(env_ds_target, "DS_TARGET_MSM"))
    target = DS_TARGET_MSM;
  else if (!strcmp(env_ds_target, "DS_TARGET_APQ"))
    target = DS_TARGET_APQ;
  else if (!strcmp(env_ds_target, "DS_TARGET_SVLTE1"))
    target = DS_TARGET_SVLTE1;
  else if (!strcmp(env_ds_target, "DS_TARGET_SVLTE2"))
    target = DS_TARGET_SVLTE2;
  else if (!strcmp(env_ds_target, "DS_TARGET_CSFB"))
    target = DS_TARGET_CSFB;
  else if (!strcmp(env_ds_target, "DS_TARGET_SGLTE"))
    target = DS_TARGET_SGLTE;
  else if (!strcmp(env_ds_target, "DS_TARGET_SGLTE2"))
    target = DS_TARGET_SGLTE2;
  else if (!strcmp(env_ds_target, "DS_TARGET_DSDA"))
    target = DS_TARGET_DSDA;
  else if (!strcmp(env_ds_target, "DS_TARGET_DSDA2"))
    target = DS_TARGET_DSDA2;
  else if (!strcmp(env_ds_target, "DS_TARGET_DSDA3"))
    target = DS_TARGET_DSDA3;
  else if (!strcmp(env_ds_target, "DS_TARGET_MDM"))
    target = DS_TARGET_MDM;
  else if (!strcmp(env_ds_target, "DS_TARGET_FUSION4_5_PCIE"))
    target = DS_TARGET_FUSION4_5_PCIE;
  else if (!strcmp(env_ds_target, "DS_TARGET_LE_MDM9X35"))
    target = DS_TARGET_LE_MDM9X35;
  else if (!strcmp(env_ds_target, "DS_TARGET_LE_MDM9X25"))
    target = DS_TARGET_LE_MDM9X25;
  else if (!strcmp(env_ds_target, "DS_TARGET_LE_MDM9X15"))
    target = DS_TARGET_LE_MDM9X15;
  else if (!strcmp(env_ds_target, "DS_TARGET_LE_LEGACY"))
    target = DS_TARGET_LE_LEGACY;
  else
    target = DS_TARGET_FUSION4_5_PCIE;

  ds_log_med("ds_get_target_offtarget(): env DS_TARGET [%s], Returned target [%d][%s]",
      env_ds_target, target, ds_get_target_str(target));
  return target;
}

/*===========================================================================
  FUNCTION  ds_set_target()
===========================================================================*/
/*!
@brief
  Set DS_TARGET variable programatically.

@arg Enum containing target type. (ds_target_t)
*/
/*=========================================================================*/

void ds_set_target_offtarget(ds_target_t target)
{
  switch (target)
  {
    case DS_TARGET_MSM:
      setenv(DS_TARGET_ENV, "DS_TARGET_MSM", 1);
      return;

    case DS_TARGET_APQ:
      setenv(DS_TARGET_ENV, "DS_TARGET_APQ", 1);
      return;

    case DS_TARGET_SVLTE1:
      setenv(DS_TARGET_ENV, "DS_TARGET_SVLTE1", 1);
      return;

    case DS_TARGET_SVLTE2:
      setenv(DS_TARGET_ENV, "DS_TARGET_SVLTE2", 1);
      return;

    case DS_TARGET_CSFB:
      setenv(DS_TARGET_ENV, "DS_TARGET_CSFB", 1);
      return;

    case DS_TARGET_SGLTE:
      setenv(DS_TARGET_ENV, "DS_TARGET_SGLTE", 1);
      return;

    case DS_TARGET_SGLTE2:
      setenv(DS_TARGET_ENV, "DS_TARGET_SGLTE2", 1);
      return;

    case DS_TARGET_DSDA:
      setenv(DS_TARGET_ENV, "DS_TARGET_DSDA", 1);
      return;

    case DS_TARGET_DSDA2:
      setenv(DS_TARGET_ENV, "DS_TARGET_DSDA2", 1);
      return;

    case DS_TARGET_DSDA3:
      setenv(DS_TARGET_ENV, "DS_TARGET_DSDA3", 1);
      return;

    case DS_TARGET_MDM:
      setenv(DS_TARGET_ENV, "DS_TARGET_MDM", 1);
      return;

    case DS_TARGET_FUSION4_5_PCIE:
      setenv(DS_TARGET_ENV, "DS_TARGET_FUSION4_5_PCIE", 1);
      return;

    case DS_TARGET_LE_MDM9X35:
      setenv(DS_TARGET_ENV, "DS_TARGET_LE_MDM9X35", 1);
      return;

    case DS_TARGET_LE_MDM9X25:
      setenv(DS_TARGET_ENV, "DS_TARGET_LE_MDM9X25", 1);
      return;

    case DS_TARGET_LE_MDM9X15:
      setenv(DS_TARGET_ENV, "DS_TARGET_LE_MDM9X15", 1);
      return;

    case DS_TARGET_LE_LEGACY:
      setenv(DS_TARGET_ENV, "DS_TARGET_LE_LEGACY", 1);
      return;

  } /* switch */

}

pid_t gettid()
{
  return syscall(SYS_gettid);
}
#endif /* FEATURE_DSUTILS_OFFTARGET */

/*===========================================================================
  FUNCTION  ds_get_target_from_soc_id()
===========================================================================*/
/*!
@brief
 Gets the current target name by using soc_id values

@return Enum containing target name.

@details
  Determines the target-type by using this information:
  - Featurization to identify legacy LE targets.
  - For Android and newer LE targets using soc_id values on device
*/
/*=========================================================================*/
static ds_target_t ds_get_target_from_soc_id()
{
  static ds_target_t target = DS_TARGET_INVALID;
  static ds_target_soc_id_t soc_id = DS_TARGET_SOC_ID_INVALID;

  #if defined(FEATURE_DATA_TARGET_MDM9X35)
    target = DS_TARGET_LE_MDM9X35;
  #elif defined(FEATURE_DATA_TARGET_MDM9X25)
    target = DS_TARGET_LE_MDM9X25;
  #elif defined(FEATURE_DATA_TARGET_MDM9X15)
    target = DS_TARGET_LE_MDM9X15;
  #endif

  if(target == DS_TARGET_INVALID)
  {
    do
    {
      FILE *fp = NULL;
      fp = fopen(DS_SOCINFO_SOC_ID_FILE_PATH,"r");
      if(NULL == fp)
      {
        ds_log_err("%s: error opening socinfo file",__func__);
        break;
      }
      if (0 == fscanf(fp,"%d", &soc_id))
      {
        ds_log_err("%s: error reading soc_id",__func__);
        fclose(fp);
        fp = NULL;
        break;
      }
      else
      {
        fclose(fp);
        fp = NULL;
        switch((int)soc_id)
        {
          case DS_TARGET_SOC_ID_MSM8909:
          case DS_TARGET_SOC_ID_MDM9609:
          case DS_TARGET_SOC_ID_MSM8209:
          case DS_TARGET_SOC_ID_MSM8208:
          case DS_TARGET_SOC_ID_MSM8609:
            target = DS_TARGET_JOLOKIA;
            break;
           break;
          case DS_TARGET_SOC_ID_MSM8992:
            target = DS_TARGET_MSM8992;
            break;
          case DS_TARGET_SOC_ID_INVALID:
          default:
            target = DS_TARGET_INVALID;
            break;
        }
      }
    }while(0);
  }
  ds_log_high("%s: soc_id [%d] Target: [%d] : [%s]", __func__, (int)soc_id, (int)target, ds_get_target_str(target));
  return target;
}

/*===========================================================================
  FUNCTION  ds_get_target()
===========================================================================*/
/*!
@brief
 Gets the current target name

@return Enum containing target name.

@details
  Determines the target-type by using this information:
  - Featurization to identify LE vs Android targets.
  - For LE targets
    - Uses further featurization to figure out MDM9x35 vs MDM9x25.
  - For Android targets
    - Uses ro.baseband to identify msm, apq, svlte1, svlte2, csfb, sglte, dsda, dsda2
    - Uses ro.baseband (mdm/mdm2) and ESOC to identify  fusion4, fusion4_5_pcie
*/
/*=========================================================================*/
ds_target_t ds_get_target()
{
  static ds_target_t target = DS_TARGET_INVALID;

#ifdef FEATURE_DS_LINUX_ANDROID
  char baseband[PROPERTY_VALUE_MAX] = {0,};
  char data_target[PROPERTY_VALUE_MAX] = {0,};
  char board_platform[PROPERTY_VALUE_MAX] = {0,};
  struct dev_info devinfo = {0,};
  int esoc_ret = RET_FAILED;
  int i;
#endif

#ifdef FEATURE_DSUTILS_OFFTARGET
  return ds_get_target_offtarget();
#endif

  target = ds_get_target_from_soc_id();

#ifdef FEATURE_DS_LINUX_ANDROID
  if( target == DS_TARGET_INVALID )
  {
    (void) property_get(DS_BASEBAND_PROP, baseband, "");
    (void) property_get(DS_TARGET_PROP, data_target, "");
    /* Call ESOC API only once */
    esoc_ret = get_system_info(&devinfo);

    ds_log_med("ds_get_target(): ro.baseband is set to [%s]", baseband);
    ds_log_med("ds_get_target(): persist.data.target is set to [%s]", data_target);
    ds_log_med("ds_get_target(): ESOC available : [%d]", esoc_ret == RET_SUCCESS ? 1 : 0);
    if (esoc_ret == RET_SUCCESS)
    {
      ds_log_med("ds_get_target(): ESOC info: num_modems [%d]", devinfo.num_modems);
      for (i = 0; i < devinfo.num_modems; i++)
      {
        ds_log_med("ds_get_target(): Modem[%d]: mdm_type: [%s]", i, devinfo.mdm_list[i].type ? "INTERNAL" : "EXTERNAL");
        ds_log_med("ds_get_target(): Modem[%d]: mdm_name: [%s]", i, devinfo.mdm_list[i].mdm_name);
        ds_log_med("ds_get_target(): Modem[%d]: mdm_link: [%s]", i, devinfo.mdm_list[i].mdm_link);
        ds_log_med("ds_get_target(): Modem[%d]: esoc_node: [%s]", i, devinfo.mdm_list[i].esoc_node);
      }
    }

    if (!(std_stricmp(DS_BASEBAND_PROP_MSM, baseband)))
    {
      /* We will use ro.board.platform to distinguish between different
       * MSM targets */
      (void) property_get(DS_BOARD_PLATFORM_PROP, board_platform, "");

      if (!(std_stricmp(DS_BOARD_PLATFORM_MSM8994, board_platform)))
      {
        target = DS_TARGET_MSM8994;
      }
      else if( !(std_stricmp(DS_BOARD_PLATFORM_MSM8916, board_platform)) ||
                 !(std_stricmp(DS_BOARD_PLATFORM_MSM8936, board_platform)) ||
                 !(std_stricmp(DS_BOARD_PLATFORM_MSM8939, board_platform)) ||
                 !(std_stricmp(DS_BOARD_PLATFORM_MSM8929, board_platform)) ||
                 !(std_stricmp(DS_BOARD_PLATFORM_MSM8629, board_platform)) ||
                 !(std_stricmp(DS_BOARD_PLATFORM_MSM8229, board_platform))   )
      {
        target = DS_TARGET_DPM_2_0;
      }
      else if( !(std_stricmp(DS_BOARD_PLATFORM_MSM8909, board_platform)) ||
                 !(std_stricmp(DS_BOARD_PLATFORM_MSM8609, board_platform))   )
      {
        target = DS_TARGET_JOLOKIA;
      }
      else
      {
        target = DS_TARGET_MSM;
      }

      /* Over ride specific for DPM 1.0 devices
         Treat it as MSM targets */
      if(!(std_stricmp(DS_TARGET_PROP_DPM1, data_target)))
      {
        target = DS_TARGET_MSM;
      }
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_APQ, baseband)))
    {
      target = DS_TARGET_APQ;
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_SVLTE1, baseband)))
    {
      target = DS_TARGET_SVLTE1;
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_SVLTE2, baseband)))
    {
      target = DS_TARGET_SVLTE2;
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_CSFB, baseband)))
    {
      target = DS_TARGET_CSFB;
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_SGLTE, baseband)))
    {
      target = DS_TARGET_SGLTE;
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_SGLTE2, baseband)))
    {
      target = DS_TARGET_SGLTE2;
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_DSDA, baseband)))
    {
      target = DS_TARGET_DSDA;
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_DSDA2, baseband)))
    {
      target = DS_TARGET_DSDA2;
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_DSDA3, baseband)))
    {
      target = DS_TARGET_DSDA3;
    }
    else if (!(std_stricmp(DS_BASEBAND_PROP_MDM, baseband)) || !(std_stricmp(DS_BASEBAND_PROP_MDM2, baseband)))
    {
      // If ESOC APIs are present, use them
      if (RET_SUCCESS == esoc_ret && devinfo.num_modems == 1)
      {
        if (!(std_stricmp(devinfo.mdm_list[0].mdm_name, DS_ESOC_MDM_NAME_9X25)))
        {
          target = DS_TARGET_MDM; //Fusion4
        }
        else if (!(std_stricmp(devinfo.mdm_list[0].mdm_name, DS_ESOC_MDM_NAME_9X35)))
        {
          if (!(std_stricmp(devinfo.mdm_list[0].mdm_link, DS_ESOC_MDM_LINK_HSIC)))
          {
            target = DS_TARGET_MDM; //Fusion4.5 HSIC (currently same as MDM)
          }
          else if (!(std_stricmp(devinfo.mdm_list[0].mdm_link, DS_ESOC_MDM_LINK_PCIE)) ||
                   !(std_stricmp(devinfo.mdm_list[0].mdm_link, DS_ESOC_MDM_LINK_HSIC_PCIE)))
          {
            target = DS_TARGET_FUSION4_5_PCIE; //Fusion4.5 PCIE or Hybrid
          }
        }
      }
      // ESOC not present/failed: fallback to persist.data.target
      else if (!std_stricmp(data_target, DS_TARGET_PROP_FUSION4_5_PCIE))
      {
        target = DS_TARGET_FUSION4_5_PCIE;
      }
      else if (!std_stricmp(data_target, DS_TARGET_PROP_FUSION4_5_HSIC))
      {
        target = DS_TARGET_MDM; //Fusion4.5 HSIC (currently same as MDM)
      }
      else
      {
        target = DS_TARGET_MDM;
      }
    }
    else
    {
      target = DS_TARGET_UNDEFINED;
    }
  }
#endif
  ds_log_high("ds_get_target(): Target: [%d] : [%s]", (int)target, ds_get_target_str(target));
  return target;
}
