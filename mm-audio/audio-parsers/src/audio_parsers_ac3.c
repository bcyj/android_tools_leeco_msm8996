/* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#include "audio_parsers.h"

struct ac3_fr_sync_info {
	unsigned short sync_word;
	unsigned short crc_1;
	unsigned short fscod;
	unsigned short frmsizcod;
	unsigned short bsid;
	unsigned short bsmod;
	unsigned int   sample_rate;
	unsigned int   ac3_fr_sz_16bit;
	unsigned int reverse_bytes;
};

static unsigned char *audio_data;
static unsigned int audio_data_sz;
static unsigned int  audio_data_index = 0;


/* AC-3 Frame size code table */
unsigned short ac3_fr_sz_code_tbl[AC3_MAX_FSCOD][NUM_AC3_FR_SIZES] =
{
	/* 48kHz */
	{	64, 64, 80, 80, 96, 96, 112, 112,
		128, 128, 160, 160, 192, 192, 224, 224,
		256, 256, 320, 320, 384, 384, 448, 448,
		512, 512, 640, 640, 768, 768, 896, 896,
		1024, 1024, 1152, 1152, 1280, 1280
	},
	/* 44.1kHz */
	{	69, 70, 87, 88, 104, 105, 121, 122,
		139, 140, 174, 175, 208, 209, 243, 244,
		278, 279, 348, 349, 417, 418, 487, 488,
		557, 558, 696, 697, 835, 836, 975, 976,
		1114, 1115, 1253, 1254, 1393, 1394
	},
	/* 32kHz */
	{	96, 96, 120, 120, 144, 144, 168, 168,
		192, 192, 240, 240, 288, 288, 336, 336,
		384, 384, 480, 480, 576, 576, 672, 672,
		768, 768, 960, 960, 1152, 1152, 1344, 1344,
		1536, 1536, 1728, 1728, 1920, 1920
	}
};

static int read_ac3_frame(unsigned char *ac3_61937_data, unsigned int size_16bit,
		struct ac3_fr_sync_info *sync_info)
{
	unsigned int i;

	if ((size_16bit < MIN_AC3_FRA_SZ_16_BIT) ||
			(size_16bit > MAX_AC3_FRA_SZ_16_BIT))  {
		fprintf(stderr, "invalid ac3 frame size %u\n", size_16bit);
		return -1;
	}

	if (audio_data_index >= audio_data_sz) {
		fprintf(stderr,"%s : end of file. audio_data_index  %u\n",
					__func__, audio_data_index);
		return -1;
	}

	memcpy(ac3_61937_data, audio_data + audio_data_index,
			(sync_info->ac3_fr_sz_16bit * 2));

	audio_data_index  = audio_data_index + (sync_info->ac3_fr_sz_16bit * 2);

	return 0;

}

int get_ac3_frame_sync_info(unsigned char *ac3_sync_info_data, unsigned int size,
		struct ac3_fr_sync_info *sync_info)
{
	unsigned int i;
	unsigned short data;

	if (size != AC3_SYNC_INFO_SZ) {
		fprintf(stderr, "invalid syncinfo size\n");
		return -1;
	}

	i = 0;
	sync_info->sync_word = (unsigned short) ((ac3_sync_info_data[i] << 8)
				| ac3_sync_info_data[i+1]);

	if (sync_info->sync_word == 0x770B)
		sync_info->reverse_bytes = 0;
	else if (sync_info->sync_word == 0x0B77) {
		sync_info->reverse_bytes = 1;
	} else {
		fprintf(stderr, "invalid syncinfo word 0x%04x\n", sync_info->sync_word);
		return -1;
	}


	i = 2;
	if (sync_info->reverse_bytes)
		sync_info->crc_1 = (unsigned short) ((ac3_sync_info_data[i] << 8)
					| ac3_sync_info_data[i+1]);
	else
		sync_info->crc_1 = (unsigned short) ((ac3_sync_info_data[i+1] << 8)
					| ac3_sync_info_data[i]);


	i = 4;
	if (sync_info->reverse_bytes)
		data = (unsigned short) ((ac3_sync_info_data[i] << 8)
					| ac3_sync_info_data[i+1]);
	else
		data = (unsigned short) ((ac3_sync_info_data[i+1] << 8)
				| ac3_sync_info_data[i]);

	sync_info->fscod =  (unsigned short)(data >> 14);
	sync_info->frmsizcod = ((data & 0x3F00) >> 8 );
	sync_info->bsid = ((data & 0x00F8) >> 3);
	sync_info->bsmod = (data & 0x0007);

