/******************************************************************************
 *
 *                                 OMX Ittiam
 *
 *                     ITTIAM SYSTEMS PVT LTD, BANGALORE
 *                             COPYRIGHT(C) 2011
 *
 *  This program is proprietary to ittiam systems pvt. ltd.,and is protected
 *  under indian copyright act as an unpublished work.its use and disclosure
 *  is limited by the terms and conditions of a license agreement.it may not
 *  be copied or otherwise  reproduced or disclosed  to persons  outside the
 *  licensee's   organization  except  in  accordance   with  the  terms and
 *  conditions  of such  an agreement. all copies and reproductions shall be
 *  the property of ittiam systems pvt. ltd.  and  must bear this  notice in
 *  its entirety.
 *
 ******************************************************************************/

/****************************************************************************

 Copyright (c) 2006-2007, 2011-2012 Qualcomm Technologies Incorporated.
 All Rights Reserved. Qualcomm Proprietary and Confidential.

 *****************************************************************************/

/*============================================================================
 O p e n M A X   w r a p p e r s
 O p e n  M A X   C o r e

 *//** @file omx_vdec_qcom_wrapper.cpp
 This module contains the implementation of the OpenMAX core & component.

 *//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Index.h>
#include <OMX_Image.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include <OMX_Other.h>


#include "omx_vdec_qcom_wrapper.h"

#include "datatypedef.h"
#include "iv.h"
#include "ivd.h"
#include <OMX_Ittiam.h>
#include <OMX_Ittiam_Vdec.h>

#include <HardwareAPI.h>
#include <gui/BufferQueue.h>
#include <GraphicBuffer.h>
#include <GraphicBufferMapper.h>

#include <cutils/properties.h>

