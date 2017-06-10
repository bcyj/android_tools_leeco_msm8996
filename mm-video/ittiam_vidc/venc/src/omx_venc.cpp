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

           Copyright (c) 2006-2008, 2010-2012, 2013 Qualcomm Technologies, Inc.
           All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*******************************************************************************/

/*============================================================================
                            O p e n M A X   w r a p p e r s
                             O p e n  M A X   C o r e

*//** @file omx_venc.cpp
  This module contains the implementation of the OpenMAX core & component.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
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

#include "omx_venc.h"

#include "ittiam_datatypes.h"
#include "iv.h"
#include "ive.h"
#include <OMX_Ittiam.h>
#include <OMX_Ittiam_Venc.h>
#include <HardwareAPI.h>
#include <gralloc_priv.h>
#include <cutils/properties.h>
#ifdef ICS
#include "QComOMXMetadata.h"
#define PMEM_CLEAN_INV_CACHES   _IOW(PMEM_IOCTL_MAGIC, 11, unsigned int)
struct pmem_addr {
    unsigned long vaddr;
    unsigned long offset;
    unsigned long length;
};
#endif


//#define PMEM_DEVICE "/dev/pmem_smipool" // For MSM8660 device
#define PMEM_DEVICE "/dev/pmem_adsp" // For 7x27A device

#define PMEM_IOCTL_MAGIC 'p'

#define PMEM_ALLOCATE_ALIGNED   _IOW(PMEM_IOCTL_MAGIC, 15, unsigned int)

#define INPUT_BUFFER_LOG 0
#define OMX_QcomIndexParamVideoEncodeMetaBufferMode 0x71C
#if INPUT_BUFFER_LOG
FILE *inputBufferFile1;
char inputfilename [] = "/data/input.yuv";
#endif


long long itGetMs(void)
{
    struct timeval t;
    long long currTime;

    if(gettimeofday(&t,NULL) == -1)
    {
         printf("Error in gettimeofday. It has returned -1. \n");
    }
    currTime = ((t.tv_sec *1000 *1000) + (t.tv_usec));
    return currTime;
}

struct pmem_allocation {
    unsigned long size;
    unsigned int align;
};


VideoHeap::VideoHeap(int fd, size_t size, void* base)
{
    // dup file descriptor, map once, use pmem
    init(dup(fd), base, size, 0 , PMEM_DEVICE);
}

// factory function executed by the core to create instances
void *get_omx_component_factory_fn(void)
{
  return (new omx_venc);
}

/* ======================================================================
FUNCTION
  omx_venc::omx_venc

DESCRIPTION
  Constructor

PARAMETERS
  None

RETURN VALUE
  None.
========================================================================== */
omx_venc::omx_venc(): hComponent(NULL),
                      use_hardware_renderer(OMX_TRUE),
                      num_output_buff_allocated(0),
                      pPlatformList(NULL),
                      pmem_fd(0),
                      pmem_baseaddress(NULL),
                      m_heap_ptr(NULL)
{
    hComponent = (OMX_COMPONENTTYPE *)malloc(sizeof(OMX_COMPONENTTYPE));
    m_enable_android_native_buffers = 0;
    mTarget8960 = 0;
#ifdef ICS 
    meta_mode_enable = false;
#endif
}


/* ======================================================================
FUNCTION
  omx_venc::~omx_venc

DESCRIPTION
  Destructor

PARAMETERS
  None

RETURN VALUE
  None.
========================================================================== */
omx_venc::~omx_venc()
{
    if(hComponent != NULL)
    {
        free(hComponent);
        hComponent = NULL;
    }
}


