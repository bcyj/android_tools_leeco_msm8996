/***********************************************************************
 * tftp_server.c
 *
 * The TFTP Server module.
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * The TFTP Server module
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2015-01-05   vm    Compiling server for TN Apps.
2014-12-10   dks   Separate client and server configs.
2014-10-17   vm    Fix the parameters in log messages.
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-10-14   dks   Move OS specific arguments out of tftp_server.
2014-07-28   rp    Add debug_info to measure timings.
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Switch to IPCRouter sockets.
2014-01-20   dks   Correct unlocking of mutex for windows.
2014-01-20   rp    Add support to do partial-file reads.
2013-12-26   rp    Add tftp-client module.
2013-12-12   nr    Cleanup rrq and wrq handler implementations.
2013-12-11   nr    Support all extended options.
2013-12-06   rp    Improve OPACK logic.
2013-12-05   rp    Add new debug-log interface.
2013-11-25   nr    Move common code to receive data.
2013-11-25   nr    Support window size option + move common code to send data.
2013-11-24   rp    Support TFTP extension options.
2013-11-21   nr    Abstract the OS layer across the OS's.
2013-11-14   rp    Create

===========================================================================*/

#include "tftp_server_config.h"
#include "tftp_server.h"
#include "tftp_comdef.h"
#include "tftp_file.h"
#include "tftp_os.h"
#include "tftp_log.h"
#include "tftp_connection.h"
#include "tftp_protocol.h"
#include "tftp_assert.h"
#include "tftp_threads.h"
#include "tftp_string.h"
#include "tftp_malloc.h"
#include "tftp_server_utils.h"
#include "tftp_server_folders.h"
#include "tftp_os_wakelocks.h"

// todo: fix this : do not include this file directly
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef FEATURE_TFTP_CLIENT_BUILD
  #error "Incorrect build configuration. Server is enabled in Client build."
#endif

#define TFTP_SERVER_TEMP_FILE_EXTENSION ".rfs_tmp"
#define TFTP_SERVER_TEMP_FILE_EXTENSION_LEN 8

#define TFTP_SERVER_MAX_TEMP_FILE_LEN (TFTP_MAX_FILE_NAME_LEN + \
                                       TFTP_SERVER_TEMP_FILE_EXTENSION_LEN)

#define TFTP_CLIENT_INSTANCE_START_MAGIC    0xB1234567
#define TFTP_CLIENT_INSTANCE_END_MAGIC      0xE7654321

struct tftp_server_config_info_type
{
  enum tftp_server_instance_id_type instance;
  uint32 service_id;
  tftp_sockaddr server_addr;
  tftp_socket socket_handle;
  int is_valid;
};


struct tftp_server_info_type;

struct tftp_server_debug_info_type
{
  uint64 total_start_time_tick;
  uint64 total_end_time_tick;
  uint64 total_time_delta;
  uint64 req_start_time_tick;
  uint64 req_end_time_tick;
  uint64 req_time_delta;
};

struct tftp_server_client_info_type
{
  uint32 start_magic;
  int is_in_use;

  tftp_thread_handle client_threads_hdl;

  struct tftp_server_info_type *server_info;

  struct tftp_connection_ipcr_addr remote_addr;
  enum tftp_server_instance_id_type server_instance;

  char raw_file_name[TFTP_MAX_FILE_NAME_LEN + 1];

  char file_name[TFTP_MAX_FILE_NAME_LEN + 1];
  char temp_file_name[TFTP_SERVER_MAX_TEMP_FILE_LEN + 1];
  tftp_file_handle fd;
  uint32 file_size;
  int8 is_atomic_put;

  void *data_buffer;
  uint32 data_buffer_size;

  struct tftp_protocol_info proto;

  struct tftp_server_debug_info_type debug_info;

  uint32 end_magic;
};


struct tftp_server_info_type
{
  tftp_mutex_handle server_mutex;
  struct tftp_server_config_info_type config[TFTP_SERVER_INSTANCE_ID_MAX];
  tftp_socket_pollfd pollfd_array[TFTP_SERVER_INSTANCE_ID_MAX];

  uint32 current_client_count;

  uint64 request_count;

  struct tftp_pkt_type rx_pkt;
  int stop_server;
};

static struct tftp_server_info_type tftp_server_inst;

static tftp_thread_return_type tftp_client_thread_func (void *arg);

#define TFTP_SERVER_EXIT(error) tftp_server_exit (error, __LINE__)

static void
tftp_server_exit (int error_code, uint32 line_num)
{
  TFTP_LOG_ERR ("TFTP-Error : Exiting with error %d at line %d", error_code,
                line_num);
  exit (error_code);
}

static void
tftp_server_create_info_file (void)
{
  int32 result, length;
  tftp_file_handle handle;
  char info_file[100];
  char buffer[20];
  const char *path;
  uint32 instance_id = TFTP_SERVER_INSTANCE_ID_MSM_MPSS;

  path = tftp_server_folders_lookup_path_prefix (instance_id);

  if (path == NULL)
  {
    TFTP_LOG_ERR ("instance id: %u has path prefix of NULL", instance_id);
    goto error;
  }

  snprintf (info_file, sizeof (info_file), "%s/%s",
            path, TFTP_SERVER_INFO_FILEPATH);

  result = tftp_file_get_file_size (info_file);
  if (result < 0)
  {
    if (result != -ENOENT )
    {
      /* Failed for some other reason.  */
      TFTP_LOG_ERR ("Stat if %s failed with %d", info_file, result);
      goto error;
    }
  }
  else
  {
    return; /* File already exists */
  }

  result = tftp_file_open (&handle, info_file, "wb");
  TFTP_ASSERT (result == 0);
  if ((result < 0) || (handle == NULL))
  {
    TFTP_LOG_ERR ("Open of  %s failed with %d", info_file, result);
    goto error;
  }

  length = snprintf (buffer, sizeof(buffer), "TFTP_SERVER.%d.%d",
                     TFTP_MAJOR_VERSION, TFTP_MINOR_VERSION);
  TFTP_ASSERT (length < (int32)sizeof(buffer));
  if (length >= (int32)sizeof(buffer))
  {
    TFTP_LOG_ERR ("Write  %s buffer too small", buffer);
    goto error;
  }

  result = tftp_file_write (handle, buffer, length);
  TFTP_ASSERT (result == length);
  if (result < 0)
  {
    TFTP_LOG_ERR ("Write of  %s failed with %d", info_file, result);
    goto error;
  }

  tftp_file_close (handle);
  return;

error:
  TFTP_LOG_ERR ("Info file creation failed..!");
}

