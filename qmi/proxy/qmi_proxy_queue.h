/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef QMI_PROXY_QUEUE_H_INCLUDED
#define QMI_PROXY_QUEUE_H_INCLUDED

struct qmi_proxy_queue;
typedef struct qmi_proxy_queue qmi_proxy_queue;

qmi_proxy_queue *qmi_proxy_queue_new
(
  void
);

void qmi_proxy_queue_delete
(
  qmi_proxy_queue *queue
);

int qmi_proxy_queue_push
(
  qmi_proxy_queue *queue,
  void *value
);

void *qmi_proxy_queue_pop
(
  qmi_proxy_queue *queue
);

int qmi_proxy_queue_is_empty
(
  qmi_proxy_queue *queue
);
#endif
