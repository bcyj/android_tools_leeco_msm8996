/******************************************************************************
@file  postproctest.c
@brief This file contains test code to verify display post processing
functionalities of msm_fb for MDP.

DESCRIPTION
Mobile Display Processor (MDP) provides sophisticated image processing features
for display panel to enhance display picture quality. This file contains
implementation of display post processing tests.

-----------------------------------------------------------------------------
Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
-----------------------------------------------------------------------------

******************************************************************************/

#include "postproctest.h"


#define MAX_CFG_LINE_LEN    4096
#define MAX_TOKENS_REQUIRED 2
#define TOKEN_DELIM_STR     " "
#define TOKEN_PARAMS_DELIM  ";"
#define TOKEN_HASH_DELIM_STR "#"

#define TOKEN_BLOCK         0
#define TOKEN_HUE           1
#define TOKEN_SATURATION    2
#define TOKEN_INTENSITY     3
#define TOKEN_CONTRAST      4
#define TOKEN_OPS           5
#define TOKEN_PP_TIMEOUT    1000

#define ARGC_CALIB_DATA_MAX_FIELDS	4
#define HSIC_CALIB_DATA_MAX_FIELDS	3
#define HSIC_CALIB_DATA_MAX_LINES	3
#define CALIB_DATA_FIELD_SEPARATOR	" "
#define GM_CALIB_DATA_MAX_FIELDS_T0	125
#define GM_CALIB_DATA_MAX_FIELDS_T1	100
#define GM_CALIB_DATA_MAX_FIELDS_T2	80
#define GM_CALIB_DATA_MAX_FIELDS_T3	100
#define GM_CALIB_DATA_MAX_FIELDS_T4	100
#define GM_CALIB_DATA_MAX_FIELDS_T5	80
#define GM_CALIB_DATA_MAX_FIELDS_T6	64
#define GM_CALIB_DATA_MAX_FIELDS_T7	80

#define PCC_COEFF_START_INDEX 2

#define PA_V2_LENGTH		8

#define IGC_LENGTH          256
#define IGC_MASK            0xFFF
#define IGC_C1_SHIFT        16
#define IGC_COLORS          3

#define HIST_LUT_LENGTH         256
#define HIST_LUT_MASK           0xFF
#define HIST_LUT_C1_SHIFT       8
#define HIST_LUT_C2_SHIFT       16
#define HIST_LUT_COLORS         3

#define QSEED_TABLE_2_MAX	1024
#define QSEED_TABLE_1_MAX	2

#define PP_OPS_WRITE		0
#define PP_OPS_READ		1

#define MAX_FRAMEWORK_TOKENS_REQUIRED	3
#define MIN_HSIC_PARAMS_REQUIRED	9
#define MIN_PCC_PARAMS_REQUIRED		35
#define MIN_HIST_OVERLAY_CFG_PARAMS_REQ 5
#define MIN_HIST_START_PARAMS_REQUIRED	4
#define MIN_HIST_STOP_PARAMS_REQUIRED	1
#define MIN_HIST_READ_PARAMS_REQUIRED	3
#define MIN_ARGC_PARAMS_REQUIRED	5
#define MIN_LUT_PARAMS_REQUIRED		3
#define MIN_QSEED_PARAMS_REQUIRED	5
#define MIN_CALIB_PARAMS_REQUIRED	2
#define MAX_DAEMON_PARAMS_REQUIRED 5
#define MIN_GM_PARAMS_REQUIRED	5
#define MIN_PA_PARAMS_REQUIRED	6
#define MIN_PA_V2_PARAMS_REQUIRED 3
#define MIN_CSC_PARAMS_REQUIRED  3
#define MIN_SHARP_PARAMS_REQUIRED  6
#define MIN_AD_PARAMS_REQUIRED 4

#define DAEMON_SOCKET "pps"

enum {
	TOKEN_OP_TYPE_SET,
	TOKEN_OP_TYPE_SLEEP,
	TOKEN_OP_TYPE_GET,
	TOKEN_OP_TYPE_THREAD_JOIN,
	TOKEN_OP_TYPE_REPEAT,
	TOKEN_OP_TYPE_MAX,
};

enum {
	TOKEN_OP_CODE_HSIC,
	TOKEN_OP_CODE_PCC,
	TOKEN_OP_CODE_ARGC,
	TOKEN_OP_CODE_IGC,
	TOKEN_OP_CODE_HIST_LUT,
	TOKEN_OP_CODE_HIST_START,
	TOKEN_OP_CODE_HIST_STOP,
	TOKEN_OP_CODE_HIST_READ,
	TOKEN_OP_CODE_DAEMON_MSG,
	TOKEN_OP_CODE_QSEED,
	TOKEN_OP_CODE_CALIB,
	TOKEN_OP_CODE_GM,
	TOKEN_OP_CODE_PA,
	TOKEN_OP_CODE_CSC,
	TOKEN_OP_CODE_SHARP,
	TOKEN_OP_CODE_SRC_HIST,
	TOKEN_OP_CODE_AD,
	TOKEN_OP_CODE_PA_V2,
	TOKEN_OP_CODE_MAX,
};

struct thread_args {
	int arg[5];
};
#define MAX_PP_THREADS	10
pthread_t pp_threads[MAX_PP_THREADS];
struct thread_args pp_thread_args[MAX_PP_THREADS];

struct repeat_info {
	int num_repeattimes;
	int num_lines;
	int num_lines_read;
	int filepos;
	int repeat_initialised;
	FILE *fp;
};

int populate_default_pp_cfg(struct display_pp_conv_cfg *pp_cfg_ptr)
{
	if(!pp_cfg_ptr)
		return -1;

	pp_cfg_ptr->ops = 1;
	pp_cfg_ptr->hue = 0;
	pp_cfg_ptr->intensity = 0;
	pp_cfg_ptr->sat = 0;
	pp_cfg_ptr->contrast = 0;

	pp_cfg_ptr->cc_matrix[0][0] = 1;
	pp_cfg_ptr->cc_matrix[0][1] = 0;
	pp_cfg_ptr->cc_matrix[0][2] = 0;
	pp_cfg_ptr->cc_matrix[0][3] = 0;

	pp_cfg_ptr->cc_matrix[1][0] = 0;
	pp_cfg_ptr->cc_matrix[1][1] = 1;
	pp_cfg_ptr->cc_matrix[1][2] = 0;
	pp_cfg_ptr->cc_matrix[1][3] = 0;

	pp_cfg_ptr->cc_matrix[2][0] = 0;
	pp_cfg_ptr->cc_matrix[2][1] = 0;
	pp_cfg_ptr->cc_matrix[2][2] = 1;
	pp_cfg_ptr->cc_matrix[2][3] = 0;

	pp_cfg_ptr->cc_matrix[3][0] = 0;
	pp_cfg_ptr->cc_matrix[3][1] = 0;
	pp_cfg_ptr->cc_matrix[3][2] = 0;
	pp_cfg_ptr->cc_matrix[3][3] = 1;

	return 0;
}

int print_conv_cfg(struct display_pp_conv_cfg *params_ptr, struct display_hsic_cust_params *hsic_cfg){
	int ret = -1;
	int i,j;

	if(params_ptr){
		printf("========================\n");
		printf("Hue = %d\n",params_ptr->hue);
		printf("Saturation = %f\n",params_ptr->sat);
		printf("Intensity = %d\n",params_ptr->intensity);
		printf("Contrast = %f\n",params_ptr->contrast);
		printf("Ops = %u\n",params_ptr->ops);
		printf("========================\n");
		ret = 0;
	}

	printf("=======================\n");
	if(hsic_cfg){
		printf("hsic_cfg->intf = %d\n", hsic_cfg->intf);
		printf("rgb2yuv =\n");
		for(i=0;i<3;i++){
			for(j=0;j<3;j++)
				printf("\t%f", hsic_cfg->cust_rgb2yuv[i][j]);
			printf("\n");
		}
		printf("yuv2rgb =\n");
		for(i=0;i<3;i++){
			for(j=0;j<3;j++)
				printf("\t%f", hsic_cfg->cust_yuv2rgb[i][j]);
			printf("\n");
		}
	} else {
		printf("hsic_cfg == NULL\n");
	}
	printf("=======================\n");
	return ret;
}

int print_pcc_cfg(struct display_pp_pcc_cfg *pcc_cfg_ptr){
	int ret = -1;
	if(pcc_cfg_ptr){
		int count;
		printf("========================\n");
		printf("PCC Options = x%x\n",pcc_cfg_ptr->ops);
		for(count = 0;count<11;count++){
			printf(
			"R[%d]= %-.18f\t G[%d]= %-.18f\t B[%d]= %-.18f\n"
				, count,
				pcc_cfg_ptr->r.pcc_coeff[count],
				count,
				pcc_cfg_ptr->g.pcc_coeff[count],
				count,
				pcc_cfg_ptr->b.pcc_coeff[count]);
		}
		printf("========================\n");
		ret = 0;
	}
	return ret;
}

int print_argc_cfg(struct display_pp_argc_lut_data *argc_lut_data_ptr){
	int ret = -1;


	if( argc_lut_data_ptr ){
		int count;

		printf("========================\n");

		for(count = 0; count < NUM_ARGC_STAGES; count++) {
			printf("R[%d].enabled = %d\tR[%d].x_start = %u\t\
				R[%d].slope = %.18f\tR[%d].offset = %.18f\n",
				count,
				argc_lut_data_ptr->r[count].is_stage_enabled,
				count,
				argc_lut_data_ptr->r[count].x_start,
				count,
				argc_lut_data_ptr->r[count].slope,
				count,
				argc_lut_data_ptr->r[count].offset);
		}

		printf("=========\n");
		for(count = 0; count < NUM_ARGC_STAGES; count++) {
			printf("G[%d].enabled = %d\tG[%d].x_start = %u\t\
				G[%d].slope = %.18f\tG[%d].offset = %.18f\n",
				count,
				argc_lut_data_ptr->g[count].is_stage_enabled,
				count,
				argc_lut_data_ptr->g[count].x_start,
				count,
				argc_lut_data_ptr->g[count].slope,
				count,
				argc_lut_data_ptr->g[count].offset);
		}

		printf("=========\n");
		for(count = 0; count < NUM_ARGC_STAGES; count++) {
			printf("B[%d].enabled = %d\tB[%d].x_start = %u\t\
				B[%d].slope = %.18f\tB[%d].offset = %.18f\n",
				count,
				argc_lut_data_ptr->b[count].is_stage_enabled,
				count,
				argc_lut_data_ptr->b[count].x_start,
				count,
				argc_lut_data_ptr->b[count].slope,
				count,
				argc_lut_data_ptr->b[count].offset);
		}

		printf("========================\n");
		ret = 0;
	}

	return ret;
}

int print_igc_cfg(struct display_pp_igc_lut_data *cfg)
{
	int index;
	int ret = -1;
	uint32_t temp;
	if (cfg) {
		printf("=== IGC DATA =====================\n");
		printf("IGC Ops = %x\n",cfg->ops);
		for (index = 0; index < IGC_LENGTH; index++) {
			printf("index = %d,\tC0 = %d,\tC1 = %d,\tC2 = %d\n",
					index, cfg->c0[index], cfg->c1[index],
					cfg->c2[index]);
		}
		printf("=== END IGC DATA =================\n");
		ret = 0;
	}
	return ret;
}

int print_hist_lut_cfg(struct display_pp_hist_lut_data *cfg)
{
	int index;
	int ret = -1;
	uint32_t temp;
	if (cfg) {
		printf("=== HIST LUT DATA =====================\n");
		printf("HIST_LUT Ops = %x,\n",cfg->ops);
		for (index = 0; index < HIST_LUT_LENGTH; index++) {
			printf("index = %d,\tC0 = %d,\tC1 = %d,\tC2 = %d\n",
					index, cfg->c0[index], cfg->c1[index],
					cfg->c2[index]);
		}
		printf("=== END HIST LUT DATA =================\n");
		ret = 0;
	}
	return ret;
}

int print_histogram_start_req(struct mdp_histogram_start_req *req)
{
	int ret = -1;
	if (req) {
		printf("=== HISTOGRAM START REQ  =====================\n");
		printf("block = %d,\tframe_cnt = %d,\tbit_mask = 0x%08x,\tnum_bins = %d\n",
				req->block, req->frame_cnt, req->bit_mask, req->num_bins);
		printf("=== END HISTOGRAM START REQ  =================\n");
		ret = 0;
	}
	return ret;
}

int print_histogram_data(struct mdp_histogram_data *data, int extra_bins)
{
	unsigned int i;
	int ret = -1;
	if (data) {
		printf("=== HISTOGRAM DATA =====================\n");
		printf("block = %d,\tbin_cnt = %d,\n", data->block,
								data->bin_cnt);
		if (data->c0) {
			printf("C0 = ");
			for (i = 0; i < data->bin_cnt; i++) {
				if ((i % 4) == 0)
					printf("\n");
				printf("%d\t", data->c0[i]);
			}
			printf("\n");
		}
		if ((PP_BLOCK(data->block) >= MDP_BLOCK_DMA_P) &&
			(PP_BLOCK(data->block) < MDP_LOGICAL_BLOCK_DISP_0)) {
			if (data->c1) {
				printf("C1 = ");
				for (i = 0; i < data->bin_cnt; i++) {
					if ((i % 4) == 0)
						printf("\n");
					printf("%d\t", data->c1[i]);
				}
				printf("\n");
			}
			if (data->c2) {
				printf("C2 = ");
				for (i = 0; i < data->bin_cnt; i++) {
					if ((i % 4) == 0)
						printf("\n");
					printf("%d\t", data->c2[i]);
				}
				printf("\n");
			}
		}
		if (data->extra_info) {
			printf("ExtraInfo = ");
			for (i = 0; i < extra_bins; i++) {
				printf("%d\t", data->extra_info[i]);
			}
			printf("\n");
		}
		printf("=== END HISTOGRAM DATA =================\n");
		ret = 0;
	}
	return ret;
}

