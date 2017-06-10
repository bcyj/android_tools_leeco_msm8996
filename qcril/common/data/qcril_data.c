/*!
  @file
  qcril_data.c

  @brief
  Handles RIL requests for DATA services.

*/

/*===========================================================================

  Copyright (c) 2009 - 2011,2014 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_data.c#17 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
06/26/09   fc      Fixed the issue of bogus RIL Request reported in call flow
                   log packet.
05/29/09   fc      Renamed functions.
05/21/09   sm      Passes auth pref to dss from ril
05/14/09   pg      Changed NULL APN handling in data call setup.
                   Mainlined FEATURE_MULTIMODE_ANDROID.
04/05/09   fc      Cleanup log macros.
03/10/09   pg      Fixed multi-mode data call.
12/29/08   fc      Fixed wrong size issue being reported for the response
                   payload of RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE.
12/23/08   asn     code reorg and IP addr fix
12/23/08   asn     Added handling on MO call-end, IP addr issue, APN trunc issue
12/15/08   asn     Fixed call teardown and improved stability
12/08/08   pg      Added multi-mode data call hook up.
11/18/08   fc      Changes to avoid APN string being truncated.
11/14/08   sm      Added temp CDMA data support.
08/29/08   asn     Added data support
08/08/08   asn     Initial version

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <stdio.h>
#include <linux/if.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include "qcril_datai.h"
#include "qcril_reqlist.h"
#include "qcril_data.h"

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#ifndef PLATFORM_LTK
#undef ASSERT
#define ASSERT( xx_exp ) ((void)0)
#endif /* PLATFORM_LTK */

#define MAX_CONCURRENT_UMTS_DATA_CALLS DS_MAX_DATA_CALLS /* bounded as [0, 255] */

#define QCRIL_DATA_IP_FAMILY_IPV4  0

/* Error codes */
#define SUCCESS  0
#define FAILURE -1

/* Booleans */
#define TRUE  1
#define FALSE 0

/* Map RIL commands to local names */
#define DS_RIL_REQ_INVALID              0
#define DS_RIL_REQ_ACT_DATA_CALL        RIL_REQUEST_SETUP_DATA_CALL
#define DS_RIL_REQ_DEACT_DATA_CALL      RIL_REQUEST_DEACTIVATE_DATA_CALL
#define DS_RIL_REQ_GET_CALL_LIST        RIL_REQUEST_DATA_CALL_LIST
#define DS_RIL_REQ_GET_LAST_FAIL_CAUSE  RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE
#define DS_RIL_IND_CALL_LIST_CHANGED    RIL_UNSOL_DATA_CALL_LIST_CHANGED

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/*! @brief Typedef variables internal to module qcril_data.c
*/
typedef struct
{
  unsigned char rflag;  /* determines RIL Token validity */
  RIL_Token     rt;     /* valid RIL token if flag is set */
  int           request;
  void         *data;
  void         *self;
} qcril_data_net_cb_info_t;

/*! @brief Typedef variables internal to module qcril_data.c
*/
typedef struct
{
  dsi_net_evt_t evt;
  void         *data;
  int           data_len;
  void         *self;
} qcril_data_event_data_t;

typedef struct
{
  /* Index to this and call table */
  unsigned int                index;

  int                         cid;

  qcril_instance_id_e_type instance_id;

  qcril_modem_id_e_type    modem_id;

  RIL_Token                   pend_tok;

  int                         pend_req;


  dsi_hndl_t                  dsi_hndl;

  /* flag to indicate whether dev_name and call_index is valid */
  unsigned char               info_flg;

  dsi_call_info_t             call_info;

  void                        *self;

} qcril_data_call_info_tbl_type;

static qcril_data_call_info_tbl_type info_tbl[ MAX_CONCURRENT_UMTS_DATA_CALLS ];

/*
  This is used to store last call end reason for responding
  to RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE. Meant for Single PDP only.
  Functionality fails for multiple PDP using QCRIL.
*/
static int last_call_end_reason = CALL_FAIL_ERROR_UNSPECIFIED;

typedef struct
{
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
  /* Response to setup data call request */
  RIL_Data_Call_Response_v6  setup_rsp;
#else
  /* Response to setup data call request */
  struct
  {
    char *cid;
    char *dev_name;
    char *ip_addr;
  }setup_rsp;
#endif /* (RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6) */

  /* Response to Request for last call fail cause */
  int   cause_code;

  /* Response to get call list request */
  RIL_Data_Call_Response_v6 *list;

  /* size */
  size_t size;

} qcril_data_call_response_type;

#define CALL_INACTIVE  0
#define CALL_ACTIVE_PHYSLINK_DOWN    1
#define CALL_ACTIVE_PHYSLINK_UP      2

typedef struct
{
  int     cid;
  int     active;
  char    type[ DS_CALL_INFO_TYPE_MAX_LEN + 1];
  char    apn[ DS_CALL_INFO_APN_MAX_LEN + 1 ];
  char    address[ DS_CALL_INFO_IP_ADDR_MAX_LEN + 1 ];
} qcril_data_context_response_type;

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
static RIL_Data_Call_Response_v6 call_tbl[ MAX_CONCURRENT_UMTS_DATA_CALLS ] =
{
  { PDP_FAIL_NONE,  0, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE,  1, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE,  2, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE,  3, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE,  4, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE,  5, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE,  6, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE,  7, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE,  8, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE,  9, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 10, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 11, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 12, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 13, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 14, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 15, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 16, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 17, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 18, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  },
  { PDP_FAIL_NONE, 19, CALL_INACTIVE, '\0', '\0', '\0', '\0', '\0'  }
};
#else /*((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))*/
static RIL_Data_Call_Response_v6 call_tbl[ MAX_CONCURRENT_UMTS_DATA_CALLS ] =
{
  { 0, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED},
  { 1, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 2, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 3, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 4, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 5, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 6, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 7, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 8, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 9, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 10, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 11, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 12, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 13, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 14, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 15, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 16, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 17, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 18, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED },
  { 19, CALL_INACTIVE, '\0', '\0', '\0', RADIO_TECH_UNKNOWN, PDP_FAIL_ERROR_UNSPECIFIED }
};
#endif /*((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))*/

/* this mutex protects call tbl */
pthread_mutex_t call_tbl_mutex;

/* this mutex protects info tbl */
pthread_mutex_t info_tbl_mutex;

/* gloabl API lock*/
pthread_mutex_t qcril_data_global_mutex;

/* global dormancy indication status */
static qcril_data_dormancy_ind_switch_type global_dorm_ind;

#define VALIDATE_LOCAL_DATA_OBJ( ptr ) \
    ( ( (ptr) != NULL ) &&  ( (ptr)->self == (void *)(ptr) ) )

#define IPCONV( ip_addr , t ) ( ( ip_addr >> (24 - 8 * t)) & 0xFF)

#define GET_DEV_INSTANCE_FROM_NAME(index)   info_tbl[index].call_info.dev_name[QCRIL_MAX_DEV_NAME_SIZE - 1] - '0'


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/
static void qcril_data_response_success
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request,
  void *response,
  size_t response_len
);

static void qcril_data_response_generic_failure
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request
);

static int qcril_data_get_call_info
(
  const qcril_data_call_info_tbl_type *const info_tbl_ptr,
  dsi_call_info_t                     *const call_info_ptr
);

static void qcril_data_unsol_call_list_changed
(
  qcril_instance_id_e_type instance_id,
  void                        *response,
  size_t                      response_len
);

static int qcril_data_util_conv_ip_addr
(
  const dsi_param_info_t  *const info,
  dsi_param_info_t        *const ip_addr_info /* output param */
);

static void qcril_data_util_buffer_dump
(
  const dsi_param_info_t  *const info
);

static void qcril_data_cleanup_call_state
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
);

static int qcril_data_util_is_req_pending
(
  const qcril_data_call_info_tbl_type *,
  unsigned int *
);

static void qcril_data_util_fill_call_params
(
  qcril_data_call_info_tbl_type *
);

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
static void qcril_data_util_update_call_state
(
  qcril_data_call_info_tbl_type *,
  int,
  int
);
#else
static void qcril_data_util_update_call_state
(
  qcril_data_call_info_tbl_type *,
  int
);
#endif /*((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))*/

static void qcril_data_util_create_setup_rsp
(
  qcril_data_call_info_tbl_type *,
  qcril_data_call_response_type *
);


#define QCRIL_MAX_DEV_NAME_SIZE             6 /*Length here is 6 as dev names are assumed to be rmnet[0,1,2]*/
#define QCRIL_DATA_MAX_DEVS                 3

typedef char  qcril_data_go_dormant_params_type[QCRIL_MAX_DEV_NAME_SIZE];

/* Go Dormant*/
void qcril_data_process_qcrilhook_go_dormant
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
);


/*===========================================================================

                                FUNCTIONS

===========================================================================*/



