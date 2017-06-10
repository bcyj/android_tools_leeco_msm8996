/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMUX40_REG_H__
#define __DEMUX40_REG_H__

#define ISP_DEMUX40_OFF             0x00000424
#define ISP_DEMUX40_LEN             7


typedef struct ISP_DemuxConfigCmdType {
  /*  period  */
  uint32_t  period         : 3;
  uint32_t  /* reserved */ :29;
  /* Demux Gain 0 Config */
  uint32_t  ch0EvenGain    :12;
  uint32_t  /* reserved */ : 4;
  uint32_t  ch0OddGain     :12;
  uint32_t  /* reserved */ : 4;
  /* Demux Gain 1 Config */
  uint32_t  ch1Gain        :12;
  uint32_t  /* reserved */ : 4;
  uint32_t  ch2Gain        :12;
  uint32_t  /* reserved */ : 4;
  /* Demux Gain 0 Config */
  uint32_t  R_ch0EvenGain  :12;
  uint32_t  /* reserved */ : 4;
  uint32_t  R_ch0OddGain   :12;
  uint32_t  /* reserved */ : 4;
  /* Demux Gain 1 Config */
  uint32_t  R_ch1Gain      :12;
  uint32_t  /* reserved */ : 4;
  uint32_t  R_ch2Gain      :12;
  uint32_t  /* reserved */ : 4;
  /* Demux Gain 1 Config */
  uint32_t  evenCfg        :32;
  /* Demux Gain 1 Config */
  uint32_t  oddCfg         :32;
} __attribute__((packed, aligned(4))) ISP_DemuxConfigCmdType;

typedef struct ISP_DemuxGainCfgCmdType {
  uint32_t  ch0EvenGain    :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch0OddGain     :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch1Gain        :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch2Gain        :10;
  uint32_t  /* reserved */ : 6;
} __attribute__((packed, aligned(4))) ISP_DemuxGainCfgCmdType;

#endif //__DEMUX40_REG_H__
