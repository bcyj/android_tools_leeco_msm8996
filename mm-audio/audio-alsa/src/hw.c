/* Copyright (c) 2009-2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#ifdef _ANDROID_
#include <linux/compiler.h>
#endif
#include <sound/asound.h>
#include "control.h"

#ifdef USE_GLIB
#include <glib.h>
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#endif /* USE_GLIB */

#define DIR_TX	2
#define DIR_RX	1

/* Parameters For Voice Start/Stop command */
#define VOICE_CALL_END		0
#define VOICE_CALL_START	1
#define COUNT_CMD_NAME		"Count"
#define STREAM_CMD_NAME		"Stream"
#define RECORD_CMD_NAME		"Record"
#define VOICE_CMD_NAME		"Voice"
#define VOLUME_CMD_NAME		"Volume"
#define VOICEMUTE_CMD_NAME	"VoiceMute"
#define VOICEVOLUME_CMD_NAME	"VoiceVolume"
#define VOICECALL_CMD_NAME	"Voice Call"
#define DEVICEVOLUME_CMD_NAME	"Device_Volume"
#define ANC_CMD_NAME		"ANC"
#define RESET_CMD_NAME		"Reset"
#define DUALMICSWITCH_CMD_NAME	"DualMic Switch"
#define LOOPBACK_CMD_NAME	"Sound Device Loopback"
#define DEVICEMUTE_CMD_NAME	"Device_Mute"
#define VOICE_EXT_CMD_NAME	"Voice Ext"
#define VOICEMUTE_EXT_CMD_NAME	"VoiceMute Ext"
#define VOICEVOLUME_EXT_CMD_NAME "VoiceVolume Ext"
#define VOICECALL_EXT_CMD_NAME	"Voice Call Ext"
#define VOICE_SESSION_CMD_NAME	"Voice session"
#define VOIP_SESSION_CMD_NAME	"VoIP session"

struct msm_control *control;
struct snd_ctl_elem_list *list;
struct snd_ctl_elem_info *info;

static int mixer_cnt; /* Total count of controls */
const char **device_names; /* Device Name List */
static int device_index; /* Count of Device controls */
static int simp_contols; /* Count of Simple controls */

/*
 * Function:	msm_mixer_count
 * Parameters:	Null
 * Description:	Return count of Mixer Controls.
 */
int msm_mixer_count(void)
{
	if ((!control) || (!(control->fd)))
		return 0;

	if (ioctl(control->fd, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0) {
		printf("ERROR:SNDRV_CTL_IOCTL_ELEM_LIST failed");
		return -errno;
	}
	return list->count;
}

/*
 * Function:	msm_mixer_elem_info
 * Parameters:	snd_ctl_elem_info-> Pointer to empty allocated structure
 *		with numid value set.
 * Description:	Return Control element's description.
 */
static int msm_mixer_elem_info(struct snd_ctl_elem_info *info)
{
	if ((!control) || (!(control->fd)))
		return 0;

	if (ioctl(control->fd, SNDRV_CTL_IOCTL_ELEM_INFO, info) < 0)
		return -errno;
	return 0;
}

/*
 * Function:	msm_mixer_elem_read
 * Parameters:	snd_ctl_elem_value-> Pointer to empty allocated structure
 *		with numid value set.
 * Description:	Return Control element's Current Value.
 */
static int msm_mixer_elem_read(struct snd_ctl_elem_value *val)
{
	if ((!control) || (!(control->fd)))
		return 0;

	if (ioctl(control->fd, SNDRV_CTL_IOCTL_ELEM_READ, val) < 0)
		return -errno;
	return 0;
}

/*
 * Function:	msm_mixer_elem_write
 * Parameters:	snd_ctl_elem_value-> Pointer to empty allocated structure
 *		with numid value set.
 * Description:	Write new value to control element.
 */
static int msm_mixer_elem_write(struct snd_ctl_elem_value *val)
{
	int rc;

	if ((!control) || (!(control->fd)))
		return 0;

	rc = ioctl(control->fd, SNDRV_CTL_IOCTL_ELEM_WRITE, val);

	if (rc < 0) {
		printf("msm_mixer_elem_write failed.\n");
		return -errno;
	}
	return 0;
}

static int get_database ()
{
	int i = 1;
	do{
		int ret;
		info[i-1].id.numid = i;
		ret = msm_mixer_elem_info(&info[i-1]);
		if (ret) {
			printf("msm_mixer_elem_info returned = %d\n", ret);
			break;
		}
		i++;
	}while(i <= mixer_cnt);
	return 0;
}

/*
 * Function:	msm_simp_ctrl_count
 * Parameters:	NULL
 * Description:	Read the count for Simple controls.
 */
static int msm_simp_ctrl_count(void)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, COUNT_CMD_NAME, sizeof (val.id.name));
	msm_mixer_elem_read(&val);
	return val.value.integer.value[0];
}


