/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QEXIF_COMPOSER_H__
#define __QEXIF_COMPOSER_H__

#include "QImageWriterObserver.h"
#include "QImage.h"
#include "QIHeapBuffer.h"
#include "QExifComposerParams.h"

extern "C" {
#include "exif.h"
#include "exif_private.h"
#include <stdlib.h>
}

/*===========================================================================
 * Class: QExifComposer
 *
 * Description: This class represents the exif composer utility
 *
 * Notes: none
 *==========================================================================*/
class QExifComposer {

public:

  /** JpegHeaderType
   *  EXIF: EXIF header
   *  JFIF: JFIF header
   *
   *  Jpeg file header type
   **/
  typedef enum {
    EXIF,
    JFIF
  } JpegHeaderType;

  /** New
   *  @aObserver: observer object reference
   *
   *  2 phase constructor
   **/
  static QExifComposer* New(QImageWriterObserver &aObserver);

  /** ~QExifComposer
   *
   *  virtual destructor
   **/
  ~QExifComposer();

  /** addBuffer
   *  @aBuffer: buffer object
   *
   *  add buffer to the composer
   **/
  int addBuffer(QIBuffer *aBuffer);

  /** Start
   *  @aThumbnail: thumbnail image object, if the thumbnail is not
   *             encoded, pass NULL
   *  @aType: jpeg header type
   *  @aSync: composer mode (synchronous or asynchronous)
   *
   *  start the composer
   *
   *  Currently only synchronous mode is supported
   **/
  int Start(QImage *aThumbnail = NULL, JpegHeaderType aType = EXIF,
    int aSync = true);

  /** SetParams
   *  @aParams: exif composer parameters
   *
   *  set composer parameters
   **/
  int SetParams(QExifComposerParams &aParams);

private:

  /** QExifComposer
   *  @aObserver: exif composer observer
   *
   *  constructor
   **/
  QExifComposer(QImageWriterObserver &aObserver);

  /** Init
   *
   *  initialize the composer
   **/
  int Init();

  /** FlushScratchBuffer
   *
   *  flush the scratch buffer
   **/
  int FlushScratchBuffer();

  /** EmitScanHeader
   *  @aParams: encoder parameters
   *
   *  emit scan header into the buffer
   **/
  void EmitScanHeader(QIEncodeParams &aParams);

  /** EmitFrameHeader
   *  @aParams: encoder parameters
   *  @aSS: image subsampling
   *
   *  emit frame header into the buffer
   **/
  void EmitFrameHeader(QIEncodeParams &aParams, QISubsampling aSS);

  /** EmitJFIF
   *
   *  emit JFIF header into the buffer
   **/
  int EmitJFIF();

  /** EmitEXIF
   *
   *  emit EXIF header into the buffer
   **/
  int EmitEXIF();

  /** EmitApp1
   *
   *  emit App1 marker into the buffer
   **/
  void EmitApp1();

  /** EmitThumbnailIfd
   *
   *  emit thubnail Ifd marker into the buffer
   **/
  void EmitThumbnailIfd();

  /** EmitGpsIfd
   *
   *  emit Gps Ifd marker into the buffer
   **/
  void EmitGpsIfd();

  /** EmitInteropIfd
   *
   *  emit Interop Ifd marker into the buffer
   **/
  void EmitInteropIfd();

  /** EmitExifIfd
   *  @nInteropIfdPointerOffset: interop pointer offset
   *
   *  emit Exif Ifd marker into the buffer
   **/
  void EmitExifIfd(uint32_t *nInteropIfdPointerOffset);

  /** Emit0thIfd
   *  @nExifIfdPointerOffset: exif pointer offset
   *  @nGpsIfdPointerOffset: gps pointer offset
   *
   *  emit 0th Ifd marker into the buffer
   **/
  void Emit0thIfd(uint32_t *nExifIfdPointerOffset,
    uint32_t *nGpsIfdPointerOffset);

  /** EmitExif
   *  @p_entry: exif tag entry
   *
   *  emit exif tag into the buffer
   **/
  void EmitExif(exif_tag_entry_ex_t *p_entry);

  /** EmitApp4
   *
   *  emit APP4 marker into the buffer
   **/
  void EmitApp4();

  /** EmitApp5
   *
   *  emit APP5 marker into the buffer
   **/
  void EmitApp5(uint32_t payload_length, uint32_t payload_written);

  /** EmitApp6
   *
   *  emit APP6 marker into the buffer
   **/
  void EmitApp6(uint32_t payload_length, uint32_t payload_written);

  /** EmitApp7
   *
   *  emit APP7 marker into the buffer
   **/
  void EmitApp7();

  /** EmitApp0
   *
   *  emit App0 marker into the buffer
   **/
  void EmitApp0();

