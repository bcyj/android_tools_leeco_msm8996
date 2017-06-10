
/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/14/13   yt      Fix critical KW errors
08/09/12   sc      Added optimizations to improve on socket switching delay.
03/22/12   at      Replacing malloc()/free() with qcril_malloc()/qcril_free()
09/27/11   at      Fix to bind the server socket only on INADDR_LOOPBACK
06/16/11   at      Fixes for pipelining multiple sockets & closing sockets
                   on card error & refresh reset
04/20/11   mib     Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <sys/socket.h>
#include <netinet/in.h>
#include "qcril_scws.h"


/*=========================================================================
  Macros for critical sections
===========================================================================*/
#define QCRIL_SCWS_INIT_CRITICAL_SECTION

#define QCRIL_SCWS_ENTER_CRITICAL_SECTION                      \
           QCRIL_MUTEX_LOCK(&qcril_scws_mutex, "qcril_scws_mutex")

#define QCRIL_SCWS_LEAVE_CRITICAL_SECTION                      \
           QCRIL_MUTEX_UNLOCK(&qcril_scws_mutex, "qcril_scws_mutex")


/*=========================================================================
  Global mutex
===========================================================================*/
static pthread_mutex_t                     qcril_scws_mutex = PTHREAD_MUTEX_INITIALIZER;

/*=========================================================================
  Global array of server sockets
===========================================================================*/
static qcril_scws_server_socket_type       server_socket[QCRIL_SCWS_MAX_SERVER_SOCKETS];

/*=========================================================================
  Global pointers with callback functions
===========================================================================*/
static qcril_scws_channel_status_cb_type  * qcril_scws_channel_status_cb  = NULL;
static qcril_scws_data_available_cb_type  * qcril_scws_data_available_cb  = NULL;


/*=========================================================================

  FUNCTION:  qcril_scws_swap_sockets

===========================================================================*/
/*!
    @brief
    Given the server socket pointer, swaps the conneceted socket at index
    sock1_index with the connected socket at index sock2_index.

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_swap_sockets
(
  qcril_scws_server_socket_type * server_ptr,
  int                             sock1_index,
  int                             sock2_index
)
{
#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
  qcril_scws_opt_traffic_analyzer_type temp_analyzer =
    server_ptr->connected_socket[sock1_index].traffic_analyzer;
#endif /* QCRIL_SCWS_DISABLE_OPTMIZATION */

  int temp_sd_holder =
    server_ptr->connected_socket[sock1_index].connected_sd;
  uint16 temp_buffer_size =
    server_ptr->connected_socket[sock1_index].buffer_size;
  uint8 *temp_buffer_ptr =
    server_ptr->connected_socket[sock1_index].buffer_ptr;

  server_ptr->connected_socket[sock1_index].connected_sd =
    server_ptr->connected_socket[sock2_index].connected_sd;
  server_ptr->connected_socket[sock2_index].connected_sd = temp_sd_holder;

  server_ptr->connected_socket[sock1_index].buffer_size =
    server_ptr->connected_socket[sock2_index].buffer_size;
  server_ptr->connected_socket[sock2_index].buffer_size = temp_buffer_size;

  server_ptr->connected_socket[sock1_index].buffer_ptr =
    server_ptr->connected_socket[sock2_index].buffer_ptr;
  server_ptr->connected_socket[sock2_index].buffer_ptr = temp_buffer_ptr;

#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
  server_ptr->connected_socket[sock1_index].traffic_analyzer =
    server_ptr->connected_socket[sock2_index].traffic_analyzer;
  server_ptr->connected_socket[sock2_index].traffic_analyzer = temp_analyzer;
#endif /* QCRIL_SCWS_DISABLE_OPTMIZATION */
} /* qcril_scws_swap_sockets */


/*=========================================================================

  FUNCTION:  qcril_scws_reorder_sockets

===========================================================================*/
/*!
    @brief
    Enqueue the socket at active_index at the back of the queue (farthest away
    from being used), and put the next available socket at the front.

    @return
    The active index after the reordering, if a reordering occurs.
    Otherwise, returns -1.
*/
/*=========================================================================*/
static int qcril_scws_reorder_sockets
(
  qcril_scws_server_socket_type * server_ptr,
  int                             active_index
)
{
  int i                           = 0;
  int j                           = 0;
  int first_open_index            = 0;
  int new_active_index            = -1;

  QCRIL_LOG_DEBUG("At start of reorder_sockets: \n");
  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    QCRIL_LOG_DEBUG("%s, connected_socket[%d].connected_sd = %x, request count = %d, buffer_size: %d, valid_bip_id: %d \n",
                    __FUNCTION__, i, server_ptr->connected_socket[i].connected_sd,
                                     server_ptr->connected_socket[i].traffic_analyzer.request_count,
                                     server_ptr->connected_socket[i].buffer_size,
                                     server_ptr->connected_socket[i].valid_bip_id);
  }

  for (i = QCRIL_SCWS_MAX_CLIENT_SOCKETS-1; i >= 0; i--)
  {
    /* search for first -1 starting from end of array,
       to avoid picking up the first -1 from the beginning
       of an array like [0x17, 0x18, -1, 0x19, -1, -1, -1].
       break if no open index available. */
    if (server_ptr->connected_socket[i].connected_sd != -1)
    {
      if (i == QCRIL_SCWS_MAX_CLIENT_SOCKETS)
      {
        break;
      }
      first_open_index = i+1;
      break;
    }
  }
  /* If no open index found, set it to the max index of the
     array, so swapping will propogate through the entire
     array. */
  if (first_open_index == 0)
  {
    first_open_index = QCRIL_SCWS_MAX_CLIENT_SOCKETS;
  }

  for (j = 0; j < QCRIL_SCWS_MAX_CLIENT_SOCKETS; j++)
  {

    if ((server_ptr->connected_socket[j].valid_bip_id == FALSE) &&
        (server_ptr->connected_socket[j].connected_sd != -1) &&
        (server_ptr->connected_socket[j].buffer_size != 0))
    {
      QCRIL_LOG_DEBUG("Moving connected_sd: 0x%X from index %d to %d \n",
                       server_ptr->connected_socket[j].connected_sd, j, active_index);

      qcril_scws_swap_sockets(server_ptr, active_index, j);

      while (j < first_open_index &&
             j+1 < first_open_index &&
             j+1 < QCRIL_SCWS_MAX_CLIENT_SOCKETS)
      {
        qcril_scws_swap_sockets(server_ptr, j, j+1);
        j++;
      }
      new_active_index = active_index;
      break;
    }
  }

  QCRIL_LOG_DEBUG("At end of reorder_sockets: \n");
  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    QCRIL_LOG_DEBUG("%s, connected_socket[%d].connected_sd = %x, request count = %d, buffer_size: %d, valid_bip_id: %d \n",
                    __FUNCTION__, i, server_ptr->connected_socket[i].connected_sd,
                                     server_ptr->connected_socket[i].traffic_analyzer.request_count,
                                     server_ptr->connected_socket[i].buffer_size,
                                     server_ptr->connected_socket[i].valid_bip_id);
  }
  return new_active_index;
} /* qcril_scws_reorder_sockets */


