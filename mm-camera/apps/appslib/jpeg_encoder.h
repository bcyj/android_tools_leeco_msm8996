/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef MMCAMERA_JPEG_ENCODER_H
#define MMCAMERA_JPEG_ENCODER_H

#ifndef DISABLE_JPEG_ENCODING
#include "exif.h"
#include "jpege.h"
#include "camera.h"
#include "camera_defs_i.h"

void jpege_event_handler(void*, jpeg_event_t event, void *p_arg);

void jpege_output_produced_handler(void*, void *, jpeg_buffer_t);
int jpege_output_produced_handler2(void*, void *, jpeg_buffer_t, uint8_t);

int8_t jpeg_encoder_init(void);
int8_t jpeg_encoder_encode(const cam_ctrl_dimension_t *,
  const uint8_t *, int, const uint8_t *, int snapshot_fd, common_crop_t *crop,
  exif_tags_info_t *exif_data, int exif_numEntries,
  const int32_t a_cbcroffset, cam_point_t* main_crop_offset,
  cam_point_t* thumb_crop_offset, int zsl_enable);

int8_t jpeg_encoder_setMainImageQuality(uint32_t quality);
int8_t jpeg_encoder_setThumbnailQuality(uint32_t quality);
int8_t jpeg_encoder_setRotation(int rotation);
void jpeg_encoder_join(void);
int8_t jpeg_encoder_get_buffer_offset(uint32_t width, uint32_t height, uint32_t* p_y_offset,
  uint32_t* p_cbcr_offset, uint32_t* p_buf_size);
void jpeg_encoder_set_3D_info(cam_3d_frame_format_t format);
void jpege_set_phy_offset(uint32_t a_phy_offset);
int8_t jpeg_encoder_hw_inline_config(void);
#endif /* DISABLE_JPEG_ENCODING */
#endif //MMCAMERA_JPEG_ENCODER_H
