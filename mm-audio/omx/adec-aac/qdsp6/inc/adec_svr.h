/*--------------------------------------------------------------------------
Copyright (c) 2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
--------------------------------------------------------------------------*/
#ifndef ADEC_SVR_H
#define ADEC_SVR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sched.h>

#ifdef _ANDROID_
#define LOG_TAG "QC_AACDEC"
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

    struct aac_ipc_info
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
    struct aac_ipc_info *omx_aac_thread_create(message_func cb,
    void* client_data,
    char *th_name);

    struct aac_ipc_info *omx_aac_event_thread_create(message_func cb,
    void* client_data,
    char *th_name);

    void *omx_aac_msg(void *info);

    void *omx_aac_events(void *info);

    /**
@brief This function stop command server

@param svr handle to command server
@return none
*/
    void omx_aac_thread_stop(struct aac_ipc_info *aac_ipc);


    /**
@brief This function post message in the command server

@param svr handle to command server
@return none
*/
    void omx_aac_post_msg(struct aac_ipc_info *aac_ipc, unsigned char id);

    void* omx_aac_comp_timer_handler(void *);
#ifdef __cplusplus
}
#endif

#endif /* ADEC_SVR */
