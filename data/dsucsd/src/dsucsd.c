/******************************************************************************

                             D S U C S D . C

******************************************************************************/

/******************************************************************************
  @file    dsucsd.c
  @brief   dsucsd definitions

  DESCRIPTION
  Implementation of dsucsd APIs

******************************************************************************/
/*===========================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
11/11/11   sg         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <pthread.h>
#include <cutils/properties.h>
#include <private/android_filesystem_config.h>
#include "comdef.h"
#include "dsucsd_debug.h"
#include "dsucsdappif_apis.h"
#include "qmi_csvt_srvc.h"
#include "ds_cmdq.h"


/*===========================================================================
                          LOCAL DEFINITIONS
===========================================================================*/

/* Max number of DSUCSD clients supported */
#define DS_UCSD_API_MAX_CLIENTS  10
#define DS_UCSD_CMDQ_MAX_CMDS    10

#define DS_UCSD_CLIENT_ID_TO_INST_ID(x)   (x+1)

#define DS_UCSD_MSM_QMI_CONN_ID           QMI_PORT_RMNET_0
#define DS_UCSD_DEFAULT_QMI_CONN_ID       DS_UCSD_MSM_QMI_CONN_ID
#define DS_UCSD_MDMUSB_QMI_CONN_ID        QMI_PORT_RMNET_USB_0

#define DS_UCSD_INVALID_DATA_PORT         NULL
#define DS_UCSD_MSM_DATA_PORT             "/dev/smd11"
#define DS_UCSD_MDMUSB_DATA_PORT          "/dev/ttyUSB1"

#define DS_UCSD_PROPERTY_BASEBAND         "ro.baseband"
#define DS_UCSD_PROPERTY_BASEBAND_SIZE    10
#define DS_UCSD_BASEBAND_VALUE_MDMUSB     "mdm"
#define DS_UCSD_BASEBAND_VALUE_MSM        "msm"
#define DS_UCSD_BASEBAND_VALUE_UNDEFINED  "undefined"

/* Structure to store the client information */
typedef struct
{
  boolean                     is_valid;
  qmi_csvt_clnt_hndl          csvt_hndl;
  qmi_csvt_instance_id_type   instance_id;
  ds_ucsd_call_event_cb_type  client_evt_cb;
  void                        *client_data;
} ds_ucsd_client_info_type;

static ds_ucsd_client_info_type  ds_ucsd_client_info_tbl[DS_UCSD_API_MAX_CLIENTS] =
{
/* is_valid       csvt_hndl              instance_id     client_evt_cb  client_data */
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL},
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL},
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL},
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL},
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL},
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL},
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL},
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL},
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL},
  {FALSE,   QMI_CSVT_INVALID_HNDL, QMI_CSVT_INVALID_INST_ID,  NULL,       NULL}
};

/* Typedef for bridgemgr command type */
typedef struct
{
  ds_cmd_t                ds_cmd;   /* Should be the first entry in the structure */
  qmi_csvt_ind_data_type  ind_data;
  void                    *user_data;
} ds_ucsd_cmdq_cmd_type;

/* Mutex to protect the ds_ucsd_client_info_tbl[] */
static pthread_mutex_t  ds_ucsd_client_info_mutex = PTHREAD_MUTEX_INITIALIZER;

static int ds_ucsd_qmi_hndl = QMI_INVALID_CLIENT_HANDLE;

static struct ds_cmdq_info_s ds_ucsd_cmdq;

static char ds_ucsd_baseband[DS_UCSD_PROPERTY_BASEBAND_SIZE] = DS_UCSD_BASEBAND_VALUE_MSM;

typedef struct
{
  const char *baseband;
  const char *conn_id;
  const char *data_port;
} ds_ucsd_baseband_port_map_type;

#define DS_UCSD_NUM_PORT_ENTRIES  \
  (sizeof(ds_ucsd_port_tbl)/sizeof(ds_ucsd_port_tbl[0]))

static ds_ucsd_baseband_port_map_type ds_ucsd_port_tbl[] =
{
  { DS_UCSD_BASEBAND_VALUE_MSM,    DS_UCSD_MSM_QMI_CONN_ID,    DS_UCSD_MSM_DATA_PORT    },
  { DS_UCSD_BASEBAND_VALUE_MDMUSB, DS_UCSD_MDMUSB_QMI_CONN_ID, DS_UCSD_MDMUSB_DATA_PORT }
};


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION ds_ucsd_api_get_active_client_count
===========================================================================*/
/*!
  Utility function for determining the number of current active clients

  @param
  None

  @return
  Number of currently active clients

  @note
  ds_ucsd_client_info_mutex must be locked before calling this function
*/
/*=========================================================================*/
static uint32
ds_ucsd_api_get_active_client_count(void)
{
  uint32  num_active_clients = 0;
  int     i;


  /* Determine the number of currently active clients */
  for (i = 0; i < DS_UCSD_API_MAX_CLIENTS; ++i)
  {
    if (ds_ucsd_client_info_tbl[i].is_valid)
    {
      ++num_active_clients;
    }
  }

  return num_active_clients;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_get_qmi_conn_id
===========================================================================*/
/*!
  Utility function for determining the QMI connection ID to use for a CSVT
  call based on an Android property (ro.baseband).

  @param
  None

  @return
  QMI connection ID to use

  @note
  ds_ucsd_client_info_mutex must be locked before calling this function
*/
/*=========================================================================*/
static const char *
ds_ucsd_api_get_qmi_conn_id(void)
{
  const char *conn_id = DS_UCSD_DEFAULT_QMI_CONN_ID;
  boolean ret = FALSE;
  int i;

  ds_ucsd_log_med("ds_ucsd_api_get_qmi_conn_port: baseband property is set to [%s]",
                  ds_ucsd_baseband);

  /* Loop through the available connection ID entries */
  for (i = 0; i < (int)DS_UCSD_NUM_PORT_ENTRIES; ++i)
  {
    /* If a matching baseband is found */
    if (!strcmp(ds_ucsd_baseband, ds_ucsd_port_tbl[i].baseband))
    {
      conn_id = ds_ucsd_port_tbl[i].conn_id;
      ret = TRUE;
      break;
    }
  }

  if (TRUE == ret)
  {
    ds_ucsd_log_med("ds_ucsd_api_get_qmi_conn_id: baseband match found returning conn_id [%s]",
                    conn_id);
  }
  else
  {
    ds_ucsd_log_err("ds_ucsd_api_get_qmi_conn_id: baseband match not found");
  }

  return conn_id;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_ind_to_str
===========================================================================*/
/*!
  Utility function to convert the given indication to a string

  @param ind [in]   Indication to convert to string

  @return
  String representation of a known indication or "UNKNOWN"
*/
/*=========================================================================*/
static const char *
ds_ucsd_api_ind_to_str
(
  qmi_csvt_ind_type  ind
)
{
  static const char *ind_str[] =
  {
    "QMI_CSVT_IND_TYPE_UNKNOWN",
    "QMI_CSVT_IND_TYPE_CALL_CONFIRM",
    "QMI_CSVT_IND_TYPE_CALL_PROGRESS",
    "QMI_CSVT_IND_TYPE_CALL_CONNECT",
    "QMI_CSVT_IND_TYPE_CALL_SETUP",
    "QMI_CSVT_IND_TYPE_INCOMING_CALL",
    "QMI_CSVT_IND_TYPE_CALL_END",
    "QMI_CSVT_IND_TYPE_CALL_MODIFY"
  };

  return ((ind <= QMI_CSVT_IND_TYPE_CALL_MODIFY) ? ind_str[ind] : ind_str[0]);
}


/*===========================================================================
  FUNCTION ds_ucsd_api_process_call_confirm_ind
===========================================================================*/
/*!
  Processing function for call confirm indication

  @param ind_data  [in]   Data associated with the QMI CSVT indication

  @param conf_data [out]  dsucsd confirmation event data to update

  @return
  None
*/
/*=========================================================================*/
static void
ds_ucsd_api_process_call_confirm_ind
(
  uint8                           instance_id,
  qmi_csvt_ind_data_type          *ind_data,
  ds_ucsd_call_confirm_info_type  *conf_data
)
{
  if (!ind_data || !conf_data)
  {
    ds_ucsd_log_err("ds_ucsd_api_process_call_confirm_ind: bad input parameter(s)");
    return;
  }

  /* Set the mandatory instance_id value */
  conf_data->inst_id = instance_id;

  /* Set the optional values if present */
  /* Call Type */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_TYPE)
  {
    conf_data->call_type = (ds_ucsd_call_type) ind_data->opt.call_type;
  }
  else
  {
    conf_data->call_type = DS_UCSD_UNKNOWN_CALL;
  }

  /* Modify Allowed */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_MODIFY_ALLOWED)
  {
    conf_data->modify_allowed = ind_data->opt.is_modify_allowed;
  }
  else
  {
    conf_data->modify_allowed = FALSE;
  }
}


