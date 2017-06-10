/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 */

#include <mdm_detect.h>
#define LOG_TAG "libmdmdetect"
#include <cutils/log.h>
#define LOG_NIDEBUG 0
#define RAMDUMP_ROOT_DIR "/data/tombstones"
#define ESOC_SOC_NAME_NODE "esoc_name"
#define ESOC_SOC_LINK_NODE "esoc_link"
#define ESOC_SOC_PORT_NODE "esoc-dev"
#define MAX_PATH_LEN 255
#define MAX_SOC_NAME_LEN 30
#define MAX_SOC_LINK_NAME_LEN 30
#define MAX_SOC_PORT_NAME_LEN 30
#define NUM_MODEM_TYPES 4
#define SSR_BUS_ROOT "/sys/bus/msm_subsys/devices"

#ifdef STUBS_ONLY
#define UNUSED __attribute__ ((unused))
char *get_soc_name(char *esoc_dev_node UNUSED) { return NULL; }
char *get_soc_link(char *esoc_dev_node UNUSED) { return NULL; }
char *get_soc_ramdump_path(char *esoc_dev_node UNUSED) { return NULL; }
char *get_soc_port(char *esoc_dev_node UNUSED) { return NULL; }
int esoc_supported(char *esoc_node UNUSED) { return RET_FAILED; }
int esoc_framework_supported() { return RET_FAILED; }
int get_peripheral_state(char *name) {return RET_FAILED;}
static int get_subsys_string(char *path, char *buf,int buf_size)
{
	return RET_FAILED;
}
int get_system_info(struct dev_info *dev UNUSED)
{
	ALOGE("Using stub - libmdmdetect not supported");
	return RET_FAILED;
}
#else

const char *supported_modems[NUM_MODEM_TYPES] = {
	"QSC",
	"MDM9x15",
	"MDM9x25",
	"MDM9x35"
};
/*
 * Helper function to read and clean up data from the ssr sysfs
 * nodes
 */
static int get_subsys_string(char *path, char *buf,int buf_size)
{
	int fd = -1;
	int str_size = 0;
	if (!path) {
		ALOGE("Invalid path string");
		goto error;
	}
	if (!buf || buf_size <= 0) {
		ALOGE("Invalid buf/buf size passed");
		goto error;
	}
	fd = open(path, O_RDONLY);
	if (fd < 0) {
		ALOGE("Failed to open %s: %s",
				path,
				strerror(errno));
		goto error;
	}
	memset(buf, '\0', buf_size);
	if (read(fd, buf, buf_size - 1) < 0) {
		ALOGE("Failed to read %s: %s",
				path,
				strerror(errno));
		goto error;
	}
	str_size = strlen(buf);
	if (str_size == 0) {
		ALOGE("Read 0 length string from : %s",
				path);
		goto error;
	}
	if (buf[str_size -1] == '\n')
		buf[str_size -1] = '\0';
	close(fd);
	return RET_SUCCESS;
error:
	if (fd >= 0)
		close(fd);
	return RET_FAILED;
}

/*
 * Given the name of a node under the root esoc dir,return
 * a pointer to a buffer containing the nane of the ESOC it
 * represents. This buffer should be freed after use.
 */
char *get_soc_name(char *esoc_dev_node)
{
	char soc_name_node_path[MAX_PATH_LEN];
	char *soc_name = (char*)malloc(MAX_SOC_NAME_LEN * sizeof(char));
	if (!soc_name) {
		ALOGE("Failed to allocate memory for soc name");
		goto error;
	}
	memset((void*)soc_name, '\0', MAX_SOC_NAME_LEN * sizeof(char));
	snprintf(soc_name_node_path,
			sizeof(soc_name_node_path),
			"%s/%s/%s",
			ESOC_ROOT_DIR,
			esoc_dev_node,
			ESOC_SOC_NAME_NODE);
	if (get_subsys_string(soc_name_node_path,
			soc_name,
			MAX_SOC_NAME_LEN * sizeof(char)) !=
			RET_SUCCESS) {
		goto error;
	}
	return soc_name;
error:
	if(soc_name)
		free(soc_name);
	return NULL;
}

