/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include "OMXImageEncoder.h"

#define DEFAULT_MN_ENC_KEY "-----BEGIN PUBLIC KEY-----\n\
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA20IRJeRrU2FOo8qeR9PL\n\
/VOegBS3Lj9Bhbfh5Vqhnr7upFoI4zYHLeQDYgdYTDmtZXLQ9lIuhvOXROQypSLU\n\
VbsbyM6xoq9uW4GtvDg9WC6gplM7cSP1Zp6x65X4G+P9QpeAkKhPYKPVGgOkX6IH\n\
dHZinygKd7Xb1UGgfiSPvWhYS+c+8OI6Idug9Lcn4qBvIaBsodbZLh1w0XReVDzt\n\
gO4dHIbDxRdQDEf8m6sPDDrAcpC+5X5MsJ/4H78HWSO2PRMJxQxrnWcAGJ1IKGwm\n\
k7BGlqXljlkCcSf0iRhiHjTYfWI/LTDlTsYYYkpv878IRu+LiVRg2ulcpn4ICS8O\n\
dQIDAQAB\n\
-----END PUBLIC KEY-----\n"


#define NUM_EXIF_MARKERS (3)
/*==============================================================================
* Function : OMXImageEncoder
* Parameters: None
* Return Value : None
* Description: Constructor
==============================================================================*/
OMXImageEncoder::OMXImageEncoder()
{
  m_inputBufferCount = 0;
  m_outputBufferCount = 0;
  m_composer = 0;
  m_mobicatComposer = 0;
  m_thumbEncoding = 0;
  m_mainImageEncoding = 0;
  m_mainEncoder = NULL;
  m_thumbEncoder = NULL;
  m_inputMainImage = NULL;
  m_outputMainImage = NULL;
  m_IONBuffer.vaddr = NULL;
  m_IONBuffer.fd = -1;
  m_inThumbImage = NULL;
  m_outThumbImage = NULL;
  m_qualityfactor.nQFactor = DEFAULT_Q_FACTOR;
  mThumbBuffer = NULL;
  mExifObjInitialized = OMX_FALSE;
  m_releaseFlag = OMX_FALSE;
  m_inTmbPort = NULL;
  m_inputTmbFormatTypes = NULL;
  m_inTmbBuffAllocCount = 0;
  mInTmbOMXBufferData = NULL;
  m_encoding_mode = OMX_Serial_Encoding;
  m_thumbEncodingComplete = OMX_FALSE;
  m_MetadataEncKey.metaKey = (OMX_U8 *) DEFAULT_MN_ENC_KEY;
  m_MetadataEncKey.keyLen = strlen(DEFAULT_MN_ENC_KEY);
  mNumExifMarkers = NUM_EXIF_MARKERS;


  pthread_mutex_init(&m_etbTmbQLock, NULL);
  QI_MUTEX_INIT(&mEncodeDoneLock);
}

/*==============================================================================
* Function : ~OMXImageEncoder
* Parameters: None
* Return Value : None
* Description: Destructor
==============================================================================*/
OMXImageEncoder::~OMXImageEncoder()
{
  m_inputBufferCount = 0;
  m_outputBufferCount = 0;
  m_composer = 0;
  m_mobicatComposer = 0;
  m_thumbEncoding = 0;

  if (m_inTmbPort) {
    delete m_inTmbPort;
  }
  if (m_inputTmbFormatTypes) {
    delete m_inputTmbFormatTypes;
  }

  pthread_mutex_destroy(&m_etbTmbQLock);
  QI_MUTEX_DESTROY(&mEncodeDoneLock);
}

/*==============================================================================
* Function : initializeOutputPort
* Parameters: OMX_PARAM_PORTDEFINITIONTYPE
* Return Value : OMX_ERRORTYPE
* Description: Initialize the Output port with default values
==============================================================================*/
void OMXImageEncoder::initializeOutputPort(
  OMX_PARAM_PORTDEFINITIONTYPE *a_outPort)
{
  a_outPort->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  a_outPort->nVersion.nVersion = OMX_SPEC_VERSION;
  a_outPort->eDir = OMX_DirOutput;
  a_outPort->nBufferCountActual = 1;
  a_outPort->nBufferCountMin = 1;
  a_outPort->bEnabled = OMX_TRUE;
  a_outPort->bPopulated = OMX_FALSE;
  a_outPort->eDomain = OMX_PortDomainImage;
  a_outPort->bBuffersContiguous = OMX_TRUE;
  a_outPort->nBufferAlignment = 4096;
  a_outPort->format.image.cMIMEType = const_cast<char *>("JPEG");
  a_outPort->format.image.nFrameWidth = 0;
  a_outPort->format.image.nFrameHeight = 0;
  a_outPort->format.image.nStride = 0;
  a_outPort->format.image.nSliceHeight = 0;
  a_outPort->format.image.bFlagErrorConcealment = OMX_FALSE;
  a_outPort->format.image.eColorFormat = OMX_COLOR_FormatUnused;
  a_outPort->format.image.eCompressionFormat = OMX_IMAGE_CodingJPEG;
  a_outPort->nBufferSize = m_inPort->nBufferSize;

  return;
}
/*==============================================================================
* Function : initializeInputPort
* Parameters: OMX_PARAM_PORTDEFINITIONTYPE
* Return Value : None
* Description: Initialize the Input port with default values
==============================================================================*/
void OMXImageEncoder::initializeInputPort(
  OMX_PARAM_PORTDEFINITIONTYPE *a_inPort)
{
  a_inPort->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  a_inPort->nVersion.nVersion = OMX_SPEC_VERSION;
  a_inPort->eDir = OMX_DirInput;
  a_inPort->nBufferCountActual = 1;
  a_inPort->nBufferCountMin = 1;
  a_inPort->bEnabled = OMX_TRUE;
  a_inPort->bPopulated = OMX_FALSE;
  a_inPort->eDomain = OMX_PortDomainImage;
  a_inPort->bBuffersContiguous = OMX_TRUE;
  a_inPort->nBufferAlignment = 4096;
  a_inPort->format.image.cMIMEType = NULL;
  a_inPort->format.image.bFlagErrorConcealment = OMX_FALSE;
  a_inPort->format.image.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  a_inPort->format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;

  return;
}

