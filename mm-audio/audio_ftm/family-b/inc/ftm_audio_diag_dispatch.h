#ifndef FTM_AUDIO_DIAG_DISPATCH_H
#define FTM_AUDIO_DIAG_DISPATCH_H
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

         F T M  A U D I O   D I A G   D I S P A T C H

GENERAL DESCRIPTION
  This module will supply functions to register with
  the diagnostic monitor and dispatch on specific
  FTM audio commands.

EXTERNALIZED FUNCTIONS


INITIALIZATION AND SEQUENCING REQUIREMENTS

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================
Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to this file.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/inc/ftm_audio_diag_dispatch.h#1 $
  $DateTime: 2011/04/05 20:05:46 $
  $Author: zhongl $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/15/09   zl     Initial revision
===========================================================================*/


#define CODEC_TABLA	1
#define CODEC_SITAR	2
#define CODEC_TAIKO 3
#define CODEC_TAIKO_I2S 4
#define CODEC_CONFIG_SUPPORT 5
void ftm_audio_diag_init(void);

void * ftm_audio_dispatch( void * request, uint16 length );

enum
{
  FTM_1X_C           = 0,
  FTM_WCDMA_C        = 1,
  FTM_GSM_C          = 2,
  FTM_1X_RX_2_C      = 3,
  FTM_BT_C           = 4,
  FTM_I2C_C          = 5,
  FTM_MC_C           = 7,
  FTM_HDR_C          = 8,
  FTM_LOG_C          = 9,
  FTM_AGPS_C         = 10,
  FTM_PMIC_C         = 11,
  FTM_GSM_BER_C      = 13,
  FTM_AUDIO_C        = 14,
  FTM_CAMERA_C       = 15,
  FTM_WCDMA_BER_C    = 16,
  FTM_GSM_EXTENDED_C = 17,
  FTM_1X_CAL_V2_C    = 18,
  FTM_MF_C           = 19,
  FTM_COMMON_C       = 20,
  FTM_WCDMA_RX_2_C   = 21,
  FTM_WLAN_C         = 22,
  FTM_DSPDIAG_C      = 23,
  FTM_QFUSE_C        = 24,
  FTM_MBP_C          = 25,
  FTM_MF_NS_C        = 26,
  FTM_SE_BER_C       = 27,
  FTM_FM_C           = 28,
  FTM_MODE_FACTORY_C = 0x8000,
  FTM_RESERVED_C     = 0x8001,
  FTM_MODE_MAX_C
};


#endif /* FTM_AUDIO_DIAG_DISPATCH_H */

