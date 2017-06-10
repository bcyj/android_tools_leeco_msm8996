/*==============================================================================
*        @file wdsm_mm_interface.h
*
*  @par DESCRIPTION:
*       Definition of the wireless display session manager MM inteface
*
*
*  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:

===============================================================================*/
#ifndef __WDSM_MM_INTERFACE_H__
#define __WDSM_MM_INTERFACE_H__

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "AEEStdDef.h"
#include "AEEstd.h"
//f/w declration of HDCP class
class CWFD_HdcpCp;

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
typedef void *WFD_MM_HANDLE;

/* WFD run time commands */
/* it corrosponds to RuntimecmdType in wfdenums */
typedef enum
{
  WFD_MM_CMD_INVALID = -1,
  WFD_MM_CMD_OPEN_AUDIO_PROXY = 0,
  WFD_MM_CMD_CLOSE_AUDIO_PROXY,
  WFD_MM_CMD_ENABLE_BITRATE_ADAPT,
  WFD_MM_CMD_DISABLE_BITRATE_ADAPT
}WFD_runtime_cmd_t;

/* WFD device type */
typedef enum {
  WFD_DEVICE_UNK,              			/* unknown device type */
  WFD_DEVICE_SOURCE,           			/* WFD source  */
  WFD_DEVICE_PRIMARY_SINK,     			/* WFD primary sink  */
  WFD_DEVICE_SECONDARY_SINK,   			/* WFD secondary sink  */
  WFD_DEVICE_SOURCE_PRIMARY_SINK,     		/* WFD source / primary sink  */
  WFD_DEVICE_INVALID           			/* invalid device type */
} WFD_device_t;

/* =====================================
** H264 codec configuration data structure
** ===================================== */
typedef struct
{
	uint8		h264_profile;
	uint8		h264_level;
	uint16		max_hres;
	uint16		max_vres;
	uint32		supported_cea_mode;
	uint32		supported_vesa_mode;
	uint32		supported_hh_mode;
	uint8		decoder_latency;
	uint16		min_slice_size;
	uint8		slice_enc_params;
	uint8		frame_rate_control_support;
} WFD_h264_codec_config_t;

typedef struct
{
	uint8		h264_profile;
	uint8		h264_level;
	uint16		max_hres;
	uint16		max_vres;
	uint32		supported_modes_bitmap;
	uint8		decoder_latency;
	uint16		min_slice_size;
	uint8		slice_enc_params;
	uint8		frame_rate_control_support;
} WFD_3d_h264_codec_config_t;

typedef struct
{
  uint8       numBlocks;
  uint8      *pEdidData;
}WFD_edid_info_type;


#define BIT0       0x00000001
#define BIT1       0x00000002
#define BIT2       0x00000004
#define BIT3       0x00000008
#define BIT4       0x00000010
#define BIT5       0x00000020
#define BIT6       0x00000040
#define BIT7       0x00000080
#define BIT8       0x00000100
#define BIT9       0x00000200
#define BIT10      0x00000400
#define BIT11      0x00000800
#define BIT12      0x00001000
#define BIT13      0x00002000
#define BIT14      0x00004000
#define BIT15      0x00008000
#define BIT16      0x00010000
#define BIT17      0x00020000
#define BIT18      0x00040000
#define BIT19      0x00080000
#define BIT20      0x00100000
#define BIT21      0x00200000
#define BIT22      0x00400000
#define BIT23      0x00800000
#define BIT24      0x01000000
#define BIT25      0x02000000
#define BIT26      0x04000000
#define BIT27      0x08000000
#define BIT28      0x10000000
#define BIT29      0x20000000
#define BIT30      0x40000000
#define BIT31      0x80000000

/*HDCP Capability versions*/
#define WFD_HDCP_2_0 BIT0
#define WFD_HDCP_2_1 BIT1

