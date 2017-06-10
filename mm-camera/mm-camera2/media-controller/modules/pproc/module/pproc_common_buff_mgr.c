/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <linux/media.h>
#include "camera_dbg.h"
#include "cam_types.h"
#include "mct_object.h"
#include "mct_list.h"
#include "mct_stream.h"
#include "pproc_common_buff_mgr.h"

#define PPROC_IDENTITY_MASK   0x0000FFFF
#define PPROC_SESSION_SHIFT 16

//#define PPROC_BUFF_MGR_DEBUG
#ifdef PPROC_BUFF_MGR_DEBUG
#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

/* Singleton buffer manager object */
static pproc_buff_mgr_t buff_mgr_obj;

static boolean pproc_common_buff_mgr_match_output_buff_idx(void *list_data,
  void *user_data)
{
  uint32_t *req_buff_idx = (uint32_t *)user_data;
  pproc_buff_mgr_frmbuffer_t *list_buff =
    (pproc_buff_mgr_frmbuffer_t *)list_data;

  CDBG("%s: req_idx:%d, list_idx:%d\n", __func__, *req_buff_idx,
    list_buff->buf_index);
  if ((*req_buff_idx == list_buff->buf_index) &&
      (list_buff->buf_type != CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF)) {
    return TRUE;
  }

  return FALSE;
}

static boolean pproc_common_buff_mgr_match_input_buff_idx(void *list_data,
  void *user_data)
{
  uint8_t *req_buff_idx = (uint8_t *)user_data;
  pproc_buff_mgr_frmbuffer_t *list_buff =
    (pproc_buff_mgr_frmbuffer_t *)list_data;

  CDBG("%s: req index :%d, list index:%d\n", __func__, *req_buff_idx,
    list_buff->buf_index);
  CDBG("req buf type %d list buf type %d",
    CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF, list_buff->buf_type);
  if ((*req_buff_idx == list_buff->buf_index) &&
      (list_buff->buf_type == CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF)) {
    return TRUE;
  }

  return FALSE;
}

static boolean pproc_common_buff_mgr_traverse_map_buf(void *list_data,
  void *user_data)
{
  mct_stream_map_buf_t *img_buf = (mct_stream_map_buf_t *)list_data;
  mct_list_t **buffer = (mct_list_t **)user_data;
  mct_list_t *mapped_buffer_list = NULL;
  pproc_buff_mgr_frmbuffer_t *pproc_img_buf = NULL;
  pproc_buff_mgr_plane_info_t *pproc_planes = NULL;
  mct_stream_buf_plane_t *frm_planes = img_buf->buf_planes;
  uint32_t i = 0;

  pproc_img_buf = malloc(sizeof(pproc_buff_mgr_frmbuffer_t));
  if (NULL == pproc_img_buf) {
    CDBG_ERROR("%s: Error allocating memory\n", __func__);
    return FALSE;
  }

  CDBG("%s: proc_buffer_list:%p\n", __func__, buffer);
  CDBG("%s: buff_idx:%d, buf type %d num_planes:%d\n", __func__,
    img_buf->buf_index, img_buf->buf_type, img_buf->num_planes);
  memset((void *)pproc_img_buf, 0, sizeof(pproc_buff_mgr_frmbuffer_t));
  mapped_buffer_list = *buffer;
  pproc_planes = pproc_img_buf->buf_planes;
  pproc_img_buf->buf_index = img_buf->buf_index;
  pproc_img_buf->num_planes = img_buf->num_planes;
  pproc_img_buf->buf_size = img_buf->buf_size;
  pproc_img_buf->buf_type = img_buf->buf_type;

  for (i = 0; i < pproc_img_buf->num_planes; i++) {
    pproc_planes[i].buf = frm_planes[i].buf;
    pproc_planes[i].size = frm_planes[i].size;
    pproc_planes[i].fd = frm_planes[i].fd;
    pproc_planes[i].offset = frm_planes[i].offset;
    pproc_planes[i].stride = frm_planes[i].stride;
    pproc_planes[i].scanline = frm_planes[i].scanline;
  }

  CDBG("%s: Before append\n", __func__);
  mapped_buffer_list = mct_list_append(mapped_buffer_list, pproc_img_buf, NULL, NULL);
  CDBG("%s: After append\n", __func__);
  if (NULL == mapped_buffer_list) {
    CDBG_ERROR("%s: Error appending node\n", __func__);
    free(pproc_img_buf);
    return FALSE;
  }

  *buffer = mapped_buffer_list;
  CDBG("%s: Exit success\n", __func__);
  return TRUE;
}

