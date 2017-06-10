/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/media.h>

#include "camera_dbg.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "ispif.h"
#include "ispif_util.h"
#include "isp_resource_mgr.h"
#include "server_debug.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ISPIF_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif

/** ispif_hw_reset: reset ISPIF HW
 *    @ispif: ispif pointer
 *    @ispif_sink_port: sink port pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function resets ISPIF HW
 *
 *  Return:  0 - Success
 *          -1 - Incorrect number of isps or hw reset error
 *
 **/
static int ispif_hw_reset(ispif_t *ispif, ispif_port_t *ispif_sink_port)
{
  int rc = 0;
  struct ispif_cfg_data *cfg_cmd = &ispif->cfg_cmd;
  ispif_sink_port_t *sink_port = &ispif_sink_port->u.sink_port;
  isp_info_t isp_info[VFE_MAX];
  uint32_t primary_cid_idx;
  int num_isps = 0;
  int i;

  if (ispif->num_active_streams > 0)
    return 0;

  num_isps = isp_get_info(isp_info);
  if (num_isps > VFE_MAX) {
    CDBG_ERROR("%s: num_ips = %d, larger than max allowed %d",
      __func__, num_isps, VFE_MAX);
    return -1;
  }

  primary_cid_idx = ispif_util_find_primary_cid(&sink_port->sensor_cfg,
    &sink_port->sensor_cap);
  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return -1;
  }
  memset(cfg_cmd, 0, sizeof(struct ispif_cfg_data));
  cfg_cmd->cfg_type = ISPIF_SET_VFE_INFO;
  cfg_cmd->vfe_info.num_vfe = num_isps;
  for (i = 0; i < num_isps; i++) {
    cfg_cmd->vfe_info.info[i].max_resolution = isp_info[i].max_resolution;
    cfg_cmd->vfe_info.info[i].id = isp_info[i].id;
    cfg_cmd->vfe_info.info[i].ver = isp_info[i].ver;
  }

  CDBG("%s: ispif init", __func__);
  rc = ioctl(ispif->fd, VIDIOC_MSM_ISPIF_CFG, cfg_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: ISPIF_SET_VFE_INFO kernel return error = %d", __func__, rc);
    return rc;
  }

  memset(cfg_cmd, 0, sizeof(struct ispif_cfg_data));
  cfg_cmd->cfg_type = ISPIF_INIT;
  cfg_cmd->csid_version =
    sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].csid_version;
  rc = ioctl(ispif->fd, VIDIOC_MSM_ISPIF_CFG, cfg_cmd);
  CDBG("%s: ispif init rc = %d", __func__, rc);
  if (rc != -EAGAIN && rc != 0) {
    /* error in ioctl init */
    CDBG_ERROR("%s: ISPIF_INIT, kernel return error = %d", __func__, rc);
    return rc;
  }

  return rc;
}

/** ispif_discover_subdev_nodes: discover ISPIF subdev
 *    @ispif: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function discover ISPIF HW
 *
 *  Return:  0 - Success
 *          -1 - error during discovering subdevices
 **/
static int ispif_discover_subdev_nodes(ispif_t *ispif)
{
  struct media_device_info mdev_info;
  int num_media_devices = 0;
  char dev_name[32];
  int rc = 0, dev_fd = 0;
  int num_entities;

  while (1) {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd >= MAX_FD_PER_PROCESS) {
      dump_list_of_daemon_fd();
      dev_fd = -1;
      break;
    }

    if (dev_fd < 0) {
      CDBG("Done discovering media devices\n");
      break;
    }

    num_media_devices++;
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      CDBG_ERROR("Error: ioctl media_dev failed: %s\n", strerror(errno));
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }

    num_entities = 1;
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        CDBG("Done enumerating media entities\n");
        close(dev_fd);
        rc = 0;
        break;
      }

      CDBG("%s:%d entity name %s type %d group id %d\n", __func__, __LINE__,
        entity.name, entity.type, entity.group_id);
      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_ISPIF) {
        snprintf(ispif->dev_name, sizeof(ispif->dev_name), "/dev/%s",
          entity.name);
        close(dev_fd);

        return 0; /* done */
      }
    }

    close(dev_fd);
  }

  return -1;
}

/** ispif_init:
 *    @ispif: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function initializes ispif module.
 *
 *  Return:  0 - Success
 *          -1 - error during discovering subdevices
 **/
int ispif_init(ispif_t *ispif)
{
  int rc = 0;
  pthread_mutex_init(&ispif->mutex, NULL);
  rc = ispif_discover_subdev_nodes(ispif);
  return rc;
}

/** ispif_destroy:
 *    @ispif: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This stub function will contain code to destroy ispif module.
 *
 *  Return:  None
 **/
void ispif_destroy (ispif_t *ispif)
{
  return;
}

/** ispif_start_session:
 *    @ispif: ispif pointer
 *    @session_id session id to be started
 *
 *  This function runs in MCTL thread context.
 *
 *  This function starts an ispif session
 *
 *  Return:  0 - Success
 *          -1 - Session not found
 **/
int ispif_start_session(ispif_t *ispif, uint32_t session_id)
{
  int i;
  ispif_session_t *session = NULL;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (ispif->sessions[i].ispif == NULL) {
      /* save the 1st unused session ptr */
      session = &ispif->sessions[i];
      memset(session, 0, sizeof(ispif_session_t));
      session->ispif = ispif;
      session->session_id = session_id;
      session->session_idx = i;

      return 0;
    }
  }

  return -1;
}

/** ispif_stop_session:
 *    @ispif: ispif pointer
 *    @session_id: id of session to be stopped
 *
 *  This function runs in MCTL thread context.
 *
 * This functions stops a session
 *
 *  Return:  0 - Success
 *          -1 - Session not found
 **/
int ispif_stop_session(ispif_t *ispif, uint32_t session_id)
{
  int i;
  ispif_session_t *session = NULL;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    session = &ispif->sessions[i];
    if (session->ispif && session->session_id == session_id) {
      /* save the 1st unused session ptr */
      memset(session, 0, sizeof(ispif_session_t));

      return 0;
    }
  }

  return -1;
}

/** ispif_reserve_sink_port:
 *    @ispif: ispif pointer
 *    @ispif_port: port to be reserved
 *    @sensor_port_cap: sersor src port capabilities
 *    @stream_info: stream info
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function tries to reserve a sink port in ispif module.
 *
 *  Return:  0 - Success
 *     -EAGAIN - Has another port matching the sensor cap.
 *      other negative value - error while reserving port
 **/
