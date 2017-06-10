/*
 * Copyright (c) 2005 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 */
/******************************************************************************
*
*                                 OMX Ittiam
*
*                     ITTIAM SYSTEMS PVT LTD, BANGALORE
*                             COPYRIGHT(C) 2011
*
*  This program is proprietary to ittiam systems pvt. ltd.,and is protected
*  under indian copyright act as an unpublished work.its use and disclosure
*  is limited by the terms and conditions of a license agreement.it may not
*  be copied or otherwise  reproduced or disclosed  to persons  outside the
*  licensee's   organization  except  in  accordance   with  the  terms and
*  conditions  of such  an agreement. all copies and reproductions shall be
*  the property of ittiam systems pvt. ltd.  and  must bear this  notice in
*  its entirety.
*
******************************************************************************/

#ifdef __cplusplus
//extern "C" {
#endif /* __cplusplus */

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Index.h>
#include <OMX_Image.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include <OMX_Other.h>
#include <OMX_QCOMExtns.h>

#include <wchar.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
//#include <sys/ioctl.h>
//#include <sys/select.h>
#include <errno.h>
#include <pthread.h>

#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include "datatypedef.h"
#include "iv.h"
#include "ivd.h"

#ifdef ENABLE_MPEG4
#include "imp4d_cxa8.h"
#endif
#ifdef ENABLE_MPEG2
#include "imp2d_cxa8.h"
#endif
#ifdef ENABLE_H264
#include "ih264d_cxa8.h"
#endif
#ifdef ENABLE_HEVC
#include "ihevcd_cxa.h"
#endif

#include <OMX_Ittiam.h>
#include <OMX_Ittiam_Vdec.h>

#include<utils/Log.h>

#define IV_ISFATALERROR(x)         (((x) >> IVD_FATALERROR) & 0x1)

unsigned long long prev_stop_ts, start_ts, stop_ts;

//Average Dec time changes
long long total_decode_time, total_delay_time, num_decoded_buffers;

long long itGetMs(void)
{
    struct timeval t;
    long long currTime;

    if(gettimeofday(&t, NULL) == -1)
    {
        printf("Error in gettimeofday. It has returned -1. \n");
    }
    currTime = ((t.tv_sec * 1000 * 1000) + (t.tv_usec));
    return currTime;
}

void ittiam_video_decoder_deinit(VIDDECTYPE* pVidDec);
void FlushOutputBuffers(VIDDECTYPE* pVidDec);
IV_API_CALL_STATUS_T ittiam_video_decoder_init(VIDDECTYPE* pVidDec);
void ittiam_video_decoder_flush(VIDDECTYPE* pVidDec);
void video_skipb_frames(VIDDECTYPE* pVidDec);
void avc_set_deblocking_lvl(VIDDECTYPE* pVidDec, UWORD32 level);
void ittiam_video_set_display_frame(VIDDECTYPE* pVidDec);
void ittiam_video_release_display_frame(VIDDECTYPE* pVidDec,
                                        UWORD32 disp_buf_id);
void ittiam_video_decoder_process(VIDDECTYPE* pVidDec,
                                  OMX_BUFFERHEADERTYPE *pInBufHdr,
                                  OMX_BUFFERHEADERTYPE *pOutBufHdr);
void ittiam_video_decoder_reset(VIDDECTYPE* pVidDec);
/*
 *  Component Thread
 *    The ComponentThread function is exeuted in a separate pThread and
 *    is used to implement the actual component functions.
 */
/*****************************************************************************/

