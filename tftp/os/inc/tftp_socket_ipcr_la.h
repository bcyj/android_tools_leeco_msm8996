/***********************************************************************
 * tftp_socket_ipcr_la.h
 *
 * Short description
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose description.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-04   rp    Create

===========================================================================*/

#ifndef __TFTP_SOCKET_IPCR_LA_H__
#define __TFTP_SOCKET_IPCR_LA_H__

#include "tftp_config_i.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>

#include "msm_ipc.h"

typedef int tftp_socket;
typedef struct sockaddr_msm_ipc tftp_sockaddr;
typedef socklen_t tftp_sockaddr_len_type;
typedef struct pollfd tftp_socket_pollfd;


#endif /* not __TFTP_SOCKET_IPCR_LA_H__ */
