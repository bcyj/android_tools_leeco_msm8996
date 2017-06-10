/*==========================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <omx_jpeg_ext.h>

char * omx_jpeg_ext_name[] = {
    "start",
    "exif",
    "thumbnail",
    "thumbnail_quality",
    "buffer_offset",
    "acbcr_offset",
    "user_preferences",
    "mobicat",
    "region",
    "image_type",
    "end"
};
/*-------------------------------------------------------------------------------------------------
* Function : omx_jpeg_queue_init
*
* Description: Initialize the jpeg queue
---------------------------------------------------------------------------------------------------*/

void omx_jpeg_queue_init(omx_jpeg_queue * queue){
    queue->back = 0;
    queue->front = -1;
    queue->size = 0;
}
/*-------------------------------------------------------------------------------------------------
* Function : omx_jpeg_message_queue_init
*
* Description:
---------------------------------------------------------------------------------------------------*/
void omx_jpeg_message_queue_init(omx_jpeg_message_queue * queue){
    omx_jpeg_queue_init(&queue->command);
    omx_jpeg_queue_init(&queue->etb);
    omx_jpeg_queue_init(&queue->ftb);
    omx_jpeg_queue_init(&queue->abort);
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->messageCount = 0;
    queue->initialized = 1;
}

//error code for add
//protected by caller
/*-------------------------------------------------------------------------------------------------
* Function : omx_jpeg_queue_insert
*
* Description:Insert into queue
---------------------------------------------------------------------------------------------------*/
int omx_jpeg_queue_insert(omx_jpeg_queue* queue, omx_jpeg_queue_item * item){
    if(queue->size==OMX_JPEG_QUEUE_CAPACITY){
        return -1;
    }

    memcpy(&queue->container[queue->back], item,
            sizeof(omx_jpeg_queue_item));
    queue->back = (queue->back +1)%OMX_JPEG_QUEUE_CAPACITY;
    queue->size++;
    //OMX_DBG_INFO("%s: Queue insert successful\n",__func__);
    return 0;
}
/*-------------------------------------------------------------------------------------------------
* Function : omx_jpeg_queue_remove
*
* Description: Remove from queue
---------------------------------------------------------------------------------------------------*/
int omx_jpeg_queue_remove(omx_jpeg_queue * queue, omx_jpeg_queue_item* item){
    if(queue->size == 0){
        return -1;
    }
    queue->front = (queue->front+1)%OMX_JPEG_QUEUE_CAPACITY;
    memcpy(item, &queue->container[queue->front],
            sizeof(omx_jpeg_queue_item));

    queue->size--;
    return 0;
}
/*-------------------------------------------------------------------------------------------------
* Function : omx_jpeg_queue_flush
*
* Description: Remove from queue
---------------------------------------------------------------------------------------------------*/
int omx_jpeg_queue_flush(omx_jpeg_queue * queue){

   int i=0;
  omx_jpeg_queue_item item;

   if(queue->size == 0)
        return 0;

  for(i=0;i<queue->size;i++){
      omx_jpeg_queue_remove(&queue,&item);
   }

    return 0;

}
