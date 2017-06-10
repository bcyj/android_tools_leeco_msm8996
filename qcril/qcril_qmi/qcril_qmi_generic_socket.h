/*!
  @file
  qcril_qmi_generic_socket.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifndef QCRIL_QMI_GENERIC_SOCKET_H
#define QCRIL_QMI_GENERIC_SOCKET_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef QMI_RIL_UTF
#define __STDC_LIMIT_MACROS
#endif

#include <sys/types.h>
#include "ril.h"
#include "qcril_qmi_client.h"
#include "qcril_other.h"
#include "qcril_arb.h"

#define QCRIL_QMI_GENERIC_SOCKET_MAX_BUF_SIZE    (1024*8)
#define QCRIL_QMI_GENERIC_SOCKET_PATH_SIZE       100
#define QCRIL_QMI_LENGTH_MESSAGE_LENGTH          4

class qcril_qmi_generic_socket_agent
{
public:

    boolean start_socket_server(char *threadName);
    boolean is_socket_server_started() { return (listen_sid != 0); }
    boolean is_socket_connected()  { return (conn_sid != 0); }
    qcril_qmi_generic_socket_agent(int max_conn);

    virtual ~qcril_qmi_generic_socket_agent();

private:

    boolean          init_socket_listenfd();
    boolean          create_recv_thread(char *threadName);
    static void*     recv_thread(void* this_ptr);
    void             recv_thread_func();
    virtual boolean  process_incoming_message() = 0;

protected:

    pthread_t                thread_id;
    int                      listen_sid;
    char                     socket_path[QCRIL_QMI_GENERIC_SOCKET_PATH_SIZE];
    int                      conn_sid;
    uint8_t                  recv_buffer[QCRIL_QMI_GENERIC_SOCKET_MAX_BUF_SIZE];
    int                      recv_byte_num;
    int                      max_connections;
#ifdef QMI_RIL_UTF
    int shutdown_request;
#endif

};

#ifdef  __cplusplus
}
#endif

#endif // QCRIL_QMI_GENERIC_SOCKET_H
