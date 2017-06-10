/* ========================================================================= *
   Purpose:  Module contains utility functions that will be used
             by cFuzzer

   Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/android_pmem.h>
#include <linux/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ion.h>
#include <sys/time.h>

/* KHRONOS header files */
#include "OMX_Types.h"
#include "OMX_Index.h"
#include "OMX_Core.h"
#include "OMX_Component.h"

#include "omx_debug.h"

#ifdef __cplusplus
extern "C" {
#include "os_pmem.h"
}
#endif

#include "mmstill_jpeg_omx_enc.h"
#include "mmstillomxenc.h"
#define OUT_FILE_PREFIX "/data/cFuzzer/cFuzzerLogs/"
#ifndef LOGI
#define LOGI ALOGE
#endif
//#define LOG_TAG "mmstill-Fuzz-wrapper"

#define PRINT_RESULT() DEBUG_PRINT("Fuzzing %s - Exit omxresult = 0x%X !!",__FUNCTION__,omxresult);
OMX_HANDLETYPE g_handle;
int g_data_alloc = 0;
OMX_BUFFERHEADERTYPE* pInBuffers = NULL;
OMX_BUFFERHEADERTYPE* pInBuffers1 = NULL;
OMX_BUFFERHEADERTYPE* pOutBuffers = NULL;


int usePadding =0;
uint8_t use_pmem = 1;

struct ion_buff_data {
  int pmem_fd;
  int ion_fd;
  struct ion_allocation_data alloc_data;
  struct ion_fd_data info_fd;
  long size;
  void *addr;
};

struct ion_buff_data Buff, Buff1, OutBuff;
OMX_U32 size, size_thumb;

int8_t get_buffer_offset(uint32_t width, uint32_t height, OMX_U32* p_y_offset,
                         OMX_U32* p_cbcr_offset, OMX_U32* p_buf_size, int usePadding,
                         int rotation,   OMX_U32 *p_cbcrStartOffset,
                         float chroma_wt);

void readFile(const char * filename, uint8_t* buffer,int w, int h);

void allocate_buffers(int w, int h, int w_thumbnail, int h_thumbnail);

int initialize_portdef(int omx_handle, OMX_PARAM_PORTDEFINITIONTYPE * port, unsigned int nPortIndex);

typedef struct {
  char                *file_name;
  uint32_t             width;
  uint32_t             height;
  uint32_t             quality;
  QOMX_IMG_COLOR_FORMATTYPE format;

} input_image_args_t;

typedef struct {
  uint32_t    input_width;
  uint32_t    input_height;
  uint32_t    h_offset;
  uint32_t    v_offset;
  uint32_t    output_width;
  uint32_t    output_height;
  uint8_t     enable;
}image_scale_cfg_t;

/*Input data structure*/
typedef struct {
  input_image_args_t   main;
  input_image_args_t   thumbnail;
  char *               output_file;
  int16_t              rotation;
  uint8_t              preference;
  uint8_t              encode_thumbnail;
  uint16_t             back_to_back_count;
  uint32_t             target_filesize;
  uint32_t             output_buf_size;
  uint32_t             abort_time;
  const char*          reference_file;
  uint8_t              use_pmem;
  image_scale_cfg_t    main_scale_cfg;
  image_scale_cfg_t    tn_scale_cfg;
} test_args_t;

static OMX_ERRORTYPE EventHandler(
  OMX_IN OMX_HANDLETYPE hComponent,
  OMX_IN OMX_PTR pAppData,
  OMX_IN OMX_EVENTTYPE eEvent,
  OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
  OMX_IN OMX_PTR pEventData);
static OMX_ERRORTYPE EmptyBufferDone(
  OMX_IN OMX_HANDLETYPE hComponent,
  OMX_IN OMX_PTR pAppData,
  OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);
static OMX_ERRORTYPE FillBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

char dummy[100];
test_args_t test_args;
static OMX_CALLBACKTYPE call_back = {&EventHandler, &EmptyBufferDone, &FillBufferDone};

