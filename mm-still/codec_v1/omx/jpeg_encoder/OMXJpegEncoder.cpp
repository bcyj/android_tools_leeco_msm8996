/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#define ATRACE_TAG ATRACE_TAG_CAMERA
#include <cutils/trace.h>
#include "OMXJpegEncoder.h"
#include <cutils/properties.h>


/*==============================================================================
* Function : getInstance
* Parameters: None
* Return Value : void * - pointer to the class object
* Description: C function that returns an instance of the class.
==============================================================================*/
extern "C" void *getInstance()
{
  void *lobj = new OMXJpegEncoder();
  QIDBG_HIGH("%s:%d] Component ptr is %p", __func__, __LINE__, lobj);
  return lobj;
}

/*==============================================================================
* Function : configureEncodedata
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Configure the encode parmeters
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::configureEncodedata()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  uint32_t l_imageSize;
  QICrop l_crop;

  //Start configuration only if the abort flag is not set
  m_inputSize.setHeight((int)m_inPort->format.image.nFrameHeight);
  m_inputSize.setWidth((int)m_inPort->format.image.nFrameWidth);
  m_inputPadSize.setHeight((int)m_inPort->format.image.nSliceHeight);
  m_inputPadSize.setWidth((int)m_inPort->format.image.nStride);

  //Translate from OMX format to QIformat
  lret = translateFormat(m_inPort->format.image.eColorFormat, &m_format,
    &m_subsampling);
  if (lret != OMX_ErrorNone) {
    return lret;
  }
  QIDBG_HIGH("%s:%d] size %dx%d pad %dx%d subsampling %d format %d",
    __func__, __LINE__,
    m_inputSize.Width(), m_inputSize.Height(),
    m_inputPadSize.Width(), m_inputPadSize.Height(),
    m_subsampling, m_format);

  //Set num of planes
  m_mainEncodeParams.setNumOfComponents(m_numOfComponents);

  l_imageSize = QImage::getImageSize(m_inputSize, m_subsampling, m_format);
  QIDBG_HIGH("%s:%d] ImageSize %d %d", __func__, __LINE__, l_imageSize,
    m_inputSize.Length());

  //Create the exif composer
  if (NULL == m_composer) {
    m_composer = QExifComposer::New(*this);
    if (m_composer == NULL) {
      QIDBG_ERROR("%s:%d] failed to create exif composer", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
  }

  //Set Crop Info
  if ((m_inputScaleInfo.nHeight) != 0 && (m_inputScaleInfo.nWidth != 0) ) {
    l_crop.setCrop((int)m_inputScaleInfo.nLeft, (int)m_inputScaleInfo.nTop,
      (int)(m_inputScaleInfo.nLeft + m_inputScaleInfo.nWidth),
      (int)(m_inputScaleInfo.nTop + m_inputScaleInfo.nHeight));
    m_mainEncodeParams.setCrop(l_crop);
  }

  /*Set output size _ if scale is enabled set the output
  size to be size after scaling*/
  if ((m_outputScaleInfo.nHeight != 0) && (m_outputScaleInfo.nWidth != 0)) {
    m_outputSize[0].setWidth((int)m_outputScaleInfo.nWidth);
    m_outputSize[0].setHeight((int)m_outputScaleInfo.nHeight);
    QIDBG_HIGH("%s:%d] Scaling enabled o/p width: %d o/p height:%d",
      __func__, __LINE__, m_outputSize[0].Width(), m_outputSize[0].Height());
  } else {
    m_outputSize[0] = m_inputSize;
    QIDBG_HIGH("%s:%d] Scaling not enabled width: %d height:%d",
      __func__, __LINE__, m_outputSize[0].Width(), m_outputSize[0].Height());
  }
  m_mainEncodeParams.setOutputSize(m_outputSize[0]);
  m_mainEncodeParams.setInputSize(m_inputSize);
  m_mainEncodeParams.setRestartInterval(0);
  m_mainEncodeParams.setRotation((uint32_t)m_rotation.nRotation);
  m_mainEncodeParams.setQuality((uint32_t)m_qualityfactor.nQFactor);

  if (m_quantTable.eQuantizationTable) {
    //ToDo: Uncomment and assign appropriately
    //m_encodeParams.setQuantTables(m_quantTable);
  } else {
    m_mainEncodeParams.setDefaultTables(m_mainEncodeParams.Quality());
  }
  if (m_huffmanTable.eHuffmanTable) {
    //ToDo: Uncomment and assign appropriately
    //m_encodeParams.setHuffTables(m_huffmanTable);
  }
  m_mainEncodeParams.setHiSpeed(m_JpegSpeedMode.speedMode ==
    QOMX_JPEG_SPEED_MODE_HIGH);

  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  return lret;
}

