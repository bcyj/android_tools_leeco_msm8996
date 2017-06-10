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
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>



#include "ittiam_datatypes.h"
#include "iv.h"
#include "ive.h"
#include "ih264_cxa8.h"

#include <OMX_Ittiam.h>
#include <OMX_Ittiam_Venc.h>

#include<utils/Log.h>

//#define USE_DUMMY_INP
//#define USE_DUMMY_OUT
#define TARGET_FRAME_RATE 30000
#define TARGET_BIT_RATE 6000000
#define MAX_FRAME_RATE  30000
#define MAX_BIT_RATE    6000000
long long prev_stop_ts, start_ts, stop_ts;
long long aver_encode_time;
long long itGetMs(void)
{
    struct timeval t;
    long long currTime;

    if(gettimeofday(&t,NULL) == -1)
    {
         printf("Error in gettimeofday. It has returned -1. \n");
    }
    currTime = ((t.tv_sec *1000 *1000) + (t.tv_usec));
    return currTime;
}
void ittiam_video_encoder_deinit(VIDENCTYPE* pVidEnc);
IV_API_CALL_STATUS_T ittiam_video_encoder_init(VIDENCTYPE* pVidEnc);
void ittiam_buffer_mapping(VIDENCTYPE* pVidEnc);
IV_API_CALL_STATUS_T ittiam_video_encoder_process(VIDENCTYPE* pVidEnc, OMX_BUFFERHEADERTYPE *pInBufHdr, OMX_BUFFERHEADERTYPE *pOutBufHdr);
void ittapi_mem_init(void **mem);
IV_API_CALL_STATUS_T ittapi_memtab_alloc(mem_mgr *mem_rec,UWORD32 u4_num_mem_recs, iv_mem_rec_t *ps_mem_recs);
void ittapi_memtab_free(mem_mgr *mem_rec);
OMX_U32 align2048(OMX_U32 a);
void initialize_variables(ih264_cxa8_ctl_set_config_ip_t *ps_set_config_ip_t);
void update_configuration_params (VIDENCTYPE *pVidEnc);