/*===========================================================================
  FUNCTION ds_ucsd_api_process_call_progress_ind
===========================================================================*/
/*!
  Processing function for call progress indication

  @param ind_data [in]    Data associated with the QMI CSVT indication

  @param prog_data [out]  dsucsd progress event data to update

  @return
  None
*/
/*=========================================================================*/
static void
ds_ucsd_api_process_call_progress_ind
(
  uint8                            instance_id,
  qmi_csvt_ind_data_type           *ind_data,
  ds_ucsd_call_progress_info_type  *prog_data,
  ds_ucsd_datapath_type            *datapath
)
{
  if (!ind_data || !prog_data || !datapath)
  {
    ds_ucsd_log_err("ds_ucsd_api_process_call_progress_ind: bad input parameter(s)");
    return;
  }

  /* Set the mandatory instance_id value */
  prog_data->inst_id = instance_id;

  /* Set the optional values if present */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_TYPE)
  {
    prog_data->call_type = (ds_ucsd_call_type) ind_data->opt.call_type;
  }
  else
  {
    prog_data->call_type = DS_UCSD_UNKNOWN_CALL;
  }

  /* FIXME: Support prog_data->progress_info? */

  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_PORT_DATA)
  {
    datapath->mode = DS_UCSD_DPATH_MODE_SIO;
    datapath->info.sio_info.sio_port = (sio_port_id_type) DEVICE_ID(ind_data->opt.port_data.port_type,
                                                                    ind_data->opt.port_data.port_num);
  }

  prog_data->datapath = datapath;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_process_call_connect_ind
===========================================================================*/
/*!
  Processing function for call connect indication

  @param ind_data [in]    Data associated with the QMI CSVT indication

  @param conn_data [out]  dsucsd event data to update

  @return
  None
*/
/*=========================================================================*/
static void
ds_ucsd_api_process_call_connect_ind
(
  uint8                           instance_id,
  qmi_csvt_ind_data_type          *ind_data,
  ds_ucsd_call_connect_info_type  *conn_data,
  ds_ucsd_datapath_type           *datapath
)
{
  if (!ind_data || !conn_data || !datapath)
  {
    ds_ucsd_log_err("ds_ucsd_api_process_call_connect_ind: bad input parameter(s)");
    return;
  }

  /* Set the mandatory instance_id value */
  conn_data->inst_id = instance_id;

  /* Set the optional values if present */
  /* Call Type */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_TYPE)
  {
    conn_data->call_type = (ds_ucsd_call_type) ind_data->opt.call_type;
  }
  else
  {
    conn_data->call_type = DS_UCSD_UNKNOWN_CALL;
  }

  /* Modify Allowed */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_MODIFY_ALLOWED)
  {
    conn_data->modify_allowed = ind_data->opt.is_modify_allowed;
  }
  else
  {
    conn_data->modify_allowed = FALSE;
  }

  /* Update datapath */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_PORT_DATA)
  {
    datapath->mode = DS_UCSD_DPATH_MODE_SIO;
    datapath->info.sio_info.sio_port = (sio_port_id_type) DEVICE_ID(ind_data->opt.port_data.port_type,
                                                                    ind_data->opt.port_data.port_num);
  }

  conn_data->datapath = datapath;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_process_call_setup_ind
===========================================================================*/
/*!
  Processing function for call setup indication

  @param ind_data [in]     Data associated with the QMI CSVT indication

  @param setup_data [out]  dsucsd event data to update

  @return
  None
*/
/*=========================================================================*/
static void
ds_ucsd_api_process_call_setup_ind
(
  uint8                         instance_id,
  qmi_csvt_ind_data_type        *ind_data,
  ds_ucsd_call_setup_info_type  *setup_data
)
{
  if (!ind_data || !setup_data)
  {
    ds_ucsd_log_err("ds_ucsd_api_process_call_setup_ind: bad input parameter(s)");
    return;
  }

  /* Set the mandatory instance_id value */
  setup_data->inst_id = instance_id;

  /* Set the optional values if present */
  /* Call Type */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_TYPE)
  {
    setup_data->call_type = (ds_ucsd_call_type) ind_data->opt.call_type;
  }
  else
  {
    setup_data->call_type = DS_UCSD_UNKNOWN_CALL;
  }

  /* Modify Allowed */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_MODIFY_ALLOWED)
  {
    setup_data->modify_allowed = ind_data->opt.is_modify_allowed;
  }
  else
  {
    setup_data->modify_allowed = FALSE;
  }
}


/*===========================================================================
  FUNCTION ds_ucsd_api_process_incoming_call_ind
===========================================================================*/
/*!
  Processing function for incoming call indication

  @param ind_data [in]     Data associated with the QMI CSVT indication

  @param incom_data [out]  dsucsd event data to update

  @return
  None
*/
/*=========================================================================*/
static void
ds_ucsd_api_process_incoming_call_ind
(
  uint8                            instance_id,
  qmi_csvt_ind_data_type           *ind_data,
  ds_ucsd_call_incoming_info_type  *incom_data
)
{
  if (!ind_data || !incom_data)
  {
    ds_ucsd_log_err("ds_ucsd_api_process_incoming_call_ind: bad input parameter(s)");
    return;
  }

  /* Set the mandatory instance_id value */
  incom_data->inst_id = instance_id;

  /* Set the optional values if present */
  /* Network Mode */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_TYPE)
  {
    incom_data->network_mode = (sys_sys_mode_e_type) ind_data->opt.network_type;
  }
  else
  {
    incom_data->network_mode = SYS_SYS_MODE_NONE;
  }

  /* Network Speed */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_SPEED)
  {
    incom_data->speed = (uint8) ind_data->opt.network_speed;
  }
  else
  {
    incom_data->speed = 0;
  }

  /* Synchronous Call */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_IS_SYNC_CALL)
  {
    incom_data->name = (uint8) ind_data->opt.is_call_synchronous;
  }
  else
  {
    incom_data->name = 0;
  }

  /* Transparent Call */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_IS_TRANS_CALL)
  {
    incom_data->connection_element = (uint8) !ind_data->opt.is_call_transparent;
  }
  else
  {
    incom_data->connection_element = 0;
  }

  /* Max Frame Size */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_MAX_FRAME_SIZE)
  {
    incom_data->waiur = (uint8) ind_data->opt.max_frame_size;
  }
  else
  {
    incom_data->waiur = 0;
  }

  /* UUS ID */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_UUS_ID)
  {
    incom_data->uus_data = (uint8 *) ind_data->opt.uus_id.uus_id;
    incom_data->uus_data_len = (uint8) strlen(ind_data->opt.uus_id.uus_id);
  }
  else
  {
    incom_data->uus_data = NULL;
    incom_data->uus_data_len = 0;
  }

  /* Incoming number */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_UUS_ID)
  {
    incom_data->caller_number = (uint8 *) ind_data->opt.incoming_num;
    incom_data->caller_number_len = (uint8) strlen(ind_data->opt.incoming_num);
  }
  else
  {
    incom_data->caller_number = NULL;
    incom_data->caller_number_len = 0;
  }
}


/*===========================================================================
  FUNCTION ds_ucsd_api_process_call_end_ind
===========================================================================*/
/*!
  Processing function for call end indication

  @param ind_data [in]   Data associated with the QMI CSVT indication

  @param end_data [out]  dsucsd event data to update

  @return
  None
*/
/*=========================================================================*/
static void
ds_ucsd_api_process_call_end_ind
(
  uint8                        instance_id,
  qmi_csvt_ind_data_type       *ind_data,
  ds_ucsd_call_end_info_type   *end_data,
  ds_ucsd_call_end_param_type  *ce_param
)
{
  if (!ind_data || !end_data || !ce_param)
  {
    ds_ucsd_log_err("ds_ucsd_api_process_call_end_ind: bad input parameter(s)");
    return;
  }

  /* Set the mandatory instance_id value */
  end_data->inst_id = instance_id;

  /* Set the optional values if present */
  /* Call end param */
  ce_param->end_cause = CM_CALL_END_NONE;

  /* Store the more specific call end reason */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_END_CAUSE)
  {
    ce_param->cc_param.cc_cause_present = TRUE;
    ce_param->cc_param.cc_cause = ind_data->opt.call_end_cause;
  }

  /* Don't need to support rej_param */
  ce_param->rej_param.present = FALSE;

  end_data->end_param = ce_param;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_process_call_modify_ind
