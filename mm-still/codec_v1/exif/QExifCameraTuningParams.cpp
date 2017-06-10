/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/
#include "QExifCameraTuningParams.h"

extern "C" {
#include "mct_stream.h"
#include "mct_event_stats.h"
}

/**
 * PARSE_F - parse a float value
 * @S - ptr to container structure
 * @F - field to parse
 */
#define PARSE_F(S,F)    (parseVal<float>("%f",     #F, (S)->F))
/**
 * PARSE_U - parse an unsigned value
 * @S - ptr to container structure
 * @F - field to parse
 */
#define PARSE_U(S,F)    (parseVal<uint32_t>("%lu", #F, (S)->F))
/**
 * PARSE_C - parse a char value
 * @S - ptr to container structure
 * @F - field to parse
 */
#define PARSE_C(S,F)    (parseVal<char>("%hhd",    #F, (S)->F))
/**
 * PARSE_I - parse an int value
 * @S - ptr to container structure
 * @F - field to parse
 */
#define PARSE_I(S,F)    (parseVal<int>("%d",       #F, (S)->F))
/**
 * PARSE_SI - parse a short int value
 * @S - ptr to container structure
 * @F - field to parse
 */
#define PARSE_SI(S,F)   (parseVal<short>("%hd",    #F, (S)->F))
/**
 * PARSE_F_A1 - parse a 1D float array
 * @S - ptr to container structure
 * @F - field to parse
 */
#define PARSE_F_A1(S,F) (parseValArr<float>("%f", #F, &((S)->F[0]), \
    sizeof((S)->F)/sizeof((S)->F[0])))
/**
 * PARSE_I_A1 - parse a 1D int array
 * @S - ptr to container structure
 * @F - field to parse
 */
#define PARSE_I_A1(S,F) (parseValArr<int>("%d", #F, &((S)->F[0]), \
    sizeof((S)->F)/sizeof((S)->F[0])))
/**
 * PARSE_U_A1 - parse a 1D unsigned array
 * @S - ptr to container structure
 * @F - field to parse
 */
#define PARSE_U_A1(S,F) (parseValArr<uint32_t>("%lu", #F, &((S)->F[0]), \
    sizeof((S)->F)/sizeof((S)->F[0])))
/**
 * PARSE_F_A2 - parse a 2D float array
 * @S - ptr to container structure
 * @F - field to parse
 */
#define PARSE_F_A2(S,F) (parseValArr<float>("%f", #F, &((S)->F[0][0]), \
    sizeof((S)->F)/sizeof((S)->F[0][0])))


/*==============================================================================
* Function : QExifCameraTuningParams
* Parameters: void
* Return Value : void
* Description: constructor
==============================================================================*/
QExifCameraTuningParams::QExifCameraTuningParams()
{
  mDstPtr = NULL;
}
/*==============================================================================
* Function : ExtractTuningInfo
* Parameters: aMetadata, aTuningInfo
* Return Value : void
* Description: Extracts and parses tuning info from metadata to a character
* buffer
==============================================================================*/
int QExifCameraTuningParams::ExtractTuningInfo(uint8_t *aMetadata, uint8_t *aTuningInfo)
{
  cam_metadata_info_t *lMeta = (cam_metadata_info_t *) aMetadata;
  mct_stream_session_metadata_info *lSessionMeta =
      (mct_stream_session_metadata_info *) lMeta->private_metadata;
  awb_update_t lAwbData;
  stats_get_data_t lAecData;
  memcpy(&lAwbData, &lSessionMeta->isp_stats_awb_data.private_data,
      sizeof(lAwbData));
  memcpy(&lAecData, &lSessionMeta->stats_aec_data.private_data,
        sizeof(lAecData));

  mDstPtr = (char *) aTuningInfo;

  // AWB STATS DATA
  PARSE_F(&lAwbData.gain, r_gain);
  PARSE_F(&lAwbData.gain, g_gain);
  PARSE_F(&lAwbData.gain, b_gain);
  PARSE_U(&lAwbData, color_temp);

  PARSE_C(&lAwbData, bounding_box.m1);
  PARSE_C(&lAwbData, bounding_box.m2);
  PARSE_C(&lAwbData, bounding_box.m3);
  PARSE_C(&lAwbData, bounding_box.m4);
  PARSE_SI(&lAwbData, bounding_box.c1);
  PARSE_SI(&lAwbData, bounding_box.c2);
  PARSE_SI(&lAwbData, bounding_box.c3);
  PARSE_SI(&lAwbData, bounding_box.c4);

  PARSE_U(&lAwbData, exterme_color_param.t1);
  PARSE_U(&lAwbData, exterme_color_param.t2);
  PARSE_U(&lAwbData, exterme_color_param.t3);
  PARSE_U(&lAwbData, exterme_color_param.t4);
  PARSE_U(&lAwbData, exterme_color_param.mg);
  PARSE_U(&lAwbData, exterme_color_param.t5);

  PARSE_I(&lAwbData, wb_mode);
  PARSE_I(&lAwbData, best_mode);
  PARSE_I_A1(&lAwbData, sample_decision);
  PARSE_I(&lAwbData, grey_world_stats);
  PARSE_I(&lAwbData, ccm_flag);

  PARSE_F_A2(&lAwbData, cur_ccm);

  // AEC DATA
  PARSE_F_A1(&lAecData, aec_get.real_gain);
  PARSE_U_A1(&lAecData, aec_get.linecount);
  PARSE_F(&lAecData, aec_get.led_off_gain);
  PARSE_U(&lAecData, aec_get.led_off_linecount);
  PARSE_I(&lAecData, aec_get.valid_entries);
  PARSE_I(&lAecData, aec_get.trigger_led);
  PARSE_F(&lAecData, aec_get.exp_time);
  PARSE_F(&lAecData, aec_get.lux_idx);
  PARSE_I(&lAecData, flag);

  return (uint8_t *)mDstPtr - aTuningInfo;
}

/*==============================================================================
* Function : parseVal
* Parameters: fmt, aTag, aVal
* Return Value : int
* Description: Parses a value of type T into a string, using format string fmt
==============================================================================*/
template <typename T> int QExifCameraTuningParams::parseVal(const char *fmt,
    const char *aTag, T aVal)
{
  int lCnt = sprintf(mDstPtr, "%s", aTag);
  mDstPtr += lCnt + 1;
  lCnt = sprintf(mDstPtr, fmt, aVal);
  mDstPtr += lCnt + 1;
  return lCnt;
}
/*==============================================================================
* Function : parseValArr
* Parameters: fmt, aTag, aValPtr, aLen
* Return Value : int
* Description: Parses an array of type T into a string, using format string fmt
==============================================================================*/
template <typename T> int QExifCameraTuningParams::parseValArr(const char *fmt,
    const char *aTag, T *aValPtr, int aLen)
{
  char *lStartDst = mDstPtr;

  int lCnt = sprintf(mDstPtr, "%s", aTag);
  mDstPtr += lCnt + 1;

  *mDstPtr++ = '[';
  while (aLen--) {
    lCnt = sprintf(mDstPtr, fmt, *aValPtr);
    mDstPtr += lCnt;
    *mDstPtr++ = ',';
    *mDstPtr++ = ' ';
    aValPtr++;
  }
  *(mDstPtr-2) = ']';
  *(mDstPtr-1) = '\0';

  return mDstPtr - lStartDst;
}
