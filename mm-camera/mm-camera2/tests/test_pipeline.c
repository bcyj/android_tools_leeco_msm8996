/* test_pipeline.c
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mtype.h"
#include "mct_controller.h"
#include "mct_pipeline.h"
#include "mct_module.h"
#include "mct_port.h"
#include "cam_intf.h"
#include "camera_dbg.h"

#if 1
#undef CDBG
#define CDBG ALOGE
#endif

#define NUM_MODULES 3

char module_names[3][10] = {"sensor", "iface", "isp"};

mct_module_t *module_init(const char *name)
{
  mct_module_t *module;
  mct_port_t *srcport, *sinkport;
  mct_port_caps_t port_caps;
  
  CDBG("%s: Enter\n", __func__);
  module = mct_module_create(name);
  if (!module) {
    CDBG_ERROR("%s: module create failed\n", __func__);
    goto module_create_failed;
  }
  srcport = mct_port_create("srcport");
  if (!srcport) {
    CDBG_ERROR("%s: SRC Port create failed\n", __func__);
    goto srcport_create_failed;
  }
  MCT_PORT_DIRECTION(srcport) = MCT_PORT_SRC;
  memset(&port_caps, 0, sizeof(mct_port_caps_t));
  port_caps.port_caps_type = MCT_PORT_CAPS_OPAQUE;
  srcport->set_caps(srcport, &port_caps);

  sinkport = mct_port_create("sinkport");
  if (!sinkport) {
    CDBG_ERROR("%s: SINK Port create failed\n", __func__);
    goto sinkport_create_failed;
  }
  MCT_PORT_DIRECTION(sinkport) = MCT_PORT_SINK;
  memset(&port_caps, 0, sizeof(mct_port_caps_t));
  port_caps.port_caps_type = MCT_PORT_CAPS_OPAQUE;
  srcport->set_caps(srcport, &port_caps);

  if (!module->add_port(module, srcport)) {
    CDBG_ERROR("%s: SRC add port failed\n", __func__);
    goto add_srcport_failed;
  }
  if (!module->add_port(module, sinkport)){
    CDBG_ERROR("%s: SINK add port failed\n", __func__);
    goto add_sinkport_failed;
  }

  CDBG("%s: Exit\n", __func__);
  return module;

  module->remove_port(module, sinkport);
add_sinkport_failed:
  module->remove_port(module, srcport);
add_srcport_failed:
  mct_port_destroy(sinkport);
sinkport_create_failed:
  mct_port_destroy(srcport);
srcport_create_failed:
  mct_module_destroy(module);
module_create_failed:
  return NULL;
}

static boolean module_deinit(void *data, void *user_data)
{
  CDBG("%s: Enter\n", __func__);
  mct_module_t *module = (mct_module_t *)data;
  mct_port_t *port;
  while (MCT_MODULE_CHILDREN(module)) {
    port = (mct_port_t *)MCT_MODULE_CHILDREN(module)->data;
    module->remove_port(module, port);
    mct_port_destroy(port);
  }
  mct_module_destroy(module);
  CDBG("%s: Exit\n", __func__);
  return TRUE;
}

void deallocate_ion_memory(int ion_fd, struct ion_fd_data *ion_info_fd,
  unsigned long addr, size_t size)
{
    struct ion_handle_data handle_data;

    munmap((void *)addr, size);
    memset(&handle_data, 0, sizeof(handle_data));
    handle_data.handle = ion_info_fd->handle;
    ioctl(ion_fd, ION_IOC_FREE, &handle_data);
    close(ion_info_fd->fd);
}

void * allocate_ion_memory(int ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, unsigned int heap_id,
  unsigned int cache_prop, size_t size, size_t align)
{
  int rc;
  void *ret;
  struct ion_handle_data handle_data;

  memset(alloc, 0, sizeof(alloc));
  alloc->len = size;
  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
  alloc->align = align;
  alloc->flags = cache_prop;
  alloc->heap_mask = ION_HEAP(heap_id);
  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    CDBG_ERROR("%s: ION allocation failed: %s\n", __func__, strerror(errno));
    goto ION_ALLOC_FAILED;
  }

  memset(ion_info_fd, 0, sizeof(struct ion_fd_data));
  ion_info_fd->handle = alloc->handle;
  rc = ioctl(ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    CDBG_ERROR("%s: ION map failed %s\n", __func__, strerror(errno));
    goto ION_MAP_FAILED;
  }

  ret = mmap(NULL, alloc->len, PROT_READ  | PROT_WRITE,
         MAP_SHARED, ion_info_fd->fd, 0);
  if (ret == MAP_FAILED) {
    CDBG_ERROR("%s: mmap failed %s\n", __func__, strerror(errno));
    goto ION_MAP_FAILED;    
  }
   
  return ret;

ION_MAP_FAILED:
    memset(&handle_data, 0, sizeof(handle_data));
    handle_data.handle = ion_info_fd->handle;
    ioctl(ion_fd, ION_IOC_FREE, &handle_data);
ION_ALLOC_FAILED:
    return NULL;
}

void populate_ds_msg(mct_serv_msg_t *msg, cam_mapping_type op_type,
  int session, int stream, cam_mapping_buf_type buf_type, int size,
  int index, int fd)
{
  memset(msg, 0, sizeof(mct_serv_msg_t));
  msg->msg_type = SERV_MSG_DS;
  msg->u.ds_msg.operation = op_type;
  msg->u.ds_msg.buf_type  = (unsigned int)buf_type;
  msg->u.ds_msg.stream    = stream;
  msg->u.ds_msg.index     = index;
  msg->u.ds_msg.session   = session;
  if (op_type == CAM_MAPPING_TYPE_FD_MAPPING) {
    msg->u.ds_msg.fd        = fd;
    msg->u.ds_msg.size      = size;
  }
}

int main(int argc, char * argv[])
{
  mct_list_t *list_modules = NULL;
  mct_pipeline_t *pipeline;
  mct_module_t *temp;
  mct_serv_msg_t cmd_msg;
  struct msm_v4l2_event_data *event_data;
  mct_event_t test_event;
  char mod_name[10];
  int i;
  int session = 0; /*default session number for this test app*/
  int stream = 0;

  /*MEMORY VARIABLES*/
  int ion_fd;

  struct ion_allocation_data stream_ion_alloc;
  struct ion_fd_data stream_ion_info_fd;
  unsigned long stream_addr;

  struct ion_allocation_data query_ion_alloc;
  struct ion_fd_data query_ion_info_fd;
  unsigned long query_addr;
  /*END OF MEMORY VARIABLES*/

  cam_stream_info_t *stream_info;

  ion_fd = open("/dev/ion", O_RDONLY);
  if (ion_fd <= 0) {
    CDBG_ERROR("%s: Ion dev open failed: %s\n", __func__, strerror(errno));
    goto ion_open_failed;
  }

  CDBG("%s: initializing modules list\n", __func__);
  /*Initialize list of modules and respective ports*/
  for (i = 0; i < NUM_MODULES; i++) {
    snprintf(mod_name, 10, module_names[i], i);
    temp = module_init(mod_name);
    if (temp) {
      CDBG("%s: appending %s to modules list\n", __func__,
        MCT_MODULE_NAME(temp));
      list_modules = mct_list_append(list_modules, temp, NULL, NULL);
    } else {
      CDBG_ERROR("%s: Module init failed\n", __func__);
      if (list_modules)
        mct_list_free_all(list_modules, module_deinit);
      list_modules = NULL;
      break;
    }
  }

  if (!list_modules) {
    goto module_init_failed;
  }

  CDBG("%s: Creating pipeline\n", __func__);
  /*Create new pipeline/session*/
  pipeline = mct_pipeline_new();
  if (!pipeline) {
    CDBG_ERROR("%s: Pipeline create failed\n", __func__);
    goto pipeline_error;
  }
  pipeline->modules = list_modules;
  pipeline->session = session;

  /* Add a new stream for testing; 
   * Should be done by mct controller when the MSM_CAMERA_NEW_STREAM 
   * ioctl is called.
   */
  memset(&cmd_msg, 0, sizeof(mct_serv_msg_t));
  cmd_msg.msg_type = SERV_MSG_HAL;
  cmd_msg.u.hal_msg.id = MSM_CAMERA_SET_PARM;
  event_data = (struct msm_v4l2_event_data *)cmd_msg.u.hal_msg.u.data;
  event_data->session_id = session;
  event_data->stream_id = stream;
  event_data->command   = MSM_CAMERA_PRIV_NEW_STREAM;
  if (!pipeline->process_serv_msg)
    goto add_stream_failed;
  CDBG("%s: Creating stream\n", __func__);
  if (!pipeline->process_serv_msg(&cmd_msg.u.hal_msg, pipeline)) {
    CDBG_ERROR("%s: Add stream failed", __func__);
    goto add_stream_failed;
  }

  /* Create a query buf ION buffer and map it into
   * the pipeline. This is merely for the purpose of testing the mapping 
   * code in pipeline.
   */
  CDBG("%s: Allocating query buffer\n", __func__);
  query_addr = (unsigned long) allocate_ion_memory(ion_fd, &query_ion_alloc,
                  &query_ion_info_fd, ION_IOMMU_HEAP_ID, ION_FLAG_CACHED,
                  sizeof(cam_capability_t), 4096);
  if (!query_addr) {
    CDBG_ERROR("%s: memory allocation failed\n", __func__);
    goto query_alloc_failed;
  }
  populate_ds_msg(&cmd_msg, CAM_MAPPING_TYPE_FD_MAPPING, session, stream,
    CAM_MAPPING_BUF_TYPE_CAPABILITY, query_ion_alloc.len, 0,
    query_ion_info_fd.fd);

  CDBG("%s: Mapping query buffer\n", __func__);
  if (pipeline->map_buf(&cmd_msg.u.ds_msg, pipeline) == FALSE) {
    CDBG_ERROR("%s: query map buf failed\n", __func__);
    goto query_map_failed;
  }
  
  /* Create a stream info ION buffer and map it into
   * the pipeline which in turn will map it into the correct stream
   */
  CDBG("%s: Allocating stream buffer\n", __func__);
  stream_addr = (unsigned long) allocate_ion_memory(ion_fd, &stream_ion_alloc,
                  &stream_ion_info_fd, ION_IOMMU_HEAP_ID, ION_FLAG_CACHED,
                  sizeof(cam_stream_info_t), 4096);
  if (!stream_addr) {
    /*memory allocation failed*/
    goto stream_alloc_failed;
  }

  populate_ds_msg(&cmd_msg, CAM_MAPPING_TYPE_FD_MAPPING, session, stream,
    CAM_MAPPING_BUF_TYPE_STREAM_INFO, stream_ion_alloc.len, 0,
    stream_ion_info_fd.fd);

  CDBG("%s: Mapping stream buffer\n", __func__);
  if (pipeline->map_buf(&cmd_msg.u.ds_msg, pipeline) == FALSE)
    goto stream_map_failed;


  /* First fill the stream_info with some data. Then,
   * select/add modules and link ports for this stream; 
   * Equivalent action to be taken by mct controller in 
   * when the MSM_CAMERA_SET_PARM ioctl is called. 
   */
  stream_info = (cam_stream_info_t *)stream_addr;
  stream_info->stream_type = CAM_STREAM_TYPE_PREVIEW;

  memset(&cmd_msg, 0, sizeof(mct_serv_msg_t));
  cmd_msg.msg_type = SERV_MSG_HAL;
  cmd_msg.u.hal_msg.id = MSM_CAMERA_SET_PARM;
  event_data = (struct msm_v4l2_event_data *)cmd_msg.u.hal_msg.u.data;
  event_data->session_id = session;
  event_data->stream_id = stream;
  event_data->command = MSM_CAMERA_PRIV_STREAM_INFO_SYNC;
  if (!pipeline->process_serv_msg)
    goto config_stream_failed;
  CDBG("%s: Configuring stream\n", __func__);
  if (!pipeline->process_serv_msg(&cmd_msg.u.hal_msg, pipeline)) {
    CDBG_ERROR("%s: Config stream failed", __func__);
    goto config_stream_failed;
  }