/*
 * Given the name of a node under the root esoc dir, return
 * a pointer to a buffer containing the name of the transport
 * link connecting the ESOC represented by the node
 * to the APQ. This buffer should be freed after use.
 */
char *get_soc_link(char *esoc_dev_node)
{
	char soc_link_node_path[MAX_PATH_LEN];
	char *soc_link = (char*)malloc(MAX_SOC_LINK_NAME_LEN * sizeof(char));
	if (!soc_link) {
		ALOGE("Failed to allocate memory for soc link");
		goto error;
	}
	memset((void*)soc_link, '\0', MAX_SOC_LINK_NAME_LEN * sizeof(char));
	snprintf(soc_link_node_path,
			sizeof(soc_link_node_path),
			"%s/%s/%s",
			ESOC_ROOT_DIR,
			esoc_dev_node,
			ESOC_SOC_LINK_NODE);
	if (get_subsys_string(soc_link_node_path,
				soc_link,
				MAX_SOC_LINK_NAME_LEN * sizeof(char)) !=
			RET_SUCCESS) {
		goto error;
	}
	return soc_link;
error:
	if (soc_link)
		free(soc_link);
	return NULL;
}

/*
 * Given the name of a node under the root esoc dir, return
 * a pointer to a buffer containing the location to where
 * ramdumps from that ESOC should be saved.The buffer should
 * be freed after use.
 */
char *get_soc_ramdump_path(char *esoc_dev_node)
{
	char *ramdump_path = NULL;
	char *temp = NULL;
	ramdump_path = (char*)malloc(MAX_PATH_LEN * sizeof(char));
	if (!ramdump_path) {
		ALOGE("Failed to allocate memory for ramdump path string");
		goto error;
	}
	temp = get_soc_name(esoc_dev_node);
	if (!temp) {
		ALOGE("Failed to get ramdump path");
		goto error;
	}
	snprintf(ramdump_path,
			(MAX_PATH_LEN * sizeof(char)),
			"%s/%s/",
			RAMDUMP_ROOT_DIR,
			temp);
	free(temp);
	return ramdump_path;
error:
	if (temp)
		free(temp);
	if (ramdump_path)
		free(ramdump_path);
	return NULL;
}

/*
 * Given the name of a node under the root esoc dir, return
 * a pointer to a buffer containing the path to the node via
 * which we communicate with the driver for this esoc.The buffer
 * should be freed after use.
 */
char* get_soc_port(char *esoc_dev_node)
{
	char soc_port_node_path[MAX_PATH_LEN];
	DIR *port_dir = NULL;
	struct dirent *de;
	int entry_found = 0;

	char *soc_port = (char*)malloc(MAX_PATH_LEN * sizeof(char));
	if (!soc_port) {
		ALOGE("Failed to allocate memory for soc port name");
		goto error;
	}
	memset((void*)soc_port, '\0', MAX_SOC_PORT_NAME_LEN * sizeof(char));
	snprintf(soc_port_node_path,
			sizeof(soc_port_node_path),
			"%s/%s/%s",
			ESOC_ROOT_DIR,
			esoc_dev_node,
			ESOC_SOC_PORT_NODE);
	port_dir = opendir(soc_port_node_path);
	if(!port_dir) {
		ALOGE("Failed to open port node directory: %s",
				strerror(errno));
		goto error;
	}
	//The esoc-dev directory will have only one file.The port via which
	// to talk to the mdm-driver for this esoc will be under
	//dev/file_name
	while((de = readdir(port_dir))) {
		if(de->d_name[0] == '.')
			continue;
		else {
			entry_found = 1;
			break;
		}
	}
	if (!entry_found) {
		ALOGE("Failed to get the name of the soc port: %s",
				strerror(errno));
		goto error;
	}
	snprintf(soc_port,
			(MAX_PATH_LEN * sizeof(char)),
			"/dev/%s",
			de->d_name);
	closedir(port_dir);
	return soc_port;
error:
	if (port_dir)
		closedir(port_dir);
	if (soc_port)
		free(soc_port);
	return NULL;
}

