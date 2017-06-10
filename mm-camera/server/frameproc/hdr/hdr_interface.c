/**********************************************************************
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <string.h>
#include "hdr.h"
#include "frameproc.h"
#include <sys/time.h>
static hdr_t *hdrCtrl[MAX_INSTANCES];
struct timezone tz;
#define BUFF_SIZE_32 32
#define Q20 20
#define GAMMA_TABLE_ENTRIES 64
#define NUMBER_OF_SETS 6
uint32_t timestamp;
typedef enum {
  THUMBNAIL,
  MAINIMG,
} image_type;

/*==========================================================================
 * FUNCTION    - hdr_init -
 *
 * DESCRIPTION:
 *=========================================================================*/
static int hdr_init(frame_proc_t *frameCtrl)
{
  uint32_t index = frameCtrl->handle & 0xFF;
  hdrCtrl[index] = malloc(sizeof(hdr_t));
  if (!hdrCtrl[index])
    return -1;
  memset(hdrCtrl[index], 0, sizeof(hdr_t));
  frameCtrl->output.hdr_d.max_num_hdr_frames = MAX_HDR_NUM_FRAMES;
  frameCtrl->output.hdr_d.exp_values[0] = 0.5;
  frameCtrl->output.hdr_d.exp_values[1] = 1;
  frameCtrl->output.hdr_d.exp_values[2] = 2;
  return 0;
}  /* hdr_init */

/*===========================================================================
 * FUNCTION    - hdr_exit -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int hdr_exit(frame_proc_t *frameCtrl)
{
  uint32_t index = frameCtrl->handle & 0xFF;
  if (hdrCtrl[index]) {
    hdr_stop(hdrCtrl[index]);
    free(hdrCtrl[index]);
    hdrCtrl[index] = NULL;
  }
  return 0;
}  /* hdr_exit */

/*===========================================================================
 * FUNCTION    - hdr_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int hdr_set_params(frame_proc_t *frameCtrl, frame_proc_set_hdr_data_t *data)
{
  int rc = 0;
  uint32_t index = frameCtrl->handle & 0xFF;
  hdr_t *hdr = hdrCtrl[index];
  switch (data->type) {
  case FRAME_PROC_HDR_ENABLE:
    frameCtrl->output.hdr_d.hdr_enable = data->hdr_init_info.hdr_enable;
    if (frameCtrl->output.hdr_d.hdr_enable) {
      rc = hdr_init(frameCtrl);
      //hdr->num_hal_buf = data->hdr_init_info.num_hal_buf;
    }
    else
      rc = hdr_exit(frameCtrl);
    break;
  case FRAME_PROC_HDR_HW_INFO:
    {
    /*  memcpy(&hdr->pGammaTableStruct.gamma_tbl,frameCtrl->input.isp_info.RGB_gamma_table,
        frameCtrl->input.isp_info.VFE_GAMMA_NUM_ENTRIES*sizeof(int16_t));*/
      hdr->pGammaTableStruct.gamma_tbl = frameCtrl->input.isp_info.RGB_gamma_table;
      hdr->pGammaTableStruct.entry = frameCtrl->input.isp_info.VFE_GAMMA_NUM_ENTRIES;
      hdr->pGammaTableStruct.gamma_t = GAMMA_TBL_ALL;
	  CDBG_HIGH("%s Gamma Table entries %d",__func__,hdr->pGammaTableStruct.entry );
      if (hdr->pGammaTableStruct.entry!=64) {
        CDBG_HIGH("Error: Invalid gamma table\n");
        return -1;
      }
      rc = hdr_calculate_gammatbl(frameCtrl,hdr);
    }
    break;
  default:
    return -1;
  }
  return rc;
}  /* hdr_set_params */

/*===========================================================================
 * FUNCTION    - hdr_execute -
 *
 * DESCRIPTION:
 *==========================================================================*/
int hdr_execute(frame_proc_t *frameCtrl)
{
  int rc = -1;
  uint32_t index = frameCtrl->handle & 0xFF;
  hdr_t *hdr = hdrCtrl[index];
  CDBG_HIGH("%s: E", __func__);
  rc = hdr_process((void *)frameCtrl, hdr);

  return rc;
}  /* hdr_execute */

