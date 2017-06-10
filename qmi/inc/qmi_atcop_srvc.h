/*
** Copyright (c) 2010-2013 Qualcomm Technologies, Inc. All Rights Reserved.
** Qualcomm Technologies Proprietary and Confidential.
*/
#ifndef QMI_ATCOP_SRVC_H
#define QMI_ATCOP_SRVC_H

#include "qmi.h"

#ifdef __cplusplus
extern "C" {
#endif


#define QMI_ATCOP_MAX_MSG_SIZE           450
#define QMI_ATCOP_AT_CMD_NAME_MAX_LEN    20
#define QMI_ATCOP_MAX_AT_CMDS_REG        20

#define QMI_ATCOP_AT_RESP_MAX_LEN        200
#define QMI_ATCOP_AT_URC_MAX_LEN         QMI_ATCOP_AT_RESP_MAX_LEN
#define QMI_ATCOP_MAX_PARAM_BUF_SIZE     QMI_ATCOP_MAX_MSG_SIZE 
#define QMI_ATCOP_MAX_TOKEN_SIZE         20
#define QMI_ATCOP_MAX_TOKENS             44


typedef unsigned long  qmi_atcop_at_cmd_hndl_type;

/*====================================================
*Definitions associated with qmi_atcop_at_cmd_fwd_req_type
=====================================================*/
typedef enum
{
  QMI_ATCOP_AT_URC_START          = 0,
  QMI_ATCOP_AT_URC_END,
  QMI_ATCOP_AT_URC_INTERMEDIATE,
  QMI_ATCOP_AT_URC_COMPLETE,
  QMI_ATCOP_AT_URC_STATUS_MAX = QMI_ATCOP_AT_URC_COMPLETE
}qmi_atcop_at_urc_status_type;

typedef enum
{
  QMI_ATCOP_AT_CMD_ABORTABLE      =  1,
  QMI_ATCOP_AT_CMD_NOT_ABORTABLE  =  0
}qmi_atcop_abort_type;

typedef struct
{
  qmi_atcop_abort_type    abort_flag;
  unsigned char           at_cmd_name[QMI_ATCOP_AT_CMD_NAME_MAX_LEN + 1];//Atcop service expects a NULL terminated AT CMD NAME string
}qmi_atcop_at_cmd_info_type;

typedef struct
{
  int                         num_of_cmds;    //number of commands in the request.
  qmi_atcop_at_cmd_info_type  qmi_atcop_at_cmd_fwd_req_type[QMI_ATCOP_MAX_AT_CMDS_REG];  
}qmi_atcop_at_cmd_fwd_req_type;


typedef struct
{
  qmi_atcop_at_cmd_hndl_type          at_hndl;
  unsigned char                       *at_urc; //Should be NULL terminated string!
  qmi_atcop_at_urc_status_type        status;
}qmi_atcop_at_fwd_urc_req_type;

/*====================================================
*Definitions associated with qmi_atcop_fwd_at_cmd_resp
=====================================================*/

typedef enum
{
  QMI_ATCOP_RESULT_ERROR = 0, // Error or CME Error, Response Buffer should contain Error details
  QMI_ATCOP_RESULT_OK    = 1, // OK result Code to terminal, Response Buffer SHOULD NOT contain OK.
  QMI_ATCOP_RESULT_OTHER = 2  // For all other result codes.
}qmi_atcop_fwd_resp_result_type;

typedef enum
{
  QMI_ATCOP_RESP_START = 0,
  QMI_ATCOP_RESP_END,
  QMI_ATCOP_RESP_INTERMEDIATE,
  QMI_ATCOP_RESP_COMPLETE,              // Response Buffer contains Complete Response and Should Not contain 'OK'.
                                        // If only OK needs to be sent set 'QMI_ATCOP_DSAT_RESP_COMPLETE'
                                        // and Response as NULL.
  QMI_ATCOP_RESP_TYPE_MAX = QMI_ATCOP_RESP_COMPLETE
}qmi_atcop_fwd_resp_response_type;

typedef struct
{
  qmi_atcop_at_cmd_hndl_type          at_hndl;
  qmi_atcop_fwd_resp_result_type      result;
  qmi_atcop_fwd_resp_response_type    response;
  unsigned char                       *at_resp;//Atcop service expects a NULL terminated AT CMD RESP, 
                                               // The Max size of the response can be QMI_ATCOP_AT_CMD_NAME_MAX_LEN
}qmi_atcop_fwd_resp_at_resp_type;

/*====================================================
  Indication Callback Related Definitions
=====================================================*/

/* Distinguishes indication message types */
typedef enum
{
  QMI_ATCOP_SRVC_INVALID_IND_TYPE,
  QMI_ATCOP_SRVC_ABORT_MSG_IND_TYPE,
  QMI_ATCOP_SRVC_AT_FWD_MSG_IND_TYPE
  /* To be filled in in future release */
} qmi_atcop_indication_id_type;

/*Definitions associated with qmi_atcop_fwd_at_cmd_ind_type*/
typedef unsigned long qmi_atcop_at_opcode_type;
#define QMI_ATCOP_NA  0x00000001
#define QMI_ATCOP_EQ  0x00000002
#define QMI_ATCOP_QU  0x00000004
#define QMI_ATCOP_AR  0x00000008


typedef unsigned char*  qmi_atcop_token_info_type;

/*Param Mask for optional TLVS*/

#define QMI_ATCOP_STATE_INFO_V_VAL              0x00000001
#define QMI_ATCOP_STATE_INFO_Q_VAL              0x00000002
#define QMI_ATCOP_STATE_INFO_S3_VAL             0x00000004
#define QMI_ATCOP_STATE_INFO_S4_VAL             0x00000008
#define QMI_ATCOP_STATE_INFO_CLIR_VAL           0x00000010
#define QMI_ATCOP_STATE_INFO_COLP_VAL           0x00000020
#define QMI_ATCOP_STATE_INFO_CMEE_VAL           0x00000040
#define QMI_ATCOP_STATE_INFO_CCUG_VAL           0x00000080
#define QMI_ATCOP_STATE_INFO_CMEC_VAL           0x00000100

typedef struct
{
  qmi_atcop_at_cmd_hndl_type        at_hndl;
  qmi_atcop_at_opcode_type          op_code;
  unsigned char                     at_name[QMI_ATCOP_AT_CMD_NAME_MAX_LEN + 1];

  /*Optional values */

  /*Token Info*/
  unsigned short                    num_tokens;//Number of valid tokens that follow
  qmi_atcop_token_info_type         tokens[QMI_ATCOP_MAX_TOKENS];

  /* param mask to indicate which of the 
  ** optional State Info values are valid when returned 
  ** to the user
  */
  unsigned  long                   param_mask;

  /*State Info values */
  unsigned long                     v_val;
  unsigned long                     q_val;
  unsigned long                     s3_val;
  unsigned long                     s4_val;
  unsigned long                     clir_val;
  unsigned long                     colp_val;
  unsigned long                     cmee_val;
  struct 
  {
    unsigned long     ccug_val1;
    unsigned long     ccug_val2;
    unsigned long     ccug_val3;
  }ccug_val;

  struct 
  {
    unsigned long     cmec_val1;
    unsigned long     cmec_val2;
    unsigned long     cmec_val3;
    unsigned long     cmec_val4;
    unsigned char     cmec_val4_is_valid; /* TRUE if cmec_val4 is valid, FALSE if not */
  }cmec_val;

}qmi_atcop_at_cmd_fwd_ind_type;

/* Async notification reporting structure */
typedef union
{
  qmi_atcop_at_cmd_hndl_type            at_hndl;
  qmi_atcop_at_cmd_fwd_ind_type         at_cmd_fwd_type;
  /* To be filled in in future release */
} qmi_atcop_indication_data_type;

typedef void (*qmi_atcop_indication_hdlr_type)
( 
  int                             user_handle,
  qmi_service_id_type             service_id,
  void                            *user_data,
  qmi_atcop_indication_id_type    ind_id,
  qmi_atcop_indication_data_type  *ind_data
);

/************************************************************************
* Function prototypes
************************************************************************/

/*===========================================================================
  FUNCTION  qmi_atcop_srvc_init_client
===========================================================================*/
/*!
@brief 
  This function is called to initialize the ATCOP service.  This function
  must be called prior to calling any other ATCOP service functions.
  Also note that this function may be called multiple times to allow 
  for multiple, independent clients.   
  
@return 
  Valid user handle which is > 0, if the init client operation was successful,
  < 0 if not, If the return code is QMI_SERVICE_ERR, then the qmi_err_code will be
  valid and indicated which QMI error occured.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/    
/*=========================================================================*/
EXTERN qmi_client_handle_type
qmi_atcop_srvc_init_client
(
  const char                      *dev_id,
  qmi_atcop_indication_hdlr_type  user_ind_msg_hdlr,
  void                            *user_ind_msg_hdlr_user_data,
  int                             *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_atcop_srvc_release_client
===========================================================================*/
/*!
@brief 
  This function is called to release a client created by the 
  qmi_atcop_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has 
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.  
  
@return 
  QMI_NO_ERR if the release client operation was sucessful, < 0 if not. 
  If return code  is  QMI_SERVICE_ERR, then the qmi_err_code will be valid 
  and will indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/    
/*=========================================================================*/
EXTERN int 
qmi_atcop_srvc_release_client
(
  int      user_handle,
  int      *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_atcop_reg_at_command_fwd_req
===========================================================================*/
/*!
@brief 
  This command is used by the client to register any AT commands that need to 
  be forwarded to it from the modem.
     
@return 
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is 
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which 
  QMI error occurred.

@note

  - Dependencies
    - qmi_atcop_srvc_init_client() must be called before calling this.

  - Side Effects
    - 
*/    
/*=========================================================================*/
EXTERN int
qmi_atcop_reg_at_command_fwd_req
(
  int                               client_handle,
  qmi_atcop_at_cmd_fwd_req_type     *cmd_fwd_req,
  int                               *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_atcop_fwd_at_cmd_resp
===========================================================================*/
/*!
@brief 
  This command is used by the client to send the response to an AT cmd 
  previously forwarded to the client from the modem
     
@return 
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is 
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which 
  QMI error occurred.
@note

  - Dependencies
    - qmi_atcop_srvc_init_client() must be called before calling this.

  - Side Effects
    - 
*/    
/*=========================================================================*/
EXTERN int
qmi_atcop_fwd_at_cmd_resp
(
  int                               client_handle,
  qmi_atcop_fwd_resp_at_resp_type   *at_resp,
  int                               *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_atcop_fwd_at_urc_req
===========================================================================*/
/*!
@brief 
  This command is used to send unsolicited response codes to the modem.
     
@return 
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is 
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which 
  QMI error occurred.

@note

  - Dependencies
    - qmi_atcop_srvc_init_client() must be called before calling this.

  - Side Effects
    - 
*/    
/*=========================================================================*/
extern int
qmi_atcop_fwd_at_urc_req
(
  int                               client_handle,
  qmi_atcop_at_fwd_urc_req_type     *urc_fwd_req,
  int                               *qmi_err_code
);

#ifdef __cplusplus
}
#endif

#endif   /*QMI_ATCOP_SRVC.H*/
