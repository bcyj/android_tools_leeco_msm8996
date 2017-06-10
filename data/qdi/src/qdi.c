/*!
  @file
  qdi.c

  @brief
  Provides a wrapper that manages multiple QMI WDS service handles for
  dual IP over a single RmNet port.

*/

/*===========================================================================

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.

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

when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/01/10   jf      created file

===========================================================================*/
/*===========================================================================
                           INCLUDE FILES
===========================================================================*/
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "ds_util.h"
#include "ds_list.h"
#include "ds_cmdq.h"
#include "qdi.h"
#include "qdi_debug.h"
#include "qdi_netlink.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define QDI_MAXIMUM_HANDLES (16)
#define QDI_CMDQ_MAX_CMDS   (25)

#define QDI_IS_VALID_HNDL(h) ((NULL != (h)) && (TRUE == (h)->valid))

/* Index starts at zero.  To maintain compatability with QMI WDS API,
   a handle of zero is never returned as this indicates that abort
   was successful. */
#define QDI_INDEX_TO_HANDLE(i) (i+1)

/*defines to gateway address conversion*/
#define QDI_IPV6_LINK_LOCAL_PREFIX_BYTE0  0xfe
#define QDI_IPV6_LINK_LOCAL_PREFIX_BYTE1  0x80
#define QDI_IPV6_ZERO_FILLER_START_POS    2
#define QDI_IPV6_ZERO_FILLER_END_POS      8
#define QDI_IPV6_SLAAC_PREFIX_LENGTH      64

#define QDI_GET_INFO_PTR(handle, info)             \
  do                                               \
  {                                                \
    int index = (handle) - 1;                      \
    info = NULL;                                   \
    if (index >= 0 && index < QDI_MAXIMUM_HANDLES) \
      info = &qdi_handle_info[index];              \
    else                                           \
    {                                              \
      QDI_LOG_FATAL("Unable to get info ptr "      \
                    "handle=%x, index=%d",         \
                    handle, index);                \
    }                                              \
  } while (0)

#define QDI_GET_TXN_ID(handle, op)   (((handle) << 16) | (op))
#define QDI_GET_OP_FROM_TXN(txn)     ((txn) & 0x0000FFFF)
#define QDI_GET_HANDLE_FROM_TXN(txn) ((int) (((unsigned int) (txn) & 0xFFFF0000) >> 16))


#define QDI_INVALID_TXN_HNDL      (0)
#define QDI_INVALID_QMI_INST      (-1)

#define QDI_MUTEX_LOCK(mutex)                                      \
  do                                                               \
  {                                                                \
    if (0 == pthread_mutex_lock(&mutex))                           \
    {                                                              \
      QDI_LOG_LOW(">>>>>> LOCK QDI MUTEX %p SUCCESS", &mutex);     \
    }                                                              \
    else                                                           \
    {                                                              \
      QDI_LOG_ERROR(">>>>>> LOCK QDI MUTEX %p FAILURE", &mutex);   \
    }                                                              \
  }                                                                \
  while (0)

#define QDI_MUTEX_UNLOCK(mutex)                                    \
  do                                                               \
  {                                                                \
    if (0 == pthread_mutex_unlock(&mutex))                         \
    {                                                              \
      QDI_LOG_LOW("<<<<<< UNLOCK QDI MUTEX %p SUCCESS", &mutex);   \
    }                                                              \
    else                                                           \
    {                                                              \
      QDI_LOG_ERROR("<<<<<< UNLOCK QDI MUTEX %p FAILURE", &mutex); \
    }                                                              \
  }                                                                \
  while (0)

typedef struct
{
  int      txn_v4;
  boolean  v4_cb_pend;
  int      txn_v6;
  boolean  v6_cb_pend;

  qdi_wds_user_async_cb_type cb_hdlr;
  qdi_wds_async_rsp_data_type cb_rsp_data;
  void* user_data;
} qdi_txn_info_t;

/* Type of an async QDI transaction */
typedef enum
{
  QDI_TXN_MIN = 0,
  QDI_TXN_START_IF = QDI_TXN_MIN,
  QDI_TXN_STOP_IF,
  QDI_TXN_RESET,
  QDI_TXN_MAX
} qdi_txn_t;

/* Private data associated with a QDI handle */
typedef struct
{
  int valid;

  const char *wds_id;
  const char *dev_id;

  qmi_client_handle_type handle_ipv4;
  qmi_client_handle_type handle_ipv6;

  void* ind_user_data;
  qmi_wds_indication_hdlr_type ind_msg_hdlr;

  ds_dll_el_t  *call_info_head;
} qdi_handle_info_t;

/* State associated with an active call */
typedef struct
{
  qdi_client_handle_t handle;

  qdi_txn_info_t start_if_txn;
  qdi_txn_info_t stop_if_txn;
  qdi_txn_info_t abort_txn;

  qdi_mode_t mode;

  int v4_pkt_srvc_conn_ind_cnt;
  int v6_pkt_srvc_conn_ind_cnt;

  int rl_qmi_inst;
} qdi_call_info_t;

static qdi_handle_info_t qdi_handle_info[QDI_MAXIMUM_HANDLES];

/* Mutex for synchronizing TXN access between main caller and the
   QMI registered callback */
static pthread_mutex_t qdi_txn_mutex = PTHREAD_MUTEX_INITIALIZER;


/* Storage for QMI WDS indication messages */
typedef struct
{
  int                           user_handle;
  qmi_service_id_type           service_id;
  void                          *user_data;
  qmi_wds_indication_id_type    ind_id;
  qmi_wds_indication_data_type  ind_data;
} qdi_wds_ind_data_t;

/* Storage for QMI WDS async response messages */
typedef struct
{
  int                          user_handle;
  qmi_service_id_type          service_id;
  int                          sys_err_code;
  int                          qmi_err_code;
  void                         *user_data;
  qmi_wds_async_rsp_id_type    rsp_id;
  qmi_wds_async_rsp_data_type  rsp_data;
} qdi_wds_async_rsp_t;

/* Type of data being stored in command buffer */
typedef enum
{
  QDI_CMDQ_CMD_TYPE_WDS_IND,
  QDI_CMDQ_CMD_TYPE_WDS_ASYNC_RESP
} qdi_cmdq_cmd_t;

/* Typedef for command data */
typedef struct
{
  qdi_cmdq_cmd_t            cmd;

  union
  {
    qdi_wds_ind_data_t      wds_ind;
    qdi_wds_async_rsp_t     async_resp;
  } data;
} qdi_cmdq_cmd_data_type;

/* Typedef for qdi command type */
typedef struct
{
  ds_cmd_t                ds_cmd;   /* Should be the first entry in the structure */
  qdi_cmdq_cmd_data_type  cmd_data;
} qdi_cmdq_cmd_type;

/* QDI Command Queue */
static ds_cmdq_info_t  qdi_cmdq;

/* Forward function declarations */
static void
qdi_indication_hdlr
(
  int                           user_handle,
  qmi_service_id_type           service_id,
  void                          *user_data,
  qmi_wds_indication_id_type    ind_id,
  qmi_wds_indication_data_type  *ind_data
);

static void
qdi_cb_hdlr
(
  int                          user_handle,
  qmi_service_id_type          service_id,
  int                          sys_err_code,
  int                          qmi_err_code,
  void                         *user_data,
  qmi_wds_async_rsp_id_type    rsp_id,
  qmi_wds_async_rsp_data_type  *rsp_data
);

typedef union
{
  void* data;
  uint32* data_u32;
  uint64* data_u64;
} addr_data_t;

static int
qdi_query_addr_and_incr_ref_count
(
  qdi_call_info_t     *call_info,
  int                 ip_family
);

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qdi_match_call_info_node
===========================================================================*/
/*!
@brief
  Comparator function for matching call_info nodes

@return
  0 - on a match
  1 - otherwise
*/
/*=========================================================================*/
static long int
qdi_match_call_info_node
(
  const void *first,
  const void *second
)
{
  long int ret = 1;

  if (first == second)
  {
    ret = 0;
  }

  return ret;
}


/*===========================================================================
  FUNCTION:  qdi_copy_tmgi_list
===========================================================================*/
/*!
    @brief
    HELPER FUNCTION copy tmgi_list from tmgi_list_ptr_from to tmgi_list_ptr_to.

    @return
    QDI_SUCCESS memory allocated, assigned the value to dest
    QDI_FAILURE   memory not allocated
*/
/*=========================================================================*/
int qdi_copy_tmgi_list
(
  qmi_wds_embms_tmgi_type **tmgi_list_ptr_to,
  qmi_wds_embms_tmgi_type *tmgi_list_ptr_from,
  unsigned char            len
)
{
  int count = 0;
  int ret   = QDI_FAILURE;

  QDI_LOG_ERROR("%s", ">>>qdi_copy_tmgi_list: ENTRY");

  do
  {
    /* validate input */
    if(NULL == tmgi_list_ptr_from ||
       NULL == tmgi_list_ptr_to)
    {
      QDI_LOG_ERROR("%s", "rcvd invalid input data");
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
      QDI_LOG_ERROR("%s", "can not allocate memory!");
      break;
    }
    ret = QDI_SUCCESS;
  }while(0);

  if (QDI_SUCCESS == ret)
  {
    QDI_LOG_ERROR("%s", ">>>qdi_copy_tmgi_list: EXIT with suc");
  }
  else
  {
    QDI_LOG_ERROR("%s", ">>>qdi_copy_tmgi_list: EXIT with err");
  }
  return ret;
}

/*===========================================================================
  FUNCTION  qdi_find_matching_call_info_node
===========================================================================*/
/*!
@brief
  Function to find a matching call_info node

@return
  valid call_info node - if a match is found
  NULL                 - otherwise

@note
  qdi_txn_mutex must be locked before calling this function
*/
/*=========================================================================*/
static qdi_call_info_t *
qdi_find_matching_call_info_node
(
  qdi_handle_info_t  *info,
  qdi_call_info_t    *call_info
)
{
  qdi_call_info_t  *ret = NULL;
  ds_dll_el_t      *next = NULL;

  QDI_LOG_TRACE_ENTRY;

  if (!info || !call_info)
  {
    QDI_LOG_ERROR("qdi_find_matching_call_info_node: invalid input param(s)");
  }
  else
  {
    next = ds_dll_search(info->call_info_head,
                         (const void *)call_info,
                         qdi_match_call_info_node);

    if (next)
    {
      QDI_LOG_LOW("qdi_find_matching_call_info_node: found call_info=%p in info=%p, handle=%d",
                  call_info,
                  info,
                  call_info->handle);

      ret = (qdi_call_info_t *)ds_dll_data(next);
    }
  }

  QDI_LOG_TRACE_EXIT;
  return ret;
}


/*===========================================================================
  FUNCTION  qdi_cleanup_call_info_node
===========================================================================*/
/*!
@brief
  Function to remove the given call_info object from the list and then free it

@return
  None

@note
  qdi_txn_mutex must be locked before calling this function
*/
/*=========================================================================*/
static void
qdi_cleanup_call_info_node
(
  qdi_handle_info_t  *info,
  qdi_call_info_t    *call_info
)
{
  ds_dll_el_t  *node = NULL;
  ds_dll_el_t  *tail = NULL;
  const void   *dummy = NULL;

  QDI_LOG_TRACE_ENTRY;

  if (!info || !call_info)
  {
    QDI_LOG_ERROR("qdi_cleanup_call_info_node: invalid input param(s)");
  }
  else
  {
    /* find the tail of the call_info list */
    node = ds_dll_next(info->call_info_head, &dummy);
    while (node != NULL)
    {
      tail = node;
      node = ds_dll_next(node, &dummy);
    }

    node = ds_dll_delete(info->call_info_head,
                         &tail,
                         (const void *)call_info,
                         qdi_match_call_info_node);

    if (node == NULL)
    {
      QDI_LOG_ERROR("qdi_cleanup_call_info_node: call_info=%p not found in info=%p",
                    call_info,
                    info);
    }
    else
    {
      QDI_LOG_MED("qdi_cleanup_call_info_node: call_info=%p IPv4 cnt=%d, IPv6 cnt=%d",
                  call_info,
                  call_info->v4_pkt_srvc_conn_ind_cnt,
                  call_info->v6_pkt_srvc_conn_ind_cnt);

      /* Free the call_info state */
      ds_free((void *)ds_dll_data(node));
      ds_dll_free(node);
    }
  }

  QDI_LOG_TRACE_EXIT;
}


/*===========================================================================
  FUNCTION  qdi_translate_qmi_ip_family_pref
===========================================================================*/
/*!
@brief
  Function to translate QMI ip family preference to QDI mode

@return
  Corresponding qdi_mode_t
*/
/*=========================================================================*/
static qdi_mode_t
qdi_translate_qmi_ip_family_pref
(
  qmi_wds_ip_family_pref_type  ip_family_pref
)
{
  qdi_mode_t  mode;

  switch (ip_family_pref)
  {
    case QMI_WDS_IP_FAMILY_PREF_IPV4:
    case QMI_WDS_IP_FAMILY_PREF_ANY:
      mode = QDI_MODE_V4;
      break;

    case QMI_WDS_IP_FAMILY_PREF_IPV6:
      mode = QDI_MODE_V6;
      break;

    case QMI_WDS_IP_FAMILY_PREF_UNSPECIFIED:
    default:
      mode = QDI_MODE_V4V6;
      break;
  }

  return mode;
}


/*===========================================================================
  FUNCTION  qdi_process_wds_ind
===========================================================================*/
/*!
@brief
  This is processing function for QMI WDS indications which gets called in the
  context of the cmdq thread

@return
  None
*/
/*=========================================================================*/
static void qdi_process_wds_ind
(
  struct ds_cmd_s *ds_cmd,
  void            *data
)
{
  qdi_cmdq_cmd_type *qdi_cmd = (qdi_cmdq_cmd_type *)ds_cmd;

  QDI_LOG_TRACE_ENTRY;

  (void) data;

  if (!qdi_cmd ||
      QDI_CMDQ_CMD_TYPE_WDS_IND != qdi_cmd->cmd_data.cmd)
  {
    QDI_LOG_ERROR("Invalid input param(s)");
    goto bail;
  }

  qdi_indication_hdlr(qdi_cmd->cmd_data.data.wds_ind.user_handle,
                      qdi_cmd->cmd_data.data.wds_ind.service_id,
                      qdi_cmd->cmd_data.data.wds_ind.user_data,
                      qdi_cmd->cmd_data.data.wds_ind.ind_id,
                      &qdi_cmd->cmd_data.data.wds_ind.ind_data);

bail:
  QDI_LOG_TRACE_EXIT;
}