int hdr_config_2frame(frame_proc_t *frameCtrl,hdr_t *hdrCtrl,image_type type);
/*===========================================================================

Function           : dump_frame

Description        : Dump hdr frames to /data
Input parameter(s) : hdr frame


Return Value       : null

=========================================================================== */
static void dump_frame(struct msm_pp_frame *frame, char * filename)
{
#if 1
  uint32_t i;
  int file_fd;

  file_fd = open(filename, O_RDWR | O_CREAT, 0777);
  if (file_fd < 0) {
    CDBG_HIGH("%s: cannot open file\n", __func__);
    return;
  }

  if (frame->num_planes > 1) {
    for (i=0; i < frame-> num_planes; i++) {
      write(file_fd, (const void *)((uint8_t*)frame->mp[i].vaddr +
        frame->mp[i].data_offset), frame->mp[i].length);
    }
  } else {
    write(file_fd, (const void *)((uint8_t*)frame->sp.vaddr +
      frame->sp.addr_offset), frame->sp.length);
  }
  close(file_fd);
#endif
}

static void dump_hdr_frame(frame_proc_t *frameCtrl, int main_tb)
{
  static int sn_cnt = 0;
  int cnt, i;
  char buf[BUFF_SIZE_32];
  int w, h;

  if (main_tb & 0x01) { /*main*/
    cnt =frameCtrl->input.mctl_info.num_main_img;
    w = frameCtrl->input.mctl_info.picture_dim.width;
    h = frameCtrl->input.mctl_info.picture_dim.height;
    for (i=0; i< cnt; i++) {
      snprintf(buf, sizeof(buf), "/data/%d_%dM%dx%d.yuv", sn_cnt, i, w, h);
      dump_frame(&frameCtrl->input.mctl_info.main_img_frame[i],buf);
    }
  }
  if (main_tb &0x02) { /*tb*/
    cnt =frameCtrl->input.mctl_info.num_thumb_img;
    w = frameCtrl->input.mctl_info.thumbnail_dim.width;
    h = frameCtrl->input.mctl_info.thumbnail_dim.height;

    for (i=0; i< cnt; i++) {
      snprintf(buf, sizeof(buf), "/data/%d_%dT%dx%d.yuv", sn_cnt, i, w, h);
      dump_frame(&frameCtrl->input.mctl_info.thumb_img_frame[i], buf);
    }
  }
  sn_cnt++;
  if (sn_cnt == 100) {
    sn_cnt = 0;
  }
}

