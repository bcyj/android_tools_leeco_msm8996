#ifndef __MUX_MP2BASEFile_H__
#define __MUX_MP2BASEFILE_H__

/* =======================================================================
                              MP2BaseFile.h
DESCRIPTION

  Copyright (c) 2011 - 2013 Qualcomm Technologies, Inc.All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */


/* =======================================================================
                             Edit History
$Source: //source/qcom/qct/multimedia2/Video/Source/FileMux/Sink/FileMux/MP2BaseFileLib/main/latest/inc/MP2BaseFile.h

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "MMTimer.h"
#include "oscl_file_io.h"
#include "filemuxtypes.h"
#include "muxbase.h"
#include "MMCriticalSection.h"
/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define FS_FILE_EXTENSION_DOT              '.'
#define ASYNC_WRITER_FRAGMENT_BUF_SIZE(x)     (1024 * 1024)
#define ASYNC_WRITER_BUF_SIZE(x)              (256 * 1024)
#define MUX_MEM_MARGIN                        65536
#define EFS_MARGIN                            65536
#define CHUNK_TRANSFER_SIZE                   1000
#define TS_RTP_PAYLOAD_SIZE                   (7 * 188)

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

//! MEM file object type
typedef struct MP2_MUX_mem_object_struct {
    uint8     *buf_ptr;
    uint32    buf_size;
    uint32    length;
    uint32    pos;
    boolean   free_buf;
} MP2_MUX_mem_object_type;


#ifdef FILEMUX_WRITE_ASYNC
typedef struct mp2_stream_handler_info
{
    uint8     *pBuffer;        //!Buffer pointer
    uint32     pHead;          //!Head of the buf
    uint32     pTail;          //!Tail of the buf
    uint32     nSize;          //!Total Size of the buf
    uint32     nFlushThreshold;//!Threshold to start flushing the buffer
    boolean    bOverflow;      //!Queue overflow
    uint32     nBytesOverflow; //!Number of bytes that is overflown
    boolean    bFlushing;      //!Buffer is being drained
}Mux_mp2_stream_handler_info_type;
#endif /*FILEMUX_WRITE_ASYNC*/

typedef struct MP2_MUX_file_struct
{
  MP2_MUX_mem_object_type *mem_file;
  OSCL_FILE *efs_file;
}MP2_MUX_file;

