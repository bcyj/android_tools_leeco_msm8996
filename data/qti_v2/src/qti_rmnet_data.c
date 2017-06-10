/******************************************************************************

                        QTI_RMNET_DATA.C

******************************************************************************/

/******************************************************************************

  @file    qti_rmnet_data.c
  @brief   Tethering Interface module for RmNET DATA interaction.


  DESCRIPTION
  This file has functions which interact with RmNET DATA for
  RMNET tethering.

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
01/22/14   sb         Add support for Fusion

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/rmnet_data.h>
#include <linux/msm_rmnet.h>

#include "qti.h"
#include "librmnetctl.h"


static  qti_rmnet_param        * rmnet_state_config;

/*===========================================================================
                               FUNCTION DEFINITIONS
=========================================================================*/


/*===========================================================================

FUNCTION QTI_RMNET_CALL_IOCTL_ON_DEV()

DESCRIPTION
  - calls the specified IOCTL on the specified interface

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
int qti_rmnet_call_ioctl_on_dev
(
  const char         *dev,
  unsigned int        req,
  struct ifreq       *ifr
)
{
  int fd;

  /* Open a temporary socket of datagram type to use for issuing the ioctl */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("call_ioctl_on_dev: socket open failed", 0, 0, 0);
    return QTI_FAILURE;
  }

  /* Set device name in the ioctl req struct */
  (void)strlcpy(ifr->ifr_name, dev, sizeof(ifr->ifr_name));

  /* Issue ioctl on the device */
  if (ioctl(fd, req, ifr) < 0)
  {
    LOG_MSG_ERROR("call_ioctl_on_dev: ioctl failed", 0, 0, 0);
    close(fd);
    return QTI_FAILURE;
  }

  /* Close temporary socket */
  close(fd);
  return QTI_SUCCESS;
}



/*===========================================================================

FUNCTION QTI_RMNET_DATA_INIT_BRIDGE()

DESCRIPTION
  - initializes the RmNet data driver

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int qti_rmnet_data_init_bridge()
{

  uint16_t status_code;
  struct rmnetctl_hndl_s  *rmnet_cfg_lib_handle;
  int ret_val;
  struct ifreq ifr;
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
  To handle QTI restart teardown bridge before setting it up
----------------------------------------------------------------------------*/
  qti_rmnet_data_teardown_bridge();