/*==============================================================================
* Function : omx_component_init
* Parameters: OMX_HANDLETYPE hComp
* Return Value : OMX_ERRORTYPE
* Description: Initialize the OMX Encoder Component with relevant data
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_init(OMX_IN OMX_HANDLETYPE hComp)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  //Create and Initialize the Image Port Param Structure
  m_imagePortParam = new OMX_PORT_PARAM_TYPE();
  if (!m_imagePortParam) {
    QIDBG_ERROR("%s:%d] Failed to allocate ImagePortParam", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_imagePortParam, 0, sizeof(OMX_PORT_PARAM_TYPE));
  m_imagePortParam->nPorts = NUM_OF_PORTS;
  m_imagePortParam->nStartPortNumber = OMX_INPUT_PORT_INDEX;

  //Initilalize the Input and Output Format types.
  m_inputFormatTypes = new OMX_IMAGE_PARAM_PORTFORMATTYPE();
  if (!m_inputFormatTypes) {
    QIDBG_ERROR("%s:%d] Failed to allocate inputFormatTypes", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_inputFormatTypes, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
  m_inputFormatTypes->nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
  m_inputFormatTypes->nVersion.nVersion = OMX_SPEC_VERSION;
  m_inputFormatTypes->nIndex = 0;
  m_inputFormatTypes->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  m_inputFormatTypes->eCompressionFormat = OMX_IMAGE_CodingUnused;

  //Thumbnail imput port
  m_inputTmbFormatTypes = new OMX_IMAGE_PARAM_PORTFORMATTYPE();
  if (!m_inputTmbFormatTypes) {
    QIDBG_ERROR("%s:%d] Failed to allocate inputFormatTypes", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_inputTmbFormatTypes, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
  m_inputTmbFormatTypes->nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
  m_inputTmbFormatTypes->nVersion.nVersion = OMX_SPEC_VERSION;
  m_inputTmbFormatTypes->nIndex = 0;
  m_inputTmbFormatTypes->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  m_inputTmbFormatTypes->eCompressionFormat = OMX_IMAGE_CodingUnused;

  m_outputFormatTypes = new OMX_IMAGE_PARAM_PORTFORMATTYPE();
  if (!m_outputFormatTypes) {
    QIDBG_ERROR("%s:%d] Failed to allocate outputFormatTypes", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_outputFormatTypes, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
  m_outputFormatTypes->nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
  m_outputFormatTypes->nVersion.nVersion = OMX_SPEC_VERSION;
  m_outputFormatTypes->nIndex = 0;
  m_outputFormatTypes->eColorFormat = OMX_COLOR_FormatUnused;
  m_outputFormatTypes->eCompressionFormat = OMX_IMAGE_CodingJPEG;

  //Initilalize the Input Port
  m_inPort = new OMX_PARAM_PORTDEFINITIONTYPE();
  if (!m_inPort) {
    QIDBG_ERROR("%s:%d] Failed to allocate inPort", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_inPort, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  m_inPort->nPortIndex = OMX_INPUT_PORT_INDEX;
  initializeInputPort(m_inPort);

  //Initialize the Thumbnail Input Port
  m_inTmbPort = new OMX_PARAM_PORTDEFINITIONTYPE();
  if (!m_inTmbPort) {
    QIDBG_ERROR("%s:%d] Failed to allocate TmbinPort", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_inTmbPort, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  m_inTmbPort->nPortIndex = OMX_INPUT_THUMBNAIL_PORT_INDEX;
  initializeInputPort(m_inTmbPort);
  m_inTmbPort->bEnabled = OMX_FALSE;


  //Initilaize the Output Port
  m_outPort = new OMX_PARAM_PORTDEFINITIONTYPE();
  if (!m_outPort) {
    QIDBG_ERROR("%s:%d] Failed to allocate outPort", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_outPort, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  m_outPort->nPortIndex = OMX_OUTPUT_PORT_INDEX;
  initializeOutputPort(m_outPort);

  //No data has been allocated yet
  m_dataAllocated = OMX_FALSE;

  //Spawn a new message thread
  m_messageThread = QIThread();
  m_messageThread.StartThread(this, TRUE);
  m_messageThread.setThreadName("OMX_ImgEnc");

  //Change state to loaded
  m_state = OMX_StateLoaded;
  m_compTransState = OMX_StateNone;
  m_inportTransState = OMX_PORT_NONE;
  m_inTmbPortTransState = OMX_PORT_NONE;
  m_outportTransState = OMX_PORT_NONE;

  m_compInitialized = OMX_TRUE;
  return lret;
}

/*==============================================================================
* Function : omx_component_get_parameter
* Parameters: hComp, paramIndex, paramData
* Return Value : OMX_ERRORTYPE
* Description: Get the specified parameter
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_get_parameter(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE paramIndex,
  OMX_INOUT OMX_PTR paramData)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lindex = (int)paramIndex;

  if((hComp == NULL) || (paramData == NULL)) {
    QIDBG_ERROR("%s : Bad parameter", __func__);
    return OMX_ErrorBadParameter;
  }
  if(m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s : State Invalid", __func__);
    return OMX_ErrorInvalidState;
  }
  pthread_mutex_lock(&m_compLock);
  switch(lindex) {
  case OMX_IndexParamPortDefinition: {
    OMX_PARAM_PORTDEFINITIONTYPE *ldestPort =
      reinterpret_cast<OMX_PARAM_PORTDEFINITIONTYPE *>(paramData);
    OMX_PARAM_PORTDEFINITIONTYPE *lPort = getPortDef(ldestPort->nPortIndex);
    if (NULL != lPort) {
      memcpy(ldestPort, lPort, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    } else {
      QIDBG_ERROR("%s: Invalid port Index", __func__);
      lret = OMX_ErrorNoMore;
    }
    break;
  }
  case OMX_IndexParamQFactor: {
    OMX_IMAGE_PARAM_QFACTORTYPE *lqualityFactor =
      reinterpret_cast <OMX_IMAGE_PARAM_QFACTORTYPE *>(paramData);
    memcpy(lqualityFactor, &m_qualityfactor,
      sizeof(OMX_IMAGE_PARAM_QFACTORTYPE));
    break;
  }
  case OMX_IndexParamImageInit: {
    OMX_PORT_PARAM_TYPE *l_destPortparam =
     reinterpret_cast <OMX_PORT_PARAM_TYPE *>(paramData);
    memcpy(l_destPortparam, m_imagePortParam, sizeof(OMX_PORT_PARAM_TYPE));
    break;
  }
  case OMX_IndexParamImagePortFormat: {
    OMX_IMAGE_PARAM_PORTFORMATTYPE *l_destImagePortFormat =
     reinterpret_cast <OMX_IMAGE_PARAM_PORTFORMATTYPE *>(paramData);
    if (((l_destImagePortFormat->nPortIndex == (OMX_U32)OMX_INPUT_PORT_INDEX)) ||
        (l_destImagePortFormat->nPortIndex == (OMX_U32)OMX_INPUT_THUMBNAIL_PORT_INDEX)){
      memcpy(l_destImagePortFormat, m_inputFormatTypes,
        sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    } else if (l_destImagePortFormat->nPortIndex ==
      (OMX_U32)OMX_OUTPUT_PORT_INDEX) {
      memcpy(l_destImagePortFormat, m_outputFormatTypes,
        sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    } else {
      QIDBG_ERROR("Invalid Port Index");
      lret = OMX_ErrorNoMore;
    }
    break;
  }
  case OMX_IndexParamHuffmanTable: {
    OMX_IMAGE_PARAM_HUFFMANTTABLETYPE *lhuffmanTable =
      reinterpret_cast <OMX_IMAGE_PARAM_HUFFMANTTABLETYPE *> (paramData);
    memcpy(lhuffmanTable, &m_huffmanTable,
      sizeof(OMX_IMAGE_PARAM_HUFFMANTTABLETYPE));
    break;
  }
  case OMX_IndexParamQuantizationTable: {
    OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE *lquantTable =
      reinterpret_cast <OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE *> (paramData);
    memcpy(lquantTable, &m_quantTable,
      sizeof(OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE));
    break;
  }
  case QOMX_IMAGE_EXT_BUFFER_OFFSET: {
    QOMX_YUV_FRAME_INFO *lbufferOffset =
      reinterpret_cast <QOMX_YUV_FRAME_INFO *> (paramData);
    memcpy(lbufferOffset, &m_imageBufferOffset, sizeof(QOMX_YUV_FRAME_INFO));
    break;
  }
  case QOMX_IMAGE_EXT_THUMBNAIL: {
    QOMX_THUMBNAIL_INFO *lthumbnailInfo =
      reinterpret_cast <QOMX_THUMBNAIL_INFO *> (paramData);
    memcpy(lthumbnailInfo, &m_thumbnailInfo, sizeof(QOMX_THUMBNAIL_INFO));
    break;
  }
  case QOMX_IMAGE_EXT_ENCODING_MODE: {
    QOMX_ENCODING_MODE *lEncodingMode =
      reinterpret_cast <QOMX_ENCODING_MODE *> (paramData);
    memcpy(lEncodingMode, &m_encoding_mode, sizeof(QOMX_ENCODING_MODE));
    break;
  }
  case QOMX_IMAGE_EXT_EXIF: {
    lret = OMX_ErrorNotImplemented;
    QIDBG_ERROR("%s: Not supported for QOMX_IMAGE_EXT_EXIF", __func__);
    break;
  }
  case QOMX_IMAGE_EXT_MOBICAT: {
    lret = OMX_ErrorNotImplemented;
    break;
  }
  case QOMX_IMAGE_EXT_MEM_OPS: {
    QOMX_MEM_OPS *lmemOps =
      reinterpret_cast <QOMX_MEM_OPS *> (paramData);
    memcpy(lmemOps, &m_memOps, sizeof(QOMX_MEM_OPS));
    break;
  }
  case QOMX_IMAGE_EXT_JPEG_SPEED: {
    QOMX_JPEG_SPEED *lJpegSpeed =
      reinterpret_cast <QOMX_JPEG_SPEED *> (paramData);
    memcpy(lJpegSpeed, &m_JpegSpeedMode, sizeof(*lJpegSpeed));
    break;
  }
  default: {
    QIDBG_ERROR("%s:%d] Unknown Parameter %d", __func__, __LINE__, paramIndex);
    lret = OMX_ErrorBadParameter;
    break;
  }
  }
  pthread_mutex_unlock(&m_compLock);
  return lret;
}

/*==============================================================================
* Function : setExifData
* Parameters: None
* Return Value : int - result
* Description: Initialize the exif Info Object and set the exif Data.
==============================================================================*/
int OMXImageEncoder::setExifData(QOMX_EXIF_INFO *a_exifInfo)
{
  int lrc = 0;
  if (!mExifObjInitialized) {
      lrc = exif_init(&m_exifInfoObj);
      if (lrc) {
        QIDBG_ERROR("%s:%d] Error Initializing the Exif Info Object", __func__,
          __LINE__);
        return lrc;
      }
     mExifObjInitialized = OMX_TRUE;
  }

  for (uint32_t i =0; i < a_exifInfo->numOfEntries; i++) {
    exif_set_tag(m_exifInfoObj, a_exifInfo->exif_data[i].tag_id,
      &(a_exifInfo->exif_data[i].tag_entry));
  }
  return lrc;
}

