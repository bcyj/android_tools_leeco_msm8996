/*****************************************************************************/
/*                                                                           */
/*                         ITTIAM MP VIDEO ARM APIS                          */
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
/*  File Name         : ive.h                                                */
/*                                                                           */
/*  Description       : This file contains all the necessary structure and   */
/*                      enumeration definitions needed for the Application   */
/*                      Program Interface(API) of the Ittiam Video Encoders  */
/*                                                                           */
/*  List of Functions : None                                                 */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         04 02 2011   100325(SH)     Draft                                */
/*                                                                           */
/*****************************************************************************/

#ifndef _IVE_H
#define _IVE_H

/*****************************************************************************/
/* Constant Macros                                                           */
/*****************************************************************************/
#define IVE_MAX_IO_BUFFERS   3

/*Enums for IVE_QUALITY_CONFIG*/
typedef enum IVE_QUALITY_CONFIG
{
  IVE_QUALITY_DUMMY            = 0xFFFFFFFF,
  IVE_DEFAULT       = 0,
  IVE_HIGH_QUALITY  = 1,
  IVE_HIGH_SPEED    = 2,
  IVE_USER_DEFINED  = 3
}IVE_QUALITY_CONFIG;

typedef enum IVE_RATE_CONTROL_CONFIG
{
  IVE_RC_DUMMY	            = 0xFFFFFFFF,
  IVE_RC_CBR_LOW_DELAY     = 1,
  IVE_RC_VBR_STORAGE          = 2,
  IVE_RC_TWOPASS   = 3,
  IVE_RC_NONE      = 4,
  IVE_RC_CBR_NON_LOW_DELAY  = 5,
  IVE_RC_RATECONTROLPRESET_DEFAULT = IVE_RC_CBR_LOW_DELAY
}IVE_RATE_CONTROL_CONFIG;

typedef enum IVE_CONFIG_PARAMS
{
    IVE_DUMMY_CONFIG = 0xFFFFFFFF,
    IVE_CONFIG_DEFAULT = 0,
    IVE_CONFIG_USER_DEFINED = 1
}IVE_CONFIG_PARAMS;



/* IVE_API_COMMAND_TYPE_T:API command type                                   */
typedef enum {
	IVE_CMD_VIDEO_NA                          = 0xFFFFFFFF,
    IVE_CMD_VIDEO_CTL                         = IV_CMD_DUMMY_ELEMENT + 1,
    IVE_CMD_VIDEO_ENCODE,
}IVE_API_COMMAND_TYPE_T;

/* IVE_CONTROL_API_COMMAND_TYPE_T: Video Control API command type            */

typedef enum {
	IVE_CMD_NA								= 0xFFFFFFFF,
	IVE_CMD_CTL_SETPARAMS					= 0x1,
	IVE_CMD_CTL_RESET						= 0x2,
	IVE_CMD_CTL_SETDEFAULT					= 0x3,
	IVE_CMD_CTL_FLUSH						= 0x4,
	IVE_CMD_CTL_GETBUFINFO					= 0x5,
	IVE_CMD_CTL_GETVERSION					= 0x6,
	IVE_CMD_CTL_SET_VUI_PARAMS				= 0x7,
	IVE_CMD_CTL_CODEC_SUBCMD_START		    = 0x10,
}IVE_CONTROL_API_COMMAND_TYPE_T;

/* IVE_ERROR_BITS_T: A UWORD32 container will be used for reporting the error*/
/* code to the application. The first 8 bits starting from LSB have been     */
/* reserved for the codec to report internal error details. The rest of the  */
/* bits will be generic for all video encoders and each bit has an associated*/
/* meaning as mentioned below. The unused bit fields are reserved for future */
/* extenstions and will be zero in the current implementation                */
typedef enum {

    /* Bit 8 - Unsupported input parameter orconfiguration.                 */
    IVE_UNSUPPORTEDPARAM                        = 0x8,
    /* Bit 9 - Fatal error (stop the codec).If there is an                  */
    /* error and this bit is not set, the error is a recoverable one.        */
    IVE_FATALERROR                              = 0x9,

    IVE_ERROR_BITS_T_DUMMY_ELEMENT              = 0xFFFFFFFF
}IVE_ERROR_BITS_T;



