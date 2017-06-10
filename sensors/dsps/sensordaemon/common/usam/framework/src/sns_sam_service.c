/*============================================================================
  @file sns_sam_service.c

  @brief
  The SAM Framework both acts as a QMI service and a QMI client.  This file
  contains the implementation of all service duties.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include "sns_init.h"
#include "sns_usmr.h"
#include "sns_debug_str.h"
#include "sns_sam_algo_api.h"
#include "sns_sam.h"
#include "sns_sam_req.h"
#include "sns_sam_pm.h"
#include "sns_sam_memmgr.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ===========================================================================*/

/* SAM services do not have a consistent set of fields in their response
messages.  However, we still wish to use a generic message to avoid per-service
checks.  This requires adding a some buffer for some responses, and to be safe,
we add it to all */
#define SNS_SAM_RESP_PAD 2

/*============================================================================
  Type Declarations
  ===========================================================================*/

/**
 * If a SMR client is not processing indication messages at an appropriate
 * rate, the SMR Framework will indicate to SAM to no longer send indications,
 * and will let SAM know when to start sending them again.  An object of this
 * type will be created when such a callback is received.
 */
struct sns_sam_resume
{
  /* Data fields necessary to add this object to a SAM list */
  sns_q_link_s qLink;

  /* Used by the infrastructure to identify unique clients */
  smr_service_hndl serviceHndl;
};
typedef struct sns_sam_resume sns_sam_resume;

/*============================================================================
  Static Data
  ===========================================================================*/

/* Contains all unprocessed incoming request messages */
sns_q_s samReqMsgQ SNS_SAM_UIMAGE_DATA;
OS_EVENT *samReqMsgQMutex SNS_SAM_UIMAGE_DATA;

/* List of clients now able to receive indications (see sns_sam_resume) */
static sns_q_s samResumeQ;
static OS_EVENT *samResumeQMutex;

/*============================================================================
  Static Function Definitions
  ===========================================================================*/

/**
 * @brief
 * This callback function is called by the SMR infrastructure when
 * infrastructure receives a request from each client(user of the service).
 *
 * @param[i] serviceHndl Handle used by infrastructure to identify clients
 * @param[i] serviceCookie Pointer to the SAM algorithm struct
 * @param[o] connHndl Token to represent this client connection to the service
 *
 * @return SMR_CB_NO_ERR or error code.
 */
smr_cb_err sns_sam_service_connect( smr_qmi_client_handle serviceHndl,
    void *serviceCookie, void **connHndl )
{
  UNREFERENCED_PARAMETER(serviceHndl);
  UNREFERENCED_PARAMETER(serviceCookie);
  UNREFERENCED_PARAMETER(connHndl);

  SNS_PRINTF_STRING_LOW_1( samModule,
    "sns_sam_service_connect %i", serviceHndl );

  *connHndl = serviceHndl;
  return SMR_CB_NO_ERR;
}

/**
 * @brief
 * This callback function is called by the SMR infrastructure when the each
 * client(user of service) deregisters with the  SMR infrastructure.
 *
 * @param[i] connHndle Token to represent client connection to the service;
 *                     Set to the smr_service_hndl in sns_sam_service_connect()
 * @param[i] serviceCookie Pointer to the SAM algorithm struct
 *
 * @return SMR_CB_NO_ERR or error code.
 */
void sns_sam_service_disconnect( void *connHndle, void *serviceCookie )
{
  UNREFERENCED_PARAMETER(connHndle);
  UNREFERENCED_PARAMETER(serviceCookie);
  smr_qmi_client_handle serviceHndl = connHndle;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "sns_sam_service_disconnect %i", connHndle );

  sns_sam_remove_all_client_req( serviceHndl );
}

/**
 * @brief
 * This callback is invoked when the infrastructure receives an incoming
 * message.  The infrastructure decodes the data and gives it to the services.
 *
 * @param[i] connHndle Token to represent this client connection to service.
 *                     Set to the smr_service_hndl in sns_sam_service_connect()
 * @param[i] reqHndl Handle provided by the infrastructure to specify this
 *                      particular transaction and message.
 * @param[i] msgID Message ID pertaining to this particular message.
 * @param[i] reqBuf C struct with the decoded data.
 * @param[i] reqBufLen Length of the c struct.
 * @param[i] serviceCookie Pointer to the SAM algorithm struct
 *
 * @return SMR_CB_NO_ERR or error code.
 */
