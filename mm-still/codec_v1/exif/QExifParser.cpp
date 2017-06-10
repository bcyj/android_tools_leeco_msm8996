/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QExifParser.h"

extern "C" {
#include "jps.h"
#include "writer_utility.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "exif_private.h"
#include "jpeg_header.h"
#include <stdlib.h>
}
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define EXIF_BYTE      1
#define EXIF_ASCII     2
#define EXIF_SHORT     3
#define EXIF_LONG      4
#define EXIF_RATIONAL  5
#define EXIF_UNDEFINED 7
#define EXIF_SLONG     9
#define EXIF_SRATIONAL 10

// The number of bytes the parser attempts to fetch each time
#define MAX_BYTES_TO_FETCH   0x2000

// The number of tables
#define JPEGD_MAXHUFFTABLES     8
#define JPEGD_MAXQUANTTABLES    4
#define JPEGD_MAXCOMPONENTS     4
#define JPEGD_MAXCOMPSINSCAN    4

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
/*===========================================================================
 * Function: New
 *
 * Description: 2 phase contructor for QExifParser
 *
 * Input parameters:
 *   aObserver - composer observer
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QExifParser* QExifParser::New(QImageReaderObserver &aObserver)
{
  int lrc = QI_SUCCESS;
  QExifParser* lParser = new QExifParser(aObserver);
  if (NULL == lParser) {
    return NULL;
  }
  lrc = lParser->Init();
  if (QI_SUCCESS != lrc) {
    delete lParser;
    return NULL;
  }
  return lParser;
}

/*===========================================================================
 * Function: QExifParser
 *
 * Description: QExifParser constuctor
 *
 * Input parameters:
 *   aObserver - composer observer
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QExifParser::QExifParser(QImageReaderObserver &aObserver)
  : mObserver(aObserver)
{
  mBuffer = NULL;
  next_byte_offset = 0;
  input_start_offset = 0;
  endianness = EXIF_BIG_ENDIAN;
  error_flag = 0;
  memset(&header, 0x0, sizeof(jpeg_header_t));
  tiff_hdr_offset = 0;
  exif_ifd_offset = 0;
  gps_ifd_offset = 0;
  first_ifd_offset = 0;
  interop_ifd_offset = 0;
}

/*===========================================================================
 * Function: ~QExifParser
 *
 * Description: QExifParser destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QExifParser::~QExifParser()
{
  exif_info_obj_t exif_info_obj = (exif_info_obj_t)header.p_exif_info;

  jpeg_frame_info_destroy(header.p_tn_frame_info);
  jpeg_frame_info_destroy(header.p_main_frame_info);
  exif_destroy(&exif_info_obj);

  memset(&header, 0, sizeof(jpeg_header_t));
}

/*===========================================================================
 * Function: Init
 *
 * Description: initializes the exif parser
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::Init()
{
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: addBuffer
 *
 * Description: This function is used to enqueue the input buffer
 *
 * Input parameters:
 *   aBuffer - pointer to the buffer object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::addBuffer(QIBuffer *aBuffer)
{
  mBuffer = aBuffer;
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Start
 *
 * Description: This function is used to start the exif parser
 *
 * Input parameters:
 *   aSync - start the composer synchronous or asynchronous
 *
 * Return values:
 *   none
 *
 * Notes: Only sync mode is supported as of now
 *==========================================================================*/
int QExifParser::Start(int aSync)
{
  int lrc = QI_SUCCESS;
  if (mBuffer == NULL) {
    QIDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
    return QI_ERR_INVALID_OPERATION;
  }

  lrc = ReadHeader();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] invalid input", __func__, __LINE__);
    return QI_ERR_INVALID_INPUT;
  }
  mObserver.ReadComplete(*mBuffer);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: FetchBytes
 *
 * Description: Fetch byte from buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
uint8_t QExifParser::FetchBytes()
{
  uint8_t byte_fetched;

  // Request buffer if necessary
  if (mBuffer->Length() <
    (next_byte_offset - input_start_offset)) {
    error_flag = true;
    QIDBG_ERROR("%s:%d] Cannot fetch byte overflow len %d next_len %d"
      " start %d", __func__, __LINE__, mBuffer->Length(),
      next_byte_offset, input_start_offset);
    return 0;
  }

  byte_fetched = *(mBuffer->Addr() + (next_byte_offset - input_start_offset));
  next_byte_offset++;
  return byte_fetched;
}

