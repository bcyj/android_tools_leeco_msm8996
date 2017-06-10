/*
 *
 * This library contains the API to load the audio calibration
 * data from database and push to the DSP
 *
 * Copyright (c) 2010-2013 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */


#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/msm_ion.h>
#include <linux/msm_audio.h>
#include <linux/msm_audio_acdb.h>
#include <linux/mfd/timpani-audio.h>
#include <linux/mfd/wcd9xxx/wcd9xxx_registers.h>
#include <linux/mfd/wcd9xxx/wcd9310_registers.h>

#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t

#include <linux/mfd/msm-adie-codec.h>
#include <sys/mman.h>

#ifdef _ANDROID_
/* definitions for Android logging */
#include <utils/Log.h>
#include "common_log.h"
#else /* _ANDROID_ */
#define LOGI(...)      fprintf(stdout,__VA_ARGS__)
#define LOGE(...)      fprintf(stderr,__VA_ARGS__)
#define LOGV(...)      fprintf(stderr,__VA_ARGS__)
#define LOGD(...)      fprintf(stderr,__VA_ARGS__)
#endif /* _ANDROID_ */

#include "acdb.h"
#include "acph.h"
#include "acdb-loader.h"
#include "acdb-anc-general.h"
#include "acdb-anc-timpani.h"
#include "acdb-anc-tabla.h"
#include "acdb-id-mapper.h"
#include "acdb-loader-def.h"
#define LOG_NDDEBUG 0
#define LOG_TAG "ACDB-LOADER"

/* ACDB Controls */
#define VSS_NETWORK_ID_DEFAULT	0x00010037
#define VSS_NETWORK_ID_VOLTE_NB	0x00011324
#define VSS_NETWORK_ID_VOLTE_WB	0x00011325
#define VSS_NETWORK_ID_VOLTE_WV	0x00011326
#define VSS_NETWORK_ID_VOIP_NB	0x00011240
#define VSS_NETWORK_ID_VOIP_WB	0x00011241
#define VSS_NETWORK_ID_VOIP_WV	0x00011242
#define VSS_NETWORK_ID_CDMA_NB	0x00010021
#define VSS_NETWORK_ID_CDMA_WB	0x00010022
#define VSS_NETWORK_ID_CDMA_WV	0x00011100
#define VSS_NETWORK_ID_GSM_NB	0x00010023
#define VSS_NETWORK_ID_GSM_WB	0x00010024
#define VSS_NETWORK_ID_GSM_WV	0x00011101
#define VSS_NETWORK_ID_WCDMA_NB	0x00010025
#define VSS_NETWORK_ID_WCDMA_WB	0x00010026
#define VSS_NETWORK_ID_WCDMA_WV	0x00011102

#define INVALID_DATA	-1

#define TEMP_CAL_BUFSZ 1024 /* 1K should be plenty */

#define EC_REF_RX_DEVS (sizeof(uint32_t) * 20)

enum {
	RX_CAL,
	TX_CAL,
	MAX_AUDPROC_TYPES
};

static int		acdb_handle;
static int		ion_handle;
static int		alloc_handle;
static int		map_handle;
static uint8_t		*virt_cal_buffer = NULL;
static int		global_offset;
static bool		first_vol_table;

static unsigned int	audproc_ioctl[] = {AUDIO_SET_AUDPROC_RX_CAL,
						AUDIO_SET_AUDPROC_TX_CAL};
static unsigned int	audvol_ioctl[] = {AUDIO_SET_AUDPROC_RX_VOL_CAL,
						AUDIO_SET_AUDPROC_TX_VOL_CAL};
static unsigned int	audtop_ioctl[] = {AUDIO_SET_ADM_RX_TOPOLOGY,
						AUDIO_SET_ADM_TX_TOPOLOGY};
static unsigned int	afe_ioctl[] = {AUDIO_SET_AFE_RX_CAL,
						AUDIO_SET_AFE_TX_CAL};
static void send_asm_topology(void);
int acdb_loader_send_tabla_anc_cal(int acdb_id, int file_descriptor);

int acdb_loader_init_ACDB(void)
{
	int				result;
	struct ion_fd_data		fd_data;
	struct ion_allocation_data	alloc_data;

	LOGD("ACDB -> ACDB_CMD_INITIALIZE\n");
	result = acdb_ioctl(ACDB_CMD_INITIALIZE, NULL, 0, NULL, 0);
	if (result) {
		LOGE("Error initializing ACDB returned = %d\n", result);
		goto done;
	}

	result = acph_init();
	if (result) {
		LOGE("Error initializing ACPH returned = %d\n", result);
		goto done;
	}

	acdb_handle = open("/dev/msm_acdb", O_RDWR);
	if (acdb_handle < 0) {
		LOGE("Cannot open /dev/msm_acdb errno: %d\n", errno);
		goto done;
	}

	if (map_handle) {
		LOGD("ACDB -> MMAP MEM from ACDB driver\n");
		virt_cal_buffer = (uint8_t *)mmap(0, ACDB_BUFFER_SIZE,
			PROT_READ | PROT_WRITE,	MAP_SHARED, map_handle, 0);
	}


	if (virt_cal_buffer == 0) {
		LOGD("No existing ION info in ACDB driver\n");

		ion_handle = open("/dev/ion", O_RDONLY);
		if (ion_handle < 0) {
			LOGE("Cannot open /dev/ion errno: %d\n", ion_handle);
			goto err_acdb;
		}

		LOGD("ACDB -> ION_IOC_ALLOC\n");
		alloc_data.len = ACDB_BUFFER_SIZE;
		alloc_data.align = 0x1000;
#ifdef QCOM_AUDIO_USE_SYSTEM_HEAP_ID
		alloc_data.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
#else
		alloc_data.heap_id_mask = ION_HEAP(ION_AUDIO_HEAP_ID);
#endif
		alloc_data.flags = 0;
		result = ioctl(ion_handle, ION_IOC_ALLOC, &alloc_data);
		if (result) {
			LOGE("ION_ALLOC errno: %d\n", result);
			goto err_alloc;
		}

		LOGD("ACDB -> ION_IOC_SHARE\n");
		fd_data.handle = alloc_data.handle;
		alloc_handle = alloc_data.handle;
		result = ioctl(ion_handle, ION_IOC_SHARE, &fd_data);
		if (result) {
			LOGE("ION_IOC_SHARE errno: %d\n", result);
			goto err_share;
		}

		LOGD("ACDB -> MMAP ADDR\n");
		map_handle = fd_data.fd;
		virt_cal_buffer = (uint8_t *)mmap(0, ACDB_BUFFER_SIZE,
			PROT_READ | PROT_WRITE, MAP_SHARED, map_handle, 0);
		if (virt_cal_buffer == MAP_FAILED) {
			LOGE("Cannot allocate ION\n");
			goto err_map;
		}

		LOGD("ACDB -> register MEM to ACDB driver: 0x%x\n",
			 (uint32_t)virt_cal_buffer);

		result = ioctl(acdb_handle, AUDIO_REGISTER_PMEM, &map_handle);
		if (result < 0) {
			LOGE("Cannot register PMEM to ACDB driver\n");
			goto err_reg;
		}

	} else {
		LOGD("ACDB -> use MEM from ACDB driver: 0x%x\n", (uint32_t)virt_cal_buffer);
	}

	/* See ../inc/<target>/acdb-loader-def.h for cal block allocation */

	send_asm_topology();
	send_mbhc_data();
	send_tabla_anc_data();
	LOGD("ACDB -> init done!\n");
	return 0;

err_reg:
	munmap(virt_cal_buffer, ACDB_BUFFER_SIZE);
err_map:
	close(map_handle);
err_share:
	result = ioctl(ion_handle, ION_IOC_FREE, &alloc_handle);
	if (result)
		LOGE("ION_IOC_FREE errno: %d\n", result);
err_alloc:
	close(ion_handle);
err_acdb:
	close(acdb_handle);
done:
	return result;
}

static int get_audcal_path(uint32_t capability)
{
	int path;

	if (capability & MSM_SNDDEV_CAP_RX)
		path = RX_CAL;
	else if (capability & MSM_SNDDEV_CAP_TX)
		path = TX_CAL;
	else
		path = INVALID_DATA;

	return path;
}

static uint32_t get_samplerate(int  acdb_id)
{
	uint32_t sample_rate = ACDB_SAMPLERATE_48000Hz;

	if (((uint32_t)acdb_id == DEVICE_BT_SCO_RX_ACDB_ID) ||
		((uint32_t)acdb_id == DEVICE_BT_SCO_TX_ACDB_ID)) {
		sample_rate = ACDB_SAMPLERATE_8000Hz;
	} else if (((uint32_t)acdb_id == DEVICE_BT_SCO_RX_WB_ACDB_ID) ||
		((uint32_t)acdb_id == DEVICE_BT_SCO_TX_WB_ACDB_ID)) {
		sample_rate = ACDB_SAMPLERATE_16000Hz;
                /*To change to 16000HZ*/
        }

	return sample_rate;
}

static uint32_t get_adm_topology(int acdb_id)
{
	int32_t				result;
	AcdbGetAudProcTopIdCmdType	acdb_get_top;
	AcdbGetTopologyIdRspType	audio_top;

	acdb_get_top.nDeviceId = acdb_id;
	acdb_get_top.nApplicationType = ACDB_APPTYPE_GENERAL_PLAYBACK;

	LOGD("ACDB -> ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID\n");

	result = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID,
		(const uint8_t *)&acdb_get_top, sizeof(acdb_get_top),
		(uint8_t *)&audio_top, sizeof(audio_top));
	if (result) {
		LOGE("Error: ACDB get adm topology for acdb id = %d, returned = %d\n",
		     acdb_id, result);
		goto err;
	}
	return audio_top.nTopologyId;
err:
	return 0;
}

static uint32_t get_asm_topology(void)
{
	int32_t					result;
	AcdbGetAudProcStrmTopIdCmdType		acdb_get_top;
	AcdbGetTopologyIdRspType		audio_top;

	acdb_get_top.nApplicationType = ACDB_APPTYPE_GENERAL_PLAYBACK;

	LOGD("ACDB -> ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID\n");

	result = acdb_ioctl(ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID,
		(const uint8_t *)&acdb_get_top, sizeof(acdb_get_top),
		(uint8_t *)&audio_top, sizeof(audio_top));
	if (result) {
		LOGE("Error: ACDB get asm topology returned = %d\n",
		     result);
		goto err;
	}
	return audio_top.nTopologyId;
err:
	return 0;
}