int postproc_framework_repeat_handler(struct repeat_info *rinfo, int op_type,
					char *token1, char *token2){
	int ret = TEST_RESULT_PASS;

	if (op_type == TOKEN_OP_TYPE_REPEAT) {

		if (rinfo->repeat_initialised == FALSE) {
			rinfo->num_repeattimes = atoi(token1);
			      rinfo->num_lines = atoi(token2);
			 rinfo->num_lines_read = 0;
					rinfo->filepos = ftell(rinfo->fp);

		    if ((rinfo->num_repeattimes > 0) && (rinfo->num_lines > 0))
				rinfo->repeat_initialised = TRUE;
		}
	} else {

		rinfo->num_lines_read++;

		if (rinfo->num_lines_read == rinfo->num_lines) {
			rinfo->num_lines_read = 0;
			rinfo->num_repeattimes--;
			if (rinfo->num_repeattimes > 0) {
				fseek(rinfo->fp, rinfo->filepos, SEEK_SET);
			} else {
				rinfo->repeat_initialised = FALSE;
				rinfo->num_lines_read = 0;
			}
		}
	}
	return ret;
}

int tokenize_params(char *inputParams, const char *delim, const int minTokenReq,
					char* tokenStr[], int *idx){
	char *tmp_token = NULL;
	int ret = 0, index = 0;
	if (!inputParams) {
		ret = -1;
		goto err;
	}
	tmp_token = strtok(inputParams, delim);
	while (tmp_token != NULL) {
		tokenStr[index++] = tmp_token;
		if (index < minTokenReq) {
			tmp_token = strtok(NULL, delim);
		}else{
			break;
		}
	}
	*idx = index;
err:
	return ret;
}

int postproc_framework_qseed_handler( int access, char* params){
	char* tokens[MIN_QSEED_PARAMS_REQUIRED];
	char* output[100];
	char *temp_token = NULL;
	int ret = TEST_RESULT_SKIP;
	uint32_t index = 0;
	uint32_t table_max;
	struct msmfb_mdp_pp cfg;
	uint32_t * values;
	cfg.op = mdp_op_qseed_cfg;
	char *line = NULL;
	FILE *fp = NULL;

	line = (char *)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		goto err_line;
	}

	memset(tokens, 0, sizeof(tokens));
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_QSEED_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	if (index != MIN_QSEED_PARAMS_REQUIRED){
		printf("invalid number of params reqiuired = %d != given = %d",
					MIN_QSEED_PARAMS_REQUIRED, index);
		goto err;
	}

	cfg.data.qseed_cfg_data.block = atoi(tokens[0]);
	cfg.data.qseed_cfg_data.qseed_data.table_num = atoi(tokens[1]);
	cfg.data.qseed_cfg_data.qseed_data.ops = atoi(tokens[2]);
	cfg.data.qseed_cfg_data.qseed_data.len = atoi(tokens[3]);
	temp_token = strstr(tokens[4],"\r\n");
	if(temp_token)
		*temp_token = '\0';

	table_max = (cfg.data.qseed_cfg_data.qseed_data.table_num == 2) ?
					QSEED_TABLE_2_MAX : QSEED_TABLE_1_MAX;

	if (table_max != cfg.data.qseed_cfg_data.qseed_data.len) {
		printf("invalid length of table entry %d != %d",
		cfg.data.qseed_cfg_data.qseed_data.len, table_max);
		goto err;
	}

	values = malloc(cfg.data.qseed_cfg_data.qseed_data.len * sizeof(uint32_t));
	if (!values) {
		printf("Cant allocate memory\n");
		goto err;
	}
	cfg.data.qseed_cfg_data.qseed_data.data = values;

	if (access == PP_OPS_WRITE) {
		fp = fopen (tokens[4], "r");
		if (!fp) {
			printf("Cant open file %s\n", tokens[4]);
			goto err_mem;
		}

		index = 0;
		while(fgets(line, MAX_CFG_LINE_LEN * sizeof(char), fp) &&
				index < table_max) {
			if (line[0] == '\r' || line[0] == '\n')
				continue;

			if ((temp_token = strtok(line, TOKEN_DELIM_STR)) != NULL) {
				do {
					values[index++] = atoi(temp_token);
					temp_token = strtok(NULL, TOKEN_DELIM_STR);
				} while (temp_token != NULL && index < table_max);
			}

		}
		fclose(fp);

		printf("Qseed block = %d\n", cfg.data.qseed_cfg_data.block);
		printf("\t table_num = %d\n", cfg.data.qseed_cfg_data.qseed_data.table_num);
		printf("\t ops = %d\n", cfg.data.qseed_cfg_data.qseed_data.ops);
		printf("\t len = %d\n", cfg.data.qseed_cfg_data.qseed_data.len);

		printf("\nCalling ioctl for qseed!!\n");
		if (ioctl(FB->fb_fd, MSMFB_MDP_PP, &cfg))
			ret = TEST_RESULT_FAIL;
		else
			ret = TEST_RESULT_PASS;
	} else if (access == PP_OPS_READ) {
		fp = fopen (tokens[4], "w+");
		if (!fp) {
			printf("Cant open file %s\n", tokens[4]);
			goto err_mem;
		}

		printf("Qseed block = %d\n", cfg.data.qseed_cfg_data.block);
		printf("\t table_num = %d\n", cfg.data.qseed_cfg_data.qseed_data.table_num);
		printf("\t ops = %d\n", cfg.data.qseed_cfg_data.qseed_data.ops);
		printf("\t len = %d\n", cfg.data.qseed_cfg_data.qseed_data.len);

		printf("\nCalling ioctl for qseed!!\n");
		if (ioctl(FB->fb_fd, MSMFB_MDP_PP, &cfg)) {
			fclose(fp);
			ret = TEST_RESULT_FAIL;
			goto err_mem;
		}

		index = 0;
		while (index < cfg.data.qseed_cfg_data.qseed_data.len) {
			fprintf(fp, "%d\t", values[index]);
			index++;
			if (!(index % 4)) {
				fprintf(fp, "\n");
			}
		}
		fclose(fp);
	}
err_mem:
	free(values);
err:
	free(line);
err_line:
	return ret;
}

int postproc_framework_calib_handler( int access, char* params){
	char* tokens[MIN_CALIB_PARAMS_REQUIRED];
	char* output[100];
	char *temp_token = NULL;
	int ret = TEST_RESULT_SKIP, index = 0;
	uint32_t table_max;
	struct msmfb_mdp_pp cfg;
	uint32_t * values;
	cfg.op = mdp_op_calib_cfg;
	char line[MAX_CFG_LINE_LEN];
	FILE *fp = NULL;

	temp_token = strtok (params,TOKEN_PARAMS_DELIM);
	while (temp_token != NULL && index < MIN_CALIB_PARAMS_REQUIRED) {
		tokens[index++] = temp_token;
		temp_token = strtok (NULL,TOKEN_PARAMS_DELIM);
	}   //end while

	if (index != MIN_CALIB_PARAMS_REQUIRED){
		printf("invalid number of params reqiuired = %d != given = %d",
					MIN_CALIB_PARAMS_REQUIRED, index);
		goto err;
	}

	cfg.data.calib_cfg.ops = (access) ? MDP_PP_OPS_READ : MDP_PP_OPS_WRITE;
	cfg.data.calib_cfg.addr = atoi(tokens[0]);
	cfg.data.calib_cfg.data = atoi(tokens[1]);

	printf("\nCalling ioctl for calib!!\n");
	if (ioctl(FB->fb_fd, MSMFB_MDP_PP, &cfg))
		ret = TEST_RESULT_FAIL;
	else
		ret = TEST_RESULT_PASS;

	if (access == PP_OPS_READ) {
		printf("data = 0x%08x", cfg.data.calib_cfg.data);
	}
err:
	return ret;
}

void *postproc_framework_hist_reader(void* arg){
	int num = *((int*)arg);
	int ret = TEST_RESULT_SKIP;
	struct mdp_histogram_data hist;
	uint32_t extra, loops, index;

	hist.block = pp_thread_args[num].arg[0];
	hist.bin_cnt = pp_thread_args[num].arg[1];
	loops = pp_thread_args[num].arg[2];

	extra = (PP_BLOCK(hist.block) >= MDP_BLOCK_DMA_P) ? 2 : 1;

	hist.c0 = malloc(hist.bin_cnt * sizeof(uint32_t));
	if(!hist.c0)
		goto err;
	if (PP_BLOCK(hist.block) < MDP_LOGICAL_BLOCK_DISP_0) {
		hist.c1 = malloc(hist.bin_cnt * sizeof(uint32_t));
		if(!hist.c1)
			goto err_c1;
		hist.c2 = malloc(hist.bin_cnt * sizeof(uint32_t));
		if(!hist.c2)
			goto err_c2;
	} else {
		hist.c1 = NULL;
		hist.c2 = NULL;
	}

	hist.extra_info = malloc(extra * sizeof(uint32_t));
	if(!hist.extra_info)
		goto err_ex;

	for (index = 0; index < loops; index++) {
		printf("Hist Loop #%d\nCalling ioctl for do_histogram!! - %d\n",
								index, hist.block);
		if (ioctl(FB->fb_fd, MSMFB_HISTOGRAM, &hist)) {
			ret = TEST_RESULT_FAIL;
			printf("Histogram FAILED! - errno = %d\n", errno);
		} else {
			ret = TEST_RESULT_PASS;
			print_histogram_data(&hist, extra);
		}
	}

	free(hist.extra_info);
err_ex:
	if (PP_BLOCK(hist.block) < MDP_LOGICAL_BLOCK_DISP_0)
		goto err_c1;
	free(hist.c2);
err_c2:
	free(hist.c1);
err_c1:
	free(hist.c0);
err:
	printf("Return value = %d, block = %d\n", ret, hist.block);
	return ret;
}

int postproc_framework_hist_read_handler(int *thread_index, char* params)
{
	char *temp_token = NULL;
	int index = 0;
	pthread_t pth;

	if (*thread_index >= MAX_PP_THREADS) {
		printf("Max num threads achieved... not adding histogram read.\n");
		return -1;
	}

	temp_token = strtok (params,TOKEN_PARAMS_DELIM);
	while (temp_token != NULL && index < MIN_HIST_READ_PARAMS_REQUIRED) {
		printf("temp_token = %s\n", temp_token);
		pp_thread_args[*thread_index].arg[index++] = atoi(temp_token);
		temp_token = strtok (NULL,TOKEN_PARAMS_DELIM);
	}   //end while

	if (index != MIN_HIST_READ_PARAMS_REQUIRED)
		goto err;


	printf("creating thread @ %d\n", *thread_index);
	pthread_create(&pth, NULL, postproc_framework_hist_reader, *thread_index);
	pp_threads[*thread_index] = pth;
	(*thread_index)++;

	return 0;
err:
	return TEST_RESULT_FAIL;
}

int postproc_framework_thread_join_handler(int *index, char* params){
	int i;
	int threads = (*index) - 1;
	int ret_temp, ret = TEST_RESULT_PASS;

	for (i = threads; i >= 0; i--) {
		pthread_join(pp_threads[i], (void **) &ret_temp);
		printf("retval = %d\n", ret_temp);
		pp_threads[i] = NULL;
		*index -= 1;
		if (ret_temp == TEST_RESULT_FAIL)
			ret = TEST_RESULT_FAIL;
	}
	return ret;
}

