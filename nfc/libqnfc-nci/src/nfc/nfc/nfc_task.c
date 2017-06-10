/******************************************************************************
 *
 *  Copyright (C) 2010-2014 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/


/******************************************************************************
 *
 *  Entry point for NFC_TASK
 *
 ******************************************************************************/
#include <string.h>
#include "gki.h"
#include "gki_common.h"
#include "nfc_target.h"
#include "bt_types.h"

#if (NFC_INCLUDED == TRUE)
#include "nfc_api.h"
#include "nfc_hal_api.h"
#include "nfc_int.h"
#include "nci_hmsgs.h"
#include "rw_int.h"
#include "ce_int.h"
#if (NFC_RW_ONLY == FALSE)
#include "llcp_int.h"
#else
#define llcp_cleanup()
#endif

#if (defined (NFA_INCLUDED) && NFA_INCLUDED == TRUE)
#include "nfa_sys.h"
#include "nfa_dm_int.h"
#endif

/*NFCC chip version offset in nfcc TLV struct*/
#define NFCC_CHIP_VERSION_OFFSET 6

/*NFCC versions*/
#define NFCC_V_20                    20
#define NFCC_V_21                    21
#define NFCC_V_24                    24
#define NFCC_V_30                    30

extern UINT8 shutingdown_reason;
/*******************************************************************************
**
** Function         nfc_start_timer
**
** Description      Start a timer for the specified amount of time.
**                  NOTE: The timeout resolution is in SECONDS! (Even
**                          though the timer structure field is ticks)
**
** Returns          void
**
*******************************************************************************/
void nfc_start_timer (TIMER_LIST_ENT *p_tle, UINT16 type, UINT32 timeout)
{
    BT_HDR *p_msg;

    /* if timer list is currently empty, start periodic GKI timer */
    if (nfc_cb.timer_queue.p_first == NULL)
    {
        /* if timer starts on other than NFC task (scritp wrapper) */
        if (GKI_get_taskid () != NFC_TASK)
        {
            /* post event to start timer in NFC task */
            if ((p_msg = (BT_HDR *) GKI_getbuf (BT_HDR_SIZE)) != NULL)
            {
                p_msg->event = BT_EVT_TO_START_TIMER;
                GKI_send_msg (NFC_TASK, NFC_MBOX_ID, p_msg);
            }
        }
        else
        {
            /* Start nfc_task 1-sec resolution timer */
            GKI_start_timer (NFC_TIMER_ID, GKI_SECS_TO_TICKS (1), TRUE);
        }
    }

    GKI_remove_from_timer_list (&nfc_cb.timer_queue, p_tle);

    p_tle->event = type;
    p_tle->ticks = timeout;         /* Save the number of seconds for the timer */

    GKI_add_to_timer_list (&nfc_cb.timer_queue, p_tle);
}

/*******************************************************************************
**
** Function         nfc_remaining_time
**
** Description      Return amount of time to expire
**
** Returns          time in second
**
*******************************************************************************/
UINT32 nfc_remaining_time (TIMER_LIST_ENT *p_tle)
{
    return (GKI_get_remaining_ticks (&nfc_cb.timer_queue, p_tle));
}

/*******************************************************************************
**
** Function         nfc_process_timer_evt
**
** Description      Process nfc GKI timer event
**
** Returns          void
**
*******************************************************************************/
void nfc_process_timer_evt (void)
{
    TIMER_LIST_ENT  *p_tle;

    GKI_update_timer_list (&nfc_cb.timer_queue, 1);

    while ((nfc_cb.timer_queue.p_first) && (!nfc_cb.timer_queue.p_first->ticks))
    {
        p_tle = nfc_cb.timer_queue.p_first;
        GKI_remove_from_timer_list (&nfc_cb.timer_queue, p_tle);

        switch (p_tle->event)
        {
        case NFC_TTYPE_NCI_WAIT_RSP:
            nfc_ncif_cmd_timeout();
            break;

        case NFC_TTYPE_WAIT_2_DEACTIVATE:
            nfc_wait_2_deactivate_timeout ();
            break;

        default:
            NFC_TRACE_DEBUG2 ("nfc_process_timer_evt: timer:0x%x event (0x%04x)", p_tle, p_tle->event);
            NFC_TRACE_DEBUG1 ("nfc_process_timer_evt: unhandled timer event (0x%04x)", p_tle->event);
        }
    }

    /* if timer list is empty stop periodic GKI timer */
    if (nfc_cb.timer_queue.p_first == NULL)
    {
        GKI_stop_timer (NFC_TIMER_ID);
    }
}