static void get_audtable(int acdb_id, int path, struct cal_block *block)
{
	int32_t				result;
	AcdbAudProcTableCmdType		audtable;
	AcdbQueryResponseType		response;
	/* Point address to designated PMEM block */
	uint32_t			*addr =
		(uint32_t *)(virt_cal_buffer + (NUM_VOCPROC_BLOCKS + path)
		* ACDB_BLOCK_SIZE);

	audtable.nDeviceId = acdb_id;
	audtable.nDeviceSampleRateId = get_samplerate(acdb_id);
	audtable.nApplicationType = ACDB_APPTYPE_GENERAL_PLAYBACK;
	audtable.nBufferPointer = (uint8_t *)addr;

	/* Set larger buffer for TX due to Quad-mic */
	if (path)
		audtable.nBufferLength = MAX_BLOCK_SIZE;
	else
		audtable.nBufferLength = ACDB_BLOCK_SIZE;

	LOGD("ACDB -> ACDB_CMD_GET_AUDPROC_COMMON_TABLE\n");

	result = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_TABLE,
		(const uint8_t *)&audtable, sizeof(audtable),
		(uint8_t *)&response, sizeof(response));
	if (result) {
		LOGE("Error: ACDB audproc returned = %d\n", result);
		block->cal_size = 0;
		goto done;
	}

	block->cal_size = response.nBytesUsedInBuffer;
	block->cal_offset = (NUM_VOCPROC_BLOCKS + path) * ACDB_BLOCK_SIZE;
done:
	return;
}

static void get_audvoltable(int acdb_id, int path, struct cal_block *block)
{
	int32_t					result;
	AcdbAudProcGainDepVolTblStepCmdType	audvoltable;
	AcdbQueryResponseType			response;
	/* Point address to designated PMEM block */
	uint32_t				*addr =
		(uint32_t *)(virt_cal_buffer + (ACDB_AUDVOL_OFFSET + path)
		* ACDB_BLOCK_SIZE);

	audvoltable.nDeviceId = acdb_id;
	audvoltable.nApplicationType = ACDB_APPTYPE_GENERAL_PLAYBACK;
	/* 0 is max volume which is default Q6 COPP volume */
	audvoltable.nVolumeIndex = 0;
	audvoltable.nBufferLength = ACDB_BLOCK_SIZE;
	audvoltable.nBufferPointer = (uint8_t *)addr;

	LOGD("ACDB -> ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_COPP\n");

	result = acdb_ioctl(ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_COPP,
		(const uint8_t *)&audvoltable, sizeof(audvoltable),
		(uint8_t *)&response, sizeof(response));
	if (result) {
		LOGE("Error: ACDB AudProc vol returned = %d\n", result);
		block->cal_size = 0;
		goto done;
	}

	block->cal_size = response.nBytesUsedInBuffer;
	block->cal_offset = (ACDB_AUDVOL_OFFSET + path) * ACDB_BLOCK_SIZE;
done:
	return;
}

static void get_afetable(int acdb_id, int path, struct cal_block *block)
{
	int32_t					result;
	AcdbAfeCommonTableCmdType		afetable;
	AcdbQueryResponseType			response;
	/* Point address to designated PMEM block */
	uint32_t				*addr =
		(uint32_t *)(virt_cal_buffer + (ACDB_AFE_OFFSET + path)
		* ACDB_BLOCK_SIZE);

	afetable.nDeviceId = acdb_id;
	/* Does not accept ACDB sample rate bit mask */
	afetable.nSampleRateId = get_samplerate(acdb_id);
	afetable.nBufferLength = ACDB_BLOCK_SIZE;
	afetable.nBufferPointer = (uint8_t *)addr;

	LOGD("ACDB -> ACDB_CMD_GET_AFE_COMMON_TABLE\n");

	result = acdb_ioctl(ACDB_CMD_GET_AFE_COMMON_TABLE,
		(const uint8_t *)&afetable, sizeof(afetable),
		(uint8_t *)&response, sizeof(response));
	if (result) {
		LOGE("Error: ACDB AFE returned = %d\n", result);
		block->cal_size = 0;
		goto done;
	}

	block->cal_size = response.nBytesUsedInBuffer;
	block->cal_offset = (ACDB_AFE_OFFSET + path) * ACDB_BLOCK_SIZE;
done:
	return;
}

#ifdef ACDB_CMD_GET_AUDIO_RECORD_RX_DEVICE_LIST

int acdb_loader_get_ecrx_device(int acdb_id)
{
       int32_t result = INVALID_DATA;
       char *pRxDevs;
       AcdbAudioRecRxListCmdType acdb_cmd;
       AcdbAudioRecRxListRspType acdb_cmd_response;

       acdb_cmd.nTxDeviceId = acdb_id;
       pRxDevs = malloc(EC_REF_RX_DEVS);
       if (pRxDevs == NULL) {
           LOGE("Error: %s Malloc Failed", __func__);
           return result;
       }
       acdb_cmd_response.pRxDevs = pRxDevs;
       result = acdb_ioctl(ACDB_CMD_GET_AUDIO_RECORD_RX_DEVICE_LIST,
                          (const uint8_t *)&acdb_cmd, sizeof(acdb_cmd),
                 (uint8_t *)&acdb_cmd_response, sizeof(acdb_cmd_response));
       if (result) {
               LOGE("Error: ACDB EC_REF_RX returned = %d\n", result);
               goto done;
       }

       if (acdb_cmd_response.nNoOfRxDevs) {
            result = acdb_cmd_response.pRxDevs[0];
       }

done:
       free(pRxDevs);
       return result;
}

#else

int acdb_loader_get_ecrx_device(int acdb_id)
{
       return INVALID_DATA;
}

#endif

static void send_adm_topology(int acdb_id, int path)
{
	int32_t		result;
	uint32_t	topology;

	LOGD("ACDB -> send_adm_topology\n");

	topology = get_adm_topology(acdb_id);

	if (!topology)
		goto done;

	result = ioctl(acdb_handle, audtop_ioctl[path],
		(unsigned int)&topology);
	if (result)
		LOGE("Error: Sending ACDB adm topology result = %d\n", result);
done:
	return;
}

static void send_asm_topology(void)
{
	int32_t		result;
	uint32_t	topology;

	LOGD("ACDB -> send_asm_topology\n");

	topology = get_asm_topology();

	if (!topology)
		goto done;

	result = ioctl(acdb_handle, AUDIO_SET_ASM_TOPOLOGY,
		(unsigned int)&topology);
	if (result)
		LOGE("Error: Sending ACDB asm topology result = %d\n", result);
done:
	return;
}

static void send_audtable(int acdb_id, int path)
{
	int32_t				result;
	/* Size buffer to store one cal block (64B) */
	/* and its size as the first element */
	uint32_t			audproc_cal[3];
	LOGD("ACDB -> send_audtable\n");

	get_audtable(acdb_id, path, (struct cal_block *)&audproc_cal[1]);

	/* Set size of cal data sent */
	audproc_cal[0] = sizeof(struct cal_block);

	LOGD("ACDB -> AUDIO_SET_AUDPROC_CAL\n");

	result = ioctl(acdb_handle, audproc_ioctl[path],
		(unsigned int)&audproc_cal);
	if (result)
		LOGE("Error: Sending ACDB audproc result = %d\n", result);
done:
	return;
}

static void send_audvoltable(int acdb_id, int path)
{
	int32_t			result;
	/* Size buffer to store one cal block (64B) */
	/* and its size as the first element */
	uint32_t		audvol_cal[3];
	LOGD("ACDB -> send_audvoltable\n");

	get_audvoltable(acdb_id, path, (struct cal_block *)&audvol_cal[1]);

	/* Set size of cal data sent */
	audvol_cal[0] = sizeof(struct cal_block);

	LOGD("ACDB -> AUDIO_SET_AUDPROC_VOL_CAL\n");
	result = ioctl(acdb_handle, audvol_ioctl[path],
		(unsigned int)&audvol_cal);
	if (result)
		LOGE("Error: Sending ACDB audproc vol result = %d\n", result);
}

static int32_t valid_afe_cal(int acdb_id)
{
	int32_t	result = false;

	if ((acdb_id == DEVICE_HEADSET_TX_ACDB_ID) ||
		(acdb_id == DEVICE_SPEAKER_RX_ACDB_ID) ||
		(acdb_id == DEVICE_SPEAKER_MONO_RX_ACDB_ID))
		result = true;

	return result;
}

static void send_afe_cal(int acdb_id, int path)
{
	int32_t			result;
	/* Size buffer to store one cal block (64B) */
	/* and its size as the first element */
	uint32_t		afe_cal[3];

	/* Still send cal to clear out old cal */
	/* since afe cal is not for all devices */
	if (valid_afe_cal(acdb_id)) {
		LOGD("ACDB -> send_afe_cal\n");
		get_afetable(acdb_id, path, (struct cal_block *)&afe_cal[1]);
	} else {
		afe_cal[1] = 0;
		afe_cal[2] = 0;
	}

	/* Set size of cal data sent */
	afe_cal[0] = sizeof(struct cal_block);

	LOGD("ACDB -> AUDIO_SET_AFE_CAL\n");
	result = ioctl(acdb_handle, afe_ioctl[path],
		(unsigned int)&afe_cal);
	if (result)
		LOGE("Error: Sending ACDB AFE result = %d\n", result);
	return;
}

void acdb_loader_send_audio_cal(int acdb_id, int capability)
{
	int32_t		path;

	if (virt_cal_buffer == NULL) {
		LOGE("ACDB -> Not correctly initialized!\n");
		goto done;
	}

	path = get_audcal_path((uint32_t)capability);
	if (path == INVALID_DATA) {
		LOGE("ACDB -> Device is not RX or TX!"
			"acdb_id = %d\n", acdb_id);
		goto done;
	}

	LOGD("ACDB -> send_audio_cal, acdb_id = %d, path =  %d\n",
		acdb_id, path);

	send_adm_topology(acdb_id, path);
	send_audtable(acdb_id, path);
	send_audvoltable(acdb_id, path);
	/* If AFE cal supported send AFE cal*/
	if (NUM_AFE_BLOCKS)
		send_afe_cal(acdb_id, path);
done:
	return;
}

