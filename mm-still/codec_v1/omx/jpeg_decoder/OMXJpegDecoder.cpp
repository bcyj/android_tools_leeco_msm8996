/*******************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#include "OMXJpegDecoder.h"



/*==============================================================================
* Function : getInstance
* Parameters: None
* Return Value : void * - pointer to the class object
* Description: C function that returns an instance of the class.
==============================================================================*/
extern "C" void *getInstance()
{
  void *lobj = new OMXJpegDecoder();
  QIDBG_HIGH("%s:%d] Component ptr is %p", __func__, __LINE__, lobj);
  return lobj;
}

/*==============================================================================
* Function : WriteError
* Parameters: ErrorType aErrorType
* Return Value : int
* Description:
==============================================================================*/
void OMXJpegDecoder::ReadError(ErrorType aErrorType)
{
   QIDBG_MED("%s:%d] ", __func__, __LINE__);
}
/*==============================================================================
* Function : WriteComplete
* Parameters: uint8_t *aBuffer, uint32_t aSize
* Return Value : int
* Description:
==============================================================================*/
void OMXJpegDecoder::ReadComplete(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*==============================================================================
* Function : WriteFragmentDone
* Parameters: QIBuffer &aBuffer
* Return Value : void
* Description:
==============================================================================*/
void OMXJpegDecoder::ReadFragmentDone(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}


OMX_ERRORTYPE OMXJpegDecoder::configureDecodeData()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  uint32_t l_imageSize;
  QICrop l_crop;

  //Start configuration only if the abort flag is not set
  m_outputSize[0].setHeight((int)m_outPort->format.image.nFrameHeight);
  m_outputSize[0].setWidth((int)m_outPort->format.image.nFrameWidth);
  m_outputPadSize.setHeight((int)m_outPort->format.image.nSliceHeight);
  m_outputPadSize.setWidth((int)m_outPort->format.image.nStride);

  //Translate from OMX format to QIformat
  lret = translateFormat(m_outPort->format.image.eColorFormat, &m_format,
      &m_subsampling);
  if (lret != OMX_ErrorNone) {
    return lret;
  }
  QIDBG_HIGH("%s:%d] size %dx%d pad %dx%d subsampling %d",
      __func__, __LINE__,
      m_outputSize[0].Width(), m_outputSize[0].Height(),
      m_outputPadSize.Width(), m_outputPadSize.Height(),
      m_subsampling);

  /*decode params*/
  m_decodeParams.setInputSize(m_outputSize[0]);
  m_decodeParams.setOutputSize(m_outputSize[0]); //TODO: Set scaled size
  m_decodeParams.setFrameInfo(m_mainFrameInfo);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE OMXJpegDecoder::configureInBuffer(
  OMX_BUFFERHEADERTYPE *a_inBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_BUFFER_INFO *lInBufferInfo = NULL;

  lInBufferInfo = reinterpret_cast<QOMX_BUFFER_INFO *>
  (a_inBuffer->pOutputPortPrivate);
  m_inputQIBuffer = new QIBuffer((uint8_t*)a_inBuffer->pBuffer,
      a_inBuffer->nAllocLen);
  if (!m_inputQIBuffer) {
    QIDBG_ERROR("%s:%d] Error allocating i/p QIBuffer", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  m_inputQIBuffer->SetFd((int)lInBufferInfo->fd);
  m_inputQIBuffer->SetOffset(lInBufferInfo->offset);
  m_inputQIBuffer->SetFilledLen(a_inBuffer->nFilledLen);

  QIDBG_HIGH("%s:%d] MAIN buf=%p, fd=%lu, offset=%lu filled=%lu", __func__, __LINE__,
       a_inBuffer->pBuffer,
       lInBufferInfo->fd,
       lInBufferInfo->offset,
       a_inBuffer->nFilledLen);

  return lret;
}

OMX_ERRORTYPE OMXJpegDecoder::configureOutBuffer(
    OMX_BUFFERHEADERTYPE *a_outBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_BUFFER_INFO *lOutBufferInfo = NULL;

  lOutBufferInfo = reinterpret_cast<QOMX_BUFFER_INFO *>
  (a_outBuffer->pOutputPortPrivate);
  QIDBG_MED("%s: %d: O/p buffer address is %p, size: %d", __func__, __LINE__,
      a_outBuffer->pBuffer, (int)a_outBuffer->nAllocLen);
  m_outputQIBuffer = new QIBuffer((uint8_t*)a_outBuffer->pBuffer,
      a_outBuffer->nAllocLen);
  if (!m_inputQIBuffer) {
    QIDBG_ERROR("%s:%d] Error allocating o/p QIBuffer", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  m_outputQIBuffer->SetFd(lOutBufferInfo->fd);
  m_outputQIBuffer->SetOffset(lOutBufferInfo->offset);
  m_outputQIBuffer->SetFilledLen(0);

  return lret;
}

/*==============================================================================
* Function : configureBuffers
* Parameters: a_inBuffer, a_outBuffer
* Return Value : OMX_ERRORTYPE
* Description: Configure the input and output buffers
==============================================================================*/
OMX_ERRORTYPE OMXJpegDecoder::configureBuffers(
  OMX_BUFFERHEADERTYPE *a_inBuffer,
  OMX_BUFFERHEADERTYPE *a_outBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_BUFFER_INFO *lInBufferInfo = NULL;
  QOMX_BUFFER_INFO *lOutBufferInfo = NULL;

  if (NULL == m_inputQIBuffer) {
    lret = configureInBuffer(a_inBuffer);
  }

  if (lret != OMX_ErrorNone) {
    return lret;
  }

  if (NULL == m_outputQIBuffer) {
    lret = configureOutBuffer(a_outBuffer);
  }

  return lret;
}

/*==============================================================================
* Function : startDecode
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Get the decoder from the factory and start decoding
==============================================================================*/
OMX_ERRORTYPE OMXJpegDecoder::startDecode()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
   int lrc = QI_SUCCESS;

   //Set the offset for each plane
   uint32_t lOffset[OMX_MAX_NUM_PLANES] = {0, 0, 0};

   //Set the physical offset for each plane
   uint32_t lPhyOffset[QI_MAX_PLANES] = {0, 0, 0};

   m_mainDecoder = m_factory.CreateDecoder(QImageCodecFactory::HW_CODEC_ONLY,
       m_decodeParams);

   if (m_mainDecoder == NULL) {
     QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
     return OMX_ErrorInsufficientResources;
   }

   m_inputImage = new QImage(m_inputQIBuffer->Addr() +
       m_inputQIBuffer->FilledLen(),
       m_inputQIBuffer->Length() - m_inputQIBuffer->FilledLen(),
       QI_BITSTREAM);

   if (m_inputImage == NULL) {
     QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
     return OMX_ErrorInsufficientResources;
   }

   m_inputImage->setFd(m_inputQIBuffer->Fd());

   m_outputImage = new QImage(m_outputPadSize, m_subsampling, m_format,
     m_outputSize[0]);

   if (m_outputImage == NULL) {
     QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
     return OMX_ErrorInsufficientResources;
   }

   lrc = m_outputImage->setDefaultPlanes(m_numOfPlanes,
       m_outputQIBuffer->Addr(), m_outputQIBuffer->Fd(), NULL, NULL);
   if (lrc) {
     QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
     return OMX_ErrorUndefined;
   }

   lrc = m_mainDecoder->setDecodeParams(m_decodeParams);
   if (lrc != QI_SUCCESS) {
     QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
     return OMX_ErrorUndefined;
   }

   lrc = m_mainDecoder->addInputImage(*m_inputImage);
   if (lrc) {
     QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
     return OMX_ErrorUndefined;
   }

   lrc = m_mainDecoder->addOutputImage(*m_outputImage);
   if (lrc) {
     QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
     return OMX_ErrorUndefined;
   }

   lrc = m_mainDecoder->addObserver(*this);
   if (lrc) {
     QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
     return OMX_ErrorUndefined;
   }

   lrc = m_mainDecoder->Start();
   if (lrc) {
     QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
     return OMX_ErrorUndefined;
   }

   return lret;
}

/*==============================================================================
 * Function : decodeImage
 * Parameters: @a_inBuffer- Input buffer passed during empty this buffer
 *             @a_outBuffer - O/p buffer passed during fill this buffer
 * Return Value : OMX_ERRORTYPE
 * Description: Configures and starts decoding
==============================================================================*/
OMX_ERRORTYPE OMXJpegDecoder::decodeImage(OMX_BUFFERHEADERTYPE *a_inBuffer,
    OMX_BUFFERHEADERTYPE *a_outBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lrc = QI_SUCCESS;


  if (!a_inBuffer || !a_outBuffer) {
    QIDBG_ERROR("%s:%d] Bad parameter",  __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s:%d] Invalid State",  __func__, __LINE__);
    return OMX_ErrorInvalidState;
  }

  pthread_mutex_lock(&m_abortlock);
  if (!m_abort_flag) {
    lret = configureBuffers(a_inBuffer, a_outBuffer);
    if (lret != OMX_ErrorNone) {
      QIDBG_ERROR("%s:%d] Error in Encode buffer configuration", __func__, __LINE__);
      goto error;
    }
    lret = configureDecodeData();
    if (lret != OMX_ErrorNone) {
      QIDBG_ERROR("%s:%d] Error in Encode configuration", __func__, __LINE__);
      goto error;
    }
    lret = startDecode();
  }

  error:
  pthread_mutex_unlock(&m_abortlock);
  return lret;
}

int OMXJpegDecoder::jpeg2QISubsampling(float *sf, jpeg_subsampling_t ss)
{
  switch(ss) {
  case JPEG_H2V2:
    *sf = 1.5;
    return QI_H2V2;
    break;
  case JPEG_H2V1:
    *sf = 2;
    return QI_H2V1;
    break;
  case JPEG_H1V2:
    *sf = 2;
    return QI_H1V2;
    break;
  case JPEG_H1V1:
    *sf = 3;
    return QI_H1V1;
  default:
    return -1;
  }
}

/*==============================================================================
 * Function : decodeImageHeader
 * Parameters: @a_inBuffer- Input buffer passed during empty this buffer
 *             @a_outBuffer - O/p buffer passed during fill this buffer
 * Return Value : OMX_ERRORTYPE
 * Description: Decodes the J
==============================================================================*/
OMX_ERRORTYPE OMXJpegDecoder::decodeImageHeader(OMX_BUFFERHEADERTYPE *a_inBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lrc = QI_SUCCESS;
  jpeg_header_t *lJpegHeader;
  float lImgSzFactor;

  if (!a_inBuffer) {
    QIDBG_ERROR("%s:%d] Bad parameter",  __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s:%d] Invalid State",  __func__, __LINE__);
    return OMX_ErrorInvalidState;
  }


  pthread_mutex_lock(&m_abortlock);
  if (!m_abort_flag) {
    // Configure input buffer
    lret = configureInBuffer(a_inBuffer);
    if (lret != OMX_ErrorNone) {
       QIDBG_ERROR("%s:%d] Invalid OMX_ErrorNone state",  __func__, __LINE__);
       goto error;
    }
    //Create the exif parser
    m_parser = QExifParser::New(*this);
    if (m_parser == NULL) {
      QIDBG_ERROR("%s:%d] failed to create exif parser", __func__, __LINE__);
      lret = OMX_ErrorUndefined;
      goto error;
    }

    lrc = m_parser->addBuffer(m_inputQIBuffer);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      lret = OMX_ErrorUndefined;
      goto error;
    }

    lrc = m_parser->Start();
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      lret = OMX_ErrorUndefined;
      goto error;
    }

    lJpegHeader = m_parser->GetHeader();
    m_mainFrameInfo = lJpegHeader->p_main_frame_info;
    m_outImgSize =  QISize(m_mainFrameInfo->width, m_mainFrameInfo->height);
    m_outImgSubsampling = (QISubsampling)jpeg2QISubsampling(&lImgSzFactor,
        m_mainFrameInfo->subsampling);
    QISubsampling lsubsample;
    translateFormat(m_inPort->format.image.eColorFormat, &m_outImgFormat,
      &lsubsample);

    m_outImgLength = CEILING16(m_mainFrameInfo->width) *
                     CEILING16(m_mainFrameInfo->height) *
                     lImgSzFactor;

    QIDBG_HIGH("%s:%d] Encoded Image is %dx%d fmt %d SS %d ", __func__, __LINE__,
        m_outImgSize.Width(), m_outImgSize.Height(),m_outImgFormat,
        m_outImgSubsampling);
  }

  error:
  pthread_mutex_unlock(&m_abortlock);
  return lret;
}

int OMXJpegDecoder::DecodeComplete(QImage *aOutputImage)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QIMessage *lmessage = NULL;

  QI_LOCK(&m_abortlock);

  if (!m_abort_flag) {
    QIMessage *lEtbMessage = new QIMessage();
    if (!lEtbMessage) {
      QI_UNLOCK(&m_abortlock);
      return QI_ERR_NO_MEMORY;
    }

    lEtbMessage->m_qMessage = OMX_MESSAGE_ETB_DONE;
    m_currentInBuffHdr->nFilledLen = 0;
    lEtbMessage->pData = m_currentInBuffHdr;
    postMessage(lEtbMessage);

    //Post FBD message to the message queue
    QIMessage *lfbdMessage = new QIMessage();
    if (!lfbdMessage) {
      QIDBG_ERROR("%s:%d] Could not allocate QIMessage", __func__,  __LINE__);
      QI_UNLOCK(&m_abortlock);
      return QI_ERR_NO_MEMORY;
    }

    //Set the filled length of the ouput buffer
    m_currentOutBuffHdr->nFilledLen = m_outputImage->FilledLen();
    QIDBG_HIGH("%s:%d] Decoded image length %d", __func__, __LINE__,
        (int)m_currentOutBuffHdr->nFilledLen);

    lfbdMessage->m_qMessage = OMX_MESSAGE_FTB_DONE;
    lfbdMessage->pData = m_currentOutBuffHdr;
    lret = postMessage(lfbdMessage);
    if (QI_ERROR(lret)) {
      QIDBG_ERROR("%s:%d] Could not send FBD", __func__,  __LINE__);
      QI_UNLOCK(&m_abortlock);
      return QI_ERR_NO_MEMORY;
    }
  }

  QI_UNLOCK(&m_abortlock);

  return QI_SUCCESS;
}

/*==============================================================================
* Function : DecodeError
* Parameters: None
* Return Value : int
* Description: This function is called from the JPEG component when there is an
* error in encoding
==============================================================================*/
int OMXJpegDecoder::DecodeError(DecodeErrorType aErrorType)
{
  int lrc = QI_SUCCESS;
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QIDBG_MED("%s:%d] Error %d", __func__, __LINE__, aErrorType);
  pthread_mutex_lock(&m_abortlock);
  if (!m_abort_flag) {
    m_executionComplete = OMX_TRUE;
    QIMessage *lErrMessage = new QIMessage();
    lErrMessage->m_qMessage = OMX_MESSAGE_EVENT_ERROR;
    lErrMessage->iData = OMX_ErrorUndefined;
    lret = postMessage(lErrMessage);
    if (lret != OMX_ErrorNone) {
      QIDBG_ERROR("%s %d: Error posting message", __func__, __LINE__);
      lrc = QI_ERR_GENERAL;
    }
  }
  pthread_mutex_unlock(&m_abortlock);
  return lrc;
}

/*==============================================================================
* Function : releaseCurrentSession
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Release all buffers associated with the current snapshot.
* Decoding should be stopped before calling this function. If not it will
* lead to a crash.
==============================================================================*/
OMX_ERRORTYPE OMXJpegDecoder::releaseCurrentSession()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] E", __func__, __LINE__);

  if (m_mainDecoder) {
    delete m_mainDecoder;
    m_mainDecoder = NULL;
  }
  if (m_inputQIBuffer) {
    delete m_inputQIBuffer;
    m_inputQIBuffer = NULL;
  }
  if (m_outputQIBuffer) {
    delete m_outputQIBuffer;
    m_outputQIBuffer = NULL;
  }
  if (m_inputImage) {
    delete m_inputImage;
    m_inputImage = NULL;
  }
  if (m_outputImage) {
    delete m_outputImage;
    m_outputImage = NULL;
  }
  if (m_parser) {
    delete m_parser;
    m_parser = NULL;
  }

  return lret;
}

