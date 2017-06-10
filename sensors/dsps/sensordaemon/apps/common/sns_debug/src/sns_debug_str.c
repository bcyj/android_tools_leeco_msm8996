/*============================================================================
@file
sns_debug_str.c

@brief
Contains main implementation of receiving and processing
debug strings on Apps processor.

Copyright (c) 2010-2011,2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*=====================================================================
  INCLUDE FILES
  =======================================================================*/
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define LOG_TAG "Sensors"

#include "sns_common.h"
#ifdef SNS_BLAST
#include "sns_debug_str_mdm.h"
#else
#include "sns_debug_str.h"
#endif
#include "sns_string_db.h"
#include "sns_debug_api.h"

#ifndef _WIN32
#  include <utils/Log.h>
#  include <common_log.h>
#  include <sys/system_properties.h>
#else /* ifndef _WIN32 */
#  include "sns_debug_str.tmh"
#endif /* _WIN32 */

#define BUFSZ 1024

#define DAEMON_DEBUG_PROP_NAME "debug.qualcomm.sns.daemon"

#ifndef LOGV_IF
#define LOGV_IF ALOGV_IF
#endif /* LOG_IF */
#ifndef LOGD_IF
#define LOGD_IF ALOGD_IF
#endif /* LOG_IF */
#ifndef LOGI_IF
#define LOGI_IF ALOGI_IF
#endif /* LOG_IF */
#ifndef LOGW_IF
#define LOGW_IF ALOGW_IF
#endif /* LOG_IF */
#ifndef LOGE_IF
#define LOGE_IF ALOGE_IF
#endif /* LOG_IF */

/*===========================================================================
  STRUCTURES
  ============================================================================*/
/*  Structure capturing a string entry */
typedef struct
{
  uint16_t string_id;
  const char *dbg_string;
} string_db_entry_s;

typedef struct
{
  sns_debug_module_id_e module_id;
  const string_db_entry_s* db_ptr;
  uint8_t db_size;
} db_list_s;

/*===========================================================================
  GLOBAL VARIABLES
  ============================================================================*/
/* Define each modules global array */
static const string_db_entry_s diag_debug_str_db[]       = DIAG_STR_DB;
static const string_db_entry_s smgr_debug_str_db[]       = SMGR_STR_DB;
static const string_db_entry_s dd_alsprx_debug_str_db[]  = DD_ALPRX_STR_DB;
static const string_db_entry_s dd_mag8975_debug_str_db[] = DD_MAG8975_STR_DB;
static const string_db_entry_s dd_accel_debug_str_db[]   = DD_ACCEL_STR_DB;
static const string_db_entry_s dd_gyro_debug_str_db[]    = DD_GYRO_STR_DB;
static const string_db_entry_s sam_debug_str_db[]        = SAM_STR_DB;
static const string_db_entry_s smr_debug_str_db[]        = SMR_STR_DB;
static const string_db_entry_s scm_debug_str_db[]        = SCM_STR_DB;