int postproc_framework_hist_stop_handler(char* params){
	char* tokens[MIN_HIST_STOP_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = TEST_RESULT_SKIP, index=0;
	uint32_t block;

	memset(tokens, 0, sizeof(tokens));
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_HIST_STOP_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	if (index != MIN_HIST_STOP_PARAMS_REQUIRED)
		goto err;

	block = atoi(tokens[0]);

	printf("Histogram stop block = %d\n", block);

	printf("\nCalling ioctl for histogram_stop!!\n");
	if (ioctl(FB->fb_fd, MSMFB_HISTOGRAM_STOP, &block)) {
		ret = TEST_RESULT_FAIL;
		printf("Histogram stop FAILED! - block = %d\n", block);
	} else
		ret = TEST_RESULT_PASS;
err:
	return ret;
}

int postproc_framework_hist_start_handler(char* params){
	char* tokens[MIN_HIST_START_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1, index=0;
	struct mdp_histogram_start_req req;

	memset(tokens, 0, sizeof(tokens));
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_HIST_START_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	if (index != MIN_HIST_START_PARAMS_REQUIRED)
		goto err;

	req.block = atoi(tokens[0]);
	req.frame_cnt = atoi(tokens[1]);
	req.bit_mask = atoi(tokens[2]);
	req.num_bins = atoi(tokens[3]);

	print_histogram_start_req(&req);

	printf("\nCalling ioctl for histogram_start!! %d\n", req.block);
	if (ioctl(FB->fb_fd, MSMFB_HISTOGRAM_START, &req)) {
		ret = TEST_RESULT_FAIL;
		printf("Histogram start FAILED! - block = %d\n", req.block);
	} else
		ret = TEST_RESULT_PASS;
	printf("\nioctl complete!! %d\n", req.block);

err:
	return ret;
}

int overlay_used = 0;

void *postproc_framework_overlay_hist_controller(void* arg){
	char *temp_token = NULL;
	int ret = -1, index=0, sleep, usr_def_pipe = 0;
	struct mdp_histogram_cfg *cfg;
	struct mdp_overlay_pp_params pp_params;
	int num = *((int*)arg);
	time_t start, curr;
	double secs;

	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	struct fbtest_params *thisFBTEST = &FBTEST;
	memset(&overlay, 0, sizeof(overlay));
	memset(&ovdata, 0, sizeof(ovdata));

	cfg = &pp_params.hist_cfg;
	cfg->block = pp_thread_args[num].arg[0];
	cfg->frame_cnt = pp_thread_args[num].arg[1];
	cfg->bit_mask = pp_thread_args[num].arg[2];
	cfg->num_bins = pp_thread_args[num].arg[3];
	sleep = pp_thread_args[num].arg[4];

	if ((cfg->block & MDSS_PP_ARG_MASK) == 0) {
		usr_def_pipe = 1;
		printf("block & argmask 0x0 == 0x%08x\n", cfg->block & MDSS_PP_ARG_MASK);
		ret = initOverlay(thisFBTEST, &overlay);
		if (ret) {
			printf("Overlay initialization failed! line=%d\n",
				__LINE__);
			goto exit_with_errors;
		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",
				__LINE__);
			ret = -OVERLAY_SET_FAILED;
			goto exit_with_errors;
		}
	} else {
		printf("block & argmask = 0x%08x\n", cfg->block & MDSS_PP_ARG_MASK);
		overlay.id = (cfg->block & MDSS_PP_ARG_MASK) >>
					MDSS_PP_ARG_SHIFT;
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_GET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_GET failed! line=%d\n", __LINE__);
			ret = -OVERLAY_SET_FAILED;
			goto exit_with_errors;
		}
	}
	VPRINT(verbose, "\nPerforming SSPP Hist configure test...\n");

	cfg->block |= overlay.id << MDSS_PP_ARG_SHIFT;
	cfg->ops = MDP_PP_OPS_ENABLE;

	printf("\n OVERLAY ID = %d\n", overlay.id);
	printf("\n OVERLAY PP BLOCK = %d\n", pp_params.hist_cfg.block);

	overlay.overlay_pp_cfg = pp_params;
	overlay.flags |= MDP_OVERLAY_PP_CFG_EN;
	overlay.overlay_pp_cfg.config_ops |= MDP_OVERLAY_PP_HIST_CFG;
	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
		printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		ret = -OVERLAY_SET_FAILED;
		goto exit_with_errors;
	}

	if (usr_def_pipe) {
		curr_overlay_id[overlay_used++] = overlay.id;   //UTF: added for cleanup code addition.
		ovdata.id = overlay.id;
	}

	ret = initOvdata(thisFBTEST, &ovdata);
	if (ret) {
		printf("Ovdata initialization failed! line=%d\n",  __LINE__);
		goto exit_with_errors;
	}

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		printf("MSMFB_OVERLAY_PLAY failed! line=%d\n", __LINE__);
		ret = -OVERLAY_PLAY_FAILED;
		goto exit_with_errors;
	}

	VPRINT(verbose, "Displaying Image on device for %d secs\n", sleep);
	time(&start);
	do {
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
				__LINE__);
			ret = TEST_RESULT_FAIL;
			goto exit_with_errors;
		}
		time(&curr);
		secs = difftime(curr, start);
	} while (((int)secs) < sleep);

	cfg->ops = MDP_PP_OPS_DISABLE;
	overlay.overlay_pp_cfg = pp_params;
	overlay.flags |= MDP_OVERLAY_PP_CFG_EN;
	overlay.overlay_pp_cfg.config_ops |= MDP_OVERLAY_PP_HIST_CFG;
	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
		printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		ret = -OVERLAY_SET_FAILED;
		goto exit_with_errors;
	}

	printf("testing writeback init ioctl %d\n", ioctl(FB->fb_fd, MSMFB_WRITEBACK_INIT, NULL));
	ret = TEST_RESULT_PASS;
exit_with_errors:
	VPRINT(verbose, "\n Test: %s\n", (ret == TEST_RESULT_PASS ? "PASS" : "FAIL"));

err:
	return ret;
}

int postproc_framework_overlay_hist_handler(int *thread_index, char* params)
{
	char *temp_token = NULL;
	int index = 0;
	pthread_t pth;

	if (*thread_index >= MAX_PP_THREADS) {
		printf("Max num threads achieved... not adding ovHist.\n");
		return -1;
	}

	temp_token = strtok (params,TOKEN_PARAMS_DELIM);
	while (temp_token != NULL && index < MIN_HIST_OVERLAY_CFG_PARAMS_REQ) {
		printf("temp_token = %s\n", temp_token);
		pp_thread_args[*thread_index].arg[index++] = atoi(temp_token);
		temp_token = strtok (NULL,TOKEN_PARAMS_DELIM);
	}   //end while

	if (index != MIN_HIST_OVERLAY_CFG_PARAMS_REQ)
		goto err;


	printf("creating thread @ %d\n", *thread_index);
	pthread_create(&pth, NULL, postproc_framework_overlay_hist_controller, *thread_index);
	pp_threads[*thread_index] = pth;
	(*thread_index)++;

	return TEST_RESULT_PASS;
err:
	return TEST_RESULT_FAIL;
}

int postproc_framework_hist_lut_handler(char *params)
{
	char *tokens[MIN_LUT_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1;
	struct display_pp_hist_lut_data lut_cfg;
	int i, index, ops, mdp_block = 0;
	uint32_t temp_val[HIST_LUT_COLORS];
	char *line = NULL;
	FILE *fp = NULL;

	struct msmfb_mdp_pp pp;
	struct fbtest_params *thisFBTEST = &FBTEST;

	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	struct mdp_hist_lut_data *ov_lut_cfg;
	struct mdp_overlay_pp_params overlay_pp_params;
	memset(&overlay, 0, sizeof(overlay));
	memset(&ovdata, 0, sizeof(ovdata));

	index = 0;
	memset(tokens, 0, sizeof(tokens));
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_LUT_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	if (index < MIN_LUT_PARAMS_REQUIRED) {
		printf("Not Enough tokens %d\n", index);
		ret = TEST_RESULT_SKIP;
		goto err;
	}

	mdp_block = atoi(tokens[0]);
	lut_cfg.ops = atoi(tokens[1]);

	temp_token = strstr(tokens[2],"\r\n");
	if(temp_token)
		*temp_token = '\0';

	fp = fopen (tokens[2], "r");
	if (!fp) {
		printf("Cant open file %s\n", tokens[2]);
		ret = TEST_RESULT_SKIP;
		goto err;
	}

	line = (char *)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		goto err;
	}

	index = 0;
	while(fgets(line, MAX_CFG_LINE_LEN * sizeof(char), fp) &&
			index < HIST_LUT_LENGTH) {
		if (line[0] == '\r' || line[0] == '\n')
			continue;

		i = 0;
		if ((temp_token = strtok(line, TOKEN_DELIM_STR)) != NULL) {
			do {
				temp_val[i++] = atoi(temp_token);
				temp_token = strtok(NULL, TOKEN_DELIM_STR);
			} while (temp_token != NULL && i < HIST_LUT_COLORS);
		}
		if (i != HIST_LUT_COLORS || 2 >= HIST_LUT_COLORS)
			break;

		lut_cfg.c0[index] = (uint8_t) temp_val[0];
		lut_cfg.c1[index] = (uint8_t) temp_val[1];
		lut_cfg.c2[index] = (uint8_t) temp_val[2];
		index++;
	}
	fclose(fp);

	if (index < HIST_LUT_LENGTH) {
		printf("Not enough parameters passed to test (%d != %d)\n",index,
				HIST_LUT_LENGTH);
		ret = TEST_RESULT_SKIP;
		goto err_mem;
	}

	print_hist_lut_cfg(&lut_cfg);

	if (PP_LOCAT(mdp_block) == MDSS_PP_DSPP_CFG) {
		printf("\nCalling user space library for HIST LUT!!\n");
		ret = display_pp_hist_set_lut(mdp_block, &lut_cfg);
	} else if (PP_LOCAT(mdp_block) == MDSS_PP_SSPP_CFG) {
		ret = initOverlay(thisFBTEST, &overlay);
		if (ret) {
			printf("Overlay initialization failed! line=%d\n",  __LINE__);
			goto err_mem;
		}

		VPRINT(verbose, "\nPerforming SSPP Hist LUT Test...\n");

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
			ret = -OVERLAY_SET_FAILED;
			goto err_mem;
		}
		curr_overlay_id[overlay_used++] = overlay.id;   //UTF: added for cleanup code addition.
		ovdata.id = overlay.id;

		overlay_pp_params.config_ops = MDP_OVERLAY_PP_HIST_LUT_CFG;
		ov_lut_cfg = &overlay_pp_params.hist_lut_cfg;
		ov_lut_cfg->block = mdp_block | overlay.id << MDSS_PP_ARG_SHIFT;
		ov_lut_cfg->ops = lut_cfg.ops;
		ov_lut_cfg->len = HIST_LUT_LENGTH;

		ov_lut_cfg->data = calloc(HIST_LUT_LENGTH, sizeof(uint32_t));
		if (!ov_lut_cfg->data) {
			printf("cannot alloc histLUT mem");
			ret = TEST_RESULT_FAIL;
			goto err_mem;
		}

		for (i = 0; i < HIST_LUT_LENGTH; i++){
			ov_lut_cfg->data[i] = lut_cfg.c0[i];
		}

		overlay.flags |= MDP_OVERLAY_PP_CFG_EN;
		overlay.overlay_pp_cfg = overlay_pp_params;
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		}

		ret = initOvdata(thisFBTEST, &ovdata);
		if (ret) {
			printf("Ovdata initialization failed! line=%d\n",  __LINE__);
			goto exit_with_errors;
		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			printf("MSMFB_OVERLAY_PLAY failed! line=%d\n", __LINE__);
			ret = -OVERLAY_PLAY_FAILED;
			goto exit_with_errors;
		}
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
			ret = TEST_RESULT_FAIL;
			goto exit_with_errors;
		}
		sleep(2);
		printf("\n trying second set\n");
		printf("\n overlay.flags = %08x, %08x", overlay.flags,
		overlay.overlay_pp_cfg.config_ops);
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		}
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			printf("MSMFB_OVERLAY_PLAY failed! line=%d\n", __LINE__);
			ret = -OVERLAY_PLAY_FAILED;
			goto exit_with_errors;
		}
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
			ret = TEST_RESULT_FAIL;
			goto exit_with_errors;
		}
		sleep(2);
		printf("\n trying third set\n");
		printf("\n overlay.flags = %08x, %08x", overlay.flags,
		overlay.overlay_pp_cfg.config_ops);
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		}
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			printf("MSMFB_OVERLAY_PLAY failed! line=%d\n", __LINE__);
			ret = -OVERLAY_PLAY_FAILED;
			goto exit_with_errors;
		}
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
			ret = TEST_RESULT_FAIL;
			goto exit_with_errors;
		}

		VPRINT(verbose, "Displaying Image on device...\n");

		if (verbose) {
			printf("press any key to continue...\n");
			getchar();
		} else {
			sleep(5);
		}

		printf("testing writeback init ioctl %d\n", ioctl(FB->fb_fd, MSMFB_WRITEBACK_INIT, NULL));

exit_with_errors:
		VPRINT(verbose, "\n Test: %s\n", (ret == TEST_RESULT_PASS ? "PASS" : "FAIL"));
		free(overlay_pp_params.hist_lut_cfg.data);
	}
err_mem:
	free(line);
err:
	return ret;
}

int postproc_framework_igc_handler(char *params)
{
	char *tokens[MIN_LUT_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1;
	struct display_pp_igc_lut_data igc_cfg;
	struct compute_params op_params;
	struct mdp_overlay_pp_params overlay_pp_params;
	int i, index, ops, mdp_block = 0;
	uint32_t temp_val[IGC_COLORS];
	char *line = NULL;
	FILE *fp = NULL;

	struct msmfb_mdp_pp pp;
	struct fbtest_params *thisFBTEST = &FBTEST;

	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	memset(&overlay, 0, sizeof(overlay));
	memset(&ovdata, 0, sizeof(ovdata));

	index = 0;
	memset(tokens, 0, sizeof(tokens));
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_LUT_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	if (index < MIN_LUT_PARAMS_REQUIRED){
		ret = TEST_RESULT_SKIP;
		goto err;
	}

	mdp_block = atoi(tokens[0]);
	igc_cfg.ops = atoi(tokens[1]);

	temp_token = strstr(tokens[2],"\r\n");
	if(temp_token)
		*temp_token = '\0';

	fp = fopen (tokens[2], "r");
	if (!fp) {
		printf("Cant open file %s\n", tokens[2]);
		ret = TEST_RESULT_SKIP;
		goto err;
	}

	line = (char *)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		fclose(fp);
		goto err;
	}

	index = 0;
	while(fgets(line, MAX_CFG_LINE_LEN * sizeof(char), fp) &&
			index < IGC_LENGTH){
		if (line[0] == '\r' || line[0] == '\n')
			continue;

		i = 0;
		if ((temp_token = strtok(line, TOKEN_DELIM_STR)) != NULL) {
			do {
				temp_val[i++] = atoi(temp_token);
				temp_token = strtok(NULL, TOKEN_DELIM_STR);
			} while (temp_token != NULL && i < IGC_COLORS);
		}
		if (i != IGC_COLORS || 2 >= IGC_COLORS)
			break;

		igc_cfg.c0[index] = temp_val[0];
		igc_cfg.c1[index] = temp_val[1];
		igc_cfg.c2[index] = temp_val[2];
		index++;
	}
	fclose(fp);

	if (index < IGC_LENGTH) {
		printf("Not enough parameters passed to test (%d != %d)\n",index,
				IGC_LENGTH);
		ret = TEST_RESULT_SKIP;
		goto err_mem;
	}

	print_igc_cfg(&igc_cfg);

	switch (PP_LOCAT(mdp_block)) {
	case MDSS_PP_DSPP_CFG:
		printf("\nCalling user space library for IGC!!\n");
		ret = display_pp_igc_set_lut(mdp_block,&igc_cfg);
		break;
	case MDSS_PP_SSPP_CFG:
		ret = initOverlay(thisFBTEST, &overlay);
		if (ret) {
			printf("Overlay initialization failed! line=%d\n",  __LINE__);
			goto exit_with_errors;
		}

		VPRINT(verbose, "\nPerforming SSPP IGC Test...\n");

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
			ret = -OVERLAY_SET_FAILED;
			goto exit_with_errors;
		}
		curr_overlay_id[overlay_used++] = overlay.id;   //UTF: added for cleanup code addition.
		ovdata.id = overlay.id;

		overlay_pp_params.igc_cfg.c0_c1_data = calloc(IGC_LENGTH,
							sizeof(uint32_t));

		if (!overlay_pp_params.igc_cfg.c0_c1_data) {
			printf("cannot alloc igc mem");
			ret = TEST_RESULT_FAIL;
			goto err_mem;
		}
		overlay_pp_params.igc_cfg.c2_data = calloc(IGC_LENGTH,
							sizeof(uint32_t));
		if (!overlay_pp_params.igc_cfg.c2_data) {
			printf("cannot alloc igc mem");
			ret = TEST_RESULT_FAIL;
			goto err_igc_mem;
		}
		op_params.operation |= PP_OP_IGC;
		op_params.params.igc_lut_params = igc_cfg;
		ret = display_pp_compute_params(&op_params,
							&overlay_pp_params);
		overlay.flags |= MDP_OVERLAY_PP_CFG_EN;

		overlay.overlay_pp_cfg = overlay_pp_params;
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		}

		ret = initOvdata(thisFBTEST, &ovdata);
		if (ret) {
			printf("Ovdata initialization failed! line=%d\n",  __LINE__);
			goto exit_with_errors;
		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			printf("MSMFB_OVERLAY_PLAY failed! line=%d\n", __LINE__);
			ret = -OVERLAY_PLAY_FAILED;
			goto exit_with_errors;
		}

		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
			ret = TEST_RESULT_FAIL;
			goto exit_with_errors;
		}

		VPRINT(verbose, "Displaying Image on device...\n");

		if (verbose) {
			printf("press any key to continue...\n");
			getchar();
		} else {
			sleep(5);
		}

		printf("testing writeback init ioctl %d\n", ioctl(FB->fb_fd, MSMFB_WRITEBACK_INIT, NULL));

