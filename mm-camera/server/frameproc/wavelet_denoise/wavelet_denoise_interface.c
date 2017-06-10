/*======================================================================
Copyright (C) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
====================================================================== */
#include <string.h>
#include "frameproc.h"
#include "camera_dbg.h"
#include "wavelet_denoise_interface.h"
#include "wavelet_denoise.h"
#include <fcntl.h>
#include <sys/time.h>

static denoise_t *wdCtrl[MAX_INSTANCES];
static const int q_factor_8bit = 7;
static const int q_factor_10bit = 5;
static pthread_mutex_t denoise_mutex = PTHREAD_MUTEX_INITIALIZER;

const CameraDenoisingType noiseProfileData_Q20_default =
{
  24,
  { 20709376,    36558852,    34905636,    49914316,
    10485759,    29917176,    27668980,    39617552,
    5505033,    16054288,    33724468,    47268448,
    2359297,     6307848,    24566306,    49660284,
    6291455,    20389880,    43197724,    55904252,
    2621442,     9580551,    26350442,    51002688 }
};

struct timeval td1, td2,td3;
struct timezone tz;
//#define BIT_EXACT_TEST

//0 - means simplified, 1 means full
#define LUMA_COMPLEXITY 1
#define CHROMA_COMPLEXITY 1


/*===========================================================================
 * FUNCTION    - wavelet_denoise_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int wavelet_denoise_set_params(frame_proc_t *frameCtrl, frame_proc_set_wd_data_t *data)
{
  int i, rc = 0;
  uint32_t index = frameCtrl->handle & 0xFF;
  denoise_t *wd = wdCtrl[index];
  switch (data->type) {
  case WAVELET_DENOISE_ENABLE:
    frameCtrl->output.wd_d.denoise_enable = data->denoise_enable;
    frameCtrl->output.wd_d.process_mode = data->process_planes;
    if (frameCtrl->output.wd_d.denoise_enable) {
      if (wavelet_denoise_init(frameCtrl) != 0) rc = -1;
    } else {
      if (wavelet_denoise_exit(frameCtrl) != 0) rc = -1;
    }
    break;
  case WAVELET_DENOISE_CALIBRATE:
    /* Calculate current gamma table for snapshot */
    if (!frameCtrl->input.isp_info.lumaAdaptationEnable) {
      if (frameCtrl->input.isp_info.RGB_gamma_table == NULL) {
        CDBG_ERROR("VFE RGB gamma table is NULL");
        return FALSE;
      }
      for (i = 0; i < frameCtrl->input.isp_info.VFE_GAMMA_NUM_ENTRIES;
        i = i + (frameCtrl->input.isp_info.VFE_GAMMA_NUM_ENTRIES /
          GAMMA_TABLE_ENTRIES)) {
        wd->input.current_gamma[i] =
          (short int)*(frameCtrl->input.isp_info.RGB_gamma_table + i);
        wd->input.current_gamma[i] =
          (wd->input.current_gamma[i] & 0xFF) << q_factor_8bit;
      }
    } else {
      if (frameCtrl->input.isp_info.LA_gamma_table == NULL) {
        CDBG_ERROR("VFE LA gamma table is NULL");
        return FALSE;
      }
      for (i = 0; i < frameCtrl->input.isp_info.VFE_LA_TABLE_LENGTH;
        i = i + (frameCtrl->input.isp_info.VFE_LA_TABLE_LENGTH /
          GAMMA_TABLE_ENTRIES)) {
        wd->input.current_gamma[i] =
          (short int)*(frameCtrl->input.isp_info.LA_gamma_table + i);
        wd->input.current_gamma[i] =
          (wd->input.current_gamma[i] & 0xFF) << q_factor_8bit;
      }
    }
    wd->input.D_new = (frameCtrl->input.statsproc_info.awb_d.snapshot_wb.g_gain *
      frameCtrl->input.statsproc_info.aec_d.snap.real_gain);
    wd->width = frameCtrl->input.mctl_info.picture_dim.width;
    wd->height = frameCtrl->input.mctl_info.picture_dim.height;
    wd->input.chromatix = frameCtrl->input.chromatix;
    wd->input.gamma_table_size = GAMMA_TABLE_SIZE;
    if (!wavelet_denoise_calibrate(frameCtrl, wd)) rc = -1;
    break;
  default:
    CDBG_ERROR("%s: Invalid AFD set param %d\n",
      __func__, data->type);
    return -1;
  }
  return rc;
} /* wavelet_denoise_set_params */

