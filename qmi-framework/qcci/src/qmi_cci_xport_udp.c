/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "qmi_cci_target.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"

#define UDP_BASE_PORT (10000)

struct xport_handle
{
  qmi_cci_client_type *clnt;
  int fd;
  uint32_t max_rx_len;
  pthread_t tid;
};

static void *reader_thread(void *handle)
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  struct sockaddr_in addr;
  unsigned char *buf;
  int rx_len;
  uint32_t addr_size = sizeof(struct sockaddr_in);

  buf = malloc(xp->max_rx_len);
  if(!buf)
  {
    printf("Unable to allocate read buffer\n");
    return NULL;
  }

  while((rx_len = recvfrom(xp->fd, buf, xp->max_rx_len, 0, (struct sockaddr *)&addr, &addr_size)) >
      0)
    qmi_cci_xport_recv(xp->clnt, &addr, buf, rx_len);

  free(buf);
  return NULL;
}

static void *xport_open
(
 void *xport_data,
 qmi_cci_client_type *clnt,
 uint32_t service_id,
 uint32_t version,
 void *addr,
 uint32_t max_rx_len
 )
{
  struct xport_handle *xp = calloc(sizeof(struct xport_handle), 1);
  if(xp)
  {
    xp->clnt = clnt;
    xp->max_rx_len = max_rx_len;
    xp->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(!xp->fd)
    {
      free(xp);
      return NULL;
    }
    /* create reader thread */
    if(pthread_create(&xp->tid, NULL, reader_thread, xp))
    {
      perror("pthread_create()");
      close(xp->fd);
      free(xp);
      return NULL;
    }
    printf("xport_open[%d]: max_rx_len=%d\n", xp->fd, max_rx_len);
  }

  return xp;
}

static qmi_client_error_type xport_send
(
 void *handle,
 void *addr,
 uint8_t *buf,
 uint32_t len
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  struct sockaddr_in *s_addr = (struct sockaddr_in *)addr;
  printf("Send[%d]: %d bytes to port %d\n", xp->fd, len, ntohs(s_addr->sin_port));
  if(sendto(xp->fd, buf, len, 0, (struct sockaddr *)s_addr, sizeof(struct sockaddr_in)) < 0)
  {
    perror("sendto()");
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
}

static void xport_close
(
 void *handle
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  char c=0;
  printf("Close[%d]\n", xp->fd);
  /* send a 1 byte message */
  if(sendto(xp->fd, &c, 1, 0, (struct sockaddr *)xp->clnt->server_addr, sizeof(struct sockaddr_in)) < 0)
    perror("sendto()");
  close(xp->fd);
  //pthread_join(xp->tid, NULL);
  free(xp);
}
static qmi_client_error_type xport_lookup
(
 void *xport_data,
 uint32_t service_id,
 uint32_t version,
 uint32_t num_entries,
 qmi_cci_service_list_type *service_list
 )
{
  struct sockaddr_in addr;
  qmi_cci_service_record_type *record;
  printf("Lookup: type=%d instance=%d\n", service_id, version);
  bzero(&addr, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(UDP_BASE_PORT + service_id);
  record = calloc(sizeof(qmi_cci_service_record_type), 1);
  memcpy(record->addr, &addr, sizeof(struct sockaddr_in));
  LIST_ADD((*service_list), record, link);
  return QMI_NO_ERR;
}

static uint32_t xport_addr_len
(
 void
 )
{
  return sizeof(struct sockaddr_in);
}

qmi_cci_xport_ops_type udp_ops =
{
  xport_open,
  xport_send,
  xport_close,
  xport_lookup,
  xport_addr_len
};