/*
 * Given the name of a node under the root esoc dir, return
 * true or false depending on if the esoc represented by the
 * node is one supported by mdm_helper.
 */
int esoc_supported(char *esoc_node)
{
	int i;
	char *soc_name = NULL;
	if(!esoc_node) {
		ALOGE("soc_name passed as NULL");
		goto error;
	}
	soc_name = get_soc_name(esoc_node);
	if(!soc_name) {
		ALOGE("Failed to read soc_name");
		goto error;
	}
	for(i = 0; i < NUM_MODEM_TYPES; i++)
	{
		if(!strncmp(soc_name, supported_modems[i],
					MAX_SOC_NAME_LEN)) {
			free(soc_name);
			return RET_SUCCESS;
		}
	}
error:
	if (soc_name)
		free(soc_name);
	return RET_FAILED;

}

/*
 * Return success/failure depending on if the esoc framework
 * is supported on the device.
 */
int esoc_framework_supported()
{
	struct stat esoc_dir_stat;
	if (stat(ESOC_ROOT_DIR, &esoc_dir_stat)) {
		ALOGI("ESOC framework not supported");
		return RET_FAILED;
	} else {
		return RET_SUCCESS;
	}
}

int get_esoc_details(struct mdm_info *dev, char *esoc_dev_name)
{
	char *temp;
	dev->type = MDM_TYPE_EXTERNAL;
	strlcpy(dev->esoc_node, esoc_dev_name, sizeof(dev->esoc_node));
	temp = get_soc_name(esoc_dev_name);
	if(!temp){
		ALOGE("Failed to get soc name for modem");
		goto error;
	}
	strlcpy(dev->mdm_name, temp, sizeof(dev->mdm_name));
	free(temp);
	temp = get_soc_port(esoc_dev_name);
	if(!temp) {
		ALOGE("Failed to get soc port for modem");
		goto error;
	}
	strlcpy(dev->drv_port, temp, sizeof(dev->drv_port));
	free(temp);
	temp = get_soc_link(esoc_dev_name);
	if (!temp) {
		ALOGE("Failed to get mdm link for modem");
		goto error;
	}
	strlcpy(dev->mdm_link, temp, sizeof(dev->mdm_link));
	free(temp);
	temp = get_soc_ramdump_path(esoc_dev_name);
	if (!temp) {
		ALOGE("Failed to get ram dump path for modem");
		goto error;
	}
	strlcpy(dev->ram_dump_path, temp, sizeof(dev->ram_dump_path));
	free(temp);
	temp = get_soc_ramdump_path(esoc_dev_name);
	if (!temp) {
		ALOGE("Failed to get ram dump path for modem");
		goto error;
	}
	strlcpy(dev->ram_dump_path, temp, sizeof(dev->ram_dump_path));
	free(temp);
	snprintf(dev->powerup_node,
			sizeof(dev->powerup_node),
			"/dev/subsys_%s",
			esoc_dev_name);
	return RET_SUCCESS;
error:
	if (temp)
		free(temp);
	return RET_FAILED;
}

int get_internal_modem_info(struct mdm_info *dev, char *dev_name)
{
	if (!dev || !dev_name) {
		ALOGE("Invalid argument passed to get_internal_modem_info");
		return RET_FAILED;
	}
	if (dev_name[0] != '\0' && dev_name[strlen(dev_name)-1] == '\n')
		dev_name[strlen(dev_name)-1] = '\0';
	dev->type = MDM_TYPE_INTERNAL;
	strlcpy(dev->esoc_node, "N/A", sizeof(dev->esoc_node));
	strlcpy(dev->drv_port, "N/A", sizeof(dev->drv_port));
	strlcpy(dev->mdm_name, dev_name, sizeof(dev->mdm_name));
	strlcpy(dev->mdm_link, LINK_SMD, sizeof(dev->mdm_link));
	snprintf(dev->powerup_node,
			sizeof(dev->powerup_node),
			"/dev/subsys_%s",
			dev_name);
	snprintf(dev->ram_dump_path,
			sizeof(dev->ram_dump_path),
			"%s/%s",
			RAMDUMP_ROOT_DIR, dev_name);
	return RET_SUCCESS;
}