===========================================================================*/
/*!
  Processing function for call modify indication

  @param ind_data [in]   Data associated with the QMI CSVT indication

  @param mod_data [out]  dsucsd event data to update

  @return
  None
*/
/*=========================================================================*/
static void
ds_ucsd_api_process_call_modify_ind
(
  uint8                          instance_id,
  qmi_csvt_ind_data_type         *ind_data,
  ds_ucsd_call_modify_info_type  *mod_data,
  ds_ucsd_datapath_type          *datapath
)
{
  if (!ind_data || !mod_data || !datapath)
  {
    ds_ucsd_log_err("ds_ucsd_api_process_call_modify_ind: bad input parameter(s)");
    return;
  }

  /* Set the mandatory instance_id value */
  mod_data->inst_id = instance_id;

  /* Set the optional values if present */
  /* Call Type*/
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_TYPE)
  {
    mod_data->call_type = (ds_ucsd_call_type) ind_data->opt.call_type;
  }
  else
  {
    mod_data->call_type = DS_UCSD_UNKNOWN_CALL;
  }

  /* Don't need to support this, only used for SCUDIF */
  mod_data->status = FALSE;

  /* Don't need to support this, only used for SCUDIF */
  mod_data->nw_initiated = FALSE;

  /* Don't need to support this, only used for SCUDIF */
  mod_data->rej_cause = 0;

  /* Update datapath */
  if (ind_data->opt.param_mask & QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_PORT_DATA)
  {
    datapath->mode = DS_UCSD_DPATH_MODE_SIO;
    datapath->info.sio_info.sio_port = (sio_port_id_type) DEVICE_ID(ind_data->opt.port_data.port_type,
                                                                    ind_data->opt.port_data.port_num);
  }

  mod_data->datapath = datapath;
}


/*===========================================================================
  FUNCTION  ds_ucsd_process_csvt_ind
===========================================================================*/
/*!
@brief
 Callback function registered with the Command Thread to process a command

@param
  ds_cmd - Command to process
  data   - Command data

@return
  void

*/
/*=========================================================================*/
static void ds_ucsd_process_csvt_ind
(
  ds_cmd_t  *ds_cmd,
  void      *data
)
{
  ds_ucsd_call_event_type      evt_data;
  ds_ucsd_client_info_type     *info = NULL;
  ds_ucsd_datapath_type        datapath;
  ds_ucsd_call_end_param_type  ce_param;
  int                          client_id;
  uint8                        instance_id;
  ds_ucsd_cmdq_cmd_type        *cmd = (ds_ucsd_cmdq_cmd_type *)ds_cmd;


  DSUCSD_LOG_TRACE_ENTRY;

  if (NULL == cmd)
  {
    ds_ucsd_log_err("ds_ucsd_process_csvt_ind: bad params\n");
    goto bail;
  }

  client_id = (int) cmd->user_data;

  if (client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_process_csvt_ind: invalid client_id=%d", client_id);
    goto bail;
  }

  ds_ucsd_log_med("ds_ucsd_process_csvt_ind: received ind=%s, client_id=%d, inst_id=0x%x",
                  ds_ucsd_api_ind_to_str(cmd->ind_data.ind_type),
                  client_id,
                  (unsigned int)cmd->ind_data.instance_id);

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_process_csvt_ind: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }

  /* If this is a network initiated indication, update the instance_id */
  if (QMI_CSVT_INVALID_INST_ID == info->instance_id)
  {
    ds_ucsd_log_med("ds_ucsd_process_csvt_ind: tlb inst_id invalid, updating to "
                    "inst_id=0x%x",
                    (unsigned int)cmd->ind_data.instance_id);

    info->instance_id = cmd->ind_data.instance_id;
  }
  else if (info->instance_id != cmd->ind_data.instance_id)
  {
    ds_ucsd_log_err("ds_ucsd_process_csvt_ind: inst_id didn't match "
                    "tbl inst_id=0x%x, given inst_id=0x%x",
                    (unsigned int)info->instance_id,
                    (unsigned int)cmd->ind_data.instance_id);
    goto bail;
  }

  instance_id = DS_UCSD_CLIENT_ID_TO_INST_ID(client_id);

  /* Initialize the event data */
  memset(&evt_data, 0, sizeof(evt_data));

  /* Initialize the datapath */
  memset(&datapath, 0, sizeof(datapath));

  /* Initialize the ce_param */
  memset(&ce_param, 0, sizeof(ce_param));

  switch (cmd->ind_data.ind_type)
  {
    case QMI_CSVT_IND_TYPE_CALL_CONFIRM:
      /* Set the call event as Call Confirm */
      evt_data.call_event = CM_CALL_EVENT_CALL_CONF;

      ds_ucsd_api_process_call_confirm_ind(instance_id,
                                           &cmd->ind_data,
                                           &evt_data.event_info.confirm);
      break;

    case QMI_CSVT_IND_TYPE_CALL_PROGRESS:
      /* Set the call event as Call Progress */
      evt_data.call_event = CM_CALL_EVENT_PROGRESS_INFO_IND;

      ds_ucsd_api_process_call_progress_ind(instance_id,
                                            &cmd->ind_data,
                                            &evt_data.event_info.progress,
                                            &datapath);
      break;

    case QMI_CSVT_IND_TYPE_CALL_CONNECT:
      /* Set the call event as Call Connect */
      evt_data.call_event = CM_CALL_EVENT_CONNECT;

      ds_ucsd_api_process_call_connect_ind(instance_id,
                                           &cmd->ind_data,
                                           &evt_data.event_info.connect,
                                           &datapath);
      break;

    case QMI_CSVT_IND_TYPE_CALL_SETUP:
      /* Set the call event as Call Setup */
      evt_data.call_event = CM_CALL_EVENT_SETUP_IND;

      ds_ucsd_api_process_call_setup_ind(instance_id,
                                         &cmd->ind_data,
                                         &evt_data.event_info.setup);
      break;

    case QMI_CSVT_IND_TYPE_INCOMING_CALL:
      /* Set the call event as Call Setup */
      evt_data.call_event = CM_CALL_EVENT_INCOM;

      ds_ucsd_api_process_incoming_call_ind(instance_id,
                                            &cmd->ind_data,
                                            &evt_data.event_info.incoming);
      break;

    case QMI_CSVT_IND_TYPE_CALL_END:
      /* Set the call event as Call End */
      evt_data.call_event = CM_CALL_EVENT_END;

      ds_ucsd_api_process_call_end_ind(instance_id,
                                       &cmd->ind_data,
                                       &evt_data.event_info.end,
                                       &ce_param);

      /* Since the call has ended, invalidate instace_id */
      info->instance_id = QMI_CSVT_INVALID_INST_ID;
      break;

    case QMI_CSVT_IND_TYPE_CALL_MODIFY:
      /* Set the call event as Call End */
      evt_data.call_event = CM_CALL_EVENT_MODIFY_IND;

      ds_ucsd_api_process_call_modify_ind(instance_id,
                                          &cmd->ind_data,
                                          &evt_data.event_info.modify,
                                          &datapath);
      break;

    default:
      ds_ucsd_log_err("ds_ucsd_process_csvt_ind: unknown indication=%d",
                      cmd->ind_data.ind_type);
      goto bail;
  }


  /* Call the client callback */
  info->client_evt_cb(&evt_data,
                      info->client_data);

bail:
  DSUCSD_LOG_TRACE_EXIT;
  return;
}


/*===========================================================================
  FUNCTION  ds_ucsd_free_cmd_cb
===========================================================================*/
/*!
@brief
 Callback function registered with the Command Thread to free a command buffer
 after execution of the command is complete

@param
  ds_cmd - Command to be freed
  data   - Command data

@return
  void

*/
/*=========================================================================*/
static void ds_ucsd_free_cmd_cb
(
  ds_cmd_t *ds_cmd,
  void     *data
)
{
  /* Free the memory allocated for the command */
  ds_free((ds_ucsd_cmdq_cmd_type *)ds_cmd);
}


