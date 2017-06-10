#ifndef QCRIL_SCWS_H
#define QCRIL_SCWS_H

/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

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
01/29/13   yt      Support for third SIM card slot
08/09/12   sc      Added optimizations to improve on socket switching delay.
04/20/11   mib     Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include "qcril_scws_opt.h"
#include "qcril_log.h"


/*===========================================================================

                           DEFINES

===========================================================================*/
#define QCRIL_SCWS_MAX_SERVER_SOCKETS      3
#define QCRIL_SCWS_MAX_CLIENT_SOCKETS     15
#define QCRIL_SCWS_TEMP_BUFFER_SIZE      500


/*===========================================================================

                           TYPES

===========================================================================*/

/* -----------------------------------------------------------------------------
   TYPEDEF:     QCRIL_SCWS_SOCKET_STATE_ENUM_TYPE

   DESCRIPTION:
     This is the type that indicates the socket state.
-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_SCWS_SOCKET_STATE_CLOSED,
  QCRIL_SCWS_SOCKET_STATE_LISTEN,
  QCRIL_SCWS_SOCKET_STATE_ESTABLISHED
} qcril_scws_socket_state_enum_type;


/* -----------------------------------------------------------------------------
   TYPEDEF:     QCRIL_SCWS_SLOT_ENUM_TYPE

   DESCRIPTION:
     Slot of the card.
-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_SCWS_SLOT_1,
  QCRIL_SCWS_SLOT_2,
  QCRIL_SCWS_SLOT_3
} qcril_scws_slot_enum_type;


/* -----------------------------------------------------------------------------
   TYPEDEF:     QCRIL_SCWS_SEND_DATA_CB_TYPE

   DESCRIPTION:
     This is the callback used by the SCWS Agent to notify the modem when
     data is received from the client (browser) on the socket.
-------------------------------------------------------------------------------*/
typedef void qcril_scws_data_available_cb_type(
  uint32                    bip_id,
  qcril_scws_slot_enum_type slot_id,
  uint16                    data_len,
  uint8 *                   data_ptr,
  uint16                    remaining_data_len);


/* -----------------------------------------------------------------------------
   TYPEDEF:     QCRIL_SCWS_CHANNEL_STATUS_CB_TYPE

   DESCRIPTION:
     This is the callback used by the SCWS Agent to notify the modem when
     there is a change in the channel status.
-------------------------------------------------------------------------------*/
typedef void qcril_scws_channel_status_cb_type(
  uint32                            bip_id,
  qcril_scws_slot_enum_type         slot_id,
  qcril_scws_socket_state_enum_type socket_state);


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_SCWS_CONNECTED_SOCKET_TYPE

   DESCRIPTION:
     This structure contains the list of connected sockets. The socket descriptor
     can be -1 in case the socket is not in use.
-------------------------------------------------------------------------------*/
typedef struct
{
  int                                     connected_sd;
  boolean                                 valid_bip_id;
  uint32                                  bip_id;
  uint16                                  buffer_size;
  uint8                                 * buffer_ptr;
  qcril_scws_opt_traffic_analyzer_type    traffic_analyzer;
} qcril_scws_connected_socket_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_SCWS_SERVER_SOCKET_TYPE

   DESCRIPTION:
     This structure contains the value of a server socket.
-------------------------------------------------------------------------------*/
typedef struct
{
  uint16                            local_port;
  pthread_t                         thread_id;
  fd_set                            fd_set;
  int                               server_sd;
  qcril_scws_slot_enum_type         slot_id;

  qcril_scws_connected_socket_type  connected_socket[QCRIL_SCWS_MAX_CLIENT_SOCKETS];
} qcril_scws_server_socket_type;



/*=========================================================================

  FUNCTION:  qcril_scws_initalize

===========================================================================*/
/*!
    @brief
    Initializes the SCWS Agent, indicating the callback functions.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_initalize(
  qcril_scws_data_available_cb_type  * data_available_cb,
  qcril_scws_channel_status_cb_type  * channel_status_cb);


/*=========================================================================

  FUNCTION:  qcril_scws_deinitalize

===========================================================================*/
/*!
    @brief
    De-initializes the SCWS Agent, freeing all allocated resources.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_deinitalize(void);


/*=========================================================================

  FUNCTION:  qcril_scws_open_channel

===========================================================================*/
/*!
    @brief
    Open channel from the UICC card.

    @return
    boolean: indicates if the command is successfull or not
*/
/*=========================================================================*/
boolean qcril_scws_open_channel(
  uint16                    local_port,
  uint32                    bip_id,
  qcril_scws_slot_enum_type slot_id);


/*=========================================================================

  FUNCTION:  qcril_scws_close_channel

===========================================================================*/
/*!
    @brief
    Close channel from the UICC card.

    @return
    boolean: indicates if the command is successfull or not
*/
/*=========================================================================*/
boolean qcril_scws_close_channel(
  uint32                    bip_id,
  qcril_scws_slot_enum_type slot_id,
  boolean                   close_server);


/*=========================================================================

  FUNCTION:  qcril_scws_send_data

===========================================================================*/
/*!
    @brief
    Send data from the UICC card.

    @return
    boolean: indicates if the command is successfull or not
*/
/*=========================================================================*/
boolean qcril_scws_send_data(
  uint32                    bip_id,
  qcril_scws_slot_enum_type slot_id,
  const uint8 *             data_ptr,
  uint16                    data_len);


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
void qcril_scws_data_available_error(
  uint32                    bip_id,
  qcril_scws_slot_enum_type slot_id);


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
void qcril_scws_card_error(
  qcril_scws_slot_enum_type slot_id);


#endif /* QCRIL_SCWS_H */