static boolean pproc_common_buff_mgr_traverse_unmap_buf(void *list_data,
  void *user_data)
{
  if (list_data != NULL) {
    free(list_data);
  }
  return TRUE;
}

static boolean pproc_common_buff_mgr_attachbuf(void *buff_list,
  mct_list_t **pproc_buff_list)
{
  mct_list_t *img_buffer_list = (mct_list_t *)buff_list;

  CDBG("%s: buf_list:%p, pproc_buff_list:%p\n", __func__, buff_list,pproc_buff_list);
  return mct_list_traverse(img_buffer_list,
    pproc_common_buff_mgr_traverse_map_buf, (void *)pproc_buff_list);
}

static void pproc_common_buff_mgr_detachbuf(void *buff_list)
{
  mct_list_t *img_buffer_list = (mct_list_t *)buff_list;

  mct_list_free_all(img_buffer_list,
    pproc_common_buff_mgr_traverse_unmap_buf);

  return;
}

static boolean pproc_common_get_bfr_mngr_subdev(int *buf_mgr_fd)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  char subdev_name[32];
  int32_t dev_fd = 0, ioctl_ret;
  boolean ret = FALSE;
  uint32_t i = 0;

  CDBG("%s:%d Enter\n", __func__, __LINE__);
  while (1) {
    int32_t num_entities = 1;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      CDBG("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      break;
    }
    num_media_devices++;
    ioctl_ret = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (ioctl_ret < 0) {
      CDBG("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model) != 0)) {
      close(dev_fd);
      continue;
    }
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      CDBG("%s:%d entity id %d", __func__, __LINE__, entity.id);
      ioctl_ret = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (ioctl_ret < 0) {
        CDBG("%s:%d Done enumerating media entities\n", __func__, __LINE__);
        break;
      }
      CDBG("%s:%d entity name %s type %d group id %d\n", __func__, __LINE__,
        entity.name, entity.type, entity.group_id);
      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_BUF_MNGR) {
        snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);
        CDBG("%s: subdev_name:%s\n", __func__, subdev_name);
        *buf_mgr_fd = open(subdev_name, O_RDWR);
        if (*buf_mgr_fd < 0) {
          CDBG("%s:Open subdev failed\n", __func__);
          continue;
        }
        CDBG("%s:Open subdev Success\n", __func__);
        ret = TRUE;
      }
    }
    close(dev_fd);
  }
  return ret;
}

boolean pproc_common_match_client(void *list_data, void *user_data)
{
  pproc_buff_mgr_client_uid_t *client_list_node =
    (pproc_buff_mgr_client_uid_t *)list_data;
  pproc_buff_mgr_client_uid_t *req_client =
    (pproc_buff_mgr_client_uid_t *)user_data;

  CDBG("%s: node_identity:%d, req_identity:%d\n", __func__, client_list_node->identity,
        req_client->identity);
  if ((client_list_node->identity == req_client->identity) &&
    (client_list_node->client_id == req_client->client_id)) {
    return TRUE;
  }
  return FALSE;
}

