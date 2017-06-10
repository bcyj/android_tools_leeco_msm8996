/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 */

#include "mdm9k_esoc.h"

static int configure_flashless_boot_dev(struct mdm_device *dev, int mode);
static int mdm_hsic_peripheral_cmd(struct mdm_device *dev, int cmd);
static void* override_thread(void *arg);
static int global_power_fd = -1;

struct mdm_private_data private_data_9x25_hsic = {
	.reset_device_before_ramdumps = 1,
	.peripheral_cmd = mdm_hsic_peripheral_cmd,
	.flashless_boot_device = "/dev/ks_hsic_bridge",
	.efs_sync_device = "/dev/efs_hsic_bridge",
	.transport_bind_node = "/sys/bus/platform/drivers/xhci_msm_hsic/bind",
	.transport_unbind_node = "/sys/bus/platform/drivers/xhci_msm_hsic/unbind",
	.transport_bind_command = "msm_hsic_host",
	.transport_unbind_command = "msm_hsic_host",
	.ram_dump_file_prefix = NULL,
	.efs_sync_file_prefix = "mdm1",
	.efs_ks_pid = 0,
	.efs_file_dir = "/dev/block/bootdevice/by-name/",
	.partition_list = {
		{
			{
				{16, TEMP_FILE_DIR "m9k0efs1.tmp"},
				"efs1.mbn",
				TEMP_FILE_DIR "m9k0efs1bin.tmp"
			},
			NULL,
			0,
		},
		{
			{
				{17, TEMP_FILE_DIR "m9k0efs2.tmp"},
				"efs2.mbn",
				TEMP_FILE_DIR "m9k0efs2bin.tmp"
			},
			NULL,
			0,
		},
		{
			{
				{20, TEMP_FILE_DIR "m9k0efs3.tmp"},
				"efs3.mbn",
				TEMP_FILE_DIR "m9k0efs3bin.tmp"
			},
			NULL,
			0,
		},
	},
	.other_prepend_images = {
		{
			{29, TEMP_FILE_DIR "m9k0acdb.tmp"},
			NULL,
			NULL,
		},
	},
	.image_list = {
		{21, "sbl1.mbn"},
		{25, "tz.mbn"},
		{30, "sdi.mbn"},
		{23, "rpm.mbn"},
		{31, "mba.mbn"},
		{8, "qdsp6sw.mbn"},
		{28, "dsp2.mbn"},
		{6, "apps.mbn"},
		{16, "/dev/block/bootdevice/by-name/mdm1m9kefs1"},
		{17, "/dev/block/bootdevice/by-name/mdm1m9kefs2"},
		{20, "/dev/block/bootdevice/by-name/mdm1m9kefs3"},
		{29, "acdb.mbn"},
		{0, NULL},
	},
};

