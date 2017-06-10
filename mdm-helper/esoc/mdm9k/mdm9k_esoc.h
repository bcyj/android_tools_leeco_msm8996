/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/esoc_ctrl.h>
#include <linux/ioctl.h>
#include <cutils/properties.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <mdm_helper.h>
#include <cutils/android_reboot.h>


#define NUM_ESOC_THREADS 2
#define MDM9K_IMAGE_PATH "/firmware/image/"
#define NUM_COMPORT_RETRIES 50
#define MAX_PATH_LEN 255
#define HSIC_NOTIFICATION_NODE "/sys/devices/msm_hsic_host/host_ready"
#define MAX_HSIC_NOTIFICATION_RETRIES 50
#define MODE_BOOT 1
#define MODE_RUNTIME 2
#define MODE_RAMDUMP 3
#define TEMP_FILE_DIR "/data/misc/mdmhelperdata/"
#define EFS_FILE_DIR "/dev/block/platform/msm_sdcc.1/by-name/"
#define KS_PATH "/system/bin/ks"
#define RAM_DUMP_IMAGE 21
#define NUM_EFS_PARTITIONS 3
#define NUM_OTHER_HEADER_PREPEND_FILES 1
#define NUM_LINK_RETRIES 50
#define DELAY_BETWEEN_RETRIES_MS 500
#define NUM_COM_PORT_RETRIES 50
#define MAX_STATUS_UP_CHECKS 60
#define BOOT_TYPE_NODE "/proc/sys/kernel/cold_boot"

typedef __u32 u32;

typedef enum Mdm9kEsocEvents {
	ESOC_EVENT_REQ_ENG_RDY = 0,
	ESOC_EVENT_REQ_XFER_IMG
} Mdm9kEsocEvents;

struct image_id_mapping {
	int image_id;
	char* filename;
};

struct headers_to_prepend {
	struct image_id_mapping file_details;
	char* header_file;
	char* binary_file;
};
struct partitions_to_file_dump {
	struct headers_to_prepend header_info;
	char *partition;
	size_t kb;
};

struct mdm_private_data {
	int reset_device_before_ramdumps;
	int (*peripheral_cmd)(struct mdm_device *dev, int nCmd);
	char* flashless_boot_device;
	char* efs_sync_device;
	char* transport_bind_node;
	char *transport_unbind_node;
	char *transport_bind_command;
	char *transport_unbind_command;
	// Prefix added to files received as RAM dumps, this
	// is used when launching kickstart for flashless boot
	char* ram_dump_file_prefix;
	// Prefix added to files recveived during an EFS sync. This
	// is used when launching kickstart for EFS sync
	char* efs_sync_file_prefix;
	char* efs_file_dir;
	pid_t efs_ks_pid;
	struct partitions_to_file_dump partition_list[NUM_EFS_PARTITIONS];
	struct headers_to_prepend other_prepend_images \
		[NUM_OTHER_HEADER_PREPEND_FILES];
	struct image_id_mapping image_list[];
};

enum {
	PERIPHERAL_CMD_BIND = 1,
	PERIPHERAL_CMD_UNBIND,
};

struct mdm_private_data private_data_9x25_hsic;
struct mdm_private_data private_data_9x25_hsic_emmc_ufs;

int mdm9k_powerup(struct mdm_device *dev);
int mdm9k_shutdown(struct mdm_device *dev);
int mdm9k_monitor(struct mdm_device *);
int mdm9k_ramdump_collect(struct mdm_device *);
int mdm9k_cleanup(struct mdm_device *);
int LoadSahara(struct mdm_device *dev, char* options);
int WaitForCOMport(char *DevNode, int max_retries, int attempt_read);
void peripheral_reset(struct mdm_device *dev);