int get_system_info(struct dev_info *dev)
{
	struct dirent *de;
	DIR *dir_esoc = NULL;
	DIR *dir_ssr_bus = NULL;
	char subsys_name_path[MAX_PATH_LEN];
	char subsys_name[MAX_NAME_LEN];
	if (!dev) {
		ALOGE("dev info paseed in as NULL");
		return RET_FAILED;
	}
	dev->num_modems = 0;
	if (esoc_framework_supported() == RET_SUCCESS) {
		dir_esoc = opendir(ESOC_ROOT_DIR);
		if (!dir_esoc) {
			ALOGE("Failed to open ESOC dir");
			goto error;
		}
		while((de = readdir(dir_esoc))) {
			if(de->d_name[0] == '.')
				continue;
			if(esoc_supported(de->d_name) == RET_SUCCESS) {
				if (get_esoc_details(&(dev->mdm_list[dev-> \
								num_modems]),
							de->d_name) !=
						RET_SUCCESS) {
					ALOGE("Failed to get ESOC info for %s",
							de->d_name);
					goto error;
				}
				dev->num_modems++;
				if (dev->num_modems > MAX_SUPPORTED_MDM) {
					ALOGE("Too many mdm's detected");
					goto error;
				}
			} else
				continue;
		}
		if (dev->num_modems == 0) {
			/*
			 * Not finding any esoc's is not necessarily a error.
			 * The api could just have been called on a non fusion
			 * target
			 */
			ALOGI("No supported ESOC's found");
		}
		closedir(dir_esoc);
		dir_esoc = NULL;
	}
	dir_ssr_bus = opendir(SSR_BUS_ROOT);
	if (!dir_ssr_bus) {
		ALOGE("Failed to open SSR root dir: %s", strerror(errno));
		goto error;
	}
	while((de = readdir(dir_ssr_bus))) {
		if(de->d_name[0] == '.')
			continue;
		snprintf(subsys_name_path,
				sizeof(subsys_name_path),
				"%s/%s/name",
				SSR_BUS_ROOT,
				de->d_name);
		if (get_subsys_string(subsys_name_path,
					subsys_name,
					sizeof(subsys_name)) !=
				RET_SUCCESS) {
			goto error;
		}
		if(!strncmp(subsys_name, "esoc", 4)){
			continue;
		}
		if(!strncmp(subsys_name, "modem", 5)) {
			ALOGI("Found internal modem: %s", subsys_name);
			if( get_internal_modem_info(&(dev->mdm_list[dev-> \
						num_modems]),
						subsys_name) != RET_SUCCESS) {
				ALOGE("Failed to get internal modem info");
				goto error;
			}
			dev->num_modems++;
		}
	}
	closedir(dir_ssr_bus);
	return RET_SUCCESS;
error:
	if (dir_esoc)
		closedir(dir_esoc);
	if (dir_ssr_bus)
		closedir(dir_ssr_bus);
	return RET_FAILED;
}

