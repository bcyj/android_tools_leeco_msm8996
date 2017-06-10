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

 Copyright (c) 2006-2007, 2011-2014 Qualcomm Technologies Incorporated.
 All Rights Reserved. Qualcomm Proprietary and Confidential.

 *****************************************************************************/

/*============================================================================
 O p e n M A X   w r a p p e r s
 O p e n  M A X   C o r e

 *//** @file omx_vdec.cpp
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
#include <OMX_QCOMExtns.h>

#include "omx_vdec.h"

#include "datatypedef.h"
#include "iv.h"
#include "ivd.h"
#include <OMX_Ittiam.h>
#include <OMX_Ittiam_Vdec.h>

#include <HardwareAPI.h>
#ifdef JELLY_BEAN
#include <gui/BufferQueue.h>
#endif
#include <gralloc_priv.h>
#include <cutils/properties.h>

//#define PMEM_DEVICE "/dev/pmem_smipool" // For MSM8660 device
#define PMEM_DEVICE "/dev/pmem_adsp" // For 7x27A device
#define PMEM_IOCTL_MAGIC 'p'

#define PMEM_ALLOCATE_ALIGNED   _IOW(PMEM_IOCTL_MAGIC, 15, unsigned int)

#define MAX_WIDTH   1920
#define MAX_HEIGHT  1088
struct pmem_allocation
{
    unsigned long size;
    unsigned int align;
};

VideoHeap::VideoHeap(int fd, size_t size, void* base)
{
    // dup file descriptor, map once, use pmem
    init(dup(fd), base, size, 0, PMEM_DEVICE);
}

// factory function executed by the core to create instances
void *get_omx_component_factory_fn(void)
{
    return (new omx_vdec);
}

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
                hComponent(NULL),
                use_hardware_renderer(OMX_TRUE),
                num_output_buff_allocated(0),
                pPlatformList(NULL),
                pmem_fd(0),
                pmem_baseaddress(NULL),
                m_heap_ptr(NULL)
{
    hComponent = (OMX_COMPONENTTYPE *)malloc(sizeof(OMX_COMPONENTTYPE));
    m_enable_android_native_buffers = 0;
    m_enable_android_native_buffers2 = true;
    mTarget8960 = 0;

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
        free (hComponent);
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
OMX_ERRORTYPE omx_vdec::component_init(OMX_STRING role)
{

    char mDeviceName[128];
    char mDeviceHwPlat[128];
    OMX_U32 disable_interlaced = 0;
    OMX_U32 low_complexity_level_max = 0;
    OMX_U32 share_disp_buf = 0;
    OMX_U32 num_cores = 1;
    OMX_U32 max_width = MAX_WIDTH;
    OMX_U32 max_height = MAX_HEIGHT;
    OMX_U32 minUndequeuedBufs = 0;
    OMX_U32 IsQcomCore = 1;
    PROCESSORTYPE processor_type;
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    OMX_IN
    OMX_VIDEO_CODINGTYPE VidFormat;
    OMX_COLOR_FORMATTYPE colorFormat =
                    (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar;

    ITTIAM_LOG("In %s function, role = %s", __FUNCTION__, role);
#ifdef JELLY_BEAN
    minUndequeuedBufs = BufferQueue::MIN_UNDEQUEUED_BUFFERS;
#else
#ifdef ICS
    minUndequeuedBufs = 2;
#endif
#endif

    if(!strncmp(role, "OMX.ittiam.video.decoder.mpeg4",
                OMX_MAX_STRINGNAME_SIZE))
    {
        VidFormat = OMX_VIDEO_CodingMPEG4;
    }
    else if(!strncmp(role, "OMX.ittiam.video.decoder.avc",
                     OMX_MAX_STRINGNAME_SIZE))
    {
        VidFormat = OMX_VIDEO_CodingAVC;
    }
    else if(!strncmp(role, "OMX.ittiam.video.decoder.h263",
                     OMX_MAX_STRINGNAME_SIZE))
    {
        VidFormat = OMX_VIDEO_CodingH263;
    }
    else if(!strncmp(role, "OMX.ittiam.video.decoder.divx4",
                     OMX_MAX_STRINGNAME_SIZE))
    {
        VidFormat = (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingDivx;
    }
    else if(!strncmp(role, "OMX.ittiam.video.decoder.divx",
                     OMX_MAX_STRINGNAME_SIZE))
    {
        VidFormat = (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingDivx;
    }
    else if(!strncmp(role, "OMX.ittiam.video.decoder.mpeg2",
                     OMX_MAX_STRINGNAME_SIZE))
    {
        VidFormat = OMX_VIDEO_CodingMPEG2;
    }
    else if(!strncmp(role, "OMX.ittiam.video.decoder.hevc",
                     OMX_MAX_STRINGNAME_SIZE))
    {
        ITTIAM_LOG("VidFormat is hevc");
        //VidFormat = (int)OMX_VIDEO_CodingHEVC;
    }
    else
    {
        eRet = OMX_ErrorInvalidComponentName;

        ITTIAM_ERROR("No matching decoder found");
        return eRet;

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
            processor_type = QCOM_7X27A;
            break;

        case 127:
        case 128:
        case 129:
        case 137:
        case 167:
            ITTIAM_LOG("Initializing for QCOM_8X25");
            processor_type = QCOM_8X25;
            break;

        case 168:
        case 169:
        case 170:
            ITTIAM_LOG("Initializing for QCOM_8X25Q");
            processor_type = QCOM_8X25Q;
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
                    processor_type = QCOM_8X12;
                    fclose(quad);
                }
                else
                {
                    ITTIAM_LOG("Initializing for QCOM_8X10");
                    processor_type = QCOM_8X10;
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
            processor_type = QCOM_8960;
            break;

        case 109:
        case 153:
        case 172:
            ITTIAM_LOG("Initializing for QCOM_APQ8064");
            processor_type = QCOM_APQ8064;
            break;

        case 130:
            ITTIAM_LOG("Initializing for QCOM_MPQ8064");
            processor_type = QCOM_MPQ8064;
            break;

        case 126:
        case 184:
            ITTIAM_LOG("Initializing for QCOM_8974");
            processor_type = QCOM_8974;
            break;

        default:
            ITTIAM_LOG("Initializing for QCOM_GENERIC");
            processor_type = QCOM_GENERIC;
            break;
    }
    if((QCOM_8X25 == processor_type) || (QCOM_8X25Q == processor_type))
    {

        colorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar;
        low_complexity_level_max = 0;
        max_width = MAX_WIDTH;
        max_height = MAX_HEIGHT;
        disable_interlaced = 1;

        share_disp_buf = 0;

        if(QCOM_8X25Q == processor_type)
        {
            num_cores = 4;
        }
        else
        {
            num_cores = 2;
        }
        m_enable_android_native_buffers2 = false;

    }
    else if((QCOM_8X10 == processor_type) || (QCOM_8X12 == processor_type))
    {
            colorFormat = (OMX_COLOR_FORMATTYPE)CUSTOM_COLOR_FormatYUV420SemiPlanar;
//        colorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar;
        low_complexity_level_max = 0;
        max_width = MAX_WIDTH;
        max_height = MAX_HEIGHT;
        disable_interlaced = 0;
        mTarget8960 = 1;
        share_disp_buf = 0;

        if(QCOM_8X12 == processor_type)
        {
            num_cores = 4;
        }
        else
        {
            num_cores = 2;
        }
        m_enable_android_native_buffers2 = false;

    }
    else if ((QCOM_8974 == processor_type) ||
                (QCOM_APQ8064 == processor_type) ||
                (QCOM_MPQ8064 == processor_type) ||
                (QCOM_8960 == processor_type))
    {

        share_disp_buf = 0;
        num_cores = 4;
        colorFormat = (OMX_COLOR_FORMATTYPE)CUSTOM_COLOR_FormatYUV420SemiPlanar;
//        colorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar;
        low_complexity_level_max = 0;
        max_width = MAX_WIDTH;
        max_height = MAX_HEIGHT;
        disable_interlaced = 0;
        mTarget8960 = 1;
        m_enable_android_native_buffers2 = false;
    }
    else if (QCOM_7X27A == processor_type)
    {

        share_disp_buf = SHARE_DISP_BUF;
        num_cores = 1;
        colorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar;
#ifdef ICS
        low_complexity_level_max = 2;
#else
        low_complexity_level_max = 3;
#endif
        max_width = 1280;
        max_height = 720;

        disable_interlaced = 1;
        m_enable_android_native_buffers2 = false;
    }
    else
    {
        share_disp_buf = 0;
        num_cores = 4;
        colorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar;
        low_complexity_level_max = 0;
        max_width = MAX_WIDTH;
        max_height = MAX_HEIGHT;
        disable_interlaced = 1;
        m_enable_android_native_buffers2 = false;
    }

ALOGE("Calling component Init with Color Format as %d",colorFormat);
ALOGE("Calling component Init with num_cores as %d",num_cores);

    eRet = ComponentInit((OMX_HANDLETYPE)hComponent,
                         VidFormat,
                         colorFormat,
                         max_width,
                         max_height,
                         share_disp_buf,
                         num_cores,
                         low_complexity_level_max,
                         disable_interlaced,
                         minUndequeuedBufs,
                         IsQcomCore,
                         processor_type);

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
    char mDeviceName[128];
    VIDDECTYPE *pVidDec;
    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    property_get("ro.product.device",mDeviceName,"0");
    ITTIAM_DEBUG("omx_vdec::get_parameter paramIndex = %x",paramIndex);

    if(paramData == NULL)
    {
        ITTIAM_DEBUG("Get Param in Invalid paramData \n");
        return OMX_ErrorBadParameter;
    }
    switch((WORD32)paramIndex)
    {
#ifdef ICS
        case OMX_GoogleAndroidIndexUseAndroidNativeBuffer2:
        case OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage:
        {
            ITTIAM_DEBUG("get_parameter: OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage");
            GetAndroidNativeBufferUsageParams* nativeBuffersUsage = (GetAndroidNativeBufferUsageParams *) paramData;
            if(nativeBuffersUsage->nPortIndex == pVidDec->sOutPortDef.nPortIndex )
            {
#ifdef USE_ION
                if(mTarget8960)
                {
                    //nativeBuffersUsage->nUsage = GRALLOC_USAGE_PRIVATE_UNCACHED;
                    //nativeBuffersUsage->nUsage = GRALLOC_USAGE_PRIVATE_MM_HEAP | GRALLOC_USAGE_PRIVATE_IOMMU_HEAP;
                    //nativeBuffersUsage->nUsage = GRALLOC_USAGE_PRIVATE_CAMERA_HEAP | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRIE_OFTEN;
                    nativeBuffersUsage->nUsage = GRALLOC_USAGE_PRIVATE_MM_HEAP | GRALLOC_USAGE_PRIVATE_IOMMU_HEAP | GRALLOC_USAGE_PRIVATE_UNCACHED;
                }
                else
                {
                            nativeBuffersUsage->nUsage = (GRALLOC_USAGE_PRIVATE_ADSP_HEAP);
                }
#else
                nativeBuffersUsage->nUsage = (GRALLOC_USAGE_PRIVATE_ADSP_HEAP);

#endif
            }
            else
            {
                ITTIAM_DEBUG("get_parameter: OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage failed!");
                eRet = OMX_ErrorBadParameter;
            }
            if ( !strncmp(mDeviceName, "mako", 4))
            {
                nativeBuffersUsage->nUsage = 0x4000000;
            }
        }
        break;
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

    ITTIAM_DEBUG("omx_vdec::set_parameter paramIndex = %x",paramIndex);

    if(paramData == NULL)
    {
        ITTIAM_DEBUG("Get Param in Invalid paramData \n");
        return OMX_ErrorBadParameter;
    }
    switch((WORD32)paramIndex)
    {
#ifdef ICS
        /* Need to allow following two set_parameters even in Idle
         * state. This is ANDROID architecture which is not in sync
         * with openmax standard. */
        case OMX_GoogleAndroidIndexEnableAndroidNativeBuffers:
        {

            ITTIAM_DEBUG("OMX_GoogleAndroidIndexEnableAndroidNativeBuffers");
            EnableAndroidNativeBuffersParams* enableNativeBuffers = (EnableAndroidNativeBuffersParams *) paramData;
            m_enable_android_native_buffers = enableNativeBuffers->enable;
            ITTIAM_DEBUG("m_enable_android_native_buffers = %d",m_enable_android_native_buffers);
        }
        break;
        case OMX_GoogleAndroidIndexUseAndroidNativeBuffer:
        {
              ITTIAM_DEBUG("use_android_native_buffer calling");
            eRet = use_android_native_buffer(hComp, paramData);
              ITTIAM_DEBUG("use_android_native_buffer = %d exit",eRet);
        }
        break;
