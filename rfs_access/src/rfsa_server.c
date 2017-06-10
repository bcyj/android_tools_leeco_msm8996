/*
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "rfsa_server.h"
#include "rfsa_vtl_server.h"

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


/****************************************************************************
* Defines                                                                   *
****************************************************************************/

#define RFSA_SERVER_CMD_QUEUE_SIZE	50

#define CLIENT_ID_ALL			0x0		/* Client ID check not in use */
#define RFSA_DL_DSP_CLIENT_ID		0x011013ec	/* Unique DSP client ID for dynamic loading */
#define RFSA_DL_MDM_CLIENT_ID		0x011013ed	/* Unique MODEM client ID for dynamic loading */

#define UIO_QCOM_VER	"1.0"
#define UIO_QCOM_DSP_NAME	"rfsa_dsp"
#define UIO_QCOM_MDM_NAME	"rfsa_mdm"

#define UIO_SYSFS_DIR_PREFIX  "uio"

#define UIO_DEV		"/dev/uio%u"
#define UIO_SYSFS_DIR	"/sys/class/uio"
#define UIO_NAME	"/sys/class/uio/uio%u/name"
#define UIO_VER		"/sys/class/uio/uio%u/version"
#define UIO_ADDR	"/sys/class/uio/uio%u/maps/map0/addr"
#define UIO_SIZE	"/sys/class/uio/uio%u/maps/map0/size"

/*FOR DEBUG FS*/
#define DEBUGFS_FILE           "/sys/kernel/debug/rmt_storage/rfsa" /* rfsa */
#define DEBUGFS_MAGIC_STRING   "1\n"

/* Wake locks */
#define WAKE_LOCK_FILE		"/sys/power/wake_lock"
#define WAKE_UNLOCK_FILE	"/sys/power/wake_unlock"
#define WAKE_LOCK_STRING	"rfs_access_lock"

/****************************************************************************
* Definitions                                                               *
****************************************************************************/

struct uio_mmap_info
{
	int32_t		uio_fd;
	uint32_t	uio_dev_num;
	uint8_t		*shared_mem;
	uint64_t	shared_mem_size;
	uint64_t	shared_mem_physical;
};

/**
* The client info structure
*/

struct rfsa_client_info
{
	uint32_t		client_id;       /* Client ID */
	char			client_name[10]; /* Client Name */
	struct uio_mmap_info	mmap_info;       /* The virtual mem map info */
	int			is_inited;
};

struct rfsa_wake_lock
{
	uint32_t         pending_req_count;
	int              lock_fd;
	int              unlock_fd;
	pthread_mutex_t  mutex;
	ssize_t          write_size;
	int              is_inited;
};

/****************************************************************************
* Variables                                                                 *
****************************************************************************/

/** name of server working thread */
static const char rfsa_server_my_thread_name[] = "RFSA_SERVER";

/** variables used to syncronize the threads */
static rfsa_lock_t rfsa_server_lock;
static rfsa_event_t rfsa_server_control_event;
static rfsa_event_t rfsa_server_work_event;
static rfsa_event_t rfsa_server_thread_end_event;

/** working thread */
static rfsa_thread_t rfsa_server_thread;

/** variables used for the queue of events */
static rfsa_list_t rfsa_server_free_cmd_q;
static rfsa_list_t rfsa_server_used_cmd_q;
static rfsa_server_work_item_t rfsa_server_cmds[RFSA_SERVER_CMD_QUEUE_SIZE];

static int stopped;
static int init_done;

static struct rfsa_wake_lock  wake_lock = {0};

/**
* Clients which are supported by the server
*/
struct rfsa_client_info client_info[] = {
	{
		.client_id = RFSA_DL_DSP_CLIENT_ID,
		.client_name = UIO_QCOM_DSP_NAME,
		.is_inited = 0,
	},

	{
		.client_id = RFSA_DL_MDM_CLIENT_ID,
		.client_name = UIO_QCOM_MDM_NAME,
		.is_inited = 0,
	},
};

/**
* Number of defined clients
*/
uint32_t  no_clients = sizeof(client_info) / sizeof(struct rfsa_client_info);

/****************************************************************************
* Path look up table							    *
****************************************************************************/
#define PATH_TABLE_PREFIX_LEN		8
#define PATH_TABLE_PATH_LEN		64
#define PATH_TABLE_MAX_ITEMS		8

struct rfsa_path_table
{
	char		prefix[PATH_TABLE_PREFIX_LEN];
	char		path[PATH_TABLE_PATH_LEN];
	uint8_t		in_use;
	uint32_t	client_id;
};

static struct rfsa_path_table sys_path_tbl[PATH_TABLE_MAX_ITEMS] =
{
	{"ADSP://", "/system/rfs/msm/adsp/", 1, RFSA_DL_DSP_CLIENT_ID},
	{"MPSS://", "/system/rfs/msm/mpss/", 1, RFSA_DL_MDM_CLIENT_ID},
	{"", "", 0, 0}
};

static char *data_path_list[PATH_TABLE_PATH_LEN] =
{
	/* All dir paths must end in a '/'  for the directories to be
	   created. */

	"/data/rfs/msm/adsp/", /* MSM-ADSP Read write folder */
	"/data/rfs/msm/mpss/", /* MSM-MPSS Read write folder */

	"/data/rfs/shared/", /* Shared for all MSM+APQ+MDM */
	"",
};

/*****************************************************************************
* Forward declarations							     *
*****************************************************************************/
static int32_t rfsa_server_stat(rfsa_packet_t *packet_ptr);
static int32_t rfsa_server_create(rfsa_packet_t *packet_ptr);
static int32_t rfsa_server_read(rfsa_packet_t *packet_ptr);
static int32_t rfsa_server_get_buff_addr(rfsa_packet_t *packet_ptr);
static int32_t rfsa_server_release_buff_addr(rfsa_packet_t *packet_ptr);
static int32_t rfsa_server_iovec_write(rfsa_packet_t *packet_ptr);
static int32_t rfsa_server_iovec_read(rfsa_packet_t *packet_ptr);

/* For wake locks */
static void rfsa_wakelock(void);
static void rfsa_wakeunlock(void);

/*****************************************************************************
* Implementations                                                           *
****************************************************************************/

static int32_t os_mkdir (const char *path)
{
	int32_t result;

	/* This mode when used is modified by umask */
	result = mkdir (path, 0700);

	if (result != 0)
		result = -(errno);
	return result;
}