int get_peripheral_state(char *name)
{
	struct dev_info dev;
	struct mdm_info *mdm;
	char *registered_ssr_name;
	int i;
	int entry_located =0;
	int valid_name = 0;
	struct dirent *de = NULL;
	DIR *dir_esoc = NULL;
	DIR *dir_ssr = NULL;
	char *soc_name = NULL;
	char name_node_path[MAX_PATH_LEN] = {0};
	char state_node_path[MAX_PATH_LEN] ={0};
	char subsys_name[MAX_NAME_LEN];
	char subsys_state[30];
	if (!name)
		return RET_FAILED;

	if (get_system_info(&dev) != RET_SUCCESS)
		return RET_FAILED;
	//Is this a name that we are aware of(registered with ssr/esoc)
	for (i = 0; i < dev.num_modems; i++)
	{
		if (!strncmp(dev.mdm_list[i].mdm_name, name,
					sizeof(dev.mdm_list[i].mdm_name))) {
			valid_name = 1;
			mdm = &dev.mdm_list[i];
			break;
		}
	}
	if (!valid_name) {
		ALOGE("Unrecognised name passed to get_peripheral_state");
		goto error;
	}
	//is this a esoc node
	if (!strncmp(mdm->esoc_node, "esoc", 4)) {
		//FInd the name esocX with which it registers with SSR
		//esocX will be the esoc dir under /sys/bus/esoc/devices
		//that represents the device whose name has been passed
		//into this function
		dir_esoc = opendir(ESOC_ROOT_DIR);
		if (!dir_esoc) {
			ALOGE("Failed to open ESOC dir");
			goto error;
		}
		while((de = readdir(dir_esoc))) {
			if (de->d_name[0] == '.')
				continue;
			soc_name = get_soc_name(de->d_name);
			if (!soc_name) {
				ALOGE("Failed to get name for %s",
						de->d_name);
				goto error;
			}
			if (!strncmp(soc_name, name,
						MAX_NAME_LEN * sizeof(char))) {
				registered_ssr_name = de->d_name;
				entry_located = 1;
				break;
			}
		}

	} else {
		//Name belongs to a regular(internal) peripheral. In this case
		//the name registered with SSR will be the same as the name
		//passed in.
		registered_ssr_name = name;
		entry_located = 1;
	}
	if (!entry_located) {
		ALOGE("Failed to locate registered ssr name for %s", name);
		goto error;
	}
	//Now we iterate through the ssr sysfs
	//dirs(/sys/bus/msm_subsys/subsysX/name looking for a entry with name
	//registered_ssr_name. When we find it subsysX/state will tell us if
	//the peripheral is online or offline.
	dir_ssr = opendir(SSR_BUS_ROOT);
	if (!dir_ssr) {
		ALOGE("Failed to open ssr bus directory");
		goto error;
	}
	while((de = readdir(dir_ssr))) {
		if (de->d_name[0] == '.')
			continue;
		// /sys/bus/msm_subsys/subsysX/name
		snprintf(name_node_path,
				sizeof(name_node_path),
				"%s/%s/name",
				SSR_BUS_ROOT,
				de->d_name);
		if (get_subsys_string(name_node_path,
					subsys_name,
					sizeof(subsys_name)) !=
				RET_SUCCESS) {
			goto error;
		}
		if (!strncmp(subsys_name,
					registered_ssr_name,
					sizeof(subsys_name))) {
			ALOGI("Found match for %s at %s",
					name,
					de->d_name);
			// /sys/bus/msm_subsys/subsysX/state
			snprintf(state_node_path,
					sizeof(state_node_path),
					"%s/%s/state",
					SSR_BUS_ROOT,
					de->d_name);
			if (get_subsys_string(state_node_path,
						subsys_state,
						sizeof(subsys_state)) !=
					RET_SUCCESS) {
				goto error;
			}
			ALOGI("Peripheral %s: state is %s", name, subsys_state);
			if (dir_ssr)
				closedir(dir_ssr);
			if (dir_esoc)
				closedir(dir_esoc);
			if (soc_name)
				free(soc_name);
			if (!strncmp(subsys_state, "OFFLINE", 7))
				return PER_STATE_OFFLINE;
			else
				return PER_STATE_ONLINE;
		}
	}
	ALOGE("Failed to find a ssr entry for %s",
			registered_ssr_name);
error:
	if (dir_esoc)
		closedir(dir_esoc);
	if (dir_ssr)
		closedir(dir_ssr);
	if (soc_name)
		free(soc_name);
	return RET_FAILED;
}
#endif
