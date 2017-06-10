/*!
  @file
  qcril_data_embms.c

  @brief
  Handles RIL EMBMS requests for DATA services.

*/

/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

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
10/17/11   sy     Initial version

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#ifdef FEATURE_DATA_EMBMS

#ifdef QCRIL_DATA_OFFTARGET
#include <netinet/in.h>
#include <errno.h>
#endif


#include "qdp.h"
#include "qcril_data.h"
#include "qcril_data_defs.h"
#include "qmi_embms_v01.h"
#include "qcril_data_utils.h"
#include "qcril_reqlist.h"

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#define SESSION_ID_INVALID 0

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/
static inline int qcril_data_get_ril_embms_tmgi_deactivated_code
(
  dsi_embms_tmgi_deactivate_reason_type_t *dsi_embms_tmgi_deactive_reason,
  int                                     *ril_embms_tmgi_deactive_code
);

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/
extern void qcril_data_net_cb(
  dsi_hndl_t         dsi_hndl,
  void               *user_data,
  dsi_net_evt_t      net_evt,
  dsi_evt_payload_t  *payload
);

/*=========================================================================
  FUNCTION:  qcril_data_get_ril_embms_tmgi_deactivated_code

===========================================================================*/
/*!
    @brief
    gets ril TMGI deactivated code given qmi deactivated reason code
    Currently, it converts every type of qmi deactivated reason code into
    generic failure. Future enhancement will include the conversion from a
    table mapping.

    @return
    FAILURE - if bad params received
    SUCCESS - other wise (must return valid RIL cause code)
*/
/*=========================================================================*/
static inline int qcril_data_get_ril_embms_tmgi_deactivated_code
(
  dsi_embms_tmgi_deactivate_reason_type_t *dsi_embms_tmgi_deactive_reason,
  int                                     *ril_embms_tmgi_deactive_code
)
{
  if(NULL == dsi_embms_tmgi_deactive_reason ||
     NULL == ril_embms_tmgi_deactive_code)
  {
    return FAILURE;
  }

  /* Return Generic failure until error code type is defined in upper layer */
  *ril_embms_tmgi_deactive_code = RIL_E_GENERIC_FAILURE;
  return SUCCESS;
}

/*===========================================================================

  FUNCTION:  qcril_data_embms_enable_data_req

===========================================================================*/
/*!
    @brief

    Handles QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_embms_enable_data_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type        instance_id;
  qcril_modem_id_e_type           modem_id;
  dsi_hndl_t                     *data_hndl;
  int                             i;
  char                            tmp_apn[ DS_CALL_INFO_APN_MAX_LEN + 1 ];
  qcril_embms_enable_response_payload_type enable_response;
  dsi_call_param_value_t          ipfamily_info;

  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_INFO( "%s", "qcril_data_embms_enable_data_req entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr ) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr);
    goto bail;
  }

  instance_id = params_ptr->instance_id;
  modem_id = params_ptr->modem_id;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  do
  {
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
      break;
    }

    /* The corresponding dsi_cb_tbl entry should have already been
       cleaned up. But, perform cleanup just in case */
    qcril_data_clear_dsi_cb_tbl_entry(&dsi_cb_tbl[i]);

    /* Allocate CID, copy APN and other call params */
    info_tbl[i].info_flg    = FALSE; /* mark call_info and dev_name as invalid */
    info_tbl[i].index       = i;
    info_tbl[i].cid         = i;

    tmp_apn[0] = '\0';
    strlcpy( info_tbl[i].call_info.apn, tmp_apn , sizeof(info_tbl[i].call_info.apn));
    info_tbl[i].instance_id = instance_id;
    info_tbl[i].modem_id    = modem_id;
    info_tbl[i].pend_tok    = params_ptr->t;
    info_tbl[i].pend_req    = QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ;
    QCRIL_LOG_INFO( "DEBUG:  %s step %d - info_tbl_ptr->pend_tok[0x%x] "
                    "info_tbl_ptr->pend_req[0x%X]",
                    __func__, 0, info_tbl[i].pend_tok, info_tbl[i].pend_req );
    info_tbl[i].qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
    info_tbl[i].dsi_hndl    = (void *)QCRIL_DATA_HNDL_INVALID;
    info_tbl[i].qdp_umts_profile_id = QCRIL_INVALID_PROFILE_ID;
    info_tbl[i].qdp_cdma_profile_id = QCRIL_INVALID_PROFILE_ID;
    info_tbl[i].self        = &info_tbl[i];

    /* store default call fail error */
    info_tbl[i].call_info.inactiveReason = PDP_FAIL_ERROR_UNSPECIFIED;

    /*Must destroy the list when disable API is implemented.*/
    list_init(&info_tbl[i].embms_txns_list);

    if ( ( info_tbl[i].dsi_hndl =
           dsi_get_data_srvc_hndl( qcril_data_net_cb,
                                 ( void *)&dsi_cb_tbl[i] ) ) == NULL )
    {
      QCRIL_LOG_ERROR( "%s", "unable to get dsi hndl" );
      break;
    }

    /* Update the dsi_cb_tbl[] entry with the required state information */
    qcril_data_update_dsi_cb_tbl_entry(&dsi_cb_tbl[i],
                                       instance_id,
                                       modem_id,
                                       params_ptr->t,
                                       info_tbl[i].dsi_hndl,
                                       &info_tbl[i]);

    QCRIL_LOG_DEBUG("info_tbl[%d] has reserved dsi_hndl[0x%x]",
                     i, info_tbl[i].dsi_hndl);

    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

    /* Currently RIL does not specify any IP family preference for
     * making an embms call. However within the qdi library which
     * handles call requests and responses, the IP preference defaults
     * to dual-IP. This causes issues while processing packet service
     * indication messages because dsi and qdi libraries have different
     * IP preferences. The indication gets dropped and the call tech
     * within the dsi handle is not updated. Until RIL explicitly sets
     * an IP preference, we will also use V4V6 as default to match with
     * the default value within qdi lib */
    ipfamily_info.buf_val = NULL;
    ipfamily_info.num_val = DSI_IP_VERSION_4_6;

    if ( dsi_set_data_call_param( info_tbl[i].dsi_hndl,
                                  DSI_CALL_INFO_IP_VERSION,
                                  &ipfamily_info ) != DSI_SUCCESS )
    {
      QCRIL_LOG_ERROR( "%s", "Unable to set ip version preference" );
      break;
    }

    if ( ( dsi_embms_enable( info_tbl[i].dsi_hndl ) ) != DSI_SUCCESS )
    {
      QCRIL_LOG_ERROR( "unable to enable EMBMS, index [%d]", info_tbl[i].index );
      QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

      memset ( &enable_response, 0, sizeof( enable_response ) );
      enable_response.call_id        = 0;
      enable_response.cause          = RIL_E_GENERIC_FAILURE;
      enable_response.resp_code      = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;

      qcril_event_queue( instance_id,
                         modem_id,
                         QCRIL_DATA_ON_STACK,
                         QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_CON,
                         (void *) &enable_response,
                         sizeof( enable_response ),
                         (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

      break;
    }
    ril_req_res = RIL_E_SUCCESS;
  }while(FALSE);

  if (RIL_E_SUCCESS != ril_req_res)
  {
    /* clean up if we had reserved an info tbl entry */
    if (i < MAX_CONCURRENT_UMTS_DATA_CALLS)
    {
      if (VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) && info_tbl[i].dsi_hndl)
      {
        qcril_data_cleanup_call_state(&info_tbl[i]);
      }

      if (VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) &&
          info_tbl[i].pend_tok == params_ptr->t)
      {
        if (info_tbl[i].qdp_umts_profile_id != QCRIL_INVALID_PROFILE_ID)
        {
          QCRIL_LOG_DEBUG("releasing QDP profile id [%d]",
                        info_tbl[i].qdp_umts_profile_id);
          qdp_profile_release(info_tbl[i].qdp_umts_profile_id);
        }
        if (info_tbl[i].qdp_cdma_profile_id != QCRIL_INVALID_PROFILE_ID)
        {
          QCRIL_LOG_DEBUG("releasing QDP profile id [%d]",
                           info_tbl[i].qdp_cdma_profile_id);
          qdp_profile_release(info_tbl[i].qdp_cdma_profile_id);
        }
      }
    }/* i < MAX_CONCURRENT_UMTS_DATA_CALLS */
  }/* RIL_E_SUCCESS != ril_req_res */

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

