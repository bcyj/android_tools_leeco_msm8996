#ifndef CNE_MSG_H
#define CNE_MSG_H

/**----------------------------------------------------------------------------
  @file CneMsg.h

  This header file provide the Logging/message macros for the CNE subsytem.
  It will include the platform dependent messaging/logging header file.

-----------------------------------------------------------------------------*/

/*=============================================================================
               Copyright (c) 2009-2014 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary
=============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  when        who   what, where, why
  ----------  ---   -------------------------------------------------------
  2009-07-15  ysk   First revision.
  2011-07-27  jnb   Added CneMsg class
  2012-09-12  mtony Refactored logging macros.
============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <cutils/log.h>
#include <common_log.h>
#include <dlfcn.h>
#include "CneLog.h"

/*
*  Need for QXDM SSID definitions.  Our ADB logging macros references these
*  definitions as well to decide how to map messages to a particular subtype.
*/
#include <msgcfg.h>

// persist property value to enable complete logging on QXDM
#define QXDM_COMPLETE_LOG_PERSIST_PROPERTY_VALUE 3974
// Nims uses additional logging value below for extra
// verbose debugging
// CND should work with this logging value as well
#define QXDM_COMPLETE_LOG_PERSIST_PROPERTY_VALUE2 7825

#define CNE_VERBOSE_LEVEL 0
#define CNE_DEBUG_LEVEL   1
#define CNE_INFO_LEVEL    2
#define CNE_WARN_LEVEL    3
#define CNE_ERROR_LEVEL   4

/*------------------------------------------------------------------------------
 * Public API Macro Functions
 * ---------------------------------------------------------------------------*/

#define SWIM_MSG_VERBOSE( ... )        CneMsg::cne_log_class_ptr->printLog( CNE_VERBOSE_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define SWIM_MSG_DEBUG( ... )          CneMsg::cne_log_class_ptr->printLog( CNE_DEBUG_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define SWIM_MSG_INFO( ... )           CneMsg::cne_log_class_ptr->printLog( CNE_INFO_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define SWIM_MSG_WARN( ... )           CneMsg::cne_log_class_ptr->printReleaseLog( CNE_WARN_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define SWIM_MSG_ERROR( ... )          CneMsg::cne_log_class_ptr->printReleaseLog( CNE_ERROR_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define SWIM_MSG_FATAL( ... )          SWIM_MSG_ERROR( __VA_ARGS__ ); CNE_ASSERT(0)

#define SWIM_MSG_PROLIX

/*
* CNE_MSG (i.e. legacy logging is always enabled)
*/
#define CNE_MSG_VERBOSE( ... )         CneMsg::cne_log_class_ptr->printReleaseLog( CNE_VERBOSE_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_DEBUG( ... )           CneMsg::cne_log_class_ptr->printReleaseLog( CNE_DEBUG_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_INFO( ... )            CneMsg::cne_log_class_ptr->printReleaseLog( CNE_INFO_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_WARN( ... )            CneMsg::cne_log_class_ptr->printReleaseLog( CNE_WARN_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_ERROR( ... )           CneMsg::cne_log_class_ptr->printReleaseLog( CNE_ERROR_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_FATAL( ... )           CNE_MSG_ERROR( __VA_ARGS__ ); CNE_ASSERT(0)

/*
* Release Log Macros
*/
#define CNE_MSG_RELEASE_VERBOSE( ... ) CneMsg::cne_log_class_ptr->printReleaseLog( CNE_VERBOSE_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_RELEASE_DEBUG( ... )   CneMsg::cne_log_class_ptr->printReleaseLog( CNE_DEBUG_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_RELEASE_INFO( ... )    CneMsg::cne_log_class_ptr->printReleaseLog( CNE_INFO_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_RELEASE_WARN( ... )    CneMsg::cne_log_class_ptr->printReleaseLog( CNE_WARN_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_RELEASE_ERROR( ... )   CneMsg::cne_log_class_ptr->printReleaseLog( CNE_ERROR_LEVEL, SUB_TYPE, __VA_ARGS__ )
#define CNE_MSG_RELEASE_FATAL( ... )   CNE_MSG_ERROR( __VA_ARGS__ ); CNE_ASSERT(0)


#define SWIM_MSG_RELEASE_VERBOSE       CNE_MSG_RELEASE_VERBOSE
#define SWIM_MSG_RELEASE_DEBUG         CNE_MSG_RELEASE_DEBUG
#define SWIM_MSG_RELEASE_INFO          CNE_MSG_RELEASE_INFO
#define SWIM_MSG_RELEASE_WARN          CNE_MSG_RELEASE_WARN
#define SWIM_MSG_RELEASE_ERROR         CNE_MSG_RELEASE_ERROR
#define SWIM_MSG_RELEASE_FATAL( ... )  SWIM_MSG_RELEASE_ERROR( __VA_ARGS__ ); CNE_ASSERT(0)

