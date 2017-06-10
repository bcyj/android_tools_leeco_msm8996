/*=========================================================================
@file omx_wma_adec.h
This module contains the class definition for openMAX WMA decoder component.

Copyright (c) 2011-2012, 2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*=====================================================================*/

#ifndef _OMX_AAC_DEC_H_
#define _OMX_AAC_DEC_H_

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <semaphore.h>
#include <linux/msm_audio.h>
#include <linux/msm_audio_aac.h>
#include <linux/msm_ion.h>

#include "Map.h"
#include "adec_svr.h"
#include "OMX_Core.h"
#include "OMX_Audio.h"
#include "qc_omx_component.h"
#include "QOMX_AudioExtensions.h"
#include "QOMX_AudioIndexExtensions.h"


extern "C" {
    void * get_omx_component_factory_fn(void);
}

//////////////////////////////////////////////////////////////////////////////
//                       Module specific globals
//////////////////////////////////////////////////////////////////////////////
#define OMX_SPEC_VERSION  0x00000101
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (x >= y?x:y)
#define IP_PORT_BITMASK                 0x02
#define OP_PORT_BITMASK                 0x01
#define IP_OP_PORT_BITMASK              0x03

//////////////////////////////////////////////////////////////////////////////
//               Macros
////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
#define DEBUG_PRINT
#endif

#define PrintFrameHdr(i,bufHdr) \
    LOGV("i=0x%x Header[%p]pBuf[%p]FilledLen[0x%x]" \
    "TS[%d]nFlags[0x%x]\n",i,\
    (void *) bufHdr,                                     \
    (void *)((OMX_BUFFERHEADERTYPE *)bufHdr)->pBuffer,   \
    (unsigned)((OMX_BUFFERHEADERTYPE *)bufHdr)->nFilledLen,\
    (unsigned)((OMX_BUFFERHEADERTYPE *)bufHdr)->nTimeStamp,\
    (unsigned)((OMX_BUFFERHEADERTYPE *)bufHdr)->nFlags)

#define META_OUT_OFFSET    32
#define MONO                1
#define STEREO              2
#define CHMASK_STEREO           3
#define CHMASK_MONO         4
#define TUNNEL                          0
#define NON_TUNNEL                      1
#define BITS_PER_BYTE                   8

#define DEFAULT_BITS_PER_SAMPLE     16
#define OMX_CORE_NUM_INPUT_BUFFERS      4
#define OMX_CORE_NUM_OUTPUT_BUFFERS     4
#define OMX_CORE_INPUT_BUFFER_SIZE  4096   //TBD
#define OMX_AAC_OUTPUT_BUFFER_SIZE  (8192 + META_OUT_OFFSET)   //TBD
#define OMX_CORE_CONTROL_CMDQ_SIZE      100

#define OMX_ADEC_DEFAULT_SF          48000
#define OMX_ADEC_DEFAULT_CH_CFG      2
#define OMX_ADEC_AAC_FRAME_LEN       1024

// BitMask Management logic

#define BITMASK_SIZE(mIndex) \
    (((mIndex) + BITS_PER_BYTE - 1)/BITS_PER_BYTE)
#define BITMASK_OFFSET(mIndex)          ((mIndex)/BITS_PER_BYTE)
#define BITMASK_FLAG(mIndex)            (1 << ((mIndex) % BITS_PER_BYTE))
#define BITMASK_CLEAR(mArray,mIndex) \
    (mArray)[BITMASK_OFFSET(mIndex)] &=  ~(BITMASK_FLAG(mIndex))
#define BITMASK_SET(mArray,mIndex) \
    (mArray)[BITMASK_OFFSET(mIndex)] |=  BITMASK_FLAG(mIndex)
#define BITMASK_PRESENT(mArray,mIndex) \
    ((mArray)[BITMASK_OFFSET(mIndex)] & BITMASK_FLAG(mIndex))
#define BITMASK_ABSENT(mArray,mIndex) \
    (((mArray)[BITMASK_OFFSET(mIndex)] & BITMASK_FLAG(mIndex)) == 0x0)

#define DUMPS_ENABLE false

