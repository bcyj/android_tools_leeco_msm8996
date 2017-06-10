/*============================================================================
  FILE:          SocketClientListener.cpp

  OVERVIEW:      Socket client listener is interface to AP sensor clients
                 (PIP) Sensor data requests and sensor data
                 injection is through unix socket.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_NDDEBUG 0
#define LOG_TAG "Slim_SocketClientListener"
#include <log_util.h>
#include <SocketClientListener.h>
#include "slim_processor.h"
#include "slim_core.h"
#include <sys/socket.h>
#include <sys/un.h>

namespace socket_client_listener {

SocketClientListener* SocketClientListener::mMe = NULL;
pthread_t SocketClientListener::mSCLthread = NULL;

/**
@brief Constructor

Function class initialization

@param  pMsgQ: Message queue
*/
SocketClientListener::SocketClientListener(void* pMsgQ):
    ClientListener(pMsgQ)
{
    LOC_LOGD("%s:%d]: SocketClientListener created. SocketClientListener: %p\n",
             __func__, __LINE__, this);

     /* Create server socket thread for establishing communication */
    if (pthread_create(&SocketClientListener::mSCLthread, NULL, SocketClientListener::ListenerThreadEntry, this) != 0)
    {
        LOC_LOGE("%s: Error cannot create SocketClientListenerThread", __FUNCTION__);

    }

}

/**
@brief Get class object

Function to get class object. Class is a singleton.

@param  pMsgQ: Message queue
*/
SocketClientListener* SocketClientListener::get(void* pMsgQ)
{
    ENTRY_LOG();
    if (NULL == mMe) {
        mMe = new SocketClientListener(pMsgQ);
    }
    return mMe;
}

//TODO:
// void SocketClientListener::handleSSR()
// {
//     ENTRY_LOG_CALLFLOW();
// }

/**
@brief Handle sensor data injection through socket

Function handles sensor data injection

@param sensorDataInd:  Sensor data
*/
bool SocketClientListener::injectSensorData(SocketClientSensorDataInd sensorDataInd)
{
    LOC_LOGD("%s:%d]\n", __func__, __LINE__);

    int size;
    char buf[MAX_BUFFER_SIZE];
    memset(buf, 0, MAX_BUFFER_SIZE);

    size = sizeof(sensorDataInd);
    if(size < MAX_BUFFER_SIZE)
    {
        memcpy(buf, &sensorDataInd, size);
    }
    else
    {
        LOC_LOGV("%s:%d] Buffer size exceeded the max limit", __func__, __LINE__);
        return false;
    }
    /* send sensor data to client socket */
    if(mClientFd > 0)
    {
        LOC_LOGV("%s:%d] Writing sensor data to client", __func__, __LINE__);
        write(mClientFd, buf, size);
    }
    else
    {
        LOC_LOGE("%s:%d] Could not write sensor data as invalid socket", __func__, __LINE__);
        return false;
    }

    LOC_LOGV("%s:%d] Returning from injectSensorData", __func__, __LINE__);
    return true;
}

