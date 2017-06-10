#ifndef QMI_PLATFORM_XML_H
#define QMI_PLATFORM_XML_H

/******************************************************************************
  @file    qmi_platform_xml.h
  @brief   QMI platform configuration module using configdb.

  DESCRIPTION
  Interface definition for QMI platform layer using XML file (configdb)=

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi.h"
#include "qmi_port_defs.h"
#include "qmi_platform_config.h"
#include "qmi_platform_qmux_io.h"

#define LINUX_QMI_MAX_CONN_SUPPORTED QMI_MAX_CONNECTIONS

/* QMI message transport types */
#define LINUX_QMI_TRANSPORT_UNDEF (0x00)
#define LINUX_QMI_TRANSPORT_SMD   (0x01)
#define LINUX_QMI_TRANSPORT_BAM   (0x02)
#define LINUX_QMI_TRANSPORT_SDIO  (0x04)
#define LINUX_QMI_TRANSPORT_USB   (0x08)
#define LINUX_QMI_TRANSPORT_SMUX  (0x10)
#define LINUX_QMI_TRANSPORT_MHI   (0x20)
#define LINUX_QMI_TRANSPORT_ALL   (LINUX_QMI_TRANSPORT_SMD |\
                                   LINUX_QMI_TRANSPORT_BAM |\
                                   LINUX_QMI_TRANSPORT_SDIO|\
                                   LINUX_QMI_TRANSPORT_SMUX|\
                                   LINUX_QMI_TRANSPORT_USB |\
                                   LINUX_QMI_TRANSPORT_MHI)

#define LINUX_QMI_CFG_PARAM_LEN 15


/* ConfigDB max string value */
#define QMI_CONFIG_STRVAL_MAX 100


#define LINUX_QMI_MHI_FWD_BEGIN QMI_CONN_ID_RMNET_MHI_0
#define LINUX_QMI_MHI_FWD_END   QMI_CONN_ID_RMNET_MHI_1
#define LINUX_QMI_MHI_FWD_SET(i, name, conn, trans, device, en, op)                               \
      if( LINUX_QMI_MHI_FWD_BEGIN+i <= LINUX_QMI_MHI_FWD_END ) {                                  \
        strlcpy (linux_qmi_conn_id_enablement_array[LINUX_QMI_MHI_FWD_BEGIN+i].data_ctl_port,    \
            name, LINUX_QMI_CFG_PARAM_LEN );                                                      \
        linux_qmi_conn_id_enablement_array[LINUX_QMI_MHI_FWD_BEGIN+i].qmi_conn_id = conn;        \
        linux_qmi_conn_id_enablement_array[LINUX_QMI_MHI_FWD_BEGIN+i].transport = trans;          \
        linux_qmi_conn_id_enablement_array[LINUX_QMI_MHI_FWD_BEGIN+i].enabled = en;              \
        linux_qmi_conn_id_enablement_array[LINUX_QMI_MHI_FWD_BEGIN+i].open_at_powerup = op;      \
        strlcpy (linux_qmi_qmux_io_conn_info[LINUX_QMI_MHI_FWD_BEGIN+i].port_id_name,             \
                 device,                                                                          \
                 sizeof(linux_qmi_qmux_io_conn_info[ LINUX_QMI_MHI_FWD_BEGIN+i ].port_id_name) ); \
      }

/* Macros for updating linux_qmi_conn_id_enablement_array for external MDM */
#define LINUX_QMI_MDM_FWD_BEGIN (QMI_CONN_ID_RMNET_MDM_0)
#define LINUX_QMI_MDM_FWD_END   (QMI_CONN_ID_RMNET_MDM_7)
#define LINUX_QMI_MDM_FWD_SET( i, conn, tran, device )                                            \
      if( LINUX_QMI_MDM_FWD_BEGIN+i <= LINUX_QMI_MDM_FWD_END ) {                                  \
        linux_qmi_conn_id_enablement_array[ LINUX_QMI_MDM_FWD_BEGIN+i ].qmi_conn_id = conn;       \
        linux_qmi_conn_id_enablement_array[ LINUX_QMI_MDM_FWD_BEGIN+i ].transport = tran;         \
        strlcpy( linux_qmi_qmux_io_conn_info[ LINUX_QMI_MDM_FWD_BEGIN+i ].port_id_name,           \
                 device,                                                                          \
                 sizeof(linux_qmi_qmux_io_conn_info[ LINUX_QMI_MDM_FWD_BEGIN+i ].port_id_name) ); \
      }