#if 0
  /* At this point the stream is set up with linked modules. 
   * Try to send some event to be propagated to every module.
   */
  mct_event_control_t control_event;
  int test_data;
  control_event.type               = MCT_EVENT_CONTROL_TEST;
  control_event.control_event_data = &test_data;

  test_event.sessionIdx   = session;
  test_event.streamIdx    = stream;
  test_event.direction    = MCT_EVENT_DOWNSTREAM;
  test_event.type         = MCT_EVENT_CONTROL_CMD;
  test_event.data         = &control_event;

  CDBG("%s: Sending event\n", __func__);
  if (!pipeline->send_event(pipeline, stream, &test_event))
    CDBG_ERROR("%s: Event failed", __func__);
#endif
finish:
  /*destroy stream link */

config_stream_failed:
  populate_ds_msg(&cmd_msg, CAM_MAPPING_TYPE_FD_UNMAPPING, session, stream,
    CAM_MAPPING_BUF_TYPE_STREAM_INFO, stream_ion_alloc.len, 0,
    stream_ion_info_fd.fd);  
  pipeline->unmap_buf(&cmd_msg.u.ds_msg, pipeline);

stream_map_failed:
  deallocate_ion_memory(ion_fd, &stream_ion_info_fd, stream_addr,
    stream_ion_alloc.len);