int mdm9k_powerup(struct mdm_device *dev)
{
	static int power_on_count = 0;
	int status = 0;
	u32 request;
	u32 cmd;
	int rcode;
	pthread_t tid;
	int wait_count = 0;
	int is_warm_boot, fd;
	char boot_reason_buf[2];
	int debug_mode_enabled = 0;
	int image_xfer_complete = 0;
	char debug_mode_string[PROPERTY_VALUE_MAX];
	struct mdm_private_data *pdata;

	property_get(DEBUG_MODE_PROPERTY, debug_mode_string, "false");
	if (!strncmp(debug_mode_string, "true", PROPERTY_VALUE_MAX)) {
		ALOGI("%s: debug mode enabled", dev->mdm_name);
		debug_mode_enabled = 1;
	}
	fd = open(BOOT_TYPE_NODE, O_RDONLY);
	if (fd < 0) {
		ALOGE("Failed to open boot type node: %s", strerror(errno));
		is_warm_boot = 0;
	} else {
		memset(boot_reason_buf, '\0', sizeof(boot_reason_buf));
		if (read(fd, (void*)boot_reason_buf, sizeof(char)) < 0) {
			ALOGE("Failed to read boot type node: %s",
					strerror(errno));
			is_warm_boot = 0;
		} else {
			is_warm_boot = !atoi(boot_reason_buf);
			if (is_warm_boot)
				ALOGI("Warm boot detected");
			else
				ALOGI("Cold boot detected");
		}
		close(fd);
	}
	if (!dev) {
		ALOGE("%s: Invalid device structure passed to %s",
				dev->mdm_name,
				__func__);
		goto error;
	}
	if (!dev->private_data) {
		ALOGE("%s: Private data not found", dev->mdm_name);
		goto error;
	}
	pdata = (struct mdm_private_data *)dev->private_data;
	char *efs_sync_args[] = {
		KS_PATH,"-m",
		"-p", pdata->efs_sync_device,
		"-w", pdata->efs_file_dir,
		"-t", "-1",
		"-l",
		(pdata->efs_sync_file_prefix == NULL) ? NULL : "-g",
		pdata->efs_sync_file_prefix,
		NULL };
	if (power_on_count == 0) {
		ALOGI("%s: Initializing environment", dev->mdm_name);
		dev->device_descriptor = open(dev->mdm_port,
				O_RDONLY | O_NONBLOCK);
		if (dev->device_descriptor < 0) {
			ALOGE("%s: Failed to open mdm device node: %s",
					dev->mdm_name,
					strerror(errno));
			goto error;
		}
		if (ioctl(dev->device_descriptor, ESOC_REG_REQ_ENG) < 0) {
			ALOGE("%s: Failed to set thread as request engine",
					dev->mdm_name);
			goto error;
		}
		if (is_warm_boot) {
			rcode = pthread_create(&tid, NULL, &override_thread,
					(void*)dev);
			if (rcode) {
				ALOGE("Failed to create override thread.\
						REBOOTING now");
				android_reboot(ANDROID_RB_RESTART, 0, 0);
			}
		}
		if (ioctl(dev->device_descriptor,
					ESOC_WAIT_FOR_REQ, &request) < 0) {
			ALOGE("%s: REQ_ENG: ESOC_WAIT_FOR_REQ ioctl failed",
					dev->mdm_name);
			goto error;
		}
		if (request != ESOC_REQ_IMG) {
			ALOGE("Expecting ESOC_REQ_IMG. Recieved : %u", request);
			goto error;
		}
	}
	//setup flashless boot device
	if (configure_flashless_boot_dev(dev, MODE_BOOT) != RET_SUCCESS) {
		ALOGE("%s: Link setup failed", dev->mdm_name);
		goto error;
	}
	if (debug_mode_enabled) {
		ALOGI("%s:mdm boot paused.set persist.mdm_boot_debug to resume",
				dev->mdm_name);
		do {
			property_get(DEBUG_MODE_PROPERTY,
					debug_mode_string,
					"false");
			usleep(1000000);
		} while(strncmp(debug_mode_string, "resume",
					6));
		property_set(DEBUG_MODE_PROPERTY, "true");
		ALOGI("%s: resuming mdm boot", dev->mdm_name);
	}
	if (!strncmp(debug_mode_string, "resume_rma", 10)) {
		ALOGI("rma boot...reinitializing boot link");
		if (configure_flashless_boot_dev(dev, MODE_BOOT) !=
				RET_SUCCESS) {
			ALOGE("%s: Link setup failed", dev->mdm_name);
			goto error;
		}
	}
	if (power_on_count == 0) {
		//Modify this function to take the link type as a argument
		if (LoadSahara(dev, "") != RET_SUCCESS) {
			ALOGE("%s: Failed to load image/collect logs",
					dev->mdm_name);
			goto error;
		}
		if (is_warm_boot) {
			ALOGI("%s: Warm boot: triggering EXPECTED reset",
					dev->mdm_name);
			android_reboot(ANDROID_RB_RESTART, 0, 0);
		}
	} else {
		if (LoadSahara(dev, "-i") != RET_SUCCESS) {
			ALOGE("%s: Failed to load images", dev->mdm_name);
			goto error;
		}
	}
	cmd = ESOC_IMG_XFER_DONE;
	if (ioctl(dev->device_descriptor, ESOC_NOTIFY, &cmd) < 0) {
		ALOGE("%s: Failed to send IMG_XFER_DONE notification",
				dev->mdm_name);
		goto error;
	}
	image_xfer_complete = 1;
	//Wait for MDM2AP_STATUS to go high.
	do {
		if(ioctl(dev->device_descriptor,
					ESOC_GET_STATUS,
					&status) < 0) {
			ALOGE("%s: ESOC_GET_STATUS ioctl failed",
					dev->mdm_name);
			goto error;
		}
		if(status == 1) {
			ALOGI("%s: MDM2AP_STATUS is now high", dev->mdm_name);
			break;
		}
		ALOGI("%s: Waiting for mdm boot", dev->mdm_name);
		usleep(1000000);
		wait_count++;
	} while(wait_count <= MAX_STATUS_UP_CHECKS);
	if (wait_count >= MAX_STATUS_UP_CHECKS) {
		ALOGE("%s: MDM did not set MDM2AP_STATUS high");
		goto error;
	}
	//If the link is HSIC then we need to reset the link and send
	//the driver the status up notification
	if(!strncmp(dev->mdm_link, LINK_HSIC, 5)) {
		//Configure the hsic to enumerate as a efs sync device
		if (configure_flashless_boot_dev(dev, MODE_RUNTIME) !=
				RET_SUCCESS) {
			ALOGE("%s: Link setup failed", dev->mdm_name);
			goto error;
		}
	}
	//Wait for efs sync port to appear
	if(pdata && pdata->efs_sync_device) {
		if(WaitForCOMport(pdata->efs_sync_device,
					NUM_COMPORT_RETRIES, 0) !=
				RET_SUCCESS) {
			ALOGE("%s: Could not detect EFS sync port",
				dev->mdm_name);
			goto error;
		}
	} else {
		ALOGE("%s: No efs_sync_device specified for target",
				dev->mdm_name);
		goto error;
	}
	pdata->efs_ks_pid = fork();
	if (pdata->efs_ks_pid  < 0) {
		ALOGE("%s: Failed to create efs sync process", dev->mdm_name);
		goto error;
	} else if(pdata->efs_ks_pid == 0) {
		//exec efs sync process
		if (global_power_fd > 0)
			close(global_power_fd);
		if (execve(KS_PATH, efs_sync_args, NULL) < 0) {
			ALOGE("%s: Failed to exec KS process for efs sync",
					dev->mdm_name);
			_exit(127);
		}
	}
	cmd = ESOC_BOOT_DONE;
	if (ioctl(dev->device_descriptor, ESOC_NOTIFY, &cmd) < 0) {
		ALOGE("%s: Failed to send ESOC_BOOT_DONE notification",
				dev->mdm_name);
		return RET_FAILED;
	}
	power_on_count++;
	return RET_SUCCESS;
error:
	if (!image_xfer_complete)
		cmd = ESOC_IMG_XFER_FAIL;
	else
		cmd = ESOC_BOOT_FAIL;
	if (dev) {
		if (ioctl(dev->device_descriptor, ESOC_NOTIFY, &cmd) < 0)
			ALOGE("%s:Failed to send IMG_XFER_FAIL notification",
					dev->mdm_name);
	}
	return RET_FAILED;
}