/*==============================================================================
* Function : omx_component_set_parameter
* Parameters: hComp, paramIndex, paramData
* Return Value : OMX_ERRORTYPE
* Description: Set the specified parameter with the value
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_set_parameter(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE paramIndex,
  OMX_IN OMX_PTR paramData)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lindex = (int)paramIndex;

  QIDBG_MED("%s %d:] ParamIndex: %d", __func__, __LINE__,
    (int)paramIndex);
  if ((hComp == NULL) || (paramData == NULL)) {
    QIDBG_ERROR("%s : Bad parameter", __func__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s : Component in Invalid State. current state = %d",
      __func__, m_state);
    return OMX_ErrorInvalidState;
  }
  if ((m_state != OMX_StateLoaded) && (m_state != OMX_StateWaitForResources)) {
    QIDBG_ERROR("%s : Component not in the right state. current state = %d",
      __func__, m_state);
    return OMX_ErrorIncorrectStateOperation;
  }

  pthread_mutex_lock(&m_compLock);

  switch(lindex) {
  case OMX_IndexParamPortDefinition: {
    OMX_PARAM_PORTDEFINITIONTYPE *ldestPort =
      reinterpret_cast <OMX_PARAM_PORTDEFINITIONTYPE *> (paramData);
    OMX_PARAM_PORTDEFINITIONTYPE *lPort = getPortDef(ldestPort->nPortIndex);

    if (NULL != lPort) {
      if ((ldestPort->nPortIndex == (OMX_U32)OMX_INPUT_PORT_INDEX) ||
         (ldestPort->nPortIndex == (OMX_U32)OMX_INPUT_PORT_INDEX)) {
        if ((ldestPort->format.image.nFrameWidth > MAX_IMAGE_WIDTH) ||
            (ldestPort->format.image.nFrameHeight > MAX_IMAGE_HEIGHT)) {
          QIDBG_ERROR("%s: Width/Height exceeds max width = %d height =%d",
              __func__, (int)ldestPort->format.image.nFrameWidth,
              (int)ldestPort->format.image.nFrameHeight);
          return OMX_ErrorUnsupportedSetting;
        }
        lPort->format.image.nStride = ldestPort->format.image.nFrameWidth;
        QIDBG_MED("%s:%d]: buffer size id %d", __func__, __LINE__,
            (int)lPort->nBufferSize);
      }
      memcpy(lPort, ldestPort, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    } else {
      QIDBG_ERROR("%s: Invalid port Index", __func__);
      lret = OMX_ErrorNoMore;
    }
    break;
  }
  case OMX_IndexParamQFactor: {
    OMX_IMAGE_PARAM_QFACTORTYPE* lqualityFactor =
      reinterpret_cast <OMX_IMAGE_PARAM_QFACTORTYPE*> (paramData);
    memcpy(&m_qualityfactor, lqualityFactor,
      sizeof(OMX_IMAGE_PARAM_QFACTORTYPE));
    break;
  }
  case OMX_IndexParamImageInit: {
    OMX_PORT_PARAM_TYPE *l_destPortparam =
      reinterpret_cast <OMX_PORT_PARAM_TYPE *>(paramData);
    memcpy(m_imagePortParam, l_destPortparam, sizeof(OMX_PORT_PARAM_TYPE));
    break;
  }
  case OMX_IndexParamImagePortFormat: {
    OMX_IMAGE_PARAM_PORTFORMATTYPE *l_destImagePortFormat =
     reinterpret_cast <OMX_IMAGE_PARAM_PORTFORMATTYPE *>(paramData);
    if ((l_destImagePortFormat->nPortIndex == (OMX_U32)OMX_INPUT_PORT_INDEX)) {
      memcpy(m_inputFormatTypes, l_destImagePortFormat,
        sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    } else if (l_destImagePortFormat->nPortIndex ==
      (OMX_U32)OMX_INPUT_THUMBNAIL_PORT_INDEX) {
      memcpy(m_inputTmbFormatTypes, l_destImagePortFormat,
        sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    } else if (l_destImagePortFormat->nPortIndex ==
      (OMX_U32)OMX_OUTPUT_PORT_INDEX) {
      memcpy(m_outputFormatTypes, l_destImagePortFormat,
        sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    } else {
      QIDBG_ERROR("Invalid Port Index");
      lret = OMX_ErrorNoMore;
    }
  break;
  }
  case OMX_IndexParamHuffmanTable: {
    OMX_IMAGE_PARAM_HUFFMANTTABLETYPE* lhuffmanTable =
      reinterpret_cast <OMX_IMAGE_PARAM_HUFFMANTTABLETYPE *> (paramData);
    memcpy(&m_huffmanTable, lhuffmanTable,
      sizeof(OMX_IMAGE_PARAM_HUFFMANTTABLETYPE));
    break;
  }
  case OMX_IndexParamQuantizationTable: {
    OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE* lquantTable =
      reinterpret_cast <OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE *> (paramData);
    memcpy(&m_quantTable, lquantTable,
      sizeof(OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE));
    break;
  }
  case QOMX_IMAGE_EXT_BUFFER_OFFSET: {
    QOMX_YUV_FRAME_INFO *lbufferOffset =
      reinterpret_cast <QOMX_YUV_FRAME_INFO *> (paramData);
    memcpy(&m_imageBufferOffset, lbufferOffset, sizeof(QOMX_YUV_FRAME_INFO));
    break;
  }
  case QOMX_IMAGE_EXT_THUMBNAIL: {
    QOMX_THUMBNAIL_INFO *lthumbnailInfo =
      reinterpret_cast <QOMX_THUMBNAIL_INFO *> (paramData);
    memcpy(&m_thumbnailInfo, lthumbnailInfo, sizeof(QOMX_THUMBNAIL_INFO));
    break;
    }
  case QOMX_IMAGE_EXT_ENCODING_MODE: {
    QOMX_ENCODING_MODE *lEncodingMode =
      reinterpret_cast <QOMX_ENCODING_MODE *> (paramData);
    memcpy(&m_encoding_mode, lEncodingMode, sizeof(QOMX_ENCODING_MODE));
    break;
  }
  case QOMX_IMAGE_EXT_EXIF: {
    QOMX_EXIF_INFO *lexifInfo =
      reinterpret_cast <QOMX_EXIF_INFO *> (paramData);
    QIDBG_MED("%s: %d]: Num of exif entries = %d", __func__, __LINE__,
      (int)lexifInfo->numOfEntries);
    int lrc = setExifData(lexifInfo);
    if (lrc) {
       lret = OMX_ErrorUndefined;
    }
    break;
  }
  case QOMX_IMAGE_EXT_MEM_OPS: {
    QOMX_MEM_OPS *lmemOps =
      reinterpret_cast <QOMX_MEM_OPS *> (paramData);
    memcpy(&m_memOps, lmemOps, sizeof(m_memOps));
    break;
  }
  case QOMX_IMAGE_EXT_MOBICAT: {
   break;
   //ToDo
  }
  case QOMX_IMAGE_EXT_JPEG_SPEED: {
    QOMX_JPEG_SPEED *lJpegSpeed =
      reinterpret_cast <QOMX_JPEG_SPEED *> (paramData);
    memcpy(&m_JpegSpeedMode, lJpegSpeed, sizeof(m_JpegSpeedMode));
    break;
  }
  default: {
    QIDBG_ERROR("%s: Unknown Parameter %d", __func__, paramIndex);
    lret = OMX_ErrorBadParameter;
    break;
  }
  }
  pthread_mutex_unlock(&m_compLock);
  return lret;
}

/*==============================================================================
* Function : omx_component_get_config
* Parameters: hComp, configIndex, configData
* Return Value : OMX_ERRORTYPE
* Description: Get the requested configuration
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_get_config(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE configIndex,
  OMX_INOUT OMX_PTR configData)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lindex = (int)configIndex;
  QIDBG_MED("%s %d]", __func__, __LINE__);
  if((hComp == NULL) || (configData == NULL)) {
    QIDBG_ERROR("%s: Bad parameter", __func__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Error: Invalid State", __func__);
    return OMX_ErrorInvalidState;
  }
  pthread_mutex_lock(&m_compLock);
  switch (lindex) {
  case OMX_IndexConfigCommonInputCrop: {
    OMX_CONFIG_RECTTYPE *linScaleData =
      reinterpret_cast<OMX_CONFIG_RECTTYPE *>(configData);
    memcpy(linScaleData, &m_inputScaleInfo, sizeof(OMX_CONFIG_RECTTYPE));
    break;
  }
  case OMX_IndexConfigCommonOutputCrop: {
    OMX_CONFIG_RECTTYPE *loutScaleData =
      reinterpret_cast<OMX_CONFIG_RECTTYPE *>(configData);
    memcpy(loutScaleData, &m_outputScaleInfo, sizeof(OMX_CONFIG_RECTTYPE));
    break;
  }
  case OMX_IndexConfigCommonRotate: {
    OMX_CONFIG_ROTATIONTYPE *lrotation =
      reinterpret_cast<OMX_CONFIG_ROTATIONTYPE *>(configData);
    memcpy(lrotation, &m_rotation, sizeof(OMX_CONFIG_ROTATIONTYPE));
    break;
  }
  case QOMX_IMAGE_EXT_WORK_BUFFER: {
    QOMX_WORK_BUFFER *lbuffer =
      reinterpret_cast<QOMX_WORK_BUFFER *>(configData);
    memcpy(lbuffer, &m_IONBuffer, sizeof(QOMX_WORK_BUFFER));
    break;
  }
  case OMX_IndexParamQFactor: {
    OMX_IMAGE_PARAM_QFACTORTYPE *lqualityFactor =
      reinterpret_cast <OMX_IMAGE_PARAM_QFACTORTYPE *>(configData);
    memcpy(lqualityFactor, &m_qualityfactor,
      sizeof(OMX_IMAGE_PARAM_QFACTORTYPE));
    break;
  }
  case QOMX_IMAGE_EXT_EXIF: {
   QIDBG_ERROR("%s: Not supported for QOMX_IMAGE_EXT_EXIF", __func__);
   lret = OMX_ErrorNotImplemented;
    break;
  }
  case OMX_IndexParamHuffmanTable: {
    OMX_IMAGE_PARAM_HUFFMANTTABLETYPE *lhuffmanTable =
      reinterpret_cast <OMX_IMAGE_PARAM_HUFFMANTTABLETYPE *> (configData);
    memcpy(lhuffmanTable, &m_huffmanTable,
      sizeof(OMX_IMAGE_PARAM_HUFFMANTTABLETYPE));
    break;
  }
  case OMX_IndexParamQuantizationTable: {
    OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE *lquantTable =
      reinterpret_cast <OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE *> (configData);
    memcpy(lquantTable, &m_quantTable,
      sizeof(OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE));
    break;
  }
  case QOMX_IMAGE_EXT_THUMBNAIL: {
    QOMX_THUMBNAIL_INFO *lthumbnailInfo =
      reinterpret_cast <QOMX_THUMBNAIL_INFO *> (configData);
    memcpy(lthumbnailInfo, &m_thumbnailInfo, sizeof(QOMX_THUMBNAIL_INFO));
    break;
  }
  case QOMX_IMAGE_EXT_MOBICAT: {
    lret = OMX_ErrorNotImplemented;
    break;
  }
  case QOMX_IMAGE_EXT_METADATA: {
    QOMX_METADATA *lMetadata =
        reinterpret_cast <QOMX_METADATA *> (configData);
    memcpy(lMetadata, &m_Metadata, sizeof(m_Metadata));
    break;
  }
  case QOMX_IMAGE_EXT_META_ENC_KEY: {
    QOMX_META_ENC_KEY *lMetaKey =
        reinterpret_cast <QOMX_META_ENC_KEY *> (configData);
    memcpy(lMetaKey, &m_MetadataEncKey, sizeof(m_MetadataEncKey));
    break;
  }

  default: {
    QIDBG_ERROR("%s: Error bad config index %d", __func__,
      (int)configIndex);
    lret = OMX_ErrorBadParameter;
    break;
  }
  }
  pthread_mutex_unlock(&m_compLock);
  return lret;
}

/*==============================================================================
* Function : omx_component_set_config
* Parameters: hComp, configIndex, configData
* Return Value : OMX_ERRORTYPE
* Description: Set parameters to configue the omx component
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_set_config(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE configIndex,
  OMX_IN OMX_PTR configData)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lindex = (int)configIndex;

  if((hComp == NULL) || (configData == NULL)) {
    QIDBG_ERROR("%s: Bad parameter", __func__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Error: Invalid State", __func__);
    return OMX_ErrorInvalidState;
  }
  pthread_mutex_lock(&m_compLock);
  switch (lindex) {
  case OMX_IndexConfigCommonInputCrop: {
    OMX_CONFIG_RECTTYPE *linScaleData =
      reinterpret_cast<OMX_CONFIG_RECTTYPE *>(configData);
    memcpy(&m_inputScaleInfo, linScaleData, sizeof(OMX_CONFIG_RECTTYPE));
    break;
  }
  case OMX_IndexConfigCommonOutputCrop: {
    OMX_CONFIG_RECTTYPE *loutScaleData =
      reinterpret_cast<OMX_CONFIG_RECTTYPE *>(configData);
    memcpy(&m_outputScaleInfo, loutScaleData, sizeof(OMX_CONFIG_RECTTYPE));
    break;
  }
  case OMX_IndexConfigCommonRotate: {
    OMX_CONFIG_ROTATIONTYPE *lrotation =
      reinterpret_cast<OMX_CONFIG_ROTATIONTYPE *>(configData);
    memcpy(&m_rotation, lrotation, sizeof(OMX_CONFIG_ROTATIONTYPE));
    break;
  }
  case QOMX_IMAGE_EXT_WORK_BUFFER: {
    QOMX_WORK_BUFFER *lbuffer =
      reinterpret_cast<QOMX_WORK_BUFFER *>(configData);
    memcpy(&m_IONBuffer, lbuffer, sizeof(QOMX_WORK_BUFFER));
    break;
  }
  case OMX_IndexParamQFactor: {
    OMX_IMAGE_PARAM_QFACTORTYPE* lqualityFactor =
      reinterpret_cast <OMX_IMAGE_PARAM_QFACTORTYPE*> (configData);
    memcpy(&m_qualityfactor, lqualityFactor,
      sizeof(OMX_IMAGE_PARAM_QFACTORTYPE));
    break;
  }
  case QOMX_IMAGE_EXT_EXIF: {
    QOMX_EXIF_INFO *lexifInfo =
      reinterpret_cast <QOMX_EXIF_INFO *> (configData);
    QIDBG_MED("%s: %d]: Num of exif entries = %d", __func__, __LINE__,
      (int)lexifInfo->numOfEntries);
    int lrc = setExifData(lexifInfo);
    if (lrc) {
       lret = OMX_ErrorUndefined;
    }
    break;
  }
    case OMX_IndexParamHuffmanTable: {
    OMX_IMAGE_PARAM_HUFFMANTTABLETYPE* lhuffmanTable =
      reinterpret_cast <OMX_IMAGE_PARAM_HUFFMANTTABLETYPE *> (configData);
    memcpy(&m_huffmanTable, lhuffmanTable,
      sizeof(OMX_IMAGE_PARAM_HUFFMANTTABLETYPE));
    break;
  }
  case OMX_IndexParamQuantizationTable: {
    OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE* lquantTable =
      reinterpret_cast <OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE *> (configData);
    memcpy(&m_quantTable, lquantTable,
      sizeof(OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE));
    break;
  }
  case QOMX_IMAGE_EXT_THUMBNAIL: {
    QOMX_THUMBNAIL_INFO *lthumbnailInfo =
      reinterpret_cast <QOMX_THUMBNAIL_INFO *> (configData);
    memcpy(&m_thumbnailInfo, lthumbnailInfo, sizeof(QOMX_THUMBNAIL_INFO));
    break;
  }
  case QOMX_IMAGE_EXT_MOBICAT: {
   break;
   //ToDo
  }
  case QOMX_IMAGE_EXT_METADATA: {
    QOMX_METADATA *lMetadata =
        reinterpret_cast <QOMX_METADATA *> (configData);
    memcpy(&m_Metadata, lMetadata, sizeof(m_Metadata));
    break;
  }
  case QOMX_IMAGE_EXT_META_ENC_KEY: {
    QOMX_META_ENC_KEY *lMetaKey =
        reinterpret_cast <QOMX_META_ENC_KEY *> (configData);
    memcpy(&m_MetadataEncKey, lMetaKey, sizeof(m_MetadataEncKey));
    break;
  }
  default: {
    QIDBG_ERROR("%s: Error bad config index %d", __func__,
      (int)configIndex);
    lret = OMX_ErrorBadParameter;
    break;
  }

  } /* end of switch case*/
  pthread_mutex_unlock(&m_compLock);
  return lret;
}

