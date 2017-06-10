#ifndef _OI_DEBUG_H
#define _OI_DEBUG_H
/** @file
 *
 * This file provides a set of debug print functions and macros.
 *
 * The functions are all implemented, regardless of whether or not the preprocessor
 * symbol OI_DEBUG is defined.  The specific implementation (in oi_debug.c) may differ
 * depending on OI_DEBUG.
 *
 * There are currently 4 debug print macros:
 *  - OI_SLOG_ERROR
 *  - OI_DBG_PRINT1
 *  - OI_DBG_PRINT2
 *  - OI_TRACE_USER
 *
 * Whether or not these debug print macros generate code depends on a combination of
 * preprocessor symbol definitions:
 *  - OI_DEBUG - if defined, all debugprint macros generate code (default for debug mode)
 *  - OI_ENABLE_DBG_PRINT1      - OI_DBG_PRINT1 generates code
 *  - OI_ENABLE_DBG_PRINT2      - OI_DBG_PRINT2 generates code
 *  - OI_ENABLE_DBG_PRINT       - OI_DBG_PRINT1 and OI_DBG_PRINT2 generates code
 *  - OI_ENABLE_TRACE           - OI_TRACE_USER generates code
 *  - OI_ENABLE_SLOG_ERROR      - OI_SLOG_ERROR generates code (default for release mode)
 *  - OI_SUPPRESS_SLOG_ERROR    - Suppresses error logging for further code size reduction
 *
 * For debug builds (OI_DEBUG defined), all debug print macros generate calls to generate
 * debug output.  A run-time filter is applied to selectively display output depending on
 * module name and runtime debug level configuration for that module.
 *
 * For release builds (OI_DEBUG not defined), debug print macros can be selectively enabled
 * at compile time defining the appropriate OI_ENABLE_xxxx.  This selective enabling may
 * be applied to any or all files.
 *
 */
/**********************************************************************************
  $AccuRev-Revision: 344/7 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/

#include "oi_stddefs.h"
//#include "oi_bt_spec.h"
//sean
//#include "oi_statustext.h"
//#include "oi_text.h"

//#include "oi_debugcontrol.h"
#include "oi_modules.h"

//#include "oi_osinterface.h"
//#include "oi_keystore.h"
//#include "oi_l2cap.h"
//#include "oi_ampmgr.h"

/**
 * Defines for message type codes.
 */

#define OI_MSG_CODE_APPLICATION               0   /**< Application output */
#define OI_MSG_CODE_ERROR                     1   /**< Error message output */
#define OI_MSG_CODE_WARNING                   2   /**< Warning message output */
#define OI_MSG_CODE_TRACE                     3   /**< User API function trace output */
#define OI_MSG_CODE_PRINT1                    4   /**< Catagory 1 debug print output */
#define OI_MSG_CODE_PRINT2                    5   /**< Catagory 2 debug print output */
#define OI_MSG_CODE_HEADER                    6   /**< Error/Debug output header */

#ifdef __SNS_MODULE__
#define OI_CURRENT_MODULE __SNS_MODULE__
#else
#define OI_CURRENT_MODULE OI_MODULE_UNKNOWN
#endif

/** \addtogroup Debugging */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Utility functions.
 */


/**
 * Looks up the manufacture name given a manifacturer id.
 *
 * @param manufacturerId    The manufacturer id from the Bluetooth device address
 *
 * @return  The name of the manufacturer or NULL if there is no match for the id.
 */
OI_CHAR *OI_BtMfgrIdToText(OI_UINT16 manufacturerId);


/**
 * Looks up the character string for one of the stack modules.
 *
 * @param module  The module to lookup
 *
 * @return  The string name for the module or NULL if the module id is invalid.
 */
const OI_CHAR *OI_ModuleToString(OI_MODULE module);


