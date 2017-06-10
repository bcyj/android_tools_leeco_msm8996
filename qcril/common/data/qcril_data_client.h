/*!
  @file
  qcril_data_client.h

  @brief
  Provides client API for other QCRIL modules.

*/

/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved

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
01/16/12   ar     Initial version

===========================================================================*/

#ifndef QCRIL_DATA_CLIENT_H
#define QCRIL_DATA_CLIENT_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "qcril_data_defs.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#define QCRIL_DATA_SUCCESS  (0)
#define QCRIL_DATA_FAILURE  (-1)

#define QCRIL_DATA_MAX_CLIENTS_SUPPORTED  (1)
#define QCRIL_DATA_CLIENT_HNDL_INVALID    (0xFFFFFFFF)

/* Type definitions */
typedef unsigned int  qcril_data_hndl_t;
typedef void          qcril_data_evt_payload_t;

/* Event notifications */
typedef enum
{
  QCRIL_DATA_EVT_CALL_CONNECTED,
  QCRIL_DATA_EVT_CALL_PHYSLINK_UP,
  QCRIL_DATA_EVT_CALL_PHYSLINK_DOWN,
  QCRIL_DATA_EVT_CALL_RELEASED,
  QCRIL_DATA_EVT_MAX
} qcril_data_net_evt_t;

/* Client event notification callback function */
typedef void (*qcril_data_net_ev_cb)( qcril_data_hndl_t         hndl,
                                      void                     *user_data,
                                      qcril_data_net_evt_t      evt,
                                      qcril_data_evt_payload_t *payload );

/* Active call information record */
typedef struct qcril_data_active_call_info_s
{
  int                   call_id;

  RIL_RadioTechnology   radioTech;                             /* Technology */

  qcril_data_addr_string_t     address;                        /* IPv4 & IPv6 */

  char            apn[ DS_CALL_INFO_APN_MAX_LEN + 1 ];         /* Access Point Name */

  char            dev_name[ DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1]; /* Network Interface */

  /* CALL_INACTIVE / CALL_ACTIVE_PHYSLINK_DOWN / CALL_ACTIVE_PHYSLINK_UP */
  int             active;                                      /* State of the data call */

} qcril_data_active_call_info_t;

#define QCRIL_DATA_MAX_CALL_RECORDS (MAX_CONCURRENT_UMTS_DATA_CALLS)


/*===========================================================================

                         GLOBAL VARIABLES

===========================================================================*/



/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_data_client_register

===========================================================================*/
/*!
    @brief
    Used to register client callback function and cookie.  Each client
    will be allocated a unique opaque client handle.

    @return
    qcril_data_client_t if successful,
    QCRIL_DATA_CLIENT_HNDL_INVALID otherwise
*/
/*=========================================================================*/
qcril_data_hndl_t qcril_data_client_register
(
  qcril_data_net_ev_cb  cb_fn,
  void                 *user_data
);


/*===========================================================================

  FUNCTION:  qcril_data_client_release

===========================================================================*/
/*!
    @brief
    Used to release client registration.

    @return
    None
*/
/*=========================================================================*/
void qcril_data_client_release
(
  qcril_data_hndl_t   hndl
);



/*===========================================================================

  FUNCTION:  qcril_data_get_active_calls

===========================================================================*/
/*!
    @brief
    Used to query the current acive calls within QCRIL-Data module.
    The caller should pass a call_list array of dimension
    QCRIL_DATA_MAX_CALL_RECORDS.  If call_list is NULL, only the
    num_calls is updated.

    @return
    QCRIL_DATA_SUCCESS on successful operation,
    QCRIL_DATA_FAILURE otherwise
*/
/*=========================================================================*/
int qcril_data_get_active_calls
(
  qcril_data_hndl_t              hndl,
  unsigned int                  *num_calls,
  qcril_data_active_call_info_t *call_list
);

/*===========================================================================

  FUNCTION:  qcril_data_client_notify

===========================================================================*/
/*!
    @brief
    Invoke the client's registered callback (if any) to pass the
    specified event and payload.

    @return
    QCRIL_DATA_SUCCESS on successful operation,
    QCRIL_DATA_FAILURE otherwise
*/
/*=========================================================================*/
int qcril_data_client_notify
(
  qcril_data_net_evt_t      evt,
  qcril_data_evt_payload_t *payload
);

#endif /* QCRIL_DATA_CLIENT_H */