typedef enum Mux_AAC_Format
{
    AAC_FORMAT_ADTS,
    AAC_FORMAT_LOAS
}Mux_AAC_Format_Type;
/*!
  @brief   MP2BaseFile Module.

  @detail This module is to record the MP2 streams
*/
class MP2BaseFile : public MUXBase
{
public:
    //! constructor
    MP2BaseFile( MUX_create_params_type *,
                 MUX_fmt_type file_format,
                 MUX_brand_type file_brand,
                 MUX_handle_type *output_handle
                 );
    //! destructor
    virtual ~MP2BaseFile();


/* =======================================================================
**                          Function Declarations
** ======================================================================= */
virtual MUX_STATUS MUX_Process_Sample( uint32 stream_number,
                                        uint32 num_samples,
                                        const MUX_sample_info_type  *sample_info,
                                        const uint8  *sample_data);
virtual MUX_STATUS MUX_write_header (uint32 stream_id, uint32 header_size, const uint8 *header);

virtual MUX_STATUS MUX_update_AVC_Timing_HRD_Params
                                    (MUX_AVC_TimingHRD_params_type *pHRDParams);
virtual MUX_STATUS MUX_update_streamport(uint64_t port);

virtual MUX_STATUS MUX_end_Processing();

virtual MUX_STATUS MUX_get_current_PTS(uint64 *pnPTS);
#if 0 //def FILEMUX_WRITE_ASYNC
virtual int FlushStream(void);
#endif /* FILEMUX_WRITE_ASYNC */
protected:
void MUX_Modify_AVC_Start_Code(
                            const uint8 * pBuffer,
                            uint32*  nSize,
                            bool bHeader);

uint32 MUX_FindNextPattern(const uint8 *streamBuf,
                           uint32 bufSize,
                           uint32 startcode);

uint32 FindCheckSum(uint8 *pData,uint32 nSize);


void InitData ();
void OpenFiles();
void GeneratePATPacket();
void GeneratePMTPacket();
uint16 GeneratePESPacket( uint32   stream_number,
                                 const MUX_sample_info_type       *sample_info,
                                 uint8  *PES_Header );
uint32 GenerateMP2TSVideoPacket( uint32   stream_number,
                                 MUX_sample_info_type       *sample_info,
                                 const uint8  *PES_Header,
                                 const uint16 PES_Header_Size,
                                 uint8  *sample_data,
                                 const uint32 offset_sample_data);

uint32 GenerateMP2TSPCRPacket( uint32   stream_number,
                                 MUX_sample_info_type       *sample_info);

uint32 GenerateMP2TSAudioPacket( uint32   stream_number,
                                 MUX_sample_info_type       *sample_info,
                                 const uint8  *PES_Header,
                                 const uint16 PES_Header_Size,
                                 uint8  *sample_data,
                                 const uint32 offset_sample_data);


MUX_STATUS GenerateMP2TSVideoPacketForHeader( uint32 stream_number,
                                              uint32 header_size,
                                              uint8 *header );
uint16 GenerateFreezeFramePESPacket(  uint32   stream_number,
                                       const MUX_sample_info_type  *sample_info,
                                       uint8  *PES_Header );

bool Destination_Mmc(const WCHAR *);
boolean Output (const uint8  *data, uint32 offset, uint32 len, bool bEnd);

#ifdef FILEMUX_WRITE_ASYNC
    bool   OpenFileHandler();
    void   CloseFileHandler();
    uint32 GetStreamBufferOccupancy(Mux_mp2_stream_handler_info_type  *m_sStreamHandler);
    uint32 PushStream(const uint8  *data, uint32 offset, uint32 len);
#endif /*  FILEMUX_WRITE_ASYNC */

uint32 Space_Check(void);
void handle_close ();
void close_writer (void);
static void timer_callback(void*);
bool GenerateAVCHRDTimingDescriptor
                        (MUX_AVC_TimingHRD_params_type *pAVCHrdParams);

#ifdef FILEMUX_WRITE_ASYNC
    mux_write_file_cb_func_ptr_type               pFnFileWrite;
    void*                                         pClientData;
    uint32                                        output_unit_size;
    Mux_mp2_stream_handler_info_type              m_sStreamHandler;
    bool                                          closeissued;
    uint8                                         mdatAtomSize[4];
    MM_HANDLE                                     MP2BaseFileMux_CS;
    MM_HANDLE                                     pBuffAvailSig;
    MM_HANDLE                                     pBuffManSigQ;
    static const uint32                           BUF_AVAIL_EVENT;
#endif /* FILEMUX_WRITE_ASYNC  */
static uint64                                 m_llTimeBase;
uint64                                        m_llVideoDelayCorrection;
bool                                          m_bLookforIframe;
uint8                                         m_TS_Stream_buf[TS_RTP_PAYLOAD_SIZE]; // 7 TS packets
uint64                                        m_nTS_Stream_bufsz;
MM_HANDLE                                     m_video_frame_timer;
bool                                          m_save_to_mmc;
MUX_brand_type                                m_file_brand;
MUX_create_params_type                        *m_Params;
Mux_AAC_Format_Type                           m_eAACFormat;
uint8                                         m_audio_stream_num;
uint8                                         m_video_stream_num;
MP2_MUX_file                                  m_filePtr;
uint32                                        m_output_offset;
uint32                                        m_output_size;
boolean                                       space_out_near;
boolean                                       space_out_imminent;
uint32                                        mmc_free_space;
uint32                                        flash_free_space;
uint64                                        m_nBaseTimestamp;
uint64                                        m_nPrevTimeStamp;
uint64                                        m_nCurrPCR;
uint32                                        m_nAudioFrameRate;
uint32                                        m_nVideoFrameRate;
uint64                                        m_nCurrAudioTime;
uint64                                        m_nCurrVideoTime; 
uint32                                        m_aCRCTable[256];
uint8*                                        m_pHeader[MUX_MAX_MEDIA_STREAMS];
uint32                                        m_nHeaderSize[MUX_MAX_MEDIA_STREAMS];
bool                                          m_bHeaderSent[MUX_MAX_MEDIA_STREAMS];
bool                                          m_bGenerateTables;
boolean                                       space_out;
MUX_handle_type                               m_output_handle;
boolean                                       m_output_open;
boolean                                       m_write_fail;
uint8*                                        m_MP2TSPacket;
uint8*                                        m_MP2PCRTSPacket;
uint8*                                        m_MP2PESPacket;
uint8*                                        m_pAVCHrdDescr;
uint8                                         m_nAVCHrdDescrLen;
MUX_AVC_TimingHRD_params_type                 m_sHRDParams;
boolean                                       m_PSI_payload_start_indicator_set;
uint8                                         m_MP2TS_continuty_counter[MUX_MAX_MEDIA_STREAMS];
uint8                                         m_MP2TS_table_continuty_counter[2];
boolean                                       m_Transport_priority;
boolean                                       m_randon_access_indicator;
uint8                                         m_adaptation_field;
uint8                                         m_PTS_DTS_Flags;
boolean                                       m_data_alignment_indicator;
bool                                          m_bFirstFrameDrop;
boolean                                       m_bAudioPresent;
boolean                                       m_bVideoPresent;
bool                                          m_bBaseTimeStampTaken;
MM_HANDLE                                     m_hCritSect;
uint32                                        m_nAudioFrameDuration90Khz;
bool                                          m_bAdjustTSforCompliance;
bool                                          m_bSendFillerNalu;
private:

#ifdef ENABLE_MUX_STATS
typedef struct mux_stats_struct
{
    uint32  nStatCount;
    unsigned long nStartTime;
    unsigned long nEndTime;
    unsigned long  nMaxTime;
    bool    bEnableMuxStat;
}mux_stats;
mux_stats muxStats;
MM_HANDLE m_pStatTimer;
uint32 m_nDuration;
static void readStatTimerHandler(void * ptr);
#endif

};
#endif  //__MUX_MP2BASEFILE_H__

