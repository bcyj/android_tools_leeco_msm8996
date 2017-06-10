/*======================================================================
Copyright (C) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
====================================================================== */

#include <sys/time.h>

#include <stdlib.h>
#include <string.h>

#include "frameproc.h"
#include "wavelet_denoise.h"
#define SKIP_YPLANE 0

/*============================================================================
                        EXTERNAL VARIABLES DEFINITIONS
============================================================================*/
static const int q_factor_8bit = 7;
static const int q_factor_10bit = 5;
static int abort_t = 0;
#if 0
const CameraDenoisingType noiseProfileData_Q20_default =
{
  24,
  {20709376,    36558852,    34905636,    49914316,
    10485759,    29917176,    27668980,    39617552,
    5505033,    16054288,    33724468,    47268448,
    2359297,     6307848,    24566306,    49660284,
    6291455,    20389880,    43197724,    55904252,
    2621442,     9580551,    26350442,    51002688}
};
#endif
static pthread_t pp_thread, pp_thread1, pp_thread2;
static pthread_cond_t abort_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t denoise_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t abort_mutex = PTHREAD_MUTEX_INITIALIZER;
static const int parameter_q_factor = 15;
static const int parameter_roundoff = (1 << (15 - 1));


#define EP_LUMA   204
#define EP_CHROMA 510


void epsilon_filter_smooth(
  uint8_t *pInputUint8,
  int16_t *pWorkBufferInt16,
  const int32_t levelWidth,
  const int32_t levelHeight,
  const int32_t imageHeight,
  const int32_t epsilon);

/*===========================================================================

FUNCTION:  wavelet_denoise_init

RETURN VALUE:
 1 - success
 0 - failure
============================================================================*/
#if 0
int wavelet_denoise_init(void *cctrl)
{
  int i;
  config_ctrl_t *ctrl = (config_ctrl_t *)cctrl;
  /* Allocate for the structure */
  denoise_obj_t  *p_denoise =
    ctrl->ppCtrl.pp_input.denoise_input.denoise_obj;
  p_denoise = (denoise_obj_t *) malloc(sizeof(denoise_obj_t));
  if (!p_denoise)
  {
    CDBG_ERROR("Malloc of denoise object failed");
    return FALSE;
  }

  /* Initialize all fields in the denoise structure*/
  memset(p_denoise, 0, sizeof(denoise_obj_t));
  p_denoise->p_noiseProfileData_Q20 =
      (CameraDenoisingType *) malloc(sizeof(CameraDenoisingType));
  if (p_denoise->p_noiseProfileData_Q20 == NULL) {
    CDBG_ERROR("Malloc failed\n");
    free(p_denoise);
    return FALSE;
  }
  memcpy(p_denoise->p_noiseProfileData_Q20,  &noiseProfileData_Q20_default,
      sizeof(CameraDenoisingType));
  ctrl->ppCtrl.pp_input.denoise_input.denoise_obj = p_denoise;
  pthread_mutex_init(&denoise_mutex, NULL);
  pthread_mutex_init(&abort_mutex, NULL);
  pthread_cond_init(&abort_cond, NULL);
  return TRUE;
}
#endif
/*===========================================================================
FUNCTION:  interpolate_gamma_correction_curve
============================================================================*/

static void
interpolate_gamma_correction_curve (short int *output_gamma, double lux_index, void *Ctrl)
{
  frame_proc_t *frameCtrl = (frame_proc_t *)Ctrl;
  chromatix_parms_type *chromatix_ptr = frameCtrl->input.chromatix;
  stats_proc_aec_info_t *aec_out = &frameCtrl->input.statsproc_info.aec_d;
  int i;double luxindex = lux_index;
  if(!luxindex)
      luxindex = aec_out->lux_idx;
  CDBG_DENOISE("Lux index = %f",aec_out->lux_idx);
  if (luxindex <= 121)
  {
      for (i = 0; i < GAMMA_TABLE_SIZE; i+=16)
      {
          output_gamma[i/16] = (short int)(chromatix_ptr->chromatix_rgb_outdoor_gamma_table_snapshot.gamma[i]<<q_factor_8bit);
      }
  }
  else if (luxindex <= 161)
  {
      for (i = 0; i < GAMMA_TABLE_SIZE; i+=16)
      {
          output_gamma[i/16] = (short int)( ((chromatix_ptr->chromatix_rgb_outdoor_gamma_table_snapshot.gamma[i]<<q_factor_8bit) * (161. - aec_out->lux_idx) +\
                                       (chromatix_ptr->chromatix_rgb_default_gamma_table_snapshot.gamma[i]<<q_factor_8bit) * (aec_out->lux_idx - 121.)) / (161. - 121.));
      }
  }
  else if (luxindex <= 250)
  {
      for (i = 0; i < GAMMA_TABLE_SIZE; i+=16)
      {
          output_gamma[i/16] = (short int) (chromatix_ptr->chromatix_rgb_default_gamma_table_snapshot.gamma[i]<<q_factor_8bit);
      }
  }
  else if (luxindex <= 289)
  {
      for (i = 0; i < GAMMA_TABLE_SIZE; i+=16)
      {
          output_gamma[i/16] = (short int) (((chromatix_ptr->chromatix_rgb_default_gamma_table_snapshot.gamma[i]<<q_factor_8bit) * (289. - aec_out->lux_idx) +\
                                       (chromatix_ptr->chromatix_rgb_yhi_ylo_gamma_table_snapshot.gamma[i]<<q_factor_8bit) * (aec_out->lux_idx - 250.)) / (289. - 250.));
      }
  }
  else
  {
      for (i = 0; i < GAMMA_TABLE_SIZE; i+=16)
      {
          output_gamma[i/16] = (short int) chromatix_ptr->chromatix_rgb_yhi_ylo_gamma_table_snapshot.gamma[i]<<q_factor_8bit;
      }
  }
}
/*===========================================================================
FUNCTION:  inverse_gamma_correction_curve
============================================================================*/
static void
inverse_gamma_correction_curve (short int *input, int *output)
{
  int i,j;
  short int count[257];
  for ( i = 0; i < 257; i++)
  {
    output[i] = 0;
    count[i] = 0;
  }
  for ( i = 0; i < GAMMA_TABLE_ENTRIES - 1; i++)
  {
    int slope = input[i+1] - input[i];
    int temp = input[i]<<4;
    for( j=0; j<(1<<4); j++, temp+=slope) {
      int floor_temp = temp>>(q_factor_8bit+4);
      int weight = temp - (floor_temp<<(q_factor_8bit+4));
      output[floor_temp] += ((1<<(q_factor_8bit+4))-weight)*((i<<4)+j);
      output[floor_temp+1] += weight*((i<<4)+j);
      count[floor_temp] += (1<<(q_factor_8bit+4))-weight;
      count[floor_temp+1] += weight;
    }
  }
  if (count[0] > 0)
  {
    output[0] = (output[0]<<q_factor_10bit)/count[0];
  }
  for ( i = 1; i < 257; i++)
  {
    if (count[i] > 0)
    {
      output[i] = (output[i]<<q_factor_10bit)/count[i];
    }else {
      output[i] = output[i - 1];
    }
  }
}
/*===========================================================================
FUNCTION:  inverse_gamma_correction
============================================================================*/
static double
inverse_gamma_correction (double input,
              int *inverse_gamma_correction_curve)
{
  int temp;
  double weight;
  temp = (int) (floor (input));
  weight = input - temp;
  return (inverse_gamma_correction_curve[temp] * (1 - weight) +
      inverse_gamma_correction_curve[temp + 1] * weight)/(1<<q_factor_10bit);
}
/*===========================================================================
FUNCTION:  gamma_correction
============================================================================*/
static double
gamma_correction (double input, short int *gamma_correction_curve)
{
  int temp ;
  double weight;
  temp= (int) (floor (input/16));
  weight = (input/16) - temp;
  return (gamma_correction_curve[temp] * (1 - weight) + gamma_correction_curve[temp + 1] *
    weight)/(1<<q_factor_8bit);
}
/*===========================================================================
FUNCTION:  binary search
============================================================================*/
static int binary_search(ReferenceNoiseProfile_type *noise,
                         int array_size, double value)
{
  int low = 0;
  int high = array_size-1;
  while(low<high) {
    int mid = low + (high-low)/2;
    if(noise[mid].referencePatchAverageValue>value) {
      low = mid+1;
    } else {
      high = mid;
    }
  }
  return low;
}
/*===========================================================================
FUNCTION:  interpolate_noise_profile
============================================================================*/
void interpolate_noise_profile (ReferenceNoiseProfile_type *noise_profile,
                               int patch_index,double calibration_level,
                               CameraDenoisingType * output,double gain)
{
  int i;
  float high = noise_profile[patch_index-1].referencePatchAverageValue;
  float low = noise_profile[patch_index].referencePatchAverageValue,noise_data;
  /* To Obtain Denoise scale value from Chromatix 207*/
  float denoise_scale = 3;
  for (i = 0; i < CAMERA_WAVELET_DENOISING_DATA_SIZE; i++) {
    noise_data=
      noise_profile[patch_index-1].referenceNoiseProfileData[i] *
      (calibration_level - low) / (high - low);
    noise_data +=
      noise_profile[patch_index].referenceNoiseProfileData[i] *
      (high - calibration_level) /(high - low);
    output->referenceNoiseProfileData[i] =
      noise_data * gain * (1 << Q20) * denoise_scale;
  }
}
/*===========================================================================

FUNCTION:  wavelet_denoise_set

RETURN VALUE:
 1 - success
  0 - failure
  ============================================================================*/