static int32_t os_autodir (const char *path)
{
	int result = 0;
	uint32_t index = 0;
	uint32_t path_len = 0;
	char *temp_path_buf;
	uint32_t temp_path_buf_size;

	if (path == NULL)
		return -EINVAL;

	path_len = strlen(path);

	if (path_len == 0)
		return -EINVAL;

	temp_path_buf_size = path_len + 1;
	temp_path_buf = malloc (temp_path_buf_size);
	if (temp_path_buf == NULL)
		return -ENOMEM;

	strlcpy (temp_path_buf, path, temp_path_buf_size);
	/* Set index to path_len - 1 as there is at least one char in the path.
	 * and then go backwards in the path and try to create the directories.
	 * We exit when the path runs out or we create one directory successfully or
	 * if an error is encountered.
	 */

	for (index = path_len - 1; index > 0; index--) {
		if (path[index] == '/') {
			temp_path_buf[index] = 0x00;
			/* Attempt to make subdirectories */
			result = os_mkdir (temp_path_buf);

			/* Account for broken symbolic links. */
			if (result == -EEXIST)
				result = 0;

			/* Exit if we are successful in creating a directory. */
			if (result == 0)
				break;
			/* Exit if we encounter an error other than ENOENT. ENOENT implies the
			 * parent directory of the directory we tried to create doesn't exist so
			 * we try to create that directory. Any other error means we must exit.
			 */
			else if (result != -ENOENT)
				break;
		}
	}

	if (result == 0) {
		/* If the current index was a '/' put it back. */
		if (path[index] == '/')
			temp_path_buf[index++] = '/';

		for (/* Current index value */; index < path_len; index++) {
			/* Go forward up the path until we reach the end */
			if (path[index] == '/') {
				/* Attempt to make subdirectories */
				result = os_mkdir(temp_path_buf);

				/* If we fail to create a directory and it doesn't exist we exit .*/
				if (result != 0) {
					if (result == -EEXIST)
						result = 0; /* The directory exists we can proceed */
					else
						break;
				}

				temp_path_buf[index] = '/';
			}
		}
	}

	if (result != 0)
		LOGE("Failed to create  dirs for(%s) errno = %d (%s)", path, result,
			strerror(-result));

	free (temp_path_buf);
	return result;
}

static int32_t rfsa_create_data_dirs (void)
{
	char **current_path;
	int32_t result = RFSA_EOK;

	current_path = data_path_list;

	while (**current_path != '\0') {
		result = os_autodir (*current_path);
		if (result != 0)
			break;

		*current_path++;
	}

	return result;
}

/**
* Checks if the packet received is valid
*/
int32_t rfsa_server_check_packet(uint32_t client_id, struct rfsa_client_info **client_ptr)
{
	uint32_t i;

	if(client_ptr == NULL)
		return RFSA_EFAILED;

	for (i = 0; i < no_clients; i++) {
		if (client_info[i].client_id == client_id) {
			if (client_info[i].is_inited == 1) {
				*client_ptr = &(client_info[i]);
				return RFSA_EOK;
			}
			else {
				LOGE("Received request for uninited client");
				return RFSA_EFAILED;
			}
		}
	}

	LOGE("Packet error: Client ID is not valid: 0x%x!\n", client_id);

	return RFSA_EFAILED;
}


/**
* Look up predefined file path
*/
static int32_t rfsa_path_lookup(char *name_i, int name_i_len, char *name_o,
				int name_o_len, uint32_t client_id)
{
	int32_t ret = RFSA_EOK;
	int i;
	int ent;
	int matched = 0;
	size_t len;

	// Match incoming name with table prefix entry
	for (ent = 0; ent < PATH_TABLE_MAX_ITEMS; ent++) {
		// If we reach the end of the table, end the search
		if (!sys_path_tbl[ent].in_use)
			break;
		// Match the prefix list, one char at a time
		for (i = 0; i < PATH_TABLE_PREFIX_LEN; i++) {
			// Match failed, of this search
			if (i > name_i_len)
				break;
			// End of search found?
			if (sys_path_tbl[ent].prefix[i] == '\0') {
				matched = 1;
				break;
			}
			if (sys_path_tbl[ent].prefix[i] != name_i[i] ||
					      name_i[i] == '\0')
				break;
		}
		if (matched)
			break;
	}
	if (!matched) {
		ret = RFSA_EFAILED;
		goto out;
	}

	// Check for client ID match
	if ((sys_path_tbl[ent].client_id != client_id) &&
	    (sys_path_tbl[ent].client_id != 0) && (client_id != 0)) {
		ret = RFSA_EFAILED;
		goto out;
	}

	// Prepend the prefix path
	len = strlcpy(name_o, sys_path_tbl[ent].path, name_o_len);
	strlcat(name_o, name_i + i, name_o_len - len);

	// Check to see if ".." or "~" string is used
	if ((strstr(name_o, "..") != NULL) ||
		(strstr(name_o, "~") != NULL)) {
		ret = RFSA_EFAILED;
		goto out;
	}

out:
	if (ret != RFSA_EOK)
		LOGE("rfsa_path_lookup failed for %s\n", name_i);

	return ret;
}

/**
* Callback function which processes every packet received
*/
int32_t rfsa_server_callback_impl(rfsa_packet_t *packet_ptr)
{
	int32_t ret = RFSA_EOK;

	if (packet_ptr == NULL) {
		LOGE("rfsa_server_callback_impl, packet_ptr = NULL");
		ret = RFSA_EFAILED;
		goto out;
	}

	switch (packet_ptr->opcode) {
	case QMI_RFSA_FILE_STAT_REQ_MSG_V01:
		ret = rfsa_server_stat(packet_ptr);
		break;
	case QMI_RFSA_FILE_CREATE_REQ_MSG_V01:
		ret = rfsa_server_create(packet_ptr);
		break;
	case QMI_RFSA_FILE_READ_REQ_MSG_V01:
		ret = rfsa_server_read(packet_ptr);
		break;
	case QMI_RFSA_GET_BUFF_ADDR_REQ_MSG_V01:
		ret = rfsa_server_get_buff_addr(packet_ptr);
		break;
	case QMI_RFSA_RELEASE_BUFF_ADDR_REQ_MSG_V01:
		ret = rfsa_server_release_buff_addr(packet_ptr);
		break;
	case QMI_RFSA_IOVEC_FILE_WRITE_REQ_MSG_V01:
		ret = rfsa_server_iovec_write(packet_ptr);
		break;
	case QMI_RFSA_IOVEC_FILE_READ_REQ_MSG_V01:
		ret = rfsa_server_iovec_read(packet_ptr);
		break;
	default:
		LOGE("Unknown opcode, opcode: %d\n", packet_ptr->opcode);
		ret = RFSA_EFAILED;
		break;
	}

out:
	return ret;
}

