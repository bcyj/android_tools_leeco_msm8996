/***********************************************************************
 * tftp_malloc.c
 *
 * The TFTP malloc module.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * The TFTP malloc module.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-08-26   rp    Bring in changes from target-unit-testing.
2014-07-28   rp    Add support to detect over-write on malloc'ed buffer.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Switch to IPCRouter sockets.
2014-01-20   dks   Fix compilation on windows builds.
2013-12-16   nr    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_malloc.h"
#include "tftp_string.h"
#include "tftp_assert.h"
#include "tftp_log.h"

#include <stdlib.h>

#if defined (TFTP_ENABLE_DEBUG_MODE)

  #include "tftp_threads.h"

  #define TFTP_MALLOC_MAGIC (0xC01D10AD) /* Cold Load */

  struct tftp_malloc_stats
  {
    uint32 magic;
    int is_inited;
    tftp_mutex_handle mutex;

    uint32 max_heap_used;
    uint32 min_heap_used;
    uint32 current_heap_used;
  };

  struct tftp_malloc_header
  {
    uint32 magic;
    uint32 actual_size_malloced;
  };

  #define TFTP_MAX_MALLOC_SIZE (UINT32_MAX - sizeof(struct tftp_malloc_header))

  // todo: fix for multi-threaded
  static struct tftp_malloc_stats tftp_malloc_stats_inst;

  volatile int tftp_malloc_log_enabled = 0;

  static int
  tftp_malloc_are_stats_inited (void)
  {
    int result = 0;

    if ((tftp_malloc_stats_inst.magic == TFTP_MALLOC_MAGIC) &&
        (tftp_malloc_stats_inst.is_inited == 1))
    {
      result = 1;
    }

    return result;
  }

  static void
  tftp_malloc_update_stats_malloc (uint32 size, int is_free)
  {
    uint32 result;
    uint32 *max_heap, *current_heap, *min_heap;
    char *call_type = "MALLOC";

    max_heap = &tftp_malloc_stats_inst.max_heap_used;
    min_heap = &tftp_malloc_stats_inst.min_heap_used;
    current_heap = &tftp_malloc_stats_inst.current_heap_used;

    result = tftp_thread_lock (&tftp_malloc_stats_inst.mutex);
    TFTP_ASSERT (result == 0);
    if (result != 0)
    {
      return;
    }

    if (is_free)
    {
      TFTP_ASSERT (*current_heap >= size);
      *current_heap -= size;
      call_type = "FREE";
    }
    else
    {
      TFTP_ASSERT ((UINT32_MAX - *current_heap) >= size);
      *current_heap += size;
    }

    if (*current_heap > *max_heap)
    {
      *max_heap = *current_heap;
    }

    if (*current_heap < *min_heap)
    {
      *min_heap = *current_heap;
    }

    if (tftp_malloc_log_enabled)
    {
      TFTP_LOG_INFO("HEAP INFO: OP=%s, size = %u, curent_heap = %u "
                    "min_heap = %u, max_heap = %u\n", call_type, size,
                    *current_heap, *min_heap, *max_heap);
    }

    result = tftp_thread_unlock (&tftp_malloc_stats_inst.mutex);
    TFTP_ASSERT (result == 0);
  }

  void
  tftp_malloc_init (void)
  {
    int result;

    memset (&tftp_malloc_stats_inst, 0, sizeof (tftp_malloc_stats_inst));

    result = tftp_thread_mutex_init (&tftp_malloc_stats_inst.mutex);
    TFTP_ASSERT (result == 0);
    if (result != 0)
    {
      return;
    }

    tftp_malloc_stats_inst.magic = TFTP_MALLOC_MAGIC;
    tftp_malloc_stats_inst.is_inited = 1;
    tftp_malloc_stats_inst.min_heap_used = ~(0);
  }

  void*
  tftp_malloc (uint32 size)
  {
    uint32 malloc_size;
    uint8 *ptr, *end_ptr;
    struct tftp_malloc_header *header, *footer;
    int result;

    result = tftp_malloc_are_stats_inited ();
    TFTP_ASSERT (result == 1);
    if (result != 1)
    {
      return NULL;
    }

    if (size > TFTP_MAX_MALLOC_SIZE)
    {
      return NULL;
    }
    malloc_size = size + sizeof (struct tftp_malloc_header) +
                         sizeof (struct tftp_malloc_header);

    ptr = (uint8 *) malloc (malloc_size);
    TFTP_ASSERT (ptr != NULL);
    if (ptr == NULL)
    {
      return NULL;
    }
    end_ptr = ptr + (size + sizeof (struct tftp_malloc_header));

    header = (struct tftp_malloc_header *) ptr;
    footer = (struct tftp_malloc_header *) end_ptr;
    ptr += sizeof (struct tftp_malloc_header);

    header->magic = TFTP_MALLOC_MAGIC;
    header->actual_size_malloced = size;

    footer->magic = TFTP_MALLOC_MAGIC;
    footer->actual_size_malloced = size;

    tftp_malloc_update_stats_malloc (size, 0);

    return (void *)ptr;
  }

  void
  tftp_free (void *ptr)
  {
    uint8 *actual_ptr, *end_ptr;
    struct tftp_malloc_header *header, *footer;
    int result;

    result = tftp_malloc_are_stats_inited ();
    TFTP_ASSERT (result == 1);
    if (result != 1)
    {
      return;
    }

    TFTP_ASSERT (ptr != NULL);
    if (ptr == NULL)
    {
      TFTP_LOG_ERR ("FREE of NULL attempted!!!");
      return;
    }

    actual_ptr = (uint8 *)ptr - sizeof (struct tftp_malloc_header);


    header = (struct tftp_malloc_header *) actual_ptr;
    end_ptr = (((uint8*)ptr) + header->actual_size_malloced);
    footer = (struct tftp_malloc_header *) end_ptr;

    TFTP_ASSERT (header->magic == TFTP_MALLOC_MAGIC);
    TFTP_ASSERT (footer->magic == TFTP_MALLOC_MAGIC);
    TFTP_ASSERT (footer->actual_size_malloced ==
                       header->actual_size_malloced);

    tftp_malloc_update_stats_malloc (header->actual_size_malloced, 1);

    free (actual_ptr);
  }

#else

  void
  tftp_malloc_init (void)
  {
    ((void)0);
  }

  void *
  tftp_malloc (uint32 size)
  {
    return malloc (size);
  }

  void
  tftp_free (void *ptr)
  {
    free (ptr);
  }

#endif
