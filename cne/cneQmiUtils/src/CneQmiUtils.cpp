
/*==============================================================================
  FILE:   Cne QMI Utils class file

              Copyright (c) 2011-2012,2014 Qualcomm Technologies, Inc.
              All Rights Reserved.
              Qualcomm Technologies Confidential and Proprietary
==============================================================================*/
/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <CneQmiUtils.h>

/**
 * Always keep this array and max elements of this array in sync with
 * qmi/platform/qmi_platform_config.h
 */
const char * qmiPorts[] = {
    QMI_PORT_RMNET_0,
    QMI_PORT_RMNET_1,
    QMI_PORT_RMNET_2,
    QMI_PORT_RMNET_3,
    QMI_PORT_RMNET_4,
    QMI_PORT_RMNET_5,
    QMI_PORT_RMNET_6,
    QMI_PORT_RMNET_7,
    QMI_PORT_RMNET_8,
    QMI_PORT_RMNET_9,
    QMI_PORT_RMNET_10,
    QMI_PORT_RMNET_11,
    QMI_PORT_RMNET_SDIO_0,
    QMI_PORT_RMNET_SDIO_1,
    QMI_PORT_RMNET_SDIO_2,
    QMI_PORT_RMNET_SDIO_3,
    QMI_PORT_RMNET_SDIO_4,
    QMI_PORT_RMNET_SDIO_5,
    QMI_PORT_RMNET_SDIO_6,
    QMI_PORT_RMNET_SDIO_7,
    QMI_PORT_RMNET_USB_0,
    QMI_PORT_RMNET_USB_1,
    QMI_PORT_RMNET_USB_2,
    QMI_PORT_RMNET_USB_3,
    QMI_PORT_RMNET_USB_4,
    QMI_PORT_RMNET_USB_5,
    QMI_PORT_RMNET_USB_6,
    QMI_PORT_RMNET_USB_7,
    QMI_PORT_RMNET_MHI_0,
    QMI_PORT_RMNET_MHI_1,
    QMI_PORT_RMNET_DATA_0,
    QMI_PORT_RMNET_DATA_1,
    QMI_PORT_RMNET_DATA_2,
    QMI_PORT_RMNET_DATA_3,
    QMI_PORT_RMNET_DATA_4,
    QMI_PORT_RMNET_DATA_5,
    QMI_PORT_RMNET_DATA_6,
    QMI_PORT_RMNET_DATA_7,
    QMI_PORT_RMNET_SMUX_0,
    QMI_PORT_RMNET2_USB_0,
    QMI_PORT_RMNET2_USB_1,
    QMI_PORT_RMNET2_USB_2,
    QMI_PORT_RMNET2_USB_3,
    QMI_PORT_RMNET2_USB_4,
    QMI_PORT_RMNET2_USB_5,
    QMI_PORT_RMNET2_USB_6,
    QMI_PORT_RMNET2_USB_7,
};
/*----------------------------------------------------------------------------
 * FUNCTION      isQmiPort

 * DESCRIPTION   verifies if the device is a valid qmi port

 * DEPENDENCIES

 * RETURN VALUE  bool true if device is a valid qmi port, false otherwise

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
bool CneQmiUtils::isQmiPort
(
  const char *device
)
{
    bool found = false;
    if (!device) return found;
    unsigned i;
    unsigned numPorts = (unsigned)( sizeof(qmiPorts)/sizeof(qmiPorts[0]));
    for (i = 0; i < numPorts; i++) {
        if (strcasecmp(device, qmiPorts[i]) == 0) {
            found = true;
            break;
        }
    }
    return found;
}