int wavelet_denoise_calibrate(void *Ctrl,  denoise_t *wdCtrl)
{
  uint8_t  i;
  frame_proc_t *frameCtrl = (frame_proc_t *)Ctrl;
  chromatix_parms_type *pchromatix = frameCtrl->input.chromatix;
  uint32_t lines_to_pad;
  double D_old =1.177;
  double top, bottom, new_top, new_bottom;
  /* AWB green gain* digital gain *Analog gain */
  double D_new = (frameCtrl->input.statsproc_info.awb_d.snapshot_wb.g_gain*
    frameCtrl->input.statsproc_info.aec_d.snap.real_gain);
  double current_level = 128, gain;
  double lowlight_luxidx = 248;
  short int calibrate_gamma[GAMMA_TABLE_ENTRIES];
  short int current_gamma[GAMMA_TABLE_ENTRIES];
  int current_gamma_inverse[257];
  double calibration_level,factor = 1;
  wavelet_gamma_table_struct gamma_table;
  wdCtrl->width = frameCtrl->input.mctl_info.picture_dim.width;
  wdCtrl->height = frameCtrl->input.mctl_info.picture_dim.height;
  wdCtrl->state = IDLE;
  /*Added check for min width/height requirement
    Also width,height not multiple of 4 is not supported */
  if (wdCtrl->width <= 128 || wdCtrl->height <= 128)
  {
    LOGV("resolution is too low does not need wavelet denoising");
    return FALSE;
  }
  if ((wdCtrl->width % 4)!=0 || (wdCtrl->height % 4)!=0)
  {
    CDBG_ERROR("Resolution not multiple of 4 cannot support wavelet denoising");
    return FALSE;
  }

  /*Dynamically set segment line size depending on image height */
  if (wdCtrl->height <= 640)
    wdCtrl->segmentLineSize = wdCtrl->height;
  else if (wdCtrl->height <= 1280)
    wdCtrl->segmentLineSize=((wdCtrl->height / 2 + 16) >> 4) << 4;
  else if (wdCtrl->height <= 2560)
    wdCtrl->segmentLineSize = ((wdCtrl->height / 4 + 16) >> 4) << 4;
  else
    wdCtrl->segmentLineSize = ((wdCtrl->height / 8 + 16) >> 4) << 4;

  /* Retrieve and check subsampling format from color */
  wdCtrl->subsampling = FRAME_PROC_H2V2;

  /*lines required for padding (2*(2^n)-2)
    for number of levels < 4 we add 16 lines to make it integer number of MCU row
    for number of levels> 4 we add 2*2^n which is multiple of MCU rows*/
  if (NUMBER_OF_LEVELS < 4)
  {
    lines_to_pad = 16;
  }
  else
  {
    lines_to_pad = 2 * (1 << NUMBER_OF_LEVELS);
  }
  wdCtrl->lines_to_pad = lines_to_pad;
  if(!pchromatix->noise_profile)
  {
    CDBG_ERROR("No noise profile data");
    return 1;
  }
  CDBG_DENOISE("new_gain %f ", D_new);
  /* Calculate low light gamma table*/
  interpolate_gamma_correction_curve (calibrate_gamma,lowlight_luxidx, Ctrl);
  /* Calculate current gamma table for snapshot */
  interpolate_gamma_correction_curve (current_gamma,0, Ctrl);
  /* find inverse gamma correction */
  inverse_gamma_correction_curve (current_gamma, current_gamma_inverse);
  top = current_level + 8;
  if (top > 255) {
    top = 255;
  }
  bottom = current_level - 8;
  if (bottom < 0) {
    bottom = 0;
  }
  /* find equivalent point in inverse gamma curve */
  new_top = inverse_gamma_correction (top, current_gamma_inverse);
  new_bottom =
    inverse_gamma_correction (bottom, current_gamma_inverse);
  new_top *= D_old / D_new;
  new_bottom *= D_old / D_new;
  if (new_top > 1023) {
    new_top = 1023;
  }
  if (new_bottom > 1023) {
    new_bottom = 1023;
  }
  /* find new top value in low light */
  new_top = gamma_correction (new_top, calibrate_gamma);
  /* find  new bottom value in low light */
  new_bottom = gamma_correction (new_bottom, calibrate_gamma);
  CDBG_DENOISE("new_top= %g newbottom= %g", new_top, new_bottom);
  CDBG_DENOISE ("calibration_level =%g, gain= %g", (new_top + new_bottom) / 2,
      (top - bottom) / (new_top - new_bottom));

  calibration_level = (new_top + new_bottom) / 2;
  if (new_top >= new_bottom + 1) {
    gain = (top - bottom) / (new_top - new_bottom);
  }
  else {
    gain = top - bottom;
  }
  /*  Calculate avg of noise patch average */
  int patch_index = binary_search(pchromatix->noise_profile,
      NUMBER_OF_SETS, calibration_level);
  if(patch_index==0) {
    patch_index = 1;
    if(calibration_level> pchromatix->noise_profile[0].referencePatchAverageValue) {
      calibration_level =pchromatix->noise_profile[0].referencePatchAverageValue;
    }
  }else if(patch_index == (NUMBER_OF_SETS-1) &&
      calibration_level <
      pchromatix->noise_profile[NUMBER_OF_SETS -1].referencePatchAverageValue) {
    calibration_level = pchromatix->noise_profile[NUMBER_OF_SETS -1].referencePatchAverageValue;
  }
  CDBG_DENOISE("patch index %d",patch_index);
  interpolate_noise_profile (pchromatix->noise_profile,patch_index,calibration_level,
      wdCtrl->p_noiseProfileData_Q20, gain);

  return 1;
}

/*===========================================================================

FUNCTION:  wavelet_denoise_abort

RETURN VALUE:
1 - success
0 - failure
============================================================================*/
#if 0
void wavelet_denoise_abort(void *cctrl)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)cctrl;
  denoise_obj_t  *p_denoise = ctrl->ppCtrl.pp_input.denoise_input.denoise_obj;
  pthread_mutex_lock(&denoise_mutex);
  if(p_denoise != NULL)
    p_denoise->state = ABORTING;
  pthread_mutex_unlock(&denoise_mutex);
  pthread_mutex_lock(&abort_mutex);
  if(abort_t)
    pthread_cond_wait(&abort_cond, &abort_mutex);
  pthread_mutex_unlock(&abort_mutex);
}
#endif

