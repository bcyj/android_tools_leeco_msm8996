/*============================================================================
  @file adec_svr.c
  This file contains helper functions required by OpenMAX Audio AMR-WB+ component.

  Copyright (c) 2010, 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <adec_svr.h>
#include <string.h>
#include <errno.h>

/**
@brief This function starts command server

@param cb pointer to callback function from the client
@param client_data reference client wants to get back
through callback
@return handle to msging thread
*/
struct amrwbplus_ipc_info *omx_amrwbplus_thread_create( message_func cb,
                                                        void* client_data,
                                                        char* th_name)
{
    int r;
    int fds[2];
    struct amrwbplus_ipc_info *amrwbplus_info;

    amrwbplus_info = calloc(1, sizeof(struct amrwbplus_ipc_info));
    if (!amrwbplus_info) return 0;

    amrwbplus_info->client_data = client_data;
    amrwbplus_info->process_msg_cb = cb;
    strlcpy(amrwbplus_info->thread_name,th_name, sizeof (amrwbplus_info->thread_name));

    if (pipe(fds)) {
        DEBUG_PRINT_ERROR("\n%s: pipe creation failed\n", __FUNCTION__);
        goto fail_pipe;
    }

    amrwbplus_info->pipe_in  = fds[0];
    amrwbplus_info->pipe_out = fds[1];


    r = pthread_create(&amrwbplus_info->thr, 0, omx_amrwbplus_msg, amrwbplus_info);
    if (r < 0) goto fail_thread;

    DEBUG_DETAIL("Created thread for %s \n", amrwbplus_info->thread_name);
    return amrwbplus_info;


    fail_thread:
    close(amrwbplus_info->pipe_in);
    close(amrwbplus_info->pipe_out);

    fail_pipe:
    free(amrwbplus_info);

    return 0;
}

/**
@brief This function starts the event thread

@param cb pointer to callback function from the client
@param client_data reference client wants to get back
through callback
@return handle to msging thread
*/
struct amrwbplus_ipc_info *omx_amrwbplus_event_thread_create( message_func cb,
                                                              void* client_data,
                                                              char* th_name)
{
    int r;
    int fds[2];
    struct amrwbplus_ipc_info *amrwbplus_info;

    amrwbplus_info = calloc(1, sizeof(struct amrwbplus_ipc_info));
    if (!amrwbplus_info) return 0;

    amrwbplus_info->client_data = client_data;
    amrwbplus_info->process_msg_cb = cb;
    strlcpy(amrwbplus_info->thread_name,th_name, sizeof (amrwbplus_info->thread_name));

    if (pipe(fds)) {
        DEBUG_PRINT_ERROR("\n%s: pipe creation failed\n", __FUNCTION__);
        goto fail_pipe;
    }

    amrwbplus_info->pipe_in  = fds[0];
    amrwbplus_info->pipe_out = fds[1];


    r = pthread_create(&amrwbplus_info->thr, 0, omx_amrwbplus_events, amrwbplus_info);
    if (r < 0) goto fail_thread;

    DEBUG_DETAIL("Created thread for %s \n", amrwbplus_info->thread_name);
    return amrwbplus_info;


    fail_thread:
    close(amrwbplus_info->pipe_in);
    close(amrwbplus_info->pipe_out);

    fail_pipe:
    free(amrwbplus_info);

    return 0;
}

/**
@brief This function processes posted messages

Once thread is being spawned, this function is run to
start processing commands posted by client

@param info pointer to context

*/
void *omx_amrwbplus_msg(void *info)
{
    struct amrwbplus_ipc_info *amrwbplus_info = (struct amrwbplus_ipc_info*)info;
    unsigned char id;
    ssize_t n;

    DEBUG_DETAIL("\n%s: message thread start\n", __FUNCTION__);

    if(amrwbplus_info) {
        while (!amrwbplus_info->dead) {
            n = read(amrwbplus_info->pipe_in, &id, 1);
            if (n == 0) {
                DEBUG_PRINT("%s: message pipe read zero\n", __FUNCTION__);
                break;
            }
            if (n == 1) {
                DEBUG_DETAIL("\n%s-->pipe_in=%d pipe_out=%d\n",
                amrwbplus_info->thread_name,
                amrwbplus_info->pipe_in,
                amrwbplus_info->pipe_out);
                amrwbplus_info->process_msg_cb(amrwbplus_info->client_data, id);
            }
            if ((n < 0) && (errno != EINTR)) {
                DEBUG_PRINT_ERROR("%s:message pipe read n=%lu err=0x%8x\n",amrwbplus_info->thread_name,n,errno);
                break;
            }
        }
    }
    if(amrwbplus_info) {
        DEBUG_DETAIL("%s: message thread stop\n", amrwbplus_info->thread_name);
    }

    return 0;
}

void *omx_amrwbplus_events(void *info)
{
    struct amrwbplus_ipc_info *amrwbplus_info = (struct amrwbplus_ipc_info*)info;
    unsigned char id = 0;
    if(amrwbplus_info) {
        DEBUG_DETAIL("%s: message thread start\n", amrwbplus_info->thread_name);
        amrwbplus_info->process_msg_cb(amrwbplus_info->client_data, id);
        DEBUG_PRINT("%s: message thread stop\n", amrwbplus_info->thread_name);
    }
    return 0;
}


void omx_amrwbplus_thread_stop(struct amrwbplus_ipc_info *amrwbplus_info)
{
    int rc = 0;
    if(amrwbplus_info) {
        DEBUG_DETAIL("%s: message thread close\n", amrwbplus_info->thread_name);
        close(amrwbplus_info->pipe_in);
        close(amrwbplus_info->pipe_out);
        rc = pthread_join(amrwbplus_info->thr,NULL);
        amrwbplus_info->pipe_out = -1;
        amrwbplus_info->pipe_in = -1;
        free(amrwbplus_info);
    }
}

void omx_amrwbplus_post_msg(struct amrwbplus_ipc_info *amrwbplus_info, unsigned int id) {
        DEBUG_DETAIL("\n%s id=%d\n", __FUNCTION__,id);
        if(amrwbplus_info) {
            write(amrwbplus_info->pipe_out, &id, 1);
        }
}
