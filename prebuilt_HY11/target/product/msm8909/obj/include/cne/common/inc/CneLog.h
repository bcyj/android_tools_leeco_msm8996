#ifndef CNE_LOG_H
#define CNE_LOG_H

/**----------------------------------------------------------------------------
  @file CneLog.h

  This header file provides the Logging classes for the CNE subsytem.

-----------------------------------------------------------------------------*/

/*=============================================================================
               Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary
=============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  hen         who      what, where, why
  ----------  -------  -------------------------------------------------------
  2013-05-06  npoddar   First revision.
============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <cutils/log.h>
#include <common_log.h>
#include <string>

/*
*  Need for QXDM SSID definitions.  Our ADB logging macros references these
*  definitions as well to decide how to map messages to a particular subtype.
*/
#include <msgcfg.h>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

#define MAIN_LOG_TAG  "QCNEA"
/*
 *  The default sub-type for classes which don't define a sub-type.
 *  "UNDEFINED SUBTYPE" will be printed in place of where the sub-type
 *  would go in the log message to let the developer know that a sub-type
 *  should be defined.
 */
#undef SUB_TYPE
#define SUB_TYPE CNE_MSG_SUBTYPE_NO_TAG_DEFINED

// always include diag headers because some macros
// always print to logcat and qxdm simultaneously
#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */
#include <msg.h>

// diag_lsm.h doesn't undefine LOG_TAG before redefining it.
// undefine LOG_TAG to prevent from getting warning messages when compiling.
#undef LOG_TAG
#include <diag_lsm.h>

#ifdef __cplusplus
  }
#endif /* __cplusplus */

// adb limits log entry sizes to 1024 bytes
// as defined in system/core/liblog/logd_write.c
// diag limits the maximum size of log entries to 512 bytes.
// Thus, we will use the lowest maximum value allowed.
#define CNE_MSG_MAX_LOG_MSG_SIZE 512

/*----------------------------------------------------------------------------
 * Subtype Tree
 * Note: Each module should be classified under one of the following sub-types.
 *       This sub-type tree will be mirrored in QXDM and adb loggin.
 *
 * QCNEA
 * |-- CAC
 * |-- CORE
 * |   |-- CAS
 * |   |-- CDE
 * |   |-- COM
 * |   |-- LEE
 * |   |-- QMI
 * |   `-- SRM
 * |-- GENERIC
 * |-- NETLINK
 * |-- NIMS
 * |-- NSRM
 * |   |-- CORE
 * |   |-- GATESM
 * |   `-- TRG
 * |-- PLCY
 * |   `-- ANDSF
 * |-- TEST
 * `-- WQE
 *   |-- BQE
 *   |-- CQE
 *   |-- ICD
 *   |-- IFSEL
 *   `-- IFSELRSM
 *
 * -------------------------------------------------------------------------*/

// TEMPORARILY hard code DBQE MSG ID until it is defined in diag
#define MSG_SSID_QCNEA_WQE_DBQE              10378

#define CNE_MSG_SUBTYPE_QCNEA               MSG_SSID_QCNEA

#define CNE_MSG_SUBTYPE_QCNEA_CAC           MSG_SSID_QCNEA_CAC           // Cne API Client

#define CNE_MSG_SUBTYPE_QCNEA_CORE          MSG_SSID_QCNEA_CORE          // Cne Core modules
#define CNE_MSG_SUBTYPE_QCNEA_CORE_CAS      MSG_SSID_QCNEA_CORE_CAS      // Cne API Service
#define CNE_MSG_SUBTYPE_QCNEA_CORE_CDE      MSG_SSID_QCNEA_CORE_CDE      // Contextual Data Estimator
#define CNE_MSG_SUBTYPE_QCNEA_CORE_COM      MSG_SSID_QCNEA_CORE_COM      // Communications
#define CNE_MSG_SUBTYPE_QCNEA_CORE_LEE      MSG_SSID_QCNEA_CORE_LEE      // Latency Estimator
#define CNE_MSG_SUBTYPE_QCNEA_CORE_QMI      MSG_SSID_QCNEA_CORE_QMI      // Qualcomm Modem Interface
#define CNE_MSG_SUBTYPE_QCNEA_CORE_SRM      MSG_SSID_QCNEA_CORE_SRM      // Resource Manager

#define CNE_MSG_SUBTYPE_QCNEA_GENERIC       MSG_SSID_QCNEA_GENERIC       // Ignored and unclassified files

#define CNE_MSG_SUBTYPE_QCNEA_NETLINK       MSG_SSID_QCNEA_NETLINK       // Netlink

#define CNE_MSG_SUBTYPE_QCNEA_NIMS          MSG_SSID_QCNEA_NIMS          // Network Interface Management System

