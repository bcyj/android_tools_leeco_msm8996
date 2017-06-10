/* mct_stream.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_STREAM_H__
#define __MCT_STREAM_H__

#include "mct_object.h"
#include "mct_module.h"
#include "cam_intf.h"

# if 0
#define MCT_ST_STATE(stream)         (MCT_STREAM_CAST(stream)->current_state)
#define MCT_ST_STATE_NEXT(stream)    (MCT_STREAM_CAST(stream)->next_state)
#define MCT_ST_STATE_PENDING(stream) (MCT_STREAM_CAST(stream)->pending_state)
#define MCT_ST_STATE_TARGET(stream)  (MCT_STREAM_CAST(stream)->target_state)

/**
 * MctSTStateChange:
 *
 * */
typedef enum {
  MCT_ST_STATE_CHANGE_STOP_TO_RESET    = (MCT_ST_STATE_STOP<<3)  |
    MCT_ST_STATE_RESET,

  MCT_ST_STATE_CHANGE_RESET_TO_RUNNING = (MCT_ST_STATE_RESET<<3) |
    MCT_ST_STATE_RUNNING,

  MCT_ST_STATE_CHANGE_RUNNING_TO_STOP = (MCT_ST_STATE_RUNNING<<3)|
    MCT_ST_STATE_STOP
} MctSTStateChange;

#define MCT_ST_STATE_RETURN(stream)  (MCT_STREAM_CAST(mod)->last_return)

#define MCT_STREAM(val)                 ((val) < 0 ? -1 : ((val) > 0 ? 1 : 0))
#define MCT_ST_STATE_GET_NEXT(cur,pending) \
  ((MctSTState)((cur) + MCT_STREAM ((int)(pending) - (int)(cur))))
#define MCT_ST_STATE_TRANSITION(cur,next)  ((MctSTStateChange)(((cur)<<3)|(next)))

#define MCT_ST_STATE_TRANSITION_CURRENT(trans) ((MctSTState)((trans)>>3))
#define MCT_ST_STATE_TRANSITION_NEXT(trans)    ((MctSTState)((trans)&0x7))

typedef boolean (* MctSTSetStateFunction)    (MctStream *stream,
                                              MctSTState state);

typedef boolean (* MctSTChangeStateFunction) (MctStream *stream,
                                              MctSTState state);
#endif /* if 0 */

typedef enum {
  MCT_STREAM_STATUS_OK,
  MCT_STRAEM_STATUS_NO_ISP_RESOURCE,
} mct_stream_status_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct _mct_stream_info_t {
  unsigned int identity;

  /* stream type*/
  cam_stream_type_t stream_type;

  /* image format */ /* for sensor, */
  cam_format_t fmt;

  /* image dimension */ /* for sensor, */
  cam_dimension_t dim;

  /* buffer plane information, will be calc based on stream_type, fmt,
     dim, and padding_info(from stream config). Info including:
     offset_x, offset_y, stride, scanline, plane offset */
  cam_stream_buf_plane_info_t buf_planes;


  /* streaming type */
  cam_streaming_mode_t streaming_mode;

  /* burst number of snapshot */
  int num_burst;

  /*DS mapped buffer information*/
  mct_list_t *img_buffer_list;

  /* Stream buffer parameters */
  cam_stream_parm_buffer_t parm_buf;

  cam_pp_feature_config_t pp_config;
  cam_stream_reproc_config_t reprocess_config;
  int num_bufs;

  mct_stream_t *stream;
  /*TODO: Add more fileds based on requirements*/
  mct_stream_status_t status;
  /* Image Stabilization type */
   cam_is_type_t is_type;
} mct_stream_info_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef enum {
  MCT_ST_STATE_NONE             = 1,
  MCT_ST_STATE_LINKED           = 2,
  MCT_ST_STATE_STOP             = 3,
  MCT_ST_STATE_RUNNING          = 4,
  MCT_ST_STATE_PENDING_RESTART  = 5,
  MCT_ST_STATE_MAX              = 6
} mct_stream_state_t;

/** mct_stream_buf_plane_t
 *    @buf:   vitual address from mmap
 *    @size:  buffer length
 *    @fd:    file descriptor from domain socket
 *
 *  Buffer plane Mapped from HAL via domain socket
 **/
typedef struct {
  void *buf;
  size_t size;
  int  fd;
  uint32_t offset;
  int32_t stride;
  int32_t scanline;
} mct_stream_buf_plane_t;

/** MctStreamMapBuf_t
 *    @buf:   vitual address from mmap
 *    @size:  buffer length
 *    @fd:    file descriptor from domain socket
 *    @index: buffer index
 *
 *  Buffers Mapped from HAL via domain socket
 **/
typedef struct {
  uint32_t buf_index;
  uint32_t num_planes;
  size_t buf_size;
  cam_mapping_buf_type buf_type;
  boolean common_fd;
  mct_stream_buf_plane_t buf_planes[VIDEO_MAX_PLANES];
} mct_stream_map_buf_t;

/** MctStreamBufs_t:
 *    @config:
 *    @imgBuf:
 *    @aecBuf:
 *    @awbBuf:
 *    @afBuf:
 *    @histBuf:
 *
 **/
typedef struct {
  void                 *stream_info;
  size_t               stream_size;
  int                   stream_fd;
  mct_list_t           *img_buf;
} mct_stream_bufs_t;

