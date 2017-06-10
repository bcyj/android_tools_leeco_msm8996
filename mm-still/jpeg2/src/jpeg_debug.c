/*========================================================================

*//** @file jpeg_debug.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/03/09   vma     Switched to use the os abstraction layer (os_*)
07/16/09   zhiminl Increased TBL_SIZE to 200.
09/07/08   vma     Created file.

========================================================================== */

#include "jpeg_debug.h"
#include "os_thread.h"
#include <stdlib.h>
#include "jpeg_common_private.h"
#include "jpeglog.h"
#include <utils/Log.h>
#include <unwind.h>
#include <sys/types.h>

#define LOG_TAG "mm-still"

#ifdef _DEBUG

/* depends how the system includes define this */
#ifdef HAVE_UNWIND_CONTEXT_STRUCT
    typedef struct _Unwind_Context __unwind_context;
#else
    typedef _Unwind_Context __unwind_context;
#endif


// Constants
#define TBL_SIZE 200

// Typedefs
typedef struct
{
    void*         ptr;
    const char*   file_name;
    uint32_t      line_num;
    uint8_t       is_valid;
    intptr_t bt [17];
    int i;
} malloc_entry_t;

// Static variables
static uint8_t         is_initialized = false;
static uint8_t         is_table_full  = false;
static os_mutex_t      leak_mutex;
static malloc_entry_t  malloc_table[TBL_SIZE];

static _Unwind_Reason_Code trace_function_jpeg(__unwind_context *context, void *arg)
{
    int i =0;
    malloc_entry_t * t = (malloc_entry_t * )arg;
    intptr_t ip = (intptr_t)_Unwind_GetIP(context);
    if(t->i> 0) {
        t->i--;
        t->bt[t->i] = ip;
        return _URC_NO_REASON;
    }

    /*
     * If we run out of space to record the address or 0 has been seen, stop
     * unwinding the stack.
     */
    return _URC_END_OF_STACK;

}

void *jpeg_malloc(uint32_t  size, const char* file_name, uint32_t line_num)
{
    int i;
    void *ptr = malloc(size);
    if (!is_initialized)
    {
        STD_MEMSET(malloc_table, 0, sizeof(malloc_entry_t) * TBL_SIZE);
        os_mutex_init(&leak_mutex);
        is_initialized = true;
    }
    os_mutex_lock(&leak_mutex);
    if (!is_table_full && ptr)
    {
        for (i = 0; i < TBL_SIZE; i++)
        {
            if (!malloc_table[i].is_valid)
            {
                malloc_table[i].is_valid = true;
                os_mutex_unlock(&leak_mutex);
                malloc_table[i].file_name = file_name;
                malloc_table[i].line_num = line_num;
                malloc_table[i].ptr = ptr;
                malloc_table[i].i = 15;
                _Unwind_Backtrace(trace_function_jpeg, (void*)&malloc_table[i]);

                return ptr;
            }
        }
        is_table_full = true;
    }
    os_mutex_unlock(&leak_mutex);
    return ptr;
}

void jpeg_free(void *ptr)
{
    int i;
    os_mutex_lock(&leak_mutex);
    if (!is_initialized)
    {
        STD_MEMSET(malloc_table, 0, sizeof(malloc_entry_t) * TBL_SIZE);
        is_initialized = true;
    }
    free(ptr);
    if (!is_table_full)
    {
        for (i = 0; i < TBL_SIZE; i++)
        {
            if (malloc_table[i].ptr == ptr && malloc_table[i].is_valid)
            {
                malloc_table[i].is_valid = false;
                os_mutex_unlock(&leak_mutex);
                return;
            }
        }
    }
    os_mutex_unlock(&leak_mutex);
}

void jpeg_show_leak(void)
{
    int i;
    int leak_cnt = 0;
    int cnt = 0;
    JPEG_DBG_LOW("Leak detection:\n");
    if (is_table_full)
    {
        JPEG_DBG_LOW("Table full. Tracking stopped\n");
        return;
    }
    for (i = 0; i < TBL_SIZE; i++)
    {
        if (malloc_table[i].is_valid)
        {
            leak_cnt++;
            JPEG_DBG_LOW("%.3d:  %s:%d\n", leak_cnt, malloc_table[i].file_name,
                 malloc_table[i].line_num);
            for (cnt = 0; cnt < 15; cnt++)
                JPEG_DBG_LOW("    %2d %p", cnt, (void *)malloc_table[i].bt[cnt]);

        }
    }
    JPEG_DBG_LOW("Total # leak: %d\n", leak_cnt);
}

#endif // #ifdef _DEBUG