/*===========================================================================

  FUNCTION:  qcril_data_net_cb

===========================================================================*/
/*!
    @brief
    Called on network events from Data Services (DSI). This routine calls
    qcril_process_event to let QCRIL call the appropriate event/cmd handler
    for event/cmd.
    Note: do not assume the context in which handler will be called


    @return
    None.
*/
/*=========================================================================*/
void qcril_data_net_cb(
  dsi_hndl_t     dsi_hndl,
  void          *user_data,
  dsi_net_evt_t  net_evt
)
{
  qcril_reqlist_public_type      reqlist_info;
  qcril_data_event_data_t       *evt;
  qcril_data_call_info_tbl_type *info_tbl_ptr;

  QCRIL_LOG_DEBUG("%s", "qcril_data_net_cb: ENTRY");

  /* Input Validation, user data was pointer to info_tbl*/
  info_tbl_ptr = ( qcril_data_call_info_tbl_type *) user_data;
  if ( ( !VALIDATE_LOCAL_DATA_OBJ( info_tbl_ptr ) ) ||
       ( dsi_hndl != info_tbl_ptr->dsi_hndl ) )
  {
    QCRIL_LOG_ERROR( "invalid arg, user_data [%#x], dsi_hndl [%#x], cb_data->dsi_hndl [%#x]",
                     (unsigned int)user_data, (unsigned int)dsi_hndl,
                     (unsigned int)info_tbl_ptr->dsi_hndl );
    goto err_bad_input;
  }
  QCRIL_DS_LOG_DBG_MEM( "info_tbl", info_tbl_ptr );
  QCRIL_LOG_DEBUG( "dsi net evt [%d], info_tbl index [%d], pend RIL Token [%d]",
                   net_evt, info_tbl_ptr->index,
                   qcril_log_get_token_id( info_tbl_ptr->pend_tok ) );

  /* Allocate from heap here and clean-up on call end */
  evt = ( qcril_data_event_data_t *)malloc( sizeof( qcril_data_event_data_t ) );
  if ( evt == NULL )
  {
    QCRIL_LOG_ERROR( "%s","unable to alloc mem for event obj" );
    goto err_bad_input;
  }
  QCRIL_DS_LOG_DBG_MEM( "event obj alloc", evt );
  memset( evt, 0, sizeof( qcril_data_event_data_t ) );

  /* Populate data event obj */
  evt->evt      = net_evt;
  evt->data     = info_tbl_ptr;
  evt->data_len = sizeof( qcril_data_call_info_tbl_type );
  evt->self     = evt;

  /*
    Call QCRIL API to process this event
    The data event hdlr will be called by RIL thread
    In case of unsol event RIL Token will be 0
  */
  QCRIL_LOG_VERBOSE( "queue QCRIL DATA event for RIL Token [%d]",
                     qcril_log_get_token_id( info_tbl_ptr->pend_tok ) );

  qcril_event_queue( info_tbl_ptr->instance_id,
                     info_tbl_ptr->modem_id,
                     QCRIL_DATA_NOT_ON_STACK,
                     QCRIL_EVT_DATA_EVENT_CALLBACK,
                     ( void * )evt,
                     sizeof( qcril_data_event_data_t ),
                     info_tbl_ptr->pend_tok );

  QCRIL_LOG_DEBUG("%s","qcril_data_net_cb: EXIT with suc");
  return;

err_label:
  if ( qcril_reqlist_query( info_tbl_ptr->instance_id, info_tbl_ptr->pend_tok, &reqlist_info ) == E_SUCCESS )
  {
    qcril_data_call_info_tbl_type *tbl_ptr = ( qcril_data_call_info_tbl_type * ) reqlist_info.sub.ds.info;

    qcril_data_response_generic_failure( tbl_ptr->instance_id, reqlist_info.t, reqlist_info.request );
    QCRIL_LOG_DEBUG("%s","respond to QCRIL as generic failure");
  }

err_bad_input:
  QCRIL_LOG_ERROR("%s","qcril_data_net_cb: EXIT with err");
  return;

}/* qcril_data_net_cb() */


/*===========================================================================

  FUNCTION:  qcril_data_init

===========================================================================*/
/*!
    @brief
    Initialize the DATA subsystem of the RIL.

    (1) Call init routine of Data Services Internal Module

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_init()
{
/*-----------------------------------------------------------------------*/
  int i = 0;

  QCRIL_LOG_DEBUG( "%s", "qcril_data_init: ENTRY" );

  dsi_init();

  memset( info_tbl, 0, sizeof info_tbl );

  for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    info_tbl[ i ].index = i;
    QCRIL_LOG_VERBOSE("info_tbl[%d].index = %d", i, i);
  }/* for() */

  pthread_mutex_init(&call_tbl_mutex, NULL);

  pthread_mutex_init(&info_tbl_mutex, NULL);

  pthread_mutex_init(&qcril_data_global_mutex, NULL);

  global_dorm_ind = DORMANCY_INDICATIONS_OFF;

  QCRIL_LOG_DEBUG( "%s", "qcril_data_init: EXIT" );
} /* qcril_data_init() */


/*===========================================================================

  FUNCTION:  qcril_data_command_hdlr

===========================================================================*/
/*!
    @brief
    Command handler for QCRIL Data

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_command_hdlr(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  QCRIL_LOG_DEBUG( "%s", "qcril_data_command_hdlr: ENTRY" );

  /*-----------------------------------------------------------------------*/
  QCRIL_DS_ASSERT( ( params_ptr != NULL ), "validate input params_ptr" );
  QCRIL_DS_ASSERT( ( ret_ptr    != NULL ), "validate input ret_ptr" );
  /*-----------------------------------------------------------------------*/
  QCRIL_DS_ASSERT( 0, "trap call to data cmd hdlr" );
  QCRIL_LOG_DEBUG( "%s", "qcril_data_command_hdlr: EXIT" );

}/* qcril_data_command_callback() */


/*===========================================================================

  FUNCTION:  qcril_data_event_hdlr

===========================================================================*/
/*!
    @brief
    Registered with QCRIL to be called by QCRIL on event
    QCRIL_EVT_DATA_EVENT_CALLBACK

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_event_hdlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr  /*output*/
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_reqlist_public_type      reqlist_info;
  qcril_data_call_info_tbl_type *info_tbl_ptr;
  qcril_data_call_response_type  response;
  qcril_data_event_data_t       *evt_info_ptr;
  unsigned int                   radio_tech = RADIO_TECH_UNKNOWN;
  char *dummy_rsp[3] = { "0", "rmnet0", "0.0.0.0" };
  unsigned int                   pend_req  = DS_RIL_REQ_INVALID;
  errno_enum_type                qcril_ret = E_FAILURE;
  int                            tmp_end_reason = CALL_FAIL_ERROR_UNSPECIFIED;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/
  QCRIL_DATA_MUTEX_LOCK(&qcril_data_global_mutex);
  QCRIL_LOG_DEBUG( "%s", "qcril_data_event_hdlr: ENTRY" );

  /* Input Validation */
  QCRIL_DS_ASSERT( params_ptr != NULL, "validate input params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate input params_ptr" );
  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    goto err_label_exit;
  }

  memset( &reqlist_info, 0, sizeof( qcril_reqlist_public_type ) );
  memset( &response,     0, sizeof( qcril_data_call_response_type ) );

  evt_info_ptr = ( qcril_data_event_data_t * )params_ptr->data;
  QCRIL_DS_LOG_DBG_MEM( "event obj", evt_info_ptr );
  if ( !VALIDATE_LOCAL_DATA_OBJ( evt_info_ptr ) )
  {
    QCRIL_LOG_ERROR( "%s", "bad event obj, cannot free mem, ret with err" );
    goto err_label_no_free;
  }

  /* Pointer to info Tbl is derived from event posted to QCRIL */
  info_tbl_ptr =  ( qcril_data_call_info_tbl_type *)evt_info_ptr->data;
  QCRIL_DS_LOG_DBG_MEM( "info_tbl", info_tbl_ptr );
  if ( !VALIDATE_LOCAL_DATA_OBJ( info_tbl_ptr ) )
  {
    QCRIL_LOG_ERROR( "%s", "invalid info_tbl ref" );
    goto err_label;
  }

  /* Check whether REQ is pending */
  if ( qcril_data_util_is_req_pending( info_tbl_ptr, &pend_req ) )
  {
    QCRIL_LOG_INFO( "RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ), info_tbl_ptr->cid, info_tbl_ptr->index );
    QCRIL_DS_ASSERT( params_ptr->t != NULL, "validate pend RIL Token" );
    QCRIL_DS_ASSERT( params_ptr->t == info_tbl_ptr->pend_tok, "validate pend RIL Token consistency" );
    if ( ( qcril_ret = qcril_reqlist_query( instance_id, params_ptr->t, &reqlist_info ) ) != E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "unable to find reqlist entry, RIL Token [%d]",
                       qcril_log_get_token_id( params_ptr->t ) );
      goto err_label;
    }

    QCRIL_LOG_DEBUG( "Req list entry found for RIL Token [%d]",
                     qcril_log_get_token_id( params_ptr->t ) );

    /* Validate xtracted local info tbl pointer for a pending RIL Token from reqlist Node */
    if ( qcril_ret == E_SUCCESS )
    {
      QCRIL_DS_ASSERT( ( info_tbl_ptr ==
                     ( qcril_data_call_info_tbl_type * )reqlist_info.sub.ds.info ),
                 "validate info_tbl ref" );
    }
    QCRIL_DS_LOG_DBG_MEM( "info_tbl", info_tbl_ptr );

  }
  else /* RIL REQ not pending */
  {
    QCRIL_LOG_INFO( "%s", "RIL REQ NOT pend" );
    QCRIL_DS_ASSERT( params_ptr->t == NULL, "validate null RIL Token" );
  }

  switch( evt_info_ptr->evt )
  {
    case DSI_EVT_NET_IS_CONN:
      QCRIL_LOG_INFO( ">>>DSI_EVT_NET_IS_CONN: START>>> cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index );
      /*
        Check pending RIL SETUP REQ
        NET_IS_CONN event is not expected if a SETUP REQ is not pending
      */
      if ( pend_req != RIL_REQUEST_SETUP_DATA_CALL )
      {
        QCRIL_LOG_DEBUG( "call setup NOT pending, RIL token [%d], pend req [%d]",
                         qcril_log_get_token_id( info_tbl_ptr->pend_tok ), info_tbl_ptr->pend_req );
        /* Hard assert here, todo: explore recovery */
        QCRIL_DS_ASSERT_H( 0, "pend req SETUP validation failed" );
        goto err_label_exit;
      }
      /* Pending RIL REQ SETUP, find in reqlist */
      else

        /* Get and fill all call attributes */
        qcril_data_util_fill_call_params( info_tbl_ptr );

      /* Update state of local tbl */
      info_tbl_ptr->pend_tok   = NULL;
      info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */

      /* Enable dormancy indications if necessary */
      if (DORMANCY_INDICATIONS_ON == global_dorm_ind)
      {
        QCRIL_LOG_DEBUG("%s", "global_dorm_ind ON: enabling dormancy indications");
        dsi_iface_ioctl(info_tbl_ptr->dsi_hndl,
                        DSI_IFACE_IOCTL_DORMANCY_INDICATIONS_ON);
      }

      /* Update call_tbl to point to latest call info */
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
      qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_UP, PDP_FAIL_NONE );
#else
      qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_UP );
