/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMUX32_REG_H__
#define __DEMUX32_REG_H__

#define ISP_DEMUX32_OFF 0x00000284
#define ISP_DEMUX32_LEN 5

typedef struct ISP_DemuxConfigCmdType {
  /*  period  */
  uint32_t  period         : 3;
  uint32_t  /* reserved */ :29;
  /* Demux Gain 0 Config */
  uint32_t  ch0EvenGain    :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch0OddGain     :10;
  uint32_t  /* reserved */ : 6;
  /* Demux Gain 1 Config */
  uint32_t  ch1Gain        :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch2Gain        :10;
  uint32_t  /* reserved */ : 6;
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

#endif //__DEMUX32_REG_H__
