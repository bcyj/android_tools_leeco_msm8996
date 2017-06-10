/******************************************************************************

                        N E T M G R _ M A I N . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_main.c
  @brief   Network Manager main function implementation

  DESCRIPTION
  Implementation of NetMgr's main function.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2015 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/14/11   jas        change netmgrd uid to radio at power up
02/08/10   ar         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h> /* open, read */
#include <signal.h>
#include <linux/capability.h>
#include <strings.h>
#include <stdint.h>

#ifndef NETMGR_OFFTARGET
#include <sys/capability.h>
#endif

#include <linux/msm_rmnet.h>        /* RmNet Kernel Constants */
#include <linux/rmnet_data.h>       /* RmNet Kernel Constants */
#include "netmgr_qmi_wda.h"
#include "netmgr_rmnet.h"

#ifdef FEATURE_DS_LINUX_ANDROID
#include <cutils/properties.h>
  #ifndef NETMGR_OFFTARGET
    #include <private/android_filesystem_config.h>
  #endif
#endif

#ifndef PROPERTY_VALUE_MAX
  #define PROPERTY_VALUE_MAX 256
#endif

#include "ds_util.h"
#include "ds_string.h"
#include "ds_trace.h"
#include "netmgr_util.h"
#include "netmgr_defs.h"
#include "netmgr_exec.h"
#include "netmgr_kif.h"
#include "netmgr_qmi.h"
#include "netmgr_tc.h"
#include "netmgr_platform.h"
#include "netmgr_main.h"
#include "netmgr_qmi_dpm.h"
#include "netmgr_qmi_dfs.h"
#include "netmgr_cmdq.h"

#ifdef FEATURE_DATA_IWLAN
#include "netmgr_iwlan_client.h"
#endif /* FEATURE_DATA_IWLAN */

#ifdef NETMGR_TEST
#include "netmgr_test.h"
#endif

/*===========================================================================
                     FORWARD DECLARATIONS
===========================================================================*/


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NETMGR_MAIN_DEFAULT_NINT         NETMGR_MAX_LINK

#ifdef FEATURE_DS_LINUX_ANDROID
/* Macros from Android property database */
#define NETMGR_MAIN_PROPERTY_NINT        "persist.data_netmgrd_nint"
#define NETMGR_MAIN_PROPERTY_NINT_SIZE   NETMGR_MAX_LINK

#define NETMGR_MAIN_PROPERTY_QOS          "persist.data.netmgrd.qos.enable"
#define NETMGR_MAIN_PROPERTY_QOS_SIZE     (5)
#define NETMGR_MAIN_PROPERTY_QOS_DEFAULT  NETMGR_TRUE    /* true or false */

#define NETMGR_MAIN_PROPERTY_QOS_HYBRID          "persist.data.netmgrd.qos.hybrid"
#define NETMGR_MAIN_PROPERTY_QOS_HYBRID_DEFAULT  NETMGR_FALSE    /* true or false */

#ifdef FEATURE_DATA_IWLAN
  #define NETMGR_MAIN_PROPERTY_IWLAN          "persist.data.iwlan.enable"
  #define NETMGR_MAIN_PROPERTY_IWLAN_SIZE     (5)
  #define NETMGR_MAIN_PROPERTY_IWLAN_DEFAULT  NETMGR_FALSE    /* true or false */

  #define NETMGR_MAIN_PROPERTY_IWLAN_IMS          "persist.data.iwlan.ims.enable"
  #define NETMGR_MAIN_PROPERTY_IWLAN_IMS_SIZE     (5)
  #define NETMGR_MAIN_PROPERTY_IWLAN_IMS_DEFAULT  NETMGR_TRUE    /* true or false */
#endif /* FEATURE_DATA_IWLAN */

#define NETMGR_MAIN_PROPERTY_RMNET_DATA          "persist.rmnet.data.enable"
#define NETMGR_MAIN_PROPERTY_RMNET_DATA_SIZE     (5)
#define NETMGR_MAIN_PROPERTY_RMNET_DATA_DEFAULT  NETMGR_FALSE    /* true or false */

#define NETMGR_MAIN_PROPERTY_WDA_ENABLE          "persist.data.wda.enable"
#define NETMGR_MAIN_PROPERTY_WDA_SIZE            (5)
#define NETMGR_MAIN_PROPERTY_WDA_ENABLE_DEFAULT  NETMGR_FALSE

#define NETMGR_MAIN_PROPERTY_TCPACKPRIO          "persist.data.tcpackprio.enable"
#define NETMGR_MAIN_PROPERTY_TCPACKPRIO_DEFAULT  NETMGR_FALSE    /* true or false */

#define NETMGR_MAIN_PROPERTY_DPM_ENABLE          "persist.data.dpm.enable"
#define NETMGR_MAIN_PROPERTY_DPM_ENABLE_DEFAULT  NETMGR_FALSE

/* In band flow control */
#define NETMGR_MAIN_PROPERTY_IBFC                "persist.data.ibfc.enable"
#define NETMGR_MAIN_PROPERTY_IBFC_DEFAULT        NETMGR_FALSE    /* true or false */

#define NETMGR_MAIN_PROPERTY_LOW_LATENCY_FILTERS          "persist.data.llf.enable"
#define NETMGR_MAIN_PROPERTY_LOW_LATENCY_FILTERS_DEFAULT  NETMGR_FALSE    /* true or false */

#define NETMGR_MAIN_PROPERTY_DROPSSDP               "persist.data.dropssdp"
#define NETMGR_MAIN_PROPERTY_DROPSSDP_SIZE          (5)
#define NETMGR_MAIN_PROPERTY_DROPSSDP_DEFAULT       NETMGR_TRUE

#endif /* FEATURE_DS_LINUX_ANDROID */

/* Default network interface name prefix; may be overridden */
LOCAL char netmgr_iname_default[] = "rmnet";


#define NETMGR_ENABLE_RMNET_DATA "persist.data.rmnet.en"
#define NETMGR_DATA_FMT_DL_FMT   "persist.data.df.dl_mode"
#define NETMGR_DATA_FMT_UL_FMT   "persist.data.df.ul_mode"
#define NETMGR_DATA_FMT_DL_PKT   "persist.data.df.agg.dl_pkt"
#define NETMGR_DATA_FMT_DL_SZE   "persist.data.df.agg.dl_size"
#define NETMGR_DATA_FMT_DL_PAD   "persist.data.df.agg.dl_pad"
#define NETMGR_DATA_FMT_MUX_CNT  "persist.data.df.mux_count"
#define NETMGR_DATA_FMT_IWLAN_MUX "persist.data.df.iwlan_mux"

#define NETMGR_DFLT_ENABLE_RMNET_DATA "0"

#define NETMGR_DATA_FMT_DFLT_LLP 2
#define NETMGR_DATA_FMT_DFLT_DL_FMT    "7"
#define NETMGR_DATA_FMT_DFLT_UL_FMT    "5"
#define NETMGR_DATA_FMT_DFLT_DL_PKT    "1"
#define NETMGR_DATA_FMT_DFLT_DL_SZE    "1504"
#define NETMGR_DATA_FMT_DFLT_DL_PAD    "0"
#define NETMGR_DATA_FMT_DFLT_MUX_CNT   "8"
#define NETMGR_DATA_FMT_DFLT_IWLAN_MUX "9"

#define NETMGR_DATA_FMT_SIZE     6

#define NETMGR_BASE_TEN 10

#ifdef FEATURE_DS_LINUX_ANDROID
  #define NETMGR_LOAD_PROP(X, Y, Z) \
    do { \
      memset(Y, 0, sizeof(Y)); \
      property_get(X, Y, Z); \
      netmgr_log_med("Read property %s; Got \"%s\"; (Default: \"%s\")\n", X, Y, Z);\
    } while(0);
#else
  #define NETMGR_LOAD_PROP(X, Y, Z) \
    do { \
      (void)strlcpy(Y, Z, sizeof(Y)); \
      netmgr_log_med("Read property %s; Using default \"%s\")\n", X, Y);\
    } while(0);
#endif

/*---------------------------------------------------------------------------
   Program configuration info
---------------------------------------------------------------------------*/
struct netmgr_main_cfg_s netmgr_main_cfg = {
  NETMGR_MAIN_RUNMODE_DEFAULT,  /* runmode */
  DS_LOG_MODE_DFLT,             /* logmode */
  -1,                           /* logthreshold */
  -1,                           /* nint */
  netmgr_iname_default,         /* iname */
  NETMGR_KIF_SKIP,              /* skip */
  NULL,                         /* dirpath */
  NULL,                         /* modscr */
  FALSE,                        /* debug */
  FALSE,                        /* runtests */
  FALSE,                        /* initialized */
  NETMGR_LINK_RMNET_0,          /* def_link */
  IWLAN_DISABLE,                /* iwlan_enabled */
  FALSE,                        /* iwlan_ims_enabled */
  FALSE,                        /* rmnet_data_enabled */
  {0,},                         /* phys_net_dev */
  {0,},                         /* modems_enabled */
  FALSE,                        /* single_qmux_ch_enabled */
  {0,},                         /* qmux_ch_name */
  {0,},                         /* smd_ch_name */
  0,                            /* epid */
  DS_EP_TYPE_EMBEDDED,          /* ep_type Default DS_EP_TYPE_EMBEDDED */
  0,                            /* ipa_cons_ep_num */
  0,                            /* ipa_prod_ep_num */
  FALSE,                        /* dpm_enabled */
  FALSE,                        /* wda_data_format */
  FALSE,                        /* tcp_ack_prio */
  FALSE,                        /* in band flow control */
  {0,},                         /* data format */
  -1,                           /* modem ssr state */
  0,                            /* rps mask */
  0,                            /* netdev budget */
  0,                            /* low latency filters */
  TRUE,                         /* Drop SSDP */
  FALSE,                         /* Process ready requests */
  0,                             /* hybrid qos */
  0,                            /* netdev max backlog */
  0,                            /* tc_ul_baserate */
  0                             /* tc_ul_burst */
}; /* Initialize everything to invalid values */

/* one of these files may hold the data control port string */
#define NETMGR_SYSFS_CONFIG_FILE_1 "/sys/module/f_rmnet/parameters/rmnet_ctl_ch"
#define NETMGR_SYSFS_CONFIG_FILE_2 "/sys/module/rmnet/parameters/rmnet_ctl_ch"

/* Array of strings used to identify which port is used by RmNET.
 * Order of strings is important as we use index of the string element
 * internally. First field is for USB driver port match, which may be
 * empty string if unused */
