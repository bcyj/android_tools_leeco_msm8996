/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef _ADSP_JPEGE_H
#define _ADSP_JPEGE_H
#ifndef __QAIC_HEADER
#define __QAIC_HEADER(ff) ff
#endif //__QAIC_HEADER

#ifndef __QAIC_HEADER_EXPORT
#define __QAIC_HEADER_EXPORT
#endif // __QAIC_HEADER_EXPORT

#ifndef __QAIC_HEADER_ATTRIBUTE
#define __QAIC_HEADER_ATTRIBUTE
#endif // __QAIC_HEADER_ATTRIBUTE

#ifndef __QAIC_IMPL
#define __QAIC_IMPL(ff) ff
#endif //__QAIC_IMPL

#ifndef __QAIC_IMPL_EXPORT
#define __QAIC_IMPL_EXPORT
#endif // __QAIC_IMPL_EXPORT

#ifndef __QAIC_IMPL_ATTRIBUTE
#define __QAIC_IMPL_ATTRIBUTE
#endif // __QAIC_IMPL_ATTRIBUTE
#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned char        uint8;
typedef unsigned short       uint16;
typedef unsigned long int    uint32;
typedef struct jpege_q6_huff_table_t jpege_q6_huff_table_t;
struct jpege_q6_huff_table_t {
   uint8 bits[17];
   uint8 values[256];
};
typedef struct jpege_q6_scale_cfg_t jpege_q6_scale_cfg_t;
struct jpege_q6_scale_cfg_t {
   uint32 crop_width;
   uint32 crop_height;
   uint32 h_offset;
   uint32 v_offset;
   uint32 output_width;
   uint32 output_height;
   uint8 enable;
};
//enum jpege_q6_color_format_t jpege_q6_color_format_t;
typedef enum {
    Q6_YCRCBLP_H2V2 = 0,
    Q6_YCBCRLP_H2V2 = 1,

    Q6_YCRCBLP_H2V1 = 2,
    Q6_YCBCRLP_H2V1 = 3,

    Q6_YCRCBLP_H1V2 = 4,
    Q6_YCBCRLP_H1V2 = 5,

    Q6_YCRCBLP_H1V1 = 6,
    Q6_YCBCRLP_H1V1 = 7,
    Q6_JPEG_COLOR_FORMAT_MAX,
   _32BIT_PLACEHOLDER_jpege_q6_color_format_t = 0x7fffffff
}jpege_q6_color_format_t;

typedef struct jpege_q6_enc_cfg_target_t jpege_q6_enc_cfg_target_t;
struct jpege_q6_enc_cfg_target_t {
   jpege_q6_huff_table_t luma_dc_huff_tbl;
   jpege_q6_huff_table_t chroma_dc_huff_tbl;
   jpege_q6_huff_table_t luma_ac_huff_tbl;
   jpege_q6_huff_table_t chroma_ac_huff_tbl;
   uint32 rotation;
   uint32 restart_interval;
   uint32 base_restart_marker;
   uint16 qtbl_0[64];
   uint16 qtbl_1[64];
   uint16 qtbl_2[64];
   uint16 qtbl_3[64];
   uint32 input_width;
   uint32 input_height;
   uint32 input_stride;
   uint32 output_MCUs;
   uint32 output_buffer_length;
   jpege_q6_color_format_t color_format;
   jpege_q6_scale_cfg_t scale_cfg;
   uint32 output_start_mcu_index;
};
/**
       * Start fatsrpc thread, Called only once for every camera session
       */
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_jpege_fastrpc_start)() __QAIC_HEADER_ATTRIBUTE;
/**
       * initalizes camera worker threads and queue
       */
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_jpege_init)() __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_jpege_q6_process)(const jpege_q6_enc_cfg_target_t* args, const uint8* plane_0_ptr, int plane_0_ptrLen, const uint8* plane_1_ptr, int plane_1_ptrLen, const uint8* plane_2_ptr, int plane_2_ptrLen, const uint8* plane_3_ptr, int plane_3_ptrLen, uint32* output_size, uint8* output_buffer_ptr, int output_buffer_ptrLen, uint32 thread_flag) __QAIC_HEADER_ATTRIBUTE;
/**
       * cleans up worker queue
       */
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_jpege_deinit)() __QAIC_HEADER_ATTRIBUTE;
#ifdef __cplusplus
}
#endif
#endif //_ADSP_JPEGE_H
