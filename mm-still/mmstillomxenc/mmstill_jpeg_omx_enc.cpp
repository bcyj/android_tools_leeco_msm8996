/* ========================================================================= * 
        Copyright © 2012 Qualcomm Technologies, Inc. All Rights Reserved.
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

/* KHRONOS header files */
#include "OMX_Types.h"
#include "OMX_Index.h"
#include "OMX_Core.h"
#include "OMX_Component.h"

#include "omx_debug.h"
#include "omx_jpeg_ext.h"

#ifdef __cplusplus
extern "C" {
#include "os_pmem.h"
}
#endif

#include "mmstill_jpeg_omx_enc.h"
#include "mmstillomxenc.h"

#define LOG_TAG "mmstill-Fuzz-wrapper"

#define PRINT_RESULT() DEBUG_PRINT("Fuzzing %s - Exit omxresult = %d !!",__FUNCTION__,omxresult);
OMX_HANDLETYPE g_handle;
int g_data_alloc = 0;
OMX_BUFFERHEADERTYPE* pInBuffers = NULL;
OMX_BUFFERHEADERTYPE* pInBuffers1 = NULL;
OMX_BUFFERHEADERTYPE* pOutBuffers = NULL;
omx_jpeg_pmem_info * pmem_info;
omx_jpeg_pmem_info * pmem_info1;

uint8_t * buffer = 0;
uint8_t * buffer_thumb = 0;
int size;
int size_thumb;
int usePadding =0;
uint8_t use_pmem = 1;
uint8_t* outBuffer;
uint8_t * buffer1 = 0;
uint8_t * buffer2 = 0;

int pmem_fd;
int ion_fd;
struct ion_allocation_data alloc_data;
struct ion_fd_data info_fd;

int pmem_fd1;
int ion_fd1;
struct ion_allocation_data alloc_data1;
struct ion_fd_data info_fd1;


int8_t get_buffer_offset(uint32_t width, uint32_t height,
                         int* p_y_offset, int* p_cbcr_offset,
                         int* p_buf_size, int usePadding, int rotation);

void readFile(const char * filename, uint8_t* buffer,int w, int h);

void allocate_buffers(int w, int h, int w_thumbnail, int h_thumbnail);

int initialize_portdef(int omx_handle, OMX_PARAM_PORTDEFINITIONTYPE * port, unsigned int nPortIndex);

