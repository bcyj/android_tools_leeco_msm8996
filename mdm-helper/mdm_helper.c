/*
 *mdm_helper: A module to monitor modem activities on fusion devices
 *
 *Copyright (C) 2012,2014 Qualcomm Technologies, Inc. All rights reserved.
 *                    Qualcomm Technologies Proprietary/GTDR
 *
 *All data and information contained in or disclosed by this document is
 *confidential and proprietary information of Qualcomm Technologies, Inc. and all
 *rights therein are expressly reserved.  By accepting this material the
 *recipient agrees that this material and the information contained therein
 *is held in confidence and in trust and will not be used, copied, reproduced
 *in whole or in part, nor its contents revealed in any manner to others
 *without the express written permission of Qualcomm Technologies, Inc.
 *
 *mdm_helper.c : Main implementation of mdm_helper
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <linux/msm_charm.h>
#include <linux/ioctl.h>
#include <cutils/properties.h>
#include <mdm_detect.h>
#include <mdm_helper_private.h>
#include <dirent.h>

static struct mdm_helper_drv mdm_helper;

/*
 * Get the ops structure for a particular esoc.
 */
static int get_soc_ops(char *esoc_dev_node,
		struct mdm_helper_ops *ops )
{
	char *soc_name = get_soc_name(esoc_dev_node);
	if (!soc_name) {
		ALOGE("Failed to populate ops structure");
		goto error;
	}
	if (!strncmp(soc_name, "MDM9x25",6) ||
			!strncmp(soc_name, "MDM9x35", 6)) {
		*ops = mdm9k_ops;
		return RET_SUCCESS;
	}
	ALOGE("Failed to get ops structure for %s", soc_name);
error:
	if (soc_name)
		free(soc_name);
	return RET_FAILED;
}

/*
 * Obtain the private data struct(if any) for the esoc.
 */
static int get_soc_private_data(char *esoc_dev_node,
		struct mdm_private_data **pdata)
{
	char *soc_name = get_soc_name(esoc_dev_node);
	char *link_name = get_soc_link(esoc_dev_node);
	if (!soc_name || !link_name) {
		ALOGE("Failed to populate private data.Invalid name/link");
		goto error;
	}
	if ((!strncmp(soc_name, "MDM9x25",6) &&
				!strncmp(link_name, LINK_HSIC, 4)) ||
			(!strncmp(soc_name, "MDM9x35",6) &&
			 !strncmp(link_name, LINK_HSIC, 4))) {
		ALOGI("Found private data for %s", soc_name);
		*pdata = &private_data_9x25_hsic;
		free(soc_name);
		free(link_name);
		return RET_SUCCESS;
	}
	ALOGI("No private data structure present for %s over link %s",
			soc_name,
			link_name);
	pdata = NULL;
	free(soc_name);
	free(link_name);
	return RET_SUCCESS;
error:
	if (soc_name)
		free(soc_name);
	if (link_name)
		free(link_name);
	return RET_FAILED;
}

