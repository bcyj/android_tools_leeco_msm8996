/*!
  @file
  netmgr_config.c

  @brief
  This file provides implementation of netmgr configration using configdb.
*/

/*===========================================================================
  Copyright (c) 2013 - 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
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

#include "netmgr_config.h"
#include "netmgr_util.h"
#include "configdb.h"
#include "ds_util.h"

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION:  netmgr_process_iwlan_config
===========================================================================*/
/*!
    @brief
    This function processes the iwlan related config items

    @param[in] target: configuration to be used within the XML file.

    @return  0 on error.
    @return -1 on failure.
*/
/*=========================================================================*/
LOCAL
int netmgr_process_iwlan_config
(
  char* target
)
{
  int result = 0;
  int ret;
  int i,j, rev_min;
  char prop_name[NETMGR_CFGDB_STRVAL_MAX];
  char prop_val_str[NETMGR_CFGDB_STRVAL_MAX];
  int32 prop_val_int;
  int32 rev_control_ports_len = 0;
  int32 rev_data_ports_len = 0;

  rev_min = 0;
  /* iwlan_enable */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".iwlan_enable");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto err;
  }
  switch(prop_val_int)
  {
    case 0:
     netmgr_main_cfg.iwlan_enabled = IWLAN_DISABLE;
     break;

    case 1:
     netmgr_main_cfg.iwlan_enabled = IWLAN_ENABLE;
     break;

    case 2:
     netmgr_main_cfg.iwlan_enabled = NSWO_ONLY;
     break;

    default:
     netmgr_main_cfg.iwlan_enabled = IWLAN_DISABLE;
     netmgr_log_err("Error reading property [%s] property value[%d]", prop_name, prop_val_int);
  }

  /* The reverse ports starting and ending index depends on the
   * modem being used. Query the correct starting and ending indexes
   */
  if (netmgr_main_cfg.modem_enable[0])
  {
    rev_min = NETMGR_MAIN_GET_INST_MIN_REV(NETMGR_MODEM_MSM);
  }
  else if(netmgr_main_cfg.modem_enable[1])
  {
    rev_min = NETMGR_MAIN_GET_INST_MIN_REV(NETMGR_MODEM_MDM);
  }

  /* rev_control_ports_len and rev_control_ports */
  j = 0;
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rev_control_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &rev_control_ports_len, sizeof(rev_control_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto err;
  }

  for (i = rev_min; i < (rev_min + rev_control_ports_len); i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".rev_control_ports.",j);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto err;
    }
    strlcpy(netmgr_ctl_port_array[i].qmi_conn_id, prop_val_str, sizeof(netmgr_ctl_port_array[i].qmi_conn_id));
    netmgr_ctl_port_array[i].enabled = TRUE;
    j++;
  }

  /* rev_data_ports_len and rev_data_ports */
  j = 0;
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rev_data_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &rev_data_ports_len, sizeof(rev_data_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto err;
  }
  for (i = rev_min; i < (rev_min + rev_data_ports_len); i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".rev_data_ports.",j);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto err;
    }
    strlcpy(netmgr_ctl_port_array[i].data_ctl_port, prop_val_str, sizeof(netmgr_ctl_port_array[i].data_ctl_port));
    netmgr_ctl_port_array[i].enabled = TRUE;
    j++;
  }

  /* Remove prefix table as an enhancement later */
  if (netmgr_main_cfg.rmnet_data_enabled)
  {
    if (netmgr_main_cfg.modem_enable[0])
    {
      /* Change prefix for reverse links */
      netmgr_log_high("%s", "Changing MSM rev prefixes");
      strlcpy(NETMGR_MAIN_GET_DEV_REV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_REV_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
    else if(netmgr_main_cfg.modem_enable[1])
    {
      /* Change prefix for reverse links */
      netmgr_log_high("%s", "Changing MDM rev prefixes");
      strlcpy(NETMGR_MAIN_GET_DEV_REV_PREFIX(NETMGR_MODEM_MDM),
              NETMGR_MAIN_REV_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
  }

err:
  return result;
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION:  netmgr_config_load
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
int netmgr_config_load
(
  char* xml_file,
  char* target
)
{
  int result = 0;
  int32 ret;
  int i;
  char prop_name[NETMGR_CFGDB_STRVAL_MAX];
  int32 num_modems = 0;
  int32 control_ports_len = 0;
  int32 data_ports_len = 0;
  int32 prop_val_int;
  char prop_val_str[NETMGR_CFGDB_STRVAL_MAX];

  netmgr_log_med("netmgr_config: Configuring using file %s, target %s\n", xml_file, target);
  ret = configdb_init(CFGDB_OPMODE_CACHED, xml_file);
  if (CFGDB_SUCCESS != ret)
  {
    netmgr_log_err("Unable to open/parse config file [%s] Err [%d]", xml_file, ret);
    result = -1;
    goto bail;
  }

  /* qmi_dpm_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".qmi_dpm_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.dpm_enabled = (prop_val_int ? TRUE : FALSE);

  /* wda_data_format_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".wda_data_format_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.wda_data_format = (prop_val_int ? TRUE : FALSE);

#ifdef NETMGR_OFFTARGET
  netmgr_main_cfg.wda_data_format = FALSE; // For now WDA data format is disabled.
#endif
  /* ep_type */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".ep_type");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]. Default[%d]", prop_name, ret, DS_EP_TYPE_EMBEDDED);
    prop_val_int = DS_EP_TYPE_EMBEDDED;
  }
  netmgr_main_cfg.ep_type = prop_val_int;

  /* single_qmux_ch_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".single_qmux_ch_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.single_qmux_ch_enabled = (prop_val_int ? TRUE : FALSE);

  /* single_qmux_ch_conn_id_str */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".single_qmux_ch_conn_id_str");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  strlcpy(netmgr_main_cfg.qmux_ch_name, prop_val_str, sizeof(netmgr_main_cfg.qmux_ch_name));

  /* single_qmux_ch_name */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".single_qmux_ch_name");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  strlcpy(netmgr_main_cfg.smd_ch_name, prop_val_str, sizeof(netmgr_main_cfg.smd_ch_name));

  /* rmnet_data_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rmnet_data_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.rmnet_data_enabled = (prop_val_int ? TRUE : FALSE);

  /* dataformat_agg_dl_pkt */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_dl_pkt");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.dl_agg_cnt = (int)prop_val_int;

  /* dataformat_agg_dl_size */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_dl_size");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.dl_agg_size = (int)prop_val_int;

  /* dataformat_dl_data_aggregation_protocol */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_dl_data_aggregation_protocol");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.dl_data_aggregation_protocol = (int)prop_val_int;

  /* phys_net_dev */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".phys_net_dev");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }

  strlcpy(netmgr_main_cfg.phys_net_dev, prop_val_str, sizeof(netmgr_main_cfg.phys_net_dev));

  /* rps_mask */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rps_mask");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.rps_mask = (int)prop_val_int;

  /* netdev_budget */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".netdev_budget");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.netdev_budget = (int)prop_val_int;

  /* low_latency_filters */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".low_latency_filters");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.low_latency_filters = (prop_val_int ? TRUE : FALSE);

  /* tc_ul_baserate */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".tc_ul_baserate");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Error reading property [%s] Err[%d]; setting to 0", prop_name, ret);
    netmgr_main_cfg.tc_ul_baserate = 0;
  }
  else
  {
    /* TODO: configdb does not have a UL type; we are limited to ~2Gbps */
    netmgr_main_cfg.tc_ul_baserate = (unsigned long)prop_val_int;
  }

  /* tc_ul_burstrate */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".tc_ul_burst");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Error reading property [%s] Err[%d]; setting to 0", prop_name, ret);
    netmgr_main_cfg.tc_ul_burst = 0;
  }
  else
  {
    netmgr_main_cfg.tc_ul_burst = (unsigned long)prop_val_int;
  }

  /* netdev_max_backlog */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".netdev_max_backlog");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.netdev_max_backlog = (int)prop_val_int;

  /* num_modems and enable status */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".num_modems");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &num_modems, sizeof(num_modems));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  for (i = 0; i < num_modems; i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".modems_enabled.",i);
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto bail;
    }
    netmgr_main_cfg.modem_enable[i] = (prop_val_int ? TRUE : FALSE);
  }

  /* Disable all the links first, only enable ones present in config file */
  for (i = 0; i < NETMGR_MAX_LINK; i++)
  {
    netmgr_ctl_port_array[i].enabled = FALSE;
  }

  /* control_ports_len and control_ports */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".control_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &control_ports_len, sizeof(control_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }

  for (i = 0; i < control_ports_len; i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".control_ports.",i);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto bail;
    }
    strlcpy(netmgr_ctl_port_array[i].qmi_conn_id, prop_val_str, sizeof(netmgr_ctl_port_array[i].qmi_conn_id));
    netmgr_ctl_port_array[i].enabled = TRUE;
  }

  /* data_ports_len and data_ports */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".data_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &data_ports_len, sizeof(data_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  for (i = 0; i < data_ports_len; i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".data_ports.",i);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto bail;
    }
    strlcpy(netmgr_ctl_port_array[i].data_ctl_port, prop_val_str, sizeof(netmgr_ctl_port_array[i].data_ctl_port));
    netmgr_ctl_port_array[i].enabled = TRUE;
  }