typedef enum {
    IVE_ERR_IO_STRCUT_PTR_IS_NULL                               =1,
    IVE_ERR_INVALID_CMD_ID                                      =2,
    IVE_ERR_GET_NUM_REC_INVALID_IO_STRUCT_SIZE                  =3,
    IVE_ERR_FILL_NUM_REC_INVALID_IO_STRUCT_SIZE                 =4,
    IVE_ERR_FILL_NUM_REC_MEM_RECS_IS_NULL                       =5,
    IVE_ERR_FILL_MEM_REC_WD_HT_NOT_SUPPORTED                    =6,
    IVE_ERR_INIT_INVALID_CODEC_HANDLE                           =7,
    IVE_ERR_INIT_INVALID_IO_STRUCT_SIZE                         =8,
    IVE_ERR_INIT_INSUFFICIENT_MEM_RECS                          =9,
    IVE_ERR_INIT_UNSUPPORTED_INPUT_COLOUR_FORMAT                =10,
    IVE_ERR_INIT_UNSUPPORTED_RECON_COLOUR_FORMAT                =11,
    IVE_ERR_INIT_WD_HT_NOT_SUPPORTED                            =12,
    IVE_ERR_INIT_UNSUPPORTED_ENCODE_QUALITY                     =13,
    IVE_ERR_INIT_UNSUPPORTED_RATE_CONTROL_CONFIG                =14,
    IVE_ERR_INIT_UNSUPPORTED_MAX_FRAMERATE                      =15,
    IVE_ERR_INIT_UNSUPPORTED_MAX_BITRATE                        =16,
    IVE_ERR_INIT_UNSUPPORTED_NUM_BFRAMES                        =17,
    IVE_ERR_INIT_UNSUPPORTED_CONTENT_TYPE                       =18,
    IVE_ERR_INIT_UNSUPPORTED_SEARCH_RANGE                       =19,
    IVE_ERR_INIT_QUARTERPEL_NOT_SUPPORTED                       =20,
    IVE_ERR_INIT_UNSUPPORTED_MAX_QP_VALUE                       =21,
    IVE_ERR_INIT_UNSUPPORTED_SLICE_MODE                         =22,
    IVE_ERR_INIT_UNSUPPORTED_SLICE_PARAM_VALUE                  =23,
    IVE_ERR_INIT_RANDOM_ACCESS_POINT_NOT_SUPPORTED              =24,
    IVE_ERR_INIT_MEM_RECS_IS_NULL                               =25,
    IVE_ERR_INIT_INVALID_MEM_RECS                               =26,
    IVE_ERR_RETR_MEM_REC_HANDLE_INVALID                         =27,
    IVE_ERR_RETR_MEM_REC_INVALID_IO_STRUCT_SIZE                 =28,
    IVE_ERR_RETR_MEM_REC_MEM_RECS_IS_NULL                       =29,
    IVE_ERR_CONTROL_CODEC_HANDLE_INVALID                        =30,
    IVE_ERR_CONTROL_INVALID_IO_STRUCT_SIZE                      =31,
    IVE_ERR_CONTROL_INVALID_SUBCMD                              =32,
    IVE_ERR_CONTROL_SET_PARAMS_WIDTH_HEIGHT_NOT_SUPPORTED       =33,
    IVE_ERR_CONTROL_SET_PARAMS_FRAME_RATE_NOT_SUPPORTED         =34,
    IVE_ERR_CONTROL_SET_PARAMS_BIT_RATE_NOT_SUPPORTED           =35,
    IVE_ERR_CONTROL_SET_PARAMS_GENERATE_HEADER_VALUE_INVALID    =36,
    IVE_ERR_CONTROL_SET_PARAMS_INTRA_FRAME_INTERVAL_INVALID     =37,
    IVE_ERR_CONTROL_SET_PARAMS_NUM_B_FRAMES_IS_NONZERO          =38,
    IVE_ERR_CONTROL_SET_PARAMS_FORCE_FRAME_VALUE_NOT_SUPPORTED  =39,
    IVE_ERR_CONTROL_SET_PARAMS_ENC_CONFIG_VALUE_NOT_SUPPORTED   =40,
    IVE_ERR_CONTROL_SET_PARAMS_AIR_REFERSH_PERIOD_IS_NONZERO    =41,
    IVE_ERR_CONTROL_SET_PARAMS_RANDOM_ACCESS_PERIOD_IS_NON_ZERO =42,
    IVE_ERR_CONTROL_SET_PARAMS_I_QP_INVALID                     =43,
    IVE_ERR_CONTROL_SET_PARAMS_P_QP_INVALID                     =44,
    IVE_ERR_CONTROL_SET_PARAMS_BUFFER_DELAY_INVALID             =45,
    IVE_ERR_CONTROL_SET_PARAMS_BUF_SIZE_NONZERO                 =46,
    IVE_ERR_CONTROL_GET_VERSION_BUFFER_IS_NULL                  =47,
    IVE_ERR_CONTROL_GET_VERSION_BUFFER_SIZE_INSUFFICIENT        =48,
    IVE_ERR_ENCODE_CODEC_HANDLE_INVALID                         =49,
    IVE_ERR_ENCODE_IO_STRUCT_SIZE_INVALID                       =50,
    IVE_ERR_ENCODE_INPUT_BUF_PTR_NULL                           =51,
    IVE_ERR_ENCODE_INPUT_BUF_STRUCT_INVALID                     =52,
    IVE_ERR_ENCODE_RECON_BUF_PTR_NULL                           =53,
    IVE_ERR_ENCODE_RECON_BUF_STRUCT_INVALID                     =54,
    IVE_ERR_ENCODE_INPUT_TIMESTAMP_INVALID                      =55,
    IVE_ERR_ENCODE_OUT_BUFS_PTR_NULL                            =56,
    IVE_ERR_ENCODE_OUTBUFS_STRCUT_INVALID                       =57,
    IVE_ERR_ENCODE_OR_CNTRL_CALLED_BEFORE_INIT					=58,
    IVE_ERR_INIT_UNSUPPORTED_STUFFING_FLAG						=59,
    IVE_ERR_DUMMY												=60
}IVE_ERROR_CODES_T;