/*
 * The shutdown function here is technically a post
 * shutdown cleanup. The notification will only be
 * recieved by mdm-helper once the 9k has finished
 * powering down. This function essentially exists
 * to clean up the HSIC link.
 */
int mdm9k_shutdown(struct mdm_device *dev)
{
	struct mdm_private_data *pdata = NULL;
	if (!dev) {
		ALOGE("Invalid device structure passed in");
		return RET_FAILED;
	}
	pdata = (struct mdm_private_data*)dev->private_data;
	if (!pdata) {
		ALOGE("private data is NULL");
		return RET_FAILED;
	}
	if(!strncmp(dev->mdm_link, LINK_HSIC, 5)) {
		if (pdata->peripheral_cmd) {
			ALOGI("%s: %s: Initiating HSIC unbind",
					dev->mdm_name,
					__func__);
			pdata->peripheral_cmd(dev,
					PERIPHERAL_CMD_UNBIND);
		}
	}
	mdm9k_monitor(dev);
	if (dev->required_action != MDM_REQUIRED_ACTION_NORMAL_BOOT) {
		ALOGE("%s: Invalid request recieved.Expected Image request",
				dev->mdm_name);
		return RET_FAILED;
	}
	dev->required_action = MDM_REQUIRED_ACTION_NONE;
	return RET_SUCCESS;
}

