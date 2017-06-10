/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

	      Diag QShrink4 Application

GENERAL DESCRIPTION
  Contains main implementation for support of Diag QShrink4 database
  retrieval from the APSS file system.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2013 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

			EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:

when       who    what, where, why
--------   ---     ----------------------------------------------------------
9/18/2013  Dixon Peterson  Created
===========================================================================*/

#include "msg.h"
#include "diag_lsm.h"
#include "diag_lsmi.h"
#include "diagcmd.h"
#include "comdef.h"
#include "stdio.h"
#include "diagpkt.h"
#include "string.h"

#include <stdlib.h>
#include <cutils/log.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#ifdef USE_GLIB
#define strlcpy g_strlcpy
#define strlcat g_strlcat
#define strndup g_strndup
#endif

#define MAX_BAD_PKT_LEN 256

#define MAX_FILEPATH_LEN 512
#define MAX_FILENAME_LEN 128
#define MAX_FILENAME_ENTRIES 256 /* Based upon uint8 data type */
#define MAX_OPEN_FILES 6
typedef struct {
	int fd;
	char filename[MAX_FILENAME_LEN];
} open_file_struct;

static open_file_struct open_files[MAX_OPEN_FILES];

#define FILE_CLOSED -1

/*===========================================================================*/
/* Function prototypes for command handlers */
/*===========================================================================*/

/* Subsystem command ids */
#define DIAG_QS4_FILE_LIST	0x020f	/* 527 */
#define DIAG_QS4_OPEN_FILE	0x0210	/* 528 */
#define DIAG_QS4_READ_FILE	0x0211	/* 529 */
#define DIAG_QS4_CLOSE_FILE	0x0212	/* 530 */

PACK(void *) file_list_handler(PACK(void *)req_pkt, uint16 pkt_len);
PACK(void *) open_handler(PACK(void *)req_pkt, uint16 pkt_len);
PACK(void *) read_handler(PACK(void *)req_pkt, uint16 pkt_len);
PACK(void *) close_handler(PACK(void *)req_pkt, uint16 pkt_len);

/*===========================================================================*/
/* User table for this client */
/*===========================================================================*/

static const diagpkt_user_table_entry_type qshrink4_user_tbl[] =
{	/* subsys_cmd_code lo, subsys_cmd_code hi, call back function */
	{DIAG_QS4_FILE_LIST, DIAG_QS4_FILE_LIST, file_list_handler},
	{DIAG_QS4_OPEN_FILE, DIAG_QS4_OPEN_FILE, open_handler},
	{DIAG_QS4_READ_FILE, DIAG_QS4_READ_FILE, read_handler},
	{DIAG_QS4_CLOSE_FILE, DIAG_QS4_CLOSE_FILE, close_handler},
};

/* Size of some of the fields in the commands */

#define HEADER_SIZE (sizeof(uint8) + sizeof(uint8) + sizeof(uint16))
#define VERSION_SIZE sizeof(uint8)
#define FILENAME_LENGTH_SIZE sizeof(uint16)
#define STATUS_SIZE sizeof(uint8)
#define FILE_SIZE_SIZE sizeof(uint32)
#define FD_SIZE sizeof(uint16)

#define PLACEHOLDER_SIZE sizeof(char)

/* Directory where the QShrink4 databases are on the APSS file system */
static char qs4dir[FILE_NAME_LEN] = {"/data/qshrink4"};

typedef PACK(struct) {
	uint8 command_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
	uint8 version;
}file_list_cmd_req_type;

typedef PACK(struct) {
	uint32 file_size;
	uint16 filename_length;
	/* Placeholder put filename here */
	char filename[1];
}filename_entry_type;

typedef PACK(struct) {
	uint8 command_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
	uint8 version;
	uint8 status;
	uint8 num_files;
	/* Placeholder for filename_entry_type data */
	char file_entries_data;
}file_list_cmd_rsp_type;

typedef PACK(struct) {
	uint8 command_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
	uint8 version;
	uint16 filename_length;
	/* Placeholder for filename */
	char filename[1];
}open_cmd_req_type;

typedef PACK(struct) {
	uint8 command_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
	uint8 version;
	uint16 fd;
	uint16 requested_bytes;
	uint32 offset;
}read_cmd_req_type;

typedef PACK(struct) {
	uint8 command_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
	uint8 version;
	uint16 fd;
	uint32 offset;
	int16 num_read;
	uint8 status;
	/* Placeholder, put data here */
	char data;
}read_cmd_rsp_type;