static void
tftp_server_read_build_config (struct tftp_server_info_type *server_info)
{
  struct tftp_server_config_info_type *config;
  enum tftp_server_instance_id_type inst;
  enum tftp_socket_result_type sock_res;

  TFTP_ASSERT (server_info != NULL);
  if (server_info == NULL)
  {
    TFTP_LOG_ERR ("server_info is null");
    return;
  }

  for (inst = TFTP_SERVER_INSTANCE_ID_INVALID;
       inst < TFTP_SERVER_INSTANCE_ID_MAX; inst++)
  {
    if (inst == TFTP_SERVER_INSTANCE_ID_INVALID)
    {
      continue;
    }

    config = &server_info->config[inst];

    sock_res = tftp_socket_set_addr_by_name (&config->server_addr,
                                              TFTP_SERVER_SERVICE_ID, inst);
    TFTP_ASSERT (sock_res == TFTP_SOCKET_RESULT_SUCCESS);

    config->service_id = TFTP_SERVER_SERVICE_ID;
    config->instance = inst;
    config->is_valid = 1;
  }
}

static void
tftp_server_init (void)
{
  uint32 result;

  memset (&tftp_server_inst, 0, sizeof (tftp_server_inst));

  tftp_server_read_build_config (&tftp_server_inst);

  tftp_malloc_init ();

  result = tftp_thread_mutex_init (&tftp_server_inst.server_mutex);
  TFTP_ASSERT (result == 0);
}

static struct tftp_server_client_info_type*
tftp_server_get_client_instance (void)
{
  uint32 result;
  struct tftp_server_client_info_type *client_info = NULL;

  result = tftp_thread_lock (&tftp_server_inst.server_mutex);
  TFTP_ASSERT (result == 0);
  if (result != 0)
  {
    return NULL;
  }

  if (tftp_server_inst.current_client_count >= TFTP_SERVER_MAX_CLIENTS)
  {
    goto End;
  }

  client_info = tftp_malloc (sizeof (struct tftp_server_client_info_type));

  if (client_info == NULL)
  {
    goto End;
  }

  memset (client_info, 0, sizeof (struct tftp_server_client_info_type));
  client_info->is_in_use = 1;
  client_info->server_info = &tftp_server_inst;

  /* There are no requests pending...grab the wakelock. */
  if(tftp_server_inst.current_client_count == 0)
  {
    tftp_os_wakelock ();
  }
  /* We have enough resources for the client info so increment the count */
  ++tftp_server_inst.current_client_count;

  client_info->start_magic = TFTP_CLIENT_INSTANCE_START_MAGIC;
  client_info->end_magic   = TFTP_CLIENT_INSTANCE_END_MAGIC;

End:
  result = tftp_thread_unlock (&tftp_server_inst.server_mutex);
  TFTP_ASSERT (result == 0);
  return client_info;
}

static void
tftp_server_free_client_instance (
    struct tftp_server_client_info_type *client_info)
{
  uint32 result;

  TFTP_ASSERT (client_info != NULL);
  if (client_info == NULL)
  {
    return;
  }

  result = tftp_thread_lock (&tftp_server_inst.server_mutex);
  TFTP_ASSERT (result == 0);
  if (result != 0)
  {
    return;
  }

  tftp_free (client_info);
  (tftp_server_inst.current_client_count)--;

  /* There are no requests pending...release the wakelock. */
  if(tftp_server_inst.current_client_count == 0)
  {
    tftp_os_wakeunlock ();
  }

  result = tftp_thread_unlock (&tftp_server_inst.server_mutex);
  TFTP_ASSERT (result == 0);
}

static int32
tftp_get_real_filepath(struct tftp_server_client_info_type *client_info)
{
  int32 result;
  size_t len;
  const char *pkt_filename;
  uint32 pkt_filename_len;

  pkt_filename = client_info->proto.rx_pkt.pkt_fields.req_pkt.filename;
  pkt_filename_len = strlen (pkt_filename);

  if (pkt_filename_len >= TFTP_MAX_FILE_NAME_LEN)
  {
    return -ENAMETOOLONG;
  }

  len = tftp_strlcpy (client_info->raw_file_name,
                 client_info->proto.rx_pkt.pkt_fields.req_pkt.filename,
                 sizeof (client_info->raw_file_name));
  if (len >= sizeof (client_info->raw_file_name))
  {
    return -ENAMETOOLONG;
  }

  //todo: fix this instance-id
  result = tftp_server_utils_normalize_path (client_info->raw_file_name,
                                              client_info->server_instance,
                                              client_info->file_name,
                                              sizeof (client_info->file_name));
  if (result != 0)
  {
    return result;
  }

  return 0;
}

static int
tftp_server_open_sockets(struct tftp_server_info_type *server_info)
{
  struct tftp_server_config_info_type *config;
  enum tftp_server_instance_id_type inst;
  tftp_socket_pollfd *pollfd_array;
  tftp_socket socket_handle;
  enum tftp_socket_result_type sock_res;

  TFTP_ASSERT (server_info != NULL);
  if (server_info == NULL)
  {
    TFTP_LOG_ERR ("server_info is null");
    return -1;
  }

  pollfd_array = server_info->pollfd_array;

  for(inst = TFTP_SERVER_INSTANCE_ID_INVALID;
      inst < TFTP_SERVER_INSTANCE_ID_MAX; inst++)
  {
    config = &server_info->config[inst];

    if (config->is_valid == 1)
    {
      sock_res = tftp_socket_open (&socket_handle, 0); /* Bind explicitly */
      TFTP_ASSERT (sock_res == TFTP_SOCKET_RESULT_SUCCESS);
      if (sock_res != TFTP_SOCKET_RESULT_SUCCESS)
      {
        TFTP_LOG_ERR ("Socket open failed errno = %d",
                      tftp_socket_get_last_errno (&socket_handle));
        return -1;
      }

      sock_res = tftp_socket_bind (&socket_handle, &config->server_addr);
      TFTP_ASSERT (sock_res == TFTP_SOCKET_RESULT_SUCCESS);
      if (sock_res != TFTP_SOCKET_RESULT_SUCCESS)
      {
        (void) tftp_socket_close (&socket_handle);
        return -1;
      }

      config->socket_handle = socket_handle;
      pollfd_array[inst].fd = tftp_socket_get_socket_desc (&socket_handle);
    }
    else
    {
      tftp_socket_invalidate_socket_handle (&config->socket_handle);
      pollfd_array[inst].fd = -1;
      pollfd_array[inst].events = 0;
      pollfd_array[inst].revents = 0;
    }
  }
  return 0;
}

