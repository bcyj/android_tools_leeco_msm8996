/*============================================================================

   Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CAM_FIFO_H__
#define __CAM_FIFO_H__

#include "camera.h"

static struct fifo_node* dequeue(struct fifo_queue* q)
{
  struct fifo_node *tmp = q->front;

  if (!tmp)
  {
      return 0;
  }

  if (q->front == q->back)
  {
      q->front = q->back = 0;
  }
  else if (q->front)
  {
      q->front = q->front->next;
  }
  tmp->next = 0;
  q->num_of_frames -=1;
  return tmp;
}

static void enqueue(struct fifo_queue* q, struct fifo_node*p)
{
  if (q->back)
  {
      q->back->next = p;
      q->back = p;
  }
  else
  {
      q->front = p;
      q->back = p;   
  }
  q->num_of_frames +=1;
  return;
}
#endif //__CAM_FIFO_H__
