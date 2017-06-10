/******************************************************************************
  @file    qmi_platform_config.c
  @brief   The QMI platform configuration layer. Provides support for
  reading from configuration file and updating internal QMUXD data structures.

  ---------------------------------------------------------------------------
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_platform_xml.h"
#include "configdb.h"
#include "qmi_qmux.h"

struct conn_id_map
{
  char conn_id_str[QMI_CONFIG_STRVAL_MAX];
  qmi_connection_id_type conn_id;
};

struct transport_map
{
    char trans_str[QMI_CONFIG_STRVAL_MAX];
    unsigned char trans_const;
};

/* Should modify the constant below when modifying this table */
static struct conn_id_map conn_id_map_tbl[] =
{
  {"QMI_CONN_ID_RMNET_0" , QMI_CONN_ID_RMNET_0 },
  {"QMI_CONN_ID_RMNET_1" , QMI_CONN_ID_RMNET_1 },
  {"QMI_CONN_ID_RMNET_2" , QMI_CONN_ID_RMNET_2 },
  {"QMI_CONN_ID_RMNET_3" , QMI_CONN_ID_RMNET_3 },
  {"QMI_CONN_ID_RMNET_4" , QMI_CONN_ID_RMNET_4 },
  {"QMI_CONN_ID_RMNET_5" , QMI_CONN_ID_RMNET_5 },
  {"QMI_CONN_ID_RMNET_6" , QMI_CONN_ID_RMNET_6 },
  {"QMI_CONN_ID_RMNET_7" , QMI_CONN_ID_RMNET_7 },
  {"QMI_CONN_ID_RMNET_8" , QMI_CONN_ID_RMNET_8 },
  {"QMI_CONN_ID_RMNET_9" , QMI_CONN_ID_RMNET_9 },
  {"QMI_CONN_ID_RMNET_10" , QMI_CONN_ID_RMNET_10 },
  {"QMI_CONN_ID_RMNET_11" , QMI_CONN_ID_RMNET_11 },
  {"QMI_CONN_ID_REV_RMNET_0" , QMI_CONN_ID_REV_RMNET_0 },
  {"QMI_CONN_ID_REV_RMNET_1" , QMI_CONN_ID_REV_RMNET_1 },
  {"QMI_CONN_ID_REV_RMNET_2" , QMI_CONN_ID_REV_RMNET_2 },
  {"QMI_CONN_ID_REV_RMNET_3" , QMI_CONN_ID_REV_RMNET_3 },
  {"QMI_CONN_ID_REV_RMNET_4" , QMI_CONN_ID_REV_RMNET_4 },
  {"QMI_CONN_ID_REV_RMNET_5" , QMI_CONN_ID_REV_RMNET_5 },
  {"QMI_CONN_ID_REV_RMNET_6" , QMI_CONN_ID_REV_RMNET_6 },
  {"QMI_CONN_ID_REV_RMNET_7" , QMI_CONN_ID_REV_RMNET_7 },
  {"QMI_CONN_ID_REV_RMNET_8" , QMI_CONN_ID_REV_RMNET_8 },
  {"QMI_CONN_ID_RMNET_SDIO_0" , QMI_CONN_ID_RMNET_SDIO_0 },
  {"QMI_CONN_ID_RMNET_SDIO_1" , QMI_CONN_ID_RMNET_SDIO_1 },
  {"QMI_CONN_ID_RMNET_SDIO_2" , QMI_CONN_ID_RMNET_SDIO_2 },
  {"QMI_CONN_ID_RMNET_SDIO_3" , QMI_CONN_ID_RMNET_SDIO_3 },
  {"QMI_CONN_ID_RMNET_SDIO_4" , QMI_CONN_ID_RMNET_SDIO_4 },
  {"QMI_CONN_ID_RMNET_SDIO_5" , QMI_CONN_ID_RMNET_SDIO_5 },
  {"QMI_CONN_ID_RMNET_SDIO_6" , QMI_CONN_ID_RMNET_SDIO_6 },
  {"QMI_CONN_ID_RMNET_SDIO_7" , QMI_CONN_ID_RMNET_SDIO_7 },
  {"QMI_CONN_ID_RMNET_MDM_0" , QMI_CONN_ID_RMNET_MDM_0 },
  {"QMI_CONN_ID_RMNET_MDM_1" , QMI_CONN_ID_RMNET_MDM_1 },
  {"QMI_CONN_ID_RMNET_MDM_2" , QMI_CONN_ID_RMNET_MDM_2 },
  {"QMI_CONN_ID_RMNET_MDM_3" , QMI_CONN_ID_RMNET_MDM_3 },
  {"QMI_CONN_ID_RMNET_MDM_4" , QMI_CONN_ID_RMNET_MDM_4 },
  {"QMI_CONN_ID_RMNET_MDM_5" , QMI_CONN_ID_RMNET_MDM_5 },
  {"QMI_CONN_ID_RMNET_MDM_6" , QMI_CONN_ID_RMNET_MDM_6 },
  {"QMI_CONN_ID_RMNET_MDM_7" , QMI_CONN_ID_RMNET_MDM_7 },
  {"QMI_CONN_ID_REV_RMNET_MDM_0" , QMI_CONN_ID_REV_RMNET_MDM_0 },
  {"QMI_CONN_ID_REV_RMNET_MDM_1" , QMI_CONN_ID_REV_RMNET_MDM_1 },
  {"QMI_CONN_ID_REV_RMNET_MDM_2" , QMI_CONN_ID_REV_RMNET_MDM_2 },
  {"QMI_CONN_ID_REV_RMNET_MDM_3" , QMI_CONN_ID_REV_RMNET_MDM_3 },
  {"QMI_CONN_ID_REV_RMNET_MDM_4" , QMI_CONN_ID_REV_RMNET_MDM_4 },
  {"QMI_CONN_ID_REV_RMNET_MDM_5" , QMI_CONN_ID_REV_RMNET_MDM_5 },
  {"QMI_CONN_ID_REV_RMNET_MDM_6" , QMI_CONN_ID_REV_RMNET_MDM_6 },
  {"QMI_CONN_ID_REV_RMNET_MDM_7" , QMI_CONN_ID_REV_RMNET_MDM_7 },
  {"QMI_CONN_ID_REV_RMNET_MDM_8" , QMI_CONN_ID_REV_RMNET_MDM_8 },
  {"QMI_CONN_ID_RMNET_USB_0" , QMI_CONN_ID_RMNET_USB_0 },
  {"QMI_CONN_ID_RMNET_USB_1" , QMI_CONN_ID_RMNET_USB_1 },
  {"QMI_CONN_ID_RMNET_USB_2" , QMI_CONN_ID_RMNET_USB_2 },
  {"QMI_CONN_ID_RMNET_USB_3" , QMI_CONN_ID_RMNET_USB_3 },
  {"QMI_CONN_ID_RMNET_USB_4" , QMI_CONN_ID_RMNET_USB_4 },
  {"QMI_CONN_ID_RMNET_USB_5" , QMI_CONN_ID_RMNET_USB_5 },
  {"QMI_CONN_ID_RMNET_USB_6" , QMI_CONN_ID_RMNET_USB_6 },
  {"QMI_CONN_ID_RMNET_USB_7" , QMI_CONN_ID_RMNET_USB_7 },
  {"QMI_CONN_ID_REV_RMNET_USB_0" , QMI_CONN_ID_REV_RMNET_USB_0 },
  {"QMI_CONN_ID_REV_RMNET_USB_1" , QMI_CONN_ID_REV_RMNET_USB_1 },
  {"QMI_CONN_ID_REV_RMNET_USB_2" , QMI_CONN_ID_REV_RMNET_USB_2 },
  {"QMI_CONN_ID_REV_RMNET_USB_3" , QMI_CONN_ID_REV_RMNET_USB_3 },
  {"QMI_CONN_ID_REV_RMNET_USB_4" , QMI_CONN_ID_REV_RMNET_USB_4 },
  {"QMI_CONN_ID_REV_RMNET_USB_5" , QMI_CONN_ID_REV_RMNET_USB_5 },
  {"QMI_CONN_ID_REV_RMNET_USB_6" , QMI_CONN_ID_REV_RMNET_USB_6 },
  {"QMI_CONN_ID_REV_RMNET_USB_7" , QMI_CONN_ID_REV_RMNET_USB_7 },
  {"QMI_CONN_ID_REV_RMNET_USB_8" , QMI_CONN_ID_REV_RMNET_USB_8 },
  {"QMI_CONN_ID_MAX_NON_BCAST" , QMI_CONN_ID_MAX_NON_BCAST },
  {"QMI_CONN_ID_RMNET_12" , QMI_CONN_ID_RMNET_12 },
  {"QMI_CONN_ID_RMNET_13" , QMI_CONN_ID_RMNET_13 },
  {"QMI_CONN_ID_RMNET_SMUX_0" , QMI_CONN_ID_RMNET_SMUX_0 },
  {"QMI_CONN_ID_RMNET_MDM2_0" , QMI_CONN_ID_RMNET_MDM2_0 },
  {"QMI_CONN_ID_RMNET_MDM2_1" , QMI_CONN_ID_RMNET_MDM2_1 },
  {"QMI_CONN_ID_RMNET_MDM2_2" , QMI_CONN_ID_RMNET_MDM2_2 },
  {"QMI_CONN_ID_RMNET_MDM2_3" , QMI_CONN_ID_RMNET_MDM2_3 },
  {"QMI_CONN_ID_RMNET_MDM2_4" , QMI_CONN_ID_RMNET_MDM2_4 },
  {"QMI_CONN_ID_RMNET_MDM2_5" , QMI_CONN_ID_RMNET_MDM2_5 },
  {"QMI_CONN_ID_RMNET_MDM2_6" , QMI_CONN_ID_RMNET_MDM2_6 },
  {"QMI_CONN_ID_RMNET_MDM2_7" , QMI_CONN_ID_RMNET_MDM2_7 },
  {"QMI_CONN_ID_RMNET_MHI_0" , QMI_CONN_ID_RMNET_MHI_0 },
  {"QMI_CONN_ID_RMNET_MHI_1" , QMI_CONN_ID_RMNET_MHI_1 },
  {"QMI_CONN_ID_PROXY" , QMI_CONN_ID_PROXY },
};

