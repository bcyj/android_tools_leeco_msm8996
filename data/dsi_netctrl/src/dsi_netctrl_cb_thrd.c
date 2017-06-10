/*!
  @file
  dsi_netctrl_cb_thrd.c

  @brief
  implements dsi_netctrl callback processing thread

*/

/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/08/10   js      created

===========================================================================*/
#include "dsi_netctrli.h"
#include "dsi_netctrl_platform.h"
#include "dsi_netctrl_mni_cb.h"
#include "dsi_netctrl_netmgr.h"
#include "ds_cmdq.h"
#include "qdi.h"

/*===========================================================================
                    LOCAL DATA STRUCTURES
===========================================================================*/
/* dsi_netctrl_cb cmd types : contains cmd type enums */
typedef enum dsi_netctrl_cb_cmd_type_e
{
  DSI_NETCTRL_CB_CMD_QMI_WDS_IND,
  DSI_NETCTRL_CB_CMD_QMI_QOS_IND,
  DSI_NETCTRL_CB_CMD_QMI_ASYNC_RSP,
  DSI_NETCTRL_CB_CMD_QMI_SYS_IND,
  DSI_NETCTRL_CB_CMD_NETMGR,
  DSI_NETCTRL_CB_CMD_MAX
} dsi_netctrl_cb_cmd_type_t;

/* dsi_netctrl_cb cmd : contains data for pkt_srvc_ind */
typedef struct dsi_netctrl_cb_wds_ind_data_s
{
  int wds_hndl;
  qmi_service_id_type qmi_service_id;
  void * user_data;
  qmi_wds_indication_id_type ind_id;
  qmi_wds_indication_data_type ind_data;
} dsi_netctrl_cb_wds_ind_data_t;

/* dsi_netctrl_cb cmd : contains data for start/stop_nw_async_rsp */
typedef struct dsi_netctrl_cb_async_rsp_s
{
  int user_handle;
  qmi_service_id_type qmi_service_id;
  int sys_err_code;
  int qmi_err_code;
  void * user_data;
  qmi_wds_async_rsp_id_type rsp_id;
  qdi_wds_async_rsp_data_type rsp_data;
} dsi_netctrl_cb_async_rsp_t;

/* dsi_netctrl_cb cmd : contains data for qmi sys ind */
typedef struct dsi_netctrl_cb_qmi_sys_ind_data_s
{
  qmi_sys_event_type event_id;
  qmi_sys_event_info_type event_info;
  void * user_data;
} dsi_netctrl_cb_qmi_sys_ind_data_t;

/* dsi_netctrl_cb cmd : contains data for pkt_srvc_ind */
typedef struct dsi_netctrl_cb_qos_ind_data_s
{
  int qos_hndl;
  qmi_service_id_type qmi_service_id;
  void * user_data;
  qmi_qos_indication_id_type ind_id;
  qmi_qos_indication_data_type ind_data;
} dsi_netctrl_cb_qos_ind_data_t;

/* data for netmgr cb */
typedef struct dsi_netmgr_cb_s
{
  netmgr_nl_events_t event;
  netmgr_nl_event_info_t info;
  void * data;
} dsi_netmgr_cb_t;

/* global queue to hold dsi_netctrl_cb commands */
/* dsi_netctrl_cb cmd : contains data */
typedef struct dsi_netctrl_cb_cmd_data_s
{
  /* type is used to discriminate union */
  dsi_netctrl_cb_cmd_type_t type;
  union
  {
    dsi_netctrl_cb_qmi_sys_ind_data_t qmi_sys_ind;
    dsi_netctrl_cb_wds_ind_data_t wds_ind;
    dsi_netctrl_cb_qos_ind_data_t qos_ind;
    dsi_netctrl_cb_async_rsp_t async_rsp;
    dsi_netmgr_cb_t netmgr;
  } data_union;
} dsi_netctrl_cb_cmd_data_t;

/* dsi_netctrl_cb cmd : contains type and data */
typedef struct dsi_netctrl_cb_cmd_s
{
  ds_cmd_t cmd;
  dsi_netctrl_cb_cmd_data_t cmd_data;
} dsi_netctrl_cb_cmd_t;

/* global queue to hold dsi_netctrl_cb commands */
struct ds_cmdq_info_s dsi_netctrl_cb_cmdq;

/* if we accumulate more than these many commands 
 * ds_cmdq will print a warning msg */
#define DSI_NETCTRL_CB_MAX_CMDS 20

/*===========================================================================
  FUNCTION:  dsi_netctrl_cb_cmd_free
===========================================================================*/
/*!
    @brief
    releases memory for the dsi_netctrl_cb cmd

    @return
    none
*/
/*=========================================================================*/
void dsi_netctrl_cb_cmd_free (ds_cmd_t * cmd, void * data)
{
  dsi_netctrl_cb_cmd_t * cmd_buf = NULL;

  DSI_LOG_VERBOSE("%s", "dsi_netctrl_cb_cmd_free ENTRY");

  if (NULL == data ||
      NULL == cmd)
  {
    DSI_LOG_FATAL("%s", "*** memory corruption : rcvd invalid data ***");
    DSI_LOG_DEBUG("%s", "dsi_netctrl_cb_cmd_free EXIT");
    return;
  }

  cmd_buf = (dsi_netctrl_cb_cmd_t *)data;
  /* verify self-reference */
  if (cmd != &cmd_buf->cmd)
  {
    DSI_LOG_FATAL("%s", "*** memory corruption : rcvd invalid data ***");
    DSI_LOG_DEBUG("%s", "dsi_netctrl_cb_cmd_free EXIT");
    return;
  }

  /* release mem */  
  dsi_free(cmd_buf);

  DSI_LOG_VERBOSE("%s", "dsi_netctrl_cb_cmd_free EXIT");
  return;
}

