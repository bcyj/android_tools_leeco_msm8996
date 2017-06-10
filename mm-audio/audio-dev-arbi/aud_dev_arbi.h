#ifndef _AUD_DEV_ARBI_H_
#define _AUD_DEV_ARBI_H_

/*============================================================================
                           aud_dev_arbi.h

DESCRIPTION:  This file contains the declarations for audio device
              arbitration.

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/

#include <sys/un.h>
#include <semaphore.h>
#include <pthread.h>
#include "aud_dev_arbi_client_if.h"

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
    IPC messages
*/
typedef enum {
    AUD_DEV_ARBI_IPC_MSG_REGISTER_FOR_DEVICE = 0,
    AUD_DEV_ARBI_IPC_MSG_DEVICE_REQUESTED,
    AUD_DEV_ARBI_IPC_MSG_DEVICE_RELEASED,
    AUD_DEV_ARBI_IPC_MSG_ACK,
    AUD_DEV_ARBI_IPC_MSG_NACK,
    AUD_DEV_ARBI_NUM_IPC_MSG
} aud_dev_arbi_ipc_msg_t;

/**
    Device owners
*/
typedef enum {
    AUD_DEV_ARBI_DEVICE_OWNER_NONE = 0,
    AUD_DEV_ARBI_DEVICE_OWNER_SERVER,
    AUD_DEV_ARBI_DEVICE_OWNER_CLIENT,
    AUD_DEV_ARBI_NUM_OWNERS
} aud_dev_arbi_device_owner_t;

/**
    Message definition
*/
typedef struct {
    aud_dev_arbi_ipc_msg_t msgId;
    aud_dev_arbi_client_t clientId;
    audio_devices_t deviceId;
} aud_dev_arbi_msg;

/**
    IPC endpoint handler interface. Must be implemented by
    users of IpcEndpoint to provide means to run the socket main
    loop and handle incoming commands.
 */
class IIpcEndpointHandler
{
    public:

        /**
        * Main socket thread loop.
        *
        * @return 0 on success, negative value otherwise
        */
        virtual int runSocketThread() = 0;

        /**
        * Incoming command handler.
        *
        * @param msg[in]  the incoming command message
        * @return event to be sent as response
        */
        virtual aud_dev_arbi_ipc_msg_t handleIncomingCommand(aud_dev_arbi_msg msg) = 0;

        /**
        * D'tor
        */
        virtual ~IIpcEndpointHandler();
};

/**
    Static helper class to initialize a socket and send/receive
    buffers over it.
 */
class Socket
{
    public:
        /**
        * Creates a unix domain socket.
        *
        * @param sockPath[in]  the socket file system path
        * @param sockAddr[out] a socket address structure to be
        *                 initialized
        * @return 0 on success, negative value on failure
        */
        static int init(const char *sockPath, struct sockaddr_un* sockAddr);

        /**
        * Sends a buffer over a socket.
        *
        * @param fd[in]  the socket file descriptor
        * @param buf[in] the buffer to be sent
        * @param len[in] the size of the buffer, in bytes
        * @return 0 on success, negative value on failure
        */
        static int sendBuf(int fd, void* buf, int len);

        /**
        * Receives data into a buffer from a socket. The transfer can be
        * aborted by writing to an optional event_fd.
        *
        * @param fd[in]       the socket file descriptor
        * @param abortFd[in] an event_fd which can be used to
        *                     signal transfer abort. 0 to ignore.
        * @param buf[in]      the buffer to filled with data
        * @param len[in]      the length of the buffer, in bytes
        * @param timeout[in]  transfer timeout, in ms
        * @return 0 on success, negative value on failure
        */
        static int receiveBuf(int fd, int abortFd, void* buf, int len, int timeout);
};

/**
    The IPC endpoint provides a synchronous mechanism to send
    commands to another endpoint and to receive events from
    another endpoint. Messages are sent over a socket. The
    endpoint user is reponsible for establishing the connection
    with the other endpoint.
 */
