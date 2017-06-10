#ifndef _QSOCKET_IPCR_H_
#define _QSOCKET_IPCR_H_
/******************************************************************************
  @file    qsocket_ipcr.h
  @brief   IPC Router protocol specific definitions

  DESCRIPTION

  IPC Router:
  IPC Router is a connection-less datagram protocol covering the Network
  and Transport layers of the OSI model. It provides a packet delimited
  end-to-end flow control. Any streaming functionalities provided by
  IPC Router are implemented on top of the datagram methods. IPC Router
  also provides a reliable transport to users as it expects reliability
  from it's link layer.

  IPC Router provides a control endpoint to allow services/clients to
  listen on events from the distributed name service.

  IPC Router also provides a distributed name service in order to provide
  location transparency to it's clients

  This header provides a set of socket-like API to to communicate
  over IPC Router.

  Certain Operating systems might need special setup. Please refer
  to qsocket_<os>.h if it exists

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

 *******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <pthread.h>
#include <fcntl.h>
#include "qsocket.h"
#include "msm_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
                          SOCKET OPTION DEFINES
============================================================================*/

/* IPC Router specific socket options */
#define QSOL_IPC_ROUTER 0

/* Sets the current socket as a control port so it
 * may receive events from the name service */
#define QSO_IPCR_SET_CONTROL_PORT 1

/* This option is valid only if the QSO_IPCR_SET_CONTROL_PORT
* option is set on the socket. This will limit the name server
* events such as NEW_NAME and REMOVE_NAME to a specific type.
* The option value is a 32 bit integer (uint32_t) which will
* specify the 'type' field of the name we are interested in */
#define QSO_IPCR_SET_TYPE_PREF 2

/*============================================================================
                         SOCKET GETADDRINFO FLAGS
============================================================================*/

/* Flags to ipcr_getaddrinfo which asks the interface
 * to ignore the provided instance field of the name to lookup */
#define IPCR_FLAGS_ANY_INSTANCE 1

/*============================================================================
                             TYPES
============================================================================*/
#define AF_IPC_ROUTER AF_MSM_IPC

typedef struct msm_ipc_port_addr ipcr_port_t;
typedef struct msm_ipc_port_name ipcr_name_t;

#define IPCR_ADDR_PORT MSM_IPC_ADDR_ID
#define IPCR_ADDR_NAME MSM_IPC_ADDR_NAME

#define qsockaddr_ipcr sockaddr_msm_ipc

/* Type returned by name lookups */
typedef struct
{
  ipcr_name_t name;
  ipcr_port_t port;
} ipcr_addrinfo_t;

/* Types of events which can be received
 * from the CTRL socket */
typedef enum
{
  IPCR_CTRL_INVALID = 0,
  IPCR_CTRL_NEW_NAME,
  IPCR_CTRL_REMOVE_NAME,
  IPCR_CTRL_REMOVE_PORT,
} ipcr_ctrl_type;

/* Control message format received from
 * the control socket */
typedef struct
{
  ipcr_ctrl_type type;
  ipcr_port_t port;
  ipcr_name_t name;
} ipcr_ctrl_msg;

/*============================================================================
               PROTOCOL SPECIFIC HELPER METHODS
============================================================================*/

/*===========================================================================
  FUNCTION  ipcr_find_name
===========================================================================*/
/*!
@brief

 Finds all ports with the associated name

@param[in]   fd        Caller's file descriptor
@param[in]   name      Name of the port
@param[out]  addrs     Pointer to array of addresses to store
                       the address of found ports with matching
                       name.
@param[out]  instances (Optional, can be NULL) Pointer to array of
                       instance IDs of the names found (Useful if
                       IPCR_FLAGS_ANY_INSTANCE is used to get the
                       instance ID of the found address). Note, the
                       array must allow the same number of entries
                       as 'addrs'
@param[inout]num_entires Input the size of the array 'addrs' and
                         if provided, 'instances'.
                         Output, the number of entries filled.
@param[in]   flags     0 for an exact match on the provided
                       TYPE and INSTANCE pair.
                       IPCR_FLAGS_ANY_INSTANCE to match just
                       the TYPE.

@return
  Number of services found upon success, negative error code on failure.

@note

  - Dependencies
    - The user must hold a valid file descriptor toward IPC Router.
      (return of qsocket(AF_IPC_ROUTER, *, *)

  - Side Effects
    - None
*/
/*=========================================================================*/
static inline int ipcr_find_name(int fd, ipcr_name_t *name,
                    struct qsockaddr_ipcr *addrs,
                    unsigned int *instances, unsigned int *num_entries,
                    unsigned int flags)
{
  uint32_t num_servers_found = 0;
  uint32_t num_entries_to_fill = 0;
  struct server_lookup_args *lookup_arg;
  int i;

  if (fd < 0)
    return -QEBADF;

  if (num_entries)
  {
    num_entries_to_fill = *num_entries;
    *num_entries = 0;
  }

  lookup_arg = (struct server_lookup_args *)malloc(sizeof(*lookup_arg)
                        + (num_entries_to_fill * sizeof(struct msm_ipc_server_info)));
  if (!lookup_arg)
    return -QENOMEM;

  lookup_arg->port_name.service = name->service;
  if (flags == IPCR_FLAGS_ANY_INSTANCE)
  {
    lookup_arg->port_name.instance = 0;
    lookup_arg->lookup_mask = 0;
  } else
  {
    lookup_arg->port_name.instance = name->instance;
    lookup_arg->lookup_mask = 0xFFFFFFFF;
  }
  lookup_arg->num_entries_in_array = num_entries_to_fill;
  lookup_arg->num_entries_found = 0;
  if (ioctl(fd, IPC_ROUTER_IOCTL_LOOKUP_SERVER, lookup_arg) < 0) {
    free(lookup_arg);
    return -errno;
  }

  for (i = 0; ((i < (int)num_entries_to_fill) && (i < lookup_arg->num_entries_found)); i++) {
    (addrs + i)->family = AF_MSM_IPC;
    (addrs + i)->address.addrtype = MSM_IPC_ADDR_ID;
    (addrs + i)->address.addr.port_addr.node_id = lookup_arg->srv_info[i].node_id;
    (addrs + i)->address.addr.port_addr.port_id = lookup_arg->srv_info[i].port_id;
    if (instances)
      *(instances + i) = lookup_arg->srv_info[i].instance;
  }
  if (num_entries)
    *num_entries = i;
  num_servers_found = lookup_arg->num_entries_found;
  free(lookup_arg);
  return num_servers_found;
}

#ifdef __cplusplus
}
#endif

#endif