static void
tftp_server_process_pkt (tftp_socket socket_handle,
                         struct tftp_pkt_type *rx_pkt,
                         enum tftp_server_instance_id_type instance)
{
  struct tftp_server_client_info_type *client_info = NULL;
  enum tftp_socket_result_type sock_res;
  int32 rx_data_size;
  tftp_sockaddr remote_addr;
  uint32 proc_id, port_id, pkt_len;
  int result;

  memset (rx_pkt, 0, sizeof(*rx_pkt));
  sock_res = tftp_socket_recvfrom (&socket_handle, rx_pkt->raw_pkt_buff,
                                   sizeof (rx_pkt->raw_pkt_buff),
                                   &remote_addr, 0, &rx_data_size);
  TFTP_DEBUG_ASSERT (sock_res == TFTP_SOCKET_RESULT_SUCCESS);
  TFTP_DEBUG_ASSERT (rx_data_size > 0);
  if ((sock_res != TFTP_SOCKET_RESULT_SUCCESS) ||
      (rx_data_size <= 0))
  {
    TFTP_LOG_ERR ("Recvfrom failed for server main thread"
                   "sock res = %d, rx_data_size = %u. errno = %d\n",
                   sock_res, rx_data_size,
                   tftp_socket_get_last_errno (&socket_handle));
    return;
  }

  rx_pkt->raw_pkt_buff_data_size = rx_data_size;

  TFTP_LOG_RAW_TFTP_BUF (rx_pkt->raw_pkt_buff, rx_data_size,
                         "Revdfrom sucess: got %d bytes", rx_data_size);

  /* Interpret the incoming TFTP-packet. */
  result = tftp_pkt_interpret (rx_pkt);
  TFTP_ASSERT (result == 0 && rx_pkt->pkt_processed == 1);
  if ((result != 0) ||
      (rx_pkt->pkt_processed != 1))
  {
    TFTP_LOG_ERR ("Main thread: Recd pkt failed to interpret\n");
    return;
  }

  if (rx_pkt->opcode != TFTP_PKT_OPCODE_TYPE_RRQ &&
      rx_pkt->opcode != TFTP_PKT_OPCODE_TYPE_WRQ)
  {
    TFTP_LOG_ERR ("%d : Not an RRQ or WRQ : Dropping pkt", rx_pkt->opcode);
    return;
  }

  client_info = tftp_server_get_client_instance ();
  if (client_info == NULL)
  {
    TFTP_LOG_ERR ("Max TFTP client limit reached. Dropping pkt");
    return;
  }

  sock_res = tftp_socket_get_addr_by_port (&socket_handle, &remote_addr,
                                            &proc_id, &port_id);
  TFTP_ASSERT(sock_res == TFTP_SOCKET_RESULT_SUCCESS);

  if (sock_res != TFTP_SOCKET_RESULT_SUCCESS)
  {
    tftp_server_free_client_instance (client_info);
    return;
  }
  client_info->remote_addr.addr_type = TFTP_CONNECTION_IPCR_ADDR_TYPE_PORT;
  client_info->remote_addr.u.port_addr.proc_id = proc_id;
  client_info->remote_addr.u.port_addr.port_id = port_id;

  client_info->server_instance = instance;

  pkt_len = sizeof (client_info->proto.rx_pkt.raw_pkt_buff);

  tftp_memscpy (&client_info->proto.rx_pkt.raw_pkt_buff, pkt_len,
                rx_pkt->raw_pkt_buff, pkt_len);

  client_info->proto.rx_pkt.raw_pkt_buff_data_size =
         rx_pkt->raw_pkt_buff_data_size;

  result = tftp_thread_create (&client_info->client_threads_hdl,
                               tftp_client_thread_func, client_info);
  TFTP_ASSERT (result == 0);
  if (result != 0)
  {
    tftp_server_free_client_instance (client_info);
  }
}

int
tftp_server (int argc, const char *argv[])
{
  struct tftp_server_info_type *server_info = NULL;
  struct tftp_server_config_info_type *config;
  enum tftp_server_instance_id_type inst;
  enum tftp_socket_result_type sock_res;
  tftp_socket_pollfd *pollfd_array;
  struct tftp_pkt_type *rx_pkt;
  int result, poll_res;
  int32 init_res;

  (void) argc;
  (void) argv;

  /* No real os ops in this function..*/
  tftp_log_init ();

  /* Should be one of  the first call to setup the required perms. */
  tftp_os_wakelocks_init();
  tftp_os_init ();

  tftp_utils_init ();
  tftp_connection_init ();

  tftp_server_init ();

  init_res = tftp_server_folders_init ();
  if (init_res != 0)
  {
    TFTP_LOG_ERR ("Server folder init failed! error = %d.\n", init_res);
    TFTP_LOG_ERR ("Attempting to continue.... some paths maybe inaccessible.");
  }

  tftp_server_create_info_file ();

  server_info = &tftp_server_inst;
  pollfd_array = server_info->pollfd_array;
  rx_pkt = &server_info->rx_pkt;

  /* Open the connections for the TFTP-Server to listen in. */
  result = tftp_server_open_sockets (server_info);
  TFTP_ASSERT (result == 0);
  if (result != 0)
  {
    TFTP_LOG_ERR ("Sockets open failed for server main thread.\n");
    TFTP_SERVER_EXIT (-1);
  }

  while (!server_info->stop_server)
  {
    /* Wait forever for a TFTP-packet from a client. */
    TFTP_LOG_DBG ("Server waiting for client-connection.");

    poll_res = tftp_socket_poll(pollfd_array, TFTP_SERVER_INSTANCE_ID_MAX, -1);
    TFTP_DEBUG_ASSERT (poll_res >= 0);
    if (poll_res < 0)
    {
      TFTP_LOG_ERR ("Poll failed with errno %d string %s.\n", -poll_res,
                    strerror (-poll_res));
      continue;
    }

    for (inst = TFTP_SERVER_INSTANCE_ID_INVALID;
         inst < TFTP_SERVER_INSTANCE_ID_MAX; inst++)
    {
      config = &server_info->config[inst];
      if (config->is_valid != 1)
      {
        continue;
      }

      if (pollfd_array[inst].revents == 0)
      {
        continue;
      }

      sock_res = tftp_socket_interpret_poll_result (&pollfd_array[inst],
                                                    poll_res);
      if (sock_res == TFTP_SOCKET_RESULT_SUCCESS)
      {
        tftp_server_process_pkt (config->socket_handle, rx_pkt, inst);
      }
      else
      {
        TFTP_LOG_ERR ("Unexpected poll result %d\n", sock_res);
      }
    }
  }

  TFTP_LOG_ERR ("Main Server thread exiting");


  return 0;
}