/* CEA Resolution/Refresh Rate */
#define		H264_CEA_640x480p60		BIT0
#define		H264_CEA_720x480p60		BIT1
#define		H264_CEA_720x480i60		BIT2
#define		H264_CEA_720x576p50		BIT3
#define		H264_CEA_720x576i50		BIT4
#define		H264_CEA_1280x720p30		BIT5
#define		H264_CEA_1280x720p60		BIT6
#define		H264_CEA_1920x1080p30		BIT7
#define		H264_CEA_1920x1080p60		BIT8
#define		H264_CEA_1920x1080i60		BIT9
#define		H264_CEA_1280x720p25		BIT10
#define		H264_CEA_1280x720p50		BIT11
#define		H264_CEA_1920x1080p25		BIT12
#define		H264_CEA_1920x1080p50		BIT13
#define		H264_CEA_1920x1080i50		BIT14
#define		H264_CEA_1280x720p24		BIT15
#define		H264_CEA_1920x1080p24		BIT16

/* VESA Resolution/Refresh Rate */
#define		H264_VESA_800x600p30		BIT0
#define		H264_VESA_800x600p60		BIT1
#define		H264_VESA_1024x768p30		BIT2
#define		H264_VESA_1024x768p60		BIT3
#define		H264_VESA_1152x864p30		BIT4
#define		H264_VESA_1152x864p60		BIT5
#define		H264_VESA_1280x768p30		BIT6
#define		H264_VESA_1280x768p60		BIT7
#define		H264_VESA_1280x800p30		BIT8
#define		H264_VESA_1280x800p60		BIT9
#define		H264_VESA_1360x768p30		BIT10
#define		H264_VESA_1360x768p60		BIT11
#define		H264_VESA_1366x768p30		BIT12
#define		H264_VESA_1366x768p60		BIT13
#define		H264_VESA_1280x1024p30		BIT14
#define		H264_VESA_1280x1024p60		BIT15
#define		H264_VESA_1400x1050p30		BIT16
#define		H264_VESA_1400x1050p60		BIT17
#define		H264_VESA_1440x900p30		BIT18
#define		H264_VESA_1440x900p60		BIT19
#define		H264_VESA_1600x900p30		BIT20
#define		H264_VESA_1600x900p60		BIT21
#define		H264_VESA_1600x1200p30		BIT22
#define		H264_VESA_1600x1200p60		BIT23
#define		H264_VESA_1680x1024p30		BIT24
#define		H264_VESA_1680x1024p60		BIT25
#define		H264_VESA_1680x1050p30		BIT26
#define		H264_VESA_1680x1050p60		BIT27
#define		H264_VESA_1920x1200p30		BIT28
#define		H264_VESA_1920x1200p60		BIT29

/* HH Resolution/Refresh Rate */
#define		H264_HH_800x480p30		BIT0
#define		H264_HH_800x480p60		BIT1
#define		H264_HH_854x480p30		BIT2
#define		H264_HH_854x480p60		BIT3
#define		H264_HH_864x480p30		BIT4
#define		H264_HH_864x480p60		BIT5
#define		H264_HH_640x360p30		BIT6
#define		H264_HH_640x360p60		BIT7

/* Display Native Resolution */
#define		H264_DISPLAY_NATIVE_SELECTION_CEA	0x00
#define		H264_DISPLAY_NATIVE_SELECTION_VESA	0x01
#define		H264_DISPLAY_NATIVE_SELECTION_HH	0x02

/* Supported H264 profile */
#define		H264_SUPPORTED_PROFILE_CBP	BIT0
#define		H264_SUPPORTED_PROFILE_CHP	BIT1

/* Maximum H264 Level Supported */
#define		H264_LEVEL_3_1			BIT0
#define		H264_LEVEL_3_2			BIT1
#define		H264_LEVEL_4			BIT2
#define		H264_LEVEL_4_1			BIT3
#define		H264_LEVEL_4_2			BIT4

/* 3D Video modes */
#define		THREED_1920x540PLUS540_p24_TBH		BIT0
#define		THREED_1280x360PLUS360_p60_TBH		BIT1
#define		THREED_1280x360PLUS360_p50_TBH		BIT2
#define		THREED_1920x1080_p24x2_FS		BIT3
#define		THREED_1280x720_p60x2_FS		BIT4
#define		THREED_1280x72_p30x2_FS			BIT5
#define		THREED_1280x72_p50x2_FS			BIT6
#define		THREED_1280x72_p25x2_FS			BIT7
#define		THREED_1920x1080PLUS45PLUS1080_p24_FP	BIT8
#define		THREED_1280x720PLUS30PLUS720_p60_FP	BIT9
#define		THREED_1280x720PLUS30PLUS720_p30_FP	BIT10
#define		THREED_1280x720PLUS30PLUS720_p50_FP	BIT11
#define		THREED_1280x720PLUS30PLUS720_p25_FP	BIT12
#define		THREED_960PLUS960x1080_i60_SSH		BIT13
#define		THREED_960PLUS960x1080_i50_SSH		BIT14