/* ======================================================================
FUNCTION
  omx_venc::ComponentInit

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
OMX_ERRORTYPE omx_venc::component_init(OMX_STRING role)
{

  char mDeviceName[128];
  char mDeviceHwPlat[128];
  OMX_ERRORTYPE eRet = OMX_ErrorNone;
  OMX_IN OMX_VIDEO_CODINGTYPE VidFormat;
  OMX_U32 high_quality_encoder = 0;
  OMX_U32 quadcore = 0;
  char alt_ref_flag[PROPERTY_VALUE_MAX];
  OMX_U32 u4_alt_ref_flag = 0;
  PROCESSORTYPE processor_type;
  OMX_COLOR_FORMATTYPE colorFormat =
                    (OMX_COLOR_FORMATTYPE) IOMX_COLOR_FormatYVU420SemiPlanar;



  ITTIAM_DEBUG("In %s function, role = %s", __FUNCTION__, role);

  if(!strncmp(role, "OMX.ittiam.video.encoder.avc",\
         OMX_MAX_STRINGNAME_SIZE))
  {
     VidFormat = OMX_VIDEO_CodingAVC;
  }
  else
  {
    eRet = OMX_ErrorInvalidComponentName;
    return eRet;

  }
  property_get("ro.product.device",mDeviceName,"0");
  property_get("ro.hw_plat",mDeviceHwPlat,"0");
  property_get("ro.alt_ref_flag",alt_ref_flag,"0");

  u4_alt_ref_flag = atoi(alt_ref_flag);

  if((u4_alt_ref_flag != 0)||(u4_alt_ref_flag != 1))
  u4_alt_ref_flag  = 1;

   ITTIAM_DEBUG("ro.product.device %s",mDeviceName);
   ITTIAM_DEBUG("ro.hw_plat %s",mDeviceHwPlat);

   char  buffer[10];
    int soc_id;
    FILE *device = NULL;
    FILE *nCPU = NULL;
    int result;

    device = fopen("/sys/devices/system/soc/soc0/id","r");
    if(device)
    {
        result = fread(buffer, 1, 4, device);
        fclose(device);
    }

    soc_id = atoi(buffer);
    ITTIAM_LOG("SOC ID : %d\n",soc_id);
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
    if((QCOM_8X25Q == processor_type) || (QCOM_8X12 == processor_type))
    {
        mTarget8960 = 0;
        high_quality_encoder = 0;
        quadcore = 1;
        ITTIAM_LOG("Quadcore Selected, Extended toolset disabled");
    }
    else if ((QCOM_8X25 == processor_type) || (QCOM_8X10 == processor_type))
    {
        mTarget8960 = 0;
        high_quality_encoder = 0;
        quadcore = 0;
        ITTIAM_LOG("DualCore Selected, Extended toolset disabled");
    }
    else if ((QCOM_8974 == processor_type) ||
                (QCOM_APQ8064 == processor_type) ||
                (QCOM_MPQ8064 == processor_type) ||
                (QCOM_8960 == processor_type))
     {
        mTarget8960 = 1;
        high_quality_encoder = 1;
        quadcore = 0;
        ITTIAM_LOG("DualCore Selected, Extended toolset enabled");
     }
       else
       {
        mTarget8960 = 0;
        high_quality_encoder = 0;
        quadcore = 0;
        ITTIAM_LOG("DualCore Selected, Extended toolset disabled");
       }
       switch (processor_type)
       {
             case GENERIC:
             case QCOM_GENERIC:
             case QCOM_7X27A:
             case QCOM_8X25:
             case QCOM_8X25Q:
                   ITTIAM_DEBUG("IOMX_COLOR_FormatYVU420SemiPlanar in component_init");
                   colorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar;
               break;
             case QCOM_8X10:
             case QCOM_8X12:
             case QCOM_8960:
             case QCOM_APQ8064:
             case QCOM_MPQ8064:
             case QCOM_8974:
             case SAMSUNG_GENERIC:
             case TI_GENERIC:
                   ITTIAM_DEBUG("OMX_COLOR_FormatYUV420SemiPlanar in component_init");
                   colorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
                   break;
         }

  eRet = ComponentInit((OMX_HANDLETYPE)hComponent, VidFormat,
                        colorFormat,high_quality_encoder,quadcore,u4_alt_ref_flag);

 #if INPUT_BUFFER_LOG
     inputBufferFile1 = fopen (inputfilename, "ab");
#endif
  return eRet;
}

/* ======================================================================
FUNCTION
  omx_venc::GetComponentVersion

DESCRIPTION
  Returns the component version.

PARAMETERS
  TBD.

RETURN VALUE
  OMX_ErrorNone.

========================================================================== */
OMX_ERRORTYPE  omx_venc::get_component_version
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
  omx_venc::SendCommand

