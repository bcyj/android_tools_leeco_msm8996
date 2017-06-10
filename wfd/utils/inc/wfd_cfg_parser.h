#ifndef _WFD_CFG_PARSER_H
#define _WFD_CFG_PARSER_H

/*==============================================================================
*        @file wfd_cfg_parser.h
*
*  @par DESCRIPTION:
*       Header file of the wfd configuration file parser
*
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

$Header: //source/qcom/qct/multimedia2/Video/wfd/utils/main/latest/inc/wfd_cfg_parser.h#5 $
$DateTime: 2012/02/24 03:40:43 $
$Change: 2227180 $

===============================================================================*/
/*===============================================================================
                            Header Inclusion
===============================================================================*/
#include "wdsm_mm_interface.h"
#include "UIBCDefs.h"
/*
 * Top level XML parsing Class
 */
enum  audioCodecName {
      LPCM,
      AAC,
      AC3
 };

enum  modes {
      WFD_MM_CEA_MODE,
      WFD_MM_VESA_MODE,
      WFD_MM_HH_MODE,
      WFD_MM_AAC_MODE,
      WFD_MM_DOLBY_DIGITAL_MODE
};

typedef struct
{
   uint32 idrIntvl;
   bool idrReqValid;
}idrFrameRequest;


typedef struct
{
   uint32 frameSkipIntvl;
   bool frameSkipValid;
}frameSkipRequest;


typedef struct {
   WFD_MM_capability_t *pCfgCapabilities;
   idrFrameRequest      idrRequestCapability;
   frameSkipRequest     frameSkippingCapability;
   WFD_uibc_capability_t uibcCapability;
}readConfigFile;

#ifndef WFD_CFG_FILE
#define WFD_CFG_FILE "/system/etc/wfdconfig.xml"
#endif
#ifndef WFD_CFG_FILE_SINK
#define WFD_CFG_FILE_SINK "/system/etc/wfdconfigsink.xml"
#endif

/* These config keys serve as index to the respective
 * config items in the config array. These should be in
 * sync with values in WfdEnums.java
*/
typedef enum  {
  AUDIO_AV_SYNC_DEL,
  AUDIO_IN_SUSPEND,
  CYCLIC_IR,
  CYCLIC_IR_NUM_MACRO_BLK,
  DISABLE_NALU_FILLER,
  DYN_BIT_ADAP,
  ENCRYPT_AUDIO_DECISION,
  ENCRYPT_NON_SECURE,
  HDCP_ENFORCED,
  PERF_LEVEL_PERF_MODE,
  PERF_LEVEL_TURBO_MODE,
  RTP_DUMP_ENABLE,
  UIBC_M14,
  UIBC_VALID,
  VIDEO_PKTLOSS_FRAME_DROP_MODE,
  DISABLE_AVSYNC_MODE,
  MAX_FPS_SUPPORTED,
  PERIODIC_IDR_INTERVAL_VALID,
  PERIODIC_IDR_INTERVAL,
  NARROW_SEARCH_RANGE,
  ENABLE_AUDIO_TRACK_LATENCY_MODE,
  AUDIO_AVSYNC_DROP_WINDOW,
  VIDEO_AVSYNC_DROP_WINDOW,
  TOTAL_CFG_KEYS //update WfdEnums.java as well
}cfgKeys;

//List of all configurable item keys
#define AUDIO_AV_SYNC_DEL_KEY "AudioAVSyncDelay"
#define AUDIO_IN_SUSPEND_KEY "AudioStreamInSuspend"
#define CYCLIC_IR_KEY "CyclicIntraRefresh"
#define CYCLIC_IR_NUM_MACRO_BLK_KEY "CIRNumberOfMacroBlocks"
#define DISABLE_NALU_FILLER_KEY "DisableFillerNalU"
#define DYN_BIT_ADAP_KEY "DynamicBitrateAdaptation"
#define ENCRYPT_AUDIO_DECISION_KEY "EncryptAudio"
#define ENCRYPT_NON_SECURE_KEY "EncyptNonSecureContent"
#define HDCP_ENFORCED_KEY "EnforceLinkHDCP"
#define PERF_LEVEL_PERF_MODE_KEY "PerformanceLevelPerformanceMode"
#define PERF_LEVEL_TURBO_MODE_KEY "PerformanceLevelTurboMode"
#define UIBC_M14_KEY "M14Support"
#define UIBC_VALID_KEY "UIBCValid"
#define VIDEO_PKTLOSS_FRAME_DROP_MODE_KEY "DropFrameOnPacketLossMode"
#define DISABLE_AVSYNC_MODE_KEY "DisableAVSync"
#define MAX_FPS_SUPPORTED_KEY "MaxFpsSupported"
#define PERIODIC_IDR_INTERVAL_VALID_KEY "PeriodicIDRValid"
#define PERIODIC_IDR_INTERVAL_KEY "PeriodicIDRInterval"
#define NARROW_SEARCH_RANGE_KEY "NarrowSearchRange"
#define ENABLE_AUDIO_TRACK_LATENCY_MODE_KEY "EnableAudioTrackLatency"
#define AUDIO_AVSYNC_DROP_WINDOW_KEY "AudioAVSyncDropWindow"
#define VIDEO_AVSYNC_DROP_WINDOW_KEY "VideoAVSyncDropWindow"


#ifdef __cplusplus
extern "C" {
#endif
    void parseCfgforUIBC(const char *filename, readConfigFile *readCfgCaps);
    void parseCfg(const char *filename,readConfigFile *readCapabilities );
    void ParseCfgForBitrate(int mode, int bit, char *pFilename, int *minBitrate, int *maxBitrate);
    int  PargeCfgForIntValueForKey(char *filename, char *pKey, int *Val);
    int getCfgItem(const char*, int*);
    void resetCfgItems();
#ifdef __cplusplus
}
#endif

#endif /* _WFD_CFG_PARSER_H*/