static int32_t get_anc_table(int acdb_id, struct cal_block *block, int timpani)
{
	int32_t result = -1;
	AcdbANCSettingCmdType acdb_cmd;
	AcdbQueryResponseType acdb_cmd_response;
	uint32_t			*addr =
		(uint32_t *)(virt_cal_buffer + (ACDB_ANC_OFFSET)
		* ACDB_BLOCK_SIZE);

	uint32_t acdb_command_id = ACDB_CMD_GET_ANC_SETTING;
	acdb_cmd.nRxDeviceId = acdb_id;
	acdb_cmd.nFrequencyId = 48000;
	acdb_cmd.nOversamplerateId = 256;
	if (timpani)
		acdb_cmd.nParamId = ACDB_PID_CODEC_DATA_TIMPANI;
	else
		acdb_cmd.nParamId = ACDB_PID_CODEC_DATA_TABLA;
	acdb_cmd.nBufferPointer = (uint8_t *)addr;
	acdb_cmd.nBufferLength = ACDB_BLOCK_SIZE;

	LOGD("ACDB -> ACDB_CMD_GET_ANC_SETTING\n");

	result = acdb_ioctl(acdb_command_id,
		(const uint8_t *)&acdb_cmd, sizeof(acdb_cmd),
		(uint8_t *)&acdb_cmd_response, sizeof(acdb_cmd_response));
	if (result) {
		LOGE("Error: ACDB ANC returned = %d\n", result);
		goto done;
	}
	block->cal_size = acdb_cmd_response.nBytesUsedInBuffer;
	block->cal_offset = (ACDB_ANC_OFFSET) * ACDB_BLOCK_SIZE;
done:
	return result;
}

#define ABS(x) (((x) < 0) ? (-1*(x)) : (x))
int32_t FP_mult(int32_t val1, int32_t val2)
{
	int32_t prod = 0;
	if ((val1 > 0 && val2 > 0) || (val1 < 0 && val2 < 0)) {
		if (ABS(val1) > (int32_t) (MAX_INT/ABS(val2)))
			prod = MAX_INT;
	}
	else if ((val1 > 0 && val2 < 0) || (val1 < 0 && val2 > 0)) {
		if (ABS(val1) > (int32_t) (MAX_INT/ABS(val2)))
			prod = -(int32_t) MAX_INT;
	}
	if (0 == prod)
		prod = val1 * val2;

	return prod;
}
int32_t FP_shift(int32_t val, int32_t shift)
{
	int32_t rnd = 1 << (ABS(shift)-1);
	int32_t val_s = val;
	/* underflow -> rounding errors */
	if (shift < 0) {
		val_s = ABS(val_s) + rnd;
		val_s = val_s >> ABS(shift);
		val_s = (val > 0) ? val_s : -val_s;
	}
	/* overflow -> saturation */
	else if (shift > 0) {
		if (ABS(val) > (int32_t) ((MAX_INT >> ABS(shift)))) {
			if (val < 0)
				val_s = -(int32_t) MAX_INT;
			else
				val_s = (int32_t) MAX_INT;
		} else
			val_s = val << ABS(shift);
	}
	return val_s;
}

uint16_t twosComp(int16_t val, int16_t bits)
{
	uint16_t res = 0;
	uint32_t width = bits + 1;
	if (val >= 0)
		res = (uint16_t) val;
	else
		res = -((-val) - (1 << width));

	return res;
}

int32_t FP_format(int32_t val, int32_t intb, int32_t fracb, int32_t max_val)
{
	val = FP_shift(val, -(ANC_COEFF_FRAC_BITS - fracb));
	/* Check for saturation */
	if (val > max_val)
		val = max_val;
	else if (val < -max_val)
		val = -max_val;
	/* convert to 2s compl */
	val = twosComp((uint16_t) val, (uint16_t) (intb + fracb));
	return val;
}

int SetANC_Shift(uint32_t *anc_config, uint32_t *anc_index,
	struct adie_codec_db_anc_cfg *pANCCfg, uint32_t ancCh)
{
	uint8_t val = 0;

	val = TIMPANI_CDC_ANC1_FF_FB_SHIFT_ANC1_FB_LPF_SHIFT_M &
		(pANCCfg->iANC_FB_Shift <<
		TIMPANI_CDC_ANC1_FF_FB_SHIFT_ANC1_FB_LPF_SHIFT_S);
	val |= TIMPANI_CDC_ANC1_FF_FB_SHIFT_ANC1_FF_LPF_SHIFT_M &
		pANCCfg->iANC_FF_Shift;
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
		TIMPANI_CDC_ANC_LPF_GAIN, TIMPANI_CDC_ANC1_FF_FB_SHIFT_M, val);

	return 0;
}

int SetANC_Scale(uint32_t *anc_config, uint32_t *anc_index,
	struct adie_codec_db_anc_cfg *pANCCfg, uint32_t ancCh)
{
	int res = 0;
	uint8_t val = 0;
	uint32_t index = 0;
	uint32_t iter = 0;
	uint32_t numFreqs = sizeof(ancFreqTable)/sizeof(struct
		adie_codec_anc_freq_setting);
	uint32_t ancFreq = (ANC_OSR * ANC_FREQ) / 1000;
	int freqFound = 0;

	/* Set scale ptr to first coeff */
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY
	(ancRegAddr[ancCh].TIMPANI_CDC_ANC_SCALE_PTR,
	TIMPANI_CDC_ANC1_SCALE_PTR_ANC1_SCALE_PTR_M, 0);
	/* Find frequency index */
	/* Frequency selection */
	while ((iter <  numFreqs) && !freqFound) {
		if (ancFreq <= (uint32_t) ancFreqTable[iter].freq) {
			freqFound = 1;
			index = iter;
		}
		iter++;
	}
	/* Write scale values to ANC0 or ANC1 */
	/* Scale A */
	val = (uint8_t) (TIMPANI_CDC_ANC1_SCALE_ANC1_SCALE_M &
		ancFreqTable[index].scaleA);
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
		TIMPANI_CDC_ANC_SCALE, 0xFF, val);
	/* Scale B MSBs */
	val = (uint8_t) (TIMPANI_CDC_ANC1_SCALE_ANC1_SCALE_M &
		(ancFreqTable[index].scaleB >> REGISTER_DEPTH));
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
		TIMPANI_CDC_ANC_SCALE, 0xFF, val);
	/* Scale B LSBs */
	val = (uint8_t) (TIMPANI_CDC_ANC1_SCALE_ANC1_SCALE_M &
		ancFreqTable[index].scaleB);
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
		TIMPANI_CDC_ANC_SCALE, 0xFF, val);
	/* Scale C */
	val = (uint8_t) (TIMPANI_CDC_ANC1_SCALE_ANC1_SCALE_M &
		ancFreqTable[index].scaleC);
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
		TIMPANI_CDC_ANC_SCALE, 0xFF, val);
	/* Scale D MSBs */
	val = (uint8_t) (TIMPANI_CDC_ANC1_SCALE_ANC1_SCALE_M &
		(ancFreqTable[index].scaleD >> REGISTER_DEPTH));
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
		TIMPANI_CDC_ANC_SCALE, 0xFF, val);
	/* Scale D LSBs */
	val = (uint8_t) (TIMPANI_CDC_ANC1_SCALE_ANC1_SCALE_M &
		ancFreqTable[index].scaleD);
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
		TIMPANI_CDC_ANC_SCALE, 0xFF, val);
	return res;
}