int ispif_reserve_sink_port(ispif_t *ispif, ispif_port_t *ispif_port,
  sensor_src_port_cap_t *sensor_port_cap, mct_stream_info_t *stream_info,
  unsigned int session_id, unsigned int stream_id)
{
  int rc = 0;
  int is_new_sink = 0;
  ispif_stream_t *stream = NULL;
  ispif_port_t *ispif_sink_port = NULL;
  ispif_session_t *session = ispif_util_get_session_by_id(ispif,  session_id);

  if (!session){
      CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
      return -1;
  }

  if (sensor_port_cap->num_cid_ch >= SENSOR_CID_CH_MAX) {
    CDBG_ERROR("%s: out of range of cid_num %d\n", __func__, SENSOR_CID_CH_MAX);
    return -EINVAL;
  }

  stream = ispif_util_find_stream(ispif, session_id, stream_id);
  if (stream != NULL) {
    CDBG_ERROR("%s: stream already exist. error\n", __func__);
    return -1;
  }

  ispif_sink_port = ispif_util_find_sink_port(ispif, sensor_port_cap);
  if (ispif_sink_port != NULL && ispif_sink_port != ispif_port) {
    /* has another port matching the sensor cap. EAGAIN */
    return -EAGAIN;
  }

  if (ispif_sink_port == NULL) {
    if (ispif_port->state == ISPIF_PORT_STATE_CREATED) {
      ispif_sink_port = ispif_port;
      ispif_sink_port->state = ISPIF_PORT_STATE_RESERVED;
      is_new_sink = 1;
    } else
      return -1;
  }

  /* add stream */
  stream = ispif_util_add_stream(ispif, session, stream_id, stream_info);
  if (!stream) {
    /* no stream available */
    if (is_new_sink) {
      ispif_sink_port->state = ISPIF_PORT_STATE_CREATED;
    }

    return -1;
  }

  if (is_new_sink) {
    ispif_sink_port->u.sink_port.sensor_cap = *sensor_port_cap;
    ispif_sink_port->session_id = session_id;
  }

  rc = ispif_util_add_stream_to_sink_port(ispif, ispif_sink_port, stream);

  if (rc < 0)
    goto error;

  return rc;

error:
  ispif_unreserve_sink_port(ispif, ispif_sink_port,
    stream->session_id, stream->stream_id);

  return rc;
}

/** ispif_reserve_src_port:
 *    @ispif: ispif pointer
 *    @src_port: src port to be reserved
 *    @stream_info: stream info
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function reserves an ispif sink port
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or port is in invalid state
 *     -EAGAIN - The given src port cannot be used to reserve
 **/
int ispif_reserve_src_port(ispif_t *ispif, ispif_port_t *src_port,
  mct_stream_info_t *stream_info, unsigned int session_id,
  unsigned int stream_id)
{
  int i, rc = 0;
  int is_new_sink = 0;
  ispif_stream_t *stream = NULL;
  ispif_port_t *ispif_src_port = NULL;
  ispif_port_t *ispif_sink_port = NULL;

  stream = ispif_util_find_stream(ispif, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not found. error\n", __func__);
    return -1;
  }

  ispif_sink_port = (ispif_port_t *)stream->sink_port->port_private;
  ispif_src_port = ispif_util_get_match_src_port(ispif, ispif_sink_port, stream);
  if (ispif_src_port != NULL && ispif_src_port != src_port) {
    /* the given src port cannot be used to reserve */
    return -EAGAIN;
  }

  if (ispif_src_port == NULL) {
    if (src_port->state == ISPIF_PORT_STATE_CREATED) {
      ispif_src_port = src_port;
      ispif_src_port->u.src_port.caps.use_pix = stream->use_pix;
      ispif_src_port->u.src_port.caps.input_type = ISP_INPUT_ISPIF;
      ispif_src_port->u.src_port.caps.sensor_cap =
        ispif_sink_port->u.sink_port.sensor_cap;
      ispif_src_port->state = ISPIF_PORT_STATE_RESERVED;
      ispif_src_port->session_id = stream->session_id;
    } else
      return -1;
  }

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_src_port->u.src_port.streams[i] == NULL) {
      ispif_src_port->u.src_port.streams[i] = stream;
      stream->link_cnt++;
      ispif_src_port->u.src_port.num_streams++;
      CDBG("%s: link_cnt = %d\n", __func__, stream->link_cnt);
      break;
    }
  }

  stream->src_port = ispif_src_port->port;
  stream->state = ISPIF_STREAM_ASSOCIATED_WITH_SRC_PORT;

  return rc;
}

/** ispif_unreserve_sink_port:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: sink port to be unreserved
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unreserves an ispif port.
 *
 *  Return:  0 - Success
 *          -1 - Stream not found
 **/
int ispif_unreserve_sink_port(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id)
{
  int i, rc = 0;
  ispif_stream_t *stream = NULL;

  stream = ispif_util_find_stream(ispif, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not found. error\n", __func__);
    return -1;
  }

  ispif_util_del_stream_from_sink_port(ispif, ispif_sink_port, stream);
  if (ispif_sink_port->u.sink_port.num_streams == 0) {
    /* no more stream associated with it. release the port */
    ispif_sink_port->state = ISPIF_PORT_STATE_CREATED;
    ispif_sink_port->session_id = 0; /* remove the session id */
    ispif_sink_port->port->peer = NULL;
  }

  if (stream->link_cnt == 0)
    ispif_util_del_stream(ispif, stream);

  return rc;
}

/** ispif_unreserve_src_port:
 *    @ispif: ispif pointer
 *    @ispif_src_port: source port to be unreserved
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unreserves an ispif source port
 *
 *  Return:  0 - Success
 *          -1 - Stream not found
 **/
int ispif_unreserve_src_port(ispif_t *ispif, ispif_port_t *ispif_src_port,
  unsigned int session_id, unsigned int stream_id)
{
  int rc = 0;
  ispif_stream_t *stream = NULL;

  stream = ispif_util_find_stream(ispif, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not found. error\n", __func__);
    return -1;
  }

  ispif_util_del_stream_from_src_port(ispif, ispif_src_port, stream);
  if (ispif_src_port->u.src_port.num_streams == 0) {
    /* no more stream associated with it. release the port */
    ispif_src_port->state = ISPIF_PORT_STATE_CREATED;
    ispif_src_port->session_id = 0;
    ispif_src_port->port->peer = NULL;
  }

  if (stream->link_cnt == 0)
    ispif_util_del_stream(ispif, stream);

  return rc;
}

/** ispif_proc_streamon:
 *    @ispif: ispif pointer
 *    @session: pointer to ispif session
 *    @ispif_sink_port: ispif sink port
 *    @num_streams: number of streams to be started
 *    @stream_ids: stream ids
 *
 *  This function runs in MCTL thread context.
 *
 *  This function start the streaming in ispif module
 *
 *  Return:  0 - Success
 *           negative value - unsuccessful start of streaming
 **/