netmgr_ctl_port_config_type
netmgr_ctl_port_array[NETMGR_MAX_LINK+1] =
{
  /* SMD/BAM transport */
  {"DATA5_CNTL",  QMI_PORT_RMNET_0,         NETMGR_LINK_RMNET_0,      TRUE,  TRUE, FALSE, FALSE, FALSE},
  {"DATA6_CNTL",  QMI_PORT_RMNET_1,         NETMGR_LINK_RMNET_1,      FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA7_CNTL",  QMI_PORT_RMNET_2,         NETMGR_LINK_RMNET_2,      FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA8_CNTL",  QMI_PORT_RMNET_3,         NETMGR_LINK_RMNET_3,      FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA9_CNTL",  QMI_PORT_RMNET_4,         NETMGR_LINK_RMNET_4,      FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA12_CNTL", QMI_PORT_RMNET_5,         NETMGR_LINK_RMNET_5,      FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA13_CNTL", QMI_PORT_RMNET_6,         NETMGR_LINK_RMNET_6,      FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA14_CNTL", QMI_PORT_RMNET_7,         NETMGR_LINK_RMNET_7,      FALSE, TRUE, FALSE, FALSE, FALSE},

  /* SDIO/USB transport */
  {"MDM0_CNTL",   QMI_PORT_RMNET_SDIO_0,    NETMGR_LINK_RMNET_8,      TRUE,  TRUE, FALSE, FALSE, FALSE},
  {"MDM1_CNTL",   QMI_PORT_RMNET_SDIO_1,    NETMGR_LINK_RMNET_9,      FALSE, TRUE, FALSE, FALSE, FALSE},
  {"MDM2_CNTL",   QMI_PORT_RMNET_SDIO_2,    NETMGR_LINK_RMNET_10,     FALSE, TRUE, FALSE, FALSE, FALSE},
  {"MDM3_CNTL",   QMI_PORT_RMNET_SDIO_3,    NETMGR_LINK_RMNET_11,     FALSE, TRUE, FALSE, FALSE, FALSE},
  {"MDM4_CNTL",   QMI_PORT_RMNET_SDIO_4,    NETMGR_LINK_RMNET_12,     FALSE, TRUE, FALSE, FALSE, FALSE},
  {"MDM5_CNTL",   QMI_PORT_RMNET_SDIO_5,    NETMGR_LINK_RMNET_13,     FALSE, TRUE, FALSE, FALSE, FALSE},
  {"MDM6_CNTL",   QMI_PORT_RMNET_SDIO_6,    NETMGR_LINK_RMNET_14,     FALSE, TRUE, FALSE, FALSE, FALSE},
  {"MDM7_CNTL",   QMI_PORT_RMNET_SDIO_7,    NETMGR_LINK_RMNET_15,     FALSE, TRUE, FALSE, FALSE, FALSE},

#ifdef FEATURE_DATA_IWLAN
  /* SMD/BAM reverse transport */
  {"DATA23_CNTL", QMI_PORT_REV_RMNET_0,     NETMGR_LINK_REV_RMNET_0,  FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA24_CNTL", QMI_PORT_REV_RMNET_1,     NETMGR_LINK_REV_RMNET_1,  FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA25_CNTL", QMI_PORT_REV_RMNET_2,     NETMGR_LINK_REV_RMNET_2,  FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA26_CNTL", QMI_PORT_REV_RMNET_3,     NETMGR_LINK_REV_RMNET_3,  FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA27_CNTL", QMI_PORT_REV_RMNET_4,     NETMGR_LINK_REV_RMNET_4,  FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA28_CNTL", QMI_PORT_REV_RMNET_5,     NETMGR_LINK_REV_RMNET_5,  FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA29_CNTL", QMI_PORT_REV_RMNET_6,     NETMGR_LINK_REV_RMNET_6,  FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA30_CNTL", QMI_PORT_REV_RMNET_7,     NETMGR_LINK_REV_RMNET_7,  FALSE, TRUE, FALSE, FALSE, FALSE},
  {"DATA31_CNTL", QMI_PORT_REV_RMNET_8,     NETMGR_LINK_REV_RMNET_8,  FALSE, TRUE, FALSE, FALSE, FALSE},

  /* USB reverse transport */
  {"RMDM0_CNTL",  QMI_PORT_REV_RMNET_USB_0, NETMGR_LINK_REV_RMNET_9,  FALSE, TRUE, FALSE, FALSE, FALSE},
  {"RMDM1_CNTL",  QMI_PORT_REV_RMNET_USB_1, NETMGR_LINK_REV_RMNET_10, FALSE, TRUE, FALSE, FALSE, FALSE},
  {"RMDM2_CNTL",  QMI_PORT_REV_RMNET_USB_2, NETMGR_LINK_REV_RMNET_11, FALSE, TRUE, FALSE, FALSE, FALSE},
  {"RMDM3_CNTL",  QMI_PORT_REV_RMNET_USB_3, NETMGR_LINK_REV_RMNET_12, FALSE, TRUE, FALSE, FALSE, FALSE},
  {"RMDM4_CNTL",  QMI_PORT_REV_RMNET_USB_4, NETMGR_LINK_REV_RMNET_13, FALSE, TRUE, FALSE, FALSE, FALSE},
  {"RMDM5_CNTL",  QMI_PORT_REV_RMNET_USB_5, NETMGR_LINK_REV_RMNET_14, FALSE, TRUE, FALSE, FALSE, FALSE},
  {"RMDM6_CNTL",  QMI_PORT_REV_RMNET_USB_6, NETMGR_LINK_REV_RMNET_15, FALSE, TRUE, FALSE, FALSE, FALSE},
  {"RMDM7_CNTL",  QMI_PORT_REV_RMNET_USB_7, NETMGR_LINK_REV_RMNET_16, FALSE, TRUE, FALSE, FALSE, FALSE},
  {"RMDM8_CNTL",  QMI_PORT_REV_RMNET_USB_8, NETMGR_LINK_REV_RMNET_17, FALSE, TRUE, FALSE, FALSE, FALSE},
#endif /* FEATURE_DATA_IWLAN */

  /* Must be last record for validation */
  {"",            "",                       NETMGR_LINK_MAX,          FALSE, FALSE, FALSE, FALSE, FALSE}
};

/* Table of transport device name prefix per Modem */
#ifdef FEATURE_DATA_IWLAN
  netmgr_main_dev_prefix_type netmgr_main_dev_prefix_tbl[NETMGR_MAX_MODEMS][NETMGR_MAX_LINK_TYPES] =
  {
    {
      { NETMGR_MAIN_RMNET_SMD_PREFIX,      NETMGR_LINK_RMNET_0,      NETMGR_LINK_RMNET_7      },
      { NETMGR_MAIN_REV_RMNET_SMD_PREFIX,  NETMGR_LINK_REV_RMNET_0,  NETMGR_LINK_REV_RMNET_8  },
    },
    {
      { NETMGR_MAIN_RMNET_SDIO_PREFIX,     NETMGR_LINK_RMNET_8,      NETMGR_LINK_RMNET_15     },
      { NETMGR_MAIN_REV_RMNET_USB_PREFIX,  NETMGR_LINK_REV_RMNET_9,  NETMGR_LINK_REV_RMNET_17 },
    }
  };
#else
  netmgr_main_dev_prefix_type netmgr_main_dev_prefix_tbl[] =
  {
    { NETMGR_MAIN_RMNET_SMD_PREFIX,  NETMGR_LINK_RMNET_0,  NETMGR_LINK_RMNET_7  },
    { NETMGR_MAIN_RMNET_SDIO_PREFIX, NETMGR_LINK_RMNET_8,  NETMGR_LINK_RMNET_15 },

    /* This must be the last entry in the table */
    { "",                            NETMGR_LINK_NONE,     NETMGR_LINK_NONE     }
  };
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_signal_handler
===========================================================================*/
/*!
@brief
  Callback registered as OS signal handler.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Toggles generation of debug messages
*/
/*=========================================================================*/
LOCAL void netmgr_signal_handler( int sig )
{
  int i = 0;

  switch (sig)
  {
  case SIGUSR1:
    /* On USR1 signal, toggle the debug flags */
    netmgr_main_cfg.debug = (netmgr_main_cfg.debug)? FALSE : TRUE;
    function_debug = (function_debug)? FALSE : TRUE;

    netmgr_log_med("Signal Handler - Setting debug flag: %d\n",netmgr_main_cfg.debug);
    netmgr_log_med("Runmode: 0x%x\n", netmgr_main_cfg.runmode);
    {
      /* Display security credentials */
      struct __user_cap_data_struct cap_data;
      struct __user_cap_header_struct cap_hdr;
      cap_hdr.version = _LINUX_CAPABILITY_VERSION;
      cap_hdr.pid = 0; /* 0 is considered self pid */
      (void)capget(&cap_hdr, &cap_data);
      netmgr_log_med("Running as: uid[%d] gid[%d] caps_perm/eff[0x%x/0x%x]\n",
                     getuid(), getgid(), cap_data.permitted, cap_data.effective);
    }

    /* Print netmgr configuration */
    netmgr_config_print();

    /* Print KIF information */
    netmgr_kif_print_state();

    /* Dump DPM related data channel information */
    netmgr_log_med("DPM enabled %d, epid %d, consumer pipe %d, producer pipe %d\n",
        netmgr_main_cfg.dpm_enabled,
        netmgr_main_cfg.epid,
        netmgr_main_cfg.consumer_pipe_num,
        netmgr_main_cfg.producer_pipe_num);
    netmgr_qmi_dpm_log_state();

    break;

  case SIGTERM:
    /* On TERM signal, exit() so atexit cleanup functions gets called */
    exit(0);
    break;

  default:
    break;
  }
}

/*===========================================================================
  FUNCTION  netmgr_main_get_qos_enabled
===========================================================================*/
/*!
@brief
  Return value for QOS enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline int netmgr_main_get_qos_enabled( void )
{
  return (NETMGR_MAIN_RUNMODE_QOSHDR ==
          (netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_QOSHDR));
}

/*===========================================================================
  FUNCTION  netmgr_main_get_qos_hybrid_enabled
===========================================================================*/
/*!
@brief
  Return value for QOS hybrid enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline int netmgr_main_get_qos_hybrid_enabled( void )
{
  return netmgr_main_cfg.hybrid_qos;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_main_get_iwlan_enabled
===========================================================================*/
/*!
@brief
  Return value for iWLAN enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline iwlan_state_t netmgr_main_get_iwlan_enabled( void )
{
  return netmgr_main_cfg.iwlan_enabled;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_iwlan_ims_enabled
===========================================================================*/
/*!
@brief
  Return value for iWLAN IMS enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline int netmgr_main_get_iwlan_ims_enabled( void )
{
  return netmgr_main_cfg.iwlan_ims_enabled;
}
#endif /* FEATURE_DATA_IWLAN */

#ifdef FEATURE_DS_LINUX_ANDROID
/*===========================================================================
  FUNCTION  netmgr_main_check_tcpackprio_enabled
===========================================================================*/
/*!
@brief
  Return value for TCP_ACK_PRIO enabled configuration item and sets the
  netmgr configuration property.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void netmgr_main_check_tcpackprio_enabled( void )
{
  char args[PROPERTY_VALUE_MAX];
  int ret;

  /* Retrieve value of TCP_ACK_PRIO */
  NETMGR_LOG_FUNC_ENTRY;
  memset(args, 0, sizeof(args));
  ret = property_get(NETMGR_MAIN_PROPERTY_TCPACKPRIO,
                     args,
                     NETMGR_MAIN_PROPERTY_TCPACKPRIO_DEFAULT);

  if (!strncmp(NETMGR_TRUE, args, sizeof(NETMGR_TRUE)))
  {
     netmgr_main_cfg.tcp_ack_prio = TRUE;
  }
  netmgr_log_med("property [%s] value[%s]",
                 NETMGR_MAIN_PROPERTY_TCPACKPRIO, args);
}