/*=========================================================================

  FUNCTION:  qcril_scws_handle_new_connection

===========================================================================*/
/*!
    @brief
    Handle a new connection received on the server socket

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_handle_new_connection
(
  qcril_scws_server_socket_type * server_ptr
)
{
  int8               i            = 0;
  int8               index        = -1;
  int                new_sd       = 0;
  struct sockaddr    address;
  int                address_size = sizeof(address);

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "%s, server_sd: 0x%X \n", __FUNCTION__, server_ptr->server_sd);

  /* Accept the new incoming connection */
  new_sd = accept(server_ptr->server_sd, &address, &address_size);

  if (new_sd < 0)
  {
    /* Error creating the new connected socket... nothing to do */
    QCRIL_LOG_ERROR( "Error accepting new connection, new_sd: 0x%X \n", new_sd);

    return;
  }

  /* Check the address... we should accept only local connections */

  /* Add socket to the array, if there is space. Preference to pick a
     spot that already has an associated BIP id */
  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    if (server_ptr->connected_socket[i].connected_sd == -1)
    {
      if (server_ptr->connected_socket[i].valid_bip_id)
      {
        /* We found a socket with a valid BIP id... use this one */
        index = i;
        break;
      }
      else if (index == -1)
      {
        /* First socket without a BIP id: continue searching */
        index = i;
      }
    }
  }

  if (index >= 0 && index < QCRIL_SCWS_MAX_CLIENT_SOCKETS)
  {
    QCRIL_LOG_DEBUG( "Storing, new_sd: 0x%X , index: %d\n", new_sd, index);

    /* Store the new socket descriptor */
    server_ptr->connected_socket[index].connected_sd = new_sd;

    /* Send the channel status only for connected socket with valid_bip_id */
    if (server_ptr->connected_socket[index].valid_bip_id)
    {
      QCRIL_ASSERT(qcril_scws_channel_status_cb);

      qcril_scws_channel_status_cb(server_ptr->connected_socket[index].bip_id,
                                   server_ptr->slot_id,
                                   QCRIL_SCWS_SOCKET_STATE_ESTABLISHED);
    }

    /* Initialize traffic analyzer for this socket */
#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
    qcril_scws_opt_reset(&server_ptr->connected_socket[index].traffic_analyzer);
#endif /* QCRIL_SCWS_DISABLE_OPTIMIZATION */
    return;
  }

  /* No space to accept the new socket */
  close(new_sd);
} /* qcril_scws_handle_new_connection */


/*=========================================================================

  FUNCTION:  qcril_scws_close_socket

===========================================================================*/
/*!
    @brief
    Close a socket. This function can be used for both the server and a
    connected socket.

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_close_socket
(
  qcril_scws_server_socket_type * server_ptr,
  int                             sd,
  boolean                         notify_card
)
{
  uint8     i                   =  0;

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "%s, socket: 0x%X, server_sd: 0x%X, notify_card: %s\n",
                 __FUNCTION__, sd, server_ptr->server_sd, !notify_card ? "FALSE" : "TRUE");

  /* Validate the socket descriptor */
  if (sd == -1)
  {
    return;
  }

  /* Case of the server socket being closed */
  if (server_ptr->server_sd == sd)
  {
    /* Close all connected sockets */
    for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
    {
      /* Notify the card, if needed */
      if (notify_card &&
          server_ptr->connected_socket[i].valid_bip_id)
      {
        QCRIL_ASSERT(qcril_scws_channel_status_cb);

        qcril_scws_channel_status_cb(server_ptr->connected_socket[i].bip_id,
                                     server_ptr->slot_id,
                                     QCRIL_SCWS_SOCKET_STATE_CLOSED);
      }

      /* Use recursive call to close the connected socket... in this case
         there is no need to notify the card again, so setting the boolean
         to FALSE */
      if (server_ptr->connected_socket[i].connected_sd != -1)
      {
        qcril_scws_close_socket(server_ptr,
                                server_ptr->connected_socket[i].connected_sd,
                                FALSE);
      }
    }

    /* Clear the server socket info, this will implicitly cause server thread
       to close. Server thread will be re-created on the next open channel request */
    server_ptr->local_port = 0;
    server_ptr->thread_id  = -1;

    /* Close the server socket */
    close(server_ptr->server_sd);
    server_ptr->server_sd = -1;

    return;
  }

  /* Handle for a connected socket */
  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    if (server_ptr->connected_socket[i].connected_sd == sd)
    {
      /* Close the socket and make the descriptor invalid, and
         clear analyzer info (if feature is enabled) */
      close(server_ptr->connected_socket[i].connected_sd);
      server_ptr->connected_socket[i].connected_sd = -1;

#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
      qcril_scws_opt_reset(&server_ptr->connected_socket[i].traffic_analyzer);
#endif /* QCRIL_SCWS_DISABLE_OPTIMIZATION */

      /* Deallocate the buffer if it's still present */
      if (server_ptr->connected_socket[i].buffer_ptr != NULL)
      {
        qcril_free(server_ptr->connected_socket[i].buffer_ptr);
        server_ptr->connected_socket[i].buffer_ptr = NULL;
      }
      server_ptr->connected_socket[i].buffer_size = 0;

      /* If the BIP id is valid and there are no other connected sockets,
         send indication to card. At this point, the BIP id remains valid */
      if (server_ptr->connected_socket[i].valid_bip_id)
      {
        if (notify_card)
        {
          QCRIL_ASSERT(qcril_scws_channel_status_cb);

          qcril_scws_channel_status_cb(server_ptr->connected_socket[i].bip_id,
                                       server_ptr->slot_id,
                                       QCRIL_SCWS_SOCKET_STATE_LISTEN);
        }
      }

      /* Exit the loop after the socket is removed */
      break;
    }
  }
} /* qcril_scws_close_socket */


