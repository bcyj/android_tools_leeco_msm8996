/**
@file
@internal
*/

/**********************************************************************************
  $AccuRev-Revision: 1189/4 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/
/*==============================================================================
  Edit History

  This section contains comments describing changes made to the module. Notice
  that changes are listed in reverse chronological order. Please use ISO format
  for dates.

  $Header: //source/qcom/qct/core/sensors/dsps/common/main/latest/util/memmgr/src/oi_debug.c#1 $
  $DateTime: 2011/01/28 12:40:41 $

  when       who  what, where, why 
  ---------- ---  -----------------------------------------------------------
  2010-12-03 pg   Updated module strings for OI_MODULE.

==============================================================================*/

//#define __SNS_MODULE__ OI_MODULE_SUPPORT
//sean
//#include "oi_argcheck.h"
#include "oi_debug.h"
#include "oi_assert.h"
//#include "oi_osinterface.h"
#include "oi_memmgr.h"

//#include "oi_debugcontrol.h"
//#include "oi_modules.h"

//#include "oi_bt_assigned_nos.h"
#include "oi_std_utils.h"
//#include "oi_dispatch.h"

//#include "oi_debugcontrol.h"
#include "oi_modules.h"

#include "oi_dump.h"
#include "oi_support_init.h"
//#include "oi_hcispec.h"
//#include "oi_varstring.h"
//#include "oi_bt_stack_config.h"
//#include "oi_config_table.h"
#ifdef OI_DEBUG

#ifdef OI_REENTRANT
#include "oi_thread.h"
#endif

#ifdef SNIFFLOG
#include "oi_sniff.h"
#endif


#define MAX_LEVEL     9     /**< Maximum verbosity level. */
#define DEFAULT_LEVEL 2     /**< Initial default verbosity level. */

typedef struct {
    OI_UINT8 levelEnableFlags;
    OI_BOOL  enableLineNum;
    OI_BOOL  overrideAll;
    OI_BOOL  useDefault;
} OI_DBG_OUTPUT_CTRL;


/**
 * If RAM is limited we use smaller debug print buffers. This means that debug
 * messages may be truncated.
 */

#ifdef SMALL_RAM_MEMORY

#define MAX_HEX_TEXT_LEN   100  /* Must be at least 18 bytes for BD Addr */
#define MAX_DBG_MSG_LEN    128
#define MAX_DBG_HDR_LEN     40

#else

#define MAX_HEX_TEXT_LEN   2048  /* Must be at least 18 bytes for BD Addr */
#define MAX_DBG_MSG_LEN    4096
#define MAX_DBG_HDR_LEN      64

#endif /* SMALL_RAM_MEMORY */


/**
 * Mutex for dbgprint
 */
#ifdef OI_REENTRANT

static OI_MUTEX mutex;

#define LOCK    OI_Mutex_Lock(&mutex);
#define UNLOCK  OI_Mutex_Unlock(&mutex);

#else

#endif

/**
 * Formatting buffer (possibly large) shared by OI_DbgPrint and OI_Printf
 */
static OI_CHAR msgBuf[MAX_DBG_MSG_LEN];

/**
 * Smaller scratch buffer
 */
static OI_CHAR smallFmtBuf[MAX_HEX_TEXT_LEN];

#if defined(OI_DEBUG) || defined(OI_DEBUG_PER_MOD)
/**
 * Header buffer only used by DbgPrint
 */
static OI_CHAR hdrBuf[MAX_DBG_HDR_LEN];

typedef struct {
    OI_BOOL checked;         /* make sure _OI_Dbg_Check() is called before _OI_DbgPrint() */
    OI_UINT8 debugLevel;     /* requested debug level */
    OI_UINT8 module;         /* current module */
    OI_UINT8 hdrIndent;      /* number of characters in the debug header */
    OI_UINT8 verbosity;      /* verbosity level of */
    OI_CHAR *hdr;            /* debug header string */
    OI_BOOL linePrinted;     /* TRUE if the line was printed. */
} DBG_INFO;

static DBG_INFO dbgInfo;


/** The debug info for the modules. IMPORTANT: these must be in the same order
 *  as in the OI_MODULE definition found in sdk/include/oi_modules.h.
 *  Keep the strings to 4 characters or less so that debug output is compact.
 */
static const OI_CHAR* moduleString[] = {
    /* profiles and protocols --> updates to oi_modules.h */

    "SAM",                          /*  00 SNS_SAM                          */
    "SMGR",                         /*  01 SNS_SMGR                         */
    "SMR",                          /*  02 SNS_SMR                          */
    "PM",                           /*  03 SNS_POWER                        */
    "TEST",                         /*  04 SNS_TEST                         */
    "DDF",                          /*  05 SNS_DDF                          */
    "EM",                           /*  06 SNS_MODULE6                      */
    "TMP7",                         /*  07 SNS_MODULE7                      */
    "TMP8",                         /*  08 SNS_MODULE8                      */
    "TMP9",                         /*  09 SNS_MODULE9                      */

    "MMGR",                         /*  10 OI_MODULE_MEMMGR                 */
    /* Various pieces of code depend on these last 2 elements occuring in a specific order:
       OI_MODULE_ALL must be the 2nd-to-last element
       OI_MODULE_UNKNOWN must be the last element
       */

    "ALL",                           /* OI_MODULE_ALL */
    "UNKN"                           /* OI_MODULE_UNKNOWN */
};


/* Must be same number of members as moduleString */
static OI_DBG_OUTPUT_CTRL moduleDbgControl[OI_NUM_MODULES + 2] = {{ 0 }};

#endif

void OI_Printf(const OI_CHAR* format, ...)
{
    UNREFERENCED_PARAMETER(format); //WIN32

    //va_list argp;

    //va_start(argp, format);
    //OI_VPrintf(format, argp);
    //va_end(argp);
}



static OI_CHAR *findExtent(OI_CHAR *msg)
{
    while (*msg != 0 && *msg != '\n') {
        msg++;
    }
    return msg;
}

#define dbgIsNum(_c) ((OI_UINT8)((_c) - '0') < (OI_UINT8)10)


const OI_CHAR *OI_ModuleToString(OI_MODULE module)
{
    if (module > OI_MODULE_ALL) {
        return NULL;
    } else {
        return moduleString[module];
    }
}



OI_BOOL OI_CheckDebugControl(OI_MODULE module, OI_UINT8 allMsgEnable, OI_UINT8 oneMsgEnable)
{
    OI_UINT8 enable;

    enable = moduleDbgControl[module].levelEnableFlags;
    if (!moduleDbgControl[module].overrideAll) {
        enable |= moduleDbgControl[OI_MODULE_ALL].levelEnableFlags;
    }
    return (((enable & allMsgEnable) == allMsgEnable) && (!oneMsgEnable || (enable & oneMsgEnable)));
}


#endif

/*****************************************************************************/