/*===========================================================================
  FUNCTION  netmgr_main_check_dropssdp_enabled
===========================================================================*/
/*!
@brief
  Return value for DROP SSDP enabled configuration item and sets the
  netmgr configuration property.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_check_dropssdp_enabled( void )
{
  char args[PROPERTY_VALUE_MAX];
  int ret;

  /* Retrieve value of DROPSSDP */
  NETMGR_LOG_FUNC_ENTRY;
  memset(args, 0, sizeof(args));
  ret = property_get(NETMGR_MAIN_PROPERTY_DROPSSDP,
                     args,
                     NETMGR_MAIN_PROPERTY_DROPSSDP_DEFAULT);

  if (!strncmp(NETMGR_FALSE, args, sizeof(NETMGR_FALSE)))
  {
     netmgr_main_cfg.dropssdp = FALSE;
  }
  netmgr_log_med("property [%s] value[%s]",
                 NETMGR_MAIN_PROPERTY_DROPSSDP, args);
  return netmgr_main_cfg.dropssdp;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_tcpackprio_enabled
===========================================================================*/
/*!
@brief
  Return value for TCP_ACK_PRIO enabled configuration item from the netmgr
  configuration property.

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_tcpackprio_enabled( void )
{
  return netmgr_main_cfg.tcp_ack_prio;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_tc_ul_baserate
===========================================================================*/
/*!
@brief
  Return value for TC_BASE_DATARATE enabled configuration item from the netmgr
  configuration property.

@return
  int -

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
unsigned long netmgr_main_get_tc_ul_baserate( void )
{
  return netmgr_main_cfg.tc_ul_baserate;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_tc_ul_burst
===========================================================================*/
/*!
@brief
  Return value for TC_UL_BURST enabled configuration item from the netmgr
  configuration property.

@return
  int -

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
unsigned long netmgr_main_get_tc_ul_burst( void )
{
  return netmgr_main_cfg.tc_ul_burst;
}

/*===========================================================================
  FUNCTION  netmgr_main_check_ibfc_enabled
===========================================================================*/
/*!
@brief
  Return value for IBFC enabled configuration item and sets the
  netmgr configuration property.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void netmgr_main_check_ibfc_enabled( void )
{
  char args[PROPERTY_VALUE_MAX];
  int ret;

  /* Retrieve value of IBFC */
  NETMGR_LOG_FUNC_ENTRY;
  memset(args, 0, sizeof(args));
  ret = property_get(NETMGR_MAIN_PROPERTY_IBFC,
                     args,
                     NETMGR_MAIN_PROPERTY_IBFC_DEFAULT);

  if (!strncmp(NETMGR_TRUE, args, sizeof(NETMGR_TRUE)))
  {
     netmgr_main_cfg.ibfc = TRUE;
  }
  netmgr_log_med("property [%s] value[%s]",
                 NETMGR_MAIN_PROPERTY_IBFC, args);
}

/*===========================================================================
  FUNCTION  netmgr_main_check_low_latency_filters_enabled
===========================================================================*/
/*!
@brief
  Return value for LLF enabled configuration item and sets the
  netmgr configuration property.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void netmgr_main_check_low_latency_filters_enabled(void)
{
  char args[PROPERTY_VALUE_MAX];
  int ret;

  /* Retrieve value of LLF */
  NETMGR_LOG_FUNC_ENTRY;
  memset(args, 0, sizeof(args));
  ret = property_get(NETMGR_MAIN_PROPERTY_LOW_LATENCY_FILTERS,
                     args,
                     NETMGR_MAIN_PROPERTY_LOW_LATENCY_FILTERS_DEFAULT);

  if (!strncmp(NETMGR_TRUE, args, sizeof(NETMGR_TRUE)))
  {
     netmgr_main_cfg.low_latency_filters = TRUE;
  }
  netmgr_log_med("property [%s] value[%s]",
                 NETMGR_MAIN_PROPERTY_LOW_LATENCY_FILTERS, args);
}
#endif /* FEATURE_DS_LINUX_ANDROID */

/*===========================================================================
  FUNCTION  netmgr_main_get_ibfc_enabled
===========================================================================*/
/*!
@brief
  Return value for IBFC enabled configuration item from the netmgr
  configuration property.

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_ibfc_enabled( void )
{
  return netmgr_main_cfg.ibfc;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_rmnet_data_enabled
===========================================================================*/
/*!
@brief
  Return value for rmnet data driver enabled configuraition item

@return
  int - TRUE/FALSE
*/
/*=========================================================================*/
inline int netmgr_main_get_rmnet_data_enabled( void )
{
  return netmgr_main_cfg.rmnet_data_enabled;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_phys_net_dev
===========================================================================*/
/*!
@brief
  Return value for physical network device if any.

@return
  char pointer to physical network device name
  NULL if none such device exists.
*/
/*=========================================================================*/
inline char *netmgr_main_get_phys_net_dev( void )
{
  return netmgr_main_cfg.phys_net_dev;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_dpm_enabled
===========================================================================*/
/*!
@brief
  Return value for Data Port Mapper enablement

@return
  int - TRUE/FALSE
*/
/*=========================================================================*/
inline int netmgr_main_get_dpm_enabled( void )
{
  return netmgr_main_cfg.dpm_enabled;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_low_latency_filters_enabled
===========================================================================*/
/*!
@brief
  Return value for low latency filters through QMI DFS enablement

@return
  int - TRUE/FALSE
*/
/*=========================================================================*/
inline int netmgr_main_get_low_latency_filters_enabled( void )
{
  return netmgr_main_cfg.low_latency_filters;
}

/*===========================================================================
  FUNCTION  netmgr_main_process_arg
===========================================================================*/
/*!
@brief
  Populates program configuration information for the specified argument and
  argument value.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
*/
/*=========================================================================*/
void
netmgr_main_process_arg(char argid, char * argval)
{
  switch (argid) {
    case 'b':
      /* run in background, i.e. run as a forked daemon process */
      netmgr_main_cfg.runmode |= NETMGR_MAIN_RUNMODE_BACK;
      fprintf(stderr, "running in background process\n");
      break;
    case 'E':
      /* use Ethernet link protocol */
      netmgr_main_cfg.runmode |= NETMGR_MAIN_RUNMODE_ETHERNET;
      fprintf(stderr, "using Ethernet link protocol\n");
      break;
#ifdef NETMGR_QOS_ENABLED
    case 'Q':
      /* use RmNET QoS header prepended to TX packets */
      netmgr_main_cfg.runmode |= NETMGR_MAIN_RUNMODE_QOSHDR;
      fprintf(stderr, "QOS enabled, using QMI header\n");
      break;
#endif /* NETMGR_QOS_ENABLED */
    case 's':
      /* Log to syslog. By default program will log to stderr */
      netmgr_main_cfg.logmode = (int)DS_LOG_MODE_SYSLOG;
      fprintf(stderr, "using syslog\n");
      break;
    case 'l':
      /* Logging threshold as an integer value */
      netmgr_main_cfg.logthreshold = ds_atoi(argval);
      fprintf(stderr, "using log level %d\n", ds_atoi(argval));
      break;
    case 'n':
      /* Number of interfaces to create */
      netmgr_main_cfg.nint = ds_atoi(argval);
      fprintf(stderr, "cfging %d interfaces\n", ds_atoi(argval));
      break;
    case 'i':
      /* Interface name to use */
      netmgr_main_cfg.iname = argval;
      fprintf(stderr, "using interface name %s\n", argval);
      break;
    case 'k':
      /* Load kernel driver module and DHCP client */
      netmgr_main_cfg.skip = NETMGR_KIF_LOAD;
      fprintf(stderr, "perform module load\n");
      break;
    case 'd':
      /* Directory pathname to search for script files */
      netmgr_main_cfg.dirpath = argval;
      fprintf(stderr, "using relative path %s\n", argval);
      break;
    case 'm':
      /* Name of driver module load script */
      netmgr_main_cfg.modscr = argval;
      fprintf(stderr, "using module load script %s\n", argval);
      break;
    case 'D':
      /* Verbose debug flag */
      netmgr_main_cfg.debug = TRUE;
      function_debug = TRUE;
      fprintf(stderr, "setting debug mode.\n");
      break;
    case 'T':
      /* Execute internal tests flag */
      netmgr_main_cfg.runtests = TRUE;
      fprintf(stderr, "setting runtests mode.\n");
      break;
    default:
      /* Ignore unknown argument */
      fprintf(stderr, "ignoring unknown arg '%c'\n", argid);
      break;
  }
  return;
}

/*===========================================================================
  FUNCTION  netmgr_main_parse_args
===========================================================================*/
/*!
@brief
  Parses all specified program command line arguments and populates
  configuration information.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
netmgr_main_parse_args (int argc, char ** argv)
{
  int i;
  char a;

  NETMGR_LOG_FUNC_ENTRY;

  for (i = 1; i < argc; ++i) {
    if (std_strlen(argv[i]) < 2) {
      /* Minimum length of a valid argument is 2, as each arg is
      ** prefixed by a '-' char.
      */
      continue;
    }

    if (*argv[i] != '-') {
      /* Every valid arg should begin with a '-' */
      continue;
    }

    /* Get next char in argument, which is the arg type */
    a = *(argv[i] + 1);

    /* Process based on type of argument */
    switch (a) {
      case 'l':
      case 'n':
      case 'i':
      case 'd':
      case 'm':
      case 'u':
      case 't':
        /* These arguments have an associated value immediately following
        ** the argument.
        */
        if (++i < argc) {
          netmgr_main_process_arg(a, argv[i]);
        }
        break;
      case 'b':
      case 'E':
      case 'Q':
      case 's':
      case 'k':
      case 'D':
      case 'T':
        /* These arguments do not have any value following the argument */
        netmgr_main_process_arg(a, 0);
        break;
      default:
        /* Everything else is an unknown arg that is ignored */
        fprintf(stderr, "unknown arg %s specified\n", argv[i]);
    }
  }

#if 0
  /* Verify Ethernet and QoS modes are not specified simultaneously;
   * this is currently not supported in Linux RmNET driver
   * (due to insufficent skb headroom) */
  if( (netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_ETHERNET) &&
      (netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_QOSHDR) ) {
    NETMGR_STOP("Ethernet protocol with QoS Header not supproted!!");
  }
#endif

  NETMGR_LOG_FUNC_EXIT;
  return;
}


