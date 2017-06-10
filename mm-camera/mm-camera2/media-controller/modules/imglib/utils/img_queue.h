/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __QIMG_QUEUE_H__
#define __QIMG_QUEUE_H__

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

typedef struct _img_node {
  void *data;
  struct _img_node *next;
} img_node_t;

typedef struct {
  img_node_t *p_front;
  img_node_t *p_rear;
  int count;
  pthread_mutex_t mutex;
  char *name;
  pthread_cond_t cond;
} img_queue_t;

typedef int (*q_wait_cond_func)(void *p_userdata);
typedef int (*q_node_process_func)(void *p_nodedata, void *p_userdata);

/* Functions */
void img_q_init(img_queue_t *p_q, char *name);
void img_q_deinit(img_queue_t *p_q);
int img_q_enqueue(img_queue_t *p_q, void *data);
void *img_q_dequeue(img_queue_t *p_q);
void img_q_flush(img_queue_t *p_q);
int img_q_count(img_queue_t *p_q);
void *img_q_wait(img_queue_t *p_q, q_wait_cond_func wait_cond,
  void *p_userdata);
void img_q_signal(img_queue_t *p_q);
int img_q_traverse(img_queue_t *p_q, q_node_process_func func,
  void *p_userdata);
void *img_q_get_last_entry(img_queue_t *p_q, q_node_process_func func,
  void *p_userdata);
void img_q_wait_for_signal(img_queue_t *p_q, q_wait_cond_func wait_cond,
  void *p_userdata);
void img_q_flush_and_destroy(img_queue_t *p_q);

#endif //__QIMG_QUEUE_H__