static int load_configuration_via_esoc()
{
	int num_soc = 0;
	int count = 0;
	int i = 0;
	struct mdm_device *dev;
	struct dev_info devinfo;

	if (get_system_info(&devinfo) != RET_SUCCESS) {
		ALOGE("Failed to get device configuration.mdm-helper disabled");
		do {
			sleep(500000);
		}while(1);
	}
	if (devinfo.num_modems == 0) {
		ALOGE("No supported ESOC's found");
		goto error;
	}
	for (count = 0; count < devinfo.num_modems; count++) {
		if (devinfo.mdm_list[count].type == MDM_TYPE_EXTERNAL)
			num_soc++;
	}
	ALOGI("%d supported modem(s) found", num_soc);
	dev = (struct mdm_device*)malloc(sizeof(struct mdm_device) * num_soc);
	if (!dev) {
		ALOGE("Failed to allocate mdm_device structure");
		goto error;
	}
	ALOGI("Setting up mdm helper device structure");
	mdm_helper.num_modems = num_soc;
	mdm_helper.baseband_name = "n/a";
	mdm_helper.platform_name = "n/a";
	mdm_helper.dev = dev;
	count = 0;
	for (i = 0; i < devinfo.num_modems; i++) {
		if (devinfo.mdm_list[i].type == MDM_TYPE_EXTERNAL) {
			strlcpy(dev[count].esoc_node,
					devinfo.mdm_list[i].esoc_node,
					sizeof(dev[count].esoc_node));
			strlcpy(dev[count].mdm_name,
					devinfo.mdm_list[i].mdm_name,
					sizeof(dev[count].mdm_name));
			if (!strcmp(devinfo.mdm_list[i].mdm_link,
							LINK_HSIC_PCIE))
				strlcpy(dev[count].mdm_link,
					LINK_HSIC,
					sizeof(dev[count].mdm_link));
			else
				strlcpy(dev[count].mdm_link,
					devinfo.mdm_list[i].mdm_link,
					sizeof(dev[count].mdm_link));
			strlcpy(dev[count].mdm_port,
					devinfo.mdm_list[i].drv_port,
					sizeof(dev[count].mdm_port));
			dev[count].device_descriptor = 0;
			dev[count].required_action = MDM_REQUIRED_ACTION_NONE;
			if (strncmp(dev[count].mdm_name, "MDM", 3) == 0)
				strlcpy(dev[count].images_path,
						MDM9K_IMAGE_PATH,
						sizeof(dev[count].images_path));
			else
				strlcpy(dev[count].images_path,
						"N/A",
						sizeof(dev->images_path));
			strlcpy(dev[count].ram_dump_path,
					devinfo.mdm_list[i].ram_dump_path,
					sizeof(dev[count].ram_dump_path));
			if (get_soc_ops(dev[count].esoc_node,
						&(dev->ops)) != RET_SUCCESS) {
				ALOGE("Failed to get ops stucture for modem");
				goto error;
			}
			if (get_soc_private_data(dev[count].esoc_node,
						(struct mdm_private_data**) \
						&dev->private_data)
					!= RET_SUCCESS) {
				ALOGW("No private data located for %s",
						dev[count].mdm_name);
			}
			ALOGI("ESOC Details:\n\tName:%s\n\tPort:%s\n\tLink:%s",
					dev->mdm_name, dev->mdm_port,
					dev->mdm_link);
			count++;
		}
	}
	return RET_SUCCESS;
error:
	return RET_FAILED;
}

int mdm_helper_init()
{
	int i, rcode;
	if (esoc_framework_supported() != RET_SUCCESS) {
		ALOGE("ESOC framework not detected");
		return RET_FAILED;
	}
	rcode = load_configuration_via_esoc();
	if (rcode != RET_SUCCESS) {
		do {
			ALOGE("Failed to load device configuration");
			sleep(500);
		}while(1);
	} else
		ALOGI("Device configuration loaded");
	for (i = 0; i < mdm_helper.num_modems; i++) {
		if (!mdm_helper.dev[i].ops.power_up ||
				!mdm_helper.dev[i].ops.post_power_up) {
			ALOGE("power_up/post_power_up undefined for %s\n",
					mdm_helper.dev[i].mdm_name);
			return RET_FAILED;
		}
	}
	return RET_SUCCESS;
}