/*===========================================================================
  FUNCTION  netmgr_main_reset_links
===========================================================================*/
/*!
@brief
  selects all the links/interfaces for use by NetMgr. Typically,
  this is the default behavior unless another subsystem (e.g.
  USB rmnet) wanted to use one of the default SMD ports.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline void
netmgr_main_reset_links(void)
{
  int i=0;

#ifdef FEATURE_DS_LINUX_ANDROID
  char  args[PROPERTY_VALUE_MAX];
  char  def[NETMGR_MAIN_PROPERTY_NINT_SIZE];
  int   ret;

  /* Query Android property database to see if nint set */
  snprintf( def, sizeof(def), "%d", NETMGR_MAIN_DEFAULT_NINT );
  ret = property_get( NETMGR_MAIN_PROPERTY_NINT, args, def );
  if( (NETMGR_MAIN_PROPERTY_NINT_SIZE-1) < ret ) {
    netmgr_log_err( "System property %s has unexpected size(%d), skippng\n",
                    NETMGR_MAIN_PROPERTY_NINT, ret );
  } else {
    ret = ds_atoi( args );
    if( NETMGR_MAX_LINK < ret ) {
      netmgr_log_err( "System property %s has exceeded limit (%d), skippng\n",
                      NETMGR_MAIN_PROPERTY_NINT, NETMGR_MAX_LINK );
    } else {
      /* Update number of active interfaces */
      netmgr_log_high( "System property %s set (%d)\n",
                       NETMGR_MAIN_PROPERTY_NINT, ret );
      netmgr_main_cfg.nint = ret;
    }
  }
#endif

  netmgr_log_high("netmgr_main_reset_links: " \
                  "reset netmgr_main_nint to %d",
                  netmgr_main_cfg.nint);

  /* Initialize link state table; this may be updated later in
   * netmgr_main_update_links() */
  for(i=0; i<NETMGR_MAX_LINK; i++)
  {
    netmgr_ctl_port_array[i].enabled = TRUE;
  }
}

/*===========================================================================
  FUNCTION  netmgr_read_data_ctl_port
===========================================================================*/
/*!
@brief
  Reads a data control port string - one at a time

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_read_data_ctl_port
(
  int fd,
  char * buf
)
{
  ssize_t bytes_read = 0, temp = 0;
  char ch;

  NETMGR_LOG_FUNC_ENTRY;

  if (buf == NULL)
    return 0;

  do
  {
    temp = read(fd, &ch, 1);

    if (temp)
    {
      /* we only care about alphanumeric chars
         and '_'; rest are ignored */
      if (('0' <= ch && ch <= '9') ||
          ('a' <= ch && ch <= 'z') ||
          ('A' <= ch && ch <= 'Z') ||
          ('_' == ch))
      {
        bytes_read += temp;
        *buf++ = ch;
        *buf = '\0';
        /* read the delimiter if we reached max len */
        if( NETMGR_CFG_PARAM_LEN == bytes_read )
        {
          read(fd, &ch, 1);
          break;
        }
      }
      else
      {
        break;
      }
    }

  } while ( bytes_read != 0 );

  NETMGR_LOG_FUNC_EXIT;
  return (int)bytes_read;
}

/*===========================================================================
  FUNCTION  netmgr_read_sysfs_config
===========================================================================*/
/*!
@brief
  Reads from syscfg files to determine which data control ports are
  in-use by other subsystems (e.g. USB)

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void netmgr_read_sysfs_config(void)
{
  int fd = 0, i = 0, bytes_read = 0;
  /* buffer to hold the value read */
  char buffer[NETMGR_CFG_PARAM_LEN+1];

  NETMGR_LOG_FUNC_ENTRY;

  fd = open(NETMGR_SYSFS_CONFIG_FILE_1, O_RDONLY);
  if (fd < 0)
  {
    netmgr_log_err( "couldn't open file %s\n",
                    NETMGR_SYSFS_CONFIG_FILE_1 );
    /* if previous file doesn't exist, try another file */
    fd = open(NETMGR_SYSFS_CONFIG_FILE_2, O_RDONLY);
    if (fd < 0)
    {
      netmgr_log_err( "couldn't open %s\n",
                      NETMGR_SYSFS_CONFIG_FILE_2 );
      return;
    }
  }

  /* read value into local buffer */
  do
  {
    /* reset buffer */
    memset( buffer, 0x0, sizeof(buffer) );

    /* read next data control port from the config file into buffer  */
    bytes_read = netmgr_read_data_ctl_port( fd, buffer );

    if (!bytes_read)
    {
      netmgr_log_med( "no more data control ports found in cfg file\n" );
      break;
    }
    else
    {
      netmgr_log_high( "data control port %s found in cfg file\n",
                       buffer );
    }

    /* go through list of data control ports and disable
       the one we just read in buffer */
    for(i=0; i<NETMGR_MAX_LINK; i++)
    {
      /*
         if a match found, disable corresponding qmi_conn_id
         as it might be used by some other module (like usb rmnet)
      */
      if( !strcmp( netmgr_ctl_port_array[i].data_ctl_port,
                   buffer ) )
      {
        netmgr_ctl_port_array[i].initialized = FALSE;
        netmgr_log_high( "link %d will not be used\n", i);
      }
    }

  } while (TRUE); /* infinite loop */

  NETMGR_LOG_FUNC_EXIT;
  return;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_main_disable_modem_reverse_links
===========================================================================*/
/*!
@brief
 This function disables all the reverse links in the given modem.

@return
  void

*/
/*=========================================================================*/
LOCAL void netmgr_main_disable_modem_reverse_links
(
  int  modem
)
{
  int i;
  int modem_start_index = NETMGR_MAIN_GET_INST_MIN_REV(modem);
  int modem_end_index = NETMGR_MAIN_GET_INST_MAX_REV(modem);

  netmgr_log_low("disabling modem [%d] reverse ports start=[%d], end=[%d]",
                 modem,
                 modem_start_index,
                 modem_end_index);

  for (i = modem_start_index; i <= modem_end_index; i++)
  {
    netmgr_ctl_port_array[i].enabled = FALSE;
  }
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION  netmgr_main_disable_modem_links
===========================================================================*/
/*!
@brief
 This function disables all the links in the modem which is disabled.
 Which modem is enabled/disabled is determined by target configuration

@return
  void

*/
/*=========================================================================*/
LOCAL void netmgr_main_disable_modem_links
(
  int   *modems_enabled,
  int   num_modems
)
{
  int i;
  int j;
  int modem_start_index;
  int modem_end_index;

  ds_assert(modems_enabled != NULL);
  ds_assert(((num_modems >= 0) && (num_modems <= NETMGR_MAX_MODEMS)));

  for(i=0; i < num_modems; i++)
  {
    netmgr_log_low("modem_enable[%d]=[%d]", i, modems_enabled[i]);
    if (TRUE != modems_enabled[i])
    {
      /* Disable the forward links on the modem */
      modem_start_index = NETMGR_MAIN_GET_INST_MIN(i);
      modem_end_index = NETMGR_MAIN_GET_INST_MAX(i);

      netmgr_ctl_port_array[modem_start_index].modem_wait = FALSE;

      netmgr_log_low("disabling modem [%d] forward ports start=[%d], end=[%d]",
                     i,
                     modem_start_index,
                     modem_end_index);

      for (j = modem_start_index; j <= modem_end_index; j++)
      {
        netmgr_ctl_port_array[j].enabled = FALSE;
      }

#ifdef FEATURE_DATA_IWLAN
      /* Disable all reverse links on the modem */
      netmgr_main_disable_modem_reverse_links(i);
#endif /* FEATURE_DATA_IWLAN */
    }
  }
}


#ifdef FEATURE_DS_LINUX_ANDROID
#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_main_process_iwlan_enabled
===========================================================================*/
/*!
@brief
 This function disables all the reverse links in the given modem.

@return
  void

*/
/*=========================================================================*/
LOCAL void netmgr_main_process_iwlan_enabled
(
  int   *modems_enabled,
  int   num_modems
)
{
#define NSWO_IWLAN_ONLY "nswo"

  int i, ret;
  char args[PROPERTY_VALUE_MAX];
  iwlan_state_t is_iwlan_enabled = IWLAN_DISABLE;
  boolean is_iwlan_ims_enabled = FALSE;

  if (!modems_enabled || (num_modems < 0) || (num_modems > NETMGR_MAX_MODEMS)) {
    netmgr_log_err("invalid parameters\n");
    return;
  }

  /* Retrieve value of NETMGR_MAIN_PROPERTY_IWLAN */
  memset(args, 0, sizeof(args));

  ret = property_get( NETMGR_MAIN_PROPERTY_IWLAN, args, NETMGR_MAIN_PROPERTY_IWLAN_DEFAULT );

  if (ret > NETMGR_MAIN_PROPERTY_IWLAN_SIZE)
  {
    netmgr_log_err("property [%s] has size [%d] that exceeds max [%d]\n",
                   NETMGR_MAIN_PROPERTY_IWLAN,
                   ret,
                   NETMGR_MAIN_PROPERTY_IWLAN_SIZE);
  }
  else
  {
    netmgr_log_med("property [%s] value[%s]", NETMGR_MAIN_PROPERTY_IWLAN, args);

    if( !strncasecmp( NETMGR_TRUE, args, sizeof(NETMGR_TRUE) ) )
    {
      is_iwlan_enabled = IWLAN_ENABLE;
    }

    if(!strncasecmp( NSWO_IWLAN_ONLY, args, sizeof(NSWO_IWLAN_ONLY) ) )
    {
      is_iwlan_enabled = NSWO_ONLY;
    }
  }

  /* Disable reverse links on all enabled modems */
  if (IWLAN_DISABLE == is_iwlan_enabled)
  {
    for (i = 0; i < num_modems; i++)
    {
      netmgr_log_low("modem_enable[%d]=[%d]", i, modems_enabled[i]);
      if (TRUE == modems_enabled[i])
      {
        /* Disable all reverse links on the modem */
        netmgr_main_disable_modem_reverse_links(i);
      }
    }
  }
  else
  {
    is_iwlan_ims_enabled = TRUE;

    /* Retrieve value of NETMGR_MAIN_PROPERTY_IWLAN_IMS */
    memset(args, 0, sizeof(args));

    ret = property_get( NETMGR_MAIN_PROPERTY_IWLAN_IMS, args, NETMGR_MAIN_PROPERTY_IWLAN_IMS_DEFAULT );

    if (ret > NETMGR_MAIN_PROPERTY_IWLAN_IMS_SIZE)
    {
      netmgr_log_err("property [%s] has size [%d] that exceeds max [%d]\n",
                     NETMGR_MAIN_PROPERTY_IWLAN_IMS,
                     ret,
                     NETMGR_MAIN_PROPERTY_IWLAN_IMS_SIZE);
    }
    else
    {
      netmgr_log_med("property [%s] value[%s]", NETMGR_MAIN_PROPERTY_IWLAN_IMS, args);

      if( !strncasecmp( NETMGR_FALSE, args, sizeof(NETMGR_FALSE) ) )
      {
        is_iwlan_ims_enabled = FALSE;
      }
    }
  }

  netmgr_main_cfg.iwlan_enabled = is_iwlan_enabled;
  netmgr_main_cfg.iwlan_ims_enabled = is_iwlan_ims_enabled;
}
#endif /* FEATURE_DATA_IWLAN */
#endif /* FEATURE_DS_LINUX_ANDROID */

/*===========================================================================
  FUNCTION  netmgr_main_process_target
===========================================================================*/
/*!
@brief
  Updates netmgr links based on current target configuration.

@return
  void

*/
/*=========================================================================*/
LOCAL void netmgr_main_process_target()
{
  int i = 0;
  int j = 0;
  int ret = 0;
  int modem_base_index = 0;
  char *prefix = NULL;
  ds_target_t target;
  const char *target_str;

  target = ds_get_target();
  target_str = ds_get_target_str(target);

  netmgr_log_med("ds_target is set to: %d [%s]", target, target_str);
  memset(netmgr_main_cfg.modem_enable, 0, sizeof(netmgr_main_cfg.modem_enable));

  if (DS_TARGET_MSM == target)
  {
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=TRUE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=FALSE;
    netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
    strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET, sizeof(netmgr_main_cfg.phys_net_dev));
  }
  else if (DS_TARGET_APQ == target)
  {
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=FALSE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=FALSE;
    netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
  }
  else if (DS_TARGET_SVLTE1 == target)
  {
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=TRUE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=TRUE;
    netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MDM);
  }
  else if (DS_TARGET_SVLTE2 == target)
  {
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=TRUE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=TRUE;
    netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MDM);
  }
  else if (DS_TARGET_CSFB == target)
  {
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=FALSE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=TRUE;
    netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MDM);
  }
  else if (DS_TARGET_MDM == target)
  {
    int index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MDM);

    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=FALSE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=TRUE;
    netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MDM);
    strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET_USB, sizeof(netmgr_main_cfg.phys_net_dev));

    /* Update the device prefix string based on whether
       rmnet_data or rmnet_usb is enabled */
    if (netmgr_main_get_rmnet_data_enabled())
    {
      netmgr_rmnet_set_device_qmi_offset(netmgr_main_cfg.def_link);
      strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MDM),
                   NETMGR_MAIN_RMNET_DATA_PREFIX,
                   NETMGR_IF_NAME_MAX_LEN );