int mdm9k_monitor(struct mdm_device *dev)
{
	u32 request = 0;
	if (!dev) {
		ALOGE("%s: Invalid device structure passed to %s",
				dev->mdm_name,
				__func__);
		return RET_FAILED;
	}
	ALOGI("%s: Monitoring mdm", dev->mdm_name);
	if (ioctl(dev->device_descriptor,
				ESOC_WAIT_FOR_REQ, &request) < 0) {
		ALOGE("%s: ESOC_WAIT_FOR_REQ ioctl failed",
				dev->mdm_name);
		return RET_FAILED;
	}
	switch(request) {
	case ESOC_REQ_DEBUG:
		ALOGI("%s: Recieved request for ramdump collection",
				dev->mdm_name);
		dev->required_action = MDM_REQUIRED_ACTION_RAMDUMPS;
		break;
	case ESOC_REQ_IMG:
		ALOGI("%s: Recieved request to transfer images",
				dev->mdm_name);
		dev->required_action = MDM_REQUIRED_ACTION_NORMAL_BOOT;
		break;
	case ESOC_REQ_SHUTDOWN:
		ALOGI("%s: Recieved shutdown request",
				dev->mdm_name);
		dev->required_action = MDM_REQUIRED_ACTION_SHUTDOWN;
		break;
	default:
		ALOGE("%s: Unknown request recieved: %u", dev->mdm_name,
				request);
		break;
	}
	return RET_SUCCESS;
}

int mdm9k_ramdump_collect(struct mdm_device *dev)
{
	struct mdm_private_data *pdata = NULL;
	u32 cmd = ESOC_DEBUG_DONE;
	int dump_collection_complete = 0;
	if (!dev) {
		ALOGE("%s: Invalid device structure passed to %s",
				dev->mdm_name,
				__func__);
		return RET_FAILED;
	}
	dev->required_action = MDM_REQUIRED_ACTION_RAMDUMPS;
	pdata = (struct mdm_private_data*)dev->private_data;
	if (configure_flashless_boot_dev(dev, MODE_RAMDUMP) != RET_SUCCESS) {
		ALOGE("%s: Failed to configure hsic port for collecting dumps",
				dev->mdm_name);
		goto error;
	}
	if (LoadSahara(dev, "-m") != RET_SUCCESS) {
		ALOGE("%s: Failed to collect dumps", dev->mdm_name);
		goto error;
	}
	if (ioctl(dev->device_descriptor, ESOC_NOTIFY,
				&cmd) < 0) {
		ALOGE("%s: :%s: Failed to send debug done notification",
				dev->mdm_name,
				__func__);
		goto error;
	}
	dump_collection_complete = 1;
	if ((mdm9k_monitor(dev) != RET_SUCCESS) &&
			dev->required_action !=
			MDM_REQUIRED_ACTION_NORMAL_BOOT) {
		ALOGE("%s: Monitor function failed/requested invalid action:%d",
				dev->mdm_name,
				dev->required_action);
		goto error;
	}
	return RET_SUCCESS;
error:
	cmd = ESOC_DEBUG_FAIL;
	if (!dump_collection_complete) {
		if (ioctl(dev->device_descriptor, ESOC_NOTIFY,
					&cmd) < 0) {
			ALOGE("%s: :%s: Failed to reset mdm",
					dev->mdm_name,
					__func__);
		}
	}
	return RET_FAILED;
}