/******************************************************************/
/*                  FUNCTION DEFINITIONS                          */
/******************************************************************/
void* buffer_allocate(ion_buff_data *p_buffer)
{
  void *l_buffer = NULL;

  int lrc = 0;
  struct ion_handle_data lhandle_data;

  p_buffer->alloc_data.len = p_buffer->size;
  p_buffer->alloc_data.align = 4096;
  p_buffer->alloc_data.flags = ION_FLAG_CACHED;
  p_buffer->alloc_data.heap_mask = (0x1 << ION_IOMMU_HEAP_ID);

  p_buffer->ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
  if(p_buffer->ion_fd < 0) {
    OMX_DBG_ERROR("%s :Ion open failed\n", __func__);
    goto ION_ALLOC_FAILED;
  }

  /* Make it page size aligned */
  p_buffer->alloc_data.len = (p_buffer->alloc_data.len + 4095) & (~4095);
  OMX_DBG_ERROR("Size aligned %d\n", p_buffer->alloc_data.len);
  lrc = ioctl(p_buffer->ion_fd, ION_IOC_ALLOC, &p_buffer->alloc_data);
  if (lrc < 0) {
    OMX_DBG_ERROR("%s:%d ION allocation failed %d\n", __func__, __LINE__, lrc);
    goto ION_OPEN_FAILED;
  }

  p_buffer->info_fd.handle = p_buffer->alloc_data.handle;
  lrc = ioctl(p_buffer->ion_fd, ION_IOC_SHARE,
    &p_buffer->info_fd);
  if (lrc < 0) {
    OMX_DBG_ERROR("%s :ION map failed %s\n", __func__, strerror(errno));
    goto ION_MAP_FAILED;
  }

  p_buffer->pmem_fd = p_buffer->info_fd.fd;

  l_buffer = mmap(NULL, p_buffer->alloc_data.len, PROT_READ  | PROT_WRITE,
    MAP_SHARED, p_buffer->pmem_fd, 0);

  if (l_buffer == MAP_FAILED) {
    OMX_DBG_ERROR("%s :ION_MMAP_FAILED: %s (%d)\n", __func__,
      strerror(errno), errno);
    goto ION_MAP_FAILED;
  }
  OMX_DBG_INFO("%s:%d] fd %d", __func__, __LINE__, p_buffer->pmem_fd);

  return l_buffer;

ION_MAP_FAILED:
  lhandle_data.handle = p_buffer->info_fd.handle;
  ioctl(p_buffer->ion_fd, ION_IOC_FREE, &lhandle_data);
ION_ALLOC_FAILED:
  close(p_buffer->ion_fd);
ION_OPEN_FAILED:
  return NULL;
}

int buffer_deallocate(ion_buff_data *p_buffer)
{
  int lrc = 0;
  int lsize = (p_buffer->size + 4095) & (~4095);

  struct ion_handle_data lhandle_data;
  lrc = munmap(p_buffer->addr, lsize);

  close(p_buffer->info_fd.fd);

  lhandle_data.handle = p_buffer->info_fd.handle;
  ioctl(p_buffer->ion_fd, ION_IOC_FREE, &lhandle_data);

  close(p_buffer->ion_fd);
  return lrc;
}

int JpegOMX_Init()
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  test_args.main.width = 0;
  test_args.main.height = 0;
  test_args.thumbnail.width = 0;
  test_args.thumbnail.height = 0;
  return OMX_Init();
}

int JpegOMX_DeInit()
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  return OMX_Deinit();
}

int JpegOMX_GetHandle(
  OMX_IN char* compName,
  OMX_IN char* appData,
  OMX_IN OMX_CALLBACKTYPE *callback)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_HANDLETYPE omx_handle;
  (void)callback;
  omxresult = OMX_GetHandle(&omx_handle,(OMX_STRING)compName,
    (OMX_PTR)appData, &call_back);
  g_handle = omx_handle;
  PRINT_RESULT();
  return (int)omx_handle;
}
int JpegOMX_GetJpegHandle()
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;

  omxresult = OMX_GetHandle(&g_handle,(OMX_STRING)"OMX.qcom.image.jpeg.encoder",
    NULL, &call_back);
  PRINT_RESULT();
  OMX_DBG_ERROR("Out %s: handle %p\n", __func__, (int *)g_handle);
  return (int)g_handle;
}
int JpegOMX_FreeHandle(
  OMX_IN int omx_handle)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  omxresult = OMX_FreeHandle((OMX_HANDLETYPE)omx_handle);
  PRINT_RESULT();
  return (int)omxresult;
}
int JpegOMX_FreeJpegHandle()
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  omxresult = OMX_FreeHandle((OMX_HANDLETYPE)g_handle);
  OMX_DBG_ERROR("In ssss%s:\n", __func__);
  PRINT_RESULT();
  g_handle = NULL;
  if (g_data_alloc){
    buffer_deallocate(&Buff);
    buffer_deallocate(&Buff1);
    buffer_deallocate(&OutBuff);
  }
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_AllocateBuffer(
  OMX_IN int omx_handle,
  OMX_IN OMX_U32 portIndex,
  OMX_IN int appData,
  OMX_IN unsigned int bytes)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_BUFFERHEADERTYPE* pBuf;
  omxresult = OMX_AllocateBuffer((OMX_COMPONENTTYPE*)omx_handle, &pBuf,
    portIndex,(OMX_PTR)appData, bytes);
  PRINT_RESULT();
  return (int)pBuf;
}