/* The following defines are used to extract all data from the AAC header.
** Each is divided into
**  the byte offset into the header
**  the byte mask for the bits
**  the number of bits to right-shift to extract a 0-based value
*/
#define AAC_SAMPLING_FREQ_INDEX_SIZE          4
#define AAC_ORIGINAL_COPY_SIZE                1
#define AAC_HOME_SIZE                         1
#define AAC_COPYRIGHT_PRESENT_SIZE            1
#define AAC_PROFILE_SIZE                      2
#define AAC_BITSTREAM_TYPE_SIZE               1
#define AAC_BITRATE_SIZE                      23
#define AAC_NUM_PFE_SIZE                      4
#define AAC_BUFFER_FULLNESS_SIZE              20
#define AAC_ELEMENT_INSTANCE_TAG_SIZE         4
#define AAC_NUM_FRONT_CHANNEL_ELEMENTS_SIZE   4
#define AAC_NUM_SIDE_CHANNEL_ELEMENTS_SIZE    4
#define AAC_NUM_BACK_CHANNEL_ELEMENTS_SIZE    4
#define AAC_NUM_LFE_CHANNEL_ELEMENTS_SIZE     2
#define AAC_NUM_ASSOC_DATA_ELEMENTS_SIZE      3
#define AAC_NUM_VALID_CC_ELEMENTS_SIZE        4
#define AAC_MONO_MIXDOWN_PRESENT_SIZE         1
#define AAC_MONO_MIXDOWN_ELEMENT_SIZE         4
#define AAC_STEREO_MIXDOWN_PRESENT_SIZE       1
#define AAC_STEREO_MIXDOWN_ELEMENT_SIZE       4
#define AAC_MATRIX_MIXDOWN_PRESENT_SIZE       1
#define AAC_MATRIX_MIXDOWN_SIZE               3
#define AAC_FCE_SIZE                          5
#define AAC_SCE_SIZE                          5
#define AAC_BCE_SIZE                          5
#define AAC_LFE_SIZE                          4
#define AAC_ADE_SIZE                          4
#define AAC_VCE_SIZE                          5
#define AAC_COMMENT_FIELD_BYTES_SIZE          8
#define AAC_COMMENT_FIELD_DATA_SIZE           8


#define AAC_MONO_SILENCE_FRAME_SIZE           10
#define AAC_STEREO_SILENCE_FRAME_SIZE         11
//10 bytes
OMX_U8 AAC_MONO_SILENCE_FRAME_DATA[]   =
{0x01, 0x40, 0x20, 0x06, 0x4F, 0xDE, 0x02, 0x70, 0x0C, 0x1C};
// 11 bytes
OMX_U8 AAC_STEREO_SILENCE_FRAME_DATA[] =
{0x21, 0x10, 0x05, 0x00, 0xA0, 0x19, 0x33, 0x87, 0xC0, 0x00, 0x7E};
#define TIMESTAMP_HALFRANGE_THRESHOLD 0x7D0

#define LOAS_GA_SPECIFIC_CONFIG(o) \
    (((o != 5) && (o >=1 ) && (o <= 7)) || \
    ((o != 18) && (o >= 17) && (o <= 23)))

#define LOAS_IS_AUD_OBJ_SUPPORTED(x) \
    ((x == 2) || (x == 4) || (x == 5) || (x == 17))

#define LOAS_IS_SFI_SUPPORTED(x) ((x >= 3) && (x <= 0x0B))

/* c is channel config and o is Audio object type */
#define LOAS_IS_CHANNEL_CONFIG_SUPPORTED(c, o) \
    (((c <= 2) && ((o == 2) || (o == 4) || (o == 5))) || \
    (((c == 1) || (c == 2)) && (o == 17)))

#define LOAS_IS_EXT_SFI_SUPPORTED(x)  ((x >= 0x03) && (x <= 0x08))

/* Extension Flag is e and Audio object type is o */
#define LOAS_IS_EXT_FLG_SUPPORTED(e,o) \
    ((((o == 2) || (o == 4) || (o == 5)) && (e == 0)) || \
    ((o == 17) && (e == 1)))

#define ION_ALLOC_ALIGN 0x1000

