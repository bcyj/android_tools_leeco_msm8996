/*============================================================================
   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef MM_CAMERA_LIVESHOT_H
#define MM_CAMERA_LIVESHOT_H
#include "camera.h"
#include "jpeg_encoder.h"
#include "exif.h"
#include "camera_defs_i.h"

#ifndef LIVESHOT_STATUS
#define LIVESHOT_STATUS
typedef enum {
    LIVESHOT_SUCCESS,
    LIVESHOT_ENCODE_ERROR,
    LIVESHOT_UNKNOWN_ERROR,
}liveshot_status;
#endif // LIVESHOT_STATUS

int8_t set_liveshot_params(uint32_t a_width, uint32_t a_height, exif_tags_info_t *a_exif_data,
                         int a_exif_numEntries, uint8_t* a_out_buffer, uint32_t a_outbuffer_size);
int8_t cancel_liveshot(void);
void set_liveshot_frame(struct msm_frame* liveshot_frame);
#endif //MM_CAMERA_LIVESHOT_H