static mct_list_t *pproc_common_buff_mgr_get_client_node(pproc_buff_mgr_client_t *buf_mgr_client,
  uint32_t identity)
{
  pproc_buff_mgr_t *buff_mgr = (pproc_buff_mgr_t *)buf_mgr_client->buff_mgr;
  pproc_buff_mgr_client_uid_t client_uid;
  mct_list_t *client = NULL;

  client_uid.client_id = buf_mgr_client->client_id;
  client_uid.identity = identity;

  CDBG("%s: client_id:%d, identity:%d\n", __func__, client_uid.client_id,
        identity);
  client = mct_list_find_custom(buff_mgr->attached_clients, &client_uid,
    pproc_common_match_client);

  return client;
}

pproc_buff_mgr_client_t *pproc_common_buff_mgr_get_client(void)
{
  pproc_buff_mgr_client_t *buff_mgr_client = NULL;
  /* check if the subdevice is open already */
  if (!buff_mgr_obj.is_subdev_open) {
    buff_mgr_obj.subdev_fd = -1;
    CDBG("%s: open device\n", __func__);
    if (pproc_common_get_bfr_mngr_subdev(&buff_mgr_obj.subdev_fd) == FALSE) {
      CDBG("%s:%d Error opening subdev node\n", __func__, __LINE__);
      return NULL;
    }
    buff_mgr_obj.is_subdev_open = 1;
  }
  buff_mgr_client =
    (pproc_buff_mgr_client_t *)malloc(sizeof(pproc_buff_mgr_client_t));
  if (buff_mgr_client == NULL) {
    if (!buff_mgr_obj.client_ref_count) {
      CDBG_ERROR("%s: client mem alloc failed\n", __func__);
      close(buff_mgr_obj.subdev_fd);
      buff_mgr_obj.subdev_fd = -1;
      buff_mgr_obj.is_subdev_open = 0;
    }
    CDBG_ERROR("%s:%d: Mem alloc error for client\n", __func__, __LINE__);
    return NULL;
  }

  memset((void *)buff_mgr_client, 0, sizeof(pproc_buff_mgr_client_t));
  buff_mgr_client->client_id = (uint32_t)buff_mgr_client;
  buff_mgr_client->buff_mgr = &buff_mgr_obj;
  buff_mgr_obj.client_ref_count++;

  CDBG("%s: clientid:%d\n", __func__, buff_mgr_client->client_id);
  return buff_mgr_client;
}

void pproc_common_buff_mgr_put_client(
  pproc_buff_mgr_client_t *buf_mgr_client)
{
  buff_mgr_obj.client_ref_count--;
  /* TODO: Should we traverse the attached client list and detach them or
     report error */
  buf_mgr_client->buff_mgr = NULL;
  free(buf_mgr_client);
  if (!buff_mgr_obj.client_ref_count) {
    if (buff_mgr_obj.is_subdev_open) {
      close(buff_mgr_obj.subdev_fd);
      buff_mgr_obj.subdev_fd = -1;
      buff_mgr_obj.is_subdev_open = 0;
    }
  }
  return;
}

boolean pproc_common_buff_mgr_attach_identity(
  pproc_buff_mgr_client_t *buf_mgr_client, uint32_t identity,
  void *buff_list)
{
  pproc_buff_mgr_t *buff_mgr = NULL;
  pproc_buff_mgr_client_data_t *client = NULL;
  mct_list_t *attach_client = NULL;

  CDBG("%s: client:%p, identity:%d, buff_list:%p\n", __func__, buf_mgr_client, identity, buff_list);
  buff_mgr = (pproc_buff_mgr_t *)buf_mgr_client->buff_mgr;
  /* Add a client node */
  client = (pproc_buff_mgr_client_data_t *)malloc(sizeof(
    pproc_buff_mgr_client_data_t));
  if (client == NULL) {
    CDBG_ERROR("%s:%d: Mem alloc error for client\n", __func__, __LINE__);
    return FALSE;
  }

  memset((void *)client, 0, sizeof(pproc_buff_mgr_client_data_t));
  client->client_uid.client_id = buf_mgr_client->client_id;
  client->client_uid.identity = identity;

  if (pproc_common_buff_mgr_attachbuf(buff_list,
    &client->client_buffs) == FALSE) {
    CDBG_ERROR("%s: Error attachbufer\n", __func__);
    pproc_common_buff_mgr_detachbuf(client->client_buffs);
    free(client);
    return FALSE;
  }

  /* Update the attached client list */
  attach_client = mct_list_append(buff_mgr->attached_clients,
    client, NULL, NULL);
  if (NULL == attach_client) {
    CDBG_ERROR("%s: Error appending to attached client list\n", __func__);
    pproc_common_buff_mgr_detachbuf(client->client_buffs);
    free(client);
    return FALSE;
  }

  CDBG("%s: attach_client:%p\n", __func__, attach_client);
  buff_mgr->attached_clients = attach_client;
  return TRUE;
}