/**
* File stat implementation
*/
int32_t rfsa_server_stat(rfsa_packet_t *packet_ptr)
{
	int32_t ret = RFSA_EOK;
	int rc = -1;
	struct stat buf;
	char fname[RFSA_MAX_FILE_PATH_V01];
	rfsa_file_stat_req_msg_v01 *file_stat_ptr;
	rfsa_file_stat_resp_msg_v01 *file_stat_resp_ptr;

	file_stat_ptr = &packet_ptr->rfsa_req.file_stat_req;
	file_stat_resp_ptr = &packet_ptr->rfsa_ret.file_stat_ret;

	ret = rfsa_path_lookup(file_stat_ptr->filename, RFSA_MAX_FILE_PATH_V01 + 1,
				fname, sizeof(fname), CLIENT_ID_ALL);
	if (ret != RFSA_EOK)
		goto out;

	file_stat_resp_ptr->flags = 0;

	rc = stat(fname, &buf);
	if (rc == -1) {
		LOGE("File (%s) not found, err: %d\n", fname, errno);
		ret = RFSA_EFAILED;
		goto out;
	}

	LOGV("Stat file (%s), ret: %d, size: %jd\n", fname, ret, buf.st_size);
	file_stat_resp_ptr->size = buf.st_size;
	file_stat_resp_ptr->size_valid = 1;

	// Check for READ operation
	rc = open(fname, O_RDONLY);
	if (rc == -1) {
		LOGE("Failed to open file (%s) as read only, err: %d\n", fname, errno);
		ret = RFSA_EFAILED;
		goto out;
	}

	file_stat_resp_ptr->flags |= RFSA_ACCESS_FLAG_READ_V01;
	close(rc);

out:
	if (ret != RFSA_EOK) {
		file_stat_resp_ptr->resp.result = QMI_RESULT_FAILURE;
		file_stat_resp_ptr->resp.error = QMI_ERR_ACCESS_DENIED_V01;
	} else {
		file_stat_resp_ptr->resp.result = QMI_RESULT_SUCCESS;
		file_stat_resp_ptr->resp.error = QMI_ERR_NONE;
	}
	packet_ptr->data_ptr = &packet_ptr->rfsa_ret.file_stat_ret;
	packet_ptr->data_size = sizeof(rfsa_file_stat_resp_msg_v01);

	return ret;
}

static void rfsa_attempt_autodir(char *fname)
{
	int32_t ret;
	ret = os_autodir (fname);

	if (ret != 0) {
		/* Try creating the data paths.
		If the autodir is in tombstones this should never fail. */
		LOGE("Retry auto dir after attempting to create data dirs");

		/* Try to create folders in the data partition */
		(void) rfsa_create_data_dirs();

		/* Attempt to auto-dir again. This will work assuming
		the data partition for at least this subsystem was
		created successfully. */
		(void) os_autodir (fname);
	}
}

/**
* File create implementation
*/
int32_t rfsa_server_create(rfsa_packet_t *packet_ptr)
{
	int32_t ret = RFSA_EOK;
	int rc = -1;
	struct stat buf;
	char fname[RFSA_MAX_FILE_PATH_V01];
	rfsa_file_create_req_msg_v01 *file_create_ptr;
	rfsa_file_create_resp_msg_v01 *file_create_resp_ptr;
	int open_flags = O_WRONLY;

	file_create_ptr = &packet_ptr->rfsa_req.file_create_req;
	file_create_resp_ptr = &packet_ptr->rfsa_ret.file_create_ret;

	ret = rfsa_path_lookup(file_create_ptr->filename, RFSA_MAX_FILE_PATH_V01 + 1,
				fname, sizeof(fname), CLIENT_ID_ALL);

	if (ret != RFSA_EOK)
		goto out;

	LOGV("Create file (%s)\n", fname);
	// Check to see if the create mode is supported or not
	if (file_create_ptr->flags & RFSA_ACCESS_FLAG_CREATE_V01) {

		/* Set the open flag. */
		open_flags |= O_CREAT;

		/* Try to create the directories. */
		rfsa_attempt_autodir(fname);
	}

	if (file_create_ptr->flags & RFSA_ACCESS_FLAG_TRUNC_V01) {
		open_flags |= O_TRUNC;
	}

	rc = open(fname, open_flags, 0600);

	if ((rc == -1) && (errno == ENOENT) &&
	   (file_create_ptr->flags & RFSA_ACCESS_FLAG_CREATE_V01)) {

		/* Handle case where the file is being written directly
		   into the symlink target dir without sub dirs and the
		   target dir pointed to by symlink doesn't exist.*/
		LOGE("Try creating data dirs.");
		/* Try to create folders in the data partition */
		(void) rfsa_create_data_dirs();

		/* Attempt reopening the file */
		rc = open(fname, open_flags, 0600);
	}

	if (rc == -1) {
		LOGE("File (%s) open failed, flags: %d, err: %d\n", fname, open_flags, errno);
		ret = RFSA_EFAILED;
		goto out;
	}

	// Close the opened handle immediately
	close(rc);

out:
	if (ret != RFSA_EOK) {
		file_create_resp_ptr->resp.result = QMI_RESULT_FAILURE;
		file_create_resp_ptr->resp.error = QMI_ERR_ACCESS_DENIED_V01;
	} else {
		file_create_resp_ptr->resp.result = QMI_RESULT_SUCCESS;
		file_create_resp_ptr->resp.error = QMI_ERR_NONE;
	}
	packet_ptr->data_ptr = &packet_ptr->rfsa_ret.file_create_ret;
	packet_ptr->data_size = sizeof(rfsa_file_create_resp_msg_v01);

	return ret;
}