int mdm9k_cleanup(struct mdm_device *dev)
{
	char debug_mode_string[PROPERTY_VALUE_MAX];
	int debug_mode_enabled = 0;
	property_get(DEBUG_MODE_PROPERTY, debug_mode_string, "false");
	if (!strncmp(debug_mode_string, "true", PROPERTY_VALUE_MAX)) {
		ALOGI("%s: debug mode enabled", dev->mdm_name);
		debug_mode_enabled = 1;
	}
	if (debug_mode_enabled) {
		do {
			ALOGE("%s: Reached failed state with dbg mode set",
					dev->mdm_name);
			usleep(1000000000);
		} while(1);
	}
	ALOGE("%s: mdm-helper reached fail state", dev->mdm_name);
	return RET_FAILED;
}

static int configure_flashless_boot_dev(struct mdm_device *dev, int mode)
{
	struct mdm_private_data *pdata = NULL;
	int cmd = ESOC_IMG_XFER_RETRY;
	int req = 0;
	int rcode, fd;
	int i;
	if (!dev) {
		ALOGE("Device structure passed in as NULL");
		return RET_FAILED;
	}
	pdata = (struct mdm_private_data*)dev->private_data;
	if(!pdata) {
		ALOGE("%s: %s: Private data is null",
				dev->mdm_name, __func__);
		return RET_FAILED;
	}
	if(!strncmp(dev->mdm_link, LINK_HSIC, 5)) {
		if (mode == MODE_BOOT || mode == MODE_RAMDUMP) {
			ALOGI("%s: Setting up %s boot link",
					dev->mdm_name,
					dev->mdm_link);
			for (i = 0; i < NUM_LINK_RETRIES; ++i) {
				if (pdata->peripheral_cmd) {
					ALOGI("%s: %s: Initiating HSIC unbind",
							dev->mdm_name,
							__func__);
					pdata->peripheral_cmd(dev,
							PERIPHERAL_CMD_UNBIND);
				}
				if (ioctl(dev->device_descriptor, ESOC_NOTIFY,
							&cmd) < 0) {
					ALOGE("%s: :%s: Failed to reset mdm",
							dev->mdm_name,
							__func__);
					return RET_FAILED;
				}
				if (ioctl(dev->device_descriptor,
							ESOC_WAIT_FOR_REQ,
							&req) < 0) {
					ALOGE("%s: %s:wait for image xfer fail",
							dev->mdm_name,
							__func__);
					return RET_FAILED;
				}
				if (req != ESOC_REQ_IMG &&
						(mode != MODE_RAMDUMP)) {
					ALOGE("%s: %s: Unnexpected request: %d",
							dev->mdm_name,
							__func__,
							req);
					continue;
				}
				usleep(500000);
				if (pdata->peripheral_cmd) {
					ALOGI("%s: %s: Initiating HSIC bind",
							dev->mdm_name,
							__func__);
					pdata->peripheral_cmd(dev,
							PERIPHERAL_CMD_BIND);
				}
				rcode = WaitForCOMport(
						pdata->flashless_boot_device,
						5, 0);
				if (rcode == RET_SUCCESS)
					break;
			}
			if (rcode != RET_SUCCESS) {
				ALOGE("%s: %s: Failed to setup HSIC link",
						dev->mdm_name,
						__func__);
				return RET_FAILED;
			}
		} else if (mode == MODE_RUNTIME) {
			ALOGI("%s: Setting up %s link for efs_sync",
					dev->mdm_name,
					dev->mdm_link);
			peripheral_reset(dev);
			ALOGI("%s: Sending boot status notification to HSIC",
					dev->mdm_name);
			for (i = 0; i < MAX_HSIC_NOTIFICATION_RETRIES; i++) {
				fd = open(HSIC_NOTIFICATION_NODE, O_WRONLY);
				if (fd < 0)
				{
					if (i >= MAX_HSIC_NOTIFICATION_RETRIES \
							-1) {
						ALOGE("%s: node open fail: %s",
								dev->mdm_name,
								strerror(errno)
								);
						return RET_FAILED;
					}
					usleep(100000);
				} else {
					rcode = write(fd, "1", sizeof(char));
					close(fd);
					if (rcode < 0) {
						ALOGE("%s:node write err: %s",
								dev->mdm_name,
								strerror(errno)
								);
						return RET_FAILED;
					} else {
						ALOGI("%s: Notification sent",
								dev->mdm_name);
						break;
					}
				}
			}
		}
	} else {
		ALOGE("%s: Link %s not supported by mdm-helper",
				dev->mdm_name,
				dev->mdm_link);
		return RET_FAILED;
	}
	return RET_SUCCESS;
}

