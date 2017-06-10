#ifndef __WFD_HDCP_CP_H__
#define __WFD_HDCP_CP_H__

/***************************************************************************
 *                             WFD_Hdcp_Cp.h
 * DESCRIPTION
 *  HDCP wrapper for WFD content protection
 *
 * Copyright (c) 2012-2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header:$
  $DateTime:$
  $Change:$
 ***************************************************************************/

/***************************************************************************
 *                   I N C L U D E-F I L E S
 ***************************************************************************/
#include "MMThread.h"
#include "AEEStdDef.h"
#include "AEEstd.h"
#include "WFDMMSourceComDef.h"
#ifdef WFD_HDCP_ENABLED
#include "DX_Hdcp_Errors.h"
#include "DX_Hdcp_Types.h"
#include "DxTypes.h"
#endif //#ifdef WFD_HDCP_ENABLED


/***************************************************************************
 *                 DATA DECLARATIONS
 ***************************************************************************/

/***************************************************************************
 *                 CONSTANT & DEFINE
 ***************************************************************************/
#define HDCP_CP_ERROR unsigned long
#define STREAM_AUDIO     0x0
#define STREAM_VIDEO     0x1
#define STREAM_INVALID   0x2
#define MAX_NUM_STREAM   2
#define PES_PVT_DATA_LEN 16
#define NUM_DOWNSTREAM_DEVICE 2
#define VIDEO_STREAMID   0x1011
#define AUDIO_STREAMID   0x1100

typedef enum __WFD_HDCP_DEVICE_REPEAT_CAPABILITY__
{
  HDCP_STREAM_DATA_REPEATABLE,
  HDCP_STREAM_DATA_NOT_REPEATABLE,
}WFD_HDCP_REPEAT_CAPABILITY;
typedef enum __WFD_HDCP_DEVICETYPE__
{
  SOURCE_DEVICE,
  PRIMARY_SINK_DEVICE,
  SECONDARY_SINK_DEVICE
}WFD_HDCP_DEVICETYPE;


typedef enum __HDCP_SESSION_STATETYPE__
{
  HDCP_SESSION_STATE_UNKNOWN,
  HDCP_SESSION_STATE_INIT,
  HDCP_SESSION_STATE_CONNECT,
  HDCP_SESSION_STATE_READY,
  HDCP_SESSION_STATE_CLOSE,
  HDCP_SESSION_STATE_DEINIT,
}HDCP_SESSION_STATETYPE;

typedef enum __HDCP_SESSION_STATUSTYPE__
{
  HDCP_STATUS_FAIL,
  HDCP_STATUS_SUCCESS,
  HDCP_STATUS_INIT_COMPLETE,
  HDCP_STATUS_UPSTREAM_CLOSE,
  HDCP_STATUS_DOWNSTREAM_CLOSE,
  HDCP_STATUS_UNAUTHENTICATED_CONNECTION,
  HDCP_STATUS_UNAUTHORIZED_CONNECTION,
  HDCP_STATUS_REVOKED_CONNECTION,
  HDCP_STATUS_UNRECOVERABLE_ERROR
}HDCP_SESSION_STATUSTYPE;