int JpegOMX_FreeBuffer(
  OMX_IN int omx_handle,
  OMX_IN OMX_U32 portIndex,
  OMX_IN int pBuf)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult = OMX_ErrorNone;
  omxresult = OMX_FreeBuffer((OMX_COMPONENTTYPE*)omx_handle,
    portIndex,(OMX_BUFFERHEADERTYPE*)pBuf);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_AllFreeBuffer()
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult = OMX_ErrorNone;
  omxresult = OMX_FreeBuffer((OMX_COMPONENTTYPE*)g_handle, 0, pInBuffers);
  if(omxresult != OMX_ErrorNone) OMX_FreeBuffer((OMX_COMPONENTTYPE*)g_handle, 2, pInBuffers1);
  if(omxresult != OMX_ErrorNone) OMX_FreeBuffer((OMX_COMPONENTTYPE*)g_handle, 1, pOutBuffers);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_GetParameter(
  OMX_IN int omx_handle,
  OMX_IN int index,
  OMX_INOUT int pCompParam)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  if(!pCompParam)
  {
    pCompParam = (int)dummy;
  }
  omxresult = OMX_GetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)index,(OMX_PTR)pCompParam);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SendCommand(
  OMX_IN int omx_handle,
  OMX_IN int cmd,
  OMX_IN OMX_U32 param,
  OMX_IN OMX_MARKTYPE cmdData)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  omxresult = OMX_SendCommand((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_COMMANDTYPE)cmd, param, &cmdData);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SendJpegCommand(
  int cmd,
  OMX_U32 param)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  omxresult = OMX_SendCommand((OMX_COMPONENTTYPE*)g_handle,
    (OMX_COMMANDTYPE)cmd, param, NULL);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_FillThisBuffer(
  OMX_IN int omx_handle,
  OMX_IN int pBufHdr)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  omxresult = OMX_FillThisBuffer((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_BUFFERHEADERTYPE*)pBufHdr);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_EmptyThisBuffer(
  OMX_IN int omx_handle,
  OMX_IN int pBufHdr)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  omxresult = OMX_EmptyThisBuffer((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_BUFFERHEADERTYPE*)pBufHdr);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_GetState(
  OMX_IN int omx_handle)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_STATETYPE state;
  omxresult = OMX_GetState((OMX_COMPONENTTYPE*)omx_handle,&state);
  PRINT_RESULT();
  return (int)state;
}

int JpegOMX_GetComponentVersion(
  OMX_IN int omx_handle,
  OMX_OUT char* componentName,
  OMX_OUT int componentVersion,
  OMX_OUT int specVersion,
  OMX_OUT int componentUUID)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_VERSIONTYPE component_Version;
  OMX_OUT OMX_VERSIONTYPE spec_Version;
  OMX_OUT OMX_UUIDTYPE component_UUID;
  omxresult = OMX_GetComponentVersion((OMX_COMPONENTTYPE*)omx_handle,
    componentName, &component_Version, &spec_Version, &component_UUID);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_GetConfig(
  OMX_IN int omx_handle,
  OMX_IN int index,
  OMX_INOUT int pCompConfig)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  if(!pCompConfig)
  {
    pCompConfig = (int)dummy;
  }
  omxresult = OMX_GetConfig((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)index,(OMX_PTR)pCompConfig);
  PRINT_RESULT();
  return (int) omxresult;
}