/*===========================================================================
  FUNCTION  qdi_process_wds_async_cb
===========================================================================*/
/*!
@brief
  This is processing function for QMI WDS async responses which gets called
  in the context of the cmdq thread

@return
  None
*/
/*=========================================================================*/
static void qdi_process_wds_async_cb
(
  struct ds_cmd_s *ds_cmd,
  void            *data
)
{
  qdi_cmdq_cmd_type *qdi_cmd = (qdi_cmdq_cmd_type *)ds_cmd;

  QDI_LOG_TRACE_ENTRY;

  (void) data;

  if (!qdi_cmd ||
      QDI_CMDQ_CMD_TYPE_WDS_ASYNC_RESP != qdi_cmd->cmd_data.cmd)
  {
    QDI_LOG_ERROR("Invalid input param(s)");
    goto bail;
  }

  qdi_cb_hdlr(qdi_cmd->cmd_data.data.async_resp.user_handle,
              qdi_cmd->cmd_data.data.async_resp.service_id,
              qdi_cmd->cmd_data.data.async_resp.sys_err_code,
              qdi_cmd->cmd_data.data.async_resp.qmi_err_code,
              qdi_cmd->cmd_data.data.async_resp.user_data,
              qdi_cmd->cmd_data.data.async_resp.rsp_id,
              &qdi_cmd->cmd_data.data.async_resp.rsp_data);

bail:
  QDI_LOG_TRACE_EXIT;
}


/*===========================================================================
  FUNCTION  qdi_cmdq_free
===========================================================================*/
/*!
@brief
  This function frees the previously allocated command buffers and gets
  called in the context of the cmdq thread

@return
  None
*/
/*=========================================================================*/
static void qdi_cmdq_free
(
  struct ds_cmd_s *ds_cmd,
  void            *data
)
{
  qdi_cmdq_cmd_type *qdi_cmd = (qdi_cmdq_cmd_type *)ds_cmd;

  QDI_LOG_TRACE_ENTRY;

  (void) data;

  if (!qdi_cmd)
  {
    QDI_LOG_ERROR("Invalid input param(s)");
    goto bail;
  }

  ds_free(qdi_cmd);

bail:
  QDI_LOG_TRACE_EXIT;
}


/*===========================================================================
  FUNCTION  qdi_wds_ind_hdlr
===========================================================================*/
/*!
@brief
  This is the indication handler registered with QMI

@return
  None
*/
/*=========================================================================*/
static void
qdi_wds_ind_hdlr
(
  int                           user_handle,
  qmi_service_id_type           service_id,
  void                          *user_data,
  qmi_wds_indication_id_type    ind_id,
  qmi_wds_indication_data_type  *ind_data
)
{
  qdi_cmdq_cmd_type  *cmd = NULL;
  qmi_wds_embms_tmgi_type   *tmgi_list_ptr = NULL;
  qmi_wds_embms_tmgi_type   *deact_tmgi_list_ptr = NULL;

  int reti = QDI_FAILURE;

  QDI_LOG_TRACE_ENTRY;

  if (!ind_data)
  {
    QDI_LOG_ERROR("Invalid input param(s)");
    goto bail;
  }

  cmd = ds_malloc(sizeof(qdi_cmdq_cmd_type));

  if (!cmd)
  {
    QDI_LOG_ERROR("Failed to allocate command");
    goto bail;
  }

  /* Store the indication data in the command buffer */
  cmd->cmd_data.cmd = QDI_CMDQ_CMD_TYPE_WDS_IND;
  cmd->cmd_data.data.wds_ind.user_handle = user_handle;
  cmd->cmd_data.data.wds_ind.service_id  = service_id;
  cmd->cmd_data.data.wds_ind.user_data   = user_data;
  cmd->cmd_data.data.wds_ind.ind_id      = ind_id;
  cmd->cmd_data.data.wds_ind.ind_data    = *ind_data;

  switch(ind_id)
  {
    /* for activate_ind/deactivate_ind/list_ind, embedded structure tmgi_list_ptr
       needs to be copied over */
    case QMI_WDS_SRVC_EMBMS_TMGI_ACTIVATE_IND_MSG:
      {
        QDI_LOG_ERROR("%s", "process QMI_WDS_SRVC_EMBMS_TMGI_ACTIVATE_IND_MSG");

        memcpy(&(cmd->cmd_data.data.wds_ind.ind_data.embms_activate_status),
               &(ind_data->embms_activate_status),
               sizeof(ind_data->embms_activate_status) );

        if(qdi_copy_tmgi_list(&tmgi_list_ptr,
                               ind_data->embms_activate_status.activation_tmgi.tmgi_list_ptr,
                               ind_data->embms_activate_status.activation_tmgi.tmgi_list_len ) < 0)
        {
          QDI_LOG_ERROR("%s", "can not process tmgi list");
          goto bail;
        }

        cmd->cmd_data.data.wds_ind.ind_data.embms_activate_status.activation_tmgi.tmgi_list_ptr =
          tmgi_list_ptr;
      }
      break;

    case QMI_WDS_SRVC_EMBMS_TMGI_DEACTIVATE_IND_MSG:
      {
        QDI_LOG_ERROR("%s", "process QMI_WDS_SRVC_EMBMS_TMGI_DEACTIVATE_IND_MSG");

        memcpy(&(cmd->cmd_data.data.wds_ind.ind_data.embms_deactivate_status),
               &(ind_data->embms_deactivate_status),
               sizeof(ind_data->embms_deactivate_status));

        if(qdi_copy_tmgi_list(&deact_tmgi_list_ptr,
                               ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr,
                               ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_len ) < 0)
        {
          QDI_LOG_ERROR("%s", "can not process tmgi list");
          goto bail;
        }
        cmd->cmd_data.data.wds_ind.ind_data.embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr =
          deact_tmgi_list_ptr;
      }
      break;

    case QMI_WDS_SRVC_EMBMS_TMGI_ACT_DEACT_IND_MSG:
      {
        QDI_LOG_ERROR("%s", "process QMI_WDS_SRVC_EMBMS_TMGI_ACT_DEACT_IND_MSG");

        memcpy(&(cmd->cmd_data.data.wds_ind.ind_data.embms_act_deact_status),
               &(ind_data->embms_act_deact_status),
               sizeof(ind_data->embms_act_deact_status ));

        if(qdi_copy_tmgi_list(&tmgi_list_ptr,
                              ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_ptr,
                              ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_len) < 0)
        {
          QDI_LOG_ERROR("%s", "can not process activate tmgi list");
          goto bail;
        }

        if(qdi_copy_tmgi_list(&deact_tmgi_list_ptr,
                              ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr,
                              ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_len) < 0)
        {
          QDI_LOG_ERROR("%s", "can not process deactivate tmgi list");
          goto bail;
        }
        cmd->cmd_data.data.wds_ind.ind_data.embms_act_deact_status.activation_tmgi.tmgi_list_ptr =
          tmgi_list_ptr;
        cmd->cmd_data.data.wds_ind.ind_data.embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr =
          deact_tmgi_list_ptr;
      }
      break;

    case QMI_WDS_SRVC_EMBMS_TMGI_LIST_IND_MSG:
      {
        QDI_LOG_ERROR("%s", "process QMI_WDS_SRVC_EMBMS_TMGI_LIST_IND_MSG");

        memcpy(&(cmd->cmd_data.data.wds_ind.ind_data.embms_list),
                 &(ind_data->embms_list),
                 sizeof(ind_data->embms_list));

        if(ind_data->embms_list.param_mask & QMI_WDS_EMBMS_LIST_IND_TMGI_LIST_PARAM_MASK )
        {
          if( 0 != ind_data->embms_list.tmgi_list.tmgi_list_len)
          {
          if(qdi_copy_tmgi_list(&tmgi_list_ptr,
                                  ind_data->embms_list.tmgi_list.tmgi_list_ptr,
                                  ind_data->embms_list.tmgi_list.tmgi_list_len ) < 0)
          {
            QDI_LOG_ERROR("%s", "can not process tmgi list");
            goto bail;
          }
            cmd->cmd_data.data.wds_ind.ind_data.embms_list.tmgi_list.tmgi_list_ptr = tmgi_list_ptr;
          }
        }
      }
      break;

    case QMI_WDS_SRVC_EMBMS_SAI_LIST_IND_MSG:
      {
        QDI_LOG_ERROR("%s", "process QMI_WDS_SRVC_EMBMS_SAI_LIST_IND_MSG");
        unsigned short *sai_list_ptr = NULL;

        memcpy(&(cmd->cmd_data.data.wds_ind.ind_data.sai_list),
               &(ind_data->sai_list),
               sizeof(ind_data->sai_list));
        if (ind_data->sai_list.param_mask & QMI_WDS_EMBMS_LIST_IND_SAI_LIST_PARAM_MASK)
        {
          if ( 0 != ind_data->sai_list.available_sai_list_len)
          {
            if(NULL ==
               (cmd->cmd_data.data.wds_ind.ind_data.sai_list.available_sai_list =
                malloc(ind_data->sai_list.available_sai_list_len * sizeof(unsigned int))))
            {
              QDI_LOG_ERROR("%s", "can not allocate memory for available_sai_list");
              goto bail;
            }
            memcpy(cmd->cmd_data.data.wds_ind.ind_data.sai_list.available_sai_list,
                   ind_data->sai_list.available_sai_list,
                   ind_data->sai_list.available_sai_list_len * sizeof(unsigned int));
          }

          if ( 0 != ind_data->sai_list.camped_sai_list_len)
          {
             if (NULL ==
                 (cmd->cmd_data.data.wds_ind.ind_data.sai_list.camped_sai_list =
                  malloc(ind_data->sai_list.camped_sai_list_len * sizeof(unsigned int))))
             {
               QDI_LOG_ERROR("%s", " can not allocate memory for camped_sai_list");
               goto bail;
             }
             memcpy(cmd->cmd_data.data.wds_ind.ind_data.sai_list.camped_sai_list,
                    ind_data->sai_list.camped_sai_list,
                    ind_data->sai_list.camped_sai_list_len * sizeof(unsigned int));
          }

          if (0 != ind_data->sai_list.num_sai_per_group_len)
          {
             if ( NULL ==
                  (cmd->cmd_data.data.wds_ind.ind_data.sai_list.num_sai_per_group =
                   malloc(ind_data->sai_list.num_sai_per_group_len * sizeof(unsigned short))))
             {
               QDI_LOG_ERROR("%s", " can not allocate memory for num_sai_per_group");
               goto bail;
             }
             memcpy(cmd->cmd_data.data.wds_ind.ind_data.sai_list.num_sai_per_group,
                    ind_data->sai_list.num_sai_per_group,
                    ind_data->sai_list.num_sai_per_group_len * sizeof(unsigned short));
          }
        }
      }
      break;

    case QMI_WDS_SRVC_EMBMS_CONTENT_DESC_CONTROL_IND_MSG:
      {
        QDI_LOG_ERROR("%s", "process QMI_WDS_SRVC_EMBMS_CONTENT_DESC_CONTROL_IND_MSG");

        memcpy(&(cmd->cmd_data.data.wds_ind.ind_data.embms_content_desc_control),
               &(ind_data->embms_content_desc_control),
               sizeof(ind_data->embms_content_desc_control));

        if(qdi_copy_tmgi_list(
              &tmgi_list_ptr,
              ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr,
              ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_len ) < 0)
        {
          QDI_LOG_ERROR("%s", "can not process tmgi list");
          goto bail;
        }
      }
      break;

    case QMI_WDS_SRVC_HANDOFF_INFORMATION_IND_MSG:
      {
        QDI_LOG_ERROR("%s", "process QMI_WDS_SRVC_HANDOFF_INFORMATION_IND_MSG");

        memcpy(&(cmd->cmd_data.data.wds_ind.ind_data.handoff_info),
               &(ind_data->handoff_info),
               sizeof(ind_data->handoff_info));
      }
      break;

    default:
      break;
  }
  /* Update the processing and free functions to call */
  cmd->ds_cmd.execute_f = qdi_process_wds_ind;
  cmd->ds_cmd.free_f    = qdi_cmdq_free;

  /* Enqueue the command to be processed later in cmdq thread context */
  if (ds_cmdq_enq(&qdi_cmdq, &cmd->ds_cmd) < 0)
  {
    QDI_LOG_ERROR("Failed to enqueue command for ind_id=%d",
                  ind_id);
    goto bail;
  }

  reti = QDI_SUCCESS;
  QDI_LOG_MED("enqueued ind_id=%d", ind_id);

bail:
  if(QDI_FAILURE == reti)
  {
    if(NULL != tmgi_list_ptr)
    {
      ds_free(tmgi_list_ptr);
    }
    if (NULL != deact_tmgi_list_ptr)
    {
      ds_free(deact_tmgi_list_ptr);
    }
    if (NULL != cmd)
    {
      if ( NULL != cmd->cmd_data.data.wds_ind.ind_data.sai_list.available_sai_list)
      {
        ds_free(cmd->cmd_data.data.wds_ind.ind_data.sai_list.available_sai_list);
      }
      if ( NULL != cmd->cmd_data.data.wds_ind.ind_data.sai_list.camped_sai_list)
      {
        ds_free(cmd->cmd_data.data.wds_ind.ind_data.sai_list.num_sai_per_group);
      }

      ds_free(cmd);
    }
  }

  QDI_LOG_TRACE_EXIT;
}


/*===========================================================================
  FUNCTION  qdi_wds_async_cb_hdlr
===========================================================================*/
/*!
@brief
  This is the indication handler registered with QMI

@return
  None
*/
/*=========================================================================*/
static void
qdi_wds_async_cb_hdlr
(
  int                          user_handle,
  qmi_service_id_type          service_id,
  int                          sys_err_code,
  int                          qmi_err_code,
  void                         *user_data,
  qmi_wds_async_rsp_id_type    rsp_id,
  qmi_wds_async_rsp_data_type  *rsp_data
)
{
  qdi_cmdq_cmd_type  *cmd = NULL;

  QDI_LOG_TRACE_ENTRY;

  if (!rsp_data)
  {
    QDI_LOG_ERROR("Invalid input param(s)");
    goto bail;
  }

  cmd = ds_malloc(sizeof(qdi_cmdq_cmd_type));

  if (!cmd)
  {
    QDI_LOG_ERROR("Failed to allocate command");
    goto bail;
  }

  /* Store the response data in the command buffer */
  cmd->cmd_data.cmd = QDI_CMDQ_CMD_TYPE_WDS_ASYNC_RESP;
  cmd->cmd_data.data.async_resp.user_handle  = user_handle;
  cmd->cmd_data.data.async_resp.service_id   = service_id;
  cmd->cmd_data.data.async_resp.sys_err_code = sys_err_code;
  cmd->cmd_data.data.async_resp.qmi_err_code = qmi_err_code;
  cmd->cmd_data.data.async_resp.user_data    = user_data;
  cmd->cmd_data.data.async_resp.rsp_id       = rsp_id;
  cmd->cmd_data.data.async_resp.rsp_data     = *rsp_data;

  /* Update the processing and free functions to call */
  cmd->ds_cmd.execute_f = qdi_process_wds_async_cb;
  cmd->ds_cmd.free_f    = qdi_cmdq_free;

  /* Enqueue the command to be processed later in cmdq thread context */
  if (ds_cmdq_enq(&qdi_cmdq, &cmd->ds_cmd) < 0)
  {
    QDI_LOG_ERROR("Failed to enqueue command for rsp_id=%d",
                  rsp_id);
    ds_free(cmd);
    goto bail;
  }

  QDI_LOG_MED("enqueued rsp_id=%d", rsp_id);

bail:
  QDI_LOG_TRACE_EXIT;
}