#define CNE_MSG_SUBTYPE_QCNEA_NSRM          MSG_SSID_QCNEA_NSRM          // Network Socket Request Manager
#define CNE_MSG_SUBTYPE_QCNEA_NSRM_CORE     MSG_SSID_QCNEA_NSRM_CORE     // Network Socket Request Manager
#define CNE_MSG_SUBTYPE_QCNEA_NSRM_GATESM   MSG_SSID_QCNEA_NSRM_GATESM   // NSRM Gate State Machine
#define CNE_MSG_SUBTYPE_QCNEA_NSRM_TRG      MSG_SSID_QCNEA_NSRM_TRG      // NSRM Triggers

#define CNE_MSG_SUBTYPE_QCNEA_PLCY          MSG_SSID_QCNEA_PLCY          // General Policy Parsers
#define CNE_MSG_SUBTYPE_QCNEA_PLCY_ANDSF    MSG_SSID_QCNEA_PLCY_ANDSF    // ANDSF Policy Parser

#define CNE_MSG_SUBTYPE_QCNEA_TEST          MSG_SSID_QCNEA_TEST          // Unit Test Logging

#define CNE_MSG_SUBTYPE_QCNEA_WQE           MSG_SSID_QCNEA_WQE           // Wifi Quality Estimator
#define CNE_MSG_SUBTYPE_QCNEA_WQE_BQE       MSG_SSID_QCNEA_WQE_BQE       // Bitrate Quality Estimator
#define CNE_MSG_SUBTYPE_QCNEA_WQE_DBQE      MSG_SSID_QCNEA_WQE_DBQE       // Transpost Quality Estimator
#define CNE_MSG_SUBTYPE_QCNEA_WQE_CQE       MSG_SSID_QCNEA_WQE_CQE       // Channel Quality Estimator
#define CNE_MSG_SUBTYPE_QCNEA_WQE_ICD       MSG_SSID_QCNEA_WQE_ICD       // Internet Connectivity Detector
#define CNE_MSG_SUBTYPE_QCNEA_WQE_IFSEL     MSG_SSID_QCNEA_WQE_IFSEL     // Interface Selection
#define CNE_MSG_SUBTYPE_QCNEA_WQE_IFSELRSM  MSG_SSID_QCNEA_WQE_IFSELRSM  // Interface Selector State Machine
#define CNE_MSG_SUBTYPE_QCNEA_ATP           MSG_SSID_QCNEA_ATP           // ATP
#define CNE_MSG_SUBTYPE_QCNEA_ATP_PLCY      MSG_SSID_QCNEA_ATP_PLCY      // ATP policy management.
#define CNE_MSG_SUBTYPE_QCNEA_ATP_RPRT      MSG_SSID_QCNEA_ATP_RPRT      // ATP report management


#define CNE_MSG_SUBTYPE_NO_TAG_DEFINED  0                                // default sub-type (should not be used)

// Define a flag to set runtime debug logging
#define CNE_LOG_QXDM_PERSIST_PROPERTY "persist.cne.logging.qxdm"

// Path where the logging library should be located
#define CNE_DEBUG_BINARY "libcnelog.so"

/*------------------------------------------------------------------------------
 * Class Definition
 * ---------------------------------------------------------------------------*/

class CneLog
{
  public:
    /*----------------------------------------------------------------------------
     * Public Types
     *--------------------------------------------------------------------------*/
     static int QXDM_VERBOSITY_LEVEL[5];
     static int ADB_VERBOSITY_LEVEL[5];

    /*----------------------------------------------------------------------------
     * Logging library handle definitions
     *--------------------------------------------------------------------------*/
    typedef void ( *_libcnelog_log_handle_t )
     ( int, const char*, const char*, char * );

    static void *_libcnelog_handle;
    static _libcnelog_log_handle_t _libcnelog_log_handle;

    /*----------------------------------------------------------------------------
     * Public Method Specifications
     * -------------------------------------------------------------------------*/