int JpegOMX_UseBuffer(
  OMX_IN int omx_handle,
  OMX_IN OMX_U32 portIndex,
  OMX_IN int appData,
  OMX_IN unsigned int numBytes,
  OMX_IN int  pBuf)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_BUFFERHEADERTYPE* pBufHdr;
  if(!pBuf)
  {
    pBuf = (int) malloc(numBytes);
  }
  omxresult = OMX_UseBuffer((OMX_COMPONENTTYPE*)omx_handle,&pBufHdr,
    portIndex,(OMX_PTR)appData,numBytes,(OMX_U8*)pBuf);
  PRINT_RESULT();
  return (int)pBufHdr;
}

int JpegOMX_AllUseBuffers()
{
  QOMX_BUFFER_INFO lbuffer_info;
  memset(&lbuffer_info, 0x0, sizeof(QOMX_BUFFER_INFO));

  OMX_DBG_ERROR("In %s:\n", __func__);
  lbuffer_info.fd = Buff.pmem_fd;
  OMX_UseBuffer(g_handle, &pInBuffers, 0, &lbuffer_info, size, (OMX_U8 *)Buff.addr);
  OMX_DBG_ERROR("Use Buffer inport success\n");
  lbuffer_info.fd = Buff1.pmem_fd;
  OMX_UseBuffer(g_handle, &pInBuffers1, 2, &lbuffer_info, size_thumb, (OMX_U8 *)Buff1.addr);
  OMX_DBG_ERROR("Use Buffer inport1 success\n");
  lbuffer_info.fd = OutBuff.pmem_fd;
  OMX_UseBuffer(g_handle, &pOutBuffers, 1, &lbuffer_info, size, (OMX_U8 *)OutBuff.addr);
  OMX_DBG_ERROR("Use Buffer outport success\n");

  return 0;
}

int JpegOMX_SetAllParameters()
{
  OMX_DBG_INFO("In %s:\n", __func__);
  OMX_DBG_ERROR("Not implemented yet\n");
  return 0;
}