exit_with_errors:
		VPRINT(verbose, "\n Test: %s\n", (ret == TEST_RESULT_PASS ? "PASS" : "FAIL"));
		free(overlay_pp_params.igc_cfg.c2_data);
err_igc_mem:
		free(overlay_pp_params.igc_cfg.c0_c1_data);
		break;
	default:
		goto err;
		break;
	}

err_mem:
	free(line);
err:
	return ret;
}

#define MAX_DAEMON_CMD_LEN 4096
int postproc_framework_daemon_test(char *params) {
	int daemon_socket = -1, ret = 0, index = 0, nargs = 0;
	char cmd[MAX_DAEMON_CMD_LEN];
	char reply[32];
	char *token[MAX_DAEMON_PARAMS_REQUIRED];
	token[index++] = strtok(params, TOKEN_PARAMS_DELIM);
	if (!strncmp(token[0], "pp:set:hsic", strlen("pp:set:hsic")))
		nargs = 4;
	else if (!strncmp(token[0], "cabl:set", strlen("cabl:set")) ||
        !strncmp(token[0], "pp:calib:load:profile", strlen("pp:calib:load:profile")))
		nargs = 1;
	if (!strncmp(token[0], "ad:init", strlen("ad:init")) ||
		!strncmp(token[0], "ad:config", strlen("ad:config")) ||
		!strncmp(token[0], "ad:input", strlen("ad:input")) ||
		!strncmp(token[0], "ad:on", strlen("ad:on")) ||
		!strncmp(token[0], "bl:set", strlen("bl:set")) ||
		!strncmp(token[0], "set", strlen("set")) ||
		!strncmp(token[0], "ad:assertiveness", strlen("ad:assertiveness"))) {
		token[index++] = strtok(NULL, ":");
	} else {
		while (nargs--)
			token[index++] = strtok(NULL, TOKEN_PARAMS_DELIM);
	}

	printf("Testing for command: %s\n", params);
	daemon_socket = socket_local_client(DAEMON_SOCKET,
			ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
	if(!daemon_socket) {
		printf("Connecting to socket failed: %s\n", strerror(errno));
		return -1;
	}

	/* make a daemon parseable command */
	if (!strncmp(params, "pp:set:hsic", strlen("pp:set:hsic")))
		sprintf(cmd, "%s %d %f %d %f\n", token[0], atoi(token[1]),
				atof(token[2]), atoi(token[3]), atof(token[4]));
	else if (!strncmp(token[0], "cabl:set", strlen("cabl:set")) ||
        !strncmp(token[0], "pp:calib:load:profile", strlen("pp:calib:load:profile"))){
		char *ptemp = NULL;
		ptemp = strstr(token[1], "\r\n");
		if (ptemp)
		     *ptemp = '\0';
		snprintf(cmd, MAX_DAEMON_CMD_LEN, "%s %s", token[0], token[1]);
        }
	else if (!strncmp(token[0], "ad:init", strlen("ad:init")) ||
		!strncmp(token[0], "ad:config", strlen("ad:config")) ||
		!strncmp(token[0], "ad:input", strlen("ad:input")) ||
		!strncmp(token[0], "ad:on", strlen("ad:on")) ||
		!strncmp(token[0], "bl:set", strlen("bl:set")) ||
		!strncmp(token[0], "set", strlen("set")) ||
		!strncmp(token[0], "ad:assertiveness", strlen("ad:assertiveness"))) {
		char *ptemp = NULL;
		ptemp = strstr(token[1], "\r\n");
		if (ptemp)
		     *ptemp = '\0';
		snprintf(cmd, MAX_DAEMON_CMD_LEN, "%s;%s", token[0], token[1]);
        }
	else
		sprintf(cmd, "%s", token[0]);

	printf("writing %s to the socket\n", cmd);
	ret = write(daemon_socket, cmd, strlen(cmd));
	if(ret < 0) {
		printf("%s: Failed to send data over socket %s\n",
				strerror(errno), DAEMON_SOCKET);
		close(daemon_socket);
		return -1;
	}

	/* listen for daemon responses */
	ret = read(daemon_socket, reply, sizeof(reply));
	if(ret < 0) {
		printf("%s: Failed to get data over socket %s\n", strerror(errno),
				DAEMON_SOCKET);
		close(daemon_socket);
		return -1;
	}
	printf("Daemon response: %s\n", reply);
	close(daemon_socket);
	return 0;
}

int pp_gm_parse_calib_data(struct gamut_tbl_c *gamut_c,
		const char* file_path){
	int ret = -1, tbl_num = 0;
	FILE *fp = NULL;
	char *line = NULL;
	char* tokens[GM_CALIB_DATA_MAX_FIELDS_T0];
	char *temp_token = NULL;
	int index = 0;

	if(!gamut_c || !file_path)
		return ret;

	fp = fopen (file_path, "r");
	if(!fp ){
		printf("Cant open file %s\n", file_path);
		ret = TEST_RESULT_SKIP;
		return ret;
	}

	line = (char*)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		goto err;
	}

	while(fgets(line, MAX_CFG_LINE_LEN*sizeof(char),fp)){
		if (line[0] == '\r' || line[0] == '\n')
			continue;

		memset(tokens, 0, sizeof(tokens));
		index = 0;

		switch(tbl_num) {
		case 0:
			{
				ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, GM_CALIB_DATA_MAX_FIELDS_T0, tokens, &index);
				if(ret){
					printf("tokenize_params failed! (Line %d)\n",__LINE__);
					goto err_mem;
				}

				for (index = 0; index < GM_CALIB_DATA_MAX_FIELDS_T0; ++index) {
					gamut_c->c0[index] = atoi(tokens[index]);
				}
			}
			break;
		case 1:
			{
				ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, GM_CALIB_DATA_MAX_FIELDS_T1, tokens, &index);
				if(ret){
					printf("tokenize_params failed! (Line %d)\n",__LINE__);
					goto err_mem;
				}

				for (index = 0; index < GM_CALIB_DATA_MAX_FIELDS_T1; ++index) {
					gamut_c->c1[index] = atoi(tokens[index]);
				}
			}
			break;
		case 2:
			{
				ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, GM_CALIB_DATA_MAX_FIELDS_T2, tokens, &index);
				if(ret){
					printf("tokenize_params failed! (Line %d)\n",__LINE__);
					goto err_mem;
				}

				for (index = 0; index < GM_CALIB_DATA_MAX_FIELDS_T2; ++index) {
					gamut_c->c2[index] = atoi(tokens[index]);
				}
			}
			break;
		case 3:
			{
				ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, GM_CALIB_DATA_MAX_FIELDS_T3, tokens, &index);
				if(ret){
					printf("tokenize_params failed! (Line %d)\n",__LINE__);
					goto err_mem;
				}

				for (index = 0; index < GM_CALIB_DATA_MAX_FIELDS_T3; ++index) {
					gamut_c->c3[index] = atoi(tokens[index]);
				}
			}
			break;
		case 4:
			{
				ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, GM_CALIB_DATA_MAX_FIELDS_T4, tokens, &index);
				if(ret){
					printf("tokenize_params failed! (Line %d)\n",__LINE__);
					goto err_mem;
				}

				for (index = 0; index < GM_CALIB_DATA_MAX_FIELDS_T4; ++index) {
					gamut_c->c4[index] = atoi(tokens[index]);
				}
			}
			break;
		case 5:
			{
				ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, GM_CALIB_DATA_MAX_FIELDS_T5, tokens, &index);
				if(ret){
					printf("tokenize_params failed! (Line %d)\n",__LINE__);
					goto err_mem;
				}

				for (index = 0; index < GM_CALIB_DATA_MAX_FIELDS_T5; ++index) {
					gamut_c->c5[index] = atoi(tokens[index]);
				}
			}
			break;
		case 6:
			{
				ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, GM_CALIB_DATA_MAX_FIELDS_T6, tokens, &index);
				if(ret){
					printf("tokenize_params failed! (Line %d)\n",__LINE__);
					goto err_mem;
				}

				for (index = 0; index < GM_CALIB_DATA_MAX_FIELDS_T6; ++index) {
					gamut_c->c6[index] = atoi(tokens[index]);
				}
			}
			break;
		case 7:
			{
				ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, GM_CALIB_DATA_MAX_FIELDS_T7, tokens, &index);
				if(ret){
					printf("tokenize_params failed! (Line %d)\n",__LINE__);
					goto err_mem;
				}

				for (index = 0; index < GM_CALIB_DATA_MAX_FIELDS_T7; ++index) {
					gamut_c->c7[index] = atoi(tokens[index]);
				}
			}
			break;
		default:
			break;
		}

		if(++tbl_num >= MDP_GAMUT_TABLE_NUM)
			break;
	}

	ret = fclose(fp);
err_mem:
	free(line);
err:
	return ret;
}


