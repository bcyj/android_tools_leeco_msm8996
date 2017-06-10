#ifndef _DSI_CONFIG_H_
#define _DSI_CONFIG_H_
/*!
  @file
  dsi_config.h

  @brief
  This file provides configuration required by DSI module. It uses configdb
  library and XML configuration file ("dsi_config.xml").
*/

/*===========================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/21/13   hm      Initial module
===========================================================================*/
#ifdef __cplusplus
extern "C" {
#endif

#include "qmi_platform_config.h"
#include "dsi_netctrl_platform.h"

/*===========================================================================
                               MACRO DEFINITIONS
===========================================================================*/

#define DSI_CFGDB_STRVAL_MAX 100

/* Android properties definitions */
#define DSI_PROPERTY_PERSIST_DATA_TARGET       "persist.data.target"
#define DSI_PERSIST_DATA_TARGET_FUSION4_5_PCIE "fusion4_5_pcie"
#define DSI_PERSIST_DATA_TARGET_FUSION4_5_HSIC "fusion4_5_hsic"

/* Macros for QOS configuration */
#define DSI_PROPERTY_QOS                       "persist.data.netmgrd.qos.enable"
#define DSI_PROPERTY_QOS_SIZE                  (5)
#define DSI_QOS_VALUE_ENABLE                   "true"

#define DSI_PROPERTY_RMNET_DATA                "persist.rmnet.data.enable"
#define DSI_PROPERTY_RMNET_DATA_SIZE           (5)
#define DSI_PROPERTY_RMNET_DATA_DEFAULT        DSI_FALSE

#define DSI_RMNET_DATA_ENABLE_VALUE            DSI_QOS_VALUE_ENABLE

#ifdef FEATURE_DS_LINUX_ANDROID
#define DSI_CONFIG_FILE_PATH "/system/etc/data/dsi_config.xml"
#else
#define DSI_CONFIG_FILE_PATH "/etc/data/dsi_config.xml"
#endif /* FEATURE_DS_LINUX_ANDROID */

#define DSI_MAX_IFACES 16
#define DSI_QMI_PORT_MAX_LEN 12
#define DSI_DEV_STR_MAX_LEN  16

#define DSI_NUM_ENTRIES(arr) (sizeof(arr)/sizeof(arr[0]))

#define DSI_PHYS_NET_DEV_RMNET0     "rmnet0"
#define DSI_PHYS_NET_DEV_RMNET_IPA0 "rmnet_ipa0"
#define DSI_PHYS_NET_DEV_RMNET_MHI0 "rmnet_mhi0"
#define DSI_PHYS_NET_DEV_RMNET_BAM0 DSI_PHYS_NET_DEV_RMNET0

/*===========================================================================
                               DATA DEFINITIONS
===========================================================================*/
extern char * dsi_qmi_port_names[DSI_MAX_IFACES];
extern char * dsi_device_names[DSI_MAX_IFACES];

/* Struct for dsi global configuration */
struct dsi_config_s {
  int   qos_enable;
  int   rmnet_data_enable;
  char  phys_net_dev[DSI_DEV_STR_MAX_LEN];
  int   single_qmux_ch_enabled;
  char  single_qmux_ch_name[DSI_QMI_PORT_MAX_LEN];
  int   num_dsi_handles;
  int   modem_ssr_state;
};

extern struct dsi_config_s dsi_config;

/*===========================================================================
  FUNCTION:  dsi_config_load
===========================================================================*/
/*!
    @brief
    This function loads the configuration based on the config file and
    "target" property specified.

    @param[in] xml_file: XML file location string.
    @param[in] target: configuration to be used within the XML file.

    @return  0 if configuration file is loaded properly.
    @return -1 if configuration file file load/parase fails.
*/
/*=========================================================================*/
int dsi_config_load
(
  const char* xml_file,
  const char* target
);

/*===========================================================================
  FUNCTION:  dsi_config_print
===========================================================================*/
/*!
    @brief
    This function prints the configuration currently loaded for DSI.

    @param None.
    @return  None.
*/
/*=========================================================================*/
void dsi_config_print();


#ifdef __cplusplus
}
#endif

#endif /* _DSI_CONFIG_H_ */
