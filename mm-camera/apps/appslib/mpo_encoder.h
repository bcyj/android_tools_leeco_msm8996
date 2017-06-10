/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef MMCAMERA_MPOE_ENCODER_H
#define MMCAMERA_MPOE_ENCODER_H

#include "camera.h"
#include "mpoe.h"
#include "jpege.h"
#include "mpoe.h"
#include "exif.h"
#include "camera_defs_i.h"

void mpo_event_handler(void*, jpeg_event_t event, void *p_arg);

int mpo_output_produced_handler(void*, void *, jpeg_buffer_t, uint8_t);

int8_t mpo_encoder_init(void);
int8_t mpo_encoder_encode(const cam_ctrl_dimension_t *,
  const uint8_t *, int, const uint8_t *, int snapshot_fd, common_crop_t *crop,
  exif_tags_info_t *exif_data, int exif_numEntries,
  const int32_t a_cbcroffset, cam_point_t* main_crop_offset,
  cam_point_t* thumb_crop_offset, int zsl_enable);

int8_t mpo_encoder_setMainImageQuality(uint32_t quality);
int8_t mpo_encoder_setThumbnailQuality(uint32_t quality);
int8_t mpo_encoder_setRotation(int rotation);
void mpo_encoder_join(void);
int8_t mpo_encoder_get_buffer_offset(
        uint32_t width, uint32_t height, uint32_t* p_y_offset,
  uint32_t* p_cbcr_offset, uint32_t* p_buf_size);
void mpo_encoder_set_api_info(uint8_t* main_buf);
#endif //MMCAMERA_JPEG_ENCODER_H
