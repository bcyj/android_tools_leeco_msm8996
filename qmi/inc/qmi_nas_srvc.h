#ifndef QMI_NAS_SRVC_H
#define QMI_NAS_SRVC_H
/******************************************************************************
  @file    qmi_nas_srvc.h
  @brief   QMI message library NAS service definitions

  DESCRIPTION
  This file contains common, external header file definitions for QMI
  interface library.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_nas_srvc_init_client() must be called to create one or more clients
  qmi_nas_srvc_release_client() must be called to delete each client when 
  finished.

  $Header: //source/qcom/qct/modem/datacommon/qmimsglib/dev/work/inc/qmi_nas_srvc.h#2 $ 
  $DateTime: 2009/07/15 10:38:12 $
  ---------------------------------------------------------------------------
  Copyright (c) 2007 - 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#include "qmi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define     QMI_NAS_MAX_RADIO_IFACES    9
#define     QMI_NAS_MAX_STR_LEN         128

typedef enum
{
  QMI_NAS_NO_SERVICE        = 0x00,
  QMI_NAS_CDMA_20001x       = 0x01,
  QMI_NAS_CDMA_2000_HRPD    = 0x02,
  QMI_NAS_AMPS              = 0x03,
  QMI_NAS_GSM               = 0x04,
  QMI_NAS_UMTS              = 0x05,
  QMI_NAS_WLAN              = 0x06,
  QMI_NAS_GPS               = 0x07,
  QMI_NAS_LTE               = 0x08
}qmi_nas_radio_interface;

/*Event Report Signal Strength information*/
typedef struct
{
  char              sig_strength;
  qmi_nas_radio_interface  radio_if;

}qmi_nas_event_report_signal_strength;

/*Get serving System Related*/
typedef enum
{
  QMI_NAS_NOT_REGISTERED            = 0,
  QMI_NAS_REGISTERED                = 1,
  QMI_NAS_NOT_REGISTERED_SEARCHING  = 2,
  QMI_NAS_REGISTRATION_DENIED       = 3,
  QMI_NAS_REGISTRATION_UNKNOWN      = 4
}qmi_nas_registration_state;

typedef enum
{
  QMI_NAS_CS_ATTACH_UNKNOWN_OR_NOTAPPLICABLE    = 0,
  QMI_NAS_CS_ATTACHED   = 1,
  QMI_NAS_CS_DETACHED   = 2
}qmi_nas_cs_attach_state;

typedef enum
{
  QMI_NAS_PS_ATTACH_UNKNOWN_OR_NOTAPPLICABLE    = 0,
  QMI_NAS_PS_ATTACHED   = 1,
  QMI_NAS_PS_ATTACH = QMI_NAS_PS_ATTACHED,
  QMI_NAS_PS_DETACHED   = 2,
  QMI_NAS_PS_DETACH = QMI_NAS_PS_DETACHED
}qmi_nas_ps_attach_state;

typedef enum
{
  QMI_NAS_REG_NETWORK_UNKNOWN   = 0x00,
  QMI_NAS_REG_NETWORK_3GPP2     = 0x01,
  QMI_NAS_REG_NETWORK_3GPP      = 0x02
}qmi_nas_reg_network_type;

typedef enum
{
  QMI_NAS_DS_CAPABILITY_NONE = 0x00,
  QMI_NAS_DS_CAPABILITY_GPRS = 0x01,
  QMI_NAS_DS_CAPABILITY_EDGE = 0x02,
  QMI_NAS_DS_CAPABILITY_HSDPA = 0x03,   
  QMI_NAS_DS_CAPABILITY_HSUPA = 0x04,
  QMI_NAS_DS_CAPABILITY_WCDMA = 0x05,
  QMI_NAS_DS_CAPABILITY_MAX = QMI_NAS_DS_CAPABILITY_WCDMA
} qmi_nas_ds_capability_type;


#define QMI_NAS_ROAMING_INDICATOR_PARAM_TYPE    0x0001
#define QMI_NAS_CURRENT_PLMN_PARAM_TYPE         0x0002
#define QMI_NAS_DS_CAPABILITY_PARAM_TYPE         0x0004

typedef struct
{
  /*Used to Validate Optional 
  Information that is returned in this structure*/
  unsigned short                param_mask;

  qmi_nas_registration_state    reg_state;
  qmi_nas_cs_attach_state       cs_attach_state;
  qmi_nas_ps_attach_state       ps_attach_state;
  qmi_nas_reg_network_type      network_type;
  short                         num_radio_interfaces;
  qmi_nas_radio_interface       radio_if[QMI_NAS_MAX_RADIO_IFACES];

  /*Optional Information*/
  unsigned char                 roaming_indicator;
  struct
  {
    int               mobile_country_code;
    int               mobile_network_code;
    unsigned char     network_desc[QMI_NAS_MAX_STR_LEN];
  }current_plmn;

  struct
  {
    unsigned char               num_capabilities;
    qmi_nas_ds_capability_type  capabilities[QMI_NAS_DS_CAPABILITY_MAX];
  }ds_capabilites; 

}qmi_nas_serving_system_info_type;