int LoadSahara(struct mdm_device *dev, char* options)
{
	char command_string[2048];
	char temp_string[256];
	struct mdm_private_data *data_9x15 =
		(struct mdm_private_data *) dev->private_data;
	int i;
	int rcode;
	ALOGI("%s: Loading Sahara images", dev->mdm_name);
	if(WaitForCOMport(data_9x15->flashless_boot_device, 5, 0) ==
			RET_FAILED) {
		ALOGE("%s: Could not find flashless boot port", dev->mdm_name);
		return RET_FAILED;
	}
	if (snprintf(command_string,
				sizeof(command_string),
				"%s %s -w %s -p %s -r %d",
				KS_PATH,
				options,
				dev->ram_dump_path,
				data_9x15->flashless_boot_device,
				RAM_DUMP_IMAGE) > sizeof(command_string)) {
		ALOGE("%s: String was truncated.", dev->mdm_name);
		return RET_FAILED;
	}
	if (data_9x15->ram_dump_file_prefix != NULL) {
		strlcat(command_string, " -g ", sizeof(command_string));
		strlcat(command_string, data_9x15->ram_dump_file_prefix,
				sizeof(command_string));
	}
	for (i = 0; data_9x15->image_list[i].filename != NULL; i++) {
		if (snprintf(temp_string,
					sizeof(temp_string),
					" -s %d:%s%s",
					data_9x15->image_list[i].image_id,
					(data_9x15->image_list[i].filename[0] ==
					 '/') ? "" : dev->images_path,
					data_9x15->image_list[i].filename) >
				sizeof(temp_string)) {
			ALOGE("%s: String was truncated.", dev->mdm_name);
			return RET_FAILED;
		}
		strlcat(command_string, temp_string, sizeof(command_string));
	}
	if (data_9x15->partition_list[0].partition != NULL) {
		for (i = 0; i < NUM_EFS_PARTITIONS; i++) {
			if (snprintf(temp_string,
						sizeof(temp_string),
						" -s %d:%s",
						data_9x15->partition_list[i].\
						header_info.file_details.\
						image_id,
						data_9x15->partition_list[i].\
						header_info.file_details.\
						filename) >
					sizeof(temp_string)) {
				ALOGE("%s: String was truncated.",
						dev->mdm_name);
				return RET_FAILED;
			}
			strlcat(command_string, temp_string,
					sizeof(command_string));
		}
	}
	if (data_9x15->other_prepend_images[0].header_file != NULL) {
		for (i = 0; i < NUM_OTHER_HEADER_PREPEND_FILES; i++) {
			if (snprintf(temp_string,
						sizeof(temp_string),
						" -s %d:%s",
						data_9x15->\
						other_prepend_images[i].\
						file_details.image_id,
						data_9x15->\
						other_prepend_images[i].\
						file_details.filename) >
					sizeof(temp_string)) {
				ALOGE("%s: String was truncated.",
						dev->mdm_name);
				return RET_FAILED;
			}
			strlcat(command_string, temp_string,
					sizeof(command_string));
		}
	}
	ALOGI("%s: Running '%s'", dev->mdm_name, command_string);
	rcode  = system(command_string);
	if(rcode != 0) {
		if (rcode == 1280) {
			ALOGE("%s: ERROR: RAM dumps were forced unexpectedly",
					dev->mdm_name);
		} else {
			ALOGE("%s: ERROR: ks return code was %d",
					dev->mdm_name, rcode);
		}
		return RET_FAILED;
	}
	ALOGE("%s: Sahara transfer completed successfully", dev->mdm_name);
	return RET_SUCCESS;
}

