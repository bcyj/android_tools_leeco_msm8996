#ifndef ACTP_H
#define ACTP_H
/** 
  \file **************************************************************************
 *
 *                                       A C T P   H E A D E R    F I L E
 *
 *DESCRIPTION
 * This header file contains all the definitions necessary for ACTP protocol 
 * layer to initialize  ACTP diag dispatcher.
 * This actp works in both ARM9 and ARM11
 *  
 * Copyright (c) 2010-2012 by Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *******************************************************************************
 */

/*
   --------------------
   |include files                |
   --------------------
   */

/*
   --------------------
   |Macros                      |
   --------------------
   */
/*ACTP subsystem command ID Range (inclusive)*/

/** 
 *ACTP subsystem command code range start ID
 */
#define ACTP_CMD_ID_START 2051 
#define ACTP_CMD_ID_END 2100 

/*
   --------------------
   |External functions      |
   --------------------
   */
/**
 * FUNCTION : actp_diag_init

 * DESCRIPTION :
 *Registers the subsystem diag dispathcer with diag dispathcer.<br>
 * Initializes the actp session.
 *
 * DEPENDENCIES : actp_diag_table[] should be initialized.<br>
 * And actp_diag_table should have proper sub system command.code range and 
 pointer to the dispatcher function.
 * Diag Packet Service must be available.
 *
 * PARAMS:
 *   callback_function - the callback function for diag packet
 *
 * RETURN VALUE : None

 * SIDE EFFECTS : None
 */
extern void actp_diag_init(
        void (*callback_function)(char_t*, uint32_t, char_t**, uint32_t*)
        );

/**
 * FUNCTION : avsACTP_diag_cmd
 *
 * DESCRIPTION :
 *This is the entry point to ACTP when seen from PC.
 *and is the sub system diag dispathcer for Audio Calibration Tranporter Protocol
 *Receives a diag packet meant for ACTP and passes to protocol layer and returns 
 the response diag packet.
 *
 * DEPENDENCIES :
 *
 * PARAMS:
 *   request - the diag request packet (packed)
 *   length - the length of the diag request packet
 *
 * RETURN VALUE : returns response diag packet.
 *In case of error, returns diag packet with error code DIAG_BAD_CMD_F
 *
 * SIDE EFFECTS : None
 */
extern PACKED void *  avsACTP_diag_cmd ( 
        PACKED void *request,
        word length
        );

#endif //ACTP_H