/*==============================================================================
* Function : configureEncodedata
* Parameters: a_inBuffer, a_outBuffer
* Return Value : OMX_ERRORTYPE
* Description: Configure the input and output buffers
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::configureBuffers(
  OMX_BUFFERHEADERTYPE *a_inBuffer,
  OMX_BUFFERHEADERTYPE *a_outBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_BUFFER_INFO *lInBufferInfo = NULL;
  QOMX_BUFFER_INFO *lOutBufferInfo = NULL;

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

  QIDBG_HIGH("%s:%d] MAIN buf=%p, fd=%lu, offset=%lu filled=%lu", __func__, __LINE__,
    a_inBuffer->pBuffer,
    lInBufferInfo->fd,
    lInBufferInfo->offset,
    a_inBuffer->nFilledLen);

  return lret;

}

/*==============================================================================
* Function : configureTmbBuffer
* Parameters: a_inBuffer, a_outBuffer
* Return Value : OMX_ERRORTYPE
* Description: Configure input thumbnail buffer
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::configureTmbBuffer(
  OMX_BUFFERHEADERTYPE *a_inTmbBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_BUFFER_INFO *lInBufferInfo = NULL;
  QOMX_BUFFER_INFO *lOutBufferInfo = NULL;

  lInBufferInfo = reinterpret_cast<QOMX_BUFFER_INFO *>
    (a_inTmbBuffer->pOutputPortPrivate);
  m_inputQTmbBuffer = new QIBuffer((uint8_t*)a_inTmbBuffer->pBuffer,
      a_inTmbBuffer->nAllocLen);
  if (!m_inputQTmbBuffer) {
    QIDBG_ERROR("%s:%d] Error allocating i/p QIBuffer", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  m_inputQTmbBuffer->SetFd((int)lInBufferInfo->fd);
  m_inputQTmbBuffer->SetOffset(lInBufferInfo->offset);
  m_inputQTmbBuffer->SetFilledLen(a_inTmbBuffer->nFilledLen);

  QIDBG_HIGH("%s:%d] TMB buf=%p, fd=%lu, offset=%lu, filled=%lu", __func__, __LINE__,
    a_inTmbBuffer->pBuffer,
    lInBufferInfo->fd,
    lInBufferInfo->offset,
    a_inTmbBuffer->nFilledLen);


  return lret;
}

/*==============================================================================
* Function : writeExifData
* Parameters: QImage *aThumbnail - The thumbnail buffer after encoding. Null
* when thumbnail is not encoded.
* Return Value : OMX_ERRORTYPE
* Description: Add the exif data and encoded thumbnail to the o/p buffer
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::writeExifData(QImage *aThumbnail,
  QIBuffer *aOutputBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lrc = 0;
  int enable_mobicat = 0;

#ifdef PROCESS_METADATA
  lret = processMetadata();

  if (OMX_ErrorNone != lret) {
    QIDBG_ERROR("%s:%d] processMetadata failed", __func__, __LINE__);
    return lret;
  }
#endif

  char value [32];
  property_get("persist.camera.mobicat", value, "0");
  enable_mobicat = atoi(value);

  //enable mobicat
  if (enable_mobicat > 0) {
    m_mobicatComposer = new QMobicatComposer();
    if (m_mobicatComposer == NULL) {
      QIDBG_ERROR("%s:%d] failed to create mobicat composer", __func__,
        __LINE__);
      return OMX_ErrorInsufficientResources;
    }

    if (enable_mobicat == PARSE_MOBICAT_DATA ||
      enable_mobicat == PARSE_MOBICAT_AND_3A_DATA) {
      char* mobicatStr = m_mobicatComposer->
        ParseMobicatData(m_Metadata.metadata);
      if (mobicatStr == 0) {
        QIDBG_ERROR("%s:%d] Error in composing mobicat string",
          __func__, __LINE__);
        return OMX_ErrorUndefined;
      }
      QIDBG_ERROR("%s:%d] m_mobicat.size %d", __func__, __LINE__,
        (int) strlen(mobicatStr));

      m_exifParams.SetMobicatFlag(true);
      m_exifParams.SetMobicat(mobicatStr);
    }
    if (enable_mobicat == PARSE_3A_DATA ||
      enable_mobicat == PARSE_MOBICAT_AND_3A_DATA) {
      char* stats_payload = m_mobicatComposer->
        Compose3AStatsPayload(m_Metadata.metadata);
      uint32_t stats_payload_size =
        m_mobicatComposer->Get3AStatsSize();

      if (stats_payload_size == 0) {
        QIDBG_ERROR("%s:%d] [MOBICAT_DBG] Stats debug payload size is 0",
          __func__, __LINE__);
      } else {
         m_exifParams.Set3AFlag(true);
         m_exifParams.Set3A(stats_payload, stats_payload_size);
      }
    }
  }

  lrc = m_composer->addBuffer(aOutputBuffer);

  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }
  m_exifParams.SetAppHeaderLen(0);
  if (aThumbnail) {
    m_exifParams.SetEncodeParams(m_thumbEncodeParams, true);
    m_exifParams.SetSubSampling(m_thumbSubsampling, true);
  }
  m_exifParams.SetEncodeParams(m_mainEncodeParams);
  m_exifParams.SetExif(&m_exifInfoObj);
  m_exifParams.SetSubSampling(m_subsampling);

  lrc = m_composer->SetParams(m_exifParams);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  lrc = m_composer->Start(aThumbnail);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }
  return lret;
}

/*==============================================================================
* Function : startEncode
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Get the encoder from the factory and start encoding
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::startEncode()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QImageCodecFactory::QCodecPrefType lCodecPref =
  QImageCodecFactory::HW_CODEC_PREF;
  int lrc = 0;
  QIBuffer *lOutBuf;

  //Set the offset for each plane
  uint32_t lOffset[OMX_MAX_NUM_PLANES] = {m_imageBufferOffset.yOffset,
    m_imageBufferOffset.cbcrOffset[0], m_imageBufferOffset.cbcrOffset[1]};

  //Set the physical offset for each plane
  uint32_t lPhyOffset[QI_MAX_PLANES] = {0,
    m_imageBufferOffset.cbcrStartOffset[0],
    m_imageBufferOffset.cbcrStartOffset[1]};

  for (int i = 0; i < 2; i++) {
    //Get the appropriate Encoder from the factory
    if (NULL == m_mainEncoder) {
      m_mainEncoder = m_factory.CreateEncoder(lCodecPref,
          m_mainEncodeParams);
      if (m_mainEncoder == NULL) {
        QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
        return OMX_ErrorInsufficientResources;
      }
    }
    m_inputMainImage = new QImage(m_inputPadSize, m_subsampling, m_format,
      m_inputSize);
    if (m_inputMainImage == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }

    lrc = m_inputMainImage->setDefaultPlanes(m_numOfPlanes,
        m_inputQIBuffer->Addr(), m_inputQIBuffer->Fd(), lOffset, lPhyOffset);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    QIBuffer lIONBuffer = QIBuffer(m_IONBuffer.vaddr, m_IONBuffer.length);
    lIONBuffer.SetFilledLen(0);
    lIONBuffer.SetFd(m_IONBuffer.fd);

    if ((m_outputQIBuffer->Fd() < 0) && (lIONBuffer.Fd() > -1)) {
      lOutBuf = &lIONBuffer;
    } else {
      lOutBuf = m_outputQIBuffer;
    }

    m_outputMainImage = new QImage(lOutBuf->Addr() +
        lOutBuf->FilledLen(),
        lOutBuf->Length() - lOutBuf->FilledLen(),
        QI_BITSTREAM);

    if (m_outputMainImage == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }

    m_outputMainImage->setFd(lOutBuf->Fd());

    lrc = m_mainEncoder->SetOutputMode(QImageEncoderInterface::ENORMAL_OUTPUT);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    lrc = m_mainEncoder->setEncodeParams(m_mainEncodeParams);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    lrc = m_mainEncoder->addInputImage(*m_inputMainImage);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    if (m_IONBuffer.length > 0) {
      m_outputMainImage->setWorkBufSize(m_IONBuffer.length);
    } else {
      m_outputMainImage->setWorkBufSize(m_outputMainImage->Length());
    }

    lrc = m_mainEncoder->addOutputImage(*m_outputMainImage);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    lrc = m_mainEncoder->addObserver(*this);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    QIDBG_ERROR("%s:%d] startEncode()", __func__, __LINE__);
    ATRACE_INT("Camera:JPEG:encode", 1);
    m_mainImageEncoding = OMX_TRUE;
    lrc = m_mainEncoder->Start();
    if (!lrc) {
      lret = OMX_ErrorNone;
      m_mainImageEncoding = OMX_TRUE;
      break;
    } else {
      delete m_mainEncoder;
      m_mainEncoder = NULL;
      lret = OMX_ErrorUndefined;
      lCodecPref = QImageCodecFactory::SW_CODEC_ONLY;
      ATRACE_INT("Camera:JPEG:encode", 0);
      QIDBG_ERROR("%s:%d] Main Image encoding failed to start, "
        "switching to alternative encoder",__func__, __LINE__);

      continue;
    }
  }

  return lret;
}

/*==============================================================================
* Function : CompleteMainImage
* Parameters: QImage
* Return Value : int
* Description: Completes main image encoding.
==============================================================================*/
int OMXJpegEncoder::CompleteMainImage()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  /* Post ETB Done and FTB Done to the queue since we dont want to do a
     callback with the Event thread from the codec layer */
  QI_LOCK(&m_abortlock);

  if (!m_abort_flag && (OMX_FALSE == m_releaseFlag)) {
    //Post EBD to the message queue
    QIMessage *lebdMessage = new QIMessage();
    if (!lebdMessage) {
      QIDBG_ERROR("%s:%d] Could not alloate QIMessage", __func__,  __LINE__);
      QI_UNLOCK(&m_abortlock);
      return QI_ERR_NO_MEMORY;
    }
    lebdMessage->m_qMessage = OMX_MESSAGE_ETB_DONE;
    //The i/p buffer has been consumed completely. Set the nFilledLen to 0x0
    m_currentInBuffHdr->nFilledLen = 0;
    lebdMessage->pData = m_currentInBuffHdr;

    //Post FBD message to the message queue
    QIMessage *lfbdMessage = new QIMessage();
    if (!lfbdMessage) {
      QIDBG_ERROR("%s:%d] Could not allocate QIMessage", __func__,  __LINE__);
      QI_UNLOCK(&m_abortlock);
      return QI_ERR_NO_MEMORY;
    }
    if (NULL != m_memOps.get_memory) {
      omx_jpeg_ouput_buf_t *jpeg_out =
        (omx_jpeg_ouput_buf_t *)m_outputQIBuffer->Addr();

      if ( (m_outputMainImage->FilledLen() + getEstimatedExifSize()) <
          m_outputMainImage->Length() ) {

        QIBuffer lApp1Buf = QIBuffer((uint8_t*)m_outputMainImage->BaseAddr() +
            m_outputMainImage->FilledLen(),
            m_outputMainImage->Length() - m_outputMainImage->FilledLen());

        if ((m_thumbnailInfo.input_height != 0) &&
            (m_thumbnailInfo.input_width != 0)) {
          lret = writeExifData(m_outThumbImage, &lApp1Buf);
        } else {
          lret = writeExifData(NULL, &lApp1Buf);
        }

        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d ", __func__,  __LINE__);
          QI_UNLOCK(&m_abortlock);
          return QI_ERR_GENERAL;
        }

        jpeg_out->size = lApp1Buf.FilledLen() + m_outputMainImage->FilledLen();
        m_memOps.get_memory(jpeg_out);

        if (!jpeg_out->vaddr) {
          QIDBG_ERROR("%s:%d get_memory failed", __func__,  __LINE__);
          return QI_ERR_GENERAL;
        }

        ATRACE_BEGIN("Camera:JPEG:memcpy");
        memcpy(jpeg_out->vaddr, m_outputMainImage->BaseAddr() +
            m_outputMainImage->FilledLen(), lApp1Buf.FilledLen());
        memcpy(jpeg_out->vaddr + lApp1Buf.FilledLen(), m_outputMainImage->BaseAddr(),
            m_outputMainImage->FilledLen());
        ATRACE_END();
        //Set the filled length of the ouput buffer
        m_currentOutBuffHdr->nFilledLen = m_outputMainImage->FilledLen() +
            lApp1Buf.FilledLen();
      } else {
        QIDBG_HIGH("%s:%d Allocating extra temp buffer for Exif ", __func__,  __LINE__);
        uint8_t *exif_buf = (uint8_t*)malloc (getEstimatedExifSize());
        if (exif_buf == NULL) {
          QIDBG_ERROR("%s:%d exif mem alloc failed", __func__,  __LINE__);
          return QI_ERR_GENERAL;
        }
        QIBuffer lApp1Buf = QIBuffer((uint8_t*)exif_buf, getEstimatedExifSize());

        if ((m_thumbnailInfo.input_height != 0) &&
            (m_thumbnailInfo.input_width != 0)) {
          lret = writeExifData(m_outThumbImage, &lApp1Buf);
        } else {
          lret = writeExifData(NULL, &lApp1Buf);
        }

        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d ", __func__,  __LINE__);
          QI_UNLOCK(&m_abortlock);
          free (exif_buf);
          return QI_ERR_GENERAL;
        }

        jpeg_out->size = lApp1Buf.FilledLen() + m_outputMainImage->FilledLen();
        m_memOps.get_memory(jpeg_out);

        if (!jpeg_out->vaddr) {
          QIDBG_ERROR("%s:%d get_memory failed", __func__,  __LINE__);
          free (exif_buf);
          return QI_ERR_GENERAL;
        }

        ATRACE_BEGIN("Camera:JPEG:memcpy");
        memcpy(jpeg_out->vaddr, exif_buf, lApp1Buf.FilledLen());
        memcpy(jpeg_out->vaddr + lApp1Buf.FilledLen(),
            m_outputMainImage->BaseAddr(), m_outputMainImage->FilledLen());
        ATRACE_END();
        //Set the filled length of the ouput buffer
        m_currentOutBuffHdr->nFilledLen = m_outputMainImage->FilledLen() +
            lApp1Buf.FilledLen();
        free (exif_buf);
      }
    } else if (m_outputQIBuffer->Addr() != m_outputMainImage->BaseAddr()) {
      ATRACE_BEGIN("Camera:JPEG:memcpyQI");
      memcpy(m_outputQIBuffer->Addr() + m_outputQIBuffer->FilledLen(),
        m_outputMainImage->BaseAddr(), m_outputMainImage->FilledLen());
      ATRACE_END();
      //Set the filled length of the ouput buffer
      m_currentOutBuffHdr->nFilledLen = m_outputMainImage->FilledLen() +
          m_outputQIBuffer->FilledLen();
    }


    QIDBG_HIGH("%s:%d] Encoded image length %d", __func__, __LINE__,
      (int)m_currentOutBuffHdr->nFilledLen);

    lfbdMessage->m_qMessage = OMX_MESSAGE_FTB_DONE;
    lfbdMessage->pData = m_currentOutBuffHdr;

    lret = postMessage(lebdMessage);
    if (QI_ERROR(lret)) {
      QIDBG_ERROR("%s:%d] Could not send EBD", __func__,  __LINE__);
      delete lebdMessage;
      delete lfbdMessage;
      QI_UNLOCK(&m_abortlock);
      return QI_ERR_NO_MEMORY;
    }

    lret = postMessage(lfbdMessage);
    if (QI_ERROR(lret)) {
      QIDBG_ERROR("%s:%d] Could not send FBD", __func__,  __LINE__);
      delete lfbdMessage;
      QI_UNLOCK(&m_abortlock);
      return QI_ERR_NO_MEMORY;
    }

    // Send message for a new encode process
    QIMessage *lEncodeMessage = new QIMessage();
    if (!lebdMessage) {
      QIDBG_ERROR("%s:%d] Could not alloate QIMessage", __func__,  __LINE__);
      QI_UNLOCK(&m_abortlock);
      return QI_ERR_NO_MEMORY;
    }
    lEncodeMessage->m_qMessage = OMX_MESSAGE_START_NEW_ENCODE;
    lret = postMessage(lEncodeMessage);
    if (QI_ERROR(lret)) {
      QIDBG_ERROR("%s:%d] Could not send Start encode", __func__,  __LINE__);
      delete lEncodeMessage;
      QI_UNLOCK(&m_abortlock);
      return QI_ERR_NO_MEMORY;
    }

  }
  QI_UNLOCK(&m_abortlock);

  return QI_SUCCESS;
}
/*==============================================================================
* Function : EncodeComplete
* Parameters: None
* Return Value : int
* Description: This function is called from the JPEG component when encoding is
* complete
==============================================================================*/
int OMXJpegEncoder::EncodeComplete(QImage *aOutputImage)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QIMessage *lmessage = NULL;

  QI_LOCK(&mEncodeDoneLock);
  if ((m_thumbEncoding == OMX_TRUE) && (m_outThumbImage != NULL) &&
    m_outThumbImage->BaseAddr() == aOutputImage->BaseAddr()) {
    ATRACE_INT("Camera:thumbnail", 0);
    QIDBG_HIGH("%s:%d] Thumbnail Encoding complete.",
      __func__, __LINE__);
    m_thumbEncoding = OMX_FALSE;
    m_thumbEncodingComplete = OMX_TRUE;

    if (NULL == m_memOps.get_memory) {
      lret = writeExifData(aOutputImage, m_outputQIBuffer);
      QIDBG_ERROR("%s:%d] Exif length: %d", __func__,  __LINE__,
        m_outputQIBuffer->FilledLen());
      if (QI_ERROR(lret)) {
        goto error;
      }
    }

    /* send ETB for thumbnail */
    QIMessage *lEtbMessage = new QIMessage();
    if (!lEtbMessage) {
      QIDBG_ERROR("%s:%d] Could not allocate QIMessage", __func__,  __LINE__);
      goto error_nomem;
    }
    lEtbMessage->m_qMessage = OMX_MESSAGE_ETB_DONE;
    lEtbMessage->pData = m_currentInTmbBuffHdr;
    postMessage(lEtbMessage);

    if (m_encoding_mode == OMX_Serial_Encoding) {
      /* Thumbnail exif write successful, Start main image encode */
      lmessage = new QIMessage();
      if (!lmessage) {
        QIDBG_ERROR("%s:%d] Could not allocate QIMessage", __func__,  __LINE__);
        goto error_nomem;
      }
      lmessage->m_qMessage = OMX_MESSAGE_START_MAIN_ENCODE;
      postMessage(lmessage);
      lmessage = NULL;
    } else {
      /* parallel encoding */
      QIDBG_MED("%s:%d] parallel encoding m_mainEncodingComplete %d", __func__,
        __LINE__, m_mainEncodingComplete);

      if (m_outputMainImage != NULL && m_outputMainImage->FilledLen() &&
        (OMX_TRUE == m_mainEncodingComplete)) {
        /* MainImage was finished first, now write MainImage */
        CompleteMainImage();
      }
    }
  } else if (m_outputMainImage != NULL &&
    m_outputMainImage->BaseAddr() == aOutputImage->BaseAddr()) {
    /* main image encoding complete */
    QIDBG_HIGH("%s:%d] MainImage Encoding complete. Filled "
      "Length = %d m_thumbEncodingComplete %d",
      __func__, __LINE__, m_outputMainImage->FilledLen(),
      m_thumbEncodingComplete);
    ATRACE_INT("Camera:JPEG:encode", 0);
    m_mainImageEncoding = OMX_FALSE;
    m_mainEncodingComplete = OMX_TRUE;

    if (m_encoding_mode == OMX_Serial_Encoding) {
      CompleteMainImage();
    } else {
      /* parallel encoding */

      /* thumbnail does not exist OR has already been encoded.
         Write MainImage to EXIF*/
      if (!m_inTmbPort->bEnabled ||
        (m_outThumbImage != NULL && m_outThumbImage->FilledLen()
        && (OMX_TRUE == m_thumbEncodingComplete))) {
        CompleteMainImage();
      }
    }
  }

  QI_UNLOCK(&mEncodeDoneLock);
  return QI_SUCCESS;