/*===========================================================================
  FUNCTION ds_ucsd_api_csvt_ind_cb
===========================================================================*/
/*!
  Indication callback function registered with QMI CSVT

  @param ind_data [in]   Data associated with the QMI CSVT indication

  @param usr_data [in]   User data provided during registration

  @return
  None
*/
/*=========================================================================*/
static void
ds_ucsd_api_csvt_ind_cb
(
  qmi_csvt_ind_data_type  *ind_data,
  void                    *user_data
)
{
  ds_ucsd_cmdq_cmd_type        *cmd = NULL;


  DSUCSD_LOG_TRACE_ENTRY;

  if (!ind_data)
  {
    ds_ucsd_log_err("ds_ucsd_api_csvt_ind_cb: bad input parameter");
    return;
  }

  /* Allocate a new command */
  if (NULL == (cmd = ds_malloc(sizeof(ds_ucsd_cmdq_cmd_type))))
  {
    ds_ucsd_log_err("ds_ucsd_api_csvt_ind_cb: alloc failed\n");
    return;
  }

  cmd->ind_data    = *ind_data;
  cmd->user_data   = user_data;
  cmd->ds_cmd.data = NULL;

  /* Asssign default execution and free handlers */
  cmd->ds_cmd.execute_f = ds_ucsd_process_csvt_ind;
  cmd->ds_cmd.free_f    = ds_ucsd_free_cmd_cb;

  ds_ucsd_log_med("ds_ucsd_api_csvt_ind_cb: posting event=%d\n", ind_data->ind_type);

  if (ds_cmdq_enq(&ds_ucsd_cmdq, (const ds_cmd_t *)cmd) < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_csvt_ind_cb: ds_cmdq_enq() failed\n");
    free(cmd);
  }

  DSUCSD_LOG_TRACE_EXIT;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_translate_to_csvt_call_params
===========================================================================*/
/*!
  Translate the given DSUCSD call parameters to QMI CSVT call params

  @param call_mode [in]  Specify single or dual mode call. Applicable only
                         for builds supporting multimedia fallback
                         (Service Change and UDI/RDI Fallback (SCUDIF)).
  @param speed [in]      Fixed network data rate, per +CBST AT command.
  @param name [in]       Synchronous or asynchronous name flag per +CBST
                         AT command.
  @param connection_element [in] Connection element per +CBST AT command.
  @param waiur [in]              Wanted air interface rate per +CHSN AT
                                 command.
  @param dial_string [in]        NULL-terminated dial string without dialing
                                 modifiers.
  @param dial_string_len [in]    Length of dial string.
  @param modifiers [in]          Dialing modifiers.
  @param uus_info [in]           User-to-user signaling for videotelephony call
  @param csvt_call_params [out]  QMI CSVT call params

  @return
  None

  @note
  This function assumes that the input parameters have been validated by the
  caller
*/
/*=========================================================================*/
static void
ds_ucsd_api_translate_to_csvt_call_params
(
  ds_ucsd_client_id_type           client_id,
  uint8                            call_mode,
  uint8                            speed,
  uint8                            name,
  uint8                            connection_element,
  uint8                            waiur,
  const uint8                      *dial_string,
  uint8                            dial_string_len,
  ds_ucsd_dial_modifier_info_type  *modifiers,
  ds_ucsd_uus_info_type            *uus_info,
  qmi_csvt_call_params_type        *csvt_call_params
)
{
  /* Update the mandatory parameters */
  /* Instance ID */
  csvt_call_params->inst_id = client_id;

  /* Call Mode */
  csvt_call_params->call_mode = call_mode;

  /* Dial String */
  strlcpy(csvt_call_params->dial_string,
         (const char *) dial_string,
         sizeof(csvt_call_params->dial_string));

  csvt_call_params->param_mask = 0;

  /* Update the optional parameters */
  /* Network Data Rate */
  csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_NETWORK_DATARATE;
  csvt_call_params->network_data_rate = speed;

  /* Synchronous Call */
  csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_SYNCHRONOUS_CALL;
  csvt_call_params->is_call_synchronous = name;

  /* Transparent Call */
  csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_TRANSPARENT_CALL;
  csvt_call_params->is_call_transparent = connection_element;

#if 0
  /* FIXME: Enable setting other optional parameters in the future. Testing so far
     was done with only the above three optional parameters enabled. Disabling the
     rest to not break existing pass cases */
  /* Air Interface Data Rate */
  csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_AIR_IFACE_DATARATE;
  csvt_call_params->air_iface_data_rate = waiur;

  if (modifiers && modifiers->modifier_present)
  {
    switch (modifiers->modifier)
    {
      case DS_UCSD_CLI_AND_CUG_MODIFIER:
      case DS_UCSD_CLI_DIAL_MODIFIER:
        /* CLI Enabled */
        csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_CLI_ENABLED;
        csvt_call_params->is_cli_enabled = modifiers->modifier_info.cli.cli_enabled;
        if (DS_UCSD_CLI_DIAL_MODIFIER == modifiers->modifier)
        {
          break;
        }
        /* Intentional fall through for DS_UCSD_CLI_AND_CUG_MODIFIER case */

      case DS_UCSD_CUG_DIAL_MODIFIER:
        /* CUG Enabled */
        csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_CUG_ENABLED;
        csvt_call_params->is_cug_enabled = modifiers->modifier_info.cug.cug_enabled;

        if (modifiers->modifier_info.cug.cug_index_enabled)
        {
          /* CUG Index */
          csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_CUG_INDEX;
          csvt_call_params->cug_index = modifiers->modifier_info.cug.cug_index_val;
        }

        /* Supress Preferred CUG */
        csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_SUPRESS_PREF_CUG;
        csvt_call_params->supress_pref_cug = modifiers->modifier_info.cug.suppress_pref_cug;

        /* Supress Outgoing Access */
        csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_SUPRESS_OUT_ACCESS;
        csvt_call_params->supress_out_access = modifiers->modifier_info.cug.suppress_outgoing_access;
        break;

      default:
        ds_ucsd_log_err("ds_ucsd_api_translate_to_csvt_call_params: unknown modifier=%d",
                        modifiers->modifier);
        break;
    }
  }

  if (uus_info && uus_info->present)
  {
    /* UUS ID Type */
    csvt_call_params->param_mask |= QMI_CSVT_CALL_PARAM_MASK_UUS_ID;
    csvt_call_params->uus_id.uus_id_type = uus_info->uus_id;

    /* UUS ID */
    strlcpy(csvt_call_params->uus_id.uus_id,
            (const char *)uus_info->uus_data,
            sizeof(csvt_call_params->uus_id.uus_id));
  }
#endif
}


/*===========================================================================
  FUNCTION ds_ucsd_api_get_target_type
===========================================================================*/
/*!
  Reads the ro.baseband Android property and updates the global
  ds_ucsd_baseband value

  @param
  None.

  @return
  None.

  @dependencies
  None.
*/
/*=========================================================================*/
static void ds_ucsd_api_update_baseband(void)
{
  int ret = 0;
  char args[PROPERTY_VALUE_MAX];
  char def[DS_UCSD_PROPERTY_BASEBAND_SIZE];

  /* retrieve value of DS_UCSD_PROPERTY_BASEBAND */
  (void)strlcpy(def,
                DS_UCSD_BASEBAND_VALUE_UNDEFINED,
                DS_UCSD_PROPERTY_BASEBAND_SIZE);

  memset(args, 0, sizeof(args));

  ret = property_get(DS_UCSD_PROPERTY_BASEBAND, args, def);

  if (ret > DS_UCSD_PROPERTY_BASEBAND_SIZE)
  {
    ds_ucsd_log_err("ds_ucsd_api_update_baseband: property [%s] has size [%d] "
                    "that exceeds max [%d]",
                    DS_UCSD_PROPERTY_BASEBAND,
                    ret,
                    DS_UCSD_PROPERTY_BASEBAND_SIZE);
    return;
  }

  (void)strlcpy(ds_ucsd_baseband,
                args,
                DS_UCSD_PROPERTY_BASEBAND_SIZE);

  ds_ucsd_log_med("ds_ucsd_api_update_baseband: baseband property is set to [%s]",
                  ds_ucsd_baseband);
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION ds_ucsd_api_register_client
===========================================================================*/
/*!
  Registers the application client with the UCSD subtask. This function
  should be called by the application task at initialization.

  @param call_types [in]    Bitmap of the UCSD call types for which the client
                            subscribes for call control notification
                            events. Currently, async/sync CSVT and Video
                            Telephony calls are supported.
  @param call_event_cb [in] Client callback function to receive call control
                            notification events.
  @param user_info_ptr [in] Client context value, which is provided
                            with call control notification events.
  @return
  The caller is returned a client ID, which must be checked to determine
  the status of registration. A valid client ID is returned on successful
  registration; DSUCSD_INVALID_CLIENT_ID on registration failure.

  @dependencies
  qmi_init() must be called before calling this function
*/
/*=========================================================================*/
ds_ucsd_client_id_type ds_ucsd_api_register_client
(
  uint8                       call_types,
  ds_ucsd_call_event_cb_type  call_event_cb,
  void                        *user_info_ptr
)
{
  qmi_csvt_clnt_hndl                 csvt_hndl;
  qmi_csvt_event_report_params_type  evt_params;
  ds_ucsd_client_id_type             client_id = DSUCSD_INVALID_CLIENT_ID;
  const char                         *conn_id = NULL;
  int i, rc, qmi_err;


  DSUCSD_LOG_TRACE_ENTRY;

  /* Lock the client info tbl mutex */
  pthread_mutex_lock(&ds_ucsd_client_info_mutex);

  if (!call_event_cb)
  {
    ds_ucsd_log_err("ds_ucsd_api_register_client: bad input parameter");
    goto bail;
  }

  /* Find an available client info table entry */
  for (i = 0; i < DS_UCSD_API_MAX_CLIENTS; ++i)
  {
    if (!ds_ucsd_client_info_tbl[i].is_valid)
    {
      break;
    }
  }

  /* Free entry not found */
  if (i >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_register_client: failed to find free tbl entry");
    goto bail;
  }

  /* If this is the first registered client, call qmi_init() */
  if (0 == ds_ucsd_api_get_active_client_count())
  {
#ifdef FEATURE_DATA_LOG_QXDM
    /* Initialize Diag services */
    if (TRUE != Diag_LSM_Init(NULL))
    {
      ds_ucsd_log_err("failed on Diag_LSM_Init");
    }
#endif

    DSUCSD_LOG_TRACE_ENTRY;

    if ((ds_ucsd_qmi_hndl = qmi_init(NULL, NULL)) < 0)
    {
      ds_ucsd_log_err("ds_ucsd_api_register_client: qmi_init failed");
      goto bail;
    }

    if (ds_cmdq_init(&ds_ucsd_cmdq, DS_UCSD_CMDQ_MAX_CMDS) < 0)
    {
      ds_ucsd_log_err("ds_ucsd_api_register_client: ds_cmdq_init failed");
      goto bail;
    }

    ds_ucsd_api_update_baseband();
  }

  conn_id = ds_ucsd_api_get_qmi_conn_id();

  if (!conn_id)
  {
    ds_ucsd_log_err("ds_ucsd_api_register_client: NULL conn_id");
    goto bail;
  }

  ds_ucsd_log_med("ds_ucsd_api_register_client: using conn_id=%s for CSVT registration",
                  conn_id);

  /* Initialize a QMI CSVT client */
  csvt_hndl = qmi_csvt_init_client(conn_id,
                                   ds_ucsd_api_csvt_ind_cb,
                                   (void *) i);

  if (QMI_CSVT_INVALID_HNDL == csvt_hndl)
  {
    ds_ucsd_log_err("ds_ucsd_api_register_client: qmi_csvt_init_client failed");

    if (0 == ds_ucsd_api_get_active_client_count())
    {
      ds_ucsd_log_high("ds_ucsd_api_register_client: releasing QMI");

      if (qmi_release(ds_ucsd_qmi_hndl) < 0)
      {
        ds_ucsd_log_err("ds_ucsd_api_register_client: qmi_release() failed");
      }
    }

    goto bail;
  }

  /* Mark the entry as being used */
  ds_ucsd_client_info_tbl[i].is_valid = TRUE;

  /* Save the CSVT client handle */
  ds_ucsd_client_info_tbl[i].csvt_hndl = csvt_hndl;

  /* Save the client callback function */
  ds_ucsd_client_info_tbl[i].client_evt_cb = call_event_cb;
  ds_ucsd_client_info_tbl[i].client_data   = user_info_ptr;

  /* Register for indications from QMI CSVT */
  memset(&evt_params, 0, sizeof(evt_params));
  evt_params.param_mask |= QMI_CSVT_EVT_REPORT_CALL_EVENTS_PARAM_MASK;
  evt_params.report_call_events = TRUE;

  evt_params.param_mask |= QMI_CSVT_EVT_REPORT_CALL_TYPE_MASK_PARAM_MASK;
  evt_params.call_type_mask = call_types;

  rc = qmi_csvt_set_event_report(csvt_hndl,
                                 &evt_params,
                                 &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_register_client: failed to register for indications "
                    "rc=%d, err=%d", rc, qmi_err);
  }

  /* Return the entry index as the client_id */
  client_id = i;

bail:
  DSUCSD_LOG_TRACE_RETURN(client_id);
  /* Unlock the client info tbl mutex */
  pthread_mutex_unlock(&ds_ucsd_client_info_mutex);
  return client_id;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_deregister_client
===========================================================================*/
/*!
  Deregisters the application client from the UCSD
  subtask. This function should be called by the application task at shutdown.

  @param client_id [in] Client ID returned on API registration.

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
/*=========================================================================*/
boolean ds_ucsd_api_deregister_client
(
  ds_ucsd_client_id_type client_id
)
{
  qmi_csvt_clnt_hndl  csvt_hndl = QMI_CSVT_INVALID_HNDL;
  boolean             ret = FALSE;
  int rc;


  DSUCSD_LOG_TRACE_ENTRY;

  /* Lock the client info tbl mutex */
  pthread_mutex_lock(&ds_ucsd_client_info_mutex);

  if (client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_deregister_client: bad input parameter");
    goto bail;
  }

  if (!ds_ucsd_client_info_tbl[client_id].is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_deregister_client: client tbl entry not valid "
                    "client_id=%d", client_id);
    goto bail;
  }

  /* Save the CSVT handle */
  csvt_hndl = ds_ucsd_client_info_tbl[client_id].csvt_hndl;

  /* Reset the client info entry */
  ds_ucsd_client_info_tbl[client_id].is_valid      = FALSE;
  ds_ucsd_client_info_tbl[client_id].csvt_hndl     = QMI_CSVT_INVALID_HNDL;
  ds_ucsd_client_info_tbl[client_id].instance_id   = QMI_CSVT_INVALID_INST_ID;
  ds_ucsd_client_info_tbl[client_id].client_evt_cb = NULL;
  ds_ucsd_client_info_tbl[client_id].client_data   = NULL;

  /* Release the QMI CSVT client */
  rc = qmi_csvt_release_client(csvt_hndl);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_deregister_client: qmi_csvt_release_client failed "
                    "err=%d", rc);
  }
  else
  {
    /* Set the operation as successful */
    ret = TRUE;
  }

  /* If this is the last client to be unregistered, call qmi_release() */
  if (0 == ds_ucsd_api_get_active_client_count())
  {
    ds_ucsd_log_high("ds_ucsd_api_deregister_client: releasing QMI");

    if (qmi_release(ds_ucsd_qmi_hndl) < 0)
    {
      ds_ucsd_log_err("ds_ucsd_api_deregister_client: qmi_release() failed");
    }

    if (ds_cmdq_deinit(&ds_ucsd_cmdq) < 0)
    {
      ds_ucsd_log_err("ds_ucsd_api_deregister_client: ds_cmdq_deinit() failed");
    }

    /* Deinit diag */
#ifdef FEATURE_DATA_LOG_QXDM
    (void) Diag_LSM_DeInit();
#endif

  }