typedef PACK(struct) {
	uint8 command_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
	uint8 version;
	uint16 fd;
}close_cmd_req_type;

typedef PACK(struct) {
	uint8 command_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
	uint8 version;
	uint16 fd;
	uint8 status;
}close_cmd_rsp_type;

/* The maximum number of data bytes to be read from a file at a time */
#define LIBRARY_OVERHEAD 4
#define MAX_READ_BYTES (4 * 1024 - (sizeof(read_cmd_rsp_type) - PLACEHOLDER_SIZE) - LIBRARY_OVERHEAD)

static void usage(char *progname)
{
	DIAG_LOGE("\n Usage for %s:\n", progname);
	DIAG_LOGE("\n-q  --qshrink4dir:\t Directory containing the APSS qshrink4 database files\n");
	DIAG_LOGE("\n-h, --help:\t usage help\n");
	DIAG_LOGE("\ne.g. diag_qshrink4_daemon -q /data/qshrink4\n");
	DIAG_LOGE("\Exiting ...\n");
	exit(0);
}

static void parse_args(int argc, char **argv)
{
	int command;
	struct option longopts[] =
	{
		{ "qshrink4dir",0,	NULL,	'q'},
		{ "help",	0,	NULL,	'h'},
	};

	while ((command = getopt_long(argc, argv, "q:h", longopts, NULL))
			!= -1) {
		switch (command) {
			case 'q':
				strlcpy(qs4dir, optarg, FILE_NAME_LEN);
				set_qshrink4_dir(qs4dir);
				break;
			case 'h':
			default:
				usage(argv[0]);
				break;
		};
	}
}

int main(int argc, char *argv[])
{
	boolean init_success = FALSE;
	int i;

	/* Parse the command line parameters */
	parse_args(argc, argv);

	/* Initialize the diag userspace library */
	init_success = Diag_LSM_Init(NULL);

	if (!init_success) {
		DIAG_LOGE("Diag QShrink4 Daemon: Diag_LSM_Init() failed. Exiting ...\n");
		return -1;
	}

	for (i = 0; i < MAX_OPEN_FILES; i++) {
		open_files[i].fd = FILE_CLOSED;
		open_files[i].filename[0] = '\0';
	}

	/* Register the Diag QShrink4 table */
	DIAGPKT_DISPATCH_TABLE_REGISTER(DIAG_SUBSYS_DIAG_SERV, qshrink4_user_tbl);

	do {
		sleep(3600);
	} while (1);

	/* Close down the diag userpace library. */
	Diag_LSM_DeInit();

	return 0;
}

PACK(uint8 *) bad_packet_len_handler(PACK(void *)req_pkt, uint16 pkt_len)
{
	PACK(uint8 *)rsp = NULL;
	uint16 bad_pkt_len = (pkt_len > MAX_BAD_PKT_LEN) ?
				MAX_BAD_PKT_LEN : pkt_len;

	/* Allocate memory for the response. Note that all
	 * fields set in the packet allocation are going
	 * to be overwritten, so 75 is just a placeholder
	 * to satisfy the diagpkt_alloc api.
	 */
	rsp = (uint8 *)diagpkt_alloc(75, (bad_pkt_len + 1));
	if (rsp) {
		rsp[0] = (uint8)DIAG_BAD_LEN_F;
		memcpy(&rsp[1], req_pkt, bad_pkt_len);
	}

	return rsp;
}