int JpegOMX_SetParameter(
  OMX_IN int omx_handle,
  OMX_IN int index,
  OMX_IN int pCompParam)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  if(!pCompParam)
  {
    pCompParam = (int)dummy;
  }
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)index, (OMX_PTR)pCompParam);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_ImageInit(
  OMX_IN int omx_handle,
  int nSize, int nPorts, int nStartPortNumber,
  OMX_U8 nVersionMajor, OMX_U8 nVersionMinor,
  OMX_U8 nRevision, OMX_U8 nStep )
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;

  OMX_PORT_PARAM_TYPE imageinit;
  imageinit.nSize = nSize;
  imageinit.nPorts = nPorts;
  imageinit.nStartPortNumber = nStartPortNumber;
  imageinit.nVersion.s.nVersionMajor = nVersionMajor;
  imageinit.nVersion.s.nVersionMinor = nVersionMinor;
  imageinit.nVersion.s.nRevision = nRevision;
  imageinit.nVersion.s.nStep = nStep;
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_IndexParamImageInit,
    (OMX_PTR)&imageinit);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_ImagePortFormat(
  OMX_IN int omx_handle,
  int nSize, int nPortIndex,
  int nIndex, int eCompressionFormat, int eColorFormat,
  OMX_U8 nVersionMajor, OMX_U8 nVersionMinor,
  OMX_U8 nRevision, OMX_U8 nStep)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_IMAGE_PARAM_PORTFORMATTYPE imageportformat;
  imageportformat.nSize = nSize;
  imageportformat.nPortIndex = nPortIndex;
  imageportformat.nIndex = nIndex;
  imageportformat.eCompressionFormat = (OMX_IMAGE_CODINGTYPE) eCompressionFormat;
  imageportformat.eColorFormat = (OMX_COLOR_FORMATTYPE) eColorFormat;
  imageportformat.nVersion.s.nVersionMajor = nVersionMajor;
  imageportformat.nVersion.s.nVersionMinor = nVersionMinor;
  imageportformat.nVersion.s.nRevision = nRevision;
  imageportformat.nVersion.s.nStep = nStep;

  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_IndexParamImagePortFormat,
    (OMX_PTR)&imageportformat);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_PortDef(
  OMX_IN int omx_handle, int port,
  int use_default_value, OMX_U32 nPortIndex,
  OMX_U32 eDir, OMX_U32 nBufferCountActual,
  OMX_U32 nBufferCountMin, OMX_U32 nBufferSize,
  int bEnabled,  int bPopulated,  int eDomain,
  OMX_STRING cMIMEType, OMX_U32 nFrameWidth,
  OMX_U32 nFrameHeight, OMX_S32 nStride, OMX_U32 nSliceHeight,
  int bFlagErrorConcealment, OMX_U32 eCompressionFormat,
  OMX_U32 eColorFormat)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_PARAM_PORTDEFINITIONTYPE * p_port =(OMX_PARAM_PORTDEFINITIONTYPE *) malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  memset(p_port, 0x00, sizeof(*p_port));

  initialize_portdef(omx_handle, p_port, nPortIndex);

  if(!use_default_value)
  {
    p_port->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    p_port->nPortIndex = nPortIndex;
    p_port->eDir = (OMX_DIRTYPE)eDir;
    p_port->nBufferCountActual = nBufferCountActual;
    p_port->nBufferCountMin = nBufferCountMin;
    p_port->bEnabled = (OMX_BOOL)bEnabled;
    p_port->bPopulated = (OMX_BOOL)bPopulated;
    p_port->eDomain = (OMX_PORTDOMAINTYPE)eDomain;
  }
  if(nPortIndex == 0)
  {
    p_port->nBufferSize = size;
    p_port->format.image.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_QCOM_IMG_COLOR_FormatYVU420SemiPlanar;
    OMX_DBG_ERROR("size %lu\n", size);
    test_args.main.width = nFrameWidth;
    test_args.main.height = nFrameHeight;
    OMX_DBG_ERROR("portIndex1 width %d: Height %d\n", (int)nFrameWidth, (int)nFrameHeight);
  }

  p_port->nPortIndex = nPortIndex;
  /*Set input port parameters*/
  p_port->format.image.nFrameWidth = nFrameWidth;
  p_port->format.image.nFrameHeight = nFrameHeight;
  p_port->format.image.nStride = nStride;
  p_port->format.image.nSliceHeight = nSliceHeight;
  p_port->nBufferSize = size;
  OMX_DBG_INFO("Calling OMX_SetParameter for OMX_IndexParamPortDefinition port %ld\n", nPortIndex);

  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_IndexParamPortDefinition,
    (OMX_PTR)p_port);

  if (OMX_ErrorNone != omxresult) {
    goto ERROR;
  }
  omxresult = OMX_GetParameter((OMX_COMPONENTTYPE*)omx_handle,
     OMX_IndexParamPortDefinition, p_port);
  if(nPortIndex == 2)
  {
    OMX_DBG_ERROR("size_thumb %ld\n", size_thumb);
    size_thumb = p_port->nBufferSize;
    test_args.thumbnail.width = nFrameWidth;
    test_args.thumbnail.height = nFrameHeight;
    OMX_DBG_ERROR("portIndex 2 width %d: Height %d\n", (int)nFrameWidth, (int)nFrameHeight);
    OMX_DBG_ERROR("size_thumb %lu\n", size_thumb);
  }
  if(test_args.main.height >0 && test_args.main.width > 0 &&
     test_args.thumbnail.width > 0 && test_args.thumbnail.height > 0)
 {
    OMX_DBG_INFO("Calling allocate buffer\n");
    allocate_buffers(1600, 1200, 1600, 1200);
  }
  port = (int)p_port;

ERROR:
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_QFactor(
  OMX_IN int omx_handle, OMX_U32 nSize,
  OMX_U32 nPortIndex, int quality)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_IMAGE_PARAM_QFACTORTYPE mainImageQuality;
  mainImageQuality.nPortIndex = nPortIndex;
  mainImageQuality.nSize = nSize;
  mainImageQuality.nQFactor = quality;
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_IndexParamQFactor,
    (OMX_PTR)&mainImageQuality);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_Exif(
  OMX_IN int omx_handle,
  int tag_id, int tag_entry_type, int tag_entry_count, int tag_entry_copy, int tag_entry_rat_num, int tag_entry_rat_denom, char * tag_entry_ascii)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;

  QOMX_EXIF_INFO info;
  QEXIF_INFO_DATA data[10];
  OMX_INDEXTYPE omx_idx;
  uint32_t idx = 0;

  data[idx].tag_id = tag_id;
  data[idx].tag_entry.type = (exif_tag_type_t)tag_entry_type;
  data[idx].tag_entry.count = tag_entry_count;
  data[idx].tag_entry.copy = tag_entry_copy;
  data[idx].tag_entry.data._ascii = tag_entry_ascii;
  data[idx].tag_entry.data._rat.num= tag_entry_rat_num;
  data[idx].tag_entry.data._rat.denom = tag_entry_rat_denom;
  idx ++;

  info.exif_data = data;
  info.numOfEntries = idx;

  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for exif\n");
  omxresult = OMX_GetExtensionIndex(g_handle,(OMX_STRING)QOMX_IMAGE_EXT_EXIF_NAME,
                                    &omx_idx);

  if (OMX_ErrorNone != omxresult) {
    goto ERROR;
  }
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle, omx_idx,
                                (OMX_PTR)&info);