static int ispif_proc_streamon(ispif_t *ispif, ispif_session_t *session,
  ispif_port_t *ispif_sink_port, int num_streams, uint32_t *stream_ids)
{
  int i, rc = 0;
  uint32_t k;
  int isp_id;
  unsigned int meta_isp_id;
  uint32_t num_cid_ch_non_meta = 0;
  ispif_stream_t *stream = NULL;
  struct ispif_cfg_data *cfg_cmd = &ispif->cfg_cmd;
  ispif_port_t *ispif_src_port;

  CDBG("%s: E", __func__);

  /* start ispif */
  if (ispif->fd <= 0) {
    ispif->fd = open(ispif->dev_name, O_RDWR | O_NONBLOCK);
     if ((ispif->fd) >= MAX_FD_PER_PROCESS) {
       dump_list_of_daemon_fd();
       ispif->fd = -1;
       return -1;
     }

    if (ispif->fd <= 0) {
      CDBG_ERROR("%s: cannot open ispif '%s'\n", __func__, ispif->dev_name);
      return -1;
    }
  }

  if (ispif->num_active_streams == 0) {
    /* first streamon, reset ispif */
    rc = ispif_hw_reset(ispif, ispif_sink_port);
    if (rc < 0) {
      CDBG_ERROR("%s: ispif_hw_reset failed", __func__);
      return rc;
    }
    CDBG_HIGH("%s: ispif_hw_reset done\n", __func__);
  }

  memset(cfg_cmd, 0, sizeof(struct ispif_cfg_data));
  cfg_cmd->cfg_type = ISPIF_CFG;
  for (i = 0; i < num_streams; i++) {
    stream = ispif_util_find_stream_in_session(session,stream_ids[i]);
    if (stream == NULL) {
      CDBG_ERROR("%s: stream not exist. error\n", __func__);
      goto error;
    }

    ispif_sink_port = stream->sink_port->port_private;
    ispif_src_port =  stream->src_port->port_private;
    ispif_sink_port->num_active_streams++;
    ispif->num_active_streams++;

    if (((ispif_sink_port->num_active_streams > 1) &&
       (stream->num_meta == 0) &&
       (ispif_src_port->u.src_port.caps.use_pix))||
        (stream->stream_info.stream_type == CAM_STREAM_TYPE_VIDEO )) {
      CDBG("%s: sink_port = %p, num_active_stream = %d, continue\n", __func__,
        ispif_sink_port, ispif_sink_port->num_active_streams);
      continue;
    }

    CDBG("%s: ispif num_active_stream = %d, sink_port_active_stream = %d\n",
      __func__, ispif->num_active_streams, ispif_sink_port->num_active_streams);

    ispif_sink_port->state = ISPIF_PORT_STATE_ACTIVE;
    for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
      if (MAX_PARAM_ENTRIES <= cfg_cmd->params.num) {
        CDBG_ERROR("%s: error, parm entries %d > max value %d\n",
          __func__, (cfg_cmd->params.num)+1, MAX_PARAM_ENTRIES);
        rc = -200;
        goto error;
      }

      /* hi 16 bits for ISP1 and low 16 bits for ISP0 */
      if (stream->used_output_mask & (0xffff << (16 * isp_id))){
        cfg_cmd->params.entries[cfg_cmd->params.num].vfe_intf =
          (enum msm_ispif_vfe_intf)isp_id;
      } else
        continue;

      cfg_cmd->params.entries[cfg_cmd->params.num].intftype =
        ispif_util_find_isp_intf_type
        (stream, stream->used_output_mask, session->vfe_mask);

      if (cfg_cmd->params.entries[cfg_cmd->params.num].intftype == INTF_MAX) {
        CDBG_ERROR("%s: invalid ispif interface mask = %d", __func__,
          cfg_cmd->params.entries[i].intftype);
        goto error;
      }

      /* Note, in current implementation, sensor put all CIDs into one array
       * with following order: 0 - x-1 : non meta CIDs
       * Start with x meta CIDs are provided.
       */
      num_cid_ch_non_meta =
        ispif_sink_port->u.sink_port.sensor_cap.num_cid_ch;
      CDBG("%s: sensor_cap: num non meta cids = %d, meta cid_num = %d\n",
        __func__, num_cid_ch_non_meta,
        (ispif_sink_port->u.sink_port.sensor_cap.num_meta_ch));

      /* only consider 1 non meta cid per stream use case now.*/
      if (num_cid_ch_non_meta > 1) {
        CDBG_ERROR(
          "%s:ERROR: 2 non meta CIDs one stream not supported yet, cids = %d\n",
          __func__, num_cid_ch_non_meta);
        return -100;
      }

      cfg_cmd->params.entries[cfg_cmd->params.num].num_cids =
        num_cid_ch_non_meta;

      for (k = 0; k < num_cid_ch_non_meta; k++) {
        cfg_cmd->params.entries[cfg_cmd->params.num].cids[k] =
          ispif_sink_port->u.sink_port.sensor_cap.sensor_cid_ch[k].cid;
        cfg_cmd->params.entries[cfg_cmd->params.num].csid =
          ispif_sink_port->u.sink_port.sensor_cap.sensor_cid_ch[k].csid;
      }

      /*dual vfe configuration*/
      if (stream->split_info.is_split) {
        ispif_out_info_t* split_info = &stream->split_info;
        if (isp_id == VFE0) {
          /* left half image */
          cfg_cmd->params.entries[cfg_cmd->params.num].crop_enable       = 1;
          cfg_cmd->params.entries[cfg_cmd->params.num].crop_start_pixel  = 0;
          cfg_cmd->params.entries[cfg_cmd->params.num].crop_end_pixel    =
            split_info->right_stripe_offset + split_info->overlap - 1;
        } else {
          cfg_cmd->params.entries[cfg_cmd->params.num].crop_enable      = 1;
          cfg_cmd->params.entries[cfg_cmd->params.num].crop_start_pixel =
            split_info->right_stripe_offset;
          cfg_cmd->params.entries[cfg_cmd->params.num].crop_end_pixel   =
            ispif_sink_port->u.sink_port.sensor_cfg.dim_output.width - 1;
        }
      }

      /* one entry added */
      cfg_cmd->params.num++;

      /*config metadata channel: assume ONE meta channel
        1. get RDI interface type
        2. config ispif
        3. assume one meta channel for now*/
      CDBG_HIGH("stream id %d, stream num_meta %d\n",
        stream->stream_id, stream->num_meta);
      /* only consider 1 non meta cid per stream use case now.*/
      if (stream->num_meta > 1) {
        CDBG_ERROR("%s: ERROR: only support one meta data now, num_meta = %d\n",
          __func__, stream->num_meta);
        return -100;
      }

      if (stream->num_meta > 0 && stream->meta_info[0].is_valid) {
        for(k = 0; k < stream->num_meta; k++){
          if (MAX_PARAM_ENTRIES <= cfg_cmd->params.num) {
            CDBG_ERROR("%s: error, parm entries %d > max value %d\n",
              __func__, (cfg_cmd->params.num)+1, MAX_PARAM_ENTRIES);
            rc = -200;
            goto error;
          }
          /*config metadata channel*/
          meta_isp_id = (stream->meta_use_output_mask & (0xffff << VFE0))? VFE0:VFE1;
          cfg_cmd->params.entries[cfg_cmd->params.num].vfe_intf =
            (enum msm_ispif_vfe_intf)meta_isp_id;

          cfg_cmd->params.entries[cfg_cmd->params.num].intftype =
            isp_interface_mask_to_interface_num(stream->meta_use_output_mask,
              (1<< meta_isp_id));
          /* one meta only map one cid */
          cfg_cmd->params.entries[cfg_cmd->params.num].num_cids = 1;
          cfg_cmd->params.entries[cfg_cmd->params.num].csid =
            ispif_sink_port->u.sink_port.sensor_cap.meta_ch[k].csid;

          if (ispif_sink_port->u.sink_port.sensor_cap.num_meta_ch > 1)
            CDBG_ERROR("%s: not support one channel multiple cid yet\n",
              __func__);
          else
            cfg_cmd->params.entries[cfg_cmd->params.num].cids[k] =
              ispif_sink_port->u.sink_port.sensor_cap.meta_ch[k].cid;

          cfg_cmd->params.num++;
        }
      }
    }
  }

  CDBG("%s: CID ready\n", __func__);
  if (cfg_cmd->params.num > 0) {
    rc = ioctl(ispif->fd, VIDIOC_MSM_ISPIF_CFG, cfg_cmd);
    if (rc != 0) {
      CDBG_ERROR("%s:#%d ISPIF_CFG error = %d\n", __func__, __LINE__, rc);
      CDBG_ERROR("%s: num_active_streams %d", __func__,
        ispif->num_active_streams);
      goto error;
    }

    cfg_cmd->cfg_type = ISPIF_START_FRAME_BOUNDARY;
    rc = ioctl(ispif->fd, VIDIOC_MSM_ISPIF_CFG, cfg_cmd);
    if (rc != 0) {
      CDBG_ERROR("%s:#%d  ISPIF_CFG error = %d\n", __func__, __LINE__, rc);
      goto error;
    }

    CDBG("%s: START_FRAME_BOUNDARY done\n", __func__);
  }

  CDBG("%s: X, rc = %d", __func__, rc);
  return rc;