/*===========================================================================

FUNCTION:  wavelet_denoise_exit

RETURN VALUE:
 1 - success
 0 - failure
============================================================================*/
#if 0
int wavelet_denoise_exit(void *cctrl)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)cctrl;
  cam_ctrl_dimension_t *dim = &(ctrl->dimInfo);
  denoise_obj_t  *p_denoise = ctrl->ppCtrl.pp_input.denoise_input.denoise_obj;
  /* Deallocate for the structure*/

  if (ctrl->ppCtrl.pp_input.denoise_input.denoise_obj)
  {
    denoise_obj_t  *p_denoise = ctrl->ppCtrl.pp_input.denoise_input.denoise_obj;
    free(p_denoise->p_noiseProfileData_Q20);
    p_denoise->p_noiseProfileData_Q20 = NULL;
    /*  Release memory of denoise */
    free(p_denoise);
    ctrl->ppCtrl.pp_input.denoise_input.denoise_obj = NULL;
  }
  pthread_cond_destroy(&abort_cond);
  pthread_mutex_destroy(&abort_mutex);
  pthread_mutex_destroy(&denoise_mutex);
  return TRUE;
}
#endif
/*===========================================================================

FUNCTION:  wavelet_denoise_execute

RETURN VALUE:
 1 - success
 0 - failure
============================================================================*/
int32_t wavelet_denoise_process(void *Ctrl, denoise_t *wdCtrl)
{
  int rc = TRUE;
  frame_proc_t *frameCtrl = (frame_proc_t *)Ctrl;
  if (!wdCtrl) {
    CDBG_ERROR("stderr denoise_start: empty input");
    return FALSE;
  }

  /* Check whether the Denoise is IDLE before proceeding */
  if (wdCtrl->state != IDLE) {
    CDBG_ERROR("stderr denoise_start: state not IDLE");
    return FALSE;
  }

  /* Try to create and initialize available underlying engines */
//  pthread_mutex_lock(&denoise_mutex);
  if (wdCtrl->state == ABORTING) {
    wdCtrl->state = IDLE;
    CDBG_ERROR("stderr denoise_start: state during ABORTING");
  //  pthread_mutex_unlock(&denoise_mutex);
    return FALSE;
  }
  if (rc) {
    wdCtrl->state = DENOISE;
    CDBG_DENOISE("calling denoising manager\n");
    wdCtrl->subsampling = frameCtrl->input.mctl_info.main_img_format;
    wdCtrl->frame  =  &frameCtrl->input.mctl_info.main_img_frame[0];
    rc = camera_denoising_manager(wdCtrl);
  }
  wdCtrl->state = IDLE;
  if (!rc) {
    CDBG_HIGH("%s: WAVELET DENOISING failed !!! ", __func__);
   // pthread_mutex_unlock(&denoise_mutex);
    return rc;
  }
  CDBG_HIGH("%s: Wavelet Denoise Success", __func__);
  //pthread_mutex_unlock(&denoise_mutex);
  return rc;
}
#if 0
int32_t wavelet_denoise_execute(void *cctrl)
{
  int rc = TRUE;
  config_ctrl_t *ctrl = (config_ctrl_t *)cctrl;
  cam_ctrl_dimension_t *dim = &(ctrl->dimInfo);
  denoise_obj_t  *p_denoise = ctrl->ppCtrl.pp_input.denoise_input.denoise_obj;
  if (!p_denoise)
  {
    CDBG_ERROR("stderr denoise_start: empty input");
    return FALSE;
  }

  /* Check whether the Denoise is IDLE before proceeding */
  if (p_denoise->state != IDLE)
  {
    CDBG_ERROR("stderr denoise_start: state not IDLE");
    return FALSE;
  }

  /* Try to create and initialize available underlying engines */
  pthread_mutex_lock(&denoise_mutex);
  if ((p_denoise != NULL) && (p_denoise->state == ABORTING))
  {
    p_denoise->state = IDLE;
    CDBG_ERROR("stderr denoise_start: state during ABORTING");
    pthread_mutex_unlock(&denoise_mutex);
    return FALSE;
  } else {
    p_denoise->state = DENOISE;
    pthread_mutex_lock(&abort_mutex);
    abort_t =1;
    pthread_mutex_unlock(&abort_mutex);
  }

  pthread_mutex_unlock(&denoise_mutex);
  if (rc)
  {
    CDBG_HIGH("calling denoising manager\n");
    p_denoise->frame  =  &ctrl->ppCtrl.pp_input.main_img_frame[0];
    if (!p_denoise->frame ||!(p_denoise->frame->buffer) ||
      p_denoise->frame->planar0_off == p_denoise->frame->planar1_off ) {
      CDBG("Wavelet_denoise_execute: empty frame!");
    } else {
      rc = camera_denoising_manager(p_denoise);
    }
  }
  pthread_mutex_lock(&abort_mutex);
  abort_t = 0;
  pthread_cond_signal(&abort_cond);
  pthread_mutex_unlock(&abort_mutex);
  pthread_mutex_lock(&denoise_mutex);
  p_denoise->state = IDLE;
  pthread_mutex_unlock(&denoise_mutex);
  if (!rc)
  {
    CDBG_ERROR("WAVELET DENOISING failed !!! ");
    return rc;
  }
  CDBG_HIGH("Wavelet Denoise Success");
  return rc;
}
#endif
void denoise_yuv_interleave_chroma_asm
(
 uint8_t                          *p_chroma_line,  // dest chroma line
 uint32_t                          chroma_width,   // src chroma width
 uint8_t                          *p_c0_line,      // src Cb/Cr line
 uint8_t                          *p_c1_line       // src Cr/Cb line
)
{
  while (chroma_width)
  {
      __asm__ __volatile__
      (
          "vld2.u8 {d0,d2}, [%1]! \n"
          "vld2.u8 {d1,d3}, [%2]! \n"
          "vst4.u8 {d0-d3}, [%0]! \n"
          :"+r"(p_chroma_line), "+r"(p_c0_line), "+r"(p_c1_line)
          :
          :"memory", "d0", "d1", "d2", "d3"
       );
       chroma_width = chroma_width - 16;
  }
} /* ppf_yuv_interleave_chroma */

void denoise_yuv_interleave_chroma
(
 uint8_t                          *p_chroma_line,  // dest chroma line
 uint32_t                          chroma_width,   // src chroma width
 uint8_t                          *p_c0_line,      // src Cb/Cr line
 uint8_t                          *p_c1_line       // src Cr/Cb line
)
{
  while (chroma_width--)
  {
      // CbCr interleaved
      *p_chroma_line++ = *p_c0_line++;
      *p_chroma_line++ = *p_c1_line++;
  }

} /* ppf_yuv_interleave_chroma */

void denoise_yuv_deinterleave_crcb_asm (
  uint8_t                          *p_chroma_line,
  uint32_t                          chroma_width,
  uint8_t                          *p_cb_line,
  uint8_t                          *p_cr_line
)
{
  // Deinterleave one chroma line
  while (chroma_width)
  {
      __asm__ __volatile__
      (
          "vld4.u8 {d0-d3}, [%0]! \n"
          "vst2.u8 {d0,d2}, [%1]! \n"
          "vst2.u8 {d1,d3}, [%2]! \n"
          :"+r"(p_chroma_line), "+r"(p_cr_line), "+r"(p_cb_line)
          :
          :"memory", "d0", "d1", "d2", "d3"
      );
      chroma_width = chroma_width - 16;
  }
}

void denoise_yuv_deinterleave_crcb (
  uint8_t                          *p_chroma_line,
  uint32_t                          chroma_width,
  uint8_t                          *p_cb_line,
  uint8_t                          *p_cr_line
)
{
  // Deinterleave one chroma line
  while (chroma_width--)
  {
      // Cr - CrCb interleaved
      *p_cr_line++ = *p_chroma_line++;
      // Cb - CrCb interleaved
      *p_cb_line++ = *p_chroma_line++;
  }
}