bail:
  DSUCSD_LOG_TRACE_RETURN(ret);
  /* Unlock the client info tbl mutex */
  pthread_mutex_unlock(&ds_ucsd_client_info_mutex);
  return ret;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_originate_call
===========================================================================*/
/*!
  Originates a CS data call. This function sends the
  DS_UCSD_APP_ORIG_CALL_CMD command to the UCSD subtask. The function
  should be called by a CS data application.

  @param client_id [in]  Client ID returned on API registration.
  @param call_mode [in]  Specify single or dual mode call. Applicable only
                         for builds supporting multimedia fallback
                         (Service Change and UDI/RDI Fallback (SCUDIF)).
  @param speed [in]      Fixed network data rate, per +CBST AT command.
  @param name [in]       Synchronous or asynchronous name flag per +CBST
                         AT command.
  @param connection_element [in] Connection element per +CBST AT command.
  @param waiur [in]              Wanted air interface rate per +CHSN AT
                                 command.
  @param dial_string [in]        NULL-terminated dial string without dialing
                                 modifiers.
  @param dial_string_len [in]    Length of dial string.
  @param modifiers [in]          Dialing modifiers.
  @param uus_info [in]           User-to-user signaling for videotelephony call
                                 two-stage dialing.

  @return
  A CS data call instance ID is returned on success; DSUCSD_INVALID_INST_ID
  on failure.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.

  @sideeffects
  May allocate an entry in the CS data call table.
*/
/*=========================================================================*/
uint8 ds_ucsd_api_originate_call
(
  ds_ucsd_client_id_type           client_id,
  uint8                            call_mode,
  uint8                            speed,
  uint8                            name,
  uint8                            connection_element,
  uint8                            waiur,
  const uint8                      *dial_string,
  uint8                            dial_string_len,
  ds_ucsd_dial_modifier_info_type  *modifiers,
  ds_ucsd_uus_info_type            *uus_info
)
{
  ds_ucsd_client_info_type  *info = NULL;
  qmi_csvt_instance_id_type  csvt_inst_id;
  qmi_csvt_call_params_type  call_params;
  uint8    inst_id = DSUCSD_INVALID_INST_ID;
  int      rc, qmi_err = QMI_NO_ERR;


  DSUCSD_LOG_TRACE_ENTRY;

  if (!dial_string || client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_originate_call: bad input parameter(s)");
    goto bail;
  }

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_originate_call: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }

  info->instance_id = DS_UCSD_CLIENT_ID_TO_INST_ID(client_id);

  memset(&call_params, 0, sizeof(call_params));

  ds_ucsd_api_translate_to_csvt_call_params(info->instance_id,
                                            call_mode,
                                            speed,
                                            name,
                                            connection_element,
                                            waiur,
                                            dial_string,
                                            dial_string_len,
                                            modifiers,
                                            uus_info,
                                            &call_params);

  /* Originate the call */
  rc = qmi_csvt_originate_call(info->csvt_hndl,
                               &call_params,
                               &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_originate_call: qmi_csvt_originate_call failed "
                    "rc=%d, err=%d", rc, qmi_err);
    info->instance_id = QMI_CSVT_INVALID_INST_ID;
    goto bail;
  }

  inst_id = info->instance_id;

