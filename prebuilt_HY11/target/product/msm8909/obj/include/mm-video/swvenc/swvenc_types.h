/**
 * @file   swvenc_types.h
 * @brief  SwVenc framework data type definitions.
 * @copyright
 *         Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 *         Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _SWVENC_TYPES_H_
#define _SWVENC_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error code enumeration.
 */
typedef enum {
    SWVENC_S_SUCCESS,                ///< success
    SWVENC_S_FAILURE,                ///< failure
    SWVENC_S_NULL_POINTER,           ///< null pointer
    SWVENC_S_INVALID_PARAMETERS,     ///< invalid parameters
    SWVENC_S_INVALID_STATE,          ///< function called in invalid state
    SWVENC_S_INSUFFICIENT_RESOURCES, ///< insufficient resources
    SWVENC_S_UNSUPPORTED,            ///< unsupported feature
    SWVENC_S_NOT_IMPLEMENTED         ///< feature not yet implemented
} SWVENC_STATUS;

#define SWVENC_FLAG_EOS       0x00000001 /**< @brief End of stream:
                                          *     set by client for last ETB;
                                          *     set by swvenc for last FBD.
                                          */
#define SWVENC_FLAG_EXTRADATA 0x00000040 /**< @brief Extradata present:
                                          *     set by client for ETB;
                                          *     set by swvenc for FBD.
                                          */

/**
 * @brief Encoder properties enumeration.
 */
typedef enum {
    SWVENC_PROPERTY_ID_PROFILE,            ///< profile
    SWVENC_PROPERTY_ID_LEVEL,              ///< level
    SWVENC_PROPERTY_ID_FRAME_SIZE,         ///< frame size
    SWVENC_PROPERTY_ID_FRAME_RATE,         ///< frame rate (frames/sec)
    SWVENC_PROPERTY_ID_TARGET_BITRATE,     ///< target bitrate (bits/sec)
    SWVENC_PROPERTY_ID_INTRA_PERIOD,       ///< intra period
    SWVENC_PROPERTY_ID_IR_CONFIG,          ///< intra refresh configuration
    SWVENC_PROPERTY_ID_RC_MODE,            ///< rate control mode
    SWVENC_PROPERTY_ID_QP,                 ///< quantization parameters
    SWVENC_PROPERTY_ID_QP_RANGE,           ///< QP range
    SWVENC_PROPERTY_ID_SLICE_CONFIG,       ///< slice configuration
    SWVENC_PROPERTY_ID_COLOR_FORMAT,       ///< color format
    SWVENC_PROPERTY_ID_EXTRADATA_TYPE,     ///< extradata type
    SWVENC_PROPERTY_ID_FRAME_ATTRIBUTES,   ///< color format
    SWVENC_PROPERTY_ID_BUFFER_REQ,         ///< buffer requirements
    SWVENC_PROPERTY_ID_IFRAME_REQUEST,     ///< I-frame request
    SWVENC_PROPERTY_ID_MPEG4_AC_PRED,      ///< MPEG-4/H.263 AC prediction
    SWVENC_PROPERTY_ID_MPEG4_DP,           ///< MPEG-4/H.263 data partitioning
    SWVENC_PROPERTY_ID_MPEG4_SVH,          ///< MPEG-4/H.263 short video header
    SWVENC_PROPERTY_ID_MPEG4_HEC,          /**< MPEG-4/H.263 header extension
                                                code */
    SWVENC_PROPERTY_ID_MPEG4_TIME_INC_RES, /**< MPEG-4/H.263 time increment
                                                resolution (ticks/sec) */
    SWVENC_PROPERTY_ID_MPEG4_NUM_HTHREADS, /**< MPEG-4/H.263 number of helper
                                                threads */
    SWVENC_PROPERTY_ID_MPEG4_BATCH_SIZE    ///< MPEG-4/H.263 batch size in MBs
} SWVENC_PROPERTY_ID;

/**
 * @brief Codec enumeration.
 */
typedef enum {
    SWVENC_CODEC_H263, ///< H.263
    SWVENC_CODEC_MPEG4 ///< MPEG-4 Part 2
} SWVENC_CODEC;

