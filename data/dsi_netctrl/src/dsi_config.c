/*!
  @file
  dsi_config.c

  @brief
  This file provides implementation of DSI configration using configdb.
*/

/*===========================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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

#include "dsi_config.h"
#include "configdb.h"
#include "string.h"

static char *port_names[] =
{
  QMI_PORT_RMNET_0,
  QMI_PORT_RMNET_1,
  QMI_PORT_RMNET_2,
  QMI_PORT_RMNET_3,
  QMI_PORT_RMNET_4,
  QMI_PORT_RMNET_5,
  QMI_PORT_RMNET_6,
  QMI_PORT_RMNET_7,

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

  QMI_PORT_RMNET_SMUX_0,

  QMI_PORT_RMNET2_USB_0,
  QMI_PORT_RMNET2_USB_1,
  QMI_PORT_RMNET2_USB_2,
  QMI_PORT_RMNET2_USB_3,
  QMI_PORT_RMNET2_USB_4,
  QMI_PORT_RMNET2_USB_5,
  QMI_PORT_RMNET2_USB_6,
  QMI_PORT_RMNET2_USB_7,

  QMI_PORT_RMNET_MHI_0,
  QMI_PORT_RMNET_MHI_1
};

/* Struct for global dsi configuration */
struct dsi_config_s dsi_config;

/*===========================================================================
  FUNCTION:  get_port_name
===========================================================================*/
/*!
    @brief
    This function returns pointer in the array to port name string.

    @param None.
    @return  None.
*/
/*=========================================================================*/
static char *get_port_name
(
  char *xml_port_name
)
{
  unsigned int i;
  for (i = 0; i < DSI_NUM_ENTRIES(port_names); i++)
  {
    if (!strcmp(xml_port_name, port_names[i]))
    {
      return port_names[i];
    }
  }
  return NULL;
}

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
)
{
  int result = -1;
  int32 ret;
  int i;
  char prop_name[DSI_CFGDB_STRVAL_MAX];
  char prop_val_str[DSI_CFGDB_STRVAL_MAX];
  int num_dsi_handles;
  int rmnet_data_enabled = DSI_FALSE;
  int qos_enabled = DSI_FALSE;
  int single_qmux_ch_enabled = DSI_FALSE;

  DSI_LOG_INFO("DSI: Configuring using file %s, target %s\n", xml_file, target);

  ret = configdb_init(CFGDB_OPMODE_CACHED, xml_file);
  if (CFGDB_SUCCESS != ret)
  {
    DSI_LOG_ERROR("Unable to open/parse config file [%s] Err [%d]", xml_file, ret);
    goto bail;
  }

  /* RnNet Data */
  snprintf(prop_name, DSI_CFGDB_STRVAL_MAX, "%s%s%s", "dsi_config.", target, ".rmnet_data_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &rmnet_data_enabled, sizeof(rmnet_data_enabled));
  if (ret != CFGDB_SUCCESS)
  {
    DSI_LOG_ERROR("Error reading property [%s] Err[%d]", prop_name, ret);
    goto bail;
  }
  dsi_config.rmnet_data_enable = rmnet_data_enabled;

  /* QoS */
  snprintf(prop_name, DSI_CFGDB_STRVAL_MAX, "%s%s%s", "dsi_config.", target, ".qos_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &qos_enabled, sizeof(qos_enabled));
  if (ret != CFGDB_SUCCESS)
  {
    DSI_LOG_ERROR("Error reading property [%s] Err[%d]", prop_name, ret);
    goto bail;
  }
  dsi_config.qos_enable = qos_enabled;

  /* Physical net device */
  snprintf(prop_name, DSI_CFGDB_STRVAL_MAX, "%s%s%s", "dsi_config.", target, ".phys_net_dev");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    DSI_LOG_ERROR("Error reading property [%s] Err[%d]", prop_name, ret);
    goto bail;
  }
  strlcpy(dsi_config.phys_net_dev, prop_val_str, sizeof(dsi_config.phys_net_dev));

  /* Single QMUX channel enabled */
  snprintf(prop_name, DSI_CFGDB_STRVAL_MAX, "%s%s%s", "dsi_config.", target, ".single_qmux_channel_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &single_qmux_ch_enabled, sizeof(single_qmux_ch_enabled));
  if (ret != CFGDB_SUCCESS)
  {
    DSI_LOG_ERROR("Error reading property [%s] Err[%d]", prop_name, ret);
    goto bail;
  }
  dsi_config.single_qmux_ch_enabled = single_qmux_ch_enabled;

  /* Single QMUX channel name */
  snprintf(prop_name, DSI_CFGDB_STRVAL_MAX, "%s%s%s", "dsi_config.", target, ".single_qmux_channel_name");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    DSI_LOG_ERROR("Error reading property [%s] Err[%d]", prop_name, ret);
    goto bail;
  }
  strlcpy(dsi_config.single_qmux_ch_name, prop_val_str, sizeof(dsi_config.single_qmux_ch_name));

  /* Number of DSI handles */
  snprintf(prop_name, DSI_CFGDB_STRVAL_MAX, "%s%s%s", "dsi_config.", target, ".num_dsi_handles");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &num_dsi_handles, sizeof(num_dsi_handles));
  if (ret != CFGDB_SUCCESS)
  {
    DSI_LOG_ERROR("Error reading property [%s] Err[%d]", prop_name, ret);
    goto bail;
  }
  dsi_config.num_dsi_handles = num_dsi_handles;

  /* DSI Device names */
  for (i = 0; i < num_dsi_handles; ++i)
  {
    snprintf(prop_name, DSI_CFGDB_STRVAL_MAX, "%s%s%s%d", "dsi_config.", target, ".device_names.",i);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      DSI_LOG_ERROR("Error reading property [%s] Err[%d]", prop_name, ret);
      goto bail;
    }

    if (NULL != dsi_device_names[i])
    {
      free(dsi_device_names[i]);
      dsi_device_names[i] = NULL;
    }

    dsi_device_names[i] = (char *)malloc(DSI_DEV_STR_MAX_LEN * sizeof(char));
    if (NULL == dsi_device_names[i])
    {
      goto bail;
    }

    strlcpy(dsi_device_names[i], prop_val_str, DSI_DEV_STR_MAX_LEN);
  }

  /* DSI Control port names */
  for (i=0; i < num_dsi_handles; ++i)
  {
    snprintf(prop_name, DSI_CFGDB_STRVAL_MAX, "%s%s%s%d", "dsi_config.", target, ".control_port_names.",i);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      DSI_LOG_ERROR("Error reading property [%s] Err[%d]", prop_name, ret);
      goto bail;
    }

    dsi_qmi_port_names[i] = get_port_name(prop_val_str);
  }

  dsi_config_print();
  result = 0;

