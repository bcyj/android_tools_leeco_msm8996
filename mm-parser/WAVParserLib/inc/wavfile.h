#ifndef WAV_FILE_H
#define WAV_FILE_H

/* =======================================================================
                              wavfile.h
DESCRIPTION

Copyright (c) 2009-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/WAVParserLib/main/latest/inc/wavfile.h#29 $
$DateTime: 2013/08/27 03:52:56 $
$Change: 4338908 $

========================================================================== */

//=============================================================================
// INCLUDES
//=============================================================================

#include "wavheaders.h"
#include "parserdatadef.h"
#include "oscl_file_io.h"
#include "qcplayer_oscl_utils.h"
#include "filebase.h"
#include "CAdpcmDecoderLib.h"
#ifdef FEATURE_FILESOURCE_WAVADPCM
// =======================================================================
// Forward Declaration
// =======================================================================
class wavformatParser;
class IxStream;
// =======================================================================
// Global Constant Data Declarations
// =======================================================================
#define AUDIO_WAV_MAX_TRACKS      1
#define WAV_STREAM_TIME_SCALE 1000  //milisec
#define AUDIO_WAV_MAX_FRAME_SIZE  3000
#define WAV_DEFAULT_AUDIO_BUF_SIZE 800

class WAVFile : public FileBase, public Parentable
{
//=============================================================================
// PRIVATE CLASS ATTRIBUTES
//=============================================================================
  private:
    unsigned char  *m_pFileBuf;  // pointer to buffer for playback from memory //
    FILESOURCE_STRING  m_filename;  // EFS file path //
    OSCL_FILE  *m_WAVFilePtr; //pointer to the file
    uint64  m_FileBufSize;
    uint64  m_fileSize;
    bool    m_bStreaming;
    bool    m_SEEK_DONE;
    uint64  m_uSeektime;
    uint16  m_BlockSize;
    uint32  m_DecodedBytes;
    uint32  m_wavformat;
    uint32  m_validDataOffset;
    uint8*  m_pDataBuffer;
    uint32  m_nDataBufSize;
    file_sample_info_type  m_audsampleinfo;
    wavformatParser  *m_pwavformatParser; // handle to amrparser module
    CAdpcmDecoderLib *m_pimaAdpcmDecoder; // handle to ADPCM Decoder Lib
    IxStream* m_pIxStream;
    FileSourceConfigItemEnum m_eConfigItem;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
    video::iStreamPort* m_pPort;
#endif
    /* PRIVATE CLASS METHODS */
    bool ParseWAVHeader();
    void InitData();
/*===========================================================================
   PUBLIC CLASS METHODS
=============================================================================*/
  public:
    //Default construnctor
    WAVFile();

    //destructor
    virtual ~WAVFile();

    uint32  FileGetData(uint64 nOffset,
                        uint32 nNumBytesRequest,
                        uint32 nMaxSize,
                        uint8 *ppData );

    WAVFile( const FILESOURCE_STRING &filename,
             unsigned char *pFileBuf,
             uint64 bufSize);
    WAVFile(IxStream*);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
    WAVFile(video::iStreamPort*);
#endif

    virtual uint64  getMediaTimestampForCurrentSample(uint32 /*id*/) ;
    virtual uint64  getMovieDuration() const;
    virtual uint32  getMovieTimescale() const;
    virtual uint64  getTrackMediaDuration(uint32 /*id*/);
    virtual uint32  getTrackMediaTimescale(uint32 /*id*/);

    virtual uint32  getTrackAudioSamplingFreq(uint32 /*id*/);
    virtual uint32  getTrackWholeIDList( uint32 *ids);
    virtual uint32  GetAudioChannelMask(int);
    virtual uint8   getTrackOTIType(uint32 id);
    virtual int32   getTrackMaxBufferSizeDB(uint32 /*id*/);
    virtual uint8   randomAccessDenied();
    virtual uint32  GetNumAudioChannels(int /*id*/);
    virtual uint32  GetAudioBitsPerSample(int /*id*/);

    virtual PARSER_ERRORTYPE  getNextMediaSample(uint32 /*ulTrackID*/,
                                                 uint8 *pucDataBuf,
                                                 uint32 *pulBufSize,
                                                 uint32 &/*rulIndex*/);
    virtual uint64  resetPlayback( uint64 pos, uint32 id
                                  ,bool /*bSetToSyncSample*/
                                  ,bool* /*bError */
                                  ,uint64 currentPosTimeStamp );

    virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
    {
      fileFormat = FILE_SOURCE_WAV;
      return FILE_SOURCE_SUCCESS;
    }
    virtual uint64 GetFileSize() {return m_fileSize;};
    virtual int32  getNumTracks(){return AUDIO_WAV_MAX_TRACKS;};

    virtual PARSER_ERRORTYPE  peekCurSample(uint32 /*trackid*/ ,
                                           file_sample_info_type *pSampleInfo);

    virtual void SetCriticalSection(MM_HANDLE);
    virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id,
                                                                uint8* buf,
                                                                uint32 *pbufSize);
    virtual FileSourceStatus SetConfiguration(FileSourceConfigItemEnum eConfigItem);
};
#endif //FEATURE_FILESOURCE_WAVADPCM
#endif //WAV_FILE_H
