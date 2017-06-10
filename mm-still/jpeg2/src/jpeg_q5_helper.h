/*========================================================================


*//** @file jpeg_q5_helper.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/03/09   vma     Switched to use the os abstraction layer (os_*)
04/16/09   vma     Added PMEM unregistration.
01/20/09   vma     Added PMEM registration.
07/29/08   vma     Created file.

========================================================================== */

#ifndef _JPEG_Q5_HELPER_H
#define _JPEG_Q5_HELPER_H

#include "os_thread.h"
#include "jpeg_common_private.h"
#include "jpeg_buffer_private.h"

#include "os_int.h"
#include "os_types.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

// Message ID
typedef enum
{
    JPEG_Q5_MSG_JPEGE_ENCODE_ACK      = 0,
    JPEG_Q5_MSG_JPEGE_OUTPUT_PRODUCED = 1,
    JPEG_Q5_MSG_JPEGE_IDLE_ACK        = 2,
    JPEG_Q5_MSG_JPEGE_ILLEGAL_COMMAND = 3,
    JPEG_Q5_MSG_JPEGD_DECODE_ACK      = 4,
    JPEG_Q5_MSG_JPEGD_OUTPUT_PRODUCED = 5,
    JPEG_Q5_MSG_JPEGD_INPUT_REQUEST   = 6,
    JPEG_Q5_MSG_JPEGD_IDLE_ACK        = 7,
    JPEG_Q5_MSG_JPEGD_ILLEGAL_COMMAND = 8,

    JPEG_Q5_MSG_MAX = 0xFFFFFFFF,

} jpeg_q5_msg_id_t;

// Q5 status - done flag
typedef enum
{
    JPEGE_STATUS_IN_PROGRESS    = 0x0,
    JPEGE_STATUS_COMPLETE       = 0x1

} jpeg_q5_enc_done_flag_t;

// Q5 status - error
typedef enum
{
    JPEGE_STATUS_NO_ERRORS               = 0x0,
    JPEGE_STATUS_HUFFMAN_OVERFLOW_ERROR  = 0x1

} jpeg_q5_enc_error_t;

// Q5 Config command type
typedef enum
{
    JPEG_Q5_CONFIG_CMD_JPEGE = 0,
    JPEG_Q5_CONFIG_CMD_JPEGD = 1,

    JPEG_Q5_CONFIG_CMD_MAX   = 0xFFFFFFFF,

} jpeg_q5_cfg_cmd_t;

// Q5 Action command type
typedef enum
{
    JPEG_Q5_ACTION_CMD_ILLEGAL               = 0,
    JPEG_Q5_ACTION_CMD_JPEGE_ENCODE          = 1,
    JPEG_Q5_ACTION_CMD_JPEGE_OUTPUT_CONSUMED = 2,
    JPEG_Q5_ACTION_CMD_JPEGD_DECODE          = 3,
    JPEG_Q5_ACTION_CMD_JPEGD_OUTPUT_CONSUMED = 4,
    JPEG_Q5_ACTION_CMD_JPEGD_INPUT           = 5,
    JPEG_Q5_ACTION_CMD_JPEGE_IDLE            = 6,
    JPEG_Q5_ACTION_CMD_JPEGD_IDLE            = 7,

    JPEG_Q5_ACTION_CMD_MAX  = 0xFFFFFFFF,

} jpeg_q5_action_cmd_t;

// Q5 Jpeg Format
typedef enum
{
    JPEG_FORMAT_H2V2,
    JPEG_FORMAT_H2V1,
    JPEG_FORMAT_H1V2,

} jpeg_q5_format_t;

// Q5 Jpeg rotation
typedef enum
{
    JPEG_ROTATION_NONE,
    JPEG_ROTATION_90_DEGREES_COUNTER_CLOCKWISE,
    JPEG_ROTATION_180_DEGREES_COUNTER_CLOCKWISE,
    JPEG_ROTATION_270_DEGREES_COUNTER_CLOCKWISE,

} jpeg_q5_rotation_t;

// Q5 Jpeg Task FW State
typedef enum
{
    JPEG_FW_IDLE_STATE,   /* JPEG DSP code is loaded and waiting */
    JPEG_FW_ENCODE_STATE, /* Doing JPEG Encode */
    JPEG_FW_DECODE_STATE, /* Doing JPEG Decode */

} jpeg_q5_fw_state_t;

// Q5 Encode fragment config type
typedef struct
{
    uint8_t *p_luma;
    uint8_t *p_chroma;

} __attribute__((packed, aligned(4))) jpeg_q5_enc_frag_cfg_t;

// Q5 Decode input buffer config type
typedef struct
{
  uint8_t   * p_input_buf;
  uint32_t    buffer_size;
  uint32_t    end_of_input  :  1;
  uint32_t    reserved      : 31; /*reserved*/

} __attribute__((packed, aligned(4))) jpeg_q5_dec_input_buf_cfg_t;