/*===========================================================================

Function           : hdr_process

Description        : Entry point for HDR algorithm
Input parameter(s) : The must be one and only one buffer with output flag set


Return Value       : hdr_return_t

Side Effects       : buffer with output_flag set will be written

=========================================================================== */
int hdr_process (void *Ctrl,hdr_t *hdrCtrl)
{
  hdr_return_t rc = HDR_SUCESS;
  hdr_config_t * pIn=&(hdrCtrl->structHdrConfig);
  struct msm_pp_frame *tempFirstFrame;
  frame_proc_t *frameCtrl = (frame_proc_t *)Ctrl;

  //check for dimension multiple of 4
  if (frameCtrl->input.mctl_info.thumbnail_dim.height %4 != 0 ||
    frameCtrl->input.mctl_info.thumbnail_dim.width %4 != 0 ||
    frameCtrl->input.mctl_info.picture_dim.height %4 != 0 ||
    frameCtrl->input.mctl_info.picture_dim.width %4 != 0) {
    CDBG_HIGH("hdr_execute: HDR not supported for non multiple of 4 width and height");
    return HDR_ERROR;
  }

  dump_hdr_frame(frameCtrl, 0x02); /*dump tb for debugging*/

  CDBG_HIGH("hdr execute snapshot and thumb frame_num %d ; %d",
    frameCtrl->input.mctl_info.num_main_img,frameCtrl->input.mctl_info.num_thumb_img);
  CDBG_HIGH("%s thumb width %d n height %d  and snapshot width %d n height %d",__func__,
    frameCtrl->input.mctl_info.thumbnail_dim.width,
	frameCtrl->input.mctl_info.thumbnail_dim.height,
	frameCtrl->input.mctl_info.picture_dim.width,
	frameCtrl->input.mctl_info.picture_dim.height);
  //If frame_num==1 call single frame hdr
  //else if frame_num==3 call two frame hdr (x, 0.5x and 2x)
  //change it later?
  if(frameCtrl->input.mctl_info.main_img_format != FRAME_PROC_H2V2 ||
    frameCtrl->input.mctl_info.thumb_img_format != FRAME_PROC_H2V2 ){
    CDBG_HIGH("%s: Unsupported image formats %d",__func__,
	  frameCtrl->input.mctl_info.main_img_format);
	  return HDR_ERROR;
  }
  if (frameCtrl->input.mctl_info.num_main_img == 1 &&
    frameCtrl->input.mctl_info.num_thumb_img == 1) {
    //Get the pIn config settings for thumbnail
    pIn->imageWidth=frameCtrl->input.mctl_info.thumbnail_dim.width;
    pIn->imageHeight= frameCtrl->input.mctl_info.thumbnail_dim.height;
    pIn->subSampleFormat = HDR_H2V2;
    pIn->chromaOrder=YCRCB;
    tempFirstFrame= &(frameCtrl->input.mctl_info.thumb_img_frame[0]);
    pIn->pHdrBuffer1Y=
      (uint8_t*)((uint8_t*)tempFirstFrame->mp[0].vaddr +
      tempFirstFrame->mp[0].data_offset);
    pIn->pHdrBuffer1C=
      (uint8_t*)((uint8_t*)tempFirstFrame->mp[1].vaddr+
      tempFirstFrame->mp[1].data_offset);
    //Call HDR single frame core
    hdrSingleFrameCore(pIn,&rc);

    //Get the pIn config settings for Main Img
    pIn->imageWidth=frameCtrl->input.mctl_info.picture_dim.width;
    pIn->imageHeight=frameCtrl->input.mctl_info.picture_dim.height;
	pIn->subSampleFormat=HDR_H2V2;
    pIn->chromaOrder=YCRCB;
    tempFirstFrame=&(frameCtrl->input.mctl_info.main_img_frame[0]);
    pIn->pHdrBuffer1Y=
      (uint8_t*)((uint8_t*)tempFirstFrame->mp[0].vaddr +
      tempFirstFrame->mp[0].data_offset);
    pIn->pHdrBuffer1C=
      (uint8_t*)((uint8_t*)tempFirstFrame->mp[1].vaddr+
      tempFirstFrame->mp[1].data_offset);
    //Call HDR single frame core
    hdrSingleFrameCore(pIn,&rc);

  } else if (frameCtrl->input.mctl_info.num_main_img == 3 &&
    frameCtrl->input.mctl_info.num_thumb_img == 3) {
    /* Apply 2 frame HDR */
    CDBG_HIGH("Entered Multi frame HDR");
    hdr_config_2frame(frameCtrl,hdrCtrl,THUMBNAIL);
    hdrTwoFrameCore(pIn,&rc);
    hdrCtrl->mParamterStruct.mCalculatedExposureRatioG=
      pIn->calculatedExposureRatioG;
	//Output is always written to 2x frame(which is the second frame), so invert them
    //if outputflag is requested in 0.5x frame(frame 1)
    /*memcpy(pIn->pHdrBuffer2Y,pIn->pHdrBuffer1Y,
      (frameCtrl->input.mctl_info.thumbnail_dim.width*
      frameCtrl->input.mctl_info.thumbnail_dim.height));
    memcpy(pIn->pHdrBuffer2C,pIn->pHdrBuffer1C,
      (frameCtrl->input.mctl_info.thumbnail_dim.width*
      (frameCtrl->input.mctl_info.thumbnail_dim.height>>1))); */
    hdr_config_2frame(frameCtrl,hdrCtrl,MAINIMG);
    hdrTwoFrameCore(pIn,&rc);
	/* Output is written to 2x frames which is hal frame*/
    /*memcpy(pIn->pHdrBuffer2Y,pIn->pHdrBuffer1Y,
      (frameCtrl->input.mctl_info.picture_dim.width*
      frameCtrl->input.mctl_info.picture_dim.height));
    memcpy(pIn->pHdrBuffer2C,pIn->pHdrBuffer1C,
      (frameCtrl->input.mctl_info.picture_dim.width*
      (frameCtrl->input.mctl_info.picture_dim.height>>1))); */
    frameCtrl->output.hdr_d.hdr_output_buf_num = 1;
  } else
    CDBG_HIGH("HDR Failed as there is not enough snap n thumb imgs");
  return rc;
}
/*===========================================================================

Function           : hdr_config_2frame

Description        : configure 2 frame
============================================================================= */
int hdr_config_2frame(frame_proc_t *frameCtrl,hdr_t *hdrCtrl,image_type type)
{
  hdr_config_t *pIn = &(hdrCtrl->structHdrConfig);
  struct msm_pp_frame *hdr_frame[MAX_HDR_NUM_FRAMES];
  switch (type) {
  case THUMBNAIL :
    CDBG_HIGH("%s E thumbnail",__func__);
    pIn->imageWidth=frameCtrl->input.mctl_info.thumbnail_dim.width;
    pIn->imageHeight=frameCtrl->input.mctl_info.thumbnail_dim.height;
    pIn->thumbMainIndicator=1;
    pIn->calculatedExposureRatioG=0;
    hdr_frame[0] = &(frameCtrl->input.mctl_info.thumb_img_frame[1]);
    hdr_frame[1] = &(frameCtrl->input.mctl_info.thumb_img_frame[2]);
    break;
  case MAINIMG:
    CDBG_HIGH("%s main img  ratio : %d E",__func__,hdrCtrl->mParamterStruct.mCalculatedExposureRatioG);
    pIn->imageWidth=frameCtrl->input.mctl_info.picture_dim.width;
    pIn->imageHeight=frameCtrl->input.mctl_info.picture_dim.height;
    pIn->thumbMainIndicator=0;
    pIn->calculatedExposureRatioG=hdrCtrl->mParamterStruct.mCalculatedExposureRatioG;
    hdr_frame[0] = &(frameCtrl->input.mctl_info.main_img_frame[1]);
    hdr_frame[1] = &(frameCtrl->input.mctl_info.main_img_frame[2]);
    break;
  default:
    return -1;
  }
  //Get the pIn config settings
  pIn->subSampleFormat=HDR_H2V2;
  pIn->chromaOrder=YCRCB;
  pIn->maxLag=200;
  //Set the gamm tables
  pIn->pRedGammaInvTable=hdrCtrl->mParamterStruct.mpRedInverseGammatable;
  pIn->pGreenGammaInvTable=hdrCtrl->mParamterStruct.mpGreenInverseGammatable;
  pIn->pBlueGammaInvTable=hdrCtrl->mParamterStruct.mpBlueInverseGammatable;
  pIn->pRedGammaTable=hdrCtrl->mParamterStruct.mpRedNewGammatable;
  pIn->pGreenGammaTable=hdrCtrl->mParamterStruct.mpGreenNewGammatable;
  pIn->pBlueGammaTable=hdrCtrl->mParamterStruct.mpBlueNewGammatable;
  pIn->pHdrBuffer2Y=
    (uint8_t*)((uint8_t*)hdr_frame[0]->mp[0].vaddr+ hdr_frame[0]->mp[0].data_offset);
  pIn->pHdrBuffer2C=
    (uint8_t*)((uint8_t*)hdr_frame[0]->mp[1].vaddr+hdr_frame[0]->mp[1].data_offset);
  pIn->pHdrBuffer1Y=
    (uint8_t*)((uint8_t*)hdr_frame[1]->mp[0].vaddr+ hdr_frame[1]->mp[0].data_offset);
  pIn->pHdrBuffer1C=
    (uint8_t*)((uint8_t*)hdr_frame[1]->mp[1].vaddr+hdr_frame[1]->mp[1].data_offset);
	CDBG_HIGH("%s X",__func__);
  return 0;
}
/*===========================================================================

Function           : hdr_stop

Description        : API to stop/abort the algoritm executing

Input parameter(s) :

Output parameter(s):

Return Value       : none

Side Effects       : None

=========================================================================== */