bail:
  if (0 == result)
  {
    DSI_LOG_INFO("%s", "dsi_config_load using configdb successful");
  }
  else
  {
    DSI_LOG_ERROR("%s", "dsi_config_load using configdb failed!!!");
  }

  return result;
}

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
void dsi_config_print()
{
  int i;

  DSI_LOG_DEBUG("%s", "DSI configuration:");
  DSI_LOG_DEBUG("qos_enabled: %d", dsi_config.qos_enable);

  DSI_LOG_DEBUG("rmnet_data_enabled: %d", dsi_config.rmnet_data_enable);
  DSI_LOG_DEBUG("physical_network_device: %s", dsi_config.phys_net_dev);

  DSI_LOG_DEBUG("single_qmux_channel_enabled: %d", dsi_config.single_qmux_ch_enabled);
  DSI_LOG_DEBUG("single_qmux_channel_name: %s", dsi_config.single_qmux_ch_name);

  DSI_LOG_DEBUG("modem_ssr_state : %d", dsi_config.modem_ssr_state);

  DSI_LOG_DEBUG("num_dsi_handles: %d", dsi_config.num_dsi_handles);
  DSI_LOG_DEBUG("%s", "device names:");
  for (i = 0; i < dsi_config.num_dsi_handles; i++)
  {
    DSI_LOG_DEBUG("..%s", dsi_device_names[i]);
  }

  DSI_LOG_DEBUG("%s", "control port names:");
  for (i = 0; i < dsi_config.num_dsi_handles; i++)
  {
    DSI_LOG_DEBUG("..%s", dsi_qmi_port_names[i]);
  }
  DSI_LOG_DEBUG("%s", "===================");
}