PACK(void *) file_list_handler(PACK(void *)req_pkt, uint16 pkt_len)
{
	/*
	 * FileList command protocol:
	 *
	 * Request:
	 *
	 * uint8 command_code
	 * uint8 subsys_id
	 * uint16 subsys_cmd_code
	 * uint8 version
	 *
	 * Response:
	 *
	 * File entry structure:
	 *
	 * uint32 file_size;
	 * uint16 filename_length;
	 * char filename[filename_length];
	 *
	 * Response command:
	 * uint8 command_code;
	 * uint8 subsys_id;
	 * uint16 subsys_cmd_code;
	 * uint8 version;
	 * uint8 status;
	 * uint8 num_files;
	 * Place holder where all file entries are placed
	 * char file_entries_data;
	 */

	file_list_cmd_req_type *req = (file_list_cmd_req_type *)req_pkt;
	file_list_cmd_rsp_type *rsp = NULL;

	DIR *dir_ptr = NULL;
	struct dirent *dir_entry = NULL;
	int status = 0;
	char filepath[MAX_FILEPATH_LEN];
	struct file_entry {
		uint32 file_size;
		uint16 filename_length;
		char* filename;
	};
	struct file_entry *file_entries;
	int num_files = 0;
	int num_file_entries = MAX_OPEN_FILES;
	int req_size = sizeof(file_list_cmd_req_type);
	int rsp_size = 0;
	int num_file_entry_bytes = 0;
	uint8 *rsp_entry = NULL;
	uint8 error_status = 0;
	int i;

	/* Check if the packet length is valid */
	if (pkt_len != req_size) {
		PACK(uint8 *)bad_rsp = NULL;
		DIAG_LOGE("diag: In %s, invalid packet length: %d, expecting: %d\n",
				__func__, pkt_len, req_size);
		bad_rsp = bad_packet_len_handler(req_pkt, pkt_len);
		return bad_rsp;
	}

	dir_ptr = opendir(qs4dir);
	if (!dir_ptr) {
		DIAG_LOGE("diag: In %s, error calling opendir on directory %s, errno: %d\n",
				__func__, qs4dir, errno);
		error_status = get_qshrink4_error(errno);

		rsp_size = sizeof(file_list_cmd_rsp_type) - PLACEHOLDER_SIZE;
		rsp = (file_list_cmd_rsp_type *)diagpkt_subsys_alloc(req->subsys_id, req->subsys_cmd_code, rsp_size);

		if (rsp) {
			rsp->version = req->version;
			rsp->status = error_status;
			rsp->num_files = num_files;
		} else {
			DIAG_LOGE("diag: In %s, unable to allocate memory for Diag QShrink4 file list command response\n",
				__func__);
		}
	} else {
		file_entries = (struct file_entry *)malloc(num_file_entries * sizeof(struct file_entry));
		if (!file_entries) {
			DIAG_LOGE("diag: In %s, Unable to allocate memory to determine directory contents\n",
				__func__);
			goto cleanup;
		}

		do
		{
			errno = 0;
			dir_entry = readdir(dir_ptr);
			if (dir_entry) {
				struct stat file_stat;
				int stat_val = 0;
				int is_reg = -1;
				strlcpy(filepath, qs4dir, MAX_FILEPATH_LEN);
				(void)strlcat(filepath, "/", MAX_FILEPATH_LEN);
				(void)strlcat(filepath, dir_entry->d_name, MAX_FILEPATH_LEN);
				stat_val = stat(filepath, &file_stat);
				is_reg = S_ISREG(file_stat.st_mode);
				/* If this is a regular file */
				if (is_reg) {
					/* If there is not enough room to add more files */
					if (num_files >= num_file_entries) {
						struct file_entry *tmp_entries;
						tmp_entries = realloc(file_entries,
								(num_file_entries + 5) * sizeof(struct file_entry));
						if (tmp_entries) {
							file_entries = tmp_entries;
							num_file_entries += 5;
						} else {
							// Error can't resize, will have to quit
							DIAG_LOGE("diag: In %s, error can't realloc, errno: %d\n",
								__func__, errno);
							error_status = get_qshrink4_error(ENOMEM);
							break;
						}
					}

					file_entries[num_files].file_size = (uint32)file_stat.st_size;
					file_entries[num_files].filename = (char *)strndup(dir_entry->d_name,
										(size_t)(MAX_FILENAME_LEN - 1));
					file_entries[num_files].filename_length = strlen(file_entries[num_files].filename) + 1;

					if (!file_entries[num_files].filename) {
						// Error no memory, will need to quit
						DIAG_LOGE("diag: In %s, no memory to strndup %s\n",
							__func__, dir_entry->d_name);
						error_status = get_qshrink4_error(ENOMEM);
						break;
					} else {
						num_file_entry_bytes += FILE_SIZE_SIZE + FILENAME_LENGTH_SIZE +
								file_entries[num_files].filename_length;
						num_files++;
					}
				}
			}
		/*
		 * Support up to MAX_FILENAME_ENTRIES, since that is the max
		 * that rsp_num_files field in the command (uint8) can support
		 */
		} while (dir_entry && (num_files < MAX_FILENAME_ENTRIES));

		rsp_size = sizeof(file_list_cmd_rsp_type) - PLACEHOLDER_SIZE;
		if (error_status == 0)
			rsp_size += num_file_entry_bytes;

		rsp = (file_list_cmd_rsp_type *)diagpkt_subsys_alloc(req->subsys_id, req->subsys_cmd_code, rsp_size);
		if (!rsp) {
			DIAG_LOGE("diag: In %s, unable to allocate memory for Diag QShrink4 file list command response\n",
				__func__);
			goto cleanup;
		}

		rsp->version = req->version;
		rsp->status = error_status;
		if (error_status != 0) {
			rsp->num_files = 0;
		} else {
			rsp->num_files = num_files;
			rsp_entry = (uint8 *)&rsp->file_entries_data;

			/* Fill in the file entries in the response */
			for (i = 0; i < num_files; i++) {
				filename_entry_type *entry = (filename_entry_type *)rsp_entry;
				int num_entry_bytes = FILE_SIZE_SIZE + FILENAME_LENGTH_SIZE +
							file_entries[i].filename_length;

				entry->file_size = file_entries[i].file_size;
				entry->filename_length = file_entries[i].filename_length;
				memcpy(entry->filename, file_entries[i].filename,
							file_entries[i].filename_length);

				rsp_entry += num_entry_bytes;
			}
		}

cleanup:
		/* Clean-up */
		if (file_entries) {
			for (i = 0; i < num_files; i++) {
				free(file_entries[i].filename);
			}

			free(file_entries);
		}

		status = closedir(dir_ptr);

		if (status == -1) {
			DIAG_LOGE("In %s, closedir error, errno: %d\n",
				__func__, errno);
		}
	}

	return rsp;
}