#endif
      dsi_req_get_data_bearer_tech(info_tbl_ptr->dsi_hndl, &radio_tech);

      /* dss API exposes the similar data bearer tech enum
       * as RIL, hence doing a type conversion here
       */
      //call_tbl [ info_tbl_ptr->index ].radioTech = (RIL_RadioTechnology) radio_tech;
      /* Create RIL REQ SETUP response */
      qcril_data_util_create_setup_rsp( info_tbl_ptr, &response );

     /* Post RIL Response */
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
      QCRIL_LOG_DEBUG( ">>>RIL RSP READY>>> cid [%d], ifname [%s], ip_addr [%s] , dnses [%s]",
                     response.setup_rsp.cid, response.setup_rsp.ifname, response.setup_rsp.addresses, response.setup_rsp.dnses );
#else
      QCRIL_LOG_DEBUG( ">>>RIL RSP READY>>> cid [%d], dev_name [%s], ip_addr [%s]",
                      response.setup_rsp.cid, response.setup_rsp.dev_name, response.setup_rsp.ip_addr );
#endif

      /* Post RIL Response */
      qcril_data_response_success( instance_id, params_ptr->t,
                                   RIL_REQUEST_SETUP_DATA_CALL,
                                   (void *) ( &(response.setup_rsp) ),
                                   sizeof response.setup_rsp );
      QCRIL_LOG_DEBUG( "%s", "<<<RIL RSP SENT<<<" );

      QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_IS_CONN: DONE<<<" );
      break; /* DSI_EVT_NET_IS_CONN */

    case DSI_EVT_NET_NO_NET:
      QCRIL_LOG_INFO( ">>>DSI_EVT_NET_NO_NET: START>>> cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index );

      /* Switch on PEND RIL REQ */
      switch( pend_req )
      {
        case RIL_REQUEST_SETUP_DATA_CALL:
        case RIL_REQUEST_DEACTIVATE_DATA_CALL:
          break;

        default:
          QCRIL_LOG_DEBUG( "RIL REQ NOT pending, Token ID [%d]",
                           qcril_log_get_token_id( params_ptr->t ) );
          QCRIL_DS_ASSERT( info_tbl_ptr->pend_req == DS_RIL_REQ_INVALID, "validate pend_req" );
          QCRIL_DS_ASSERT( info_tbl_ptr->pend_tok == NULL, "validate pend_tok" );
      }

      /* Verify call params and mark call as inactive */
      QCRIL_DS_ASSERT( call_tbl [ info_tbl_ptr->index ].cid == info_tbl_ptr->cid,
                       "validate call_tbl.cid" );

      /* Get call fail reason or use default reason */
      if ( dsi_get_last_call_fail_cause(  info_tbl_ptr->dsi_hndl,
                                          &tmp_end_reason ) == DSI_SUCCESS )
      {
        //call_tbl [ info_tbl_ptr->index ].inactiveReason = tmp_end_reason;
        QCRIL_LOG_INFO( "got call end reason [%u]", tmp_end_reason );
      }
      else
      {
        //call_tbl [ info_tbl_ptr->index ].inactiveReason = PDP_FAIL_ERROR_UNSPECIFIED;
        QCRIL_LOG_ERROR( "dsi_get_last_call_fail_cause FAILED, setting [%u]", PDP_FAIL_ERROR_UNSPECIFIED );
      }

      dsi_rel_data_srvc_hndl( info_tbl_ptr->dsi_hndl );
      qcril_data_cleanup_call_state( info_tbl_ptr );

      /* Post RIL RSP Success */
      if ( pend_req == RIL_REQUEST_SETUP_DATA_CALL )
      {
        qcril_data_response_generic_failure( instance_id,
                                             params_ptr->t,
                                             pend_req );
      }
      else
      if ( pend_req == RIL_REQUEST_DEACTIVATE_DATA_CALL )
      {
        qcril_data_response_success( instance_id,
                                     params_ptr->t,
                                     pend_req,
                                     NULL,
                                     0 );
      }
      else
      {
        /* NO_NET event from LL, postpone cleanup, send ind to RIL */
        goto unsol_rsp;
      }

      /* Update state of local tbl LAST
       * this is to indicate that the pending request is NOW
       * fully processed (info_tbl entry is clean)
       */
      QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);
      info_tbl_ptr->pend_tok   = NULL;
      info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */
      QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

      QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_NO_NET: processing complete<<<" );
      goto ret;
      break; /*DSI_EVT_NET_NO_NET*/

    case DSI_EVT_PHSYLINK_DOWN_STATE:
      /* Verify call params and mark call as inactive */
      QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_PHSYLINK_DOWN_STATE: processing Started<<<" );
      QCRIL_DS_ASSERT( call_tbl [ info_tbl_ptr->index ].cid == info_tbl_ptr->cid,
                       "validate call_tbl.cid" );

      /* Update call_tbl to point to latest call info */
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
      qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_DOWN, PDP_FAIL_NONE );
#else
      qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_DOWN );
#endif

      goto unsol_rsp;

    case DSI_EVT_PHSYLINK_UP_STATE:
      /* Verify call params and mark call as inactive */
      QCRIL_LOG_INFO( "%s",  "<<<DSI_EVT_PHSYLINK_UP_STATE: processing Started<<<" );
      QCRIL_DS_ASSERT( call_tbl [ info_tbl_ptr->index ].cid == info_tbl_ptr->cid,
                       "validate call_tbl.cid" );
      /* Update call_tbl to point to latest call info */
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
      qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_UP, PDP_FAIL_NONE );
#else
      qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_UP );
#endif

      goto unsol_rsp;

    case DSI_EVT_NET_RECONFIGURED:
      QCRIL_LOG_INFO( ">>>DSI_EVT_NET_RECONFIGURED: START>>> cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index );

      /* Get and fill all call attributes */
      qcril_data_util_fill_call_params( info_tbl_ptr );

      QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_RECONFIGURED: DONE<<<" );
      goto unsol_rsp;

    default:
      QCRIL_LOG_ERROR( "invalid dsi_net_ev [%d]", (dsi_net_evt_t)params_ptr->data );
      QCRIL_DS_ASSERT( 0, "validate dsi_net_ev" );
      goto err_label_exit;
   }/* switch() */

unsol_rsp:
  qcril_data_unsol_call_list_changed( instance_id,
                                      call_tbl,
                                      sizeof( call_tbl ) );
ret:
  QCRIL_DS_ASSERT( evt_info_ptr != NULL, "validate event obj" );
  QCRIL_LOG_INFO( "%s", "try event obj dealloc" );
  if ( evt_info_ptr != NULL ) free( evt_info_ptr );
  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_DEBUG( "%s", "qcril_data_event_hdlr: EXIT with suc" );
  return;

err_label:
  QCRIL_DS_ASSERT( evt_info_ptr != NULL, "validate event obj" );
  QCRIL_LOG_INFO( "%s", "try event obj dealloc" );
  if ( evt_info_ptr != NULL ) free( evt_info_ptr );

err_label_no_free:
  if ( qcril_reqlist_query( instance_id, params_ptr->t, &reqlist_info ) == E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s", "respond to QCRIL as generic failure" );
    qcril_data_response_generic_failure( instance_id, reqlist_info.t, reqlist_info.request );
  }

err_label_exit:
  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_ERROR("%s", "qcril_data_event_hdlr: EXIT with err");
  return;

}/* qcril_data_event_hdlr() */