/*===========================================================================

Function            : separate_planes

Description         : Returns 3 planes of y ,Cb and Cr  with the requested size

Input parameter(s)  : frame

Output parameter(s) : None

=========================================================================== */
static int separate_planes(struct msm_pp_frame *frame ,  rect *p_rect,
                            rect_data *p_rect_data, uint32_t chromaShiftIndicatorVert,
			    uint32_t chromaShiftIndicatorHori)
{
  uint32_t i = 0;
  uint8_t *ptr,*temp_cbptr,*temp_crptr;
  uint32_t offset = 0;

  p_rect_data->cb_plane =
    (uint8_t *) malloc(sizeof(uint8_t) * ((p_rect->imageWidth * p_rect->dy)/4));
  p_rect_data->cr_plane =
    (uint8_t *) malloc(sizeof(uint8_t) * ((p_rect->imageWidth * p_rect->dy)/4));
  if (p_rect_data->cr_plane == NULL || p_rect_data->cb_plane == NULL)
  {
    CDBG_ERROR("Error allocating separate cb and cr buffers");
    return FALSE;
  }

  /* Copy Cb and Cr plane */
    temp_cbptr = p_rect_data->cb_plane;
    temp_crptr = p_rect_data->cr_plane;
  for(i = 0; i < ((p_rect->dy)/2); i++) {
    ptr = (uint8_t *)frame->mp[1].vaddr + frame->mp[1].data_offset+ ((p_rect->y>>1) * p_rect->imageWidth) + offset;
    if(((p_rect->imageWidth)/2)%16)
    {
      denoise_yuv_deinterleave_crcb(ptr, (p_rect->imageWidth>>1), (temp_cbptr), (temp_crptr));
    }
    else
    {
      denoise_yuv_deinterleave_crcb_asm(ptr, (p_rect->imageWidth>>1), (temp_cbptr), (temp_crptr));
    }
    offset = offset + p_rect->imageWidth;
    temp_cbptr = temp_cbptr + ((p_rect->imageWidth)/2);
    temp_crptr = temp_crptr + ((p_rect->imageWidth)/2);
  }
  CDBG_DENOISE("Copied Cb plane from %p to %p  with size %d",
      (uint8_t *)frame->mp[1].vaddr + frame->mp[1].data_offset+((p_rect->y * p_rect->imageWidth)/2),
      p_rect_data->cb_plane,(p_rect->imageWidth * p_rect->dy)/4);
  CDBG_DENOISE("Copying Cr plane from %p to %p  with size %d",
      (uint8_t *)frame->mp[1].vaddr + frame->mp[1].data_offset+((p_rect->y * p_rect->imageWidth)/2),
      p_rect_data->cr_plane,(p_rect->imageWidth * p_rect->dy)/4);
  return TRUE;
}
/*===========================================================================

Function            : join_planes

Description         : Returns 3 planes of y ,Cb and Cr  with the requested size

Input parameter(s)  : OS_THREAD_FUNC_ARG_T arg

Output parameter(s) : None

Return Value        : None

Side Effects        : None

=========================================================================== */
int join_planes(struct msm_pp_frame *frame,rect_data *p_rect_data,
       uint32_t       start_offset_Y,
                       uint32_t       start_offset_Cb,
                       uint32_t       start_offset_Cr,
		        uint32_t       chromaShiftIndicatorVert,
			 uint32_t       chromaShiftIndicatorHori)
{
  uint32_t cb_off = 0;
  uint32_t cr_off = 0;
  uint32_t i =0;
  uint8_t *ptr;
  uint32_t bytes_to_copy;
  uint32_t offset = 0;

  /* Copy Cb and Cr plane*/
  CDBG_DENOISE("Join Cb from %p to  %p  with size %d ",p_rect_data->cb_plane +start_offset_Cb+ cb_off,
      (uint8_t *)frame->mp[1].vaddr +  frame->mp[1].data_offset +
      ((p_rect_data->output_position * p_rect_data->output_width)/2) +i,
      (p_rect_data->output_width * p_rect_data->output_height)/4);

  bytes_to_copy = (p_rect_data->output_width)/2;
  for(i = 0; i<(p_rect_data->output_height/2); i++) {
      ptr = (uint8_t *)frame->mp[1].vaddr +  frame->mp[1].data_offset +
        ((p_rect_data->output_position * p_rect_data->output_width)/2) + offset;
      if(((p_rect_data->output_width)/2)%16)
      {
        denoise_yuv_interleave_chroma(ptr,bytes_to_copy, p_rect_data->cr_plane +start_offset_Cr + cr_off,
                                    p_rect_data->cb_plane + start_offset_Cb + cb_off);
      }
      else
      {
        denoise_yuv_interleave_chroma_asm(ptr,bytes_to_copy, p_rect_data->cr_plane +start_offset_Cr + cr_off,
                                    p_rect_data->cb_plane + start_offset_Cb + cb_off);
      }

      offset = offset + p_rect_data->output_width;
      cb_off = cb_off + (p_rect_data->output_width/2);
      cr_off = cr_off + (p_rect_data->output_width/2);
  }
#if 0
  for(i =0 ; i< ((p_rect_data->output_width * p_rect_data->output_height)/2); i++)
  {
    if((i%2))
    {
      memcpy((uint8_t *)frame->buffer + frame->planar1_off +
        ((p_rect_data->output_position * p_rect_data->output_width)/2) +i,
         p_rect_data->cb_plane +start_offset_Cb + cb_off,
         1);
      cb_off ++;
    } else {
      memcpy( (uint8_t *)frame->buffer + frame->planar1_off +
        ((p_rect_data->output_position * p_rect_data->output_width)/2)+ i,
         p_rect_data->cr_plane + start_offset_Cr +cr_off,
         1);
      cr_off++;
    }
  }
#endif
  CDBG_DENOISE("Join Cr from %p to  %p  with size %d",p_rect_data->cr_plane + start_offset_Cr +cr_off,
      (uint8_t *)frame->mp[1].vaddr + frame->mp[1].data_offset +
      ((p_rect_data->output_position * p_rect_data->output_width)/2)+ i,
      (p_rect_data->output_width * p_rect_data->output_height)/4);
  free(p_rect_data->cr_plane);
  free(p_rect_data->cb_plane);
  return TRUE;
}