#define QMI_PLATFORM_CONFIG_MAX_CONN_ID ((int)(sizeof(conn_id_map_tbl)/sizeof(conn_id_map_tbl[0])))

/* Should modify the constant below when modifying this table */
static struct transport_map trans_map_table[]=
{
  { "LINUX_QMI_TRANSPORT_UNDEF" , LINUX_QMI_TRANSPORT_UNDEF },
  { "LINUX_QMI_TRANSPORT_SMD"   , LINUX_QMI_TRANSPORT_SMD   },
  { "LINUX_QMI_TRANSPORT_BAM"   , LINUX_QMI_TRANSPORT_BAM   },
  { "LINUX_QMI_TRANSPORT_SDIO"  , LINUX_QMI_TRANSPORT_SDIO  },
  { "LINUX_QMI_TRANSPORT_USB"   , LINUX_QMI_TRANSPORT_USB   },
  { "LINUX_QMI_TRANSPORT_SMUX"  , LINUX_QMI_TRANSPORT_SMUX  },
  { "LINUX_QMI_TRANSPORT_MHI"   , LINUX_QMI_TRANSPORT_MHI   },
  { "LINUX_QMI_TRANSPORT_ALL"   , LINUX_QMI_TRANSPORT_ALL   },
};

#define QMI_PLATFORM_CONFIG_MAX_TRANSPORTS ((int)(sizeof(trans_map_table)/sizeof(trans_map_table[0])))