static int32
tftp_server_attempt_autodir(const char *fname, uint16_t mode, int32_t gid)
{
  int32_t ret;
  ret = tftp_os_auto_mkdir (fname, mode, gid);

  if (ret != 0)
  {
    /* Try creating the data paths.
    If the autodir is in tombstones this should never fail. */
    TFTP_LOG_ERR("Retry auto dir after attempting to create data dirs");

    /* Try to create folders in the data partition */
    (void) tftp_server_folders_init();

    /* Attempt to auto-dir again. This will work assuming
    the data partition for at least this subsystem was
    created successfully. */
    ret = tftp_os_auto_mkdir (fname, mode, gid);
  }
  return ret;
}

static int32
tftp_server_file_open_helper (struct tftp_server_client_info_type *client_info)
{
  const char *path, *file_path;
  char *temp_file_path;
  struct tftp_protocol_info *proto;
  tftp_file_handle *handle;
  uint32 temp_file_path_buf_size;

  char *file_mode = "bad_mode";
  int32 mkdir_res, fd, open_res = 0;
  uint16 dir_mode_bits = TFTP_DEFAULT_DIR_MODE;
  uint16 file_mode_bits = TFTP_DEFAULT_FILE_MODE;
  int32 gid = -1;
  int is_shared_file = 0;
  int32 oflags = O_RDONLY;

  proto = &client_info->proto;
  file_path = client_info->file_name;
  temp_file_path = client_info->temp_file_name;
  temp_file_path_buf_size = sizeof(client_info->temp_file_name);

  handle = &client_info->fd;
  path = file_path;

  TFTP_ASSERT ((proto->type == TFTP_REQUEST_TYPE_WRQ) ||
               (proto->type == TFTP_REQUEST_TYPE_RRQ));

  if (proto->type == TFTP_REQUEST_TYPE_RRQ)
  {
    oflags = O_RDONLY;
    file_mode = "rb";
  }
  else if (proto->type == TFTP_REQUEST_TYPE_WRQ)
  {
    if (proto->do_append == 1)
    {
      file_mode = "ab";
      oflags = O_CREAT | O_WRONLY | O_APPEND;
    }
    else if (proto->is_seek_offset_valid == 1)
    {
      file_mode = "rb+";
      oflags = O_CREAT | O_RDWR;
    }
    else
    {
      file_mode = "wb";
      oflags = O_CREAT | O_WRONLY | O_TRUNC;

      snprintf (temp_file_path, temp_file_path_buf_size, "%s%s", file_path,
                TFTP_SERVER_TEMP_FILE_EXTENSION);

      path = temp_file_path;
      tftp_os_unlink (temp_file_path);
      client_info->is_atomic_put = 1;
      TFTP_LOG_DBG ("Using temp file[%s] for atomic put", temp_file_path);
    }
  }

  is_shared_file = tftp_server_folders_check_if_shared_file (path);

  if (is_shared_file == 1)
  {
    file_mode_bits = TFTP_SHARED_FILE_MODE;
    dir_mode_bits  = TFTP_SHARED_DIR_MODE;
    gid            = TFTP_SHARED_GID;
  }

  open_res = tftp_os_open_file (path, oflags, file_mode_bits, gid);
  if (open_res < 0)
  {
    if (proto->type == TFTP_REQUEST_TYPE_RRQ)
    {
      return open_res;
    }
    else
    {
      if (open_res != -ENOENT)
      {
        return open_res;
      }
      mkdir_res = tftp_server_attempt_autodir (path, dir_mode_bits, gid);

      if (mkdir_res == 0)
      {
        open_res = tftp_os_open_file (path, oflags, file_mode_bits, gid);
        if (open_res < 0)
        {
          if (open_res != -ENOENT)
          {
            return open_res;
          }

          /* Handle case where the file is being written directly
             into the symlink target dir without sub dirs and the
             target dir pointed to by symlink doesn't exist.*/
          TFTP_LOG_ERR("Try creating data dirs.");
          /* Try to create folders in the data partition */
          (void) tftp_server_folders_init();

          /* Attempt reopening the file */
          open_res = tftp_os_open_file (path, oflags, file_mode_bits, gid);
          if (open_res < 0)
          {
            return open_res;
          }
        }
      }
      else
      {
        return mkdir_res; //TODO: maybe return open_res??
      }
    }
  }

  /* If we are here then the file was successfully opened. Now get a file
     stream from the fd  */
  fd = open_res;
  open_res = tftp_file_open_stream (handle, fd, file_mode);
  if (open_res != 0)
  {
    return open_res;
  }

  if (proto->seek_offset > 0)
  {
    open_res = tftp_file_seek (*handle, proto->seek_offset, SEEK_SET);
    if (open_res != 0)
    {
      tftp_file_close (*handle);
    }
  }

  return open_res;
}


static void
tftp_server_set_option_value (uint32 *option, uint32 max_value,
                               uint32 new_value)
{
  if (*option < new_value)
  {
    *option = new_value;
  }

  if (*option > max_value)
  {
    *option = max_value;
  }
}