/*===========================================================================
  FUNCTION  qdi_indication_hdlr
===========================================================================*/
/*!
@brief
  This is the indication handler registered with QMI

@return
  None
*/
/*=========================================================================*/
static void
qdi_indication_hdlr
(
  int                           user_handle,
  qmi_service_id_type           service_id,
  void                          *user_data,
  qmi_wds_indication_id_type    ind_id,
  qmi_wds_indication_data_type  *ind_data
)
{
  qdi_client_handle_t qdi_handle = (qdi_client_handle_t)(intptr_t) user_data;
  qdi_call_info_t  *call_info = NULL;
  ds_dll_el_t      *node = NULL;
  ds_dll_el_t      *next = NULL;
  boolean          send_ind = FALSE;
  const void       *dummy = NULL;
  qdi_handle_info_t* info = NULL;
  QDI_GET_INFO_PTR(qdi_handle, info);

  QDI_LOG_TRACE_ENTRY;

  QDI_LOG_MED("user_handle=0x%x, qdi_handle=%d, ind_id=%d",
              user_handle,
              qdi_handle,
              ind_id);

  if (!info || !info->ind_msg_hdlr || !ind_data)
  {
    QDI_LOG_ERROR("Invalid input param(s)");
    QDI_LOG_TRACE_EXIT;
    return;
  }

  if (ind_id == QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG)
  {
    QDI_LOG_MED("dev=%s param_mask=0X%x link_status=%d reconfig_required=%d "
                "call_end_reason=%d,[%d/%d] ip_family=%d",
                (info->wds_id) ? info->wds_id : "",
                ind_data->pkt_srvc_status.param_mask,
                ind_data->pkt_srvc_status.link_status,
                (int) ind_data->pkt_srvc_status.reconfig_required,
                ind_data->pkt_srvc_status.call_end_reason.legacy_reason,
                ind_data->pkt_srvc_status.call_end_reason.call_end_reason_verbose.verbose_reason_type,
                ind_data->pkt_srvc_status.call_end_reason.call_end_reason_verbose.verbose_reason,
                ind_data->pkt_srvc_status.ip_family);
  }

  /* For a Dual-IP call, send only one pkt srvc indication for that call */
  if (QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG == ind_id)
  {
    /* Lock the mutex before updating state */
    QDI_MUTEX_LOCK(qdi_txn_mutex);

    /* Process the indication on all active calls */
    node = ds_dll_next(info->call_info_head, &dummy);

    for (; NULL != node; node = next)
    {
      call_info = (qdi_call_info_t *) ds_dll_data(node);

      QDI_LOG_MED("processing call_info object=%p, mode=0x%x, IPV4 cnt=%d, IPV6 cnt=%d",
                  call_info,
                  call_info->mode,
                  call_info->v4_pkt_srvc_conn_ind_cnt,
                  call_info->v6_pkt_srvc_conn_ind_cnt);

      next = ds_dll_next(node, &dummy);

      switch (ind_data->pkt_srvc_status.link_status)
      {
        case QMI_WDS_PACKET_DATA_CONNECTED:
          if (TRUE == ind_data->pkt_srvc_status.reconfig_required)
          {
            QDI_LOG_MED("Skipping increment of pkt srvc count during reconfiguration: mode=0x%x, v4_cnt=%d, v6_cnt=%d",
                        call_info->mode,
                        call_info->v4_pkt_srvc_conn_ind_cnt,
                        call_info->v6_pkt_srvc_conn_ind_cnt);
          }
          else
          {
            if (((QMI_WDS_IP_FAMILY_PREF_IPV4 == ind_data->pkt_srvc_status.ip_family ||
                  QMI_WDS_IP_FAMILY_PREF_UNSPECIFIED == ind_data->pkt_srvc_status.ip_family) &&
                 call_info->mode & QDI_MODE_V4) &&
                info->handle_ipv4 == user_handle)
            {
              if(call_info->v4_pkt_srvc_conn_ind_cnt == 0)
              {
                ++call_info->v4_pkt_srvc_conn_ind_cnt;
              }
              QDI_LOG_MED("IPV4 pkt_srvc_conn_ind_cnt=%d, mode=0x%x",
                          call_info->v4_pkt_srvc_conn_ind_cnt,
                          call_info->mode);
              QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "IPV4 PKT_CONNECTED ref_cnt=%d",
                                 call_info->v4_pkt_srvc_conn_ind_cnt);
              send_ind = TRUE;

            }
            else if ((QMI_WDS_IP_FAMILY_PREF_IPV6 == ind_data->pkt_srvc_status.ip_family &&
                      call_info->mode & QDI_MODE_V6) &&
                     info->handle_ipv6 == user_handle)
            {
              if(call_info->v6_pkt_srvc_conn_ind_cnt == 0)
              {
                ++call_info->v6_pkt_srvc_conn_ind_cnt;
              }
              QDI_LOG_MED("IPV6 pkt_srvc_conn_ind_cnt=%d, mode=0x%x",
                          call_info->v6_pkt_srvc_conn_ind_cnt,
                          call_info->mode);
              QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "IPV6 PKT_CONNECTED ref_cnt=%d",
                                 call_info->v6_pkt_srvc_conn_ind_cnt);
              send_ind = TRUE;
            }

          }
          break;

        case QMI_WDS_PACKET_DATA_DISCONNECTED:
        if ((QMI_WDS_IP_FAMILY_PREF_IPV4 == ind_data->pkt_srvc_status.ip_family ||
             QMI_WDS_IP_FAMILY_PREF_UNSPECIFIED == ind_data->pkt_srvc_status.ip_family) &&
            info->handle_ipv4 == user_handle)
        {
          if (call_info->v4_pkt_srvc_conn_ind_cnt > 0)
          {
            --call_info->v4_pkt_srvc_conn_ind_cnt;
            QDI_LOG_MED("IPV4 pkt_srvc_conn_ind_cnt=%d, mode=0x%x",
                        call_info->v4_pkt_srvc_conn_ind_cnt,
                        call_info->mode);
            QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "IPV4 PKT_DISCONNECTED ref_cnt=%d",
                               call_info->v4_pkt_srvc_conn_ind_cnt);
            send_ind = TRUE;
          }
          else
          {
            QDI_LOG_ERROR("unexpected IPV4 pkt_srvc_ind for link_status disconnected recvd");
          }
        }
        else if (QMI_WDS_IP_FAMILY_PREF_IPV6 == ind_data->pkt_srvc_status.ip_family &&
                 info->handle_ipv6 == user_handle)
        {
          if (call_info->v6_pkt_srvc_conn_ind_cnt > 0)
          {
            --call_info->v6_pkt_srvc_conn_ind_cnt;
            QDI_LOG_MED("IPV6 pkt_srvc_conn_ind_cnt=%d, mode=0x%x",
                        call_info->v6_pkt_srvc_conn_ind_cnt,
                        call_info->mode);
            QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "IPV6 PKT_DISCONNECTED ref_cnt=%d",
                               call_info->v6_pkt_srvc_conn_ind_cnt);
            send_ind = TRUE;
          }
          else
          {
            QDI_LOG_ERROR("unexpected IPV6 pkt_srvc_ind for link_status disconnected recvd");
          }
        }
        else
        {
          QDI_LOG_LOW("ignoring DISCONNECT pkt_srvc_ind with invalid ip_family=%d",
                      ind_data->pkt_srvc_status.ip_family);
          send_ind = FALSE;
          continue;
        }
        break;

        default:
          QDI_LOG_LOW("ignoring pkt_srvc_ind for link_status=%d",
                      ind_data->pkt_srvc_status.link_status);
          break;
      }
    }

    /* Unlock the mutex after updating state */
    QDI_MUTEX_UNLOCK(qdi_txn_mutex);
  }
  else
  {
    /* Follow above logic of dual-ip call based decision to send
       the indication only for pkt_srvc_ind. Send indication by default
       for all other indications */
    send_ind = TRUE;
  }

  if (send_ind)
  {
    info->ind_msg_hdlr(user_handle,
                       service_id,
                       info->ind_user_data,
                       ind_id,
                       ind_data);
  }

  /* free memory allocated for EMBMS */
  if((ind_id == QMI_WDS_SRVC_EMBMS_TMGI_ACTIVATE_IND_MSG) &&
     (NULL != ind_data->embms_activate_status.activation_tmgi.tmgi_list_ptr))
  {
    QDI_LOG_MED("free tmgi_list_ptr allocated from activate_ind_data");
    free(ind_data->embms_activate_status.activation_tmgi.tmgi_list_ptr);
    ind_data->embms_activate_status.activation_tmgi.tmgi_list_ptr = NULL;
  }

  else if((ind_id == QMI_WDS_SRVC_EMBMS_TMGI_DEACTIVATE_IND_MSG) &&
     (NULL != ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr))
  {
    QDI_LOG_MED("free tmgi_list_ptr allocated from deactivate_ind_data");
    free(ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr);
    ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr = NULL;
  }

  else if(ind_id == QMI_WDS_SRVC_EMBMS_TMGI_ACT_DEACT_IND_MSG)
  {
    if(NULL != ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_ptr)
  {
      QDI_LOG_MED("free activate tmgi_list_ptr allocated from act_deact_ind_data");
      free(ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_ptr);
      ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_ptr = NULL;
    }
    else if( NULL != ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr)
    {
      QDI_LOG_MED("free deactivate tmgi_list_ptr allocated from act_deact_ind_data");
      free(ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr);
      ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr = NULL;
    }
  }

  else if((ind_id == QMI_WDS_SRVC_EMBMS_TMGI_LIST_IND_MSG)&&
     (NULL != ind_data->embms_list.tmgi_list.tmgi_list_ptr))
  {
    QDI_LOG_MED("free tmgi_list_ptr allocated from list_ind_data");
    free(ind_data->embms_list.tmgi_list.tmgi_list_ptr);
    ind_data->embms_list.tmgi_list.tmgi_list_ptr = NULL;
  }

  else if((ind_id == QMI_WDS_SRVC_EMBMS_CONTENT_DESC_CONTROL_IND_MSG)&&
     (NULL != ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr))
  {
    QDI_LOG_MED("free tmgi_list_ptr allocated from content_desc_ctrl_ind_data");
    free(ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr);
    ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr = NULL;
  }

  else if (ind_id == QMI_WDS_SRVC_EMBMS_SAI_LIST_IND_MSG)
  {
     if ( NULL != ind_data->sai_list.available_sai_list)
     {
       free(ind_data->sai_list.available_sai_list);
       ind_data->sai_list.available_sai_list = NULL;
     }
     if ( NULL != ind_data->sai_list.camped_sai_list)
     {
       free(ind_data->sai_list.camped_sai_list);
       ind_data->sai_list.camped_sai_list = NULL;
     }
     if ( NULL != ind_data->sai_list.num_sai_per_group)
     {
       free(ind_data->sai_list.num_sai_per_group);
       ind_data->sai_list.num_sai_per_group = NULL;
     }
  }

  QDI_LOG_TRACE_EXIT;
}

/*===========================================================================
  FUNCTION  qdi_find_call_info_in_table
===========================================================================*/
/*!
@brief
  This function searches the qdi_hanle_info table for a matching input
  call_info handle

@return
  Valid qdi_handle on success
  QDI_INVALID_CLIENT_HANDLE on failure

@note
  qdi_txn_mutex must be locked before calling this function
*/
/*=========================================================================*/
static qdi_client_handle_t
qdi_find_call_info_in_table
(
  qdi_call_info_t  *call_info
)
{
  qdi_client_handle_t  qdi_handle = QDI_INVALID_CLIENT_HANDLE;
  int i;


  QDI_LOG_TRACE_ENTRY;

  if (!call_info)
  {
    QDI_LOG_ERROR("Invalid input param(s)");
    goto bail;
  }

  for (i = 0; i < QDI_MAXIMUM_HANDLES; i++)
  {
    qdi_handle_info_t  *info = &qdi_handle_info[i];

    if (TRUE == info->valid &&
        NULL != qdi_find_matching_call_info_node(info, call_info))
    {
      qdi_handle = QDI_INDEX_TO_HANDLE(i);
      break;
    }
  }

bail:
  QDI_LOG_TRACE_RETURN(qdi_handle);
  return qdi_handle;
}