int SetANC_LPFCoeffs(uint32_t *anc_config, uint32_t *anc_index,
	struct adie_codec_db_anc_cfg *pANCCfg, uint32_t ancCh)
{
	int res = 0;
	int32_t coeff = 0;
	uint32_t u_coeff = 0;
	uint8_t valMSBs = 0;
	uint8_t valLSBs = 0;
	uint32_t iter = 0;
	/* Set coeff ptr to first coeff */
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY(
		ancRegAddr[ancCh].TIMPANI_CDC_ANC_LPF_PTR,
		TIMPANI_CDC_ANC1_LPF_COEFF_PTR_ANC1_LPF_COEFF_PTR_M, 0);
	/* Write LPF coeffs to ANC0 or ANC1 */
	/* Write FF coeffs */
	for (iter = 0; iter < ANC_NUM_LPF_FF_B_COEFFS + ANC_NUM_LPF_FF_A_COEFFS;
	iter++) {
		coeff = pANCCfg->iANC_FF_LPFCoeff[iter];
		u_coeff = FP_format(coeff, ANC_LPF_COEFFS_REG_INT_BITS,
		ANC_LPF_COEFFS_REG_FRAC_BITS, ANC_LPF_COEFFS_MAX_VAL);
		valMSBs = (uint8_t)
		(TIMPANI_CDC_ANC1_LPF_COEFF_MSB_ANC1_LPF_COEFF_MSB_M & (u_coeff >>
			REGISTER_DEPTH));
		valLSBs = (uint8_t)
		(TIMPANI_CDC_ANC1_LPF_COEFF_LSB_ANC1_LPF_COEFF_LSB_M & u_coeff);
		anc_config[(*anc_index)++] =
			ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
			TIMPANI_CDC_ANC_LPF_MSB, 0xFF, valMSBs);
		anc_config[(*anc_index)++] =
			ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
			TIMPANI_CDC_ANC_LPF_LSB, 0xFF, valLSBs);
	}
	/* Write FB coeffs */
	for (iter = 0; iter < ANC_NUM_LPF_FB_B_COEFFS + ANC_NUM_LPF_FB_A_COEFFS;
	iter++) {
		coeff = pANCCfg->iANC_FB_LPFCoeff[iter];
		u_coeff = FP_format(coeff, ANC_LPF_COEFFS_REG_INT_BITS,
			ANC_LPF_COEFFS_REG_FRAC_BITS, ANC_LPF_COEFFS_MAX_VAL);
		valMSBs = (uint8_t)
		(TIMPANI_CDC_ANC1_IIR_COEFF_MSB_ANC1_IIR_COEFF_MSB_M & (u_coeff
			>> REGISTER_DEPTH));
		valLSBs = (uint8_t)
		(TIMPANI_CDC_ANC1_IIR_COEFF_LSB_ANC1_IIR_COEFF_LSB_M & u_coeff);
		anc_config[(*anc_index)++] =
			ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
			TIMPANI_CDC_ANC_LPF_MSB, 0xFF, valMSBs);
		anc_config[(*anc_index)++] =
			ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
			TIMPANI_CDC_ANC_LPF_LSB, 0xFF, valLSBs);
	}
	return res;
}
int SetANC_IIRCoeffs(uint32_t *anc_config, uint32_t *anc_index,
	struct adie_codec_db_anc_cfg *pANCCfg, uint32_t ancCh)
{
	int res = 0;
	int32_t coeff = 0;
	uint32_t u_coeff = 0;
	uint8_t valMSBs = 0;
	uint8_t valLSBs = 0;
	uint32_t iter = 0;
	uint8_t gainIndex = (uint8_t) (pANCCfg->ancChnlConfig[ancCh].iANC_Gain
		+ ANC_GAIN_TABLE_OFFSET);
	/* Set coeff ptr to first coeff */
	anc_config[(*anc_index)++] = ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
	TIMPANI_CDC_ANC_IIR_PTR,
	TIMPANI_CDC_ANC1_IIR_COEFF_PTR_ANC1_IIR_COEFF_PTR_M, 0);
	/* Write IIR coeffs to ANC0 or ANC1 */
	/* Write FF coeffs */
	for (iter = 0; iter < ANC_NUM_IIR_FF_B_COEFFS +
		ANC_NUM_IIR_FF_A_COEFFS; iter++) {
		coeff = pANCCfg->iANC_FFCoeff[iter];
		if (iter < ANC_NUM_IIR_FF_B_COEFFS) {
			coeff = FP_mult(coeff,
				ancGainTable[gainIndex].multiplier);
			coeff = FP_shift(coeff, -ANC_GAIN_TABLE_FRAC_BITS);
		}
		u_coeff = FP_format(coeff, ANC_IIR_COEFFS_REG_INT_BITS,
			ANC_IIR_COEFFS_REG_FRAC_BITS, ANC_IIR_COEFFS_MAX_VAL);
		valMSBs = (uint8_t)
			(TIMPANI_CDC_ANC1_IIR_COEFF_MSB_ANC1_IIR_COEFF_MSB_M &
			(u_coeff >> REGISTER_DEPTH));
		valLSBs = (uint8_t)
		(TIMPANI_CDC_ANC1_IIR_COEFF_LSB_ANC1_IIR_COEFF_LSB_M & u_coeff);
		anc_config[(*anc_index)++] =
			ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
			TIMPANI_CDC_ANC_IIR_MSB, 0xFF, valMSBs);
		anc_config[(*anc_index)++] =
			ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
			TIMPANI_CDC_ANC_IIR_LSB, 0xFF, valLSBs);
	}
	/* Write FB coeffs */
	for (iter = 0; iter < ANC_NUM_IIR_FB_B_COEFFS + ANC_NUM_IIR_FB_A_COEFFS;
	iter++) {
		coeff = pANCCfg->iANC_FBCoeff[iter];
		if (iter < ANC_NUM_IIR_FB_B_COEFFS) {
			coeff = FP_mult(coeff,
				ancGainTable[gainIndex].multiplier);
			coeff = FP_shift(coeff, -ANC_GAIN_TABLE_FRAC_BITS);
		}
		u_coeff = FP_format(coeff, ANC_IIR_COEFFS_REG_INT_BITS,
			ANC_IIR_COEFFS_REG_FRAC_BITS, ANC_IIR_COEFFS_MAX_VAL);
		valMSBs = (uint8_t)
			(TIMPANI_CDC_ANC1_IIR_COEFF_MSB_ANC1_IIR_COEFF_MSB_M &
			(u_coeff >> REGISTER_DEPTH));
		valLSBs = (uint8_t)
			(TIMPANI_CDC_ANC1_IIR_COEFF_LSB_ANC1_IIR_COEFF_LSB_M &
			u_coeff);
		anc_config[(*anc_index)++] =
			ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
			TIMPANI_CDC_ANC_IIR_MSB, 0xFF, valMSBs);
		anc_config[(*anc_index)++] =
			ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].
			TIMPANI_CDC_ANC_IIR_LSB, 0xFF, valLSBs);
	}
	return res;
}
int SetANCControlSettings(uint32_t *anc_config, uint32_t *anc_index,
	struct adie_codec_db_anc_cfg *pANCCfg, uint32_t ancCh,
	bool ancDMICselect)
{
	uint8_t mask1 = 0;
	uint8_t val1 = 0;
	uint8_t mask2 = 0;
	uint8_t val2 = 0;
	int freqFound = 0;
	uint32_t numFreqs = sizeof(ancFreqTable)/
		sizeof(struct adie_codec_anc_freq_setting);
	uint32_t ancFreq = (ANC_OSR * ANC_FREQ) / 1000;
	uint32_t iter = 0;

	/* Enable/disable feedback mode */
	mask1 |= TIMPANI_CDC_ANC1_CTL1_ANC1_FB_EN_M;
	val1 |= ((true == pANCCfg->bANC_FeedBackEnbl) ? 1 : 0) <<
		TIMPANI_CDC_ANC1_CTL1_ANC1_FB_EN_S;

	/* Set DMIC/ADC switch */
	mask1 |= TIMPANI_CDC_ANC1_CTL1_ANC1_ADC_DMIC_SEL_M;
	val1 |= (ancDMICselect ? 1 : 0) <<
		TIMPANI_CDC_ANC1_CTL1_ANC1_ADC_DMIC_SEL_S;

	/* Enable/disable LR mixing */
	mask1 |= TIMPANI_CDC_ANC1_CTL1_ANC1_LR_EN_M;
	if ((true == pANCCfg->bANC_LRMixEnbl) &&
	(ancCh == pANCCfg->eANC_LRMixOutputChannel)) {
		val1 |= 1 << TIMPANI_CDC_ANC1_CTL1_ANC1_LR_EN_S;
	}

	/* Frequency selection */
	mask2 = TIMPANI_CDC_ANC1_CTL2_ANC1_FREQ_SEL_M;
	iter = 0;
	while ((iter <  numFreqs) && !freqFound) {
		if (ancFreq <= (uint32_t) ancFreqTable[iter].freq) {
			freqFound = 1;
			val2 = (uint8_t) (ancFreqTable[iter].setting <<
			TIMPANI_CDC_ANC1_CTL2_ANC1_FREQ_SEL_S);
		}
		iter++;
	}
	anc_config[(*anc_index)++] =
	ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].TIMPANI_CDC_ANC_CTL1, mask1,
		val1);
	anc_config[(*anc_index)++] =
	ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].TIMPANI_CDC_ANC_CTL2, mask2,
		val2);

	return 0;
}
void SetANCInputSettings(uint32_t *anc_config, uint32_t *anc_index,
	struct adie_codec_db_anc_cfg *pANCCfg, uint32_t ancCh,
	bool * ancDMICselect)
{
	enum adie_codec_device_port_id ancInputDevice =
		pANCCfg->ancChnlConfig[ancCh].eANC_InputDevice;
	uint8_t mask = 0;
	uint8_t val = 0;
	uint8_t shift_DMICsel = 0;
	uint8_t shift_ANCchn = 0;
	*ancDMICselect = false;

	if (ADIE_CODEC_DEV_PORT_ID_NONE != ancInputDevice) {
		mask = mask | TIMPANI_CDC_ANC_INPUT_MUX_ANC1_DMIC_SEL_M;
		switch (ancInputDevice) {
		case ADIE_CODEC_DEV_PORT_ID_DMICA:
		case ADIE_CODEC_DEV_PORT_ID_DMICB:
			shift_DMICsel =
				TIMPANI_CDC_ANC_INPUT_MUX_ANC1_DMIC_SEL_S;
			val = 0x0;
			*ancDMICselect = true;
			break;
		case ADIE_CODEC_DEV_PORT_ID_AMIC1:
		case ADIE_CODEC_DEV_PORT_ID_AMIC2:
		case ADIE_CODEC_DEV_PORT_ID_TX_AUX:
		case ADIE_CODEC_DEV_PORT_ID_LINE_IN:
			shift_DMICsel =
				TIMPANI_CDC_ANC_INPUT_MUX_ANC1_ADC_SEL_S;
			val = 0x2;
			*ancDMICselect = false;
			break;
		default :
			break;
		}
	}

	if (ADIE_CODEC_ANC_CHANNEL_1 == ancCh) {
	/* ANC 1 - default connected to Left input - unless swap enabled */
		shift_ANCchn = TIMPANI_CDC_ANC_INPUT_MUX_ANC1_DMIC_SEL_S;
		val |= (false == pANCCfg->ancChnlConfig[ancCh].
			bANC_InputLRSwap) ? 0 : 1;
	} else {
		/* ANC 2 - default connected to Right input - unless swap enabled */
		shift_ANCchn = TIMPANI_CDC_ANC_INPUT_MUX_ANC2_DMIC_SEL_S;
		val |= (false == pANCCfg->ancChnlConfig[ancCh].
			bANC_InputLRSwap) ? 1 : 0;
	}

	/* Shift mask and val according to DMIC/ADC selection */
	mask = mask << shift_DMICsel;
	val = val << shift_DMICsel;
	/* Shift mask and val according to ANC channel selection */
	mask = mask << shift_ANCchn;
	val = val << shift_ANCchn;

	anc_config[(*anc_index)++] =
	ADIE_CODEC_PACK_ENTRY(TIMPANI_A_CDC_ANC_INPUT_MUX, mask, val);
}
int convert_anc_data_to_timpani(struct adie_codec_db_anc_cfg *pANCCfg)
{
	uint32_t index;
	uint32_t reg, mask, val;
	uint32_t anc_index = 0;
	uint32_t ancCh;
	bool ancDMICselect;
	struct storage_adie_codec_anc_data anc_config;

        memset(&anc_config,0,sizeof(anc_config));
	for(ancCh = 0; ancCh < NUM_ANC_COMPONENTS; ancCh++) {
		/* Disable ANC */
		anc_config.writes[anc_index++] = ADIE_CODEC_PACK_ENTRY
		(ancRegAddr[ancCh].TIMPANI_CDC_ANC_CTL1,
		TIMPANI_CDC_ANC1_CTL1_ANC1_EN_M, 0x00);
		/* Program MUXES */
		/* Setup ANC Input MUX */
		SetANCInputSettings((uint32_t *)anc_config.writes, &anc_index,
		pANCCfg, ancCh, &ancDMICselect);
		/* Setup ANC Control registers */
		SetANCControlSettings((uint32_t *)anc_config.writes,&anc_index,
		pANCCfg, ancCh, ancDMICselect);
		/* Program Coefficents */
		SetANC_IIRCoeffs((uint32_t *)anc_config.writes, &anc_index,
		pANCCfg, ancCh);
		SetANC_LPFCoeffs((uint32_t *)anc_config.writes, &anc_index,
		pANCCfg, ancCh);
		/* Program scale and shift values */
		SetANC_Scale((uint32_t *)anc_config.writes, &anc_index, pANCCfg,
			ancCh);
		SetANC_Shift((uint32_t *)anc_config.writes, &anc_index, pANCCfg,
			ancCh);
		/* Soft Reset */
		anc_config.writes[anc_index++] =
		ADIE_CODEC_PACK_ENTRY(ancRegAddr[ancCh].TIMPANI_CDC_ANC_CTL1,
		TIMPANI_CDC_ANC1_CTL1_ANC1_SOFT_RESET_M,
		TIMPANI_CDC_ANC1_CTL1_ANC1_SOFT_RESET_ANC_RESET);
		/* Take out of reset */
		anc_config.writes[anc_index++] = ADIE_CODEC_PACK_ENTRY
		(ancRegAddr[ancCh].TIMPANI_CDC_ANC_CTL1,
		TIMPANI_CDC_ANC1_CTL1_ANC1_SOFT_RESET_M,
		TIMPANI_CDC_ANC1_CTL1_ANC1_SOFT_RESET_ANC_ACTIVE);

		/* Enable ANC */
		anc_config.writes[anc_index++] = ADIE_CODEC_PACK_ENTRY
		(ancRegAddr[ancCh].TIMPANI_CDC_ANC_CTL1,
			TIMPANI_CDC_ANC1_CTL1_ANC1_EN_M, 0xFF);
	}
	LOGE("done with conversion, anc config size is %d\n", anc_index);
	anc_config.size = anc_index;
	memset(pANCCfg, 0, ACDB_BLOCK_SIZE);
	memcpy(pANCCfg, &anc_config, sizeof(anc_config));

	return anc_index;
}