static void
tftp_server_init_state (struct tftp_protocol_info *proto,
                           enum tftp_protocol_request_type req_type)
{
  proto->type = req_type;

  proto->block_size = TFTP_SERVER_DEFAULT_DATA_BLOCK_SIZE;
  proto->timeout_in_ms = TFTP_SERVER_DEFAULT_TIMEOUT_IN_MS;
  proto->window_size = TFTP_SERVER_DEFAULT_WINDOW_SIZE;
  proto->seek_offset = 0;
  proto->do_append = 0;
  proto->max_pkt_retry_count = TFTP_MAX_PKT_RETRY_COUNT;
  proto->max_wrong_pkt_count = TFTP_MAX_WRONG_PKT_COUNT;

  /* Add the additional time-out for last ack.*/
  proto->last_ack_timeout_ms = proto->timeout_in_ms +
                            TFTP_SERVER_LAST_ACK_ADDITIONAL_TIMEOUT_MS;
}

static int32
tftp_server_parse_req_options (
      struct tftp_server_client_info_type *client_info)
{
  int32 result = 0;
  uint32 option_val, option_idx;
  struct tftp_protocol_info *proto;
  struct tftp_pkt_req_pkt_fields *req_pkt;
  struct tftp_pkt_options_type *options;

  TFTP_ASSERT (client_info != NULL);
  if (client_info == NULL)
  {
    result = -1;
    goto End;
  }

  proto = &client_info->proto;

  TFTP_ASSERT ((proto->rx_pkt.opcode == TFTP_PKT_OPCODE_TYPE_RRQ) ||
               (proto->rx_pkt.opcode == TFTP_PKT_OPCODE_TYPE_WRQ));
  req_pkt = &proto->rx_pkt.pkt_fields.req_pkt;

  if (req_pkt->options.extended_options_count == 0)
  {
    result = 0;
    goto End;
  }

  if (req_pkt->options.are_all_options_valid != 1)
  {
    result = -1;
    goto End;
  }

  options = &proto->options;
  options->are_all_options_valid = 1;
  options->extended_options_count = req_pkt->options.extended_options_count;

  for (option_idx = 0;
       option_idx < req_pkt->options.extended_options_count;
      ++option_idx)
  {
    TFTP_ASSERT (req_pkt->options.options[option_idx].is_valid == 1);
    option_val = req_pkt->options.options[option_idx].option_value;

    switch (req_pkt->options.options[option_idx].option_id)
    {
      case TFTP_PKT_OPTION_ID_BLOCK_SIZE:
      {
        tftp_server_set_option_value (&proto->block_size,
                                       TFTP_MAX_DATA_BLOCK_SIZE,
                                       option_val);
        options->options[option_idx].option_id =
                                                TFTP_PKT_OPTION_ID_BLOCK_SIZE;
        options->options[option_idx].option_value = proto->block_size;
        break;
      }

      case TFTP_PKT_OPTION_ID_TSIZE:
      {
        if (proto->type == TFTP_REQUEST_TYPE_RRQ)
        {
          options->options[option_idx].option_value = client_info->file_size;
        }
        else
        {
          options->options[option_idx].option_value = proto->tsize;
        }
        options->options[option_idx].option_id = TFTP_PKT_OPTION_ID_TSIZE;
        break;
      }

      case TFTP_PKT_OPTION_ID_SEEK:
      {
        proto->is_seek_offset_valid = 1;
        proto->seek_offset = option_val;
        options->options[option_idx].option_id = TFTP_PKT_OPTION_ID_SEEK;
        options->options[option_idx].option_value = proto->seek_offset;
        break;
      }

      case TFTP_PKT_OPTION_ID_APPEND:
      {
        proto->do_append = 1;
        options->options[option_idx].option_id = TFTP_PKT_OPTION_ID_APPEND;
        options->options[option_idx].option_value = 0;
        break;
      }

      case TFTP_PKT_OPTION_ID_UNLINK:
      {
        proto->do_unlink = 1;
        options->options[option_idx].option_id = TFTP_PKT_OPTION_ID_UNLINK;
        options->options[option_idx].option_value = 0;
        break;
      }

      case TFTP_PKT_OPTION_ID_TIMEOUT_IN_SECS:
      {
        option_val *= 1000; /* Convert to milli seconds. */
        tftp_server_set_option_value (&proto->timeout_in_ms,
                                       TFTP_MAX_TIMEOUT_IN_MS,
                                       option_val);
        proto->current_timeout_ms = proto->timeout_in_ms;
        options->options[option_idx].option_id =
                                           TFTP_PKT_OPTION_ID_TIMEOUT_IN_SECS;
        options->options[option_idx].option_value =proto->timeout_in_ms / 1000;
        break;
      }

      case TFTP_PKT_OPTION_ID_TIMEOUT_IN_MS:
      {
        tftp_server_set_option_value (&proto->timeout_in_ms,
                                       TFTP_MAX_TIMEOUT_IN_MS,
                                       option_val);
        proto->current_timeout_ms = proto->timeout_in_ms;
        options->options[option_idx].option_id =
                                             TFTP_PKT_OPTION_ID_TIMEOUT_IN_MS;
        options->options[option_idx].option_value = proto->timeout_in_ms;
        break;
      }

      case TFTP_PKT_OPTION_ID_WINDOW_SIZE:
      {
        if (option_val == 0)
        {
          proto->window_size = option_val;
        }
        else
        {
          tftp_server_set_option_value (&proto->window_size,
                                         TFTP_MAX_WINDOW_SIZE,
                                         option_val);
        }
        options->options[option_idx].option_id =
                                           TFTP_PKT_OPTION_ID_WINDOW_SIZE;
        options->options[option_idx].option_value = proto->window_size;
        break;
      }

      default:
      {
        result = -1;
        TFTP_LOG_ERR ("Invalid OACK id = %d, val = %d",
                       options->options[option_idx].option_id,
                       option_val);
        break;
      }
    }

    if (result != 0)
    {
      break;
    }
  }

  /* Add the additional time-out for last ack.*/
  proto->last_ack_timeout_ms = proto->timeout_in_ms +
                            TFTP_SERVER_LAST_ACK_ADDITIONAL_TIMEOUT_MS;

End:
  return result;
}


