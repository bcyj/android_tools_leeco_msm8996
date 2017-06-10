/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#include <fcntl.h>
#include <linux/msm_mdp.h>
#include <linux/videodev2.h>
#include <linux/msm_vidc_dec.h>
#include <media/msm_media_info.h>
#include "venc/inc/omx_video_common.h"
#include <HardwareAPI.h>
#include "vtest_ComDef.h"
#include "vtest_Debug.h"
#include "vtest_Thread.h"
#include "vtest_SignalQueue.h"
#include "vtest_Sleeper.h"
#include "vtest_Time.h"
#include "vtest_File.h"
#include "vtest_Decoder.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_DECODER"

using namespace android;

namespace vtest {

struct CmdType {
    OMX_EVENTTYPE   eEvent;
    OMX_COMMANDTYPE eCmd;
    OMX_U32         sCmdData;
    OMX_ERRORTYPE   eResult;
};

Decoder::Decoder(CodecConfigType *pConfig)
    : ISource(),
      m_bInputEOSReached(OMX_FALSE),
      m_bSecureSession(OMX_FALSE),
      m_bPortReconfig(OMX_FALSE),
      m_pSignalQueue(new SignalQueue(32, sizeof(CmdType))),
      m_pFreeBufferQueue(new SignalQueue(32, sizeof(BufferInfo*))),
      m_nFrames(0),
      m_nFramerate(0),
      m_nFrameWidth(0),
      m_nFrameHeight(0),
      m_nInputBufferCount(0),
      m_nInputBufferSize(0),
      m_nOutputBufferCount(0),
      m_nOutputBufferSize(0),
      m_nOriginalOutputBufferCount(0),
      m_hDecoder(NULL),
      m_eState(OMX_StateLoaded),
      m_eStatePending(OMX_StateLoaded),
      m_eCodec(OMX_VIDEO_CodingUnused),
      m_ePlaybackMode(DynamicPortReconfig) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_STRING role = (OMX_STRING)"";
    static OMX_CALLBACKTYPE callbacks =
        { EventCallback, EmptyDoneCallback, FillDoneCallback };

    snprintf(m_pName, PROPERTY_FILENAME_MAX, "Decoder");
    VTEST_MSG_LOW("%s created", Name());
    m_eCodec = pConfig->eCodec;

    // open decoder handle, set compression format on i/p port
    GetComponentRole(pConfig->bSecureSession, &pConfig->eFileType, &role);

    m_sPortFmt.format.video.eCompressionFormat = m_eCodec;

    result = OMX_GetHandle(&m_hDecoder, role, this, &callbacks);
    if (result != OMX_ErrorNone || m_hDecoder == NULL) {
        VTEST_MSG_ERROR("Error getting component handle, rv: %d", result);
        return;
    }

    QOMX_VIDEO_QUERY_DECODER_INSTANCES decoder_instances;
    result = OMX_GetConfig(m_hDecoder,
            (OMX_INDEXTYPE)OMX_QcomIndexQueryNumberOfVideoDecInstance,
            &decoder_instances);
    FAILED0(result, "Decoder: Failed OMX_QcomIndexQueryNumberOfVideoDecInstance");

    VTEST_MSG_HIGH("Decoder: Number of decoder instances %u",
                   (unsigned int)decoder_instances.nNumOfInstances);

    // Get the port information
    OMX_INIT_STRUCT(&m_sPortParam, OMX_PORT_PARAM_TYPE);
    result = OMX_GetParameter(
            m_hDecoder, OMX_IndexParamVideoInit, (OMX_PTR)&m_sPortParam);
    FAILED0(result, "Decoder: Failed OMX_IndexParamVideoInit");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Decoder::~Decoder() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    //Move this a to an intermediate function between stop and destructor
    result = SetState(OMX_StateLoaded, OMX_TRUE);
    FAILED(result, "Could not move to OMX_StateLoaded!");

    // delete this first to avoid a event-callback crash
    if (m_hDecoder != NULL) {
        OMX_FreeHandle(m_hDecoder);
        m_hDecoder = NULL;
    }
    if (m_pSignalQueue != NULL) {
        delete m_pSignalQueue;
        m_pSignalQueue = NULL;
    }
    if (m_pFreeBufferQueue != NULL) {
        delete m_pFreeBufferQueue;
        m_pFreeBufferQueue = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PortBufferCapability Decoder::GetBufferRequirements(OMX_U32 ePortIndex) {

    PortBufferCapability sBufCap;

    Mutex::Autolock autoLock(m_pLock);
    memset(&sBufCap, 0, sizeof(PortBufferCapability));

    if (m_nInputBufferCount <= 0 || m_nOutputBufferSize <= 0) {
        VTEST_MSG_ERROR("Error: must call configure to get valid buf reqs");
        return sBufCap;
    }

    if (ePortIndex == PORT_INDEX_IN) {
        sBufCap.bAllocateBuffer = OMX_TRUE;
        sBufCap.bUseBuffer = OMX_FALSE;
        sBufCap.pSource = this;
        sBufCap.ePortIndex = ePortIndex;
        sBufCap.nWidth = m_nFrameWidth;
        sBufCap.nHeight = m_nFrameHeight;
        sBufCap.nMinBufferSize = m_nInputBufferSize;
        sBufCap.nMinBufferCount = m_nInputBufferCount;
        sBufCap.nExtraBufferCount = 0;
        sBufCap.nBufferUsage = 0;
    } else if (ePortIndex == PORT_INDEX_OUT) {
        sBufCap.bAllocateBuffer = OMX_TRUE;
        sBufCap.bUseBuffer = OMX_TRUE;
        sBufCap.pSource = this;
        sBufCap.ePortIndex = ePortIndex;
        sBufCap.nWidth = m_nFrameWidth;
        sBufCap.nHeight = m_nFrameHeight;
        sBufCap.nMinBufferSize = m_nOutputBufferSize;
        /* Should always report OriginalBufferCount so that
         * before starting the source, whoever queries these
         * requirements gets the same value. m_nOutputBufferCount
         * gets updated on AllocateBuffer */
        sBufCap.nMinBufferCount = m_nOriginalOutputBufferCount;
        sBufCap.nExtraBufferCount = 0;
        sBufCap.nBufferUsage = m_nOutputBufferUsage;
    } else {
        VTEST_MSG_ERROR("Error: invalid port selection");
    }
    return sBufCap;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::Start() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_eState != OMX_StateExecuting) {
        result = SetState(OMX_StateExecuting, OMX_TRUE);
        FAILED1(result, "Could not move to executing state!");
    }
    return ISource::Start();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::Stop() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    {
        Mutex::Autolock autoLock(m_pLock);
        m_bThreadStop = OMX_TRUE;
        if ((m_eState == OMX_StateIdle)
                || (m_eStatePending == OMX_StateIdle)) {
            return result;
        }
    }

    result = SetState(OMX_StateIdle, OMX_FALSE);
    FAILED(result, "Could not move to OMX_StateIdle!");

    /* We should not acquire a lock across sendcommand because of a possible
     * deadlock scenario */
    Flush(OMX_ALL);
    OMX_SendCommand(m_hDecoder, OMX_CommandPortDisable, PORT_INDEX_OUT, 0);
    OMX_SendCommand(m_hDecoder, OMX_CommandPortDisable, PORT_INDEX_IN, 0);
    return ISource::Stop();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::Configure(CodecConfigType *pConfig,
        BufferManager *pBufManager, ISource *pSource, ISource *pSink) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (!m_hDecoder) {
        VTEST_MSG_ERROR("handle is null");
        return OMX_ErrorBadParameter;
    }

    result = ISource::Configure(pConfig, pBufManager, pSource, pSink);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("Decoder configure failed");
        return result;
    }

