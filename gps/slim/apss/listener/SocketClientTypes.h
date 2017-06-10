/*============================================================================
@file SocketClientTypes.h

  Socket client types header file

  Header defines data structures for socket communication
  between the sensor client on AP and socket client listener
  interface of daemon.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef SOCKET_CLIENT_TYPES_H
#define SOCKET_CLIENT_TYPES_H

#include <slim_utils.h>

#define SLIM_MSG_SOCKET_PORT_NUM   28602
#ifdef DEBUG_X86
#define SLIM_MSG_SOCKET_PORT_PATH  "/tmp/gsiff_quipc_unix_socket"
#else
#define SLIM_MSG_SOCKET_PORT_PATH  "/data/misc/location/quipc/gsiff_socket"
#endif

#define MAX_SENSOR_DATA_SAMPLES 50
#define MAX_BUFFER_SIZE 1024

/* Evaluates the actual length of `sockaddr_un' structure. */
#define SOCKADDR_UN_SIZE(p) ((size_t)(((struct sockaddr_un *) NULL)->sun_path) \
           + strlen ((p)->sun_path))

/**
 * Enum for Message ID
 */
typedef enum
{
  eSLIM_SOCKET_CLIENT_MSG_ID_NONE = 0,

  eSLIM_SOCKET_CLIENT_MSG_ID_OPEN_REQ = 1000,
  eSLIM_SOCKET_CLIENT_MSG_ID_OPEN_RESP,

  eSLIM_SOCKET_CLIENT_MSG_ID_CLOSE_REQ,
  eSLIM_SOCKET_CLIENT_MSG_ID_CLOSE_RESP,

  eSLIM_SOCKET_CLIENT_MSG_ID_SENSOR_DATA_REQ,
  eSLIM_SOCKET_CLIENT_MSG_ID_SENSOR_DATA_IND,
  eSLIM_SOCKET_CLIENT_MSG_ID_SENSOR_DATA_RESP,

  eSLIM_SOCKET_CLIENT_MSG_ID_PEDOMETER_REQ,
  eSLIM_SOCKET_CLIENT_MSG_ID_PEDOMETER_IND,
  eSLIM_SOCKET_CLIENT_MSG_ID_PEDOMETER_RESP,

  eSLIM_SOCKET_MESSAGE_ID_MAX = 2147483647
} SocketClientMsgId;


/**
 * The message header. Used in both incoming and outgoing messages of socket clients
 */
typedef struct
{
    slimMessageHeaderStructT slimHeader;
    /**<   Total size for this message (in bytes) */
    SocketClientMsgId msgId;
    /**<   Unique message ID for each message type */
    uint32_t msgSize;
    /**<   Total size for this message (in bytes) */

} SocketClientMsgHeader;

typedef struct
{
    SocketClientMsgHeader msgHeader;
    slim_EnableSensorDataTxnStructType msgPayload;
} SocketClientSensorDataReq;

typedef struct
{
    SocketClientMsgHeader msgHeader;
    slimSensorDataStructT msgPayload;
} SocketClientSensorDataInd;

typedef struct
{
    SocketClientMsgHeader msgHeader;
} SocketClientGenericResponse;

typedef struct
{
    SocketClientMsgHeader msgHeader;
    slim_OpenTxnStructType msgPayload;
} SocketClientOpenReq;

typedef struct
{
    SocketClientMsgHeader msgHeader;
    slim_GenericTxnStructType msgPayload;
} SocketClientCloseReq;

typedef struct
{
    SocketClientMsgHeader msgHeader;
    slim_GetProviderTimeRequestTxnStructType msgPayload;
} SocketClientTimeReq;

typedef struct
{
    SocketClientMsgHeader msgHeader;
    slimGetProviderTimeResponseStructT msgPayload;
} SocketClientTimeInd;

typedef struct
{
    SocketClientMsgHeader msgHeader;
    slim_EnablePedometerTxnStructType msgPayload;
} SocketClientPedometerDataReq;

typedef struct
{
    SocketClientMsgHeader msgHeader;
    slimPedometerDataStructT msgPayload;
} SocketClientPedometerDataInd;

#endif //SOCKET_CLIENT_TYPES_H