error:
  CDBG("%s:ERROR, rc = %d\n", __func__, rc);
  /* need to roll back */
  if (ispif->num_active_streams == 0) {
    if (ispif->fd > 0) {
      close(ispif->fd);
      ispif->fd = 0;
    }
  }

  return rc;
}

/** ispif_streamon:
 *    @ispif: pointer to ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @session_id: session id
 *    @stream_id: stream id
 *    @event: MCTL control event
 *
 *  This function runs in MCTL thread context.
 *
 * This function start ispif streaming
 *
 *  Return:  0 - Success
 *          -1 - cannot find session/stream or
 *               start streaming is not successful
 **/
int ispif_streamon(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event)
{
  int k, rc = 0;
  ispif_stream_t *stream = NULL;
  ispif_session_t *session = ispif_util_get_session_by_id(ispif, session_id);
  int num_streams;
  uint32_t stream_ids[ISP_MAX_STREAMS];

  CDBG("%s: E", __func__);

  if (!session){
      CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
      return -1;
  }

  stream = ispif_util_find_stream_in_session(session, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not exist. error\n", __func__);
    return -1;
  }

  if (session->hal_bundling_mask != 0 &&
      (session->hal_bundling_mask & (1 << stream->stream_idx))) {
    session->streamon_bundling_mask |= (1 << stream->stream_idx);
    if (session->hal_bundling_mask != session->streamon_bundling_mask) {
      CDBG("%s: stream bundle mask = 0x%x, streamon_bundle = 0x%x\n",
        __func__, session->hal_bundling_mask, session->streamon_bundling_mask);
      /* nop */
      return 0;
    } else {
      rc = ispif_util_get_stream_ids_by_mask(session,
        session->hal_bundling_mask, &num_streams, stream_ids);
    }

  } else {
    stream_ids[0] = stream_id;
    num_streams = 1;
  }
  session->streamoff_error = FALSE;
  rc = ispif_proc_streamon(ispif, session, ispif_sink_port, num_streams,
         stream_ids);
  if (rc == 0) {
    session->active_count += num_streams;
    CDBG_HIGH("%s: session_id = %d, active_streams = %d\n",
      __func__, session->session_id, session->active_count);
  }
  return rc;
}

/** ispif_proc_streamoff:
 *    @ispif: pointer to ispif instance
 *    @session: ispif session
 *    @num_streams:  number ot streams
 *    @stream_ids: array of stream ids
 *
 *  This function runs in MCTL thread context.
 *
 * This function stops ispif streaming
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or
 *               invalid interface mask
 *               VIDIOC_MSM_ISPIF_CFG ioctl returned error
 *        -100 - Try to stop two non meta- CIDs with one stream
 *        -200 - Too many parameter entries
 **/