/*===========================================================================
 * FUNCTION    - wavelet_denoise_execute -
 *
 * DESCRIPTION:
 *==========================================================================*/
int wavelet_denoise_execute(frame_proc_t *frameCtrl)
{
  int i, rc = 0;
  uint32_t index = frameCtrl->handle & 0xFF;
  denoise_t *wd = wdCtrl[index];
  long exe_time;
  uint32_t fd;
  uint8_t *pinput;
  uint32_t oldwidth, oldheight;
  struct msm_pp_frame *oldframe;

  CDBG_HIGH("%s: E", __func__);

  /* Wavelet Denoise for snapshot img */
  for (i = 0; i < frameCtrl->input.mctl_info.num_main_img; i++) {
    if (frameCtrl->ignore_snap_frame == i) continue;
    wd->width = frameCtrl->input.mctl_info.picture_dim.width;
    wd->height = frameCtrl->input.mctl_info.picture_dim.height;
    wd->frame  =  &frameCtrl->input.mctl_info.main_img_frame[i];

#ifdef BIT_EXACT_TEST
    oldwidth = wd->width;
    oldheight = wd->height;
    oldframe = wd->frame;
    wd->width = 3264;
    wd->height = 2448;

    pinput = malloc(wd->width * wd->height * 1.5);

    wd->frame->mp[0].vaddr = (uint32_t)pinput;
    wd->frame->mp[0].data_offset = 0;
    wd->frame->mp[1].vaddr = (uint32_t)((uint8_t *)pinput + wd->width * wd->height);
    wd->frame->mp[1].data_offset = 0;
    memcpy(wd->Y_noiseProfileData_Q20,
      void *) & noiseProfileData_Q20_default,sizeof(CameraDenoisingType));
memcpy(wd->Chroma_noiseProfileData_Q20, (void *)&noiseProfileData_Q20_default,
  sizeof(CameraDenoisingType));

wd->y_complexity = LUMA_COMPLEXITY;
wd->cbcr_complexity = CHROMA_COMPLEXITY;
fd = open("/data/input1.ycrcb", O_RDWR | O_CREAT, 0777);
read(fd, (void *)pinput, (wd->width * wd->height * 1.5));
close(fd);
#endif
//wd->y_complexity = 0;
//wd->cbcr_complexity = 0;
//wd->process_planes = DENOISE_STREAMLINE_YCBCR;

gettimeofday(&td1, &tz);
if (!wavelet_denoise_process(frameCtrl, wd)) {
  rc = -1;
  return rc;
}
gettimeofday(&td2, &tz);
exe_time = 1000000 * (td2.tv_sec - td1.tv_sec) + td2.tv_usec - td1.tv_usec;
CDBG_ERROR("Wavelet main image time Complexity : %d %d %d Resolution: %d x %d, Time %ld",
  wd->process_planes, wd->y_complexity, wd->cbcr_complexity, wd->width, wd->height, exe_time);
#ifdef BIT_EXACT_TEST
fd = open("/data/output1.ycrcb", O_RDWR | O_CREAT | O_TRUNC, 0777);
write(fd, (void *)pinput, (wd->width * wd->height * 1.5));
close(fd);
wd->width = oldwidth;
wd->height = oldheight;
wd->frame = oldframe;
free(pinput);
#endif

}
for (i = 0; i < frameCtrl->input.mctl_info.num_thumb_img; i++) {
  if (frameCtrl->ignore_snap_frame == i) continue;
  wd->width = frameCtrl->input.mctl_info.thumbnail_dim.width;
  wd->height = frameCtrl->input.mctl_info.thumbnail_dim.height;
  wd->frame  =  &frameCtrl->input.mctl_info.thumb_img_frame[i];

  //wd->y_complexity = 0;
  //wd->cbcr_complexity = 0;
  //wd->process_planes = DENOISE_STREAMLINE_YCBCR;

  if (!wavelet_denoise_process(frameCtrl, wd)) {
    rc = -1;
    return rc;
  }
}
return rc;
} /* aec_process */

/*===========================================================================
 * FUNCTION    - wavelet_denoise_exit -
 *
 * DESCRIPTION:
 *==========================================================================*/