/*****************************************************************************/
/*   Initialize encoder                                                      */
/*****************************************************************************/

/* IVE_API_COMMAND_TYPE_T::e_cmd = IVE_CMD_INIT                              */


typedef struct {
    /* u4_size of the structure                                         */
    UWORD32                                 u4_size;
    IVE_API_COMMAND_TYPE_T         e_cmd;
    UWORD32                                 u4_num_mem_rec;
    UWORD32                                 u4_frm_max_wd;
    UWORD32                                 u4_frm_max_ht;
    IV_COLOR_FORMAT_T                 input_colour_format;
    IV_COLOR_FORMAT_T                 recon_colour_format;
    IVE_QUALITY_CONFIG                 u4_enc_quality_config;
    IVE_RATE_CONTROL_CONFIG        u4_rate_control_config;
    UWORD32                                 u4_max_framerate;
    UWORD32                                 u4_max_bitrate;
    UWORD32                                 u4_max_num_bframes;
    IV_CONTENT_TYPE_T                 content_type;
    UWORD32                                 u4_stuffing_disabled;
    UWORD32                                 u4_max_searchrange_xy;
    UWORD32                                 u4_enable_quarterpel;
    UWORD32                                 u4_max_qp;
    UWORD32									u4_air_refresh_period;
    UWORD32                                 u4_slice_packet_mode;
    UWORD32                                 u4_slice_packet_param_value;
    UWORD32                                 u4_enable_random_access_points;
    iv_mem_rec_t                            *pv_mem_rec_location;
}ive_init_ip_t;


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                 u4_size;
    UWORD32                                 u4_error_code;
}ive_init_op_t;


/*****************************************************************************/
/*   Video Encode                                                            */
/*****************************************************************************/


/* IVE_API_COMMAND_TYPE_T::e_cmd = IVE_CMD_VIDEO_DECODE                      */


typedef struct {
    /* u4_size of the structure                                         */
    UWORD32                                 u4_size;
    IVE_API_COMMAND_TYPE_T                  e_cmd;
	iv_yuv_buf_t							*input_buf;
	iv_yuv_buf_t							*recon_buf;
	UWORD32									u4_topfield_first;
	UWORD32									u4_inp_timestamp;
	iv_bufs_t								*out_bufs;
	IV_COLOR_FORMAT_T						u1_colour_format;
	void									*me_info_buf;
}ive_video_encode_ip_t;


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                 u4_size;
    UWORD32                                 u4_error_code;
    UWORD32									u4_encoded_frame_type;
    UWORD32									u4_out_timestamp;
    UWORD32									u4_frame_skipped;
    UWORD32									u4_bytes_generated;
}ive_video_encode_op_t;



/*****************************************************************************/
/*   Video control  Flush                                                    */
/*****************************************************************************/
/* IVE_API_COMMAND_TYPE_T::e_cmd            = IVE_CMD_VIDEO_CTL              */
/* IVE_CONTROL_API_COMMAND_TYPE_T::e_sub_cmd    = IVE_CMD_ctl_FLUSH          */



typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                 u4_size;
    IVE_API_COMMAND_TYPE_T                  e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T          e_sub_cmd;
}ive_ctl_flush_ip_t;


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                 u4_size;
    UWORD32                                 u4_error_code;
}ive_ctl_flush_op_t;

/*****************************************************************************/
/*   Video control reset                                                     */
/*****************************************************************************/
/* IVE_API_COMMAND_TYPE_T::e_cmd            = IVE_CMD_VIDEO_CTL              */
/* IVE_CONTROL_API_COMMAND_TYPE_T::e_sub_cmd    = IVE_CMD_ctl_RESET          */


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                 u4_size;
    IVE_API_COMMAND_TYPE_T                  e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T          e_sub_cmd;
}ive_ctl_reset_ip_t;


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                 u4_size;
    UWORD32                                 u4_error_code;
}ive_ctl_reset_op_t;


