/*--------------------------------------------------------------------------
Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
--------------------------------------------------------------------------*/
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
struct wma_ipc_info *omx_wma_thread_create(
message_func cb,
void* client_data,
char* th_name)
{
    int r;
    int fds[2];
    struct wma_ipc_info *wma_info;

    wma_info = calloc(1, sizeof(struct wma_ipc_info));
    if (!wma_info) return 0;

    wma_info->client_data = client_data;
    wma_info->process_msg_cb = cb;
    strlcpy(wma_info->thread_name,th_name, sizeof (wma_info->thread_name));

    if (pipe(fds)) {
        DEBUG_PRINT_ERROR("\n%s: pipe creation failed\n", __FUNCTION__);
        goto fail_pipe;
    }

    wma_info->pipe_in = fds[0];
    wma_info->pipe_out = fds[1];


    r = pthread_create(&wma_info->thr, 0, omx_wma_msg, wma_info);
    if (r < 0) goto fail_thread;

    DEBUG_DETAIL("Created thread for %s \n", wma_info->thread_name);
    return wma_info;


    fail_thread:
    close(wma_info->pipe_in);
    close(wma_info->pipe_out);

    fail_pipe:
    free(wma_info);

    return 0;
}

/**
@brief This function starts the event thread

@param cb pointer to callback function from the client
@param client_data reference client wants to get back
through callback
@return handle to msging thread
*/
struct wma_ipc_info *omx_wma_event_thread_create(
message_func cb,
void* client_data,
char* th_name)
{
    int r;
    int fds[2];
    struct wma_ipc_info *wma_info;

    wma_info = calloc(1, sizeof(struct wma_ipc_info));
    if (!wma_info) return 0;

    wma_info->client_data = client_data;
    wma_info->process_msg_cb = cb;
    strlcpy(wma_info->thread_name,th_name, sizeof (wma_info->thread_name));

    if (pipe(fds)) {
        DEBUG_PRINT_ERROR("\n%s: pipe creation failed\n", __FUNCTION__);
        goto fail_pipe;
    }

    wma_info->pipe_in = fds[0];
    wma_info->pipe_out = fds[1];


    r = pthread_create(&wma_info->thr, 0, omx_wma_events, wma_info);
    if (r < 0) goto fail_thread;

    DEBUG_DETAIL("Created thread for %s \n", wma_info->thread_name);
    return wma_info;


    fail_thread:
    close(wma_info->pipe_in);
    close(wma_info->pipe_out);

    fail_pipe:
    free(wma_info);

    return 0;
}

/**
@brief This function processes posted messages

Once thread is being spawned, this function is run to
start processing commands posted by client

@param info pointer to context

*/
void *omx_wma_msg(void *info)
{
    struct wma_ipc_info *wma_info = (struct wma_ipc_info*)info;
    unsigned char id;
    ssize_t n;

    DEBUG_DETAIL("\n%s: message thread start\n", __FUNCTION__);
    while (!wma_info->dead) {
        n = read(wma_info->pipe_in, &id, 1);
        if (n == 0)
        {
	   DEBUG_PRINT("%s: message pipe read zero\n", __FUNCTION__);
           break;
        }
        if (n == 1) {
            DEBUG_DETAIL("\n%s-->pipe_in=%d pipe_out=%d\n",
            wma_info->thread_name,
            wma_info->pipe_in,
            wma_info->pipe_out);

            wma_info->process_msg_cb(wma_info->client_data, id);

        }
        if ((n < 0) && (errno != EINTR))
        {
            DEBUG_PRINT_ERROR("%s:message pipe read n=%lu err=0x%8x\n",wma_info->thread_name,n,errno);
            break;
        }
    }
    DEBUG_DETAIL("%s: message thread stop\n", wma_info->thread_name);

    return 0;
}

void *omx_wma_events(void *info)
{
    struct wma_ipc_info *wma_info = (struct wma_ipc_info*)info;
    unsigned char id = 0;

    DEBUG_DETAIL("%s: message thread start\n", wma_info->thread_name);
    wma_info->process_msg_cb(wma_info->client_data, id);
    DEBUG_PRINT("%s: message thread stop\n", wma_info->thread_name);
    return 0;
}


void omx_wma_thread_stop(struct wma_ipc_info *wma_info)
{
    int rc = 0;
    DEBUG_DETAIL("%s: message thread close\n", wma_info->thread_name);
    close(wma_info->pipe_in);
    close(wma_info->pipe_out);
    rc = pthread_join(wma_info->thr,NULL);
    wma_info->pipe_out = -1;
    wma_info->pipe_in = -1;
    free(wma_info);
}

void omx_wma_post_msg(struct wma_ipc_info *wma_info, unsigned int id) {
    DEBUG_DETAIL("\n%s id=%d\n", __FUNCTION__,id);
    write(wma_info->pipe_out, &id, 1);
}