#endif
        default:
        {
            eRet = hComponent->SetParameter(hComponent, paramIndex, paramData);;
        }
    }
    ITTIAM_DEBUG("set Param = %d exit",eRet);
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
#ifdef ICS
    if(!strncmp(paramName,"OMX.google.android.index.enableAndroidNativeBuffers", sizeof("OMX.google.android.index.enableAndroidNativeBuffers") - 1))
    {
        *indexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexEnableAndroidNativeBuffers;
    }
    else if(!strncmp(paramName,"OMX.google.android.index.useAndroidNativeBuffer2", sizeof("OMX.google.android.index.enableAndroidNativeBuffer2") - 1))
    {
        if(false == m_enable_android_native_buffers2)
        {
            ITTIAM_DEBUG("Extension: %s is not supported\n", paramName);
            return OMX_ErrorNotImplemented;
        }
        else
        {
            ITTIAM_DEBUG("Extension: %s is  supported\n", paramName);
            *indexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexUseAndroidNativeBuffer2;
        }

    }
    else if(!strncmp(paramName,"OMX.google.android.index.useAndroidNativeBuffer", sizeof("OMX.google.android.index.enableAndroidNativeBuffer") - 1))
    {
        ITTIAM_DEBUG("Extension: %s is supported\n", paramName);
        *indexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexUseAndroidNativeBuffer;
    }
    else if(!strncmp(paramName,"OMX.google.android.index.getAndroidNativeBufferUsage", sizeof("OMX.google.android.index.getAndroidNativeBufferUsage") - 1))
    {
        *indexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage;
    }
    else