#ifdef WFD_HDCP_ENABLED
typedef enum __HDCP_CP_ERRORTYPE__
{
  /** Success */
  QC_HDCP_CP_ErrorNone = 0,
  /** Error is undefined */
  QC_HDCP_CP_ErrorUndefined = (signed long) 0x80000100,
  /** Insufficient resource to performed desired action */
  QC_HDCP_CP_InsufficientResources = (signed long) 0x80000101,
  QC_HDCP_CP_ErrorCipherDisabled = (signed long) 0x80000102,
  /** Bad parameter passed as an argument */
  QC_HDCP_CP_BadParameter = (signed long ) 0x80000103,
  /* Functionality not implemented */
  QC_HDCP_CP_NotImplemented = (signed long ) 0x80000104,
  QC_HDCP_UNEXPECTED_MESSAGE_ERROR = DX_HDCP_UNEXPECTED_MESSAGE_ERROR,
  QC_HDCP_RECEIVERS_NUM_OVERFLOW = DX_HDCP_RECEIVERS_NUM_OVERFLOW,
  QC_HDCP_SESSIONS_NUM_OVERFLOW = DX_HDCP_SESSIONS_NUM_OVERFLOW,
  QC_HDCP_STREAMS_NUM_OVERFLOW = DX_HDCP_STREAMS_NUM_OVERFLOW,
  QC_HDCP_STREAM_COUNTER_OVERFLOW = DX_HDCP_STREAM_COUNTER_OVERFLOW,
  QC_HDCP_ILLEGAL_CONFIG_PARAM = DX_HDCP_ILLEGAL_CONFIG_PARAM,
  QC_HDCP_ELEMENT_TYPE_MISMATCH =  DX_HDCP_ELEMENT_TYPE_MISMATCH,
  QC_HDCP_SET_CONFIG_PARAM_FORBIDDEN_BEFORE_INIT = DX_HDCP_SET_CONFIG_PARAM_FORBIDDEN_BEFORE_INIT,
  QC_HDCP_SESSIONS_NUMBER_MISMATCH = DX_HDCP_SESSIONS_NUMBER_MISMATCH,
  QC_HDCP_SESSION_ALREADY_OPEN = DX_HDCP_SESSION_ALREADY_OPEN,
  QC_HDCP_SESSION_ALREADY_CLOSE = DX_HDCP_SESSION_ALREADY_CLOSE,
  QC_HDCP_CONNECTIONS_NUMBER_MISMATCH = DX_HDCP_CONNECTIONS_NUMBER_MISMATCH,
  QC_HDCP_CONNECTION_ALREADY_OPEN = DX_HDCP_CONNECTION_ALREADY_OPEN,
  QC_HDCP_CONNECTION_ALREADY_CLOSE = DX_HDCP_CONNECTION_ALREADY_CLOSE,
  QC_HDCP_ONLY_SINGLE_TSMT_PER_RCV = DX_HDCP_ONLY_SINGLE_TSMT_PER_RCV,
  QC_HDCP_CERT_SIGNATURE_VERIFICATION_FAILED = DX_HDCP_CERT_SIGNATURE_VERIFICATION_FAILED,
  QC_HDCP_SRM_SIGNATURE_VERIFICATION_FAILED = DX_HDCP_SRM_SIGNATURE_VERIFICATION_FAILED,
  QC_HDCP_H_PRIM_TIMEOUT = DX_HDCP_H_PRIM_TIMEOUT,
  QC_HDCP_WRONG_MESSAGE_LENGTH = DX_HDCP_WRONG_MESSAGE_LENGTH,
  QC_HDCP_CORRUPTED_RECEIVER_DATA = DX_HDCP_CORRUPTED_RECEIVER_DATA,
  QC_HDCP_CONNECTION_TIMEOUT = DX_HDCP_CONNECTION_TIMEOUT,
  QC_HDCP_H_VERIFICATION_FAILED = DX_HDCP_H_VERIFICATION_FAILED,
  QC_HDCP_LOCALITY_CHECK_FAILED = DX_HDCP_LOCALITY_CHECK_FAILED,
  QC_HDCP_SRM_FILE_INEXIST = DX_HDCP_SRM_FILE_INEXIST,
  QC_HDCP_NON_ACTIVE_SESSION = DX_HDCP_NON_ACTIVE_SESSION,
  QC_HDCP_NON_AUTHENTICATED_CONNECTION = DX_HDCP_NON_AUTHENTICATED_CONNECTION,
  QC_HDCP_NON_AUTHORIZED_CONNECTION = DX_HDCP_NON_AUTHORIZED_CONNECTION,
  QC_HDCP_SESSION_KEY_ENCRYPTION_FAILED = DX_HDCP_SESSION_KEY_ENCRYPTION_FAILED,
  QC_HDCP_SESSION_KEY_DECRYPTION_FAILED = DX_HDCP_SESSION_KEY_DECRYPTION_FAILED,
  QC_HDCP_SESSION_UNDEFINED_SECURED_SERVICE = DX_HDCP_SESSION_UNDEFINED_SECURED_SERVICE,
  QC_HDCP_INVALID_PES_PRIVATE_DATA_STRUCT = DX_HDCP_INVALID_PES_PRIVATE_DATA_STRUCT,
  QC_HDCP_SRM_REVOCATION_LIST_CONTAINS_THE_RECEIVER_ID = DX_HDCP_SRM_REVOCATION_LIST_CONTAINS_THE_RECEIVER_ID,
  QC_HDCP_STREAM_ID_MISMATCH = DX_HDCP_STREAM_ID_MISMATCH,
  QC_HDCP_INPUT_CTR_MISMATCH = DX_HDCP_INPUT_CTR_MISMATCH,
  QC_HDCP_UPSTREAM_PROPAGATION_FAILED = DX_HDCP_UPSTREAM_PROPAGATION_FAILED,
  QC_HDCP_UPSTREAM_PROPAGATION_SEQNUMV_ROLLOVER = DX_HDCP_UPSTREAM_PROPAGATION_SEQNUMV_ROLLOVER,
  QC_HDCP_UPSTREAM_PROPAGATION_TYPE1_HDCP_CONFLICT = DX_HDCP_UPSTREAM_PROPAGATION_TYPE1_HDCP_CONFLICT,
  QC_HDCP_UPSTREAM_PROPAGATION_TOPOLOGY_EXCEEDED = DX_HDCP_UPSTREAM_PROPAGATION_TOPOLOGY_EXCEEDED,
  QC_HDCP_DOWNSTREAM_PROPAGATION_FAILED = DX_HDCP_DOWNSTREAM_PROPAGATION_FAILED,
  QC_HDCP_UPSTREAM_PROPAGATION_SEQNUMM_ROLLOVER = DX_HDCP_UPSTREAM_PROPAGATION_SEQNUMM_ROLLOVER,
  QC_HDCP_INVALID_MESSAGE_ID = DX_HDCP_INVALID_MESSAGE_ID,
  QC_HDCP_SECURED_SERVICE_INIT_FAILED = DX_HDCP_SECURED_SERVICE_INIT_FAILED,
  QC_HDCP_SECURED_SERVICE_DEVICE_ROOTED =  DX_HDCP_SECURED_SERVICE_DEVICE_ROOTED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_KH_FAILED = DX_HDCP_SECURED_SERVICE_COMPUTE_KH_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_KD_FAILED = DX_HDCP_SECURED_SERVICE_COMPUTE_KD_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_H_FAILED = DX_HDCP_SECURED_SERVICE_COMPUTE_H_FAILED,
  QC_HDCP_SECURED_SERVICE_DECRYPT_KM_WITH_KPRIV_FAILED = DX_HDCP_SECURED_SERVICE_DECRYPT_KM_WITH_KPRIV_FAILED,
  QC_HDCP_SECURED_SERVICE_DECRYPT_KM_WITH_KH_FAILED = DX_HDCP_SECURED_SERVICE_DECRYPT_KM_WITH_KH_FAILED,
  QC_HDCP_SECURED_SERVICE_VERIFY_DATA_SIGNATURE_FAILED = DX_HDCP_SECURED_SERVICE_VERIFY_DATA_SIGNATURE_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_ENCRYPTED_NEW_KM_WITH_KPUB_FAILED = DX_HDCP_SECURED_SERVICE_GET_ENCRYPTED_NEW_KM_WITH_KPUB_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_ENCRYPTED_KM_WITH_KH_FAILED = DX_HDCP_SECURED_SERVICE_GET_ENCRYPTED_KM_WITH_KH_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_L_FAILED = DX_HDCP_SECURED_SERVICE_COMPUTE_L_FAILED,
  QC_HDCP_SECURED_SERVICE_CIPHER_DATA_FAILED = DX_HDCP_SECURED_SERVICE_CIPHER_DATA_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_DKEY2_FAILED = DX_HDCP_SECURED_SERVICE_COMPUTE_DKEY2_FAILED,
  QC_HDCP_SECURED_SERVICE_START_TIME_COUNT_FAILED = DX_HDCP_SECURED_SERVICE_START_TIME_COUNT_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_EKH_FAILED = DX_HDCP_SECURED_SERVICE_GET_EKH_FAILED,
  QC_HDCP_SECURED_SERVICE_STORE_EKH_FAILED = DX_HDCP_SECURED_SERVICE_STORE_EKH_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_H_PRIM_STORED_KM_FAILED = DX_HDCP_SECURED_SERVICE_GET_H_PRIM_STORED_KM_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_H_PRIM_NO_STORED_KM_FAILED = DX_HDCP_SECURED_SERVICE_GET_H_PRIM_NO_STORED_KM_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_V_FAILED = DX_HDCP_SECURED_SERVICE_COMPUTE_V_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_M_FAILED = DX_HDCP_SECURED_SERVICE_COMPUTE_M_FAILED,
  QC_HDCP_SECURED_SERVICE_CERTIFICATE_RETRIEVAL_FAILED = DX_HDCP_SECURED_SERVICE_CERTIFICATE_RETRIEVAL_FAILED,
  QC_HDCP_SECURED_SERVICE_PROVISIONING_FAILED = DX_HDCP_SECURED_SERVICE_PROVISIONING_FAILED,
  QC_HDCP_SECURED_SERVICE_CIPHER_AUTHENTICATION_FAILED = DX_HDCP_SECURED_SERVICE_CIPHER_AUTHENTICATION_FAILED,
  QC_HDCP_SECURED_SERVICE_ERROR_LAST = DX_HDCP_SECURED_SERVICE_ERROR_LAST//Add new Discretix errors before this

}HDCP_CP_ERRORTYPE;
#else
typedef enum __HDCP_CP_ERRORTYPE__
{
  /** Success */
  QC_HDCP_CP_ErrorNone = 0,
  /** Error is undefined */
  QC_HDCP_CP_ErrorUndefined = (signed long) 0x80000100,
  /** Insufficient resource to performed desired action */
  QC_HDCP_CP_InsufficientResources = (signed long) 0x80000101,
  QC_HDCP_CP_ErrorCipherDisabled = (signed long) 0x80000102,
  /** Bad parameter passed as an argument */
  QC_HDCP_CP_BadParameter = (signed long ) 0x80000103,
  /* Functionality not implemented */
  QC_HDCP_CP_NotImplemented = (signed long ) 0x80000104,
  QC_HDCP_UNEXPECTED_MESSAGE_ERROR,
  QC_HDCP_RECEIVERS_NUM_OVERFLOW,
  QC_HDCP_SESSIONS_NUM_OVERFLOW,
  QC_HDCP_STREAMS_NUM_OVERFLOW,
  QC_HDCP_STREAM_COUNTER_OVERFLOW,
  QC_HDCP_ILLEGAL_CONFIG_PARAM,
  QC_HDCP_ELEMENT_TYPE_MISMATCH,
  QC_HDCP_SET_CONFIG_PARAM_FORBIDDEN_BEFORE_INIT,
  QC_HDCP_SESSIONS_NUMBER_MISMATCH,
  QC_HDCP_SESSION_ALREADY_OPEN,
  QC_HDCP_SESSION_ALREADY_CLOSE,
  QC_HDCP_CONNECTIONS_NUMBER_MISMATCH,
  QC_HDCP_CONNECTION_ALREADY_OPEN,
  QC_HDCP_CONNECTION_ALREADY_CLOSE,
  QC_HDCP_ONLY_SINGLE_TSMT_PER_RCV,
  QC_HDCP_CERT_SIGNATURE_VERIFICATION_FAILED,
  QC_HDCP_SRM_SIGNATURE_VERIFICATION_FAILED,
  QC_HDCP_H_PRIM_TIMEOUT,
  QC_HDCP_WRONG_MESSAGE_LENGTH,
  QC_HDCP_CORRUPTED_RECEIVER_DATA,
  QC_HDCP_CONNECTION_TIMEOUT,
  QC_HDCP_H_VERIFICATION_FAILED,
  QC_HDCP_LOCALITY_CHECK_FAILED,
  QC_HDCP_SRM_FILE_INEXIST,
  QC_HDCP_NON_ACTIVE_SESSION,
  QC_HDCP_NON_AUTHENTICATED_CONNECTION,
  QC_HDCP_NON_AUTHORIZED_CONNECTION,
  QC_HDCP_SESSION_KEY_ENCRYPTION_FAILED,
  QC_HDCP_SESSION_KEY_DECRYPTION_FAILED,
  QC_HDCP_SESSION_UNDEFINED_SECURED_SERVICE,
  QC_HDCP_INVALID_PES_PRIVATE_DATA_STRUCT,
  QC_HDCP_SRM_REVOCATION_LIST_CONTAINS_THE_RECEIVER_ID,
  QC_HDCP_STREAM_ID_MISMATCH,
  QC_HDCP_INPUT_CTR_MISMATCH,
  QC_HDCP_UPSTREAM_PROPAGATION_FAILED,
  QC_HDCP_UPSTREAM_PROPAGATION_SEQNUMV_ROLLOVER,
  QC_HDCP_UPSTREAM_PROPAGATION_TYPE1_HDCP_CONFLICT,
  QC_HDCP_UPSTREAM_PROPAGATION_TOPOLOGY_EXCEEDED,
  QC_HDCP_DOWNSTREAM_PROPAGATION_FAILED,
  QC_HDCP_UPSTREAM_PROPAGATION_SEQNUMM_ROLLOVER,
  QC_HDCP_INVALID_MESSAGE_ID,
  QC_HDCP_SECURED_SERVICE_INIT_FAILED,
  QC_HDCP_SECURED_SERVICE_DEVICE_ROOTED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_KH_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_KD_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_H_FAILED,
  QC_HDCP_SECURED_SERVICE_DECRYPT_KM_WITH_KPRIV_FAILED,
  QC_HDCP_SECURED_SERVICE_DECRYPT_KM_WITH_KH_FAILED,
  QC_HDCP_SECURED_SERVICE_VERIFY_DATA_SIGNATURE_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_ENCRYPTED_NEW_KM_WITH_KPUB_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_ENCRYPTED_KM_WITH_KH_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_L_FAILED,
  QC_HDCP_SECURED_SERVICE_CIPHER_DATA_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_DKEY2_FAILED,
  QC_HDCP_SECURED_SERVICE_START_TIME_COUNT_FAILED,
  QC_HDCP_SECURED_SERVICE_CHECK_TIME_COUNT_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_EKH_FAILED,
  QC_HDCP_SECURED_SERVICE_STORE_EKH_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_H_PRIM_STORED_KM_FAILED,
  QC_HDCP_SECURED_SERVICE_GET_H_PRIM_NO_STORED_KM_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_V_FAILED,
  QC_HDCP_SECURED_SERVICE_COMPUTE_M_FAILED,
  QC_HDCP_SECURED_SERVICE_CERTIFICATE_RETRIEVAL_FAILED,
  QC_HDCP_SECURED_SERVICE_PROVISIONING_FAILED,
  QC_HDCP_SECURED_SERVICE_ERROR_LAST//Add new Discretix errors before this

}HDCP_CP_ERRORTYPE;
#endif
/***************************************************************************
 *                 TYPE & DEFINEITION
 ***************************************************************************/
 /*! @brief HDCP SOURCE Notification Callback datatype */