#ifdef FEATURE_DATA_IWLAN
      /* Change the prefix for the reverse ports */
      strlcpy(netmgr_main_dev_prefix_tbl[NETMGR_MODEM_MDM][NETMGR_REV_LINK].prefix,
              NETMGR_MAIN_REV_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
#endif /* FEATURE_DATA_IWLAN */
    }
    else
    {
      strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MDM),
                   NETMGR_MAIN_RMNET_USB_PREFIX,
                   NETMGR_IF_NAME_MAX_LEN );
    }

    /* Replace SDIO transport with USB */
    strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
                 QMI_PORT_RMNET_USB_0,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
                 QMI_PORT_RMNET_USB_1,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
                 QMI_PORT_RMNET_USB_2,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
                 QMI_PORT_RMNET_USB_3,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
                 QMI_PORT_RMNET_USB_4,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
                 QMI_PORT_RMNET_USB_5,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
                 QMI_PORT_RMNET_USB_6,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
                 QMI_PORT_RMNET_USB_7,
                 NETMGR_CFG_CONNID_LEN );
  }
  else if (DS_TARGET_DSDA == target)
  {
    int index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MDM);

    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=TRUE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=TRUE;
    netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MDM);
    strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET_USB, sizeof(netmgr_main_cfg.phys_net_dev));

    /* Replace SDIO transport with USB */
    strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MDM),
             NETMGR_MAIN_RMNET_USB_PREFIX,
             NETMGR_IF_NAME_MAX_LEN );

    strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_0,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_1,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_2,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_3,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_4,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_5,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_6,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_7,
             NETMGR_CFG_CONNID_LEN );

    /* Replace MSM ports with SMUX port */
    index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);

    strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
             NETMGR_MAIN_RMNET_SMUX_PREFIX,
             NETMGR_IF_NAME_MAX_LEN );

    strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
             QMI_PORT_RMNET_SMUX_0,
             NETMGR_CFG_CONNID_LEN );

    netmgr_ctl_port_array[ index+1 ].enabled = FALSE;
    netmgr_ctl_port_array[ index+2 ].enabled = FALSE;
    netmgr_ctl_port_array[ index+3 ].enabled = FALSE;
    netmgr_ctl_port_array[ index+4 ].enabled = FALSE;
    netmgr_ctl_port_array[ index+5 ].enabled = FALSE;
    netmgr_ctl_port_array[ index+6 ].enabled = FALSE;
    netmgr_ctl_port_array[ index+7 ].enabled = FALSE;

  }
  else if (DS_TARGET_DSDA2 == target)
  {
    int index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);

    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=TRUE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=TRUE;
    netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
    strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET, sizeof(netmgr_main_cfg.phys_net_dev));

    /* Replace first modem ports with USB */
    strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
             NETMGR_MAIN_RMNET_USB_PREFIX,
             NETMGR_IF_NAME_MAX_LEN );

    strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_0,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_1,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_2,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_3,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_4,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_5,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_6,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
             QMI_PORT_RMNET_USB_7,
             NETMGR_CFG_CONNID_LEN );

    /* Replace second modem ports with 2 modem USB */
    index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MDM);

    strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MDM),
             NETMGR_MAIN_RMNET2_USB_PREFIX,
             NETMGR_IF_NAME_MAX_LEN );

    strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
             QMI_PORT_RMNET2_USB_0,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
             QMI_PORT_RMNET2_USB_1,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
             QMI_PORT_RMNET2_USB_2,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
             QMI_PORT_RMNET2_USB_3,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
             QMI_PORT_RMNET2_USB_4,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
             QMI_PORT_RMNET2_USB_5,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
             QMI_PORT_RMNET2_USB_6,
             NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
             QMI_PORT_RMNET2_USB_7,
             NETMGR_CFG_CONNID_LEN );
  }
  else if (DS_TARGET_DSDA3 == target || DS_TARGET_SGLTE == target)
  {
    int index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MDM);

    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=TRUE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=TRUE;
    netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
    strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET, sizeof(netmgr_main_cfg.phys_net_dev));

    /* Replace SDIO transport with USB */
    strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MDM),
             NETMGR_MAIN_RMNET_SMUX_PREFIX,
             NETMGR_IF_NAME_MAX_LEN );

    strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
             QMI_PORT_RMNET_SMUX_0,
             NETMGR_CFG_CONNID_LEN );
  }
  else if (DS_TARGET_FUSION4_5_PCIE == target)
  {
    netmgr_log_med("netmgrd: Fusion4.5 configuration, using configdb");
    if (0 != netmgr_config_load(NETMGR_CONFIG_FILE, NETMGR_PROPERTY_DATA_TARGET_VALUE_FUSION4_5_PCIE))
    {
      netmgr_log_err("netmgrd: Configuration using %s [%s] failed, reverting to defaults",
          NETMGR_CONFIG_FILE, NETMGR_PROPERTY_DATA_TARGET_VALUE_FUSION4_5_PCIE);

      int index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
      netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=TRUE;
      netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=FALSE;

      netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
      strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET, sizeof(netmgr_main_cfg.phys_net_dev));

      /* Replace multiple control channels with single control channel */
      strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
                   NETMGR_MAIN_RMNET_DATA_PREFIX,
                   NETMGR_IF_NAME_MAX_LEN );
      netmgr_main_cfg.rmnet_data_enabled = TRUE;
      strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET_MHI, sizeof(netmgr_main_cfg.phys_net_dev));
      netmgr_main_cfg.wda_data_format = TRUE;
      netmgr_main_cfg.data_format.dl_data_aggregation_protocol = 7;

      netmgr_main_cfg.single_qmux_ch_enabled = TRUE;
      strlcpy(netmgr_main_cfg.qmux_ch_name, QMI_PORT_RMNET_MHI_0, sizeof(netmgr_main_cfg.qmux_ch_name));
      strlcpy(netmgr_main_cfg.smd_ch_name, "MHICTL0", sizeof(netmgr_main_cfg.smd_ch_name));

      strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_0,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_1,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_2,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_3,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_4,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_5,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_6,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_7,
               NETMGR_CFG_CONNID_LEN );

#ifdef FEATURE_DATA_IWLAN
      /* We will be setting persist.data.iwlan.enable property to true
       * by default in the config file * If reading from the config file
       * fails for some reason, we will default it to true */
      netmgr_main_cfg.iwlan_enabled = IWLAN_ENABLE;

      /* Change prefix for reverse links */
      strlcpy(NETMGR_MAIN_GET_DEV_REV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_REV_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);

      /* Get starting index of reverse rmnet ports */
      index = NETMGR_MAIN_GET_INST_MIN_REV(NETMGR_MODEM_MSM);

      strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_0,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_1,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_2,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_3,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_4,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_5,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_6,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_7,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+8 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_8,
                   NETMGR_CFG_CONNID_LEN );