boolean pproc_common_buff_mgr_detach_identity(
  pproc_buff_mgr_client_t *buf_mgr_client, uint32_t identity)
{
  pproc_buff_mgr_t *buff_mgr = (pproc_buff_mgr_t *)buf_mgr_client->buff_mgr;
  pproc_buff_mgr_client_uid_t client_uid;
  pproc_buff_mgr_client_data_t *list_client_node;
  mct_list_t *client = NULL;

  client_uid.client_id = buf_mgr_client->client_id;
  client_uid.identity = identity;

  client = mct_list_find_custom(buff_mgr->attached_clients,
    &client_uid, pproc_common_match_client);
  if (client == NULL) {
    CDBG_ERROR("%s:%d client not found\n", __func__, __LINE__);
    return FALSE;
  }

  list_client_node = (pproc_buff_mgr_client_data_t *)client->data;
  pproc_common_buff_mgr_detachbuf(list_client_node->client_buffs);
  buff_mgr->attached_clients = mct_list_remove(buff_mgr->attached_clients,
    (void *)list_client_node);
  free(list_client_node);
  return FALSE;
}

boolean pproc_common_buff_mgr_get_offline_buffer(
  pproc_buff_mgr_client_t *buf_mgr_client, uint32_t identity,
  pproc_buff_mgr_frmbuffer_t **buffer, uint8_t buf_index)
{
  int ret = 0;
  mct_list_t *list_node;
  pproc_buff_mgr_client_data_t *client_data;

  CDBG("%s: Enter\n", __func__);
  CDBG("%s:%d\n", __func__, __LINE__);
  /* Pick the correct buffer from mapped list */
  list_node = pproc_common_buff_mgr_get_client_node(buf_mgr_client, identity);
  if (!list_node) {
    CDBG("%s:%d\n", __func__, __LINE__);
    CDBG_ERROR("%s: Stream buffer list not found\n", __func__);
    return FALSE;
  }

  CDBG("%s:%d\n", __func__, __LINE__);
  client_data = (pproc_buff_mgr_client_data_t *)list_node->data;

  list_node = mct_list_find_custom(client_data->client_buffs,
    (void *)&buf_index, pproc_common_buff_mgr_match_input_buff_idx);
  if (!list_node) {
    CDBG("%s:%d\n", __func__, __LINE__);
    CDBG_ERROR("%s: Stream buffer not found in list\n", __func__);
    return FALSE;
  }
  CDBG("%s:%d\n", __func__, __LINE__);
  *buffer = (pproc_buff_mgr_frmbuffer_t *)list_node->data;
  CDBG("%s: Exit\n", __func__);
  return TRUE;
}

