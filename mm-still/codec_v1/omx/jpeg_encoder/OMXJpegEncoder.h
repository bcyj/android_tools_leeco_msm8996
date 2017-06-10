/*******************************************************************************
* Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include "OMXImageEncoder.h"
#include "QExifComposerParams.h"
#include "QExifComposer.h"
#include "QImageEncoderObserver.h"
#include "QIHeapBuffer.h"
#include "QExifCameraTuningParams.h"
#include "QCrypt.h"

#define PARSE_MOBICAT_DATA          1
#define PARSE_3A_DATA               2
#define PARSE_MOBICAT_AND_3A_DATA   3
/*===========================================================================
 * Class: OMXJpegEncoder
 *
 * Description: This class represents the core OMX Jpeg encoder component
 *
 * Notes: none
 *==========================================================================*/
class OMXJpegEncoder: public OMXImageEncoder, QImageEncoderObserver,
  QImageWriterObserver
{

protected:
  /** encodeImage
   *  @a_inBuffer: input buffer header
   *  @a_outBuffer: output buffer header
   *
   *  Start Image Encoding
   **/
  virtual OMX_ERRORTYPE encodeImage(OMX_BUFFERHEADERTYPE *a_inBuffer,
    OMX_BUFFERHEADERTYPE *a_inTmbBuffer,
    OMX_BUFFERHEADERTYPE *a_outBuffer);

  /** configureBuffers
   *  @a_inBuffer: input buffer header
   *  @a_outBuffer: output buffer header
   *
   *  Configure the input and output buffers
   **/
  virtual OMX_ERRORTYPE configureBuffers(OMX_BUFFERHEADERTYPE *a_inBuffer,
    OMX_BUFFERHEADERTYPE *a_outBuffer);

  /** configureEncodedata
   *
   *  Configure the encode parmeters
   **/
  virtual OMX_ERRORTYPE configureEncodedata();

  /** processMetadata
   *
   *  Process makernote data
   **/
  virtual OMX_ERRORTYPE processMetadata();

  /*==============================================================================
   * Function : configureTmbBuffer
   * Parameters: a_inBuffer, a_outBuffer
   * Return Value : OMX_ERRORTYPE
   * Description: Configure input thumbnail buffer
   ==============================================================================*/
  OMX_ERRORTYPE configureTmbBuffer(
      OMX_BUFFERHEADERTYPE *a_inTmbBuffer);

  /** writeExifData
   *  @aThumbnail: The thumbnail buffer after encoding. Null
   *             when thumbnail is not encoded
   *
   *  Add the exif data and encoded thumbnail to the o/p buffer
   **/
  OMX_ERRORTYPE writeExifData(QImage *aThumbnail = NULL, QIBuffer *aOutBuffer = NULL);

  /** startEncode
   *
   *  Get the encoder from the factory and start encoding
   **/
  OMX_ERRORTYPE startEncode();

  /** CompleteMainImage
   *
   * When encoding thumbnail and main image in parallel, call
   * this function to release all buffers associated with the
   * current snapshot.
  **/
  int CompleteMainImage();

  /** configureThumbnailData
   *
   *  Configure the encode parameters for thumbnail
   **/
  OMX_ERRORTYPE configureThumbnailData();

  /** startThumbnailEncode
   *
   *  Start Thumbnail Encode
   **/
  OMX_ERRORTYPE startThumbnailEncode();

  /** WriteFragmentDone
   *  @aBuffer: output buffer
   *
   *  Callback function from ExifComposer for piecewise composing
   **/
  void WriteFragmentDone(QIBuffer &aBuffer);

  /** WriteFragmentDone
   *  @aBuffer: output buffer
   *
   *  Callback function from ExifComposer to indicate the
   *  completion of exif composition
   **/
  void WriteComplete(QIBuffer &aBuffer);

  /** WriteError
   *  aErrorType: error type
   *
   *  Callback function from ExifComposer to indicate error
   **/
  void WriteError(ErrorType aErrorType);

  /** OutputFragment
   *  @aBuffer: output buffer
   *
   *  Callback function from encoder tramework for piecewise
   *  encoding
   **/
  int OutputFragment(QIBuffer &aBuffer);

  /** EncodeError
   *  aErrorType: error type
   *
   *  Callback function from encoder tramework to indicate error
   **/
  int EncodeError(EncodeErrorType aErrorType);

  /** EncodeComplete
   *
   *  This function is called from the JPEG component framework
   *  when encoding is complete
   **/
  int EncodeComplete(QImage *aOutputImage);

  /** releaseCurrentSession
   *
   *  Release all buffers associated with the current snapshot.
   *  Encoding shold be stopped before calling this function. If
   *  not it will lead to a crash.
   **/
  OMX_ERRORTYPE releaseCurrentSession();

  /** releaseCodecLibs
   *
   *  Release the encoder instances
   **/
  OMX_ERRORTYPE releaseCodecLibs();

  /** preloadCodecLibs
   *
   *  pre-load the encoders
   **/
  OMX_ERRORTYPE preloadCodecLibs();

};