PACK(void *) open_handler(PACK(void *)req_pkt, uint16 pkt_len)
{
	/*
	 * The open command protocol:
	 *
	 * Request:
	 *
	 * uint8 command_code;
	 * uint8 subsys_id;
	 * uint16 subsys_cmd_code;
	 * uint8 version;
	 * uint16 filename_length;
	 * char filename[filename_length];
	 *
	 * Response:
	 *
	 * uint8 command_code;
	 * uint8 subsys_id;
	 * uint16 subsys_cmd_code;
	 * uint8 version;
	 * uint16 filename_length;
	 * char filename[filename_length];
	 * uint16 fd;
	 * uint8 status;
	 */

	open_cmd_req_type *req = (open_cmd_req_type *)req_pkt;
	PACK(uint8 *)rsp = NULL;

	int rsp_size = 0;
	int version_offset = 0;
	int filename_length_offset = 0;
	int filename_offset = 0;
	int fd_offset = 0;
	int status_offset = 0;
	uint8 *rsp_version = NULL;
	uint16 *rsp_filename_length = NULL;
	uint8 *rsp_filename = NULL;
	uint16 *rsp_fd = NULL;
	uint8 * rsp_status = NULL;
	int i;
	int fd = -1;
	char filepath[MAX_FILEPATH_LEN];
	int is_diff = 1;
	int file_index = -1;
	uint8 error_status = 0;

	version_offset = HEADER_SIZE;
	filename_length_offset = version_offset + VERSION_SIZE;
	filename_offset = filename_length_offset + FILENAME_LENGTH_SIZE;
	fd_offset = filename_offset + req->filename_length;
	status_offset = fd_offset + FD_SIZE;

	/* Check to make sure the packet length is correct */
	if (pkt_len != (filename_offset + req->filename_length)) {
		PACK(uint8 *)bad_rsp = NULL;
		DIAG_LOGE("diag: In %s, invalid packet length: %d, expecting: %d\n",
				__func__, pkt_len, (filename_offset + req->filename_length));
		bad_rsp = bad_packet_len_handler(req_pkt, pkt_len);
		return bad_rsp;
	}

	rsp_size = status_offset + STATUS_SIZE;

	rsp = (uint8 *)diagpkt_subsys_alloc(req->subsys_id, req->subsys_cmd_code, rsp_size);
	if (!rsp) {
		DIAG_LOGE("diag: In %s, unable to allocate memory for Diag QShrink4 open command response\n",
			__func__);
		goto bail;
	}

	rsp_version = (uint8 *)rsp + version_offset;
	rsp_filename_length = (uint16 *)((uint8 *)rsp + filename_length_offset);
	rsp_filename = (uint8 *)rsp + filename_offset;
	rsp_fd = (uint16 *)((uint8 *)rsp + fd_offset);
	rsp_status = (uint8 *)rsp + status_offset;

	/* Verify the version */
	if (req->version != 1) {
		error_status = get_qshrink4_error(EINVAL);
		goto respond;
	}

	/* Determine if the file is already open */
	for (i = 0; i < MAX_OPEN_FILES; i++) {
		if (open_files[i].fd != FILE_CLOSED) {
			int name_len = strlen(open_files[i].filename) + 1;
			if (req->filename_length == name_len) {
				is_diff = strncmp(open_files[i].filename,
						req->filename,
						req->filename_length);
				if (!is_diff) {
					/* File is already open, we don't
					 * need to do anything else
					 */
					file_index = i;
					break;
				}
			}
		}
	}

	/* If the filename is different from any open files, open the file */
	if (is_diff) {
		strlcpy(filepath, qs4dir, MAX_FILEPATH_LEN);
		(void)strlcat(filepath, "/", MAX_FILEPATH_LEN);
		(void)strlcat(filepath, req->filename, MAX_FILEPATH_LEN);

		for (i = 0; i < MAX_OPEN_FILES; i++) {
			if (open_files[i].fd == FILE_CLOSED) {
				fd = open(filepath,O_RDONLY);
				if (fd == -1) {
					DIAG_LOGE("diag: In %s, open error, errno: %d, filename: %s\n",
						__func__, errno, filepath);
					error_status = get_qshrink4_error(errno);
				} else {
					int num_bytes = (req->filename_length < MAX_FILENAME_LEN) ?
							req->filename_length : MAX_FILENAME_LEN;
					open_files[i].fd = fd;
					file_index = i;
					memcpy(open_files[i].filename,
						req->filename, num_bytes);
				}
				break;
			}
		}
		if (i == MAX_OPEN_FILES) {
			/* Too many files open, can't open another */
			error_status = get_qshrink4_error(EMFILE);
		}
	}

respond:
	*rsp_version = req->version;
	*rsp_filename_length = req->filename_length;
	memcpy(rsp_filename, req->filename, req->filename_length);
	/* The fd in the command is actually the index into the open_files array */
	*rsp_fd = file_index;
	*rsp_status = (file_index == -1) ? error_status: 0;

bail:
	return rsp;
}

