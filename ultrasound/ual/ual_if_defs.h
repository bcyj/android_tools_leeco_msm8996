#ifndef __UAL_IF_DEFS_H__
#define __UAL_IF_DEFS_H__

/*============================================================================
                           ual_if_defs.h

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

// Triggers directory for interface between media and ultrasound frameworks
const char *TRIGGER_DIRECTORY_NAME = "/data/usf/media_if";

// Stop US RX trigger file name
const char *UAL_STOP_RX_TRIGGER_FILE_NAME = "/data/usf/media_if/usf_rx_stop";


#endif // __UAL_IF_DEFS_H__