/* =====================================
** LPCM codec data structure
** ===================================== */
typedef struct
{
	uint32	    supported_modes_bitmap;
	uint8       decoder_latency;
} WFD_lpcm_codec_config_t;

//#define		LPCM_(SamplingFrequency)_(BitWidth)_(channels)
#define		LPCM_441_16_2			BIT0
#define		LPCM_48_16_2			BIT1
#define		LPCM_48_16_4			BIT2
#define		LPCM_48_16_6			BIT3
#define		LPCM_48_16_8			BIT4
#define		LPCM_48_20_24_2			BIT5
#define		LPCM_48_20_24_4			BIT6
#define		LPCM_48_20_24_6			BIT7
#define		LPCM_48_20_24_8			BIT8
#define		LPCM_96_16_2			BIT9
#define		LPCM_96_16_4			BIT10
#define		LPCM_96_16_6			BIT11
#define		LPCM_96_16_8			BIT12
#define		LPCM_96_20_24_2			BIT13
#define		LPCM_96_20_24_4			BIT14
#define		LPCM_96_20_24_6			BIT15
#define		LPCM_96_20_24_8			BIT16
#define		LPCM_192_16_2			BIT17
#define		LPCM_192_16_4			BIT18
#define		LPCM_192_16_6			BIT19
#define		LPCM_192_20_24_2		BIT20
#define		LPCM_192_20_24_4		BIT21
#define		LPCM_192_20_24_6		BIT22

#define		LPCM_20_24_OPTION_MASK	BIT31

/* =====================================================
** Video frame rate change - Support for frame skipping 
** =================================================== */
#define WFD_FRAME_RATE_CHANGE_UNSUPPORTED 0
#define WFD_FRAME_RATE_CHANGE_SUPPORTED BIT0

/* =====================================
** AAC codec data structure
** ===================================== */
typedef struct
{
	uint16		supported_modes_bitmap;
	uint8		decoder_latency;
} WFD_aac_codec_config_t;

//#define		AAC_(SamplingFrequency)_(BitWidth)_(channels)
#define		AAC_48_16_2			BIT0
#define		AAC_48_16_4			BIT1
#define		AAC_48_16_6			BIT2
#define		AAC_48_16_8			BIT3


/* =====================================
** Dolby Digital (AC3) codec data structure
** ===================================== */
typedef struct
{
	uint32		supported_modes_bitmap;
	uint8		decoder_latency;
} WFD_dolby_digital_codec_config_t;

//#define		DOLBY_DIGITAL_(SamplingFrequency)_(BitWidth)_(channels)_(option)
#define		DOLBY_DIGITAL_48_16_2_AC3	BIT0
#define		DOLBY_DIGITAL_48_16_4_AC3	BIT1
#define		DOLBY_DIGITAL_48_16_6_AC3	BIT2
#define		DOLBY_DIGITAL_48_16_8_EAC3	BIT3


/* =====================================
** Content Protection Capability data structure
** ===================================== */
typedef struct
{
	uint8		content_protection_capability;
	uint16		content_protection_ake_port;
} WFD_content_protection_capability_config_t;

/* This enumeration type list the different types of RTP port */
typedef enum{
  RTP_PORT_UDP =0,   /* RTP port type UDP */
  RTP_PORT_TCP       /* RTP port type TCP */
}WFD_rtp_port_type;

/* =====================================
**  Transport Capability data structure
** ===================================== */
typedef struct
{
    uint8               transport_capability;
    int                 rtpSock;
    int                 rtcpSock;
    uint16              port1_id;
    uint16              port1_rtcp_id;
    uint16              port2_id;
    WFD_rtp_port_type eRtpPortType;
} WFD_transport_capability_config_t;