/*===========================================================================

  FUNCTION:  qcril_data_request_setup_data_call

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SETUP_DATA_CALL

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_setup_data_call
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_modem_id_e_type       modem_id = QCRIL_DEFAULT_MODEM_ID;
  qcril_reqlist_public_type      reqlist_info;
  dsi_hndl_t                     *data_hndl;
  int                            i, apn_len;
  dsi_param_info_t               apn_info, username_info, password_info,authpref_info;
  dsi_param_info_t               profile_id;
  const char                     *ril_apn;
  const char                     *ril_user;
  const char                     *ril_pass;
  const char                     *ril_auth_pref = NULL;
  const char                     *ril_tech = NULL;
  const char                     *ril_profile = NULL;
  const char                     *ril_ip_family = NULL;
  char                           tmp_apn[ DS_CALL_INFO_APN_MAX_LEN + 1 ];
  qcril_data_net_cb_info_t       *trn;
  int                            profile_param_id = 0;
  qcril_reqlist_u_type           u_info;
  qcril_reqlist_public_type      reqlist_entry;

  QCRIL_DATA_MUTEX_LOCK(&qcril_data_global_mutex);

  /*-----------------------------------------------------------------------*/
  if ((params_ptr == NULL) || (ret_ptr == NULL) ||
      ((params_ptr->datalen % 4 ) != 0))
  {
    QCRIL_LOG_ERROR( " Bad Input params_ptr [%#x], ret_ptr [%#x], datalen [%d]",
                     (unsigned int)params_ptr, (unsigned int)ret_ptr,
                     params_ptr->datalen );
    goto err_bad_input;
  }
  instance_id = params_ptr->instance_id;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR( "%s", " Bad Instance Id received");
    goto err_bad_input;
  }
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_VERBOSE( "%s", "qcril_data_request_setup_data_call: ENTRY" );

  memset( &u_info,  0, sizeof( qcril_reqlist_u_type ) );
  memset( tmp_apn,   0, DS_CALL_INFO_APN_MAX_LEN + 1 );
  memset( &apn_info, 0, sizeof( dsi_param_info_t ) );
  memset( &username_info, 0, sizeof( dsi_param_info_t ) );
  memset( &password_info, 0, sizeof( dsi_param_info_t ) );
  memset( &authpref_info, 0, sizeof( dsi_param_info_t ) );
  memset( &profile_id, 0, sizeof( dsi_param_info_t ) );

  ril_apn  = ((const char **)params_ptr->data)[2];
  ril_user = ((const char **)params_ptr->data)[3];
  ril_pass = ((const char **)params_ptr->data)[4];
  ril_tech = ((const char **)params_ptr->data)[0];
  ril_profile = ((const char **)params_ptr->data)[1];
  ril_auth_pref = ((const char **)params_ptr->data)[5];
  ril_ip_family = ((const char **)params_ptr->data)[6];

  /*If ril_ip_family is NULL data call will be defaulted to ipv4*/
  if ((ril_ip_family != NULL) && (atoi(ril_ip_family) != QCRIL_DATA_IP_FAMILY_IPV4))
  {
    QCRIL_LOG_ERROR( "%s", "BAD input: Unsupported IP family specified \n");
    goto err_bad_input;
  }

  if ( (ril_user != NULL) && (ril_pass != NULL) )
  {
    QCRIL_LOG_DEBUG( "RIL provided USERNAME len [%d], PASSWORD len [%d]",
                      strlen( ril_user), strlen( ril_pass ) );
  }

  if ( ril_apn != NULL )
  {
    /* APN len calculations */
    apn_len = MINIMUM( strlen( ril_apn ), DS_CALL_INFO_APN_MAX_LEN );
    memcpy( tmp_apn, ril_apn, apn_len );
    tmp_apn[ apn_len ] = '\0';
    QCRIL_LOG_DEBUG( "RIL APN [%s]", ril_apn );
  }
  else
  {
    tmp_apn[0] = '\0';
    QCRIL_LOG_DEBUG( "%s", "RIL APN is NULL, Use NULL string." );
  }

  if (!ril_auth_pref)
  {
    QCRIL_LOG_DEBUG("%s","No Authentication Preference provided.");
  }
  else
  {
    QCRIL_LOG_DEBUG("RIL provided authentication preference %s",ril_auth_pref);
  }

  for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    if( !VALIDATE_LOCAL_DATA_OBJ( (&info_tbl[i]) ) )
    {
      break;
    }
  }/* for() */

  QCRIL_LOG_DEBUG( "found free info_tbl at index [%d]", i);

  if ( i == MAX_CONCURRENT_UMTS_DATA_CALLS )
  {
    QCRIL_LOG_ERROR( "%s", "no free info_tbl entry" );
    goto err_label;
  }

  /* Allocate CID, copy APN and other call params */
  info_tbl[i].info_flg            = FALSE; /* mark call_info and dev_name as invalid */
  info_tbl[i].index               = i;
  info_tbl[i].cid                 = i;
  call_tbl[info_tbl[i].index].cid = i;
  strcpy( info_tbl[i].call_info.apn, tmp_apn );
  info_tbl[i].instance_id         = instance_id;
  info_tbl[i].modem_id            = modem_id;
  info_tbl[i].pend_tok            = params_ptr->t;
  info_tbl[i].pend_req            = RIL_REQUEST_SETUP_DATA_CALL;
  info_tbl[i].self                = &info_tbl[i];

  if ( ( info_tbl[i].dsi_hndl = dsi_get_data_srvc_hndl( qcril_data_net_cb,
                                                        ( void *) &info_tbl[i] ) ) == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "unable to get dsi hndl" );
    goto err_label;
  }

  QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

  /*Register for Physlink UP DOWN Indication Events */
  QCRIL_LOG_DEBUG("%s", "Registering for Physlink Events");

  if (dsi_reg_physlink_up_down_events(info_tbl[i].dsi_hndl) == DSI_ERROR)
  {
    QCRIL_LOG_ERROR( "%s", "Error registering for Physlink Events");
  }

  /* set profile id in the dsi store */

  if (ril_profile == NULL)
  {
      QCRIL_LOG_ERROR("%s", "NULL profile (params->data[1]) received");
      QCRIL_LOG_ERROR("%s", "RIL interface advises to use value 0");
      goto err_label;
  }

  if ((atoi(ril_profile)) != RIL_DATA_PROFILE_DEFAULT)
  {
    if(ril_tech == NULL)
    {
      QCRIL_LOG_ERROR("RIL provided profile id without specifying"
                      " the technology (CDMA/UMTS). This profile id"
                      " %s will be ignored\n", ril_profile);
      goto err_label;
    }
    profile_id.buf = (void *)ril_profile;
    profile_id.len = strlen(ril_profile);
    QCRIL_LOG_VERBOSE("RIL provided PROFILE ID len [%d], Number [%s]",
                      profile_id.len, (char *) profile_id.buf);

    if (!strcmp(ril_tech, QCRIL_CDMA_STRING))
    {
      profile_param_id = DATA_CALL_INFO_CDMA_PROFILE_IDX;
    }
    else if (!strcmp(ril_tech, QCRIL_UMTS_STRING))
    {
      profile_param_id = DATA_CALL_INFO_UMTS_PROFILE_IDX;
    }
    else
    {
      QCRIL_LOG_ERROR("RIL provided incorrect/malformed technology %s\n", ril_tech);
      goto err_label;
    }
    if ( ( dsi_set_data_call_param(info_tbl[i].dsi_hndl,
                                   profile_param_id,
                                   &profile_id ) ) != DSI_SUCCESS )
    {
      QCRIL_LOG_ERROR("unable to set profile id, index [%d]", info_tbl[i].index );
      goto err_label;
    }
  }

  /* Setup buffer */
  if (ril_apn != NULL)
  {
    apn_info.buf = (void *)ril_apn;
    apn_info.len = strlen( ril_apn );
    QCRIL_LOG_VERBOSE( "RIL provided APN len [%d], APN string [%s]",
                       apn_info.len, (char *) apn_info.buf );
  }
  else
  {
    apn_info.buf = "";
    apn_info.len = 0;
    QCRIL_LOG_VERBOSE( "%s", "RIL did not provide APN, use null string as name" );
  }

  if ( ( dsi_set_data_call_param( info_tbl[i].dsi_hndl ,
                                  DATA_CALL_INFO_APN_NAME,
                                 &apn_info ) ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "unable to set APN, index [%d]", info_tbl[i].index );
    goto err_label;
  }

  if (ril_user != NULL)
  {
    username_info.buf = (void *)ril_user;
    username_info.len = strlen( ril_user );
    QCRIL_LOG_VERBOSE( "RIL provided UserName, len [%d]", username_info.len);
    if ( ( dsi_set_data_call_param( info_tbl[i].dsi_hndl ,
                                    DATA_CALL_INFO_USERNAME,
                                   &username_info ) ) != DSI_SUCCESS )
    {
      QCRIL_LOG_ERROR("unable to set UserName, index [%d]", info_tbl[i].index );
      goto err_label;
    }
  }
  if (ril_pass != NULL)
  {
    password_info.buf = (void *)ril_pass;
    password_info.len = strlen( ril_pass );
    QCRIL_LOG_VERBOSE( "RIL provided Password, len [%d]", password_info.len);

    if ( ( dsi_set_data_call_param( info_tbl[i].dsi_hndl ,
                                    DATA_CALL_INFO_PASSWORD,
                                    &password_info ) ) != DSI_SUCCESS )
    {
      QCRIL_LOG_ERROR("unable to set PassWord, index [%d]", info_tbl[i].index );
      goto err_label;
    }
  }

  /*Set Authentication Preference*/
  if (ril_auth_pref != NULL)
  {
    authpref_info.buf = (void *) ril_auth_pref;
    authpref_info.len = strlen(ril_auth_pref);
    if ( ( dsi_set_data_call_param( info_tbl[i].dsi_hndl ,
                                    DATA_CALL_INFO_AUTH_PREF,
                                    &authpref_info) ) != DSI_SUCCESS )
    {
      QCRIL_LOG_ERROR("unable to set AUTH PREF, index [%d]", info_tbl[i].index );
      goto err_label;
    }
  }

  /* Insert local info tbl ref in reqlist */
  u_info.ds.info = (void *) &info_tbl[i];
  qcril_reqlist_default_entry( params_ptr->t,
                               params_ptr->event_id,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to insert entry to ReqList */
    QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
    return;
  }

  QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, setup data call" );

  /* Start data call */
  if ( ( dsi_req_start_data_call( info_tbl[i].dsi_hndl ) ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "unable to setup data call, index [%d]", info_tbl[i].index );
    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );
    qcril_reqlist_free( instance_id, params_ptr->t );
    goto err_label;
  }

  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_DEBUG("%s", "qcril_data_request_setup_default_pdp: Return with SUCCESS");
  return;

err_label:
  if (VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) && info_tbl[i].dsi_hndl)
  {
    dsi_rel_data_srvc_hndl( info_tbl[i].dsi_hndl );
    qcril_data_cleanup_call_state( &info_tbl[i] );
  }

err_bad_input:
  if( params_ptr )
  {
    QCRIL_LOG_DEBUG("%s", "respond to QCRIL as generic FAILURE");
    qcril_data_response_generic_failure( params_ptr->instance_id,
                                         params_ptr->t,
                                         RIL_REQUEST_SETUP_DATA_CALL );
  }

  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_ERROR("%s", "qcril_data_request_setup_default_pdp: Return with FAILURE");
  return;

}/* qcril_data_request_setup_data_call() */


