/* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <byteswap.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "iec_60958_61937.h"

#define DBG_IEC_LIB(fmt, args...)/*   \
{ \
	fprintf(stderr, "IEC_60958: " fmt, ##args);\
}*/


#define NUM_FRA_IN_BLOCK		192
#define FRAME_60958_SZ			8	/* bytes */
#define	PREABLE_61937_SZ_16_BIT		4	/* in 16 bit words */
#define NUM_SUB_FRAME_PER_60958_FRAME	2
#define AC3_PAUSE_REP_PER		3
/* Pause repetition period is same for all the DTS types */
#define DTS_PAUSE_REP_PER		3

#define SZ_61937_DATA_IN_60958_SUB_FRAME     2
#define SZ_PAUSE_BURST_PAYLOAD   32 /* bits*/
#define SZ_16_BIT_PAUSE_BURST     6

#define HDMI_IEC60958_P_BIT31          31
#define HDMI_IEC60958_C_BIT30          30
#define HDMI_IEC60958_U_BIT29          29
#define HDMI_IEC60958_V_BIT28          28

#define HDMI_IEC60958_PREAMBLE_BIT3    3
#define HDMI_IEC60958_PREAMBLE_BIT2    2
#define HDMI_IEC60958_PREAMBLE_BIT1    1
#define HDMI_IEC60958_PREAMBLE_BIT0    0

#define HDMI_IEC60958_AUX_START_BIT4   4
#define HDMI_IEC60958_MSB_ALIGN_BIT8   8

#define PAUSE_BURST 0
#define DATA_BURST  1
#define NULL_BURST  2


enum hmdi_data_type {
	pcm_linear,
	non_linear
};

unsigned int channel_status[NUM_FRA_IN_BLOCK];
struct dts_frame_config  cur_dts_fr_config;
struct ac3_frame_config  cur_ac3_fr_config;
unsigned int  prev_fr_sample_rate = 0;

#ifdef LOG_IEC_60958_FRAMES
FILE *w_fp;
#endif

#ifdef LOG_IEC_61937_BURST
FILE *burst_fp;
#endif

static unsigned short pause_burst[SZ_16_BIT_PAUSE_BURST];
static unsigned int cur_frame = 0;

int get_60958_61937_config(struct config_60958_61937 *config_60958_61937)
{
	switch (config_60958_61937->codec_type) {

	case IEC_61937_CODEC_AC3:
		DBG_IEC_LIB("IEC_61937_CODEC_AC3\n");
		config_60958_61937->sz_61937_burst = 2 *
			(MAX_AC3_FRA_SZ_16_BIT + PREABLE_61937_SZ_16_BIT);

		config_60958_61937->rep_per_60958 =
			AC3_REP_PER * FRAME_60958_SZ;
		break;
	case IEC_61937_CODEC_DTS_I:
		DBG_IEC_LIB("IEC_61937_CODEC_DTS_I\n");
		config_60958_61937->sz_61937_burst = 2 *
			(MAX_DTS_FRA_SZ_16_BIT + PREABLE_61937_SZ_16_BIT);

		config_60958_61937->rep_per_60958 =
			DTS_TYPE_I_REP_PER * FRAME_60958_SZ;
		break;
	case IEC_61937_CODEC_DTS_II:
		DBG_IEC_LIB("IEC_61937_CODEC_DTS_II\n");
		config_60958_61937->sz_61937_burst = 2 *
			(MAX_DTS_FRA_SZ_16_BIT + PREABLE_61937_SZ_16_BIT);
		config_60958_61937->rep_per_60958 =
			DTS_TYPE_II_REP_PER * FRAME_60958_SZ;
		break;
	case IEC_61937_CODEC_DTS_III:
		DBG_IEC_LIB("IEC_61937_CODEC_DTS_III\n");
		config_60958_61937->sz_61937_burst = 2 *
			(MAX_DTS_FRA_SZ_16_BIT + PREABLE_61937_SZ_16_BIT);
		config_60958_61937->rep_per_60958 =
			DTS_TYPE_III_REP_PER * FRAME_60958_SZ;
		break;
	default:
		fprintf(stderr, "un supported codec %d\n",
					config_60958_61937->codec_type);
		return -1;

	}
	config_60958_61937->dma_buf_sz = config_60958_61937->rep_per_60958;
	return 0;
}

