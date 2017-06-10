/* =======================================================================
                              WFDMMSourceVideoSource.cpp
DESCRIPTION

This module is for WFD source implementation. Takes care of interacting
with Encoder (which in turn interacts with capture module).

Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */


/* =======================================================================
                             Edit History
   When            Who           Why
-----------------  ------------  -----------------------------------------------
12/27/2013                       InitialDraft
========================================================================== */

/*========================================================================
 *                             Include Files
 *==========================================================================*/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WFDMMSourceVideoSource"

#include "WFDMMSourceSignalQueue.h"
#include "WFDMMSourceVideoSource.h"
#include "MMDebugMsg.h"
#include "WFDMMLogs.h"
#include "WFDMMSourcePmem.h"
#include "QOMX_VideoExtensions.h"
#include "WFD_HdcpCP.h"
#include "wfd_cfg_parser.h"
#include "MMMalloc.h"
#include "MMMemory.h"
#include <cutils/properties.h>
#include "WFDUtils.h"

#ifndef WFD_ICS
#include "common_log.h"
#endif
#include <threads.h>
#define MM_MSG_PRIO_MEDIUM MM_MSG_PRIO_ERROR

/*========================================================================
 *                          Defines/ Macro
 *==========================================================================*/
#define VIDEO_PVTDATA_TYPE_FILLERNALU           0
#define VIDEO_PVTDATA_TYPE_FILLERNALU_ENCRYPTED 1
#define PES_PVT_DATA_LEN 16
#define BUFFER_EXTRA_DATA 1024

#define WFD_H264_PROFILE_CONSTRAINED_HIGH 2
#define FRAME_SKIP_FPS_VARIANCE 20
// Frame skipping delay
uint32 video_frame_skipping_start_delay = 0;

/**!
 * @brief Helper macro to set private/internal flag
 */
#define FLAG_SET(_pCtx_, _f_) (_pCtx_)->nFlags  |= (_f_)

/**!
 * @brief Helper macro to check if a flag is set or not
 */
#define FLAG_ISSET(_pCtx_, _f_) (((_pCtx_)->nFlags & (_f_)) ? OMX_TRUE : OMX_FALSE)

/**!
 * @brief Helper macro to clear a private/internal flag
 */
#define FLAG_CLEAR(_pCtx_, _f_) (_pCtx_)->


OMX_U8 VideoSource::sFillerNALU[FILLER_NALU_SIZE] =
{0x00, 0x00, 0x00, 0x01, 0x0c, 0xff, 0xff, 0x80};

//This macro provides a single point exit from function on
//encountering any error
#define WFD_OMX_ERROR_CHECK(result,error) ({ \
    if(result!= OMX_ErrorNone) \
    {\
        WFDMMLOGE(error);\
        WFDMMLOGE1("due to %x", result);\
        goto EXIT;\
    }\
})


/*!*************************************************************************
 * @brief     CTOR
 *
 * @param[in] NONE
 *
 * @return    NONE
 *
 * @note
 **************************************************************************/
VideoSource::VideoSource():
    m_nNumBuffers(0),
    m_pHdcpOutBuffers(NULL),
    m_pVideoSourceOutputBuffers(NULL),
    m_pHDCPOutputBufQ(NULL),
    m_nMuxBufferCount(0),
    m_bStarted(OMX_FALSE),
    m_bPause(OMX_FALSE),
    m_pFrameDeliverFn(NULL),
    ionfd(-1),
    m_secureSession(OMX_FALSE),
    m_nFillerInFd(-1),
    m_nFillerOutFd(-1),
    m_hFillerInHandle(0),
    m_hFillerOutHandle(0),
    m_pFillerDataInPtr(NULL),
    m_pFillerDataOutPtr(NULL),
    m_bFillerNaluEnabled(OMX_TRUE),
    m_pVencoder(NULL),
    m_bDropVideoFrame(OMX_FALSE)
{
    WFDMMLOGE("Creating VideoSource...");
    m_eVideoSrcState    = WFDMM_VideoSource_STATE_INIT;
    m_pHdcpHandle       = NULL;
    m_bHdcpSessionValid = OMX_FALSE;
    m_pHdcpOutBuffers   = NULL;
    /**---------------------------------------------------------------------
         Decision to encrypt non secure content or not is made by application
         or user based on the WFD config file
        ------------------------------------------------------------------------
       */
    int nVal;
    nVal = 0;
    // CHeck if Filler NALU is disabled
    getCfgItem(DISABLE_NALU_FILLER_KEY,&nVal);
    if(nVal == 1)
    {
        m_bFillerNaluEnabled = OMX_FALSE;
    }
#ifdef ENABLE_WFD_STATS
    memset(&wfdStats,0,sizeof(wfd_stats_struct));
    m_pStatTimer = NULL;
    wfdStats.bEnableWfdStat = OMX_TRUE;
    m_nDuration = 5000;

    if(0 != MM_Timer_Create((int) m_nDuration, 1, readStatTimerHandler, (void *)(this), &m_pStatTimer))
    {
        WFDMMLOGE("Creation of timer failed");
    }
#endif

    m_bEnableProfiling = WFDMMSourceStatistics::isProfilingEnabled();

    if(m_bEnableProfiling)
    {
        m_pWFDMMSrcStats = WFDMMSourceStatistics::getInstance();
    }

    m_pVencoder = MM_New(WFDMMSourceVideoEncode);

    if(!m_pVencoder)
    {
        WFDMMLOGE("Failed to create Video Encoder");
        return;
    }
    return;
}

/*!*************************************************************************
 * @brief     DTOR
 *
 * @param[in] NONE
 *
 * @return    NONE
 *
 * @note
 **************************************************************************/