/*=========================================================================*/
smr_cb_err sns_sam_process_req( void *connHndle,
    smr_req_handle reqHndl, unsigned int msgID, void *reqBuf,
    unsigned int reqBufLen, void *serviceCookie )
{
  uint8_t errOS;
  sns_sam_req_msg *reqMsg = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
    sizeof(sns_sam_req_msg) );
  SNS_ASSERT(NULL != reqMsg);
  sns_sam_sensor_algo *algo = serviceCookie;
  int32_t errQMI;
  smr_cb_err rv = SMR_CB_NO_ERR;

  SNS_PRINTF_STRING_LOW_2( samModule,
    "sns_sam_process_req %i (%i)", msgID, connHndle );

  reqMsg->msg.msg.buf = (intptr_t)SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
    reqBufLen );
  SNS_ASSERT(NULL != (void*)reqMsg->msg.msg.buf);
  SNS_OS_MEMZERO( (void*)reqMsg->msg.msg.buf, reqBufLen );
  SNS_OS_MEMSCPY( (void*)reqMsg->msg.msg.buf, reqBufLen, reqBuf, reqBufLen );
  reqMsg->msg.msg.bufSize = reqBufLen;
  reqMsg->msg.msgID = msgID;
  reqMsg->msg.sensorUID = &algo->sensor.sensorUID;

  reqMsg->reqHndl = reqHndl;
  reqMsg->serviceHndl = connHndle;

  reqMsg->respMsg.sensorUID = reqMsg->msg.sensorUID;
  reqMsg->respMsg.msgID = reqMsg->msg.msgID;

  errQMI = qmi_idl_get_message_c_struct_len( algo->sensor.serviceObj,
    QMI_IDL_RESPONSE, reqMsg->respMsg.msgID, &reqMsg->respMsg.msg.bufSize );
  if( QMI_IDL_LIB_NO_ERR == errQMI )
  {
    reqMsg->respMsg.msg.buf = (intptr_t)SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
      reqMsg->respMsg.msg.bufSize + SNS_SAM_RESP_PAD );
    SNS_ASSERT(NULL != (void*)reqMsg->respMsg.msg.buf);
    SNS_OS_MEMZERO( (void*)reqMsg->respMsg.msg.buf,
      reqMsg->respMsg.msg.bufSize );

    sns_q_link( reqMsg, (sns_q_link_s*)reqMsg );

    sns_os_mutex_pend( samReqMsgQMutex, 0, &errOS );
    sns_q_put( &samReqMsgQ, (sns_q_link_s*)reqMsg );
    sns_os_mutex_post( samReqMsgQMutex );

    sns_os_sigs_post( sns_sam_sig_event,
      SNS_SAM_REQ_MSG_SIG, OS_FLAG_SET, &errOS );
  }
  else
  {
    SNS_SAM_MEM_FREE( (void*)reqMsg->msg.msg.buf );
    SNS_SAM_MEM_FREE( (void*)reqMsg );
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Error looking-up response message size %i", errQMI );
    rv = SMR_CB_INTERNAL_ERR;
  }

  // Note: If the # of messages exceeds X, we should vote for a higher CPU
  return rv;
}

/**
 * @brief
 * This callback function (if provided) is called by the framework
 * when a previously busy client becomes available for more indications.
 *
 * @param[i] serviceHndl Handle used by infrastructure to identify services
 * @param[i] connHndle Token to represent this client connection to service;
 *                     Set to the smr_service_hndl in sns_sam_service_connect()
 * @param[i] serviceCookie Pointer to the SAM algorithm struct
 */
void sns_sam_resume_ind( smr_service_hndl serviceHndl,
    void *connHndle, void *serviceCookie )
{
  UNREFERENCED_PARAMETER(connHndle);
  UNREFERENCED_PARAMETER(serviceCookie);
  sns_sam_resume *resumeData = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
    sizeof(sns_sam_resume) );
  SNS_ASSERT(NULL != resumeData);
  uint8_t errOS;

  SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_resume_ind" );

  resumeData->serviceHndl = serviceHndl;
  sns_q_link( resumeData, (sns_q_link_s*)resumeData );

  sns_os_mutex_pend( samResumeQMutex, 0, &errOS );
  sns_q_put( &samResumeQ, (sns_q_link_s*)resumeData );
  sns_os_mutex_post( samResumeQMutex );

  sns_os_sigs_post( sns_sam_sig_event,
    SNS_SAM_SVC_RESUME_SIG, OS_FLAG_SET, &errOS );
}

/*============================================================================
  Function Definitions
  ===========================================================================*/

sns_sam_err
sns_sam_service_reg( sns_sam_sensor_algo *algo )
{
  smr_err err;
  void *serviceCookie = (void*)algo;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "Registering service: %x", algo->sensor.sensorUID );

  err = smr_service_register( algo->sensor.serviceObj, SNS_SAM_SMR_SVC_PRI,
      sns_sam_service_connect, sns_sam_service_disconnect,
      sns_sam_process_req, serviceCookie, &algo->serviceProvider );
  if( SMR_NO_ERR != err )
  {
    SNS_PRINTF_STRING_ERROR_2( samModule,
      "Unable to register service: %x, error %i",
      algo->sensor.sensorUID, err );
  }
  else
  {
    algo->sensor.isAvailable = true;
  }

  return ( SMR_NO_ERR == err ) ? SAM_ENONE : SAM_ESMR;
}