/*
 * Function:	msm_mixer_open
 * Parameters:	const char *name[ path to ControlC0 devicenode]
 *		card Number = 0;
 * Description:	Open an instance of mixer control dev node
 *		and populate the informations for available controls.
 */
int msm_mixer_open(const char *name, int card)
{
	int mode = O_RDWR, i;

	if (card < 0 || card >= 32) {
		printf("Invalid card index %d", card);
		return -EINVAL;
	}

	control = calloc(sizeof(struct msm_control), 1);
	if (control == NULL) {
		goto err;
	}

	list = (struct snd_ctl_elem_list *)
		calloc(sizeof(struct snd_ctl_elem_list), 1);
	if (list == NULL) {
		printf("ERROR allocating the memory to list\n");
		goto err;
	}

	control->fd = open(name, mode);
	if (control->fd < 0) {
		printf("ERROR allocating the memory to control\n");
		goto err;
	}
	control->card = card;

	mixer_cnt = msm_mixer_count();
	printf("mixer_cnt =%d\n", mixer_cnt);

	info = (struct snd_ctl_elem_info *)calloc(mixer_cnt,
			 sizeof(struct snd_ctl_elem_info));
	if (control->fd < 0) {
		printf("ERROR allocating the memory to info\n");
		goto err;
	}
	get_database();
	simp_contols = msm_simp_ctrl_count();
	device_index = simp_contols;

	device_names = calloc(sizeof(char *) * (mixer_cnt - simp_contols), 1);
	if (device_names == NULL) {
		goto err;
		close(control->fd);
		free(control);
		free(list);
		free(info);
		return -ENOMEM;
	}
	for (i = 0; i < (mixer_cnt - simp_contols); i++)
		device_names[i] = (char *)info[i + device_index].id.name;

	return 0;
err:
	msm_mixer_close();
	return -ENOMEM;
}

/*
 * Function:	msm_mixer_close
 * Parameters:	NULL
 * Description:	Close the open instance of mixer control dev node
 *		and free the allocated memories.
 */
int msm_mixer_close(void)
{
	if (control) {
		if (control->fd > 0)
			close(control->fd);
		free(control);
		control = NULL;
	}
	if (list) {
		free(list);
		list = NULL;
	}
	if (info) {
		free(info);
		info = NULL;
	}
	if (device_names) {
		free(device_names);
		device_names = NULL;
	}
	return 0;
}

/*
 * Function:	msm_get_device_class
 * Parameters:	int device_id
 * Description:	Return the class ie COPP ID
 */
int msm_get_device_class(int device_id)
{
	struct snd_ctl_elem_value val;

	val.id.numid = device_id + device_index + 1;
	msm_mixer_elem_read(&val);
	return val.value.integer.value[0];
}

/*
 * Function:	msm_get_device_capability
 * Parameters:	int device_id
 * Description:	Return the capability masks for the device
 *		Possible values are one or more of these values orred.
 *		SNDDEV_CAP_RX 0x1 	-> RX direction
 *		SNDDEV_CAP_TX 0x2 	-> TX direction
 *		SNDDEV_CAP_VOICE 0x4 	-> Support voice call
 *		SNDDEV_CAP_PLAYBACK 0x8 -> Support playback
 *		SNDDEV_CAP_FM 0x10 	-> Support FM radio
 *		SNDDEV_CAP_TTY 0x20 	-> Support TTY
 */
int msm_get_device_capability(int device_id)
{
	struct snd_ctl_elem_value val;

	val.id.numid = device_id + device_index + 1;
	msm_mixer_elem_read(&val);
	return val.value.integer.value[1];
}

/*
 * Function:	msm_get_device_count
 * Parameters:	NULL
 * Description:	Return the count of Device controls
 */
int msm_get_device_count(void)
{
	return mixer_cnt - simp_contols;
}

/*
 * Function:	msm_get_device_list
 * Parameters:	NULL
 * Description:	Return the pointer to device name array
 */
