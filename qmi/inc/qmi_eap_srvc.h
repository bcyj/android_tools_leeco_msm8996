#ifndef QMI_EAP_SRVC_H
#define QMI_EAP_SRVC_H
/******************************************************************************
  @file    qmi_eap_srvc.h
  @brief   QMI message library EAP service definitions

  DESCRIPTION
  This file contains common, external header file definitions for QMI
  interface library.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_eap_srvc_init_client() must be called to create one or more clients
  qmi_eap_srvc_release_client() must be called to delete each client when 
  finished.

  $Header: //source/qcom/qct/modem/datacommon/qmimsglib/dev/work/inc/qmi_eap_srvc.h#1 $ 
  $DateTime: 2009/07/15 10:38:12 $
  ---------------------------------------------------------------------------
  Copyright (c) 2007 - 2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi.h"

#ifdef __cplusplus
extern "C" {
#endif



/*Maximum available message length.This defines the max length
of a client's packet in request/response. */
#define  QMI_EAP_MAX_PKT_LEN     1900
#define  QMI_EAP_MAX_STR_LEN      128

typedef enum
{
  QMI_EAP_SRVC_INVALID_IND_MSG,
  QMI_EAP_SRVC_SESSION_RESULT_IND_MSG,
  QMI_EAP_AKA_RESULT_IND_MSG,
  QMI_EAP_NOTIF_CODE_IND_MSG,
  QMI_EAP_ERROR_CODE_IND_MSG,
  QMI_EAP_AUTH_REJ_IND_MSG,
  /* To be filled in in future release */
} qmi_eap_indication_id_type;


typedef enum
{
  QMI_EAP_AUTHENTICATION_SUCCESS   = 0x00,
  QMI_EAP_AUTHENTICATION_FAILURE   = 0x01,
} qmi_eap_auth_result_ind_param_type;

typedef enum
{
  QMI_EAP_INITIATE_AKA_ALGORITHM_SUCCESS      = 0,
  QMI_EAP_INITIATE_AKA_ALGORITHM_SYNC_FAILURE = 1,
  QMI_EAP_INITIATE_AKA_ALGORITHM_FAILURE      = 2,
}qmi_eap_initiate_aka_algorithm_status_type;

#define QMI_EAP_AKA_V1_OR_V2_AUTH_RESP_PARAMS      0x01   

typedef union
{
  qmi_eap_auth_result_ind_param_type      auth_result;

  struct
  {
    unsigned long                               aka_handle;
    qmi_eap_initiate_aka_algorithm_status_type  status;

    /* Bitmask that indicates if the following field is valid*/
    unsigned char param_mask;
    
    struct
    {
      unsigned char digest_len;
      unsigned char digest[QMI_EAP_MAX_STR_LEN];
      unsigned char aka_data_len;
      unsigned char aka_data[QMI_EAP_MAX_STR_LEN];
    }v1_or_v2_auth_params;
  }aka_result;

  unsigned short  eap_notif_code;
  unsigned short  eap_err_code;

}qmi_eap_indication_data_type;

typedef void (*qmi_eap_indication_hdlr_type)
( 
  int                           user_handle,
  qmi_service_id_type           service_id,
  void                          *user_data,
  qmi_eap_indication_id_type    ind_id,
  qmi_eap_indication_data_type  *ind_data
);

typedef enum
{
  QMI_EAP_AKA_ALGORITHM_VERSION_1 = 0,
  QMI_EAP_AKA_ALGORITHM_VERSION_2 = 1,
}qmi_eap_aka_algorithm_version;


#define QMI_EAP_AKA_V1_OR_V2_AUTH_PARAMS      0x01   

typedef struct
{

  qmi_eap_aka_algorithm_version aka_version;

  /* Bitmask that indicates if the following field is valid*/

  unsigned char param_mask;
  
  struct
  {
    unsigned char rand_len;
    unsigned char rand[QMI_EAP_MAX_STR_LEN];
    unsigned char auth_len;
    unsigned char auth[QMI_EAP_MAX_STR_LEN];
  }v1_or_v2_auth_params;
  
}qmi_eap_initiate_aka_algorithm_type;