/* For turning event reporting on/off */
typedef enum
{
  QMI_NAS_EVENT_REPORTING_OFF = 0,
  QMI_NAS_EVENT_REPORTING_ON = 1
} qmi_nas_event_report_state_type; 


/* Distinguishes indication message types */

typedef enum
{
  QMI_NAS_SRVC_INVALID_IND_MSG,
  QMI_NAS_SRVC_EVENT_REPORT_IND_MSG,
  QMI_NAS_SRVC_SERVING_SYSTEM_IND_MSG
  /* To be filled in in future release */
} qmi_nas_indication_id_type;


/* Async notification reporting structure */
typedef union
{
  qmi_nas_event_report_signal_strength  signal_strength;
  qmi_nas_serving_system_info_type      serving_system;
} qmi_nas_indication_data_type;


typedef void (*qmi_nas_indication_hdlr_type)
( 
  int                           user_handle,
  qmi_service_id_type           service_id,
  void                          *user_data,
  qmi_nas_indication_id_type    ind_id,
  qmi_nas_indication_data_type  *ind_data
);


/* For turning indications on/off */
typedef enum
{
  QMI_NAS_INDICATIONS_DISABLE = 0,
  QMI_NAS_INDICATIONS_ENABLE  = 1
} qmi_nas_indication_state_type; 

#define QMI_NAS_SYS_SEL_PREF_IND     0x0001
#define QMI_NAS_DDTM_EVENTS_IND      0x0002
#define QMI_NAS_SERVING_SYSTEM_IND   0x0004

typedef struct
{
  /*Used to Validate Optional 
  Information that is returned in this structure*/
  unsigned short                  param_mask;

  qmi_nas_indication_state_type   reg_sys_sel_pref;
  qmi_nas_indication_state_type   reg_ddtm_events;
  qmi_nas_indication_state_type   reg_serving_system;
}qmi_nas_indication_register_info_type;


/*===========================================================================
  FUNCTION  qmi_nas_srvc_init_client
===========================================================================*/
/*!
@brief 
  This function is called to initialize the NAS service.  This function
  must be called prior to calling any other NAS service functions.
  For the time being, the indication handler callback and user data
  should be set to NULL until this is implemented.  Also note that this
  function may be called multiple times to allow for multiple, independent
  clients.   
  
@return 
  0 if abort operation was sucessful, < 0 if not.  If return code is 
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will 
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/    
/*=========================================================================*/
EXTERN qmi_client_handle_type
qmi_nas_srvc_init_client
(
  const char                    *dev_id,
  qmi_nas_indication_hdlr_type  user_ind_msg_hdlr,
  void                          *user_ind_msg_hdlr_user_data,
  int                           *qmi_err_code
);



/*===========================================================================
  FUNCTION  qmi_nas_srvc_release_client
===========================================================================*/
/*!
@brief 
  This function is called to release a client created by the 
  qmi_nas_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has 
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.  
  
@return 
  0 if operation was sucessful, < 0 if not.  If return code is 
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will 
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/    
/*=========================================================================*/
EXTERN int 
qmi_nas_srvc_release_client
(
  int      user_handle,
  int      *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_nas_set_event_report_state
===========================================================================*/
/*!
@brief 
  Set the NAS event reporting state
     
  
@return 

@note

  - Dependencies
    - qmi_nas_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/    
/*=========================================================================*/
EXTERN int
qmi_nas_set_event_report_state
(
  int                               client_handle,
  qmi_nas_event_report_state_type   report_state,
  unsigned char                     num_of_signal_strength_thresholds,
  char                              *thresholds_list,
  int                               *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_nas_get_serving_system
===========================================================================*/
/*!
@brief 
  This message queries for information on the system that is currently 
  providing service.
     
@return 

@note

  - Dependencies
    - qmi_nas_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/    
/*=========================================================================*/
EXTERN int
qmi_nas_get_serving_system
(
  int                               client_handle,
  qmi_nas_serving_system_info_type  *resp_data,
  int                               *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_nas_initiate_ps_attach_detach
===========================================================================*/
/*!
@brief 
  This function is used to initiate a PS domain attach or detach.
     
@return 

@note

  - Dependencies
    - qmi_nas_srvc_init_client() must be called before calling this.

  - Side Effects
    - Attach or Detach is initiated
*/    
/*=========================================================================*/
EXTERN int
qmi_nas_initiate_ps_attach_detach
(
  int                         client_handle,
  qmi_nas_ps_attach_state     attach_detach,
  int                               *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_nas_indication_register
===========================================================================*/
/*!
@brief 
  Set the NAS indication registration state for specified control point.
     
  
@return 

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/    
/*=========================================================================*/
int
qmi_nas_indication_register
(
  int                                     client_handle,
  qmi_nas_indication_register_info_type  *ind_state,
  int                                    *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_nas_indication_register_all
===========================================================================*/
/*!
@brief 
  Set the NAS indication registration state for all active control points.
     
  
@return 

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/    
/*=========================================================================*/
int
qmi_nas_indication_register_all
(
  qmi_nas_indication_register_info_type  *ind_state,
  int                                    *qmi_err_code
);


#ifdef __cplusplus
}
#endif

#endif   /*QMI_NAS_SRVC.H*/
