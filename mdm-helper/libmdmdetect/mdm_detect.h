/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <cutils/properties.h>

#define ESOC_ROOT_DIR "/sys/bus/esoc/devices"
#define SSR_BUS_ROOT "/sys/bus/msm_subsys/devices"
#define RET_SUCCESS 0
#define RET_FAILED 1
#define MAX_SUPPORTED_MDM 4
#define MAX_NAME_LEN 32
#define MAX_PATH_LEN 255
#define LINK_HSIC "HSIC"
#define LINK_PCIE "PCIe"
#define LINK_HSIC_PCIE "HSIC+PCIe"
#define LINK_SMD "SMD"
#define PER_STATE_ONLINE 3
#define PER_STATE_OFFLINE 4
typedef enum MdmType {
	MDM_TYPE_EXTERNAL = 0,
	MDM_TYPE_INTERNAL,
} MdmType;

struct mdm_info {
	MdmType type;
	char mdm_name[MAX_NAME_LEN];
	char mdm_link[MAX_NAME_LEN];
	char powerup_node[MAX_PATH_LEN];
	char drv_port[MAX_PATH_LEN];
	char ram_dump_path[MAX_PATH_LEN];
	char esoc_node[MAX_NAME_LEN];
};


struct dev_info {
	int num_modems;
	struct mdm_info mdm_list[MAX_SUPPORTED_MDM];
};

char *get_soc_name(char *esoc_dev_node);
char *get_soc_link(char *esoc_dev_node);
char *get_soc_ramdump_path(char *esoc_dev_node);
char *get_soc_port(char *esoc_dev_node);
int esoc_supported(char *esoc_node);
int esoc_framework_supported();
int get_system_info(struct dev_info *dev);
int get_peripheral_state(char *name);