sns_sam_err
sns_sam_service_unreg( sns_sam_sensor_algo const *algo )
{
  smr_err err;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "Unregistering service: %x", algo->sensor.sensorUID );

  err = smr_service_unregister( algo->serviceProvider );
  if( SMR_NO_ERR != err )
  {
    SNS_PRINTF_STRING_ERROR_2( samModule,
      "Unable to register service: %x, error %i",
      algo->sensor.sensorUID, err );
  }

 return ( SMR_NO_ERR == err ) ? SAM_ENONE : SAM_ESMR;
}

sns_sam_err
sns_sam_service_send_resp( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg const *respMsg )
{
  smr_err err;

  SNS_PRINTF_STRING_LOW_1( samModule, "Sending response %i", respMsg->msgID );

  err = smr_service_send_resp( reqMsg->reqHndl, respMsg->msgID,
    (void*)respMsg->msg.buf, respMsg->msg.bufSize );
  if( SMR_NO_ERR != err )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Unable to send response, error %i", err );
  }

  return ( SMR_NO_ERR == err ) ? SAM_ENONE : SAM_ESMR;
}

SNS_SAM_UIMAGE_CODE sns_sam_err
sns_sam_service_send_ind( sns_sam_ind_msg const *indMsg,
    sam_client_req *clientReq )
{
  sns_sam_err rv = SAM_ENONE;
  smr_err err;

  if( !clientReq->clientBusy )
  {
    err = smr_service_send_ind( clientReq->serviceHndl, indMsg->msgID,
      (void*)indMsg->msg.buf, indMsg->msg.bufSize );

    if( SMR_TIMEOUT_ERR == err )
    {
      clientReq->clientBusy = true;
      SNS_PRINTF_STRING_MEDIUM_0( samModule,
        "Client busy, dropping indication" );
      rv = SAM_ENOT_AVAILABLE;
    }
    else if( SMR_NO_ERR != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Unable to send indication, error %i", err );
      rv = SAM_ESMR;
    }
  }
  else
  {
    SNS_PRINTF_STRING_MEDIUM_0( samModule,
      "Client busy, dropping indication" );
    rv = SAM_ENOT_AVAILABLE;
  }

  return rv;
}

void
sns_sam_process_req_q( void )
{
  sns_q_link_s *qCurr;
  uint8_t errOS;
  sns_sam_req_msg *reqMsg = NULL;

  // Adjust the SAM MIPS vote based on Queue count
  sns_sam_pm_adjust_mips( );
  do
  {
    sns_os_mutex_pend( samReqMsgQMutex, 0, &errOS );
    qCurr = sns_q_get( &samReqMsgQ );
    reqMsg = (sns_sam_req_msg*)qCurr;
    sns_os_mutex_post( samReqMsgQMutex );

    if( NULL != reqMsg )
    {
      sns_sam_handle_req( reqMsg );
      SNS_SAM_MEM_FREE( (void*)reqMsg->msg.msg.buf );
      SNS_SAM_MEM_FREE( (void*)reqMsg->respMsg.msg.buf );
      SNS_SAM_MEM_FREE( reqMsg );
    }
  } while( NULL != reqMsg );
}

void
sns_sam_process_resume_q( void )
{
  sns_q_link_s *qCurr;
  uint8_t errOS;
  sns_sam_resume *resumeData = NULL;

  do
  {
    sns_os_mutex_pend( samResumeQMutex, 0, &errOS );
    qCurr = sns_q_get( &samResumeQ );
    resumeData = (sns_sam_resume*)qCurr;
    sns_os_mutex_post( samResumeQMutex );

    if( NULL != resumeData )
    {
      sns_sam_mark_client_avail( resumeData->serviceHndl );
      SNS_SAM_MEM_FREE( (void*)resumeData );
    }
  } while( NULL != resumeData );
}

sns_sam_err
sns_sam_service_init_fw( void )
{
  const uint8_t priority0 = 0;
  uint8_t errOS = 0;

  samReqMsgQMutex = sns_os_mutex_create_uimg( priority0, &errOS );
  if( 0 != errOS )
  {
    SNS_PRINTF_STRING_FATAL_1( samModule, "Cannot create mutex %i", errOS );
    return SAM_EFAILED;
  }

  samResumeQMutex = sns_os_mutex_create( priority0, &errOS );
  if( 0 != errOS )
  {
    SNS_PRINTF_STRING_FATAL_1( samModule, "Cannot create mutex %i", errOS );
    return SAM_EFAILED;
  }

  sns_q_init( &samReqMsgQ );
  sns_q_init( &samResumeQ );

  return SAM_ENONE;
}