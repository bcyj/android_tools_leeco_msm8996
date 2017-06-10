/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __PPROC_COMMON_BUFF_MGR_H__
#define __PPROC_COMMON_BUFF_MGR_H__

#include "mct_list.h"
#include <media/msmb_generic_buf_mgr.h>
#include <linux/videodev2.h>

typedef struct _pproc_buff_mgr_plane_info{
  void *buf;
  uint32_t  size;
  int  fd;
  uint32_t offset;
  uint32_t stride;
  uint32_t scanline;
} pproc_buff_mgr_plane_info_t;

typedef struct _pproc_buff_mgr_frmbuffer {
  uint32_t buf_index;
  uint32_t num_planes;
  uint32_t buf_size;
  cam_mapping_buf_type buf_type;
  pproc_buff_mgr_plane_info_t buf_planes[VIDEO_MAX_PLANES];
} pproc_buff_mgr_frmbuffer_t;

typedef struct _pproc_buff_mgr_client_uid {
  /* Multiple clients can be encounted with same identity. */
  uint32_t    client_id;
  /* Identity is bitfield of session and stream ids */
  uint32_t    identity;
} pproc_buff_mgr_client_uid_t;

typedef struct _pproc_buff_mgr_client_data {
  /* Unique client identification */
  pproc_buff_mgr_client_uid_t client_uid;
  /* Associated buffer list */
  mct_list_t *client_buffs;
} pproc_buff_mgr_client_data_t;

/** _pproc_buff_mgr_client:
 *    @client_id: pointer address of 'this' object stored as
 *                unique client id.
 *    @buff_mgr:  handle to buffer manager object
 *
 * Client object used by clients
 **/
typedef struct _pproc_buff_mgr_client {
  uint32_t client_id;
  void    *buff_mgr;
} pproc_buff_mgr_client_t;

typedef struct _pproc_buff_mgr {
  uint8_t     is_subdev_open;
  int32_t     subdev_fd;
  uint32_t    client_ref_count;
  mct_list_t *attached_clients;
} pproc_buff_mgr_t;

pproc_buff_mgr_client_t *pproc_common_buff_mgr_get_client(void);

void pproc_common_buff_mgr_put_client(
  pproc_buff_mgr_client_t *buf_mgr_client);

boolean pproc_common_buff_mgr_attach_identity(
  pproc_buff_mgr_client_t *buf_mgr_client, uint32_t identity,
  void *buff_list);

boolean pproc_common_buff_mgr_detach_identity(
  pproc_buff_mgr_client_t *buf_mgr_client, uint32_t identity);

boolean pproc_common_buff_mgr_get_buffer(
  pproc_buff_mgr_client_t *buf_mgr_client, uint32_t identity,
  pproc_buff_mgr_frmbuffer_t **buffer);

boolean pproc_common_buff_mgr_get_offline_buffer(
  pproc_buff_mgr_client_t *buf_mgr_client, uint32_t identity,
  pproc_buff_mgr_frmbuffer_t **buffer, uint8_t buf_index);

boolean pproc_common_buff_mgr_put_buffer(
  pproc_buff_mgr_client_t *buf_mgr_client, uint32_t identity,
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp);

boolean pproc_common_buff_mgr_buffer_done(
  pproc_buff_mgr_client_t *buf_mgr_client, uint32_t identity,
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp);

#endif /* __PPROC_COMMON_BUFF_MGR_H__ */
