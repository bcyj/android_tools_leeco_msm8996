/***********************************************************************
 * tftp_os_wakelocks_la.c
 *
 * Short description.
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose Description
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
2014-08-19   nr   Create

===========================================================================*/
#include "tftp_os_wakelocks.h"
#include "tftp_log.h"
#include "tftp_assert.h"
#include "tftp_os.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#if !defined (TFTP_LA_BUILD)
  #error "This file should only be included for LA build"
#endif

#ifdef TFTP_SIMULATOR_BUILD
  extern int setgroups(size_t size, const gid_t *list);
#endif

#ifdef TFTP_LE_BUILD_ONLY
  #define TFTP_ROOT_USER 0
#else
  #define TFTP_ROOT_USER AID_ROOT
#endif

#ifndef TFTP_LE_BUILD_ONLY
  /* For capabilities */
  #include <private/android_filesystem_config.h>
  #include <sys/prctl.h>
  #include <sys/capability.h>

  /*
   * CAP_BLOCK_SUSPEND was introduced in 3.5 so it may not be present if we are
   * building in older builds.
   */
  #ifndef CAP_BLOCK_SUSPEND
    #define CAP_BLOCK_SUSPEND 36

    #ifdef CAP_LAST_CAP
      #undef CAP_LAST_CAP
      #define CAP_LAST_CAP CAP_BLOCK_SUSPEND
    #endif /* ifdef CAP_LAST_CAP */

  #endif /* ifndef CAP_BLOCK_SUSPEND */

#endif /* #ifndef TFTP_LE_BUILD_ONLY */

#if defined (TFTP_SIMULATOR_BUILD)
  #define PATH_PREFIX  SIM_OUTPUT_DIR
#else
  #define PATH_PREFIX  ""
#endif

/* Wake locks */
#define WAKE_LOCK_O_FLAGS  (O_WRONLY|O_APPEND)

#define WAKE_LOCK_FILE    PATH_PREFIX"/sys/power/wake_lock"
#define WAKE_UNLOCK_FILE  PATH_PREFIX"/sys/power/wake_unlock"

#define WAKE_LOCK_STRING  "tftp_server_wakelock"

/* To set the supplementary-groups */
#define TFTP_GID_LIST_SIZE 4

struct tftp_os_la_wakelock_struct
{
  int                lock_fd;
  int                unlock_fd;
  ssize_t            write_size;
  int                is_inited;
  uint64             lock_count;
  uint64             unlock_count;
};

static struct tftp_os_la_wakelock_struct  tftp_os_la_wakelock = {0};

static int open_wakelock_files (void)
{
  tftp_os_la_wakelock.unlock_fd = -1;
  tftp_os_la_wakelock.lock_fd = -1;

  /* Open wakelock files */
  tftp_os_la_wakelock.lock_fd = open(WAKE_LOCK_FILE, WAKE_LOCK_O_FLAGS);
  if(tftp_os_la_wakelock.lock_fd < 0)
  {
    TFTP_LOG_ERR("Unable to open wake lock file %s "
                 "return val: %d, error no: %d string = %s\n",
                 WAKE_LOCK_FILE, tftp_os_la_wakelock.lock_fd, errno,
                 strerror(errno));
    return -1;
  }

  tftp_os_la_wakelock.unlock_fd = open(WAKE_UNLOCK_FILE, WAKE_LOCK_O_FLAGS);
  if(tftp_os_la_wakelock.unlock_fd < 0)
  {
    close(tftp_os_la_wakelock.lock_fd);
    tftp_os_la_wakelock.lock_fd = -1;
    TFTP_LOG_ERR("Unable to open wake unlock file %s "
                "return val: %d, error no: %d string = %s\n",
                WAKE_UNLOCK_FILE, tftp_os_la_wakelock.unlock_fd, errno,
                strerror(errno));
    return -1;
  }

  return 0;
}

#if defined (TFTP_LE_BUILD_ONLY) || defined (TFTP_SIMULATOR_BUILD)

static int setup_wakelock_perms (void)
{
  return 0;
}

#else

/* Drop unused capabilities */
static void drop_excess_capabilities(void)
{
  unsigned long cap;
  int err;

  /* Allow the caps to be retained after change uid.
     If this fails only wakelock capability is lost. */
  if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0)
  {
    TFTP_LOG_ERR("set keepcaps failed! errno=%d ( %s )", errno,
                  strerror(errno));
  }

  for (cap = 0; prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0; cap++)
  {

    if ((cap == CAP_SETUID) || (cap == CAP_SETGID) ||
        (cap == CAP_BLOCK_SUSPEND))
    {
      continue;
    }

    err = prctl(PR_CAPBSET_DROP, cap, 0, 0, 0);
    if ((err < 0) && (errno != EINVAL))
    {
      TFTP_LOG_ERR("Drop capability for cap=%lu failed errno=%d ( %s )", cap,
        errno, strerror(errno));
    }
  }
}