void* ComponentThread(void* pThreadData)
{
    int i, fd1;
    fd_set rfds;
    WORD32 cmddata;
    ThrCmdType cmd;

    // Variables related to decoder buffer handling
    OMX_BUFFERHEADERTYPE *pInBufHdr = NULL;
    OMX_BUFFERHEADERTYPE *pOutBufHdr = NULL;
    OMX_MARKTYPE *pMarkBuf = NULL;

    // Variables related to decoder timeouts
    struct timespec abstime;
    int ret_val;
    int nTimeout;

    // Recover the pointer to my component specific data
    VIDDECTYPE* pVidDec = (VIDDECTYPE*)pThreadData;

    while(1)
    {
        fd1 = pVidDec->cmdpipe[0];
        FD_ZERO(&rfds);
        FD_SET(fd1, &rfds);

        // Check for new command
        i = select(pVidDec->cmdpipe[0] + 1, &rfds, NULL, NULL, NULL);

        if(FD_ISSET(pVidDec->cmdpipe[0], &rfds))
        {
            // retrieve command and data from pipe
            pthread_mutex_lock(&pVidDec->pipes_mutex);
            read(pVidDec->cmdpipe[0], &cmd, sizeof(cmd));
            read(pVidDec->cmddatapipe[0], &cmddata, sizeof(cmddata));
            pthread_mutex_unlock(&pVidDec->pipes_mutex);

            // State transition command
            if(cmd == SetState)
            {
                ITTIAM_DEBUG("SetState Command");
                pthread_mutex_lock(&pVidDec->signal_mutex);
                pVidDec->cmd_pending = 0;
                pthread_mutex_unlock(&pVidDec->signal_mutex);

                // If the parameter states a transition to the same state
                //   raise a same state transition error.
                if(pVidDec->state == (OMX_STATETYPE)(cmddata))
                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventError,
                                                      OMX_ErrorSameState, 0,
                                                      NULL);
                else
                {
                    // transitions/callbacks made based on state transition table
                    // cmddata contains the target state
                    switch((OMX_STATETYPE)(cmddata))
                    {
                        case OMX_StateInvalid:
                            pVidDec->state = OMX_StateInvalid;
                            data_sync_barrier();
                            pVidDec->pCallbacks->EventHandler(
                                            pVidDec->hSelf, pVidDec->pAppData,
                                            OMX_EventError,
                                            OMX_ErrorInvalidState, 0, NULL);
                            pVidDec->pCallbacks->EventHandler(
                                            pVidDec->hSelf, pVidDec->pAppData,
                                            OMX_EventCmdComplete,
                                            OMX_CommandStateSet, pVidDec->state,
                                            NULL);
                            break;
                        case OMX_StateLoaded:
                            if(pVidDec->state == OMX_StateIdle
                                            || pVidDec->state
                                                            == OMX_StateWaitForResources)
                            {
                                ret_val = 0;

                                pthread_mutex_lock(&pVidDec->signal_mutex);

                                // Transition happens only when the ports are unpopulated
                                if((pVidDec->sInBufList.nAllocSize > 0)
                                                || pVidDec->sOutBufList.nAllocSize
                                                                > 0)
                                {

                                    //gettimeofday(&abstime,NULL);
                                    clock_gettime(CLOCK_REALTIME, &abstime);
                                    abstime.tv_sec += OMX_TIMEOUT_SEC;
                                    ITTIAM_DEBUG("Waiting for buffers to be freed");
                                    ret_val = pthread_cond_timedwait(
                                                    &pVidDec->buffers_signal,
                                                    &pVidDec->signal_mutex,
                                                    &abstime);

                                }
                                pthread_mutex_unlock(&pVidDec->signal_mutex);

                                if(ret_val == 0)
                                {
                                    // DeInitialize the decoder when moving from Idle to Loaded
                                    if(pVidDec->state == OMX_StateIdle
                                                    && pVidDec->initdone == 1)
                                    {
                                        pVidDec->initdone = 0;
                                        ittiam_video_decoder_deinit(pVidDec);
                                    }

                                    pVidDec->state = OMX_StateLoaded;
                                    data_sync_barrier();
                                    pVidDec->pCallbacks->EventHandler(
                                                    pVidDec->hSelf,
                                                    pVidDec->pAppData,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandStateSet,
                                                    pVidDec->state, NULL);

                                }
                                else
                                {
                                    // Transition to loaded failed
                                    pVidDec->pCallbacks->EventHandler(
                                                    pVidDec->hSelf,
                                                    pVidDec->pAppData,
                                                    OMX_EventError,
                                                    OMX_ErrorTimeout, 0, NULL);

                                }
                            }
                            else
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            break;
                        case OMX_StateIdle:

                            if(pVidDec->state == OMX_StateInvalid)
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            else
                            {
                                // Return buffers if currently in pause and executing
                                if(pVidDec->state == OMX_StatePause
                                                || pVidDec->state
                                                                == OMX_StateExecuting)
                                {
                                    if(pInBufHdr)
                                    {
                                        // Return input buffer to client for refill
                                        pVidDec->pCallbacks->EmptyBufferDone(
                                                        pVidDec->hSelf,
                                                        pVidDec->pAppData,
                                                        pInBufHdr);
                                        pInBufHdr = NULL;
                                    }
                                    ittiam_video_decoder_flush(pVidDec);
                                    ListFlushEntries(pVidDec->sInBufList,
                                                     pVidDec);

                                    ITTIAM_DEBUG("Calling ListFlush or FlushOut");

                                    if(1 == pVidDec->share_disp_buf)
                                        FlushOutputBuffers(pVidDec);
                                    else
                                        ListFlushEntries(pVidDec->sOutBufList,
                                                         pVidDec);
                                    ITTIAM_DEBUG("Calling ListFlush or FlushOut done");
                                }

                                ret_val = 0;
                                ITTIAM_DEBUG("pthread_mutex_lock for signal");
                                pthread_mutex_lock(&pVidDec->signal_mutex);

                                // All "enabled" Ports have to be "populated", before transition completes
                                if((pVidDec->sInPortDef.bEnabled
                                                != pVidDec->sInPortDef.bPopulated)
                                                || (pVidDec->sOutPortDef.bEnabled
                                                                != pVidDec->sOutPortDef.bPopulated))
                                {
                                    pVidDec->bufferallocationpending = 1;
                                    data_sync_barrier();
                                    ITTIAM_LOG("bufferallocationpending");
                                    do {
                                          struct timespec time;
                                          clock_gettime(CLOCK_REALTIME, &time);
                                          time.tv_sec += 1;
                                          ret_val = pthread_cond_timedwait(&pVidDec->buffers_signal,&pVidDec->signal_mutex,&time);
                                          if (ret_val != ETIMEDOUT)
                                              break;
                                          ALOGE("Timed out waiting for buffer allocation");
                                    } while(!pVidDec->exitPending);

                                }
                                ITTIAM_DEBUG("pthread_mutex_unlock for signal");
                                pthread_mutex_unlock(&pVidDec->signal_mutex);

                                if(pVidDec->bufferallocationpending)
                                {
                                    pVidDec->state = OMX_StateInvalid;
                                    data_sync_barrier();
                                    pVidDec->pCallbacks->EventHandler(
                                                    pVidDec->hSelf,
                                                    pVidDec->pAppData,
                                                    OMX_EventError,
                                                    OMX_ErrorInvalidState, 0,
                                                    NULL);
                                    ITTIAM_LOG("Invalid state");
                                }
                                else
                                {
                                    // Initialize the decoder when moving from Loaded to Idle
                                    if(pVidDec->state == OMX_StateLoaded)
                                    {
                                        IV_API_CALL_STATUS_T e_dec_status;
                                        e_dec_status =
                                                        ittiam_video_decoder_init(
                                                                        pVidDec);
                                        if(IV_SUCCESS != e_dec_status)
                                        {
                                            pVidDec->pCallbacks->EventHandler(
                                                            pVidDec->hSelf,
                                                            pVidDec->pAppData,
                                                            OMX_EventError,
                                                            OMX_ErrorInvalidState,
                                                            OMX_StateInvalid,
                                                            NULL);
                                        }
                                        else
                                        {
                                            pVidDec->initdone = 1;
                                        }
                                    }

                                    ITTIAM_DEBUG("Sending OMX_EventCmdComplete for Idle");
                                    pVidDec->state = OMX_StateIdle;
                                    data_sync_barrier();
                                    pVidDec->pCallbacks->EventHandler(
                                                    pVidDec->hSelf,
                                                    pVidDec->pAppData,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandStateSet,
                                                    pVidDec->state, NULL);

                                    // Allocate input buffer
                                    //pInBuf = (OMX_U8*)OMX_OSAL_Malloc(2048);

                                }

                            }
                            break;
                        case OMX_StateExecuting:
                            // Transition can only happen from pause or idle state
                            if(pVidDec->state == OMX_StateIdle
                                            || pVidDec->state == OMX_StatePause)
                            {

                                // Return buffers if currently in pause
                                if(pVidDec->state == OMX_StatePause)
                                {
                                    ListFlushEntries(pVidDec->sInBufList,
                                                     pVidDec);
                                    if(1 == pVidDec->share_disp_buf)
                                        FlushOutputBuffers(pVidDec);
                                    else
                                        ListFlushEntries(pVidDec->sOutBufList,
                                                         pVidDec);

                                }
                                pVidDec->state = OMX_StateExecuting;
                                data_sync_barrier();
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandStateSet,
                                                pVidDec->state, NULL);

                            }
                            else
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            break;
                        case OMX_StatePause:
                            // Transition can only happen from idle or executing state
                            if(pVidDec->state == OMX_StateIdle
                                            || pVidDec->state
                                                            == OMX_StateExecuting)
                            {

                                pVidDec->state = OMX_StatePause;
                                data_sync_barrier();
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandStateSet,
                                                pVidDec->state, NULL);
                            }
                            else
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            break;
                        case OMX_StateWaitForResources:
                            if(pVidDec->state == OMX_StateLoaded)
                            {
                                pVidDec->state = OMX_StateWaitForResources;
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandStateSet,
                                                pVidDec->state, NULL);
                            }
                            else
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            break;
                        case OMX_StateKhronosExtensions:
                        case OMX_StateVendorStartUnused:
                        case OMX_StateMax:
                            /* Not handled */
                            break;
                    }
                }
            }
            else if(cmd == DisablePort)
            {
                ITTIAM_DEBUG("DisablePort Command");
                // Disable Port(s)
                // cmddata contains the port index to be stopped.
                // It is assumed that 0 is input and 1 is output port for this component
                // The cmddata value -1 means that both input and output ports will be stopped.
                if(cmddata == 0x0 || cmddata == -1)
                {
                    // Return all input buffers
                    if(pInBufHdr)
                    {
                        // Return input buffer to client for refill
                        pVidDec->pCallbacks->EmptyBufferDone(pVidDec->hSelf,
                                                             pVidDec->pAppData,
                                                             pInBufHdr);
                        pInBufHdr = NULL;
                    }
                    ListFlushEntries(pVidDec->sInBufList, pVidDec)
                    // Disable port
                    pVidDec->sInPortDef.bEnabled = OMX_FALSE;
                    data_sync_barrier();
                }
                if(cmddata == 0x1 || cmddata == -1)
                {
                    // Return all output buffers
                    if(1 == pVidDec->share_disp_buf)
                        FlushOutputBuffers(pVidDec);
                    else
                        ListFlushEntries(pVidDec->sOutBufList, pVidDec);

                    ittiam_video_decoder_flush(pVidDec);
                    // Disable port
                    pVidDec->sOutPortDef.bEnabled = OMX_FALSE;
                    data_sync_barrier();
                    if(pVidDec->PortReconfiguration)
                    {
                        ittiam_video_decoder_reset(pVidDec);
                    }
                }
                // Wait for all buffers to be freed
                pthread_mutex_lock(&pVidDec->signal_mutex);
                ret_val = 0;
                // Transition happens only when the ports are unpopulated
                if(pVidDec->PortReconfiguration
                                && (pVidDec->sOutBufList.nAllocSize > 0))
                {

                    //gettimeofday(&abstime,NULL);
                    clock_gettime(CLOCK_REALTIME, &abstime);
                    abstime.tv_sec += OMX_TIMEOUT_SEC;
                    ITTIAM_DEBUG("Waiting for buffers to be freed");
                    ret_val = pthread_cond_timedwait(&pVidDec->buffers_signal,
                                                     &pVidDec->signal_mutex,
                                                     &abstime);

                }
                pthread_mutex_unlock(&pVidDec->signal_mutex);

                if(ret_val == 0)
                {
                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventCmdComplete,
                                                      OMX_CommandPortDisable,
                                                      0x1, NULL);
                }
                else
                {
                    ITTIAM_LOG("Something bad happend ");
                }
            }
            else if(cmd == EnablePort)
            {
                ITTIAM_DEBUG("EnablePort Command");
                // Enable Port(s)
                // cmddata contains the port index to be restarted.
                // It is assumed that 0 is input and 1 is output port for this component.
                // The cmddata value -1 means both input and output ports will be restarted.

                if(cmddata == 0x0 || cmddata == -1)
                    pVidDec->sInPortDef.bEnabled = OMX_TRUE;

                if(cmddata == 0x1 || cmddata == -1)
                    pVidDec->sOutPortDef.bEnabled = OMX_TRUE;

                // Wait for port to be populated
                nTimeout = 0;
                while(1)
                {
                    // Return cmdcomplete event if input port populated
                    if(cmddata == 0x0
                                    && (pVidDec->state == OMX_StateLoaded
                                                    || pVidDec->sInPortDef.bPopulated))
                    {
                        pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                          pVidDec->pAppData,
                                                          OMX_EventCmdComplete,
                                                          OMX_CommandPortEnable,
                                                          0x0, NULL);
                        break;
                    }
                    // Return cmdcomplete event if output port populated
                    if(cmddata == 0x1
                                    && (pVidDec->state == OMX_StateLoaded
                                                    || pVidDec->sOutPortDef.bPopulated))
                    {
                        pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                          pVidDec->pAppData,
                                                          OMX_EventCmdComplete,
                                                          OMX_CommandPortEnable,
                                                          0x1, NULL);
                        break;
                    }
                    // Return cmdcomplete event if input and output ports populated
                    if(cmddata == -1
                                    && (pVidDec->state == OMX_StateLoaded
                                                    || (pVidDec->sInPortDef.bPopulated
                                                                    && pVidDec->sOutPortDef.bPopulated)))
                    {
                        pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                          pVidDec->pAppData,
                                                          OMX_EventCmdComplete,
                                                          OMX_CommandPortEnable,
                                                          0x0, NULL);
                        pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                          pVidDec->pAppData,
                                                          OMX_EventCmdComplete,
                                                          OMX_CommandPortEnable,
                                                          0x1, NULL);
                        break;
                    }

                    if(nTimeout++ > OMX_MAX_TIMEOUTS)
                    {
                        pVidDec->pCallbacks->EventHandler(
                                        pVidDec->hSelf,
                                        pVidDec->pAppData,
                                        OMX_EventError,
                                        OMX_ErrorPortUnresponsiveDuringAllocation,
                                        0, NULL);
                        break;
                    }

                    millisleep(OMX_TIMEOUT_MSEC);

                }
            }
            else if(cmd == Flush)
            {
                ITTIAM_DEBUG("Flush Command");
                pthread_mutex_lock(&pVidDec->signal_mutex);
                pVidDec->cmd_pending = 0;
                pthread_mutex_unlock(&pVidDec->signal_mutex);
                // Flush port(s)
                // cmddata contains the port index to be flushed.
                // It is assumed that 0 is input and 1 is output port for this component
                // The cmddata value -1 means that both input and output ports will be flushed.
                if(cmddata == 0x0 || cmddata == -1)
                {
                    // Return all input buffers and send cmdcomplete
                    if(pInBufHdr)
                    {
                        // Return input buffer to client for refill
                        pVidDec->pCallbacks->EmptyBufferDone(pVidDec->hSelf,
                                                             pVidDec->pAppData,
                                                             pInBufHdr);
                        pInBufHdr = NULL;
                    }
                    ListFlushEntries(pVidDec->sInBufList, pVidDec)
                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventCmdComplete,
                                                      OMX_CommandFlush, 0x0,
                                                      NULL);
                }
                if(cmddata == 0x1 || cmddata == -1)
                {
                    // Flush the ittiam decoder
                    ittiam_video_decoder_flush(pVidDec);

                    if(pOutBufHdr)
                    {
                        // Return input buffer to client for refill
                        pVidDec->NumFBD++;
                        //ITTIAM_DEBUG("NumFBD %d LINE %d", (WORD32)pVidDec->NumFBD, __LINE__);
                        pVidDec->pCallbacks->FillBufferDone(pVidDec->hSelf,
                                                            pVidDec->pAppData,
                                                            pOutBufHdr);
                        pOutBufHdr = NULL;
                    }
                    if(1 == pVidDec->share_disp_buf)
                    {
                        pVidDec->wait_for_op_buffers = 1;
                        // Return all output buffers and send cmdcomplete
                        ITTIAM_DEBUG("wait_for_op_buffers %d ", __LINE__);
                        FlushOutputBuffers(pVidDec);
                    }
                    else
                    {
                        ITTIAM_DEBUG("Calling list flush entries %d ",
                                     __LINE__);
                        ListFlushEntries(pVidDec->sOutBufList, pVidDec);
                    }

                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventCmdComplete,
                                                      OMX_CommandFlush, 0x1,
                                                      NULL);
                }
            }
            else if(cmd == StopThread)
            {
                ITTIAM_LOG("StopThread Command");
                // Kill thread
                goto EXIT;
            }
            else if(cmd == FillBuf)
            {
                ITTIAM_DEBUG("FillBuf  = %p ",
                             ((OMX_BUFFERHEADERTYPE*)cmddata));
                // Fill buffer
                ListSetEntry(pVidDec->sOutBufList,
                             ((OMX_BUFFERHEADERTYPE*)cmddata))
            }
            else if(cmd == EmptyBuf)
            {

                // Empty buffer
                ListSetEntry(pVidDec->sInBufList,
                             ((OMX_BUFFERHEADERTYPE *)cmddata))
                // Mark current buffer if there is outstanding command
                if(pMarkBuf)
                {
                    ((OMX_BUFFERHEADERTYPE *)(cmddata))->hMarkTargetComponent =
                                    pMarkBuf->hMarkTargetComponent;
                    ((OMX_BUFFERHEADERTYPE *)(cmddata))->pMarkData =
                                    pMarkBuf->pMarkData;
                    pMarkBuf = NULL;
                }
            }
            else if(cmd == MarkBuf)
            {
                ITTIAM_DEBUG("MarkBuf Command");

                if(!pMarkBuf)
                    pMarkBuf = (OMX_MARKTYPE *)(cmddata);
            }
        }
        // Buffer processing
        // Only happens when the component is in executing state.
        while(pVidDec->state == OMX_StateExecuting
                        && pVidDec->sInPortDef.bEnabled
                        && pVidDec->sOutPortDef.bEnabled
                        && ((pVidDec->sInBufList.nSizeOfList > 0) || pInBufHdr)
                        && ((pVidDec->sOutBufList.nSizeOfList > 0) || pOutBufHdr)
                        && (pVidDec->cmd_pending == 0)
                        && (pVidDec->PortReconfiguration == 0))
        {

            if(pInBufHdr == NULL)
            {
                ListGetEntry(pVidDec->sInBufList, pInBufHdr)
                ITTIAM_DEBUG("Get input buffer Filled length = %d nOffset = %d",
                             (WORD32)pInBufHdr->nFilledLen,
                             (WORD32)pInBufHdr->nOffset);
                pInBufHdr->nOffset = 0;
            }
            // If there is no output buffer, get one from list
            if (!pOutBufHdr)
            ListGetEntry(pVidDec->sOutBufList, pOutBufHdr)

            // Check for EOS flag
            if(pInBufHdr)
            {

                // Check for mark buffers
                if(pInBufHdr->pMarkData)
                {
                    // Copy mark to output buffer header
                    if(pOutBufHdr)
                    {
                        pOutBufHdr->pMarkData = pInBufHdr->pMarkData;
                        // Copy handle to output buffer header
                        pOutBufHdr->hMarkTargetComponent =
                                        pInBufHdr->hMarkTargetComponent;
                    }
                }
                // Trigger event handler
                if(pInBufHdr->hMarkTargetComponent == pVidDec->hSelf
                                && pInBufHdr->pMarkData)
                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventMark, 0, 0,
                                                      pInBufHdr->pMarkData);

            }

            // NULL check for I/O buffers
            if((pInBufHdr==NULL) || (pOutBufHdr==NULL))
            {
                ITTIAM_DEBUG("Decoder I/O buffers are NULL");
                break;
            }

            ITTIAM_DEBUG("ittiam_video_decoder_process");
             // Decode frame
             start_ts = itGetMs();
             pthread_mutex_lock(&pVidDec->codec_mutex);
             ittiam_video_decoder_process(pVidDec, pInBufHdr, pOutBufHdr);
             pthread_mutex_unlock(&pVidDec->codec_mutex);
             stop_ts = itGetMs();
            ITTIAM_DEBUG("ittiam_video_decoder_process done");

             ITTIAM_DEBUG("Start time  = %lld Stop time = %lld time comsumed = %lld delay = %lld",
                                    start_ts, stop_ts, stop_ts - start_ts, start_ts - prev_stop_ts);
             prev_stop_ts = stop_ts;


