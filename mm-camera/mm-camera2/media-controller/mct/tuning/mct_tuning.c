/* mct_tuning.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <dlfcn.h>

#include "mct_controller.h"
#include "modules.h"
#include "camera_dbg.h"
#include "eztune_interface.h"

/* Tuning library specific data structures */
void *(*create_tuning_server)( void *, uint32_t);
void  (*delete_tuning_server)( void *);
void  (*notify_tuning_server)( void *, uint32_t, void *);
void *tuning_control_handle = NULL;
void *tuning_preview_handle = NULL;
static void *tuning_lib = NULL;
static void *pipeline_instance = NULL;

/** mct_start_tuning_server
 *    @pipeline: mct_pipeline_t* pointer to pipeline
 *
 * Opens eztune server dll and starts the server.
 * Server runs in a separate thread, hence no need to call
 * this API from a separate thread context
 *
 **/
void mct_start_tuning_server (mct_pipeline_t *pipeline)
{
    MCT_OBJECT_LOCK(pipeline);

    /* Avoid multiple instances. If an instance is running do not
    create a new one */
    /* YUV sensor does not need tuning. Hence do not start Eztune
      server if the sensor is YUV*/
    if (tuning_lib || tuning_control_handle || (pipeline->query_data.sensor_cap.sensor_format == FORMAT_YCBCR)) {
        goto end;
    }

    tuning_lib = dlopen("libmmcamera_tuning.so", RTLD_NOW);

    if (tuning_lib == NULL) {
        CDBG_ERROR("Tuning lib open failed: %s", dlerror());
        goto end;
    }

    *(void **) &(create_tuning_server) = dlsym(tuning_lib, "eztune_create_server");
    *(void **) &(delete_tuning_server) = dlsym(tuning_lib, "eztune_delete_server");
    *(void **) &(notify_tuning_server) = dlsym(tuning_lib, "eztune_notify_server");

    if (!create_tuning_server || !delete_tuning_server) {
        CDBG_ERROR("Error obtaining symbols in tuning libs");
        goto end;
    }

    tuning_control_handle = create_tuning_server(pipeline, EZTUNE_SERVER_CONTROL);

    tuning_preview_handle = create_tuning_server(pipeline, EZTUNE_SERVER_PREVIEW);

    pipeline_instance = pipeline;

end:
    MCT_OBJECT_UNLOCK(pipeline);
    return;
}

/** mct_stop_tuning_server
 *    @pipeline: mct_pipeline_t* pointer to pipeline
 *
 * Stops eztune server and closes associated dll
 *
 **/
void mct_stop_tuning_server(mct_pipeline_t *pipeline)
{
    MCT_OBJECT_LOCK(pipeline);

    pipeline_instance = NULL;

    if (tuning_control_handle)
        delete_tuning_server(tuning_control_handle);

    if (tuning_preview_handle)
        delete_tuning_server(tuning_preview_handle);

    if (tuning_lib)
        dlclose(tuning_lib);

    tuning_control_handle = NULL;
    tuning_preview_handle = NULL;
    tuning_lib = NULL;

    MCT_OBJECT_UNLOCK(pipeline);

    return;
}

/** mct_notify_metadata_frame
 *    @param: pointer to metadata frame
 *
 **/
void mct_notify_metadata_frame(void *param)
{
    if (notify_tuning_server && pipeline_instance) {
        MCT_OBJECT_LOCK(pipeline_instance);
        if(tuning_control_handle) {
            notify_tuning_server(tuning_control_handle, EZTUNE_METADATA_NOTIFY, param);
        }
        //we are notifying metadata to image port also as 3A Live will need metadata to fill 3A packet
        //this is the simplest way of achieving the goal, else one Image instance of Process and Adaptor
        //will have to pull metadata from Control instance of Process and Adaptor
        if (tuning_preview_handle) {
            notify_tuning_server(tuning_preview_handle, EZTUNE_METADATA_NOTIFY, param);
        }
        MCT_OBJECT_UNLOCK(pipeline_instance);
    }
}

/** mct_notify_reload_tuning
 *
 **/
void mct_notify_reload_tuning(void)
{
    // Eztune copy of Chromatix headers needs to be updated in below use cases
    // 1. ZSL to non-ZSL mode transition or vice versa
    // 2. Snapshot triggered from UI app in non-ZSL mode
    // So, notify the Eztune server when preview stream starts
    if (tuning_control_handle && notify_tuning_server && pipeline_instance) {
        MCT_OBJECT_LOCK(pipeline_instance);
        notify_tuning_server(tuning_control_handle, EZTUNE_RELOAD_TUNING_NOTIFY, NULL);
        MCT_OBJECT_UNLOCK(pipeline_instance);
    }
}

