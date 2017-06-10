/*--------------------------------------------------------------------------
Copyright (c) 2011 Qualcomm Technologies, Inc.
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
struct aac_ipc_info *omx_aac_thread_create(
                             message_func cb,
                             void* client_data,
                             char* th_name)
{
    int r;
    int fds[2];
    struct aac_ipc_info *aac_info;
    size_t len = (strlen(th_name) < 128) ? strlen(th_name) : 128;

    aac_info = calloc(1, sizeof(struct aac_ipc_info));
    if (!aac_info) return 0;

    aac_info->client_data = client_data;
    aac_info->process_msg_cb = cb;
    strlcpy(aac_info->thread_name,th_name, len);

    if (pipe(fds)) {
        DEBUG_PRINT_ERROR("\n%s: pipe creation failed\n", __FUNCTION__);
        goto fail_pipe;
    }

    aac_info->pipe_in = fds[0];
    aac_info->pipe_out = fds[1];


    r = pthread_create(&aac_info->thr, 0, omx_aac_msg, aac_info);
    if (r < 0) goto fail_thread;

    DEBUG_DETAIL("Created thread for %s \n", aac_info->thread_name);
    return aac_info;


    fail_thread:
    close(aac_info->pipe_in);
    close(aac_info->pipe_out);

    fail_pipe:
    free(aac_info);

    return 0;
}

/**
@brief This function starts the event thread

@param cb pointer to callback function from the client
@param client_data reference client wants to get back
through callback
@return handle to msging thread
*/
struct aac_ipc_info *omx_aac_event_thread_create(
message_func cb,
void* client_data,
char* th_name)
{
    int r;
    int fds[2];
    struct aac_ipc_info *aac_info;
    size_t len = (strlen(th_name) < 128) ? strlen(th_name) : 128;

    aac_info = calloc(1, sizeof(struct aac_ipc_info));
    if (!aac_info) return 0;

    aac_info->client_data = client_data;
    aac_info->process_msg_cb = cb;
    strlcpy(aac_info->thread_name,th_name, len);

    if (pipe(fds)) {
        DEBUG_PRINT_ERROR("\n%s: pipe creation failed\n", __FUNCTION__);
        goto fail_pipe;
    }

    aac_info->pipe_in = fds[0];
    aac_info->pipe_out = fds[1];


    r = pthread_create(&aac_info->thr, 0, omx_aac_events, aac_info);
    if (r < 0) goto fail_thread;

    DEBUG_DETAIL("Created thread for %s \n", aac_info->thread_name);
    return aac_info;


    fail_thread:
    close(aac_info->pipe_in);
    close(aac_info->pipe_out);

    fail_pipe:
    free(aac_info);

    return 0;
}

/**
@brief This function processes posted messages

Once thread is being spawned, this function is run to
start processing commands posted by client

@param info pointer to context

*/
void *omx_aac_msg(void *info)
{
    struct aac_ipc_info *aac_info = (struct aac_ipc_info*)info;
    unsigned char id;
    ssize_t n;

    DEBUG_DETAIL("\n%s: message thread start\n", __FUNCTION__);
    while (!aac_info->dead) {
        n = read(aac_info->pipe_in, &id, 1);
        if (n == 0)
        {
       DEBUG_PRINT("%s: message pipe read zero\n", __FUNCTION__);
           break;
        }
        if (n == 1) {
            DEBUG_DETAIL("\n%s-->pipe_in=%d pipe_out=%d\n",
            aac_info->thread_name,
            aac_info->pipe_in,
            aac_info->pipe_out);

            aac_info->process_msg_cb(aac_info->client_data, id);

        }
        if ((n < 0) && (errno != EINTR))
        {
            DEBUG_PRINT_ERROR("%s:message pipe read n=%lu err=0x%8x\n",aac_info->thread_name,n,errno);
            break;
        }
    }
    DEBUG_DETAIL("%s: message thread stop\n", aac_info->thread_name);

    return 0;
}

void *omx_aac_events(void *info)
{
    struct aac_ipc_info *aac_info = (struct aac_ipc_info*)info;
    unsigned char id = 0;

    DEBUG_DETAIL("%s: message thread start\n", aac_info->thread_name);
    aac_info->process_msg_cb(aac_info->client_data, id);
    DEBUG_PRINT("%s: message thread stop\n", aac_info->thread_name);
    return 0;
}


void omx_aac_thread_stop(struct aac_ipc_info *aac_info)
{
    int rc = 0;
    DEBUG_DETAIL("%s: message thread close\n", aac_info->thread_name);
    close(aac_info->pipe_in);
    close(aac_info->pipe_out);
    rc = pthread_join(aac_info->thr,NULL);
    aac_info->pipe_out = -1;
    aac_info->pipe_in = -1;
    free(aac_info);
}

void omx_aac_post_msg(struct aac_ipc_info *aac_info, unsigned char id) {
    DEBUG_DETAIL("\n%s id=%d\n", __FUNCTION__,id);
    write(aac_info->pipe_out, &id, 1);
}