VideoSource::~VideoSource()
{
    #ifdef ENABLE_WFD_STATS
    // WFD:STATISTICS -- start
    if(wfdStats.bEnableWfdStat && 0 != wfdStats.nStatCount)
    {
       MM_MSG_PRIO2(MM_STATISTICS, MM_PRIO_MEDIUM,
                   "WFD:STATISTICS: Average Roundtrip time of buffer \
                   in Userspace is =%llu ms Max time is =%llu ms",
                   wfdStats.nCumulativeStatTime/wfdStats.nStatCount,
                   wfdStats.nMaxStatTime);
    }
    if(m_pStatTimer != NULL)
    {
       MM_Timer_Release(m_pStatTimer);
    }
    // WFD:STATISTICS -- end
    #endif /* ENABLE_WFD_STATS */
    OMX_ERRORTYPE result = OMX_ErrorNone;
    int timeoutCnt=1000;/*timeout counter*/
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceVideoSource::~VideoSource()");

    if(m_pVencoder)
    {
        MM_Delete(m_pVencoder);
    }

    if(m_eVideoSrcState!=  WFDMM_VideoSource_STATE_IDLE)
    {
      while((m_eVideoSrcState !=  WFDMM_VideoSource_STATE_IDLE)&&timeoutCnt)
      {
        MM_Timer_Sleep(5);
        timeoutCnt--;
      }
    }

    if(m_bHdcpSessionValid)
    {
        DeallocateHDCPResources();
    }

    if(m_pWFDMMSrcStats)
    {
        WFDMMSourceStatistics::deleteInstance();
        m_pWFDMMSrcStats = NULL;
    }

    WFDMMLOGH("~WFDMMSourceVideoSource completed");
}

/*!*************************************************************************
 * @brief     Configures the source
 *
 * @param[in] nFrames The number of frames to to play.
 * @param[in] nBuffers The number of buffers allocated for the session.
 * @param[in] pFrameDeliverFn Frame delivery callback.
 * @param[in] bFrameSkipEnabled frame skipping enabled
 * @param[in] nFrameSkipLimitInterval frame skipping time interval
 *
 * @return    OMX_ERRORTYPE
 *
 * @note
 **************************************************************************/
OMX_ERRORTYPE VideoSource::Configure(
    VideoEncStaticConfigType* pConfig,
    OMX_S32 nBuffers,
    FrameDeliveryFnType pFrameDeliverFn,
    eventHandlerType pEventHandlerFn,
    OMX_BOOL bFrameSkipEnabled,
    OMX_U64 nFrameSkipLimitInterval,
    OMX_U32 nModuleId,
    void *appData)
{
    (void)nBuffers;
    (void)bFrameSkipEnabled;
    (void)nFrameSkipLimitInterval;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    if(!pConfig)
    {
      WFDMMLOGE("WFDMMSourceVideoSource::bad params");
      return OMX_ErrorBadParameter;
    }// if pConfig
    m_pEventHandlerFn = pEventHandlerFn;
    m_pFrameDeliverFn = pFrameDeliverFn;
    m_nModuleId       = nModuleId;
    m_appData         = appData;
    if(m_pHdcpHandle &&
      (m_pHdcpHandle->m_eHdcpSessionStatus == HDCP_STATUS_SUCCESS))
    {
        m_bHdcpSessionValid = OMX_TRUE;
    }
    if(m_eVideoSrcState==  WFDMM_VideoSource_STATE_INIT)
    {
        result = m_pVencoder->configure(pConfig,
                                        &VideoSourceEventHandlerCb,
                                        &VideoSourceFrameDelivery,
                                        m_bHdcpSessionValid,
                                        WFDMM_VENC_MODULE_ID,
                                        this);

        m_nNumBuffers = m_pVencoder->GetNumOutBuf();
        if(result != OMX_ErrorNone)
        {
            return result;
        }
        if(m_bHdcpSessionValid)
        {
            /*-----------------------------------------------------------------
                     HDCP handle is valid which implies HDCP connection has
                     gone through. Now that encoder module has been configured,
                     go ahead and allocate the resources required for HDCP.
                    -----------------------------------------------------------
                    */
            result = AllocateHDCPResources();
            if(result != OMX_ErrorNone)
            {
                WFDMMLOGE("Failed to allocate HDCP resources");
                return result;
            }
        }

        if(!m_bHdcpSessionValid && m_bFillerNaluEnabled)
        {
            /*---------------------------------------------------------------------
             For non HDCP session
            -----------------------------------------------------------------------
            */
            m_pFillerDataOutPtr = (unsigned char*)sFillerNALU;
        }

        WFDMMLOGH("WFDMMSourceVideoSource::Allocated all resources");
        m_eVideoSrcState = WFDMM_VideoSource_STATE_IDLE;
    }
    return result;
}

/*!*************************************************************************
 * @brief     Timer handler for reading statistics flag from command line
 *
 * @param[in] ptr Reference to the current instance
 *
 * @return    NONE
 *
 * @note
 **************************************************************************/
#ifdef ENABLE_WFD_STATS
// TODO: Needs to be moved to appropriate location
void VideoSource::readStatTimerHandler(void* ptr)
{
    if(!ptr)
    {
        return;
    }
    VideoSource* pMe= (VideoSource*)ptr;
    char szTemp[PROPERTY_VALUE_MAX];
    if(property_get("persist.debug.enable_wfd_stats",szTemp,NULL)<0)
    {
        WFDMMLOGE("Failed to read persist.debug.enable_wfd_stats");
        return;
    }
    if(strcmp(szTemp,"false")==0)
    {
        memset(&(pMe->wfdStats),0, sizeof(wfdStats));
        pMe->wfdStats.bEnableWfdStat = OMX_FALSE;
    }
    else
    {
        pMe->wfdStats.bEnableWfdStat = OMX_TRUE;
    }
}
#endif


