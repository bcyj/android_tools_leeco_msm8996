#ifndef __AMRFile_H__
#define __AMRFile_H__
/* =======================================================================
                              amrfile.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright (c) 2009-2015 Qualcomm Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRNBParserLib/main/latest/inc/amrfile.h#23 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "oscl_file_io.h"
#include "qcplayer_oscl_utils.h"

#include "filebase.h"
#include "amrheaders.h"
#ifdef FEATURE_FILESOURCE_AMR
/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class amrParser;
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
#define AUDIO_AMR_MAX_TRACKS      1
#define AMR_STREAM_TIME_SCALE 1000  //milisec
#define AUDIO_AMR_MAX_FRAME_SIZE  32
#define AMR_DEFAULT_AUDIO_BUF_SIZE 800

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Function Declaration
** ======================================================================= */

/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  AMRFile

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.
========================================================================== */

class AMRFile : public FileBase, public Parentable
{
private:
  unsigned char  *m_pFileBuf;  // pointer to buffer for playback from memory //
  uint64          m_FileBufSize;
  uint64          m_fileSize;
  bool            m_bStreaming;
  bool            m_SEEK_DONE;
  uint64          m_uSeektime;
  IxStream*       m_pIxStream;
  OSCL_FILE*      m_AMRFilePtr; //pointer to the file
  amrParser*      m_pamrParser; // handle to amrparser module
  FILESOURCE_STRING     m_filename;  // EFS file path //
  file_sample_info_type m_audsampleinfo;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* m_pPort;
#endif

  bool ParseAMRHeader(); //calls new on AMR parser
  void InitData();

public:
  AMRFile(); //Default construnctor

  AMRFile(const FILESOURCE_STRING &filename, unsigned char *pFileBuf,
          uint64 bufSize);
  AMRFile(IxStream*);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  AMRFile(video::iStreamPort*);
#endif
  virtual ~AMRFile(); // destructor

  uint32 FileGetData(uint64 nOffset, uint32 nNumBytesRequest,
                     uint32 nMaxSize, uint8 *ppData);

  //inline functions
  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_AMR_NB;
    return FILE_SOURCE_SUCCESS;
  }
  virtual uint64 GetFileSize() {return m_fileSize;};
  virtual int32  getNumTracks(){return AUDIO_AMR_MAX_TRACKS;};
  virtual uint8  getTrackOTIType(uint32 /*id*/ ) {return AMR_AUDIO;};

   // Timestamp/Duration related
  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);
  virtual uint64 getMovieDuration() const;
  virtual uint32 getMovieTimescale() const
                 {return MILLISEC_TIMESCALE_UNIT;};
  virtual uint64 getTrackMediaDuration(uint32 /*id*/)
                 {return getMovieDuration();};
  virtual uint32 getTrackMediaTimescale(uint32 /*id*/)
                 {return MILLISEC_TIMESCALE_UNIT;};

  // Other audio related APIs
  virtual uint32 getTrackWholeIDList( uint32 *ids);
  virtual uint32 getTrackAudioSamplingFreq(uint32 id);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);
  virtual uint32 GetNumAudioChannels(int id);

  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid,
                                         file_sample_info_type* pSampleInfo);

/* ======================================================================
  FUNCTION
    getNextMediaSample

  DESCRIPTION
    Its a function from the SDK to get sample data from file.

  ARGUMENTS
    ulTrackID    TrackID of elementary stream
    pucDataBuf   Data buffer for frame
    pulBufSize   Data buffer size
    rulIndex     Index

  DEPENDENCIES
    None

  RETURN VALUE
    PARSER_ERRORTYPE

  SIDE EFFECTS
    None
========================================================================== */
  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID,
                                              uint8 *pucDataBuf,
                                              uint32 *pulBufSize,
                                              uint32 &rulIndex);
  virtual uint64 resetPlayback(uint64 pos, uint32 id,
                               bool bSetToSyncSample, bool *bError,
                               uint64 currentPosTimeStamp);
  virtual uint8  randomAccessDenied() {
    if (getMovieDuration()) return 0;
    return 1; }

  virtual void SetCriticalSection(MM_HANDLE);

  /*!
  @brief      Set Audio track output mode.
  @details    This function is used to set Audio track output mode.
  @param[in]  eConfigParam  Configuration parameter need to set for o/p mode.
  @return     FileSourceStatus status.
                FILE_SOURCE_SUCCESS in case of success.
                FILE_SOURCE_FAIL in case of error.
  @note       None
  */
  virtual FileSourceStatus SetAudioOutputMode(
                            FileSourceConfigItemEnum eConfigParam);

  /*!
  @brief      Get Audio track output mode
  @details    This function is used to get Audio track output mode.
  @param[out] pbConfigStatus  Status of queried configuration parameter
  @param[in]  eConfigParam    Configuration parameter queried for o/p mode
  @return     FileSourceStatus status.
                FILE_SOURCE_SUCCESS in case of success.
                FILE_SOURCE_FAIL in case of error.
  @note       None
  */
  virtual FileSourceStatus GetAudioOutputMode(
                            bool* pbConfigStatus,
                            FileSourceConfigItemEnum eConfigParam);
};
#endif //FEATURE_FILESOURCE_AMR
#endif  /* __AMRFile_H__ */