/*=========================================================================

  FUNCTION:  qcril_scws_socket_closed_switch_to_next

===========================================================================*/
/*!
    @brief
    Handles sending of data if there is a client socket established with
    saved data.

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_socket_closed_switch_to_next
(
  qcril_scws_server_socket_type * server_ptr
)
{
  int i = 0;
  int j = 0;

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  /* Check if any socket was removed that was associated with a valid bip id */
  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    if ((server_ptr->connected_socket[i].valid_bip_id) &&
        (server_ptr->connected_socket[i].connected_sd == -1))
    {
      /* This is free spot where a connected socket can be associated, so
         find the next avaliable connected socket & copy it */
      for (j = 0; j < QCRIL_SCWS_MAX_CLIENT_SOCKETS; j++)
      {
        if ((server_ptr->connected_socket[j].valid_bip_id == FALSE) &&
            (server_ptr->connected_socket[j].connected_sd != -1))
        {
          QCRIL_LOG_DEBUG("Moving connected_sd: 0x%X from index %d to %d \n",
                     server_ptr->connected_socket[j].connected_sd, j, i);

          server_ptr->connected_socket[i].connected_sd =
            server_ptr->connected_socket[j].connected_sd;
          server_ptr->connected_socket[j].connected_sd = -1;

          server_ptr->connected_socket[i].buffer_size =
            server_ptr->connected_socket[j].buffer_size;
          server_ptr->connected_socket[j].buffer_size = 0;

          server_ptr->connected_socket[i].buffer_ptr =
            server_ptr->connected_socket[j].buffer_ptr;
          server_ptr->connected_socket[j].buffer_ptr = NULL;

#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
          /* handle for traffic analyzer */
          server_ptr->connected_socket[i].traffic_analyzer =
            server_ptr->connected_socket[j].traffic_analyzer;
          qcril_scws_opt_reset(&server_ptr->connected_socket[j].traffic_analyzer);
#endif /* QCRIL_SCWS_DISABLE_OPTIMIZATION */

          /* Send the channel status since we would have sent listen state before */
          QCRIL_ASSERT(qcril_scws_channel_status_cb);

          qcril_scws_channel_status_cb(server_ptr->connected_socket[i].bip_id,
                                       server_ptr->slot_id,
                                       QCRIL_SCWS_SOCKET_STATE_ESTABLISHED);

          /* If there is cached data, send data to card and free it */
          if (server_ptr->connected_socket[i].buffer_size > 0 &&
              server_ptr->connected_socket[i].buffer_ptr != NULL)
          {
            QCRIL_LOG_DEBUG("Sending data to modem: bip_id: 0x%X, num_bytes: 0x%X, socket: 0x%X, \n",
                            server_ptr->connected_socket[i].bip_id,
                            server_ptr->connected_socket[i].buffer_size,
                            server_ptr->connected_socket[i].connected_sd);

            /* Send the data to the modem */
            QCRIL_ASSERT(qcril_scws_data_available_cb);

            qcril_scws_data_available_cb(server_ptr->connected_socket[i].bip_id,
                                         server_ptr->slot_id,
                                         server_ptr->connected_socket[i].buffer_size,
                                         server_ptr->connected_socket[i].buffer_ptr,
                                         0);

            qcril_free(server_ptr->connected_socket[i].buffer_ptr);
            server_ptr->connected_socket[i].buffer_ptr = NULL;
            server_ptr->connected_socket[i].buffer_size = 0;
          }

          /* Exit the loop if new connected socket was filled */
          break;
        }
      } /* inner for loop for j */
    }
  } /* outer for loop for i */
} /* qcril_scws_socket_closed_switch_to_next */


/*=========================================================================

  FUNCTION:  qcril_scws_socket_connected_switch_to_next

===========================================================================*/
/*!
    @brief
    Handles sending of data if there is a client socket established with
    saved data.

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_socket_connected_switch_to_next
(
  qcril_scws_server_socket_type * server_ptr,
  uint32                          bip_id
)
{
  int i                  = 0;
  int j                  = 0;
  int new_active_index   = 0;

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  /* Search the socket with given bip_id */
  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    if (server_ptr->connected_socket[i].valid_bip_id &&
        server_ptr->connected_socket[i].bip_id == bip_id)
    {
      qcril_scws_opt_reset(&server_ptr->connected_socket[i].traffic_analyzer);

      /* This is free spot where a connected socket can be associated, so
         find the next avaliable connected socket & copy it */
      new_active_index = qcril_scws_reorder_sockets(server_ptr, i);

      if (new_active_index > -1)
      {
        /* Notify the card of the reordering */
        QCRIL_ASSERT(qcril_scws_channel_status_cb);

        qcril_scws_channel_status_cb(server_ptr->connected_socket[i].bip_id,
                                     server_ptr->slot_id,
                                     QCRIL_SCWS_SOCKET_STATE_LISTEN);

        qcril_scws_channel_status_cb(server_ptr->connected_socket[i].bip_id,
                                     server_ptr->slot_id,
                                     QCRIL_SCWS_SOCKET_STATE_ESTABLISHED);

        /* If there is cached data, send data to card and free it */
        if (server_ptr->connected_socket[i].buffer_size > 0 &&
            server_ptr->connected_socket[i].buffer_ptr != NULL)
        {
          QCRIL_LOG_DEBUG("Sending data to modem: bip_id: 0x%X, num_bytes: 0x%X, socket: 0x%X, \n",
                          server_ptr->connected_socket[i].bip_id,
                          server_ptr->connected_socket[i].buffer_size,
                          server_ptr->connected_socket[i].connected_sd);

          /* Send the data to the modem */
          QCRIL_ASSERT(qcril_scws_data_available_cb);

          qcril_scws_data_available_cb(server_ptr->connected_socket[i].bip_id,
                                       server_ptr->slot_id,
                                       server_ptr->connected_socket[i].buffer_size,
                                       server_ptr->connected_socket[i].buffer_ptr,
                                       0);

          qcril_free(server_ptr->connected_socket[i].buffer_ptr);
          server_ptr->connected_socket[i].buffer_ptr = NULL;
          server_ptr->connected_socket[i].buffer_size = 0;
        }

        /* Return if new connected socket was filled */
        return;
      }
    }
  } /* outer for loop for i */
} /* qcril_scws_socket_connected_switch_to_next */