bail:
  DSUCSD_LOG_TRACE_RETURN(inst_id);
  return inst_id;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_confirm_call
===========================================================================*/
/*!
  Confirms an incoming CS data call. This function sends the
  DS_UCSD_APP_CONFIRM_CALL_CMD command to the UCSD subtask.  The function
  should be called by a CS data application.

  @param client_id [in] Client ID returned on API registration.
  @param inst_id [in]   Allocated CS data call instance ID.
  @param call_type [in] Specify preferred mode of a dual mode call.
                        Applicable only for builds supporting
                        multimedia fallback (SCUDIF).
  @param result_params_ptr [in] Indicates if the setup is rejected and gives
                                a cause value.

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.

  @sideeffects
  If call_type is changed relative to the setup event, a preferred bearer
  order will be changed in the network signaling.
*/
/*=========================================================================*/
boolean ds_ucsd_api_confirm_call
(
  ds_ucsd_client_id_type     client_id,
  uint8                      inst_id,
  ds_ucsd_call_type          call_type,
  ds_ucsd_answer_param_type *result_params_ptr
)
{
  ds_ucsd_client_info_type  *info = NULL;
  boolean  ret = FALSE;
  boolean  confirm_call = TRUE;
  int      rc, qmi_err = QMI_NO_ERR;


  DSUCSD_LOG_TRACE_ENTRY;

  if (client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_confirm_call: bad input parameter");
    goto bail;
  }

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_confirm_call: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }
  else if (DS_UCSD_CLIENT_ID_TO_INST_ID(client_id) != inst_id)
  {
    ds_ucsd_log_err("ds_ucsd_api_confirm_call: inst_id didn't match "
                    "client_id=%d, given inst_id=%d", client_id, inst_id);
    goto bail;
  }

  if (result_params_ptr)
  {
    confirm_call = !result_params_ptr->reject;
  }

  rc = qmi_csvt_confirm_call(info->csvt_hndl,
                             info->instance_id,
                             confirm_call,
                             &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_confirm_call: qmi_csvt_confirm_call failed "
                    "rc=%d, err=%d", rc, qmi_err);
    goto bail;
  }

  /* Set the operation as successful */
  ret = TRUE;

bail:
  DSUCSD_LOG_TRACE_RETURN(ret);
  return ret;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_answer_call
===========================================================================*/
/*!
  Answers a CS data call. This function sends the
  DS_UCSD_APP_ANSWER_CALL_CMD command to the UCSD subtask. The function
  should be called by a CS data application.

  @param client_id [in]         Client ID returned on API registration.
  @param inst_id [in]           Allocated CS data call instance ID.
  @param answer_params_ptr [in] Indicates if the call answer is
                                rejected and gives a cause value.

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
/*=========================================================================*/
boolean ds_ucsd_api_answer_call
(
  ds_ucsd_client_id_type     client_id,
  uint8                      inst_id,
  ds_ucsd_answer_param_type *answer_params_ptr
)
{
  ds_ucsd_client_info_type  *info = NULL;
  boolean  ret = FALSE;
  boolean  answer_call = TRUE;
  int      rc, qmi_err = QMI_NO_ERR;


  DSUCSD_LOG_TRACE_ENTRY;

  if (client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_answer_call: bad input parameter");
    goto bail;
  }

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_answer_call: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }
  else if (DS_UCSD_CLIENT_ID_TO_INST_ID(client_id) != inst_id)
  {
    ds_ucsd_log_err("ds_ucsd_api_answer_call: inst_id didn't match "
                    "client_id=%d, given inst_id=%d", client_id, inst_id);
    goto bail;
  }

  if (answer_params_ptr)
  {
    answer_call = !answer_params_ptr->reject;
  }

  rc = qmi_csvt_answer_call(info->csvt_hndl,
                            info->instance_id,
                            answer_call,
                            &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_answer_call: qmi_csvt_answer_call failed "
                    "rc=%d, err=%d", rc, qmi_err);
    goto bail;
  }

  /* Set the operation as successful */
  ret = TRUE;

bail:
  DSUCSD_LOG_TRACE_RETURN(ret);
  return ret;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_end_call
===========================================================================*/
/*!
  Ends a CS data call. This function sends the
  DS_UCSD_APP_END_CALL_CMD command to the UCSD subtask.
  The function should be called by a CS data application.

  @param client_id [in]      Client ID returned on API registration.
  @param inst_id [in]        Allocated CS data call instance ID.

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
/*=========================================================================*/
boolean ds_ucsd_api_end_call
(
  ds_ucsd_client_id_type     client_id,
  uint8                      inst_id
)
{
  ds_ucsd_client_info_type  *info = NULL;
  boolean  ret = FALSE;
  int      rc, qmi_err = QMI_NO_ERR;


  DSUCSD_LOG_TRACE_ENTRY;

  if (client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_end_call: bad input parameter");
    goto bail;
  }

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_end_call: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }
  else if (DS_UCSD_CLIENT_ID_TO_INST_ID(client_id) != inst_id)
  {
    ds_ucsd_log_err("ds_ucsd_api_end_call: inst_id didn't match "
                    "client_id=%d, given inst_id=%d", client_id, inst_id);
    goto bail;
  }

  rc = qmi_csvt_end_call(info->csvt_hndl,
                         info->instance_id,
                         &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_end_call: qmi_csvt_end_call failed "
                    "rc=%d, err=%d", rc, qmi_err);
    goto bail;
  }

  /* Set the operation as successful */
  ret = TRUE;

bail:
  DSUCSD_LOG_TRACE_RETURN(ret);
  return ret;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_modify_call