void init_chan_status(unsigned int sample_rate, enum hmdi_data_type data_type)
{
	int i;
	unsigned int *ch_status = channel_status;

	memset(ch_status, 0, sizeof(channel_status));

	for (i = 0; i < NUM_FRA_IN_BLOCK; i++) {
		ch_status[i] = 0;
	}

	DBG_IEC_LIB("%s sample_rate %u, data_type %u\n", __func__,
			sample_rate,data_type);

	/* block start bit in preamble bit 3 */
	ch_status[0] = 1UL << HDMI_IEC60958_PREAMBLE_BIT3;

	if (data_type == non_linear) {

	/*The purpose of channel status bit 1 is to indicate if IEC 60958
	 * is used to convey linear PCM or non-linear PCM. This bit shall
	 * be set to '1' when IEC 60958 is used to convey non-linear PCM
	 * encoded audio bitstreams
	*/
		ch_status[1] = 1UL << HDMI_IEC60958_C_BIT30;
	}

	switch (sample_rate) {
	case 32000:		// 1100 in 24..27
		ch_status[24] = 1UL << HDMI_IEC60958_C_BIT30;
		ch_status[25] = 1UL << HDMI_IEC60958_C_BIT30;
		break;
	case 44100:		// 0000 in 24..27
		break;
	case 88200:		// 0001 in 24..27
		ch_status[27] = 1UL << HDMI_IEC60958_C_BIT30;
		break;
	case 176400:		// 0011 in 24..27
		ch_status[26] = 1UL << HDMI_IEC60958_C_BIT30;
		ch_status[27] = 1UL << HDMI_IEC60958_C_BIT30;
		break;
	case 48000:		// 0100 in 24..27
		ch_status[25] = 1UL << HDMI_IEC60958_C_BIT30;
		break;
	case 96000:		// 0101 in 24..27
		ch_status[25] = 1UL << HDMI_IEC60958_C_BIT30;
		ch_status[27] = 1UL << HDMI_IEC60958_C_BIT30;
		break;
	case 192000:		// 0111 in 24..27
		ch_status[25] = 1UL << HDMI_IEC60958_C_BIT30;
		ch_status[26] = 1UL << HDMI_IEC60958_C_BIT30;
		ch_status[27] = 1UL << HDMI_IEC60958_C_BIT30;
		break;
	default:
		fprintf(stderr, "%s Invalid sample_rate %u\n", __func__,
				sample_rate);

		break;
	}
}


static void memcpy_reverse_16_bit_word(unsigned char *audio_61937_burst,
		unsigned char *audio_frame, unsigned int audio_frame_sz)
{
	unsigned int i = 0;
	unsigned short *src = (unsigned short*)audio_frame;
	unsigned short *dst = (unsigned short*)audio_61937_burst;

	while ( i < audio_frame_sz) {

		*dst  = bswap_16(*src);
		dst++;
		src++;
		i = i + 2;
	}
}

int get_61937_burst(unsigned char *audio_61937_burst,
		unsigned int sz_61937_burst,
		unsigned char *audio_frame, unsigned int audio_frame_sz,
		struct codec_61937_config *codec_61937_config)
{
	unsigned short *dst;
	unsigned int dst_index;
	unsigned char *ch;
	unsigned short *audio_61937_burst_16_bit_word;
	struct dts_frame_config dts_fr_config;
	struct ac3_frame_config ac3_fr_config;