const char **msm_get_device_list(void)
{
	return device_names;
}

/*
 * Function:	msm_get_device
 * Parameters:	const char * name
 * Description:	Return device number associated with the device name
 */
int msm_get_device(const char * name)
{
	int i = 0;
	int v = 0;
	for (i = 0; i < mixer_cnt; i++) {
		v = strcmp((char *)info[i].id.name, name);
		if (v == 0)
			return info[i].id.numid - (device_index + 1);
	}
	return -ENODEV;
}

/*
 * Function:	msm_en_device
 * Parameters:	int dev_id
 * 		int set[0/1]
 * Description:	Enable/Disable device associated with the device
 * 		id provided
 */
int msm_en_device(int dev_id, int set)
{
	struct snd_ctl_elem_value val;
	int device = dev_id + device_index + 1;

	val.id.numid = device;
	val.value.integer.value[0] = set;

	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_route_stream
 * Parameters:	int dir[playback(1)/record(2)]
		int dec_id[ session id for the playback session(0-5)]
 * 		int set[0/1]
 * Description:	Route session id to the device associated with the device
 * 		id provided
 */
int msm_route_stream (int dir, int dec_id, int dev_id, int set)
{
	struct snd_ctl_elem_value val;
	if(dir > 2 || dir < 1)
		return -EINVAL;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	if (dir == 1)
		strlcpy((char *)val.id.name, STREAM_CMD_NAME, sizeof(val.id.name));
	else if (dir ==2)
		strlcpy((char *)val.id.name, RECORD_CMD_NAME, sizeof(val.id.name));
	else
		return -EINVAL;
	val.value.integer.value[0] = dec_id;
	val.value.integer.value[1] = dev_id;
	val.value.integer.value[2] = set;
	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_route_voice
 * Parameters:	int rx_dev_id
		int tx_dev_id
 * 		int set[0/1]
 * Description:	Route the voice session to the device associated
 *		with the device	id provided
 */
int msm_route_voice (int rx_dev_id, int tx_dev_id, int set)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOICE_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = rx_dev_id;
	val.value.integer.value[1] = tx_dev_id;
	val.value.integer.value[2] = set;
	val.value.integer.value[3] = 0;
	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_set_volume
 * Parameters:	int dec_id
		float volume in percentage[0-100]
 * Description:	Set the volume gain to the session id provided
 */
int msm_set_volume(int dec_id, float volume)
{
	struct snd_ctl_elem_value val;

	/* Keep the precision up to 4 digit */
	int factor = 10000;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOLUME_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = dec_id;
	val.value.integer.value[1] = volume*factor;
	val.value.integer.value[2] = factor;

	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_set_voice_tx_mute
 * Parameters:	int mute[0/1]
 * Description:	Mute/Unmute the Voice Tx session.
 */
int msm_set_voice_tx_mute(int mute)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOICEMUTE_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = DIR_TX;
	val.value.integer.value[1] = mute;
	val.value.integer.value[2] = 0;

	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_set_voice_tx_mute_ext
 * Parameters:	int mute[0/1]
 * 		int session_id
 * Description:	Mute/Unmute the Voice Tx session.
 */
int msm_set_voice_tx_mute_ext(int mute, int session_id)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOICEMUTE_EXT_CMD_NAME,
		sizeof(val.id.name));
	val.value.integer.value[0] = DIR_TX;
	val.value.integer.value[1] = mute;
	val.value.integer.value[2] = session_id;

	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_set_voice_rx_vol
 * Parameters:	int dec_id
		int volume in percentage[0-100]
 * Description:	Set the volume gain to voice Rx session.
 */
int msm_set_voice_rx_vol(int volume)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOICEVOLUME_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = DIR_RX;
	val.value.integer.value[1] = volume;
	val.value.integer.value[2] = 0;

	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_set_voice_rx_vol_ext
 * Parameters:	int volume in percentage[0-100]
 * 		int session_id
 * Description:	Set the volume gain to voice Rx session.
 */
int msm_set_voice_rx_vol_ext(int volume, int session_id)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOICEVOLUME_EXT_CMD_NAME,
		sizeof(val.id.name));
	val.value.integer.value[0] = DIR_RX;
	val.value.integer.value[1] = volume;
	val.value.integer.value[2] = session_id;

	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_start_voice
 * Parameters:	NULL
 * Description:	Trigger start Voice Call
 */
int msm_start_voice(void)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOICECALL_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = VOICE_CALL_START;
	val.value.integer.value[1] = 0;

	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_start_voice_ext
 * Parameters:	int session_id
 * Description:	Trigger start Voice Call
 */
int msm_start_voice_ext(int session_id)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOICECALL_EXT_CMD_NAME,
		sizeof(val.id.name));
	val.value.integer.value[0] = VOICE_CALL_START;
	val.value.integer.value[1] = session_id;

	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_end_voice
 * Parameters:	NULL
 * Description:	Trigger stop Voice Call
 */
