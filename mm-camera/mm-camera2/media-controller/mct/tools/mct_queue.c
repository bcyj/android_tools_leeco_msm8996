/* mct_queue.c
 *
 * This file contains the helper methods and implementation for managing
 * queues.
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mtype.h"
#include "mct_queue.h"

void mct_queue_init(mct_queue_t *q)
{
  if (!q)
    return;

  q->head = q->tail = NULL;
  q->length = 0;
}

void mct_queue_free(mct_queue_t *q)
{
  if (!q)
    return;

  mct_list_free_list(q->head);
  free(q);
  q = NULL;
}

void mct_queue_free_all(mct_queue_t *q, mct_queue_traverse_func traverse)
{
  if (!q)
    return;

  mct_list_free_all(q->head, traverse);
  free(q);
  q = NULL;
}

void mct_queue_traverse(mct_queue_t *q, mct_queue_traverse_func traverse,
  void *data)
{
  mct_list_t *temp_list;

  if (!q || !traverse)
    return;

  mct_list_traverse(q->head, traverse, data);
}

/**
 *
 **/
void mct_queue_push_tail(mct_queue_t *q, void *data)
{
  if (!q)
    return;

  q->tail = mct_list_append(q->tail, data, NULL, NULL);

  if (!q->tail)
    return;

  if (q->tail->next)
    q->tail = q->tail->next[0];
  else
    q->head = q->tail;

  q->length++;
}

void *mct_queue_pop_head(mct_queue_t *q)
{
  if (!q)
    return NULL;

  if (q->head) {
    mct_list_t *node = q->head;
    void    *data = node->data;

    if (node->next) {
      q->head = node->next[0];
      free(node->next);
      node->next = NULL;
    } else {
      q->head = NULL;
    }
    if (q->head) {
      q->head->prev = NULL;
    } else {
      q->tail = NULL;
    }

    free(node);
    q->length--;

    return data;
  }

  return NULL;
}

/*  mct_queue_look_at_head
 *
 *  Description:
 *  Returns the data from the head of the queue.
 *  Does not add/remove anything in queue.
 **/
void *mct_queue_look_at_head(mct_queue_t *q)
{
  if (!q)
    return NULL;
  if (q->head)
    return q->head->data;
  else
    return NULL;
}

void mct_queue_flush(mct_queue_t *q, mct_queue_traverse_func traverse)
{
  if (!q)
    return;

  mct_list_free_all(q->head, traverse);
  mct_queue_init(q);
  return;
}