class IpcEndpoint
{
    public:
        /**
        * D'tor
        */
        virtual ~IpcEndpoint();

        /**
        * Initializes the endpoint, creates the main socket thread and
        * calls the client-supplied handler to establish a connection
        * with the other endpoint.
        *
        * @param ipcHandler[in]  client-supplied handler for endpoint
        * @return 0 on success, negative value on failure
        */
        int start(IIpcEndpointHandler* ipcHandler);

        /**
        * Stops the socket threads and de-initializes the endpoint.
        *
        */
        void stop();

        /**
        * Sends an event to the connected endpoint, and waits for a
        * response from it.
        *
        * @param sockFd[in]    file descriptor of socket
        * @param msg[in]        message to send to other endpoint
        * @param response[out]  response received from other endpoint
        * @return 0 on success, negative value on failure
        */
        int notify(int sockFd,
                   aud_dev_arbi_msg msg,
                   aud_dev_arbi_ipc_msg_t* response);

        /**
        * Runs the session handling loop: wait for incoming commands
        * from other endpoint, and respond to client request to send
        * outgoing commands to the other endpoint. This function is to
        * be called by the endpoint client after establishing the
        * connection with the other endpoint.
        *
        * @param sockFd[in]    socket for session
        * @return true if thread should be stopped after returning from
        *         this function, false otherwise
        */
        bool handleSession(int sockFd);

    protected:

        // client IPC endpoint handler
        IIpcEndpointHandler* mIpcEpHandler;

        // semaphore, signaled when a response for an outgoing command
        // is received
        sem_t mCmdDone;

        // response received for the outgoing command
        aud_dev_arbi_ipc_msg_t mCmdResult;

        // socket thread
        int mThreadId;

        // event_fd used to signal aborting the current read
        int mAbortReadFd;

        // indicates whether a command was sent for which a response was
        // yet to be received
        bool mOutgoingCmdPending;

        // flag specifying whether the endpoint should be stopped
        bool mStopEp;

        /**
        * Waits for an incoming event from the other endpoint. When
        * received, handles it according to the current endpoint state.
        *
        * @param sockFd[in]    socket for session
        * @return 0 on success, negative value on failure
        */
        int handleIncomingMessage(int sockFd);

        /**
        * Thread callback function for pthread.
        *
        * @param param[in]    thread context
        * @return always NULL
        */
        static void *threadFuncCb(void* param);
};


/**
    The server side of audio device arbitration. Keeps an
    internal state of the device ownership (server/client/none).
    Provides the user the ability to acquire/release the device
    and sends the proper notifications to the client side if
    needed.
 */
class ServerIpcEndpoint : public IIpcEndpointHandler
{
    public:
        /**
        * D'tor
        */
        virtual ~ServerIpcEndpoint();

        /**
        * Starts the server endpoint. The endpoint will act as a socket
        * server, waiting for an incoming connection from the client.
        *
        * @param sockPath[in]  path of socket to be used for
        *                       communication with client
        * @return 0 on success, negative value on failure
        */
        int start(const char *sockPath);

        /**
        * Stops the endpoint.
        */
        void stop();

        /**
        * Acquires the device. If the device is currently used by the
        * client, a command will be sent to the client requesting the
        * device's release. Execution will block until a response is
        * received from the client (or timeout is reached).
        *
        * @param device[in]  device to acquire
        *
        * @return true if a positive reponse was received from the
        *         client, false otherwise
        */
        bool acquireDevice(audio_devices_t device);

        /**
        * Releases the device (if used by the server) and notifies the
        * client (if client endpoint is connected).
        *
        * @param device[in]  device to release
        *
        * @return true if a positive reponse was received from the
        *         client or the server is not the owner, false otherwise
        */
        bool releaseDevice(audio_devices_t device);

        /**
        * Runs the socket thread: waits for an incoming connection and
        * then launches the endpoint's session handling loop.
        *
        * @return 0 on success, negative value on failure
        */
        virtual int runSocketThread();

