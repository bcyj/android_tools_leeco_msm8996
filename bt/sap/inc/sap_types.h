/******************************************************************************NS

  @file    sap_types.h
  @brief

  DESCRIPTION

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

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
#ifndef __SAP_TYPES_H__
#define __SAP_TYPES_H__

#include <sys/socket.h>
#include "sap_constants.h"


typedef struct
{
        unsigned char id;
        unsigned char reserved;
        unsigned short int length;
        unsigned char value[0];
        /*Padding of 0-3 bytes
        based on the length of
        parameter*/

}__attribute__((packed))sap_param;

typedef struct
{
        unsigned char id;
        unsigned char param_count;
        unsigned short int reserved;
        unsigned char params[0];
}__attribute__((packed))sap_msg;

typedef struct
{
        unsigned char msg_type;
        unsigned short int msg_len;
        union
        {
                unsigned char ctrl_msg;
                sap_msg msg;
        };
}__attribute__((packed))sap_ipc_msg;

typedef struct {
        uint8_t b[6];
        /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} __attribute__((packed)) bdaddr_t;

struct sockaddr_rc {
        /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
        sa_family_t rc_family;
        bdaddr_t rc_bdaddr;
        uint8_t rc_channel;
};

sap_internal_error send_status_ind(int a_socket, sap_status_change a_stat);
sap_internal_error send_conn_resp(int a_socket, sap_connection_status a_stat, unsigned short a_max_msg_sz);
sap_internal_error send_disconn_resp(int a_socket );
sap_internal_error send_disconn_ind(int a_socket, sap_disconnect_type a_type);
sap_internal_error send_tx_apdu_resp(int a_socket, sap_result_code a_result, unsigned char* a_resp_apdu, int a_apdu_len);
sap_internal_error send_tx_atr_resp(int a_socket, sap_result_code a_result, unsigned char* a_resp_atr, int a_atr_len);
sap_internal_error send_sim_off_resp(int a_socket, sap_result_code a_result);
sap_internal_error send_sim_on_resp(int a_socket, sap_result_code a_result);
sap_internal_error send_sim_reset_resp(int a_socket, sap_result_code a_result);
sap_internal_error send_tx_card_stat(int a_socket, sap_result_code a_result, unsigned char* a_status, int a_len);
sap_internal_error send_error_resp(int a_socket );
void set_sap_state(sap_state_type a_sap_state);


#endif /*__SAP_TYPES_H__*/
