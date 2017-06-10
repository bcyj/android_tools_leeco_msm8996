/*****************************************************************************/
/*                                                                           */
/*                 MPEG4 ASP Decoder on Cortex A8 Ittiam APIs                */
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
/*  File Name         : imp4d_cxa8_cxa8.h                                    */
/*                                                                           */
/*  Description       : This file contains all the necessary structure and   */
/*                      enumeration definitions needed for the Application   */
/*                      Program Interface(API) of the Ittiam MPEG4 ASP       */
/*                      Decoder on Cortex A8 - Neon platform                 */
/*                                                                           */
/*  List of Functions : imp4d_cxa8_api_function                              */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         26 08 2010   100239(RCY)     Draft                                */
/*                                                                           */
/*****************************************************************************/

#ifndef _IMP4D_CXA8_H
#define _IMP4D_CXA8_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iv.h"
#include "ivd.h"
/*****************************************************************************/
/* Constant Macros                                                           */
/*****************************************************************************/

#define IMP4D_CXA8_EXPORT_MEM_RECORDS   22

/*****************************************************************************/
/* Function Macros                                                           */
/*****************************************************************************/
#define IS_IVD_CONCEALMENT_APPLIED(x)       (x & (1 << IVD_APPLIEDCONCEALMENT))
#define IS_IVD_INSUFFICIENTDATA_ERROR(x)    (x & (1 << IVD_INSUFFICIENTDATA))
#define IS_IVD_CORRUPTEDDATA_ERROR(x)       (x & (1 << IVD_CORRUPTEDDATA))
#define IS_IVD_CORRUPTEDHEADER_ERROR(x)     (x & (1 << IVD_CORRUPTEDHEADER))
#define IS_IVD_UNSUPPORTEDINPUT_ERROR(x)    (x & (1 << IVD_UNSUPPORTEDINPUT))
#define IS_IVD_UNSUPPORTEDPARAM_ERROR(x)    (x & (1 << IVD_UNSUPPORTEDPARAM))
#define IS_IVD_FATAL_ERROR(x)               (x & (1 << IVD_FATALERROR))
#define IS_IVD_INVALID_BITSTREAM_ERROR(x)   (x & (1 << IVD_INVALID_BITSTREAM))
#define IS_IVD_INCOMPLETE_BITSTREAM_ERROR(x) (x & (1 << IVD_INCOMPLETE_BITSTREAM))

/*****************************************************************************/
/* API Function Prototype                                                    */
/*****************************************************************************/
IV_API_CALL_STATUS_T imp4d_cxa8_api_function(iv_obj_t *ps_handle, void *pv_api_ip,void *pv_api_op);

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/
/* Codec Error codes for MPEG4 ASP Decoder                                   */