/*==============================================================================
* Function : getPortDef
* Parameters: @eIdx: Port index
* Return Value : OMX_PARAM_PORTDEFINITIONTYPE *
* Description: Get pointer to the port definition corresponding
*              to the given index
==============================================================================*/
OMX_PARAM_PORTDEFINITIONTYPE *OMXImageEncoder::getPortDef(OMX_U32 eIdx)
{
  port_index ePort = (port_index) eIdx;
  switch (ePort) {
    case OMX_INPUT_PORT_INDEX:
      return m_inPort;
      break;
    case OMX_OUTPUT_PORT_INDEX:
      return m_outPort;
      break;
    case OMX_INPUT_THUMBNAIL_PORT_INDEX:
      return m_inTmbPort;
      break;
    default:
      QIDBG_ERROR("%s: Unknown port index %lu", __func__, eIdx);
      return NULL;
  }
}

/*==============================================================================
* Function : use_input_buffer
* Parameters: bufferHdr, bytes, buffer
* Return Value : OMX_ERRORTYPE
* Description: Set the Input buffers passed by the OMX client
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::use_input_buffer(
  OMX_INOUT OMX_BUFFERHEADERTYPE** abufferHdr,
  OMX_U32 abytes,
  OMX_U8* abuffer,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 aport)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  OMX_PARAM_PORTDEFINITIONTYPE *lport;
  QOMX_Buffer_Data_t **lbufferData;
  OMX_U32 *lBufAllocCount;
  int i = 0;
  QIDBG_MED("%s: Port Index = %d", __func__, (int) aport);

  if (aport == (OMX_U32)OMX_INPUT_PORT_INDEX) {
    lport = m_inPort;
    lbufferData = &mInOMXBufferData;
    lBufAllocCount = &m_inBuffAllocCount;
  } else if (aport == (OMX_U32)OMX_INPUT_THUMBNAIL_PORT_INDEX) {
    lport = m_inTmbPort;
    lbufferData = &mInTmbOMXBufferData;
    lBufAllocCount = &m_inTmbBuffAllocCount;
  } else {
    QIDBG_ERROR("%s %d: Error: Invalid port index", __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }

  if (*lBufAllocCount == 0) {
    /* validate buffer count */
    if (lport->nBufferCountActual <= 0) {
      QIDBG_ERROR("%s:%d] Error: invalid buffer count", __func__, __LINE__);
      return OMX_ErrorBadParameter;
    }
    *lbufferData = new QOMX_Buffer_Data_t[lport->nBufferCountActual];
    if (!*lbufferData) {
      QIDBG_ERROR("%s %d: Error: Allocation failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
    memset(*lbufferData, 0, sizeof(QOMX_Buffer_Data_t));
  }



  i = *lBufAllocCount;
  (*lbufferData)[i].mHeader.nAllocLen = abytes;
  (*lbufferData)[i].mHeader.nInputPortIndex = (OMX_U32)aport;
  (*lbufferData)[i].mHeader.nSize = sizeof(OMX_BUFFERHEADERTYPE);
  (*lbufferData)[i].mHeader.nVersion.nVersion = OMX_SPEC_VERSION;
  (*lbufferData)[i].mHeader.pBuffer = abuffer;
  (*lbufferData)[i].mHeader.pAppPrivate = appData;
  (*lbufferData)[i].mHeader.nOffset = 0;
  (*lbufferData)[i].mHeader.pPlatformPrivate = &(*lbufferData)[i].mInfo;
  (*lbufferData)[i].mHeader.pInputPortPrivate = lport;

  (*lbufferData)[i].mInfo.offset = 0;
  (*lbufferData)[i].mInfo.fd = -1;

  if (appData) {
    QOMX_BUFFER_INFO *lbufferInfo = (QOMX_BUFFER_INFO *)appData;
    (*lbufferData)[i].mInfo.fd =  lbufferInfo->fd;
    (*lbufferData)[i].mInfo.offset = lbufferInfo->offset;
    QIDBG_MED("%s:%d] buffer fd %d offset %d", __func__,
      __LINE__, (int)(*lbufferData)[i].mInfo.fd,
      (int)(*lbufferData)[i].mInfo.offset);
  }
  (*lbufferData)[i].mHeader.pPlatformPrivate = &(*lbufferData)[i].mInfo;
  (*lbufferData)[i].mHeader.pOutputPortPrivate = &(*lbufferData)[i].mInfo;

  *abufferHdr = &(*lbufferData)[i].mHeader;
  (*lbufferData)[i].valid = OMX_TRUE;
  (*lBufAllocCount)++;

  if (lport->nBufferCountActual == *lBufAllocCount) {
    lport->bPopulated = OMX_TRUE;
  }
  QIDBG_MED("%s:%d] BufferCountActual = %d, inBuffAllocCount = %d", __func__,
    __LINE__, (int)lport->nBufferCountActual, (int)*lBufAllocCount);
  return lret;
}

