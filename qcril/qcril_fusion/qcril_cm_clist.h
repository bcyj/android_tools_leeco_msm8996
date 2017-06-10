/*!
  @file
  qcril_cm_clist.h

  @brief
  Encapsulates information related to active calls.

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_cm_clist.h#8 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
11/05/09   sb      Added support to replace call list entry info.
05/14/09   pg      Added support for CDMA phase II under FEATURE_MULTIMODE_ANDROID_2.
                   Mainlined FEATURE_MULTIMODE_ANDROID.
04/28/09   fc      Standardize xxx_enum_type to xxx_e_type.
04/05/09   fc      Cleanup log macros.
01/26/08   fc      Logged assertion info.
12/16/08   fc      Change to store call mode used in mobile originated or
                   mobile terminated call.
12/08/08   pg      Added support to pass CDMA voice privacy mode to RILD.
05/08/08   fc      First cut implementation.
05/05/08   da      Initial framework.


===========================================================================*/

#ifndef QCRIL_CM_CLIST_H
#define QCRIL_CM_CLIST_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include "cm.h"
#include "ril.h"
#include "qcril_log.h"
#include "qcril_cmi.h"


/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#define QCRIL_CM_CLIST_REPORT_CALL_STATE_CHANGED( call_type, call_state ) \
  ( ( ( call_type == CM_CALL_TYPE_VOICE ) || ( call_type == CM_CALL_TYPE_EMERGENCY ) || ( call_type == CM_CALL_TYPE_TEST ) || \
      ( call_type == CM_CALL_TYPE_STD_OTASP ) || ( call_type == CM_CALL_TYPE_NON_STD_OTASP ) ) &&                             \
    ( ( call_state == QCRIL_CM_CLIST_STATE_DIALING ) || ( call_state == QCRIL_CM_CLIST_STATE_INCOMING ) ||                    \
      ( call_state == QCRIL_CM_CLIST_STATE_WAITING ) || ( call_state == QCRIL_CM_CLIST_STATE_ALERTING ) ||                    \
      ( call_state == QCRIL_CM_CLIST_STATE_ACTIVE ) || ( call_state == QCRIL_CM_CLIST_STATE_IDLE ) ) )                                                   

#define QCRIL_CM_CLIST_CALL_TYPE_IS_VOICE( call_type ) \
  ( ( call_type == CM_CALL_TYPE_VOICE ) || ( call_type == CM_CALL_TYPE_EMERGENCY ) || ( call_type == CM_CALL_TYPE_TEST ) || \
    ( call_type == CM_CALL_TYPE_STD_OTASP ) || ( call_type == CM_CALL_TYPE_NON_STD_OTASP ) )

#define QCRIL_CM_CLIST_CONN_INDEX_NONE 0

/*! @brief Call State
*/
typedef enum
{
  QCRIL_CM_CLIST_STATE_IDLE     = 0x00,
  QCRIL_CM_CLIST_STATE_ACTIVE   = 0x01,
  QCRIL_CM_CLIST_STATE_HOLDING  = 0x02,
  QCRIL_CM_CLIST_STATE_DIALING  = 0x04,
  QCRIL_CM_CLIST_STATE_ALERTING = 0x08,
  QCRIL_CM_CLIST_STATE_INCOMING = 0x10,
  QCRIL_CM_CLIST_STATE_WAITING  = 0x20,
  QCRIL_CM_CLIST_STATE_SETUP    = 0x40
} qcril_cm_clist_state_e_type;

/*! @brief Call Info
*/
typedef struct
{
  qcril_modem_id_e_type          modem_id;   /* Indicates which modem the call was setup */ 
  uint32                         conn_index; /* GSM index for use with, e.g. AT+CHLD */
  cm_call_id_type                call_id;    /* Call ID */
  cm_call_type_e_type            call_type;  /* Type of call e.g. Voice */
  cm_call_direction_e_type       direction;  /* Incoming or Outgoing */
  qcril_cm_clist_state_e_type    prev_state; /* Previous state of the call */
  qcril_cm_clist_state_e_type    state;      /* State of the call */
  cm_call_mode_info_e_type       call_mode;  /* Mode that the incoming or outgoing call started */
  cm_als_line_e_type             line;       /* ALS line indicator */
  cm_num_s_type                  num;        /* Phone number */
  boolean                        answered;   /* Indicates if the incoming call has been answered or not */
  boolean                        is_private; /* Voice privacy mode */
  char                           name[ CM_MAX_ALPHA_TAG_CHARS ]; /* Remote party name */
  uint8                          name_presentation; /* Remote party name presentation */
  boolean                        local_ringback;/* Indicates if the local ringback is set or not for MO call */
  boolean                        is_uus;/* Indicates if it is a MT UUS call*/
  cm_call_event_user_data_s_type uus_data; /* MT UUS Data */
} qcril_cm_clist_public_type;