UWORD8 dummy_inp[MAX_WIDTH * MAX_HEIGHT * 3 / 2];
UWORD8 dummy_out[MAX_WIDTH * MAX_HEIGHT * 3 / 2];
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
    OMX_U32 cmddata;
    ThrCmdType cmd;
    IV_API_CALL_STATUS_T e_enc_return_status;
    // Variables related to decoder buffer handling
    OMX_BUFFERHEADERTYPE *pInBufHdr = NULL;
    OMX_BUFFERHEADERTYPE *pOutBufHdr = NULL;
    OMX_MARKTYPE *pMarkBuf = NULL;

    // Variables related to decoder timeouts
    struct timespec abstime;
    int ret_val;
    int nTimeout;


    // Recover the pointer to my component specific data
    VIDENCTYPE* pVidEnc = (VIDENCTYPE*)pThreadData;

    while (1){
       fd1 = pVidEnc->cmdpipe[0];
       FD_ZERO(&rfds);
       FD_SET(fd1,&rfds);

       // Check for new command
       i = select(pVidEnc->cmdpipe[0]+1, &rfds, NULL, NULL, NULL);

       if (FD_ISSET(pVidEnc->cmdpipe[0], &rfds)){
          // retrieve command and data from pipe
          pthread_mutex_lock(&pVidEnc->pipes_mutex);
          read(pVidEnc->cmdpipe[0], &cmd, sizeof(cmd));
          read(pVidEnc->cmddatapipe[0], &cmddata, sizeof(cmddata));
          pthread_mutex_unlock(&pVidEnc->pipes_mutex);




          // State transition command
      if (cmd == SetState){
              ITTIAM_DEBUG("SetState Command");
            pthread_mutex_lock(&pVidEnc->signal_mutex);
            pVidEnc->cmd_pending = 0;
            pthread_mutex_unlock(&pVidEnc->signal_mutex);

             // If the parameter states a transition to the same state
             //   raise a same state transition error.
             if (pVidEnc->state == (OMX_STATETYPE)(cmddata))
                pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                                OMX_EventError, OMX_ErrorSameState, 0 , NULL);
             else{
            // transitions/callbacks made based on state transition table
                // cmddata contains the target state
            switch ((OMX_STATETYPE)(cmddata)){
                   case OMX_StateInvalid:
                   pVidEnc->state = OMX_StateInvalid;
                   pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                               OMX_EventError, OMX_ErrorInvalidState, 0 , NULL);
                   pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                               OMX_EventCmdComplete, OMX_CommandStateSet, pVidEnc->state, NULL);
               break;
           case OMX_StateLoaded:
                  if (pVidEnc->state == OMX_StateIdle ||
                                                pVidEnc->state == OMX_StateWaitForResources){
                    ret_val = 0;

                    pthread_mutex_lock(&pVidEnc->signal_mutex);

                    // Transition happens only when the ports are unpopulated
                    if ((pVidEnc->sInBufList.nAllocSize > 0) || pVidEnc->sOutBufList.nAllocSize > 0){

                        //gettimeofday(&abstime,NULL);
                        clock_gettime(CLOCK_REALTIME , &abstime);
                        abstime.tv_sec += OMX_TIMEOUT_SEC;
								ITTIAM_DEBUG("Waiting for buffers to be freed");
                        ret_val = pthread_cond_timedwait(&pVidEnc->buffers_signal,
                                                    &pVidEnc->signal_mutex, &abstime);

                    }
                    pthread_mutex_unlock(&pVidEnc->signal_mutex);

                    if(ret_val == 0)
                    {
                        // DeInitialize the decoder when moving from Idle to Loaded
                        if(pVidEnc->state == OMX_StateIdle && pVidEnc->initdone == 1)
                        {
                            pVidEnc->initdone = 0;
                            ittiam_video_encoder_deinit(pVidEnc);
                        }

                        pVidEnc->state = OMX_StateLoaded;
                        pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,
                                   pVidEnc->pAppData, OMX_EventCmdComplete,
                                   OMX_CommandStateSet, pVidEnc->state, NULL);

                    }
                    else
                    {
                        // Transition to loaded failed
                        pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,
                                    pVidEnc->pAppData, OMX_EventError,
                                    OMX_ErrorTimeout, 0 , NULL);

                    }
                  }
               else
                  pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                                    OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);
               break;
           case OMX_StateIdle:

                if (pVidEnc->state == OMX_StateInvalid)
                  pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                                 OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);
                else{
                    // Return buffers if currently in pause and executing
                    if (pVidEnc->state == OMX_StatePause ||
                                              pVidEnc->state == OMX_StateExecuting){
                        if(pInBufHdr)
                        {
                             // Return input buffer to client for refill
                             pVidEnc->pCallbacks->EmptyBufferDone(pVidEnc->hSelf, pVidEnc->pAppData, pInBufHdr);
                             pVidEnc->NumEBD++;
                             ITTIAM_DEBUG("NumEBD %d %d",(WORD32)pVidEnc->NumEBD,__LINE__);
                             pInBufHdr = NULL;
                        }

                        ListFlushEntries(pVidEnc->sInBufList, pVidEnc)
                        ListFlushEntries(pVidEnc->sOutBufList, pVidEnc)
                        //FlushOutputBuffers(pVidEnc);
                    }

                    ret_val = 0;

                    pthread_mutex_lock(&pVidEnc->signal_mutex);

                     // All "enabled" Ports have to be "populated", before transition completes
                     if ((pVidEnc->sInPortDef.bEnabled != pVidEnc->sInPortDef.bPopulated) ||
                        (pVidEnc->sOutPortDef.bEnabled != pVidEnc->sOutPortDef.bPopulated)) {
                            //gettimeofday(&abstime,NULL);
                            clock_gettime(CLOCK_REALTIME , &abstime);
                            abstime.tv_sec += 1;
                            ret_val = pthread_cond_timedwait(&pVidEnc->buffers_signal,
                                            &pVidEnc->signal_mutex, &abstime);

                    }
                    pthread_mutex_unlock(&pVidEnc->signal_mutex);

                    if(ret_val == 0)
                    {
                         // Initialize the decoder when moving from Loaded to Idle
                         if(pVidEnc->state == OMX_StateLoaded)
                         {
                                    e_enc_return_status = ittiam_video_encoder_init(pVidEnc);
                                    if(e_enc_return_status != IV_SUCCESS)
                                    {
                                        ITTIAM_ERROR("Encoder Init failed \n");
                                        pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData, OMX_EventError, OMX_ErrorInvalidState, OMX_StateInvalid, NULL );

                                    }
                                    else
                                    {
                                        pVidEnc->initdone = 1;
                                    }
                          }

                         ITTIAM_DEBUG("Setting state to Idle");
                         pVidEnc->state = OMX_StateIdle;
                         pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,
                                 pVidEnc->pAppData, OMX_EventCmdComplete,
                                       OMX_CommandStateSet, pVidEnc->state, NULL);

                         // Allocate input buffer
                         //pInBuf = (OMX_U8*)OMX_OSAL_Malloc(2048);

                    }
                    else {
                         // Idle transition failed
                         pVidEnc->state = OMX_StateIdle;
                         pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,
                                 pVidEnc->pAppData, OMX_EventCmdComplete,
                                       OMX_CommandStateSet, pVidEnc->state, NULL);

                        }

                  }
                break;
           case OMX_StateExecuting:
              // Transition can only happen from pause or idle state
              if (pVidEnc->state == OMX_StateIdle ||
                                                       pVidEnc->state == OMX_StatePause){
                    ITTIAM_DEBUG("State transition to idle recieved");

                  // Return buffers if currently in pause
                  if (pVidEnc->state == OMX_StatePause){
                         ListFlushEntries(pVidEnc->sInBufList, pVidEnc)
                        // FlushOutputBuffers(pVidEnc);
                  }
                  pVidEnc->state = OMX_StateExecuting;
                  pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                                OMX_EventCmdComplete, OMX_CommandStateSet, pVidEnc->state, NULL);

                  }
               else
               {
                  ITTIAM_DEBUG("In correct State transition to idle recieved");
                  pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                                     OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);
                                 }
               break;
           case OMX_StatePause:
                   // Transition can only happen from idle or executing state
               if (pVidEnc->state == OMX_StateIdle || pVidEnc->state == OMX_StateExecuting){

                  pVidEnc->state = OMX_StatePause;
                  pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                            OMX_EventCmdComplete, OMX_CommandStateSet, pVidEnc->state, NULL);
                      }
               else
                  pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                                     OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);
                break;
           case OMX_StateWaitForResources:
                  if (pVidEnc->state == OMX_StateLoaded) {
                      pVidEnc->state = OMX_StateWaitForResources;
                      pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                                 OMX_EventCmdComplete, OMX_CommandStateSet, pVidEnc->state, NULL);
                  }
                  else
                      pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                                     OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);
                  break;
            default:
                ITTIAM_ERROR("It shouldn't be here");
                break;
           }
          }
         }
      else if (cmd == DisablePort){
              ITTIAM_DEBUG("DisablePort Command");
          // Disable Port(s)
          // cmddata contains the port index to be stopped.
          // It is assumed that 0 is input and 1 is output port for this component
          // The cmddata value -1 means that both input and output ports will be stopped.
          if (cmddata == 0x0 || (WORD32)cmddata == -1){
              // Return all input buffers
                 if(pInBufHdr)
             {
                 // Return input buffer to client for refill
                 pVidEnc->pCallbacks->EmptyBufferDone(pVidEnc->hSelf, pVidEnc->pAppData, pInBufHdr);
                 pInBufHdr = NULL;
                 pVidEnc->NumEBD++;
                    //ALOGD("NumEBD %d %d",pVidEnc->NumEBD,__LINE__);
             }
                  ListFlushEntries(pVidEnc->sInBufList, pVidEnc)
              // Disable port
                 pVidEnc->sInPortDef.bEnabled = OMX_FALSE;
          }
           if (cmddata == 0x1 || (WORD32)cmddata == -1){
                // Return all output buffers
                 //    FlushOutputBuffers(pVidEnc);

            //  ittiam_video_decoder_flush(pVidEnc);
                  // Disable port
                pVidEnc->sOutPortDef.bEnabled = OMX_FALSE;
          }
          // Wait for all buffers to be freed
          nTimeout = 0;
          while (1){
                if (cmddata == 0x0 && !pVidEnc->sInPortDef.bPopulated){
                   // Return cmdcomplete event if input unpopulated
                   pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                         OMX_EventCmdComplete, OMX_CommandPortDisable, 0x0, NULL);
                   break;
                   }
                if (cmddata == 0x1 && !pVidEnc->sOutPortDef.bPopulated){
                   // Return cmdcomplete event if output unpopulated
                   pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                         OMX_EventCmdComplete, OMX_CommandPortDisable, 0x1, NULL);
                   break;
                   }
                if ((WORD32)cmddata == -1 &&  !pVidEnc->sInPortDef.bPopulated &&
                                                  !pVidEnc->sOutPortDef.bPopulated){
                       // Return cmdcomplete event if inout & output unpopulated
                    pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                         OMX_EventCmdComplete, OMX_CommandPortDisable, 0x0, NULL);
                    pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                         OMX_EventCmdComplete, OMX_CommandPortDisable, 0x1, NULL);
                    break;
                   }
                if (nTimeout++ > OMX_MAX_TIMEOUTS){
                    pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,pVidEnc->pAppData,
                                   OMX_EventError, OMX_ErrorPortUnresponsiveDuringDeallocation,
                                                                                     0 , NULL);
                       break;
                   }
                millisleep(OMX_TIMEOUT_MSEC);
                }
              }
      else if (cmd == EnablePort){
              ITTIAM_DEBUG("EnablePort Command");
          // Enable Port(s)
          // cmddata contains the port index to be restarted.
          // It is assumed that 0 is input and 1 is output port for this component.
          // The cmddata value -1 means both input and output ports will be restarted.

          if (cmddata == 0x0 || (WORD32)cmddata == -1)
             pVidEnc->sInPortDef.bEnabled = OMX_TRUE;

          if (cmddata == 0x1 || (WORD32)cmddata == -1)
             pVidEnc->sOutPortDef.bEnabled = OMX_TRUE;

          // Wait for port to be populated
          nTimeout = 0;
          while (1){
                   // Return cmdcomplete event if input port populated
                if (cmddata == 0x0 && (pVidEnc->state == OMX_StateLoaded ||
                                                        pVidEnc->sInPortDef.bPopulated)){
                   pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                        OMX_EventCmdComplete, OMX_CommandPortEnable, 0x0, NULL);
                    break;
                   }
                   // Return cmdcomplete event if output port populated
                if (cmddata == 0x1 && (pVidEnc->state == OMX_StateLoaded ||
                                                             pVidEnc->sOutPortDef.bPopulated)){
                   pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,
                                     pVidEnc->pAppData, OMX_EventCmdComplete,
                                                 OMX_CommandPortEnable, 0x1, NULL);
                   break;
                }
                   // Return cmdcomplete event if input and output ports populated
                if ((WORD32)cmddata == -1 && (pVidEnc->state == OMX_StateLoaded ||
                                             (pVidEnc->sInPortDef.bPopulated &&
                                                    pVidEnc->sOutPortDef.bPopulated))){
                    pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,
                                           pVidEnc->pAppData, OMX_EventCmdComplete,
                                                       OMX_CommandPortEnable, 0x0, NULL);
                    pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf,
                                           pVidEnc->pAppData, OMX_EventCmdComplete,
                                                        OMX_CommandPortEnable, 0x1, NULL);
                    break;
                }

                if (nTimeout++ > OMX_MAX_TIMEOUTS){
                    pVidEnc->pCallbacks->EventHandler(
                                              pVidEnc->hSelf,pVidEnc->pAppData,
                                              OMX_EventError,
                                           OMX_ErrorPortUnresponsiveDuringAllocation,0,NULL);
                    break;
                }

                millisleep(OMX_TIMEOUT_MSEC);

                }
              }
      else if (cmd == Flush){
              ITTIAM_DEBUG("Flush Command");
            pthread_mutex_lock(&pVidEnc->signal_mutex);
            pVidEnc->cmd_pending = 0;
            pthread_mutex_unlock(&pVidEnc->signal_mutex);
              // Flush port(s)
                  // cmddata contains the port index to be flushed.
                  // It is assumed that 0 is input and 1 is output port for this component
                  // The cmddata value -1 means that both input and output ports will be flushed.
              if (cmddata == 0x0 || (WORD32)cmddata == -1){
                 // Return all input buffers and send cmdcomplete
            if(pInBufHdr)
            {
                // Return input buffer to client for refill
                pVidEnc->pCallbacks->EmptyBufferDone(pVidEnc->hSelf, pVidEnc->pAppData, pInBufHdr);
                pInBufHdr = NULL;
                pVidEnc->NumEBD++;
                //ALOGD("NumEBD %d %d",pVidEnc->NumEBD,__LINE__);
            }
                     ListFlushEntries(pVidEnc->sInBufList, pVidEnc)
                     pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                              OMX_EventCmdComplete, OMX_CommandFlush, 0x0, NULL);
             }
              if (cmddata == 0x1 || (WORD32)cmddata == -1){

                pVidEnc->wait_for_op_buffers = 1;
                 // Return all output buffers and send cmdcomplete
                  //   FlushOutputBuffers(pVidEnc);
                 pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                              OMX_EventCmdComplete, OMX_CommandFlush, 0x1, NULL);
             }
              }
      else if (cmd == StopThread){
              ITTIAM_DEBUG("StopThread Command");
          // Kill thread
              goto EXIT;
              }
      else if (cmd == FillBuf){

              // Fill buffer
                  ListSetEntry(pVidEnc->sOutBufList, ((OMX_BUFFERHEADERTYPE*) cmddata))
                  }
      else if (cmd == EmptyBuf){
              // Empty buffer
              ListSetEntry(pVidEnc->sInBufList, ((OMX_BUFFERHEADERTYPE *) cmddata))
              // Mark current buffer if there is outstanding command
          if (pMarkBuf){
             ((OMX_BUFFERHEADERTYPE *)(cmddata))->hMarkTargetComponent =
                                                       pMarkBuf->hMarkTargetComponent;
             ((OMX_BUFFERHEADERTYPE *)(cmddata))->pMarkData = pMarkBuf->pMarkData;
             pMarkBuf = NULL;
             }
          }
      else if (cmd == MarkBuf){
              ITTIAM_DEBUG("MarkBuf Command");

              if (!pMarkBuf)
                 pMarkBuf = (OMX_MARKTYPE *)(cmddata);
              }
      }
       // Buffer processing
       // Only happens when the component is in executing state.
       while (pVidEnc->state == OMX_StateExecuting &&
           pVidEnc->sInPortDef.bEnabled &&
           pVidEnc->sOutPortDef.bEnabled &&
           ((pVidEnc->sInBufList.nSizeOfList > 0) || pInBufHdr) &&
           ((pVidEnc->sOutBufList.nSizeOfList > 0) || pOutBufHdr) &&
           (pVidEnc->cmd_pending == 0)){

          if(pInBufHdr == NULL)
          {
	                ListGetEntry(pVidEnc->sInBufList, pInBufHdr)
                ITTIAM_DEBUG("Get input buffer Filled length = %d nOffset = %d", (WORD32)pInBufHdr->nFilledLen, (WORD32)pInBufHdr->nOffset);
                pInBufHdr->nOffset = 0;
          }
              // If there is no output buffer, get one from list
          if (!pOutBufHdr)
                 ListGetEntry(pVidEnc->sOutBufList, pOutBufHdr)

          // Check for EOS flag
          if (pInBufHdr){

            // Check for mark buffers
            if (pInBufHdr->pMarkData){
                    // Copy mark to output buffer header
                if (pOutBufHdr){
                   pOutBufHdr->pMarkData = pInBufHdr->pMarkData;
                       // Copy handle to output buffer header
                       pOutBufHdr->hMarkTargetComponent = pInBufHdr->hMarkTargetComponent;}
              }
            // Trigger event handler
                if (pInBufHdr->hMarkTargetComponent == pVidEnc->hSelf && pInBufHdr->pMarkData)
              pVidEnc->pCallbacks->EventHandler(pVidEnc->hSelf, pVidEnc->pAppData,
                                                        OMX_EventMark, 0, 0, pInBufHdr->pMarkData);
                    }
            // NULL check for I/O buffers
            if((pInBufHdr==NULL) || (pOutBufHdr==NULL))
            {
                  ITTIAM_DEBUG("Encoder I/O buffers are NULL");
                  break;
            }
             ITTIAM_DEBUG("Calling process");
             // Decode frame
             start_ts = itGetMs();
             pthread_mutex_lock(&pVidEnc->codec_mutex);
             ittiam_video_encoder_process(pVidEnc, pInBufHdr, pOutBufHdr);
             pthread_mutex_unlock(&pVidEnc->codec_mutex);
             stop_ts = itGetMs();
             ALOGE("Ittiam: Start time  = %lld Stop time = %lld time comsumed = %lld delay = %lld",
                                    start_ts, stop_ts, stop_ts - start_ts, start_ts - prev_stop_ts);
             prev_stop_ts = stop_ts;

		aver_encode_time += (stop_ts - start_ts);

        if(pVidEnc->data_generation_started)
        {
            if (pInBufHdr)
             {
                 // Return input buffer to client for refill
                 ITTIAM_DEBUG("Calling empty buffer done");
                 pVidEnc->pCallbacks->EmptyBufferDone(pVidEnc->hSelf, pVidEnc->pAppData, pInBufHdr);
                 pInBufHdr = NULL;
                 pVidEnc->NumEBD++;
                ITTIAM_DEBUG("NumEBD %d %d",(WORD32)pVidEnc->NumEBD,(WORD32)__LINE__);
             }
        }

             if (pOutBufHdr->nFilledLen != 0)
             {

                ITTIAM_DEBUG("Calling fill buffer done");
                pVidEnc->pCallbacks->FillBufferDone(pVidEnc->hSelf, pVidEnc->pAppData, pOutBufHdr);
                pOutBufHdr = NULL;
                pVidEnc->NumFBD++;
                ITTIAM_DEBUG("NumFBD %d %d",(WORD32)pVidEnc->NumFBD,(WORD32)__LINE__);
             }

          }

       }