typedef enum
{
  QMI_EAP_SEND_EAP_PKT_RSP_ID
}qmi_eap_async_rsp_id_type;

typedef union
{
  struct
  {
    void                *resp_data; /* Pointer to the Response Packet*/
    unsigned long       length;     /*Length of the Response Packet*/
  }eap_send_pkt_resp;

}qmi_eap_async_rsp_data_type;

/* EAP service user async callback function prototype.  The callback that is 
** registered with any WDS service asynchronous function and will be called
** when the QMI reply is received
*/ 
typedef  void (*qmi_eap_user_async_cb_type) 
(
  int                          user_handle,
  qmi_service_id_type          service_id,
  int                          sys_err_code,
  int                          qmi_err_code,
  void                         *user_data,
  qmi_eap_async_rsp_id_type    rsp_id,
  qmi_eap_async_rsp_data_type  *rsp_data
);

/*===========================================================================
  FUNCTION  qmi_eap_srvc_init_client
===========================================================================*/
/*!
@brief 
  This function is called to initialize the EAP service.  This function
  must be called prior to calling any other EAP service functions.
  For the time being, the indication handler callback and user data
  should be set to NULL until this is implemented.  Also note that this
  function may be called multiple times to allow for multiple, independent
  clients.   
  
@return 
  Returns a user handle if success, This user handle can be used for other 
  operations using this service.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/    
/*=========================================================================*/