#ifdef __cplusplus
extern "C" {
#endif
    void ittiam_video_pad_output_buffer(VIDDECTYPE *pVidDec, UWORD8 *pu1_out_buf);
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

// factory function executed by the core to create instances
void *get_omx_component_factory_fn(void)
{
    return (new omx_vdec);
}


int debugLogsEnable;
#define MAX_NAME_STRLEN 64
typedef struct
{
    char name[MAX_NAME_STRLEN];
    OMX_VIDEO_CODINGTYPE format;
}ComponentMappingT;
static const ComponentMappingT gComponentMapping[] = {
    {"OMX.ittiam.video.decoder.hevc", OMX_VIDEO_CodingHEVC},
    {"OMX.ittiam.video.decoder.vp9", OMX_VIDEO_CodingVP9},
    {"OMX.ittiam.video.decoder.avc", OMX_VIDEO_CodingAVC},
    {"OMX.ittiam.video.decoder.mpeg4", OMX_VIDEO_CodingMPEG4},
    {"OMX.ittiam.video.decoder.h263", OMX_VIDEO_CodingH263},
    {"OMX.ittiam.video.decoder.divx4", OMX_VIDEO_CodingMPEG4},
    {"OMX.ittiam.video.decoder.divx", OMX_VIDEO_CodingMPEG4},
    {"OMX.ittiam.video.decoder.xvid", OMX_VIDEO_CodingMPEG4},
    {"OMX.ittiam.video.decoder.mpeg2", OMX_VIDEO_CodingMPEG2},
    {"OMX.qcom.video.decoder.hevc", OMX_VIDEO_CodingHEVC},
    {"OMX.qcom.video.decoder.vp9", OMX_VIDEO_CodingVP9},
    {"OMX.qcom.video.decoder.avc", OMX_VIDEO_CodingAVC},
    {"OMX.qcom.video.decoder.mpeg4", OMX_VIDEO_CodingMPEG4},
    {"OMX.qcom.video.decoder.h263", OMX_VIDEO_CodingH263},
    {"OMX.qcom.video.decoder.divx4", OMX_VIDEO_CodingMPEG4},
    {"OMX.qcom.video.decoder.divx", OMX_VIDEO_CodingMPEG4},
    {"OMX.qcom.video.decoder.xvid", OMX_VIDEO_CodingMPEG4},
    {"OMX.qcom.video.decoder.mpeg2", OMX_VIDEO_CodingMPEG2},
};
/* ======================================================================
 FUNCTION
 omx_vdec::omx_vdec

 DESCRIPTION
 Constructor

 PARAMETERS
 None

 RETURN VALUE
 None.
 ========================================================================== */
omx_vdec::omx_vdec() :
                hComponent(NULL)
{
    hComponent = (OMX_COMPONENTTYPE *)OMX_OSAL_Malloc(sizeof(OMX_COMPONENTTYPE));

}

/* ======================================================================
 FUNCTION
 omx_vdec::~omx_vdec

 DESCRIPTION
 Destructor

 PARAMETERS
 None

 RETURN VALUE
 None.
 ========================================================================== */
omx_vdec::~omx_vdec()
{
    if(hComponent != NULL)
    {
        OMX_OSAL_Free (hComponent);

        hComponent = NULL;
    }
}

/* ======================================================================
 FUNCTION
 omx_vdec::ComponentInit

 DESCRIPTION
 Initialize the component.

 PARAMETERS
 ctxt -- Context information related to the self.
 id   -- Event identifier. This could be any of the following:
 1. Command completion event
 2. Buffer done callback event
 3. Frame done callback event

 RETURN VALUE
 None.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::component_init(OMX_STRING cComponentName)
{

    char mDeviceName[128];
    char mDeviceHwPlat[128];
    OMX_U32 disableInterlaced = 0;
    OMX_U32 shareDispBuf = SHARE_DISP_BUF;
    OMX_U32 numCores = 1;
    OMX_U32 maxWidth = MAX_WIDTH;
    OMX_U32 maxHeight = MAX_HEIGHT;
    OMX_U32 minUndequeuedBufs = 0;
    PROCESSORTYPE processorType;
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    OMX_VIDEO_CODINGTYPE VidFormat;
    OMX_COLOR_FORMATTYPE colorFormat;


    ITTIAM_LOG("In %s function, cComponentName = %s", __FUNCTION__, cComponentName);

    minUndequeuedBufs = 0;

    {
        WORD32 i, numRoles;
        numRoles = sizeof(gComponentMapping) / sizeof(ComponentMappingT);
        VidFormat = OMX_VIDEO_CodingUnused;
        for(i = 0; i < numRoles; i++)
        {
            if(!strncmp(cComponentName, gComponentMapping[i].name,
                    MAX_NAME_STRLEN))
            {
                   VidFormat = gComponentMapping[i].format;
                   break;
            }
        }
    }

    {
      char property_value[PROPERTY_VALUE_MAX] = {0};
      property_get("vidc.dec.debug.logs", property_value, "0");
      debugLogsEnable = atoi(property_value);
    }

    char buffer[10];
    int soc_id;
    FILE *device = NULL;
    int result;

    device = fopen("/sys/devices/system/soc/soc0/id", "r");
    if(device)
    {
        result = fread(buffer, 1, 4, device);
        fclose(device);
    }

    soc_id = atoi(buffer);
    ITTIAM_LOG("SOC ID : %d\n", soc_id);

    switch(soc_id)
    {
        case 43:
        case 44:
        case 61:
        case 67:
        case 68:
        case 69:
        case 90:
        case 91:
        case 92:
        case 97:
        case 101:
        case 102:
        case 103:
        case 136:
            ITTIAM_LOG("Initializing for QCOM_7X27A");
            processorType = QCOM_7X27A;
            break;

        case 127:
        case 128:
        case 129:
        case 137:
        case 167:
            ITTIAM_LOG("Initializing for QCOM_8X25");
            processorType = QCOM_8X25;
            break;

        case 168:
        case 169:
        case 170:
            ITTIAM_LOG("Initializing for QCOM_8X25Q");
            processorType = QCOM_8X25Q;
            break;

        case 147:
        case 161:
        case 162:
        case 163:
        case 164:
        case 165:
        case 166:
            /* Once SOC Ids are different for 8x10 and 8x12, following to be cleaned up */
            {

                FILE *quad = fopen("/sys/devices/system/cpu/cpu2/online", "r");
                if(quad)
                {
                    ITTIAM_LOG("Initializing for QCOM_8X12");
                    processorType = QCOM_8X12;
                    fclose(quad);
                }
                else
                {
                    ITTIAM_LOG("Initializing for QCOM_8X10");
                    processorType = QCOM_8X10;
                }
            }
            break;

        case 87:
        case 122:
        case 123:
        case 124:
        case 138:
        case 139:
        case 140:
        case 141:
            ITTIAM_LOG("Initializing for QCOM_8960");
            processorType = QCOM_8960;
            break;

        case 109:
        case 153:
        case 172:
            ITTIAM_LOG("Initializing for QCOM_APQ8064");
            processorType = QCOM_APQ8064;
            break;

        case 130:
            ITTIAM_LOG("Initializing for QCOM_MPQ8064");
            processorType = QCOM_MPQ8064;
            break;

        case 126:
        case 184:
            ITTIAM_LOG("Initializing for QCOM_8974");
            processorType = QCOM_8974;
            break;

        default:
            ITTIAM_LOG("Initializing for QCOM_GENERIC");
            processorType = QCOM_GENERIC;
            break;
    }

    colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
    maxWidth = MAX_WIDTH;
    maxHeight = MAX_HEIGHT;
        disableInterlaced = 1;

