/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                 Securemux

GENERAL DESCRIPTION
Secure mux TrustZone commands.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2013 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                            EDIT HISTORY FOR MODULE

$Header:

when       who    what, where, why
--------   ---    ----------------------------------------------------------
09/30/2013  yb    Created
===========================================================================*/
#ifndef SMUX_TZ_COMMANDS_H_
#define SMUX_TZ_COMMANDS_H_

#include <ctype.h>
#include <stdio.h>
#include "smux_mem.h"
#include "common_log.h"

#define SMUX_MAX_PHYSICAL_CHUNKS 10
#define SMUX_PHYSICAL_CHUNK_LEN (1024*1024)

// Enumerated types

typedef enum {
  SEC_MUX_SUCCESS,
  SEC_MUX_INVALID_PARAMS,
  SEC_MUX_BUFFER_FULL,
  SEC_MUX_ERR_SECURITY_FAULT,
  SEC_MUX_GENERAL_FAILURE ,
  SEC_MUX_ERR_SIZE = 0x7FFFFFFF
}smux_status_t;

/*-------------------------------------------------------------------------*/

typedef enum smux_audio_type
{
	SEC_MUX_STREAM_AUDIO_NONE,						/* flag indicating no audio       */
	SEC_MUX_STREAM_AUDIO_QCELP13K_FULL,					/* PureVoice QCELP-13K fixed full */
	SEC_MUX_STREAM_AUDIO_QCELP13K_HALF,					/* PureVoice QCELP-13K fixed half */
	SEC_MUX_STREAM_AUDIO_EVRC,						/* Enhanced Variable Rate Codec   */
	SEC_MUX_STREAM_AUDIO_EVRC_PV,						/* EVRC, PacketVideo variant      */
	SEC_MUX_STREAM_AUDIO_AMR,						/* GSM Adaptive Multi Rate codec  */
	SEC_MUX_STREAM_AUDIO_MPEG1_L3,						/* MPEG-1, Layer 3 (MP3) codec    */
	SEC_MUX_STREAM_AUDIO_MPEG2_L3,						/* MPEG-2, Layer 3 (MP3) codec    */
	SEC_MUX_STREAM_AUDIO_MPEG4_AAC,						/* MPEG-4 Advanced Audio Codec    */
	SEC_MUX_STREAM_AUDIO_PUREVOICE,						/* 3GPP2 QCELP                    */
	SEC_MUX_STREAM_AUDIO_AMR_WB,						/* AMR Wideband codec (TS 26.171) */
	SEC_MUX_STREAM_AUDIO_AMR_WB_PLUS,					/* Extended AMR WB (TS 26.290)    */
	SEC_MUX_STREAM_AUDIO_EVRC_B,						/* EVRC-B type codec							*/
	SEC_MUX_STREAM_AUDIO_EVRC_WB,						/* EVRC-WB type codec						*/
	SEC_MUX_STREAM_AUDIO_PCM,						/* Raw audio formats              */
	SEC_MUX_STREAM_AUDIO_UNK,						/* Unknown type of audio          */
	SEC_MUX_STREAM_AUDIO_INVALID = 0x7FFFFFFF     /* invalid audio stream type      */
} smux_audio_type_t;

/*-------------------------------------------------------------------------*/

typedef enum smux_video_type
{
    SEC_MUX_STREAM_VIDEO_NONE,				/* flag indicating no video         */
    SEC_MUX_STREAM_VIDEO_MPEG4,					/* ISO/IEC 14496-2                  */
    SEC_MUX_STREAM_VIDEO_H263,				  /* H.263				*/
    SEC_MUX_STREAM_VIDEO_H264,				/* H.264                            */
    SEC_MUX_STREAM_VIDEO_JPEG,					/* SKT-MOD JPEG                     */
    SEC_MUX_STREAM_VIDEO_BMP,					/* SKT-MOD BMP                      */
    SEC_MUX_STREAM_VIDEO_STILL_IMAGE,			/* PV Specific Still Image          */
    SEC_MUX_STREAM_VIDEO_UNK,				/* unknown type of video            */
    SEC_MUX_STREAM_VIDEO_INVALID = 0x7FFFFFFF	/* invalid video stream type			*/
} smux_video_type_t;