#define CNE_ASSERT(x) assert(x)

/*------------------------------------------------------------------------------
 * Initialization Macros
 * ---------------------------------------------------------------------------*/
#define CNE_LOGGING_INIT(x) CneMsg::init(x)
#define CNE_LOGGING_DEINIT() Diag_LSM_DeInit()

/*------------------------------------------------------------------------------
 * Class Definition
 * ---------------------------------------------------------------------------*/

#ifdef __cplusplus

class CneMsg {

  public:

  #undef SUB_TYPE
  #define SUB_TYPE CNE_MSG_SUBTYPE_QCNEA_GENERIC  // subtype for CneMsg class logging


    /*-----------------------------------------------------------------------------
     * FUNCTION     init
     *
     * DESCRIPTION  initialize logging
     *
     * DEPENDENCIES None
     *
     * RETURN VALUE None
     *
     * SIDE EFFECTS
     *---------------------------------------------------------------------------*/

    static void init(byte * ptr){
      Diag_LSM_Init(ptr);
      cne_log_property_value = CneLog::getPropertyValue();
      CneLog::_libcnelog_handle = dlopen(CNE_DEBUG_BINARY, RTLD_NOW);
      if( CneLog::_libcnelog_handle == NULL ){
        if((cne_log_property_value == QXDM_COMPLETE_LOG_PERSIST_PROPERTY_VALUE) ||
           (cne_log_property_value == QXDM_COMPLETE_LOG_PERSIST_PROPERTY_VALUE2)){
          cne_log_class_ptr = new CneLogDiagAdditional();
         }
        else{
           cne_log_class_ptr = new CneLogDiag();
        }
      }else{
         CneLog::_libcnelog_log_handle = (CneLog::_libcnelog_log_handle_t)
         dlsym( CneLog::_libcnelog_handle, "logaddtlmsg" );
         if( CneLog::_libcnelog_log_handle == NULL ){
           if((cne_log_property_value == QXDM_COMPLETE_LOG_PERSIST_PROPERTY_VALUE) ||
              (cne_log_property_value == QXDM_COMPLETE_LOG_PERSIST_PROPERTY_VALUE2)){
             cne_log_class_ptr = new CneLogDiagAdditional();
           }
           else{
             cne_log_class_ptr = new CneLogDiag();
           }
         }
         else{
           cne_log_class_ptr = new CneLogAdb();
         }
       }
    }

    /*------------------------------------------------------------------------------
     * FUNCTION      abortOnError
     *
     * DESCRIPTION   if flag is equal to -1, log the message, then abort the program
     *
     * DEPENDENCIES  abort
     *
     * RETURN VALUE  this method does not return
     *
     * SIDE EFFECTS  program will be aborted on error
     *----------------------------------------------------------------------------*/
    static void abortOnError(int flag, char const* const msg) {
        if (flag == -1) {
            CNE_MSG_ERROR("Critical Error, Aborting: %s", msg);
            CNE_MSG_ERROR("    %s (Errno %d)", strerror(errno), errno);
            abort();
        }
    }

    /*------------------------------------------------------------------------------
     * FUNCTION      warnOnError
     *
     * DESCRIPTION   if flag is equal to -1, log the message
     *
     * DEPENDENCIES  -
     *
     * RETURN VALUE  true, if flag is equal to -1, otherwise false
     *
     * SIDE EFFECTS  -
     *----------------------------------------------------------------------------*/
    static bool warnOnError(int flag, char const* const msg) {
        if (flag == -1) {
            CNE_MSG_ERROR("Error: %s", msg);
            CNE_MSG_ERROR("    %s (Errno %d)", strerror(errno), errno);
            return true;
        }
        return false;
    }

    static bool logOnError(int flag, char const* const msg) {
      if (flag == -1) {
          CNE_MSG_DEBUG("Error: %s", msg);
          CNE_MSG_DEBUG("    %s (Errno %d)", strerror(errno), errno);
          return true;
      }
      return false;
    }

    static inline bool getDebugLogEnabled() {
        return (CneLog::getPropertyValue() ==
                QXDM_COMPLETE_LOG_PERSIST_PROPERTY_VALUE);
    }

    static CneLog *cne_log_class_ptr;
    static int cne_log_property_value;
};

#endif /* __cplusplus */

#endif /* #ifndef CNE_MSG_H */