int postproc_framework_gm_handler(char* params)
{
	char* tokens[MIN_GM_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1, index=0;
	int mdp_block = 0;
	struct display_pp_gm_lut_data gm_lut_data;

	struct msmfb_mdp_pp pp;
	struct mdp_gamut_cfg_data *gamut_cfg_data = NULL;

	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_GM_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	temp_token = strstr(tokens[MIN_GM_PARAMS_REQUIRED-1],"\r\n");
	if(temp_token)
		*temp_token = '\0';

	mdp_block = atoi(tokens[0]);
	gm_lut_data.ops = atoi(tokens[1]);

	//Set PP configuration
	memset(&pp, 0, sizeof(pp));
	gamut_cfg_data = &pp.data.gamut_cfg_data;

	pp.op = mdp_op_gamut_cfg;
	gamut_cfg_data->block = mdp_block;
	gamut_cfg_data->flags = gm_lut_data.ops;

	gamut_cfg_data->tbl_size[0] = GAMUT_T0_SIZE;
	gamut_cfg_data->tbl_size[1] = GAMUT_T1_SIZE;
	gamut_cfg_data->tbl_size[2] = GAMUT_T2_SIZE;
	gamut_cfg_data->tbl_size[3] = GAMUT_T3_SIZE;
	gamut_cfg_data->tbl_size[4] = GAMUT_T4_SIZE;
	gamut_cfg_data->tbl_size[5] = GAMUT_T5_SIZE;
	gamut_cfg_data->tbl_size[6] = GAMUT_T6_SIZE;
	gamut_cfg_data->tbl_size[7] = GAMUT_T7_SIZE;

	gamut_cfg_data->r_tbl[0] = gm_lut_data.gamut_lut_tbl.r.c0;
	gamut_cfg_data->r_tbl[1] = gm_lut_data.gamut_lut_tbl.r.c1;
	gamut_cfg_data->r_tbl[2] = gm_lut_data.gamut_lut_tbl.r.c2;
	gamut_cfg_data->r_tbl[3] = gm_lut_data.gamut_lut_tbl.r.c3;
	gamut_cfg_data->r_tbl[4] = gm_lut_data.gamut_lut_tbl.r.c4;
	gamut_cfg_data->r_tbl[5] = gm_lut_data.gamut_lut_tbl.r.c5;
	gamut_cfg_data->r_tbl[6] = gm_lut_data.gamut_lut_tbl.r.c6;
	gamut_cfg_data->r_tbl[7] = gm_lut_data.gamut_lut_tbl.r.c7;

	gamut_cfg_data->g_tbl[0] = gm_lut_data.gamut_lut_tbl.g.c0;
	gamut_cfg_data->g_tbl[1] = gm_lut_data.gamut_lut_tbl.g.c1;
	gamut_cfg_data->g_tbl[2] = gm_lut_data.gamut_lut_tbl.g.c2;
	gamut_cfg_data->g_tbl[3] = gm_lut_data.gamut_lut_tbl.g.c3;
	gamut_cfg_data->g_tbl[4] = gm_lut_data.gamut_lut_tbl.g.c4;
	gamut_cfg_data->g_tbl[5] = gm_lut_data.gamut_lut_tbl.g.c5;
	gamut_cfg_data->g_tbl[6] = gm_lut_data.gamut_lut_tbl.g.c6;
	gamut_cfg_data->g_tbl[7] = gm_lut_data.gamut_lut_tbl.g.c7;

	gamut_cfg_data->b_tbl[0] = gm_lut_data.gamut_lut_tbl.b.c0;
	gamut_cfg_data->b_tbl[1] = gm_lut_data.gamut_lut_tbl.b.c1;
	gamut_cfg_data->b_tbl[2] = gm_lut_data.gamut_lut_tbl.b.c2;
	gamut_cfg_data->b_tbl[3] = gm_lut_data.gamut_lut_tbl.b.c3;
	gamut_cfg_data->b_tbl[4] = gm_lut_data.gamut_lut_tbl.b.c4;
	gamut_cfg_data->b_tbl[5] = gm_lut_data.gamut_lut_tbl.b.c5;
	gamut_cfg_data->b_tbl[6] = gm_lut_data.gamut_lut_tbl.b.c6;
	gamut_cfg_data->b_tbl[7] = gm_lut_data.gamut_lut_tbl.b.c7;

	ret = pp_gm_parse_calib_data(&(gm_lut_data.gamut_lut_tbl.r), tokens[2]);
	if(!ret){
		ret = pp_gm_parse_calib_data(&(gm_lut_data.gamut_lut_tbl.g), tokens[3]);
		if(!ret){
			ret = pp_gm_parse_calib_data(&(gm_lut_data.gamut_lut_tbl.b), tokens[4]);
			if(ret){
				printf("Cannot open file %s\n", tokens[4]);
			}
		}else{
			printf("Cannot open file %s\n", tokens[3]);
		}
	}else{
		printf("Cannot open file %s\n", tokens[2]);
	}

	printf("\nCalling user space library for Gamut Mapping!!\n");
	ret = ioctl(FB->fb_fd, MSMFB_MDP_PP, &pp);
err:
	return ret;
}

int print_pa_cfg(struct mdp_pa_cfg *pa_cfg_ptr){
	int ret = -1;
	if(pa_cfg_ptr){
		printf("======= PA PARAMS =================\n");
		printf("flags = %d \n", pa_cfg_ptr->flags);
		printf("Hue = %d\n", pa_cfg_ptr->hue_adj);
		printf("Saturation = %d\n", pa_cfg_ptr->sat_adj);
		printf("Intensity = %d\n", pa_cfg_ptr->val_adj);
		printf("Contrast = %d\n", pa_cfg_ptr->cont_adj);
		printf("========END OF PA PARAMS================\n");
		ret = 0;
	}
	return ret;
}

int rotateCheck(struct fbtest_params *thisFBTEST, int *flags){
	int ret = 0;

	if(!thisFBTEST || !flags){
		ret = -1;
		goto err;
	}

	switch (thisFBTEST->rotateDegree) {
	case ROT_270:
		*flags |= MDP_FLIP_LR|MDP_FLIP_UD;
		break;
	case ROT_90:
		*flags |= MDP_ROT_90;
		break;
	case ROT_180:
		*flags |= MDP_FLIP_LR|MDP_FLIP_UD;
		break;
	case ROT_0:
		break;
	default:
		VPRINT(verbose, "[ROTATE] Rotation degree is not proper\n");
		break;
	}

	switch (thisFBTEST->rotateFlip) {
	case FLIP_LR:
		*flags ^= MDP_FLIP_LR;
		break;
	case FLIP_UD:
		*flags ^= MDP_FLIP_UD;
		break;
	case FLIP_LRUD:
		*flags ^= MDP_FLIP_LR|MDP_FLIP_UD;
		break;
	case FLIP_NOP:
		break;
	default:
		VPRINT(verbose, "[ROTATE] Rotation Flip value is not proper\n");
		break;
	}
err:
	return ret;
}

int initOverlay(struct fbtest_params *thisFBTEST, struct mdp_overlay *overlay)
{
	uint32_t src_w, src_h;
	struct mdp_rect src, dst;
	int ret = 0;

	if(!thisFBTEST || !overlay){
		ret = -1;
		goto err;
	}

	src_w = thisFBTEST->fileParams.inResolution.width;
	src_h = thisFBTEST->fileParams.inResolution.height;

	src.x = thisFBTEST->coordinate.x ? : 0;
	src.y = thisFBTEST->coordinate.y ? : 0;
	src.w = thisFBTEST->cropresolution.width ? : src_w - src.x;
	src.h = thisFBTEST->cropresolution.height ? : src_h - src.y;

	dst.x = thisFBTEST->imgOffset.startX ? : 0;
	dst.y = thisFBTEST->imgOffset.startY ? : 0;
	dst.w = thisFBTEST->outResolution.width ? : src.w;
	dst.h = thisFBTEST->outResolution.height ? : src.h;

	ret = rotateCheck(thisFBTEST, &overlay->flags);
	if (ret) {
		printf("rotateCheck failed. \n");
		goto err;
	}

	overlay->src.format = thisFBTEST->fileParams.inputFormat;
	overlay->src.width  = src_w;
	overlay->src.height = src_h;
	overlay->z_order = 0;
	overlay->alpha = 0xFF;
	overlay->transp_mask = MDP_TRANSP_NOP;

	overlay->id = MSMFB_NEW_REQUEST;
	overlay->src_rect = src;
	overlay->dst_rect = dst;

err:
	return ret;
}

int initOvdata(struct fbtest_params *thisFBTEST,
                struct msmfb_overlay_data *ovdata)
{
	struct fb_var_screeninfo *vinfo;
	int mem_fd = -1;
	size_t bytesRead = 0;

	FILE *fptr = NULL;
	uint32_t frame_size, in_size, out_size;
	unsigned char *src_buf = 0, *dst_buf = 0;
	uint32_t src_offset = 0, dst_offset = 0;

	uint32_t flags = 0, data_flags;
	int ret = 0;

	ret = rotateCheck(thisFBTEST, &flags);
	if (ret) {
		printf("rotateCheck failed. \n");
		goto err;
	}

	frame_size = getFrameSize(thisFBTEST->fileParams.inResolution.width,
                              thisFBTEST->fileParams.inResolution.height,
                              thisFBTEST->fileParams.inputFormat);

	in_size = ALIGN(frame_size, 4096);

	out_size = getFrameSize(ALIGN(thisFBTEST->fileParams.inResolution.width, 128),
                            ALIGN(thisFBTEST->fileParams.inResolution.height, 128),
                            thisFBTEST->fileParams.inputFormat);

	out_size = ALIGN(out_size, 4096);

	vinfo = &(FB->fb_vinfo);

	printf("frame_size=%u in_size=%u out_size=%u\n", frame_size, in_size, out_size);

	if (!allocMEM(in_size + out_size)) {
		printf("using pmem\n");
		src_buf = MEM->mem_buf;
		src_offset = 0;
		dst_buf = src_buf + in_size;
		dst_offset = src_offset + in_size;
		mem_fd = MEM->mem_fd;

		data_flags = 0;
	} else {
		printf("using framebuffer memory\n");

		if (flags & MDP_ROT_90) {
			dst_offset = 0;
			dst_buf = FB->fb_buf + dst_offset;

			src_offset = dst_offset + out_size;
			src_buf = dst_buf + out_size;
		} else {
			int bpp = (vinfo->bits_per_pixel / 8);

			src_offset = (vinfo->xres * vinfo->yres * bpp);
			printf("bpp = %d src_offset=%d\n", bpp, src_offset);
			src_buf = FB->fb_buf + src_offset;
		}

		mem_fd = FB->fb_fd;
		data_flags = MDP_MEMORY_ID_TYPE_FB;
	}


	//open image file
	printf("fileParams.filenamePath: %s \n", thisFBTEST->fileParams.filenamePath);
	fptr = fopen(thisFBTEST->fileParams.filenamePath, "rb");
	if(fptr == NULL){
		printf("[FORMAT] Cannot open raw Image file!\n");
		ret = -FOPEN_FAILED;
		goto err;
	}

	//read image in buffer
	bytesRead  = fread(src_buf, 1, frame_size, fptr);
	if (bytesRead == 0) {
		printf("[FORMAT] fread failed! (Line %d)\n",__LINE__);
		fclose(fptr);
		ret = -FREAD_FAILED;
		goto err;
	}

	//close the file
	fclose(fptr);

	ovdata->data.memory_id = mem_fd;
	ovdata->data.offset = src_offset;
	ovdata->data.flags = data_flags;
	ovdata->dst_data.memory_id = mem_fd;
	ovdata->dst_data.offset = dst_offset;
	ovdata->dst_data.flags = data_flags;

err:
	return ret;

}

int postproc_framework_pa_handler(char* params)
{
	char* tokens[MIN_PA_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1, index=0;
	int mdp_block = 0;
	struct compute_params op_params;
	struct display_pp_pa_cfg pa_cfg;
	struct mdp_overlay_pp_params overlay_pp_params;
	struct msmfb_mdp_pp pp;

	struct fbtest_params *thisFBTEST = &FBTEST;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	memset(&overlay, 0, sizeof(overlay));
	memset(&ovdata, 0, sizeof(ovdata));

	printf("\n Enter pa_handler function. \n");
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_PA_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}
	temp_token = strstr(tokens[MIN_PA_PARAMS_REQUIRED-1],"\r\n");
	if(temp_token)
		*temp_token = '\0';

	mdp_block = atoi(tokens[0]);
	pa_cfg.ops = atoi(tokens[1]);
	pa_cfg.hue = atof(tokens[2]);
	pa_cfg.sat = atof(tokens[3]);
	pa_cfg.intensity = atoi(tokens[4]);
	pa_cfg.contrast = atof(tokens[5]);

	op_params.operation |= PP_OP_PA;
	op_params.params.pa_params = pa_cfg;

	ret = display_pp_compute_params(&op_params, &overlay_pp_params);
	ret = print_pa_cfg(&(overlay_pp_params.pa_cfg));

	switch (PP_LOCAT(mdp_block)) {
	case MDSS_PP_DSPP_CFG:
		memset(&pp, 0, sizeof(pp));
		pp.op = mdp_op_pa_cfg;
		pp.data.pa_cfg_data.block = mdp_block;
		pp.data.pa_cfg_data.pa_data = overlay_pp_params.pa_cfg;

		printf("\nCalling user space library for Picture Adjustment!!\n");
		ret = ioctl(FB->fb_fd, MSMFB_MDP_PP, &pp);
		break;
	case MDSS_PP_SSPP_CFG:
		ret = initOverlay(thisFBTEST, &overlay);
		if (ret) {
			printf("Overlay initialization failed! line=%d\n",  __LINE__);
			goto exit_with_errors;
		}

		VPRINT(verbose, "\nPerforming SPA Test...\n");

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
			ret = -OVERLAY_SET_FAILED;
			goto exit_with_errors;
		}
		ovdata.id = overlay.id;

		printf("overlay.id = %d\n",  overlay.id);
		overlay.flags |= MDP_OVERLAY_PP_CFG_EN;
		overlay.overlay_pp_cfg.config_ops = MDP_OVERLAY_PP_PA_CFG;
		overlay.overlay_pp_cfg.pa_cfg = overlay_pp_params.pa_cfg;

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		}

		ret = initOvdata(thisFBTEST, &ovdata);
		if (ret) {
			printf("Ovdata initialization failed! line=%d\n",  __LINE__);
			goto exit_with_errors;
		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			printf("MSMFB_OVERLAY_PLAY failed! line=%d\n", __LINE__);
			ret = -OVERLAY_PLAY_FAILED;
			goto exit_with_errors;
		}

		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
			ret = TEST_RESULT_FAIL;
			goto exit_with_errors;
		}

		VPRINT(verbose, "Displaying Image on device...\n");

		if (verbose) {
			printf("press any key to continue...\n");
			getchar();
		} else {
			sleep(5);
		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &ovdata)) {
			printf("\nERROR! OVERLAY_UNSET failed! Id=%d\n",
					ovdata.id);
			ret = TEST_RESULT_FAIL;
		}


exit_with_errors:
		VPRINT(verbose, "\n Test: %s\n", (ret == TEST_RESULT_PASS ? "PASS" : "FAIL"));
		break;
	default:
		goto err;
		break;
	}
err:
	return ret;
}

int postproc_parse_pa_v2_global_adj(struct display_pp_pa_v2_cfg *pa_v2_cfg, char *line_r)
{
	char *temp_token;
	char *end;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	pa_v2_cfg->global_hue = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	pa_v2_cfg->global_saturation = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	pa_v2_cfg->sat_thresh = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	pa_v2_cfg->global_value = (int)strtol(temp_token, &end, 0);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	pa_v2_cfg->global_contrast = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	return 0;
}

int postproc_parse_pa_v2_six_zone_hue(struct display_pp_pa_v2_cfg *pa_v2_cfg,
		char *line_r)
{
	int items = MDP_SIX_ZONE_LUT_SIZE, i = 0;
	char *temp_token, *end;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;
	do {
		pa_v2_cfg->six_zone_cfg.hue[i++] = strtof(temp_token, &end);
		if (temp_token == end) {
			printf("ERROR: invalid arguemnt in six zone hue LUT\n");
			return -EINVAL;
		}

		temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	} while (temp_token != NULL && i < items);

	if (i != items) {
		printf("ERROR: not enough hue items (%d/%d) for six zone LUT\n", i, items);
		return -EINVAL;
	}

	return 0;
}