static int ispif_proc_streamoff(ispif_t *ispif, ispif_session_t *session,
  int num_streams, uint32_t *stream_ids)
{
  int rc = 0;
  int i, isp_id;
  uint32_t k;
  boolean brc = FALSE;
  unsigned int meta_isp_id;
  ispif_stream_t *stream = NULL;
  ispif_port_t *ispif_sink_port;
  ispif_port_t *ispif_src_port;
  struct ispif_cfg_data *cfg_cmd = &ispif->cfg_cmd;
  uint32_t prim_cid_idx;

  CDBG("%s: Enter\n", __func__);
  memset(cfg_cmd, 0, sizeof(struct ispif_cfg_data));
  for (i = 0; i < num_streams; i++) {
    stream = ispif_util_find_stream_in_session(session, stream_ids[i]);
    if (stream == NULL) {
      CDBG_ERROR("%s: Cannot find stream(sessionid = %d, streamid = %d\n",
        __func__, session->session_id, stream_ids[i]);
      return -1;
    }

    /* get ISPIF sink port*/
    ispif_sink_port = stream->sink_port->port_private;
    ispif_src_port = stream->src_port->port_private;

    if((ispif_sink_port->num_active_streams <= 0) ||
           (ispif->num_active_streams <= 0)) {
      /* should not happen */
      CDBG_ERROR ("%s: error: No streams to be stopped", __func__);
      return -1;
    }

    if(ispif_sink_port->num_active_streams > 0)
      ispif_sink_port->num_active_streams--;
    else
      ispif_sink_port->num_active_streams = 0;

    if (ispif->num_active_streams > 0)
      ispif->num_active_streams--;
    else
      ispif->num_active_streams = 0;

    prim_cid_idx = ispif_util_find_primary_cid(
      &(ispif_sink_port->u.sink_port.sensor_cfg),
      &(ispif_sink_port->u.sink_port.sensor_cap));

    if (SENSOR_CID_CH_MAX - 1 < prim_cid_idx) {
      CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
      return -1;
    }

    CDBG("%s: ispif active stream = %d, sink_port_active_stream = %d\n",
      __func__, ispif->num_active_streams, ispif_sink_port->num_active_streams);

    if (((ispif_sink_port->num_active_streams > 0) && (ispif_src_port->u.src_port.caps.use_pix)) ||
        (stream->stream_info.stream_type == CAM_STREAM_TYPE_VIDEO)){
      CDBG("%s: sink_port = %p, active_stream = %d, continue\n", __func__,
        ispif_sink_port, ispif_sink_port->num_active_streams);
      continue;
    }

    ispif_sink_port->state = ISPIF_PORT_STATE_RESERVED;
    cfg_cmd->cfg_type = ISPIF_STOP_IMMEDIATELY;
    for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
      /* hi 16 bits for ISP1 and low 16 bits for ISP0 */
      if (stream->used_output_mask & (0xffff << (16 * isp_id)))
        cfg_cmd->params.entries[cfg_cmd->params.num].vfe_intf =
          (enum msm_ispif_vfe_intf)isp_id;
      else
        continue;

      cfg_cmd->params.entries[cfg_cmd->params.num].intftype =
        ispif_util_find_isp_intf_type
        (stream, stream->used_output_mask, session->vfe_mask);

      if (cfg_cmd->params.entries[cfg_cmd->params.num].intftype == INTF_MAX) {
        CDBG_ERROR("%s: invalid ispif interface mask = %d",
          __func__, cfg_cmd->params.entries[cfg_cmd->params.num].intftype);
        return -1;
      }

      /* only consider 1 cid per stream use case now.*/
      if (ispif_sink_port->u.sink_port.sensor_cap.num_cid_ch > 1) {
        CDBG_ERROR("%s: ERROR: 2 CIDs one stream not supported yet!!!!\n",
          __func__);
        return -100;
      }

      cfg_cmd->params.entries[cfg_cmd->params.num].num_cids =
        ispif_sink_port->u.sink_port.sensor_cap.num_cid_ch;
      for (k = 0; k < ispif_sink_port->u.sink_port.sensor_cap.num_cid_ch; k++) {
        cfg_cmd->params.entries[cfg_cmd->params.num].cids[k] =
          ispif_sink_port->u.sink_port.sensor_cap.sensor_cid_ch[k].cid;
      }

      cfg_cmd->params.entries[cfg_cmd->params.num].csid =
        ispif_sink_port->u.sink_port.sensor_cap.sensor_cid_ch[prim_cid_idx].csid;
      cfg_cmd->params.num++;
      if (cfg_cmd->params.num > MAX_PARAM_ENTRIES) {
        CDBG_ERROR("%s: error, parm entries %d > max value %d\n",
          __func__, cfg_cmd->params.num, MAX_PARAM_ENTRIES);
        rc = -200;
        goto end;
      }
       /*config metadata channel: assume ONE meta channel
        1. get RDI interface type
        2. config ispif
        3. assume one meta channel for now*/
      CDBG_HIGH("stream id %d, stream num_meta %d\n",
        stream->stream_id, stream->num_meta);
      /* only consider 1 non meta cid per stream use case now.*/
      if (stream->num_meta > 1) {
        CDBG_ERROR("%s: ERROR: only support one meta data now, num_meta = %d\n",
          __func__, stream->num_meta);
        return -100;
      }
      if (MAX_PARAM_ENTRIES <= cfg_cmd->params.num) {
        CDBG_ERROR("%s: error, parm entries %d > max value %d\n",
          __func__, (cfg_cmd->params.num)+1, MAX_PARAM_ENTRIES);
        return -200;
      }
      if (stream->num_meta > 0 && stream->meta_info[0].is_valid) {
        for (k = 0; k < stream->num_meta; k++) {
          /*config metadata channel*/
          meta_isp_id = (stream->meta_use_output_mask & (0xffff << VFE0)) ?
            VFE0 : VFE1;
          cfg_cmd->params.entries[cfg_cmd->params.num].vfe_intf =
            (enum msm_ispif_vfe_intf)isp_id;

          cfg_cmd->params.entries[cfg_cmd->params.num].intftype =
            isp_interface_mask_to_interface_num(stream->meta_use_output_mask,
              (1 << meta_isp_id));

          if ((isp_interface_mask_to_interface_num(stream->meta_use_output_mask,
             (1 << meta_isp_id))) < 0 )
           continue;

          /* one meta only map one cid */
          cfg_cmd->params.entries[cfg_cmd->params.num].num_cids = 1;
          cfg_cmd->params.entries[cfg_cmd->params.num].csid =
            ispif_sink_port->u.sink_port.sensor_cap.meta_ch[k].csid;

          if (ispif_sink_port->u.sink_port.sensor_cap.num_meta_ch > 1)
            CDBG_ERROR("%s: not support one channel multiple cid yet\n",
              __func__);
          else
            cfg_cmd->params.entries[cfg_cmd->params.num].cids[k] =
              ispif_sink_port->u.sink_port.sensor_cap.meta_ch[k].cid;

          cfg_cmd->params.num++;
          if (cfg_cmd->params.num > MAX_PARAM_ENTRIES) {
            CDBG_ERROR("%s: error, parm entries %d > max value %d\n",
              __func__, cfg_cmd->params.num, MAX_PARAM_ENTRIES);
            rc = -200;
            goto end;
          }
        }
      }
    }
  }
  /* if we had a timeout error while stopping previous stream, the ISPIF is
     already in bad state, so there is no point in stopping other streams
     as well. The ISPIF will recover during HW reset at streamon. */
  if(session->streamoff_error == FALSE) {
    if (cfg_cmd->params.num > 0) {

      rc = ioctl(ispif->fd, VIDIOC_MSM_ISPIF_CFG, cfg_cmd);

      if(rc != 0) {
        if (errno == ETIMEDOUT) {
          CDBG_ERROR("%s, error - ISPIF stop on frame boundary timed out!",
            __func__);
          /*  ISPIF couldn't stop properly.
              We initiate recovery sequence now */
          mct_bus_msg_t bus_msg;
          mct_bus_msg_error_message_t err_msg;
          memset(&bus_msg, 0, sizeof(bus_msg));

          /* Close device */
          if (ispif->fd > 0) {
            close(ispif->fd);
            ispif->fd = 0;
          }
          session->streamoff_error = TRUE;
          /* Bus message to MCTL for each active session */
          for (i = 0; i < ISP_MAX_SESSIONS; i++) {
            if (ispif->sessions[i].ispif != NULL) {
              bus_msg.sessionid = ispif->sessions[i].session_id;
              bus_msg.type = MCT_BUS_MSG_SEND_HW_ERROR;
              if (TRUE != mct_module_post_bus_msg(ispif->module, &bus_msg))
                CDBG_ERROR("%s: MCT_BUS_MSG_ERROR_MESSAGE to bus error\n", __func__);

            }
          }

          return rc;
        } else {
          CDBG_ERROR("%s: ISPIF_CFG error = %d\n", __func__, rc);
          assert(fd == 0);

          return rc;
        }
      }

      if (ispif->num_active_streams == 0) {
        /* no more active stream close ispif */
        if (ispif->fd > 0) {
          close(ispif->fd);
          ispif->fd = 0;
        }
      }
    }
  }