#if defined(_SC_NPROCESSORS_ONLN)
    numCores = sysconf(_SC_NPROCESSORS_ONLN);
#else
    // _SC_NPROC_ONLN must be defined...
    numCores = sysconf(_SC_NPROC_ONLN);
#endif



    eRet = ComponentInit((OMX_HANDLETYPE)hComponent,
                         VidFormat,
                         colorFormat,
                         maxWidth,
                         maxHeight,
                         shareDispBuf,
                         numCores,
                         disableInterlaced,
                         minUndequeuedBufs,
                         processorType,
                         0);

    return eRet;
}

/* ======================================================================
 FUNCTION
 omx_vdec::GetComponentVersion

 DESCRIPTION
 Returns the component version.

 PARAMETERS
 TBD.

 RETURN VALUE
 OMX_ErrorNone.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::get_component_version
(
                OMX_IN OMX_HANDLETYPE hComp,
                OMX_OUT OMX_STRING componentName,
                OMX_OUT OMX_VERSIONTYPE* componentVersion,
                OMX_OUT OMX_VERSIONTYPE* specVersion,
                OMX_OUT OMX_UUIDTYPE* componentUUID
)
{
    return hComponent->GetComponentVersion(hComponent, componentName, componentVersion,
                    specVersion, componentUUID);
}
/* ======================================================================
 FUNCTION
 omx_vdec::SendCommand

 DESCRIPTION
 Returns zero if all the buffers released..

 PARAMETERS
 None.

 RETURN VALUE
 true/false

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::send_command(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_COMMANDTYPE cmd,
                OMX_IN OMX_U32 param1,
                OMX_IN OMX_PTR cmdData
)
{
    return hComponent->SendCommand(hComponent, cmd, param1, cmdData);
}

/* ======================================================================
 FUNCTION
 omx_vdec::GetParameter

 DESCRIPTION
 OMX Get Parameter method implementation

 PARAMETERS
 <TBD>.

 RETURN VALUE
 Error None if successful.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::get_parameter(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_INDEXTYPE paramIndex,
                OMX_INOUT OMX_PTR paramData)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    VIDDECTYPE *pVidDec;
    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    ITTIAM_DEBUG("omx_vdec::get_parameter paramIndex = %x",paramIndex);

    if(paramData == NULL)
    {
        ITTIAM_DEBUG("Get Param in Invalid paramData \n");
        return OMX_ErrorBadParameter;
    }
    switch((WORD32)paramIndex)
    {

        case IOMX_GoogleAndroidIndexUseAndroidNativeBuffer2:
        case IOMX_GoogleAndroidIndexGetAndroidNativeBufferUsage:
        {
            ITTIAM_DEBUG("get_parameter: OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage");
            GetAndroidNativeBufferUsageParams* nativeBuffersUsage = (GetAndroidNativeBufferUsageParams *) paramData;
            if(nativeBuffersUsage->nPortIndex == pVidDec->sOutPortDef.nPortIndex )
            {
                WORD32 flags = GRALLOC_USAGE_SW_WRITE_OFTEN;

                if(pVidDec->shareDispBuf)
                    flags |= GRALLOC_USAGE_SW_READ_OFTEN;

                nativeBuffersUsage->nUsage = flags;
            }
            else
            {
                ITTIAM_DEBUG("get_parameter: OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage failed!");
                eRet = OMX_ErrorBadParameter;
            }
        }
        break;
#ifdef ENABLE_DESCRIBE_COLOR_FORMAT
        case IOMX_GoogleAndroidIndexDescribeColorFormat:
        {
            DescribeColorFormatParams *params = (DescribeColorFormatParams *)paramData;
            MediaImage *image = &params->sMediaImage;
            memset(image, 0, sizeof(MediaImage));
            image->mType = MediaImage::MEDIA_IMAGE_TYPE_UNKNOWN;
            image->mNumPlanes = 0;
            image->mWidth = params->nFrameWidth;
            image->mHeight = params->nFrameHeight;

            switch((WORD32)params->eColorFormat)
            {
                /* YUV/YVU 420 Planar */
                case IOMX_COLOR_FormatYVU420Planar:
                case OMX_COLOR_FormatYUV420Planar:
                    image->mPlane[image->U].mOffset = params->nStride * params->nSliceHeight;
                    image->mPlane[image->U].mColInc = 1;
                    image->mPlane[image->U].mRowInc = params->nStride / 2;
                    image->mPlane[image->U].mHorizSubsampling = 2;
                    image->mPlane[image->U].mVertSubsampling = 2;

                    image->mPlane[image->V].mOffset = image->mPlane[image->U].mOffset
                            + (params->nStride * params->nSliceHeight / 4);
                    image->mPlane[image->V].mColInc = 1;
                    image->mPlane[image->V].mRowInc = params->nStride / 2;
                    image->mPlane[image->V].mHorizSubsampling = 2;
                    image->mPlane[image->V].mVertSubsampling = 2;
                    break;

                /* YUV 420 Semi Planar */
                case OMX_TI_COLOR_FormatYUV420PackedSemiPlanar:
                case OMX_COLOR_FormatYUV420SemiPlanar:
                case OMX_COLOR_FormatYUV420Flexible:
                    image->mPlane[image->U].mOffset = params->nStride * params->nSliceHeight;
                    image->mPlane[image->U].mColInc = 2;
                    image->mPlane[image->U].mRowInc = params->nStride;
                    image->mPlane[image->U].mHorizSubsampling = 2;
                    image->mPlane[image->U].mVertSubsampling = 2;

                    image->mPlane[image->V].mOffset = params->nStride * params->nSliceHeight + 1;
                    image->mPlane[image->V].mColInc = 2;
                    image->mPlane[image->V].mRowInc = params->nStride;
                    image->mPlane[image->V].mHorizSubsampling = 2;
                    image->mPlane[image->V].mVertSubsampling = 2;


                    break;
                case IOMX_COLOR_FormatYVU420SemiPlanar:
                case OMX_QCOM_COLOR_FormatYVU420SemiPlanar:
                case IOMXQ_COLOR_FormatYUV420PackedSemiPlanar32m:
                    image->mPlane[image->U].mOffset = params->nStride * params->nSliceHeight + 1;
                    image->mPlane[image->U].mColInc = 2;
                    image->mPlane[image->U].mRowInc = params->nStride;
                    image->mPlane[image->U].mHorizSubsampling = 2;
                    image->mPlane[image->U].mVertSubsampling = 2;

                    image->mPlane[image->V].mOffset = params->nStride * params->nSliceHeight;
                    image->mPlane[image->V].mColInc = 2;
                    image->mPlane[image->V].mRowInc = params->nStride;
                    image->mPlane[image->V].mHorizSubsampling = 2;
                    image->mPlane[image->V].mVertSubsampling = 2;
                    break;

                default:
                    ITTIAM_ERROR("Unknown Color Format %8x", pVidDec->sOutPortDef.format.video.eColorFormat);
                    return OMX_ErrorUndefined;
            }

            image->mType = MediaImage::MEDIA_IMAGE_TYPE_YUV;
            image->mNumPlanes = 3;
            image->mBitDepth = 8;
            image->mPlane[image->Y].mOffset = 0;
            image->mPlane[image->Y].mColInc = 1;
            image->mPlane[image->Y].mRowInc = params->nStride;
            image->mPlane[image->Y].mHorizSubsampling = 1;
            image->mPlane[image->Y].mVertSubsampling = 1;
            eRet = OMX_ErrorNone;
            break;
        }