EXIT:
    return (void*)OMX_ErrorNone;
}
IV_API_CALL_STATUS_T ittiam_video_encoder_init(VIDENCTYPE* pVidEnc) {

    IV_API_CALL_STATUS_T e_enc_status;
    UWORD32 u4_num_mem_recs;
    void *pv_api_ip,*pv_api_op;

    aver_encode_time = 0;

    {
        /* --------------------------------------------------------------------- */
        /*                      Getting Number of MemRecords                     */
        /* --------------------------------------------------------------------- */

        ih264_cxa8_num_mem_rec_ip_t     num_mem_rec_ip_t;
        ih264_cxa8_num_mem_rec_op_t     num_mem_rec_op_t;

        num_mem_rec_ip_t.s_ive_num_mem_rec_ip_t.u4_size = sizeof(ih264_cxa8_num_mem_rec_ip_t);
        num_mem_rec_ip_t.s_ive_num_mem_rec_ip_t.e_cmd = IV_CMD_GET_NUM_MEM_REC;
        num_mem_rec_op_t.s_ive_num_mem_rec_op_t.u4_size = sizeof(ih264_cxa8_num_mem_rec_op_t);

        pv_api_ip = &num_mem_rec_ip_t;
        pv_api_op = &num_mem_rec_op_t;

        ITTIAM_DEBUG("Get Number of Mem Records");
        e_enc_status = ih264e_cxa8_api_function(0,pv_api_ip,pv_api_op);
        if(IV_SUCCESS != e_enc_status)
        {
            ITTIAM_ERROR("Error in get mem records");
            return e_enc_status;
        }

        u4_num_mem_recs = num_mem_rec_op_t.s_ive_num_mem_rec_op_t.u4_num_mem_rec;
    }


    pVidEnc->pv_mem_rec_location    = (iv_mem_rec_t*)malloc(u4_num_mem_recs * sizeof(iv_mem_rec_t));
    if(pVidEnc->pv_mem_rec_location == NULL)
        ITTIAM_ERROR("\nAllocation failure\n");


    {
        /* --------------------------------------------------------------------- */
        /*                      Getting MemRecords Attributes                    */
        /* --------------------------------------------------------------------- */

        ih264_cxa8_fill_mem_rec_ip_t    fill_mem_rec_ip_t;
        ih264_cxa8_fill_mem_rec_op_t    fill_mem_rec_op_t;

        fill_mem_rec_ip_t.s_ive_fill_mem_rec_ip_t.u4_size = sizeof(ih264_cxa8_fill_mem_rec_ip_t);
        fill_mem_rec_op_t.s_ive_fill_mem_rec_op_t.u4_size = sizeof(ih264_cxa8_fill_mem_rec_op_t);
        fill_mem_rec_ip_t.s_ive_fill_mem_rec_ip_t.e_cmd = IV_CMD_FILL_NUM_MEM_REC;
        fill_mem_rec_ip_t.s_ive_fill_mem_rec_ip_t.pv_mem_rec_location = pVidEnc->pv_mem_rec_location;
        fill_mem_rec_ip_t.s_ive_fill_mem_rec_ip_t.u4_max_frm_wd = pVidEnc->sInPortDef.format.video.nFrameWidth;
        fill_mem_rec_ip_t.s_ive_fill_mem_rec_ip_t.u4_max_frm_ht = pVidEnc->sInPortDef.format.video.nFrameHeight;
        fill_mem_rec_ip_t.u4_nmb_grp                            = 8;

        iv_mem_rec_t     *ps_mem_rec;

        pv_api_ip = &fill_mem_rec_ip_t;
        pv_api_op = &fill_mem_rec_op_t;
        ITTIAM_DEBUG("Fill Mem Record attributes");

        e_enc_status = ih264e_cxa8_api_function(0,pv_api_ip,pv_api_op);

        if(IV_SUCCESS != e_enc_status)
        {
            ITTIAM_ERROR("Error in fill mem record attributes");
            return e_enc_status;
        }

        ps_mem_rec  = pVidEnc->pv_mem_rec_location;
        pVidEnc->mem   = NULL;
        ittapi_mem_init(&(pVidEnc->mem));
        e_enc_status = ittapi_memtab_alloc(pVidEnc->mem,u4_num_mem_recs, ps_mem_rec);
        if(e_enc_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in MALLOC of Memtab buffers \n ");
            return e_enc_status;
        }

#ifdef USE_DUMMY_INP
        memset(dummy_inp, 128, sizeof(dummy_inp));
#endif

    }

    {
        /* --------------------------------------------------------------------- */
        /*                        Codec Instance Creation                        */
        /* --------------------------------------------------------------------- */
        ih264_cxa8_init_ip_t                  init_ip_t;
        ih264_cxa8_init_op_t                  init_op_t;

        iv_mem_rec_t *mem_tab;
        mem_tab=(iv_mem_rec_t*)pVidEnc->pv_mem_rec_location;

        pVidEnc->iVenc_cxa8_api_function = ih264e_cxa8_api_function;
        void   *dec_fxns = (void *)pVidEnc->iVenc_cxa8_api_function;


        init_ip_t.s_ive_init_ip_t.e_cmd  = IV_CMD_INIT;
        init_ip_t.s_ive_init_ip_t.u4_size = sizeof(ih264_cxa8_init_ip_t);
        init_op_t.s_ive_init_op_t.u4_size = sizeof(ih264_cxa8_init_op_t);
        init_ip_t.s_ive_init_ip_t.u4_enc_quality_config = IVE_USER_DEFINED;
        init_ip_t.s_ive_init_ip_t.u4_frm_max_ht = pVidEnc->sInPortDef.format.video.nFrameHeight;
        init_ip_t.s_ive_init_ip_t.u4_frm_max_wd = pVidEnc->sInPortDef.format.video.nFrameWidth;
        init_ip_t.s_ive_init_ip_t.u4_max_num_bframes    = 0;
		if(pVidEnc->sInColorFormat == (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar)
			pVidEnc->input_color_format = IV_YUV_420P;
		else if(pVidEnc->sInColorFormat == (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar)
			pVidEnc->input_color_format = IV_YUV_420SP_UV;
		else if(pVidEnc->sInColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar)
			pVidEnc->input_color_format = IV_YUV_420SP_VU;
		else if(pVidEnc->sInColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FORMATYUV420PackedSemiPlanar32m)
			pVidEnc->input_color_format = IV_YUV_420SP_UV;
		else
		{
			ITTIAM_ERROR("\nInvalid Color format \n");
			e_enc_status = IV_FAIL;
			return (e_enc_status);
		}
        init_ip_t.s_ive_init_ip_t.input_colour_format = pVidEnc->input_color_format;
        init_ip_t.s_ive_init_ip_t.content_type  = IV_PROGRESSIVE;
        init_ip_t.s_ive_init_ip_t.recon_colour_format   = -1;
        init_ip_t.s_ive_init_ip_t.u4_stuffing_disabled  = 0;
        init_ip_t.s_ive_init_ip_t.u4_enable_quarterpel  = 0;
        init_ip_t.s_ive_init_ip_t.u4_max_qp = 51;
        init_ip_t.s_ive_init_ip_t.u4_air_refresh_period = 1000;
        init_ip_t.s_ive_init_ip_t.u4_slice_packet_mode  = 0;
        init_ip_t.s_ive_init_ip_t.u4_slice_packet_param_value   = 20000;
        init_ip_t.s_ive_init_ip_t.u4_enable_random_access_points    = 0;
        init_ip_t.s_ive_init_ip_t.u4_max_searchrange_xy = 32 | (32 << 16);
        init_ip_t.s_ive_init_ip_t.u4_max_bitrate    = MAX_BIT_RATE;
        init_ip_t.s_ive_init_ip_t.u4_max_framerate  = MAX_FRAME_RATE;
        init_ip_t.s_ive_init_ip_t.u4_rate_control_config    = 2;
        init_ip_t.s_ive_init_ip_t.u4_num_mem_rec    = u4_num_mem_recs;
        init_ip_t.s_ive_init_ip_t.pv_mem_rec_location = mem_tab;
        init_ip_t.e_level = IH264E_LEVEL_31;
        init_ip_t.e_profile = IH264E_PROFILE_BP;
        init_ip_t.u4_nmb_grp = 8;

        pv_api_ip = &init_ip_t;
        pv_api_op = &init_op_t;

        pVidEnc->g_ENCHDL       = (iv_obj_t*) mem_tab[0].pv_base;
        pVidEnc->g_ENCHDL->pv_fxns = dec_fxns;
        pVidEnc->g_ENCHDL->u4_size =  sizeof(iv_obj_t);

        ITTIAM_DEBUG("Codec Instance Creation");
        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL,pv_api_ip,pv_api_op);
        if(e_enc_status != IV_SUCCESS)
        {
            UWORD32 *pu4_error_code;
            pu4_error_code =(UWORD32 *)pv_api_op + 1;
            ITTIAM_ERROR("Error in Init %x ",*(pu4_error_code));
            return e_enc_status;
        }

    }


    {
        /* --------------------------------------------------------------------- */
        /*                        set ME Information Enable Flag                 */
        /* --------------------------------------------------------------------- */
        ih264_cxa8_ctl_set_me_info_enable_ip_t s_ctl_set_me_info_enable_ip;
        ih264_cxa8_ctl_set_me_info_enable_op_t s_ctl_set_me_info_enable_op;
        UWORD32 u4_me_info_enable = 0;

		ITTIAM_DEBUG("Setting ME Information Flag");
        s_ctl_set_me_info_enable_ip.e_cmd = IVE_CMD_VIDEO_CTL;
        s_ctl_set_me_info_enable_ip.e_sub_cmd = IH264_CXA8_CMD_CTL_SET_ME_INFO_ENABLE;
        s_ctl_set_me_info_enable_ip.u4_me_info_enable = u4_me_info_enable;
        s_ctl_set_me_info_enable_ip.u4_size = sizeof(ih264_cxa8_ctl_set_me_info_enable_ip_t);
        s_ctl_set_me_info_enable_op.u4_size = sizeof(ih264_cxa8_ctl_set_me_info_enable_op_t);
        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL, (void *) &s_ctl_set_me_info_enable_ip,
                (void *) &s_ctl_set_me_info_enable_op);
        if(IV_SUCCESS != e_enc_status)
        {
            ITTIAM_ERROR("Error in Setting ME Information Flag: 0x%x\n", s_ctl_set_me_info_enable_op.u4_error_code);
            return e_enc_status;
        }
    }

    {
        /* --------------------------------------------------------------------- */
        /*                        set number of cores                            */
        /* --------------------------------------------------------------------- */
        ih264_cxa8_ctl_set_num_cores_ip_t s_ctl_set_cores_ip;
        ih264_cxa8_ctl_set_num_cores_op_t s_ctl_set_cores_op;

        s_ctl_set_cores_ip.e_cmd = IVE_CMD_VIDEO_CTL;
        s_ctl_set_cores_ip.e_sub_cmd = IH264_CXA8_CMD_CTL_SET_NUM_CORES;
        s_ctl_set_cores_ip.u4_num_cores = 2;

		ITTIAM_DEBUG("Setting Number of Cores");

        if(pVidEnc->quadcore)
        {
            s_ctl_set_cores_ip.u4_num_cores = 3;
        }
        s_ctl_set_cores_ip.u4_size = sizeof(ih264_cxa8_ctl_set_num_cores_ip_t);
        s_ctl_set_cores_op.u4_size = sizeof(ih264_cxa8_ctl_set_num_cores_op_t);

        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL, (void *)&s_ctl_set_cores_ip,(void *)&s_ctl_set_cores_op);
        if(IV_SUCCESS != e_enc_status)
        {
            ITTIAM_ERROR("Error in Setting number of cores : 0x%x\n", s_ctl_set_cores_op.u4_error_code);
            return e_enc_status;
        }

    }

    /* --------------------------------------------------------------------- */
    /*                        Get Codec Version                              */
    /* --------------------------------------------------------------------- */

    {
        ih264_cxa8_ctl_getversioninfo_ip_t s_ctl_set_getversioninfo_ip;
        ih264_cxa8_ctl_getversioninfo_op_t s_ctl_set_getversioninfo_op;
        char i1_version_string[512];

		ITTIAM_DEBUG("Get Version of Encoder Library");
        s_ctl_set_getversioninfo_ip.s_ive_ctl_getversioninfo_ip_t.e_cmd = IVE_CMD_VIDEO_CTL;
        s_ctl_set_getversioninfo_ip.s_ive_ctl_getversioninfo_ip_t.e_sub_cmd = IVE_CMD_CTL_GETVERSION;
        s_ctl_set_getversioninfo_ip.s_ive_ctl_getversioninfo_ip_t.pv_version_buffer = i1_version_string;
        s_ctl_set_getversioninfo_ip.s_ive_ctl_getversioninfo_ip_t.u4_version_buffer_size = sizeof(i1_version_string);
        s_ctl_set_getversioninfo_ip.s_ive_ctl_getversioninfo_ip_t.u4_size = sizeof(ih264_cxa8_ctl_getversioninfo_ip_t);
        s_ctl_set_getversioninfo_op.s_ive_ctl_getversioninfo_op_t.u4_size = sizeof(ih264_cxa8_ctl_getversioninfo_op_t);
        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL, (void *) &s_ctl_set_getversioninfo_ip,
                (void *) &s_ctl_set_getversioninfo_op);
        if (e_enc_status != IV_SUCCESS) {
            ITTIAM_ERROR("Getting version info call failed : 0x%x\n",s_ctl_set_getversioninfo_op.s_ive_ctl_getversioninfo_op_t.u4_error_code);
            return e_enc_status;
        }
        ITTIAM_LOG("CODEC VERSION %s", (char *)s_ctl_set_getversioninfo_ip.s_ive_ctl_getversioninfo_ip_t.pv_version_buffer);
    }


    {

        /* --------------------------------------------------------------------- */
        /*                        set Deblock level                              */
        /* --------------------------------------------------------------------- */
        ih264_cxa8_ctl_disable_deblock_ip_t s_ip;
        ih264_cxa8_ctl_disable_deblock_op_t s_op;
        s_ip.u4_size = sizeof(ih264_cxa8_ctl_disable_deblock_ip_t);
        s_ip.u4_disable_deblk_level = 3;

        if(pVidEnc->highqualityencode)
        {
            s_ip.u4_disable_deblk_level = 0;
        }

        s_op.u4_size = sizeof(ih264_cxa8_ctl_disable_deblock_op_t);

		ITTIAM_DEBUG("Setting Deblock Level");
        pv_api_ip = (void *)&s_ip;
        pv_api_op = (void *)&s_op;

        s_ip.e_cmd = IVE_CMD_VIDEO_CTL;
        s_ip.e_sub_cmd = IH264_CXA8_CMD_CTL_DISABLE_DEBLOCK;

        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL, pv_api_ip, pv_api_op);

        if(e_enc_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("set disable deblock level failed");
            return e_enc_status;
        }


    }

    {
        /* ---------------------------------------------------------------------------*/
        /*                      Set VUI Parameters                                    */
        /* ---------------------------------------------------------------------------*/
        ih264_cxa8_vui_ip_t vui_ip_t;
        ih264_cxa8_vui_op_t vui_op_t;

        vui_ip_t.u4_size = sizeof(ih264_cxa8_vui_ip_t);
        vui_op_t.u4_size = sizeof(ih264_cxa8_vui_op_t);
        vui_ip_t.e_cmd = IVE_CMD_VIDEO_CTL;
        vui_ip_t.e_sub_cmd  = IVE_CMD_CTL_SET_VUI_PARAMS;
        vui_ip_t.video_signal_type_present_flag = 0;
        vui_ip_t.uc_vui_seq_parameters_present_flag = 0;
        vui_ip_t.video_full_range_flag = 0;
        vui_ip_t.colour_description_present_flag = 0;
        vui_ip_t.vui_colour_primaries = 0;
        vui_ip_t.vui_transfer_characteristics = 0;
        vui_ip_t.matrix_coefficients = 0;
        vui_ip_t.vui_video_format = 0;

		ITTIAM_DEBUG("Setting VUI Parameters");
        pv_api_ip = &vui_ip_t;
        pv_api_op = &vui_op_t;

        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL,pv_api_ip,pv_api_op);
        if(e_enc_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in Set VUI parameter %x ",vui_op_t.u4_error_code);
            return e_enc_status;
        }

    }

    {
        /* --------------------------------------------------------------------- */
        /*                      Get I/O Buffer Requirement                       */
        /* --------------------------------------------------------------------- */
        ih264_cxa8_ctl_getbufinfo_ip_t  get_buf_info_ip_t;
        ih264_cxa8_ctl_getbufinfo_op_t  get_buf_info_op_t;

        get_buf_info_ip_t.s_ive_ctl_getbufinfo_ip_t.u4_size = sizeof(ih264_cxa8_ctl_getbufinfo_ip_t);
        get_buf_info_op_t.s_ive_ctl_getbufinfo_op_t.u4_size = sizeof(ih264_cxa8_ctl_getbufinfo_op_t);

        get_buf_info_ip_t.s_ive_ctl_getbufinfo_ip_t.e_cmd = IVE_CMD_VIDEO_CTL;
        get_buf_info_ip_t.s_ive_ctl_getbufinfo_ip_t.e_sub_cmd = IVE_CMD_CTL_GETBUFINFO;
        ITTIAM_DEBUG("Get I/O Buffer requirement");
        pv_api_ip = &get_buf_info_ip_t;
        pv_api_op = &get_buf_info_op_t;
        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL,pv_api_ip,pv_api_op);
         if(e_enc_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in Second Init");
            return e_enc_status;
        }
        pVidEnc->u4_min_num_out_bufs = get_buf_info_op_t.s_ive_ctl_getbufinfo_op_t.u4_min_num_out_bufs;
        pVidEnc->u4_min_out_buf_size = get_buf_info_op_t.s_ive_ctl_getbufinfo_op_t.u4_min_out_buf_size[0];
    }

    ittiam_buffer_mapping(pVidEnc);
	pVidEnc->temp_buffer = (OMX_U8*)malloc(MAX_WIDTH*MAX_HEIGHT*2);
	ITTIAM_DEBUG("Init Done");

    return e_enc_status;
}