typedef struct {
  mct_bus_msg_sensor_metadata_t sensor_data;
  mct_bus_msg_stats_aec_metadata_t stats_aec_data;
  mct_bus_msg_isp_stats_awb_metadata_t isp_stats_awb_data;
  mct_bus_msg_isp_gamma_t isp_gamma_data;
  unsigned int                     frame_idx;
  struct timeval                   timestamp;
} mct_stream_session_metadata_info;

typedef struct {
  uint32_t frame_number;
  uint32_t frame_index;
} mct_stream_frame_num_idx_map_t;

typedef struct {
  int32_t                          current_buf_idx;
  int                              buf_mgr_dev_fd;
  int32_t                          get_buf_err;
  int32_t                          current_frame_idx;
  mct_list_t                       *frame_num_id_map;
  mct_stream_session_metadata_info session_meta;
} mct_stream_metadata_t;

/* add modules to and remove modules from the stream */
typedef boolean (* mct_stream_add_module_func)
  (mct_stream_t *stream, mct_module_t *module);

typedef boolean (* mct_stream_remove_module_func)
  (mct_stream_t *stream, mct_module_t *module);

typedef boolean (* mct_stream_insert_module_func)
  (/* FIXME */);

typedef boolean (* mct_stream_configure_func)
  (/* FIXME */);

typedef boolean (* mct_stream_send_event_func)
  (mct_stream_t *stream, mct_event_t *event);

typedef boolean (* mct_stream_link_modules_func) (mct_stream_t *stream);

typedef void    (* mct_stream_unlink_modules_func)
  (mct_stream_t *stream, mct_module_t *mod1, mct_module_t *mod2);

typedef boolean (* mct_stream_map_buf_func)
  (void *msg, mct_stream_t *stream);

typedef boolean (* mct_stream_unmap_buf_func)
  (void *msg, mct_stream_t *stream);

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
struct _mct_stream {
  /* Stream's parent should be pipeline,
   * Stream's children should be modules which is arranged from
   * source module to sink module -
   * Note: Only Stream's children list can grow into a TREE form */
  mct_object_t     object;

  uint32_t streamid;
  mct_stream_info_t  streaminfo;

  /*
   * Everything is in one thread context, there is no need
   * to stuck in State. We shall difinitely NOT consider
   * asynchronous State transition, otherwise things could
   * end up with much complexity.
   *
   * At stream layer, 3 States: LINKED STOP and RUNNING should be
   * sufficient.
   */
  mct_stream_state_t   state;
  mct_stream_bufs_t    buffers;
  void                 *stream_private;

  mct_stream_metadata_t metadata_stream; /*Could be added to stream private??*/
  mct_list_t            *frame_num_idx_list;

  /* virtual functions */
  mct_stream_add_module_func     add_module;
  mct_stream_remove_module_func  remove_module;
  mct_stream_insert_module_func  insert_module;
  mct_stream_configure_func      configure;
  mct_stream_send_event_func     send_event;
  mct_stream_link_modules_func   link;
  mct_stream_unlink_modules_func unlink;
  mct_stream_map_buf_func        map_buf;
  mct_stream_unmap_buf_func      unmap_buf;
};

#define MCT_STREAM_CAST(stream)        ((mct_stream_t *)(stream))
#define MCT_STREAM_PARENT(stream)      (MCT_OBJECT_PARENT(stream))
#define MCT_STREAM_NUM_CHILDREN(stream)(MCT_OBJECT_NUM_CHILDREN(stream))
#define MCT_STREAM_CHILDREN(stream)    (MCT_OBJECT_CHILDREN(stream))
#define MCT_STREAM_LOCK(stream)        (MCT_OBJECT_LOCK(stream))
#define MCT_STREAM_UNLOCK(stream)      (MCT_OBJECT_UNLOCK(stream))
#define MCT_STREAM_LINK(stream)        (MCT_STREAM_CAST(stream)->link)
#define MCT_STREAM_STREAMINFO(stream)  \
  (MCT_STREAM_CAST(stream)->buffers.stream_info)

#define mct_stream_set_add_module_func(stream,f)  \
  MCT_STREAM_CAST(stream)->add_module = f

#define mct_stream_set_remove_module_func(stream,f)  \
  MCT_STREAM_CAST(stream)->remove_module = f

#define mct_stream_set_insert_module_func(stream,f)  \
  MCT_STREAM_CAST(stream)->insert_module = f

#define mct_stream_set_configure_func(stream,f)  \
  MCT_STREAM_CAST(stream)->configure = f

#define mct_stream_set_send_event_func(stream,f)  \
  MCT_STREAM_CAST(stream)->send_event = f

#define mct_stream_set_link_modules_func(stream,f)  \
  MCT_STREAM_CAST(stream)->link_modules = f

#define mct_stream_set_unlink_modules_func(stream,f)  \
  MCT_STREAM_CAST(stream)->unlink_modules = f

#define mct_stream_set_map_buf_func(stream,f)  \
  MCT_STREAM_CAST(stream)->map_buf = f

#define mct_stream_set_unmap_buf_func(stream,f)  \
  MCT_STREAM_CAST(stream)->unmap_buf = f

mct_stream_t *mct_stream_new(uint32_t stream_id);
void          mct_stream_destroy(mct_stream_t *stream);

boolean mct_stream_metadata_bus_msg(mct_stream_t *stream,
  mct_bus_msg_t *bus_msg);

boolean mct_stream_link_modules(mct_stream_t *stream,
  mct_module_t *mod1, mct_module_t *mod2, ...);
boolean mct_stream_remvove_stream_from_module(void *data,
  void *user_data);
void mct_stream_operate_unlink(void *d1, void *d2,
  const void *user_data);

#endif /* __MCT_STREAM_H__ */