	fprintf(stderr, "sync word 0x%04x fscod 0x%02x frmsizcod 0x%02x"
		" bsid 0x%02x bsmod = 0x%02x\n",
		sync_info->sync_word,
		sync_info->fscod,
		sync_info->frmsizcod,
		sync_info->bsid,
		sync_info->bsmod);

	if (sync_info->fscod >= AC3_MAX_FSCOD) {
		fprintf(stderr, "invalid fscod 0x%02x\n", sync_info->fscod);
		return -1;
	}

	if (sync_info->frmsizcod >= NUM_AC3_FR_SIZES) {
		fprintf(stderr, "invalid frmsizcod 0x%02x\n",
				sync_info->frmsizcod);
		return -1;
	}

	if ((sync_info->bsid > 8) || (sync_info->bsid < 0)) {
		fprintf(stderr, "invalid bsid 0x%02x\n", sync_info->bsid);
		return -1;
	}

	if (sync_info->bsmod > 7) {
		fprintf(stderr, "invalid bsmod 0x%02x\n", sync_info->bsmod);
		return -1;
	}

	return 0;
}

int get_ac3_frame_info(struct ac3_fr_sync_info *sync_info)
{
	switch (sync_info->fscod) {
	case 0:
		sync_info->sample_rate = 48000;
		break;
	case 1:
		sync_info->sample_rate = 44100;
		break;
	case 2:
		sync_info->sample_rate = 32000;
		break;
	default:
		sync_info->sample_rate = -1;
		fprintf(stderr, "invalid fscod 0x%02x\n",
				sync_info->fscod);
		return -1;
	}

	sync_info->ac3_fr_sz_16bit =
		ac3_fr_sz_code_tbl[sync_info->fscod][sync_info->frmsizcod];

	fprintf(stderr, "sample rate %d  ac3_frame_size in 16 bit words %d\n",
		sync_info->sample_rate,
		sync_info->ac3_fr_sz_16bit);
	return 0;
}

int get_ac3_first_frame_info(struct audio_parser_codec_info *audio_codec_info)
{
	unsigned char ac3_sync_info_data[AC3_SYNC_INFO_SZ];
	struct ac3_fr_sync_info sync_info;
	int rc;

	memcpy(ac3_sync_info_data, audio_data,
		AC3_SYNC_INFO_SZ);

	rc = get_ac3_frame_sync_info(ac3_sync_info_data, AC3_SYNC_INFO_SZ,
				&sync_info);
	if (rc == -1)
		return rc;

	rc = get_ac3_frame_info(&sync_info);
	if (rc == -1)
		return rc;

	audio_codec_info->codec_config.ac3_fr_info.sample_rate =
						sync_info.sample_rate;
	return 0;
}


int get_ac3_frame(unsigned char *frame, unsigned int sz,
		struct audio_parser_codec_info *audio_codec_info)
{
	unsigned char ac3_sync_info_data[AC3_SYNC_INFO_SZ];
	struct ac3_fr_sync_info sync_info;
	int rc;

	memset(frame, 0, sz);

	if (audio_data_index < audio_data_sz) {

		memcpy(ac3_sync_info_data, audio_data + audio_data_index ,
				AC3_SYNC_INFO_SZ);

		rc = get_ac3_frame_sync_info(ac3_sync_info_data, AC3_SYNC_INFO_SZ,
				&sync_info);
		if (rc == -1)
			return rc;

		rc = get_ac3_frame_info(&sync_info);
		if (rc == -1)
			return rc;

		audio_codec_info->codec_type = AUDIO_PARSER_CODEC_AC3;

		audio_codec_info->codec_config.ac3_fr_info.ac3_fr_sz_16bit =
			sync_info.ac3_fr_sz_16bit;

		audio_codec_info->codec_config.ac3_fr_info.bsmod =
			sync_info.bsmod;

		audio_codec_info->codec_config.ac3_fr_info.sample_rate =
				sync_info.sample_rate;

		audio_codec_info->codec_config.ac3_fr_info.reverse_bytes  =
				sync_info.reverse_bytes;

		rc = read_ac3_frame(frame, sz / 2, &sync_info);

		if (rc == -1) {
			fprintf(stderr, "%s No frame. errors\n" ,__func__);
			return -1;
		}
		return 0;

	} else
		return -1;
}