IV_API_CALL_STATUS_T ittiam_video_encoder_process(VIDENCTYPE* pVidEnc, OMX_BUFFERHEADERTYPE *pInBufHdr, OMX_BUFFERHEADERTYPE *pOutBufHdr)
{
    IV_API_CALL_STATUS_T e_enc_status = IV_SUCCESS;
    ih264_cxa8_video_encode_ip_t    video_encode_ip_t;
    ih264_cxa8_video_encode_op_t    video_encode_op_t;
    iv_yuv_buf_t input_buf;
    iv_bufs_t   out_bufs_t;
    UWORD32 pu4_buf_size[MAX_NUM_IO_BUFS];
    UWORD8 *pu4_buf_list[MAX_NUM_IO_BUFS];
    void *pv_api_ip,*pv_api_op;
    UWORD32 i;
    UWORD8 *u4_debug_variable;

    UWORD32 u4_y_buf_size,u4_uv_buf_size;
    if(pVidEnc->sInPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420PackedSemiPlanar32m4ka)
    {
        u4_y_buf_size = pVidEnc->sInPortDef.format.video.nFrameWidth * (((pVidEnc->sInPortDef.format.video.nFrameHeight + 31) >> 5) << 5);
    }
    else
    {
        u4_y_buf_size = pVidEnc->sInPortDef.format.video.nFrameWidth * pVidEnc->sInPortDef.format.video.nFrameHeight;
    }

    if(pVidEnc->mUseLifeEffects)
    {
        u4_y_buf_size = align2048(u4_y_buf_size);
    }

    u4_uv_buf_size = u4_y_buf_size >> 1;

    if(!(pVidEnc->hdr_encode_done))
    {
        /*****************************************************************************/
        /*   API Call: Set the run time (dynamics) parameters before the Process call*/
        /*****************************************************************************/

        ih264_cxa8_ctl_set_config_ip_t               set_config_ip_t;
        ih264_cxa8_ctl_set_config_op_t               set_config_op_t;

        set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_size = sizeof(ih264_cxa8_ctl_set_config_ip_t);
        set_config_op_t.s_ive_ctl_set_config_op_t.u4_size = sizeof(ih264_cxa8_ctl_set_config_op_t);

		ITTIAM_DEBUG("Set Control Config Parameters");

        initialize_variables(&set_config_ip_t);
        set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_generate_header = 1;
        set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_capture_width =  pVidEnc->sInPortDef.format.video.nFrameWidth;
        set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_inp_width = pVidEnc->sInPortDef.format.video.nFrameWidth;
        set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_inp_height = pVidEnc->sInPortDef.format.video.nFrameHeight;
		/*
		set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_src_frame_rate  =  pVidEnc->sInPortDef.format.video.xFramerate;
		set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_tgt_frame_rate = pVidEnc->sInPortDef.format.video.xFramerate;
		set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_target_bitrate = pVidEnc->sInPortDef.format.video.nBitrate;
            */

        if(pVidEnc->highqualityencode)
        {
            set_config_ip_t.u4_high_quality_me = 1;
            set_config_ip_t.u4_enable_halfpel = 1;
            set_config_ip_t.u4_enable_i4x4 = 1;
        }

        pVidEnc->hdr_encode_done = 1;

        pv_api_ip = &set_config_ip_t;
        pv_api_op = &set_config_op_t;

        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL,pv_api_ip,pv_api_op);

        if(e_enc_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("SetParameter failed");
            return e_enc_status;
        }

        video_encode_ip_t.s_ive_video_encode_ip_t.u4_size = sizeof(ih264_cxa8_video_encode_ip_t);
        video_encode_op_t.s_ive_video_encode_op_t.u4_size = sizeof(ih264_cxa8_video_encode_op_t);
        video_encode_ip_t.s_ive_video_encode_ip_t.e_cmd = IVE_CMD_VIDEO_ENCODE;
        video_encode_ip_t.s_ive_video_encode_ip_t.input_buf = NULL;
        video_encode_ip_t.s_ive_video_encode_ip_t.recon_buf = NULL;
        video_encode_ip_t.s_ive_video_encode_ip_t.u4_inp_timestamp  = pVidEnc->FramesEncoded;
        video_encode_ip_t.s_ive_video_encode_ip_t.u4_topfield_first = 1;
		video_encode_ip_t.s_ive_video_encode_ip_t.u1_colour_format = pVidEnc->input_color_format;
        video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs = &out_bufs_t;
        out_bufs_t.pu1_buffs = pu4_buf_list;
        out_bufs_t.pu4_buf_sizes = pu4_buf_size;
        video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->u4_size = sizeof(iv_bufs_t);
        video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->u4_numbufs = pVidEnc->u4_min_num_out_bufs;
        video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->pu4_buf_sizes[0]  = (pVidEnc->sInPortDef.format.video.nFrameWidth * pVidEnc->sInPortDef.format.video.nFrameHeight * 3)>>2;
#ifdef USE_DUMMY_OUT
        video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->pu1_buffs[0]  = dummy_out;
#else
        video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->pu1_buffs[0]  = pOutBufHdr->pBuffer;
#endif

        pv_api_ip   = &video_encode_ip_t;
        pv_api_op = &video_encode_op_t;

        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL,pv_api_ip,pv_api_op);

        if(e_enc_status != 0)
        {
            UWORD32 *pu4_error_code;
            pu4_error_code = (UWORD32 *)pv_api_op + 1;
            ITTIAM_ERROR("error_code = %d\n",*(pu4_error_code));
            pOutBufHdr->nFilledLen = 0;
            ITTIAM_ERROR("Video Encode  header failed e_enc_status=%d",e_enc_status);
            return e_enc_status;
        }
        else
        {
            pOutBufHdr->nFilledLen = video_encode_op_t.s_ive_video_encode_op_t.u4_bytes_generated;
            pOutBufHdr->nTimeStamp = pInBufHdr->nTimeStamp;
            pOutBufHdr->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;

        }

        set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_generate_header    = 0;
        pv_api_ip = &set_config_ip_t;
        pv_api_op = &set_config_op_t;

        e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL,pv_api_ip,pv_api_op);
        if(e_enc_status != 0)
        {
            UWORD32 *pu4_error_code;
            pu4_error_code = (UWORD32 *)pv_api_op + 1;
            ITTIAM_ERROR("error_code = %d\n",*(pu4_error_code));
            return e_enc_status;
        }

        ITTIAM_DEBUG("Header generated");
		ITTIAM_DEBUG("Start Encoding");

        return e_enc_status;

    }