/**
* File read implementation
*/
int32_t rfsa_server_read(rfsa_packet_t *packet_ptr)
{
	int32_t ret = RFSA_EOK;
	int size_to_read;
	rfsa_file_read_req_msg_v01 *file_read_ptr;
	rfsa_file_read_resp_msg_v01 *file_read_resp_ptr;
	int32_t count;
	char fname[RFSA_MAX_FILE_PATH_V01];
	struct rfsa_client_info *client_ptr;
	int fp;

	file_read_ptr = &packet_ptr->rfsa_req.file_read_req;
	file_read_resp_ptr = &packet_ptr->rfsa_ret.file_read_ret;

	ret = rfsa_server_check_packet(file_read_ptr->client_id, &client_ptr);
	if (ret != RFSA_EOK)
		goto out;

	ret = rfsa_path_lookup(file_read_ptr->filename, RFSA_MAX_FILE_PATH_V01 + 1,
				fname, sizeof(fname), file_read_ptr->client_id);

	if (ret != RFSA_EOK)
		goto out;

	LOGV("Read file (%s), size: %d\n", fname, file_read_ptr->size);
	fp = open(fname, O_RDONLY);
	if (fp == -1) {
		LOGE("File (%s) open failed, flags: %d, err: %d\n", fname, O_RDONLY, errno);
		ret = RFSA_EFAILED;
		goto out;
	}

	// Check if seek operation is required
	if (file_read_ptr->size != 0) {
		if (lseek(fp, file_read_ptr->offset, SEEK_SET) == -1) {
			LOGE("File (%s) seek failed, offset: %d, err: %d\n",
				fname, file_read_ptr->offset, errno);
			ret = RFSA_EFAILED;
			goto cleanup;
		}
	}

	/* Cannot memset the shared ram as this is device memory and requires
	 * alligned access. The current memset implementation on some clib's cannot
	 * handle the requirement of alligned access.
	 */
	//memset(client_ptr->mmap_info.shared_mem, 0, client_ptr->mmap_info.shared_mem_size);

	if (file_read_ptr->size > client_ptr->mmap_info.shared_mem_size)
		size_to_read = client_ptr->mmap_info.shared_mem_size;
	else
		size_to_read = file_read_ptr->size;

	count = read(fp, client_ptr->mmap_info.shared_mem, size_to_read);
	if (count < 0) {
		LOGE("File (%s) read failed, count: %d, err: %d\n", fname, count, errno);
		ret = RFSA_EFAILED;
	}

cleanup:
	// Close the file at the end of the read
	close(fp);

out:
	if (ret != RFSA_EOK) {
		file_read_resp_ptr->data_valid = 0;
		file_read_resp_ptr->resp.result = QMI_RESULT_FAILURE;
		file_read_resp_ptr->resp.error = QMI_ERR_ACCESS_DENIED_V01;
	} else {
		file_read_resp_ptr->data.count = count;
		file_read_resp_ptr->data_valid = 1;
		file_read_resp_ptr->data.buffer = client_ptr->mmap_info.shared_mem_physical;
		file_read_resp_ptr->resp.result = QMI_RESULT_SUCCESS;
		file_read_resp_ptr->resp.error = QMI_ERR_NONE;
	}
	packet_ptr->data_ptr = &packet_ptr->rfsa_ret.file_read_ret;
	packet_ptr->data_size = sizeof(rfsa_file_read_resp_msg_v01);

	return ret;
}

/**
* Buffer allocation for IOVEC operation
*/
int32_t rfsa_server_get_buff_addr(rfsa_packet_t *packet_ptr)
{
	int32_t ret = RFSA_EOK;
	rfsa_get_buff_addr_req_msg_v01 *get_buff_addr_ptr;
	rfsa_get_buff_addr_resp_msg_v01 *get_buff_addr_resp_ptr;
	struct rfsa_client_info *client_ptr;

	get_buff_addr_ptr = &packet_ptr->rfsa_req.get_buff_addr_req;
	get_buff_addr_resp_ptr = &packet_ptr->rfsa_ret.get_buff_addr_ret;

	ret = rfsa_server_check_packet(get_buff_addr_ptr->client_id, &client_ptr);
	if (ret != RFSA_EOK)
		goto out;

	if (get_buff_addr_ptr->size > client_ptr->mmap_info.shared_mem_size) {
		LOGE("Requested memory size (%u) exceed allowed amount (%"PRIu64")\n",
			get_buff_addr_ptr->size, client_ptr->mmap_info.shared_mem_size);
		ret = RFSA_EFAILED;
		goto out;
	}

	// Look up the physical memory allocated for the buffer in question
	// and return the allocated address
	get_buff_addr_resp_ptr->address_valid = 1;
	get_buff_addr_resp_ptr->address = client_ptr->mmap_info.shared_mem_physical;

	LOGV("Server alloc buffer, id: %d buf addr: %x\n", get_buff_addr_ptr->client_id,
					   (int32_t)get_buff_addr_resp_ptr->address);

out:
	if (ret != RFSA_EOK) {
		get_buff_addr_resp_ptr->resp.result = QMI_RESULT_FAILURE;
		get_buff_addr_resp_ptr->resp.error = QMI_ERR_ACCESS_DENIED_V01;
	} else {
		get_buff_addr_resp_ptr->resp.result = QMI_RESULT_SUCCESS;
		get_buff_addr_resp_ptr->resp.error = QMI_ERR_NONE;
	}
	packet_ptr->data_ptr = &packet_ptr->rfsa_ret.get_buff_addr_ret;
	packet_ptr->data_size = sizeof(rfsa_get_buff_addr_resp_msg_v01);

	return ret;
}

/**
* Buffer de-allocation for IOVEC operation - dummy function
*/
int32_t rfsa_server_release_buff_addr(rfsa_packet_t *packet_ptr)
{
	int32_t ret = RFSA_EOK;
	rfsa_release_buff_addr_req_msg_v01 *free_buff_addr_ptr;
	rfsa_release_buff_addr_resp_msg_v01 *free_buff_addr_resp_ptr;
	struct rfsa_client_info *client_ptr;

	free_buff_addr_ptr = &packet_ptr->rfsa_req.free_buff_addr_req;
	free_buff_addr_resp_ptr = &packet_ptr->rfsa_ret.free_buff_addr_ret;

	ret = rfsa_server_check_packet(free_buff_addr_ptr->client_id, &client_ptr);
	if (ret != RFSA_EOK) {
		free_buff_addr_resp_ptr->resp.result = QMI_RESULT_FAILURE;
		free_buff_addr_resp_ptr->resp.error = QMI_ERR_ACCESS_DENIED_V01;
	} else {
		free_buff_addr_resp_ptr->resp.result = QMI_RESULT_SUCCESS;
		free_buff_addr_resp_ptr->resp.error = QMI_ERR_NONE;
	}
	packet_ptr->data_ptr = &packet_ptr->rfsa_ret.free_buff_addr_ret;
	packet_ptr->data_size = sizeof(rfsa_release_buff_addr_resp_msg_v01);

	return ret;
}

