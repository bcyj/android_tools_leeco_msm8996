/***************************************************************************************************
    @file
    util_memory.c

    @brief
    Implements functions supported in util_memory.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "util_memory.h"
#include "util_log.h"





/***************************************************************************************************
    @function
    util_memory_alloc

    @implementation detail
    None.
***************************************************************************************************/
void* util_memory_alloc(size_t size_of_memory_to_be_allocated)
{
    void *temp_ptr;

    temp_ptr = NULL;

    if(NIL < size_of_memory_to_be_allocated)
    {
        temp_ptr = malloc(size_of_memory_to_be_allocated);
        if(temp_ptr)
        {
            memset(temp_ptr,
                   NIL,
                   size_of_memory_to_be_allocated);
        }
        else
        {
            UTIL_LOG_MSG("Failed to allocate memory");
        }
    }
    else
    {
        UTIL_LOG_MSG("size of memory to be allocated is invalid");
    }

    return temp_ptr;
}

/***************************************************************************************************
    @function
    util_memory_free

    @implementation detail
    None.
***************************************************************************************************/
void util_memory_free(void** to_be_freed_memory_ptr)
{
    if(to_be_freed_memory_ptr)
    {
        if(NULL != *to_be_freed_memory_ptr)
        {
            free(*to_be_freed_memory_ptr);
            *to_be_freed_memory_ptr = NULL;
        }
        else
        {
            UTIL_LOG_MSG("To be freed pointer is NULL");
        }
    }
    else
    {
        UTIL_LOG_MSG("Null pointer passed");
    }
}