bail:
  QCRIL_LOG_INFO( "qcril_data_embms_enable_data_req completed %d", (int) ril_req_res );
}

/*===========================================================================

  FUNCTION:  qcril_data_embms_activate_tmgi

===========================================================================*/
/*!
    @brief

    Handles QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_embms_activate_tmgi
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type                 instance_id;
  qcril_modem_id_e_type                    modem_id;
  qcril_request_resp_params_type           resp;
  unsigned char                            cid;
  unsigned int                             i;
  int                                      dsi_tmgi_deactivate_reason;
  dsi_embms_tmgi_status_field_type         status_field = CHECK_ACTIVATE_STATUS;

  qcril_reqlist_u_type                     u_info;
  qcril_reqlist_public_type                reqlist_entry;

  int       qcril_tmgi_deactivate_reason =QCRIL_DATA_EMBMS_ERROR_UNKNOWN;
  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;

  embms_activate_tmgi_req_msg_v01       *activate_req;
  embms_activate_tmgi_resp_msg_v01       activate_resp;
  qcril_data_embms_tmgi_requests_type    *tmgi_txn = NULL;

  QCRIL_LOG_INFO( "%s", "qcril_data_embms_activate_tmgi entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr ) || (NULL == params_ptr->data))
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr);
    goto bail;
  }

  instance_id  = params_ptr->instance_id;
  modem_id     = params_ptr->modem_id;
  activate_req = params_ptr->data;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_LOG_INFO("qcril_data_embms_activate_tmgi dbg_trace_id:%d",
                  activate_req->dbg_trace_id);
  cid = activate_req->call_id;

  /* Print TMGI SAI and EARFCN values */
  QCRIL_LOG_DEBUG("qcril_data_embms_activate_tmgi TMGI:%x%x%x%x%x%x",
                                     activate_req->tmgi_info.tmgi[0],
                                     activate_req->tmgi_info.tmgi[1],
                                     activate_req->tmgi_info.tmgi[2],
                                     activate_req->tmgi_info.tmgi[3],
                                     activate_req->tmgi_info.tmgi[4],
                                     activate_req->tmgi_info.tmgi[5]);
  QCRIL_LOG_DEBUG("qcril_data_embms_activate_tmgi preemption priority:%d",activate_req->preemption_priority);
  for( i = 0; i < activate_req->earfcnlist_len; ++i)
  {
    QCRIL_LOG_DEBUG("qcril_data_embms_activate_tmgi earfcn:%d",activate_req->earfcnlist[i]);
  }
  if( TRUE == activate_req->saiList_valid )
  {
    QCRIL_LOG_DEBUG("qcril_data_embms_activate_tmgi SAI list length :%d",activate_req->saiList_len);
    for( i = 0; i < activate_req->saiList_len; ++i)
    {
      QCRIL_LOG_DEBUG("qcril_data_embms_activate_tmgi SAI :%d",activate_req->saiList[i]);
    }
  }

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  do
  {
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
      break;
    }

    /* Update the dsi_cb_tbl[] entry with the required state information */
    qcril_data_update_dsi_cb_tbl_entry(&dsi_cb_tbl[i],
                                       instance_id,
                                       modem_id,
                                       params_ptr->t,
                                       info_tbl[i].dsi_hndl,
                                       &info_tbl[i]);

    QCRIL_LOG_DEBUG("info_tbl[%d] has reserved dsi_hndl[0x%x]",
                     i, info_tbl[i].dsi_hndl);

    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

    tmgi_txn = malloc( sizeof(qcril_data_embms_tmgi_requests_type));

    if( !tmgi_txn )
    {
      /* Fail to insert entry to ReqList */
      QCRIL_LOG_ERROR( "%s", "failed to allocate a transaction" );
      break;
    }

    memset(tmgi_txn, 0, sizeof(qcril_data_embms_tmgi_requests_type));
    tmgi_txn->pend_req = QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI;
    tmgi_txn->pend_tok = params_ptr->t;

    memcpy((char *)(&tmgi_txn->tmgi[0]),
           (char *)(&activate_req->tmgi_info.tmgi[0]),
           QCRIL_DATA_EMBMS_TMGI_LEN );

    /* Append to list of Pending TMGI transactions */
    list_push_back(&info_tbl[i].embms_txns_list,  &tmgi_txn->link);

    QCRIL_LOG_DEBUG("Inserted request:%s, and token:%d, into the transactions list",
                    qcril_log_lookup_event_name(tmgi_txn->pend_req),
                    qcril_log_get_token_id(tmgi_txn->pend_tok ));

    /* Insert local info tbl ref in reqlist */
    u_info.ds.info = (void *) &info_tbl[i];
    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 modem_id,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI,
                                 &u_info,
                                 &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to insert entry to ReqList */
      /* Release tmgi txn object */
      list_pop_item( &info_tbl[i].embms_txns_list, &tmgi_txn->link );

      QCRIL_LOG_DEBUG("%s", "Failed to intert entry to ReqList");
      QCRIL_LOG_DEBUG("deleting tmgi transaction from the list, request:%s, token:%d",
                      qcril_log_lookup_event_name(tmgi_txn->pend_req),
                      qcril_log_get_token_id(tmgi_txn->pend_tok ))
      free( tmgi_txn );

      break;
    }

    QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, tmgi operation" );

    if( DSI_SUCCESS != dsi_embms_tmgi_activate(info_tbl[i].dsi_hndl,
                                               (char *)(&activate_req->tmgi_info.tmgi[0]),
                                               (unsigned int*)&activate_req->earfcnlist[0],
                                               activate_req->earfcnlist_len,
                                               (unsigned long)activate_req->preemption_priority,
                                               (unsigned int*)&activate_req->saiList[0],
                                               (unsigned char)activate_req->saiList_len,
                                               activate_req->dbg_trace_id))

    {
      QCRIL_LOG_ERROR(" unable to activate TMGI, index [%d]", info_tbl[i].index );

      if ( SUCCESS == dsi_embms_get_tmgi_deactivate_reason(info_tbl[i].dsi_hndl,status_field,
                                                           &dsi_tmgi_deactivate_reason) )
      {
        QCRIL_LOG_ERROR("TMGI activate failure reason QMI_WDS:%d", dsi_tmgi_deactivate_reason);
        if (SUCCESS == qcril_data_utils_embms_get_ril_status_code(dsi_tmgi_deactivate_reason,
                                                      &qcril_tmgi_deactivate_reason))
        {
          QCRIL_LOG_ERROR("TMGI activate failure reason QCRIL_STATUS_CODE:%d",
                          qcril_tmgi_deactivate_reason );
        }
        else
        {
          QCRIL_LOG_ERROR("%s", " unable to get QCRIL TMGI activate failure reason");
        }
      }
      else
      {
        QCRIL_LOG_ERROR("%s", "dsi_get_embms_tmgi_deactivate_reason failed");
      }

      break;
    }/* DSI_SUCCESS != dsi_embms_tmgi_activate */

    ril_req_res = RIL_E_SUCCESS;
  } while ( FALSE );

  if( RIL_E_SUCCESS != ril_req_res )
  {
    QCRIL_LOG_ERROR("%s","activate tmgi request returned Error");

    /* Release tmgi txn object */
    if(NULL != tmgi_txn)
    {
      list_pop_item( &info_tbl[i].embms_txns_list, &tmgi_txn->link );

      QCRIL_LOG_DEBUG("deleting tmgi transaction from the list, request:%s, token:%d",
                      qcril_log_lookup_event_name(tmgi_txn->pend_req),
                      qcril_log_get_token_id(tmgi_txn->pend_tok ))

      free( tmgi_txn );
    }

    /* prepare response */
    memset ( &activate_resp, 0, sizeof( activate_resp ) );
    activate_resp.call_id_valid = TRUE;
    activate_resp.call_id = cid;
    activate_resp.resp_code          = qcril_tmgi_deactivate_reason;
    activate_resp.dbg_trace_id       = activate_req->dbg_trace_id;
    activate_resp.tmgi_info_valid    = TRUE;
    activate_resp.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
    memcpy(&activate_resp.tmgi_info.tmgi[0],
           &activate_req->tmgi_info.tmgi[0],
           TMGI_LENGTH_MAX_V01);

    qcril_default_request_resp_params( instance_id,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       ril_req_res,/* revisit on error code */
                                       &resp );

    resp.resp_pkt = &activate_resp;
    resp.resp_len = sizeof( activate_resp );
    qcril_send_request_response( &resp );
    QCRIL_LOG_ERROR("activate tmgi request failed, sending response dbg_trace_id: %d",
                    activate_resp.dbg_trace_id);
  }
  /* else {} RIL_E_SUCCESS == ril_req_res
     activate response will be sent from callback function
   */

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