/**
* IOVEC write operation
*/
int32_t rfsa_server_iovec_write(rfsa_packet_t *packet_ptr)
{
	int32_t ret = RFSA_EOK;
	rfsa_iovec_file_write_req_msg_v01 *iovec_write_ptr;
	rfsa_iovec_file_write_resp_msg_v01 *iovec_write_resp_ptr;
	uint32_t count;
	uint32_t item;
	struct rfsa_client_info *client_ptr;
	int fp = -1;
	uint32_t buf_offset;
	uint32_t file_offset;
	uint32_t bytes;
	char fname[RFSA_MAX_FILE_PATH_V01];
	int open_flags = O_WRONLY;

	iovec_write_ptr = &packet_ptr->rfsa_req.iovec_write_req;
	iovec_write_resp_ptr = &packet_ptr->rfsa_ret.iovec_write_ret;

	ret = rfsa_server_check_packet(iovec_write_ptr->client_id, &client_ptr);
	if (ret != RFSA_EOK)
		goto out;

	ret = rfsa_path_lookup(iovec_write_ptr->filename, RFSA_MAX_FILE_PATH_V01 + 1,
				fname, sizeof(fname), iovec_write_ptr->client_id);

	if (ret != RFSA_EOK)
		goto out;

	LOGV("Server iovec write, file: %s Pieces: %d\n", fname, iovec_write_ptr->iovec_struct_len);

	// Loop write operation
	for (item = 0; item < iovec_write_ptr->iovec_struct_len; item ++) {
		// Check range of data
		buf_offset = iovec_write_ptr->iovec_struct[item].buff_addr_offset;
		file_offset = iovec_write_ptr->iovec_struct[item].file_offset;
		bytes = iovec_write_ptr->iovec_struct[item].size;

		LOGV("buf_offset=%u file_offset=%u bytes=%u", buf_offset, file_offset, bytes);
		// Check if the output data buffer is out of range
		if ((buf_offset + bytes) > client_ptr->mmap_info.shared_mem_size) {
			LOGE("Mem buffer out of range %d\n", buf_offset + bytes);
			ret = RFSA_EFAILED;
			goto out;
		}

		/* Check for append mode */
		if(file_offset == UINT32_MAX)
			open_flags = O_WRONLY | O_APPEND;
		else
			open_flags = O_WRONLY;

		fp = open(fname, open_flags);
		if (fp < 0) {
			LOGE("Write iovec File (%s) open failed, offset: %d, err: %d\n",
				fname, file_offset, errno);
			ret = RFSA_EFAILED;
			goto out;
		}

		if(open_flags == O_WRONLY) {
			// Perform seek
			if (lseek(fp, file_offset, SEEK_SET) == -1) {
				LOGE("Write iovec File (%s) seek failed, offset: %d, err: %d\n",
					fname, file_offset, errno);
				ret = RFSA_EFAILED;
				goto out;
			}
		}
		// Write the data block
		count = write(fp, client_ptr->mmap_info.shared_mem + buf_offset, bytes);

		// Check the amount of data written, error if not the same
		if (count != bytes) {
			LOGE("Write size mismatch, expect %d, got %d, fp=%d err: %d\n",
				bytes, count, fp, errno);
			ret = RFSA_EFAILED;
			goto out;
		}
		close(fp);
		fp = -1;
	}

out:
	if(fp >= 0)
		close(fp);

	if (ret != RFSA_EOK) {
		iovec_write_resp_ptr->resp.result = QMI_RESULT_FAILURE;
		iovec_write_resp_ptr->resp.error = QMI_ERR_ACCESS_DENIED_V01;
	} else {
		iovec_write_resp_ptr->resp.result = QMI_RESULT_SUCCESS;
		iovec_write_resp_ptr->resp.error = QMI_ERR_NONE;
	}

	packet_ptr->data_ptr = &packet_ptr->rfsa_ret.iovec_write_ret;
	packet_ptr->data_size = sizeof(rfsa_iovec_file_write_resp_msg_v01);

	return ret;
}

/**
* IOVEC read operation
*/
int32_t rfsa_server_iovec_read(rfsa_packet_t *packet_ptr)
{
	int32_t ret = RFSA_EOK;
	rfsa_iovec_file_read_req_msg_v01 *iovec_read_ptr;
	rfsa_iovec_file_read_resp_msg_v01 *iovec_read_resp_ptr;
	uint32_t count;
	uint32_t item;
	struct rfsa_client_info *client_ptr;
	int fp;
	uint32_t buf_offset;
	uint32_t file_offset;
	uint32_t bytes;
	char fname[RFSA_MAX_FILE_PATH_V01];

	iovec_read_ptr = &packet_ptr->rfsa_req.iovec_read_req;
	iovec_read_resp_ptr = &packet_ptr->rfsa_ret.iovec_read_ret;

	ret = rfsa_server_check_packet(iovec_read_ptr->client_id, &client_ptr);
	if (ret != RFSA_EOK)
		goto out;

	ret = rfsa_path_lookup(iovec_read_ptr->filename, RFSA_MAX_FILE_PATH_V01 + 1,
				fname, sizeof(fname), iovec_read_ptr->client_id);
	if (ret != RFSA_EOK)
		goto out;

	LOGV("Server iovec read, file: %s Pieces: %d\n", fname, iovec_read_ptr->iovec_struct_len);
	fp = open(fname, O_RDONLY);
	if (fp == -1) {
		LOGE("Failed to open file (%s) as read only, err: %d\n", fname, errno);
		ret = RFSA_EFAILED;
		goto out;
	}

	// Loop read operation
	for (item = 0; item < iovec_read_ptr->iovec_struct_len; item ++) {
		// Check range of data
		buf_offset = iovec_read_ptr->iovec_struct[item].buff_addr_offset;
		file_offset = iovec_read_ptr->iovec_struct[item].file_offset;
		bytes = iovec_read_ptr->iovec_struct[item].size;

		// Check if the output data buffer is out of range
		if ((buf_offset + bytes) > client_ptr->mmap_info.shared_mem_size) {
			LOGE("Mem buffer out of range\n");
			ret = RFSA_EFAILED;
			goto cleanup;
		}

		// Perform seek
		if (lseek(fp, file_offset, SEEK_SET) == -1) {
			LOGE("File (%s) seek failed, offset: %d, err: %d\n",
				fname, file_offset, errno);
			ret = RFSA_EFAILED;
			goto cleanup;
		}

		// Read the data block
		count = read(fp, client_ptr->mmap_info.shared_mem + buf_offset, bytes);

		// Check the amount of data written, error if not the same
		if (count != bytes) {
			LOGE("Read size mismatch, expect %d, got %d, err: %d\n",
				bytes, count, errno);
			ret = RFSA_EFAILED;
			goto cleanup;
		}
	}

cleanup:
	close(fp);
out:
	if (ret != RFSA_EOK) {
		iovec_read_resp_ptr->resp.result = QMI_RESULT_FAILURE;
		iovec_read_resp_ptr->resp.error = QMI_ERR_ACCESS_DENIED_V01;
	} else {
		iovec_read_resp_ptr->resp.result = QMI_RESULT_SUCCESS;
		iovec_read_resp_ptr->resp.error = QMI_ERR_NONE;
	}
	packet_ptr->data_ptr = &packet_ptr->rfsa_ret.iovec_read_ret;
	packet_ptr->data_size = sizeof(rfsa_iovec_file_read_resp_msg_v01);

	return ret;
}

