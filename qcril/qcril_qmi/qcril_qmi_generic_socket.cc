/******************************************************************************
  @file    qcril_qmi_generic_socket.c
  @brief   Base class for OEM and IMS socket classes

  DESCRIPTION
    Handles generic socket related functions

  ---------------------------------------------------------------------------

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#define __STDC_FORMAT_MACROS 1
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "qcril_qmi_generic_socket.h"

extern "C" {
    #include "qcril_log.h"
    #include "qcrili.h"
    #include "qcril_reqlist.h"
}

#ifdef QMI_RIL_UTF
#include "unistd.h"
extern "C" uint32 qcril_get_time_milliseconds();
#endif

#if defined(FEATURE_TARGET_GLIBC_x86) || defined(QMI_RIL_UTF)
   extern "C" size_t strlcat(char *, const char *, size_t);
   extern "C" size_t strlcpy(char *, const char *, size_t);
#endif

/*===========================================================================
  FUNCTION  qcril_qmi_generic_socket_agent::qcril_qmi_generic_socket_agent
===========================================================================*/
/*!
    @brief  constructor for qcril_qmi_generic_socket_agent class
*/
/*=========================================================================*/
qcril_qmi_generic_socket_agent::qcril_qmi_generic_socket_agent(int max_conn) :
  listen_sid(0),
  conn_sid(0),
  recv_byte_num(0),
#ifdef QMI_RIL_UTF
  shutdown_request(0),
#endif
  max_connections(max_conn)
{}

qcril_qmi_generic_socket_agent::~qcril_qmi_generic_socket_agent()
{
  return;
}

/*===========================================================================
  FUNCTION  qcril_qmi_generic_socket_agent::start_socket_server
===========================================================================*/
/*!
    @brief Start GENERIC socket server
*/
/*=========================================================================*/
boolean qcril_qmi_generic_socket_agent::start_socket_server
(
  char *threadName
)
{
    QCRIL_LOG_FUNC_ENTRY();

    if (listen_sid == 0)
    {
        if (!init_socket_listenfd())
        {
            if (create_recv_thread(threadName))
            {
                QCRIL_LOG_DEBUG("create_recv_thread failed");
            }
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

    boolean ret = (listen_sid == 0);
    QCRIL_LOG_FUNC_RETURN_WITH_RET(int(ret));
    return ret;

} // qcril_qmi_generic_socket_agent::start_socket_server

/*===========================================================================
  FUNCTION  qcril_qmi_generic_socket_agent::create_recv_thread
===========================================================================*/
/*!
    @brief Create GENERIC request receiver thread
*/
/*=========================================================================*/
boolean qcril_qmi_generic_socket_agent::create_recv_thread
(
  char *threadName
)
{
    boolean ret = 0;
    QCRIL_LOG_FUNC_ENTRY();
    pthread_attr_t attr;
#ifdef QMI_RIL_UTF
    pthread_attr_init (&attr);
    if ( 0 == utf_pthread_create_handler(&thread_id, &attr, qcril_qmi_generic_socket_agent::recv_thread, this) )
    {
#else
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if ( 0 == pthread_create(&thread_id, &attr, qcril_qmi_generic_socket_agent::recv_thread, this) )
    {
#endif
        qmi_ril_set_thread_name(thread_id, threadName);
    }
    else
    {
        QCRIL_LOG_ERROR("pthread creation failed");
        ret = 1;
    }
    pthread_attr_destroy(&attr);

    QCRIL_LOG_FUNC_RETURN();
    return ret;
}

/*===========================================================================
  FUNCTION  qcril_qmi_generic_socket_agent::init_socket_listenfd
===========================================================================*/
/*!
    @brief Create and start listening on socket
*/
/*=========================================================================*/
boolean qcril_qmi_generic_socket_agent::init_socket_listenfd
(
  void
)
{
    QCRIL_LOG_FUNC_ENTRY();
    struct sockaddr_un servaddr;

    /* creating socket */
    if ((listen_sid = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        QCRIL_LOG_ERROR("socket creation failed");
        return 1;
    }

    /* configuring server address structure */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family      = AF_UNIX;
    strncpy(servaddr.sun_path, socket_path, sizeof(servaddr.sun_path)-1);
    unlink(servaddr.sun_path);

    QCRIL_LOG_INFO("bind ...%s", servaddr.sun_path);

    /* binding our socket to the service port */
    if (bind(listen_sid, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
       QCRIL_LOG_ERROR("Could not bind socket to socket path");
       return 1;
    }

    QCRIL_LOG_INFO("listen ...");

    /* convert our socket to a listening socket */
    if (listen(listen_sid, max_connections) < 0)
    {
        QCRIL_LOG_ERROR("Could not make socket passive");
        return 1;
    }

    QCRIL_LOG_INFO("listen socket init finished %d", listen_sid);
    QCRIL_LOG_FUNC_RETURN();
    return 0;

} // qcril_qmi_generic_socket_agent::init_socket_listenfd

/*===========================================================================
  FUNCTION  qcril_qmi_generic_socket_agent::recv_thread
===========================================================================*/
/*!
    @brief Receive thread entry function
*/
/*=========================================================================*/
void *qcril_qmi_generic_socket_agent::recv_thread
(
  void *this_ptr
)
{
    ((qcril_qmi_generic_socket_agent *) this_ptr)->recv_thread_func();
    return NULL;

} // qcril_qmi_generic_socket_agent::recv_thread

/*===========================================================================
  FUNCTION  qcril_qmi_generic_socket_agent::recv_thread_func
===========================================================================*/
/*!
    @brief Receive message handler
*/
/*=========================================================================*/
void qcril_qmi_generic_socket_agent::recv_thread_func
(
  void
)
{
    QCRIL_LOG_FUNC_ENTRY();
    struct sockaddr_un cliaddr;
    socklen_t          clilen = sizeof(cliaddr);
    int                sid;
    int                read_length;

    for (;;)
    {
      if ((conn_sid = accept(listen_sid, (struct sockaddr*) &cliaddr, &clilen)) < 0)
      {
        QCRIL_LOG_ERROR("accept failed");
      }
      else
      {
        for (;;)
        {
          memset(recv_buffer, 0, sizeof(recv_buffer));

          if ((recv_byte_num = recv(conn_sid, recv_buffer, QCRIL_QMI_LENGTH_MESSAGE_LENGTH, 0)) < QCRIL_QMI_LENGTH_MESSAGE_LENGTH)
          {
#ifdef QMI_RIL_UTF
            if ( recv_byte_num == 3 )
            {
              if (strcmp((char*)(recv_buffer), "rs") == 0)
              {
                shutdown_request = 1;
                process_incoming_message();
              }
            }
#endif
              QCRIL_LOG_ERROR("receive message failed");
              break;
          }
          else
          {
            read_length = ntohl(*(unsigned int *)recv_buffer);

            QCRIL_LOG_ERROR("read %d bytes length message with length %d",
                             QCRIL_QMI_LENGTH_MESSAGE_LENGTH, read_length);

            if ((read_length > (int)sizeof(recv_buffer)) || (read_length <= 0))
            {
              QCRIL_LOG_ERROR("invalid length %d", read_length);
              break;
            }
            else
            {
              if ((recv_byte_num = recv(conn_sid, recv_buffer, read_length, 0)) <= 0)
              {
                QCRIL_LOG_ERROR("receive message failed");
                break;
              }
              else
              {
                process_incoming_message();
              }
            }
          }
        }
        close(conn_sid);
      }
    }

    QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_generic_socket_agent::recv_thread_func