#endif /* FEATURE_DATA_IWLAN */
    }

    netmgr_config_print();
  }
  else if( DS_TARGET_DPM_2_0 == target)
  {
    netmgr_log_med("netmgrd: DPM 2.0 configuration, using configdb");
    if (0 != netmgr_config_load(NETMGR_CONFIG_FILE, target_str ))
    {
      netmgr_log_err("netmgrd: Configuration using %s [%s] failed, reverting to defaults",
                      NETMGR_CONFIG_FILE, target_str);

      /*set MSM only configuration to open smd control and data ports.*/
      netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM] = TRUE;
      netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM] = FALSE;

      /* Replace multiple control channels with single control channel */
      int index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
      netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
      strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
                    NETMGR_MAIN_RMNET_DATA_PREFIX,
                    NETMGR_IF_NAME_MAX_LEN );
      strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET_BAM, sizeof(netmgr_main_cfg.phys_net_dev));
      netmgr_main_cfg.single_qmux_ch_enabled = TRUE;
      strlcpy(netmgr_main_cfg.qmux_ch_name, QMI_PORT_RMNET_0, sizeof(netmgr_main_cfg.qmux_ch_name));
      strlcpy(netmgr_main_cfg.smd_ch_name, "DATA5_CNTL", sizeof(netmgr_main_cfg.smd_ch_name));

      strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
                   QMI_PORT_RMNET_DATA_0,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
                   QMI_PORT_RMNET_DATA_1,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
                   QMI_PORT_RMNET_DATA_2,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
                   QMI_PORT_RMNET_DATA_3,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
                   QMI_PORT_RMNET_DATA_4,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
                   QMI_PORT_RMNET_DATA_5,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
                   QMI_PORT_RMNET_DATA_6,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
                   QMI_PORT_RMNET_DATA_7,
                   NETMGR_CFG_CONNID_LEN );

#ifdef FEATURE_DATA_IWLAN
      /* We will be setting persist.data.iwlan.enable property to true
       * by default in the config file * If reading from the config file
       * fails for some reason, we will default it to true */
      netmgr_main_cfg.iwlan_enabled = TRUE;

      /* Change prefix for reverse links */
      strlcpy(NETMGR_MAIN_GET_DEV_REV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_REV_RMNET_SMD_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);

      /* Get starting index of reverse rmnet ports */
      index = NETMGR_MAIN_GET_INST_MIN_REV(NETMGR_MODEM_MSM);

      strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_0,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_1,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_2,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_3,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_4,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_5,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_6,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_7,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+8 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_8,
                   NETMGR_CFG_CONNID_LEN );
#endif /* FEATURE_DATA_IWLAN */
    }
    netmgr_config_print();
  }
  else if(DS_TARGET_JOLOKIA == target )
  {
    netmgr_log_med("netmgrd: JOLOKIA configuration, using configdb");
    if (0 != netmgr_config_load(NETMGR_CONFIG_FILE, target_str ))
    {
      netmgr_log_err("%s: unable to load configuration for JO",__func__);
      return;
    }
  }
  else if (DS_TARGET_LE_MDM9X25 == target ||
           DS_TARGET_LE_MDM9X15 == target ||
           DS_TARGET_LE_LEGACY == target )
  {
    /*set MSM only configuration to open smd control and data ports.*/
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM] = TRUE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM] = FALSE;
  }
  else if (DS_TARGET_LE_MDM9X35 == target)
  {
    /*set MSM only configuration to open smd control and data ports.*/
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM] = TRUE;
    netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM] = FALSE;

    /* Replace multiple control channels with single control channel */
    int index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);

    strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
                 NETMGR_MAIN_RMNET_DATA_PREFIX,
                 NETMGR_IF_NAME_MAX_LEN );
    strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET_IPA, sizeof(netmgr_main_cfg.phys_net_dev));
    netmgr_main_cfg.single_qmux_ch_enabled = TRUE;
    strlcpy(netmgr_main_cfg.qmux_ch_name, QMI_PORT_RMNET_0, sizeof(netmgr_main_cfg.qmux_ch_name));
    strlcpy(netmgr_main_cfg.smd_ch_name, "DATA5_CNTL", sizeof(netmgr_main_cfg.smd_ch_name));

    strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
                 QMI_PORT_RMNET_DATA_0,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
                 QMI_PORT_RMNET_DATA_1,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
                 QMI_PORT_RMNET_DATA_2,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
                 QMI_PORT_RMNET_DATA_3,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
                 QMI_PORT_RMNET_DATA_4,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
                 QMI_PORT_RMNET_DATA_5,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
                 QMI_PORT_RMNET_DATA_6,
                 NETMGR_CFG_CONNID_LEN );
    strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
                 QMI_PORT_RMNET_DATA_7,
                 NETMGR_CFG_CONNID_LEN );
  }
  else if (DS_TARGET_MSM8994 == target)
  {
    netmgr_log_med("netmgrd: MSM8994 configuration, using configdb");
    if (0 != netmgr_config_load(NETMGR_CONFIG_FILE, NETMGR_PROPERTY_DATA_TARGET_VALUE_MSM8994))
    {
      netmgr_log_err("netmgrd: Configuration using %s [%s] failed, reverting to defaults",
          NETMGR_CONFIG_FILE, NETMGR_PROPERTY_DATA_TARGET_VALUE_MSM8994);

      int index = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
      netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]=TRUE;
      netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]=FALSE;

      netmgr_main_cfg.def_link = NETMGR_MAIN_GET_INST_MIN(NETMGR_MODEM_MSM);
      strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET, sizeof(netmgr_main_cfg.phys_net_dev));

      /* Replace multiple control channels with single control channel */
      strlcpy( NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
                   NETMGR_MAIN_RMNET_DATA_PREFIX,
                   NETMGR_IF_NAME_MAX_LEN );
      netmgr_main_cfg.rmnet_data_enabled = TRUE;
      strlcpy(netmgr_main_cfg.phys_net_dev, NETMGR_PHYS_NET_DEV_RMNET_IPA, sizeof(netmgr_main_cfg.phys_net_dev));
      netmgr_main_cfg.wda_data_format = TRUE;
      netmgr_main_cfg.data_format.dl_data_aggregation_protocol = 7;

      netmgr_main_cfg.single_qmux_ch_enabled = TRUE;
      strlcpy(netmgr_main_cfg.qmux_ch_name, QMI_PORT_RMNET_0, sizeof(netmgr_main_cfg.qmux_ch_name));
      strlcpy(netmgr_main_cfg.smd_ch_name, "DATA5_CNTL", sizeof(netmgr_main_cfg.smd_ch_name));

      strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_0,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_1,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_2,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_3,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_4,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_5,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_6,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
               QMI_PORT_RMNET_DATA_7,
               NETMGR_CFG_CONNID_LEN );

#ifdef FEATURE_DATA_IWLAN
      /* We will be setting persist.data.iwlan.enable property to true
       * by default in the config file * If reading from the config file
       * fails for some reason, we will default it to true */
      netmgr_main_cfg.iwlan_enabled = IWLAN_ENABLE;

      /* Change prefix for reverse links */
      strlcpy(NETMGR_MAIN_GET_DEV_REV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_REV_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);

      /* Get starting index of reverse rmnet ports */
      index = NETMGR_MAIN_GET_INST_MIN_REV(NETMGR_MODEM_MSM);

      strlcpy( netmgr_ctl_port_array[ index+0 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_0,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+1 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_1,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+2 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_2,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+3 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_3,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+4 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_4,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+5 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_5,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+6 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_6,
               NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+7 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_7,
                   NETMGR_CFG_CONNID_LEN );
      strlcpy( netmgr_ctl_port_array[ index+8 ].qmi_conn_id,
               QMI_PORT_REV_RMNET_DATA_8,
                   NETMGR_CFG_CONNID_LEN );
#endif /* FEATURE_DATA_IWLAN */
    }

    netmgr_config_print();
  }
  else
  {
    netmgr_log_med("netmgrd: loading target %s configuration", target_str);
    if (0 != netmgr_config_load(NETMGR_CONFIG_FILE, target_str ))
    {
      netmgr_log_err("%s: Unable to load target %s config from xml", __func__, target_str);
      return;
    }
    netmgr_config_print();
  }

  netmgr_main_disable_modem_links(netmgr_main_cfg.modem_enable, NETMGR_MAX_MODEMS);

#ifdef FEATURE_DATA_IWLAN
  netmgr_main_process_iwlan_enabled(netmgr_main_cfg.modem_enable, NETMGR_MAX_MODEMS);
#endif /* FEATURE_DATA_IWLAN */
}

#ifdef FEATURE_DS_LINUX_ANDROID

/*===========================================================================
  FUNCTION  netmgr_main_process_qos_enabled
===========================================================================*/
/*!
@brief
  Updates netmgr configuration based on QOS enabled property value

@return
  void

*/
/*=========================================================================*/
LOCAL void netmgr_main_process_qos_enabled()
{
  char args[PROPERTY_VALUE_MAX];
  int ret;

  /* Retrieve value of NETMGR_MAIN_PROPERTY_QOS */
  memset(args, 0, sizeof(args));
  ret = property_get( NETMGR_MAIN_PROPERTY_QOS, args, NETMGR_MAIN_PROPERTY_QOS_DEFAULT );
  if (ret > NETMGR_MAIN_PROPERTY_QOS_SIZE)
  {
    netmgr_log_err("property [%s] has size [%d] that exceeds max [%d]\n",
                   NETMGR_MAIN_PROPERTY_QOS,
                   ret,
                   NETMGR_MAIN_PROPERTY_QOS_SIZE);
    return;
  }

  netmgr_log_med("property [%s] value[%s]", NETMGR_MAIN_PROPERTY_QOS, args);
  if( !strncmp( NETMGR_FALSE, args, sizeof(NETMGR_FALSE) ) )
  {
    /* Clear QOS enabled flag */
    netmgr_main_cfg.runmode &= ~NETMGR_MAIN_RUNMODE_QOSHDR;
  }
  else
  {
    /* By default, we need to enable QOS if the property is not set
     * to false explicitly */
    if (strncmp( NETMGR_TRUE, args, sizeof(NETMGR_TRUE) ) )
    {
      /* Default to true even if not set explicitly */
      netmgr_log_err("Unsupported state value,using default[%s]\n",
                     NETMGR_MAIN_PROPERTY_QOS_DEFAULT);
    }

    netmgr_main_cfg.runmode |= NETMGR_MAIN_RUNMODE_QOSHDR;
  }
}

