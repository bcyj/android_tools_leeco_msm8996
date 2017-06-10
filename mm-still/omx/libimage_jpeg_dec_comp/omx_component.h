/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __OMX_COMP_H__
#define __OMX_COMP_H__

#include "OMX_Types.h"
#include "OMX_Index.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "omx_debug.h"
#include "omx_jpeg_ext.h"
#include "mpod_private.h"

#include "stdio.h"

extern OMX_VERSIONTYPE version;

typedef struct omx_jpeg_decoder_t omx_jpeg_decoder_engine;

typedef struct omx_jpegd_input_buffer{
    //mpod_src_t mpod_source;
    OMX_BUFFERHEADERTYPE * inputHeader;
    struct omx_jpegd_comp * comp;
    uint8_t* addr;
    int fd;
    int length;
    int offset; // remember offset problem
    int etbDone;
    }omx_jpegd_input_buffer;

 typedef struct omx_jpegd_outputbuffer{
    OMX_BUFFERHEADERTYPE * outputHeader;
    uint8_t * addr;
    int fd;
    int length;
    int offset;
    int ftbDone;
}omx_jpegd_output_buffer;


typedef enum {
    OMX_JPEGD_MAIN_IMAGE=0,
    OMX_JPEGD_THUMBNAIL_IMAGE,
}omx_jpeg_decoded_image_type;


//TODO free all pointers when deinit think there should be a deinit method
typedef struct omx_jpegd_comp{

    OMX_COMPONENTTYPE * omxComponent;
    OMX_PARAM_PORTDEFINITIONTYPE* inPort;
    OMX_PARAM_PORTDEFINITIONTYPE* outPort;
    OMX_PORT_PARAM_TYPE* portParam;
    OMX_IMAGE_PARAM_PORTFORMATTYPE * inputFormatType;
    OMX_IMAGE_PARAM_PORTFORMATTYPE* outputFormatType;

    OMX_CALLBACKTYPE * callback;
    OMX_PTR callbackAppData;

    omx_jpeg_decoder_engine *decoder;

    omx_jpegd_input_buffer * inputBuffer;
    omx_jpegd_output_buffer * outputBuffer;

    omx_jpeg_message_queue * queue;
    pthread_t messageThread;

   // exif_info_obj_t exifInfo;
    omx_jpeg_thumbnail thumbnail;
    int thumbnailPresent;
    uint8_t *outBuffer[20];
    omx_jpeg_type decoder_type;

    //Not sure
    OMX_CONFIG_RECTTYPE outputCrop;

    int rotation;
 //  omx_jpeg_buffer_offset offset;

    int decoding;
    int initialized;
    int decode_success;

    int inTransition;
    OMX_STATETYPE currentState;
    OMX_STATETYPE targetState;
    pthread_mutex_t stateLock;
    pthread_cond_t stateCond;

    int bufferCount;
    omx_jpeg_user_preferences preferences;
    omx_jpeg_region region;

    pthread_mutex_t mLock;

    //make it object lock
    pthread_mutex_t lock;
    pthread_cond_t cond;

    int imageCount;//Number of main images
    int totalImageCount;//Number of thumbnail+main images

} omx_jpegd_comp;


omx_jpegd_input_buffer * inputBuffer;

omx_jpegd_output_buffer * outputBuffer;

/* Function to start decoding */
typedef int(*omx_decoder_start_t)(
    omx_jpegd_comp *comp, omx_jpegd_input_buffer *inputBuffer);

/* Function to configure output buffer */
typedef int(*omx_decoder_configure_output_t)(
    omx_jpegd_comp *comp);

/* Function to stop decoding */
typedef int(*omx_decoder_stop_t)(
    omx_jpegd_comp *comp);

/* Definition of Jpeg Encode Engine object */
struct omx_jpeg_decoder_t
{
    omx_decoder_start_t               decode;
    omx_decoder_stop_t                stop;
   // omx_decoder_configure_output_t    configure_output_buffer;
    uint8_t                           is_initialized;
};

void errorHandler();

void get_component_fns(OMX_COMPONENTTYPE *component_fns);

void jpegd_event_handler(void *p_user_data, jpeg_event_t event, void *p_arg);

int jpegd_input_req_handler(void *p_user_data,
                           jpeg_buffer_t   buffer,
                           uint32_t  start_offset,
                           uint32_t        length);

int postMessage(omx_jpeg_message_queue * mqueue, omx_jpeg_queue_type type,
        omx_jpeg_queue_item * item);

void jpegdInvokeStop(omx_jpegd_comp * comp);

void configure_mpo_decoder(omx_jpeg_decoder_engine *p_obj);
void configure_jpeg_decoder(omx_jpeg_decoder_engine *p_obj);

#endif /* __OMX_COMP_H__ */