/*===========================================================================

  FUNCTION:  qcril_data_request_deactivate_data_call

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DEACTIVATE_DATA_CALL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_deactivate_data_call
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_modem_id_e_type       modem_id = QCRIL_DEFAULT_MODEM_ID;
  int                            i;
  qcril_reqlist_u_type           u_info;
  qcril_reqlist_public_type      reqlist_entry;
  int                            cid;
  unsigned int                   intcid, pend_req = DS_RIL_REQ_INVALID;
  const char                     *cid_ptr;
  const char                     **in_data;

  QCRIL_DATA_MUTEX_LOCK(&qcril_data_global_mutex);

  /*-----------------------------------------------------------------------*/
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR( " Bad Input params_ptr [%#x], ret_ptr [%#x], ",
                     (unsigned int)params_ptr, (unsigned int)ret_ptr );
    goto err_bad_input;
  }
  instance_id = params_ptr->instance_id;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR( "%s", " Bad Instance Id received");
    goto err_bad_input;
  }
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_data_request_deact_data_call: ENTRY" );

  cid_ptr  = ((const char **)params_ptr->data)[0];

  memset( &u_info, 0, sizeof( qcril_reqlist_u_type ) );

  cid    = strtol( cid_ptr, NULL, 10 );

  QCRIL_LOG_DEBUG( "RIL says CID [%d], len [%d]",
                   cid, params_ptr->datalen );

  QCRIL_DS_ASSERT( params_ptr->datalen == sizeof cid, "validate CID len" );

  for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    QCRIL_LOG_DEBUG("info_tbl CID [%d], index [%d]", info_tbl[i].cid, i);

    if( ( VALIDATE_LOCAL_DATA_OBJ( &info_tbl[i] ) ) && ( info_tbl[i].cid == cid ) )
    {
      QCRIL_LOG_DEBUG("found matching CID [%d], index [%d]", cid, i);
      break;
    }
  }/* for() */

  if ( i == MAX_CONCURRENT_UMTS_DATA_CALLS )
  {
    QCRIL_LOG_ERROR( "no valid CID [%d] match found", cid );
    goto err_label;
  }

  /* Check whether REQ is pending */
  if ( qcril_data_util_is_req_pending( &info_tbl[i], &pend_req ) )
  {
    QCRIL_LOG_INFO( "UNEXPECTED RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ),
                    info_tbl[i].cid, info_tbl[i].index );
    /* if deactivate request is already pending, ignore another one */
    if (pend_req == RIL_REQUEST_DEACTIVATE_DATA_CALL)
    {
        QCRIL_LOG_INFO("RIL_REQUEST_DEACTIVATE_DATA_CALL already pending, "
                       "cid [%d], index [%d]",
                       info_tbl[i].cid, info_tbl[i].index);
    }
    goto err_label;
  }

  /*----------------------------------------------------------------------
    Call deactivation
  ----------------------------------------------------------------------*/
  info_tbl[i].instance_id = instance_id;
  info_tbl[i].modem_id    = modem_id;
  info_tbl[i].pend_tok    = params_ptr->t;
  info_tbl[i].pend_req    = RIL_REQUEST_DEACTIVATE_DATA_CALL;

  /* Insert local info tbl ref in reqlist */
  u_info.ds.info = ( void *) &info_tbl[i];

  qcril_reqlist_default_entry( params_ptr->t,
                               RIL_REQUEST_DEACTIVATE_DATA_CALL,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );

  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add entry */
    QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
    return;
  }

  /* Stop data call */
  QCRIL_LOG_DEBUG( "%s", "call termination starting..." );
  if ( ( dsi_req_stop_data_call( info_tbl[i].dsi_hndl ) ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "unable to teardown data call, index [%d]", i );
    goto err_label;
  }

  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_DEBUG("%s", "qcril_data_request_deact_data_call: EXIT with suc");
  return;

err_label:
  qcril_data_response_generic_failure( instance_id,
                                       params_ptr->t,
                                       RIL_REQUEST_DEACTIVATE_DATA_CALL);
  QCRIL_LOG_DEBUG("%s", "respond to QCRIL as generic failure");

err_bad_input:
  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_ERROR("%s", "qcril_data_request_deact_data_call: EXIT with err");
  return;

} /* qcril_data_request_deactivate_data_call() */


/*===========================================================================

  FUNCTION:  qcril_data_request_last_data_call_fail_cause

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE.
    This function calls
    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_last_data_call_fail_cause
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_data_call_response_type response;

  QCRIL_DATA_MUTEX_LOCK(&qcril_data_global_mutex);
  /*-----------------------------------------------------------------------*/
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR( " Bad Input params_ptr [%#x], ret_ptr [%#x], ",
                     (unsigned int)params_ptr, (unsigned int)ret_ptr );
    goto err_bad_input;
  }
  instance_id = params_ptr->instance_id;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR( "%s", " Bad Instance Id received");
    goto err_bad_input;
  }
  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_VERBOSE( "%s", "qcril_data_request_last_data_call_fail_cause: ENTRY" );

  memset( &response, 0, sizeof ( qcril_data_call_response_type ) );

  response.cause_code = last_call_end_reason;
  response.size = sizeof( response.cause_code );

  QCRIL_LOG_VERBOSE( "send cause code [%u], size [%d] ",
                     response.cause_code, response.size );

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE,
                               (void *) &(response.cause_code),
                               response.size );

  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_DEBUG("%s", "qcril_data_request_last_data_call_fail_cause: EXIT with suc");
  return;

err_label:
  QCRIL_LOG_DEBUG("%s", "respond to QCRIL as generic failure");
  qcril_data_response_generic_failure( instance_id, params_ptr->t, RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE );
err_bad_input:
  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_ERROR("%s", "qcril_data_request_last_data_fail_cause: EXIT with err");
  return;

} /* qcril_data_request_last_data_call_fail_cause() */


/*===========================================================================

  FUNCTION:  qcril_data_request_data_call_list

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DATA_CALL_LIST.

    @return
    PDP Context List
    None.
*/
/*=========================================================================*/
void qcril_data_request_data_call_list
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_data_call_response_type response;
  int i = 0;

  QCRIL_DATA_MUTEX_LOCK(&qcril_data_global_mutex);

  /*-----------------------------------------------------------------------*/
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR( " Bad Input params_ptr [%#x], ret_ptr [%#x], ",
                     (unsigned int)params_ptr, (unsigned int)ret_ptr );
    goto err_bad_input;
  }
  instance_id = params_ptr->instance_id;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR( "%s", " Bad Instance Id received");
    goto err_bad_input;
  }
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_data_request_data_context_list: ENTRY" );

  memset( &response, 0, sizeof( qcril_data_call_response_type ) );

  /* Create and post RIL Response */
  response.list = call_tbl;
  response.size = sizeof(RIL_Data_Call_Response_v6) * MAX_CONCURRENT_UMTS_DATA_CALLS;


  QCRIL_DS_LOG_DBG_MEM( "response", response.list );

  QCRIL_LOG_DEBUG( "len [%d]", response.size );

  /* lock call_tbl while we provide it to RIL */
  QCRIL_DATA_MUTEX_LOCK(&call_tbl_mutex);

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               RIL_REQUEST_DATA_CALL_LIST,
                               (void *) call_tbl,
                               response.size );

  /* unlock call_tbl now */
  QCRIL_DATA_MUTEX_UNLOCK(&call_tbl_mutex);
  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_DEBUG("%s", "qcril_data_request_data_call_list: EXIT with suc");
  return;

err_label:
  qcril_data_response_generic_failure( instance_id, params_ptr->t, RIL_REQUEST_DATA_CALL_LIST );
  QCRIL_LOG_DEBUG("%s", "respond to QCRIL as generic failure");
err_bad_input:
  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  QCRIL_LOG_ERROR("%s", "qcril_data_request_data_call_list: EXIT with err");
  return;

} /* qcril_data_request_data_call_list() */

/*===========================================================================

  FUNCTION:  qcril_data_request_omh_profile_info

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_DATA_CALL_PROFILE.
    Currently empty stub as it's only required for certain target that
    should be using the definition from qcril_data_netctrl.c instead. If
    upper layers behave, this function from qcril_data.c should never be
    called.

    @return
*/
/*=========================================================================*/
void qcril_data_request_omh_profile_info
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_request_resp_params_type resp;

  QCRIL_LOG_DEBUG("%s","qcril_data_request_omh_profile_info: ENTRY");

  /* sanity check */
  if (NULL == params_ptr)
  {
    QCRIL_LOG_ERROR("%s","qcril_data_request_omh_profile_info: "
                    "NULL params_ptr rcvd");
    QCRIL_LOG_DEBUG("%s","qcril_data_request_omh_profile_info: EXIT");
    return;
  }

  /* Remove the entry from ReqList */
  (void) qcril_reqlist_free (params_ptr->instance_id,
                             params_ptr->t);
  /* Send not supported as the response to the RIL command */
  qcril_default_request_resp_params( params_ptr->instance_id,
                                     params_ptr->t,
                                     RIL_REQUEST_GET_DATA_CALL_PROFILE,
                                     RIL_E_REQUEST_NOT_SUPPORTED,
                                     &resp );
  qcril_send_request_response( &resp );
  QCRIL_LOG_DEBUG("%s","qcril_data_request_omh_profile_info: EXIT");
}

void qcril_data_request_set_data_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_LOG_DEBUG("%s","qcril_data_request_set_data_profile: UNSUPPORTED");
}


/*-------------------------------------------------------------------------

                           UTILITY ROUTINES

-------------------------------------------------------------------------*/


/*=========================================================================
  FUNCTION:  qcril_data_response_generic_failure

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send E_GENERIC_FAILURE response.

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_response_generic_failure
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request
)
{
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_ERROR( "%s", "qcril_data_response_generic_failure" );

  /* Remove the entry from ReqList */
  (void) qcril_reqlist_free( instance_id, t );

  /* Send GenericFailure as the response to the RIL command */
  qcril_default_request_resp_params( instance_id, t, request, RIL_E_GENERIC_FAILURE, &resp );
  qcril_send_request_response( &resp );

} /* qcril_data_response_generic_failure */