end:
  CDBG("%s: X, rc = %d\n", __func__, rc);

  return rc;

}

/** ispif_streamoff:
 *    @ispif: pointer to ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @session_id: session id
 *    @stream_id: stream id
 *    @event: MCTL control event
 *
 *  This function runs in MCTL thread context.
 *
 * This function stops ispif streaming
 *
 *  Return:  0 - Success
 *          -1 - Invalid stream/session ID or
 *               unsuccessful stopping of stream
 **/
int ispif_streamoff(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event)
{
  int rc = 0;
  int k;
  ispif_stream_t *stream = NULL;
  ispif_session_t *session = ispif_util_get_session_by_id(ispif, session_id);
  int num_streams;
  uint32_t stream_ids[ISP_MAX_STREAMS];

  CDBG("%s: Enter\n", __func__);

  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
    return -1;
  }

  stream = ispif_util_find_stream_in_session(session, stream_id);
  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (sessid = %d, streamid = %d)\n",
      __func__, session_id, stream_id);
    return -1;
  }

  if (session->hal_bundling_mask != 0 &&
      (session->hal_bundling_mask & (1 << stream->stream_idx))) {
    if (session->streamoff_bundling_mask == 0) {
      /* first bundle streamoff */
      session->streamoff_bundling_mask |= (1 << stream->stream_idx);
      rc = ispif_util_get_stream_ids_by_mask(session,
        session->hal_bundling_mask, &num_streams, stream_ids);
    } else {
      session->streamoff_bundling_mask |= (1 << stream->stream_idx);
      goto end;
    }
  } else {
    stream_ids[0] = stream_id;
    num_streams = 1;
  }

  rc = ispif_proc_streamoff(ispif, session, num_streams, stream_ids);
  if (rc == 0) {
    session->active_count -= num_streams;
    CDBG_HIGH("%s: session_id = %d, active_streams = %d\n",
      __func__, session->session_id, session->active_count);
    if (session->active_count == 0) {
      int i;

      /* when there is no active stream running for this session
       * we can loop through it to release isp resource.
       */
      for (i = 0; i < ISP_MAX_STREAMS; i++) {
        if (session->streams[i].stream_id)
          ispif_util_release_isp_resource(ispif, &session->streams[i]);
      }
    }
  }

end:
  if (session->hal_bundling_mask &&
      session->streamoff_bundling_mask == session->hal_bundling_mask) {
    session->hal_bundling_mask = 0;
    session->streamon_bundling_mask = 0;
    session->streamoff_bundling_mask = 0;
  }

  return rc;

}

/** ispif_unlink_sink_port:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @peer_port: peer to which sink port is connected
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unlinks ispif sink port
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream
 **/
int ispif_unlink_sink_port(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id)
{
  int rc = 0;
  ispif_stream_t *stream = NULL;

  stream = ispif_util_find_stream(ispif, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: no stream found. error, sessionid = %d, streamid = %d\n",
      __func__, session_id, stream_id);

    return -1;
  }

  return rc;
}

/** ispif_unlink_src_port:
 *    @ispif: ispif pointer
 *    @ispif_src_port: ispif source port
 *    @peer_port: port to which ispif source port is connected
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unlinks ispif source port
 *
 *  Return:  0 - Success
 *          -1 - cannot find stream
 **/
int ispif_unlink_src_port(ispif_t *ispif, ispif_port_t *ispif_src_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id)
{
  int i, rc = 0, use_rdi = 0, use_pix = 0;
  ispif_stream_t *stream = NULL;
  ispif_src_port_t *src_port = &ispif_src_port->u.src_port;
  ispif_session_t *session = NULL;

  stream = ispif_util_find_stream(ispif, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: error, stream (sessid = %d, streamid = %d, src_port = %p) "
      "not found.\n", __func__, session_id, stream_id, ispif_src_port);

    return -1;
  }
  session = (ispif_session_t *)stream->session;
  /* remove stream's VFE output mask */
  if (session->active_count > 0) {
    /* this code handle the use case taht,
     * when other streams are active, one stream is unlinked.
     * In most common use case ISP resource has been released
     * when the last stream is streamoff. Here we mainly take
     * care of the Rad hoc DI going away while PIX streams
     * still running use case. */
    ispif_util_release_isp_resource(ispif, stream);
  }

  return rc;
}

/** ispif_link_sink_port:
 *    @ispif: pointer to ispif instance
 *    @ispif_port: pointer to ispif sink port
 *    @peer_port: pointer to which sink port will link
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This pointer links ispif sink port with corresponding source port of other
 *  module.
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or
 *               stream not in given sink port or
 *               peer is not matching
 **/
int ispif_link_sink_port(ispif_t *ispif, ispif_port_t *ispif_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id)
{
  int i, rc = 0;
  ispif_stream_t *stream = NULL;

  stream = ispif_util_find_stream(ispif, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find stream, sessioN_id = %d, stream_id = %d\n",
      __func__, session_id, stream_id);

    return -1;
  }

  if (ispif_util_find_stream_in_sink_port(ispif, ispif_port, stream) == FALSE) {
    /* error. stream is not in sink port. */
    CDBG_ERROR("%s: stream (session = %d, stream = %d) not in sink port\n",
      __func__, stream->session_id, stream->stream_id);

    return -1;
  }

  if (ispif_port->port->peer == NULL)
    ispif_port->port->peer = peer_port;
  else {
    if (ispif_port->port->peer != peer_port) {
      CDBG_ERROR("%s: peer port not matching (existing = %p, new = %p\n",
        __func__, ispif_port->port->peer, peer_port);
      rc = -1;
    }
  }
  CDBG("%s: link stream sessionid = %d, streamid = %d to sink port %p\n",
    __func__, session_id, stream_id, ispif_port);

  return rc;
}

/** ispif_link_src_port:
 *    @ispif: pointer to ispif instance
 *    @ispif_port: pointer to ispif source port
 *    @peer_port: pointer to which sink port will link
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This pointer links ispif source port with corresponding sink port of other
 *  module.
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or stream not reserved at source port
 **/