EXTERN qmi_client_handle_type
qmi_eap_srvc_init_client
(
  const char                    *dev_id,
  qmi_eap_indication_hdlr_type  user_ind_msg_hdlr,
  void                          *user_ind_msg_hdlr_user_data,
  int                           *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_eap_srvc_release_client
===========================================================================*/
/*!
@brief 
  This function is called to release a client created by the 
  qmi_eap_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has 
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.  
  
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

EXTERN int 
qmi_eap_srvc_release_client
(
  int      user_handle,
  int      *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_eap_reset
===========================================================================*/
/*!
@brief
  Resets EAP service. This command resets the state of the requesting
  control point. It clears all the resources that were set up for the EAP
  session started by the control point.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give
  you the QMI error reason.  Otherwise, qmi_err_code will have meaningless
  data.

@note

  - Dependencies
    - qmi_connection_init() must be called before calling this.

  - Side Effects
    - Resets EAP service
*/
/*=========================================================================*/
EXTERN int
qmi_eap_reset
(
  int                         client_handle,
  int                         *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_eap_auth_start_eap_session
===========================================================================*/
/*!
@brief 
    This function is used to start the EAP session. This takes an optional 
    eap methods mask parameter. These are the EAP methods which can be
     supported.
  
@return 
  0 if abort operation was sucessful, < 0 if not.  If return code is 
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will 
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/    
/*=========================================================================*/

/*Set the eap_method_mask to one or more of the following methods
  Set eap_method_mask to QMI_EAP_METHOD_MASK_UNSET if not using the mask.*/

#define QMI_EAP_METHOD_MASK_UNSET      0x00000000
#define QMI_EAP_SIM_METHOD_MASK        0x00000001
#define QMI_EAP_AKA_METHOD_MASK        0x00000002
#define QMI_EAP_AKA_PRIME_METHOD_MASK  0x00000004

EXTERN int
qmi_eap_auth_start_eap_session
(
  int                           user_handle,
  unsigned long                 eap_method_mask, /* This parameter is optional, 
                                                  set this to QMI_EAP_METHOD_MASK_UNSET if not using 
                                                  the method_mask*/
  int                           *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_eap_auth_start_eap_session_ex
===========================================================================*/
/*!
@brief 
    This function is used to start the EAP session. It takes multiple optional
    EAP paramters.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
typedef enum
{
  QMI_EAP_AKA_ALGO_NONE      = 0x0000,
  QMI_EAP_AKA_ALGO_SHA1      = 0x0001,
  QMI_EAP_AKA_ALGO_MILENAGE  = 0x0002,
  QMI_EAP_AKA_ALGO_CAVE      = 0x0003,
  QMI_EAP_SIM_ALGO_GSM       = 0x0004,
  QMI_EAP_SIM_ALGO_USIM_GSM  = 0x0005,
  QMI_EAP_SIM_AKA_ALGO_MAX
} qmi_eap_sim_aka_algo_enum_type;

/* Parameter indication bits */
#define QMI_EAP_AUTH_START_EAP_METHOD_MASK_PARAM                      0x00000001
#define QMI_EAP_AUTH_START_EAP_USER_ID_PARAM                          0x00000002
#define QMI_EAP_AUTH_START_EAP_META_ID_PARAM                          0x00000004
#define QMI_EAP_AUTH_START_EAP_SIM_AKA_ALGO_PARAM                     0x00000008

typedef struct
{
  /* Bitmask which indicates which of the below
  ** parameters has been set
  */

  unsigned long params_mask;

  /* Parameters, more will be added later */
  unsigned long                    eap_method_mask;
  unsigned char                    user_id_len;
  unsigned char                    *user_id;
  unsigned char                    eap_meta_id_len;
  unsigned char                    *eap_meta_id;
  qmi_eap_sim_aka_algo_enum_type   eap_sim_aka_algo;
} qmi_eap_auth_start_eap_params_type;

EXTERN int
qmi_eap_auth_start_eap_session_ex
(
  int                                   user_handle,
  qmi_eap_auth_start_eap_params_type   *eap_auth_start,
  int                                  *qmi_err_code
);

/* QMI_AUTH_INDICATION_REGISTER related defines */
#define QMI_AUTH_EAP_NOTIF_CODE_REG_PARAM_MASK 0x00000001
#define QMI_AUTH_EAP_ERROR_CODE_REG_PARAM_MASK 0x00000002
#define QMI_AUTH_EAP_AUTH_REJ_REG_PARAM_MASK   0x00000004

typedef struct
{
  /* Bitmask which indicates which of the below
  ** parameters has been set
  */
  unsigned int   param_mask;

  /* Valid if QMI_AUTH_EAP_NOTIF_CODE_REG_PARAM_MASK is set */
  unsigned char  report_eap_notif_code;
  /* Valid if QMI_AUTH_EAP_ERROR_CODE_REG_PARAM_MASK is set */
  unsigned char  report_eap_err_code;
  /* Valid if QMI_AUTH_EAP_AUTH_REJ_REG_PARAM_MASK is set */
  unsigned char  report_eap_auth_reject;

} qmi_auth_indication_reg_type;

/*===========================================================================
  FUNCTION  qmi_eap_auth_send_eap_packet
===========================================================================*/
/*!
@brief 
    This function is used to send an eap request packet, and receive and 
    EAP response packet in reply.
  
@return 
  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which 
  can be used to cancel the transaction via the qmi_eap_abort_txn() command. 

@note
    This function executes asynchronously, User should register a callback
    function to be called when this service layer receives a response for 
    this request.

    If the user doesnot a asyn response for this request, qmi_eap_abort_txn()
    should be used to abort the request transaction.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/    
/*=========================================================================*/

EXTERN int
qmi_eap_auth_send_eap_packet
(
  int                           user_handle,
  qmi_eap_user_async_cb_type    user_cb,              /*User call back function*/
  void                          *user_data,           /*User data*/
  void                          *eap_request_packet,  /*Max EAP Req Message size is  QMI_EAP_MAX_PKT_LEN*/
  int                           request_pkt_length,   /*Set it to the length of the request packet.*/
  int                           *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_eap_auth_end_eap_session
===========================================================================*/
/*!
@brief 
    This function is used to end an eap session 
  
@return 
  0 if abort operation was sucessful, < 0 if not.  If return code is 
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will 
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/    
/*=========================================================================*/
EXTERN int
qmi_eap_auth_end_eap_session
(
  int                           user_handle,
  int                           *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_eap_auth_get_session_keys
===========================================================================*/
/*!
@brief 
    This function is used to query the session keys. This message should be
    used by the user after receiving the EAP session SUCCESS indiction.
  
@return 
  0 if abort operation was sucessful, < 0 if not.  If return code is 
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will 
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/    
/*=========================================================================*/
EXTERN int 
qmi_eap_auth_get_session_keys
(
  int                           user_handle,
  void                          *eap_keys_resp_packet,  /* Max EAP Resp Message size is  QMI_EAP_MAX_PKT_LEN*/
  int                           resp_pkt_length,        /*Set this to the length of the response buffer size.*/
  int                           *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_eap_delete_async_txn
===========================================================================*/
/*!
@brief 
    This function is used to cancel an asynchronous transaction.  Note
    that there is an inherent race condition in that an asynchronous response
    may be in the process of being processed when this is called, and may 
    still show up after this function is sucessfully called.
  
@return 
  QMI_NO_ERR if operation is successful, QMI_INTERNAL_ERR if not.

@note
    

  - Side Effects: Cancels async transaction 
*/    
/*=========================================================================*/
EXTERN int
qmi_eap_delete_async_txn
(
  int user_handle,
  int async_txn_handle
);

/*===========================================================================
  FUNCTION  qmi_eap_auth_initiate_aka_algorithm
===========================================================================*/
/*!
@brief 
    This command initiates the AKA algorithm.
  
@return 
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is 
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will 
  indicate which QMI error occurred.  

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_eap_srvc_init_client() must be called for the associated port first.

  - Side Effects
*/    
/*=========================================================================*/
EXTERN int 
qmi_eap_auth_initiate_aka_algorithm
(
  int                                    user_handle,
  qmi_eap_initiate_aka_algorithm_type   *aka_algorithm,
  unsigned long                         *aka_handle,
  int                                   *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_auth_set_subscription_binding
===========================================================================*/
/*!
@brief
    This command associates the client with the requested subscription

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_eap_srvc_init_client() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/

typedef enum {
  QMI_AUTH_SUBS_TYPE_PRIMARY   = 0x0001, /* Primary */
  QMI_AUTH_SUBS_TYPE_SECONDARY = 0x0002, /* Secondary */
  QMI_AUTH_SUBS_TYPE_TERTIARY  = 0x0003, /* Tertiary */
  QMI_AUTH_SUBS_TYPE_MAX
} qmi_auth_subscription_type;

EXTERN int
qmi_auth_set_subscription_binding
(
  int                          user_handle,
  qmi_auth_subscription_type   subscription,
  int                          *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_auth_get_bind_subscription
===========================================================================*/
/*!
@brief
    This command queries the current subscription associated with the client

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_eap_srvc_init_client() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
EXTERN int
qmi_auth_get_bind_subscription
(
  int                          user_handle,
  qmi_auth_subscription_type   *subscription,
  int                          *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_auth_indication_register
===========================================================================*/
/*!
@brief
  Register/deregister for different QMI AUTH indications. Indications include
  QMI_AUTH_EAP_NOTIFICATION_CODE_IND, QMI_AUTH_EAP_ERROR_CODE_IND,
  QMI_AUTH_EAP_AUTH_REJ_IND

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_eap_srvc_init_client() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
EXTERN int
qmi_auth_indication_register
(
  int                           user_handle,
  qmi_auth_indication_reg_type  *ind_reg,
  int                           *qmi_err_code
);

#ifdef __cplusplus
}
#endif

#endif   /*QMI_EAP_SRVC.H*/