/**
 * @name Debug Output Macros
 * There are 7 catagories of output defined.  The catagories and
 * functions/macros that implement those catagories are defined below:
 *
 * OI_Printf(fmt, ...)                  - Always output.  Intended for use
 *                                        primarily by the application.
 *
 * OI_SLOG_ERROR(status, (fmt, ...))    - Always output (except when build with
 *                                        OI_SUPPRESS_SLOG_ERROR).  Intended
 *                                        primarily for severe errors, errors
 *                                        that cannot be handled, or indications
 *                                        of bugs found at run time.
 *
 * OI_SLOG_WARNING(status, (fmt, ...))  - Normally output, can be suppressed
 *                                        with appropriate control code.
 *                                        Intended for all error conditions not
 *                                        covered by OI_SLOG_ERROR, particularly
 *                                        non-critical errors, for example, when
 *                                        a remote device send corrupted data.
 *
 * OI_TRACE_USER(freq, (fmt, ...))      - Only output when enabled with control
 *                                        code U.  Intended for entry to user
 *                                        API's and prior to calling user
 *                                        callbacks.
 *
 * OI_DBG_PRINT1(freq, (fmt, ...))      - Only output when enabled with control
 *                                        code D.  Intended for tracing
 *                                        high-level data flow, state changes,
 *                                        and unusual code paths.
 *
 * OI_DBG_PRINT2(freq, (fmt, ...))      - Only output when enabled with control
 *                                        code E.  Intended for more verbose
 *                                        information and for use when an
 *                                        alternate color would make
 *                                        understanding the debug output easier.
 *
 * Note that the macros OI_SLOG_*(), OI_TRACE_*() and OI_DBG_PRINT?() take the
 * format string and its associated arguments inside a second set of
 * parentheses.
 *
 * The macros OI_SLOG_*() take a parameter, status, in addition to the normal
 * format and format arguments for reporting the status code in both debug and
 * release mode.  See example:
 *
   @code
   OI_SLOG_ERROR(OI_FAIL, ("%s, we have a problem", cityStr));
  
   Debug Output => 1234.567 MODULE module.c:123    **** Houston, we have a problem: OI_FAIL
   Release Output => 1234.567 SLOG_ERROR: 45,123,114
   @endcode
 * 
 * The debug print functions uses printf style formatting and allows multiple
 * arguments.  The debug print function should always be called via the macros
 * listed below, not directly.  The macros set global line and file variables
 * that are used by the debug print function.
 *
 * See sdk/include/oi_debugcontrol.h for details on setting/controlling the
 * debug output.
 * @{
 */

//sean printf
//#define OI_Printf APP_TRACE_DBG

//sean OI_Print
//#define OI_Print OI_Printf

#if defined(OI_SUPPRESS_SLOG_ERROR)  /* if SLOG_ERRORs are suppressed, so is everything else */

#define OI_SLOG_ERROR(_status, _msg)   do {} while (0)
#define OI_SLOG_WARNING(_status, _msg) do {} while (0)
#define _OI_DBG_MSG(_status, _msg)     do {} while (0)

#else /* defined(OI_SUPPRESS_SLOG_ERROR) */

#define _OI_LOG_MSG(_code, _status, _msg)                               \
    do {                                                                \
        if (_OI_Dbg_Check((_code), OI_CURRENT_MODULE, __FILE__, __LINE__)) { \
            _OI_Dbg_Printf _msg ;                                       \
            _OI_Dbg_Complete((_status));                                \
        }                                                               \
    } while (0)

#define _OI_DBG_MSG(_code, _msg)                                             \
    do {                                                                     \
        if (_OI_Dbg_Check((_code), OI_CURRENT_MODULE, __FILE__, __LINE__)) { \
            _OI_Dbg_Printf_Complete _msg ;                                   \
        }                                                                    \
    } while (0)

#if defined(OI_DEBUG) || defined(OI_ENABLE_SLOG_ERROR)
#define OI_SLOG_ERROR(_status, _msg) _OI_LOG_MSG(OI_MSG_CODE_ERROR, (_status), _msg )
#else
#define OI_SLOG_ERROR(_status, _msg) OI_LogError(OI_CURRENT_MODULE, __LINE__, (_status))
#endif

#if defined(OI_DEBUG) || defined(OI_ENABLE_SLOG_WARNING)
#define OI_SLOG_WARNING(_status, _msg) _OI_LOG_MSG(OI_MSG_CODE_WARNING, (_status), _msg )
#else
#define OI_SLOG_WARNING(_status, _msg) do {} while (0)
#endif

#endif /* defined(OI_SUPPRESS_SLOG_ERROR) */

#if defined(OI_DEBUG) || defined(OI_ENABLE_TRACE) || defined(OI_ENABLE_TRACE_USER)
#define OI_TRACE(_msg)      _OI_DBG_MSG(OI_MSG_CODE_TRACE,     _msg )
#else
#define OI_TRACE(_msg)      do {} while (0)
#endif

#if defined(OI_DEBUG) || defined(OI_ENABLE_TRACE) || defined(OI_ENABLE_TRACE_USER)
#define OI_TRACE_USER(_msg)      _OI_DBG_MSG(OI_MSG_CODE_TRACE,     _msg )
#else
#define OI_TRACE_USER(_msg)      do {} while (0)
#endif

#if defined(OI_DEBUG) || defined(OI_ENABLE_DBG_PRINT) || defined(OI_ENABLE_DBG_PRINT1)
#define OI_DBG_PRINT1(_msg)      _OI_DBG_MSG(OI_MSG_CODE_PRINT1, _msg )
#else
#define OI_DBG_PRINT1(_msg)      do {} while (0)
#endif

