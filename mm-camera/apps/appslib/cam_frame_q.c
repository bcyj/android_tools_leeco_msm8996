/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CAM_FRAME_Q_H__
#define __CAM_FRAME_Q_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "cam_fifo.h"
#include "camera_dbg.h"

#ifdef ENABLE_Q_LOGGING
  #undef CDBG
  #define CDBG LOGE
#else
  #undef CDBG
  #define CDBG(fmt, args...) do{}while(0)
#endif

/*===========================================================================
   FUNCTION get_frame

   DESCRIPTION this function get a frame from the head
===========================================================================*/
struct msm_frame* get_frame(struct fifo_queue* queue) 
{
  struct msm_frame *p = 0;
  CDBG("get_frame: %s in n = %d\n", queue->name, queue->num_of_frames);
  pthread_mutex_lock(&(queue->mut));
  CDBG("get_frame: front = %p\n", (uint8_t *)queue->front);
  if (queue->front) {
    struct fifo_node *node = dequeue (queue);
    CDBG("get_frame: freeing the node = %p\n", (struct msm_frame *)node->f);
    if (node) {
      p = (struct msm_frame *)node->f;
      free (node);
    }
  }
  pthread_mutex_unlock(&(queue->mut));
  CDBG("get_frame: out n = %d\n", queue->num_of_frames);
  return p;
}

/*===========================================================================
   FUNCTION add_frame

   DESCRIPTION this function add a frame to the tail
===========================================================================*/
int8_t add_frame(struct fifo_queue* queue, struct msm_frame *p)
{
 if (!p) {
    CDBG("add_frame: null passed in\n");
    return FALSE;
  }
  CDBG("add_frame: %s in address = 0x%x \n", queue->name, (uint32_t)p->buffer);
  CDBG("add_frame: in n = %d\n", queue->num_of_frames);

  pthread_mutex_lock(&(queue->mut));

  struct fifo_node *node = malloc (sizeof (struct fifo_node));

  if (node) {
    node->f = p;
    node->next = 0;
    enqueue (queue, node);
  } else {
    pthread_mutex_unlock(&(queue->mut));
    CDBG("add_frame: out of memory\n");
    return FALSE;
  }

  pthread_mutex_unlock(&(queue->mut));
  CDBG("add_frame: out n = %d\n", queue->num_of_frames);
  return TRUE;
}
   
/*===========================================================================
   FUNCTION flush_queue

   DESCRIPTION this function empties the queue
===========================================================================*/
inline void flush_queue (struct fifo_queue* queue)
{
  CDBG("flush_queue: %s in n = %d\n", queue->name, queue->num_of_frames);
  pthread_mutex_lock(&(queue->mut));

  while (queue->front) {
    struct fifo_node *node = dequeue (queue);
    if (node) {
      CDBG("flush_queue: freing the node = %x\n",
        (uint32_t)((struct msm_frame*)node->f)->buffer);
      free (node);
    }
  }
  pthread_mutex_unlock(&(queue->mut));
  CDBG("flush_queue: out n = %d\n", queue->num_of_frames);
}

/*===========================================================================
   FUNCTION flush_and_destroy_queue

   DESCRIPTION this function empties the queue and frees the contents
===========================================================================*/
inline void flush_and_destroy_queue (struct fifo_queue* queue)
{
  CDBG("flush_queue: %s in n = %d\n", queue->name, queue->num_of_frames);
  pthread_mutex_lock(&(queue->mut));

  while (queue->front) {
    struct fifo_node *node = dequeue (queue);
    if (node) {
      CDBG("flush_queue: freing the node = %x\n",
        (uint32_t)((struct msm_frame*)node->f)->buffer);
      if (node->f)
        free(node->f);
      free (node);
    }
  }
  pthread_mutex_unlock(&(queue->mut));
  CDBG("flush_queue: out n = %d\n", queue->num_of_frames);
}

/*===========================================================================
  FUNCTION wait_queue

  DESCRIPTION this function empties the queue
===========================================================================*/
void wait_queue (struct fifo_queue* queue)
{
  CDBG("wait_queue in %s %d", queue->name, queue->num_of_frames);
  pthread_mutex_lock(&(queue->mut));
  if ((queue->num_of_frames) <=0){
    pthread_cond_wait(&(queue->wait), &(queue->mut));
  }
  pthread_mutex_unlock(&(queue->mut));
  CDBG("wait_queue out");
}

/*===========================================================================
FUNCTION signal_queue

DESCRIPTION this function empties the queue
===========================================================================*/
void signal_queue (struct fifo_queue* queue)
{
  CDBG("signal_queue in %s", queue->name);
  pthread_mutex_lock(&(queue->mut));
  pthread_cond_signal(&(queue->wait));
  pthread_mutex_unlock(&(queue->mut));
  CDBG("signal_queue out");
}

/********  ITERATOR APIs *********/
/*===========================================================================
FUNCTION begin

DESCRIPTION this function returns the first frame of queue
===========================================================================*/
struct msm_frame* begin (void** iterator, struct fifo_queue* queue)
{
  struct fifo_node *node = queue->front;
  struct msm_frame *p = NULL;
  *iterator = (void *)node;
  if (node == NULL)
    return p;
  p = (struct msm_frame *)node->f;
  return p;
}

/*===========================================================================
FUNCTION next

DESCRIPTION this function returns the next frame of queue
===========================================================================*/
struct msm_frame* next (void** iterator)
{
  struct fifo_node *node = (struct fifo_node *)*iterator;
  struct msm_frame *p = NULL;
  if (node == NULL)
    return p;
  node = node->next;
  if (node == NULL)
    return p;
  p = (struct msm_frame *)node->f;
  *iterator = (void *)node;
  return p;
}

/*===========================================================================
FUNCTION end

DESCRIPTION this function returns the end frame of queue
===========================================================================*/
struct msm_frame* end (struct fifo_queue* queue)
{
  struct fifo_node *node = queue->back;
  struct msm_frame *p = NULL;
  if (node == NULL)
    return p;
  p = (struct msm_frame *)node->f;
  return p;
}
#endif /* __CAM_FRAME_Q_H__ */