#ifdef ICS
    for(i = 0; i < NUM_IN_BUFFERS; i++)
    {
        if((pInBufHdr->pBuffer) == pVidEnc->inp_buf_id_mapping[i])
        {
            ITTIAM_DEBUG("Found matching id %d", i);
            pVidEnc->input_buffer = (OMX_U8*)pVidEnc->inp_y_id_mapping[i];
            //memcpy(pVidEnc->temp_buffer,pVidEnc->input_buffer,pVidEnc->buffersize);

            break;
        }
    }
#else
    pVidEnc->input_buffer = pInBufHdr->pBuffer;
#endif

// #define DUMP_INPUT_TO_STORAGE
#ifdef DUMP_INPUT_TO_STORAGE
    {
        static int cnt = 0;
        static FILE *fp = NULL;
		int bytes_written = 0;
        if(cnt == 0)
        {
            fp = fopen(INPUT_DUMP_PATH,"wb");
            if(fp == NULL)
                ITTIAM_ERROR("fopen failed");
            else
                ITTIAM_DEBUG("fopen successful");
        }
        else
        {
            fp = fopen(INPUT_DUMP_PATH,"ab");
            if(fp == NULL)
                ITTIAM_ERROR( "fappend failed");
        }
        cnt++;
        //fwrite(&s_video_decode_ip.u4_num_Bytes, 4, 1, fp);
		bytes_written = fwrite(pVidEnc->input_buffer, 1, (pVidEnc->sInPortDef.format.video.nFrameWidth*pVidEnc->sInPortDef.format.video.nFrameHeight*3)>>1, fp);
		ITTIAM_LOG("%d Bytes Written \n",bytes_written);
		fclose(fp);
    }