// Average Dec time changes
    total_delay_time +=
            ( (((start_ts - prev_stop_ts) > 5000) || ((start_ts - prev_stop_ts) < 0)) ? 0 : (start_ts - prev_stop_ts));
        total_decode_time += stop_ts - start_ts;
        num_decoded_buffers++;
        prev_stop_ts = stop_ts;


            if(pInBufHdr->nOffset >= pInBufHdr->nFilledLen)
            {
                // Return input buffer to client for refill
                pVidDec->NumEBD++;
                ITTIAM_DEBUG("NumEBD %d LINE %d", (WORD32)pVidDec->NumEBD,
                             __LINE__);
                pVidDec->pCallbacks->EmptyBufferDone(pVidDec->hSelf,
                                                     pVidDec->pAppData,
                                                     pInBufHdr);
                pInBufHdr = NULL;
            }
            if((0 == pVidDec->share_disp_buf) || (pOutBufHdr->nFilledLen != 0))
            {

                pOutBufHdr = NULL;
            }
        }

    }
    if(0 && (0 == pVidDec->share_disp_buf) && (pOutBufHdr)
                    && (0 == (pOutBufHdr->nFlags & OMX_BUFFERFLAG_EOS)))
    {
        ITTIAM_DEBUG("Calling last FillBufferDone\n");
        pVidDec->pCallbacks->FillBufferDone(pVidDec->hSelf, pVidDec->pAppData,
                                            pOutBufHdr);
        pOutBufHdr = NULL;
    }

    EXIT: return (void*)OMX_ErrorNone;
}
void FlushOutputBuffers(VIDDECTYPE* pVidDec)
{
    UWORD32 i;
    for(i = 0; i < pVidDec->sOutPortDef.nBufferCountActual; i++)
    {
        if(pVidDec->sOutBufList.pBufHdr[i]->nFlags & CUSTOM_BUFFERFLAG_OWNED)
        {
            pVidDec->sOutBufList.pBufHdr[i]->nFlags =
                            pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                            & (~CUSTOM_BUFFERFLAG_OWNED);
            pVidDec->pCallbacks->FillBufferDone(
                            pVidDec->hSelf, pVidDec->pAppData,
                            pVidDec->sOutBufList.pBufHdr[i]);
        }
    }
}

IV_API_CALL_STATUS_T ittiam_video_decoder_init(VIDDECTYPE* pVidDec)
{

    IV_API_CALL_STATUS_T e_dec_status;
    UWORD32 u4_num_mem_recs;
    UWORD32 u4_num_reorder_frames;
    UWORD32 u4_num_ref_frames;

    //Average Dec time changes
    total_decode_time = 0;
    num_decoded_buffers = 0;
    total_delay_time = 0;
    {
        iv_num_mem_rec_ip_t s_no_of_mem_rec_query_ip;
        iv_num_mem_rec_op_t s_no_of_mem_rec_query_op;

        //s_no_of_mem_rec_query_ip.u4_size = sizeof(iv_num_mem_rec_ip_t);
        s_no_of_mem_rec_query_ip.u4_size = sizeof(s_no_of_mem_rec_query_ip);
        s_no_of_mem_rec_query_op.u4_size = sizeof(s_no_of_mem_rec_query_op);
        s_no_of_mem_rec_query_ip.e_cmd = IV_CMD_GET_NUM_MEM_REC;

        /*****************************************************************************/
        /*   API Call: Get Number of Mem Records                                     */
        /*****************************************************************************/
        ITTIAM_DEBUG("Get Number of Mem Records");
        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void*)&s_no_of_mem_rec_query_ip,
                        (void*)&s_no_of_mem_rec_query_op);
        if(IV_SUCCESS != e_dec_status)
        {
            ITTIAM_ERROR("Error in get mem records");
            return (e_dec_status);

            /*sprintf(i1_error_string, "Error in get mem records");
             appl_exit_with_error(&i1_error_string[0]);*/
        }

        u4_num_mem_recs = s_no_of_mem_rec_query_op.u4_num_mem_rec;
    }

    pVidDec->pv_mem_rec_location = (iv_mem_rec_t*)malloc(u4_num_mem_recs * sizeof(iv_mem_rec_t));
    if(pVidDec->pv_mem_rec_location == NULL)
    {
        ITTIAM_LOG("\nAllocation failure\n");
        e_dec_status = IV_FAIL;
        return (e_dec_status);
    }


    {

            /* Map OMX eColorFormat to Ittiam Color Format enum */

            if(pVidDec->sOutPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar)
                pVidDec->e_output_format = IV_YUV_420P;
            else if(pVidDec->sOutPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar)
                pVidDec->e_output_format = IV_YUV_420SP_UV;
//            else if(pVidDec->sOutPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar)
//                pVidDec->e_output_format = IV_YUV_420SP_VU;
//            else if(pVidDec->sOutPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FORMATYUV420PackedSemiPlanar32m)
//                pVidDec->e_output_format = IV_YUV_420SP_UV;
		else if (pVidDec->sOutPortDef.format.video.eColorFormat == CUSTOM_COLOR_FormatYUV420SemiPlanar)
			pVidDec->e_output_format  = IV_YUV_420SP_UV;
		else if (pVidDec->sOutPortDef.format.video.eColorFormat == CUSTOM_COLOR_FormatYVU420SemiPlanar)
                  pVidDec->e_output_format = IV_YUV_420SP_VU;

            else
            {
                ITTIAM_LOG("\nInvalid Color format \n");
                e_dec_status = IV_FAIL;
                return (e_dec_status);

            }

            /* Initialize number of ref and reorder modes (for H264 and HEVC). In case of thumb set it to 1 ref and 0 reorder */
            u4_num_reorder_frames = 16;
            u4_num_ref_frames = 16;

            if(pVidDec->ThumbnailMode)
            {
                u4_num_reorder_frames = 0;
                u4_num_ref_frames = 1;
            }

    }

    {
        UWORD32 i;
        iv_fill_mem_rec_ip_t s_fill_mem_rec_ip;
        iv_fill_mem_rec_op_t s_fill_mem_rec_op;
#ifdef ENABLE_H264
        ih264d_cxa8_fill_mem_rec_ip_t s_h264d_fill_mem_rec_ip_t;
#endif
#ifdef ENABLE_HEVC
        ihevcd_cxa_fill_mem_rec_ip_t s_hevcd_fill_mem_rec_ip_t;
#endif
#ifdef ENABLE_MPEG4
        imp4d_cxa8_fill_mem_rec_ip_t s_mp4d_fill_mem_rec_ip_t;
#endif
#ifdef ENABLE_MPEG2
        imp2d_cxa8_fill_mem_rec_ip_t s_mp2d_fill_mem_rec_ip_t;
#endif
        iv_mem_rec_t *ps_mem_rec;
        iv_fill_mem_rec_ip_t *s_fill_mem_rec_ip_ptr;

        switch((int)pVidDec->sInPortDef.format.video.eCompressionFormat)
        {
#ifdef ENABLE_H264
        case OMX_VIDEO_CodingAVC:
            {
                UWORD32 i4_level = 31;
                if((pVidDec->sOutPortDef.format.video.nFrameWidth * pVidDec->sOutPortDef.format.video.nFrameHeight) > (1280 * 720))
                    i4_level = 41;

                s_fill_mem_rec_ip_ptr = (iv_fill_mem_rec_ip_t *)&s_h264d_fill_mem_rec_ip_t;
                s_h264d_fill_mem_rec_ip_t.s_ivd_fill_mem_rec_ip_t.u4_size = sizeof(ih264d_cxa8_fill_mem_rec_ip_t);
                s_h264d_fill_mem_rec_ip_t.i4_level = i4_level;
                s_h264d_fill_mem_rec_ip_t.u4_num_reorder_frames = u4_num_reorder_frames;
                s_h264d_fill_mem_rec_ip_t.u4_num_ref_frames = u4_num_ref_frames;
                s_h264d_fill_mem_rec_ip_t.u4_share_disp_buf = pVidDec->share_disp_buf;
                s_h264d_fill_mem_rec_ip_t.u4_num_extra_disp_buf = 0;

                s_h264d_fill_mem_rec_ip_t.e_output_format = pVidDec->e_output_format;

            }
            break;
#endif
#ifdef ENABLE_HEVC
        case OMX_VIDEO_CodingHEVC:
            {
                s_fill_mem_rec_ip_ptr = (iv_fill_mem_rec_ip_t *)&s_hevcd_fill_mem_rec_ip_t;
                s_hevcd_fill_mem_rec_ip_t.s_ivd_fill_mem_rec_ip_t.u4_size = sizeof(ihevcd_cxa_fill_mem_rec_ip_t);
                s_hevcd_fill_mem_rec_ip_t.i4_level = 41;
                s_hevcd_fill_mem_rec_ip_t.u4_num_reorder_frames = u4_num_reorder_frames;
                s_hevcd_fill_mem_rec_ip_t.u4_num_ref_frames = u4_num_ref_frames;

                s_hevcd_fill_mem_rec_ip_t.u4_share_disp_buf = pVidDec->share_disp_buf;
                s_hevcd_fill_mem_rec_ip_t.u4_num_extra_disp_buf = 0;
                s_hevcd_fill_mem_rec_ip_t.e_output_format = pVidDec->e_output_format;


            }
            break;
#endif
#ifdef ENABLE_MPEG4
        case OMX_VIDEO_CodingMPEG4:
        case OMX_VIDEO_CodingH263:
        case QOMX_VIDEO_CodingDivx:
            {
                s_fill_mem_rec_ip_ptr = (iv_fill_mem_rec_ip_t *)&s_mp4d_fill_mem_rec_ip_t;
                s_mp4d_fill_mem_rec_ip_t.s_ivd_fill_mem_rec_ip_t.u4_size = sizeof(imp4d_cxa8_fill_mem_rec_ip_t);
                s_mp4d_fill_mem_rec_ip_t.u4_share_disp_buf = pVidDec->share_disp_buf;
                s_mp4d_fill_mem_rec_ip_t.e_output_format = pVidDec->e_output_format;


            }
            break;
#endif
#ifdef ENABLE_MPEG2
        case OMX_VIDEO_CodingMPEG2:
            {
                s_fill_mem_rec_ip_ptr = (iv_fill_mem_rec_ip_t *)&s_mp2d_fill_mem_rec_ip_t;
                s_mp2d_fill_mem_rec_ip_t.s_ivd_fill_mem_rec_ip_t.u4_size = sizeof(imp2d_cxa8_fill_mem_rec_ip_t);
                s_mp2d_fill_mem_rec_ip_t.u4_share_disp_buf = pVidDec->share_disp_buf;
                s_mp2d_fill_mem_rec_ip_t.e_output_format = pVidDec->e_output_format;

            }
            break;
#endif
        default:
            {
                s_fill_mem_rec_ip_ptr = (iv_fill_mem_rec_ip_t *)&s_fill_mem_rec_ip;
                s_fill_mem_rec_ip.u4_size = sizeof(iv_fill_mem_rec_ip_t);
            }
            break;
        }

        s_fill_mem_rec_ip_ptr->e_cmd = IV_CMD_FILL_NUM_MEM_REC;
        s_fill_mem_rec_ip_ptr->pv_mem_rec_location = pVidDec->pv_mem_rec_location;
        s_fill_mem_rec_ip_ptr->u4_max_frm_wd = pVidDec->sOutPortDef.format.video.nFrameWidth;
        s_fill_mem_rec_ip_ptr->u4_max_frm_ht = pVidDec->sOutPortDef.format.video.nFrameHeight;
        s_fill_mem_rec_op.u4_size = sizeof(iv_fill_mem_rec_op_t);

        ps_mem_rec = pVidDec->pv_mem_rec_location;
        for(i = 0; i < u4_num_mem_recs; i++)
            ps_mem_rec[i].u4_size = sizeof(iv_mem_rec_t);

        /*****************************************************************************/
        /*   API Call: Fill Mem Records                                              */
        /*****************************************************************************/

        ITTIAM_DEBUG("Fill Mem Records");
        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void *)s_fill_mem_rec_ip_ptr,
                        (void *)&s_fill_mem_rec_op);

        u4_num_mem_recs = s_fill_mem_rec_op.u4_num_mem_rec_filled;

        if(IV_SUCCESS != e_dec_status)
        {
            ITTIAM_ERROR("Error in fill mem records");
            return (e_dec_status);
        }

        ps_mem_rec = pVidDec->pv_mem_rec_location;

        for(i = 0; i < u4_num_mem_recs; i++)
        {

            ps_mem_rec->pv_base = memalign(ps_mem_rec->u4_mem_alignment,
                                           ps_mem_rec->u4_mem_size);
            if(ps_mem_rec->pv_base == NULL)
            {
                ITTIAM_LOG("\nAllocation failure\n");
                e_dec_status = IV_FAIL;
                return (e_dec_status);

            }

            ps_mem_rec++;
        }
    }
    {
        ivd_init_ip_t s_init_ip;
        ivd_init_op_t s_init_op;
#ifdef ENABLE_H264
        ih264d_cxa8_init_ip_t s_h264d_init_ip_t;
#endif
#ifdef ENABLE_HEVC
        ihevcd_cxa_init_ip_t s_hevcd_init_ip_t;
#endif
#ifdef ENABLE_MPEG4
        imp4d_cxa8_init_ip_t s_mp4d_init_ip_t;
#endif

#ifdef ENABLE_MPEG2
        imp2d_cxa8_init_ip_t s_mp2d_init_ip_t;

#endif
        ivd_init_ip_t *s_init_ip_ptr;

        void *dec_fxns = (void *)pVidDec->iVdec_cxa8_api_function;
        iv_mem_rec_t *mem_tab;

        mem_tab = (iv_mem_rec_t*)pVidDec->pv_mem_rec_location;

        switch((int)pVidDec->sInPortDef.format.video.eCompressionFormat)
        {
#ifdef ENABLE_H264
        case OMX_VIDEO_CodingAVC:
            {
                s_init_ip_ptr = (ivd_init_ip_t *)&s_h264d_init_ip_t;
                s_h264d_init_ip_t.s_ivd_init_ip_t.u4_size =
                                sizeof(ih264d_cxa8_init_ip_t);
                s_h264d_init_ip_t.i4_level = 31;

                if((pVidDec->sOutPortDef.format.video.nFrameWidth * pVidDec->sOutPortDef.format.video.nFrameHeight) > (1280 * 720))
                    s_h264d_init_ip_t.i4_level = 41;

                s_h264d_init_ip_t.u4_num_reorder_frames = u4_num_reorder_frames;
                s_h264d_init_ip_t.u4_num_ref_frames = u4_num_ref_frames;
                s_h264d_init_ip_t.u4_share_disp_buf = pVidDec->share_disp_buf;
                s_h264d_init_ip_t.u4_num_extra_disp_buf = 0;

            }
            break;
#endif
#ifdef ENABLE_HEVC
        case OMX_VIDEO_CodingHEVC:
            {
                s_init_ip_ptr = (ivd_init_ip_t *)&s_hevcd_init_ip_t;
                s_hevcd_init_ip_t.s_ivd_init_ip_t.u4_size = sizeof(ihevcd_cxa_init_ip_t);
                s_hevcd_init_ip_t.i4_level = 41;
                s_hevcd_init_ip_t.u4_num_reorder_frames = u4_num_reorder_frames;
                s_hevcd_init_ip_t.u4_num_ref_frames = u4_num_ref_frames;

                s_hevcd_init_ip_t.u4_share_disp_buf = pVidDec->share_disp_buf;
                s_hevcd_init_ip_t.u4_num_extra_disp_buf = 0;

            }
            break;
#endif
#ifdef ENABLE_MPEG4
        case OMX_VIDEO_CodingMPEG4:
        case OMX_VIDEO_CodingH263:
        case QOMX_VIDEO_CodingDivx:
            {
                s_init_ip_ptr = (ivd_init_ip_t *)&s_mp4d_init_ip_t;
                s_mp4d_init_ip_t.s_ivd_init_ip_t.u4_size =
                                sizeof(imp4d_cxa8_init_ip_t);
                s_mp4d_init_ip_t.u4_share_disp_buf = pVidDec->share_disp_buf;
            }
            break;
#endif
#ifdef ENABLE_MPEG2
        case OMX_VIDEO_CodingMPEG2:
            {
                s_init_ip_ptr = (ivd_init_ip_t *)&s_mp2d_init_ip_t;
                s_mp2d_init_ip_t.s_ivd_init_ip_t.u4_size =
                                sizeof(imp2d_cxa8_init_ip_t);
                s_mp2d_init_ip_t.u4_share_disp_buf = pVidDec->share_disp_buf;
            }
            break;
#endif
        default:
            {
                s_init_ip_ptr = (ivd_init_ip_t *)&s_init_ip;
                s_init_ip.u4_size = sizeof(ivd_init_ip_t);
            }
            break;
        }

        s_init_ip_ptr->e_cmd = (IVD_API_COMMAND_TYPE_T)IV_CMD_INIT;
        s_init_ip_ptr->pv_mem_rec_location = mem_tab;
        s_init_ip_ptr->u4_frm_max_wd =
                        pVidDec->sOutPortDef.format.video.nFrameWidth;
        s_init_ip_ptr->u4_frm_max_ht =
                        pVidDec->sOutPortDef.format.video.nFrameHeight;

        s_init_op.u4_size = sizeof(ivd_init_op_t);

        s_init_ip_ptr->u4_num_mem_rec = u4_num_mem_recs;
        s_init_ip_ptr->e_output_format = pVidDec->e_output_format;

        pVidDec->g_DECHDL = (iv_obj_t*)mem_tab[0].pv_base;
        pVidDec->g_DECHDL->pv_fxns = dec_fxns;
        pVidDec->g_DECHDL->u4_size = sizeof(iv_obj_t);
        /*****************************************************************************/
        /*   API Call: Initialize the Decoder                                        */
        /*****************************************************************************/

        ITTIAM_DEBUG("Initialize the Decoder");
        e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                        (void *)s_init_ip_ptr,
                                                        (void *)&s_init_op);
        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in Init %x", s_init_op.u4_error_code);
            /*sprintf(i1_error_string, "Error in Init");
             appl_exit_with_error(&i1_error_string[0]);*/
            return (e_dec_status);
        }
        pVidDec->initdone = 1;


    }
    switch((int)pVidDec->sInPortDef.format.video.eCompressionFormat)
    {
#ifdef ENABLE_H264
    case OMX_VIDEO_CodingAVC:
    {

        ih264d_cxa8_ctl_set_num_cores_ip_t s_ctl_set_cores_ip;
        ih264d_cxa8_ctl_set_num_cores_op_t s_ctl_set_cores_op;

        s_ctl_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_set_cores_ip.e_sub_cmd = IH264D_CXA8_CMD_CTL_SET_NUM_CORES;
        s_ctl_set_cores_ip.u4_num_cores = MIN(pVidDec->num_cores,
                                              H264_MAX_NUM_CORES);
        s_ctl_set_cores_ip.u4_size = sizeof(ih264d_cxa8_ctl_set_num_cores_ip_t);
        s_ctl_set_cores_op.u4_size = sizeof(ih264d_cxa8_ctl_set_num_cores_op_t);
    ALOGE("Ittiam: number of cores for AVC = %d", s_ctl_set_cores_ip.u4_num_cores);
        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void *)&s_ctl_set_cores_ip,
                        (void *)&s_ctl_set_cores_op);
        if(IV_SUCCESS != e_dec_status)
        {
            ITTIAM_ERROR("Error in Setting number of cores %x\n",s_ctl_set_cores_op.u4_error_code);
            return (e_dec_status);
        }

    }
    break;
