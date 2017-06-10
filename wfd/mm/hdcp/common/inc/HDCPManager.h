#ifndef __HDCP_MANAGER_H__
#define __HDCP_MANAGER_H__
/*==============================================================================
*       HDCPManager.h
*
*  DESCRIPTION:
*       Manager class for HDCP
*
*
*  Copyright (c) 2012-2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/
#include "stdio.h"
#include "MMThread.h"
#include <utils/String8.h>

//#include "DX_Hdcp_Receiver.h"
//#include "DX_Hdcp_Errors.h"
//#include "DX_VOS_Errors.h"
/*==============================================================================

                        DATA DECLARATIONS

================================================================================
*/
/*------------------------------------------------------------------------------
** Constant / Define Declarations
**------------------------------------------------------------------------------
*/
#define HDCP_AUDIO_TRACK_ID 1
#define HDCP_VIDEO_TRACK_ID 0
/*------------------------------------------------------------------------------
** Forward Declarations
**------------------------------------------------------------------------------
***/

/*------------------------------------------------------------------------------
** Type Declarations
**------------------------------------------------------------------------------
***/
typedef enum HDCPMode
{
    HDCP_MODE_TX,
    HDCP_MODE_RX,
    HDCP_MODE_UNKNOWN = 0xFFFFFFFF
}HDCPModeType;

typedef enum HDCPStatus
{
    HDCP_FAIL,
    HDCP_BAD_PARAMS,
    HDCP_SUCCESS,
    HDCP_IN_PROGRESS,
    HDCP_INIT_COMPLETE,
    HDCP_UPSTREAM_CLOSE,
    HDCP_DOWNSTREAM_CLOSE,
    HDCP_UNAUTHENTICATED_CONNECTION,
    HDCP_UNAUTHORIZED_CONNECTION,
    HDCP_REVOKED_CONNECTION,
    HDCP_INVALID_STATE,
    HDCP_INVALID_STATUS = 0xFFFFFFFF
}HDCPStatusType;

typedef enum HDCPState
{
    HDCP_STATE_DEINIT,
    HDCP_STATE_INIT,
    HDCP_STATE_CONNECTING,
    HDCP_STATE_DISCONNECTING,
    HDCP_STATE_CONNECTED,
    HDCP_STATE_PROCESSING,
    HDCP_STATE_ERROR = 0xFFFFFFFF
}HDCPStateType;

typedef enum HDCPCocec
{
     HDCP_CODEC_AAC = 1,
     HDCP_CODEC_AC3,
     HDCP_CODEC_LPCM,
     HDCP_CODEC_AVC
}HDCPCodecType;

typedef void (*HDCPEventCbType)(HDCPStateType eState,
                                  HDCPStatusType eStatus,
                                  void *pUserData);


/*------------------------------------------------------------------------------
** Global Constant Data Declarations
**------------------------------------------------------------------------------
***/

/*------------------------------------------------------------------------------
** Global Data Declarations
**------------------------------------------------------------------------------
***/

/*==============================================================================
**                          Macro Definitions
**==============================================================================
***/


/*==============================================================================
**                        Class Declarations
**==============================================================================
***/
class HDCPManager
{
public:

    HDCPManager();

    ~HDCPManager();

    HDCPStatusType registerCallback
    (
        HDCPEventCbType pCb,
        void *pUsrData
    );

    HDCPStateType  getHDCPManagerState();

    HDCPStatusType initializeHDCPManager();

    HDCPStatusType deinitializeHDCPManager();

    HDCPStatusType setupHDCPSession
    (
        HDCPModeType eMode,
        char *pIpAddr,
        unsigned short portNum
    );

    HDCPStatusType teardownHDCPSession();

    void  HDCPManThread( void );

    HDCPStatusType decrypt
    (
        const unsigned char *pesPrivateData,
        const unsigned char *pIn,
        unsigned char *pOut,
        unsigned long nLen
    );

    HDCPStatusType decrypt
    (
        const unsigned char *pesPrivateData,
        const unsigned char *pIn,
        unsigned char *pOut,
        unsigned long nLen,
        int nStreamId
    );

    HDCPStatusType setParameter
    (
        int paramID,
        void *paramData
    );

   void setStreamType(int nStreamType); // Set the stream Type
   bool isCodecInfoSet(int iMedia); // if codec info is set or not
   void constructCodecAndStreamType(int nStream, char *msg2); // constructs the codec and stream combination

private:
    static int  threadEntry
    (
        void* ptr
    );

    void initData();

    static void DXCallBack
    (
        int status,
        void *arg1 ,
        void *arg2
    );
    HDCPStateType m_eState;
    FILE *fp;
    MM_HANDLE m_pHDCPManThread;
    MM_HANDLE m_pHDCPManSignalQ;
    MM_HANDLE m_pHDCPManCloseSignal;
    MM_HANDLE m_pListenSignal;
    MM_HANDLE m_pConnectSignal;
    MM_HANDLE m_pDisconnectSignal;

    static const int m_nHDCPManCloseEvent;
    static const int m_nHDCPManListenEvent;
    static const int m_nHDCPManConnectEvent;
    static const int m_nHDCPManDisconnectEvent;
    unsigned short m_nPortNum;
    unsigned int m_ipAddr;
    unsigned char  m_ipAddrArray[4];
    HDCPEventCbType m_pEventCb;
    void *m_pUsrData;
    HDCPModeType m_eMode;

    static HDCPStatus m_eTopologyStatus; // keeps track of topology update status
    static bool m_bPropagationStatus; // keeps track of propagation status from DX
    static MM_HANDLE m_hCriticalSect;

    bool m_bAudioCodecInfoAvailable;
    bool m_bVideoCodecInfoAvailable;

    unsigned int m_nRetryCount; //tetry count for Event CIPHER ENABLE/DISABLE
    int m_nStreamType; //Audio/Video steam information will be stored
    int m_nAudioStreamType; // If Audio codec information is updated or not
    int m_nVideoStreamType; // Video codec information is updated or not
    // CB from Display HDCP manager to get topology update
    static int DISPCallBack(int eventType, void *arg1);
    static void hdcp_update_topology(void *msg);

};
#endif/*__HDCP_SINK_MANAGER_H__ */