// Q5 Decode output buffer config type
typedef struct
{
  uint8_t  * p_luma_buf;
  uint32_t   luma_buf_size;
  uint8_t  * p_chroma_buf;

} __attribute__((packed, aligned(4))) jpeg_q5_dec_output_buf_cfg_t;

// Q5 Encode config command header type
typedef struct
{
    jpeg_q5_cfg_cmd_t       command        : 32;

} __attribute__((packed, aligned(4))) jpeg_q5_cfg_cmd_hdr_t;

// Q5 Encode output buffer config type
typedef struct
{
    uint8_t * p_buffer;
    uint32_t  size;

} __attribute__((packed, aligned(4))) jpeg_q5_enc_output_buf_cfg_t;

// Q5 Encode upsampler polyphase filter coefficient type
typedef struct
{
    uint32_t        part_1                       : 32;
    uint32_t        part_2                       : 32;

} __attribute__((packed, aligned(4))) polyphase_coeff_t;

// Q5 Encode command header
typedef struct
{
    jpeg_q5_action_cmd_t    header : 32;

} __attribute__((packed, aligned(4))) jpeg_q5_enc_cmd_t;

// Q5 Decode command header
typedef struct
{
    jpeg_q5_action_cmd_t    header : 32;

} __attribute__((packed, aligned(4))) jpeg_q5_dec_cmd_t;

// Q5 Encode idle command
typedef struct
{
    jpeg_q5_action_cmd_t    header : 32;

} __attribute__((packed, aligned(4))) jpeg_q5_enc_idle_cmd_t;

// Q5 Decode idle command
typedef struct
{
    jpeg_q5_action_cmd_t    header : 32;

} __attribute__((packed, aligned(4))) jpeg_q5_dec_idle_cmd_t;

// Q5 Encode output consumed command
typedef struct
{
    jpeg_q5_action_cmd_t               header           : 32;
    jpeg_q5_enc_output_buf_cfg_t       output_buffer;

} __attribute__((packed, aligned(4))) jpeg_q5_enc_output_consumed_cmd_t;

// Q5 Decode output consumed command
typedef struct
{
    jpeg_q5_action_cmd_t               header           : 32;
    jpeg_q5_dec_output_buf_cfg_t       output_buffer;

} __attribute__((packed, aligned(4))) jpeg_q5_dec_output_consumed_cmd_t;

// Q5 Decode input buffer request command
typedef struct
{
    jpeg_q5_action_cmd_t               header           : 32;
    jpeg_q5_dec_input_buf_cfg_t        input_request;

} __attribute__((packed, aligned(4))) jpeg_q5_dec_input_request_cmd_t;

// Q5 Encode configuration command
typedef struct
{
    jpeg_q5_cfg_cmd_hdr_t        header;

    /* JPEG Encode Processing Configuration */
    jpeg_q5_format_t             subsample          :  2;
    uint32_t                     /* reserved */     :  6;
    jpeg_q5_rotation_t           rotation           :  2;
    uint32_t                     fragments          :  4;
    uint32_t                     /* reserved */     :  2;
    uint32_t                     restart            : 16;
     /* JPEG Encode Input Size Configuration */
    uint32_t                     input_height       : 16;
    uint32_t                     input_width        : 16;
     /* JPEG Encode Output Size Configuration */
    uint32_t                     output_height      : 16;
    uint32_t                     output_width       : 16;
     /* JPEG Encode Fragment Size Configuration */
    uint32_t                     fragment_height    : 16;
    uint32_t                     /* reserved */     : 16;
     /* JPEG Encode Fragment Configuration */
    jpeg_q5_enc_frag_cfg_t       fragment_config[8];
     /* JPEG Encode Output Buffer RTOS Partition Number */
    uint32_t                     rtos_partition_num :  4;
    uint32_t                     /* reserved */     : 28;
     /* JPEG Encode Output Buffer Configuration */
    jpeg_q5_enc_output_buf_cfg_t output_buffer_cfg[2];

     /* JPEG Encode Luma Quantization Table */
    uint16_t                     luma_qtbl[64];
     /* JPEG Encode Chroma Quantization Table */
    uint16_t                     chroma_qtbl[64];

     /* JPEG Encode Upsampler Input Size Configuration */
    uint32_t                     upsampler_input_width   : 12;
    uint32_t                     /* reserved */          :  4;
    uint32_t                     upsampler_input_height  : 12;
    uint32_t                     /* reserved */          :  3;
    uint32_t                     upsampler_enable        :  1;
     /* JPEG Encode Upsampler Input Frame Offset Configuration */
    uint32_t                     upsampler_hoffset       : 12;
    uint32_t                     /* reserved */          :  4;
    uint32_t                     upsampler_voffset       : 12;
    uint32_t                     /* reserved */          :  4;
    /* JPEG Encode Upsampler Polyphase Filter Coefficients */
    polyphase_coeff_t            coeff[32];

} __attribute__((packed, aligned(4))) jpeg_q5_enc_cfg_cmd_t;

