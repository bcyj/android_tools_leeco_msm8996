#ifndef __MMSTILL_JPEG_OMX_ENC_H__
#define __MMSTILL_JPEG_OMX_ENC_H__
/* ========================================================================= *
        Copyright © 2012 Qualcomm Technologies, Inc. All Rights Reserved.
              Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#include "OMX_Types.h"
#include "OMX_Index.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "omx_debug.h"

#ifndef OMX_CODEC_V1_WRAPPER
#include "omx_jpeg_ext.h"
#else
#include "QOMX_JpegExtensions.h"
#include "qomx_core.h"
#include "qexif.h"
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define PAD_TO_WORD(a)(((a)+3)&~3)
#endif


#define DEBUG_PRINT ALOGE
#define DEBUG_PRINT_ERROR ALOGE

int JpegOMX_Init();
int JpegOMX_DeInit();
int JpegOMX_GetHandle(
  OMX_IN char *compName,
  OMX_IN char *appData,
  OMX_IN OMX_CALLBACKTYPE *callback);

int JpegOMX_GetJpegHandle();

int JpegOMX_FreeHandle(
  OMX_IN int omx_handle);

int JpegOMX_FreeJpegHandle();

int JpegOMX_AllocateBuffer(
  OMX_IN int omx_handle,
  OMX_IN OMX_U32 portIndex,
  OMX_IN int appData,
  OMX_IN unsigned int bytes);

int JpegOMX_FreeBuffer(
  OMX_IN int omx_handle,
  OMX_IN OMX_U32 portIndex,
  OMX_IN int pBuf);

int JpegOMX_AllFreeBuffer();

int JpegOMX_GetParameter(
  OMX_IN int omx_handle,
  OMX_IN int index,
  OMX_INOUT int pCompParam);

int JpegOMX_SendCommand(
  OMX_IN int omx_handle,
  OMX_IN int cmd,
  OMX_IN OMX_U32 param,
  OMX_IN OMX_MARKTYPE cmdData);

int JpegOMX_SendJpegCommand(
  int cmd,
  OMX_U32 param);

int JpegOMX_FillThisBuffer(
  OMX_IN int omx_handle,
  OMX_IN int pBufHdr);

int JpegOMX_EmptyThisBuffer(
  OMX_IN int omx_handle,
  OMX_IN int pBufHdr);

int JpegOMX_GetState(
  OMX_IN int omx_handle);

int JpegOMX_GetComponentVersion(
  OMX_IN int omx_handle,
  OMX_OUT char *componentName,
  OMX_OUT int componentVersion,
  OMX_OUT int specVersion,
  OMX_OUT int componentUUID);

int JpegOMX_GetConfig(
  OMX_IN int omx_handle,
  OMX_IN int index,
  OMX_INOUT int pCompConfig);

int JpegOMX_UseBuffer(
  OMX_IN int omx_handle,
  OMX_IN OMX_U32 portIndex,
  OMX_IN int appData,
  OMX_IN unsigned int numBytes,
  OMX_IN int  pBuf);

int JpegOMX_AllUseBuffers();

int JpegOMX_SetAllParameters();

int JpegOMX_SetParameter(
  OMX_IN int omx_handle,
  OMX_IN int index,
  OMX_IN int pCompParam);

int JpegOMX_SetParameter_ImageInit(
  OMX_IN int omx_handle,
  int nSize, int nPorts, int nStartPortNumber,
  OMX_U8 nVersionMajor, OMX_U8 nVersionMinor,
  OMX_U8 nRevision, OMX_U8 nStep); //OMX_PORT_PARAM_TYPE

int JpegOMX_SetParameter_ImagePortFormat(
  OMX_IN int omx_handle,
  int nSize, int nPortIndex,
  int nIndex, int eCompressionFormat, int eColorFormat,
  OMX_U8 nVersionMajor, OMX_U8 nVersionMinor,
  OMX_U8 nRevision, OMX_U8 nStep); //OMX_IMAGE_PARAM_PORTFORMATTYPE

int JpegOMX_SetParameter_PortDef(
  OMX_IN int omx_handle, int port,
  int use_default_value, OMX_U32 nPortIndex,
  OMX_U32 eDir, OMX_U32 nBufferCountActual,
  OMX_U32 nBufferCountMin, OMX_U32 nBufferSize,
  int bEnabled,  int bPopulated,  int eDomain,
  OMX_STRING cMIMEType, OMX_U32 nFrameWidth,
  OMX_U32 nFrameHeight, OMX_S32 nStride, OMX_U32 nSliceHeight,
  int bFlagErrorConcealment, OMX_U32 eCompressionFormat,
  OMX_U32 eColorFormat); //OMX_PARAM_PORTDEFINITIONTYPE

int JpegOMX_SetParameter_QFactor(
  OMX_IN int omx_handle, OMX_U32 nSize,
  OMX_U32 nPortIndex, int quality); //OMX_IMAGE_PARAM_QFACTORTYPE

int JpegOMX_SetParameter_Exif(
  OMX_IN int omx_handle,
  int tag_id, int tag_entry_type,
  int tag_entry_count, int tag_entry_copy,
  int tag_entry_rat_num, int tag_entry_rat_denom,
  char *tag_entry_ascii); //omx_jpeg_exif_info_tag

int JpegOMX_SetParameter_Thumbnail(
  OMX_IN int omx_handle,
  int height, int width,
  int scaling, int cropWidth,
  int cropHeight,
  int left, int top); //omx_jpeg_thumbnail

int JpegOMX_SetParameter_ThumbQuality(
  OMX_IN int omx_handle,
  OMX_IN int thumbnailquality_nQFactor); //omx_jpeg_thumbnail_quality

int JpegOMX_SetParameter_BufferOffset(
  OMX_IN int omx_handle,
  int width, int height,
  int usePadding, int  rotation); //omx_jpeg_buffer_offset
#ifndef OMX_CODEC_V1_WRAPPER
int JpegOMX_SetParameter_ACbCrOffset(
  OMX_IN int omx_handle,
  OMX_IN omx_jpeg_buffer_offset ACbCrOffset);
#endif
int JpegOMX_SetParameter_UserPreferences(
  OMX_IN int omx_handle,
  int color_format,
  int thumbnail_color_format,
  int preference); //omx_jpeg_user_preferences

int JpegOMX_SetAllConfigs();

int JpegOMX_SetConfig(
  OMX_IN int omx_handle,
  OMX_IN int index,
  OMX_IN int pConfiParam);

  int JpegOMX_SetConfig_Rotate(
  OMX_IN int omx_handle,
  OMX_U32  port_index,
  OMX_S32 rotation); //OMX_CONFIG_ROTATIONTYPE

int JpegOMX_SetConfig_InputCrop(
  OMX_IN int omx_handle,
  OMX_U32 left, OMX_U32 top,
  OMX_U32 width, OMX_U32 height,
  OMX_U32 port_index); //OMX_CONFIG_RECTTYPE

int JpegOMX_SetConfig_OutputCrop(
  OMX_IN int omx_handle,
  OMX_U32 left, OMX_U32 top,
  OMX_U32 width, OMX_U32 height,
  OMX_U32 port_index); //OMX_CONFIG_RECTTYPE

int JpegOMX_GetExtIdx(
  OMX_IN int omx_handle,
  OMX_IN char *compName);

/*int JpegOMX_SetCallBacks(
  OMX_IN int omx_handle,
  OMX_IN OMX_CALLBACKTYPE* callbacks,
  OMX_IN int appData);

int JpegOMX_ComponentDeInit(
  OMX_IN int omx_handle);*/

int JpegOMX_ComponentTunnelRequest(
  OMX_IN int omx_handle,
  OMX_IN unsigned int port,
  OMX_IN int peerComponent,
  OMX_IN unsigned int peerPort,
  OMX_INOUT OMX_TUNNELSETUPTYPE tunnelSetup);

int StartEncode(void);

#endif /* __MMSTILL_JPEG_OMX_ENC_H__ */