//typedef void (*WFD_HdcpSourceCbFuncType)(HDCP_SRC_STATUS_CBTYPE CbStatus);

/***************************************************************************
 *                 FORWARD DECLARATIONS
 ***************************************************************************/

/***************************************************************************
 *                 CLASS DECLARATIONS
 ***************************************************************************/
#ifdef WFD_HDCP_ENABLED
class CWFD_HdcpCp;
static HDCP_CP_ERRORTYPE WFD_HdcpCpCleanUp(CWFD_HdcpCp *pHDCPContext);
static HDCP_CP_ERRORTYPE WFD_HdcpCpReconnect(CWFD_HdcpCp *pHDCPContext);

#endif


/*!
 *  @brief    WFD_HdcpCp Module.
 *
 *  @details  WFD_HdcpCp is an wrapper on top of DX_Hdcp_Transmitter API
 *            to access HDCP transmitter service provided by libDxHdcp module.
 *
 *  @note
 *
 */

class CWFD_HdcpCp
{
public:
  /**!
   * @brief      CTOR
   * @details    CTOR to create & init a HDCP Source CP session
   * @param[in]  EventCb     Event notification callback function
   * @param[in]  pClientData Client user data
   * @param[in]  ulIpAddr    HDCP Transmitter peer IP Address
   * @param[in]  nCtrlPort   Control port on which HDCP Transmitter( WFD-SRC)
   *             & HDCP Receiver ( WFD-SINK) communicate session message
   */
  CWFD_HdcpCp(
    void *pClientData,
    unsigned long ulIpAddr,
    int nCtrlPort,
    unsigned char ucPrimStreamType,
    unsigned char ucSecStreamType,
    WFD_HDCP_DEVICETYPE eDeviceType,
    unsigned char ucHdcpVer);