bail:
  QCRIL_LOG_INFO( "qcril_data_embms_activate_tmgi completed %d", (int) ril_req_res );
}

/*===========================================================================

  FUNCTION:  qcril_data_embms_deactivate_tmgi

===========================================================================*/
/*!
    @brief

    Handles QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_embms_deactivate_tmgi
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type                 instance_id;
  qcril_modem_id_e_type                    modem_id;
  qcril_request_resp_params_type           resp;
  unsigned char                            cid;
  int                                      i;
  int                                      dsi_tmgi_deactivate_reason;
  dsi_embms_tmgi_status_field_type         status_field = CHECK_DEACTIVATE_STATUS;

  qcril_reqlist_u_type                     u_info;
  qcril_reqlist_public_type                reqlist_entry;

  int       qcril_tmgi_deactivate_reason = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;
  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;


  embms_deactivate_tmgi_req_msg_v01       *deactivate_req;
  embms_deactivate_tmgi_resp_msg_v01       deactivate_resp;
  qcril_data_embms_tmgi_requests_type     *tmgi_txn = NULL;

  QCRIL_LOG_INFO( "%s", "qcril_data_embms_deactivate_tmgi entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr ) || (NULL == params_ptr->data))
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr);
    goto bail;
  }

  instance_id    = params_ptr->instance_id;
  modem_id       = params_ptr->modem_id;
  deactivate_req = params_ptr->data;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_LOG_INFO("qcril_data_embms_deactivate_tmgi dbg_trace_id:%d",
                 deactivate_req->dbg_trace_id);
  cid = deactivate_req->call_id;

  /* Print TMGI SAI and EARFCN values */
  QCRIL_LOG_DEBUG("qcril_data_embms_deactivate_tmgi TMGI:%x%x%x%x%x%x",
                                     deactivate_req->tmgi_info.tmgi[0],
                                     deactivate_req->tmgi_info.tmgi[1],
                                     deactivate_req->tmgi_info.tmgi[2],
                                     deactivate_req->tmgi_info.tmgi[3],
                                     deactivate_req->tmgi_info.tmgi[4],
                                     deactivate_req->tmgi_info.tmgi[5]);

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  do
  {
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
      break;
    }

    /* Update the dsi_cb_tbl[] entry with the required state information */
    qcril_data_update_dsi_cb_tbl_entry(&dsi_cb_tbl[i],
                                       instance_id,
                                       modem_id,
                                       params_ptr->t,
                                       info_tbl[i].dsi_hndl,
                                       &info_tbl[i]);

    QCRIL_LOG_DEBUG("info_tbl[%d] has reserved dsi_hndl[0x%x]",
                    i, info_tbl[i].dsi_hndl);

    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

    tmgi_txn = malloc( sizeof(qcril_data_embms_tmgi_requests_type) );

    if( !tmgi_txn )
    {
      /* Fail to insert entry to ReqList */
      QCRIL_LOG_ERROR( "%s", "failed to allocate a transaction" );
      break;
    }

    memset(tmgi_txn, 0, sizeof(qcril_data_embms_tmgi_requests_type));
    tmgi_txn->pend_req = QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI;
    tmgi_txn->pend_tok = params_ptr->t;

    memcpy((char *)(&tmgi_txn->deact_tmgi[0]),
           (char *)(&deactivate_req->tmgi_info.tmgi[0]),
           QCRIL_DATA_EMBMS_TMGI_LEN );

    /* Append to list of Pending TMGI transactions */
    list_push_back(&info_tbl[i].embms_txns_list, &tmgi_txn->link);

    QCRIL_LOG_DEBUG("Inserted request:%s, and token:%d, into the transactions list",
                    qcril_log_lookup_event_name(tmgi_txn->pend_req),
                    qcril_log_get_token_id(tmgi_txn->pend_tok ));


    /* Insert local info tbl ref in reqlist */
    u_info.ds.info = (void *) &info_tbl[i];
    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 modem_id,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI,
                                 &u_info,
                                 &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to insert entry to ReqList */
      /* Release tmgi txn object */
      list_pop_item( &info_tbl[i].embms_txns_list, &tmgi_txn->link );
      QCRIL_LOG_DEBUG("%s", "Failed to intert entry to ReqList");
      QCRIL_LOG_DEBUG("deleting tmgi transaction from the list, request:%s, token:%d", 
                      qcril_log_lookup_event_name(tmgi_txn->pend_req),
                      qcril_log_get_token_id(tmgi_txn->pend_tok ));
      free( tmgi_txn );

      break;
    }

    QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, tmgi operation" );

    if( DSI_SUCCESS != dsi_embms_tmgi_deactivate(info_tbl[i].dsi_hndl,
                                                (char *)(&deactivate_req->tmgi_info.tmgi[0]),
                                                 deactivate_req->dbg_trace_id))
    {
      QCRIL_LOG_ERROR(" unable to deactivate TMGI, index [%d]", info_tbl[i].index );

      if ( SUCCESS == dsi_embms_get_tmgi_deactivate_reason(info_tbl[i].dsi_hndl,status_field,
                                                           &dsi_tmgi_deactivate_reason) )
      {
        QCRIL_LOG_ERROR("TMGI deactivate failure reason QMI_WDS:%d", dsi_tmgi_deactivate_reason);
        if (SUCCESS == qcril_data_utils_embms_get_ril_status_code(dsi_tmgi_deactivate_reason,
                                                      &qcril_tmgi_deactivate_reason))
        {
          QCRIL_LOG_ERROR("TMGI deactivate failure reason QCRIL_STATUS_CODE:%d",
                          qcril_tmgi_deactivate_reason );
        }
        else
        {
          QCRIL_LOG_ERROR("%s", " unable to get QCRIL TMGI deactivate failure reason");
        }
      }
      else
      {
        QCRIL_LOG_ERROR("%s", "dsi_get_embms_tmgi_deactivate_reason failed");
      }

      break;
    }/* DSI_SUCCESS != dsi_embms_tmgi_deactivate */

    ril_req_res = RIL_E_SUCCESS;
  } while ( FALSE );

  if( RIL_E_SUCCESS != ril_req_res )
  {
    QCRIL_LOG_ERROR("%s","deactivate tmgi request returned Error");

    /* Release tmgi txn object */
    if(NULL != tmgi_txn)
    {
      list_pop_item( &info_tbl[i].embms_txns_list, &tmgi_txn->link );
      QCRIL_LOG_DEBUG("deleting tmgi transaction from the list, request:%s, token:%d",
                      qcril_log_lookup_event_name(tmgi_txn->pend_req),
                      qcril_log_get_token_id(tmgi_txn->pend_tok ));
      free( tmgi_txn );
    }

    /* prepare response */
    memset ( &deactivate_resp, 0, sizeof( deactivate_resp ) );
    deactivate_resp.call_id_valid = TRUE;
    deactivate_resp.call_id = cid;
    deactivate_resp.resp_code = qcril_tmgi_deactivate_reason;
    deactivate_resp.dbg_trace_id = deactivate_req->dbg_trace_id;
    deactivate_resp.tmgi_info_valid = TRUE;
    deactivate_resp.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
    memcpy(&(deactivate_resp.tmgi_info.tmgi[0]),
           &deactivate_req->tmgi_info.tmgi[0],
           TMGI_LENGTH_MAX_V01);

    qcril_default_request_resp_params( instance_id,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       ril_req_res,/* revisit on error code */
                                       &resp );

    resp.resp_pkt = &deactivate_resp;
    resp.resp_len = sizeof( deactivate_resp );
    qcril_send_request_response( &resp );

    QCRIL_LOG_ERROR("deactivate tmgi request failed sending response dbg_trace_id: %d",
                    deactivate_resp.dbg_trace_id);
  }
  /* else {} RIL_E_SUCCESS == ril_req_res
     deactivate response will be sent from callback function
   */
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