	if ( (audio_frame_sz + 8) >  (sz_61937_burst)) {

		fprintf(stderr, "audio_frame_sz + 8 is greate than "
			"61937_burst_size.\n");
		fprintf(stderr, "audio_frame_sz + 8 = %u "
			"  61937_burst_size %u\n", audio_frame_sz + 8,
			sz_61937_burst);
			return  -1;
	}

	memset(audio_61937_burst, 0, sz_61937_burst);

	audio_61937_burst_16_bit_word = (unsigned short *)audio_61937_burst;

	audio_61937_burst_16_bit_word[0] = 0xF872;	/* 61937 Preamble Pa*/
	audio_61937_burst_16_bit_word[1] = 0x4E1F;	/* 61937 Preamble Pb*/

	switch(codec_61937_config->codec_type) {
	case IEC_61937_CODEC_AC3:
		ac3_fr_config = codec_61937_config->codec_config.ac3_fr_config;

		audio_61937_burst_16_bit_word[2] = IEC_61937_CODEC_AC3 |
				((ac3_fr_config.bsmod << 8) & 0x0700);

		/* AC-3 frame size in bits*/
		audio_61937_burst_16_bit_word[3] =
		ac3_fr_config.ac3_fr_sz_16bit * 16;

		/* copy the frame after leaving first 8 bytes for
		61937 premeable
		*/
		if (ac3_fr_config.reverse_bytes) {

			memcpy_reverse_16_bit_word(audio_61937_burst + 8,
						audio_frame, audio_frame_sz);
		} else {
			memcpy(audio_61937_burst + 8, audio_frame,
			audio_frame_sz);
		}
		break;
	case IEC_61937_CODEC_DTS_I:
	case IEC_61937_CODEC_DTS_II:
	case IEC_61937_CODEC_DTS_III:
		dts_fr_config = codec_61937_config->codec_config.dts_fr_config;

		if (codec_61937_config->codec_type == IEC_61937_CODEC_DTS_I)
			audio_61937_burst_16_bit_word[2] =
				IEC_61937_CODEC_DTS_I; //DTS type 1
		if (codec_61937_config->codec_type == IEC_61937_CODEC_DTS_II)
			audio_61937_burst_16_bit_word[2] =
				IEC_61937_CODEC_DTS_II; //DTS type 2
		if (codec_61937_config->codec_type == IEC_61937_CODEC_DTS_III)
			audio_61937_burst_16_bit_word[2] =
				IEC_61937_CODEC_DTS_III; //DTS type 3

		/* DTS frame size in bits -> word Pd of the 61937 preamble*/
		audio_61937_burst_16_bit_word[3] =
			dts_fr_config.dts_fr_sz_8bit * 8;
		if(!dts_fr_config.reverse_bytes) {
			memcpy(audio_61937_burst + 8, audio_frame,
				audio_frame_sz);
		} else {
			memcpy_reverse_16_bit_word(audio_61937_burst + 8,
						audio_frame, audio_frame_sz);
		}
		break;
	default:
		fprintf(stderr, "un supported codec %d\n",
				codec_61937_config->codec_type);

		return -1;
	}

#ifdef LOG_IEC_61937_BURST
	dst  = (unsigned short *)audio_61937_burst;
	dst_index = 0;
	fprintf(burst_fp,"%04hX  ",*dst);
	dst++;
	dst_index++;

	while (dst_index < (sz_61937_burst / 2)) {

		if (dst_index % 4 == 0)
			fprintf(burst_fp,"\n");

		fprintf(burst_fp,"%04hX  ",*dst);

		dst++;
		dst_index++;
	}
#endif
	return  0;
}