/*-------------------------------------------------------------------------*/

// Data structures

typedef struct smux_buff_descr {
	void *base_addr;
	uint32_t length;
} smux_buff_descr_t;

/*-------------------------------------------------------------------------*/

typedef struct smux_secure_buff_descr {
	struct smux_buff_descr buff_chunks[SMUX_MAX_PHYSICAL_CHUNKS];
	uint32_t length;
} smux_secure_buff_descr_t;

/*-------------------------------------------------------------------------*/
typedef struct smux_sample_info
{
    uint32_t nSize;											/* size of sample in bytes															*/
    uint32_t  nStreamNum;										/* Index of the current stream														*/
    char aExtra[16]; /* Extra data pointer																		*/
    uint8_t u8ExtraLen;												/* Extra Data Length																		  */
    uint8_t u8IsSync;											/* Indication if sample is a random access point					*/
    smux_secure_buff_descr_t	outputBuffer;		/* output buffer handler																	*/
    smux_secure_buff_descr_t	inputSample;		/* input buffer handler						*/
    smux_buff_descr_t			userData;						/* user data buffer handler															*/
    uint32_t  u32InOffset;									/* Offset in the input buffer														*/
    uint32_t  u32OutOffset;									/* Offset is the output buffer														*/
    uint32_t  u32PCROffset;									/* Offset in PCR/PMT/PAT buffer to write									*/
    uint32_t u32UserDataOffset;								/* Offset in user data buffer to write										*/
    uint64_t u64PTSBase;											/* PTS time base																					*/
    uint8_t u8IsUserData;											/* flag if there is user data to write										*/
    uint8_t u8IsPCRData;											/* flag if there is PCR/PAT/PMT data to write						*/
    uint8_t u8StartOfPES;											/* flag if that is the first call with the same sample		*/
    uint8_t u8NsDbgFlag;											/* flag if that is debug mode														*/
} smux_sample_info_t;

/*-------------------------------------------------------------------------*/

typedef struct smux_config
{
    smux_audio_type_t		audioType;					/* audio stream type			 */
    smux_video_type_t		videoType;					/* video stream type			 */
    uint16_t					u16AudioPid;						/* audio pid							 */
    uint16_t					u16VideoPid;						/* video pid							 */
    uint8_t					u8AudioStreamNum;					/* stream number of audio        */
    uint8_t					u8VideoStreamNum;					/* stream number of video        */
} smux_config_t;
/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* Loading the requested application */
int32_t smux_start_app(struct QSEECom_handle **l_QSEEComHandle,
                        const char *appname, int32_t buf_size);

/* Configure the secure mux instance in the TZ */
smux_status_t SMUX_config(struct QSEECom_handle *handle , smux_config_t *configArgs ,struct smux_ion_info *tables_ihandle);

/* Mux the video/audio sample to the output buffer and put the PAT/PMT/PCR/userdata packets in the output */
smux_status_t SMUX_Process_Sample(struct QSEECom_handle *handle ,smux_sample_info_t* sample_info,
																	struct smux_ion_info *input_ihandle, struct smux_ion_info *output_ihandle,
																	struct smux_ion_info *userdata_ihandle,	uint32_t*  nInOffset,
																	uint32_t*  nPCROffset, uint32_t*  nUserDataOffset, uint32_t* nOutputOffset);

/* Unloading the requested application */
int32_t smux_shutdown_app(struct QSEECom_handle **l_QSEEComHandle);

#ifdef __cplusplus
}
#endif

#endif /* SMUX_TZ_COMMANDS_H_ */

