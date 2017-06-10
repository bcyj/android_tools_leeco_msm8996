/*****************************************************************************/
/*                                                                           */
/*                 H.264  Encoder on Cortex A8 Ittiam APIs                */
/*                     ITTIAM SYSTEMS PVT LTD, BANGALORE                     */
/*                             COPYRIGHT(C) 2010                             */
/*                                                                           */
/*  This program  is  proprietary to  Ittiam  Systems  Private  Limited  and */
/*  is protected under Indian  Copyright Law as an unpublished work. Its use */
/*  and  disclosure  is  limited by  the terms  and  conditions of a license */
/*  agreement. It may not be copied or otherwise  reproduced or disclosed to */
/*  persons outside the licensee's organization except in accordance with the*/
/*  terms  and  conditions   of  such  an  agreement.  All  copies  and      */
/*  reproductions shall be the property of Ittiam Systems Private Limited and*/
/*  must bear this notice in its entirety.                                   */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  File Name         : ih264_cxa8.h                                    */
/*                                                                           */
/*  Description       : This file contains all the necessary structure and   */
/*                      enumeration definitions needed for the Application   */
/*                      Program Interface(API) of the Ittiam MPEG4        */
/*                      Encoder on Cortex A8 - Neon platform                 */
/*                                                                           */
/*  List of Functions : ih264e_cxa8_api_function                              */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         26 08 2010   100239(RCY)     Draft                                */
/*                                                                           */
/*****************************************************************************/

#ifndef _IH264_CXA8_H
#define _IH264_CXA8_H

#include "iv.h"
#include "ive.h"
/*****************************************************************************/
/* Constant Macros                                                           */
/*****************************************************************************/


/*****************************************************************************/
/* API Function Prototype                                                    */
/*****************************************************************************/
IV_API_CALL_STATUS_T ih264e_cxa8_api_function(iv_obj_t *ps_handle, void *pv_api_ip,void *pv_api_op);

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/
/* Coenc Error codes for H264 Encoder                                        */

typedef enum {

    IH264_LEVEL_DUMMY       = 0xFFFFFFFF,
    IH264E_LEVEL_10         = 0,
    IH264E_LEVEL_1B         = 1,
    IH264E_LEVEL_11         = 2,
    IH264E_LEVEL_12         = 3,
    IH264E_LEVEL_13         = 4,
    IH264E_LEVEL_20         = 5,
    IH264E_LEVEL_21         = 6,
    IH264E_LEVEL_22         = 7,
    IH264E_LEVEL_30         = 8,
    IH264E_LEVEL_31         = 9,
    IH264E_LEVEL_32         = 10,
    IH264E_LEVEL_40         = 11,
    IH264E_LEVEL_41         = 12,
    IH264E_LEVEL_42         = 13,
    IH264E_LEVEL_50         = 14,
    IH264E_LEVEL_51         = 15
}IH264E_LEVEL_T;

typedef enum {
    DUMMY_PROFILE = 0xFFFFFFFF,
    IH264E_PROFILE_BP = 0
}IH264E_PROFILE_T;

typedef enum
{
    IH264_ERROR_CODE_DUMMY                                      = 0xFFFFFFFF,
    IH264E_ERR_CONTROL_SET_PARAMS_WIDTH_HEIGHT_NOT_SUPPORTED    = IVE_ERR_DUMMY + 1,
    IH264E_ERR_PROFILE_LEVEL_CORRECTED                          = IVE_ERR_DUMMY + 2,
    IVE_ERR_CONTROL_SET_PARAMS_FRAME_LEVEL_QP_FLAG_INVAID       = IVE_ERR_DUMMY + 3,
    IVE_ERR_CONTROL_SET_PARAMS_NAL_SIZE_INSERTION_FLAG_INVALID  = IVE_ERR_DUMMY + 4,
    IVE_ERR_CONTROL_SET_PARAMS_VUI_SEQ_PARAM_FLAG_INVALID       = IVE_ERR_DUMMY + 5,
    IVE_ERR_CONTROL_SET_PARAMS_VIDEO_SIGNAL_TYPE_FLAG_INVALID   = IVE_ERR_DUMMY + 6,
    IVE_ERR_CONTROL_SET_PARAMS_COLOUR_DESCRIPTION_FLAG_INVALID  = IVE_ERR_DUMMY + 7,
    IVE_ERR_CONTROL_SET_PARAMS_VIDEO_FULL_RANGE_FLAG_INVALID    = IVE_ERR_DUMMY + 8,
    IVE_ERR_CONTROL_SET_PARAMS_VIDEO_VIDEO_FORMAT_INVALID       = IVE_ERR_DUMMY + 9,
    IH264E_ERR_INVALID_NMB_GROUP                                = IVE_ERR_DUMMY + 10,
    IH264E_ERR_SLICE_CUTOFF_VAL_WRONG                           = IVE_ERR_DUMMY + 11,
    IH264E_ERR_INVALID_DISABLE_DEBLOCK_LEVEL                    = IVE_ERR_DUMMY + 12,
    IH264E_ERR_INVALID_ME_QUALITY_FLAG                          = IVE_ERR_DUMMY + 13,
    IH264E_ERR_MIN_QP_WRONG                                     = IVE_ERR_DUMMY + 14,
    IH264E_ERR_MAX_QP_WRONG                                     = IVE_ERR_DUMMY + 15,
    IVE_ERR_INIT_INVALID_HALF_PEL_FLAG                          = IVE_ERR_DUMMY + 16,
    IVE_ERR_INIT_INVALID_INTRA_4X4_FLAG                         = IVE_ERR_DUMMY + 17,
    IVE_ERR_SET_PARAMS_NUM_OF_CORES_NOT_SUPPORTED               = IVE_ERR_DUMMY + 18,
    IH264E_ERR_OUTPUT_BITSTREAM_BUFFER_SIZE_EXCEEDED            = IVE_ERR_DUMMY + 19,
    IVE_ERR_INIT_INVALID_QPEL_FLAG                              = IVE_ERR_DUMMY + 20,
    IVE_ERR_INIT_REUSE_BUFFER_TYPE_INVALID                      = IVE_ERR_DUMMY + 21,
    IVE_ERR_INIT_RESUSE_BUFFER_SCALE_TOO_LARGE                  = IVE_ERR_DUMMY + 22,
    IVE_ERR_INIT_ALTERNATE_REFERENCE_FLAG_INVALID               = IVE_ERR_DUMMY + 23,
    IVE_ERR_INIT_FAST_SAD_FLAG_INVALID                          = IVE_ERR_DUMMY + 24
}IH264E_ERROR_CODES_T;


