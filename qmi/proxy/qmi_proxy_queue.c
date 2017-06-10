/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdlib.h>
#include <qmi.h>
#include "qmi_proxy_queue.h"

#ifdef TEST_QMI_PROXY_SM
#include <stdio.h>
#include <stdlib.h>
#include <qmi.h>
#define QMI_PROXY_DEBUG_MSG printf
#define QMI_PROXY_ERR_MSG printf
#define TRUE 1
#define FALSE 0
#endif

struct qmi_proxy_queue_node;
typedef struct qmi_proxy_queue_node qmi_proxy_queue_node;
//typedef struct qmi_proxy_queue qmi_proxy_queue;

struct qmi_proxy_queue_node
{
  void *user_data;
  qmi_proxy_queue_node *next;
  qmi_proxy_queue_node *prev;
};

struct qmi_proxy_queue
{
  qmi_proxy_queue_node *head;
  qmi_proxy_queue_node *tail;
};

/* Function prototypes */
static inline int qmi_proxy_queue_head_tail_consistent
(
  qmi_proxy_queue *queue
);

/* Function definitions */
qmi_proxy_queue *qmi_proxy_queue_new
(
  void
)
{
  qmi_proxy_queue *queue;

  queue = calloc(1, sizeof(*queue));

  if (!queue) {
    QMI_PROXY_ERR_MSG("%s: Allocation failed\n", __FUNCTION__);
  }
  return queue;
}

void qmi_proxy_queue_delete
(
  qmi_proxy_queue *queue
)
{
  free(queue);
}

static inline int qmi_proxy_queue_head_tail_consistent
(
  qmi_proxy_queue *queue
)
{
  return (queue) &&
          ( (!queue->head && !queue->tail) ||
                  (queue->head && queue->tail &&
                          (!queue->head->next) &&
                          (!queue->tail->prev)
                  )
          );
}

int qmi_proxy_queue_push
(
  qmi_proxy_queue *queue,
  void *value
)
{
  int rc = QMI_NO_ERR;
  qmi_proxy_queue_node *new_node;

  new_node = calloc(1, sizeof(*new_node));

  if (new_node)
  {
    new_node->user_data = value;

    if ( qmi_proxy_queue_head_tail_consistent(queue) )
    {
      if ( queue->tail)
      {
        new_node->next = queue->tail;
        new_node->prev = NULL;
        queue->tail->prev = new_node;
        queue->tail = new_node;
      }
      else
      {
        queue->tail = new_node;
        queue->head = new_node;
      }
    }
    else
    {
      QMI_PROXY_ERR_MSG("%s: head and tail inconsistent", __FUNCTION__);
      rc = QMI_INTERNAL_ERR;
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Allocation failed", __FUNCTION__);
    rc = QMI_INTERNAL_ERR;
  }
  return rc;
}

void *qmi_proxy_queue_pop
(
  qmi_proxy_queue *queue
)
{
  void *ret = NULL;
  qmi_proxy_queue_node *node;

  if ( qmi_proxy_queue_head_tail_consistent(queue) )
  {
    node = queue->head;
    if (node)
    {
      queue->head = queue->head->prev;
      if (queue->head)
      {
        queue->head->next = NULL;
      }
      else
      {
        queue->tail = NULL;
      }
      ret = node->user_data;
      free(node);
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s  head and tail inconsistent", __FUNCTION__);
  }

  return ret;
}

int qmi_proxy_queue_is_empty
(
  qmi_proxy_queue *queue
)
{
  int ret = TRUE;
  if ( qmi_proxy_queue_head_tail_consistent(queue) && (queue->tail))
  {
	  ret = FALSE;
  }
  return ret;
}
