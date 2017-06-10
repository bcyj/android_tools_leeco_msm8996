/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef OMX_JPEG_EXT_H_
#define OMX_JPEG_EXT_H_

#include <OMX_Image.h>
#include "exif.h"
#include "omx_jpeg_common.h"

typedef struct omx_jpeg_pmem_info {
    int fd;
    int offset;
} omx_jpeg_pmem_info;

typedef struct omx_jpeg_exif_info_tag {
    exif_tag_id_t      tag_id;
    exif_tag_entry_t  tag_entry;

} omx_jpeg_exif_info_tag;

typedef struct omx_jpeg_buffer_offset {
    int width;
    int height;
    int yOffset;
    int cbcrOffset;
    int totalSize;
    int paddedFrameSize;
} omx_jpeg_buffer_offset;

typedef struct omx_jpeg_mobicat {
    uint8_t      *mobicatData;
    int32_t       mobicatDataLength;

} omx_jpeg_mobicat;

#define OMX_JPEG_PREFIX "omx.qcom.jpeg.exttype."
#define OMX_JPEG_PREFIX_LENGTH 22

/*adding to enum also add to the char name array down*/
typedef enum {
    OMX_JPEG_EXT_START = 0x7F000000,
    OMX_JPEG_EXT_EXIF,
    OMX_JPEG_EXT_THUMBNAIL,
    OMX_JPEG_EXT_THUMBNAIL_QUALITY,
    OMX_JPEG_EXT_BUFFER_OFFSET,
    OMX_JPEG_EXT_ACBCR_OFFSET,
    OMX_JPEG_EXT_USER_PREFERENCES,
    OMX_JPEG_EXT_MOBICAT,
    OMX_JPEG_EXT_REGION,
    OMX_JPEG_EXT_IMAGE_TYPE,
    OMX_JPEG_EXT_END,
} omx_jpeg_ext_index;

extern char * omx_jpeg_ext_name[];
/*char * omx_jpeg_ext_name[] = {
    "start",
    "exif",
    "thumbnail",
    "thumbnail_quality",
    "buffer_offset",
    "acbcr_offset",
    "user_preferences",
    "region",
    "end"
};*/

/*assume main img scaling*/

typedef struct omx_jpeg_thumbnail {
    int width;
    int height;
    int scaling;
    int cropWidth;
    int cropHeight;
    int left;
    int top;
} omx_jpeg_thumbnail;

typedef struct omx_jpeg_thumbnail_quality {
    OMX_U32 nQFactor;
} omx_jpeg_thumbnail_quality;

typedef struct omx_jpeg_user_preferences {
    omx_jpeg_color_format color_format;
    omx_jpeg_color_format thumbnail_color_format;
    omx_jpeg_preference preference;
} omx_jpeg_user_preferences;

typedef struct omx_jpeg_region{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
}omx_jpeg_region;

typedef struct omx_jpeg_type{
  omx_jpeg_image_type image_type;
}omx_jpeg_type;

#endif /* OMX_JPEG_EXT_H_ */
