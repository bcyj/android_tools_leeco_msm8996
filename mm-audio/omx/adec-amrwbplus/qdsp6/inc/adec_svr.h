/*============================================================================
  @file adec_svr.h
  This file contains helper functions declarations required by OpenMAX Audio AMR-WB+ component.

  Copyright (c) 2010, 2012, 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/
#ifndef ADEC_SVR_H
#define ADEC_SVR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sched.h>

#ifdef _ANDROID_
#define LOG_TAG "QC_AMRWBPLUSDEC"
#endif
#include "qc_omx_msg.h"

#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
#define DEBUG_PRINT
#endif

/**
@brief audio decoder command server structure

This structure maintains the command
server context

*/
    typedef void (*message_func)(void* client_data, unsigned char id);

    struct amrwbplus_ipc_info
    {
        pthread_t    thr;
        int          pipe_in;
        int          pipe_out;
        int          dead;
        message_func process_msg_cb;
        void         *client_data;
        char         thread_name[128];
    };

    /**
@brief This function starts command server

@param cb pointer to callback function from the client
@param client_data reference client wants to get back
through callback
@return handle to command server
*/
    struct amrwbplus_ipc_info *omx_amrwbplus_thread_create(message_func cb,
    void* client_data,
    char *th_name);

    struct amrwbplus_ipc_info *omx_amrwbplus_event_thread_create(message_func cb,
    void* client_data,
    char *th_name);

    void *omx_amrwbplus_msg(void *info);

    void *omx_amrwbplus_events(void *info);

    /**
@brief This function stop command server

@param svr handle to command server
@return none
*/
    void omx_amrwbplus_thread_stop(struct amrwbplus_ipc_info *amrwbplus_ipc);


    /**
@brief This function post message in the command server

@param svr handle to command server
@return none
*/
    void omx_amrwbplus_post_msg(struct amrwbplus_ipc_info *amrwbplus_ipc, unsigned int id);

    void* omx_amrwbplus_comp_timer_handler(void *);
#ifdef __cplusplus
}
#endif

#endif /* ADEC_SVR */
