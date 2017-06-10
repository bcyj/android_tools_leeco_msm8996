/******************************************************************************

                        QTI_QMI.C

******************************************************************************/

/******************************************************************************

  @file    qti_qmi.c
  @brief   Qualcomm Tethering Interface QMI Messaging Implementation File

  DESCRIPTION
  Implementation file for QTI QMI messaging functions.

  ---------------------------------------------------------------------------
  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
  
******************************************************************************/



/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/23/13   sb/mp      Fix to prevent race condition during call bring up.
03/29/13   sb         QTI boot up optimizations for RNDIS.
02/04/12   sb/mp      Added support for dynamic USB composition switching.
12/05/12   mp         Added ioctl call to fetch the RNDIS downlink packet
                      size got from host.
10/03/12   mp         Fix to make QTI use QCMAP WWAN config.
06/29/12   sc         Revised version
05/24/12   sb         Initial version

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "qti_qmi.h"
#include "wireless_data_administrative_service_v01.h"
#include "wireless_data_service_v01.h"
#include "qmi_client_instance_defs.h"
#include "qmi_client.h"
#include "ds_util.h"
#include "ds_Utils_DebugMsg.h"

/*---------------------------------------------------------------------------
  QTI configuration paramters
---------------------------------------------------------------------------*/
static qti_conf_t * qti_qmi_conf;
unsigned long data_aggregation_max_datagrams;
unsigned long data_aggregation_max_size;

/*---------------------------------------------------------------------------
IOCTL to USB for RNDIS max datagrams
---------------------------------------------------------------------------*/
typedef unsigned char	u8;
typedef unsigned long   u32;
#define RNDIS_QC_IOCTL_MAGIC 'i'
#define RNDIS_QC_GET_MAX_PKT_PER_XFER   _IOR(RNDIS_QC_IOCTL_MAGIC, 1, u8)
#define RNDIS_QC_GET_MAX_PKT_SIZE	_IOR(RNDIS_QC_IOCTL_MAGIC, 2, u32)
#define QCMAP_QTI_IP_V4 4
#define QCMAP_QTI_IP_V6 6
#define QCMAP_QTI_IP_V4V6 10
#define QCMAP_QTI_MTYPE_ID 1
#define QCMAP_QTI_KEY_ID 100
#define QCMAP_QTI_MSG_SIZE (sizeof(int)*QCMAP_QTI_WWAN_PARAMS)
#define SET_DTR_HIGH 1
#define SET_DTR_LOW  0
#define RNDIS_MAX_PKTS_DIVISION_FACTOR 1500 /* Used to obtain the max datagrams
                                               from the max buffer size */

qcmap_qti_msg_t qcmap_qti_msg;
/*========================================================================== 

FUNCTION QTI_QMI_WDA_IND_CB()

DESCRIPTION

  Indication call back for WDA client: This function is registered during 
  WDA client initialization and is invoked when any indication is sent from 
  WDA service on modem to the client.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS 
  None

==========================================================================*/
int qti_qmi_wda_ind_cb(void)
{
  return QTI_SUCCESS;
}

/*========================================================================== 
  
FUNCTION QTI_QMI_WDS_IND_CB()

DESCRIPTION

  Indication call back for WDS client: This function is registered during 
  WDS client initialization and is invoked when any indication is sent from 
  WDS service on modem to the client.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS 
  None

==========================================================================*/
int qti_qmi_wds_ind_cb(void)
{
  return QTI_SUCCESS;
}