int WaitForCOMport(char *DevNode, int max_retries, int attempt_read)
{
	struct stat status_buf;
	int i;

	ALOGI("Testing if port \"%s\" exists", DevNode);
	for (i = 0; i < max_retries && stat(DevNode, &status_buf) < 0; i++) {
		ALOGE("Couldn't find \"%s\", %i of %i", DevNode, i+1,
				max_retries);
		usleep(DELAY_BETWEEN_RETRIES_MS * 1000);
	}
	if (i == max_retries) {
		ALOGE("'%s' was not found", DevNode);
		return RET_FAILED;
	}
	if (attempt_read) {
		FILE *fd;
		ALOGI("Attempting to open port \"%s\" for reading", DevNode);
		for (i=0; i<NUM_COM_PORT_RETRIES && (fd = fopen(DevNode,"r"))==\
				NULL; i++) {
			ALOGE("Couldn't open \"%s\", %i of %i", DevNode, i+1,
					NUM_COM_PORT_RETRIES);
			usleep(DELAY_BETWEEN_RETRIES_MS*1000);
		}
		if (i == NUM_COM_PORT_RETRIES) {
			ALOGE("'%s' could not be opened for reading", DevNode);
			return RET_FAILED;
		}
		fclose(fd);
	}
	return RET_SUCCESS;
}

void peripheral_reset(struct mdm_device *dev)
{
	struct mdm_private_data *pdata = NULL;

	if (!dev)
		return;
	pdata = (struct mdm_private_data*)dev->private_data;
	if (!pdata->peripheral_cmd)
		return;
	pdata->peripheral_cmd(dev, PERIPHERAL_CMD_UNBIND);
	usleep (10000);
	pdata->peripheral_cmd(dev, PERIPHERAL_CMD_BIND);
}

static int mdm_hsic_peripheral_cmd(struct mdm_device *dev, int nCmd)
{
	struct mdm_private_data *pdata = NULL;
	int fd = -1;
	if (!dev) {
		ALOGE("No device specified.");
		return RET_FAILED;
	}
	pdata = (struct mdm_private_data *)dev->private_data;
	if (!pdata) {
		ALOGE("No private data.");
		return RET_FAILED;
	}
	if (!pdata->transport_bind_node ||
			!pdata->transport_unbind_node ||
			!pdata->transport_bind_command ||
			!pdata->transport_unbind_command) {
		ALOGE("Bind/Unbind not supported on this target");
		return 0;
	}
	switch (nCmd) {
	case PERIPHERAL_CMD_BIND:
		fd = open(pdata->transport_bind_node, O_WRONLY);
		if (fd < 0) {
			ALOGE("Failed to open bind node : %s", strerror(errno));
			goto error;
		}
		if(write(fd, pdata->transport_bind_command,
				strlen(pdata->transport_bind_command)) < 0) {
			ALOGE("Failed to write to bind node: %s",
					strerror(errno));
			goto error;
		}
		break;
	case PERIPHERAL_CMD_UNBIND:
		fd = open(pdata->transport_unbind_node, O_WRONLY);
		if (fd < 0) {
			ALOGE("Failed to open bind node : %s", strerror(errno));
			goto error;
		}
		if(write(fd, pdata->transport_unbind_command,
				strlen(pdata->transport_unbind_command)) < 0) {
			ALOGE("Failed to write to bind node: %s",
					strerror(errno));
			goto error;
		}
		break;
	default:
		ALOGE("Unrecognised command.");
		goto error;
	}
	close(fd);
	return 0;
error:
	if (fd >= 0)
		close(fd);
	return -1;
}

static void* override_thread(void *arg)
{
	struct mdm_device *dev = (struct mdm_device*)arg;
	char powerup_node[MAX_PATH_LEN];
	int fd;
	snprintf(powerup_node, sizeof(powerup_node),
			"/dev/subsys_%s",
			dev->esoc_node);
	fd = open(powerup_node, O_RDONLY);
	if (fd < 0) {
		ALOGE("%s: Override thread failed to open esoc node: %s",
				dev->mdm_name,
				powerup_node);
	}
	do {
		sleep(50000);
	} while(1);
	return NULL;
}