int32
tftp_server_send_data_cb (void *param, uint16 block_number, uint8 **data_buf)
{
  struct tftp_server_client_info_type *info;
  int32 result, seek_posn;
  uint32 read_size;
  uint16 last_block_number;

  info = (struct tftp_server_client_info_type *)param;
  TFTP_ASSERT (info != NULL);
  if (info == NULL)
  {
    result = -EINVAL;
    goto End;
  }

  *data_buf = info->data_buffer;

  seek_posn = info->proto.seek_offset;
  seek_posn += (block_number - 1) * info->proto.block_size;

  result = tftp_file_seek (info->fd, seek_posn, SEEK_SET);
  if (result != 0)
  {
    goto End;
  }

  read_size = info->proto.block_size;

  if (info->proto.tsize > 0)
  {
    last_block_number = ((info->proto.tsize - 1) / info->proto.block_size) + 1;
    if (block_number < last_block_number)
    {
      read_size = info->proto.block_size;
    }
    else if (block_number == last_block_number)
    {
      if (info->proto.tsize % info->proto.block_size == 0)
      {
        read_size = info->proto.block_size;
      }
      else
      {
        read_size = info->proto.tsize % info->proto.block_size;
      }
    }
    else if (block_number == (last_block_number + 1))
    {
      read_size = 0;
    }
    else
    {
      /* We should not get callback for more than TSIZE. */
      TFTP_ASSERT (0);
    }
   }

  result = tftp_file_read (info->fd, info->data_buffer, read_size);

End:
  return result;
}



static int32
tftp_thread_rrq_req_handler (struct tftp_server_client_info_type *info)
{
  int32 ret_val = 0;
  uint32 sent_data_size = 0;
  enum tftp_protocol_result proto_res;
  struct tftp_protocol_info *proto;

  TFTP_ASSERT (info != NULL);
  if (info == NULL)
  {
    TFTP_LOG_ERR ("Invalid arguments: info is null");
    return -1;
  }

  proto = &info->proto;
  if (proto->options.extended_options_count > 0)
  {
    proto_res = tftp_protocol_server_send_oack_wait_for_ack (proto);
    if (proto_res != TFTP_PROTOCOL_SUCCESS)
    {
      goto End;
    }
  }

  proto_res = tftp_protocol_send_data (proto, NULL, 0, &sent_data_size,
                    tftp_server_send_data_cb, info);

End:
  if (proto_res == TFTP_PROTOCOL_EOT)
  {
    proto_res = TFTP_PROTOCOL_SUCCESS;
  }

  if (proto_res == TFTP_PROTOCOL_SUCCESS)
  {
    ret_val = 0;
  }
  else
  {
    ret_val = -1;
  }

  if (info->fd != NULL)
  {
    /* Flush the data. */
    tftp_file_close (info->fd);
    info->fd = NULL;
  }

  TFTP_LOG_DBG ("RRQ stats : sent_size = %u total-blocks = %d "
                "total-bytes = %d timedout-pkts = %d, wrong-pkts = %d",
                sent_data_size,
                proto->debug_info.total_blocks,
                proto->debug_info.total_bytes,
                proto->debug_info.total_timeouts,
                proto->debug_info.total_wrong_pkts);

  if (proto_res == TFTP_PROTOCOL_EOT)
  {
    TFTP_LOG_DBG ("Received an EOT");
  }
  else if (proto_res == TFTP_PROTOCOL_TIMEOUT)
  {
    TFTP_LOG_ERR ("Timeout when sending data for RRQ.");
  }
  else if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    TFTP_LOG_ERR ("RRQ send failed with an ERROR.");
  }
  else
  {
    TFTP_LOG_DBG ("RRQ was successfully processed.");
  }

  return ret_val;
}

int32
tftp_server_recv_data_cb (void *param, uint16 block_number,
                          uint8 *data_buf, uint32 data_buf_size)
{
  struct tftp_server_client_info_type *info;
  int32 result, seek_posn;

  info = (struct tftp_server_client_info_type *)param;
  TFTP_ASSERT (info != NULL);
  if (info == NULL)
  {
    result  = -EINVAL;
    goto End;
  }

  seek_posn = info->proto.seek_offset;
  seek_posn += (block_number - 1) * info->proto.block_size;

  result = tftp_file_seek (info->fd, seek_posn, SEEK_SET);
  if (result != 0)
  {
    goto End;
  }

  result = tftp_file_write (info->fd, data_buf, data_buf_size);

End:
  return result;
}

int32
tftp_server_recv_data_close_cb (void *param, uint16 block_number,
                           uint8 *data_buf, uint32 data_buf_size)
{
  int32 ret_val = 0;
  struct tftp_server_client_info_type *info;
  (void)block_number;
  (void)data_buf;
  (void)data_buf_size;

  info = (struct tftp_server_client_info_type *)param;
  TFTP_ASSERT (info != NULL);
  if (info == NULL)
  {
    return -EINVAL;
  }

  if (info->fd != NULL)
  {
    /* Flush the data. */
    (void) tftp_file_close (info->fd);
    info->fd = NULL;
  }

  /* On success rename the file to ensure either old file or new file data
       exists and never an in between state. */
  if (info->is_atomic_put)
  {
    int32 result;
    result = tftp_os_rename (info->temp_file_name, info->file_name);
    if (result != 0)
    {
      ret_val = result;
      TFTP_LOG_DBG ("Rename failed for atomic put error=%d[%s], [%s to %s]",
                     -result,strerror(-result), info->temp_file_name,
                     info->file_name);
    }
  }

  return ret_val;
}


static int32
tftp_thread_wrq_req_handler (struct tftp_server_client_info_type *info)
{
  int32 ret_val = 0;
  uint32 recd_data_size = 0;
  enum tftp_protocol_result proto_res;
  struct tftp_protocol_info *proto;

  proto = &(info->proto);
  TFTP_ASSERT (proto->rx_pkt.opcode == TFTP_PKT_OPCODE_TYPE_WRQ);

  if (proto->options.extended_options_count > 0)
  {
    proto_res = tftp_protocol_server_send_oack_wait_for_data (proto);
  }
  else
  {
    proto_res = tftp_protocol_send_ack_wait_for_data (proto);
  }
  if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    goto End;
  }

  recd_data_size = 0;
  proto_res = tftp_protocol_recv_data (proto, NULL, 0, &recd_data_size,
                                        tftp_server_recv_data_cb,
                                        tftp_server_recv_data_close_cb, info);