int wavelet_denoise_exit(frame_proc_t *frameCtrl)
{
  uint32_t index = frameCtrl->handle & 0xFF;
  denoise_t *wd = NULL;
  if (index >= MAX_INSTANCES) return -1;
  wd = wdCtrl[index];
  if (!wd) return 0;
  if (wd->Y_noiseProfileData_Q20) {
    free(wd->Y_noiseProfileData_Q20);
    wd->Y_noiseProfileData_Q20 = NULL;
  }
  if (wd->Chroma_noiseProfileData_Q20) {
    free(wd->Chroma_noiseProfileData_Q20);
    wd->Chroma_noiseProfileData_Q20 = NULL;
  }
  if (wdCtrl[index]) {
    free(wdCtrl[index]);
    wdCtrl[index] = NULL;
  }
  return 0;
} /* wavelet_denoise_exit */


/*===========================================================================

FUNCTION:  wavelet_denoise_init

RETURN VALUE:
 1 - success
 0 - failure
============================================================================*/

int wavelet_denoise_init(frame_proc_t *frameCtrl)
{
  uint32_t index = frameCtrl->handle & 0xFF;
  wdCtrl[index] = malloc(sizeof(denoise_t));
  if (!wdCtrl[index]) return -1;
  denoise_t *wd = wdCtrl[index];
  if (!wd) return -1;
  memset(wdCtrl[index], 0, sizeof(denoise_t));
  int i;
  switch (frameCtrl->output.wd_d.process_mode) {
  case  WAVELET_DENOISE_YCBCR_PLANE:
    wd->process_planes = WAVELET_DENOISE_YCBCR_PLANE;
    wd->y_complexity = 1;
    wd->cbcr_complexity = 1;
    break;
  case  WAVELET_DENOISE_CBCR_ONLY:
    wd->process_planes = WAVELET_DENOISE_CBCR_ONLY;
    wd->y_complexity = 0;
    wd->cbcr_complexity = 1;
    break;
  case WAVELET_DENOISE_STREAMLINE_YCBCR:
    wd->process_planes = WAVELET_DENOISE_YCBCR_PLANE;
    wd->y_complexity = 0;
    wd->cbcr_complexity = 0;
    break;
  case WAVELET_DENOISE_STREAMLINED_CBCR:
    wd->process_planes = WAVELET_DENOISE_CBCR_ONLY;
    wd->y_complexity = 0;
    wd->cbcr_complexity = 0;
    break;
  }
  wd->denoise_scale_Y = frameCtrl->input.chromatix->denoise_scale;
  wd->denoise_scale_chroma = frameCtrl->input.chromatix->denoise_scale;
  /* gain trigger = 0   use lux index
     gain trigger = 1   use gain
  */
  wd->gain_trigger = 1;
  /* Initialize all fields in the denoise structure*/
  wd->Y_noiseProfileData_Q20 =
    (CameraDenoisingType *)malloc(sizeof(CameraDenoisingType));
  wd->Chroma_noiseProfileData_Q20 =
    (CameraDenoisingType *)malloc(sizeof(CameraDenoisingType));
  if (wd->Y_noiseProfileData_Q20 == NULL ||
    wd->Chroma_noiseProfileData_Q20 == NULL) {
    CDBG_ERROR("Malloc failed\n");
    if (wd) {
      free(wd);
      wd = NULL;
    }
    return -1;
  }
  memcpy(wd->Y_noiseProfileData_Q20,  &noiseProfileData_Q20_default,
    sizeof(CameraDenoisingType));
  memcpy(wd->Chroma_noiseProfileData_Q20,  &noiseProfileData_Q20_default,
    sizeof(CameraDenoisingType));
  return 0;
}

/*===========================================================================
FUNCTION:  interpolate_gamma_correction_curve
============================================================================*/

