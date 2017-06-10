/*******************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#include "OMXImageDecoder.h"
#include "QImageDecoderObserver.h"
#include "QExifParser.h"


/*===========================================================================
 * Class: OMXImageDecoder
 *
 * Description: This class represents the core OMX Jpeg encoder component
 *
 * Notes: none
 *==========================================================================*/
class OMXJpegDecoder: public OMXImageDecoder, public QImageDecoderObserver,
  public QImageReaderObserver
{
  protected:

  /** decodeImage
   *  @a_inBuffer: input buffer header
   *  @a_outBuffer: output buffer header
   *
   *  Start Image Encoding
   **/
  virtual OMX_ERRORTYPE decodeImage(OMX_BUFFERHEADERTYPE *a_inBuffer,
    OMX_BUFFERHEADERTYPE *a_outBuffer);

  /** configureBuffers
   *  @a_inBuffer: input buffer header
   *  @a_outBuffer: output buffer header
   *
   *  Configure the input and output buffers
   **/
  virtual OMX_ERRORTYPE configureBuffers(OMX_BUFFERHEADERTYPE *a_inBuffer,
    OMX_BUFFERHEADERTYPE *a_outBuffer);

  /** configureInBuffer
   *  @a_inBuffer: input buffer header
   *
   *  Configure the input buffer
   **/
  virtual OMX_ERRORTYPE configureInBuffer(OMX_BUFFERHEADERTYPE *a_inBuffer);


  /** configureOutBuffer
   *  @a_outBuffer: output buffer header
   *
   *  Configure the output buffer
   **/
  virtual OMX_ERRORTYPE configureOutBuffer(OMX_BUFFERHEADERTYPE *a_outBuffer);

  /** releaseCurrentSession
   *
   *  Release all buffers associated with the current decode process.
   *  Decoding shold be stopped before calling this function. If
   *  not it will lead to a crash.
   **/
  OMX_ERRORTYPE releaseCurrentSession();

  /** decodeImageHeader
   *  @a_inBuffer: input buffer header
   *
   *  decode the image header
   **/
  virtual OMX_ERRORTYPE decodeImageHeader(OMX_BUFFERHEADERTYPE *a_inBuffer);

  /** DecodeComplete
   *
   *  This function is called from the JPEG component framework
   *  when decoding is complete
   **/
  int DecodeComplete(QImage *aOutputImage);


  void ReadError(ErrorType aErrorType);

  /** ReadComplete
   *  @aBuffer: output buffer
   *
   *  Callback function from exif parser to indicate
   *  successful header read.
   **/
  void ReadComplete(QIBuffer &aBuffer);

  /** WriteFragmentDone
   *  @aBuffer: output buffer
   *
   *  Callback function from exif parser for piecewise parsing
   **/
  void ReadFragmentDone(QIBuffer &aBuffer);

  /** DecodeError
   *  aErrorType: error type
   *
   *  Callback function from decoder framework to indicate error
   **/
  int DecodeError(DecodeErrorType aErrorType);

  /** startDecode
   *
   *  Get the decoder from the factory and start decoding
   **/
  OMX_ERRORTYPE startDecode();

  /** configureDecodeData
   *
   *  Configure the decode parameters
   **/
  OMX_ERRORTYPE configureDecodeData();

  private:

  /** SetSourceBuffer
   *
   *  Allocate input buffer and set the configure the decoder
   *  with the input.
   *  @a_inBuffer: Input buffer passed during EmptyThisBuffer
   **/
  int SetSourceBuffer(OMX_BUFFERHEADERTYPE *a_inBuffer);


  /** jpeg2QISubsampling
   *
   * Convert from jpeg_subsampling_t to QISubsampling.
   * @sf: image size factor
   * @jss: subsampling to convert
   */
  int jpeg2QISubsampling(float *sf, jpeg_subsampling_t jss);

  protected:

  /** m_mainFrameInfo
   *
   * Encoded frame information, extracted from header.
   */
  jpeg_frame_info_t *m_mainFrameInfo;
};