DESCRIPTION
  Returns zero if all the buffers released..

PARAMETERS
  None.

RETURN VALUE
  true/false

========================================================================== */
OMX_ERRORTYPE  omx_venc::send_command(OMX_IN OMX_HANDLETYPE hComp,
                                      OMX_IN OMX_COMMANDTYPE cmd,
                                      OMX_IN OMX_U32 param1,
                                      OMX_IN OMX_PTR cmdData
                                      )
{
  return hComponent->SendCommand(hComponent, cmd, param1, cmdData);
}

/* ======================================================================
FUNCTION
  omx_venc::GetParameter

DESCRIPTION
  OMX Get Parameter method implementation

PARAMETERS
  <TBD>.

RETURN VALUE
  Error None if successful.

========================================================================== */
OMX_ERRORTYPE  omx_venc::get_parameter(OMX_IN OMX_HANDLETYPE     hComp,
                                           OMX_IN OMX_INDEXTYPE paramIndex,
                                           OMX_INOUT OMX_PTR     paramData)
{

  return hComponent->GetParameter(hComponent, paramIndex, paramData);
}

/* ======================================================================
FUNCTION
  omx_venc::Setparameter

DESCRIPTION
  OMX Set Parameter method implementation.

PARAMETERS
  <TBD>.

RETURN VALUE
  OMX Error None if successful.

========================================================================== */
OMX_ERRORTYPE  omx_venc::set_parameter(OMX_IN OMX_HANDLETYPE     hComp,
                                           OMX_IN OMX_INDEXTYPE paramIndex,
                                           OMX_IN OMX_PTR        paramData)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    VIDENCTYPE *pVidEnc;
    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

       ITTIAM_DEBUG("%s paramIndex = %x",__FUNCTION__,paramIndex);


    if(paramData == NULL)
    {
         ITTIAM_ERROR("Get Param in Invalid paramData \n");
         return OMX_ErrorBadParameter;
    }
    switch((OMX_QCOM_EXTN_INDEXTYPE)paramIndex)
    {
#ifdef ICS
     case OMX_QcomIndexParamVideoEncodeMetaBufferMode:
    {
        StoreMetaDataInBuffersParams *pParam =
        (StoreMetaDataInBuffersParams*)paramData;

        if(pParam->nPortIndex == 0x0)
      {
        if(pParam->bStoreMetaData != meta_mode_enable && pVidEnc->mUseLifeEffects != 1)
        {
            meta_mode_enable = pParam->bStoreMetaData;
            if(meta_mode_enable) {
            pVidEnc->sInPortDef.nBufferCountMin = 7;
            pVidEnc->sInPortDef.nBufferCountActual= 7;
            pVidEnc->sInPortDef.nBufferSize= sizeof(encoder_media_buffer_type);
            pVidEnc->meta_mode_enable= 1;
            ITTIAM_DEBUG("Meta mode enabled\n");

        }
        }
    }
    break;
    }

#endif
    default:
    {
      eRet = hComponent->SetParameter(hComponent, paramIndex, paramData);
      break;
    }
  }
  return eRet;
}

/* ======================================================================
FUNCTION
  omx_venc::GetConfig

DESCRIPTION
  OMX Get Config Method implementation.

PARAMETERS
  <TBD>.

RETURN VALUE
  OMX Error None if successful.

========================================================================== */
OMX_ERRORTYPE  omx_venc::get_config(OMX_IN OMX_HANDLETYPE      hComp,
                                        OMX_IN OMX_INDEXTYPE configIndex,
                                        OMX_INOUT OMX_PTR     configData)

{
  return hComponent->GetConfig(hComponent, configIndex, configData);
}

/* ======================================================================
FUNCTION
  omx_venc::SetConfig

DESCRIPTION
  OMX Set Config method implementation

PARAMETERS
  <TBD>.

RETURN VALUE
  OMX Error None if successful.
========================================================================== */
OMX_ERRORTYPE  omx_venc::set_config(OMX_IN OMX_HANDLETYPE      hComp,
                                        OMX_IN OMX_INDEXTYPE configIndex,
                                        OMX_IN OMX_PTR        configData)
{
  return hComponent->SetConfig(hComponent, configIndex, configData);
}