/*****************************************************************************/
/*   Video control  Set Params                                               */
/*****************************************************************************/
/* IVE_API_COMMAND_TYPE_T::e_cmd        = IVE_CMD_VIDEO_CTL                  */
/* IVE_CONTROL_API_COMMAND_TYPE_T::e_sub_cmd=IVE_CMD_ctl_SETPARAMS           */
/* IVE_CONTROL_API_COMMAND_TYPE_T::e_sub_cmd=IVE_CMD_ctl_SETDEFAULT          */

typedef struct {
    /* u4_size of the structure                                         */
    UWORD32                                     u4_size;
    IVE_API_COMMAND_TYPE_T                      e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
	UWORD32										u4_inp_width;
	UWORD32										u4_inp_height;
	UWORD32										u4_src_frame_rate;
	UWORD32										u4_tgt_frame_rate;
	UWORD32										u4_target_bitrate;
	UWORD32										u4_generate_header;
	UWORD32										u4_capture_width;
	UWORD32										u4_intra_frame_interval;
	UWORD32										u4_num_b_frames;
	IV_PICTURE_CODING_TYPE_T					force_frame;
    IVE_CONFIG_PARAMS                           enc_config;
    UWORD32                                     u4_air_refresh_period;
    UWORD32                                     u4_random_access_period;
    UWORD32                                     u4_set_i_qp;
    UWORD32                                     u4_set_p_qp;
    UWORD32                                     u4_set_b_qp;
    UWORD32                                     u4_vbv_buffer_delay;
    UWORD32                                     u4_vbv_buf_size;
    UWORD8	                                   u1_nal_size_insertion_flag;
    UWORD32                                     u4_set_minQp_I_frame;
    UWORD32                                     u4_set_minQp_P_frame;
    UWORD32                                     u4_set_maxQp_I_frame;
    UWORD32                                     u4_set_maxQp_P_frame;
}ive_ctl_set_config_ip_t;


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;
}ive_ctl_set_config_op_t;

/*****************************************************************************/
/*   Video control:Get Buf Info                                              */
/*****************************************************************************/

/* IVE_API_COMMAND_TYPE_T::e_cmd         = IVE_CMD_VIDEO_CTL                 */
/* IVE_CONTROL_API_COMMAND_TYPE_T::e_sub_cmd=IVE_CMD_ctl_GETBUFINFO          */


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                     u4_size;
    IVE_API_COMMAND_TYPE_T                      e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
}ive_ctl_getbufinfo_ip_t;


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;
    /* no of input buffers required for codec */
    UWORD32                                     u4_min_num_in_bufs;
    /* no of output buffers required for codec */
    UWORD32                                     u4_min_num_out_bufs;
    /* sizes of each input buffer required */
    UWORD32                                     u4_min_in_buf_size[IVE_MAX_IO_BUFFERS];
    /* sizes of each output buffer required */
    UWORD32                                     u4_min_out_buf_size[IVE_MAX_IO_BUFFERS];
    /* sizes of buffer required to store Dsp info */
    UWORD32                                     u4_min_me_info_buf_size;

}ive_ctl_getbufinfo_op_t;




/*****************************************************************************/
/*   Video control:Get Version Info                                          */
/*****************************************************************************/

/* IVE_API_COMMAND_TYPE_T::e_cmd        = IVE_CMD_VIDEO_CTL                  */
/* IVE_CONTROL_API_COMMAND_TYPE_T::e_sub_cmd=IVE_CMD_ctl_GETVERSION          */


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                     u4_size;
    IVE_API_COMMAND_TYPE_T                      e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
    void                                        *pv_version_buffer;
    UWORD32                                     u4_version_buffer_size;
}ive_ctl_getversioninfo_ip_t;


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;
}ive_ctl_getversioninfo_op_t;


/*****************************************************************************/
/*   Video control:set	default params		                                  */
/*****************************************************************************/
/* IVE_API_COMMAND_TYPE_T::e_cmd        = IVE_CMD_VIDEO_CTL                  */
/* IVE_CONTROL_API_COMMAND_TYPE_T::e_sub_cmd=IVE_CMD_CTL_SETDEFAULT          */
typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                     u4_size;
    IVE_API_COMMAND_TYPE_T                      e_cmd;
    IVE_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
}ive_ctl_setdefault_ip_t;


typedef struct{
    /* u4_size of the structure                                         */
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;
}ive_ctl_setdefault_op_t;



#endif /* _IVE_H */