static int fill_60958_frames(unsigned short *src , unsigned int src_sz_16bit,
			unsigned char *hdmi_non_l_rep_per, unsigned int buf_sz,
			unsigned int cur_frame, int data_type)
{
	unsigned int src_index = 0;
	unsigned int subframe;
	unsigned int packed_data, packed_data_1;
	unsigned int align_shift;
	unsigned int data_mask;
	unsigned int *dst;
	unsigned int dst_index = 0;
	unsigned short data_61937 = 0;
	unsigned int i = 0;

	dst  = (unsigned int*)hdmi_non_l_rep_per;

	align_shift =
		HDMI_IEC60958_MSB_ALIGN_BIT8 + HDMI_IEC60958_AUX_START_BIT4;
	data_mask = 0x0000FFFF;

	src_index = 0;
	dst_index = 0;
	DBG_IEC_LIB("%s:cur frame index %u\n", __func__, cur_frame);
	switch (data_type){
	case DATA_BURST:
		DBG_IEC_LIB("DATA_BURST: dst_sz %u\n", buf_sz);
		while (dst_index < buf_sz) {
			subframe = 0;
			while (subframe <= 1 ) {

				if (src_index < src_sz_16bit) {
					data_61937 = *src;
					src++;
					src_index++;
				} else
					data_61937 = 0;

				packed_data = (unsigned int)  (data_61937 & data_mask);
				packed_data <<= align_shift;
				packed_data |= channel_status[cur_frame];

				/* set validity flag to 1 for non-linear PCM */

				packed_data |= 1UL << HDMI_IEC60958_V_BIT28;

				// calculate parity
				// parity bit carries from bit 4 to 31 inclusive,
				// carry an even number of ones and an even number
				// of zeros

				packed_data_1 = packed_data & 0xFFFFFFF0;
				packed_data_1 ^= packed_data_1 >> 1;
				packed_data_1 ^= packed_data_1 >> 2;
				packed_data_1 =
					(packed_data_1 & 0x11111111U) * 0x11111111U;

				// fill P bit, Parity value at bit28
				*dst++ =
					packed_data |
						((packed_data_1 << 3) & 0x80000000);
				dst_index = dst_index + 4;

				subframe++;
				DBG_IEC_LIB("data_61937 %04x dst %08x "
					"dst_index %u subframe %u\n",
					data_61937, *dst, dst_index, subframe);
			}
			// HDMI channel status cycle every 192 frames
			cur_frame++;
			if (cur_frame == NUM_FRA_IN_BLOCK) {
					cur_frame = 0;
				DBG_IEC_LIB("resetting cur_frame index \n");
			}
		}
		break;
	case PAUSE_BURST: {
		DBG_IEC_LIB("PAUSE_BURST: dst_sz %u\n", buf_sz);
		unsigned short tmp[6] = {0, 0, 0, 0, 0,0};
		while (dst_index < buf_sz) {
			subframe = 0;
			while ((subframe <= 1) && (dst_index < buf_sz) ) {
				data_61937 = src[src_index];
				DBG_IEC_LIB("sub_frame_index %u: src val %04x \n",
					subframe, data_61937);
				src_index++;

				packed_data = (unsigned int)  (data_61937 & data_mask);
				packed_data <<= align_shift;
				packed_data |= channel_status[cur_frame];

				/* set validity flag to 1 for non-linear PCM */

				packed_data |= 1UL << HDMI_IEC60958_V_BIT28;

				// calculate parity
				// parity bit carries from bit 4 to 31 inclusive,
				// carry an even number of ones and an even number
				// of zeros

				packed_data_1 = packed_data & 0xFFFFFFF0;
				packed_data_1 ^= packed_data_1 >> 1;
				packed_data_1 ^= packed_data_1 >> 2;
				packed_data_1 =
					(packed_data_1 & 0x11111111U) * 0x11111111U;

				// fill P bit, Parity value at bit28
				*dst++ =
					packed_data |
						((packed_data_1 << 3) & 0x80000000);

				subframe++;
				dst_index = dst_index + 4;
				if (src_index == SZ_16_BIT_PAUSE_BURST)
				{
					DBG_IEC_LIB("reset src index\n");
					src_index = 0;
					if((buf_sz - dst_index) < 24) {
						DBG_IEC_LIB("Buf sz remaining < 24\n");
						src = tmp;
					}
				}

			}
			 // HDMI channel status cycle every 192 frames
			cur_frame++;
			if (cur_frame == NUM_FRA_IN_BLOCK) {
					cur_frame = 0;
				DBG_IEC_LIB("resetting cur_frame index \n");
			}

		}
		break;
	}
	default:
		fprintf(stderr, "Invalid data_type\n");
		return -1;
	}

#ifdef LOG_IEC_60958_FRAMES
        dst  = (unsigned int*)hdmi_non_l_rep_per;
        dst_index = 0;

        fprintf(w_fp, "\n");
        fprintf(w_fp,"%08X  ",*dst);
        dst++;
        dst_index = dst_index + 4;

        while (dst_index < buf_sz) {

                if (dst_index % 16 == 0)
                         fprintf(w_fp,"\n");

                fprintf(w_fp,"%08X  ",*dst);

                dst++;
                dst_index = dst_index + 4;
        }
#endif

	return cur_frame;

}

