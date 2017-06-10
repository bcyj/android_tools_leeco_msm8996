#ifndef __MP2BASEFILESECURE_H__
#define __MP2BASEFILESECURE_H__

/* =======================================================================
                              MP2BaseFileSecure.h
DESCRIPTION

  Copyright (c) 2011 - 2013 Qualcomm Technologies, Inc.All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */


/* =======================================================================
                             Edit History
$Source: //source/qcom/qct/multimedia2/Video/Source/FileMux/Sink/FileMux/MP2BaseFileLib/main/latest/inc/MP2BaseFileSecure.h

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "muxbase.h"
#include "smux_mem.h"
/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

typedef enum SecMux_AAC_Format
{
    SEC_AAC_FORMAT_ADTS,
    SEC_AAC_FORMAT_LOAS
}SecMux_AAC_Format_Type;

/* -----------------------------------------------------------------------
** MP2BaseFileSecure Class Definition
**
** This class Mux secure Video/Audio sample to secure output stream.
** ----------------------------------------------------------------------- */

class MP2BaseFileSecure : public MUXBase
{
public:
    //! constructor
    MP2BaseFileSecure( MUX_create_params_type *,
                 MUX_fmt_type file_format,
                 MUX_brand_type file_brand,
                 MUX_handle_type *output_handle
                 );
    //! destructor
    virtual ~MP2BaseFileSecure();


/* =======================================================================
**                          Function Declarations
** ======================================================================= */
virtual MUX_STATUS MUX_Process_Sample( uint32 stream_number,
                                        uint32 num_samples,
                                        const MUX_sample_info_type  *sample_info,
                                        const uint8  *sample_data);

MUX_STATUS MUX_write_user_meta_data (const void *data, uint32 size, uint32 timeout);

virtual MUX_STATUS MUX_end_Processing();

protected:

uint32 FindCheckSum(uint8 *pData, uint32 nSize);
void InitData ();
void GeneratePATPacket();
void GeneratePMTPacket();
uint32 GenerateMP2TSPCRPacket(MUX_sample_info_type   *sample_info);
void PCR_callback(void*);
bool GenerateAVCHRDTimingDescriptor
                        (MUX_AVC_TimingHRD_params_type *pAVCHrdParams);

static uint64                                m_llTimeBase;
uint64                                        m_prev_PCR_time;
uint64                                        m_prev_userdata_time;
uint64                                        m_llVideoDelayCorrection;
bool                                          m_bLookforIframe;
MUX_brand_type                                m_file_brand;
MUX_create_params_type                        *m_Params;
SecMux_AAC_Format_Type                        m_eAACFormat;
uint8                                         m_audio_stream_num;
uint8                                         m_video_stream_num;
uint64                                        m_nBaseTimestamp;
uint64                                        m_nCurrPCR;
uint32                                        m_nAudioFrameRate;
uint32                                        m_nVideoFrameRate;
uint64                                        m_nCurrAudioTime;
uint64                                        m_nCurrVideoTime;
bool                                          m_bGenerateTables;
MUX_handle_type                               m_output_handle;
uint8*                                        m_pAVCHrdDescr;
uint8                                         m_nAVCHrdDescrLen;
MUX_AVC_TimingHRD_params_type                 m_sHRDParams;
uint8                                         m_user_data_continuty_counter;
uint8                                         m_MP2TS_table_continuty_counter[3];
boolean                                       m_randon_access_indicator;
uint8                                         m_adaptation_field;
boolean                                       m_bAudioPresent;
boolean                                       m_bVideoPresent;
bool                                          m_bBaseTimeStampTaken;
uint32                                        m_nAudioFrameDuration90Khz;
bool                                          m_bAdjustTSforCompliance;
struct QSEECom_handle*                        securemux_handler;
boolean                                       m_is_pending_tables;
struct smux_ion_info                        m_PAT_PMT_PCR_handler;
struct smux_ion_info*                        m_userdata_handler;
uint64                                        m_user_timeout;
uint16                                        m_table_offset;
bool                                          m_is_PCR_data;
bool                                          m_is_user_data;

};
#endif  //__MP2BASEFILESECURE_H__

