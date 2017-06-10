#ifndef NETMGR_CONFIG_H
#define NETMGR_CONFIG_H
/*!
  @file
  netmgr_config.h

  @brief
  This file provides configuration required by netmgr module. It uses configdb
  library and XML configuration file ("netmgr_config.xml").
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

#ifdef __cplusplus
extern "C" {
#endif

#include "netmgr.h"
#include "netmgr_defs.h"


/*===========================================================================
                               MACRO DEFINITIONS
===========================================================================*/

#define NETMGR_PROPERTY_DATA_TARGET "persist.data.target"
#define NETMGR_PROPERTY_DATA_TARGET_VALUE_FUSION4_5_PCIE "fusion4_5_pcie"
#define NETMGR_PROPERTY_DATA_TARGET_VALUE_FUSION4_5_HSIC "fusion4_5_hsic"
#define NETMGR_PROPERTY_DATA_TARGET_VALUE_MSM8994        "msm8994"

#ifdef FEATURE_DS_LINUX_ANDROID
#define NETMGR_CONFIG_FILE  "/system/etc/data/netmgr_config.xml"
#else
#define NETMGR_CONFIG_FILE  "/etc/data/netmgr_config.xml"
#endif /* FEATURE_DS_LINUX_ANDROID */

#define NETMGR_CFGDB_STRVAL_MAX 100

/* Structure for QMI data control devices. It also contains a boolean
 * member to identify if the link is enabled or not. */
#define NETMGR_CFG_PARAM_LEN  20
#define NETMGR_CFG_CONNID_LEN 30

#define NETMGR_DEVICE_NAME_SIZE 100

/* Macros representing program run mode */
#define  NETMGR_MAIN_RUNMODE_DEFAULT     0x00  /* Default runmode          */
#define  NETMGR_MAIN_RUNMODE_BACK        0x01  /* Run as forked deamon     */
#define  NETMGR_MAIN_RUNMODE_ETHERNET    0x02  /* Use Ethernet framing     */
#define  NETMGR_MAIN_RUNMODE_QOSHDR      0x04  /* Use RmNET QoS header     */

/* Macros representing physical network devices */
#define NETMGR_PHYS_NET_DEV_RMNET       "rmnet0"
#define NETMGR_PHYS_NET_DEV_RMNET_IPA   "rmnet_ipa0"
#define NETMGR_PHYS_NET_DEV_RMNET_USB   "rmnet_usb0"
#define NETMGR_PHYS_NET_DEV_RMNET_MHI   "rmnet_mhi0"
#define NETMGR_PHYS_NET_DEV_RMNET_BAM   NETMGR_PHYS_NET_DEV_RMNET

/*===========================================================================
                     DEFINITIONS AND DECLARATIONS
===========================================================================*/
typedef struct netmgr_data_format_s {
  int qos_format;
  int link_prot;
  int ul_data_aggregation_protocol;
  int dl_data_aggregation_protocol;
  int dl_agg_size;
  int dl_agg_cnt;
  int dl_agg_pad;
  int num_mux_channels;
  int num_iwlan_channels;
  int qos_header_format;
  int qos_header_format_valid;
} netmgr_data_format_t;

typedef enum iwlan_state_e
{
  IWLAN_DISABLE,
  IWLAN_ENABLE,
  NSWO_ONLY
}iwlan_state_t;

/* Collection of program configuration info */
struct netmgr_main_cfg_s {
  int runmode;         /* Process run mode */
  int logmode;         /* Logging mode */
  int logthreshold;    /* Logging threshold */
  int nint;            /* Number of interfaces */
  char * iname;        /* Name of virtual ethernet interface */
  int skip;            /* Whether to skip driver module loading */
  char * dirpath;      /* Directory pathname to look for script files */
  char * modscr;       /* Name of script to use for loading module */
  boolean debug;       /* Verbose debug flag */
  boolean runtests;    /* Execute internal tests flag */
  boolean initialized; /* Program initialization completed */
  netmgr_link_id_t  def_link; /* Default link */
  iwlan_state_t iwlan_enabled;      /* iWLAN feature is enabled */
  boolean iwlan_ims_enabled;  /* iWLAN IMS feature is enabled */

  boolean rmnet_data_enabled; /* RmNET data enabled */
  char phys_net_dev[NETMGR_DEVICE_NAME_SIZE]; /* Physical network device name */