/*=========================================================================
  FUNCTION:  qcril_data_response_success

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send RIL_E_SUCCESS response.

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_response_success
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request,
  void *response,
  size_t response_len
)
{
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_data_response_success" );

  /* Successfully remove the entry from ReqList */
  (void) qcril_reqlist_free( instance_id, t );

  /* Send SUCCESS as the response to the RIL command */
  qcril_default_request_resp_params( instance_id, t, request, RIL_E_SUCCESS, &resp );
  resp.resp_pkt = response;
  resp.resp_len = response_len;
  qcril_send_request_response( &resp );

} /* qcril_data_response_success */


/*=========================================================================
  FUNCTION:  qcril_data_unsol_call_list_changed

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send RIL_E_SUCCESS response.

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_unsol_call_list_changed
(
  qcril_instance_id_e_type instance_id,
  void                        *response,
  size_t                      response_len
)
{
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_data_unsol_call_list_changed: ENTRY" );

  QCRIL_LOG_DEBUG( "%s", "sending RIL_UNSOL_DATA_CALL_LIST_CHANGED" );

  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_DATA_CALL_LIST_CHANGED, &unsol_resp );
  unsol_resp.resp_pkt = ( void * ) response;
  unsol_resp.resp_len = response_len;
  qcril_send_unsol_response( &unsol_resp );

  QCRIL_LOG_DEBUG( "%s", "qcril_data_unsol_call_list_changed: EXIT with succ" );
  return;

}
/*=========================================================================
  FUNCTION:  qcril_data_cleanup_call_state

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send RIL_E_SUCCESS response.

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_cleanup_call_state
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
)
{
  QCRIL_LOG_DEBUG( "%s", "qcril_data_cleanup_call_state: ENTRY" );


  if ( !VALIDATE_LOCAL_DATA_OBJ( info_tbl_ptr ) )
  {
    QCRIL_LOG_ERROR( "%s", "invalid info_tbl_ptr" );
    goto err_label;
  }

  QCRIL_LOG_DEBUG( "clean up local info tbl, index [%d], cid [%d]",
                      info_tbl_ptr->index, info_tbl_ptr->cid );

  info_tbl_ptr->cid      = CALL_ID_INVALID;
  memset( &info_tbl_ptr->call_info, 0, sizeof info_tbl_ptr->call_info );
  info_tbl_ptr->dsi_hndl = NULL;
  info_tbl_ptr->pend_tok = NULL;
  info_tbl_ptr->pend_req = DS_RIL_REQ_INVALID;
  info_tbl_ptr->self     = NULL;

  /* update the corresponding entry in the call tbl as well */
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
   qcril_data_util_update_call_state( info_tbl_ptr, CALL_INACTIVE, PDP_FAIL_NONE );
#else
  qcril_data_util_update_call_state( info_tbl_ptr, CALL_INACTIVE );
#endif

  QCRIL_LOG_DEBUG( "%s", "qcril_data_cleanup_call_state: EXIT with succ" );
  return;

err_label:
  QCRIL_LOG_DEBUG( "%s", "qcril_data_cleanup_call_state: exit with FAILURE" );
  return;
}/* qcril_data_cleanup_call_state() */

/*=========================================================================
  FUNCTION:  qcril_data_get_call_info

===========================================================================*/
/*!
    @brief
    Get data call info from DSI module
    This routine populates:
    Call Type   :  IPv4, IPv6 etc
    APN:          Access Point Name
    IP Address  :
    Device Name :

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_get_call_info
(
  const qcril_data_call_info_tbl_type *const  info_tbl_ptr,
  dsi_call_info_t                     *const  call_info_ptr
)
{
  dsi_param_info_t           call_type_info, ip_addr_info, info;

  memset( &info, 0, sizeof( dsi_param_info_t ) );
  memset( &ip_addr_info,   0, sizeof( dsi_param_info_t ) );

  QCRIL_LOG_DEBUG( "%s", "qcril_data_get_call_info: ENTRY" );

  QCRIL_LOG_DEBUG( "dsi_hndl [%d]", (unsigned int)info_tbl_ptr->dsi_hndl );

  /* Get call type */
  if ( ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                  DATA_CALL_INFO_CALL_TYPE,
                                 &info ) ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "%s", "unable to get type" );
    QCRIL_DS_LOG_DBG_MEM( "info_tbl", info_tbl_ptr );
    goto err_label;
  }

  qcril_data_util_buffer_dump( &info );

  /* Check type len */
  if ( info.len > DS_CALL_INFO_TYPE_MAX_LEN )
  {
    QCRIL_LOG_ERROR( "type too long, len [%d], restricting to [%d]",
                     info.len, DS_CALL_INFO_TYPE_MAX_LEN );
    info.len = DS_CALL_INFO_TYPE_MAX_LEN;
  }

  memcpy( call_info_ptr->type, info.buf, info.len );
  call_info_ptr->type[ info.len ] = '\0';

  /*----------------------------------------------------------------------------
    Get APN
  ----------------------------------------------------------------------------*/
  if ( ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                  DATA_CALL_INFO_APN_NAME,
                                 &info ) ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "unable to get call param, info_tbl index [%d]", info_tbl_ptr->index );
    goto err_label;
  }

  /* Check APN len */
  if ( info.len > DS_CALL_INFO_APN_MAX_LEN )
  {
    QCRIL_LOG_ERROR( "APN too long, len [%d], restricting to [%d]",
                     info.len, DS_CALL_INFO_APN_MAX_LEN );
    info.len = DS_CALL_INFO_TYPE_MAX_LEN;
  }

  qcril_data_util_buffer_dump( &info );

  strncpy( call_info_ptr->apn, info.buf, info.len );
  call_info_ptr->apn[ info.len ] = '\0';

  /*----------------------------------------------------------------------------
    Get address
  ----------------------------------------------------------------------------*/
  if ( ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                  DATA_CALL_INFO_IP_ADDR,
                                 &info ) ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "%s", "unable to get ip addr" );
    goto err_label;
  }

  /* Check IP address len */
  if ( info.len > DS_CALL_INFO_IP_ADDR_MAX_LEN )
  {
    QCRIL_LOG_ERROR( "IP addr too long, len [%d], restrict to [%d]",
                     info.len, DS_CALL_INFO_IP_ADDR_MAX_LEN );
    info.len = DS_CALL_INFO_TYPE_MAX_LEN;
  }

  qcril_data_util_buffer_dump( &info );

  /* Get IP address as a string */
  if ( qcril_data_util_conv_ip_addr( &info, &ip_addr_info ) != SUCCESS )
  {
    QCRIL_LOG_ERROR( "ip addr conv err, ip addr [%d]",
                        *(( char *)info.buf) );
    goto err_label;
  }

  strncpy( call_info_ptr->address, ip_addr_info.buf, ip_addr_info.len );
  call_info_ptr->address[ ip_addr_info.len ] = '\0';

  QCRIL_LOG_DEBUG( "ip addr [%s]", call_info_ptr->address );


  /*---------------------------------------------------------
    Get device name
  ---------------------------------------------------------*/
  if ( ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                  DATA_CALL_INFO_DEVICE_NAME,
                                 &info ) ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "%s", "unable to get dev name" );
    goto err_label;
  }

  /* Check device name len */
  if ( info.len > DS_CALL_INFO_DEV_NAME_MAX_LEN )
  {
    QCRIL_LOG_ERROR( "dev_name too long, len [%d]", info.len );
    goto err_label;
  }

  strncpy( call_info_ptr->dev_name, info.buf, info.len );
  call_info_ptr->dev_name[ info.len ] = '\0';

  QCRIL_LOG_DEBUG( "dev_name [%s]", call_info_ptr->dev_name );

  QCRIL_LOG_DEBUG( "%s", "qcril_data_get_call_info: EXIT with suc" );
  return SUCCESS;

err_label:
  QCRIL_LOG_ERROR("%s", "qcril_data_get_call_info: EXIT with err");
  return FAILURE;

}/* qcril_data_get_call_info() */

static int qcril_data_util_conv_ip_addr(
  const dsi_param_info_t  *const info,
  dsi_param_info_t        *const ip_addr_info /* output param */
)
{
  char *ip_addr = "0.0.0.0\0";

  QCRIL_LOG_DEBUG( "%s", "qcril_data_util_conv_ip_addr: ENTRY" );

  QCRIL_DS_ASSERT( info->len == sizeof( long int ), "validate IPv4 addr len" );

  ip_addr_info->len = strlen( ip_addr );
  strncpy( ip_addr_info->buf, ip_addr, ip_addr_info->len );

  QCRIL_LOG_DEBUG( "%s", "qcril_data_util_conv_ip_addr: EXIT" );

  return SUCCESS;

}/* qcril_data_util_conv_ip_addr */


/*=========================================================================
  FUNCTION:  qcril_data_util_buffer_dump

===========================================================================*/
/*!
    @brief

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_buffer_dump(
  const dsi_param_info_t    *const info
)
{
  unsigned int i = 0;
  const dsi_param_info_t * tmp = (dsi_param_info_t *) info;

  if ( tmp == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "cannot dump null ptr" );
    goto err_label;
  }

  QCRIL_LOG_DEBUG( "%s", ">>>start buffer dump>>>" );

  for( i = 0; i < tmp->len; i++ )
  {
    if ( isalpha( ( (char *)tmp->buf )[ i ]) )
    {
      QCRIL_LOG_DEBUG( "buffer[%d] = %c", i, ((char *)tmp->buf)[ i ] );
    }
    else
    {
      QCRIL_LOG_DEBUG( "buffer[%d] = %#x", i, ((unsigned int *)tmp->buf)[ i ] );
    }
  }

  QCRIL_LOG_DEBUG( "%s", "<<<end buffer dump<<<" );

err_label:
  return;
}/* qcril_data_buffer_dump() */


