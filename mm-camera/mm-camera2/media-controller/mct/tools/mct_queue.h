/* mct_queue.h
 *  														 .
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_QUEUE_H__
#define __MCT_QUEUE_H__

#include "mct_list.h"

typedef void (*mct_queue_free_func)     (void *data, void *user_data);

typedef mct_list_traverse_func mct_queue_traverse_func;

typedef struct _mct_queue {
  mct_list_t   *head;
  mct_list_t   *tail;
  uint32_t length;
} mct_queue_t;

#define MCT_QUEUE_IS_EMPTY(q) \
  (((mct_queue_t *)(q)->head == NULL) ? TRUE : FALSE)

#define mct_queue_new malloc(sizeof(mct_queue_t));

#define MCT_QUEUE_FIND_CUSTOM(q, data, f) \
     mct_list_find_custom((mct_queue_t*)(q)->head, (void *)d, \
     (mct_list_find_func)f)



void  mct_queue_init      (mct_queue_t *q);
void  mct_queue_free      (mct_queue_t *q);
void  mct_queue_free_all  (mct_queue_t *q, mct_queue_traverse_func traverse);
void  mct_queue_traverse  (mct_queue_t *q, mct_queue_traverse_func traverse,
        void *data);
void  mct_queue_flush     (mct_queue_t *q, mct_queue_traverse_func traverse);
void  mct_queue_push_tail (mct_queue_t *q, void *data);
void* mct_queue_pop_head  (mct_queue_t *q);
void* mct_queue_look_at_head (mct_queue_t *q);

#endif /* __MCT_QUEUE_H__ */
