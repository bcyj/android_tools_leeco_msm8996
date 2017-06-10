#ifndef _DS_QMI_QCMAP_MSGR_H
#define _DS_QMI_QCMAP_MSGR_H
/*===========================================================================

                         D S _ Q M I _ Q C M A P _ M S G R . H

DESCRIPTION

 The Data Services QMI Qualcomm Mobile Access Point Messenger header file.

EXTERNALIZED FUNCTIONS

   qmi_qcmap_msgr_init()
     Register the QCMAP service with QMUX for all applicable QMI links

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/
/*===========================================================================

when        who    what, where, why
--------    ---    ----------------------------------------------------------
07/11/12    gk     Created module
===========================================================================*/

#include "comdef.h"
#include "qmi_csi.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "QCMAP_ConnectionManager.h"
#include "qcmap_cm_api.h"

#define QCMAP_MSGR_MAX_CLIENT_HANDLES (5)

typedef void (*qcmap_msgr_wwan_ind_cb_fcn)
(
  void                           *user_data,              /* Call back User data */
  qcmap_msgr_wwan_status_enum_v01      conn_status,             /* Connection Status enum */
  qcmap_msgr_wwan_call_end_type_enum_v01 call_end,
  int call_end_reason_code 
);

typedef void (*qcmap_msgr_qcmap_ind_cb_fcn)
(
  void                           *user_data,              /* Call back User data */
  qcmap_msgr_mobile_ap_status_enum_v01      conn_status             /* Connection Status enum */
);

typedef void (*qcmap_msgr_station_mode_ind_cb_fcn)
(
  void                           *user_data,              /* Call back User data */
  qcmap_msgr_station_mode_status_enum_v01 conn_status              /* Connection Status enum */
);

typedef void (*qcmap_msgr_cradle_mode_ind_cb_fcn)
(
  void                           *user_data,              /* Call back User data */
  qcmap_msgr_cradle_status_enum_v01 conn_status              /* Connection Status enum */
);

/*---------------------------------------------------------------------------
  QMI QCMAP CM Service state info
---------------------------------------------------------------------------*/
typedef struct
{
  qmi_csi_service_handle service_handle;
  int                  num_clients;
  void*                client_handle_list[QCMAP_MSGR_MAX_CLIENT_HANDLES];
  int                  qmi_instance;
  int                  client_ref_count;
} qmi_qcmap_msgr_state_info_type;

/*---------------------------------------------------------------------------
  status callback data
---------------------------------------------------------------------------*/
typedef struct
{
  qmi_qcmap_msgr_state_info_type *svc_cb;
  unsigned int                 map_instance;
  qcmap_msgr_wwan_ind_cb_fcn     wwan_cb;
  qcmap_msgr_qcmap_ind_cb_fcn    qcmap_cb;
  qcmap_msgr_station_mode_ind_cb_fcn      sta_cb;
  qcmap_msgr_cradle_mode_ind_cb_fcn      cradle_cb;
}qmi_qcmap_msgr_status_cb_data;


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_INIT()

  DESCRIPTION
    Register the QCMAP CM service with QMUX for all applicable QMI links

  PARAMETERS
    None

  RETURN VALUE
    None

  DEPENDENCIES
    None

  SIDE EFFECTS
    None
===========================================================================*/
extern int  qmi_qcmap_msgr_init
(
  void
);

typedef struct
{
  int                           handle;
  qcmap_msgr_wwan_status_enum_v01     wwan_status;
  qmi_qcmap_msgr_status_cb_data     *cb_ptr;
  QCMAP_ConnectionManager           *Mgr;
} qmi_qcmap_msgr_softap_handle_type;
#endif /* _DS_QMI_QCMAP_MSGR_H */