/*! @brief Call IDs List
*/
typedef struct
{
  uint32 num_of_call_ids;                               /* Number of call IDs in the list */
  cm_call_mode_info_e_type call_mode[ CM_CALL_ID_MAX ]; /* Mode that the incoming or outgoing call started */
  cm_call_id_type call_id[ CM_CALL_ID_MAX ];            /* Call ID */
  boolean answered[ CM_CALL_ID_MAX ];                   /* Indicates whether the call has been answered */
  qcril_modem_id_e_type modem_id[ CM_CALL_ID_MAX ];     /* Modem ID */
} qcril_cm_clist_call_ids_list_type;

/*! @brief Call Info List
*/
typedef struct
{
  uint32 num_of_calls;                               /* Number of calls in the list */
  boolean in_mpty;                                   /* Indicates whether entries with CM_CALL_TYPE_VOICE as call type 
                                                        and QCRIL_CM_STATE_ACTIVE asstate in mpty or not */
  qcril_cm_clist_public_type info[ CM_CALL_ID_MAX ]; /* Call info */
} qcril_cm_clist_call_info_list_type;


/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

void qcril_cm_clist_init( void );

IxErrnoType qcril_cm_clist_new( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,
                                const cm_mm_call_info_s_type *call_info_ptr, qcril_cm_clist_state_e_type call_state, 
                                qcril_cm_call_event_user_data_s_type *uus_ptr, boolean *unsol_call_state_changed_ptr );

IxErrnoType qcril_cm_clist_replace( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,
                                    cm_call_state_e_type call_state, cm_call_id_type call_id, uint32 conn_index, 
                                    const cm_mm_call_info_s_type *call_info_ptr, 
                                    boolean *unsol_call_state_changed_ptr );

IxErrnoType qcril_cm_clist_incoming ( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, cm_call_id_type call_id, 
                                      qcril_cm_call_event_user_data_s_type *uus_ptr, boolean *unsol_call_state_changed_ptr );

IxErrnoType qcril_cm_clist_answer( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, cm_call_id_type call_id );

IxErrnoType qcril_cm_clist_alert( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, cm_call_id_type call_id,
                                  boolean *unsol_call_state_changed_ptr );

IxErrnoType qcril_cm_clist_connect( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, cm_call_id_type call_id,
                                    boolean *unsol_call_state_changed_ptr );

IxErrnoType qcril_cm_clist_manage( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,
                                   const active_calls_list_s_type *active_calls_list_ptr, boolean *unsol_call_state_changed_ptr );

IxErrnoType qcril_cm_clist_free( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, cm_call_id_type call_id, 
                                 boolean *unsol_call_state_changed_ptr );

void qcril_cm_clist_free_all( void );

IxErrnoType qcril_cm_clist_update_voice_privacy_mode( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, 
                                                      cm_call_id_type call_id, boolean privacy_mode, boolean *unsol_call_state_changed_ptr );

IxErrnoType qcril_cm_clist_update_name( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,  
                                        cm_call_id_type call_id, char * name );

IxErrnoType qcril_cm_clist_update_name_presentation( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, 
                                                     cm_call_id_type call_id, int  name_presentation );

IxErrnoType qcril_cm_clist_update_number( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,
                                          cm_call_id_type call_id, cm_num_s_type * number );

IxErrnoType qcril_cm_clist_update_local_ringback( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, 
                                                  cm_call_id_type call_id, boolean is_local_ringback );

boolean qcril_cm_clist_voice_is_emergency( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, cm_call_id_type call_id );

boolean qcril_cm_clist_is_uus_call( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, cm_call_id_type call_id );

boolean qcril_cm_clist_is_local_ringback( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, cm_call_id_type call_id ); 

boolean qcril_cm_clist_voice_call_is_mpty( qcril_instance_id_e_type instance_id );

IxErrnoType qcril_cm_clist_query_voice_call_id( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,  
                                                cm_call_id_type call_id, qcril_cm_clist_public_type *info_ptr );

IxErrnoType qcril_cm_clist_query_call_id( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, cm_call_id_type call_id, 
                                          qcril_cm_clist_public_type *clist_info_ptr );

IxErrnoType qcril_cm_clist_query_by_conn_index( qcril_instance_id_e_type instance_id, uint32 conn_index, 
                                                qcril_cm_clist_public_type *info_ptr );

void qcril_cm_clist_query_voice_call_ids_list_by_state( qcril_instance_id_e_type instance_id, uint32 state_mask, 
                                                        qcril_cm_clist_call_ids_list_type *call_ids_list_ptr );

void qcril_cm_clist_query_call_info_list( qcril_instance_id_e_type instance_id, qcril_cm_clist_call_info_list_type *info_list_ptr );

#endif /* QCRIL_CM_CLIST_H */
