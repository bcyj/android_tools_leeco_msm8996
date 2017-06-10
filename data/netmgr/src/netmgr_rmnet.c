/******************************************************************************

                          N E T M G R _ R M N E T . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_rmnet.h
  @brief   Network Manager RmNet Data configuration

  DESCRIPTION
  Network Manager RmNet Data configuration

******************************************************************************/
/*===========================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include <stdlib.h>
#include <stdint.h>
#include <linux/rmnet_data.h>       /* RmNet Kernel Constants */
#include <linux/msm_rmnet.h>        /* RmNet Kernel Constants */
#include "netmgr_defs.h"
#include "netmgr_platform.h"
#include "netmgr_util.h"
#include "netmgr_qmi_wda.h"
#include <netmgr_rmnet.h>     /* NetMgr RmNet Constants */
#include "librmnetctl.h"         /* RmNet Configuration Library */
#include <cutils/properties.h>

#define NETMGR_PHYS_TXPORT_INIT_RETRIES 120

uint32_t netmgr_rmnet_data_device_qmi_offset;

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

static int
_netmgr_rmnet_create_muxed_channel
(
  rmnetctl_hndl_t *hndl,
  const char     *phys_dev,
  unsigned int   dev_id,
  unsigned int   mux_id,
  const char     *dev_prefix
)
{
  int rc;
  uint16_t status_code;
  char dev_name[16];

  NETMGR_LOG_FUNC_ENTRY;

  do
  {
    if (dev_prefix)
    {
      rc = rmnet_new_vnd_prefix(hndl,
                                dev_id,
                                &status_code,
                                RMNETCTL_NEW_VND,
                                dev_prefix);
    }
    else
    {
      rc = rmnet_new_vnd(hndl, dev_id, &status_code, RMNETCTL_NEW_VND);
    }
    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to create virtual device in slot %d [rc:%d][status_code:%d]",
                       __func__, dev_id, rc, status_code);
      break;
    }
    rc = rmnet_get_vnd_name(hndl,
                            dev_id,
                            &status_code,
                            dev_name,
                            sizeof(dev_name));
    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed get name of  device in slot %d [rc:%d][status_code:%d]",
                       __func__, dev_id, rc, status_code);
      break;
    }
    rc = rmnet_set_logical_ep_config(hndl,
                                     (int32_t)mux_id,
                                     RMNET_EPMODE_VND,
                                     phys_dev,
                                     (const char *)&dev_name,
                                     &status_code);
    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to configure forward flow for rmnet_data%d [rc:%d][status_code:%d]",
                       __func__, mux_id, rc, status_code);
      break;
    }
    rc = rmnet_set_logical_ep_config(hndl,
                                     (int32_t)mux_id,
                                     RMNET_EPMODE_VND,
                                     (const char *)&dev_name,
                                     phys_dev,
                                     &status_code);
    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to configure reverse flow for rmnet_data%d [rc:%d][status_code:%d]",
                       __func__, mux_id, rc, status_code);
      break;
    }
      netmgr_log_low("%s(): Created new virtual device %s in slot %d attached to %s:%d",
                     __func__, dev_name, dev_id, phys_dev, mux_id);
  } while (0);

  NETMGR_LOG_FUNC_EXIT;
  if (!rc)
    return NETMGR_RMNET_SUCCESS;
  else
    return NETMGR_RMNET_LIB_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_set_feature_flags
