
/******************************************************************************
  @file    qmi_platform_config.h
  @brief   Platform-specific external QMI definitions.

  DESCRIPTION
  This file contains platform specific configuration definitions
  for QMI interface library.


  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  $Header: $
  $DateTime: $
  ---------------------------------------------------------------------------
  Copyright (c) 2007 - 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef QMI_PLATFORM_CONFIG_H
#define QMI_PLATFORM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum length of QMI port name (including null terminator) */
#define QMI_DEVICE_NAME_SIZE (100)

/* smd */
#define QMI_PORT_RMNET_0  "rmnet0"
#define QMI_PORT_RMNET_1  "rmnet1"
#define QMI_PORT_RMNET_2  "rmnet2"
#define QMI_PORT_RMNET_3  "rmnet3"
#define QMI_PORT_RMNET_4  "rmnet4"
#define QMI_PORT_RMNET_5  "rmnet5"
#define QMI_PORT_RMNET_6  "rmnet6"
#define QMI_PORT_RMNET_7  "rmnet7"
#define QMI_PORT_RMNET_8  "rmnet8"
#define QMI_PORT_RMNET_9  "rmnet9"
#define QMI_PORT_RMNET_10 "rmnet10"
#define QMI_PORT_RMNET_11 "rmnet11"

/* reverse rmnet ports */
#define QMI_PORT_REV_RMNET_0  "rev_rmnet0"
#define QMI_PORT_REV_RMNET_1  "rev_rmnet1"
#define QMI_PORT_REV_RMNET_2  "rev_rmnet2"
#define QMI_PORT_REV_RMNET_3  "rev_rmnet3"
#define QMI_PORT_REV_RMNET_4  "rev_rmnet4"
#define QMI_PORT_REV_RMNET_5  "rev_rmnet5"
#define QMI_PORT_REV_RMNET_6  "rev_rmnet6"
#define QMI_PORT_REV_RMNET_7  "rev_rmnet7"
#define QMI_PORT_REV_RMNET_8  "rev_rmnet8"

/* sdio */
#define QMI_PORT_RMNET_SDIO_0 "rmnet_sdio0"
#define QMI_PORT_RMNET_SDIO_1 "rmnet_sdio1"
#define QMI_PORT_RMNET_SDIO_2 "rmnet_sdio2"
#define QMI_PORT_RMNET_SDIO_3 "rmnet_sdio3"
#define QMI_PORT_RMNET_SDIO_4 "rmnet_sdio4"
#define QMI_PORT_RMNET_SDIO_5 "rmnet_sdio5"
#define QMI_PORT_RMNET_SDIO_6 "rmnet_sdio6"
#define QMI_PORT_RMNET_SDIO_7 "rmnet_sdio7"

/* usb */
#define QMI_PORT_RMNET_USB_0 "rmnet_usb0"
#define QMI_PORT_RMNET_USB_1 "rmnet_usb1"
#define QMI_PORT_RMNET_USB_2 "rmnet_usb2"
#define QMI_PORT_RMNET_USB_3 "rmnet_usb3"
#define QMI_PORT_RMNET_USB_4 "rmnet_usb4"
#define QMI_PORT_RMNET_USB_5 "rmnet_usb5"
#define QMI_PORT_RMNET_USB_6 "rmnet_usb6"
#define QMI_PORT_RMNET_USB_7 "rmnet_usb7"

/* MHI */
#define QMI_PORT_RMNET_MHI_0 "rmnet_mhi0"
#define QMI_PORT_RMNET_MHI_1 "rmnet_mhi1"

/* data */
#define QMI_PORT_RMNET_DATA_0 "rmnet_data0"
#define QMI_PORT_RMNET_DATA_1 "rmnet_data1"
#define QMI_PORT_RMNET_DATA_2 "rmnet_data2"
#define QMI_PORT_RMNET_DATA_3 "rmnet_data3"
#define QMI_PORT_RMNET_DATA_4 "rmnet_data4"
#define QMI_PORT_RMNET_DATA_5 "rmnet_data5"
#define QMI_PORT_RMNET_DATA_6 "rmnet_data6"
#define QMI_PORT_RMNET_DATA_7 "rmnet_data7"

