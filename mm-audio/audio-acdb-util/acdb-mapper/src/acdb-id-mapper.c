/* audcal-ctrl.c
 *
 * This library contains the API to load the audio calibration
 * data from database and push to the DSP
 *
 * Copyright (c) 2010 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */

#include <string.h>
#include "acdb-id-mapper.h"
#include "control.h"

struct acdb_id_info
{
	char dev_name[80];
	int acdb_id;
};

static struct acdb_id_info mapping_table[] =
{
	{"handset_rx", DEVICE_HANDSET_RX_ACDB_ID},
	{"handset_tx", DEVICE_HANDSET_TX_ACDB_ID},
	{"speaker_stereo_rx", DEVICE_SPEAKER_RX_ACDB_ID},
	{"speaker_mono_tx", DEVICE_SPEAKER_TX_ACDB_ID},
	{"headset_stereo_rx", DEVICE_HEADSET_RX_ACDB_ID},
	{"headset_mono_tx", DEVICE_HEADSET_TX_ACDB_ID},
	{"fmradio_handset_rx", DEVICE_FMRADIO_HANDSET_RX_ACDB_ID},
	{"fmradio_headset_rx", DEVICE_FMRADIO_HEADSET_RX_ACDB_ID},
	{"fmradio_speaker_rx", DEVICE_FMRADIO_SPEAKER_RX_ACDB_ID},
	{"handset_dual_mic_endfire_tx", DEVICE_DUALMIC_HANDSET_TX_ENDFIRE_ACDB_ID},
	{"speaker_dual_mic_endfire_tx", DEVICE_DUALMIC_SPEAKER_TX_ENDFIRE_ACDB_ID},
	{"handset_dual_mic_broadside_tx", DEVICE_DUALMIC_HANDSET_TX_BROADSIDE_ACDB_ID},
	{"speaker_dual_mic_broadside_tx", DEVICE_DUALMIC_SPEAKER_TX_BROADSIDE_ACDB_ID},
	{"tty_headset_mono_rx", DEVICE_TTY_HEADSET_MONO_RX_ACDB_ID},
	{"tty_headset_mono_tx", DEVICE_TTY_HEADSET_MONO_TX_ACDB_ID},
	{"bt_sco_rx", DEVICE_BT_SCO_RX_ACDB_ID},
	{"bt_sco_tx", DEVICE_BT_SCO_TX_ACDB_ID},
	{"headset_stereo_speaker_stereo_rx", DEVICE_SPEAKER_HEADSET_RX_ACDB_ID},
	{"fmradio_stereo_tx", DEVICE_FMRADIO_STEREO_TX_ACDB_ID},
	{"hdmi_stereo_rx", DEVICE_HDMI_STEREO_RX_ACDB_ID},
	{"anc_headset_stereo_rx", DEVICE_ANC_HEADSET_STEREO_RX_ACDB_ID},
};

int acdb_mapper_get_acdb_id_from_dev_name(char *dev_name, int *acdb_id)
{
	unsigned int idx;

	for (idx = 0; idx < (sizeof(mapping_table)/sizeof(struct acdb_id_info)); idx++) {
		if(strcmp((char* )dev_name,mapping_table[idx].dev_name) == 0) {
			if (acdb_id)
				*acdb_id = mapping_table[idx].acdb_id;
			return 0;
		}
	}
	return -1;
}

int acdb_mapper_get_acdb_id_from_dev_id(int dev_id, int *acdb_id)
{
	unsigned int idx;

	for (idx = 0; idx < (sizeof(mapping_table)/sizeof(struct acdb_id_info)); idx++) {
		if(dev_id == msm_get_device(mapping_table[idx].dev_name)) {
			if (acdb_id)
				*acdb_id = mapping_table[idx].acdb_id;
			return 0;
		}
	}
	return -1;
}