typedef enum {
    IH264_CXA8_CMD_CTL_DISABLE_DEBLOCK = IVE_CMD_CTL_CODEC_SUBCMD_START,
    IH264_CXA8_CMD_CTL_SET_NUM_CORES,
    IH264_CXA8_CMD_CTL_SET_ME_INFO_ENABLE,
}IH264_CXA8_CMD_CTL_SUB_CMDS;


/*****************************************************************************/
/* Extended Structures                                                       */
/*****************************************************************************/

/*****************************************************************************/
/*  Get Number of Memory Records                                             */
/*****************************************************************************/


typedef struct {
    iv_num_mem_rec_ip_t                    s_ive_num_mem_rec_ip_t;
}ih264_cxa8_num_mem_rec_ip_t;


typedef struct{
    iv_num_mem_rec_op_t                    s_ive_num_mem_rec_op_t;
}ih264_cxa8_num_mem_rec_op_t;


/*****************************************************************************/
/*  Fill Memory Records                                                      */
/*****************************************************************************/


typedef struct {
    iv_fill_mem_rec_ip_t                   s_ive_fill_mem_rec_ip_t;
    UWORD32                                u4_nmb_grp;
}ih264_cxa8_fill_mem_rec_ip_t;


typedef struct{
    iv_fill_mem_rec_op_t                   s_ive_fill_mem_rec_op_t;
}ih264_cxa8_fill_mem_rec_op_t;

/*****************************************************************************/
/*  Retrieve Memory Records                                                  */
/*****************************************************************************/


typedef struct {
    iv_retrieve_mem_rec_ip_t               s_ive_retrieve_mem_rec_ip_t;
}ih264_cxa8_retrieve_mem_rec_ip_t;


typedef struct{
    iv_retrieve_mem_rec_op_t               s_ive_retrieve_mem_rec_op_t;
}ih264_cxa8_retrieve_mem_rec_op_t;






/*****************************************************************************/
/*Structure for VUI and SEI params                                                               */
/*****************************************************************************/

typedef enum
{
  VF_COMPONENT,
  VF_PAL,
  VF_NTSC,
  VF_SECAM,
  VF_MAC,
  UNSPECIFIED_VIDEO_FORMAT,
  VF_RESERVED

}video_format;




typedef struct
{
    UWORD32                                 u4_size;
    IVE_API_COMMAND_TYPE_T                  e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T          e_sub_cmd;

    UWORD8 video_signal_type_present_flag;
    video_format    vui_video_format;
    UWORD8 video_full_range_flag;
    UWORD8 colour_description_present_flag;
    UWORD8 vui_colour_primaries;
    UWORD8  vui_transfer_characteristics;
    UWORD8 matrix_coefficients;
    UWORD8 uc_vui_seq_parameters_present_flag;

}ih264_cxa8_vui_ip_t;

typedef struct
{
    UWORD32                                 u4_size;
    UWORD32                                 u4_error_code;
}ih264_cxa8_vui_op_t;

/*****************************************************************************/
/*   Initialize encoder                                                      */
/*****************************************************************************/

typedef enum {
    DUMMY_SLICE = 0xFFFFFFFF,
    NO_SLICE = 0,
    MB_SLICE = 1,
    BIT_SLICE = 2
}IH264E_SLICE_MODE_T;