static int string_to_uint64(uint64_t *output, char *string)
{
	char *end;
	unsigned long long value;


	value = strtoull(string, &end, 0);

	if (*end != 0)
		return -1;

	if ((errno == ERANGE && value == ULLONG_MAX) || value > UINT64_MAX)
		return -1;

	*output = (uint64_t)value;
	return 0;
}

static int get_uio_dev_num(uint32_t *dev_num, char *dir_name)
{
	int index, result;
	uint64_t output;
	index = strlen(UIO_SYSFS_DIR_PREFIX);

	result = string_to_uint64(&output, &dir_name[index]);
	if (result != 0)
		return -1;

	if(output > UINT32_MAX)
		return -1;

	*dev_num = (uint32_t)output;
	return 0;
}

static int read_file_to_buffer(char *file, char *buffer, uint32_t buffer_size)
{
	FILE *fd;
	char *fgets_ret;
	uint32_t str_len;

	fd = fopen(file, "r");
	if (!fd) {
		LOGE("Unable to open file %s\n", file);
		return -1;
	}
	fgets_ret = fgets(buffer, buffer_size, fd);
	fclose(fd);

	if (!fgets_ret) {
		LOGE("fgets failed with %d for %s\n", errno, file);
		return -1;
	}
	str_len = strlen(buffer);
	if(str_len > 0)
		buffer[str_len - 1] = 0;
	return 0;
}

static int check_uio_dev_version(uint32_t dev_num)
{
	int result = -1;
	char path_buffer[50], device_version[50];

	snprintf(path_buffer, sizeof(path_buffer), UIO_VER, dev_num);

	result = read_file_to_buffer(path_buffer, device_version,
		sizeof(device_version));

	if (result != 0)
		return -1;

	if(strcmp(device_version, UIO_QCOM_VER) != 0) {
		LOGE("Device version %s is not supported", device_version);
		return -1;
	}
	return 0;
}

static int check_uio_dev_name(char *client_name, uint32_t dev_num)
{
	int result = -1;
	char path_buffer[50], device_name[50];

	snprintf(path_buffer, sizeof(path_buffer), UIO_NAME, dev_num);

	result = read_file_to_buffer(path_buffer, device_name,
		sizeof(device_name));

	if (result != 0)
		return -1;

	if(strcasecmp(device_name, client_name) != 0)
		return -1;

	LOGV("Found dev for client %s", client_name);
	return 0;
}


static int do_mmap(char *dev_path, struct uio_mmap_info *mmap_info)
{
	uint8_t *virtual_address = NULL;

	mmap_info->uio_fd = open(dev_path, O_RDWR | O_DSYNC);
	if (mmap_info->uio_fd < 0) {
		LOGE("Unable to open %s\n", dev_path);
		return -1;
	}

	virtual_address = mmap(NULL, mmap_info->shared_mem_size,
				PROT_READ | PROT_WRITE, MAP_SHARED,
				mmap_info->uio_fd, 0);

	if (virtual_address == MAP_FAILED) {
		LOGE("\n mmap() failed for %s\n", dev_path);
		close(mmap_info->uio_fd);
		return -1;
	}
	mmap_info->shared_mem = virtual_address;
	return 0;
}

static int setup_client_mmap_buffer(struct uio_mmap_info *mmap_info)
{
	char path_buffer[50], value[50];
	uint64_t size, address;
	int result;
	uint32_t dev_num = mmap_info->uio_dev_num;

	snprintf(path_buffer, sizeof(path_buffer), UIO_SIZE, dev_num);

	result = read_file_to_buffer(path_buffer, value, sizeof(value));
	if (result != 0)
		return -1;

	result = string_to_uint64(&size, value);
	if (result != 0)
		return -1;

	snprintf(path_buffer, sizeof(path_buffer), UIO_ADDR, dev_num);

	result = read_file_to_buffer(path_buffer, value, sizeof(value));
	if (result != 0)
		return -1;

	result = string_to_uint64(&address, value);
	if (result != 0)
		return -1;

	mmap_info->shared_mem_size = size;
	mmap_info->shared_mem_physical = address;

	snprintf(path_buffer, sizeof(path_buffer), UIO_DEV, dev_num);

	result = do_mmap(path_buffer, mmap_info);
	if (result != 0)
		return -1;

	LOGV("\n Setup mmap buffers complete\n");
	return 0;
}

static int32_t uio_device_setup_shared_mem (char *client_name,
					struct uio_mmap_info *mmap_info)
{
	DIR *uiodir = NULL;
	struct dirent *uiodirent = NULL;
	int result, is_dev_found = 0;
	size_t uio_sysfs_dir_prefix_len;
	uint32_t dev_num;


	LOGV("Setting up shared mem for client %s", client_name);

	uiodir = opendir(UIO_SYSFS_DIR);
	if(!uiodir) {
		LOGE("open dir failed on %s, errno = %d\n", UIO_SYSFS_DIR,
			errno);
		return RFSA_EFAILED;
	}

	is_dev_found = 0;
	uio_sysfs_dir_prefix_len = strlen(UIO_SYSFS_DIR_PREFIX);

	/* Parse through every UIO device under /sys/class/uio */
	while((uiodirent = readdir(uiodir))) {
		if(strlen(uiodirent->d_name) <= uio_sysfs_dir_prefix_len)
			continue;

		result = strncasecmp(uiodirent->d_name, UIO_SYSFS_DIR_PREFIX,
					uio_sysfs_dir_prefix_len);

		if(result != 0)
			continue;

		result = get_uio_dev_num(&dev_num, uiodirent->d_name);
		if(result != 0)
			continue;

		result = check_uio_dev_name(client_name, dev_num);
		if(result != 0)
			continue;

		result = check_uio_dev_version(dev_num);
		if(result == 0) {
			mmap_info->uio_dev_num = dev_num;
			is_dev_found = 1;
			break;
		}
	}
	closedir(uiodir);

	if(is_dev_found != 1)
		return RFSA_EFAILED;

	/* we found the device so set it up */
	result = setup_client_mmap_buffer(mmap_info);
	if(result != 0)
		return RFSA_EFAILED;

	return RFSA_EOK;
}

static int32_t rfsa_server_mmap(struct rfsa_client_info *client_info)
{
	int32_t result = RFSA_EFAILED;
	struct uio_mmap_info *mmap_info = NULL;
	char *client_name = NULL;

	if(client_info == NULL)
		return RFSA_EFAILED;

	mmap_info = &(client_info->mmap_info);
	client_name = client_info->client_name;
	result = uio_device_setup_shared_mem(client_name, mmap_info);

	if(result == RFSA_EOK)
		client_info->is_inited = 1;

	return result;
}