bail:
  QCRIL_LOG_INFO( "qcril_data_embms_deactivate_tmgi completed %d", (int) ril_req_res );
}


/*===========================================================================

  FUNCTION:  qcril_data_embms_activate_deactivate_tmgi

===========================================================================*/
/*!
    @brief

    Handles QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_embms_activate_deactivate_tmgi
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type                 instance_id;
  qcril_modem_id_e_type                    modem_id;
  qcril_request_resp_params_type           resp;
  unsigned char                            cid;
  unsigned int                             i;
  int                                      dsi_tmgi_deactivate_reason;
  dsi_embms_tmgi_status_field_type         status_field = CHECK_ACTIVATE_DEACTIVATE_STATUS;

  qcril_reqlist_u_type                     u_info;
  qcril_reqlist_public_type                reqlist_entry;

  int       qcril_tmgi_deactivate_reason = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;
  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;

  embms_activate_deactivate_tmgi_req_msg_v01       *act_deactivate_req;
  embms_activate_deactivate_tmgi_resp_msg_v01       act_deactivate_resp;
  qcril_data_embms_tmgi_requests_type    *tmgi_txn = NULL;

  QCRIL_LOG_INFO( "%s", "qcril_data_embms_act_deactivate_tmgi entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr )|| (NULL == params_ptr->data))
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr);
    goto bail;
  }

  instance_id  = params_ptr->instance_id;
  modem_id     = params_ptr->modem_id;
  act_deactivate_req = params_ptr->data;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_LOG_INFO("qcril_data_embms_activate_deactivate_tmgi dbg_trace_id:%d",
                 act_deactivate_req->dbg_trace_id);
  cid = act_deactivate_req->call_id;

  /* Print TMGI SAI and EARFCN values */
  QCRIL_LOG_DEBUG("qcril_data_embms_activate_deactivate_tmgi Activate TMGI:%x%x%x%x%x%x",
                                               act_deactivate_req->act_tmgi_info.tmgi[0],
                                               act_deactivate_req->act_tmgi_info.tmgi[1],
                                               act_deactivate_req->act_tmgi_info.tmgi[2],
                                               act_deactivate_req->act_tmgi_info.tmgi[3],
                                               act_deactivate_req->act_tmgi_info.tmgi[4],
                                               act_deactivate_req->act_tmgi_info.tmgi[5]);
  QCRIL_LOG_DEBUG("qcril_data_embms_activate_deactivate_tmgi Deactivate TMGI:%x%x%x%x%x%x",
                                               act_deactivate_req->deact_tmgi_info.tmgi[0],
                                               act_deactivate_req->deact_tmgi_info.tmgi[1],
                                               act_deactivate_req->deact_tmgi_info.tmgi[2],
                                               act_deactivate_req->deact_tmgi_info.tmgi[3],
                                               act_deactivate_req->deact_tmgi_info.tmgi[4],
                                               act_deactivate_req->deact_tmgi_info.tmgi[5]);
  QCRIL_LOG_DEBUG("qcril_data_embms_activate_deactivate_tmgi preemption priority:%d",act_deactivate_req->preemption_priority);
  for( i = 0; i < act_deactivate_req->earfcnlist_len; ++i)
  {
    QCRIL_LOG_DEBUG("qcril_data_embms_activate_deactivate_tmgi earfcn:%d",act_deactivate_req->earfcnlist[i]);
  }
  if( TRUE == act_deactivate_req->saiList_valid )
  {
    QCRIL_LOG_DEBUG("qcril_data_embms_activate_deactivate_tmgi SAI list length :%d",act_deactivate_req->saiList_len);
    for( i = 0; i < act_deactivate_req->saiList_len; ++i)
    {
      QCRIL_LOG_DEBUG("qcril_data_embms_activate_deactivate_tmgi SAI :%d",act_deactivate_req->saiList[i]);
    }
  }

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);
  
  do
  {
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
      break;
    }

    /* Update the dsi_cb_tbl[] entry with the required state information */
    qcril_data_update_dsi_cb_tbl_entry(&dsi_cb_tbl[i],
                                       instance_id,
                                       modem_id,
                                       params_ptr->t,
                                       info_tbl[i].dsi_hndl,
                                       &info_tbl[i]);

    QCRIL_LOG_DEBUG("info_tbl[%d] has reserved dsi_hndl[0x%x]",
                     i, info_tbl[i].dsi_hndl);

    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

    tmgi_txn = malloc( sizeof(qcril_data_embms_tmgi_requests_type));

    if( !tmgi_txn )
    {
      /* Fail to insert entry to ReqList */
      QCRIL_LOG_ERROR( "%s", "failed to allocate a transaction" );
      break;
    }

    memset(tmgi_txn, 0, sizeof(qcril_data_embms_tmgi_requests_type));
    tmgi_txn->pend_req = QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI;
    tmgi_txn->pend_tok = params_ptr->t;

    memcpy((char *)(&tmgi_txn->deact_tmgi[0]),
           (char *)(&act_deactivate_req->deact_tmgi_info.tmgi[0]),
           QCRIL_DATA_EMBMS_TMGI_LEN );
    memcpy((char *)(&tmgi_txn->tmgi[0]),
           (char *)(&act_deactivate_req->act_tmgi_info.tmgi[0]),
           QCRIL_DATA_EMBMS_TMGI_LEN );

    /* Append to list of Pending TMGI transactions */
    list_push_back(&info_tbl[i].embms_txns_list,  &tmgi_txn->link);

    QCRIL_LOG_DEBUG("Inserted request:%s, and token:%d, into the transactions list",
                    qcril_log_lookup_event_name(tmgi_txn->pend_req),
                    qcril_log_get_token_id(tmgi_txn->pend_tok ));

    /* Insert local info tbl ref in reqlist */
    u_info.ds.info = (void *) &info_tbl[i];
    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 modem_id,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI,
                                 &u_info,
                                 &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to insert entry to ReqList */
      /* Release tmgi txn object */
      list_pop_item( &info_tbl[i].embms_txns_list, &tmgi_txn->link );
      QCRIL_LOG_DEBUG("%s", "Failed to intert entry to ReqList");
      QCRIL_LOG_DEBUG("deleting tmgi transaction from the list, request:%s, token:%d",
                      qcril_log_lookup_event_name(tmgi_txn->pend_req),
                      qcril_log_get_token_id(tmgi_txn->pend_tok ))
      free( tmgi_txn );

      break;
    }

    QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, tmgi operation" );

    if( DSI_SUCCESS != dsi_embms_tmgi_activate_deactivate(info_tbl[i].dsi_hndl,
                                               (char *)(&act_deactivate_req->act_tmgi_info.tmgi[0]),
                                               (unsigned int*)&act_deactivate_req->earfcnlist[0],
                                               act_deactivate_req->earfcnlist_len,
                                               (unsigned long)act_deactivate_req->preemption_priority,
                                               (char *)(&act_deactivate_req->deact_tmgi_info.tmgi[0]),
                                               (unsigned int*)&act_deactivate_req->saiList[0],
                                               (unsigned char)act_deactivate_req->saiList_len,
                                               act_deactivate_req->dbg_trace_id))

    {
      QCRIL_LOG_ERROR(" unable to activate TMGI, index [%d]", info_tbl[i].index );

      if ( SUCCESS == dsi_embms_get_tmgi_deactivate_reason(info_tbl[i].dsi_hndl,status_field,
                                                           &dsi_tmgi_deactivate_reason) )
      {
        QCRIL_LOG_ERROR("TMGI activate_deactivate failure reason QMI_WDS:%d", dsi_tmgi_deactivate_reason);
        if (SUCCESS == qcril_data_utils_embms_get_ril_status_code(dsi_tmgi_deactivate_reason,
                                                      &qcril_tmgi_deactivate_reason))
        {
          QCRIL_LOG_ERROR("TMGI activate_deactivate failure reason QCRIL_STATUS_CODE:%d",
                          qcril_tmgi_deactivate_reason );
        }
        else
        {
          QCRIL_LOG_ERROR("%s", " unable to get QCRIL TMGI activate_deactivate failure reason");
        }
      }
      else
      {
        QCRIL_LOG_ERROR("%s", "dsi_get_embms_tmgi_deactivate_reason failed");
      }

      break;
    }/* DSI_SUCCESS != dsi_embms_tmgi_activate_deactivate */

    ril_req_res = RIL_E_SUCCESS;
  } while ( FALSE );


  if( RIL_E_SUCCESS != ril_req_res )
  {
    QCRIL_LOG_ERROR("%s","activate_deactivate tmgi request returned Error");

    /* Release tmgi txn object */
    if(NULL != tmgi_txn)
    {
      list_pop_item( &info_tbl[i].embms_txns_list, &tmgi_txn->link );

      QCRIL_LOG_DEBUG("deleting tmgi transaction from the list, request:%s, token:%d",
                      qcril_log_lookup_event_name(tmgi_txn->pend_req),
                      qcril_log_get_token_id(tmgi_txn->pend_tok ))

      free( tmgi_txn );
    }

    /* prepare response */
    memset ( &act_deactivate_resp, 0, sizeof( act_deactivate_resp ) );
    act_deactivate_resp.call_id_valid          = TRUE;
    act_deactivate_resp.call_id                = cid;
    act_deactivate_resp.act_resp_code          = qcril_tmgi_deactivate_reason;
    act_deactivate_resp.deact_resp_code        = qcril_tmgi_deactivate_reason;
    act_deactivate_resp.dbg_trace_id           = act_deactivate_req->dbg_trace_id;

    act_deactivate_resp.act_tmgi_info_valid    = TRUE;
    act_deactivate_resp.act_tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
    memcpy(&(act_deactivate_resp.act_tmgi_info.tmgi[0]),
           &(act_deactivate_req->act_tmgi_info.tmgi[0]),
           TMGI_LENGTH_MAX_V01);

    act_deactivate_resp.deact_tmgi_info_valid    = TRUE;
    act_deactivate_resp.deact_tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
    memcpy(&(act_deactivate_resp.deact_tmgi_info.tmgi[0]),
           &(act_deactivate_req->deact_tmgi_info.tmgi[0]),
           TMGI_LENGTH_MAX_V01);

    qcril_default_request_resp_params( instance_id,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       ril_req_res,/* revisit on error code */
                                       &resp );

    resp.resp_pkt = &act_deactivate_resp;
    resp.resp_len = sizeof( act_deactivate_resp );
    qcril_send_request_response( &resp );
    QCRIL_LOG_ERROR("act_deactivate tmgi request failed sending response dbg_trace_id: %d",
                    act_deactivate_resp.dbg_trace_id);
  }
  /* else {} RIL_E_SUCCESS == ril_req_res
     activate_deactivate response will be sent from callback function
   */

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