//////////////////////////////////////////////////////////////////
//          TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////
enum in_format{
    FORMAT_ADTS = 0,
    FORMAT_ADIF = 1,
    FORMAT_LOAS = 2,
    FORMAT_RAW  = 3,
};
struct adts_fixed_header
{
    OMX_U16 sync_word;
    OMX_U8  id;
    OMX_U8  layer;
    OMX_U8  protection_absent;
    OMX_U8  profile;
    OMX_U8  sampling_frequency_index;
    OMX_U8  private_bit;
    OMX_U8  channel_configuration;
    OMX_U8  original_copy;
    OMX_U8  home;
    OMX_U8  emphasis;
};
struct adts_var_header
{
    OMX_U8  copyright_id_bit;
    OMX_U8  copyright_id_start;
    OMX_U16 aac_frame_length;
    OMX_U8  adts_buffer_fullness;
    OMX_U8  no_raw_data_blocks_in_frame;
};
struct adts_header
{
    struct adts_fixed_header fixed;
    struct adts_var_header   var;
};
struct aac_raw
{
    OMX_U8 aud_obj_type;
    OMX_U8 freq_index;
    OMX_U8 channel_config;
    OMX_U8 sbr_present_flag;
    OMX_U8 sbr_ps_present_flag;
    OMX_U8 ext_aud_obj_type;
    OMX_U8 ext_freq_index;
    OMX_U8 ext_channel_config;
};
struct adif_header
{
    OMX_U8 variable_bit_rate;
    OMX_U8 aud_obj_type;
    OMX_U8 freq_index;
    OMX_U8 channel_config;
    OMX_U32 sample_rate;
};
struct loas_header
{
    OMX_U8 aud_obj_type;
    OMX_U8 freq_index;
    OMX_U8 channel_config;
};
struct aac_header
{
    in_format input_format;
    union
    {
        struct adts_header adts;
        struct adif_header adif;
        struct loas_header loas;
        struct aac_raw     raw;
    }head;
};
typedef enum {
    AAC_CHANNEL_UNKNOWN = 0,
    AAC_CHANNEL_MONO,       /* Single channel (mono) data*/
    AAC_CHANNEL_DUAL,       /* Stereo data*/
    AAC_CHANNEL_TRIPLE,     /* 3 channels: 1+2 (UNSUPPORTED)*/
    AAC_CHANNEL_QUAD,       /* 4 channels: 1+2+1 (UNSUPPORTED)*/
    AAC_CHANNEL_QUINTUPLE,  /* 5 channels: 1+2+2 (UNSUPPORTED)*/
    AAC_CHANNEL_SEXTUPLE,   /* 5+1 channels: 1+2+2+1 (UNSUPPORTED)*/
    AAC_CHANNEL_OCTUPLE,    /* 7+1 channels: 1+2+2+2+1 (UNSUPPORTED)*/
    AAC_CHANNEL_DUAL_MONO,  /* Dual Mono: 1+1 (Two SCEs)*/
    AAC_CHANNEL_UNSUPPORTED /* Indicating CMX is currently playing*/
    /* unsupported Channel mode.*/
} aac_channel_enum_type;

static OMX_U32 aac_frequency_index[16] = { 96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350,
    0x000,//invalid index
    0x000,//invalid index
    0x000 // no index, value provided is actual frequency
};


class COmxAacDec;

// OMX aac audio decoder class
class COmxAacDec: public qc_omx_component
{
public:

    COmxAacDec();
    virtual ~COmxAacDec();

    OMX_ERRORTYPE component_init(OMX_STRING role);
    OMX_ERRORTYPE component_deinit(OMX_HANDLETYPE hComp);

    static void process_in_port_msg(void *client_data, unsigned char id);
    static void process_out_port_msg(void *client_data, unsigned char id);
    static void process_command_msg(void *client_data, unsigned char id);
    static void process_event_cb(void *client_data, unsigned char id);

    OMX_ERRORTYPE free_buffer(OMX_HANDLETYPE hComp,
    OMX_U32              port,
    OMX_BUFFERHEADERTYPE *buffer);

