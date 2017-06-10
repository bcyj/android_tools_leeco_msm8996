/******************************************************************************
  @file    ds_profile_os_linux.h
  @brief   Operating System specific header

  DESCRIPTION
  This header defines API for OS (LINUX) specific logging, locking mechanisms. 

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/data/1x/707/main/latest/src/ds707_data_session_profile.c#32 $ $DateTime: 2009/09/11 10:21:08 $ $Author: lipings $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#ifndef DS_PROFILE_OS_LINUX_H
#define DS_PROFILE_OS_LINUX_H

#include "comdef.h"
#include "rex.h"
#include "pthread.h"
#include "string.h"
#include "msg.h"
#include <msgcfg.h>
#include <diag_lsm.h>
#include <log.h>

#ifdef TEST_FRAMEWORK
  #include <qmutex.h>
  typedef qmutex_t            plm_lock_type;
#else
  //typedef rex_crit_sect_type  plm_lock_type;
  typedef pthread_mutex_t       plm_lock_type;
#endif /*TEST_FRAMEWORK*/

int ds_profile_log_init(  char *lib );
int ds_profile_lock_init( plm_lock_type *lock );

#ifdef TEST_FRAMEWORK
int ds_profile_lock_acq( plm_lock_type lock );
int ds_profile_lock_rel( plm_lock_type lock );
#else
int ds_profile_lock_acq( plm_lock_type *lock );
int ds_profile_lock_rel( plm_lock_type *lock );
#endif /* TEST_FRAMEWORK */

//#define DS_PROFILE_LOGD(format, arg1) MSG_MED( format, arg1, 0, 0 )
//#define DS_PROFILE_LOGE(format, arg1) MSG_ERROR( format, arg1, 0, 0 ) 

/*=========================================================================
  FUNCTION:  ds_profile_format_log_msg

===========================================================================*/
/*!
    @brief          
    Format debug message for logging.

    @return
    None
*/
/*=========================================================================*/
void ds_profile_format_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
);


#define DS_PROFILE_MAX_DIAG_LOG_MSG_SIZE   256

/* Log message to Diag */
#define DS_PROFILE_LOG_MSG_DIAG( lvl, ... )                                      \
  {                                                                              \
    char buf[ DS_PROFILE_MAX_DIAG_LOG_MSG_SIZE ];                                       \
                                                                                 \
    /* Format message for logging */                                             \
    ds_profile_format_log_msg( buf, DS_PROFILE_MAX_DIAG_LOG_MSG_SIZE, __VA_ARGS__ );           \
                                                                                 \
    /* Log message to Diag */                                                    \
    MSG_SPRINTF_1( MSG_SSID_DIAG, lvl, "%s", buf );                              \
  }



#define DS_PROFILE_LOGD(...) DS_PROFILE_LOG_MSG_DIAG( MSG_LEGACY_HIGH, __VA_ARGS__)
#define DS_PROFILE_LOGE(...) DS_PROFILE_LOG_MSG_DIAG( MSG_LEGACY_ERROR, __VA_ARGS__)

#define DS_PROFILE_MEM_ALLOC( size, client ) malloc( size)
#define DS_PROFILE_MEM_FREE( ptr, client ) free( ptr)

#define DS_PROFILE_STR_LEN( str ) strlen( str )
#define DS_PROFILE_STR_CPY( str1, str2, size ) strlcpy( str1, str2, size )
#define DS_PROFILE_STR_CMP(str1, str2, len ) strncmp( str1, str2, len )
//#define DS_PROFILE_STR_PRINTF( str, len, format, ... ) std_strlprintf( str, len, format, ...)

#endif /* DS_PROFILE_OS_LINUX_H */