/*=========================================================================

  FUNCTION:  qcril_scws_handle_data_from_socket

===========================================================================*/
/*!
    @brief
    Handles data coming from a socket.

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_handle_data_from_socket
(
  qcril_scws_server_socket_type * server_ptr,
  int                             connected_sd
)
{
  int       i              = 0;
  int       incoming_index = -1;
  ssize_t   received_len   = 0;
  uint8     socket_buffer[QCRIL_SCWS_TEMP_BUFFER_SIZE];

  /* To be used if we need to concatenate
     two buffers */
  uint16    total_size     = 0;
  uint8   * temp_buffer    = NULL;

#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
  qcril_scws_analyzer_state_enum_type state;
#endif

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  memset(&socket_buffer, 0, QCRIL_SCWS_TEMP_BUFFER_SIZE);

  received_len = recv(connected_sd,
                      socket_buffer,
                      QCRIL_SCWS_TEMP_BUFFER_SIZE,
                      0);

  if (received_len <= 0)
  {
    /* Error case - first close the specified socket */
    QCRIL_LOG_DEBUG("%s, closing socket inside handle_data recv less than zero\n",
                    __FUNCTION__);
    qcril_scws_close_socket(server_ptr, connected_sd, TRUE);

    /* Closed socket could be the one with a valid bip id, so check if there
       is a pending socket to be processed */
    qcril_scws_socket_closed_switch_to_next(server_ptr);

    return;
  }

  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    if (server_ptr->connected_socket[i].connected_sd == connected_sd)
    {
      incoming_index = i;
      break;
    }
  }

  if (incoming_index == -1)
  {
    QCRIL_LOG_DEBUG("%s, Not able to handle incoming data.\n", __FUNCTION__);
    /* Not able to handle the incoming data... this should not happen.
       Close the socket for safety reasons */
    qcril_scws_close_socket(server_ptr, connected_sd, TRUE);
    return;
  }

  QCRIL_LOG_DEBUG( "Received data, 0x%X bytes from socket: 0x%X\n", received_len, connected_sd);

  /* Mine header info from RX data (from browser to card) and update traffic analyzer state */
#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
  qcril_scws_opt_process_rx(&server_ptr->connected_socket[incoming_index].traffic_analyzer,
                            socket_buffer,
                            received_len);
#endif /* QCRIL_SCWS_DISABLE_OPTIMIZATION */

  /* Check if the socket is associated to a BIP id */
  if (server_ptr->connected_socket[incoming_index].valid_bip_id)
  {
    QCRIL_LOG_DEBUG("Sending data to modem: bip_id: 0x%X, num_bytes: 0x%X, socket: 0x%X, \n",
                     server_ptr->connected_socket[incoming_index].bip_id, received_len, connected_sd);

    /* Send the data to the modem */
    QCRIL_ASSERT(qcril_scws_data_available_cb);

    qcril_scws_data_available_cb(server_ptr->connected_socket[incoming_index].bip_id,
                                 server_ptr->slot_id,
                                 received_len,
                                 socket_buffer,
                                 0);

    /* We can return... nothing else to do */
    return;
  }

#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
  /* If there is an available connected socket in IDLE state,
     switch to that socket and start transmitting its data
     to the card. This only occurs if the BIP id of the socket
     with incoming data is not valid. */
  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    qcril_scws_analyzer_state_enum_type state =
      server_ptr->connected_socket[i].traffic_analyzer.analyzer_state;

    if (server_ptr->connected_socket[i].valid_bip_id &&
        server_ptr->connected_socket[i].connected_sd != -1  &&
        state == QCRIL_SCWS_OPT_ANALYZER_STATE_IDLE)
    {
      qcril_scws_swap_sockets(server_ptr, i, incoming_index);
      QCRIL_LOG_DEBUG("Incoming data on socket at index %d. Swapping with available socket at index %d\n",
                       incoming_index, i);
      QCRIL_LOG_DEBUG("Sending data to modem: bip_id: 0x%X, num_bytes: 0x%X, socket: 0x%X, \n",
                       server_ptr->connected_socket[i].bip_id, received_len, connected_sd);

      /* Notify the card of the swap */
      QCRIL_ASSERT(qcril_scws_channel_status_cb);

      qcril_scws_channel_status_cb(server_ptr->connected_socket[i].bip_id,
                                   server_ptr->slot_id,
                                   QCRIL_SCWS_SOCKET_STATE_LISTEN);

      qcril_scws_channel_status_cb(server_ptr->connected_socket[i].bip_id,
                                   server_ptr->slot_id,
                                   QCRIL_SCWS_SOCKET_STATE_ESTABLISHED);

      /* Send the data to the modem */
      QCRIL_ASSERT(qcril_scws_data_available_cb);

      qcril_scws_data_available_cb(server_ptr->connected_socket[i].bip_id,
                                   server_ptr->slot_id,
                                   received_len,
                                   socket_buffer,
                                   0);
      return;
    }
  }
#endif /* QCRIL_SCWS_DISABLE_OPTIMIZATION */

  /* Calculate new buffer size, as the sum of the current buffer size
      plus the new received data and allocate a buffer */
  total_size = received_len + server_ptr->connected_socket[incoming_index].buffer_size;
  temp_buffer = qcril_malloc(total_size);

  if (temp_buffer == NULL)
  {
    QCRIL_LOG_DEBUG("%s, Error allocating memory for temporary buffer\n", __FUNCTION__);
    /* Error allocating the memory... we need to close this socket */
    qcril_scws_close_socket(server_ptr, connected_sd, TRUE);
    return;
  }

  QCRIL_LOG_DEBUG("Saving data received from socket: 0x%X, num_bytes: 0x%X \n",
                  connected_sd, received_len);

  /* Copy the old buffer into the new one */
  if (server_ptr->connected_socket[incoming_index].buffer_size > 0 &&
      server_ptr->connected_socket[incoming_index].buffer_ptr != NULL)
  {
    memcpy(temp_buffer,
           server_ptr->connected_socket[incoming_index].buffer_ptr,
           server_ptr->connected_socket[incoming_index].buffer_size);
  }
  /* Add the new data after the existing buffer */
  memcpy(temp_buffer + server_ptr->connected_socket[incoming_index].buffer_size,
         socket_buffer,
         received_len);

  /* Free the old buffer, as no longer needed */
  if (server_ptr->connected_socket[incoming_index].buffer_ptr != NULL)
  {
    qcril_free(server_ptr->connected_socket[incoming_index].buffer_ptr);
    server_ptr->connected_socket[incoming_index].buffer_ptr = NULL;
  }

  /* Associate the new buffer with the socket */
  server_ptr->connected_socket[incoming_index].buffer_size += received_len;
  server_ptr->connected_socket[incoming_index].buffer_ptr = temp_buffer;
  temp_buffer = NULL;

  return;
} /* qcril_scws_handle_data_from_socket */