    OMX_ERRORTYPE empty_this_buffer(OMX_HANDLETYPE hComp,
    OMX_BUFFERHEADERTYPE *buffer);

    OMX_ERRORTYPE fill_this_buffer(OMX_HANDLETYPE hComp,
    OMX_BUFFERHEADERTYPE *buffer);

    OMX_ERRORTYPE allocate_buffer(OMX_HANDLETYPE hComp,
    OMX_BUFFERHEADERTYPE **bufferHdr,
    OMX_U32              port,
    OMX_PTR              appData,
    OMX_U32              bytes);

    OMX_ERRORTYPE use_buffer(OMX_HANDLETYPE hComp,
    OMX_BUFFERHEADERTYPE **bufferHdr,
    OMX_U32              port,
    OMX_PTR              appData,
    OMX_U32              bytes,
    OMX_U8               *buffer);

    OMX_ERRORTYPE component_role_enum(OMX_HANDLETYPE hComp,
    OMX_U8         *role,
    OMX_U32        index);

    OMX_ERRORTYPE get_config(OMX_HANDLETYPE hComp,
    OMX_INDEXTYPE  configIndex,
    OMX_PTR        configData);

    OMX_ERRORTYPE get_parameter(OMX_HANDLETYPE hComp,
    OMX_INDEXTYPE  paramIndex,
    OMX_PTR        paramData);

    OMX_ERRORTYPE get_state(OMX_HANDLETYPE hComp,
    OMX_STATETYPE  *state);

    OMX_ERRORTYPE get_extension_index(OMX_HANDLETYPE hComp,
    OMX_STRING     paramName,
    OMX_INDEXTYPE  *indexType);

    OMX_ERRORTYPE get_component_version(OMX_HANDLETYPE hComp,
    OMX_STRING      componentName,
    OMX_VERSIONTYPE *componentVersion,
    OMX_VERSIONTYPE *specVersion,
    OMX_UUIDTYPE    *componentUUID);

    OMX_ERRORTYPE set_callbacks(OMX_HANDLETYPE hComp,
    OMX_CALLBACKTYPE *callbacks,
    OMX_PTR          appData);

    OMX_ERRORTYPE set_config(OMX_HANDLETYPE hComp,
    OMX_INDEXTYPE  configIndex,
    OMX_PTR        configData);

    OMX_ERRORTYPE set_parameter(OMX_HANDLETYPE hComp,
    OMX_INDEXTYPE  paramIndex,
    OMX_PTR        paramData);

    OMX_ERRORTYPE send_command(OMX_HANDLETYPE  hComp,
    OMX_COMMANDTYPE cmd,
    OMX_U32         param1,
    OMX_PTR         cmdData);

    OMX_ERRORTYPE use_EGL_image(OMX_HANDLETYPE hComp,
    OMX_BUFFERHEADERTYPE **bufferHdr,
    OMX_U32              port,
    OMX_PTR              appData,
    void *               eglImage);

    OMX_ERRORTYPE component_tunnel_request(OMX_HANDLETYPE hComp,
    OMX_U32             port,
    OMX_HANDLETYPE      peerComponent,
    OMX_U32             peerPort,
    OMX_TUNNELSETUPTYPE *tunnelSetup);

    bool post_command(unsigned long p1, unsigned long p2, unsigned char id);

    // Deferred callback identifiers
    enum
    {
        OMX_COMPONENT_GENERATE_EVENT       = 0x1,
        OMX_COMPONENT_GENERATE_BUFFER_DONE = 0x2,
        OMX_COMPONENT_GENERATE_ETB         = 0x3,
        OMX_COMPONENT_GENERATE_COMMAND     = 0x4,
        OMX_COMPONENT_GENERATE_FRAME_DONE  = 0x5,
        OMX_COMPONENT_GENERATE_FTB         = 0x6,
        OMX_COMPONENT_GENERATE_EOS         = 0x7,
        OMX_COMPONENT_PORTSETTINGS_CHANGED = 0x8,
        OMX_COMPONENT_SUSPEND              = 0x09,
        OMX_COMPONENT_RESUME               = 0x0a,
        OMX_COMPONENT_STREAM_INFO          = 0x0b
    };

