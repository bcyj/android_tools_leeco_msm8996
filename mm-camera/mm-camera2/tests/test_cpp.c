/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <linux/ion.h>
#include <linux/msm_ion.h>
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "mct_pipeline.h"
#include "mct_module.h"
#include "mct_stream.h"
#include "modules.h"
#include "pproc_port.h"
#include "cam_intf.h"
#include "camera_dbg.h"
#include <dlfcn.h>

#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

struct v4l2_frame_buffer {
  struct v4l2_buffer buffer;
  unsigned long addr[VIDEO_MAX_PLANES];
  uint32_t size;
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
};

pthread_cond_t  frame_done_cond;
pthread_mutex_t mutex;
boolean         frame_pending = FALSE;

typedef struct {
  uint32_t     input_width;
  uint32_t     input_height;
  uint32_t     output_width;
  uint32_t     output_height;
  uint32_t     process_window_first_pixel;
  uint32_t     process_window_first_line;
  uint32_t     process_window_width;
  uint32_t     process_window_height;
  uint16_t     rotation;
  uint16_t     mirror;
  double       h_scale_ratio;
  double       v_scale_ratio;
  char         input_filename[256];
  char         output_filename[256];
  char         chromatix_file[256];
  char         chromatix_common_file[256];
  double       noise_profile[4];
  double       luma_weight;
  double       chroma_weight;
  double       denoise_ratio;
  double       sharpness_ratio;
  int          run;
  float        lux_idx;
  float        real_gain;
} cpp_testcase_input_t;

uint8_t *do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int *mapFd)
{
  void                  *ret; /* returned virtual address */
  int                    rc = 0;
  struct ion_handle_data handle_data;
  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    CDBG_ERROR("ION allocation failed %s len %d\n", strerror(errno), alloc->len);
    goto ION_ALLOC_FAILED;
  }

  ion_info_fd->handle = alloc->handle;
  rc = ioctl(ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    CDBG_ERROR("ION map failed %s\n", strerror(errno));
    goto ION_MAP_FAILED;
  }
  *mapFd = ion_info_fd->fd;
  ret = mmap(NULL, alloc->len, PROT_READ | PROT_WRITE, MAP_SHARED, *mapFd, 0);
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

int do_munmap_ion(int ion_fd, struct ion_fd_data *ion_info_fd, void *addr,
  size_t size)
{
  int                    rc = 0;
  struct ion_handle_data handle_data;

  rc = munmap(addr, size);
  close(ion_info_fd->fd);
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
  return rc;
}

void dump_test_case_params(cpp_testcase_input_t *test_case)
{
  int i;
  CDBG("input_width: %d\n", test_case->input_width);
  CDBG("input_height: %d\n", test_case->input_height);
  CDBG("process_window_first_pixel: %d\n", test_case->process_window_first_pixel);
  CDBG("process_window_first_line: %d\n", test_case->process_window_first_line);
  CDBG("process_window_width: %d\n", test_case->process_window_width);
  CDBG("process_window_height: %d\n", test_case->process_window_height);
  CDBG("rotation: %d\n", test_case->rotation);
  CDBG("mirror: %d\n", test_case->mirror);
  CDBG("h_scale_ratio: %f\n", test_case->h_scale_ratio);
  CDBG("v_scale_ratio: %f\n", test_case->v_scale_ratio);
  CDBG("input_filename: %s\n", test_case->input_filename);
  CDBG("output_filename: %s\n", test_case->output_filename);
  for (i = 0; i < 4; i++) {
    CDBG("noise_profile[%d]: %f\n", i, test_case->noise_profile[i]);
  }
  CDBG("luma_weight: %f\n", test_case->luma_weight);
  CDBG("chroma_weight: %f\n", test_case->chroma_weight);
  CDBG("denoise_ratio: %f\n", test_case->denoise_ratio);
  CDBG("sharpness_ratio: %f\n", test_case->sharpness_ratio);
  CDBG("run: %d\n", test_case->run);
}