void hdr_stop ( hdr_t *hdrCtrl)
{
  //free memory
  if (hdrCtrl->mParamterStruct.mpRedInverseGammatable!=NULL) {
    free(hdrCtrl->mParamterStruct.mpRedInverseGammatable);
    hdrCtrl->mParamterStruct.mpRedInverseGammatable = NULL;
  }
  if (hdrCtrl->mParamterStruct.mpGreenInverseGammatable!=NULL) {
    free(hdrCtrl->mParamterStruct.mpGreenInverseGammatable);
    hdrCtrl->mParamterStruct.mpGreenInverseGammatable=NULL;
  }
  if (hdrCtrl->mParamterStruct.mpBlueInverseGammatable!=NULL) {
    free(hdrCtrl->mParamterStruct.mpBlueInverseGammatable);
    hdrCtrl->mParamterStruct.mpBlueInverseGammatable=NULL;
  }
  if (hdrCtrl->mParamterStruct.mpRedNewGammatable!=NULL) {
    free(hdrCtrl->mParamterStruct.mpRedNewGammatable);
    hdrCtrl->mParamterStruct.mpRedNewGammatable = NULL;
  }
  if (hdrCtrl->mParamterStruct.mpGreenNewGammatable!=NULL) {
    free(hdrCtrl->mParamterStruct.mpGreenNewGammatable);
    hdrCtrl->mParamterStruct.mpGreenNewGammatable=NULL;
  }
  if (hdrCtrl->mParamterStruct.mpBlueNewGammatable!=NULL) {
    free(hdrCtrl->mParamterStruct.mpBlueNewGammatable);
    hdrCtrl->mParamterStruct.mpBlueNewGammatable=NULL;
  }
  return;
}