typedef struct {
  char                *file_name;
  uint32_t             width;
  uint32_t             height;
  uint32_t             quality;
  omx_jpeg_color_format format;

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
  omx_jpeg_preference  preference;
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
  g_handle = NULL;
  if (g_data_alloc){
      OMX_DBG_ERROR("Cleaning up global data\n");
      if(pmem_info) free(pmem_info);
      if(pmem_info1) free(pmem_info1);
      os_pmem_free(pmem_fd, size, buffer);
      os_pmem_free(pmem_fd1, size_thumb, buffer_thumb);
      os_pmem_free(pmem_fd, size, outBuffer );
      os_pmem_fd_close(&pmem_fd, ion_fd,
                                  &info_fd);
      os_pmem_fd_close(&pmem_fd1,ion_fd1,
                                  &info_fd1);
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
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_DBG_ERROR("pInBuffers:%p pInBuffers1: %p pOutBuffers: %p\n", pInBuffers, pInBuffers1, pOutBuffers);
  OMX_DBG_ERROR("pmem_info:%p pmem_info1: %p \n", pmem_info, pmem_info1);
  OMX_DBG_ERROR("buffer:%p buffer_thumb: %p outBuffer: %p\n", buffer, buffer_thumb, outBuffer);
  OMX_UseBuffer(g_handle, &pInBuffers, 0, pmem_info, size, buffer);
  OMX_DBG_ERROR("Use Buffer inport success\n");
  OMX_UseBuffer(g_handle, &pInBuffers1, 2, pmem_info1, size_thumb, buffer_thumb);
  OMX_DBG_ERROR("Use Buffer inport1 success\n");
  OMX_UseBuffer(g_handle, &pOutBuffers, 1, NULL, size, outBuffer);
  OMX_DBG_ERROR("Use Buffer outport success\n");
  return 0;
}

int JpegOMX_SetAllParameters()
{
  OMX_DBG_INFO("In %s:\n", __func__);
  OMX_INDEXTYPE type;
  omx_jpeg_exif_info_tag tag;
  OMX_CONFIG_ROTATIONTYPE rotType;
  omx_jpeg_thumbnail thumbnail;
  OMX_CONFIG_RECTTYPE recttype;

  OMX_INDEXTYPE buffer_offset;
  omx_jpeg_buffer_offset bufferoffset;
  OMX_INDEXTYPE user_preferences;
  omx_jpeg_user_preferences userpreferences;
  static omx_jpeg_thumbnail_quality thumbnailQuality;
  static OMX_INDEXTYPE thumbnailQualityType;

  int w,h;
  int w_thumbnail, h_thumbnail;
  int i =0;
  int rc = 0,c =0;
  int t = 0;
  const char * filename;
  const char * filename_thumb;
  int usePadding =0;
  OMX_DBG_INFO("Allocating input port\n");
  OMX_PARAM_PORTDEFINITIONTYPE * inputPort = (OMX_PARAM_PORTDEFINITIONTYPE *)
  malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE ));
  OMX_DBG_INFO("Allocating input port1\n");
  OMX_PARAM_PORTDEFINITIONTYPE * inputPort1 =(OMX_PARAM_PORTDEFINITIONTYPE *)
  malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE ));
  OMX_DBG_INFO("Allocating output port\n");
  OMX_PARAM_PORTDEFINITIONTYPE * outputPort =(OMX_PARAM_PORTDEFINITIONTYPE *)
  malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE ));

  inputPort->nPortIndex = 0;
  inputPort1->nPortIndex = 2;
  outputPort->nPortIndex = 1;
  OMX_DBG_INFO("Calling GetParameter for input port %x\n",  g_handle);
  OMX_GetParameter(g_handle, OMX_IndexParamPortDefinition, inputPort);
  OMX_DBG_INFO("Calling GetParameter for input port1\n");
  OMX_GetParameter(g_handle, OMX_IndexParamPortDefinition, inputPort1);
  OMX_DBG_INFO("Calling GetParameter for output port\n");
  OMX_GetParameter(g_handle, OMX_IndexParamPortDefinition, outputPort);

  test_args_t test_args;
  memset(&test_args, 0, sizeof(test_args));
  test_args.main.format =  OMX_YCRCBLP_H2V2;
  test_args.main.quality = 75;
  test_args.thumbnail.format =  OMX_YCRCBLP_H2V2;
  test_args.thumbnail.quality = 75;
  test_args.back_to_back_count = 1;
  test_args.preference =  OMX_JPEG_PREF_HW_ACCELERATED_PREFERRED;
  test_args.main.width = 1600;
  test_args.main.height = 1200;
  w = test_args.main.width;
  h = test_args.main.height;
  test_args.thumbnail.width = 720;
  test_args.thumbnail.height = 480;
  test_args.encode_thumbnail = 1;
  w_thumbnail = test_args.thumbnail.width;
  h_thumbnail = test_args.thumbnail.height;

 /*Set buffer offset*/
  bufferoffset.width = test_args.main.width;
  bufferoffset.height = test_args.main.height;
  OMX_DBG_INFO("Calling get_buffer_offset\n");
  get_buffer_offset( bufferoffset.width, bufferoffset.height,
                     &bufferoffset.yOffset,
                     &bufferoffset.cbcrOffset, &bufferoffset.totalSize,
                     usePadding,test_args.rotation);
  OMX_DBG_INFO("Calling gOMX_GetExtensionIndex\n");
  OMX_GetExtensionIndex(g_handle,(char *)"omx.qcom.jpeg.exttype.buffer_offset",
                        &buffer_offset);
  OMX_DBG_INFO("Buffer offset: yOffset =%d, cbcrOffset =%d, totalSize = %d\n",
       bufferoffset.yOffset,
       bufferoffset.cbcrOffset,bufferoffset.totalSize);
  OMX_DBG_INFO("Calling OMX_SetParameter for buffer_offset\n");
  OMX_SetParameter(g_handle, buffer_offset, &bufferoffset);

  /*Set input port parameters*/
  inputPort->format.image.nFrameWidth = w;
  inputPort->format.image.nFrameHeight = h;
  inputPort->format.image.nStride = w;
  inputPort->format.image.nSliceHeight = h;
  inputPort->nBufferSize = bufferoffset.totalSize;
  size = bufferoffset.totalSize;
  OMX_DBG_INFO("Calling OMX_SetParameter for OMX_IndexParamPortDefinition input port\n");
  OMX_SetParameter(g_handle, OMX_IndexParamPortDefinition, inputPort);
  OMX_GetParameter(g_handle, OMX_IndexParamPortDefinition, inputPort);

  inputPort1->format.image.nFrameWidth = w_thumbnail;
  inputPort1->format.image.nFrameHeight = h_thumbnail;
  inputPort1->format.image.nStride = w_thumbnail;
  inputPort1->format.image.nSliceHeight = h_thumbnail;
  OMX_DBG_INFO("Calling OMX_SetParameter for OMX_IndexParamPortDefinition input port1\n");
  OMX_SetParameter(g_handle, OMX_IndexParamPortDefinition, inputPort1);
  OMX_GetParameter(g_handle, OMX_IndexParamPortDefinition, inputPort1);

  bufferoffset.width = w;
  bufferoffset.height = h;

  size_thumb = inputPort1->nBufferSize;
  allocate_buffers(w, h, w_thumbnail, h_thumbnail);

  userpreferences.color_format = test_args.main.format;
  userpreferences.thumbnail_color_format = test_args.thumbnail.format;
  userpreferences.preference = test_args.preference;
  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for user_preferences\n");
  OMX_GetExtensionIndex(g_handle,(char *)"omx.qcom.jpeg.exttype.user_preferences",
                        &user_preferences);

  OMX_SetParameter(g_handle,user_preferences,&userpreferences);

  /*set exif indo*/
  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for exif\n");
  OMX_GetExtensionIndex(g_handle, (char *)"omx.qcom.jpeg.exttype.exif", &type);
  tag.tag_id = EXIFTAGID_GPS_LONGITUDE_REF;
  printf("\ntag_id EXIFTAGID_GPS_LONGITUDE_REF %d \n", tag.tag_id);
  tag.tag_entry.type = EXIF_ASCII;
  tag.tag_entry.count = 2;
  tag.tag_entry.copy = 1;
  tag.tag_entry.data._ascii = "se";
  OMX_SetParameter(g_handle, type, &tag);

  tag.tag_id = EXIFTAGID_GPS_LONGITUDE;
  printf("\ntag_id EXIFTAGID_GPS_LONGITUDE %d \n", tag.tag_id);
  tag.tag_entry.type = EXIF_RATIONAL;
  tag.tag_entry.count = 1;
  tag.tag_entry.copy = 1;
  tag.tag_entry.data._rat.num= 31;
  tag.tag_entry.data._rat.denom = 1;
  OMX_SetParameter(g_handle, type, &tag);

  /*Set thumbnail data*/
  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for thumbnail\n");
  if (test_args.encode_thumbnail)
  {
    OMX_GetExtensionIndex(g_handle, (char *)"omx.qcom.jpeg.exttype.thumbnail", &type);

    thumbnail.width = test_args.thumbnail.width;
    thumbnail.height = test_args.thumbnail.height;
    thumbnail.scaling = 1;//test_args.tn_scale_cfg.enable;
    thumbnail.cropWidth = 3200; //test_args.tn_scale_cfg.input_width;
    thumbnail.cropHeight = 2400; //test_args.tn_scale_cfg.input_height;
    thumbnail.left = 0; //test_args.tn_scale_cfg.h_offset;
    thumbnail.top = 0;// test_args.tn_scale_cfg.v_offset;
    OMX_DBG_INFO("Calling SetParam for thumbnail ret = %d\n", OMX_SetParameter(g_handle, type, &thumbnail));
    //OMX_SetParameter(g_handle, type, &thumbnail);
  }

  /*Set Thumbnail quality*/
  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for thumbnail_quality\n");
  OMX_GetExtensionIndex(g_handle, (char *)"omx.qcom.jpeg.exttype.thumbnail_quality",
                        &thumbnailQualityType);
  OMX_GetParameter(g_handle, thumbnailQualityType, &thumbnailQuality);
  thumbnailQuality.nQFactor = test_args.thumbnail.quality;
  OMX_DBG_INFO("Setting thumbnail quality %d\n",test_args.thumbnail.quality);
  OMX_SetParameter(g_handle, thumbnailQualityType, &thumbnailQuality);

  /*Set Main image quality*/
  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for mainImageQuality\n");
  OMX_IMAGE_PARAM_QFACTORTYPE * mainImageQuality =(OMX_IMAGE_PARAM_QFACTORTYPE *)
    malloc(sizeof(OMX_IMAGE_PARAM_QFACTORTYPE));
  mainImageQuality->nPortIndex = 0;
  mainImageQuality->nQFactor = test_args.main.quality;
  OMX_DBG_INFO("Setting main image quality %d\n",test_args.main.quality);
  OMX_SetParameter(g_handle,OMX_IndexParamQFactor, mainImageQuality);
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
  p_port->nBufferSize = nBufferSize;
  OMX_DBG_INFO("Calling OMX_SetParameter for OMX_IndexParamPortDefinition port %d\n", nPortIndex);

  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_IndexParamPortDefinition,
    (OMX_PTR)p_port);
  omxresult = OMX_GetParameter((OMX_COMPONENTTYPE*)omx_handle,
     OMX_IndexParamPortDefinition, p_port);
  if(nPortIndex == 2)
  {
    size_thumb = p_port->nBufferSize;
    test_args.thumbnail.width = nFrameWidth;
    test_args.thumbnail.height = nFrameHeight;
    OMX_DBG_ERROR("portIndex 2 width %d: Height %d\n", (int)nFrameWidth, (int)nFrameHeight);
  }
  if(test_args.main.height >0 && test_args.main.width > 0 &&
     test_args.thumbnail.width > 0 && test_args.thumbnail.height > 0)
 {
    OMX_DBG_INFO("Calling allocate buffer\n");
    //allocate_buffers(test_args.main.width, test_args.main.height, test_args.thumbnail.width, test_args.thumbnail.height);
    allocate_buffers(1600, 1200, 1600, 1200);
  }
  port = (int)p_port;
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
  omx_jpeg_exif_info_tag tag;
  OMX_INDEXTYPE type;
  tag.tag_id = tag_id;
  tag.tag_entry.type = (exif_tag_type_t) tag_entry_type;
  tag.tag_entry.count = tag_entry_count;
  tag.tag_entry.copy = tag_entry_copy;
  tag.tag_entry.data._rat.num= tag_entry_rat_num;
  tag.tag_entry.data._rat.denom = tag_entry_rat_denom;
  tag.tag_entry.data._ascii = tag_entry_ascii;

  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for exif\n");
  OMX_GetExtensionIndex(g_handle, (char *)"omx.qcom.jpeg.exttype.exif", &type);
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
                (OMX_INDEXTYPE)type,
  (OMX_PTR)&tag);
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
  omx_jpeg_thumbnail thumbnail;
  thumbnail.width = width;
  thumbnail.height = height;
  thumbnail.scaling = scaling;
  thumbnail.cropWidth = cropWidth;
  thumbnail.cropHeight = cropHeight;
  thumbnail.left = left;
  thumbnail.top = top;

  //"omx.qcom.jpeg.exttype.thumbnail" == OMX_JPEG_EXT_THUMBNAIL
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_JPEG_EXT_THUMBNAIL,
    (OMX_PTR)&thumbnail);
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_ThumbQuality(
  OMX_IN int omx_handle,
  OMX_IN int thumbnailquality_nQFactor)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  omx_jpeg_thumbnail_quality thumbnailquality;
  thumbnailquality.nQFactor = thumbnailquality_nQFactor;
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_JPEG_EXT_THUMBNAIL_QUALITY,
    (OMX_PTR)&thumbnailquality);
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
  OMX_INDEXTYPE buffer_offset;
  /*Set buffer offset*/
  omx_jpeg_buffer_offset bufferoffset;
  bufferoffset.width = width;
  bufferoffset.height = height;
  OMX_DBG_INFO("Calling get_buffer_offset width %d height %d \n", width, height);
  get_buffer_offset( bufferoffset.width, bufferoffset.height,
                     &bufferoffset.yOffset,
                     &bufferoffset.cbcrOffset, &bufferoffset.totalSize,
                     usePadding,rotation);
  OMX_DBG_INFO("Buffer offset: yOffset =%d, cbcrOffset =%d, totalSize = %d\n",
       bufferoffset.yOffset,
       bufferoffset.cbcrOffset,bufferoffset.totalSize);
  OMX_GetExtensionIndex((OMX_COMPONENTTYPE*)omx_handle,(char *)"omx.qcom.jpeg.exttype.buffer_offset",
                        &buffer_offset);
  omxresult = OMX_SetParameter ((OMX_COMPONENTTYPE*)omx_handle,
       (OMX_INDEXTYPE) buffer_offset,
  (OMX_PTR) &bufferoffset);
  size = bufferoffset.totalSize;
  //"omx.qcom.jpeg.exttype.buffer_offset" == OMX_JPEG_EXT_BUFFER_OFFSET
  /*omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_JPEG_EXT_BUFFER_OFFSET,
    (OMX_PTR)&bufferoffset);*/
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetParameter_ACbCrOffset(
  OMX_IN int omx_handle,
  OMX_IN omx_jpeg_buffer_offset ACbCrOffset)
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_JPEG_EXT_ACBCR_OFFSET,
    (OMX_PTR)&ACbCrOffset);
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
  OMX_ERRORTYPE omxresult=OMX_ErrorNone;
  omx_jpeg_user_preferences userpreferences;
  OMX_INDEXTYPE user_preferences;
  userpreferences.color_format = (omx_jpeg_color_format)color_format;
  userpreferences.thumbnail_color_format = (omx_jpeg_color_format) thumbnail_color_format;
  userpreferences.preference = (omx_jpeg_preference) preference;
  OMX_DBG_INFO("Calling OMX_GetExtensionIndex for user_preferences\n");
  OMX_GetExtensionIndex((OMX_COMPONENTTYPE*)omx_handle,(char *)"omx.qcom.jpeg.exttype.user_preferences",
                        &user_preferences);
  omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
      (OMX_INDEXTYPE) user_preferences,
  (OMX_PTR) &userpreferences);
  // "omx.qcom.jpeg.exttype.user_preferences" == OMX_JPEG_EXT_USER_PREFERENCES
  /*omxresult = OMX_SetParameter((OMX_COMPONENTTYPE*)omx_handle,
    (OMX_INDEXTYPE)OMX_JPEG_EXT_USER_PREFERENCES,
    (OMX_PTR)&userpreferences); */
  PRINT_RESULT();
  return (int)omxresult;
}