/*===========================================================================
  FUNCTION  qdi_cb_hdlr
===========================================================================*/
/*!
@brief
  This is the callback handler registered with QMI for all asynchronous
  requests

@return
  None
*/
/*=========================================================================*/
static void
qdi_cb_hdlr
(
  int                          user_handle,
  qmi_service_id_type          service_id,
  int                          sys_err_code,
  int                          qmi_err_code,
  void                         *user_data,
  qmi_wds_async_rsp_id_type    rsp_id,
  qmi_wds_async_rsp_data_type  *rsp_data
)
{
  qdi_call_info_t  *call_info = (qdi_call_info_t *) user_data;
  qdi_client_handle_t qdi_handle;
  qdi_handle_info_t* info = NULL;
  qdi_txn_info_t* txn = NULL;
  int ip_family;

  QDI_LOG_TRACE_ENTRY;

  if (!call_info || !rsp_data)
  {
    QDI_LOG_ERROR("Invalid input param(s)");
    QDI_LOG_TRACE_EXIT;
    return;
  }

  QDI_LOG_MED("user_handle=0x%x, service_id=%d, rsp_id=0x%x, call_info=%p "
              "sys_err_code=%d, qmi_err_code=%d",
              user_handle,
              service_id,
              rsp_id,
              call_info,
              sys_err_code,
              qmi_err_code);

  /* Lock the transaction mutex */
  QDI_MUTEX_LOCK(qdi_txn_mutex);

  qdi_handle = qdi_find_call_info_in_table(call_info);

  if (QDI_INVALID_CLIENT_HANDLE == qdi_handle)
  {
    QDI_LOG_ERROR("Unable to locate qdi_handle for call_info=%p",
                  call_info);
    goto bail;
  }
  else if (call_info->handle != qdi_handle)
  {
    QDI_LOG_ERROR("Unable to match handle for call_info=%p, call_info->handle=%d, qdi_handle=%d",
                  call_info,
                  call_info->handle,
                  qdi_handle);
    goto bail;
  }

  QDI_GET_INFO_PTR(qdi_handle, info);

  if (!info)
  {
    QDI_LOG_ERROR("unable to locate info entry for qdi_handle=%d",
                  qdi_handle);
    goto bail;
  }

  QDI_LOG_MED("dev=%s qdi_handle=%d rsp_data={pkt_data_handle=%ld"
              " call_end_reason = {%d,%d/%d}}",
              (info->wds_id) ? info->wds_id : "",
              qdi_handle,
              rsp_data->start_nw_rsp.pkt_data_handle,
              rsp_data->start_nw_rsp.call_end_reason.legacy_reason,
              rsp_data->start_nw_rsp.call_end_reason.call_end_reason_verbose.verbose_reason_type,
              rsp_data->start_nw_rsp.call_end_reason.call_end_reason_verbose.verbose_reason);

  switch (rsp_id)
  {
    case QMI_WDS_SRVC_START_NW_ASYNC_RSP_MSG:
      txn = &call_info->start_if_txn;
      break;

    case QMI_WDS_SRVC_STOP_NW_ASYNC_RSP_MSG:
      txn = &call_info->stop_if_txn;
      break;

    case QMI_WDS_SRVC_ABORT_ASYNC_RSP_MSG:
      txn = &call_info->abort_txn;
      break;

    case QMI_WDS_SRVC_RESET_ASYNC_RSP_MSG: /* intentional fallthrough */
    case QMI_WDS_SRVC_INVALID_ASYNC_RSP_MSG:
    default:
      goto bail;
  }

  if (user_handle == info->handle_ipv4)
  {
    QDI_LOG_MED("callback is for IPv4 bound QMI WDS client");
    ip_family = AF_INET;
    txn->v4_cb_pend = FALSE;

    txn->cb_rsp_data.flags |= QDI_RSP_DATA_V4;
    memcpy(&txn->cb_rsp_data.rsp_data_v4.rsp_data,
           rsp_data,
           sizeof(txn->cb_rsp_data.rsp_data_v4.rsp_data));
    txn->cb_rsp_data.rsp_data_v4.sys_err = sys_err_code;
    txn->cb_rsp_data.rsp_data_v4.qmi_err = qmi_err_code;
  }
  else if (user_handle == info->handle_ipv6)
  {
    QDI_LOG_MED("callback is for IPv6 bound QMI WDS client");
    ip_family = AF_INET6;
    txn->v6_cb_pend = FALSE;

    txn->cb_rsp_data.flags |= QDI_RSP_DATA_V6;
    memcpy(&txn->cb_rsp_data.rsp_data_v6.rsp_data,
           rsp_data,
           sizeof(txn->cb_rsp_data.rsp_data_v6.rsp_data));
    txn->cb_rsp_data.rsp_data_v6.sys_err = sys_err_code;
    txn->cb_rsp_data.rsp_data_v6.qmi_err = qmi_err_code;
  }
  else
  {
    QDI_LOG_ERROR("Invalid QMI handle");
    goto bail;
  }

  if(sys_err_code == QMI_NO_ERR &&
     rsp_id == QMI_WDS_SRVC_START_NW_ASYNC_RSP_MSG &&
     QDI_INVALID_QMI_INST != call_info->rl_qmi_inst)
  {
    QDI_LOG_MED("Another call active on iface[%s] from a different process",info->dev_id);
    /* Query address and increment call_info ref count */
    if( QDI_SUCCESS != qdi_query_addr_and_incr_ref_count(call_info,
                                                         ip_family))
    {
      QDI_LOG_ERROR("Cannot increment pkt_srvc_cnt for ip_family[%d] on iface[%s]",
                    ip_family,
                    info->dev_id);
    }
  }

  if (txn->v4_cb_pend == FALSE && txn->v6_cb_pend == FALSE)
  {
    /* There are no more callbacks expected for this command.  Call client
       callback. */
    QDI_LOG_HIGH("Calling client callback function");


    /* For a Dual-IP call, send a SUCCESS if one of the families returns
       a SUCCESS */
    if (QDI_MODE_V4V6 == call_info->mode &&
        QDI_RSP_DATA_V4_V6 == (txn->cb_rsp_data.flags & QDI_RSP_DATA_V4_V6))
    {
      if (QMI_NO_ERR == txn->cb_rsp_data.rsp_data_v4.sys_err)
      {
        sys_err_code = txn->cb_rsp_data.rsp_data_v4.sys_err;
        qmi_err_code = txn->cb_rsp_data.rsp_data_v4.qmi_err;
      }
      else if (QMI_NO_ERR == txn->cb_rsp_data.rsp_data_v6.sys_err)
      {
        sys_err_code = txn->cb_rsp_data.rsp_data_v6.sys_err;
        qmi_err_code = txn->cb_rsp_data.rsp_data_v6.qmi_err;
      }
      else
      {
        /* Both V4 and V6 start_nw_if() returned error. Check if there's a NO_EFFECT
           error on either of them */
        if (QMI_SERVICE_ERR == txn->cb_rsp_data.rsp_data_v6.sys_err &&
            (QMI_SERVICE_ERR_NO_EFFECT == txn->cb_rsp_data.rsp_data_v6.qmi_err ||
             QMI_SERVICE_ERR_NONE == txn->cb_rsp_data.rsp_data_v6.qmi_err))
        {
          sys_err_code = txn->cb_rsp_data.rsp_data_v6.sys_err;
          qmi_err_code = txn->cb_rsp_data.rsp_data_v6.qmi_err;
        }
        else
        {
          sys_err_code = txn->cb_rsp_data.rsp_data_v4.sys_err;
          qmi_err_code = txn->cb_rsp_data.rsp_data_v4.qmi_err;
        }
      }
    }

    QDI_LOG_LOW("qdi_cb_hdlr: user_data=0x%lx", (unsigned long)txn->user_data);

    if (txn->cb_hdlr)
    {
      txn->cb_hdlr(qdi_handle,
                   service_id,
                   sys_err_code,
                   qmi_err_code,
                   txn->user_data,
                   rsp_id,
                   &txn->cb_rsp_data);
    }
    else
    {
      QDI_LOG_ERROR("qdi_cb_hdlr: unexpected callback received txn_v4=%d, txn_v6=%d",
                    txn->txn_v4,
                    txn->txn_v6);
    }

    /* Clear the transaction data */
    txn->txn_v4 = QDI_INVALID_TXN_HNDL;
    txn->txn_v6 = QDI_INVALID_TXN_HNDL;
    txn->cb_hdlr   = NULL;
    txn->user_data = NULL;
    memset(&txn->cb_rsp_data, 0, sizeof(txn->cb_rsp_data));
  }
  else
  {
    QDI_LOG_HIGH("Delay calling client callback function until both IPv4 and "
                 "IPv6 callbacks arrive.");
  }

bail:
  /* Unlock the transaction mutex */
  QDI_MUTEX_UNLOCK(qdi_txn_mutex);

  QDI_LOG_TRACE_EXIT;
}

/*===========================================================================
  FUNCTION qdi_convert_gateway
===========================================================================*/
/*!
@brief
  This function does an inplace conversion of a global gateway address
  to link local address.

  Example:
  2002:c023:9c17:0c23:c979:4631:83b4:fff1 will be converted to
  fe80:0000:0000:0000:c979:4631:83b4:fff1

@return
  QDI_SUCCESS on successful operation
  QDI_FAILURE otherwise
*/
/*=========================================================================*/
static int qdi_convert_gateway
(
  char *gateway_addr
)
{
  if (!gateway_addr)
  {
    QDI_LOG_ERROR("qdi_convert_gateway:invalid input received");
    return QDI_FAILURE;
  }
  /*first two bytes: fe80*/
  gateway_addr[0] = QDI_IPV6_LINK_LOCAL_PREFIX_BYTE0;
  gateway_addr[1] = QDI_IPV6_LINK_LOCAL_PREFIX_BYTE1;

  /*next six bytes: 0000 0000 0000*/
  memset(gateway_addr + QDI_IPV6_ZERO_FILLER_START_POS,
         0,
         QDI_IPV6_ZERO_FILLER_END_POS -
           QDI_IPV6_ZERO_FILLER_START_POS);

  return QDI_SUCCESS;
}

/*===========================================================================
  FUNCTION  qdi_get_modem_link_info
===========================================================================*/
/*!
@brief
  Function to query Modem WDS information for specific link.

@return
  QDI_SUCCESS on successful operation
  QDI_FAILURE otherwise
*/
/*=========================================================================*/
static int qdi_get_modem_link_info
(
  qmi_client_handle_type  client_hndl,
  int                     ip_family,
  dsi_addr_info_t         *addr
)
{
  qmi_wds_req_runtime_settings_params_type req_mask;
  qmi_wds_profile_id_type            profile_id;
  qmi_wds_profile_params_type        profile_params;
  qmi_wds_curr_call_info_type        call_info;
  int err_code;
  int result;
  int ret = QDI_FAILURE;
  struct sockaddr_in *sin   = NULL;
  struct sockaddr_in6 *sin6 = NULL;
  struct sockaddr_storage* __attribute__((__may_alias__)) addr_ptr;

  QDI_LOG_TRACE_ENTRY;

  req_mask =
    (
      QMI_WDS_GET_CURR_CALL_INFO_IP_FAMILY_PARAM_MASK         |
      QMI_WDS_GET_CURR_CALL_INFO_GATEWAY_INFO_PARAM_MASK      |
      QMI_WDS_GET_CURR_CALL_INFO_DNS_ADDR_PARAM_MASK
    );

  /* Query Modem for requested parameters */
  result = qmi_wds_get_curr_call_info( client_hndl,
                                       req_mask,
                                       &profile_id,
                                       &profile_params,
                                       &call_info,
                                       &err_code);
  if( QMI_NO_ERR != result ) {
    QDI_LOG_ERROR("failed on qmi_wds_get_curr_call_info: "
                  "ret=%d err=%d\n",
                  result, err_code);
    goto bail;
  }

  /* Verify that the address family is expected */
  if (QMI_WDS_CURR_CALL_INFO_IP_FAMILY & call_info.mask)
  {
    qmi_wds_ip_family  wds_ip_family;
    wds_ip_family = (ip_family == AF_INET) ? QMI_WDS_IPV4_TYPE : QMI_WDS_IPV6_TYPE;

    if (call_info.ip_family != wds_ip_family)
    {
      QDI_LOG_ERROR("IP address family mismatch, recv ip_family:%d, req ip_family: %d\n",
                    call_info.ip_family,
                    wds_ip_family);
      goto bail;
    }
  }
  else
  {
    QDI_LOG_ERROR("IP address family undefined\n");
    goto bail;
  }

  /* Process Gateway address */
  if (QMI_WDS_CURR_CALL_INFO_IPV4_GATEWAY_ADDR & call_info.mask)
  {
    uint32_t gw_addr = htonl( (uint32_t)call_info.ipv4_gateway_addr );
    addr_ptr = &addr->gtwy_addr_s.addr;
    sin = (struct sockaddr_in*)addr_ptr;

    addr->gtwy_addr_s.addr.ss_family = (sa_family_t)ip_family;
    memcpy(SASTORAGE_DATA(addr->gtwy_addr_s.addr),
           &gw_addr,
           MIN(sizeof(gw_addr),sizeof(sin->sin_addr)));
    addr->gtwy_addr_s.valid_addr = TRUE;
  }
  else if (QMI_WDS_CURR_CALL_INFO_IPV6_GTWY_ADDR & call_info.mask)
  {
    addr_ptr = &addr->gtwy_addr_s.addr;
    sin6 = (struct sockaddr_in6*)addr_ptr;

    addr->gtwy_addr_s.addr.ss_family = (sa_family_t)ip_family;
    /*convert only if we get a global scope gateway*/
    if (call_info.ipv6_gateway_addr_info.ipv6_prefix_len ==
          QDI_IPV6_SLAAC_PREFIX_LENGTH)
    {
      qdi_convert_gateway((char *)&call_info.ipv6_gateway_addr_info.ipv6_addr);
    }
    memcpy(SASTORAGE_DATA(addr->gtwy_addr_s.addr),
           call_info.ipv6_gateway_addr_info.ipv6_addr,
           MIN(sizeof(call_info.ipv6_gateway_addr_info.ipv6_addr),sizeof(sin6->sin6_addr)));
    addr->gtwy_addr_s.valid_addr = TRUE;
  }
  else
  {
    QDI_LOG_ERROR("Gateway IP address undefined\n");
    goto bail;
  }

  /* Process DNS addresses (primary & secondary) */
  memset(&addr->dnsp_addr_s, 0x0, sizeof(addr->dnsp_addr_s));
  memset(&addr->dnss_addr_s, 0x0, sizeof(addr->dnss_addr_s));

  if (QMI_WDS_CURR_CALL_INFO_PRIMARY_DNS_IPV4_ADDR & call_info.mask)
  {
    uint32 dnsp_addr = htonl( (uint32_t)call_info.primary_dns_ipv4_addr );
    addr_ptr = &addr->gtwy_addr_s.addr;
    sin = (struct sockaddr_in*)addr_ptr;

    addr->dnsp_addr_s.addr.ss_family = (sa_family_t)ip_family;
    memcpy(SASTORAGE_DATA(addr->dnsp_addr_s.addr),
           &dnsp_addr,
           MIN(sizeof(dnsp_addr),sizeof(sin->sin_addr)));
    addr->dnsp_addr_s.valid_addr = TRUE;
  }
  else if (QMI_WDS_CURR_CALL_INFO_PRIMARY_DNS_IPV6_ADDR & call_info.mask)
  {
    addr_ptr = &addr->dnsp_addr_s.addr;
    sin6 = (struct sockaddr_in6*)addr_ptr;

    addr->dnsp_addr_s.addr.ss_family = (sa_family_t)ip_family;
    memcpy(SASTORAGE_DATA(addr->dnsp_addr_s.addr),
           call_info.primary_dns_ipv6_addr,
           MIN(sizeof(call_info.primary_dns_ipv6_addr),sizeof(sin6->sin6_addr)));
    addr->dnsp_addr_s.valid_addr = TRUE;
  }
  else
  {
    QDI_LOG_ERROR("DNS primary address undefined\n");
  }

  if (QMI_WDS_CURR_CALL_INFO_SECONDARY_DNS_IPV4_ADDR & call_info.mask)
  {
    uint32 dnss_addr = htonl( (uint32_t)call_info.secondary_dns_ipv4_addr );
    addr_ptr = &addr->dnss_addr_s.addr;
    sin = (struct sockaddr_in*)addr_ptr;

    addr->dnss_addr_s.addr.ss_family = (sa_family_t)ip_family;
    memcpy(SASTORAGE_DATA(addr->dnss_addr_s.addr),
           &dnss_addr,
           MIN(sizeof(dnss_addr),sizeof(sin->sin_addr)));
    addr->dnss_addr_s.valid_addr = TRUE;
  }
  else if (QMI_WDS_CURR_CALL_INFO_SECONDARY_DNS_IPV6_ADDR & call_info.mask)
  {
    addr_ptr = &addr->dnss_addr_s.addr;
    sin6 = (struct sockaddr_in6*)addr_ptr;

    addr->dnss_addr_s.addr.ss_family = (sa_family_t)ip_family;
    memcpy(SASTORAGE_DATA(addr->dnss_addr_s.addr),
           call_info.secondary_dns_ipv6_addr,
           MIN(sizeof(call_info.secondary_dns_ipv6_addr),sizeof(sin6->sin6_addr)));
    addr->dnss_addr_s.valid_addr = TRUE;
  }
  else
  {
    QDI_LOG_ERROR("DNS secondary address undefined\n");
  }

  ret = QDI_SUCCESS;

bail:
  QDI_LOG_TRACE_RETURN(ret);
  return ret;
}