/* Pointer to debug string databases of sensor modules */
static const db_list_s sensor_dbg_db_ptrs[] = {
  // DIAG module
  {SNS_DBG_MOD_APPS_DIAG, diag_debug_str_db,
   (sizeof(diag_debug_str_db)/sizeof(diag_debug_str_db[0]))},
  //SAM DSPS Module
  {SNS_DBG_MOD_DSPS_SAM, sam_debug_str_db,
   (sizeof(sam_debug_str_db)/sizeof(sam_debug_str_db[0]))},
  //SAM Apps Module
  {SNS_DBG_MOD_APPS_SAM, sam_debug_str_db,
   (sizeof(sam_debug_str_db)/sizeof(sam_debug_str_db[0]))},
  //SMGR Module
  {SNS_DBG_MOD_DSPS_SMGR, smgr_debug_str_db,
   (sizeof(smgr_debug_str_db)/sizeof(smgr_debug_str_db[0]))},
  //SMR Apps Module
  {SNS_DBG_MOD_APPS_SMR, smr_debug_str_db,
   (sizeof(smr_debug_str_db)/sizeof(smr_debug_str_db[0]))},
  //SMR DSPS Module
  {SNS_DBG_MOD_DSPS_SMR, smr_debug_str_db,
   (sizeof(smr_debug_str_db)/sizeof(smr_debug_str_db[0]))},
  //ALS Proximity Device Driver Module
  {SNS_DBG_MOD_DSPS_DD_ALSPRX, dd_alsprx_debug_str_db,
   (sizeof(dd_alsprx_debug_str_db)/sizeof(dd_alsprx_debug_str_db[0]))},
  //AKM8975 Magnetometer Device Driver Module
  {SNS_DBG_MOD_DSPS_DD_MAG8975, dd_mag8975_debug_str_db,
   (sizeof(dd_mag8975_debug_str_db)/sizeof(dd_mag8975_debug_str_db[0]))},
  //Accel Device Driver Module
  {SNS_DBG_MOD_DSPS_DD_ACCEL, dd_accel_debug_str_db,
   (sizeof(dd_accel_debug_str_db)/sizeof(dd_accel_debug_str_db[0]))},
  //Gyro Device Driver Module
  {SNS_DBG_MOD_DSPS_DD_GYRO, dd_gyro_debug_str_db,
   (sizeof(dd_gyro_debug_str_db)/sizeof(dd_gyro_debug_str_db[0]))},
  //SCM Module
  {SNS_DBG_MOD_DSPS_SCM, scm_debug_str_db,
    (sizeof(scm_debug_str_db)/sizeof(scm_debug_str_db[0]))}
};

typedef enum {
  LOG_LEVEL_ALL,
  LOG_LEVEL_VERBOSE,
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_DISABLED
} log_level_e;

/* Global variable to hold debug message filtering mask */
#ifdef _WIN32
static uint64_t dbg_str_mask_g=0xFFFFFFFFFFFFFFFF; //Win32: disable all messages
static log_level_e g_log_level = LOG_LEVEL_ALL;
#else
static uint64_t dbg_str_mask_g=0x2; //All bits are set are except DIAG
static log_level_e g_log_level = LOG_LEVEL_WARN;
#endif


/*===========================================================================
  FUNCTIONS
  ============================================================================*/
/*===========================================================================

  FUNCTION:   sns_debug_printf_string

  ===========================================================================*/
/*!
  @brief
  Prints out the debug string.

  @param[i] module_id         : Module id assigned by Sensor Message Router
  @param[i] priority          : Priority of the message string
  @param[i] debug_str         : Debug string
  @param[i] fmt_params_ptr*   : Pointer to format parameter structure

  @return
  No return value.
*/
/*=========================================================================*/
void sns_debug_printf_string(sns_debug_module_id_e module_id,
                             uint8_t priority,
                             const char *debug_str,
                             const debug_params_s *fmt_params_ptr)
{
  char buf[BUFSZ];
  int index = 0;
  int strsz;

  UNREFERENCED_PARAMETER(module_id);

  if ( (fmt_params_ptr == NULL) || (debug_str == NULL) ||
       (priority > SNS_MSG_ERROR) )
  {
    LOGI("Format parameters OR Debug String is NULL or Priority Incorrect");
    return;
  }

  // Implementation using printf, Can be seen using ADB LogCat
  strsz = snprintf(buf+index, BUFSZ-index,
                   "%s(%d):",
                   fmt_params_ptr->filename,fmt_params_ptr->line_num);

  if( strsz > 0 && strsz <= BUFSZ )
  {
    index = strsz;
  }
  else
  {
    LOGI("Error %d in snprintf!", strsz);
  }

  switch (fmt_params_ptr->num_params_valid)
  {
    case 0:
      snprintf(buf+index, BUFSZ-index,
               "%s",debug_str);
      break;
    case 1:
      snprintf(buf+index, BUFSZ-index,
               debug_str,fmt_params_ptr->param1);
      break;
    case 2:
      snprintf(buf+index, BUFSZ-index,
               debug_str,fmt_params_ptr->param1,fmt_params_ptr->param2);
      break;
    case 3:
      snprintf(buf+index, BUFSZ-index,
               debug_str,
               fmt_params_ptr->param1,
               fmt_params_ptr->param2,
               fmt_params_ptr->param3);
      break;
  };
  if( index > 0 )
  {
    buf[BUFSZ-1] = '\0';
    switch(priority)
    {
      case SNS_MSG_LOW:
        LOGV_IF(g_log_level <= LOG_LEVEL_VERBOSE,
                "%s",buf);
        break;

      case SNS_MSG_MEDIUM:
        LOGD_IF(g_log_level <= LOG_LEVEL_DEBUG,
                "%s",buf);
        break;

      case SNS_MSG_HIGH:
        LOGI_IF(g_log_level <= LOG_LEVEL_INFO,
                "%s",buf);
        break;

      case SNS_MSG_FATAL:
        LOGW_IF(g_log_level <= LOG_LEVEL_WARN,
                "%s",buf);
        break;

      case SNS_MSG_ERROR:
      default:
        LOGE_IF(g_log_level <= LOG_LEVEL_ERROR,
                "%s",buf);
        break;
    }; //end of switch
  }
  //Implementation using DIAG done through MACROS

} //end of function sns_debug_printf_string


