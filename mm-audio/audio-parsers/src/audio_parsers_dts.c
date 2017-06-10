/* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <byteswap.h>

#include "audio_parsers.h"
#define DBG_DTS_PARSER(fmt, args...)/*   \
{ \
        fprintf(stderr, "Dts_parser: " fmt, ##args);\
}*/


struct dts_fr_sync_info {
	unsigned int   sync_word;
	unsigned short sync_word_ext; //extra 6bits for sync word
	unsigned int   sample_rate;
	unsigned int   dts_fr_sz_8bit;
	unsigned char  dts_type;
	unsigned int   reverse_bytes;
};


static unsigned char *audio_data;
static unsigned int audio_data_sz;
static unsigned int  audio_data_index = 0;
static unsigned char *frame_hdr;

static int read_dts_frame(unsigned char *dts_61937_data,
			unsigned int size_16bit,
			struct dts_fr_sync_info *sync_info)
{
	unsigned short *tmp_ptr;
        unsigned short *tmp_addr;
        unsigned short *end_of_dts_frame;
        static unsigned int sync_word_dst = 0;


        if ((size_16bit < MIN_DTS_FRA_SZ_16_BIT) ||
                        (size_16bit > MAX_DTS_FRA_SZ_16_BIT))  {
                fprintf(stderr, "invalid dts frame size %u\n", size_16bit);
                return -1;
        }

        if (audio_data_index >= audio_data_sz) {
                fprintf(stderr,"%s : end of file. audio_data_index  %u\n",
                                        __func__, audio_data_index);
                return -1;
        }

        memcpy(dts_61937_data, audio_data + audio_data_index,
                        sync_info->dts_fr_sz_8bit);

	DBG_DTS_PARSER("audio_data_index = %d\n", audio_data_index);

	if (audio_data_index == 0) {
		tmp_addr = audio_data +  sync_info->dts_fr_sz_8bit;
		if( (*tmp_addr == 0XFE7F) && *(++tmp_addr)== 0x0180 ) {
			sync_word_dst = sync_info->dts_fr_sz_8bit;
			goto end;
		}

		tmp_ptr = (unsigned short *)(audio_data) + sync_info->dts_fr_sz_8bit/2;
		end_of_dts_frame = (unsigned short *)(audio_data) +
					sync_info->dts_fr_sz_8bit/2;
		while (*tmp_ptr != 0XFE7F){
				tmp_ptr++;
		}
		DBG_DTS_PARSER("Number of padding bytes %u\n",
				((tmp_ptr - end_of_dts_frame) * 2));
		if (*tmp_ptr == 0xFE7F) {
			tmp_addr = tmp_ptr + 1;
			if (*tmp_addr == 0X0180)
			{
				DBG_DTS_PARSER("Sync word found\n");
				if(tmp_ptr - end_of_dts_frame)
					sync_word_dst =
						((tmp_ptr - end_of_dts_frame) * 2) - 1 +
						sync_info->dts_fr_sz_8bit;
			}
		}
	}
end:
        DBG_DTS_PARSER("Sync_word_distance %u\n", sync_word_dst);
	audio_data_index  = audio_data_index + sync_word_dst;
        return 0;

}

int get_dts_frame_sync_info(unsigned char *dts_sync_info_data,
			unsigned int size,
			struct dts_fr_sync_info *sync_info)
{
	int i = 0;
	char tmp = 0;
	unsigned tmp1 = 0;
	unsigned short lower_16bit = 0;
	unsigned short upper_16bit = 0;
	unsigned int sync_word = 0;

	if (size != DTS_SYNC_INFO_SZ) {
		DBG_DTS_PARSER("invalid syncinfo size\n");
		return -1;
	}

	i = 0;
        tmp1 = (unsigned short) ((dts_sync_info_data[i] << 8)
                                | dts_sync_info_data[i+1]);

        if (tmp1 == 0x7FFE) {
                sync_info->reverse_bytes = 1;
	}
        else if (tmp1 == 0xFE7F) {
                sync_info->reverse_bytes = 0;
        } else {
                fprintf(stderr, "invalid syncinfo word 0x%04x\n",
			sync_info->sync_word);
                return -1;
        }

