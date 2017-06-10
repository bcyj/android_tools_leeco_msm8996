/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QEXIF_PARSER_H__
#define __QEXIF_PARSER_H__

#include "QImageReaderObserver.h"
#include "QImage.h"
#include "QIHeapBuffer.h"

extern "C" {
#include "exif.h"
#include "jpeg_header.h"
#include "exif_private.h"
#include <stdlib.h>
}

/*===========================================================================
 * Class: QExifParser
 *
 * Description: This class represents the exif parser utility
 *
 * Notes: The header will be destroyed if the parser object is destroyed.
 *        if the header needs to be used, make sure that the parser is
 *        retained. Otherwise the client should create a copy of the header
 *        and maintain it.
 *==========================================================================*/
class QExifParser {

public:

  /** New
   *  @aObserver: observer for the parser
   *
   *  2 phase constructor
   **/
  static QExifParser* New(QImageReaderObserver &aObserver);

  /** ~QExifParser
   *
   *  destructor
   **/
  ~QExifParser();

  /** addBuffer
   *  @aBuffer: buffer object
   *
   *  adds the buffer to the parser
   **/
  int addBuffer(QIBuffer *aBuffer);

  /** Start
   *  @aSync: flag to indicate whether to run the parser in
   *        synchronous or asynchronous mode
   *
   *  starts the exif parser
   **/
  int Start(int aSync = true);

  /** GetHeader
   *
   *  gets the jpeg header
   **/
  inline jpeg_header_t *GetHeader()
  {
    return &header;
  }

private:

  /** QExifParser
   *  @aObserver: observer for the parser
   *
   *  constructor
   **/
  QExifParser(QImageReaderObserver &aObserver);

  /** Init
   *
   *  initialize the parser
   **/
  int Init();

  /** ProcessSOF
   *  @p_frame_info: pointer to frame info header
   *  @marker: SOF marker code
   *
   *  process the SOF marker
   **/
  int ProcessSOF(jpeg_frame_info_t *p_frame_info,
    jpeg_marker_t marker);

  /** ProcessSOS
   *  @p_frame_info: pointer to frame info header
   *
   *  process the SOS marker
   **/
  int ProcessSOS(jpeg_frame_info_t *p_frame_info);

  /** FindSOI
   *
   *  finds the SOI marker
   **/
  int FindSOI();

  /** ProcessDHT
   *  @p_frame_info: pointer to frame info header
   *
   *  process the DHT marker
   **/
  int ProcessDHT(jpeg_frame_info_t *p_frame_info);

  /** ProcessDQT
   *  @p_frame_info: pointer to frame info header
   *
   *  process the DQT marker
   **/
  int ProcessDQT(jpeg_frame_info_t *p_frame_info);

  /** ProcessDRI
   *  @p_frame_info: pointer to frame info header
   *
   *  process the DRI marker
   **/
  int ProcessDRI(jpeg_frame_info_t *p_frame_info);

  /** ProcessApp0
   *  @p_frame_info: pointer to frame info header
   *
   *  process the App0 marker
   **/
  int ProcessApp0();

  /** ProcessApp1
   *  @p_frame_info: pointer to frame info header
   *
   *  process the App1 marker
   **/
  int ProcessApp1();

  /** ProcessApp3
   *  @p_frame_info: pointer to frame info header
   *
   *  process the App3 marker
   **/
  int ProcessApp3();

  /** ParseSOF
   *  @p_frame_info: pointer to frame info header
   *
   *  process the SOF marker
   **/
  int ParseSOF(jpeg_frame_info_t *p_frame_info);

  /** FindNextHeader
   *
   *  function to find the next header
   **/
  jpeg_marker_t FindNextHeader();

  /** ProcessZeroIfd
   *
   *  process the 0th Ifd marker
   **/
  int ProcessZeroIfd();

  /** ProcessExifIfd
   *
   *  process the Exif Ifd marker
   **/
  int ProcessExifIfd();

  /** ProcessGpsIfd
   *
   *  process the Gps Ifd marker
   **/
  int ProcessGpsIfd();

  /** ProcessFirstIfd
   *
   *  process the First Ifd marker
   **/
  int ProcessFirstIfd();

  /** FetchTag
   *  @pp_tag_entry: list of exif tag entries
   *  @expected_type: expected type of the tag
   *  @tag_id: ID of the tag
   *
   *  fetch the exif tag from the list
   **/
  int FetchTag(exif_tag_entry_ex_t **pp_tag_entry,
    uint16_t expected_type,
    exif_tag_id_t tag_id);

  /** FetchBytes
   *
   *  fetch one byte from the buffer
   **/
  uint8_t FetchBytes();

  /** Fetch2Bytes
   *
   *  fetch 2 bytes from the buffer
   **/
  uint16_t Fetch2Bytes();

  /** Fetch4Bytes
   *
   *  fetch 4 bytes from the buffer
   **/
  uint32_t Fetch4Bytes();

  /** FetchNBytes
   *  @p_dest: destination buffer
   *  @bytes_to_fetch: number of bytes to fetch
   *  @p_bytes_fetched: number of bytes fetched
   *
   *  fetch N bytes from the buffer
   **/
  void FetchNBytes(uint8_t *p_dest, uint32_t bytes_to_fetch,
    uint32_t *p_bytes_fetched);

  /** ReadHeader
   *
   *  Main function to read the header
   **/
  int ReadHeader();

private:

  /** mBuffer
   *
   *  Exif Buffer to be parsed
   **/
  QIBuffer *mBuffer;

  /** mObserver
   *
   *  Image reader observer reference
   **/
  QImageReaderObserver &mObserver;

  /** next_byte_offset
   *
   *  The overal offset in the input stream of the byte to be
   *  fetched next
   **/
  uint32_t next_byte_offset;

  /** input_start_offset
   *
   *  The start offset in the overall input bitstream of the first
   *  byte of data in the input buffer
   **/
  uint32_t input_start_offset;

  /** endianness
   *
   *  The endianess (little/big)
   **/
  exif_endianness_t endianness;

  /** error_flag
   *
   *  The error value during parsing
   **/
  uint8_t error_flag;

  /** header
   *
   *  Jpeg header structure
   **/
  jpeg_header_t header;

  /** tiff_hdr_offset
   *
   *  TIFF header offset
   **/
  uint32_t tiff_hdr_offset;

  /** exif_ifd_offset
   *
   *  EXIF Ifd offset
   **/
  uint32_t exif_ifd_offset;

  /** gps_ifd_offset
   *
   *  Gps Ifd offset
   **/
  uint32_t gps_ifd_offset;

  /** first_ifd_offset
   *
   *  First Ifd offset
   **/
  uint32_t first_ifd_offset;

  /** interop_ifd_offset
   *
   *  Interop Ifd offset
   **/
  uint32_t interop_ifd_offset;
};

#endif //__QEXIF_PARSER_H__
