#ifndef _OMX_FileMux_H_
#define _OMX_FileMux_H_

/*==============================================================================
*        @file OMX_FileMux.h
*
*  @par DESCRIPTION:
*       This is the definition of the OMX interface for File Mux
*       It contains data types & interface calls exposed to the OMX base class
*
*
*  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

 $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/omx/mm-mux/omxmux/inc/omx_filemux.h#1 $


when        who         what, where, why
--------    ------         --------------------------------------------------------
07/01/09    sanal       Created file
================================================================================
*/

/*******************************************************************************
*                         INCLUDE FILES FOR MODULE
********************************************************************************
*/

#include "qmmList.h"
#include "MMCriticalSection.h"
#include "QOMX_FileFormatExtensions.h"
#include "QOMX_IVCommonExtensions.h"
#include "QOMX_AudioExtensions.h"
#include "qc_omx_component.h"
/*******************************************************************************
    VENDOR EXTENSIONS FOR FILE MUX COMPONENT
********************************************************************************
*/

//#define QOMX_QcomIndexParamContainerInfo 0x7F1FFFF0





/*******************************************************************************
*  \brief                      MACRO DEFINITIONS
********************************************************************************
*/

#define  OMX_MUX_INDEX_PORT_AUDIO                  0
#define  OMX_MUX_INDEX_PORT_VIDEO                  1
#define  OMX_MUX_INDEX_PORT_TEXT                   2
#define  OMX_MUX_INDEX_PORT_OUTPUT                 3
#define  OMX_MUX_INDEX_MAX_PORT                    4
#define  OMX_MUX_INDEX_PORT_NONE                  -2

#define  OMX_MUX_MAX_STREAMS                           3



#if defined WINCE || defined PLATFORM_LTK
  class __declspec( dllimport ) FileMux;
#define MMI_DLL __declspec( dllexport )
#else
  class FileMux;
  #define MMI_DLL
#endif

namespace video
{
  class iStreamPort;
}

extern "C" {
  OMX_API void * get_omx_component_factory_fn(void);
}
/*******************************************************************************
*  \brief                      CLASS DEFINITIONS
********************************************************************************
*/
/*!
  @brief   OMX_FileMux

  @detail This module gives a standard interface to OMX mux component  to access
  the Qualcomm movie file writer implementation. The private section  deals with
  the implementation details

*/
#define OMX_SPEC_VERSION 0x00020101

class OMX_FileMux : public qc_omx_component
{
public:

    /**********************************************************************//**
    * @brief Class destructor
    *************************************************************************/
    OMX_FileMux();
    virtual ~OMX_FileMux();
   /**********************************************************************//**
    * @brief Class constructor
    *************************************************************************/
    static OMX_FileMux* get_instance();


    /**********************************************************************//**
    * @brief Initializes the component
    *
    * @return error if unsuccessful.
    *************************************************************************/
    virtual OMX_ERRORTYPE component_init(OMX_IN OMX_STRING pComponentName);

    //////////////////////////////////////////////////////////////////////////
    /// For the following methods refer to corresponding function descriptions
    /// in the OMX_COMPONENTTYPE structure in OMX_Componenent.h
    //////////////////////////////////////////////////////////////////////////

    virtual OMX_ERRORTYPE get_component_version(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_OUT OMX_STRING pComponentName,
      OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
      OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
      OMX_OUT OMX_UUIDTYPE* pComponentUUID);

    virtual OMX_ERRORTYPE send_command(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_COMMANDTYPE Cmd,
      OMX_IN  OMX_U32 nParam1,
      OMX_IN  OMX_PTR pCmdData);

    virtual OMX_ERRORTYPE get_parameter(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_INDEXTYPE nParamIndex,
      OMX_INOUT OMX_PTR pComponentParameterStructure);


    virtual OMX_ERRORTYPE set_parameter(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_INDEXTYPE nIndex,
      OMX_IN  OMX_PTR pComponentParameterStructure);


    virtual OMX_ERRORTYPE get_config(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_INDEXTYPE nIndex,
      OMX_INOUT OMX_PTR pComponentConfigStructure);


    virtual OMX_ERRORTYPE set_config(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_INDEXTYPE nIndex,
      OMX_IN  OMX_PTR pComponentConfigStructure);


    virtual OMX_ERRORTYPE get_extension_index(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_STRING cParameterName,
      OMX_OUT OMX_INDEXTYPE* pIndexType);


    virtual OMX_ERRORTYPE get_state(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_OUT OMX_STATETYPE* pState);