bail:
  QCRIL_LOG_INFO( "qcril_data_embms_activate_deactivate_tmgi completed %d", (int) ril_req_res );
}

/*===========================================================================

  FUNCTION:  qcril_data_embms_get_available_tmgi

===========================================================================*/
/*!
    @brief

    Handles QCRIL_EVT_HOOK_EMBMS_GET_AVAILABLE_TMGI.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_embms_get_available_tmgi
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type                 instance_id;
  qcril_modem_id_e_type                    modem_id;
  qcril_request_resp_params_type           resp;
  unsigned char                            cid;
  uint32_t                                 i, j;
  dsi_embms_tmgi_info_type                 embms_tmgi;
  dsi_embms_tmgi_list_type                 list_type;

  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;

  embms_get_available_tmgi_req_msg_v01    *get_available_req = NULL;
  embms_get_available_tmgi_resp_msg_v01    get_available_resp;

  QCRIL_LOG_INFO( "%s", "qcril_data_embms_get_available_tmgi entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr ) || (NULL == params_ptr->data))
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                       (unsigned int *)params_ptr, (unsigned int *)ret_ptr);
    goto bail;
  }

  instance_id       = params_ptr->instance_id;
  modem_id          = params_ptr->modem_id;
  get_available_req = params_ptr->data;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_LOG_INFO("qcril_data_embms_get_available_tmgi dbg_trace_id:%d",
                 get_available_req->dbg_trace_id);
  cid = get_available_req->call_id;

  /* initialize variable */
  memset(&embms_tmgi, 0, sizeof(embms_tmgi));
  list_type = QMI_WDS_EMBMS_TMGI_LIST_AVAILABLE;

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  do
  {
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
      break;
    }

    QCRIL_LOG_DEBUG("info_tbl[%d] has reserved dsi_hndl[0x%x]",
                    i, info_tbl[i].dsi_hndl);

    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );


    if (NULL == (embms_tmgi.embms_tmgi_actdeact_params.tmgi_list =
                 malloc(NUMBER_MAX_V01 * sizeof(qmi_wds_embms_tmgi_type))))
    {
      QCRIL_LOG_ERROR(" %s", " unable to allocate memory");
      break;
    }

    if( DSI_SUCCESS != dsi_embms_tmgi_list_query(info_tbl[i].dsi_hndl,
                                                 list_type,
                                                 &embms_tmgi,
                                                 get_available_req->dbg_trace_id))
    {
      QCRIL_LOG_ERROR(" unable to get available tmgi list, index [%d]", info_tbl[i].index);
      break;
    }/* DSI_SUCCESS != dsi_embms_tmgi_list_query */

    ril_req_res = RIL_E_SUCCESS;
  } while ( FALSE );

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

  /* prepare response */
  memset( &get_available_resp, 0, sizeof( get_available_resp ) );

  /* populate dbg_trace_id
     MODEM does not send debug_trace_id in resp, need to get it from request
   */
  get_available_resp.dbg_trace_id = get_available_req->dbg_trace_id;

  if(RIL_E_SUCCESS == ril_req_res)
  {
    get_available_resp.tmgi_info_valid = TRUE;
    get_available_resp.resp_code = QCRIL_DATA_EMBMS_ERROR_NONE;

    /* validate length of tmgi_list */
    get_available_resp.tmgi_info_len =
      (embms_tmgi.embms_tmgi_actdeact_params.tmgi_list_len > NUMBER_MAX_V01) ?
      NUMBER_MAX_V01 : embms_tmgi.embms_tmgi_actdeact_params.tmgi_list_len ;

    for (j = 0; j < get_available_resp.tmgi_info_len; j++)
    {
      get_available_resp.tmgi_info[j].tmgi_len = TMGI_LENGTH_MAX_V01;
      memcpy(&(get_available_resp.tmgi_info[j].tmgi[0]),
             &(embms_tmgi.embms_tmgi_actdeact_params.tmgi_list[j].tmgi[0]),
             TMGI_LENGTH_MAX_V01);
    }
  }
  else
  {
    get_available_resp.tmgi_info_valid = FALSE;
    get_available_resp.resp_code = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;
  }
  qcril_default_request_resp_params( instance_id,
                                    params_ptr->t,
                                    params_ptr->event_id,
                                    ril_req_res,/* revisit on error code */
                                    &resp );

  resp.resp_pkt = &get_available_resp;
  resp.resp_len = sizeof( get_available_resp );
  qcril_send_request_response( &resp );

  /* free memory */
  if( NULL != embms_tmgi.embms_tmgi_actdeact_params.tmgi_list )
  {
    QCRIL_LOG_INFO("%s", "release memory \n");
    free(embms_tmgi.embms_tmgi_actdeact_params.tmgi_list);
    embms_tmgi.embms_tmgi_actdeact_params.tmgi_list = NULL;
  }