#endif

        default:
        {
            eRet = hComponent->GetParameter(hComponent, paramIndex, paramData);
        }

    }
    return eRet;
}

/* ======================================================================
 FUNCTION
 omx_vdec::Setparameter

 DESCRIPTION
 OMX Set Parameter method implementation.

 PARAMETERS
 <TBD>.

 RETURN VALUE
 OMX Error None if successful.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::set_parameter(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_INDEXTYPE paramIndex,
                OMX_IN OMX_PTR paramData)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    VIDDECTYPE *pVidDec;
    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    ITTIAM_DEBUG("omx_vdec::set_parameter paramIndex = %x",paramIndex);

    if(paramData == NULL)
    {
        ITTIAM_DEBUG("Get Param in Invalid paramData \n");
        return OMX_ErrorBadParameter;
    }
    switch((WORD32)paramIndex)
    {
        /* Need to allow following two set_parameters even in Idle
         * state. This is ANDROID architecture which is not in sync
         * with openmax standard. */
        case IOMX_GoogleAndroidIndexUseAndroidNativeBuffer:
        {
            eRet = use_android_native_buffer(hComp, paramData);
        }
        break;
        case IOMX_GoogleAndroidIndexEnableAndroidNativeBuffer:
        {
            EnableAndroidNativeBuffersParams *params = (EnableAndroidNativeBuffersParams *)paramData;
            if(params->nPortIndex == pVidDec->sOutPortDef.nPortIndex )
            {
                if(OMX_FALSE == params->enable)
                {
                    ITTIAM_LOG("Disabling native buffer usage");
                    pVidDec->bufferType = BUFFER_TYPE_LOCAL;
                }
            }
            else
            {
                ITTIAM_DEBUG("set_parameter: OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage failed!");
                eRet = OMX_ErrorBadParameter;
            }
            break;
        }