/*========================================================================== 

FUNCTION QTI_SET_DTR()

DESCRIPTION

  Sends DTR high or low messages to smd driver.    
DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS 
  None

==========================================================================*/
int qti_set_dtr(uint8_t set)
{
  int fd = 0, dtr_sig;
  int ret = QTI_FAILURE;
  fd = open("/dev/smdcntl8", O_RDWR);
  if (fd <= 0)
  {
    LOG_MSG_ERROR("Opening the device file failed errno %u error %s\n",
        errno, strerror(errno),0);
    return ret;
  }

  if (set == SET_DTR_HIGH)
  {
    dtr_sig = 0;
    dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
    if( dtr_sig >= 0  && !(dtr_sig & TIOCM_DTR))
    {
      LOG_MSG_INFO1("DTR bit not set..setting....dtr_sig:%d\n", dtr_sig,0,0);
      dtr_sig |= TIOCM_DTR;
      if((ioctl(fd, TIOCMSET, (void *)dtr_sig)) == -1)
      {
        LOG_MSG_INFO1("Ioctl call to set DTR bit failed errno %u error %s\n",
            errno, strerror(errno),0);
      }
      else
      {
        dtr_sig = 0;
        dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
        if( dtr_sig >= 0  && (dtr_sig & TIOCM_DTR))
        {
          LOG_MSG_INFO1("DTR bit set:%d\n", dtr_sig,0,0);
          ret=QTI_SUCCESS;
        }
        else
        { 
          LOG_MSG_INFO1("Unable to set DTR bit ",0,0,0);
        }
      }
    }
    else if (dtr_sig == -1)
    {
      LOG_MSG_ERROR("Failed to get DTR bits..exiting...failed errno %u error %s\n",
          errno, strerror(errno),0);
    }
  }
  else
  {
    dtr_sig = 0;
    dtr_sig |=  ioctl(fd, TIOCMGET, &dtr_sig);
    if( dtr_sig >= 0 && (dtr_sig & TIOCM_DTR))
    {
      LOG_MSG_INFO1("Clearing DTR bit....\n",0,0,0);
      dtr_sig &= (~TIOCM_DTR);
      if(ioctl(fd, TIOCMSET, (void *)dtr_sig) == -1)
      {
        LOG_MSG_ERROR("Set DTR bit failed.. errno %u error %s\n",
            errno, strerror(errno),0);
      }
      else
      {
        dtr_sig = 0;
        dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
        if( dtr_sig >= 0  && !(dtr_sig & TIOCM_DTR))
        {
          LOG_MSG_INFO1("Successfully reset DTR bit",0,0,0);
          ret = QTI_SUCCESS;
        }
        else
        {
          LOG_MSG_ERROR("Unable to Clear DTR bit \n",0,0,0)
        }
      }
    }
    else if( dtr_sig == -1)
    {
      LOG_MSG_ERROR("Failed to get DTR bits..exiting...failed errno %u error %s\n",
          errno, strerror(errno),0);
    }
  }
  if(fd > 0)
  {
    close(fd);
    fd = 0;
  }
  return ret;
}



/*========================================================================== 
  
FUNCTION QTI_QMI_WAIT_FOR_SRV_THEN_GET_INFO()

DESCRIPTION

  This function is used during WDA/WDS client initialization where we wait
  for the QMI service to come up and then we get the information about
  the service before we initialize a client for that service

DEPENDENCIES
  None.

RETURN VALUE
  TRUE on success
  FALSE on failure


SIDE EFFECTS 
  None

==========================================================================*/

static boolean qti_qmi_wait_for_srv_then_get_info
(
  qmi_idl_service_object_type svc_obj,
  qmi_service_info *qmi_svc_info
)
{
  qmi_client_os_params os_params;
  qmi_client_type notifier_handle;
  qmi_client_error_type err;
  boolean success = FALSE;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -*/

  LOG_MSG_INFO1("Entering qti_qmi_wait_for_srv_then_get_info",0,0,0);

  /*-----------------------------------------------------------------------
  Initialize client notifier
  ------------------------------------------------------------------------*/
  err = qmi_client_notifier_init(svc_obj, &os_params, &notifier_handle);
  if (err != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Error %d while trying to initialize client notifier", 
                  err,0,0);
  }
  else
  {
    /*---------------------------------------------------------------------
    Get service info
    ---------------------------------------------------------------------*/
    err = qmi_client_get_service_instance(svc_obj, 
                                          QMI_CLIENT_QMUX_RMNET_INSTANCE_8, 
                                          qmi_svc_info);
    if (err != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Error %d while trying to get service info", err,0,0);
    }
    else
    {
      LOG_MSG_INFO1("Got service instance success",0,0,0);
      success = TRUE;
    }

    /*----------------------------------------------------------------------
     We need to release the client notifier here. Client notifier is only
     used to notify the client when the QMI service comes up.However client
     notifier acts as an actual QMI service client even though it is not used
     as an actual client of the service. We need to release it here to avoid
     unnecessary overhead of maintaining another client.
    ------------------------------------------------------------------------*/
    err = qmi_client_release(notifier_handle);
    if (err != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Error %d while releasing client notifier handle", err,0,0);
    }
  }

  return success;
}

/*===========================================================================
FUNCTION QTI_QMI_FREE()

DESCRIPTION

  This function is used to release the QMI WDS clients and reset the call
  handles
 
DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS 
  None

===========================================================================*/

static int qti_qmi_free(void)
{
  int ret_val;
  int err;
/*- - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - --  - - - */

  LOG_MSG_INFO1("Enter qti-qmi-free function",0,0,0);
  err = qmi_client_release(qti_qmi_conf->qmi_qti_v4_wds_handle);
  if (err != QMI_NO_ERR)
  {
     LOG_MSG_ERROR("Error %d while releasing wds v4 client handle", err,0,0);
  }

  err = qmi_client_release(qti_qmi_conf->qmi_qti_v6_wds_handle);
  if (err != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Error %d while releasing wds v6 client handle", err,0,0);
  }

  qti_qmi_conf->qmi_qti_v4_wds_call_handle = (uint32) NULL;
  qti_qmi_conf->qmi_qti_v6_wds_call_handle = (uint32) NULL;
  return QTI_SUCCESS;
}

