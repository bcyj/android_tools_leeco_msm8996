/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file implements the media/module/master controller's focus logic in
   the mm-camera server. The functionalities of this modules are:

   1. config, process/parse awb/aec stats buffers and events
   2. control the statsproc interface
============================================================================*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <media/msm_isp.h>

#include "camera_dbg.h"
#include "camera.h"
#include "tgtcommon.h"
#include "vfe.h"

#ifdef ENABLE_STATS_PARSER_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_stats_req_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_req_buf(int fd, int num_bufs, int stats_type, int reg_unreg)
{
  int rc = 0;
  struct msm_stats_reqbuf reqbuf;
  CDBG("%s: stats_type: %d, num_bufs : %d, reg_unreg : %d",
    __func__,stats_type, num_bufs, reg_unreg);

  /* If flag is REGBUF, do REQBUF's only*/
  if(reg_unreg == VFE_STATS_REGBUF) {
    reqbuf.num_buf = num_bufs;
    reqbuf.stats_type = stats_type;
        rc = ioctl(fd, MSM_CAM_IOCTL_STATS_REQBUF, &reqbuf);
        if(rc < 0) {
          CDBG_ERROR("%s: error = %d, stats_type = %d, num_bufs = %d",
                                 __func__, rc, stats_type, num_bufs);
          return rc;
        }
        /* If flag is UNREGBUF, do UNREGBUF and REQBUF's only*/
  } else if(reg_unreg == VFE_STATS_UNREGBUF) {
    reqbuf.num_buf = num_bufs;
    reqbuf.stats_type = stats_type;
        /* calls buf_unprepare to unmap buffers*/
        rc = ioctl(fd, MSM_CAM_IOCTL_STATS_UNREG_BUF, &reqbuf);
        if(rc < 0) {
          CDBG_ERROR("%s: error = %d, stats_type = %d, num_bufs = %d",
                                 __func__, rc, stats_type, num_bufs);
          return rc;
        }
        /* numbufs = 0 relases the stats buff containers*/
        reqbuf.num_buf = 0;
        rc = ioctl(fd, MSM_CAM_IOCTL_STATS_REQBUF, &reqbuf);
        if(rc < 0) {
          CDBG_ERROR("%s: error = %d, stats_type = %d, num_bufs = %d",
        __func__, rc, stats_type, num_bufs);
          return rc;
        }
  } else
    CDBG_ERROR(":%s: should not come here\n", __func__);

  return rc;
}
/*===========================================================================
 * FUNCTION    - vfe_stats_flush_bufq -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_flush_bufq(int fd, int stats_type)
{
  int rc = 0;
  struct msm_stats_flush_bufq flushbuf;
  flushbuf.stats_type = stats_type;
  rc = ioctl(fd, MSM_CAM_IOCTL_STATS_FLUSH_BUFQ, &flushbuf);
  if(rc < 0) {
    CDBG_ERROR("%s: error = %d, stats_type = %d",
               __func__, rc, stats_type);
  }
  return rc;
}
/*===========================================================================
 * FUNCTION    - vfe_stats_enqueuebuf -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_enqueuebuf(int fd, int stats_type, int buf_idx,
  mem_buffer_struct_t *memBuf)
{
  CDBG("%s: stats_type : %d, buf_idx :%d\n", __func__, stats_type, buf_idx);
  int rc = 0;
  struct msm_stats_buf_info qbuf;
  memset(&qbuf, 0, sizeof(struct msm_stats_buf_info));
  qbuf.type = stats_type;
  qbuf.fd = memBuf->fd;
  qbuf.vaddr = memBuf->buf;
  qbuf.len = memBuf->ion_alloc.len;
  qbuf.active = 1;
  qbuf.buf_idx = buf_idx;
  rc = ioctl(fd, MSM_CAM_IOCTL_STATS_ENQUEUEBUF, &qbuf);
  if (rc < 0)
    CDBG_ERROR("%s: rc = %d, stats_type = %d, idx = %d, vaddr = %p, size = %d",
           __func__, rc, stats_type, buf_idx, qbuf.vaddr, qbuf.len);
  return rc;
}

/*===========================================================================
 * FUNCTION    - vfe_reg_unreg_pmem_buff -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_reg_unreg_pmem_buff(int type, int camfd, int stats_fd,
  uint8_t *buf, int cmd)
{
  struct msm_pmem_info pmemBuf;
  int32_t rc = TRUE;

  /* register the PMEM to kernel */
  memset(&pmemBuf, 0, sizeof(pmemBuf));
  pmemBuf.type   = type;
  pmemBuf.fd     = stats_fd;
  pmemBuf.vaddr  = buf;
  pmemBuf.active = TRUE;

  rc = ioctl(camfd, cmd, &pmemBuf);
  if (rc < 0) {
    CDBG_ERROR("%s : ioctl failed. cmd : %d rc = %d\n", __func__, cmd, rc);
  }
  return rc;
}
/*===========================================================================
 * FUNCTION    - vfe_stats_parser_get_buf_ptr -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_stats_parser_get_buf_ptr(stats_parser_mod_t *parser_mod,
        vfe_params_t *params, stats_buffers_type_t **pp_stats_bufs)
{
  *pp_stats_bufs = &parser_mod->stats_bufs;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_bayer_stats_buffer_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int  vfe_bayer_stats_buffer_init(void *ctrl)
{
  uint8_t cnt;
  int32_t rc;
  uint32_t awb_buf_size = 0, af_buf_size = 0;
  uint32_t buf_size;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  int fd = p_obj->vfe_params.camfd;
  uint32_t *stats_marker =
    &(p_obj->vfe_module.stats.parser_mod.stats_marker);
  bayer_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);

  CDBG("%s: E %d \n", __func__, *stats_marker);
    /* allocate PMEM buffer for BE stats*/
  rc = vfe_stats_req_buf(fd, BE_STATS_BUFNUM, MSM_STATS_TYPE_BE,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < BE_STATS_BUFNUM; cnt++) {
    buf_size = (VFE_BAYER_EXPOSURE_BUF_SIZE * sizeof(uint32_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->BeBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: BE get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("Bayer Exposure stats buf = 0x%p, fd = %d\n",
      stats_buf_data->BeBuf[cnt].buf, stats_buf_data->BeBuf[cnt].fd);

    if (!(stats_buf_data->BeBuf[cnt].buf)) {
      /* Do we need to release previous buffers? */
      CDBG_ERROR("%s : BE failed \n", __func__);
      goto ERROR;
    }
    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_BE, cnt,
           &(stats_buf_data->BeBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */
  *stats_marker |= STATS_BE;

  /* allocate PMEM buffer for BG stats*/
  rc = vfe_stats_req_buf(fd, BG_STATS_BUFNUM, MSM_STATS_TYPE_BG,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < BG_STATS_BUFNUM; cnt++) {
    buf_size = (VFE_BAYER_GRID_BUF_SIZE * sizeof(uint32_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->BgBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: BG get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("Bayer Grid stats buf = 0x%p, fd = %d\n",
      stats_buf_data->BgBuf[cnt].buf, stats_buf_data->BgBuf[cnt].fd);

    if (!(stats_buf_data->BgBuf[cnt].buf)) {
      /* Do we need to release previous buffers? */
      CDBG_ERROR("%s : BG failed \n", __func__);
      goto ERROR;
    }
    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_BG, cnt,
           &(stats_buf_data->BgBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */
  *stats_marker |= STATS_BG;

  /* allocate PMEM buffer for BF stats*/
  rc = vfe_stats_req_buf(fd, BF_STATS_BUFNUM, MSM_STATS_TYPE_BF,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < BF_STATS_BUFNUM; cnt++) {
    buf_size = (VFE_BAYER_FOCUS_BUF_SIZE * sizeof(uint32_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->BfBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: BF get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("Bayer Focus stats buf = 0x%p, fd = %d\n",
      stats_buf_data->BfBuf[cnt].buf, stats_buf_data->BfBuf[cnt].fd);

    if (!(stats_buf_data->BfBuf[cnt].buf)) {
      /* Do we need to release previous buffers? */
      CDBG_ERROR("%s : BF failed \n", __func__);
      goto ERROR;
    }
    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_BF, cnt,
           &(stats_buf_data->BfBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */
  *stats_marker |= STATS_BF;

  /* allocate PMEM buffer for BG stats*/
  rc = vfe_stats_req_buf(fd, BHIST_STATS_BUFNUM, MSM_STATS_TYPE_BHIST,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < BHIST_STATS_BUFNUM; cnt++) {
    buf_size = (VFE_BAYER_HIST_BUF_SIZE * sizeof(uint32_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->BhistBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: BHIST get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("Bayer Hist stats buf = 0x%p, fd = %d\n",
      stats_buf_data->BhistBuf[cnt].buf, stats_buf_data->BhistBuf[cnt].fd);

    if (!(stats_buf_data->BhistBuf[cnt].buf)) {
      /* Do we need to release previous buffers? */
      CDBG_ERROR("%s : BF failed \n", __func__);
      goto ERROR;
    }
    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_BHIST, cnt,
           &(stats_buf_data->BhistBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */
  *stats_marker |= STATS_BHIST;

  CDBG("%s: stats_marker : %d X\n", __func__, *stats_marker);
  return VFE_SUCCESS;
ERROR:
  vfe_stats_buffer_free(ctrl);
  return TRUE;
} // vfe_bayer_stats_buffer_init

/*===========================================================================
 * FUNCTION    - vfe_legacy_stats_buffer_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_legacy_stats_buffer_init(void *ctrl)
{
  uint8_t cnt;
  int32_t rc;
  uint32_t awb_buf_size = 0, af_buf_size = 0;
  uint32_t buf_size;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  int fd = p_obj->vfe_params.camfd;
  uint32_t *stats_marker =
    &(p_obj->vfe_module.stats.parser_mod.stats_marker);
  legacy_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.legacy_stats_buf);

#ifdef VFE_2X
  /* allocate PMEM buffer for AWB and AEC stats*/
  rc = vfe_stats_req_buf(fd, AWB_AEC_STATS_BUFNUM, MSM_STATS_TYPE_AE_AW,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  /* allocate PMEM buffer for AWB and AEC stats*/
  for (cnt = 0; cnt < AWB_AEC_STATS_BUFNUM; cnt++) {

    buf_size = (VFE2X_AWB_AEC_BUF_SIZE * sizeof(uint32_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->AwbAecBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: AWB_AEC get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("AWB AEC stats buf = 0x%p, fd = %d\n",
      stats_buf_data->AwbAecBuf[cnt].buf, stats_buf_data->AwbAecBuf[cnt].fd);

    if (!(stats_buf_data->AwbAecBuf[cnt].buf)) {
      /* Do we need to release previous buffers? */
      CDBG_ERROR("%s : AWB_AEC failed \n", __func__);
      goto ERROR;
    }
    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_AE_AW, cnt,
           &(stats_buf_data->AwbAecBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */
  *stats_marker |= STATS_AEAW;
#else
  /* allocate PMEM buffer for AEC stats*/
  CDBG_ERROR("%s: AEC_STATS_BUFNUM", __func__);
  rc = vfe_stats_req_buf(fd, AEC_STATS_BUFNUM, MSM_STATS_TYPE_AEC,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < AEC_STATS_BUFNUM; cnt++) {

    buf_size = (VFE32_AEC_BUF_SIZE * sizeof(uint32_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->AecBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: AEC get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("AEC stats buf = 0x%p, fd = %d\n", stats_buf_data->AecBuf[cnt].buf,
      stats_buf_data->AecBuf[cnt].fd);

    if (!(stats_buf_data->AecBuf[cnt].buf)) {
      /* Do we need to release previous buffers? */
      CDBG_ERROR("%s : AEC Buff failed\n", __func__);
      goto ERROR;
    }
    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_AEC, cnt,
           &(stats_buf_data->AecBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */
  *stats_marker |= STATS_AEC;
  CDBG_ERROR("%s: AEC_STATS_BUFNUM", __func__);

#endif /* END of OTHER THAN VFE_2X */

  /* allocate PMEM buffer for AF  stats*/
  CDBG("allocate PMEM buffer for AF stats\n");
#ifdef VFE_2X
  af_buf_size = VFE2X_AF_BUF_SIZE;
#else
  af_buf_size = VFE32_AF_BUF_SIZE;
#endif

  CDBG("%s: AF buf size %d", __func__, af_buf_size);
  rc = vfe_stats_req_buf(fd, AF_STATS_BUFNUM, MSM_STATS_TYPE_AF,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < AF_STATS_BUFNUM; cnt++) {

    buf_size = (af_buf_size * sizeof(uint32_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->AfBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: AF get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("AF stats buf = 0x%p, fd = %d\n", stats_buf_data->AfBuf[cnt].buf,
      stats_buf_data->AfBuf[cnt].fd);

    if (!(stats_buf_data->AfBuf[cnt].buf)) {
      /* Do we need to release previous buffers? */
      CDBG_ERROR("%s : AF Buff failed\n", __func__);
      goto ERROR;
    }
    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_AF, cnt,
           &(stats_buf_data->AfBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  }
  *stats_marker |= STATS_AF;

  CDBG("%s: stats_marker : %d X\n", __func__, *stats_marker);
  return VFE_SUCCESS;
ERROR:
  vfe_stats_buffer_free(ctrl);
  return FALSE;
} // vfe_legacy_stats_buffer_init

/*===========================================================================
 * FUNCTION    - vfe_common_stats_buffer_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_common_stats_buffer_init(void *ctrl)
{
  uint8_t cnt;
  int32_t rc;
  uint32_t awb_buf_size = 0;
  uint32_t buf_size;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  int fd = p_obj->vfe_params.camfd;
  uint32_t *stats_marker =
    &(p_obj->vfe_module.stats.parser_mod.stats_marker);
  common_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);

  /* allocate PMEM buffer for IHIST stats */
  rc = vfe_stats_req_buf(fd, IHIST_STATS_BUFNUM, MSM_STATS_TYPE_IHIST,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < IHIST_STATS_BUFNUM; cnt++) {

    buf_size = (VFE32_IHIST_BUF_SIZE * sizeof(uint32_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->IhistBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: IHIST get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("IHIST stats buf = 0x%p, fd = %d\n", stats_buf_data->IhistBuf[cnt].buf,
      stats_buf_data->IhistBuf[cnt].fd);
    if (!(stats_buf_data->IhistBuf[cnt].buf)) {
      CDBG_ERROR("%s : IHIST Buff failed\n", __func__);
      goto ERROR;
    }

    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_IHIST, cnt,
           &(stats_buf_data->IhistBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */

  *stats_marker |= STATS_IHIST;

  /* allocate PMEM buffer for CS stats */
  rc = vfe_stats_req_buf(fd, CS_STATS_BUFNUM, MSM_STATS_TYPE_CS,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < CS_STATS_BUFNUM; cnt++) {

    buf_size = (VFE32_CS_BUF_SIZE * sizeof(uint16_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->CSBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: CS get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("CS stats buf = 0x%p, fd = %d\n", stats_buf_data->CSBuf[cnt].buf,
      stats_buf_data->CSBuf[cnt].fd);

    if (!(stats_buf_data->CSBuf[cnt].buf)) {
      CDBG_ERROR("%s : CS Buff failed\n", __func__);
      goto ERROR;
    }

    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_CS, cnt,
           &(stats_buf_data->CSBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */

  *stats_marker |= STATS_CS;

  /* allocate PMEM buffer for RS stats */
  rc = vfe_stats_req_buf(fd, RS_STATS_BUFNUM, MSM_STATS_TYPE_RS,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < RS_STATS_BUFNUM; cnt++) {
    buf_size = (VFE32_RS_BUF_SIZE * sizeof(uint16_t));

    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->RSBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: RS get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("RS stats buf = 0x%p, fd = %d\n", stats_buf_data->RSBuf[cnt].buf,
      stats_buf_data->RSBuf[cnt].fd);

    if (!(stats_buf_data->RSBuf[cnt].buf)) {
      CDBG_ERROR("%s : RS Buff failed\n", __func__);
      goto ERROR;
    }

    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_RS, cnt,
           &(stats_buf_data->RSBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */
  *stats_marker |= STATS_RS;


#ifdef VFE_31
  awb_buf_size = VFE31_AWB_BUF_SIZE;
#else
  awb_buf_size = VFE32_AWB_BUF_SIZE;
#endif

  CDBG("%s: AWB buf size %d", __func__, awb_buf_size);
  /* allocate PMEM buffer for AWB stats*/
  rc = vfe_stats_req_buf(fd, AWB_STATS_BUFNUM, MSM_STATS_TYPE_AWB,
    VFE_STATS_REGBUF);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    goto ERROR;
  }
  for (cnt = 0; cnt < AWB_STATS_BUFNUM; cnt++) {
    buf_size = (awb_buf_size * sizeof(uint32_t));
    if(p_obj->ops->get_mem_buffer) {
        p_obj->ops->get_mem_buffer(p_obj->ops->parent,
          &(stats_buf_data->AwbBuf[cnt]), buf_size);
    } else {
        CDBG_HIGH("%s: AWB get mem error\n", __func__);
        goto ERROR;
    }

    CDBG("AWB stats buf = 0x%p, fd = %d\n", stats_buf_data->AwbBuf[cnt].buf,
      stats_buf_data->AwbBuf[cnt].fd);

    if (!(stats_buf_data->AwbBuf[cnt].buf)) {
      /* Do we need to release previous buffers? */
      CDBG_ERROR("%s : AWB Buf failed\n", __func__);
      goto ERROR;
    }
    rc = vfe_stats_enqueuebuf(fd, MSM_STATS_TYPE_AWB, cnt,
           &(stats_buf_data->AwbBuf[cnt]));
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      goto ERROR;
    }
  } /* for */
  *stats_marker |= STATS_AWB;

  CDBG("%s: stats_marker : %d X\n", __func__, *stats_marker);
  return VFE_SUCCESS;

ERROR:
  vfe_stats_buffer_free(ctrl);
  return FALSE;
} /* vfe_common_stats_buffer_init */

/*===========================================================================
 * FUNCTION    - vfe_legacy_stats_buffer_free -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_legacy_stats_buffer_free(void *ctrl)
{
  uint8_t cnt;
  int32_t rc;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  //TODO: need to add check for the type of buf
  legacy_stats_buffer_t *legacy_stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.legacy_stats_buf);
  uint32_t stats_marker =
    p_obj->vfe_module.stats.parser_mod.stats_marker;

  int fd = p_obj->vfe_params.camfd;
  CDBG("%s: E stats_marker : %d\n", __func__, stats_marker);

#ifdef VFE_2X

  CDBG("deallocate PMEM buffer for AEC AWB stats\n");
  if (stats_marker & STATS_AEAW) {
    rc = vfe_stats_req_buf(fd, AWB_AEC_STATS_BUFNUM, MSM_STATS_TYPE_AE_AW,
      VFE_STATS_UNREGBUF);

    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }
    for (cnt = 0; cnt < AWB_AEC_STATS_BUFNUM; cnt++) {
      if(p_obj->ops->put_mem_buffer) {
        rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
               &(legacy_stats_buf_data->AwbAecBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: AEC_AWB put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: AEC_AWB put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_AEAW

#else /*  Other than VFE_2X */

  CDBG("deallocate PMEM buffer for AEC stats\n");

  if (stats_marker & STATS_AEC) {
    rc = vfe_stats_req_buf(fd, AEC_STATS_BUFNUM, MSM_STATS_TYPE_AEC,
      VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }
    for (cnt = 0; cnt < AEC_STATS_BUFNUM; cnt++) {
        if(p_obj->ops->put_mem_buffer) {
        rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
               &(legacy_stats_buf_data->AecBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: AEC put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: AEC put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_AEC
#endif

  CDBG("deallocate PMEM buffer for AF stats\n");
  if (stats_marker & STATS_AF) {
    rc = vfe_stats_req_buf(fd, AF_STATS_BUFNUM, MSM_STATS_TYPE_AF,
      VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }
    for (cnt = 0; cnt < AF_STATS_BUFNUM; cnt++) {
      if(p_obj->ops->put_mem_buffer) {
          rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
                 &(legacy_stats_buf_data->AfBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: AF put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: AF put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_AF
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_common_stats_buffer_free -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_common_stats_buffer_free(void *ctrl)
{
  uint8_t cnt;
  int32_t rc;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  //TODO: need to add check for the type of buf
  common_stats_buffer_t *common_stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);
  uint32_t stats_marker =
    p_obj->vfe_module.stats.parser_mod.stats_marker;

  int fd = p_obj->vfe_params.camfd;
  CDBG("%s: E stats_marker : %d\n", __func__, stats_marker);

  CDBG("deallocate PMEM buffer for IHIST stats\n");
  if (stats_marker & STATS_IHIST) {
    rc = vfe_stats_req_buf(fd, IHIST_STATS_BUFNUM, MSM_STATS_TYPE_IHIST,
      VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }
    for (cnt = 0; cnt < IHIST_STATS_BUFNUM; cnt++) {
        if(p_obj->ops->put_mem_buffer) {
        rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
               &(common_stats_buf_data->IhistBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: IHIST put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: IHIST put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_IHIST

  CDBG("deallocate PMEM buffer for CS stats\n");
  if (stats_marker & STATS_CS) {
    rc = vfe_stats_req_buf(fd, CS_STATS_BUFNUM, MSM_STATS_TYPE_CS,
      VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }

    for (cnt = 0; cnt < CS_STATS_BUFNUM; cnt++) {
      if(p_obj->ops->put_mem_buffer) {
        rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
               &(common_stats_buf_data->CSBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: CS put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: CS put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_CS

  CDBG("deallocate PMEM buffer for RS stats\n");
  if (stats_marker & STATS_RS) {
    rc = vfe_stats_req_buf(fd, RS_STATS_BUFNUM, MSM_STATS_TYPE_RS,
      VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }
    for (cnt = 0; cnt < RS_STATS_BUFNUM; cnt++) {
      if(p_obj->ops->put_mem_buffer) {
        rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
               &(common_stats_buf_data->RSBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: RS put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: RS put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_RS

  CDBG("deallocate PMEM buffer for AWB stats\n");
  if (stats_marker & STATS_AWB) {
    rc = vfe_stats_req_buf(fd, AWB_STATS_BUFNUM, MSM_STATS_TYPE_AWB,
      VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }
    for (cnt = 0; cnt < AWB_STATS_BUFNUM; cnt++) {
        if(p_obj->ops->put_mem_buffer) {
        rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
               &(common_stats_buf_data->AwbBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: AWB put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: AWB put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_AWB

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_bayer_stats_buffer_free -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bayer_stats_buffer_free(void *ctrl)
{
  uint8_t cnt;
  int32_t rc;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  //TODO: need to add check for the type of buf
  bayer_stats_buffer_t *bayer_stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  uint32_t stats_marker =
    p_obj->vfe_module.stats.parser_mod.stats_marker;
  int fd = p_obj->vfe_params.camfd;

  CDBG("%s: E stats_marker : %d\n", __func__, stats_marker);

  CDBG("deallocate PMEM buffer for BG stats\n");
  if (stats_marker & STATS_BG) {
    rc = vfe_stats_req_buf(fd, BG_STATS_BUFNUM, MSM_STATS_TYPE_BG,
      VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }
    for (cnt = 0; cnt < BG_STATS_BUFNUM; cnt++) {
      if(p_obj->ops->put_mem_buffer) {
          rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
                 &(bayer_stats_buf_data->BgBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: BG put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: BG put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_BG

  CDBG("deallocate PMEM buffer for BE stats\n");
  if (stats_marker & STATS_BE) {
    rc = vfe_stats_req_buf(fd, BE_STATS_BUFNUM, MSM_STATS_TYPE_BE,
      VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }
    for (cnt = 0; cnt < BE_STATS_BUFNUM; cnt++) {
      if(p_obj->ops->put_mem_buffer) {
          rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
                 &(bayer_stats_buf_data->BeBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: BE put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: BE put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_BE

  CDBG("deallocate PMEM buffer for BF stats\n");
  if (stats_marker & STATS_BF) {
    rc = vfe_stats_req_buf(fd, BF_STATS_BUFNUM, MSM_STATS_TYPE_BF,
	  VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }

    for (cnt = 0; cnt < BF_STATS_BUFNUM; cnt++) {
      if(p_obj->ops->put_mem_buffer) {
          rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
                 &(bayer_stats_buf_data->BfBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: BF put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: BF put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_BF

  CDBG("deallocate PMEM buffer for BHIST stats\n");
  if (stats_marker & STATS_BHIST) {
    rc = vfe_stats_req_buf(fd, BHIST_STATS_BUFNUM, MSM_STATS_TYPE_BHIST,
      VFE_STATS_UNREGBUF);
    if (rc < 0) {
      CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
      return FALSE;
    }

    for (cnt = 0; cnt < BHIST_STATS_BUFNUM; cnt++) {
      if(p_obj->ops->put_mem_buffer) {
          rc = p_obj->ops->put_mem_buffer(p_obj->ops->parent,
                 &(bayer_stats_buf_data->BhistBuf[cnt]));
        if (rc < 0) {
          CDBG_ERROR("%s: BHIST put mem failed", __func__);
        }
      } else {
          CDBG_HIGH("%s: BHIST put mem error\n", __func__);
          return FALSE;
      }
    }
  } //stats_marker & STATS_BHIST

  CDBG("%s: X\n", __func__);
  return VFE_SUCCESS;
} /* vfe_stats_buffer_free */

/*===========================================================================
 * FUNCTION    - vfe_stats_buffer_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_stats_buffer_init(void *ctrl)
{
  vfe_status_t rc = VFE_SUCCESS;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  CDBG("%s: E\n", __func__);

  p_obj->vfe_module.stats.parser_mod.stats_marker = 0;

  if(p_obj->vfe_module.stats.use_bayer_stats) {
    rc = vfe_bayer_stats_buffer_init(ctrl);
    if (rc != VFE_SUCCESS) {
      CDBG_ERROR("%s: Bayer stats buffer init Failed\n",__func__);
      return rc;
    }
  } else {
    rc = vfe_legacy_stats_buffer_init(ctrl);
    if (rc != VFE_SUCCESS) {
      CDBG_ERROR("%s: Legacy stats buffer init Failed\n",__func__);
      return rc;
    }
  }
#ifndef VFE_2X
  rc = vfe_common_stats_buffer_init(ctrl);
  if (rc != VFE_SUCCESS) {
    CDBG_ERROR("%s: Common stats buffer init Failed\n",__func__);
    return rc;
  }
#endif
  return rc;
}

/*===========================================================================
 * FUNCTION    - vfe_stats_buffer_free -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_stats_buffer_free(void *ctrl)
{
  vfe_status_t rc = VFE_SUCCESS;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  uint32_t *stats_marker =
    &(p_obj->vfe_module.stats.parser_mod.stats_marker);
  CDBG("%s: E\n", __func__);

  if(p_obj->vfe_module.stats.use_bayer_stats) {
    rc = vfe_bayer_stats_buffer_free(ctrl);
    if (rc != VFE_SUCCESS) {
      CDBG_ERROR("%s: Bayer stats buffer free Failed\n",__func__);
      return rc;
    }
  } else {
    rc = vfe_legacy_stats_buffer_free(ctrl);
    if (rc != VFE_SUCCESS) {
      CDBG_ERROR("%s: Legacy stats buffer free Failed\n",__func__);
      return rc;
    }
  }
#ifndef VFE_2X
  rc = vfe_common_stats_buffer_free(ctrl);
  if (rc != VFE_SUCCESS) {
    CDBG_ERROR("%s: Common stats buffer free Failed\n",__func__);
    return rc;
  }
#endif
  return rc;
}

/*==============================================================================
 * FUNCTION    - vfe_stats_release_buf -
 *
 * DESCRIPTION:
 *============================================================================*/
vfe_status_t vfe_stats_release_buf(vfe_ctrl_info_t *p_obj, int buf_type,
  int buf_idx, void *parm)
{
  int rc = 0;
  struct msm_stats_buf *release_buf = (struct msm_stats_buf *)parm;
  mem_buffer_struct_t cache_buf;

  CDBG("%s: releasing STATS buffer type %d buf = 0x%lx, fd: %d\n", __func__, buf_type,
    release_buf->buffer, release_buf->fd);
  release_buf->type = buf_type;

  memset(&cache_buf, 0, sizeof(mem_buffer_struct_t));
  cache_buf.fd = release_buf->fd;
  cache_buf.buf = (uint8_t *)release_buf->buffer;
  cache_buf.ion_alloc.handle = release_buf->handle;
  cache_buf.ion_alloc.len = release_buf->length;

  if(p_obj->ops->cache_invalidate) {
    rc = p_obj->ops->cache_invalidate(p_obj->ops->parent, &cache_buf);
    if (rc < 0) {
      CDBG_ERROR("%s: %d, cache_invalidate failed \n", __func__, buf_type);
      return rc;
    }
  } else {
      CDBG_HIGH("%s: %d, cache_invalidate error\n", __func__, buf_type);
      return FALSE;
  }
  rc = vfe_stats_enqueuebuf(p_obj->vfe_params.camfd, buf_type, buf_idx,
         &cache_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: Line: %d failed", __func__, __LINE__);
    return rc;
  }
  return VFE_SUCCESS;
} /* vfe_stats_release_buf */

/*===========================================================================
 * FUNCTION    - vfe_release_all_stats_buff -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_release_all_stats_buff(vfe_ctrl_info_t *p_obj)
{
  struct msm_stats_buf release_buf;
  legacy_stats_buffer_t *legacy_stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.legacy_stats_buf);
  common_stats_buffer_t *common_stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);
  bayer_stats_buffer_t *bayer_stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  uint32_t stats_marker =
    p_obj->vfe_module.stats.parser_mod.stats_marker;
  int i =0, rc =0;

  CDBG("%s: E : stats_marker: %d \n", __func__, stats_marker);

#ifdef VFE_2X
  if (stats_marker & STATS_AEAW) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd,
           MSM_STATS_TYPE_AE_AW);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }
    for (i = 0; i < AWB_AEC_STATS_BUFNUM; ++i) {
      legacy_stats_buf_data->cur_AecBuf = i;
      release_buf.type = STAT_AEAW;
      release_buf.buffer =
        (unsigned long) legacy_stats_buf_data->AwbAecBuf[i].buf;
      release_buf.fd = legacy_stats_buf_data->AwbAecBuf[i].fd;
      release_buf.handle = legacy_stats_buf_data->AwbAecBuf[i].fd_data.handle;
      release_buf.length = legacy_stats_buf_data->AwbAecBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_AE_AW, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  } //*stats_marker | STATS_AEAW
#else

  if (stats_marker & STATS_AEC) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd,
      MSM_STATS_TYPE_AEC);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }
    for (i = 0; i < AEC_STATS_BUFNUM; ++i) {
      legacy_stats_buf_data->cur_AecBuf = i;
      release_buf.type = STAT_AEC;
      release_buf.buffer =
        (unsigned long) legacy_stats_buf_data->AecBuf[i].buf;
      release_buf.fd = legacy_stats_buf_data->AecBuf[i].fd;
      release_buf.handle = legacy_stats_buf_data->AecBuf[i].fd_data.handle;
      release_buf.length = legacy_stats_buf_data->AecBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_AEC, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  }// *stats_marker | STATS_AEC

  if (stats_marker & STATS_AWB) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd,
      MSM_STATS_TYPE_AWB);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }

    for (i = 0; i < AWB_STATS_BUFNUM; ++i) {
      common_stats_buf_data->cur_AwbBuf = i;
      release_buf.type = STAT_AWB;
      release_buf.buffer =
        (unsigned long) common_stats_buf_data->AwbBuf[i].buf;
      release_buf.fd = common_stats_buf_data->AwbBuf[i].fd;
      release_buf.handle = common_stats_buf_data->AwbBuf[i].fd_data.handle;
      release_buf.length = common_stats_buf_data->AwbBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_AWB, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  }// *stats_marker | STATS_AWB

  if (stats_marker & STATS_IHIST) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd, MSM_STATS_TYPE_IHIST);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }

    for (i = 0; i < IHIST_STATS_BUFNUM; ++i) {
      common_stats_buf_data->cur_IhistBuf = i;
      release_buf.type = STAT_IHIST;
      release_buf.buffer =
        (unsigned long) common_stats_buf_data->IhistBuf[i].buf;
      release_buf.fd = common_stats_buf_data->IhistBuf[i].fd;
      release_buf.handle = common_stats_buf_data->IhistBuf[i].fd_data.handle;
      release_buf.length = common_stats_buf_data->IhistBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_IHIST, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  }

  if (stats_marker & STATS_RS) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd, MSM_STATS_TYPE_RS);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }

    for (i = 0; i < RS_STATS_BUFNUM; ++i) {
      common_stats_buf_data->cur_RSBuf = i;
      release_buf.type = STAT_RS;
      release_buf.buffer =
        (unsigned long) common_stats_buf_data->RSBuf[i].buf;
      release_buf.fd = common_stats_buf_data->RSBuf[i].fd;
      release_buf.handle = common_stats_buf_data->RSBuf[i].fd_data.handle;
      release_buf.length = common_stats_buf_data->RSBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_RS, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  }// *stats_marker | STATS_RS

  if (stats_marker & STATS_CS) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd, MSM_STATS_TYPE_CS);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }

    for (i = 0; i < CS_STATS_BUFNUM; ++i) {
      common_stats_buf_data->cur_CSBuf = i;
      release_buf.type = STAT_CS;
      release_buf.buffer =
        (unsigned long) common_stats_buf_data->CSBuf[i].buf;
      release_buf.fd = common_stats_buf_data->CSBuf[i].fd;
      release_buf.handle = common_stats_buf_data->CSBuf[i].fd_data.handle;
      release_buf.length = common_stats_buf_data->CSBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_CS, i,
           (void *)&release_buf);
      if (rc < 0) {
        CDBG("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  } // *stats_marker | STATS_CS

#endif

  if (stats_marker & STATS_AF) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd, MSM_STATS_TYPE_AF);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }

    for (i = 0; i < AF_STATS_BUFNUM; ++i) {
      legacy_stats_buf_data->cur_AFBuf = i;
      release_buf.type = STAT_AF;
      release_buf.buffer =
        (unsigned long) legacy_stats_buf_data->AfBuf[i].buf;
      release_buf.fd = legacy_stats_buf_data->AfBuf[i].fd;
      release_buf.handle = legacy_stats_buf_data->AfBuf[i].fd_data.handle;
      release_buf.length = legacy_stats_buf_data->AfBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_AF, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  }

    if (stats_marker & STATS_BE) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd,
      MSM_STATS_TYPE_BE);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }
    for (i = 0; i < BE_STATS_BUFNUM; ++i) {
      bayer_stats_buf_data->cur_bg_idx = i;
      release_buf.type = STAT_BE;
      release_buf.buffer = (unsigned long) bayer_stats_buf_data->BeBuf[i].buf;
      release_buf.fd = bayer_stats_buf_data->BeBuf[i].fd;
      release_buf.handle = bayer_stats_buf_data->BeBuf[i].fd_data.handle;
      release_buf.length = bayer_stats_buf_data->BeBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_BE, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  }

  if (stats_marker & STATS_BG) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd,
      MSM_STATS_TYPE_BG);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }
    for (i = 0; i < BG_STATS_BUFNUM; ++i) {
      bayer_stats_buf_data->cur_bg_idx = i;
      release_buf.type = STAT_BG;
      release_buf.buffer = (unsigned long) bayer_stats_buf_data->BgBuf[i].buf;
      release_buf.fd = bayer_stats_buf_data->BgBuf[i].fd;
      release_buf.handle = bayer_stats_buf_data->BgBuf[i].fd_data.handle;
      release_buf.length = bayer_stats_buf_data->BgBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_BG, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  }

  if (stats_marker & STATS_BF) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd,
      MSM_STATS_TYPE_BF);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }
    for (i = 0; i < BF_STATS_BUFNUM; ++i) {
      bayer_stats_buf_data->cur_bf_idx = i;
      release_buf.type = STAT_BF;
      release_buf.buffer = (unsigned long) bayer_stats_buf_data->BfBuf[i].buf;
      release_buf.fd = bayer_stats_buf_data->BfBuf[i].fd;
      release_buf.handle = bayer_stats_buf_data->BfBuf[i].fd_data.handle;
      release_buf.length = bayer_stats_buf_data->BfBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_BF, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  }

  if (stats_marker & STATS_BHIST) {
    rc = vfe_stats_flush_bufq(p_obj->vfe_params.camfd,
      MSM_STATS_TYPE_BHIST);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_stats_flush_bufq failed\n", __func__);
    }
    for (i = 0; i < BHIST_STATS_BUFNUM; ++i) {
      bayer_stats_buf_data->cur_bhist_idx = i;
      release_buf.type = STAT_BHIST;
      release_buf.buffer = (unsigned long) bayer_stats_buf_data->BhistBuf[i].buf;
      release_buf.fd = bayer_stats_buf_data->BhistBuf[i].fd;
      release_buf.handle = bayer_stats_buf_data->BhistBuf[i].fd_data.handle;
      release_buf.length = bayer_stats_buf_data->BhistBuf[i].ion_alloc.len;

      rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_BHIST, i,
        (void *)&release_buf);
      if (rc < 0) {
        CDBG("%s: vfe_stats_release_buf failed\n", __func__);
      }
    }
  } //*stats_marker | STATS_BHIST

  CDBG("%s: X \n", __func__);
} /* vfe_release_all_stats_buff */

#ifndef VFE_2X // for VFE31 and VFE32
/*===========================================================================
 * FUNCTION    -  vfe_stats_parse_AEC_stats_regions -
 *
 * DESCRIPTION: parse the vfe AEC stats buffer.
 *==========================================================================*/
static int vfe_stats_parse_AEC_stats_regions(
  vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  int32_t  numRegions = 0, i;
  uint32_t *current_region;
  uint32_t high_shift_bits;
  uint32_t *SY;
  uint32_t *ae_statsOutputBuffer;
  uint32_t shift_bits =
    p_obj->vfe_module.stats.aec_stats.aec_stats_cmd.shiftBits;
  legacy_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.legacy_stats_buf);
  isp_stats_t *vfe_stats_struct = &(p_stats_output->vfe_stats_struct);

  ae_statsOutputBuffer = (uint32_t*)
  stats_buf_data->AecBuf[stats_buf_data->cur_AecBuf].buf;

  CDBG("%s: ae buff: %p, vfe_stat_struct :%p\n", __func__, ae_statsOutputBuffer,
    vfe_stats_struct);

  numRegions = MCTL_AEC_NUM_REGIONS;
  if (ae_statsOutputBuffer != NULL && vfe_stats_struct != NULL) {
    SY = vfe_stats_struct->aec_op.SY;

     //TODO: HARD CODE for now, sp_input->mctl_info.numRegions;
    //numRegions = MCTL_AEC_NUM_REGIONS;
    /* Translate packed 4 32 bit word per region struct comming from the VFE
     * into more usable struct for microprocessor algorithms,
     * vfeStatDspOutput - up to 4k output of DSP from VFE block for AEC and
       AWB control */

    /* copy pointer to VFE stat 2 output region, plus 1 for header */
    current_region = ae_statsOutputBuffer;

    /* for 7500 EXP statistics, still need to input the MaxDY, MinY
     * and MaxY in the future. */
    if (shift_bits < 16) {
      high_shift_bits = 16 - shift_bits;
      for (i = 0; i < numRegions/2; i++) {
        /* Either 64 or 256 regions processed here */

        /* 16 bits sum of Y. */
        *SY = ((*(current_region)) & 0x0000FFFF) << shift_bits;
        SY++;
        *SY = (((*(current_region)) & 0xFFFF0000) >> high_shift_bits );
        SY++;

        current_region ++; /* each step is a 32 bit words or 32 bytes long */
        /* which is 2 of 16 bit words         */
      }
    } else if (shift_bits > 16) {

      high_shift_bits = shift_bits - 16;
      for (i = 0; i < numRegions/2; i++) {
        /* Either 64 or 256 regions processed here */

        /* 16 bits sum of Y. */
        *SY = ((*(current_region)) & 0x0000FFFF) << shift_bits;
        SY++;
        *SY = (((*(current_region)) & 0xFFFF0000) << high_shift_bits );
        SY++;

        current_region ++; /* each step is a 32 bit words or 32 bytes long */
        /* which is 2 of 16 bit words         */
      }
    } else {
      for (i = 0; i < numRegions/2; i++) {
        /* Either 64 or 256 regions processed here */

        /* 16 bits sum of Y. */
        *SY = ((*(current_region)) & 0x0000FFFF) << 16;
        SY++;
        *SY = ((*(current_region)) & 0xFFFF0000);
        SY++;

        current_region ++; /* each step is a 32 bit words or 32 bytes long */
        /* which is 2 of 16 bit words         */
      }
    }
  } else {
    CDBG_ERROR("%s: output Null pointer: %s %d ", __func__,
      __FILE__, __LINE__);
  }
  return numRegions;
} /* vfe_stats_parse_AEC_stats_regions */

/*===========================================================================
 * FUNCTION    -  vfe_stats_parse_AWB_stats_regions -
 *
 * DESCRIPTION: parse the vfe AWB stats buffer.
 *==========================================================================*/
static void vfe_stats_parse_AWB_stats_regions(vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  int32_t  numRegions, i;
  uint32_t *current_region;
  uint32_t high_shift_bits;
  uint8_t inputNumReg;
  uint32_t *SCb, *SCr, *SY1, *NSCb;
  uint32_t *awb_statsOutputBuffer;
  int rc = 0;
  //TODO: need to add check for the type of buf
  common_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);
  isp_stats_t *vfe_stats_struct = &(p_stats_output->vfe_stats_struct);

  awb_statsOutputBuffer = (uint32_t*)
  stats_buf_data->AwbBuf[stats_buf_data->cur_AwbBuf].buf;

  if (awb_statsOutputBuffer != NULL && vfe_stats_struct != NULL) {
    SCb = vfe_stats_struct->awb_op.SCb;
    SCr = vfe_stats_struct->awb_op.SCr;
    SY1 = vfe_stats_struct->awb_op.SY1;
    NSCb = vfe_stats_struct->awb_op.NSCb;

    numRegions = MCTL_AEC_NUM_REGIONS; //sp_input->mctl_info.numRegions;
    /* Translate packed 4 32 bit word per region struct comming from the VFE
     * into more usable struct for microprocessor algorithms,
     * vfeStatDspOutput - up to 4k output of DSP from VFE block for AEC and
       AWB control */

    /* copy pointer to VFE stat 2 output region, plus 1 for header */
    current_region = awb_statsOutputBuffer;
#ifdef VFE_32
    for (i = 0; i < numRegions; i++) {
      /* Either 64 or 256 regions processed here */
      /* 16 bits sum of Y. */
      *SY1 = ((*(current_region)) & 0x01FFFFFF);
      SY1++;
      current_region ++;  /* each step is a 32 bit words or 32 bytes long */
                          /* which is 2 of 16 bit words */
      *SCb = ((*(current_region)) & 0x01FFFFFF);
      SCb++;
      current_region ++;
      *SCr = ((*(current_region)) & 0x01FFFFFF);
      SCr++;
      current_region ++;
      /* NSCb counter should not left shifted by bitshift */
      *NSCb = ((*(current_region)) & 0x0001FFFF);
      NSCb++;
      current_region ++;
    }
    vfe_stats_struct->awb_op.GLB_Y = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.GLB_Cb = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.GLB_Cr = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.GLB_N = ((*(current_region)) & 0x1FFFFFF);
    current_region ++;
    vfe_stats_struct->awb_op.Green_R = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.Green_G = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.Green_B = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.Green_N = ((*current_region) & 0x1FFFFFF);
    current_region ++;
    vfe_stats_struct->awb_op.ExtBlue_R = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.ExtBlue_G = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.ExtBlue_B = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.ExtBlue_N = ((*current_region) & 0x1FFFFFF);
    current_region ++;
    vfe_stats_struct->awb_op.ExtRed_R = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.ExtRed_G = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.ExtRed_B = *(current_region);
    current_region ++;
    vfe_stats_struct->awb_op.ExtRed_N = ((*current_region) & 0x1FFFFFF);
  } else {
    CDBG_ERROR("%s: output Null pointer: Parsing skipped %s %d ", __func__,
      __FILE__, __LINE__);
  }
#else
    int32_t shift_bits;
    //vfe_output_t vfe_out;
    //rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    //  p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_AWB_SHIFT_BITS, (void *)&vfe_out);

    shift_bits = p_obj->vfe_module.stats.awb_stats.VFE_StatsAwb_ConfigCmd.shiftBits;
    CDBG("%s: AWB shift bits %d", __func__, shift_bits);

    if (shift_bits < 16) {
      high_shift_bits = 16 - shift_bits;
      for (i = 0; i < numRegions; i++) {
        /* Either 64 or 256 regions processed here */
        /* 16 bits sum of Y. */
        *SY1 = ((*(current_region)) & 0x0000FFFF) <<
          shift_bits;
        *SCb = ((*(current_region)) & 0xFFFF0000) >> high_shift_bits;

        current_region ++; /* each step is a 32 bit words or 32 bytes long */
        /* which is 2 of 16 bit words         */
        *SCr = ((*(current_region)) & 0x0000FFFF) <<
          shift_bits;
        /* NSCb counter should not left shifted by bitshift */
        *NSCb = ((*(current_region)) & 0xFFFF0000) >> 16;
        if(((*NSCb) >> shift_bits) < 10 ) {
          *SY1 = 0;
          *SCb = 0;
          *SCr = 0;
          *NSCb = 0;
        }
        SY1++;
        SCb++;
        SCr++;
        NSCb++;
        current_region ++; /* each step is a 32 bit words or 32 bytes long */
        /* which is 2 of 16 bit words         */
      }
    } else if (shift_bits > 16) {
      high_shift_bits = shift_bits - 16;
      for (i = 0; i < numRegions; i++) {
        /* Either 64 or 256 regions processed here */

        /* 16 bits sum of Y. */
        *SY1 = ((*(current_region)) & 0x0000FFFF) <<
          shift_bits;
        SY1++;
        *SCb = ((*(current_region)) & 0xFFFF0000) << high_shift_bits;
        SCb++;
        current_region ++; /* each step is a 32 bit words or 32 bytes long */
        /* which is 2 of 16 bit words         */

        *SCr = ((*(current_region)) & 0x0000FFFF) <<
         shift_bits;
        SCr++;
        *NSCb = ((*(current_region)) & 0xFFFF0000) << high_shift_bits;
        NSCb++;
        current_region ++; /* each step is a 32 bit words or 32 bytes long */
        /* which is 2 of 16 bit words         */
      }
    } else {
      for (i = 0; i < numRegions; i++) {
        /* Either 64 or 256 regions processed here */

        /* 16 bits sum of Y. */
        *SY1 = ((*(current_region)) & 0x0000FFFF) << 16;
        SY1++;
        *SCb = ((*(current_region)) & 0xFFFF0000) ;
        SCb++;
        current_region ++; /* each step is a 32 bit words or 32 bytes long */
        /* which is 2 of 16 bit words         */

        *SCr = ((*(current_region)) & 0x0000FFFF) << 16;
        SCr++;
        *NSCb = ((*(current_region)) & 0xFFFF0000);
        NSCb++;
        current_region ++; /* each step is a 32 bit words or 32 bytes long */
        /* which is 2 of 16 bit words         */
      }
    }
  } else {
    CDBG_ERROR("isp3a_shared_parse_regions: output Null pointer: %s %d ",
      __FILE__, __LINE__);
  }
#endif

} /* vfe_stats_parse_AWB_stats_regions */

/*===========================================================================
 * FUNCTION    -  vfe_stats_parse_RS_stats -
 *
 * DESCRIPTION: parse the vfe RS stats buffer.
 *==========================================================================*/
static void vfe_stats_parse_RS_stats(vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  int  i, num;
  uint32_t *RSum, *RS_statsOutputBuffer, shiftBits = 0;
  uint16_t *current_region;
  //TODO: need to add check for the type of buf
  common_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);
  isp_stats_t *vfe_stats_struct = &(p_stats_output->vfe_stats_struct);

  RS_statsOutputBuffer = (uint32_t*) stats_buf_data->
    RSBuf[stats_buf_data->cur_RSBuf].buf;

  if (RS_statsOutputBuffer != NULL && vfe_stats_struct != NULL) {

    current_region = (uint16_t *)RS_statsOutputBuffer;
    RSum = vfe_stats_struct->rs_op.row_sum;
    /* to do regeion cnt & shift bits should come from ISP */
    num = p_obj->vfe_params.rs_cs_params.rs_num_rgns;
    shiftBits = p_obj->vfe_params.rs_cs_params.rs_shift_bits;

    CDBG("%s: num = %d, shiftBits = %d\n", __func__, num, shiftBits);
    for (i = 0; i < num; i++)
      *RSum++ = (*current_region++) << shiftBits;
  } else
    CDBG_ERROR("%s: output Null pointer: Parsing skipped", __func__);
} /* vfe_stats_parse_RS_stats */

/*===========================================================================
 * FUNCTION    -  vfe_stats_parse_CS_stats -
 *
 * DESCRIPTION: parse the vfe CS stats buffer.
 *==========================================================================*/
static void vfe_stats_parse_CS_stats(vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  int  i, num;
  uint32_t *CSum, *CS_statsOutputBuffer, shiftBits = 0;
  uint16_t *current_region;
  //TODO: need to add check for the type of buf
  common_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);
  static uint32_t frame_count = 0;
  isp_stats_t *vfe_stats_struct = &(p_stats_output->vfe_stats_struct);

  CS_statsOutputBuffer = (uint32_t*) stats_buf_data->
    CSBuf[stats_buf_data->cur_CSBuf].buf;

  if (CS_statsOutputBuffer != NULL && vfe_stats_struct != NULL) {

    current_region = (uint16_t *)CS_statsOutputBuffer;
    CSum = vfe_stats_struct->cs_op.col_sum;
    /* to do regeion cnt & shift bits should come from ISP */
    num = p_obj->vfe_params.rs_cs_params.cs_num_rgns;
    shiftBits = p_obj->vfe_params.rs_cs_params.cs_shift_bits;
    CDBG("%s: num = %d, shiftBits = %d\n", __func__, num, shiftBits);
    for (i = 0; i < num; i++)
      *CSum++ = (*current_region++) << shiftBits;
  } else
    CDBG_ERROR("%s: output Null pointer: Parsing skipped", __func__);
} /* vfe_stats_parse_CS_stats */

/*===========================================================================
 * FUNCTION    - vfe_stats_parse_BE_stats_regions
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_stats_parse_BE_stats_regions(vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  uint32_t *SY,*Sr, *Sb, *Sgr, *Sgb;
  uint32_t *r_num, *b_num, *gr_num, *gb_num;
  uint32_t *be_statsOutputBuffer;
  uint32_t *current_region;
  uint32_t  i, x, y;

  bayer_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  isp_stats_t *vfe_stats_struct = &(p_stats_output->vfe_stats_struct);

  be_statsOutputBuffer = (uint32_t*)
  stats_buf_data->BeBuf[stats_buf_data->cur_be_idx].buf;
  if (be_statsOutputBuffer == NULL) {
    CDBG("be_statsOutputBuffer == NULL\n");
  }

  if (be_statsOutputBuffer != NULL && vfe_stats_struct != NULL) {
    SY = vfe_stats_struct->be_op.SY;
    Sr = vfe_stats_struct->bayer_r_sum;
    Sb = vfe_stats_struct->bayer_b_sum;
    Sgr = vfe_stats_struct->bayer_gr_sum;
    Sgb = vfe_stats_struct->bayer_gb_sum;
    r_num = vfe_stats_struct->bayer_r_num;
    b_num = vfe_stats_struct->bayer_b_num;
    gr_num = vfe_stats_struct->bayer_gr_num;
    gb_num = vfe_stats_struct->bayer_gb_num;

    current_region = be_statsOutputBuffer;
    /*
      BE Stats expect:
      1 - 24bit out of 32bit r_sum
      2 - 24bit out of 32bit b_sum
      3 - 24bit out of 32bit gr_sum
      4 - 24bit out of 32bit gb_sum
      5 - 16bit out of 32bit USL bnum, 16bit out of 32bit LSL rnum
      6 - 16bit out of 32bit USL gbnum, 16bit out of 32bit LSL grnum

      Expect buf_size = 32*24 * 6
    */
    for (i=0; i<(768/*32*24*/); i++) {
      //parse BE stats
      /* 32*24 regions, total 3888 */
      /* 24 bits sum of r, b, gr, gb. */

      *Sr = ((*(current_region)) & 0x00FFFFFF);
      Sr++;
      current_region ++;
      *Sb = ((*(current_region)) & 0x00FFFFFF);
      Sb++;
      current_region ++;
      *Sgr = ((*(current_region)) & 0x00FFFFFF);
      Sgr++;
      current_region ++;
      *Sgb = ((*(current_region)) & 0x00FFFFFF);
      Sgb++;
      current_region ++;
      /*16 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum*/
      *r_num = ((*(current_region)) & 0x0000FFFF);
      *b_num = ((*(current_region)) & 0xFFFF0000) >> 16;
      current_region++;
      *gr_num = ((*(current_region)) & 0x0000FFFF);
      *gb_num = ((*(current_region)) & 0xFFFF0000) >> 16;

      current_region ++;
      r_num++;
      b_num++;
      gr_num++;
      gb_num++;
    }
    /*convert bayer r,g,b stat into Ysum to make it work on current 3a version
      that uses 16x16 Ysum*/
    SY = vfe_stats_struct->be_op.SY;
    Sr = vfe_stats_struct->bayer_r_sum;
    Sb = vfe_stats_struct->bayer_b_sum;
    Sgr = vfe_stats_struct->bayer_gr_sum;
    Sgb = vfe_stats_struct->bayer_gb_sum;
    r_num = vfe_stats_struct->bayer_r_num;
    b_num = vfe_stats_struct->bayer_b_num;
    gr_num = vfe_stats_struct->bayer_gr_num;
    gb_num = vfe_stats_struct->bayer_gb_num;


    for (x=0; x<16; x++) {
      for (y=0; y<16; y++) {
        uint32_t bidx, bidx1, bidx2;
        uint32_t n_pixel;
        uint64_t tempr,tempgr, tempb, tnr, tnb, tngr;

        bidx =  y*3*32 + x*4;
        bidx1 = bidx + 32;
        bidx2 = bidx1 + 32;

        tempgr = Sgr[bidx]+Sgr[bidx+1]+Sgr[bidx+2]+Sgr[bidx+3]
                 + Sgr[bidx1]+Sgr[bidx1+1]+Sgr[bidx1+2]+Sgr[bidx1+3]
                 + Sgr[bidx2]+Sgr[bidx2+1]+Sgr[bidx2+2]+Sgr[bidx2+3];
        tngr = gr_num[bidx]+gr_num[bidx+1]+gr_num[bidx+2]+gr_num[bidx+3]
               + gr_num[bidx1]+gr_num[bidx1+1]+gr_num[bidx1+2]+gr_num[bidx1+3]
               + gr_num[bidx2]+gr_num[bidx2+1]+gr_num[bidx2+2]+gr_num[bidx2+3];
        if (tngr < 1) tngr =1;

        tempr = Sr[bidx]+Sr[bidx+1]+Sr[bidx+2]+Sr[bidx+3]
                + Sr[bidx1]+Sr[bidx1+1]+Sr[bidx1+2]+Sr[bidx1+3]
                + Sr[bidx2]+Sr[bidx2+1]+Sr[bidx2+2]+Sr[bidx2+3];
        tnr = r_num[bidx]+r_num[bidx+1]+r_num[bidx+2]+r_num[bidx+3]
              + r_num[bidx1]+r_num[bidx1+1]+r_num[bidx1+2]+r_num[bidx1+3]
              + r_num[bidx2]+r_num[bidx2+1]+r_num[bidx2+2]+r_num[bidx2+3];
        if (tnr < 1) tnr =1;

        tempb = Sb[bidx]+Sb[bidx+1]+Sb[bidx+2]+Sb[bidx+3]
                + Sb[bidx1]+Sb[bidx1+1]+Sb[bidx1+2]+Sb[bidx1+3]
                + Sb[bidx2]+Sb[bidx2+1]+Sb[bidx2+2]+Sb[bidx2+3];
        tnb = b_num[bidx]+b_num[bidx+1]+b_num[bidx+2]+b_num[bidx+3]
              + b_num[bidx1]+b_num[bidx1+1]+b_num[bidx1+2]+b_num[bidx1+3]
              + b_num[bidx2]+b_num[bidx2+1]+b_num[bidx2+2]+b_num[bidx2+3];
        if (tnb < 1) tnb =1;

        tempgr = tempgr / tngr;
        tempr = tempr / tnr;
        tempb = tempb / tnb;

        SY[x+(y*16)] = (tempr>>2) + (tempgr>>1) + (tempb>>2);
        n_pixel = tngr+tnr+tnb;

        if (tngr <= 1 ||tnr <= 1 || tnb <= 1)
          SY[x+(y*16)] = 255;
      }
    }
  } else {
    CDBG("%s: output Null pointer: %s %d ", __func__,__FILE__, __LINE__);
  }
  return 256;
} /* vfe_stats_parse_BE_stats_regions */

/*===========================================================================
 * FUNCTION    - vfe_stats_parse_BG_stats_regions
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_stats_parse_BG_stats_regions(vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  uint32_t *SY,*Sr, *Sb, *Sgr, *Sgb;
  uint32_t *r_num, *b_num, *gr_num, *gb_num;
  uint32_t *ae_statsOutputBuffer;
  uint32_t *current_region;
  uint32_t  i, x, y;

  bayer_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  isp_stats_t *vfe_stats_struct = &(p_stats_output->vfe_stats_struct);

  ae_statsOutputBuffer = (uint32_t*)
  stats_buf_data->BgBuf[stats_buf_data->cur_bg_idx].buf;
  if (ae_statsOutputBuffer == NULL) {
    CDBG("ae_statsOutputBuffer == NULL\n");
  }

  if (ae_statsOutputBuffer != NULL && vfe_stats_struct != NULL) {
    SY = vfe_stats_struct->aec_op.SY;
    Sr = vfe_stats_struct->bayer_r_sum;
    Sb = vfe_stats_struct->bayer_b_sum;
    Sgr = vfe_stats_struct->bayer_gr_sum;
    Sgb = vfe_stats_struct->bayer_gb_sum;
    r_num = vfe_stats_struct->bayer_r_num;
    b_num = vfe_stats_struct->bayer_b_num;
    gr_num = vfe_stats_struct->bayer_gr_num;
    gb_num = vfe_stats_struct->bayer_gb_num;

    current_region = ae_statsOutputBuffer;
    /*
      BG Stats expect:
      1 - 23bit out of 32bit r_sum
      2 - 23bit out of 32bit b_sum
      3 - 23bit out of 32bit gr_sum
      4 - 23bit out of 32bit gb_sum
      5 - 15bit out of 32bit USL bnum, 15bit out of 32bit LSL rnum
      6 - 15bit out of 32bit USL gbnum, 15bit out of 32bit LSL grnum

      Expect buf_size = 72*54 * 6 = 23328  (uint32)  93312
    */
    for (i=0; i<(3888/*72*54*/); i++) {
      //parse AE stats
      /* 72*54 regions, total 3888 */
      /* 23 bits sum of r, b, gr, gb. */

      *Sr = ((*(current_region)) & 0x007FFFFF);
      Sr++;
      current_region ++;
      *Sb = ((*(current_region)) & 0x007FFFFF);
      Sb++;
      current_region ++;
      *Sgr = ((*(current_region)) & 0x007FFFFF);
      Sgr++;
      current_region ++;
      *Sgb = ((*(current_region)) & 0x007FFFFF);
      Sgb++;
      current_region ++;
      /*15 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum*/
      *r_num = ((*(current_region)) & 0x00007FFF);
      *b_num = ((*(current_region)) & 0x7FFF0000) >> 16;
      current_region++;
      *gr_num = ((*(current_region)) & 0x00007FFF);
      *gb_num = ((*(current_region)) & 0x7FFF0000) >> 16;

      current_region ++;
      r_num++;
      b_num++;
      gr_num++;
      gb_num++;
    }
    /*convert bayer r,g,b stat into Ysum to make it work on current 3a version
      that uses 16x16 Ysum*/
    SY = vfe_stats_struct->aec_op.SY;
    Sr = vfe_stats_struct->bayer_r_sum;
    Sb = vfe_stats_struct->bayer_b_sum;
    Sgr = vfe_stats_struct->bayer_gr_sum;
    Sgb = vfe_stats_struct->bayer_gb_sum;
    r_num = vfe_stats_struct->bayer_r_num;
    b_num = vfe_stats_struct->bayer_b_num;
    gr_num = vfe_stats_struct->bayer_gr_num;
    gb_num = vfe_stats_struct->bayer_gb_num;


    for (x=0; x<16; x++) {
      for (y=0; y<16; y++) {
        uint32_t bidx, bidx1, bidx2;
        uint32_t n_pixel;
        uint64_t tempr,tempgr, tempb, tnr, tnb, tngr;

        bidx =  y*3*72 + x*4;
        bidx1 = bidx + 72;
        bidx2 = bidx1 + 72;

#if 0
        SY[x+(y*16)] = Sgr[bidx]+Sgr[bidx+1]+Sgr[bidx+2]+Sgr[bidx+3]
                       + Sgr[bidx1]+Sgr[bidx1+1]+Sgr[bidx1+2]+Sgr[bidx1+3]
                       + Sgr[bidx2]+Sgr[bidx2+1]+Sgr[bidx2+2]+Sgr[bidx2+3];
        n_pixel = gr_num[bidx]+gr_num[bidx+1]+gr_num[bidx+2]+gr_num[bidx+3]
                  + gr_num[bidx1]+gr_num[bidx1+1]+gr_num[bidx1+2]+gr_num[bidx1+3]
                  + gr_num[bidx2]+gr_num[bidx2+1]+gr_num[bidx2+2]+gr_num[bidx2+3];

        if (n_pixel<=0)
          SY[x+(y*16)] = 255;
        else
          SY[x+(y*16)] = SY[x+(y*16)] / n_pixel;
#else

        tempgr = Sgr[bidx]+Sgr[bidx+1]+Sgr[bidx+2]+Sgr[bidx+3]
                 + Sgr[bidx1]+Sgr[bidx1+1]+Sgr[bidx1+2]+Sgr[bidx1+3]
                 + Sgr[bidx2]+Sgr[bidx2+1]+Sgr[bidx2+2]+Sgr[bidx2+3];
        tngr = gr_num[bidx]+gr_num[bidx+1]+gr_num[bidx+2]+gr_num[bidx+3]
               + gr_num[bidx1]+gr_num[bidx1+1]+gr_num[bidx1+2]+gr_num[bidx1+3]
               + gr_num[bidx2]+gr_num[bidx2+1]+gr_num[bidx2+2]+gr_num[bidx2+3];
        if (tngr < 1) tngr =1;

        tempr = Sr[bidx]+Sr[bidx+1]+Sr[bidx+2]+Sr[bidx+3]
                + Sr[bidx1]+Sr[bidx1+1]+Sr[bidx1+2]+Sr[bidx1+3]
                + Sr[bidx2]+Sr[bidx2+1]+Sr[bidx2+2]+Sr[bidx2+3];
        tnr = r_num[bidx]+r_num[bidx+1]+r_num[bidx+2]+r_num[bidx+3]
              + r_num[bidx1]+r_num[bidx1+1]+r_num[bidx1+2]+r_num[bidx1+3]
              + r_num[bidx2]+r_num[bidx2+1]+r_num[bidx2+2]+r_num[bidx2+3];
        if (tnr < 1) tnr =1;

        tempb = Sb[bidx]+Sb[bidx+1]+Sb[bidx+2]+Sb[bidx+3]
                + Sb[bidx1]+Sb[bidx1+1]+Sb[bidx1+2]+Sb[bidx1+3]
                + Sb[bidx2]+Sb[bidx2+1]+Sb[bidx2+2]+Sb[bidx2+3];
        tnb = b_num[bidx]+b_num[bidx+1]+b_num[bidx+2]+b_num[bidx+3]
              + b_num[bidx1]+b_num[bidx1+1]+b_num[bidx1+2]+b_num[bidx1+3]
              + b_num[bidx2]+b_num[bidx2+1]+b_num[bidx2+2]+b_num[bidx2+3];
        if (tnb < 1) tnb =1;

        tempgr = tempgr / tngr;
        tempr = tempr / tnr;
        tempb = tempb / tnb;

        SY[x+(y*16)] = (tempr>>2) + (tempgr>>1) + (tempb>>2);
        n_pixel = tngr+tnr+tnb;

        if (tngr <= 1 ||tnr <= 1 || tnb <= 1)
          SY[x+(y*16)] = 255;
#endif
      }
    }
  } else {
    CDBG("%s: output Null pointer: %s %d ", __func__,__FILE__, __LINE__);
  }
  return 256;
} /* vfe_stats_parse_BG_stats_regions */

/*===========================================================================
 * FUNCTION    - vfe_stats_parse_BF_stats_regions
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_stats_parse_BF_stats_regions(vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  int window;
  uint32_t i, j;
  uint8_t *autofocusMultiWindowGrid = NULL;
  uint32_t *Sr,*Sb, *Sgr, *Sgb, *bf_stats;
  uint32_t *r_sh, *b_sh, *gr_sh, *gb_sh;
  uint32_t *r_num, *b_num, *gr_num, *gb_num;
  uint32_t *current_region = NULL;

  bayer_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  isp_stats_t *vfe_stats_struct = &(p_stats_output->vfe_stats_struct);
  uint32_t *afStatsOutputPtr =(uint32_t *)
  stats_buf_data->BfBuf[stats_buf_data->cur_bf_idx].buf;
#if 1
  //CDBG("mctl_stats_AF_bayer_stats_regions 1 afStatsOutputPtr = %d\n",*afStatsOutputPtr);
  if (afStatsOutputPtr != NULL && vfe_stats_struct != NULL) {

    bf_stats = vfe_stats_struct->bf_stats;
    Sr = vfe_stats_struct->bf_r_sum;
    Sb = vfe_stats_struct->bf_b_sum;
    Sgr = vfe_stats_struct->bf_gr_sum;
    Sgb = vfe_stats_struct->bf_gb_sum;
    r_sh = vfe_stats_struct->bf_r_sharp;
    b_sh = vfe_stats_struct->bf_b_sharp;
    gr_sh = vfe_stats_struct->bf_gr_sharp;
    gb_sh = vfe_stats_struct->bf_gb_sharp;
    r_num = vfe_stats_struct->bf_r_num;
    b_num = vfe_stats_struct->bf_b_num;
    gr_num = vfe_stats_struct->bf_gr_num;
    gb_num = vfe_stats_struct->bf_gb_num;

    current_region = afStatsOutputPtr;

    //CDBG("mctl_stats_AF_bayer_stats_regions 2 afStatsOut/putPtr=%d\n",*afStatsOutputPtr);
    memcpy(bf_stats, current_region, sizeof(uint32_t) * 2520);
    current_region = bf_stats;
    //Expect buf_size = 14*18 * 10 = 2520  (uint32)  10080
#if 1
    for (i=0; i<(14*18); i++) {
      /*parse AF stats */
      /* 14*18 regions, total 252 */

      *Sr = ((*(current_region)) & 0x00FFFFFF);
      Sr++;
      current_region++;
      *Sb = ((*(current_region)) & 0x00FFFFFF);
      Sb++;
      current_region++;
      *Sgr = ((*(current_region)) & 0x00FFFFFF);
      Sgr++;
      current_region++;
      *Sgb = ((*(current_region)) & 0x00FFFFFF);
      Sgb++;
      current_region++;
      *r_sh = *current_region;
      r_sh++;
      current_region++;
      *b_sh = *current_region;
      b_sh++;
      current_region++;
      *gr_sh = *current_region;
      gr_sh++;
      current_region++;
      *gb_sh = *current_region;
      gb_sh++;
      current_region++;
      *r_num = ((*(current_region)) & 0x0000FFFF);
      *b_num = ((*(current_region)) & 0xFFFF0000) >> 16;
      current_region++;
      *gr_num = ((*(current_region)) & 0x0000FFFF);
      *gb_num = ((*(current_region)) & 0xFFFF0000) >> 16;
      current_region++;
    }
#endif
    gr_sh = vfe_stats_struct->bf_gr_sharp;
    vfe_stats_struct->af_op.Focus = 0;
    for (i=0; i<(14*18); i++) {
      vfe_stats_struct->af_op.Focus += gr_sh[i];
    }
    vfe_stats_struct->af_op.NFocus = 1;

    CDBG("%s:Focus=%d NRegions=%d \n", __func__, vfe_stats_struct->af_op.Focus,14*18);
  }


#if 0
  CDBG("%s: Focus Rectangle=%d\n", __func__,
       ctrl->afCtrl.parm_focusrect.current_value);
  switch (ctrl->afCtrl.parm_focusrect.current_value) {
  case AUTO:
    if (ctrl->afCtrl.roiInfo.num_roi >= 2) {
      autofocusMultiWindowGrid = af_face_detection_windows;
      CDBG("%s: Multiple AF windows\n", __func__);
    }
    break;

  case SPOT:
    autofocusMultiWindowGrid = af_spot_windows;
    break;

  case AVERAGE:
    autofocusMultiWindowGrid = af_average_windows;
    break;

  case CENTER_WEIGHTED:
    autofocusMultiWindowGrid = af_center_weighted_windows;
    break;

  default:
    break;
  }
#endif
#else
  vfe_stats_struct->Focus = 100;
#endif

} /* vfe_stats_parse_BF_stats_regions */

/*===========================================================================
 * FUNCTION    - vfe_stats_parse_BHIST_stats_regions
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_stats_parse_BHIST_stats_regions(vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  uint32_t *Srh, *Sbh, *Sgrh, *Sgbh;
  uint32_t *hist_statsOutputBuffer;
  uint32_t *current_region;
  uint32_t  i;
  bayer_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  isp_stats_t *vfe_stats_struct = &(p_stats_output->vfe_stats_struct);

  hist_statsOutputBuffer = (uint32_t*)
  stats_buf_data->BhistBuf[stats_buf_data->cur_bhist_idx].buf;

  if (vfe_stats_struct == NULL) {
    CDBG("vfe_stats_struct == NULL\n");
  }
  if (hist_statsOutputBuffer == NULL) {
    CDBG("hist_statsOutputBuffer == NULL\n");
  }

  if (hist_statsOutputBuffer != NULL && vfe_stats_struct != NULL) {
    //parse histogram data
    Srh = vfe_stats_struct->bayer_r_hist;
    Sbh = vfe_stats_struct->bayer_b_hist;
    Sgrh = vfe_stats_struct->bayer_gr_hist;
    Sgbh = vfe_stats_struct->bayer_gb_hist;
    current_region = hist_statsOutputBuffer;
    for (i=0; i<256; i++) { //0 to 255, total 256 bins
      *Srh = ((*(current_region)) & 0x007FFFFF);
      Srh++; current_region++;
      *Sbh = ((*(current_region)) & 0x007FFFFF);
      Sbh++; current_region++;
      *Sgrh = ((*(current_region)) & 0x007FFFFF);
      Sgrh++; current_region++;
      *Sgbh = ((*(current_region)) & 0x007FFFFF);
      Sgbh++; current_region++;
    }
  }
} /* vfe_stats_parse_BHIST_stats_regions */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_AE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_AE(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0;
  int i = 0;
  struct msm_stats_buf release_buf;
  int32_t  numRegions;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  legacy_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.legacy_stats_buf);
  aec_stats_t *stats_mod = &(p_obj->vfe_module.stats.aec_stats);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);
  struct msm_cam_evt_divert_frame div_frame;
  vfe_stats_ae_struct_t *aec_parse_op_buf = NULL;

  CDBG("%s E: fd = %d\n", __func__, buf->fd);

  for (i = 0; i < AEC_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->AecBuf[i].fd) {
      stats_buf_data->cur_AecBuf = i;
      release_buf.type = STAT_AEC;
      release_buf.buffer = (unsigned long) stats_buf_data->AecBuf[i].buf;
      release_buf.fd = stats_buf_data->AecBuf[i].fd;
      release_buf.handle = stats_buf_data->AecBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->AecBuf[i].ion_alloc.len;
      break;
    }
  }
  if (i == AEC_STATS_BUFNUM) {
    CDBG_ERROR("%s: AEC_STATS buffer mismatch: fd = %d\n", __func__, buf->fd);
    rc = -ENOENT;
  }

  if (!isp_started) {
    /*do nothing, if camera not started or af is active, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  } else {

    stats_output->numRegions =
      vfe_stats_parse_AEC_stats_regions(p_obj, stats_output);
    stats_output->aec_params.shift_bits =
      p_obj->vfe_module.stats.aec_stats.aec_stats_cmd.shiftBits;
    stats_output->aec_params.rgn_width =
      p_obj->vfe_module.stats.aec_stats.aec_stats_cmd.rgnWidth;
    stats_output->aec_params.rgn_height =
      p_obj->vfe_module.stats.aec_stats.aec_stats_cmd.rgnHeight;

    if(stats_mod->use_hal_buf) {
      if(p_obj->ops->get_stats_op_buffer) {
        aec_parse_op_buf = p_obj->ops->get_stats_op_buffer(
          p_obj->ops->parent, &div_frame, MSM_STATS_TYPE_AEC);
      } else {
        CDBG_HIGH("%s: get_stats_op_buffer failed\n", __func__);
      }

      CDBG("%s: buffer : %p\n", __func__, aec_parse_op_buf);
      if(aec_parse_op_buf != NULL) {
        aec_parse_op_buf->cm_data.height =
          stats_output->aec_params.rgn_height;
        aec_parse_op_buf->cm_data.width =
          stats_output->aec_params.rgn_width;
        aec_parse_op_buf->cm_data.stats_type =
          MSM_STATS_TYPE_AEC;
        aec_parse_op_buf->cm_data.stats_version = 0;
        aec_parse_op_buf->cm_data.frame_id = 0;

        memcpy(&(aec_parse_op_buf->ae),
          &(stats_output->vfe_stats_struct.aec_op),
          sizeof(vfe_stats_ae_data_t));

        if(p_obj->ops->put_stats_op_buffer) {
          p_obj->ops->put_stats_op_buffer(p_obj->ops->parent,
            &div_frame, MSM_STATS_TYPE_AEC);
        } else {
          CDBG_HIGH("%s: put_stats_op_buffer failed\n", __func__);
        }
      }
    }
  }
  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_AEC,
         stats_buf_data->cur_AecBuf, (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
  }

  stats_output->aec_bg_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_AE */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_AWB -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_AWB(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0, i = 0;
  struct msm_stats_buf release_buf;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  common_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);
  awb_stats_t* stats_mod = &(p_obj->vfe_module.stats.awb_stats);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);
  struct msm_cam_evt_divert_frame div_frame;
  vfe_stats_awb_struct_t *parse_op_buf = NULL;

  CDBG("%s E: fd = %d\n", __func__, buf->fd);

  for (i = 0; i < AWB_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->AwbBuf[i].fd) {
      stats_buf_data->cur_AwbBuf = i;
      release_buf.type = STAT_AWB;
      release_buf.buffer = (unsigned long) stats_buf_data->AwbBuf[i].buf;
      release_buf.fd = stats_buf_data->AwbBuf[i].fd;
      release_buf.handle = stats_buf_data->AwbBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->AwbBuf[i].ion_alloc.len;
      break;
    }
  }
  if (i == AWB_STATS_BUFNUM) {
    CDBG_ERROR("%s: AWB_STATS buffer mismatch: fd = %d\n", __func__, buf->fd);
    rc = -ENOENT;
  }
  if (!isp_started) {
    /*do nothing, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  } else {
    vfe_stats_parse_AWB_stats_regions(p_obj, stats_output);
    stats_output->awb_shift_bits =
      stats_mod->VFE_StatsAwb_ConfigCmd.shiftBits;
    if(stats_mod->use_hal_buf) {
      if(p_obj->ops->get_stats_op_buffer) {
        parse_op_buf = p_obj->ops->get_stats_op_buffer(
          p_obj->ops->parent, &div_frame, MSM_STATS_TYPE_AWB);
      } else
        CDBG_HIGH("%s: get_stats_op_buffer failed\n", __func__);

      CDBG("%s: buffer : %p\n", __func__, parse_op_buf);
      if(parse_op_buf != NULL) {
        parse_op_buf->cm_data.height =
          stats_mod->VFE_StatsAwb_ConfigCmd.rgnHeight;
        parse_op_buf->cm_data.width =
          stats_mod->VFE_StatsAwb_ConfigCmd.rgnWidth;
        parse_op_buf->cm_data.stats_type = MSM_STATS_TYPE_AWB;
        parse_op_buf->cm_data.stats_version = 0;
        parse_op_buf->cm_data.frame_id = 0;

        memcpy(&(parse_op_buf->awb),
          &(stats_output->vfe_stats_struct.awb_op),
          sizeof(vfe_stats_awb_data_t));

        if(p_obj->ops->put_stats_op_buffer) {
          p_obj->ops->put_stats_op_buffer(p_obj->ops->parent,
            &div_frame, MSM_STATS_TYPE_AWB);
        } else
          CDBG_HIGH("%s: put_stats_op_buffer failed\n", __func__);
      }
    }
  }
  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_AWB,
          stats_buf_data->cur_AwbBuf, (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
  }

  stats_output->awb_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_AWB */
/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_IHIST -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_IHIST(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0;
  int i = 0;
  static int flag = 0;
  uint16_t *hist_statsBuffer;
  struct msm_stats_buf release_buf;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  ihist_stats_t* stats_mod = &(p_obj->vfe_module.stats.ihist_stats);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);
  common_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);
  struct msm_cam_evt_divert_frame div_frame;
  vfe_stats_ihist_struct_t *parse_op_buf = NULL;

  CDBG("%s E: fd = %d\n", __func__, buf->fd);
  for (i = 0; i < IHIST_STATS_BUFNUM; ++i) {
    if (buf->ihist.fd == stats_buf_data->IhistBuf[i].fd) {
      stats_buf_data->cur_IhistBuf = i;
      release_buf.type = STAT_IHIST;
      release_buf.buffer = (unsigned long) stats_buf_data->IhistBuf[i].buf;
      release_buf.fd = stats_buf_data->IhistBuf[i].fd;
      release_buf.handle = stats_buf_data->IhistBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->IhistBuf[i].ion_alloc.len;
      break;
    }
  }

  CDBG("%s: E fd : %d, index : %d \n", __func__, stats_buf_data->IhistBuf[i].fd,
    stats_buf_data->cur_IhistBuf);
  if (i == IHIST_STATS_BUFNUM) {
    CDBG_ERROR("IHIST_STATS buffer mismatch: fd = %d, buffer = %lx\n",
      buf->ihist.fd, buf->ihist.buff);
    return -ENOENT;
  }

  hist_statsBuffer =
    (uint16_t*)(stats_buf_data->IhistBuf[stats_buf_data->cur_IhistBuf].buf);

  stats_output->ihist_index = stats_buf_data->cur_IhistBuf;

  for (i= 0; i< 256; i++) {
    //for apps
    stats_output->ihist_stats_buffer[i] = (uint16_t) *hist_statsBuffer;
    //for vfe parsing
    p_obj->vfe_module.stats.ihist_stats.vfe_Ihist_data[i] = (uint32_t ) *hist_statsBuffer;
    hist_statsBuffer++;
  }

  /* Avoiding LA algorithm execution if la disabled in chromatix */
  if (!isp_started) {
    /*do nothing, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  } else {
    stats_output->ihist_shift_bits =
      p_obj->vfe_module.stats.ihist_stats.ihist_stats_cmd.shiftBits;
    if (VFE_SUCCESS != vfe_stats_process_hist(p_obj))
    {
      CDBG_ERROR("%s: parsing failed\n", __func__);
      return VFE_ERROR_GENERAL;
    }
    if(stats_mod->use_hal_buf) {
      if(p_obj->ops->get_stats_op_buffer) {
        parse_op_buf = p_obj->ops->get_stats_op_buffer(
          p_obj->ops->parent, &div_frame, MSM_STATS_TYPE_IHIST);
      } else
        CDBG_HIGH("%s: get_stats_op_buffer failed\n", __func__);

      CDBG("%s: buffer : %p\n", __func__, parse_op_buf);
      if(parse_op_buf != NULL) {
        parse_op_buf->cm_data.height = 0;
        parse_op_buf->cm_data.width = 0;
        parse_op_buf->cm_data.stats_type = MSM_STATS_TYPE_IHIST;
        parse_op_buf->cm_data.stats_version = 0;
        parse_op_buf->cm_data.frame_id = 0;

        memcpy(&(parse_op_buf->ihist.buffer),
          &(stats_output->ihist_stats_buffer),
          sizeof(uint16_t) * 256);

        if(p_obj->ops->put_stats_op_buffer) {
          p_obj->ops->put_stats_op_buffer(p_obj->ops->parent,
            &div_frame, MSM_STATS_TYPE_IHIST);
        } else
          CDBG_HIGH("%s: put_stats_op_buffer failed\n", __func__);
      }
    }
  }

  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_IHIST,
         stats_buf_data->cur_IhistBuf, (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
  }

  stats_output->ihist_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_IHIST */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_RS -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_RS(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0, i = 0;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  common_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);
  struct msm_stats_buf release_buf;
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);

  CDBG("%s E: fd = %d\n", __func__, buf->fd);
  for (i = 0; i < RS_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->RSBuf[i].fd) {
      stats_buf_data->cur_RSBuf = i;
      release_buf.type = STAT_RS;
      release_buf.buffer = (unsigned long) stats_buf_data->RSBuf[i].buf;
      release_buf.fd = stats_buf_data->RSBuf[i].fd;
      release_buf.handle = stats_buf_data->RSBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->RSBuf[i].ion_alloc.len;
      break;
    }
  }
  if (i == RS_STATS_BUFNUM) {
    CDBG_ERROR("%s: RS_STATS buffer mismatch: fd = %d\n", __func__, buf->fd);
    rc = -ENOENT;
  }
  if (!isp_started)
    /*do nothing, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  else
    vfe_stats_parse_RS_stats(p_obj, stats_output);

  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_RS, stats_buf_data->cur_RSBuf,
         (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: vfe_stats_release_buf failed : state %d\n", __func__,
      isp_started);
  }

  stats_output->rs_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_RS */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_CS -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_CS(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0, i = 0;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  common_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.common_stats_buf);
  struct msm_stats_buf release_buf;
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);

  CDBG("%s E: fd = %d\n", __func__, buf->fd);
  for (i = 0; i < CS_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->CSBuf[i].fd) {
      stats_buf_data->cur_CSBuf = i;
      release_buf.type = STAT_CS;
      release_buf.buffer = (unsigned long) stats_buf_data->CSBuf[i].buf;
      release_buf.fd = stats_buf_data->CSBuf[i].fd;
      release_buf.handle = stats_buf_data->CSBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->CSBuf[i].ion_alloc.len;
      break;
    }
  }
  if (i == CS_STATS_BUFNUM) {
    CDBG_ERROR("%s: CS_STATS buffer mismatch: fd = %d\n", __func__, buf->fd);
    rc = -ENOENT;
  }
  if (!isp_started)
    /*do nothing, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  else
    vfe_stats_parse_CS_stats(p_obj, stats_output);

  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_CS, stats_buf_data->cur_CSBuf,
         (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: vfe_stats_release_buf failed : state %d\n", __func__,
      isp_started);
  }

  stats_output->cs_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_CS */


/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_BE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_BE(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0;
  int i = 0;
  struct msm_stats_buf release_buf;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  bayer_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);

  CDBG("%s E: buf = 0x%lx, fd = %d\n", __func__, buf->buffer,
    buf->fd);

  for (i = 0; i < BE_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->BeBuf[i].fd) {
      stats_buf_data->cur_be_idx = i;
      release_buf.type = STAT_BE;
      release_buf.buffer = (unsigned long) stats_buf_data->BeBuf[i].buf;
      release_buf.fd = stats_buf_data->BeBuf[i].fd;
      release_buf.handle = stats_buf_data->BeBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->BeBuf[i].ion_alloc.len;
      /*vfe_stats_dump((void *)release_buf.buffer,
                      stats_buf_data->BeBuf[i].buf_size,
                      MSG_ID_STATS_BE);*/
      break;
    }
  }
  if (i == BE_STATS_BUFNUM) {
    CDBG("BE_STATS buffer mismatch: fd = %d, buffer = %lx\n",
      buf->fd, buf->buffer);
    rc = -ENOENT;
  }

  if (!isp_started) {
    /*do nothing, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  } else {
      stats_output->numRegions = vfe_stats_parse_BE_stats_regions(
                                   p_obj, stats_output);
  }

  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_BE,
    stats_buf_data->cur_be_idx, (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: %d failed\n", __func__, __LINE__);
  }

  stats_output->be_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_BE */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_BG -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_BG(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0;
  int i = 0;
  struct msm_stats_buf release_buf;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  bayer_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);

  CDBG("%s E: buf = 0x%lx, fd = %d\n", __func__, buf->buffer,
    buf->fd);

  for (i = 0; i < BG_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->BgBuf[i].fd) {
      stats_buf_data->cur_bg_idx = i;
      release_buf.type = STAT_BG;
      release_buf.buffer = (unsigned long) stats_buf_data->BgBuf[i].buf;
      release_buf.fd = stats_buf_data->BgBuf[i].fd;
      release_buf.handle = stats_buf_data->BgBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->BgBuf[i].ion_alloc.len;
      /*vfe_stats_dump((void *)release_buf.buffer,
                      stats_buf_data->BgBuf[i].buf_size,
                      MSG_ID_STATS_BG);*/
      break;
    }
  }
  if (i == BG_STATS_BUFNUM) {
    CDBG("BG_STATS buffer mismatch: fd = %d, buffer = %lx\n",
      buf->fd, buf->buffer);
    rc = -ENOENT;
  }

  if (!isp_started) {
    /*do nothing, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  } else {
      stats_output->numRegions = vfe_stats_parse_BG_stats_regions(
                                   p_obj, stats_output);
  }

  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_BG,
    stats_buf_data->cur_bg_idx, (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: %d failed\n", __func__, __LINE__);
  }

  stats_output->aec_bg_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_BG */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_BF -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_BF(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0;
  int i = 0;
  struct msm_stats_buf release_buf;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  bayer_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);

  CDBG("%s E: buf = 0x%lx, fd = %d\n", __func__, buf->buffer,
    buf->fd);

  for (i = 0; i < BF_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->BfBuf[i].fd) {
      stats_buf_data->cur_bf_idx = i;
      release_buf.type = STAT_BF;
      release_buf.buffer = (unsigned long) stats_buf_data->BfBuf[i].buf;
      release_buf.fd = stats_buf_data->BfBuf[i].fd;
      release_buf.handle = stats_buf_data->BfBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->BfBuf[i].ion_alloc.len;
      /*vfe_stats_dump((void *)release_buf.buffer,
                      stats_buf_data->BfBuf[i].buf_size,
                      MSG_ID_STATS_BF);*/
      break;
    }
  }
  CDBG("cur_Bayer AFBuf = %d\n", stats_buf_data->cur_bf_idx);
  if (i == BF_STATS_BUFNUM) {
    CDBG("BF_STATS buffer mismatch: fd = %d, buffer = %lx\n",
      buf->fd, buf->buffer);
    rc = -ENOENT;
  }

  if (!isp_started) {
    /*do nothing, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  } else {
      vfe_stats_parse_BF_stats_regions(p_obj, stats_output);
  }
  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_BF,
    stats_buf_data->cur_bf_idx, (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: %d failed\n", __func__, __LINE__);
  }

  stats_output->af_bf_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_BF */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_BHIST -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_BHIST(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0;
  int i = 0;
  struct msm_stats_buf release_buf;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  bayer_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.bayer_stats_buf);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);

  CDBG("%s E: buf = 0x%lx, fd = %d\n", __func__, buf->buffer,
    buf->fd);

  for (i = 0; i < BHIST_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->BhistBuf[i].fd) {
      stats_buf_data->cur_bhist_idx = i;
      release_buf.type = STAT_BHIST;
      release_buf.buffer = (unsigned long) stats_buf_data->BhistBuf[i].buf;
      release_buf.fd = stats_buf_data->BhistBuf[i].fd;
      release_buf.handle = stats_buf_data->BhistBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->BhistBuf[i].ion_alloc.len;
      /*vfe_stats_dump((void *)release_buf.buffer,
                      stats_buf_data->BhistBuf[i].buf_size,
                      MSG_ID_STATS_BHIST);*/
      break;
    }
  }
  if (i == BHIST_STATS_BUFNUM) {
    CDBG("BHIST_STATS buffer mismatch: fd = %d, buffer = %lx\n",
      buf->fd, buf->buffer);
    rc = -ENOENT;
  }

  if (!isp_started) {
    /*do nothing, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  } else {
    //vfe_stats_parse_BHIST_stats_regions(p_obj, stats_output);
  }

  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_BHIST,
    stats_buf_data->cur_bhist_idx, (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: %d failed\n", __func__, __LINE__);
  }

  stats_output->bhist_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_BHIST */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_COMPOSITE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_COMPOSITE(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int8_t rc = 0;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *)stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);
  vfe_stats_output_t *vfe_output = stats_output;

  CDBG("%s, vfe common message = 0x%x\n", __func__, buf->status_bits);
  //TODO: Add Bayer stats
  if((buf->status_bits & VFE_STATS_AEC) && buf->aec.buff) {
    rc = vfe_stats_proc_MSG_ID_STATS_AE(p_obj, isp_started, type,
           adsp, vfe_output);
    if (rc < 0)
      return rc;
  }
  if((buf->status_bits & VFE_STATS_AWB) && buf->awb.buff) {
    rc = vfe_stats_proc_MSG_ID_STATS_AWB(p_obj, isp_started, type,
           adsp, vfe_output);
    if (rc < 0)
      return rc;
  }
  if((buf->status_bits & VFE_STATS_AF) && buf->af.buff) {
    rc = vfe_stats_proc_MSG_ID_STATS_AF(p_obj, isp_started, type,
           adsp, vfe_output);
    if (rc < 0)
       return rc;
  }
  if((buf->status_bits & VFE_STATS_IHIST) && buf->ihist.buff) {
    rc = vfe_stats_proc_MSG_ID_STATS_IHIST(p_obj, isp_started, type,
             adsp, vfe_output);
    if (rc < 0)
      return rc;
  }
  if((buf->status_bits & VFE_STATS_RS) && buf->rs.buff) {
    rc = vfe_stats_proc_MSG_ID_STATS_RS(p_obj, isp_started, type,
           adsp, vfe_output);
    if (rc < 0)
       return rc;
  }
  if((buf->status_bits & VFE_STATS_CS) && buf->cs.buff) {
    rc = vfe_stats_proc_MSG_ID_STATS_CS(p_obj, isp_started, type,
           adsp, vfe_output);
    if (rc < 0)
      return rc;
  }
  return rc;
}

#else /* VFE_2X */
/*===========================================================================
 * FUNCTION    -  vfe_stats_parse_WB_EXP_stats_regions -
 *
 * DESCRIPTION: parse the vfe AEC stats buffer.
 *==========================================================================*/
static int vfe_stats_parse_WB_EXP_stats_regions(vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  int32_t  numRegions = 0, i;
  uint32_t *current_region;
  uint32_t high_shift_bits;

  uint32_t *SY;
  uint32_t *wb_expStatsOutputBuffer;
  //TODO: need to add check for the type of buf
  legacy_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.legacy_stats_buf);
  isp_stats_t *vfeStatStruct = &(p_stats_output->vfe_stats_struct);

  CDBG("%s:\n", __func__);
  /* TODO: hardcoding numRegions as inputNumReg is hardcoded to 16*16 */
  //if (inputNumReg == VFE_16x16)
    numRegions = 256;
  //else
  //  numRegions = 64;

  wb_expStatsOutputBuffer =
      (uint32_t *) (stats_buf_data->AwbAecBuf[stats_buf_data->cur_AwbAecBuf].buf);

  if (wb_expStatsOutputBuffer != NULL && vfeStatStruct != NULL) {
    /* Translate packed 4 32 bit word per region struct comming from the VFE
     * into more usable struct for microprocessor algorithms,
     * vfeStatDspOutput - up to 4k output of DSP from VFE block for AEC and
     AWB control */

    /* copy pointer to VFE stat 2 output region, plus 1 for header */
    CDBG("Entered the parsing\n");
    current_region = wb_expStatsOutputBuffer + 1;

    /* for 7500 EXP statistics, still need to input the MaxDY, MinY
     * and MaxY in the future. */
    for (i = 0; i < numRegions; i++) {
      /* Either 64 or 256 regions processed here */

      /* 24 bits sum of Y. */
      vfeStatStruct->aec_op.SY[i] = ((*(current_region + 4)) & 0x1FFFFFF);

      /* 24 bits sum of Y that meets boundary conditions */
      vfeStatStruct->awb_op.SY1[i] = ((*(current_region + 0)) & 0x1FFFFFF);

      /* Assemble Sub Cb from two words to produce the 24 bit value */
      vfeStatStruct->awb_op.SCb[i] = ((*(current_region + 1)) & 0x1FFFFFF);

      /* Assemble Sub Cr from two words to produce the 24 bit value */
      vfeStatStruct->awb_op.SCr[i] = ((*(current_region + 2)) & 0x1FFFFFF);

      /* Number of pixels from this region that meet the inequalities */
      vfeStatStruct->awb_op.NSCb[i] = ((*(current_region + 3)) & 0x1FFFF);

      current_region += 8;	/* each region is 8 32 bit words or 32 bytes long */
      /* increment int32_t pointer to next region         */
//      CDBG("index : %d :: SY : %d, SY1 : %d, SCb : %d, SCr : %d, NSCb : %d",
//      i, vfeStatStruct->SY[i], vfeStatStruct->SY1[i], vfeStatStruct->SCb[i], vfeStatStruct->SCr[i], vfeStatStruct->NSCb[i]);
    }
    return numRegions;

  } else
        CDBG("%s: output Null pointer: %s %d ", __func__,
          __FILE__, __LINE__);

  //TODO: update 3A
  return FALSE;
} /* vfe_stats_parse_WB_EXP_stats_regions */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_WB_EXP -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_WB_EXP(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0;
  int i = 0;
  struct msm_stats_buf release_buf;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  legacy_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.legacy_stats_buf);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);

  CDBG("%s E: fd = %d\n", __func__, buf->fd);

  for (i = 0; i < AWB_AEC_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->AwbAecBuf[i].fd) {
      stats_buf_data->cur_AwbAecBuf = i;
      release_buf.type = STAT_AEAW;
      release_buf.buffer = (unsigned long) stats_buf_data->AwbAecBuf[i].buf;
      release_buf.fd = stats_buf_data->AwbAecBuf[i].fd;
      release_buf.length = stats_buf_data->AwbAecBuf[i].ion_alloc.len;
      release_buf.handle = stats_buf_data->AwbAecBuf[i].fd_data.handle;
      break;
    }
  }
  if (i == AWB_AEC_STATS_BUFNUM) {
    CDBG_ERROR("%s: AEC_AWB_STATS buffer mismatch: fd = %d\n",__func__, buf->fd);
    rc = -ENOENT;
  }

  if (!isp_started) {
    /*do nothing, just ack the buffer*/
    CDBG("%s Camera not started yet. Just ACK the buffer ", __func__);
  } else {
      stats_output->numRegions = vfe_stats_parse_WB_EXP_stats_regions(p_obj,
                                   stats_output);
      stats_output->aec_params.rgn_width =
        p_obj->vfe_module.stats.aecawb_stats.rgn_width;
      stats_output->aec_params.rgn_height =
        p_obj->vfe_module.stats.aecawb_stats.rgn_height;
      CDBG("VFE_ID_STATS_WB_EXP numReg %d\n", stats_output->numRegions);
  }

  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_AE_AW,
         stats_buf_data->cur_AwbAecBuf, (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: vfe_stats_release_buf failed : state %d\n", __func__,
      isp_started);
  }

  stats_output->wbexp_done = TRUE;
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_WB_EXP */

#endif

/*===========================================================================
 * FUNCTION    - vfe_stats_parse_AF_stats_regions -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_stats_parse_AF_stats_regions(vfe_ctrl_info_t *p_obj,
  vfe_stats_output_t *p_stats_output)
{
  int window;
  uint32_t i, j;
  uint8_t *autofocusMultiWindowGrid = p_obj->vfe_params.af_params.multi_roi_win;
  uint32_t focus_value[NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS];
  //TODO: need to add check for the type of buf
  legacy_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.legacy_stats_buf);
  isp_stats_t *vfe_stats_struct = &(p_stats_output->vfe_stats_struct);

  uint32_t *afStatsOutputPtr =
    (uint32_t *)stats_buf_data->AfBuf[stats_buf_data->cur_AFBuf].buf;
  /* Save number of focus values */
  uint32_t af_focus_value_count;

  for (i = 0; i < NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS; i++)
    focus_value[i] = 0;

  vfe_stats_struct->af_op.Focus = 0;
  /* Array out of bound check */
  if (vfe_stats_struct->af_op.NFocus > AUTOFOCUS_STATS_BUFFER_MAX_ENTRIES)
    vfe_stats_struct->af_op.NFocus = AUTOFOCUS_STATS_BUFFER_MAX_ENTRIES;

  if (autofocusMultiWindowGrid == NULL) {
    /* Single window stats */
    CDBG("%s: Single window stats\n", __func__);
#ifdef VFE_2X
    afStatsOutputPtr++;
    for (i = 0; (uint32_t) i < vfe_stats_struct->af_op.NFocus; i++)
        vfe_stats_struct->af_op.Focus += *afStatsOutputPtr++;
#else
    vfe_stats_struct->af_op.Focus = *afStatsOutputPtr;
    vfe_stats_struct->af_op.Focus = vfe_stats_struct->af_op.Focus << 1;
#endif
  } else {

    CDBG("%s: Multi window stats\n", __func__);
    i = 0;
    uint32_t temp = 0;
    for (j = 0; j < NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS * NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS; j++) {
      temp = *afStatsOutputPtr++;
      afStatsOutputPtr++;
      if (j == autofocusMultiWindowGrid[i]) {

        vfe_stats_struct->af_op.Focus += temp << 1;
        CDBG("%s: j=%d temp = %d vfeStatStruct->Focus=%d", __func__, j,
          temp, vfe_stats_struct->af_op.Focus);
        i++;
      }
    }
  }

  vfe_stats_struct->af_op.NFocus = p_obj->vfe_stats_struct.af_op.NFocus;
  CDBG("%s:Focus=%d NFocus=%d \n", __func__, vfe_stats_struct->af_op.Focus,
    vfe_stats_struct->af_op.NFocus);
} /* vfe_stats_parse_AF_stats_regions */

/*===========================================================================
 * FUNCTION    - vfe_stats_proc_MSG_ID_STATS_AF -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_stats_proc_MSG_ID_STATS_AF(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output)
{
  int rc = 0;
  int i = 0;
  struct msm_stats_buf release_buf;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)vfe_ctrl_obj;
  legacy_stats_buffer_t *stats_buf_data =
    &(p_obj->vfe_module.stats.parser_mod.stats_bufs.s.legacy_stats_buf);
  af_stats_t *stats_mod = &(p_obj->vfe_module.stats.af_stats);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);
  struct msm_cam_evt_divert_frame div_frame;
  vfe_stats_af_struct_t *parse_op_buf = NULL;

  CDBG("%s E: fd = %d\n", __func__, buf->fd);
  for (i = 0; i < AF_STATS_BUFNUM; ++i) {
    if (buf->fd == stats_buf_data->AfBuf[i].fd) {
      stats_buf_data->cur_AFBuf = i;
      release_buf.buffer = (unsigned long)stats_buf_data->AfBuf[i].buf;
      release_buf.fd = stats_buf_data->AfBuf[i].fd;
      release_buf.handle = stats_buf_data->AfBuf[i].fd_data.handle;
      release_buf.length = stats_buf_data->AfBuf[i].ion_alloc.len;
      break;
    }
  }

  if (i == AF_STATS_BUFNUM) {
    CDBG_ERROR("%s: AF_STATS buffer mismatch: fd = %d \n", __func__, buf->fd);
    rc = -ENOENT;
    return rc;
  }

  if (!isp_started) {
    /*do nothing, just ack the buffer*/
  } else {
    vfe_stats_parse_AF_stats_regions(p_obj, stats_output);
    /* Hand over the next stats buffer to VFE */
    CDBG("curAfBuf = %d, releasing AF STATS buffer = %lu\n",
      stats_buf_data->cur_AFBuf, release_buf.buffer);
    if(stats_mod->use_hal_buf) {
      if(p_obj->ops->get_stats_op_buffer) {
        parse_op_buf = p_obj->ops->get_stats_op_buffer(p_obj->ops->parent,
        &div_frame, MSM_STATS_TYPE_AF);
      } else {
        CDBG_HIGH("%s: get_stats_op_buffer failed\n", __func__);
      }
      CDBG("%s: buffer : %p\n", __func__, parse_op_buf);
      if(parse_op_buf != NULL) {
        parse_op_buf->cm_data.height = 0;
        parse_op_buf->cm_data.width = 0;
        parse_op_buf->cm_data.stats_type = MSM_STATS_TYPE_AF;
        parse_op_buf->cm_data.stats_version = 0;
        parse_op_buf->cm_data.frame_id = 0;

        memcpy(&(parse_op_buf->af),
          &(stats_output->vfe_stats_struct.af_op),
          sizeof(vfe_stats_af_data_t));

        if(p_obj->ops->put_stats_op_buffer) {
          p_obj->ops->put_stats_op_buffer(p_obj->ops->parent,
            &div_frame, MSM_STATS_TYPE_AF);
        } else
          CDBG_HIGH("%s: put_stats_op_buffer failed\n", __func__);
      }
    }
  }

  rc = vfe_stats_release_buf(p_obj, MSM_STATS_TYPE_AF, stats_buf_data->cur_AFBuf,
         (void *)&release_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: vfe_stats_release_buf failed\n", __func__);
  }

  stats_output->af_bf_done = TRUE;
  CDBG("%s: X rc = %d", __func__, rc);
  return rc;
} /* vfe_stats_proc_MSG_ID_STATS_AF */

static void vfe_stats_dump(void *vaddr, uint32_t len, int msg_id)
{
  static int bg_cnt = 0;
  static int be_cnt = 0;
  static int bf_cnt = 0;
  static int bhist_cnt = 0;
  char bufp[128];
  int file_fdp;

  switch(msg_id) {
  case MSG_ID_STATS_BG:
    sprintf(bufp, "/data/bg_%d.yuv", bg_cnt++);
    if(bg_cnt > 50) {
      return;
    }
    break;
  case MSG_ID_STATS_BE:
    sprintf(bufp, "/data/be_%d.yuv", be_cnt++);
    if(be_cnt > 50) {
      return;
    }
    break;
  case MSG_ID_STATS_BF:
    sprintf(bufp, "/data/bf_%d.yuv", bf_cnt++);
    if(bf_cnt > 50) {
      return;
    }
    break;
  case MSG_ID_STATS_BHIST:
    sprintf(bufp, "/data/bhist_%d.yuv", bhist_cnt++);
    if(bhist_cnt > 50) {
      return;
    }
    break;
  default:
    return;
  }
  file_fdp = open(bufp, O_RDWR | O_CREAT, 0777);
  if (file_fdp < 0) {
    CDBG("cannot open file %s\n", bufp);
    goto end;
  }
  CDBG("%s:dump frame to '%s'\n", __func__, bufp);
  write(file_fdp,(const void *)vaddr, len);
  close(file_fdp);
end:
  return;
}

/*===========================================================================
 * FUNCTION    - vfe_af_handle_default_roi
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t vfe_af_handle_default_roi(vfe_ctrl_info_t *p_obj)
{
  int8_t rc = 0;
  isp_stats_t *vfe_stats_struct = &(p_obj->vfe_stats_struct);
  vfe_af_params_t *af_params = &(p_obj->vfe_params.af_params);
  chromatix_parms_type *chromatix_ptr = p_obj->vfe_params.chroma3a;
  unsigned int fovcrop_width, fovcrop_height;
#ifndef VFE_40
  uint32_t first_pixel = p_obj->vfe_module.fov_mod.fov_cmd.firstPixel;
  uint32_t last_pixel = p_obj->vfe_module.fov_mod.fov_cmd.lastPixel;
  uint32_t first_line = p_obj->vfe_module.fov_mod.fov_cmd.firstLine;
  uint32_t last_line = p_obj->vfe_module.fov_mod.fov_cmd.lastLine;
#else
  uint32_t first_pixel =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.firstPixel;
  uint32_t last_pixel =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.lastPixel;
  uint32_t first_line =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.firstLine;
  uint32_t last_line =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.lastLine;
#endif
  fovcrop_width = last_pixel - first_pixel + 1;
  fovcrop_height = last_line - first_line + 1;

  CDBG("%s: AF stats for Default window\n", __func__);

#ifndef VFE_2X
  af_params->shift_bits = 1;
  uint32_t rgnWidth = ((uint32_t)(fovcrop_width *
    chromatix_ptr->af_config.horizontalClipRatio) >> 1) << 1;
  af_params->rgn_width = rgnWidth - 1;
  af_params->rgn_hoffset =
    ((fovcrop_width -  rgnWidth) / 2 + first_pixel + 2);
  uint32_t rgnHeight = ((uint32_t)(fovcrop_height *
    chromatix_ptr->af_config.verticalClipRatio) >> 1) << 1;

  CDBG("%s: rgnWidth=%d,rgnHeight=%d,fovcrop_width=%d,fovcrop_height=%d,"
    "HClipRatio=%f, VClipRatio=%f,firstPixel=%d,firstLine=%d\n", __func__,
    rgnWidth, rgnHeight, fovcrop_width, fovcrop_height,
    chromatix_ptr->af_config.horizontalClipRatio,
    chromatix_ptr->af_config.verticalClipRatio,
    first_pixel, first_line);

  af_params->rgn_height = rgnHeight - 1;
  af_params->rgn_voffset =
    ((fovcrop_height - rgnHeight) / 2 +
    first_line + 1);

  af_params->rgn_hnum  = 0;
  af_params->rgn_vnum  = 0;
#else
  //TODO: Get from active crop info.
  af_params->rgn_height = fovcrop_height >>1;
  af_params->rgn_width = fovcrop_width >> 1;
  af_params->rgn_voffset = af_params->rgn_height >> 1;
  af_params->rgn_hoffset = af_params->rgn_width >> 1;
#endif

  vfe_stats_struct->af_op.NFocus = (af_params->rgn_height >> 1) -1;
  return TRUE;
} /* vfe_af_handle_default_roi_mode */

/*===========================================================================
 * FUNCTION    - vfe_af_handle_single_roi
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t vfe_af_handle_single_roi(vfe_ctrl_info_t *p_obj,
  vfe_stats_af_params_t *ip_af_params)
{
  int rc = TRUE;
  uint32_t diff_horizontal, diff_vertical;
  unsigned int fovcrop_width, fovcrop_height;
  isp_stats_t *vfe_stats_struct = &(p_obj->vfe_stats_struct);
  vfe_af_params_t *af_params = &(p_obj->vfe_params.af_params);
  roi_t *region = &(ip_af_params->region);
  uint32_t frame_width = ip_af_params->frame_width;
  uint32_t frame_height = ip_af_params->frame_height;
  uint32_t camif_width = ip_af_params->camif_width;
  uint32_t camif_height = ip_af_params->camif_height;

  CDBG("%s: AF stats for Single ROI\n", __func__);

#ifndef VFE_40
  uint32_t first_pixel = p_obj->vfe_module.fov_mod.fov_cmd.firstPixel;
  uint32_t last_pixel = p_obj->vfe_module.fov_mod.fov_cmd.lastPixel;
  uint32_t first_line = p_obj->vfe_module.fov_mod.fov_cmd.firstLine;
  uint32_t last_line = p_obj->vfe_module.fov_mod.fov_cmd.lastLine;
#else
  uint32_t first_pixel =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.firstPixel;
  uint32_t last_pixel =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.lastPixel;
  uint32_t first_line =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.firstLine;
  uint32_t last_line =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.lastLine;
#endif

  fovcrop_width = last_pixel - first_pixel + 1;
  fovcrop_height = last_line - first_line + 1;

  CDBG("%s: fovcrop_width/height: %d x %d camif_width/height:"
    "%d x %d frame_width/height: %d x %d", __func__,
    fovcrop_width, fovcrop_height, camif_width, camif_height,
    frame_width, frame_height);

  CDBG("%s: region: %d %d %d %d", __func__,
    region->x, region->y, region->x + region->dx, region->y + region->dy);

  /* Check if region sizes are same as frame sizes. If yes we'll use
     default mode. Also if both are zero, we'll use default mode. */
  if ((((region->x + region->dx) >= frame_width) &&
    ((region->y + region->dy) >= frame_height)) ||
    (!(region->x + region->dx) && !(region->y + region->dy))) {
    return vfe_af_handle_default_roi(p_obj);
  }

  /* Scale the AF window dimension and offset accordingly */
  af_params->rgn_voffset =
    region->y * fovcrop_height / frame_height;
  af_params->rgn_hoffset =
    region->x * fovcrop_width  / frame_width;

  diff_horizontal = (camif_width - fovcrop_width) / 2;
  diff_vertical = (camif_height - fovcrop_height) / 2;
  /* voffset and hoffset should be with respect to CAMIF */
  af_params->rgn_voffset += diff_vertical;
  af_params->rgn_hoffset += diff_horizontal;

  CDBG("%s: %d = %d * %d / %d\n", __func__,
    af_params->rgn_hoffset,
    region->x, fovcrop_width, frame_width);

#ifdef VFE_2X
  uint32_t rgnHeight =
    ((uint32_t)(region->dy * fovcrop_height / frame_height));
   af_params->rgn_height = rgnHeight;// - 1;

   uint32_t rgnWidth =
     ((uint32_t)(region->dx * fovcrop_width  / frame_width));
   af_params->rgn_width = rgnWidth;// - 1;
#else

  uint32_t rgnHeight =
    ((uint32_t)(region->dy * fovcrop_height / frame_height)>> 1) << 1;
  af_params->rgn_height = rgnHeight - 1;

  uint32_t rgnWidth =
    ((uint32_t)(region->dx * fovcrop_width  / frame_width)>> 1) << 1;
  af_params->rgn_width = rgnWidth - 1;

  if ((uint32_t)(af_params->rgn_hoffset +
    (af_params->rgn_hnum+ 1) *
    (af_params->rgn_width + 1)) >
    camif_width) {
    uint32_t delta_adjust_width =
    (af_params->rgn_hoffset +
    (af_params->rgn_hnum + 1) *
    (af_params->rgn_width + 1)) - camif_width + 5;
    af_params->rgn_hoffset =
      af_params->rgn_hoffset - delta_adjust_width;
  }
  if ((uint32_t)(af_params->rgn_voffset +
    (af_params->rgn_vnum + 1) *
    (af_params->rgn_height + 1)) >
    camif_height) {
    uint32_t delta_adjust_height =
      (af_params->rgn_voffset +
      (af_params->rgn_vnum + 1) *
      (af_params->rgn_height + 1)) - camif_height + 5;
      af_params->rgn_voffset =
        af_params->rgn_voffset - delta_adjust_height;
  }

  if (((uint32_t)(af_params->rgn_hoffset +
    (af_params->rgn_hnum + 1) *
    (af_params->rgn_width + 1)) >
    camif_width)
    ||
    (uint32_t)(af_params->rgn_voffset +
    (af_params->rgn_vnum + 1) *
    (af_params->rgn_height + 1)) >
    camif_height) {
    CDBG("%s:%d: rgnHOffset=%d rgnVOffset=%d co-orinates\
      are out of range.\n",__func__, __LINE__,
      af_params->rgn_hoffset,
      af_params->rgn_voffset);
      return FALSE;
    }
#endif

  /* Limit it to execution region */
  if (af_params->rgn_hoffset < 4) {
    af_params->rgn_hoffset = 4;
    /* adjust rgn_width so that it doesn't extend beyond fov width limit */
    af_params->rgn_width -= 4;
  }

  if (af_params->rgn_hoffset > 4091) {
    af_params->rgn_hoffset = 4091;
  }

  if (af_params->rgn_voffset < 2) {
    af_params->rgn_voffset = 2;
    /* adjust rgn_height so that it doesn't extend beyond fov height limit */
    af_params->rgn_height -= 2;
  }

  if (af_params->rgn_voffset > 4092) {
    af_params->rgn_voffset = 4092;
  }

  vfe_stats_struct->af_op.NFocus = (af_params->rgn_height >> 1) -1;

  CDBG("%s: after: region width height H off and V off = %d %d %d %d",
    __func__,
    af_params->rgn_width,
    af_params->rgn_height,
    af_params->rgn_hoffset,
    af_params->rgn_voffset);

  return TRUE;
}/* vfe_af_handle_single_roi */

/*===========================================================================
 * FUNCTION    - vfe_af_handle_multiple_roi
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t vfe_af_handle_multiple_roi(
  vfe_ctrl_info_t *p_obj,
  vfe_stats_af_params_t *ip_af_params)
{
  int8_t rc;
  unsigned int fovcrop_width, fovcrop_height;
  isp_stats_t *vfe_stats_struct = &(p_obj->vfe_stats_struct);
  vfe_af_params_t *af_params = &(p_obj->vfe_params.af_params);
  CDBG("%s: AF stats for Multiple ROI\n", __func__);

#ifndef VFE_40
  uint32_t first_pixel = p_obj->vfe_module.fov_mod.fov_cmd.firstPixel;
  uint32_t last_pixel = p_obj->vfe_module.fov_mod.fov_cmd.lastPixel;
  uint32_t first_line = p_obj->vfe_module.fov_mod.fov_cmd.firstLine;
  uint32_t last_line = p_obj->vfe_module.fov_mod.fov_cmd.lastLine;
#else
  uint32_t first_pixel =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.firstPixel;
  uint32_t last_pixel =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.lastPixel;
  uint32_t first_line =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.firstLine;
  uint32_t last_line =
    p_obj->vfe_module.fov_mod.fov_enc_cmd.y_crop_cfg.lastLine;
#endif

  fovcrop_width = last_pixel - first_pixel + 1;
  fovcrop_height = last_line - first_line + 1;
  af_params->multi_roi_win = ip_af_params->af_mult_window;
  vfe_stats_struct->af_op.NFocus = ip_af_params->multi_roi_nfocus;

#ifndef VFE_2X
  uint32_t partitions = 11;
  uint32_t regions = 9;

  af_params->shift_bits = 1;
  af_params->rgn_vnum = regions - 1;
  af_params->rgn_hnum = regions - 1;
  af_params->rgn_voffset =
    FLOOR2(fovcrop_height / partitions) + 1;
  if (af_params->rgn_voffset < 2)
    af_params->rgn_voffset = 2;

  af_params->rgn_hoffset =
    FLOOR2(fovcrop_width / partitions) + 2;

  if (af_params->rgn_hoffset < 4)
    af_params->rgn_hoffset = 4;

  uint32_t tmpRgnHeight = FLOOR2(fovcrop_height / partitions);
  af_params->rgn_height = tmpRgnHeight - 1;

  uint32_t tmpRgnWidth = FLOOR2(fovcrop_width / partitions);
  af_params->rgn_width = tmpRgnWidth - 1;
#else
  af_params->rgn_height = fovcrop_height;
  af_params->rgn_width = fovcrop_width;
#endif

  return TRUE;
} /* vfe_af_handle_multiple_roi */

/*===========================================================================
 * FUNCTION    - vfe_af_calc_roi
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_af_calc_roi(void *p_obj,
  vfe_stats_af_params_t *ip_af_params)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_ctrl_info_t *vfe_ctrl_obj = (vfe_ctrl_info_t *)p_obj;
  CDBG("%s E\n", __func__);
  vfe_ctrl_obj->vfe_params.af_params.multi_roi_win =
    ip_af_params->af_mult_window;

  switch(ip_af_params->roi_type) {
  case AF_STATS_CONFIG_MODE_DEFAULT:
    vfe_af_handle_default_roi(vfe_ctrl_obj);
    break;
  case AF_STATS_CONFIG_MODE_SINGLE:
    vfe_af_handle_single_roi(vfe_ctrl_obj, ip_af_params);
    break;
  case AF_STATS_CONFIG_MODE_MULTIPLE:
    vfe_af_handle_multiple_roi(vfe_ctrl_obj, ip_af_params);
    break;
  default:
    CDBG_ERROR(":%s: Invalid roi type error\n", __func__);
    break;
  }
  CDBG("%s X\n", __func__);
  return VFE_SUCCESS;
}