void init_60958_frame(unsigned short *src , unsigned int src_sz_16bit,
		unsigned char *hdmi_non_l_rep_per,
		unsigned int sz_audio_60958_frame)
{
	//unsigned int cur_frame = 0;
	unsigned int *dst;
	unsigned int dst_index = 0;

	memset(hdmi_non_l_rep_per, 0, sz_audio_60958_frame);

	cur_frame =  fill_60958_frames(src ,src_sz_16bit,
					hdmi_non_l_rep_per,
					sz_audio_60958_frame,
					cur_frame, DATA_BURST);
}

int get_60958_frame(unsigned char *audio_60958_frame,
		unsigned int sz_60958_frame,
		unsigned char *audio_61937_burst,
		unsigned int sz_61937_burst,
		struct codec_61937_config *codec_61937_config)
{
	int rc;
	unsigned short *audio_61937_burst_16_bit_word;
	unsigned short frame_sz_16bit = 0;

	switch (codec_61937_config->codec_type){
	case IEC_61937_CODEC_AC3:
		 cur_ac3_fr_config =
			codec_61937_config->codec_config.ac3_fr_config;

		if ((prev_fr_sample_rate == 0) ||
		(cur_ac3_fr_config.sample_rate != prev_fr_sample_rate)) {

				init_chan_status(cur_ac3_fr_config.sample_rate,
						non_linear);

				prev_fr_sample_rate =
					cur_ac3_fr_config.sample_rate;
		}

		audio_61937_burst_16_bit_word =
				(unsigned short*)(audio_61937_burst);

		DBG_IEC_LIB("frame's first two bytes : "
			"61937_burst[8] = 0x%x  [9] = 0x%x,\n"
			"audio_word[4] = 0x%hx bsmod = %d frame_size = %u"
			" sample_rate = %u\n",
			(unsigned int)audio_61937_burst[8],
			(unsigned int)audio_61937_burst[9],
			audio_61937_burst_16_bit_word[4],
			cur_ac3_fr_config.bsmod,
			cur_ac3_fr_config.ac3_fr_sz_16bit,
			cur_ac3_fr_config.sample_rate);

		init_60958_frame(audio_61937_burst_16_bit_word,
			cur_ac3_fr_config.ac3_fr_sz_16bit + PREABLE_61937_SZ_16_BIT,
			audio_60958_frame, sz_60958_frame);
		break;
	case IEC_61937_CODEC_DTS_I:
	case IEC_61937_CODEC_DTS_II:
	case IEC_61937_CODEC_DTS_III:
		cur_dts_fr_config =
			codec_61937_config->codec_config.dts_fr_config;

		if(cur_dts_fr_config.dts_fr_sz_8bit % 2)
			frame_sz_16bit = cur_dts_fr_config.dts_fr_sz_8bit/2 + 1;
		else
			frame_sz_16bit = cur_dts_fr_config.dts_fr_sz_8bit/2;
		if ((prev_fr_sample_rate == 0) ||
		(cur_dts_fr_config.sample_rate != prev_fr_sample_rate)) {

				init_chan_status(cur_dts_fr_config.sample_rate,
						non_linear);

				prev_fr_sample_rate =
					cur_dts_fr_config.sample_rate;
		}

		audio_61937_burst_16_bit_word =
			(unsigned short*)(audio_61937_burst);

		init_60958_frame(audio_61937_burst_16_bit_word,
				frame_sz_16bit + PREABLE_61937_SZ_16_BIT,
				audio_60958_frame, sz_60958_frame);
		break;
	default:
		fprintf(stderr, "un supported codec %d\n",
			codec_61937_config->codec_type);
		return -1;
	}