#endif

#ifdef USE_DUMMY_INP
    pVidEnc->input_buffer = dummy_inp;
    for(i = MAX_HEIGHT/2 - 32; i < MAX_HEIGHT/2 + 32; i++)
        memset(&dummy_inp[i * MAX_WIDTH], pVidEnc->NumEBD, MAX_WIDTH);
#endif

    //ITTIAM_DEBUG("pVidEnc->input_buffer %x", (WORD32)pVidEnc->input_buffer);
    video_encode_ip_t.s_ive_video_encode_ip_t.u4_size = sizeof(ih264_cxa8_video_encode_ip_t);
    video_encode_op_t.s_ive_video_encode_op_t.u4_size = sizeof(ih264_cxa8_video_encode_op_t);
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf = &input_buf;
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_size  = sizeof(iv_yuv_buf_t);
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_y_wd = pVidEnc->sInPortDef.format.video.nFrameWidth;
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_y_ht = pVidEnc->sInPortDef.format.video.nFrameHeight;
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_y_strd = pVidEnc->sInPortDef.format.video.nFrameWidth;

    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_u_wd = (pVidEnc->sInPortDef.format.video.nFrameWidth)>> 1;
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_u_ht = (pVidEnc->sInPortDef.format.video.nFrameHeight) >> 1 ;
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_u_strd = (pVidEnc->sInPortDef.format.video.nFrameWidth) >> 1;

    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_v_wd = (pVidEnc->sInPortDef.format.video.nFrameWidth)>> 1;
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_v_ht = (pVidEnc->sInPortDef.format.video.nFrameHeight) >> 1 ;
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->u4_v_strd = (pVidEnc->sInPortDef.format.video.nFrameWidth) >> 1;
    video_encode_ip_t.s_ive_video_encode_ip_t.u4_inp_timestamp  = pVidEnc->FramesEncoded;
    video_encode_ip_t.s_ive_video_encode_ip_t.recon_buf = NULL;

    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->pv_y_buf  = pVidEnc->input_buffer;
    video_encode_ip_t.s_ive_video_encode_ip_t.input_buf->pv_u_buf  = pVidEnc->input_buffer + u4_y_buf_size;


    video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs = &out_bufs_t;
    out_bufs_t.pu1_buffs = pu4_buf_list;
    out_bufs_t.pu4_buf_sizes = pu4_buf_size;
    video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->u4_size = sizeof(iv_bufs_t);
    video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->u4_numbufs = pVidEnc->u4_min_num_out_bufs;

