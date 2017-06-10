/******************************************************************************
  @file    qcril_qmi_ims_socket.c
  @brief   qcril qmi - ims socket

  DESCRIPTION
    Handles ims socket related functions

  ---------------------------------------------------------------------------

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#define __STDC_FORMAT_MACROS 1
#include <pthread.h>
#include "qcril_qmi_ims_socket.h"
#include <sys/socket.h>
#include <sys/un.h>

#ifdef QMI_RIL_UTF
#include "unistd.h"
#include "netinet/ip.h"
extern "C" uint32 qcril_get_time_milliseconds();
#endif

#if defined(FEATURE_TARGET_GLIBC_x86)  || defined(QMI_RIL_UTF)
    extern "C" size_t strlcat(char *, const char *, size_t);
    extern "C" size_t strlcpy(char *, const char *, size_t);
#endif

extern "C" {
    #include "qcril_log.h"
    #include "qcrili.h"
    #include "qcril_qmi_ims_packing.h"
    #include "qcril_qmi_ims_misc.h"
    #include "qcril_reqlist.h"
    #include "hlos_csvt_core.h"
    #include "qcril_qmi_ims_flow_control.h"
}

#include "qcril_qmi_singleton_agent.h"

#ifdef QMI_RIL_UTF
#define QCRIL_QMI_IMS_SOCKET_PATH "./rild_ims"
#else
#define QCRIL_QMI_IMS_SOCKET_PATH "/dev/socket/qmux_radio/rild_ims"
#endif

#define QCRIL_QMI_IMS_SOCKET_MAX_PENDING_CONNS 5

class qcril_qmi_ims_socket_agent : public qcril_qmi_singleton_agent<qcril_qmi_ims_socket_agent>
{
public:
    boolean is_socket_server_started() { return (listen_sid != 0); }
    boolean is_socket_connected()  { return (conn_sid >= 0); }
    boolean send_message(RIL_Token token, Ims__MsgType type, Ims__MsgId message_id, Ims__Error error, const void* msg, int msg_len);
    void close_socket_connection();

    qcril_qmi_ims_socket_agent();

private:
    boolean init_socket_listenfd();
    void recv_thread_handler();
    boolean process_incoming_message();
    int get_message_size();
    void thread_func();
#ifdef QMI_RIL_UTF
    void ims_state_reset();
#endif

private: // data member
    pthread_t thread_id;
    int listen_sid;
    int conn_sid;
    static const int MSG_OFFSET = 4;

    uint8_t recv_buffer[QCRIL_QMI_IMS_SOCKET_MAX_BUF_SIZE];
    int recvd_byte_num;
    uint8_t send_buffer[QCRIL_QMI_IMS_SOCKET_MAX_BUF_SIZE];
};

//===========================================================================
// qcril_qmi_ims_socket_agent::qcril_qmi_ims_socket_agent
//===========================================================================
qcril_qmi_ims_socket_agent::qcril_qmi_ims_socket_agent() :
   qcril_qmi_singleton_agent<qcril_qmi_ims_socket_agent>(QMI_RIL_IMS_SOCKET_THREAD_NAME),
   listen_sid(0),
   conn_sid(0),
   recvd_byte_num(0)
{} // qcril_qmi_ims_socket_agent::qcril_qmi_ims_socket_agent

//===========================================================================
// qcril_qmi_ims_socket_agent::thread_func
//===========================================================================
void qcril_qmi_ims_socket_agent::thread_func()
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
} // qcril_qmi_ims_socket_agent::thread_func

#ifdef QMI_RIL_UTF
//===========================================================================
// qcril_qmi_ims_socket_agent::ims_state_reset
//===========================================================================
void qcril_qmi_ims_socket_agent::ims_state_reset()
{
  listen_sid = 0;
  reset_state_reboot();
}
#endif

//===========================================================================
// qcril_qmi_ims_socket_agent::send_message
//===========================================================================
boolean qcril_qmi_ims_socket_agent::send_message(RIL_Token token, Ims__MsgType type, Ims__MsgId message_id, Ims__Error error, const void* msg, int msg_len)
{
    boolean ret = 0;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_LOG_INFO("type: %d, message_id: %d, error: %d", type, message_id, error);

    if (is_socket_connected())
    {
        size_t tag_size = 0;
        size_t msg_size = 0;

        tag_size = qcril_qmi_ims_pack_msg_tag(qcril_qmi_ims_free_and_convert_ril_token_to_ims_token(token), type, message_id, error, send_buffer+4, sizeof(send_buffer));
        msg_size = qcril_qmi_ims_pack_msg((void*) msg,type,message_id, &(send_buffer[tag_size+4]), sizeof(send_buffer)-tag_size);
        QCRIL_LOG_INFO("tag_size: %d, msg_szie: %d", tag_size, msg_size);

        unsigned char *tmp = (unsigned char*) send_buffer;
        tmp[0] = (unsigned char)((tag_size+msg_size) >> 24);
        tmp[1] = (unsigned char)(((tag_size+msg_size) >> 16 ) & 0xff);
        tmp[2] = (unsigned char)(((tag_size+msg_size) >> 8 ) & 0xff);
        tmp[3] = (unsigned char)((tag_size+msg_size) & 0xff);

        qcril_qmi_print_hex(send_buffer, tag_size+msg_size+4);

        send(conn_sid, send_buffer, tag_size+msg_size+4, 0);
    }
    else
    {
        QCRIL_LOG_INFO("socket is not connected");
        ret = 1;
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET((int) ret);
    return ret;
} // qcril_qmi_ims_socket_agent::send_message

//===========================================================================
// qcril_qmi_ims_socket_agent::get_message_size
//===========================================================================
int qcril_qmi_ims_socket_agent::get_message_size()
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
} // qcril_qmi_ims_socket_agent::get_message_size

//===========================================================================
// qcril_qmi_ims_socket_agent::process_incoming_message
//===========================================================================
boolean qcril_qmi_ims_socket_agent::process_incoming_message()
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

        if (recvd_byte_num < size + MSG_OFFSET)
        {
            err = TRUE;
            break;
        }

        size_t tag_size = recv_buffer[MSG_OFFSET];
        size_t packed_msg_size = size - 1 - tag_size;
        size_t unpacked_msg_size = 0;

        Ims__MsgTag *msg_tag_ptr = NULL;
        void *msg_data_ptr = NULL;

        qcril_evt_e_type ims_req;

        msg_tag_ptr = qcril_qmi_ims_unpack_msg_tag(recv_buffer+MSG_OFFSET);

        qcril_qmi_ims_parse_packed_msg(msg_tag_ptr->type, msg_tag_ptr->id, recv_buffer+tag_size+1+MSG_OFFSET, packed_msg_size, &msg_data_ptr, &unpacked_msg_size, &ims_req);

        QCRIL_LOG_INFO("token: %d, type: %d, message_id: %d, error: %d", msg_tag_ptr->token, msg_tag_ptr->type, msg_tag_ptr->id, msg_tag_ptr->error);

        if(FALSE == hlos_csvt_request_handler(msg_tag_ptr->id, msg_tag_ptr->token, msg_data_ptr, unpacked_msg_size))
        {
            if (QCRIL_EVT_IMS_SOCKET_REQ_DIAL == ims_req)
            {
                qcril_qmi_voice_process_for_ims_dial(msg_data_ptr,
                                                     unpacked_msg_size,
                                                     qcril_qmi_ims_convert_ims_token_to_ril_token(msg_tag_ptr->token));
            }
            else
            {
                qcril_qmi_ims_flow_control_event_queue(QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_REQ,
                                                       QCRIL_DATA_NOT_ON_STACK,
                                                       (qcril_evt_e_type)ims_req,
                                                       msg_data_ptr, unpacked_msg_size,
                                                       qcril_qmi_ims_convert_ims_token_to_ril_token(msg_tag_ptr->token) );
            }
        }

        qcril_qmi_ims__msg_tag__free_unpacked(msg_tag_ptr, NULL);

    } while (FALSE);
    QCRIL_LOG_FUNC_RETURN();
    return err;
} // qcril_qmi_ims_socket_agent::process_incoming_message

//===========================================================================
// qcril_qmi_ims_socket_agent::init_socket_listenfd
//===========================================================================
boolean qcril_qmi_ims_socket_agent::init_socket_listenfd()
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
    snprintf(servaddr.sun_path, sizeof(servaddr.sun_path), "%s%d", QCRIL_QMI_IMS_SOCKET_PATH, qmi_ril_get_process_instance_id());
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
    if (listen(listen_sid, QCRIL_QMI_IMS_SOCKET_MAX_PENDING_CONNS) < 0)
    {
       QCRIL_LOG_ERROR("Error in listen");
       return 1;
    }
    QCRIL_LOG_INFO("listen socket init finished");

    return 0;
} // qcril_qmi_ims_socket_agent::init_socket_listenfd

//===========================================================================
// qcril_qmi_ims_socket_agent::recv_thread_handler
//===========================================================================
void qcril_qmi_ims_socket_agent::recv_thread_handler()
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

            qcril_qmi_voice_ims_send_unsol_radio_state_change_helper();

            while (TRUE)
            {
                newly_recvd_byte_num = recv( conn_sid,
                                             recv_buffer + recvd_byte_num,
                                             sizeof(recv_buffer) - recvd_byte_num,
                                             0 );

#ifdef QMI_RIL_UTF
                if ( newly_recvd_byte_num == 6 )
                {
                  if (strncmp((char*)recv_buffer, "reset", 6) == 0)
                  {
                    close(listen_sid);
                    close(conn_sid);
                    ims_state_reset();
                    pthread_exit(NULL);
                  }
                }
#endif


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
} // qcril_qmi_ims_socket_agent::recv_thread_func

extern "C" {

//===========================================================================
// qcril_qmi_ims_socket_init
//===========================================================================
void qcril_qmi_ims_socket_init()
{
    QCRIL_LOG_FUNC_ENTRY();
    char *threadName = (char *)qcril_malloc(QCRIL_QMI_IMS_SOCKET_MAX_THREAD_NAME_SIZE);

    if(threadName)
    {
      snprintf(threadName, QCRIL_QMI_IMS_SOCKET_MAX_THREAD_NAME_SIZE, "%s%d", QMI_RIL_IMS_SOCKET_THREAD_NAME, qmi_ril_get_process_instance_id());
      QCRIL_LOG_INFO("..Create IMS thread on SUB%d with name %s", qmi_ril_get_process_instance_id(), threadName);

      qcril_qmi_ims_socket_agent::get_instance()->init(threadName);
      qcril_free(threadName);
    }
    else
    {
      QCRIL_LOG_ERROR("..Failed to allocate memory");
    }
    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_ims_socket_init

//===========================================================================
// qcril_qmi_ims_socket_send_flow_control
//===========================================================================
void qcril_qmi_ims_socket_send_flow_control(RIL_Token token, Ims__MsgType type,
                                            Ims__MsgId message_id,
                                            Ims__Error error,
                                            const void* msg, int msg_len)
{
    qcril_reqlist_public_type request_info;

    QCRIL_LOG_FUNC_ENTRY();
    if ( E_SUCCESS == qcril_reqlist_query(QCRIL_DEFAULT_INSTANCE_ID, token, &request_info) ||
         ( IMS__MSG_ID__REQUEST_DIAL != message_id &&
           IMS__MSG_ID__REQUEST_ANSWER != message_id &&
           IMS__MSG_ID__REQUEST_HANGUP_WAITING_OR_BACKGROUND != message_id
         )
       )
    {
        qcril_reqlist_free( QCRIL_DEFAULT_INSTANCE_ID , token );
        qcril_qmi_ims_socket_agent::get_instance()->send_message(token,type,message_id,error,msg,msg_len);
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_ims_socket_send_flow_control

//===========================================================================
// qcril_qmi_ims_socket_send
//===========================================================================
void qcril_qmi_ims_socket_send(RIL_Token token, Ims__MsgType type, Ims__MsgId message_id, Ims__Error error, const void* msg, int msg_len)
{
    QCRIL_LOG_FUNC_ENTRY();

    qcril_reqlist_free( QCRIL_DEFAULT_INSTANCE_ID , token );
    qcril_qmi_ims_socket_agent::get_instance()->send_message(token,type,message_id,error,msg,msg_len);

    if(type == IMS__MSG_TYPE__RESPONSE)
    {
        QCRIL_LOG_INFO("Check for any pending request in flow control %d", message_id);
        qcril_qmi_ims_flow_control_event_queue(QCRIL_QMI_IMS_FLOW_CONTROL_REQ_COMPLETE,
                                               QCRIL_DATA_NOT_ON_STACK,
                                               qcril_qmi_ims_map_request_to_event(message_id),
                                               NULL, 0, token);
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_ims_socket_send

//===========================================================================
// qcril_qmi_ims_socket_send_empty_payload_unsol_resp
//===========================================================================
void qcril_qmi_ims_socket_send_empty_payload_unsol_resp(Ims__MsgId msg_id)
{
    qcril_qmi_ims_socket_send(0, IMS__MSG_TYPE__UNSOL_RESPONSE, msg_id, IMS__ERROR__E_SUCCESS, NULL, 0);
} // qcril_qmi_ims_socket_send_empty_payload_unsol_resp

} // end of extern "C"