/*=========================================================================
  FUNCTION:  qcril_data_util_is_pending

===========================================================================*/
/*!
    @brief

    @return
    NONE
*/
/*=========================================================================*/
static int qcril_data_util_is_req_pending
(
  const qcril_data_call_info_tbl_type *info_tbl_ptr,
  unsigned int *pend_req
)
{
  int ret = FALSE;

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  if ( info_tbl_ptr == NULL || pend_req == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Bad input received" );
    goto err_label;
  }

  if ( ( info_tbl_ptr->pend_tok == NULL ) &&
       ( info_tbl_ptr->pend_req == DS_RIL_REQ_INVALID ) )
  {
    *pend_req = DS_RIL_REQ_INVALID;
    goto err_label;
  }
  else if ( ( info_tbl_ptr->pend_tok != NULL ) &&
            ( info_tbl_ptr->pend_req != DS_RIL_REQ_INVALID ) )
  {
    *pend_req = info_tbl_ptr->pend_req;
    ret = TRUE;
  }
  else
  {
    QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
    QCRIL_DS_ASSERT_H( 0, "bad state, pend_tok and pend_req out of sync" );
    return FALSE;
  }

err_label:
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

  return ret;
}/* qcril_data_util_is_req_pending() */


static void qcril_data_util_fill_call_params
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
)
{
  dsi_param_info_t               param_info;
#define AUTO_BUF_LEN  MAXIMUM( MAXIMUM( DS_CALL_INFO_DEV_NAME_MAX_LEN,DS_CALL_INFO_APN_MAX_LEN ),DS_CALL_INFO_IP_ADDR_MAX_LEN )
  char                           tmp_buf[ AUTO_BUF_LEN ] = {"unknown\0"};
  int                            tmp_len   = 0, tmp_ipaddr = 0;

  memset( &param_info,   0, sizeof( dsi_param_info_t ) );
  param_info.buf = (void *)tmp_buf;
  param_info.len = sizeof tmp_buf;

  /* Fill CID */
  sprintf( info_tbl_ptr->call_info.cid, "%d", info_tbl_ptr->cid);

  /* Get device name */
  if ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                DATA_CALL_INFO_DEVICE_NAME,
                                &param_info ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "get dev name FAILED, ret [unknown], dsi_hndl [%x]", (unsigned int)info_tbl_ptr->dsi_hndl );
  }
  else
  {
    /* Copy according to presentation layer, limit size to IFNAMSIZ  */
    tmp_len = MINIMUM( param_info.len, IFNAMSIZ );
    memcpy( info_tbl_ptr->call_info.dev_name, param_info.buf, tmp_len );
    info_tbl_ptr->call_info.dev_name[ tmp_len ] = '\0';
    QCRIL_LOG_DEBUG( "dev_name [%s], len [%d]", info_tbl_ptr->call_info.dev_name, tmp_len );
  }

  /* Get call type and update local call tbl */
  if ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                DATA_CALL_INFO_CALL_TYPE,
                                &param_info ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "unable to get call type, ret unknown, dsi_hndl [%x]", (unsigned int)info_tbl_ptr->dsi_hndl );
  }
  else
  {
    /* Copy according to presentation layer  */
    memcpy( info_tbl_ptr->call_info.type, param_info.buf,
            param_info.len );
    info_tbl_ptr->call_info.type[ param_info.len ] = '\0';

    QCRIL_LOG_DEBUG( "call_type [%s], len [%d]", info_tbl_ptr->call_info.type, param_info.len );
  }

  /* Get IP addr and update local call tbl */
  if ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                DATA_CALL_INFO_IP_ADDR,
                                &param_info ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "get IP addr FAILED, ret [unknown], dsi_hndl [%x]", (unsigned int)info_tbl_ptr->dsi_hndl );
  }
  else
  {
    /* Conv and copy according to presentation layer  */
    if ( param_info.len == QCRIL_IPV4_ADDR )
    {
      tmp_ipaddr = *( (unsigned long int *)(param_info.buf) );
      snprintf( info_tbl_ptr->call_info.address, DS_CALL_INFO_IP_ADDR_MAX_LEN+1, "%d.%d.%d.%d%c",
               IPCONV(tmp_ipaddr, 3), IPCONV(tmp_ipaddr, 2), IPCONV(tmp_ipaddr, 1), IPCONV(tmp_ipaddr, 0), '\0' );
    }
    else /* handle non IPv4 addr */
    {
      snprintf( info_tbl_ptr->call_info.address, DS_CALL_INFO_IP_ADDR_MAX_LEN,"%s", "unknown\0" );
    }

    QCRIL_LOG_DEBUG( "ip addr [%s]", info_tbl_ptr->call_info.address );
  }

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
  /* Get the gateway and update local call tbl */
  if ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                DATA_CALL_INFO_GATEWAY,
                                &param_info ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "unable to get gateway, ret unknown, dsi_hndl [%x]", (unsigned int)info_tbl_ptr->dsi_hndl );
  }
  else
  {
    if ( param_info.len == QCRIL_IPV4_ADDR )
    {
      tmp_ipaddr = *( (unsigned long int *)(param_info.buf) );
      snprintf( info_tbl_ptr->call_info.gateway, DS_CALL_INFO_IP_ADDR_MAX_LEN+1, "%d.%d.%d.%d%c",
        IPCONV(tmp_ipaddr, 3), IPCONV(tmp_ipaddr, 2), IPCONV(tmp_ipaddr, 1), IPCONV(tmp_ipaddr, 0), '\0' );
    }
    else
    {
      snprintf( info_tbl_ptr->call_info.gateway, DS_CALL_INFO_IP_ADDR_MAX_LEN, "%s", "unknown\0" );
    }

    QCRIL_LOG_DEBUG( "gateway [%s]", info_tbl_ptr->call_info.gateway );
  }

  /* Get primary dns address and update local call tbl */
  int tmp_primdnsaddr = 0;
  int tmp_secdnsaddr = 0;

  if ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                DATA_CALL_INFO_PRIM_DNS_ADDR,
                                &param_info ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "unable to get dns address, ret unknown, dsi_hndl [%x]", (unsigned int)info_tbl_ptr->dsi_hndl );
  }
  else
  {
    if ( param_info.len == QCRIL_IPV4_ADDR )
    {
      tmp_primdnsaddr = *( (unsigned long int *)(param_info.buf) );
    }
    else
    {
      snprintf( info_tbl_ptr->call_info.dns_address, DS_CALL_INFO_IP_ADDR_MAX_LEN, "%s", "unknown\0" );
    }
  }

  /* Get secondary dns address and update local call tbl */
  if ( dsi_get_data_call_param( info_tbl_ptr->dsi_hndl,
                                DATA_CALL_INFO_SECO_DNS_ADDR,
                                &param_info ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "unable to get dns address, ret unknown, dsi_hndl [%x]", (unsigned int)info_tbl_ptr->dsi_hndl );
  }
  else
  {
    if ( param_info.len == QCRIL_IPV4_ADDR )
    {
      tmp_secdnsaddr = *( (unsigned long int *)(param_info.buf) );
    }
    else
    {
      snprintf( info_tbl_ptr->call_info.dns_address, DS_CALL_INFO_IP_ADDR_MAX_LEN, "%s", "unknown\0" );
    }
  }
   /*Copy both primary and secondary dns seperated by a space and ends with '\0' as expected by RIL */
   snprintf( info_tbl_ptr->call_info.dns_address, DS_CALL_INFO_DNS_ADDR_MAX_LEN+1, "%d.%d.%d.%d%c%d.%d.%d.%d%c",
     IPCONV(tmp_primdnsaddr, 3), IPCONV(tmp_primdnsaddr, 2), IPCONV(tmp_primdnsaddr, 1), IPCONV(tmp_primdnsaddr, 0), ' ',
     IPCONV(tmp_secdnsaddr, 3), IPCONV(tmp_secdnsaddr, 2), IPCONV(tmp_secdnsaddr, 1), IPCONV(tmp_secdnsaddr, 0), '\0' );

   QCRIL_LOG_DEBUG( "dnses are [%s]", info_tbl_ptr->call_info.dns_address );

#endif /*((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))*/
}/* qcril_data_util_fill_call_params() */

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
/*=========================================================================
  FUNCTION:  qcril_data_util_update_call_state

===========================================================================*/
/*!
    @brief
    Updates call state.

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_update_call_state
(
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  int call_state,
  int status
)
{
  QCRIL_DS_ASSERT( ( info_tbl_ptr->index < MAX_CONCURRENT_UMTS_DATA_CALLS ),
                   "validate info_tbl index value range" );

  /* lock call_tbl while updating */
  QCRIL_DATA_MUTEX_LOCK(&call_tbl_mutex);

  /* Point call table into info table */
  call_tbl[ info_tbl_ptr->index ].active    = call_state;
  call_tbl[ info_tbl_ptr->index ].type      = info_tbl_ptr->call_info.type;
  call_tbl[ info_tbl_ptr->index ].status    = status;
  call_tbl[ info_tbl_ptr->index ].addresses = info_tbl_ptr->call_info.address;
  call_tbl[ info_tbl_ptr->index ].ifname    = info_tbl_ptr->call_info.dev_name;
  call_tbl[ info_tbl_ptr->index ].dnses     = info_tbl_ptr->call_info.dns_address;
  call_tbl[ info_tbl_ptr->index ].gateways  = info_tbl_ptr->call_info.gateway;

  /* unlock call_tbl now */
  QCRIL_DATA_MUTEX_UNLOCK(&call_tbl_mutex);

}/*qcril_data_util_update_call_state*/