typedef enum {

    IMP4D_CXA8_VID_HDR_DEC_NUM_FRM_BUF_NOT_SUFFICIENT   = IVD_DUMMY_ELEMENT_FOR_CODEC_EXTENSIONS + 1,
    IMP4D_CXA8_IP_API_STRUCT_SIZE_INCORRECT,
    IMP4D_CXA8_OP_API_STRUCT_SIZE_INCORRECT,
    IMP4D_CXA8_VERS_BUF_PTR_NULL,
    IMP4D_CXA8_VERS_BUF_INSUFFICIENT,
    IMP4D_CXA8_CTL_SKIP_MODE_NOT_SUPPORTED,
    IMP4D_CXA8_CTL_DEC_MODE_NOT_SUPPORTED,
    IMP4D_CXA8_CTL_DISP_OUT_MODE_NOT_SUPPORTED,
	IMP4D_CXA8_CTL_DISP_WD_NOT_SUPPORTED,
    IMP4D_CXA8_FRM_DEC_FRAME_BUFFER_NOT_FREE,
    IMP4D_CXA8_FRM_DEC_VOP_START_CODE_NOT_FOUND,
    IMP4D_CXA8_GET_DISP_QUEUE_EMPTY,
    IMP4D_CXA8_GET_UD_CURRENT_UD_ID_NOT_VALID,
    IMP4D_CXA8_GET_UD_UD_SAVE_NOT_ENABLE,
    IMP4D_CXA8_INVALID_MOT_COMP_MODE,
    IMP4D_CXA8_VOP_TYPE_NOT_IPB,
    IMP4D_CXA8_SPRITE_BRIGHTNESS_CONTROL_NOT_SUPPORTED,
    IMP4D_CXA8_INVALID_VOP_QUANT,
    IMP4D_CXA8_SVH_VOP_NOT_IP,
    IMP4D_CXA8_SVH_SOURCE_FORMAT_INVALID,
    IMP4D_CXA8_OBMC_NOT_SUPORTED,
    IMP4D_CXA8_NOT_8_BIT_PIXEL_NOT_SUPPORTED,
    IMP4D_CXA8_NEW_PRED_ENABLE_NOT_SUPPORTED,
    IMP4D_CXA8_SCALABILITY_NOT_SUPPORTED,
    IMP4D_CXA8_NON_RECT_VOL_SHAPE_NOT_SUPPORTED,
    IMP4D_CXA8_SVOP_NOT_SUPPORTED,
    IMP4D_CXA8_FEATURE_NOT_SUPPORTED_IN_SEQUENCE_HEADER,
    IMP4D_CXA8_MARKER_BIT_NOT_FOUND,
    IMP4D_CXA8_MARKER_BIT_NOT_FOUND_RVLC,
    IMP4D_CXA8_ZERO_BIT_NOT_FOUND_SVH,
    IMP4D_CXA8_MARKER_BIT_NOT_FOUND_SEQUENCE_HEADER,
    IMP4D_CXA8_RESYNC_MARKER_NOT_FOUND,
    IMP4D_CXA8_VIDEO_OBJECT_LAYER_START_CODE_NOT_FOUND,
    IMP4D_CXA8_BIT_RATE_SET_TO_ZERO,
    IMP4D_CXA8_BUFFER_SIZE_SET_TO_ZERO,
    IMP4D_CXA8_NUM_VLD_COEF_EXCEED_64,
    IMP4D_CXA8_VOP_START_CODE_NOT_FOUND,
    IMP4D_CXA8_GOV_START_CODE_NOT_FOUND,
    IMP4D_CXA8_INVALID_MV_TABLE_ACCESS,
    IMP4D_CXA8_USER_DATA_START_CODE_NOT_FOUND,
    IMP4D_CXA8_INVALID_DC_VALUE_SVH,
    IMP4D_CXA8_FOUR_RESERVED_ZERO_BITS_NOT_FOUND,
    IMP4D_CXA8_INVALID_PRED_TYPE,
    IMP4D_CXA8_BVOP_WITHOUT_REF_FRAME,
    IMP4D_CXA8_INVALID_MOTION_MARKER,
    IMP4D_CXA8_INVALID_FCODE,
    IMP4D_CXA8_INVALID_GOB,
    IMP4D_CXA8_SEQUENCE_START_CODE_NOT_FOUND,
    IMP4D_CXA8_INVALID_PARAM_VALUE,
    IMP4D_CXA8_INVALID_PACKET,
    IMP4D_CXA8_VISUAL_OBJECT_START_CODE_NOT_FOUND,
    IMP4D_CXA8_INVALID_HUFFMAN_CODE,
    IMP4D_CXA8_INVALID_RVLC_CODE,
    IMP4D_CXA8_INVALID_HUFFMAN_CODE_DC_COEFF,
    IMP4D_CXA8_INVALID_HUFFMAN_CODE_DCT_COEFF,
    IMP4D_CXA8_ERROR_ESC_CODE_SEQ_TYPE3,
    IMP4D_CXA8_INVALID_LEVEL_SHORT_VIDEO_HEADER,
    IMP4D_CXA8_INVALID_HUFFMAN_CODE_FOR_RVLC,
    IMP4D_CXA8_GMC_SVOP_NOT_SUPPORTED,
    IMP4D_CXA8_VISUAL_OBJECT_SEQ_START_CODE_NOT_FOUND,
    IMP4D_CXA8_INVALID_CBPY,
    IMP4D_CXA8_NON_ZERO_STUFFING_BIT,
    IMP4D_CXA8_STATIC_SVOP_NOT_SUPPORTED,
    IMP4D_CXA8_GRAYSCALE_VOP_SHAPE_NOT_SUPPORTED,
    IMP4D_CXA8_UNSPECIFIED_MB,
    IMP4D_CXA8_ESCAPE_FOLLOWS_ESCAPE,
    IMP4D_CXA8_INTER_BLOCK_WITHOUT_IFRAME,
    IMP4D_CXA8_VOL_WD_HT_NOT_MULTIPLE_OF_2,
    IMP4D_CXA8_DISPLAY_WIDTH_NOT_SUPPORTED
}IMP4D_CXA8_ERROR_CODES_T;

