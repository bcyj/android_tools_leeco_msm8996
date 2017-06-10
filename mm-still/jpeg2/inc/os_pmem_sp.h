/*========================================================================

*//** @file os_pmem_sp.h

OS abstracted PMEM layer (specific to Linux)

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
05/12/09   vma     Created file.

========================================================================== */

#ifndef _OS_PMEM_SP_H
#define _OS_PMEM_SP_H

#include "os_int.h"
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/msm_adsp.h>
#ifdef ANDROID
#include <linux/android_pmem.h>
#endif

#ifndef PAGESIZE
#define PAGESIZE  4096
#endif

#ifndef PMEM_GET_PHYS
#define PMEM_GET_PHYS   _IOW('p', 1, unsigned int)
#endif //PMEM_GET_PHYS

// file descriptor for pmem driver
typedef int pmem_fd_t;

#define OS_PMEM_FD_INIT_VALUE_SP   -1
#endif // #ifndef _OS_PMEM_SP_H