/*===========================================================================
  FUNCTION  qmi_platform_config_get_transport_const
===========================================================================*/
/*!
@brief
  Returns transport ID if a string representation of transport is provided.
*/
/*=========================================================================*/
static unsigned char qmi_platform_config_get_transport_const
(
  char *transport_str
)
{
  int i =0;

  for( i = 0; i < QMI_PLATFORM_CONFIG_MAX_TRANSPORTS ; i++)
  {
      if(!strcmp(trans_map_table[i].trans_str, transport_str))
      {
          return trans_map_table[i].trans_const;
      }
  }

  return LINUX_QMI_TRANSPORT_UNDEF;
}

/*===========================================================================
  FUNCTION  qmi_platform_config_get_transport_str
===========================================================================*/
/*!
@brief
  Returns transport ID string if a transport macro value is provided.
*/
/*=========================================================================*/
static char *qmi_platform_config_get_transport_str
(
  unsigned char transport
)
{
  int i =0;

  for (i = 0; i < QMI_PLATFORM_CONFIG_MAX_TRANSPORTS; i++)
  {
    if (trans_map_table[i].trans_const == transport)
    {
      return trans_map_table[i].trans_str;
    }
  }

  return "LINUX_QMI_TRANSPORT_UNDEF";
}


/*===========================================================================
  FUNCTION  qmi_platform_config_get_conn_id_const
===========================================================================*/
/*!
@brief
  Returns connection ID if a string representation of conn_id is provided.
*/
/*=========================================================================*/
static qmi_connection_id_type qmi_platform_config_get_conn_id_const
(
  char *conn_id_str
)
{

  int i;

  for( i=0; i < QMI_PLATFORM_CONFIG_MAX_CONN_ID; ++i)
  {
    if(!strcmp(conn_id_map_tbl[i].conn_id_str, conn_id_str))
    {
      return conn_id_map_tbl[i].conn_id;
    }
  }

  return QMI_CONN_ID_INVALID;
}

