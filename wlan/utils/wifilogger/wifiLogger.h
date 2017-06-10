/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*/

/******************************************************************************
 * wifiLogger.h
 *
 * This file contains preprocessor defintion and other datatype definitions
 * for WifiLogger_app
 *
 * Eranna Vinay Krishna, 05/23/2014
 *
 ******************************************************************************/

#ifndef _WIFI_LOGGER_H_
#define _WIFI_LOGGER_H_

#define DEFAULT_SELECT_TIMEOUT (3) //3 Sec
#define MAX_DRV_MSG_SIZE (1024*8) //8K

#define MAX_FILE_SIZE (128*1024)
#define MAX_FILES 4
#define DEFAULT_LOG_PATH "/sdcard/"
#define FILENAME_PREFIX  "WLANDrv_Logs_"

typedef enum eWLogMsgTypes {
   WLAN_NL_TYPE_LOG_REG = 1, // Message to driver to register
   WLAN_NL_TYPE_LOG_MSG = 89, // Message from driver with logs
} tWLogMsgTypes;

typedef struct sWLogServerContext {
   int        radio;
   tAniIpc    *ipcs;   /* IPC struct for on which the server listens */
   tAniIpc    *ipcnl;  /* IPC struct the Netlink socket that the
                        * server uses to pass messages back and
                        * forth from the Pseudo Driver kernel
                        * module.
                        */
   tAniIpc    *clIpc;  /* The accepted socket to the client */
   struct nlmsghdr nl; /* A prebuilt and cached Netlink msg hdr */
   struct sockaddr_nl *snl;/* return from getsockname in aniAsfIpcOpen */
   int logToConsole;
   int logToSDCard;
   char logFilePath[1024];
} tWLogCtxt ;

#endif // _WIFI_LOGGER_H__