End:
  if (proto_res == TFTP_PROTOCOL_EOT)
  {
    proto_res = TFTP_PROTOCOL_SUCCESS;
  }

  if (proto_res == TFTP_PROTOCOL_SUCCESS)
  {
    ret_val = 0;
  }
  else
  {
    ret_val = -1;
  }

  if (info->fd != NULL)
  {
    /* Flush the data. */
    tftp_file_close (info->fd);
    info->fd = NULL;
  }

  TFTP_LOG_DBG ("WRQ stats : total-blocks = %d : total-bytes = %d : %d"
                " timedout-pkts = %d, wrong-pkts = %d",
                proto->debug_info.total_blocks,
                proto->debug_info.total_bytes, recd_data_size,
                proto->debug_info.total_timeouts,
                proto->debug_info.total_wrong_pkts);

  if (proto_res == TFTP_PROTOCOL_EOT)
  {
    TFTP_LOG_DBG ("Received an EOT");
  }
  else if (proto_res == TFTP_PROTOCOL_TIMEOUT)
  {
    TFTP_LOG_ERR ("Timeout when sending data for WRQ.");
  }
  else if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    TFTP_LOG_ERR ("WRQ send failed with an ERROR.");
  }
  else
  {
    TFTP_LOG_DBG ("WRQ was successfully processed.");
  }

  return ret_val;
}

static void
tftp_thread_unlink_handler(struct tftp_server_client_info_type *info)
{
  struct tftp_protocol_info *proto;
  enum tftp_connection_result connection_open_res;
  enum tftp_pkt_error_code error_code;
  char *error_msg;
  int32 unlink_res;

  proto = &(info->proto);
  if (proto->rx_pkt.opcode != TFTP_PKT_OPCODE_TYPE_WRQ)
  {
    error_msg = "Unlink option used in an RRQ";
    error_code = TFTP_PKT_ERROR_CODE_ILLEGAL_FTP_OPERATION;

    TFTP_LOG_ERR ("Unlink option used in an RRQ sending error pkt.");
    (void) tftp_protocol_send_error_packet (&info->proto,
                                            error_code, error_msg);
    return;
  }

  connection_open_res =
              tftp_connection_set_timeout (&info->proto.connection_hdl,
                                           proto->timeout_in_ms);
  TFTP_ASSERT (connection_open_res == TFTP_CONNECTION_RESULT_SUCCESS);
  if (connection_open_res != TFTP_CONNECTION_RESULT_SUCCESS)
  {
    error_code = TFTP_PKT_ERROR_CODE_NOT_DEFINED ;
    error_msg = "Timeout could not be set for unlink";

    TFTP_LOG_ERR ("Set new timeout failed during unlink. timeout = %u\n",
                   proto->timeout_in_ms);

    (void) tftp_protocol_send_error_packet (&info->proto,
                                            error_code, error_msg);
    return;
  }

  unlink_res = tftp_os_unlink (info->file_name);
  if (unlink_res < 0)
  {
    int32 error_val = -unlink_res;

    tftp_protocol_send_errno_as_error_packet (proto, error_val);
    return;
  }

  (void) tftp_protocol_server_send_oack_no_wait(&info->proto);
}

static void
tftp_thread_req_handler (struct tftp_server_client_info_type *client_info)
{
  int32 result;
  enum tftp_pkt_error_code error_code = TFTP_PKT_ERROR_CODE_NOT_DEFINED;
  char *req_name_str = "BAD REQUEST";
  char *error_msg = NULL;
  int32 open_res, option_parse_result;
  enum tftp_connection_result connection_open_res;
  enum tftp_protocol_result proto_res;
  enum tftp_protocol_request_type req_type = TFTP_REQUEST_TYPE_INVALID;
  struct tftp_protocol_info *proto = NULL;
  struct tftp_server_debug_info_type *debug_info = NULL;

  TFTP_ASSERT (client_info != NULL);
  if (client_info == NULL)
  {
    TFTP_LOG_ERR ("Thread req handler received null client info. \n");
    return;
  }

   switch (client_info->proto.rx_pkt.opcode)
   {
    case TFTP_PKT_OPCODE_TYPE_RRQ:
      req_type = TFTP_REQUEST_TYPE_RRQ;
      req_name_str = "RRQ";
      break;

    case TFTP_PKT_OPCODE_TYPE_WRQ:
      req_type = TFTP_REQUEST_TYPE_WRQ;
      req_name_str = "WRQ";
      break;

    default:
      TFTP_LOG_ERR ("Request type is unknown.");
      TFTP_ASSERT (0);
      break;
   }

  if (req_type == TFTP_REQUEST_TYPE_INVALID)
  {
    return;
  }

  proto = &client_info->proto;
  debug_info = &client_info->debug_info;

  debug_info->total_start_time_tick = tftp_timetick_get ();

  tftp_server_init_state (proto, req_type);

  /* Open the connection with client.  This should be done before we can send
     an response or error packets. */
  tftp_memscpy (&proto->remote_addr, sizeof (proto->remote_addr),
                &client_info->remote_addr, sizeof (proto->remote_addr));

  proto_res = tftp_protocol_open_connection (proto);
  TFTP_ASSERT (proto_res == TFTP_PROTOCOL_SUCCESS);
  if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    //TODO: Change the error log to something better? sid and inst id are
    // not really important.
    TFTP_LOG_ERR ("Connection open failed: sid = %d id = %d timeout(ms) = %d",
                  proto->remote_addr.u.name_addr.service_id,
                  proto->remote_addr.u.name_addr.instance_id,
                  proto->timeout_in_ms);
    return;
  }

  proto_res = tftp_protocol_connect_remote_addr (proto);
  if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    TFTP_LOG_ERR ("Connect_remote failed: proc = %d port = %d timeout(ms)= %d",
                  proto->remote_addr.u.port_addr.proc_id,
                  proto->remote_addr.u.port_addr.port_id,
                  proto->timeout_in_ms);
    goto End;
  }

  result = tftp_get_real_filepath (client_info);
  if (result != 0)
  {
    TFTP_LOG_ERR ("File access denied sending Access Violation.");
    error_code = TFTP_PKT_ERROR_CODE_ACCESS_VIOLATION;
    error_msg = strerror (-result);
    goto Error;
  }

  client_info->file_size = tftp_file_get_file_size (client_info->file_name);

  option_parse_result = tftp_server_parse_req_options (client_info);
  if (option_parse_result != 0)
  {
    error_code = TFTP_PKT_ERROR_CODE_INVALID_OPTIONS;
    error_msg = "Invalid options in the request";
    goto Error;
  }

  if(proto->do_unlink == 1)
  {
    tftp_thread_unlink_handler(client_info);
    goto End;
  }

  /* Open the file only after the options have been parsed. */
  open_res = tftp_server_file_open_helper (client_info);
  if (open_res < 0)
  {
    int32 error_val = -open_res;

    tftp_protocol_send_errno_as_error_packet (proto, error_val);
    goto End;
  }

  connection_open_res =
              tftp_connection_set_timeout (&client_info->proto.connection_hdl,
                                           proto->timeout_in_ms);
  TFTP_ASSERT (connection_open_res == TFTP_CONNECTION_RESULT_SUCCESS);
  if (connection_open_res != TFTP_CONNECTION_RESULT_SUCCESS)
  {
    TFTP_LOG_ERR ("Set new timeout failed for %s. timeout = %u\n",
                   req_name_str, proto->timeout_in_ms);
    error_code = TFTP_PKT_ERROR_CODE_NOT_DEFINED ;
    error_msg = "Timeout could not be set";
    goto Error;
  }

  /* Zero window is infinite and needs to be handled specially. */
  if (proto->window_size > 0)
  {
    client_info->data_buffer_size = proto->window_size * proto->block_size;
  }
  else
  {
    client_info->data_buffer_size = proto->block_size;
  }
  TFTP_ASSERT (client_info->data_buffer_size > 0);

  client_info->data_buffer = tftp_malloc (client_info->data_buffer_size);
  if (client_info->data_buffer == NULL)
  {
    error_code = TFTP_PKT_ERROR_CODE_NOT_DEFINED;
    error_msg = "Could not allocate data buffer";
    TFTP_LOG_DBG ("Malloc returned NULL");
    goto Error;
  }

  if (req_type == TFTP_REQUEST_TYPE_RRQ)
  {
    result = tftp_thread_rrq_req_handler (client_info);
  }
  else
  {
    result = tftp_thread_wrq_req_handler (client_info);
  }

  goto End;