ERROR:
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_Thumbnail(
  OMX_IN int omx_handle,
  int height, int width,
  int scaling, int cropWidth,
  int cropHeight,
  int left, int top)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  QOMX_THUMBNAIL_INFO info;
  OMX_INDEXTYPE omx_idx;

  info.input_width = width;
  info.input_height = height;
  info.scaling_enabled = scaling;
  info.crop_info.nWidth = cropWidth;
  info.crop_info.nHeight = cropHeight;
  info.crop_info.nLeft = left;
  info.crop_info.nTop = top;
  info.output_width = 640;
  info.output_height = 480;

  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for thumbnail\n");
  omxresult = OMX_GetExtensionIndex(g_handle,(OMX_STRING)QOMX_IMAGE_EXT_THUMBNAIL_NAME,
                                    &omx_idx);

  if (OMX_ErrorNone != omxresult) {
    goto ERROR;
  }
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle, omx_idx,
                                (OMX_PTR)&info);

ERROR:
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_ThumbQuality(
  OMX_IN int omx_handle,
  OMX_IN int thumbnailquality_nQFactor)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_IMAGE_PARAM_QFACTORTYPE quality;
  quality.nPortIndex = 0;
  quality.nQFactor = thumbnailquality_nQFactor;

  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle, OMX_IndexParamQFactor,
                                (OMX_PTR)&quality);

ERROR:
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_BufferOffset(
  OMX_IN int omx_handle,
  int width, int height,
  int usePadding, int  rotation)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;

  /*Set buffer offset*/
  OMX_INDEXTYPE idx;
  QOMX_YUV_FRAME_INFO frame_info;
  OMX_DBG_INFO("Calling get_buffer_offset width %d height %d \n", width, height);
  get_buffer_offset( width, height,
                     &frame_info.yOffset,
                     frame_info.cbcrOffset, &size,
                     usePadding, rotation, frame_info.cbcrStartOffset, 1.5);
  OMX_DBG_INFO("Buffer offset: yOffset =%ld, cbcrOffset =%ld, cbcrStartOffset = %ld size = %ld\n",
    frame_info.yOffset, frame_info.cbcrOffset[0],frame_info.cbcrStartOffset[0], size);
  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for bufferoffset\n");
  omxresult = OMX_GetExtensionIndex(g_handle,
                        (OMX_STRING)QOMX_IMAGE_EXT_BUFFER_OFFSET_NAME,
                        &idx);
  if (OMX_ErrorNone != omxresult) {
    goto ERROR;
  }
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle, idx,
                                &frame_info);
ERROR:
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_UserPreferences(
  OMX_IN int omx_handle,
  int color_format,
  int thumbnail_color_format,
  int preference )
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_DBG_ERROR("Not implemented yet\n");
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetAllConfigs()
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_DBG_ERROR("Not implemented yet\n");

  return OMX_ErrorNone;
}