bail:
  QCRIL_LOG_INFO( "qcril_data_embms_get_available_tmgi completed[%d]",
                  (int) ril_req_res );
}

/*===========================================================================

  FUNCTION:  qcril_data_embms_get_active_tmgi

===========================================================================*/
/*!
    @brief

    Handles QCRIL_EVT_HOOK_EMBMS_GET_ACTIVE_TMGI.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_embms_get_active_tmgi
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type                 instance_id;
  qcril_modem_id_e_type                    modem_id;
  qcril_request_resp_params_type           resp;
  unsigned char                            cid;
  uint32_t                                 i, j;
  dsi_embms_tmgi_info_type                 embms_tmgi;
  dsi_embms_tmgi_list_type                 list_type;

  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;

  embms_get_active_tmgi_req_msg_v01    *get_active_req = NULL;
  embms_get_active_tmgi_resp_msg_v01    get_active_resp;

  QCRIL_LOG_INFO( "%s", "qcril_data_embms_get_active_tmgi entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr ) ||(NULL == params_ptr->data))
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr);
    goto bail;
  }

  instance_id    = params_ptr->instance_id;
  modem_id       = params_ptr->modem_id;
  get_active_req = params_ptr->data;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_LOG_INFO("qcril_data_embms_get_active_tmgi dbg_trace_id:%d",
                   get_active_req->dbg_trace_id);
  cid = get_active_req->call_id;

  /* initialize variable */
  memset(&embms_tmgi, 0, sizeof(embms_tmgi));
  list_type = QMI_WDS_EMBMS_TMGI_LIST_ACTIVE;

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  do
  {
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
      break;
    }

    QCRIL_LOG_DEBUG("info_tbl[%d] has reserved dsi_hndl[0x%x]",
                    i, info_tbl[i].dsi_hndl);

    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

    if (NULL == (embms_tmgi.embms_tmgi_actdeact_params.tmgi_list =
                 malloc(NUMBER_MAX_V01 * sizeof(qmi_wds_embms_tmgi_type))))
    {
      QCRIL_LOG_ERROR(" %s", " unable to allocate memory");
      break;
    }

    if( DSI_SUCCESS != dsi_embms_tmgi_list_query(info_tbl[i].dsi_hndl,
                                                 list_type,
                                                 &embms_tmgi,
                                                 get_active_req->dbg_trace_id))
    {
      QCRIL_LOG_ERROR(" unable to get active tmgi list, index [%d]", info_tbl[i].index);
      break;
    }/* DSI_SUCCESS != dsi_embms_tmgi_list_query */

    ril_req_res = RIL_E_SUCCESS;
  } while ( FALSE );

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

  /* prepare response */
  memset( &get_active_resp, 0, sizeof( get_active_resp ) );

  /* populate dbg_trace_id
     MODEM does not send debug_trace_id in resp, need to get it from request
   */
  get_active_resp.dbg_trace_id = embms_tmgi.embms_tmgi_actdeact_params.dbg_trace_id;

  if(RIL_E_SUCCESS == ril_req_res)
  {
    get_active_resp.tmgi_info_valid = TRUE;
    get_active_resp.resp_code = QCRIL_DATA_EMBMS_ERROR_NONE;

    /* validate length of tmgi_list */
    get_active_resp.tmgi_info_len =
      (embms_tmgi.embms_tmgi_actdeact_params.tmgi_list_len > NUMBER_MAX_V01) ?
      NUMBER_MAX_V01 : embms_tmgi.embms_tmgi_actdeact_params.tmgi_list_len ;

    for (j = 0; j < get_active_resp.tmgi_info_len; j++)
    {
      get_active_resp.tmgi_info[j].tmgi_len = TMGI_LENGTH_MAX_V01;
      memcpy(&(get_active_resp.tmgi_info[j].tmgi[0]),
             &(embms_tmgi.embms_tmgi_actdeact_params.tmgi_list[j].tmgi[0]),
             TMGI_LENGTH_MAX_V01);
    }
  }
  else
  {
    get_active_resp.tmgi_info_valid = FALSE;
    get_active_resp.resp_code = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;
  }

  qcril_default_request_resp_params( instance_id,
                                    params_ptr->t,
                                    params_ptr->event_id,
                                    ril_req_res,/* revisit on error code */
                                    &resp );

  resp.resp_pkt = &get_active_resp;
  resp.resp_len = sizeof( get_active_resp );
  qcril_send_request_response( &resp );

  /* free memory */
  if(NULL != embms_tmgi.embms_tmgi_actdeact_params.tmgi_list)
  {
    QCRIL_LOG_INFO("%s", "release memory \n");
    free(embms_tmgi.embms_tmgi_actdeact_params.tmgi_list);
    embms_tmgi.embms_tmgi_actdeact_params.tmgi_list = NULL;
  }