/*******************************************************************************
**
** Function         nfc_stop_timer
**
** Description      Stop a timer.
**
** Returns          void
**
*******************************************************************************/
void nfc_stop_timer (TIMER_LIST_ENT *p_tle)
{
    GKI_remove_from_timer_list (&nfc_cb.timer_queue, p_tle);

    /* if timer list is empty stop periodic GKI timer */
    if (nfc_cb.timer_queue.p_first == NULL)
    {
        GKI_stop_timer (NFC_TIMER_ID);
    }
}

/*******************************************************************************
**
** Function         nfc_start_quick_timer
**
** Description      Start a timer for the specified amount of time.
**                  NOTE: The timeout resolution depends on including modules.
**                  QUICK_TIMER_TICKS_PER_SEC should be used to convert from
**                  time to ticks.
**
**
** Returns          void
**
*******************************************************************************/
void nfc_start_quick_timer (TIMER_LIST_ENT *p_tle, UINT16 type, UINT32 timeout)
{
    BT_HDR *p_msg;

    /* if timer list is currently empty, start periodic GKI timer */
    if (nfc_cb.quick_timer_queue.p_first == NULL)
    {
        /* if timer starts on other than NFC task (scritp wrapper) */
        if (GKI_get_taskid () != NFC_TASK)
        {
            /* post event to start timer in NFC task */
            if ((p_msg = (BT_HDR *) GKI_getbuf (BT_HDR_SIZE)) != NULL)
            {
                p_msg->event = BT_EVT_TO_START_QUICK_TIMER;
                GKI_send_msg (NFC_TASK, NFC_MBOX_ID, p_msg);
            }
        }
        else
        {
            /* Quick-timer is required for LLCP */
            GKI_start_timer (NFC_QUICK_TIMER_ID, ((GKI_SECS_TO_TICKS (1) / QUICK_TIMER_TICKS_PER_SEC)), TRUE);
        }
    }

    GKI_remove_from_timer_list (&nfc_cb.quick_timer_queue, p_tle);
    if (p_tle != NULL)
    {
        p_tle->event = type;
        p_tle->ticks = timeout; /* Save the number of ticks for the timer */
    }
    GKI_add_to_timer_list (&nfc_cb.quick_timer_queue, p_tle);
}




/*******************************************************************************
**
** Function         nfc_stop_quick_timer
**
** Description      Stop a timer.
**
** Returns          void
**
*******************************************************************************/
void nfc_stop_quick_timer (TIMER_LIST_ENT *p_tle)
{
    GKI_remove_from_timer_list (&nfc_cb.quick_timer_queue, p_tle);

    /* if timer list is empty stop periodic GKI timer */
    if (nfc_cb.quick_timer_queue.p_first == NULL)
    {
        GKI_stop_timer (NFC_QUICK_TIMER_ID);
    }
}

/*******************************************************************************
**
** Function         nfc_process_quick_timer_evt
**
** Description      Process quick timer event
**
** Returns          void
**
*******************************************************************************/
void nfc_process_quick_timer_evt (void)
{
    TIMER_LIST_ENT  *p_tle;

    GKI_update_timer_list (&nfc_cb.quick_timer_queue, 1);

    while ((nfc_cb.quick_timer_queue.p_first) && (!nfc_cb.quick_timer_queue.p_first->ticks))
    {
        p_tle = nfc_cb.quick_timer_queue.p_first;
        GKI_remove_from_timer_list (&nfc_cb.quick_timer_queue, p_tle);

        switch (p_tle->event)
        {
#if (NFC_RW_ONLY == FALSE)
        case NFC_TTYPE_LLCP_LINK_MANAGER:
        case NFC_TTYPE_LLCP_LINK_INACT:
        case NFC_TTYPE_LLCP_DATA_LINK:
        case NFC_TTYPE_LLCP_DELAY_FIRST_PDU:
            llcp_process_timeout (p_tle);
            break;
#endif
        case NFC_TTYPE_RW_T1T_RESPONSE:
            rw_t1t_process_timeout (p_tle);
            break;
        case NFC_TTYPE_RW_T2T_RESPONSE:
            rw_t2t_process_timeout (p_tle);
            break;
        case NFC_TTYPE_RW_T3T_RESPONSE:
            rw_t3t_process_timeout (p_tle);
            break;
        case NFC_TTYPE_RW_T4T_RESPONSE:
            rw_t4t_process_timeout (p_tle);
            break;
        case NFC_TTYPE_RW_I93_RESPONSE:
            rw_i93_process_timeout (p_tle);
            break;
#if (NFC_RW_ONLY == FALSE)
        case NFC_TTYPE_CE_T4T_UPDATE:
            ce_t4t_process_timeout (p_tle);
            break;
#endif
        default:
            NFC_TRACE_DEBUG1 ("nfc_process_quick_timer_evt: unhandled timer event (0x%04x)", p_tle->event);
            break;
        }
    }

    /* if timer list is empty stop periodic GKI timer */
    if (nfc_cb.quick_timer_queue.p_first == NULL)
    {
        GKI_stop_timer (NFC_QUICK_TIMER_ID);
    }
}