#endif
#ifdef ENABLE_HEVC
    case OMX_VIDEO_CodingHEVC:
    {

        ihevcd_cxa_ctl_set_num_cores_ip_t s_ctl_set_cores_ip;
        ihevcd_cxa_ctl_set_num_cores_op_t s_ctl_set_cores_op;

        s_ctl_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_set_cores_ip.e_sub_cmd = IHEVCD_CXA_CMD_CTL_SET_NUM_CORES;
        s_ctl_set_cores_ip.u4_num_cores = MIN(pVidDec->num_cores,
                                              HEVC_MAX_NUM_CORES);
        s_ctl_set_cores_ip.u4_size = sizeof(ihevcd_cxa_ctl_set_num_cores_ip_t);
        s_ctl_set_cores_op.u4_size = sizeof(ihevcd_cxa_ctl_set_num_cores_op_t);

        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void *)&s_ctl_set_cores_ip,
                        (void *)&s_ctl_set_cores_op);
        if(IV_SUCCESS != e_dec_status)
        {
            ITTIAM_ERROR("Error in Setting number of cores\n");
            return (e_dec_status);
        }

    }
    break;
#endif
#ifdef ENABLE_MPEG4
    case OMX_VIDEO_CodingMPEG4:
    case OMX_VIDEO_CodingH263:
    case QOMX_VIDEO_CodingDivx:
    {

        imp4d_cxa8_ctl_set_num_cores_ip_t s_ctl_set_cores_ip;
        imp4d_cxa8_ctl_set_num_cores_op_t s_ctl_set_cores_op;

        s_ctl_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_set_cores_ip.e_sub_cmd = IMP4D_CXA8_CMD_CTL_SET_NUM_CORES;
        s_ctl_set_cores_ip.u4_num_cores = MIN(pVidDec->num_cores,
                                              MPEG4_MAX_NUM_CORES);
        s_ctl_set_cores_ip.u4_size = sizeof(imp4d_cxa8_ctl_set_num_cores_ip_t);
        s_ctl_set_cores_op.u4_size = sizeof(imp4d_cxa8_ctl_set_num_cores_op_t);
        ALOGE("Ittiam: number of cores for MPEG4 = %d", s_ctl_set_cores_ip.u4_num_cores);
        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void *)&s_ctl_set_cores_ip,
                        (void *)&s_ctl_set_cores_op);
        if(IV_SUCCESS != e_dec_status)
            if(IV_SUCCESS != e_dec_status)
            {
                ITTIAM_ERROR("Error in Setting number of cores\n");
                return (e_dec_status);
            }

    }
    break;
#endif
#ifdef ENABLE_MPEG2
    case OMX_VIDEO_CodingMPEG2:
    {
        imp2d_cxa8_ctl_set_num_cores_ip_t s_ctl_set_cores_ip;
        imp2d_cxa8_ctl_set_num_cores_op_t s_ctl_set_cores_op;

        s_ctl_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_set_cores_ip.e_sub_cmd = IMP2D_CXA8_CMD_CTL_SET_NUM_CORES;
        s_ctl_set_cores_ip.u4_num_cores = MIN(pVidDec->num_cores,
                                              MPEG2_MAX_NUM_CORES);
        s_ctl_set_cores_ip.u4_size = sizeof(imp2d_cxa8_ctl_set_num_cores_ip_t);
        s_ctl_set_cores_op.u4_size = sizeof(imp2d_cxa8_ctl_set_num_cores_op_t);

        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void *)&s_ctl_set_cores_ip,
                        (void *)&s_ctl_set_cores_op);
        if(IV_SUCCESS != e_dec_status)
        {
            ITTIAM_ERROR("Error in Setting number of cores\n");
            return (e_dec_status);
        }

    }
    break;
#endif
    default:
        break;
}
    /*****************************************************************************/
    /*   API Call: Set the run time (dynamics) parameters before the Process call*/
    /*****************************************************************************/
    {

        ivd_ctl_set_config_ip_t s_ctl_dec_ip;
        ivd_ctl_set_config_op_t s_ctl_dec_op;
        UWORD32 count;
        s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
        s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_NONE;

        s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
        s_ctl_dec_ip.e_vid_dec_mode = IVD_DECODE_HEADER;
        s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
        s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
        s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

        ITTIAM_DEBUG("Set the run time (dynamics) parameters ");
        pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                         (void *)&s_ctl_dec_ip,
                                         (void *)&s_ctl_dec_op);

        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in Setting the run time (dynamics) parameters e_dec_status = %d u4_error_code = %x\n",
                  e_dec_status, s_ctl_dec_op.u4_error_code);
            pVidDec->initdone = 0;
            return (e_dec_status);
        }
        else
        {
            pVidDec->initdone = 1;
        }

        for(count = 0; count < STAMP_IDX;count++)
        {
            pVidDec->nTimeStamp[count] = 0;
            pVidDec->CheckForValidity[count] = 0;

        }

    }

    /* Get the codec version */
    {
        ivd_ctl_getversioninfo_ip_t s_ctl_dec_ip;
        ivd_ctl_getversioninfo_op_t s_ctl_dec_op;
        UWORD8 au1_buf[512];

        s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_GETVERSION;
        s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_getversioninfo_ip_t);
        s_ctl_dec_op.u4_size = sizeof(ivd_ctl_getversioninfo_op_t);
        s_ctl_dec_ip.pv_version_buffer = au1_buf;
        s_ctl_dec_ip.u4_version_buffer_size = sizeof(au1_buf);

        e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                         (void *)&(s_ctl_dec_ip),
                                         (void *)&(s_ctl_dec_op));

        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in Getting Version number e_dec_status = %d u4_error_code = %x\n",
                  e_dec_status, s_ctl_dec_op.u4_error_code);
        }
        else
        {
            ITTIAM_LOG("Ittiam Decoder Version number: %s\n",
                  (char *)s_ctl_dec_ip.pv_version_buffer);
        }
    }
    return (e_dec_status);

}

