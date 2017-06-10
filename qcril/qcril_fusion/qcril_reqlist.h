/*!
  @file
  qcril_reqlist.h

  @brief
  reqlist is a general purpose module for the RIL to manage outstanding
  RIL_REQUESTs.

*/

/*===========================================================================

  Copyright (c) 2009 - 2010 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_reqlist.h#3 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
12/08/09   sb      Added function to update subsystem information.
04/05/09   fc      Cleanup log macros.
01/26/08   fc      Logged assertion info.
09/11/08   asn     Added data support
05/08/08   fc      Changes to linked list implementation.
05/05/08   da      Initial framework.


===========================================================================*/

#ifndef QCRIL_REQLIST_H
#define QCRIL_REQLIST_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "IxErrno.h"
#include "ril.h"
#include "qcrili.h"
#include "qcril_cm.h"
#include "qcril_log.h"


/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/* ReqList entry states */
typedef enum
{
  QCRIL_REQ_FREE                      = 0x0001, /*!< Request is not waiting for RPC callback or AMSS events */
  QCRIL_REQ_AWAITING_CALLBACK         = 0x0002, /*!< Request is waiting for RPC callback */
  QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS = 0x0004, /*!< Request is waiting for AMSS event. */
  QCRIL_REQ_COMPLETED_SUCCESS         = 0x0008, /*!< Request completed, success */
  QCRIL_REQ_COMPLETED_FAILURE         = 0x0010, /*!< Request completed, failure */
  QCRIL_REQ_BLOCKED                   = 0x0012  /*!< Request blocked due to flow control */
} qcril_req_state_e_type;

/* For simplicity just use a C union to hold the subsystem specific data
   that is to be kept in the reqlist */

typedef struct
{
  /* CM subsystem specific data for reqlist */
  qcril_cm_req_info_type info;
} qcril_reqlist_cm_type;

typedef struct
{
  /* SMS subsystem specific data for reqlist */
  boolean valid_client_msg_ref;
  wms_message_number_type client_msg_ref;
  wms_message_number_type wms_msg_ref;
} qcril_reqlist_sms_type;

typedef struct
{
  /* DATA SERVICE subsystem specific data for reqlist */
  void *info;
} qcril_reqlist_ds_type;

typedef struct
{
  /* UIM subsystem specific data for reqlist */
  int dummy;
} qcril_reqlist_uim_type;

typedef struct
{
  /* Other subsystem specific data for reqlist */
  int dummy;
} qcril_reqlist_other_type;

typedef union
{
  qcril_reqlist_cm_type cm;
  qcril_reqlist_sms_type sms;
  qcril_reqlist_ds_type ds;
  qcril_reqlist_uim_type uim;
  qcril_reqlist_other_type other;
} qcril_reqlist_u_type;

typedef struct
{
  uint16 req_id;                                           /* Reqlist entry ID */
  RIL_Token t;                                             /* Token ID */
  int request;                                             /* RIL Request or QCRIL Event */
  boolean valid_sub_id;                                    /* Indicates whether sub id is valid */
  uint32 sub_id;                                           /* Subsystem ID (e.g Call ID, SS Ref, Invoke ID, Supplementary Service Type
                                                              or event ID) */
  qcril_req_state_e_type state[ QCRIL_MAX_MODEM_ID ];      /* State of ReqList Entry */
  qcril_evt_e_type pending_event_id[ QCRIL_MAX_MODEM_ID ]; /* Pending Event ID */
  qcril_reqlist_u_type sub;                                /*!< Union of subsystem specific information */
} qcril_reqlist_public_type;


/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/


void qcril_reqlist_init( void );

void qcril_reqlist_default_entry( RIL_Token t, int request, qcril_modem_id_e_type modem_id, qcril_req_state_e_type state, 
                                  qcril_evt_e_type pending_event_id, qcril_reqlist_u_type *sub_ptr,
                                  qcril_reqlist_public_type *req_ptr );

IxErrnoType qcril_reqlist_new( qcril_instance_id_e_type instance_id, qcril_reqlist_public_type *entry_ptr ); 

void qcril_reqlist_free_all( void );

IxErrnoType qcril_reqlist_free( qcril_instance_id_e_type instance_id, RIL_Token t );

IxErrnoType qcril_reqlist_update_state( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, RIL_Token t, 
                                        qcril_req_state_e_type state );

IxErrnoType qcril_reqlist_update_pending_event_id( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, 
                                                   RIL_Token t, qcril_evt_e_type pending_event_id );

IxErrnoType qcril_reqlist_update_sub_id( qcril_instance_id_e_type instance_id, RIL_Token t, uint32 sub_id );

IxErrnoType qcril_reqlist_query( qcril_instance_id_e_type instance_id, RIL_Token t, qcril_reqlist_public_type *info_ptr );

IxErrnoType qcril_reqlist_query_by_request( qcril_instance_id_e_type instance_id, int request,  qcril_reqlist_public_type *info_ptr );

IxErrnoType qcril_reqlist_query_by_req_id( uint16 req_id, qcril_instance_id_e_type *instance_id_ptr, 
                                           qcril_reqlist_public_type *info_ptr );

IxErrnoType qcril_reqlist_query_by_event( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, 
                                          qcril_evt_e_type pending_event_id, qcril_reqlist_public_type *info_ptr );

IxErrnoType qcril_reqlist_query_by_sub_id( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, uint32 sub_id, qcril_reqlist_public_type *info_ptr );

IxErrnoType qcril_reqlist_query_by_event_and_sub_id( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, 
                                                     uint32 pending_event_id, uint32 sub_id, qcril_reqlist_public_type *info_ptr );

IxErrnoType qcril_reqlist_query_by_event_all_states( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, 
                                                     qcril_evt_e_type pending_event_id, qcril_reqlist_public_type *info_ptr ); 

IxErrnoType qcril_reqlist_query_by_event_and_sub_id_all_states( qcril_instance_id_e_type instance_id, 
                                                                qcril_modem_id_e_type modem_id, uint32 pending_event_id, 
                                                                uint32 sub_id, qcril_reqlist_public_type *info_ptr );

IxErrnoType qcril_reqlist_update_sub_info( qcril_instance_id_e_type instance_id, RIL_Token t, qcril_reqlist_u_type *sub_ptr ); 

IxErrnoType qcril_reqlist_complete_all_amss_events( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, RIL_Token t,
                                                    qcril_req_state_e_type state, qcril_modem_ids_list_type *modem_ids_done_list,
                                                    IxErrnoType *result );
#endif /* QCRIL_REQLIST_H */