/*===========================================================================
  FUNCTION  qdi_validate_call_attempt
===========================================================================*/
/*!
@brief
  This function determintes if the current call attempt is valid or not

@return
  QDI_SUCCESS - if the call attempt is valid
  QDI_FAILURE - otherwise

@note
  qdi_txn_mutex must be locked before calling this function
*/
/*=========================================================================*/
static int
qdi_validate_call_attempt
(
  qdi_client_handle_t  user_handle,
  qdi_call_info_t      *call_info,
  boolean              partial_retry,
  qdi_mode_t           mode
)
{
  int ret = QDI_FAILURE;
  int rc = QMI_INTERNAL_ERR, qmi_err_code;
  qdi_handle_info_t *info = NULL;
  QDI_GET_INFO_PTR(user_handle, info);


  QDI_LOG_TRACE_ENTRY;

  if (!info || !call_info)
  {
    QDI_LOG_ERROR("qdi_validate_call_attempt: Invalid input param(s)");
    goto bail;
  }

  /* Make sure that this is a valid call handle */
  if (call_info->handle != user_handle ||
      NULL == qdi_find_matching_call_info_node(info, call_info))
  {
    QDI_LOG_ERROR("qdi_validate_call_attempt: Invalid call_hndl=%p", call_info);
    goto bail;
  }

  /* New call attempt */
  if (FALSE == partial_retry)
  {
    QDI_LOG_MED("qdi_validate_call_attempt: new call attempt");

    /* If the pkt_srvc counters are non-zero for a new call attempt,
       issue a stop_nw_if() and fail the call attempt */
    if (0 != call_info->v4_pkt_srvc_conn_ind_cnt ||
        0 != call_info->v6_pkt_srvc_conn_ind_cnt)
    {
      QDI_LOG_ERROR("qdi_validate_call_attempt: non-zero pkt_srv counters call_info=%p, v4=%d, v6=%d",
                    call_info,
                    call_info->v4_pkt_srvc_conn_ind_cnt,
                    call_info->v6_pkt_srvc_conn_ind_cnt);

      if (QMI_NO_ERR != (rc = qmi_wds_stop_nw_if(info->handle_ipv4,
                                                 NULL,
                                                 NULL,
                                                 &qmi_err_code)))
      {
        QDI_LOG_ERROR("Failed to stop V4 call rc=%d, err=%d",
                      rc, qmi_err_code);
      }

      if (QMI_NO_ERR != (rc = qmi_wds_stop_nw_if(info->handle_ipv6,
                                                 NULL,
                                                 NULL,
                                                 &qmi_err_code)))
      {
        QDI_LOG_ERROR("Failed to stop V6 call rc=%d, err=%d",
                      rc, qmi_err_code);
      }

      goto bail;
    }
    else
    {
      /* Save the mode for the new call */
      call_info->mode = mode;
    }
  }
  /* Partial retry attempt */
  else
  {
    QDI_LOG_MED("qdi_validate_call_attempt: partial retry call attempt");

    /* If the pkt_srvc counters are zero for a partial retry attempt,
       fail the call attempt */
    if (0 == call_info->v4_pkt_srvc_conn_ind_cnt &&
        0 == call_info->v6_pkt_srvc_conn_ind_cnt)
    {
      QDI_LOG_ERROR("qdi_validate_call_attempt: zero pkt_srv counters call_info=%p, v4=%d, v6=%d",
                    call_info,
                    call_info->v4_pkt_srvc_conn_ind_cnt,
                    call_info->v6_pkt_srvc_conn_ind_cnt);
      goto bail;
    }
    else if (mode != call_info->mode)
    {
      QDI_LOG_ERROR("qdi_validate_call_attempt: call IP mode mismatch call_info=%p, req=%d, actual=%d",
                    call_info,
                    mode,
                    call_info->mode);
      goto bail;
    }
  }

  ret = QDI_SUCCESS;

bail:
  QDI_LOG_TRACE_RETURN(ret);
  return ret;
}