void pad_yuv(char *dst_ptr,
             char *src_ptr,
             WORD32 display_width,
             WORD32 display_height,
             WORD32 decoded_width,
             WORD32 decoded_height)
{
    WORD32 i, j;
    ITTIAM_LOG("Padding display_width = %d, display_height = %d", display_width,
          display_height);
    ITTIAM_LOG("Padding decoded_width = %d, decoded_height = %d", decoded_width,
          decoded_height);

    // luma copy
    for(i = 0; i < display_height; i++)
    {
        for(j = 0; j < display_width; j++)
        {
            *dst_ptr++ = *src_ptr++;
        }
        dst_ptr += decoded_width - display_width;
    }
    dst_ptr += (decoded_width * (decoded_height - display_height));

    // chroma copy
    for(i = 0; i < (display_height / 2); i++)
    {
        for(j = 0; j < display_width; j++)
        {
            *dst_ptr++ = *src_ptr++;
        }
        dst_ptr += decoded_width - display_width;
    }

}
//DRC Changes
/*****************************************************************************/
/*  Function Name : ittiam_video_decoder_reset                             */
/*  Description   : Called from the component thread,                                                           */
/*                           If Dynamic resolution changes occur                                                  */
/*                        while the component is in the executing state. */
/*  Inputs        :                                                          */
/*  Globals       :                                                          */
/*  Processing    :                                                          */
/*  Outputs       :                                                          */
/*  Returns       :                                                          */
/*  Issues        :                                                          */
/*  Revision History:                                                        */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         09 03 2010   Ittiam          Draft                                */
/*****************************************************************************/
void ittiam_video_decoder_reset(VIDDECTYPE* pVidDec)
{
    IV_API_CALL_STATUS_T e_dec_status;

    ivd_ctl_reset_ip_t s_ctl_reset_ip_t;
    ivd_ctl_reset_op_t s_ctl_reset_op_t;
    s_ctl_reset_ip_t.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_reset_ip_t.e_sub_cmd = IVD_CMD_CTL_RESET;
    s_ctl_reset_ip_t.u4_size = sizeof(ivd_ctl_reset_ip_t);
    s_ctl_reset_op_t.u4_size = sizeof(ivd_ctl_reset_op_t);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    (void *)&s_ctl_reset_ip_t,
                                                    (void *)&s_ctl_reset_op_t);
    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for codec reset failed\n");
    }
}
//DRC Changes
void ittiam_video_decoder_process(VIDDECTYPE* pVidDec,
                                  OMX_BUFFERHEADERTYPE *pInBufHdr,
                                  OMX_BUFFERHEADERTYPE *pOutBufHdr)
{
    IV_API_CALL_STATUS_T e_dec_status;

    ivd_video_decode_ip_t s_video_decode_ip;
    ivd_video_decode_op_t s_video_decode_op;
//    UWORD32 u4_op_len_produced;
//    WORD32 width, height, stride;
//    WORD32 header_decode_error = 0;
    UWORD32 i;
    UWORD32 count;
    UWORD32 hdr_decode_done;


    for(count = 0; count < STAMP_IDX; count++)
    {
        if(pVidDec->CheckForValidity[count] == 0)
        {
            break;
        }
    }
    if(count == STAMP_IDX)
    {
        ITTIAM_ERROR("Unable to find free slot to hold timestamp overwriting last entry");
        count = STAMP_IDX - 1;
    }
    hdr_decode_done = 0;
    pVidDec->CheckForValidity[count] = 1;
    pVidDec->nTimeStamp[count] = pInBufHdr->nTimeStamp;

    UWORD32 u4_op_len_produced;
    WORD32 width, height, stride;
    WORD32 header_decode_error = 0;

    width = pVidDec->sOutPortDef.format.video.nFrameWidth;
    if(pVidDec->mProcessorType == SAMSUNG_GENERIC)
    {
        height = pVidDec->sOutPortDef.format.video.nFrameHeight;
    }
    else
    {
        height = pVidDec->sOutPortDef.format.video.nSliceHeight;
    }
    stride = pVidDec->sOutPortDef.format.video.nStride;
    if(pVidDec->wait_for_op_buffers == 1)
    {
        ITTIAM_DEBUG("nBufferCountMin = %d",
                     (WORD32)pVidDec->sOutPortDef.nBufferCountMin);
        pthread_mutex_unlock(&pVidDec->codec_mutex);
        while(1)
        {
            UWORD32 num_buf_owned = 0;
            for(i = 0; i < pVidDec->sOutPortDef.nBufferCountActual; i++)
            {

                if(pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                & CUSTOM_BUFFERFLAG_OWNED)
                {
                    num_buf_owned++;
                }
            }
            if(num_buf_owned < pVidDec->sOutPortDef.nBufferCountMin)
            {
                ITTIAM_DEBUG("num_buf_owned =%d pVidDec->sOutPortDef.nBufferCountMin =%d",
                             (WORD32)num_buf_owned,
                             (WORD32)pVidDec->sOutPortDef.nBufferCountMin);
                ITTIAM_DEBUG("Buffer NOT with the component. Waiting ... ");
                usleep(5000);
            }
            else
            {
                break;
            }
        };
        pVidDec->wait_for_op_buffers = 0;
        pthread_mutex_lock(&pVidDec->codec_mutex);
    }

    if(pVidDec->hdr_decode_done == 0)
    {
        ivd_ctl_set_config_ip_t s_ctl_dec_ip;
        ivd_ctl_set_config_op_t s_ctl_dec_op;

        ivd_video_decode_ip_t s_video_decode_ip;
        ivd_video_decode_op_t s_video_decode_op;

        s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
        s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_NONE;

        s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
        s_ctl_dec_ip.e_vid_dec_mode = IVD_DECODE_HEADER;
        s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
        s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
        s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);
//        pInBufHdr->nOffset += 4;
        ITTIAM_DEBUG("Header Decode called");

        e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                        (void *)&s_ctl_dec_ip,
                                                        (void *)&s_ctl_dec_op);

        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in setting decoder in DECODE_HEADER mode decode e_dec_status = %d u4_error_code = %x\n",
                  (WORD32)e_dec_status, (WORD32)s_ctl_dec_op.u4_error_code);
        }

    }

    s_video_decode_ip.e_cmd = IVD_CMD_VIDEO_DECODE;
    s_video_decode_ip.u4_ts = count;
    s_video_decode_ip.pv_stream_buffer = pInBufHdr->pBuffer
                    + pInBufHdr->nOffset;
    s_video_decode_ip.u4_num_Bytes = pInBufHdr->nFilledLen - pInBufHdr->nOffset;
    s_video_decode_ip.u4_size = sizeof(ivd_video_decode_ip_t);
    s_video_decode_op.u4_size = sizeof(ivd_video_decode_op_t);

    if(pOutBufHdr==NULL)
    {
        ITTIAM_DEBUG("Decoder o/p buffer is NULL");
        return;
    }
    if(0 == pVidDec->share_disp_buf)
    {
        if((pVidDec->sOutPortDef.format.video.eColorFormat == OMX_COLOR_FormatYUV420SemiPlanar) &&
           (pVidDec->mProcessorType == SAMSUNG_GENERIC))
        {
            UWORD32 i;
            for(i = 0; i < NUM_OUT_BUFFERS; i++)
            {
                if((pOutBufHdr->pBuffer) == pVidDec->inp_y_id_mapping[i])
                {
                    s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0] =
                                    stride * height;
                    s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1] =
                                    (stride * height) / 4;
                    s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[2] =
                                    (stride * height) / 4;

                    s_video_decode_ip.s_out_buffer.pu1_bufs[0] =
                                    pOutBufHdr->pBuffer;
                    s_video_decode_ip.s_out_buffer.pu1_bufs[1] =
                                    (OMX_U8*)pVidDec->inp_uv_id_mapping[i];
                    s_video_decode_ip.s_out_buffer.pu1_bufs[2] =
                                    (OMX_U8*)pVidDec->inp_uv_id_mapping[i]
                                                    + (stride * height) / 4;
                    s_video_decode_ip.s_out_buffer.u4_num_bufs = 3;
                    ITTIAM_DEBUG("Found buffer pointer for Y and UV");
                    break;
                }
            }

        }
        else
        {

            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0] = stride * height;
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1] = (stride * height) / 4;
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[2] = (stride * height) / 4;

            s_video_decode_ip.s_out_buffer.pu1_bufs[0] = pOutBufHdr->pBuffer;
            s_video_decode_ip.s_out_buffer.pu1_bufs[1] = s_video_decode_ip.s_out_buffer.pu1_bufs[0] + (stride * height);
            s_video_decode_ip.s_out_buffer.pu1_bufs[2] = s_video_decode_ip.s_out_buffer.pu1_bufs[1] + ((stride * height) / 4);
            s_video_decode_ip.s_out_buffer.u4_num_bufs = 3;
        }
    }

    /*****************************************************************************/
    /*   API Call: Video Decode                                                  */
    /*****************************************************************************/

//  ITTIAM_DEBUG("Input buffer Filled length = %d nOffset = %d", pInBufHdr->nFilledLen, pInBufHdr->nOffset);

//  ITTIAM_DEBUG("Video Decode  process call u4_num_Bytes = %d start_code = %x ",
//                  s_video_decode_ip.u4_num_Bytes, ((WORD32 *)s_video_decode_ip.pv_stream_buffer)[0]);

//#define FILE_DUMP
#ifdef FILE_DUMP
    {
        static int cnt = 0;
        static FILE *fp = NULL;
        if(cnt == 0)
        {

            fp = fopen(INPUT_DUMP_PATH,"wb");
            if(fp == NULL)
                ITTIAM_DEBUG("fopen failed for %s", INPUT_DUMP_PATH);
        }
        else
        {
            fp = fopen(INPUT_DUMP_PATH,"ab");

            if(fp == NULL)
                ITTIAM_DEBUG( "fopen append failed for %s", INPUT_DUMP_PATH);
        }
        cnt++;
        if(fp != NULL)
        {
            ITTIAM_LOG("DEBUG Code: Dumping %d bytes from %x", s_video_decode_ip.u4_num_Bytes, s_video_decode_ip.pv_stream_buffer);
            fwrite(s_video_decode_ip.pv_stream_buffer, 1, s_video_decode_ip.u4_num_Bytes, fp);
            fclose(fp);
            fp = NULL;
        }
    }