#ifdef USE_DUMMY_OUT
    video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->pu1_buffs[0]  = dummy_out;
#else
    video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->pu1_buffs[0]  = pOutBufHdr->pBuffer;
#endif

    video_encode_ip_t.s_ive_video_encode_ip_t.out_bufs->pu4_buf_sizes[0]  = (pVidEnc->sInPortDef.format.video.nFrameWidth * pVidEnc->sInPortDef.format.video.nFrameHeight * 3)>>2;
    video_encode_ip_t.s_ive_video_encode_ip_t.u4_topfield_first = 1;
    video_encode_ip_t.s_ive_video_encode_ip_t.u1_colour_format = pVidEnc->input_color_format;
	/*
	set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_src_frame_rate  =  pVidEnc->sInPortDef.format.video.xFramerate;
	set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_tgt_frame_rate = pVidEnc->sInPortDef.format.video.xFramerate;
	set_config_ip_t.s_ive_ctl_set_config_ip_t.u4_target_bitrate = pVidEnc->sInPortDef.format.video.nBitrate;
	*/
    video_encode_ip_t.s_ive_video_encode_ip_t.e_cmd = IVE_CMD_VIDEO_ENCODE;
    pv_api_ip = &video_encode_ip_t;
    pv_api_op = &video_encode_op_t;

    e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL,pv_api_ip,pv_api_op);

    if(e_enc_status != 0)
    {
        UWORD32 *pu4_error_code;
        pu4_error_code = (UWORD32 *)pv_api_op + 1;
        pOutBufHdr->nFilledLen = 0;
        ITTIAM_ERROR("Video Encode  IVE_CMD_VIDEO_ENCODE failed e_enc_status=%d error_code = 0x%x",e_enc_status, video_encode_op_t.s_ive_video_encode_op_t.u4_error_code);
    }
    else
    {

        if(pVidEnc->BufferUnmaprequired)
        munmap (pVidEnc->input_buffer, pVidEnc->buffersize);

        pVidEnc->data_generation_started = 1;
        pVidEnc->FramesEncoded++;
        if (video_encode_op_t.s_ive_video_encode_op_t.u4_encoded_frame_type == 0)
        {
            ITTIAM_DEBUG("May be key Frame");
            pOutBufHdr->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
        }
        pOutBufHdr->nFilledLen = video_encode_op_t.s_ive_video_encode_op_t.u4_bytes_generated;
        pOutBufHdr->nTimeStamp = pInBufHdr->nTimeStamp;
        ITTIAM_DEBUG("Encoded Frame :: %d, bytes Generated %d",(int)pVidEnc->FramesEncoded,
        (int)video_encode_op_t.s_ive_video_encode_op_t.u4_bytes_generated);

    }
    return e_enc_status;
}