/*===========================================================================
 * Function: Fetch2Bytes
 *
 * Description: Fetch 2 bytes from buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
uint16_t QExifParser::Fetch2Bytes()
{
  uint8_t byte1 = FetchBytes();
  uint8_t byte2 = FetchBytes();

  return(endianness == EXIF_BIG_ENDIAN) ?
  ((byte1 << 8) + byte2) :
  ((byte2 << 8) + byte1);
}

/*===========================================================================
 * Function: Fetch4Bytes
 *
 * Description: Fetch 4 bytes from buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
uint32_t QExifParser::Fetch4Bytes()
{
  uint8_t byte1 = FetchBytes();
  uint8_t byte2 = FetchBytes();
  uint8_t byte3 = FetchBytes();
  uint8_t byte4 = FetchBytes();

  return (endianness == EXIF_BIG_ENDIAN) ?
    ((byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4) :
    ((byte4 << 24) + (byte3 << 16) + (byte2 << 8) + byte1);
}

/*===========================================================================
 * Function: Fetch4Bytes
 *
 * Description: Fetch N bytes from buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
void QExifParser::FetchNBytes(uint8_t *p_dest, uint32_t bytes_to_fetch,
  uint32_t *p_bytes_fetched)
{
  uint32_t bytes_fetched = 0;

  while (bytes_fetched < bytes_to_fetch) {
    uint32_t bytes_to_copy;
    // Request buffer if necessary
    if (mBuffer->Length() <
      (next_byte_offset - input_start_offset)) {
      QIDBG_ERROR("%s:%d] Cannot fetch byte overflow", __func__, __LINE__);
      return;
    }

    // Compute how many bytes should be copied
    bytes_to_copy = STD_MIN((bytes_to_fetch - bytes_fetched),
      ((mBuffer->FilledLen() + input_start_offset) - next_byte_offset));

    // Copy
    memcpy(p_dest, mBuffer->Addr() +
      (next_byte_offset - input_start_offset),
      bytes_to_copy);
    p_dest += bytes_to_copy;
    next_byte_offset += bytes_to_copy;
    bytes_fetched += bytes_to_copy;
  }
  if (p_bytes_fetched)
    *p_bytes_fetched = bytes_fetched;
}

/*===========================================================================
 * Function: FindNextHeader
 *
 * Description: Find next header from the buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
jpeg_marker_t QExifParser::FindNextHeader()
{
  uint32_t byte1;

  do {

    do {
      byte1 = FetchBytes();
      if (error_flag) {
        return M_EOI;
      }
    } while (byte1 != 0xFF);

    do {
      byte1 = FetchBytes();
      if (error_flag) {
        return M_EOI;
      }

    } while (byte1 == 0xFF);

  } while (byte1 == 0);

  // save to cast to uint8_t
  return (jpeg_marker_t)byte1;
}

/*===========================================================================
 * Function: ReadHeader
 *
 * Description: Read Exif header
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ReadHeader()
{
  int rc;

  // Find SOI marker
  rc = FindSOI();

  if (QI_SUCCEEDED(rc)) {
    header.p_main_frame_info = (jpeg_frame_info_t *)JPEG_MALLOC(
      sizeof(jpeg_frame_info_t));
    if (!header.p_main_frame_info) {
      rc = QI_ERR_NO_MEMORY;
    } else {
      memset(header.p_main_frame_info, 0, sizeof(jpeg_frame_info_t));
    }
  }
  if (QI_SUCCEEDED(rc)) {
    rc = ParseSOF(header.p_main_frame_info);
  }
  if (QI_ERROR(rc)) {
    return QI_ERR_GENERAL;
  }
  jpeg_dump_header(&header);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ParseSOF
 *
 * Description: Parse SOF marker
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ParseSOF(jpeg_frame_info_t *p_frame_info)
{
  int rc;
  for (;;) {
    jpeg_marker_t marker = FindNextHeader();
    QIDBG_LOW("%s:%d] Marker %x", __func__, __LINE__,
      0xff00 | (uint16_t)marker);
    switch (marker) {
    case M_SOF0:
    case M_SOF1:
    case M_SOF2:
    case M_SOF3:
    case M_SOF5:
    case M_SOF6:
    case M_SOF7:
    case M_SOF9:
    case M_SOF10:
    case M_SOF11:
    case M_SOF13:
    case M_SOF14:
    case M_SOF15:
      rc = ProcessSOF(p_frame_info, marker);
      if (QI_ERROR (rc)) return rc;
      break;
    case M_SOI:
      // not expected to see SOI again, return failure
      return QI_ERR_GENERAL;
    case M_EOI:
      // not expected to see EOI, return failure
      return QI_ERR_GENERAL;
    case M_SOS:
      rc = ProcessSOS(p_frame_info);
      if (QI_ERROR (rc)) return rc;
      return QI_SUCCESS;
    case M_DHT:
      rc = ProcessDHT(p_frame_info);
      if (QI_ERROR (rc)) return rc;
      break;
    case M_DQT:
      rc = ProcessDQT(p_frame_info);
      if (QI_ERROR (rc)) return rc;
      break;
    case M_DRI:
      rc = ProcessDRI(p_frame_info);
      if (QI_ERROR (rc)) return rc;
      break;
    case M_APP0:
      // Even failed to process APP0, still continue since
      // it is not mandatory
      (void)ProcessApp0();
      break;
    case M_APP1:
      // Even failed to process APP1, still continue since
      // it is not mandatory
      (void)ProcessApp1();
      endianness = EXIF_BIG_ENDIAN;
      break;
    case M_APP3:
      // Even failed to process APP3, still continue since
      // it is not mandatory
      (void)ProcessApp3();
      break;

    default: {
      // Skip unsupported marker
      uint16_t bytesToSkip = Fetch2Bytes();

      if (bytesToSkip >= 2)
        bytesToSkip -= 2;

      QIDBG_LOW("Skipping unsupported marker (FF%X)...", marker);
      next_byte_offset += bytesToSkip;
      break;
    }
    }
  }
}

/*===========================================================================
 * Function: FindSOI
 *
 * Description: Find SOI marker
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::FindSOI()
{
  jpeg_marker_t m1, m2;
  uint32_t cnt = 256;

  m1 = (jpeg_marker_t)FetchBytes();
  m2 = (jpeg_marker_t)FetchBytes();

  if ((m1 == (jpeg_marker_t) 0xFF) && (m2 == M_SOI)) {
    return QI_SUCCESS;
  }
  // not found immediately, continuing searching for another 256 bytes
  while (cnt) {
    m1 = m2;
    m2 = (jpeg_marker_t)FetchBytes();
    if ( (m1 == (jpeg_marker_t) 0xFF) && (m2 == M_SOI)) {
        return QI_SUCCESS;
    }
    cnt--;
  }
  return QI_ERR_GENERAL;
}

/*===========================================================================
 * Function: ProcessSOF
 *
 * Description: process SOF marker
 *
 * Input parameters:
 *   p_frame_info - frame information
 *   marker - jpeg marker
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessSOF(jpeg_frame_info_t *p_frame_info,
  jpeg_marker_t marker)
{
  uint32_t i, len, comp_infos_len;

  switch (marker) {
  case M_SOF0:
    p_frame_info->process = JPEG_PROCESS_BASELINE;
    break;
  case M_SOF1:
    p_frame_info->process = JPEG_PROCESS_EXTENDED_HUFFMAN;
    break;
  case M_SOF2:
    p_frame_info->process = JPEG_PROCESS_PROGRESSIVE_HUFFMAN;
    break;
  case M_SOF3:
    p_frame_info->process = JPEG_PROCESS_LOSSLESS_HUFFMAN;
    break;
  case M_SOF5:
    p_frame_info->process = JPEG_PROCESS_DIFF_SEQ_HUFFMAN;
    break;
  case M_SOF6:
    p_frame_info->process = JPEG_PROCESS_DIFF_PROG_HUFFMAN;
    break;
  case M_SOF7:
    p_frame_info->process = JPEG_PROCESS_DIFF_LOSSLESS_HUFFMAN;
    break;
  case M_SOF9:
    p_frame_info->process = JPEG_PROCESS_SEQ_ARITHMETIC;
    break;
  case M_SOF10:
    p_frame_info->process = JPEG_PROCESS_PROGRESSIVE_ARITHMETIC;
    break;
  case M_SOF11:
    p_frame_info->process = JPEG_PROCESS_LOSSLESS_ARITHMETIC;
    break;
  case M_SOF13:
    p_frame_info->process = JPEG_PROCESS_DIFF_SEQ_ARITHMETIC;
    break;
  case M_SOF14:
    p_frame_info->process = JPEG_PROCESS_DIFF_PROG_ARITHMETIC;
    break;
  case M_SOF15:
    p_frame_info->process = JPEG_PROCESS_DIFF_LOSSLESS_ARITHMETIC;
    break;

  default:
    return JPEGERR_EUNSUPPORTED;
  }

  // Get length of the compoent infos
  len = Fetch2Bytes();

  // Get precision
  p_frame_info->precision = FetchBytes();

  // Get height
  p_frame_info->height = Fetch2Bytes();

  // Get width
  p_frame_info->width = Fetch2Bytes();

  // Get number of components in frame
  comp_infos_len = FetchBytes();

  // Validation on number of components
  if (comp_infos_len == 0 || comp_infos_len > 4 ||
    len != (8 + comp_infos_len * 3))
    return QI_ERR_GENERAL;

  // Get component infos
  p_frame_info->p_comp_infos = (jpeg_comp_info_t *)
    JPEG_MALLOC(comp_infos_len * sizeof(jpeg_comp_info_t));
  if (!p_frame_info->p_comp_infos) {
    return QI_ERR_NO_MEMORY;
  }
  memset(p_frame_info->p_comp_infos, 0, comp_infos_len *
    sizeof(jpeg_comp_info_t));

  for (i = 0; i < comp_infos_len; i++) {
    uint8_t temp_byte;
    jpeg_comp_info_t *p_info = p_frame_info->p_comp_infos + i;

    p_info->comp_id = FetchBytes();
    temp_byte = FetchBytes();
    p_info->sampling_h = temp_byte >> 4;
    p_info->sampling_v = temp_byte & 0x0F;
    p_info->qtable_sel = FetchBytes();

    if (p_info->qtable_sel > 3)
      return QI_ERR_GENERAL;
  }
  p_frame_info->num_comps = comp_infos_len;

  // Derive subsampling information
  if (comp_infos_len == 1) {
      p_frame_info->subsampling = JPEG_GRAYSCALE;
  } else if (comp_infos_len == 3) {
    uint8_t subsampling = 3; // JPEG_H1V1
    if (p_frame_info->p_comp_infos[0].sampling_h == 2) {
      subsampling -= 2;
    }
    if (p_frame_info->p_comp_infos[0].sampling_v == 2) {
      subsampling -= 1;
    }
    p_frame_info->subsampling = (jpeg_subsampling_t)subsampling;
  } else {
    // Unsupprted number of components
    QIDBG_LOW("ProcessSOF: unexpected number of components\n");
    return JPEGERR_EUNSUPPORTED;
  }

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ProcessSOS
 *
 * Description: process SOS marker
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessSOS(jpeg_frame_info_t *p_frame_info)
{
  int rc = QI_SUCCESS;
  uint8_t temp_byte, j;
  uint32_t len, comp_sels_len, i;
  jpeg_scan_info_t *p_scan_info = jpeg_add_scan_info(p_frame_info);

  if (!p_scan_info)
    return QI_ERR_NO_MEMORY;

  len = Fetch2Bytes();

  p_scan_info->offset = next_byte_offset + (len - 2);

  comp_sels_len = FetchBytes();

  if (comp_sels_len != 1 && comp_sels_len != 3) {
    QIDBG_LOW("ProcessSOS: unexpected number of component selectors: %d\n",
      comp_sels_len);
    return QI_ERR_GENERAL;
  }

  if (len != 6 + comp_sels_len * 2) {
    QIDBG_LOW("%s:%d] length not match with number of component selectors",
      __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  p_scan_info->p_selectors = (jpeg_comp_entropy_sel_t *)
    JPEG_MALLOC(comp_sels_len * sizeof(jpeg_comp_entropy_sel_t));
  if (!p_scan_info->p_selectors) {
    return QI_ERR_NO_MEMORY;
  }

  for (i = 0; i < comp_sels_len; i++) {
    p_scan_info->p_selectors[i].comp_id = FetchBytes();
    temp_byte = FetchBytes();
    p_scan_info->p_selectors[i].dc_selector = temp_byte >> 4;
    p_scan_info->p_selectors[i].ac_selector = temp_byte & 0x0F;

    // Map the component Id to the index to the component Info
    for (j = 0; j < p_frame_info->num_comps; j++) {
      if (p_scan_info->p_selectors[i].comp_id ==
        p_frame_info->p_comp_infos[j].comp_id) {
        p_scan_info->p_selectors[i].comp_id = j;
        break;
      }
    }

    // No corresponding Id found
    if (j == p_frame_info->num_comps ||
        p_scan_info->p_selectors[i].dc_selector >= 4 ||
        p_scan_info->p_selectors[i].ac_selector >= 4) {
        rc = QI_ERR_GENERAL;
    }

    // Make sure the tables are present
    if (!(p_frame_info->htable_present_flag &
      (1 << p_scan_info->p_selectors[i].dc_selector)) ||
      !(p_frame_info->htable_present_flag &
      (1 << p_scan_info->p_selectors[i].ac_selector))) {
      QIDBG_MED("ProcessSOS: invalid entropy table selector\n");
      return QI_ERR_GENERAL;
    }
  }

  if (QI_SUCCEEDED(rc)) {

    p_scan_info->spec_start = FetchBytes();
    p_scan_info->spec_end   = FetchBytes();
    temp_byte = FetchBytes();
    p_scan_info->succ_approx_h = temp_byte >> 4;
    p_scan_info->succ_approx_l = temp_byte & 0x0F;
    p_scan_info->num_selectors = comp_sels_len;

    // Validation
    if (!p_frame_info->qtable_present_flag ||
      !p_frame_info->htable_present_flag ||
      p_scan_info->spec_start >= 64 || p_scan_info->spec_end >= 64 ||
      p_scan_info->succ_approx_h >= 14 ||
      p_scan_info->succ_approx_l >= 14) {
      rc = QI_ERR_GENERAL;
    }
  }

  return rc;
}

/*===========================================================================
 * Function: ProcessDQT
 *
 * Description: process quant tables
 *
 * Input parameters:
 *   p_frame_info - pointer to frame info
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessDQT(jpeg_frame_info_t *p_frame_info)
{
  uint32_t i, j, length, dest;

  length = Fetch2Bytes();

  for (i = 0; i < length / 64; i++) {
    uint8_t temp_byte = FetchBytes();
    p_frame_info->quant_precision = temp_byte >> 4;
    dest = temp_byte & 0x0F;

    if(p_frame_info->quant_precision > 1 || dest >= 4)
      return QI_ERR_GENERAL;

    // Set the bit flag to indicate which table are read
    p_frame_info->qtable_present_flag |= (1 << dest);

    // Allocate qtable
    if (!p_frame_info->p_qtables[dest]) {
      p_frame_info->p_qtables[dest] = (jpeg_quant_table_t)
        JPEG_MALLOC(64 * sizeof(uint16_t));
      if (!p_frame_info->p_qtables[dest]) {
          return QI_ERR_NO_MEMORY;
      }
    }
    for (j = 0; j < 64; j++) {
      p_frame_info->p_qtables[dest][j] = (p_frame_info->quant_precision) ?
        Fetch2Bytes() : FetchBytes();
    }

    // validation
    if (2 + 65 + 64 * (uint32_t) p_frame_info->quant_precision > length) {
      return QI_ERR_GENERAL;
    }
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ProcessDHT
 *
 * Description: process DHT marker
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessDHT(jpeg_frame_info_t *p_frame_info)
{
  uint32_t i, sum, index, size;

  // Base on Table B.5 of spec. to read in the Huffman table
  // Fetch the size of this chunk
  size = Fetch2Bytes();
  size -= 2;            /* not include the size */

  while (size > 16) {
    index = FetchBytes();
    if (index & 0xEC) {
      // Bad DHT table class and destination
      return QI_ERR_GENERAL;
    }
    if (index & 0x10)
      index = (index & 0x3) + 4;

    // Set the bit flag to indicate which table are read
    p_frame_info->htable_present_flag |= (1 << index);

    if (index >= JPEGD_MAXHUFFTABLES) {
      // Make sure the buffer index is within range
      return QI_ERR_GENERAL;
    }

    p_frame_info->p_htables[index].bits[0] = 0;
    sum = 0;

    for (i = 1; i < 17; i++) {
      p_frame_info->p_htables[index].bits[i] = FetchBytes();
      sum += p_frame_info->p_htables[index].bits[i];
    }

    if (sum > 256) {
      // Bad DHT cnt
      return QI_ERR_GENERAL;
    }

    for (i = 0; i < sum; i++)
      p_frame_info->p_htables[index].values[i] = FetchBytes();

    /* subtract the number of BITS and the BITS total count */
    size -= (17 + sum);
  }

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ProcessDRI
 *
 * Description: process DRI marker
 *
 * Input parameters:
 *   p_frame_info - pointer to frame info
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessDRI(jpeg_frame_info_t *p_frame_info)
{
  uint32_t len = Fetch2Bytes();

  if (len != 4)
    return QI_ERR_GENERAL;

  p_frame_info->restart_interval = Fetch2Bytes();
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ProcessApp0
 *
 * Description: process App0 marker
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessApp0()
{
  int rc = QI_SUCCESS;
  uint32_t app0_start_offset = next_byte_offset;
  uint32_t len = Fetch2Bytes();

  // Create the JFIF info if not already exists
  if (!header.p_exif_info) {
    header.p_exif_info = (exif_info_t *)JPEG_MALLOC(sizeof(exif_info_t));
    if (!header.p_exif_info)
      rc = QI_ERR_NO_MEMORY;
    else
      memset(header.p_exif_info, 0, sizeof(exif_info_t));
  }

  // Match with "JFIF"
  if (QI_SUCCEEDED(rc))
    rc = (Fetch4Bytes() == 0x4A464946) ? QI_SUCCESS : QI_ERR_GENERAL;

  if (QI_SUCCEEDED(rc)) {
    // Skip the padded bits 0x00
    (void) FetchBytes();
  }

  if (QI_SUCCEEDED (rc)) {
    // Match the JFIF Version
    rc = (Fetch2Bytes() <= 0x0102) ? QI_SUCCESS : QI_ERR_GENERAL;
  }

  if (QI_SUCCEEDED (rc)) {
    // resolution unit
    //rc = FetchTag(, &header.p_exif_info->tiff_ifd.p_resolution_unit,
    // EXIF_SHORT, EXIFTAGID_RESOLUTION_UNIT);
    exif_tag_entry_ex_t *p_new_tag;

    exif_destroy_tag_entry(header.p_exif_info->tiff_ifd.p_resolution_unit);

    p_new_tag = exif_create_tag_entry();

    if (!p_new_tag)
      return QI_ERR_NO_MEMORY;

    p_new_tag->entry.data._short = FetchBytes() + 1;

    if (QI_SUCCEEDED(rc)) {
      p_new_tag->entry.copy = true;
      p_new_tag->entry.count = 1;
      p_new_tag->entry.type = (exif_tag_type_t)EXIF_SHORT;
      p_new_tag->tag_id = EXIFTAGID_RESOLUTION_UNIT;
      header.p_exif_info->tiff_ifd.p_resolution_unit = p_new_tag;
    } else {
      exif_destroy_tag_entry(p_new_tag);
    }

  }

  if (QI_SUCCEEDED(rc)) {

    exif_tag_entry_ex_t *p_new_tag;

    exif_destroy_tag_entry(header.p_exif_info->tiff_ifd.p_x_resolution);

    p_new_tag = exif_create_tag_entry();

    if (!p_new_tag)
      return QI_ERR_NO_MEMORY;

    /*p_new_tag->entry.data._rats = (rat_t *)JPEG_MALLOC(sizeof(rat_t));
    if (!p_new_tag->entry.data._rats)
    {
        rc = QI_ERR_NO_MEMORY;
    }
    else */
    {
      p_new_tag->entry.data._rat.num   = (uint32_t)Fetch2Bytes();
      p_new_tag->entry.data._rat.denom = 1;
    }

    if (QI_SUCCEEDED(rc)) {
      p_new_tag->entry.copy = true;
      p_new_tag->entry.count = 1;
      p_new_tag->entry.type = (exif_tag_type_t)EXIF_RATIONAL;
      p_new_tag->tag_id = EXIFTAGID_X_RESOLUTION;
      header.p_exif_info->tiff_ifd.p_x_resolution = p_new_tag;
    } else {
      exif_destroy_tag_entry(p_new_tag);
    }
  }

  if (QI_SUCCEEDED (rc)) {

    exif_tag_entry_ex_t *p_new_tag;

    exif_destroy_tag_entry(header.p_exif_info->tiff_ifd.p_y_resolution);

    p_new_tag = exif_create_tag_entry();

    if (!p_new_tag)
      return QI_ERR_NO_MEMORY;

    /*p_new_tag->entry.data._rats = (rat_t *)JPEG_MALLOC(sizeof(rat_t));
    if (!p_new_tag->entry.data._rats)
    {
        rc = QI_ERR_NO_MEMORY;
    }
    else */
    {
      p_new_tag->entry.data._rat.num   = (uint32_t)Fetch2Bytes();
      p_new_tag->entry.data._rat.denom = 1;
    }

    if (QI_SUCCEEDED(rc))
    {
        p_new_tag->entry.copy = true;
        p_new_tag->entry.count = 1;
        p_new_tag->entry.type = (exif_tag_type_t)EXIF_RATIONAL;
        p_new_tag->tag_id = EXIFTAGID_Y_RESOLUTION;
        header.p_exif_info->tiff_ifd.p_y_resolution = p_new_tag;
    } else {
      exif_destroy_tag_entry(p_new_tag);
    }
  }

  if (QI_ERROR(rc)) {
    exif_info_obj_t exif_info_obj = (exif_info_obj_t)header.p_exif_info;
    exif_destroy(&exif_info_obj);
    header.p_exif_info = NULL;
  }

  // skip to end of APP0
  next_byte_offset = app0_start_offset + len;
  return rc;

}

/*===========================================================================
 * Function: ProcessApp1
 *
 * Description: process App1 marker
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessApp1()
{
  int rc = QI_SUCCESS;
  uint32_t app1_start_offset = next_byte_offset;
  uint32_t len = Fetch2Bytes();
  uint32_t nEndianness = 0;

  // Create the exif info if not already exists
  if (!header.p_exif_info) {
    header.p_exif_info = (exif_info_t *)JPEG_MALLOC(sizeof(exif_info_t));
    if (!header.p_exif_info)
      rc = QI_ERR_NO_MEMORY;
    else
      memset(header.p_exif_info, 0, sizeof(exif_info_t));
  }

  // Match with "Exif"
  if (QI_SUCCEEDED(rc))
    rc = (Fetch4Bytes() == 0x45786966) ? QI_SUCCESS : QI_ERR_GENERAL;

  if (QI_SUCCEEDED(rc)) {
    // Skip the padded bits
    (void)Fetch2Bytes();

    // Store TIFF header offset
    tiff_hdr_offset = next_byte_offset;

    // nEndianness (Big-Endian = 0x4D4D; Little-Endian = 0x4949)
    nEndianness = Fetch2Bytes();
    rc = (nEndianness == 0x4d4d || nEndianness == 0x4949) ?
      QI_SUCCESS : QI_ERR_GENERAL;
  }

  if (QI_SUCCEEDED(rc)) {
    endianness = (nEndianness == 0x4d4d) ?
      EXIF_BIG_ENDIAN : EXIF_LITTLE_ENDIAN;

    // Match the TIFF Version
    rc = (Fetch2Bytes() == 0x2A) ? QI_SUCCESS : QI_ERR_GENERAL;
  }

  if (QI_SUCCEEDED(rc))
    rc = ProcessZeroIfd();

  if (QI_SUCCEEDED(rc))
    rc = ProcessExifIfd();

  if (QI_SUCCEEDED(rc))
    rc = ProcessGpsIfd();

  if (QI_SUCCEEDED(rc))
    rc = ProcessFirstIfd();

  if (QI_ERROR(rc)) {
    exif_info_obj_t exif_info_obj = (exif_info_obj_t)header.p_exif_info;
    exif_destroy(&exif_info_obj);
    header.p_exif_info = NULL;
  }

  next_byte_offset = app1_start_offset + len;
  return rc;
}

/*===========================================================================
 * Function: ProcessApp3
 *
 * Description: process App3 marker
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessApp3()
{
  int rc = QI_SUCCESS;
  uint32_t app3_start_offset = next_byte_offset;
  uint16_t app3_len = Fetch2Bytes();
  uint8_t  misc_byte, layout_byte, type_byte;

  // Zero out all fields
  memset(&(header.jps_info), 0, sizeof(jps_cfg_3d_t));

  // Match with "_JPSJPS_"
  if (QI_SUCCEEDED(rc) &&
    Fetch4Bytes() == 0x5F4A5053 &&
    Fetch4Bytes() == 0x4A50535F) {
    rc = QI_SUCCESS;
  } else {
    rc = QI_ERR_GENERAL;
  }

  if (QI_SUCCEEDED(rc)) {
    // consume descriptor length (uint16_t)
    (void)Fetch2Bytes();

    // extract seperation (uint8_t)
    header.jps_info.separation = FetchBytes();

    // extract Misc. Flags (uint8_t)
    misc_byte = FetchBytes();

    header.jps_info.height_flag = (jps_height_t)(IS_HALF_HEIGHT(misc_byte));
    header.jps_info.width_flag  = (jps_width_t)(IS_HALF_WIDTH(misc_byte));
    header.jps_info.field_order =
      (jps_field_order_t)(IS_LEFT_FIRST(misc_byte));

    // extract Layout
    layout_byte = (jps_layout_t)FetchBytes();

    // only support side_by_side and over_under format
    if (layout_byte != 0x02 && layout_byte != 0x03) {
      rc = JPEGERR_EUNSUPPORTED;
    }

    header.jps_info.layout = (jps_layout_t)layout_byte;

    // extract Type
    type_byte = FetchBytes();

    // only supports stereoscopic image
    if (type_byte != 0x01) {
      rc = JPEGERR_EUNSUPPORTED;
    }
  }

  next_byte_offset = app3_start_offset + app3_len;

  return rc;
}

/*===========================================================================
 * Function: ProcessZeroIfd
 *
 * Description: process 0th Ifd
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessZeroIfd()
{
  int rc = QI_SUCCESS;
  uint32_t nIfdOffset = Fetch4Bytes();
  int nNumTags, i;

  next_byte_offset = tiff_hdr_offset + nIfdOffset;

  nNumTags = Fetch2Bytes();
  for (i = 0; i < nNumTags; i++) {

    uint16_t tag_id = Fetch2Bytes();
    switch (tag_id) {
    case _ID_IMAGE_DESCRIPTION:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_image_description,
        EXIF_ASCII, EXIFTAGID_IMAGE_DESCRIPTION);
      break;
    case _ID_MAKE:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_make,
        EXIF_ASCII, EXIFTAGID_MAKE);
      break;
    case _ID_MODEL:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_model,
        EXIF_ASCII, EXIFTAGID_MODEL);
      break;
    case _ID_ORIENTATION:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_orientation,
        EXIF_SHORT, EXIFTAGID_ORIENTATION);
      break;
    case _ID_X_RESOLUTION:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_x_resolution,
        EXIF_RATIONAL, EXIFTAGID_X_RESOLUTION);
      break;
    case _ID_Y_RESOLUTION:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_y_resolution,
        EXIF_RATIONAL, EXIFTAGID_Y_RESOLUTION);
      break;
    case _ID_RESOLUTION_UNIT:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_resolution_unit,
        EXIF_SHORT, EXIFTAGID_RESOLUTION_UNIT);
      break;
    case _ID_YCBCR_POSITIONING:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_ycbcr_positioning,
        EXIF_SHORT, EXIFTAGID_YCBCR_POSITIONING);
      break;
    case _ID_COMPRESSION:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_compression,
        EXIF_SHORT, EXIFTAGID_COMPRESSION);
      break;
    case _ID_SOFTWARE:
      rc = FetchTag(&header.p_exif_info->tiff_ifd.p_software,
        EXIF_ASCII, EXIFTAGID_SOFTWARE);
      break;
    case _ID_EXIF_IFD_PTR:
    case _ID_GPS_IFD_PTR: {
      const uint16_t exif_type = Fetch2Bytes();
      const uint32_t cnt = Fetch4Bytes();
      if (exif_type != EXIF_LONG || cnt != 1)
          return QI_ERR_GENERAL;
      if (tag_id == _ID_EXIF_IFD_PTR)
          exif_ifd_offset = Fetch4Bytes();
      else
          gps_ifd_offset = Fetch4Bytes();
    }
      break;
    default:
      next_byte_offset += 10;
      break;
    }

    if (rc != JPEGERR_TAGTYPE_UNEXPECTED && QI_ERROR(rc))
      return rc;
  }
  first_ifd_offset = Fetch4Bytes();

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ProcessExifIfd
 *
 * Description: process Exif Ifd
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessExifIfd()
{
  int rc = QI_SUCCESS;
  uint16_t nNumTags, i;
  uint32_t nSaveOffset;

  if (!exif_ifd_offset)
    return QI_SUCCESS;

  next_byte_offset = tiff_hdr_offset + exif_ifd_offset;

  nNumTags = Fetch2Bytes();
  for (i = 0; i < nNumTags; i++) {
    uint16_t tag_id = Fetch2Bytes();

    switch (tag_id) {
    case _ID_EXIF_VERSION:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_version,
        EXIF_UNDEFINED, EXIFTAGID_EXIF_VERSION);
      break;
    case _ID_EXIF_DATE_TIME_ORIGINAL:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_date_time_original,
        EXIF_ASCII, EXIFTAGID_EXIF_DATE_TIME_ORIGINAL);
      break;
    case _ID_EXIF_COMPONENTS_CONFIG:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_components_config,
        EXIF_UNDEFINED, EXIFTAGID_EXIF_COMPONENTS_CONFIG);
      break;
    case _ID_EXIF_USER_COMMENT:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_user_comment,
        EXIF_UNDEFINED, EXIFTAGID_EXIF_USER_COMMENT);
      break;
    case _ID_EXIF_MAKER_NOTE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_maker_note,
        EXIF_UNDEFINED, EXIFTAGID_EXIF_MAKER_NOTE);
      break;
    case _ID_EXIF_FLASHPIX_VERSION:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_flashpix_version,
        EXIF_UNDEFINED, EXIFTAGID_EXIF_FLASHPIX_VERSION);
      break;
    case _ID_EXIF_COLOR_SPACE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_color_space,
        EXIF_SHORT, EXIFTAGID_EXIF_COLOR_SPACE);
      break;
    case _ID_EXIF_PIXEL_X_DIMENSION:
      nSaveOffset = next_byte_offset;
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_pixel_x_dimension,
        EXIF_LONG, EXIFTAGID_EXIF_PIXEL_X_DIMENSION);

      if (rc == JPEGERR_TAGTYPE_UNEXPECTED) {
        next_byte_offset = nSaveOffset;
        rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_pixel_x_dimension,
        EXIF_SHORT, EXIFTAGID_EXIF_PIXEL_X_DIMENSION);
      }
      break;
    case _ID_EXIF_PIXEL_Y_DIMENSION:
      nSaveOffset = next_byte_offset;
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_pixel_y_dimension,
        EXIF_LONG, EXIFTAGID_EXIF_PIXEL_Y_DIMENSION);
      if (rc == JPEGERR_TAGTYPE_UNEXPECTED) {
        next_byte_offset = nSaveOffset;
        rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_pixel_y_dimension,
        EXIF_SHORT, EXIFTAGID_EXIF_PIXEL_Y_DIMENSION);
      }
      break;
    case _ID_EXPOSURE_TIME:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exposure_time,
        EXIF_RATIONAL, EXIFTAGID_EXPOSURE_TIME);
      break;
    case _ID_EXIF_DATE_TIME_DIGITIZED:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exif_date_time_digitized,
        EXIF_ASCII, EXIFTAGID_EXIF_DATE_TIME_DIGITIZED);
      break;
    case _ID_FLASH:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_flash,
        EXIF_SHORT, EXIFTAGID_FLASH);
      break;
    case _ID_CUSTOM_RENDERED:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_custom_rendered,
        EXIF_SHORT, EXIFTAGID_CUSTOM_RENDERED);
      break;
    case _ID_EXPOSURE_MODE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exposure_mode,
        EXIF_SHORT, EXIFTAGID_EXPOSURE_MODE);
      break;
    case _ID_WHITE_BALANCE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_white_balance,
        EXIF_SHORT, EXIFTAGID_WHITE_BALANCE);
      break;
    case _ID_DIGITAL_ZOOM_RATIO:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_digital_zoom_ratio,
        EXIF_RATIONAL, EXIFTAGID_DIGITAL_ZOOM_RATIO);
      break;
    case _ID_SCENE_CAPTURE_TYPE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_scene_capture_type,
        EXIF_SHORT, EXIFTAGID_SCENE_CAPTURE_TYPE);
      break;
    case _ID_SUBJECT_DISTANCE_RANGE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_subject_distance_range,
        EXIF_SHORT, EXIFTAGID_SUBJECT_DISTANCE_RANGE);
      break;
    case _ID_APERTURE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_aperture,
        EXIF_RATIONAL, EXIFTAGID_APERTURE);
      break;
    case _ID_MAX_APERTURE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_max_aperture,
        EXIF_RATIONAL, EXIFTAGID_MAX_APERTURE);
      break;
    case _ID_LIGHT_SOURCE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_light_source,
        EXIF_SHORT, EXIFTAGID_LIGHT_SOURCE);
      break;
    case _ID_SENSING_METHOD:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_sensing_method,
        EXIF_SHORT, EXIFTAGID_SENSING_METHOD);
      break;
    case _ID_FILE_SOURCE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_file_source,
        EXIF_UNDEFINED, EXIFTAGID_FILE_SOURCE);
      break;
    case _ID_CFA_PATTERN:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_cfa_pattern,
        EXIF_UNDEFINED, EXIFTAGID_CFA_PATTERN);
      break;
    case _ID_FOCAL_LENGTH_35MM:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_focal_length_35mm,
        EXIF_SHORT, EXIFTAGID_FOCAL_LENGTH_35MM);
      break;
    case _ID_CONTRAST:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_contrast,
        EXIF_SHORT, EXIFTAGID_CONTRAST);
      break;
    case _ID_SATURATION:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_saturation,
        EXIF_SHORT, EXIFTAGID_SATURATION);
      break;
    case _ID_METERING_MODE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_metering_mode,
        EXIF_SHORT, EXIFTAGID_METERING_MODE);
      break;
    case _ID_SHARPNESS:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_sharpness,
        EXIF_SHORT, EXIFTAGID_SHARPNESS);
      break;
    case _ID_GAIN_CONTROL:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_gain_control,
        EXIF_SHORT, EXIFTAGID_GAIN_CONTROL);
      break;
    case _ID_F_NUMBER:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_f_number,
        EXIF_RATIONAL, EXIFTAGID_F_NUMBER);
      break;
    case _ID_OECF:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_oecf,
        EXIF_UNDEFINED, EXIFTAGID_OECF);
      break;
    case _ID_SHUTTER_SPEED:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_shutter_speed,
        EXIF_SRATIONAL, EXIFTAGID_SHUTTER_SPEED);
      break;
    case _ID_EXPOSURE_PROGRAM:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exposure_program,
        EXIF_SHORT, EXIFTAGID_EXPOSURE_PROGRAM);
      break;
    case _ID_BRIGHTNESS:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_brightness,
        EXIF_SRATIONAL, EXIFTAGID_BRIGHTNESS);
      break;
    case _ID_FOCAL_LENGTH:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_focal_length,
        EXIF_RATIONAL, EXIFTAGID_FOCAL_LENGTH);
      break;
    case _ID_EXPOSURE_INDEX:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_exposure_index,
        EXIF_RATIONAL, EXIFTAGID_EXPOSURE_INDEX);
      break;
    case _ID_SUBJECT_DISTANCE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_subject_distance,
        EXIF_RATIONAL, EXIFTAGID_SUBJECT_DISTANCE);
      break;
    case _ID_SUBJECT_LOCATION:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_subject_location,
        EXIF_SHORT, EXIFTAGID_SUBJECT_LOCATION);
      break;
    case _ID_SCENE_TYPE:
      rc = FetchTag(&header.p_exif_info->exif_ifd.p_scene_type,
        EXIF_UNDEFINED, EXIFTAGID_SCENE_TYPE);
      break;
    case _ID_INTEROP_IFD_PTR: {
      const uint16_t exif_type = Fetch2Bytes();
      const uint32_t cnt = Fetch4Bytes();
      if (exif_type != EXIF_LONG || cnt != 1)
          return QI_ERR_GENERAL;
      interop_ifd_offset = Fetch4Bytes();
    }
      break;
    default:
      next_byte_offset += 10;
      break;
    }
    if (rc != JPEGERR_TAGTYPE_UNEXPECTED && QI_ERROR(rc))
      return rc;
  }

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ProcessGpsIfd
 *
 * Description: process GPS Ifd
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessGpsIfd()
{
  int rc = QI_SUCCESS;
  int nNumTags, i;

  if (!gps_ifd_offset)
    return QI_SUCCESS;

  next_byte_offset = tiff_hdr_offset + gps_ifd_offset;

  nNumTags = Fetch2Bytes();
  for (i = 0; i < nNumTags; i++) {
    uint16_t tag_id = Fetch2Bytes();
    switch (tag_id) {
    case _ID_GPS_VERSION_ID:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_version_id,
        EXIF_BYTE, EXIFTAGID_GPS_VERSION_ID);
      break;
    case _ID_GPS_LATITUDE_REF:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_latitude_ref,
        EXIF_ASCII, EXIFTAGID_GPS_LATITUDE_REF);
      break;
    case _ID_GPS_LATITUDE:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_latitude,
        EXIF_RATIONAL, EXIFTAGID_GPS_LATITUDE);
      break;
    case _ID_GPS_LONGITUDE_REF:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_longitude_ref,
        EXIF_ASCII, EXIFTAGID_GPS_LONGITUDE_REF);
      break;
    case _ID_GPS_LONGITUDE:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_longitude,
        EXIF_RATIONAL, EXIFTAGID_GPS_LONGITUDE);
      break;
    case _ID_GPS_ALTITUDE_REF:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_altitude_ref,
        EXIF_BYTE, EXIFTAGID_GPS_ALTITUDE_REF);
      break;
    case _ID_GPS_TIMESTAMP:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_timestamp,
        EXIF_RATIONAL, EXIFTAGID_GPS_TIMESTAMP);
      break;
    case _ID_GPS_MAPDATUM:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_map_datum,
        EXIF_ASCII, EXIFTAGID_GPS_MAPDATUM);
      break;
    case _ID_GPS_AREAINFORMATION:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_area_information,
        EXIF_UNDEFINED, EXIFTAGID_GPS_AREAINFORMATION);
      break;
    case _ID_GPS_DATESTAMP:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_datestamp,
        EXIF_ASCII, EXIFTAGID_GPS_DATESTAMP);
      break;
    case _ID_GPS_PROCESSINGMETHOD:
      rc = FetchTag(&header.p_exif_info->gps_ifd.p_processing_method,
        EXIF_UNDEFINED, EXIFTAGID_GPS_PROCESSINGMETHOD);
      break;
    default:
      next_byte_offset += 10;
      break;
    }
    if (rc != JPEGERR_TAGTYPE_UNEXPECTED && QI_ERROR(rc))
      return rc;
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ProcessFirstIfd
 *
 * Description: process 1st Ifd
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::ProcessFirstIfd()
{
  int rc = QI_SUCCESS;
  uint16_t nNumTags, i;

  if (!first_ifd_offset)
      return QI_SUCCESS;

  next_byte_offset = tiff_hdr_offset + first_ifd_offset;

  nNumTags = Fetch2Bytes();
  for (i = 0; i < nNumTags; i++) {
    uint16_t tag_id = Fetch2Bytes();
    switch (tag_id) {
    case _ID_IMAGE_DESCRIPTION:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_image_description,
        EXIF_ASCII, EXIFTAGID_IMAGE_DESCRIPTION);
      break;
    case _ID_MAKE:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_make,
        EXIF_ASCII, EXIFTAGID_MAKE);
      break;
    case _ID_MODEL:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_model,
        EXIF_ASCII, EXIFTAGID_MODEL);
      break;
    case _ID_ORIENTATION:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_orientation,
        EXIF_SHORT, EXIFTAGID_ORIENTATION);
      break;
    case _ID_X_RESOLUTION:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_x_resolution,
        EXIF_RATIONAL, EXIFTAGID_X_RESOLUTION);
      break;
    case _ID_Y_RESOLUTION:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_y_resolution,
        EXIF_RATIONAL, EXIFTAGID_Y_RESOLUTION);
      break;
    case _ID_RESOLUTION_UNIT:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_resolution_unit,
        EXIF_SHORT, EXIFTAGID_RESOLUTION_UNIT);
      break;
    case _ID_YCBCR_POSITIONING:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_ycbcr_positioning,
        EXIF_SHORT, EXIFTAGID_YCBCR_POSITIONING);
      break;
    case _ID_COMPRESSION:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_compression,
        EXIF_SHORT, EXIFTAGID_COMPRESSION);
      break;
    case _ID_SOFTWARE:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_software,
        EXIF_ASCII, EXIFTAGID_SOFTWARE);
      break;
    case _ID_TN_JPEGINTERCHANGE_FORMAT:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_interchange_format,
        EXIF_LONG, EXIFTAGID_TN_JPEGINTERCHANGE_FORMAT);
      break;
    case _ID_TN_JPEGINTERCHANGE_FORMAT_L:
      rc = FetchTag(&header.p_exif_info->thumbnail_ifd.p_interchange_format_l,
        EXIF_LONG, EXIFTAGID_TN_JPEGINTERCHANGE_FORMAT_L);
      break;
    default:
      break;
    }
    if (rc != JPEGERR_TAGTYPE_UNEXPECTED && QI_ERROR(rc))
        return rc;
  }
  if (QI_SUCCEEDED(rc) &&
      header.p_exif_info->thumbnail_ifd.p_interchange_format &&
      header.p_exif_info->thumbnail_ifd.p_interchange_format_l) {
    // Process thumbnail
    next_byte_offset = tiff_hdr_offset +
      header.p_exif_info->thumbnail_ifd.p_interchange_format->entry.data._long;

    rc = FindSOI();
    if (QI_SUCCEEDED(rc)) {
      jpeg_frame_info_destroy(header.p_tn_frame_info);
      header.p_tn_frame_info = (jpeg_frame_info_t *)
        JPEG_MALLOC(sizeof(jpeg_frame_info_t));
      if (!header.p_tn_frame_info) {
        rc = QI_ERR_NO_MEMORY;
      } else {
        memset(header.p_tn_frame_info, 0, sizeof(jpeg_frame_info_t));
      }
    }

    if (QI_SUCCEEDED(rc)) {
      exif_endianness_t save = endianness;
      endianness = EXIF_BIG_ENDIAN;
      rc = ParseSOF(header.p_tn_frame_info);
      endianness = save;
    }
  }
  return rc;
}

/*===========================================================================
 * Function: FetchTag
 *
 * Description: Create a tag and fill in the information based on the input
 *              stream
 *
 * Input parameters:
 *   pp_tag_entry -
 *   expected_type -
 *   tag_id -
 *
 * Return values:
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
int QExifParser::FetchTag( exif_tag_entry_ex_t **pp_tag_entry,
  uint16_t expected_type,
  exif_tag_id_t tag_id)
{
  uint32_t tag_start_offset, cnt, i;
  int rc = QI_SUCCESS;
  exif_tag_entry_ex_t *p_new_tag;

  // Destroy any previously allocated entry
  exif_destroy_tag_entry(*pp_tag_entry);

  p_new_tag = exif_create_tag_entry();

  if (!p_new_tag)
    return QI_ERR_NO_MEMORY;

  // Save the current offset for possible later seeking
  tag_start_offset      = next_byte_offset;
  p_new_tag->entry.type = (exif_tag_type_t)Fetch2Bytes();
  cnt                   = Fetch4Bytes();

  // Check if the fetched type is expected
  if (p_new_tag->entry.type != expected_type) {
    // Reset fetch offset
    next_byte_offset = tag_start_offset;
    exif_destroy_tag_entry(p_new_tag);
    //nullify the pp_tag_entry, since it is already destroyed,
    //so that next time when FetchTag is been called in,
    //it will not double free the tag entry.
    *pp_tag_entry = NULL;
    return JPEGERR_TAGTYPE_UNEXPECTED;
  }

  switch (expected_type) {
  case EXIF_BYTE:
    if (cnt > 1) {
      p_new_tag->entry.data._bytes = (uint8_t *)JPEG_MALLOC(cnt);

      if (!p_new_tag->entry.data._bytes) {
          rc = QI_ERR_NO_MEMORY;
          break;
      }

      // data doesn't fit into 4 bytes, read from an offset
      if (cnt > 4)
          next_byte_offset = tiff_hdr_offset + Fetch4Bytes();

      FetchNBytes(p_new_tag->entry.data._bytes, cnt, NULL);
    } else {
      p_new_tag->entry.data._byte = FetchBytes();
    }
    break;
  case EXIF_SHORT:
    if (cnt > 1) {
      //Check if cnt * sizeof(uint16_t) is overflowing before
      // allocating memory
      // cnt * sizeof(uint16_t) should be less than the limit of uint32_t
      if(cnt > ((1<<(sizeof(uint32_t)*8 - sizeof(uint16_t)))-1)) {
        //This means it is been overflown,
        //and not something good to proceed
        QIDBG_ERROR(" %s Too big cnt = 0x%x\n",__func__,cnt);
        rc = QI_ERR_GENERAL;
        break;
      }

      p_new_tag->entry.data._shorts = (uint16_t*)JPEG_MALLOC(
        cnt * sizeof(uint16_t));
      if (!p_new_tag->entry.data._shorts) {
        rc = QI_ERR_NO_MEMORY;
        break;
      }

      // Data doesn't fit into 4 bytes, read from an offset
      if (cnt > 4)
        next_byte_offset = tiff_hdr_offset + Fetch4Bytes();

      for (i = 0; i < cnt; i++) {
        p_new_tag->entry.data._shorts[i] = Fetch2Bytes();
      }
    } else {
      p_new_tag->entry.data._short = Fetch2Bytes();
    }
    break;
  case EXIF_LONG:
  case EXIF_SLONG:
    if (cnt > 1) {
      //Check if cnt * sizeof(uint32_t) is overflowing before
      // allocating memory
      // cnt * sizeof(uint32_t) should be less than the limit of uint32_t
      if(cnt > ((1<<(sizeof(uint32_t)*8 - sizeof(uint32_t)))-1)) {
        //This means it is been overflown,
        //and not something good to proceed
        QIDBG_ERROR(" %s Too big cnt = 0x%x\n",__func__,cnt);
        rc = QI_ERR_GENERAL;
        break;
      }
      p_new_tag->entry.data._longs = (uint32_t *)JPEG_MALLOC(
        cnt * sizeof(uint32_t));
      if (!p_new_tag->entry.data._longs) {
        rc = QI_ERR_NO_MEMORY;
        break;
      }

      // Data doesn't fit into 4 bytes, read from an offset
      next_byte_offset = tiff_hdr_offset + Fetch4Bytes();
      for (i = 0; i < cnt; i++) {
        p_new_tag->entry.data._longs[i] = (uint32_t)Fetch4Bytes();
      }
    } else {
      p_new_tag->entry.data._long = (uint32_t)Fetch4Bytes();
    }
    break;
  case EXIF_RATIONAL:
  case EXIF_SRATIONAL:
    // Data doesn't fit into 4 bytes, read from an offset
    next_byte_offset = tiff_hdr_offset + Fetch4Bytes();
    if (cnt > 1) {
      //Check if cnt * sizeof(rat_t) is overflowing before
      // allocating memory
      // cnt * sizeof(rat_t) should be less than the limit of uint32_t
       if(cnt > ((1<<(sizeof(uint32_t)*8 - sizeof(rat_t)))-1)) {
         //This means it is been overflown,
         //and not something good to proceed
         QIDBG_ERROR(" %s Too big cnt = 0x%x\n",__func__,cnt);
         rc = QI_ERR_GENERAL;
         break;
       }

       p_new_tag->entry.data._rats =
         (rat_t *)JPEG_MALLOC(cnt * sizeof(rat_t));
       if (!p_new_tag->entry.data._rats) {
         rc = QI_ERR_NO_MEMORY;
         break;
       }

       for (i = 0; i < cnt; i++) {
         p_new_tag->entry.data._rats[i].num   = Fetch4Bytes();
         p_new_tag->entry.data._rats[i].denom = Fetch4Bytes();
       }
    } else {
      p_new_tag->entry.data._rat.num   = Fetch4Bytes();
      p_new_tag->entry.data._rat.denom = Fetch4Bytes();
    }
    break;
  case EXIF_ASCII:
  case EXIF_UNDEFINED: {
    //Check if cnt * sizeof(char) is overflowing before allocating memory
    // cnt * sizeof(char) should be less than the limit of uint32_t
    if(cnt > (0xFFFFFFFF/sizeof(char))) {
      //This means it is been overflown,
      //and not something good to proceed
      QIDBG_ERROR(" %s Too big cnt = 0x%x\n",__func__,cnt);
      rc = QI_ERR_GENERAL;
      break;
    }
    p_new_tag->entry.data._ascii = (char *)JPEG_MALLOC(
      cnt * sizeof(char));
    if (!p_new_tag->entry.data._ascii) {
      rc = QI_ERR_NO_MEMORY;
      break;
    }

    if (cnt > 4)
      next_byte_offset = tiff_hdr_offset + Fetch4Bytes();

    FetchNBytes((uint8_t *)p_new_tag->entry.data._ascii, cnt, NULL);
  }
    break;
  default:
    break;
  }

  if (QI_SUCCEEDED(rc)) {
    p_new_tag->entry.copy = true;
    p_new_tag->entry.count = cnt;
    p_new_tag->tag_id = tag_id;
    *pp_tag_entry = p_new_tag;
    next_byte_offset = tag_start_offset + 10;
  } else {
    next_byte_offset = tag_start_offset;
    exif_destroy_tag_entry(p_new_tag);
    //nullify the pp_tag_entry, since it is already destroyed,
    //so that next time when FetchTag is been called in,
    //it will not double free the tag entry.
    *pp_tag_entry = NULL;
  }
  return rc;
}