int postproc_parse_pa_v2_six_zone_sat(struct display_pp_pa_v2_cfg *pa_v2_cfg,
		char *line_r)
{
	int items = MDP_SIX_ZONE_LUT_SIZE, i = 0;
	char *temp_token, *end;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	do {
		pa_v2_cfg->six_zone_cfg.sat[i++] = strtof(temp_token, &end);
		if (temp_token == end) {
			printf("ERROR: invalid arguemnt in six zone saturation LUT\n");
			return -EINVAL;
		}

		temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	} while (temp_token != NULL && i < items);

	if (i != items) {
		printf("ERROR: not enough sat items (%d/%d) for six zone LUT", i, items);
		return -EINVAL;
	}

	return 0;
}

int postproc_parse_pa_v2_six_zone_val(struct display_pp_pa_v2_cfg *pa_v2_cfg,
		char *line_r)
{
	int items = MDP_SIX_ZONE_LUT_SIZE, i = 0;
	char *temp_token, *end;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	do {
		pa_v2_cfg->six_zone_cfg.val[i++] = strtof(temp_token, &end);
		if (temp_token == end) {
			printf("ERROR: invalid arguemnt in six zone value LUT\n");
			return -EINVAL;
		}

		temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	} while (temp_token != NULL && i < items);

	if (i != items) {
		printf("ERROR: not enough val items (%d/%d) for six zone LUT", i, items);
		return -EINVAL;
	}

	return 0;
}

int postproc_parse_pa_v2_six_zone_thresh(struct display_pp_pa_v2_cfg *pa_v2_cfg,
		char *line_r)
{
	char *temp_token, *end;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	pa_v2_cfg->six_zone_cfg.sat_min = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	pa_v2_cfg->six_zone_cfg.val_min = (int)strtol(temp_token, &end, 0);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	pa_v2_cfg->six_zone_cfg.val_max = (int)strtol(temp_token, &end, 0);
	if (temp_token == end)
		return -EINVAL;

	return 0;
}

int postproc_parse_pa_v2_mem_col(struct display_pp_mem_col_cfg *cfg,
		char *line_r)
{
	char *temp_token;
	char *end;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->offset = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->hue_slope = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->sat_slope = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->val_slope = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->hue_min = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->hue_max = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->sat_min = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->sat_max = strtof(temp_token, &end);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->val_min = (int)strtol(temp_token, &end, 0);
	if (temp_token == end)
		return -EINVAL;

	temp_token = strtok_r(NULL, TOKEN_DELIM_STR, &line_r);
	if (!temp_token)
		return -EINVAL;

	cfg->val_max = (int)strtol(temp_token, &end, 0);
	if (temp_token == end)
		return -EINVAL;

	return 0;
}

int postproc_framework_pa_v2_handler(char* params)
{
	char* tokens[MIN_PA_V2_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1, line_op = 0;
	int i, index, items;
	int mdp_block = 0;
	struct display_pp_pa_v2_cfg pa_v2_cfg;
	struct compute_params op_params;
	struct mdp_overlay_pp_params overlay_pp_params;
	struct msmfb_mdp_pp pp;
	FILE *fp = NULL;
	char *line = NULL, *line_r;

	struct fbtest_params *thisFBTEST = &FBTEST;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	memset(&overlay, 0, sizeof(overlay));
	memset(&ovdata, 0, sizeof(ovdata));
	memset(&pa_v2_cfg, 0, sizeof(pa_v2_cfg));

	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_PA_V2_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("Tokenize_params failed! (Line %d) ret=%d\n",__LINE__, ret);
		goto err;
	}
	if (index < MIN_PA_V2_PARAMS_REQUIRED) {
		printf("Not enough parameters for PA v2 postproc test (%d/%d)",
				index, MIN_PA_V2_PARAMS_REQUIRED);
		ret = TEST_RESULT_SKIP;
		goto err;
	}

	mdp_block = atoi(tokens[0]);
	pa_v2_cfg.ops = atoi(tokens[1]);
	if (pa_v2_cfg.ops & MDP_PP_OPS_READ)
		goto compute_params;

	temp_token = strstr(tokens[2],"\r\n");
	if(temp_token)
		*temp_token = '\0';
	fp = fopen (tokens[2], "r");
	if (!fp) {
		printf("Cant open file %s errno: %d\n", tokens[2], errno);
		ret = TEST_RESULT_SKIP;
		goto err;
	}

	line = (char *)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		fclose(fp);
		goto err;
	}

	index = 0;
	while(fgets(line, MAX_CFG_LINE_LEN * sizeof(char), fp) &&
			index < PA_V2_LENGTH){
		if (line[0] == '\r' || line[0] == '\n' || line[0] == '#')
			continue;

		items = 0;
		i = 0;
		line_r = NULL;
		if ((temp_token = strtok_r(line, TOKEN_DELIM_STR, &line_r)) != NULL) {
			line_op = atoi(temp_token);
			switch(line_op) {
			case PA_GLOBAL_ADJ_LINE:
				ret = postproc_parse_pa_v2_global_adj(&pa_v2_cfg, line_r);
				if (ret) {
					printf("ERROR: not enough global adjustment items\n");
					fclose(fp);
					goto err;
				}
				break;
			case PA_SIX_ZONE_HUE_LINE:
				ret = postproc_parse_pa_v2_six_zone_hue(&pa_v2_cfg, line_r);
				if (ret)
					goto err;
				break;
			case PA_SIX_ZONE_SAT_LINE:
				ret = postproc_parse_pa_v2_six_zone_sat(&pa_v2_cfg, line_r);
				if (ret)
					goto err;
				break;
			case PA_SIX_ZONE_VAL_LINE:
				ret = postproc_parse_pa_v2_six_zone_val(&pa_v2_cfg, line_r);
				if (ret)
					goto err;
				break;
			case PA_SIX_ZONE_THRESH_LINE:
				ret = postproc_parse_pa_v2_six_zone_thresh(&pa_v2_cfg, line_r);
				if (ret) {
					printf("ERROR: not enough threshold items for six zone\n");
					goto err;
				}
				break;
			case PA_MEM_COL_SKIN_LINE:
				ret = postproc_parse_pa_v2_mem_col(&pa_v2_cfg.skin_cfg, line_r);
				if (ret) {
					printf("ERROR: not enough Skin memory color items\n");
					fclose(fp);
					goto err;
				}
				break;
			case PA_MEM_COL_SKY_LINE:
				ret = postproc_parse_pa_v2_mem_col(&pa_v2_cfg.sky_cfg, line_r);
				if (ret) {
					printf("ERROR: not enough Sky memory color items\n");
					fclose(fp);
					goto err;
				}
				break;
			case PA_MEM_COL_FOL_LINE:
				ret = postproc_parse_pa_v2_mem_col(&pa_v2_cfg.fol_cfg, line_r);
				if (ret) {
					printf("ERROR: not enough Foliage memory color items\n");
					fclose(fp);
					goto err;
				}
				break;
			default:
				printf("WARNING: unrecognized line operation (%d) in config text file.\n", line_op);
				break;
			}
		}
		index++;
	}
	fclose(fp);

compute_params:
	/* Allocate memory for the six zone LUT registers */
	if (pa_v2_cfg.ops & MDP_PP_PA_SIX_ZONE_ENABLE) {
		overlay_pp_params.pa_v2_cfg.six_zone_curve_p0 = (uint32_t *)
				calloc(MDP_SIX_ZONE_LUT_SIZE,
				sizeof(uint32_t));
		if (!overlay_pp_params.pa_v2_cfg.six_zone_curve_p0) {
			printf("Can't allocate memory for six zone LUTs\n");
			ret = -ENOMEM;
			goto err;
		}

		overlay_pp_params.pa_v2_cfg.six_zone_curve_p1 = (uint32_t *)
				calloc(MDP_SIX_ZONE_LUT_SIZE,
				sizeof(uint32_t));
		if (!overlay_pp_params.pa_v2_cfg.six_zone_curve_p1) {
			printf("Can't allocate memory for six zone LUTs\n");
			ret = -ENOMEM;
			goto err;
		}
	}

	printf("Computing picture adjustment v2 params\n");
	op_params.operation |= PP_OP_PA_V2;
	op_params.params.pa_v2_params = pa_v2_cfg;

	ret = display_pp_compute_params(&op_params, &overlay_pp_params);
	if (ret) {
		printf("Error computing PAv2 params, ret = %d\n", ret);
		goto err;
	}

	switch (PP_LOCAT(mdp_block)) {
	case MDSS_PP_DSPP_CFG:
		memset(&pp, 0, sizeof(pp));
		pp.op = mdp_op_pa_v2_cfg;
		pp.data.pa_v2_cfg_data.block = mdp_block;
		pp.data.pa_v2_cfg_data.pa_v2_data = overlay_pp_params.pa_v2_cfg;

		printf("\nCalling user space library for Picture Adjustment!!\n");
		ret = ioctl(FB->fb_fd, MSMFB_MDP_PP, &pp);
		break;
	case MDSS_PP_SSPP_CFG:
		ret = initOverlay(thisFBTEST, &overlay);
		if (ret) {
			printf("Overlay initialization failed! line=%d\n",  __LINE__);
			goto exit_with_errors;
		}

		VPRINT(verbose, "\nPerforming SPA Test...\n");

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
			ret = -OVERLAY_SET_FAILED;
			goto exit_with_errors;
		}
		ovdata.id = overlay.id;

		printf("overlay.id = %d\n",  overlay.id);
		overlay.flags |= MDP_OVERLAY_PP_CFG_EN;
		overlay.overlay_pp_cfg.config_ops = MDP_OVERLAY_PP_PA_V2_CFG;
		overlay.overlay_pp_cfg.pa_v2_cfg = overlay_pp_params.pa_v2_cfg;

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
			printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		}

		ret = initOvdata(thisFBTEST, &ovdata);
		if (ret) {
			printf("Ovdata initialization failed! line=%d\n",  __LINE__);
			goto exit_with_errors;
		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			printf("MSMFB_OVERLAY_PLAY failed! line=%d\n", __LINE__);
			ret = -OVERLAY_PLAY_FAILED;
			goto exit_with_errors;
		}

		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
			ret = TEST_RESULT_FAIL;
			goto exit_with_errors;
		}

		VPRINT(verbose, "Displaying Image on device...\n");

		if (verbose) {
			printf("press any key to continue...\n");
			getchar();
		} else {
			sleep(5);
		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &ovdata)) {
			printf("\nERROR! OVERLAY_UNSET failed! Id=%d\n",
					ovdata.id);
			ret = TEST_RESULT_FAIL;
		}

exit_with_errors:
		VPRINT(verbose, "\n Test: %s\n", (ret == TEST_RESULT_PASS ? "PASS" : "FAIL"));
		break;
	default:
		goto err;
		break;
	}
err:
	free(line);
	if (pa_v2_cfg.ops & MDP_PP_PA_SIX_ZONE_ENABLE) {
		free(overlay_pp_params.pa_v2_cfg.six_zone_curve_p0);
		free(overlay_pp_params.pa_v2_cfg.six_zone_curve_p1);
	}
	if (ret)
		printf("Failure in PAv2 handler, ret = %d\n", ret);
	return ret;
}

int postproc_framework_sharp_handler(char* params)
{
	char* tokens[MIN_SHARP_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1, index=0;
	int mdp_block = 0;
	struct display_pp_sharp_cfg sharp_cfg;
	struct mdp_overlay_pp_params overlay_pp_params;
	struct msmfb_mdp_pp pp;

	struct fbtest_params *thisFBTEST = &FBTEST;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;

	memset(&overlay, 0, sizeof(overlay));
	memset(&ovdata, 0, sizeof(ovdata));

	printf("\n Enter sharp_handler function. \n");
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_SHARP_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}
	temp_token = strstr(tokens[MIN_SHARP_PARAMS_REQUIRED-1],"\r\n");
	if(temp_token)
		*temp_token = '\0';

	mdp_block = atoi(tokens[0]);
	sharp_cfg.ops = atoi(tokens[1]);
	sharp_cfg.strength = atoi(tokens[2]);
	sharp_cfg.edge_thr = atoi(tokens[3]);
	sharp_cfg.smooth_thr = atoi(tokens[4]);
	sharp_cfg.noise_thr = atoi(tokens[5]);

	ret = display_pp_compute_sharp_params(&sharp_cfg, &overlay_pp_params);

	ret = initOverlay(thisFBTEST, &overlay);
	if (ret) {
		printf("Overlay initialization failed! line=%d\n",  __LINE__);
		goto exit_with_errors;
	}

	VPRINT(verbose, "\nPerforming sharpening Test...\n");

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
		printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		ret = -OVERLAY_SET_FAILED;
		goto exit_with_errors;
	}

	curr_overlay_id[overlay_used++] = overlay.id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay.id;

	printf("overlay.id = %d\n",  overlay.id);
	overlay.flags |= MDP_OVERLAY_PP_CFG_EN;
	overlay.overlay_pp_cfg.config_ops = MDP_OVERLAY_PP_SHARP_CFG;
	overlay.overlay_pp_cfg.sharp_cfg = overlay_pp_params.sharp_cfg;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
		printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
	}

	ret = initOvdata(thisFBTEST, &ovdata);
	if (ret) {
		printf("Ovdata initialization failed! line=%d\n",  __LINE__);
		goto exit_with_errors;
	}

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		printf("MSMFB_OVERLAY_PLAY failed! line=%d\n", __LINE__);
		ret = -OVERLAY_PLAY_FAILED;
		goto exit_with_errors;
	}

	if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
		printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
		ret = TEST_RESULT_FAIL;
		goto exit_with_errors;
	}

	VPRINT(verbose, "Displaying Image on device...\n");

	if (verbose) {
		printf("press any key to continue...\n");
		getchar();
	} else {
		sleep(5);
	}

	printf("testing writeback init ioctl %d\n", ioctl(FB->fb_fd, MSMFB_WRITEBACK_INIT, NULL));

exit_with_errors:
	VPRINT(verbose, "\n Test: %s\n", (ret == TEST_RESULT_PASS ? "PASS" : "FAIL"));