#endif
    {
        return hComponent->GetExtensionIndex(hComponent, paramName, indexType);
    }
    return OMX_ErrorNone;

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
                OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
                OMX_IN OMX_U32 port,
                OMX_IN OMX_PTR appData,
                OMX_IN OMX_U32 bytes,
                OMX_IN OMX_U8* buffer)

{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    private_handle_t *handle = NULL;
    OMX_U8 *buff = buffer;
#ifdef ICS

    if((num_output_buff_allocated == 0) && (port == 0x1 /* OUTPUT PORT */))
    {
        allocate_platform_private();
        if((pPlatformList==NULL)||(pPMEMInfo==NULL)||(pPlatformEntry==NULL)) {
            ITTIAM_DEBUG("Unable to allocate memory for platform specific info");
            return OMX_ErrorInsufficientResources;
        }
    }
#endif
    if((port == 0x1 /* OUTPUT PORT */) && (true == m_enable_android_native_buffers2))
    {
        handle = (private_handle_t *)buff;

        buff = (OMX_U8*)mmap(0, handle->size,
                        PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0);

        if (buff == MAP_FAILED)
        {
            ITTIAM_ERROR("mmap failed");
            return OMX_ErrorInsufficientResources;
        }
        eRet = hComponent->UseBuffer(hComponent, bufferHdr, port, appData, bytes, buff);
    }
    else
    {
        eRet = hComponent->UseBuffer(hComponent, bufferHdr, port, appData, bytes, buffer);
    }

    if((m_enable_android_native_buffers) && (port == 0x1 /* OUTPUT PORT */) && (false == m_enable_android_native_buffers2))
    {
        VIDDECTYPE *pVidDec;
        int i = num_output_buff_allocated;
        pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

        UseAndroidNativeBufferParams *params = (UseAndroidNativeBufferParams *)appData;
        sp<android_native_buffer_t> nBuf = params->nativeBuffer;
        private_handle_t *handle = (private_handle_t *)nBuf->handle;
        if(!handle)
        {
            ITTIAM_DEBUG("Native Buffer handle is NULL");
            return OMX_ErrorBadParameter;
        }

        pPMEMInfo[i].pmem_fd = handle->fd;
        pPMEMInfo[i].offset = pVidDec->offset;

        pPlatformList[i].nEntries = 1;
        pPlatformList[i].entryList = &pPlatformEntry[i];

        pPlatformEntry[i].type = OMX_QCOM_PLATFORM_PRIVATE_PMEM;
        pPlatformEntry[i].entry = &pPMEMInfo[i];

        (*bufferHdr)->pAppPrivate = params->pAppPrivate;
        (*bufferHdr)->pPlatformPrivate = &pPlatformList[i];
        num_output_buff_allocated++;
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
                OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
                OMX_IN OMX_U32 port,
                OMX_IN OMX_PTR appData,
                OMX_IN OMX_U32 bytes)
{
    ITTIAM_DEBUG("omx_vdec::allocate_buffer");
    return hComponent->AllocateBuffer(hComponent, bufferHdr, port, appData, bytes);

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
    int ret;
    if(port == 0x1 /* OUTPUT PORT */)
    {
        int nbufSize;
        VIDDECTYPE *pVidDec;
        pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
        nbufSize = pVidDec->sOutPortDef.nBufferSize;
        num_output_buff_allocated--;

#ifdef ICS
        if(m_enable_android_native_buffers)
        {
            ret = munmap (buffer->pBuffer, nbufSize);
            if(ret)
            {
                ITTIAM_ERROR("munmap failed");
            }
            if(num_output_buff_allocated == 0)
            {
                free(pPlatformList);
                pPlatformList = NULL;
            }
        }
        else
#endif
        {
            if(num_output_buff_allocated == 0)
            {

                free(pPlatformList);
                pPlatformList = NULL;

                ret = munmap (pmem_baseaddress, (nbufSize * pVidDec->sOutPortDef.nBufferCountActual));
                if(ret)
                {
                    ITTIAM_ERROR("munmap failed");
                }

                pmem_baseaddress = NULL;

                m_heap_ptr = NULL;

                close(pmem_fd);
                pmem_fd = 0;
            }
        }
    }
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
                OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
    return hComponent->FillThisBuffer(hComponent, buffer);
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

    ITTIAM_DEBUG("num_output_buff_allocated = %d", (int)num_output_buff_allocated);
    if(pPlatformList)
    {
        free(pPlatformList);
        pPlatformList = NULL;
    }

    if(pmem_baseaddress)
    {
        ret = munmap (pmem_baseaddress, (nbufSize * pVidDec->sOutPortDef.nBufferCountActual));
        if(ret)
        {
            ITTIAM_ERROR("munmap failed");
        }

        pmem_baseaddress = NULL;
    }

    m_heap_ptr = NULL;

    if(pmem_fd)
    {
        close(pmem_fd);
        pmem_fd = 0;
    }

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

//extern void cache_l1_hit(void *buffer);
#ifdef ICS
OMX_ERRORTYPE omx_vdec::use_android_native_buffer(OMX_IN OMX_HANDLETYPE hComp, OMX_PTR data)
{
    VIDDECTYPE *pVidDec;
    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    ITTIAM_DEBUG("Inside use_android_native_buffer");
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    UseAndroidNativeBufferParams *params = (UseAndroidNativeBufferParams *)data;

    if((params == NULL) ||
                    (params->nativeBuffer == NULL) ||
                    (params->nativeBuffer->handle == NULL) ||
                    !m_enable_android_native_buffers)
    return OMX_ErrorBadParameter;

    sp<android_native_buffer_t> nBuf = params->nativeBuffer;
    private_handle_t *handle = (private_handle_t *)nBuf->handle;

    if(pVidDec->sOutPortDef.nPortIndex == params->nPortIndex)
    { //android native buffers can be used only on Output port
        ITTIAM_DEBUG("use_android_native_buffer handle->size = %d, handle->fd = %x",handle->size, handle->fd);

        OMX_U8 *buffer = (OMX_U8*)mmap(0, handle->size,
                        PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0);

        if(buffer == MAP_FAILED)
        {
            ITTIAM_DEBUG("Failed to mmap pmem with fd = %d, size = %d", handle->fd, handle->size);
            return OMX_ErrorInsufficientResources;
        }

        handle->offset = pVidDec->offset;

        eRet = use_buffer(hComp,params->bufferHeader,params->nPortIndex,data,handle->size,buffer);

    }
    else
    {
        eRet = OMX_ErrorBadParameter;
    }
     ITTIAM_DEBUG("omx_vdec::use_android_native_buffer %d",eRet);
    return eRet;
}
#endif
void omx_vdec::allocate_platform_private()
{
    VIDDECTYPE *pVidDec;
    pVidDec =
                    (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    ITTIAM_DEBUG("\n Allocate o/p buffer case - Header List allocation");
    int nPlatformEntrySize = 0;
    int nPlatformListSize = 0;
    int nPMEMInfoSize = 0;

    nPMEMInfoSize = pVidDec->sOutPortDef.nBufferCountActual
                    * sizeof(OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO);
    nPlatformListSize = pVidDec->sOutPortDef.nBufferCountActual
                    * sizeof(OMX_QCOM_PLATFORM_PRIVATE_LIST);
    nPlatformEntrySize = pVidDec->sOutPortDef.nBufferCountActual
                    * sizeof(OMX_QCOM_PLATFORM_PRIVATE_ENTRY);

    // Alloc mem for platform specific info
    char *pPtr = NULL;
    pPtr = (char*)calloc(nPlatformListSize + nPlatformEntrySize + nPMEMInfoSize,
                         1);

    pPlatformList = (OMX_QCOM_PLATFORM_PRIVATE_LIST *)(pPtr);
    pPlatformEntry = (OMX_QCOM_PLATFORM_PRIVATE_ENTRY *)(((char *)pPlatformList)
                    + nPlatformListSize);
    pPMEMInfo = (OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *)(((char *)pPlatformEntry)
                    + nPlatformEntrySize);

}

bool omx_vdec::align_pmem_buffers(int pmem_fd,
                                  OMX_U32 buffer_size,
                                  OMX_U32 alignment)
{
    struct pmem_allocation allocation;
    allocation.size = buffer_size;
    allocation.align = clip2(alignment);
    if(allocation.align < 4096)
    {
        allocation.align = 4096;
    }
    if(ioctl(pmem_fd, PMEM_ALLOCATE_ALIGNED, &allocation) < 0)
    {
        ITTIAM_DEBUG("\n Aligment(%u) failed with pmem driver Sz(%lu)",
                     allocation.align, allocation.size);
        return false;
    }
    return true;
}