/*===========================================================================
  FUNCTION:  dsi_netctrl_cb_cmd_exec
===========================================================================*/
/*!
    @brief
    This function is registered as executive function in each command that
    is posted to dsi_netctrl_cb global queue. 
    When called, this function further calls appropriate functions based
    on the command types.

    @return
    none
*/
/*=========================================================================*/
void dsi_netctrl_cb_cmd_exec (struct ds_cmd_s * cmd, void * data)
{
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;
  dsi_netctrl_cb_cmd_t * cmd_buf = NULL;

  DSI_LOG_VERBOSE("%s", "dsi_netctrl_cb_cmd_exec: ENTRY");

  do
  {
    if (NULL == cmd ||
        NULL == data)
    {
      DSI_LOG_FATAL("%s", "*** memory corruption : rcvd invalid data ***");
      break;
    }

    cmd_buf = (dsi_netctrl_cb_cmd_t *)data;
    /* verify self-refernce */
    if (&cmd_buf->cmd != cmd)
    {
      DSI_LOG_FATAL("%s", "*** memory corruption : rcvd invalid data ***");
      break;
    }

    reti = DSI_SUCCESS;
    switch(cmd_buf->cmd_data.type)
    {
    case DSI_NETCTRL_CB_CMD_QMI_WDS_IND:
      dsi_process_wds_ind
        (
          cmd_buf->cmd_data.data_union.wds_ind.wds_hndl,
          cmd_buf->cmd_data.data_union.wds_ind.qmi_service_id,
          cmd_buf->cmd_data.data_union.wds_ind.user_data,
          cmd_buf->cmd_data.data_union.wds_ind.ind_id,
          &cmd_buf->cmd_data.data_union.wds_ind.ind_data
          );
      break;
    case DSI_NETCTRL_CB_CMD_QMI_ASYNC_RSP:
      dsi_process_async_wds_rsp
        (
          cmd_buf->cmd_data.data_union.async_rsp.user_handle,
          cmd_buf->cmd_data.data_union.async_rsp.qmi_service_id,
          cmd_buf->cmd_data.data_union.async_rsp.sys_err_code,
          cmd_buf->cmd_data.data_union.async_rsp.qmi_err_code,
          cmd_buf->cmd_data.data_union.async_rsp.user_data,
          cmd_buf->cmd_data.data_union.async_rsp.rsp_id,
          &cmd_buf->cmd_data.data_union.async_rsp.rsp_data
        );
      break;
    case DSI_NETCTRL_CB_CMD_QMI_QOS_IND:
      dsi_process_qos_ind
        (
          cmd_buf->cmd_data.data_union.qos_ind.qos_hndl,
          cmd_buf->cmd_data.data_union.qos_ind.qmi_service_id,
          cmd_buf->cmd_data.data_union.qos_ind.user_data,
          cmd_buf->cmd_data.data_union.qos_ind.ind_id,
          &cmd_buf->cmd_data.data_union.qos_ind.ind_data
        );
      break;
    case DSI_NETCTRL_CB_CMD_QMI_SYS_IND:
      dsi_process_qmi_sys_ind
        (
          cmd_buf->cmd_data.data_union.qmi_sys_ind.event_id,
          &cmd_buf->cmd_data.data_union.qmi_sys_ind.event_info
        );
      break;
    case DSI_NETCTRL_CB_CMD_NETMGR:
      dsi_process_netmgr_ev
        (
          cmd_buf->cmd_data.data_union.netmgr.event,
          &cmd_buf->cmd_data.data_union.netmgr.info,
          cmd_buf->cmd_data.data_union.netmgr.data
        );
      break;
    default:
      DSI_LOG_FATAL("%s", "*** memory corruption: rcvd invalid data ***");
      reti = DSI_ERROR;
      break;
    }
    if (DSI_ERROR == reti)
    {
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_netctrl_cb_cmd_exec: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_netctrl_cb_cmd_exec: EXIT with err");
  }

}

/*===========================================================================
  FUNCTION:  dsi_qmi_wds_cmd_cb
===========================================================================*/
/*!
    @brief
    callback function registered for asynchronous qmi wds commands
    currently used for
    start_nw_if
    stop_nw_if
    This function will post a command to dsi_netctrl_cb thread for
    later processing.

    @return
    none
*/
/*=========================================================================*/
void dsi_qmi_wds_cmd_cb
(
  int                           user_handle,     /* QMI Msg Lib client ID  */
  qmi_service_id_type           service_id,      /* QMI service ID         */
  int                           sys_err_code,    /* QMI Msg Lib error      */
  int                           qmi_err_code,    /* QMI error              */
  void                         *user_data,       /* Callback context       */
  qmi_wds_async_rsp_id_type     rsp_id,          /* QMI Msg Lib txn ID     */
  qdi_wds_async_rsp_data_type  *rsp_data         /* QMI Msg Lib txn data   */
)
{
  int ret = DSI_ERROR;
  dsi_netctrl_cb_cmd_t * cmd_buf = NULL;

  DSI_LOG_VERBOSE( "%s", ">>>dsi_qmi_wds_cmd_cb: ENTRY" );

  do
  {
    DSI_LOG_VERBOSE(">>>qmi wds cmd_cb: user_handle=0x%x", user_handle);
    DSI_LOG_VERBOSE(">>>qmi wds cmd_cb: service_id=%d", (int)service_id);
    DSI_LOG_VERBOSE(">>>qmi wds cmd_cb: sys_err_code=%d", sys_err_code);
    DSI_LOG_VERBOSE(">>>qmi wds cmd_cb: qmi_err_code=%d", qmi_err_code);
    DSI_LOG_VERBOSE(">>>qmi wds cmd_cb: rsp_id=%d", rsp_id);
    DSI_LOG_VERBOSE(">>>qmi wds cmd_cb: rsp_data=%p", rsp_data);

    if (NULL == rsp_data)
    {
      DSI_LOG_FATAL("%s", ">>>*** rcvd NULL rsp_data ***");
      break;
    }

    cmd_buf = (dsi_netctrl_cb_cmd_t *)dsi_malloc(sizeof(dsi_netctrl_cb_cmd_t));
    if (NULL == cmd_buf)
    {
      DSI_LOG_ERROR("%s",">>>malloc failed for dsi_netctrl_cb_cmd_t");
      break;
    }
    
    /* set parameters in our internal structure  */
    cmd_buf->cmd_data.data_union.async_rsp.user_handle = user_handle;
    cmd_buf->cmd_data.data_union.async_rsp.qmi_service_id = service_id;
    cmd_buf->cmd_data.data_union.async_rsp.sys_err_code = sys_err_code;
    cmd_buf->cmd_data.data_union.async_rsp.qmi_err_code = qmi_err_code;
    cmd_buf->cmd_data.data_union.async_rsp.user_data = user_data;
    cmd_buf->cmd_data.data_union.async_rsp.rsp_id = rsp_id;
    /* there are no embedded pointers inside rsp_data structure, so
     * memcpy should be enough to copy everything */
    memcpy(&(cmd_buf->cmd_data.data_union.async_rsp.rsp_data), 
           rsp_data,
           sizeof(cmd_buf->cmd_data.data_union.async_rsp.rsp_data));

    /* set broad category to discriminate data, at the end
       dsc_cmd_q is going to call the execute_f with data */
    cmd_buf->cmd_data.type = DSI_NETCTRL_CB_CMD_QMI_ASYNC_RSP;

    /* prepare ds_cmd_t required by ds_cmdq */
    cmd_buf->cmd.execute_f = dsi_netctrl_cb_cmd_exec;
    cmd_buf->cmd.free_f = dsi_netctrl_cb_cmd_free;
    /* self pointer. this will be freed later */
    cmd_buf->cmd.data = cmd_buf;

    /* post command to global dsi_netctrl_cb queue */
    DSI_LOG_VERBOSE(">>>posting cmd [%p] to dsi_netctrl_cb queue",
                  &cmd_buf->cmd);
    ds_cmdq_enq(&dsi_netctrl_cb_cmdq, &cmd_buf->cmd);
    
    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE( "%s", ">>>dsi_qmi_wds_cmd_cb: EXIT with suc" );
  }
  else
  {
    DSI_LOG_VERBOSE( "%s", ">>>dsi_qmi_wds_cmd_cb: EXIT with err" );
  }

}

/*===========================================================================
  FUNCTION:  dsi_netctrl_copy_tmgi_list
===========================================================================*/
/*!
    @brief
    HELPER FUNCTION copy tmgi_list from tmgi_list_ptr_from to tmgi_list_ptr_to.

    @return
    DSI_SUCCESS memory allocated, assigned the value to dest
    DSI_ERROR   memory not allocated
*/
/*=========================================================================*/
int dsi_netctrl_copy_tmgi_list
(
  qmi_wds_embms_tmgi_type **tmgi_list_ptr_to,
  qmi_wds_embms_tmgi_type *tmgi_list_ptr_from,
  unsigned char            len
)
{
  int count = 0;
  int ret   = DSI_ERROR;

  DSI_LOG_VERBOSE("%s", ">>>dsi_netctrl_copy_tmgi_list: ENTRY");

  do
  {
    /* validate input */
    if(0 == len)
    {
      DSI_LOG_VERBOSE("%s", "rcvd zero len, nothing to copy");
      ret = DSI_SUCCESS;
      break;
    }

    if(NULL == tmgi_list_ptr_from ||
       NULL == tmgi_list_ptr_to)
    {
      DSI_LOG_ERROR("%s", "rcvd invalid input data");
      break;
    }

    /* allocate memory */
    *tmgi_list_ptr_to = (qmi_wds_embms_tmgi_type *)
                       malloc(len * sizeof(qmi_wds_embms_tmgi_type));
    if(NULL != (*tmgi_list_ptr_to))
    {
      /* zero out memory */
      memset(*tmgi_list_ptr_to,
             0,
             len * sizeof(qmi_wds_embms_tmgi_type));
      for (count = 0; count < len; count++)
      {
        memcpy((*tmgi_list_ptr_to)[count].tmgi,
               tmgi_list_ptr_from[count].tmgi,
               QMI_WDS_EMBMS_TMGI_SIZE);
        (*tmgi_list_ptr_to)[count].session_id =
                         tmgi_list_ptr_from[count].session_id;

        (*tmgi_list_ptr_to)[count].session_id_valid =
                         tmgi_list_ptr_from[count].session_id_valid;

      }
    }
    else
    {
      DSI_LOG_ERROR("%s", "can not allocate memory!");
      break;
    }
    ret = DSI_SUCCESS;
  }while(0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", ">>>dsi_netctrl_copy_tmgi_list: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", ">>>dsi_netctrl_copy_tmgi_list: EXIT with err");
  }
  return ret;
}
/*===========================================================================
  FUNCTION:  dsi_qmi_wds_ind_cb
===========================================================================*/
/*!
    @brief
    callback function registered for wds indications.
    This function will post a command to separate dsi_netctrl_cb thread
    for later processing.

    @return
    none
*/
/*=========================================================================*/
void dsi_qmi_wds_ind_cb
(
  int wds_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_wds_indication_id_type ind_id,
  qmi_wds_indication_data_type * ind_data
)
{
  int ret = DSI_ERROR;
  dsi_netctrl_cb_cmd_t * cmd_buf = NULL;
  qmi_wds_embms_tmgi_type   *tmgi_list_ptr = NULL;
  qmi_wds_embms_tmgi_type   *deact_tmgi_list_ptr = NULL;

  int count;
  int reti = DSI_SUCCESS;

  DSI_LOG_VERBOSE("%s", ">>>dsi_qmi_wds_ind_cb: ENTRY");

  do
  {
    if (NULL == ind_data)
    {
      DSI_LOG_FATAL("%s", "*** rcvd invalid ind_data ***");
      break;
    }

    cmd_buf = (dsi_netctrl_cb_cmd_t *)dsi_malloc(sizeof(dsi_netctrl_cb_cmd_t));
    if (NULL == cmd_buf)
    {
      DSI_LOG_FATAL("%s","*** malloc failed for dsi_netctrl_cb cmd ***");
      break;
    }

    DSI_LOG_VERBOSE(">>>received wds_ind with wds_hndl [0x%x] "
                    "sid [%d] user_data [0x%p] ind_id [%d] ind_data [%p]",
                    wds_hndl, sid, user_data, ind_id, ind_data);
    /* set parameters in our internal structure  */
    cmd_buf->cmd_data.data_union.wds_ind.wds_hndl = wds_hndl;
    cmd_buf->cmd_data.data_union.wds_ind.qmi_service_id = sid;
    cmd_buf->cmd_data.data_union.wds_ind.user_data = user_data;
    cmd_buf->cmd_data.data_union.wds_ind.ind_id = ind_id;

    switch(ind_id)
    {
      /* for activate_ind/deactivate_ind/act_deact_ind/ list_ind, embedded structure
         tmgi_list_ptr needs to be copied over */

      case QMI_WDS_SRVC_EMBMS_TMGI_ACTIVATE_IND_MSG:
        {
          DSI_LOG_VERBOSE("%s", "process QMI_WDS_SRVC_EMBMS_TMGI_ACTIVATE_IND_MSG");

          memcpy(&(cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_activate_status),
                 &(ind_data->embms_activate_status),
                 sizeof(ind_data->embms_activate_status));
 
          if(dsi_netctrl_copy_tmgi_list(&tmgi_list_ptr,
                                        ind_data->embms_activate_status.activation_tmgi.tmgi_list_ptr,
                                        ind_data->embms_activate_status.activation_tmgi.tmgi_list_len ) < 0)
          {
            DSI_LOG_ERROR("%s", "can not process tmgi list");
            reti = DSI_ERROR;
            break;
          }
          cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_activate_status.activation_tmgi.tmgi_list_ptr =
            tmgi_list_ptr;
        }
        break;

      case QMI_WDS_SRVC_EMBMS_TMGI_DEACTIVATE_IND_MSG:
        {
          DSI_LOG_VERBOSE("%s", "process QMI_WDS_SRVC_EMBMS_TMGI_DEACTIVATE_IND_MSG");

          memcpy(&(cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_deactivate_status),
                 &(ind_data->embms_deactivate_status),
                 sizeof(ind_data->embms_deactivate_status));

          if(dsi_netctrl_copy_tmgi_list(&tmgi_list_ptr,
                                        ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr,
                                        ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_len ) < 0)
          {
            DSI_LOG_ERROR("%s", "can not process tmgi list");
            reti = DSI_ERROR;
            break;
          }
          cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr =
            tmgi_list_ptr;
        }
        break;

      case QMI_WDS_SRVC_EMBMS_TMGI_ACT_DEACT_IND_MSG:
        {
          DSI_LOG_VERBOSE("%s", "process QMI_WDS_SRVC_EMBMS_TMGI_ACT_DEACT_IND_MSG");

          memcpy(&(cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_act_deact_status),
                 &(ind_data->embms_act_deact_status),
                 sizeof(ind_data->embms_act_deact_status));

          /* copy activate tmgi list */
          if(dsi_netctrl_copy_tmgi_list(&tmgi_list_ptr,
                                        ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_ptr,
                                        ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_len ) < 0)
          {
            DSI_LOG_ERROR("%s", "can not process tmgi list");
            reti = DSI_ERROR;
            break;
          }
          cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_act_deact_status.activation_tmgi.tmgi_list_ptr =
            tmgi_list_ptr;

          /* copy deactivate tmgi list */
          if(dsi_netctrl_copy_tmgi_list(&deact_tmgi_list_ptr,
                                        ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr,
                                        ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_len ) < 0)
          {
            DSI_LOG_ERROR("%s", " can not process tmgi list");
            reti = DSI_ERROR;
            break;
          }
          cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr =
            deact_tmgi_list_ptr;
        }
        break;

      case QMI_WDS_SRVC_EMBMS_TMGI_LIST_IND_MSG:
        {
          DSI_LOG_VERBOSE("%s", "process QMI_WDS_SRVC_EMBMS_TMGI_LIST_IND_MSG");

          memcpy(&(cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_list),
                 &(ind_data->embms_list),
                 sizeof(ind_data->embms_list));

          if(ind_data->embms_list.param_mask & QMI_WDS_EMBMS_LIST_IND_TMGI_LIST_PARAM_MASK )
          {
            if(0 != ind_data->embms_list.tmgi_list.tmgi_list_len)
            {
              if(dsi_netctrl_copy_tmgi_list(&tmgi_list_ptr,
                                            ind_data->embms_list.tmgi_list.tmgi_list_ptr,
                                            ind_data->embms_list.tmgi_list.tmgi_list_len ) < 0)
              {
                DSI_LOG_ERROR("%s", "can not process tmgi list");
                reti = DSI_ERROR;
                break;
              }
              cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_list.tmgi_list.tmgi_list_ptr =
                tmgi_list_ptr;
            }
          }
        }
        break;

      case QMI_WDS_SRVC_EMBMS_CONTENT_DESC_CONTROL_IND_MSG:
        {
          DSI_LOG_VERBOSE("%s", "process QMI_WDS_SRVC_EMBMS_CONTENT_DESC_CONTROL_IND_MSG");

          memcpy(&(cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_content_desc_control),
                 &(ind_data->embms_content_desc_control),
                 sizeof(ind_data->embms_content_desc_control));

          if(0 != ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_len)
          {
            if(dsi_netctrl_copy_tmgi_list(
                  &tmgi_list_ptr,
                  ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr,
                  ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_len ) < 0)
            {
              DSI_LOG_ERROR("%s", "can not process tmgi list");
              reti = DSI_ERROR;
              break;
            }
            cmd_buf->cmd_data.data_union.wds_ind.ind_data.embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr =
              tmgi_list_ptr;
            DSI_LOG_VERBOSE("%s %d:%X,%X,%X,%X,%X,%X",
              "process QMI_WDS_SRVC_EMBMS_CONTENT_DESC_CONTROL_IND_MSG - TMGI",
              ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_len,
              ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr->tmgi[0],
              ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr->tmgi[1],
              ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr->tmgi[2],
              ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr->tmgi[3],
              ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr->tmgi[4],
              ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr->tmgi[5]);
          }
        }
        break;

      case QMI_WDS_SRVC_EMBMS_SAI_LIST_IND_MSG:
        {
          DSI_LOG_VERBOSE("%s", "process QMI_WDS_SRVC_EMBMS_SAI_LIST_IND_MSG");
          memcpy(&(cmd_buf->cmd_data.data_union.wds_ind.ind_data.sai_list),
                 &(ind_data->sai_list),
                 sizeof(ind_data->sai_list));

          unsigned short    temp_size = 0;
          unsigned int *temp_list_ptr = NULL;

          if (ind_data->sai_list.param_mask & QMI_WDS_EMBMS_LIST_IND_SAI_LIST_PARAM_MASK)
          {
            temp_size = ind_data->sai_list.available_sai_list_len;
            if (0 != temp_size)
            {
              if(NULL == (temp_list_ptr = (unsigned int *)malloc(temp_size * sizeof(unsigned int))))
              {
                 reti = DSI_ERROR;
                 break;
              }
              memcpy(temp_list_ptr,
                     ind_data->sai_list.available_sai_list,
                     temp_size * sizeof(unsigned int) );
            }
            cmd_buf->cmd_data.data_union.wds_ind.ind_data.sai_list.available_sai_list = temp_list_ptr;
            cmd_buf->cmd_data.data_union.wds_ind.ind_data.sai_list.available_sai_list_len = temp_size;

            temp_list_ptr = NULL;
            temp_size = ind_data->sai_list.camped_sai_list_len;
            if (0 != temp_size)
            {
              if( NULL == (temp_list_ptr = (unsigned int *)malloc(temp_size * sizeof(unsigned int))))
              {
                 reti = DSI_ERROR;
                 break;
              }
              memcpy(temp_list_ptr,
                     ind_data->sai_list.camped_sai_list,
                     temp_size * sizeof(unsigned int));
            }
            cmd_buf->cmd_data.data_union.wds_ind.ind_data.sai_list.camped_sai_list = temp_list_ptr;
            cmd_buf->cmd_data.data_union.wds_ind.ind_data.sai_list.camped_sai_list_len = temp_size;

            temp_list_ptr = NULL;
            temp_size = ind_data->sai_list.num_sai_per_group_len;
            if ( 0 != temp_size)
            {
              if( NULL == (temp_list_ptr = (unsigned int *)malloc(temp_size * sizeof(unsigned short))))
              {
                 reti = DSI_ERROR;
                 break;
              }
              memcpy(temp_list_ptr,
                     ind_data->sai_list.num_sai_per_group,
                     temp_size * sizeof(unsigned short));
            }
            cmd_buf->cmd_data.data_union.wds_ind.ind_data.sai_list.num_sai_per_group = (unsigned short*)temp_list_ptr;
            cmd_buf->cmd_data.data_union.wds_ind.ind_data.sai_list.num_sai_per_group_len = temp_size;
          }
        }
        break;

      default:
        {
          /* there are no embedded pointers inside ind_data structure, so
           * memcpy should be enough to copy everything */
          memcpy(&(cmd_buf->cmd_data.data_union.wds_ind.ind_data),
                 ind_data,
                 sizeof(cmd_buf->cmd_data.data_union.wds_ind.ind_data));
        }
        break;
    }

    /* set broad category to discriminate data, at the end
       dsc_cmd_q is going to call the execute_f with data */
    cmd_buf->cmd_data.type = DSI_NETCTRL_CB_CMD_QMI_WDS_IND;

    /* prepare ds_cmd_t required by ds_cmdq */
    cmd_buf->cmd.execute_f = dsi_netctrl_cb_cmd_exec;
    cmd_buf->cmd.free_f = dsi_netctrl_cb_cmd_free;
    /* self pointer. this will be freed later */
    cmd_buf->cmd.data = cmd_buf;

    /* post command to global dsi_netctrl_cb queue */
    DSI_LOG_VERBOSE(">>>posting cmd [%p] to dsi_netctrl_cb queue",
                  &cmd_buf->cmd);
    ds_cmdq_enq(&dsi_netctrl_cb_cmdq, &cmd_buf->cmd);

    if(DSI_SUCCESS == reti)
    {
      ret = DSI_SUCCESS;
    }
    else
    {
      break;
    }
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", ">>>dsi_qmi_wds_ind_cb: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", ">>>dsi_qmi_wds_ind_cb: EXIT with err");
  }

}

/*===========================================================================
  FUNCTION:  dsi_qmi_sys_cb
===========================================================================*/
/*!
    @brief
    This function is registered as system callback with QMI Msg Lib.

    @return
    none
*/
/*=========================================================================*/
void
dsi_qmi_sys_cb
(
  qmi_sys_event_type event_id,
  const qmi_sys_event_info_type * event_info,
  void * user_data
)
{
  int i = 0;
  int iface_start = 0;
  int iface_end = 0;
  int modem = 0;
  int valid_ind = DSI_FALSE;
  dsi_netctrl_cb_cmd_t * cmd_buf = NULL;
  qmi_sys_event_type sys_ev;
  boolean event_of_concern = DSI_TRUE;

  DSI_LOG_VERBOSE("%s", ">>>dsi_qmi_sys_cb: ENTRY");

  switch(event_id)
  {
  case QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND:
    break;
  case QMI_SYS_EVENT_MODEM_IN_SERVICE_IND:
    break;
  default:
    DSI_LOG_VERBOSE("ignore qmi sys ind [%d]", event_id);
    event_of_concern = DSI_FALSE;
    break;
  }

  if(DSI_TRUE != event_of_concern)
  {
    return;
  }

  if (NULL == event_info)
  {
    DSI_LOG_FATAL("%s", ">>>dsi_qmi_sys_cb: Invalid parameter received!");
    return;
  }

  /* We need to check if the qmi sys ind is coming in from the
   * correct conn_id */
  if (DSI_TRUE == dsi_config.single_qmux_ch_enabled)
  {
    /* For single qmux control channel targets, we need to compare
     * with only one channel name */
    if (strcmp(dsi_config.single_qmux_ch_name,
               event_info->qmi_modem_service_ind.dev_id))
    {
      DSI_LOG_VERBOSE("ignore qmi sys ind [%d] on device [%s]",
                      event_id, dsi_config.single_qmux_ch_name);
      return;
    }
  }
  else
  {
    /* For other targets we need to compare the device id for sys ind
     * with all active qmi connection ids */
    for (modem=0; modem<DSI_MAX_MODEMS; modem++)
    {
      /* Loop through all the modems to find the valid iface
       * and matching device id */
      iface_start = DSI_MODEM_GET_IFACE_START(modem);
      iface_end   = DSI_MODEM_GET_IFACE_END(modem);

      for (i=iface_start; i<=iface_end; i++)
      {
        if (!strcmp(dsi_qmi_port_names[i],
                       event_info->qmi_modem_service_ind.dev_id))
        {
          /* valid_ind flag will be set if we found a matching device id
           * to indicate that the qmi indication is valid and needs to be
           * processed */
          valid_ind = DSI_TRUE;
          break;
        }
      }

      if (DSI_TRUE == valid_ind)
      {
        break;
      }
    }

    if (DSI_FALSE == valid_ind)
    {
      /* We have checked all ifaces with the incoming dev id and could
       * not find a match, ignore sys indication */
      DSI_LOG_VERBOSE("ignore qmi sys ind [%d] on device [%s]",
                      event_id, dsi_config.single_qmux_ch_name);
      return;
    }
  }

  cmd_buf = (dsi_netctrl_cb_cmd_t *)
    dsi_malloc(sizeof(dsi_netctrl_cb_cmd_t));
  if (NULL == cmd_buf)
  {
    DSI_LOG_FATAL("%s","*** malloc failed for dsi_netctrl_cb cmd ***");
    return;
  }

  /* set parameters in our internal structure  */
  cmd_buf->cmd_data.data_union.qmi_sys_ind.event_id = event_id;
  cmd_buf->cmd_data.data_union.qmi_sys_ind.user_data = user_data;
  /* there are no embedded pointers inside event_info structure, so
   * memcpy should be enough to copy everything */
  memcpy(&(cmd_buf->cmd_data.data_union.qmi_sys_ind.event_info),
         event_info,
         sizeof(cmd_buf->cmd_data.data_union.qmi_sys_ind.event_info));

  /* set broad category to discriminate data, at the end
     dsc_cmd_q is going to call the execute_f with data */
  cmd_buf->cmd_data.type = DSI_NETCTRL_CB_CMD_QMI_SYS_IND;

  /* prepare ds_cmd_t required by ds_cmdq */
  cmd_buf->cmd.execute_f = dsi_netctrl_cb_cmd_exec;
  cmd_buf->cmd.free_f = dsi_netctrl_cb_cmd_free;
  /* self pointer. this will be freed later */
  cmd_buf->cmd.data = cmd_buf;

  /* post command to global dsi_netctrl_cb queue */
  DSI_LOG_VERBOSE(">>>posting cmd [%p] to dsi_netctrl_cb queue",
                &cmd_buf->cmd);
  ds_cmdq_enq(&dsi_netctrl_cb_cmdq, &cmd_buf->cmd);

  DSI_LOG_VERBOSE("%s", ">>>dsi_qmi_sys_cb: EXIT");
}


/*===========================================================================
  FUNCTION:  dsi_qmi_qos_ind_cb
===========================================================================*/
/*!
    @brief
    callback function registered for qos indications.
    This function will post a command to separate dsi_netctrl_cb thread
    for later processing.

    @return
    none
*/
/*=========================================================================*/
void dsi_qmi_qos_ind_cb
(
  int qos_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_qos_indication_id_type ind_id,
  qmi_qos_indication_data_type * ind_data
)
{
  int ret = DSI_ERROR;
  dsi_netctrl_cb_cmd_t * cmd_buf = NULL;

  DSI_LOG_VERBOSE("%s", ">>>dsi_qmi_qos_ind_cb: ENTRY");

  do
  {
    if (NULL == ind_data)
    {
      DSI_LOG_FATAL("%s", "*** rcvd invalid ind_data ***");
      break;
    }

    cmd_buf = (dsi_netctrl_cb_cmd_t *)dsi_malloc(sizeof(dsi_netctrl_cb_cmd_t));
    if (NULL == cmd_buf)
    {
      DSI_LOG_FATAL("%s","*** malloc failed for dsi_netctrl_cb cmd ***");
      break;
    }

    DSI_LOG_VERBOSE(">>>received qos_ind with qos_hndl [0x%x] "
                    "sid [%d] user_data [0x%p] ind_id [%d] ind_data [%p]",
                    qos_hndl, sid, user_data, ind_id, ind_data);
    /* set parameters in our internal structure  */
    cmd_buf->cmd_data.data_union.qos_ind.qos_hndl = qos_hndl;
    cmd_buf->cmd_data.data_union.qos_ind.qmi_service_id = sid;
    cmd_buf->cmd_data.data_union.qos_ind.user_data = user_data;
    cmd_buf->cmd_data.data_union.qos_ind.ind_id = ind_id;
    /* there are no embedded pointers inside ind_data structure, so
     * memcpy should be enough to copy everything */
    memcpy(&(cmd_buf->cmd_data.data_union.qos_ind.ind_data),
           ind_data,
           sizeof(cmd_buf->cmd_data.data_union.qos_ind.ind_data));

    /* set broad category to discriminate data, at the end
       dsc_cmd_q is going to call the execute_f with data */
    cmd_buf->cmd_data.type = DSI_NETCTRL_CB_CMD_QMI_QOS_IND;

    /* prepare ds_cmd_t required by ds_cmdq */
    cmd_buf->cmd.execute_f = dsi_netctrl_cb_cmd_exec;
    cmd_buf->cmd.free_f = dsi_netctrl_cb_cmd_free;
    /* self pointer. this will be freed later */
    cmd_buf->cmd.data = cmd_buf;

    /* post command to global dsi_netctrl_cb queue */
    DSI_LOG_VERBOSE(">>>posting cmd [%p] to dsi_netctrl_cb queue",
                  &cmd_buf->cmd);
    ds_cmdq_enq(&dsi_netctrl_cb_cmdq, &cmd_buf->cmd);

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", ">>>dsi_qmi_qos_ind_cb: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", ">>>dsi_qmi_qos_ind_cb: EXIT with err");
  }
}

/*===========================================================================
  FUNCTION:  dsi_netmgr_cb
===========================================================================*/
/*!
    @brief
    callback function registered with netmgr

    @return
    void
*/
/*=========================================================================*/
void dsi_netmgr_cb
(
  netmgr_nl_events_t event,
  netmgr_nl_event_info_t * info,
  void * data
)
{
  int ret = DSI_ERROR;
  dsi_netctrl_cb_cmd_t * cmd_buf = NULL;

  DSI_LOG_VERBOSE("%s", ">>>dsi_netmgr_cb ENTRY");

  do
  {
    ret = DSI_ERROR;

    if (NULL == info)
    {
      DSI_LOG_FATAL("%s","*** NULL info rcvd ***");
      break;
    }

    cmd_buf = (dsi_netctrl_cb_cmd_t *)dsi_malloc(sizeof(dsi_netctrl_cb_cmd_t));
    if (NULL == cmd_buf)
    {
      DSI_LOG_FATAL("%s","*** malloc failed for dsi_netctrl_cb cmd ***");
      break;
    }

    DSI_LOG_VERBOSE(">>>received netmgr_cb with event [%d] "
                  "info [%p] data [%p]", (int)event, info, data);
    DSI_LOG_VERBOSE(">>>found info->link = [%d]", info->link);
                  
    /* set parameters in our internal structure  */
    cmd_buf->cmd_data.data_union.netmgr.event = event;
    /* there are no embedded pointers inside info structure, so
     * memcpy should be enough to copy everything */
    memcpy(&(cmd_buf->cmd_data.data_union.netmgr.info),
           info, 
           sizeof(cmd_buf->cmd_data.data_union.netmgr.info));
    cmd_buf->cmd_data.data_union.netmgr.data = data;

    /* set broad category to discriminate data, at the end
       dsc_cmd_q is going to call the execute_f with data */
    cmd_buf->cmd_data.type = DSI_NETCTRL_CB_CMD_NETMGR;

    /* prepare ds_cmd_t required by ds_cmdq */
    cmd_buf->cmd.execute_f = dsi_netctrl_cb_cmd_exec;
    cmd_buf->cmd.free_f = dsi_netctrl_cb_cmd_free;
    /* self pointer. this will be freed later */
    cmd_buf->cmd.data = cmd_buf;

    /* post command to global dsi_netctrl_cb queue */
    DSI_LOG_VERBOSE(">>>posting cmd [%p] to dsi_netctrl_cb queue",
                  &cmd_buf->cmd);
    ds_cmdq_enq(&dsi_netctrl_cb_cmdq, &cmd_buf->cmd);

    ret = DSI_SUCCESS;
  } while(0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_VERBOSE("%s", ">>>dsi_netmgr_cb EXIT with err");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", ">>>dsi_netmgr_cb EXIT with suc");
  }

}

/*===========================================================================
  FUNCTION:  dsi_netctrl_cb_deinit
===========================================================================*/
/*!
    @brief
    This function must be called at init time

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
void dsi_netctrl_cb_deinit()
{
  DSI_LOG_DEBUG("%s", "dsi_netctrl_cb_deinit ENTRY");
  if (0 != ds_cmdq_deinit(&dsi_netctrl_cb_cmdq))
  {
    DSI_LOG_ERROR("%s","*** could not deinit dsi_netctrl_cb_cmdq ***");
  }
  DSI_LOG_DEBUG("%s", "dsi_netctrl_cb_deinit EXIT");
}

/*===========================================================================
  FUNCTION:  dsi_netctrl_cb_init
===========================================================================*/
/*!
    @brief
    This function must be called at init time

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_netctrl_cb_init()
{
  int rc;
  int ret = DSI_SUCCESS;

  DSI_LOG_DEBUG("%s", "dsi_netctrl_cb_init ENTRY");
  /* init ds_cmdq queue */
  rc = ds_cmdq_init(&dsi_netctrl_cb_cmdq, DSI_NETCTRL_CB_MAX_CMDS);
  if (0 != rc)
  {
    DSI_LOG_FATAL("*** ds_cmdq_init failed with err [%d] ***", rc);
    ret = DSI_ERROR;
  }

  DSI_LOG_DEBUG("%s", "dsi_netctrl_cb_init EXIT");
  return ret;
}