===========================================================================*/
/*!
  Modifies a CS data call to a new call type. This function
  sends the DS_UCSD_APP_MODIFY_CALL_CMD command to the UCSD
  subtask. The function should be called by a CS data application.
  It is applicable only for builds supporting multimedia fallback (SCUDIF).

  @param client_id [in]      Client ID returned on API registration.
  @param inst_id [in]        Allocated CS data call instance ID.
  @param msg_type [in]       Specifies command or response.
  @param new_call_type [in]  Indicates the new call type for the originating
                             modify request.
  @param accept [in]         Indicates an accept/reject for the incoming modify
                             request.

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
/*=========================================================================*/
boolean ds_ucsd_api_modify_call
(
  ds_ucsd_client_id_type     client_id,
  uint8                      inst_id,
  ds_ucsd_msg_type           msg_type,
  ds_ucsd_call_type          new_call_type,
  boolean                    accept
)
{
  ds_ucsd_client_info_type          *info = NULL;
  qmi_csvt_modify_call_params_type  modify_params;
  boolean  ret = FALSE;
  int      rc, qmi_err = QMI_NO_ERR;


  DSUCSD_LOG_TRACE_ENTRY;

  if (client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_modify_call: bad input parameter");
    goto bail;
  }

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_modify_call: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }
  else if (DS_UCSD_CLIENT_ID_TO_INST_ID(client_id) != inst_id)
  {
    ds_ucsd_log_err("ds_ucsd_api_modify_call: inst_id didn't match "
                    "client_id=%d, given inst_id=%d", client_id, inst_id);
    goto bail;
  }

  /* Fill in the parameters for the modify request */
  memset(&modify_params, 0, sizeof(modify_params));

  /* Fill the mandatory parameters */
  /* Instance ID */
  modify_params.instance_id = info->instance_id;

  /* New call type */
  modify_params.new_call_type = (qmi_csvt_call_type) new_call_type;

  rc = qmi_csvt_modify_call(info->csvt_hndl,
                            &modify_params,
                            &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_modify_call: qmi_csvt_modify_call failed "
                    "rc=%d, err=%d", rc, qmi_err);
    goto bail;
  }

  /* Set the operation as successful */
  ret = TRUE;

bail:
  DSUCSD_LOG_TRACE_RETURN(ret);
  return ret;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_lookup_cm_callid
===========================================================================*/
/*!
  Performs a lookup in the CS data call table to find the
  Call Manager (CM) call ID that corresponds to the CS data call
  instance ID.

  @param inst_id [in]  Allocated CS data call instance ID.

  @return
  A mapped CM call ID is returned on success; CM_CALL_ID_INVALID on
  failure.

  @dependencies
  None.
*/
/*=========================================================================*/
uint8 ds_ucsd_api_lookup_cm_callid
(
  const uint8 inst_id
)
{
  uint8  cm_call_id = DSUCSD_INVALID_INST_ID;
  int i;


  DSUCSD_LOG_TRACE_ENTRY;

  ds_ucsd_log_err("ds_ucsd_api_lookup_cm_callid: unsupported function");

  DSUCSD_LOG_TRACE_RETURN(cm_call_id);

  return cm_call_id;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_set_rlp
===========================================================================*/
/*!
  Changes the RLP parameters for
  the modem processor. The changes are system-wide and affect all future
  nontransparent CS data calls.

  @param client_id [in]      Client ID returned on API registration.
  @param rlp_params_ptr [in] Pointer to the RLP and data compression
                             parameters.

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
/*=========================================================================*/
boolean ds_ucsd_api_set_rlp
(
  ds_ucsd_client_id_type      client_id,
  const ds_ucsd_nt_info_type *rlp_params_ptr
)
{
  qmi_csvt_set_rlp_params_type  set_rlp_params;
  ds_ucsd_client_info_type  *info = NULL;
  boolean  ret = FALSE;
  int      rc, qmi_err = QMI_NO_ERR;


  DSUCSD_LOG_TRACE_ENTRY;

  if (!rlp_params_ptr || client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_set_rlp: bad input parameter(s)");
    goto bail;
  }

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_set_rlp: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }

  /* Fill the set RLP parameters */
  memset(&set_rlp_params, 0, sizeof(set_rlp_params));

  if (rlp_params_ptr->rlp_info.present)
  {
    /* Mark that RLP params as valid */
    set_rlp_params.param_mask |= QMI_CSVT_RLP_PARAMS_VALID_PARAM_MASK;

    /* Version */
    set_rlp_params.rlp_params.version          = rlp_params_ptr->rlp_info.version;

    /* TX Window Size */
    set_rlp_params.rlp_params.tx_window_size   = rlp_params_ptr->rlp_info.iws;

    /* RX Window Size */
    set_rlp_params.rlp_params.rx_window_size   = rlp_params_ptr->rlp_info.mws;

    /* Ack Timer */
    set_rlp_params.rlp_params.ack_timer        = rlp_params_ptr->rlp_info.T1;

    /* Retransmission Attempts */
    set_rlp_params.rlp_params.retrans_attempts = rlp_params_ptr->rlp_info.N2;

    /* Resequencing Timer */
    set_rlp_params.rlp_params.reseq_timer      = rlp_params_ptr->rlp_info.T4;
  }

  if (rlp_params_ptr->v42_info.present)
  {
    /* Mark that V42 params as valid */
    set_rlp_params.param_mask |= QMI_CSVT_V42_PARAMS_VALID_PARAM_MASK;

    /* Direction */
    set_rlp_params.v42_params.direction        = rlp_params_ptr->v42_info.direction;

    /* Negotiation Preference */
    set_rlp_params.v42_params.negotiation_pref = rlp_params_ptr->v42_info.negotiation;

    /* Max Dictionary Size */
    set_rlp_params.v42_params.max_dict_size    = rlp_params_ptr->v42_info.max_dict;

    /* Max String Size */
    set_rlp_params.v42_params.max_str_size     = rlp_params_ptr->v42_info.max_string;
  }

  rc = qmi_csvt_set_rlp_params(info->csvt_hndl,
                               &set_rlp_params,
                               &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_set_rlp: qmi_csvt_set_rlp_params failed "
                    "rc=%d, err=%d", rc, qmi_err);
    goto bail;
  }

  /* Set the operation as successful */
  ret = TRUE;

bail:
  DSUCSD_LOG_TRACE_RETURN(ret);
  return ret;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_get_rlp
===========================================================================*/
/*!
  Queries the RLP parameters from the modem processor. The parameters
  apply to nontransparent CS data calls.

  @param client_id [in]       Client ID returned on API registration.
  @param rlp_params_ptr [out] Pointer to the RLP and data compression
                              parameters.

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
/*=========================================================================*/
boolean ds_ucsd_api_get_rlp
(
  ds_ucsd_client_id_type     client_id,
  ds_ucsd_rlp_sets_type     *rlp_params_ptr
)
{
  qmi_csvt_get_rlp_params_type  get_rlp_params;
  ds_ucsd_client_info_type  *info = NULL;
  boolean  ret = FALSE;
  int      rc, qmi_err = QMI_NO_ERR;
  int      i, j;
  uint16 param_mask[QMI_CSVT_MAX_RLP_PARAMS] =
  {
    QMI_CSVT_RLP_PARAMS_V1_VALID_PARAM_MASK,
    QMI_CSVT_RLP_PARAMS_V2_VALID_PARAM_MASK,
    QMI_CSVT_RLP_PARAMS_V3_VALID_PARAM_MASK
  };
  qmi_csvt_rlp_params_type *qmi_rlp_params[QMI_CSVT_MAX_RLP_PARAMS] =
  {
    &get_rlp_params.rlp_params_v1,
    &get_rlp_params.rlp_params_v2,
    &get_rlp_params.rlp_params_v3
  };


  DSUCSD_LOG_TRACE_ENTRY;

  if (!rlp_params_ptr || client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_rlp: bad input parameter(s)");
    goto bail;
  }

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_rlp: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }

  /* Initialize the output params to invalid */
  memset(rlp_params_ptr, 0, sizeof(ds_ucsd_rlp_sets_type));

  /* Get the RLP parameters */
  memset(&get_rlp_params, 0, sizeof(get_rlp_params));

  rc = qmi_csvt_get_rlp_params(info->csvt_hndl,
                               &get_rlp_params,
                               &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_rlp: qmi_csvt_get_rlp_params failed "
                    "rc=%d, err=%d", rc, qmi_err);
    goto bail;
  }

  /* Copy the RLP parameters */
  for (i = 0, j = 0; i < QMI_CSVT_MAX_RLP_PARAMS; ++i)
  {
    /* If the particular RLP parameter set is valid */
    if (get_rlp_params.param_mask & param_mask[i])
    {
      rlp_params_ptr->rlp_sets[j].present = TRUE;

      rlp_params_ptr->rlp_sets[j].version = qmi_rlp_params[i]->version;
      rlp_params_ptr->rlp_sets[j].iws     = qmi_rlp_params[i]->tx_window_size;
      rlp_params_ptr->rlp_sets[j].mws     = qmi_rlp_params[i]->rx_window_size;
      rlp_params_ptr->rlp_sets[j].T1      = qmi_rlp_params[i]->ack_timer;
      rlp_params_ptr->rlp_sets[j].N2      = qmi_rlp_params[i]->retrans_attempts;
      rlp_params_ptr->rlp_sets[j].T4      = qmi_rlp_params[i]->reseq_timer;

      /* Incremenent the number of valid sets seen so far */
      ++j;
    }
  }

  /* Update the number of valid sets */
  rlp_params_ptr->num_sets = j;

  /* If V42 parameters are valid */
  if (get_rlp_params.param_mask & QMI_CSVT_V42_PARAMS_VALID_PARAM_MASK)
  {
    rlp_params_ptr->v42_info.present = TRUE;

    rlp_params_ptr->v42_info.direction   = get_rlp_params.v42_params.direction;
    rlp_params_ptr->v42_info.negotiation = get_rlp_params.v42_params.negotiation_pref;
    rlp_params_ptr->v42_info.max_dict    = get_rlp_params.v42_params.max_dict_size;
    rlp_params_ptr->v42_info.max_string  = get_rlp_params.v42_params.max_str_size;
  }

  /* Set the operation as successful */
  ret = TRUE;