/*=========================================================================

  FUNCTION:  qcril_scws_handle_socket_error

===========================================================================*/
/*!
    @brief
    Handle a socket error returned from the select

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_handle_socket_error
(
  qcril_scws_server_socket_type * server_ptr
)
{
  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_DEBUG("%s, closing socket inside handle_socket_error\n",
                    __FUNCTION__);

  /* There was an error in the select... close the server socket */
  qcril_scws_close_socket(server_ptr, server_ptr->server_sd, TRUE);
} /* qcril_scws_handle_socket_error */


/*=========================================================================

  FUNCTION:  qcril_scws_handle_socket_event

===========================================================================*/
/*!
    @brief
    Handle a socket event: this is invoked in the normal case when
    an event is received in the select

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_handle_socket_event
(
  qcril_scws_server_socket_type * server_ptr
)
{
  uint8  i        =  0;

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  if (FD_ISSET(server_ptr->server_sd, &server_ptr->fd_set))
  {
    qcril_scws_handle_new_connection(server_ptr);
  }

  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    if (server_ptr->connected_socket[i].connected_sd != -1)
    {
      if (FD_ISSET(server_ptr->connected_socket[i].connected_sd, &server_ptr->fd_set))
      {
        qcril_scws_handle_data_from_socket(server_ptr,
                                           server_ptr->connected_socket[i].connected_sd);
      }
    }
  }
} /* qcril_scws_handle_socket_event */


/*=========================================================================

  FUNCTION:  qcril_scws_set_fd

===========================================================================*/
/*!
    @brief
    Set the descriptors to be monitored in the select()

    @return
    Value of the highest descriptor
*/
/*=========================================================================*/
static int qcril_scws_set_fd
(
  qcril_scws_server_socket_type * server_ptr
)
{
  uint8  i        =  0;
  int    max_sd   = -1;

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return max_sd;
  }

  QCRIL_SCWS_ENTER_CRITICAL_SECTION;

  /* Clears out the fd_set from all descriptors. */
  FD_ZERO(&server_ptr->fd_set);

  /* Set the server socket */
  FD_SET(server_ptr->server_sd, &server_ptr->fd_set);
  max_sd = server_ptr->server_sd;

  /* Set all connected sockets */
  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    if (server_ptr->connected_socket[i].connected_sd != -1)
    {
      FD_SET(server_ptr->connected_socket[i].connected_sd, &server_ptr->fd_set);
      if (server_ptr->connected_socket[i].connected_sd > max_sd)
      {
        max_sd = server_ptr->connected_socket[i].connected_sd;
      }
    }
  }

  QCRIL_SCWS_LEAVE_CRITICAL_SECTION;

  return max_sd;
} /* qcril_scws_set_fd */


/*=========================================================================

  FUNCTION:  qcril_scws_select

===========================================================================*/
/*!
    @brief
    Main function with the select() for the sockets

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_select
(
  qcril_scws_server_socket_type * server_ptr,
  int                             max_sd
)
{
  int            select_ret = 0;
  struct timeval timeout;

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  /* Timeout value... should we use NULL here? */
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  /* The select needs to be executed outside the critical section
     to avoid blocking the main thread */
  select_ret = select(max_sd + 1, &server_ptr->fd_set, NULL, NULL, &timeout);

  QCRIL_SCWS_ENTER_CRITICAL_SECTION;

  if (select_ret < 0)
  {
    qcril_scws_handle_socket_error(server_ptr);
  }
  else if (select_ret == 0)
  {
    /* Nothing to do in this case */
  }
  else
  {
    qcril_scws_handle_socket_event(server_ptr);
  }

  QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
} /* qcril_scws_select */


/*=========================================================================

  FUNCTION:  qcril_scws_socket_thread

===========================================================================*/
/*!
    @brief
    Thread function to handle sockets

    @return
    None
*/
/*=========================================================================*/
static void qcril_scws_socket_thread(void* arg)
{
  /* Retrieve server socket structure used for this thread */
  qcril_scws_server_socket_type * server_ptr = (qcril_scws_server_socket_type *)arg;

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_DEBUG("Started socket thread for local_port: %d", server_ptr->local_port);

  /* Start infinite loop for the socket */
  while(server_ptr->local_port != 0)
  {
    int max_sd = 0;

    max_sd = qcril_scws_set_fd(server_ptr);
    qcril_scws_select(server_ptr, max_sd);
  }

  QCRIL_LOG_DEBUG("Exiting socket thread for local_port: %d", server_ptr->local_port);

} /* qcril_scws_socket_thread */


/*=========================================================================

  FUNCTION:  qcril_scws_initalize

===========================================================================*/
/*!
    @brief
    Initializes the SCWS module

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_initalize
(
  qcril_scws_data_available_cb_type  * data_available_cb,
  qcril_scws_channel_status_cb_type  * channel_status_cb
)
{
  uint8  i  = 0;
  uint8  j  = 0;

  /* Initialize critical section */
  QCRIL_SCWS_INIT_CRITICAL_SECTION

  /* Initialize global callbacks */
  qcril_scws_data_available_cb  = data_available_cb;
  qcril_scws_channel_status_cb  = channel_status_cb;

  /* Initialize array of server sockets */
  memset(server_socket, 0, sizeof(server_socket));

  for (i = 0; i < QCRIL_SCWS_MAX_SERVER_SOCKETS; i++)
  {
    server_socket[i].local_port = 0;
    server_socket[i].thread_id  = -1;
    server_socket[i].server_sd  = -1;

    for (j = 0; j < QCRIL_SCWS_MAX_CLIENT_SOCKETS; j++)
    {
      server_socket[i].connected_socket[j].connected_sd = -1;
      server_socket[i].connected_socket[j].valid_bip_id = FALSE;
    }
  }

  QCRIL_LOG_DEBUG("%s", "qcril_scws initalized");
} /* qcril_scws_initalize */