static void* modem_state_machine(void *arg)
{
	struct mdm_device *dev = (struct mdm_device *) arg;
	MdmHelperStates state = MDM_HELPER_STATE_POWERUP;
	ALOGI("Starting %s",dev->mdm_name);
	do {
		switch (state) {
		case MDM_HELPER_STATE_POWERUP:
			ALOGI("%s : switching state to POWERUP", dev->mdm_name);
			if (dev->ops.power_up(dev)
					!= RET_SUCCESS) {
				ALOGE("%s : Powerup failed", dev->mdm_name);
				state = MDM_HELPER_STATE_FAIL;
				break;
			}
			if (dev->required_action ==
					MDM_REQUIRED_ACTION_RAMDUMPS) {
				state = MDM_HELPER_STATE_RAMDUMP;
				dev->required_action =
					MDM_REQUIRED_ACTION_NONE;
			} else
				state = MDM_HELPER_STATE_POST_POWERUP;
			break;
		case MDM_HELPER_STATE_POST_POWERUP:
			ALOGI("%s : switching state to POST POWERUP/monitor",
					dev->mdm_name);
			if (dev->ops.post_power_up(dev) !=
					RET_SUCCESS) {
				ALOGE("%s : Post power_up failed",
						dev->mdm_name);
				state = MDM_HELPER_STATE_FAIL;
				break;
			}
			if (dev->required_action ==
					MDM_REQUIRED_ACTION_RAMDUMPS) {
				state = MDM_HELPER_STATE_RAMDUMP;
				dev->required_action =
					MDM_REQUIRED_ACTION_NONE;
				break;
			} else if (dev->required_action ==
					MDM_REQUIRED_ACTION_NORMAL_BOOT) {
				state = MDM_HELPER_STATE_POWERUP;
				dev->required_action =
					MDM_REQUIRED_ACTION_NONE;
				break;
			} else if (dev->required_action ==
					MDM_REQUIRED_ACTION_SHUTDOWN) {
				if (dev->ops.shutdown) {
					state = MDM_HELPER_STATE_SHUTDOWN;
				} else {
					ALOGI("%s:No shutdown function present",
							dev->mdm_name);
					state = MDM_HELPER_STATE_POST_POWERUP;
				}
				dev->required_action =
					MDM_REQUIRED_ACTION_NONE;
				break;
			}
			ALOGE("%s : post pwrup returned unsupported action:%d",
					dev->mdm_name,
					dev->required_action);
			state = MDM_HELPER_STATE_FAIL;
			break;
		case MDM_HELPER_STATE_RAMDUMP:
			ALOGI("%s : Switching state to RAMDUMP",
					dev->mdm_name);
			if (dev->ops.prep_for_ramdumps) {
				if (dev->ops.\
					prep_for_ramdumps(dev)
						!= RET_SUCCESS) {
					ALOGE("%s :prep_for_ramdump failed",
							dev->mdm_name);
					state = MDM_HELPER_STATE_FAIL;
					break;
				}
			}
			if (dev->ops.collect_ramdumps) {
				if (dev->ops.\
					collect_ramdumps(dev) !=
						RET_SUCCESS){
					ALOGE("%s :ramdump collect failed",
							dev->mdm_name);
					state = MDM_HELPER_STATE_FAIL;
					break;
				}
			} else {
				ALOGE("%s :No collect_ramdump function defined",
						dev->mdm_name);
				state = MDM_HELPER_STATE_FAIL;
				break;
			}
			if (dev->ops.post_ramdump_collect) {
				if (dev->ops.\
					post_ramdump_collect(dev)
						!= RET_SUCCESS) {
					ALOGE("%s :post ramdump collect failed",
							dev->mdm_name);
					state = MDM_HELPER_STATE_FAIL;
					break;
				}
			}
			if ((dev->required_action !=
					MDM_REQUIRED_ACTION_NORMAL_BOOT) &&
					(dev->required_action !=
					MDM_REQUIRED_ACTION_NONE) &&
					(dev->required_action !=
					 MDM_REQUIRED_ACTION_SHUTDOWN)) {
				ALOGE("%s:Invalid state trans req:state:%d",
						dev->mdm_name,
						dev->required_action);
			}
			if (dev->required_action ==
					MDM_REQUIRED_ACTION_SHUTDOWN)
				state = MDM_HELPER_STATE_SHUTDOWN;
			else
				state = MDM_HELPER_STATE_POWERUP;
			dev->required_action = MDM_REQUIRED_ACTION_NONE;
			break;
		case MDM_HELPER_STATE_REBOOT:
			ALOGI("%s : Normal reboot request", dev->mdm_name);
			if (!dev->ops.reboot) {
				ALOGE("%s : Reboot function not defined",
						dev->mdm_name);
				state = MDM_HELPER_STATE_FAIL;
				break;
			}
			if (dev->ops.reboot(dev) != RET_SUCCESS) {
				state = MDM_HELPER_STATE_FAIL;
				break;
			}
			state = MDM_HELPER_STATE_POST_POWERUP;
			break;
		case MDM_HELPER_STATE_SHUTDOWN:
			ALOGI("%s: Handling shutdown request",
					dev->mdm_name);
			if (!dev->ops.shutdown) {
				ALOGE("%s: No shutdown function defined",
						dev->mdm_name);
				state = MDM_HELPER_STATE_FAIL;
				break;
			}
			if (dev->ops.shutdown(dev) != RET_SUCCESS) {
				ALOGE("%s: Shutdown function failed",
						dev->mdm_name);
				state = MDM_HELPER_STATE_FAIL;
				break;
			}
			state = MDM_HELPER_STATE_POWERUP;
			break;
		case MDM_HELPER_STATE_FAIL:
			ALOGE("%s : Reached failed state. exiting",
					dev->mdm_name);
			if (dev->ops.failure_cleanup) {
				ALOGI("%s : Calling cleanup function",
						dev->mdm_name);
				dev->ops.failure_cleanup(dev);
			} else
				ALOGI("%s : No cleanup function defined",
						dev->mdm_name);
			return 0;
		default:
			ALOGE("%s : Reached unknown state %d.exiting",
					dev->mdm_name,
					state);
			state = MDM_HELPER_STATE_FAIL;
			break;
		}
	} while(1);
}