  /**!
   *  Default CTOR
   */
   virtual ~CWFD_HdcpCp(void);

  /**!
   * @brief      Init Session
   * @details    Init a WFD_SRC HDCP Session
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  //HDCP_CP_ERROR WFD_HdcpSessionInit(EventCbFunction EventCb);
  HDCP_CP_ERRORTYPE WFD_HdcpSessionInit();

  /**!
   * @brief      De Init Session
   * @details    De-Init a WFD_SRC HDCP Session
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  HDCP_CP_ERRORTYPE WFD_HdcpSessionDeInit();

  /**!
   * @brief      Create HDCP Session
   * @details    Create a new HDCP session and connect
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  HDCP_CP_ERRORTYPE WFD_HdcpCreateSession();

  /**!
   * @brief      Connect HDCP Session
   * @details    Connect to a HDCP session
   * @param[in]  ulTimeSout Connection time out
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  HDCP_CP_ERRORTYPE WFD_HdcpSessionConnect(
    unsigned long ulRetryCnt);

  /**!
   * @brief      Listen HDCP Session
   * @details    Listen to a HDCP session
   * @param[in]  ulTimeSout Connection time out
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  HDCP_CP_ERRORTYPE WFD_HdcpSessionListen();

  /**!
   * @brief      Disconnect HDCP Session
   * @details    Disconnect to a HDCP session
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  HDCP_CP_ERRORTYPE WFD_HdcpDisconnect();

  /**!
   * @brief      Close HDCP Session
   * @details    Close HDCP session and disconnect
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  HDCP_CP_ERRORTYPE WFD_HdcpCloseSession();


  /**!
   * @brief      Open Stream
   * @details    Open Stream for WFD_SRC HDCP Session
   * @param[in]  ucContentStreamID  Content stream ID
   * @param[in]  ucStreamType       Content stream type?
   * @param[out] pucStreamID        StreamID return from Dx_Hdcp library
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  HDCP_CP_ERRORTYPE WFD_HdcpOpenStream(
    uint32 ucContentStreamID,
    unsigned char ucStreamType);

  /**!
   * @brief      Close Stream
   * @details    Close Stream for WFD_SRC HDCP Session
   * @param[in]  ucStreamType       Content stream type?
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  HDCP_CP_ERRORTYPE WFD_HdcpCloseStream(
    unsigned char ucStreamType);

  /**!
   * @brief      Encrypt PES Payload
   * @details    Encrypt PES payload of particular content stream
   * @param[out] aPESPvtHeader        PES private header data
   * @param[in]  pucPESClrPayload     PES clear data payload
   * @param[out] pucPESEncrytPayload  PES encrypted data payload
   * @param[out] ulPayloadLen         Payload length
   * @return     RETURN '0' if SUCCESS
   *             HDCP_CP_ERROR code in FAILURE
   */
  HDCP_CP_ERRORTYPE WFD_HdcpDataEncrypt(
    unsigned char ucStreamType,
    unsigned char* aPESPvtHeader,
    unsigned char * pucPESClrPayload,
    unsigned char * pucPESEncrytPayload,
    unsigned long  ulPayloadLen );
  /**!
   * @brief      HDCP Source CP thread entry.
   * @details	 This function is the entry point to HDCP CP Session handler thread
   * @param[in]  ptr This pointer
   * @return     None
   */
  static int WFD_HdcpCp_ThreadEntry( void* ptr );