/*=============================================================================

         FUNCTION:            AllocateHDCPResources

         DESCRIPTION:
*//**       @brief           responisble for HDCP specific resource allocation
                                   for VideoSource
*//**
@par     DEPENDENCIES:
                            Should be called once Encoder module is configured.
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE VideoSource::AllocateHDCPResources()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    WFDMMLOGE("AllocateHDCPResources");
    if(!m_bHdcpSessionValid)
    {
        WFDMMLOGE("HDCP session is not established");
        return OMX_ErrorInvalidState;
    }

    if(!m_pVencoder)
    {
        WFDMMLOGE("Invalid encoder component");
        return OMX_ErrorBadParameter;
    }

    /*-----------------------------------------------------------------
     Both output buffers and ION buffers for HDCP should be same in
     number
    -----------------------------------------------------------
    */

    /*-----------------------------------------------------------------
     Create Q for holding OMX BufferHeaders in case of HDCP session which
     will be delivered to MUX
    -----------------------------------------------------------
    */

    OMX_S32 buffer_size = m_pVencoder->GetOutBufSize();
    m_pHDCPOutputBufQ = MM_New_Args(SignalQueue,(10, sizeof(OMX_BUFFERHEADERTYPE*)));
    if(!m_pHDCPOutputBufQ)
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,
            "Could not create HDCPOutputBufQ");
    }

    /*-----------------------------------------------------------------
     Create the array of OMX BufferHeaders in case of HDCP session which
     will be delivered to MUX.
    -------------------------------------------------------------------
    */

    m_pVideoSourceOutputBuffers = MM_New_Array(OMX_BUFFERHEADERTYPE*,m_nNumBuffers);
    if(!m_pVideoSourceOutputBuffers)
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,
            "Could not allocate VideoSourceOutputBufferHeaders");
    }

    /*-----------------------------------------------------------------
     Allocate ION buffers for HDCP output buffers
    -------------------------------------------------------------------
    */

    m_pHdcpOutBuffers = (struct buffer*)calloc(m_nNumBuffers,sizeof(buffer));
    if(!m_pHdcpOutBuffers)
    {
        result =  OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,"Failed to allocate HDCP output buffers");
    }

    ionfd = open("/dev/ion",  O_RDONLY);
    if (ionfd < 0)
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,"Failed to open ion device");
    }

    WFDMMLOGE1("Opened ion device = %d\n", ionfd);

    for (int i = 0; i < m_nNumBuffers; i++)
    {

        /*-----------------------------------------------------------------
         Allocate OMX BufferHeaders
        -----------------------------------------------------------
        */
        m_pVideoSourceOutputBuffers[i] = (OMX_BUFFERHEADERTYPE* )\
                            MM_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
        if(!m_pVideoSourceOutputBuffers[i])
        {
            result = OMX_ErrorInsufficientResources;
            WFD_OMX_ERROR_CHECK(result,"Could not allocate VideoSourceOutputBuffers");
        }

        /*-----------------------------------------------------------------
         Nullify appPrivate because this will be populated with ION buffer
            -----------------------------------------------------------
        */
        m_pVideoSourceOutputBuffers[i]->pAppPrivate = NULL;

        /*-----------------------------------------------------------------
         Nullify outputPortPrivate because this will be populated with OMX
         BufferHeader received from encoder
            -----------------------------------------------------------
        */
        m_pVideoSourceOutputBuffers[i]->pOutputPortPrivate = NULL;

        /*-----------------------------------------------------------------
         Allocate ION memory for the HDCP output buffers
            -----------------------------------------------------------
        */

        memfd = allocate_ion_mem((unsigned int)buffer_size, &(m_pHdcpOutBuffers[i].handle), ionfd,
                    ION_QSECOM_HEAP_ID,(OMX_BOOL) OMX_FALSE);
        if(memfd < 0)
        {
            WFDMMLOGE("Failed to allocate ion memory for HDCP buffers");
            return OMX_ErrorInsufficientResources;
        }

        WFDMMLOGE1("memfd = %d ", memfd);

        m_pHdcpOutBuffers[i].start = (unsigned char *)
        mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
        m_pHdcpOutBuffers[i].length = buffer_size;
        m_pHdcpOutBuffers[i].fd = memfd;
        m_pHdcpOutBuffers[i].offset = 0;
        m_pHdcpOutBuffers[i].index = i;
        WFDMMLOGH3("allocated buffer(%p) of size = %ld, fd = %d",
                m_pHdcpOutBuffers[i].start, buffer_size, memfd);
        if (m_pHdcpOutBuffers[i].start == MAP_FAILED)
        {
            WFDMMLOGE("Could not allocate ION buffers");
            return OMX_ErrorInsufficientResources;
        }

        m_pVideoSourceOutputBuffers[i]->pBuffer = reinterpret_cast<OMX_U8*>
                                                (m_pHdcpOutBuffers[i].start);
        m_pVideoSourceOutputBuffers[i]->pAppPrivate = reinterpret_cast<OMX_U8*>
                                                (&m_pHdcpOutBuffers[i]);
        /*-----------------------------------------------------------------
         Push the allocated buffer to the Q now that all its
         fields are properly polulated
        -----------------------------------------------------------------
        */

        m_pHDCPOutputBufQ->Push(&(m_pVideoSourceOutputBuffers[i]),
                                sizeof(m_pVideoSourceOutputBuffers[i]));
    }//for loop allocate buffer on ion memory
    if(m_bFillerNaluEnabled)
    {
      WFDMMLOGE("Allocate filler Nalu buffers");

      m_nFillerInFd = allocate_ion_mem(FILLER_ION_BUF_SIZE, &m_hFillerInHandle,
                                       ionfd, ION_QSECOM_HEAP_ID, OMX_FALSE);

      if(m_nFillerInFd <= 0)
      {
          WFDMMLOGE("Failed to allocate In FillerNalu ION buffer");
      }
      else
      {
          m_pFillerDataInPtr = (unsigned char*)
                         mmap(NULL, FILLER_ION_BUF_SIZE, PROT_READ | PROT_WRITE,
                              MAP_SHARED, m_nFillerInFd, 0);
          if(m_pFillerDataInPtr == NULL)
          {
            WFDMMLOGE("Failed to allocate In FillerNalu buffer mmap");
          }
          else
          {
            //Initialize the input buffer with fixed Filler NALU
            memcpy(m_pFillerDataInPtr, sFillerNALU, sizeof(sFillerNALU));
          }
      }

      m_nFillerOutFd = allocate_ion_mem(FILLER_ION_BUF_SIZE, &m_hFillerOutHandle,
                                        ionfd, ION_QSECOM_HEAP_ID, OMX_FALSE);

      if(m_nFillerOutFd <= 0)
      {
          WFDMMLOGE("Failed to allocate Out FillerNalu ION buffer");
      }
      else
      {
          m_pFillerDataOutPtr = (unsigned char*)
                       mmap(NULL, FILLER_ION_BUF_SIZE, PROT_READ | PROT_WRITE,
                                                MAP_SHARED, m_nFillerOutFd, 0);
          if(m_pFillerDataOutPtr == NULL)
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "Failed to allocate In FillerNalu buffer mmap");
          }
      }
    }