int ispif_link_src_port(ispif_t *ispif, ispif_port_t *ispif_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id)
{
  int rc = 0;
  ispif_stream_t *stream = NULL;

  stream = ispif_util_find_stream(ispif, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find stream, sessioN_id = %d, stream_id = %d\n",
      __func__, session_id, stream_id);

    return -1;
  }

  if (stream->src_port != ispif_port->port) {
    /* stream is not reserved at teh given src port */
    CDBG_ERROR("%s: stream (sessid = %d, streamid = %d, ) "
      "not reserved at src port %p\n", __func__, session_id, stream_id,
      ispif_port);

    return -1;
  }

  if (ispif_port->port->peer == NULL)
    ispif_port->port->peer = peer_port;
  else {
    if (ispif_port->port->peer != peer_port) {
      CDBG_ERROR("%s: peer port not matching (existing = %p, new = %p\n",
        __func__, ispif_port->port->peer, peer_port);
      rc = -1;
    }
  }

  return rc;
}

int ispif_set_fast_aec_mode(
  ispif_t *ispif,
  uint32_t session_id,
  void *data)
{
  int rc = 0;
  ispif_session_t *session = ispif_util_get_session_by_id(ispif, session_id);
  mct_fast_aec_mode_t *fast_aec_mode = data;
  if (session == NULL) {
    CDBG_ERROR("%s: cannot find session %d\n",
      __func__, session_id);
    return -1;
  }
  session->fast_aec_mode = fast_aec_mode->enable;
  if (session->fast_aec_mode)
    session->hal_bundling_mask = 0;
  return rc;
}

/** ispif_set_split_info:
 *    @ispif: ispif pointer
 *    @session_id: session id
 *    @stream_id: stream id
 *    @split_info: structure with split info (dual vfe mode)
 *
 *  This function runs in MCTL thread context.
 *
 *  This function sets image split info to a stream
 *
 *  Return:  0 - Success
 *          -1 - Invalid session or stream ID
 **/
int ispif_set_split_info(ispif_t *ispif, uint32_t session_id,
  uint32_t stream_id, ispif_out_info_t *split_info)
{
  ispif_stream_t *stream = ispif_util_find_stream(
    ispif, session_id, stream_id);
  if (!stream) {
    CDBG_ERROR("%s invalid session/stream ID\n", __func__);

    return -1;
  }

  stream->split_info = *split_info;

  return 0;
}

/** ispif_meta_channel_config:
 *    @ispif: ispif pointer
 *    @session_id: session id
 *    @stream_id: stream id
 *    @ispif_sink_port: ispif sink port
 *
 *  This function runs in MCTL thread context.
 *
 *  This function configures the meta channel
 *
 *  Return:  0 - Success
 *          -1 - cannot find session or RDI reserve error
 **/
int ispif_meta_channel_config(ispif_t *ispif, uint32_t session_id,
  uint32_t stream_id, ispif_port_t *ispif_sink_port)
{
  int i, rc = 0;
  uint32_t vfe_mask;
  ispif_sink_port_t *sink_port = &ispif_sink_port->u.sink_port;
  ispif_session_t *session;

  session = ispif_util_get_session_by_id(ispif,  session_id);
  if(!session) {
    CDBG_ERROR("%s: error: Cannot find session!", __func__);

    return -1;
  }

  if(ispif->meta_info.sensor_meta_info[0].is_valid && (session->num_meta ==0)){
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (sink_port->streams[i] &&
        sink_port->streams[i]->stream_id == stream_id) {
        vfe_mask = 0;
        CDBG("%s: add Meta channel, num_meta= %d, stream id %d\n",
          __func__, ispif->meta_info.num_meta, stream_id);
        sink_port->streams[i]->num_meta = ispif->meta_info.num_meta;
        /* expend to multiple meta on one stream */
        sink_port->streams[i]->meta_info[0] =
          ispif->meta_info.sensor_meta_info[0];
        /*reserve RDI, assume only one meta*/
        rc = reserve_isp_resource(0, TRUE, &sink_port->sensor_cap, NULL, 0, 0,
            0, session->session_idx,
            &sink_port->streams[i]->meta_use_output_mask, &vfe_mask);

        session->rdi_cnt++;
	 session->num_meta++;
        session->vfe_mask |= vfe_mask;
      }
    }
  }

  return rc;
}

/** ispif_sink_port_config:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @stream_idi: stream id
 *    @session_id: session id
 *    @sensor_cfg: semsor configuration
 *
 *  This function configures ispif sink port
 *
 *  Return: 0 for success and negative error on failure
 **/
int ispif_sink_port_config(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t stream_id, uint32_t session_id, sensor_out_info_t *sensor_cfg)
{
  int i, rc = 0;
  ispif_sink_port_t *sink_port = &ispif_sink_port->u.sink_port;
  ispif_stream_t *stream = NULL;
  uint32_t vfe_id_mask = 0;
  /* copy sensor config params */
  sink_port->sensor_cfg = *sensor_cfg;
  CDBG("%s: Sink port start config\n", __func__);
  /* add meta channel to stream*/
  if(!ispif->meta_info.sensor_meta_info[0].is_valid){
    for (i = 0; i < sink_port->num_streams; i++) {
      sink_port->streams[i]->num_meta = 0;
    }
  }

  ispif_util_dump_sensor_cfg(sink_port);

  sensor_cfg->is_retry = FALSE;

  rc = ispif_util_reserve_isp_resource(ispif, ispif_sink_port);
  if (rc < 0) {
    stream = NULL;
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (ispif_sink_port->u.sink_port.streams[i]) {
        stream = ispif_sink_port->u.sink_port.streams[i];
        break;
      }
    }

    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find stream\n", __func__);
      goto end;
    }

    if (!ispif_util_has_pix_resource(sink_port,
        &stream->stream_info)) {
      /* has ISP pixel resource so this is not ISP resource starving case */

      return -ERROR_CODE_ISP_RESOURCE_STARVING;
    } else {
      sensor_cfg->is_retry = TRUE;

      return -EAGAIN;
    }
  }

  /* loop each stream in the same sink port to
   * send stream cfg to isp sink port */
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_sink_port->u.sink_port.streams[i] &&
        ispif_sink_port->u.sink_port.streams[i]->session) {
      stream = ispif_sink_port->u.sink_port.streams[i];
      rc = ispif_util_config_src_port(ispif, ispif_sink_port, stream);
      if (rc < 0) {
        CDBG_ERROR("%s: src_port stream cfg error = %d\n", __func__, rc);
        goto end;
      }
    }
  }

end:
  return rc;
}

/** ispif_set_hal_param:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @stream_idi: stream id
 *    @session_id: session id
 *    @event: MCTL control event
 *
 *  This function runs in MCTL thread context.
 *
 *  This stub function sets hal parameter to ispif module
 *
 *  Return: 0
 **/