  /**!
   * @brief 	 HDCP source CP session handler
   * @details	 This function handles all the event posted to HDCP CP session thread
   * @return	 None
   */
  void WFD_HdcpSessionHandlerThread( );

  /**!
   * @brief 	 HDCP Event Notification
   * @details	 This function handles hdcp event
   * @param[in]  eHdcpEvent    HDCP Event type
   * @param[in]  pConnectionid ConnectionId for this session
   * @param[in]  pStreamId     StreamID for this session
   * @return	 None
   */
  //static void NotifyHdcpCpEvents(EDxHdcpEventType eHdcpEvent, void* pConnectionid, void* pStreamid);

  /**!
   * @brief 	 HDCP Status Notification
   * @details	 This function return HDCP Init status
   * @return	 HDCP Session status
   */
  HDCP_SESSION_STATUSTYPE WFD_HdcpCpSessionStatus(){return m_eHdcpSessionStatus;};

/**!
 * @brief    HDCP Event Call Back Function
 * @details  This function sends error notification to WFDMMSource
 * @return   0 on success else failure
 */
  int WFD_HdcpCpConfigureCallback(eventHandlerType pCallback, unsigned long moduleId, void *pData);


/**!
 * @brief      HDCP Source Cleaup Session
 * @details  This function Cleans up the current HDCP session
 * @param[in]  Current HDCP context
 * @return     HDCP Error value
 */
  friend HDCP_CP_ERRORTYPE WFD_HdcpCpCleanUp(CWFD_HdcpCp *pHDCPContext);

/**!
 * @brief      HDCP Reconnect Session
 * @details    This function Reconnects to the remote device
 * @param[in]  Current HDCP context
 * @return     HDCP Error value
 */
  friend HDCP_CP_ERRORTYPE WFD_HdcpCpReconnect(CWFD_HdcpCp *pHDCPContext);

