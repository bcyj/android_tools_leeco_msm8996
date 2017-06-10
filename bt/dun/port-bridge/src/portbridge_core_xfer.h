/******************************************************************************

  @file    portbridge_core_xfer.h
  @brief   Control and Data Transfer threads header

  DESCRIPTION
  Header for the control and data transfer threads

 ******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2013 Qualcomm Technologies, Inc. All Rights Reserved

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
#ifndef __PORTBRIDGE_CORE_XFER_H__
#define __PORTBRIDGE_CORE_XFER_H__

#define SMDPORT_NAME_LEN 100

/*Number of bytes transferred between ext/smd ports*/
#define DUN_MAX_IPC_MSG_LEN           (DUN_IPC_HEADER_SIZE + DUN_MAXBUFSIZE)
#define DUN_IPC_HEADER_SIZE           (3)
#define DUN_IPC_CTRL_MSG_SIZE         (1)
#define DUN_IPC_MDM_STATUS_MSG_SIZE   (1)

#define DUN_MAXBUFSIZE                (32764)
#define DUN_MAXBUFSIZE_DL             (4028)

#define RMT_MODEM_SIGNAL_DTRDSR        0x01
#define RMT_MODEM_SIGNAL_RTSCTS        0x02
#define RMT_MODEM_SIGNAL_RI            0x04
#define RMT_MODEM_SIGNAL_DCD           0x08

/*To enable or disable throughput debugging*/
#define THROUGHPUT_DEBUGGING 1

#define INVALID_SOCKET     (-1)

#define YES                (1)
#define NO                 (0)

/*Port Parameter Structure*/
typedef struct {
    char smdportfname[SMDPORT_NAME_LEN];
    int  smdport_fd;
    int  conn_sk;
    volatile char rmt_mdm_bits; /* rfcomm channel bits */
    volatile int ulink_running;
    volatile int dlink_running;
    volatile int ctrl_running;
    pthread_t portctrlxfer_thread;
    pthread_t portdataxfr_thread_ulink;
    pthread_t portdataxfr_thread_dlink;
} dun_portparams_s;

typedef enum
{
    DUN_IPC_MSG_DUN_REQUEST = 0x00,
    DUN_IPC_MSG_DUN_RESPONSE,
    DUN_IPC_MSG_CTRL_REQUEST,
    DUN_IPC_MSG_CTRL_RESPONSE,
    DUN_IPC_MSG_MDM_STATUS
} dun_ipc_msg_type;

typedef enum
{
    DUN_CRTL_MSG_DISCONNECT_REQ = 0x00,
    DUN_CRTL_MSG_CONNECTED_RESP,
    DUN_CRTL_MSG_DISCONNECTED_RESP,
} dun_ipc_ctrl_msg_type;

typedef struct
{
    unsigned char msg_type;
    unsigned short int msg_len;
    union
    {
        unsigned char ctrl_msg;
        char modem_status;
        unsigned char xfer_buf[DUN_MAXBUFSIZE];
    };
}__attribute__((packed))dun_ipc_msg;

/* DUN internal errors*/
typedef enum
{
    DUN_ERR_NONE        = -1000,
    DUN_ERR_ILLEGAL_ARG = -1001,
    DUN_ERR_UNDEFINED   = -1002,
}dun_internal_error;


/*Declaration of the USB port parameters*/
extern dun_portparams_s pb_dun_portparams;

/*Starts the ctrl and data xfer threads*/
extern int pb_start_xfer_threads(dun_portparams_s *);
/*Stops the ctrl and data xfer threads*/
extern void pb_stop_xfer_threads(dun_portparams_s *);

/*Initializes the smd and ext ports*/
extern int pb_init_ports(dun_portparams_s *);
/*Closes the FDs to the smd and ext port*/
extern void pb_close_fd_to_ports(dun_portparams_s *);
/*Starts the thrgpt logging thread*/
void pb_start_thrgpt_thread(void);
/*Stops the thrgpt logging thread*/
void pb_stop_thrgpt_thread(void);


#endif /* __PORTBRIDGE_CORE_XFER_H__ */