EXIT:
    return result;
}

/*=============================================================================

         FUNCTION:            AllocateHDCPResources

         DESCRIPTION:
*//**       @brief           responisble for HDCP specific resource allocation
                                   for VideoSource
*//**
@par     DEPENDENCIES:
                            Should be called once Encoder module is configured.
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE VideoSource::DeallocateHDCPResources()
{
    if(!m_bHdcpSessionValid)
    {
        return OMX_ErrorNone;
    }
    WFDMMLOGE("Deallocating HDCP resources");

    struct ion_handle_data handle_data;
    int ret = -1;

    if(m_pVideoSourceOutputBuffers)
    {
        for (unsigned int i = 0; i < (unsigned int)m_nNumBuffers; i++)
        {
            if(m_pVideoSourceOutputBuffers[i])
            {
                if((m_pHdcpHandle != NULL) &&(m_pHdcpOutBuffers != NULL))
                {
                    /*---------------------------------------------------------
                            Unmap the memory
                    -------------------------------------------------------
                            */
                    if(m_pHdcpOutBuffers[i].start != NULL)
                    {
                        if (munmap(m_pHdcpOutBuffers[i].start, m_pHdcpOutBuffers[i].length) == -1)
                        {
                            WFDMMLOGE2("error in munmap at idx %d :%s",
                                i,strerror(errno));
                        }
                        m_pHdcpOutBuffers[i].start = NULL;
                        m_pHdcpOutBuffers[i].length = 0;
                    }

                    /*---------------------------------------------------------
                            Free the ION handle
                                ----------------------------------------------
                            */

                    if(m_pHdcpOutBuffers[i].handle != 0)
                    {
                        handle_data.handle = m_pHdcpOutBuffers[i].handle;
                        ret = ioctl(ionfd, ION_IOC_FREE, &handle_data);
                        if(ret)
                        {
                            WFDMMLOGE2("Error in freeing handle at idx %d : %d",
                                i,ret);
                        }
                        m_pHdcpOutBuffers[i].handle = 0;
                    }

                    /*---------------------------------------------------------
                            Close the fd
                        -------------------------------------------------------
                            */

                    if(m_pHdcpOutBuffers[i].fd > 0)
                    {
                        close(m_pHdcpOutBuffers[i].fd);
                        WFDMMLOGH2("closing hdcp ion fd = %d ret type = %d",
                                    m_pHdcpOutBuffers[i].fd, ret);
                        m_pHdcpOutBuffers[i].fd = -1;
                    }
                }

                if(m_pVideoSourceOutputBuffers[i] != NULL)
                {
                    MM_Free(m_pVideoSourceOutputBuffers[i]);
                    m_pVideoSourceOutputBuffers[i] = NULL;
                }

            }
        }

        if(m_nFillerInFd > 0)
        {
            if(m_pFillerDataInPtr)
            {
                munmap(m_pFillerDataInPtr, FILLER_ION_BUF_SIZE);
                m_pFillerDataInPtr = NULL;
            }

            if(m_hFillerInHandle != 0)
            {
                handle_data.handle = m_hFillerInHandle;
                ret = ioctl(ionfd, ION_IOC_FREE, &handle_data);
                m_hFillerInHandle = 0;
            }

            close(m_nFillerInFd);
            WFDMMLOGH2("closing hdcp filler ion fd = %d ret type = %d \n",
                        m_nFillerInFd, ret);
            m_nFillerInFd = -1;
        }

        if(m_nFillerOutFd > 0)
        {
            if(m_pFillerDataOutPtr)
            {
                munmap(m_pFillerDataOutPtr, FILLER_ION_BUF_SIZE);
                m_pFillerDataOutPtr = NULL;
            }

            if(m_hFillerOutHandle != 0)
            {
                handle_data.handle = m_hFillerOutHandle;
                ret = ioctl(ionfd, ION_IOC_FREE, &handle_data);
                m_hFillerOutHandle = 0;
            }

            close(m_nFillerOutFd);
            WFDMMLOGH2("closing hdcp filler ion fd = %d ret type = %d \n",
                 m_nFillerOutFd, ret);
            m_nFillerOutFd = -1;
        }

        if(m_pHdcpOutBuffers)
        {
            MM_Free(m_pHdcpOutBuffers);
            m_pHdcpOutBuffers = NULL;
        }

        MM_Delete_Array(m_pVideoSourceOutputBuffers);
        m_pVideoSourceOutputBuffers = NULL;
    }

    if(ionfd > 0)
    {
        close(ionfd);
        ionfd = -1;
    }

    return OMX_ErrorNone;
}