/*===========================================================================

Function           : hdr_calculate_gamma

Description        : API to program gamma table accordingly

Input parameter(s) :

Output parameter(s):

Return Value       : true/false

Side Effects       : None

=========================================================================== */
int hdr_calculate_gammatbl(void *Ctrl, hdr_t *hdrCtrl)
{
  frame_proc_t * frameCtrl = (frame_proc_t *)Ctrl;
  int rc;
  //Program  gamma tables accordingly
  if ((hdrCtrl->pGammaTableStruct.gamma_t==GAMMA_TBL_ALL)) {
    //allocate memory for inverse gamma and new gamma correction tables
    hdrCtrl->mParamterStruct.mpRedInverseGammatable=
      (uint32_t*)malloc(256*sizeof(uint32_t));
    hdrCtrl->mParamterStruct.mpGreenInverseGammatable=
      (uint32_t*)malloc(256*sizeof(uint32_t));
    hdrCtrl->mParamterStruct.mpBlueInverseGammatable=
      (uint32_t*)malloc(256*sizeof(uint32_t));
    hdrCtrl->mParamterStruct.mpRedNewGammatable=
      (uint32_t*)malloc(4096*sizeof(uint32_t));
    hdrCtrl->mParamterStruct.mpGreenNewGammatable=
      (uint32_t*)malloc(4096*sizeof(uint32_t));
    hdrCtrl->mParamterStruct.mpBlueNewGammatable=
      (uint32_t*)malloc(4096*sizeof(uint32_t));

    if (hdrCtrl->mParamterStruct.mpRedInverseGammatable==NULL ||
      hdrCtrl->mParamterStruct.mpGreenInverseGammatable==NULL||
      hdrCtrl->mParamterStruct.mpBlueInverseGammatable==NULL||
      hdrCtrl->mParamterStruct.mpRedNewGammatable==NULL||
      hdrCtrl->mParamterStruct.mpGreenNewGammatable==NULL||
      hdrCtrl->mParamterStruct.mpBlueNewGammatable==NULL) {
      if (hdrCtrl->mParamterStruct.mpRedInverseGammatable!=NULL) {
        free(hdrCtrl->mParamterStruct.mpRedInverseGammatable);
        hdrCtrl->mParamterStruct.mpRedInverseGammatable=NULL;
      }
      if (hdrCtrl->mParamterStruct.mpGreenInverseGammatable!=NULL) {
        free(hdrCtrl->mParamterStruct.mpGreenInverseGammatable);
        hdrCtrl->mParamterStruct.mpGreenInverseGammatable=NULL;
      }
      if (hdrCtrl->mParamterStruct.mpBlueInverseGammatable!=NULL) {
        free(hdrCtrl->mParamterStruct.mpBlueInverseGammatable);
        hdrCtrl->mParamterStruct.mpBlueInverseGammatable=NULL;
      }
      if (hdrCtrl->mParamterStruct.mpRedNewGammatable!=NULL) {
        free(hdrCtrl->mParamterStruct.mpRedNewGammatable);
        hdrCtrl->mParamterStruct.mpRedNewGammatable=NULL;
      }
      if (hdrCtrl->mParamterStruct.mpGreenNewGammatable!=NULL) {
        free(hdrCtrl->mParamterStruct.mpGreenNewGammatable);
        hdrCtrl->mParamterStruct.mpGreenNewGammatable=NULL;
      }
      if (hdrCtrl->mParamterStruct.mpBlueNewGammatable!=NULL) {
        free(hdrCtrl->mParamterStruct.mpBlueNewGammatable);
        hdrCtrl->mParamterStruct.mpBlueNewGammatable=NULL;
      }
      CDBG_HIGH("Gamma table malloc failed for GAMMA_TABLE_ALL");
      return -1;
    }

    //For R
    rc=hdrCalculateInverseGamma(&(hdrCtrl->pGammaTableStruct),
      hdrCtrl->mParamterStruct.mpRedInverseGammatable);
    if (rc!=HDR_SUCESS){
      CDBG_HIGH("%s calculate inverse red failed",__func__);
      return -1;
    }
    hdrCalculateNewGammaTable(&(hdrCtrl->pGammaTableStruct),
      hdrCtrl->mParamterStruct.mpRedNewGammatable);
    //For G
    memcpy(hdrCtrl->mParamterStruct.mpGreenInverseGammatable,hdrCtrl->mParamterStruct.mpRedInverseGammatable,256*sizeof(uint32_t));
    memcpy(hdrCtrl->mParamterStruct.mpGreenNewGammatable,hdrCtrl->mParamterStruct.mpRedNewGammatable,4096*sizeof(uint32_t));
    //For B
    memcpy(hdrCtrl->mParamterStruct.mpBlueInverseGammatable,hdrCtrl->mParamterStruct.mpRedInverseGammatable,256*sizeof(uint32_t));
    memcpy(hdrCtrl->mParamterStruct.mpBlueNewGammatable,hdrCtrl->mParamterStruct.mpRedNewGammatable,4096*sizeof(uint32_t));

  }  //else if ((hdr->hdrCtrl->pGammaTableStruct.gamma_t==GAMMA_TBL_ALL))
  else {
    if ((hdrCtrl->pGammaTableStruct.gamma_t==GAMMA_TBL_R)) {
      //For R
      hdrCtrl->mParamterStruct.mpRedInverseGammatable=
        (uint32_t*)malloc(256*sizeof(uint32_t));
      hdrCtrl->mParamterStruct.mpRedNewGammatable=
        (uint32_t*)malloc(4096*sizeof(uint32_t));
      if (hdrCtrl->mParamterStruct.mpRedInverseGammatable==NULL ||
        hdrCtrl->mParamterStruct.mpRedNewGammatable==NULL) {
        if (hdrCtrl->mParamterStruct.mpRedInverseGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpRedInverseGammatable);
          hdrCtrl->mParamterStruct.mpRedInverseGammatable=NULL;
        }
        if (hdrCtrl->mParamterStruct.mpGreenInverseGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpGreenInverseGammatable);
          hdrCtrl->mParamterStruct.mpGreenInverseGammatable=NULL;
        }
        if (hdrCtrl->mParamterStruct.mpBlueInverseGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpBlueInverseGammatable);
          hdrCtrl->mParamterStruct.mpBlueInverseGammatable=NULL;
        }
        if (hdrCtrl->mParamterStruct.mpRedNewGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpRedNewGammatable);
          hdrCtrl->mParamterStruct.mpRedNewGammatable=NULL;
        }
        if (hdrCtrl->mParamterStruct.mpGreenNewGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpGreenNewGammatable);
          hdrCtrl->mParamterStruct.mpGreenNewGammatable=NULL;
        }
        if (hdrCtrl->mParamterStruct.mpBlueNewGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpBlueNewGammatable);
          hdrCtrl->mParamterStruct.mpBlueNewGammatable=NULL;
        }
        return -1;
      }
      rc = hdrCalculateInverseGamma(&(hdrCtrl->pGammaTableStruct),
        hdrCtrl->mParamterStruct.mpRedInverseGammatable);
    if (rc!=HDR_SUCESS){
      CDBG_HIGH("%s calculate inverse red 1 failed",__func__);
      return -1;
    }
      hdrCalculateNewGammaTable(&(hdrCtrl->pGammaTableStruct),
        hdrCtrl->mParamterStruct.mpRedNewGammatable);
    } else if (hdrCtrl->pGammaTableStruct.gamma_t==GAMMA_TBL_G) {
      //For G
      hdrCtrl->mParamterStruct.mpGreenInverseGammatable=
        (uint32_t*)malloc(256*sizeof(uint32_t));
      hdrCtrl->mParamterStruct.mpGreenNewGammatable=
        (uint32_t*)malloc(4096*sizeof(uint32_t));
      if (hdrCtrl->mParamterStruct.mpGreenInverseGammatable==NULL ||
        hdrCtrl->mParamterStruct.mpGreenNewGammatable==NULL) {
        if (hdrCtrl->mParamterStruct.mpGreenInverseGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpGreenInverseGammatable);
          hdrCtrl->mParamterStruct.mpGreenInverseGammatable=NULL;
        }
        if (hdrCtrl->mParamterStruct.mpGreenNewGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpGreenNewGammatable);
          hdrCtrl->mParamterStruct.mpGreenNewGammatable=NULL;
        }
        return -1;
      }
      rc =hdrCalculateInverseGamma(&(hdrCtrl->pGammaTableStruct),
        hdrCtrl->mParamterStruct.mpGreenInverseGammatable);
    if (rc!=HDR_SUCESS){
      CDBG_HIGH("%s calculate inverse green failed",__func__);
      return -1;
    }
      hdrCalculateNewGammaTable(&(hdrCtrl->pGammaTableStruct),
        hdrCtrl->mParamterStruct.mpGreenNewGammatable);
    } else if (hdrCtrl->pGammaTableStruct.gamma_t==GAMMA_TBL_B) {
      //For B
      hdrCtrl->mParamterStruct.mpBlueInverseGammatable=
        (uint32_t*)malloc(256*sizeof(uint32_t));
      hdrCtrl->mParamterStruct.mpBlueNewGammatable=
        (uint32_t*)malloc(4096*sizeof(uint32_t));
      if (hdrCtrl->mParamterStruct.mpBlueInverseGammatable==NULL ||
        hdrCtrl->mParamterStruct.mpBlueNewGammatable==NULL) {
        if (hdrCtrl->mParamterStruct.mpBlueInverseGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpBlueInverseGammatable);
          hdrCtrl->mParamterStruct.mpBlueInverseGammatable=NULL;
        }
        if (hdrCtrl->mParamterStruct.mpBlueNewGammatable!=NULL) {
          free(hdrCtrl->mParamterStruct.mpBlueNewGammatable);
          hdrCtrl->mParamterStruct.mpBlueNewGammatable=NULL;
        }
        return -1;
      }
      hdrCalculateInverseGamma(&(hdrCtrl->pGammaTableStruct),
        hdrCtrl->mParamterStruct.mpBlueInverseGammatable);
      hdrCalculateNewGammaTable(&(hdrCtrl->pGammaTableStruct),
        hdrCtrl->mParamterStruct.mpBlueNewGammatable);

    }  //end if ((hdr->hdrCtrl->pGammaTableStruct.gamma_t==GAMMA_TBL_B))
  }  //end if ((hdr->hdrCtrl->pGammaTableStruct.gamma_t==GAMMA_TBL_ALL))
  return 0;
}
/*===========================================================================

Function           : hdrCalculateInverseGamma

Description        : Internal function to calculate and set inverse gamma tables

Input parameter(s) :

Output parameter(s):

Return Value       : none

Side Effects       : None

=========================================================================== */

