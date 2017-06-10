#ifndef AUDTP_H
#define AUDTP_H

/** 
\file **************************************************************************
*
*                                       A T P    H E A D E R    F I L E
*
*DESCRIPTION
* This header file contains all the definitions necessary for other
* modules to interface and use the protocol(ATP protocol) which works on top of diag .
*  
*  This file is common to both protocol component on PC and protocol module on AMSS.
*  PC only/phone only code is wrapped with in #ifdef WINPC
*  
* Copyright (c) 2010-2011 by Qualcomm Technologies, Inc.
* All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************
*/

/*
      --------------------
      |include files                |
      --------------------
 */

#include "acdb_includes.h"

/** Definitions for char_t, word, etc. */
#include "comdef.h"
#include "audtpi.h"

#include "diagcmd.h"
#include "diagpkt.h"
#include "diag_lsm.h"

/*
      --------------------
      | Macros and type defs |
      --------------------
 */
#define LOCAL_DIAG_MAX_RX_PKT_SIZ (((DIAG_MAX_TX_PKT_SIZ) <= (DIAG_MAX_RX_PKT_SIZ)) ? (DIAG_MAX_TX_PKT_SIZ) : (DIAG_MAX_RX_PKT_SIZ))

#define DIAG_MAX_PKT_HEADER_SIZ 0x20
/**< Maximum Diag request packet size*/
#define DIAG_REQ_PKT_SIZE (LOCAL_DIAG_MAX_RX_PKT_SIZ - DIAG_MAX_PKT_HEADER_SIZ)
/**< Maximum Diag response packet size*/
#define DIAG_RES_PKT_SIZE (LOCAL_DIAG_MAX_RX_PKT_SIZ - DIAG_MAX_PKT_HEADER_SIZ)


/**
* Structure definition for diag packet header
*/
typedef struct diag_pkt_header_tag
{
    uint8_t cmdCode;/**< diag command code*/
    uint8_t subsysId;/**< subsystem ID*/
    uint16_t subsysCmdCode;/**< command code with in susbsystem*/
} diag_pkt_header_struct;

/**
* Structure definition for diag request packet<br>
*Packet size is fixed.
*/
typedef struct diag_pkt_req_tag
{
    diag_pkt_header_struct header;/**< diag packet header*/
    char_t payload_req[DIAG_REQ_PKT_SIZE];/**< buffer to hold payload*/
} diag_pkt_req_struct;

/**
* Structure definition for diag response packet<br>
*Packet size is fixed.
*/
typedef struct diag_pkt_resp_tag
{
    diag_pkt_header_struct header;/**< diag packet header*/
    char_t payload_resp[DIAG_RES_PKT_SIZE];/**< buffer to hold payload*/
} diag_pkt_resp_struct;

#ifdef WINPC
/**
* Structure definition to hold a whole context data related to a ATP session.on 
*TX side
*/
typedef struct atp_pc_context_tag
{
/**
* Pointer to the head of a linked list, which holds the atp frames of a request 
* buffer
*/
    atp_frame_dblnk_lst_struct *start_node_ptr;
/**
*pointer to the current frame in request buffer, which is being processed
*/
    atp_frame_dblnk_lst_struct *current_node_ptr;
/**
*Pointer to the response buffer context
*/
    buffer_context_struct resp_buf_cntxt;
/**
*pointer the call back function which sends request packet synchronusly and 
*returns response packet.
*/
    bool_t (*send_atp_frame_ptr)(diag_pkt_req_struct *, diag_pkt_resp_struct *);
/**
*pointer the call back function in application layer which receives the 
*response buffer
*/
    void (*receive_resp_buffer_ptr)(char_t *resp_buf_ptr, uint32_t buf_length);
} atp_pc_context_struct;
#else
/**
* Structure definition to hold a whole context data related to a ATP session.on 
*RX side
*/
typedef struct atp_phone_context_tag
{
/**
* Pointer to the head of a linked list, which holds the atp frames of a response 
* buffer
*/
    atp_frame_dblnk_lst_struct *resp_start_node_ptr;
/**
*pointer to the current frame in response buffer, which is being processed
*/
    atp_frame_dblnk_lst_struct *resp_current_node_ptr;
/**
*Pointer to the response buffer context
*/
    buffer_context_struct req_buf_cntxt;
/**
*pointer the response frame generated for request frame
*/
    atp_frame_struct* atp_current_resp_frame_ptr;
/**
*Inidicates whether the RX side is expecting a frame with GNF flag.
*/
    bool_t is_next_frame_gnf;
/**
*Pointer to a call back function in application layer, which takes the request 
buffer and returns a response buffer
*/
    void (*receive_req_buffer_ptr)
        (
         char_t *req_buf_ptr, uint32_t req_buf_length,
         char_t **resp_buf_ptr, uint32_t *resp_buf_length
        );
} atp_phone_context_struct;
#endif

/*
      ---------------------
      |Exported functions       |
      ---------------------
 */
#ifdef WINPC

#ifdef __cplusplus
extern "C" 
{
#endif
/**
* FUNCTION : atp_send_buffer

* DESCRIPTION :
* Send a request buffer to target.
* If buffer sent successfully, returns the response through the call back 
*function: receive_resp_buffer_ptr of context_ptr.
*
* DEPENDENCIES : send_atp_frame_ptr memeber in context_ptr must be pointing to 
*                           appropriate function.
* PARAMS:
* buffer_ptr - pointer to the input buffer 
* buffer_length - bufer length
* atp_pc_context_ptr - ponter to ATP session(on PC side)
*
* RETURN VALUE : If sent successfully returns TRUE or FALSE

* SIDE EFFECTS : Frees buffer_ptr and resets the pointer to NULL.
 */
extern bool_t atp_send_buffer (
        char_t* buffer_ptr,
        uint32_t buffer_length,
        atp_pc_context_struct *atp_pc_context_ptr
        );
#ifdef __cplusplus
}
#endif
#else
/**
* FUNCTION : atp_receive_diag_pkt

* DESCRIPTION :
* Receive a request diag packet from acm diag dispatcher (avsACMdiag_cmd())
*.and return a response diag packet.
*
* DEPENDENCIES : receive_req_buffer_ptr memeber in atp_phone_context_ptr must be 
pointing to.appropriate function, which can process the request buffer and 
provide a response buffer.

* PARAMS:
*  atp_diag_req_pkt_ptr - pointer to the diag packet which contains the atp frame from request buffer
*  atp_diag_rsp_pkt_ptr - pointer to the diag packet which contains the atp frame from response buffer
*  atp_phone_context_ptr - pointer to the phone context to whome request to be sent and resposne received
*
* RETURN VALUE : If sent successfully returns TRUE or FALSE

* SIDE EFFECTS : Will NOT Free response buffer.
 */
extern void atp_receive_diag_pkt (
        diag_pkt_req_struct *atp_diag_req_pkt_ptr,
        diag_pkt_resp_struct **atp_diag_rsp_pkt_ptr,
        atp_phone_context_struct *atp_phone_context_ptr
        );
extern void atp_diag_init(void);
#endif
#endif