/**
 * @brief Macro for printing codec.
 */
#define SWVENC_CODEC_STRING(x)                  \
    ((x == SWVENC_CODEC_H263) ?                 \
     "h263" :                                   \
     ((x == SWVENC_CODEC_MPEG4) ?               \
      "mpeg4" :                                 \
      "invalid"))

/**
 * @brief H.263 profile enumeration.
 */
typedef enum {
    SWVENC_PROFILE_H263_BASELINE ///< H.263 baseline profile
} SWVENC_PROFILE_H263;

/**
 * @brief Macro for printing H.263 profile.
 */
#define SWVENC_PROFILE_H263_STRING(x)           \
    ((x == SWVENC_PROFILE_H263_BASELINE) ?      \
     "baseline profile" :                       \
     "invalid")

/**
 * @brief MPEG-4 profile enumeration.
 */
typedef enum {
    SWVENC_PROFILE_MPEG4_SIMPLE,         ///< MPEG-4 simple profile
    SWVENC_PROFILE_MPEG4_ADVANCED_SIMPLE ///< MPEG-4 advanced simple profile
} SWVENC_PROFILE_MPEG4;

/**
 * @brief Macro for printing MPEG-4 profile.
 */
#define SWVENC_PROFILE_MPEG4_STRING(x)                  \
    ((x == SWVENC_PROFILE_MPEG4_SIMPLE) ?               \
     "simple profile" :                                 \
     ((x == SWVENC_PROFILE_MPEG4_ADVANCED_SIMPLE) ?     \
      "advanced simple profile" :                       \
      "invalid"))

/**
 * @brief Profile union.
 */
typedef union {
    SWVENC_PROFILE_H263  h263;  ///< H.263 profile
    SWVENC_PROFILE_MPEG4 mpeg4; ///< MPEG-4 profile
} SWVENC_PROFILE;

/**
 * @brief H.263 level enumeration.
 */
typedef enum {
    SWVENC_LEVEL_H263_10, ///< H.263 level 10
    SWVENC_LEVEL_H263_20, ///< H.263 level 20
    SWVENC_LEVEL_H263_30, ///< H.263 level 30
    SWVENC_LEVEL_H263_40, ///< H.263 level 40
    SWVENC_LEVEL_H263_45, ///< H.263 level 45
    SWVENC_LEVEL_H263_50, ///< H.263 level 50
    SWVENC_LEVEL_H263_60, ///< H.263 level 60
    SWVENC_LEVEL_H263_70  ///< H.263 level 70
} SWVENC_LEVEL_H263;

/**
 * @brief Macro for printing H.263 level.
 */
#define SWVENC_LEVEL_H263_STRING(x)             \
    ((x == SWVENC_LEVEL_H263_10) ?              \
     "10" :                                     \
     ((x == SWVENC_LEVEL_H263_20) ?             \
      "20" :                                    \
      ((x == SWVENC_LEVEL_H263_30) ?            \
       "30" :                                   \
       ((x == SWVENC_LEVEL_H263_40) ?           \
        "40" :                                  \
        ((x == SWVENC_LEVEL_H263_45) ?          \
         "45" :                                 \
         ((x == SWVENC_LEVEL_H263_50) ?         \
          "50" :                                \
          ((x == SWVENC_LEVEL_H263_60) ?        \
           "60" :                               \
           ((x == SWVENC_LEVEL_H263_70) ?       \
            "70" :                              \
            "invalid"))))))))

/**
 * @brief MPEG-4 level enumeration.
 */
typedef enum {
    SWVENC_LEVEL_MPEG4_0,  ///< MPEG-4 level 0
    SWVENC_LEVEL_MPEG4_0B, ///< MPEG-4 level 0b
    SWVENC_LEVEL_MPEG4_1,  ///< MPEG-4 level 1
    SWVENC_LEVEL_MPEG4_2,  ///< MPEG-4 level 2
    SWVENC_LEVEL_MPEG4_3,  ///< MPEG-4 level 3
    SWVENC_LEVEL_MPEG4_3B, ///< MPEG-4 level 3b
    SWVENC_LEVEL_MPEG4_4,  ///< MPEG-4 level 4
    SWVENC_LEVEL_MPEG4_4A, ///< MPEG-4 level 4a
    SWVENC_LEVEL_MPEG4_5,  ///< MPEG-4 level 5
    SWVENC_LEVEL_MPEG4_6,  ///< MPEG-4 level 6
    SWVENC_LEVEL_MPEG4_7,  ///< MPEG-4 level 7
    SWVENC_LEVEL_MPEG4_8,  ///< MPEG-4 level 8
    SWVENC_LEVEL_MPEG4_9   ///< MPEG-4 level 9
} SWVENC_LEVEL_MPEG4;