static void enable_capabilities(void)
{
  int ret;
  struct __user_cap_header_struct capheader;
  struct __user_cap_data_struct capdata[2];

  memset(&capheader, 0, sizeof(capheader));
  memset(&capdata, 0, sizeof(capdata));

  capheader.version = _LINUX_CAPABILITY_VERSION_3;
  capheader.pid = 0; /* self  */

  /* Enable block suspend */
  capdata[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].permitted |=
                                                CAP_TO_MASK(CAP_BLOCK_SUSPEND);

  capdata[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].effective |=
                                                CAP_TO_MASK(CAP_BLOCK_SUSPEND);

  /* If this fails only wakelock capability is lost. */
  if ((ret = capset(&capheader, capdata)) < 0)
  {
    TFTP_LOG_ERR("capset failed: errno=%d ( %s ), ret = %d\n", errno,
                  strerror(errno), ret);
  }
  else
  {
    TFTP_LOG_DBG("Capset success!");
  }
}

static int setup_wakelock_perms (void)
{
  int ret;
  gid_t gid_list[TFTP_GID_LIST_SIZE] = {AID_RFS, AID_RFS_SHARED, AID_SYSTEM,
                                        AID_NET_RAW};

  /* Drop the excess capabilities. */
  drop_excess_capabilities();

  ret = setgroups(TFTP_GID_LIST_SIZE, gid_list);
  if (ret < 0)
  {
    TFTP_LOG_ERR("Failed to set supplementary-groups(ret %d) errno = %d (%s)",
                  ret, errno, strerror(errno));
    return -1; /* It is BAD if we cannot set supplementary gids */
  }

  ret = setgid(AID_RFS);
  if (ret < 0)
  {
    TFTP_LOG_ERR("Failed to set GID to RFS (ret %d) errno = %d (%s)",
                  ret, errno, strerror(errno));
    return -1; /* It is BAD if we cannot drop to RFS from root */
  }

  ret = setuid(AID_RFS);
  if (ret < 0)
  {
    TFTP_LOG_ERR("Failed to set UID to RFS (ret %d) errno = %d (%s)",
                  ret, errno, strerror(errno));
    return -1; /* It is BAD if we cannot drop to RFS from root */
  }

  /* enable the capabilities after uid changed. */
  enable_capabilities();
  return 0;
}

#endif /* #ifdef TFTP_LE_BUILD_ONLY */

void
tftp_os_wakelocks_init(void)
{
  int result;
  uid_t my_uid;

  memset (&tftp_os_la_wakelock, 0, sizeof(tftp_os_la_wakelock));

  my_uid = getuid();

  if (my_uid != TFTP_ROOT_USER)
  {
    TFTP_LOG_ERR("Not ROOT user but as user:%u so cant init wakelock\n",
                  my_uid);
    return;
  }

  /* This should occur before opening the files to prevent SE Linux from
     blocking the service. */
  result = setup_wakelock_perms();
  if(result != 0)
  {
    return;
  }

  result = open_wakelock_files();
  if(result != 0)
  {
    return;
  }

  tftp_os_la_wakelock.write_size = strlen(WAKE_LOCK_STRING);
  tftp_os_la_wakelock.is_inited = 1;
}

void
tftp_os_wakelock(void)
{
  ssize_t write_res;

  TFTP_ASSERT(tftp_os_la_wakelock.unlock_count ==
              tftp_os_la_wakelock.lock_count);
  tftp_os_la_wakelock.lock_count++;

  if (tftp_os_la_wakelock.is_inited != 1)
  {
    #ifndef TFTP_LE_BUILD_ONLY
      TFTP_LOG_ERR("TFTP_wakelock : NOT initialized \n");
    #endif
    return;
  }
  write_res = write(tftp_os_la_wakelock.lock_fd, WAKE_LOCK_STRING,
                    tftp_os_la_wakelock.write_size);
  if(write_res == tftp_os_la_wakelock.write_size)
  {
    TFTP_LOG_DBG("Wake-lock acquired successfully\n");
  }
  else
  {
    TFTP_LOG_ERR("Wake-lock *NOT* acquired,write_res =%zd  errno %d (%s)\n",
                  write_res, errno, strerror(errno));
  }
}

void
tftp_os_wakeunlock(void)
{
  ssize_t write_res;

  tftp_os_la_wakelock.unlock_count++;
  TFTP_ASSERT(tftp_os_la_wakelock.unlock_count ==
              tftp_os_la_wakelock.lock_count);

  if (tftp_os_la_wakelock.is_inited != 1)
  {
    #ifndef TFTP_LE_BUILD_ONLY
      TFTP_LOG_ERR("TFTP_wakeunlock : NOT initialized \n");
    #endif
    return;
  }
  write_res = write(tftp_os_la_wakelock.unlock_fd, WAKE_LOCK_STRING,
                    tftp_os_la_wakelock.write_size);

  if(write_res == tftp_os_la_wakelock.write_size)
  {
    TFTP_LOG_DBG("Wake-lock released successfully\n");
  }
  else
  {
    TFTP_LOG_ERR("Wake-lock *NOT* released,write_res =%zd  errno %d (%s)\n",
                  write_res, errno, strerror(errno));
  }
}
