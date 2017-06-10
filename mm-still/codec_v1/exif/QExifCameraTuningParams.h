/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/
#ifndef Q_EXIF_CAM_TUNING_PRMS
#define Q_EXIF_CAM_TUNING_PRMS



/*===========================================================================
 * Class: QExifCameraTuningParams
 *
 * Description: This class provides a tuning info extraction and parsing
 * facility
 *
 * Notes: none
 *==========================================================================*/
class QExifCameraTuningParams {
public:
  /**
   * ExtractTuningInfo
   *
   * @aMetada - metadata buffer
   * @aTuningInfo - output tuning info buffer
   *
   * Extract and parse the tuning info. Return length of written data.
   */
  int ExtractTuningInfo(uint8_t *aMetadata, uint8_t *aTuningInfo);
  /**
   * QExifCameraTuningParams
   */
  QExifCameraTuningParams();

private:
  /**
   * mDstPtr
   *
   * Output destination pointer
   */
  char *mDstPtr;
  /**
   * parseVal
   *
   * @fmt - output format string
   * @aTag - tag of value
   * @aVal - value to parse
   *
   * Parse a value of type T
   */
  template <typename T> int parseVal(const char *fmt,
      const char *aTag, T aVal);
  /**
   * parseValArr
   *
   * @fmt - output format string
   * @aTag - tag of value
   * @aVal - value to parse
   *
   * Parse an array of type T
   */
  template <typename T> int parseValArr(const char *fmt,
      const char *aTag, T *aValPtr, int aLen);
};
#endif