/*=========================================================================

  FUNCTION:  qcril_scws_deinitalize

===========================================================================*/
/*!
    @brief
    Initializes the SCWS module

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_deinitalize
(
  void
)
{
  uint8   i   = 0;

  QCRIL_SCWS_ENTER_CRITICAL_SECTION;

  /* Close all the sockets */
  for (i = 0; i < QCRIL_SCWS_MAX_SERVER_SOCKETS; i++)
  {
    if (server_socket[i].local_port != 0 &&
        server_socket[i].server_sd != -1)
    {
      qcril_scws_close_socket(&server_socket[i],
                              server_socket[i].server_sd,
                              FALSE);
    }
  }

  /* Reset callback pointers */
  qcril_scws_data_available_cb  = NULL;
  qcril_scws_channel_status_cb  = NULL;

  QCRIL_SCWS_LEAVE_CRITICAL_SECTION;

  QCRIL_LOG_DEBUG("%s", "qcril_scws de-initalized");
} /* qcril_scws_deinitalize */


/*=========================================================================

  FUNCTION:  qcril_scws_add_bip_id_to_server_socket

===========================================================================*/
/*!
    @brief
    Function that adds a new BIP id to an existing server socket

    @return
    None
*/
/*=========================================================================*/
static boolean qcril_scws_add_bip_id_to_server_socket
(
  qcril_scws_server_socket_type * server_ptr,
  uint32                          bip_id
)
{
  int8 i     = 0;
  int8 index = -1;

  /* Sanity check */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return FALSE;
  }

  QCRIL_LOG_INFO( "%s, bip_id: 0x%X, server_sd: 0x%X\n",
                    __FUNCTION__, bip_id, server_ptr->server_sd);
  /* Add the new BIP id. If there is a socket that it's
     already connected, use that one, otherwise pick a free
     socket */
  for (i = 0; i < QCRIL_SCWS_MAX_CLIENT_SOCKETS; i++)
  {
    if (!server_ptr->connected_socket[i].valid_bip_id)
    {
      if (server_ptr->connected_socket[i].connected_sd != -1)
      {
        /* Found a socket that it's already connected. At this
           point we can break from the for loop */
        index = i;
        break;
      }
      if (server_ptr->connected_socket[i].connected_sd == -1 &&
          index == -1)
      {
        /* Found a socket that does not have a BIP id, but it is
           not connected, so continue the loop */
        index = i;
      }
    }
  }

  if (index < 0 || index > QCRIL_SCWS_MAX_CLIENT_SOCKETS)
  {
    /* No space for the new BIP id */
    QCRIL_LOG_ERROR("No space for the new BIP id: 0x%X", bip_id);
    return FALSE;
  }

  QCRIL_LOG_DEBUG("Adding bip_id 0x%X to server_ptr->connected_socket[%d]", bip_id, index);

  server_ptr->connected_socket[index].valid_bip_id = TRUE;
  server_ptr->connected_socket[index].bip_id       = bip_id;

  /* If the socket is already connected, notify the card */
  if (server_ptr->connected_socket[index].connected_sd != -1)
  {
    QCRIL_ASSERT(qcril_scws_channel_status_cb);

    qcril_scws_channel_status_cb(bip_id,
                                 server_ptr->slot_id,
                                 QCRIL_SCWS_SOCKET_STATE_ESTABLISHED);
  }

  /* If the socket is already connected and has some data in
     the buffer, send the data */
  if (server_ptr->connected_socket[index].connected_sd != -1 &&
      server_ptr->connected_socket[index].buffer_size > 0 &&
      server_ptr->connected_socket[index].buffer_ptr != NULL)
  {
    QCRIL_ASSERT(qcril_scws_data_available_cb);

    qcril_scws_data_available_cb(bip_id,
                                 server_ptr->slot_id,
                                 server_ptr->connected_socket[index].buffer_size,
                                 server_ptr->connected_socket[index].buffer_ptr,
                                 0);

    /* Free the buffer */
    server_ptr->connected_socket[index].buffer_size = 0;
    qcril_free(server_ptr->connected_socket[index].buffer_ptr);
    server_ptr->connected_socket[index].buffer_ptr = NULL;
  }

  return TRUE;
} /* qcril_scws_add_bip_id_to_server_socket */


