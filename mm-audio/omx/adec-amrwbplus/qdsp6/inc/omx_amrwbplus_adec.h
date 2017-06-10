/*=========================================================================
@file omx_amrwbplus_adec.h
This module contains the class definition for openMAX AMR-WB+ decoder component.

Copyright (c) 2010, 2012, 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*//*=====================================================================*/

#ifndef _OMX_AMRWBPLUS_DEC_H_
#define _OMX_AMRWBPLUS_DEC_H_

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
#include <linux/msm_audio_amrwbplus.h>
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
    DEBUG_PRINT("i=0x%x Header[%p]pBuf[%p]FilledLen[0x%x]" \
    "TS[0x%x]nFlags[0x%x]\n",i,\
    (void *) bufHdr,                                     \
    (void *)((OMX_BUFFERHEADERTYPE *)bufHdr)->pBuffer,   \
    (unsigned)((OMX_BUFFERHEADERTYPE *)bufHdr)->nFilledLen,\
    (unsigned)((OMX_BUFFERHEADERTYPE *)bufHdr)->nTimeStamp,\
    (unsigned)((OMX_BUFFERHEADERTYPE *)bufHdr)->nFlags)


#define MONO                              1
#define STEREO                            2
#define CHMASK_STEREO                     3
#define CHMASK_MONO                       4
#define TUNNEL                            0
#define NON_TUNNEL                        1
#define BITS_PER_BYTE                     8
#define OMX_ADEC_MIN                      0
#define OMX_ADEC_MAX                      100
#define DEFAULT_BITS_PER_SAMPLE           16
#define DEFAULT_SAMPLING_RATE             44100
#define DEFAULT_CHANNEL_MODE              2
#define OMX_CORE_NUM_INPUT_BUFFERS        2
#define OMX_CORE_NUM_OUTPUT_BUFFERS       2
#define OMX_CORE_INPUT_BUFFER_SIZE        1024
#define OMX_AMRWBPLUS_OUTPUT_BUFFER_SIZE  8192
#define OMX_CORE_CONTROL_CMDQ_SIZE        100

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


#define ION_ALLOC_ALIGN 0x1000

class COmxAmrwbplusDec;

// OMX amrwbplus audio decoder class
class COmxAmrwbplusDec: public qc_omx_component
{
public:

    COmxAmrwbplusDec();
    virtual ~COmxAmrwbplusDec();

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

    OMX_ERRORTYPE use_buffer( OMX_HANDLETYPE hComp,
                              OMX_BUFFERHEADERTYPE **bufferHdr,
                              OMX_U32              port,
                              OMX_PTR              appData,
                              OMX_U32              bytes,
                              OMX_U8               *buffer);

    OMX_ERRORTYPE component_role_enum(OMX_HANDLETYPE hComp,
                                      OMX_U8         *role,
                                      OMX_U32        index);