int acdb_loader_send_anc_cal(int acdb_id)
{
	int32_t			 result;
	int			     index;
	uint32_t			anc_cal[3];
	struct cal_block		*block;
	struct adie_codec_db_anc_cfg *ancCfg;
	uint32_t			*addr;

	block = (struct cal_block *)&anc_cal[1];

	if (!block) {
		LOGE("Error retrieving calibration block\n");
		return -EPERM;
	}
	result = get_anc_table(acdb_id, block, 1);
	if (result) {
		return result;
	}
	addr = (uint32_t *)(virt_cal_buffer + block->cal_offset);
	ancCfg = (struct adie_codec_db_anc_cfg *)(addr);

	convert_anc_data_to_timpani(ancCfg);

	/* Set size of Cal buffers */
	anc_cal[0] = sizeof(struct cal_block);

	LOGD("ACDB -> AUDIO_SET_ANC_CAL\n");
	result = ioctl(acdb_handle, AUDIO_SET_ANC_CAL,
		(unsigned int)&anc_cal);
	if (result)
		LOGE("Error: Sending ACDB anc result = %d\n", result);
done:
	return 0;
}

#ifdef _ANDROID_
static mode_t ALLPERMS = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
#endif

void send_mbhc_data(void)
{
	int mbhc_fd, result;
	AcdbGblTblCmdType global_cmd;
	AcdbQueryResponseType   response;
	uint8_t *calbuf;

	LOGD("send mbhc data\n");

	mbhc_fd = creat(MBHC_BIN_PATH, ALLPERMS);

	if (mbhc_fd < 0)
	{
		LOGE("Error opening MBHC file %d\n", errno);
		return;
	}

	calbuf = malloc(TEMP_CAL_BUFSZ);
	if (calbuf == NULL) {
		LOGE("Fail to allocate memory for button detection calibration\n");
		goto close_fd;
	}

	global_cmd.nModuleId = ACDB_MID_MBHC;
	global_cmd.nParamId = ACDB_PID_GENERAL_CONFIG;
	global_cmd.nBufferLength = TEMP_CAL_BUFSZ;
	global_cmd.nBufferPointer = calbuf;

	result = acdb_ioctl(ACDB_CMD_GET_GLBTBL_DATA,
		(const uint8_t *) &global_cmd, sizeof(global_cmd),
		(uint8_t *) &response, sizeof(response));

	if (result)
	{
		LOGE("Error reading MBHC general config returned = %x\n", result);
		goto acdb_error;
	}

	result = write(mbhc_fd, calbuf, response.nBytesUsedInBuffer);
	if ( result != response.nBytesUsedInBuffer) {
		LOGE("Error writing MBHC calibration data\n", result);
		goto acdb_error;
	}

	global_cmd.nParamId = ACDB_PID_PLUG_REMOVAL_DETECTION;

	result = acdb_ioctl(ACDB_CMD_GET_GLBTBL_DATA,
		(const uint8_t *) &global_cmd, sizeof(global_cmd),
		(uint8_t *) &response, sizeof(response));

	if (result)
	{
		LOGE("Error reading MBHC removal config returned = %x\n", result);
		goto acdb_error;
	}

	result = write(mbhc_fd, calbuf, response.nBytesUsedInBuffer);
	if ( result != response.nBytesUsedInBuffer) {
		LOGE("Error writing MBHC calibration data\n", result);
		goto acdb_error;
	}

	global_cmd.nParamId = ACDB_PID_PLUG_TYPE_DETECTION;

	result = acdb_ioctl(ACDB_CMD_GET_GLBTBL_DATA,
		(const uint8_t *) &global_cmd, sizeof(global_cmd),
		(uint8_t *) &response, sizeof(response));

	if (result)
	{
		LOGE("Error reading MBHC plug type config returned = %x\n", result);
		goto acdb_error;
	}

	result = write(mbhc_fd, calbuf, response.nBytesUsedInBuffer);
	if ( result != response.nBytesUsedInBuffer) {
		LOGE("Error writing MBHC calibration data\n", result);
		goto acdb_error;
	}
	global_cmd.nParamId = ACDB_PID_BUTTON_PRESS_DETECTION;

	result = acdb_ioctl(ACDB_CMD_GET_GLBTBL_DATA,
		(const uint8_t *) &global_cmd, sizeof(global_cmd),
		(uint8_t *) &response, sizeof(response));

	if (result)
	{
		LOGE("Error reading MBHC button press config returned = %x\n", result);
		goto acdb_error;
	}

	result = write(mbhc_fd, calbuf, response.nBytesUsedInBuffer);
	if ( result != response.nBytesUsedInBuffer) {
		LOGE("Error writing MBHC calibration data\n", result);
		goto acdb_error;
	}

	global_cmd.nParamId = ACDB_PID_IMPEDANCE_DETECTION;

	result = acdb_ioctl(ACDB_CMD_GET_GLBTBL_DATA,
		(const uint8_t *) &global_cmd, sizeof(global_cmd),
		(uint8_t *) &response, sizeof(response));

	if (result)
	{
		LOGE("Error reading MBHC impedance config returned = %x\n", result);
		goto acdb_error;
	}

	result = write(mbhc_fd, calbuf, response.nBytesUsedInBuffer);
	if ( result != response.nBytesUsedInBuffer) {
		LOGE("Error writing MBHC calibration data\n", result);
		goto acdb_error;
	}

	fsync(mbhc_fd);
	free(calbuf);
	close(mbhc_fd);
	return;

acdb_error:
	free(calbuf);
close_fd:
	close(mbhc_fd);
	unlink(MBHC_BIN_PATH);
}


void send_tabla_anc_data(void)
{
	uint32_t anc_configurations = 6;
	uint32_t anc_base_configuration = 26;
	uint32_t anc_reserved[3] = {0, 0, 0};
	int i;
	LOGD("send tabla anc data\n");
	int result = creat (WCD9310_ANC_BIN_PATH, ALLPERMS);
	if (result < 0)
	{
		LOGE("Error opening anc file %d\n", errno);
		return;
	}

	write (result, anc_reserved, sizeof(uint32_t) * 3);
	write (result, &anc_configurations, sizeof(uint32_t));

	for (i = 0; i < (int32_t)anc_configurations; i++) {
		acdb_loader_send_tabla_anc_cal(i + anc_base_configuration, result);
	}
	close(result);
}

int SetTablaANC_IIRCoeffs(uint32_t *anc_config, uint32_t *anc_index,
	struct adie_codec_tabla_db_anc_cfg *pANCCfg, uint32_t ancCh)
{
	int res = 0;
	int32_t coeff = 0;
	uint32_t u_coeff = 0;
	uint8_t valMSBs = 0;
	uint8_t valLSBs = 0;
	uint32_t iter = 0;
	uint32_t offset = ancCh * 128;
	uint8_t gainIndex = (uint8_t) (pANCCfg[ancCh].anc_gain + ANC_GAIN_TABLE_OFFSET);

