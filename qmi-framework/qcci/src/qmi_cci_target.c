/******************************************************************************
  @file    qmi_cci_target.c
  @brief   The QMI common client interface target specific module

  ---------------------------------------------------------------------------
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#include <string.h>
#include <ctype.h>
#include "qmi_client.h"
#include "qmi_cci_target.h"
#include "qmi_cci_common.h"

#ifdef QMI_FW_ANDROID
#include <cutils/properties.h>
#include <private/android_filesystem_config.h>

#define QMI_FW_PROPERTY_BASEBAND "ro.baseband"
#define QMI_FW_PROPERTY_BASEBAND_SIZE (PROPERTY_VALUE_MAX)
#define QMI_FW_BASEBAND_VALUE_APQ "apq"
#define QMI_FW_BASEBAND_VALUE_UNDEFINED "undefined"
#endif

#define QMI_FW_CONF_FILE "/etc/qmi_fw.conf"
#define MAX_LINE_LENGTH 80
#define QMI_CCI_DBG_CONF_STR "QMI_CCI_DEBUG_LEVEL="

#ifdef QCCI_OVER_QMUX
#include "qmi_client_instance_defs.h"
extern qmi_cci_xport_ops_type qmuxd_ops;
#endif

extern qmi_cci_xport_ops_type qcci_ipc_router_ops;
extern int open_lookup_sock_fd(void);
extern void close_lookup_sock_fd(void);
extern int smem_log_init(void);
extern void smem_log_exit(void);
extern void qmi_cci_init(void);
extern void qmi_cci_deinit(void);

static unsigned int use_qmuxd = 0;

#ifdef QMI_FW_SYSLOG
#define DEFAULT_DBG_LEVEL 4
#else
#define DEFAULT_DBG_LEVEL 5
#endif

unsigned int qmi_cci_debug_level = DEFAULT_DBG_LEVEL;

#if defined (QMI_FW_ANDROID) || defined (QMI_FW_SYSLOG)
static void qmi_cci_debug_init(void)
{
  char line[MAX_LINE_LENGTH];
  char debug_level[2];
  FILE *fp;

  fp = fopen(QMI_FW_CONF_FILE, "r");
  if(!fp)
    return;

  while(fgets(line, MAX_LINE_LENGTH, fp))
  {
    if(!strncmp(line, QMI_CCI_DBG_CONF_STR, strlen(QMI_CCI_DBG_CONF_STR)))
    {
      debug_level[0] = line[strlen(QMI_CCI_DBG_CONF_STR)];
      debug_level[1] = '\0';
      if(isdigit(debug_level[0]))
      {
        qmi_cci_debug_level = atoi(debug_level);
        break;
      }
    }
  }
  fclose(fp);
}
#else
static void qmi_cci_debug_init(void)
{
}
#endif

#ifdef QCCI_OVER_QMUX
/*===========================================================================
  FUNCTION  qmi_cci_xport_register
===========================================================================*/
qmi_client_error_type qmi_cci_qmux_xport_register
(
 qmi_client_qmux_instance_type instance
)
{
  if ((instance < QMI_CLIENT_QMUX_BASE_INSTANCE) ||
      (instance > QMI_CLIENT_QMUX_MAX_INSTANCE_IDS))
    return QMI_INTERNAL_ERR;

  qmi_cci_xport_start(&qmuxd_ops, (void *)instance);
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_cci_xport_unregister
===========================================================================*/
qmi_client_error_type qmi_cci_qmux_xport_unregister
(
 qmi_client_qmux_instance_type instance
)
{
  if ((instance < QMI_CLIENT_QMUX_BASE_INSTANCE) ||
      (instance > QMI_CLIENT_QMUX_MAX_INSTANCE_IDS))
    return QMI_INTERNAL_ERR;

  qmi_cci_xport_stop(&qmuxd_ops, (void *)instance);
  return QMI_NO_ERR;
}
#endif

/*===========================================================================
FUNCTION: qmi_fw_cci_init

DESCRIPTION:
   Initialize the QCCI library.  This function is called when the QCCI
   shared library is loaded, before application's main() is started.

DEPENDENCIES:
   None

ARGUMENTS:
   None

RETURN VALUE:
   None

SIDE EFFECTS:
===========================================================================*/
#ifdef __GNUC__
void __attribute__ ((constructor)) qmi_fw_cci_init(void)
{
  qmi_cci_init();
  smem_log_init();
  qmi_cci_xport_start(&qcci_ipc_router_ops, NULL);
#ifdef QCCI_OVER_QMUX
#ifdef QMI_FW_ANDROID
  int ret = 0;
  char args[QMI_FW_PROPERTY_BASEBAND_SIZE];
  char def[QMI_FW_PROPERTY_BASEBAND_SIZE];

  (void)strlcpy(def, QMI_FW_BASEBAND_VALUE_UNDEFINED,
                QMI_FW_PROPERTY_BASEBAND_SIZE);
  memset(args, 0, sizeof(args));
  ret = property_get(QMI_FW_PROPERTY_BASEBAND, args, def);
  if ((ret > 0) && (ret <= QMI_FW_PROPERTY_BASEBAND_SIZE))
  {
    /*In non APQ targets, use QMUXD*/
    if(strncmp(args, QMI_FW_BASEBAND_VALUE_APQ,
               sizeof(QMI_FW_BASEBAND_VALUE_APQ)))
    {
      use_qmuxd = 1;
      qmi_cci_xport_start(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
      qmi_cci_xport_start(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
      qmi_cci_xport_start(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
      qmi_cci_xport_start(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);
    }
  }
#else
  use_qmuxd = 1;
  qmi_cci_xport_start(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
  qmi_cci_xport_start(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
  qmi_cci_xport_start(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
  qmi_cci_xport_start(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);
#endif
#endif
  qmi_cci_debug_init();
}
#endif

/*===========================================================================
FUNCTION: qmi_fw_cci_deinit

DESCRIPTION:
   Cleans up the QCCI library.  This function is called after exit() or
   after application's main() completes.

DEPENDENCIES:
   None

ARGUMENTS:
   None

RETURN VALUE:
   None

SIDE EFFECTS:
===========================================================================*/
#ifdef __GNUC__
void __attribute__ ((destructor)) qmi_fw_cci_deinit(void)
{
  qmi_cci_xport_stop(&qcci_ipc_router_ops, NULL);
  if (use_qmuxd)
  {
#ifdef QCCI_OVER_QMUX
    qmi_cci_xport_stop(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
    qmi_cci_xport_stop(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
    qmi_cci_xport_stop(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
    qmi_cci_xport_stop(&qmuxd_ops, (void *)QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);
#endif
  }
  /* The lookup socket file descriptor is not opened in the constructor due
   * to boot up delay issues.*/
  close_lookup_sock_fd();
  smem_log_exit();
  qmi_cci_deinit();
}
#endif