/**
 * @brief Macro for printing MPEG-4 level.
 */
#define SWVENC_LEVEL_MPEG4_STRING(x)            \
    ((x == SWVENC_LEVEL_MPEG4_0) ?              \
     "0" :                                      \
     ((x == SWVENC_LEVEL_MPEG4_0B) ?            \
      "0b" :                                    \
      ((x == SWVENC_LEVEL_MPEG4_1) ?            \
       "1" :                                    \
       ((x == SWVENC_LEVEL_MPEG4_2) ?           \
        "2" :                                   \
        ((x == SWVENC_LEVEL_MPEG4_3) ?          \
         "3" :                                  \
         ((x == SWVENC_LEVEL_MPEG4_3B) ?        \
          "3b" :                                \
          ((x == SWVENC_LEVEL_MPEG4_4) ?        \
           "4" :                                \
           ((x == SWVENC_LEVEL_MPEG4_4A) ?      \
            "4a" :                              \
            ((x == SWVENC_LEVEL_MPEG4_5) ?      \
             "5" :                              \
             ((x == SWVENC_LEVEL_MPEG4_6) ?     \
              "6" :                             \
              ((x == SWVENC_LEVEL_MPEG4_7) ?    \
               "7" :                            \
               ((x == SWVENC_LEVEL_MPEG4_8) ?   \
                "8" :                           \
                ((x == SWVENC_LEVEL_MPEG4_9) ?  \
                 "9" :                          \
                 "invalid")))))))))))))

/**
 * @brief Level union.
 */
typedef union {
    SWVENC_LEVEL_H263  h263;  ///< H.263 level
    SWVENC_LEVEL_MPEG4 mpeg4; ///< MPEG-4 level
} SWVENC_LEVEL;

/**
 * @brief Frame size structure.
 */
typedef struct {
    unsigned int width;  ///< frame width
    unsigned int height; ///< frame height
} SWVENC_FRAME_SIZE;

/**
 * @brief Intra period structure.
 *
 * bframes/pframes must be an integer number.
 * intra period = (pframes + bframes + 1)
 */
typedef struct {
    unsigned int pframes; ///< number of P-frames per I-frame
    unsigned int bframes; ///< number of B-frames per I-frame
} SWVENC_INTRA_PERIOD;

/**
 * @brief Intra refresh mode enumeration.
 */
typedef enum {
    SWVENC_IR_MODE_NONE,            ///< intra refresh off
    SWVENC_IR_MODE_CYCLIC,          ///< intra refresh cyclic
    SWVENC_IR_MODE_ADAPTIVE,        ///< intra refresh adaptive
    SWVENC_IR_MODE_CYCLIC_ADAPTIVE, ///< intra refresh cyclic + adaptive
    SWVENC_IR_MODE_RANDOM           ///< intra refresh random
} SWVENC_IR_MODE;

/**
 * @brief Macro for printing intra refresh mode.
 */
#define SWVENC_IR_MODE_STRING(x)                                    \
    ((x == SWVENC_IR_MODE_NONE) ? "none" :                          \
     ((x == SWVENC_IR_MODE_CYCLIC) ? "cyclic" :                     \
      ((x == SWVENC_IR_MODE_ADAPTIVE) ? "adaptive" :                \
       ((x == SWVENC_IR_MODE_CYCLIC_ADAPTIVE) ? "cyclic-adaptive" : \
        ((x == SWVENC_IR_MODE_RANDOM) ? "random" :                  \
         "invalid intra refresh mode")))))

/**
 * @brief Intra refresh configuration structure.
 */