PACK(void *) read_handler(PACK(void *)req_pkt, uint16 pkt_len)
{
	/*
	 * Read command protocol:
	 *
	 * Request command:
	 * uint8 command_code
	 * uint8 subsys_id
	 * uint16 subsys_cmd_code
	 * uint8 version
	 * uint16 fd
	 * uint16 requested_bytes
	 * uint32 offset
	 *
	 * Response command:
	 *
	 * uint8 command_code
	 * uint8 subsys_id
	 * uint16 subsys_cmd_code
	 * uint8 version
	 * uint16 fd
	 * uint32 offset
	 * int16 num_read
	 * uint8 status
	 * char data[num_read] - data is placed here
	 */

	read_cmd_req_type *req = (read_cmd_req_type *)req_pkt;
	read_cmd_rsp_type *rsp = NULL;

	int req_size = sizeof(read_cmd_req_type);
	int rsp_size = 0;

	size_t num_bytes_to_read = 0;
	off_t offset = 0;
	off_t seek_offset = 0;
	uint8 error_status = 0;

	ssize_t num_bytes_read = 0;
	unsigned char buf[MAX_READ_BYTES];
	int i;

	/* Verify packet length, version number and file descriptor */
	if (pkt_len != req_size) {
		/* Check if the packet length is valid */
		PACK(uint8 *)bad_rsp = NULL;
		DIAG_LOGE("diag: In %s, invalid packet length: %d, expecting: %d\n",
				__func__, pkt_len, req_size);
		bad_rsp = bad_packet_len_handler(req_pkt, pkt_len);
		return bad_rsp;
	} else if (req->version != 1) {
		DIAG_LOGE("diag: In %s, invalid version: %d\n",
			__func__, req->version);
		error_status = get_qshrink4_error(EINVAL);
	} else if (req->fd >= MAX_OPEN_FILES) {
		DIAG_LOGE("diag: In %s, invalid fd: %d, out of range\n",
			__func__, req->fd);
		error_status = get_qshrink4_error(EBADF);
	} else if (open_files[req->fd].fd == FILE_CLOSED) {
		DIAG_LOGE("diag: In %s, fd is invalid, req->fd: %d, fd: %d, filename: %s\n",
			__func__, req->fd, open_files[req->fd].fd,
			open_files[req->fd].filename);
		error_status = get_qshrink4_error(EBADF);
	} else {
		num_bytes_to_read = (req->requested_bytes > MAX_READ_BYTES) ?
					MAX_READ_BYTES : req->requested_bytes;
		offset = req->offset;
		/* Seek to the appropriate offset in the file */
		seek_offset = lseek(open_files[req->fd].fd, offset, SEEK_SET);
		if (seek_offset == -1) {
			DIAG_LOGE("diag: In %s, seek error, errno: %d, filename: %s\n",
			__func__, errno, open_files[req->fd].filename);
			error_status = get_qshrink4_error(errno);
		} else {
			num_bytes_read = read(open_files[req->fd].fd, buf,
						num_bytes_to_read);
			if (num_bytes_read == -1) {
				DIAG_LOGE("diag: In %s, read error, errno: %d, packet fd: %d, filename: %s\n",
					__func__, errno, req->fd,
					open_files[req->fd].filename);
				error_status = get_qshrink4_error(errno);
			}
		}
	}

	/* Prepare the response packet */
	rsp_size = sizeof(read_cmd_rsp_type) - PLACEHOLDER_SIZE;
	if (error_status == 0)
		rsp_size += num_bytes_read;

	rsp = (read_cmd_rsp_type *)diagpkt_subsys_alloc(req->subsys_id, req->subsys_cmd_code,
						rsp_size);
	if (!rsp) {
		DIAG_LOGE("diag: In %s, unable to allocate memory for Diag QShrink4 read command response\n",
			__func__);
		goto bail;
	}

	rsp->version = req->version;
	rsp->fd = req->fd;
	rsp->offset = req->offset;
	rsp->num_read = (error_status == 0) ? num_bytes_read : -1;
	rsp->status = error_status;
	/* Only include data if there was no error */
	if (error_status == 0)
		memcpy(&rsp->data, buf, num_bytes_read);

bail:
	return rsp;
}