int hdrCalculateInverseGamma(hdr_gamma_table_struct_t *pGammaTableStruct,
  uint32_t * inverseGammatable)
{
  uint16_t * pGammaTableValues;
  uint16_t gammavalue,gammadelta;
  uint32_t tempnew,tempold,i,j,k,index,numberOfGammaMatches,totalCount;

  pGammaTableValues=(uint16_t*)(pGammaTableStruct->gamma_tbl);
  numberOfGammaMatches=0;
  totalCount=0;
  k=0;
  tempold=0;
  tempnew=0;
  //Interpolate gamma table
  for (i=0;i<64;i++) {
    gammavalue=(*pGammaTableValues) & 0xff;
    gammadelta=(*pGammaTableValues++) >> 8;
	CDBG_HIGH("Gamma value + delta %d,  %d", gammavalue, gammadelta);
    for (j=0;j<16;j++) {
      index=i*16+j;
      tempnew=gammavalue + ((gammadelta*j+8)>>4);
      tempnew=MIN(tempnew,255);
      if (index!=0) {
        if (tempnew==tempold) {
          numberOfGammaMatches+=index;
          totalCount=totalCount+1;
        } else {
          if (k<256) {
            inverseGammatable[k]=(numberOfGammaMatches+(totalCount>>1))/totalCount;
            k=k+1;
            numberOfGammaMatches=0;
            totalCount=0;
            numberOfGammaMatches+=index;
            totalCount=totalCount+1;
          } else {
            CDBG_HIGH("hdrSetParameter: Gamma table inverse calculation failed \n");
            return HDR_ERROR;
          }
        }
        tempold=tempnew;
      } else {
        tempold=tempnew;
        numberOfGammaMatches+=index;
        totalCount=totalCount+1;
      }

    }
  }
  inverseGammatable[0]=0;
  for (i=k;i<256;i++) {
    inverseGammatable[i]=(numberOfGammaMatches+(totalCount>>1))/totalCount;
  }
  inverseGammatable[255]=1023;
  return HDR_SUCESS;
}