#if defined(OI_DEBUG) || defined(OI_ENABLE_DBG_PRINT) || defined(OI_ENABLE_DBG_PRINT2)
#define OI_DBG_PRINT2(_msg)      _OI_DBG_MSG(OI_MSG_CODE_PRINT2, _msg )
#else
#define OI_DBG_PRINT2(_msg)      do {} while (0)
#endif


/* Backwards compatibility for old-style macros. */
#define OI_LOG_ERROR(_msg) OI_SLOG_ERROR(OI_STATUS_NONE, _msg )
#define OI_DBGPRINT(_msg)  OI_DBG_PRINT1(_msg )
#define OI_DBGPRINT2(_msg) OI_DBG_PRINT2(_msg )
#define OI_DBGTRACE(_msg)  OI_TRACE_USER(_msg )


#if defined(OI_DEBUG) && !defined(OI_ENABLE_SLOG_ERROR)
#define OI_ENABLE_SLOG_ERROR 1
#endif

#if defined(OI_DEBUG) && !defined(OI_ENABLE_SLOG_WARNING)
#define OI_ENABLE_SLOG_WARNING 1
#endif

/**@}*/

/*
 * A lot of code that is conditionally compiled in with OI_DEBUG is support
 * code for the various debug print macros defined above.  Since it is
 * possible to enable only a specific debug print macro at compile time for a
 * given module, that module will most likely need OI_DEBUG defined as well.
 */
#if !defined(OI_DEBUG) && (defined(OI_ENABLE_SLOG_ERROR) || defined(OI_ENABLE_SLOG_WARNING) || defined(OI_ENABLE_TRACE) || defined(OI_ENABLE_TRACE_USER) || defined(OI_ENABLE_DBG_PRINT) || defined(OI_ENABLE_DBG_PRINT1) || defined(OI_ENABLE_DBG_PRINT2))
#define OI_DEBUG_PER_MOD 1
#endif

/**
 * Do not call this function directly.  Use the OI_SLOG_ERROR(),
 * OI_SLOG_WARNING(), OI_TRACE_*(), and OI_DBG_PRINT?() macros below.
 */
//void _OI_Dbg_Complete(OI_STATUS status);

/**
 * Do not call this function directly.  Use the OI_SLOG_ERROR(),
 * OI_SLOG_WARNING(), OI_TRACE_*(), and OI_DBG_PRINT?() macros below.
 */
void _OI_Dbg_Printf(const OI_CHAR* format, ...);

/**
 * Do not call this function directly.  Use the OI_SLOG_ERROR(),
 * OI_SLOG_WARNING(), OI_TRACE_*(), and OI_DBG_PRINT?() macros below.
 */
void _OI_Dbg_Printf_Complete(const OI_CHAR* format, ...);

/**
 * Do not call this function directly.  Use the OI_SLOG_ERROR(),
 * OI_SLOG_WARNING(), OI_TRACE_*(), and OI_DBG_PRINT?() macros below.
 */
OI_BOOL _OI_Dbg_Check(OI_UINT8 debugLevel,
                      OI_UINT8 module,
                      const OI_CHAR *filename,
                      OI_UINT lineno);

/**
 * DEBUG_ONLY - a little macro for single-line statements and variable declarations
 * Avoids the clutter of \#ifdef OI_DEBUG in the source code.
 *
 * For example, to declare a variable that is only used when OI_DEBUG is
 * defined:
   @code
       DEBUGONLY( OI_UINT counter = 0; )
       ...
       DEBUGONLY( ++counter; );
       ...
       OI_DBG_PRINT1(1, ("Counter is %d", counter));
   @endcode
 */

#if defined(OI_DEBUG)
#define DEBUG_ONLY(stmt)    stmt
#else
#define DEBUG_ONLY(stmt)
#endif

/*
 * Utility functions - data type to text
 */

//OI_CHAR *OI_LinkKeyTypeText(OI_BT_LINK_KEY_TYPE keyType);
//sean
//OI_CHAR *OI_KeyTypeText(OI_KEY_TYPE keyType);

//OI_CHAR *OI_AmpKeyTypeText(OI_AMP_TYPE keyType);

//OI_CHAR *OI_LinkTypeText(OI_UINT8 linkType);                /**< ACL/SCO/eSCO */

//OI_CHAR *OI_LinkModeText(OI_UINT8 mode);                    /**< active/sniff/park/hold? */

//OI_CHAR *OI_HciDataTypeText(OI_UINT8 hciDataType) ;         /**< h4 types */

//OI_CHAR *OI_HciRoleText(OI_UINT8 role);                     /**< master/slave */

//OI_CHAR  *OI_IoCapsText(OI_UINT8  caps);                    /**< SSP io capabilities */

//OI_CHAR *OI_EIR_TypeText(OI_UINT8 type);                    /**< EIR data types */

//sean
//OI_CHAR* OI_L2CAP_ModeText(OI_L2CAP_MODE mode);             /**< L2CAP mode */

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_DEBUG_H */