/*===========================================================================

  FUNCTION:   sns_debug_printf_string_id

  ===========================================================================*/
/*===========================================================================*/
/*!
  @brief
  Prints out the debug string based on the string identifier.

  @param[i] module_id         : Module id assigned by Sensor Message Router
  @param[i] priority          : Priority of the message string
  @param[i] debug_str_id      : Debug string identifier
  @param[i] fmt_params_ptr*   : Pointer to format parameter structure

  @return
  No return value.
*/
/*=========================================================================*/
void sns_debug_printf_string_id(sns_debug_module_id_e module_id,
                                uint8_t priority,
                                uint16_t string_id_param,
                                const debug_params_s *fmt_params_ptr)
{
  char                  buf[BUFSZ];
  int                   index = 0;
  int                   strsz;
  uint16_t              i,j,str_db_size;
  static uint8_t        db_size;
  const string_db_entry_s* db_ptr = NULL;
  const char*           str_ptr;

  /* Input parameters validity check */
  if (fmt_params_ptr == NULL)
  {
    LOGI("sns_debug_printf_string_id: Format parameters is NULL");
    return ;
  }

  /* Get string database pointer from module id */
  if (db_size == 0)
  {
    db_size = sizeof(sensor_dbg_db_ptrs)/sizeof(sensor_dbg_db_ptrs[1]);
  }

  for (i = 0; i < db_size; i++)
  {
    if ( sensor_dbg_db_ptrs[i].module_id == module_id )
    {
      db_ptr = sensor_dbg_db_ptrs[i].db_ptr;
      break;
    }
  }
  /* IMPORTANT : Do NOT ReUSE i in the code after this point */
  if ( (db_ptr == NULL) ||           // NULL check for pointer
       (string_id_param > sensor_dbg_db_ptrs[i].db_size )
       // Check if string id is greater maximum string id in the db
       )
  {
    LOGI("db ptr NULL OR string ID not valid (%d)",string_id_param);
    MSG_SPRINTF_3(MSG_SSID_SNS,
                  MSG_LEGACY_ERROR,
                  "db ptr NULL OR string ID not valid (%d: %d)",
                  module_id,
                  string_id_param,
                  0);
    return;
  }

  str_ptr = db_ptr[string_id_param].dbg_string;
  /* Does the string id match the right location in the string database */
  if (string_id_param != db_ptr[string_id_param].string_id)
  {
    /*The string does not match, search the database for the right string entry*/
    str_db_size = sensor_dbg_db_ptrs[i].db_size;
    for(j = 0; j <= str_db_size; j++)
    {
      if ( db_ptr[j].string_id == string_id_param )
      {
        str_ptr = db_ptr[j].dbg_string;
        break;
      }
    } // end of for

    /* String was not found print message and return */
    if (j > str_db_size)
    {
      LOGI("%s(%d): String not found in DB = %d. FIX it",
           fmt_params_ptr->filename,
           fmt_params_ptr->line_num,
           string_id_param);

      MSG_SPRINTF_3(MSG_SSID_SNS,
                    MSG_LEGACY_ERROR,
                    "%s(%d): String not found in DB = %d. FIX it",
                    fmt_params_ptr->filename,
                    fmt_params_ptr->line_num,
                    string_id_param);
      return;
    } //end of nested if

  } // end of if

  /* Create buf output for ADB logcat */
  strsz = snprintf(buf+index, BUFSZ-index,
                   "%s(%d):",
                   fmt_params_ptr->filename,
                   fmt_params_ptr->line_num);

  if( strsz > 0 && strsz <= BUFSZ )
  {
    index = strsz;
  }
  else
  {
    LOGI("Error %d in snprintf!", strsz);
  }

  if (fmt_params_ptr->num_params_valid == 0)
  {
    snprintf(buf+index, BUFSZ-index,
             "%s",str_ptr);
  }
  else if (fmt_params_ptr->num_params_valid == 1)
  {
    snprintf(buf+index, BUFSZ-index,
             str_ptr,fmt_params_ptr->param1);
  }
  else if (fmt_params_ptr->num_params_valid == 2)
  {
    snprintf(buf+index, BUFSZ-index,
                     str_ptr,fmt_params_ptr->param1,fmt_params_ptr->param2);
  }
  else if (fmt_params_ptr->num_params_valid == 3)
  {
    snprintf(buf+index, BUFSZ-index,
             str_ptr,
             fmt_params_ptr->param1,
             fmt_params_ptr->param2,
             fmt_params_ptr->param3);
  }
  if( index > 0 )
  {
    buf[BUFSZ-1] = '\0';
  }
  else
  {
    buf[0] = '\0';
  }

  /* Output the string to QXDM */
  switch(priority)
  {
    case SNS_MSG_LOW:
      /* Output the string to QXDM */
      MSG_SPRINTF_2(MSG_SSID_SNS,
                    MSG_LEGACY_LOW,
                    "%s(%d):",
                    fmt_params_ptr->filename,
                    fmt_params_ptr->line_num);

      MSG_SPRINTF_FMT_VAR_3(MSG_SSID_SNS,
                            MSG_LEGACY_LOW,
                            (const char*)str_ptr,
                            fmt_params_ptr->param1,
                            fmt_params_ptr->param2,
                            fmt_params_ptr->param3);
      LOGV_IF(g_log_level <= LOG_LEVEL_VERBOSE,
              "%s",buf);
      break;

    case SNS_MSG_MEDIUM:
      /* Output the string to QXDM */
      MSG_SPRINTF_2(MSG_SSID_SNS,
                    MSG_LEGACY_MED,
                    "%s(%d):",
                    fmt_params_ptr->filename,
                    fmt_params_ptr->line_num);

      MSG_SPRINTF_FMT_VAR_3(MSG_SSID_SNS,
                            MSG_LEGACY_MED,
                            (const char*)str_ptr,
                            fmt_params_ptr->param1,
                            fmt_params_ptr->param2,
                            fmt_params_ptr->param3);
      LOGD_IF(g_log_level <= LOG_LEVEL_DEBUG,
              "%s",buf);
      break;


    case SNS_MSG_HIGH:
      /* Output the string to QXDM */
      MSG_SPRINTF_2(MSG_SSID_SNS,
                    MSG_LEGACY_HIGH,
                    "%s(%d):",
                    fmt_params_ptr->filename,
                    fmt_params_ptr->line_num);

      MSG_SPRINTF_FMT_VAR_3(MSG_SSID_SNS,
                            MSG_LEGACY_HIGH,
                            (const char*)str_ptr,
                            fmt_params_ptr->param1,
                            fmt_params_ptr->param2,
                            fmt_params_ptr->param3);
      LOGI_IF(g_log_level <= LOG_LEVEL_INFO,
              "%s",buf);
      break;


    case SNS_MSG_FATAL:
      /* Output the string to QXDM */
      MSG_SPRINTF_2(MSG_SSID_SNS,
                    MSG_LEGACY_FATAL,
                    "%s(%d):",
                    fmt_params_ptr->filename,
                    fmt_params_ptr->line_num);

      MSG_SPRINTF_FMT_VAR_3(MSG_SSID_SNS,
                            MSG_LEGACY_FATAL,
                            (const char*)str_ptr,
                            fmt_params_ptr->param1,
                            fmt_params_ptr->param2,
                            fmt_params_ptr->param3);
      LOGW_IF(g_log_level <= LOG_LEVEL_WARN,
              "%s",buf);
      break;

    case SNS_MSG_ERROR:
      /* Output the string to QXDM */
      MSG_SPRINTF_2(MSG_SSID_SNS,
                    MSG_LEGACY_ERROR,
                    "%s(%d):",
                    fmt_params_ptr->filename,
                    fmt_params_ptr->line_num);

      MSG_SPRINTF_FMT_VAR_3(MSG_SSID_SNS,
                            MSG_LEGACY_ERROR,
                            (const char*)str_ptr,
                            fmt_params_ptr->param1,
                            fmt_params_ptr->param2,
                            fmt_params_ptr->param3);
      LOGE_IF(g_log_level <= LOG_LEVEL_ERROR,
              "%s",buf);
      break;

    default:
      break;
  }; //end of switch
}