	/* Set coeff ptr to first coeff */
	anc_config[(*anc_index)++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT1_B2_CTL + offset), 0xFF, 0x00);

	/* Write FF coeffs */
	for (iter = 0; iter < TABLA_ANC_NUM_IIR_FF_A_COEFFS + TABLA_ANC_NUM_IIR_FF_B_COEFFS; iter++) {

		coeff = pANCCfg->anc_ff_coeff[iter];
		if (iter >= TABLA_ANC_NUM_IIR_FF_B_COEFFS) {
			coeff = FP_mult(coeff, ancGainTable[gainIndex].multiplier);
			coeff = FP_shift(coeff, -ANC_GAIN_TABLE_FRAC_BITS);
		}
		u_coeff = FP_format(coeff, ANC_IIR_COEFFS_REG_INT_BITS, ANC_IIR_COEFFS_REG_FRAC_BITS, ANC_IIR_COEFFS_MAX_VAL);
		valMSBs = (uint8_t) (0x01 & (u_coeff >> REGISTER_DEPTH));
		valLSBs = (uint8_t) (0xFF & u_coeff);
		anc_config[(*anc_index)++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT1_B3_CTL + offset), 0xFF, valMSBs);
		anc_config[(*anc_index)++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT1_B4_CTL + offset), 0xFF, valLSBs);
	}

	/* Write FB coeffs */
	for (iter = 0; iter < TABLA_ANC_NUM_IIR_FB_A_COEFFS + TABLA_ANC_NUM_IIR_FB_B_COEFFS; iter++) {
		coeff = pANCCfg->anc_fb_coeff[iter];
		if (iter >= TABLA_ANC_NUM_IIR_FB_A_COEFFS) {
			coeff = FP_mult(coeff, ancGainTable[gainIndex].multiplier);
			coeff = FP_shift(coeff, -ANC_GAIN_TABLE_FRAC_BITS);
		}
		u_coeff = FP_format(coeff, ANC_IIR_COEFFS_REG_INT_BITS, ANC_IIR_COEFFS_REG_FRAC_BITS, ANC_IIR_COEFFS_MAX_VAL);
		valMSBs = (uint8_t) (0xFF & (u_coeff >> REGISTER_DEPTH));
		valLSBs = (uint8_t) (0xFF & u_coeff);
		anc_config[(*anc_index)++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT1_B3_CTL + offset), 0xFF, valMSBs);
		anc_config[(*anc_index)++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT1_B4_CTL + offset), 0xFF, valLSBs);
	}
	return res;
}
int SetTablaANC_LPFCoeffs(uint32_t *anc_config, uint32_t *anc_index,
	struct adie_codec_tabla_db_anc_cfg *pANCCfg, uint32_t ancCh)
{
	int res = 0;
	int32_t coeff = 0;
	uint32_t u_coeff = 0;
	uint8_t valMSBs = 0;
	uint8_t valLSBs = 0;
	uint32_t iter = 0;
	uint32_t offset = ancCh * 128;

	/* Set coeff ptr to first coeff */
	anc_config[(*anc_index)++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT2_B1_CTL + offset), 0xFF, 0x00);

	/* Write FF coeffs */
	for (iter = 0; iter < TABLA_ANC_NUM_LPF_FF_A_COEFFS + TABLA_ANC_NUM_LPF_FF_B_COEFFS; iter++) {
		coeff = pANCCfg[ancCh].anc_ff_lpf_coeff[iter];
		u_coeff = FP_format(coeff, ANC_LPF_COEFFS_REG_INT_BITS, ANC_LPF_COEFFS_REG_FRAC_BITS, ANC_LPF_COEFFS_MAX_VAL);
		valMSBs = (uint8_t) (0x0F & (u_coeff >> REGISTER_DEPTH));
		valLSBs = (uint8_t) (0xFF & u_coeff);
		anc_config[(*anc_index)++] =  TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT2_B2_CTL + offset), 0xFF, valMSBs);
		anc_config[(*anc_index)++] =  TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT2_B3_CTL + offset), 0xFF, valLSBs);
	}
	/* Write FB coeffs */
	for (iter = 0; iter < TABLA_ANC_NUM_LPF_FB_B_COEFFS + TABLA_ANC_NUM_LPF_FB_A_COEFFS;
	iter++) {
		coeff = pANCCfg[ancCh].anc_fb_lpf_coeff[iter];
		u_coeff = FP_format(coeff, ANC_LPF_COEFFS_REG_INT_BITS, ANC_LPF_COEFFS_REG_FRAC_BITS, ANC_LPF_COEFFS_MAX_VAL);
		valMSBs = (uint8_t) (0x0F & (u_coeff >> REGISTER_DEPTH));
		valLSBs = (uint8_t) (0xFF & u_coeff);
		anc_config[(*anc_index)++] =  TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT2_B2_CTL + offset), 0xFF, valMSBs);
		anc_config[(*anc_index)++] =  TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT2_B3_CTL + offset), 0xFF, valLSBs);
	}

	return res;
}

int convert_anc_data_to_tabla(struct adie_codec_tabla_db_anc_cfg *pANCCfg, int file_descriptor)
{
	uint32_t index;
	uint32_t reg, mask, val;
	uint32_t temp_ctl_reg_val;
	uint32_t anc_index = 0;
	int j;
	uint32_t offset;
	uint32_t ancCh;
	bool ancDMICselect;
	struct storage_adie_codec_anc_data anc_config;

	for(ancCh = 0; ancCh < NUM_ANC_COMPONENTS; ancCh++) {
		if (!pANCCfg[ancCh].input_device)
			continue;

		offset = ancCh * 128;

		anc_config.writes[anc_index++] = TABLA_CODEC_PACK_ENTRY(TABLA_A_CDC_CLK_ANC_RESET_CTL, ancCh ? 0xC : 0x3, ancCh ? 0xC : 0x3);

		temp_ctl_reg_val = 0;
		if (pANCCfg[ancCh].ff_out_enable)
			temp_ctl_reg_val |= 0x1;
		if ((pANCCfg[ancCh].input_device & 0xF) >= ADIE_CODEC_DMIC1)
			temp_ctl_reg_val |= 0x2;
		if (pANCCfg[ancCh].anc_lr_mix_enable)
			temp_ctl_reg_val |= 0x4;
		if (pANCCfg[ancCh].hybrid_enable)
			temp_ctl_reg_val |= 0x8;
		if (pANCCfg[ancCh].ff_in_enable)
			temp_ctl_reg_val |= 0x10;
		if (pANCCfg[ancCh].dcflt_enable)
			temp_ctl_reg_val |= 0x20;
		if (pANCCfg[ancCh].smlpf_enable)
			temp_ctl_reg_val |= 0x40;

		anc_config.writes[anc_index++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_CTL + offset), 0xFF, temp_ctl_reg_val);

		anc_config.writes[anc_index++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_SHIFT + offset), 0xFF, (pANCCfg[ancCh].anc_ff_shift << 4) | pANCCfg[ancCh].anc_fb_shift);

		/* IIR COEFFS */
		SetTablaANC_IIRCoeffs((uint32_t *)anc_config.writes, &anc_index, pANCCfg, ancCh);

		anc_config.writes[anc_index++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT1_B1_CTL + offset), 0x01, pANCCfg[ancCh].adaptive ? 1 : 0);

		/* LPF COEFFS */
		SetTablaANC_LPFCoeffs((uint32_t *)anc_config.writes, &anc_index, pANCCfg, ancCh);

		/* ANC SMLPF CTL */
		anc_config.writes[anc_index++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT3_CTL + offset), 0xFF, pANCCfg[ancCh].smlpf_shift);
		/* ANC DCFLT CTL */
		anc_config.writes[anc_index++] = TABLA_CODEC_PACK_ENTRY((TABLA_A_CDC_ANC1_FILT4_CTL + offset), 0xFF, pANCCfg[ancCh].dcflt_shift);

		anc_config.writes[anc_index++] = TABLA_CODEC_PACK_ENTRY(TABLA_A_CDC_CLK_ANC_CLK_EN_CTL, ancCh ? 0xC : 0x3, (1 | (1 << pANCCfg[ancCh].anc_feedback_enable)) << (ancCh*2));
		anc_config.writes[anc_index++] = TABLA_CODEC_PACK_ENTRY(TABLA_A_CDC_CLK_ANC_RESET_CTL, ancCh ? 0xC : 0x3, ~((1 | (1 << pANCCfg[ancCh].anc_feedback_enable)) << (ancCh*2)));
	}
	LOGE("done with conversion, anc config size is %d\n", anc_index);
	anc_config.size = anc_index;

	write (file_descriptor, &anc_config.size, sizeof(anc_config.size));
	write (file_descriptor, anc_config.writes, anc_config.size * TABLA_PACKED_REG_SIZE);

	return anc_index;
}

int acdb_loader_send_tabla_anc_cal(int acdb_id, int file_descriptor)
{
	int32_t		  result;
	int			  index;
	uint32_t			anc_cal[3];
	struct cal_block		*block;
	struct adie_codec_tabla_db_anc_cfg *ancCfg;
	uint32_t			*addr;

	block = (struct cal_block *)&anc_cal[1];

	if (!block) {
		LOGE("Error retrieving calibration block\n");
		return -EPERM;
	}
	result = get_anc_table(acdb_id, block, 0);
	if (result) {
		return result;
	}
	addr = (uint32_t *)(virt_cal_buffer + block->cal_offset);
	ancCfg = (struct adie_codec_tabla_db_anc_cfg *)(addr);

	convert_anc_data_to_tabla(ancCfg, file_descriptor);

	return 0;
}
#ifdef USE_8660_LEGACY_ACDB_INTF
static uint32_t get_voice_topology(int acdb_id)
{
	int32_t			result;
	AcdbDeviceInfoCmdType	acdb_dev_info_cmd;
	AcdbDeviceInfo3Type	acdb_dev_info;
	AcdbQueryResponseType	response;

	acdb_dev_info_cmd.nDeviceId = acdb_id;
	acdb_dev_info_cmd.nParamId = ACDB_DEVICE_INFO_PARAM3;
	acdb_dev_info_cmd.nBufferLength = sizeof(acdb_dev_info);
	acdb_dev_info_cmd.nBufferPointer = (uint8_t *)&acdb_dev_info;

	LOGD("ACDB -> ACDB_CMD_GET_DEVICE_INFO\n");

	result = acdb_ioctl(ACDB_CMD_GET_DEVICE_INFO,
		(const uint8_t *)&acdb_dev_info_cmd, sizeof(acdb_dev_info_cmd),
		(uint8_t *)&response, sizeof(response));
	if (result) {
		LOGE("Error: ACDB get voice rx topology for acdb id = %d, returned = %d\n",
		     acdb_id, result);
		goto err;
	}
	return acdb_dev_info.ulVoiceTopologyId;
err:
	return 0;
}
#else
static uint32_t get_voice_topology(int acdb_id)
{
	int32_t				result;
	AcdbGetVocProcTopIdCmdType	acdb_get_top;
	AcdbGetTopologyIdRspType	audio_top;

	acdb_get_top.nDeviceId = acdb_id;

	LOGD("ACDB -> ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID\n");

	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID,
		(const uint8_t *)&acdb_get_top, sizeof(acdb_get_top),
		(uint8_t *)&audio_top, sizeof(audio_top));
	if (result) {
		LOGE("Error: ACDB get voice rx topology for acdb id = %d, returned = %d\n",
		     acdb_id, result);
		goto err;
	}
	return audio_top.nTopologyId;