/*=========================================================================

  FUNCTION:  qcril_scws_create_new_server_socket

===========================================================================*/
/*!
    @brief
    Function that creates a new server socket. This function is also
    responsible for starting the thread for the new socket.

    @return
    None
*/
/*=========================================================================*/
static boolean qcril_scws_create_new_server_socket
(
  qcril_scws_server_socket_type * server_ptr,
  uint16                          local_port,
  qcril_scws_slot_enum_type       slot_id,
  uint32                          bip_id

)
{
  int                res             = 0;
  int                reuse_addr      = 1;
  struct sockaddr_in server_address;

  /* Sanity checks */
  if (server_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid input: NULL server_ptr\n");
    QCRIL_ASSERT(0);
    return FALSE;
  }

  QCRIL_ASSERT(server_ptr->local_port == 0);

  QCRIL_LOG_INFO( "%s, local_port: %d, bip_id 0x%X\n", __FUNCTION__, local_port, bip_id);

  /* Assign the local port and the slot */
  server_ptr->local_port = local_port;
  server_ptr->slot_id    = slot_id;

  /* Initialize the BIP id to the first connected socket. The
     socket descriptor is still invalid */
  server_ptr->connected_socket[0].valid_bip_id = TRUE;
  server_ptr->connected_socket[0].bip_id       = bip_id;
  server_ptr->connected_socket[0].connected_sd = -1;

  /* Create the socket */
  server_ptr->server_sd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_ptr->server_sd == -1)
  {
    QCRIL_LOG_ERROR("%s", "Failure to create the socket");
    memset(server_ptr, 0, sizeof(qcril_scws_server_socket_type));
    return FALSE;
  }

  /* Set socket options */
  setsockopt(server_ptr->server_sd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

  /* Bind the socket to the local port */
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  server_address.sin_port = htons(local_port);
  if (bind(server_ptr->server_sd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0 )
  {
    QCRIL_LOG_ERROR("%s", "Failure to bind the socket");
    close(server_ptr->server_sd);
    memset(server_ptr, 0, sizeof(qcril_scws_server_socket_type));
    return FALSE;
  }

  /* Start listening */
  res = listen(server_ptr->server_sd, QCRIL_SCWS_MAX_CLIENT_SOCKETS);
  if (res == -1)
  {
    QCRIL_LOG_ERROR("%s", "Failure to listen on the socket");
	close(server_ptr->server_sd);
    memset(server_ptr, 0, sizeof(qcril_scws_server_socket_type));
    return FALSE;
  }

  /* Start the thread to execute the select */
  res = pthread_create(&server_ptr->thread_id,
                       NULL,
                       (void*)qcril_scws_socket_thread,
                       (void*)(server_ptr));
  if (res != 0)
  {
    QCRIL_LOG_ERROR("%s", "Failure to start the thread for the new socket");
	close(server_ptr->server_sd);
    memset(server_ptr, 0, sizeof(qcril_scws_server_socket_type));
    return FALSE;
  }

  return TRUE;
} /* qcril_scws_create_new_server_socket */


/*=========================================================================

  FUNCTION:  qcril_scws_open_channel

===========================================================================*/
/*!
    @brief
    Function that handles the open channel coming from the card

    @return
    None
*/
/*=========================================================================*/
boolean qcril_scws_open_channel
(
  uint16                    local_port,
  uint32                    bip_id,
  qcril_scws_slot_enum_type slot_id
)
{
  uint8   i     = 0;

  QCRIL_LOG_INFO( "%s, local_port: %d, bip_id: 0x%X, slot_id: %d \n",
                  __FUNCTION__, local_port, bip_id, slot_id);

  /* Validate parameters */
  if (local_port == 0)
  {
    /* Invalid local port */
    return FALSE;
  }

  QCRIL_SCWS_ENTER_CRITICAL_SECTION;

  /* Check if there is already a socket with the same local port */
  for (i = 0; i < QCRIL_SCWS_MAX_SERVER_SOCKETS; i++)
  {
    if (server_socket[i].local_port != 0 &&
        server_socket[i].local_port == local_port)
    {
      boolean res = FALSE;

      if (server_socket[i].slot_id == slot_id)
      {
        QCRIL_LOG_INFO( "Adding new bip id to existing server, local_port: 0x%x,\n", local_port);
        res = qcril_scws_add_bip_id_to_server_socket(&server_socket[i], bip_id);
      }
      else
      {
        QCRIL_LOG_INFO( "Server port is already busy on other slot, local_port: 0x%x,\n", local_port);
      }

      QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
      return res;
    }
  }

  /* Find an empty spot to open the new server socket */
  for (i = 0; i < QCRIL_SCWS_MAX_SERVER_SOCKETS; i++)
  {
    if (server_socket[i].local_port == 0)
    {
      boolean res = qcril_scws_create_new_server_socket(&server_socket[i], local_port, slot_id, bip_id);

      QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
      return res;
    }
  }

  /* Error opening the socket */
  QCRIL_LOG_ERROR("%s", "Error opening the new server socket");

  QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
  return FALSE;
} /* qcril_scws_open_channel */


/*=========================================================================

  FUNCTION:  qcril_scws_close_channel

===========================================================================*/
/*!
    @brief
    Function that handles the close channel coming from the card

    @return
    None
*/
/*=========================================================================*/
boolean qcril_scws_close_channel
(
  uint32                    bip_id,
  qcril_scws_slot_enum_type slot_id,
  boolean                   close_server
)
{
  uint8     i   = 0;
  uint8     j   = 0;
  uint8     k   = 0;

  QCRIL_LOG_INFO( "%s, bip_id: 0x%X, close_server: %s \n",
                   __FUNCTION__, bip_id, !close_server ? "FALSE" : "TRUE");

  QCRIL_SCWS_ENTER_CRITICAL_SECTION;

  for (i = 0; i < QCRIL_SCWS_MAX_SERVER_SOCKETS; i++)
  {
    if (server_socket[i].local_port == 0)
    {
      continue;
    }
    if (server_socket[i].slot_id != slot_id)
    {
      continue;
    }

    for (j = 0; j < QCRIL_SCWS_MAX_CLIENT_SOCKETS; j++)
    {
      if (server_socket[i].connected_socket[j].valid_bip_id &&
          server_socket[i].connected_socket[j].bip_id == bip_id)
      {
        /* Close the connected socket for that BIP id, if present */
        if (server_socket[i].connected_socket[j].connected_sd != -1)
        {
          qcril_scws_close_socket(&server_socket[i],
                                  server_socket[i].connected_socket[j].connected_sd,
                                  FALSE);
        }

        if (close_server)
        {
          /* Remove the current BIP id */
          server_socket[i].connected_socket[j].valid_bip_id = FALSE;
          server_socket[i].connected_socket[j].bip_id       = 0;

          /* Check if there is any other BIP id associated with this
             server socket. If there are other BIP ids, then the server
             socket needs to remain open */
          for (k = 0; k < QCRIL_SCWS_MAX_CLIENT_SOCKETS; k++)
          {
            if (server_socket[i].connected_socket[k].valid_bip_id)
            {
              QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
              return TRUE;
            }
          }

          /* No more BIP ids... close the server socket (this automatically
             closes all the connected sockets as well */
          qcril_scws_close_socket(&server_socket[i],
                                  server_socket[i].server_sd,
                                  FALSE);
        }
        else
        {
          /* In this case, we need to see if there is any connected
             socket that does not have a valid BIP id in order to start
             transfering that data to the card, using the existing BIP id */
          qcril_scws_socket_closed_switch_to_next(&server_socket[i]);
        }

        QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
        return TRUE;
      }
    }
  }

  QCRIL_LOG_ERROR("%s", "No matching BIP id found to close the socket");

  QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
  return FALSE;
} /* qcril_scws_close_channel */


/*=========================================================================

  FUNCTION:  qcril_scws_send_data

===========================================================================*/
/*!
    @brief
    Function that handles the send data coming from the card

    @return
    None
*/
/*=========================================================================*/
boolean qcril_scws_send_data
(
  uint32                    bip_id,
  qcril_scws_slot_enum_type slot_id,
  const uint8 *             data_ptr,
  uint16                    data_len
)
{
  uint8 i = 0;
  uint8 j = 0;

#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
  boolean switch_socket = FALSE;
#endif /* QCRIL_SCWS_DISABLE_OPTIMIZATION */

  QCRIL_LOG_INFO( "%s, bip_id: 0x%X, data_len: 0x%X \n", __FUNCTION__, bip_id, data_len);

  /* Check parameters */
  if (data_len == 0 || data_ptr == NULL)
  {
    return FALSE;
  }

  QCRIL_SCWS_ENTER_CRITICAL_SECTION;

  /* Search for correct connection */
  for (i = 0; i < QCRIL_SCWS_MAX_SERVER_SOCKETS; i++)
  {
    if (server_socket[i].local_port == 0)
    {
      continue;
    }
    if (server_socket[i].slot_id != slot_id)
    {
      continue;
    }

    QCRIL_LOG_DEBUG("%s, Valid local_port for server_socket[%d]: %d \n",
                    __FUNCTION__, i, server_socket[i].local_port);

    for (j = 0; j < QCRIL_SCWS_MAX_CLIENT_SOCKETS; j++)
    {
      if (server_socket[i].connected_socket[j].valid_bip_id &&
          server_socket[i].connected_socket[j].bip_id == bip_id &&
          server_socket[i].connected_socket[j].connected_sd != -1)
      {
        boolean   result    = FALSE;
        ssize_t   sent_data = 0;

        QCRIL_LOG_DEBUG("Sending data to Client on socket: 0x%X, server_socket[%d].connected_socket[%d] \n",
                        server_socket[i].connected_socket[j].connected_sd, i, j);

#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
        switch_socket = qcril_scws_opt_process_tx(
                          &server_socket[i].connected_socket[j].traffic_analyzer,
                          data_ptr,
                          data_len);
#endif /* QCRIL_SCWS_DISABLE_OPTIMIZATION */

        /* Loop until the entire data is sent */
        while ((sent_data = send(server_socket[i].connected_socket[j].connected_sd, data_ptr, data_len, 0)) != -1)
        {
          /* Check if the entire data was sent */
          if (sent_data >= data_len)
          {
            result = TRUE;
            break;
          }
          if (sent_data < 0)
          {
            /* This is an error... indicate the error and close move the socket to closed */
            QCRIL_LOG_ERROR("Error sending data for: server_socket[%d].connected_socket[%d] \n",
                            i, j);

            result = FALSE;

            qcril_scws_close_socket(&server_socket[i],
                                    server_socket[i].connected_socket[j].connected_sd,
                                    TRUE);

            /* Continue with the next socket data if available */
            qcril_scws_socket_closed_switch_to_next(&server_socket[i]);

            break;
          }

          /* Shift the data and continue sending */
          data_ptr += sent_data;
          data_len -= sent_data;
       }

#ifndef QCRIL_SCWS_DISABLE_OPTIMIZATION
        if (switch_socket)
        {
          qcril_scws_socket_connected_switch_to_next(&server_socket[i], bip_id);
        }
#endif /* QCRIL_SCWS_DISABLE_OPTIMIZATION */

        QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
        return result;
      }
    }
  }

  QCRIL_LOG_ERROR("%s", "No matching BIP id found to send the data");

  QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
  return FALSE;
} /* qcril_scws_send_data */


/*=========================================================================

  FUNCTION:  qcril_scws_data_available_error

===========================================================================*/
/*!
    @brief
    Indicates an error for a previously sent data_available command.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_data_available_error
(
  uint32                        bip_id,
  qcril_scws_slot_enum_type     slot_id
)
{
  uint8 i = 0;
  uint8 j = 0;

  QCRIL_LOG_INFO( "%s, bip_id: 0x%X, slot_id: %d \n",
                  __FUNCTION__, bip_id, slot_id);

  QCRIL_SCWS_ENTER_CRITICAL_SECTION;

  /* Search for correct connection */
  for (i = 0; i < QCRIL_SCWS_MAX_SERVER_SOCKETS; i++)
  {
    if (server_socket[i].local_port == 0)
    {
      continue;
    }
    if (server_socket[i].slot_id != slot_id)
    {
      continue;
    }

    for (j = 0; j < QCRIL_SCWS_MAX_CLIENT_SOCKETS; j++)
    {
      if (server_socket[i].connected_socket[j].valid_bip_id &&
          server_socket[i].connected_socket[j].bip_id == bip_id &&
          server_socket[i].connected_socket[j].connected_sd != -1)
      {
        QCRIL_LOG_DEBUG("Closing connected_socket: server_socket[%d].connected_socket[%d]\n",
                        i, j);

        qcril_scws_close_socket(&server_socket[i],
                                server_socket[i].connected_socket[j].connected_sd,
                                TRUE);

        /* Continue with the next socket data if available */
        qcril_scws_socket_closed_switch_to_next(&server_socket[i]);

        QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
        return;
      }
    }
  }

  QCRIL_LOG_ERROR("%s", "No matching BIP id found to close the socket");

  QCRIL_SCWS_LEAVE_CRITICAL_SECTION;

} /* qcril_scws_data_available_error */


/*=========================================================================

  FUNCTION:  qcril_scws_card_error

===========================================================================*/
/*!
    @brief
    Notifies the agent of a card error, so that server socket can be cleaned.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_card_error
(
  qcril_scws_slot_enum_type slot_id
)
{
  uint8 i = 0;
  uint8 j = 0;

  QCRIL_LOG_INFO( "%s, slot_id: 0x%X \n", __FUNCTION__, slot_id);

  QCRIL_SCWS_ENTER_CRITICAL_SECTION;

  /* loop thru all server sockets on the slot and close
     all of them */
  for (i = 0; i < QCRIL_SCWS_MAX_SERVER_SOCKETS; i++)
  {
    if (server_socket[i].local_port == 0)
    {
      continue;
    }
    if (server_socket[i].slot_id != slot_id)
    {
      continue;
    }

    /* Close the server socket, this automatically closes all connected sockets as well */
    qcril_scws_close_socket(&server_socket[i],
                            server_socket[i].server_sd,
                            FALSE);
  }

  QCRIL_SCWS_LEAVE_CRITICAL_SECTION;
} /* qcril_scws_card_error */