int ispif_set_hal_param(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event)
{
  return 0;
}

/** ispif_set_hal_stream_param:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @stream_idi: stream id
 *    @session_id: session id
 *    @event: MCTL control event
 *
 *  This function runs in MCTL thread context.
 *
 *  This function sets hal stream parameter to ispif module
 *
 *  Return:  0 - No error
 *          -1 - Cannot find session or stream
 **/
int ispif_set_hal_stream_param(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event)
{
  int rc = 0;
  cam_stream_parm_buffer_t *param =
    event->u.ctrl_event.control_event_data;

  switch (param->type) {
  case CAM_STREAM_PARAM_TYPE_SET_BUNDLE_INFO:
    rc = ispif_util_set_bundle(ispif, ispif_sink_port,
      session_id, stream_id, &param->bundleInfo);
    break;

  default:
    break;

  } /* switch (param->type) */

  return rc;
}

/** ispif_dual_isp_pip_switch:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @sensor_cfg: sensor configuration
 *
 *  This function runs in MCTL thread context.
 *
 *  This function switches from dual isp mode to PIP mode
 *
 *  Return:  0 - No error
 *          -1 - Cannot find the stream to switch
 **/
int ispif_dual_isp_pip_switch(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  sensor_out_info_t *sensor_cfg)
{
  int i, rc = 0;
  ispif_sink_port_t *sink_port = &ispif_sink_port->u.sink_port;
  ispif_stream_t *stream = NULL;
  uint32_t primary_cid_idx;

  stream = NULL;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_sink_port->u.sink_port.streams[i]) {
      stream = ispif_sink_port->u.sink_port.streams[i];
      break;
    }
  }

  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find stream\n", __func__);

    return -1;
  }

  primary_cid_idx = ispif_util_find_primary_cid(
    &(sink_port->sensor_cfg),
    &(sink_port->sensor_cap));

  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return -1;
  }

  /* there is no VFE pixel interface available. Check if there is one
   * session occupying two VFEs. If yes launch dual
   * VFE to PIP switching logic */
  CDBG_HIGH("%s: trigger dual VFE to PIP switching, "
    "identifity = 0x%x, sensor fmt = %d, stream fmt = %d\n",
    __func__, stream->stream_info.identity,
    sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].fmt,
    stream->stream_info.fmt);
  ispif_util_dual_vfe_to_pip_switching(ispif,
    &sink_port->sensor_cap, stream);

  return rc;
}

/** ispif_resume_pending_session:
 *    @ispif: pointer to ispif instance
 *
 *  This function runs in MCTL thread context.
 *
 *  This function resumes a stopped pending session
 *
 *  Returns: None
 **/
void ispif_resume_pending_session(ispif_t *ispif)
{
  int i;
  ispif_session_t *session = NULL;
  mct_bus_msg_t bus_msg;
  mct_bus_msg_error_message_t err_msg = MCT_ERROR_MSG_RESUME_VFE_STREAMING;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (ispif->sessions[i].ispif != NULL &&
          ispif->sessions[i].need_resume) {
      session = &ispif->sessions[i];
      session->need_resume = FALSE;
      CDBG_HIGH("%s: session %d needs to be resumed\n",
        __func__, session->session_id);
      break;
    }
  }

  if (session) {
    /* send bus msg to  dual isp */
    memset(&bus_msg, 0, sizeof(bus_msg));
    bus_msg.sessionid = session->session_id;
    bus_msg.type = MCT_BUS_MSG_ERROR_MESSAGE;
    bus_msg.size = sizeof(err_msg);
    bus_msg.msg = (void *)&err_msg;
    if (TRUE != mct_module_post_bus_msg(ispif->module, &bus_msg))
      CDBG_ERROR("%s: MCT_BUS_MSG_ERROR_MESSAGE to bus error\n", __func__);
  }
}

/** ispif_need_restore_dual_isp_session:
 *    @ispif: pointer to ispif instance
 *    @session_id: session id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function checks if a session needs to be restored in dual isp mode
 *
 *  Returns: True  - if session needs to be restored in dual isp mode
 *           False - otherwise
 **/
boolean ispif_need_restore_dual_isp_session(ispif_t *ispif, uint32_t session_id)
{
  int i;
  ispif_session_t *session = NULL;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (ispif->sessions[i].session_id == session_id) {
      session = &ispif->sessions[i];
      break;
    }
  }

  if (session && session->trigger_dual_isp_restore) {
    CDBG_HIGH("%s: session %d triggers dual isp session restore process\n",
      __func__, session_id);
    session->trigger_dual_isp_restore = FALSE;
    return TRUE;
  }

  return FALSE;
}

/** ispif_restore_dual_isp_session:
 *    @ispif: pointer to ispif instance
 *
 *  This function runs in MCTL thread context.
 *
 *  This function restores dual isp session
 *
 *  Returns: None
 **/
void ispif_restore_dual_isp_session(ispif_t *ispif)
{
  int i;
  ispif_session_t *session = NULL;
  ispif_stream_t *stream = NULL;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (ispif->sessions[i].ispif != NULL &&
          ispif->sessions[i].dual_single_isp_switching) {
      session = &ispif->sessions[i];
      session->dual_single_isp_switching = FALSE;
      CDBG_HIGH("%s: dual isp session %d needs to be restored\n",
        __func__, session->session_id);
      break;
    }
  }

  if (session) {
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (session->streams[i].stream_id) {
        stream = &session->streams[i];
        break;
      }
    }

    if (stream) {
      mct_event_t mct_event;
      uint32_t op_pix_clk = session->high_op_pix_clk;
      mct_bus_msg_t bus_msg;
      mct_bus_msg_error_message_t err_msg = MCT_ERROR_MSG_RSTART_VFE_STREAMING;

      memset(&mct_event, 0, sizeof(mct_event));
      mct_event.identity = stream->stream_info.identity;
      mct_event.type = MCT_EVENT_MODULE_EVENT;
      mct_event.direction = MCT_EVENT_UPSTREAM;
      mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_CHANGE_OP_PIX_CLK;
      mct_event.u.module_event.module_event_data = (void *)&op_pix_clk;
      mct_port_send_event_to_peer(stream->sink_port->peer, &mct_event);

      /* send bus msg to  dual isp */
      memset(&bus_msg, 0, sizeof(bus_msg));
      bus_msg.sessionid = session->session_id;
      bus_msg.type = MCT_BUS_MSG_ERROR_MESSAGE;
      bus_msg.size = sizeof(err_msg);
      bus_msg.msg = (void *)&err_msg;
      if (TRUE != mct_module_post_bus_msg(ispif->module, &bus_msg))
        CDBG_ERROR("%s: MCT_BUS_MSG_ERROR_MESSAGE to bus error\n", __func__);
    }
  }
}