typedef struct {
    SWVENC_IR_MODE mode;    ///< intra refresh mode
    unsigned int   cir_mbs; ///< num consecutive MBs intra coded for cyclic mode
} SWVENC_IR_CONFIG;

/**
 * @brief Rate control mode enumeration.
 */
typedef enum {
    SWVENC_RC_MODE_NONE,    ///< rate control off
    SWVENC_RC_MODE_VBR_VFR, ///< variable bitrate + variable frame rate
    SWVENC_RC_MODE_VBR_CFR, ///< variable bitrate + constant frame rate
    SWVENC_RC_MODE_CBR_VFR, ///< constant bitrate + variable frame rate
    SWVENC_RC_MODE_CBR_CFR  ///< constant bitrate + constant frame rate
} SWVENC_RC_MODE;

/**
 * @brief Macro for printing rate control mode.
 */
#define SWVENC_RC_MODE_STRING(x)                      \
    ((x == SWVENC_RC_MODE_NONE) ? "none" :            \
     ((x == SWVENC_RC_MODE_VBR_VFR) ? "vbr-vfr" :     \
      ((x == SWVENC_RC_MODE_VBR_CFR) ? "vbr-cfr" :    \
       ((x == SWVENC_RC_MODE_CBR_VFR) ? "cbr-vfr" :   \
        ((x == SWVENC_RC_MODE_CBR_CFR) ? "cbr-cfr" :  \
         "invalid rate control mode")))))

/**
 * @brief Quantization parameters structure.
 */
typedef struct {
    unsigned int qp_i; ///< I-frame QP
    unsigned int qp_p; ///< P-frame QP
    unsigned int qp_b; ///< B-frame QP
} SWVENC_QP;

/**
 * @brief QP range structure.
 */
typedef struct {
    unsigned int min_qp_packed; /**< min QP in packed format:
                                 * - bits  0- 7: min I-frame QP
                                 * - bits  8-15: min P-frame QP
                                 * - bits 16-23: min B-frame QP
                                 */
    unsigned int max_qp_packed; /**< max QP in packed format:
                                 * - bits  0- 7: max I-frame QP
                                 * - bits  8-15: max P-frame QP
                                 * - bits 16-23: max B-frame QP
                                 */
} SWVENC_QP_RANGE;

/**
 * @brief Slice mode enumeration.
 */
typedef enum {
    SWVENC_SLICE_MODE_OFF,  ///< no slicing
    SWVENC_SLICE_MODE_MB,   ///< MB-based slicing
    SWVENC_SLICE_MODE_BYTE, ///< byte-based slicing
    SWVENC_SLICE_MODE_GOB   ///< GOB-based slicing
} SWVENC_SLICE_MODE;

/**
 * @brief Macro for printing slice mode.
 */
#define SWVENC_SLICE_MODE_STRING(x)             \
    ((x == SWVENC_SLICE_MODE_OFF) ? "off" :     \
     ((x == SWVENC_SLICE_MODE_MB) ? "mb" :      \
      ((x == SWVENC_SLICE_MODE_BYTE) ? "byte" : \
       ((x == SWVENC_SLICE_MODE_GOB) ? "gob" :  \
        "invalid slice mode"))))

/**
 * @brief Slice configuration structure.
 */
typedef struct {
    SWVENC_SLICE_MODE mode; ///< slice mode
    unsigned int      size; ///< slice size in bytes or MBs
} SWVENC_SLICE_CONFIG;

/**
 * @brief Color format enumeration.
 */
typedef enum {
    SWVENC_COLOR_FORMAT_NV12, ///< NV12 interleaved format
    SWVENC_COLOR_FORMAT_NV21  ///< NV21 interleaved format
} SWVENC_COLOR_FORMAT;

/**
 * @brief Macro for printing color format.
 */
#define SWVENC_COLOR_FORMAT_STRING(x)                   \
    ((x == SWVENC_COLOR_FORMAT_NV12) ? "nv12" :         \
     ((x == SWVENC_COLOR_FORMAT_NV21) ? "nv21" :        \
      "invalid color format"))

/**
 * @brief Extradata type enum.
 */