/*****************************************************************************/
/* Extended Structures                                                       */
/*****************************************************************************/
typedef enum {
	IMP4D_CXA8_CMD_CTL_DISABLE_QPEL = IVD_CMD_CTL_CODEC_SUBCMD_START,
	IMP4D_CXA8_CMD_CTL_SET_NUM_CORES
}IMP4D_CXA8_CMD_CTL_SUB_CMDS;


/*****************************************************************************/
/*  Get Number of Memory Records                                             */
/*****************************************************************************/


typedef struct {
    iv_num_mem_rec_ip_t                    s_ivd_num_mem_rec_ip_t;
}imp4d_cxa8_num_mem_rec_ip_t;


typedef struct{
    iv_num_mem_rec_op_t                    s_ivd_num_mem_rec_op_t;
}imp4d_cxa8_num_mem_rec_op_t;


/*****************************************************************************/
/*  Fill Memory Records                                                      */
/*****************************************************************************/


typedef struct {
    iv_fill_mem_rec_ip_t                   s_ivd_fill_mem_rec_ip_t;
	/* Flag to enable sharing of reference buffers between decoder
	and application */

    UWORD32									u4_share_disp_buf;

    /* format in which codec has to give out frame data for display */
    IV_COLOR_FORMAT_T                       e_output_format;

}imp4d_cxa8_fill_mem_rec_ip_t;


typedef struct{
    iv_fill_mem_rec_op_t                   s_ivd_fill_mem_rec_op_t;
}imp4d_cxa8_fill_mem_rec_op_t;

/*****************************************************************************/
/*  Retrieve Memory Records                                                  */
/*****************************************************************************/


typedef struct {
    iv_retrieve_mem_rec_ip_t               s_ivd_retrieve_mem_rec_ip_t;
}imp4d_cxa8_retrieve_mem_rec_ip_t;


typedef struct{
    iv_retrieve_mem_rec_op_t               s_ivd_retrieve_mem_rec_op_t;
}imp4d_cxa8_retrieve_mem_rec_op_t;


/*****************************************************************************/
/*   Initialize decoder                                                      */
/*****************************************************************************/


typedef struct {
    ivd_init_ip_t                           s_ivd_init_ip_t;
	/* Flag to enable sharing of reference buffers between decoder
	and application */
    UWORD32									u4_share_disp_buf;

}imp4d_cxa8_init_ip_t;


typedef struct{
    ivd_init_op_t                           s_ivd_init_op_t;
}imp4d_cxa8_init_op_t;


/*****************************************************************************/
/*   Video Decode                                                            */
/*****************************************************************************/


typedef struct {
    ivd_video_decode_ip_t                   s_ivd_video_decode_ip_t;
}imp4d_cxa8_video_decode_ip_t;


typedef struct{
    ivd_video_decode_op_t                   s_ivd_video_decode_op_t;
}imp4d_cxa8_video_decode_op_t;


/*****************************************************************************/
/*   Get Display Frame                                                       */
/*****************************************************************************/


typedef struct
{
    ivd_get_display_frame_ip_t              s_ivd_get_display_frame_ip_t;
}imp4d_cxa8_get_display_frame_ip_t;


typedef struct
{
    ivd_get_display_frame_op_t              s_ivd_get_display_frame_op_t;
}imp4d_cxa8_get_display_frame_op_t;



/*****************************************************************************/
/*   Set Display Frame                                                       */
/*****************************************************************************/
typedef struct
{
    ivd_set_display_frame_ip_t              s_ivd_set_display_frame_ip_t;
}imp4d_cxa8_set_display_frame_ip_t;


typedef struct
{
    ivd_set_display_frame_op_t              s_ivd_set_display_frame_op_t;
}imp4d_cxa8_set_display_frame_op_t;