int JpegOMX_SetAllConfigs()
{
  OMX_DBG_ERROR("In %s:\n", __func__);
  OMX_CONFIG_ROTATIONTYPE rotType;
  OMX_CONFIG_RECTTYPE recttype;
  /*Set rotation angle*/
  rotType.nPortIndex = 1;
  rotType.nRotation = 0;
  OMX_SetConfig(g_handle, OMX_IndexConfigCommonRotate, &rotType);

  /*Set scaling parameters*/
  recttype.nLeft = 0;// test_args.main_scale_cfg.h_offset;
  recttype.nTop = 0;//test_args.main_scale_cfg.v_offset;
  recttype.nWidth = 3200;//test_args.main_scale_cfg.input_width;
  recttype.nHeight = 2400;//test_args.main_scale_cfg.input_height;
  recttype.nPortIndex = 1;

  OMX_SetConfig(g_handle, OMX_IndexConfigCommonOutputCrop, &recttype);

  recttype.nLeft = 0;// test_args.main_scale_cfg.h_offset;
  recttype.nTop = 0;//test_args.main_scale_cfg.v_offset;
  recttype.nWidth = 1600;//test_args.main_scale_cfg.input_width;
  recttype.nHeight = 1200;//test_args.main_scale_cfg.input_height;
  recttype.nPortIndex = 1;

  OMX_SetConfig(g_handle, OMX_IndexConfigCommonInputCrop, &recttype);
  return 0;
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
  OMX_DBG_ERROR("In %s:\n", __func__);
  int fd = open("/data/cFuzzer/cFuzzerLogs/out.jpg", O_RDWR|O_CREAT);
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
int8_t get_buffer_offset(uint32_t width, uint32_t height,
                         int* p_y_offset, int* p_cbcr_offset,
                         int* p_buf_size, int usePadding, int rotation)
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
    *p_buf_size = padded_size * 3/2;
  } else
  {
    *p_y_offset = 0;
    *p_cbcr_offset = PAD_TO_WORD(width*height);
    *p_buf_size = *p_cbcr_offset * 3/2;
  }
  return 0;
}

