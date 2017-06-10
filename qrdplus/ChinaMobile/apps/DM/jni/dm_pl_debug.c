#include <stdio.h>
#include <stdarg.h>
#include "dm_pl_debug.h"
#include "dm_error.h"
#include "dm_pl_fs.h"
//#include <utils/Log.h>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"DMLIB ==>",__VA_ARGS__)

#ifndef DM_LOG_SWITCH
#define DM_LOG_SWITCH
#endif

#ifdef  DM_LOG_SWITCH

//#define LOGE
#ifndef DM_TRACE
#define DM_TRACE LOGE
#endif

#define     LOG_MSG_LEN     200 //8*1024
static IS8 logMsg[LOG_MSG_LEN] = { 0 };

static IU32 dmDebugMask = 0xFFFFFFFF;
static const char *getStringByMask(IU32 mask);

void dmPrintLog(IS8 *str) {
    DM_TRACE("MMIDM==> dmPrintLog:[%s]", str);
}

/**
 *  @brief   This function outputs message for debug purpose.
 *  @param   mask  [I]Refer to Mask value to use.
 *  @param   str   [I]Message to print.
 *  @param   num   [I]Print param.
 *
 *  @return  none.
 *
 *  @note    none.
 */
void dm_debug_trace(IU32 mask, const IS8 *str, ...) {
    if (dmDebugMask & mask) {
        //IS8  logMsg[LOG_MSG_LEN]={0};
        va_list arglist;
        va_start(arglist, str); /* Initialize variable arguments. */

        memset(logMsg, 0, LOG_MSG_LEN);
        strlcpy(logMsg, (char *) getStringByMask(mask), sizeof(logMsg));
        vsnprintf(logMsg + strlen(logMsg), sizeof(logMsg) - strlen(logMsg), str, arglist);

        dmPrintLog(logMsg);

        va_end(arglist); /* Reset variable arguments.      */
    }
}

/**
 * get the string of specified mask
 *
 * \param mask mask ID
 *
 * \return the string name of the the specified mask
 */
static const char *getStringByMask(IU32 mask) {
    static struct {
        IU32 mask;
        char name[30];
    } maskPair[] = { { DM_DEBUG_TREE_MASK, "*** TREE *** " }, {
            DM_DEBUG_SOCKET_MASK, "*** SOCKET *** " } };

    static char *szUnknown = "*** UNKNOWN ***";

    int i = 0, n = sizeof(maskPair) / sizeof(maskPair[0]);
    for (; i < n; i++) {
        if (mask == maskPair[i].mask) {
            return maskPair[i].name;
        }
    }

    return szUnknown;
}

/**
 *  @brief   This function outputs message for debug purpose.
 *
 *  @param   mask  [I]Refer to Mask value to use.
 *  @param   str   [I]Message to print.
 *  @param   num   [I]Print param.
 *
 *  @return  none.
 *
 *  @note    none.
 */
void dm_debug_print(IU32 mask, IS8 * str, IS32 num) {
    if ((dmDebugMask & mask) != 0) {
        IS8 *pBuf;

        memset(logMsg, 0, LOG_MSG_LEN);

        if (str != NULL ) {
            //memset(logMsg,0,8000);
            snprintf(logMsg, sizeof(logMsg), "%s ===== value=%d ====== %s",
                    getStringByMask(mask), num, str);
        } else {
            strlcpy(logMsg, "dm_debug_print error, str == NULL\n", sizeof(logMsg));
        }
        dmPrintLog(logMsg);
    }
}

/**
 * this function outputs message for debug purpose
 *
 * \param mask mask type
 * \param file the filename which this function is called
 * \param line the line which this function is called
 * \param str  the message to print
 * \param num  print parameter
 *
 * \return none
 */
void dm_debug_print_ex(IU32 mask, const IS8 *file, IU32 line, const IS8 *str,
        IU32 num) {
    if ((dmDebugMask & mask) != 0) {
        memset(logMsg, 0, LOG_MSG_LEN);
        strlcpy(logMsg, getStringByMask(mask), sizeof(logMsg));
        snprintf(logMsg + strlen(logMsg), sizeof(logMsg) - strlen(logMsg),
                "File:%s  Line:%d   ", file, line);
        snprintf(logMsg + strlen(logMsg), sizeof(logMsg) - strlen(logMsg), str, num);
        dmPrintLog(logMsg);
    }
}

/**
 *  @brief   This function sets the print mask flags.
 *
 *  @param   mask   [I]The value to set.
 *
 *  @return  none.
 *
 *  @note    none.
 */
void dmDebugSetMask(IU32 mask) {
    dmDebugMask = mask;
}

/**
 *  @brief   This function gets the print mask flags.
 *
 *  @param   none.
 *
 *  @return  Debug Print Mask value.
 *
 *  @note    none.
 */
IU32 dmDebugGetMask(void) {
    return dmDebugMask;
}

#endif // DM_LOG_SWITCH
