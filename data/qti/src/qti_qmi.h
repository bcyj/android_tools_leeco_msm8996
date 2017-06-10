
/******************************************************************************

                        QTI_QMI.H

******************************************************************************/

/******************************************************************************

  @file    qti_qmi.h
  @brief   Qualcomm Tethering Interface QMI Messaging module

  DESCRIPTION
  Header file for QTI QMI messaging functions.

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
10/03/12   mp         Fix to make QTI use QCMAP WWAN config.
06/29/12   sc         Revised version 
05/24/12   sb         Initial version

******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_addr.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include "comdef.h"
#include "ds_util.h"
#include "qti_netlink.h"

#define QTI_QMI_TIMEOUT_VALUE 500
#define QCMAP_QTI_WWAN_PARAMS 6

typedef enum
{
  QTI_LINK_UP_EVENT =1,
  QTI_LINK_DOWN_EVENT
} qti_qmi_event_e;

typedef enum
{
  RNDIS_IF = 0,
  ECM_IF
} qti_interface_e;

typedef struct
{
  long mtype;
  int  mtext[QCMAP_QTI_WWAN_PARAMS];
} qcmap_qti_msg_t;

/*===========================================================================
  FUNCTION qti_qmi_init
===========================================================================*/
/*!
@brief
  Initialize qti qmi-message setup
@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_qmi_init
(
  qti_conf_t * qti_conf
);

/*===========================================================================
  FUNCTION qti_qmi_cmd_exec
===========================================================================*/
/*!
@brief
  Sending out QMI message to modem
@return
  QTI_FAILURE or QTI_SUCCESS 

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_qmi_cmd_exec
(
  qti_qmi_event_e event,
  qti_interface_e interface
);

/*===========================================================================
  FUNCTION qti_qmi_rcv_msg
===========================================================================*/
/*!
@brief
  Receives the message queued by QCMAP for fetching the WWAN params
@return
  QTI_FAILURE or QTI_SUCCESS 

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_qmi_rcv_msg(void);