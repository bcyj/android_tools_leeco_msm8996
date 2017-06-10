#ifndef __USF_H__
#define __USF_H__

/*============================================================================
                           usf.h

DESCRIPTION:  Ultrasound Framework header file

Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.  Qualcomm Technologies Proprietary
Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.

==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include <linux/ioctl.h>
#include "usf_types.h"

/*-----------------------------------------------------------------------------
  Consts and macros
-----------------------------------------------------------------------------*/
#define USF_IOCTL_MAGIC 'U'

#define US_SET_TX_INFO   _IOW(USF_IOCTL_MAGIC, 0, \
                              us_tx_info_type)
#define US_START_TX      _IO(USF_IOCTL_MAGIC, 1)
#define US_GET_TX_UPDATE _IOWR(USF_IOCTL_MAGIC, 2, \
                              us_tx_update_info_type)
#define US_SET_RX_INFO   _IOW(USF_IOCTL_MAGIC, 3, \
                              us_rx_info_type)
#define US_SET_RX_UPDATE _IOWR(USF_IOCTL_MAGIC, 4, \
                               us_rx_update_info_type)
#define US_START_RX      _IO(USF_IOCTL_MAGIC, 5)

#define US_STOP_TX      _IO(USF_IOCTL_MAGIC, 6)
#define US_STOP_RX      _IO(USF_IOCTL_MAGIC, 7)

#endif //  __USF_H__