err:
	return ret;
}

int pp_csc_parse_calib_data(struct mdp_csc_cfg *csc_cfg,
	const char* file_path){
	int ret = -1, tbl_num = 0;
	FILE *fp = NULL;
	char *line = NULL;
	char* tokens[CSC_CALIB_DATA_MAX_FIELDS_MV];
	char *temp_token = NULL;
	int index = 0;

	if(!csc_cfg || !file_path)
		return ret;

	fp = fopen (file_path, "r");
	if(!fp ){
		printf("Cant open file %s\n", file_path);
		ret = TEST_RESULT_SKIP;
		return ret;
	}

	line = (char*)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		goto err;
	}
	while(fgets(line, MAX_CFG_LINE_LEN*sizeof(char),fp)){
		if (line[0] == '\r' || line[0] == '\n')
			continue;
		memset(tokens, 0, sizeof(tokens));
		index = 0;

		switch(tbl_num) {
		case 0:
			{
                ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, CSC_CALIB_DATA_MAX_FIELDS_MV, tokens, &index);
                if(ret){
                    printf("tokenize_params failed! (Line %d)\n",__LINE__);
                    goto err_mem;
                }

				for (index = 0; index < CSC_CALIB_DATA_MAX_FIELDS_MV; ++index) {
					csc_cfg->csc_mv[index] = atoi(tokens[index]);
				}
			}
			break;
		case 1:
			{
                ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, CSC_CALIB_DATA_MAX_FIELDS_BV, tokens, &index);
                if(ret){
                    printf("tokenize_params failed! (Line %d)\n",__LINE__);
                    goto err_mem;
                }
				for (index = 0; index < CSC_CALIB_DATA_MAX_FIELDS_BV; ++index) {
					csc_cfg->csc_pre_bv[index] = atoi(tokens[index]);
				}
			}
			break;
		case 2:
			{
                ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, CSC_CALIB_DATA_MAX_FIELDS_BV, tokens, &index);
                if(ret){
                    printf("tokenize_params failed! (Line %d)\n",__LINE__);
                    goto err_mem;
                }
				for (index = 0; index < CSC_CALIB_DATA_MAX_FIELDS_BV; ++index) {
					csc_cfg->csc_post_bv[index] = atoi(tokens[index]);
				}
			}
			break;
		case 3:
			{
                ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, CSC_CALIB_DATA_MAX_FIELDS_LV, tokens, &index);
                if(ret){
                    printf("tokenize_params failed! (Line %d)\n",__LINE__);
                    goto err_mem;
                }
				for (index = 0; index < CSC_CALIB_DATA_MAX_FIELDS_LV; ++index) {
					csc_cfg->csc_pre_lv[index] = atoi(tokens[index]);
				}
			}
			break;
		case 4:
			{
                ret = tokenize_params(line, CALIB_DATA_FIELD_SEPARATOR, CSC_CALIB_DATA_MAX_FIELDS_LV, tokens, &index);
                if(ret){
                    printf("tokenize_params failed! (Line %d)\n",__LINE__);
                    goto err_mem;
                }
				for (index = 0; index < CSC_CALIB_DATA_MAX_FIELDS_LV; ++index) {
					csc_cfg->csc_post_lv[index] = atoi(tokens[index]);
				}
			}
			break;

		default:
			break;
		}

		if(++tbl_num >= CSC_CALIB_DATA_MAX_LINES)
			break;
	}

	ret = fclose(fp);
err_mem:
	free(line);
err:
	return ret;
}


int postproc_framework_csc_handler(char* params)
{
	char* tokens[MIN_CSC_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1, index=0;
	int mdp_block = 0;
	struct mdp_csc_cfg csc_cfg;
	printf("\n Enter csc_handler function. \n");
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_CSC_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}
	temp_token = strstr(tokens[MIN_CSC_PARAMS_REQUIRED-1],"\r\n");
	if(temp_token)
		*temp_token = '\0';

	csc_cfg.flags = atoi(tokens[1]);
	ret = pp_csc_parse_calib_data(&csc_cfg, tokens[2]);

	struct fbtest_params *thisFBTEST = &FBTEST;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	memset(&overlay, 0, sizeof(overlay));
	memset(&ovdata, 0, sizeof(ovdata));

	ret = initOverlay(thisFBTEST, &overlay);
	if (ret) {
		printf("Overlay initialization failed! line=%d\n",  __LINE__);
		goto exit_with_errors;
	}

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
		printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
		ret = -OVERLAY_SET_FAILED;
		goto exit_with_errors;
	}

	curr_overlay_id[overlay_used++] = overlay.id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay.id;

	overlay.flags |= MDP_OVERLAY_PP_CFG_EN;
	overlay.overlay_pp_cfg.config_ops = MDP_OVERLAY_PP_CSC_CFG;
	overlay.overlay_pp_cfg.csc_cfg = csc_cfg;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay) < 0) {
		printf("MSMFB_OVERLAY_SET failed! line=%d\n",  __LINE__);
	}

	ret = initOvdata(thisFBTEST, &ovdata);
	if (ret) {
		printf("Ovdata initialization failed! line=%d\n",  __LINE__);
		goto exit_with_errors;
	}

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		printf("MSMFB_OVERLAY_PLAY failed! line=%d\n", __LINE__);
		ret = -OVERLAY_PLAY_FAILED;
		goto exit_with_errors;
	}

	if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
		printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
		ret = TEST_RESULT_FAIL;
		goto exit_with_errors;
	}

	VPRINT(verbose, "Displaying Image on device...\n");

	if (verbose) {
		printf("press any key to continue...\n");
		getchar();
	} else {
		sleep(5);
	}

	printf("testing writeback init ioctl %d\n", ioctl(FB->fb_fd, MSMFB_WRITEBACK_INIT, NULL));

exit_with_errors:
	VPRINT(verbose, "\n Test: %s\n", (ret == TEST_RESULT_PASS ? "PASS" : "FAIL"));
err:
	return ret;

}

int postproc_framework_pcc_handler(char* params){
	char* tokens[MIN_PCC_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1, index=0;
	struct display_pp_pcc_cfg pcc_cfg;
	int mdp_block = 0, offset=0;
	struct display_pcc_coeff *coeff_ptr = NULL;

	memset(tokens, 0, sizeof(tokens));
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_PCC_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	mdp_block = atoi(tokens[0]);
	pcc_cfg.ops = (uint32_t)atoi(tokens[1]);
	coeff_ptr = &pcc_cfg.r;
	while(33 != offset){
		for(index=0;index<11;index++){
			if (coeff_ptr)
				coeff_ptr->pcc_coeff[index]=atof(tokens[index
					+ PCC_COEFF_START_INDEX+offset]);
		}
		offset += 11;
		switch(offset){
		case 11:
			coeff_ptr = &pcc_cfg.g;
			break;

		case 22:
			coeff_ptr = &pcc_cfg.b;
			break;

		default:
			coeff_ptr = NULL;
			break;
		}
	}

	print_pcc_cfg(&pcc_cfg);

	printf("\nCalling user space library for PCC!!\n");
	ret = display_pp_pcc_set_cfg(mdp_block, &pcc_cfg);
err:
	return ret;
}

int pp_argc_parse_calib_data(struct display_pp_argc_stage_data *stage_data,
		const char* file_path){
	int ret = -1, stage_num = 0;
	FILE *fp = NULL;
	char *line = NULL;
	char* tokens[ARGC_CALIB_DATA_MAX_FIELDS];
	char *temp_token = NULL;
	int index = 0;

	if(!stage_data || !file_path)
		goto err;

	fp = fopen (file_path, "r");
	if(!fp ){
		goto err;
	}

	line = (char *)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		goto err;
	}

	while(fgets(line, MAX_CFG_LINE_LEN*sizeof(char),fp)){
		if (line[0] == '\r' || line[0] == '\n')
			continue;

		memset(tokens, 0, sizeof(tokens));
		index = 0;

		temp_token = strtok (line, CALIB_DATA_FIELD_SEPARATOR);
		while (temp_token != NULL) {
			tokens[index++] = temp_token;
			if(index < ARGC_CALIB_DATA_MAX_FIELDS){
				temp_token = strtok (NULL,
						CALIB_DATA_FIELD_SEPARATOR);
			} else {
				break;
			}
		}   //end while

		stage_data[stage_num].is_stage_enabled = atoi(tokens[0]);
		if( stage_data[stage_num].is_stage_enabled ){
			stage_data[stage_num].x_start = atoi(tokens[1]);
			stage_data[stage_num].slope = atof(tokens[2]);
			stage_data[stage_num].offset = atof(tokens[3]);
		} else {
			stage_data[stage_num].x_start = 0;
			stage_data[stage_num].slope = 0;
			stage_data[stage_num].offset = 0;
		}

		if(++stage_num >= NUM_ARGC_STAGES)
			break;	//done reading data for all stages
	}

	ret = fclose(fp);
	free(line);
err:
	return ret;
}

int postproc_framework_argc_handler(char* params){
	char* tokens[MIN_ARGC_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1, index=0;
	struct display_pp_pcc_cfg pcc_cfg;
	int mdp_block = 0, offset=0;
	struct display_pp_argc_lut_data argc_lut_data;

	memset(tokens, 0, sizeof(tokens));
	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_ARGC_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	temp_token = strstr(tokens[4],"\r\n");
	if(temp_token)
		*temp_token = '\0';

	mdp_block = atoi(tokens[0]);
	argc_lut_data.ops = atoi(tokens[1]);
	ret = pp_argc_parse_calib_data(&argc_lut_data.r[0], tokens[2]);
	if (!ret ) {
		ret = pp_argc_parse_calib_data(&argc_lut_data.g[0], tokens[3]);
		if(!ret ) {
			ret = pp_argc_parse_calib_data(&argc_lut_data.b[0],
					tokens[4]);
		}
	}

	if(ret)
		return ret;

	ret = print_argc_cfg(&argc_lut_data);

	printf("\nCalling user space library for AR GC!!\n");
	ret = display_pp_argc_set_lut(mdp_block, &argc_lut_data);
err:
	return ret;
}

int postproc_framework_ad_handler(char* params) {
	char* tokens[MIN_AD_PARAMS_REQUIRED];
	int ret = TEST_RESULT_SKIP;
	FILE *fp = NULL;
	char *line = NULL;
	char *temp_token = NULL;
	uint32_t temp;
	int index = 0, i = 0, j = 0;
	int items, step;

	struct msmfb_mdp_pp pp, in;

	struct mdss_ad_init *init = &pp.data.ad_init_cfg.params.init;
	struct mdss_ad_cfg *cfg = &pp.data.ad_init_cfg.params.cfg;

	struct mdss_ad_input *input = &in.data.ad_input;

	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_AD_PARAMS_REQUIRED, tokens, &index);
	if (ret) {
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	pp.op = mdp_op_ad_cfg;
	pp.data.ad_init_cfg.ops = atoi(tokens[0]) | MDP_PP_AD_INIT;
	in.op = mdp_op_ad_input;

	/* Initialization parsing*/
	fp = fopen(tokens[1], "r");
	if (!fp)
		goto err;

	line = (char *)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		goto err;
	}

	while(fgets(line, MAX_CFG_LINE_LEN * sizeof(char), fp) && i < 17) {
		if (line[0] == '\r' || line[0] == '\n' || line[0] == '#')
			continue;
		switch (i) {
		case 0:
			items = 33;
			break;
		case 1:
			items = 33;
			break;
		case 2:
			items = 2;
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
			items = 1;
			break;
		default:
			break;
		}
		printf("\n", temp);
		temp_token = strtok(line, CALIB_DATA_FIELD_SEPARATOR);
		if (temp_token != NULL) {
			j = 0;
			do {
				temp = atoi(temp_token);
				printf("%d ", temp);
				switch (i) {
				case 0:
					init->asym_lut[j] = temp;
					break;
				case 1:
					init->color_corr_lut[j] = temp;
					break;
				case 2:
					init->i_control[j] = (uint8_t) temp;
					break;
				case 3:
					init->black_lvl = (uint8_t) temp;
					break;
				case 4:
					init->white_lvl = (uint16_t) temp;
					break;
				case 5:
					init->var = (uint8_t) temp;
					break;
				case 6:
					init->limit_ampl = (uint8_t) temp;
					break;
				case 7:
					init->i_dither = (uint8_t) temp;
					break;
				case 8:
					init->slope_max = (uint8_t) temp;
					break;
				case 9:
					init->slope_min = (uint8_t) temp;
					break;
				case 10:
					init->dither_ctl = (uint8_t) temp;
					break;
				case 11:
					init->format = (uint8_t) temp;
					break;
				case 12:
					init->auto_size = (uint8_t) temp;
					break;
				case 13:
					init->frame_w = (uint16_t) temp;
					break;
				case 14:
					init->frame_h = (uint16_t) temp;
					break;
				case 15:
					init->logo_v = (uint8_t) temp;
					break;
				case 16:
					init->logo_h = (uint8_t) temp;
					break;
				default:
					break;
				}
				j++;
				temp_token = strtok(NULL, CALIB_DATA_FIELD_SEPARATOR);
			} while (temp_token != NULL && j < items);
			if (j != items)
				printf("not enough items (%d/%d) on input line %d", j, items, i);
		}
		i++;
	}
	ret = fclose(fp);
	printf("\nCalling ioctl for AD init!\n");
	if (ioctl(FB->fb_fd, MSMFB_MDP_PP, &pp)) {
		printf("ioctl failed, errno = %d", errno);
		ret = TEST_RESULT_FAIL;
	} else
		ret = TEST_RESULT_PASS;

	if (verbose) {
		printf("press any key to continue...\n");
		getchar();
	} else {
		sleep(5);
	}

	/* Calibration parsing*/
	pp.data.ad_init_cfg.ops = MDP_PP_AD_CFG;
	memset(&pp.data.ad_init_cfg.params.cfg, 0, sizeof(struct mdss_ad_cfg));

	fp = fopen(tokens[2], "r");
	if (!fp)
		goto err;

	line = (char *)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		goto err;
	}
	printf("Parsing CFG ...... \n");

	i = 0;
	while(fgets(line, MAX_CFG_LINE_LEN * sizeof(char), fp) && i < 11) {
		if (line[0] == '\r' || line[0] == '\n' || line[0] == '#')
			continue;
		switch (i) {
		case 0:
		case 2:
		case 3:
		case 4:
		case 5:
		case 8:
		case 9:
		case 10:
			items = 1;
			break;
		case 1:
			items = 33;
			break;
		case 6:
			items = 2;
			break;
		case 7:
			items = 4;
			break;
		default:
			break;
		}
		printf("\n");
		temp_token = strtok(line, CALIB_DATA_FIELD_SEPARATOR);
		if (temp_token != NULL) {
			j = 0;
			do {
				temp = atoi(temp_token);
				printf("%d ", temp);
				switch (i) {
				case 0:
					cfg->mode = temp;
					break;
				case 1:
					cfg->al_calib_lut[j] = temp;
					break;
				case 2:
					cfg->backlight_min = (uint16_t) temp;
					break;
				case 3:
					cfg->backlight_max = (uint16_t) temp;
					break;
				case 4:
					cfg->backlight_scale = (uint16_t) temp;
					break;
				case 5:
					cfg->amb_light_min = (uint16_t) temp;
					break;
				case 6:
					cfg->filter[j] = (uint16_t) temp;
					break;
				case 7:
					cfg->calib[j] = (uint16_t) temp;
					break;
				case 8:
					cfg->strength_limit = (uint8_t) temp;
					break;
				case 9:
					cfg->t_filter_recursion = (uint8_t) temp;
					break;
				case 10:
					cfg->stab_itr = (uint16_t) temp;
					break;
				default:
					break;
				}
				j++;
				temp_token = strtok(NULL, CALIB_DATA_FIELD_SEPARATOR);
			} while (temp_token != NULL && j < items);
			if (j != items)
				printf("not enough items (%d/%d) on input line %d", j, items, i);
		}
		i++;
	}
	ret = fclose(fp);
	printf("\nCalling ioctl for AD cfg!\n");
	if (ioctl(FB->fb_fd, MSMFB_MDP_PP, &pp)) {
		printf("ioctl failed, errno = %d", errno);
		ret = TEST_RESULT_FAIL;
	} else
		ret = TEST_RESULT_PASS;

	if (verbose) {
		printf("press any key to continue...\n");
		getchar();
	} else {
		sleep(5);
	}

	input->mode = cfg->mode;

	fopen(tokens[3], "r");
	if (!fp)
		goto err;
	while(fgets(line, MAX_CFG_LINE_LEN * sizeof(char), fp)) {
		if (line[0] == '\r' || line[0] == '\n' || line[0] == '#')
			continue;
		temp_token = strtok(line, CALIB_DATA_FIELD_SEPARATOR);
		if (temp_token != NULL) {
			do {
				temp = atoi(temp_token);
				printf("\nCalling ioctl for AD in, input = %d!\n", temp);
				input->in.amb_light = temp;
				if (ioctl(FB->fb_fd, MSMFB_MDP_PP, &in)) {
					printf("ioctl failed, errno = %d", errno);
					ret = TEST_RESULT_FAIL;
				} else
					ret = TEST_RESULT_PASS;
				temp_token = strtok(NULL, CALIB_DATA_FIELD_SEPARATOR);
				if (verbose) {
					printf("press key for next");
					getchar();
				} else
					usleep(200000);
			} while (temp_token != NULL);
		}
	}
	ret = fclose(fp);
	free(line);