int parse_test_case_file(char **argv, cpp_testcase_input_t *test_case)
{
  char *filename = argv[1];
  char  type[256], value[256];
  FILE *fp;

  CDBG_HIGH("%s:%d] file name: %s\n", __func__, __LINE__, filename);
  test_case->run = 1;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    CDBG_ERROR("%s:%d] Cannot open test case file!\n", __func__, __LINE__);
    return -1;
  }
  while (!feof(fp)) {
    if (fscanf(fp, "%s %s", type, value) != 2)
      break;

    if (!strncmp(type, "input_width", 256)) {
      test_case->input_width = atoi(value);
    } else if (!strncmp(type, "input_height", 256)) {
      test_case->input_height = atoi(value);
    }else if (!strncmp(type, "output_width", 256)) {
      test_case->output_width = atoi(value);
    } else if (!strncmp(type, "output_height", 256)) {
      test_case->output_height = atoi(value);
    } else if (!strncmp(type, "process_window_first_pixel", 256)) {
      test_case->process_window_first_pixel = atoi(value);
    } else if (!strncmp(type, "process_window_first_line", 256)) {
      test_case->process_window_first_line = atoi(value);
    } else if (!strncmp(type, "process_window_width", 256)) {
      test_case->process_window_width = atoi(value);
    } else if (!strncmp(type, "process_window_height", 256)) {
      test_case->process_window_height = atoi(value);
    } else if (!strncmp(type, "rotation", 256)) {
      test_case->rotation = atoi(value);
    } else if (!strncmp(type, "mirror", 256)) {
      test_case->mirror = atoi(value);
    } else if (!strncmp(type, "h_scale_ratio", 256)) {
      test_case->h_scale_ratio = atof(value);
    } else if (!strncmp(type, "v_scale_ratio", 256)) {
      test_case->v_scale_ratio = atof(value);
    } else if (!strncmp(type, "input_filename", 256)) {
      strncpy(test_case->input_filename, value, 256);
    } else if (!strncmp(type, "output_filename", 256)) {
      strncpy(test_case->output_filename, value, 256);
    } else if (!strncmp(type, "chromatix_file", 256)) {
      strlcpy(test_case->chromatix_file, value, 256);
    } else if (!strncmp(type, "chromatix_common_file", 256)) {
      strlcpy(test_case->chromatix_common_file, value, 256);
    } else if (!strncmp(type, "noise_profile0", 256)) {
      test_case->noise_profile[0] = atof(value);
    } else if (!strncmp(type, "noise_profile1", 256)) {
      test_case->noise_profile[1] = atof(value);
    } else if (!strncmp(type, "noise_profile2", 256)) {
      test_case->noise_profile[2] = atof(value);
    } else if (!strncmp(type, "noise_profile3", 256)) {
      test_case->noise_profile[3] = atof(value);
    } else if (!strncmp(type, "luma_weight", 256)) {
      test_case->luma_weight = atof(value);
    } else if (!strncmp(type, "chroma_weight", 256)) {
      test_case->chroma_weight = atof(value);
    } else if (!strncmp(type, "denoise_ratio", 256)) {
      test_case->denoise_ratio = atof(value);
    } else if (!strncmp(type, "run", 256)) {
      test_case->run = atoi(value);
    } else if (!strncmp(type, "sharpness_ratio", 256)) {
      test_case->sharpness_ratio = atof(value);
    } else if (!strncmp(type, "lux_idx", 256)) {
      test_case->lux_idx = atof(value);
    } else if (!strncmp(type, "real_gain", 256)) {
      test_case->real_gain = atof(value);
    }
  }
  dump_test_case_params(test_case);
  return 0;
}

#define PPROC_TEST_INPUT_WIDTH 640
#define PPROC_TEST_INPUT_HEIGHT 480
#define PPROC_TEST_ALIGN_4K 4096

