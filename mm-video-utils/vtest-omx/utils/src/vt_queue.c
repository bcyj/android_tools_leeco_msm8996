/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential

Copyright (c) 2010 The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of The Linux Foundation nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

/*========================================================================
  Include Files
 ==========================================================================*/
#include "vt_debug.h"
#include "vt_queue.h"
#include <stdlib.h>

typedef struct vt_queue_type
{
   int head;
   int size;
   unsigned char* data;
   int max_queue_size;
   int max_data_size;
} vt_queue_type;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_queue_create(void** handle, int max_queue_size, int max_data_size)
{
   int result = 0;

   if (handle)
   {
      vt_queue_type* queue = (vt_queue_type*) malloc(sizeof(vt_queue_type));
      if (!queue)
          return 1;

      *handle = (void*) queue;

      queue->max_queue_size = max_queue_size;
      queue->max_data_size = max_data_size;
      queue->data = NULL;
      queue->head = 0;
      queue->size = 0;

      if (queue->max_queue_size > 0)
      {
         if (queue->max_data_size > 0)
         {
            queue->data = malloc(queue->max_queue_size * queue->max_data_size);
            if (!queue->data)
            {
               VTEST_MSG_ERROR("error allocating data array");
               free((void*) queue);
               result = 1;
            }
         }
      }
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_queue_destroy(void* handle)
{
   int result = 0;

   if (handle)
   {
      free(handle);
   }
   else
   {
      VTEST_MSG_ERROR("invalid handle");
      result = 1;
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_queue_pop(void* handle, void* data, int data_size)
{
   int result = 0;

   if (handle)
   {
      vt_queue_type* queue = (vt_queue_type*) handle;
      result = vt_queue_peek(handle, data, data_size);

      if (result == 0)
      {
         --queue->size;
         queue->head = (queue->head + 1) % queue->max_queue_size;
      }
   }
   else
   {
      VTEST_MSG_ERROR("invalid handle");
      result = 1;
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_queue_push(void* handle, void* data, int data_size)
{
   int result = 0;

   if (handle)
   {
      vt_queue_type* queue = (vt_queue_type*) handle;

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
               memcpy(&queue->data[byte_offset], data, data_size);
               ++queue->size;
            }
            else
            {
               VTEST_MSG_ERROR("Data is null");
               result = 1;
            }
         }
         else
         {
            VTEST_MSG_ERROR("Data size is wrong");
            result = 1;
         }
      }
      else
      {
         VTEST_MSG_ERROR("Q is full");
         result = 1;
      }
   }
   else
   {
      VTEST_MSG_ERROR("invalid handle");
      result = 1;
   }

   return result;
}

int vt_queue_peek(void* handle, void* data, int data_size)
{
   int result = 0;

   if (handle)
   {
      vt_queue_type* queue = (vt_queue_type*) handle;
      // see if data size is okay
      if (data_size >= 0 && data_size <= queue->max_data_size)
      {
         // make sure we have no null data
         if (data != NULL)
         {
            // make sure we have something on the queue
            if (vt_queue_size(handle) > 0)
            {
               int byte_offset = queue->head * queue->max_data_size;
               memcpy(data, &queue->data[byte_offset], data_size);
            }
            else
            {
               VTEST_MSG_ERROR("queue is empty");
               result = 1;
            }
         }
         else
         {
            VTEST_MSG_ERROR("Data is null");
            result = 1;
         }
      }
      else
      {
         VTEST_MSG_ERROR("Data size is wrong");
         result = 1;
      }
   }
   else
   {
      VTEST_MSG_ERROR("invalid handle");
      result = 1;
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_queue_size(void* handle)
{
   int size = 0;

   if (handle)
   {
      vt_queue_type* queue = (vt_queue_type*) handle;
      size = queue->size;
   }
   else
   {
      VTEST_MSG_ERROR("invalid handle");
   }

   return size;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_queue_full(void* handle)
{
   vt_queue_type* queue = (vt_queue_type*) handle;
   return (queue && queue->size == queue->max_queue_size) ? 1 : 0;
}