/* =====================================
** IP address of peer device(s)  ---  the transport subelement has only port info, so we need this to hold IP address of peer device(s)
** ===================================== */
typedef struct
{
	uint32			ipv4_addr1;
	uint32			ipv4_addr2;
        uint8                   device_addr1[24]; //mac address terminated with 0
        uint8                   device_addr2[24]; //mac address terminated with 0
} WFD_peer_ipv4_addr_t;


/* =====================================
** QoS setting for RTP connection(s) ---  the QoS setting includes DSCP vlaues, which will be put in IP header of RTP connection(s)
** ===================================== */
typedef struct
{
	uint8			qos_dscp1;
	uint8			qos_dscp2;
} WFD_rtp_qos_t;


/* This enumerated type lists the different types of video. */
typedef enum {
	WFD_VIDEO_UNK,    	/* unknown video codec type      */
	WFD_VIDEO_H264,   	/*  H264 video  */
	WFD_VIDEO_3D,     	/* 3D video */
	WFD_VIDEO_INVALID 	/* invalid video type      */
} WFD_video_type;

/* This enumerated type lists the different types of audio. */
typedef enum {
	WFD_AUDIO_UNK,          /* unknown audio */
	WFD_AUDIO_LPCM,         /* LPCM audio  */
	WFD_AUDIO_AAC,          /* AAC audio  */
	WFD_AUDIO_DOLBY_DIGITAL,/* Dolby Digital audio  (AC3) */
	WFD_AUDIO_INVALID       /* invalid audio */
} WFD_audio_type;


/* Video codec type */
#define WFD_MAX_NUM_H264_PROFILES 8
typedef struct {
	uint8			native_bitmap;
	uint8			preferred_display_mode_supported;
	/* H264 codec type  */
	WFD_h264_codec_config_t	*h264_codec;
	uint8			num_h264_profiles;
} WFD_video_codec_config_t;

typedef struct {
	uint8			native_bitmap;
	uint8 			preferred_display_mode_supported;
	/* H264 codec type  */
	WFD_3d_h264_codec_config_t	*h264_codec;
	uint8				num_h264_profiles;
} WFD_3d_video_codec_config_t;

typedef union {
	WFD_video_codec_config_t	video_config;
	WFD_3d_video_codec_config_t	threed_video_config;
} WFD_video_config;

/* Audio codec type */
typedef struct {
	/* lpcm codec type  */
	WFD_lpcm_codec_config_t			lpcm_codec;
	/* aac codec type  */
	WFD_aac_codec_config_t			aac_codec;
	/* dolby digital codec type  */
	WFD_dolby_digital_codec_config_t	dolby_digital_codec;
} WFD_audio_config;
/* Definitions of constants for extended capability*/
#define WFD_EXTENDED_CAPABILITY_SUB_ELEMENT_ID      7
#define WFD_EXTENDED_CAPABILITY_SUB_ELEMENT_LENGTH  2
#define WFD_EXTENDED_CAPABILITY_UIBC                BIT0
typedef struct __HDCP_CTXTYPE__
{
  CWFD_HdcpCp *hHDCPSession;
  bool  bHDCPSEssionValid;
}HDCP_CTXTYPE;

typedef struct
{
	uint8	sub_element_id;	/* Should be set to 7 for this structure*/
	uint8	length;		/* Should be set to 2 */
	uint16	ext_capability_bitmap;	/*Based on Table  5.27, spec 1.17 */
}WFD_extended_capability_config_t;

/* A/V capability */
typedef struct {
    WFD_video_type                  video_method;
    WFD_audio_type                  audio_method;
    WFD_video_config                video_config;
    WFD_audio_config                audio_config;
    bool                            standby_resume_support;

    WFD_content_protection_capability_config_t content_protection_config;
    WFD_transport_capability_config_t   transport_capability_config;

    WFD_peer_ipv4_addr_t            peer_ip_addrs;
    WFD_rtp_qos_t                   rtp_qos_settings;
    WFD_edid_info_type              edid;

        //holds pointer to surface of playback. Applicable only for sink
    void                           *pSurface;
    HDCP_CTXTYPE                    HdcpCtx;
    uint64                          decoder_latency;
} WFD_MM_capability_t;

typedef enum
{
    AV_CONTROL_PLAY,
    AV_CONTROL_PAUSE,
    AV_CONTROL_FLUSH,
    AV_CONTROL_SET_VOLUME,
    AV_CONTROL_SET_DECODER_LATENCY
}WFD_MM_AV_Stream_Control_t;

