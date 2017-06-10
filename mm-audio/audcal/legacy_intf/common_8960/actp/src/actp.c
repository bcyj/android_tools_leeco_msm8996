/** 
  \file **************************************************************************
 *
 *  A U D I O   C A L I B R A T I O N   T R A N S P O R T E R   P R O T O C O L
 *
 *DESCRIPTION
 * This file contains the implementation of ACTP diag dispatcher 
 *
 *INITIALIZATION REQUIREMENTS:
 * actp_diag_init must be called to initialise ACTP module.
 * actp_diag_init must be called ~AFTER~ initialization of SOUND TASK
 * A pointer to a call-back function with format (char_t*, uint32_t, char_t**, uint32_t*)
 * needs to be passed in.
 *
 * Copyright (c) 2010-2012 by Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *******************************************************************************
 */

/*
   -------------------------------
   |Include Files                |
   -------------------------------
   */
#include "audtp.h"
#include "actp.h"
#include "acdb_includes.h"

/*
   ----------------------------------
   | Static Variable Definitions    |
   ----------------------------------
   */
/**
 *this is actp diag dispatch table that needs to be registered with diag 
 dispathcer to route the diag packets sent from PC to actp diag dispatcher.
 */
static const diagpkt_user_table_entry_type  actp_diag_table[] =
{
    {
        ACTP_CMD_ID_START,
        ACTP_CMD_ID_END,
        avsACTP_diag_cmd
    }
};

/** ACTP session*/
static atp_phone_context_struct actp_phone_context; 

/*
   ----------------------------------
   | Externalized Function Definitions    |
   ----------------------------------
   */
/**
 * FUNCTION : actp_diag_init

 * DESCRIPTION : * Registers the subsystem diag dispathcer with diag dispathcer.<br>
 * Initializes the actp session.and give it a call-back function.
 *
 * DEPENDENCIES : actp_diag_table[] should be initialized.<br>
 * And actp_diag_table should have proper sub system command.code range and <br>
 pointer to the dispatcher function. A pointer to a call-back function with <br>
 format (char_t*, uint32_t, char_t**, uint32_t*) needs to be passed in.
 *
 * RETURN VALUE : None

 * SIDE EFFECTS : None
 */
void actp_diag_init(
        void (*pfnCallback_Function)(char_t*, uint32_t, char_t**, uint32_t*)
        )
{
    boolean bDiagInit = FALSE;
    LOGE("actp_diag_init: call diag init function with %08X\n",
          (uint32_t)pfnCallback_Function);
    bDiagInit = Diag_LSM_Init(NULL);
    if (!bDiagInit)
    {
       LOGE("actp_diag_init: diag init failed\n");
       return;
    }
    /**Register command range and diag subsystem dispatcher*/
    DIAGPKT_DISPATCH_TABLE_REGISTER (DIAG_SUBSYS_AUDIO_SETTINGS,
            actp_diag_table);
    memset(&actp_phone_context,0,sizeof(atp_phone_context_struct));
    /** Set the pointer to the application callback function which receives the 
      request buffer and provides a response*/
    actp_phone_context.receive_req_buffer_ptr = pfnCallback_Function;
}

/**
 * FUNCTION : avsACTP_diag_cmd
 *
 * DESCRIPTION :
 *This is the entry point to ACTP when seen from PC.
 *and is the sub system diag dispathcer for Audio Calibration manager
 *Receives a diag packet ment for ACTP and passes to protocol layer and returns 
 the response diag packet.
 *
 * DEPENDENCIES :
 *
 * RETURN VALUE : returns response diag packet.
 *In case of error, returns diag packet with error code DIAG_BAD_CMD_F
 *
 * SIDE EFFECTS : None
 */
PACKED void * avsACTP_diag_cmd(
        PACKED void *req_ptr,
        word pkt_len
        )
{  

    diag_pkt_resp_struct *rsp_diag_pkt_ptr  = NULL;
    diag_pkt_resp_struct *actp_rsp_ptr = NULL;
    diag_pkt_req_struct request; 
    word actp_rsp_pkt_len = 0;
    /** Initialize subsystem command code*/
    uint16_t command_code = ACTP_CMD_ID_START;
    if (NULL != req_ptr)
    {
        /**Create an unpacked version of diag packet*/
        memcpy((void *)&request, (void *)req_ptr,pkt_len);
        /**Get command code*/
        command_code = (uint16_t) diagpkt_subsys_get_cmd_code((void *) &request);
        /** Verify is the command code valid */
        if (command_code < ACTP_CMD_ID_START || command_code > ACTP_CMD_ID_END)
        {
            /**If not in the range return error*/
            rsp_diag_pkt_ptr = (diag_pkt_resp_struct*) diagpkt_err_rsp(DIAG_BAD_CMD_F, req_ptr, pkt_len);
        }
        else
        {
            /**Send diag packet to protocol layer*/

            /**Call ATP to process the packet*/
            atp_receive_diag_pkt(&request, &actp_rsp_ptr, &actp_phone_context);
            /**If we have not received response frame from protocol layer return error*/
            if(NULL == actp_rsp_ptr)
            {
                rsp_diag_pkt_ptr = (diag_pkt_resp_struct*) diagpkt_err_rsp(DIAG_BAD_CMD_F, req_ptr, pkt_len);
            }
            else
            {
                actp_rsp_pkt_len = DIAG_RES_PKT_SIZE + sizeof(diag_pkt_header_struct);
                /**
                  Create a diag packet using diag api . It automatically delets the 
                  packet after we commit the packet.
                  */
                rsp_diag_pkt_ptr = (diag_pkt_resp_struct*) diagpkt_subsys_alloc (
                        DIAG_SUBSYS_AUDIO_SETTINGS,(uint16_t) command_code, actp_rsp_pkt_len);
                //Klocwork fix: check rsp_diag_pkt_ptr if it is a null ptr.
                if (rsp_diag_pkt_ptr == NULL)
                {
                   rsp_diag_pkt_ptr = (diag_pkt_resp_struct*) diagpkt_err_rsp(DIAG_BAD_CMD_F, req_ptr, pkt_len);
                   if (actp_rsp_ptr != NULL)
                   {
                      free(actp_rsp_ptr);
                   }
                   return (rsp_diag_pkt_ptr);
                }
                memcpy(rsp_diag_pkt_ptr,actp_rsp_ptr,actp_rsp_pkt_len);
                /**Send the diag packet .This will be received on PC side as response 
                  packet*/
                if (rsp_diag_pkt_ptr == NULL)
                //Klocwork fix: check rsp_diag_pkt_ptr if it is a null ptr.
                {
                    rsp_diag_pkt_ptr = (diag_pkt_resp_struct*) diagpkt_err_rsp(DIAG_BAD_CMD_F, req_ptr, pkt_len);
                    return (rsp_diag_pkt_ptr);
                }
                diagpkt_commit(rsp_diag_pkt_ptr);
                rsp_diag_pkt_ptr = NULL;
                /**Free the response packet we received from protocol layer*/
                free(actp_rsp_ptr);
            }
        }  
    }
    else
    {
        rsp_diag_pkt_ptr = (diag_pkt_resp_struct*) diagpkt_err_rsp(DIAG_BAD_CMD_F, req_ptr, pkt_len);
    }
    return (rsp_diag_pkt_ptr);
}