/*!*************************************************************************
 * @brief     Start video source
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE VideoSource::Start()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    // make sure we've been configured
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMSourceVideoSource::SourceThread Start");
    if(!m_pVencoder)
    {
        result = OMX_ErrorUndefined;
        WFD_OMX_ERROR_CHECK(result,"Failed to start!");
    }
    if(m_eVideoSrcState == WFDMM_VideoSource_STATE_PLAY)
    {
        WFDMMLOGH("Already in Playing, Ignore");
    }
    else if(m_eVideoSrcState == WFDMM_VideoSource_STATE_PAUSE)
    {
        result = m_pVencoder->Resume();
        WFD_OMX_ERROR_CHECK(result,"Failed to start!");
    }
    else if(m_eVideoSrcState == WFDMM_VideoSource_STATE_IDLE)
    {
        result = m_pVencoder->Start();
        WFD_OMX_ERROR_CHECK(result,"Failed to start!");
        if(m_pEventHandlerFn)
        {
            m_pEventHandlerFn( m_appData, m_nModuleId,
               WFDMMSRC_VIDEO_SESSION_START, OMX_ErrorNone, 0);
        }
    }
    else
    {
        result = OMX_ErrorInvalidState;
        WFD_OMX_ERROR_CHECK(result,"Failed to start!");
    }
    m_eVideoSrcState = WFDMM_VideoSource_STATE_PLAY;

EXIT:
    return result;
}

/*!*************************************************************************
 * @brief     Pause video source
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE VideoSource::Pause()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if(m_eVideoSrcState == WFDMM_VideoSource_STATE_PLAY)
    {
        m_eVideoSrcState = WFDMM_VideoSource_STATE_PAUSING;
        if(m_pVencoder)
        {
            result = m_pVencoder->Pause();
            WFD_OMX_ERROR_CHECK(result,"Failed to pause!");
        }
        m_eVideoSrcState = WFDMM_VideoSource_STATE_PAUSE;
    }
    else
    {
        result = OMX_ErrorInvalidState;
        WFD_OMX_ERROR_CHECK(result,"Failed to pause!");
    }
EXIT:
    return result;
}

/*!*************************************************************************
 * @brief     Finish video source
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE VideoSource::Finish()
{
    WFDMMLOGH("VideoSource Finish");
    m_eVideoSrcState = WFDMM_VideoSource_STATE_STOPPING;
    if(m_pVencoder)
    {
        WFDMMLOGH("Calling Encoder Stop");
        m_pVencoder->Stop();
    }
    m_eVideoSrcState = WFDMM_VideoSource_STATE_IDLE;
    return OMX_ErrorNone;
}

/*!*************************************************************************
 * @brief     Get buffer header
 *
 * @param[in] NONE
 *
 * @return    OMX_BUFFERHEADERTYPE**
 *
 * @note      NONE
 **************************************************************************/
OMX_BUFFERHEADERTYPE** VideoSource::GetBuffers()
{
    OMX_BUFFERHEADERTYPE** ppBuffers;
    if(m_pVencoder)
    {
        ppBuffers = m_pVencoder->GetOutputBuffHdrs();
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMSourceVideoSource::GetBuffers success");
    }
    else
    {
        ppBuffers = NULL;
    }
    return ppBuffers;
}

/*!*************************************************************************
 * @brief     Get number of buffer
 *
 * @param[in] NONE
 * @param[out]pnBuffers
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE VideoSource::GetOutNumBuffers(
    OMX_U32 nPortIndex, OMX_U32* pnBuffers)
{
    if(nPortIndex != 0)
    {
        return OMX_ErrorBadPortIndex;
    }
    if(!pnBuffers)
    {
        return OMX_ErrorBadParameter;
    }
    if(m_pVencoder)
    {
        *pnBuffers = m_pVencoder->GetNumOutBuf();
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                 "WFDMMSourceVideoSource::GetOutNumBuffers = %ld",
                 *pnBuffers);
    }
    return OMX_ErrorNone;
}

/*!*************************************************************************
 * @brief     Changes the frame rate
 *            The frame rate will take effect immediately.
 * @param[in] nFramerate New frame rate
 *
 * @return    OMX_ERRORTYPE
 *
 * @note
 **************************************************************************/
OMX_ERRORTYPE VideoSource::ChangeFrameRate(OMX_S32 nFramerate)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                 "WFDMMSourceVideoSource::ChangeFrameRate %ld", nFramerate);
    return result;
}

/*!*************************************************************************
 * @brief     Changes the bit rate
 *            The bit rate will take effect immediately.
 *
 * @param[in] nBitrate The new bit rate.
 *
 * @return    OMX_ERRORTYPE
 * @note      None
 **************************************************************************/
OMX_ERRORTYPE VideoSource::ChangeBitrate(OMX_S32 nBitrate)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceVideoSource::ChangeBitrate %ld", nBitrate);
    if(nBitrate > 0)
    {
      {
        if(m_pVencoder)
        {
          m_pVencoder->ChangeBitrate(nBitrate);
        }
      }
    }// if nBitRate
    return result;
}