#ifdef ENABLE_ADAPTIVE_PLAYBACK
        case IOMX_GoogleAndroidIndexEnableAdaptivePlayback:
        if (((PrepareForAdaptivePlaybackParams *)(paramData))->bEnable)
        {
            pVidDec->initWidth     = ((PrepareForAdaptivePlaybackParams *)(paramData))->nMaxFrameWidth;
            pVidDec->initHeight    = ((PrepareForAdaptivePlaybackParams *)(paramData))->nMaxFrameHeight;
        }
        eRet = OMX_ErrorNone;
        break;
#endif
        default:
        {
            eRet = hComponent->SetParameter(hComponent, paramIndex, paramData);;
        }
    }
    return eRet;
}

/* ======================================================================
 FUNCTION
 omx_vdec::GetConfig

 DESCRIPTION
 OMX Get Config Method implementation.

 PARAMETERS
 <TBD>.

 RETURN VALUE
 OMX Error None if successful.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::get_config(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_INDEXTYPE configIndex,
                OMX_INOUT OMX_PTR configData)

{
    return hComponent->GetConfig(hComponent, configIndex, configData);
}

/* ======================================================================
 FUNCTION
 omx_vdec::SetConfig

 DESCRIPTION
 OMX Set Config method implementation

 PARAMETERS
 <TBD>.

 RETURN VALUE
 OMX Error None if successful.
 ========================================================================== */
OMX_ERRORTYPE omx_vdec::set_config(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_INDEXTYPE configIndex,
                OMX_IN OMX_PTR configData)
{
    return hComponent->SetConfig(hComponent, configIndex, configData);
}