/*==============================================================================
* Function : use_output_buffer
* Parameters: bufferHdr, bytes, buffer
* Return Value : OMX_ERRORTYPE
* Description: Set the Input buffers passed by the OMX client
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::use_output_buffer(
  OMX_INOUT OMX_BUFFERHEADERTYPE** abufferHdr,
  OMX_U32 abytes,
  OMX_U8* abuffer,
  OMX_IN OMX_PTR appData,
  OMX_U32 aport)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int i = 0;
  QIDBG_MED("%s: Port Index = %d", __func__, (int) aport);

  if (m_outBuffAllocCount == 0) {
    mOutOMXBufferData = new QOMX_Buffer_Data_t[m_outPort->nBufferCountActual];
    if (NULL == mOutOMXBufferData) {
      QIDBG_ERROR("%s:%d] Error: Allocation failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
    memset(mOutOMXBufferData, 0, sizeof(QOMX_Buffer_Data_t));
  }
  i = m_outBuffAllocCount;
  mOutOMXBufferData[i].mHeader.nAllocLen = abytes;
  mOutOMXBufferData[i].mHeader.nInputPortIndex = (OMX_U32)aport;
  mOutOMXBufferData[i].mHeader.pOutputPortPrivate =
    &mOutOMXBufferData[i].mInfo;
  mOutOMXBufferData[i].mHeader.nSize = sizeof(OMX_BUFFERHEADERTYPE);
  mOutOMXBufferData[i].mHeader.nVersion.nVersion = OMX_SPEC_VERSION;
  mOutOMXBufferData[i].mHeader.pBuffer = abuffer;
  mOutOMXBufferData[i].mHeader.nOffset = 0;
  mOutOMXBufferData[i].mHeader.nFilledLen = 0;

  mOutOMXBufferData[i].mInfo.offset = 0;
  mOutOMXBufferData[i].mInfo.fd =  -1;

  if (appData) {
    QOMX_BUFFER_INFO *lbufferInfo = (QOMX_BUFFER_INFO *)appData;
    mOutOMXBufferData[i].mInfo.fd =  lbufferInfo->fd;
    mOutOMXBufferData[i].mInfo.offset = lbufferInfo->offset;
  }
  mOutOMXBufferData[i].mHeader.pAppPrivate =
    &(mOutOMXBufferData[i].mInfo);
  mOutOMXBufferData[i].mHeader.pPlatformPrivate = &mOutOMXBufferData->mInfo;
  mOutOMXBufferData[i].mHeader.pOutputPortPrivate = &mOutOMXBufferData->mInfo;
  mOutOMXBufferData[i].valid = OMX_TRUE;
  *abufferHdr = &mOutOMXBufferData[i].mHeader;

  m_outBuffAllocCount++;

  if (m_outPort->nBufferCountActual == m_outBuffAllocCount) {
    m_outPort->bPopulated = OMX_TRUE;
  }

  QIDBG_MED("%s %d: BufferCountActual = %d, outBuffAllocCount = %d", __func__,
    __LINE__, (int)m_outPort->nBufferCountActual, (int)m_outBuffAllocCount);
  return lret;

}

/*==============================================================================
* Function : omx_component_use_buffer
* Parameters: hComp, bufferHdr - Buffer header needs to allocated by the
* component, port - Port associated with the buffer, appData, bytes -
* size of the buffer, buffer - address of the buffer
* Return Value : OMX_ERRORTYPE
* Description: Create the bufferheader and set the buffers and any relevant data
* and return the header to the OMX client
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_use_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 bytes,
  OMX_IN OMX_U8* buffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  OMX_PARAM_PORTDEFINITIONTYPE *lport;
  OMX_U32 lBufAllocCount;

  if (bufferHdr == NULL || buffer == NULL || bytes == 0 ) {
    QIDBG_ERROR("%s %d]: bad param 0x%p %ld 0x%p", __func__, __LINE__,
      bufferHdr, bytes, buffer);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s %d] : Invalid State", __func__, __LINE__);
    return OMX_ErrorInvalidState;
  }
  if (((m_state == OMX_StateLoaded) &&
    (m_compTransState != OMX_StateIdle_Pending)) &&
    (m_state != OMX_StateIdle) &&
    (m_state != OMX_StateWaitForResources)) {
    QIDBG_ERROR("%s %d]: Not allowed in current state %d", __func__,
      __LINE__, m_state);
    return OMX_ErrorIncorrectStateOperation;
  }


  if (port == (OMX_U32)OMX_INPUT_THUMBNAIL_PORT_INDEX) {
    lport = m_inTmbPort;
    lBufAllocCount = m_inTmbBuffAllocCount;
  } else {
    lport = m_inPort;
    lBufAllocCount = m_inBuffAllocCount;
  }

  //ToDo : add check for executing with disabled port
  pthread_mutex_lock(&m_compLock);
  if ((port == (OMX_U32)OMX_INPUT_PORT_INDEX) ||
     (port == (OMX_U32)OMX_INPUT_THUMBNAIL_PORT_INDEX)) {
    if (lport->bEnabled) {
      if (bytes != lport->nBufferSize) {
        QIDBG_ERROR("%s:%d] exceeds the buffer size requested previously = "
          "%d", __func__, (int)bytes, (int)lport->nBufferSize);
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorBadParameter;
      }
      if (lBufAllocCount == lport->nBufferCountActual) {
        QIDBG_ERROR("%s:%d] Error: exceeds actual number of buffers "
          " requested %d %d",
          __func__, __LINE__, (int)lBufAllocCount,
          (int)lport->nBufferCountActual);
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorInsufficientResources;
      }
      lret = use_input_buffer(bufferHdr, bytes, buffer, appData, port);
    } else {
      QIDBG_ERROR("%s:%d] Error I/p port disabled", __func__, __LINE__);
      lret = OMX_ErrorNotReady;
    }
  }

  if (port == (OMX_U32) OMX_OUTPUT_PORT_INDEX) {
    if (m_outPort->bEnabled) {
      if (bytes != m_outPort->nBufferSize) {
        QIDBG_ERROR("%s %d: %d In o/p port exceeds the buffer size "
          "requested previously = %d",__func__, __LINE__, (int)bytes,
          (int)m_outPort->nBufferSize);
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorBadParameter;
      }
      if (m_outBuffAllocCount == m_inPort->nBufferCountActual) {
        QIDBG_ERROR("%s: Error:exceeds actual number of buffers requested",
          __func__);
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorInsufficientResources;
      }
      lret = use_output_buffer(bufferHdr, bytes, buffer, appData, port);
    } else {
      QIDBG_ERROR("%s : Error O/p port disabled", __func__);
      lret = OMX_ErrorNotReady;
    }
  }
  QIDBG_MED("%s %d: Inport populated = %d, Outport populated = %d", __func__,
    __LINE__, (int)m_inPort->bPopulated, (int)m_outPort->bPopulated);
  //Check if all the required ports are populated and set the flag
  if (m_inPort->bPopulated && m_outPort->bPopulated) {
    m_dataAllocated = OMX_TRUE;
  }
  QIDBG_MED("%s:%d] All Data Allocated = %d", __func__, __LINE__,
    m_dataAllocated);
  //If all the ports are populated and the component is in the idle pending
  //State transition to the Idle state
  if (m_dataAllocated && m_compTransState == OMX_StateIdle_Pending) {
    QIMessage *lMessage = new QIMessage();
    if (lMessage == NULL) {
      QIDBG_ERROR("%s:%d] cannot create message", __func__, __LINE__);
      lret = OMX_ErrorInsufficientResources;
    } else {
      // Preload the codec libraries
      preloadCodecLibs();

      m_state = OMX_StateIdle;
      m_compTransState = OMX_StateNone;
      lMessage->iData = m_state;
      lMessage->m_qMessage = OMX_MESSAGE_CHANGE_STATE_DONE;
      lret = postMessage(lMessage);
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] cannot post message", __func__, __LINE__);
        delete lMessage;
      }
    }
  }
  pthread_mutex_unlock(&m_compLock);
  return lret;
 }

/*==============================================================================
* Function : Execute
* Parameters: void
* Return Value : void
* Description: This method is inherited from the QIThreadObject class and
* is called from the StartThread method of the thread class as soon as a *
* startthread is called by the object on a new thread.
==============================================================================*/
void OMXImageEncoder::Execute()
{
  if (m_messageThread.IsSelf()) {
    handleMessage();
  }
}

/*==============================================================================
* Function : port_disable
* Parameters: OMX_U32 portIndex
* Return Value : OMX_ERRORTYPE
* Description: Disable the specified port and return any buffers
* that the port is holding throught ETB Done or FTB Done. Wait for
* OMX_Free_buffer to be complete to finish disabling the port.
==============================================================================*/
void OMXImageEncoder::portDisable(OMX_U32 a_portIndex)
{
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s : Error Invalid State",__func__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      OMX_ErrorIncorrectStateOperation, a_portIndex, NULL);
    sem_post(&m_cmdLock);
    return;
  }
  if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inPort->nPortIndex)) {
  //If comp is in state loaded and not in transition to idle,enable the port
    if ((m_state == OMX_StateLoaded) &&
      (!(m_compTransState == OMX_StateIdle_Pending))) {
      if (m_inBuffAllocCount > 0) {
        //Flush out through empty this buffer done
        m_inportTransState = OMX_PORT_DISABLE_PENDING;
      }
    } else if((m_state == OMX_StateWaitForResources) ||
      (m_state == OMX_StateLoaded)) {
      m_inPort->bEnabled = OMX_FALSE;
      m_callbacks->EventHandler(m_compHandle, m_appData,
      OMX_EventCmdComplete, OMX_CommandPortEnable, m_inPort->nPortIndex, NULL);
    } else {
        //Flush out input buffers
        m_inportTransState = OMX_PORT_DISABLE_PENDING;
    }
  } else if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inTmbPort->nPortIndex)){
     if ((m_state == OMX_StateLoaded) &&
      (!(m_compTransState == OMX_StateIdle_Pending))) {
      if (m_inBuffAllocCount > 0) {
        //Flush out through empty this buffer done
        m_inTmbPortTransState = OMX_PORT_DISABLE_PENDING;
      }
    } else if((m_state == OMX_StateWaitForResources) ||
      (m_state == OMX_StateLoaded)) {
      m_inTmbPort->bEnabled = OMX_FALSE;
      m_callbacks->EventHandler(m_compHandle, m_appData,
      OMX_EventCmdComplete, OMX_CommandPortEnable, m_inTmbPort->nPortIndex, NULL);
    } else {
        //Flush out input buffers
        m_inportTransState = OMX_PORT_DISABLE_PENDING;
    }
  }
  sem_post(&m_cmdLock);
}

/*==============================================================================
* Function : port_enable
* Parameters: OMX_U32 portIndex
* Return Value : OMX_ERRORTYPE
* Description: This method is used to enable the port
==============================================================================*/
void OMXImageEncoder::portEnable(OMX_U32 a_portIndex)
{
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s : Error Invalid State",__func__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      OMX_ErrorIncorrectStateOperation, a_portIndex, NULL);
    sem_post(&m_cmdLock);
    return;
  }
  if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inPort->nPortIndex)) {
    m_inPort->bEnabled = OMX_TRUE;
    //If comp is in state loaded and not in transition to idle,enable the port
    if (((m_state == OMX_StateLoaded) &&
      (!(m_compTransState == OMX_StateIdle_Pending))) ||
      (m_state == OMX_StateWaitForResources)) {
        m_callbacks->EventHandler(m_compHandle, m_appData,
        OMX_EventCmdComplete, OMX_CommandPortEnable, m_inPort->nPortIndex,
        NULL);
    } else {
        m_inportTransState = OMX_PORT_ENABLE_PENDING;
    }
  } else if ((a_portIndex == OMX_ALL) ||
      (a_portIndex == m_inTmbPort->nPortIndex)) {
       m_inTmbPort->bEnabled = OMX_TRUE;
    if (((m_state == OMX_StateLoaded) &&
       (!(m_compTransState == OMX_StateIdle_Pending))) ||
        (m_state == OMX_StateWaitForResources)) {

      m_callbacks->EventHandler(m_compHandle, m_appData,
          OMX_EventCmdComplete, OMX_CommandPortEnable, m_inPort->nPortIndex,
          NULL);
    } else {
        m_inTmbPortTransState = OMX_PORT_ENABLE_PENDING;
    }
  } else if ((a_portIndex == OMX_ALL) ||
    (a_portIndex == m_outPort->nPortIndex)) {
      m_outPort->bEnabled = OMX_TRUE;
      if (((m_state == OMX_StateLoaded) &&
        (!(m_compTransState == OMX_StateIdle_Pending))) ||
        (m_state == OMX_StateWaitForResources)) {
           m_callbacks->EventHandler(m_compHandle, m_appData,
           OMX_EventCmdComplete,OMX_CommandPortEnable, m_outPort->nPortIndex,
           NULL);
      } else {
        m_outportTransState = OMX_PORT_ENABLE_PENDING;
      }
  } else {
    QIDBG("%s: ERROR Invalid Port Index", __func__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      OMX_CommandPortEnable, a_portIndex, NULL);
  }
  //ToDo:Handle port enable on OMX_StateIdle and OMX_StateExecuting
  //in the OMX_StateExecuting state, then that port shall begin
  //transferring buffers
  if (m_inPort->bEnabled && m_outPort->bEnabled) {
    if (m_state == OMX_StateExecuting) {
     //StartEncode?
    }
    if (m_state == OMX_StateIdle && (!m_state == OMX_StateIdle_Pending)) {
      //ToDo
    }
  }
  sem_post(&m_cmdLock);
}