    inline OMX_U8 get_eos_bm()
    {
        return m_eos_bm;
    }
    inline void set_eos_bm(OMX_U8 eos)
    {
        m_eos_bm = eos;
    }
    inline void set_is_pause_to_exe(bool flg)
    {
        m_pause_to_exe = flg;
    }
    inline bool is_pause_to_exe()
    {
        return m_pause_to_exe;
    }
    inline OMX_STATETYPE get_state()
    {
        pthread_mutex_lock(&m_state_lock);
        OMX_STATETYPE state = m_state;
        pthread_mutex_unlock(&m_state_lock);
        return state;
    }
    inline void set_state(OMX_STATETYPE state)
    {
        pthread_mutex_lock(&m_state_lock);
        m_state = state;
        pthread_mutex_unlock(&m_state_lock);
    }
    inline int get_drv_fd()
    {
        return m_drv_fd;
    }
    inline void set_drv_fd(int fd)
    {
        m_drv_fd = fd;
    }
    inline void reset_drv_fd()
    {
        m_drv_fd = -1;
    }
    inline void inc_num_il_in_buf()
    {
    pthread_mutex_lock(&in_buf_count_lock);
        ++nNumInputBuf;
    pthread_mutex_unlock(&in_buf_count_lock);
    return;
    }
    inline void dec_num_il_in_buf()
    {
    pthread_mutex_lock(&in_buf_count_lock);
        --nNumInputBuf;
    pthread_mutex_unlock(&in_buf_count_lock);
    return;
    }
    inline int get_num_il_in_buf()
    {
    int num;
    pthread_mutex_lock(&in_buf_count_lock);
        num = nNumInputBuf;
    pthread_mutex_unlock(&in_buf_count_lock);
    return num;
    }
    inline void reset_num_il_in_buf()
    {
    pthread_mutex_lock(&in_buf_count_lock);
        nNumInputBuf = 0;
    pthread_mutex_unlock(&in_buf_count_lock);
    return;
    }
    inline void inc_num_drv_in_buf()
    {
    pthread_mutex_lock(&in_buf_count_lock);
        ++drv_inp_buf_cnt;
    pthread_mutex_unlock(&in_buf_count_lock);
    return;
    }
    inline void dec_num_drv_in_buf()
    {
    pthread_mutex_lock(&in_buf_count_lock);
        --drv_inp_buf_cnt;
    pthread_mutex_unlock(&in_buf_count_lock);
    return;
    }
    inline int get_num_drv_in_buf()
    {
    int num;
    pthread_mutex_lock(&in_buf_count_lock);
        num = drv_inp_buf_cnt;
    pthread_mutex_unlock(&in_buf_count_lock);
        return num;
    }
    inline void reset_num_drv_in_buf()
    {
    pthread_mutex_lock(&in_buf_count_lock);
        drv_inp_buf_cnt = 0;
    pthread_mutex_unlock(&in_buf_count_lock);
    return;
    }
   inline void inc_num_drv_out_buf()
    {
    pthread_mutex_lock(&out_buf_count_lock);
        ++drv_out_buf_cnt;
    pthread_mutex_unlock(&out_buf_count_lock);
    return;
    }
    inline void dec_num_drv_out_buf()
    {
    pthread_mutex_lock(&out_buf_count_lock);
        --drv_out_buf_cnt;
    pthread_mutex_unlock(&out_buf_count_lock);
    return;
    }
    inline int get_num_drv_out_buf()
    {
    int num;
        pthread_mutex_lock(&out_buf_count_lock);
        num = drv_out_buf_cnt;
    pthread_mutex_unlock(&out_buf_count_lock);
    return num;
    }
    inline void reset_num_drv_out_buf()
    {
    pthread_mutex_lock(&out_buf_count_lock);
        drv_out_buf_cnt = 0;
    pthread_mutex_unlock(&out_buf_count_lock);
    return;
    }

    struct mmap_info
    {
        void* pBuffer;
        unsigned map_buf_size;
        unsigned filled_len;
        struct ion_fd_data ion_fd_data;
        struct ion_allocation_data ion_alloc_data;
    };
private:

