/* Copyright (c) 2011, 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef ACDB_LOADER_H

#define REGISTER_DEPTH 8

#define MAX_ANC_WRITES 305

struct storage_adie_codec_anc_data {
	uint32_t size;
	uint32_t writes[MAX_ANC_WRITES];
};

enum adie_codec_anc_channel {
	ADIE_CODEC_ANC_CHANNEL_1 = 0,
	ADIE_CODEC_ANC_CHANNEL_2,
	ADIE_CODEC_ANC_CHANNEL_NUM
};

struct adie_codec_anc_freq_setting {
	uint32_t    freq;
	uint32_t    setting;
	uint32_t    scaleA;
	uint32_t    scaleB;
	uint32_t    scaleC;
	uint32_t    scaleD;
};

static struct adie_codec_anc_freq_setting ancFreqTable[] = {
	{ 512, 1, 1, 65535, 1, 65535},
	{ 705, 1, 1, 65535, 1, 65535},
	{ 1024, 2, 1, 65535, 8, 8192},
	{ 1411, 2, 1, 65535, 8, 8192},
	{ 1536, 3, 3, 21845, 9, 7282},
	{ 2048, 4, 4, 16384, 16, 4096},
	{ 2822, 4, 4, 16384, 16, 4096},
	{ 3072, 6, 9, 7282, 24, 2731},
	{ 4096, 8, 16, 4096, 32, 2048},
	{ 5644, 8, 16, 4096, 32, 2048},
	{ 6144, 12, 36, 1820, 48, 1365},
	{ 8192, 16, 64, 1024, 64, 1024},
	{ 11289, 16, 64, 1024, 64, 1024},
	{ 12288, 24, 144, 455, 96, 683}
};

#define MAX_INT 2147483647
#define ANC_COEFF_INT_BITS 2
#define ANC_COEFF_FRAC_BITS 13
#define ANC_GAIN_TABLE_OFFSET 12
#define ANC_GAIN_TABLE_FRAC_BITS 13
#define ANC_OSR	  256
#define ANC_FREQ  48000
#define NUM_ANC_COMPONENTS 2

struct adie_codec_anc_gain_table {
	int32_t gain;
	uint32_t multiplier;
};

static struct adie_codec_anc_gain_table ancGainTable[] = {
	{(int32_t) (-6 * 2), (int32_t) (0.501183091 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-5.5 * 2),  (int32_t) (0.530880422 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-5 * 2),    (int32_t) (0.562337452 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-4.5 * 2),  (int32_t) (0.595658451 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-4 * 2),    (int32_t) (0.630953868 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-3.5 * 2), (int32_t) (0.668340695 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-3 * 2),    (int32_t) (0.707942859 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-2.5 * 2),  (int32_t) (0.749891627 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-2 * 2),    (int32_t) (0.794326046 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-1.5 * 2),  (int32_t) (0.841393403 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-1 * 2),    (int32_t) (0.89124971 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (-0.5 * 2),  (int32_t) (0.944060226 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (0 * 2),     (int32_t)(1 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (0.5 * 2),   (int32_t) (1.059254455 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (1 * 2),     (int32_t) (1.12202 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (1.5 * 2),   (int32_t) (1.188504683 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (2 * 2),     (int32_t) (1.25892888 *	(1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (2.5 * 2),   (int32_t) (1.333526025 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (3 * 2),     (int32_t) (1.412543382 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (3.5 * 2),   (int32_t) (1.49624287 *	(1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (4 * 2),     (int32_t) (1.584901926 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (4.5 * 2),   (int32_t) (1.678814425 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (5 * 2),     (int32_t) (1.778291659 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (5.5 * 2),   (int32_t) (1.883663362 * (1 << ANC_GAIN_TABLE_FRAC_BITS))},
	{(int32_t) (6 * 2),     (int32_t) (1.995278807 * (1 << ANC_GAIN_TABLE_FRAC_BITS))}
};

#endif /* ACDB_LOADER_H */