static boolean pproc_test_port_event(mct_port_t *port, mct_event_t *event)
{
  switch(event->type) {
  case MCT_EVENT_MODULE_EVENT: {
    switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT_ACK:
      pthread_mutex_lock(&mutex);
      frame_pending = FALSE;
      pthread_cond_signal(&frame_done_cond);
      pthread_mutex_unlock(&mutex);
      break;
    default:
      break;
    }
    break;
  }
  default:
    break;
  }
  return TRUE;
}

static boolean pproc_test_create_stream_info(unsigned int identity,
  mct_stream_info_t *stream_info, mct_list_t *img_buf_list,
  cpp_testcase_input_t *test_case)
{
  cam_pp_offline_src_config_t *offline_src_cfg;
  cam_pp_feature_config_t     *pp_feature_config;

  stream_info->identity = identity;
  stream_info->fmt = CAM_FORMAT_YUV_420_NV12;
  stream_info->stream_type = CAM_STREAM_TYPE_OFFLINE_PROC;
  stream_info->streaming_mode = CAM_STREAMING_MODE_BURST;
  stream_info->num_burst = 1;
  stream_info->img_buffer_list = img_buf_list;

  stream_info->buf_planes.plane_info.num_planes = 2;
  stream_info->reprocess_config.pp_type = CAM_OFFLINE_REPROCESS_TYPE;
  if (test_case->output_width && test_case->output_height) {
    stream_info->dim.width = test_case->output_width;
    stream_info->dim.height = test_case->output_height;
    stream_info->buf_planes.plane_info.mp[0].stride = test_case->output_width;
    stream_info->buf_planes.plane_info.mp[0].scanline = test_case->output_height;
  } else {
    stream_info->dim.width = PPROC_TEST_INPUT_WIDTH;
    stream_info->dim.height = PPROC_TEST_INPUT_HEIGHT;
    stream_info->buf_planes.plane_info.mp[0].stride = PPROC_TEST_INPUT_WIDTH;
    stream_info->buf_planes.plane_info.mp[0].scanline = PPROC_TEST_INPUT_HEIGHT;

  }

  offline_src_cfg = &stream_info->reprocess_config.offline;
  offline_src_cfg->num_of_bufs = 1;
  offline_src_cfg->input_fmt = CAM_FORMAT_YUV_420_NV12;
  if (test_case->input_width && test_case->input_height) {
    offline_src_cfg->input_dim.width = test_case->input_width;
    offline_src_cfg->input_dim.height = test_case->input_height;
    offline_src_cfg->input_buf_planes.plane_info.mp[0].stride =
        test_case->input_width;
    offline_src_cfg->input_buf_planes.plane_info.mp[0].scanline =
        test_case->input_height;
  } else {
    offline_src_cfg->input_dim.width = PPROC_TEST_INPUT_WIDTH;
    offline_src_cfg->input_dim.height = PPROC_TEST_INPUT_HEIGHT;
    offline_src_cfg->input_buf_planes.plane_info.mp[0].stride =
      PPROC_TEST_INPUT_WIDTH;
    offline_src_cfg->input_buf_planes.plane_info.mp[0].scanline =
      PPROC_TEST_INPUT_HEIGHT;
  }

  pp_feature_config = &stream_info->reprocess_config.pp_feature_config;
  pp_feature_config->feature_mask = CAM_QCOM_FEATURE_CROP |
    CAM_QCOM_FEATURE_DENOISE2D | CAM_QCOM_FEATURE_SHARPNESS;
  pp_feature_config->input_crop.left = 0;
  pp_feature_config->input_crop.top = 0;
  pp_feature_config->input_crop.width = PPROC_TEST_INPUT_WIDTH;
  pp_feature_config->input_crop.height = PPROC_TEST_INPUT_HEIGHT;

  stream_info->stream = mct_stream_new(identity & 0x0000FFFF);
  stream_info->stream->streaminfo = *stream_info;
  return TRUE;
}

static boolean pproc_test_destroy_stream_info(mct_stream_info_t *stream_info)
{
  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(stream_info->stream));
  free(stream_info->stream);
  return TRUE;
}

