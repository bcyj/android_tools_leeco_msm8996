/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QMOBICATCOMPOSER_H__
#define __QMOBICATCOMPOSER_H__

extern "C" {
#include "mct_stream.h"
}

/*===========================================================================
 * Class: QMobicatComposer
 *
 * Description: This class represents the mobicat composer utility
 *
 * Notes: none
 *==========================================================================*/
class QMobicatComposer {

public:

  /** QMobicatComposer
   *
   *  Constructor
   **/
  QMobicatComposer();

  /** ~QMobicatComposer
   *
   *  virtual destructor
   **/
  ~QMobicatComposer();

   /** ParseMobicatData
   *  @metadata: contains metadata info
   *
   *  Parse metadata into mobicat tags and return string
   *
   **/
  char* ParseMobicatData(uint8_t *metadata);

   /** Compose3AStatsPayload
   *  @metadata: contains metadata info
   *
   *  compose stats payload
   **/
  char* Compose3AStatsPayload(uint8_t *metadata);

  /** Get3AStatsSize
   *
   *  return length of stats payload
   **/
  uint32_t Get3AStatsSize();

   /** ExtractAECData
   *  @lMeta: contains metadata info
   *
   *  Get AEC data from metadata and add to stats payload
   **/
  void ExtractAECData(cam_metadata_info_t *lMeta);

   /** ExtractAWBData
   *  @lMeta: contains metadata info
   *
   *  Get AWB data from metadata and add to stats payload
   **/
  void ExtractAWBData(cam_metadata_info_t *lMeta);

  /** ExtractAFData
   *  @lMeta: contains metadata info
   *
   *  Get AF data from metadata and add to stats payload
   **/
  void ExtractAFData(cam_metadata_info_t *lMeta);

  /** ExtractASDData
   *  @lMeta: contains metadata info
   *
   *  Get ASD data from metadata and add to stats payload
   **/
  void ExtractASDData(cam_metadata_info_t *lMeta);

  /** ExtractStatsData
   *  @lMeta: contains metadata info
   *
   *  Get Stats data from metadata and add to stats payload
   **/
  void ExtractStatsData(cam_metadata_info_t *lMeta);

private:

  /** mScratchBuf
   *
   *  Temp scratch buffer
   **/
  char *mScratchBuf;

  /** mMobicatStr
   *
   *  Parsed Mobicat String
   **/
  char *mMobicatStr;

  /** mStatsPayload
   *
   *  Stats payload
   **/
  char *mStatsPayload;

  /** mStats_payload_size
   *
   *  Stats payload size
   **/
  uint32_t mStats_payload_size;

  /** parseVal
   *
   * @fmt - output format string
   * @aTag - mobicat tag
   * @aVal - value to parse
   *
   * Parse a value of type T
   */
  template <typename T> void parseVal(const char *fmt,
      const char *aTag, T aVal);

  /** parseValArr
   *
   * @fmt - output format string
   * @aTag - mobicat tag
   * @aValPtr - array to parse
   * @aLen - Length of array
   *
   * Parse an array of type T
   */
  template <typename T> void parseValArr(const char *fmt,
      const char *aTag, T *aValPtr, int aLen);

};

#endif //__QMOBICATCOMPOSER_H__