/*===========================================================================

Function            : camera_denoising_manager

Description         : Top level camera denoising function
                      1) Fetch X+padded lines from input image
                      2) Trigger 3 threads for Y,Cb and Cr to denoise the image
                      3) Output Y lines to get output image

Input parameter(s)  : OS_THREAD_FUNC_ARG_T arg

Output parameter(s) : None

Return Value        : None

Side Effects        : None

=========================================================================== */
static int camera_denoising_manager(denoise_t  *p_denoise)
{
 /* Luma,Cb and Cr input destination pointers */
  uint8_t *lumaIpDstPtr = NULL, *cbIpDstPtr = NULL, *crIpDstPtr = NULL;
  int16_t *workingBufferPtrCb, *lineBufferPtrCb;
  int16_t *workingBufferPtrCr, *lineBufferPtrCr;
  uint32_t imageWidth, imageHeight;
  uint32_t chromaSegmentWidth = 0, chromaSegmentHeight;
  uint32_t chromaShiftIndicatorHori = 0, chromaShiftIndicatorVert = 0;
  uint32_t linesToFetch, linesToOutput, remainingRows, outputLinePosition=0;
  uint32_t numberOfLinestoPad;
  int    rc;
  /* 2 threads for cb and Cr respectively */
  dwt_noise_reduction_struct dwtCbSegmentStruct;
  dwt_noise_reduction_struct dwtCrSegmentStruct;
  CameraDenoisingType *pLocalNoiseProfileData;
  rect    *p_rect  = (rect *) malloc(sizeof(rect));
  uint32_t start_offset_Cb , start_offset_Cr;

  imageWidth = p_denoise->width;
  imageHeight =  p_denoise->height;
  CDBG_DENOISE("Image Width %d , Image Height %d", imageWidth, imageHeight);
  pLocalNoiseProfileData = p_denoise->p_noiseProfileData_Q20;

  if (!p_rect) {
    CDBG_ERROR("Heap Memory allocation failed");
    pthread_mutex_lock(&denoise_mutex);
    p_denoise->state = IDLE;
    pthread_mutex_unlock(&denoise_mutex);
    return FALSE;
  }

  /*lines required for padding (2*(2^n)-2)
  for number of levels < 4 we add 16 lines to make it integer number of MCU row
  for number of levels > 4 we add 2*2^n which is multiple of MCU rows */
  numberOfLinestoPad = p_denoise->lines_to_pad;
  /* Allocate memory in heap for Y,Cb and Cr components */
  if (p_denoise->subsampling == FRAME_PROC_H2V2)
  {
    chromaSegmentWidth = imageWidth >> 1;
    chromaSegmentHeight = (p_denoise->segmentLineSize + 2 * numberOfLinestoPad) >> 1;
    chromaShiftIndicatorHori = 1;
    chromaShiftIndicatorVert = 1;
  }
  else if (p_denoise->subsampling == FRAME_PROC_H2V1)
  {
    chromaSegmentWidth = imageWidth >> 1;
    chromaSegmentHeight = (p_denoise->segmentLineSize + 2 * numberOfLinestoPad);
    chromaShiftIndicatorHori = 1;
    chromaShiftIndicatorVert = 0;
  }
  else if (p_denoise->subsampling == FRAME_PROC_H1V2)
  {
    chromaSegmentWidth = imageWidth;
    chromaSegmentHeight = (p_denoise->segmentLineSize + 2 * numberOfLinestoPad) >> 1;
    chromaShiftIndicatorHori = 0;
    chromaShiftIndicatorVert = 1;
  }
  else if (p_denoise->subsampling == FRAME_PROC_H1V1)
  {
    chromaSegmentWidth = imageWidth;
    chromaSegmentHeight = (p_denoise->segmentLineSize + 2 * numberOfLinestoPad);
    chromaShiftIndicatorHori = 0;
    chromaShiftIndicatorVert = 0;
  }

  /* Allocate working buffer and line buffer pointers */
  workingBufferPtrCb =
    (int16_t *) malloc(sizeof(int16_t) * (imageWidth >> chromaShiftIndicatorHori)*
    ((p_denoise->segmentLineSize+2*numberOfLinestoPad)>>chromaShiftIndicatorVert));
  lineBufferPtrCb =
    (int16_t *) malloc(sizeof(int16_t) * 4 *(max(imageWidth, imageHeight) + 4));
  workingBufferPtrCr =
    (int16_t *) malloc(sizeof(int16_t) * (imageWidth >> chromaShiftIndicatorHori)*
    ((p_denoise->segmentLineSize+2*numberOfLinestoPad)>>chromaShiftIndicatorVert));
  lineBufferPtrCr =
    (int16_t *) malloc(sizeof(int16_t) * 4 * (max(imageWidth, imageHeight) + 4));

  if (workingBufferPtrCb == NULL || lineBufferPtrCb == NULL ||
      workingBufferPtrCr == NULL || lineBufferPtrCr == NULL)
  {
    if (workingBufferPtrCb)
    {
      free(workingBufferPtrCb);
      workingBufferPtrCb = NULL;
    }
    if (lineBufferPtrCb)
    {
      free(lineBufferPtrCb);
      lineBufferPtrCb = NULL;
    }
    if (workingBufferPtrCr)
    {
      free(workingBufferPtrCr);
      workingBufferPtrCr = NULL;
    }
    if (lineBufferPtrCr)
    {
      free(lineBufferPtrCr);
      lineBufferPtrCr = NULL;
    }
    CDBG_ERROR("Heap Memory allocation failed");
    pthread_mutex_lock(&denoise_mutex);
    p_denoise->state = IDLE;
    pthread_mutex_unlock(&denoise_mutex);
    return FALSE;
  }

  /*if frameheight > Segment size then linestofetch= SEGMENT_LINE_SIZE + padding lines
  else linestofetch=frameheight*/
  if (imageHeight > p_denoise->segmentLineSize  + numberOfLinestoPad)
  {
    linesToFetch = numberOfLinestoPad + p_denoise->segmentLineSize;
    linesToOutput = p_denoise->segmentLineSize;
  }
  else
  {
    linesToFetch = imageHeight;
    linesToOutput = linesToFetch;
  }
  /*1) Fetch the noisy image segment
  2) Do wavelet denoising for Y,Cb and Cr in 3 separate threads
  3) Output the denoised image segment*/
  p_rect->y = 0;
  p_rect->imageWidth = imageWidth;
  p_rect->dy = linesToFetch;

  start_offset_Cb = 0;
  start_offset_Cr = 0;

  do
  {
    /* Request data */
    rect_data  p_rect_data;
    CDBG_DENOISE("prect y %d   imageWidth %d  dy %d",p_rect->y,p_rect->imageWidth,
      p_rect->dy);
    //pthread_mutex_lock(&denoise_mutex);
    CDBG_DENOISE("Before Calling seperate_planes\n");
    if ((p_denoise != NULL) && (p_denoise->state != ABORTING))
      rc = separate_planes(p_denoise->frame, p_rect, &p_rect_data, 0, 0);
    else{
      rc = FALSE;
    }
    CDBG_DENOISE("AFter Calling seperate_planes\n");
    //pthread_mutex_unlock(&denoise_mutex);
    CDBG_DENOISE("Before creating a thread\n");
    /*p_rect_data->data.planes[2] = p_rect_data->data.planes[1];
    If fetch succesful start processing */
    if (rc &&
        p_rect_data.cb_plane &&
        p_rect_data.cr_plane )
    {
      dwtCbSegmentStruct.inputImagePtr = p_rect_data.cb_plane;
      dwtCbSegmentStruct.workingImagePtr = workingBufferPtrCb;
      dwtCbSegmentStruct.lineImagePtr = lineBufferPtrCb;
      dwtCbSegmentStruct.inputWidth = imageWidth >> chromaShiftIndicatorHori;
      dwtCbSegmentStruct.inputHeight = linesToFetch >> chromaShiftIndicatorVert;
      dwtCbSegmentStruct.levels = NUMBER_OF_LEVELS;
      dwtCbSegmentStruct.noiseProfilePtr = pLocalNoiseProfileData;
      dwtCbSegmentStruct.lumaChromaIndicator = 1;
      rc = pthread_create(&pp_thread1, NULL, wavelet_denoising_segment,
          (void *)(&dwtCbSegmentStruct));

    CDBG_DENOISE("After creating a thread\n");
      /* If there is a failure in creating the thread, clean up and exit */
      if (rc)
      {

        CDBG_ERROR("create thread failed\n");
        free(workingBufferPtrCb);
        free(workingBufferPtrCr);
        free(lineBufferPtrCb);
        free(lineBufferPtrCr);
        free(p_rect);
        p_denoise->state = IDLE;
        return FALSE;
      }
    CDBG_DENOISE("Before creating a thread\n");
      dwtCrSegmentStruct.inputImagePtr = p_rect_data.cr_plane;
      dwtCrSegmentStruct.workingImagePtr = workingBufferPtrCr;
      dwtCrSegmentStruct.lineImagePtr = lineBufferPtrCr;
      dwtCrSegmentStruct.inputWidth = imageWidth >> chromaShiftIndicatorHori;
      dwtCrSegmentStruct.inputHeight = linesToFetch >> chromaShiftIndicatorVert;
      dwtCrSegmentStruct.levels = NUMBER_OF_LEVELS;
      dwtCrSegmentStruct.noiseProfilePtr = pLocalNoiseProfileData;
      dwtCrSegmentStruct.lumaChromaIndicator = 2;

      rc = pthread_create(&pp_thread2, NULL, wavelet_denoising_segment,
        (void *)(&dwtCrSegmentStruct));

    CDBG_DENOISE("After creating a thread\n");
      /* If there is a failure in creating the thread, clean up and exit */
      if (rc)
      {
        free(workingBufferPtrCb);
        free(workingBufferPtrCr);
        free(lineBufferPtrCb);
        free(lineBufferPtrCr);
        free(p_rect);
        p_denoise->state = IDLE;
        return FALSE;
      }
      /*  Join the thread */
      CDBG_DENOISE("wait for threads to complete\n");
      pthread_join(pp_thread1, NULL);
      pthread_join(pp_thread2, NULL);
      CDBG_DENOISE("two threads exited\n");

    }
    else
    {

      free(workingBufferPtrCb);
      free(workingBufferPtrCr);
      free(lineBufferPtrCb);
      free(lineBufferPtrCr);
      free(p_rect);
      p_denoise->state = IDLE;
      return FALSE;
    }

    /* Set output property */
    p_rect_data.output_width = imageWidth;
    p_rect_data.output_height = linesToOutput;
    p_rect_data.output_position= outputLinePosition;

    //pthread_mutex_lock(&denoise_mutex);
    if( (p_denoise != NULL) && (p_denoise->state != ABORTING))
      rc = join_planes(p_denoise->frame,&p_rect_data, 0,
           start_offset_Cb, start_offset_Cr, 0, 0);
    else {
      free(p_rect_data.y_plane);
      free(p_rect_data.cr_plane);
      free(p_rect_data.cb_plane);
      rc = FALSE;
    }
    //pthread_mutex_unlock(&denoise_mutex);
    if (!rc)
    {
      free(workingBufferPtrCb);
      free(workingBufferPtrCr);
      free(lineBufferPtrCb);
      free(lineBufferPtrCr);
      free(p_rect);
      p_denoise->state = IDLE;
      return FALSE;
    }
    outputLinePosition = outputLinePosition + linesToOutput;
    remainingRows = imageHeight - outputLinePosition;

    /* Update fetch vertical postion */
    p_rect->y += (linesToFetch - 2 * numberOfLinestoPad);

    /*calculate new input source pointer positions
    Input Segment Start = Output Segment Start - numberOfLinestoPad
    Input segment end = Output segment end + number of lines to pad

    calculate remaining lines to fetch */
    if (remainingRows <= p_denoise->segmentLineSize)
    {
      linesToFetch = numberOfLinestoPad + remainingRows;
      linesToOutput = remainingRows;
    }
    else if (remainingRows <= 2 * numberOfLinestoPad + p_denoise->segmentLineSize)
    {
      linesToFetch = numberOfLinestoPad + remainingRows;
      linesToOutput = remainingRows;
    }
    else
    {
      linesToFetch = 2 * numberOfLinestoPad + p_denoise->segmentLineSize;
      linesToOutput = p_denoise->segmentLineSize;
    }

    p_rect->dy = linesToFetch;
    start_offset_Cb = chromaSegmentWidth * (numberOfLinestoPad >> chromaShiftIndicatorVert);
    start_offset_Cr = chromaSegmentWidth * (numberOfLinestoPad >> chromaShiftIndicatorVert);
  }
  while (remainingRows>0);

  p_denoise->state = IDLE;

  free(workingBufferPtrCb);
  workingBufferPtrCb = NULL;
  free(workingBufferPtrCr);
  workingBufferPtrCr = NULL;
  free(lineBufferPtrCb);
  lineBufferPtrCb = NULL;
  free(lineBufferPtrCr);
  lineBufferPtrCr = NULL;
  free(p_rect);


  return TRUE;
}
/*===========================================================================
FUNCTION        wavelet_denoising_segment

DESCRIPTION     High level function performs wavelet denoising

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void *wavelet_denoising_segment(void *arg)
{
  uint32_t segmentWidth,segmentHeight;
  uint32_t levelWidth[NUMBER_OF_LEVELS+1],levelHeight[NUMBER_OF_LEVELS+1];
  uint32_t i;
  uint32_t hl_max[NUMBER_OF_LEVELS+1],hh_max[NUMBER_OF_LEVELS+1];
  CameraDenoisingType * pNoiseProfile;
  uint32_t *pNoiseProfileData;
  uint32_t datasize;

  CDBG("wavelet_denoising_segment E\n");
  dwt_noise_reduction_struct *pLocalDenoisingInput = (dwt_noise_reduction_struct *)arg;

  segmentWidth = pLocalDenoisingInput->inputWidth;
  segmentHeight = pLocalDenoisingInput->inputHeight;

  levelHeight[0] = segmentHeight;
  levelWidth[0] = segmentWidth;

  for(i = 0; i < NUMBER_OF_LEVELS; i++)
  {
    levelWidth[i+1] = (levelWidth[i] + 1) >> 1;
    levelHeight[i+1] = (levelHeight[i] + 1) >> 1;
  }
  /* read the thresholds */
  pNoiseProfile = (CameraDenoisingType *)pLocalDenoisingInput->noiseProfilePtr;
  pNoiseProfileData = (uint32_t*)pNoiseProfile->referenceNoiseProfileData;

  /*1 means cb
  2 means cr */
  datasize=WAVELET_DENOISING_DATA_SIZE_BY_6;
  if (pLocalDenoisingInput->lumaChromaIndicator==1)
  {
    pNoiseProfileData=pNoiseProfileData+(datasize*2);
  }
  else if (pLocalDenoisingInput->lumaChromaIndicator==2)
  {
    pNoiseProfileData=pNoiseProfileData+(datasize*4);
  }

  for(i = 0; i < (datasize); i++)
  {
    hl_max[i] = *pNoiseProfileData++;
  }
  for(i = 0; i < (datasize); i++)
  {
    hh_max[i] = *pNoiseProfileData++;
  }

  /* Call wavelet transform with rotate for first level */
  dwt_haar_rot(pLocalDenoisingInput->inputImagePtr,
      pLocalDenoisingInput->workingImagePtr,
      levelWidth[0],
      levelHeight[0],
      pLocalDenoisingInput->lineImagePtr);

  /* do edge thresholding */
  if (pLocalDenoisingInput->lumaChromaIndicator==0)
  {
    edge_weighting(pLocalDenoisingInput->workingImagePtr,
        levelHeight[0],
        levelWidth[0],
        segmentHeight,
        WEIGHT_LUMA,
        (uint32_t)((10*hl_max[0] + parameter_q21_roundoff) >> 21),
        (uint32_t)((hl_max[0] + parameter_q20_roundoff) >> 20),
        (uint32_t)((hh_max[0] + parameter_q20_roundoff) >> 20));
  }
  else
  {
    edge_weighting(pLocalDenoisingInput->workingImagePtr,
        levelHeight[0],
        levelWidth[0],
        segmentHeight,
        WEIGHT_CHROMA,
        (uint32_t)((15*hl_max[0] + parameter_q21_roundoff) >> 21),
        (uint32_t)((hl_max[0] + parameter_q20_roundoff) >> 20),
        (uint32_t)((hh_max[0] + parameter_q20_roundoff) >> 20));
  }
  /*for all other levesl call wavelet transform with no rotation
  and then call edge thresholding */
  for (i=1; i<NUMBER_OF_LEVELS; i++)
  {
    dwt_53tab(pLocalDenoisingInput->inputImagePtr,
        pLocalDenoisingInput->workingImagePtr,
        levelWidth[i],
        levelHeight[i],
        segmentHeight,
        pLocalDenoisingInput->lineImagePtr);
    if (pLocalDenoisingInput->lumaChromaIndicator==0)
    {
      edge_weighting(pLocalDenoisingInput->workingImagePtr,
          levelHeight[i],
          levelWidth[i],
          segmentHeight,
          WEIGHT_LUMA,
          (uint32_t)((10*hl_max[i] + parameter_q21_roundoff) >> 21),
          (uint32_t)((hl_max[i] + parameter_q20_roundoff) >> 20),
          (uint32_t)((hh_max[i] + parameter_q20_roundoff) >> 20));
    }
    else
    {
      edge_weighting(pLocalDenoisingInput->workingImagePtr,
          levelHeight[i],
          levelWidth[i],
          segmentHeight,
          WEIGHT_CHROMA,
          (uint32_t)((15*hl_max[i] + parameter_q21_roundoff) >> 21),
          (uint32_t)((hl_max[i] + parameter_q20_roundoff) >> 20),
          (uint32_t)((hh_max[i] + parameter_q20_roundoff) >> 20));
    }

  }
  /*Call the epsilon filter function to smooth the final level
  pLocalDenoisingInput->lumaChromaIndicator ==0 luma
  pLocalDenoisingInput->lumaChromaIndicator ==1 chroma */

  if (pLocalDenoisingInput->lumaChromaIndicator==0)
  {
    epsilon_filter_smooth(pLocalDenoisingInput->inputImagePtr,
        pLocalDenoisingInput->workingImagePtr,
        levelWidth[NUMBER_OF_LEVELS],
        levelHeight[NUMBER_OF_LEVELS],
        segmentHeight,
        EP_LUMA);
  }
  else
  {
    epsilon_filter_smooth(pLocalDenoisingInput->inputImagePtr,
        pLocalDenoisingInput->workingImagePtr,
        levelWidth[NUMBER_OF_LEVELS],
        levelHeight[NUMBER_OF_LEVELS],
        segmentHeight,
        EP_CHROMA);
  }

  /* call inverse wavelet without any rotation for all levels except first */
  wavelet_transform_inverse_2d_2lines (pLocalDenoisingInput->inputImagePtr,
      pLocalDenoisingInput->workingImagePtr,
      levelWidth[3],
      levelHeight[3],
      segmentHeight,
      pLocalDenoisingInput->lineImagePtr);
  wavelet_transform_inverse_2d_2lines (pLocalDenoisingInput->inputImagePtr,
      pLocalDenoisingInput->workingImagePtr,
      levelWidth[2],
      levelHeight[2],
      segmentHeight,
      pLocalDenoisingInput->lineImagePtr);
  wavelet_transform_inverse_2d_2lines (pLocalDenoisingInput->inputImagePtr,
      pLocalDenoisingInput->workingImagePtr,
      levelWidth[1],
      levelHeight[1],
      segmentHeight,
      pLocalDenoisingInput->lineImagePtr);
  /* for first call inverse wavelet with rotate */

  wavelet_transform_inverse_2d_rot_haar (pLocalDenoisingInput->inputImagePtr,
      pLocalDenoisingInput->workingImagePtr,
      segmentWidth,
      segmentHeight,
      pLocalDenoisingInput->lineImagePtr);

  CDBG("wavelet_denoising_segment X\n");
  return NULL;
}