int JpegOMX_SetConfig(
  OMX_IN int omx_handle,
  OMX_IN int index,
  OMX_IN int pConfiParam)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  if(!pConfiParam)
  {
    pConfiParam = (int)dummy;
  }
  omxresult = OMX_SetConfig((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)index, (OMX_PTR)pConfiParam);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetConfig_Rotate(
  OMX_IN int omx_handle,
  OMX_U32  port_index,
  OMX_S32 rotation)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  /*Set rotation angle*/
  OMX_CONFIG_ROTATIONTYPE rotType;
  rotType.nPortIndex = port_index;
  rotType.nRotation = rotation;
  omxresult = OMX_SetConfig((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_IndexConfigCommonRotate,
    (OMX_PTR)&rotType);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetConfig_InputCrop(
  OMX_IN int omx_handle,
  OMX_U32 left, OMX_U32 top,
  OMX_U32 width, OMX_U32 height,
  OMX_U32 port_index )
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_CONFIG_RECTTYPE recttype;
  /*Set scaling parameters*/
  recttype.nLeft = left;
  recttype.nTop = top;
  recttype.nWidth = width;
  recttype.nHeight = height;
  recttype.nPortIndex = 1;

  omxresult = OMX_SetConfig((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_IndexConfigCommonInputCrop,
    (OMX_PTR)&recttype);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetConfig_OutputCrop(
  OMX_IN int omx_handle,
  OMX_U32 left, OMX_U32 top,
  OMX_U32 width, OMX_U32 height,
  OMX_U32 port_index )
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_CONFIG_RECTTYPE recttype;
  /*Set scaling parameters*/
  recttype.nLeft = left;
  recttype.nTop = top;
  recttype.nWidth = width;
  recttype.nHeight = height;
  recttype.nPortIndex = 1;

  omxresult = OMX_SetConfig((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_IndexConfigCommonOutputCrop,
    (OMX_PTR)&recttype);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_GetExtIdx(
  OMX_IN int omx_handle,
  OMX_IN char* compName)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  OMX_INDEXTYPE type;
  omxresult = OMX_GetExtensionIndex((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_STRING)compName,&type);
  PRINT_RESULT();
  return (int)type;
}

/*int JpegOMX_SetCallBacks(
  OMX_IN int omx_handle,
  OMX_IN OMX_CALLBACKTYPE* callbacks,
  OMX_IN int appData)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;

  omxresult = OMX_SetCallBacks((OMX_COMPONENTTYPE*)omx_handle,
    callbacks, (OMX_PTR)appData);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_ComponentDeInit(
  OMX_IN int omx_handle)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;

  omxresult = OMX_ComponentDeInit((OMX_COMPONENTTYPE*)omx_handle);
  PRINT_RESULT();
  return (int)omxresult;
}*/

int JpegOMX_ComponentTunnelRequest(
  OMX_IN int omx_handle,
  OMX_IN unsigned int port,
  OMX_IN int peerComponent,
  OMX_IN unsigned int peerPort,
  OMX_INOUT OMX_TUNNELSETUPTYPE tunnelSetup)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  /*
  omxresult = OMX_ComponentTunnelRequest((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_U32)port, (OMX_HANDLETYPE)peerComponent, (OMX_U32)peerPort,
    &tunnelSetup);*/
  PRINT_RESULT();
  return (int)omxresult;
}

static OMX_ERRORTYPE EventHandler(
  OMX_IN OMX_HANDLETYPE hComponent,
  OMX_IN OMX_PTR pAppData,
  OMX_IN OMX_EVENTTYPE eEvent,
  OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
  OMX_IN OMX_PTR pEventData)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  printf("\nEventHandler: Callback Received");
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE EmptyBufferDone(
  OMX_IN OMX_HANDLETYPE hComponent,
  OMX_IN OMX_PTR pAppData,
  OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  printf("\nEmptyBufferDone: Callback Received");
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE FillBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
  time_t rawtime;
  char tmp_buff[80];
  OMX_DBG_ERROR("In %s:\n", __func__);
  time(&rawtime);
  strftime(tmp_buff, sizeof(tmp_buff), OUT_FILE_PREFIX"out_%H_%M_%S.jpg", localtime(&rawtime));
  int fd = open(tmp_buff, O_RDWR|O_CREAT, 0666);
  write(fd, pBuffer->pBuffer, pBuffer->nFilledLen);
  close(fd);
  int fVal = 99;     // Return value
  signalWaitFunc("FillBufferDone", &fVal, sizeof(int));
  printf("FillBufferDone: Callback Received");
  return OMX_ErrorNone;
}

int StartEncode(void)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_EmptyThisBuffer(g_handle, pInBuffers);
  OMX_EmptyThisBuffer(g_handle, pInBuffers1);
  OMX_FillThisBuffer(g_handle, pOutBuffers);
  return 0;
}