/*===========================================================================
  FUNCTION  qmi_platform_config_get_conn_id_str
===========================================================================*/
/*!
@brief
  Gets platform connection ID string from connection ID enum.
*/
/*=========================================================================*/
static char * qmi_platform_config_get_conn_id_str
(
  qmi_connection_id_type conn_id
)
{

  int i;

  for (i = 0; i < QMI_PLATFORM_CONFIG_MAX_CONN_ID; ++i)
  {
    if(conn_id_map_tbl[i].conn_id == conn_id)
    {
      return conn_id_map_tbl[i].conn_id_str;
    }
  }

  return "QMI_CONN_ID_INVALID";
}


/*===========================================================================
  FUNCTION  qmi_config_print()
===========================================================================*/
/*!
@brief
  Prints the QMI configuration values. This include what is the data port name,
  connection ID number, whether it was enabled and opened at powerup etc.
*/
/*=========================================================================*/
void qmi_config_print()
{
  int i;

  QMI_DEBUG_MSG_0("qmuxd: configuration: index, ch_name, conn_id_str, transport, device, enabled, open_at_powerup\n");
  for (i = 0; i < LINUX_QMI_MAX_CONN_SUPPORTED; i++)
  {
    QMI_DEBUG_MSG("..%d: %s %s %s %d %d", i,
        linux_qmi_conn_id_enablement_array[i].data_ctl_port,
        qmi_platform_config_get_conn_id_str(linux_qmi_conn_id_enablement_array[i].qmi_conn_id),
        qmi_platform_config_get_transport_str(linux_qmi_conn_id_enablement_array[i].transport),
        linux_qmi_conn_id_enablement_array[i].enabled,
        linux_qmi_conn_id_enablement_array[i].open_at_powerup);
  }
}