#endif
    reprocess:
    start_ts = itGetMs();
    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    (void *)&s_video_decode_ip,
                                                    (void *)&s_video_decode_op);
    stop_ts = itGetMs();
    ITTIAM_DEBUG("TimeTaken(us) = %-6llu DelayBetweenCalls = %-6llu", (stop_ts - start_ts),
                 (start_ts - pVidDec->prev_stop_ts));

    if(1 != s_video_decode_op.u4_frame_decoded_flag)
    {
        /* If the input did not contain picture data, then ignore the associated timestamp */
        pVidDec->CheckForValidity[count] = 0;
    }

    if(pVidDec->disable_interlaced)
    {
        if((e_dec_status == IV_SUCCESS) && (pVidDec->hdr_decode_done == 0))
        {

            if(0 == s_video_decode_op.u4_progressive_frame_flag)
            {
                ITTIAM_ERROR("Found interlaced stream which is not supported");
                e_dec_status = IV_FAIL;
            }

        }
    }

    /* In case of thumbnail mode, call flush and get the first decoded frame out, if frame is decoded and not returned */
    if((pVidDec->ThumbnailMode == 1) &&
        (s_video_decode_op.u4_output_present == 0) &&
        (s_video_decode_op.u4_frame_decoded_flag == 1))
    {

        ivd_ctl_flush_ip_t s_video_flush_ip;
        ivd_ctl_flush_op_t s_video_flush_op;

        s_video_flush_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_video_flush_ip.e_sub_cmd = IVD_CMD_CTL_FLUSH;
        s_video_flush_ip.u4_size = sizeof(ivd_ctl_flush_ip_t);
        s_video_flush_op.u4_size = sizeof(ivd_ctl_flush_op_t);

        ITTIAM_LOG("Flush called in thumbnail mode");

        /*****************************************************************************/
        /*   API Call: Video Flush                                                  */
        /*****************************************************************************/
        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void *)&s_video_flush_ip,
                        (void *)&s_video_flush_op);

        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in video Flush e_dec_status = %d u4_error_code = %d\n",
                  e_dec_status, s_video_flush_op.u4_error_code);
            goto send_eos;
            //return;
        }

        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void *)&s_video_decode_ip,
                        (void *)&s_video_decode_op);
        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_LOG("Video Decode  in flush failed e_dec_status=%d u4_error_code =%d",
                  e_dec_status, s_video_decode_op.u4_error_code);
            goto send_eos;
        }
        else
        {
            ITTIAM_LOG("Video Decode  in flush output present %d",
                  s_video_decode_op.u4_output_present);

        }

    }


    if((pVidDec->seek_to_I_frame == 1) && (s_video_decode_op.u4_frame_decoded_flag == 1))
    {
        ITTIAM_DEBUG("Disabling seek_to_I_frame");
        video_skipb_frames(pVidDec);
        pVidDec->seek_to_I_frame = 0;
    }

    if((e_dec_status != IV_SUCCESS) || (0 == s_video_decode_ip.u4_num_Bytes))
    {
        pOutBufHdr->nFilledLen = 0;
        if(s_video_decode_op.u4_error_code)
            ITTIAM_ERROR("Error in video Frame decode e_dec_status = %d u4_error_code = %x\n",
                  e_dec_status, s_video_decode_op.u4_error_code);

        /* In case of shared mode, if the decoder is waiting for buffers, then return to processing after acquiring buffer */
        if(pVidDec->share_disp_buf &&
          ((s_video_decode_op.u4_error_code & 0xFF) == IVD_DEC_REF_BUF_NULL))
        {
            UWORD32 num_buf_owned = 0;
            for(i = 0; i < pVidDec->sOutPortDef.nBufferCountActual; i++)
                if(pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                & CUSTOM_BUFFERFLAG_OWNED)
                    num_buf_owned++;
            ITTIAM_DEBUG("Number of buffers owned by the component = %d",
                         num_buf_owned);
            pthread_cond_wait(&pVidDec->codec_signal, &pVidDec->codec_mutex);

            if(pVidDec->cmd_pending == 1)
                return;

            goto reprocess;
        }
        else if((s_video_decode_op.u4_error_code & 0xFF) == 0x8E)
        {
            int32_t width, height;
            width = s_video_decode_op.u4_pic_wd;
            height = s_video_decode_op.u4_pic_ht;
            ITTIAM_LOG("width as it is in DRC %d", width);
            ITTIAM_LOG("height %d", height);
            if(pVidDec->sInPortDef.format.video.eCompressionFormat
                            == OMX_VIDEO_CodingAVC)
            {
                pVidDec->sOutPortDef.format.video.nFrameWidth =
                                s_video_decode_op.u4_pic_wd;
                pVidDec->sOutPortDef.format.video.nFrameHeight =
                                s_video_decode_op.u4_pic_ht;
                ITTIAM_LOG("Decoder in DRC error u4_error_code = %x. \n",
                      s_video_decode_op.u4_error_code);
                ITTIAM_LOG("nFrameWidth = %d. \n", s_video_decode_op.u4_pic_wd);
                ITTIAM_LOG("nFrameHeight = %d. \n", s_video_decode_op.u4_pic_ht);
                pVidDec->PortReconfiguration = 1;
                pVidDec->hdr_decode_done = 0;
                pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                  pVidDec->pAppData,
                                                  OMX_EventPortSettingsChanged,
                                                  0x1, 0, NULL);
                goto send_eos;
            }
        }
        else if(IV_ISFATALERROR(s_video_decode_op.u4_error_code) || (pVidDec->hdr_decode_done == 0))
        {
            ITTIAM_ERROR("Decoder in error u4_error_code = %x. Stopping decoding.\n", s_video_decode_op.u4_error_code);
            pInBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
            header_decode_error = 1;
        }
        else if(0 == s_video_decode_ip.u4_num_Bytes)
        {
            /* When number of bytes passed to the codec is zero, it is assumed to be end of sequence,
               decoder is set in flush mode to return buffers held internally */
            OMX_BUFFERHEADERTYPE *pOutBufHdr_temp;
            IV_API_CALL_STATUS_T e_dec_status;

            ivd_ctl_flush_ip_t s_video_flush_ip;
            ivd_ctl_flush_op_t s_video_flush_op;
            int32_t width, height, stride;
            UWORD32 count;

            pOutBufHdr_temp = NULL;

            ITTIAM_LOG("Returning last few buffers from decoder");
            if(pOutBufHdr==NULL)
                ListGetEntry(pVidDec->sOutBufList, pOutBufHdr)

            s_video_flush_ip.e_cmd              = IVD_CMD_VIDEO_CTL;
            s_video_flush_ip.e_sub_cmd          = IVD_CMD_CTL_FLUSH;
            s_video_flush_ip.u4_size = sizeof(ivd_ctl_flush_ip_t);
            s_video_flush_op.u4_size = sizeof(ivd_ctl_flush_op_t);



            /*****************************************************************************/
            /*   API Call: Video Flush                                                  */
            /*****************************************************************************/
            e_dec_status  = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,(void *)&s_video_flush_ip,
                            (void *)&s_video_flush_op);


            if(e_dec_status != IV_SUCCESS)
            {
                ITTIAM_ERROR("Error in video Flush e_dec_status = %d u4_error_code = %d\n",e_dec_status, s_video_flush_op.u4_error_code);
            }
            height = pVidDec->sOutPortDef.format.video.nSliceHeight;
            stride = pVidDec->sOutPortDef.format.video.nStride;


            while((IV_SUCCESS == e_dec_status) && (pOutBufHdr != NULL))
            {
                ivd_video_decode_ip_t s_video_decode_ip;
                ivd_video_decode_op_t s_video_decode_op;

                s_video_decode_ip.e_cmd                 = IVD_CMD_VIDEO_DECODE;
                s_video_decode_ip.u4_ts                 = 0;
                s_video_decode_ip.pv_stream_buffer      = NULL;
                s_video_decode_ip.u4_num_Bytes          = 0;
                s_video_decode_ip.u4_size = sizeof(ivd_video_decode_ip_t);
                s_video_decode_op.u4_size = sizeof(ivd_video_decode_op_t);

                if(0 == pVidDec->share_disp_buf)
                {
                    s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0] =  stride * height;
                    s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1] = (stride * height) / 4;
                    s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[2] = (stride * height) / 4;

                    s_video_decode_ip.s_out_buffer.pu1_bufs[0]          = pOutBufHdr->pBuffer;
                    s_video_decode_ip.s_out_buffer.pu1_bufs[1]          = s_video_decode_ip.s_out_buffer.pu1_bufs[0] + ( stride * height);
                    s_video_decode_ip.s_out_buffer.pu1_bufs[2]          = s_video_decode_ip.s_out_buffer.pu1_bufs[1] + ((stride * height) / 4);
                    s_video_decode_ip.s_out_buffer.u4_num_bufs          = 3;
                }
                e_dec_status  = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,(void *)&s_video_decode_ip,
                                (void *)&s_video_decode_op);
                if(e_dec_status != IV_SUCCESS)
                {
                    ITTIAM_DEBUG("Video Decode  in flush failed e_dec_status=%d u4_error_code =%d",e_dec_status, s_video_decode_op.u4_error_code);
                }
                else
                {
                     ITTIAM_DEBUG("Video Decode  in flush output present %d",s_video_decode_op.u4_output_present);
                                                    pOutBufHdr->nTimeStamp = pVidDec->nTimeStamp[s_video_decode_op.u4_ts];
                                                    pVidDec->NumFBD++;
                                                    pVidDec->CheckForValidity[s_video_decode_op.u4_ts] = 0;
                    pOutBufHdr->nFilledLen = ((3 * pVidDec->sOutPortDef.format.video.nFrameWidth * pVidDec->sOutPortDef.format.video.nSliceHeight) / 2);

                    /* Check if there is an extra empty output buffer that needs to be filled */
                    if (!pOutBufHdr_temp)
                        ListGetEntry(pVidDec->sOutBufList, pOutBufHdr_temp)
                    if(pOutBufHdr_temp != NULL)
                    {
                        ITTIAM_DEBUG("NumFBD %d LINE %d",(int)pVidDec->NumFBD,__LINE__);
                        ITTIAM_DEBUG(" Calling FillBufferDone %p %d %lld",pOutBufHdr,__LINE__,pOutBufHdr->nTimeStamp);
                        pVidDec->pCallbacks->FillBufferDone(pVidDec->hSelf, pVidDec->pAppData, pOutBufHdr);
                        pOutBufHdr = NULL;

                        pOutBufHdr = pOutBufHdr_temp;
                        pOutBufHdr_temp = NULL;
                    }
                    else
                    {
                        /* If there is no extra empty output buffer that needs to be filled, treat the current one as last */
                        /* This is handled in send_eos:  */
                        break;
                    }



                }

            }



        }

        pInBufHdr->nOffset += s_video_decode_op.u4_num_bytes_consumed;
        goto send_eos;
    }

    pInBufHdr->nOffset += s_video_decode_op.u4_num_bytes_consumed;

    if(s_video_decode_op.u4_pic_wd > 0 && s_video_decode_op.u4_pic_ht > 0)
        hdr_decode_done = 1;

    if(pVidDec->hdr_decode_done == 0 && hdr_decode_done == 1)
    {

        ittiam_video_set_display_frame(pVidDec);


        pVidDec->hdr_decode_done = 1;
        data_sync_barrier();
        if((0 == pVidDec->share_disp_buf) && (pOutBufHdr))
        {
            pOutBufHdr->nTimeStamp = 0;
            pOutBufHdr->nFilledLen = 0;
            pVidDec->NumFBD++;
            ITTIAM_DEBUG("NumFBD %d LINE %d", (WORD32)pVidDec->NumFBD, __LINE__);
            ITTIAM_DEBUG(" Calling FillBufferDone bufferhdr %p line %d timestamp %lld",
                         pOutBufHdr, __LINE__, pInBufHdr->nTimeStamp);
            pVidDec->pCallbacks->FillBufferDone(pVidDec->hSelf,
                                                pVidDec->pAppData, pOutBufHdr);
            pOutBufHdr = NULL;
        }

        return;
    }

    if(IV_SUCCESS == e_dec_status)
    {
        //MediaBuffer *mbuf = drainOutputBuffer();
        UWORD32 width, height;

        width = s_video_decode_op.u4_pic_wd;
        height = s_video_decode_op.u4_pic_ht;

        pVidDec->prev_stop_ts = stop_ts;
        if(pVidDec->sInPortDef.format.video.eCompressionFormat
                        == OMX_VIDEO_CodingAVC)
        {
            if(width != pVidDec->sOutPortDef.format.video.nFrameWidth)
            {
                ITTIAM_LOG("Need port reconfiguration width%d",
                      (UWORD32)pVidDec->sOutPortDef.format.video.nFrameWidth);
                ITTIAM_LOG("Need port reconfiguration width%d",
                      (UWORD32)pVidDec->sOutPortDef.format.video.nFrameWidth);
                ITTIAM_LOG("Need port reconfiguration actual width %d",
                      (UWORD32)width);
                pVidDec->sOutPortDef.format.video.nFrameWidth = width;
                pVidDec->sOutPortDef.format.video.nFrameHeight = height;
                //pVidDec->sOutPortDef.format.video.nSliceHeight = align32(height);
                //pVidDec->sOutPortDef.format.video.nStride = align32(width);
                pVidDec->PortReconfiguration = 1;
                pVidDec->hdr_decode_done = 0;
                pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                  pVidDec->pAppData,
                                                  OMX_EventPortSettingsChanged,
                                                  0x1, 0, NULL);
                pInBufHdr->nOffset = 0; //In DRC same buffer needs to be passed again.
                goto send_eos;

            }
        }
        if(s_video_decode_op.u4_output_present == 0)
        {
            pOutBufHdr->nFilledLen = 0;
            if(s_video_decode_op.u4_error_code)
            {
                ITTIAM_DEBUG("Video Decode  get output failed e_dec_status=%d u4_error_code =%x", e_dec_status, s_video_decode_op.u4_error_code);
            }
        }
        else
        {
            ITTIAM_DEBUG("TIMESTAMP IDX s_video_decode_ip.u4_ts %d s_video_decode_op.u4_ts %d pInBufHdr->nTimeStamp %lld pVidDec->nTimeStamp[s_video_decode_op.u4_ts] %lld\n",
                         s_video_decode_ip.u4_ts, s_video_decode_op.u4_ts,
                         pInBufHdr->nTimeStamp,
                         pVidDec->nTimeStamp[s_video_decode_op.u4_ts]);

            if(1 == pVidDec->share_disp_buf)
            {
                WORD32 offset_threshold;

                offset_threshold = (pVidDec->y_height_padding / 2)
                                * pVidDec->sOutPortDef.format.video.nStride
                                + (pVidDec->width_padding / 2);
                ITTIAM_DEBUG("Y buffer ptr -- %x offset_threshold %d",
                             (WORD32)s_video_decode_op.s_disp_frm_buf.pv_y_buf,
                             offset_threshold);
                for(i = 0; i < pVidDec->sOutPortDef.nBufferCountActual; i++)
                {
                    WORD32 offset =
                                    (char *)s_video_decode_op.s_disp_frm_buf.pv_y_buf
                                                    - (char *)pVidDec->sOutBufList.pBufHdr[i]->pBuffer;
                    ITTIAM_DEBUG("Comparing with ptr -- %x offset_threshold %d offset %d",
                                 (WORD32)pVidDec->sOutBufList.pBufHdr[i]->pBuffer,
                                 offset_threshold, offset);
                    if((offset >= 0) && (offset <= offset_threshold))
                    {
                        ITTIAM_DEBUG("i value %d offset %d", i, offset);
                        break;
                    }
                }

                ITTIAM_DEBUG("Buffer ownership = %x",
                             (WORD32)pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                             & CUSTOM_BUFFERFLAG_OWNED);
                ITTIAM_DEBUG("nTimeStamp = %lld", pInBufHdr->nTimeStamp);
                if(!(pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                & CUSTOM_BUFFERFLAG_OWNED))
                    return;

                pVidDec->sOutBufList.pBufHdr[i]->nFilledLen = ((3 * width
                                * height) / 2);
                pVidDec->sOutBufList.pBufHdr[i]->nTimeStamp =
                                pInBufHdr->nTimeStamp;
                pVidDec->sOutBufList.pBufHdr[i]->nFlags =
                                pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                                & (~CUSTOM_BUFFERFLAG_OWNED);

                pVidDec->sOutBufList.pBufHdr[i]->nOffset = pVidDec->offset;

                pVidDec->sOutBufList.pBufHdr[i]->nTimeStamp =
                                pVidDec->nTimeStamp[s_video_decode_op.u4_ts];
                pVidDec->CheckForValidity[s_video_decode_op.u4_ts] = 0;

                ITTIAM_DEBUG(" Calling FillBufferDone bufferhdr %p line %d timestamp %lld",
                             pVidDec->sOutBufList.pBufHdr[i], __LINE__,
                             pVidDec->sOutBufList.pBufHdr[i]->nTimeStamp);
                pVidDec->pCallbacks->FillBufferDone(
                                pVidDec->hSelf, pVidDec->pAppData,
                                pVidDec->sOutBufList.pBufHdr[i]);
                pOutBufHdr->nFilledLen =
                                ((3
                                                * pVidDec->sOutPortDef.format.video.nFrameWidth
                                                * pVidDec->sOutPortDef.format.video.nSliceHeight)
                                                / 2);
            }
            else
            {
                ITTIAM_DEBUG("s_video_decode_op.u4_ts  before calling fill buffer done %d",
                             s_video_decode_op.u4_ts);
                pOutBufHdr->nTimeStamp =
                                pVidDec->nTimeStamp[s_video_decode_op.u4_ts];
                pVidDec->CheckForValidity[s_video_decode_op.u4_ts] = 0;
                pOutBufHdr->nFilledLen =
                                ((3 * pVidDec->sOutPortDef.format.video.nFrameWidth * pVidDec->sOutPortDef.format.video.nSliceHeight) / 2);

#ifdef OUT_FILE_DUMP
                {
                   static int cnt = 0;
                   static FILE *fp = NULL;
                   if(cnt == 0)
                   {
                       ITTIAM_DEBUG("Before fopen ");
                       fp = fopen("/data/media/ittiam/avc_dump.raw","ab");
                       if(fp == NULL)
                           ITTIAM_DEBUG("fopen failed");
                       else
                           ITTIAM_DEBUG("fopen successful");
                   }
                   else
                   {
                       fp = fopen("/data/media/ittiam/avc_dump.raw","ab");
                       if(fp == NULL)
                       ITTIAM_DEBUG( "fopen append failed");
                   }
                   cnt++;
                   if(fp != NULL)
                   {
                       //fwrite(&s_video_decode_ip.u4_num_Bytes, 4, 1, fp);
                       fwrite(pOutBufHdr->pBuffer, 1,pOutBufHdr->nFilledLen , fp);
                       fclose(fp);
                       fp = NULL;

                   }
                 }
#endif
                ITTIAM_DEBUG(" Calling FillBufferDone bufferhdr %p line %d timestamp %lld",
                             pOutBufHdr, __LINE__, pOutBufHdr->nTimeStamp);
                pVidDec->pCallbacks->FillBufferDone(pVidDec->hSelf,
                                                    pVidDec->pAppData,
                                                    pOutBufHdr);
                pOutBufHdr = NULL;
                return;
            }
        }

    }
    send_eos:
    if(pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
    {
        if(1 == pVidDec->share_disp_buf)
        {
            for(i = 0; i < pVidDec->sOutPortDef.nBufferCountActual; i++)
            {
                if(pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                & CUSTOM_BUFFERFLAG_OWNED)
                {
                    break;
                }
            }

            if(i == pVidDec->sOutPortDef.nBufferCountActual)
            {
                pthread_cond_wait(&pVidDec->codec_signal,
                                  &pVidDec->codec_mutex);
                goto send_eos;
            }

            // Copy flag to output buffer header
            if(pVidDec->sOutBufList.pBufHdr[i])
                pVidDec->sOutBufList.pBufHdr[i]->nFlags |= OMX_BUFFERFLAG_EOS;

            // Trigger event handler
            pVidDec->pCallbacks->EventHandler(pVidDec->hSelf, pVidDec->pAppData,
                                              OMX_EventBufferFlag, 0x1,
                                              pInBufHdr->nFlags, NULL);
            // Clear flag
            pInBufHdr->nFlags = 0;
            pVidDec->sOutBufList.pBufHdr[i]->nFilledLen = 0;
            pVidDec->sOutBufList.pBufHdr[i]->nTimeStamp = pInBufHdr->nTimeStamp;
            pVidDec->sOutBufList.pBufHdr[i]->nFlags =
                            pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                            & (~CUSTOM_BUFFERFLAG_OWNED);
            ITTIAM_DEBUG(" Calling FillBufferDone %p %d",
                         pVidDec->sOutBufList.pBufHdr[i], __LINE__);
            pVidDec->pCallbacks->FillBufferDone(
                            pVidDec->hSelf, pVidDec->pAppData,
                            pVidDec->sOutBufList.pBufHdr[i]);
        }
        else
        {
            pInBufHdr->nFlags = 0;
            pOutBufHdr->nTimeStamp = pInBufHdr->nTimeStamp;
            pOutBufHdr->nFilledLen = 0;
            pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
            pVidDec->NumFBD++;
            ITTIAM_DEBUG("NumFBD %d LINE %d", (WORD32)pVidDec->NumFBD,
                         __LINE__);
            ITTIAM_DEBUG(" Calling FillBufferDone %p %d %lld", pOutBufHdr,
                         __LINE__, pInBufHdr->nTimeStamp);
            pVidDec->pCallbacks->FillBufferDone(pVidDec->hSelf,
                                                pVidDec->pAppData, pOutBufHdr);
            pOutBufHdr = NULL;
            if(header_decode_error)
            {
                ITTIAM_DEBUG(" Header decode error");
#ifdef JELLY_BEAN
                pVidDec->pCallbacks->EventHandler(pVidDec->hSelf, pVidDec->pAppData, OMX_EventError, OMX_ErrorInvalidState, OMX_StateInvalid, NULL );
#endif
            }
        }
        //Prevent the process function to be called again
        pthread_mutex_lock(&pVidDec->signal_mutex);
        pVidDec->cmd_pending = 1;
        pthread_mutex_unlock(&pVidDec->signal_mutex);

        pInBufHdr->nOffset = pInBufHdr->nFilledLen;

    }

    if((0 == pVidDec->share_disp_buf) && (pOutBufHdr))
    {
        pOutBufHdr->nTimeStamp = 0;
        pOutBufHdr->nFilledLen = 0;
        pVidDec->NumFBD++;
        ITTIAM_DEBUG("NumFBD %d LINE %d", (WORD32)pVidDec->NumFBD, __LINE__);
        ITTIAM_DEBUG(" Calling FillBufferDone bufferhdr %p line %d timestamp %lld",
                     pOutBufHdr, __LINE__, pOutBufHdr->nTimeStamp);
        pVidDec->pCallbacks->FillBufferDone(pVidDec->hSelf, pVidDec->pAppData,
                                            pOutBufHdr);
        pOutBufHdr = NULL;
    }

}

