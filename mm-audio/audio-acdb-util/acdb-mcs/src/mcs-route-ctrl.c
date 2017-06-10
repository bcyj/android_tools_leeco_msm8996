/*
 * Copyright (c) 2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 *
 */

#define LOG_TAG "MCS-RT-CTL"

#include <utils/Log.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _TINY_ALSA_LIB_
#include <tinyalsa/asoundlib.h>
#include "mcs-tinyalsa-wrapper.h"
#else
#include <alsa_audio.h>
#endif
#include "common_log.h"
#include "mcs-route-ctrl.h"

#define ACDB_DEV_STR "acdb_dev_id:"

struct mcs_route_ctrl_info * mcs_route_ctrl_init(const char* config_filename,
						int sndcard_num)
{
	struct mcs_route_ctrl_info *route_info;
	route_info = calloc(1, sizeof(struct mcs_route_ctrl_info));

	if (route_info == NULL) {
		ALOGE("Failed to allocate the memory.");
		return NULL;
	}

	route_info->file_hdl = fopen(config_filename,"rb");
	if (route_info->file_hdl == NULL) {
		ALOGE("Can't open the configuration file %s.", config_filename);
		return NULL;
	}

	route_info->sndcard_num = sndcard_num;

	return route_info;
}

int mcs_route_ctrl_set_path(struct mcs_route_ctrl_info * route_hdl,
				int acdb_id,
				int ena_flag,
				int * pcm_device)
{
	char *p, array[100], *pmode;
	char en[] = "enable";
	char dis[] = "disable";
	int len = 0, found = 0, len2 = 0;
	struct mixer *mxr = NULL;
	struct mixer_ctl *ctl = 0;
	char *temp;
	int params, sublen, ret = 0;


	if (route_hdl == NULL) {
		ALOGE("Invalid MCS routing control handle.");
		return -EINVAL;
	}

	if (route_hdl->file_hdl == NULL) {
		ALOGE("Invalid configuration file handle.");
		return -EINVAL;
	}

	rewind(route_hdl->file_hdl);

	pmode = (ena_flag)?en:dis;

	/* Find the test case */
	while((p = fgets(array, sizeof(array), route_hdl->file_hdl))) {

		if (strcasestr(p, ACDB_DEV_STR)) {
			if (atoi(&p[sizeof(ACDB_DEV_STR)-1]) == acdb_id) {
				found = 1;
				break;
			}
		}
	}
	if (!found) {
		ALOGE("Can't find ACDB ID %d from configuration file.", acdb_id);
		return -EACCES;
	}

	ALOGD("Found acdb_dev_id:%d %s", acdb_id, pmode);

	/* Find enable or disable commands*/
	found = 0;

	while((p = fgets(array, sizeof(array), route_hdl->file_hdl))) {
		len = strnlen(p,sizeof(array));
		p[len-1] = '\0';
		len--;
		/* Comment added print comment */
		if (!strncmp(&p[0], "#", 1)) {
			ALOGD("%s", p);
		} else if (strstr(p, "Rxdevice")) {
			temp = NULL;
			if ((temp = strstr(p,":"))) {
				sublen = temp - p;
				len = len - sublen - 1;
				temp++;
				temp[len] = '\0';
				if (*temp >= '0' || *temp <= '9')
					*pcm_device = atoi(temp);
				else
					*pcm_device = -1;
			}
		} else if (strstr(p, "Txdevice")) {
			temp = NULL;
			if ((temp = strstr(p,":"))) {
				sublen = temp - p;
				len = len - sublen - 1;
				temp++;
				temp[len] = '\0';
				if (*temp >= '0' || *temp <= '9')
					*pcm_device = atoi(temp);
				else
					*pcm_device = -1;
			}
		} else if (strstr(p, pmode)) {
			found = 1;
			break;
		}
	}
	if (!found) {
		ALOGE("Sequence for %s not found", pmode);
		return -1;
	} else {
		ALOGD("Sequence for %s found", pmode);
	}
	pmode = (!ena_flag)?en:dis;
	mxr = mixer_open(route_hdl->sndcard_num);
	if (!mxr) {
		ALOGE("Opening mixer control failed");
		return -1;
	}
	while((p = fgets(array, sizeof(array), route_hdl->file_hdl))) {
		len = strnlen(p,sizeof(array));
		p[len-1] = '\0';
		len--;
		if (strstr(p, ACDB_DEV_STR) || strstr(p, pmode)) {
			break;
		} else {
			if (len) {
				char ctlname[100];
				char ctlval[100];
				temp = strstr(p,":");
				if (temp) {
					sublen = temp - p;
					memcpy(ctlname, p, sublen);
					ctlname[sublen] = '\0';
					ctl = get_ctl(mxr, ctlname);
					if (!ctl) {
						ALOGE("Failed to get %s\n", ctlname);
						break;
					}
					sublen = len - sublen;
					sublen--;
					temp++;
					memcpy(ctlval, temp, sublen);
					ctlval[sublen] = '\0';
					int val = -1;
					while(sublen > 0) {
						if (*temp == ' ') {
							temp++;
							sublen--;
						}else if (*temp >= '0' && *temp <= '9') {
							val = atoi(temp);
							break;
						} else {
							val = 0;
							break;
						}
					}
					if (val < 0) {
						ALOGE("Invalid param for val");
						return -EINVAL;
					} else if (!val) {
						ALOGD("Select %s %s", ctlname, ctlval);
						ret = mixer_ctl_select(ctl, ctlval);
					} else {
						ALOGD("Set %s %d", ctlname, val);
						ret = mixer_ctl_set(ctl, val);
					}
				}
		}
	  }
	}
	mixer_close(mxr);
	return ret;
}