static int32_t rfsa_server_init_all_clients(void)
{
	uint32_t i;
	int32_t result = RFSA_EFAILED;
	struct rfsa_client_info *curr_client = NULL;

	for (i = 0; i < no_clients; i++) {
		curr_client = &(client_info[i]);
		result = rfsa_server_mmap(curr_client);
		if(result != RFSA_EOK) {
			break;
		}
	}

	return result;
}

static void rfsa_server_isr_lock_fn(void)
{
	(void)rfsa_lock_enter(rfsa_server_lock);
}

static void rfsa_server_isr_unlock_fn(void)
{
	(void)rfsa_lock_leave(rfsa_server_lock);
}

int32_t rfsa_server_get_free_packet(rfsa_server_work_item_t **item)
{
	int32_t ret;

	ret = rfsa_list_remove_head(&rfsa_server_free_cmd_q, ((rfsa_list_node_t**)item));

	return ret;
}

void rfsa_server_add_to_queue(rfsa_server_work_item_t *item)
{
	int32_t res;

	LOGV("opcode: 0x%x\n", item->rfsa_packet.opcode);
	res = rfsa_list_add_tail(&rfsa_server_used_cmd_q, (rfsa_list_node_t *)&item->link);
	if (res == RFSA_EOK) {
		pthread_mutex_lock(&wake_lock.mutex);
		if(wake_lock.pending_req_count == 0)
			rfsa_wakelock();
		if(wake_lock.pending_req_count < UINT32_MAX)
			wake_lock.pending_req_count++;
		pthread_mutex_unlock(&wake_lock.mutex);
	}
	(void)rfsa_event_signal(rfsa_server_work_event);
}

static int32_t rfsa_server_worker_fn(void *param)
{
	int32_t ret;
	rfsa_server_work_item_t *item;

	(void)param;

	(void)rfsa_event_create(&rfsa_server_work_event);
	(void)rfsa_event_signal(rfsa_server_control_event);

	while (1) {
		ret = rfsa_event_wait(rfsa_server_work_event);

		if ((ret != RFSA_EOK) || stopped)
			break;

		while (!stopped) {
			ret = rfsa_list_remove_head(&rfsa_server_used_cmd_q, ((rfsa_list_node_t** )&item));
			if (ret != RFSA_EOK)
				break;

			LOGV("Process command item:0x%016" PRIXPTR, (uintptr_t)item);
			rfsa_server_callback_impl(&item->rfsa_packet);

			rfsa_vtl_server_response(item);
			(void)rfsa_list_add_tail(&rfsa_server_free_cmd_q, (rfsa_list_node_t* )&item->link);

			pthread_mutex_lock(&wake_lock.mutex);
			if(wake_lock.pending_req_count > 0)
				wake_lock.pending_req_count--;

			if(wake_lock.pending_req_count == 0)
				rfsa_wakeunlock();

			pthread_mutex_unlock(&wake_lock.mutex);
		}
	}

	(void)rfsa_event_destroy(rfsa_server_work_event);
	(void)rfsa_event_signal(rfsa_server_control_event);
	(void)rfsa_event_signal(rfsa_server_thread_end_event);

	return RFSA_EOK;
}

/**
* Initialize the server
*/
int32_t rfsa_server_init(void)
{
	int index;

	if (init_done) {
		LOGE("rfsa_server_init already called\n");
		return RFSA_EFAILED;
	}

	stopped = 0;

	(void)rfsa_lock_create(&rfsa_server_lock);
	(void)rfsa_event_create(&rfsa_server_control_event);

	(void)rfsa_event_create(&rfsa_server_thread_end_event);

	(void)rfsa_list_init(&rfsa_server_free_cmd_q, rfsa_server_isr_lock_fn,
		rfsa_server_isr_unlock_fn);

	for (index = 0; index < RFSA_SERVER_CMD_QUEUE_SIZE; ++index) {
		(void)rfsa_list_add_tail(&rfsa_server_free_cmd_q,
			(rfsa_list_node_t* )&rfsa_server_cmds[index].link);
	}

	(void)rfsa_list_init(&rfsa_server_used_cmd_q, rfsa_server_isr_lock_fn,
				rfsa_server_isr_unlock_fn);

	(void)rfsa_thread_create(&rfsa_server_thread, rfsa_server_my_thread_name,
				100, NULL, 0, rfsa_server_worker_fn, NULL);
	(void)rfsa_event_wait(rfsa_server_control_event);

	init_done = 1;
	LOGI("Initialized RFS server\n");
	return rfsa_vtl_server_init();
}

/**
* De-Initialize the server
*/
int32_t rfsa_server_deinit(void)
{
	int32_t ret;
	rfsa_server_work_item_t *item;

	if (!init_done) {
		LOGE("rfsa_server_deinit called ahead of rfsa_server_init\n");
		return RFSA_EFAILED;
	}

	/* signal the working thread to finish */
	stopped = 1;
	(void)rfsa_event_signal(rfsa_server_work_event);

	/* wait for working thread to finish */
	ret = rfsa_event_wait(rfsa_server_thread_end_event);
	if (ret != RFSA_EOK)
		LOGE("Error on rfsa_event_wait, ret: %d\n", ret);

	ret = rfsa_vtl_server_deinit();
	if (ret != RFSA_EOK)
		LOGE("Error on rfsa_vtl_server_deinit, ret: %d\n", ret);

	while (1) {
		ret = rfsa_list_remove_head(&rfsa_server_used_cmd_q, ((rfsa_list_node_t** )&item));
		if (ret != RFSA_EOK)
			break;
	}

	while (1) {
		ret = rfsa_list_remove_head(&rfsa_server_free_cmd_q, ((rfsa_list_node_t** )&item));
		if (ret != RFSA_EOK)
			break;
	}

	(void)rfsa_lock_destroy(rfsa_server_lock);
	(void)rfsa_event_destroy(rfsa_server_control_event);

	init_done = 0;
	return ret;
}

