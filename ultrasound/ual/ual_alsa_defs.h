#ifndef __UAL_ALSA_DEFS_H__
#define __UAL_ALSA_DEFS_H__

/*============================================================================
                           ual_alsa_defs.h

DESCRIPTION:  Some definitions, related to interface with ALSA

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include <linux/types.h>

/*-----------------------------------------------------------------------------
  Consts and macros
-----------------------------------------------------------------------------*/
// Number of PCM indexes and sound cards,required for interface with ALSA:PCM
const uint16_t PCM_INDS_NUM = 4;
// Name of file, consisting the PCM indexes
const char *UAL_PCM_INDS_NAME = "/data/usf/pcm_inds.txt";
// Max size of PCM device name
const uint16_t MAX_PCM_DEV_NAME_SIZE = 20;
// Prefix of the PCM device name for RX
const char *UAL_RX_PCM_DEVICE_PREF = "hw:";
// Prefix of the PCM device name for TX
const char *UAL_TX_PCM_DEVICE_PREF = "hw:";

#endif // __UAL_ALSA_DEFS_H__