/* ======================================================================
 FUNCTION
 omx_vdec::GetExtensionIndex

 DESCRIPTION
 OMX GetExtensionIndex method implementaion.  <TBD>

 PARAMETERS
 <TBD>.

 RETURN VALUE
 OMX Error None if everything successful.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::get_extension_index(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_STRING paramName,
                OMX_OUT OMX_INDEXTYPE* indexType)
{
    return hComponent->GetExtensionIndex(hComponent, paramName, indexType);
}

/* ======================================================================
 FUNCTION
 omx_vdec::GetState

 DESCRIPTION
 Returns the state information back to the caller.<TBD>

 PARAMETERS
 <TBD>.

 RETURN VALUE
 Error None if everything is successful.
 ========================================================================== */
OMX_ERRORTYPE omx_vdec::get_state(OMX_IN OMX_HANDLETYPE hComp,
                OMX_OUT OMX_STATETYPE* state)
{
    return hComponent->GetState(hComponent, state);
}

/* ======================================================================
 FUNCTION
 omx_vdec::ComponentTunnelRequest

 DESCRIPTION
 OMX Component Tunnel Request method implementation. <TBD>

 PARAMETERS
 None.

 RETURN VALUE
 OMX Error None if everything successful.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::component_tunnel_request(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_U32 port,
                OMX_IN OMX_HANDLETYPE peerComponent,
                OMX_IN OMX_U32 peerPort,
                OMX_INOUT OMX_TUNNELSETUPTYPE* tunnelSetup)
{
    return hComponent->ComponentTunnelRequest(hComponent, port, peerComponent, peerPort, tunnelSetup);
}

/* ======================================================================
 FUNCTION
 omx_vdec::UseBuffer

 DESCRIPTION
 OMX Use Buffer method implementation.

 PARAMETERS
 <TBD>.

 RETURN VALUE
 OMX Error None , if everything successful.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::use_buffer(
                OMX_IN OMX_HANDLETYPE hComp,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes,
                OMX_IN OMX_U8* pBuffer)

{

    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    VIDDECTYPE *pVidDec;
    OMX_BUFFERHEADERTYPE *pBufferHdr;
    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    *ppBufferHdr = NULL;
    eRet = UseBuffer(hComponent,ppBufferHdr,nPortIndex,pAppPrivate,nSizeBytes,pBuffer);
    pBufferHdr = *ppBufferHdr;

    if(pBufferHdr && (nPortIndex == pVidDec->sOutPortDef.nPortIndex) && (BUFFER_TYPE_NATIVEBUF2 == pVidDec->bufferType))
    {
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        Rect bounds(pVidDec->sOutPortDef.format.video.nFrameWidth, pVidDec->sOutPortDef.format.video.nFrameHeight);
        WORD32 flags = GRALLOC_USAGE_SW_WRITE_OFTEN;

        if(pVidDec->shareDispBuf)
            flags |= GRALLOC_USAGE_SW_READ_OFTEN;
        //Get the virtual address and unlock. Buffer will be locked again in fillThisBuffer call
        mapper.lock((buffer_handle_t)pBufferHdr->pBuffer, flags, bounds, &pBufferHdr->pOutputPortPrivate);

        /* If nFrameWidth is not same as nDispWidth (because of align32), memset the remaining bytes to zero.
           This is needed since some of the CTS tests do checksum calculation without taking crop into consideration */
        if(pVidDec->sOutPortDef.format.video.nFrameWidth > pVidDec->nDispWidth)
        {
            ittiam_video_pad_output_buffer(pVidDec, (UWORD8 *)pBufferHdr->pOutputPortPrivate);
        }
        mapper.unlock((buffer_handle_t)pBufferHdr->pBuffer);
    }
    else if(pBufferHdr &&  (nPortIndex == pVidDec->sOutPortDef.nPortIndex))
    {
        pBufferHdr->pOutputPortPrivate = pBufferHdr->pBuffer;
    }
    return eRet;
}