typedef struct {
    ive_init_ip_t                           s_ive_init_ip_t;
    IH264E_LEVEL_T                          e_level;
    IH264E_PROFILE_T                        e_profile;
    UWORD32                                 u4_nmb_grp;
}ih264_cxa8_init_ip_t;


typedef struct{
    ive_init_op_t                           s_ive_init_op_t;
}ih264_cxa8_init_op_t;


/*****************************************************************************/
/*   Video Decode                                                            */
/*****************************************************************************/


typedef struct {
    ive_video_encode_ip_t                   s_ive_video_encode_ip_t;
}ih264_cxa8_video_encode_ip_t;


typedef struct{
    ive_video_encode_op_t                   s_ive_video_encode_op_t;
}ih264_cxa8_video_encode_op_t;



/*****************************************************************************/
/*   Video control  Flush                                                    */
/*****************************************************************************/


typedef struct{
    ive_ctl_flush_ip_t                      s_ive_ctl_flush_ip_t;
}ih264_cxa8_ctl_flush_ip_t;


typedef struct{
    ive_ctl_flush_op_t                      s_ive_ctl_flush_op_t;
}ih264_cxa8_ctl_flush_op_t;

/*****************************************************************************/
/*   Video control reset                                                     */
/*****************************************************************************/


typedef struct{
    ive_ctl_reset_ip_t                      s_ive_ctl_reset_ip_t;
}ih264_cxa8_ctl_reset_ip_t;


typedef struct{
    ive_ctl_reset_op_t                      s_ive_ctl_reset_op_t;
}ih264_cxa8_ctl_reset_op_t;


/*****************************************************************************/
/*   Video control  Set Params                                               */
/*****************************************************************************/


typedef struct {
    ive_ctl_set_config_ip_t                 s_ive_ctl_set_config_ip_t;
    UWORD32                                 u4_high_quality_me;
    UWORD32                                 u4_enable_halfpel;
    UWORD32                                 u4_enable_i4x4;
    UWORD32                                 u4_intrarefresh_en;
    UWORD32                                 u4_enable_qpel;
    UWORD32                                 u4_reuse_buf_scale;
    UWORD32                                 u1_reuse_buf_type;
    UWORD8                                  u1_alt_ref_frame;
    UWORD8                                  u1_fast_sad;
}ih264_cxa8_ctl_set_config_ip_t;


typedef struct{
    ive_ctl_set_config_op_t             s_ive_ctl_set_config_op_t;
}ih264_cxa8_ctl_set_config_op_t;

/*****************************************************************************/
/*   Video control:Get Buf Info                                              */
/*****************************************************************************/


typedef struct{
    ive_ctl_getbufinfo_ip_t             s_ive_ctl_getbufinfo_ip_t;
}ih264_cxa8_ctl_getbufinfo_ip_t;



typedef struct{
    ive_ctl_getbufinfo_op_t             s_ive_ctl_getbufinfo_op_t;
}ih264_cxa8_ctl_getbufinfo_op_t;



/*****************************************************************************/
/*   Video control:Get Version Info                                          */
/*****************************************************************************/


typedef struct{
    ive_ctl_getversioninfo_ip_t         s_ive_ctl_getversioninfo_ip_t;
}ih264_cxa8_ctl_getversioninfo_ip_t;



typedef struct{
    ive_ctl_getversioninfo_op_t         s_ive_ctl_getversioninfo_op_t;
}ih264_cxa8_ctl_getversioninfo_op_t;

/*****************************************************************************/
/*   Video control:Set default params                                       */
/*****************************************************************************/


typedef struct{
    ive_ctl_setdefault_ip_t         s_ive_ctl_setdefault_ip_t;
}ih264_cxa8_ctl_setdefault_ip_t;



typedef struct{
    ive_ctl_setdefault_op_t         s_ive_ctl_setdefault_op_t;
}ih264_cxa8_ctl_setdefault_op_t;

/*****************************************************************************/
/*   Video control:Set deblock disable level                                 */
/*****************************************************************************/


typedef struct{
    UWORD32                                     u4_size;
    IVE_API_COMMAND_TYPE_T                      e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
    UWORD32                                     u4_disable_deblk_level;
}ih264_cxa8_ctl_disable_deblock_ip_t;

typedef struct{
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;
}ih264_cxa8_ctl_disable_deblock_op_t;

typedef struct{
    UWORD32                                     u4_size;
    IVE_API_COMMAND_TYPE_T                      e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
    UWORD32                                     u4_num_cores;
}ih264_cxa8_ctl_set_num_cores_ip_t;

typedef struct{
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;
}ih264_cxa8_ctl_set_num_cores_op_t;

typedef struct{
    UWORD32                                     u4_size;
    IVE_API_COMMAND_TYPE_T                      e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
    UWORD32                                      u4_me_info_enable;
}ih264_cxa8_ctl_set_me_info_enable_ip_t;

typedef struct{
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;
}ih264_cxa8_ctl_set_me_info_enable_op_t;

#endif /* _IH264_CXA8_H */
