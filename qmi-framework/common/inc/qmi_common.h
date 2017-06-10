/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2011, 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef QMI_COMMON_H
#define QMI_COMMON_H

#include <string.h>

#define QMI_REQUEST_CONTROL_FLAG 0x00
#define QMI_RESPONSE_CONTROL_FLAG 0x02
#define QMI_INDICATION_CONTROL_FLAG 0x04
#define QMI_HEADER_SIZE 7
#define MAX_ADDR_LEN (16)
#define VERSION_MASK 0xff
#define GET_VERSION(x) (x & 0xff)
#define SET_VERSION(x) (x & 0xff)
#define INSTANCE_MASK 0xff00
#define GET_INSTANCE(x) ((x & 0xff00) >> 8)
#define SET_INSTANCE(x) ((x & 0xff) << 8)

#define LIST(type, name) \
  struct { \
    type *head; \
    type *tail; \
    unsigned int count; \
  } name

#define LINK(type, link) \
  struct { \
    type *prev; \
    type *next; \
  } link

#define LIST_INIT(list) do {  \
  (list).head = (list).tail = NULL; \
  (list).count = 0; \
} while(0)

#define LINK_INIT(link) (link).next = (link).prev = NULL

#define LIST_HEAD(list) (list.head)
#define LIST_TAIL(list) (list.tail)
#define LIST_CNT(list) (list.count)
#define LIST_REMOVE(list, node, link) \
  do { \
    if((node)->link.prev) \
      (node)->link.prev->link.next = (node)->link.next; \
    else /* node at the front of the list */ \
      list.head = (node)->link.next; \
    if((node)->link.next) \
      (node)->link.next->link.prev = (node)->link.prev; \
    else /* node at the end of the list */ \
      list.tail = (node)->link.prev; \
    list.count--; \
  } while(0)

#define LIST_ADD(list, node, link) \
  do { \
    if(!list.tail) \
    { \
      /* first node on the list */ \
      list.tail = list.head = node; \
    } \
    else \
    { \
      (node)->link.prev = list.tail; \
      list.tail->link.next = node; \
      list.tail = node; \
    } \
    list.count++; \
  } while(0)

#define LIST_FIND(list, iter, link, test) do {  \
  iter = (list).head; \
  while(iter) { \
    if(test) {  \
      break;  \
    } \
    iter = (iter)->link.next;  \
  } \
} while(0)


/* QMI message format:
 * +------------------+---------------+---------------+------------+
 * | 1-byte cntl flag | 2-byte txn ID | 2-byte msg ID | 2-byte len |
 * +------------------+---------------+---------------+------------+
 */
static inline void encode_header
(
 unsigned char *buf,
 unsigned char cntl_flag,
 uint16_t txn_id,
 uint16_t msg_id,
 uint16_t msg_len
 )
{
  memcpy(buf, &cntl_flag, 1);
  memcpy(buf+1, &txn_id, 2);
  memcpy(buf+3, &msg_id, 2);
  memcpy(buf+5, &msg_len, 2);
}

static inline void decode_header
(
 unsigned char *buf,
 uint8_t *cntl_flag,
 uint16_t *txn_id,
 uint16_t *msg_id,
 uint16_t *msg_len
)
{
  memcpy(cntl_flag, buf, 1);
  memcpy(txn_id, buf+1, 2);
  memcpy(msg_id, buf+3, 2);
  memcpy(msg_len, buf+5, 2);
}

#endif