    Mutex::Autolock autoLock(m_pLock);

    m_bSecureSession = pConfig->bSecureSession;
    m_nInputBufferCount = pConfig->nInBufferCount;
    m_nOutputBufferCount = pConfig->nOutBufferCount;
    m_nFrames = pConfig->nFrames;
    m_nFrameWidth = pConfig->nFrameWidth;
    m_nFrameHeight = pConfig->nFrameHeight;
    m_ePlaybackMode = pConfig->ePlaybackMode;

    OMX_QCOM_PARAM_PORTDEFINITIONTYPE inputPortFmt;
    OMX_INIT_STRUCT(&inputPortFmt, OMX_QCOM_PARAM_PORTDEFINITIONTYPE);

    inputPortFmt.nPortIndex = OMX_DirInput;
    FileType eFileType = pConfig->eFileType;
    OMX_U32 nNalSize = 0;

    switch (eFileType) {
        case FILE_TYPE_DAT_PER_AU:
        case FILE_TYPE_PICTURE_START_CODE:
        case FILE_TYPE_MPEG2_START_CODE:
        case FILE_TYPE_RCV:
        case FILE_TYPE_VC1:
        case FILE_TYPE_DIVX_311:
        case FILE_TYPE_VP8:
            inputPortFmt.nFramePackingFormat = OMX_QCOM_FramePacking_OnlyOneCompleteFrame;
            break;
        case FILE_TYPE_ARBITRARY_BYTES:
        case FILE_TYPE_264_NAL_SIZE_LENGTH:
        case FILE_TYPE_DIVX_4_5_6:
            inputPortFmt.nFramePackingFormat = OMX_QCOM_FramePacking_Arbitrary;
            break;
        default:
            inputPortFmt.nFramePackingFormat = OMX_QCOM_FramePacking_Unspecified;
    }
    result = OMX_SetParameter(m_hDecoder,
            (OMX_INDEXTYPE)OMX_QcomIndexPortDefn, (OMX_PTR)&inputPortFmt);
    FAILED1(result, "error OMX_QcomIndexPortDefn");

    //Enable extradata
    QOMX_ENABLETYPE extra_data;
    extra_data.bEnable = OMX_TRUE;

    char frameinfo_value[PROPERTY_VALUE_MAX] = {0};
    char interlace_value[PROPERTY_VALUE_MAX] = {0};
    char h264info_value[PROPERTY_VALUE_MAX] = {0};
    char video_qp_value[PROPERTY_VALUE_MAX] = {0};
    char videoinput_bitsinfo_value[PROPERTY_VALUE_MAX] = {0};
    char framepack_value[PROPERTY_VALUE_MAX] = {0};
    char extn_userdata_value[PROPERTY_VALUE_MAX] = {0};
    char property_value[PROPERTY_VALUE_MAX] = {0};

    OMX_U32 frameinfo = 0, interlace = 0, h264info = 0, video_qp = 0, videoinput_bitsinfo = 0,
            framepack = 0, extn_userdata = 0;

    property_get("vidc.vdec.debug.frameinfo", frameinfo_value, "0");
    frameinfo = atoi(frameinfo_value);
    if (frameinfo) {
        OMX_SetParameter(m_hDecoder,
            (OMX_INDEXTYPE)OMX_QcomIndexParamFrameInfoExtraData,
            (OMX_PTR)&extra_data);
    }

    property_get("vidc.vdec.debug.interlace", interlace_value, "0");
    interlace = atoi(interlace_value);
    if (interlace) {
        OMX_SetParameter(m_hDecoder,
            (OMX_INDEXTYPE)OMX_QcomIndexParamInterlaceExtraData,
            (OMX_PTR)&extra_data);
    }

    property_get("vidc.vdec.debug.h264info", h264info_value, "0");
    h264info = atoi(h264info_value);
    if (h264info) {
        OMX_SetParameter(m_hDecoder,
            (OMX_INDEXTYPE)OMX_QcomIndexParamH264TimeInfo,
            (OMX_PTR)&extra_data);
    }

    property_get("vidc.vdec.debug.video_qp_value", video_qp_value, "0");
    video_qp = atoi(video_qp_value);
    if (video_qp) {
        OMX_SetParameter(m_hDecoder,
            (OMX_INDEXTYPE)OMX_QcomIndexParamVideoQPExtraData,
            (OMX_PTR)&extra_data);
    }

    property_get("vidc.vdec.debug.input_bitsinfo", videoinput_bitsinfo_value, "0");
    videoinput_bitsinfo = atoi(videoinput_bitsinfo_value);
    if (videoinput_bitsinfo) {
        OMX_SetParameter(m_hDecoder,
            (OMX_INDEXTYPE)OMX_QcomIndexParamVideoInputBitsInfoExtraData,
            (OMX_PTR)&extra_data);
    }

    property_get("vidc.vdec.debug.framepacking", framepack_value, "0");
    framepack = atoi(framepack_value);
    if (framepack) {
        VTEST_MSG_HIGH("Enabling extradata : OMX_QcomIndexParamVideoFramePackingExtradata");
        result = OMX_SetParameter(m_hDecoder,
             (OMX_INDEXTYPE)OMX_QcomIndexParamVideoFramePackingExtradata,
             (OMX_PTR)&extra_data);
        FAILED1(result, "Failed to set param type : OMX_QcomIndexParamVideoFramePackingExtradata");
    }

    property_get("vidc.vdec.debug.extn_userdata", extn_userdata_value, "0");
    extn_userdata = atoi(extn_userdata_value);
    VTEST_MSG_HIGH("vidc.dec.debug.extn_userdata value is %d", (int)extn_userdata);
    if (extn_userdata) {
        VTEST_MSG_HIGH("Enabling user extradata : OMX_QcomIndexEnableExtnUserData");
        result = OMX_SetParameter(m_hDecoder,
             (OMX_INDEXTYPE)OMX_QcomIndexEnableExtnUserData,
             (OMX_PTR)&extra_data);
        FAILED1(result, "Failed to set param type : OMX_QcomIndexEnableExtnUserData");
    }

    property_get("vidc.vdec.debug.mpeg2seqdisplay", property_value, "0");
    if (atoi(property_value)) {
        VTEST_MSG_HIGH("Enabling extradata: OMX_QcomIndexParamMpeg2SeqDispExtraData");
        result = OMX_SetParameter(m_hDecoder,
             (OMX_INDEXTYPE)OMX_QcomIndexParamMpeg2SeqDispExtraData,
             (OMX_PTR)&extra_data);
        FAILED1(result, "Failed to set param type: OMX_QcomIndexParamMpeg2SeqDispExtraData");
    }