err:
	return 0;
}
#endif

static void get_sidetone(int rxacdb_id, int txacdb_id,
			struct sidetone_cal *cal_data)
{
	int32_t				result;
	AcdbAfeDataCmdType		sidetone;
	AcdbQueryResponseType		response;

	sidetone.nTxDeviceId = txacdb_id;
	sidetone.nRxDeviceId = rxacdb_id;
	sidetone.nModuleId = ACDB_MID_SIDETONE;
	sidetone.nParamId = ACDB_PID_SIDETONE;
	sidetone.nBufferLength = sizeof(cal_data);
	sidetone.nBufferPointer = (uint8_t *)cal_data;

	LOGD("ACDB -> ACDB_CMD_GET_AFE_DATA\n");

	result = acdb_ioctl(ACDB_CMD_GET_AFE_DATA,
		(const uint8_t *)&sidetone, sizeof(sidetone),
		(uint8_t *)&response, sizeof(response));
	if (result)
		LOGE("Error: ACDB AFE DATA Returned = %d\n", result);
}

static void get_voctable(int acdb_network, int acdb_samplerate,
				int vss_network, int rxacdb_id,
				int txacdb_id, struct cal_block *block)
{
	int32_t				result;
	AcdbVocProcTableCmdType		voctable;
	AcdbQueryResponseType		response;
	/* Point address to designated PMEM block */
	uint32_t			*addr =
		(uint32_t *)(virt_cal_buffer + global_offset);

	voctable.nTxDeviceId = txacdb_id;
	voctable.nRxDeviceId = rxacdb_id;
	voctable.nTxDeviceSampleRateId = get_samplerate(txacdb_id);
	voctable.nRxDeviceSampleRateId = get_samplerate(rxacdb_id);
	voctable.nNetworkId = acdb_network;
	voctable.nVocProcSampleRateId = acdb_samplerate;
	voctable.nBufferLength = MAX_BLOCK_SIZE;
	voctable.nBufferPointer = (uint8_t *)&addr[2];

	LOGD("ACDB -> ACDB_CMD_GET_VOCPROC_COMMON_TABLE\n");

	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_COMMON_TABLE,
		(const uint8_t *)&voctable, sizeof(voctable),
		(uint8_t *)&response, sizeof(response));
	if (result) {
		LOGE("Error: ACDB VocProc 0x%x Returned = %d\n",
			acdb_network, result);
		block->cal_size = 0;
		goto done;
	}

	addr[0] = vss_network;
	addr[1] = response.nBytesUsedInBuffer;
	/* Add inserted network id & cal data size to block size */
	block->cal_size = response.nBytesUsedInBuffer + 2 * sizeof(uint32_t);
	block->cal_offset = global_offset;
	global_offset += block->cal_size;
done:
	return;
}

static void get_vocstrmtable(int acdb_network, int acdb_samplerate,
				int vss_network, struct cal_block *block)
{
	int32_t				result;
	AcdbVocStrmTableCmdType		vocstrmtable;
	AcdbQueryResponseType		response;
	/* Point address to designated PMEM block */
	uint32_t			*addr =
		(uint32_t *)(virt_cal_buffer + global_offset);

	vocstrmtable.nNetworkId = acdb_network;
	vocstrmtable.nVocProcSampleRateId = acdb_samplerate;
	vocstrmtable.nBufferLength = ACDB_BLOCK_SIZE;
	vocstrmtable.nBufferPointer = (uint8_t *) &addr[2];

	LOGD("ACDB -> ACDB_CMD_GET_VOCPROC_STREAM_TABLE\n");

	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_STREAM_TABLE,
		(const uint8_t *)&vocstrmtable,	sizeof(vocstrmtable),
		(uint8_t *)&response, sizeof(response));
	if (result) {
		LOGE("Error: ACDB VocProc 0x%x Stream Returned = %d\n",
			acdb_network, result);
		block->cal_size = 0;
		goto done;
	}

	addr[0] = vss_network;
	addr[1] = response.nBytesUsedInBuffer;
	/* Add inserted network id & cal data size to block size */
	block->cal_size = response.nBytesUsedInBuffer + 2 * sizeof(uint32_t);
	block->cal_offset = global_offset;
	global_offset += block->cal_size;
done:
	return;
}

static void get_vocvoltable(int acdb_network, int acdb_samplerate,
				int vss_network, int rxacdb_id,
				int txacdb_id, struct cal_block *block)
{
	AcdbVocProcVolTblCmdType	vocvoltable;
	int32_t				result;
	AcdbQueryResponseType		response;
	/* Point address to designated PMEM block */
	uint32_t			*addr =
		(uint32_t *)(virt_cal_buffer + global_offset);

	vocvoltable.nTxDeviceId = txacdb_id;
	vocvoltable.nRxDeviceId = rxacdb_id;
	vocvoltable.nNetworkId = acdb_network;
	vocvoltable.nVocProcSampleRateId = acdb_samplerate;
	vocvoltable.nBufferLength = ACDB_BLOCK_SIZE;

	/* If calibration is set in separate packets (in-band)*/
	/* then always send number of steps, otherwise */
	/* only first cal block should have the number of */
	/* Volume steps as per the Q6 api change on 8960  */
	if (VOICE_CAL_IN_BAND || first_vol_table)
		vocvoltable.nBufferPointer = (uint8_t *) &addr[1];
	else
		vocvoltable.nBufferPointer = (uint8_t *) &addr[0];

	LOGD("ACDB -> ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL\n");

	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL,
		(const uint8_t *)&vocvoltable,	sizeof(vocvoltable),
		(uint8_t *)&response, sizeof(response));
	if (result) {
		LOGE("Error: ACDB Vocvol 0x%x Vol Returned = %d\n",
			acdb_network, result);
		block->cal_size = 0;
		goto done;
	}

	/* If calibration is set in separate packets (in-band)*/
	/* then always send number of steps, otherwise */
	/* only pack number of volume steps for */
	/* first voice volume buffer */
	if (VOICE_CAL_IN_BAND || first_vol_table) {
		first_vol_table = 0;
		addr[0] = addr[1];
		addr[1] = vss_network;
		block->cal_size = response.nBytesUsedInBuffer + sizeof(uint32_t);
	} else {
		addr[0] = vss_network;
		block->cal_size = response.nBytesUsedInBuffer;
	}
	/* Add inserted network id to size */
	block->cal_offset = global_offset;
	global_offset += block->cal_size;
done:
	return;
}

static void send_voice_rx_topology(int acdb_id)
{
	int32_t		result;
	uint32_t	topology;

	LOGD("ACDB -> send_voice_rx_topology\n");

	topology = get_voice_topology(acdb_id);

	if (!topology)
		goto done;

	result = ioctl(acdb_handle, AUDIO_SET_VOICE_RX_TOPOLOGY,
		(unsigned int)&topology);
	if (result)
		LOGE("Error: Sending ACDB voice rx topology result = %d\n", result);
done:
	return;
}

static void send_voice_tx_topology(int acdb_id)
{
	int32_t		result;
	uint32_t	topology;

	LOGD("ACDB -> send_voice_tx_topology\n");

	topology = get_voice_topology(acdb_id);

	if (!topology)
		goto done;

	result = ioctl(acdb_handle, AUDIO_SET_VOICE_TX_TOPOLOGY,
		(unsigned int)&topology);
	if (result)
		LOGE("Error: Sending ACDB voice tx topology result = %d\n", result);
done:
	return;
}

static void send_sidetone(int rxacdb_id, int txacdb_id)
{
	int 		result;
	/* Size to one sidetone_cal block (32B) */
	/* and store size as first element */
	uint32_t	sidetone_cal[2];

	get_sidetone(rxacdb_id, txacdb_id,
		(struct sidetone_cal *)&sidetone_cal[1]);

	/* Sidetone_cal data size */
	sidetone_cal[0] = sizeof(struct sidetone_cal);

	LOGD("ACDB -> AUDIO_SET_SIDETONE_CAL\n");
	result = ioctl(acdb_handle, AUDIO_SET_SIDETONE_CAL,
			(unsigned int)&sidetone_cal);
	if (result)
		LOGE("Error: Sending ACDB sidetone data result = %d\n", result);
}