/*---------------------------------------------------------------------------
  Get the RmNet data driver handle
----------------------------------------------------------------------------*/

  if ((ret_val = rmnetctl_init(&rmnet_cfg_lib_handle, &status_code)) != RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to initialize RmNet data driver handle."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  Associate MHI interface to RmNet data driver
----------------------------------------------------------------------------*/
  if ((ret_val = rmnet_associate_network_device(rmnet_cfg_lib_handle,
                                                MHI_DATA_INTERFACE,
                                                &status_code,
                                                RMNETCTL_DEVICE_ASSOCIATE))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to associate MHI interface with RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    rmnetctl_cleanup(rmnet_cfg_lib_handle);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  Associate USB interface to RmNet data driver
----------------------------------------------------------------------------*/
  if (( ret_val = rmnet_associate_network_device(rmnet_cfg_lib_handle,
                                              USB_DATA_INTERFACE,
                                              &status_code,
                                              RMNETCTL_DEVICE_ASSOCIATE))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to associate USB interface with RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    rmnetctl_cleanup(rmnet_cfg_lib_handle);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  Set logical EP point config for MHI and USB interface
----------------------------------------------------------------------------*/
  if (( ret_val = rmnet_set_logical_ep_config(rmnet_cfg_lib_handle,
                                           -1,
                                           RMNET_EPMODE_BRIDGE,
                                           MHI_DATA_INTERFACE,
                                           USB_DATA_INTERFACE,
                                           &status_code))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set logical end point config for MHI and USB interface."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    rmnetctl_cleanup(rmnet_cfg_lib_handle);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  Set logical EP point config for USB and MHI interface
----------------------------------------------------------------------------*/
  if (( ret_val = rmnet_set_logical_ep_config(rmnet_cfg_lib_handle,
                                           -1,
                                           RMNET_EPMODE_BRIDGE,
                                           USB_DATA_INTERFACE,
                                           MHI_DATA_INTERFACE,
                                           &status_code))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set logical end point config for USB and MHI interface."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    rmnetctl_cleanup(rmnet_cfg_lib_handle);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  Release RmNet data driver handle
----------------------------------------------------------------------------*/
  rmnetctl_cleanup(rmnet_cfg_lib_handle);

/*---------------------------------------------------------------------------
  Set IP mode on USB interface
----------------------------------------------------------------------------*/
  if (qti_rmnet_call_ioctl_on_dev(USB_DATA_INTERFACE,
                                  RMNET_IOCTL_SET_LLP_IP,
                                  &ifr) != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set IP mode on USB interface", 0, 0, 0);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  Set IP mode on MHI interface
----------------------------------------------------------------------------*/
  if (qti_rmnet_call_ioctl_on_dev(MHI_DATA_INTERFACE,
                                  RMNET_IOCTL_SET_LLP_IP,
                                  &ifr) != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set IP mode on MHI interface", 0, 0, 0);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  bring down both USB and MHI interfaces
----------------------------------------------------------------------------*/
  ds_system_call("ifconfig rmnet_mhi0 down",
                 strlen("ifconfig rmnet_mhi0 down"));

  ds_system_call("ifconfig usb_rmnet0 down",
                 strlen("ifconfig usb_rmnet0 down"));

/*---------------------------------------------------------------------------
  Set default MTU and MRU on USB and MHI interfaces
----------------------------------------------------------------------------*/
  ret_val = qti_rmnet_modem_set_mru(DEFAULT_MTU_MRU_VALUE);

  if (ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set default MRU value on modem", 0, 0, 0);
    return QTI_FAILURE;
  }

  ret_val = qti_rmnet_modem_set_mtu(DEFAULT_MTU_MRU_VALUE);

  if (ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set default MTU value on modem", 0, 0, 0);
    return QTI_FAILURE;
  }

  ret_val = qti_rmnet_ph_set_mru(DEFAULT_MTU_MRU_VALUE);

  if (ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set default MTU value on peripheral", 0, 0, 0);
    return QTI_FAILURE;
  }

  ret_val = qti_rmnet_ph_set_mtu(DEFAULT_MTU_MRU_VALUE);

  if (ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set default MTU value on peripheral", 0, 0, 0);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  Bring up MHI and USB interfaces
----------------------------------------------------------------------------*/
  ds_system_call("ifconfig rmnet_mhi0 up",
                 strlen("ifconfig rmnet_mhi0 up"));

  ds_system_call("ifconfig usb_rmnet0 up",
                 strlen("ifconfig usb_rmnet0 up"));

  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_RMNET_DATA_TEARDOWN_BRIDGE()

DESCRIPTION
  resets RmNet data driver

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int qti_rmnet_data_teardown_bridge()
{

  uint16_t status_code;
  struct rmnetctl_hndl_s  *rmnet_cfg_lib_handle;
  int ret_val;
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
  Bring down MHI and USB interfaces
----------------------------------------------------------------------------*/
  ds_system_call("ifconfig rmnet_mhi0 down",
                 strlen("ifconfig rmnet_mhi0 down"));

  ds_system_call("ifconfig rmnet_usb0 down",
                 strlen("ifconfig rmnet_usb0 down"));

/*---------------------------------------------------------------------------
  Get RmNet data driver handle
----------------------------------------------------------------------------*/
  if ((ret_val = rmnetctl_init(&rmnet_cfg_lib_handle, &status_code)) != RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to initialize RmNet data driver handle."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  Unset logical ep config for MHI and USB interfaces from RmNet data driver
----------------------------------------------------------------------------*/
  if ((ret_val = rmnet_unset_logical_ep_config(rmnet_cfg_lib_handle,
                                               -1,
                                               MHI_DATA_INTERFACE,
                                               &status_code))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to unset logical ep config for MHI interface with RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
  }

  if ((ret_val = rmnet_unset_logical_ep_config(rmnet_cfg_lib_handle,
                                               -1,
                                               USB_DATA_INTERFACE,
                                               &status_code))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to unset logical ep config for USB interface with RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
  }

/*---------------------------------------------------------------------------
  Unassociate MHI and USB interfaces from RmNet data driver
----------------------------------------------------------------------------*/
  if ((ret_val = rmnet_associate_network_device(rmnet_cfg_lib_handle,
                                                MHI_DATA_INTERFACE,
                                                &status_code,
                                                RMNETCTL_DEVICE_UNASSOCIATE))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to unassociate MHI interface from RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
  }

  if (( ret_val = rmnet_associate_network_device(rmnet_cfg_lib_handle,
                                              USB_DATA_INTERFACE,
                                              &status_code,
                                              RMNETCTL_DEVICE_UNASSOCIATE))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to unassociate USB interface from RmNet data driver"
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
  }

/*---------------------------------------------------------------------------
  Release RmNet data driver handle
----------------------------------------------------------------------------*/
  rmnetctl_cleanup(rmnet_cfg_lib_handle);

  return QTI_SUCCESS;

}