	memcpy(&sync_word, dts_sync_info_data, 4); //copy 32bit sync word
	if (!sync_info->reverse_bytes) {
		lower_16bit = (unsigned short)(sync_word << 16);
		upper_16bit = (unsigned short)(sync_word >> 16);
		lower_16bit = ((lower_16bit >> 8) | (lower_16bit << 8));
		upper_16bit = ((upper_16bit >> 8) | (upper_16bit << 8));
		sync_info->sync_word = lower_16bit | upper_16bit;
	} else {
		sync_info->sync_word = sync_word;
	}
	i = 4;
	tmp1 = 0;
	if (sync_info->reverse_bytes) {
		tmp1 = (unsigned short)((dts_sync_info_data[i] << 8)
                                | dts_sync_info_data[i+1]);
	} else {
		tmp1 = (unsigned short)((dts_sync_info_data[i+1] << 8)
                                | dts_sync_info_data[i]);
	}
	//Need to copy only 6bits
	sync_info->sync_word_ext = (unsigned short) ((tmp1 & 0xFC00) >> 12);

	if (sync_info->sync_word != 0x0180FE7F) {
		fprintf(stderr, "sync word not matching %x \n",
			sync_info->sync_word);
		return -1;
	}
	return 0;
}

void extract_bits(unsigned char *input, unsigned char num_bits_reqd,
			unsigned short *out_value, unsigned int hdr_bit_index)
{
	unsigned int output = 0;
	unsigned int value = 0;
	unsigned int byte_index;
	unsigned char   bit_index;
	unsigned char   bits_avail_in_byte;
	unsigned char   num_to_copy;
	unsigned char   mask;
	unsigned char   num_remaining = num_bits_reqd;

	while(num_remaining) {
		byte_index = hdr_bit_index / 8;
		bit_index  = hdr_bit_index % 8;

		bits_avail_in_byte = 8 - bit_index;
		num_to_copy = MIN(bits_avail_in_byte, num_remaining);

		mask = ~(0xff << bits_avail_in_byte);

		value = input[byte_index] & mask;
		value = value >> (bits_avail_in_byte - num_to_copy);

		hdr_bit_index += num_to_copy;
		num_remaining -= num_to_copy;

		output = (output << num_to_copy) | value;
	}
	*out_value = (unsigned short)output;
}

static void memcpy_reverse_16_bit_word(unsigned char *dst_addr,
                unsigned char *src_addr, unsigned int size)
{
        unsigned int i = 0;
        unsigned short *src = (unsigned short*)src_addr;
        unsigned short *dst = (unsigned short*)dst_addr;

        while ( i < size) {

                *dst  = bswap_16(*src);
                dst++;
                src++;
                i = i + 2;
        }
}

int get_dts_frame_info(struct dts_fr_sync_info *sync_info)
{
	unsigned short samp_rate = 0;
	unsigned short frame_size = 0;
	unsigned short nblks = 0;
	unsigned short fsize = 0;
	int i = 0;
	unsigned short tmp1 = 0;
	unsigned short ch;
	unsigned short bit_rate = 0;


	frame_hdr = (unsigned char *)malloc(12); //Actual frame hdr size 86 bits
	if (frame_hdr == NULL) {
		fprintf(stderr, "Failed to allocate memory for frame hdr\n");
		return -1;
	}
	memset(frame_hdr, '0', 12);
	i = 0;
        tmp1 = (unsigned short) ((audio_data[i] << 8)
                                | audio_data[i+1]);

        if (tmp1 == 0x7FFE)
                sync_info->reverse_bytes = 0;
        else if (tmp1 == 0xFE7F)
                sync_info->reverse_bytes = 1;

	if (sync_info->reverse_bytes) {
		 memcpy_reverse_16_bit_word(frame_hdr,
				audio_data+audio_data_index+4, 11);
	} else {
		 memcpy(frame_hdr, audio_data+audio_data_index+4, 11);
	}
	/* Extract 4bits for sample rate */
	extract_bits(frame_hdr, 4, &samp_rate, 34);

	if (samp_rate == 0x01) {
		sync_info->sample_rate = 8000;
	} else if(samp_rate == 0x02) {
		sync_info->sample_rate = 16000;
	} else if(samp_rate == 0x03) {
                sync_info->sample_rate = 32000;
        } else if(samp_rate == 0x06) {
                sync_info->sample_rate = 11025;
        } else if(samp_rate == 0x07) {
                sync_info->sample_rate = 22050;
        } else if(samp_rate == 0x08) {
                sync_info->sample_rate = 44100;
        } else if(samp_rate == 0x0B) {
                sync_info->sample_rate = 12000;
        } else if(samp_rate == 0x0C) {
                sync_info->sample_rate = 24000;
        } else if(samp_rate == 0x0D) {
                sync_info->sample_rate = 48000;
        } else {
		fprintf(stderr, "Invalid sample rate %d\n", samp_rate);
		return -1;
        }

	/* Extract 7bits for no of PCM sample blocks */
	extract_bits(frame_hdr, 7, &nblks, 7);
	if (((nblks+1) * 32) == 512) {
		DBG_DTS_PARSER("DTS TYPE 1\n");
		sync_info->dts_type = DTS_TYPE_1;
	} else if (((nblks+1) * 32) == 1024) {
		DBG_DTS_PARSER("DTS TYPE 2\n");
		sync_info->dts_type = DTS_TYPE_2;
	} else if (((nblks+1) * 32) == 2048) {
		DBG_DTS_PARSER("DTS TYPE 3\n");
		sync_info->dts_type = DTS_TYPE_3;
	} else {
		fprintf(stderr, "Invalid DTS type\n");
	}
	/* Extract 14bits for frame size */
	extract_bits(frame_hdr, 14, &fsize, 14);
	sync_info->dts_fr_sz_8bit = fsize + 1;

	/* Extract 6bits for channel arrangement */
	extract_bits(frame_hdr, 6, &ch, 28);

	/* Extract 5bits for bit rate */
	extract_bits(frame_hdr, 5, &bit_rate, 38);

	DBG_DTS_PARSER("Sample rate %u frame size = %u channel info = %u \
			bitrate = %u\n", sync_info->sample_rate,
			sync_info->dts_fr_sz_8bit, ch, bit_rate);
	free(frame_hdr);
	return 0;
}