error:
  QI_UNLOCK(&mEncodeDoneLock);
  /* Propagate error */
  lmessage = new QIMessage();
  if (lmessage) {
    lmessage->m_qMessage = OMX_MESSAGE_EVENT_ERROR;
    lmessage->iData = lret;
    postMessage(lmessage);
  }
  return QI_ERR_GENERAL;

error_nomem:
  /* TBD: Propagate error */
  QI_UNLOCK(&mEncodeDoneLock);
  return QI_ERR_NO_MEMORY;
}

/*==============================================================================
* Function : EncodeError
* Parameters: None
* Return Value : int
* Description: This function is called from the JPEG component when there is an
* error in encoding
==============================================================================*/
int OMXJpegEncoder::EncodeError(EncodeErrorType aErrorType)
{
  int lrc = QI_SUCCESS;
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QIDBG_MED("%s:%d] Error %d", __func__, __LINE__, aErrorType);
  pthread_mutex_lock(&m_abortlock);
  if (!m_abort_flag) {
    m_executionComplete = OMX_TRUE;
    QIMessage *lErrMessage = new QIMessage();
    lErrMessage->m_qMessage = OMX_MESSAGE_EVENT_ERROR;
    if (ERROR_OVERFLOW == aErrorType) {
      lErrMessage->iData = OMX_ErrorOverflow;
    } else {
      lErrMessage->iData = OMX_ErrorUndefined;
    }
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
* Function : OutputFragment
* Parameters: QIBuffer &aBuffer
* Return Value : int
* Description:
==============================================================================*/
int OMXJpegEncoder::OutputFragment(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*==============================================================================
* Function : WriteError
* Parameters: ErrorType aErrorType
* Return Value : int
* Description:
==============================================================================*/
void OMXJpegEncoder::WriteError(ErrorType aErrorType)
{
   QIDBG_MED("%s:%d] ", __func__, __LINE__);
}
/*==============================================================================
* Function : WriteComplete
* Parameters: uint8_t *aBuffer, uint32_t aSize
* Return Value : int
* Description:
==============================================================================*/
void OMXJpegEncoder::WriteComplete(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*==============================================================================
* Function : WriteFragmentDone
* Parameters: QIBuffer &aBuffer
* Return Value : void
* Description:
==============================================================================*/
void OMXJpegEncoder::WriteFragmentDone(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*==============================================================================
* Function : configureThumbnailData
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Configure the encode parameters for thumbnail
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::configureThumbnailData()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QICrop l_crop;
  uint32_t l_imageSize;

  //Start configuration only if the abort flag is not set
  m_inputTmbSize.setHeight((int)m_inTmbPort->format.image.nFrameHeight);
  m_inputTmbSize.setWidth((int)m_inTmbPort->format.image.nFrameWidth);
  m_inputTmbPadSize.setHeight((int)m_inTmbPort->format.image.nSliceHeight);
  m_inputTmbPadSize.setWidth((int)m_inTmbPort->format.image.nStride);

  QIDBG_HIGH("%s:%d] size %dx%d pad %dx%d subsampling %d",
    __func__, __LINE__,
    m_inputTmbSize.Width(), m_inputTmbSize.Height(),
    m_inputTmbPadSize.Width(), m_inputTmbPadSize.Height(),
    m_subsampling);

  m_thumbEncodeParams.setNumOfComponents(m_numOfComponents);

  //Set Crop Info if cropping is enabled
  if ((m_thumbnailInfo.crop_info.nWidth) != 0 &&
    (m_thumbnailInfo.crop_info.nHeight != 0) ) {
    l_crop.setCrop((int)m_thumbnailInfo.crop_info.nLeft,
      (int)m_thumbnailInfo.crop_info.nTop,
      (int)(m_thumbnailInfo.crop_info.nLeft + m_thumbnailInfo.crop_info.nWidth),
      (int)(m_thumbnailInfo.crop_info.nTop +
      m_thumbnailInfo.crop_info.nHeight));
      m_thumbEncodeParams.setCrop(l_crop);
  }

  //Translate from OMX format to QIformat
  lret = translateFormat(m_inTmbPort->format.image.eColorFormat, &m_thumbFormat,
    &m_thumbSubsampling);
  if (lret != OMX_ErrorNone) {
    return lret;
  }

   /*Set output size if scale is enabled set the output
  size to be size after scaling*/
  if ((m_thumbnailInfo.output_width != 0) &&
    (m_thumbnailInfo.output_height != 0)) {
     m_outputSize[1].setWidth((int)m_thumbnailInfo.output_width);
     m_outputSize[1].setHeight((int)m_thumbnailInfo.output_height);
     QIDBG_HIGH("%s:%d] Thumbnail Scaling enabled o/p width: %d o/p height:%d",
       __func__, __LINE__, m_outputSize[1].Width(), m_outputSize[1].Height());
  } else {
    m_outputSize[1].setWidth((int)m_inTmbPort->format.image.nFrameWidth);
    m_outputSize[1].setHeight((int)m_inTmbPort->format.image.nFrameHeight);
    QIDBG_HIGH("%s:%d] Thumbnail Scaling not enabled width: %d height:%d",
      __func__, __LINE__, m_outputSize[1].Width(), m_outputSize[1].Height());
  }

  m_thumbEncodeParams.setOutputSize(m_outputSize[1]);
  m_thumbEncodeParams.setInputSize(m_inputTmbSize);
  m_thumbEncodeParams.setRestartInterval(0);
  m_thumbEncodeParams.setRotation((uint32_t)m_thumbnailInfo.rotation);
  m_thumbEncodeParams.setQuality(DEFAULT_THUMB_Q_FACTOR);
  m_thumbEncodeParams.setDefaultTables(m_thumbEncodeParams.Quality());
  m_thumbEncodeParams.setHiSpeed(m_JpegSpeedMode.speedMode ==
    QOMX_JPEG_SPEED_MODE_HIGH);
  return lret;
}

/*==============================================================================
* Function : startThumbnailEncode
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Start Thumbnail Encode
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::startThumbnailEncode()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lrc = 0;

  QOMX_YUV_FRAME_INFO *lbufferOffset = &m_thumbnailInfo.tmbOffset;
  if (!m_inTmbPort->bEnabled) {
    lbufferOffset = &m_imageBufferOffset;
    QIDBG_ERROR("%s:%d] TMB PORT IS NOT ENABLED", __func__, __LINE__);
  }

  //Set the offset for each plane
  uint32_t lOffset[OMX_MAX_NUM_PLANES] = {lbufferOffset->yOffset,
    lbufferOffset->cbcrOffset[0] , lbufferOffset->cbcrOffset[1]};

  //Set the physical offset for each plane
  uint32_t lPhyOffset[QI_MAX_PLANES] = {0,
    lbufferOffset->cbcrStartOffset[0],
    lbufferOffset->cbcrStartOffset[1]};

  if (NULL == m_thumbEncoder) {
    if (m_thumbFormat == QI_MONOCHROME) {
      QIDBG_MED("%s:%d] Monochrome thumbnail format, switching to HW encoder",
        __func__, __LINE__);
      m_thumbEncoder = m_factory.CreateEncoder(QImageCodecFactory::HW_CODEC_ONLY,
        m_thumbEncodeParams);
    } else {
      m_thumbEncoder = m_factory.CreateEncoder(QImageCodecFactory::SW_CODEC_ONLY,
      m_thumbEncodeParams);
    }
    if (m_thumbEncoder == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
  }

  m_inThumbImage = new QImage(m_inputTmbPadSize, m_thumbSubsampling, m_thumbFormat,
    m_inputTmbSize);
  if (m_inThumbImage == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }

  lrc = m_inThumbImage->setDefaultPlanes(m_numOfPlanes, m_inputQTmbBuffer->Addr(),
    m_inputQTmbBuffer->Fd(), lOffset, lPhyOffset);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  /* Allocate thumbnail buffer */
  uint32_t lThumbSize = QImage::getImageSize(m_thumbEncodeParams.OutputSize(),
    m_thumbSubsampling, m_thumbFormat);
  QIDBG_MED("%s:%d] lThumbSize %d", __func__, __LINE__, lThumbSize);
  mThumbBuffer = QIHeapBuffer::New(lThumbSize);
  if (mThumbBuffer == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }

  m_outThumbImage = new QImage(mThumbBuffer->Addr(),
    mThumbBuffer->Length(), QI_BITSTREAM);
  if (m_outThumbImage == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }

  m_outThumbImage->SetFilledLen(0);

  lrc = m_thumbEncoder->SetOutputMode(QImageEncoderInterface::ENORMAL_OUTPUT);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  lrc = m_thumbEncoder->setEncodeParams(m_thumbEncodeParams);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  lrc = m_thumbEncoder->addInputImage(*m_inThumbImage);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  if (m_IONBuffer.length > 0) {
    m_outThumbImage->setWorkBufSize(m_IONBuffer.length);
  } else {
    m_outThumbImage->setWorkBufSize(m_outThumbImage->Length());
  }

  lrc = m_thumbEncoder->addOutputImage(*m_outThumbImage);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  lrc = m_thumbEncoder->addObserver(*this);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  m_thumbEncoding = OMX_TRUE;
  QIDBG_HIGH("%s:%d] startThumbnailEncode()", __func__, __LINE__);
  lrc = m_thumbEncoder->Start();
  if (lrc ) {
    m_thumbEncoding = OMX_FALSE;
    QIDBG_ERROR("%s:%d] Thumbnail encoding failed to start",
      __func__, __LINE__);
    return OMX_ErrorUndefined;
  }
  ATRACE_INT("Camera:thumbnail", 1);
  QIDBG_HIGH("%s:%d] Started Thumbnail encoding", __func__, __LINE__);
  return lret;

}

/*==============================================================================
* Function : preloadCodecLibs
* Parameters:
* Return Value : OMX_ERRORTYPE
* Description: pre-load the encoders
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::preloadCodecLibs()
{
  QIDBG_HIGH("%s:%d] E", __func__, __LINE__);

  OMX_ERRORTYPE lret = configureEncodedata();
  if (lret != OMX_ErrorNone) {
    QIDBG_ERROR("%s:%d] Error in Encode configuration", __func__, __LINE__);
    return lret;
  }
  if (NULL == m_mainEncoder) {
    m_mainEncoder = m_factory.CreateEncoder(QImageCodecFactory::HW_CODEC_PREF,
        m_mainEncodeParams);
    if (m_mainEncoder == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
  }

  QIDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : encodeImage
* Parameters: OMX_BUFFERHEADERTYPE *a_inBuffer - Input Buffer passed during
* ETB, OMX_BUFFERHEADERTYPE *a_outBuffer - O/p buffer passed during FTB
* Return Value : OMX_ERRORTYPE
* Description: Start Image Encoding
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::encodeImage(
  OMX_BUFFERHEADERTYPE *a_inBuffer,
  OMX_BUFFERHEADERTYPE *a_inTmbBuffer,
  OMX_BUFFERHEADERTYPE *a_outBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  if (!a_inBuffer || !a_outBuffer) {
    QIDBG_ERROR("%s:%d] Bad parameter",  __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s:%d] Invalid State",  __func__, __LINE__);
    return OMX_ErrorInvalidState;
  }

  pthread_mutex_lock(&m_abortlock);
  m_releaseFlag = OMX_FALSE;
  if (!m_abort_flag) {

    m_mainEncodingComplete = OMX_FALSE;
    if ((m_mainEncodeParams.Rotation() != (uint32_t)m_rotation.nRotation) &&
        (!m_mainEncodeParams.Rotation() || !m_rotation.nRotation)) {
      if (NULL != m_mainEncoder) {
        delete m_mainEncoder;
        m_mainEncoder = NULL;
      }
    }
    lret = configureEncodedata();
    if (lret != OMX_ErrorNone) {
      QIDBG_ERROR("%s:%d] Error in Encode configuration", __func__, __LINE__);
      goto error;
    }
    lret = configureBuffers(a_inBuffer, a_outBuffer);
    if (lret != OMX_ErrorNone) {
      QIDBG_ERROR("%s:%d] Error in Encode buffer configuration", __func__, __LINE__);
      goto error;
    }
    if ((m_thumbnailInfo.input_height != 0) &&
      (m_thumbnailInfo.input_width != 0)) {
      // Configure thumbnail buffer
      m_thumbEncodingComplete = OMX_FALSE;
      lret = configureTmbBuffer(a_inTmbBuffer);
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] Error in Thumbnail bufffer configuration",
            __func__, __LINE__);
        goto error;
      }

      lret = configureThumbnailData();
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] Error in Encode configuration", __func__, __LINE__);
        goto error;
      }

      /* Monochrome can only be encoded with HW encoder. Thus, switch to
         serial and use the HW for both thumbnail and main*/
      if (m_thumbFormat == QI_MONOCHROME) {
         m_encoding_mode = OMX_Serial_Encoding;
      }

      /* If Parallel encoding is enabled, encode both main image and thumbnail
         in parallel. Start main image first followed by thumbnail */
      if (m_encoding_mode == OMX_Parallel_Encoding) {
        lret = startEncode();
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in Start Encode", __func__, __LINE__);
          goto error;
        }
        lret = startThumbnailEncode();
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in Starting thumbnail encode",
            __func__, __LINE__);
          goto error;
        }
      } else {
        /*If serial encoding start thumbnail first*/
        lret = startThumbnailEncode();
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in Starting thumbnail encode",
            __func__, __LINE__);
          goto error;
        }
      }
    } else {
      /*If thumbnail is not present we call write exif from
        here with thumbnail as NULL*/
      if (NULL == m_memOps.get_memory) {
        lret = writeExifData(NULL, m_outputQIBuffer);
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in Exif Composer", __func__, __LINE__);
          goto error;
        }
        QIDBG_MED("%s:%d] Finished writing Exif", __func__, __LINE__);
      }
      lret = startEncode();
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] Error in Start Encode", __func__, __LINE__);
        goto error;
      }
    }
  }