/*===========================================================================

  FUNCTION:   sns_debug_set_dbg_str_mask

===========================================================================*/
/*===========================================================================*/
/*!
  @brief
  Sets the debug string mask. Essentially a copy of the mask bits to a global
  variable visible to only this file is made.

  @param[i] bit_mask         : Debug string bit mask

  @return
  No return value.
*/
/*=========================================================================*/
void sns_debug_set_dbg_str_mask(uint64_t bit_mask)
{
  char                debug_prop[PROP_VALUE_MAX];
  int                 debug_prop_len;

  /* Get the current Android log level from the android property */

  debug_prop_len = __system_property_get( DAEMON_DEBUG_PROP_NAME, debug_prop );
  if( debug_prop_len == 1 ) {
    switch( debug_prop[0] ) {
      case '0':
        g_log_level = LOG_LEVEL_DISABLED;
        break;
      case '1':
        g_log_level = LOG_LEVEL_ALL;
        break;
      case 'v':
      case 'V':
        g_log_level = LOG_LEVEL_VERBOSE;
        break;
      case 'd':
      case 'D':
        g_log_level = LOG_LEVEL_DEBUG;
        break;
      case 'i':
      case 'I':
        g_log_level = LOG_LEVEL_INFO;
        break;
      case 'w':
      case 'W':
        g_log_level = LOG_LEVEL_WARN;
        break;
      case 'e':
      case 'E':
        g_log_level = LOG_LEVEL_ERROR;
        break;
      default:
        break;
    }
    LOGE("%s: Setting log level to %d", __FUNCTION__, g_log_level);
  } else if( debug_prop_len > 1 ) {
    LOGE("%s: invalid value for %s: %s. Enabling all logs", __FUNCTION__,
         DAEMON_DEBUG_PROP_NAME, debug_prop );
    g_log_level = LOG_LEVEL_ALL;
  }

  dbg_str_mask_g = bit_mask;

} //end of sns_debug_set_dbg_str_mask

/*===========================================================================

  FUNCTION:   sns_debug_is_module_disabled

===========================================================================*/
/*===========================================================================*/
/*!
  @brief
  Returns 1 if the the module's debug strings need to be filtered.

  @param[i] module_id        : Debug module id

  @return
  uint8_t : 1 - Module's messages should be filtered
            0 - Module's messages should NOT be filtered
*/
/*=========================================================================*/
uint8_t sns_debug_is_module_disabled(sns_debug_module_id_e module_id)
{
  uint64_t   temp_i=1;
  uint8_t    ret_value=0;

  /* Check if the messages from the module are enabled
   * based on the debug string filter mask
   */
  if ((dbg_str_mask_g & (temp_i << module_id)) != 0)
  {
    ret_value = 1;
  }
  return ret_value;
} //end of sns_debug_set_dbg_str_mask
