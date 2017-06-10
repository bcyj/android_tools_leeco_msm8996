/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 */

#include <stdio.h>
#include <cutils/properties.h>
#include "mdm_detect.h"
#define LOG_TAG "ssr_setup"
#include <cutils/log.h>

#define SSR_BUS_ROOT "/sys/bus/msm_subsys/devices"
#define SSR_SETUP_PROP "persist.sys.ssr.restart_level"
#define MAX_NUM_SUBSYS 10
#define SUBSYS_NAME_LEN 30

static char subsys_list[MAX_NUM_SUBSYS][SUBSYS_NAME_LEN];

struct ssr_node_map {
	char subsys_name[SUBSYS_NAME_LEN];
	char ssr_node[MAX_PATH_LEN];
};
struct ssr_data {
	int count;
	struct ssr_node_map subsys_map[MAX_NUM_SUBSYS];
};
static struct ssr_data ssr_info;

//Goes through the ssr devices directory and
//fills up the ssr_info structure with the
//subsys name/node mappings as well as the
//total number of subsystems currently supported
//on the target.
int populate_subsys_map()
{
	DIR *dir_ssr_bus;
	struct dirent *de;
	int fd = -1;
	int count = 0;
	char subsys_name_path[MAX_PATH_LEN];
	char subsys_name[MAX_NAME_LEN];
	char *soc_name;

	dir_ssr_bus = opendir(SSR_BUS_ROOT);
	if (!dir_ssr_bus) {
		ALOGE("Failed to open ssr root dir: %s",
				strerror(errno));
		goto error;
	}
	//Go through /sys/bus/msm_subsys/devices dir
	while((de = readdir(dir_ssr_bus))) {
		if (de->d_name[0] == '.')
			continue;
		if (count >= MAX_NUM_SUBSYS) {
			ALOGE("Too many subsystems found. Bailing out");
			return RET_FAILED;
		}
		snprintf(subsys_name_path,
				sizeof(subsys_name_path),
				"%s/%s/name",
				SSR_BUS_ROOT,
				de->d_name);
		fd = open(subsys_name_path, O_RDONLY);
		if (fd < 0) {
			ALOGE("Failed to open %s : %s",
					subsys_name_path,
					strerror(errno));
			goto error;
		}
		memset(subsys_name, '\0', sizeof(subsys_name));
		if (read(fd, (void*)subsys_name,
					sizeof(subsys_name) -1 ) < 0) {
			ALOGE("Failed to read %s: %s", subsys_name_path,
					strerror(errno));
			goto error;
		}
		if (subsys_name[strlen(subsys_name) - 1] == '\n')
			subsys_name[strlen(subsys_name) - 1] = '\0';
		close(fd);
		fd = -1;
		//If the device is a esoc then lookup actual device
		//name from libmdmdetect.
		if (!strncmp(subsys_name, "esoc", 4)) {
			soc_name = get_soc_name(subsys_name);
			if (!soc_name) {
				ALOGE("Failed to get esoc name");
				goto error;
			}
			strlcpy(subsys_name,
					soc_name,
					sizeof(subsys_name));
			free(soc_name);
		}
		if (strlen(subsys_name) > 0 &&
				subsys_name[strlen(subsys_name) - 1] == '\n')
			subsys_name[strlen(subsys_name) - 1] = '\0';
		strlcpy(ssr_info.subsys_map[count].subsys_name,
				subsys_name,
				sizeof(ssr_info.subsys_map[count].subsys_name));
		strlcpy(ssr_info.subsys_map[count].ssr_node,
				de->d_name,
				sizeof(ssr_info.subsys_map[count].ssr_node));
		ssr_info.count++;
		count++;
	}
	closedir(dir_ssr_bus);
	return RET_SUCCESS;
error:
	if (fd >= 0)
		close(fd);
	if (dir_ssr_bus)
		closedir(dir_ssr_bus);
	return RET_FAILED;
}

