/***********************************************************************
 * tftp_server_folders_la.c
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
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Create

===========================================================================*/

#include "tftp_server_config.h"
#include "tftp_server_folders.h"
#include "tftp_assert.h"
#include "tftp_file.h"
#include "tftp_os.h"
#include "tftp_log.h"
#include "string.h"

#if !defined (TFTP_LA_BUILD)
  #error "This file should only be compiled for LA builds."
#endif

#if defined (TFTP_SIMULATOR_BUILD)
  #define PATH_PREFIX  SIM_OUTPUT_DIR
#else
  #define PATH_PREFIX  ""
#endif

#if defined (TFTP_SIMULATOR_BUILD)
  #define DATA_PATH_PREFIX  SIM_OUTPUT_DIR
#elif defined (TFTP_LE_BUILD_ONLY)
  #define DATA_PATH_PREFIX  "/usr"
#else
  #define DATA_PATH_PREFIX  ""
#endif

struct tftp_server_folders_path_prefix_map_type
{
  uint32 instance_id;
  const char *path_prefix;
};

static struct tftp_server_folders_path_prefix_map_type
  tftp_server_folders_path_prefix_map[] =
{
  {TFTP_SERVER_INSTANCE_ID_MSM_MPSS,    PATH_PREFIX"/system/rfs/msm/mpss"},
  {TFTP_SERVER_INSTANCE_ID_MSM_ADSP,    PATH_PREFIX"/system/rfs/msm/adsp"},
  {TFTP_SERVER_INSTANCE_ID_MDM_MPSS,    PATH_PREFIX"/system/rfs/mdm/mpss"},
  {TFTP_SERVER_INSTANCE_ID_MDM_ADSP,    PATH_PREFIX"/system/rfs/mdm/adsp"},
  {TFTP_SERVER_INSTANCE_ID_MDM_SPARROW, PATH_PREFIX"/system/rfs/mdm/sparrow"},
  {TFTP_SERVER_INSTANCE_ID_APQ_GSS,     PATH_PREFIX"/system/rfs/apq/gnss"},
};

/* One entry is required per prefix above. */
static const char* tftp_server_folders_shared_real_path_list[] = {
  PATH_PREFIX"/system/rfs/msm/mpss/hlos/",
  PATH_PREFIX"/system/rfs/msm/adsp/hlos/",
  PATH_PREFIX"/system/rfs/mdm/mpss/hlos/",
  PATH_PREFIX"/system/rfs/mdm/adsp/hlos/",
  PATH_PREFIX"/system/rfs/mdm/sparrow/hlos/",
  PATH_PREFIX"/system/rfs/apq/gnss/hlos/",
};

static const char* tftp_server_folders_data_folders_list[] = {
  DATA_PATH_PREFIX"/persist/rfs/shared/",
  DATA_PATH_PREFIX"/persist/rfs/msm/mpss/",
  DATA_PATH_PREFIX"/persist/rfs/msm/adsp/",
  DATA_PATH_PREFIX"/persist/rfs/mdm/mpss/",
  DATA_PATH_PREFIX"/persist/rfs/mdm/adsp/",
  DATA_PATH_PREFIX"/persist/rfs/mdm/sparrow/",
  DATA_PATH_PREFIX"/persist/rfs/apq/gnss/",
};

const char* tftp_server_folders_hlos_shared_data_folders_list[] = {
  DATA_PATH_PREFIX"/persist/hlos_rfs/shared/",
};


/* TODO: Fix to chmod and chown for sharing.  */
static int32
tftp_server_folders_create_hlos_shared_data_folders (void)
{
  uint32 i, num_entries;
  int32 result, error_count;
  const char *dir_path;

  num_entries = (sizeof (tftp_server_folders_hlos_shared_data_folders_list) /
                sizeof (tftp_server_folders_hlos_shared_data_folders_list[0]));

  error_count = 0;

  for ( i = 0; i < num_entries; ++i )
  {
    dir_path = tftp_server_folders_hlos_shared_data_folders_list[i];
    TFTP_ASSERT (dir_path != NULL);
    if ((dir_path == NULL) || (*dir_path == '\0'))
    {
      continue;
    }

    result = tftp_os_auto_mkdir (dir_path, TFTP_SHARED_DIR_MODE,
                                  TFTP_SHARED_GID);
    if (result != 0)
    {
      error_count++;
      TFTP_LOG_ERR("Failed to auto_dir for(%s) errno = %d (%s)", dir_path,
                    result, strerror(-result));
    }
    tftp_os_mkdir (dir_path, TFTP_SHARED_DIR_MODE);
  }

  result = 0;
  if (error_count > 0)
  {
    result = 1;
  }
  return result;
}

static int32
tftp_server_folders_create_data_folders (void)
{
  uint32 i, num_entries;
  int32 result, error_count;
  const char *dir_path;

  num_entries = (sizeof (tftp_server_folders_data_folders_list) /
                 sizeof (tftp_server_folders_data_folders_list[0]));

  error_count = 0;

  for ( i = 0; i < num_entries; ++i )
  {
    dir_path = tftp_server_folders_data_folders_list[i];
    TFTP_ASSERT (dir_path != NULL);
    if ((dir_path == NULL) || (*dir_path == '\0'))
    {
      continue;
    }

    result = tftp_os_auto_mkdir (dir_path, TFTP_DEFAULT_DIR_MODE, -1);
    if (result != 0)
    {
      error_count++;
      TFTP_LOG_ERR("Failed to auto_dir for(%s) errno = %d (%s)", dir_path,
                    result, strerror(-result));
    }
    tftp_os_mkdir (dir_path, TFTP_DEFAULT_DIR_MODE);
  }

  result = 0;
  if (error_count > 0)
  {
    result = 1;
  }
  return result;
}

int
tftp_server_folders_init (void)
{
  int result;

  result = tftp_server_folders_create_data_folders ();

  if(result != 0)
  {
    return result;
  }

  return tftp_server_folders_create_hlos_shared_data_folders ();
}

const char*
tftp_server_folders_lookup_path_prefix (uint32 instance_id)
{
  uint32 i, num_entries;
  const char *path_prefix = NULL;
  struct tftp_server_folders_path_prefix_map_type *prefix_entry;

  num_entries = (sizeof (tftp_server_folders_path_prefix_map) /
                 sizeof (tftp_server_folders_path_prefix_map[0]));

  for ( i = 0; i < num_entries; ++i )
  {
    prefix_entry = &tftp_server_folders_path_prefix_map[i];
    if (prefix_entry->instance_id == instance_id)
    {
      path_prefix = prefix_entry->path_prefix;
      break;
    }
  }

  return path_prefix;
}

int
tftp_server_folders_check_if_shared_file (const char *path)
{
  uint32 i, num_entries, length;
  int is_shared_file = 0;
  const char *path_prefix = NULL;

  num_entries = (sizeof (tftp_server_folders_shared_real_path_list) /
                 sizeof (tftp_server_folders_shared_real_path_list[0]));

  for ( i = 0; i < num_entries; ++i )
  {
    path_prefix = tftp_server_folders_shared_real_path_list[i];
    TFTP_ASSERT (path_prefix != NULL);
    if ((path_prefix == NULL) || (*path_prefix == '\0'))
    {
      continue;
    }
    length = strlen(path_prefix);
    if (strncmp (path, path_prefix, length) == 0)
    {
      TFTP_LOG_DBG ("Found shared path : %s", path);
      is_shared_file = 1;
      break;
    }
  }

  return is_shared_file;
}