void ittiam_video_set_display_frame(VIDDECTYPE* pVidDec)
{
    ivd_ctl_getbufinfo_ip_t s_ctl_dec_ip;
    ivd_ctl_getbufinfo_op_t s_ctl_dec_op;
    WORD32 outlen = 0;
    UWORD32 num_display_buf;
    UWORD32 i;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_GETBUFINFO;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_getbufinfo_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_getbufinfo_op_t);
    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    (void *)&s_ctl_dec_ip,
                                                    (void *)&s_ctl_dec_op);
    if(e_dec_status != IV_SUCCESS)
    {
        ITTIAM_ERROR("Error in Get Buf Info");
    }

    ITTIAM_DEBUG("u4_num_disp_bufs = %d", s_ctl_dec_op.u4_num_disp_bufs);
    num_display_buf = s_ctl_dec_op.u4_num_disp_bufs < pVidDec->sOutPortDef.nBufferCountActual ?
                                    s_ctl_dec_op.u4_num_disp_bufs :
                                    pVidDec->sOutPortDef.nBufferCountActual;

    if(1 == pVidDec->share_disp_buf)
        num_display_buf = pVidDec->sOutPortDef.nBufferCountActual;

    for(i = 0; i < num_display_buf;)
    {

        if(1) //pVidDec->sOutBufList.pBufHdr[i]->nFlags & CUSTOM_BUFFERFLAG_OWNED)
        {
            pVidDec->s_disp_buffers[i].u4_min_out_buf_size[0] =
                            s_ctl_dec_op.u4_min_out_buf_size[0];
            pVidDec->s_disp_buffers[i].u4_min_out_buf_size[1] =
                            s_ctl_dec_op.u4_min_out_buf_size[1];
            pVidDec->s_disp_buffers[i].u4_min_out_buf_size[2] =
                            s_ctl_dec_op.u4_min_out_buf_size[2];

            outlen = s_ctl_dec_op.u4_min_out_buf_size[0];

            if(s_ctl_dec_op.u4_min_num_out_bufs > 1)
                outlen += s_ctl_dec_op.u4_min_out_buf_size[1];

            if(s_ctl_dec_op.u4_min_num_out_bufs > 2)
                outlen += s_ctl_dec_op.u4_min_out_buf_size[2];


            pVidDec->s_disp_buffers[i].pu1_bufs[0] =
                            pVidDec->sOutBufList.pBufHdr[i]->pBuffer;

            ITTIAM_DEBUG("Output buffer ptr %d -- %x", i,
                         (WORD32)pVidDec->sOutBufList.pBufHdr[i]->pBuffer);

            if(pVidDec->s_disp_buffers[i].pu1_bufs[0] == NULL)
                ITTIAM_LOG("\nAllocation failure\n");

            if(pVidDec->mProcessorType == SAMSUNG_GENERIC)
            {
                ITTIAM_LOG("Entered Exynos specific shared mode u4_min_num_out_bufs  %d",
                      s_ctl_dec_op.u4_min_num_out_bufs);
                if(s_ctl_dec_op.u4_min_num_out_bufs > 1)
                    pVidDec->s_disp_buffers[i].pu1_bufs[1] = (OMX_U8*)pVidDec->inp_uv_id_mapping[i];

                if(s_ctl_dec_op.u4_min_num_out_bufs > 2)
                    pVidDec->s_disp_buffers[i].pu1_bufs[2] = (OMX_U8*)pVidDec->inp_uv_id_mapping[i];

                pVidDec->s_disp_buffers[i].u4_num_bufs = s_ctl_dec_op.u4_min_num_out_bufs;

                pVidDec->disp_buf_id_mapping[i] = pVidDec->s_disp_buffers[i].pu1_bufs[0];

                i++;
            }
            else
            {

                if(s_ctl_dec_op.u4_min_num_out_bufs > 1)
                    pVidDec->s_disp_buffers[i].pu1_bufs[1] = pVidDec->s_disp_buffers[i].pu1_bufs[0] + (s_ctl_dec_op.u4_min_out_buf_size[0]);

                if(s_ctl_dec_op.u4_min_num_out_bufs > 2)
                    pVidDec->s_disp_buffers[i].pu1_bufs[2] = pVidDec->s_disp_buffers[i].pu1_bufs[1] + (s_ctl_dec_op.u4_min_out_buf_size[1]);

                pVidDec->s_disp_buffers[i].u4_num_bufs = s_ctl_dec_op.u4_min_num_out_bufs;

                pVidDec->disp_buf_id_mapping[i] = pVidDec->s_disp_buffers[i].pu1_bufs[0];

                i++;
            }
        }
        else
        {
            ITTIAM_DEBUG("Buffer NOT with the component. Waiting ... ");
            usleep(5000);
        }
    }

    if(1 == pVidDec->share_disp_buf)
    {
        ivd_set_display_frame_ip_t s_set_display_frame_ip;
        ivd_set_display_frame_op_t s_set_display_frame_op;

        s_set_display_frame_ip.e_cmd = IVD_CMD_SET_DISPLAY_FRAME;
        s_set_display_frame_ip.u4_size = sizeof(ivd_set_display_frame_ip_t);
        s_set_display_frame_op.u4_size = sizeof(ivd_set_display_frame_op_t);

        s_set_display_frame_ip.num_disp_bufs = num_display_buf;

        ITTIAM_DEBUG("s_set_display_frame_ip.num_disp_bufs = %d",
                     s_set_display_frame_ip.num_disp_bufs);

        memcpy(&(s_set_display_frame_ip.s_disp_buffer),
               &(pVidDec->s_disp_buffers),
               s_set_display_frame_ip.num_disp_bufs * sizeof(ivd_out_bufdesc_t));

        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void *)&s_set_display_frame_ip,
                        (void *)&s_set_display_frame_op);

        if(IV_SUCCESS != e_dec_status)
        {
            ITTIAM_ERROR("Error in Set display frame %d error code %x", e_dec_status,
                  s_set_display_frame_op.u4_error_code);
        }

        pthread_mutex_unlock(&pVidDec->codec_mutex);
        while(1)
        {
            UWORD32 num_buf_owned = 0;
            for(i = 0; i < pVidDec->sOutPortDef.nBufferCountActual; i++)
            {
                if(pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                & CUSTOM_BUFFERFLAG_OWNED)
                {
                    num_buf_owned++;
                }
            }
#ifdef ICS
            if(num_buf_owned < (pVidDec->sOutPortDef.nBufferCountMin - 2))
#else
            if(num_buf_owned < (pVidDec->sOutPortDef.nBufferCountMin))
#endif
            {
                ITTIAM_DEBUG("num_buf_owned =%d pVidDec->sOutPortDef.nBufferCountMin =%d",
                             num_buf_owned,
                             (WORD32)pVidDec->sOutPortDef.nBufferCountMin);
                ITTIAM_DEBUG("SDF Buffer NOT with the component. Waiting ... ");
                usleep(5000);
            }
            else
            {
                break;
            }
        };

        pVidDec->wait_for_op_buffers = 0;
        pthread_mutex_lock(&pVidDec->codec_mutex);

        for(i = 0; i < pVidDec->sOutPortDef.nBufferCountActual; i++)
        {
            if(pVidDec->sOutBufList.pBufHdr[i]->nFlags
                            & CUSTOM_BUFFERFLAG_OWNED)
            {
                ittiam_video_release_display_frame(pVidDec, i);
            }
        }
    }


    /* Set the decoder in frame decode mode */
    {

        ivd_ctl_set_config_ip_t s_ctl_dec_ip;
        ivd_ctl_set_config_op_t s_ctl_dec_op;
        s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
        s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_NONE;

        s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
        s_ctl_dec_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
        s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
        s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
        s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

        pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                         (void *)&s_ctl_dec_ip,
                                         (void *)&s_ctl_dec_op);
    }
}