/* reverse rmnet_usb ports */
#define QMI_PORT_REV_RMNET_USB_0 "rev_rmnet_usb0"
#define QMI_PORT_REV_RMNET_USB_1 "rev_rmnet_usb1"
#define QMI_PORT_REV_RMNET_USB_2 "rev_rmnet_usb2"
#define QMI_PORT_REV_RMNET_USB_3 "rev_rmnet_usb3"
#define QMI_PORT_REV_RMNET_USB_4 "rev_rmnet_usb4"
#define QMI_PORT_REV_RMNET_USB_5 "rev_rmnet_usb5"
#define QMI_PORT_REV_RMNET_USB_6 "rev_rmnet_usb6"
#define QMI_PORT_REV_RMNET_USB_7 "rev_rmnet_usb7"
#define QMI_PORT_REV_RMNET_USB_8 "rev_rmnet_usb8"

/* reverse rmnet_data ports*/
#define QMI_PORT_REV_RMNET_DATA_0 "r_rmnet_data0"
#define QMI_PORT_REV_RMNET_DATA_1 "r_rmnet_data1"
#define QMI_PORT_REV_RMNET_DATA_2 "r_rmnet_data2"
#define QMI_PORT_REV_RMNET_DATA_3 "r_rmnet_data3"
#define QMI_PORT_REV_RMNET_DATA_4 "r_rmnet_data4"
#define QMI_PORT_REV_RMNET_DATA_5 "r_rmnet_data5"
#define QMI_PORT_REV_RMNET_DATA_6 "r_rmnet_data6"
#define QMI_PORT_REV_RMNET_DATA_7 "r_rmnet_data7"
#define QMI_PORT_REV_RMNET_DATA_8 "r_rmnet_data8"

/* smux */
#define QMI_PORT_RMNET_SMUX_0 "rmnet_smux0"

/* second usb */
#define QMI_PORT_RMNET2_USB_0 "rmnet2_usb0"
#define QMI_PORT_RMNET2_USB_1 "rmnet2_usb1"
#define QMI_PORT_RMNET2_USB_2 "rmnet2_usb2"
#define QMI_PORT_RMNET2_USB_3 "rmnet2_usb3"
#define QMI_PORT_RMNET2_USB_4 "rmnet2_usb4"
#define QMI_PORT_RMNET2_USB_5 "rmnet2_usb5"
#define QMI_PORT_RMNET2_USB_6 "rmnet2_usb6"
#define QMI_PORT_RMNET2_USB_7 "rmnet2_usb7"

/* IPA */
#define QMI_PORT_RMNET_IPA_0  "rmnet_ipa0"

/* Virtual port */
#define QMI_PORT_PROXY        "qmi_proxy"

/* This define is used to tell QMI library which port should be used for
** internal QMI library communications with modem.
** If you want to change the default port that is used internally in QMI
** library, set the QMI_PLATFORM_INTERNAL_USE_PORT_ID #define to the string/function
** you want to use as your default internal port.  For example if I wanted that port
** to be the RMNET_2 QMI port, I would define as follows:

#define QMI_PLATFORM_INTERNAL_USE_PORT_ID QMI_PORT_RMNET_2

** By default (this value not defined), QMI_PORT_RMNET_0 is default
*/

/* This function will be defined in qmi_platform.c */
extern
const char * qmi_linux_get_internal_use_port
(
  void
);

#define QMI_PLATFORM_INTERNAL_USE_PORT_ID qmi_linux_get_internal_use_port()


#ifdef __cplusplus
}
#endif

#endif /* QMI_PLATFORM_CONFIG_H */

