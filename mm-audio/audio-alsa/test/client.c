/* Copyright (c) 2009 by Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <locale.h>
#include <sound/asound.h>
#include "control.h"


int main(void)
{
	int mixer_cnt = 0;
	int control;
	int i = 0;
	int j = 0;
	int dev_cnt = 0;
	int device_id;
	const char **device_names;

	printf("msm_mixer_open: Opening the device\n");
	control = msm_mixer_open("/dev/snd/controlC0", 0);
	if(control< 0)
		printf("ERROR opening the device\n");

	mixer_cnt = msm_mixer_count();
	printf("msm_mixer_count:mixer_cnt =%d\n",mixer_cnt);

	dev_cnt = msm_get_device_count();
	device_names = msm_get_device_list();

	printf("got device_list %d\n",dev_cnt);

	for(i = 0; i < dev_cnt;) {
		device_id = msm_get_device(device_names[i]);
		if(device_id >= 0)
			printf("\nFound device: %s:dev_id: %d\n", device_names[i], device_id);
		i++;
	}

	msm_mixer_close();
	return 0;
}
