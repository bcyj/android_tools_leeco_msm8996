/*============================================================================
  FILE:          SocketClientWrapper.cpp

  OVERVIEW:      Socket client wrapper converts request from sensor cleint
                 into socket messages and forwards it to the daemon.
                 Also delivers sensor data from provider to sensor client.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_NDDEBUG 0
#define LOG_TAG "Slim_SocketClientWrapper"
#include <stdlib.h>
#include <log_util.h>

#include "SocketClientWrapper.h"

namespace socket_client_wrapper {

SocketClientWrapper* SocketClientWrapper::mMe = NULL;
int SocketClientWrapper::mSocketFd = 0;
slimNotifyCallbackFunctionT SocketClientWrapper::mCallbackFunction = NULL;
pthread_t SocketClientWrapper::SCWthread = NULL;

/**
@brief Constructor

Function class initialization
*/
SocketClientWrapper::SocketClientWrapper()
{
     LOC_LOGD("%s:%d]: SocketClientWrapper created. SocketClientWrapper: %p\n",
             __func__, __LINE__, this);

     OpenConnection();

}

/**
@brief Open the socket connection to server socket to establish
       two way communication with server socket

Function tries to establish connection with the server socket. Launches
the SocketClientWrapperThread to receive sensor data indications/responses
from the slim daemon
*/
int SocketClientWrapper::OpenConnection()
{
    int errcode = -1;
    struct sockaddr_un server;
    int servlen;

    LOC_LOGI("%s[%d]: Trying to connect to server socket. path %s\n",
                   __func__, __LINE__, SLIM_MSG_SOCKET_PORT_PATH);

    /* Create socket */
    SocketClientWrapper::mSocketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (SocketClientWrapper::mSocketFd < 0)
    {
        // error handling
        LOC_LOGE("%s[%d]: error opening stream socket, errno=%d\n", __func__, __LINE__, errno);
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sun_family = AF_UNIX;
    strlcpy(server.sun_path, SLIM_MSG_SOCKET_PORT_PATH, sizeof(SLIM_MSG_SOCKET_PORT_PATH));
    servlen = strlen(server.sun_path) + sizeof(server.sun_family);
    /* Connect to socket */
    if (connect(SocketClientWrapper::mSocketFd, (struct sockaddr *)&server, servlen) < 0)
    {
        LOC_LOGE("%s[%d]: error connecting stream socket, errno=%d\n", __func__, __LINE__, errno);
        return -3;
    }

    //create thread
    /* Create server socket thread for establishing communication to AP Sensor client*/
    if (pthread_create(&(SocketClientWrapper::SCWthread), NULL, SocketClientWrapper::SocketClientWrapperThread, NULL) != 0)
    {
        LOC_LOGE("%s: Error cannot create SocketClientWrapperThread", __FUNCTION__);
        return -1;
    }
    return 0;
}

/**
@brief Get class object

Function to get class object. Class is a singleton.
*/
SocketClientWrapper* SocketClientWrapper::get()
{
    if (NULL == mMe) {
        mMe = new SocketClientWrapper();
    }
    return mMe;
}

/**
@brief Register sensor client with SLIM CORE

Function sends message to slim daemon to initiate client
registration with SLIM CORE

@param  z_Txn: Parameters for sensor client registration
*/
int SocketClientWrapper::ClientRegister(slim_OpenTxnStructType &z_Txn)
{
    LOC_LOGD("%s:%d] Received client register", __func__, __LINE__);

    SocketClientOpenReq openRequest;
    memset(&openRequest, 0, sizeof(openRequest));

    openRequest.msgHeader.msgId = eSLIM_SOCKET_CLIENT_MSG_ID_OPEN_REQ;
    openRequest.msgHeader.msgSize = sizeof(SocketClientOpenReq);
    openRequest.msgPayload = z_Txn;

    LOC_LOGD("%s:%d] sizeof(SocketClientOpenReq) is %d, sizeof(openRequest.msgHeader) is %d",
             __func__, __LINE__, sizeof(SocketClientOpenReq), sizeof(openRequest.msgHeader));

    SocketClientWrapper::mCallbackFunction = openRequest.msgPayload.fn_Callback;
    LOC_LOGD("%s:%d] Received client register, p_Handle is %08X", __func__, __LINE__, z_Txn.p_Handle);
    LOC_LOGD("%s:%d] Received fn_Callback is %08X", __func__, __LINE__, openRequest.msgPayload.fn_Callback);
    // If socket_not_present or socket_not_connected, then error/warning
    if ( (SocketClientWrapper::mSocketFd < 0)
        )
    {
        LOC_LOGE("%s[%d]: socket not present/connected\n",
                        __func__, __LINE__);
        return -1;
    }

    if (write(SocketClientWrapper::mSocketFd, &openRequest, sizeof(openRequest)) < 0)
    {
        LOC_LOGE("%s[%d]: error writing on socket\n", __func__, __LINE__);
        return -1;
    }
    return 0;
}

/**
@brief Deregister sensor client with SLIM CORE

Function sends message to slim daemon to initiate client
de-registration with SLIM CORE

@param  z_Txn: Parameters for sensor client de-registration
*/
int SocketClientWrapper::ClientDeRegister(slim_GenericTxnStructType &z_Txn)
{
    LOC_LOGD("%s:%d] Received client deregister", __func__, __LINE__);

    SocketClientCloseReq closeRequest;
    memset(&closeRequest, 0, sizeof(closeRequest));

    closeRequest.msgHeader.msgId = eSLIM_SOCKET_CLIENT_MSG_ID_CLOSE_REQ;
    closeRequest.msgHeader.msgSize = sizeof(SocketClientCloseReq);
    closeRequest.msgPayload = z_Txn;

    // If socket_not_present or socket_not_connected, then error/warning
    if ( (SocketClientWrapper::mSocketFd < 0)
        )
    {
        LOC_LOGE("%s[%d]: socket not present/connected\n",
                        __func__, __LINE__);
        return -1;
    }

    if (write(SocketClientWrapper::mSocketFd, &closeRequest, sizeof(closeRequest)) < 0)
    {
        LOC_LOGE("%s[%d]: error writing on socket\n", __func__, __LINE__);
        return -1;
    }
    return 0;
}

/**
@brief Enabling or disabling of sensor data streaming for sensor client

Function sends enabling or disabling of sensor data
streaming for sensor client to slim daemon

@param  z_Txn: Request parameters for sensor streaming
*/
int SocketClientWrapper::requestSensorData(slim_EnableSensorDataTxnStructType &z_Txn)
{
    LOC_LOGD("%s:%d] Received sensor data request", __func__, __LINE__);

    SocketClientSensorDataReq sensorDataRequest;
    memset(&sensorDataRequest, 0, sizeof(sensorDataRequest));

    sensorDataRequest.msgHeader.msgId = eSLIM_SOCKET_CLIENT_MSG_ID_SENSOR_DATA_REQ;
    sensorDataRequest.msgHeader.msgSize = sizeof(SocketClientSensorDataReq);
    sensorDataRequest.msgPayload = z_Txn;
    LOC_LOGD("%s:%d] Received sensor request, p_Handle is %08X", __func__, __LINE__, z_Txn.z_TxnData.p_Handle);
    // If socket_not_present or socket_not_connected, then error/warning
    if ( (SocketClientWrapper::mSocketFd < 0)
        )
    {
        LOC_LOGE("%s[%d]: socket not present/connected\n",
                        __func__, __LINE__);
        return -1;
    }

    if (write(SocketClientWrapper::mSocketFd, &sensorDataRequest, sizeof(sensorDataRequest)) < 0)
    {
        LOC_LOGE("%s[%d]: error writing on socket\n", __func__, __LINE__);
        return -1;
    }
    return 0;
}

/**
@brief Thread listening for sensor data indications/responses
from slim daemon

Function listens for sensor data indications/responses from slim daemon
and forwards it to client using client callback function

@param  arg: NULL
*/
void* SocketClientWrapper::SocketClientWrapperThread(void* arg)
{
    SocketClientMsgHeader slim_msg_hdr;
    void* slim_msg_ptr;
    int rval = -1;

    LOC_LOGI("%s[%d]: SLIMCW_SOCK_HDLR thread starting\n", __func__, __LINE__);

    while (1)
    {
        memset(&slim_msg_hdr, 0, sizeof(slim_msg_hdr));
        LOC_LOGD("%s[%d]: Ready to read from socket...\n", __func__, __LINE__);

        if ((rval  = read(SocketClientWrapper::mSocketFd, &slim_msg_hdr, sizeof(slim_msg_hdr))) < 0)
        {
            LOC_LOGE("%s[%d]: error reading stream message for header, errno %d\n",
                            __func__, __LINE__, errno);
            break;
        }
        else if (rval == 0)
        {
            LOC_LOGE("%s[%d]: Ending connection\n", __func__, __LINE__);
            break;
        }
        else
        {
            /* Message received: check msg size */
            uint32_t q_msg_size = slim_msg_hdr.msgSize;
            uint32_t q_msg_payload_size = q_msg_size - sizeof(SocketClientMsgHeader);
            uint32_t u_num_bytes_read = rval;
            uint32_t u_num_bytes_remaining = 0;

            void* slim_msg_ptr = (void*) malloc(q_msg_size);
            if(slim_msg_ptr == NULL)
            {
                LOC_LOGE("%s: slim_msg_ptr malloc %d failed", __FUNCTION__,  q_msg_payload_size);
                break;
            }

            LOC_LOGD("%s[%d]: Recv'd message header, id=%d, size=%d,  transaction=%d, payload-size=%d, rval=%d\n",
                          __func__, __LINE__, slim_msg_hdr.msgId, slim_msg_hdr.msgSize,
                           slim_msg_hdr.slimHeader.txnId, q_msg_payload_size, rval);

            u_num_bytes_remaining = q_msg_payload_size;
            while (u_num_bytes_remaining > 0)
            {
                if ((rval  = read(SocketClientWrapper::mSocketFd, (slim_msg_ptr+u_num_bytes_read), u_num_bytes_remaining)) <= 0)
                {
                    LOC_LOGE("%s[%d]: error reading stream message for payload\n", __func__, __LINE__);
                    // TODO: Response to client?
                }
                else
                {
                    u_num_bytes_read += rval;
                    u_num_bytes_remaining -= rval;
                    LOC_LOGD("%s[%d]: Ready to process payload.. rval=%d, total-read=%d, rem=%d\n",
                                  __func__, __LINE__, rval, u_num_bytes_read, u_num_bytes_remaining);
                }
            }
            if (u_num_bytes_read == q_msg_size)
            {
                SocketClientWrapper::ProcessReceivedMessage(slim_msg_hdr, q_msg_payload_size /*slim_msg_hdr*/, slim_msg_ptr);
            }
            free(slim_msg_ptr);
        }
    }

    LOC_LOGI("%s[%d]: Done listening for sensor data indications hence"
             "killing thread and closing socket\n",
             __func__, __LINE__);
    close(SocketClientWrapper::mSocketFd);
    SocketClientWrapper::mSocketFd = -1;
    //TODO: Notify client

    LOC_LOGI("%s[%d]: SLIMCW_SOCK_HDLR thread exiting\n", __func__, __LINE__);
    return NULL;
}

/**
@brief Process received sensor data indications/response from
       slim daemon

Function forwards sensor client the sensor data
indication/response from slim daemon

@param  slim_msg_hdr: Message header
@param  q_msg_payload_size: Payload size
@param  slim_msg_ptr: Pointer to indication/response data
*/
int SocketClientWrapper::ProcessReceivedMessage
(
   SocketClientMsgHeader &slim_msg_hdr,
   uint32_t q_msg_payload_size,
   void* slim_msg_ptr
)
{
    int ret_val = 0;
    //TODO: Forward response to the client
    if ( (slim_msg_ptr == NULL) )
    {
        LOC_LOGE("%s[%d]: SLIMCW_SOCK_HDLR: Null payload %p\n",
                        __func__, __LINE__, slim_msg_ptr);
        return -1;
    }

    LOC_LOGI("%s[%d]: SLIMCW_SOCK_HDLR: received msg-id=%d, payload-size=%d\n",
                   __func__, __LINE__, slim_msg_hdr.msgId, q_msg_payload_size);
    switch (slim_msg_hdr.msgId)
    {
    case eSLIM_SOCKET_CLIENT_MSG_ID_OPEN_RESP:
    {
        LOC_LOGV("%s:%d] Received open response",
                 __func__,__LINE__);
        break;
    }
    case eSLIM_SOCKET_CLIENT_MSG_ID_SENSOR_DATA_RESP:
    {
        LOC_LOGV("%s:%d] Received sensor data response",
                 __func__,__LINE__);
        break;
    }
    case eSLIM_SOCKET_CLIENT_MSG_ID_CLOSE_RESP:
    {
        LOC_LOGV("%s:%d] Received close response",
                 __func__,__LINE__);
        break;
    }

    case eSLIM_SOCKET_CLIENT_MSG_ID_SENSOR_DATA_IND:
    {

        LOC_LOGV("%s:%d] Received sensor indication",
                 __func__,__LINE__);

        LOC_LOGV("%s:%d] Received sensor service is %d",
                 __func__,__LINE__, slim_msg_hdr.slimHeader.service);

        if(slim_msg_hdr.slimHeader.msgId ==eSLIM_MESSAGE_ID_SENSOR_DATA_IND)
        {
            SocketClientSensorDataInd *pz_SensorData =
                (SocketClientSensorDataInd*)slim_msg_ptr;

            IF_LOC_LOGV {
                LOC_LOGV("%s: Received sensor-%d data with len-%d", __func__,
                        pz_SensorData->msgPayload.sensorType, pz_SensorData->msgPayload.samples_len);

                for(uint32_t i=0;i<pz_SensorData->msgPayload.samples_len;i++)
                {
                    LOC_LOGV("%s: Received data with time offset-%d, data-(%f,%f,%f)", __func__,
                            pz_SensorData->msgPayload.samples[i].sampleTimeOffset,
                            pz_SensorData->msgPayload.samples[i].sample[0],
                            pz_SensorData->msgPayload.samples[i].sample[1],
                            pz_SensorData->msgPayload.samples[i].sample[2]);
                }
            }

            if(SocketClientWrapper::mCallbackFunction)
                SocketClientWrapper::mCallbackFunction(0,&slim_msg_hdr.slimHeader, (void *)&pz_SensorData->msgPayload);
        }
        else
        {
            LOC_LOGV("%s:%d] Received is not indication",
                 __func__,__LINE__);
        }


        break;
    }

    default:
        LOC_LOGE("%s[%d]: Unknown/unexpected message type %d\n",
                        __func__, __LINE__, slim_msg_hdr.msgId);
        break;
    }

    LOC_LOGD("SLIMCW: %s process msg, returned = %d \n", __func__, ret_val);
    return ret_val;
}
};//namespace