boolean pproc_common_buff_mgr_get_buffer(pproc_buff_mgr_client_t *buf_mgr_client,
  uint32_t identity, pproc_buff_mgr_frmbuffer_t **buffer)
{
  int ret = 0;
  pproc_buff_mgr_t *buff_mgr = (pproc_buff_mgr_t *)buf_mgr_client->buff_mgr;
  struct msm_buf_mngr_info buff;
  mct_list_t *list_node;
  pproc_buff_mgr_client_data_t *client_data;

  CDBG("%s: Enter\n", __func__);
  buff.session_id = (identity >> PPROC_SESSION_SHIFT) & PPROC_IDENTITY_MASK;
  buff.stream_id = identity & PPROC_IDENTITY_MASK;
  ret = ioctl(buff_mgr->subdev_fd, VIDIOC_MSM_BUF_MNGR_GET_BUF, &buff);
  if (ret < 0) {
    CDBG("%s:Failed to get_buf", __func__);
    return FALSE;
  }

  CDBG("%s: bufferidx:%d, identity:0x%x\n", __func__, buff.index, identity);
  /* Pick the correct buffer from mapped list */
  list_node = pproc_common_buff_mgr_get_client_node(buf_mgr_client, identity);
  if (!list_node) {
    CDBG_ERROR("%s: Stream buffer list not found\n", __func__);
    ioctl(buff_mgr->subdev_fd, VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buff);
    return FALSE;
  }

  client_data = (pproc_buff_mgr_client_data_t *)list_node->data;

  list_node = mct_list_find_custom(client_data->client_buffs, (void *)&buff.index,
    pproc_common_buff_mgr_match_output_buff_idx);
  if (!list_node) {
    CDBG_ERROR("%s: Stream buffer not found in list\n", __func__);
    ioctl(buff_mgr->subdev_fd, VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buff);
    return FALSE;
  }
  *buffer = (pproc_buff_mgr_frmbuffer_t *)list_node->data;
  CDBG("%s: Exit\n", __func__);
  return TRUE;
}

boolean pproc_common_buff_mgr_put_buffer(pproc_buff_mgr_client_t *buf_mgr_client,
  uint32_t identity, uint32_t buff_idx, uint32_t frameid, struct timeval timestamp)
{
  int ret = 0;
  pproc_buff_mgr_t *buff_mgr = (pproc_buff_mgr_t *)buf_mgr_client->buff_mgr;
  struct msm_buf_mngr_info buff;

  CDBG("%s: Enter\n", __func__);
  buff.index = buff_idx;
  buff.session_id = (identity >> PPROC_SESSION_SHIFT) & PPROC_IDENTITY_MASK;
  buff.stream_id = identity & PPROC_IDENTITY_MASK;
  buff.frame_id = frameid;
  buff.timestamp = timestamp;
  ret = ioctl(buff_mgr->subdev_fd, VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buff);
  if (ret < 0) {
    CDBG("%s:Failed to put_buf", __func__);
    return FALSE;
  }
  CDBG("%s: bufferidx:%d, identity:0x%x\n", __func__, buff.index, identity);
  CDBG("%s: Exit\n", __func__);
  return TRUE;
}

boolean pproc_common_buff_mgr_buffer_done(pproc_buff_mgr_client_t *buf_mgr_client,
  uint32_t identity, uint32_t buff_idx, uint32_t frameid, struct timeval timestamp)
{
  int ret = 0;
  pproc_buff_mgr_t *buff_mgr = (pproc_buff_mgr_t *)buf_mgr_client->buff_mgr;
  struct msm_buf_mngr_info buff;

  CDBG("%s: Enter\n", __func__);
  buff.index = buff_idx;
  buff.session_id = (identity >> PPROC_SESSION_SHIFT) & PPROC_IDENTITY_MASK;
  buff.stream_id = identity & PPROC_IDENTITY_MASK;
  buff.frame_id = frameid;
  buff.timestamp = timestamp;
  ret = ioctl(buff_mgr->subdev_fd, VIDIOC_MSM_BUF_MNGR_BUF_DONE, &buff);
  if (ret < 0) {
    CDBG("%s:Failed to do buf_done", __func__);
    return FALSE;
  }

  CDBG("%s: bufferidx:%d, identity:0x%x\n", __func__, buff.index, identity);
  CDBG("%s: Exit\n", __func__);
  return TRUE;
}