/*!*************************************************************************
 * @brief     Request Intra VOP
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE VideoSource::RequestIntraVOP()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceVideoSource::RequestIntraVOP");
    if(m_pVencoder)
    {
        result = m_pVencoder->RequestIntraVOP();
    }
    return result;
}

/*=============================================================================

         FUNCTION:            VideoSourceFrameDelivery

         DESCRIPTION:
*//**       @brief           Function responsible for sending frames to MUX

*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      pBufferHdr   buffer fhaving encoded data

     *         @param[in]      pThis          pointer to get current instance

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
void VideoSource::VideoSourceFrameDelivery(OMX_BUFFERHEADERTYPE* pEncBufferHdr, void* pThis)
{
    if(!pEncBufferHdr || !pThis)
    {
        WFDMMLOGE(" Invalid parameters to VideoSourceFrameDelivery");
        return;
    }
    VideoSource* pMe = (VideoSource*)pThis;

    if(pEncBufferHdr->nFlags & OMX_BUFFERFLAG_SYNCFRAME)
    {
        pMe->m_bDropVideoFrame = OMX_FALSE;
    }

    if(pMe->m_eVideoSrcState != WFDMM_VideoSource_STATE_PLAY
            ||pMe->m_bDropVideoFrame)
    {
        WFDMMLOGH("Not delivering frame to MUX");
        pMe->m_pVencoder->SetFreeBuffer(pEncBufferHdr);
        return;
    }

    if(pMe->m_bHdcpSessionValid)
    {
        //Adopt encryption path
        pMe->EncryptData(pEncBufferHdr);
    }
    else
    {
        if(pMe->m_bFillerNaluEnabled)
        {
            pMe->FillFillerBytesExtraData(pEncBufferHdr,NULL,1);
        }
        pMe->m_pFrameDeliverFn(pEncBufferHdr,pMe->m_appData);
    }
}

/*!*************************************************************************
 * @brief     Set Free buffer
 *
 * @param[in] pBufferHdr OMX_BUFFERHEADERTYPE passed in
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE VideoSource::SetFreeBuffer(OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE* pEncOutputBuff = NULL;
    if (pBufferHdr != NULL && pBufferHdr->pBuffer != NULL)
    {
        if(m_bHdcpSessionValid)
        {
            /*-----------------------------------------------------------------
                     Extract the Buffer Header to be sent back to encoder
                     from the received one
            -----------------------------------------------------------
                    */
            pEncOutputBuff = reinterpret_cast<OMX_BUFFERHEADERTYPE*>
                                    (pBufferHdr->pOutputPortPrivate);

            /*-----------------------------------------------------------------
                         Push back the buffer Header to the HDCP output Q
                    -----------------------------------------------------------
                    */
            m_pHDCPOutputBufQ->Push(&pBufferHdr,sizeof(pBufferHdr));
        }
        else
        {
            /*-----------------------------------------------------------------
                     Push back same buffer header to encoder as received
                     back from Mux
                    -----------------------------------------------------------
                    */
            pEncOutputBuff = pBufferHdr;
        }

        if(m_bEnableProfiling)
        {
            if(m_eVideoSrcState == WFDMM_VideoSource_STATE_PLAY)
            {
                if(m_pWFDMMSrcStats)
                {
                    m_pWFDMMSrcStats->recordMuxStat(pEncOutputBuff);
                }
            }
        }

        m_pVencoder->SetFreeBuffer(pEncOutputBuff);
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceVideoSource::bad params");
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/*=============================================================================

         FUNCTION:            EncryptData

         DESCRIPTION:
*//**       @brief           Function for encrypting encoded data
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      pBufferHdr   buffer fhaving encoded data

     *         @param[in]      pThis          pointer to get current instance

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

void VideoSource::EncryptData(OMX_BUFFERHEADERTYPE* pEncBufferHdr)
{
    WFDMMLOGE("Received data to encrypt");
    if(!pEncBufferHdr)
    {
        WFDMMLOGE("Invalid argument to EncryptData");
        return;
    }
    OMX_BUFFERHEADERTYPE* pBuffHdr;
    m_pHDCPOutputBufQ->Pop(&pBuffHdr, sizeof(pBuffHdr),100);
    if(!pBuffHdr)
    {
        WFDMMLOGE("Failed to POP from HDCP BufQ");
        m_pVencoder->SetFreeBuffer(pEncBufferHdr);
        return;
    }

    /*-----------------------------------------------------------------
             Extract relevant details from the encoder output
            -----------------------------------------------------------
    */
    pBuffHdr->nAllocLen        = pEncBufferHdr->nAllocLen;
    pBuffHdr->nFilledLen       = pEncBufferHdr->nFilledLen;
    pBuffHdr->nSize            = pEncBufferHdr->nSize;
    pBuffHdr->nFlags           = pEncBufferHdr->nFlags;
    pBuffHdr->nInputPortIndex  = pEncBufferHdr->nInputPortIndex;
    pBuffHdr->nOffset          = pEncBufferHdr->nOffset;
    pBuffHdr->nOutputPortIndex = pEncBufferHdr->nOutputPortIndex;
    pBuffHdr->nTimeStamp       = pEncBufferHdr->nTimeStamp;
    pBuffHdr->nVersion         = pEncBufferHdr->nVersion;

    buff_hdr_extra_info* tempExtra = static_cast<buff_hdr_extra_info*>
        (pEncBufferHdr->pPlatformPrivate);

    if(pEncBufferHdr->pOutputPortPrivate)
    {
        uint8 ucPESPvtData[PES_PVT_DATA_LEN] = {0};
        pmem* ion_buffer_in =  (static_cast<pmem*>
                                  (pEncBufferHdr->pOutputPortPrivate));
        buffer* ion_buffer_out = (reinterpret_cast<buffer*>
                                    (pBuffHdr->pAppPrivate));

        //Reset in case encryption fails, to avoid spurious stats
        if(tempExtra)
        {
            tempExtra->nEncryptTime = 0;
        }

        unsigned ulStatus =
        m_pHdcpHandle->WFD_HdcpDataEncrypt(STREAM_VIDEO ,
                          (unsigned char*)ucPESPvtData,
                          (unsigned char *) (uint64)(ion_buffer_in->fd),
                          (unsigned char *) (uint64)(ion_buffer_out->fd),
                           pEncBufferHdr->nFilledLen);
        if( ulStatus != 0)
        {
            WFDMMLOGE1("Error in HDCP Encryption! %x", ulStatus);
            /*------------------------------------------------------------------
             Release back buffer to encoder, and push back buffer
             Header to Q and report a runtime error
             -------------------------------------------------------------------
            */
            m_pHDCPOutputBufQ->Push(&pBuffHdr, sizeof(pBuffHdr));
            m_pVencoder->SetFreeBuffer(pEncBufferHdr);

            if(false == m_pHdcpHandle->proceedWithHDCP())
            {
                WFDMMLOGE("Cipher enablement wait timed out");
                if(m_pEventHandlerFn)
                {
                  m_pEventHandlerFn( m_appData, m_nModuleId,WFDMMSRC_ERROR,
                    OMX_ErrorUndefined, 0);
                }
            }
            else
            {
            /*------------------------------------------------------------------
             In case a frame is dropped request an IDR from encoder to ensure
             that the sink always does receive an IDR, else we might end up in a
             scenario where an IDR frame is dropped due to CIPHER not being
             enabled and then once it's enabled, we end up sending only P frames
             until the sink explicitly requests for an IDR (not guaranteed) or
             the IDR interval expires.
             -------------------------------------------------------------------
            */
                m_pVencoder->RequestIntraVOP();
                m_bDropVideoFrame = OMX_TRUE;
            }
            return;
        }

        if(m_bEnableProfiling)
        {
            if(m_pWFDMMSrcStats)
            {
                m_pWFDMMSrcStats->recordVideoEncryptStat(pEncBufferHdr);
            }
        }

        for ( int idx = 0; idx < PES_PVT_DATA_LEN; idx++)
        {
            WFDMMLOGE3("Encrypt PayloadLen[%lu] PES_PVTData[%d]:%x",
                        pEncBufferHdr->nFilledLen,
                        idx,
                        ucPESPvtData[idx]);
        }

        /*-----------------------------------------------------------------
             Fill PESPvtData at end of the encrypted buffer, as an extra data
            -----------------------------------------------------------
            */

        FillHdcpCpExtraData( pBuffHdr, ucPESPvtData, 1);

        if(m_bFillerNaluEnabled)
        {
          memset((void*)ucPESPvtData, 0, sizeof(ucPESPvtData));
          if(m_nFillerInFd > 0 && m_nFillerOutFd > 0)
          {
             unsigned long ulStatus = m_pHdcpHandle->WFD_HdcpDataEncrypt(
                                    STREAM_VIDEO ,
                                   (unsigned char*)ucPESPvtData,
                                   (unsigned char *)(uint64) (m_nFillerInFd),
                                   (unsigned char *) (uint64)(m_nFillerOutFd),
                                    FILLER_NALU_SIZE);
             if( ulStatus != 0)
             {
                 WFDMMLOGE("Error in Filler NALU HDCP Encryption");
             }
             else
             {
                 FillFillerBytesExtraData(pBuffHdr,
                                          ucPESPvtData, 1);
                 WFDMMLOGE("Filler NALU HDCP Encryption");
             }
          }
        }

        pBuffHdr->pOutputPortPrivate = reinterpret_cast<OMX_U8*>
                                    (pEncBufferHdr);
        if(m_pFrameDeliverFn)
        {
            m_pFrameDeliverFn(pBuffHdr,m_appData);
        }
    }
    else
    {
        WFDMMLOGE("Can't extract fd from encoded buffer!");
    }
}