/*=========================================================================
  FUNCTION:  qcril_data_util_create_setup_rsp

===========================================================================*/
/*!
    @brief
    Create RIL REQ SETUP response.

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_create_setup_rsp
(
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  qcril_data_call_response_type *rsp_ptr
)
{
  /* Transfer the call state */
  memcpy( &rsp_ptr->setup_rsp,
          &call_tbl[ info_tbl_ptr->index ],
          sizeof(rsp_ptr->setup_rsp) );
  rsp_ptr->size = sizeof(rsp_ptr->setup_rsp);
}/* qcril_data_util_create_setup_rsp()*/

#else /*((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))*/
static void qcril_data_util_update_call_state
(
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  int call_state
)
{
  QCRIL_DS_ASSERT( ( info_tbl_ptr->index < MAX_CONCURRENT_UMTS_DATA_CALLS ),
                   "validate info_tbl index value range" );

  /* lock call_tbl while updating */
  QCRIL_DATA_MUTEX_LOCK(&call_tbl_mutex);

  call_tbl[ info_tbl_ptr->index ].active  = call_state;
  //call_tbl[ info_tbl_ptr->index ].apn     = info_tbl_ptr->call_info.apn;
  //call_tbl[ info_tbl_ptr->index ].address = info_tbl_ptr->call_info.address;
  call_tbl[ info_tbl_ptr->index ].type    = info_tbl_ptr->call_info.type;

  /* unlock call_tbl now */
  QCRIL_DATA_MUTEX_UNLOCK(&call_tbl_mutex);

}/* qcril_data_util_update_call_state()*/

static void qcril_data_util_create_setup_rsp
(
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  qcril_data_call_response_type *rsp_ptr
)
{
  rsp_ptr->setup_rsp.cid      = info_tbl_ptr->call_info.cid;
  rsp_ptr->setup_rsp.dev_name = info_tbl_ptr->call_info.dev_name;
  rsp_ptr->setup_rsp.ip_addr  = info_tbl_ptr->call_info.address;
  rsp_ptr->size               = sizeof rsp_ptr->setup_rsp ;

}/* qcril_data_util_create_setup_rsp() */
#endif /*((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))*/

/*===========================================================================

  FUNCTION:  qcril_data_process_qcrilhook_go_dormant

===========================================================================*/
/*!
    @brief

    Handles RIL_REQUEST_OEM_HOOK_RAW - QCRIL_EVT_HOOK_DATA_GO_DORMANT.
    The input to this handler can be a name of an interface, in which
    case this routine will issue dormancy command on the specified interface.
    If no input is provided, Dormancy is issued on all Active interfaces.
    The request is considered to be successful on receipt of PHSYLINK_DOWN
    indication from DSS.

    @return

    None on Success, Generic failure response on Failure

    Notes: This handler assumes that dev names are enumerated as rmnet[0,1,2].
*/
/*=========================================================================*/


void qcril_data_process_qcrilhook_go_dormant
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_data_go_dormant_params_type dev_name;
  int i, ret_val;
  int rmnet_physlink_down[QCRIL_DATA_MAX_DEVS] = {FALSE,FALSE,FALSE};
  int rmnet_phsylink_status_arr_index;
  dsi_iface_ioctl_type   ioctl = DSI_IFACE_IOCTL_GO_DORMANT;

  QCRIL_DATA_MUTEX_LOCK(&qcril_data_global_mutex);
  /*-----------------------------------------------------------------------*/
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR( " Bad Input params_ptr [%#x], ret_ptr [%#x], ",
                     (unsigned int)params_ptr, (unsigned int)ret_ptr );
    goto err_bad_input;
  }
  instance_id = params_ptr->instance_id;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR( "%s", " Bad Instance Id received");
    goto err_bad_input;
  }

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s","Entered: qcril_data_process_qcrilhook_go_dormant" );
  QCRIL_LOG_DEBUG( " request = %d", params_ptr->event_id );
  QCRIL_LOG_DEBUG( " Data Length = %d", params_ptr->datalen );
  QCRIL_LOG_DEBUG( " token = %d", params_ptr->t );

  if (params_ptr->datalen == 0)
  {
    /*Issue Go Dormant on all active interfaces*/
    QCRIL_LOG_DEBUG( "%s","RIL provided NULL dev name will issue Dormancy on all active interfaces");

    for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
    {
      QCRIL_LOG_DEBUG( "on Index = %d, call_tbl[ info_tbl[i].index ].active = %d ",i,call_tbl[ info_tbl[i].index ].active);

      if(VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) &&
         (call_tbl[ info_tbl[i].index ].active  != CALL_INACTIVE) &&
         rmnet_physlink_down[GET_DEV_INSTANCE_FROM_NAME(i)] == FALSE)
      {
        QCRIL_LOG_DEBUG( "selected index = %d",i);
        if ((ret_val = dsi_iface_ioctl(info_tbl[i].dsi_hndl, ioctl)) == DSI_ERROR)
        {
          QCRIL_LOG_ERROR( "%s","Request to issue Dormancy failed.");
          goto err_label;
        }
        rmnet_physlink_down[GET_DEV_INSTANCE_FROM_NAME(i)] = TRUE;
      }
    }/* for() */
  }
  else if (params_ptr->datalen > 0 && params_ptr->datalen == QCRIL_MAX_DEV_NAME_SIZE)
  {
    memcpy(&dev_name, params_ptr->data,QCRIL_MAX_DEV_NAME_SIZE);

    /*get rmnet array index from input param and validate it*/
    int rmnet_phsylink_status_arr_index = dev_name[QCRIL_MAX_DEV_NAME_SIZE - 1] - '0';

    if (rmnet_phsylink_status_arr_index < 0 || rmnet_phsylink_status_arr_index > 2)
    {
      QCRIL_LOG_ERROR( "%s","rmnet interface name not Valid. Valid Input is rmnet[0,1,2]");
      goto err_label;
    }

    for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
    {
      /*search info table for the specified interface*/
      if (VALIDATE_LOCAL_DATA_OBJ((&info_tbl[i])) &&
          (call_tbl[ info_tbl[i].index ].active  != CALL_INACTIVE) &&
          (!strncmp(info_tbl[i].call_info.dev_name,dev_name,QCRIL_MAX_DEV_NAME_SIZE)) &&
          rmnet_physlink_down[rmnet_phsylink_status_arr_index] == FALSE)
      {
        if ((ret_val = dsi_iface_ioctl(info_tbl[i].dsi_hndl, ioctl)) == DSI_ERROR)
        {
          QCRIL_LOG_ERROR( "%s","Request to issue Dormancy failed.");
          goto err_label;
        }
        rmnet_physlink_down[rmnet_phsylink_status_arr_index] = TRUE;
      }
    } /*for*/
  } /*else*/
  else
  {
    QCRIL_LOG_ERROR( "%s","qcril_data_process_qcrilhook_go_dormant: Bad input received");
    goto err_label;
  }

  QCRIL_LOG_INFO( "%s","qcril_data_process_qcrilhook_go_dormant: EXIT with SUCCESS");

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               params_ptr->event_id,
                               NULL, 0 );
  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  return;

err_label:
  qcril_data_response_generic_failure( instance_id,
                                       params_ptr->t,
                                       params_ptr->event_id );
err_bad_input:
  QCRIL_LOG_INFO( "%s","qcril_data_process_qcrilhook_go_dormant: EXIT with ERROR");
  QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_global_mutex);
  return;
}

/*===========================================================================

  FUNCTION:  qcril_data_toggle_dormancy_indications

===========================================================================*/
/*!
    @brief

    Handles request to turn ON/OFF dormancy indications. Typically called to
    turn off indications when in power save mode  and turn back on when out
    of power save mode.

    @return QCRIL_DS_SUCCESS on success and QCRIL_DS_ERROR on failure.
*/
/*=========================================================================*/

int
qcril_data_toggle_dormancy_indications
(
  qcril_data_dormancy_ind_switch_type       dorm_ind_switch
)
{
  int i, rmnet_physlink_toggled[QCRIL_DATA_MAX_DEVS] = {FALSE,FALSE,FALSE};
  int ret_val = QCRIL_DS_ERROR;

  dsi_iface_ioctl_type   ioctl;

  if (dorm_ind_switch == DORMANCY_INDICATIONS_OFF)
  {
    ioctl = DSI_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF;
  }
  else if (dorm_ind_switch == DORMANCY_INDICATIONS_ON)
  {
    ioctl = DSI_IFACE_IOCTL_DORMANCY_INDICATIONS_ON;
  }
  else
  {
    QCRIL_LOG_ERROR( "%s","Bad input received.");
    goto err_label;
  }

  QCRIL_LOG_DEBUG( "%s","Switch ON/OFF dormancy indications on all active interfaces");

  /* save the dormancy indication switch */
  global_dorm_ind = dorm_ind_switch;

  for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    if(VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) &&
       (call_tbl[ info_tbl[i].index ].active  != CALL_INACTIVE) &&
       rmnet_physlink_toggled[GET_DEV_INSTANCE_FROM_NAME(i)] == FALSE)
    {
      QCRIL_LOG_DEBUG( "selected index = %d",i);
      if ((ret_val = dsi_iface_ioctl(info_tbl[i].dsi_hndl, ioctl)) == DSI_ERROR)
      {
        QCRIL_LOG_ERROR( "%s","Request to toggle Dormancy indication failed.");
        ret_val = QCRIL_DS_ERROR;
        goto err_label;
      }
      rmnet_physlink_toggled[GET_DEV_INSTANCE_FROM_NAME(i)] = TRUE;
    }
  }/* for() */

  QCRIL_LOG_INFO( "%s","qcril_data_toggle_dormancy_indications: EXIT with SUCCESS");
  return ret_val = QCRIL_DS_SUCCESS;

err_label:
  QCRIL_LOG_ERROR( "%s","qcril_data_toggle_dormancy_indications: EXIT with ERROR");
  return ret_val;
}