  bool WFD_HDCP_Events_Supported();

/**!
 * @brief        HDCP
 * @details      This function reports whether encryption can be proceeded with
 * @param[in]    NONE
 * @return       Whether HDCP can be proceeded with or not
 */
  bool proceedWithHDCP()const {return m_bKeepGoing;}

public:

#ifdef WFD_HDCP_ENABLED
  /****************************************************************************

   * @brief      HDCP Event Notification
   * @details    This function handles hdcp event
   * @param[in]  eHdcpEvent    HDCP Event type
   * @param[in]  pConnectionid ConnectionId for this session
   * @param[in]  pStreamId     StreamID for this session
   * @return     None
   ****************************************************************************/
  static void NotifyHdcpCpEvents(EDxHdcpEventType eHdcpEvent,
                              void* pConnectionid,
                              void* pStreamid);
#endif
  static HDCP_SESSION_STATUSTYPE m_eHdcpSessionStatus;
  static WFD_HDCP_REPEAT_CAPABILITY m_eHdcpStreamType;
  // App data
  static void *m_pAppData;
  // Event Handler Type
  static eventHandlerType m_pEventHandlerFn;
  //! Module Id for Error Reporting
  static unsigned long m_uModuleId;
  //! HDCP Session ID for particular session
  void*             m_ucHdcpSessionID;
  //! HDCP ClientID for particular session
  void*                     m_ucHdcpClientID;
  //! HDCP downstream IP address
  unsigned long             m_ulPeerIpAddr;
  //! HDCP control port
  unsigned int              m_ulHdcpCtrlPort;
  //! HDCP audio stream ID
  void*                     m_ucHdcpAudioStreamID;
  //! HDCP video stream ID
  void*                     m_ucHdcpVideoStreamID;
  unsigned char             m_ucHdcpPrimaryStreamType;
  unsigned char             m_ucHdcpSecondaryStreamType;
  //! To store client data passed by caller in CWFD_HdcpSrcCp CTOR
  void*                     m_pClientData;
  //! HDCP SRC cp worker thread handle.
  MM_HANDLE                 m_hHdcpCpThread;
  //! HDCP thread status
  bool                      m_bThreadRunning;
  bool                      m_bHdcpSessionConnect;
  static bool               m_bHdcpCipherstatus;
  bool                      m_bHdcpCipherEventSupported;
  //total length of frames
  int64 m_nEncryptFrameSize;
  int64 m_nMaxFrameSize;
  //Total number of frames encrypted
  int64 m_nEncryptFrames;
  //total time taken for encryption
  int64 m_nEncryptTime;
  int64 m_nMaxEncryptTime;
  // Critical section variable for encrypt API
  MM_HANDLE  m_encryptCriticalSectionHandle;
  private:
  //! HDCP Ctx
  static CWFD_HdcpCp*       s_pHdcpCtx;
  //! Time when Cipher Disabled event comes for first time (at start/middle)
  unsigned long             m_nCipherWaitStart;
  //! Time when Cipher enabled event comes after a stream of Cipher disabled
  unsigned long             m_nCipherWaitEnd;
  //! Flag to indicate if further encryption can be called
  bool                      m_bKeepGoing;
};

#endif /*__WFD_HDCP_SOURCE_CP_H__*/