    OMX_ERRORTYPE get_config( OMX_HANDLETYPE hComp,
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

    OMX_ERRORTYPE set_config( OMX_HANDLETYPE hComp,
                              OMX_INDEXTYPE  configIndex,
                              OMX_PTR        configData);

    OMX_ERRORTYPE set_parameter(OMX_HANDLETYPE hComp,
                                OMX_INDEXTYPE  paramIndex,
                                OMX_PTR        paramData);

    OMX_ERRORTYPE send_command( OMX_HANDLETYPE  hComp,
                                OMX_COMMANDTYPE cmd,
                                OMX_U32         param1,
                                OMX_PTR         cmdData);

    OMX_ERRORTYPE use_EGL_image(OMX_HANDLETYPE hComp,
                                OMX_BUFFERHEADERTYPE **bufferHdr,
                                OMX_U32              port,
                                OMX_PTR              appData,
                                void *               eglImage);

    OMX_ERRORTYPE component_tunnel_request( OMX_HANDLETYPE hComp,
                                            OMX_U32             port,
                                            OMX_HANDLETYPE      peerComponent,
                                            OMX_U32             peerPort,
                                            OMX_TUNNELSETUPTYPE *tunnelSetup);

    bool post_command(unsigned int p1, unsigned int p2, unsigned char id);

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
        OMX_COMPONENT_RESUME               = 0x0a
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

    OMX_S32                             m_volume;
    OMX_PTR                             m_app_data;

    OMX_U8                              m_eos_bm;
    OMX_U8                              m_flush_cnt;
    OMX_U8                              m_first_amrwbplus_header;
    OMX_U8                              m_is_alloc_buf;

    int                                 drv_inp_buf_cnt;
    int                                 drv_out_buf_cnt;
    int                                 nNumInputBuf;
    int                                 nNumOutputBuf;
    int                                 m_drv_fd;
    int                                 ionfd;

    bool                                m_to_idle;
    bool                                m_pause_to_exe;
    bool                                is_in_th_sleep;
    bool                                is_out_th_sleep;
    bool                                bFlushinprogress;
    bool                                bOutputPortReEnabled;
    bool                                m_in_use_buf_case;
    bool                                m_out_use_buf_case;
    OMX_U8                              m_session_id;
    OMX_U32                             m_inp_act_buf_count;
    OMX_U32                             m_out_act_buf_count;
    unsigned int                        m_inp_current_buf_count;
    unsigned int                        m_out_current_buf_count;
    unsigned int                        m_comp_deinit;

    unsigned int                        m_flags;
    unsigned int                        m_fbd_cnt;
    OMX_U64                             nTimestamp;
    unsigned int                        pcm_feedback;
    OMX_U32                             output_buffer_size;
    OMX_U32                             input_buffer_size;
    unsigned int                        pSamplerate;
    unsigned int                        pChannels;
    unsigned int                        pBitrate ;
    unsigned int                        m_ftb_cnt;
    unsigned int                        m_drv_buf_cnt;

    volatile int                        m_is_event_done;
    volatile int                        m_is_in_th_sleep;
    volatile int                        m_is_out_th_sleep;
    volatile int                        m_flush_cmpl_event;

    omx_cmd_queue                       m_cmd_q;
    omx_cmd_queue                       m_din_q;
    omx_cmd_queue                       m_cin_q;
    omx_cmd_queue                       m_dout_q;
    omx_cmd_queue                       m_cout_q;
    omx_cmd_queue                       m_fbd_q;
    omx_cmd_queue                       m_ebd_q;

    sem_t                               sem_States;
    sem_t                               sem_flush_cmpl_state;
    pthread_cond_t                      cond;
    pthread_cond_t                      in_cond;
    pthread_cond_t                      out_cond;
    pthread_mutex_t                     m_inputlock;
    pthread_mutex_t                     m_commandlock;
    pthread_mutex_t                     m_outputlock;
    pthread_mutex_t                     m_flush_lock;
    pthread_mutex_t                     m_event_lock;
    pthread_mutex_t                     m_in_th_lock;
    pthread_mutex_t                     m_state_lock;
    pthread_mutex_t                     m_in_th_lock_1;
    pthread_mutex_t                     m_out_th_lock;
    pthread_mutex_t                     m_out_th_lock_1;
    pthread_mutex_t                     out_buf_count_lock;
    pthread_mutex_t                     in_buf_count_lock;
    pthread_mutex_t                     m_flush_cmpl_lock;

    pthread_mutexattr_t                 m_state_lock_attr;
    pthread_mutexattr_t                 m_flush_attr;
    pthread_mutexattr_t                 m_outputlock_attr;
    pthread_mutexattr_t                 m_commandlock_attr;
    pthread_mutexattr_t                 m_inputlock_attr;
    pthread_mutexattr_t                 m_in_th_attr_1;
    pthread_mutexattr_t                 m_in_th_attr;
    pthread_mutexattr_t                 m_out_th_attr_1;
    pthread_mutexattr_t                 m_out_th_attr;
    pthread_mutexattr_t                 out_buf_count_lock_attr;
    pthread_mutexattr_t                 in_buf_count_lock_attr;
    pthread_mutexattr_t                 m_flush_cmpl_attr;
    pthread_mutexattr_t                 m_event_attr;

    input_buffer_map                    m_input_buf_hdrs;
    output_buffer_map                   m_output_buf_hdrs;
    input_buffer_map                    m_loc_in_use_buf_hdrs;
    output_buffer_map                   m_loc_out_use_buf_hdrs;
    drv_out_buf                         m_drv_out_buf;

    struct amrwbplus_ipc_info           *m_ipc_to_in_th;
    struct amrwbplus_ipc_info           *m_ipc_to_out_th;
    struct amrwbplus_ipc_info           *m_ipc_to_cmd_th;
    struct amrwbplus_ipc_info           *m_ipc_to_event_th;

    OMX_BOOL                            m_inp_bEnabled;
    OMX_BOOL                            m_inp_bPopulated;
    OMX_BOOL                            m_out_bEnabled;
    OMX_BOOL                            m_out_bPopulated;
    OMX_STATETYPE                       m_state;
    OMX_CALLBACKTYPE                    m_cb;
    OMX_PRIORITYMGMTTYPE                m_priority_mgm ;
    OMX_PARAM_BUFFERSUPPLIERTYPE        m_buffer_supplier;

    QOMX_AUDIO_PARAM_AMRWBPLUSTYPE      m_adec_param;
    OMX_SUSPENSIONPOLICYTYPE            suspensionPolicy;
    OMX_AUDIO_PARAM_PCMMODETYPE         m_pcm_param;
    OMX_PARAM_COMPONENTROLETYPE         component_Role;
    bool                                m_error_propogated;
    ///////////////////////////////////////////////////////////////////
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

    void process_events(COmxAmrwbplusDec *client_data);

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
    bool is_platform_8960();
};
#endif // _OMX_AMRWBPLUS_DEC_H_
