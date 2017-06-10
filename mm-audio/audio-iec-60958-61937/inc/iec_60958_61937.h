/* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __IEC_60958_61937
#define __IEC_60958_61937


#define AC3_SYNC_INFO_SZ 		6 /* bytes*/

#define MIN_AC3_FRA_SZ_16_BIT		64 	/* in 16 bit words */
#define MAX_AC3_FRA_SZ_16_BIT		1920  	/* in 16 bit words */
#define AC3_FRA_SZ			(MAX_AC3_FRA_SZ_16_BIT * 2)

#define AC3_MAX_FSCOD 			3
#define NUM_AC3_FR_SIZES 		38

#define MIN_DTS_FRA_SZ_16_BIT           512     /* in 16 bit words */
#define MAX_DTS_FRA_SZ_16_BIT           8192    /* in 16 bit words */
#define DTS_TYPE_I_REP_PER              512 /* num of 60958 Frames */
#define DTS_TYPE_II_REP_PER             1024 /* num of 60958 Frames */
#define DTS_TYPE_III_REP_PER            2048 /* num of 60958 Frames */
#define AC3_REP_PER                     1536 /* num of 60958 Frames */

enum iec_61937_code_type {
	IEC_61937_CODEC_AC3 = 1,
	IEC_61937_CODEC_DTS_I = 11,
	IEC_61937_CODEC_DTS_II = 12,
	IEC_61937_CODEC_DTS_III = 13,
	IEC_61937_CODEC_DTS_IV = 17,
};

struct ac3_frame_config {
	unsigned int ac3_fr_sz_16bit;	/* size of AC3 frame in 16 bit words */
	unsigned char bsmod;
	unsigned int sample_rate;

	/* if byte 0 is 0x0B and byte 1 is 0x77, set reverse_bytes = 1 */
	unsigned int reverse_bytes;
};

struct dts_frame_config {
	unsigned int dts_fr_sz_8bit;
	unsigned int sample_rate;
	unsigned char dts_type;
	unsigned int reverse_bytes;
};

struct codec_61937_config {
	enum iec_61937_code_type codec_type;
	union {
		struct ac3_frame_config ac3_fr_config;
		struct dts_frame_config dts_fr_config;
	}codec_config;
};

struct config_60958_61937{
	enum iec_61937_code_type codec_type;
	unsigned int dma_buf_sz;
	unsigned int sz_61937_burst;
	unsigned int rep_per_60958;
};

int init_60958_61937_framer(void);

int deinit_60958_61937_framer(void);

int get_60958_61937_config(struct config_60958_61937 *config_60958_61937);

int get_61937_burst(unsigned char *audio_61937_burst ,
	unsigned int sz_61937_burst,	/* size of 61937 burst in bytes */
	unsigned char *audio_frame,
	unsigned int audio_frame_sz,	/* size of audio frame in bytes */
	struct codec_61937_config *codec_61937_config);

int get_60958_frame(unsigned char *audio_60958_frame,
	unsigned int sz_60958_frame,	/* size of 60958 frames in bytes */
	unsigned char *audio_61937_burst,
	unsigned int sz_61937_burst,	/* size of 61937 burst in bytes */
	struct codec_61937_config *codec_61937_config);

int get_60958_61937_pause_burst(unsigned char *pause_burst_60958_frame,
	unsigned int sz_60958_frame,/* size of 60958 pause frame sequence in bytes */
	struct config_60958_61937 *config_60958_61937);
#endif
