#ifndef __USF_H__
#define __USF_H__

/*============================================================================
                           usf.h

DESCRIPTION:  Ultrasound Framework header file

Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include <linux/types.h>
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

#define US_SET_DETECTION _IOWR(USF_IOCTL_MAGIC, 8, \
                               us_detect_info_type)

#define US_GET_VERSION  _IOWR(USF_IOCTL_MAGIC, 9, \
                               us_version_info_type)

#define US_SET_TX_STREAM_PARAM   _IOW(USF_IOCTL_MAGIC, 10, \
                               us_stream_param_type)
#define US_GET_TX_STREAM_PARAM  _IOWR(USF_IOCTL_MAGIC, 11, \
                               us_stream_param_type)
#define US_SET_RX_STREAM_PARAM   _IOW(USF_IOCTL_MAGIC, 12, \
                               us_stream_param_type)
#define US_GET_RX_STREAM_PARAM  _IOWR(USF_IOCTL_MAGIC, 13, \
                               us_stream_param_type)

#endif // __USF_H__