    virtual OMX_ERRORTYPE component_tunnel_request(
      OMX_IN  OMX_HANDLETYPE hComp,
      OMX_IN  OMX_U32 nPort,
      OMX_IN  OMX_HANDLETYPE hTunneledComp,
      OMX_IN  OMX_U32 nTunneledPort,
      OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup);

    virtual OMX_ERRORTYPE use_buffer(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
      OMX_IN OMX_U32 nPortIndex,
      OMX_IN OMX_PTR pAppPrivate,
      OMX_IN OMX_U32 nSizeBytes,
      OMX_IN OMX_U8* pBuffer);

    virtual OMX_ERRORTYPE allocate_buffer(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
      OMX_IN OMX_U32 nPortIndex,
      OMX_IN OMX_PTR pAppPrivate,
      OMX_IN OMX_U32 nSizeBytes);

    virtual OMX_ERRORTYPE free_buffer(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_U32 nPortIndex,
      OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    virtual OMX_ERRORTYPE empty_this_buffer(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    virtual OMX_ERRORTYPE fill_this_buffer(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

    virtual OMX_ERRORTYPE set_callbacks(
      OMX_IN  OMX_HANDLETYPE hComponent,
      OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
      OMX_IN  OMX_PTR pAppData);

    virtual OMX_ERRORTYPE component_deinit(
      OMX_IN  OMX_HANDLETYPE hComponent);

    virtual OMX_ERRORTYPE use_EGL_image(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
      OMX_IN OMX_U32 nPortIndex,
      OMX_IN OMX_PTR pAppPrivate,
      OMX_IN void* eglImage);

    virtual OMX_ERRORTYPE component_role_enum(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_OUT OMX_U8 *cRole,
      OMX_IN OMX_U32 nIndex);

    OMX_ERRORTYPE OMX_FileMux_Command
    (
        OMX_HANDLETYPE hFd,
        OMX_U32 nCode,
        OMX_PTR pData
    );

    /***************************************************************************
    *  \brief Public Functions
    ****************************************************************************
    */

    /*==========================================================================

             FUNCTION:         OMX_FileMux_DriverCallback

             DESCRIPTION:
    *//**       @brief         Registered callback with File Mux layer
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    static void OMX_FileMux_DriverCallback
    (
        int   status,                    /**< Status from FileMux layer below */
        void *pClientData,               /**< MMI interface instance data     */
        void *pSampleInfo,               /**< Sample info that is processed   */
        void *pBuffer                    /**< Buffer to be released           */
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_Open

             DESCRIPTION:
    *//**       @brief         Creates MMI FileMux instance
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None

    @par     SIDE EFFECTS:
                               OMX_ErrorInsufficientResources
                               OMX_ErrorNone

    *//*======================================================================*/
    static OMX_U32 OMX_FileMux_Open(OMX_HANDLETYPE *pFd);

protected:
    /*! This   section   includes  the  implementation  details for FileMux  MMI
     *  interface
     */

    FileMux                    *pFileMux;   /**< Handle  o f   lower  layer  mux
                                             *   implementation
                                             */
    OMX_U32                     nFileFormat;           /**< Need to replace with
                                                        *   OMX extension TBD
                                                        */
    OMX_U64                 nFileSizeLimit;            /** File size limit */
    OMX_TICKS               nFileDurationLimit;        /** Movie duration
                                                        * limit
                                                        */

    QOMX_CONTAINER_FORMATTYPE   nFileBrand;            /**< Need to replace with
                                                        *   OMX extension TBD
                                                        */
    OMX_U32                     nFragmentDuration;     /**< Duration of each
                                                        *   fragment for
                                                        *   fragmented file
                                                        *   format
                                                        */
    QMM_ListHandleType          pCmdQueue;              /**< Command  queue  for
                                                        *   for component
                                                        */
    OMX_U32                     nAVInterlacePeriod;     /**< video and audio
                                                        *   interleaved after
                                                        *   this duration
                                                        */
    OMX_U16                    *pContentURI;           /**< Content URI
                                                        */
    OMX_U32                     nContentURISize;        /**< Size of Content URI
                                                        */


    video::iStreamPort         *pIStreamPort;          /**< IStreamPort pointer
                                                         * to support playback
                                                         * from IStreamPort
                                                         */

    OMX_BOOL                    bReorder;               /**< To reorder stream &
                                                         *   meta data or not
                                                         */
    OMX_BOOL                    bOutputLimitReached;   /**< Mux output limit
                                                         *  reached
                                                         */

    QMM_ListHandleType          pSampleInfoQueue;       /**< Lets use a queue
                                                        *   to handle sampleinfo
                                                        */

    void                        *pFileMuxParams;   /**< Structure to initialize
                                                    *  File Mux interface
                                                    */

    void                        *pFileMuxStreams;  /**< Stream information for
                                                    *  initializing File Mux
                                                    *  interface
                                                    */
    OMX_BOOL                    bMuxOpen;          /**< Maintains if Mux is open
                                                    * or not
                                                    */
    OMX_BOOL                    bUUIDWritten;      /**< Indicates if UUID is
                                                    * written to file.
                                                    */

    MM_HANDLE                   pCSHandle;         /**< Handle to video OSAL
                                                    */

    OMX_BOOL                    bStatus;           /**< Status of the current
                                                    * instance of MMI FileMux
                                                    */
    OMX_BUFFERHEADERTYPE       *pTmpVidBufferHdr;  /**< Temporarily holds
                                                    * video buffer header.
                                                    */
    OMX_U8                      nAACStreamHeader[2];

    OMX_TICKS                  nStatisticsInterval;

    QOMX_RECORDINGSTATISTICSTYPE nRecStatistics;


    typedef struct
    {
       QMM_ListLinkType                 pLink;
       OMX_BUFFERHEADERTYPE             *pBuffHdr;

    } OMX_FileMux_BuffHdrLinkType;

    /*!
    @brief Stream information.

    @detail
           This structure  will hold  all  stream information required to
           process the samples sequentially.
    */
    typedef struct OMX_FileMux_StreamInfo
    {
        OMX_TICKS nStartTime;           /**< Time stamp of the first sample   */
        OMX_TICKS nPrevTimeStamp;       /**< Time stamp of the previous sample*/
        OMX_U32   nCurrDelta;           /**< Delta of the current sample      */
        OMX_U32   nPrevDelta;           /**< Delta of previous sample         */
        OMX_BOOL  bHeaderReceived;      /**< Set if header received for stream*/
    }OMX_FileMux_StreamInfoType;


    OMX_FileMux_StreamInfoType sStreamInfo[OMX_MUX_MAX_STREAMS];
                                        /**< Stream specific information for
                                         *   each stream stored in this array
                                         */

    /*!
    @brief Media Format specific information.

    @detail
           This structure  will hold  all  configuration information specific
           to each codec type for each port.
    */
    typedef union OMX_FileMux_MediaFormatConfig
    {
        /*Audio Config Types*/
        OMX_AUDIO_PARAM_AACPROFILETYPE sAACInfo;
        QOMX_AUDIO_PARAM_AC3TYPE sAC3Info;
        OMX_AUDIO_PARAM_AMRTYPE sAMRInfo;
        OMX_AUDIO_PARAM_QCELP13TYPE sQCELPInfo;
        OMX_AUDIO_PARAM_EVRCTYPE sEVRCInfo;
        OMX_AUDIO_PARAM_PCMMODETYPE sPCMInfo;
        QOMX_AUDIO_PARAM_AMRWBPLUSTYPE sAMRWBInfo;

        /*Video Config Type*/
        OMX_VIDEO_PARAM_H263TYPE sH263Info;
        OMX_VIDEO_PARAM_MPEG4TYPE sMPEG4Info;
        OMX_VIDEO_PARAM_AVCTYPE  sAVCInfo;
    }OMX_FileMux_MediaFormatConfigType;

    /*!
    @brief Structure for keeping Audio/Video/Text port information.

    @detail
           This structure  will hold  all  configuration information  about each
           port used in  the file mux  driver. This includes buffer  information
           for all buffers allocated  or told to use for this port.
    @Note
           These arrays  will be initialized  in the  order ports of type audio,
           video, text as of now. The  implementation of this interface  can fix
           the number of ports of each class.

    */
    typedef struct OMX_FileMuxPortInfo
    {
        OMX_VERSIONTYPE       nVersion;       /**< OMX   specification  version
                                               *   information
                                               */
        OMX_U32               nPortNumber;    /**< Port number
                                               */
        OMX_PARAM_PORTDEFINITIONTYPE sPortDef;/**< Keeps config information  for
                                               *   each port
                                               */
        OMX_FileMux_MediaFormatConfigType sFormatSpecificInfo;
                                              /**< Information related to
                                               *  specific audio/video format
                                               */
        QMM_ListHandleType    pBufferQueue;   /**< Queue to  push buffer headers
                                               *   for this port
                                               */

        QMM_ListHandleType    pCmdQueue;      /**< Command  queue  for the port
                                               */
        OMX_U8                *pSyntaxHdr;    /**< Syntax Header for the port
                                                   media*/
        OMX_U32               nSyntaxHeaderLen;/**< Syntax Header length
                                                */
        OMX_BUFFERHEADERTYPE  *pBuffHeaderArray;/**< Buffer headers
                                                */
        OMX_BOOL              *pbComponentAllocated;

        OMX_BOOL              bPopulated;     /**< Populated or not
                                               */
        OMX_BOOL              bUnPopulated;

        OMX_U32               nNumBuffAllocated;

        OMX_BOOL              bDisableRequested;

        OMX_BOOL              bEnableRequested;

    }OMX_FileMuxPortInfoType;


    OMX_FileMuxPortInfoType  *arrPortConfig;  /**< This is the pointer  to array
                                               *   of  port  configurations
                                               *   Number  of total ports
                                               *   is  fixed  by  implementation
                                               */

    OMX_U32                   nNumPorts[OMX_MUX_INDEX_MAX_PORT];
                                              /**< Number  of ports of each type
                                               */

    /*!
    @brief Structure for keeping Audio/Video/synchronozation.

    @detail
           This structure  will hold  all  timeing information for audio and
           video streams which is used for AV Sync.
    @Note


    */
    typedef struct OMX_FileMuxAVSyncInfo
    {
        OMX_TICKS    nVideoStartTime;                  /**< Time stamp of first
                                                        * Sample
                                                        */
        OMX_TICKS    nAudioStartTime;                  /**< Time stamp of first
                                                        * Sample
                                                        */
        OMX_BOOL     bAVSyncDone;                      /**< AV Sync has been
                                                        * performed
                                                        */
        OMX_BOOL     bVideoStarted;                    /**< Received a sample
                                                        * at video input port
                                                        */
        OMX_BOOL     bAudioStarted;                    /**< Received a sample
                                                        * at audio input port
                                                        */
        OMX_U64      nAVTimeDiff;                      /**< Time difference
                                                        * between audio and
                                                        * video start times
                                                        */
        OMX_U64      nCurrentAudioTime;                /**< Curent time duration
                                                        * of audio stream in
                                                        * microseconds
                                                        */
        OMX_U64      nCurrentVideoTime;                /**< Curent time duration
                                                        * of video stream in
                                                        * micro seconds
                                                        */
        OMX_S64      nAVTimeDiffAdjust;                /**< Time in ms to be
                                                        * adjusted at start of
                                                        * recording. Can be used
                                                        * for AVSync or for
                                                        * delay compensation in
                                                        * audio or video
                                                        */
        OMX_BOOL     bVideoStreamEnded;                /**< EOS for Video came*/
        OMX_BOOL     bAudioStreamEnded;                /**< EOS for Audio came*/
    }OMX_FileMuxAVSyncInfoType;

    OMX_FileMuxAVSyncInfoType   sAVSyncInfo;


    /*!
    @brief Structure for keeping  Mux Statistics.

    @detail
           This structure  will maintain statistics information for audio and
           video streams
    @Note


    */
    typedef struct OMX_FileMuxStatistics
    {
        OMX_U32 nNumVidFramesWritten;
        OMX_U32 nNumVidFramesDropped;
        OMX_U32 nNumAudFramesWritten;
        OMX_U32 nNumAudFramesDropped;
        OMX_U32 nSilentFramesInserted;
        OMX_U32 nVideoDuration;
        OMX_U32 nAudioDuration;
        OMX_U32 nVideoBytes;
        OMX_U32 nAudioBytes;
        OMX_U32 nAudioCodec;
        OMX_U32 nVideoCodec;
        OMX_U8 *pVideoCodec;
        OMX_U8 *pAudioCodec;
        OMX_U8 *pRole;
    }OMX_FileMuxStatistics;

    OMX_FileMuxStatistics sMuxStats;



    QMM_ListHandleType        pMediaInfoQueue; /**< Queue to  push buffer
                                                *  media information
                                                */

    typedef struct
    {
        QMM_ListLinkType                 pLink;
        QOMX_MEDIAINFOTYPE*              pMediaInfo;

    } OMX_FileMux_MediaInfoLinkType;


    typedef struct
    {
       OMX_U32                 nPortIndex;       /**<
                                                 * Port number on which this
                                                 * command should be applied.
                                                 */

       OMX_BUFFERHEADERTYPE   *pBufferHdr;       /**<
                                                 * Buffer header encapsulating
                                                 * the buffer which need to be
                                                 * processed and the port with
                                                 * which this buffer is
                                                 * associated.
                                                 */

    } OMX_Mux_BufferCmdType;


    typedef struct
    {
        OMX_U32 nExtraOffset;                    /**< Offset where extradata
                                                  * Starts
                                                  */
        OMX_U32 nExtraSize;                      /**< Size of extra info
                                                  */
        OMX_U8* pExtra;                          /**< Ptr to extra info
                                                  */
    }OMX_Mux_ExtraInfo;

    OMX_STRING          m_pComponentName;
    OMX_STATETYPE       m_eState;
    OMX_STATETYPE       m_eTargetState;
    OMX_CALLBACKTYPE    *m_pCallbacks;
    OMX_CALLBACKTYPE    m_sCallbacks;
    OMX_PTR             m_pAppData;
    OMX_HANDLETYPE      m_hSelf;
    OMX_Mux_ExtraInfo   m_HDCPInfo;
    OMX_Mux_ExtraInfo   m_FmtPvtInfo;
    QOMX_ENCRYPTIONTYPE nEncryptTypeConfigParameters;
    /*--------------------------------------------------------------------------
                           Private functions
    ----------------------------------------------------------------------------
    */

    /*==========================================================================

             FUNCTION:         MMI_FileMux_CopyString

             DESCRIPTION:
    *//**       @brief         Following function copies an array of specified
                               size to another array or a string to another.
                               Source and destination can be of different data
                               types

    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               Number of bytes left in the destination,
                               excluding null character for string copy.


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    template <typename srcChar, typename destChar>
    int OMX_FileMux_CopyString(destChar *pDst, srcChar *pSrc, int nSize)
    {
        if(!pDst || !pSrc || !nSize)
        {
            return nSize;
        }
        do
        {
            *pDst  = (destChar)*pSrc++;
        }
        while(*pDst++ !=0 && --nSize);
        if(!nSize)
        {
            pDst[-1] = (destChar)0;
        }
        return nSize;
    }


/*==============================================================================

         FUNCTION:         OMX_FileMux_Close

         DESCRIPTION:
*//**       @brief         Closes the MMI layer instance for  file mux
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_U32 OMX_FileMux_Close(OMX_HANDLETYPE hFd);

    /*==========================================================================

             FUNCTION:         OMX_FileMux_GetSetParam

             DESCRIPTION:
    *//**       @brief         This function Sets/Gets openmax defined
                               technology specific parameters.

    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE or appropriate error code


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_GetSetParam
    (
        int           nCode,
        OMX_INDEXTYPE eIndex,
        void          *pParam
    );


    /*==========================================================================

             FUNCTION:         OMX_FileMux_GetSetCustomParam

             DESCRIPTION:
    *//**       @brief         This function Gets/Sets OMX custom params.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               MMI_S_COMPLETE or appropriate error codes


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_GetSetCustomParam
    (
        int     nCode,
        OMX_U32 nParamIndex,
        void*   pComponentParameterStructure
    );

    /*==========================================================================

             FUNCTION:         MMI_FileMux_GetExtensionIndex

             DESCRIPTION:
    *//**       @brief         Gives the extension index of a string.

    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               MMI_S_COMPLETE or appropriate error code


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_GetExtensionIndex
    (
        OMX_STRING     cParamName,
        OMX_INDEXTYPE  *pIndex
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_UseBuffer

             DESCRIPTION:
    *//**       @brief         Client passes the buffer information for each
                               buffer that is going to be used for the port by
                               these calls.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
 /*   int OMX_FileMux_UseBuffer
    (
        MMI_UseBufferCmdType *
    );*/

    /*==========================================================================

             FUNCTION:         OMX_FileMux_AllocBuffer

             DESCRIPTION:
    *//**       @brief         Allocates memory for port buffers as requested by
                               client.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE or OMX_S_ENOSWRES


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
  /*  int OMX_FileMux_AllocBuffer
    (
        OMX_AllocBufferCmdType *
    );*/


    /*==========================================================================

             FUNCTION:         OMX_FileMux_FreeBuffer

             DESCRIPTION:
    *//**       @brief         Free the memory allocated for port buffers on
                               each call. This will be called only if this layer
                                has allocated the memory.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
 /*   int OMX_FileMux_FreeBuffer
    (
        OMX_FreeBufferCmdType *
    );*/
    OMX_ERRORTYPE Set_Target_State(OMX_STATETYPE eState);
    /*==========================================================================

             FUNCTION:         OMX_FileMux_QueueStreamBuffer

             DESCRIPTION:
    *//**       @brief         This function queues stream buffer to inner layer
                                for decode.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_QueueStreamBuffer
    (
        OMX_Mux_BufferCmdType *
    );


    /*==========================================================================

             FUNCTION:         OMX_FileMux_QueueAudioStreamBuffer

             DESCRIPTION:
    *//**       @brief         This function queues stream buffer to inner layer
                                for decode for audio stream.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_QueueAudioStreamBuffer
    (
        OMX_Mux_BufferCmdType *
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_DeQueueAudioBuffers

             DESCRIPTION:
    *//**       @brief         This function returns the buffer or send
                               buffers in queue to lower layer.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         bProcess: OMX_TRUE if buffers need to be sent to
                                                   filemux
                                         OMX_FALSE if buffers need to be just
                                                   returned.

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_DeQueueAudioBuffers
    (
        OMX_BOOL bProcess,
        OMX_U32  nPortIndex
    );
    /*==========================================================================

             FUNCTION:         OMX_FileMux_PushSampleToFileMux

             DESCRIPTION:
    *//**       @brief         This function queues stream buffer to inner
                               layer for muxing.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_PushSampleToFileMux
    (
        OMX_U32                 nStreamNum,
        OMX_U32                 nDelta,
        OMX_BUFFERHEADERTYPE   *pBuffHdr

    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_DoAVSyncCoarse

             DESCRIPTION:
    *//**       @brief         This function does AVsync at a higher level. Drop
                               buffer headers and return it back, if required.
                               However this does nt look into the content of the
                               buffer.

    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         nPortIndex

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_DoAVSyncCoarse
    (
        OMX_U32  nPortIndex
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_DoAVSyncFine

             DESCRIPTION:
    *//**       @brief         This function does AVsync at a buffer level.
                                Drops  frames as required from within a buffer.

    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         nPortIndex
                               nBuffHdr

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_DoAVSyncFine
    (
        OMX_U32               nPortIndex
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_DoAVSyncFineADTS

             DESCRIPTION:
    *//**       @brief         This function does AVsync at a buffer level.
                                Drops  frames as required from within a buffer
                                for ADTS.

    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         nPortIndex
                               nBuffHdr

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_DoAVSyncFineADTS
    (
        OMX_U32               nPortIndex
    );
    /*==========================================================================

             FUNCTION:         OMX_FileMux_ProcessExtraData

             DESCRIPTION:
    *//**       @brief         Process any extra data at the end of buffer.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         OMX_BUFFERHEADERTYPE

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE or appropriate error


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_ProcessExtraData
    (
        OMX_BUFFERHEADERTYPE   *pBuffHdr
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_ReplaceAVCStartCodes

             DESCRIPTION:
    *//**       @brief         Replace NAL startcodes with NAL size for h264.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         OMX_BUFFERHEADERTYPE

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE or appropriate error


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_ReplaceAVCStartCodes
    (
        OMX_BUFFERHEADERTYPE    *pBuffHdr,
        OMX_OTHER_EXTRADATATYPE *pExtra
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_AVSyncFindNumAudioPacketsToInsert

             DESCRIPTION:
    *//**       @brief         This function finds the number of auido packets
                               of silence to insert.

    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         nPortIndex

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_U32 OMX_FileMux_AVSyncFindNumAudioPacketsToInsert
    (
        OMX_U32  nPortIndex
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_AVSyncInsertSilentAudioPackets

             DESCRIPTION:
    *//**       @brief         Inserts the desired amount of silent packets for
                               doing AVSync.

    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         nPortIndex

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*========================================================================
    */
    OMX_ERRORTYPE OMX_FileMux_AVSyncInsertSilentAudioPackets
    (
        OMX_U32  nportindex,
        OMX_U32  npacketinsert
    );


    /*==========================================================================

             FUNCTION:         OMX_FileMux_QueueVideoStreamBuffer

             DESCRIPTION:
    *//**       @brief         This function queues stream buffer to inner layer
                                for decode for audio stream.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_QueueVideoStreamBuffer
    (
        OMX_Mux_BufferCmdType *
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_Flush

             DESCRIPTION:
    *//**       @brief         Flushes the buffers waiting to be processed on a
                               particular port. Return unused buffers back to
                               client.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param

                nPortIndex[in] : Port on which flush should be done


    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_Flush
    (
        OMX_U32  nPortIndex
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_Start

             DESCRIPTION:
    *//**       @brief         This function makes all resources ready for
                               execution . In our case we need to make File Mux
                               layer ready for processing.
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after all set
                               parameters have been successfully performed.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_Start
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_Stop

             DESCRIPTION:
    *//**       @brief         Stop processing. The settings should be retained.
                               After giving another start another new session
                               should be possible.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_Stop
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_Pause

             DESCRIPTION:
    *//**       @brief         Pauses processing in the File Mux layer.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE on success
                               OMX_S_EINVALSTATE on failure


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_Pause
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_Resume

             DESCRIPTION:
    *//**       @brief         Resume processing in the File Mux layer.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE on success
                               OMX_S_EINVALSTATE on failure


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_Resume
    (
        void
    );

    /*!
    @brief If client gives header in stream we need to extract it.

    @detail
           This may  be  required  in  partial  frame cases or start of session.
    */
    OMX_U32 OMX_FileMux_FindHeaderOffset
    (
        OMX_U32 pattern,               /**< Marker of header pattern to search*/
        OMX_U8  pattern_length,        /**< Length of pattern in bits         */
        OMX_U8* p_buffer               /**< Pointer to stream data            */
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_ValidateFileFormat

             DESCRIPTION:
    *//**       @brief         This function validates audio video codec support
                               with the file brand set.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_TRUE or OMX_FALSE


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_ValidateFileFormat
    (
        QOMX_CONTAINER_FORMATTYPE fileFormat,
        OMX_AUDIO_CODINGTYPE audioCoding,
        OMX_VIDEO_CODINGTYPE videoCoding
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_DisablePort

             DESCRIPTION:
    *//**       @brief         A particular port is disabled by this call. Free
                               buffer calls may follow this and port may be
                               depopulated.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_EnablePort
    (
        OMX_U32 nPortIndex
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_DisablePort

             DESCRIPTION:
    *//**       @brief         A particular port is disabled by this call. Free
                               buffer calls may follow this and port may be
                               depopulated.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_DisablePort
    (
        OMX_U32 nPortIndex
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_SetMediaParams

             DESCRIPTION:
    *//**       @brief         Set the codec specific params for audio and video
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE or appropriate error


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_SetMediaParams
    (
        OMX_INDEXTYPE eMediaIndex,
        void *pSrcStruct,
        void *pDstStruct
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_CreateFileMux

             DESCRIPTION:
    *//**       @brief         This function creates the lower fileMux layer.
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after all set
                               parameters have been successfully performed.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE or appropriate error code


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_CreateFileMux
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_LoadResources

             DESCRIPTION:
    *//**       @brief         All resources required for processing are
                                allocated in this call.
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after all set
                               parameters have been successfully performed.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE or error codes


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_LoadResources
    (
        void
    );


    /*==========================================================================

             FUNCTION:         OMX_FileMux_ReleaseResources

             DESCRIPTION:
    *//**       @brief         All resources required for processing are
                               released in this call.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE or error codes


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_ReleaseResources
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_CloseFileMux

             DESCRIPTION:
    *//**       @brief         Closes the file Mux instance.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_S_COMPLETE or error codes


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_CloseFileMux
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_PopulateFileMuxParams

             DESCRIPTION:
    *//**       @brief         This function populates the structures
                                required to initialize FileMux Interface
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after all set
                               parameters have been successfully performed.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               FALSE - on failure \n
                               TRUE - on success \n


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_PopulateFileMuxParams
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_ConvertOMXBrandToFileMuxBrand

             DESCRIPTION:
    *//**       @brief         This function converts file brand enums from
                               OMX type to file mux type.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               Mux_brand_type


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    int OMX_FileMux_ConvertOMXBrandToFileMuxBrand
    (
        void
    );


    /*==========================================================================

             FUNCTION:         OMX_FileMux_ProcessFileMuxCb

             DESCRIPTION:
    *//**       @brief         Process callback from FileMux driverlayer
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    void OMX_FileMux_ProcessFileMuxCb
    (
        int   status,                    /**< Status from FileMux layer below */
        void *pClientData,               /**< OMX interface instance data     */
        void *pSampleInfo,               /**< Sample info that is processed   */
        void *pBuffer                    /**< Buffer to be released           */
    );


    /*==========================================================================

             FUNCTION:         OMX_FileMux_PopulateVideoCommonParams

             DESCRIPTION:
    *//**       @brief         Populate video codec independent params required
                               for initializing FileMux
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after all set
                               parameters have been successfully performed.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               TRUE or FALSE


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_PopulateVideoCommonParams
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_PopulateVideoCodecSpecificParams

             DESCRIPTION:
    *//**       @brief         Populate video codec specific params required for
                               initializing FileMux
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after all set
                               parameters have been successfully performed.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               TRUE or FALSE


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_PopulateVideoCodecSpecificParams
    (
        void
    );


    /*==========================================================================

             FUNCTION:         OMX_FileMux_PopulateAudioCommonParams

             DESCRIPTION:
    *//**       @brief         Populate audio codec independent parameters
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after all set
                               parameters have been successfully performed.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               TRUE or FALSE


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_PopulateAudioCommonParams
    (
        void
    );


    /*==========================================================================

             FUNCTION:         OMX_FileMux_PopulateAudioCodecSpecificParams

             DESCRIPTION:
    *//**       @brief         Initialize audio codec specific params required
                               for FileMux
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after all set
                               parameters have been successfully performed.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_PopulateAudioCodecSpecificParams
    (
        void
    );

    /*==========================================================================

         FUNCTION:         OMX_FileMux_GetAudioBitrate

         DESCRIPTION:
    *//**       @brief         Returns the audio bitrate based on settings
    *//**

    @par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
    *//*
         PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                           OMX_U32 bitrate


    @par     SIDE EFFECTS:
                           None

    *//*======================================================================*/
    OMX_U32 OMX_FileMux_GetAudioBitrate
    (
        void
    );


    /*==============================================================================

             FUNCTION:         OMX_FileMux_PrintStatistics

             DESCRIPTION:
    *//**       @brief         Prints recprding statistics at end of recording.
    *//**

    @par     DEPENDENCIES:

    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return


    @par     SIDE EFFECTS:
                               None

    *//*==========================================================================*/
    void OMX_FileMux_PrintStatistics
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_WriteMediaInfo

             DESCRIPTION:
    *//**       @brief         Writes Media Information
    *//**

    @par     DEPENDENCIES:
                           This function can be called now in executing state
                            only. //TBD
    *//*
              PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                           OMX_U32 bitrate


     @par     SIDE EFFECTS:
                           None

    *//*======================================================================*/
     OMX_ERRORTYPE OMX_FileMux_WriteMediaInfo
    (
         QOMX_MEDIAINFOTYPE *pMediaInfo
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_InitMediaInfo

             DESCRIPTION:
    *//**       @brief         Initialize mediainfo queue
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_InitMediaInfo
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_DeInitMediaInfo

             DESCRIPTION:
    *//**       @brief         DeInitialize mediainfo queue
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    void OMX_FileMux_DeInitMediaInfo
    (
        void
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_MediaInfoAddItem

             DESCRIPTION:
    *//**       @brief         Updates media Info in queue.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_MediaInfoAddItem
    (
        QOMX_MEDIAINFOTYPE* pMediaInfo
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_MediaInfoSearchUUIDAndReplace

             DESCRIPTION:
    *//**       @brief         Updates media Info cache.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_MediaInfoSearchUUIDAndReplace
    (
        QOMX_MEDIAINFOTYPE* pMediaInfo
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_MediaInfoSearchUUIDAndReplace

             DESCRIPTION:
    *//**       @brief         Updates media Info cache.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_BOOL OMX_FileMux_MediaInfoSearchItemAndReplace
    (
        QOMX_MEDIAINFOTYPE* pMediaInfo
    );

    /*==========================================================================

             FUNCTION:         OMX_FileMux_MediaInfoFetchItemWithTag

             DESCRIPTION:
    *//**       @brief         Fetch already pushed Item with the tag.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_FileMux_MediaInfoLinkType* OMX_FileMux_MediaInfoFetchItemWithTag
    (
        QOMX_MEDIAINFOTYPE* pMediaInfo
    );

    /*==========================================================================

             FUNCTION:         MMI_FileMux_MediaInfoUpdate

             DESCRIPTION:
    *//**       @brief         Updates media Info cache.
    *//**

    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    int MMI_FileMux_MediaInfoUpdate
    (
        QOMX_MEDIAINFOTYPE* pMediaInfo
    );
    /*==========================================================================

             FUNCTION:         MMI_FileMux_WriteMediaInfo

             DESCRIPTION:
    *//**       @brief         Writes Media Info to file
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after moving to
                  executing state.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_U32 bitrate


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    int MMI_FileMux_WriteMediaInfoAll
    (
        void
    );

    /*==========================================================================

             FUNCTION:         MMI_FileMux_WriteMediaInfoUUIDAll

             DESCRIPTION:
    *//**       @brief         Writes Media Info to file
    *//**

    @par     DEPENDENCIES:
                               This function can be called only after moving to
                  executing state.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_U32 bitrate


    @par     SIDE EFFECTS:
                               None

    *//*======================================================================*/
    OMX_ERRORTYPE OMX_FileMux_WriteMediaInfoUUIDAll
    (
        void
    );


    /*==========================================================================

             FUNCTION:         OMX_FileMux_GetH263Level

             DESCRIPTION:
    *//**       @brief         Returns the H263 level from settings
    *//**

    @par     DEPENDENCIES:
                               This function uses fps bitrate and resolution to
                               get the level information.
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_U32 level


    @par     SIDE EFFECTS:
                               None

    *//*========================================================================
    */

    OMX_U32 OMX_FileMux_GetH263Level
    (
        OMX_U32 width,
        OMX_U32 height,
        OMX_U32 framePerSec,
        OMX_U32 bitrate,
        OMX_U32 profile
    );
};

#endif /*__OMX_FileMux_H__*/