typedef enum {
    SWVENC_EXTRADATA_TYPE_NONE,      ///< no extradata
    SWVENC_EXTRADATA_TYPE_SLICE_INFO ///< slice information
} SWVENC_EXTRADATA_TYPE;

/**
 * @brief Macro for printing extradata.
 */
#define SWVENC_EXTRADATA_TYPE_STRING(x)                         \
    ((x == SWVENC_EXTRADATA_TYPE_NONE) ? "none" :               \
     ((x == SWVENC_EXTRADATA_TYPE_SLICE_INFO) ? "slice info" :  \
      "invalid extradata type"))

/**
 * @brief Slice information structure.
 */
typedef struct {
    unsigned int offset; ///< offset of slice in bytes
    unsigned int length; ///< length of slice in bytes
} SWVENC_SLICE_INFO;

/**
 * @brief Extradata slice information structure.
 */
typedef struct {
    unsigned int       num_slices;   ///< number of slices
    SWVENC_SLICE_INFO *p_slice_info; /**< @brief pointer to 'num_slices' slice
                                          info structure(s) */
} SWVENC_EXTRADATA_SLICE_INFO;

/**
 * @brief Frame attributes structure.
 */
typedef struct {
    unsigned int stride_luma;   ///< stride of luma plane
    unsigned int stride_chroma; ///< stride of chroma plane
    unsigned int offset_luma;   ///< offset of luma plane
    unsigned int offset_chroma; ///< offset of chroma plane
    unsigned int size;          ///< size of luma plane + chroma plane
} SWVENC_FRAME_ATTRIBUTES;

/**
 * @brief Buffer type enumeration.
 */
typedef enum {
    SWVENC_BUFFER_INPUT,    ///< input buffer
    SWVENC_BUFFER_OUTPUT,   ///< output buffer
    SWVENC_BUFFER_EXTRADATA ///< extradata buffer
} SWVENC_BUFFER;

/**
 * @brief Buffer requirements structure.
 */
typedef struct {
    SWVENC_BUFFER type;      ///< buffer type
    unsigned int  size;      ///< buffer size
    unsigned int  mincount;  ///< buffer count (minimum)
    unsigned int  maxcount;  ///< buffer count (maximum)
    unsigned int  alignment; ///< buffer alignment
} SWVENC_BUFFER_REQ;

/**
 * @brief Encoder property information union.
 */
typedef union {
    SWVENC_PROFILE          profile;            ///< profile
    SWVENC_LEVEL            level;              ///< level
    SWVENC_FRAME_SIZE       frame_size;         ///< frame size
    unsigned int            frame_rate;         ///< frame rate (frames/sec)
    unsigned int            target_bitrate;     ///< target bitrate (bits/sec)
    SWVENC_INTRA_PERIOD     intra_period;       ///< intra period
    SWVENC_IR_CONFIG        ir_config;          ///< intra refresh configuration
    SWVENC_RC_MODE          rc_mode;            ///< rate control mode
    SWVENC_QP               qp;                 ///< quantization parameters
    SWVENC_QP_RANGE         qp_range;           ///< QP range
    SWVENC_SLICE_CONFIG     slice_config;       ///< slice configuration
    SWVENC_COLOR_FORMAT     color_format;       ///< color format
    SWVENC_EXTRADATA_TYPE   extradata_type;     ///< extradata type
    SWVENC_FRAME_ATTRIBUTES frame_attributes;   ///< frame attributes
    SWVENC_BUFFER_REQ       buffer_req;         ///< buffer requirements
    bool                    mpeg4_ac_pred;      ///< AC prediction
    bool                    mpeg4_dp;           ///< data partitioning
    bool                    mpeg4_svh;          ///< short video header
    unsigned int            mpeg4_hec;          ///< header extension code
    unsigned int            mpeg4_time_inc_res; /**< @brief time increment
                                                     resolution (ticks/sec) */
    unsigned int            mpeg4_num_hthreads; ///< number of helper threads
    unsigned int            mpeg4_batch_size;   ///< batch size in MBs
} SWVENC_PROPERTY_INFO;

/**
 * @brief Encoder property structure.
 */