// Q5 Decode configuration command
typedef struct
{
    jpeg_q5_cfg_cmd_hdr_t        header;

    /* Jpeg Decode image size configuration */
    uint32_t                     height                 : 16;
    uint32_t                     width                  : 16;
    jpeg_q5_format_t             subsample              :  2;
    uint32_t                     /*reserved*/           : 14;
    uint32_t                     scaling_factor         :  2;
    uint32_t                     /*reserved*/           : 14;
    uint32_t                     restart                : 16;
    uint32_t                     /*reserved*/           : 16;
    uint32_t                     rtos_partition_num     :  4;
    uint32_t                     /*reserved*/           : 28;

    /* Jpeg Decode input buffer configuration */
    jpeg_q5_dec_input_buf_cfg_t  input_buf_cfg;

    /* Jpeg Decode output buffer configurations */
    jpeg_q5_dec_output_buf_cfg_t output_buf_cfg[2];

    /* Jpeg Decode luma & chroma dequantization table */
    uint32_t                     dequant_tables[2][16];

    /* Jpeg Decode Huffman count tables */
    uint32_t                     huff_dc_luma_code_count[4];
    uint32_t                     huff_dc_luma_code_value[3];

    uint32_t                     huff_dc_chroma_code_count[4];
    uint32_t                     huff_dc_chroma_code_value[3];

    uint32_t                     huff_ac_luma_code_count[4];
    uint32_t                     huff_ac_luma_code_value[41];

    uint32_t                     huff_ac_chroma_code_count[4];
    uint32_t                     huff_ac_chroma_code_value[41];

} __attribute__((packed, aligned(4))) jpeg_q5_dec_cfg_cmd_t;

typedef struct
{
  jpeg_q5_enc_done_flag_t            done_flag        :  1;
  uint32_t                           /* reserved */   : 15;
  uint32_t                           error            : 16;

} __attribute__((packed, aligned(4))) jpeg_q5_enc_status_t;

// Q5 Encode message - Output produced message
typedef struct
{
   uint8_t *                 p_buf;
   uint32_t                  size;   /* Num bytes in buffer */
   jpeg_q5_enc_status_t      status; /* Status of encoder */

} __attribute__((packed, aligned(4))) jpeg_q5_enc_output_produced_msg_t;

// Q5 Decode message - Output produced message
typedef struct
{
   uint8_t *                 p_luma_buf;
   uint8_t *                 p_chroma_buf;
   uint32_t                  num_mcu         : 16; /* Number of MCU decoded */
   uint32_t                  /*reserved*/    : 16;
   uint32_t                  decode_state    :  1; /* 0 - in progress, 1 - done */
   uint32_t                  /*reserved*/    : 15;
   uint32_t                  error_msg       : 16;

} __attribute__((packed, aligned(4))) jpeg_q5_dec_output_produced_msg_t;

// Q5 generic message - Illegal command message
typedef struct
{
   uint32_t         command     : 16;   // jpeg_q5_action_cmd_t
   uint32_t         state       : 16;   // jpeg_q5_fw_state_t

} __attribute__((packed, aligned(4))) jpeg_q5_illegal_cmd_t;

#define JPEG_Q5_IOCTL_ERROR     0xffff

// State of the helper
typedef enum
{
    JPEG_Q5_HELPER_UNINITIALIZED = 0,
    JPEG_Q5_HELPER_READY,
    JPEG_Q5_HELPER_QUITING,

} jpeg_q5_helper_state_t;

// Handler for events from the q5 helper
typedef void (*jpeg_q5_event_handler_t)(void*       p_client,   // The pointer to the client using the helper
                                        uint16_t    event_id,   // The event id
                                        uint8_t    *p_buf,      // The event payload
                                        uint32_t    len);       // The payload length

typedef struct
{
    int                      fd;        // descriptor;
    os_mutex_t               mutex;     // mutex
    os_cond_t                cond;      // condition var
    os_thread_t              thread;    // os thread object
    void*                    p_client;  // Pointer to the client
                                        // using the helper
    jpeg_q5_event_handler_t  p_handler; // the event handler of the client
    jpeg_q5_helper_state_t   state;     // the state of the helper

} jpeg_q5_helper_t;

int jpeg_q5_helper_init(jpeg_q5_helper_t        *p_helper,
                        void                    *p_client,
                        jpeg_q5_event_handler_t  p_handler);

int jpeg_q5_helper_register_pmem(jpeg_q5_helper_t *p_helper,
                                 jpeg_buf_t       *p_buf);

int jpeg_q5_helper_unregister_pmem(jpeg_q5_helper_t *p_helper);

int jpeg_q5_helper_send_cfg_command(jpeg_q5_helper_t *p_helper,
                                    uint8_t          *cmd_buf,
                                    uint32_t          cmd_len);

int jpeg_q5_helper_send_action_command(jpeg_q5_helper_t *p_helper,
                                       uint8_t          *cmd_buf,
                                       uint32_t          cmd_len);

void jpeg_q5_helper_destroy(jpeg_q5_helper_t *p_helper);

#endif /* _JPEG_Q5_HELPER_H */
