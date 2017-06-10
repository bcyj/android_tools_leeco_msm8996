/* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AUDIO_PARSERS_H
#define __AUDIO_PARSERS_H


#define AC3_SYNC_INFO_SZ 		6 /* bytes*/

#define MIN_AC3_FRA_SZ_16_BIT		64	/* in 16 bit words */
#define MAX_AC3_FRA_SZ_16_BIT		1920	/* in 16 bit words */
#define AC3_FRA_SZ			(MAX_AC3_FRA_SZ_16_BIT * 2)

#define AC3_MAX_FSCOD 			3
#define NUM_AC3_FR_SIZES 		38

#define MIN_DTS_FRA_SZ_16_BIT		256	/* in 16 bit words */
#define MAX_DTS_FRA_SZ_16_BIT		8192	/* in 16 bit words */ // Assuming till DTS type 3
#define DTS_FRA_SZ			(MAX_DTS_FRA_SZ_16_BIT * 2)
#define DTS_SYNC_INFO_SZ 		5  /* 32 sync word +6 sync word ext*/

#define DTS_TYPE_1			11
#define DTS_TYPE_2			12
#define DTS_TYPE_3			13
#define DTS_TYPE_4			17

#define MIN(x, y) (x<y? x: y)

enum audio_parser_code_type {
	AUDIO_PARSER_CODEC_AC3 = 1,
	AUDIO_PARSER_CODEC_DTS = 2,
};

struct ac3_frame_info {
	unsigned int ac3_fr_sz_16bit;
	unsigned char bsmod;
	unsigned int sample_rate;
	unsigned int reverse_bytes;
};

struct dts_frame_info {
        unsigned int  dts_fr_sz_8bit;
        unsigned int  sample_rate;
        unsigned char dts_type;
        unsigned int  reverse_bytes;
};

enum hmdi_data_type {
	pcm_linear,
	non_linear
};
struct audio_parser_channel_staus {
	unsigned char data_type;
	unsigned char copyright;
	unsigned char format_info;
	unsigned char category_code;
	unsigned char source_num;
	unsigned char channel_num;
	unsigned char sample_rate;
	unsigned char clock_accu;
	unsigned char word_len;
	unsigned char bit_rate;
	unsigned char cgmsa;
};

struct audio_parser_codec_info {
	enum audio_parser_code_type codec_type;
	union {
		struct ac3_frame_info ac3_fr_info;
		struct dts_frame_info dts_fr_info;
	}codec_config;
};

int init_audio_parser(unsigned char *audio_stream_data,
	unsigned int audio_stream_sz, enum audio_parser_code_type);

int get_audio_frame(unsigned char *frame, unsigned int sz,
		struct audio_parser_codec_info *audio_parser_codec_info);

int get_first_frame_info(struct audio_parser_codec_info *audio_codec_info);

int get_ac3_frame(unsigned char *frame, unsigned int sz,
		struct audio_parser_codec_info *audio_codec_info);

int get_dts_frame(unsigned char *frame, unsigned int sz,
		struct audio_parser_codec_info *audio_codec_info);

void get_silent_frame(unsigned char *silent_frame);

void init_audio_parser_dts(unsigned char *audio_stream_data,
			unsigned int audio_stream_sz);

int get_channel_status(unsigned char *channel_status, enum audio_parser_code_type code_type);

void init_audio_parser_ac3(unsigned char *audio_stream_data,
			unsigned int audio_stream_sz);
#endif