    // PlaybackMode
    switch (m_ePlaybackMode) {

        OMX_INDEXTYPE index;
        OMX_STRING name;

        case QCSmoothStreaming:
            VTEST_MSG_HIGH("Enabling QCSmoothStreaming");
            result = OMX_SetParameter(m_hDecoder,
                    (OMX_INDEXTYPE)OMX_QcomIndexParamEnableSmoothStreaming,
                    (OMX_PTR)&m_sPortFmt);
            FAILED1(result, "Failed to set QCSmoothStreaming");
            break;
        case AdaptiveSmoothStreaming:
            VTEST_MSG_HIGH("Enabling AdaptiveSmoothStreaming");
            name = const_cast<OMX_STRING>(
                    "OMX.google.android.index.prepareForAdaptivePlayback");
            result = OMX_GetExtensionIndex(m_hDecoder, name, &index);
            FAILED1(result, "OMX_GetExtensionIndex %s failed", name);

            PrepareForAdaptivePlaybackParams params;
            params.nSize = sizeof(params);
            params.nVersion.s.nVersionMajor = 1;
            params.nVersion.s.nVersionMinor = 0;
            params.nVersion.s.nRevision = 0;
            params.nVersion.s.nStep = 0;
            params.nPortIndex = OMX_DirOutput;
            params.bEnable = OMX_TRUE;
            params.nMaxFrameWidth = pConfig->nAdaptiveWidth;
            params.nMaxFrameHeight = pConfig->nAdaptiveHeight;

            result = OMX_SetParameter(m_hDecoder, index, &params);
            FAILED1(result, "Failed to set AdaptiveSmoothStreaming");
            break;
        case DynamicBufferMode:
            VTEST_MSG_ERROR("DynamicBufferMode currently unsupported");
            return OMX_ErrorBadParameter;
        default:
            VTEST_MSG_HIGH("Running in default DynamicPortReconfig mode");
            break;
    }

    // Query the decoder input buffer requirements
    OMX_INIT_STRUCT(&m_sPortFmt, OMX_PARAM_PORTDEFINITIONTYPE);
    m_sPortFmt.nPortIndex = m_sPortParam.nStartPortNumber;
    result = OMX_GetParameter(m_hDecoder, OMX_IndexParamPortDefinition, &m_sPortFmt);
    FAILED1(result, "Decoder: error OMX_IndexParamPortDefinition");

    VTEST_MSG_HIGH("Decoder: Input buffer count min %u\n", (unsigned int)m_sPortFmt.nBufferCountMin);
    VTEST_MSG_HIGH("Decoder: Input buffer size %u\n", (unsigned int)m_sPortFmt.nBufferSize);
    m_nInputBufferSize = m_sPortFmt.nBufferSize;
    if (m_sPortFmt.eDir != OMX_DirInput) {
        VTEST_MSG_ERROR("Decoder: expected input port");
        return OMX_ErrorUndefined;
    }

    // Set the frame and re-query the decoder input buffer requirements
    m_sPortFmt.format.video.nFrameHeight = m_nFrameHeight;
    m_sPortFmt.format.video.nFrameWidth  = m_nFrameWidth;
    m_sPortFmt.format.video.xFramerate = m_nFramerate;
    result = OMX_SetParameter(m_hDecoder, OMX_IndexParamPortDefinition, (OMX_PTR)&m_sPortFmt);
    FAILED1(result, "Decoder: error set OMX_IndexParamPortDefinition");

    result = OMX_GetParameter(m_hDecoder, OMX_IndexParamPortDefinition, (OMX_PTR)&m_sPortFmt);
    FAILED1(result, "Decoder: error get OMX_IndexParamPortDefinition");

    VTEST_MSG_HIGH("Decoder: Actual Input Buffer Count %u", (unsigned int)m_sPortFmt.nBufferCountActual);

    // override input buffer count if needed
    if (m_nInputBufferCount < m_sPortFmt.nBufferCountActual) {
        m_nInputBufferCount = m_sPortFmt.nBufferCountActual;
    }

    // Get output port buffer requirements
    m_sPortFmt.nPortIndex = m_sPortParam.nStartPortNumber + 1;

    if (pConfig->bDecoderDownScalar) {
        QOMX_INDEXDOWNSCALAR downScalar;
        downScalar.bEnable = OMX_FALSE;
        if (pConfig->nOutputFrameWidth && pConfig->nOutputFrameHeight) {
            downScalar.bEnable = OMX_TRUE;
            VTEST_MSG_HIGH("Decoder: Enabling downscalar to component, downscaled Height = %u Width = %u",
                        pConfig->nOutputFrameHeight, pConfig->nOutputFrameWidth);
            result = OMX_SetParameter(m_hDecoder,
                        (OMX_INDEXTYPE)OMX_QcomIndexParamVideoDownScalar,
                        (OMX_PTR)&downScalar);
            FAILED1(result, "Could not set DecoderDownScalar");

            result = OMX_GetParameter(m_hDecoder, OMX_IndexParamPortDefinition, (OMX_PTR)&m_sPortFmt);
            FAILED1(result, "Decoder: error get output width and height");
            // Set the width and height on output port
            m_sPortFmt.format.video.nFrameHeight = pConfig->nOutputFrameHeight;
            m_sPortFmt.format.video.nFrameWidth  = pConfig->nOutputFrameWidth;
            result = OMX_SetParameter(m_hDecoder, OMX_IndexParamPortDefinition, (OMX_PTR)&m_sPortFmt);
            FAILED1(result, "Decoder: error set output width and height");
        }
    }

    OMX_GetParameter(m_hDecoder, OMX_IndexParamPortDefinition, &m_sPortFmt);
    FAILED1(result, "Decoder: Error - OMX_IndexParamPortDefinition output");

    if (m_sPortFmt.eDir != OMX_DirOutput) {
        VTEST_MSG_ERROR("Error - Expect Output Port\n");
        return OMX_ErrorUndefined;
    }

    VTEST_MSG_HIGH("Decoder: Output buffer count min %u\n", (unsigned int)m_sPortFmt.nBufferCountActual);
    VTEST_MSG_HIGH("Decoder: Output buffer size %u\n", (unsigned int)m_sPortFmt.nBufferSize);
    m_nOutputBufferSize = m_sPortFmt.nBufferSize;
    // override input buffer count if needed
    if (m_nOutputBufferCount < m_sPortFmt.nBufferCountActual) {
        m_nOutputBufferCount = m_sPortFmt.nBufferCountActual;
    }
    m_nOriginalOutputBufferCount = m_nOutputBufferCount;