/*==============================================================================
* Function : process_Message
* Parameters: Message of type QIBase
* Return Value : OMX_ERRORTYPE
* Description: This method processes the commands/events from the message queue
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::processMessage(QIMessage *a_Message)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] E", __func__, __LINE__);

  if(!a_Message) {
    QIDBG_ERROR("%s:%d] bad parameter", __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }

  if(m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s:%d] Invalid state", __func__, __LINE__);
    return OMX_ErrorInvalidState;
  }

  switch(a_Message->m_qMessage) {
  case OMX_MESSAGE_CHANGE_STATE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_CHANGE_STATE", __func__, __LINE__);
    changeState((OMX_STATETYPE)a_Message->iData);
    break;
  }

  case OMX_MESSAGE_PORT_ENABLE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_PORT_ENABLE", __func__, __LINE__);
    portEnable((OMX_U32)a_Message->iData);
    break;
  }

  case OMX_MESSAGE_PORT_DISABLE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_PORT_ENABLE", __func__, __LINE__);
    portDisable((OMX_U32)a_Message->iData);
    break;
  }

  case OMX_MESSAGE_FLUSH: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_FLUSH", __func__, __LINE__);
    handleCommandFlush((OMX_U32)a_Message->iData);
    break;
  }

  case OMX_MESSAGE_START_MAIN_ENCODE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_START_MAIN_ENCODE", __func__, __LINE__);
    //Delete the thumbnail encoder before starting the main image encoding
    pthread_mutex_lock(&m_abortlock);
    if (m_thumbEncoder) {
       m_thumbEncoder->Stop();
    }
    if (!m_abort_flag) {
      lret = startEncode();
    }
    pthread_mutex_unlock(&m_abortlock);
    if (lret != OMX_ErrorNone) {
      //Todo: change state to Invalid
    }
    break;
  }

  case OMX_MESSAGE_ETB_DONE: {
    if ((m_state != OMX_StateExecuting) ||
      (m_compTransState == OMX_StateIdle_Pending)) {
      QIDBG_MED("%s:%d] Skip OMX_MESSAGE_ETB_DONE", __func__, __LINE__);
    } else {
      QIDBG_MED("%s:%d] OMX_MESSAGE_ETB_DONE", __func__, __LINE__);

      emptyBufferDone((OMX_BUFFERHEADERTYPE *)a_Message->pData);
    }
    break;
  }

  case OMX_MESSAGE_FTB_DONE: {
    if ((m_state != OMX_StateExecuting) ||
      (m_compTransState == OMX_StateIdle_Pending)) {
      QIDBG_MED("%s:%d] Skip OMX_MESSAGE_FTB_DONE", __func__, __LINE__);
    } else {
      QIDBG_MED("%s:%d] OMX_MESSAGE_FTB_DONE", __func__, __LINE__);

      /*Release Buffers associated with the current encode*/
      releaseCurrentSession();

      fillBufferDone((OMX_BUFFERHEADERTYPE *)a_Message->pData);
      if (m_ftbQueue.Count() == 0) {
        //Finished flushing all the buffers.
        m_executionComplete = OMX_TRUE;
      }
    }
    break;
  }

  case OMX_MESSAGE_EVENT_ERROR: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_EVENT_ERROR", __func__, __LINE__);

    /*Release Buffers associated with the current encode*/
    releaseCurrentSession();

    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      a_Message->iData, 0, NULL);
    break;
  }

  case OMX_MESSAGE_CHANGE_STATE_DONE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_CHANGE_STATE_DONE", __func__, __LINE__);

    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventCmdComplete,
      OMX_CommandStateSet, a_Message->iData, NULL);
    break;
  }

  case OMX_MESSAGE_START_NEW_ENCODE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_START_NEW_ENCODE", __func__, __LINE__);
    lret = Start();
    break;
  }

  default: {
    QIDBG_ERROR("%s:%d] Invalid Message = %d", __func__, __LINE__,
      a_Message->m_qMessage);
    break;
  }
  } /* end of switch*/

  if (a_Message) {
    delete a_Message;
    a_Message = NULL;
  }
  return lret;
}

/*==============================================================================
* Function : flushBufferQueues
* Parameters: OMX_U32 a_portIndex - Port Index of the port who's buffers
* need to be flushed
* Return Value : OMX_ERRORTYPE
* Description: Return the unprocessed buffers associated with the port. This can
* be called when the client calls command flush, during transition to
* OMX_StateIdle from Executing/Pause or transition to OMX_StateInvalid or
* when the port is disabled.
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::flushBufferQueues(OMX_U32 a_portIndex)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_Buffer *l_buffer = NULL;

  if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inPort->nPortIndex)) {
    for (int i = 0; i < m_etbQueue.Count(); i++) {
      l_buffer = (QOMX_Buffer *)m_etbQueue.Dequeue();
      if(l_buffer) {
        emptyBufferDone((OMX_BUFFERHEADERTYPE *)l_buffer->getBuffer());
      }
    }
  } else if ((a_portIndex == OMX_ALL) ||
      (a_portIndex == m_inTmbPort->nPortIndex)) {
    for (int i = 0; i < m_etbTmbQueue.Count(); i++) {
      l_buffer = (QOMX_Buffer *)m_etbTmbQueue.Dequeue();
      if(l_buffer) {
        emptyBufferDone((OMX_BUFFERHEADERTYPE *)l_buffer->getBuffer());
      }
    }
  } else if ((a_portIndex == OMX_ALL) ||
    (a_portIndex == m_outPort->nPortIndex)) {
    for (int i = 0; i < m_ftbQueue.Count(); i++) {
      l_buffer = (QOMX_Buffer *)m_ftbQueue.Dequeue();
      if(l_buffer) {
        fillBufferDone((OMX_BUFFERHEADERTYPE *)l_buffer->getBuffer());
      }
    }
  } else {
    QIDBG_ERROR("%s %d]: Bad Port Index", __func__, __LINE__);
    lret = OMX_ErrorBadPortIndex;
  }

  if (l_buffer) {
    delete(l_buffer);
    l_buffer = NULL;
  }

  return lret;
}

/*==============================================================================
* Function : handleCommandFlush
* Parameters: OMX_U32 a_portIndex - port Index of the port who's buffers
* need to be flushed
* Return Value : void
* Description: Handle the Flush command from the omx client. Calls
* flush_Buffer_Queues to flush out unprocessed buffers associated with
* a_portIndex
==============================================================================*/
void OMXImageEncoder::handleCommandFlush(OMX_U32 a_portIndex)
{
   OMX_ERRORTYPE lret = OMX_ErrorNone;
   lret = flushBufferQueues(a_portIndex);
   if (lret == OMX_ErrorNone ) {
     if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inPort->nPortIndex)) {
       m_callbacks->EventHandler(m_compHandle, m_appData,
         OMX_EventCmdComplete, OMX_CommandFlush, m_inPort->nPortIndex, NULL);
     } else if ((a_portIndex == OMX_ALL) ||
       (a_portIndex == m_inTmbPort->nPortIndex)) {
       m_callbacks->EventHandler(m_compHandle, m_appData,
         OMX_EventCmdComplete, OMX_CommandFlush, m_inTmbPort->nPortIndex, NULL);
     } else if ((a_portIndex == OMX_ALL) ||
       (a_portIndex == m_outPort->nPortIndex)) {
       m_callbacks->EventHandler(m_compHandle, m_appData,
         OMX_EventCmdComplete, OMX_CommandFlush, m_outPort->nPortIndex,
         NULL);
     }
   } else {
     m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError, lret,
       a_portIndex, NULL);
   }
  sem_post(&m_cmdLock);
}