        /**
        * Handles incoming commands from the client (acquire/release
        * device). If the client requests to acquire
        * the device while it is owned by the server, the request will
        * be denied. Otherwise, ownership will move to the client.
        *
        * @param msg[in]  the incoming command to be handled
        * @return the reponse for the command to be sent to the client
        */
        virtual aud_dev_arbi_ipc_msg_t handleIncomingCommand(aud_dev_arbi_msg msg);

    protected:

        // underlying endpoint mechanism for sending/receiving commands
        IpcEndpoint mIpcEndpoint;

        // device for which notifications are sent
        audio_devices_t mDevice;

        // current device owner
        aud_dev_arbi_device_owner_t mDeviceOwner;

        // socket address
        struct sockaddr_un mAddress;

        // client id
        aud_dev_arbi_client_t mClient;

        // socket file descriptor
        int mSocketFd;

        // session (connection) file descriptor
        int mClientFd;

        // indication whether endpoint was started
        bool mIsActive;

        // mutex to synchronize handling of incoming/outgoing commands
        pthread_mutex_t mMutex;
};

/**
    The client side of audio device arbitration. Provides the
    user the ability to acquire/release the device and sends the
    proper notifications to the server side. When a device
    release request is received, calls the client-supplied
    callback to handle the specifics of the release sequence.
 */
class ClientIpcEndpoint : public IIpcEndpointHandler
{
    public:
        /**
        * D'tor
        */
        virtual ~ClientIpcEndpoint();

        /**
        * Starts the client endpoint. The endpoint will act as a socket
        * client and will establish a connection with the server
        * endpoint.
        *
        * @param sockPath[in]  path of socket to be used for
        *                       communication with server
        * @param evtCb[in]     function to be called when an event is
        *                       received from the server endpoint
        * @return 0 on success, negative value on failure
        */
        int start(const char *sockPath, aud_dev_arbi_event_cb_t evtCb);

        /**
        * Stops the endpoint.
        */
        void stop();

        /**
        * Registers for notifications about a specific device.
        *
        * @param client[in]  enumeration of the client ID
        * @param device[in]  device to register for
        *
        * @return true if a positive reponse was received from the
        *         server, false otherwise
        */
        bool registerForDevice(aud_dev_arbi_client_t client, audio_devices_t device);

        /**
        * Acquires the device. Sends a command to the server requesting
        * the device and waits for reponse.
        *
        * @param device[in]  device to acquire
        *
        * @return true if a positive reponse was received from the
        *         server, false otherwise
        */
        bool acquireDevice(audio_devices_t device);

        /**
        * Releases the device. Sends a command to the server notifying
        * that the device is not in use by the client and waits for a
        * response.
        *
        * @param device[in]  device to release
        *
        * @return true if a positive reponse was received from the
        *         server, false otherwise
        */
        bool releaseDevice(audio_devices_t device);

        /**
        * Runs the socket thread: connects to the server socket
        * and then launches the endpoint's session handling loop.
        *
        * @return 0 on success, negative value on failure
        */
        virtual int runSocketThread();

        /**
        * Handles incoming commands from the server. The client-supplied
        * function is called upon receiving a command. For a release
        * request, then device is expected to be available when
        * returning from this function.
        *
        * @param msg[in]  the incoming command to be handled
        * @return the reponse for the command to be sent to the server
        */
        virtual aud_dev_arbi_ipc_msg_t handleIncomingCommand(aud_dev_arbi_msg msg);

    protected:

        // underlying endpoint mechanism for sending/receiving commands
        IpcEndpoint mIpcEndpoint;

        // socket address
        struct sockaddr_un mAddress;

        // connection file descriptor
        int mSocketFd;

        // client id
        aud_dev_arbi_client_t mClient;

        // indication whether endpoint was started
        bool mIsActive;

        // the client-supplied event callback function
        aud_dev_arbi_event_cb_t mEvtCb;

        // mutex synchronizing event sending/receiving
        pthread_mutex_t mMutex;
};

#endif // _AUD_DEV_ARBI_H_