    // set decode color pallet
    OMX_COLOR_FORMATTYPE color_fmt;
    color_fmt = (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m;
    VTEST_MSG_HIGH("Decoder: Format[0x%x] requested", color_fmt);

    OMX_S32 index = 0;
    OMX_VIDEO_PARAM_PORTFORMATTYPE videoportFmt;
    OMX_INIT_STRUCT(&videoportFmt, OMX_VIDEO_PARAM_PORTFORMATTYPE);

    while (result == OMX_ErrorNone) {
        videoportFmt.nPortIndex = OMX_DirOutput;
        videoportFmt.nIndex = index;
        result = OMX_GetParameter(m_hDecoder,
                OMX_IndexParamVideoPortFormat, (OMX_PTR)&videoportFmt);
        if ((result == OMX_ErrorNone) && (videoportFmt.eColorFormat == color_fmt)) {
            VTEST_MSG_HIGH("Decoder: Format[0x%x] supported by OMX Decoder", color_fmt);
            break;
        }
        index++;
    }
    FAILED1(result, "Decoder: Error in retrieving supported color formats");


    result = OMX_SetParameter(m_hDecoder, OMX_IndexParamVideoPortFormat,
                              (OMX_PTR) & videoportFmt);
    FAILED1(result, "Decoder: Setting format[0x%x] failed", color_fmt);

    // set picture order
    QOMX_VIDEO_DECODER_PICTURE_ORDER picture_order;
    OMX_INIT_STRUCT(&picture_order, QOMX_VIDEO_DECODER_PICTURE_ORDER);

    picture_order.eOutputPictureOrder = pConfig->eDecoderPictureOrder;
    picture_order.nPortIndex = OMX_DirOutput;
    OMX_SetParameter(m_hDecoder,
            (OMX_INDEXTYPE)OMX_QcomIndexParamVideoDecoderPictureOrder,
            (OMX_PTR)&picture_order);
    FAILED1(result, "Decoder: ERROR: Setting picture order!");

    VTEST_MSG_HIGH("Decoder: Video format: W x H (%u x %u)",
                   (unsigned int)m_sPortFmt.format.video.nFrameWidth,
                   (unsigned int)m_sPortFmt.format.video.nFrameHeight);

    if (eFileType == FILE_TYPE_264_NAL_SIZE_LENGTH) {
        nNalSize = 4;
    }
    if (m_eCodec == OMX_VIDEO_CodingAVC) {

        OMX_VIDEO_CONFIG_NALSIZE naluSize;
        naluSize.nNaluBytes = nNalSize; //2; //appears to always be 2 or 4
        result = OMX_SetConfig(m_hDecoder, OMX_IndexConfigVideoNalSize, (OMX_PTR)&naluSize);
        FAILED(result, "Decoder: ERROR: OMX_IndexConfigVideoNalSize");
        VTEST_MSG_MEDIUM("Decoder: Nal length set to %u", (unsigned int)naluSize.nNaluBytes);
    }

    result = GetGraphicBufferUsage(&m_nOutputBufferUsage);
    FAILED1(result, "Failed to get graphic buffer usage flags");

    // go to Idle, so we can allocate buffers
    result = SetState(OMX_StateIdle, OMX_FALSE);
    FAILED1(result, "Could not move to OMX_StateIdle!");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::SetBuffer(BufferInfo *pBuffer, ISource *pSource) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    {
        Mutex::Autolock autoLock(m_pLock);
        /* TODO : give all buffers back to Buffer Manager */
        if (m_bThreadStop) {
            result = m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
            return OMX_ErrorNoMore;
        }
        if (m_bPortReconfig && (pSource == m_pSink)) {
            result = m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
            return result;
        }
    }

    result = ISource::SetBuffer(pBuffer, pSource);
    if (result != OMX_ErrorNone) {
        return result;
    }

    {
        Mutex::Autolock autoLock(m_pLock);
        if ((pBuffer->pHeaderIn->nFlags & OMX_BUFFERFLAG_EOS)
                && (pSource == m_pSource)) {
            VTEST_MSG_HIGH("Got input EOS");
            m_bInputEOSReached = OMX_TRUE;
        }
    }

    if (pSource == m_pSource) {  //input port

        VTEST_MSG_LOW("%s ==> %s: ETB", pSource->Name(), Name());
        result = OMX_EmptyThisBuffer(m_hDecoder, pBuffer->pHeaderIn);
        FAILED1(result, "ETB error, 0x%lx (%p %p)",
                pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    } else if (pSource == m_pSink) { //output port

        VTEST_MSG_LOW("%s ==> %s: FTB", pSource->Name(), Name());
        result = OMX_FillThisBuffer(m_hDecoder, pBuffer->pHeaderOut);
        FAILED1(result, "FTB error, 0x%lx (%p %p)",
                pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    } else {
        VTEST_MSG_ERROR("Invalid source : %s", pSource->Name());
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::AllocateBuffers(BufferInfo **pBuffers,
        OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
        OMX_U32 nBufferSize, OMX_U32 ePortIndex, OMX_U32 nBufferUsage) {

    (void)nWidth, (void)nHeight, (void)nBufferUsage;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_U32 nOriginalBufferCount = -1;
    OMX_U32 nOriginalBufferSize = -1;

    Mutex::Autolock autoLock(m_pLock);
    VTEST_MSG_HIGH("Alloc %s size %u count %u",
            OMX_PORT_NAME(ePortIndex), (unsigned int)nBufferSize, (unsigned int)nBufferCount);

    if (ePortIndex == PORT_INDEX_IN) {
        nOriginalBufferCount = m_nInputBufferCount;
        nOriginalBufferSize = m_nInputBufferSize;
        m_sPortFmt.nPortIndex = m_sPortParam.nStartPortNumber;
    } else if (ePortIndex == PORT_INDEX_OUT) {
        nOriginalBufferCount = m_nOutputBufferCount;
        nOriginalBufferSize = m_nOutputBufferSize;
        m_sPortFmt.nPortIndex = m_sPortParam.nStartPortNumber + 1;
    }

    if ((nOriginalBufferCount > nBufferCount) ||
        (nOriginalBufferSize > nBufferSize)) {
        VTEST_MSG_ERROR(
            "Allocate Buffers failure : original count : %u, new count : %u, "
            "original size : %u, new size : %u",
            (unsigned int)nOriginalBufferCount, (unsigned int)nBufferCount, (unsigned int)nOriginalBufferSize, (unsigned int)nBufferSize);
        return OMX_ErrorBadParameter;
    }

    OMX_GetParameter(m_hDecoder, OMX_IndexParamPortDefinition, &m_sPortFmt);
    FAILED1(result, "Decoder: Error - OMX_IndexParamPortDefinition output");

    m_sPortFmt.nBufferCountActual = nBufferCount;
    m_sPortFmt.nBufferSize = nBufferSize;
    result = OMX_SetParameter(m_hDecoder,
            OMX_IndexParamPortDefinition, (OMX_PTR)&m_sPortFmt);
    FAILED1(result, "Decoder: error set OMX_IndexParamPortDefinition");

    *pBuffers = new BufferInfo[nBufferCount];
    memset(*pBuffers, 0, sizeof(BufferInfo) * nBufferCount);
    for (OMX_U32 i = 0; i < nBufferCount; i++) {

        BufferInfo *pBuffer = &((*pBuffers)[i]);
        OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);

        result = OMX_AllocateBuffer(m_hDecoder, pHeaderPtr,
                ePortIndex, this, nBufferSize);
        FAILED1(result, "error allocating buffer");

        //fix pHandle, ok as it is not currently used
        pBuffer->pHandle = 0xDEADBEEF;
        pBuffer->pHeaderIn = pBuffer->pHeaderOut = *pHeaderPtr;

        VTEST_MSG_HIGH("%s alloc_ct=%u sz=%u handle=0x%x hdr=(%p %p)",
            OMX_PORT_NAME(ePortIndex), (unsigned int)i + 1, (unsigned int)nBufferSize,
            (unsigned int)pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    }

    if (ePortIndex == PORT_INDEX_IN) {
        m_nInputBufferCount = nBufferCount;
        m_nInputBufferSize = nBufferSize;
    } else if (ePortIndex == PORT_INDEX_OUT) {
        m_nOutputBufferCount = nBufferCount;
        m_nOutputBufferSize = nBufferSize;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::UseBuffers(BufferInfo **pBuffers,
        OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
        OMX_U32 nBufferSize, OMX_U32 ePortIndex) {

    (void)nWidth, (void)nHeight;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_U32 nOriginalBufferCount = -1;
    OMX_U32 nOriginalBufferSize = -1;
    Mutex::Autolock autoLock(m_pLock);

    if (ePortIndex == PORT_INDEX_IN) {
        nOriginalBufferCount = m_nInputBufferCount;
        nOriginalBufferSize = m_nInputBufferSize;
        m_sPortFmt.nPortIndex = m_sPortParam.nStartPortNumber;
    } else if (ePortIndex == PORT_INDEX_OUT) {
        nOriginalBufferCount = m_nOutputBufferCount;
        nOriginalBufferSize = m_nOutputBufferSize;
        m_sPortFmt.nPortIndex = m_sPortParam.nStartPortNumber + 1;
    }

    VTEST_MSG_HIGH("Decoder: %s size %u count %u",
        OMX_PORT_NAME(ePortIndex), (unsigned int)nBufferSize, (unsigned int)nBufferCount);
    if ((nOriginalBufferCount > nBufferCount)
            || (nOriginalBufferSize > nBufferSize)) {
        VTEST_MSG_ERROR("Use Buffers failure : original count : %u, new count : %u, original size : %u, new size : %u",
                (unsigned int)nOriginalBufferCount, (unsigned int)nBufferCount, (unsigned int)nOriginalBufferSize, (unsigned int)nBufferSize);
        return OMX_ErrorBadParameter;
    }

    if (strstr(m_pSink->Name(), "Native") != NULL) {
        VTEST_MSG_LOW("SetNativeWindowEnable");
        result = SetNativeWindowEnable();
        FAILED(result, "Could not enable native window on component");
    }

    OMX_GetParameter(m_hDecoder, OMX_IndexParamPortDefinition, &m_sPortFmt);
    FAILED1(result, "Decoder: Error - OMX_IndexParamPortDefinition output");

    m_sPortFmt.nBufferCountActual = nBufferCount;
    m_sPortFmt.nBufferSize = nBufferSize;
    result = OMX_SetParameter(m_hDecoder,
            OMX_IndexParamPortDefinition, (OMX_PTR)&m_sPortFmt);
    FAILED1(result, "Decoder: error set OMX_IndexParamPortDefinition");

    for (OMX_U32 i = 0; i < nBufferCount; i++) {

        BufferInfo *pBuffer = &((*pBuffers)[i]);
        OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);

        result = OMX_UseBuffer(m_hDecoder, pHeaderPtr,
                ePortIndex, this, nBufferSize, (OMX_U8 *)pBuffer->pHandle);
        FAILED(result, "error in use buffer");

        // for NativeWindow case, supply all headers
        pBuffer->pHeaderIn = (pBuffer->pHeaderIn == NULL)
                                ? *pHeaderPtr : pBuffer->pHeaderIn;
        pBuffer->pHeaderOut = (pBuffer->pHeaderOut == NULL)
                                ? *pHeaderPtr : pBuffer->pHeaderOut;

        VTEST_MSG_HIGH("%s use_ct=%u sz=%u handle=0x%x hdr=(%p %p)",
                OMX_PORT_NAME(ePortIndex), (unsigned int)i + 1, (unsigned int)nBufferSize,
            (unsigned int)pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    }
    if (ePortIndex == PORT_INDEX_IN) {
        m_nInputBufferCount = nBufferCount;
        m_nInputBufferSize = nBufferSize;
    } else if (ePortIndex == PORT_INDEX_OUT) {
        m_nOutputBufferCount = nBufferCount;
        m_nOutputBufferSize = nBufferSize;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::ThreadRun(OMX_PTR pThreadData) {

    (void)pThreadData;
    OMX_ERRORTYPE result = OMX_ErrorNone;

    while (m_bThreadStop == OMX_FALSE) {
        if (m_bPortReconfig) {
            result = PortReconfigOutput();
            FAILED2(result, SetError(), "PortReconfigOutput failed");
        }
        Sleeper::Sleep(10);
    }

    VTEST_MSG_HIGH("thread exiting...");
    return result;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::FreeBuffer(
        BufferInfo *pBuffer, OMX_U32 ePortIndex) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);
    if (*pHeaderPtr == NULL) {
        return result;
    }

    VTEST_MSG_MEDIUM("Freeing buffer %p 0x%lu",
            *pHeaderPtr, pBuffer->pHandle);

    result = OMX_FreeBuffer(m_hDecoder, ePortIndex, *pHeaderPtr);
    FAILED1(result, "Error freeing %s", OMX_PORT_NAME(ePortIndex));

    *pHeaderPtr = NULL;
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::SetState(
        OMX_STATETYPE eState, OMX_BOOL bSynchronous) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    /*if we get an error before idle complete (allocation failure) and
     * we try to move to loaded in the destructor*/
    if (eState == m_eState) {
        return result;
    }

    // check for pending state transition
    if (m_eState != m_eStatePending) {
        result = WaitState(m_eStatePending);
        FAILED1(result, "wait for %s failed, waiting for alloc/dealloc?!?!",
                OMX_STATE_NAME(m_eStatePending));
    }

    // check for invalid transition
    if ((eState == OMX_StateLoaded && m_eState != OMX_StateIdle) ||
        (eState == OMX_StateExecuting && m_eState != OMX_StateIdle)) {
        VTEST_MSG_ERROR(
            "Error: invalid state tranisition: state %s to %s",
            OMX_STATE_NAME(eState), OMX_STATE_NAME(m_eState));
        result = OMX_ErrorIncorrectStateTransition;
    }

    VTEST_MSG_MEDIUM("goto state %s...", OMX_STATE_NAME(eState));
    result = OMX_SendCommand(
            m_hDecoder, OMX_CommandStateSet, (OMX_U32)eState, NULL);
    m_eStatePending = eState;

    FAILED1(result, "failed to set state");
    if (bSynchronous == OMX_TRUE) {
        result = WaitState(eState);
        FAILED1(result, "failed to wait in set state in synchronous mode");
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::WaitState(OMX_STATETYPE eState) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    CmdType cmd;

    VTEST_MSG_MEDIUM("waiting for state %s...", OMX_STATE_NAME(eState));
    m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0); // infinite wait
    result = cmd.eResult;

    if (cmd.eEvent == OMX_EventCmdComplete) {

        if (cmd.eCmd == OMX_CommandStateSet) {

            if ((OMX_STATETYPE)cmd.sCmdData == eState) {
                m_eState = (OMX_STATETYPE)cmd.sCmdData;
            } else {
                VTEST_MSG_ERROR("wrong state (%d)", (int)cmd.sCmdData);
                result = OMX_ErrorUndefined;
            }

        } else {
            VTEST_MSG_ERROR("expecting state change");
            result = OMX_ErrorUndefined;
        }

    } else {
        VTEST_MSG_ERROR("expecting cmd complete");
        result = OMX_ErrorUndefined;
    }

    if (result == OMX_ErrorNone) {
        VTEST_MSG_MEDIUM("reached state %s...", OMX_STATE_NAME(eState));
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::HandleOutputPortSettingsChange(OMX_U32 nData2) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_U32 nOutPortIndex = 1;
    OMX_CONFIG_RECTTYPE rect;

    Mutex::Autolock autoLock(m_pLock);

    if (nData2 == OMX_IndexConfigCommonOutputCrop || nData2 == OMX_IndexConfigCommonScale) {
        rect.nPortIndex = nOutPortIndex;
        result = OMX_GetConfig(m_hDecoder, OMX_IndexConfigCommonOutputCrop, &rect);
        FAILED1(result, "Failed to get crop rectangle");
        VTEST_MSG_HIGH("Got Crop Rect: (%d, %d) (%u x %u)",
                       (int)rect.nLeft, (int)rect.nTop, (unsigned int)rect.nWidth, (unsigned int)rect.nHeight);

        result = m_pSink->PortReconfig(
                PORT_INDEX_IN, m_nFrameWidth, m_nFrameHeight, rect);
        FAILED1(result, "GotPortReconfig failed on sink");
    } else if (nData2 == 0 || nData2 == OMX_IndexParamPortDefinition) {
        // check all buffers are with us (in Allocate buffer case)
        assert(m_eState != OMX_StateExecuting);
        VTEST_MSG_MEDIUM("Reconfiguring output port");
        m_bPortReconfig = OMX_TRUE;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::PortReconfigOutput() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_U32 nOutPortIndex = 1;
    OMX_CONFIG_RECTTYPE rect;
    CmdType cmd;

    VTEST_MSG_MEDIUM("PortReconfig Calling Flush on output port");
    Flush(PORT_INDEX_OUT);

    VTEST_MSG_MEDIUM("PortReconfig OMX_CommandPortDisable");
    result = OMX_SendCommand(m_hDecoder, OMX_CommandPortDisable, PORT_INDEX_OUT, 0);
    FAILED1(result, "Disable Output Port failed");

    // Wait for OMX_comp/sink to return all buffers
    BufferInfo *pBuffer = NULL;
    OMX_U32 nFreeBufferCount = 0;
    while (m_pFreeBufferQueue->GetSize() > 0) {

        result = m_pFreeBufferQueue->Pop(&pBuffer, sizeof(pBuffer), 0);
        if ((pBuffer == NULL) || (result != OMX_ErrorNone)) {
            VTEST_MSG_ERROR("Error in getting buffer, it might be null");
        } else {
            VTEST_MSG_MEDIUM("PortReconfig free buffer count %u",
                    (unsigned int)nFreeBufferCount);
        }
        nFreeBufferCount++;
    }

    // Free all old buffers
    VTEST_MSG_MEDIUM("PortReconfig free OUTPUT buffers");
    m_pBufferManager->FreeBuffers(this, PORT_INDEX_OUT);

    // wait for OMX_comp to respond OMX_CommandPortDisable
    // this only happens once all buffers are freed
    VTEST_MSG_MEDIUM("PortReconfig getting response");
    if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone) {
        FAILED1(result, "Error popping signal");
    }
    if (cmd.eResult != OMX_ErrorNone || cmd.eEvent != OMX_EventCmdComplete ||
        cmd.eCmd != OMX_CommandPortDisable) {
        result = OMX_ErrorUndefined;
        FAILED1(result, "expecting complete for command : %d", OMX_CommandPortDisable);
    }

    // ask OMX_comp for new settings
    VTEST_MSG_MEDIUM("PortReconfig get new settings");
    m_sPortFmt.nPortIndex = nOutPortIndex;
    OMX_GetParameter(m_hDecoder, OMX_IndexParamPortDefinition, &m_sPortFmt);
    if (m_sPortFmt.eDir != OMX_DirOutput) {
        VTEST_MSG_ERROR("Error - Expect Output Port\n");
        return OMX_ErrorUndefined;
    }
    m_nFrameHeight = m_sPortFmt.format.video.nFrameHeight;
    m_nFrameWidth = m_sPortFmt.format.video.nFrameWidth;
    m_nOutputBufferCount = m_sPortFmt.nBufferCountActual;

    /* Update original count here as set by decoder again
     * otherwise we will always allocate the number of
     * buffers set before reconfig. (As read by BufferManager
     * via GetBufferRequirements) */
    m_nOriginalOutputBufferCount = m_nOutputBufferCount;

    m_nOutputBufferSize = m_sPortFmt.nBufferSize;
    rect.nLeft = 0;
    rect.nTop = 0;
    rect.nWidth = m_nFrameWidth;
    rect.nHeight = m_nFrameHeight;
    VTEST_MSG_MEDIUM("PortReconfig Min Buffer Count = %u", (unsigned int)m_nOutputBufferCount);
    VTEST_MSG_MEDIUM("PortReconfig Buffer Size = %u", (unsigned int)m_nOutputBufferSize);
    VTEST_MSG_MEDIUM("PortReconfig width : %u, height : %u", (unsigned int)m_nFrameWidth, (unsigned int)m_nFrameHeight);

    // notify sink PortReconf done
    VTEST_MSG_MEDIUM("PortReconfig restarting sink");
    result = m_pSink->PortReconfig(
            PORT_INDEX_IN, m_nFrameWidth, m_nFrameHeight, rect);
    FAILED1(result, "PortReconfig done failed on sink");

    VTEST_MSG_MEDIUM("PortReconfig re-enabling port");
    result = OMX_SendCommand(m_hDecoder, OMX_CommandPortEnable, PORT_INDEX_OUT, 0);
    FAILED1(result, "Enable Output Port failed");

    // re-allocate all buffers on the port
    VTEST_MSG_MEDIUM("PortReconfig re-allocate OUTPUT buffers");
    result = m_pBufferManager->SetupBufferPool(this, m_pSink);
    FAILED1(result, "Error in realloacting buffer pool");

    // wait for OMX_comp to respond OMX_CommandPortEnabled
    // this only happens once all buffers are allocated
    VTEST_MSG_MEDIUM("PortReconfig getting response");
    if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone) {
        VTEST_MSG_ERROR("error popping signal");
    }
    if (cmd.eResult != OMX_ErrorNone || cmd.eEvent != OMX_EventCmdComplete ||
        cmd.eCmd != OMX_CommandPortEnable) {
        VTEST_MSG_ERROR("expecting complete for command : %d", OMX_CommandPortEnable);
        result = OMX_ErrorUndefined;
    }
    m_bPortReconfig = OMX_FALSE;

    // enqueue newly allocated buffers
    OMX_U32 nBufferCount;
    BufferInfo *pBuffers;
    result = m_pBufferManager->GetBuffers(this, PORT_INDEX_OUT, &pBuffers, &nBufferCount);
    FAILED1(result, "Error in Getting Buffers");

    for (OMX_U32 i = 0; i < nBufferCount; i++) {
        VTEST_MSG_MEDIUM("port-reconfig requeue buffer %lu (%p %p)",
                pBuffers[i].pHandle,
                pBuffers[i].pHeaderIn, pBuffers[i].pHeaderOut);
        result = SetBuffer(&pBuffers[i], m_pSink);
        FAILED1(result, "Error setting port buffer %p", pBuffers[i].pHeaderOut);
    }

    VTEST_MSG_MEDIUM("port-reconfig done");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::SetNativeWindowEnable() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_INDEXTYPE index;
    OMX_VERSIONTYPE ver;

    ver.s.nVersionMajor = 1;
    ver.s.nVersionMinor = 0;
    ver.s.nRevision = 0;
    ver.s.nStep = 0;

    result  = OMX_GetExtensionIndex(m_hDecoder,
        const_cast<OMX_STRING>(
            "OMX.google.android.index.enableAndroidNativeBuffers"), &index);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("OMX_GetExtensionIndex failed");
        return result;
    }

    OMX_U32 nOutPortIndex = 1;
    EnableAndroidNativeBuffersParams params = {
        sizeof(EnableAndroidNativeBuffersParams), ver, nOutPortIndex, OMX_TRUE,
    };

    result = OMX_SetParameter(m_hDecoder, index, &params);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("OMX_EnableAndroidNativeBuffers failed");
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::GetGraphicBufferUsage(OMX_U32 *nBufferUsage) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_INDEXTYPE index;
    OMX_VERSIONTYPE ver;

    *nBufferUsage = 0;
    ver.s.nVersionMajor = 1;
    ver.s.nVersionMinor = 0;
    ver.s.nRevision = 0;
    ver.s.nStep = 0;

    //Get graphic buffer usage flags from component
    result  = OMX_GetExtensionIndex(
        m_hDecoder,
        const_cast<OMX_STRING>(
            "OMX.google.android.index.getAndroidNativeBufferUsage"),
        &index);
    FAILED1(result, "OMX_GetExtensionIndex failed for getAndroidNativeBufferUsage");

    OMX_U32 nOutPortIndex = 1;

    GetAndroidNativeBufferUsageParams params = {
        sizeof(GetAndroidNativeBufferUsageParams), ver, nOutPortIndex, 0,
    };

    result = OMX_GetParameter(m_hDecoder, index, &params);
    FAILED1(result, "OMX_GetAndroidNativeBufferUsage failed with error %d", result);

    *nBufferUsage = params.nUsage;
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::EventCallback(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData, OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2, OMX_IN OMX_PTR pEventData) {

    (void)hComponent; (void)pEventData;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    Decoder *pThis = (Decoder *)pAppData;

    if (eEvent == OMX_EventCmdComplete) {

        if ((OMX_COMMANDTYPE)nData1 == OMX_CommandStateSet) {
            VTEST_MSG_HIGH("Event callback: state is %s",
                           OMX_STATE_NAME((OMX_STATETYPE)nData2));

            CmdType cmd;
            cmd.eEvent = OMX_EventCmdComplete;
            cmd.eCmd = OMX_CommandStateSet;
            cmd.sCmdData = nData2;
            cmd.eResult = result;
            result = pThis->m_pSignalQueue->Push(&cmd, sizeof(cmd));
            FAILED1(result, "push to signal queue failed");

        } else if ((OMX_COMMANDTYPE)nData1 == OMX_CommandFlush) {
            VTEST_MSG_MEDIUM("Event callback: flush complete on port : %s",
                    OMX_PORT_NAME(nData2));

            CmdType cmd;
            cmd.eEvent = OMX_EventCmdComplete;
            cmd.eCmd = OMX_CommandFlush;
            cmd.sCmdData = 0;
            cmd.eResult = result;
            result = pThis->m_pSignalQueue->Push(&cmd, sizeof(cmd));
            FAILED1(result, "push to signal queue failed");

        } else if ((OMX_COMMANDTYPE)nData1 == OMX_CommandPortDisable) {
            VTEST_MSG_MEDIUM("Event callback: %s port disable",
                    OMX_PORT_NAME(nData2));

            /* Only queue port disable during port reconfig,
             * during stop, it clashes with the event for moving
             * to loaded state */
            if (pThis->m_bPortReconfig) {
                CmdType cmd;
                cmd.eEvent = OMX_EventCmdComplete;
                cmd.eCmd = OMX_CommandPortDisable;
                cmd.sCmdData = 0;
                cmd.eResult = result;
                result = pThis->m_pSignalQueue->Push(&cmd, sizeof(cmd));
                FAILED1(result, "push to signal queue failed");
            }

        } else if ((OMX_COMMANDTYPE)nData1 == OMX_CommandPortEnable) {
            VTEST_MSG_MEDIUM("Event callback: %s port enable",
                    OMX_PORT_NAME(nData2));

            CmdType cmd;
            cmd.eEvent = OMX_EventCmdComplete;
            cmd.eCmd = OMX_CommandPortEnable;
            cmd.sCmdData = 0;
            cmd.eResult = result;
            result = pThis->m_pSignalQueue->Push(&cmd, sizeof(cmd));
            FAILED1(result, "push to signal queue failed");

        } else {
            result = OMX_ErrorUndefined;
            FAILED2(result, pThis->SetError(), "Unimplemented command");
        }
    } else if (eEvent == OMX_EventError) {
        result = OMX_ErrorUndefined;
        FAILED2(result, pThis->SetError(),
                "Event callback: async error nData1 %u, nData2 %u",
                (unsigned int)nData1, (unsigned int)nData2);
    } else if (eEvent == OMX_EventBufferFlag) {
        VTEST_MSG_MEDIUM("got buffer flag event");
    } else if (eEvent ==  OMX_EventPortSettingsChanged) {
        VTEST_MSG_MEDIUM("OMX_EventPortSettingsChanged port[%u]\n", (unsigned int)nData1);

        OMX_U32 nOutPortIndex = 1;
        if (nData1 == nOutPortIndex) {
            result = pThis->HandleOutputPortSettingsChange(nData2);
        } else {
            VTEST_MSG_ERROR("reconfig not supported on port : %u", (unsigned int)nData1);
        }
    } else {
        result = OMX_ErrorUndefined;
        FAILED2(result, pThis->SetError(), "Unimplemented event");
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::EmptyDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pHeader) {

    (void)hComponent;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    Decoder *pThis = (Decoder *)pAppData;
    BufferInfo *pBuffer = NULL;

    VTEST_MSG_MEDIUM("EBD %p", pHeader);
    result = pThis->m_pBufferManager->GetBuffer(
            pThis, PORT_INDEX_IN, pHeader, &pBuffer);
    FAILED2(result, pThis->SetError(),
            "Error in finding buffer %p", pBuffer);

    Mutex::Autolock autoLock(pThis->m_pLock);

    /* TODO : give all buffers back to Buffer Manager */
    if (pThis->m_bThreadStop || pThis->m_bInputEOSReached) {
        result = pThis->m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
        return result;
    }
    result = pThis->m_pSource->SetBuffer(pBuffer, pThis);
    if (result != OMX_ErrorNone) {
        /* Don't treat it as fatal, because there is possibility where the
         * eos hasn't reached us and source is not expecting any more buffers */
        VTEST_MSG_HIGH("SetBuffer on source failed pBuffer: %p", pBuffer);
        pThis->m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::FillDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pHeader) {

    (void)hComponent;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    Decoder *pThis = (Decoder *)pAppData;
    BufferInfo *pBuffer = NULL;

    VTEST_MSG_MEDIUM("FBD %p", pHeader);
    result = pThis->m_pBufferManager->GetBuffer(
        pThis, PORT_INDEX_OUT, pHeader, &pBuffer);
    FAILED2(result, pThis->SetError(),
            "Error in finding buffer %p", pBuffer);

    Mutex::Autolock autoLock(pThis->m_pLock);

    /* TODO : give all buffers back to Buffer Manager */
    if (pThis->m_bPortReconfig || pThis->m_bThreadStop) {
        result = pThis->m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
        return result;
    }

    result = pThis->m_pSink->SetBuffer(pBuffer, pThis);
    if (pHeader->nFlags & OMX_BUFFERFLAG_EOS) {
        VTEST_MSG_MEDIUM("Decoder: Got output EOS\n");
        pThis->m_bThreadStop = OMX_TRUE;
    }
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("SetBuffer on sink failed pBuffer: %p", pBuffer);
        pThis->m_bThreadStop = OMX_TRUE;
        pThis->m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::GetComponentRole(
        OMX_BOOL bSecureSession, FileType *eFileType, OMX_STRING *role) {

    if (*eFileType >= FILE_TYPE_COMMON_CODEC_MAX) {
        OMX_U32 specificFileType = 0;
        switch ((int)m_eCodec) {
            case OMX_VIDEO_CodingAVC:
                specificFileType = FILE_TYPE_START_OF_H264_SPECIFIC;
                break;
            case QOMX_VIDEO_CodingDivx:
                specificFileType = FILE_TYPE_START_OF_DIVX_SPECIFIC;
                break;
            case OMX_VIDEO_CodingMPEG4:
            case OMX_VIDEO_CodingH263:
                specificFileType = FILE_TYPE_START_OF_MP4_SPECIFIC;
                break;
            case OMX_VIDEO_CodingWMV:
                specificFileType = FILE_TYPE_START_OF_VC1_SPECIFIC;
                break;
            case OMX_VIDEO_CodingMPEG2:
                specificFileType = FILE_TYPE_START_OF_MPEG2_SPECIFIC;
                break;
            case OMX_VIDEO_CodingVP8:
                specificFileType = FILE_TYPE_START_OF_VP8_SPECIFIC;
                break;
            default:
                VTEST_MSG_ERROR("Error: Unknown code %d", m_eCodec);
                return OMX_ErrorBadParameter;
        }
        *eFileType = (FileType)
            (specificFileType +
             *eFileType - FILE_TYPE_COMMON_CODEC_MAX);
    }

    switch (*eFileType) {
        case FILE_TYPE_DAT_PER_AU:
        case FILE_TYPE_ARBITRARY_BYTES:
        case FILE_TYPE_264_NAL_SIZE_LENGTH:
        case FILE_TYPE_PICTURE_START_CODE:
        case FILE_TYPE_MPEG2_START_CODE:
        case FILE_TYPE_RCV:
        case FILE_TYPE_VC1:
        case FILE_TYPE_DIVX_4_5_6:
        case FILE_TYPE_DIVX_311:
        case FILE_TYPE_VP8:
            break;
        default:
            VTEST_MSG_ERROR("Error: Unknown file type %d", *eFileType);
            return OMX_ErrorBadParameter;
    }

    switch ((int)m_eCodec) {
        case OMX_VIDEO_CodingAVC:
            *role = (OMX_STRING)"OMX.qcom.video.decoder.avc";
            break;
        case OMX_VIDEO_CodingMPEG4:
            *role = (OMX_STRING)"OMX.qcom.video.decoder.mpeg4";
            break;
        case OMX_VIDEO_CodingH263:
            *role = (OMX_STRING)"OMX.qcom.video.decoder.h263";
            break;
        case OMX_VIDEO_CodingWMV:
            if (*eFileType == FILE_TYPE_RCV) {
                *role = (OMX_STRING)"OMX.qcom.video.decoder.wmv";
            } else {
                *role = (OMX_STRING)"OMX.qcom.video.decoder.vc1";
            }
            break;
        case QOMX_VIDEO_CodingDivx:
            if (*eFileType == FILE_TYPE_DIVX_311) {
                *role = (OMX_STRING)"OMX.qcom.video.decoder.divx311";
            } else if (*eFileType == FILE_TYPE_DIVX_4_5_6) {
                *role = (OMX_STRING)"OMX.qcom.video.decoder.divx";
            } else {
                VTEST_MSG_ERROR("Error: Unsupported file type : %d for codec format : %d ",
                                m_eCodec, *eFileType);
                return OMX_ErrorBadParameter;
            }
            break;
        case OMX_VIDEO_CodingMPEG2:
            *role = (OMX_STRING)"OMX.qcom.video.decoder.mpeg2";
            break;
        case QOMX_VIDEO_CodingHevc:
            *role = (OMX_STRING)"OMX.qcom.video.decoder.hevc";
            break;
        case OMX_VIDEO_CodingVP8:
            *role = (OMX_STRING)"OMX.qcom.video.decoder.vp8";
            break;
        default:
            VTEST_MSG_ERROR("Error: Unsupported codec %d", m_eCodec);
            return OMX_ErrorBadParameter;
    }

    if (bSecureSession) {
        switch ((int)m_eCodec) {
            case OMX_VIDEO_CodingAVC:
                *role = (OMX_STRING)"OMX.qcom.video.decoder.avc.secure";
                break;
            case OMX_VIDEO_CodingMPEG2:
                *role = (OMX_STRING)"OMX.qcom.video.decoder.mpeg2.secure";
                break;
            case QOMX_VIDEO_CodingHevc:
                *role = (OMX_STRING)"OMX.qcom.video.decoder.hevc.secure";
                break;
            case OMX_VIDEO_CodingWMV:
                if (*eFileType == FILE_TYPE_RCV) {
                    *role = (OMX_STRING)"OMX.qcom.video.decoder.wmv.secure";
                } else {
                    *role = (OMX_STRING)"OMX.qcom.video.decoder.vc1.secure";
                }
                break;
            default:
                VTEST_MSG_ERROR("Secure session not supported for codec format : %d",
                                m_eCodec);
                return OMX_ErrorBadParameter;
        }
    }


    VTEST_MSG_MEDIUM("Role : %s, Codec Format : %d, File Type : %d",
                    *role, m_eCodec, *eFileType);
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Decoder::Flush(OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    CmdType cmd;

    result = OMX_SendCommand(m_hDecoder, OMX_CommandFlush, nPortIndex, 0);
    FAILED1(result, "Flush failed");

    if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone) {
        FAILED1(result, "Error popping signal");
    }
    if (cmd.eResult != OMX_ErrorNone || cmd.eEvent != OMX_EventCmdComplete ||
        cmd.eCmd != OMX_CommandFlush) {
        result = OMX_ErrorUndefined;
        VTEST_MSG_ERROR("Was expecting complete for flush command");
    }

    /* Wait for flush complete for both ports */
    if (nPortIndex == OMX_ALL) {
        if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone) {
            FAILED1(result, "Error popping signal");
        }
        if (cmd.eResult != OMX_ErrorNone || cmd.eEvent != OMX_EventCmdComplete ||
                cmd.eCmd != OMX_CommandFlush) {
            result = OMX_ErrorUndefined;
            VTEST_MSG_ERROR("Was expecting complete for flush command");
        }
    }
    return result;
}

} // namespace vtest