/*===========================================================================
FUNCTION        epsilon_filter_smooth

DESCRIPTION     epsilon filter to smooth on final level

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void epsilon_filter_smooth(
    uint8_t *pInputUint8,
    int16_t *pWorkBufferInt16,
    const int32_t levelWidth,
    const int32_t levelHeight,
    const int32_t imageHeight,
    const int32_t epsilon)
{
  const int16_t filterCoeff = 3277; /* Q15 of 0.1*/
  int32_t  i, j, sumIndex;
  int32_t  diff[4], sum;
  int16_t  *pInputInt16 = (int16_t *)pInputUint8;  /* Type casting to int16_t*/

  for (j = 0; j < levelWidth; j++)
  {
    /* Copy one line horizontally from working buffer(16 bits/pixel) to input buffer */
    memcpy(pInputInt16 + j * levelHeight, pWorkBufferInt16 + j * imageHeight,
      levelHeight * sizeof(int16_t));
  }

  for(j = 0; j < levelWidth; j++)
  {
    for(i = 0; i < levelHeight; i++)
    {
      /*------------------------------------------------------------
        Epsilon filtering on current pixle's neighborhood set:
        -   -     (top)     -     -
        - (left) current (right)  -
        -   -   (bottom)    -     -

        And on final level, image is still rotated.
        --------------------------------------------------------------*/

      /* diff[0]
       if at top most row, = 0
       otherwise, = pixel value of top neighborhood minus current pixel value */
      diff[0] = (j == 0 ? 0 : (*(pInputInt16 - levelHeight) - *pInputInt16));

      /* diff[1]
       if at left most column, = 0
       otherwise, = pixel value of left neighborhood minus current pixel value */
      diff[1] = (i == 0 ? 0 : (*(pInputInt16 - 1) - *pInputInt16));

      /* diff[2]
       if at right most column, = 0
       otherwise, = pixel value of right neighborhood minus current pixel value */
      diff[2] = (i == (levelHeight - 1) ? 0 : (*(pInputInt16 + 1) - *pInputInt16));

      /* diff[3]
       if at bottom most column, = 0
       otherwise, = pixel value of bottom neighborhood minus current pixel value */
      diff[3] = (j == (levelWidth - 1) ? 0 : (*(pInputInt16 + levelHeight) - *pInputInt16));

      /* sum up if less than epsilon to preserve edge */
      sumIndex = 4;
      sum = 0;
      while (sumIndex--)
      {
        if (abs(diff[sumIndex]) < epsilon)
        {
          sum += diff[sumIndex];
        }
      }
      /*  filtering */
      sum = (sum * filterCoeff + parameter_q15_roundoff) >> parameter_q15_factor; /*Q15*/
      /*  write back to working buffer */
      *pWorkBufferInt16 = (int16_t) (*pInputInt16 + sum);
      pInputInt16++;
      pWorkBufferInt16++;

    }
    /* update working buffer pointer to next line */
    pWorkBufferInt16 += imageHeight - levelHeight;
  }
}