/*******************************************************************************
**
** Function         nfc_task_shutdown_nfcc
**
** Description      Handle NFC shutdown
**
** Returns          nothing
**
*******************************************************************************/
void nfc_task_shutdown_nfcc (void)
{
    BT_HDR        *p_msg;

    /* Free any messages still in the mbox */
    while ((p_msg = (BT_HDR *) GKI_read_mbox (NFC_MBOX_ID)) != NULL)
    {
        GKI_freebuf (p_msg);
    }

    nfc_gen_cleanup ();

    if (nfc_cb.flags & NFC_FL_POWER_OFF_SLEEP)
    {
        nfc_set_state (NFC_STATE_W4_HAL_CLOSE);
        nfc_cb.p_hal->close(shutingdown_reason);
    }
    else if (nfc_cb.flags & NFC_FL_POWER_CYCLE_NFCC)
    {
        nfc_set_state (NFC_STATE_W4_HAL_OPEN);
        nfc_cb.p_hal->power_cycle();
    }
    else
    {
        nfc_set_state (NFC_STATE_W4_HAL_CLOSE);
        nfc_cb.p_hal->close(shutingdown_reason);

        /* Perform final clean up */
        llcp_cleanup ();

        /* Stop the timers */
        GKI_stop_timer (NFC_TIMER_ID);
        GKI_stop_timer (NFC_QUICK_TIMER_ID);
#if (defined (NFA_INCLUDED) && NFA_INCLUDED == TRUE)
        GKI_stop_timer (NFA_TIMER_ID);
#endif
    }
}

/*****************************************************************************************
**
** Function         nfc_task_get_nfcc_info
**
** Description      Recieves nfcc information coming from nfcc and forward it further to
**                  to nfc service.
**
** Returns          nothing
**
*****************************************************************************************/
void nfc_task_get_nfcc_info(BT_HDR *p_msg)
{
    UINT8 *p = NULL;
    tNFC_RESPONSE   *nfcc_res = NULL;
    UINT32 nfcc_version;

    NFC_TRACE_DEBUG0 ("nfc_task_get_nfcc_info()");

    if(p_msg == NULL)
    {
        NFC_TRACE_DEBUG0 ("NFCC info null");
        return;
    }

    NFC_TRACE_DEBUG0 ("Sending NFCC info");

    p = (UINT8 *) (p_msg + 1) + p_msg->offset;

    nfa_dm_cb.nfcc_version = p[NFCC_CHIP_VERSION_OFFSET];

    switch(nfa_dm_cb.nfcc_version)
    {
        case NFCC_V_20:
           break;
        case NFCC_V_21:
           break;
        case NFCC_V_24:
           break;
        case NFCC_V_30:
           break;
        default:
           NFC_TRACE_DEBUG0 ("Wrong nfcc version.Read from conf file");
           GetNumValue("NFCC_VERSION", &nfcc_version, sizeof(nfcc_version));
           if(nfcc_version)
           {
               nfa_dm_cb.nfcc_version = nfcc_version;
           }
           else
           {
               /*default 3.0*/
               nfa_dm_cb.nfcc_version = NFCC_V_30;
           }
           break;
    }

    NFC_TRACE_DEBUG1 ("nfa_dm_cb.nfcc_version = %d",nfa_dm_cb.nfcc_version);

    nfcc_res = (tNFC_RESPONSE*)malloc(sizeof(tNFC_RESPONSE));
    if(nfcc_res != NULL)
    {
        nfcc_res->nfcc_info_data.len = p_msg->len;
        nfcc_res->nfcc_info_data.value = (UINT8*)malloc( nfcc_res->nfcc_info_data.len);
        if(nfcc_res->nfcc_info_data.value != NULL)
        {
            if(nfcc_res->nfcc_info_data.len != 0)
            {
                memcpy(nfcc_res->nfcc_info_data.value,p,nfcc_res->nfcc_info_data.len);
                if (nfc_cb.p_resp_cback)
                {
                    (*nfc_cb.p_resp_cback) (NFC_NFCC_INFO_REVT, nfcc_res);
                }
            }
            /*free acquired mem*/
            free(nfcc_res->nfcc_info_data.value);
            nfcc_res->nfcc_info_data.value = NULL;
        }
        else
        {
            NFC_TRACE_DEBUG0 ("Memory allocation failed");
        }
        /*free acquired mem*/
        free(nfcc_res);
        nfcc_res = NULL;
    }
    else
    {
        NFC_TRACE_DEBUG0 ("Memory allocation failed for nfcc_res");
    }
}