/* Status */
typedef enum {
    WFD_STATUS_SUCCESS,
    WFD_STATUS_FAIL,
    WFD_STATUS_NOTSUPPORTED,
    WFD_STATUS_BADPARAM,
    WFD_STATUS_MEMORYFAIL,
    WFD_STATUS_RUNTIME_ERROR,
    WFD_STATUS_READY,
    WFD_STATUS_RTCP_RR_MESSAGE,
    WFD_STATUS_CONNECTED,
    WFD_STATUS_PROXY_CLOSED,
    WFD_STATUS_PROXY_OPENED,
    WFD_STATUS_INVALID
} WFD_status_t;

/* General MM Updates to SM-B */
typedef enum {
  WFD_EVENT_NO_EVENT,
  WFD_EVENT_HDCP_CONNECT_DONE,
  WFD_EVENT_MM_VIDEO,
  WFD_EVENT_MM_AUDIO,
  WFD_EVENT_MM_SESSION_EVENT,
  WFD_EVENT_MM_RTP,
  WFD_EVENT_MM_HDCP,
  WFD_EVENT_MAX_EVENT,
}WFD_MM_Event_t;

/* Audio/Video stream control selection */
typedef enum {
	WFD_STREAM_AV,					/* play/pause both Video and Audio */
	WFD_STREAM_VIDEO,				/* play/pause video only */
	WFD_STREAM_AUDIO,				/* play/pause audio only */
} WFD_AV_select_t;

/* WFD HDCP version detials */
typedef enum {
   WFD_HDCP_VERSION_2_0 = 1,
   WFD_HDCP_VERSION_2_1,
   WFD_HDCP_VERSION_2_2,
   WFD_HDCP_VERSION_2_3,
   WFD_HDCP_VERSION_MAX = WFD_HDCP_VERSION_2_3
}WFD_HDCP_version_t;

typedef void (*wfd_mm_capability_change_cb)(WFD_MM_HANDLE);
typedef void (*wfd_mm_stream_play_cb)(WFD_MM_HANDLE, WFD_status_t status);
typedef void (*wfd_mm_stream_pause_cb)(WFD_MM_HANDLE, WFD_status_t status);
typedef void (*wfd_mm_update_session_cb)(WFD_MM_HANDLE, WFD_status_t status);
typedef void (*wfd_mm_update_event_cb)(WFD_MM_Event_t, WFD_status_t, void *pEvtData);
typedef void (*wfd_mm_request_IDRframe_cb)(WFD_MM_HANDLE);

typedef void (*wfd_av_format_change_timing_cb)(WFD_MM_HANDLE);