error:
  pthread_mutex_unlock(&m_abortlock);
  return lret;
}


/*==============================================================================
* Function : processMetadata
* Parameters: none
* Return Value : OMX_ERRORTYPE
* Description: Prepare and encrypt makernote data
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::processMetadata()
{
  OMX_ERRORTYPE lRet = OMX_ErrorNone;
  QExifCameraTuningParams lTuningParams;
  uint8_t *lParsedMeta = NULL;
  QCrypt *lCrypto = NULL;
  uint8_t *lEncData = NULL;
  int lCRet = 0;
  int lLen = 0;
  int lMsgLen = 0;

  if (NULL == m_Metadata.metadata) {
    QIDBG_ERROR("%s:%d] Metadata not present",
            __func__, __LINE__);
    return lRet;
  }

  lParsedMeta = new uint8_t[MAX_PARSED_METADATA_SIZE];
  if (NULL == lParsedMeta) {
    QIDBG_ERROR("%s:%d] Failed to allocate metadata buffer",
        __func__, __LINE__);
    lRet = OMX_ErrorInsufficientResources;
    goto cleanup;
  }

  lLen = lTuningParams.ExtractTuningInfo(m_Metadata.metadata, lParsedMeta);
  if (lLen > MAX_PARSED_METADATA_SIZE) {
    QIDBG_ERROR("%s:%d] Parsed metadata output exceeds buffer",
        __func__, __LINE__);
    lRet = OMX_ErrorInsufficientResources;
    goto cleanup;
  }

  lCrypto = QCrypt::New();
  if (NULL == lCrypto) {
    QIDBG_ERROR("%s:%d] Failed to create qcrypt instance",
            __func__, __LINE__);
    lRet = OMX_ErrorInsufficientResources;
    goto cleanup;
  }

  lCRet = lCrypto->setEncKey((char*)m_MetadataEncKey.metaKey, m_MetadataEncKey.keyLen);
  if (QI_SUCCESS != lCRet) {
    QIDBG_ERROR("%s:%d] Failed to set encryption key",
        __func__, __LINE__);
    lRet = OMX_ErrorBadParameter;
    goto cleanup;
  }

  lMsgLen = lCrypto->setEncMsgLen(lLen);
  if (!lMsgLen) {
    QIDBG_ERROR("%s:%d] Failed to set message length",
        __func__, __LINE__);
    lRet = OMX_ErrorBadParameter;
    goto cleanup;
  }

  lEncData = new uint8_t[lMsgLen];
  if (NULL == lEncData) {
    QIDBG_ERROR("%s:%d] Failed to allocate encrypted data buffer",
        __func__, __LINE__);
    lRet = OMX_ErrorInsufficientResources;
    goto cleanup;
  }

  lMsgLen = lCrypto->encrypt(lParsedMeta, lEncData);
  if (!lMsgLen) {
    QIDBG_ERROR("%s:%d] Error during encryption",
        __func__, __LINE__);
    lRet = OMX_ErrorBadParameter;
    goto cleanup;
  }

  QIDBG_ERROR("%s:%d] Encrypted makernote of size %d",
             __func__, __LINE__, lMsgLen);

  exif_tag_entry_t lExifTag;
  lExifTag.type = EXIF_UNDEFINED;
  lExifTag.copy = 1;
  lExifTag.count = lMsgLen;
  lExifTag.data._undefined = lEncData;

  lCRet = exif_set_tag(m_exifInfoObj, EXIFTAGID_EXIF_MAKER_NOTE, &lExifTag);
  if (JPEGERR_SUCCESS != lCRet) {
    QIDBG_ERROR("%s:%d] Failed to set exif tag",
        __func__, __LINE__);
    lRet = OMX_ErrorBadParameter;
    goto cleanup;
  }
cleanup:
  if (NULL != lParsedMeta) {
    delete lParsedMeta;
    lParsedMeta = NULL;
  }
  if (NULL != lCrypto) {
    delete lCrypto;
    lCrypto = NULL;
  }
  if (NULL != lEncData) {
    delete lEncData;
    lEncData = NULL;
  }

  return lRet;
}


/*==============================================================================
* Function : releaseCodecLibs
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: release the instances of the encoder libs
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::releaseCodecLibs()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  QIDBG_HIGH("%s:%d] E", __func__, __LINE__);

  if (m_mainEncoder) {
    delete(m_mainEncoder);
    m_mainEncoder = NULL;
  }
  if (m_thumbEncoder) {
    delete(m_thumbEncoder);
    m_thumbEncoder = NULL;
  }

  QIDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return lret;
}

/*==============================================================================
* Function : releaseCurrentSession
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Release all buffers associated with the current snapshot.
* Encoding shold be stopped before calling this function. If not it will
* lead to a crash.
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoder::releaseCurrentSession()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] E", __func__, __LINE__);

  QI_LOCK(&m_abortlock);
  m_releaseFlag = OMX_TRUE;
  m_thumbEncodingComplete = OMX_FALSE;
  m_mainEncodingComplete = OMX_FALSE;
  QI_UNLOCK(&m_abortlock);

  if (m_mainEncoder) {
    m_mainEncoder->ReleaseSession();
  }

  if (m_thumbEncoder) {
    delete(m_thumbEncoder);
    m_thumbEncoder = NULL;
  }

  if (m_inThumbImage) {
    delete(m_inThumbImage);
    m_inThumbImage = NULL;
  }
  if (m_outThumbImage) {
    delete(m_outThumbImage);
    m_outThumbImage = NULL;
  }
  if (m_inputQIBuffer) {
    delete(m_inputQIBuffer);
    m_inputQIBuffer = NULL;
  }
  if (m_inputQTmbBuffer) {
    delete(m_inputQTmbBuffer);
    m_inputQTmbBuffer = NULL;
  }

  if (m_inputMainImage) {
    delete(m_inputMainImage);
    m_inputMainImage = NULL;
  }
  if (m_outputQIBuffer) {
    delete(m_outputQIBuffer);
    m_outputQIBuffer = NULL;
  }
  if (m_outputMainImage) {
    delete(m_outputMainImage);
    m_outputMainImage = NULL;
  }
 /* if (m_exifInfo.exif_data) {
    delete [] m_exifInfo.exif_data;
    m_exifInfo.exif_data = NULL;
    m_exifInfo.numOfEntries = 0;
  }*/
  if (mThumbBuffer) {
    delete mThumbBuffer;
    mThumbBuffer = NULL;
  }
  if (OMX_TRUE == mExifObjInitialized) {
    exif_destroy(&m_exifInfoObj);
    mExifObjInitialized = OMX_FALSE;
  }
  if (m_composer) {
    delete m_composer;
    m_composer = NULL;
  }
  if (m_mobicatComposer) {
    delete m_mobicatComposer;
    m_mobicatComposer = NULL;
  }
  QIDBG_MED("%s:%d] X", __func__, __LINE__);
  return lret;
}