stream_alloc_failed:
  populate_ds_msg(&cmd_msg, CAM_MAPPING_TYPE_FD_UNMAPPING, session, stream,
    CAM_MAPPING_BUF_TYPE_CAPABILITY, query_ion_alloc.len, 0,
    query_ion_info_fd.fd);  
  pipeline->unmap_buf(&cmd_msg.u.ds_msg, pipeline);

query_map_failed:
  deallocate_ion_memory(ion_fd, &query_ion_info_fd, query_addr,
    query_ion_alloc.len);  

query_alloc_failed:
  memset(&cmd_msg, 0, sizeof(mct_serv_msg_t));
  cmd_msg.msg_type = SERV_MSG_HAL;
  cmd_msg.u.hal_msg.id = MSM_CAMERA_SET_PARM;
  event_data = (struct msm_v4l2_event_data *)cmd_msg.u.hal_msg.u.data;
  event_data->session_id = session;
  event_data->stream_id = stream; 
  event_data->command   = MSM_CAMERA_PRIV_DEL_STREAM; 
  if(pipeline->process_serv_msg)
    pipeline->process_serv_msg(&cmd_msg.u.hal_msg, pipeline);

add_stream_failed:
  mct_pipeline_destroy(pipeline);

pipeline_error:
  mct_list_free_all(list_modules, module_deinit);

module_init_failed:
  close(ion_fd);

ion_open_failed:
  return 0;
}