bail:
  DSUCSD_LOG_TRACE_RETURN(ret);
  return ret;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_get_callstats
===========================================================================*/
/*!
  Queries the data traffic statistics for the specified
  CS data call. The statistics are tracked only for those calls using the
  SIO data path. If a call is inactive, the last statistics are returned.

  @param client_id [in]       Client ID returned on API registration.
  @param inst_id [in]         Allocated CS data call instance ID.
  @param callstats_ptr [out]  Pointer to the call statistics information.

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
/*=========================================================================*/
boolean ds_ucsd_api_get_callstats
(
  ds_ucsd_client_id_type     client_id,
  uint8                      inst_id,
  ds_ucsd_call_stats_type   *callstats_ptr
)
{
  ds_ucsd_client_info_type  *info = NULL;
  qmi_csvt_call_stats_type  csvt_call_stats;
  boolean  ret = FALSE;
  int      rc, qmi_err = QMI_NO_ERR;


  DSUCSD_LOG_TRACE_ENTRY;

  if (!callstats_ptr || client_id < 0 || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_callstats: bad input parameter(s)");
    goto bail;
  }

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_callstats: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }
  else if (DS_UCSD_CLIENT_ID_TO_INST_ID(client_id) != inst_id)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_callstats: inst_id didn't match "
                    "client_id=%d, given inst_id=%d", client_id, inst_id);
    goto bail;
  }

  memset(&csvt_call_stats, 0, sizeof(csvt_call_stats));

  rc = qmi_csvt_get_call_stats(info->csvt_hndl,
                               info->instance_id,
                               &csvt_call_stats,
                               &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_callstats: qmi_csvt_get_call_stats failed "
                    "rc=%d, err=%d", rc, qmi_err);
    goto bail;
  }

  /* Initialize the output pointer to invalid */
  memset(callstats_ptr, 0, sizeof(ds_ucsd_call_stats_type));

  /* Check if any of the optional parameters are returned */
  /* Call Active */
  if (csvt_call_stats.param_mask & QMI_CSVT_CALL_STATS_CALL_ACTIVE_PARAM_MASK)
  {
    callstats_ptr->call_active = csvt_call_stats.is_call_active;
    callstats_ptr->present = TRUE;
  }

  /* TX Counter */
  if (csvt_call_stats.param_mask & QMI_CSVT_CALL_STATS_TX_COUNTER_PARAM_MASK)
  {
    callstats_ptr->tx_data_counter = csvt_call_stats.tx_counter;
    callstats_ptr->present = TRUE;
  }

  /* RX Counter */
  if (csvt_call_stats.param_mask & QMI_CSVT_CALL_STATS_RX_COUNTER_PARAM_MASK)
  {
    callstats_ptr->rx_data_counter = csvt_call_stats.rx_counter;
    callstats_ptr->present = TRUE;
  }

  /* Set the operation as successful */
  ret = TRUE;

bail:
  DSUCSD_LOG_TRACE_RETURN(ret);
  return ret;
}


/*===========================================================================
  FUNCTION ds_ucsd_api_get_calltype
===========================================================================*/
/*!
  Queries the UCSD stack call type for the specified
  CM call ID. If the call instance mapped to the call ID is
  present, a UCSD call type is returned. If a call instance cannot be
  found, the call type will be DS_UCSD_UNKNOWN_CALL, and the return
  value will be FALSE.

  @param cm_call_id [in]  CM call ID.
  @param call_type [out]  UCSD call type.

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  None.
*/
/*=========================================================================*/
boolean ds_ucsd_api_get_calltype
(
  const uint8     cm_call_id,
  uint8           *call_type
)
{
  ds_ucsd_client_info_type  *info = NULL;
  qmi_csvt_call_info_type   csvt_call_info;
  boolean  ret = FALSE;
  int      rc, qmi_err = QMI_NO_ERR;
  uint8    client_id = cm_call_id;


  DSUCSD_LOG_TRACE_ENTRY;

  if (!call_type || client_id >= DS_UCSD_API_MAX_CLIENTS)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_calltype: bad input parameter(s)");
    goto bail;
  }

  info = &ds_ucsd_client_info_tbl[client_id];

  if (!info->is_valid)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_calltype: client tbl entry "
                    "not valid client_id=%d", client_id);
    goto bail;
  }

  memset(&csvt_call_info, 0, sizeof(csvt_call_info));

  /* Initialize the output param to invalid */
  *call_type = (uint8) QMI_CSVT_CALL_TYPE_INVALID;

  rc = qmi_csvt_get_call_info(info->csvt_hndl,
                              info->instance_id,
                              &csvt_call_info,
                              &qmi_err);

  if (rc < 0)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_calltype: qmi_csvt_get_call_type failed "
                    "rc=%d, err=%d", rc, qmi_err);
    goto bail;
  }

  *call_type = (uint8) csvt_call_info.call_type;

  /* Set the operation as successful */
  ret = TRUE;

bail:
  DSUCSD_LOG_TRACE_RETURN(ret);
  return ret;
}

/*===========================================================================
  FUNCTION ds_ucsd_api_get_data_port
===========================================================================*/
/*!
  Returns the data port to use for the current target

  @param data_port [out]  Data port to use

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  None.
*/
/*=========================================================================*/
boolean ds_ucsd_api_get_data_port
(
  const char  **data_port
)
{
  boolean ret = FALSE;
  int i;

  if (!data_port)
  {
    ds_ucsd_log_err("ds_ucsd_api_get_data_port: bad input parameter(s)");
    goto bail;
  }

  ds_ucsd_log_med("ds_ucsd_api_get_data_port: baseband property is set to [%s]",
                  ds_ucsd_baseband);

  *data_port = DS_UCSD_INVALID_DATA_PORT;

  /* Loop through the available data port entries */
  for (i = 0; i < (int)DS_UCSD_NUM_PORT_ENTRIES; ++i)
  {
    /* If a matching baseband is found */
    if (!strcmp(ds_ucsd_baseband, ds_ucsd_port_tbl[i].baseband))
    {
      *data_port = ds_ucsd_port_tbl[i].data_port;
      ret = TRUE;
      break;
    }
  }

  if (TRUE == ret)
  {
    ds_ucsd_log_med("ds_ucsd_api_get_data_port: baseband match found returning data port [%s]",
                    *data_port);
  }
  else
  {
    ds_ucsd_log_err("ds_ucsd_api_get_data_port: baseband match not found");
  }

bail:
  return ret;
}