#ifdef FEATURE_DATA_IWLAN
  /* Process iWLAN related configuration items */
  if (0 != netmgr_process_iwlan_config(target))
  {
    netmgr_log_err("%s", "Error processing iWLAN config items");
    result = -1;
    goto bail;
  }
#endif /* FEATURE_DATA_IWLAN */

  /* Remove prefix table as an enhancement later */
  if (netmgr_main_cfg.rmnet_data_enabled)
  {
    if (netmgr_main_cfg.modem_enable[0])
    {
      /* If rmnet_data is enabled, then update the dev-prefix for approriate modem */
      strlcpy(NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
    else if(netmgr_main_cfg.modem_enable[1])
    {
      /* If rmnet_data is enabled, then update the dev-prefix for approriate modem */
      strlcpy(NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MDM),
              NETMGR_MAIN_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
  }

bail:
  return result;
}

/*===========================================================================
  FUNCTION:  netmgr_config_print
===========================================================================*/
/*!
    @brief
    This function prints the configuration currently loaded for netmgr.

    @param None.
    @return  None.
*/
/*=========================================================================*/
void netmgr_config_print()
{
  int i;
  netmgr_log_med("==== Netmanager Configuration ====");
  netmgr_log_med("..qmi_dpm_enabled        : [%d]", netmgr_main_cfg.dpm_enabled);
  netmgr_log_med("..wda_data_format_enabled: [%d]", netmgr_main_cfg.wda_data_format);
  netmgr_log_med("..single_qmux_ch_enabled : [%d]", netmgr_main_cfg.single_qmux_ch_enabled);
  netmgr_log_med("..single_qmux_ch_name    : [%s]", netmgr_main_cfg.qmux_ch_name);
  netmgr_log_med("..single_qmux_ch_id      : [%s]", netmgr_main_cfg.smd_ch_name);
  netmgr_log_med("..rmnet_data_enabled     : [%d]", netmgr_main_cfg.rmnet_data_enabled);
  netmgr_log_med("..phys_net_dev           : [%s]", netmgr_main_cfg.phys_net_dev);
  netmgr_log_med("..msm_modem_enabled      : [%d]", netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]);
  netmgr_log_med("..mdm_modem_enabled      : [%d]", netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]);
  netmgr_log_med("..modem_ssr_state        : [%d]", netmgr_main_cfg.modem_ssr_state);
  netmgr_log_med("..df.dl_agg_size         : [%d]", netmgr_main_cfg.data_format.dl_agg_size);
  netmgr_log_med("..df.dl_agg_cnt          : [%d]", netmgr_main_cfg.data_format.dl_agg_cnt);
  netmgr_log_med("..df.dl_agg_mode         : [%d]", netmgr_main_cfg.data_format.dl_data_aggregation_protocol);

  for (i = 0; i < NETMGR_MAX_LINK; i++)
  {
    netmgr_log_med("Link[%d] port[%s] qmi[%s] modem_wait[%d] enabled[%d]",
                    netmgr_ctl_port_array[i].link_id,
                    netmgr_ctl_port_array[i].data_ctl_port,
                    netmgr_ctl_port_array[i].qmi_conn_id,
                    netmgr_ctl_port_array[i].modem_wait,
                    netmgr_ctl_port_array[i].enabled);
  }
  netmgr_log_med("========");


  return;
}