void readFile(const char * filename, uint8_t* buffer,int w, int h)
{
  int fd = open(filename, O_RDONLY, 0777);
  int s = (w*h*3)/2;
  buffer[0] = 100;
  buffer[100] = 11;
  buffer[1000] = 10101;
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
    alloc_data.len = size;
    alloc_data.align = 4096;
    alloc_data.flags = 0x1 << ION_IOMMU_HEAP_ID;
    OMX_DBG_INFO("calling fd open ion/pmem\n");
    /*rc = os_pmem_fd_open(&pmem_fd);*/
    rc = os_pmem_fd_open(&pmem_fd, &ion_fd,
                         &alloc_data, &info_fd);
    OMX_DBG_INFO("pmem Open ret %d pmem_fd%d\n", rc, (int) pmem_fd);
    rc = os_pmem_allocate(pmem_fd, size, &buffer);
    OMX_DBG_INFO("pmem allocate ret %d size %d\n", rc, size);
    t = os_pmem_get_phy_addr(pmem_fd, &buffer1);
    OMX_DBG_INFO("pmem get_phy_addr ret %d \n", t);
    //memset(buffer, '0xFE', size);
    readFile("/data/cFuzzer/cFuzzerTests/LondonTower_2M_1600x1200_full.ycrcb", buffer, w, h);
    OMX_DBG_INFO("After memset\n");
  }
  pmem_info = (omx_jpeg_pmem_info *)malloc(sizeof(omx_jpeg_pmem_info));
  OMX_DBG_INFO("After malloc\n");
  pmem_info->fd = pmem_fd;
  pmem_info->offset = 0 ;

  OMX_DBG_INFO("Allocating buffers using ion/pmem for thumbnail\n");
  /*For thumbnail buffer*/
  if (use_pmem)
  {
    alloc_data1.len = size_thumb;
    alloc_data1.align = 4096;
    alloc_data1.flags = 0x1 << ION_IOMMU_HEAP_ID;

    rc = os_pmem_fd_open(&pmem_fd1,&ion_fd1,
                         &alloc_data1, &info_fd1);
    rc = os_pmem_allocate(pmem_fd1, size_thumb, &buffer_thumb);
    t = os_pmem_get_phy_addr(pmem_fd1, &buffer2);
  }

  readFile("/data/cFuzzer/cFuzzerTests/LondonTower_2M_1600x1200_full.ycrcb", buffer_thumb, w_thumbnail, h_thumbnail);

  OMX_DBG_INFO("Allocating buffers using ion/pmem for output\n");
  /*Allocate output buffer*/
  if (use_pmem)
  {
    rc = os_pmem_allocate(pmem_fd, size, &outBuffer );
  }

  pmem_info1 = (omx_jpeg_pmem_info *) malloc(sizeof(omx_jpeg_pmem_info));
  pmem_info1->fd = pmem_fd1;
  pmem_info1->offset = 0 ;

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
  return (int)omxresult;
}