err:
	return ret;
}


#define CONVERSION_RGB2YUV 0
#define CONVERSION_YUV2RGB 1

int pp_hsic_parse_calib_data(struct display_hsic_cust_params *hsic_cfg,
		int conversion, const char* file_path) {
	int ret = -1, line_num = 0;
	FILE *fp = NULL;
	char *line = NULL;
	float tokens[HSIC_CALIB_DATA_MAX_FIELDS];
	char *temp_token = NULL;
	int i, index = 0;

	memset(tokens, 0, sizeof(tokens));

	if (!hsic_cfg || !file_path)
		goto err;

	if (conversion != CONVERSION_RGB2YUV && conversion !=CONVERSION_YUV2RGB)
		goto err;

	fp = fopen (file_path, "r");
	if (!fp)
		goto err;

	line = (char *)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		goto err;
	}

	while(fgets(line, MAX_CFG_LINE_LEN * sizeof(char), fp) &&
			index < HSIC_CALIB_DATA_MAX_LINES) {
		if (line[0] == '\r' || line[0] == '\n')
			continue;

		i = 0;
		if ((temp_token = strtok(line, CALIB_DATA_FIELD_SEPARATOR)) != NULL) {
			do {
				tokens[i++] = atof(temp_token);
				temp_token = strtok(NULL, CALIB_DATA_FIELD_SEPARATOR);
			} while (temp_token != NULL && i < HSIC_CALIB_DATA_MAX_FIELDS);
		}
		if (i != HSIC_CALIB_DATA_MAX_FIELDS)
			break;

		if (conversion == CONVERSION_RGB2YUV) {
			hsic_cfg->cust_rgb2yuv[index][0] = tokens[0];
			hsic_cfg->cust_rgb2yuv[index][1] = tokens[1];
			hsic_cfg->cust_rgb2yuv[index][2] = tokens[2];
		} else if (conversion == CONVERSION_YUV2RGB) {
			hsic_cfg->cust_yuv2rgb[index][0] = tokens[0];
			hsic_cfg->cust_yuv2rgb[index][1] = tokens[1];
			hsic_cfg->cust_yuv2rgb[index][2] = tokens[2];
		}
		index++;
	}
	ret = fclose(fp);
	free(line);
err:
	return ret;
}

int postproc_framework_hsic_handler(char* params){
	char* tokens[MIN_HSIC_PARAMS_REQUIRED];
	char *temp_token = NULL;
	int ret = -1, index=0;
	struct display_pp_conv_cfg pp_cfg;
	int mdp_block = 0;
	struct display_hsic_cust_params *hsic_cfg = NULL;

	memset(tokens, 0, sizeof(tokens));

	ret = tokenize_params(params, TOKEN_PARAMS_DELIM, MIN_HSIC_PARAMS_REQUIRED, tokens, &index);
	if(ret){
		printf("tokenize_params failed! (Line %d)\n",__LINE__);
		goto err;
	}

	if (index >= 7) {
		hsic_cfg = malloc(sizeof(struct display_hsic_cust_params));
		if (!hsic_cfg)
			return ret;
		else
			memset(hsic_cfg, 0, sizeof(struct display_hsic_cust_params));
	} else {
		hsic_cfg = NULL;
	}

	ret = populate_default_pp_cfg(&pp_cfg);

	if(!ret){
		mdp_block = atoi(tokens[0]);
		pp_cfg.ops = (uint32_t)atoi(tokens[1]);
		pp_cfg.hue = atoi(tokens[2]);
		pp_cfg.sat = atof(tokens[3]);
		pp_cfg.intensity = atoi(tokens[4]);
		pp_cfg.contrast = atof(tokens[5]);
		if (index >= 7) {
			hsic_cfg->intf = atoi(tokens[6]);
			printf("hsic calib\n");
			if (index == 9) {
				temp_token = strstr(tokens[7],"\r\n");
				if(temp_token)
					*temp_token = '\0';
				temp_token = strstr(tokens[8],"\r\n");
				if(temp_token)
					*temp_token = '\0';
				pp_hsic_parse_calib_data(hsic_cfg,
						CONVERSION_RGB2YUV, tokens[7]);
				pp_hsic_parse_calib_data(hsic_cfg,
						CONVERSION_YUV2RGB, tokens[8]);
			}
		}
		print_conv_cfg(&pp_cfg, hsic_cfg);

		ret = display_pp_conv_init(mdp_block, hsic_cfg);
		if( 0 == ret ){
			ret = display_pp_conv_set_cfg(mdp_block,&pp_cfg);
		}
	}

	if (hsic_cfg) {
		free(hsic_cfg);
		hsic_cfg = NULL;
	}
err:
	return ret;
}

int parsePPFrameworkFile(const char* cfg_file_path) {
	FILE *fp = NULL;
	char *line = NULL, *input_line = NULL;
	char* tokens[MAX_FRAMEWORK_TOKENS_REQUIRED];
	char *temp_token = NULL;
	int index = 0;
	int ret = -1, op_type=-1, op_code=-1, count;
	int test_ret = -1;
	int num_threads = 0;
	struct repeat_info rinfo;

	fp = fopen (cfg_file_path, "r");
	if (!fp) {
		printf("File Open Failed %s Line: %d errno: %s\n",
		cfg_file_path, __LINE__, strerror(errno));
		ret = errno;
		goto err;
	}

	/* initialize the postproc lib */
	ret = display_pp_init();
	if (ret) {
		printf("display_pp_init() failed: errno: %d\n", ret);
		fclose(fp);
		goto err;
	}

	memset(&rinfo, 0, sizeof(struct repeat_info));
	rinfo.fp = fp;

	line = (char *)malloc(MAX_CFG_LINE_LEN * sizeof(char));
	if(!line){
		printf("Cant allocate memory\n");
		goto err;
	}

	while(fgets(line, MAX_CFG_LINE_LEN*sizeof(char),fp)){
		if (line[0] == '\r' || line[0] == '\n' || line[0] == '#')
			continue;

		index = 0;
		count = 0;
		while (count < MAX_FRAMEWORK_TOKENS_REQUIRED)
			tokens[count++] = NULL;

		op_type=-1;
		op_code=-1;

		/* Pick up String till # is encountered */
		input_line = strtok(line, (char *)TOKEN_HASH_DELIM_STR);
		if (input_line == NULL)
			input_line = (char *)&line;

		temp_token = strtok(input_line, (char *)TOKEN_DELIM_STR);
		while (temp_token != NULL) {
			tokens[index++] = temp_token;
			if(index<MAX_FRAMEWORK_TOKENS_REQUIRED){
				temp_token = strtok(NULL, TOKEN_DELIM_STR);
			} else {
				break;
			}
		}   //end while

		op_type = atoi(tokens[0]);

		switch(op_type){
		case TOKEN_OP_TYPE_SET:
			op_code = atoi(tokens[1]);
			switch(op_code){
			case TOKEN_OP_CODE_HSIC:
				ret = postproc_framework_hsic_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_PCC:
				ret = postproc_framework_pcc_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_GM:
				ret = postproc_framework_gm_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_PA:
				ret = postproc_framework_pa_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_PA_V2:
				ret = postproc_framework_pa_v2_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_CSC:
				ret = postproc_framework_csc_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_ARGC:
				ret = postproc_framework_argc_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_IGC:
				ret = postproc_framework_igc_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_HIST_LUT:
				ret = postproc_framework_hist_lut_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_SRC_HIST:
				ret = postproc_framework_overlay_hist_handler(
						&num_threads, tokens[2]);
				break;

			case TOKEN_OP_CODE_HIST_START:
				ret = postproc_framework_hist_start_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_HIST_STOP:
				ret = postproc_framework_hist_stop_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_DAEMON_MSG:
				ret = postproc_framework_daemon_test(tokens[2]);
				break;

			case TOKEN_OP_CODE_SHARP:
				ret = postproc_framework_sharp_handler(tokens[2]);
				break;

			case TOKEN_OP_CODE_QSEED:
				ret =
				postproc_framework_qseed_handler(PP_OPS_WRITE, tokens[2]);
				break;

			case TOKEN_OP_CODE_CALIB:
				ret = postproc_framework_calib_handler(PP_OPS_WRITE, tokens[2]);
				break;
			case TOKEN_OP_CODE_AD:
				ret = postproc_framework_ad_handler(tokens[2]);
				break;
			default:
				printf("Unsupported sub-operation type: %s %s",
						tokens[0], tokens[1]);
				break;
			}
			break;

		case TOKEN_OP_TYPE_SLEEP:
			sleep(atoi(tokens[1]));
			ret = TEST_RESULT_PASS;
			break;

		case TOKEN_OP_TYPE_GET:
			op_code = atoi(tokens[1]);
			switch (op_code) {
			case TOKEN_OP_CODE_HIST_READ:
				ret = postproc_framework_hist_read_handler(&num_threads, tokens[2]);
				break;
			case TOKEN_OP_CODE_QSEED:
				ret =
				postproc_framework_qseed_handler(PP_OPS_READ, tokens[2]);
				break;
			case TOKEN_OP_CODE_CALIB:
				ret =
				postproc_framework_calib_handler(PP_OPS_READ, tokens[2]);
				break;

			default:
				printf("Unsupported sub-operation type: %s %s",
						tokens[0], tokens[1]);
				break;
			}
			break;
		case TOKEN_OP_TYPE_THREAD_JOIN:
			ret = postproc_framework_thread_join_handler(&num_threads,tokens[1]);
			break;

		case TOKEN_OP_TYPE_REPEAT:
			ret = postproc_framework_repeat_handler(&rinfo, op_type,
							tokens[1], tokens[2]);
			break;
		default:
			printf("Unsupported operation type: %s ", tokens[0]);
			break;
		}

		if (ret < 0)
			printf("Failed Test case (%d): %s %s", ret, tokens[0], tokens[1]);

		if (rinfo.repeat_initialised)
			ret = postproc_framework_repeat_handler(&rinfo, op_type,
							tokens[1], tokens[2]);
	}
	test_ret = ret;
	/*  free the postproc resources */
	ret = display_pp_exit();

	if (!ret)
		ret = test_ret;

	fclose(fp);

	free(line);
err:
	return ret;
}


