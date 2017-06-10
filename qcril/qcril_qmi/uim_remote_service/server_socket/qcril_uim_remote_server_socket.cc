/******************************************************************************
  @file    qcril_uim_remote_server_socket.c
  @brief   qcril - uim remote card socket

  DESCRIPTION
    Handles uim remote card socket related functions

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#define __STDC_FORMAT_MACROS 1
#include <pthread.h>
#include "qcril_uim_remote_server_socket.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <pb_decode.h>

extern "C" {
    #include "qcril_log.h"
    #include "qcrili.h"
    #include "qcril_uim_remote_server_packing.h"
    #include "qcril_uim_remote_server_msg_meta.h"
    #include "qcril_reqlist.h"
    #include "qcril_uim_sap.h"
}

#define QCRIL_UIM_REMOTE_SERVER_SOCKET_PATH "/dev/socket/qmux_radio/uim_remote_server_socket"

class qcril_uim_remote_server_socket_agent
{
public:
    static qcril_uim_remote_server_socket_agent* get_instance();
    int init();
    boolean is_socket_server_started() { return (listen_sid != 0); }
    boolean is_socket_connected()  { return (conn_sid >= 0); }
    void close_socket_connection();
    RIL_RadioFunctions* qcril_uim_functions;
    boolean qcril_uim_remote_server_is_inited();
    qcril_uim_remote_server_socket_agent();

private:
    boolean init_socket_listenfd();
    void recv_thread_handler();
    boolean process_incoming_message();
    int get_message_size();
    void thread_func();
    static void* thread_func_wrapper(void* this_ptr);
    boolean send_message(uint32_t token,
                         MsgType type,
                         MsgId message_id,
                         Error error,
                         const void* msg, int msg_len);
    static void qcril_uim_remote_server_OnRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen);
    static void qcril_uim_remote_server_OnUnsolicitedResponse(int unsolResponse, const void *data, size_t datalen);
    static void qcril_uim_remote_server_requestTimedCallback(RIL_TimedCallback callback, void* param, const struct timeval *relativeTime);
    static struct RIL_Env uimRilEnv;

private:
    static qcril_uim_remote_server_socket_agent* instance;
    pthread_t thread_id;
    int token;
    char thread_name_str[QMI_RIL_THREAD_NAME_MAX_SIZE];
    int listen_sid;
    int conn_sid;
    static const int MSG_OFFSET = 4;
    int inited;
    uint8_t recv_buffer[QCRIL_UIM_REMOTE_SERVER_SOCKET_MAX_BUF_SIZE];
    int recvd_byte_num;
    uint8_t send_buffer[QCRIL_UIM_REMOTE_SERVER_SOCKET_MAX_BUF_SIZE];
};
qcril_uim_remote_server_socket_agent* qcril_uim_remote_server_socket_agent::instance = NULL;

//===========================================================================
// qcril_uim_remote_server_socket_agent::uimRilEnv
//===========================================================================
struct RIL_Env qcril_uim_remote_server_socket_agent::uimRilEnv =
{
    .OnRequestComplete = qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_OnRequestComplete,
    .OnUnsolicitedResponse = qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_OnUnsolicitedResponse,
    .RequestTimedCallback = NULL
};

//================================================================================
// qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_OnRequestComplete
//================================================================================
void qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_OnRequestComplete(RIL_Token t,
                                                                                RIL_Errno e,
                                                                                void *response,
                                                                                size_t responselen)
{
    if(get_instance()->inited)
    {
        MsgHeader *header = (MsgHeader *)t;
        if(header)
        {
            boolean ret = get_instance()->send_message(header->token,
                                       MsgType_RESPONSE,
                                       header->id,
                                       qcril_uim_remote_server_convert_ril_error_to_uim_error(e),
                                       response, (int)responselen);

            qcril_qmi_npb_release(MsgHeader_fields, header);
            qcril_free(header);
            if(ret)
            {
                QCRIL_LOG_ERROR("unable to send the response");
            }
        }
        else
        {
            QCRIL_LOG_ERROR("no token header!");
        }
    }
    else
    {
        QCRIL_LOG_ERROR("Socket not inited");
    }
}

//====================================================================================
// qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_OnUnsolicitedResponse
//====================================================================================
void qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_OnUnsolicitedResponse(int unsolResponse,
                                                                                    const void *data,
                                                                                    size_t datalen)
{
    if(get_instance()->inited)
    {
        boolean ret = get_instance()->send_message(0,
                                   MsgType_UNSOL_RESPONSE,
                                   MsgId(unsolResponse),
                                   Error_RIL_E_SUCCESS,
                                   data, datalen);
        if(ret)
        {
            QCRIL_LOG_ERROR("unable to send the unsol response");
        }
    }
    else
    {
        QCRIL_LOG_ERROR("Socket not inited");
    }
}

//====================================================================================
// qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_is_inited
//====================================================================================
boolean qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_is_inited()
{
    return inited;
}
//===========================================================================
// qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_socket_agent
//===========================================================================
qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_socket_agent() : inited(0),
   listen_sid(0),
   conn_sid(0),
   recvd_byte_num(0),
   token(0)
{
    strlcpy(thread_name_str, QMI_RIL_UIM_REMOTE_SERVER_SOCKET_THREAD_NAME, sizeof(thread_name_str));
} // qcril_uim_remote_server_socket_agent::qcril_uim_remote_server_socket_agent

//===========================================================================
// qcril_uim_remote_server_socket_agent::get instance
//===========================================================================
qcril_uim_remote_server_socket_agent* qcril_uim_remote_server_socket_agent::get_instance()
{
    if(NULL == instance)
    {
        instance = new qcril_uim_remote_server_socket_agent();
    }
    return instance;
} // qcril_uim_remote_server_socket_agent::get instance

//===========================================================================
// qcril_uim_remote_server_socket_agent::init
//===========================================================================
int qcril_uim_remote_server_socket_agent::init()
{
    int ret = 0;
    int create_thread = 0;
    pthread_attr_t attr;
    if (!inited)
    {
       QCRIL_LOG_INFO("init agent..");
       pthread_attr_init (&attr);
       pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
       if ( 0 == pthread_create(&thread_id, &attr, qcril_uim_remote_server_socket_agent::thread_func_wrapper, this) )
       {
          if (strlen(thread_name_str))
          {
             qmi_ril_set_thread_name(thread_id, thread_name_str);
          }
       }
       else
       {
          QCRIL_LOG_ERROR("Error in pthread_create");
          create_thread = -1;
       }
       pthread_attr_destroy(&attr);

      if (0 == create_thread)
      {
         QCRIL_LOG_INFO("Created the Socket thread, calling ril_init_sap");
         //TODO: for multisim, need to create argv from rild instance
         char **argv = NULL;
         qcril_uim_functions = const_cast<RIL_RadioFunctions*>(RIL_SAP_Init(&uimRilEnv, 0, argv));
         if(qcril_uim_functions)
         {
             inited = TRUE;
         }
      }
      else
      {
         QCRIL_LOG_DEBUG("agent is already inited before..");
         ret = -1;
      }
   }
   QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
   return ret;
} // qcril_uim_remote_server_socket_agent::init

//===========================================================================
// qcril_uim_remote_server_socket_agent::thread_func
//===========================================================================
void qcril_uim_remote_server_socket_agent::thread_func()
{
    QCRIL_LOG_FUNC_ENTRY();

    if (listen_sid == 0)
    {
        if (!init_socket_listenfd())
        {
            recv_thread_handler();
        }
        else
        {
            listen_sid = 0;
        }
    }
    else
    {
        QCRIL_LOG_DEBUG("socket already initialized.");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_uim_remote_server_socket_agent::thread_func

//===========================================================================
// qcril_uim_remote_server_socket_agent::thread_func_wrapper
//===========================================================================
void* qcril_uim_remote_server_socket_agent::thread_func_wrapper(void* this_ptr)
{
    ((qcril_uim_remote_server_socket_agent *) this_ptr)->thread_func();
    return NULL;
}

//===========================================================================
// qcril_uim_remote_server_socket_agent::send_message
//===========================================================================
boolean qcril_uim_remote_server_socket_agent::send_message(uint32_t token,
                                                           MsgType type,
                                                           MsgId message_id,
                                                           Error error,
                                                           const void* msg, int msg_len)
{
    boolean ret = 0;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_LOG_INFO("type: %d, message_id: %d, error: %d", type, message_id, error);

    if (is_socket_connected())
    {
        size_t msg_size = 0;

        msg_size = qcril_uim_remote_server_pack_msg_tag(msg, msg_len,
                                                        token,
                                                        type, message_id,
                                                        error, send_buffer+4,
                                                        sizeof(send_buffer)-4);

        QCRIL_LOG_INFO("msg_size: %d",msg_size);

        unsigned char *tmp = (unsigned char*) send_buffer;
        tmp[0] = (unsigned char)((msg_size) >> 24);
        tmp[1] = (unsigned char)(((msg_size) >> 16 ) & 0xff);
        tmp[2] = (unsigned char)(((msg_size) >> 8 ) & 0xff);
        tmp[3] = (unsigned char)((msg_size) & 0xff);

        qcril_qmi_print_hex(send_buffer, msg_size+4);
        QCRIL_LOG_INFO("Sending response");
        send(conn_sid, send_buffer, msg_size+4, 0);
    }
    else
    {
        QCRIL_LOG_INFO("socket is not connected");
        ret = 1;
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET((int) ret);
    return ret;
} // qcril_uim_remote_server_socket_agent::send_message

//===========================================================================
// qcril_uim_remote_server_socket_agent::get_message_size
//===========================================================================
int qcril_uim_remote_server_socket_agent::get_message_size()
{
    int size = -1;
    if (recvd_byte_num >= MSG_OFFSET)
    {
        size = 0;
        // message size will be in 4 bytes(MSG_OFFSET) big-endian format
        for (int i=0; i<MSG_OFFSET; i++)
        {
            size = (size << 8) + recv_buffer[i];
        }
    }
    QCRIL_LOG_FUNC_RETURN_WITH_RET(size);
    return size;
} // qcril_uim_remote_server_socket_agent::get_message_size

//===========================================================================
// qcril_uim_remote_server_socket_agent::process_incoming_message
//===========================================================================
boolean qcril_uim_remote_server_socket_agent::process_incoming_message()
{
    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_LOG_INFO("received %d bytes", recvd_byte_num);
    qcril_qmi_print_hex(recv_buffer, recvd_byte_num);

    boolean err = FALSE;
    int size = get_message_size();

    do
    {
        if (size < 0)
        {
            err = TRUE;
            break;
        }

        if (recvd_byte_num < size+MSG_OFFSET )
        {
            err = TRUE;
            break;
        }

        MsgHeader *msg_tag_ptr = NULL;

        qcril_evt_e_type uim_req;
        QCRIL_LOG_INFO("unpacking Message tag");
        msg_tag_ptr = qcril_uim_remote_server_unpack_msg_tag(recv_buffer+MSG_OFFSET, size);

        if(msg_tag_ptr)
        {
            QCRIL_LOG_INFO( "msg: %s, type: %d, message_id: %d, error: %d",
                            qcril_uim_remote_server_get_msg_log_str(msg_tag_ptr->id, msg_tag_ptr->type),
                            msg_tag_ptr->type,
                            msg_tag_ptr->id,
                            msg_tag_ptr->error);

            qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                               QCRIL_DEFAULT_MODEM_ID,
                               QCRIL_DATA_NOT_ON_STACK,
                               (qcril_evt_e_type) qcril_uim_remote_server_get_msg_event(msg_tag_ptr->id, msg_tag_ptr->type),
                               msg_tag_ptr,
                               sizeof(*msg_tag_ptr),
                               qcril_uim_remote_server_convert_uim_token_to_ril_token(msg_tag_ptr->token) );
        }
    } while (FALSE);
    QCRIL_LOG_FUNC_RETURN();
    return err;
} // qcril_uim_remote_server_socket_agent::process_incoming_message

//===========================================================================
// qcril_uim_remote_server_socket_agent::init_socket_listenfd
//===========================================================================
boolean qcril_uim_remote_server_socket_agent::init_socket_listenfd()
{
    QCRIL_LOG_FUNC_ENTRY();

    struct sockaddr_un servaddr;
    /* creating a socket */
    if ((listen_sid = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
       QCRIL_LOG_ERROR("Error in socket");
       return 1;
    }

    /* configuring server address structure */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family      = AF_UNIX;
    snprintf(servaddr.sun_path, sizeof(servaddr.sun_path), "%s", QCRIL_UIM_REMOTE_SERVER_SOCKET_PATH);
    unlink(servaddr.sun_path);

    QCRIL_LOG_INFO("bind to socket path %s", servaddr.sun_path);
    /* binding our socket to the service port */
    if (bind(listen_sid, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
       QCRIL_LOG_ERROR("Error in bind");
       return 1;
    }

    QCRIL_LOG_INFO("listen ...");
    /* convert our socket to a listening socket */
    if (listen(listen_sid, 0) < 0) //just one client for the server
    {
       QCRIL_LOG_ERROR("Error in listen");
       return 1;
    }
    QCRIL_LOG_INFO("listen socket init finished");

    return 0;
} // qcril_uim_remote_server_socket_agent::init_socket_listenfd

//===========================================================================
// qcril_uim_remote_server_socket_agent::recv_thread_handler
//===========================================================================
void qcril_uim_remote_server_socket_agent::recv_thread_handler()
{
    QCRIL_LOG_FUNC_ENTRY();
    struct sockaddr_un cliaddr;
    socklen_t          clilen = sizeof(cliaddr);
    int newly_recvd_byte_num;
    int msg_size;

    do
    {
        if ((conn_sid = accept(listen_sid, (struct sockaddr*) &cliaddr, &clilen)) < 0)
        {
            // TODO: ERROR handling
            QCRIL_LOG_ERROR("accept failed. conn_sid: %d", conn_sid);
        }
        else
        {
            QCRIL_LOG_INFO("client connected with conn_sid %d", conn_sid);
            recvd_byte_num = 0;

            while (TRUE)
            {
                newly_recvd_byte_num = recv( conn_sid,
                                             recv_buffer + recvd_byte_num,
                                             sizeof(recv_buffer) - recvd_byte_num,
                                             0 );


                if (newly_recvd_byte_num <= 0)
                {
                    QCRIL_LOG_ERROR("recv failed. recv_byte_num: %d. closing the socket.", recvd_byte_num);
                    close(conn_sid);
                    break;
                }
                else
                {
                    recvd_byte_num += newly_recvd_byte_num;

                    while (!process_incoming_message())
                    {
                        msg_size = get_message_size();
                        recvd_byte_num = recvd_byte_num - msg_size - MSG_OFFSET;
                        if (recvd_byte_num > 0)
                        {
                            memcpy(recv_buffer, recv_buffer + msg_size + MSG_OFFSET, recvd_byte_num);
                        }
                    }
                }
            }
        }
    } while (TRUE);
    QCRIL_LOG_FUNC_RETURN();
} // qcril_uim_remote_server_socket_agent::recv_thread_func

extern "C" {

//===========================================================================
// qcril_uim_remote_server_socket_init
//===========================================================================
void qcril_uim_remote_server_socket_init()
{
    QCRIL_LOG_FUNC_ENTRY();

    if(qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID)
    {
        qcril_uim_remote_server_socket_agent::get_instance()->init();
        QCRIL_LOG_INFO("..Create UIM thread on SUB%d with name %s", qmi_ril_get_process_instance_id());
    }
    else
    {
        QCRIL_LOG_INFO("uim remote server socket cannot be started on other rild instances except default");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_uim_remote_server_socket_init

//===========================================================================
// qcril_uim_remote_server_socket_dispatch_request
//===========================================================================
void qcril_uim_remote_server_socket_dispatch_request(const qcril_request_params_type *const params_ptr,
                                                    qcril_request_return_type *const ret_ptr)
{
    QCRIL_LOG_FUNC_ENTRY();
    if(qcril_uim_remote_server_socket_agent::get_instance()->qcril_uim_remote_server_is_inited())
    {
        MsgHeader* header = NULL;
        if(params_ptr->data == NULL)
        {
            QCRIL_LOG_ERROR("..invalid params");
        }
        else
        {
            header = (MsgHeader*) params_ptr->data;
            QCRIL_LOG_INFO("dispatching request for msg: %s (id: %d, type: %d)",
                            qcril_uim_remote_server_get_msg_log_str(header->id, header->type),
                            header->id, header->type);
            qcril_binary_data_type *bin_data = (qcril_binary_data_type*)header->payload.arg;
            qcril_uim_remote_server_socket_agent::get_instance()->qcril_uim_functions->onRequest(header->id, bin_data->data, bin_data->len, header);
            QCRIL_LOG_INFO("dispatched request");
        }
    }
    else
    {
        QCRIL_LOG_INFO("socket and the sap init not initialized");

    }
    QCRIL_LOG_FUNC_RETURN();
}

} // end of extern "C"