  int modem_enable[NETMGR_MAX_MODEMS]; /* Modems enabled status */

  boolean single_qmux_ch_enabled; /* Is single QMUX channel enabled */
  char qmux_ch_name[NETMGR_DEVICE_NAME_SIZE];/* Single QMUX channel name */
  char smd_ch_name[NETMGR_DEVICE_NAME_SIZE]; /* Single SMD control channel */
  uint32 epid;                  /* Data channel EPID associated with QMUX ch */
  uint32 ep_type;               /* EP_TYPE based on the underlying physical channel */
  uint32 consumer_pipe_num;     /* IPA consumer EP number for data channel */
  uint32 producer_pipe_num;     /* IPA producer EP number for data channel */

  boolean dpm_enabled;        /* Data port-mapper enabled */
  boolean wda_data_format;    /* Use WDA data format (default is CTL) */
  boolean tcp_ack_prio;       /* TCP Ack Prioritization is enabled */
  boolean ibfc;               /* In-band flow control */
  struct netmgr_data_format_s data_format;
  int modem_ssr_state;        /* Modem SSR state */
  int rps_mask;               /* RPS mask on transport */
  int netdev_budget;          /* System-wide netdev budget */
  boolean low_latency_filters;  /* Use QMI DFS for low latency filters */
  boolean dropssdp;           /* Drop SSDP packets on WWAN */
  boolean process_ready_reqs; /* Should netmgr start responding to ready reqs */
  boolean hybrid_qos;         /* Hybrid qos */
  int netdev_max_backlog; /* System-wide netdev max backlog */
  unsigned long tc_ul_baserate; /* Maximum TC UL rate */
  int           tc_ul_burst;    /*TC UL Burst */
};

extern struct netmgr_main_cfg_s netmgr_main_cfg;


typedef struct netmgr_ctl_port_config_s
{
  char              data_ctl_port[NETMGR_CFG_PARAM_LEN];
  char              qmi_conn_id[NETMGR_CFG_CONNID_LEN];
  netmgr_link_id_t  link_id;
  boolean           modem_wait;
  boolean           enabled;
  boolean           v4_qos_rules_enabled;
  boolean           v6_qos_rules_enabled;
  boolean           initialized;
}netmgr_ctl_port_config_type;

extern netmgr_ctl_port_config_type netmgr_ctl_port_array[NETMGR_MAX_LINK+1];

#ifdef FEATURE_DATA_IWLAN
  #define NETMGR_MAIN_GET_INST_MIN(MODEM)                         \
    netmgr_main_dev_prefix_tbl[MODEM][NETMGR_FWD_LINK].inst_min
  #define NETMGR_MAIN_GET_INST_MAX(MODEM)                         \
    netmgr_main_dev_prefix_tbl[MODEM][NETMGR_FWD_LINK].inst_max
  #define NETMGR_MAIN_GET_INST_MIN_REV(MODEM)                     \
    netmgr_main_dev_prefix_tbl[MODEM][NETMGR_REV_LINK].inst_min
  #define NETMGR_MAIN_GET_INST_MAX_REV(MODEM)                     \
    netmgr_main_dev_prefix_tbl[MODEM][NETMGR_REV_LINK].inst_max
  #define NETMGR_MAIN_GET_DEV_PREFIX(MODEM)                       \
    netmgr_main_dev_prefix_tbl[MODEM][NETMGR_FWD_LINK].prefix
  #define NETMGR_MAIN_GET_DEV_REV_PREFIX(MODEM)                   \
    netmgr_main_dev_prefix_tbl[MODEM][NETMGR_REV_LINK].prefix
#else
  #define NETMGR_MAIN_GET_INST_MIN(MODEM)                         \
    netmgr_main_dev_prefix_tbl[MODEM].inst_min
  #define NETMGR_MAIN_GET_INST_MAX(MODEM)                         \
    netmgr_main_dev_prefix_tbl[MODEM].inst_max
  #define NETMGR_MAIN_GET_DEV_PREFIX(MODEM)                       \
    netmgr_main_dev_prefix_tbl[MODEM].prefix
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
);

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
void netmgr_config_print();

#ifdef __cplusplus
}
#endif

#endif /* NETMGR_CONFIG_H */