/**
@brief Local function that handles callback data

Functionis called to handle indication/response routing

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
void SocketClientListener::callbackHandler(uint64 t_CallbackData,
                                           const slimMessageHeaderStructT *pz_MessageHeader,
                                           void *p_Message)
{
    ENTRY_LOG();
    if (NULL == pz_MessageHeader)
    {
        return;
    }

    switch (pz_MessageHeader->service)
    {
    case eSLIM_SERVICE_SENSOR_ACCEL:
    case eSLIM_SERVICE_SENSOR_ACCEL_TEMP:
    case eSLIM_SERVICE_SENSOR_GYRO:
    case eSLIM_SERVICE_SENSOR_GYRO_TEMP:
    case eSLIM_SERVICE_SENSOR_BARO:
    case eSLIM_SERVICE_SENSOR_MAG_CALIB:
    case eSLIM_SERVICE_SENSOR_MAG_UNCALIB:
        handleSensorService(t_CallbackData, pz_MessageHeader, p_Message);
        break;
    case eSLIM_SERVICE_PEDOMETER:
        handlePedometerService(t_CallbackData, pz_MessageHeader, p_Message);
        break;
    default:
        break;
    }
}

/**
@brief Handler for all sensor service messages

Function is handler for all sensor service messages

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
bool SocketClientListener::handleSensorService(uint64 t_CallbackData,
                                      const slimMessageHeaderStructT *pz_MessageHeader,
                                      void *p_Message)
{
    if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_SENSOR_DATA_IND)
    {
        if (eSLIM_SUCCESS == pz_MessageHeader->msgError &&
            NULL != p_Message)
        {
            slimSensorDataStructT *pz_SensorData =
                (slimSensorDataStructT*)p_Message;

            IF_LOC_LOGV {
                LOC_LOGV("%s: Received sensor-%d data with len-%d", __func__,
                        pz_SensorData->sensorType, pz_SensorData->samples_len);

                for(uint32_t i=0;i<pz_SensorData->samples_len;i++)
                {
                    LOC_LOGV("%s: Received data with time offset-%d, data-(%f,%f,%f)", __func__,
                            pz_SensorData->samples[i].sampleTimeOffset,
                            pz_SensorData->samples[i].sample[0],
                            pz_SensorData->samples[i].sample[1],
                            pz_SensorData->samples[i].sample[2]);
                }
            }

            SocketClientSensorDataInd sensorDataInd;
            sensorDataInd.msgHeader.msgId = eSLIM_SOCKET_CLIENT_MSG_ID_SENSOR_DATA_IND;
            sensorDataInd.msgHeader.msgSize = sizeof(SocketClientSensorDataInd);
            sensorDataInd.msgHeader.slimHeader = *pz_MessageHeader;
            sensorDataInd.msgPayload = *pz_SensorData;

            LOC_LOGI("%s: Sending Sensor Data to Socket client.",__func__);
            injectSensorData(sensorDataInd);
        }
    }
    else if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_SENSOR_DATA_ENABLE_RESP)
    {
        LOC_LOGI("%s: Received sensor enable response", __func__);
    }
    else
    {
         LOC_LOGI("%s: Received invalid message", __func__);
    }
    return true;
}

/**
@brief Handler for all pedometer service messages

Function is handler for all pedometer service messages

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
bool SocketClientListener::handlePedometerService(uint64 t_CallbackData,
                                         const slimMessageHeaderStructT *pz_MessageHeader,
                                         void *p_Message)
{

    if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_PEDOMETER_IND)
    {
        if (eSLIM_SUCCESS == pz_MessageHeader->msgError &&
            NULL != p_Message)
        {
            LOC_LOGI("%s: Received pedometer inject indication ", __func__);

            slimPedometerDataStructT *pz_PedometerData =
                (slimPedometerDataStructT*)p_Message;

            LOC_LOGI("%s: Received Pedometer data,"
                     "step_confidence = %u, step_count = %lu, "
                     "step_count_uncertainity = %ld, step_rate = %4.2f Hz",
                     __FUNCTION__, pz_PedometerData->stepConfidence,
                     pz_PedometerData->stepCount, pz_PedometerData->stepCountUncertainty,
                     pz_PedometerData->stepRate);

            LOC_LOGI("%s: Sending Pedometer Data to Socket client",
                     __func__);
            //TODO:PHASE2
            //injectPedometerData(pedometerData);
        }

    }
    else if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_PEDOMETER_ENABLE_RESP)
    {
        LOC_LOGI("%s: Received pedometer response", __func__);
    }

    return true;
}

/**
@brief Listener thread to read incoming requests from
socket clients and route to it daemon task

Function is a listener thread to read incoming requests from
socket clients and route to it daemon task
*/
//TODO: multiclient support, refinement of the function
int SocketClientListener::SocketClientListenerThread()
{
    struct sockaddr_un addr_un;
    int socket_fd;
    int result = 0;
    const char * const server_addr = SLIM_MSG_SOCKET_PORT_PATH;

    LOC_LOGV("%s:%d] path %s\n", __func__,__LINE__, server_addr);
    socket_fd = socket (AF_UNIX, SOCK_STREAM, 0);
    mServerFd = socket_fd;
    if (socket_fd < 0) {
        LOC_LOGE("%s:%d] socket failed\n", __func__,__LINE__);
        return -1;
    }

    unlink(server_addr);
    memset(&addr_un, 0, sizeof(addr_un));
    addr_un.sun_family = AF_UNIX;
    strlcpy(addr_un.sun_path, server_addr, sizeof(addr_un.sun_path));

    result = bind (socket_fd, (struct sockaddr *) &addr_un,
                   SOCKADDR_UN_SIZE(&addr_un));
    if (result != 0) {
        LOC_LOGE("%s:%d] bind failed! result %d: %s\n", __func__,__LINE__, result, strerror(errno));
        close(socket_fd);
        return -1;
    }

    struct group * gps_group = getgrnam("gps");
    if (gps_group != NULL)
    {
        result = chown (server_addr, -1, gps_group->gr_gid);
        result = (result == 0) ? chmod (server_addr, 0770) : result;
        if (result != 0)
        {
            ALOGE("%s: chown/chmod for socket result, gid = %d, result = %d, error = %d:%s\n",
                  __func__, gps_group->gr_gid, result, errno, strerror(errno));
        }
    }
    else
    {
        ALOGE("%s: getgrnam for gps failed, error code = %d\n", __func__,  errno);
    }

    //listen for client request
    result = listen (socket_fd, 5);
    if (result != 0) {
        LOC_LOGE("%s:%d] listen failed result %d: %s\n", __func__,__LINE__, result, strerror(errno));
        close(socket_fd);
        return -1;
    }
    LOC_LOGV("%s:%d] succeeded. Return Val %d\n", __func__, __LINE__, socket_fd);

    char buf[MAX_BUFFER_SIZE];
    int len = -1;
    memset(buf, 0, sizeof(buf));
    int bytes_read = 0;
    int bytes_remaining = 0;

    while(1)
    {
        mClientFd = accept(socket_fd, NULL, 0);
        if (mClientFd == -1)
        {
            LOC_LOGE(" Error accepting client connection\n");
            continue;
        }

        while(1)
        {
            SocketClientMsgHeader msg_header;
            int header_len = sizeof(msg_header);

            LOC_LOGV(" header_len %d\n", header_len);
            len = read(mClientFd, &msg_header, header_len);
            LOC_LOGV(" Read client result %d ", len);
            if (len > 0)
            {
                LOC_LOGV("%s:%d] Received data from client len=%d, msgId=%d",
                         __func__,__LINE__, len, msg_header.msgId);

                /*read the req payload */
                int req_payload_len =  msg_header.msgSize - sizeof(SocketClientMsgHeader);
                LOC_LOGV("%s:%d] req_payload_len=%d", __func__,__LINE__, req_payload_len);
                bytes_remaining = req_payload_len;
                bytes_read = len;
                void* p_req_payload = (void*) malloc(msg_header.msgSize);
                if(p_req_payload == NULL)
                {
                    LOC_LOGE("%s: p_req_payload malloc %d failed", __FUNCTION__, bytes_remaining);
                    break;
                }

                while(bytes_remaining > 0)
                {
                    len = read(mClientFd, (p_req_payload+bytes_read),bytes_remaining);
                    if(len < 0)
                    {
                        LOC_LOGE("Socket read error");
                    }
                    else
                    {
                        bytes_read += len;
                        bytes_remaining -= len;
                    }
                }

                // if(bytes_read == req_payload_len)
                // {
                    switch(msg_header.msgId)
                    {
                    case eSLIM_SOCKET_CLIENT_MSG_ID_SENSOR_DATA_REQ:
                    {
                        LOC_LOGD("%s:%d] Received sensor data request, msg_id is %d", __func__, __LINE__, msg_header.msgId);

                        SocketClientSensorDataReq* request = (SocketClientSensorDataReq*)p_req_payload;

                        slim_EnableSensorDataTxnStructType z_Txn;
                        memset(&z_Txn, 0, sizeof(z_Txn));

                        z_Txn.z_TxnData.p_Handle = request->msgPayload.z_TxnData.p_Handle;
                        z_Txn.z_TxnData.u_ClientTxnId = request->msgPayload.z_TxnData.u_ClientTxnId;

                        z_Txn.z_Request.enableConf.providerFlags = request->msgPayload.z_Request.enableConf.providerFlags;
                        z_Txn.z_Request.sensor = request->msgPayload.z_Request.sensor;
                        z_Txn.z_Request.enableConf.enable = request->msgPayload.z_Request.enableConf.enable;
                        z_Txn.z_Request.sampleCount = request->msgPayload.z_Request.sampleCount;
                        z_Txn.z_Request.reportRate = request->msgPayload.z_Request.reportRate;


                        LOC_LOGD("%s:%d] Received client sensor data request, p_Handle is %08X", __func__, __LINE__, z_Txn.z_TxnData.p_Handle);
                        ClientListener::processSensorDataRequest(z_Txn);

                        break;
                    }
                    case eSLIM_SOCKET_CLIENT_MSG_ID_OPEN_REQ:
                    {
                        LOC_LOGD("%s:%d] Received client open request, msg_id is %d", __func__, __LINE__, msg_header.msgId);

                        SocketClientOpenReq* request = (SocketClientOpenReq*)p_req_payload;

                        LOC_LOGD("%s:%d] sizeof(slim_OpenTxnStructType) is %d, sizeof(p_req_payload) is %d",
                                 __func__, __LINE__, sizeof(slim_OpenTxnStructType), sizeof(p_req_payload));

                        slim_OpenTxnStructType z_Txn;
                        memset(&z_Txn, 0, sizeof(z_Txn));

                        z_Txn.p_Handle = request->msgPayload.p_Handle;
                        z_Txn.fn_Callback = SocketClientListener::forwardCallbackData;

                        z_Txn.q_OpenFlags = request->msgPayload.q_OpenFlags;
                        z_Txn.t_CallbackData = request->msgPayload.t_CallbackData;
                        LOC_LOGD("%s:%d] req_payload->p_Handle is %08X", __func__, __LINE__, request->msgPayload.p_Handle);
                        LOC_LOGD("%s:%d] Received client open request, p_Handle is %08X", __func__, __LINE__, z_Txn.p_Handle);
                        LOC_LOGD("%s:%d] Received fn_Callback is %08X", __func__, __LINE__, __func__, __LINE__, request->msgPayload.fn_Callback);

                        ClientListener::processClientRegister(z_Txn);
                        break;
                    }
                    case eSLIM_SOCKET_CLIENT_MSG_ID_CLOSE_REQ:
                    {
                        LOC_LOGD("%s:%d] Received client close request, msg_id is %d", __func__, __LINE__, msg_header.msgId);

                        slim_GenericTxnStructType* req_payload =
                            (slim_GenericTxnStructType*)p_req_payload;

                        slim_GenericTxnStructType z_Txn;
                        memset(&z_Txn, 0, sizeof(z_Txn));

                        z_Txn.e_Service = req_payload->e_Service;
                        z_Txn.e_Type = req_payload->e_Type;
                        z_Txn.z_TxnData.p_Handle = req_payload->z_TxnData.p_Handle;

                         ClientListener::processClientDeRegister(z_Txn);
                        break;
                    }
                    default:
                        LOC_LOGV("%s:%d] Received invalid request, msg_id is %d", __func__, __LINE__, msg_header.msgId);
                        break;
                    }
                    // }
                // else
                // {
                //     LOC_LOGE("Invalid payload");
                // }
                free(p_req_payload);
            }
            else if(len == 0)
            {
                LOC_LOGI("%s:%d] Closing client-connection", __func__,__LINE__);
                close(mClientFd);
                mClientFd = -1;
                break;
            }
            else
            {
                LOC_LOGE("%s:%d] Error: empty request client", __func__,__LINE__);
            }
        } /* while(1) loop for reading client-data from client-socket */
    } /* while(1) loop for waiting for new clients, and accepting connections */

    return socket_fd;
}

/**
@brief Static function to call member function

Function is created as static so that it can be passed to pthread_create
as function pointer. Calls the member function thread to gain access to
member functions

@param This  Singleton class object
*/
void* SocketClientListener::ListenerThreadEntry(void* This)
{
    ((SocketClientListener*)This)->SocketClientListenerThread();
    return NULL;
}

/**
@brief Function that handles callback data

Function is a callback called from SLIM CORE with indication/
response information. In turn calls the non-static member function of
the class to route the handling. Needs to be static to adhere to the callback
C style function signature defined by slim_api.h

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
void SocketClientListener::forwardCallbackData(uint64_t t_CallbackData,
                                               const slimMessageHeaderStructT *pz_MessageHeader,
                                               void *p_Message)
{
    ENTRY_LOG();
    if(SocketClientListener::mMe != NULL)

    {
        (SocketClientListener::mMe)->callbackHandler(t_CallbackData, pz_MessageHeader, p_Message);

    }

}

};//namespace