/* ======================================================================
FUNCTION
  omx_venc::GetExtensionIndex

DESCRIPTION
  OMX GetExtensionIndex method implementaion.  <TBD>

PARAMETERS
  <TBD>.

RETURN VALUE
  OMX Error None if everything successful.

========================================================================== */
OMX_ERRORTYPE  omx_venc::get_extension_index(OMX_IN OMX_HANDLETYPE      hComp,
                                                OMX_IN OMX_STRING      paramName,
                                                OMX_OUT OMX_INDEXTYPE* indexType)
{
#ifdef ICS
    if (!strncmp(paramName, "OMX.google.android.index.storeMetaDataInBuffers",sizeof("OMX.google.android.index.storeMetaDataInBuffers") - 1)) {
        *indexType = (OMX_INDEXTYPE)OMX_QcomIndexParamVideoEncodeMetaBufferMode;
    }
    else if(!strncmp(paramName,"OMX.google.android.index.useAndroidNativeBuffer2", sizeof("OMX.google.android.index.enableAndroidNativeBuffer2") - 1)) {
        ITTIAM_ERROR("Extension: %s is not supported\n", paramName);
        return OMX_ErrorNotImplemented;
    }
    else if(!strncmp(paramName,"OMX.google.android.index.useAndroidNativeBuffer", sizeof("OMX.google.android.index.enableAndroidNativeBuffer") - 1)) {
        ITTIAM_ERROR("Extension: %s is not supported\n", paramName);
        return OMX_ErrorNotImplemented;
    }
    else if(!strncmp(paramName,"OMX.google.android.index.getAndroidNativeBufferUsage", sizeof("OMX.google.android.index.getAndroidNativeBufferUsage") - 1)) {
        ITTIAM_ERROR("Extension: %s is not supported\n", paramName);
        return OMX_ErrorNotImplemented;
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
  omx_venc::GetState

DESCRIPTION
  Returns the state information back to the caller.<TBD>

PARAMETERS
  <TBD>.

RETURN VALUE
  Error None if everything is successful.
========================================================================== */
OMX_ERRORTYPE  omx_venc::get_state(OMX_IN OMX_HANDLETYPE  hComp,
                                       OMX_OUT OMX_STATETYPE* state)
{
  return hComponent->GetState(hComponent, state);
}

/* ======================================================================
FUNCTION
  omx_venc::ComponentTunnelRequest

DESCRIPTION
  OMX Component Tunnel Request method implementation. <TBD>

PARAMETERS
  None.

RETURN VALUE
  OMX Error None if everything successful.

========================================================================== */
OMX_ERRORTYPE  omx_venc::component_tunnel_request(OMX_IN OMX_HANDLETYPE                hComp,
                                                     OMX_IN OMX_U32                        port,
                                                     OMX_IN OMX_HANDLETYPE        peerComponent,
                                                     OMX_IN OMX_U32                    peerPort,
                                                     OMX_INOUT OMX_TUNNELSETUPTYPE* tunnelSetup)
{
  return hComponent->ComponentTunnelRequest(hComponent, port, peerComponent, peerPort, tunnelSetup);
}

/* ======================================================================
FUNCTION
  omx_venc::UseBuffer

DESCRIPTION
  OMX Use Buffer method implementation.

PARAMETERS
  <TBD>.

RETURN VALUE
  OMX Error None , if everything successful.

========================================================================== */
OMX_ERRORTYPE  omx_venc::use_buffer(
                         OMX_IN OMX_HANDLETYPE            hComp,
                         OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
                         OMX_IN OMX_U32                   port,
                         OMX_IN OMX_PTR                   appData,
                         OMX_IN OMX_U32                   bytes,
                         OMX_IN OMX_U8*                   buffer)

{
   ITTIAM_DEBUG("%s",__FUNCTION__);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    VIDENCTYPE *pVidEnc;
    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    eRet = hComponent->UseBuffer(hComponent, bufferHdr, port, appData, bytes, buffer);



    return eRet;
}


// AllocateBuffer  -- API Call
/* ======================================================================
FUNCTION
  omx_venc::AllocateBuffer

DESCRIPTION
  Returns zero if all the buffers released..

PARAMETERS
  None.

RETURN VALUE
  true/false

========================================================================== */
OMX_ERRORTYPE  omx_venc::allocate_buffer(OMX_IN OMX_HANDLETYPE                hComp,
                                     OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
                                     OMX_IN OMX_U32                        port,
                                     OMX_IN OMX_PTR                     appData,
                                     OMX_IN OMX_U32                       bytes)
{
   ITTIAM_DEBUG("%s",__FUNCTION__);
   OMX_ERRORTYPE eRet = OMX_ErrorNone;
    VIDENCTYPE *pVidEnc;
    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
#ifdef ICS
     if((port == 0x0 /* INPUT PORT */) && meta_mode_enable){
            bytes = sizeof(encoder_media_buffer_type);
        }
#endif
    eRet = hComponent->AllocateBuffer(hComponent, bufferHdr, port, appData, bytes);
    return eRet;
  }


// Free Buffer - API call
/* ======================================================================
FUNCTION
  omx_venc::FreeBuffer

DESCRIPTION

PARAMETERS
  None.

RETURN VALUE
  true/false

========================================================================== */
OMX_ERRORTYPE  omx_venc::free_buffer(OMX_IN OMX_HANDLETYPE         hComp,
                                      OMX_IN OMX_U32                 port,
                                      OMX_IN OMX_BUFFERHEADERTYPE* buffer)

{
  return hComponent->FreeBuffer(hComponent, port, buffer);
}


/* ======================================================================
FUNCTION
  omx_venc::EmptyThisBuffer

DESCRIPTION
  This routine is used to push the encoded video frames to
  the video decoder.

PARAMETERS
  None.

RETURN VALUE
  OMX Error None if everything went successful.

========================================================================== */
OMX_ERRORTYPE  omx_venc::empty_this_buffer(OMX_IN OMX_HANDLETYPE         hComp,
                                           OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{

#ifdef ICS
    VIDENCTYPE *pVidEnc;
    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_U8* tempbuffer;
  encoder_media_buffer_type *media_buffer =(encoder_media_buffer_type *)buffer->pBuffer;
  #if INPUT_BUFFER_LOG
   int y_size = 0;
   int c_offset = 0;
   unsigned char *buf_addr = NULL;

   y_size =  1280*720;
   c_offset= (y_size + 2047) & (~(2047));
  #endif


   if(media_buffer->buffer_type == kMetadataBufferTypeCameraSource)
   {
       UWORD32 i;
        for(i = 0; i < NUM_IN_BUFFERS; i++)
        {
            if((buffer->pBuffer) == pVidEnc->inp_buf_id_mapping[i])
            {
                ITTIAM_DEBUG("%s Found matching id %d buffer->pBuffer %x buffer %x media_buffer->meta_handle->data[3] %x",
                     __FUNCTION__,i, (WORD32)(buffer->pBuffer), (WORD32)buffer, (WORD32)media_buffer->meta_handle->data[3]);
                if(mTarget8960)
                {
                    pVidEnc->inp_y_id_mapping[i] = (OMX_U8*)mmap(0, media_buffer->meta_handle->data[2],
                                            PROT_READ|PROT_WRITE, MAP_SHARED, media_buffer->meta_handle->data[0], 0);
                    pVidEnc->BufferUnmaprequired =1;
#if INPUT_BUFFER_LOG
                    buf_addr = (unsigned char *)pVidEnc->inp_y_id_mapping[i];
#endif
                }
                else
                pVidEnc->inp_y_id_mapping[i] = (void*)media_buffer->meta_handle->data[3];

                if(pVidEnc->inp_y_id_mapping[i] == NULL)
                {
                    ITTIAM_ERROR("Input Buffer is NULL");
                    pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData, OMX_EventError, OMX_ErrorInvalidState, OMX_StateInvalid, NULL );
                }
                pVidEnc->buffersize = media_buffer->meta_handle->data[2];
                break;
            }
        }

   }
   else{
        private_handle_t *handle = (private_handle_t *)media_buffer->meta_handle;
        UWORD32 i;
        for(i = 0; i < NUM_IN_BUFFERS; i++)
        {
            if((buffer->pBuffer) == pVidEnc->inp_buf_id_mapping[i])
        {
                   ITTIAM_DEBUG("Getting handle for gralloc buffers");
                pVidEnc->inp_y_id_mapping[i] = (void *)handle->base;
                    pVidEnc->BufferUnmaprequired = 0;
            if(pVidEnc->inp_y_id_mapping[i] == NULL)
            {
                ITTIAM_ERROR("Input Buffer is NULL");
                pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData, OMX_EventError, OMX_ErrorInvalidState, OMX_StateInvalid, NULL );
            }
                pVidEnc->buffersize = handle->size;
            break;

            }
        }
        }

#endif
#if INPUT_BUFFER_LOG
if(inputBufferFile1)
{
    fwrite((const char *)buf_addr, y_size, 1, inputBufferFile1);
    fwrite((const char *)(buf_addr + c_offset), (y_size>>1), 1, inputBufferFile1);

}
#endif
  return hComponent->EmptyThisBuffer(hComponent, buffer);
}


/* ======================================================================
FUNCTION
  omx_venc::FillThisBuffer

DESCRIPTION
  IL client uses this method to release the frame buffer
  after displaying them.

PARAMETERS
  None.

RETURN VALUE
  true/false

========================================================================== */
OMX_ERRORTYPE  omx_venc::fill_this_buffer(OMX_IN OMX_HANDLETYPE  hComp,
                                          OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  return hComponent->FillThisBuffer(hComponent, buffer);
}

/* ======================================================================
FUNCTION
  omx_venc::SetCallbacks

DESCRIPTION
  Set the callbacks.

PARAMETERS
  None.

RETURN VALUE
  OMX Error None if everything successful.

========================================================================== */
OMX_ERRORTYPE  omx_venc::set_callbacks(OMX_IN OMX_HANDLETYPE        hComp,
                                           OMX_IN OMX_CALLBACKTYPE* callbacks,
                                           OMX_IN OMX_PTR             appData)
{

  return hComponent->SetCallbacks(hComponent, callbacks, appData);

}

/* ======================================================================
FUNCTION
  omx_venc::ComponentDeInit

DESCRIPTION
  Destroys the component and release memory allocated to the heap.

PARAMETERS
  <TBD>.

RETURN VALUE
  OMX Error None if everything successful.

========================================================================== */
OMX_ERRORTYPE  omx_venc::component_deinit(OMX_IN OMX_HANDLETYPE hComp)
{

      int nbufSize;
      VIDENCTYPE *pVidEnc;
      pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
      nbufSize = pVidEnc->sOutPortDef.nBufferSize;

#if INPUT_BUFFER_LOG
 fclose (inputBufferFile1);
#endif

  return hComponent->ComponentDeInit(hComponent);

}


/* ======================================================================
FUNCTION
  omx_venc::UseEGLImage

DESCRIPTION
  OMX Use EGL Image method implementation <TBD>.

PARAMETERS
  <TBD>.

RETURN VALUE
  Not Implemented error.

========================================================================== */
OMX_ERRORTYPE  omx_venc::use_EGL_image(OMX_IN OMX_HANDLETYPE                hComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
                                          OMX_IN OMX_U32                        port,
                                          OMX_IN OMX_PTR                     appData,
                                          OMX_IN void*                      eglImage)
{

  return hComponent->UseEGLImage(hComponent, bufferHdr, port, appData, eglImage);

}


/* ======================================================================
FUNCTION
  omx_venc::ComponentRoleEnum

DESCRIPTION
  OMX Component Role Enum method implementation.

PARAMETERS
  <TBD>.

RETURN VALUE
  OMX Error None if everything is successful.
========================================================================== */
OMX_ERRORTYPE  omx_venc::component_role_enum(OMX_IN OMX_HANDLETYPE hComp,
                                                OMX_OUT OMX_U8*        role,
                                                OMX_IN OMX_U32        index)
{
  return hComponent->ComponentRoleEnum(hComponent, role, index);
}


