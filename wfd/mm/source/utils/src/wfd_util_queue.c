/* =======================================================================
                              wfd_util_queue.c
DESCRIPTION
  This header defines the wfd source item class.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
   $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/src/wfd_util_queue.c

========================================================================== */

/*========================================================================
  Include Files
 ==========================================================================*/
#include "wfd_util_debug.h"
#include "wfd_util_queue.h"
#include "MMDebugMsg.h"
#include <stdlib.h>

typedef struct venc_queue_type
{
  int head;
  int size;
  unsigned char* data;
  int max_queue_size;
  int max_data_size;
} venc_queue_type;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_queue_create(void** handle, int max_queue_size, int max_data_size)
{
  int result = 0;

  if (handle)
  {
    venc_queue_type* queue = (venc_queue_type*) malloc(sizeof(venc_queue_type));
    *handle = (void*) queue;

    if(queue != NULL)
    {
        queue->max_queue_size = max_queue_size;
        queue->max_data_size = max_data_size;
        queue->data = NULL;
        queue->head = 0;
        queue->size = 0;

        if (queue->max_queue_size > 0)
        {
            if (queue->max_data_size > 0)
            {
                queue->data = malloc((size_t)(queue->max_queue_size * queue->max_data_size));
                if (!queue->data)
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"error allocating data array");
                    free((void*) queue);
                    queue = NULL;
                    result = 1;
                }
            }
        }
    }
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_queue_destroy(void* handle)
{
  int result = 0;

  if (handle)
  {
    venc_queue_type* queue = (venc_queue_type*)handle;
    if (queue->data != NULL)
    {
      free(queue->data);
    }
    free(handle);
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"invalid handle");
    result = 1;
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_queue_pop(void* handle, void* data, int data_size)
{
  int result = 0;

  if (handle)
  {
    venc_queue_type* queue = (venc_queue_type*) handle;
    result = venc_queue_peek(handle, data, data_size);

    if (result == 0)
    {
      --queue->size;
      queue->head = (queue->head + 1) % queue->max_queue_size;
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"invalid handle");
    result = 1;
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_queue_push(void* handle, void* data, int data_size)
{
  int result = 0;

  if (handle)
  {
    venc_queue_type* queue = (venc_queue_type*) handle;

    // see if queue is full
    if (queue->size < queue->max_queue_size)
    {
      // see if data size is okay
      if (data_size >= 0 && data_size <= queue->max_data_size)
      {
        // make sure we have no null data
        if (data != NULL)
        {
          int index = (queue->head + queue->size) % queue->max_queue_size;
          int byte_offset = index * queue->max_data_size;
          memcpy(&queue->data[byte_offset], data, (size_t)data_size);
          ++queue->size;
        }
        else
        {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Data is null");
          result = 1;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Data size is wrong");
        result = 1;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Q is full");
      result = 1;
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"invalid handle");
    result = 1;
  }

  return result;
}

int venc_queue_peek(void* handle, void* data, int data_size)
{
  int result = 0;

  if (handle)
  {
    venc_queue_type* queue = (venc_queue_type*) handle;
    // see if data size is okay
    if (data_size >= 0 && data_size <= queue->max_data_size)
    {
      // make sure we have no null data
      if (data != NULL)
      {
        // make sure we have something on the queue
        if (venc_queue_size(handle) > 0)
        {
          int byte_offset = queue->head * queue->max_data_size;
          memcpy(data, &queue->data[byte_offset], (size_t)data_size);
        }
        else
        {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_MEDIUM,"queue is empty");
          result = 1;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Data is null");
        result = 1;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Data size is wrong");
      result = 1;
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"invalid handle");
    result = 1;
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_queue_size(void* handle)
{
  int size = 0;

  if (handle)
  {
    venc_queue_type* queue = (venc_queue_type*) handle;
    size = queue->size;
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"invalid handle");
  }

  return size;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_queue_full(void* handle)
{
  venc_queue_type* queue = (venc_queue_type*) handle;
  return (queue && queue->size == queue->max_queue_size) ? 1 : 0;
}