/***********************************************************************************!
 * @brief      Fill Extra data in buffer
 * @details    Fill HDCP PES pvt extra data at end of buffer.
 * @param[in]  pBufHdr         Payload buffer header
 * @param[in]  pucPESPvtHeader PES Pvt data
 * @param[in]  nPortIndex      Port index
 * @return     RETURN 'OMX_ErrorNone' if SUCCESS
 *             OMX_ErrorBadParameter code in FAILURE
 ***********************************************************************************/
OMX_ERRORTYPE  VideoSource::FillHdcpCpExtraData(
                          OMX_BUFFERHEADERTYPE *pBufHdr,
                          OMX_U8* pucPESPvtHeader,
                          OMX_U32 nPortIndex)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  uint64 ulAddr;
  OMX_OTHER_EXTRADATATYPE *pHdcpCpExtraData;
  if( (NULL != pBufHdr ) && (NULL != pucPESPvtHeader))
  {
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceVideoSource::pBufHdr->pBuffer[%p] FilledLen[%ld]",
                pBufHdr->pBuffer,
                pBufHdr->nFilledLen);
    /* Skip encoded frame payload length filled by V4L2 driver */
    ulAddr = (uint64) ( pBufHdr->pBuffer) +  pBufHdr->nFilledLen;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceVideoSource::ulAddr[%llu]", ulAddr);
    /* Aligned address to DWORD boundary */
    ulAddr = (ulAddr + 0x3) & (~0x3);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceVideoSource::Aligned ulAddr[%llu]", ulAddr);
    pHdcpCpExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
    /* Update pBufHdr flag, to indicate that it carry extra data */
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceVideoSource::pHdcpCpExtraData[%p]", pHdcpCpExtraData);

    FLAG_SET(pBufHdr,OMX_BUFFERFLAG_EXTRADATA);
    /* Extra Data size = ExtraDataType*/
    pHdcpCpExtraData->nSize = sizeof(OMX_OTHER_EXTRADATATYPE) + sizeof(OMX_U8)* PES_PVT_DATA_LEN -4;
    pHdcpCpExtraData->nVersion = pBufHdr->nVersion;
    pHdcpCpExtraData->nPortIndex = nPortIndex;
    pHdcpCpExtraData->nDataSize = PES_PVT_DATA_LEN;
    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDMMSourceVideoSource::size[%ld] PortIndex[%ld] nDataSize[%ld]",
      pHdcpCpExtraData->nSize,
      pHdcpCpExtraData->nPortIndex,pHdcpCpExtraData->nDataSize );
    pHdcpCpExtraData->eType = (OMX_EXTRADATATYPE)QOMX_ExtraDataHDCPEncryptionInfo;
    /* Fill PES_PVT_DATA into data*/
    memcpy(pHdcpCpExtraData->data,pucPESPvtHeader, PES_PVT_DATA_LEN );
    /* Append OMX_ExtraDataNone */
    ulAddr += pHdcpCpExtraData->nSize;
    pHdcpCpExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
    pHdcpCpExtraData->nSize = sizeof(OMX_OTHER_EXTRADATATYPE);
    pHdcpCpExtraData->nVersion = pBufHdr->nVersion;
    pHdcpCpExtraData->nPortIndex = nPortIndex;
    pHdcpCpExtraData->eType = OMX_ExtraDataNone;
    pHdcpCpExtraData->nDataSize = 0;
  }
  else
  {
    eError = OMX_ErrorBadParameter;
  }
  return eError;
}

/***********************************************************************************!
 * @brief      Fill FillerBytes Extra data in buffer
 * @details    Fill Fillerbytes NALU extra data at end of buffer.
 * @param[in]  pBufHdr         Payload buffer header
 * @param[in]  pucPESPvtHeader PES Pvt data
 * @param[in]  nPortIndex      Port index
 * @return     RETURN 'OMX_ErrorNone' if SUCCESS
 *             OMX_ErrorBadParameter code in FAILURE
 ***********************************************************************************/