static void install_bits(unsigned char *input,
		unsigned char num_bits_reqd,
		unsigned char value,
		unsigned char *hdr_bit_index)
{
	unsigned int byte_index;
	unsigned char bit_index;
	unsigned char bits_avail_in_byte;
	unsigned char num_to_copy;
	unsigned char   byte_to_copy;

	unsigned char   num_remaining = num_bits_reqd;
	unsigned char  bit_mask;

	bit_mask = 0xFF;

	while (num_remaining) {
		byte_index = (*hdr_bit_index) >> 3;
		bit_index  = (*hdr_bit_index) &  0x07;

		bits_avail_in_byte = 8 - bit_index;

		num_to_copy = MIN(bits_avail_in_byte, num_remaining);

		byte_to_copy = ((unsigned char)((value >> (num_remaining - num_to_copy)) & 0xFF) <<
		(bits_avail_in_byte - num_to_copy));
		if (num_to_copy == num_remaining)
			input[byte_index] &= ((unsigned char)(bit_mask >> num_to_copy));
		else
			input[byte_index] &= ((unsigned char)(bit_mask << num_to_copy));
		input[byte_index] |= byte_to_copy;

		*hdr_bit_index += num_to_copy;

		num_remaining -= num_to_copy;
	}
}

void get_silent_frame(unsigned char *silent_frame)
{
	/* 32 bits for sync word, after that skip 14bits
	* to edit FSIZE
	*/
	unsigned char temp = 46;
	memcpy(silent_frame, audio_data + audio_data_index, 11);
	/* Set FSIZE i.e. 14bits to 0 */
	install_bits(silent_frame, 14, 0, &temp);
}

int get_dts_first_frame_info(struct audio_parser_codec_info *audio_codec_info)
{
	unsigned char dts_sync_info_data[DTS_SYNC_INFO_SZ];
	struct dts_fr_sync_info sync_info;
	int rc = 0;

	memcpy(dts_sync_info_data, audio_data,
		DTS_SYNC_INFO_SZ);
	rc = get_dts_frame_sync_info(dts_sync_info_data,
					DTS_SYNC_INFO_SZ,
					&sync_info);
	if (rc == -1) {
		fprintf(stderr, "return val of sync_info %d \n", rc);
		return rc;
	}

	rc = get_dts_frame_info(&sync_info);
	if (rc == -1)
		return rc;
	audio_codec_info->codec_config.dts_fr_info.sample_rate =
						sync_info.sample_rate;
	audio_codec_info->codec_config.dts_fr_info.dts_type =
						sync_info.dts_type;
	return 0;
}

int get_dts_frame(unsigned char *frame, unsigned int sz,
		struct audio_parser_codec_info *audio_codec_info)
{
	unsigned char dts_sync_info_data[DTS_SYNC_INFO_SZ];
	struct dts_fr_sync_info sync_info;
	int rc = 0;

	memset(frame, 0, sz);