    /*----------------------------------------------------------------------------
     * FUNCTION      CneLog Constructor
     *
     * DESCRIPTION   None
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    CneLog() {
    }

    /*----------------------------------------------------------------------------
     * FUNCTION      CneLog Destructor
     *
     * DESCRIPTION   None
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    virtual ~CneLog(){}

    /*----------------------------------------------------------------------------
     * FUNCTION      printLog
     *
     * DESCRIPTION   Print the non-release logs
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    virtual void printLog( int level, int ssid, const char *fmt, ... );

    /*----------------------------------------------------------------------------
     * FUNCTION      logVerbose
     *
     * DESCRIPTION   Print the release logs
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    virtual void printReleaseLog( int level, int ssid, const char *fmt, ... );

    /*----------------------------------------------------------------------------
     * FUNCTION      getPropertyValue
     *
     * DESCRIPTION   Get the persist property value for runtime logging
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  persist.cne.logging property value
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    static int getPropertyValue();

  protected:
    /*----------------------------------------------------------------------------
     * FUNCTION      printToQXDM
     *
     * DESCRIPTION   Helper function that prints the log messages to QXDM
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void printToQXDM( int level, int ssid, const char *fmt, va_list ap );

    /*----------------------------------------------------------------------------
     * FUNCTION      printToAdb
     *
     * DESCRIPTION   Helper function that prints the log messages to ADB
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void printToAdb( int level, int ssid, const char *fmt, va_list ap );

    /*----------------------------------------------------------------------------
     * FUNCTION      subtypeToStr
     *
     * DESCRIPTION   Returns the appropriate tag for the log message based on ssid
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  SubType for the log message
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    const char * subtypeToStr( const int ssid ){
      switch( ssid ){
        case CNE_MSG_SUBTYPE_QCNEA:                return NULL;
        case CNE_MSG_SUBTYPE_QCNEA_CAC:            return "CAC";
        case CNE_MSG_SUBTYPE_QCNEA_CORE:           return "CORE";
        case CNE_MSG_SUBTYPE_QCNEA_CORE_CAS:       return "CORE:CAS";
        case CNE_MSG_SUBTYPE_QCNEA_CORE_CDE:       return "CORE:CDE";
        case CNE_MSG_SUBTYPE_QCNEA_CORE_COM:       return "CORE:COM";
        case CNE_MSG_SUBTYPE_QCNEA_CORE_LEE:       return "CORE:LEE";
        case CNE_MSG_SUBTYPE_QCNEA_CORE_QMI:       return "CORE:QMI";
        case CNE_MSG_SUBTYPE_QCNEA_CORE_SRM:       return "CORE:SRM";
        case CNE_MSG_SUBTYPE_QCNEA_GENERIC:        return "GENERIC";
        case CNE_MSG_SUBTYPE_QCNEA_NETLINK:        return "NETLINK";
        case CNE_MSG_SUBTYPE_QCNEA_NIMS:           return "NIMS";
        case CNE_MSG_SUBTYPE_QCNEA_NSRM:           return "NSRM";
        case CNE_MSG_SUBTYPE_QCNEA_NSRM_CORE:      return "NSRM:CORE";
        case CNE_MSG_SUBTYPE_QCNEA_NSRM_GATESM:    return "NSRM:GATESM";
        case CNE_MSG_SUBTYPE_QCNEA_NSRM_TRG:       return "NSRM:TRG";
        case CNE_MSG_SUBTYPE_QCNEA_PLCY:           return "PLCY";
        case CNE_MSG_SUBTYPE_QCNEA_PLCY_ANDSF:     return "PLCY:ANDSF";
        case CNE_MSG_SUBTYPE_QCNEA_TEST:           return "TEST";
        case CNE_MSG_SUBTYPE_QCNEA_WQE:            return "WQE";
        case CNE_MSG_SUBTYPE_QCNEA_WQE_BQE:        return "WQE:BQE";
        case CNE_MSG_SUBTYPE_QCNEA_WQE_DBQE:       return "WQE:DBQE";
        case CNE_MSG_SUBTYPE_QCNEA_WQE_CQE:        return "WQE:CQE";
        case CNE_MSG_SUBTYPE_QCNEA_WQE_ICD:        return "WQE:ICD";
        case CNE_MSG_SUBTYPE_QCNEA_WQE_IFSEL:      return "WQE:IFSEL";
        case CNE_MSG_SUBTYPE_QCNEA_WQE_IFSELRSM:   return "WQE:IFSELRSM";
        case CNE_MSG_SUBTYPE_QCNEA_ATP:            return "ATP";
        case CNE_MSG_SUBTYPE_QCNEA_ATP_PLCY:       return "ATP:PLCY";
        case CNE_MSG_SUBTYPE_QCNEA_ATP_RPRT:       return "ATP:RPRT";
        default:                                   return "UNDEFINED";
      }
    }
};

class CneLogDiag : public CneLog
{
  public:
    CneLogDiag(){
    }
};

class CneLogDiagAdditional : public CneLog
{
  public:
    CneLogDiagAdditional(){
    }

    /*----------------------------------------------------------------------------
     * FUNCTION      printLog
     *
     * DESCRIPTION   Print the logs to QXDM
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void printLog( int level, int ssid, const char *fmt, ... );
};

class CneLogAdb : public CneLog
{
  public:
    CneLogAdb(){
    }

    /*----------------------------------------------------------------------------
     * FUNCTION      printLog
     *
     * DESCRIPTION   Print the logs to ADB
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void printLog( int level, int ssid, const char *fmt, ... );

    /*----------------------------------------------------------------------------
     * FUNCTION      printReleaseLog
     *
     * DESCRIPTION   Print the release logs to ADB
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void printReleaseLog( int level, int ssid, const char *fmt, ... );
};
#endif /* CNE_LOG_H */