int msm_end_voice(void)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOICECALL_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = VOICE_CALL_END;
	val.value.integer.value[1] = 0;

	return msm_mixer_elem_write(&val);
}

/*
 * Function:	msm_end_voice_ext
 * Parameters:	int session_id
 * Description:	Trigger stop Voice Call
 */
int msm_end_voice_ext(int session_id)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, VOICECALL_EXT_CMD_NAME,
		sizeof(val.id.name));
	val.value.integer.value[0] = VOICE_CALL_END;
	val.value.integer.value[1] = session_id;

	return msm_mixer_elem_write(&val);
}

int msm_set_device_volume(int dev_id, int volume)
{
	struct snd_ctl_elem_value val;

	int rc;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, DEVICEVOLUME_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = dev_id;
	val.value.integer.value[1] = volume;

	printf("msm_set_device_volume : dev_id =%d, volume =  %d\n", dev_id, volume);

	rc = msm_mixer_elem_write(&val);

	if (rc < 0)
		printf("msm_set_device_volume failed.\n");

	return rc;
}
int msm_enable_anc(int dev_id, int enable)
{
	struct snd_ctl_elem_value val;

	int rc;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, ANC_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = dev_id;
	val.value.integer.value[1] = enable;

	printf("msm_enable_anc : dev_id =%d, enable =  %d\n", dev_id, enable);

	rc = msm_mixer_elem_write(&val);

	if (rc < 0)
		printf("msm_enable_anc failed.\n");

	return rc;
}

int msm_reset_all_device(void)
{
	struct snd_ctl_elem_value val;

	int rc;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, RESET_CMD_NAME, sizeof(val.id.name));

	printf("Resetting all devices\n");

	rc = msm_mixer_elem_write(&val);

	if (rc < 0)
		printf("Resetting all devices failed\n");

	return rc;
}

int msm_set_dual_mic_config(int enc_session_id, int config)
{
	struct snd_ctl_elem_value val;

	int rc;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, DUALMICSWITCH_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = enc_session_id;
	val.value.integer.value[1] = config;

	printf("msm_set_dual_config : enc_session_id =%d,"
			" config =  %d\n", enc_session_id, config);

	rc = msm_mixer_elem_write(&val);

	if (rc < 0)
		printf("msm_set_dual_mic_config failed.\n");

	return rc;
}

/*
 * Function:	msm_snd_dev_loopback
 * Parameters:	int rx_dev_id
		int tx_dev_id
 * 		int set[0/1]
 * Description:	Route the session from tx device to rx device.
 */
int msm_snd_dev_loopback (int rx_dev_id, int tx_dev_id, int set)
{
	struct snd_ctl_elem_value val;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, LOOPBACK_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = rx_dev_id;
	val.value.integer.value[1] = tx_dev_id;
	val.value.integer.value[2] = set;
	return msm_mixer_elem_write(&val);
}

int msm_device_mute(int dev_id, int mute)
{
	struct snd_ctl_elem_value val;

	int rc;

	memset(&val, 0, sizeof(struct snd_ctl_elem_value));
	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strlcpy((char *)val.id.name, DEVICEMUTE_CMD_NAME, sizeof(val.id.name));
	val.value.integer.value[0] = dev_id;
	val.value.integer.value[1] = mute;

	printf("msm_device_mute: dev_id= %d"
			" mute =%d\n", dev_id, mute);

	rc = msm_mixer_elem_write(&val);

	if (rc < 0)
		printf("msm_device_mute failed.\n");

	return rc;
}

int msm_get_voc_session(const char *name)
{
	struct snd_ctl_elem_value val;
	memset(&val, 0, sizeof(struct snd_ctl_elem_value));

	val.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;

	if (name != NULL) {
		strlcpy((char *)val.id.name, name, sizeof(val.id.name));

		msm_mixer_elem_read(&val);

		return val.value.integer.value[0];
	} else {
		return 0;
	}
}