void ittiam_video_release_display_frame(VIDDECTYPE* pVidDec,
                                        UWORD32 disp_buf_id)
{
    ivd_rel_display_frame_ip_t s_video_rel_disp_ip;
    ivd_rel_display_frame_op_t s_video_rel_disp_op;
    IV_API_CALL_STATUS_T e_dec_status;
    s_video_rel_disp_ip.e_cmd = IVD_CMD_REL_DISPLAY_FRAME;
    s_video_rel_disp_ip.u4_size = sizeof(ivd_rel_display_frame_ip_t);
    s_video_rel_disp_op.u4_size = sizeof(ivd_rel_display_frame_op_t);
    s_video_rel_disp_ip.u4_disp_buf_id = disp_buf_id;

    ITTIAM_LOG("Releasing buffer with id %d\n", disp_buf_id);
    e_dec_status = pVidDec->iVdec_cxa8_api_function(
                    pVidDec->g_DECHDL, (void *)&s_video_rel_disp_ip,
                    (void *)&s_video_rel_disp_op);

    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_ERROR("Error in release display frame %d error code %x", e_dec_status,
              s_video_rel_disp_op.u4_error_code);
    }

}

void ittiam_video_decoder_deinit(VIDDECTYPE* pVidDec)
{

    iv_retrieve_mem_rec_ip_t s_clear_dec_ip;
    iv_retrieve_mem_rec_op_t s_clear_dec_op;
    UWORD32 u4_num_mem_recs;
    s_clear_dec_ip.pv_mem_rec_location = pVidDec->pv_mem_rec_location;

     //Average Dec time changes
    ALOGE("Decode Stats: Total time %lld \t total frames %d \t average time %lld \n total delay time %lld \t Average delay time %lld",
    total_decode_time,  num_decoded_buffers , total_decode_time/num_decoded_buffers, total_delay_time, total_delay_time/num_decoded_buffers);
    s_clear_dec_ip.e_cmd              = IV_CMD_RETRIEVE_MEMREC;
    s_clear_dec_ip.u4_size = sizeof(iv_retrieve_mem_rec_ip_t);
    s_clear_dec_op.u4_size = sizeof(iv_retrieve_mem_rec_op_t);

    pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL, (void *)&s_clear_dec_ip,
                                     (void *)&s_clear_dec_op);

    {
        iv_mem_rec_t *ps_mem_rec;
        UWORD16 u2_i;

        u4_num_mem_recs = s_clear_dec_op.u4_num_mem_rec_filled;

        ps_mem_rec = s_clear_dec_ip.pv_mem_rec_location;

        for(u2_i = 0; u2_i < u4_num_mem_recs; u2_i++)
        {
            free(ps_mem_rec->pv_base);
            ps_mem_rec++;
        }
        free(pVidDec->pv_mem_rec_location);
    }

}

void ittiam_video_decoder_flush(VIDDECTYPE* pVidDec)
{
    IV_API_CALL_STATUS_T e_dec_status;

    ivd_ctl_flush_ip_t s_video_flush_ip;
    ivd_ctl_flush_op_t s_video_flush_op;
    int32_t width, height, stride;
    void *dummy_out_buf;

    s_video_flush_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_video_flush_ip.e_sub_cmd = IVD_CMD_CTL_FLUSH;
    s_video_flush_ip.u4_size = sizeof(ivd_ctl_flush_ip_t);
    s_video_flush_op.u4_size = sizeof(ivd_ctl_flush_op_t);

    ITTIAM_LOG("Flush called");

    UWORD32 count;

    for(count = 0; count < STAMP_IDX; count++)
    {
        pVidDec->nTimeStamp[count] = 0;
        pVidDec->CheckForValidity[count] = 0;

    }

    /*****************************************************************************/
    /*   API Call: Video Flush                                                  */
    /*****************************************************************************/
    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    (void *)&s_video_flush_ip,
                                                    (void *)&s_video_flush_op);

    if(e_dec_status != IV_SUCCESS)
    {
        ITTIAM_DEBUG("Error in video Flush e_dec_status = %d u4_error_code = %d\n",
                     e_dec_status, s_video_flush_op.u4_error_code);
        return;
    }
    height = pVidDec->sOutPortDef.format.video.nSliceHeight;
    stride = pVidDec->sOutPortDef.format.video.nStride;

    dummy_out_buf = OMX_OSAL_Malloc((3 * stride * height) / 2);
    if(dummy_out_buf == NULL)
        return;

    while(IV_SUCCESS == e_dec_status)
    {
        ivd_video_decode_ip_t s_video_decode_ip;
        ivd_video_decode_op_t s_video_decode_op;

        s_video_decode_ip.e_cmd = IVD_CMD_VIDEO_DECODE;
        s_video_decode_ip.u4_ts = 0;
        s_video_decode_ip.pv_stream_buffer = NULL;
        s_video_decode_ip.u4_num_Bytes = 0;
        s_video_decode_ip.u4_size = sizeof(ivd_video_decode_ip_t);
        s_video_decode_op.u4_size = sizeof(ivd_video_decode_op_t);

        if(0 == pVidDec->share_disp_buf)
        {
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0] = stride
                            * height;
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1] = (stride
                            * height) / 4;
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[2] = (stride
                            * height) / 4;

            s_video_decode_ip.s_out_buffer.pu1_bufs[0] = dummy_out_buf;
            s_video_decode_ip.s_out_buffer.pu1_bufs[1] =
                            s_video_decode_ip.s_out_buffer.pu1_bufs[0]
                                            + (stride * height);
            s_video_decode_ip.s_out_buffer.pu1_bufs[2] =
                            s_video_decode_ip.s_out_buffer.pu1_bufs[1]
                                            + ((stride * height) / 4);
            s_video_decode_ip.s_out_buffer.u4_num_bufs = 3;
        }

        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->g_DECHDL, (void *)&s_video_decode_ip,
                        (void *)&s_video_decode_op);
        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_DEBUG("Video Decode  in flush failed e_dec_status=%d u4_error_code =%d",
                         e_dec_status, s_video_decode_op.u4_error_code);
        }
        else
        {
            ITTIAM_DEBUG("Video Decode  in flush output present %d",
                         s_video_decode_op.u4_output_present);

        }

    }
    OMX_OSAL_Free(dummy_out_buf);
    return;
}

WORD32 get_num_disp_buffers(UWORD32 width, UWORD32 height)
{
    UWORD32 level_idc = 31;
    UWORD32 i4_size = 0;
    UWORD32 ui16_frmWidthInMbs = width / 16;
    UWORD32 ui16_frmHeightInMbs = height / 16;

    i4_size = 6912000;

    i4_size = i4_size / (ui16_frmWidthInMbs * (ui16_frmHeightInMbs));

    i4_size = i4_size / 384;
    i4_size = i4_size < 16 ? i4_size : 16;
    i4_size += (i4_size + 1);
    i4_size = i4_size < 32 ? i4_size : 32;
    return (i4_size);
}

void mpeg4_disable_qpel(VIDDECTYPE* pVidDec)
{
    imp4d_cxa8_ctl_disable_qpel_ip_t s_ip;
    imp4d_cxa8_ctl_disable_qpel_op_t s_op;
    IV_API_CALL_STATUS_T e_dec_status;
    s_ip.u4_size = sizeof(imp4d_cxa8_ctl_disable_qpel_ip_t);
    s_op.u4_size = sizeof(imp4d_cxa8_ctl_disable_qpel_op_t);
    s_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ip.e_sub_cmd = IMP4D_CXA8_CMD_CTL_DISABLE_QPEL;
    s_ip.u4_disable_qpel_level = 1; /* Disable Qpel in B frames */

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    (void *)&s_ip,
                                                    (void *)&s_op);
    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for mpeg4_disable_qpel Failed\n");
    }

}

void mpeg4_enable_qpel(VIDDECTYPE* pVidDec)
{
    imp4d_cxa8_ctl_disable_qpel_ip_t s_ip;
    imp4d_cxa8_ctl_disable_qpel_op_t s_op;
    IV_API_CALL_STATUS_T e_dec_status;
    s_ip.u4_size = sizeof(imp4d_cxa8_ctl_disable_qpel_ip_t);
    s_op.u4_size = sizeof(imp4d_cxa8_ctl_disable_qpel_op_t);
    s_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ip.e_sub_cmd = IMP4D_CXA8_CMD_CTL_DISABLE_QPEL;
    s_ip.u4_disable_qpel_level = 0; /* Enable Qpel */

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    (void *)&s_ip,
                                                    (void *)&s_op);

    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for mpeg4_enable_qpel Failed\n");
    }
}

void avc_set_deblocking_lvl(VIDDECTYPE* pVidDec, UWORD32 level)
{
    ih264d_cxa8_ctl_disable_deblock_ip_t s_ip;
    ih264d_cxa8_ctl_disable_deblock_op_t s_op;
    void *pv_api_ip, *pv_api_op;
    IV_API_CALL_STATUS_T e_dec_status;
    s_ip.u4_size = sizeof(ih264d_cxa8_ctl_disable_deblock_ip_t);
    s_ip.u4_disable_deblk_level = level;

    s_op.u4_size = sizeof(ih264d_cxa8_ctl_disable_deblock_op_t);

    pv_api_ip = (void *)&s_ip;
    pv_api_op = (void *)&s_op;

    s_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ip.e_sub_cmd = IH264D_CXA8_CMD_CTL_DISABLE_DEBLOCK;

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    pv_api_ip, pv_api_op);

    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for avc_set_deblocking_lvl Failed\n");
    }

}


void video_skipb_frames(VIDDECTYPE* pVidDec)
{
    ivd_ctl_set_config_ip_t s_ctl_dec_ip;
    ivd_ctl_set_config_op_t s_ctl_dec_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
    s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_B;

    s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_dec_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    (void *)&s_ctl_dec_ip,
                                                    (void *)&s_ctl_dec_op);
    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for video_skipb_frames Failed\n");
    }
}

void video_skip_none(VIDDECTYPE* pVidDec)
{
    ivd_ctl_set_config_ip_t s_ctl_dec_ip;
    ivd_ctl_set_config_op_t s_ctl_dec_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
    s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_NONE;

    s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_dec_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    (void *)&s_ctl_dec_ip,
                                                    (void *)&s_ctl_dec_op);
    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for video_skip_none Failed\n");
    }
}

void video_skip_pb_frames(VIDDECTYPE* pVidDec)
{
    ivd_ctl_set_config_ip_t s_ctl_dec_ip;
    ivd_ctl_set_config_op_t s_ctl_dec_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
    s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_PB;

    s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_dec_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->g_DECHDL,
                                                    (void *)&s_ctl_dec_ip,
                                                    (void *)&s_ctl_dec_op);

    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for video_skip_pb_frames Failed\n");
    }
}

void codec_print(char *str)
{
    ITTIAM_DEBUG("CODEC DEBUG PRINT %s\n", str);
}

void codec_print1(char *str, int val)
{
    ITTIAM_DEBUG("CODEC DEBUG PRINT");
    ITTIAM_DEBUG(str, val);
}

#ifdef __cplusplus
//}
#endif /* __cplusplus */