int main(int argc, char * argv[])
{
  boolean                  rc = FALSE;
  int32_t                  main_ret = -1;
  cpp_testcase_input_t     test_case;
  int                      ionfd = 0;
  struct v4l2_frame_buffer in_frame, out_frame;
  int                      in_frame_fd = 0, out_frame_fd = 0;
  int                      read_len = 0;
  int                      in_file_fd, out_file_fd;
  char                     out_fname[256];
  mct_module_t            *pproc;
  mct_port_t              *pproc_port = NULL, *test_port = NULL;
  mct_stream_info_t        stream_info;
  unsigned int             identity;
  mct_event_t              event;
  mct_stream_map_buf_t     img_buf_input, img_buf_output, meta_buf;
  mct_list_t              *img_buf_list = NULL, *list;
  cam_stream_parm_buffer_t parm_buf;
  mct_pipeline_t          *pipeline = NULL;
  void                    *chromatix_header = NULL;
  void                    *chromatixPtr = NULL;
  void                    *chromatix_common_header = NULL;
  void                    *chromatixCommonPtr = NULL;
  mct_stream_session_metadata_info *priv_metadata;
  cam_metadata_info_t       *metadata = NULL;
  void *(*open_lib)(void);
  stats_get_data_t        stats_get;

  pthread_mutex_init(&mutex, NULL);
  if (argc > 1) {
    main_ret = parse_test_case_file(argv, &test_case);
    if (main_ret < 0)
      return main_ret;
  } else {
    CDBG_ERROR("%s:%d] Usage: cpp-test-app <test case file>\n", __func__,
      __LINE__);
    goto EXIT1;
  }

  in_file_fd = open(test_case.input_filename, O_RDWR | O_CREAT, 0777);
  if (in_file_fd < 0) {
    CDBG_ERROR("%s:%d] Cannot open input file\n", __func__, __LINE__);
    goto EXIT1;
  }

  chromatix_header = dlopen((const char *)test_case.chromatix_file, RTLD_NOW);
  if(!chromatix_header) {
    CDBG_ERROR("Error opening chromatix file %s \n",test_case.chromatix_file);
  } else {
    *(void **)&open_lib = dlsym(chromatix_header, "load_chromatix");
    if (!open_lib) {
      CDBG_ERROR("Fail to find symbol %s",dlerror());
    } else {
      chromatixPtr = open_lib();
    }
  }

  chromatix_common_header = dlopen((const char *)test_case.chromatix_common_file, RTLD_NOW);
  if(!chromatix_common_header) {
    CDBG_ERROR("Error opening chromatix file %s \n",test_case.chromatix_common_file);
  } else {
    *(void **)&open_lib = dlsym(chromatix_common_header, "load_chromatix");
    if (!open_lib) {
      CDBG_ERROR("Fail to find symbol %s",dlerror());
    } else {
      chromatixCommonPtr = open_lib();
    }
  }
  pthread_cond_init(&frame_done_cond, NULL);

  /* open ion device */
  ionfd = open("/dev/ion", O_RDONLY | O_SYNC);
  if (ionfd < 0) {
    CDBG_ERROR("%s:%d] Ion device open failed\n", __func__, __LINE__);
    goto EXIT2;
  }

  /* Create input buffer */
  memset(&in_frame, 0, sizeof(struct v4l2_frame_buffer));
  if (test_case.input_width && test_case.input_height) {
    in_frame.ion_alloc[0].len = test_case.input_width *
      test_case.input_height *1.5f;
  } else {
    in_frame.ion_alloc[0].len = PPROC_TEST_INPUT_WIDTH *
      PPROC_TEST_INPUT_HEIGHT * 1.5;
  }
  in_frame.ion_alloc[0].heap_id_mask = (0x1 << ION_IOMMU_HEAP_ID);
  in_frame.ion_alloc[0].align = PPROC_TEST_ALIGN_4K;
  in_frame.addr[0] = (unsigned long)do_mmap_ion(ionfd,
    &(in_frame.ion_alloc[0]), &(in_frame.fd_data[0]), &in_frame_fd);
  if (!in_frame.addr[0]) {
    CDBG_ERROR("%s:%d] error mapping input ion fd\n", __func__, __LINE__);
    goto EXIT3;
  }
  memset(&img_buf_input, 0, sizeof(mct_stream_map_buf_t));
  img_buf_input.buf_planes[0].buf = (void *)in_frame.addr[0];
  img_buf_input.buf_planes[0].fd = in_frame_fd;
  img_buf_input.num_planes = 2;
  img_buf_input.buf_index = 0;
  img_buf_input.buf_type = CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF;
  img_buf_input.common_fd = TRUE;
  list = mct_list_append(img_buf_list, &img_buf_input, NULL, NULL);
  if (!list) {
    CDBG_ERROR("%s:%d] error appending input buffer\n", __func__, __LINE__);
    goto EXIT4;
  }
  img_buf_list = list;

  /* Read from input file */
  read_len = read(in_file_fd, (void *)in_frame.addr[0],
    in_frame.ion_alloc[0].len);
  if (read_len != (int)in_frame.ion_alloc[0].len) {
    CDBG_ERROR("%s:%d] Copy input image failed read_len %d, file_len:%d\n",
      __func__, __LINE__, read_len, in_frame.ion_alloc[0].len);
    //goto EXIT5;
  }

  /* Create output buffer */
  memset(&out_frame, 0, sizeof(struct v4l2_frame_buffer));
  if (test_case.output_width && test_case.output_height) {
    out_frame.ion_alloc[0].len = test_case.output_width *
      test_case.output_height * 1.5;
  } else {
    out_frame.ion_alloc[0].len = PPROC_TEST_INPUT_WIDTH *
        PPROC_TEST_INPUT_HEIGHT * 1.5;
  }
  out_frame.ion_alloc[0].heap_id_mask = (0x1 << ION_IOMMU_HEAP_ID);
  out_frame.ion_alloc[0].align = PPROC_TEST_ALIGN_4K;
  out_frame.addr[0] = (unsigned long)do_mmap_ion(ionfd,
    &(out_frame.ion_alloc[0]), &(out_frame.fd_data[0]), &out_frame_fd);
  memset((void *) out_frame.addr[0], 128, out_frame.ion_alloc[0].len);
  if (!out_frame.addr[0]) {
    CDBG_ERROR("%s:%d] error mapping output ion fd\n", __func__, __LINE__);
    goto EXIT5;
  }
  memset(&img_buf_output, 0, sizeof(mct_stream_map_buf_t));
  img_buf_output.buf_planes[0].buf = (void *)out_frame.addr[0];
  img_buf_output.buf_planes[0].fd = out_frame_fd;
  img_buf_output.num_planes = 2;
  img_buf_output.buf_index = 1;
  img_buf_output.buf_type = CAM_MAPPING_BUF_TYPE_STREAM_BUF;
  img_buf_output.common_fd = TRUE;
  list = mct_list_append(img_buf_list, &img_buf_output, NULL, NULL);
  if (!list) {
    CDBG_ERROR("%s:%d] error appending output buffer\n", __func__, __LINE__);
    goto EXIT6;
  }
  img_buf_list = list;

  metadata = malloc(sizeof(cam_metadata_info_t));
  if(metadata == NULL) {
    CDBG_ERROR("Fail to allocate metadata buffer\n");
  } else {
    priv_metadata = (mct_stream_session_metadata_info *)metadata->private_metadata;
    priv_metadata->sensor_data.chromatix_ptr = chromatixPtr;
    priv_metadata->sensor_data.common_chromatix_ptr = chromatixCommonPtr;
    stats_get.aec_get.lux_idx = test_case.lux_idx;
    stats_get.aec_get.real_gain[0] = test_case.real_gain;
    memcpy(&priv_metadata->stats_aec_data.private_data, &stats_get,
      sizeof(stats_get_data_t));
    memset(&meta_buf, 0, sizeof(mct_stream_map_buf_t));
    meta_buf.buf_planes[0].buf = (void *)metadata;
    meta_buf.buf_planes[0].fd = 0;
    meta_buf.num_planes = 0;
    meta_buf.buf_index = 2;
    meta_buf.buf_type = CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF;
    meta_buf.common_fd = TRUE;
    list = mct_list_append(img_buf_list, &meta_buf, NULL, NULL);
    if (!list) {
      CDBG_ERROR("%s:%d] error appending meta buffer\n", __func__, __LINE__);
    }
    img_buf_list = list;
  }

  /* Start session on pproc */
  pproc = (mct_module_t *)pproc_module_init("pproc");
  if (!pproc) {
    CDBG_ERROR("%s:%d] error getting pproc module\n", __func__, __LINE__);
    goto EXIT7;
  }

  rc = pproc->start_session(pproc, 0x0050);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error starting session in pproc\n", __func__, __LINE__);
    goto EXIT8;
  }

  /* Create a test port and set function handles */
  test_port = mct_port_create("test_port");
  if (!test_port) {
    CDBG_ERROR("%s:%d] error creating test port", __func__, __LINE__);
    goto EXIT9;
  }
  mct_port_set_event_func(test_port, pproc_test_port_event);

  test_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  /* Create pipeline */
  pipeline = mct_pipeline_new();
  if (!pipeline) {
    CDBG_ERROR("%s:%d] error creating pipeline", __func__, __LINE__);
    goto EXIT10;
  }

  /* Create stream info */
  identity = pack_identity(0x0050, 0x0050);
  memset(&stream_info, 0, sizeof(mct_stream_info_t));
  rc = pproc_test_create_stream_info(identity, &stream_info,
    img_buf_list, &test_case);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error creating stream info\n", __func__, __LINE__);
    goto EXIT11;
  }

  /* Add stream to pipeline */
  rc = mct_object_set_parent(MCT_OBJECT_CAST(stream_info.stream),
    MCT_OBJECT_CAST(pipeline));
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error adding stream to pipeline\n", __func__, __LINE__);
    goto EXIT12;
  }

  /* Caps reserve on pproc module */
  pproc_port = pproc_port_resrv_port_on_module(pproc, &stream_info,
    MCT_PORT_SINK, test_port);
  if (!pproc_port) {
    CDBG_ERROR("%s:%d] error reserving pproc port\n", __func__, __LINE__);
    goto EXIT13;
  }

  /* Ext link on pproc port */
  rc = pproc_port->ext_link(identity, pproc_port, test_port);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error linking pproc port\n", __func__, __LINE__);
    goto EXIT14;
  }

  rc = mct_port_add_child(identity, pproc_port);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error adding identity to port\n", __func__, __LINE__);
    goto EXIT15;
  }

  rc = mct_object_set_parent(MCT_OBJECT_CAST(pproc),
    MCT_OBJECT_CAST(stream_info.stream));
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error adding pproc to stream\n", __func__, __LINE__);
    goto EXIT16;
  }

  /* Stream on event */
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMON;
  event.u.ctrl_event.control_event_data = (void *)&stream_info;
  rc = pproc_port->event_func(pproc_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error in streaming on\n", __func__, __LINE__);
    goto EXIT17;
  }

  /* Set output buffer to stream */
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_PPROC_SET_OUTPUT_BUFF;
  event.u.module_event.module_event_data = (void *)&img_buf_output;
  rc = pproc_port->event_func(pproc_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error setting output buffer to stream\n", __func__,
      __LINE__);
    goto EXIT18;
  }

  /* Trigger stream param buffer event */
  pthread_mutex_lock(&mutex);
  frame_pending = TRUE;
  memset(&parm_buf, 0, sizeof(cam_stream_parm_buffer_t));
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
  event.u.ctrl_event.control_event_data = (void *)&parm_buf;
  /* Set reprocess offline parameters */
  parm_buf.type = CAM_STREAM_PARAM_TYPE_DO_REPROCESS;
  parm_buf.reprocess.frame_idx = 0; /* Frame id */
  parm_buf.reprocess.buf_index = 0;
    if (metadata) {
      parm_buf.reprocess.meta_present = 1;
      parm_buf.reprocess.meta_buf_index = 2;
    }
    if (test_case.process_window_height && test_case.process_window_width) {
      parm_buf.reprocess.frame_pp_config.crop.crop_enabled = 1;
      parm_buf.reprocess.frame_pp_config.crop.input_crop.left =
        test_case.process_window_first_pixel;
      parm_buf.reprocess.frame_pp_config.crop.input_crop.top =
        test_case.process_window_first_line;
      parm_buf.reprocess.frame_pp_config.crop.input_crop.width =
        test_case.process_window_width;
      parm_buf.reprocess.frame_pp_config.crop.input_crop.height =
        test_case.process_window_height;
    }
  rc = pproc->process_event(pproc, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending stream param buff event\n", __func__,
      __LINE__);
    frame_pending = FALSE;
    pthread_mutex_unlock(&mutex);
    goto EXIT18;
  }

  /* Wait for condition signal */
  while (frame_pending == TRUE) {
    pthread_cond_wait(&frame_done_cond, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  /* Copy to output file */
  sprintf(out_fname, "%s_%d.yuv", test_case.output_filename, 0);
  out_file_fd = open(out_fname, O_RDWR | O_CREAT, 0777);
  if (out_file_fd < 0) {
    CDBG_ERROR("%s:%d] Cannot open file\n", __func__, __LINE__);
    goto EXIT18;
  }
  write(out_file_fd, (const void *)out_frame.addr[0],
    out_frame.ion_alloc[0].len);

EXIT:
  close(out_file_fd);
EXIT18:
  /* Stream off event */
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
  event.u.ctrl_event.control_event_data = (void *)&stream_info;
  pproc_port->event_func(pproc_port, &event);
EXIT17:
  mct_list_free_all_on_data(MCT_OBJECT_CHILDREN(stream_info.stream),
    mct_stream_remvove_stream_from_module, stream_info.stream);
EXIT16:
  mct_port_remove_child(identity, pproc_port);
EXIT15:
  /* Unlink on pproc port */
  pproc_port->un_link(identity, pproc_port, test_port);
EXIT14:
  /* Caps unreserve on pproc port */
  pproc_port->check_caps_unreserve(pproc_port, identity);
EXIT13:
  MCT_PIPELINE_CHILDREN(pipeline) =
    mct_list_remove(MCT_PIPELINE_CHILDREN(pipeline), stream_info.stream);
  (MCT_PIPELINE_NUM_CHILDREN(pipeline))--;
EXIT12:
  pproc_test_destroy_stream_info(&stream_info);
EXIT11:
  mct_pipeline_destroy(pipeline);
EXIT10:
  mct_port_destroy(test_port);
EXIT9:
  /* Stop session on pproc */
  pproc->stop_session(pproc, 0x0050);
EXIT8:
  pproc_module_deinit(pproc);
EXIT7:
  if (metadata) {
    img_buf_list = mct_list_remove(img_buf_list, &metadata);
    free(metadata);
  }
  img_buf_list = mct_list_remove(img_buf_list, &img_buf_output);
EXIT6:
  do_munmap_ion(ionfd, &(out_frame.fd_data[0]), (void *)out_frame.addr[0],
    out_frame.ion_alloc[0].len);
EXIT5:
  img_buf_list = mct_list_remove(img_buf_list, &img_buf_input);
EXIT4:
  do_munmap_ion(ionfd, &(in_frame.fd_data[0]), (void *)in_frame.addr[0],
    in_frame.ion_alloc[0].len);
EXIT3:
  close(ionfd);
EXIT2:
  if (chromatix_header) {
    dlclose(chromatix_header);
  }
  if (chromatix_common_header) {
    dlclose(chromatix_common_header);
  }
  close(in_file_fd);
EXIT1:
  pthread_mutex_destroy(&mutex);
  return main_ret;
}