/*===========================================================================
FUNCTION QTI_WDA_INIT()

DESCRIPTION

  This function initializes WDA client handle
 
DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS 
  None

===========================================================================*/

static int qti_wda_init(void)
{
  qmi_service_info qmi_svc_info;
  int ret_val;
  qmi_client_error_type err;
  qmi_idl_service_object_type qti_wda_service_object;

  memset(&qmi_svc_info, 0, sizeof(qmi_service_info));

  /*------------------------------------------------------------------------
  Register the USB tethered port with QCCI because we want WDA service on
  USB tethered data port.
  -------------------------------------------------------------------------*/
  err = qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_INSTANCE_8);
  if (err != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Error registering QCCI transport: %d", err,0,0);
  }
  else
  {
    LOG_MSG_INFO1("QMI CCI register successful",0,0,0);
  }

  /*------------------------------------------------------------------
  Get the service object, wait for the service and get the information
  about the service and then get an handle for the service.
  -------------------------------------------------------------------*/
  qti_wda_service_object = wda_get_service_object_v01();

  if(!qti_wda_service_object)
  {
    LOG_MSG_ERROR("qti wda get service object failed",0,0,0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("WDA service obj success",0,0,0);
  if(!qti_qmi_wait_for_srv_then_get_info(qti_wda_service_object, 
                                         &qmi_svc_info))
  {
    LOG_MSG_ERROR("Error getting info for QTI QMI service %d",0,0,0);
  }
  else
  {
    ret_val = qmi_client_init(&qmi_svc_info, 
                              qti_wda_service_object,
                              (void *)qti_qmi_wda_ind_cb, 
                              NULL,
                              NULL, 
                              &(qti_qmi_conf->qmi_qti_wda_handle));
    if(ret_val != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Error while trying to initialize client ",0,0,0);
    }
    else
    {
      LOG_MSG_INFO1("Successfully allocated client",0,0,0);
      return QTI_SUCCESS;
    }
  }
}

/*===========================================================================

FUNCTION QTI_WDS_INIT()

DESCRIPTION

  This function initializes WDS client handle

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS 
  None
===========================================================================*/

static int qti_wds_init(void)
{
  qmi_service_info qmi_svc_info;
  int ret_val;
  qmi_client_error_type err;
  qmi_idl_service_object_type qti_wds_service_object;

  memset(&qmi_svc_info, 0, sizeof(qmi_service_info));
/*- - - - - - - - - --  - - - - - - -- - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
  Register the USB tethered port with QCCI because we want WDA service on
  USB tethered data port.
  -------------------------------------------------------------------------*/
  err = qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_INSTANCE_8);
  if (err != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Error registering QCCI transport: %d", err,0,0);
  }
  else
  {
    LOG_MSG_INFO1("QMI CCI register successful",0,0,0);
  }

  LOG_MSG_INFO1("Entering WDS init",0,0,0);

  /*------------------------------------------------------------------
  Get the service object, wait for the service and get the information
  about the service and then get an handle for the service.
  -------------------------------------------------------------------*/
  qti_wds_service_object = wds_get_service_object_v01();

  if(!qti_wds_service_object)
  {
    LOG_MSG_ERROR("qti wds get service object failed",0,0,0);
    return QTI_FAILURE;
  }

  if(!qti_qmi_wait_for_srv_then_get_info(qti_wds_service_object, 
                                         &qmi_svc_info))
  {
    LOG_MSG_ERROR("Error getting info for QTI QMI service %d",0,0,0);
    return QTI_FAILURE;
  }
  else
  {
    /*----------------------------------------------------------------
    Get handle for IPv4
    ------------------------------------------------------------------*/
    ret_val = qmi_client_init(&qmi_svc_info,
                              qti_wds_service_object,
                              (void *) qti_qmi_wds_ind_cb,
                              NULL,
                              NULL,
                              &(qti_qmi_conf->qmi_qti_v4_wds_handle));
    if(ret_val != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Error while trying to initialize client ",0,0,0);
      return QTI_FAILURE;
    }
    else
    {
      LOG_MSG_INFO1("Successfully allocated client init WDS service for v4, handle %d",
                    qti_qmi_conf->qmi_qti_v4_wds_handle,0,0);
    }

    /*----------------------------------------------------------------------
    Get handle for IPV6
    -----------------------------------------------------------------------*/
    ret_val = qmi_client_init(&qmi_svc_info,
                              qti_wds_service_object,
                              (void *) qti_qmi_wds_ind_cb,
                              NULL, 
                              NULL, 
                              &(qti_qmi_conf->qmi_qti_v6_wds_handle));
    if(ret_val != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Error while trying to initialize client ",0,0,0);
      return QTI_FAILURE;
    }
    else
    {
      LOG_MSG_INFO1("Successfully allocated client for v6, handle %d",
                    qti_qmi_conf->qmi_qti_v6_wds_handle,0,0);
      return QTI_SUCCESS;
    }
  }
}