/*===========================================================================
FUNCTION        dwt_53tab

DESCRIPTION     wavelet transform forward function on 2d
                for level 2 ~ maximal level

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void dwt_53tab (
    uint8_t *pInput,
    int16_t *pWorkBufferInt16,
    const  uint32_t levelWidth,
    const  uint32_t levelHeight,
    const  uint32_t imageHeight,
    int16_t *pLineBufferInt16)
{
  uint32_t i;

  /*------------------------------------------------------------
    1. Perform horizontal read from working buffer(16 bits/pixel)
    2. Process vertical FWT 1d
    3. Perform vertical write to input buffer(16 bits/pixel)

    4. Perform horizontal read from input buffer(16 bits/pixel)
    5. Process horizontal FWT 1d
    6. Perform vertical write to working buffer(16 bits/pixel)
    --------------------------------------------------------------*/
  /* Wavelet transform horizontal */
  for (i = 0; i <= levelWidth - 2; i += 2)
  {
    /* Copy one line horizontally from working buffer(16 bits/pixel) to line buffer */
    memcpy(pLineBufferInt16 + 2,
      pWorkBufferInt16 + i * imageHeight, levelHeight * sizeof(int16_t));
    /* Copy another line so that 2 input lines are in the buffer to be processed together */
    memcpy(pLineBufferInt16 + 2 + levelHeight + 2 + 2,
      pWorkBufferInt16 + (i+1) * imageHeight,
      levelHeight * sizeof(int16_t));

    /* Wavelet transform 1d */
    dwt_53tab_int16_randomwrite_2lines((int16_t *)pInput + i, /* output pointer*/
        pLineBufferInt16,    /*input pointer */
        levelHeight,         /*output length */
        levelWidth);         /* output increment */
  }

  for (; i < levelWidth; i++)
  {
    /*  Copy one line horizontally from working buffer(16 bits/pixel) to line buffer */
    memcpy(pLineBufferInt16 + 2,
      pWorkBufferInt16 + i * imageHeight, levelHeight * sizeof(int16_t));

    /*  Wavelet transform 1d */
    dwt_53tab_int16_randomwrite(((int16_t *)pInput) + i, /* output pointer */
        pLineBufferInt16,      /* input pointer */
        levelHeight,           /* output length */
        levelWidth);           /* output increment */
  }

  /*  Wavelet transform vertical */
  for (i = 0; i <= levelHeight - 2; i += 2)
  {
    /* Copy one line horizontally from input buffer(16 bits/pixel) to line buffer */
    memcpy(pLineBufferInt16 + 2,
      ((int16_t *)pInput) + i * levelWidth, levelWidth * sizeof(int16_t));
    /* Copy another line so that 2 input lines are in the buffer to be processed together */
    memcpy(pLineBufferInt16 + 2 + levelWidth + 2 + 2,
    ((int16_t *)pInput) + (i+1) * levelWidth,
    levelWidth * sizeof(int16_t));

    /* Wavelet transform 1d */
    dwt_53tab_int16_randomwrite_2lines(pWorkBufferInt16 + i,  /*output pointer */
        pLineBufferInt16,      /*input pointer */
        levelWidth,            /*output length */
        imageHeight);          /* output increment */
  }

  for (; i < levelHeight; i++)
  {
    /* Copy one line horizontally from input buffer(16 bits/pixel) to line buffer */
    memcpy(pLineBufferInt16 + 2, ((int16_t *)pInput) + i * levelWidth,
      levelWidth * sizeof(int16_t));

    /*  Wavelet transform 1d */
    dwt_53tab_int16_randomwrite(pWorkBufferInt16 + i,  /* output pointer */
        pLineBufferInt16,      /*input pointer */
        levelWidth,            /*output length*/
        imageHeight);          /*output increment */
  }
}

/*===========================================================================
FUNCTION        dwt_haar_rot

DESCRIPTION     wavelet transform forward function on 2d for first level
                and rotate image size

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void dwt_haar_rot (
    uint8_t *pInput,
    int16_t *pWorkBufferInt16,
    const  uint32_t imageWidth,
    const  uint32_t imageHeight,
    int16_t *pLineBufferInt16)
{
  uint32_t  i;
  uint8_t  *pLineBufferUint8;

  /* Wavelet transform horizontal
  split up */
  pLineBufferUint8 = pInput;
  for (i = 0; i <= imageHeight - 4; i += 4 )
  {
    /*  Wavelet transform horizontal */
    dwt_haar_uint8_randomwrite_4lines(pWorkBufferInt16 + i, /*output pointer */
        pLineBufferUint8,     /* intput pointer */
        imageWidth,           /* oUtput length */
        imageHeight);         /* output increment */

    pLineBufferUint8 = pLineBufferUint8 + imageWidth * 4;
  }

  for (; i < imageHeight; i++ )
  {
    /* Wavelet transform horizontal */
    dwt_haar_uint8_randomwrite(pWorkBufferInt16 + i, /*output pointer*/
        pLineBufferUint8,     /*intput pointer */
        imageWidth,           /*output length*/
        imageHeight);         /*output increment*/

    pLineBufferUint8 = pLineBufferUint8 + imageWidth;
  }

  /* Wavelet transform vertical */
  for (i = 0; i < imageWidth; i++)
  {
    /*  Copy one line to scratch buffer */
    memcpy(pLineBufferInt16, pWorkBufferInt16 + i * imageHeight,
        imageHeight * sizeof(*pWorkBufferInt16));

    /* Wavelet transform 1d */
    dwt_haar_int16_seqwrite(pWorkBufferInt16 + i * imageHeight, /*output pointer */
        pLineBufferInt16,                   /*intput pointer */
        imageHeight,
        1);
  }
}

/*===========================================================================
FUNCTION        wavelet_transform_inverse_2d

DESCRIPTION     wavelet transform inverse function on 2d
                for level 2 ~ maximal level

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void wavelet_transform_inverse_2d (
    uint8_t *pInput,
    int16_t *pWorkBufferInt16,
    const uint32_t levelWidth,
    const uint32_t levelHeight,
    const uint32_t imageHeight,
    int16_t *pLineBufferInt16)
{
  uint32_t i, j;
  int16_t *pHoriInt16;

  /*------------------------------------------------------------
    1. Perform horizontal read from working buffer (16 bits/pixel)
    2. Process vertical IWT 1d
    3. Perform vertical write to input buffer (16 bits/pixel)

    4. Perform horizontal read from input buffer (16 bits/pixel)
    5. Process horizontal IWT 1d
    6. Perform vertical write to working buffer (16 bits/pixel)
    --------------------------------------------------------------*/
  /* Wavelet transform horizontal*/
  for (i = 0; i < levelWidth; i++)
  {
    pHoriInt16 = pWorkBufferInt16 + i * imageHeight;
    /* Copy to line buffer*/
    for (j = 0; j < levelHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }
    for (j = 1; j < levelHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }

    /* Wavelet transform 1d */
    wavelet_transform_inverse_1d_int16 ((int16_t *)(pInput + 2 * i), /*output pointer */
        pLineBufferInt16,          /*intput pointer*/
        levelHeight,               /*output length*/
        (levelWidth << 1));        /*output increment*/
  }

  pHoriInt16 = (int16_t *)pInput;
  /* Wavelet transform vertical*/
  for (i = 0; i < levelHeight; i++)
  {
    /* Copy to line buffer*/
    for (j = 0; j < levelWidth; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }
    for (j = 1; j < levelWidth; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }
    /* Wavelet transform 1d*/
    wavelet_transform_inverse_1d_int16(pWorkBufferInt16 + i,  /*output pointer*/
        pLineBufferInt16,          /*input pointer*/
        levelWidth,                /*output length*/
        (imageHeight << 1));       /*output increment*/
  }
}