/*****************************************************************************/
/*   Release Display Buffers                                                 */
/*****************************************************************************/


typedef struct
{
    ivd_rel_display_frame_ip_t                  s_ivd_rel_display_frame_ip_t;
}imp4d_cxa8_rel_display_frame_ip_t;


typedef struct
{
    ivd_rel_display_frame_op_t                  s_ivd_rel_display_frame_op_t;
}imp4d_cxa8_rel_display_frame_op_t;

/*****************************************************************************/
/*   Video control  Flush                                                    */
/*****************************************************************************/


typedef struct{
    ivd_ctl_flush_ip_t                      s_ivd_ctl_flush_ip_t;
}imp4d_cxa8_ctl_flush_ip_t;


typedef struct{
    ivd_ctl_flush_op_t                      s_ivd_ctl_flush_op_t;
}imp4d_cxa8_ctl_flush_op_t;

/*****************************************************************************/
/*   Video control reset                                                     */
/*****************************************************************************/


typedef struct{
    ivd_ctl_reset_ip_t                      s_ivd_ctl_reset_ip_t;
}imp4d_cxa8_ctl_reset_ip_t;


typedef struct{
    ivd_ctl_reset_op_t                      s_ivd_ctl_reset_op_t;
}imp4d_cxa8_ctl_reset_op_t;


/*****************************************************************************/
/*   Video control  Set Params                                               */
/*****************************************************************************/


typedef struct {
    ivd_ctl_set_config_ip_t             s_ivd_ctl_set_config_ip_t;
}imp4d_cxa8_ctl_set_config_ip_t;


typedef struct{
    ivd_ctl_set_config_op_t             s_ivd_ctl_set_config_op_t;
}imp4d_cxa8_ctl_set_config_op_t;

/*****************************************************************************/
/*   Video control:Get Buf Info                                              */
/*****************************************************************************/


typedef struct{
    ivd_ctl_getbufinfo_ip_t             s_ivd_ctl_getbufinfo_ip_t;
}imp4d_cxa8_ctl_getbufinfo_ip_t;



typedef struct{
    ivd_ctl_getbufinfo_op_t             s_ivd_ctl_getbufinfo_op_t;
}imp4d_cxa8_ctl_getbufinfo_op_t;


/*****************************************************************************/
/*   Video control:Getstatus Call                                            */
/*****************************************************************************/


typedef struct{
    ivd_ctl_getstatus_ip_t                  s_ivd_ctl_getstatus_ip_t;
}imp4d_cxa8_ctl_getstatus_ip_t;



typedef struct{
    ivd_ctl_getstatus_op_t                  s_ivd_ctl_getstatus_op_t;
}imp4d_cxa8_ctl_getstatus_op_t;


/*****************************************************************************/
/*   Video control:Get Version Info                                          */
/*****************************************************************************/


typedef struct{
    ivd_ctl_getversioninfo_ip_t         s_ivd_ctl_getversioninfo_ip_t;
}imp4d_cxa8_ctl_getversioninfo_ip_t;



typedef struct{
    ivd_ctl_getversioninfo_op_t         s_ivd_ctl_getversioninfo_op_t;
}imp4d_cxa8_ctl_getversioninfo_op_t;

/*****************************************************************************/
/*   Video control:Disable Qpel                                  		 	 */
/*****************************************************************************/
typedef struct{
    UWORD32                                     u4_size;
    IVD_API_COMMAND_TYPE_T                      e_cmd;
    IVD_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
    /* u4_disable_qpel_level 0 : Does not disable qpel
       u4_disable_qpel_level 1 : Disables qpel in B pictures */
    UWORD32 									u4_disable_qpel_level;
}imp4d_cxa8_ctl_disable_qpel_ip_t;

typedef struct{
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;
}imp4d_cxa8_ctl_disable_qpel_op_t;


typedef struct{
    UWORD32                                     u4_size;
    IVD_API_COMMAND_TYPE_T                      e_cmd;
    IVD_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
    UWORD32 									u4_num_cores;
}imp4d_cxa8_ctl_set_num_cores_ip_t;

typedef struct{
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;
}imp4d_cxa8_ctl_set_num_cores_op_t;

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _IMP4D_CXA8_H */