void extract_bits(unsigned char *input, unsigned int hdr_bit_index,
			unsigned char num_bits_reqd, unsigned short *out_value);
void set_bits(unsigned char *input, unsigned char num_bits_reqd,
		unsigned char value, unsigned char *hdr_bit_index);

void init_chan_status(unsigned char * ch_status,
		struct audio_parser_channel_staus *audio_channel_status)
{
	unsigned char bit_index;

	bit_index = 0;
	/* block start bit in preamble bit 3 */
	set_bits(ch_status, 1, 1, &bit_index);

	bit_index = 1;
	if (audio_channel_status->data_type == non_linear) {
		set_bits(ch_status, 1, 1, &bit_index);
	}

	//Copyright info asserted
	bit_index = 2;
	set_bits(ch_status, 1, audio_channel_status->copyright, &bit_index);

	bit_index = 24;
	switch (audio_channel_status->sample_rate) {
	case 0:		//48k -- 0100 in 24..27
		set_bits(ch_status, 4, 0x04, &bit_index);
		break;
	case 1:		//44.1k --- 0000 in 24..27
		break;
	case 2:		//32k --- 1100 in 24..27
		set_bits(ch_status, 4, 0x0C, &bit_index);
		break;
	default:
		fprintf(stderr, "%s Invalid sample_rate %u\n", __func__,
				audio_channel_status->sample_rate);
		break;
	}

	bit_index = 40;
	set_bits(ch_status, 2, audio_channel_status->cgmsa, &bit_index);
}
int get_ac3_frame_channel_info(struct audio_parser_channel_staus *audio_channel_status)
{
	unsigned int i;
	unsigned short data;
	unsigned short sync_word;
	unsigned char reverse_bytes;
	unsigned char acmod;

        //non-linear pcm
	audio_channel_status->data_type = non_linear;

	i = 0;
	sync_word = (unsigned short) ((audio_data[i] << 8)
				| audio_data[i+1]);

	if (sync_word == 0x770B)
		reverse_bytes = 0;
	else if (sync_word == 0x0B77) {
		reverse_bytes = 1;
	} else {
		fprintf(stderr, "invalid sync_word 0x%04x\n", sync_word);
		return -1;
	}

	i = 4;
	if (reverse_bytes)
		data = (unsigned short) ((audio_data[i] << 8)
					| audio_data[i+1]);
	else
		data = (unsigned short) ((audio_data[i+1] << 8)
				| audio_data[i]);

	audio_channel_status->sample_rate =  (unsigned short)(data >> 14);


	i = 6;
	if (reverse_bytes)
		data = (unsigned short) ((audio_data[i] << 8)
					| audio_data[i+1]);
	else
		data = (unsigned short) ((audio_data[i+1] << 8)
				| audio_data[i]);
	acmod =  (unsigned short)(data >> 13);

	if(acmod == 0) {
		i = 14;
		if (reverse_bytes)
			data = (unsigned short) ((audio_data[i] << 8)
						| audio_data[i+1]);
		else
			data = (unsigned short) ((audio_data[i+1] << 8)
					| audio_data[i]);
		audio_channel_status->copyright =  !((unsigned short)(data >> 15));
		if(audio_channel_status->copyright)
			audio_channel_status->cgmsa = 0x00;
		else
			audio_channel_status->cgmsa = 0x03;
	} else {
		i = 10;
		if (reverse_bytes)
			data = (unsigned short) ((audio_data[i] << 8)
						| audio_data[i+1]);
		else
			data = (unsigned short) ((audio_data[i+1] << 8)
					| audio_data[i]);
		audio_channel_status->copyright =  !((unsigned short)(data >> 14) & 0x01);
		if(audio_channel_status->copyright)
			audio_channel_status->cgmsa = 0x00;
		else
			audio_channel_status->cgmsa = 0x03;
	}
	return 0;
}

int get_channel_status_ac3(unsigned char *channel_status)
{
	int no_of_bytes;
	int rc;
	struct audio_parser_channel_staus audio_channel_status;
	memset(&audio_channel_status, 0, sizeof(audio_channel_status));
	rc = get_ac3_frame_channel_info(&audio_channel_status);
	if(rc != -1) {
	   init_chan_status(channel_status, &audio_channel_status);
	}
	return rc;
}

void init_audio_parser_ac3(unsigned char *audio_stream_data,
	unsigned int audio_stream_sz)
{
	audio_data = audio_stream_data;
	audio_data_sz = audio_stream_sz;
	audio_data_index = 0;
}