int setup_ssr(int num_subsys)
{
	char ssr_toggle_path[MAX_PATH_LEN];
	int fd = -1;
	int enable_ssr = 0;
	int i, j;
	int enable_count = 0;
	int enable_all = 0;
	int disable_all = 0;
	DIR *dir_ssr_bus = NULL;
	int match_found = 0;
	struct dirent *de;
	char enable_list[MAX_NUM_SUBSYS][SUBSYS_NAME_LEN];

	enable_all = !strncmp(subsys_list[0], "ALL_ENABLE", 10);
	disable_all = !strncmp(subsys_list[0], "ALL_DISABLE", 11);
	//Go through list of names passed in the command line and compare them
	//to the list of supported subsys names. Add any matches to the enable
	//list
	for (i = 0; i < num_subsys; i++)
	{
		for (j = 0; j < ssr_info.count; j++) {
			if (!strncmp(subsys_list[i],
						ssr_info.subsys_map[j].\
						subsys_name,
						sizeof(subsys_list[i])) ||
					enable_all) {
				strlcpy(enable_list[enable_count],
						ssr_info.subsys_map[j].ssr_node,
						sizeof(enable_list \
							[enable_count]));
				enable_count++;
				match_found = 1;
				if (!enable_all)
					break;

			}
		}
		if (enable_all)
			break;
		if (!match_found && !disable_all)
			ALOGE("No subsys found with name %s", subsys_list[i]);
		match_found = 0;
	}
	if (enable_count == 0 && !disable_all) {
		ALOGE("Argument list does not match any know subsystem");
		return RET_FAILED;
	}
	//Go through ssr_bus_root dir enabling everything in the enable list
	//and disabling everything else.
	dir_ssr_bus = opendir(SSR_BUS_ROOT);
	if (!dir_ssr_bus) {
		ALOGE("Failed to open SSR root dir: %s", strerror(errno));
		goto error;
	}
	while((de = readdir(dir_ssr_bus))) {
		if (de->d_name[0] == '.')
			continue;
		for (i = 0; i < enable_count; i++)
		{
			if (!strncmp(enable_list[i],
						de->d_name,
						sizeof(enable_list[i]))) {
				enable_ssr = 1;
				break;
			}
		}
		snprintf(ssr_toggle_path,
				sizeof(ssr_toggle_path),
				"%s/%s/restart_level",
				SSR_BUS_ROOT,
				de->d_name);
		fd = open(ssr_toggle_path, O_WRONLY);
		if (fd < 0) {
			ALOGE("Failed to open %s : %s",
					ssr_toggle_path,
					strerror(errno));
			goto error;
		}
		if (enable_ssr) {
			ALOGI("Enabling SSR for %s", de->d_name);
			if (write(fd, "related",7) < 0) {
				ALOGE("Failed to write to ssr node: %s",
						strerror(errno));
				goto error;
			}
		} else {
			ALOGI("Disabling ssr for %s", de->d_name);
			if (write(fd, "system",6) < 0) {
				ALOGE("Failed to write to ssr node: %s",
						strerror(errno));
				goto error;
			}
		}
		close(fd);
		fd = -1;
		enable_ssr = 0;
	}
	closedir(dir_ssr_bus);
	return RET_SUCCESS;
error:
	if (fd >= 0)
		close(fd);
	if (dir_ssr_bus)
		closedir(dir_ssr_bus);
	return RET_FAILED;
}

int dump_supported_subsys()
{
	int i;
	ALOGI("Supported subsystems:");
	for (i = 0; i < ssr_info.count; i++) {
		ALOGI("%d : %s", i+1, ssr_info.subsys_map[i].subsys_name);
	}
	return RET_SUCCESS;
}

int main(int argc, char *argv[])
{
	int i = 0;
	char *tok_ptr;
	int use_prop = 0;
	char *subsys_name;
	int num_subsys = 0;
	char subsys_buf[PROPERTY_VALUE_MAX] = {0};
	if (argc < 2) {
		ALOGI("Using persist.sys.ssr.restart_level for ssr_setup");
		use_prop = 1;
	}
	if (argc > MAX_NUM_SUBSYS + 1) {
		ALOGE("Invalid arguments: Too many subsys names provided");
		return RET_FAILED;
	}
	if (populate_subsys_map() != RET_SUCCESS) {
		ALOGE("Failed to get subsys list");
		return RET_FAILED;
	}
	if ((!use_prop) && !strncmp(argv[1], "--dump", 6)) {
		return dump_supported_subsys();
	}
	if (!use_prop) {
		for (i = 1; i < argc; i++)
		{
			strlcpy(subsys_list[i-1], argv[i],
					SUBSYS_NAME_LEN * sizeof(char));
			num_subsys++;
		}
	} else {
		property_get(SSR_SETUP_PROP, subsys_buf, "N/A");
		if (!strncmp(subsys_buf, "N/A", sizeof(subsys_buf))) {
			ALOGI("ssr prop empty. Disabling SSR for all modules");
			strlcpy(subsys_list[0], "ALL_DISABLE", SUBSYS_NAME_LEN *
					sizeof(char));
			num_subsys++;
			goto setup;
		}
		subsys_name = strtok_r(subsys_buf, " ", &tok_ptr);
		while (subsys_name && num_subsys < MAX_NUM_SUBSYS) {
			strlcpy(subsys_list[num_subsys], subsys_name,
					SUBSYS_NAME_LEN * sizeof(char));
			num_subsys++;
			subsys_name = strtok_r(NULL, " ", &tok_ptr);
		}
		if (num_subsys >= MAX_NUM_SUBSYS)
			ALOGW("Too many subsystems.List has been truncated");
	}
setup:
	setup_ssr(num_subsys);
	return 0;
}