    enum port_indexes
    {
        OMX_CORE_INPUT_PORT_INDEX   = 0,
        OMX_CORE_OUTPUT_PORT_INDEX  = 1

    };
    // Bit Positions
    enum flags_bit_positions
    {
        OMX_COMPONENT_IDLE_PENDING            =0x1,
        OMX_COMPONENT_LOADING_PENDING         =0x2,
        OMX_COMPONENT_MUTED                   =0x3,
        // Defer transition to Enable
        OMX_COMPONENT_INPUT_ENABLE_PENDING    =0x4,
        // Defer transition to Enable
        OMX_COMPONENT_OUTPUT_ENABLE_PENDING   =0x5,
        // Defer transition to Disable
        OMX_COMPONENT_INPUT_DISABLE_PENDING   =0x6,
        // Defer transition to Disable
        OMX_COMPONENT_OUTPUT_DISABLE_PENDING  =0x7
    };

    struct omx_event
    {
        unsigned long param1;
        unsigned long param2;
        unsigned char id;
    };

    struct omx_cmd_queue
    {
        omx_event m_q[OMX_CORE_CONTROL_CMDQ_SIZE];
        unsigned m_read;
        unsigned m_write;
        unsigned m_size;
        omx_cmd_queue();
        ~omx_cmd_queue();
        bool insert_entry(unsigned long p1, unsigned long p2, unsigned char id);
        bool pop_entry(unsigned long *p1,unsigned long *p2, unsigned char *id);
        bool get_msg_id(unsigned char *id);
    };

    typedef struct TIMESTAMP
    {
        unsigned int LowPart;
        unsigned int HighPart;
    }__attribute__((packed)) TIMESTAMP;

    typedef struct metadata_input
    {
        OMX_U8    reserved[18];
        OMX_U16   offsetVal;
        TIMESTAMP nTimeStamp;
        OMX_U32   nFlags;
    }__attribute__((packed)) META_IN;

    typedef struct metadata_output {
        unsigned int reserved[7];
        unsigned int num_of_frames;
    }__attribute__ ((packed))META_OUT;

    typedef struct dec_meta_out
    {
        unsigned int offset_to_frame;
        unsigned int frame_size;
        unsigned int encoded_pcm_samples;
        unsigned int msw_ts;
        unsigned int lsw_ts;
        unsigned int nflags;
    } __attribute__ ((packed))DEC_META_OUT;

    typedef Map<OMX_BUFFERHEADERTYPE*, OMX_BUFFERHEADERTYPE*>
    input_buffer_map;

    typedef Map<OMX_BUFFERHEADERTYPE*, OMX_BUFFERHEADERTYPE*>
    output_buffer_map;

    typedef Map<struct mmap_info*, struct mmap_info*> drv_out_buf;

    OMX_S32                      m_volume;
    OMX_PTR                      m_app_data;

    OMX_U8                       m_eos_bm;
    OMX_U8                       m_flush_cnt;
    OMX_U8                       m_first_aac_header;
    unsigned int                 m_bytes_to_skip;
    unsigned                     m_aac_hdr_bit_index;
    OMX_U8                       m_is_alloc_buf;

    int                          drv_inp_buf_cnt;
    int                          drv_out_buf_cnt;
    int                          nNumInputBuf;
    int                          nNumOutputBuf;
    int                          m_drv_fd;
    int                          ionfd;

    bool                         m_to_idle;
    bool                         m_pause_to_exe;
    bool                         is_in_th_sleep;
    bool                         is_out_th_sleep;
    bool                         bFlushinprogress;
    bool                         bOutputPortReEnabled;
    bool                         m_in_use_buf_case;
    bool                         m_out_use_buf_case;
    unsigned short               m_session_id;
    unsigned int                 m_inp_act_buf_count;
    unsigned int                 m_out_act_buf_count;
    unsigned int                 m_inp_current_buf_count;
    unsigned int                 m_out_current_buf_count;
    unsigned int                 m_comp_deinit;