/*===========================================================================
  FUNCTION  qmi_platform_config_configure_ports_from_xml
===========================================================================*/
/*!
@brief
  This function would use given XML and given target configuration
  to configure the ports correctly.

@params[in] xml_file: XML file to be used for configuration.
@params[in] target: Target configuration within that XML file

@return

@note
  - Side Effects
*/
/*=========================================================================*/
int qmi_platform_config_configure_ports_xml
(
  char *xml_file,
  char *target
)
{
  char param[QMI_CONFIG_STRVAL_MAX];
  char str_val[QMI_CONFIG_STRVAL_MAX];
  int int_val;
  int32 num_ctrl_ports, ret;
  int result = -1, i;
  qmi_connection_id_type conn_id;
  unsigned char transport;
  char ch_name[QMI_CONFIG_STRVAL_MAX];
  char conn_id_str[QMI_CONFIG_STRVAL_MAX];
  char transport_str[QMI_CONFIG_STRVAL_MAX];
  char device_name[QMI_CONFIG_STRVAL_MAX];
  char device[QMI_CONFIG_STRVAL_MAX];
  int32 enabled;
  int32 open_at_powerup;


  QMI_DEBUG_MSG_2("qmuxd: Loading configuration file %s[%s]\n", xml_file, target);

  /* Pre-fill with default configuration (in case configdb fails) */
  for (i = 0; i < LINUX_QMI_MAX_CONN_SUPPORTED; i++)
  {
    linux_qmi_conn_id_enablement_array[i].enabled = FALSE;
    linux_qmi_conn_id_enablement_array[i].open_at_powerup = FALSE;
  }

  ret = configdb_init(CFGDB_OPMODE_CACHED, xml_file);
  if (CFGDB_SUCCESS != ret)
  {
    QMI_ERR_MSG_2("qmuxd: Cannot open/parse configdb file %s Err[%d]\n", xml_file,ret);
    goto bail;
  }

  snprintf(param, QMI_CONFIG_STRVAL_MAX, "%s%s%s", "qmi_config.", target, ".control_ports_len");
  ret = configdb_get_parameter(param, CFGDB_TYPE_INT, &num_ctrl_ports, sizeof(num_ctrl_ports));
  if (CFGDB_SUCCESS != ret)
  {
    QMI_ERR_MSG_2("qmuxd: Cannot parse %s, config file %s\n", param, xml_file);
    goto bail;
  }

  for (i = 0; i < num_ctrl_ports; i++)
  {
    snprintf(param, QMI_CONFIG_STRVAL_MAX, "%s%s%s%d", "qmi_config.", target, ".ch_name.", i);
    ret = configdb_get_parameter(param, CFGDB_TYPE_STRING, ch_name, sizeof(ch_name));
    if (CFGDB_SUCCESS != ret)
    {
      QMI_ERR_MSG_2("qmuxd: Cannot parse %s, config file %s\n", param, xml_file);
      goto bail;
    }

    snprintf(param, QMI_CONFIG_STRVAL_MAX, "%s%s%s%d", "qmi_config.", target, ".conn_id.", i);
    ret = configdb_get_parameter(param, CFGDB_TYPE_STRING, conn_id_str, sizeof(conn_id_str));
    if (CFGDB_SUCCESS != ret)
    {
      QMI_ERR_MSG_2("qmuxd: Cannot parse %s, config file %s\n", param, xml_file);
      goto bail;
    }

    conn_id = qmi_platform_config_get_conn_id_const(conn_id_str);
    if (QMI_CONN_ID_INVALID == conn_id)
    {
      QMI_ERR_MSG_2("qmuxd: Invalid conn id string parsed [%s], conf [%s]\n", conn_id_str, xml_file);
      goto bail;
    }

    snprintf(param, QMI_CONFIG_STRVAL_MAX, "%s%s%s%d", "qmi_config.", target, ".transport.", i);
    ret = configdb_get_parameter(param, CFGDB_TYPE_STRING, transport_str, sizeof(transport_str));
    if (CFGDB_SUCCESS != ret)
    {
      QMI_ERR_MSG_2("qmuxd: Cannot parse %s, config file %s\n", param, xml_file);
      goto bail;
    }

    transport = qmi_platform_config_get_transport_const(transport_str);
    if (LINUX_QMI_TRANSPORT_UNDEF == transport)
    {
      QMI_ERR_MSG_2("qmuxd: Invalid transport string parsed [%s], conf [%s]\n", transport_str, xml_file);
      goto bail;
    }

    snprintf(param, QMI_CONFIG_STRVAL_MAX, "%s%s%s%d", "qmi_config.", target, ".enabled.", i);
    ret = configdb_get_parameter(param, CFGDB_TYPE_INT, &enabled, sizeof(enabled));
    if (CFGDB_SUCCESS != ret)
    {
      QMI_ERR_MSG_2("qmuxd: Cannot parse %s, config file %s\n", param, xml_file);
      goto bail;
    }

    snprintf(param, QMI_CONFIG_STRVAL_MAX, "%s%s%s%d", "qmi_config.", target, ".open_at_powerup.", i);
    ret = configdb_get_parameter(param, CFGDB_TYPE_INT, &open_at_powerup, sizeof(open_at_powerup));
    if (CFGDB_SUCCESS != ret)
    {
      QMI_ERR_MSG_2("qmuxd: Cannot parse %s, config file %s\n", param, xml_file);
      goto bail;
    }

#ifdef FEATURE_QMI_TEST
    snprintf(param, QMI_CONFIG_STRVAL_MAX, "%s%s%s%d", "qmi_config.", target, ".dev_name_test.", i);
    ret = configdb_get_parameter(param, CFGDB_TYPE_STRING, device_name, sizeof(device_name));
    if (CFGDB_SUCCESS != ret)
    {
      QMI_ERR_MSG_2("qmuxd: Cannot parse %s, config file %s\n", param, xml_file);
      goto bail;
    }
#else
    snprintf(param, QMI_CONFIG_STRVAL_MAX, "%s%s%s%d", "qmi_config.", target, ".dev_name.", i);
    ret = configdb_get_parameter(param, CFGDB_TYPE_STRING, device_name, sizeof(device_name));
    if (CFGDB_SUCCESS != ret)
    {
      QMI_ERR_MSG_2("qmuxd: Cannot parse %s, config file %s\n", param, xml_file);
      goto bail;
    }
#endif

    LINUX_QMI_UPDATE_CONFIG(ch_name, conn_id, transport, device_name, enabled, open_at_powerup);

    qmi_qmux_disable_port(conn_id, ch_name, FALSE);
  }

  qmi_config_print();
  result = 0;

bail:
  return result;
} /* qmi_platform_config_configure_ports_xml */