typedef struct {
    SWVENC_PROPERTY_ID   id;   ///< property ID
    SWVENC_PROPERTY_INFO info; ///< property information
} SWVENC_PROPERTY;

/**
 * @brief Frame type enumeration.
 */
typedef enum {
    SWVENC_FRAME_TYPE_I, ///< I-frame
    SWVENC_FRAME_TYPE_P, ///< P-frame
    SWVENC_FRAME_TYPE_B  ///< B-frame
} SWVENC_FRAME_TYPE;

/**
 * @brief Macro for printing frame type.
 */
#define SWVENC_FRAME_TYPE_STRING(x)             \
    ((x == SWVENC_FRAME_TYPE_I) ? "I" :         \
     ((x == SWVENC_FRAME_TYPE_P) ? "P" :        \
      ((x == SWVENC_FRAME_TYPE_B) ? "B" :       \
       "unknown")))

/**
 * @brief Input buffer structure.
 */
typedef struct {
    unsigned char      *p_buffer;      ///< buffer pointer
    unsigned int        size;          ///< allocated size
    unsigned int        filled_length; ///< filled length
    unsigned long long  timestamp;     ///< timestamp in microseconds
    unsigned int        flags;         ///< buffer flags
    unsigned char      *p_client_data; ///< pointer to client data
} SWVENC_IPBUFFER;

/**
 * @brief Output buffer structure.
 */
typedef struct {
    unsigned char        *p_buffer;       ///< buffer pointer
    unsigned int          size;           ///< allocated size
    unsigned int          filled_length;  ///< filled length
    unsigned long long    timestamp;      ///< timestamp in microseconds
    unsigned int          flags;          ///< buffer flags
    SWVENC_FRAME_TYPE     frame_type;     ///< frame type
    SWVENC_EXTRADATA_TYPE extradata_type; ///< extradata type
    unsigned char        *p_extradata;    ///< extradata pointer
    unsigned char        *p_client_data;  ///< pointer to client data
} SWVENC_OPBUFFER;

/**
 * @brief Event enumeration.
 */
typedef enum {
    SWVENC_EVENT_FLUSH_DONE, ///< flush done
    SWVENC_EVENT_FATAL_ERROR ///< fatal error
} SWVENC_EVENT;

/**
 * @brief Software Encoder object handle dummy typedef.
 */
typedef void *SWVENC_HANDLE;

/**
 * @brief Callback function pointers structure.
 */
typedef struct {
    /**
     * @brief Callback function pointer to return input YUV buffer to client.
     *
     * @param swvenc:     SwVenc handle.
     * @param p_ipbuffer: Pointer to input YUV buffer.
     * @param p_client:   Client handle.
     *
     * @retval SWVENC_S_SUCCESS if success, error status otherwise.
     */
    SWVENC_STATUS (*pfn_empty_buffer_done)(SWVENC_HANDLE    swvenc,
                                           SWVENC_IPBUFFER *p_ipbuffer,
                                           void            *p_client);
    /**
     * @brief Callback function pointer to return output bitstream buffer to
     *        client.
     *
     * @param swvenc:     SwVenc handle.
     * @param p_opbuffer: Pointer to output bitstream buffer.
     * @param p_client:   Client handle.
     *
     * @retval SWVENC_S_SUCCESS if success, error status otherwise.
     */
    SWVENC_STATUS (*pfn_fill_buffer_done)(SWVENC_HANDLE    swvenc,
                                          SWVENC_OPBUFFER *p_opbuffer,
                                          void            *p_client);
    /**
     * @brief Callback function pointer to notify client of an event.
     *
     * @param swvenc:   SwVenc handle.
     * @param event:    Event.
     * @param p_client: Client handle.
     *
     * @retval SWVENC_S_SUCCESS if success, error status otherwise.
     */
    SWVENC_STATUS (*pfn_event_notification)(SWVENC_HANDLE swvenc,
                                            SWVENC_EVENT  event,
                                            void         *p_client);
    /**
     * @brief Client callback handle.
     */
    void *p_client;
} SWVENC_CALLBACK;

#ifdef __cplusplus
} // closing brace for: extern "C" {
#endif

#endif // #ifndef _SWVENC_TYPES_H_
