/*
 * Copyright (C) 2008 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define LOG_NIDEBUG 0
#define LOG_TAG "mm-camera stacktrace"
#ifdef _ANDROID_
#include <utils/Log.h>
#else
#define LOGE(fmt, args...) fprintf(stderr, fmt, ##args)
#endif

#include <pthread.h>
pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;

extern int mmcamera_stacktrace(intptr_t* addrs, size_t max_entries);

#define MAX_BACKTRACE_DEPTH 15

struct hdr_t {
    struct hdr_t *prev;
    struct hdr_t *next;
    intptr_t bt[MAX_BACKTRACE_DEPTH];
    int bt_depth;
};
typedef struct hdr_t hdr_t;

static unsigned num;
static hdr_t *last;

static inline void add(hdr_t *hdr)
{
    hdr->prev = 0;
    hdr->next = last;
    if (last) last->prev = hdr;
    last = hdr;
    num++;
}

void *__override_malloc(size_t size)
{
    pthread_mutex_lock(&memory_mutex);
    hdr_t *hdr = malloc(sizeof(hdr_t)+size);
    if (hdr) {
        hdr->bt_depth = mmcamera_stacktrace(
                            hdr->bt, MAX_BACKTRACE_DEPTH);
        add(hdr);
        pthread_mutex_unlock(&memory_mutex);
        return hdr+1;
    }
    pthread_mutex_unlock(&memory_mutex);
    return NULL;
}

void __override_free(hdr_t *hdr)
{
    pthread_mutex_lock(&memory_mutex);
    if (hdr) {
        hdr--;

        if (hdr->prev)
            hdr->prev->next = hdr->next;
        else
            last = hdr->next;
        if (hdr->next)
            hdr->next->prev = hdr->prev;
        free(hdr);
        num--;
    }
    pthread_mutex_unlock(&memory_mutex);
}

void *__override_calloc(size_t nmemb, size_t size)
{
    pthread_mutex_lock(&memory_mutex);
    hdr_t *hdr = calloc(1, sizeof(hdr_t)+nmemb*size);
    if (hdr) {
        hdr->bt_depth = mmcamera_stacktrace(
                            hdr->bt, MAX_BACKTRACE_DEPTH);
        add(hdr);
        pthread_mutex_unlock(&memory_mutex);
        return hdr + 1;
    }
    pthread_mutex_unlock(&memory_mutex);
    return NULL;
}

void *__override_realloc(void *ptr, size_t size)
{
    pthread_mutex_lock(&memory_mutex);
    hdr_t *hdr = realloc(ptr, sizeof(hdr_t)+size);
    if (hdr) {
        hdr->bt_depth = mmcamera_stacktrace(
                            hdr->bt, MAX_BACKTRACE_DEPTH);
        add(hdr);
        pthread_mutex_unlock(&memory_mutex);
        return hdr + 1;
    }
    pthread_mutex_unlock(&memory_mutex);
    return NULL;
}

static void free_leaked_memory(void) __attribute__((destructor));

static void free_leaked_memory(void)
{
    pthread_mutex_lock(&memory_mutex);
    hdr_t *del; int cnt;
    while(last) {
        del = last;
        last = last->next;
        LOGE("+++ DELETING LEAKED MEMORY AT %p (%d REMAINING)\n",
                del + 1, num);
        for (cnt = 0; cnt < del->bt_depth; cnt++)
            LOGE("    %2d %p", del->bt_depth - cnt, (void *)del->bt[cnt]);
        free(del);
        num--;
    }
    pthread_mutex_unlock(&memory_mutex);
}
