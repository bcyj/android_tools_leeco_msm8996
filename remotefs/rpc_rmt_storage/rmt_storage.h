/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 */

#ifndef __RMT_STORAGE_SERVER_H
#define __RMT_STORAGE_SERVER_H

#include <linux/types.h>
#include <linux/ioctl.h>

#ifdef ANDROID
#define HAVE_LIBLOG
#endif

#ifdef HAVE_LIBLOG
#define LOG_TAG "rmt_storage"
#include "cutils/log.h"
#include "common_log.h"
#else
// define necessary log macros and types
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define LOGI(...)      fprintf(stdout,__VA_ARGS__)
#define LOGE(...)      fprintf(stderr,__VA_ARGS__)
#define LOGV(...)      fprintf(stderr,__VA_ARGS__)
#endif

#define RMT_STORAGE_OPEN              0
#define RMT_STORAGE_WRITE             1
#define RMT_STORAGE_CLOSE             2
#define RMT_STORAGE_SEND_USER_DATA    3
#define RMT_STORAGE_READ              4

#define RMT_STORAGE_MAX_IOVEC_XFR_CNT 5
#define MAX_NUM_CLIENTS 10
#define MAX_RAMFS_TBL_ENTRIES 3
#define MAX_SHRD_MEM_ENTRIES (MAX_RAMFS_TBL_ENTRIES + 2)
#define RAMFS_BLOCK_SIZE 512

enum {
	RMT_STORAGE_NO_ERROR = 0,	/* Success */
	RMT_STORAGE_ERROR_PARAM,	/* Invalid parameters */
	RMT_STORAGE_ERROR_PIPE,		/* RPC pipe failure */
	RMT_STORAGE_ERROR_UNINIT,	/* Server is not initalized */
	RMT_STORAGE_ERROR_BUSY,		/* Device busy */
	RMT_STORAGE_ERROR_DEVICE	/* Remote storage device */
} rmt_storage_status;

struct rmt_storage_iovec_desc {
	uint32_t sector_addr;
	uint32_t data_phy_addr;
	uint32_t num_sector;
};

#define MAX_PATH_NAME 32
struct rmt_storage_event {
	uint32_t id;		/* Event ID */
	uint32_t sid;		/* Storage ID */
	uint32_t handle;	/* Client handle */
	char path[MAX_PATH_NAME];
	struct rmt_storage_iovec_desc xfer_desc[RMT_STORAGE_MAX_IOVEC_XFR_CNT];
	uint32_t xfer_cnt;
	uint32_t usr_data;
};

struct rmt_storage_send_sts {
	uint32_t err_code;
	uint32_t data;
	uint32_t handle;
	uint32_t xfer_dir;
};

struct rmt_shrd_mem_param {
	uint32_t sid;		/* Storage ID */
	uint32_t start;		/* Physical memory address */
	uint32_t size;		/* Physical memory size */
	void *base;		/* Virtual user-space memory address */
};

struct partition_lookup_entry {
	uint8_t partition_type;
	char path[MAX_PATH_NAME];
	char devpath[MAX_PATH_NAME];
	int fd;
};

#define SECTOR_SIZE 	512
#define BOOT_REC_SIG 	0xAA55

#define MAX_MBR_ENTRIES 4
#define MAX_EBR_ENTRIES 2

#define EXTENDED_PARTITION_TYPE	5

struct partition_entry {
	uint8_t status;
	uint8_t rsvd0[3];
	uint8_t type;
	uint8_t rsvd1[3];
	uint32_t start_sector;
	uint32_t partition_size;
} __attribute__((__packed__));

struct boot_rec {
	uint8_t rsvd0[446];
	struct partition_entry part_entry[MAX_MBR_ENTRIES];
	uint16_t sig;
} __attribute__((__packed__));

#define RMT_STORAGE_IOCTL_MAGIC (0xC2)

#define RMT_STORAGE_SHRD_MEM_PARAM \
	_IOWR(RMT_STORAGE_IOCTL_MAGIC, 0, struct rmt_shrd_mem_param)

#define RMT_STORAGE_WAIT_FOR_REQ \
	_IOR(RMT_STORAGE_IOCTL_MAGIC, 1, struct rmt_storage_event)

#define RMT_STORAGE_SEND_STATUS \
	_IOW(RMT_STORAGE_IOCTL_MAGIC, 2, struct rmt_storage_send_sts)
#endif