bail:
  QCRIL_LOG_INFO( "qcril_data_embms_get_active_tmgi completed[%d]",
                  (int) ril_req_res );
}

/*===========================================================================

  FUNCTION:  qcril_data_embms_content_desc_update

===========================================================================*/
/*!
    @brief

    Handles QCRIL_EVT_HOOK_EMBMS_CONTENT_DESC_UPDATE.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_embms_content_desc_update
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type                 instance_id;
  qcril_modem_id_e_type                    modem_id;
  qcril_request_resp_params_type           resp;
  unsigned char                            cid;
  uint32_t                                 i;

  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;

  embms_update_content_desc_req_msg_v01 *get_update_req = NULL;
  embms_update_content_desc_resp_msg_v01 get_update_resp;

  QCRIL_LOG_INFO( "%s", "qcril_data_embms_content_desc_update entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr ) ||(NULL == params_ptr->data))
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr);
    goto bail;
  }

  instance_id    = params_ptr->instance_id;
  modem_id       = params_ptr->modem_id;
  get_update_req = params_ptr->data;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_LOG_INFO("qcril_data_embms_content_desc_update dbg_trace_id:%d",
                   get_update_req->dbg_trace_id);
  cid = get_update_req->call_id;

  /* Print TMGI values */
  QCRIL_LOG_DEBUG("qcril_data_embms_content_desc_update TMGI:[%X,%X,%X,%X,%X,%X]",
                                     get_update_req->tmgi_info.tmgi[0],
                                     get_update_req->tmgi_info.tmgi[1],
                                     get_update_req->tmgi_info.tmgi[2],
                                     get_update_req->tmgi_info.tmgi[3],
                                     get_update_req->tmgi_info.tmgi[4],
                                     get_update_req->tmgi_info.tmgi[5]);

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  do
  {
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
      break;
    }

    QCRIL_LOG_DEBUG("info_tbl[%d] has reserved dsi_hndl[0x%x]",
                    i, info_tbl[i].dsi_hndl);

    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

    if( DSI_SUCCESS != dsi_embms_content_desc_update(info_tbl[i].dsi_hndl,
                                                     (char *)(&get_update_req->tmgi_info.tmgi[0]),
                                                     (unsigned char)get_update_req->content_desc_valid,
                                                     get_update_req->content_desc_len,
                                                     get_update_req->content_desc,
                                                     get_update_req->dbg_trace_id) )
    {
      QCRIL_LOG_ERROR(" unable to update content desc, index [%d]", info_tbl[i].index);
      break;
    }/* DSI_SUCCESS != dsi_embms_content_desc_update */

    ril_req_res = RIL_E_SUCCESS;
  } while ( FALSE );

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

  /* prepare response */
  memset( &get_update_resp, 0, sizeof( get_update_resp ) );

  /* populate dbg_trace_id
     MODEM does not send debug_trace_id in resp, need to get it from request
   */
  get_update_resp.dbg_trace_id = get_update_req->dbg_trace_id;

  if(RIL_E_SUCCESS == ril_req_res)
  {
    get_update_resp.resp_code = QCRIL_DATA_EMBMS_ERROR_NONE;
  }
  else
  {
    get_update_resp.resp_code = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;
  }

  qcril_default_request_resp_params( instance_id,
                                    params_ptr->t,
                                    params_ptr->event_id,
                                    ril_req_res,/* revisit on error code */
                                    &resp );

  resp.resp_pkt = &get_update_resp;
  resp.resp_len = sizeof( get_update_resp );
  qcril_send_request_response( &resp );

bail:
  QCRIL_LOG_INFO( "qcril_data_embms_content_desc_update completed[%d]",
                  (int) ril_req_res );
}
#endif /* FEATURE_DATA_EMBMS */