// AllocateBuffer  -- API Call
/* ======================================================================
 FUNCTION
 omx_vdec::AllocateBuffer

 DESCRIPTION
 Returns zero if all the buffers released..

 PARAMETERS
 None.

 RETURN VALUE
 true/false

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::allocate_buffer(OMX_IN OMX_HANDLETYPE hComp,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 port,
                OMX_IN OMX_PTR appData,
                OMX_IN OMX_U32 bytes)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    VIDDECTYPE *pVidDec;
    OMX_BUFFERHEADERTYPE *pBufferHdr;
    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    ITTIAM_DEBUG("omx_vdec::allocate_buffer");
	*ppBufferHdr = NULL;
     eRet = hComponent->AllocateBuffer(hComponent, ppBufferHdr, port, appData, bytes);
         pBufferHdr = *ppBufferHdr;
     if(pBufferHdr && (port == pVidDec->sOutPortDef.nPortIndex))
     {

         pBufferHdr->pOutputPortPrivate = pBufferHdr->pBuffer;
         if(pVidDec->sOutPortDef.format.video.nFrameWidth > pVidDec->nDispWidth)
         {
             ittiam_video_pad_output_buffer(pVidDec, (UWORD8 *)pBufferHdr->pOutputPortPrivate);
         }
     }

     return eRet;

}

// Free Buffer - API call
/* ======================================================================
 FUNCTION
 omx_vdec::FreeBuffer

 DESCRIPTION

 PARAMETERS
 None.

 RETURN VALUE
 true/false

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::free_buffer(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_U32 port,
                OMX_IN OMX_BUFFERHEADERTYPE* buffer)

{
    return hComponent->FreeBuffer(hComponent, port, buffer);
}

/* ======================================================================
 FUNCTION
 omx_vdec::EmptyThisBuffer

 DESCRIPTION
 This routine is used to push the encoded video frames to
 the video decoder.

 PARAMETERS
 None.

 RETURN VALUE
 OMX Error None if everything went successful.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::empty_this_buffer(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
    return hComponent->EmptyThisBuffer(hComponent, buffer);
}

/* ======================================================================
 FUNCTION
 omx_vdec::FillThisBuffer

 DESCRIPTION
 IL client uses this method to release the frame buffer
 after displaying them.

 PARAMETERS
 None.

 RETURN VALUE
 true/false

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::fill_this_buffer(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
   VIDDECTYPE *pVidDec;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    if(BUFFER_TYPE_NATIVEBUF2 == pVidDec->bufferType)
    {
        WORD32 flags;
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        Rect bounds(pVidDec->sOutPortDef.format.video.nFrameWidth, pVidDec->sOutPortDef.format.video.nFrameHeight);
        flags = GRALLOC_USAGE_SW_WRITE_OFTEN;

        if(pVidDec->shareDispBuf)
            flags |= GRALLOC_USAGE_SW_READ_OFTEN;

        mapper.lock((buffer_handle_t)pBufferHdr->pBuffer, flags, bounds, &pBufferHdr->pOutputPortPrivate);
    }
    else
    {
        pBufferHdr->pOutputPortPrivate = pBufferHdr->pBuffer;
    }
    return hComponent->FillThisBuffer(hComponent, pBufferHdr);
}

/* ======================================================================
 FUNCTION
 omx_vdec::SetCallbacks

 DESCRIPTION
 Set the callbacks.

 PARAMETERS
 None.

 RETURN VALUE
 OMX Error None if everything successful.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::set_callbacks(OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_CALLBACKTYPE* callbacks,
                OMX_IN OMX_PTR appData)
{

    return hComponent->SetCallbacks(hComponent, callbacks, appData);

}

/* ======================================================================
 FUNCTION
 omx_vdec::ComponentDeInit

 DESCRIPTION
 Destroys the component and release memory allocated to the heap.

 PARAMETERS
 <TBD>.

 RETURN VALUE
 OMX Error None if everything successful.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::component_deinit(OMX_IN OMX_HANDLETYPE hComp)
{

    int nbufSize, ret;
    VIDDECTYPE *pVidDec;
    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    nbufSize = pVidDec->sOutPortDef.nBufferSize;

    return hComponent->ComponentDeInit(hComponent);

}

/* ======================================================================
 FUNCTION
 omx_vdec::UseEGLImage

 DESCRIPTION
 OMX Use EGL Image method implementation <TBD>.

 PARAMETERS
 <TBD>.

 RETURN VALUE
 Not Implemented error.

 ========================================================================== */