/*===========================================================================
  FUNCTION  netmgr_main_process_qos_hybrid_enabled
===========================================================================*/
LOCAL void netmgr_main_process_qos_hybrid_enabled()
{
  char args[PROPERTY_VALUE_MAX];
  int ret;

  /* Retrieve value of NETMGR_MAIN_PROPERTY_QOS_HYBRID */
  memset(args, 0, sizeof(args));
  ret = property_get(NETMGR_MAIN_PROPERTY_QOS_HYBRID, args,
                     NETMGR_MAIN_PROPERTY_QOS_HYBRID_DEFAULT);

  netmgr_log_med("property [%s] value[%s]", NETMGR_MAIN_PROPERTY_QOS_HYBRID, args);
  if( !strncmp( NETMGR_FALSE, args, sizeof(NETMGR_FALSE) ) )
  {
    /* Clear QOS hybrid flag */
    netmgr_main_cfg.hybrid_qos = FALSE;
  }
  else if( !strncmp(NETMGR_TRUE, args, sizeof(NETMGR_TRUE) ) )
  {
    /* Set QOS hybrid flag */
    netmgr_main_cfg.hybrid_qos = TRUE;
  }
  else
  {
    netmgr_log_err("Unsupported state value, using default[%s]\n",
                   NETMGR_MAIN_PROPERTY_QOS_HYBRID_DEFAULT);
  }

}
#endif /* FEATURE_DS_LINUX_ANDROID */


/*===========================================================================
  FUNCTION  netmgr_main_process_dpm_enabled()
===========================================================================*/
/*!
@brief
  If DPM is enabled/required, use QMI_DPM message to open ports on modem.

@return
  void

*/
/*=========================================================================*/
LOCAL void
netmgr_main_process_dpm_enabled
(
   void
)
{
#ifdef FEATURE_DS_LINUX_ANDROID
  char args[PROPERTY_VALUE_MAX];
  int ret;

  /* Retrieve value of NETMGR_MAIN_PROPERTY_DPM_ENABLE */
  NETMGR_LOG_FUNC_ENTRY;
  memset(args, 0, sizeof(args));
  property_get(NETMGR_MAIN_PROPERTY_DPM_ENABLE,
               args,
               NETMGR_MAIN_PROPERTY_DPM_ENABLE_DEFAULT);

  if (!strncmp(NETMGR_TRUE, args, sizeof(NETMGR_TRUE)))
  {
    netmgr_main_cfg.dpm_enabled = TRUE;
  }

  netmgr_log_med("property [%s] value[%s]",
      NETMGR_MAIN_PROPERTY_DPM_ENABLE, args);

#elif defined(FEATURE_DATA_MDM_SINGLE_QMUX_CHANNEL)

  NETMGR_LOG_FUNC_ENTRY;
  netmgr_log_med("MDM platform, single QMUX channel enabled, enabling DPM\n");
  netmgr_main_cfg.dpm_enabled = TRUE;

#endif

  if (netmgr_main_get_dpm_enabled())
  {
    /* Send port open message to Modem */
    if(NETMGR_SUCCESS != netmgr_qmi_dpm_init())
    {
      netmgr_log_err("FATAL: netmgr_qmi_dpm_init() failed\n" );
    }
    else
    {
      netmgr_log_med( "netmgr_qmi_dpm_init() successful. \n" );
    }

    if (NETMGR_SUCCESS != netmgr_qmi_dpm_port_open())
    {
      netmgr_log_err("FATAL: netmgr_qmi_dpm_port_open() failed\n");
    }
    else
    {
      netmgr_log_med( "netmgr_qmi_dpm_port_open() successful.\n" );
    }
  }
  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
  FUNCTION  netmgr_main_process_wda_data_format_enabled()
===========================================================================*/
/*!
@brief
  If QMI_WDA format is enabled/required, update the configuration
  in netmgr_main_cfg.

@return
  void

*/
/*=========================================================================*/
LOCAL void
netmgr_main_process_wda_data_format_enabled
(
  void
)
{
#ifdef FEATURE_DS_LINUX_ANDROID
  char args[PROPERTY_VALUE_MAX];
  int ret;

  /* Retrieve value of NETMGR_MAIN_PROPERTY_WDA_ENABLE */
  NETMGR_LOG_FUNC_ENTRY;
  memset(args, 0, sizeof(args));
  ret = property_get(NETMGR_MAIN_PROPERTY_WDA_ENABLE,
                     args,
                     NETMGR_MAIN_PROPERTY_WDA_ENABLE_DEFAULT);
  if (!strncmp(NETMGR_TRUE, args, sizeof(NETMGR_TRUE)))
  {
    netmgr_main_cfg.wda_data_format = TRUE;
  }

  netmgr_log_med("property [%s] value[%s]",
      NETMGR_MAIN_PROPERTY_WDA_ENABLE, args);

#elif defined(FEATURE_DATA_MDM_SINGLE_QMUX_CHANNEL)

  NETMGR_LOG_FUNC_ENTRY;
  netmgr_log_med("MDM platform, single QMUX channel enabled, enabling WDA data format\n");
  netmgr_main_cfg.wda_data_format = TRUE;

#endif

  NETMGR_LOG_FUNC_EXIT;

}


/*===========================================================================
  FUNCTION  netmgr_main_process_rmnet_data_enabled
===========================================================================*/
/*!
@brief
  Updates netmgr configuration based on QOS enabled property value

@return
  void

*/
/*=========================================================================*/
LOCAL void
netmgr_main_process_rmnet_data_enabled
(
   void
)
{

#ifdef FEATURE_DS_LINUX_ANDROID
  char args[PROPERTY_VALUE_MAX];
  int ret;

  NETMGR_LOG_FUNC_ENTRY;

  /* Retrieve value of NETMGR_MAIN_PROPERTY_RMNET_DATA */
  memset(args, 0, sizeof(args));
  ret = property_get( NETMGR_MAIN_PROPERTY_RMNET_DATA,
                      args,
                      NETMGR_MAIN_PROPERTY_RMNET_DATA_DEFAULT );

  if (ret > NETMGR_MAIN_PROPERTY_RMNET_DATA_SIZE)
  {
    netmgr_log_err("property [%s] has size [%d] that exceeds max [%d]\n",
                   NETMGR_MAIN_PROPERTY_RMNET_DATA,
                   ret,
                   NETMGR_MAIN_PROPERTY_RMNET_DATA_SIZE);
    return;
  }

  netmgr_log_med("property [%s] value[%s]",
                    NETMGR_MAIN_PROPERTY_RMNET_DATA, args);

  if( !strncmp( NETMGR_TRUE, args, sizeof(NETMGR_TRUE) ) )
  {
    netmgr_main_cfg.rmnet_data_enabled = TRUE;
  }
  else
  {
    netmgr_log_err("Unsupported state value, using default[%s]\n",
                   NETMGR_MAIN_PROPERTY_RMNET_DATA_DEFAULT);
  }

#elif defined(FEATURE_DATA_MDM_SINGLE_QMUX_CHANNEL)

  NETMGR_LOG_FUNC_ENTRY;
  netmgr_log_med("MDM platform, single QMUX channel, enable RMNET data\n");
  netmgr_main_cfg.rmnet_data_enabled = TRUE;

#endif

  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
  FUNCTION  netmgr_main_update_links
===========================================================================*/
/*!
@brief
  Update the link array to disable those for any SMD port used by
  external subsystem, and any over the number of links requested.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline void netmgr_main_update_links(void)
{
  int i = 0;
  int cnt = 0;

  NETMGR_LOG_FUNC_ENTRY;

  /* Verify number of specified links supported by configuration */
  if( NETMGR_MAX_LINK < netmgr_main_cfg.nint )
  {
    NETMGR_STOP( "Number links (%d) exceeds limit (%d), stopped!!",
                 netmgr_main_cfg.nint, NETMGR_MAX_LINK );
  }

  /* Verify there are active links requested */
  if( 0 == netmgr_main_cfg.nint )
  {
    NETMGR_STOP( "All links disabled, stopped!!" );
  }

  /* Check for external sybsystem config file in SYSFS */
  netmgr_read_sysfs_config();

#if 0  // TEMPORARY: SDIO RmNET driver crashing on close IOCTL
  /*  Validate link table against preconfigured & requested configuration */
  cnt = netmgr_main_cfg.nint;
  for(i=0; i<NETMGR_MAX_LINK; i++)
  {
    /* Ensure preconfigured links not exhausted */
    if( cnt && (NETMGR_LINK_MAX == netmgr_ctl_port_array[i].link_id) ) {
      goto bail;
    }

    /* Disable any link over number configured as it is not required */
    if( cnt && netmgr_ctl_port_array[i].enabled ) {
      /* Decrement active link counter */
      cnt--;
    } else {
      /* Suppress further processing for this link */
      netmgr_ctl_port_array[i].enabled = FALSE;
    }
  }

  netmgr_log_med( "netmgr_main_update_links: %d links enabled\n",
                  (netmgr_main_cfg.nint-cnt) );

bail:
#else
  /* Loop over array for unused links */
  for(i=netmgr_main_cfg.nint; i<NETMGR_MAX_LINK; i++)
  {
    /* Suppress further processing for this link */
    netmgr_ctl_port_array[i].enabled = FALSE;
  }
#endif

  /* Verify configured link number available; cnt should be zero */
  if( 0 < cnt ) {
    netmgr_log_err( "WARNING Number requested links (%d) exceed those available",
                    netmgr_main_cfg.nint );
  }

  netmgr_main_process_target();

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_main_sm_inited
===========================================================================*/
/*!
@brief
  posts NETMGR_INITED_EV to each state machine instance

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void netmgr_main_sm_inited(void)
{
  int i=0;
  netmgr_exec_cmd_t * cmd = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  for(i=0; i<NETMGR_MAX_LINK; i++)
  {
    /* Skip disabled links */
    if( netmgr_ctl_port_array[i].enabled == FALSE )
    {
      netmgr_log_low( "netmgr_main_sm_inited: ignoring link[%d]\n", i );
      continue;
    }
    if(netmgr_ctl_port_array[i].initialized == FALSE )
    {
      netmgr_log_low( "netmgr_main_sm_inited: ignoring un-initialized link[%d]\n", i );
      continue;
    }
    /* Allocate a command object */
    cmd = netmgr_exec_get_cmd();
    NETMGR_ASSERT(cmd);

    /* Set command object parameters */
    cmd->data.type = NETMGR_INITED_EV;
    cmd->data.link = i;
    memset(&cmd->data.info, 0, sizeof(cmd->data.info));

    /* Post command for processing in the command thread context */
    if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
      netmgr_log_err("failed to put commmand\n");
      netmgr_exec_release_cmd( cmd );
    }
  }

  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
  FUNCTION  netmgr_diag_cleanup