/*===========================================================================
  FUNCTION  qdi_query_addr_and_incr_ref_count
===========================================================================*/
/*!
@brief
  This function queries netmgr for presence of a valid ip_family address
  on the iface specified in the call_info structure and increments the
  corresponding ref counter in call_info structure.

@return
  QDI_SUCCESS - on successful query to netmgr
  QDI_FAILURE - otherwise
*/
/*=========================================================================*/
static int
qdi_query_addr_and_incr_ref_count
(
  qdi_call_info_t     *call_info,
  int                 ip_family
)
{
  qdi_client_handle_t qdi_handle;
  qdi_handle_info_t *info = NULL;
  dsi_addr_info_t  addr_info;

  QDI_LOG_TRACE_ENTRY;

  if( !call_info )
  {
    QDI_LOG_ERROR("Invalid input parameters call_info=%p", call_info);
    QDI_LOG_TRACE_RETURN(QDI_FAILURE);
    return QDI_FAILURE;
  }

  qdi_handle = call_info->handle;
  QDI_GET_INFO_PTR(qdi_handle, info);

  if( !QDI_IS_VALID_HNDL(info) )
  {
    QDI_LOG_FATAL("Valid call_info=%p. Invalid qdi_handle=%d, info=%p . "
                  "Possible memory corruption",
                  call_info, qdi_handle, info);
    QDI_LOG_TRACE_RETURN(QDI_FAILURE);
    return QDI_FAILURE;
  }

  if( QDI_SUCCESS != qdi_get_addr_info(qdi_handle,
                                       info->dev_id,
                                       ip_family,
                                       &addr_info,
                                       QMI_WDS_IFACE_NAME_NOT_REPORTED))
  {
    QDI_LOG_ERROR("No valid address present for ip_family[%d] on iface=[%s]. Not incrementing cnt",
                   ip_family, info->dev_id);
    QDI_LOG_TRACE_RETURN(QDI_FAILURE);
    return QDI_FAILURE;
  }
  else
  {
    switch(ip_family)
    {
      case AF_INET:
        if(call_info->v4_pkt_srvc_conn_ind_cnt == 0)
        {
          ++call_info->v4_pkt_srvc_conn_ind_cnt;
        }
        break;
      case AF_INET6:
        if(call_info->v6_pkt_srvc_conn_ind_cnt == 0)
        {
          ++call_info->v6_pkt_srvc_conn_ind_cnt;
        }
        break;
      default:
        QDI_LOG_ERROR("Invalid ip_family received");
        break;
    }
  }

  QDI_LOG_LOW("qdi_handle=%d call_info=%p v4_pkt_ind_cnt=%d v6_pkt_ind_cnt=%d",
              qdi_handle,
              call_info,
              call_info->v4_pkt_srvc_conn_ind_cnt,
              call_info->v6_pkt_srvc_conn_ind_cnt);
  QDI_LOG_TRACE_RETURN(QDI_SUCCESS);
  return QDI_SUCCESS;
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qdi_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the QDI library.  This function
  must be called prior to calling any other QDI functions.

@return
  QDI_SUCCESS if the operation was sucessful
  QDI_FAILURE otherwise

*/
/*=========================================================================*/
int
qdi_init (void)
{
  /* Initialize the QDI command queue */
  return ds_cmdq_init(&qdi_cmdq, QDI_CMDQ_MAX_CMDS);
}


/*===========================================================================
  FUNCTION  qdi_release
===========================================================================*/
/*!
@brief
  This function is called to deinitialize the QDI library.

@return
  QDI_SUCCESS if the operation was sucessful
  QDI_FAILURE otherwise

*/
/*=========================================================================*/
int
qdi_release (void)
{
  /* De-initialize the QDI command queue */
  return ds_cmdq_deinit(&qdi_cmdq);
}


/*===========================================================================
  FUNCTION  qdi_wds_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the QDI service.  This function
  must be called prior to calling any other QDI service functions.

@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
  - Dependencies
    - qmi_connection_init() must be called for the associated port first.
*/
/*=========================================================================*/
qdi_client_handle_t
qdi_wds_srvc_init_client
(
  const char                             *wds_id,
  const char                             *dev_id,
  qmi_wds_indication_hdlr_type           user_ind_msg_hdlr,
  void                                   *user_ind_msg_hdlr_user_data,
  qmi_wds_bind_mux_data_port_params_type *bind_params,
  qmi_wds_bind_subscription_type          subs_id,
  int                                    *qmi_err_code
)
{
  int i, err;
  int ret = QMI_INTERNAL_ERR;
  qdi_handle_info_t* info = NULL;
  qdi_client_handle_t qdi_handle = QDI_INVALID_CLIENT_HANDLE;

  QDI_LOG_TRACE_ENTRY;

  for (i = 0; i < QDI_MAXIMUM_HANDLES; i++)
  {
    if (!qdi_handle_info[i].valid)
    {
      info = &qdi_handle_info[i];
      break;
    }
  }

  if (!info)
  {
    QDI_LOG_ERROR("No more qdi handles available");
    goto error;
  }
  else if (!wds_id            ||
           !dev_id            ||
           !user_ind_msg_hdlr ||
           !qmi_err_code)
  {
    QDI_LOG_ERROR("invalid input param(s)");
    goto error;
  }

  qdi_handle = QDI_INDEX_TO_HANDLE(i);

  memset(info, 0x0, sizeof(qdi_handle_info_t));

  /* Save the device name */
  info->wds_id = wds_id;
  info->dev_id = dev_id;

  /* Initialize QMI WDS clients for both IP family versions.*/
  if ((info->handle_ipv4 = qmi_wds_srvc_init_client(wds_id,
                                                    qdi_wds_ind_hdlr,
                                                    (void *)(intptr_t)qdi_handle,
                                                    qmi_err_code)) < 0)
  {
    ret = info->handle_ipv4;
    QDI_LOG_ERROR("Error initializing IPv4 bound QMI WDS client");
    goto error;
  }
  else if( NULL != bind_params )
  {
    if (QMI_NO_ERR != qmi_wds_bind_mux_data_port(info->handle_ipv4,
                                                 bind_params,
                                                 qmi_err_code))
    {
      QDI_LOG_ERROR("qdi_wds_srvc_init_clients: qmi_wds_bind_mux_data_port failed "
                     "for conn_id %p qmi_cid_v4 %d with error %d,\n",
                     (int*)user_ind_msg_hdlr_user_data, info->handle_ipv4, *qmi_err_code);
    }

    QDI_LOG_MED("qdi_wds_srvc_init_clients: qmi_wds_bind_mux_data_port successful"
                   "for conn_id %p qmi_cid_v4 %d, EPID %d:0x%lx\n",
                   (int*)user_ind_msg_hdlr_user_data, info->handle_ipv4,
                   bind_params->ep_id.ep_type, bind_params->ep_id.iface_id);
  }

  if ((info->handle_ipv6 = qmi_wds_srvc_init_client(wds_id,
                                                    qdi_wds_ind_hdlr,
                                                    (void *)(intptr_t)qdi_handle,
                                                    qmi_err_code)) < 0)
  {
    ret = info->handle_ipv6;
    QDI_LOG_ERROR("Error initializing IPv6 bound QMI WDS client");
    goto error;
  }
  else if( NULL != bind_params)
  {
    if (QMI_NO_ERR != qmi_wds_bind_mux_data_port(info->handle_ipv6,
                                                 bind_params,
                                                 qmi_err_code))
    {
      QDI_LOG_ERROR("qdi_wds_srvc_init_clients: qmi_wds_bind_mux_data_port failed "
                     "for conn_id %p qmi_cid_v6 %d with error %d,\n",
                     (int*)user_ind_msg_hdlr_user_data, info->handle_ipv6, *qmi_err_code);
    }

    QDI_LOG_MED("qdi_wds_srvc_init_clients: qmi_wds_bind_mux_data_port successful"
                   "for conn_id %p qmi_cid_v6 %d, EPID %d:0x%lx\n",
                   (int*)user_ind_msg_hdlr_user_data, info->handle_ipv6,
                   bind_params->ep_id.ep_type, bind_params->ep_id.iface_id);
  }

  if (QMI_NO_ERR != qmi_wds_bind_subscription ( info->handle_ipv4,
                              (qmi_wds_bind_subscription_type)subs_id,
                               qmi_err_code))
  {
    QDI_LOG_ERROR("qdi_wds_srvc_init_clients: subs_id binding failed");
  }

  if (QMI_NO_ERR != qmi_wds_bind_subscription ( info->handle_ipv6,
                              (qmi_wds_bind_subscription_type)subs_id,
                               qmi_err_code))
  {
    QDI_LOG_ERROR("qdi_wds_srvc_init_clients: subs_id binding failed");
  }

  QDI_LOG_LOW("ipv4 / ipv6 handles = 0x%08x / 0x%08x", info->handle_ipv4, info->handle_ipv6);

  info->call_info_head = ds_dll_init(NULL);

  if (!info->call_info_head)
  {
    goto error;
  }

  info->ind_user_data = user_ind_msg_hdlr_user_data;
  info->ind_msg_hdlr = user_ind_msg_hdlr;
  info->valid = TRUE;

  QDI_LOG_TRACE_RETURN(qdi_handle);
  return qdi_handle;

error:
  if (info)
  {
    /* Ignore error code returned by release_client. qmi_err_code has
     most applicable error value. */
    if (info->handle_ipv4 > 0)
    {
      if (qmi_wds_srvc_release_client(info->handle_ipv4, &err) < 0)
      {
        QDI_LOG_ERROR("Failed to release IPv4 QMI WDS client handle");
      }
    }

    if (info->handle_ipv6 > 0)
    {
      if (qmi_wds_srvc_release_client(info->handle_ipv6, &err) < 0)
      {
        QDI_LOG_ERROR("Failed to release IPv6 QMI WDS client handle");
      }
    }
  }

  QDI_LOG_TRACE_RETURN(ret);
  return ret;
}

/*===========================================================================
  FUNCTION  qdi_wds_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qdi_wds_srvc_init_client() function.

@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
  - Dependencies
    - qmi_connection_init() must be called for the associated port first.
*/
/*=========================================================================*/
int
qdi_wds_srvc_release_client
(
  qdi_client_handle_t  user_handle,
  int                  *qmi_err_code
)
{
  int ret = QMI_NO_ERR;
  int ret_v4 = QMI_INTERNAL_ERR, ret_v6 = QMI_INTERNAL_ERR;
  int qmi_err_v4 = QMI_SERVICE_ERR_NONE, qmi_err_v6 = QMI_SERVICE_ERR_NONE;
  int err;
  const void   *dummy = NULL;
  ds_dll_el_t  *node = NULL;
  ds_dll_el_t  *next = NULL;
  qdi_handle_info_t* info;
  QDI_GET_INFO_PTR(user_handle, info);

  QDI_LOG_TRACE_ENTRY;

  if (!info || !qmi_err_code)
  {
    QDI_LOG_ERROR("Invalid input param(s)");
    QDI_LOG_TRACE_RETURN(QMI_INTERNAL_ERR);
    return QMI_INTERNAL_ERR;
  }

  /* we may run into errors below, but we should make
     this handle available regardless to allow re-attempt */
  info->valid = FALSE;

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  /* Release both clients.  If there is an error, return the corresponding
     error codes */
  if ((ret_v4 = qmi_wds_srvc_release_client(info->handle_ipv4,
                                            &qmi_err_v4)) < 0)
  {
    QDI_LOG_ERROR("Failed to release IPv4 QMI WDS client handle");
    *qmi_err_code = qmi_err_v4;
    ret = ret_v4;
  }

  if ((ret_v6 = qmi_wds_srvc_release_client(info->handle_ipv6,
                                           &qmi_err_v6)) < 0)
  {
    QDI_LOG_ERROR("Failed to release IPv6 QMI WDS client handle");
    *qmi_err_code = qmi_err_v6;
    ret = ret_v6;
  }

  /* Lock the txn mutex */
  QDI_MUTEX_LOCK(qdi_txn_mutex);

  /* Remove any call objects */
  node = ds_dll_next(info->call_info_head, &dummy);
  while (NULL != node)
  {
    next = ds_dll_next(node, &dummy);
    qdi_cleanup_call_info_node(info, (qdi_call_info_t *)ds_dll_data(node));
    node = next;
  }

  /* Destroy the list */
  ds_dll_destroy(info->call_info_head);
  info->call_info_head = NULL;

  /* Unlock the txn mutex */
  QDI_MUTEX_UNLOCK(qdi_txn_mutex);

  QDI_LOG_TRACE_RETURN(ret);
  return ret;
}

/*===========================================================================
  FUNCTION  qdi_alloc_call_handle
===========================================================================*/
/*!
@brief
  This function is called to allocate a new call handle

@return
  Valid call handle on success
  QDI_INVALID_CALL_HANDLE on error

@note
  None
*/
/*=========================================================================*/
qdi_call_handle_t
qdi_alloc_call_handle
(
  qdi_client_handle_t  user_handle
)
{
  qdi_call_info_t    *call_info = NULL;
  qdi_handle_info_t  *info = NULL;


  QDI_GET_INFO_PTR(user_handle, info);

  QDI_LOG_TRACE_ENTRY;

  if (!QDI_IS_VALID_HNDL(info))
  {
    QDI_LOG_ERROR("Invalid input user_handle=%d",
                  user_handle);
    goto bail;
  }

  call_info = ds_malloc(sizeof(qdi_call_info_t));

  if (!call_info)
  {
    QDI_LOG_ERROR("qdi_alloc_call_handle: failed to allocate memory");
    goto bail;
  }

  memset(call_info, 0, sizeof(qdi_call_info_t));
  call_info->handle = user_handle;
  /* Set the route lookup qmi instance to invalid
     It will be set appropriately by doing rl in dsi.
     The correct instance id will be passed to qdi when
     there is SNI call from another process*/
  call_info->rl_qmi_inst = QDI_INVALID_QMI_INST;

  /* Lock the transaction mutex */
  QDI_MUTEX_LOCK(qdi_txn_mutex);

  /* Add to the call list */
  if (!ds_dll_enq(info->call_info_head, NULL, call_info))
  {
    QDI_LOG_ERROR("qdi_wds_start_nw_if: failed to enqueue call_info=%p",
                  call_info);
    ds_free(call_info);
    call_info = NULL;
  }
  else
  {
    QDI_LOG_LOW("qdi_alloc_call_handle: enqueued call_info=%p",
                call_info);
  }

  /* Unlock the transaction mutex */
  QDI_MUTEX_UNLOCK(qdi_txn_mutex);

bail:
  QDI_LOG_TRACE_RETURN_PTR(call_info);
  return (qdi_call_handle_t)call_info;
}

/*===========================================================================
  FUNCTION  qdi_release_call_handle
===========================================================================*/
/*!
@brief
  This function is called to release a previously allocated call handle

@return
  None

*/
/*=========================================================================*/
void
qdi_release_call_handle
(
  qdi_call_handle_t  call_hndl
)
{
  qdi_call_info_t      *call_info = (qdi_call_info_t *)call_hndl;
  qdi_handle_info_t    *info = NULL;
  qdi_client_handle_t  user_handle;


  QDI_LOG_TRACE_ENTRY;

  if (!call_info)
  {
    QDI_LOG_ERROR("Invalid input call_hndl=%p", call_info);
    goto bail;
  }

  user_handle = call_info->handle;

  QDI_GET_INFO_PTR(user_handle, info);

  if (!QDI_IS_VALID_HNDL(info))
  {
    QDI_LOG_ERROR("Invalid user_handle=%d", user_handle);
    goto bail;
  }

  /* Lock the transaction mutex */
  QDI_MUTEX_LOCK(qdi_txn_mutex);

  qdi_cleanup_call_info_node(info, call_info);

  /* Unlock the transaction mutex */
  QDI_MUTEX_UNLOCK(qdi_txn_mutex);

bail:
  QDI_LOG_TRACE_EXIT;
}

/*===========================================================================
  FUNCTION  qdi_wds_start_nw_if
===========================================================================*/
/*!
@brief
  This function is used to bring up a data call. Call profile parameters are
  specified in the params parameter.

  The result of this call is reported asynchronously via the user_cb
  callback function.

@return
  If the return code < 0, the operation failed and there won't be an
  asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qdi_wds_abort() command.

@note
  - Dependencies
    - qdi_wds_srvc_init_client() must be called before calling this.
*/
/*=========================================================================*/
int
qdi_wds_start_nw_if
(
  qdi_client_handle_t              user_handle,
  qdi_call_handle_t                call_hndl,
  qmi_wds_start_nw_if_params_type  *params,
  boolean                          partial_retry,
  qdi_wds_user_async_cb_type       user_cb,
  void                             *user_data,
  int                              rl_qmi_inst,
  qmi_wds_call_end_reason_type     *call_end_reason_resp,
  int                              *qmi_err_code
)
{
  int ret = QMI_INTERNAL_ERR;
  int ret_v4 = QMI_INTERNAL_ERR, ret_v6 = QMI_INTERNAL_ERR;
  int err;
  qmi_ip_family_pref_type tmp_ip_pref;
  qmi_wds_call_end_reason_type  ce_reason_v4 =
  {
    QMI_WDS_CE_REASON_UNDEFINED,
    {QMI_WDS_CE_TYPE_INTERNAL, QMI_WDS_VERBOSE_CE_INTERNAL_ERROR}
  };
  qmi_wds_call_end_reason_type  ce_reason_v6 =
  {
    QMI_WDS_CE_REASON_UNDEFINED,
    {QMI_WDS_CE_TYPE_INTERNAL, QMI_WDS_VERBOSE_CE_INTERNAL_ERROR}
  };
  int              qmi_err_v4, qmi_err_v6;
  qdi_mode_t       mode = QDI_MODE_NOT_SET;
  dsi_addr_info_t  addr_info;
  ds_dll_el_t      *node = NULL;
  qdi_call_info_t  *call_info = (qdi_call_info_t *)call_hndl;
  qdi_handle_info_t* info = NULL;

  QDI_GET_INFO_PTR(user_handle, info);

  QDI_LOG_TRACE_ENTRY;
  QDI_L2S_ENTRY_PARAMS("dsi_hndl=0x%p", user_data);

  if (!QDI_IS_VALID_HNDL(info) || !call_info || !params || !user_cb || !call_end_reason_resp || !qmi_err_code)
  {
    QDI_LOG_ERROR("Invalid input param(s) user_handle=%d, info=%p, call_info=%p, "
                  "params=%p, user_cb=%p, ce_reason=%p, qmi_err_code=%p",
                  user_handle,
                  info,
                  call_info,
                  params,
                  user_cb,
                  call_end_reason_resp,
                  qmi_err_code);

    QDI_LOG_ERROR("Invalid input param(s)");
    QDI_LOG_TRACE_RETURN(QMI_INTERNAL_ERR);
    return QMI_INTERNAL_ERR;
  }

  /* Initialize call end reason to error */
  *call_end_reason_resp = ce_reason_v4;
  *qmi_err_code = QMI_INTERNAL_ERR;

  /* Determine the IP family of the call */
  if (params->params_mask & QMI_WDS_START_NW_IP_FAMILY_PREF_PARAM)
  {
    mode = qdi_translate_qmi_ip_family_pref(params->ip_family_pref);
  }
  else
  {
    QDI_LOG_HIGH("qdi_wds_start_nw_if: IP family not specified, creating new call_info");
    mode = QDI_MODE_V4V6;
  }

  /* Lock the transaction mutex */
  QDI_MUTEX_LOCK(qdi_txn_mutex);

  /* Update rl_qmi_inst id obatined from dsi route lookup */
    call_info->rl_qmi_inst = rl_qmi_inst;

  /* Validate the call attempt */
  if (QDI_SUCCESS != qdi_validate_call_attempt(user_handle,
                                               call_info,
                                               partial_retry,
                                               mode))
  {
    QDI_LOG_ERROR("call attempt validation failed");
    goto bail;
  }

  /* bind v4 handle to IPV4 family */
  ret_v4 = qmi_wds_set_client_ip_pref(info->handle_ipv4,
                                      QMI_WDS_IP_FAMILY_PREF_IPV4,
                                      qmi_err_code);

  if (QMI_NO_ERR == ret_v4)
  {
    QDI_LOG_LOW("Successfully bound IPv4 QMI WDS client to IPv4 family");
  }
  else if (QMI_SERVICE_ERR == ret_v4 && QMI_SERVICE_ERR_NO_EFFECT == *qmi_err_code)
  {
    QDI_LOG_LOW("IPv4 QMI WDS client already bound to IPv4 family");
  }
  else
  {
    QDI_LOG_ERROR(" binding v4 handle to IPV4 family failed rc=%d, qmi_err=%d",
                  ret_v4,
                  *qmi_err_code);
  }

  /* bind v6 handle to IPV6 family */
  ret_v6 = qmi_wds_set_client_ip_pref(info->handle_ipv6,
                                      QMI_WDS_IP_FAMILY_PREF_IPV6,
                                      qmi_err_code);

  if (QMI_NO_ERR == ret_v6)
  {
    QDI_LOG_LOW("Successfully bound IPv6 QMI WDS client to IPv6 family");
  }
  else if (QMI_SERVICE_ERR == ret_v6 && QMI_SERVICE_ERR_NO_EFFECT == *qmi_err_code)
  {
    QDI_LOG_LOW("IPv6 QMI WDS client already bound to IPv6 family");
  }
  else
  {
    QDI_LOG_ERROR(" binding v6 handle to IPV6 family failed rc=%d, qmi_err=%d",
                  ret_v6,
                  *qmi_err_code);
  }

  switch (mode)
  {
    case QDI_MODE_V4:
      QDI_LOG_MED("IPv4 call requested");

      call_info->mode = QDI_MODE_V4;
      call_info->start_if_txn.v4_cb_pend = TRUE;
      call_info->start_if_txn.v6_cb_pend = FALSE;
      break;

    case QDI_MODE_V6:
      QDI_LOG_MED("IPv6 call requested");

      call_info->mode = QDI_MODE_V6;
      call_info->start_if_txn.v4_cb_pend = FALSE;
      call_info->start_if_txn.v6_cb_pend = TRUE;
      break;

    case QDI_MODE_V4V6:
    default:
      /* Check if this is a Dual-IP partial retry scenario */
      if (call_info->v4_pkt_srvc_conn_ind_cnt != 0 &&
          call_info->v6_pkt_srvc_conn_ind_cnt != 0)
      {
        QDI_LOG_ERROR("Both addresses families are already up... "
                      "checking addresses, mode=%d",
                      call_info->mode);

        mode = QDI_MODE_NOT_SET;
        call_info->start_if_txn.v4_cb_pend = FALSE;
        call_info->start_if_txn.v6_cb_pend = FALSE;

        /* If we don't have a valid address for this interface (previous retry
           failure), try stopping and restarting the failed call */
        if (QDI_SUCCESS != qdi_get_addr_info(user_handle,
                                             info->dev_id,
                                             AF_INET,
                                             &addr_info,
                                             QMI_WDS_IFACE_NAME_NOT_REPORTED))
        {
          QDI_LOG_MED("Partial retry: invalid address... attempting to stop V4 call");

          if (QMI_NO_ERR != (ret_v4 = qmi_wds_stop_nw_if(info->handle_ipv4,
                                                         NULL,
                                                         NULL,
                                                         &qmi_err_v4)))
          {
            QDI_LOG_ERROR("Failed to stop V4 call rc=%d, err=%d",
                          ret_v4, qmi_err_v4);
          }

          QDI_LOG_MED("Partial retry... attempting V4 call, mode=%d",
                      call_info->mode);
          mode |= QDI_MODE_V4;
          call_info->start_if_txn.v4_cb_pend = TRUE;
          call_info->v4_pkt_srvc_conn_ind_cnt = 0;
        }
        else
        {
          QDI_LOG_MED("Valid IPV4 address found");
        }

        if (QDI_SUCCESS != qdi_get_addr_info(user_handle,
                                             info->dev_id,
                                             AF_INET6,
                                             &addr_info,
                                             QMI_WDS_IFACE_NAME_NOT_REPORTED))
        {
          QDI_LOG_MED("Partial retry: invalid address... attempting to stop V6 call");

          if (QMI_NO_ERR != (ret_v6 = qmi_wds_stop_nw_if(info->handle_ipv6,
                                                         NULL,
                                                         NULL,
                                                         &qmi_err_v6)))
          {
            QDI_LOG_ERROR("Failed to stop V6 call rc=%d, err=%d",
                          ret_v6, qmi_err_v6);
          }

          QDI_LOG_MED("Partial retry... attempting V6 call, mode=%d",
                      call_info->mode);
          mode |= QDI_MODE_V6;
          call_info->start_if_txn.v6_cb_pend = TRUE;
          call_info->v6_pkt_srvc_conn_ind_cnt = 0;
        }
        else
        {
          QDI_LOG_MED("Valid IPV6 address found");
        }

        /* If we are not retrying any of the address families, bail */
        if (call_info->v4_pkt_srvc_conn_ind_cnt != 0 &&
            call_info->v6_pkt_srvc_conn_ind_cnt != 0)
        {
          QDI_LOG_ERROR("Valid IPV4, IPV6 addresses found, returning call failure");
          goto bail;
        }
      }
      else if (call_info->v4_pkt_srvc_conn_ind_cnt != 0)
      {
        QDI_LOG_MED("Partial retry... attempting V6 call, mode=%d",
                    call_info->mode);
        mode = QDI_MODE_V6;
        call_info->start_if_txn.v4_cb_pend = FALSE;
        call_info->start_if_txn.v6_cb_pend = TRUE;
      }
      else if (call_info->v6_pkt_srvc_conn_ind_cnt != 0)
      {
        QDI_LOG_MED("Partial retry... attempting V4 call, mode=%d",
                    call_info->mode);
        mode = QDI_MODE_V4;
        call_info->start_if_txn.v4_cb_pend = TRUE;
        call_info->start_if_txn.v6_cb_pend = FALSE;
      }
      else
      {
        QDI_LOG_MED("IPv4v6 call requested");

        call_info->mode = QDI_MODE_V4V6;
        call_info->start_if_txn.v4_cb_pend = TRUE;
        call_info->start_if_txn.v6_cb_pend = TRUE;
      }

      break;
  }

  switch (mode)
  {
    case QDI_MODE_V4:
      ret_v4 = qmi_wds_start_nw_if(info->handle_ipv4,
                                   params,
                                   qdi_wds_async_cb_hdlr,
                                   (void*) call_info,
                                   &ce_reason_v4,
                                   &qmi_err_v4);

      if (ret_v4 < 0)
      {
        QDI_LOG_ERROR("Failed to start network interface using IPv4 "
                      "bound handle");
        call_info->start_if_txn.v4_cb_pend = FALSE;
        call_info->start_if_txn.txn_v4     = QDI_INVALID_TXN_HNDL;
        *call_end_reason_resp              = ce_reason_v4;
        *qmi_err_code                      = qmi_err_v4;
        QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "V4 SNI failed ce_reason=%d, qmi_err=%d",
                           ce_reason_v4, qmi_err_v4);
        goto bail;
      }
      else
      {
        QDI_LOG_LOW("qdi_wds_start_nw_if: txn_v4=0x%08x", ret_v4);
        call_info->start_if_txn.txn_v4 = ret_v4;
        QDI_L2S_MSG(call_info->rl_qmi_inst, "V4 SNI succeeded");
      }
      break;

    case QDI_MODE_V6:
      ret_v6 = qmi_wds_start_nw_if(info->handle_ipv6,
                                   params,
                                   qdi_wds_async_cb_hdlr,
                                   (void*) call_info,
                                   &ce_reason_v6,
                                   &qmi_err_v6);

      if (ret_v6 < 0)
      {
        QDI_LOG_ERROR("Failed to start network interface using IPv6 "
                      "bound handle");
        call_info->start_if_txn.v6_cb_pend = FALSE;
        call_info->start_if_txn.txn_v6     = QDI_INVALID_TXN_HNDL;
        *call_end_reason_resp              = ce_reason_v6;
        *qmi_err_code                      = qmi_err_v6;
        QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "V6 SNI failed ce_reason=%d, qmi_err=%d",
                           ce_reason_v6, qmi_err_v6);
        goto bail;
      }
      else
      {
        QDI_LOG_LOW("qdi_wds_start_nw_if: txn_v6=0x%08x", ret_v6);
        call_info->start_if_txn.txn_v6 = ret_v6;
        QDI_L2S_MSG(call_info->rl_qmi_inst, "V6 SNI succeeded");
      }
      break;

    /* For a V4V6 call check if either of the SNI calls is successful */
    case QDI_MODE_V4V6:
      tmp_ip_pref = params->ip_family_pref;

      params->ip_family_pref = QMI_IP_FAMILY_PREF_IPV4;
      ret_v4 = qmi_wds_start_nw_if(info->handle_ipv4,
                                   params,
                                   qdi_wds_async_cb_hdlr,
                                   (void*) call_info,
                                   &ce_reason_v4,
                                   &qmi_err_v4);

      if (ret_v4 < 0)
      {
        QDI_LOG_ERROR("Failed to start network interface using IPv4 "
                      "bound handle");
        call_info->start_if_txn.v4_cb_pend = FALSE;
        call_info->start_if_txn.txn_v4     = QDI_INVALID_TXN_HNDL;
        QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "V4 SNI failed ce_reason=%d, qmi_err=%d",
                           ce_reason_v4,qmi_err_v4);
      }
      else
      {
        QDI_LOG_LOW("qdi_wds_start_nw_if: txn_v4=0x%08x", ret_v4);
        call_info->start_if_txn.txn_v4 = ret_v4;
        QDI_L2S_MSG(call_info->rl_qmi_inst, "V4 SNI succeeded");
      }

      params->ip_family_pref = QMI_IP_FAMILY_PREF_IPV6;
      ret_v6 = qmi_wds_start_nw_if(info->handle_ipv6,
                                   params,
                                   qdi_wds_async_cb_hdlr,
                                   (void*) call_info,
                                   &ce_reason_v6,
                                   &qmi_err_v6);

      if (ret_v6 < 0)
      {
        QDI_LOG_ERROR("Failed to start network interface using IPv6 "
                      "bound handle");
        call_info->start_if_txn.v6_cb_pend = FALSE;
        call_info->start_if_txn.txn_v6     = QDI_INVALID_TXN_HNDL;
        QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "V6 SNI failed ce_reason=%d, qmi_err=%d",
                           ce_reason_v6,qmi_err_v6);
      }
      else
      {
        QDI_LOG_LOW("qdi_wds_start_nw_if: txn_v6=0x%08x", ret_v6);
        call_info->start_if_txn.txn_v6 = ret_v6;
        QDI_L2S_MSG(call_info->rl_qmi_inst, "V6 SNI succeeded");
      }

      params->ip_family_pref = tmp_ip_pref;

      /* If either of the calls is successful then return a SUCCESS, otherwise
         return the IPv4 error codes */
      if (ret_v4 >= 0)
      {
        *call_end_reason_resp = ce_reason_v4;
        *qmi_err_code         = qmi_err_v4;
      }
      else if (ret_v6 >= 0)
      {
        *call_end_reason_resp = ce_reason_v6;
        *qmi_err_code         = qmi_err_v6;
      }
      else
      {
        *call_end_reason_resp = ce_reason_v4;
        *qmi_err_code         = qmi_err_v4;
        goto bail;
      }
      break;

    default:
      QDI_LOG_ERROR("Invalid QDI mode=%d", call_info->mode);
      goto bail;
  }

  call_info->start_if_txn.cb_hdlr = user_cb;
  call_info->start_if_txn.user_data = user_data;
  ret = QDI_GET_TXN_ID(user_handle, QDI_TXN_START_IF);

