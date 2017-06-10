/*============================================================================
Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <linux/media.h>
#include <cutils/log.h>
#include <poll.h>
#include <media/msmb_isp.h>
#include <errno.h>
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "camera_dbg.h"
#include "isp_log.h"

uint8_t *do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int *mapFd)
{
  void *ret; /* returned virtual address */
  int rc = 0;
  struct ion_handle_data handle_data;

  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    CDBG_ERROR("ION allocation failed\n");
    goto ION_ALLOC_FAILED;
  }

  ion_info_fd->handle = alloc->handle;
  rc = ioctl(ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    CDBG_ERROR("ION map failed %s\n", strerror(errno));
    goto ION_MAP_FAILED;
  }
  *mapFd = ion_info_fd->fd;
  ret = mmap(NULL,
    alloc->len,
    PROT_READ  | PROT_WRITE,
    MAP_SHARED,
    *mapFd,
    0);

  if (ret == MAP_FAILED) {
    CDBG_ERROR("ION_MMAP_FAILED: %s (%d)\n", strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  return ret;

ION_MAP_FAILED:
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
ION_ALLOC_FAILED:
  return NULL;
}

int do_munmap_ion (int ion_fd, struct ion_fd_data *ion_info_fd,
                   void *addr, size_t size)
{
  int rc = 0;
  rc = munmap(addr, size);
  close(ion_info_fd->fd);

  struct ion_handle_data handle_data;
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
  return rc;
}

struct isp_frame_buffer {
  struct v4l2_buffer buffer;
  unsigned long addr[VIDEO_MAX_PLANES];
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
};

struct isp_plane_alloc_info {
  unsigned long len;
  uint32_t offset;
};

struct isp_buf_alloc_info {
  struct isp_plane_alloc_info plane_info[VIDEO_MAX_PLANES];
  int num_planes;
};

int isp_init_buffer(struct isp_frame_buffer *buf,
          int ion_fd, struct isp_buf_alloc_info *alloc_info)
{
  int current_fd = -1, i;
  unsigned long current_addr = 0;
  memset(buf, 0, sizeof(struct isp_frame_buffer));
  buf->buffer.m.planes = malloc(
   sizeof(struct v4l2_plane) * alloc_info->num_planes);
  if (!buf->buffer.m.planes) {
    ISP_DBG(ISP_MOD_COM,"%s: no mem\n", __func__);
    return -1;
  }

  for (i = 0; i < alloc_info->num_planes; i++)
  {
    if (alloc_info->plane_info[i].offset == 0)
    {
      buf->ion_alloc[i].len = alloc_info->plane_info[i].len;
      buf->ion_alloc[i].flags = 0;
      buf->ion_alloc[i].heap_id_mask = 0x1 << ION_IOMMU_HEAP_ID;
      buf->ion_alloc[i].align = 4096;
      current_addr = (unsigned long) do_mmap_ion(ion_fd,
        &(buf->ion_alloc[i]), &(buf->fd_data[i]), &current_fd);
      if (!current_addr) {
        CDBG_ERROR("do_mmap_ion function return NULL\n");
        return -1;
      }
      memset((void *) current_addr, 128, buf->ion_alloc[i].len);
    }
    buf->addr[i] = current_addr + alloc_info->plane_info[i].offset;
    buf->buffer.m.planes[i].m.userptr = current_fd;
    buf->buffer.m.planes[i].data_offset = alloc_info->plane_info[i].offset;
  }
  buf->buffer.length = alloc_info->num_planes;
  return 0;
}

void isp_release_buffer(struct isp_frame_buffer *buf, int ion_fd)
{
  int i;
  for (i = 0; i < (int)buf->buffer.length; i++)
  {
    if (buf->ion_alloc[i].len != 0)
      do_munmap_ion(ion_fd, &(buf->fd_data[0]),
        (void *)buf->addr[i], buf->ion_alloc[i].len);
  }
  free(buf->buffer.m.planes);
}

void isp_test_func(const char *dev_name, int fd, int use_local_buf, uint32_t session_id, uint32_t stream_id) {
  int vfe_fd = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[11];
  int i, rc = 0;
  struct isp_frame_buffer buffer[10];

  if (fd == 0)
    vfe_fd = open(dev_name, O_RDWR | O_NONBLOCK);
  else
    vfe_fd = fd;
  ISP_DBG(ISP_MOD_COM,"%s: %d, fd = %d\n", __func__, __LINE__, vfe_fd);
#if 0
  uint32_t reset_data;
  reset_data = 0x1FF;// A_VFE_0_GLOBAL_RESET_CMD

  cfg_cmd.cfg_data = (void *) &reset_data;
  cfg_cmd.cmd_len = sizeof(uint32_t);
  cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
  cfg_cmd.num_cfg = 1;

  reg_cfg_cmd[0].cmd_data = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[0].len = sizeof(uint32_t);
  reg_cfg_cmd[0].reg_offset = 0xC;
  rc = ioctl(vfe_fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  ISP_DBG(ISP_MOD_COM,"%s: %d, rc = %d, VIDIOC_MSM_VFE_REG_CFG\n", __func__, __LINE__, rc);
#endif
  usleep(1000000);

  int ion_fd = 0;
  ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
  if (ion_fd < 0) {
    if(fd == 0)
      close(vfe_fd);
    CDBG_ERROR("Ion device open failed\n");
    return;
  }
  uint32_t sensor_width, sensor_height;
  uint32_t crop_width, crop_height;
  uint32_t camif_width, camif_height;

  sensor_width = 1984;
  sensor_height = 1508;
  crop_width = 0;
  crop_height = 0;
  camif_width = sensor_width - crop_width;
  camif_height = sensor_height - crop_height;

  struct isp_buf_alloc_info alloc_info;
  alloc_info.num_planes = 2;
  alloc_info.plane_info[0].len = camif_width * camif_height * 1.5;
  alloc_info.plane_info[0].offset = 0;
  alloc_info.plane_info[1].len = 0;//640 * 480 / 2;
  alloc_info.plane_info[1].offset = camif_width * camif_height;

  if (use_local_buf == 1) {
    for (i = 0; i < 10; i++)
    {
      isp_init_buffer(&buffer[i], ion_fd, &alloc_info);
    }

    struct msm_isp_buf_request buf_request;
    buf_request.session_id = session_id;
    buf_request.stream_id = stream_id;// + (1 <<16);
    buf_request.num_buf = 10;
    buf_request.handle = 0;
    rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_REQUEST_BUF, &buf_request);
    ISP_DBG(ISP_MOD_COM,"%d, rc = %d\n", buf_request.handle, rc);
/*
    struct msm_isp_qbuf_info qbuf_info;
    qbuf_info.handle = buf_request.handle;
    for (i = 0; i < 10; i++)
    {
      qbuf_info.buffer = buffer[i].buffer;
      qbuf_info.buf_idx = i;
      rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_ENQUEUE_BUF, &qbuf_info);

    }
*/
  }

  struct msm_vfe_axi_stream_request_cmd stream_req;
  stream_req.axi_stream_handle = 0;
  stream_req.session_id = session_id;
  stream_req.stream_id = stream_id;// + (1 <<16);
  /*
  stream_req.output_format = V4L2_PIX_FMT_SBGGR8;
  stream_req.stream_src = CAMIF_RAW;
  */
  stream_req.output_format = V4L2_PIX_FMT_NV21;
  stream_req.stream_src = PIX_ENCODER;
  stream_req.plane_cfg[0].output_plane_format = Y_PLANE;
  stream_req.plane_cfg[0].output_width = camif_width;
  stream_req.plane_cfg[0].output_height = camif_height;
  stream_req.plane_cfg[0].output_stride = camif_width;
  stream_req.plane_cfg[0].output_scan_lines = camif_height;
  stream_req.plane_cfg[1].output_plane_format = CRCB_PLANE;
  stream_req.plane_cfg[1].output_width = camif_width;
  stream_req.plane_cfg[1].output_height = camif_height / 2;
  stream_req.plane_cfg[1].output_stride = camif_width;
  stream_req.plane_cfg[1].output_scan_lines = camif_height / 2;

  rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_REQUEST_STREAM, &stream_req);

  struct msm_vfe_axi_stream_cfg_cmd stream_cfg;
  stream_cfg.cmd = START_STREAM;
  stream_cfg.num_streams = 1;
  stream_cfg.stream_handle[0] = stream_req.axi_stream_handle;
  rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_CFG_STREAM, &stream_cfg);

  uint32_t data2[][2] = {
    //{0x010, 0x3fffffff},//VFE_MODULE_RESET
    //{0x010, 0x00000000},//VFE_MODULE_RESET
    //{0x014, 0x3fffffff},//VFE_CGC_OVERRIDE
    {0x018, 0x08821014},//VFE_MODULE_CFG
    {0x01C, 0x00000000},//VFE_CORE_CFG
    {0x050, 0x00000001},//VFE_BUS_CFG
    {0x054, 0x00000000},//VFE_BUS_IO_FORMAT_CFG
    {0x2e8, 0x00000003},//VFE_RDI_CFG_0
    /*IRQ MASK*/
    //{0x028, 0xffff00f1},
    //{0x02C, 0xfffffeff},
    /*CAMIF*/
    {0x2f8, 0x00000040},//VFE_CAMIF_CFG
    {0x300, sensor_height << 16 | sensor_width},//VFE_CAMIF_FRAME_CFG
    {0x304, camif_width - 1},//VFE_CAMIF_WINDOW_WIDTH_CFG
    {0x308, camif_height - 1},//VFE_CAMIF_WINDOW_HEIGHT_CFG
    {0x314, 0xffffffff},//VFE_CAMIF_IRQ_SUBSAMPLE_PATTERN
    /*TEST GEN*/
    {0x934, 0x00000000},//VFE_TESTGEN_CFG
    {0x940, 0x0000001F},//VFE_HW_TESTGEN_CFG
    {0x944, (sensor_height - 1) << 16 | (sensor_width - 1)},//VFE_HW_TESTGEN_IMAGE_CFG
    {0x948, 0x00000000},//VFE_HW_TESTGEN_SOF_OFFSET_CFG
    {0x94c, 0x00000000},//VFE_HW_TESTGEN_EOF_NOFFSET_CFG
    {0x950, 0x00000000},//VFE_HW_TESTGEN_SOL_OFFSET_CFG
    {0x954, 0x00000000},//VFE_HW_TESTGEN_EOL_NOFFSET_CFG
    {0x958, 0x000005ff},//VFE_HW_TESTGEN_HBI_CFG
    {0x95c, 0x00010FFF},//VFE_HW_TESTGEN_VBL_CFG
    {0x960, 0x00000000},//VFE_HW_TESTGEN_SOF_DUMMY_LINE_CFG
    {0x964, 0x00000000},//VFE_HW_TESTGEN_EOF_DUMMY_LINE_CFG
    {0x968, 0x00000100},//VFE_HW_TESTGEN_COLOR_BARS_CFG
    {0x96c, 0x00000000},//VFE_HW_TESTGEN_RANDOM_CFG
    /*DEMUX*/
    {0x424, 0x00000001},
    {0x428, 0x00800080},
    {0x42C, 0x00800080},
    {0x430, 0x00800080},
    {0x434, 0x00800080},
    {0x438, 0x000000c9},
    {0x43C, 0x000000ac},
    /*DEMOSAIC*/
    {0x440, 0x00000000},
    /*CLF*/
    {0x588, 0x00000001},
    /*COLOR_CONVERT*/
    {0x640, 0x0000004c},
    {0x644, 0x00000096},
    {0x648, 0x0000001d},
    {0x64C, 0x00000000},
    /*Chroma Enhancement*/
    {0x650, 0x0072005e},
    {0x654, 0x0fa90fa3},
    {0x658, 0x00670072},
    {0x65C, 0x0fce0fc8},
    {0x660, 0x00800080},
    /*CLAMP*/
    {0x874, 0x00ffffff},
    {0x878, 0x00000000},
    {0x87C, 0x00ffffff},
    {0x880, 0x00000000},
    /*Scaler*/
    {0x778, 0x00000003},
    {0x77C, (camif_width/2) << 16 | camif_width},
    {0x780, 0x00320000},
    {0x784, 0x00000000},
    {0x788, 0x00000000},
    {0x78C, 0x00000000},
    {0x790, (camif_height/2) << 16 | camif_height},
    {0x794, 0x00320000},
    {0x798, 0x00000000},
    {0x79C, 0x00000000},
    {0x7A0, 0x00000000},
    /*REG UPDATE*/
    {0x378, 0x00000001},
    /*CAMIF START*/
    {0x2f4, 0x00000001},
    /*TESTGEN CMD*/
    //{0x93c, 0x00000001},
  };

  for (i = 0; i < (int)(sizeof(data2)/sizeof(data2[0])); i++)
  {
    uint32_t temp_data = data2[i][1];
    cfg_cmd.cfg_data = (void *) &temp_data;
    cfg_cmd.cmd_len = sizeof(uint32_t);
    cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[0].u.rw_info.len = sizeof(uint32_t);
    reg_cfg_cmd[0].u.rw_info.reg_offset = data2[i][0];
    rc = ioctl(vfe_fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  }

   ISP_DBG(ISP_MOD_COM,"dump frame\n");
   if (0) {
    int out_file_fd;
    out_file_fd = open("/data/misc/camera/vfe_dump0.yuv", O_RDWR | O_CREAT, 0777);
    if (out_file_fd < 0) {
      ISP_DBG(ISP_MOD_COM,"Cannot open file, errno: %s\n", strerror(errno));
    }
    write(out_file_fd, (const void *) buffer[0].addr[0], buffer[0].ion_alloc[0].len);
    close(out_file_fd);
    out_file_fd = open("/data/misc/camera/vfe_dump1.raw", O_RDWR | O_CREAT, 0777);
    if (out_file_fd < 0) {
      ISP_DBG(ISP_MOD_COM,"Cannot open file\n");
    }
    write(out_file_fd, (const void *) buffer[1].addr[0], buffer[1].ion_alloc[0].len);
    close(out_file_fd);
    ISP_DBG(ISP_MOD_COM,"dump frame done\n");

    for (i = 0; i < 10; i++)
    {
      isp_release_buffer(&buffer[i], ion_fd);
    }
  }
  ISP_DBG(ISP_MOD_COM,"%s: X, %d\n", __func__, __LINE__);

  close(ion_fd);
  if(fd == 0)
    close(vfe_fd);
}