    unsigned int                 m_flags;
    unsigned int                 m_fbd_cnt;
    OMX_U64                      nTimestamp;
    unsigned int                 pcm_feedback;
    unsigned int                 output_buffer_size;
    unsigned int                 input_buffer_size;
    unsigned int                 pSamplerate;
    unsigned int                 pChannels;
    unsigned int                 pBitrate ;
    unsigned int                 m_ftb_cnt;
    unsigned int                 m_drv_buf_cnt;

    bool                         adif;

    volatile int                 m_is_event_done;
    volatile int                 m_is_in_th_sleep;
    volatile int                 m_is_out_th_sleep;
    volatile int                 m_flush_cmpl_event;

    omx_cmd_queue                m_cmd_q;
    omx_cmd_queue                m_din_q;
    omx_cmd_queue                m_cin_q;
    omx_cmd_queue                m_dout_q;
    omx_cmd_queue                m_cout_q;
    omx_cmd_queue                m_fbd_q;
    omx_cmd_queue                m_ebd_q;

    sem_t                        sem_States;
    sem_t            sem_flush_cmpl_state;
    pthread_cond_t               cond;
    pthread_cond_t               in_cond;
    pthread_cond_t               out_cond;
    pthread_mutex_t              m_inputlock;
    pthread_mutex_t              m_commandlock;
    pthread_mutex_t              m_outputlock;
    pthread_mutex_t              m_flush_lock;
    pthread_mutex_t              m_event_lock;
    pthread_mutex_t              m_in_th_lock;
    pthread_mutex_t              m_state_lock;
    pthread_mutex_t              m_in_th_lock_1;
    pthread_mutex_t              m_out_th_lock;
    pthread_mutex_t              m_out_th_lock_1;
    pthread_mutex_t              out_buf_count_lock;
    pthread_mutex_t              in_buf_count_lock;
    pthread_mutex_t              m_flush_cmpl_lock;

    pthread_mutexattr_t          m_state_lock_attr;
    pthread_mutexattr_t          m_flush_attr;
    pthread_mutexattr_t          m_outputlock_attr;
    pthread_mutexattr_t          m_commandlock_attr;
    pthread_mutexattr_t          m_inputlock_attr;
    pthread_mutexattr_t          m_in_th_attr_1;
    pthread_mutexattr_t          m_in_th_attr;
    pthread_mutexattr_t          m_out_th_attr_1;
    pthread_mutexattr_t          m_out_th_attr;
    pthread_mutexattr_t          out_buf_count_lock_attr;
    pthread_mutexattr_t          in_buf_count_lock_attr;
    pthread_mutexattr_t          m_flush_cmpl_attr;
    pthread_mutexattr_t          m_event_attr;

    input_buffer_map             m_input_buf_hdrs;
    output_buffer_map            m_output_buf_hdrs;
    input_buffer_map             m_loc_in_use_buf_hdrs;
    output_buffer_map            m_loc_out_use_buf_hdrs;
    drv_out_buf                  m_drv_out_buf;

    struct aac_ipc_info          *m_ipc_to_in_th;
    struct aac_ipc_info          *m_ipc_to_out_th;
    struct aac_ipc_info          *m_ipc_to_cmd_th;
    struct aac_ipc_info          *m_ipc_to_event_th;

    OMX_BOOL                     m_inp_bEnabled;
    OMX_BOOL                     m_inp_bPopulated;
    OMX_BOOL                     m_out_bEnabled;
    OMX_BOOL                     m_out_bPopulated;
    OMX_STATETYPE                m_state;
    OMX_CALLBACKTYPE             m_cb;
    OMX_PRIORITYMGMTTYPE         m_priority_mgm ;
    OMX_PARAM_BUFFERSUPPLIERTYPE m_buffer_supplier;

    OMX_AUDIO_PARAM_AACPROFILETYPE  m_adec_param;
    QOMX_AUDIO_CONFIG_DUALMONOTYPE  m_adec_config_dualmono;

    OMX_SUSPENSIONPOLICYTYPE     suspensionPolicy;
    OMX_AUDIO_PARAM_PCMMODETYPE  m_pcm_param;
    OMX_PARAM_COMPONENTROLETYPE  component_Role;

    ////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////
    void buffer_done_cb(OMX_BUFFERHEADERTYPE *bufHdr, bool flg = false);

