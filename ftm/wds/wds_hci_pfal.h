/*==========================================================================

                     WDS HCI PFAL Header File

Description
   PFAL API declarations of the Wds hci pfal component.

# Copyright (c) 2012 by Qualcomm Atheros, Inc..
# All Rights Reserved.
# Qualcomm Atheros Confidential and Proprietary.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/18/11   subrsrin  Created a header file to hold the PFAL declarations to
                     configure the UART and send/receive packets between
                     UART and SMD channels
5/8/12     ankurn    Added support for ANT+ HCI commands
===========================================================================*/
#define BT_CMD_PKT 1
#define FM_CMD_PKT 2
#define BT_ACL_PKT 3
#define ANT_CMD_PKT  4
#define ANT_DATA_PKT 5

/*BT HS UART TTY DEVICE */
#define BT_HS_NMEA_DEVICE "/dev/ttyGS0"
#define BT_HS_UART_DEVICE "/dev/ttyHSL0"
/*BT RIVA-SMD CHANNELS */
#define APPS_RIVA_BT_ACL_CH  "/dev/smd2"
#define APPS_RIVA_BT_CMD_CH  "/dev/smd3"
#define APPS_RIVA_FM_CMD_CH  "/dev/smd1"
#define APPS_RIVA_ANT_CMD    "/dev/smd5"
#define APPS_RIVA_ANT_DATA   "/dev/smd6"

#define BT_ONLY      0
#define FM_ONLY      1
#define ANT_ONLY     2
#define ALL_SMD_CHANNELS 3
#define UART_BT_ONLY  4
#define UART_ANT_ONLY  5

#define BT_CMD_PKT_HDR_LEN  2
#define FM_CMD_PKT_HDR_LEN  2
#define ACL_PKT_HDR_LEN  4
#define ANT_CMD_PKT_HDR_LEN   1
#define ANT_DATA_PKT_HDR_LEN  1
#define BT_FM_PKT_UART_HDR_LEN  4
#define ACL_PKT_UART_HDR_LEN  5
#define ANT_CMD_DATA_PKT_UART_HDR_LEN 2

#define BT_EVT_PKT_HDR_LEN_UART  (BT_CMD_PKT_HDR_LEN+1)
#define ACL_PKT_HDR_LEN_UART  (ACL_PKT_HDR_LEN+1)

/* ANT data packet type */
#define ANT_DATA_TYPE_BROADCAST  0x4E
#define ANT_DATA_TYPE_ACKNOWLEDGED  0x4F
#define ANT_DATA_TYPE_BURST     0x50
#define ANT_DATA_TYPE_ADV_BURST     0x72

/*Packet Identifiers */
#define BT_CMD_PKT_ID 0x01
#define FM_CMD_PKT_ID 0x11
#define BT_EVT_PKT_ID 0x04
#define FM_EVT_PKT_ID 0x14
#define ANT_CMD_PKT_ID  0x0C
#define ANT_EVT_PKT_ID  0x0C
#define ANT_DATA_PKT_ID 0x0E
#define BT_ACL_DATA_PKT_ID 0x02

#define SMD_BUFF_SIZE 9000
#define UART_BUF_SIZE 9000

/*===========================================================================
FUNCTION   wds_pkt_dispatch

DESCRIPTION
  Writes data into SMD interfaces

DEPENDENCIES
  NIL

RETURN VALUE
  NIL

SIDE EFFECTS
  None

===========================================================================*/
void wds_pkt_dispatch(void *ftm_bt_pkt ,int cmd_len, int pkt_type);
/*===========================================================================
FUNCTION   wds_initiate_thread

DESCRIPTION
  Initiates the reader thread to monitor the SMD interfaces

DEPENDENCIES
  NIL

RETURN VALUE
  NIL

SIDE EFFECTS
  None

===========================================================================*/
void wds_initiate_thread(void);
/*===========================================================================
FUNCTION   get_acl_pkt_length

DESCRIPTION
  Routine to calculate the ACL packet data length

DEPENDENCIES
  NIL

RETURN VALUE
  ACL packet data length

SIDE EFFECTS
  None

===========================================================================*/
int get_acl_pkt_length(unsigned char, unsigned char);
/*===========================================================================
FUNCTION   init_uart

DESCRIPTION
  Routine to set the properties of UART port

DEPENDENCIES
  NIL

RETURN VALUE
  File descriptor of the UART port

SIDE EFFECTS
  None

===========================================================================*/
int init_uart(char *);