===========================================================================*/
/*!
@brief
  Set specific feature flags for ingress and egress

@arg *phys_dev physical device corresponding to a transport to modem
@arg ingress_flags ingress data format bitmask for physical device
@arg egress_flags  egress data format bitmask for physical device
@arg tail_spacing  tail spacing for the packet on the physical device

@return
  int - NETMGR_RMNET_SUCCESS if successful
        NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
        NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_set_feature_flags
(
  const char        *phys_dev,
  unsigned int      ingress_flags,
  unsigned int      egress_flags,
  uint32_t          tail_spacing
)
{
  rmnetctl_hndl_t *rmnet_cfg_handle;
  uint16_t status_code;
  int rc, result, vdev_loop;
  char dev_name[16];

  NETMGR_LOG_FUNC_ENTRY;

  result = NETMGR_RMNET_SUCCESS;
  if (!phys_dev)
  {
    netmgr_log_err("%s(): Internal error: dev cannot be null!\n", __func__);
    return NETMGR_RMNET_BAD_ARGS;
  }

  if ((rc = rmnetctl_init(&rmnet_cfg_handle, &status_code) != RMNETCTL_SUCCESS))
  {
    netmgr_log_err("%s(): Init config handle failed [rc:%d][status_code:%d]\n",
                   __func__, rc, status_code);
    return NETMGR_RMNET_LIB_FAILURE;
  }

  if (ingress_flags != (unsigned int)NETMGR_RMNET_NO_FEATURE_FLAGS)
  {
    rc = rmnet_set_link_ingress_data_format_tailspace(rmnet_cfg_handle,
                                                      ingress_flags,
                                                      (uint8_t)tail_spacing,
                                                      phys_dev,
                                                      &status_code);
    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to set ingress format %d on %s [rc:%d][status_code:%d]\n",
                     __func__, ingress_flags, phys_dev, rc, status_code);
      result = NETMGR_RMNET_LIB_FAILURE;
      goto cleanup;
    }
    netmgr_log_low("%s(): Set ingress format %d on %s\n",
                   __func__, ingress_flags, phys_dev);
  }

  if (egress_flags != (unsigned int)NETMGR_RMNET_NO_FEATURE_FLAGS)
  {
    rc = rmnet_set_link_egress_data_format(rmnet_cfg_handle,
                                           egress_flags,
                                           0,
                                           0,
                                           phys_dev,
                                           &status_code);
    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to set egress format %d on %s [rc:%d][status_code:%d]\n",
                     __func__, egress_flags, phys_dev, rc, status_code);
      result = NETMGR_RMNET_LIB_FAILURE;
      goto cleanup;
    }
  }

cleanup:
  rmnetctl_cleanup(rmnet_cfg_handle);

  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_embedded
===========================================================================*/
/*!
@brief
  Create RmNet Data logical channels

@arg *phys_dev physical device corresponding to a transport to modem
@arg base_rmnet initial virtual RmNet device number
@arg base_mux_id initial mux id to use when tying virtual devices to physical
@arg num_virt_dev Number of virtual devices to create. These typicall correspond
     to logical Mux channels
@arg dev_prefix virtual network device name prefix

@return
  int - NETMGR_RMNET_SUCCESS if successful
        NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
        NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_create_muxed_channels
(
  const char        *phys_dev,
  unsigned int      base_rmnet,
  unsigned int      base_mux_id,
  unsigned char     num_virt_dev,
  const char        *dev_prefix
)
{
  int rc;
  unsigned int vdev_loop;
  uint16_t status_code;
  rmnetctl_hndl_t *hndl;

  NETMGR_LOG_FUNC_ENTRY;

  /* Input validation */

  if (!phys_dev)
  {
    netmgr_log_err("%s(): Internal error: dev cannot be null!\n", __func__);
    return NETMGR_RMNET_BAD_ARGS;
  }

  if ((0 == num_virt_dev) || ((base_rmnet + num_virt_dev) > NETMGR_MAX_LINK))
  {
    netmgr_log_err("%s(): Bad number of virtual devices: %d+%d\n",
                   __func__, base_rmnet, num_virt_dev);
    return NETMGR_RMNET_BAD_ARGS;
  }

  /* Initialize RmNet config handle */

  if ((rc = rmnetctl_init(&hndl, &status_code) != RMNETCTL_SUCCESS))
  {
    netmgr_log_err("%s(): Init config handle failed [rc:%d][status_code:%d]\n",
                   __func__, rc, status_code);
    return NETMGR_RMNET_LIB_FAILURE;
  }


  /* Create Virtual Devs configuration */
  for (vdev_loop = 0; vdev_loop < num_virt_dev; vdev_loop ++)
  {
    _netmgr_rmnet_create_muxed_channel(hndl,
                                       phys_dev,
                                       base_rmnet + vdev_loop,
                                       base_mux_id + vdev_loop,
                                       dev_prefix);
  }

  rmnetctl_cleanup(hndl);

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_RMNET_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_phys_port
===========================================================================*/
/*!
@brief
  Configure RmNet Data kernel module to associate with physical device

@arg *phys_dev physical device corresponding to a transport to modems
@arg ingress_flags ingress data format bitmask for physical device
@arg egress_flags  egress data format bitmask for physical device

@return
  int - NETMGR_RMNET_SUCCESS if successful
        NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
        NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_configure_phys_port
(
  const char        *phys_dev,
  unsigned int      ingress_flags,
  unsigned int      egress_flags
)
{
  rmnetctl_hndl_t *rmnet_cfg_handle;
  uint16_t status_code;
  int rc, result;

  NETMGR_LOG_FUNC_ENTRY;
  result = NETMGR_RMNET_SUCCESS;
  if (!phys_dev)
  {
    netmgr_log_err("%s(): Internal error: dev cannot be null!\n", __func__);
    return NETMGR_RMNET_BAD_ARGS;
  }

  if ((rc = rmnetctl_init(&rmnet_cfg_handle, &status_code) != RMNETCTL_SUCCESS))
  {
    netmgr_log_err("%s(): Init config handle failed [rc:%d][status_code:%d]\n",
                   __func__, rc, status_code);
    return NETMGR_RMNET_LIB_FAILURE;
  }

  rc = rmnet_associate_network_device(rmnet_cfg_handle,
                                      phys_dev,
                                      &status_code,
                                      RMNETCTL_DEVICE_ASSOCIATE);
  if (RMNETCTL_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to associate %s [rc:%d][status_code:%d]\n",
                   __func__, phys_dev, rc, status_code);
    result = NETMGR_RMNET_LIB_FAILURE;
    goto cleanup;
  }
  netmgr_log_low("%s(): Associated device: %s\n", __func__, phys_dev);

  rc = rmnet_set_link_ingress_data_format(rmnet_cfg_handle,
                                          ingress_flags,
                                          phys_dev,
                                          &status_code);
  if (RMNETCTL_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to set ingress format %d on %s [rc:%d][status_code:%d]\n",
                   __func__, ingress_flags, phys_dev, rc, status_code);
    result = NETMGR_RMNET_LIB_FAILURE;
    goto cleanup;
  }
  netmgr_log_low("%s(): Set ingress format %d on %s\n",
                 __func__, ingress_flags, phys_dev);

  rc = rmnet_set_link_egress_data_format(rmnet_cfg_handle,
                                         egress_flags,
                                         0,
                                         0,
                                         phys_dev,
                                         &status_code);
  if (RMNETCTL_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to set egress format %d on %s [rc:%d][status_code:%d]\n",
                   __func__, egress_flags, phys_dev, rc, status_code);
    result = NETMGR_RMNET_LIB_FAILURE;
    goto cleanup;
  }
  netmgr_log_low("%s(): Set egress format %d on %s\n",
                 __func__, egress_flags, phys_dev);

cleanup:
  rmnetctl_cleanup(rmnet_cfg_handle);

  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_query_vnd_name
===========================================================================*/
/*!
@brief
  Query RmNet Data kernel module for the name of a VND

@arg dev_id numerical ID for VND
@arg name buffer to store VND name
@arg name_len length of name buffer in bytes

@return
  int - NETMGR_RMNET_SUCCESS if successful
        NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
        NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_query_vnd_name
(
  uint32_t  dev_id,
  char     *name,
  uint32_t  name_len
)
{
  rmnetctl_hndl_t *rmnet_cfg_handle;
  uint16_t status_code;
  int rc, rrc;

  NETMGR_LOG_FUNC_ENTRY;

  rrc = NETMGR_RMNET_SUCCESS;

  if (!name)
  {
    netmgr_log_err("%s(): Internal error: name cannot be null!\n", __func__);
    return NETMGR_RMNET_BAD_ARGS;
  }

  if ((rc = rmnetctl_init(&rmnet_cfg_handle, &status_code) != RMNETCTL_SUCCESS))
  {
    netmgr_log_err("%s(): Init config handle failed [rc:%d][status_code:%d]\n",
                   __func__, rc, status_code);
    return NETMGR_RMNET_LIB_FAILURE;
  }

  rc = rmnet_get_vnd_name(rmnet_cfg_handle, dev_id, &status_code, name, name_len);
  if (RMNETCTL_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed get name of vnd in slot %d [rc:%d][status_code:%d]\n",
                     __func__, dev_id, rc, status_code);
    rrc = NETMGR_RMNET_LIB_FAILURE;
  }

  rmnetctl_cleanup(rmnet_cfg_handle);

  NETMGR_LOG_FUNC_EXIT;
  return rrc;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_convert_rmnet_edf_to_ioctl_edf
===========================================================================*/
/*!
@brief
  Convert RmNet Data format flags to IOCTL format flags (egress)
*/
/*=========================================================================*/
uint32_t
netmgr_rmnet_convert_rmnet_edf_to_ioctl_edf
(
  uint32_t flags
)
{
  uint32_t flags_ioctl = 0;
  if (flags & RMNET_EGRESS_FORMAT_MAP)
    flags_ioctl |= RMNET_IOCTL_EGRESS_FORMAT_MAP;
  if (flags & RMNET_EGRESS_FORMAT_AGGREGATION)
    flags_ioctl |= RMNET_IOCTL_EGRESS_FORMAT_AGGREGATION;
  if (flags & RMNET_EGRESS_FORMAT_MUXING)
    flags_ioctl |= RMNET_IOCTL_EGRESS_FORMAT_MUXING ;
  return flags_ioctl;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_convert_rmnet_idf_to_ioctl_idf
===========================================================================*/
/*!
@brief
  Convert RmNet Data format flags to IOCTL format flags (ingress)
*/
/*=========================================================================*/
uint32_t
netmgr_rmnet_convert_rmnet_idf_to_ioctl_idf
(
  uint32_t flags
)
{
  uint32_t flags_ioctl = 0;
  if (flags & RMNET_INGRESS_FORMAT_MAP)
    flags_ioctl |= RMNET_IOCTL_INGRESS_FORMAT_MAP;
  if (flags & RMNET_INGRESS_FORMAT_DEAGGREGATION)
    flags_ioctl |= RMNET_IOCTL_INGRESS_FORMAT_DEAGGREGATION;
  if (flags & RMNET_INGRESS_FORMAT_DEMUXING)
    flags_ioctl |= RMNET_IOCTL_INGRESS_FORMAT_DEMUXING;
  if (flags & RMNET_INGRESS_FORMAT_MAP_CKSUMV3)
    flags_ioctl |= RMNET_IOCTL_INGRESS_FORMAT_CHECKSUM;
  return flags_ioctl;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_add_del_flow
===========================================================================*/
/*!
@brief
  Add / Delete a modem flow handle - tc flow handle mapping in the kernel.

@arg link id of the virtual network device to create the link
@arg mdm_flow_hndl modem flow id
@arg tc_flow_hndl tc flow id
@arg set_flow option to add or delete a flow pair mapping

@return
  int - NETMGR_SUCCESS if successful
        NETMGR_FAILURE if the operation fails
*/
/*=========================================================================*/
int
netmgr_rmnet_add_del_flow
(
  int       link,
  uint32_t  mdm_flow_hndl,
  uint32_t  tc_flow_hndl,
  uint8_t   set_flow
)
{
  int rc;
  int result = NETMGR_FAILURE;
  rmnetctl_hndl_t *rmnet_cfg_handle;
  uint16_t status_code;

  NETMGR_LOG_FUNC_ENTRY;

  /* Initialize RmNet config handle */

  if ((rc = rmnetctl_init(&rmnet_cfg_handle, &status_code) != RMNETCTL_SUCCESS))
  {
    netmgr_log_err("%s(): Init config handle failed [rc:%d][status_code:%d]\n",
                    __func__, rc, status_code);
    goto error;
  }

  /* Add / Delete Flow handle */

  if ((rc = rmnet_add_del_vnd_tc_flow(rmnet_cfg_handle,
                                      (uint32_t)link,
                                      mdm_flow_hndl,
                                      tc_flow_hndl,
                                      set_flow,
                                      &status_code) != RMNETCTL_SUCCESS))
  {
    if (set_flow == RMNETCTL_ADD_FLOW)
    {
      netmgr_log_err("%s(): Adding flow failed [rc:%d][status_code:%d]\n",
                       __func__, rc, status_code);
    }
    else
    {
      netmgr_log_err("%s(): Deleting flow failed [rc:%d][status_code:%d]\n",
                       __func__, rc, status_code);
    }
    goto error;
  }

  result = NETMGR_SUCCESS;
  if (set_flow == RMNETCTL_ADD_FLOW)
  {
    netmgr_log_low("%s(): %s in link = %d, MAP Flow Handle = %d, TC Flow Handle = %d",
                   __func__,
                   "Added flow in kernel",
                   link,
                   mdm_flow_hndl,
                   tc_flow_hndl);
  }
  else
  {
    netmgr_log_low("%s(): %s in link = %d, MAP Flow Handle = %d, TC Flow Handle = %d",
                     __func__,
                   "Deleted flow in kernel",
                   link,
                   mdm_flow_hndl,
                   tc_flow_hndl);
  }

error:
  rmnetctl_cleanup(rmnet_cfg_handle);
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_get_ingress_data_format
===========================================================================*/
/*!
@brief
  Gets the ingress data format for a particular device.

@arg link id of the virtual network device to create the link
@arg flow_id flow id of the modem

@return
  int - NETMGR_SUCCESS if successful
        NETMGR_FAILURE if the operation fails
*/
/*=========================================================================*/
int
netmgr_rmnet_get_ingress_data_format
(
  const char        *phys_dev,
  uint32_t          *ingress_flags
)
{
  int rc;
  int result = NETMGR_FAILURE;
  uint16_t status_code;
  rmnetctl_hndl_t *rmnet_cfg_handle = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if (!phys_dev || !ingress_flags)
  {
    netmgr_log_err("%s(): Arguments cannot be NULL!\n", __func__);
    goto error;
  }

  /* Initialize RmNet config handle */

  if ((rc = rmnetctl_init(&rmnet_cfg_handle, &status_code) != RMNETCTL_SUCCESS))
  {
    netmgr_log_err("%s(): Init config handle failed [rc:%d][status_code:%d]\n",
                   __func__, rc, status_code);
    goto error;
  }

  /* Get the ingress flags */

  if ((rc = rmnet_get_link_ingress_data_format(rmnet_cfg_handle,
                                               phys_dev,
                                               ingress_flags,
                                               &status_code) != RMNETCTL_SUCCESS))
  {
    netmgr_log_err("%s(): Failed to get the flags for deivce %s [rc:%d][status_code:%d]\n",
                   __func__, phys_dev, rc, status_code);
    goto error;
  }

  result = NETMGR_SUCCESS;

error:
  rmnetctl_cleanup(rmnet_cfg_handle);
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_get_device_qmi_offset
===========================================================================*/
/*!
@brief
  Gets the netmgr_rmnet_data_device_qmi_offset.

@arg qmi_offset qmi offset of the rmnet data device

@return
  int - NETMGR_SUCCESS if successful
        NETMGR_FAILURE if the operation fails
*/
/*=========================================================================*/
int
netmgr_rmnet_get_device_qmi_offset
(
  uint32_t          *qmi_offset
)
{
  int result = NETMGR_FAILURE;
  NETMGR_LOG_FUNC_ENTRY;

  if (!qmi_offset)
  {
    netmgr_log_err("%s(): Arguments cannot be NULL!\n", __func__);
    goto error;
  }

  *qmi_offset = netmgr_rmnet_data_device_qmi_offset;
  result = NETMGR_SUCCESS;
error:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_set_device_qmi_offset
===========================================================================*/
/*!
@brief
  Sets the netmgr_rmnet_data_device_qmi_offset.

@arg qmi_offset qmi offset to be set for the rmnet data device

@return
  void
*/
/*=========================================================================*/
void
netmgr_rmnet_set_device_qmi_offset
(
  uint32_t          qmi_offset
)
{
  netmgr_rmnet_data_device_qmi_offset = qmi_offset;
}


/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_ep_parameters
===========================================================================*/
/*!
@brief
  Configure ep parameters for IPA based targets

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_configure_ep_params()
{
  int i;

  /* This configuration is specific for IPA transport */
  if(0 != strncmp(NETMGR_PHYS_NET_DEV_RMNET_IPA, netmgr_main_cfg.phys_net_dev,
             sizeof(NETMGR_PHYS_NET_DEV_RMNET_IPA)))
  {
    /* Bail for Non IPA targets */
    netmgr_log_med("%s: Not a IPA target. Ignore ep configuration", __func__);
    return NETMGR_SUCCESS;
  }

  for (i = 0; i < NETMGR_PHYS_TXPORT_INIT_RETRIES; i++)
  {
    if (NETMGR_SUCCESS !=
        netmgr_kif_configure_ep_params(netmgr_main_cfg.phys_net_dev))
    {
      netmgr_log_err("%s: Failed to configure ep parameters for [%s]. Sleeping 1s before retry",
                     __func__, netmgr_main_cfg.phys_net_dev);
      sleep(1);
    }
    else
    {
      netmgr_log_med("%s: ep config completed successfully", __func__);
      break;
    }
  }

  if(i == NETMGR_PHYS_TXPORT_INIT_RETRIES)
  {
    netmgr_log_err("%s: unable to configure ep_id for transport [%s]",
                   __func__, netmgr_main_cfg.phys_net_dev);
  }
  return NETMGR_SUCCESS;
}



/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_embedded_data
===========================================================================*/
/*!
@brief


@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_configure_embedded_data
(
  void
)
{
  unsigned int mux_offset;
  char platform_name[PROPERTY_VALUE_MAX];
  int i, rc;
  uint32_t ingress_flags, egress_flags;
  netmgr_data_format_t *data_format = &netmgr_main_cfg.data_format;

  for (i = 0; i < NETMGR_PHYS_TXPORT_INIT_RETRIES; i++)
  {
    if (NETMGR_SUCCESS !=
        netmgr_kif_init_physical_transport(netmgr_main_cfg.phys_net_dev, data_format))
    {
      netmgr_log_err("%s() Failed to init physical transport %s. Sleeping 1s before retry",
                     __func__, netmgr_main_cfg.phys_net_dev);
      sleep(1);
    }
    else
    {
      netmgr_log_med("%s() Successfully init physical transport %s",
                     __func__, netmgr_main_cfg.phys_net_dev);
      break;
    }
  }

  mux_offset = NETMGR_RMNET_START;
  if (data_format->dl_data_aggregation_protocol == NETMGR_WDA_DL_QMAPV3)
  {
    ingress_flags = RMNET_INGRESS_FORMAT_MAP | RMNET_INGRESS_FORMAT_DEMUXING | RMNET_INGRESS_FORMAT_MAP_CKSUMV3;
  }
  else
  {
    ingress_flags = RMNET_INGRESS_FORMAT_MAP | RMNET_INGRESS_FORMAT_DEMUXING;
  }
  egress_flags =  RMNET_EGRESS_FORMAT_MAP  | RMNET_EGRESS_FORMAT_MUXING;
  if (netmgr_main_get_ibfc_enabled())
  {
    ingress_flags = ingress_flags | RMNET_INGRESS_FORMAT_MAP_COMMANDS;
  }

  rc= netmgr_rmnet_configure_phys_port (netmgr_main_cfg.phys_net_dev,
                                        ingress_flags,
                                        egress_flags);

  if (NETMGR_RMNET_SUCCESS != rc)
    return NETMGR_SUCCESS;

  rc = netmgr_rmnet_create_muxed_channels (netmgr_main_cfg.phys_net_dev,
                                           mux_offset,
                                           mux_offset + 1,
                                           (unsigned char)data_format->num_mux_channels,
                                           NULL);
  if (NETMGR_RMNET_SUCCESS != rc)
    return NETMGR_SUCCESS;

  netmgr_kif_optional_physical_transport_ioctls(netmgr_main_cfg.phys_net_dev,
                                                mux_offset,
                                                mux_offset + 1,
                                                (uint32_t)data_format->num_mux_channels,
                                                ingress_flags,
                                                egress_flags);
#ifdef FEATURE_DATA_IWLAN
  /* Add the reverse RmNet ports with mux IDs starting where forward RmNet ports
   * end */
  mux_offset = NETMGR_RMNET_START + (unsigned int)data_format->num_mux_channels;
  netmgr_rmnet_create_muxed_channels (netmgr_main_cfg.phys_net_dev,
                                      mux_offset,
                                      mux_offset + 1,
                                      (unsigned char)data_format->num_iwlan_channels,
                                      NETMGR_MAIN_REV_RMNET_DATA_PREFIX);
#endif /* FEATURE_DATA_IWLAN */
  return NETMGR_SUCCESS;
}

int
netmgr_rmnet_remove_all_configuration
(
  void
)
{
  char dev_name[16];
  rmnetctl_hndl_t *rmnet_cfg_handle = NULL;
  const char *phys_dev = netmgr_main_cfg.phys_net_dev;
  netmgr_data_format_t *data_format = &netmgr_main_cfg.data_format;
  uint32_t i;
  int rc;
  uint16_t status_code;
  NETMGR_LOG_FUNC_ENTRY;

  /* Initialize RmNet config handle */
  netmgr_log_err("%s(): Triggering reset configs on %s", __func__, phys_dev);

  if ((rc = rmnetctl_init(&rmnet_cfg_handle, &status_code) != RMNETCTL_SUCCESS))
  {
    netmgr_log_err("%s(): Init config handle failed [rc:%d][status_code:%d]\n",
                   __func__, rc, status_code);
    goto error;
  }

  for (i = 0; i < (unsigned int)(data_format->num_mux_channels + data_format->num_iwlan_channels); i++)
  {
    memset(dev_name, 0x0, sizeof(dev_name));
    rc = rmnet_get_vnd_name(rmnet_cfg_handle, i, &status_code, dev_name, sizeof(dev_name));
    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed get name of  device in slot %d [rc:%d][status_code:%d]",
                       __func__, i, rc, status_code);
      continue;
    }

    netmgr_log_err("%s(): Cleaning up [%d] %s", __func__, i, dev_name);

    rc = rmnet_unset_logical_ep_config(rmnet_cfg_handle, (int32_t)(i+1), phys_dev, &status_code);

    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to unset ingress lepc [rc:%d][status_code:%d]",
                       __func__, rc, status_code);
    }
    rc = rmnet_unset_logical_ep_config(rmnet_cfg_handle, -1, dev_name, &status_code);

    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to unset egress lepc [rc:%d][status_code:%d]",
                       __func__, rc, status_code);
    }

    rc = rmnet_new_vnd(rmnet_cfg_handle, i, &status_code,  RMNETCTL_FREE_VND);
    if (RMNETCTL_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to free vnd %d [rc:%d][status_code:%d]",
                       __func__, i, rc, status_code);
      continue;
    }
  }
  netmgr_log_err("%s(): Unassoc %s", __func__, phys_dev);
  rc = rmnet_associate_network_device(rmnet_cfg_handle,
                                      phys_dev,
                                      &status_code,
                                      RMNETCTL_DEVICE_UNASSOCIATE);

  if (RMNETCTL_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to unassociate dev %s [rc:%d][status_code:%d]",
                       __func__, phys_dev, rc, status_code);
    /* Fall through for cleanup and error handling */
  }
  rmnetctl_cleanup(rmnet_cfg_handle);

error:
  NETMGR_LOG_FUNC_EXIT;

  return (RMNETCTL_SUCCESS == rc ? NETMGR_SUCCESS : NETMGR_FAILURE);
}
