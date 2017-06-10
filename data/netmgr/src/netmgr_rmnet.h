/******************************************************************************

                          N E T M G R _ R M N E T . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_rmnet.h
  @brief   Network Manager RmNet Data configuration internal header file

  DESCRIPTION
  Network Manager RmNet Data configuration internal header file

******************************************************************************/
/*===========================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

#ifndef __NETMGR_RMNET_H__
#define __NETMGR_RMNET_H__

#define NETMGR_RMNET_SUCCESS          0
#define NETMGR_RMNET_BAD_ARGS         1
#define NETMGR_RMNET_LIB_FAILURE      2

#define NETMGR_RMNET_NO_FEATURE_FLAGS   -1

#define NETMGR_RMNET_ADD_FLOW         1
#define NETMGR_RMNET_DEL_FLOW         0

/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_embedded
===========================================================================*/
/*!
@brief
  Configure RmNet Data kernel module for embedded modem

@arg *phys_dev physical device corresponding to a transport to modem
@arg base_rmnet initial virtual RmNet device number
@arg base_mux_id initial mux id to use when tying virtual devices to physical
@arg num_virt_dev Number of virtual devices to create. These typicall correspond
     to logical Mux channels
@arg ingress_flags ingress data format bitmask for physical device
@arg egress_flags  egress data format bitmask for physical device

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
);

int
netmgr_rmnet_configure_phys_port
(
  const char        *phys_dev,
  unsigned int      ingress_flags,
  unsigned int      egress_flags
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);


/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_ep_params
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
netmgr_rmnet_configure_ep_params
(
  void
);

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
);


/*===========================================================================
  FUNCTION  netmgr_rmnet_remove_all_configuration
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
netmgr_rmnet_remove_all_configuration
(
  void
);
#endif  /* __NETMGR_RMNET_H__ */