bail:
  /* Unlock the transaction mutex */
  QDI_MUTEX_UNLOCK(qdi_txn_mutex);

  QDI_LOG_TRACE_RETURN(ret);
  QDI_L2S_EXIT_WITH_STATUS("dsi_hndl=0x%p", user_data);
  return ret;
}

/*===========================================================================
  FUNCTION  qdi_wds_stop_nw_if
===========================================================================*/
/*!
@brief
  This function is used to bring down a data call. The result of this call is
  reported asynchronously via the user_cb callback function.

@return
  If the return code < 0, the operation failed and there won't be an
  asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qdi_wds_abort() command.

@note
  - Dependencies
    - qdi_wds_srvc_init_client() must be called before calling this.
*/
/*=========================================================================*/
int
qdi_wds_stop_nw_if
(
  qdi_client_handle_t         user_handle,
  qdi_call_handle_t           call_hndl,
  qdi_wds_user_async_cb_type  user_cb,
  qdi_mode_t                  stop_mode,
  void                        *user_data,
  int                         *qmi_err_code
)
{
  int ret = QMI_INTERNAL_ERR;
  int ret_v4 = QMI_INTERNAL_ERR, ret_v6 = QMI_INTERNAL_ERR;
  int qmi_err_v4, qmi_err_v6;
  qdi_handle_info_t* info;
  qdi_call_info_t  *call_info = (qdi_call_info_t *)call_hndl;

  QDI_GET_INFO_PTR(user_handle, info);

  QDI_LOG_TRACE_ENTRY;
  QDI_L2S_ENTRY_PARAMS("dsi_hndl=0x%p", user_data);

  if (!QDI_IS_VALID_HNDL(info) || !call_info || !user_cb || !qmi_err_code)
  {
    QDI_LOG_ERROR("Invalid input param(s) user_handle=%d, info=%p, call_info=%p, "
                  "user_cb=%p, qmi_err_code=%p",
                  user_handle,
                  info,
                  call_info,
                  user_cb,
                  qmi_err_code);

    QDI_LOG_TRACE_RETURN(QMI_INTERNAL_ERR);
    return QMI_INTERNAL_ERR;
  }

  /* Lock the transaction mutex */
  QDI_MUTEX_LOCK(qdi_txn_mutex);

  /* Make sure that this is a valid call handle */
  if (call_info->handle != user_handle ||
      NULL == qdi_find_matching_call_info_node(info, call_info))
  {
    QDI_LOG_ERROR("qdi_wds_sopt_nw_if: unable to locate call_info=%p mode=0x%x",
                  call_info,
                  call_info->mode);
    goto bail;
  }

  if(QDI_MODE_NOT_SET == stop_mode)
  {
    stop_mode = call_info->mode;
  }

  switch (stop_mode)
  {
    case QDI_MODE_V4:
      call_info->stop_if_txn.v4_cb_pend = TRUE;
      call_info->stop_if_txn.v6_cb_pend = FALSE;

      ret_v4 = qmi_wds_stop_nw_if(info->handle_ipv4,
                                  qdi_wds_async_cb_hdlr,
                                  (void*) call_info,
                                  &qmi_err_v4);

      if (ret_v4 < 0)
      {
        QDI_LOG_ERROR("Failed to stop network interface using IPv4 "
                      "bound handle");
        call_info->stop_if_txn.v4_cb_pend = FALSE;
        call_info->stop_if_txn.txn_v4     = QDI_INVALID_TXN_HNDL;
        *qmi_err_code = qmi_err_v4;
        QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "Stopping V4 failed qmi_err=%d",
                           qmi_err_v4);
        goto bail;
      }
      else
      {
        QDI_LOG_LOW("qdi_wds_stop_nw_if: txn_v4=0x%08x", ret_v4);
        call_info->stop_if_txn.txn_v4 = ret_v4;
      }
      break;

    case QDI_MODE_V6:
      call_info->stop_if_txn.v4_cb_pend = FALSE;
      call_info->stop_if_txn.v6_cb_pend = TRUE;

      ret_v6 = qmi_wds_stop_nw_if(info->handle_ipv6,
                                  qdi_wds_async_cb_hdlr,
                                  (void*) call_info,
                                  &qmi_err_v6);

      if (ret_v6 < 0)
      {
        QDI_LOG_ERROR("Failed to stop network interface using IPv6 "
                      "bound handle");
        call_info->stop_if_txn.v6_cb_pend = FALSE;
        call_info->stop_if_txn.txn_v6     = QDI_INVALID_TXN_HNDL;
        *qmi_err_code = qmi_err_v6;
        QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "Stopping V6 failed qmi_err=%d",
                           qmi_err_v6);
        goto bail;
      }
      else
      {
        QDI_LOG_LOW("qdi_wds_stop_nw_if: txn_v6=0x%08x", ret_v6);
        call_info->stop_if_txn.txn_v6 = ret_v6;
      }
      break;

    case QDI_MODE_V4V6:
      call_info->stop_if_txn.v4_cb_pend = TRUE;
      call_info->stop_if_txn.v6_cb_pend = TRUE;

      ret_v4 = qmi_wds_stop_nw_if(info->handle_ipv4,
                                  qdi_wds_async_cb_hdlr,
                                  (void*) call_info,
                                  &qmi_err_v4);

      if (ret_v4 < 0)
      {
        QDI_LOG_ERROR("Failed to stop network interface using IPv4 "
                      "bound handle");
        call_info->stop_if_txn.v4_cb_pend = FALSE;
        call_info->stop_if_txn.txn_v4     = QDI_INVALID_TXN_HNDL;
        QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "Stopping V4 failed qmi_err=%d",
                           qmi_err_v4);
      }
      else
      {
        QDI_LOG_LOW("qdi_wds_stop_nw_if: txn_v4=0x%08x", ret_v4);
        call_info->stop_if_txn.txn_v4 = ret_v4;
      }

      ret_v6 = qmi_wds_stop_nw_if(info->handle_ipv6,
                                  qdi_wds_async_cb_hdlr,
                                  (void*) call_info,
                                  &qmi_err_v6);

      if (ret_v6 < 0)
      {
        QDI_LOG_ERROR("Failed to stop network interface using IPv6 "
                      "bound handle");
        call_info->stop_if_txn.v6_cb_pend = FALSE;
        call_info->stop_if_txn.txn_v6     = QDI_INVALID_TXN_HNDL;
        QDI_L2S_FORMAT_MSG(call_info->rl_qmi_inst, "Stopping V6 failed qmi_err=%d",
                           qmi_err_v6);
      }
      else
      {
        QDI_LOG_LOW("qdi_wds_stop_nw_if: txn_v6=0x%08x", ret_v6);
        call_info->stop_if_txn.txn_v6 = ret_v6;
      }

      /* If either of the calls is successful then return a SUCCESS, otherwise
         return the IPv4 error code */
      if (ret_v4 >= 0)
      {
        *qmi_err_code = qmi_err_v4;
      }
      else if (ret_v6 >= 0)
      {
        *qmi_err_code = qmi_err_v6;
      }
      else
      {
        *qmi_err_code = qmi_err_v4;
        goto bail;
      }
      break;

    default:
      QDI_LOG_ERROR("Invalid QDI mode=%d", call_info->mode);
      goto bail;
  }

  call_info->stop_if_txn.cb_hdlr = user_cb;
  call_info->stop_if_txn.user_data = user_data;
  ret = QDI_GET_TXN_ID(user_handle, QDI_TXN_STOP_IF);

bail:
  /* Unlock the transaction mutex */
  QDI_MUTEX_UNLOCK(qdi_txn_mutex);

  QDI_LOG_TRACE_RETURN(ret);
  QDI_L2S_EXIT_WITH_STATUS("dsi_hndl=0x%p", user_data);

  return ret;
}