bool  VideoSource::FillFillerBytesExtraData(
                          OMX_BUFFERHEADERTYPE *pBuffHdr,
                          OMX_U8* pucPESPvtHeader,
                          OMX_U32 nPortIndex)
{
  OMX_OTHER_EXTRADATATYPE *pExtra;

  if(NULL != pBuffHdr)
  {
    OMX_U8 *pTmp = pBuffHdr->pBuffer +
                         pBuffHdr->nOffset + pBuffHdr->nFilledLen + 3;

    pExtra = (OMX_OTHER_EXTRADATATYPE *) ((reinterpret_cast<long>(pTmp)) & ~3);

    if(pBuffHdr->nFlags & OMX_BUFFERFLAG_EXTRADATA)
    {
      //Extra Data already set. Find the end
      while(pExtra->eType != OMX_ExtraDataNone)
      {
        pExtra = (OMX_OTHER_EXTRADATATYPE *)
                      (((OMX_U8 *) pExtra) + pExtra->nSize);

        if(reinterpret_cast<long>(pExtra) + sizeof(OMX_OTHER_EXTRADATATYPE) >=
          reinterpret_cast<long>(pBuffHdr->pBuffer) + pBuffHdr->nAllocLen)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "Fiiler Bytes Reached out of bounds");
          return false;
        }
      }
    }

    OMX_U32 nBytesToFill = FILLER_NALU_SIZE + 1 /*Signalling Byte*/
                                            + 1 /*Length */;

    if(pucPESPvtHeader != NULL)
    {
      /*Filler Nalu us encrypted*/
      nBytesToFill += PES_PVT_DATA_LEN + 1 /*Length */;
    }

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                 "VideoSource Fiiler Bytes nBytesToFill = %lu",
                  nBytesToFill);

    /** Check out of bound access in the pBuffer */
    if(reinterpret_cast<long>(pExtra) + sizeof(OMX_OTHER_EXTRADATATYPE) +
                                           ((nBytesToFill +3) & (~3))  >
       reinterpret_cast<long>(pBuffHdr->pBuffer) + pBuffHdr->nAllocLen)
    {
      /*Can't fit in filler bytes*/
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "VideoSource Can't fit in fillerNALU");
      return false;
    }

    FLAG_SET(pBuffHdr,OMX_BUFFERFLAG_EXTRADATA);

    /* Extra Data size = ExtraDataType*/
    pExtra->nVersion = pBuffHdr->nVersion;
    pExtra->nPortIndex = nPortIndex;
    pExtra->nDataSize = nBytesToFill;
    nBytesToFill += 3;
    nBytesToFill &= (~3);
    pExtra->nSize = sizeof(OMX_OTHER_EXTRADATATYPE) + nBytesToFill -4;

    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDMMSourceVideoSource:: Filler size[%ld] PortIndex[%ld] nDataSize[%ld]",
      pExtra->nSize,
      pExtra->nPortIndex,pExtra->nDataSize );

    /* Using MAX to pass generic ExtraData Info using signalling byte to
       identify the type of data*/
    pExtra->eType = OMX_ExtraDataMax;

    /**------------------------------------------------------------------------
    Filler NALU format:= |SigByte|Size|PES Pvt Data|Size|Filler NALU |
    ---------------------------------------------------------------------------
    */

    /**Fill the extra data bytes */
    uint32 nOffset = 0;

    /*Signal Encrypted Payload or non-encrypted Payload*/
    pExtra->data[nOffset] = VIDEO_PVTDATA_TYPE_FILLERNALU;

    if(pucPESPvtHeader != NULL)
    {
      pExtra->data[nOffset] = VIDEO_PVTDATA_TYPE_FILLERNALU_ENCRYPTED;
    }
    nOffset++;

    /**If encypted first add the PES private data */
    if(pucPESPvtHeader != NULL)
    {
      pExtra->data[nOffset] = PES_PVT_DATA_LEN;
      nOffset++;
      /* Fill PES_PVT_DATA into data*/
      memcpy(pExtra->data + nOffset,pucPESPvtHeader, PES_PVT_DATA_LEN );
      nOffset += PES_PVT_DATA_LEN;
    }

    /** FIll the filler NALU bytes */
    pExtra->data[nOffset] = FILLER_NALU_SIZE;
    nOffset++;
    memcpy(pExtra->data + nOffset, m_pFillerDataOutPtr, FILLER_NALU_SIZE);

    /** Fill the extradataNone if there is space left. Mux will
     *  check against AllocLen to prevent access beyond limit */
    pExtra = (OMX_OTHER_EXTRADATATYPE *)
           (reinterpret_cast<OMX_U8*>(pExtra) + pExtra->nSize);
    if(reinterpret_cast<long>(pExtra) + sizeof(OMX_OTHER_EXTRADATATYPE) >=
        reinterpret_cast<long>(pBuffHdr->pBuffer) + pBuffHdr->nAllocLen)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                     "Fiiler Bytes: XtraNone Reached out of bounds");
      return true;
    }
    pExtra->nSize = sizeof(OMX_OTHER_EXTRADATATYPE);
    pExtra->nVersion = pBuffHdr->nVersion;
    pExtra->nPortIndex = nPortIndex;
    pExtra->eType = OMX_ExtraDataNone;
    pExtra->nDataSize = 0;
  }

  return false;
}

/*==============================================================================

         FUNCTION:         eventHandler

         DESCRIPTION:
*//**       @brief         Static function that handles events from encoder
                                modules


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        pThis - this pointer
                          nModuleId - Id of the module reporting event
                          nEvent - Type of event
                          nStatus - status associated with event
                          nData  - More information about event


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void VideoSource::VideoSourceEventHandlerCb(void *pThis, OMX_U32 nModuleId,
                      WFDMMSourceEventType nEvent,
                      OMX_ERRORTYPE nStatus, void* nData)
{
    (void) nData;
    (void) nStatus;
    if(!pThis)
    {
        WFDMMLOGE("Invalid Me, can't handle device callback");
        return;
    }
    VideoSource* pMe= static_cast<VideoSource*>(pThis);
    WFDMMLOGE1("Received callback from module %ld",nModuleId);
    switch(nEvent)
    {
        case WFDMMSRC_ERROR:
            pMe->m_pEventHandlerFn(pMe->m_appData, pMe->m_nModuleId,
                   WFDMMSRC_ERROR, OMX_ErrorNone, 0);
            break;
        default:
            WFDMMLOGE("Simply unreachable!");
            break;
    }
    return;
}