/*==============================================================================
* Function : omx_component_empty_this_buffer
* Parameters: OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE buffer
* Return Value : OMX_ERRORTYPE
* Description: The client makes this call with the input buffer indicating the
* component to start processing the input buffer
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_empty_this_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  QOMX_Buffer *lInputBuffer;
  QIQueue *lBufQueue;
  OMX_PARAM_PORTDEFINITIONTYPE *lPort;

  if (hComp == NULL || buffer == NULL ||
    (buffer->nSize != sizeof(OMX_BUFFERHEADERTYPE))) {
    QIDBG_ERROR("%s: Bad Parameter", __func__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Invalid State", __func__);
    return OMX_ErrorInvalidState;
  }
  if ((m_state != OMX_StateExecuting) && (m_state != OMX_StatePause)) {
    QIDBG_ERROR("%s: Current State is %d. Operation Not allowed",
      __func__, m_state);
    return OMX_ErrorInvalidState;
  }
  if ((buffer->nVersion.nVersion != OMX_SPEC_VERSION)) {
    QIDBG_ERROR("%s: Error - Invalid version ", __func__);
    return OMX_ErrorVersionMismatch;
  }
  if ((buffer->nInputPortIndex != m_inPort->nPortIndex) &&
      (buffer->nInputPortIndex != m_inTmbPort->nPortIndex)) {
      QIDBG_ERROR("%s : bad Port index %d",
        __func__,(int)buffer->nInputPortIndex);
      return OMX_ErrorBadPortIndex;
  }

  if (buffer->nInputPortIndex == m_inPort->nPortIndex) {
    lPort = m_inPort;
    lBufQueue = &m_etbQueue;
  } else {
    // Thumbnail input
    lPort = m_inTmbPort;
    lBufQueue = &m_etbTmbQueue;
  }

  if(!lPort->bEnabled) {
    QIDBG_ERROR("%s: Error Port not enabled %d",
      __func__, (int)buffer->nInputPortIndex);
    return OMX_ErrorIncorrectStateOperation;
  }
  pthread_mutex_lock(&m_etbQLock);
  pthread_mutex_lock(&m_etbTmbQLock);

  if (m_etbQueue.Count() || ((!m_inTmbPort->bEnabled)||(m_etbTmbQueue.Count()))){
    if (!lBufQueue->Count()) {
      m_etbFlag = OMX_TRUE;
    }
   }

  lInputBuffer = new QOMX_Buffer(buffer);
  if (NULL == lInputBuffer) {
    QIDBG_ERROR("%s:%d] cannot allocate OMX buffer",
      __func__, __LINE__);
    pthread_mutex_unlock(&m_etbTmbQLock);
    pthread_mutex_unlock(&m_etbQLock);
    return OMX_ErrorInsufficientResources;
  }
  if (QI_ERROR((*lBufQueue).Enqueue(lInputBuffer))) {
    QIDBG_ERROR("%s:%d] cannot enqueue OMX buffer",
      __func__, __LINE__);
    delete(lInputBuffer);
    lInputBuffer = NULL;
    pthread_mutex_unlock(&m_etbTmbQLock);
    pthread_mutex_unlock(&m_etbQLock);
    return OMX_ErrorInsufficientResources;
  }

  QIDBG_MED("%s %d: ETB Queue count = %d, Expected number of buffers = %d ",
    __func__, __LINE__, (*lBufQueue).Count(), (int)lPort->nBufferCountActual);

  pthread_mutex_unlock(&m_etbTmbQLock);
  pthread_mutex_unlock(&m_etbQLock);

  //Start encode if in execute state already and etb and ftb flags are set
  QIDBG_MED("%s:%d] state %d etb_flag %d ftb_flag %d",
    __func__, __LINE__, m_state, m_etbFlag, m_ftbFlag);
  if (m_state == OMX_StateExecuting && m_etbFlag && m_ftbFlag) {
    QIDBG_MED("%s %d: Going to Start Encoding. Etb and ftb flags are set",
      __func__, __LINE__);

    // Send message for a new encode process
    QIMessage *lEncodeMessage = new QIMessage();
    if (!lEncodeMessage) {
      QIDBG_ERROR("%s:%d] Could not alloate QIMessage", __func__,  __LINE__);
      return OMX_ErrorInsufficientResources;
    }
    lEncodeMessage->m_qMessage = OMX_MESSAGE_START_NEW_ENCODE;
    lrc = postMessage(lEncodeMessage);
    if (QI_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d] Could not send Start encode", __func__,  __LINE__);
      delete lEncodeMessage;
      return OMX_ErrorInsufficientResources;
    }
  }
  return lrc;
}

/*==============================================================================
* Function : omx_component_fill_this_buffer
* Parameters: OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE buffer
* Return Value : OMX_ERRORTYPE
* Description: The client makes this call with the out buffer indicating the
* component to start processing the output buffer
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_fill_this_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  QOMX_Buffer *lOutputBuffer;

  if (hComp == NULL || buffer == NULL ||
    (buffer->nSize != sizeof(OMX_BUFFERHEADERTYPE))) {
    QIDBG_ERROR("%s: Bad Parameter", __func__);
    return OMX_ErrorBadParameter;
  }

  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Invalid State", __func__);
    return OMX_ErrorInvalidState;
  }

  if ((m_state != OMX_StateExecuting) && (m_state != OMX_StatePause)) {
    QIDBG_ERROR("%s: Current State is %d. Operation Not allowed",
      __func__, m_state);
    return OMX_ErrorInvalidState;
  }

  if ((buffer->nVersion.nVersion != OMX_SPEC_VERSION)) {
    QIDBG_ERROR("%s: Error - Invalid version ", __func__);
    return OMX_ErrorVersionMismatch;
  }

  if((buffer->nOutputPortIndex == m_outPort->nPortIndex) &&
    (!m_outPort->bEnabled)) {
    QIDBG_ERROR("%s: Error Port not enabled %d",
      __func__, (int)buffer->nOutputPortIndex);
    return OMX_ErrorIncorrectStateOperation;
  }
  pthread_mutex_lock(&m_ftbQLock);

  lOutputBuffer = new QOMX_Buffer(buffer);
  if (NULL == lOutputBuffer) {
    QIDBG_ERROR("%s:%d] cannot allocate OMX buffer",
      __func__, __LINE__);
    pthread_mutex_unlock(&m_ftbQLock);
    return OMX_ErrorInsufficientResources;
  }

  if (!m_ftbQueue.Count()) {
    m_ftbFlag = OMX_TRUE;
  }

  if (QI_ERROR(m_ftbQueue.Enqueue(lOutputBuffer))) {
    QIDBG_ERROR("%s:%d] cannot enqueue buffer",
      __func__, __LINE__);
    delete(lOutputBuffer);
    lOutputBuffer = NULL;
    pthread_mutex_unlock(&m_ftbQLock);
    return OMX_ErrorInsufficientResources;
  }

  QIDBG_MED("%s %d: FTB Queue count = %d, Expected number of buffers = %d ",
    __func__, __LINE__, m_ftbQueue.Count(),
    (int)m_outPort->nBufferCountActual);


  pthread_mutex_unlock(&m_ftbQLock);
  //Start encode if in execute state already and etb and ftb flags are set

  QIDBG_MED("%s:%d] state %d etb_flag %d ftb_flag %d",
    __func__, __LINE__, m_state, m_etbFlag, m_ftbFlag);
  if ((m_state == OMX_StateExecuting) && m_etbFlag && m_ftbFlag) {
    QIDBG_MED("%s %d: Going to Start Encoding. Etb and ftb flags are set",
      __func__, __LINE__);
    //lrc = Start();
    // Send message for a new encode process
    QIMessage *lEncodeMessage = new QIMessage();
    if (!lEncodeMessage) {
      QIDBG_ERROR("%s:%d] Could not alloate QIMessage", __func__,  __LINE__);
      return OMX_ErrorInsufficientResources;
    }
    lEncodeMessage->m_qMessage = OMX_MESSAGE_START_NEW_ENCODE;
    lrc = postMessage(lEncodeMessage);
    if (QI_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d] Could not send Start encode", __func__,  __LINE__);
      delete lEncodeMessage;
      return OMX_ErrorInsufficientResources;
    }
    /* reset the flags */
    m_etbFlag = OMX_FALSE;
    m_ftbFlag = OMX_FALSE;
  }
  return lrc;
}

/*==============================================================================
* Function : start
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Start encode/decode of the image. This method is called when the
* etb and ftb are done and the component is in the execute state. Dequeue the
* buffers from the etb and ftb queue and call encode/decode.
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::Start()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_Buffer *lInputBuffer, *lOutputBuffer, *lInputTmbBuffer;

  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s:%d] Invalid state", __func__, __LINE__);
    return OMX_ErrorInvalidState;
  }
  if (m_state != OMX_StateExecuting) {
    QIDBG_ERROR("%s:%d] Incorrect state %d", __func__, __LINE__,
      (int)m_state);
    return OMX_ErrorIncorrectStateOperation;
  }

  pthread_mutex_lock(&m_etbQLock);
  if (m_inTmbPort->bEnabled) {
    pthread_mutex_lock(&m_etbTmbQLock);
  }
  pthread_mutex_lock(&m_ftbQLock);

  if (!m_etbQueue.Count() || (m_inTmbPort->bEnabled && !m_etbQueue.Count()) ||
     (!m_ftbQueue.Count())) {
    pthread_mutex_unlock(&m_ftbQLock);
    if (m_inTmbPort->bEnabled) {
      pthread_mutex_unlock(&m_etbTmbQLock);
    }
    pthread_mutex_unlock(&m_etbQLock);

    return OMX_ErrorNone;
  }

  lInputBuffer = (QOMX_Buffer *)m_etbQueue.Dequeue();

  if ((NULL == lInputBuffer) || (NULL == lInputBuffer->getBuffer())) {
    QIDBG_ERROR("%s:%d] Input Buffer is NULL", __func__, __LINE__);
    goto error_locked;
  }

  //Assign the current i/p buffer
  m_currentInBuffHdr = lInputBuffer->getBuffer();

  if (lInputBuffer) {
    delete(lInputBuffer);
    lInputBuffer = NULL;
  }

  m_currentInTmbBuffHdr = m_currentInBuffHdr;
  if (m_inTmbPort->bEnabled) {
    lInputTmbBuffer = (QOMX_Buffer *)m_etbTmbQueue.Dequeue();
    if ((NULL == lInputTmbBuffer) || (NULL == lInputTmbBuffer->getBuffer())) {
      QIDBG_ERROR("%s:%d] Thumbnail Input Buffer is NULL", __func__, __LINE__);
     // return OMX_ErrorBadParameter;
    } else {
      m_currentInTmbBuffHdr = lInputTmbBuffer->getBuffer();
    }

    if (lInputTmbBuffer) {
      delete(lInputTmbBuffer);
      lInputTmbBuffer = NULL;
    }
  }

  lOutputBuffer = (QOMX_Buffer *)m_ftbQueue.Dequeue();
  if ((NULL == lOutputBuffer) || (NULL == lOutputBuffer->getBuffer())) {
    QIDBG_ERROR("%s:%d] output Buffer is NULL", __func__, __LINE__);
    goto error_locked;
  }

  //Assign the current o/p buffer
  m_currentOutBuffHdr = lOutputBuffer->getBuffer();

  if (lOutputBuffer) {
    delete(lOutputBuffer);
    lOutputBuffer = NULL;
  }

  pthread_mutex_unlock(&m_ftbQLock);
  if (m_inTmbPort->bEnabled) {
    pthread_mutex_unlock(&m_etbTmbQLock);
  }
  pthread_mutex_unlock(&m_etbQLock);

  //Call Encode Image
  lret = encodeImage(m_currentInBuffHdr, m_currentInTmbBuffHdr,
                     m_currentOutBuffHdr);
  if (lret != OMX_ErrorNone) {
    //Transition to InvalidState?
    goto error;
  }

  return lret;

error_locked:
  pthread_mutex_unlock(&m_ftbQLock);
  if (m_inTmbPort->bEnabled) {
    pthread_mutex_unlock(&m_etbTmbQLock);
  }
  pthread_mutex_unlock(&m_etbQLock);


error:

  QIMessage *lErrMessage = new QIMessage();
  lErrMessage->m_qMessage = OMX_MESSAGE_EVENT_ERROR;
  lErrMessage->iData = OMX_ErrorUndefined;
  lret = postMessage(lErrMessage);
  if (lret != OMX_ErrorNone) {
    QIDBG_ERROR("%s %d: Error posting message", __func__, __LINE__);
  }
  return lret;
}

/*==============================================================================
* Function : abortExecution
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Handle the component moving to from Executing/Pause state to idle
* state. Abort the current encoding session and return the unprocessed buffers.
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::abortExecution()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lrc = 0;
  //If Thumbnail Encoding is in Progress stop the encoder

  if (m_thumbEncoding && m_thumbEncoder) {
    lrc = m_thumbEncoder->Stop();
    if (!lrc) {
     //ToDo
    }
    m_thumbEncoding = OMX_FALSE;
  }
  if (m_mainImageEncoding && m_mainEncoder) {
    lrc = m_mainEncoder->Stop();
    if (!lrc) {
     //ToDo
    }
    m_mainImageEncoding = OMX_FALSE;
  }
  //Release Buffers associated with current encode
  lret = releaseCurrentSession();

  if (lret != OMX_ErrorNone) {
    return lret;
  }

  //Flush out the message queue
  if (m_queue.Count() > 0) {
    m_queue.DeleteAll();
  }
  //Flush all the ports to return any unprocessed buffers they are holding
  flushBufferQueues(OMX_ALL);

  return lret;
}
/*==============================================================================
* Function : omx_component_allocate_buffer
* Parameters: hComp, bufferHdr, port, appData, bytes
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_allocate_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr, OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 bytes)
{
  return OMX_ErrorNotImplemented;
}

/*==============================================================================
* Function : CanFreeBuffers
* Parameters: aPort, aBuffer, pBufferData
* Return Value : OMX_BOOL
* Description: This function is used to check if all the buffers are passed to
*              the component.
==============================================================================*/
OMX_BOOL OMXImageEncoder::CanFreeBuffers(OMX_PARAM_PORTDEFINITIONTYPE *aPort,
  OMX_BUFFERHEADERTYPE *aBuffer, QOMX_Buffer_Data_t *pBufferData)
{
  int i = 0;
  OMX_BOOL lCanFreeBuffers = OMX_TRUE;

  if (!aBuffer || !aPort || !pBufferData) {
    QIDBG_ERROR("%s:%d] Error invalid input, cannot free buffers",
      __func__, __LINE__);
    return OMX_FALSE;
  }

  for (i = 0; i < (int)aPort->nBufferCountActual; i++) {
    if (aBuffer == &pBufferData[i].mHeader) {
      pBufferData[i].valid = OMX_FALSE;
      break;
    }
  }

  for (i = 0; i < (int)aPort->nBufferCountActual; i++) {
    if (pBufferData[i].valid == OMX_TRUE) {
      lCanFreeBuffers = OMX_FALSE;
      break;
    }
  }
  return lCanFreeBuffers;
}

