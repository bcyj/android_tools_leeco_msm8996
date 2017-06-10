/* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#include "audio_parsers.h"

int get_audio_frame(unsigned char *frame, unsigned int sz,
	struct audio_parser_codec_info *audio_codec_info)
{
	int rc;
	switch (audio_codec_info->codec_type) {
	case AUDIO_PARSER_CODEC_AC3:
		rc = get_ac3_frame(frame, sz, audio_codec_info);

		break;
	case AUDIO_PARSER_CODEC_DTS:
		rc = get_dts_frame(frame, sz, audio_codec_info);

		break;
	default:
		fprintf(stderr, "%s un supported code type %u\n" ,
				__func__, audio_codec_info->codec_type);
		return -1;
	}
	return rc;
}

int get_first_frame_info(struct audio_parser_codec_info *audio_codec_info)
{
	switch(audio_codec_info->codec_type) {
	case AUDIO_PARSER_CODEC_AC3:
		get_ac3_first_frame_info(audio_codec_info);
		break;
	case AUDIO_PARSER_CODEC_DTS:
		get_dts_first_frame_info(audio_codec_info);
		break;
	default:
		fprintf(stderr, "Unsupported codec type\n");
		return -1;
	}
	return 0;
}

int get_channel_status(unsigned char *channel_status, enum audio_parser_code_type code_type)
{
	if (code_type == AUDIO_PARSER_CODEC_AC3) {
		get_channel_status_ac3(channel_status);
		return 0;
	} else if (code_type == AUDIO_PARSER_CODEC_DTS) {
		get_channel_status_dts(channel_status);
		return 0;
	} else {
		fprintf(stderr, "%s un supported code type %u\n" ,
				__func__, code_type);
		return -1;
	}
}

int init_audio_parser(unsigned char *audio_stream_data,
	unsigned int audio_stream_sz, enum audio_parser_code_type code_type)
{
	int max_frame_sz;

	if (code_type == AUDIO_PARSER_CODEC_AC3) {
		init_audio_parser_ac3(audio_stream_data, audio_stream_sz);
		max_frame_sz = MAX_AC3_FRA_SZ_16_BIT * 2;
		return max_frame_sz;
	} else if (code_type == AUDIO_PARSER_CODEC_DTS) {
		init_audio_parser_dts(audio_stream_data, audio_stream_sz);
		max_frame_sz = MAX_DTS_FRA_SZ_16_BIT * 2;
		return max_frame_sz;
	} else {
		fprintf(stderr, "%s un supported code type %u\n" ,
				__func__, code_type);
		return -1;
	}
}