int rfsa_server_check_received_data(int msg_id, int req_c_struct_len)
{
	switch (msg_id) {
	case QMI_RFSA_FILE_STAT_REQ_MSG_V01:
		return (req_c_struct_len == sizeof(rfsa_file_stat_req_msg_v01));
	case QMI_RFSA_FILE_CREATE_REQ_MSG_V01:
		return (req_c_struct_len == sizeof(rfsa_file_create_req_msg_v01));
	case QMI_RFSA_FILE_READ_REQ_MSG_V01:
		return (req_c_struct_len == sizeof(rfsa_file_read_req_msg_v01));
	case QMI_RFSA_GET_BUFF_ADDR_REQ_MSG_V01:
		return (req_c_struct_len == sizeof(rfsa_get_buff_addr_req_msg_v01));
	case QMI_RFSA_RELEASE_BUFF_ADDR_REQ_MSG_V01:
		return (req_c_struct_len == sizeof(rfsa_release_buff_addr_req_msg_v01));
	case QMI_RFSA_IOVEC_FILE_WRITE_REQ_MSG_V01:
		return (req_c_struct_len == sizeof(rfsa_iovec_file_write_req_msg_v01));
	case QMI_RFSA_IOVEC_FILE_READ_REQ_MSG_V01:
		return (req_c_struct_len == sizeof(rfsa_iovec_file_read_req_msg_v01));
	default:
		LOGE("Unknown opcode, opcode: %d\n", msg_id);
		break;
	}

	return 0;
}

static void write_debugfs_info(void)
{
	int fd;
	fd = open(DEBUGFS_FILE, O_WRONLY);
	if(fd == -1) {
		LOGE("debug open failed with errno %d", errno);
		return;
	}
	write(fd, DEBUGFS_MAGIC_STRING, strlen(DEBUGFS_MAGIC_STRING));
	close(fd);
}

static int open_wakelock_files (void)
{
	wake_lock.unlock_fd = -1;
	wake_lock.lock_fd = -1;

	/* Open wakelock files */
	wake_lock.lock_fd = open(WAKE_LOCK_FILE, O_WRONLY|O_APPEND);
	if(wake_lock.lock_fd < 0) {
		LOGE("Unable to open wake lock file %s : return val: %d, error no: %d string = %s\n",
			WAKE_LOCK_FILE, wake_lock.lock_fd, errno, strerror(errno));
		return -1;
	}

	wake_lock.unlock_fd = open(WAKE_UNLOCK_FILE, O_WRONLY|O_APPEND);
	if(wake_lock.unlock_fd < 0) {
		close(wake_lock.lock_fd);
		wake_lock.lock_fd = -1;
		LOGE("Unable to open wake unlock file %s : return val: %d, error no: %d string = %s\n",
			WAKE_UNLOCK_FILE, wake_lock.lock_fd, errno, strerror(errno));
		return -1;
	}

	return 0;
}

/* Drop unused capabilities */
static void drop_excess_capabilities(void)
{
	unsigned long cap;
	int err;

	/* Allow the caps to be retained after change uid.
	   If this fails only wakelock capability is lost. */
	if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0)
		LOGE("set keepcaps failed! errno=%d ( %s )", errno,
			strerror(errno));

	for (cap = 0; prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0; cap++) {

		if ((cap == CAP_SETUID) || (cap == CAP_SETGID) || (cap == CAP_BLOCK_SUSPEND))
			continue;

		err = prctl(PR_CAPBSET_DROP, cap, 0, 0, 0);
		if ((err < 0) && (errno != EINVAL))
			LOGE("Drop capability for cap=%lu failed errno=%d ( %s )", cap,
				errno, strerror(errno));

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
	capdata[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].permitted |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);
	capdata[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].effective |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);

	/* If this fails only wakelock capability is lost. */
	if ((ret = capset(&capheader, capdata)) < 0)
		LOGE("capset failed: errno=%d ( %s ), ret = %d\n", errno,
			strerror(errno), ret);
	else
		LOGI("Capset success!");
}

static int setup_wakelock_perms (void)
{
	int ret;

	/* Drop the excess capabilities. */
	drop_excess_capabilities();

	ret = setuid(AID_SYSTEM);
	if (ret < 0) {
		LOGE("Error changing uid to system( ret %d) errno = %d\n", ret, errno);
		exit(1); /* It is BAD if we cannot drop to system from root */
	}

	/* enable the capabilities after uid changed. */
	enable_capabilities();
	return 0;
}


static void init_wakelock(void)
{
	int result;
	uid_t my_uid;

	memset (&wake_lock, 0, sizeof(wake_lock));

	my_uid = getuid();

	if (my_uid != AID_ROOT)
	{
		LOGE("Not ROOT user but as user:%u so cant init wakelock\n", my_uid);
		return;
	}

	result = pthread_mutex_init(&wake_lock.mutex, NULL);
	if(result != 0) {
		LOGE("Wake lock mutex not inited : return val: %d, string = %s\n",
			result, strerror(result));
		return;
	}

	result = open_wakelock_files();
	if(result != 0)
		return;

	result = setup_wakelock_perms();
	if(result != 0)
		return;

	wake_lock.write_size = strlen(WAKE_LOCK_STRING);
	wake_lock.pending_req_count = 0;
	wake_lock.is_inited = 1;
}

static void rfsa_wakelock(void)
{
	ssize_t write_res;
	if (wake_lock.is_inited != 1) {
		LOGE("rfsa_wakelock : NOT initialized \n");
		return;
	}
	write_res = write(wake_lock.lock_fd, WAKE_LOCK_STRING, wake_lock.write_size);
	if(write_res == wake_lock.write_size)
		LOGI("Wake-lock acquired successfully\n");
	else
		LOGE("Wake-lock *NOT* acquired,write_res =%zd  errno %d (%s)\n",write_res, errno,
			strerror(errno));
}

static void rfsa_wakeunlock(void)
{
	ssize_t write_res;
	if (wake_lock.is_inited != 1) {
		LOGE("rfsa_wakeunlock : NOT initialized \n");
		return;
	}
	write_res = write(wake_lock.lock_fd, WAKE_LOCK_STRING, wake_lock.write_size);
	if(write_res == wake_lock.write_size)
		LOGI("Wake-lock released successfully\n");
	else
		LOGE("Wake-lock *NOT* released,write_res =%zd  errno %d (%s)\n",write_res, errno,
			strerror(errno));
}



int main(int argc, char **argv)
{
	int32_t ret = RFSA_EOK;
	pid_t pid;

	(void)argc;
	(void)argv;

	/* Write to debugfs */
	write_debugfs_info();

	ret = rfsa_server_init_all_clients();
	if (ret != RFSA_EOK) {
		LOGE("Client initialization failed at startup.\n");
		exit(1);
	}
	LOGI("All clients successfully initialized.\n");

	init_wakelock();

	ret = rfsa_create_data_dirs();

	/* Continue as this failure would only prevent readwrite access the
	   access to the readonly and other folders will still be fine. */
	if (ret != RFSA_EOK) {
		LOGE("All Data dirs couldnt be created.\n");
	}
	else
		LOGI("All Data dirs exist or were created successfully.\n");

	ret = rfsa_server_init();
	if (ret != RFSA_EOK) {
		LOGE("rfsa_server_init failed.\n");
		exit(1);
	}

	return 0;
}