/*===========================================================================

FUNCTION QTI_QMI_INIT()

DESCRIPTION

  This function initializes QTI configuration structure.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS 
  None
===========================================================================*/

int qti_qmi_init
(
  qti_conf_t * qti_conf
)
{
  LOG_MSG_INFO1("Entering qti_qmi_init",0,0,0);
  /*----------------------------------------------------------------------
  Initialize QTI_QMI conf so that the QTI state is monitored in QTI_QMI also.
  ----------------------------------------------------------------------*/
  qti_qmi_conf = qti_conf;

  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_QMI_GET_MAX_AGG_SIZE()

DESCRIPTION

  This function obtains the max aggregation size parameter
  required for supporting RNDIS

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS 
  None
===========================================================================*/
void qti_qmi_get_max_agg_size()
{
  int fd;
/*- - - -- - - - - - - - -- - - - - - - - - - - - - - - - - - - - - - - - */

  LOG_MSG_INFO1("Entering qti_qmi_get_max_agg_size",0,0,0);
  /*-----------------------------------------------------------------------
  Obtain the max aggregation parameter required for RNDIS aggr
  ------------------------------------------------------------------------*/
  if ((fd=open("/dev/android_rndis_qc",O_RDWR)) < 0)
  {
    /*----------------------------------------------------------------
    This condition should not occur generally. Since an RTM_NEWLINK is
    already got for the interface, it means enumeration was fine. So,
    setting max pkt size to default value which gives max_datagrams as 1.
    ----------------------------------------------------------------*/
    data_aggregation_max_size = 2048;
    LOG_MSG_ERROR("rndis device file failed to open,"
                  "Using default Max Pkt size : %ld",
                  data_aggregation_max_size,0,0);
    return;
  }

  if (ioctl(fd,
            RNDIS_QC_GET_MAX_PKT_SIZE,
            &data_aggregation_max_size) < 0)
  {
    /*-----------------------------------------------------------
    No aggregation when the IOCTL fails. So, setting the max_size
    as 2Kbytes which gives max_datagrams as 1.
    -----------------------------------------------------------*/
    data_aggregation_max_size = 2048;
    LOG_MSG_ERROR("ioctl failed use default Max Pkt size: %ld",
                  data_aggregation_max_size,0,0);
    close(fd);
  }
  else
  {
    LOG_MSG_INFO1("ioctl success, Max Pkt size: %ld",
                  data_aggregation_max_size,0,0);
    close(fd);
  }

  return;
}

/*===========================================================================

FUNCTION QTI_QMI_CMD_EXEC()

DESCRIPTION

  This function performs the execution of commands present in command queue.
  It mainly is involved in sending required QMI messages to modem to perform
  QMI specific operations
 
DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS 
  None
/*=========================================================================*/

int qti_qmi_cmd_exec
(
  qti_qmi_event_e event, 
  qti_interface_e interface
)
{

  wda_set_data_format_req_msg_v01 wda01_req;
  wda_set_data_format_resp_msg_v01 wda01_resp;

  wds_start_network_interface_req_msg_v01 qmi_start_req;
  wds_start_network_interface_resp_msg_v01 qmi_start_resp;

  wds_set_client_ip_family_pref_req_msg_v01 qmi_set_v4_client_req;
  wds_set_client_ip_family_pref_resp_msg_v01 qmi_set_v4_client_resp;

  wds_set_client_ip_family_pref_req_msg_v01 qmi_set_v6_client_req;
  wds_set_client_ip_family_pref_resp_msg_v01 qmi_set_v6_client_resp;

  wds_stop_network_interface_req_msg_v01 qmi_stop_req;
  wds_stop_network_interface_resp_msg_v01 qmi_stop_resp;

  qmi_client_error_type qmi_err, err;

  int ret_val;
/*- - - - -- - - - - - - - - - - - - - - - - - - - - - --  - - - - - -*/
  LOG_MSG_INFO1("entered qti qmi cmd exec function",0,0,0);

  /* ------------------------------------------------------------------
     Do not process commands before QCMAP signals about LAN connection.
     This is done to perform synchronization between QCMAP and QTI
  -------------------------------------------------------------------*/
  if(qti_qmi_conf->qti_qcmap_proceed == 0 )
  {
    return QTI_SUCCESS;
  }
  /*-----------------------------------------------------------------------
  Handle different events and perform QMI specific operations
  -------------------------------------------------------------------------*/
  switch(event)
  {
    /*--------------------------------------------------------------------
    This event is processed when we get netlink up event upon USB cable 
    plug in. 
    - Here we signal QCMAP about the USB cable plug in. 
    - Send WDA set data format message to indicate to modem about aggr prot 
    - Send WDS start network interface for v4 and v6 to bring up call on 
    tethered data port. 
    ----------------------------------------------------------------------*/
    case QTI_LINK_UP_EVENT:
      if(qti_qmi_conf->state == QTI_LINK_DOWN_WAIT)
      {
        LOG_MSG_ERROR("QTI:Call already up.ignoring Link Up command",0,0,0);
        break;
      }

      LOG_MSG_INFO1("Calling QTI set DTR ",0,0,0);

      if(qti_set_dtr(SET_DTR_HIGH)<0)
      {
        LOG_MSG_ERROR("QTI failed to signal DTR high",0,0,0);
      }
      /*------------------------------------------------------------------- 
        Informing QCMAP about USB cable plug in
      ---------------------------------------------------------------------*/
      LOG_MSG_INFO1("Posting signal USR1 to QCMAP ConnectionManager",0,0,0);
      LOG_MSG_INFO1("pkill -USR1 QCMAP_ConnectionManager",0,0,0);
      ds_system_call("pkill -USR1 QCMAP_ConnectionManager", 
                     strlen("pkill -USR1 QCMAP_ConnectionManager"));
      /*------------------------------------------------------------------- 
        Calling qti_qmi_get_max_agg_size() to fetch the DL max pkt size
        got from the host PC in REMOTE_NDIS_INITIALIZE_MSG.MaxTransferSize
      ---------------------------------------------------------------------*/
      qti_qmi_get_max_agg_size();
      /*-------------------------------------------------------------------
      Prepare WDA set data format message
      ---------------------------------------------------------------------*/
      memset(&wda01_req, 0, sizeof(wda_set_data_format_req_msg_v01));
      wda01_req.qos_format_valid = TRUE;
      wda01_req.qos_format = FALSE;
      wda01_req.link_prot_valid = TRUE;
      wda01_req.link_prot = WDA_LINK_LAYER_ETHERNET_MODE_V01;
      if(interface == RNDIS_IF)
      {
        wda01_req.dl_data_aggregation_protocol_valid = TRUE;
        wda01_req.dl_data_aggregation_protocol = WDA_DL_DATA_AGG_RNDIS_ENABLED_V01;
        wda01_req.ul_data_aggregation_protocol_valid = TRUE;
        wda01_req.ul_data_aggregation_protocol = WDA_UL_DATA_AGG_RNDIS_ENABLED_V01;
        wda01_req.dl_data_aggregation_max_size_valid = TRUE;
        wda01_req.dl_data_aggregation_max_size = data_aggregation_max_size;
        /* Find max datagrams from the buffer size obtained */
        wda01_req.dl_data_aggregation_max_datagrams_valid = TRUE;
        wda01_req.dl_data_aggregation_max_datagrams =
                    data_aggregation_max_size/RNDIS_MAX_PKTS_DIVISION_FACTOR;
      }
      else if(interface == ECM_IF)
      {
        wda01_req.dl_data_aggregation_protocol_valid = TRUE;
        wda01_req.dl_data_aggregation_protocol = WDA_UL_DATA_AGG_DISABLED_V01;
        wda01_req.ul_data_aggregation_protocol_valid = TRUE;
        wda01_req.ul_data_aggregation_protocol = WDA_DL_DATA_AGG_DISABLED_V01;
      }

      /* ----------------------------------------------------------------------
         Initialize WDS client intializaton
       -----------------------------------------------------------------------*/
      ret_val = qti_wds_init();
      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("QTI WDS init failed",0,0,0);
        return QTI_FAILURE;
      }
      LOG_MSG_INFO1("WDS init success",0,0,0);

      /* ----------------------------------------------------------------------
         Initialize WDA client intializaton
      -----------------------------------------------------------------------*/
      ret_val = qti_wda_init();
      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize QTI WDA",0,0,0);
        return QTI_FAILURE;
      }

      LOG_MSG_INFO1("WDA init success",0,0,0);

      /*----------------------------------------------------------------------- 
       Send WDA set data format message
      ------------------------------------------------------------------------*/
      qmi_err = qmi_client_send_msg_sync(qti_qmi_conf->qmi_qti_wda_handle,
                                       QMI_WDA_SET_DATA_FORMAT_REQ_V01,
                                       &wda01_req,
                                       sizeof(wda01_req),
                                       &wda01_resp,
                                       sizeof(wda01_resp),
                                       QTI_QMI_TIMEOUT_VALUE);

      /*------------------------------------------------------------------------
      Release WDA client as it is no longer needed
      -------------------------------------------------------------------------*/
      err = qmi_client_release(qti_qmi_conf->qmi_qti_wda_handle);

      if (err != QMI_NO_ERR)
      {
        LOG_MSG_ERROR("Error %d while releasing client notifier handle", err,0,0);
      }

      if(qmi_err == QMI_NO_ERR && wda01_resp.resp.result == QMI_NO_ERR)
      {
        LOG_MSG_INFO1("Succeed WDA set data format",0,0,0);
        memset(&qmi_set_v4_client_req, 0, 
               sizeof(wds_set_client_ip_family_pref_req_msg_v01));

        /*----------------------------------------------------------------------
        Set IP family preference before bringing up the data call for V4 
        ------------------------------------------------------------------------*/ 
        qmi_set_v4_client_req.ip_preference = WDS_IP_FAMILY_IPV4_V01;
        qmi_err = qmi_client_send_msg_sync(qti_qmi_conf->qmi_qti_v4_wds_handle, 
                                           QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_REQ_V01,
                                           &qmi_set_v4_client_req,
                                           sizeof(qmi_set_v4_client_req), 
                                           &qmi_set_v4_client_resp,
                                           sizeof(qmi_set_v4_client_resp), 
                                           QTI_QMI_TIMEOUT_VALUE);
        if(qmi_err == QMI_NO_ERR && qmi_set_v4_client_resp.resp.result == QMI_NO_ERR)
        {
          LOG_MSG_INFO1("Succeed WDS set client for v4",0,0,0);
        }
        else
        {
          LOG_MSG_ERROR("Failed WDS set client for v4 error %d", qmi_err,0,0);
        }

        /*--------------------------------------------------------------------- 
          Call start network interface for V4
         ----------------------------------------------------------------------*/

        memset(&qmi_start_req, 0, sizeof(wds_start_network_interface_req_msg_v01));
        qmi_start_req.ip_family_preference_valid = TRUE;
        qmi_start_req.ip_family_preference = WDS_IP_FAMILY_PREF_IPV4_V01;

        if(qcmap_qti_msg.mtext[0] > 0)
        {
          qmi_start_req.technology_preference_valid = TRUE;
          qmi_start_req.technology_preference = qcmap_qti_msg.mtext[0];
        }

        if(qcmap_qti_msg.mtext[2] > 0)
        {
          qmi_start_req.profile_index_valid = TRUE;
          qmi_start_req.profile_index =qcmap_qti_msg.mtext[2];
        }

        if(qcmap_qti_msg.mtext[3] > 0)
        {
          qmi_start_req.profile_index_3gpp2_valid = TRUE;
          qmi_start_req.profile_index_3gpp2 =qcmap_qti_msg.mtext[3];
        }

        qmi_err = qmi_client_send_msg_sync(qti_qmi_conf->qmi_qti_v4_wds_handle,
                                           QMI_WDS_START_NETWORK_INTERFACE_REQ_V01,
                                           &qmi_start_req,
                                           sizeof(qmi_start_req),
                                           &qmi_start_resp,
                                           sizeof(qmi_start_resp),
                                           QTI_QMI_TIMEOUT_VALUE);

        if(qmi_err == QMI_NO_ERR && qmi_start_resp.resp.result == QMI_NO_ERR)
        {
          LOG_MSG_INFO1("Succeed WDS SNI for v4",0,0,0);
          qti_qmi_conf->qmi_qti_v4_wds_call_handle = qmi_start_resp.pkt_data_handle;
        }
        else
        {
          LOG_MSG_ERROR("Start network interface request failed for V4 error %d",qmi_err,0,0);
          return QTI_FAILURE;
        }

        /* Call SNI for V6 only if QCMAP ip family is V4V6 or V6 */
        if((qcmap_qti_msg.mtext[1] == QCMAP_QTI_IP_V6)||
           (qcmap_qti_msg.mtext[1] == QCMAP_QTI_IP_V4V6))
        {
          /*----------------------------------------------------------------------
          Set IP family preference before bringing up the data call for V6
          ------------------------------------------------------------------------*/ 
          memset(&qmi_set_v6_client_req, 0, sizeof(wds_set_client_ip_family_pref_req_msg_v01));
          qmi_set_v6_client_req.ip_preference = WDS_IP_FAMILY_IPV6_V01;
          qmi_err = qmi_client_send_msg_sync(qti_qmi_conf->qmi_qti_v6_wds_handle,
                                             QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_REQ_V01,
                                             &qmi_set_v6_client_req,
                                             sizeof(qmi_set_v6_client_req),
                                             &qmi_set_v6_client_resp,
                                             sizeof(qmi_set_v6_client_resp),
                                             QTI_QMI_TIMEOUT_VALUE);
          if(qmi_err == QMI_NO_ERR && qmi_set_v6_client_resp.resp.result == QMI_NO_ERR)
          {
            LOG_MSG_INFO1("Succeed WDS set client for v6",0,0,0);
          }
          else
          {
            LOG_MSG_ERROR("Failed WDS set client for v6 error %d", qmi_err,0,0);
          }

          /*--------------------------------------------------------------------- 
            Call start network interface for V6
          ----------------------------------------------------------------------*/
          memset(&qmi_start_req, 0, sizeof(wds_start_network_interface_req_msg_v01));
          qmi_start_req.ip_family_preference_valid = TRUE;
          qmi_start_req.ip_family_preference = WDS_IP_FAMILY_PREF_IPV6_V01;

          if(qcmap_qti_msg.mtext[0] > 0)
          {
            qmi_start_req.technology_preference_valid = TRUE;
            qmi_start_req.technology_preference = qcmap_qti_msg.mtext[0];
          }

          if(qcmap_qti_msg.mtext[4] > 0)
          {
            qmi_start_req.profile_index_valid = TRUE;
            qmi_start_req.profile_index = qcmap_qti_msg.mtext[4];
          }

          if(qcmap_qti_msg.mtext[5] > 0)
          {
            qmi_start_req.profile_index_3gpp2_valid = TRUE;
            qmi_start_req.profile_index_3gpp2 = qcmap_qti_msg.mtext[5];
          }
		  
          qmi_err = qmi_client_send_msg_sync(qti_qmi_conf->qmi_qti_v6_wds_handle,
                                             QMI_WDS_START_NETWORK_INTERFACE_REQ_V01,
                                             &qmi_start_req,
                                             sizeof(qmi_start_req),
                                             &qmi_start_resp,
                                             sizeof(qmi_start_resp),
                                             QTI_QMI_TIMEOUT_VALUE);
          if(qmi_err == QMI_NO_ERR && qmi_start_resp.resp.result == QMI_NO_ERR)
          {
            LOG_MSG_INFO1("Succeed WDS SNI for v6",0,0,0);
            qti_qmi_conf->qmi_qti_v6_wds_call_handle = qmi_start_resp.pkt_data_handle;
          }
          else
          {
            LOG_MSG_ERROR("Start network interface request failed for v6 error %d",
                          qmi_err,0,0);
            return QTI_FAILURE;
          }
        }
      }
      else
      {
        LOG_MSG_ERROR("WDA set data format failed",0,0,0);
        return QTI_FAILURE;
      }

      qti_qmi_conf->state=QTI_LINK_DOWN_WAIT;
      ds_system_call("echo QTI:LINK_DOWN_WAIT state > /dev/kmsg",
                     strlen("echo QTI:LINK_DOWN_WAIT state > /dev/kmsg"));
      break;

    /*------------------------------------------------------------------------
     Processes netlink down event which happens upon USB cable plug out
     - Here we inform QCMAP about USB cable plug out.
     - Perform stop network interface for V4 and V6 calls on USB tethered port
    --------------------------------------------------------------------------*/
    case QTI_LINK_DOWN_EVENT:
      if(qti_qmi_conf->state == QTI_LINK_UP_WAIT)
      {
        LOG_MSG_ERROR("QTI:Call already down.ignoring command",0,0,0);
        break;
      }

      if(qti_set_dtr(SET_DTR_LOW)<0)
      {
        LOG_MSG_ERROR("QTI failed to signal DTR high",0,0,0);
      }
      /*----------------------------------------------------------------------
        Signal QCMAP about USB cable plug out
      ----------------------------------------------------------------------*/
      LOG_MSG_INFO1("pkill -USR2 QCMAP_ConnectionManager",0,0,0);
      ds_system_call("pkill -USR2 QCMAP_ConnectionManager", 
                     strlen("pkill -USR2 QCMAP_ConnectionManager"));

      /*-----------------------------------------------------------------------
      Do stop network interface for V4
      ------------------------------------------------------------------------*/
      memset(&qmi_stop_req, 0, sizeof(wds_stop_network_interface_req_msg_v01));
      memset(&qmi_stop_resp, 0, sizeof(wds_stop_network_interface_resp_msg_v01));
      qmi_stop_req.pkt_data_handle = qti_qmi_conf->qmi_qti_v4_wds_call_handle;
      qmi_err = qmi_client_send_msg_sync(qti_qmi_conf->qmi_qti_v4_wds_handle,
                                         QMI_WDS_STOP_NETWORK_INTERFACE_REQ_V01,
                                         &qmi_stop_req,
                                         sizeof(qmi_stop_req),
                                         &qmi_stop_resp,
                                         sizeof(qmi_stop_resp),
                                         QTI_QMI_TIMEOUT_VALUE);
      if(qmi_err!= QMI_NO_ERR)
      {
        LOG_MSG_ERROR("Stop network interface failed for v4 error %d",qmi_err,0,0);
      }
      else
      {
        LOG_MSG_INFO1("Succeed Stop network interface for v4",0,0,0);
      }

      /*-----------------------------------------------------------------------
      Do stop network interface for V6
      ------------------------------------------------------------------------*/
      memset(&qmi_stop_req, 0, sizeof(wds_stop_network_interface_req_msg_v01));
      memset(&qmi_stop_resp, 0, sizeof(wds_stop_network_interface_resp_msg_v01));

      qmi_stop_req.pkt_data_handle = qti_qmi_conf->qmi_qti_v6_wds_call_handle;
      qmi_err = qmi_client_send_msg_sync(qti_qmi_conf->qmi_qti_v6_wds_handle,
                                         QMI_WDS_STOP_NETWORK_INTERFACE_REQ_V01,
                                         &qmi_stop_req,
                                         sizeof(qmi_stop_req),
                                         &qmi_stop_resp,
                                         sizeof(qmi_stop_resp),
                                         QTI_QMI_TIMEOUT_VALUE);
      if(qmi_err!= QMI_NO_ERR)
      {
        LOG_MSG_ERROR("Stop network interface failed for v6 error %d",qmi_err,0,0);
      }
      else
      {
        LOG_MSG_INFO1("Succeed Stop network interface for v6",0,0,0);
      }

      /*---------------------------------------------------------------------------
       Free WDS clients.
       //TODO: Free WDS clients based on the disconnected indication received
       from WDS service
      -----------------------------------------------------------------------------*/
      qti_qmi_free();
      qti_qmi_conf->state = QTI_LINK_UP_WAIT;
      ds_system_call("echo QTI:LINK_UP_WAIT state > /dev/kmsg",
                     strlen("echo QTI:LINK_UP_WAIT state > /dev/kmsg"));
      break;

    default:
      LOG_MSG_INFO1("Ignoring event %d received",event,0,0);
      break;
  }

  LOG_MSG_INFO1("Succeed handle QMI event",0,0,0);
  return QTI_SUCCESS;
}