	return  sz_60958_frame;
}

int get_60958_61937_pause_burst(unsigned char *pause_burst_60958_frame,
		unsigned int sz_audio_60958_frame,
		struct config_60958_61937 *config_60958_61937)
{
	unsigned int num_of_16_bit_61937_words ,i;

	unsigned short* dst;
	unsigned int dst_index;

	switch (config_60958_61937->codec_type) {

	case IEC_61937_CODEC_AC3:
                num_of_16_bit_61937_words =
                        AC3_REP_PER * NUM_SUB_FRAME_PER_60958_FRAME;
                break;
	case IEC_61937_CODEC_DTS_I:
		num_of_16_bit_61937_words =
			DTS_TYPE_I_REP_PER * NUM_SUB_FRAME_PER_60958_FRAME;
		break;
	case IEC_61937_CODEC_DTS_II:
		num_of_16_bit_61937_words =
			DTS_TYPE_II_REP_PER * NUM_SUB_FRAME_PER_60958_FRAME;
		break;
	case IEC_61937_CODEC_DTS_III:
		num_of_16_bit_61937_words =
			DTS_TYPE_III_REP_PER * NUM_SUB_FRAME_PER_60958_FRAME;
		break;
	default:
		fprintf(stderr, "un supported codec %d\n",
					config_60958_61937->codec_type);
		return -1;

	}

	pause_burst[0] = 0xF872; /* 61937 Preamble Pa*/
	pause_burst[1] = 0x4E1F;

	pause_burst[2] = 0x0003;
	pause_burst[3] = SZ_PAUSE_BURST_PAYLOAD;
	pause_burst[4] = 0;
	pause_burst[5] = 0;

	cur_frame = fill_60958_frames(pause_burst, SZ_16_BIT_PAUSE_BURST,
					pause_burst_60958_frame,
					config_60958_61937->dma_buf_sz,
					cur_frame, PAUSE_BURST);
	return 0;
}

int init_60958_61937_framer(void)
{
	DBG_IEC_LIB("%s\n", __func__);

	init_chan_status(48000, non_linear);


#ifdef LOG_IEC_60958_FRAMES
	w_fp = fopen("/sdcard/audio_60958_frames","w");
	if (w_fp == NULL) {
		fprintf(stderr, "cannot open audio_61937_burst for write\n");
		return -1;
	}
#endif
#ifdef LOG_IEC_61937_BURST
	burst_fp = fopen("/sdcard/audio_61937_burst","w");
	if (burst_fp == NULL) {
		fprintf(stderr, "cannot open audio_61937_burst for write\n");
		return -1;
	}
#endif
	prev_fr_sample_rate = 0;
	cur_frame = 0;

	return 0;
}

int deinit_60958_61937_framer(void)
{

#ifdef LOG_IEC_60958_FRAMES
	fclose(w_fp);
#endif
#ifdef LOG_IEC_61937_BURST
	fclose(burst_fp);
#endif
	return 0;
}