typedef struct 
{
  wfd_mm_capability_change_cb           capability_cb;  // Callback to inform session manager if there is any change in the MM capabilities
  wfd_mm_request_IDRframe_cb            idr_cb;         // Callback to inform sink session manager to send a request an IDR frame 
  wfd_av_format_change_timing_cb        av_format_change_cb;  // Callback to notify AV format chnage timing to session manager
  wfd_mm_update_event_cb                update_event_cb; //Callback to notify general events
}WFD_MM_callbacks_t;
/* =====================================================
**---------------------  Function prototype -----------------------------
** =====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/** @brief Get local MM capability structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_MM_capability_t - pointer to local capability structure
                WFD_device_t  - WFD device type                                       
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_local_capability
  (WFD_MM_HANDLE, WFD_MM_capability_t*, WFD_device_t);

/** @brief get negotiated capability
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_MM_capability_t - pointer to user-preferred capability structure
               WFD_MM_capability_t - pointer to remote capability structure
               WFD_MM_capability_t - pointer to negotiated capability structure
               WFD_MM_capability_t - pointer to common capability structure
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_negotiated_capability
  (WFD_MM_HANDLE, WFD_MM_capability_t*, WFD_MM_capability_t*, WFD_MM_capability_t*,WFD_MM_capability_t*);



/** @brief Get extended capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_extended_capability_config_t - pointer to video capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_get_extended_capability
  (WFD_MM_HANDLE, WFD_extended_capability_config_t*, WFD_device_t);

WFD_status_t wfd_mm_create_session
  (WFD_MM_HANDLE*, WFD_device_t, WFD_MM_capability_t*, WFD_MM_callbacks_t*);

/** @brief destroy MM session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_destroy_session(WFD_MM_HANDLE);

/** @brief Play
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the playing of Audio + Video or Audio only or Video only
               wfd_mm_capability_change_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_play(WFD_MM_HANDLE, WFD_AV_select_t, wfd_mm_stream_play_cb);

/** @brief Pause the session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the pausing of Audio + Video or Audio only or Video only
               wfd_mm_capability_change_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_pause(WFD_MM_HANDLE, WFD_AV_select_t, wfd_mm_stream_pause_cb);

/** @brief standby
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the playing of Audio + Video or Audio only or Video only               
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_standby(WFD_MM_HANDLE, wfd_mm_stream_pause_cb pCallback);

/** @brief resume
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the pausing of Audio + Video or Audio only or Video only               
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_resume(WFD_MM_HANDLE);

/** @brief update the session with new capabilty parameters
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_MM_capability_t - pointer to negotiated capabilty structure
               wfd_mm_update_session_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_update_session(WFD_MM_HANDLE, WFD_MM_capability_t*, wfd_mm_update_session_cb);			//not supported in initial implementation

//WFD_status_t wfd_mm_audio_to_primary_sink( WFD_MM_HANDLE, void (*wfd_mm_audio_to_primary_sink_cb)(WFD_MM_HANDLE, WFD_status_t);		//2nd sink currently not supported in initial implementation
//WFD_status_t wfd_mm_audio_to_secondary_sink( WFD_MM_HANDLE, void (*wfd_mm_audio_to_secondary_sink_cb)(WFD_MM_HANDLE, WFD_status_t) );	//2nd sink currently not supported in initial implementation

/** @brief send IDR frame from source
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_send_IDRframe(WFD_MM_HANDLE);

/** @brief set the frame rate at source
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  *@param[in] uint32 - Frame rate
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_set_framerate(WFD_MM_HANDLE, uint32);

/** @brief set the frame rate at source
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  *@param[in] uint32 - Bitrate
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_set_bitrate(WFD_MM_HANDLE hHandle, uint32);


/** @brief notify MM layer with runtime command
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_send_runtime_command(WFD_MM_HANDLE,WFD_runtime_cmd_t );


/** @brief get proposed capability
  *
  * @param[in] WFD_MM_HANDLE - WFD MM instance handle
               WFD_MM_capability_t - pointer to local capability structure
               WFD_MM_capability_t - pointer to remote capability structure
               WFD_MM_capability_t - pointer to proposed capability structure
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_proposed_capability
  (WFD_MM_HANDLE, WFD_MM_capability_t*, WFD_MM_capability_t*, WFD_MM_capability_t*);


/** @brief query AV format chnage timing parameters
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  *@param[in] uint32 - PTS
  *@param[in] uint32 - DTS
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_av_format_change_timing(WFD_MM_HANDLE, uint32*, uint32*);

/** @brief
  *
  * @param[in] WFD_MM_HANDLE - WFD MM  instance handle
  *
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_create_HDCP_session(WFD_MM_HANDLE*,
                                         WFD_device_t,
                                         WFD_MM_capability_t*,
                                         WFD_MM_capability_t*,
                                         WFD_MM_capability_t*);

/** @brief
  *
  * @param[in] WFD_MM_HANDLE - WFD MM instance handle
  * @param[in] WFD_device_t - WFD Device Type
*/
WFD_status_t wfd_mm_destroy_hdcp_session(WFD_MM_HANDLE hHandle,
                                         WFD_device_t tDevice);
/** @brief
  *
  * @param[in] hHandle - WFD Device type
  * @param[in] eDeviceType - WFD MM  instance handle
  * @param[i] control        - stream control command
  * @param[i] controlValue   - value associated with control
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_av_stream_control( WFD_MM_HANDLE hHandle,WFD_device_t eDeviceType,
                                        WFD_MM_AV_Stream_Control_t control, int64 controlValue);

WFD_status_t wfd_mm_get_current_PTS(WFD_MM_HANDLE hHandle, uint64 *timeStamp);

#ifdef __cplusplus
}
#endif
#endif //__WDSM_MM_INTERFACE_H__