Error:
  (void) tftp_protocol_send_error_packet (&client_info->proto,
                                           error_code, error_msg);

End:

  if (client_info->data_buffer != NULL)
  {
    tftp_free (client_info->data_buffer);
  }

  if (debug_info != NULL)
  {
    debug_info->total_end_time_tick = tftp_timetick_get ();
    debug_info->total_time_delta = tftp_timetick_diff_in_usec (
                                      debug_info->total_start_time_tick,
                                      debug_info->total_end_time_tick);
    TFTP_LOG_DBG ("%s stats in us:", req_name_str);
    TFTP_LOG_DBG ("Total API = %u", debug_info->total_time_delta);
    TFTP_LOG_DBG ("Total-TX = %u",
                  proto->connection_hdl.debug_info.total_send_time_delta);
    TFTP_LOG_DBG ("Total-RX = %u",
                  proto->connection_hdl.debug_info.total_recv_time_delta);
    TFTP_LOG_DBG ("Max-TX = %u",
                  proto->connection_hdl.debug_info.max_send_time_delta);
    TFTP_LOG_DBG ("Max-RX = %u",
                  proto->connection_hdl.debug_info.max_recv_time_delta);
    TFTP_LOG_DBG ("Min-TX = %u",
                  proto->connection_hdl.debug_info.min_send_time_delta);
    TFTP_LOG_DBG ("Min-RX = %u",
                  proto->connection_hdl.debug_info.min_recv_time_delta);
  }

  proto_res = tftp_protocol_close_connection (&client_info->proto);
  TFTP_ASSERT (proto_res == TFTP_PROTOCOL_SUCCESS);

  if (client_info->fd != NULL)
  {
    tftp_file_close (client_info->fd);
    client_info->fd = NULL;
  }

  return;
}

static void
tftp_client_thread_main (struct tftp_server_client_info_type *client_info)
{
  int result;
  struct tftp_pkt_type *rx_pkt;

  TFTP_ASSERT (client_info != NULL);
  if (client_info == NULL)
  {
    TFTP_LOG_ERR ("tftp_client_thread_main got null client info. \n");
    return;
  }

  rx_pkt = &client_info->proto.rx_pkt;

  result = tftp_pkt_interpret (rx_pkt);
  TFTP_ASSERT (result == 0 && rx_pkt->pkt_processed == 1);
  if ((result != 0) ||
      (rx_pkt->pkt_processed != 1))
  {
    TFTP_LOG_ERR ("Received pkt failed to interpret\n");
    return;
  }

  TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_RRQ ||
               rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_WRQ);

  tftp_thread_req_handler (client_info);
}

static tftp_thread_return_type
tftp_client_thread_func (void *arg)
{
  struct tftp_server_client_info_type *client_info;

  client_info = (struct tftp_server_client_info_type *)arg;
  TFTP_ASSERT (client_info != NULL);
  if (client_info == NULL)
  {
    TFTP_LOG_ERR ("Invalid arguments: client_info is null : aborting!");
    goto End;
  }

  TFTP_ASSERT (client_info->start_magic ==
               TFTP_CLIENT_INSTANCE_START_MAGIC);

  TFTP_ASSERT (client_info->end_magic == TFTP_CLIENT_INSTANCE_END_MAGIC);

  tftp_client_thread_main (client_info);

  TFTP_ASSERT (client_info->start_magic ==
                     TFTP_CLIENT_INSTANCE_START_MAGIC);

  TFTP_ASSERT (client_info->end_magic == TFTP_CLIENT_INSTANCE_END_MAGIC);

  tftp_server_free_client_instance (client_info);

End:
  tftp_thread_return ();
}