static void* modem_proxy_routine(void *arg)
{
	struct mdm_device *dev = (struct mdm_device*)arg;
	char powerup_node[MAX_PATH_LEN];
	int fd;
	snprintf(powerup_node, sizeof(powerup_node),
			"/dev/subsys_%s",
			dev->esoc_node);
	fd = open(powerup_node, O_RDONLY);
	if (fd < 0) {
		ALOGE("%s: Proxy thread failed to open esoc node: %s",
				dev->mdm_name,
				powerup_node);
	}
	do {
		sleep(50000);
	} while(1);
	return NULL;
}

int main(int argc, char *argv[])
{
	int i;
	int rcode = 0;
	int valid_argument = 0;
	pthread_t *tid = NULL;
	void *(*mdm_routine) (void *);

	if (mdm_helper_init()) {
		ALOGE("Initializaion failed");
		return 0;
	}

	if (argc > 1) {
		ALOGI("Standalone mode");
		for (i = 0; i < mdm_helper.num_modems; i++)
		{
			if (!strncmp(argv[1],mdm_helper.dev[i].mdm_name,
						MDM_NAME_LEN)) {
				valid_argument = 1;
				break;
			}
		}
		if (!valid_argument) {
			ALOGE("Unrecognised modem :%s",argv[1]);
			return RET_FAILED;
		}
		if (mdm_helper.dev[i].ops.standalone_mode)
			return mdm_helper.dev[i].ops.\
				standalone_mode(&mdm_helper.dev[i],
					&argv[1]);
		else
			ALOGE("No standalone function defined");
		return RET_FAILED;
	}
	ALOGI("Starting MDM helper");
	tid = (pthread_t*) malloc(mdm_helper.num_modems * sizeof(pthread_t));
	if (!tid) {
		ALOGE("Failed to create modem threads : %s",strerror(errno));
		return RET_FAILED;
	}

	if (!strcmp(basename(argv[0]), "mdm_helper_proxy"))
		mdm_routine = &modem_proxy_routine;
	else
		mdm_routine = &modem_state_machine;

	for (i = 0; i < mdm_helper.num_modems; i++) {
		ALOGI("Creating thread for %s",mdm_helper.dev[i].mdm_name);
		rcode = pthread_create(&tid[i], NULL,
				mdm_routine,
				(void*)(&(mdm_helper.dev[i])));
		if (rcode) {
			ALOGE("Failed to create thread for %s\n",
					mdm_helper.dev[i].mdm_name);
			return RET_FAILED;
		}
	}
	do {
		sleep(250000);
	} while(1);
	return 0;
}
