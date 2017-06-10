#ifndef QCP_FILE_H
#define QCP_FILE_H
// =======================================================================
//                              qcpfile.h
//DESCRIPTION
//  Meaningful description of the definitions contained in this file.
//  Description must specify if the module is portable specific, mobile
//  specific, or common to both, and it should alert the reader if the
//  module contains any conditional definitions which tailors the module to
//  different targets.
//  Copyright 2009-2013 Qualcomm Technologies Inc., All Rights Reserved
//  Qualcomm Technologies Proprietary and Confidential.

//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/QCPParserLib/main/latest/inc/qcpfile.h#27 $
//$DateTime: 2013/08/27 03:52:56 $
//$Change: 4338908 $

//==========================================================================


//=======================================================================
//               Includes and Public Data Declarations
//==========================================================================
//==========================================================================

//                     INCLUDE FILES FOR MODULE

//==========================================================================

#include "oscl_file_io.h"
#include "filebase.h"
#include "qcpheaders.h"
#include "parserdatadef.h"
#include "filesourcetypes.h"
//==========================================================================

//                        DATA DECLARATIONS

//==========================================================================
// -----------------------------------------------------------------------
// Constant / Define Declarations
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Type Declarations
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------
class qcpParser;
class IxStream;
//-----------------------------------------------------------------------
// Global Constant Data Declarations
//-----------------------------------------------------------------------

#define AUDIO_QCP_MAX_TRACKS      1
#define QCP_STREAM_TIME_SCALE 1000  //milisec
#define AUDIO_QCP_MAX_FRAME_SIZE  32
#define QCP_DEFAULT_AUDIO_BUF_SIZE 800

// -----------------------------------------------------------------------
// Global Data Declarations
// -----------------------------------------------------------------------

//=======================================================================
//                          Function Declaration
//=======================================================================

//=======================================================================
//                        Class Declarations
//=======================================================================

//======================================================================
//CLASS
//  QCPFile
//
//DESCRIPTION
//  Thorough, meaningful description of what this function does.
//
//DEPENDENCIES
//  List any dependencies for this function, global variables, state,
//  resource availability, etc.
//
//RETURN VALUE
//  Enumerate possible return values
//
//SIDE EFFECTS
//  Detail any side effects.
//==========================================================================

class QCPFile : public FileBase, public Parentable
{

private:
  unsigned char        *m_pFileBuf;     /* pointer to buffer for playback   */
  uint64                m_FileBufSize;  /* file buffer size                 */
  uint64                m_fileSize;     /* file length                      */
  FILESOURCE_STRING     m_filename;     /* EFS file path specified          */
  bool                  m_bStreaming;   /* boolean to check streaming status*/
  OSCL_FILE            *m_QCPFilePtr;   /* pointer to the file              */
  qcpParser            *m_pqcpParser;   /* handle to qcp parser module      */
  file_sample_info_type m_audsampleinfo;/* qcp audio sample information     */
  bool                  m_SEEK_DONE;    /* boolean to check file seek status*/
  uint64                m_uSeektime;    /* timestamp for seek               */
  IxStream             *m_pIxStream;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* m_pPort;
#endif

  bool ParseQCPHeader();                  /**< calls new on QCP parser      */
  void InitData();

public:
  uint32 FileGetData
     (
     uint64 nOffset,                  /**< offset in file           */
     uint32 nNumBytesRequest,         /**< number of byte requested */
     uint32 nMaxSize,                 /**< maximum size             */
     uint8 *ppData                    /**< file data read           */
     );

  QCPFile();                             /**< default constructor      */

  QCPFile(const FILESOURCE_STRING &filename, unsigned char *pFileBuf,
          uint64 bufSize);
  QCPFile(IxStream*);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  QCPFile(video::iStreamPort*);
#endif
  virtual ~QCPFile();                     /**< destructor               */

  virtual  uint64 getMediaTimestampForCurrentSample(uint32 /*id*/);
  virtual uint64 getMovieDuration() const;
  virtual uint32 getMovieTimescale() const;
  virtual uint64 getTrackMediaDuration(uint32 /*id*/);
  virtual uint32 getTrackMediaTimescale(uint32 /*id*/);
  virtual uint32 getTrackAudioSamplingFreq(uint32 /*id*/);
  virtual uint8  getTrackOTIType(uint32 /*id*/);
  virtual int32  getTrackMaxBufferSizeDB(uint32 /*id*/);
  virtual uint32 getTrackWholeIDList( uint32 *ids);
  virtual uint32 GetNumAudioChannels(int /*id*/);
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum Enum);
  virtual FileSourceStatus GetAudioOutputMode(bool* bret,
                                              FileSourceConfigItemEnum Enum);

  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_QCP;
    return FILE_SOURCE_SUCCESS;
  }
  virtual int32  getNumTracks(){return AUDIO_QCP_MAX_TRACKS;};
  virtual uint64 GetFileSize() {return m_fileSize;};
  virtual uint64 resetPlayback
     (
     uint64  pos,
     uint32  /*id*/,
     bool    /*bSetToSyncSample*/,
     bool*   /*bError*/,
     uint64  /*currentPosTimeStamp*/
     );
  virtual uint8 randomAccessDenied();

  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID,
                                              uint8 *pucDataBuf,
                                              uint32 *pulBufSize,
                                              uint32 &rulIndex);
  virtual PARSER_ERRORTYPE peekCurSample (uint32 /*trackid*/,
                                          file_sample_info_type *pSampleInfo);

  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 /*id*/,
                                                             uint8*  buf,
                                                             uint32 *pbufSize);

  virtual void SetCriticalSection(MM_HANDLE);

};

#endif  /* __QCPFile_H__ */