/*===========================================================================
  FUNCTION  qdi_wds_abort
===========================================================================*/
/*!
@brief
  Aborts an asynchronous QDI operation. If the user_cb function pointer is
  set to NULL, then this function will be invoked synchronously, otherwise
  it will be invoked asynchronously.  The txn_handle parameter is the
  return code returned from any other asynchronous WDS call.  Note that
  synchronous API calls cannot be aborted.

@return
  0 if abort operation was sucessful, < 0 if not.  In the case where the
  abort is successful, an asychronous reply to the aborted command will NOT
  be returned, otherwise, it will.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error.  Note that asynchronous abort commands cannot
  themselves be aborted.

@note
  - Dependencies
    - qdi_wds_srvc_init_client() must be called before calling this.
*/
/*=========================================================================*/
int qdi_wds_abort
(
  qdi_client_handle_t         user_handle,
  qdi_call_handle_t           call_hndl,
  int                         txn_handle,
  qdi_wds_user_async_cb_type  user_cb,
  void                        *user_data,
  int                         *qmi_err_code
)
{
  int ret = QMI_INTERNAL_ERR;
  int ret_v4 = QMI_NO_ERR, ret_v6 = QMI_NO_ERR;
  int op;
  qdi_txn_info_t* txn = NULL;
  qdi_call_info_t  *call_info = (qdi_call_info_t *)call_hndl;

  qdi_handle_info_t* info;
  QDI_GET_INFO_PTR(user_handle, info);

  QDI_LOG_TRACE_ENTRY;

  if (!QDI_IS_VALID_HNDL(info) || !call_info)
  {
    QDI_LOG_ERROR("Invalid QDI handle info");
    return QMI_INTERNAL_ERR;
  }

  if (QDI_GET_HANDLE_FROM_TXN(txn_handle) != user_handle)
  {
    QDI_LOG_ERROR("Invalid txn_handle");
    return QMI_INTERNAL_ERR;
  }

  op = QDI_GET_OP_FROM_TXN(txn_handle);

  if (op < QDI_TXN_MIN || op >= QDI_TXN_MAX)
  {
    QDI_LOG_ERROR("Invalid operation from handle");
    return QMI_INTERNAL_ERR;
  }

  /* Lock the transaction mutex */
  QDI_MUTEX_LOCK(qdi_txn_mutex);

  /* Make sure that this is a valid call handle */
  if (call_info->handle != user_handle ||
      NULL == qdi_find_matching_call_info_node(info, call_info))
  {
    QDI_LOG_ERROR("qdi_wds_sopt_nw_if: unable to locate call_info=%p mode=0x%x",
                  call_info,
                  call_info->mode);
    goto bail;
  }

  /* Update the pending flags */
  switch (call_info->mode)
  {
    case QDI_MODE_V4:
      call_info->abort_txn.v4_cb_pend = TRUE;
      call_info->abort_txn.v6_cb_pend = FALSE;
      break;

    case QDI_MODE_V6:
      call_info->abort_txn.v4_cb_pend = FALSE;
      call_info->abort_txn.v6_cb_pend = TRUE;
      break;

    case QDI_MODE_V4V6:
      call_info->abort_txn.v4_cb_pend = TRUE;
      call_info->abort_txn.v6_cb_pend = TRUE;
      break;

    default:
      QDI_LOG_ERROR("Invalid QDI mode=%d", call_info->mode);
      goto bail;
  }

  /* Determine the transaction to abort */
  switch(op)
  {
    case QDI_TXN_START_IF:
      txn = &call_info->start_if_txn;
      break;

    case QDI_TXN_STOP_IF:
      txn = &call_info->stop_if_txn;
      break;

    case QDI_TXN_RESET:
      /* Reset is not handled as there is no QDI reset API. Intentional
         fallthrough */
    default:
      QDI_LOG_ERROR("Invalid operation=%d", op);
      goto bail;
  }

  if (txn->txn_v4 != QDI_INVALID_TXN_HNDL)
  {
    ret_v4 = qmi_wds_abort(info->handle_ipv4,
                           txn->txn_v4,
                           user_cb ? qdi_wds_async_cb_hdlr : NULL,
                           (void*) call_info,
                           qmi_err_code);

    if (ret_v4 < 0)
    {
      QDI_LOG_ERROR("Failed to abort IPv4 [0x%8x] txn with ret=%d, err=%d",
                    txn->txn_v4, ret_v4, *qmi_err_code);

      call_info->abort_txn.v4_cb_pend = FALSE;

      /* We could get into a situation where for a Dual-IP call one of the
         address families would come up while the other wouldn't. The abort
         would fail for the family that has already come up. Attempt to stop
         the call in that scenario */
      if (QMI_SERVICE_ERR_INVALID_TX_ID == *qmi_err_code)
      {
        QDI_LOG_MED("Invalid TX Err recvd... Attempting to stop the V4 call");

        ret_v4 = qmi_wds_stop_nw_if(info->handle_ipv4,
                                    NULL,
                                    NULL,
                                    qmi_err_code);

        if (ret_v4 < 0)
        {
          QDI_LOG_ERROR("Failed to stop network interface using IPv4 "
                        "bound handle");
        }
      }
    }
    else
    {
      /* clear the aborted transaction handle */
      txn->txn_v4 = QDI_INVALID_TXN_HNDL;
      call_info->abort_txn.txn_v4 = ret_v4;
    }
  }
  else if (QDI_MODE_V4V6 == call_info->mode)
  {
    QDI_LOG_MED("Dual-IP call with invalid TXN... Attempting to stop the V4 call");

    ret_v4 = qmi_wds_stop_nw_if(info->handle_ipv4,
                                NULL,
                                NULL,
                                qmi_err_code);

    if (ret_v4 < 0)
    {
      QDI_LOG_ERROR("Failed to stop network interface using IPv4 "
                    "bound handle");
    }
  }

  if (txn->txn_v6 != QDI_INVALID_TXN_HNDL)
  {
    ret_v6 = qmi_wds_abort(info->handle_ipv6,
                           txn->txn_v6,
                           user_cb ? qdi_wds_async_cb_hdlr : NULL,
                           (void*) call_info,
                           qmi_err_code);

    if (ret_v6 < 0)
    {
      QDI_LOG_ERROR("Failed to abort IPv6 [0x%8x] txn with ret=%d, err=%d",
                    txn->txn_v6, ret_v6, *qmi_err_code);

      call_info->abort_txn.v6_cb_pend = FALSE;

      /* We could get into a situation where for a Dual-IP call one of the
         address families would come up while the other wouldn't. The abort
         would fail for the family that has already come up. Attempt to stop
         the call in that scenario */
      if (QMI_SERVICE_ERR_INVALID_TX_ID == *qmi_err_code)
      {
        QDI_LOG_MED("Invalid TX Err recvd... Attempting to stop the V6 call");

        ret_v6 = qmi_wds_stop_nw_if(info->handle_ipv6,
                                    NULL,
                                    NULL,
                                    qmi_err_code);

        if (ret_v6 < 0)
        {
          QDI_LOG_ERROR("Failed to stop network interface using IPv6 "
                        "bound handle");
        }
      }
    }
    else
    {
      /* clear the aborted transaction handle */
      txn->txn_v6 = QDI_INVALID_TXN_HNDL;
      call_info->abort_txn.txn_v6 = ret_v6;
    }
  }
  else if (QDI_MODE_V4V6 == call_info->mode)
  {
    QDI_LOG_MED("Dual-IP call with invalid TXN... Attempting to stop the V6 call");

    ret_v6 = qmi_wds_stop_nw_if(info->handle_ipv6,
                                NULL,
                                NULL,
                                qmi_err_code);

    if (ret_v6 < 0)
    {
      QDI_LOG_ERROR("Failed to stop network interface using IPv6 "
                    "bound handle");
    }
  }

  /* If one of the aborts was successful return a SUCCESS */
  if (ret_v4 >= 0 || ret_v6 >= 0)
  {
    call_info->abort_txn.cb_hdlr   = user_cb;
    call_info->abort_txn.user_data = user_data;
    ret                            = QMI_NO_ERR;
    *qmi_err_code                  = QMI_SERVICE_ERR_NONE;
  }

bail:
  /* Unlock the transaction mutex */
  QDI_MUTEX_UNLOCK(qdi_txn_mutex);

  QDI_LOG_TRACE_RETURN(ret);
  return ret;
}

/*===========================================================================
  FUNCTION  qdi_get_qmi_wds_handle
===========================================================================*/
/*!
@brief
  This function returns the QMI handle corresponding to the QDI handle

@return
  QMI_INTERNAL_ERR on error or the corresponding QMI handle

@note
  - Dependencies
    - qdi_wds_srvc_init_client() must be called before calling this.
*/
/*=========================================================================*/
int
qdi_get_qmi_wds_handle
(
  qdi_client_handle_t user_handle
)
{
  qdi_handle_info_t       *info = NULL;
  qmi_client_handle_type  qmi_wds_hndl;
  ds_dll_el_t             *node = NULL;
  const void              *dummy = NULL;

  QDI_GET_INFO_PTR(user_handle, info);

  QDI_LOG_TRACE_ENTRY;

  if (!QDI_IS_VALID_HNDL(info))
  {
    QDI_LOG_TRACE_RETURN(QMI_INTERNAL_ERR);
    return QMI_INTERNAL_ERR;
  }

  /* Lock the transaction mutex */
  QDI_MUTEX_LOCK(qdi_txn_mutex);

  /* Default to the IPv4 handle */
  qmi_wds_hndl = info->handle_ipv4;

  /* Check if we have an active call */
  node = ds_dll_next(info->call_info_head, &dummy);

  if (NULL != node)
  {
    qdi_call_info_t  *call_info = (qdi_call_info_t *) ds_dll_data(node);

    /* If this is a V6 only call then use the V6 QMI WDS handle */
    if (NULL != call_info                        &&
        0 == call_info->v4_pkt_srvc_conn_ind_cnt &&
        0 != call_info->v6_pkt_srvc_conn_ind_cnt)
    {
      qmi_wds_hndl = info->handle_ipv6;
    }
  }

  /* Unlock the transaction mutex */
  QDI_MUTEX_UNLOCK(qdi_txn_mutex);

  QDI_LOG_TRACE_RETURN_HEX(qmi_wds_hndl);
  return qmi_wds_hndl;
}

/*==========================================================================
  FUNCTION: qdi_get_qmi_wds_handle_for_ip
===========================================================================*/
/*!
  @brief
  Returns the QMI WDS handle corresponding to the QDI handle based on IP
  type

 @return
  QMI_INTERNAL_ERR on error or the corresponding QMI handle

 @note
  - Dependencies
    - qdi_wds_srvc_init_client() must be called before calling this.
*/
/*=========================================================================*/
int
qdi_get_qmi_wds_handle_for_ip
(
  qdi_client_handle_t     user_handle,
  qmi_ip_family_pref_type ip_type
)
{
  qdi_handle_info_t       *info = NULL;
  qmi_client_handle_type  qmi_wds_hndl;

  QDI_GET_INFO_PTR(user_handle, info);

  QDI_LOG_TRACE_ENTRY;

  if (!QDI_IS_VALID_HNDL(info))
  {
    QDI_LOG_TRACE_RETURN(QMI_INTERNAL_ERR);
    return QMI_INTERNAL_ERR;
  }

  /* Lock the transaction mutex */
  QDI_MUTEX_LOCK(qdi_txn_mutex);

  /* Check which WDS handle is requested */
  switch (ip_type)
  {
  case QMI_IP_FAMILY_PREF_IPV4:
    qmi_wds_hndl = info->handle_ipv4;
    break;
  case QMI_IP_FAMILY_PREF_IPV6:
    qmi_wds_hndl = info->handle_ipv6;
    break;
  default:
    qmi_wds_hndl = info->handle_ipv4;
    break;
  }

  /* Unlock the transaction mutex */
  QDI_MUTEX_UNLOCK(qdi_txn_mutex);

  QDI_LOG_TRACE_RETURN_HEX(qmi_wds_hndl);
  return qmi_wds_hndl;
}

/*===========================================================================
  FUNCTION  qdi_get_addr_info
===========================================================================*/
/*!
@brief
  This function returns the IP address of the given IP family for the given
  interface

@param
  user_handle - Interface on which IP address is being requested
  ifname      - IP family of the address to return
  ipfamily    - AF_INET or AF_INET6

@return
  QDI_SUCCESS on successful operation
  QDI_FAILURE otherwise

@note
  - Dependencies
    - qdi_wds_srvc_init_client() must be called before calling this.
*/
/*=========================================================================*/
int
qdi_get_addr_info
(
  qdi_client_handle_t     user_handle,
  const char              *ifname,
  int                     ip_family,
  dsi_addr_info_t         *addr_info,
  qmi_wds_iface_name_type tech_name

)
{
  int                          ret = QDI_FAILURE;
  qdi_handle_info_t            *info = NULL;
  int                          handle = QMI_INVALID_CLIENT_HANDLE;
  qmi_wds_ip_family_pref_type  ip_pref = QMI_WDS_IP_FAMILY_PREF_UNSPECIFIED;
  int                          qmi_err_code;
  addr_data_t                  addr_data;

  QDI_GET_INFO_PTR(user_handle, info);

  QDI_LOG_TRACE_ENTRY;

  if (!QDI_IS_VALID_HNDL(info) || NULL == ifname || NULL == addr_info)
  {
    QDI_LOG_ERROR("qdi_get_addr_info: Bad param(s)");
    goto bail;
  }

  /* Obtain the IP address information via netlink from the kernel */
  if (QDI_SUCCESS != qdi_nl_get_ip_addr_info(ifname, ip_family, addr_info, tech_name))
  {
    QDI_LOG_ERROR("qdi_get_addr_info: failed to get IP address from kernel "
                  "ifname=%s, ip_family=%s\n",
                  ifname,
                  DSI_GET_IP_FAMILY(ip_family));
    goto bail;
  }

  handle = (AF_INET == ip_family) ? info->handle_ipv4 : info->handle_ipv6;

  /* Obtain the Gateway and DNS information from the modem */
  if (QDI_SUCCESS != qdi_get_modem_link_info(handle, ip_family, addr_info))
  {
    QDI_LOG_ERROR("qdi_get_addr_info: failed to get info from modem "
                  "for ifname=%s trying ip_family=%s\n",
                  ifname,
                  DSI_GET_IP_FAMILY(ip_family));
    goto bail;
  }

  /* Print the interface addresses being returned */
  if (AF_INET == ip_family)
  {
    addr_data.data = &(SASTORAGE_DATA(addr_info->iface_addr_s.addr));
    DS_INET4_NTOP(med,"Interface",   ((unsigned char*) addr_data.data_u32));
    addr_data.data = &(SASTORAGE_DATA(addr_info->gtwy_addr_s.addr));
    DS_INET4_NTOP(med,"Gateway",     ((unsigned char*)addr_data.data_u32));
    addr_data.data = &(SASTORAGE_DATA(addr_info->dnsp_addr_s.addr));
    DS_INET4_NTOP(med,"DNS Primary", ((unsigned char*) addr_data.data_u32));
    addr_data.data = &(SASTORAGE_DATA(addr_info->dnss_addr_s.addr));
    DS_INET4_NTOP(med,"DNS Secondary",((unsigned char*)addr_data.data_u32));
  }
  else
  {
    addr_data.data = &(SASTORAGE_DATA(addr_info->iface_addr_s.addr));
    DS_INET6_NTOP(med,"Interface",    ((unsigned char*)addr_data.data_u64));
    addr_data.data = &(SASTORAGE_DATA(addr_info->gtwy_addr_s.addr));
    DS_INET6_NTOP(med,"Gateway",      ((unsigned char*)addr_data.data_u64));
    addr_data.data = &(SASTORAGE_DATA(addr_info->dnsp_addr_s.addr));
    DS_INET6_NTOP(med,"DNS Primary",  ((unsigned char*)addr_data.data_u64));
    addr_data.data = &(SASTORAGE_DATA(addr_info->dnss_addr_s.addr));
    DS_INET6_NTOP(med,"DNS Secondary",((unsigned char*)addr_data.data_u64));
  }

  ret = QDI_SUCCESS;

bail:
  QDI_LOG_TRACE_RETURN(ret);
  return ret;
}