int qti_qmi_rcv_msg(void)
{
  int msgqid;
  key_t key;

  memset(&qcmap_qti_msg, 0, sizeof(qcmap_qti_msg_t));

  /* Key obtained should be the same as that calculated in QCMAP_CM */
  if((key = ftok("/usr/bin/QCMAP_ConnectionManager", QCMAP_QTI_KEY_ID)) == -1) 
  {
    LOG_MSG_ERROR("Error generating key", 0,0,0);
    return QTI_FAILURE;
  }

  if((msgqid = msgget(key, 0666)) == -1) 
  {
    LOG_MSG_ERROR("Error getting the message queue", 0,0,0);
    return QTI_FAILURE;
  }
  

  /* Dequeue the message from the queue using the msgqid, and msg type
     msgrcv(qid, msg_ptr, payload size, mtype, flags) */
  if(msgrcv(msgqid, &qcmap_qti_msg, QCMAP_QTI_MSG_SIZE, QCMAP_QTI_MTYPE_ID, 0) == -1) 
  {
    LOG_MSG_ERROR("Error receiving the message", 0,0,0);
    return QTI_FAILURE;
  }

  /*	Remove the message queue after successfully dequeuing message */
  if(msgctl(msgqid, IPC_RMID, NULL) == -1)
  {
    LOG_MSG_ERROR("Error removing the message queue",0,0,0);
  }

  LOG_MSG_INFO1("Tech preference = %d ; "
                "IP family = %d; "
                "V4 UMTS PROFILE ID = %d; ",
                qcmap_qti_msg.mtext[0],
                qcmap_qti_msg.mtext[1],
                qcmap_qti_msg.mtext[2]);
  LOG_MSG_INFO1("V4 CDMA PROFILE ID = %d ; ",
                qcmap_qti_msg.mtext[3],0,0);

  if((qcmap_qti_msg.mtext[1] == QCMAP_QTI_IP_V6)||
     (qcmap_qti_msg.mtext[1] == QCMAP_QTI_IP_V4V6))
  {
    LOG_MSG_INFO1("V6 UMTS PROFILE ID = %d; "
                  "V6 CDMA PROFILE ID = %d; ",
                  qcmap_qti_msg.mtext[4],
                  qcmap_qti_msg.mtext[5],0);
  }

  return QTI_SUCCESS;
}