/*===========================================================================

Function           : hdrCalculateNewGammaTable

Description        : Internal function to calculate new gamma tables

Input parameter(s) :

Output parameter(s):

Return Value       : none

Side Effects       : None

=========================================================================== */

void hdrCalculateNewGammaTable(hdr_gamma_table_struct_t *pGammaTableStruct,
  uint32_t * newGammatable)
{

  uint16_t *pGammaTableValues;
  uint16_t gammavalue,gammadelta=0;
  uint32_t i,j,index;

  pGammaTableValues=(uint16_t*)(pGammaTableStruct->gamma_tbl);

  //Interpolate gamma table 0-2047 values
  for (i=0;i<64;i++) {
    gammavalue=(*pGammaTableValues) & 0xff;
    gammadelta=(*pGammaTableValues++) >> 8;
    for (j=0;j<32;j++) {
      index=i*32+j;
      newGammatable[index]=(gammavalue<<2) + ((gammadelta*j+4)>>3);
      newGammatable[index]=MIN(newGammatable[index],1023);
    }
  }
  // we need to find out gamma value of 2048 index considering gamma table interpolated to 4096
  pGammaTableValues=(uint16_t*)(pGammaTableStruct->gamma_tbl);
  pGammaTableValues=pGammaTableValues+32;
  gammadelta=(*pGammaTableValues) >> 8;

  index=2048;
  //interpolate 2048-4095 values
  for (i=0;i<2048;i++) {
    newGammatable[index]=newGammatable[2047]+((gammadelta*i + 8)>>4);
    index++;
  }
  //Normalize gamma table
  for (i=0;i<4096;i++) {
    newGammatable[i]=newGammatable[i]*1023/newGammatable[4095];
  }
}