/************Helper Functions**********/
int8_t get_buffer_offset(uint32_t width, uint32_t height, OMX_U32* p_y_offset,
                         OMX_U32* p_cbcr_offset, OMX_U32* p_buf_size, int usePadding,
                         int rotation,   OMX_U32 *p_cbcrStartOffset,
                         float chroma_wt)
{
  OMX_DBG_INFO("jpeg_encoder_get_buffer_offset");
  if ((NULL == p_y_offset) || (NULL == p_cbcr_offset))
  {
    return -1;
  }

  if (usePadding)
  {
    int cbcr_offset = 0;
    uint32_t actual_size = width*height;
    uint32_t padded_size = width * CEILING16(height);
    *p_y_offset = 0;
    *p_cbcr_offset = padded_size;
    if ((rotation == 90) || (rotation == 180))
    {
      *p_y_offset += padded_size - actual_size;
      *p_cbcr_offset += ((padded_size - actual_size) >> 1);
    }
    *p_buf_size = padded_size * chroma_wt;
  } else
  {
    *p_y_offset = 0;
    *p_cbcr_offset = 0;
    *p_cbcrStartOffset = PAD_TO_WORD(width*height);
    *p_buf_size = *p_cbcrStartOffset * chroma_wt;
  }
  return 0;
}

void readFile(const char * filename, uint8_t* buffer,int w, int h)
{
  int fd = open(filename, O_RDONLY, 0777);
  int s = (w*h*3)/2;
  buffer[0] = 100;
  buffer[100] = 11;
  *((uint16_t *) &buffer[1000]) = 10101;
  int i ;
  for (i =0; i<(s/1000);i++)
  {
    int r = read(fd, buffer+(1000*i), 1000);
  }
  close(fd);
}

void allocate_buffers(int w, int h, int w_thumbnail, int h_thumbnail)
{
  int rc = 0,c =0;
  int t = 0;
  g_data_alloc= 1;
  OMX_DBG_INFO("Allocating buffers using ion/pmem\n");
  OMX_DBG_INFO("w:%d h:%d  w_thumbnail:%d  h_thumbnail:%d\n",w, h, w_thumbnail,h_thumbnail);
  /*Allocate input buffer*/
  if (use_pmem)
  {
    Buff.size = size;
    OMX_DBG_INFO("alloc ion/pmem buffrer\n");
    Buff.addr = buffer_allocate(&Buff);
    readFile("/data/cFuzzer/cFuzzerTests/LondonTower_2M_1600x1200_full.ycrcb", (uint8_t *)Buff.addr, w, h);
    OMX_DBG_INFO("After memset\n");
  }

  OMX_DBG_INFO("Allocating buffers using ion/pmem for thumbnail\n");
  /*For thumbnail buffer*/
  if (use_pmem)
  {
    Buff1.size = size_thumb;
    Buff1.addr = buffer_allocate(&Buff1);
  }

  OMX_DBG_INFO("Reafing input file\n");
  readFile("/data/cFuzzer/cFuzzerTests/LondonTower_2M_1600x1200_full.ycrcb", (uint8_t *)Buff1.addr, w_thumbnail, h_thumbnail);
  OMX_DBG_INFO("Allocating buffers using ion/pmem for output\n");
  /*Allocate output buffer*/
  if (use_pmem)
  {
    OutBuff.size = size;
    OutBuff.addr = buffer_allocate(&OutBuff);
  }
}

int initialize_portdef(int omx_handle, OMX_PARAM_PORTDEFINITIONTYPE * port, unsigned int nPortIndex)
{
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;

  port->format.image.nFrameHeight = 3000;
  port->format.image.nFrameWidth = 4000;
  port->format.image.eCompressionFormat = OMX_IMAGE_CodingJPEG;

  port->nPortIndex = nPortIndex;

  omxresult = OMX_GetParameter((OMX_COMPONENTTYPE*)omx_handle,OMX_IndexParamPortDefinition,port);
  OMX_DBG_INFO(" nPortIndex = %u g_portdef.nBufferCountActual = %u",nPortIndex,(unsigned int)port->nBufferCountActual);
  OMX_DBG_INFO(" g_portdef.nBufferSize = %u ",(unsigned int)port->nBufferSize);
  PRINT_RESULT();
  return (int)omxresult;
}