/*==============================================================================
* Function : qomx_component_free_buffer
* Parameters: hComp, port, buffer
* Return Value : OMX_ERRORTYPE
* Description: This function is called by the client to free the buffer headers
* allocated during UseBuffer/Allocate Buffer functions
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_free_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_BUFFERHEADERTYPE *buffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int i = 0;

  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, m_state);
  if (m_state == OMX_StateIdle) {
    if (m_compTransState != OMX_StateLoaded_Pending) {
      QIDBG_ERROR("%s:%d: Component needs to be in transition to Loaded state "
        "to free buffers", __func__, __LINE__);
      return OMX_ErrorIncorrectStateOperation;
    }
  } else {
    QIDBG_ERROR("%s:%d: Invalid State operation. Current state is %d", __func__,
      __LINE__, m_state);
    return OMX_ErrorIncorrectStateOperation;
  }

  if (port == m_inPort->nPortIndex) {
    if (CanFreeBuffers(m_inPort, buffer, mInOMXBufferData)) {
      QIDBG_MED("%s:%d] Releasing %d input bufferheaders",
        __func__, __LINE__, (int)m_inPort->nBufferCountActual);
      if (m_inBuffAllocCount > 0) {
        if (mInOMXBufferData) {
          delete[] mInOMXBufferData;
          mInOMXBufferData = NULL;
        }
        m_inBuffAllocCount = 0;
      }
    }
  } else if (port == m_inTmbPort->nPortIndex) {
    if (CanFreeBuffers(m_inTmbPort, buffer, mInTmbOMXBufferData)) {
      QIDBG_MED("%s:%d]: Releasing %d input Tmb bufferheaders",
        __func__, __LINE__, (int)m_inTmbPort->nBufferCountActual);
      if (m_inTmbBuffAllocCount > 0) {
        if (mInTmbOMXBufferData) {
          delete[] mInTmbOMXBufferData;
          mInTmbOMXBufferData = NULL;
        }
        m_inBuffAllocCount = 0;
      }
    }
  } else if (port == m_outPort->nPortIndex) {
    if (CanFreeBuffers(m_outPort, buffer, mOutOMXBufferData)) {
      QIDBG_MED("%s:%d]: Releasing %d output bufferheaders",
        __func__, __LINE__, (int)m_outPort->nBufferCountActual);
      if (m_outBuffAllocCount > 0) {
        if (mOutOMXBufferData) {
          delete[] mOutOMXBufferData;
          mOutOMXBufferData = NULL;
        }
        m_outBuffAllocCount = 0;
      }
    }
  } else {
    QIDBG_ERROR("%s:%d]: Invalid Port Index %d", __func__, __LINE__,
      (int)port);
    lret = OMX_ErrorBadPortIndex;
  }
  if ((m_inBuffAllocCount == 0) && (m_outBuffAllocCount == 0) &&
    (m_compTransState == OMX_StateLoaded_Pending)) {
    QIMessage *lMessage = new QIMessage();
    if (lMessage == NULL) {
      QIDBG_ERROR("%s:%d] cannot create message", __func__, __LINE__);
      lret = OMX_ErrorInsufficientResources;
    } else {
      // Release the codec libs
      releaseCodecLibs();

      m_state = OMX_StateLoaded;
      m_compTransState = OMX_StateNone;
      lMessage->iData = m_state;
      lMessage->m_qMessage = OMX_MESSAGE_CHANGE_STATE_DONE;
      lret = postMessage(lMessage);
      QIDBG_HIGH("%s:%d: Free Buffer complete: State change from"
        "OMX_StateLoadedPending to OMX_StateLoaded", __func__, __LINE__);
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] cannot post message", __func__, __LINE__);
        delete lMessage;
      }
    }
  }
  return lret;
}

/*==============================================================================
* Function : omx_component_role_enum
* Parameters: OMX_HANDLETYPE hComp, OMX_U8 *role, OMX_U32 index
* Return Value : OMX_ERRORTYPE
* Description: Handle the transition to the invalid state
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_role_enum(OMX_IN OMX_HANDLETYPE hComp,
    OMX_OUT OMX_U8 *role,
    OMX_IN OMX_U32 index)
{
  return OMX_ErrorNotImplemented;
}

/*==============================================================================
* Function : abortMessageThread
* Parameters: none
* Return Value : none
* Description: This function will abort the message handler thread
==============================================================================*/
void OMXImageEncoder::abortMessageThread()
{
  QIDBG_MED("%s %d]: mState %d", __func__, __LINE__, m_state);

  pthread_mutex_lock(&m_queueLock);
  pthread_cond_signal(&m_queueCond);
  m_thread_exit_flag = OMX_TRUE;
  pthread_mutex_unlock(&m_queueLock);
  m_messageThread.JoinThread();
}

/*==============================================================================
* Function : omx_component_deinit
* Parameters: hComp
* Return Value : OMX_ERRORTYPE
* Description: This function will deinitiale the component.
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_deinit(OMX_IN OMX_HANDLETYPE hComp)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int i = 0;

  QIDBG_MED("%s %d]: ", __func__, __LINE__);

  if (hComp == NULL) {
    QIDBG_ERROR("%s %d]: Bad parameter", __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }

  if (m_messageThread.IsSelf()) {
    QIDBG_ERROR("%s %d]: Deinit called from the message thread.. Not allowed",
      __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  pthread_mutex_lock(&m_compLock);
  if (m_compInitialized) {
    if ((m_state == OMX_StateLoaded) || m_state == OMX_StateInvalid) {
      QIDBG_MED("%s %d]: ", __func__, __LINE__);
      abortMessageThread();
    }
    QIDBG_MED("%s %d]: m_state %d", __func__, __LINE__, m_state);
    if (m_state == OMX_StateExecuting) {
      abortExecution();
      if (m_dataAllocated) {
        for (i = 0; i < (int)m_inPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_inPort->nPortIndex,
            &mInOMXBufferData[i].mHeader);
        }
        for (i = 0; i < (int)m_inTmbPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_inTmbPort->nPortIndex,
            &mInTmbOMXBufferData[i].mHeader);
        }
        for (i = 0; i < (int)m_outPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_outPort->nPortIndex,
            &mOutOMXBufferData[i].mHeader);
        }
      }
      m_state = OMX_StateInvalid;
      abortMessageThread();
    }
    if (m_state == OMX_StateIdle) {
      if (m_dataAllocated) {
        for (i = 0; i < (int)m_inPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_inPort->nPortIndex,
            &mInOMXBufferData[i].mHeader);
        }
        for (i = 0; i < (int)m_inTmbPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_inTmbPort->nPortIndex,
            &mInTmbOMXBufferData[i].mHeader);
        }
        for (i = 0; i < (int)m_outPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_outPort->nPortIndex,
            &mOutOMXBufferData[i].mHeader);
        }
      }
      m_state = OMX_StateInvalid;
      abortMessageThread();
    }
  }
  m_compInitialized = OMX_FALSE;
  pthread_mutex_unlock(&m_compLock);
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : getEstimatedExifSize
* Parameters:
* Return Value : exif size
* Description: This function will return the estimated exif size.
* Notes: Need to update mNumExifMarkers when adding new markers to EXIF
==============================================================================*/
OMX_U32 OMXImageEncoder::getEstimatedExifSize()
{
  return 64*1024 * mNumExifMarkers;
}