===========================================================================*/
/*!
@brief
  Performs cleanup of Diag LSM resources.  Invoked at process termination.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
netmgr_diag_cleanup(void)
{
#ifdef FEATURE_DATA_LOG_QXDM
  (void) Diag_LSM_DeInit();
#endif
}

/*===========================================================================
  FUNCTION  netmgr_load_data_format
===========================================================================*/
/*!
@brief
  Populates the runtime data format based on ADB properties or configuration.

@return
  0 always.
*/
/*=========================================================================*/
int
netmgr_load_data_format
(
  void
)
{
  char args[PROPERTY_VALUE_MAX];
  netmgr_data_format_t *props = &(netmgr_main_cfg.data_format);
  if (!props)
  {
    netmgr_log_err("%s() called with null params!", __func__);
    return NETMGR_FAILURE;
  }

  memset(props, 0, sizeof(netmgr_data_format_t));

  props->qos_format = netmgr_main_get_qos_enabled() & (!netmgr_main_get_qos_hybrid_enabled());
  props->link_prot = NETMGR_DATA_FMT_DFLT_LLP;

  NETMGR_LOAD_PROP(NETMGR_DATA_FMT_UL_FMT, args, NETMGR_DATA_FMT_DFLT_UL_FMT);
  props->ul_data_aggregation_protocol = (int) strtol (args, NULL, NETMGR_BASE_TEN);

  NETMGR_LOAD_PROP(NETMGR_DATA_FMT_DL_FMT, args, NETMGR_DATA_FMT_DFLT_DL_FMT);
  props->dl_data_aggregation_protocol = (int) strtol (args, NULL, NETMGR_BASE_TEN);

  NETMGR_LOAD_PROP(NETMGR_DATA_FMT_MUX_CNT, args, NETMGR_DATA_FMT_DFLT_MUX_CNT);
  props->num_mux_channels = (int) strtol (args, NULL, NETMGR_BASE_TEN);

  NETMGR_LOAD_PROP(NETMGR_DATA_FMT_IWLAN_MUX, args, NETMGR_DATA_FMT_DFLT_IWLAN_MUX);
  props->num_iwlan_channels = (int) strtol (args, NULL, NETMGR_BASE_TEN);

  NETMGR_LOAD_PROP(NETMGR_DATA_FMT_DL_SZE, args, NETMGR_DATA_FMT_DFLT_DL_SZE);
  props->dl_agg_size = (int) strtol (args, NULL, NETMGR_BASE_TEN);

  NETMGR_LOAD_PROP(NETMGR_DATA_FMT_DL_PKT, args, NETMGR_DATA_FMT_DFLT_DL_PKT);
  props->dl_agg_cnt = (int) strtol (args, NULL, NETMGR_BASE_TEN);

  NETMGR_LOAD_PROP(NETMGR_DATA_FMT_DL_PAD, args, NETMGR_DATA_FMT_DFLT_DL_PAD);
  props->dl_agg_pad = (int) strtol (args, NULL, NETMGR_BASE_TEN);

  netmgr_log_high("Netmgr Data format: ");
  netmgr_log_high("..qos: %d", props->qos_format);
  netmgr_log_high("..link_prot: %d", props->link_prot);
  netmgr_log_high("..num_mux_channels: %d", props->num_mux_channels);
  netmgr_log_high("..num_iwlan_mux_channels: %d", props->num_iwlan_channels);
  netmgr_log_high("..ul_data_aggregation_protocol: %d", props->ul_data_aggregation_protocol);
  netmgr_log_high("..dl_data_aggregation_protocol: %d", props->dl_data_aggregation_protocol);
  netmgr_log_high("..dl_agg_size: %d", props->dl_agg_size);
  netmgr_log_high("..dl_agg_cnt: %d", props->dl_agg_cnt);
  netmgr_log_high("..dl_agg_pad: %d", props->dl_agg_pad);

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_main
===========================================================================*/
/*!
@brief
  Main entry point of the core program. Performs all program initialization.

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
netmgr_main (int argc, char ** argv)
{

#ifdef FEATURE_DATA_LOG_QXDM
  /* Initialize Diag services */
  if ( TRUE != Diag_LSM_Init(NULL) )
  {
    netmgr_log_err("failed on Diag_LSM_Init\n" );
  }
  else
  {
    atexit(netmgr_diag_cleanup);
  }
#endif
#ifdef FEATURE_DATA_LOG_SYSLOG
  /* Initialize logging as per desired mode */
  netmgr_log_init(netmgr_main_cfg.logthreshold, netmgr_main_cfg.logmode);
#endif

#ifdef FEATURE_WAIT_FOR_MODEM
  /* Interim solution for device open latency in driver layer */
  sleep( FEATURE_WAIT_FOR_MODEM );
#endif

  NETMGR_LOG_FUNC_ENTRY;

  DS_OPEN_TRACE_MARKER;

  /* Initialize number of active interfaces to default; may be overridden
   * by system property or command-line argument */
  netmgr_main_cfg.nint = NETMGR_MAIN_DEFAULT_NINT;

  /* Initialze number of links; may be overridded */
  netmgr_main_reset_links();

#ifdef NETMGR_QOS_ENABLED
  /* Turn QoS header on based on QoS ADB property */
  netmgr_main_process_qos_enabled();
#endif /* NETMGR_QOS_ENABLED */

  /* Parse command line arguments and populate configuration blob */
  netmgr_main_parse_args(argc, argv);

  netmgr_load_data_format();

  /* If rmnet_data is enabled, perform all rmnet data related configuration */
  netmgr_main_process_rmnet_data_enabled();

  /* Update links state based on external subsystem usage */
  netmgr_main_update_links();

  /* Initialize netmgr command queue */
  if (NETMGR_SUCCESS != netmgr_cmdq_init())
  {
    netmgr_log_err("netmgr_qmi_dpm_cmdq_init(): Failed!");
  }

  if (netmgr_main_get_rmnet_data_enabled())
  {
#ifdef FEATURE_DS_LINUX_ANDROID
    netmgr_main_check_ibfc_enabled();
#endif /* FEATURE_DS_LINUX_ANDROID */
    /* This function call is effective only for IPA based targets */
    netmgr_rmnet_configure_ep_params();

    /* Perform DPM port open if DPM is enabled for this target */
    netmgr_main_process_dpm_enabled();

    netmgr_rmnet_configure_embedded_data();
  }

  /* Check if WDA data format needs to be used */
  netmgr_main_process_wda_data_format_enabled();

  /* Run as a daemon, if requested */
  if( netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_BACK ) {
    netmgr_daemonize();
    netmgr_log_low( "daemonize completed\n" );
    (void)sleep(1);
  }

  /* Register signal handler */
  signal(SIGUSR1, netmgr_signal_handler);
  signal(SIGUSR2, netmgr_signal_handler);
  signal(SIGTERM, netmgr_signal_handler);

  /* Initialize executive module */
  netmgr_exec_init( netmgr_main_cfg.nint, netmgr_ctl_port_array );

  /* Initialize platform layer */
  netmgr_platform_init();
  netmgr_log_med( "platform init completed\n" );

#ifdef FEATURE_DS_LINUX_ANDROID
  netmgr_main_check_dropssdp_enabled();
#endif /* FEATURE_DS_LINUX_ANDROID */
  netmgr_kif_powerup_init(netmgr_ctl_port_array, netmgr_main_cfg.iname);

  netmgr_main_process_qos_hybrid_enabled();

  if (!netmgr_main_get_qos_hybrid_enabled())
  {
    /* Check to see if qos_header_format is needed. Gets the value of the WDA
     * data format property qos_header_format to be sent to the modem.
     * qos_header_format is applicable only when baseband is not mdm2
     * and netmgr_kif_ifioctl_set_qosmode() is implemented. */
     netmgr_kif_get_qos_header_format((const char *)netmgr_main_get_phys_net_dev(),
                                      NETMGR_RMNET_START,
                                      &(netmgr_main_cfg.data_format));
  }
  /* Initialize QMI interface module */
  netmgr_qmi_init( netmgr_main_cfg.nint, netmgr_ctl_port_array, netmgr_main_cfg.data_format);

#ifdef FEATURE_DS_LINUX_ANDROID
  netmgr_main_check_low_latency_filters_enabled();
#endif /* FEATURE_DS_LINUX_ANDROID */
  if (netmgr_main_get_low_latency_filters_enabled())
  {
    netmgr_qmi_dfs_enable_low_latency_filters((const char *)netmgr_main_get_phys_net_dev());
  }

  netmgr_log_med( "qmi init completed\n" );

  if (!netmgr_main_get_qos_hybrid_enabled())
  {
    /* Sets the qos_header_format for every virtual network device if
     * applicable. */
     netmgr_kif_set_qos_header_format(NETMGR_RMNET_START,
                                      &(netmgr_main_cfg.data_format));
  }
  /* Initialize kernel interface module */
  netmgr_kif_init( netmgr_main_cfg.nint,
                   netmgr_main_cfg.skip,
                   netmgr_main_cfg.dirpath,
                   netmgr_main_cfg.modscr);
  netmgr_log_med( "kif init completed\n" );

#ifdef NETMGR_QOS_ENABLED
  netmgr_main_check_tcpackprio_enabled();
  if( netmgr_main_get_qos_enabled() )
  {
    /* Initialize traffic control module */
    netmgr_tc_init( netmgr_main_cfg.nint, netmgr_ctl_port_array );
    netmgr_log_low( "tc init completed\n" );
  }
#endif // NETMGR_QOS_ENABLED

  netmgr_main_cfg.initialized = TRUE;
  netmgr_log_high("Initialization complete.\n");

  /* bring up each SM instance in INITED state */
  netmgr_main_sm_inited();

#ifdef FEATURE_DATA_IWLAN
  if (IWLAN_DISABLE != netmgr_main_get_iwlan_enabled())
  {
    /* Install all the static iwlan iptable rules */
    if (NETMGR_SUCCESS != netmgr_kif_iwlan_install_iptables_rules(AF_INET))
    {
      netmgr_log_err("Failed to install V4 iwlan rules!\n");
    }

    if (NETMGR_SUCCESS != netmgr_kif_iwlan_install_iptables_rules(AF_INET6))
    {
      netmgr_log_err("Failed to install V6 iwlan rules!\n");
    }

    /* Register for WLAN events */
    if (NETMGR_SUCCESS != netmgrIwlanClientInit())
    {
      netmgr_log_err("Error in registering for WLAN events!");
    }
  }
#endif /* FEATURE_DATA_IWLAN */

#if (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))
  /* adjust uid/gid and capabilities */
  if (ds_change_user_cap( AID_RADIO, AID_INET,
                          (1ULL << CAP_NET_ADMIN) | (1ULL << CAP_NET_RAW)) != 0)
  {
    netmgr_log_err("couldn't change uid and capabilities at power up");
    exit(EXIT_FAILURE);
  }
#endif

  /* Start responding to ready requests from clients */
  netmgr_main_cfg.process_ready_reqs = TRUE;

  /* Indefinitely wait on command processing thread. */
  netmgr_log_med( "netmgr going into wait loop" );
  netmgr_exec_wait();

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