/** mct_notify_snapshot_triggered
 *
  **/
void mct_notify_snapshot_triggered(void)
{
    if (tuning_control_handle && notify_tuning_server && pipeline_instance) {
        MCT_OBJECT_LOCK(pipeline_instance);
        notify_tuning_server(tuning_control_handle, EZTUNE_SNAPSHOT_PENDING_NOTIFY, NULL);
        MCT_OBJECT_UNLOCK(pipeline_instance);
    }
}

static boolean mct_tuning_validate_buff_index(void *data1, void *data2)
{
    mct_stream_map_buf_t *list_buf = (mct_stream_map_buf_t *)data1;
    uint32_t *buf_idx = (uint32_t *)data2;

    assert(list_buf && buf_idx);

    if (list_buf->buf_index == *buf_idx)
        return TRUE;
    else
        return FALSE;
}

//#define MCT_TUNING_FILEDUMP

static void mct_tuning_dump_frame(uint8_t *buffer, cam_frame_len_offset_t plane_info)
{
#ifdef MCT_TUNING_FILEDUMP
    int32_t file_fd = open("/data/test_img.yuv", O_RDWR | O_CREAT, 0777);

    int i;
    void *data;
    uint32_t written_len = 0;

    for (i = 0; i < plane_info.num_planes; i++) {
        uint32_t index = plane_info.mp[i].offset;
        if (i > 0) {
            index += plane_info.mp[i - 1].len;
        }
        int j;
        for (j = 0; j <  plane_info.mp[i].height; j++) {
            data = (void *)((uint8_t *)buffer + index);
            written_len += write(file_fd, data,  plane_info.mp[i].width);
            index +=  plane_info.mp[i].stride;
        }
    }
    close(file_fd);
#endif
}

void mct_tuning_notify_preview_frame(isp_buf_divert_t *p_buf_divert, mct_stream_t *stream)
{
    uint8_t *buffer;
    mct_list_t *img_buf_list;
    mct_stream_map_buf_t *img_buf;
    mct_stream_info_t streaminfo;

    if (!tuning_lib || !tuning_preview_handle || !pipeline_instance) {
        goto end;
    }

    MCT_OBJECT_LOCK(pipeline_instance);

    if (p_buf_divert->native_buf) {
        buffer = p_buf_divert->vaddr;
    } else {
        img_buf_list = mct_list_find_custom(stream->streaminfo.img_buffer_list,
                                            &p_buf_divert->buffer.index, mct_tuning_validate_buff_index);

        if (img_buf_list)
            img_buf = (mct_stream_map_buf_t *)img_buf_list->data;

        if (img_buf)
            buffer = (uint8_t *)img_buf->buf_planes[0].buf;
    }
    //for debug
    mct_tuning_dump_frame(buffer, stream->streaminfo.buf_planes.plane_info);

    //make copy of stream info and override img_buffer_list with the
    //actual buffer pointer. Intent is to send stream info and buffer
    //pointer in one call to tuning server
    streaminfo = stream->streaminfo;
    streaminfo.img_buffer_list = buffer;

    notify_tuning_server(tuning_preview_handle, EZTUNE_PREVIEW_NOTIFY, &streaminfo);

    MCT_OBJECT_UNLOCK(pipeline_instance);

end:
    return;
}

boolean mct_trigger_snapshot(void)
{
    boolean ret = TRUE;
    mct_pipeline_t *pipeline = (mct_pipeline_t *)pipeline_instance;
    mct_bus_t *bus = NULL;
    mct_bus_msg_t bus_msg;

    if (NULL == pipeline) {
        CDBG_ERROR("Error: Pipeline instance do not exist");
        ret = FALSE;
        goto end;
    }

    memset(&bus_msg, 0x0, sizeof(mct_bus_msg_t));
    bus = pipeline->bus;
    bus_msg.type = MCT_BUS_MSG_SEND_EZTUNE_EVT;
    bus_msg.size = 0;
    bus_msg.sessionid = bus->session_id;
    bus->post_msg_to_bus(bus, &bus_msg);

end:
    return ret;
}

void mct_tuning_notify_snapshot_frame(void *params)
{
    if (!tuning_lib || !tuning_preview_handle || !pipeline_instance) {
        CDBG_ERROR("Error: Tuning instance do not exist");
        goto end;
    }

    MCT_OBJECT_LOCK(pipeline_instance);
    notify_tuning_server(tuning_preview_handle, EZTUNE_SNAPSHOT_NOTIFY, params);
    MCT_OBJECT_UNLOCK(pipeline_instance);

end:
    return;
}

boolean mct_tunig_check_status(void)
{
    if (NULL == pipeline_instance) {
        return FALSE;
    }

    return TRUE;
}