void ittiam_video_encoder_deinit(VIDENCTYPE* pVidEnc) {

    IV_API_CALL_STATUS_T e_enc_status;
    void *pv_api_ip,*pv_api_op;

    ih264_cxa8_retrieve_mem_rec_ip_t retrieve_mem_ip_t;
    ih264_cxa8_retrieve_mem_rec_op_t retrieve_mem_op_t;

    retrieve_mem_ip_t.s_ive_retrieve_mem_rec_ip_t.u4_size = sizeof(ih264_cxa8_retrieve_mem_rec_ip_t);
    retrieve_mem_op_t.s_ive_retrieve_mem_rec_op_t.u4_size = sizeof(ih264_cxa8_retrieve_mem_rec_op_t);

    retrieve_mem_ip_t.s_ive_retrieve_mem_rec_ip_t.e_cmd = IV_CMD_RETRIEVE_MEMREC;
    retrieve_mem_ip_t.s_ive_retrieve_mem_rec_ip_t.ps_mem_rec_location = pVidEnc->pv_mem_rec_location;

    ALOGE("Ittiam Stats: Total time %lld \t total frames %d \t average time %lld ",
                         aver_encode_time,  pVidEnc->FramesEncoded , aver_encode_time/pVidEnc->FramesEncoded);


    pv_api_ip = &retrieve_mem_ip_t;
    pv_api_op = &retrieve_mem_op_t;

    e_enc_status = ih264e_cxa8_api_function(pVidEnc->g_ENCHDL,pv_api_ip,pv_api_op);
    if(e_enc_status != IV_SUCCESS)
    {
        ITTIAM_ERROR("Deinit failed e_enc_status=%d",e_enc_status);
    }
    else
    {
        ittapi_memtab_free(pVidEnc->mem);
        free(pVidEnc->pv_mem_rec_location);
        free(pVidEnc->temp_buffer);
    }

}
void ittiam_buffer_mapping(VIDENCTYPE* pVidEnc)
{
    UWORD32 i;

    for(i = 0; i < pVidEnc->sInPortDef.nBufferCountMin; )
    {
        pVidEnc->inp_buf_id_mapping[i] = pVidEnc->sInBufList.pBufHdr[i]->pBuffer;
        ITTIAM_DEBUG("ittiam_buffer_mapping pVidEnc->inp_buf_id_mapping[%d] %x",i, (WORD32)pVidEnc->inp_buf_id_mapping[i]);
        pVidEnc->inp_y_id_mapping[i] = NULL;
        i++;
    }
}

void ittapi_mem_init(void **mem)
{
    UWORD32 u4_i;
    mem_mgr *mem_rec;

    *mem = (void  *)malloc(sizeof(mem_mgr));
    mem_rec = (mem_mgr *)*mem;
    mem_rec->u4_mem_cnt = 0;

    for(u4_i = 0; u4_i < MAX_MEM_REC; u4_i++ )
    {
        mem_rec->memRec[u4_i].u1_align_mem_ptr = NULL;
        mem_rec->memRec[u4_i].u1_mem_ptr       = NULL;
        mem_rec->memRec[u4_i].u4_memsize       = 0;
        mem_rec->memRec[u4_i].u4_alignment     = 0;
     }


}
IV_API_CALL_STATUS_T ittapi_memtab_alloc(mem_mgr *mem_rec,UWORD32 u4_num_mem_recs, iv_mem_rec_t *ps_mem_recs)
{
    UWORD8  *u1_ptr;
    UWORD32  u4_i;

    UWORD32     size = 0;
    IV_API_CALL_STATUS_T e_enc_status = IV_SUCCESS;

    if(u4_num_mem_recs >= MAX_MEM_REC)
    {
        ITTIAM_ERROR ("Error: Cannot Allocate More than %d MemTabs", MAX_MEM_REC);
        e_enc_status = IV_FAIL;
        return e_enc_status;
    }

    for(u4_i = 0; u4_i < u4_num_mem_recs; u4_i++)
    {
        UWORD32 alignment = ps_mem_recs[u4_i].u4_mem_alignment;

        u1_ptr = (UWORD8*)malloc(ps_mem_recs[u4_i].u4_mem_size+ alignment );


        if(NULL == u1_ptr)
        {
            ITTIAM_ERROR("ERROR in allocation for size %d for memtab number %d",ps_mem_recs[u4_i].u4_mem_size, u4_i);
            e_enc_status = IV_FAIL;
            return e_enc_status;
        }

        mem_rec->memRec[u4_i].u1_mem_ptr       = u1_ptr;
        mem_rec->memRec[u4_i].u4_memsize       = ps_mem_recs[u4_i].u4_mem_size + alignment;
        mem_rec->memRec[u4_i].u4_alignment     = alignment;

        size    += mem_rec->memRec[u4_i].u4_memsize;

        u1_ptr = (UWORD8*)(((WORD32)u1_ptr + alignment - 1) & ~(alignment -1));

        mem_rec->memRec[u4_i].u1_align_mem_ptr = u1_ptr;
        ps_mem_recs[u4_i].pv_base                 = u1_ptr;
        mem_rec->u4_mem_cnt++;
    }
return e_enc_status;
    //      assert(mem_rec->u4_mem_cnt == u4_num_mem_recs);
}

void ittapi_memtab_free(mem_mgr *mem_rec)
{
    UWORD32  u4_i;
    for(u4_i = 0; u4_i < mem_rec->u4_mem_cnt; u4_i++)
    {

        free(mem_rec->memRec[u4_i].u1_mem_ptr);

        mem_rec->memRec[u4_i].u1_mem_ptr       = NULL;
        mem_rec->memRec[u4_i].u1_align_mem_ptr = NULL;
        mem_rec->memRec[u4_i].u4_memsize       = 0;
        mem_rec->memRec[u4_i].u4_alignment     = 0;

    }
    free(mem_rec);
}
void initialize_variables(ih264_cxa8_ctl_set_config_ip_t *ps_set_config_ip_t)
{
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.e_cmd = IVE_CMD_VIDEO_CTL;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.e_sub_cmd = IVE_CMD_CTL_SETPARAMS;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.enc_config = IVE_CONFIG_USER_DEFINED;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_src_frame_rate  =  TARGET_FRAME_RATE;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_tgt_frame_rate = TARGET_FRAME_RATE;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_target_bitrate = TARGET_BIT_RATE;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_generate_header = 0;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_intra_frame_interval = 30;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.force_frame  = IV_NA_FRAME;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_num_b_frames = 0;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_air_refresh_period = 1000;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_random_access_period = 0;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u1_nal_size_insertion_flag = 0;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_set_i_qp               = 18;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_set_p_qp               = 18;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_set_b_qp               = 28;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_vbv_buffer_delay       = 1000;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_vbv_buf_size           = (1200 * 20000);
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_set_minQp_I_frame = 10;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_set_minQp_P_frame = 10;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_set_maxQp_I_frame = 51;
    ps_set_config_ip_t->s_ive_ctl_set_config_ip_t.u4_set_maxQp_P_frame = 51;
    ps_set_config_ip_t->u4_high_quality_me = 0;
    ps_set_config_ip_t->u4_enable_halfpel = 0;
    ps_set_config_ip_t->u4_enable_i4x4 = 0;
    ps_set_config_ip_t->u4_intrarefresh_en = 0;
    ps_set_config_ip_t->u4_enable_qpel  = 0;
    ps_set_config_ip_t->u4_reuse_buf_scale = 64;
    ps_set_config_ip_t->u1_reuse_buf_type = 1;
    ps_set_config_ip_t->u1_alt_ref_frame = 0;//pVidEnc->u4_alt_ref_frame;
    ps_set_config_ip_t->u1_fast_sad      = 1;
}

void __aeabi_assert(int expression)
{
    assert(expression);
}

 void codec_print(char *str )
{
	ITTIAM_LOG("CODEC DEBUG PRINT %s\n", str);
}

void codec_print1(char *str, int val, char *fname, int line)
{
	ITTIAM_LOG("CODEC DEBUG PRINT %s %d file %s line %d\n", str, val, fname, line);
}

#ifdef __cplusplus
//}
#endif /* __cplusplus */