	if (audio_data_index < audio_data_sz) {
		memcpy(dts_sync_info_data, audio_data + audio_data_index,
				DTS_SYNC_INFO_SZ);
		rc = get_dts_frame_sync_info(dts_sync_info_data,
					DTS_SYNC_INFO_SZ,
					&sync_info);
		if (rc == -1) {
			fprintf(stderr, "return val of sync_info %d \n", rc);
			return rc;
		}

		rc = get_dts_frame_info(&sync_info);
		if (rc == -1)
			return rc;
		audio_codec_info->codec_type = AUDIO_PARSER_CODEC_DTS;

		audio_codec_info->codec_config.dts_fr_info.dts_fr_sz_8bit =
			sync_info.dts_fr_sz_8bit;

		audio_codec_info->codec_config.dts_fr_info.sample_rate =
				sync_info.sample_rate;

		audio_codec_info->codec_config.dts_fr_info.dts_type =
				sync_info.dts_type;

		audio_codec_info->codec_config.dts_fr_info.reverse_bytes =
				!sync_info.reverse_bytes;

		rc = read_dts_frame(frame, sz / 2, &sync_info);

		if (rc == -1) {
			fprintf(stderr, "%s No frame. errors\n" ,__func__);
			return -1;
		}
		return 0;

	} else {
		fprintf(stderr, "audio_data_index %d audio_data_sz %d ",
			audio_data_index, audio_data_sz);
		return -1;
	}
}

void set_bits(unsigned char *input,
		unsigned char num_bits_reqd,
		unsigned char value,
		unsigned char *hdr_bit_index)
{

	install_bits(input, num_bits_reqd, value,
			hdr_bit_index);
}
static void init_chan_status(unsigned char * ch_status,
		struct audio_parser_channel_staus *audio_channel_status)
{
	int bit_index;

	bit_index = 0;
	/* block start bit in preamble bit 3 */
	install_bits(ch_status, 1, 1, &bit_index);
	bit_index = 1;
	if (audio_channel_status->data_type == non_linear) {
		install_bits(ch_status, 1, 1, &bit_index);
	}
	bit_index = 24;
	switch (audio_channel_status->sample_rate) {

	case 0x03:		// 32k --- 1100 in 24..27
		install_bits(ch_status, 4, 0x0C, &bit_index);
		break;
	case 0x08:		//44.1k -- 0000 in 24..27
		break;
	case 0x0D:		//48k -- 0100 in 24..27
		install_bits(ch_status, 4, 0x04, &bit_index);
		break;
	case 0x01:		//8k
	case 0x02:		//16k
	case 0x06:		//11.025k
	case 0x07:		//22.05k
	case 0x0B:		//12k
	case 0x0C:		//24k
	default:
		fprintf(stderr, "%s Invalid sample_rate %u\n", __func__,
				audio_channel_status->sample_rate);
		break;
	}

}
int get_dts_frame_channel_info(struct audio_parser_channel_staus *audio_channel_status)
{
	unsigned short samp_rate = 0;
	unsigned short frame_size = 0;
	int i = 0;
	unsigned short tmp1 = 0;
	unsigned short bit_rate = 0;
	int reverse_bytes = 0;

	//non-linear pcm
	audio_channel_status->data_type = non_linear;

	frame_hdr = (unsigned char *)malloc(12); //Actual frame hdr size 86 bits
	if (frame_hdr == NULL) {
		fprintf(stderr, "Failed to allocate memory for frame hdr\n");
		return -1;
	}
	memset(frame_hdr, '0', 12);
	i = 0;
        tmp1 = (unsigned short) ((audio_data[i] << 8)
                                | audio_data[i+1]);

        if (tmp1 == 0x7FFE)
                reverse_bytes = 0;
        else if (tmp1 == 0xFE7F)
                reverse_bytes = 1;

	if (reverse_bytes) {
		 memcpy_reverse_16_bit_word(frame_hdr,
				audio_data+audio_data_index+4, 11);
	} else {
		 memcpy(frame_hdr, audio_data+audio_data_index+4, 11);
	}
	/* Extract 4bits for sample rate */
	extract_bits(frame_hdr, 4, &samp_rate, 34);
	audio_channel_status->sample_rate =  samp_rate;
	/* Extract 5bits for bit rate */
	extract_bits(frame_hdr, 5, &bit_rate, 38);
	audio_channel_status->bit_rate =  bit_rate;

	free(frame_hdr);
	return 0;
}

int get_channel_status_dts(unsigned char *channel_status)
{
	int no_of_bytes;
	int rc;
	struct audio_parser_channel_staus audio_channel_status;
	memset(&audio_channel_status, 0, sizeof(audio_channel_status));
	rc = get_dts_frame_channel_info(&audio_channel_status);
	if(rc != -1) {
	    init_chan_status(channel_status, &audio_channel_status);
	}
	return 0;
}

void init_audio_parser_dts(unsigned char *audio_stream_data,
				unsigned int audio_stream_sz)
{
	audio_data = audio_stream_data;
	audio_data_sz = audio_stream_sz;
	audio_data_index = 0;
}