/*===========================================================================
FUNCTION        wavelet_transform_inverse_1d_int16_haar

DESCRIPTION     wavelet transform inverse function on 1d for first level
                and rotate back to original size

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void wavelet_transform_inverse_1d_int16_haar(
    int16_t *pOutputLowpass,
    int16_t *pInput,
    const uint32_t length,
    const uint32_t outputIncrement)
{
  /*synthesis polyphase filter*/
  const int16_t lowpassFilter[1] = {23170};  /* Q15*/
  const int16_t highpassFilter[1] = {23170}; /* Q15 */
  uint32_t i;
  int32_t  sum;
   /*plus 1 is for odd length*/
  int16_t *pOutputHighpass = pOutputLowpass + ((outputIncrement + 1) >> 1);

  /* update input pointer because lowpassFilter is downgraded from 5 to 3*/
  for (i = length; i >= 2; i -= 2)
  {
    /* Low pass filtering*/
    sum =  (*(pInput) + *(pInput+1)) * lowpassFilter[0];
    *pOutputLowpass = (int16_t) ((sum + Q15_ROUNDOFF) >> Q15);  /*Q15*/
    pOutputLowpass += outputIncrement;

    /* High pass filtering*/
    sum =  (*(pInput) - *(pInput+1)) * highpassFilter[0];
    *pOutputHighpass = (int16_t) ((sum + Q15_ROUNDOFF) >> Q15); /*Q15*/
    pOutputHighpass += outputIncrement;
    pInput += 2;
  }

  if (i)
  {
    /* Low pass filtering*/
    sum =   (*(pInput) + *(pInput+1)) * lowpassFilter[0];
    *pOutputLowpass = (int16_t) ((sum + Q15_ROUNDOFF) >> Q15);  /*Q15*/
  }

}
/*===========================================================================
FUNCTION        wavelet_transform_inverse_2d_rot_haar

DESCRIPTION     wavelet transform inverse function on 2d for first level
                and rotate back to original size

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void wavelet_transform_inverse_2d_rot_haar (
    uint8_t *pInput,
    int16_t *pWorkBufferInt16,
    const uint32_t imageWidth,
    const uint32_t imageHeight,
    int16_t *pLineBufferInt16)
{
  int16_t *pWorkBufferHori;
  int16_t *pInputHori;
  uint32_t i, j;

  pWorkBufferHori = pWorkBufferInt16;

  /* Wavelet transform horizontal*/
  for (i = 0; i < (imageWidth >> 1) - 4; i += 4)
  {
    /* Copy one line horizontally
     from working buffer (16 bits/pixel) to line buffer*/
    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + j) = *pWorkBufferHori ++;
    }

    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + imageHeight + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + imageHeight + j) = *pWorkBufferHori ++;
    }

    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 * imageHeight + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 * imageHeight + j) = *pWorkBufferHori ++;
    }

    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + 3 * imageHeight + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + 3 * imageHeight + j) = *pWorkBufferHori ++;
    }

    /* Haar transform 1d
    output pointer*/
    wavelet_transform_inverse_1d_int16_haar_4lines((int16_t *)(pInput + 2 * i),
        pLineBufferInt16,          /*input pointer*/
        imageHeight,               /*output length*/
        imageWidth);               /*output increment*/
  }

  for (; i < (imageWidth >> 1); i++ )
  {
    /* Copy one line horizontally
     from working buffer (16 bits/pixel) to line buffer*/
    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + j) = *pWorkBufferHori ++;
    }

    /* Haar transform 1d*/
    wavelet_transform_inverse_1d_int16_haar((int16_t *)(pInput + 2 * i), /*output pointer*/
        pLineBufferInt16,          /*input pointer*/
        imageHeight,               /*output length*/
        imageWidth);               /*output increment*/
  }

  for (i = 0; i < (imageWidth >> 1) - 4; i += 4)
  {
    /* Copy one line from working buffer to line buffer*/
    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + j) = *pWorkBufferHori ++;
    }

    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + imageHeight + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + imageHeight + j) = *pWorkBufferHori ++;
    }

    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 * imageHeight + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 * imageHeight + j) = *pWorkBufferHori ++;
    }

    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + 3 * imageHeight + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + 3 * imageHeight + j) = *pWorkBufferHori ++;
    }

    /* Haar transform 1d*/
    wavelet_transform_inverse_1d_int16_haar_4lines (pWorkBufferInt16 + i, /*output pointer//i*/
        pLineBufferInt16,     /*input pointer*/
        imageHeight,          /*output length*/
        imageWidth);          /*output increment*/
  }


  for (; i < (imageWidth >> 1); i++ )
  {
    /* Copy one line from working buffer to line buffer */
    for (j = 0; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + j) = *pWorkBufferHori ++;
    }
    for (j = 1; j < imageHeight; j += 2)
    {
      *(pLineBufferInt16 + j) = *pWorkBufferHori ++;
    }

    /* Haar transform 1d */
    wavelet_transform_inverse_1d_int16_haar(pWorkBufferInt16 + i, /*output pointer//i*/
        pLineBufferInt16,     /*input pointer*/
        imageHeight,          /*output length*/
        imageWidth);          /*output increment*/
  }

  pInputHori = (int16_t *)pInput;
  pWorkBufferHori = pWorkBufferInt16;
  /* Wavelet transform vertical*/
  for (i = 0; i < imageHeight; i++)
  {
    /* Copy one line horizontally
     from both input buffer and working buffer to line buffer*/
    for (j = 0; j < imageWidth; j += 2)
    {
      *(pLineBufferInt16 + j) = *pInputHori ++;
    }
    for (j = 1; j < imageWidth; j += 2)
    {
      *(pLineBufferInt16 + j) = *pWorkBufferHori ++;
    }
    /* Wavelet transform 1d*/
    wavelet_transform_inverse_1d_uint8_haar (pInput + i * imageWidth, /*output pointer*/
        pLineBufferInt16,        /*input pointer*/
        imageWidth);             /*output length*/
  }
}

void wavelet_transform_inverse_2d_2lines (
    uint8_t *pInput,
    int16_t *pWorkBufferInt16,
    const uint32_t levelWidth,
    const uint32_t levelHeight,
    const uint32_t imageHeight,
    int16_t *pLineBufferInt16)
{
  uint32_t i, j;
  int16_t *pHoriInt16;


  /* Wavelet transform horizontal*/
  for (i = 0; i < levelWidth - 2; i += 2)
  {
    pHoriInt16 = pWorkBufferInt16 + i * imageHeight;
    /* Copy to line buffer */
    for (j = 0; j < levelHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }
    for (j = 1; j < levelHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }

    pHoriInt16 = pWorkBufferInt16 + (i + 1) * imageHeight;
    for (j = 0; j < levelHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 + levelHeight + 4 + j) = *pHoriInt16 ++;
    }
    for (j = 1; j < levelHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 + levelHeight + 4 + j) = *pHoriInt16 ++;
    }

    /* Wavelet transform 1d */
    wavelet_transform_inverse_1d_int16_2lines ((int16_t *)(pInput + 2 * i), /*output pointer*/
        pLineBufferInt16,          /*intput pointer*/
        levelHeight,               /*output length*/
        (levelWidth << 1));        /*output increment*/
  }
  for (; i < levelWidth; i++)
  {
    pHoriInt16 = pWorkBufferInt16 + i * imageHeight;
    /* Copy to line buffer*/
    for (j = 0; j < levelHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }
    for (j = 1; j < levelHeight; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }

    /* Wavelet transform 1d*/
    wavelet_transform_inverse_1d_int16 ((int16_t *)(pInput + 2 * i), /*output pointer*/
        pLineBufferInt16,          /*intput pointer*/
        levelHeight,               /*utput length*/
        (levelWidth << 1));        /*output increment*/
  }


  pHoriInt16 = (int16_t *)pInput;
  /* Wavelet transform vertical*/
  for (i = 0; i < levelHeight - 2; i += 2)
  {
    /* Copy to line buffer*/
    for (j = 0; j < levelWidth; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }
    for (j = 1; j < levelWidth; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }
    for (j = 0; j < levelWidth; j += 2)
    {
      *(pLineBufferInt16 + 2 + levelWidth + 4 + j) = *pHoriInt16 ++;
    }
    for (j = 1; j < levelWidth; j += 2)
    {
      *(pLineBufferInt16 + 2 + levelWidth + 4 + j) = *pHoriInt16 ++;
    }
    /* Wavelet transform 1d*/
    wavelet_transform_inverse_1d_int16_2lines(pWorkBufferInt16 + i,      /*output pointer*/
        pLineBufferInt16,          /*input pointer*/
        levelWidth,                /*output length*/
        (imageHeight << 1));       /*output increment*/

  }

  for (; i < levelHeight; i++)
  {
    /* Copy to line buffer*/
    for (j = 0; j < levelWidth; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }
    for (j = 1; j < levelWidth; j += 2)
    {
      *(pLineBufferInt16 + 2 + j) = *pHoriInt16 ++;
    }
    /* Wavelet transform 1d */
    wavelet_transform_inverse_1d_int16(pWorkBufferInt16 + i,      /*output pointer*/
        pLineBufferInt16,          /*input pointer*/
        levelWidth,                /*output length*/
        (imageHeight << 1));       /*output increment*/
  }
}