PACK(void *) close_handler(PACK(void *)req_pkt, uint16 pkt_len)
{
	/*
	 * Close command protocol
	 *
	 * Request:
	 * uint8 command_code
	 * uint8 subsys_id
	 * uint16 subsys_cmd_code
	 * uint8 version
	 * uint16 fd
	 *
	 * Response:
	 * uint8 command_code
	 * uint8 subsys_id
	 * uint16 subsys_cmd_code
	 * uint8 version
	 * uint16 fd
	 * uint8 status
	 */

	close_cmd_req_type *req = (close_cmd_req_type *)req_pkt;
	close_cmd_rsp_type *rsp = NULL;

	int req_size = sizeof(close_cmd_req_type);
	int rsp_size = sizeof(close_cmd_rsp_type);
	int i;
	int status = 0;
	uint8 error_status = 0;

	if (pkt_len != req_size) {
		/* Check if the packet length is valid */
		PACK(uint8 *)bad_rsp = NULL;
		DIAG_LOGE("diag: In %s, invalid packet length: %d, expecting: %d\n",
				__func__, pkt_len, req_size);
		bad_rsp = bad_packet_len_handler(req_pkt, pkt_len);
		return bad_rsp;
	} else if (req->version != 1) {
		DIAG_LOGE("diag: In %s, invalid version: %d\n",
			__func__, req->version);
		error_status = get_qshrink4_error(EINVAL);
	} else if (req->fd >= MAX_OPEN_FILES) {
		DIAG_LOGE("diag: In %s, invalid fd: %d\n",
			__func__, req->fd);
		error_status = get_qshrink4_error(EBADF);
	} else if (open_files[req->fd].fd == FILE_CLOSED) {
		DIAG_LOGE("diag: In %s, fd is invalid, req->fd: %d, fd: %d, filename: %s\n",
			__func__, req->fd, open_files[req->fd].fd,
			open_files[req->fd].filename);
		error_status = get_qshrink4_error(EBADF);
	} else {
		status = close(open_files[req->fd].fd);
		if (status == -1) {
			DIAG_LOGE("diag: In %s, close error, errno: %d\n",
				__func__, errno);
			error_status = get_qshrink4_error(errno);
		} else {
			open_files[req->fd].fd = FILE_CLOSED;
			open_files[req->fd].filename[0] = '\0';
		}
	}

	rsp = (close_cmd_rsp_type *)diagpkt_subsys_alloc(req->subsys_id, req->subsys_cmd_code,
						rsp_size);
	if (!rsp) {
		DIAG_LOGE("diag: In %s, unable to allocate memory for Diag QShrink4 close command response\n",
			__func__);
		goto bail;
	}

	rsp->version = req->version;
	rsp->fd = req->fd;
	rsp->status = error_status;
bail:
	return rsp;
}