static void
interpolate_gamma_correction_curve(short int *output_gamma,
  uint8_t *gamma_table)
{
  int i;
  for (i = 0; i < GAMMA_TABLE_SIZE; i += 16) {
    output_gamma[i / 16] = (short int)(gamma_table[i] << q_factor_8bit);
  }
}
/*===========================================================================
FUNCTION:  inverse_gamma_correction_curve
============================================================================*/
static void
inverse_gamma_correction_curve(short int *input, int *output)
{
  int i, j;
  short int count[257];
  for (i = 0; i < 257; i++) {
    output[i] = 0;
    count[i] = 0;
  }
  for (i = 0; i < GAMMA_TABLE_ENTRIES - 1; i++) {
    int slope = input[i + 1] - input[i];
    int temp = input[i] << 4;
    for (j = 0; j < (1 << 4); j++, temp += slope) {
      int floor_temp = temp >> (q_factor_8bit + 4);
      int weight = temp - (floor_temp << (q_factor_8bit + 4));
      output[floor_temp] += ((1 << (q_factor_8bit + 4)) - weight) * ((i << 4) + j);
      output[floor_temp + 1] += weight * ((i << 4) + j);
      count[floor_temp] += (1 << (q_factor_8bit + 4)) - weight;
      count[floor_temp + 1] += weight;
    }
  }
  if (count[0] > 0) {
    output[0] = (output[0] << q_factor_10bit) / count[0];
  }
  for (i = 1; i < 257; i++) {
    if (count[i] > 0) {
      output[i] = (output[i] << q_factor_10bit) / count[i];
    } else {
      output[i] = output[i - 1];
    }
  }
}
/*===========================================================================
FUNCTION:  inverse_gamma_correction
============================================================================*/
static double
inverse_gamma_correction(double input,
  int *inverse_gamma_correction_curve)
{
  int temp;
  double weight;
  temp = (int)(floor(input));
  weight = input - temp;
  return(inverse_gamma_correction_curve[temp] * (1 - weight) +
    inverse_gamma_correction_curve[temp + 1] * weight) / (1 << q_factor_10bit);
}
/*===========================================================================
FUNCTION:  gamma_correction
============================================================================*/
static double
gamma_correction(double input, short int *gamma_correction_curve)
{
  int temp;
  double weight;
  temp = (int)(floor(input / 16));
  weight = (input / 16) - temp;
  return(gamma_correction_curve[temp] * (1 - weight) + gamma_correction_curve[temp + 1] *
    weight) / (1 << q_factor_8bit);
}
/*===========================================================================
FUNCTION:  binary search
============================================================================*/
static int binary_search(ReferenceNoiseProfile_type *noise,
  int array_size, double value)
{
  int low = 0;
  int high = array_size - 1;
  while (low < high) {
    int mid = low + (high - low) / 2;
    if (noise[mid].referencePatchAverageValue < value) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return low;
}
/*===========================================================================
FUNCTION:  interpolate_noise_profile
============================================================================*/
void interpolate_noise_profile(ReferenceNoiseProfile_type *noise_profile,
  int patch_index, double calibration_level,
  denoise_t *wdCtrl, double gain)
{
  int i;
  float high = noise_profile[patch_index].referencePatchAverageValue;
  float low = noise_profile[patch_index - 1].referencePatchAverageValue, noise_data;
  for (i = 0; i < CAMERA_WAVELET_DENOISING_DATA_SIZE; i++) {
    noise_data = (noise_profile[patch_index - 1].referenceNoiseProfileData[i]);
    noise_data -=
      ((noise_profile[patch_index].referenceNoiseProfileData[i] -
        noise_profile[patch_index - 1].referenceNoiseProfileData[i]) *
      (low - calibration_level) / (high - low));
    wdCtrl->Y_noiseProfileData_Q20->referenceNoiseProfileData[i] =
      noise_data * gain * (1 << Q20) * wdCtrl->denoise_scale_Y;
    if (wdCtrl->y_complexity == 0) wdCtrl->Y_noiseProfileData_Q20->referenceNoiseProfileData[i] =
        wdCtrl->Y_noiseProfileData_Q20->referenceNoiseProfileData[i] / 2;
    wdCtrl->Chroma_noiseProfileData_Q20->referenceNoiseProfileData[i] =
      noise_data * gain * (1 << Q20) * wdCtrl->denoise_scale_chroma;
    if (wdCtrl->cbcr_complexity == 0) wdCtrl->Chroma_noiseProfileData_Q20->referenceNoiseProfileData[i] =
        wdCtrl->Chroma_noiseProfileData_Q20->referenceNoiseProfileData[i] / 2;

  }
}
/*===========================================================================

FUNCTION:  wavelet_denoise_calibrate

RETURN VALUE:
 1 - success
  0 - failure
  ============================================================================*/
int wavelet_denoise_calibrate(void *Ctrl, denoise_t *wdCtrl)
{
  uint8_t  i;
  uint32_t lines_to_pad;
  frame_proc_t *frameCtrl = (frame_proc_t *)Ctrl;
  chromatix_parms_type *pchromatix = frameCtrl->input.chromatix;
  double D_old = 1.177;
  double top, bottom, new_top, new_bottom;
  /* AWB green gain* digital gain *Analog gain */
  double D_new = (frameCtrl->input.statsproc_info.awb_d.snapshot_wb.g_gain *
    frameCtrl->input.statsproc_info.aec_d.snap.real_gain);
  double current_level = 128, gain;
  short int calibrate_gamma[GAMMA_TABLE_ENTRIES];
  short int current_gamma[GAMMA_TABLE_ENTRIES];
  int current_gamma_inverse[257];
  double calibration_level;
  ReferenceNoiseProfile_type *noise_profile = NULL;

  wdCtrl->state = IDLE;
  /*lines required for padding (2*(2^n)-2)
  for number of levels <= 4 we add 16 lines to make it integer number of MCU row
  for number of levels> 4 we add 2*2^(n - 1) which is multiple of MCU rows*/
  if (NUMBER_OF_LEVELS <= 4) {
    lines_to_pad = 16;
  } else {
    lines_to_pad = 2 * (1 << (NUMBER_OF_LEVELS - 1));
  }
  wdCtrl->lines_to_pad = lines_to_pad;

  if (!pchromatix->noise_profile) {
    CDBG_ERROR("No noise profile data");
    return 1;
  }
  noise_profile = pchromatix->noise_profile;
  CDBG_DENOISE("new_gain %f ", D_new);
  /* Calculate low light gamma table*/
  interpolate_gamma_correction_curve(calibrate_gamma,
    pchromatix->chromatix_rgb_yhi_ylo_gamma_table_snapshot.gamma);
  /* Calculate current gamma table for snapshot */
  if (!frameCtrl->input.isp_info.lumaAdaptationEnable) {
    if (frameCtrl->input.isp_info.RGB_gamma_table == NULL) {
      CDBG_ERROR("VFE RGB gamma table is NULL");
      return FALSE;
    }
    for (i = 0; i < frameCtrl->input.isp_info.VFE_GAMMA_NUM_ENTRIES;
      i = i + (frameCtrl->input.isp_info.VFE_GAMMA_NUM_ENTRIES / GAMMA_TABLE_ENTRIES)) {
      current_gamma[i] = (short int)*(frameCtrl->input.isp_info.RGB_gamma_table + i);
      //current_gamma[i] = (short int ) (vcmd->VFE_RGB_GammaCfgCmd.Gamatbl.table[i]);
      current_gamma[i] = (current_gamma[i] & 0xFF) << q_factor_8bit;
    }
  } else {
    if (frameCtrl->input.isp_info.LA_gamma_table == NULL) {
      CDBG_ERROR("VFE LA gamma table is NULL");
      return FALSE;
    }
    for (i = 0; i < frameCtrl->input.isp_info.VFE_LA_TABLE_LENGTH;
      i = i + (frameCtrl->input.isp_info.VFE_LA_TABLE_LENGTH / GAMMA_TABLE_ENTRIES)) {
      //      current_gamma[i] = (short int ) (vcmd->VFE_LACfgCmd.TblEntry.table[i]);
      current_gamma[i] =  (short int)*(frameCtrl->input.isp_info.LA_gamma_table + i);
      current_gamma[i] = (current_gamma[i] & 0xFF) << q_factor_8bit;
    }
  }
  /* find inverse gamma correction */
  inverse_gamma_correction_curve(current_gamma, current_gamma_inverse);
  top = current_level + 8;
  if (top > 255) {
    top = 255;
  }
  bottom = current_level - 8;
  if (bottom < 0) {
    bottom = 0;
  }
  /* find equivalent point in inverse gamma curve */
  new_top = inverse_gamma_correction(top, current_gamma_inverse);
  new_bottom =
    inverse_gamma_correction(bottom, current_gamma_inverse);
  new_top *= D_old / D_new;
  new_bottom *= D_old / D_new;
  if (new_top > 1023) {
    new_top = 1023;
  }
  if (new_bottom > 1023) {
    new_bottom = 1023;
  }
  /* find new top value in low light */
  new_top = gamma_correction(new_top, calibrate_gamma);
  /* find  new bottom value in low light */
  new_bottom = gamma_correction(new_bottom, calibrate_gamma);
  CDBG_DENOISE("new_top= %g newbottom= %g", new_top, new_bottom);
  CDBG_DENOISE("calibration_level =%g, gain= %g", (new_top + new_bottom) / 2,
    (top - bottom) / (new_top - new_bottom));

  calibration_level = (new_top + new_bottom) / 2;
  if (new_top >= new_bottom + 1) {
    gain = (top - bottom) / (new_top - new_bottom);
  } else {
    gain = top - bottom;
  }
  /* if gain_trigger is 1 , use gain to interpolate noise data */
  if (wdCtrl->gain_trigger == 1) {
    calibration_level = frameCtrl->input.statsproc_info.aec_d.snap.real_gain;
    gain = 1;
  }
  /*  Calculate avg of noise patch average */
  int patch_index = binary_search(noise_profile,
    NUMBER_OF_SETS, calibration_level);
  if (patch_index == 0) {
    patch_index = 1;
  } else if (patch_index == (NUMBER_OF_SETS - 1) &&
    calibration_level >
      noise_profile[NUMBER_OF_SETS - 1].referencePatchAverageValue) {
    calibration_level = noise_profile[NUMBER_OF_SETS - 1].referencePatchAverageValue;
  }
  CDBG_DENOISE("patch index %d", patch_index);
  interpolate_noise_profile(noise_profile, patch_index, calibration_level,
    wdCtrl, gain);
  return 1;
}

/*===========================================================================

FUNCTION:  wavelet_denoise_process

RETURN VALUE:
 1 - success
 0 - failure
============================================================================*/
int32_t wavelet_denoise_process(void *Ctrl, denoise_t *wdCtrl)
{
  int rc = TRUE;
  frame_proc_t *frameCtrl = (frame_proc_t *)Ctrl;
  chromatix_parms_type *pchromatix = frameCtrl->input.chromatix;
  denoise_lib_t denoiseCtrl;

  if (!wdCtrl) {
    CDBG_ERROR("stderr denoise_start: empty input");
    return FALSE;
  }

  /* Check whether the Denoise is IDLE before proceeding */
  if (wdCtrl->state != IDLE) {
    CDBG_ERROR("stderr denoise_start: state not IDLE");
    return FALSE;
  }
  if (frameCtrl->input.statsproc_info.aec_d.lux_idx <
      pchromatix->wavelet_enable_index) {
    CDBG_ERROR("WNR not triggered as Lux Index %f is less than %d ",
      frameCtrl->input.statsproc_info.aec_d.lux_idx,
      pchromatix->wavelet_enable_index);
    return TRUE;
  }

  /* Try to create and initialize available underlying engines */
  pthread_mutex_lock(&denoise_mutex);
  if (wdCtrl->state == ABORTING) {
    wdCtrl->state = IDLE;
    CDBG_ERROR("stderr denoise_start: state during ABORTING");
    pthread_mutex_unlock(&denoise_mutex);
    return FALSE;
  }
  if (rc) {
    wdCtrl->state = DENOISE;
    CDBG_DENOISE("calling denoising manager\n");
    wdCtrl->subsampling = frameCtrl->input.mctl_info.main_img_format;
    /*Added check for min width/height requirement
    Also width,height not multiple of 4 is not supported */
    if (wdCtrl->width <= 128 || wdCtrl->height <= 128) {
      LOGV("resolution is too low does not need wavelet denoising");
      return FALSE;
    }
    if ((wdCtrl->width % 4) != 0 || (wdCtrl->height % 4) != 0) {
      CDBG_ERROR("Resolution not multiple of 4 cannot support wavelet denoising");
      return FALSE;
    }

    /*Dynamically set segment line size depending on image height */
    if (wdCtrl->height <= 640) wdCtrl->segmentLineSize = wdCtrl->height;
    else if (wdCtrl->height <= 1280) wdCtrl->segmentLineSize = ((wdCtrl->height / 2 + 16) >> 4) << 4;
    else if (wdCtrl->height <= 2560) wdCtrl->segmentLineSize = ((wdCtrl->height / 4 + 16) >> 4) << 4;
    else wdCtrl->segmentLineSize = ((wdCtrl->height / 8 + 16) >> 4) << 4;

    //Populate denoise object to be passed into the lib
    denoiseCtrl.width = wdCtrl->width;
    denoiseCtrl.height = wdCtrl->height;
    denoiseCtrl.subsampling = wdCtrl->subsampling;
    denoiseCtrl.lines_to_pad = wdCtrl->lines_to_pad;
    denoiseCtrl.state = wdCtrl->state;
    denoiseCtrl.Y_noiseProfileData_Q20 = wdCtrl->Y_noiseProfileData_Q20;
    denoiseCtrl.Chroma_noiseProfileData_Q20 = wdCtrl->Chroma_noiseProfileData_Q20;
    denoiseCtrl.segmentLineSize = wdCtrl->segmentLineSize;
    denoiseCtrl.process_planes = wdCtrl->process_planes;
    denoiseCtrl.y_complexity = wdCtrl->y_complexity;
    denoiseCtrl.cbcr_complexity = wdCtrl->cbcr_complexity;

    switch (wdCtrl->subsampling) {
    case FRAME_PROC_H2V2:
      {
        denoiseCtrl.subsampling = DENOISE_H2V2;
        break;

      }
    case FRAME_PROC_H1V2:
      {
        denoiseCtrl.subsampling = DENOISE_H1V2;
        break;

      }
    case FRAME_PROC_H2V1:
      {
        denoiseCtrl.subsampling = DENOISE_H2V1;
        break;

      }
    case FRAME_PROC_H1V1:
      {
        denoiseCtrl.subsampling = DENOISE_H1V1;
        break;

      }
    default:
      {
        CDBG_ERROR("%s: WAVELET DENOISING failed wrong subsampling format %d !!! ",
           __func__, denoiseCtrl.subsampling);
        return FALSE;
      }
    }
    switch (wdCtrl->process_planes) {
    case WAVELET_DENOISE_YCBCR_PLANE:
      {
        denoiseCtrl.process_planes = DENOISE_YCBCR_PLANE;
        break;

      }
    case WAVELET_DENOISE_CBCR_ONLY:
      {
        denoiseCtrl.process_planes = DENOISE_CBCR_ONLY;
        break;

      }
    case WAVELET_DENOISE_STREAMLINE_YCBCR:
      {
        denoiseCtrl.process_planes = DENOISE_STREAMLINE_YCBCR;
        break;

      }
    case WAVELET_DENOISE_STREAMLINED_CBCR:
      {
        denoiseCtrl.process_planes = DENOISE_STREAMLINED_CBCR;
        break;

      }
    default:
      {
        CDBG_ERROR("%s: WAVELET DENOISING failed wrong denoising mode requested %d !!! ",
           __func__, wdCtrl->process_planes);
        return FALSE;
      }
    }

    denoiseCtrl.plumaplane = (uint8_t *)wdCtrl->frame->mp[0].vaddr + wdCtrl->frame->mp[0].data_offset;
    if (wdCtrl->frame->num_planes != 3) {
      denoiseCtrl.pchromaplane1 = (uint8_t *)wdCtrl->frame->mp[1].vaddr + wdCtrl->frame->mp[1].data_offset;
      denoiseCtrl.denoiseplane = DENOISE_SEMI_PLANAR;
    } else {
      denoiseCtrl.pchromaplane1 = (uint8_t *)wdCtrl->frame->mp[1].vaddr + wdCtrl->frame->mp[1].data_offset;
      denoiseCtrl.pchromaplane2 = (uint8_t *)wdCtrl->frame->mp[2].vaddr + wdCtrl->frame->mp[2].data_offset;
      denoiseCtrl.denoiseplane = DENOISE_PLANAR;
    }

    rc = camera_denoising_manager(&denoiseCtrl);
  }
  wdCtrl->state = IDLE;
  if (!rc) {
    CDBG_ERROR("%s: WAVELET DENOISING failed !!! ", __func__);
    pthread_mutex_unlock(&denoise_mutex);
    return rc;
  }
  CDBG_HIGH("%s: Wavelet Denoise Success", __func__);
  pthread_mutex_unlock(&denoise_mutex);
  return rc;
}