    void frame_done_cb(OMX_BUFFERHEADERTYPE *bufHdr, bool flg = false);

    OMX_ERRORTYPE allocate_input_buffer(OMX_HANDLETYPE       hComp,
    OMX_BUFFERHEADERTYPE **bufferHdr,
    OMX_U32              port,
    OMX_PTR              appData,
    OMX_U32              bytes);
    OMX_ERRORTYPE  allocate_output_buffer(
    OMX_IN OMX_HANDLETYPE            hComp,
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
    OMX_IN OMX_U32                   port,
    OMX_IN OMX_PTR                   appData,
    OMX_IN OMX_U32                   bytes);

    OMX_ERRORTYPE  use_input_buffer(
    OMX_IN OMX_HANDLETYPE            hComp,
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
    OMX_IN OMX_U32                   port,
    OMX_IN OMX_PTR                   appData,
    OMX_IN OMX_U32                   bytes,
    OMX_IN OMX_U8*                   buffer);

    OMX_ERRORTYPE  use_output_buffer(
    OMX_IN OMX_HANDLETYPE            hComp,
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
    OMX_IN OMX_U32                   port,
    OMX_IN OMX_PTR                   appData,
    OMX_IN OMX_U32                   bytes,
    OMX_IN OMX_U8*                   buffer);

    OMX_ERRORTYPE empty_this_buffer_proxy(OMX_HANDLETYPE       hComp,
    OMX_BUFFERHEADERTYPE *buffer);

    OMX_ERRORTYPE fill_this_buffer_proxy(OMX_HANDLETYPE       hComp,
    OMX_BUFFERHEADERTYPE *buffer);

    OMX_ERRORTYPE send_command_proxy(OMX_HANDLETYPE hComp,
    OMX_COMMANDTYPE  cmd,
    OMX_U32       param1,
    OMX_PTR      cmdData);

    bool post_input(unsigned long p1, unsigned long p2, unsigned char id);
    bool post_output(unsigned long p1, unsigned long p2, unsigned char id);

    void process_events(COmxAacDec *client_data);

    bool allocate_done(void);
    bool release_done(OMX_U32 param1);

    bool execute_omx_flush(OMX_IN OMX_U32 param1, bool cmd_cmpl=true);
    bool execute_input_omx_flush(void);
    bool execute_output_omx_flush(void);

    bool search_input_bufhdr(OMX_BUFFERHEADERTYPE *buffer);
    bool search_output_bufhdr(OMX_BUFFERHEADERTYPE *buffer);

    bool prepare_for_ebd(OMX_BUFFERHEADERTYPE *bufHdr);

    bool audio_register_ion(struct mmap_info *ion_buf);
    bool audio_deregister_ion(struct mmap_info *ion_buf);
    bool aio_write(msm_audio_aio_buf *audio_aio_buf);
    bool aio_read(msm_audio_aio_buf *audio_aio_buf);
    void wait_for_event();

    void event_complete();
    void wait_for_flush_event();
    void event_flush_complete();
    void in_th_sleep();
    void out_th_sleep();
    void in_sleep();
    void out_sleep();

    void* alloc_ion_buffer(unsigned int bufsize);
    void free_ion_buffer(void** ion_buffer);

    void in_th_goto_sleep();
    void in_th_wakeup();
    void out_th_goto_sleep();
    void out_th_wakeup();
    void flush_ack();
    void deinit_decoder();

    OMX_ERRORTYPE  aac_frameheader_parser(OMX_BUFFERHEADERTYPE *buffer,
                                          struct aac_header    *header);

    OMX_ERRORTYPE  set_aac_config(OMX_AUDIO_PARAM_AACPROFILETYPE  *m_adec_param_tmp);

    void audaac_extract_adif_header(OMX_U8 *data,
                                    struct aac_header *aac_header_info);

    int audaac_extract_loas_header(OMX_U8 *data, OMX_U32 len,
                                   struct aac_header *aac_header_info);

    void audaac_extract_bits(OMX_U8*, OMX_U8, OMX_U32*);
};
#endif // _OMX_AAC_DEC_H_