static void send_voctable(int rxacdb_id, int txacdb_id)
{
	int result;
	/* Allocate a cal_block (64B) for each network */
	/* Add + 1 so we can store size as first element */
	uint32_t vocproc_cal[NUM_NETWORKS * 2 + 1];
	uint32_t offset = 1;

	if (SUPPORT_VOLTE_CAL) {
		get_voctable(ACDB_NETWORK_ID_VOLTE,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_VOLTE_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
		offset = offset + 2;
		get_voctable(ACDB_NETWORK_ID_VOLTE, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_VOLTE_WB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
		offset = offset + 2;
		get_voctable(ACDB_NETWORK_ID_VOLTE,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_VOLTE_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
		offset = offset + 2;
	}

	get_voctable(ACDB_NETWORK_ID_VOIP,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_VOIP_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;
	get_voctable(ACDB_NETWORK_ID_VOIP, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_VOIP_WB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;
	get_voctable(ACDB_NETWORK_ID_VOIP,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_VOIP_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;

	get_voctable(ACDB_NETWORK_ID_CDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_CDMA_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;
	get_voctable(ACDB_NETWORK_ID_CDMA, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_CDMA_WB,	rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;
	get_voctable(ACDB_NETWORK_ID_CDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_CDMA_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;

	get_voctable(ACDB_NETWORK_ID_GSM,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_GSM_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;
	get_voctable(ACDB_NETWORK_ID_GSM, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_GSM_WB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;
	get_voctable(ACDB_NETWORK_ID_GSM,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_GSM_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;

	get_voctable(ACDB_NETWORK_ID_WCDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_WCDMA_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;
	get_voctable(ACDB_NETWORK_ID_WCDMA, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_WCDMA_WB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);
	offset = offset + 2;
	get_voctable(ACDB_NETWORK_ID_WCDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_WCDMA_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocproc_cal[offset]);

	vocproc_cal[0] = NUM_NETWORKS * sizeof(struct cal_block);

	LOGD("ACDB -> AUDIO_SET_VOCPROC_CAL\n");
	result = ioctl(acdb_handle, AUDIO_SET_VOCPROC_CAL, (unsigned int)&vocproc_cal);
	if (result)
		LOGE("Error: Sending ACDB VocProc data result = %d\n", result);
}

static void send_vocstrmtable(void) {
	int result;
	/* Allocate a cal_block (64B) for each network */
	/* Add + 1 so we can store size as first element */
	uint32_t vocstrm_cal[NUM_NETWORKS * 2 + 1];
	uint32_t offset = 1;

	if (SUPPORT_VOLTE_CAL) {
		get_vocstrmtable(ACDB_NETWORK_ID_VOLTE,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_VOLTE_NB,
			(struct cal_block *)&vocstrm_cal[offset]);
		offset = offset + 2;
		get_vocstrmtable(ACDB_NETWORK_ID_VOLTE, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_VOLTE_WB,
			(struct cal_block *)&vocstrm_cal[offset]);
		offset = offset + 2;
		get_vocstrmtable(ACDB_NETWORK_ID_VOLTE,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_VOLTE_WV,
			(struct cal_block *)&vocstrm_cal[offset]);
		offset = offset + 2;
	}

	get_vocstrmtable(ACDB_NETWORK_ID_VOIP,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_VOIP_NB,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;
	get_vocstrmtable(ACDB_NETWORK_ID_VOIP, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_VOIP_WB,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;
	get_vocstrmtable(ACDB_NETWORK_ID_VOIP,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_VOIP_WV,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;

	get_vocstrmtable(ACDB_NETWORK_ID_CDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_CDMA_NB,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;
	get_vocstrmtable(ACDB_NETWORK_ID_CDMA, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_CDMA_WB,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;
	get_vocstrmtable(ACDB_NETWORK_ID_CDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_CDMA_WV,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;

	get_vocstrmtable(ACDB_NETWORK_ID_GSM,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_GSM_NB,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;
	get_vocstrmtable(ACDB_NETWORK_ID_GSM, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_GSM_WB,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;
	get_vocstrmtable(ACDB_NETWORK_ID_GSM,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_GSM_WV,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;

	get_vocstrmtable(ACDB_NETWORK_ID_WCDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_WCDMA_NB,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;
	get_vocstrmtable(ACDB_NETWORK_ID_WCDMA, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_WCDMA_WB,
			(struct cal_block *)&vocstrm_cal[offset]);
	offset = offset + 2;
	get_vocstrmtable(ACDB_NETWORK_ID_WCDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_WCDMA_WV,
			(struct cal_block *)&vocstrm_cal[offset]);

	vocstrm_cal[0] = NUM_NETWORKS * sizeof(struct cal_block);

	LOGD("ACDB -> AUDIO_SET_VOCPROC_STREAM_CAL\n");
	result = ioctl(acdb_handle, AUDIO_SET_VOCPROC_STREAM_CAL,
		(unsigned int)&vocstrm_cal);
	if (result < 0)
		LOGE("Error: Sending ACDB VOCPROC STREAM fail result %d\n",
			result);
}


static void send_vocvoltable(int rxacdb_id, int txacdb_id)
{
	int result;
	/* Allocate a cal_block (64B) for each network */
	/* Add + 1 so we can store size as first element */
	uint32_t vocvol_cal[NUM_NETWORKS * 2 + 1];
	uint32_t offset = 1;

	/* needed for formating data for Q6 */
	/* when packing data out-of-band */
	first_vol_table = 1;

	if (SUPPORT_VOLTE_CAL) {
		get_vocvoltable(ACDB_NETWORK_ID_VOLTE,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_VOLTE_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
		offset = offset + 2;
		get_vocvoltable(ACDB_NETWORK_ID_VOLTE, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_VOLTE_WB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
		offset = offset + 2;
		get_vocvoltable(ACDB_NETWORK_ID_VOLTE,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_VOLTE_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
		offset = offset + 2;
	}

	get_vocvoltable(ACDB_NETWORK_ID_VOIP,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_VOIP_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;
	get_vocvoltable(ACDB_NETWORK_ID_VOIP, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_VOIP_WB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;
	get_vocvoltable(ACDB_NETWORK_ID_VOIP,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_VOIP_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;

	get_vocvoltable(ACDB_NETWORK_ID_CDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_CDMA_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;
	get_vocvoltable(ACDB_NETWORK_ID_CDMA, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_CDMA_WB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;
	get_vocvoltable(ACDB_NETWORK_ID_CDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_CDMA_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;

	get_vocvoltable(ACDB_NETWORK_ID_GSM,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_GSM_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;
	get_vocvoltable(ACDB_NETWORK_ID_GSM, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_GSM_WB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;
	get_vocvoltable(ACDB_NETWORK_ID_GSM,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_GSM_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;

	get_vocvoltable(ACDB_NETWORK_ID_WCDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE,
			VSS_NETWORK_ID_WCDMA_NB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;
	get_vocvoltable(ACDB_NETWORK_ID_WCDMA, ACDB_VOCPROC_ID_WIDEBAND,
			VSS_NETWORK_ID_WCDMA_WB, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);
	offset = offset + 2;
	get_vocvoltable(ACDB_NETWORK_ID_WCDMA,
			ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE,
			VSS_NETWORK_ID_WCDMA_WV, rxacdb_id, txacdb_id,
			(struct cal_block *)&vocvol_cal[offset]);

	vocvol_cal[0] = NUM_NETWORKS * sizeof(struct cal_block);

	LOGD("ACDB -> AUDIO_SET_VOCPROC_VOL_CAL\n");
	result = ioctl(acdb_handle, AUDIO_SET_VOCPROC_VOL_CAL,
			(unsigned int)&vocvol_cal);
	if (result)
		LOGE("Error: Sending ACDB VocProc data result = %d\n", result);
}

static int validate_voc_cal_dev_pair(int rxacdb_id, int txacdb_id)
{
	int result;
	AcdbDevicePairType dev_pair;
	AcdbDevicePairingResponseType response;

	dev_pair.nTxDeviceId = txacdb_id;
	dev_pair.nRxDeviceId = rxacdb_id;

        result = acdb_ioctl(ACDB_CMD_IS_DEVICE_PAIRED, &dev_pair,
			sizeof(dev_pair), &response, sizeof(response));

	if (result < 0) {
		LOGE("Error: failure to vaildate the device pair = %d\n",
			result);
		goto done;
	}

	result = (int)response.ulIsDevicePairValid;
done:
	return result;
}
	
void acdb_loader_send_voice_cal(int rxacdb_id, int txacdb_id)
{
	LOGD("ACDB -> send_voice_cal, acdb_rx = %d, acdb_tx = %d\n",
		rxacdb_id, txacdb_id);

	/* check if it is valid RX/TX device pair */
	if (validate_voc_cal_dev_pair(rxacdb_id, txacdb_id) != 1) {
		LOGE("Error invalid device pair");
		goto done;
	}

	if (virt_cal_buffer == NULL) {
		LOGE("ACDB -> Not correctly initialized!\n");
		goto done;
	}

	/* Used to place voice data sequentially */
	global_offset = 0;
	send_voice_rx_topology(rxacdb_id);
	send_voice_tx_topology(txacdb_id);
	send_sidetone(rxacdb_id, txacdb_id);
	send_voctable(rxacdb_id, txacdb_id);
	send_vocvoltable(rxacdb_id, txacdb_id);

	if (global_offset > CVP_BUFFER_SIZE)
		LOGE("ACDB -> Voice cal overwrote block size by %d bytes!!!\n",
		(global_offset - CVP_BUFFER_SIZE));

	/* CVS data has to be 4K aligned */
	/* since CVS is seprate call to Q6 */
	global_offset = CVP_BUFFER_SIZE;
	send_vocstrmtable();

	/* Send AFE cal for both devices */
	/* if AFE cal supported */
	if (NUM_AFE_BLOCKS) {
		send_afe_cal(txacdb_id, TX_CAL);
		send_afe_cal(rxacdb_id, RX_CAL);
	}

	LOGD("ACDB -> Sent VocProc Cal!\n");
done:
	return;
}


void acdb_loader_deallocate_ACDB(void)
{
	int	result;

	LOGD("ACDB -> deallocate_ACDB\n");
	munmap(virt_cal_buffer, ACDB_BUFFER_SIZE);
	close(map_handle);
	result = ioctl(ion_handle, ION_IOC_FREE, &alloc_handle);
	if (result)
		LOGE("ION_IOC_FREE errno: %d\n", result);
	close(ion_handle);
	close(acdb_handle);
	LOGD("ACDB -> deallocate_ACDB done!\n");
}


#ifdef ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID

int acdb_loader_get_remote_acdb_id(unsigned int native_acdb_id)
{
	int				result;
	AcdbGetRmtCompDevIdCmdType	cmd;
	AcdbGetRmtCompDevIdRspType	response;

	LOGD("ACDB -> acdb_loader_get_remote_acdb_id, acdb_id = %d\n",
		native_acdb_id);

	if (virt_cal_buffer == NULL) {
		LOGE("ACDB -> Not correctly initialized!\n");
		result = INVALID_DATA;
		goto done;
	}

	cmd.nNativeDeviceId = native_acdb_id;

	result = acdb_ioctl(ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID, &cmd,
			sizeof(cmd), &response, sizeof(response));
	if (result < 0) {
		LOGE("Error: Remote ACDB ID lookup failed = %d\n",
			result);
		goto done;
	}

	result = response.nRmtDeviceId;
done:
	return result;
}

#endif /* ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID */
