/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __OMX_COMP_H__
#define __OMX_COMP_H__

#include "OMX_Types.h"
#include "OMX_Index.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "omx_debug.h"

#include "jpege.h"
#include "omx_jpeg_ext.h"

#include "stdio.h"

void *jpeg_malloc(uint32_t size, const char* file_name, uint32_t line_num);
void  jpeg_free(void *ptr);
void  jpeg_show_leak(void);

#define INPUT_PORT 0
#define OUTPUT_PORT 1
#define INPUT_PORT1 2

typedef struct omx_jpeg_input_buffer {
    jpege_src_t jpege_source;
    jpege_img_data_t main_img_info;
    jpege_img_data_t tn_img_info;
    OMX_BUFFERHEADERTYPE * inputHeader;
    struct omx_jpeg_comp * comp;
    uint8_t* addr;
    int fd;
    int length;
    int offset;
    int etbDone;
}omx_jpeg_input_buffer;

typedef struct omx_jpeg_output_buffer {
    jpege_dst_t jpege_dest;
    jpeg_buffer_t jpege_dest_buffer[2];
    OMX_BUFFERHEADERTYPE * outputHeader;
    struct omx_jpeg_comp * comp;
    uint8_t * addr;
    int fd;
    int length;
    int offset;
}omx_jpeg_output_buffer;




/*TODO free all pointers when deinit think there should be a deinit method*/
typedef struct omx_jpeg_comp {

    OMX_COMPONENTTYPE * omxComponent;
    OMX_PARAM_PORTDEFINITIONTYPE* inPort;
    OMX_PARAM_PORTDEFINITIONTYPE* inPort1;
    OMX_PARAM_PORTDEFINITIONTYPE* outPort;
    OMX_PORT_PARAM_TYPE* portParam;
    OMX_IMAGE_PARAM_PORTFORMATTYPE * inputFormatType;
    OMX_IMAGE_PARAM_PORTFORMATTYPE* outputFormatType;

    OMX_CALLBACKTYPE * callback;
    OMX_PTR callbackAppData;

    jpege_cfg_t jpege_config;
    jpege_obj_t jpeg_encoder;

    omx_jpeg_input_buffer * inputBuffer;
    omx_jpeg_input_buffer * inputBuffer1;
    omx_jpeg_output_buffer * outputBuffer;

    omx_jpeg_message_queue * queue;
    pthread_t messageThread;

    exif_info_obj_t exifInfo;
    omx_jpeg_thumbnail thumbnail;
    int thumbnailPresent;
    int num_inputs_freed;
    OMX_CONFIG_RECTTYPE inputCrop;
    OMX_CONFIG_RECTTYPE outputCrop;
    int rotation;
    omx_jpeg_buffer_offset offset;
    omx_jpeg_buffer_offset aoffset;
    omx_jpeg_mobicat mobiData;
    int encoding;
    int mobicatenable;
    int initialized;
    int isJpegEngineActive;
    int inTransition;
    OMX_STATETYPE currentState;
    OMX_STATETYPE targetState;
    pthread_mutex_t stateLock;
    pthread_cond_t stateCond;

    int bufferCount;
    pthread_mutex_t mLock;

    /*make it object lock*/
    pthread_mutex_t lock;
    pthread_cond_t cond;

    pthread_mutex_t abort_mutex;
    omx_jpeg_thumbnail_quality thumbnailQuality;
    omx_jpeg_user_preferences preferences;

    OMX_IMAGE_PARAM_QFACTORTYPE mainImageQuality;
    int arr[100];
} omx_jpeg_comp;

void get_component_fns(OMX_COMPONENTTYPE *component_fns);


#endif /* __OMX_COMP_H__ */