OMX_ERRORTYPE omx_vdec::use_EGL_image(OMX_IN OMX_HANDLETYPE hComp,
                OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
                OMX_IN OMX_U32 port,
                OMX_IN OMX_PTR appData,
                OMX_IN void* eglImage)
{

    return hComponent->UseEGLImage(hComponent, bufferHdr, port, appData, eglImage);

}

/* ======================================================================
 FUNCTION
 omx_vdec::ComponentRoleEnum

 DESCRIPTION
 OMX Component Role Enum method implementation.

 PARAMETERS
 <TBD>.

 RETURN VALUE
 OMX Error None if everything is successful.
 ========================================================================== */
OMX_ERRORTYPE omx_vdec::component_role_enum(OMX_IN OMX_HANDLETYPE hComp,
                OMX_OUT OMX_U8* role,
                OMX_IN OMX_U32 index)
{
    return hComponent->ComponentRoleEnum(hComponent, role, index);
}

OMX_ERRORTYPE FillBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    VIDDECTYPE *pVidDec;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if(BUFFER_TYPE_NATIVEBUF2 == pVidDec->bufferType)
    {
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        mapper.unlock((buffer_handle_t)pBufferHdr->pBuffer);
    }
    pBufferHdr->nFlags = pBufferHdr->nFlags & (~CUSTOM_BUFFERFLAG_OWNED);

    /* If current timstamps is same as previous buffers timestamp,                   */
    /* then ignore the current bufer. This is to handle packed B frames in avi files */
    /* Input: ... PB (Timestamp1) NotCoded (Timestamp2) ..                           */
    /* Output: ... B(Timestamp1) P(Timestamp1) NotCoded(Timstamp2) ...               */
    /* In the above output NotCoded picture is same as P, hence P can be ignored     */
    /* Output After ignoring P                                                       */
    /*        ... B(Timestamp1) NotCoded(Timstamp2) ...                              */

    if(pVidDec->prevTimeStamp == pBufferHdr->nTimeStamp)
    {
        pBufferHdr->nFilledLen = 0;
    }
    if(pBufferHdr->nFilledLen)
        pVidDec->prevTimeStamp = pBufferHdr->nTimeStamp;
    else
        pBufferHdr->nTimeStamp = 0;

    ITTIAM_DEBUG("Calling FillBufferDone bufferhdr %p length %d timestamp %lld",
                 pBufferHdr, (WORD32)pBufferHdr->nFilledLen, pBufferHdr->nTimeStamp);

    return pVidDec->pCallbacks->FillBufferDone(hComponent, pAppData, pBufferHdr);
    }


OMX_ERRORTYPE omx_vdec::use_android_native_buffer(OMX_IN OMX_HANDLETYPE hComp, OMX_PTR data)
{
    VIDDECTYPE *pVidDec;
    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    OMX_U32 frameSize;
    OMX_U32 bufWidth, bufHeight;
    OMX_U8 *buffer;

    UseAndroidNativeBufferParams *params = (UseAndroidNativeBufferParams *)data;

    if((params == NULL) ||
                    (params->nativeBuffer == NULL) ||
                    (params->nativeBuffer->handle == NULL))
    return OMX_ErrorBadParameter;

    sp<android_native_buffer_t> nBuf = params->nativeBuffer;

    if(pVidDec->sOutPortDef.nPortIndex == params->nPortIndex)
    {
        GraphicBuffer* tempbuf = (GraphicBuffer *)params->nativeBuffer.get();
        WORD32 flags;
        bufWidth = ALIGN16(tempbuf->width);
        bufHeight = ALIGN16(tempbuf->height);

        frameSize = (bufWidth * bufHeight * 3) / 2;
        frameSize = ALIGN4096(frameSize);
        flags = GRALLOC_USAGE_SW_WRITE_OFTEN;

        if(pVidDec->shareDispBuf)
            flags |= GRALLOC_USAGE_SW_READ_OFTEN;

        tempbuf->lock(flags, (void**)(&buffer));
        eRet = use_buffer(hComp,params->bufferHeader,params->nPortIndex,data,frameSize,(OMX_U8 *)tempbuf);
        tempbuf->unlock();

    }
    else
    {
        eRet = OMX_ErrorBadParameter;
    }
    return eRet;
}