/*******************************************************************************
**
** Function         nfc_task
**
** Description      NFC event processing task
**
** Returns          nothing
**
*******************************************************************************/
UINT32 nfc_task (UINT32 param)
{
    UINT16  event;
    BT_HDR  *p_msg;
    BOOLEAN free_buf;

    /* Initialize the nfc control block */
    memset (&nfc_cb, 0, sizeof (tNFC_CB));
    nfc_cb.trace_level = NFC_INITIAL_TRACE_LEVEL;
    nfc_cb.get_nfcc_info = FALSE;


    NFC_TRACE_DEBUG0 ("NFC_TASK started.");

    /* main loop */
    while (TRUE)
    {
        event = GKI_wait (0xFFFF, 0);

        /* Handle NFC_TASK_EVT_TRANSPORT_READY from NFC HAL */
        if (event & NFC_TASK_EVT_TRANSPORT_READY)
        {
            NFC_TRACE_DEBUG0 ("NFC_TASK got NFC_TASK_EVT_TRANSPORT_READY.");

            /* Reset the NFC controller. */
            nfc_set_state (NFC_STATE_CORE_INIT);
            nci_snd_core_reset (NCI_RESET_TYPE_RESET_CFG);
        }

        if (event & NFC_MBOX_EVT_MASK)
        {
            /* Process all incoming NCI messages */
            while ((p_msg = (BT_HDR *) GKI_read_mbox (NFC_MBOX_ID)) != NULL)
            {
                free_buf = TRUE;

                /* Determine the input message type. */
                switch (p_msg->event & BT_EVT_MASK)
                {
                    case BT_EVT_TO_NFC_NCI:
                        if((!nfc_cb.get_nfcc_info))
                        {
                            nfc_task_get_nfcc_info(p_msg);
                            nfc_cb.get_nfcc_info = TRUE;
                        }
                        else
                        {
                            free_buf = nfc_ncif_process_event (p_msg);
                        }

                        break;

                    case BT_EVT_TO_START_TIMER :
                        /* Start nfc_task 1-sec resolution timer */
                        GKI_start_timer (NFC_TIMER_ID, GKI_SECS_TO_TICKS (1), TRUE);
                        break;

                    case BT_EVT_TO_START_QUICK_TIMER :
                        /* Quick-timer is required for LLCP */
                        GKI_start_timer (NFC_QUICK_TIMER_ID, ((GKI_SECS_TO_TICKS (1) / QUICK_TIMER_TICKS_PER_SEC)), TRUE);
                        break;

                    case BT_EVT_TO_NFC_MSGS:
                        nfc_main_handle_hal_evt ((tNFC_HAL_EVT_MSG*)p_msg);
                        break;

                    default:
                        NFC_TRACE_DEBUG1 ("nfc_task: unhandle mbox message, event=%04x", p_msg->event);
                        break;
                }
                /*check if the buffer status is BUF_STATUS_UNLINKED*/
                if (BUF_STATUS_UNLINKED == GKI_getbufstatus(p_msg) && free_buf)
                {
                    GKI_freebuf (p_msg);
                }
            }
        }

        /* Process gki timer tick */
        if (event & NFC_TIMER_EVT_MASK)
        {
            nfc_process_timer_evt ();
        }

        /* Process quick timer tick */
        if (event & NFC_QUICK_TIMER_EVT_MASK)
        {
            nfc_process_quick_timer_evt ();
        }

#if (defined (NFA_INCLUDED) && NFA_INCLUDED == TRUE)
        if (event & NFA_MBOX_EVT_MASK)
        {
            while ((p_msg = (BT_HDR *) GKI_read_mbox (NFA_MBOX_ID)) != NULL)
            {
                nfa_sys_event (p_msg);
            }
        }

        if (event & NFA_TIMER_EVT_MASK)
        {
            nfa_sys_timer_update ();
        }
#endif

    }


    NFC_TRACE_DEBUG0 ("nfc_task terminated");

    return 0;
}

#endif /* NFC_INCLUDED == TRUE */