  /** EmitSOS
   *  @aParams: Encode parms
   *
   *  emit SOS marker into the buffer
   **/
  void EmitSOS(QIEncodeParams &aParams);

  /** EmitSOF
   *  @code: marker code
   *  @aParams: encode parameter
   *  @aSubSampling: image subsampling
   *
   *  emit SOF marker into the buffer
   **/
  void EmitSOF(jpeg_marker_t code, QIEncodeParams &aParams,
    QISubsampling aSubSampling);

  /** EmitDRI
   *  @nRestartInterval: restart interval
   *
   *  emit DRI marker into the buffer
   **/
  void EmitDRI(uint16_t nRestartInterval);

  /** EmitDHT
   *  @htbl: huffman table
   *  @index: huffman index
   *
   *  emit DHT marker into the buffer
   **/
  void EmitDHT(const QIHuffTable::HuffTable *htbl, int index);

  /** EmitDQT
   *  @qtbl: quant table
   *
   *  emit DQT marker into the buffer
   **/
  void EmitDQT(uint16_t *qtbl);

  /** FinishIfd
   *
   *  finish writing Ifd into the buffer
   **/
  void FinishIfd();

  /** StartIfd
   *
   *  start writing Ifd into the buffer
   **/
  void StartIfd();

  /** FlushFileHeader
   *
   *  flush file header into the buffer
   **/
  int FlushFileHeader();

  /** FlushThumbnail
   *
   *  flush thumbnail into the buffer
   **/
  int FlushThumbnail();

private:

  /** mWriterParams
   *
   *  exif composer parameter
   **/
  QExifComposerParams *mWriterParams;

  /** mBuffer
   *
   *  buffer passed by the user
   **/
  QIBuffer *mBuffer;

  /** mCurrentOffset
   *
   *  current offset filled within the buffer
   **/
  uint32_t mCurrentOffset;

  /** mAheadBufOffset
   *
   *  current offset filled within the ahead buffer
   **/
  uint32_t mAheadBufOffset;

  /** mAheadBuffer
   *
   *  ahead buffer allocated in heap
   **/
  QIHeapBuffer *mAheadBuffer;

  /** mThumbnail
   *
   *  thumbnail image passed by the user
   **/
  QImage *mThumbnail;

  /** mObserver
   *
   *  exif composer observer
   **/
  QImageWriterObserver &mObserver;

  /** nTiffHeaderOffset
   *
   *  Save the Tiff header location
   **/
  uint32_t nTiffHeaderOffset;

  /** nApp1LengthOffset
   *
   *  Save the App1 length location
   **/
  uint32_t nApp1LengthOffset;

  /** nThumbnailOffset
   *
   *  Save the thumbnail starting location
   **/
  uint32_t nThumbnailOffset;

  /** nThumbnailStreamOffset
   *
   *  Save the thumbnail stream starting location
   **/
  uint32_t nThumbnailStreamOffset;

  /** nGpsIfdPointerOffset
   *
   *  Save the Gps Ifd pointer location
   **/
  uint32_t nGpsIfdPointerOffset;

  /** nThumbnailIfdPointerOffset
   *
   *  Save the Thumbnail Ifd pointer location
   **/
  uint32_t nThumbnailIfdPointerOffset;

  /** nFieldCount
   *
   *  Keep count of how many tags are written
   **/
  uint32_t nFieldCount;

  /** nFieldCountOffset
   *
   *  Save the location to write the IFD count
   **/
  uint32_t nFieldCountOffset;

  /** nJpegInterchangeLOffset
   *
   *  Save the offset in ThumbnailIfd specifying the end of
   *  thumbnail offset
   **/
  uint32_t nJpegInterchangeLOffset;

  /** fHeaderWritten
   *
   *  Flag indicating whether header is written
   **/
  uint8_t fHeaderWritten;

  /** fApp1Present
   *
   *  Flag indicating if App1 header is present
   **/
  uint8_t fApp1Present;

  /** app2_present
   *
   *  Flag indicating if App2 header is present
   **/
  uint8_t app2_present;

  /** app2_start_offset
   *
   *  The App2 start offset
   **/
  uint32_t app2_start_offset;

  /** app2_header_length
   *
   *  The App2 data length
   **/
  uint32_t app2_header_length;

  /** p_exif_info
   *
   *  The Exif Info object
   **/
  exif_info_t *p_exif_info;

  /** is_exif_info_owned
   *
   *  Flag indicating whether the exif info object is owned by the
   *  writer
   **/
  uint8_t is_exif_info_owned;

  /** overflow_flag
   *
   *  Flag indicating whether any overflow has occurred
   **/
  uint8_t overflow_flag;
};

#endif //__QEXIF_COMPOSER_H__