/* Macros for updating linux_qmi_conn_id_enablement_array for second external modem */
#define LINUX_QMI_QSC_BEGIN (QMI_CONN_ID_RMNET_SMUX_0)
#define LINUX_QMI_QSC_END   (QMI_CONN_ID_RMNET_SMUX_0)
#define LINUX_QMI_QSC_SET( i, conn, tran, device )                                            \
      if( LINUX_QMI_QSC_BEGIN+i <= LINUX_QMI_QSC_END ) {                                      \
        linux_qmi_conn_id_enablement_array[ LINUX_QMI_QSC_BEGIN+i ].qmi_conn_id = conn;       \
        linux_qmi_conn_id_enablement_array[ LINUX_QMI_QSC_BEGIN+i ].transport = tran;         \
        strlcpy( linux_qmi_qmux_io_conn_info[ LINUX_QMI_QSC_BEGIN+i ].port_id_name,           \
                 device,                                                                      \
                 sizeof(linux_qmi_qmux_io_conn_info[ LINUX_QMI_QSC_BEGIN+i ].port_id_name) ); \
      }

#define LINUX_QMI_MDM2_BEGIN (QMI_CONN_ID_RMNET_MDM2_0)
#define LINUX_QMI_MDM2_END   (QMI_CONN_ID_RMNET_MDM2_7)
#define LINUX_QMI_MDM2_SET( i, conn, tran, device )                                            \
      if( LINUX_QMI_MDM2_BEGIN+i <= LINUX_QMI_MDM2_END ) {                                     \
        linux_qmi_conn_id_enablement_array[ LINUX_QMI_MDM2_BEGIN+i ].qmi_conn_id = conn;       \
        linux_qmi_conn_id_enablement_array[ LINUX_QMI_MDM2_BEGIN+i ].transport = tran;         \
        strlcpy( linux_qmi_qmux_io_conn_info[ LINUX_QMI_MDM2_BEGIN+i ].port_id_name,           \
                 device,                                                                       \
                 sizeof(linux_qmi_qmux_io_conn_info[ LINUX_QMI_MDM2_BEGIN+i ].port_id_name) ); \
      }

/* Macros for updating linux_qmi_conn_id_enablement_array for a given conn */
#define LINUX_QMI_UPDATE_CONFIG(name, conn, trans, device, en, op)         \
      strlcpy( linux_qmi_conn_id_enablement_array[conn].data_ctl_port,     \
               name, LINUX_QMI_CFG_PARAM_LEN );                            \
      linux_qmi_conn_id_enablement_array[ conn ].qmi_conn_id = conn;       \
      linux_qmi_conn_id_enablement_array[ conn ].transport = trans;        \
      linux_qmi_conn_id_enablement_array[ conn ].enabled = en;             \
      linux_qmi_conn_id_enablement_array[ conn ].open_at_powerup = op;     \
      strlcpy( linux_qmi_qmux_io_conn_info[ conn ].port_id_name,           \
               device,                                                     \
               sizeof(linux_qmi_qmux_io_conn_info[ conn ].port_id_name) ); \

/*
   following structure holds the relationship between smd data control
   port and qmi connection id. It also contains a boolean member to
   identify if the qmi connection id is enabled or not
*/
typedef struct linux_qmi_conn_id_enablement_s
{
  char data_ctl_port[LINUX_QMI_CFG_PARAM_LEN];
  qmi_connection_id_type qmi_conn_id;
  unsigned char transport;
  unsigned char enabled;
  unsigned char open_at_powerup;
} linux_qmi_conn_id_config_s;

extern linux_qmi_conn_id_config_s linux_qmi_conn_id_enablement_array[LINUX_QMI_MAX_CONN_SUPPORTED];

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
);

#endif

